/****************************************************************************
*
*    Copyright (c) 2005 - 2018 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


#include "gc_glsl_emit_code.h"
#define _USE_F2I_OPCODE  1

gceSTATUS
sloCODE_EMITTER_NewBasicBlock(
    IN sloCOMPILER Compiler,
    IN sloCODE_EMITTER CodeEmitter
    );

gceSTATUS
sloCODE_EMITTER_EndBasicBlock(
    IN sloCOMPILER Compiler,
    IN sloCODE_EMITTER CodeEmitter
    );

gceSTATUS
sloCODE_EMITTER_EmitCode1(
    IN sloCOMPILER Compiler,
    IN sloCODE_EMITTER CodeEmitter,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN sleOPCODE Opcode,
    IN gcsTARGET * Target,
    IN gcsSOURCE * Source
    );

gceSTATUS
sloCODE_EMITTER_EmitCode2(
    IN sloCOMPILER Compiler,
    IN sloCODE_EMITTER CodeEmitter,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN sleOPCODE Opcode,
    IN gcsTARGET * Target,
    IN gcsSOURCE * Source0,
    IN gcsSOURCE * Source1
    );

gcSL_CONDITION
_ConvCondition(
    IN sleCONDITION Condition
    );

static gceSTATUS
_EmitOpcodeConditionAndFormat(
    IN sloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gcSL_OPCODE Opcode,
    IN gcSL_CONDITION Condition,
    IN gcSL_FORMAT Format,
    IN gcSHADER_PRECISION Precision,
    IN gcsTARGET * Target
    );

#if GC_ENABLE_DUAL_FP16
static gctCONST_STRING
_GetPrecisionName(
    IN gcSHADER_PRECISION Precision
)
{
    switch (Precision)
    {
    case gcSHADER_PRECISION_DEFAULT:
        return "default_precision";

    case gcSHADER_PRECISION_HIGH:
        return "highp";

    case gcSHADER_PRECISION_MEDIUM:
        return "mediump";

    case gcSHADER_PRECISION_LOW:
        return "lowp";

    case gcSHADER_PRECISION_ANY:
        return "anyp";

    default:
        gcmASSERT(0);
        return "Invalid";
    }
}
#endif

gctUINT
gcGetDataTypeSize(
    IN gcSHADER_TYPE DataType
    )
{
    switch (DataType)
    {
    case gcSHADER_FLOAT_X1:
    case gcSHADER_FLOAT_X2:
    case gcSHADER_FLOAT_X3:
    case gcSHADER_FLOAT_X4:
    case gcSHADER_BOOLEAN_X1:
    case gcSHADER_BOOLEAN_X2:
    case gcSHADER_BOOLEAN_X3:
    case gcSHADER_BOOLEAN_X4:
    case gcSHADER_INTEGER_X1:
    case gcSHADER_INTEGER_X2:
    case gcSHADER_INTEGER_X3:
    case gcSHADER_INTEGER_X4:
    case gcSHADER_UINT_X1:
    case gcSHADER_UINT_X2:
    case gcSHADER_UINT_X3:
    case gcSHADER_UINT_X4:
    case gcSHADER_SAMPLER_1D:
    case gcSHADER_SAMPLER_2D:
    case gcSHADER_SAMPLER_3D:
    case gcSHADER_SAMPLER_BUFFER:
    case gcSHADER_SAMPLER_CUBIC:
    case gcSHADER_SAMPLER_CUBEMAP_ARRAY:
    case gcSHADER_SAMPLER_EXTERNAL_OES:
    case gcSHADER_SAMPLER_2D_SHADOW:
    case gcSHADER_SAMPLER_CUBE_SHADOW:
    case gcSHADER_SAMPLER_CUBEMAP_ARRAY_SHADOW:
    case gcSHADER_SAMPLER_2D_MS:
    case gcSHADER_SAMPLER_2D_MS_ARRAY:

    case gcSHADER_SAMPLER_1D_ARRAY:
    case gcSHADER_SAMPLER_1D_ARRAY_SHADOW:
    case gcSHADER_SAMPLER_2D_ARRAY:
    case gcSHADER_SAMPLER_2D_ARRAY_SHADOW:

    case gcSHADER_ISAMPLER_2D:
    case gcSHADER_ISAMPLER_3D:
    case gcSHADER_ISAMPLER_BUFFER:
    case gcSHADER_ISAMPLER_CUBIC:
    case gcSHADER_ISAMPLER_CUBEMAP_ARRAY:
    case gcSHADER_ISAMPLER_2D_ARRAY:
    case gcSHADER_ISAMPLER_2D_MS:
    case gcSHADER_ISAMPLER_2D_MS_ARRAY:

    case gcSHADER_USAMPLER_2D:
    case gcSHADER_USAMPLER_3D:
    case gcSHADER_USAMPLER_BUFFER:
    case gcSHADER_USAMPLER_CUBIC:
    case gcSHADER_USAMPLER_CUBEMAP_ARRAY:
    case gcSHADER_USAMPLER_2D_ARRAY:
    case gcSHADER_USAMPLER_2D_MS:
    case gcSHADER_USAMPLER_2D_MS_ARRAY:

    case gcSHADER_IMAGE_2D:
    case gcSHADER_IMAGE_3D:
    case gcSHADER_IMAGE_CUBE:
    case gcSHADER_IMAGE_CUBEMAP_ARRAY:
    case gcSHADER_IMAGE_2D_ARRAY:
    case gcSHADER_IMAGE_BUFFER:
    case gcSHADER_IIMAGE_2D:
    case gcSHADER_IIMAGE_3D:
    case gcSHADER_IIMAGE_CUBE:
    case gcSHADER_IIMAGE_2D_ARRAY:
    case gcSHADER_IIMAGE_CUBEMAP_ARRAY:
    case gcSHADER_IIMAGE_BUFFER:
    case gcSHADER_UIMAGE_2D:
    case gcSHADER_UIMAGE_3D:
    case gcSHADER_UIMAGE_CUBE:
    case gcSHADER_UIMAGE_CUBEMAP_ARRAY:
    case gcSHADER_UIMAGE_2D_ARRAY:
    case gcSHADER_UIMAGE_BUFFER:

    case gcSHADER_ATOMIC_UINT:
        return 1;

    case gcSHADER_FLOAT_2X2:
    case gcSHADER_FLOAT_2X3:
    case gcSHADER_FLOAT_2X4:
        return 2;

    case gcSHADER_FLOAT_3X3:
    case gcSHADER_FLOAT_3X2:
    case gcSHADER_FLOAT_3X4:
        return 3;

    case gcSHADER_FLOAT_4X4:
    case gcSHADER_FLOAT_4X2:
    case gcSHADER_FLOAT_4X3:
        return 4;

    default:
        gcmASSERT(0);
        return 1;
    }
}

gctUINT8
gcGetDataTypeComponentCount(
    IN gcSHADER_TYPE DataType
    )
{
    switch (DataType)
    {
    case gcSHADER_FLOAT_X1:
    case gcSHADER_BOOLEAN_X1:
    case gcSHADER_INTEGER_X1:
    case gcSHADER_UINT_X1:
    case gcSHADER_ATOMIC_UINT:
        return 1;

    case gcSHADER_FLOAT_X2:
    case gcSHADER_BOOLEAN_X2:
    case gcSHADER_INTEGER_X2:
    case gcSHADER_UINT_X2:
        return 2;

    case gcSHADER_FLOAT_X3:
    case gcSHADER_BOOLEAN_X3:
    case gcSHADER_INTEGER_X3:
    case gcSHADER_UINT_X3:
        return 3;

    case gcSHADER_FLOAT_X4:
    case gcSHADER_BOOLEAN_X4:
    case gcSHADER_INTEGER_X4:
    case gcSHADER_UINT_X4:

    case gcSHADER_SAMPLER_1D:
    case gcSHADER_SAMPLER_2D:
    case gcSHADER_SAMPLER_3D:
    case gcSHADER_SAMPLER_BUFFER:
    case gcSHADER_SAMPLER_CUBIC:
    case gcSHADER_SAMPLER_EXTERNAL_OES:
    case gcSHADER_SAMPLER_2D_SHADOW:
    case gcSHADER_SAMPLER_CUBE_SHADOW:

    case gcSHADER_SAMPLER_1D_ARRAY:
    case gcSHADER_SAMPLER_1D_ARRAY_SHADOW:
    case gcSHADER_SAMPLER_2D_ARRAY:
    case gcSHADER_SAMPLER_2D_ARRAY_SHADOW:

    case gcSHADER_ISAMPLER_2D:
    case gcSHADER_ISAMPLER_3D:
    case gcSHADER_ISAMPLER_BUFFER:
    case gcSHADER_ISAMPLER_CUBIC:
    case gcSHADER_ISAMPLER_2D_ARRAY:

    case gcSHADER_USAMPLER_2D:
    case gcSHADER_USAMPLER_3D:
    case gcSHADER_USAMPLER_BUFFER:
    case gcSHADER_USAMPLER_CUBIC:
    case gcSHADER_USAMPLER_2D_ARRAY:

    case gcSHADER_SAMPLER_2D_MS:
    case gcSHADER_ISAMPLER_2D_MS:
    case gcSHADER_USAMPLER_2D_MS:
    case gcSHADER_SAMPLER_2D_MS_ARRAY:
    case gcSHADER_ISAMPLER_2D_MS_ARRAY:
    case gcSHADER_USAMPLER_2D_MS_ARRAY:
    case gcSHADER_IMAGE_2D:                      /* 0x17 */
    case gcSHADER_IIMAGE_2D:                     /* 0x39 */
    case gcSHADER_UIMAGE_2D:                     /* 0x3A */
    case gcSHADER_IMAGE_3D:                      /* 0x18 */
    case gcSHADER_IIMAGE_3D:                     /* 0x3B */
    case gcSHADER_UIMAGE_3D:                     /* 0x3C */
    case gcSHADER_IMAGE_CUBE:                    /* 0x3D */
    case gcSHADER_IIMAGE_CUBE:                   /* 0x3E */
    case gcSHADER_UIMAGE_CUBE:                   /* 0x3F */
    case gcSHADER_IMAGE_2D_ARRAY:                /* 0x40 */
    case gcSHADER_IIMAGE_2D_ARRAY:               /* 0x41 */
    case gcSHADER_UIMAGE_2D_ARRAY:               /* 0x42 */
    case gcSHADER_IMAGE_BUFFER:
    case gcSHADER_IIMAGE_BUFFER:
    case gcSHADER_UIMAGE_BUFFER:

    case gcSHADER_FLOAT_2X2:
        return 4;

    case gcSHADER_FLOAT_2X3:
        return 6;

    case gcSHADER_FLOAT_2X4:
        return 8;

    case gcSHADER_FLOAT_3X3:
        return 9;

    case gcSHADER_FLOAT_3X2:
        return 6;

    case gcSHADER_FLOAT_3X4:
        return 12;

    case gcSHADER_FLOAT_4X4:
        return 16;

    case gcSHADER_FLOAT_4X2:
        return 8;

    case gcSHADER_FLOAT_4X3:
        return 12;

    default:
        gcmASSERT(0);
        return 1;
    }
}

gcSHADER_TYPE
gcGetComponentDataType(
    IN gcSHADER_TYPE DataType
    )
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
        return gcSHADER_FLOAT_X1;

    case gcSHADER_BOOLEAN_X1:
    case gcSHADER_BOOLEAN_X2:
    case gcSHADER_BOOLEAN_X3:
    case gcSHADER_BOOLEAN_X4:
        return gcSHADER_BOOLEAN_X1;

    case gcSHADER_INTEGER_X1:
    case gcSHADER_INTEGER_X2:
    case gcSHADER_INTEGER_X3:
    case gcSHADER_INTEGER_X4:
        return gcSHADER_INTEGER_X1;

    case gcSHADER_UINT_X1:
    case gcSHADER_UINT_X2:
    case gcSHADER_UINT_X3:
    case gcSHADER_UINT_X4:
        return gcSHADER_UINT_X1;

    default:
        gcmASSERT(0);
        return gcSHADER_FLOAT_X1;
    }
}

static gctCONST_STRING OpName[] =
{
    "gcSL_NOP",                           /* 0x00 */
    "gcSL_MOV",                           /* 0x01 */
    "gcSL_SAT",                           /* 0x02 */
    "gcSL_DP3",                           /* 0x03 */
    "gcSL_DP4",                           /* 0x04 */
    "gcSL_ABS",                           /* 0x05 */
    "gcSL_JMP",                           /* 0x06 */
    "gcSL_ADD",                           /* 0x07 */
    "gcSL_MUL",                           /* 0x08 */
    "gcSL_RCP",                           /* 0x09 */
    "gcSL_SUB",                           /* 0x0A */
    "gcSL_KILL",                          /* 0x0B */
    "gcSL_TEXLD",                         /* 0x0C */
    "gcSL_CALL",                          /* 0x0D */
    "gcSL_RET",                           /* 0x0E */
    "gcSL_NORM",                          /* 0x0F */
    "gcSL_MAX",                           /* 0x10 */
    "gcSL_MIN",                           /* 0x11 */
    "gcSL_POW",                           /* 0x12 */
    "gcSL_RSQ",                           /* 0x13 */
    "gcSL_LOG",                           /* 0x14 */
    "gcSL_FRAC",                          /* 0x15 */
    "gcSL_FLOOR",                         /* 0x16 */
    "gcSL_CEIL",                          /* 0x17 */
    "gcSL_CROSS",                         /* 0x18 */
    "gcSL_TEXLDPROJ",                     /* 0x19 */
    "gcSL_TEXBIAS",                       /* 0x1A */
    "gcSL_TEXGRAD",                       /* 0x1B */
    "gcSL_TEXLOD",                        /* 0x1C */
    "gcSL_SIN",                           /* 0x1D */
    "gcSL_COS",                           /* 0x1E */
    "gcSL_TAN",                           /* 0x1F */
    "gcSL_EXP",                           /* 0x20 */
    "gcSL_SIGN",                          /* 0x21 */
    "gcSL_STEP",                          /* 0x22 */
    "gcSL_SQRT",                          /* 0x23 */
    "gcSL_ACOS",                          /* 0x24 */
    "gcSL_ASIN",                          /* 0x25 */
    "gcSL_ATAN",                          /* 0x26 */
    "gcSL_SET",                           /* 0x27 */
    "gcSL_DSX",                           /* 0x28 */
    "gcSL_DSY",                           /* 0x29 */
    "gcSL_FWIDTH",                        /* 0x2A */
    "gcSL_DIV",                           /* 0x2B */
    "gcSL_MOD",                           /* 0x2C */
    "gcSL_AND_BITWISE",                   /* 0x2D */
    "gcSL_OR_BITWISE",                    /* 0x2E */
    "gcSL_XOR_BITWISE",                   /* 0x2F */
    "gcSL_NOT_BITWISE",                   /* 0x30 */
    "gcSL_LSHIFT",                        /* 0x31 */
    "gcSL_RSHIFT",                        /* 0x32 */
    "gcSL_ROTATE",                        /* 0x33 */
    "gcSL_BITSEL",                        /* 0x34 */
    "gcSL_LEADZERO",                      /* 0x35 */
    "gcSL_LOAD",                          /* 0x36 */
    "gcSL_STORE",                         /* 0x37 */
    "gcSL_BARRIER",                       /* 0x38 */
    "gcSL_STORE1",                        /* 0x39 */
    "gcSL_ATOMADD",                       /* 0x3A */
    "gcSL_ATOMSUB",                       /* 0x3B */
    "gcSL_ATOMXCHG",                      /* 0x3C */
    "gcSL_ATOMCMPXCHG",                   /* 0x3D */
    "gcSL_ATOMMIN",                       /* 0x3E */
    "gcSL_ATOMMAX",                       /* 0x3F */
    "gcSL_ATOMOR",                        /* 0x40 */
    "gcSL_ATOMAND",                       /* 0x41 */
    "gcSL_ATOMXOR",                       /* 0x42 */
    "gcSL_TEXLDPCF",                      /* 0x43 */
    "gcSL_TEXLDPCFPROJ",                  /* 0x44 */
    "gcSL_TEXLODQ",                       /* 0x45  ES31 */
    "gcSL_FLUSH",                         /* 0x46  ES31 */
    "gcSL_JMP_ANY",                       /* 0x47  ES31 */
    "gcSL_BITRANGE",                      /* 0x48  ES31 */
    "gcSL_BITRANGE1",                     /* 0x49  ES31 */
    "gcSL_BITEXTRACT",                    /* 0x4A  ES31 */
    "gcSL_BITINSERT",                     /* 0x4B  ES31 */
    "gcSL_FINDLSB",                       /* 0x4C  ES31 */
    "gcSL_FINDMSB",                       /* 0x4D  ES31 */
    "gcSL_IMAGE_OFFSET",                  /* 0x4E  ES31 */
    "gcSL_IMAGE_ADDR",                    /* 0x4F  ES31 */
    "gcSL_SINPI",                         /* 0x50 */
    "gcSL_COSPI",                         /* 0x51 */
    "gcSL_TANPI",                         /* 0x52 */
    "gcSL_ADDLO",                         /* 0x53 */  /* Float only. */
    "gcSL_MULLO",                         /* 0x54 */  /* Float only. */
    "gcSL_CONV",                          /* 0x55 */
    "gcSL_GETEXP",                        /* 0x56 */
    "gcSL_GETMANT",                       /* 0x57 */
    "gcSL_MULHI",                         /* 0x58 */  /* Integer only. */
    "gcSL_CMP",                           /* 0x59 */
    "gcSL_I2F",                           /* 0x5A */
    "gcSL_F2I",                           /* 0x5B */
    "gcSL_ADDSAT",                        /* 0x5C */  /* Integer only. */
    "gcSL_SUBSAT",                        /* 0x5D */  /* Integer only. */
    "gcSL_MULSAT",                        /* 0x5E */  /* Integer only. */
    "gcSL_DP2",                           /* 0x5F */
    "gcSL_UNPACK",                        /* 0x60 */
    "gcSL_IMAGE_WR",                      /* 0x61 */
    "gcSL_SAMPLER_ADD",                   /* 0x62 */
    "gcSL_MOVA",                          /* 0x63, HW MOVAR/MOVF/MOVI, VIRCG only */
    "gcSL_IMAGE_RD",                      /* 0x64 */
    "gcSL_IMAGE_SAMPLER",                 /* 0x65 */
    "gcSL_NORM_MUL",                      /* 0x66  VIRCG only */
    "gcSL_NORM_DP2",                      /* 0x67  VIRCG only */
    "gcSL_NORM_DP3",                      /* 0x68  VIRCG only */
    "gcSL_NORM_DP4",                      /* 0x69  VIRCG only */
    "gcSL_PRE_DIV",                       /* 0x6A  VIRCG only */
    "gcSL_PRE_LOG2",                      /* 0x6B  VIRCG only */
    "gcSL_TEXGATHER",                     /* 0x6C  ES31 */
    "gcSL_TEXFETCH_MS",                   /* 0x6D  ES31 */
    "gcSL_POPCOUNT",                      /* 0x6E  ES31(OCL1.2)*/
    "gcSL_BIT_REVERSAL",                  /* 0x6F  ES31 */
    "gcSL_BYTE_REVERSAL",                 /* 0x70  ES31 */
    "gcSL_TEXPCF",                        /* 0x71  ES31 */
    "gcSL_UCARRY",                        /* 0x72  ES31 UCARRY is a condition op, while gcSL
                                                   has not enough bits to represent more */
    "gcSL_TEXU",                          /* 0x73  paired with gcSL_TEXLD to implement HW texld_u_plain */
    "gcSL_TEXU_LOD",                      /* 0x74  paired with gcSL_TEXLD to implement HW texld_u_lod */
    "gcSL_MEM_BARRIER",                   /* 0x75 */
    "gcSL_SAMPLER_ASSIGN",                /* 0x76 */
    "gcSL_GET_SAMPLER_IDX",               /* 0x77 */
    "gcSL_IMAGE_RD_3D",                   /* 0x78 */
    "gcSL_IMAGE_WR_3D",                   /* 0x79 */
    "gcSL_CLAMP0MAX",                     /* 0x7A */
    "gcSL_FMA_MUL",                       /* 0x7B FMA first part: MUL */
    "gcSL_FMA_ADD",                       /* 0x7C FMA second part: ADD */
    "gcSL_ATTR_ST",                       /* 0x7D ATTR_ST attribute(0+temp(1).x), InvocationIndex, val */
    "gcSL_ATTR_LD",                       /* 0x7E ATTR_LD dest, attribute(0+temp(1).x), InvocationIndex */
    "gcSL_EMIT_VERTEX",                   /* 0x7F For function "EmitVertex" */
    "gcSL_END_PRIMITIVE",                 /* 0x80 For function "EndPrimitive" */
    "gcSL_ARCTRIG0",                      /* 0x81 */
    "gcSL_ARCTRIG1",                      /* 0x82 */
    "gcSL_MUL_Z",                         /* 0x83 */
    "gcSL_NEG",                           /* 0x84 */
    "gcSL_LONGLO",                        /* 0x85 */
    "gcSL_LONGHI",                        /* 0x86 */
    "gcSL_MOV_LONG",                      /* 0x87 */
    "gcSL_MADSAT",                        /* 0x88 */
    "gcSL_COPY",                          /* 0x89 */
    "gcSL_LOAD_L",                        /* 0x8A */
    "gcSL_STORE_L",                       /* 0x8B */
    "gcSL_IMAGE_ADDR_3D",                 /* 0x8C */
    "gcSL_GET_SAMPLER_LMM",               /* 0x8D */
    "gcSL_GET_SAMPLER_LBS",               /* 0x8E */
    "gcSL_TEXLD_U",                       /* 0x8F */
    "gcSL_PARAM_CHAIN",                   /* 0x90 */
    "gcSL_INTRINSIC",                     /* 0x91 */
    "gcSL_INTRINSIC_ST",                  /* 0x92 */
};
char _checkOpName_size[sizeof(OpName)/sizeof(OpName[0]) == gcSL_MAXOPCODE];

gctCONST_STRING
GetOpcodeName(
    IN gcSL_OPCODE Opcode
    )
{
    if (Opcode < gcSL_MAXOPCODE)
    {
        return OpName[Opcode];
    }
    else
    {
        gcmASSERT(0);
        return "invalid opcode";
    }
}

gcSHADER_PRECISION
GetHigherPrecison(
    gcSHADER_PRECISION     precision1,
    gcSHADER_PRECISION     precision2)
{
    return precision1 > precision2 ? precision1 : precision2;
}

static gctCONST_STRING
_GetConditionName(
    IN gcSL_CONDITION Condition
    )
{
    switch (Condition)
    {
    case gcSL_ALWAYS:           return "gcSL_ALWAYS";
    case gcSL_NOT_EQUAL:        return "gcSL_NOT_EQUAL";
    case gcSL_LESS_OR_EQUAL:    return "gcSL_LESS_OR_EQUAL";
    case gcSL_LESS:             return "gcSL_LESS";
    case gcSL_EQUAL:            return "gcSL_EQUAL";
    case gcSL_GREATER:          return "gcSL_GREATER";
    case gcSL_GREATER_OR_EQUAL: return "gcSL_GREATER_OR_EQUAL";
    case gcSL_ZERO:             return "gcSL_ZERO";
    case gcSL_NOT_ZERO:         return "gcSL_NOT_ZERO";
    case gcSL_AND:              return "gcSL_AND";
    case gcSL_OR:               return "gcSL_OR";
    case gcSL_XOR:              return "gcSL_XOR";

    default:
    gcmASSERT(0);
    return "Invalid";
    }
}

static gctCONST_STRING
_GetEnableName(
    IN gctUINT8 Enable,
    OUT gctCHAR buf[5]
    )
{
    gctINT i = 0;

    if (Enable & gcSL_ENABLE_X) buf[i++] = 'X';
    if (Enable & gcSL_ENABLE_Y) buf[i++] = 'Y';
    if (Enable & gcSL_ENABLE_Z) buf[i++] = 'Z';
    if (Enable & gcSL_ENABLE_W) buf[i++] = 'W';

    gcmASSERT(i > 0);
    buf[i] = '\0';

    return buf;
}

static gctCHAR
_GetSwizzleChar(
    IN gctUINT8 Swizzle
    )
{
    switch (Swizzle)
    {
    case gcSL_SWIZZLE_X: return 'X';
    case gcSL_SWIZZLE_Y: return 'Y';
    case gcSL_SWIZZLE_Z: return 'Z';
    case gcSL_SWIZZLE_W: return 'W';

    default:
        gcmASSERT(0);
        return 'X';
    }
}

static gctCONST_STRING
_GetSwizzleName(
    IN gctUINT8 Swizzle,
    OUT gctCHAR buf[5]
    )
{
    buf[0] = _GetSwizzleChar((Swizzle >> 0) & 3);
    buf[1] = _GetSwizzleChar((Swizzle >> 2) & 3);
    buf[2] = _GetSwizzleChar((Swizzle >> 4) & 3);
    buf[3] = _GetSwizzleChar((Swizzle >> 6) & 3);
    buf[4] = '\0';

    return buf;
}

static gctUINT8
_ConvertEnable2Swizzle(
    IN gctUINT32 Enable
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

    case gcSL_ENABLE_XY:
        return gcSL_SWIZZLE_XYYY;

    case gcSL_ENABLE_XZ:
        return gcSL_SWIZZLE_XZZZ;

    case gcSL_ENABLE_XW:
        return gcSL_SWIZZLE_XWWW;

    case gcSL_ENABLE_YZ:
        return gcSL_SWIZZLE_YZZZ;

    case gcSL_ENABLE_YW:
        return gcSL_SWIZZLE_YWWW;

    case gcSL_ENABLE_ZW:
        return gcSL_SWIZZLE_ZWWW;

    case gcSL_ENABLE_XYZ:
        return gcSL_SWIZZLE_XYZZ;

    case gcSL_ENABLE_XYW:
        return gcSL_SWIZZLE_XYWW;

    case gcSL_ENABLE_XZW:
        return gcSL_SWIZZLE_XZWW;

    case gcSL_ENABLE_YZW:
        return gcSL_SWIZZLE_YZWW;

    case gcSL_ENABLE_XYZW:
        return gcSL_SWIZZLE_XYZW;
    }

    gcmFATAL("ERROR: Invalid enable 0x%04X", Enable);
    return gcSL_SWIZZLE_XYZW;
}

static gctCONST_STRING
_GetIndexModeName(
    IN gcSL_INDEXED IndexMode
    )
{
    switch (IndexMode)
    {
    case gcSL_NOT_INDEXED:  return "gcSL_NOT_INDEXED";
    case gcSL_INDEXED_X:    return "gcSL_INDEXED_X";
    case gcSL_INDEXED_Y:    return "gcSL_INDEXED_Y";
    case gcSL_INDEXED_Z:    return "gcSL_INDEXED_Z";
    case gcSL_INDEXED_W:    return "gcSL_INDEXED_W";

    default:
        gcmASSERT(0);
        return "Invalid";
    }
}

static gctCONST_STRING
_GetTypeName(
    IN gcSL_TYPE Type
    )
{
    switch (Type)
    {
    case gcSL_NONE:        return "gcSL_NONE";
    case gcSL_TEMP:        return "gcSL_TEMP";
    case gcSL_ATTRIBUTE:   return "gcSL_ATTRIBUTE";
    case gcSL_UNIFORM:       return "gcSL_UNIFORM";
    case gcSL_SAMPLER:       return "gcSL_SAMPLER";
    case gcSL_CONSTANT:       return "gcSL_CONSTANT";

    default:
        gcmASSERT(0);
        return "Invalid";
    }
}

gctCONST_STRING
gcGetDataTypeName(
    IN gcSHADER_TYPE DataType
    )
{
    switch (DataType)
    {
    case gcSHADER_FLOAT_X1:            return "gcSHADER_FLOAT_X1";
    case gcSHADER_FLOAT_X2:            return "gcSHADER_FLOAT_X2";
    case gcSHADER_FLOAT_X3:            return "gcSHADER_FLOAT_X3";
    case gcSHADER_FLOAT_X4:            return "gcSHADER_FLOAT_X4";
    case gcSHADER_FLOAT_2X2:        return "gcSHADER_FLOAT_2X2";
    case gcSHADER_FLOAT_2X3:        return "gcSHADER_FLOAT_2X3";
    case gcSHADER_FLOAT_2X4:        return "gcSHADER_FLOAT_2X4";
    case gcSHADER_FLOAT_3X2:        return "gcSHADER_FLOAT_3X2";
    case gcSHADER_FLOAT_3X3:        return "gcSHADER_FLOAT_3X3";
    case gcSHADER_FLOAT_3X4:        return "gcSHADER_FLOAT_3X4";
    case gcSHADER_FLOAT_4X2:        return "gcSHADER_FLOAT_4X2";
    case gcSHADER_FLOAT_4X3:        return "gcSHADER_FLOAT_4X3";
    case gcSHADER_FLOAT_4X4:        return "gcSHADER_FLOAT_4X4";
    case gcSHADER_BOOLEAN_X1:        return "gcSHADER_BOOLEAN_X1";
    case gcSHADER_BOOLEAN_X2:        return "gcSHADER_BOOLEAN_X2";
    case gcSHADER_BOOLEAN_X3:        return "gcSHADER_BOOLEAN_X3";
    case gcSHADER_BOOLEAN_X4:        return "gcSHADER_BOOLEAN_X4";
    case gcSHADER_INTEGER_X1:        return "gcSHADER_INTEGER_X1";
    case gcSHADER_INTEGER_X2:        return "gcSHADER_INTEGER_X2";
    case gcSHADER_INTEGER_X3:        return "gcSHADER_INTEGER_X3";
    case gcSHADER_INTEGER_X4:        return "gcSHADER_INTEGER_X4";
    case gcSHADER_UINT_X1:            return "gcSHADER_UINT_X1";
    case gcSHADER_UINT_X2:            return "gcSHADER_UINT_X2";
    case gcSHADER_UINT_X3:            return "gcSHADER_UINT_X3";
    case gcSHADER_UINT_X4:            return "gcSHADER_UINT_X4";
    case gcSHADER_SAMPLER_1D:        return "gcSHADER_SAMPLER_1D";
    case gcSHADER_SAMPLER_2D:        return "gcSHADER_SAMPLER_2D";
    case gcSHADER_SAMPLER_3D:        return "gcSHADER_SAMPLER_3D";
    case gcSHADER_SAMPLER_CUBIC:        return "gcSHADER_SAMPLER_CUBIC";
    case gcSHADER_SAMPLER_CUBEMAP_ARRAY:   return "gcSHADER_SAMPLER_CUBEMAP_ARRAY";
    case gcSHADER_SAMPLER_EXTERNAL_OES:    return "gcSHADER_SAMPLER_EXTERNAL_OES";
    case gcSHADER_SAMPLER_2D_SHADOW:    return "gcSHADER_SAMPLER_2D_SHADOW";
    case gcSHADER_SAMPLER_CUBE_SHADOW:    return "gcSHADER_SAMPLER_CUBE_SHADOW";
    case gcSHADER_SAMPLER_CUBEMAP_ARRAY_SHADOW:    return "gcSHADER_SAMPLER_CUBEMAP_ARRAY_SHADOW";

    case gcSHADER_SAMPLER_1D_ARRAY:         return "gcSHADER_SAMPLER_1D_ARRAY";
    case gcSHADER_SAMPLER_1D_ARRAY_SHADOW:  return "gcSHADER_SAMPLER_1D_ARRAY_SHADOW";
    case gcSHADER_SAMPLER_2D_ARRAY:         return "gcSHADER_SAMPLER_2D_ARRAY";
    case gcSHADER_SAMPLER_2D_ARRAY_SHADOW:  return "gcSHADER_SAMPLER_2D_ARRAY_SHADOW";

    case gcSHADER_ISAMPLER_2D:              return "gcSHADER_ISAMPLER_2D";
    case gcSHADER_ISAMPLER_3D:              return "gcSHADER_ISAMPLER_3D";
    case gcSHADER_ISAMPLER_CUBIC:           return "gcSHADER_ISAMPLER_CUBIC";
    case gcSHADER_ISAMPLER_CUBEMAP_ARRAY:   return "gcSHADER_ISAMPLER_CUBEMAP_ARRAY";
    case gcSHADER_ISAMPLER_2D_ARRAY:        return "gcSHADER_ISAMPLER_2D_ARRAY";

    case gcSHADER_USAMPLER_2D:              return "gcSHADER_USAMPLER_2D";
    case gcSHADER_USAMPLER_3D:              return "gcSHADER_USAMPLER_3D";
    case gcSHADER_USAMPLER_CUBIC:           return "gcSHADER_USAMPLER_CUBIC";
    case gcSHADER_USAMPLER_CUBEMAP_ARRAY:   return "gcSHADER_USAMPLER_CUBEMAP_ARRAY";
    case gcSHADER_USAMPLER_2D_ARRAY:        return "gcSHADER_USAMPLER_2D_ARRAY";


    case gcSHADER_SAMPLER_2D_MS:            return "gcSHADER_SAMPLER_2D_MS";
    case gcSHADER_ISAMPLER_2D_MS:           return "gcSHADER_ISAMPLER_2D_MS";
    case gcSHADER_USAMPLER_2D_MS:           return "gcSHADER_USAMPLER_2D_MS";
    case gcSHADER_SAMPLER_2D_MS_ARRAY:      return "gcSHADER_SAMPLER_2D_MS_ARRAY";
    case gcSHADER_ISAMPLER_2D_MS_ARRAY:     return "gcSHADER_ISAMPLER_2D_MS_ARRAY";
    case gcSHADER_USAMPLER_2D_MS_ARRAY:     return "gcSHADER_USAMPLER_2D_MS_ARRAY";

    case gcSHADER_IMAGE_2D:                 return "gcSHADER_IMAGE_2D";
    case gcSHADER_IMAGE_3D:                 return "gcSHADER_IMAGE_3D";
    case gcSHADER_IIMAGE_2D:                return "gcSHADER_IIMAGE_2D";
    case gcSHADER_UIMAGE_2D:                return "gcSHADER_UIMAGE_2D";
    case gcSHADER_IIMAGE_3D:                return "gcSHADER_IIMAGE_3D";
    case gcSHADER_UIMAGE_3D:                return "gcSHADER_UIMAGE_3D";
    case gcSHADER_IMAGE_CUBE:               return "gcSHADER_IMAGE_CUBE";
    case gcSHADER_IMAGE_CUBEMAP_ARRAY:      return "gcSHADER_IMAGE_CUBEMAP_ARRAY";
    case gcSHADER_IIMAGE_CUBE:              return "gcSHADER_IIMAGE_CUBE";
    case gcSHADER_IIMAGE_CUBEMAP_ARRAY:     return "gcSHADER_IIMAGE_CUBEMAP_ARRAY";
    case gcSHADER_UIMAGE_CUBE:              return "gcSHADER_UIMAGE_CUBE";
    case gcSHADER_UIMAGE_CUBEMAP_ARRAY:     return "gcSHADER_UIMAGE_CUBEMAP_ARRAY";
    case gcSHADER_IMAGE_2D_ARRAY:           return "gcSHADER_IMAGE_2D_ARRAY";
    case gcSHADER_IIMAGE_2D_ARRAY:          return "gcSHADER_IIMAGE_2D_ARRAY";
    case gcSHADER_UIMAGE_2D_ARRAY:          return "gcSHADER_UIMAGE_2D_ARRAY";
    case gcSHADER_SAMPLER_BUFFER:           return "gcSHADER_SAMPLER_BUFFER";
    case gcSHADER_ISAMPLER_BUFFER:          return "gcSHADER_ISAMPLER_BUFFER";
    case gcSHADER_USAMPLER_BUFFER:          return "gcSHADER_USAMPLER_BUFFER";
    case gcSHADER_IMAGE_BUFFER:             return "gcSHADER_IMAGE_BUFFER";
    case gcSHADER_IIMAGE_BUFFER:            return "gcSHADER_IIMAGE_BUFFER";
    case gcSHADER_UIMAGE_BUFFER:            return "gcSHADER_UIMAGE_BUFFER";
    case gcSHADER_ATOMIC_UINT:              return "gcSHADER_ATOMIC_UINT";
    default:
        gcmASSERT(0);
        return "Invalid";
    }
}

static gctCONST_STRING
_GetFormatName(
    IN gcSL_FORMAT Format
    )
{
    switch (Format)
    {
    case gcSL_FLOAT:    return "gcSL_FLOAT";
    case gcSL_INTEGER:    return "gcSL_INTEGER";
    case gcSL_BOOLEAN:    return "gcSL_BOOLEAN";
    case gcSL_UINT32:    return "gcSL_UINT32";
    case gcSL_INT16:    return "gcSL_INT16";
    case gcSL_UINT16:    return "gcSL_UINT16";
    case gcSL_FLOAT16:    return "gcSL_FLOAT16";
    case gcSL_INT8:       return "gcSL_INT8";
    case gcSL_UINT8:       return "gcSL_UINT8";

    default:
        gcmASSERT(0);
        return "Invalid";
    }
}

static gctCONST_STRING
_GetQualifierName(
    IN gceINPUT_OUTPUT Qualifier
    )
{
    switch (Qualifier)
    {
    case gcvFUNCTION_INPUT: return "gcvFUNCTION_INPUT";
    case gcvFUNCTION_OUTPUT: return "gcvFUNCTION_OUTPUT";
    case gcvFUNCTION_INOUT: return "gcvFUNCTION_INOUT";

    default:
        gcmASSERT(0);
        return "Invalid";
    }
}

gctBOOL
gcIsSamplerDataType(
    IN gcSHADER_TYPE DataType
    )
{
    switch (DataType)
    {
    case gcSHADER_SAMPLER_1D:
    case gcSHADER_SAMPLER_2D:
    case gcSHADER_SAMPLER_3D:
    case gcSHADER_SAMPLER_BUFFER:
    case gcSHADER_SAMPLER_CUBIC:
    case gcSHADER_SAMPLER_CUBEMAP_ARRAY:
    case gcSHADER_ISAMPLER_2D:
    case gcSHADER_ISAMPLER_3D:
    case gcSHADER_ISAMPLER_BUFFER:
    case gcSHADER_ISAMPLER_CUBIC:
    case gcSHADER_ISAMPLER_CUBEMAP_ARRAY:
    case gcSHADER_USAMPLER_2D:
    case gcSHADER_USAMPLER_3D:
    case gcSHADER_USAMPLER_BUFFER:
    case gcSHADER_USAMPLER_CUBIC:
    case gcSHADER_USAMPLER_CUBEMAP_ARRAY:
    case gcSHADER_SAMPLER_EXTERNAL_OES:
    case gcSHADER_SAMPLER_2D_SHADOW:
    case gcSHADER_SAMPLER_CUBE_SHADOW:
    case gcSHADER_SAMPLER_CUBEMAP_ARRAY_SHADOW:

    case gcSHADER_ISAMPLER_2D_ARRAY:
    case gcSHADER_USAMPLER_2D_ARRAY:

    case gcSHADER_SAMPLER_1D_ARRAY:
    case gcSHADER_SAMPLER_1D_ARRAY_SHADOW:
    case gcSHADER_SAMPLER_2D_ARRAY:
    case gcSHADER_SAMPLER_2D_ARRAY_SHADOW:

    case gcSHADER_SAMPLER_2D_MS:
    case gcSHADER_ISAMPLER_2D_MS:
    case gcSHADER_USAMPLER_2D_MS:
    case gcSHADER_SAMPLER_2D_MS_ARRAY:
    case gcSHADER_ISAMPLER_2D_MS_ARRAY:
    case gcSHADER_USAMPLER_2D_MS_ARRAY:
        return gcvTRUE;

    default:
        return gcvFALSE;
    }
}

gctBOOL
gcIsImageDataType(
    IN gcSHADER_TYPE DataType
    )
{
    switch (DataType)
    {
    case gcSHADER_IMAGE_2D:                      /* 0x17 */
    case gcSHADER_IIMAGE_2D:                     /* 0x39 */
    case gcSHADER_UIMAGE_2D:                     /* 0x3A */
    case gcSHADER_IMAGE_3D:                      /* 0x18 */
    case gcSHADER_IIMAGE_3D:                     /* 0x3B */
    case gcSHADER_UIMAGE_3D:                     /* 0x3C */
    case gcSHADER_IMAGE_CUBE:                    /* 0x3D */
    case gcSHADER_IIMAGE_CUBE:                   /* 0x3E */
    case gcSHADER_UIMAGE_CUBE:                   /* 0x3F */
    case gcSHADER_IMAGE_2D_ARRAY:                /* 0x40 */
    case gcSHADER_IIMAGE_2D_ARRAY:               /* 0x41 */
    case gcSHADER_UIMAGE_2D_ARRAY:               /* 0x42 */
    case gcSHADER_IMAGE_CUBEMAP_ARRAY:           /* 0x51 */
    case gcSHADER_IIMAGE_CUBEMAP_ARRAY:          /* 0x52 */
    case gcSHADER_UIMAGE_CUBEMAP_ARRAY:          /* 0x53 */
    case gcSHADER_IMAGE_BUFFER:
    case gcSHADER_IIMAGE_BUFFER:
    case gcSHADER_UIMAGE_BUFFER:
        return gcvTRUE;

    default:
        return gcvFALSE;
    }
}

static
void _DumpLastIR(gcSHADER Shader)
{
    gctUINT pc;
    gcSL_INSTRUCTION code;

    if (Shader->instrIndex == gcSHADER_OPCODE)
    {
        pc = Shader->lastInstruction - 1;
    }
    else
    {
        pc = Shader->lastInstruction;
    }
    code = Shader->code + pc;

    dbg_dumpIR(code,pc);
}

gctBOOL
gcIsAtomicDataType(
    IN gcSHADER_TYPE DataType
    )
{
    switch (DataType)
    {
    case gcSHADER_ATOMIC_UINT:
        return gcvTRUE;

    default:
        return gcvFALSE;
    }
}

gctBOOL
gcIsSamplerArrayDataType(
    IN gcSHADER_TYPE DataType
    )
{
    switch (DataType)
    {
    case gcSHADER_ISAMPLER_2D_ARRAY:
    case gcSHADER_USAMPLER_2D_ARRAY:

    case gcSHADER_SAMPLER_1D_ARRAY:
    case gcSHADER_SAMPLER_1D_ARRAY_SHADOW:
    case gcSHADER_SAMPLER_2D_ARRAY:
    case gcSHADER_SAMPLER_2D_ARRAY_SHADOW:

    case gcSHADER_SAMPLER_2D_MS_ARRAY:
    case gcSHADER_ISAMPLER_2D_MS_ARRAY:
    case gcSHADER_USAMPLER_2D_MS_ARRAY:
        return gcvTRUE;

    default:
        return gcvFALSE;
    }
}

gctBOOL
gcIsImageArrayDataType(
    IN gcSHADER_TYPE DataType
    )
{
    switch (DataType)
    {
    case gcSHADER_IMAGE_2D_ARRAY:                /* 0x40 */
    case gcSHADER_IIMAGE_2D_ARRAY:               /* 0x41 */
    case gcSHADER_UIMAGE_2D_ARRAY:               /* 0x42 */
        return gcvTRUE;

    default:
        return gcvFALSE;
    }
}

gctBOOL
gcIsScalarDataType(
    IN gcSHADER_TYPE DataType
    )
{
    switch (DataType)
    {
    case gcSHADER_FLOAT_X1:
    case gcSHADER_BOOLEAN_X1:
    case gcSHADER_INTEGER_X1:
    case gcSHADER_UINT_X1:
    case gcSHADER_ATOMIC_UINT:
        return gcvTRUE;

    default:
        return gcvFALSE;
    }
}

gctBOOL
gcIsVectorDataType(
    IN gcSHADER_TYPE DataType
    )
{
    switch (DataType)
    {
    case gcSHADER_FLOAT_X2:
    case gcSHADER_FLOAT_X3:
    case gcSHADER_FLOAT_X4:
    case gcSHADER_BOOLEAN_X2:
    case gcSHADER_BOOLEAN_X3:
    case gcSHADER_BOOLEAN_X4:
    case gcSHADER_INTEGER_X2:
    case gcSHADER_INTEGER_X3:
    case gcSHADER_INTEGER_X4:
    case gcSHADER_UINT_X2:
    case gcSHADER_UINT_X3:
    case gcSHADER_UINT_X4:
        return gcvTRUE;

    default:
        return gcvFALSE;
    }
}

gctBOOL
gcIsMatrixDataType(
    IN gcSHADER_TYPE DataType
    )
{
    switch (DataType)
    {
    case gcSHADER_FLOAT_2X2:
    case gcSHADER_FLOAT_2X3:
    case gcSHADER_FLOAT_2X4:
    case gcSHADER_FLOAT_3X2:
    case gcSHADER_FLOAT_3X3:
    case gcSHADER_FLOAT_3X4:
    case gcSHADER_FLOAT_4X2:
    case gcSHADER_FLOAT_4X3:
    case gcSHADER_FLOAT_4X4:
        return gcvTRUE;

    default:
        return gcvFALSE;
    }
}

gctBOOL
gcIsSymmetricalMatrixDataType(
    IN gcSHADER_TYPE DataType
    )
{
    switch (DataType)
    {
    case gcSHADER_FLOAT_2X2:
    case gcSHADER_FLOAT_3X3:
    case gcSHADER_FLOAT_4X4:
        return gcvTRUE;

    default:
        return gcvFALSE;
    }
}

gctBOOL
gcIsFloatDataType(
    IN gcSHADER_TYPE DataType
    )
{
    if(gcIsMatrixDataType(DataType))
    {
         return gcvTRUE;
    }
    else
    {
        switch (DataType)
        {
        case gcSHADER_FLOAT_X1:
        case gcSHADER_FLOAT_X2:
        case gcSHADER_FLOAT_X3:
        case gcSHADER_FLOAT_X4:
             return gcvTRUE;

        default:
             return gcvFALSE;
        }
    }
}

gctBOOL
gcIsIntegerDataType(
    IN gcSHADER_TYPE DataType
    )
{
    switch (DataType)
    {
    case gcSHADER_BOOLEAN_X1:
    case gcSHADER_BOOLEAN_X2:
    case gcSHADER_BOOLEAN_X3:
    case gcSHADER_BOOLEAN_X4:
    case gcSHADER_INTEGER_X1:
    case gcSHADER_INTEGER_X2:
    case gcSHADER_INTEGER_X3:
    case gcSHADER_INTEGER_X4:
    case gcSHADER_UINT_X1:
    case gcSHADER_UINT_X2:
    case gcSHADER_UINT_X3:
    case gcSHADER_UINT_X4:
    case gcSHADER_ATOMIC_UINT:
        return gcvTRUE;

    default:
        return gcvFALSE;
    }
}

gctBOOL
gcIsUnsignedIntegerDataType(
    IN gcSHADER_TYPE DataType
    )
{
    switch (DataType)
    {
    case gcSHADER_BOOLEAN_X1:
    case gcSHADER_BOOLEAN_X2:
    case gcSHADER_BOOLEAN_X3:
    case gcSHADER_BOOLEAN_X4:
    case gcSHADER_UINT_X1:
    case gcSHADER_UINT_X2:
    case gcSHADER_UINT_X3:
    case gcSHADER_UINT_X4:
    case gcSHADER_ATOMIC_UINT:
        return gcvTRUE;

    default:
        return gcvFALSE;
    }
}

gcSHADER_TYPE
gcGetVectorComponentDataType(
    IN gcSHADER_TYPE DataType
    )
{
    switch (DataType)
    {
    case gcSHADER_FLOAT_X2:
    case gcSHADER_FLOAT_X3:
    case gcSHADER_FLOAT_X4:
        return gcSHADER_FLOAT_X1;

    case gcSHADER_BOOLEAN_X2:
    case gcSHADER_BOOLEAN_X3:
    case gcSHADER_BOOLEAN_X4:
        return gcSHADER_BOOLEAN_X1;

    case gcSHADER_INTEGER_X2:
    case gcSHADER_INTEGER_X3:
    case gcSHADER_INTEGER_X4:
        return gcSHADER_INTEGER_X1;

    case gcSHADER_UINT_X2:
    case gcSHADER_UINT_X3:
    case gcSHADER_UINT_X4:
        return gcSHADER_UINT_X1;

    default:
        gcmASSERT(0);
        return gcSHADER_FLOAT_X1;
    }
}

gcSHADER_TYPE
gcGetVectorSliceDataType(
    IN gcSHADER_TYPE DataType,
    IN gctUINT8 Components
    )
{
    switch (DataType)
    {
    case gcSHADER_FLOAT_X2:
    case gcSHADER_FLOAT_X3:
    case gcSHADER_FLOAT_X4:
        switch (Components)
        {
        case 1: return gcSHADER_FLOAT_X1;
        case 2: return gcSHADER_FLOAT_X2;
        case 3: return gcSHADER_FLOAT_X3;
        case 4: return gcSHADER_FLOAT_X4;

        default:
            gcmASSERT(0);
            return gcSHADER_FLOAT_X1;
        }

    case gcSHADER_BOOLEAN_X2:
    case gcSHADER_BOOLEAN_X3:
    case gcSHADER_BOOLEAN_X4:
        switch (Components)
        {
        case 1: return gcSHADER_BOOLEAN_X1;
        case 2: return gcSHADER_BOOLEAN_X2;
        case 3: return gcSHADER_BOOLEAN_X3;
        case 4: return gcSHADER_BOOLEAN_X4;

        default:
            gcmASSERT(0);
            return gcSHADER_BOOLEAN_X1;
        }

    case gcSHADER_INTEGER_X2:
    case gcSHADER_INTEGER_X3:
    case gcSHADER_INTEGER_X4:
        switch (Components)
        {
        case 1: return gcSHADER_INTEGER_X1;
        case 2: return gcSHADER_INTEGER_X2;
        case 3: return gcSHADER_INTEGER_X3;
        case 4: return gcSHADER_INTEGER_X4;

        default:
            gcmASSERT(0);
            return gcSHADER_INTEGER_X1;
        }

    case gcSHADER_UINT_X2:
    case gcSHADER_UINT_X3:
    case gcSHADER_UINT_X4:
        switch (Components)
        {
        case 1: return gcSHADER_UINT_X1;
        case 2: return gcSHADER_UINT_X2;
        case 3: return gcSHADER_UINT_X3;
        case 4: return gcSHADER_UINT_X4;

        default:
            gcmASSERT(0);
            return gcSHADER_UINT_X1;
        }

    default:
        gcmASSERT(0);
        return DataType;
    }
}

gcSHADER_TYPE
gcConvScalarToVectorDataType(
    IN gcSHADER_TYPE DataType,
    IN gctUINT8 Components
    )
{
    switch (DataType)
    {
    case gcSHADER_FLOAT_X1:
        switch (Components)
        {
        case 1: return gcSHADER_FLOAT_X1;
        case 2: return gcSHADER_FLOAT_X2;
        case 3: return gcSHADER_FLOAT_X3;
        case 4: return gcSHADER_FLOAT_X4;

        default:
            gcmASSERT(0);
            return gcSHADER_FLOAT_X4;
        }

    case gcSHADER_BOOLEAN_X1:
        switch (Components)
        {
        case 1: return gcSHADER_BOOLEAN_X1;
        case 2: return gcSHADER_BOOLEAN_X2;
        case 3: return gcSHADER_BOOLEAN_X3;
        case 4: return gcSHADER_BOOLEAN_X4;

        default:
            gcmASSERT(0);
            return gcSHADER_BOOLEAN_X4;
        }

    case gcSHADER_INTEGER_X1:
        switch (Components)
        {
        case 1: return gcSHADER_INTEGER_X1;
        case 2: return gcSHADER_INTEGER_X2;
        case 3: return gcSHADER_INTEGER_X3;
        case 4: return gcSHADER_INTEGER_X4;

        default:
            gcmASSERT(0);
            return gcSHADER_INTEGER_X4;
        }

    case gcSHADER_UINT_X1:
        switch (Components)
        {
        case 1: return gcSHADER_UINT_X1;
        case 2: return gcSHADER_UINT_X2;
        case 3: return gcSHADER_UINT_X3;
        case 4: return gcSHADER_UINT_X4;

        default:
            gcmASSERT(0);
            return gcSHADER_UINT_X4;
        }

    default:
        gcmASSERT(0);
        return gcSHADER_FLOAT_X4;
    }
}

gcSHADER_TYPE
gcChangeElementDataType(
    IN gcSHADER_TYPE BaseDataType,
    IN gcSHADER_TYPE ToElementDataType
    )
{
    if(gcIsMatrixDataType(BaseDataType) ||
       gcIsSamplerDataType(BaseDataType))
    {
       return ToElementDataType;
    }

    return gcConvScalarToVectorDataType(ToElementDataType,
                                        gcGetDataTypeComponentCount(BaseDataType));
}

gctUINT8
gcGetVectorDataTypeComponentCount(
    IN gcSHADER_TYPE DataType
    )
{
    switch (DataType)
    {
    case gcSHADER_FLOAT_X1:
    case gcSHADER_BOOLEAN_X1:
    case gcSHADER_INTEGER_X1:
    case gcSHADER_UINT_X1:
    case gcSHADER_ATOMIC_UINT:
        return 1;

    case gcSHADER_FLOAT_X2:
    case gcSHADER_BOOLEAN_X2:
    case gcSHADER_INTEGER_X2:
    case gcSHADER_UINT_X2:
        return 2;

    case gcSHADER_FLOAT_X3:
    case gcSHADER_BOOLEAN_X3:
    case gcSHADER_INTEGER_X3:
    case gcSHADER_UINT_X3:
        return 3;

    case gcSHADER_FLOAT_X4:
    case gcSHADER_BOOLEAN_X4:
    case gcSHADER_INTEGER_X4:
    case gcSHADER_UINT_X4:
        return 4;

    default:
        gcmASSERT(0);
        return 4;
    }
}

gcSHADER_TYPE
gcGetVectorComponentSelectionDataType(
    IN gcSHADER_TYPE DataType,
    IN gctUINT8    Components
    )
{
    switch (DataType)
    {
    case gcSHADER_FLOAT_X2:
    case gcSHADER_FLOAT_X3:
    case gcSHADER_FLOAT_X4:
        switch (Components)
        {
        case 1: return gcSHADER_FLOAT_X1;
        case 2: return gcSHADER_FLOAT_X2;
        case 3: return gcSHADER_FLOAT_X3;
        case 4: return gcSHADER_FLOAT_X4;

        default:
            gcmASSERT(0);
            return gcSHADER_FLOAT_X1;
        }

    case gcSHADER_BOOLEAN_X2:
    case gcSHADER_BOOLEAN_X3:
    case gcSHADER_BOOLEAN_X4:
        switch (Components)
        {
        case 1: return gcSHADER_BOOLEAN_X1;
        case 2: return gcSHADER_BOOLEAN_X2;
        case 3: return gcSHADER_BOOLEAN_X3;
        case 4: return gcSHADER_BOOLEAN_X4;

        default:
            gcmASSERT(0);
            return gcSHADER_BOOLEAN_X1;
        }

    case gcSHADER_INTEGER_X2:
    case gcSHADER_INTEGER_X3:
    case gcSHADER_INTEGER_X4:
        switch (Components)
        {
        case 1: return gcSHADER_INTEGER_X1;
        case 2: return gcSHADER_INTEGER_X2;
        case 3: return gcSHADER_INTEGER_X3;
        case 4: return gcSHADER_INTEGER_X4;

        default:
            gcmASSERT(0);
            return gcSHADER_INTEGER_X1;
        }

    case gcSHADER_UINT_X2:
    case gcSHADER_UINT_X3:
    case gcSHADER_UINT_X4:
        switch (Components)
        {
        case 1: return gcSHADER_UINT_X1;
        case 2: return gcSHADER_UINT_X2;
        case 3: return gcSHADER_UINT_X3;
        case 4: return gcSHADER_UINT_X4;

        default:
            gcmASSERT(0);
            return gcSHADER_UINT_X1;
        }

    default:
        gcmASSERT(0);
        return gcSHADER_FLOAT_X1;
    }
}

gcSHADER_TYPE
gcGetMatrixColumnDataType(
    IN gcSHADER_TYPE DataType
    )
{
    switch (DataType)
    {
    case gcSHADER_FLOAT_2X2:
        return gcSHADER_FLOAT_X2;

    case gcSHADER_FLOAT_2X3:
        return gcSHADER_FLOAT_X3;

    case gcSHADER_FLOAT_2X4:
        return gcSHADER_FLOAT_X4;

    case gcSHADER_FLOAT_3X3:
        return gcSHADER_FLOAT_X3;

    case gcSHADER_FLOAT_3X2:
        return gcSHADER_FLOAT_X2;

    case gcSHADER_FLOAT_3X4:
        return gcSHADER_FLOAT_X4;

    case gcSHADER_FLOAT_4X4:
        return gcSHADER_FLOAT_X4;

    case gcSHADER_FLOAT_4X2:
        return gcSHADER_FLOAT_X2;

    case gcSHADER_FLOAT_4X3:
        return gcSHADER_FLOAT_X3;

    default:
        gcmASSERT(0);
        return gcSHADER_FLOAT_X4;
    }
}

gctUINT
gcGetMatrixDataTypeColumnCount(
    IN gcSHADER_TYPE DataType
    )
{
    switch (DataType)
    {
    case gcSHADER_FLOAT_2X2:
    case gcSHADER_FLOAT_2X3:
    case gcSHADER_FLOAT_2X4:
        return 2;

    case gcSHADER_FLOAT_3X2:
    case gcSHADER_FLOAT_3X3:
    case gcSHADER_FLOAT_3X4:
        return 3;

    case gcSHADER_FLOAT_4X2:
    case gcSHADER_FLOAT_4X3:
    case gcSHADER_FLOAT_4X4:
        return 4;

    case gcSHADER_FLOAT_X2:
    case gcSHADER_FLOAT_X3:
    case gcSHADER_FLOAT_X4:
        return 1;

    default:
        gcmASSERT(0);
        return 4;
    }
}

gctUINT
gcGetMatrixDataTypeRowCount(
    IN gcSHADER_TYPE DataType
    )
{
    switch (DataType)
    {
    case gcSHADER_FLOAT_X2:
    case gcSHADER_FLOAT_2X2:
    case gcSHADER_FLOAT_3X2:
    case gcSHADER_FLOAT_4X2:
        return 2;

    case gcSHADER_FLOAT_X3:
    case gcSHADER_FLOAT_2X3:
    case gcSHADER_FLOAT_3X3:
    case gcSHADER_FLOAT_4X3:
        return 3;

    case gcSHADER_FLOAT_X4:
    case gcSHADER_FLOAT_4X4:
    case gcSHADER_FLOAT_2X4:
    case gcSHADER_FLOAT_3X4:
        return 4;

    default:
        gcmASSERT(0);
        return 4;
    }
}

static gceSTATUS
_AddAttributeWithLocation(
    IN sloCOMPILER Compiler,
    IN gctCONST_STRING Name,
    IN gcSHADER_TYPE Type,
    IN gcSHADER_PRECISION Precision,
    IN gctUINT Length,
    IN gctUINT ArrayLengthCount,
    IN gctBOOL IsTexture,
    IN gcSHADER_SHADERMODE ShaderMode,
    IN gctINT Location,
    IN gctINT FieldIndex,
    IN gctBOOL IsInvariant,
    IN gctBOOL IsPrecise,
    OUT gcATTRIBUTE * Attribute
    )
{
    gcSHADER    binary;
    gceSTATUS   status;

    gcmHEADER();

    gcmASSERT(Attribute);

    gcmVERIFY_OK(sloCOMPILER_GetBinary(Compiler, &binary));

    if (Compiler->context.dumpOptions & slvDUMP_CODE_EMITTER)
    {
        gcmVERIFY_OK(sloCOMPILER_Dump(Compiler,
                                      slvDUMP_CODE_EMITTER,
                                      "gcSHADER_AddAttributeWithLocation(Shader, \"%s\", %s, %d, %s);",
                                      Name,
                                      gcGetDataTypeName(Type),
                                      Length,
                                      (IsTexture)? "true" : "false"));
    }

    status = gcSHADER_AddAttributeWithLocation(binary,
                                               Name,
                                               Type,
                                               Precision,
                                               Length,
                                               ArrayLengthCount,
                                               IsTexture,
                                               ShaderMode,
                                               Location,
                                               FieldIndex,
                                               IsInvariant,
                                               IsPrecise,
                                               Attribute);
    if (gcmIS_ERROR(status)) {
       gcmFOOTER();
       return status;
    }

    gcmFOOTER();
    return status;
}

static gceSTATUS
_AddUniform(
    IN sloCOMPILER Compiler,
    IN gctCONST_STRING Name,
    IN gcsSHADER_VAR_INFO *UniformInfo,
    OUT gctINT16* ThisUniformIndex,
    OUT gcUNIFORM * Uniform
    )
{
    gcSHADER    binary;
    gceSTATUS   status;

    gcmHEADER();

    gcmASSERT(Uniform);

    gcmVERIFY_OK(sloCOMPILER_GetBinary(Compiler, &binary));

    if (Compiler->context.dumpOptions & slvDUMP_CODE_EMITTER)
    {
        gcmVERIFY_OK(sloCOMPILER_Dump(Compiler,
                          slvDUMP_CODE_EMITTER,
                          "gcSHADER_AddUniformEx1(Shader, \"%s\", %s, %d);",
                          Name,
                          gcGetDataTypeName(UniformInfo->type),
                          UniformInfo->arraySize));
    }

    status = gcSHADER_AddUniformEx1(binary,
                                    Name,
                                    UniformInfo->type,
                                    UniformInfo->precision,
                                    UniformInfo->location,
                                    UniformInfo->binding,
                                    UniformInfo->offset,
                                    UniformInfo->arrayCount,
                                    UniformInfo->arraySizeList,
                                    UniformInfo->varCategory,
                                    UniformInfo->u.numStructureElement,
                                    UniformInfo->parent,
                                    UniformInfo->prevSibling,
                                    UniformInfo->imageFormat,
                                    ThisUniformIndex,
                                    Uniform);

    if (gcmIS_ERROR(status))
    {
        gcmFOOTER();
        return status;
    }

    /*
    ** Current HW can't support 128 bpp image, so if a image is 32bit,
    ** we need to add a extra image layer to handle it.
    ** But if this image is a buffer, we can support 128 bpp, so we don't need to add this extra layer.
    */
    if (!__SUPPORT_128_BPP_DATA__ &&
        !isUniformImageBuffer((*Uniform)) &&
        (UniformInfo->imageFormat == gcIMAGE_FORMAT_RGBA32F ||
         UniformInfo->imageFormat == gcIMAGE_FORMAT_RGBA32I ||
         UniformInfo->imageFormat == gcIMAGE_FORMAT_RGBA32UI))
    {
        gcUNIFORM extraImageLayer = gcvNULL;
        gctSTRING symbol = gcvNULL;
        gctSIZE_T length = 0;
        gctUINT offset = 0;
        gctPOINTER pointer = gcvNULL;
        gctINT16 extraImageLayerIndex = -1;

        gcmASSERT((UniformInfo->type == gcSHADER_IMAGE_2D || UniformInfo->type == gcSHADER_IMAGE_3D) ||
                  (UniformInfo->type >= gcSHADER_IIMAGE_2D && UniformInfo->type <= gcSHADER_UIMAGE_2D_ARRAY) ||
                  (UniformInfo->type >= gcSHADER_IMAGE_CUBEMAP_ARRAY && UniformInfo->type <= gcSHADER_UIMAGE_CUBEMAP_ARRAY) ||
                  (UniformInfo->type >= gcSHADER_IMAGE_BUFFER && UniformInfo->type <= gcSHADER_UIMAGE_BUFFER));

        gcmASSERT((UniformInfo->prevSibling == -1) &&
                  (UniformInfo->parent == -1));

        length = gcoOS_StrLen(Name, gcvNULL);
        length += 21;

        gcoOS_Allocate(gcvNULL, length, &pointer);
        gcoOS_ZeroMemory(pointer, length);
        symbol = pointer;

        gcmVERIFY_OK(gcoOS_PrintStrSafe(symbol,
                                        length,
                                        &offset,
                                        "#sh_imageExtraLayer_%s",
                                        Name));

        status = gcSHADER_AddUniformEx1(binary,
                                        symbol,
                                        UniformInfo->type,
                                        UniformInfo->precision,
                                        -1,
                                        UniformInfo->binding,
                                        UniformInfo->offset,
                                        UniformInfo->arrayCount,
                                        UniformInfo->arraySizeList,
                                        UniformInfo->varCategory,
                                        UniformInfo->u.numStructureElement,
                                        GetUniformIndex(*Uniform),
                                        UniformInfo->prevSibling,
                                        UniformInfo->imageFormat,
                                        &extraImageLayerIndex,
                                        &extraImageLayer);

        gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, symbol));

        if (gcmIS_ERROR(status))
        {
            gcmFOOTER();
            return status;
        }

        SetUniformKind(extraImageLayer, gcvUNIFORM_KIND_IMAGE_EXTRA_LAYER);
        SetUniformFlag(extraImageLayer, gcvUNIFORM_FLAG_COMPILER_GEN);
        gcUNIFORM_SetFormat(extraImageLayer,  UniformInfo->format, gcvFALSE);

        if (Uniform)
        {
            (*Uniform)->firstChild = extraImageLayerIndex;
        }
        if (ThisUniformIndex)
        {
            extraImageLayer->parent = *ThisUniformIndex;
        }
    }

    status =  gcUNIFORM_SetFormat(*Uniform, UniformInfo->format, gcvFALSE);
    gcmFOOTER();
    return status;
}

static gceSTATUS
_AddOutputWithLocation(
    IN sloCOMPILER Compiler,
    IN gctCONST_STRING Name,
    IN gcSHADER_TYPE Type,
    IN gcSHADER_PRECISION Precision,
    IN gctBOOL IsArray,
    IN gctUINT32 Length,
    IN gctREG_INDEX TempRegister,
    IN gcSHADER_SHADERMODE ShaderMode,
    IN gctINT Location,
    IN gctINT FieldIndex,
    IN gctBOOL IsInvariant,
    IN gctBOOL IsPrecise,
    IN gceLAYOUT_QUALIFIER LayoutQual,
    OUT gcOUTPUT * Output
    )
{
    gcSHADER  binary;
    gceSTATUS status;
    gcOUTPUT output;

    gcmHEADER();

    gcmVERIFY_OK(sloCOMPILER_GetBinary(Compiler, &binary));

    if (Compiler->context.dumpOptions & slvDUMP_CODE_EMITTER)
    {
        gcmVERIFY_OK(sloCOMPILER_Dump(Compiler,
                                          slvDUMP_CODE_EMITTER,
                                          "gcSHADER_AddOutputWithLocation(Shader, \"%s\", %s, %d, %d);",
                                          Name,
                                          gcGetDataTypeName(Type),
                                          Length,
                                          TempRegister));
    }

    status = gcSHADER_AddOutputWithLocation(binary,
                                            Name,
                                            Type,
                                            Precision,
                                            IsArray,
                                            Length,
                                            (gctUINT16) TempRegister,
                                            ShaderMode,
                                            Location,
                                            FieldIndex,
                                            IsInvariant,
                                            IsPrecise,
                                            &output);

    if (gcmIS_ERROR(status))
    {
        gcmFOOTER();
        return status;
    }

    status = gcOUTPUT_SetLayoutQualifier(output, LayoutQual);
    SetShaderOutputBlends(binary, LayoutQual);

    if (Output)
    {
        *Output = output;
    }

    gcmFOOTER();
    return status;
}

static gceSTATUS
_AddOutputIndexed(
    IN sloCOMPILER Compiler,
    IN gctCONST_STRING Name,
    IN gctUINT32 Index,
    IN gctREG_INDEX TempIndex
    )
{
    gcSHADER    binary;
    gceSTATUS   status;

    gcmHEADER();

    gcmVERIFY_OK(sloCOMPILER_GetBinary(Compiler, &binary));

    gcmVERIFY_OK(sloCOMPILER_Dump(
                                Compiler,
                                slvDUMP_CODE_EMITTER,
                                "gcSHADER_AddOutputIndexed(Shader, \"%s\", %d, %d);",
                                Name,
                                Index,
                                TempIndex));

    status = gcSHADER_AddOutputIndexed(binary, Name, Index, (gctUINT16)TempIndex);

    gcmFOOTER();
    return status;
}

static gceSTATUS
_AddVariable(
    IN sloCOMPILER Compiler,
    IN gctCONST_STRING Name,
    IN gctREG_INDEX TempRegister,
    IN gcsSHADER_VAR_INFO *VarInfo,
    OUT gctINT16* ThisVarIndex
    )
{
    gcSHADER    binary;
    gceSTATUS   status;

    gcmHEADER();

    gcmVERIFY_OK(sloCOMPILER_GetBinary(Compiler, &binary));

    if (Compiler->context.dumpOptions & slvDUMP_CODE_EMITTER)
    {
        gcmVERIFY_OK(sloCOMPILER_Dump(Compiler,
                    slvDUMP_CODE_EMITTER,
                    "gcSHADER_AddVariableEx1(Shader, \"%s\", %s, %d, %d);",
                    Name,
                    gcGetDataTypeName(VarInfo->type),
                    VarInfo->arraySize,
                    TempRegister));
    }

    status = gcSHADER_AddVariableEx1(binary,
                                    Name,
                                    (gctUINT16) TempRegister,
                                    VarInfo,
                                    ThisVarIndex);

    gcmFOOTER();
    return status;
}

static gceSTATUS
_UpdateVariableTempReg(
    IN sloCOMPILER Compiler,
    IN gctUINT varIndex,
    IN gctREG_INDEX newTempRegIndex
    )
{
    gcSHADER    binary;
    gceSTATUS   status;

    gcmHEADER();

    gcmVERIFY_OK(sloCOMPILER_GetBinary(Compiler, &binary));

    status = gcSHADER_UpdateVariable(binary,
                                     varIndex,
                                     gcvVARIABLE_UPDATE_TEMPREG,
                                     newTempRegIndex);

    gcmFOOTER();
    return status;
}

gctCONST_STRING
gcGetAttributeName(
    IN gcSHADER Shader,
    IN gcATTRIBUTE Attribute
    )
{
    gctCONST_STRING    attributeName;

    gcmVERIFY_OK(gcATTRIBUTE_GetName(Shader, Attribute, gcvTRUE, gcvNULL, &attributeName));

    return attributeName;
}

gctCONST_STRING
gcGetUniformName(
    IN gcUNIFORM Uniform
    )
{
    gctCONST_STRING    uniformName;

    gcmVERIFY_OK(gcUNIFORM_GetName(Uniform, gcvNULL, &uniformName));

    return uniformName;
}


static gceSTATUS
_GetSampler(
    IN sloCOMPILER Compiler,
    IN gcUNIFORM Uniform,
    OUT gctUINT32 * Sampler
    )
{
    gceSTATUS   status;

    gcmHEADER();

    gcmASSERT(Uniform);

    if (Compiler->context.dumpOptions & slvDUMP_CODE_EMITTER)
    {
        gcmVERIFY_OK(sloCOMPILER_Dump(Compiler,
                                      slvDUMP_CODE_EMITTER,
                                      "gcUNIFORM_GetSampler(\"%s\", Sampler);",
                                      gcGetUniformName(Uniform)));
    }

    status = gcUNIFORM_GetSampler(Uniform, Sampler);

    gcmFOOTER();
    return status;
}

static gceSTATUS
_AddLabel(
    IN sloCOMPILER Compiler,
    IN gctUINT Label
    )
{
    gcSHADER    binary;
    gceSTATUS   status;

    gcmHEADER();

    gcmVERIFY_OK(sloCOMPILER_GetBinary(Compiler, &binary));

    gcmVERIFY_OK(sloCOMPILER_Dump(
                                Compiler,
                                slvDUMP_CODE_EMITTER,
                                "gcSHADER_AddLabel(Shader, %d);",
                                Label));

    status = gcSHADER_AddLabel(binary, Label);

    gcmFOOTER();
    return status;
}

static gceSTATUS
_AddFunction(
    IN sloCOMPILER Compiler,
    IN gctCONST_STRING Name,
    OUT gcFUNCTION * Function
    )
{
    gcSHADER    binary;
    gceSTATUS   status;

    gcmHEADER();

    gcmASSERT(Function);

    gcmVERIFY_OK(sloCOMPILER_GetBinary(Compiler, &binary));

    gcmVERIFY_OK(sloCOMPILER_Dump(Compiler,
                                  slvDUMP_CODE_EMITTER,
                                  "gcSHADER_AddFunction(Shader, \"%s\");",
                                  Name));

    status = gcSHADER_AddFunction(binary, Name, Function);

    gcmFOOTER();
    return status;
}

static gceSTATUS
_BeginFunction(
    IN sloCOMPILER Compiler,
    IN gcFUNCTION Function
    )
{
    gcSHADER    binary;
    gceSTATUS   status;

    gcmHEADER();

    gcmVERIFY_OK(sloCOMPILER_GetBinary(Compiler, &binary));

    gcmVERIFY_OK(sloCOMPILER_Dump(Compiler,
                                  slvDUMP_CODE_EMITTER,
                                  "gcSHADER_BeginFunction(Shader);"));

    status = gcSHADER_BeginFunction(binary, Function);

    gcmFOOTER();
    return status;
}

static gceSTATUS
_EndFunction(
    IN sloCOMPILER Compiler,
    IN gcFUNCTION Function
)
{
    gcSHADER    binary;
    gceSTATUS   status;

    gcmHEADER();

    gcmVERIFY_OK(sloCOMPILER_GetBinary(Compiler, &binary));

    gcmVERIFY_OK(sloCOMPILER_Dump(Compiler,
                                  slvDUMP_CODE_EMITTER,
                                  "gcSHADER_EndFunction(Shader);"));

    status = gcSHADER_EndFunction(binary, Function);

    gcmFOOTER();
    return status;
}

static gceSTATUS
_AddFunctionArgument(
    IN sloCOMPILER Compiler,
    IN gcFUNCTION Function,
    IN gctUINT16 VariableIndex,
    IN gctREG_INDEX TempIndex,
    IN gctUINT8 Enable,
    IN gctUINT8 Qualifier,
    IN gctUINT8 Precision,
    IN gctBOOL IsPrecise
    )
{
    gctCHAR        buf[5];
    gceSTATUS   status;

    gcmHEADER();

    if (Compiler->context.dumpOptions & slvDUMP_CODE_EMITTER)
    {
        gcmVERIFY_OK(sloCOMPILER_Dump(Compiler,
                                      slvDUMP_CODE_EMITTER,
                                      "gcFUNCTION_AddArgument(Function, %d, Variable, %d, gcSL_ENABLE_%s, %s, %s);",
                                      TempIndex,
                                      VariableIndex,
                                      _GetEnableName(Enable, buf),
                                      _GetQualifierName((gceINPUT_OUTPUT) Qualifier),
                                      _GetPrecisionName((gcSHADER_PRECISION) Precision)));
    }

    status = gcFUNCTION_AddArgument(Function, VariableIndex, (gctUINT16)TempIndex, Enable, Qualifier, Precision, IsPrecise);

    gcmFOOTER();
    return status;
}

gceSTATUS
_GetFunctionLabel(
    IN sloCOMPILER Compiler,
    IN gcFUNCTION Function,
    OUT gctUINT_PTR Label
    )
{
    gceSTATUS status;

    gcmHEADER();

    gcmVERIFY_OK(sloCOMPILER_Dump(Compiler,
                  slvDUMP_CODE_EMITTER,
                  "gcFUNCTION_GetLabel(Function, Label);"));

    status = gcFUNCTION_GetLabel(Function, Label);

    gcmFOOTER();
    return status;
}

gcSL_FORMAT
slConvImageFormatToFormat(
    IN sloCOMPILER Compiler,
    IN gceIMAGE_FORMAT ImageFormat
    )
{
    /* All these image related uniforms stores
       image address (x),
       stride (y),
       image size (z)
       other packed misc info (w)

       so whole uniform are 32 unsigned integer.
    */
    return gcSL_UINT32;
}

gcSL_FORMAT
slConvDataTypeToFormat(
sloCOMPILER Compiler,
gcSHADER_TYPE DataType
)
{
    gcSL_FORMAT format = gcSL_FLOAT;

#if TREAT_ES20_INTEGER_AS_FLOAT
    if (sloCOMPILER_IsHaltiVersion(Compiler))
    {
#endif
        switch (DataType) {
        case gcSHADER_FLOAT_X1:
        case gcSHADER_FLOAT_X2:
        case gcSHADER_FLOAT_X3:
        case gcSHADER_FLOAT_X4:
        case gcSHADER_FLOAT_4X2:
        case gcSHADER_FLOAT_4X3:
        case gcSHADER_FLOAT_4X4:
        case gcSHADER_FLOAT_2X2:
        case gcSHADER_FLOAT_2X3:
        case gcSHADER_FLOAT_2X4:
        case gcSHADER_FLOAT_3X2:
        case gcSHADER_FLOAT_3X3:
        case gcSHADER_FLOAT_3X4:
            format = gcSL_FLOAT;
            break;

        case gcSHADER_INTEGER_X1:
        case gcSHADER_INTEGER_X2:
        case gcSHADER_INTEGER_X3:
        case gcSHADER_INTEGER_X4:
            format = gcSL_INTEGER;
            break;

        case gcSHADER_UINT_X1:
        case gcSHADER_UINT_X2:
        case gcSHADER_UINT_X3:
        case gcSHADER_UINT_X4:
            format = gcSL_UINT32;
            break;

        case gcSHADER_BOOLEAN_X1:
        case gcSHADER_BOOLEAN_X2:
        case gcSHADER_BOOLEAN_X3:
        case gcSHADER_BOOLEAN_X4:
            format = gcSL_BOOLEAN;
            break;

        case gcSHADER_SAMPLER_1D:
        case gcSHADER_SAMPLER_2D:
        case gcSHADER_SAMPLER_3D:
        case gcSHADER_SAMPLER_BUFFER:
        case gcSHADER_SAMPLER_CUBIC:
        case gcSHADER_SAMPLER_CUBEMAP_ARRAY:
        case gcSHADER_SAMPLER_EXTERNAL_OES:
        case gcSHADER_SAMPLER_2D_SHADOW:
        case gcSHADER_SAMPLER_CUBE_SHADOW:
        case gcSHADER_SAMPLER_CUBEMAP_ARRAY_SHADOW:
        case gcSHADER_SAMPLER_1D_ARRAY:
        case gcSHADER_SAMPLER_1D_ARRAY_SHADOW:
        case gcSHADER_SAMPLER_2D_ARRAY:
        case gcSHADER_SAMPLER_2D_ARRAY_SHADOW:

        case gcSHADER_SAMPLER_2D_MS:
        case gcSHADER_SAMPLER_2D_MS_ARRAY:
            format = gcSL_FLOAT;
            break;

        case gcSHADER_ISAMPLER_2D:
        case gcSHADER_ISAMPLER_3D:
        case gcSHADER_ISAMPLER_BUFFER:
        case gcSHADER_ISAMPLER_CUBIC:
        case gcSHADER_ISAMPLER_CUBEMAP_ARRAY:
        case gcSHADER_ISAMPLER_2D_ARRAY:
        case gcSHADER_ISAMPLER_2D_MS:
        case gcSHADER_ISAMPLER_2D_MS_ARRAY:
            format = gcSL_INTEGER;
            break;

        case gcSHADER_USAMPLER_2D:
        case gcSHADER_USAMPLER_3D:
        case gcSHADER_USAMPLER_BUFFER:
        case gcSHADER_USAMPLER_CUBIC:
        case gcSHADER_USAMPLER_CUBEMAP_ARRAY:
        case gcSHADER_USAMPLER_2D_ARRAY:
        case gcSHADER_USAMPLER_2D_MS:
        case gcSHADER_USAMPLER_2D_MS_ARRAY:
            format = gcSL_UINT32;
            break;

        case gcSHADER_IMAGE_2D:
        case gcSHADER_IMAGE_3D:
        case gcSHADER_IMAGE_BUFFER:
        case gcSHADER_IMAGE_CUBE:
        case gcSHADER_IMAGE_CUBEMAP_ARRAY:
        case gcSHADER_IMAGE_2D_ARRAY:
        case gcSHADER_IIMAGE_2D:
        case gcSHADER_IIMAGE_3D:
        case gcSHADER_IIMAGE_BUFFER:
        case gcSHADER_IIMAGE_CUBE:
        case gcSHADER_IIMAGE_CUBEMAP_ARRAY:
        case gcSHADER_IIMAGE_2D_ARRAY:
        case gcSHADER_UIMAGE_2D:
        case gcSHADER_UIMAGE_3D:
        case gcSHADER_UIMAGE_BUFFER:
        case gcSHADER_UIMAGE_CUBE:
        case gcSHADER_UIMAGE_CUBEMAP_ARRAY:
        case gcSHADER_UIMAGE_2D_ARRAY:
            format = gcSL_UINT32;
            break;

        case gcSHADER_ATOMIC_UINT:
            format = gcSL_UINT32;
            break;
        default:
            gcmASSERT(0);
        }
#if TREAT_ES20_INTEGER_AS_FLOAT
    }
#endif
    return format;
}

#if GC_ENABLE_DUAL_FP16
static gceSTATUS
_EmitOpcodeAndTargetFormatted(
    IN sloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gcSL_OPCODE Opcode,
    IN gcsTARGET * Target,
    IN gcSL_FORMAT Format
    )
{
    gceSTATUS status;
    gcSHADER binary;
    gctCHAR  buf[5];
    gctUINT32 srcLoc;

    gcmHEADER();

    gcmASSERT(Target);

    gcmVERIFY_OK(sloCOMPILER_GetBinary(Compiler, &binary));

    srcLoc = GCSL_Build_SRC_LOC(LineNo, StringNo);

    if (!Target)
    {
        if (Compiler->context.dumpOptions & slvDUMP_CODE_EMITTER)
        {
            gcmVERIFY_OK(sloCOMPILER_Dump(Compiler,
                                          slvDUMP_CODE_EMITTER,
                                          "gcSHADER_AddOpcode(Shader, %s);",
                                          GetOpcodeName(Opcode)));
        }

        status = gcSHADER_AddOpcode(binary,
                                    Opcode,
                                    0,
                                    gcSL_ENABLE_NONE,
                                    gcSL_FLOAT,
                                    gcSHADER_PRECISION_DEFAULT,
                                    srcLoc);
    }
    else if (Target->indexMode == gcSL_NOT_INDEXED)
    {
        if (Compiler->context.dumpOptions & slvDUMP_CODE_EMITTER)
        {
            gcmVERIFY_OK(sloCOMPILER_Dump(Compiler,
                                          slvDUMP_CODE_EMITTER,
                                          "gcSHADER_AddOpcodeIndexedWithPrecision(Shader, %s, dst = r%d, gcSL_ENABLE_%s, %s, %d, %s, %s);",
                                          GetOpcodeName(Opcode),
                                          Target->tempRegIndex,
                                          _GetEnableName(Target->enable, buf),
                                          _GetIndexModeName(gcSL_NOT_INDEXED),
                                          0,
                                          _GetFormatName(Format),
                                          _GetPrecisionName(Target->precision)));
        }

        status = gcSHADER_AddOpcodeIndexedWithPrecision(binary,
                                                        Opcode,
                                                        (gctUINT16) Target->tempRegIndex,
                                                        Target->enable,
                                                        gcSL_NOT_INDEXED,
                                                        0,
                                                        Format,
                                                        Target->precision,
                                                        srcLoc);
    }
    else
    {
        if (Compiler->context.dumpOptions & slvDUMP_CODE_EMITTER)
        {
            gcmVERIFY_OK(sloCOMPILER_Dump(Compiler,
                                          slvDUMP_CODE_EMITTER,
                                          "gcSHADER_AddOpcodeIndexedWithPrecision(Shader, %s, dst = r%d, gcSL_ENABLE_%s, %s, index = r%d, %s, %s);",
                                          GetOpcodeName(Opcode),
                                          Target->tempRegIndex,
                                          _GetEnableName(Target->enable, buf),
                                          _GetIndexModeName(Target->indexMode),
                                          Target->indexRegIndex,
                                          _GetFormatName(Format),
                                          _GetPrecisionName(Target->precision)));
        }

        status = gcSHADER_AddOpcodeIndexedWithPrecision(binary,
                                                        Opcode,
                                                        (gctUINT16) Target->tempRegIndex,
                                                        Target->enable,
                                                        Target->indexMode,
                                                        (gctUINT16) Target->indexRegIndex,
                                                        Format,
                                                        Target->precision,
                                                        srcLoc);
    }

   if (gcmIS_ERROR(status))
   {
       gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                       LineNo,
                                       StringNo,
                                       slvREPORT_INTERNAL_ERROR,
                                       "failed to add the opcode"));

       gcmFOOTER();
       return status;
   }

   gcmFOOTER_NO();
   return gcvSTATUS_OK;
}
#else
static gceSTATUS
_EmitOpcodeAndTargetFormatted(
    IN sloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gcSL_OPCODE Opcode,
    IN gcsTARGET * Target,
    IN gcSL_FORMAT Format
    )
{
    gceSTATUS status;
    gcSHADER binary;
    gctCHAR  buf[5];
    gctUINT32 srcLoc;

    gcmHEADER();

    gcmASSERT(Target);

    srcLoc = GCSL_Build_SRC_LOC(LineNo, StringNo);

    gcmVERIFY_OK(sloCOMPILER_GetBinary(Compiler, &binary));
    if (Target->indexMode == gcSL_NOT_INDEXED)
    {
        gcmVERIFY_OK(sloCOMPILER_Dump(Compiler,
                                      slvDUMP_CODE_EMITTER,
                                      "gcSHADER_AddOpcode(Shader, %s, %d, gcSL_ENABLE_%s, %s);",
                                      GetOpcodeName(Opcode),
                                      Target->tempRegIndex,
                                      _GetEnableName(Target->enable, buf),
                                      _GetFormatName(Format)));

        status = gcSHADER_AddOpcode(binary,
                                    Opcode,
                                    (gctUINT16) Target->tempRegIndex,
                                    Target->enable,
                                    Format,
                                    srcLoc);
   }
   else
   {
        gcmVERIFY_OK(sloCOMPILER_Dump(Compiler,
                                      slvDUMP_CODE_EMITTER,
                                      "gcSHADER_AddOpcodeIndexed(Shader, %s, %d, gcSL_ENABLE_%s, %s, %d, %s);",
                                      GetOpcodeName(Opcode),
                                      Target->tempRegIndex,
                                      _GetEnableName(Target->enable, buf),
                                      _GetIndexModeName(Target->indexMode),
                                      Target->indexRegIndex,
                                      _GetFormatName(Format)));

        status = gcSHADER_AddOpcodeIndexed(binary,
                                           Opcode,
                                           (gctUINT16) Target->tempRegIndex,
                                           Target->enable,
                                           Target->indexMode,
                                           (gctUINT16)Target->indexRegIndex,
                                           Format,
                                           srcLoc);
   }

   if (gcmIS_ERROR(status))
   {
       gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                       LineNo,
                                       StringNo,
                                       slvREPORT_INTERNAL_ERROR,
                                       "failed to add the opcode"));

       gcmFOOTER();
       return status;
   }

   gcmFOOTER_NO();
   return gcvSTATUS_OK;
}
#endif

static gceSTATUS
_EmitOpcodeConditional(
    IN sloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gcSL_OPCODE Opcode,
    IN gcSL_CONDITION Condition,
    IN gcsSOURCE *Source,
    IN gctLABEL Label
    )
{
    gceSTATUS    status;
    gcSHADER    binary;
    gctUINT32   srcLoc;

    gcmHEADER();

    srcLoc = GCSL_Build_SRC_LOC(LineNo, StringNo);

    gcmVERIFY_OK(sloCOMPILER_GetBinary(Compiler, &binary));
    if(Source)
    {
        gcSL_FORMAT format;

        format = slConvDataTypeToFormat(Compiler, Source->dataType);

        if (Compiler->context.dumpOptions & slvDUMP_CODE_EMITTER)
        {
            gcmVERIFY_OK(sloCOMPILER_Dump(Compiler,
                                          slvDUMP_CODE_EMITTER,
                                          "gcSHADER_AddOpcodeConditionalFormatted(Shader, %s, %s, %s, label = l%d);",
                                          GetOpcodeName(Opcode),
                                          _GetConditionName(Condition),
                                          _GetFormatName(format),
                                          Label));
        }

        status = gcSHADER_AddOpcodeConditionalFormatted(binary, Opcode, Condition, format, Label, srcLoc);
    }
    else
    {
        if (Compiler->context.dumpOptions & slvDUMP_CODE_EMITTER)
        {
            gcmVERIFY_OK(sloCOMPILER_Dump(Compiler,
                                          slvDUMP_CODE_EMITTER,
                                          "gcSHADER_AddOpcodeConditional(Shader, %s, %s, label = l%d);",
                                          GetOpcodeName(Opcode),
                                          _GetConditionName(Condition),
                                          Label));
        }

        status = gcSHADER_AddOpcodeConditional(binary, Opcode, Condition, Label, srcLoc);
    }

    if (gcmIS_ERROR(status))
    {
        gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                        LineNo,
                                        StringNo,
                                        slvREPORT_INTERNAL_ERROR,
                                        "failed to add the conditional opcode"));

        gcmFOOTER();
        return status;
    }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

#if GC_ENABLE_DUAL_FP16
static gceSTATUS
_EmitSourceTemp(
    IN sloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gctBOOL IsSamplerIndex,
    IN gcsSOURCE *Source
    )
{
    gceSTATUS   status;
    gcSL_FORMAT format;
    gcSHADER    binary;
    gctCHAR    buf[5];

    gcmHEADER();

    gcmASSERT(Source);

    format = slConvDataTypeToFormat(Compiler, Source->dataType);
    gcmVERIFY_OK(sloCOMPILER_GetBinary(Compiler, &binary));
    if (IsSamplerIndex)
    {
        if (Compiler->context.dumpOptions & slvDUMP_CODE_EMITTER)
        {
            gcmVERIFY_OK(sloCOMPILER_Dump(Compiler,
                                          slvDUMP_CODE_EMITTER,
                                          "gcSHADER_AddSourceSamplerIndexedFormattedwithPrecision(Shader,"
                                          " gcSL_SWIZZLE_%s, %s, src=r%d, %s, %s);",
                                          _GetSwizzleName(Source->u.sourceReg.swizzle, buf),
                                          _GetIndexModeName(Source->u.sourceReg.indexMode),
                                          Source->u.sourceReg.regIndex,
                                          _GetFormatName(format),
                                          _GetPrecisionName(Source->precision)));
        }

        status = gcSHADER_AddSourceSamplerIndexedFormattedWithPrecision(binary,
                                                                        gcSL_SWIZZLE_XYZW,
                                                                        gcSL_INDEXED_X,
                                                                        (gctUINT16)Source->u.sourceReg.regIndex,
                                                                        format,
                                                                        Source->precision);
    }
    else if (Source->u.sourceReg.indexMode == gcSL_NOT_INDEXED)
    {
        if (Compiler->context.dumpOptions & slvDUMP_CODE_EMITTER)
        {
            gcmVERIFY_OK(sloCOMPILER_Dump(Compiler,
                                          slvDUMP_CODE_EMITTER,
                                          "gcSHADER_AddSourceIndexedWithPrecision(Shader, %s, src = r%d, gcSL_SWIZZLE_%s, %s, %d, %s, %s);",
                                          _GetTypeName(gcSL_TEMP),
                                          Source->u.sourceReg.regIndex,
                                          _GetSwizzleName(Source->u.sourceReg.swizzle, buf),
                                          _GetIndexModeName(gcSL_NOT_INDEXED),
                                          0,
                                          _GetFormatName(format),
                                          _GetPrecisionName(Source->precision)));
        }

        status = gcSHADER_AddSourceIndexedWithPrecision(binary,
                                                        gcSL_TEMP,
                                                        Source->u.sourceReg.regIndex,
                                                        Source->u.sourceReg.swizzle,
                                                        gcSL_NOT_INDEXED,
                                                        0,
                                                        format,
                                                        Source->precision);
    }
    else
    {
        if (Compiler->context.dumpOptions & slvDUMP_CODE_EMITTER)
        {
            gcmVERIFY_OK(sloCOMPILER_Dump(Compiler,
                                          slvDUMP_CODE_EMITTER,
                                          "gcSHADER_AddSourceIndexedWithPrecision(Shader, %s, src = r%d, gcSL_SWIZZLE_%s, %s, %d, %s, %s);",
                                          _GetTypeName(gcSL_TEMP),
                                          Source->u.sourceReg.regIndex,
                                          _GetSwizzleName(Source->u.sourceReg.swizzle, buf),
                                          _GetIndexModeName(Source->u.sourceReg.indexMode),
                                          Source->u.sourceReg.indexRegIndex,
                                          _GetFormatName(format),
                                          _GetPrecisionName(Source->precision)));
        }

        status = gcSHADER_AddSourceIndexedWithPrecision(binary,
                                                        gcSL_TEMP,
                                                        Source->u.sourceReg.regIndex,
                                                        Source->u.sourceReg.swizzle,
                                                        Source->u.sourceReg.indexMode,
                                                        (gctUINT16)Source->u.sourceReg.indexRegIndex,
                                                        format,
                                                        Source->precision);
    }

    if (gcmIS_ERROR(status))
    {
        gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                        LineNo,
                                        StringNo,
                                        slvREPORT_INTERNAL_ERROR,
                                        "failed to add the source"));

        gcmFOOTER();
        return status;
    }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

static gceSTATUS
_EmitSourceTempWithFormat(
    IN sloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gctBOOL IsSamplerIndex,
    IN gcsSOURCE *Source,
    IN gcSL_FORMAT Format
    )
{
    gceSTATUS   status;
    gcSHADER    binary;
    gctCHAR    buf[5];

    gcmHEADER();

    gcmASSERT(Source);


    gcmVERIFY_OK(sloCOMPILER_GetBinary(Compiler, &binary));
    if (IsSamplerIndex)
    {
        if (Compiler->context.dumpOptions & slvDUMP_CODE_EMITTER)
        {
            gcmVERIFY_OK(sloCOMPILER_Dump(Compiler,
                                          slvDUMP_CODE_EMITTER,
                                          "gcSHADER_AddSourceSamplerIndexedFormattedwithPrecision(Shader,"
                                          " gcSL_SWIZZLE_%s, %s, src = r%d, %s, %s);",
                                          _GetSwizzleName(Source->u.sourceReg.swizzle, buf),
                                          _GetIndexModeName(Source->u.sourceReg.indexMode),
                                          Source->u.sourceReg.regIndex,
                                          _GetFormatName(Format),
                                          _GetPrecisionName(Source->precision)));
        }

        status = gcSHADER_AddSourceSamplerIndexedFormattedWithPrecision(binary,
                                                                        gcSL_SWIZZLE_XYZW,
                                                                        gcSL_INDEXED_X,
                                                                        (gctUINT16)Source->u.sourceReg.regIndex,
                                                                        Format,
                                                                        Source->precision);
    }
    else if (Source->u.sourceReg.indexMode == gcSL_NOT_INDEXED)
    {
        if (Compiler->context.dumpOptions & slvDUMP_CODE_EMITTER)
        {
            gcmVERIFY_OK(sloCOMPILER_Dump(Compiler,
                                          slvDUMP_CODE_EMITTER,
                                          "gcSHADER_AddSourceIndexedWithPrecision(Shader, %s, src = r%d, gcSL_SWIZZLE_%s, %s, %d, %s, %s);",
                                          _GetTypeName(gcSL_TEMP),
                                          Source->u.sourceReg.regIndex,
                                          _GetSwizzleName(Source->u.sourceReg.swizzle, buf),
                                          _GetIndexModeName(gcSL_NOT_INDEXED),
                                          0,
                                          _GetFormatName(Format),
                                          _GetPrecisionName(Source->precision)));
        }

        status = gcSHADER_AddSourceIndexedWithPrecision(binary,
                                                        gcSL_TEMP,
                                                        Source->u.sourceReg.regIndex,
                                                        Source->u.sourceReg.swizzle,
                                                        gcSL_NOT_INDEXED,
                                                        0,
                                                        Format,
                                                        Source->precision);
    }
    else
    {
        if (Compiler->context.dumpOptions & slvDUMP_CODE_EMITTER)
        {
            gcmVERIFY_OK(sloCOMPILER_Dump(Compiler,
                                          slvDUMP_CODE_EMITTER,
                                          "gcSHADER_AddSourceIndexedWithPrecision(Shader, %s, src = r%d, gcSL_SWIZZLE_%s, %s, index = r%d, %s, %s);",
                                          _GetTypeName(gcSL_TEMP),
                                          Source->u.sourceReg.regIndex,
                                          _GetSwizzleName(Source->u.sourceReg.swizzle, buf),
                                          _GetIndexModeName(Source->u.sourceReg.indexMode),
                                          Source->u.sourceReg.indexRegIndex,
                                          _GetFormatName(Format),
                                          _GetPrecisionName(Source->precision)));
        }

        status = gcSHADER_AddSourceIndexedWithPrecision(binary,
                                                        gcSL_TEMP,
                                                        Source->u.sourceReg.regIndex,
                                                        Source->u.sourceReg.swizzle,
                                                        Source->u.sourceReg.indexMode,
                                                        (gctUINT16)Source->u.sourceReg.indexRegIndex,
                                                        Format,
                                                        Source->precision);
    }

    if (gcmIS_ERROR(status))
    {
        gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                        LineNo,
                                        StringNo,
                                        slvREPORT_INTERNAL_ERROR,
                                        "failed to add the source"));

        gcmFOOTER();
        return status;
    }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

static gceSTATUS
_EmitSourceAttributeWithFormat(
    IN sloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gcsSOURCE * Source,
    IN gcSL_FORMAT Format
    )
{
    gceSTATUS    status;
    gcSHADER binary;
    gctCHAR  buf[5];

    gcmHEADER();

    gcmASSERT(Source);

    gcmVERIFY_OK(sloCOMPILER_GetBinary(Compiler, &binary));
    if (Source->u.sourceReg.indexMode == gcSL_NOT_INDEXED)
    {
        if (Compiler->context.dumpOptions & slvDUMP_CODE_EMITTER)
        {
            gcmVERIFY_OK(sloCOMPILER_Dump(Compiler,
                                          slvDUMP_CODE_EMITTER,
                                          "gcSHADER_AddSourceAttributeIndexedFormattedWithPrecision(Shader, \"%s\","
                                          " gcSL_SWIZZLE_%s, src = r%d, %s, %s);",
                                          gcGetAttributeName(binary, Source->u.sourceReg.u.attribute),
                                          _GetSwizzleName(Source->u.sourceReg.swizzle, buf),
                                          Source->u.sourceReg.regIndex,
                                          _GetIndexModeName(gcSL_NOT_INDEXED),
                                          0,
                                          _GetFormatName(Format),
                                          _GetPrecisionName(Source->precision)));
        }

        status = gcSHADER_AddSourceAttributeIndexedFormattedWithPrecision(binary,
                                                                          Source->u.sourceReg.u.attribute,
                                                                          Source->u.sourceReg.swizzle,
                                                                          Source->u.sourceReg.regIndex,
                                                                          gcSL_NOT_INDEXED,
                                                                          0,
                                                                          Format,
                                                                          Source->precision);
    }
    else
    {
        if (Compiler->context.dumpOptions & slvDUMP_CODE_EMITTER)
        {
            gcmVERIFY_OK(sloCOMPILER_Dump(Compiler,
                                          slvDUMP_CODE_EMITTER,
                                          "gcSHADER_AddSourceAttributeIndexedFormattedWithPrecision(Shader, \"%s\","
                                          " gcSL_SWIZZLE_%s, src = r%d, %s, index = r%d, %s, %s);",
                                          gcGetAttributeName(binary, Source->u.sourceReg.u.attribute),
                                          _GetSwizzleName(Source->u.sourceReg.swizzle, buf),
                                          Source->u.sourceReg.regIndex,
                                          _GetIndexModeName(Source->u.sourceReg.indexMode),
                                          Source->u.sourceReg.indexRegIndex,
                                          _GetFormatName(Format),
                                          _GetPrecisionName(Source->precision)));
        }

        status = gcSHADER_AddSourceAttributeIndexedFormattedWithPrecision(binary,
                                                                          Source->u.sourceReg.u.attribute,
                                                                          Source->u.sourceReg.swizzle,
                                                                          Source->u.sourceReg.regIndex,
                                                                          Source->u.sourceReg.indexMode,
                                                                          (gctUINT16)Source->u.sourceReg.indexRegIndex,
                                                                          Format,
                                                                          Source->precision);
    }

    if (gcmIS_ERROR(status))
    {
        gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                        LineNo,
                                        StringNo,
                                        slvREPORT_INTERNAL_ERROR,
                                        "failed to add the source attribute"));

        gcmFOOTER();
        return status;
    }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

static gceSTATUS
_EmitSourceAttribute(
    IN sloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gcsSOURCE * Source
    )
{
    gceSTATUS    status;
    gcSL_FORMAT format;
    gcSHADER binary;
    gctCHAR  buf[5];

    gcmHEADER();

    gcmASSERT(Source);
    format = slConvDataTypeToFormat(Compiler, Source->dataType);

    gcmVERIFY_OK(sloCOMPILER_GetBinary(Compiler, &binary));
    if (Source->u.sourceReg.indexMode == gcSL_NOT_INDEXED)
    {
        if (Compiler->context.dumpOptions & slvDUMP_CODE_EMITTER)
        {
            gcmVERIFY_OK(sloCOMPILER_Dump(Compiler,
                                          slvDUMP_CODE_EMITTER,
                                          "gcSHADER_AddSourceAttributeIndexedFormattedWithPrecision(Shader, \"%s\","
                                          " gcSL_SWIZZLE_%s, src = r%d, %s, %s);",
                                          gcGetAttributeName(binary, Source->u.sourceReg.u.attribute),
                                          _GetSwizzleName(Source->u.sourceReg.swizzle, buf),
                                          Source->u.sourceReg.regIndex,
                                          _GetIndexModeName(gcSL_NOT_INDEXED),
                                          0,
                                          _GetFormatName(format),
                                          _GetPrecisionName(Source->precision)));
        }

        status = gcSHADER_AddSourceAttributeIndexedFormattedWithPrecision(binary,
                                                                          Source->u.sourceReg.u.attribute,
                                                                          Source->u.sourceReg.swizzle,
                                                                          Source->u.sourceReg.regIndex,
                                                                          gcSL_NOT_INDEXED,
                                                                          0,
                                                                          format,
                                                                          Source->precision);
    }
    else
    {
        if (Compiler->context.dumpOptions & slvDUMP_CODE_EMITTER)
        {
            gcmVERIFY_OK(sloCOMPILER_Dump(Compiler,
                                          slvDUMP_CODE_EMITTER,
                                          "gcSHADER_AddSourceAttributeIndexedFormattedWithPrecision(Shader, \"%s\","
                                          " gcSL_SWIZZLE_%s, src = r%d, %s, index = r%d, %s, %s);",
                                          gcGetAttributeName(binary, Source->u.sourceReg.u.attribute),
                                          _GetSwizzleName(Source->u.sourceReg.swizzle, buf),
                                          Source->u.sourceReg.regIndex,
                                          _GetIndexModeName(Source->u.sourceReg.indexMode),
                                          Source->u.sourceReg.indexRegIndex,
                                          _GetFormatName(format),
                                          _GetPrecisionName(Source->precision)));
        }

        status = gcSHADER_AddSourceAttributeIndexedFormattedWithPrecision(binary,
                                                                          Source->u.sourceReg.u.attribute,
                                                                          Source->u.sourceReg.swizzle,
                                                                          Source->u.sourceReg.regIndex,
                                                                          Source->u.sourceReg.indexMode,
                                                                          (gctUINT16)Source->u.sourceReg.indexRegIndex,
                                                                          format,
                                                                          Source->precision);
    }

    if (gcmIS_ERROR(status))
    {
        gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                        LineNo,
                                        StringNo,
                                        slvREPORT_INTERNAL_ERROR,
                                        "failed to add the source attribute"));

        gcmFOOTER();
        return status;
    }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}


static gceSTATUS
_EmitSourceUniformWithFormat(
    IN sloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gcsSOURCE * Source,
    IN gcSL_FORMAT Format
    )
{
    gceSTATUS status;
    gcSHADER binary;
    gctCHAR  buf[5];

    gcmHEADER();

    gcmASSERT(Source);

    gcmVERIFY_OK(sloCOMPILER_GetBinary(Compiler, &binary));
    if (Source->u.sourceReg.indexMode == gcSL_NOT_INDEXED)
    {
        if (Compiler->context.dumpOptions & slvDUMP_CODE_EMITTER)
        {
            gcmVERIFY_OK(sloCOMPILER_Dump(Compiler,
                                          slvDUMP_CODE_EMITTER,
                                          "gcSHADER_AddSourceUniformIndexedFormattedWithPrecision(Shader, \"%s\","
                                          " gcSL_SWIZZLE_%s, src = r%d, %s, %d, %s, %s);",
                                          gcGetUniformName(Source->u.sourceReg.u.uniform),
                                          _GetSwizzleName(Source->u.sourceReg.swizzle, buf),
                                          Source->u.sourceReg.regIndex,
                                          _GetIndexModeName(gcSL_NOT_INDEXED),
                                          0,
                                          _GetFormatName(Format),
                                          _GetPrecisionName(Source->precision)));
        }

        status = gcSHADER_AddSourceUniformIndexedFormattedWithPrecision(binary,
                                                                        Source->u.sourceReg.u.uniform,
                                                                        Source->u.sourceReg.swizzle,
                                                                        Source->u.sourceReg.regIndex,
                                                                        gcSL_NOT_INDEXED,
                                                                        gcSL_NONE_INDEXED,
                                                                        0,
                                                                        Format,
                                                                        Source->precision);
    }
    else
    {
        if (Compiler->context.dumpOptions & slvDUMP_CODE_EMITTER)
        {
            gcmVERIFY_OK(sloCOMPILER_Dump(Compiler,
                                          slvDUMP_CODE_EMITTER,
                                          "gcSHADER_AddSourceUniformIndexedFormattedWithPrecision(Shader, \"%s\","
                                          " gcSL_SWIZZLE_%s, src = r%d, %s, index = r%d, %s, %s);",
                                          gcGetUniformName(Source->u.sourceReg.u.uniform),
                                          _GetSwizzleName(Source->u.sourceReg.swizzle, buf),
                                          Source->u.sourceReg.regIndex,
                                          _GetIndexModeName(Source->u.sourceReg.indexMode),
                                          Source->u.sourceReg.indexRegIndex,
                                          _GetFormatName(Format),
                                          _GetPrecisionName(Source->precision)));
        }

        status = gcSHADER_AddSourceUniformIndexedFormattedWithPrecision(binary,
                                                                        Source->u.sourceReg.u.uniform,
                                                                        Source->u.sourceReg.swizzle,
                                                                        Source->u.sourceReg.regIndex,
                                                                        Source->u.sourceReg.indexMode,
                                                                        Source->u.sourceReg.indexedLevel,
                                                                        (gctUINT16)Source->u.sourceReg.indexRegIndex,
                                                                        Format,
                                                                        Source->precision);
    }

    if (gcmIS_ERROR(status))
    {
        gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                        LineNo,
                                        StringNo,
                                        slvREPORT_INTERNAL_ERROR,
                                        "failed to add the source uniform"));

          gcmFOOTER();
           return status;
    }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

static gceSTATUS
_EmitSourceUniform(
    IN sloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gcsSOURCE * Source
    )
{
    gceSTATUS status;
    gcSL_FORMAT format;
    gcSHADER binary;
    gctCHAR  buf[5];
    gcSHADER_TYPE  type;

    gcmHEADER();

    gcmASSERT(Source);

    type = Source->u.sourceReg.u.uniform->u.type;

    if ((type == gcSHADER_IMAGE_2D || type == gcSHADER_IMAGE_3D) ||
        (type >= gcSHADER_IIMAGE_2D && type <= gcSHADER_UIMAGE_2D_ARRAY))
    {
        format = slConvImageFormatToFormat(Compiler, Source->u.sourceReg.u.uniform->imageFormat);
    }
    else
    {
        format = slConvDataTypeToFormat(Compiler, Source->dataType);
    }

    gcmVERIFY_OK(sloCOMPILER_GetBinary(Compiler, &binary));
    if (Source->u.sourceReg.indexMode == gcSL_NOT_INDEXED)
    {
        if (Compiler->context.dumpOptions & slvDUMP_CODE_EMITTER)
        {
            gcmVERIFY_OK(sloCOMPILER_Dump(Compiler,
                                          slvDUMP_CODE_EMITTER,
                                          "gcSHADER_AddSourceUniformIndexedFormattedWithPrecision(Shader, \"%s\","
                                          " gcSL_SWIZZLE_%s, src = r%d, %s, %d, %s, %s);",
                                          gcGetUniformName(Source->u.sourceReg.u.uniform),
                                          _GetSwizzleName(Source->u.sourceReg.swizzle, buf),
                                          Source->u.sourceReg.regIndex,
                                          _GetIndexModeName(gcSL_NOT_INDEXED),
                                          0,
                                          _GetFormatName(format),
                                          _GetPrecisionName(Source->precision)));
        }

        status = gcSHADER_AddSourceUniformIndexedFormattedWithPrecision(binary,
                                                                        Source->u.sourceReg.u.uniform,
                                                                        Source->u.sourceReg.swizzle,
                                                                        Source->u.sourceReg.regIndex,
                                                                        gcSL_NOT_INDEXED,
                                                                        gcSL_NONE_INDEXED,
                                                                        0,
                                                                        format,
                                                                        Source->precision);
    }
    else
    {
        if (Compiler->context.dumpOptions & slvDUMP_CODE_EMITTER)
        {
            gcmVERIFY_OK(sloCOMPILER_Dump(Compiler,
                                          slvDUMP_CODE_EMITTER,
                                          "gcSHADER_AddSourceUniformIndexedFormattedWithPrecision(Shader, \"%s\","
                                          " gcSL_SWIZZLE_%s, src = r%d, %s, index = r%d, %s, %s);",
                                          gcGetUniformName(Source->u.sourceReg.u.uniform),
                                          _GetSwizzleName(Source->u.sourceReg.swizzle, buf),
                                          Source->u.sourceReg.regIndex,
                                          _GetIndexModeName(Source->u.sourceReg.indexMode),
                                          Source->u.sourceReg.indexRegIndex,
                                          _GetFormatName(format),
                                          _GetPrecisionName(Source->precision)));
        }

        status = gcSHADER_AddSourceUniformIndexedFormattedWithPrecision(binary,
                                                                        Source->u.sourceReg.u.uniform,
                                                                        Source->u.sourceReg.swizzle,
                                                                        Source->u.sourceReg.regIndex,
                                                                        Source->u.sourceReg.indexMode,
                                                                        Source->u.sourceReg.indexedLevel,
                                                                        (gctUINT16)Source->u.sourceReg.indexRegIndex,
                                                                        format,
                                                                        Source->precision);
    }

    if (gcmIS_ERROR(status))
    {
        gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                        LineNo,
                                        StringNo,
                                        slvREPORT_INTERNAL_ERROR,
                                        "failed to add the source uniform"));

          gcmFOOTER();
           return status;
    }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

static gceSTATUS
_EmitSourceOutput(
    IN sloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gcsSOURCE * Source
    )
{
    gceSTATUS    status;
    gcSL_FORMAT format;
    gcSHADER binary;
    gctCHAR  buf[5];

    gcmHEADER();

    gcmASSERT(Source);
    format = slConvDataTypeToFormat(Compiler, Source->dataType);

    gcmVERIFY_OK(sloCOMPILER_GetBinary(Compiler, &binary));
    if (Source->u.sourceReg.indexMode == gcSL_NOT_INDEXED)
    {
        if (Compiler->context.dumpOptions & slvDUMP_CODE_EMITTER)
        {
            gcmVERIFY_OK(sloCOMPILER_Dump(Compiler,
                                          slvDUMP_CODE_EMITTER,
                                          "gcSHADER_AddSourceOutputIndexedFormattedWithPrecision(Shader,"
                                          " gcSL_SWIZZLE_%s, src = r%d, %s, %s);",
                                          _GetSwizzleName(Source->u.sourceReg.swizzle, buf),
                                          Source->u.sourceReg.regIndex,
                                          _GetIndexModeName(gcSL_NOT_INDEXED),
                                          0,
                                          _GetFormatName(format),
                                          _GetPrecisionName(Source->precision)));
        }

        status = gcSHADER_AddSourceOutputIndexedFormattedWithPrecision(binary,
                                                                       Source->u.sourceReg.u.output,
                                                                       Source->u.sourceReg.swizzle,
                                                                       Source->u.sourceReg.regIndex,
                                                                       gcSL_NOT_INDEXED,
                                                                       0,
                                                                       format,
                                                                       Source->precision);
    }
    else
    {
        if (Compiler->context.dumpOptions & slvDUMP_CODE_EMITTER)
        {
            gcmVERIFY_OK(sloCOMPILER_Dump(Compiler,
                                          slvDUMP_CODE_EMITTER,
                                          "gcSHADER_AddSourceOutputIndexedFormattedWithPrecision(Shader,"
                                          " gcSL_SWIZZLE_%s, src = r%d, %s, index = r%d, %s, %s);",
                                          _GetSwizzleName(Source->u.sourceReg.swizzle, buf),
                                          Source->u.sourceReg.regIndex,
                                          _GetIndexModeName(Source->u.sourceReg.indexMode),
                                          Source->u.sourceReg.indexRegIndex,
                                          _GetFormatName(format),
                                          _GetPrecisionName(Source->precision)));
        }

        status = gcSHADER_AddSourceOutputIndexedFormattedWithPrecision(binary,
                                                                       Source->u.sourceReg.u.output,
                                                                       Source->u.sourceReg.swizzle,
                                                                       Source->u.sourceReg.regIndex,
                                                                       Source->u.sourceReg.indexMode,
                                                                       (gctUINT16)Source->u.sourceReg.indexRegIndex,
                                                                       format,
                                                                       Source->precision);
    }

    if (gcmIS_ERROR(status))
    {
        gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                        LineNo,
                                        StringNo,
                                        slvREPORT_INTERNAL_ERROR,
                                        "failed to add the source attribute"));

        gcmFOOTER();
        return status;
    }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}


static gceSTATUS
_EmitSourceConstantWithFormat(
    IN sloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gcsSOURCE *Source,
    IN gcSL_FORMAT Format
    )
{
    gceSTATUS status;
    gctFLOAT floatConstant[1];
    gctINT32 intConstant[1];
    gctUINT32 uintConstant[1];
    gcSHADER binary;
    void * constantPtr;

    gcmHEADER();

    gcmVERIFY_OK(sloCOMPILER_GetBinary(Compiler, &binary));

#if TREAT_ES20_INTEGER_AS_FLOAT
    if (sloCOMPILER_IsHaltiVersion(Compiler))
    {
#endif
        switch (Format)
        {
        case gcSL_FLOAT:
        case gcSL_FLOAT16:
            floatConstant[0] = Source->u.sourceConstant.u.floatConstant;
            constantPtr = (void *)floatConstant;
            break;

        case gcSL_INTEGER:
        case gcSL_INT16:
        case gcSL_INT8:
            intConstant[0] = Source->u.sourceConstant.u.intConstant;
            constantPtr = (void *)intConstant;
            break;

        case gcSL_UINT32:
        case gcSL_UINT16:
        case gcSL_UINT8:
            uintConstant[0] = Source->u.sourceConstant.u.uintConstant;
            constantPtr = (void *)uintConstant;
            break;

        case gcSL_BOOLEAN:
            intConstant[0] = Source->u.sourceConstant.u.boolConstant;
            constantPtr = (void *)intConstant;
            break;

        default:
            gcmASSERT(0);
            status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
            gcmFOOTER();
            return status;
        }
#if TREAT_ES20_INTEGER_AS_FLOAT
    }
    else
    {
        switch (Format)
        {
        case gcSL_FLOAT:
        case gcSL_FLOAT16:
            floatConstant[0] = Source->u.sourceConstant.u.floatConstant;
            break;

        case gcSL_INTEGER:
        case gcSL_INT16:
        case gcSL_INT8:
            floatConstant[0] = slmI2F(Source->u.sourceConstant.u.intConstant);
            break;

        case gcSL_UINT32:
        case gcSL_UINT16:
        case gcSL_UINT8:
            floatConstant[0] = slmU2F(Source->u.sourceConstant.u.uintConstant);
            break;

        case gcSL_BOOLEAN:
            floatConstant[0] = slmB2F(Source->u.sourceConstant.u.boolConstant);
            break;

        default:
            gcmASSERT(0);
            status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
            gcmFOOTER();
            return status;
        }

        constantPtr = (void *) floatConstant;
        /* Format = gcSL_FLOAT; */
    }
#endif

    if (Compiler->context.dumpOptions & slvDUMP_CODE_EMITTER)
    {
        gcmVERIFY_OK(sloCOMPILER_Dump(Compiler,
                                      slvDUMP_CODE_EMITTER,
                                      "gcSHADER_AddSourceConstantFormattedWithPrecision(Shader, 0x%x, \"%s\", \"%s\");",
                                      constantPtr, _GetFormatName(Format), _GetPrecisionName(Source->precision)));
    }

    status = gcSHADER_AddSourceConstantFormattedWithPrecision(binary, constantPtr, Format, Source->precision);

    if (gcmIS_ERROR(status))
    {
        gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                        LineNo,
                                        StringNo,
                                        slvREPORT_INTERNAL_ERROR,
                                        "failed to add the source constant"));
        gcmFOOTER();
        return status;
    }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

static gceSTATUS
_EmitSourceConstant(
    IN sloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gcsSOURCE *Source
    )
{
    gceSTATUS status;
    gctFLOAT floatConstant[1];
    gctINT32 intConstant[1];
    gctUINT32 uintConstant[1];
    gcSHADER binary;
    void * constantPtr;
    gcSL_FORMAT format;

    gcmHEADER();

    gcmVERIFY_OK(sloCOMPILER_GetBinary(Compiler, &binary));

#if TREAT_ES20_INTEGER_AS_FLOAT
    if (sloCOMPILER_IsHaltiVersion(Compiler))
    {
#endif
        switch (Source->dataType)
        {
        case gcSHADER_FLOAT_X1:
        case gcSHADER_FLOAT_X2:
        case gcSHADER_FLOAT_X3:
        case gcSHADER_FLOAT_X4:
            floatConstant[0] = Source->u.sourceConstant.u.floatConstant;
            constantPtr = (void *)floatConstant;
            format = gcSL_FLOAT;
            break;

        case gcSHADER_INTEGER_X1:
        case gcSHADER_INTEGER_X2:
        case gcSHADER_INTEGER_X3:
        case gcSHADER_INTEGER_X4:
            intConstant[0] = Source->u.sourceConstant.u.intConstant;
            constantPtr = (void *)intConstant;
            format = gcSL_INTEGER;
            break;

        case gcSHADER_UINT_X1:
        case gcSHADER_UINT_X2:
        case gcSHADER_UINT_X3:
        case gcSHADER_UINT_X4:
            uintConstant[0] = Source->u.sourceConstant.u.uintConstant;
            constantPtr = (void *)uintConstant;
            format = gcSL_UINT32;
            break;

        case gcSHADER_BOOLEAN_X1:
        case gcSHADER_BOOLEAN_X2:
        case gcSHADER_BOOLEAN_X3:
        case gcSHADER_BOOLEAN_X4:
            intConstant[0] = Source->u.sourceConstant.u.boolConstant;
            constantPtr = (void *)intConstant;
            format = gcSL_BOOLEAN;
            break;

        default:
            gcmASSERT(0);
            status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
            gcmFOOTER();
            return status;
        }
#if TREAT_ES20_INTEGER_AS_FLOAT
    }
    else
    {
        switch (Source->dataType)
        {
        case gcSHADER_FLOAT_X1:
        case gcSHADER_FLOAT_X2:
        case gcSHADER_FLOAT_X3:
        case gcSHADER_FLOAT_X4:
            floatConstant[0] = Source->u.sourceConstant.u.floatConstant;
            break;

        case gcSHADER_INTEGER_X1:
        case gcSHADER_INTEGER_X2:
        case gcSHADER_INTEGER_X3:
        case gcSHADER_INTEGER_X4:
            floatConstant[0] = slmI2F(Source->u.sourceConstant.u.intConstant);
            break;

        case gcSHADER_UINT_X1:
        case gcSHADER_UINT_X2:
        case gcSHADER_UINT_X3:
        case gcSHADER_UINT_X4:
            floatConstant[0] = slmU2F(Source->u.sourceConstant.u.uintConstant);
            break;

        case gcSHADER_BOOLEAN_X1:
        case gcSHADER_BOOLEAN_X2:
        case gcSHADER_BOOLEAN_X3:
        case gcSHADER_BOOLEAN_X4:
            floatConstant[0] = slmB2F(Source->u.sourceConstant.u.boolConstant);
            break;

        default:
            gcmASSERT(0);
            status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
            gcmFOOTER();
            return status;
        }

        constantPtr = (void *) floatConstant;
        format = gcSL_FLOAT;
    }
#endif
    if (Compiler->context.dumpOptions & slvDUMP_CODE_EMITTER)
    {
        gcmVERIFY_OK(sloCOMPILER_Dump(Compiler,
                                      slvDUMP_CODE_EMITTER,
                                      "gcSHADER_AddSourceConstantFormattedWithPrecision(Shader, 0x%x, \"%s\", \"%s\");",
                                      constantPtr, _GetFormatName(format), _GetPrecisionName(Source->precision)));
    }

    status = gcSHADER_AddSourceConstantFormattedWithPrecision(binary, constantPtr, format, Source->precision);

    if (gcmIS_ERROR(status))
    {
        gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                        LineNo,
                                        StringNo,
                                        slvREPORT_INTERNAL_ERROR,
                                        "failed to add the source constant"));
        gcmFOOTER();
        return status;
    }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}
#else
static gceSTATUS
_EmitSourceTemp(
    IN sloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gctBOOL IsSamplerIndex,
    IN gcsSOURCE *Source
    )
{
    gceSTATUS   status;
    gcSL_FORMAT format;
    gcSHADER    binary;
    gctCHAR    buf[5];

    gcmHEADER();

    gcmASSERT(Source);

    format = slConvDataTypeToFormat(Compiler, Source->dataType);
    gcmVERIFY_OK(sloCOMPILER_GetBinary(Compiler, &binary));
    if (IsSamplerIndex)
    {
        gcmVERIFY_OK(sloCOMPILER_Dump(Compiler,
                                      slvDUMP_CODE_EMITTER,
                                      "gcSHADER_AddSourceSamplerIndexedFormatted(Shader,"
                                      " gcSL_SWIZZLE_%s, %s, %d, %s);",
                                      _GetSwizzleName(Source->u.sourceReg.swizzle, buf),
                                      _GetIndexModeName(Source->u.sourceReg.indexMode),
                                      Source->u.sourceReg.regIndex,
                                      _GetFormatName(format)));

        status = gcSHADER_AddSourceSamplerIndexedFormatted(binary,
                                                           gcSL_SWIZZLE_XYZW,
                                                           gcSL_INDEXED_X,
                                                           (gctUINT16)Source->u.sourceReg.regIndex,
                                                           format);
    }
    else if (Source->u.sourceReg.indexMode == gcSL_NOT_INDEXED)
    {
        gcmVERIFY_OK(sloCOMPILER_Dump(Compiler,
                                      slvDUMP_CODE_EMITTER,
                                      "gcSHADER_AddSource(Shader, %s, %d, gcSL_SWIZZLE_%s, %s);",
                                      _GetTypeName(gcSL_TEMP),
                                      Source->u.sourceReg.regIndex,
                                      _GetSwizzleName(Source->u.sourceReg.swizzle, buf),
                                      _GetFormatName(format)));

        status = gcSHADER_AddSource(binary,
                                    gcSL_TEMP,
                                    (gctUINT16)Source->u.sourceReg.regIndex,
                                    Source->u.sourceReg.swizzle,
                                    format);
    }
    else
    {
        gcmVERIFY_OK(sloCOMPILER_Dump(Compiler,
                                      slvDUMP_CODE_EMITTER,
                                      "gcSHADER_AddSourceIndexed(Shader, %s, %d, gcSL_SWIZZLE_%s, %s, %d, %s);",
                                      _GetTypeName(gcSL_TEMP),
                                      Source->u.sourceReg.regIndex,
                                      _GetSwizzleName(Source->u.sourceReg.swizzle, buf),
                                      _GetIndexModeName(Source->u.sourceReg.indexMode),
                                      Source->u.sourceReg.indexRegIndex,
                                      _GetFormatName(format)));

        status = gcSHADER_AddSourceIndexed(binary,
                                           gcSL_TEMP,
                                           (gctUINT16)Source->u.sourceReg.regIndex,
                                           Source->u.sourceReg.swizzle,
                                           Source->u.sourceReg.indexMode,
                                           (gctUINT16)Source->u.sourceReg.indexRegIndex,
                                           format);
    }

    if (gcmIS_ERROR(status))
    {
        gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                        LineNo,
                                        StringNo,
                                        slvREPORT_INTERNAL_ERROR,
                                        "failed to add the source"));

        gcmFOOTER();
        return status;
    }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

static gceSTATUS
_EmitSourceAttribute(
    IN sloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gcsSOURCE * Source
    )
{
    gceSTATUS    status;
    gcSL_FORMAT format;
    gcSHADER binary;
    gctCHAR  buf[5];

    gcmHEADER();

    gcmASSERT(Source);
    format = slConvDataTypeToFormat(Compiler, Source->dataType);

    gcmVERIFY_OK(sloCOMPILER_GetBinary(Compiler, &binary));
    if (Source->u.sourceReg.indexMode == gcSL_NOT_INDEXED)
    {

    gcmVERIFY_OK(sloCOMPILER_Dump(Compiler,
                      slvDUMP_CODE_EMITTER,
                      "gcSHADER_AddSourceAttributeFormattedWithPrecision(Shader, \"%s\","
                      " gcSL_SWIZZLE_%s, %d, %s);",
                      gcGetAttributeName(binary, Source->u.sourceReg.u.attribute),
                      _GetSwizzleName(Source->u.sourceReg.swizzle, buf),
                      Source->u.sourceReg.regIndex,
                      _GetFormatName(format)));

    status = gcSHADER_AddSourceAttributeFormatted(binary,
                                                      Source->u.sourceReg.u.attribute,
                                                      Source->u.sourceReg.swizzle,
                                                      Source->u.sourceReg.regIndex,
                                                      format);
    }
    else
    {
    gcmVERIFY_OK(sloCOMPILER_Dump(Compiler,
                      slvDUMP_CODE_EMITTER,
                      "gcSHADER_AddSourceAttributeIndexedFormatted(Shader, \"%s\","
                      " gcSL_SWIZZLE_%s, %d, %s, %d, %s);",
                      gcGetAttributeName(binary, Source->u.sourceReg.u.attribute),
                      _GetSwizzleName(Source->u.sourceReg.swizzle, buf),
                      Source->u.sourceReg.regIndex,
                      _GetIndexModeName(Source->u.sourceReg.indexMode),
                      Source->u.sourceReg.indexRegIndex,
                      _GetFormatName(format)));

    status = gcSHADER_AddSourceAttributeIndexedFormatted(binary,
                                                             Source->u.sourceReg.u.attribute,
                                                             Source->u.sourceReg.swizzle,
                                                             Source->u.sourceReg.regIndex,
                                                             Source->u.sourceReg.indexMode,
                                                             (gctUINT16)Source->u.sourceReg.indexRegIndex,
                                                             format);
    }

    if (gcmIS_ERROR(status))
    {
    gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                        LineNo,
                                        StringNo,
                                        slvREPORT_INTERNAL_ERROR,
                                        "failed to add the source attribute"));

        gcmFOOTER();
           return status;
    }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

static gceSTATUS
_EmitSourceUniform(
    IN sloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gcsSOURCE * Source
)
{
    gceSTATUS status;
    gcSL_FORMAT format;
    gcSHADER binary;
    gctCHAR  buf[5];

    gcmHEADER();

    gcmASSERT(Source);

    format = slConvDataTypeToFormat(Compiler, Source->dataType);
    gcmVERIFY_OK(sloCOMPILER_GetBinary(Compiler, &binary));
    if (Source->u.sourceReg.indexMode == gcSL_NOT_INDEXED)
    {
        gcmVERIFY_OK(sloCOMPILER_Dump(Compiler,
                                      slvDUMP_CODE_EMITTER,
                                      "gcSHADER_AddSourceUniformFormatted(Shader, \"%s\","
                                      " gcSL_SWIZZLE_%s, %d, %s);",
                                      gcGetUniformName(Source->u.sourceReg.u.uniform),
                                     _GetSwizzleName(Source->u.sourceReg.swizzle, buf),
                                     Source->u.sourceReg.regIndex,
                                     _GetFormatName(format)));

        status = gcSHADER_AddSourceUniformFormatted(binary,
                                                    Source->u.sourceReg.u.uniform,
                                                    Source->u.sourceReg.swizzle,
                                                    Source->u.sourceReg.regIndex,
                                                    format);
    }
    else
    {
        gcmVERIFY_OK(sloCOMPILER_Dump(Compiler,
                                      slvDUMP_CODE_EMITTER,
                                      "gcSHADER_AddSourceUniformIndexedFormatted(Shader, \"%s\","
                                      " gcSL_SWIZZLE_%s, %d, %s, %d, %s);",
                                      gcGetUniformName(Source->u.sourceReg.u.uniform),
                                      _GetSwizzleName(Source->u.sourceReg.swizzle, buf),
                                      Source->u.sourceReg.regIndex,
                      _GetIndexModeName(Source->u.sourceReg.indexMode),
                                      Source->u.sourceReg.indexRegIndex,
                                      _GetFormatName(format)));

        status = gcSHADER_AddSourceUniformIndexedFormatted(binary,
                                                           Source->u.sourceReg.u.uniform,
                                                           Source->u.sourceReg.swizzle,
                                                           Source->u.sourceReg.regIndex,
                                                           Source->u.sourceReg.indexMode,
                                                           (gctUINT16)Source->u.sourceReg.indexRegIndex,
                                                           format);
    }

    if (gcmIS_ERROR(status))
    {
        gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                        LineNo,
                                        StringNo,
                                        slvREPORT_INTERNAL_ERROR,
                                        "failed to add the source uniform"));

          gcmFOOTER();
           return status;
    }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

static gceSTATUS
_EmitSourceConstant(
    IN sloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gcsSOURCE *Source
    )
{
    gceSTATUS status;
    gctFLOAT floatConstant[1];
    gctINT32 intConstant[1];
    gctUINT32 uintConstant[1];
    gcSHADER binary;
    void * constantPtr;
    gcSL_FORMAT format;

    gcmHEADER();

    gcmVERIFY_OK(sloCOMPILER_GetBinary(Compiler, &binary));

#if TREAT_ES20_INTEGER_AS_FLOAT
    if (sloCOMPILER_IsHaltiVersion(Compiler))
    {
#endif
        switch (Source->dataType)
        {
        case gcSHADER_FLOAT_X1:
        case gcSHADER_FLOAT_X2:
        case gcSHADER_FLOAT_X3:
        case gcSHADER_FLOAT_X4:
            floatConstant[0] = Source->u.sourceConstant.u.floatConstant;
            constantPtr = (void *)floatConstant;
            format = gcSL_FLOAT;
            break;

        case gcSHADER_INTEGER_X1:
        case gcSHADER_INTEGER_X2:
        case gcSHADER_INTEGER_X3:
        case gcSHADER_INTEGER_X4:
            intConstant[0] = Source->u.sourceConstant.u.intConstant;
            constantPtr = (void *)intConstant;
            format = gcSL_INTEGER;
            break;

        case gcSHADER_UINT_X1:
        case gcSHADER_UINT_X2:
        case gcSHADER_UINT_X3:
        case gcSHADER_UINT_X4:
            uintConstant[0] = Source->u.sourceConstant.u.uintConstant;
            constantPtr = (void *)uintConstant;
            format = gcSL_UINT32;
            break;

        case gcSHADER_BOOLEAN_X1:
        case gcSHADER_BOOLEAN_X2:
        case gcSHADER_BOOLEAN_X3:
        case gcSHADER_BOOLEAN_X4:
            intConstant[0] = Source->u.sourceConstant.u.boolConstant;
            constantPtr = (void *)intConstant;
            format = gcSL_BOOLEAN;
            break;

        default:
            gcmASSERT(0);
            status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
            gcmFOOTER();
            return status;
        }
#if TREAT_ES20_INTEGER_AS_FLOAT
    }
    else
    {
        switch (Source->dataType)
        {
        case gcSHADER_FLOAT_X1:
        case gcSHADER_FLOAT_X2:
        case gcSHADER_FLOAT_X3:
        case gcSHADER_FLOAT_X4:
            floatConstant[0] = Source->u.sourceConstant.u.floatConstant;
            break;

        case gcSHADER_INTEGER_X1:
        case gcSHADER_INTEGER_X2:
        case gcSHADER_INTEGER_X3:
        case gcSHADER_INTEGER_X4:
            floatConstant[0] = slmI2F(Source->u.sourceConstant.u.intConstant);
            break;

        case gcSHADER_UINT_X1:
        case gcSHADER_UINT_X2:
        case gcSHADER_UINT_X3:
        case gcSHADER_UINT_X4:
            floatConstant[0] = slmU2F(Source->u.sourceConstant.u.uintConstant);
            break;

        case gcSHADER_BOOLEAN_X1:
        case gcSHADER_BOOLEAN_X2:
        case gcSHADER_BOOLEAN_X3:
        case gcSHADER_BOOLEAN_X4:
            floatConstant[0] = slmB2F(Source->u.sourceConstant.u.boolConstant);
            break;

        default:
            gcmASSERT(0);
            status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
            gcmFOOTER();
            return status;
        }

        constantPtr = (void *) floatConstant;
        format = gcSL_FLOAT;
    }
#endif

    gcmVERIFY_OK(sloCOMPILER_Dump(Compiler,
                                  slvDUMP_CODE_EMITTER,
                                  "gcSHADER_AddSourceConstantFormatted(Shader, 0x%x, \"%s\");",
                                  constantPtr, _GetFormatName(format)));

    status =  gcSHADER_AddSourceConstantFormatted(binary, constantPtr, format);

    if (gcmIS_ERROR(status))
    {
    gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                    LineNo,
                    StringNo,
                    slvREPORT_INTERNAL_ERROR,
                    "failed to add the source constant"));
        gcmFOOTER();
        return status;
    }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}
#endif


static gceSTATUS
_EmitSourceWithModifiers(
    IN sloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gcsSOURCE * Source,
    IN slsASM_MODIFIERS *SourceModifiers
    )
{
    gceSTATUS   status;
    gcSL_FORMAT format;
    gcmHEADER();

    gcmASSERT(Source);

    if (SourceModifiers && SourceModifiers->modifiers[sleASM_MODIFIER_OPND_PRECISION].value != -1)
    {
        Source->precision = (gcSHADER_PRECISION)SourceModifiers->modifiers[sleASM_MODIFIER_OPND_PRECISION].value;
    }

    format = slConvDataTypeToFormat(Compiler, Source->dataType);

    if (SourceModifiers && SourceModifiers->modifiers[sleASM_MODIFIER_OPND_FORMAT].value != -1)
    {
        format = (gcSL_FORMAT)SourceModifiers->modifiers[sleASM_MODIFIER_OPND_FORMAT].value;
    }

    switch (Source->type)
    {
    case gcvSOURCE_TEMP:
        status = _EmitSourceTempWithFormat(Compiler,
                                 LineNo,
                                 StringNo,
                                 gcIsSamplerDataType(Source->dataType),
                                 Source,
                                 format);
        break;

    case gcvSOURCE_VERTEX_ID:
    case gcvSOURCE_INSTANCE_ID:
        status = _EmitSourceTempWithFormat(Compiler,
                                 LineNo,
                                 StringNo,
                                 gcvFALSE,
                                 Source,
                                 format);
        break;

   case gcvSOURCE_ATTRIBUTE:
        status = _EmitSourceAttributeWithFormat(Compiler,
                                      LineNo,
                                      StringNo,
                                      Source,
                                      format);
        break;

   case gcvSOURCE_UNIFORM:
       {
           gcSHADER_TYPE type = Source->u.sourceReg.u.uniform->u.type;

           if ((type == gcSHADER_IMAGE_2D || type == gcSHADER_IMAGE_3D) ||
               (type >= gcSHADER_IIMAGE_2D && type <= gcSHADER_UIMAGE_2D_ARRAY))
           {
               format = slConvImageFormatToFormat(Compiler, Source->u.sourceReg.u.uniform->imageFormat);
           }
           else
           {
               format = slConvDataTypeToFormat(Compiler, Source->dataType);
           }

           status = _EmitSourceUniformWithFormat(Compiler,
               LineNo,
               StringNo,
               Source,
               format);
       }
        break;

   case gcvSOURCE_CONSTANT:
        status = _EmitSourceConstantWithFormat(Compiler,
                                     LineNo,
                                     StringNo,
                                     Source,
                                     format);
        break;

    case gcvSOURCE_TARGET_FORMAT:
    {
        gcSHADER binary;

        gcmVERIFY_OK(sloCOMPILER_GetBinary(Compiler, &binary));

        if (Compiler->context.dumpOptions & slvDUMP_CODE_EMITTER)
        {
            gcmVERIFY_OK(sloCOMPILER_Dump(Compiler,
                                          slvDUMP_CODE_EMITTER,
                                          "gcSHADER_AddSourceConstantFormatted(binary, 0x%x, \"%s\");",
                                          format, _GetFormatName(gcSL_UINT32)));
        }

        status = gcSHADER_AddSourceConstantFormatted(binary, &format, gcSL_UINT32);

        if (gcmIS_ERROR(status))
        {
            gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                            LineNo,
                                            StringNo,
                                            slvREPORT_INTERNAL_ERROR,
                                            "failed to add the source constant"));
            gcmFOOTER();
            return status;
        }
        break;
    }

    default:
        gcmASSERT(0);
        status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
        gcmFOOTER();
        return status;
    }

    /* Unsupported currently. */
    if (SourceModifiers && SourceModifiers->modifiers[sleASM_MODIFIER_OPND_NEG].value != -1)
    {
        /*
        *source = gcmSL_SOURCE_SET(*source, Neg, 1);
        */
    }

    /* Unsupported currently. */
    if (SourceModifiers && SourceModifiers->modifiers[sleASM_MODIFIER_OPND_ABS].value != -1)
    {
        /*
        *source = gcmSL_SOURCE_SET(*source, Abs, 1);
        */
    }

    gcmFOOTER();
    return status;
}

static gceSTATUS
_EmitSource(
    IN sloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gcsSOURCE * Source
    )
{
    gceSTATUS   status;

    gcmHEADER();

    gcmASSERT(Source);

    switch (Source->type)
    {
    case gcvSOURCE_TEMP:
        status = _EmitSourceTemp(Compiler,
                                LineNo,
                                StringNo,
                                gcIsSamplerDataType(Source->dataType),
                                Source);
        break;

    case gcvSOURCE_VERTEX_ID:
    case gcvSOURCE_INSTANCE_ID:
        status = _EmitSourceTemp(Compiler,
                                LineNo,
                                StringNo,
                                gcvFALSE,
                                Source);
        break;

    case gcvSOURCE_ATTRIBUTE:
        status = _EmitSourceAttribute(Compiler,
                                    LineNo,
                                    StringNo,
                                    Source);
        break;

    case gcvSOURCE_UNIFORM:
        status = _EmitSourceUniform(Compiler,
                                    LineNo,
                                    StringNo,
                                    Source);
        break;

    case gcvSOURCE_OUTPUT:
        status = _EmitSourceOutput(Compiler,
                                    LineNo,
                                    StringNo,
                                    Source);
        break;

    case gcvSOURCE_CONSTANT:
        status = _EmitSourceConstant(Compiler,
                                    LineNo,
                                    StringNo,
                                    Source);
        break;

    case gcvSOURCE_TARGET_FORMAT:
        {
            gctUINT32 format[1];
            gcSHADER binary;

            gcmVERIFY_OK(sloCOMPILER_GetBinary(Compiler, &binary));

            format[0] = slConvDataTypeToFormat(Compiler, Source->dataType);

            if (Compiler->context.dumpOptions & slvDUMP_CODE_EMITTER)
            {
                gcmVERIFY_OK(sloCOMPILER_Dump(Compiler,
                                            slvDUMP_CODE_EMITTER,
                                            "gcSHADER_AddSourceConstantFormatted(binary, 0x%x, \"%s\");",
                                            format, _GetFormatName(gcSL_UINT32)));
            }

            status = gcSHADER_AddSourceConstantFormatted(binary, format, gcSL_UINT32);

            if (gcmIS_ERROR(status))
            {
                gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                                LineNo,
                                                StringNo,
                                                slvREPORT_INTERNAL_ERROR,
                                                "failed to add the source constant"));
                gcmFOOTER();
                return status;
            }
            break;
        }

    default:
        gcmASSERT(0);
        status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
        gcmFOOTER();
        return status;
    }

    gcmFOOTER();
    return status;
}

gceSTATUS
slNewAttributeWithLocation(
    IN sloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gctCONST_STRING Name,
    IN gcSHADER_TYPE DataType,
    IN gcSHADER_PRECISION Precision,
    IN gctUINT Length,
    IN gctUINT ArrayLengthCount,
    IN gctBOOL IsTexture,
    IN gcSHADER_SHADERMODE ShaderMode,
    IN gctINT Location,
    IN gctINT FieldIndex,
    IN gctBOOL IsInvariant,
    IN gctBOOL IsPrecise,
    OUT gcATTRIBUTE * Attribute
    )
{
    gceSTATUS status;

    gcmHEADER();

    if (Compiler->context.dumpOptions & slvDUMP_CODE_EMITTER)
    {
        gcmVERIFY_OK(sloCOMPILER_Dump(Compiler,
                                      slvDUMP_CODE_EMITTER,
                                      "attribute line=%d string=%d name=\"%s\""
                                      "dataType=%s length=%d lengthCount=%d",
                                      LineNo,
                                      StringNo,
                                      Name,
                                      gcGetDataTypeName(DataType),
                                      Length,
                                      ArrayLengthCount));
    }

    sloCOMPILER_IncrDumpOffset(Compiler);

    status = _AddAttributeWithLocation(Compiler,
                                       Name,
                                       DataType,
                                       Precision,
                                       Length,
                                       ArrayLengthCount,
                                       IsTexture,
                                       ShaderMode,
                                       Location,
                                       FieldIndex,
                                       IsInvariant,
                                       IsPrecise,
                                       Attribute);

    if (gcmIS_ERROR(status))
    {
        gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                        LineNo,
                                        StringNo,
                                        slvREPORT_INTERNAL_ERROR,
                                        "failed to add the attribute"));
        gcmFOOTER();
        return status;
    }

    sloCOMPILER_DecrDumpOffset(Compiler);

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
slNewUniform(
    IN sloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gctCONST_STRING Name,
    IN gcsSHADER_VAR_INFO *UniformInfo,
    OUT gctINT16* ThisUniformIndex,
    OUT gcUNIFORM * Uniform
    )
{
    gceSTATUS status;

    gcmHEADER();

    if (Compiler->context.dumpOptions & slvDUMP_CODE_EMITTER)
    {
        gcmVERIFY_OK(sloCOMPILER_Dump(Compiler,
                          slvDUMP_CODE_EMITTER,
                          "uniform line=%d string=%d name=\"%s\""
                          " dataType=%s length=%d",
                          LineNo,
                          StringNo,
                          Name,
                          gcGetDataTypeName(UniformInfo->type),
                          UniformInfo->arraySize));
    }

    sloCOMPILER_IncrDumpOffset(Compiler);

    status = _AddUniform(Compiler,
                             Name,
                             UniformInfo,
                             ThisUniformIndex,
                             Uniform);

    if (gcmIS_ERROR(status))
    {
        gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                        LineNo,
                        StringNo,
                        slvREPORT_INTERNAL_ERROR,
                        "failed to add the uniform"));

            gcmFOOTER();
            return status;
    }

    sloCOMPILER_DecrDumpOffset(Compiler);

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
slGetUniformSamplerIndex(
    IN sloCOMPILER Compiler,
    IN gcUNIFORM UniformSampler,
    OUT gctREG_INDEX * Index
    )
{
    gceSTATUS    status;
    gctUINT32    sampler;

    gcmHEADER();

    status = _GetSampler(Compiler, UniformSampler, &sampler);

    if (gcmIS_ERROR(status))
    {
        gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                        0,
                                        0,
                                        slvREPORT_INTERNAL_ERROR,
                                        "failed to get the uniform index"));

        gcmFOOTER();
        return status;
    }

    *Index = (gctREG_INDEX)sampler;

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
slNewOutputWithLocation(
    IN sloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gctCONST_STRING Name,
    IN gcSHADER_TYPE DataType,
    IN gcSHADER_PRECISION Precision,
    IN gctBOOL IsArray,
    IN gctUINT Length,
    IN gctREG_INDEX TempRegIndex,
    IN gctINT Location,
    IN gctINT FieldIndex,
    IN gctBOOL IsInvariant,
    IN gctBOOL IsPrecise,
    IN gcSHADER_SHADERMODE ShaderMode,
    IN gceLAYOUT_QUALIFIER LayoutQual,
    OUT gcOUTPUT * Output
    )
{
    gceSTATUS    status;
    gctUINT      i;
    gctUINT      rows;

    gcmHEADER();

    if (Compiler->context.dumpOptions & slvDUMP_CODE_EMITTER)
    {
        gcmVERIFY_OK(sloCOMPILER_Dump(Compiler,
                          slvDUMP_CODE_EMITTER,
                          "output line=%d string=%d name=\"%s\""
                          " dataType=%s length=%d tempRegIndex=%d",
                          LineNo,
                          StringNo,
                          Name,
                          gcGetDataTypeName(DataType),
                          Length,
                          TempRegIndex));
    }

    sloCOMPILER_IncrDumpOffset(Compiler);

    status = _AddOutputWithLocation(Compiler,
                                    Name,
                                    DataType,
                                    Precision,
                                    IsArray,
                                    Length,
                                    TempRegIndex,
                                    ShaderMode,
                                    Location,
                                    FieldIndex,
                                    IsInvariant,
                                    IsPrecise,
                                    LayoutQual,
                                    Output);

    if (gcmIS_ERROR(status))
    {
        gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                        LineNo,
                                        StringNo,
                                        slvREPORT_INTERNAL_ERROR,
                                        "failed to add the output"));

            gcmFOOTER();
            return status;
    }

    rows = gcGetDataTypeSize(DataType);
    for (i = 1; i < Length; i++)
    {
        status = _AddOutputIndexed(Compiler,
                                   Name,
                                   i,
                                   TempRegIndex + (i*rows));

        if (gcmIS_ERROR(status))
        {
            gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                            LineNo,
                                            StringNo,
                                            slvREPORT_INTERNAL_ERROR,
                                            "failed to add the indexed output"));

            gcmFOOTER();
            return status;
        }
    }

    sloCOMPILER_DecrDumpOffset(Compiler);

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
slNewVariable(
    IN sloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gctCONST_STRING Name,
    IN gctREG_INDEX TempRegIndex,
    IN gcsSHADER_VAR_INFO *VarInfo,
    OUT gctINT16* ThisVarIndex
    )
{
    gceSTATUS status;

    gcmHEADER();

    if (Compiler->context.dumpOptions & slvDUMP_CODE_EMITTER)
    {
        gcmVERIFY_OK(
            sloCOMPILER_Dump(Compiler,
                    slvDUMP_CODE_EMITTER,
                    "variable line=%d string=%d name=\"%s\" "
                    "dataType=%s length=%d tempRegIndex=%d",
                    LineNo,
                    StringNo,
                    Name,
                    gcGetDataTypeName(VarInfo->type),
                    VarInfo->arraySize,
                    TempRegIndex));
    }

    sloCOMPILER_IncrDumpOffset(Compiler);

    status = _AddVariable(Compiler,
                              Name,
                              TempRegIndex,
                              VarInfo,
                              ThisVarIndex);

    if (gcmIS_ERROR(status))
    {
        gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                        LineNo,
                                        StringNo,
                                        slvREPORT_INTERNAL_ERROR,
                                        "failed to add the variable"));

        gcmFOOTER();
        return status;
    }

    sloCOMPILER_DecrDumpOffset(Compiler);

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
slUpdateVariableTempReg(
    IN sloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gctUINT varIndex,
    IN gctREG_INDEX newTempRegIndex
    )
{
    gceSTATUS status;

    gcmHEADER();

    status = _UpdateVariableTempReg(Compiler, varIndex, newTempRegIndex);

    if (gcmIS_ERROR(status))
    {
        gcmVERIFY_OK(
            sloCOMPILER_Report(Compiler,
                               LineNo,
                               StringNo,
                               slvREPORT_INTERNAL_ERROR,
                               "failed to update the variable"));

        gcmFOOTER();
        return status;
    }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
slSetLabel(
    IN sloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gctLABEL Label
    )
{
    gceSTATUS            status;
    sloCODE_EMITTER        codeEmitter;

    gcmHEADER();

    codeEmitter = Compiler->codeEmitter;
    gcmASSERT(codeEmitter);

    status = sloCODE_EMITTER_NewBasicBlock(Compiler, codeEmitter);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    if (LineNo != 0)
    {
        gcmVERIFY_OK(sloCOMPILER_Dump(
                                    Compiler,
                                    slvDUMP_CODE_EMITTER,
                                    "<LABEL line=\"%d\" string=\"%d\" no=\"%d\">",
                                    LineNo,
                                    StringNo,
                                    Label));
    }
    else
    {
        gcmVERIFY_OK(sloCOMPILER_Dump(
                                    Compiler,
                                    slvDUMP_CODE_EMITTER,
                                    "<LABEL no=\"%d\">",
                                    Label));
    }

    status = _AddLabel(Compiler, Label);

    gcmVERIFY_OK(sloCOMPILER_Dump(
                                Compiler,
                                slvDUMP_CODE_EMITTER,
                                "</LABEL>"));

    if (gcmIS_ERROR(status))
    {
        gcmVERIFY_OK(sloCOMPILER_Report(
                                        Compiler,
                                        LineNo,
                                        StringNo,
                                        slvREPORT_INTERNAL_ERROR,
                                        "failed to add the label"));

        gcmFOOTER();
        return status;
    }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gctUINT8
gcGetDefaultEnable(
    IN gcSHADER_TYPE DataType
    )
{
    switch (DataType)
    {
    case gcSHADER_FLOAT_X1:
    case gcSHADER_BOOLEAN_X1:
    case gcSHADER_UINT_X1:
    case gcSHADER_INTEGER_X1:
    case gcSHADER_ATOMIC_UINT:
        return gcSL_ENABLE_X;

    case gcSHADER_FLOAT_X2:
    case gcSHADER_FLOAT_2X2:
    case gcSHADER_FLOAT_3X2:
    case gcSHADER_FLOAT_4X2:
    case gcSHADER_BOOLEAN_X2:
    case gcSHADER_UINT_X2:
    case gcSHADER_INTEGER_X2:
        return gcSL_ENABLE_XY;

    case gcSHADER_FLOAT_X3:
    case gcSHADER_FLOAT_2X3:
    case gcSHADER_FLOAT_3X3:
    case gcSHADER_FLOAT_4X3:
    case gcSHADER_BOOLEAN_X3:
    case gcSHADER_UINT_X3:
    case gcSHADER_INTEGER_X3:
        return gcSL_ENABLE_XYZ;

    case gcSHADER_FLOAT_X4:
    case gcSHADER_FLOAT_2X4:
    case gcSHADER_FLOAT_3X4:
    case gcSHADER_FLOAT_4X4:
    case gcSHADER_BOOLEAN_X4:
    case gcSHADER_UINT_X4:
    case gcSHADER_INTEGER_X4:
        return gcSL_ENABLE_XYZW;

    case gcSHADER_ISAMPLER_2D:
    case gcSHADER_ISAMPLER_3D:
    case gcSHADER_ISAMPLER_BUFFER:
    case gcSHADER_ISAMPLER_CUBIC:
    case gcSHADER_ISAMPLER_CUBEMAP_ARRAY:
    case gcSHADER_ISAMPLER_2D_ARRAY:

    case gcSHADER_USAMPLER_2D:
    case gcSHADER_USAMPLER_3D:
    case gcSHADER_USAMPLER_BUFFER:
    case gcSHADER_USAMPLER_CUBIC:
    case gcSHADER_USAMPLER_CUBEMAP_ARRAY:
    case gcSHADER_USAMPLER_2D_ARRAY:

    case gcSHADER_SAMPLER_1D:
    case gcSHADER_SAMPLER_2D:
    case gcSHADER_SAMPLER_3D:
    case gcSHADER_SAMPLER_BUFFER:
    case gcSHADER_SAMPLER_CUBIC:
    case gcSHADER_SAMPLER_CUBEMAP_ARRAY:
    case gcSHADER_SAMPLER_2D_SHADOW:
    case gcSHADER_SAMPLER_CUBE_SHADOW:
    case gcSHADER_SAMPLER_CUBEMAP_ARRAY_SHADOW:
    case gcSHADER_SAMPLER_1D_ARRAY:
    case gcSHADER_SAMPLER_1D_ARRAY_SHADOW:
    case gcSHADER_SAMPLER_2D_ARRAY:
    case gcSHADER_SAMPLER_2D_ARRAY_SHADOW:
    case gcSHADER_SAMPLER_EXTERNAL_OES:

    case gcSHADER_SAMPLER_2D_MS:
    case gcSHADER_ISAMPLER_2D_MS:
    case gcSHADER_USAMPLER_2D_MS:
    case gcSHADER_SAMPLER_2D_MS_ARRAY:
    case gcSHADER_ISAMPLER_2D_MS_ARRAY:
    case gcSHADER_USAMPLER_2D_MS_ARRAY:

        return gcSL_ENABLE_X;

    case gcSHADER_IMAGE_2D:                      /* 0x17 */
    case gcSHADER_IIMAGE_2D:                     /* 0x39 */
    case gcSHADER_UIMAGE_2D:                     /* 0x3A */
    case gcSHADER_IMAGE_3D:                      /* 0x18 */
    case gcSHADER_IIMAGE_3D:                     /* 0x3B */
    case gcSHADER_UIMAGE_3D:                     /* 0x3C */
    case gcSHADER_IMAGE_CUBE:                    /* 0x3D */
    case gcSHADER_IIMAGE_CUBE:                   /* 0x3E */
    case gcSHADER_UIMAGE_CUBE:                   /* 0x3F */
    case gcSHADER_IMAGE_2D_ARRAY:                /* 0x40 */
    case gcSHADER_IIMAGE_2D_ARRAY:               /* 0x41 */
    case gcSHADER_UIMAGE_2D_ARRAY:               /* 0x42 */
    case gcSHADER_IMAGE_CUBEMAP_ARRAY:           /* 0x51 */
    case gcSHADER_IIMAGE_CUBEMAP_ARRAY:          /* 0x52 */
    case gcSHADER_UIMAGE_CUBEMAP_ARRAY:          /* 0x53 */
    case gcSHADER_IMAGE_BUFFER:
    case gcSHADER_IIMAGE_BUFFER:
    case gcSHADER_UIMAGE_BUFFER:

        return gcSL_ENABLE_XYZW;

    default:
        gcmASSERT(0);
        return gcSL_ENABLE_XYZW;
    }
}

gctUINT8
gcGetVectorComponentEnable(
    IN gctUINT8 Enable,
    IN gctUINT8 Component
    )
{
    gctUINT8        i;
    gctUINT8        enables[4] = {0};

    for (i = 0; i < 4; i++)
    {
        if (Enable & gcSL_ENABLE_X)
        {
            enables[i]    = gcSL_ENABLE_X;
            Enable        &= ~gcSL_ENABLE_X;
        }
        else if (Enable & gcSL_ENABLE_Y)
        {
            enables[i]    = gcSL_ENABLE_Y;
            Enable        &= ~gcSL_ENABLE_Y;
        }
        else if (Enable & gcSL_ENABLE_Z)
        {
            enables[i]    = gcSL_ENABLE_Z;
            Enable        &= ~gcSL_ENABLE_Z;
        }
        else if (Enable & gcSL_ENABLE_W)
        {
            enables[i]    = gcSL_ENABLE_W;
            Enable        &= ~gcSL_ENABLE_W;
        }
        else
        {
            break;
        }
    }

    gcmASSERT(Component < i);

    if(Component < i)
        return enables[Component];
    else
        return 0;
}

gctUINT8
gcGetDefaultSwizzle(
    IN gcSHADER_TYPE DataType
    )
{
    switch (DataType)
    {
    case gcSHADER_FLOAT_X1:
    case gcSHADER_BOOLEAN_X1:
    case gcSHADER_UINT_X1:
    case gcSHADER_INTEGER_X1:
    case gcSHADER_ATOMIC_UINT:
        return gcSL_SWIZZLE_XXXX;

    case gcSHADER_FLOAT_X2:
    case gcSHADER_BOOLEAN_X2:
    case gcSHADER_UINT_X2:
    case gcSHADER_INTEGER_X2:
        return gcSL_SWIZZLE_XYYY;

    case gcSHADER_FLOAT_X3:
    case gcSHADER_BOOLEAN_X3:
    case gcSHADER_UINT_X3:
    case gcSHADER_INTEGER_X3:
        return gcSL_SWIZZLE_XYZZ;

    case gcSHADER_FLOAT_X4:
    case gcSHADER_BOOLEAN_X4:
    case gcSHADER_UINT_X4:
    case gcSHADER_INTEGER_X4:
    case gcSHADER_SAMPLER_1D:
    case gcSHADER_SAMPLER_2D:
    case gcSHADER_SAMPLER_3D:
    case gcSHADER_SAMPLER_BUFFER:
    case gcSHADER_SAMPLER_CUBIC:
    case gcSHADER_SAMPLER_2D_SHADOW:
    case gcSHADER_SAMPLER_CUBE_SHADOW:

    case gcSHADER_SAMPLER_1D_ARRAY:
    case gcSHADER_SAMPLER_1D_ARRAY_SHADOW:
    case gcSHADER_SAMPLER_2D_ARRAY:
    case gcSHADER_SAMPLER_2D_ARRAY_SHADOW:

    case gcSHADER_ISAMPLER_2D:
    case gcSHADER_ISAMPLER_3D:
    case gcSHADER_ISAMPLER_BUFFER:
    case gcSHADER_ISAMPLER_CUBIC:
    case gcSHADER_ISAMPLER_2D_ARRAY:

    case gcSHADER_USAMPLER_2D:
    case gcSHADER_USAMPLER_3D:
    case gcSHADER_USAMPLER_BUFFER:
    case gcSHADER_USAMPLER_CUBIC:
    case gcSHADER_USAMPLER_2D_ARRAY:

    case gcSHADER_SAMPLER_EXTERNAL_OES:

    case gcSHADER_SAMPLER_2D_MS:
    case gcSHADER_ISAMPLER_2D_MS:
    case gcSHADER_USAMPLER_2D_MS:
    case gcSHADER_SAMPLER_2D_MS_ARRAY:
    case gcSHADER_ISAMPLER_2D_MS_ARRAY:
    case gcSHADER_USAMPLER_2D_MS_ARRAY:

    case gcSHADER_IMAGE_2D:                      /* 0x17 */
    case gcSHADER_IIMAGE_2D:                     /* 0x39 */
    case gcSHADER_UIMAGE_2D:                     /* 0x3A */
    case gcSHADER_IMAGE_3D:                      /* 0x18 */
    case gcSHADER_IIMAGE_3D:                     /* 0x3B */
    case gcSHADER_UIMAGE_3D:                     /* 0x3C */
    case gcSHADER_IMAGE_CUBE:                    /* 0x3D */
    case gcSHADER_IIMAGE_CUBE:                   /* 0x3E */
    case gcSHADER_UIMAGE_CUBE:                   /* 0x3F */
    case gcSHADER_IMAGE_2D_ARRAY:                /* 0x40 */
    case gcSHADER_IIMAGE_2D_ARRAY:               /* 0x41 */
    case gcSHADER_UIMAGE_2D_ARRAY:               /* 0x42 */
    case gcSHADER_IMAGE_BUFFER:
    case gcSHADER_IIMAGE_BUFFER:
    case gcSHADER_UIMAGE_BUFFER:
        return gcSL_SWIZZLE_XYZW;

    default:
        gcmASSERT(0);
        return gcSL_SWIZZLE_XYZW;
    }
}

gctUINT8
gcGetVectorComponentSwizzle(
    IN gctUINT8 Swizzle,
    IN gctUINT8 Component
    )
{
    gctUINT8 value = 0;

    switch (Component)
    {
    case 0:
        value = (Swizzle >> 0) & 3;
        break;

    case 1:
        value = (Swizzle >> 2) & 3;
        break;

    case 2:
        value = (Swizzle >> 4) & 3;
        break;

    case 3:
        value = (Swizzle >> 6) & 3;
        break;

    default:
        gcmASSERT(0);
    }

    return value | (value << 2) | (value << 4) | (value << 6);
}

static gceSTATUS
_EmitCode(
    IN sloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gcSL_OPCODE Opcode,
    IN gcsTARGET * Target,
    IN gcsSOURCE * Source0,
    IN gcsSOURCE * Source1
    );

static gceSTATUS
_MakeNewSource(
    IN sloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gcsSOURCE * Source,
    OUT gcsSOURCE * NewSource
    )
{
    gceSTATUS           status;
    gcsTARGET           tempTarget;
    sloCODE_EMITTER     codeEmitter;

    gcmHEADER();

    codeEmitter = Compiler->codeEmitter;
    gcmASSERT(codeEmitter);

    /* flush out previous instructions */
    status = sloCODE_EMITTER_EndBasicBlock(Compiler, codeEmitter);
    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcsTARGET_Initialize(&tempTarget,
                             Source->dataType,
                             Source->precision,
                             slNewTempRegs(Compiler, 1),
                             gcSL_ENABLE_XYZW,
                             gcSL_NOT_INDEXED,
                             0);

    status = _EmitCode(
                    Compiler,
                    LineNo,
                    StringNo,
                    gcSL_MOV,
                    &tempTarget,
                    Source,
                    gcvNULL);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcsSOURCE_InitializeTempReg(
                                NewSource,
                                Source->dataType,
                                Source->precision,
                                tempTarget.tempRegIndex,
                                gcSL_SWIZZLE_XYZW,
                                gcSL_NOT_INDEXED,
                                0);

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

static gceSTATUS
_MakeNewSourceForUniformInUBO(
    IN sloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gcsSOURCE * Source,
    OUT gcsSOURCE * NewSource
    )
{
    gceSTATUS status;
    gcsTARGET tempTarget;
    gcsSOURCE tempSource;
    gcSHADER_TYPE dataType;
    sloCODE_EMITTER codeEmitter;

    gcmHEADER();

    gcmASSERT(Source->type == gcvSOURCE_UNIFORM);

    codeEmitter = Compiler->codeEmitter;
    gcmASSERT(codeEmitter);

    /* flush out previous instructions */
    status = sloCODE_EMITTER_EndBasicBlock(Compiler, codeEmitter);
    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    dataType = Source->u.sourceReg.u.uniform->u.type;
    if(gcIsMatrixDataType(dataType))
    {
        dataType = gcGetMatrixColumnDataType(dataType);
    }

    gcsTARGET_Initialize(&tempTarget,
                         dataType,
                         Source->precision,
                         slNewTempRegs(Compiler, 1),
                         gcSL_ENABLE_XYZW,
                         gcSL_NOT_INDEXED,
                         0);

    tempSource = *Source;
    tempSource.u.sourceReg.swizzle = gcGetDefaultSwizzle(dataType);

    status = _EmitCode(Compiler,
                       LineNo,
                       StringNo,
                       gcSL_MOV,
                       &tempTarget,
                       &tempSource,
                       gcvNULL);
    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcsSOURCE_InitializeTempReg(NewSource,
                                Source->dataType,
                                Source->precision,
                                tempTarget.tempRegIndex,
                                Source->u.sourceReg.swizzle,
                                gcSL_NOT_INDEXED,
                                0);

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

static gceSTATUS
_PrepareSource(
    IN sloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gcsTARGET * Target,
    IN gcsSOURCE * Source,
    OUT gcsSOURCE * NewSource
    )
{
    gceSTATUS    status;
    sloCODE_GENERATOR codeGenerator = Compiler->codeGenerator;

    gcmHEADER();

    gcmASSERT(Source);
    gcmASSERT(NewSource);

    if (codeGenerator->createDefaultUBO
        && Source->type == gcvSOURCE_UNIFORM
        && !gcIsSamplerDataType(Source->u.sourceReg.u.uniform->u.type)
        && !isUniformBlockAddress(Source->u.sourceReg.u.uniform))
    {
        status = _MakeNewSourceForUniformInUBO(Compiler,
                                               LineNo,
                                               StringNo,
                                               Source,
                                               NewSource);
        if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }
    }
    else
    {
        gctBOOL insertAssign;

        if (Target != gcvNULL)
        {
            insertAssign = (Source->type == gcvSOURCE_TEMP
                            && Target->tempRegIndex == Source->u.sourceReg.regIndex);
        }
        else
        {
            insertAssign = (Source->type == gcvSOURCE_UNIFORM);
        }

        if (insertAssign)
        {
            status = _MakeNewSource(Compiler,
                                    LineNo,
                                    StringNo,
                                    Source,
                                    NewSource);
            if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }
        }
        else
        {
            *NewSource = *Source;
        }
    }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

static gceSTATUS
_PrepareAnotherSource(
    IN sloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gcsTARGET * Target,
    IN gcsSOURCE * Source0,
    IN gcsSOURCE * Source1,
    OUT gcsSOURCE * NewSource1
    )
{
    gceSTATUS    status;
    sloCODE_GENERATOR codeGenerator = Compiler->codeGenerator;

    gcmHEADER();

    gcmASSERT(Source0);
    gcmASSERT(Source1);
    gcmASSERT(NewSource1);

    if (codeGenerator->createDefaultUBO
        && Source1->type == gcvSOURCE_UNIFORM
        && !gcIsSamplerDataType(Source1->u.sourceReg.u.uniform->u.type)
        && !isUniformBlockAddress(Source1->u.sourceReg.u.uniform))
    {
        status = _MakeNewSourceForUniformInUBO(Compiler,
                                               LineNo,
                                               StringNo,
                                               Source1,
                                               NewSource1);
        if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }
    }
    else {
        gctBOOL insertAssign;

        insertAssign =
            (Source1->type == gcvSOURCE_CONSTANT && Source0->type == gcvSOURCE_UNIFORM)
            || (Source1->type == gcvSOURCE_UNIFORM && Source0->type == gcvSOURCE_CONSTANT)
            || (Source1->type == gcvSOURCE_UNIFORM && Source0->type == gcvSOURCE_UNIFORM
                && (Source1->u.sourceReg.u.uniform != Source0->u.sourceReg.u.uniform
                    || Source1->u.sourceReg.regIndex != Source0->u.sourceReg.regIndex));

        if (Target != gcvNULL)
        {
            insertAssign =
                (insertAssign
                || (Source1->type == gcvSOURCE_TEMP
                    && Target->tempRegIndex == Source1->u.sourceReg.regIndex));
        }
        {
            gctBOOL     useFullNewLinker = gcvFALSE;
            gctBOOL     hasHalti2 = GetHWHasHalti2();

            useFullNewLinker = gcUseFullNewLinker(hasHalti2);

            if (useFullNewLinker && insertAssign)
            {
                insertAssign = gcvFALSE;
            }
        }
        if (insertAssign)
        {
            status = _MakeNewSource(Compiler,
                                    LineNo,
                                    StringNo,
                                    Source1,
                                    NewSource1);
            if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }
        }
        else
        {
            *NewSource1 = *Source1;
        }
    }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

static gcSL_OPCODE
_ConvOpcode(
    sleOPCODE opcode
    )
{
    switch (opcode)
    {
    case slvOPCODE_ASSIGN:                  return gcSL_MOV;
    case slvOPCODE_CONV:                    return gcSL_CONV;

    case slvOPCODE_ADD:                     return gcSL_ADD;
    case slvOPCODE_SAMPLER_ASSIGN:          return gcSL_SAMPLER_ASSIGN;
    case slvOPCODE_SUB:                     return gcSL_SUB;
    case slvOPCODE_MUL:                     return gcSL_MUL;
    case slvOPCODE_MULHI:                   return gcSL_MULHI;
    case slvOPCODE_DIV:                     gcmASSERT(0); return gcSL_NOP;
    case slvOPCODE_IDIV:                    gcmASSERT(0); return gcSL_NOP;
#if !_GEN_MOD_IN_BACKEND
    case slvOPCODE_MOD:                     gcmASSERT(0); return gcSL_NOP;
#else
    case slvOPCODE_MOD:                     return gcSL_MOD;
#endif

    case slvOPCODE_TEXTURE_LOAD:            return gcSL_TEXLD;
    case slvOPCODE_TEXTURE_LOAD_U:          return gcSL_TEXLD_U;
    case slvOPCODE_TEXTURE_LOAD_PROJ:       return gcSL_TEXLDPROJ;
    case slvOPCODE_TEXTURE_LOAD_PCF:        return gcSL_TEXLDPCF;
    case slvOPCODE_TEXTURE_LOAD_PCFPROJ:    return gcSL_TEXLDPCFPROJ;
    case slvOPCODE_TEXTURE_BIAS:            return gcSL_TEXBIAS;
    case slvOPCODE_TEXTURE_GRAD:            return gcSL_TEXGRAD;
    case slvOPCODE_TEXTURE_LOD:             return gcSL_TEXLOD;
    case slvOPCODE_TEXTURE_GATHER:          return gcSL_TEXGATHER;
    case slvOPCODE_TEXTURE_FETCH_MS:        return gcSL_TEXFETCH_MS;
    case slvOPCODE_TEXTURE_U:               return gcSL_TEXU;
    case slvOPCODE_TEXTURE_U_LOD:           return gcSL_TEXU_LOD;

    case slvOPCODE_FLOAT_TO_INT:            return gcSL_NOP;
    case slvOPCODE_FLOAT_TO_BOOL:           return gcSL_NOP;
    case slvOPCODE_FLOAT_TO_HALF:           return gcSL_NOP;
    case slvOPCODE_HALF_TO_FLOAT:           return gcSL_NOP;
    case slvOPCODE_INT_TO_BOOL:             return gcSL_NOP;

    case slvOPCODE_FLOAT_TO_UINT:           return gcSL_F2I;
    case slvOPCODE_UINT_TO_BOOL:            return gcSL_NOP;
    case slvOPCODE_INT_TO_UINT:             return gcSL_CONV;
    case slvOPCODE_INT_TO_FLOAT:            return gcSL_I2F;
    case slvOPCODE_UINT_TO_INT:             return gcSL_CONV;
    case slvOPCODE_UINT_TO_FLOAT:           return gcSL_I2F;
    case slvOPCODE_BOOL_TO_FLOAT:           return gcSL_I2F;
    case slvOPCODE_BOOL_TO_INT:             return gcSL_MOV;
    case slvOPCODE_BOOL_TO_UINT:            return gcSL_MOV;

    case slvOPCODE_INVERSE:                 return gcSL_RCP;

    case slvOPCODE_LESS_THAN:               gcmASSERT(0); return gcSL_NOP;
    case slvOPCODE_LESS_THAN_EQUAL:         gcmASSERT(0); return gcSL_NOP;
    case slvOPCODE_GREATER_THAN:            gcmASSERT(0); return gcSL_NOP;
    case slvOPCODE_GREATER_THAN_EQUAL:      gcmASSERT(0); return gcSL_NOP;
    case slvOPCODE_EQUAL:                   gcmASSERT(0); return gcSL_NOP;
    case slvOPCODE_NOT_EQUAL:               gcmASSERT(0); return gcSL_NOP;

    case slvOPCODE_ANY:                     return gcSL_NOP;
    case slvOPCODE_ALL:                     return gcSL_NOP;
    case slvOPCODE_NOT:                     return gcSL_NOP;

    case slvOPCODE_SIN:                     return gcSL_SIN;
    case slvOPCODE_COS:                     return gcSL_COS;
    case slvOPCODE_TAN:                     return gcSL_TAN;

    case slvOPCODE_ASIN:                    return gcSL_ASIN;
    case slvOPCODE_ACOS:                    return gcSL_ACOS;
    case slvOPCODE_ATAN:                    return gcSL_ATAN;
    case slvOPCODE_ATAN2:                   return gcSL_NOP;

    case slvOPCODE_POW:                     return gcSL_POW;
    case slvOPCODE_EXP2:                    return gcSL_EXP;
    case slvOPCODE_LOG2:                    return gcSL_LOG;
    case slvOPCODE_SQRT:                    return gcSL_SQRT;
    case slvOPCODE_INVERSE_SQRT:            return gcSL_RSQ;

    case slvOPCODE_ABS:                     return gcSL_ABS;
    case slvOPCODE_SIGN:                    return gcSL_SIGN;
    case slvOPCODE_FLOOR:                   return gcSL_FLOOR;
    case slvOPCODE_CEIL:                    return gcSL_CEIL;
    case slvOPCODE_FRACT:                   return gcSL_FRAC;
    case slvOPCODE_MIN:                     return gcSL_MIN;
    case slvOPCODE_MAX:                     return gcSL_MAX;
    case slvOPCODE_SATURATE:                return gcSL_SAT;
    case slvOPCODE_STEP:                    return gcSL_STEP;
    case slvOPCODE_DOT:                     return gcSL_NOP;
    case slvOPCODE_CROSS:                   return gcSL_CROSS;
    case slvOPCODE_NORMALIZE:               return gcSL_NOP;

    case slvOPCODE_JUMP:                    return gcSL_JMP;
    case slvOPCODE_CALL:                    return gcSL_CALL;
    case slvOPCODE_RETURN:                  return gcSL_RET;
    case slvOPCODE_DISCARD:                 return gcSL_KILL;

    case slvOPCODE_DFDX:                    return gcSL_DSX;
    case slvOPCODE_DFDY:                    return gcSL_DSY;
    case slvOPCODE_FWIDTH:                  return gcSL_FWIDTH;

    case slvOPCODE_LOAD:                    return gcSL_LOAD;
    case slvOPCODE_STORE1:                  return gcSL_STORE1;
    case slvOPCODE_ATTR_LD:                 return gcSL_ATTR_LD;
    case slvOPCODE_ATTR_ST:                 return gcSL_ATTR_ST;
    case slvOPCODE_AND_BITWISE:             return gcSL_AND_BITWISE;
    case slvOPCODE_OR_BITWISE:              return gcSL_OR_BITWISE;
    case slvOPCODE_XOR_BITWISE:             return gcSL_XOR_BITWISE;
    case slvOPCODE_NOT_BITWISE:             return gcSL_NOT_BITWISE;
    case slvOPCODE_LSHIFT:                  return gcSL_LSHIFT;
    case slvOPCODE_RSHIFT:                  return gcSL_RSHIFT;

    case slvOPCODE_POPCOUNT:                return gcSL_POPCOUNT;
    case slvOPCODE_FINDLSB:                 return gcSL_FINDLSB;
    case slvOPCODE_FINDMSB:                 return gcSL_FINDMSB;
    case slvOPCODE_BIT_REVERSAL:            return gcSL_BIT_REVERSAL;
    case slvOPCODE_BIT_EXTRACT:             return gcSL_BITEXTRACT;
    case slvOPCODE_BIT_RANGE:               return gcSL_BITRANGE;
    case slvOPCODE_BIT_RANGE1:              return gcSL_BITRANGE1;
    case slvOPCODE_BIT_INSERT:              return gcSL_BITINSERT;
    case slvOPCODE_UCARRY:                  return gcSL_UCARRY;

    case slvOPCODE_ATOMADD:                 return gcSL_ATOMADD;
    case slvOPCODE_ATOMSUB:                 return gcSL_ATOMSUB;
    case slvOPCODE_ATOMMIN:                 return gcSL_ATOMMIN;
    case slvOPCODE_ATOMMAX:                 return gcSL_ATOMMAX;
    case slvOPCODE_ATOMOR:                  return gcSL_ATOMOR;
    case slvOPCODE_ATOMAND:                 return gcSL_ATOMAND;
    case slvOPCODE_ATOMXOR:                 return gcSL_ATOMXOR;
    case slvOPCODE_ATOMXCHG:                return gcSL_ATOMXCHG;
    case slvOPCODE_ATOMCMPXCHG:             return gcSL_ATOMCMPXCHG;

    case slvOPCODE_SET:                     return gcSL_SET;
    case slvOPCODE_CMP:                     return gcSL_CMP;
    case slvOPCODE_BARRIER:                 return gcSL_BARRIER;
    case slvOPCODE_MEMORY_BARRIER:          return gcSL_MEM_BARRIER;

    case slvOPCODE_IMAGE_WRITE:             return gcSL_IMAGE_WR;
    case slvOPCODE_IMAGE_READ:              return gcSL_IMAGE_RD;
    case slvOPCODE_IMAGE_ADDRESS:           return gcSL_IMAGE_ADDR;
    case slvOPCODE_IMAGE_ADDRESS_3D:        return gcSL_IMAGE_ADDR_3D;

    case slvOPCODE_GET_SAMPLER_IDX:         return gcSL_GET_SAMPLER_IDX;
    case slvOPCODE_GET_SAMPLER_LMM:         return gcSL_GET_SAMPLER_LMM;
    case slvOPCODE_GET_SAMPLER_LBS:         return gcSL_GET_SAMPLER_LBS;
    case slvOPCODE_IMAGE_READ_3D:           return gcSL_IMAGE_RD_3D;
    case slvOPCODE_IMAGE_WRITE_3D:          return gcSL_IMAGE_WR_3D;
    case slvOPCODE_CLAMP0MAX:               return gcSL_CLAMP0MAX;
    case slvOPCODE_EMIT_VERTEX:             return gcSL_EMIT_VERTEX;
    case slvOPCODE_END_PRIMITIVE:           return gcSL_END_PRIMITIVE;

    case slvOPCODE_LOAD_L:                  return gcSL_LOAD_L;
    case slvOPCODE_STORE_L:                 return gcSL_STORE_L;
    case slvOPCODE_PARAM_CHAIN:             return gcSL_PARAM_CHAIN;
    case slvOPCODE_INTRINSIC:               return gcSL_INTRINSIC;
    case slvOPCODE_INTRINSIC_ST:            return gcSL_INTRINSIC_ST;
    default:
        gcmASSERT(0);
        return gcSL_NOP;
    }
}

static gceSTATUS
_EmitCode(
    IN sloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gcSL_OPCODE Opcode,
    IN gcsTARGET * Target,
    IN gcsSOURCE * Source0,
    IN gcsSOURCE * Source1
    )
{
    gceSTATUS    status = gcvSTATUS_OK;
    gcSL_FORMAT  format = gcSL_FLOAT;
    gcSHADER binary;
    gctUINT ps;
    gctBOOL      dump;

    gcmHEADER();

    if ((Target && gcIsMatrixDataType(Target->dataType)) ||
        (Source0 && gcIsMatrixDataType(Source0->dataType)) )
    {
        status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
        goto OnError;
    }

    dump = (Compiler->context.dumpOptions & slvDUMP_CODE_EMITTER);

    if (dump)
    {
        gcmVERIFY_OK(sloCOMPILER_GetBinary(Compiler, &binary));

        gcmVERIFY_OK(gcSHADER_GetInstructionCount(binary, &ps));

        if (binary->instrIndex != gcSHADER_OPCODE)
            ps++;

        gcmVERIFY_OK(sloCOMPILER_Dump(Compiler,
                                      slvDUMP_CODE_EMITTER,
                                      "%04u: opcode=\"%s\" line=\"%d\" string=\"%d\" ",
                                      ps,
                                      GetOpcodeName(Opcode),
                                      LineNo,
                                      StringNo));

        sloCOMPILER_IncrDumpOffset(Compiler);

        gcmVERIFY_OK(sloCOMPILER_Dump(Compiler,
                                      slvDUMP_CODE_EMITTER,
                                      "targetDataType=%s",
                                      Target ? gcGetDataTypeName(Target->dataType): "none"));

        if (Source0 && !Source1)
        {
            gcmVERIFY_OK(sloCOMPILER_Dump(Compiler,
                                          slvDUMP_CODE_EMITTER,
                                          "sourceDataType=%s",
                                          gcGetDataTypeName(Source0->dataType)));
        }
        else if (Source0 && Source1)
        {
            gcmASSERT(!gcIsMatrixDataType(Source1->dataType));

            gcmVERIFY_OK(sloCOMPILER_Dump(Compiler,
                                          slvDUMP_CODE_EMITTER,
                                          "source0DataType=%s source1DataType=%s",
                                          gcGetDataTypeName(Source0->dataType),
                                          gcGetDataTypeName(Source1->dataType)));
        }
    }

    if (Target && (gctINT)(Target->format) != -1)
    {
        format = Target->format;
    }
    else if (Target)
    {
        format = slConvDataTypeToFormat(Compiler, Target->dataType);
    }

    gcmONERROR(_EmitOpcodeAndTargetFormatted(Compiler,
                                           LineNo,
                                           StringNo,
                                           Opcode,
                                           Target,
                                           format));

    if (Source0 != gcvNULL)
    {
        gcmONERROR(_EmitSource(Compiler,
                             LineNo,
                             StringNo,
                             Source0));
    }

    if (Source1 != gcvNULL)
    {
        gcmONERROR(_EmitSource(Compiler,
                             LineNo,
                             StringNo,
                             Source1));
    }

    if (dump)
    {
        gcSHADER binary = gcvNULL;

        gcmVERIFY_OK(sloCOMPILER_GetBinary(Compiler, &binary));

        _DumpLastIR(binary);

        sloCOMPILER_DecrDumpOffset(Compiler);
    }


OnError:
    gcmFOOTER_NO();
    return status;
}

gceSTATUS
slEmitBuiltinAsmCode(
    IN sloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gcSL_OPCODE Opcode,
    IN slsASM_OPCODE * AsmOpcode,
    IN gcsTARGET * Target,
    IN slsASM_MODIFIERS *TargetModifiers,
    IN gcsSOURCE * Source0,
    IN slsASM_MODIFIERS *Source0Modifiers,
    IN gcsSOURCE * Source1,
    IN slsASM_MODIFIERS *Source1Modifiers
    )
{
    gceSTATUS           status = gcvSTATUS_OK;
    gcSHADER            binary;
    sloCODE_EMITTER     codeEmitter;
    gcSL_FORMAT         format;
    gcSL_CONDITION      condition;
    gcSHADER_PRECISION  precision;
    gctUINT32           srcLoc;

    gcmHEADER();

    if ((Target && gcIsMatrixDataType(Target->dataType)) ||
        (Source0 && gcIsMatrixDataType(Source0->dataType)) ||
        (Source1 && gcIsMatrixDataType(Source1->dataType)))
    {
        status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
        goto OnError;
    }

    codeEmitter = Compiler->codeEmitter;
    gcmASSERT(codeEmitter);

    /* flush out previous instructions */
    status = sloCODE_EMITTER_EndBasicBlock(Compiler, codeEmitter);
    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmVERIFY_OK(sloCOMPILER_GetBinary(Compiler, &binary));

    if (Compiler->context.dumpOptions & slvDUMP_CODE_EMITTER)
    {
        gcmVERIFY_OK(sloCOMPILER_Dump(Compiler,
                                      slvDUMP_CODE_EMITTER,
                                      "<INSTRUCTION line=\"%d\" string=\"%d\" opcode=\"%s\""
                                      " targetDataType=\"%s\"",
                                      LineNo,
                                      StringNo,
                                      GetOpcodeName(Opcode),
                                      Target ? gcGetDataTypeName(Target->dataType): "none"));

        if (Source0 && !Source1)
        {
            gcmVERIFY_OK(sloCOMPILER_Dump(Compiler,
                                          slvDUMP_CODE_EMITTER,
                                          " sourceDataType=\"%s\">",
                                          gcGetDataTypeName(Source0->dataType)));
        }
        else if (Source0 && Source1)
        {
            gcmASSERT(!gcIsMatrixDataType(Source1->dataType));

            gcmVERIFY_OK(sloCOMPILER_Dump(Compiler,
                                          slvDUMP_CODE_EMITTER,
                                          " source0DataType=\"%s\" source1DataType=\"%s\">",
                                          gcGetDataTypeName(Source0->dataType),
                                          gcGetDataTypeName(Source1->dataType)));
        }
    }

    if (TargetModifiers && TargetModifiers->modifiers[sleASM_MODIFIER_OPND_FORMAT].value != -1)
    {
        format = (gcSL_FORMAT)TargetModifiers->modifiers[sleASM_MODIFIER_OPND_FORMAT].value;
    }
    else if (Target && (gctINT)(Target->format) != -1)
    {
        format = Target->format;
    }
    else if (Target)
    {
        format = slConvDataTypeToFormat(Compiler, Target->dataType);
    }
    else
    {
        format = gcSL_FLOAT;
    }

    if ((gctINT)(AsmOpcode->condition) != -1)
    {
        condition = (gcSL_CONDITION)AsmOpcode->condition;
    }
    else
    {
        condition = gcSL_ALWAYS;
    }

    if (TargetModifiers && TargetModifiers->modifiers[sleASM_MODIFIER_OPND_PRECISION].value != -1)
    {
        precision = (gcSHADER_PRECISION)TargetModifiers->modifiers[sleASM_MODIFIER_OPND_PRECISION].value;
    }
    else if (Target)
    {
        precision = Target->precision;
    }
    else
    {
        precision = (gcSHADER_PRECISION)gcSL_PRECISION_HIGH;
    }
    srcLoc = GCSL_Build_SRC_LOC(LineNo, StringNo);

    if (Target)
    {
        gcmONERROR(_EmitOpcodeConditionAndFormat(Compiler,
                                               LineNo,
                                               StringNo,
                                               Opcode,
                                               condition,
                                               format,
                                               precision,
                                               Target));
    }
    else
    {
        gcmONERROR(gcSHADER_AddOpcode(binary,
                                    Opcode,
                                    0,
                                    gcSL_ENABLE_NONE,
                                    format,
                                    gcSHADER_PRECISION_DEFAULT,
                                    srcLoc));
    }

    if ((gctINT)(AsmOpcode->round) != -1)
    {
        gcSHADER_AddRoundingMode(
            binary,
            (gcSL_ROUND)AsmOpcode->round);
    }

    if ((gctINT)(AsmOpcode->satuate) != -1)
    {
        gcSHADER_AddSaturation(
            binary,
            (gcSL_MODIFIER_SAT)AsmOpcode->satuate);
    }

    if (Source0 != gcvNULL)
    {
        gcmONERROR(_EmitSourceWithModifiers(Compiler,
                             LineNo,
                             StringNo,
                             Source0,
                             Source0Modifiers));
    }

    if (Opcode == gcSL_CONV)
    {
        gctUINT32 format[1];
        format[0] = Source1->u.sourceConstant.u.uintConstant;
        status = gcSHADER_AddSourceConstantFormatted(binary, format, gcSL_UINT32);
    }
    else if (Source1 != gcvNULL)
    {
        gcmONERROR(_EmitSourceWithModifiers(Compiler,
                             LineNo,
                             StringNo,
                             Source1,
                             Source1Modifiers));
    }

    gcmVERIFY_OK(sloCOMPILER_Dump(Compiler,
                                  slvDUMP_CODE_EMITTER,
                                  "</INSTRUCTION>"));

OnError:
    gcmFOOTER_NO();
    return status;
}

static gceSTATUS
_EmitCodeFormatted(
    IN sloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gcSL_OPCODE Opcode,
    IN gcSL_FORMAT Format,
    IN gcsTARGET * Target,
    IN gcsSOURCE * Source0,
    IN gcsSOURCE * Source1
    )
{
    gceSTATUS    status;

    gcmHEADER();

    gcmASSERT(Target);
    gcmASSERT(!gcIsMatrixDataType(Target->dataType));
    gcmASSERT(Source0);
    gcmASSERT(!gcIsMatrixDataType(Source0->dataType));

    gcmVERIFY_OK(sloCOMPILER_Dump(Compiler,
                                  slvDUMP_CODE_EMITTER,
                                  "<INSTRUCTION line=\"%d\" string=\"%d\" opcode=\"%s\""
                                  " targetDataType=\"%s\"",
                                  LineNo,
                                  StringNo,
                                  GetOpcodeName(Opcode),
                                  gcGetDataTypeName(Target->dataType)));

    if (Source1 == gcvNULL)
    {
        gcmVERIFY_OK(sloCOMPILER_Dump(Compiler,
                                      slvDUMP_CODE_EMITTER,
                                      " sourceDataType=\"%s\">",
                                      gcGetDataTypeName(Source0->dataType)));
    }
    else
    {
        gcmASSERT(!gcIsMatrixDataType(Source1->dataType));

        gcmVERIFY_OK(sloCOMPILER_Dump(Compiler,
                                      slvDUMP_CODE_EMITTER,
                                      " source0DataType=\"%s\" source1DataType=\"%s\">",
                                      gcGetDataTypeName(Source0->dataType),
                                      gcGetDataTypeName(Source1->dataType)));
    }

    status = _EmitOpcodeAndTargetFormatted(Compiler,
                                           LineNo,
                                           StringNo,
                                           Opcode,
                                           Target,
                                           Format);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    status = _EmitSource(Compiler,
                             LineNo,
                             StringNo,
                             Source0);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    if (Source1 != gcvNULL)
    {
        status = _EmitSource(Compiler,
                             LineNo,
                             StringNo,
                             Source1);

        if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }
    }

    gcmVERIFY_OK(sloCOMPILER_Dump(Compiler,
                                  slvDUMP_CODE_EMITTER,
                                  "</INSTRUCTION>"));

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

static gceSTATUS
_EmitBranchCode(
    IN sloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gcSL_OPCODE Opcode,
    IN gcSL_CONDITION Condition,
    IN gctLABEL Label,
    IN gcsSOURCE * Source0,
    IN gcsSOURCE * Source1
    )
{
    gceSTATUS            status;
    sloCODE_EMITTER        codeEmitter;

    gcmHEADER();

    codeEmitter = Compiler->codeEmitter;
    gcmASSERT(codeEmitter);

    status = sloCODE_EMITTER_EndBasicBlock(Compiler, codeEmitter);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    if (Compiler->context.dumpOptions & slvDUMP_CODE_EMITTER)
    {
        gcmVERIFY_OK(sloCOMPILER_Dump(
                                    Compiler,
                                    slvDUMP_CODE_EMITTER,
                                    "<INSTRUCTION line=\"%d\" string=\"%d\" opcode=\"%s\""
                                    " condition=\"%s\" label=\"%d\"",
                                    LineNo,
                                    StringNo,
                                    GetOpcodeName(Opcode),
                                    _GetConditionName(Condition),
                                    Label));

        if (Source0 != gcvNULL)
        {
            gcmASSERT(!gcIsMatrixDataType(Source0->dataType));

            gcmVERIFY_OK(sloCOMPILER_Dump(
                                        Compiler,
                                        slvDUMP_CODE_EMITTER,
                                        " source0DataType=\"%s\"",
                                        gcGetDataTypeName(Source0->dataType)));
        }

        if (Source1 != gcvNULL)
        {
            gcmASSERT(!gcIsMatrixDataType(Source1->dataType));

            gcmVERIFY_OK(sloCOMPILER_Dump(
                                        Compiler,
                                        slvDUMP_CODE_EMITTER,
                                        " source1DataType=\"%s\"",
                                        gcGetDataTypeName(Source1->dataType)));
        }

        gcmVERIFY_OK(sloCOMPILER_Dump(
                                    Compiler,
                                    slvDUMP_CODE_EMITTER,
                                    ">"));
    }

    status = _EmitOpcodeConditional(Compiler,
                    LineNo,
                    StringNo,
                    Opcode,
                    Condition,
                    Source0,
                    Label);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    if (Source0 != gcvNULL)
    {
        status = _EmitSource(
                            Compiler,
                            LineNo,
                            StringNo,
                            Source0);

        if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }
    }

    if (Source1 != gcvNULL)
    {
        status = _EmitSource(
                            Compiler,
                            LineNo,
                            StringNo,
                            Source1);

        if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }
    }

    gcmVERIFY_OK(sloCOMPILER_Dump(
                                Compiler,
                                slvDUMP_CODE_EMITTER,
                                "</INSTRUCTION>"));

    status = sloCODE_EMITTER_NewBasicBlock(Compiler, codeEmitter);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

static gceSTATUS
_EmitNullTargetCode(
    IN sloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN sleOPCODE Opcode,
    IN gcsSOURCE * Source0,
    IN gcsSOURCE * Source1
    )
{
    gceSTATUS    status;
    sloCODE_EMITTER    codeEmitter;
    gcSL_OPCODE opcode;
    gcSHADER binary;

    gcmVERIFY_OK(sloCOMPILER_GetBinary(Compiler, &binary));
    codeEmitter = Compiler->codeEmitter;
    gcmASSERT(codeEmitter);

    status = sloCODE_EMITTER_EndBasicBlock(Compiler, codeEmitter);
    if (gcmIS_ERROR(status)) return status;

    opcode = _ConvOpcode(Opcode);

    if (Compiler->context.dumpOptions & slvDUMP_CODE_EMITTER)
    {
        gcmVERIFY_OK(sloCOMPILER_Dump(Compiler,
                                      slvDUMP_CODE_EMITTER,
                                      "<INSTRUCTION line=\"%d\" string=\"%d\" opcode=\"%s\"",
                                      LineNo,
                                      StringNo,
                                      GetOpcodeName(opcode)));

        if (Source0 != gcvNULL)
        {
            gcmASSERT(!gcIsMatrixDataType(Source0->dataType));

            gcmVERIFY_OK(sloCOMPILER_Dump(Compiler,
                                          slvDUMP_CODE_EMITTER,
                                          " source0DataType=\"%s\"",
                                          gcGetDataTypeName(Source0->dataType)));
        }

        if (Source1 != gcvNULL)
        {
            gcmASSERT(!gcIsMatrixDataType(Source1->dataType));

            gcmVERIFY_OK(sloCOMPILER_Dump(Compiler,
                                          slvDUMP_CODE_EMITTER,
                                          " source1DataType=\"%s\"",
                                          gcGetDataTypeName(Source1->dataType)));
        }

        gcmVERIFY_OK(sloCOMPILER_Dump(Compiler,
                                      slvDUMP_CODE_EMITTER,
                                      ">"));
    }

    status = gcSHADER_AddOpcode(binary,
                                opcode,
                                0,
                                gcSL_ENABLE_NONE,
                                gcSL_FLOAT,
                                gcSHADER_PRECISION_DEFAULT,
                                GCSL_Build_SRC_LOC(LineNo, StringNo));

    if (gcmIS_ERROR(status)) return status;

    if (Source0 != gcvNULL)
    {
        status = _EmitSource(Compiler,
                             LineNo,
                             StringNo,
                             Source0);
        if (gcmIS_ERROR(status)) return status;
    }

    if (Source1 != gcvNULL)
    {
        status = _EmitSource(Compiler,
                             LineNo,
                             StringNo,
                             Source1);
        if (gcmIS_ERROR(status)) return status;
    }

    gcmVERIFY_OK(sloCOMPILER_Dump(Compiler,
                                  slvDUMP_CODE_EMITTER,
                                  "</INSTRUCTION>"));

    return gcvSTATUS_OK;
}

gceSTATUS
slEmitNullTargetCode(
    IN sloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN sleOPCODE Opcode,
    IN gcsSOURCE * Source0,
    IN gcsSOURCE * Source1
    )
{
    gceSTATUS status;
    gcsSOURCE *source1;
    gcsSOURCE newSource1;

    if(Source0 && Source1)
    {
       status = _PrepareAnotherSource(Compiler,
                                      LineNo,
                                      StringNo,
                                      gcvNULL,
                                      Source0,
                                      Source1,
                                      &newSource1);
       if (gcmIS_ERROR(status)) return status;
       source1 = &newSource1;
    }
    else
    {
        source1 = Source1;
    }

    return _EmitNullTargetCode(Compiler,
                               LineNo,
                               StringNo,
                               Opcode,
                               Source0,
                               source1);
}

gceSTATUS
slEmitConvCode(
    IN sloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gcsTARGET * Target,
    IN gcsSOURCE * Source,
    IN gcSHADER_TYPE DataType
    )
{

    gcsSOURCE source1[1];

    gcsSOURCE_InitializeTargetFormat(source1, DataType);
    return slEmitCode2(Compiler,
               LineNo,
               StringNo,
               slvOPCODE_CONV,
               Target,
               Source,
               source1);
}

gceSTATUS
slEmitAssignCode(
    IN sloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gcsTARGET * Target,
    IN gcsSOURCE * Source
    )
{
    return slEmitCode1(
                    Compiler,
                    LineNo,
                    StringNo,
                    slvOPCODE_ASSIGN,
                    Target,
                    Source);
}

static gceSTATUS
_EmitFloatToIntCode(
    IN sloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gcsTARGET * Target,
    IN gcsSOURCE * Source
    )
{
    gceSTATUS        status;

    gcmHEADER();

    gcmASSERT(Target);
    gcmASSERT(Source);

#if _USE_F2I_OPCODE
#if TREAT_ES20_INTEGER_AS_FLOAT
    if (sloCOMPILER_IsHaltiVersion(Compiler))
    {
#endif
        status = _EmitCode(Compiler,
                           LineNo,
                           StringNo,
                           gcSL_F2I,
                           Target,
                           Source,
                           gcvNULL);
        if (gcmIS_ERROR(status)) return status;
#if TREAT_ES20_INTEGER_AS_FLOAT
    }
    /*
    ** Right now we don't support INTEGER type for a ES20 shader,
    ** so we need to convert it to FLOAT.
    */
    else
    {
        /* CONV.RTZ, target, source, float*/
        gcSHADER binary;
        gctUINT32 format[1];

        gcmVERIFY_OK(sloCOMPILER_GetBinary(Compiler, &binary));

        status = _EmitOpcodeAndTargetFormatted(Compiler,
                                               LineNo,
                                               StringNo,
                                               gcSL_CONV,
                                               Target,
                                               gcSL_FLOAT);
        if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

        gcSHADER_AddRoundingMode(binary, gcSL_ROUND_RTZ);

        status = _EmitSource(Compiler,
                             LineNo,
                             StringNo,
                             Source);
        if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

        format[0] = gcSL_FLOAT;
        status = gcSHADER_AddSourceConstantFormatted(binary, format, gcSL_UINT32);
        if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }
    }
#endif
#else
#if TREAT_ES20_INTEGER_AS_FLOAT
    if (sloCOMPILER_IsHaltiVersion(Compiler))
    {
#endif
        status = _EmitCode(Compiler,
                           LineNo,
                           StringNo,
                           gcSL_F2I,
                           Target,
                           Source,
                           gcvNULL);
        if (gcmIS_ERROR(status)) return status;
#if TREAT_ES20_INTEGER_AS_FLOAT
    }
    else
    {
        slsIOPERAND    intermIOperands[3];
        gcsTARGET    intermTargets[3];
        gcsSOURCE    intermSources[3];

        /* sign t0, source */
        slsIOPERAND_New(Compiler, &intermIOperands[0], Source->dataType, Source->precision);
        gcsTARGET_InitializeUsingIOperand(&intermTargets[0], &intermIOperands[0]);

        status = _EmitCode(Compiler,
                           LineNo,
                           StringNo,
                           gcSL_SIGN,
                           &intermTargets[0],
                           Source,
                           gcvNULL);

        if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

        /* abs t1, source */
        slsIOPERAND_New(Compiler, &intermIOperands[1], Source->dataType, Source->precision);
        gcsTARGET_InitializeUsingIOperand(&intermTargets[1], &intermIOperands[1]);

        status = _EmitCode(Compiler,
                           LineNo,
                           StringNo,
                           gcSL_ABS,
                           &intermTargets[1],
                           Source,
                           gcvNULL);

        if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

        /* floor t2, t1 */
        slsIOPERAND_New(Compiler, &intermIOperands[2], Source->dataType, Source->precision);
        gcsTARGET_InitializeUsingIOperand(&intermTargets[2], &intermIOperands[2]);
        gcsSOURCE_InitializeUsingIOperand(&intermSources[1], &intermIOperands[1]);

        status = _EmitCode(Compiler,
                           LineNo,
                           StringNo,
                           gcSL_FLOOR,
                           &intermTargets[2],
                           &intermSources[1],
                           gcvNULL);

        if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

        /* mul target, t0, t2 */
        gcsSOURCE_InitializeUsingIOperand(&intermSources[0], &intermIOperands[0]);
        gcsSOURCE_InitializeUsingIOperand(&intermSources[2], &intermIOperands[2]);

        status = _EmitCode(Compiler,
                           LineNo,
                           StringNo,
                           gcSL_MUL,
                           Target,
                           &intermSources[0],
                           &intermSources[2]);

        if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }
    }
#endif
#endif

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

static gceSTATUS
_EmitFloatToHalfCode(
    IN sloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gcsTARGET * Target,
    IN gcsSOURCE * Source
    )
{
    gctUINT32 format[1];
    gceSTATUS status;
    gcSHADER binary;

    gcmHEADER();

    gcmASSERT(Target);
    gcmASSERT(Source);

    gcmVERIFY_OK(sloCOMPILER_GetBinary(Compiler, &binary));
    status = _EmitOpcodeAndTargetFormatted(Compiler,
                                           LineNo,
                                           StringNo,
                                           gcSL_CONV,
                                           Target,
                                           gcSL_FLOAT16);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    status = _EmitSource(Compiler,
                         LineNo,
                         StringNo,
                         Source);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    format[0] = gcSL_FLOAT;
    status = gcSHADER_AddSourceConstantFormatted(binary, format, gcSL_UINT32);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

static gceSTATUS
_EmitHalfToFloatCode(
    IN sloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gcsTARGET * Target,
    IN gcsSOURCE * Source
    )
{
    gctUINT32 format[1];
    gceSTATUS status;
    gcSHADER binary;

    gcmHEADER();

    gcmASSERT(Target);
    gcmASSERT(Source);

    gcmVERIFY_OK(sloCOMPILER_GetBinary(Compiler, &binary));
    status = _EmitOpcodeAndTargetFormatted(Compiler,
                                           LineNo,
                                           StringNo,
                                           gcSL_CONV,
                                           Target,
                                           gcSL_FLOAT);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    status = _EmitSourceTempWithFormat(Compiler,
                            LineNo,
                            StringNo,
                            gcIsSamplerDataType(Source->dataType),
                            Source,
                            gcSL_FLOAT16);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    format[0] = gcSL_FLOAT16;
    status = gcSHADER_AddSourceConstantFormatted(binary, format, gcSL_UINT32);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

static gceSTATUS
_EmitScalarFloatOrIntToBoolCode(
    IN sloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gcsTARGET * Target,
    IN gcsSOURCE * Source
    )
{
    gceSTATUS       status;
    gctLABEL        endLabel;
    gcsSOURCE       constSource;
    gcsSOURCE       newSource;
    gcsSOURCE       falseSource;

    gcmHEADER();

    endLabel = slNewLabel(Compiler);

    status = _PrepareSource(Compiler,
                            LineNo,
                            StringNo,
                            gcvNULL,
                            Source,
                            &newSource);
    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    /* Make sure that source0 and source1 have the same data type. */
    if (Source->dataType == gcSHADER_FLOAT_X1 || Source->dataType == gcSHADER_FLOAT64_X1)
    {
        gcsSOURCE_InitializeFloatConstant(&falseSource, gcSHADER_PRECISION_MEDIUM, slmB2F(gcvFALSE));
    }
    else
    {
        gcsSOURCE_InitializeIntConstant(&falseSource, gcSHADER_PRECISION_MEDIUM, 0);
#if TREAT_ES20_INTEGER_AS_FLOAT
        gcsSOURCE_InitializeFloatConstant(&falseSource, gcSHADER_PRECISION_MEDIUM, slmB2F(gcvFALSE));
#endif
    }

    status = _EmitBranchCode(
                        Compiler,
                        LineNo,
                        StringNo,
                        _ConvOpcode(slvOPCODE_JUMP),
                        gcSL_EQUAL,
                        endLabel,
                        &newSource,
                        &falseSource);
    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    /* mov target, true */
#if TREAT_ES20_INTEGER_AS_FLOAT
    if (sloCOMPILER_IsHaltiVersion(Compiler)) {
#endif
       gcsSOURCE_InitializeBoolConstant(&constSource, gcSHADER_PRECISION_MEDIUM, gcvTRUE);
#if TREAT_ES20_INTEGER_AS_FLOAT
    }
    else {
       gcsSOURCE_InitializeFloatConstant(&constSource, gcSHADER_PRECISION_MEDIUM, slmB2F(gcvTRUE));
    }
#endif

    status = _EmitCode(Compiler,
               LineNo,
               StringNo,
               gcSL_MOV,
               Target,
               &constSource,
               gcvNULL);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    /* end: */
    status = slSetLabel(Compiler, LineNo, StringNo, endLabel);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

static gceSTATUS
_EmitFloatOrIntToBoolCode(
    IN sloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gcsTARGET * Target,
    IN gcsSOURCE * Source
    )
{
    gceSTATUS    status;
    gctUINT      i;
    gcsTARGET    componentTarget;
    gcsSOURCE    componentSource;
    gcsSOURCE    constSource0;

    gcmHEADER();

    gcmASSERT(Target);
    gcmASSERT(Source);

    /* mov target, false */
#if TREAT_ES20_INTEGER_AS_FLOAT
    if (sloCOMPILER_IsHaltiVersion(Compiler)) {
#endif
       gcsSOURCE_InitializeBoolConstant(&constSource0, gcSHADER_PRECISION_MEDIUM, gcvFALSE);
#if TREAT_ES20_INTEGER_AS_FLOAT
    }
    else {
       gcsSOURCE_InitializeFloatConstant(&constSource0, gcSHADER_PRECISION_MEDIUM, slmB2F(gcvFALSE));
    }
#endif
    status = _EmitCode(Compiler,
               LineNo,
               StringNo,
               gcSL_MOV,
               Target,
               &constSource0,
               gcvNULL);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    if (Target->dataType == gcSHADER_BOOLEAN_X1)
    {
        status = _EmitScalarFloatOrIntToBoolCode(Compiler,
                            LineNo,
                            StringNo,
                            Target,
                            Source);

        if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }
    }
    else
    {
        gcmASSERT(Target->dataType == gcSHADER_BOOLEAN_X2
                    || Target->dataType == gcSHADER_BOOLEAN_X3
                    || Target->dataType == gcSHADER_BOOLEAN_X4);

        for (i = 0; i < gcGetVectorDataTypeComponentCount(Target->dataType); i++)
        {
            gcsTARGET_InitializeAsVectorComponent(&componentTarget, Target, i);
            gcsSOURCE_InitializeAsVectorComponent(&componentSource, Source, i);

            status = _EmitScalarFloatOrIntToBoolCode(Compiler,
                                LineNo,
                                StringNo,
                                &componentTarget,
                                &componentSource);

            if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }
        }
    }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

static gceSTATUS
_EmitAnyCode(
    IN sloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gcsTARGET * Target,
    IN gcsSOURCE * Source
    )
{
    gceSTATUS    status;
    gctLABEL    elseLabel, endLabel;
    gcsSOURCE    constSource;

    gcmHEADER();

    elseLabel    = slNewLabel(Compiler);
    endLabel    = slNewLabel(Compiler);

    /* jump else if all components are false */
    status = slEmitTestBranchCode(
                                Compiler,
                                LineNo,
                                StringNo,
                                slvOPCODE_JUMP,
                                elseLabel,
                                gcvFALSE,
                                Source);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    /* mov target, true */
#if TREAT_ES20_INTEGER_AS_FLOAT
    if (sloCOMPILER_IsHaltiVersion(Compiler)) {
#endif
       gcsSOURCE_InitializeBoolConstant(&constSource, gcSHADER_PRECISION_MEDIUM, gcvTRUE);
#if TREAT_ES20_INTEGER_AS_FLOAT
    }
    else {
       gcsSOURCE_InitializeFloatConstant(&constSource, gcSHADER_PRECISION_MEDIUM, slmB2F(gcvTRUE));
    }
#endif

    status = _EmitCode(
                        Compiler,
                        LineNo,
                        StringNo,
                        gcSL_MOV,
                        Target,
                        &constSource,
                        gcvNULL);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    /* jump end */
    status = slEmitAlwaysBranchCode(
                                    Compiler,
                                    LineNo,
                                    StringNo,
                                    slvOPCODE_JUMP,
                                    endLabel);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    /* else: */
    status = slSetLabel(Compiler, LineNo, StringNo, elseLabel);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    /* mov target, false */
#if TREAT_ES20_INTEGER_AS_FLOAT
    if (sloCOMPILER_IsHaltiVersion(Compiler)) {
#endif
       gcsSOURCE_InitializeBoolConstant(&constSource, gcSHADER_PRECISION_MEDIUM, gcvFALSE);
#if TREAT_ES20_INTEGER_AS_FLOAT
    }
    else {
       gcsSOURCE_InitializeFloatConstant(&constSource, gcSHADER_PRECISION_MEDIUM, slmB2F(gcvFALSE));
    }
#endif

    status = _EmitCode(
                        Compiler,
                        LineNo,
                        StringNo,
                        gcSL_MOV,
                        Target,
                        &constSource,
                        gcvNULL);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    /* end: */
    status = slSetLabel(Compiler, LineNo, StringNo, endLabel);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

static gceSTATUS
_EmitAllCode(
    IN sloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gcsTARGET * Target,
    IN gcsSOURCE * Source
    )
{
    gceSTATUS    status;
    gctLABEL    elseLabel, endLabel;
    gcsSOURCE    constSource;

    gcmHEADER();

    elseLabel    = slNewLabel(Compiler);
    endLabel    = slNewLabel(Compiler);

    /* jump else if all components are true */
    status = slEmitTestBranchCode(
                                Compiler,
                                LineNo,
                                StringNo,
                                slvOPCODE_JUMP,
                                elseLabel,
                                gcvTRUE,
                                Source);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    /* mov target, false */
    gcsSOURCE_InitializeFloatConstant(&constSource, gcSHADER_PRECISION_MEDIUM, slmB2F(gcvFALSE));

    status = _EmitCode(
                        Compiler,
                        LineNo,
                        StringNo,
                        gcSL_MOV,
                        Target,
                        &constSource,
                        gcvNULL);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    /* jump end */
    status = slEmitAlwaysBranchCode(
                                    Compiler,
                                    LineNo,
                                    StringNo,
                                    slvOPCODE_JUMP,
                                    endLabel);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    /* else: */
    status = slSetLabel(Compiler, LineNo, StringNo, elseLabel);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    /* mov target, true */
#if TREAT_ES20_INTEGER_AS_FLOAT
    if (sloCOMPILER_IsHaltiVersion(Compiler)) {
#endif
       gcsSOURCE_InitializeBoolConstant(&constSource, gcSHADER_PRECISION_MEDIUM, gcvTRUE);
#if TREAT_ES20_INTEGER_AS_FLOAT
    }
    else {
       gcsSOURCE_InitializeFloatConstant(&constSource, gcSHADER_PRECISION_MEDIUM, slmB2F(gcvTRUE));
    }
#endif

    status = _EmitCode(
                        Compiler,
                        LineNo,
                        StringNo,
                        gcSL_MOV,
                        Target,
                        &constSource,
                        gcvNULL);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    /* end: */
    status = slSetLabel(Compiler, LineNo, StringNo, endLabel);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

static gceSTATUS
_EmitScalarNotCode(
    IN sloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gcsTARGET * Target,
    IN gcsSOURCE * Source
    )
{
    gceSTATUS    status;
    gctLABEL    elseLabel, endLabel;
    gcsSOURCE    constSource;

    gcmHEADER();

    elseLabel    = slNewLabel(Compiler);
    endLabel    = slNewLabel(Compiler);

    /* jump else if (source == true) */
    status = slEmitTestBranchCode(
                                Compiler,
                                LineNo,
                                StringNo,
                                slvOPCODE_JUMP,
                                elseLabel,
                                gcvTRUE,
                                Source);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    /* mov target, true */
#if TREAT_ES20_INTEGER_AS_FLOAT
    if (sloCOMPILER_IsHaltiVersion(Compiler)) {
#endif
       gcsSOURCE_InitializeBoolConstant(&constSource, gcSHADER_PRECISION_MEDIUM, gcvTRUE);
#if TREAT_ES20_INTEGER_AS_FLOAT
    }
    else {
       gcsSOURCE_InitializeFloatConstant(&constSource, gcSHADER_PRECISION_MEDIUM, slmB2F(gcvTRUE));
    }
#endif

    status = _EmitCode(
                        Compiler,
                        LineNo,
                        StringNo,
                        gcSL_MOV,
                        Target,
                        &constSource,
                        gcvNULL);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    /* jump end */
    status = slEmitAlwaysBranchCode(
                                    Compiler,
                                    LineNo,
                                    StringNo,
                                    slvOPCODE_JUMP,
                                    endLabel);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    /* else: */
    status = slSetLabel(Compiler, LineNo, StringNo, elseLabel);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    /* mov target, false */
#if TREAT_ES20_INTEGER_AS_FLOAT
    if (sloCOMPILER_IsHaltiVersion(Compiler)) {
#endif
       gcsSOURCE_InitializeBoolConstant(&constSource, gcSHADER_PRECISION_MEDIUM, gcvFALSE);
#if TREAT_ES20_INTEGER_AS_FLOAT
    }
    else {
       gcsSOURCE_InitializeFloatConstant(&constSource, gcSHADER_PRECISION_MEDIUM, slmB2F(gcvFALSE));
    }
#endif

    status = _EmitCode(
                        Compiler,
                        LineNo,
                        StringNo,
                        gcSL_MOV,
                        Target,
                        &constSource,
                        gcvNULL);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    /* end: */
    status = slSetLabel(Compiler, LineNo, StringNo, endLabel);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

static gceSTATUS
_EmitNotCode(
    IN sloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gcsTARGET * Target,
    IN gcsSOURCE * Source
    )
{
    gceSTATUS    status;
    gctUINT        i;
    gcsTARGET    componentTarget;
    gcsSOURCE    componentSource;

    gcmHEADER();

    gcmASSERT(Target);
    gcmASSERT(Source);

    if (Target->dataType == gcSHADER_BOOLEAN_X1)
    {
        status = _EmitScalarNotCode(
                                    Compiler,
                                    LineNo,
                                    StringNo,
                                    Target,
                                    Source);

        if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }
    }
    else
    {
        gcmASSERT(Target->dataType == gcSHADER_BOOLEAN_X2
                    || Target->dataType == gcSHADER_BOOLEAN_X3
                    || Target->dataType == gcSHADER_BOOLEAN_X4);

        for (i = 0; i < gcGetVectorDataTypeComponentCount(Target->dataType); i++)
        {
            gcsTARGET_InitializeAsVectorComponent(&componentTarget, Target, i);
            gcsSOURCE_InitializeAsVectorComponent(&componentSource, Source, i);

            status = _EmitScalarNotCode(
                                        Compiler,
                                        LineNo,
                                        StringNo,
                                        &componentTarget,
                                        &componentSource);

            if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }
        }
    }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

static gceSTATUS
_EmitDP2Code(
    IN sloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gcsTARGET * Target,
    IN gcsSOURCE * Source0,
    IN gcsSOURCE * Source1
    );

static gceSTATUS
_EmitExpandedNORM2Code(
    IN sloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gcsTARGET * Target,
    IN gcsSOURCE * Source
    )
{
    gceSTATUS        status;
    slsIOPERAND        intermIOperands[2];
    gcsTARGET        intermTargets[2];
    gcsSOURCE        intermSources[2];

    gcmHEADER();

    gcmASSERT(Target);
    gcmASSERT(Source);

    /* dp2 t0, source, source */
    slsIOPERAND_New(Compiler, &intermIOperands[0], gcSHADER_FLOAT_X1, Source->precision);
    gcsTARGET_InitializeUsingIOperand(&intermTargets[0], &intermIOperands[0]);

    status = _EmitDP2Code(
                    Compiler,
                    LineNo,
                    StringNo,
                    &intermTargets[0],
                    Source,
                    Source);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    /* rsq t1, t0 */
    slsIOPERAND_New(Compiler, &intermIOperands[1], gcSHADER_FLOAT_X1, Source->precision);
    gcsTARGET_InitializeUsingIOperand(&intermTargets[1], &intermIOperands[1]);
    gcsSOURCE_InitializeUsingIOperand(&intermSources[0], &intermIOperands[0]);

    status = _EmitCode(
                    Compiler,
                    LineNo,
                    StringNo,
                    gcSL_RSQ,
                    &intermTargets[1],
                    &intermSources[0],
                    gcvNULL);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    /* mul target, source, t1 */
    gcsSOURCE_InitializeUsingIOperand(&intermSources[1], &intermIOperands[1]);

    status = _EmitCode(
                    Compiler,
                    LineNo,
                    StringNo,
                    gcSL_MUL,
                    Target,
                    Source,
                    &intermSources[1]);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

static gceSTATUS
_EmitExpandedNORM3Code(
    IN sloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gcsTARGET * Target,
    IN gcsSOURCE * Source
    )
{
    gceSTATUS        status;
    slsIOPERAND        intermIOperands[2];
    gcsTARGET        intermTargets[2];
    gcsSOURCE        intermSources[2];

    gcmHEADER();

    gcmASSERT(Target);
    gcmASSERT(Source);

    if (Source == gcvNULL)
    {
        status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
        gcmFOOTER();
        return status;
    }

    /* dp3 t0, source, source */
    slsIOPERAND_New(Compiler, &intermIOperands[0], gcSHADER_FLOAT_X1, Source->precision);
    gcsTARGET_InitializeUsingIOperand(&intermTargets[0], &intermIOperands[0]);

    status = _EmitCode(
                    Compiler,
                    LineNo,
                    StringNo,
                    gcSL_DP3,
                    &intermTargets[0],
                    Source,
                    Source);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    /* rsq t1, t0 */
    slsIOPERAND_New(Compiler, &intermIOperands[1], gcSHADER_FLOAT_X1, Source->precision);
    gcsTARGET_InitializeUsingIOperand(&intermTargets[1], &intermIOperands[1]);
    gcsSOURCE_InitializeUsingIOperand(&intermSources[0], &intermIOperands[0]);

    status = _EmitCode(
                    Compiler,
                    LineNo,
                    StringNo,
                    gcSL_RSQ,
                    &intermTargets[1],
                    &intermSources[0],
                    gcvNULL);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    /* mul target, source, t1 */
    gcsSOURCE_InitializeUsingIOperand(&intermSources[1], &intermIOperands[1]);

    status = _EmitCode(
                    Compiler,
                    LineNo,
                    StringNo,
                    gcSL_MUL,
                    Target,
                    Source,
                    &intermSources[1]);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

static gceSTATUS
_EmitExpandedNORM4Code(
    IN sloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gcsTARGET * Target,
    IN gcsSOURCE * Source
    )
{
    gceSTATUS        status;
    slsIOPERAND        intermIOperands[2];
    gcsTARGET        intermTargets[2];
    gcsSOURCE        intermSources[2];

    gcmHEADER();

    gcmASSERT(Target);
    gcmASSERT(Source);

    if (Source == gcvNULL)
    {
        status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
        gcmFOOTER();
        return status;
    }

    /* dp4 t0, source, source */
    slsIOPERAND_New(Compiler, &intermIOperands[0], gcSHADER_FLOAT_X1, Source->precision);
    gcsTARGET_InitializeUsingIOperand(&intermTargets[0], &intermIOperands[0]);

    status = _EmitCode(
                    Compiler,
                    LineNo,
                    StringNo,
                    gcSL_DP4,
                    &intermTargets[0],
                    Source,
                    Source);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    /* rsq t1, t0 */
    slsIOPERAND_New(Compiler, &intermIOperands[1], gcSHADER_FLOAT_X1, Source->precision);
    gcsTARGET_InitializeUsingIOperand(&intermTargets[1], &intermIOperands[1]);
    gcsSOURCE_InitializeUsingIOperand(&intermSources[0], &intermIOperands[0]);

    status = _EmitCode(
                    Compiler,
                    LineNo,
                    StringNo,
                    gcSL_RSQ,
                    &intermTargets[1],
                    &intermSources[0],
                    gcvNULL);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    /* mul target, source, t1 */
    gcsSOURCE_InitializeUsingIOperand(&intermSources[1], &intermIOperands[1]);

    status = _EmitCode(
                    Compiler,
                    LineNo,
                    StringNo,
                    gcSL_MUL,
                    Target,
                    Source,
                    &intermSources[1]);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

/* map to gcSL_NORM for all vector sizes*/
static gceSTATUS
_EmitNormalizeCode(
    IN sloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gcsTARGET * Target,
    IN gcsSOURCE * Source
    )
{
    gceSTATUS   status;

    gcmHEADER();

    gcmASSERT(Target);
    gcmASSERT(Source);
    if (Source == gcvNULL)
    {
        status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
        gcmFOOTER();
        return status;
    }

    switch (Source->dataType)
    {
        /* normalize(float) = sign(float); */
    case gcSHADER_FLOAT_X1:
        status = _EmitCode(Compiler, LineNo, StringNo, gcSL_SIGN,
                               Target, Source, gcvNULL);
        break;

    case gcSHADER_FLOAT_X2:
        if (sloCOMPILER_ExpandNorm(Compiler))
            status = _EmitExpandedNORM2Code(Compiler,LineNo,StringNo,Target,Source);
        else
            status = _EmitCode(Compiler, LineNo, StringNo, gcSL_NORM,
                               Target, Source, gcvNULL);
        break;
    case gcSHADER_FLOAT_X3:
        if (sloCOMPILER_ExpandNorm(Compiler))
            status = _EmitExpandedNORM3Code(Compiler,LineNo,StringNo,Target,Source);
        else
            status = _EmitCode(Compiler, LineNo, StringNo, gcSL_NORM,
                               Target, Source, gcvNULL);
        break;
    case gcSHADER_FLOAT_X4:
        if (sloCOMPILER_ExpandNorm(Compiler))
            status = _EmitExpandedNORM4Code(Compiler,LineNo,StringNo,Target,Source);
        else
            status = _EmitCode(Compiler, LineNo, StringNo, gcSL_NORM,
                               Target, Source, gcvNULL);
        break;
    default:
            status = gcvSTATUS_OK;
        gcmASSERT(0);
    }

    gcmFOOTER();
    return status;
}

typedef gceSTATUS
(* sltEMIT_SPECIAL_CODE_FUNC_PTR1)(
    IN sloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gcsTARGET * Target,
    IN gcsSOURCE * Source
    );

typedef struct _slsSPECIAL_CODE_EMITTER1
{
    sleOPCODE                        opcode;

    sltEMIT_SPECIAL_CODE_FUNC_PTR1    codeEmitter;
}
slsSPECIAL_CODE_EMITTER1;

static slsSPECIAL_CODE_EMITTER1 SpecialCodeEmitterTable1[] =
{
    {slvOPCODE_FLOAT_TO_INT,    _EmitFloatToIntCode},
    {slvOPCODE_FLOAT_TO_BOOL,    _EmitFloatOrIntToBoolCode},
    {slvOPCODE_FLOAT_TO_HALF,    _EmitFloatToHalfCode},
    {slvOPCODE_HALF_TO_FLOAT,    _EmitHalfToFloatCode},
    {slvOPCODE_INT_TO_BOOL,        _EmitFloatOrIntToBoolCode},
    {slvOPCODE_UINT_TO_BOOL,    _EmitFloatOrIntToBoolCode},

    {slvOPCODE_ANY,            _EmitAnyCode},
    {slvOPCODE_ALL,            _EmitAllCode},
    {slvOPCODE_NOT,            _EmitNotCode},

    {slvOPCODE_NORMALIZE,        _EmitNormalizeCode}
};

const gctUINT SpecialCodeEmitterCount1 =
                        sizeof(SpecialCodeEmitterTable1) / sizeof(slsSPECIAL_CODE_EMITTER1);

static gceSTATUS
_EmitCodeImpl1(
    IN sloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN sleOPCODE Opcode,
    IN gcsTARGET * Target,
    IN gcsSOURCE * Source
    )
{
    gceSTATUS                        status;
    gcsSOURCE                        newSource;
    gctUINT                            i;
    sltEMIT_SPECIAL_CODE_FUNC_PTR1    specialCodeEmitter = gcvNULL;

    gcmHEADER();

    if (Source)
    {
        status = _PrepareSource(
                                Compiler,
                                LineNo,
                                StringNo,
                                Target,
                                Source,
                                &newSource);

        if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }
    }

    for (i = 0; i < SpecialCodeEmitterCount1; i++)
    {
        if (SpecialCodeEmitterTable1[i].opcode == Opcode)
        {
            specialCodeEmitter = SpecialCodeEmitterTable1[i].codeEmitter;
            break;
        }
    }

    if (specialCodeEmitter != gcvNULL)
    {
        status = (*specialCodeEmitter)(
                                    Compiler,
                                    LineNo,
                                    StringNo,
                                    Target,
                                    Source ? &newSource : gcvNULL);

        if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }
    }
    else
    {
        status = _EmitCode(
                        Compiler,
                        LineNo,
                        StringNo,
                        _ConvOpcode(Opcode),
                        Target,
                        Source ? &newSource : gcvNULL,
                        gcvNULL);

        if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }
    }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
slEmitCode1(
    IN sloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN sleOPCODE Opcode,
    IN gcsTARGET * Target,
    IN gcsSOURCE * Source
    )
{
    sloCODE_EMITTER    codeEmitter;
    gceSTATUS          status;

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);

    if(_ConvOpcode(Opcode) == gcSL_CONV)
    {
        status =  slEmitConvCode(Compiler,
                              LineNo,
                              StringNo,
                              Target,
                              Source,
                              Source->dataType);
    }
    else
    {
        codeEmitter = Compiler->codeEmitter;
        gcmASSERT(codeEmitter);

        status = sloCODE_EMITTER_EmitCode1(Compiler,
                                           codeEmitter,
                                           LineNo,
                                           StringNo,
                                           Opcode,
                                           Target,
                                           Source);
    }
    gcmFOOTER();
    return status;
}

static void
gcsSOURCE_CONSTANT_Inverse(
    IN OUT gcsSOURCE * Source
    )
{
    gcmASSERT(Source);
    gcmASSERT(Source->type == gcvSOURCE_CONSTANT);

    switch (gcGetComponentDataType(Source->dataType))
    {
    case gcSHADER_FLOAT_X1:
        Source->u.sourceConstant.u.floatConstant =
                (gctFLOAT)1.0 / Source->u.sourceConstant.u.floatConstant;
        break;

    case gcSHADER_INTEGER_X1:
    case gcSHADER_UINT_X1:
        if (Source->dataType == gcSHADER_INTEGER_X1 ||
            Source->dataType == gcSHADER_UINT_X1)
        {
            Source->dataType = gcSHADER_FLOAT_X1;
        }
        else
        {
            Source->dataType = gcConvScalarToVectorDataType(
                                        gcSHADER_FLOAT_X1,
                                        gcGetDataTypeComponentCount(Source->dataType));
        }

        Source->u.sourceConstant.u.floatConstant =
                (gctFLOAT)1.0 / (gctFLOAT)Source->u.sourceConstant.u.intConstant;
        break;

    default:
        gcmASSERT(0);
    }
}

static gceSTATUS
_EmitMulForDivCode(
    IN sloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gcsTARGET * Target,
    IN gcsSOURCE * Source0,
    IN gcsSOURCE * Source1
    )
{
    gceSTATUS        status;
    slsIOPERAND      intermIOperand;
    gcsTARGET        intermTarget;
    gcsSOURCE        intermSource;

    gcmHEADER();

    gcmASSERT(Target);
    gcmASSERT(Source0);
    gcmASSERT(Source1);

    switch (gcGetComponentDataType(Target->dataType))
    {
    case gcSHADER_FLOAT_X1:
        /* mul target, source0, source1 */
        status = _EmitCode(
                        Compiler,
                        LineNo,
                        StringNo,
                        gcSL_MUL,
                        Target,
                        Source0,
                        Source1);

        if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

        break;

    case gcSHADER_INTEGER_X1:
        /* mul t0, source0, source1 */
        slsIOPERAND_New(Compiler, &intermIOperand, Target->dataType, Target->precision);
        gcsTARGET_InitializeUsingIOperand(&intermTarget, &intermIOperand);

        status = _EmitCode(
                        Compiler,
                        LineNo,
                        StringNo,
                        gcSL_MUL,
                        &intermTarget,
                        Source0,
                        Source1);

        if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

        /* floor target, t0 */
        gcsSOURCE_InitializeUsingIOperand(&intermSource, &intermIOperand);

        _EmitFloatToIntCode(Compiler, LineNo, StringNo, Target, &intermSource);

        if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

        break;

    default:
        gcmASSERT(0);
    }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

static gceSTATUS
_EmitDivCode(
    IN sloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gcsTARGET * Target,
    IN gcsSOURCE * Source0,
    IN gcsSOURCE * Source1
    )
{
    gceSTATUS        status;
    slsIOPERAND      intermIOperand;
    gcsTARGET        intermTarget;
    gcsSOURCE        intermSource;
    gcmHEADER();

    gcmASSERT(Target);
    gcmASSERT(Source0);
    gcmASSERT(Source1);


    if(gcIsIntegerDataType(Target->dataType)
#if TREAT_ES20_INTEGER_AS_FLOAT
        && sloCOMPILER_IsHaltiVersion(Compiler)
#endif
      )
    {
        if(Target->precision == gcSHADER_PRECISION_MEDIUM)
        {
            gcSL_FORMAT format = gcSL_INT16;

            if(gcIsUnsignedIntegerDataType(Target->dataType))
            {
                format = gcSL_UINT16;
            }
            status = _EmitCodeFormatted(Compiler,
                                        LineNo,
                                        StringNo,
                                        gcSL_DIV,
                                        format,
                                        Target,
                                        Source0,
                                        Source1);
            if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }
        }
        else
        {
            gcSHADER_TYPE operandType;
            slsIOPERAND intermIOperand[1];
            gcsTARGET intermTarget[1];
            gcsSOURCE intermSource0[1], intermSource1[1];

            operandType = gcConvScalarToVectorDataType(gcSHADER_FLOAT_X1,
                                                       gcGetVectorDataTypeComponentCount(Target->dataType));

             slsIOPERAND_New(Compiler, intermIOperand, operandType, Source0->precision);
             gcsTARGET_InitializeUsingIOperand(intermTarget, intermIOperand);

             status = _EmitCode(Compiler,
                                LineNo,
                                StringNo,
                                gcSL_I2F,
                                intermTarget,
                                Source0,
                                gcvNULL);
            if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

             gcsSOURCE_InitializeUsingIOperand(intermSource0, intermIOperand);

             slsIOPERAND_New(Compiler, intermIOperand, operandType, Source1->precision);
             gcsTARGET_InitializeUsingIOperand(intermTarget, intermIOperand);

             status = _EmitCode(Compiler,
                                LineNo,
                                StringNo,
                                gcSL_I2F,
                                intermTarget,
                                Source1,
                                gcvNULL);
            if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

             gcsSOURCE_InitializeUsingIOperand(intermSource1, intermIOperand);

             slsIOPERAND_New(Compiler, intermIOperand, operandType, Target->precision);
             gcsTARGET_InitializeUsingIOperand(intermTarget, intermIOperand);

             status = _EmitCode(Compiler,
                                LineNo,
                                StringNo,
                                gcSL_DIV,
                                intermTarget,
                                intermSource0,
                                intermSource1);
            if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

             gcsSOURCE_InitializeUsingIOperand(intermSource0, intermIOperand);

             status = _EmitCodeFormatted(Compiler,
                                         LineNo,
                                         StringNo,
                                         gcSL_F2I,
                                         slConvDataTypeToFormat(Compiler, Target->dataType),
                                         Target,
                                         intermSource0,
                                         gcvNULL);
             if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }
        }
    }
    else if (Source1->type == gcvSOURCE_CONSTANT)
    {
        /* mul target, source0, 1 / constant_source1 */
        intermSource = *Source1;
        gcsSOURCE_CONSTANT_Inverse(&intermSource);

        status = _EmitMulForDivCode(Compiler,
                                    LineNo,
                                    StringNo,
                                    Target,
                                    Source0,
                                    &intermSource);

        if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }
    }
    else
    {
        if (slmIsLanguageVersion3_1(Compiler))
        {
            /* ES31 requires more accurate DIV */
            status = _EmitCode(Compiler,
                                LineNo,
                                StringNo,
                                gcSL_DIV,
                                Target,
                                Source0,
                                Source1);

            if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }
        }
        else
        {

            /* rcp t0, source1 */
            slsIOPERAND_New(Compiler, &intermIOperand, Source1->dataType, Source1->precision);
            gcsTARGET_InitializeUsingIOperand(&intermTarget, &intermIOperand);

            status = _EmitCode(
                            Compiler,
                            LineNo,
                            StringNo,
                            gcSL_RCP,
                            &intermTarget,
                            Source1,
                            gcvNULL);

            if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

            /* mul target, source0, t0 */
            gcsSOURCE_InitializeUsingIOperand(&intermSource, &intermIOperand);

            status = _EmitMulForDivCode(
                                        Compiler,
                                        LineNo,
                                        StringNo,
                                        Target,
                                        Source0,
                                        &intermSource);

            if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }
        }
    }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

static gceSTATUS
_EmitDP2Code(
    IN sloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gcsTARGET * Target,
    IN gcsSOURCE * Source0,
    IN gcsSOURCE * Source1
    )
{
    gceSTATUS        status;
    slsIOPERAND        intermIOperand;
    gcsTARGET        intermTarget;
    gcsSOURCE        intermSourceX, intermSourceY;

    gcmHEADER();

    gcmASSERT(Target);
    gcmASSERT(Source0);
    gcmASSERT(Source1);

    /* mul t0, source0, source1 */
    slsIOPERAND_New(Compiler, &intermIOperand, gcSHADER_FLOAT_X2,
        GetHigherPrecison(Source1->precision, Source0->precision));
    gcsTARGET_InitializeUsingIOperand(&intermTarget, &intermIOperand);

    status = _EmitCode(
                    Compiler,
                    LineNo,
                    StringNo,
                    gcSL_MUL,
                    &intermTarget,
                    Source0,
                    Source1);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    /* add target, t0.x, t0.y */
    gcsSOURCE_InitializeUsingIOperandAsVectorComponent(
                                                    &intermSourceX,
                                                    &intermIOperand,
                                                    gcSL_SWIZZLE_XXXX);

    gcsSOURCE_InitializeUsingIOperandAsVectorComponent(
                                                    &intermSourceY,
                                                    &intermIOperand,
                                                    gcSL_SWIZZLE_YYYY);

    status = _EmitCode(
                    Compiler,
                    LineNo,
                    StringNo,
                    gcSL_ADD,
                    Target,
                    &intermSourceX,
                    &intermSourceY);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

static gceSTATUS
_EmitScalarCompareCode(
    IN sloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN sleCONDITION Condition,
    IN gcsTARGET * Target,
    IN gcsSOURCE * Source0,
    IN gcsSOURCE * Source1
    )
{
    gceSTATUS    status;
    gctLABEL    elseLabel, endLabel;
    gcsSOURCE    constSource;

    gcmHEADER();

    elseLabel    = slNewLabel(Compiler);
    endLabel    = slNewLabel(Compiler);

    /* jump else if true */
    status = slEmitCompareBranchCode(
                                    Compiler,
                                    LineNo,
                                    StringNo,
                                    slvOPCODE_JUMP    ,
                                    Condition,
                                    elseLabel,
                                    Source0,
                                    Source1);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    /* mov target, false */
#if TREAT_ES20_INTEGER_AS_FLOAT
    if (sloCOMPILER_IsHaltiVersion(Compiler)) {
#endif
       gcsSOURCE_InitializeBoolConstant(&constSource, gcSHADER_PRECISION_MEDIUM, gcvFALSE);
#if TREAT_ES20_INTEGER_AS_FLOAT
    }
    else {
       gcsSOURCE_InitializeFloatConstant(&constSource, gcSHADER_PRECISION_MEDIUM, slmB2F(gcvFALSE));
    }
#endif

    status = _EmitCode(
                        Compiler,
                        LineNo,
                        StringNo,
                        gcSL_MOV,
                        Target,
                        &constSource,
                        gcvNULL);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    /* jump end */
    status = slEmitAlwaysBranchCode(
                                    Compiler,
                                    LineNo,
                                    StringNo,
                                    slvOPCODE_JUMP,
                                    endLabel);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    /* else: */
    status = slSetLabel(Compiler, LineNo, StringNo, elseLabel);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    /* mov target, true */
#if TREAT_ES20_INTEGER_AS_FLOAT
    if (sloCOMPILER_IsHaltiVersion(Compiler)) {
#endif
       gcsSOURCE_InitializeBoolConstant(&constSource, gcSHADER_PRECISION_MEDIUM, gcvTRUE);
#if TREAT_ES20_INTEGER_AS_FLOAT
    }
    else {
       gcsSOURCE_InitializeFloatConstant(&constSource, gcSHADER_PRECISION_MEDIUM, slmB2F(gcvTRUE));
    }
#endif

    status = _EmitCode(
                        Compiler,
                        LineNo,
                        StringNo,
                        gcSL_MOV,
                        Target,
                        &constSource,
                        gcvNULL);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    /* end: */
    status = slSetLabel(Compiler, LineNo, StringNo, endLabel);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

#if GC_ENABLE_DUAL_FP16
static gceSTATUS
_EmitOpcodeConditionAndTarget(
    IN sloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gcSL_OPCODE Opcode,
    IN gcSL_CONDITION Condition,
    IN gcsTARGET * Target
    )
{
    gceSTATUS    status;
    gcSL_FORMAT format;
    gcSHADER binary;
    gctCHAR  buf[5];
    gctUINT32 srcLoc;

    gcmASSERT(Target);

    srcLoc = GCSL_Build_SRC_LOC(LineNo, StringNo);
    format = slConvDataTypeToFormat(Compiler, Target->dataType);
    gcmVERIFY_OK(sloCOMPILER_GetBinary(Compiler, &binary));

    if (Target->indexMode == gcSL_NOT_INDEXED)
    {
        if (Compiler->context.dumpOptions & slvDUMP_CODE_EMITTER)
        {
            gcmVERIFY_OK(sloCOMPILER_Dump(Compiler,
                                          slvDUMP_CODE_EMITTER,
                                          "gcSHADER_AddOpcodeConditionIndexedWithPrecision(Shader, %s, %s, %d, gcSL_ENABLE_%s, %s, %d, %s, %s);",
                                          GetOpcodeName(Opcode),
                                          _GetConditionName(Condition),
                                          Target->tempRegIndex,
                                          _GetEnableName(Target->enable, buf),
                                          _GetIndexModeName(gcSL_NOT_INDEXED),
                                          0,
                                          _GetFormatName(format),
                                          _GetPrecisionName(Target->precision)));
        }

        status = gcSHADER_AddOpcodeConditionIndexedWithPrecision(binary,
                                                                 Opcode,
                                                                 Condition,
                                                                 (gctUINT16)Target->tempRegIndex,
                                                                 Target->enable,
                                                                 gcSL_NOT_INDEXED,
                                                                 0,
                                                                 format,
                                                                 Target->precision,
                                                                 srcLoc);
    }
    else
    {
        if (Compiler->context.dumpOptions & slvDUMP_CODE_EMITTER)
        {
            gcmVERIFY_OK(sloCOMPILER_Dump(Compiler,
                                          slvDUMP_CODE_EMITTER,
                                          "gcSHADER_AddOpcodeConditionIndexedWithPrecision(Shader, %s, %s, %d, gcSL_ENABLE_%s, %s, %d, %s, %s);",
                                          GetOpcodeName(Opcode),
                                          _GetConditionName(Condition),
                                          Target->tempRegIndex,
                                          _GetEnableName(Target->enable, buf),
                                          _GetIndexModeName(Target->indexMode),
                                          Target->indexRegIndex,
                                          _GetFormatName(format),
                                          _GetPrecisionName(Target->precision)));
        }

        status = gcSHADER_AddOpcodeConditionIndexedWithPrecision(binary,
                                                                 Opcode,
                                                                 Condition,
                                                                 (gctUINT16) Target->tempRegIndex,
                                                                 Target->enable,
                                                                 Target->indexMode,
                                                                 (gctUINT16) Target->indexRegIndex,
                                                                 format,
                                                                 Target->precision,
                                                                 srcLoc);
    }

    if (gcmIS_ERROR(status))
    {
        sloCOMPILER_Report(Compiler,
                           LineNo,
                           StringNo,
                           slvREPORT_INTERNAL_ERROR,
                           "failed to add the opcode");

        return status;
    }

    return gcvSTATUS_OK;
}
#else
static gceSTATUS
_EmitOpcodeConditionAndTarget(
    IN sloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gcSL_OPCODE Opcode,
    IN gcSL_CONDITION Condition,
    IN gcsTARGET * Target
    )
{
    gceSTATUS    status;
    gcSL_FORMAT format;
    gcSHADER binary;
    gctCHAR  buf[5];
    gctUINT32 srcLoc;

    gcmASSERT(Target);

    format = slConvDataTypeToFormat(Compiler, Target->dataType);
    gcmVERIFY_OK(sloCOMPILER_GetBinary(Compiler, &binary));

    srcLoc = GCSL_Build_SRC_LOC(LineNo, StringNo);

    if (Target->indexMode == gcSL_NOT_INDEXED) {
        gcmVERIFY_OK(sloCOMPILER_Dump(Compiler,
                                      slvDUMP_CODE_EMITTER,
                                      "gcSHADER_AddOpcodeConditionIndexed(Shader, %s, %s, %d, gcSL_ENABLE_%s, %s, %d, %s);",
                                      GetOpcodeName(Opcode),
                                      _GetConditionName(Condition),
                                      Target->tempRegIndex,
                                      _GetEnableName(Target->enable, buf),
                                      _GetIndexModeName(gcSL_NOT_INDEXED),
                                      0,
                                      _GetFormatName(format)));

        status = gcSHADER_AddOpcodeConditionIndexed(binary,
                                                    Opcode,
                                                    Condition,
                                                    (gctUINT16)Target->tempRegIndex,
                                                    Target->enable,
                                                    gcSL_NOT_INDEXED,
                                                    0,
                                                    format,
                                                    srcLoc);
    }
    else {
        gcmVERIFY_OK(sloCOMPILER_Dump(Compiler,
                                      slvDUMP_CODE_EMITTER,
                                      "gcSHADER_AddOpcodeConditionIndexed(Shader, %s, %s, %d, gcSL_ENABLE_%s, %s, %d, %s);",
                                      GetOpcodeName(Opcode),
                                      _GetConditionName(Condition),
                                      Target->tempRegIndex,
                                      _GetEnableName(Target->enable, buf),
                                      _GetIndexModeName(Target->indexMode),
                                      Target->indexRegIndex,
                                      _GetFormatName(format)));

        status = gcSHADER_AddOpcodeConditionIndexed(binary,
                                                    Opcode,
                                                    Condition,
                                                    (gctUINT16) Target->tempRegIndex,
                                                    Target->enable,
                                                    Target->indexMode,
                                                    (gctUINT16) Target->indexRegIndex,
                                                    format,
                                                    srcLoc);
    }

    if (gcmIS_ERROR(status)) {
    sloCOMPILER_Report(Compiler,
               LineNo,
               StringNo,
               slvREPORT_INTERNAL_ERROR,
               "failed to add the opcode");

        return status;
    }

    return gcvSTATUS_OK;
}
#endif

static gceSTATUS
_EmitOpcodeConditionAndFormat(
    IN sloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gcSL_OPCODE Opcode,
    IN gcSL_CONDITION Condition,
    IN gcSL_FORMAT Format,
    IN gcSHADER_PRECISION Precision,
    IN gcsTARGET * Target
    )
{
    gceSTATUS    status;
    gcSHADER binary;
    gctCHAR  buf[5];

    gcmASSERT(Target);

    gcmVERIFY_OK(sloCOMPILER_GetBinary(Compiler, &binary));

    if (Compiler->context.dumpOptions & slvDUMP_CODE_EMITTER)
    {
        gcmVERIFY_OK(sloCOMPILER_Dump(Compiler,
                                      slvDUMP_CODE_EMITTER,
                                      "gcSHADER_AddOpcodeConditionIndexedWithPrecision(Shader, %s, %s, %d, gcSL_ENABLE_%s, %s, %d, %s, %s);",
                                      GetOpcodeName(Opcode),
                                      _GetConditionName(Condition),
                                      Target->tempRegIndex,
                                      _GetEnableName(Target->enable, buf),
                                      _GetIndexModeName(Target->indexMode),
                                      Target->indexRegIndex,
                                      _GetFormatName(Format),
                                      _GetPrecisionName(Precision)));
    }

    status = gcSHADER_AddOpcodeConditionIndexedWithPrecision(binary,
                                                             Opcode,
                                                             Condition,
                                                             (gctUINT16) Target->tempRegIndex,
                                                             Target->enable,
                                                             Target->indexMode,
                                                             (gctUINT16) Target->indexRegIndex,
                                                             Format,
                                                             Precision,
                                                             GCSL_Build_SRC_LOC(LineNo, StringNo));

    if (gcmIS_ERROR(status))
    {
        sloCOMPILER_Report(Compiler,
                           LineNo,
                           StringNo,
                           slvREPORT_INTERNAL_ERROR,
                           "failed to add the opcode");

        return status;
    }

    return gcvSTATUS_OK;
}

static gceSTATUS
_EmitCompareSetCode(
    IN sloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gcSHADER_TYPE Type,
    IN gcsTARGET * Target,
    IN gcsSOURCE * Cond
    )
{
   gceSTATUS status;
   gcsSOURCE trueSource[1];

   gcmASSERT(Target);
   gcmASSERT(Cond);

#if TREAT_ES20_INTEGER_AS_FLOAT
    if (sloCOMPILER_IsHaltiVersion(Compiler))
    {
#endif
        if (Type == gcSHADER_FLOAT_X1)
        {
            gcsSOURCE_InitializeFloatConstant(trueSource, gcSHADER_PRECISION_MEDIUM, slmB2F(gcvTRUE));
        }
        else
        {
            gcsSOURCE_InitializeBoolConstant(trueSource, gcSHADER_PRECISION_MEDIUM, gcvTRUE);
        }
#if TREAT_ES20_INTEGER_AS_FLOAT
    }
    else
    {
        gcsSOURCE_InitializeFloatConstant(trueSource, gcSHADER_PRECISION_MEDIUM, slmB2F(gcvTRUE));
    }
#endif

   status = _EmitOpcodeConditionAndTarget(Compiler,
                                          LineNo,
                                          StringNo,
                                          gcSL_SET,
                                          gcSL_ZERO,
                                          Target);
   if (gcmIS_ERROR(status)) return status;

   status = _EmitSource(Compiler,
            LineNo,
            StringNo,
            Cond);
   if (gcmIS_ERROR(status)) return status;

   status =  _EmitSource(Compiler,
                 LineNo,
                 StringNo,
                 Cond);
   if (gcmIS_ERROR(status)) return status;

   status = _EmitOpcodeConditionAndTarget(Compiler,
                                          LineNo,
                                          StringNo,
                                          gcSL_SET,
                                          gcSL_NOT_ZERO,
                                          Target);
   if (gcmIS_ERROR(status)) return status;

   status = _EmitSource(Compiler,
            LineNo,
            StringNo,
            Cond);
   if (gcmIS_ERROR(status)) return status;

   return _EmitSource(Compiler,
              LineNo,
              StringNo,
              trueSource);
}

gceSTATUS
slEmitCompareSetCode(
    IN sloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN sleCONDITION Condition,
    IN gcsTARGET *Target,
    IN gcsSOURCE * Source0,
    IN gcsSOURCE * Source1
    )
{
   gceSTATUS status;
   gcsSOURCE newSource0[1], newSource1[1];
   gcsTARGET intermTarget[2];
   gcsSOURCE intermSource[2];
   slsIOPERAND intermIOperand[2];
   gcSHADER_TYPE type, intermTargetType;

   gcmASSERT(Source0);
   gcmASSERT(Source1);

#if TREAT_ES20_INTEGER_AS_FLOAT
    if (sloCOMPILER_IsHaltiVersion(Compiler))
    {
#endif
        if (gcGetComponentDataType(Source0->dataType) == gcSHADER_FLOAT_X1)
        {
            type = gcSHADER_FLOAT_X1;
        }
        else
        {
            type = gcSHADER_INTEGER_X1;
        }
#if TREAT_ES20_INTEGER_AS_FLOAT
    }
    else
    {
        type = gcSHADER_FLOAT_X1;
    }
#endif

   /* cmp source0, source1 */
   intermTargetType = gcChangeElementDataType(Target->dataType,
                                              type);
   slsIOPERAND_New(Compiler, &intermIOperand[0], intermTargetType, Target->precision);
   gcsTARGET_InitializeUsingIOperand(&intermTarget[0], &intermIOperand[0]);
   gcsSOURCE_InitializeUsingIOperand(&intermSource[0], &intermIOperand[0]);

   status = _PrepareSource(Compiler,
                           LineNo,
                           StringNo,
                           Target,
                           Source0,
                           newSource0);
   if (gcmIS_ERROR(status)) return status;

   status = _PrepareAnotherSource(Compiler,
                                  LineNo,
                                  StringNo,
                                  Target,
                                  newSource0,
                                  Source1,
                                  newSource1);
   if (gcmIS_ERROR(status)) return status;

   status = _EmitOpcodeConditionAndTarget(Compiler,
                                          LineNo,
                                          StringNo,
                                          gcSL_CMP,
                                          _ConvCondition(Condition),
                                          &intermTarget[0]);
   if (gcmIS_ERROR(status)) return status;

   status = _EmitSource(Compiler,
            LineNo,
            StringNo,
            newSource0);
   if (gcmIS_ERROR(status)) return status;

   status = _EmitSource(Compiler,
                        LineNo,
                        StringNo,
                        newSource1);
   if (gcmIS_ERROR(status)) return status;

    /* when using cmp/set pattern to generate compare functions, we would use the data type of source0 for target.
    ** So if source0 is a float and the target is a bool, we need to add a F2I for target.
    */
    if (
#if TREAT_ES20_INTEGER_AS_FLOAT
        sloCOMPILER_IsHaltiVersion(Compiler) &&
#endif
        (type == gcSHADER_FLOAT_X1))
    {
        slsIOPERAND_New(Compiler, &intermIOperand[1], intermTargetType, Target->precision);
        gcsTARGET_InitializeUsingIOperand(&intermTarget[1], &intermIOperand[1]);
        gcsSOURCE_InitializeUsingIOperand(&intermSource[1], &intermIOperand[1]);

        status = _EmitCompareSetCode(Compiler,
                              LineNo,
                              StringNo,
                              type,
                              &intermTarget[1],
                              &intermSource[0]);
        if (gcmIS_ERROR(status)) return status;

        return _EmitCode(Compiler,
                           LineNo,
                           StringNo,
                           gcSL_F2I,
                           Target,
                           &intermSource[1],
                           gcvNULL);
    }
    else
    {
        return _EmitCompareSetCode(Compiler,
                              LineNo,
                              StringNo,
                              type,
                              Target,
                              &intermSource[0]);
    }
}

gceSTATUS
slEmitSelectCode(
    IN sloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gcsTARGET *Target,
    IN gcsSOURCE * Cond,
    IN gcsSOURCE * Source0,
    IN gcsSOURCE * Source1
    )
{
    gceSTATUS status;
    gcsSOURCE newCond, newSource0, newSource1;
    sloCODE_EMITTER codeEmitter;

    gcmHEADER();
    gcmASSERT(Target);
    gcmASSERT(Cond);
    gcmASSERT(Source0);
    gcmASSERT(Source1);

    codeEmitter = Compiler->codeEmitter;
    gcmASSERT(codeEmitter);

    /* flush out previous instructions */
    status = sloCODE_EMITTER_EndBasicBlock(Compiler, codeEmitter);
    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    status = _PrepareSource(Compiler,
                            LineNo,
                            StringNo,
                            Target,
                            Cond,
                            &newCond);
    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    status = _PrepareAnotherSource(Compiler,
                                   LineNo,
                                   StringNo,
                                   Target,
                                   &newCond,
                                   Source0,
                                   &newSource0);
    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    status = _PrepareAnotherSource(Compiler,
                                   LineNo,
                                   StringNo,
                                   Target,
                                   &newCond,
                                   Source1,
                                   &newSource1);
    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    status = _EmitOpcodeConditionAndTarget(Compiler,
                                           LineNo,
                                           StringNo,
                                           gcSL_SET,
                                           gcSL_ZERO,
                                           Target);
    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    status = _EmitSource(Compiler,
                         LineNo,
                         StringNo,
                         &newCond);
    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    status = _EmitSource(Compiler,
                         LineNo,
                         StringNo,
                         &newSource0);
    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    status = _EmitOpcodeConditionAndTarget(Compiler,
                                           LineNo,
                                           StringNo,
                                           gcSL_SET,
                                           gcSL_NOT_ZERO,
                                           Target);
    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    status = _EmitSource(Compiler,
                         LineNo,
                         StringNo,
                         &newCond);
    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    status = _EmitSource(Compiler,
                         LineNo,
                         StringNo,
                         &newSource1);
    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

static gceSTATUS
_EmitCompareCode(
    IN sloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN sleCONDITION Condition,
    IN gcsTARGET * Target,
    IN gcsSOURCE * Source0,
    IN gcsSOURCE * Source1
    )
{
    gceSTATUS    status;

    gcmHEADER();

    gcmASSERT(Target);
    gcmASSERT(Source0);
    gcmASSERT(Source1);

    if (Target->dataType == gcSHADER_BOOLEAN_X1)
    {
        status = _EmitScalarCompareCode(Compiler,
                                        LineNo,
                                        StringNo,
                                        Condition,
                                        Target,
                                        Source0,
                                        Source1);

        if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }
    }
    else
    {
        gcsTARGET    componentTarget;
        gcsSOURCE    componentSource0, componentSource1;
        gctUINT i;

        /* Don't generate cmp/set instructions for compare functions for the chips that don't support cmp/set. */
        if (GetHWHasHalti0())
        {
            status = slEmitCompareSetCode(Compiler,
                                          LineNo,
                                          StringNo,
                                          Condition,
                                          Target,
                                          Source0,
                                          Source1);
            if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }
        }
        else
        {
            gcmASSERT(Target->dataType == gcSHADER_BOOLEAN_X2 ||
                      Target->dataType == gcSHADER_BOOLEAN_X3 ||
                      Target->dataType == gcSHADER_BOOLEAN_X4);

            for (i = 0; i < gcGetVectorDataTypeComponentCount(Target->dataType); i++)
            {
                gcsTARGET_InitializeAsVectorComponent(&componentTarget, Target, i);
                gcsSOURCE_InitializeAsVectorComponent(&componentSource0, Source0, i);
                gcsSOURCE_InitializeAsVectorComponent(&componentSource1, Source1, i);

                status = _EmitScalarCompareCode(Compiler,
                                                LineNo,
                                                StringNo,
                                                Condition,
                                                &componentTarget,
                                                &componentSource0,
                                                &componentSource1);

                if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }
            }
        }
    }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

static gceSTATUS
_EmitLessThanCode(
    IN sloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gcsTARGET * Target,
    IN gcsSOURCE * Source0,
    IN gcsSOURCE * Source1
    )
{
    gceSTATUS status;

    gcmHEADER();

    gcmASSERT(Target);
    gcmASSERT(Source0);
    gcmASSERT(Source1);

    status = _EmitCompareCode(
                            Compiler,
                            LineNo,
                            StringNo,
                            slvCONDITION_LESS_THAN,
                            Target,
                            Source0,
                            Source1);

    gcmFOOTER();
    return status;
}

static gceSTATUS
_EmitLessThanEqualCode(
    IN sloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gcsTARGET * Target,
    IN gcsSOURCE * Source0,
    IN gcsSOURCE * Source1
    )
{
    gceSTATUS status;

    gcmHEADER();

    gcmASSERT(Target);
    gcmASSERT(Source0);
    gcmASSERT(Source1);

    status = _EmitCompareCode(
                            Compiler,
                            LineNo,
                            StringNo,
                            slvCONDITION_LESS_THAN_EQUAL,
                            Target,
                            Source0,
                            Source1);

    gcmFOOTER();
    return status;
}

static gceSTATUS
_EmitGreaterThanCode(
    IN sloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gcsTARGET * Target,
    IN gcsSOURCE * Source0,
    IN gcsSOURCE * Source1
    )
{
    gceSTATUS status;

    gcmHEADER();

    gcmASSERT(Target);
    gcmASSERT(Source0);
    gcmASSERT(Source1);

    status = _EmitCompareCode(
                            Compiler,
                            LineNo,
                            StringNo,
                            slvCONDITION_GREATER_THAN,
                            Target,
                            Source0,
                            Source1);

    gcmFOOTER();
    return status;
}

static gceSTATUS
_EmitGreaterThanEqualCode(
    IN sloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gcsTARGET * Target,
    IN gcsSOURCE * Source0,
    IN gcsSOURCE * Source1
    )
{
    gceSTATUS status;

    gcmHEADER();

    gcmASSERT(Target);
    gcmASSERT(Source0);
    gcmASSERT(Source1);

    status = _EmitCompareCode(
                            Compiler,
                            LineNo,
                            StringNo,
                            slvCONDITION_GREATER_THAN_EQUAL,
                            Target,
                            Source0,
                            Source1);

    gcmFOOTER();
    return status;
}

static gceSTATUS
_EmitEqualCode(
    IN sloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gcsTARGET * Target,
    IN gcsSOURCE * Source0,
    IN gcsSOURCE * Source1
    )
{
    gceSTATUS status;

    gcmHEADER();

    gcmASSERT(Target);
    gcmASSERT(Source0);
    gcmASSERT(Source1);

    status = _EmitCompareCode(
                            Compiler,
                            LineNo,
                            StringNo,
                            slvCONDITION_EQUAL,
                            Target,
                            Source0,
                            Source1);

    gcmFOOTER();
    return status;
}

static gceSTATUS
_EmitNotEqualCode(
    IN sloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gcsTARGET * Target,
    IN gcsSOURCE * Source0,
    IN gcsSOURCE * Source1
    )
{
    gceSTATUS status;

    gcmHEADER();

    gcmASSERT(Target);
    gcmASSERT(Source0);
    gcmASSERT(Source1);

    status = _EmitCompareCode(
                            Compiler,
                            LineNo,
                            StringNo,
                            slvCONDITION_NOT_EQUAL,
                            Target,
                            Source0,
                            Source1);

    gcmFOOTER();
    return status;
}

static gceSTATUS
_EmitDotCode(
    IN sloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gcsTARGET * Target,
    IN gcsSOURCE * Source0,
    IN gcsSOURCE * Source1
    )
{
    gceSTATUS status;

    gcmHEADER();

    gcmASSERT(Target);
    gcmASSERT(Source0);
    gcmASSERT(Source1);

    switch (Source0->dataType)
    {
    case gcSHADER_FLOAT_X1:
        status = _EmitCode(
                        Compiler,
                        LineNo,
                        StringNo,
                        gcSL_MUL,
                        Target,
                        Source0,
                        Source1);
        break;

    case gcSHADER_FLOAT_X2:
        status = _EmitCode(
                        Compiler,
                        LineNo,
                        StringNo,
                        gcSL_DP2,
                        Target,
                        Source0,
                        Source1);
        break;

    case gcSHADER_FLOAT_X3:
        status = _EmitCode(
                        Compiler,
                        LineNo,
                        StringNo,
                        gcSL_DP3,
                        Target,
                        Source0,
                        Source1);
        break;

    case gcSHADER_FLOAT_X4:
        status = _EmitCode(
                        Compiler,
                        LineNo,
                        StringNo,
                        gcSL_DP4,
                        Target,
                        Source0,
                        Source1);
        break;

    default:
        status = gcvSTATUS_OK;
        gcmASSERT(0);
    }

    gcmFOOTER();
    return status;
}

static gceSTATUS
_EmitScalarOrVectorModDivCode(
                              IN sloCOMPILER Compiler,
                              IN gctUINT LineNo,
                              IN gctUINT StringNo,
                              IN gcSL_OPCODE Opcode,
                              IN gcSL_FORMAT Format,
                              IN gcsTARGET * Target,
                              IN gcsSOURCE * Source0,
                              IN gcsSOURCE * Source1
                              )
{
    gceSTATUS   status = gcvSTATUS_OK;
    gctUINT8    i;
    gcsTARGET   componentTarget;
    gcsSOURCE   componentSource0, componentSource1;

    gcmHEADER();

    gcmASSERT(Target);
    gcmASSERT(Source0);
    gcmASSERT(Source1);

    if(gcGetDataTypeComponentCount(Target->dataType) != 1)
    {
        for (i = 0; i < gcGetVectorDataTypeComponentCount(Target->dataType); i++) {

            gcsTARGET_InitializeAsVectorComponent(&componentTarget, Target, i);

            if(gcGetDataTypeComponentCount(Source0->dataType) != 1)
            {
                gcsSOURCE_InitializeAsVectorComponent(&componentSource0, Source0, i);
            }
            else
            {
                status = _MakeNewSource(
                    Compiler,
                    LineNo,
                    StringNo,
                    Source0,
                    &componentSource0
                    );
                if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }
            }

            if(gcGetDataTypeComponentCount(Source1->dataType) != 1)
            {
                gcsSOURCE_InitializeAsVectorComponent(&componentSource1, Source1, i);
            }
            else
            {
                status = _MakeNewSource(
                    Compiler,
                    LineNo,
                    StringNo,
                    Source1,
                    &componentSource1
                    );
                if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }
            }

            status = _EmitCodeFormatted(Compiler,
                                        LineNo,
                                        StringNo,
                                        Opcode,
                                        Format,
                                        &componentTarget,
                                        &componentSource0,
                                        &componentSource1);
            if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }
        }
    }
    else
    {
        status = _EmitCodeFormatted(Compiler,
                                    LineNo,
                                    StringNo,
                                    Opcode,
                                    Format,
                                    Target,
                                    Source0,
                                    Source1);
        if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }
    }

    gcmFOOTER();
    return status;
}

#if !_GEN_MOD_IN_BACKEND
static gceSTATUS
_EmitModCode(
             IN sloCOMPILER Compiler,
             IN gctUINT LineNo,
             IN gctUINT StringNo,
             IN gcsTARGET * Target,
             IN gcsSOURCE * Source0,
             IN gcsSOURCE * Source1
             )
{
    gceSTATUS status = gcvSTATUS_OK;
    gcSL_FORMAT format;
    gctUINT8 i;
    gcsTARGET   componentTarget;
    gcsSOURCE   componentSource0, componentSource1;

    gcmHEADER();

    gcmASSERT(Target);
    gcmASSERT(Source0);
    gcmASSERT(Source1);

    format = slConvDataTypeToFormat(Compiler,
        Target->dataType);

    if (format == gcSL_INT8 ||
        format == gcSL_UINT8 ||
        format == gcSL_INT16 ||
        format == gcSL_UINT16 ||
        format == gcSL_INTEGER ||
        format == gcSL_UINT32)
    {
        if(gcGetDataTypeComponentCount(Target->dataType) != 1)
        {
            for (i = 0; i < gcGetVectorDataTypeComponentCount(Target->dataType); i++)
            {
                gcsTARGET_InitializeAsVectorComponent(&componentTarget, Target, i);

                if(gcGetDataTypeComponentCount(Source0->dataType) != 1)
                {
                    gcsSOURCE_InitializeAsVectorComponent(&componentSource0, Source0, i);
                }
                else
                {
                    status = _MakeNewSource(
                        Compiler,
                        LineNo,
                        StringNo,
                        Source0,
                        &componentSource0
                        );
                    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }
                }

                if(gcGetDataTypeComponentCount(Source1->dataType) != 1)
                {
                    gcsSOURCE_InitializeAsVectorComponent(&componentSource1, Source1, i);
                }
                else
                {
                    status = _MakeNewSource(
                        Compiler,
                        LineNo,
                        StringNo,
                        Source1,
                        &componentSource1
                        );
                    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }
                }

                status = _EmitCodeFormatted(Compiler,
                                            LineNo,
                                            StringNo,
                                            gcSL_MOD,
                                            format,
                                            &componentTarget,
                                            &componentSource0,
                                            &componentSource1);

                if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }
            }
        }
        else
        {
            status = _EmitCodeFormatted(Compiler,
                LineNo,
                StringNo,
                gcSL_MOD,
                format,
                Target,
                Source0,
                Source1);

            if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }
        }
    }
    else
    {
        gcmASSERT(format == gcSL_INTEGER || format == gcSL_UINT32);
        status = _EmitScalarOrVectorModDivCode(Compiler,
            LineNo,
            StringNo,
            gcSL_MOD,
            format,
            Target,
            Source0,
            Source1);

        if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }
    }
    gcmFOOTER();
    return status;
}
#endif

static gceSTATUS
_EmitIdivCode(
              IN sloCOMPILER Compiler,
              IN gctUINT LineNo,
              IN gctUINT StringNo,
              IN gcsTARGET * Target,
              IN gcsSOURCE * Source0,
              IN gcsSOURCE * Source1
              )
{
    gceSTATUS status;
    gcSL_FORMAT format;

    gcmHEADER();

    gcmASSERT(Target);
    gcmASSERT(Source0);
    gcmASSERT(Source1);

    format = slConvDataTypeToFormat(Compiler,
        Target->dataType);

    if(format == gcSL_INT8 ||
        format == gcSL_UINT8 ||
        format == gcSL_INT16 ||
        format == gcSL_UINT16)
    {

            status = _EmitCodeFormatted(Compiler,
                LineNo,
                StringNo,
                gcSL_DIV,
                format,
                Target,
                Source0,
                Source1);

            if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }
    }
    else
    {
        gcmASSERT(format == gcSL_INTEGER || format == gcSL_UINT32);
        status = _EmitScalarOrVectorModDivCode(Compiler,
            LineNo,
            StringNo,
            gcSL_DIV,
            format,
            Target,
            Source0,
            Source1);

        if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }
    }

    gcmFOOTER();
    return gcvSTATUS_OK;
}

static gceSTATUS
_EmitLoadCode(
              IN sloCOMPILER Compiler,
              IN gctUINT LineNo,
              IN gctUINT StringNo,
              IN gcsTARGET * Target,
              IN gcsSOURCE * Source0,
              IN gcsSOURCE * Source1
              )
{
    gceSTATUS status = gcvSTATUS_OK;

    status = _EmitCode(
                    Compiler,
                    LineNo,
                    StringNo,
                    gcSL_LOAD,
                    Target,
                    Source0,
                    Source1);

    if (Target->dataType == gcSHADER_BOOLEAN_X1 ||
        Target->dataType == gcSHADER_BOOLEAN_X2 ||
        Target->dataType == gcSHADER_BOOLEAN_X3 ||
        Target->dataType == gcSHADER_BOOLEAN_X4)
    {
        slsIOPERAND iOperand;
        gcsTARGET target;
        gcsSOURCE source, newSource;
        gcsSOURCE zeroSource, oneSource;

        slsIOPERAND_New(Compiler, &iOperand, Target->dataType, Target->precision);
        gcsTARGET_InitializeUsingIOperand(&target, &iOperand);
        gcsSOURCE_InitializeUintConstant(&zeroSource, Target->precision, 0);
        gcsSOURCE_InitializeUintConstant(&oneSource, Target->precision, 1);

        gcsSOURCE_InitializeTempReg(&source,
                                    (Target)->dataType,
                                    (Target)->precision,
                                    (Target)->tempRegIndex,
                                    _ConvertEnable2Swizzle(Target->enable),
                                    gcSL_NOT_INDEXED,
                                    0);

        gcmONERROR(slEmitSelectCode(Compiler,
                                    LineNo,
                                    StringNo,
                                    &target,
                                    &source,
                                    &zeroSource,
                                    &oneSource));

        gcsSOURCE_InitializeUsingIOperand(&newSource, &iOperand);

        gcmONERROR(_EmitCodeImpl1(Compiler,
                                    LineNo,
                                    StringNo,
                                    slvOPCODE_ASSIGN,
                                    Target,
                                    &newSource));
    }

    if (gcmIS_ERROR(status)) { return status; }

OnError:
    return status;
}

static gceSTATUS
_EmitStoreCode(
              IN sloCOMPILER Compiler,
              IN gctUINT LineNo,
              IN gctUINT StringNo,
              IN gcsTARGET * Target,
              IN gcsSOURCE * Source0,
              IN gcsSOURCE * Source1
              )
{
    gceSTATUS status = gcvSTATUS_OK;
    gcsSOURCE newSource;
    const gctINT enableCount[16] = {0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4};
    gctINT i, j;

    if (Target->dataType == gcSHADER_BOOLEAN_X1 ||
        Target->dataType == gcSHADER_BOOLEAN_X2 ||
        Target->dataType == gcSHADER_BOOLEAN_X3 ||
        Target->dataType == gcSHADER_BOOLEAN_X4)
    {
        slsIOPERAND iOperand;
        gcsTARGET target;
        gcsSOURCE zeroSource, oneSource;
        gcSL_SWIZZLE newSwizzle[4];

        slsIOPERAND_New(Compiler, &iOperand, Target->dataType, Target->precision);
        gcsTARGET_InitializeUsingIOperand(&target, &iOperand);
        gcsSOURCE_InitializeUintConstant(&zeroSource, Target->precision, 0);
        gcsSOURCE_InitializeUintConstant(&oneSource, Target->precision, 1);

        if (Source1->type != gcvSOURCE_CONSTANT)
        {
            for (i = 0; i < enableCount[Target->enable]; i++)
            {
                newSwizzle[i] = gcmExtractSwizzle(Source1->u.sourceReg.swizzle, i);
            }
            for (j = i; j < 4; j++)
            {
                if (i == 0)
                {
                    newSwizzle[j] = gcSL_SWIZZLE_X;
                }
                else
                {
                    newSwizzle[j] = newSwizzle[i - 1];
                }
            }
            Source1->u.sourceReg.swizzle = (gctUINT8)gcmComposeSwizzle(newSwizzle[0],
                                                             newSwizzle[1],
                                                             newSwizzle[2],
                                                             newSwizzle[3]);
        }

        gcmONERROR(slEmitSelectCode(Compiler,
                                    LineNo,
                                    StringNo,
                                    &target,
                                    Source1,
                                    &zeroSource,
                                    &oneSource));

        gcsSOURCE_InitializeUsingIOperand(&newSource, &iOperand);
    }
    else
    {
        newSource = *Source1;
    }

    status = _EmitCode(
                    Compiler,
                    LineNo,
                    StringNo,
                    gcSL_STORE1,
                    Target,
                    Source0,
                    &newSource);

    if (gcmIS_ERROR(status)) { return status; }

OnError:
    return status;
}

static gceSTATUS
_EmitLoadLCode(
              IN sloCOMPILER Compiler,
              IN gctUINT LineNo,
              IN gctUINT StringNo,
              IN gcsTARGET * Target,
              IN gcsSOURCE * Source0,
              IN gcsSOURCE * Source1
              )
{
    gceSTATUS status = gcvSTATUS_OK;

    status = _EmitCode(
                    Compiler,
                    LineNo,
                    StringNo,
                    gcSL_LOAD_L,
                    Target,
                    Source0,
                    Source1);

    if (Target->dataType == gcSHADER_BOOLEAN_X1 ||
        Target->dataType == gcSHADER_BOOLEAN_X2 ||
        Target->dataType == gcSHADER_BOOLEAN_X3 ||
        Target->dataType == gcSHADER_BOOLEAN_X4)
    {
        slsIOPERAND iOperand;
        gcsTARGET target;
        gcsSOURCE source, newSource;
        gcsSOURCE zeroSource, oneSource;

        slsIOPERAND_New(Compiler, &iOperand, Target->dataType, Target->precision);
        gcsTARGET_InitializeUsingIOperand(&target, &iOperand);
        gcsSOURCE_InitializeUintConstant(&zeroSource, Target->precision, 0);
        gcsSOURCE_InitializeUintConstant(&oneSource, Target->precision, 1);

        gcsSOURCE_InitializeTempReg(&source,
                                    (Target)->dataType,
                                    (Target)->precision,
                                    (Target)->tempRegIndex,
                                    _ConvertEnable2Swizzle(Target->enable),
                                    gcSL_NOT_INDEXED,
                                    0);

        gcmONERROR(slEmitSelectCode(Compiler,
                                    LineNo,
                                    StringNo,
                                    &target,
                                    &source,
                                    &zeroSource,
                                    &oneSource));

        gcsSOURCE_InitializeUsingIOperand(&newSource, &iOperand);

        gcmONERROR(_EmitCodeImpl1(Compiler,
                                    LineNo,
                                    StringNo,
                                    slvOPCODE_ASSIGN,
                                    Target,
                                    &newSource));
    }

    if (gcmIS_ERROR(status)) { return status; }

OnError:
    return status;
}

static gceSTATUS
_EmitStoreLCode(
              IN sloCOMPILER Compiler,
              IN gctUINT LineNo,
              IN gctUINT StringNo,
              IN gcsTARGET * Target,
              IN gcsSOURCE * Source0,
              IN gcsSOURCE * Source1
              )
{
    gceSTATUS status = gcvSTATUS_OK;
    gcsSOURCE newSource;
    const gctINT enableCount[16] = {0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4};
    gctINT i, j;

    if (Target->dataType == gcSHADER_BOOLEAN_X1 ||
        Target->dataType == gcSHADER_BOOLEAN_X2 ||
        Target->dataType == gcSHADER_BOOLEAN_X3 ||
        Target->dataType == gcSHADER_BOOLEAN_X4)
    {
        slsIOPERAND iOperand;
        gcsTARGET target;
        gcsSOURCE zeroSource, oneSource;
        gcSL_SWIZZLE newSwizzle[4];

        slsIOPERAND_New(Compiler, &iOperand, Target->dataType, Target->precision);
        gcsTARGET_InitializeUsingIOperand(&target, &iOperand);
        gcsSOURCE_InitializeUintConstant(&zeroSource, Target->precision, 0);
        gcsSOURCE_InitializeUintConstant(&oneSource, Target->precision, 1);

        if (Source1->type != gcvSOURCE_CONSTANT)
        {
            for (i = 0; i < enableCount[Target->enable]; i++)
            {
                newSwizzle[i] = gcmExtractSwizzle(Source1->u.sourceReg.swizzle, i);
            }
            for (j = i; j < 4; j++)
            {
                if (i == 0)
                {
                    newSwizzle[j] = gcSL_SWIZZLE_X;
                }
                else
                {
                    newSwizzle[j] = newSwizzle[i - 1];
                }
            }
            Source1->u.sourceReg.swizzle = (gctUINT8)gcmComposeSwizzle(newSwizzle[0],
                                                             newSwizzle[1],
                                                             newSwizzle[2],
                                                             newSwizzle[3]);
        }

        gcmONERROR(slEmitSelectCode(Compiler,
                                    LineNo,
                                    StringNo,
                                    &target,
                                    Source1,
                                    &zeroSource,
                                    &oneSource));

        gcsSOURCE_InitializeUsingIOperand(&newSource, &iOperand);
    }
    else
    {
        newSource = *Source1;
    }

    status = _EmitCode(
                    Compiler,
                    LineNo,
                    StringNo,
                    gcSL_STORE_L,
                    Target,
                    Source0,
                    &newSource);

    if (gcmIS_ERROR(status)) { return status; }

OnError:
    return status;
}

typedef gceSTATUS
(* sltEMIT_SPECIAL_CODE_FUNC_PTR2)(
    IN sloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gcsTARGET * Target,
    IN gcsSOURCE * Source0,
    IN gcsSOURCE * Source1
    );

typedef struct _slsSPECIAL_CODE_EMITTER2
{
    sleOPCODE                        opcode;

    sltEMIT_SPECIAL_CODE_FUNC_PTR2    codeEmitter;
}
slsSPECIAL_CODE_EMITTER2;

static slsSPECIAL_CODE_EMITTER2 SpecialCodeEmitterTable2[] =
{
    {slvOPCODE_DIV,                _EmitDivCode},

    {slvOPCODE_LESS_THAN,          _EmitLessThanCode},
    {slvOPCODE_LESS_THAN_EQUAL,    _EmitLessThanEqualCode},
    {slvOPCODE_GREATER_THAN,       _EmitGreaterThanCode},
    {slvOPCODE_GREATER_THAN_EQUAL, _EmitGreaterThanEqualCode},
    {slvOPCODE_EQUAL,              _EmitEqualCode},
    {slvOPCODE_NOT_EQUAL,          _EmitNotEqualCode},

    {slvOPCODE_DOT,                _EmitDotCode},
#if !_GEN_MOD_IN_BACKEND
    {slvOPCODE_MOD,                _EmitModCode},
#endif
    {slvOPCODE_IDIV,               _EmitIdivCode},
    {slvOPCODE_LOAD,               _EmitLoadCode},
    {slvOPCODE_STORE1,             _EmitStoreCode},
    {slvOPCODE_LOAD_L,             _EmitLoadLCode},
    {slvOPCODE_STORE_L,            _EmitStoreLCode},
};

const gctUINT SpecialCodeEmitterCount2 =
                                       sizeof(SpecialCodeEmitterTable2) / sizeof(slsSPECIAL_CODE_EMITTER2);

static gceSTATUS
_EmitCodeImpl2(
    IN sloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN sleOPCODE Opcode,
    IN gcsTARGET * Target,
    IN gcsSOURCE * Source0,
    IN gcsSOURCE * Source1
    )
{
    gceSTATUS    status;
    gcsSOURCE    newSource0, newSource1;
    gctUINT      i;
    sltEMIT_SPECIAL_CODE_FUNC_PTR2    specialCodeEmitter = gcvNULL;

    gcmHEADER();

    if(Opcode == slvOPCODE_LOAD ||
       Opcode == slvOPCODE_STORE1 ||
       Opcode == slvOPCODE_LOAD_L ||
       Opcode == slvOPCODE_STORE_L ||
       Opcode == slvOPCODE_ATTR_LD ||
       Opcode == slvOPCODE_ATTR_ST)
    {
        newSource0 = *Source0;
        newSource1 = *Source1;
    }
    else
    {
        status = _PrepareSource(Compiler,
                                LineNo,
                                StringNo,
                                Target,
                                Source0,
                                &newSource0);
        if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

        status = _PrepareAnotherSource(Compiler,
                                       LineNo,
                                       StringNo,
                                       Target,
                                       &newSource0,
                                       Source1,
                                       &newSource1);
        if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }
    }

    for (i = 0; i < SpecialCodeEmitterCount2; i++)
    {
        if (SpecialCodeEmitterTable2[i].opcode == Opcode)
        {
            specialCodeEmitter = SpecialCodeEmitterTable2[i].codeEmitter;
            break;
        }
    }

    if (specialCodeEmitter != gcvNULL)
    {
        status = (*specialCodeEmitter)(
                                    Compiler,
                                    LineNo,
                                    StringNo,
                                    Target,
                                    &newSource0,
                                    &newSource1);

        if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }
    }
    else
    {
        status = _EmitCode(
                        Compiler,
                        LineNo,
                        StringNo,
                        _ConvOpcode(Opcode),
                        Target,
                        &newSource0,
                        &newSource1);

        if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }
    }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
slEmitCode2(
    IN sloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN sleOPCODE Opcode,
    IN gcsTARGET * Target,
    IN gcsSOURCE * Source0,
    IN gcsSOURCE * Source1
    )
{
    sloCODE_EMITTER        codeEmitter;
    gceSTATUS           status;

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);

    codeEmitter = Compiler->codeEmitter;
    gcmASSERT(codeEmitter);

    status = sloCODE_EMITTER_EmitCode2(
                                    Compiler,
                                    codeEmitter,
                                    LineNo,
                                    StringNo,
                                    Opcode,
                                    Target,
                                    Source0,
                                    Source1);
    gcmFOOTER();
    return status;
}

gceSTATUS
slEmitAlwaysBranchCode(
    IN sloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN sleOPCODE Opcode,
    IN gctLABEL Label
    )
{
    return _EmitBranchCode(
                        Compiler,
                        LineNo,
                        StringNo,
                        _ConvOpcode(Opcode),
                        gcSL_ALWAYS,
                        Label,
                        gcvNULL,
                        gcvNULL);
}

gceSTATUS
slEmitTestBranchCode(
    IN sloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN sleOPCODE Opcode,
    IN gctLABEL Label,
    IN gctBOOL TrueBranch,
    IN gcsSOURCE * Source
    )
{
    gceSTATUS        status;
    gcsSOURCE        newSource;
    gcsSOURCE        falseSource;

    gcmHEADER();

    gcmASSERT(Source);

    status = _PrepareSource(
                            Compiler,
                            LineNo,
                            StringNo,
                            gcvNULL,
                            Source,
                            &newSource);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

#if TREAT_ES20_INTEGER_AS_FLOAT
    if (sloCOMPILER_IsHaltiVersion(Compiler)) {
#endif
       gcsSOURCE_InitializeBoolConstant(&falseSource, gcSHADER_PRECISION_MEDIUM, gcvFALSE);
#if TREAT_ES20_INTEGER_AS_FLOAT
    }
    else {
       gcsSOURCE_InitializeFloatConstant(&falseSource, gcSHADER_PRECISION_MEDIUM, slmB2F(gcvFALSE));
    }
#endif

    status = _EmitBranchCode(
                        Compiler,
                        LineNo,
                        StringNo,
                        _ConvOpcode(Opcode),
                        TrueBranch ? gcSL_NOT_EQUAL : gcSL_EQUAL,
                        Label,
                        &newSource,
                        &falseSource);

    gcmFOOTER();
    return status;
}

gcSL_CONDITION
_ConvCondition(
    IN sleCONDITION Condition
    )
{
    switch (Condition)
    {
    case slvCONDITION_EQUAL:               return gcSL_EQUAL;
    case slvCONDITION_NOT_EQUAL:           return gcSL_NOT_EQUAL;
    case slvCONDITION_LESS_THAN:           return gcSL_LESS;
    case slvCONDITION_LESS_THAN_EQUAL:     return gcSL_LESS_OR_EQUAL;
    case slvCONDITION_GREATER_THAN:        return gcSL_GREATER;
    case slvCONDITION_GREATER_THAN_EQUAL:  return gcSL_GREATER_OR_EQUAL;
    case slvCONDITION_XOR:                 return gcSL_XOR;
    case slvCONDITION_AND:                 return gcSL_AND;
    case slvCONDITION_OR:                  return gcSL_OR;
    case slvCONDITION_ZERO:                return gcSL_ZERO;
    case slvCONDITION_NOT_ZERO:            return gcSL_NOT_ZERO;

    default:
    gcmASSERT(0);
    return gcSL_EQUAL;
    }
}

gceSTATUS
slEmitCompareBranchCode(
    IN sloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN sleOPCODE Opcode,
    IN sleCONDITION Condition,
    IN gctLABEL Label,
    IN gcsSOURCE * Source0,
    IN gcsSOURCE * Source1
    )
{
    gceSTATUS status;
    gcsSOURCE source0Buf[1];
    gcsSOURCE *newSource0;
    gcsSOURCE newSource1[1];
    sloCODE_GENERATOR codeGenerator = Compiler->codeGenerator;

    gcmHEADER();

    gcmASSERT(Source0);
    gcmASSERT(Source1);

    if (codeGenerator->createDefaultUBO
        && Source0->type == gcvSOURCE_UNIFORM
        && !gcIsSamplerDataType(Source0->u.sourceReg.u.uniform->u.type)
        && !gcIsImageDataType(Source0->u.sourceReg.u.uniform->u.type)
        && !isUniformBlockAddress(Source0->u.sourceReg.u.uniform))
    {
        status = _MakeNewSourceForUniformInUBO(Compiler,
                                               LineNo,
                                               StringNo,
                                               Source0,
                                               source0Buf);
        if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }
        newSource0 = source0Buf;
    }
    else newSource0 = Source0;

    status = _PrepareAnotherSource(Compiler,
                                   LineNo,
                                   StringNo,
                                   gcvNULL,
                                   newSource0,
                                   Source1,
                                   newSource1);
    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    status = _EmitBranchCode(Compiler,
                             LineNo,
                             StringNo,
                             _ConvOpcode(Opcode),
                             _ConvCondition(Condition),
                             Label,
                             newSource0,
                             newSource1);

    gcmFOOTER();
    return status;
}

gceSTATUS
slBeginMainFunction(
    IN sloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo
    )
{
    gceSTATUS            status;
    sloCODE_EMITTER        codeEmitter;

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);

    codeEmitter = Compiler->codeEmitter;
    gcmASSERT(codeEmitter);

    status = sloCODE_EMITTER_NewBasicBlock(Compiler, codeEmitter);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
slEndMainFunction(
    IN sloCOMPILER Compiler
    )
{
    gceSTATUS            status;
    sloCODE_EMITTER        codeEmitter;

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);

    codeEmitter = Compiler->codeEmitter;
    gcmASSERT(codeEmitter);

    status = sloCODE_EMITTER_EndBasicBlock(Compiler, codeEmitter);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
slNewFunction(
    IN sloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gctCONST_STRING Name,
    OUT gcFUNCTION * Function
    )
{
    gceSTATUS            status;

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    gcmASSERT(Name);
    gcmASSERT(Function);

    status = _AddFunction(
                        Compiler,
                        Name,
                        Function);

    if (gcmIS_ERROR(status))
    {
        gcmVERIFY_OK(sloCOMPILER_Report(
                                        Compiler,
                                        LineNo,
                                        StringNo,
                                        slvREPORT_INTERNAL_ERROR,
                                        "failed to add the function: '%s'",
                                        Name));

        gcmFOOTER();
        return status;
    }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
slNewFunctionArgument(
    IN sloCOMPILER Compiler,
    IN gcFUNCTION Function,
    IN gctUINT16 VariableIndex,
    IN gcSHADER_TYPE DataType,
    IN gctUINT Length,
    IN gctREG_INDEX TempRegIndex,
    IN gctUINT8 Qualifier,
    IN gctUINT8 Precision,
    IN gctBOOL IsPrecise
    )
{
    gceSTATUS   status;
    gctUINT     i, j, regCount;

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    gcmASSERT(Function);

    switch (DataType)
    {
    case gcSHADER_FLOAT_2X2:
    case gcSHADER_FLOAT_2X3:
    case gcSHADER_FLOAT_2X4:
        {
            regCount = 2;
            break;
        }
    case gcSHADER_FLOAT_3X2:
    case gcSHADER_FLOAT_3X3:
    case gcSHADER_FLOAT_3X4:
        {
            regCount = 3;
            break;
        }
    case gcSHADER_FLOAT_4X2:
    case gcSHADER_FLOAT_4X3:
    case gcSHADER_FLOAT_4X4:
        {
            regCount = 4;
            break;
        }
    default:
        {
            regCount = 1;
            break;
        }
    }

    for (i = 0; i < Length; i++)
    {
        for (j = 0; j < regCount; j++)
        {
            status = _AddFunctionArgument(Compiler,
                                          Function,
                                          VariableIndex,
                                          TempRegIndex + (i * regCount + j),
                                          gcGetDefaultEnable(DataType),
                                          Qualifier,
                                          Precision,
                                          IsPrecise);

            if (gcmIS_ERROR(status))
            {
                gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                                0,
                                                0,
                                                slvREPORT_INTERNAL_ERROR,
                                                "failed to add the function argument"));

                gcmFOOTER();
                return status;
            }
        }
    }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
slBeginFunction(
    IN sloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gcFUNCTION Function
    )
{
    gceSTATUS            status;
    sloCODE_EMITTER        codeEmitter;

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    gcmASSERT(Function);

    codeEmitter = Compiler->codeEmitter;
    gcmASSERT(codeEmitter);

    status = sloCODE_EMITTER_NewBasicBlock(Compiler, codeEmitter);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    status = _BeginFunction(Compiler, Function);

    if (gcmIS_ERROR(status))
    {
        gcmVERIFY_OK(sloCOMPILER_Report(
                                        Compiler,
                                        LineNo,
                                        StringNo,
                                        slvREPORT_INTERNAL_ERROR,
                                        "failed to begin function"));

        gcmFOOTER();
        return status;
    }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
slEndFunction(
    IN sloCOMPILER Compiler,
    IN gcFUNCTION Function
    )
{
    gceSTATUS            status;
    sloCODE_EMITTER        codeEmitter;

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    gcmASSERT(Function);

    codeEmitter = Compiler->codeEmitter;
    gcmASSERT(codeEmitter);

    status = sloCODE_EMITTER_EndBasicBlock(Compiler, codeEmitter);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    status = _EndFunction(Compiler, Function);

    if (gcmIS_ERROR(status))
    {
        gcmVERIFY_OK(sloCOMPILER_Report(
                                        Compiler,
                                        0,
                                        0,
                                        slvREPORT_INTERNAL_ERROR,
                                        "failed to end function"));

        gcmFOOTER();
        return status;
    }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
slGetFunctionLabel(
    IN sloCOMPILER Compiler,
    IN gcFUNCTION Function,
    OUT gctLABEL * Label
    )
{
    gceSTATUS    status;

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    gcmASSERT(Function);

    status = _GetFunctionLabel(Compiler, Function, Label);

    if (gcmIS_ERROR(status))
    {
        gcmVERIFY_OK(sloCOMPILER_Report(
                                        Compiler,
                                        0,
                                        0,
                                        slvREPORT_INTERNAL_ERROR,
                                        "failed to get function label"));

        gcmFOOTER();
        return status;
    }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

/* sloCODE_EMITTER */
typedef enum _sleCODE_TYPE
{
    slvCODE_INVALID,
    slvCODE_ONE_OPERAND,
    slvCODE_TWO_OPERANDS
}
sleCODE_TYPE;

typedef struct _slsCODE_INFO
{
    sleCODE_TYPE    type;

    gctUINT            lineNo;

    gctUINT            stringNo;

    sleOPCODE        opcode;

    gcsTARGET        target;

    gcsSOURCE        source0;

    gcsSOURCE        source1;
}
slsCODE_INFO;

struct _sloCODE_EMITTER
{
    slsOBJECT        object;

    slsCODE_INFO    currentCodeInfo;
};

gceSTATUS
sloCODE_EMITTER_Construct(
    IN sloCOMPILER Compiler,
    OUT sloCODE_EMITTER * CodeEmitter
    )
{
    gceSTATUS            status;
    sloCODE_EMITTER        codeEmitter;

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    gcmASSERT(CodeEmitter);

    do
    {
        gctPOINTER pointer = gcvNULL;
        status = sloCOMPILER_Allocate(
                                    Compiler,
                                    (gctSIZE_T)sizeof(struct _sloCODE_EMITTER),
                                    &pointer);

        if (gcmIS_ERROR(status)) break;

        /* Initialize the members */
        codeEmitter                         = pointer;
        codeEmitter->object.type            = slvOBJ_CODE_EMITTER;

        codeEmitter->currentCodeInfo.type    = slvCODE_INVALID;

        *CodeEmitter = codeEmitter;

        gcmFOOTER_NO();
        return gcvSTATUS_OK;
    }
    while (gcvFALSE);

    *CodeEmitter = gcvNULL;

    gcmFOOTER();
    return status;
}

gceSTATUS
sloCODE_EMITTER_Destroy(
    IN sloCOMPILER Compiler,
    IN sloCODE_EMITTER CodeEmitter
    )
{
    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    slmVERIFY_OBJECT(CodeEmitter, slvOBJ_CODE_EMITTER);;

    gcmVERIFY_OK(sloCOMPILER_Free(Compiler, CodeEmitter));

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
sloCODE_EMITTER_EmitCurrentCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_EMITTER CodeEmitter
    )
{
    gceSTATUS            status;

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    slmVERIFY_OBJECT(CodeEmitter, slvOBJ_CODE_EMITTER);

    switch (CodeEmitter->currentCodeInfo.type)
    {
    case slvCODE_INVALID:
        break;

    case slvCODE_ONE_OPERAND:
        CodeEmitter->currentCodeInfo.type = slvCODE_INVALID;

        status = _EmitCodeImpl1(
                                Compiler,
                                CodeEmitter->currentCodeInfo.lineNo,
                                CodeEmitter->currentCodeInfo.stringNo,
                                CodeEmitter->currentCodeInfo.opcode,
                                &CodeEmitter->currentCodeInfo.target,
                                &CodeEmitter->currentCodeInfo.source0);

        if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

        break;

    case slvCODE_TWO_OPERANDS:
        CodeEmitter->currentCodeInfo.type = slvCODE_INVALID;

        status = _EmitCodeImpl2(
                                Compiler,
                                CodeEmitter->currentCodeInfo.lineNo,
                                CodeEmitter->currentCodeInfo.stringNo,
                                CodeEmitter->currentCodeInfo.opcode,
                                &CodeEmitter->currentCodeInfo.target,
                                &CodeEmitter->currentCodeInfo.source0,
                                &CodeEmitter->currentCodeInfo.source1);

        if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

        break;

    default:
        gcmASSERT(0);
    }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
sloCODE_EMITTER_NewBasicBlock(
    IN sloCOMPILER Compiler,
    IN sloCODE_EMITTER CodeEmitter
    )
{
    gceSTATUS            status;

    gcmHEADER();

    /* End the previous basic block */
    status = sloCODE_EMITTER_EndBasicBlock(Compiler, CodeEmitter);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
sloCODE_EMITTER_EndBasicBlock(
    IN sloCOMPILER Compiler,
    IN sloCODE_EMITTER CodeEmitter
    )
{
    gceSTATUS            status;

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    slmVERIFY_OBJECT(CodeEmitter, slvOBJ_CODE_EMITTER);

    status = sloCODE_EMITTER_EmitCurrentCode(Compiler, CodeEmitter);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
slEmitOpCodeResType(
    IN sloCOMPILER Compiler,
    IN sleOPCODE_RES_TYPE OpCodeResType
    )
{
    gceSTATUS               status;
    gcSHADER                binary;
    gcSL_OPCODE_RES_TYPE    resOpType = gcSL_OPCODE_RES_TYPE_NONE;

    gcmHEADER();

    gcmVERIFY_OK(sloCOMPILER_GetBinary(Compiler, &binary));

    switch (OpCodeResType)
    {
    case slvOPCODE_RES_TYPE_FETCH:
        resOpType = gcSL_OPCODE_RES_TYPE_FETCH;
        break;

    case slvOPCODE_RES_TYPE_FETCH_MS:
        resOpType = gcSL_OPCODE_RES_TYPE_FETCH_MS;
        break;

    default:
        gcmASSERT(gcvFALSE);
        break;
    }

    status = gcSHADER_UpdateResOpType(binary,
                                      resOpType);
    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER_NO();
    return status;
}

gceSTATUS
slEmitCurrentCode(
    IN sloCOMPILER Compiler
    )
{
    gceSTATUS               status;
    sloCODE_EMITTER         codeEmitter;

    gcmHEADER();

    codeEmitter = Compiler->codeEmitter;
    gcmASSERT(codeEmitter);

    status = sloCODE_EMITTER_EmitCurrentCode(Compiler, codeEmitter);
    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

#define slmMERGE_DATA_TYPE(dataType0, dataType1) \
    gcConvScalarToVectorDataType( \
        gcGetComponentDataType(dataType0), \
        gcGetDataTypeComponentCount(dataType0) + gcGetDataTypeComponentCount(dataType1))

static void
_MergeEnable(
    IN OUT gctUINT8 * ResultEnable,
    IN gctUINT8 Enable
    )
{
    gcmASSERT(ResultEnable);

    *ResultEnable |= Enable;
}

static void
_MergeEnableAndSwizzle(
    IN OUT gctUINT8 * ResultEnable,
    IN gctUINT8 Enable,
    IN OUT gctUINT8 * ResultSwizzle,
    IN gctUINT8 Swizzle
    )
{
    gcmASSERT(ResultEnable);
    gcmASSERT(ResultSwizzle);

    if (Enable & gcSL_ENABLE_X)
    {
        *ResultSwizzle = (*ResultSwizzle & ~0x03) | (Swizzle & 0x03);
    }

    if (Enable & gcSL_ENABLE_Y)
    {
        *ResultSwizzle = (*ResultSwizzle & ~0x0C) | (Swizzle & 0x0C);
    }

    if (Enable & gcSL_ENABLE_Z)
    {
        *ResultSwizzle = (*ResultSwizzle & ~0x30) | (Swizzle & 0x30);
    }

    if (Enable & gcSL_ENABLE_W)
    {
        *ResultSwizzle = (*ResultSwizzle & ~0xC0) | (Swizzle & 0xC0);
    }

    *ResultEnable |= Enable;
}

static gctBOOL
_CanTargetsBeMerged(
    IN gcsTARGET * Target0,
    IN gcsTARGET * Target1
    )
{
    gcmASSERT(Target0);
    gcmASSERT(Target1);

    do
    {
        if (gcGetComponentDataType(Target0->dataType)
            != gcGetComponentDataType(Target1->dataType)) break;

        if (Target0->tempRegIndex != Target1->tempRegIndex) break;

        if (Target0->indexMode != Target1->indexMode) break;

        if (Target0->indexMode != gcSL_NOT_INDEXED
            && Target0->indexRegIndex != Target1->indexRegIndex) break;

        if ((Target0->enable & Target1->enable) != 0) break;

        return gcvTRUE;
    }
    while (gcvFALSE);

    return gcvFALSE;
}

static gctBOOL
_CanSourcesBeMerged(
    IN gcsTARGET * Target0,
    IN gcsSOURCE * Source0,
    IN gcsTARGET * Target1,
    IN gcsSOURCE * Source1
    )
{
    gcmASSERT(Target0);
    gcmASSERT(Source0);
    gcmASSERT(Target1);
    gcmASSERT(Source1);

    do
    {
        if (Source0->type != Source1->type
            || gcGetComponentDataType(Source0->dataType)
                != gcGetComponentDataType(Source1->dataType)) break;

        if (Source0->type == gcvSOURCE_CONSTANT)
        {
            if (Source0->u.sourceConstant.u.intConstant
                != Source1->u.sourceConstant.u.intConstant) break;
        }
        else
        {
            if (Source1->type == gcvSOURCE_TEMP
                && Source1->u.sourceReg.regIndex == Target0->tempRegIndex) break;

            if (Source0->type == gcvSOURCE_ATTRIBUTE
                && Source0->u.sourceReg.u.attribute
                    != Source1->u.sourceReg.u.attribute) break;

            if (Source0->type == gcvSOURCE_UNIFORM
                && Source0->u.sourceReg.u.uniform
                    != Source1->u.sourceReg.u.uniform) break;

            if (Source0->u.sourceReg.regIndex
                != Source1->u.sourceReg.regIndex) break;

            if (Source0->u.sourceReg.indexMode
                != Source1->u.sourceReg.indexMode) break;

            if (Source0->u.sourceReg.indexMode != gcSL_NOT_INDEXED
                && Source0->u.sourceReg.indexRegIndex
                    != Source1->u.sourceReg.indexRegIndex) break;
        }

        return gcvTRUE;
    }
    while (gcvFALSE);

    return gcvFALSE;
}

gceSTATUS
sloCODE_EMITTER_TryToMergeCode1(
    IN sloCOMPILER Compiler,
    IN sloCODE_EMITTER CodeEmitter,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN sleOPCODE Opcode,
    IN gcsTARGET * Target,
    IN gcsSOURCE * Source,
    OUT gctBOOL * Merged
    )
{

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    slmVERIFY_OBJECT(CodeEmitter, slvOBJ_CODE_EMITTER);
    gcmASSERT(Merged);

    do
    {
        /* Check the type and opcode */
        if (CodeEmitter->currentCodeInfo.type != slvCODE_ONE_OPERAND
            || CodeEmitter->currentCodeInfo.opcode != Opcode) break;

        if (gcIsImageDataType(CodeEmitter->currentCodeInfo.target.dataType) ||
            gcIsImageDataType(Target->dataType) ||
            gcIsImageDataType(Source->dataType))
        {
            break;
        }

        /* Check the target */
        if (!_CanTargetsBeMerged(&CodeEmitter->currentCodeInfo.target, Target)) break;

        /* Check the source */
        if (!_CanSourcesBeMerged(
                                &CodeEmitter->currentCodeInfo.target,
                                &CodeEmitter->currentCodeInfo.source0,
                                Target,
                                Source)) break;

        /* Merge the code */
        CodeEmitter->currentCodeInfo.target.dataType =
                slmMERGE_DATA_TYPE(CodeEmitter->currentCodeInfo.target.dataType, Target->dataType);

        CodeEmitter->currentCodeInfo.source0.dataType =
                slmMERGE_DATA_TYPE(CodeEmitter->currentCodeInfo.source0.dataType, Source->dataType);

        if (CodeEmitter->currentCodeInfo.source0.type == gcvSOURCE_CONSTANT)
        {
            _MergeEnable(&CodeEmitter->currentCodeInfo.target.enable, Target->enable);
        }
        else
        {
            _MergeEnableAndSwizzle(
                                &CodeEmitter->currentCodeInfo.target.enable,
                                Target->enable,
                                &CodeEmitter->currentCodeInfo.source0.u.sourceReg.swizzle,
                                Source->u.sourceReg.swizzle);
        }

        *Merged = gcvTRUE;

        gcmFOOTER_NO();
        return gcvSTATUS_OK;
    }
    while (gcvFALSE);

    *Merged = gcvFALSE;

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
sloCODE_EMITTER_EmitCode1(
    IN sloCOMPILER Compiler,
    IN sloCODE_EMITTER CodeEmitter,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN sleOPCODE Opcode,
    IN gcsTARGET * Target,
    IN gcsSOURCE * Source
    )
{
    gceSTATUS    status;
    gctBOOL        merged;

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    slmVERIFY_OBJECT(CodeEmitter, slvOBJ_CODE_EMITTER);

    if (!(Compiler->context.optimizationOptions & slvOPTIMIZATION_DATA_FLOW))
    {
        status = _EmitCodeImpl1(
                            Compiler,
                            LineNo,
                            StringNo,
                            Opcode,
                            Target,
                            Source);

        gcmFOOTER();
        return status;
    }

    status = sloCODE_EMITTER_TryToMergeCode1(
                                            Compiler,
                                            CodeEmitter,
                                            LineNo,
                                            StringNo,
                                            Opcode,
                                            Target,
                                            Source,
                                            &merged);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    if (!merged)
    {
        status = sloCODE_EMITTER_EmitCurrentCode(Compiler, CodeEmitter);

        if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

        CodeEmitter->currentCodeInfo.type        = slvCODE_ONE_OPERAND;
        CodeEmitter->currentCodeInfo.lineNo        = LineNo;
        CodeEmitter->currentCodeInfo.stringNo    = StringNo;
        CodeEmitter->currentCodeInfo.opcode        = Opcode;
        CodeEmitter->currentCodeInfo.target        = *Target;
        CodeEmitter->currentCodeInfo.source0    = *Source;
    }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

static void
_MergeEnableAndTwoSwizzles(
    IN OUT gctUINT8 * ResultEnable,
    IN gctUINT8 Enable,
    IN OUT gctUINT8 * ResultSwizzle0,
    IN gctUINT8 Swizzle0,
    IN OUT gctUINT8 * ResultSwizzle1,
    IN gctUINT8 Swizzle1
    )
{
    gcmASSERT(ResultEnable);
    gcmASSERT(ResultSwizzle0);
    gcmASSERT(ResultSwizzle1);

    if (Enable & gcSL_ENABLE_X)
    {
        *ResultSwizzle0 = (*ResultSwizzle0 & ~0x03) | (Swizzle0 & 0x03);
        *ResultSwizzle1 = (*ResultSwizzle1 & ~0x03) | (Swizzle1 & 0x03);
    }

    if (Enable & gcSL_ENABLE_Y)
    {
        *ResultSwizzle0 = (*ResultSwizzle0 & ~0x0C) | (Swizzle0 & 0x0C);
        *ResultSwizzle1 = (*ResultSwizzle1 & ~0x0C) | (Swizzle1 & 0x0C);
    }

    if (Enable & gcSL_ENABLE_Z)
    {
        *ResultSwizzle0 = (*ResultSwizzle0 & ~0x30) | (Swizzle0 & 0x30);
        *ResultSwizzle1 = (*ResultSwizzle1 & ~0x30) | (Swizzle1 & 0x30);
    }

    if (Enable & gcSL_ENABLE_W)
    {
        *ResultSwizzle0 = (*ResultSwizzle0 & ~0xC0) | (Swizzle0 & 0xC0);
        *ResultSwizzle1 = (*ResultSwizzle1 & ~0xC0) | (Swizzle1 & 0xC0);
    }

    *ResultEnable |= Enable;
}

gceSTATUS
sloCODE_EMITTER_TryToMergeCode2(
    IN sloCOMPILER Compiler,
    IN sloCODE_EMITTER CodeEmitter,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN sleOPCODE Opcode,
    IN gcsTARGET * Target,
    IN gcsSOURCE * Source0,
    IN gcsSOURCE * Source1,
    OUT gctBOOL * Merged
    )
{
    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    slmVERIFY_OBJECT(CodeEmitter, slvOBJ_CODE_EMITTER);
    gcmASSERT(Merged);

    do
    {
        /* Check the type and opcode */
        if (CodeEmitter->currentCodeInfo.type != slvCODE_TWO_OPERANDS
            || CodeEmitter->currentCodeInfo.opcode != Opcode) break;

        /* Check the target */
        if (!_CanTargetsBeMerged(&CodeEmitter->currentCodeInfo.target, Target)) break;

        /* Check the sources */
        if (!_CanSourcesBeMerged(
                                &CodeEmitter->currentCodeInfo.target,
                                &CodeEmitter->currentCodeInfo.source0,
                                Target,
                                Source0)) break;

        if (!_CanSourcesBeMerged(
                                &CodeEmitter->currentCodeInfo.target,
                                &CodeEmitter->currentCodeInfo.source1,
                                Target,
                                Source1)) break;

        /* Merge the code */
        CodeEmitter->currentCodeInfo.target.dataType =
                slmMERGE_DATA_TYPE(CodeEmitter->currentCodeInfo.target.dataType, Target->dataType);

        CodeEmitter->currentCodeInfo.source0.dataType =
                slmMERGE_DATA_TYPE(CodeEmitter->currentCodeInfo.source0.dataType, Source0->dataType);

        CodeEmitter->currentCodeInfo.source1.dataType =
                slmMERGE_DATA_TYPE(CodeEmitter->currentCodeInfo.source1.dataType, Source1->dataType);

        if (CodeEmitter->currentCodeInfo.source0.type != gcvSOURCE_CONSTANT
            && CodeEmitter->currentCodeInfo.source1.type != gcvSOURCE_CONSTANT)
        {
            _MergeEnableAndTwoSwizzles(
                                &CodeEmitter->currentCodeInfo.target.enable,
                                Target->enable,
                                &CodeEmitter->currentCodeInfo.source0.u.sourceReg.swizzle,
                                Source0->u.sourceReg.swizzle,
                                &CodeEmitter->currentCodeInfo.source1.u.sourceReg.swizzle,
                                Source1->u.sourceReg.swizzle);
        }
        else if (CodeEmitter->currentCodeInfo.source0.type != gcvSOURCE_CONSTANT)
        {
            _MergeEnableAndSwizzle(
                                &CodeEmitter->currentCodeInfo.target.enable,
                                Target->enable,
                                &CodeEmitter->currentCodeInfo.source0.u.sourceReg.swizzle,
                                Source0->u.sourceReg.swizzle);
        }
        else if (CodeEmitter->currentCodeInfo.source1.type != gcvSOURCE_CONSTANT)
        {
            _MergeEnableAndSwizzle(
                                &CodeEmitter->currentCodeInfo.target.enable,
                                Target->enable,
                                &CodeEmitter->currentCodeInfo.source1.u.sourceReg.swizzle,
                                Source1->u.sourceReg.swizzle);
        }
        else
        {
            _MergeEnable(&CodeEmitter->currentCodeInfo.target.enable, Target->enable);
        }

        *Merged = gcvTRUE;

        gcmFOOTER_NO();
        return gcvSTATUS_OK;
    }
    while (gcvFALSE);

    *Merged = gcvFALSE;

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
sloCODE_EMITTER_EmitCode2(
    IN sloCOMPILER Compiler,
    IN sloCODE_EMITTER CodeEmitter,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN sleOPCODE Opcode,
    IN gcsTARGET * Target,
    IN gcsSOURCE * Source0,
    IN gcsSOURCE * Source1
    )
{
    gceSTATUS    status;
    gctBOOL        merged;

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    slmVERIFY_OBJECT(CodeEmitter, slvOBJ_CODE_EMITTER);

    if (!(Compiler->context.optimizationOptions & slvOPTIMIZATION_DATA_FLOW))
    {
        status = _EmitCodeImpl2(
                            Compiler,
                            LineNo,
                            StringNo,
                            Opcode,
                            Target,
                            Source0,
                            Source1);

        gcmFOOTER();
        return status;
    }

    status = sloCODE_EMITTER_TryToMergeCode2(
                                            Compiler,
                                            CodeEmitter,
                                            LineNo,
                                            StringNo,
                                            Opcode,
                                            Target,
                                            Source0,
                                            Source1,
                                            &merged);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    if (!merged)
    {
        status = sloCODE_EMITTER_EmitCurrentCode(Compiler, CodeEmitter);

        if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

        CodeEmitter->currentCodeInfo.type        = slvCODE_TWO_OPERANDS;
        CodeEmitter->currentCodeInfo.lineNo        = LineNo;
        CodeEmitter->currentCodeInfo.stringNo    = StringNo;
        CodeEmitter->currentCodeInfo.opcode        = Opcode;
        CodeEmitter->currentCodeInfo.target        = *Target;
        CodeEmitter->currentCodeInfo.source0    = *Source0;
        CodeEmitter->currentCodeInfo.source1    = *Source1;
    }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}
