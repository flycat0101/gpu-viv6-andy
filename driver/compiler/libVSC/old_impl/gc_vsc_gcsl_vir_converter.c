/****************************************************************************
*
*    Copyright (c) 2005 - 2019 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


#include "gc_vsc.h"
#include "vir/lower/gc_vsc_vir_lower_common_func.h"

#if gcdENABLE_3D

/* Zone used for header/footer. */
#define _GC_OBJ_ZONE    gcdZONE_COMPILER

#include "vir/lower/gc_vsc_vir_ml_2_ll.h"
#include "vir/lower/gc_vsc_vir_hl_2_ml.h"
#include "vir/lower/gc_vsc_vir_lower_common_func.h"

#define OPCODE_TEXBIAS      (VIR_OP_TEXLD)
#define OPCODE_TEXBIAS_U    (VIR_OP_TEXLD)
#define EXTERN_TEXBIAS      (1)

#define OPCODE_TEXGRAD (VIR_OP_TEXLD)
#define EXTERN_TEXGRAD (2)

#define OPCODE_TEXLOD       (VIR_OP_TEXLD)
#define OPCODE_TEXLOD_U     (VIR_OP_TEXLD_U)
#define EXTERN_TEXLOD       (3)

#define OPCODE_TEXGATHER (VIR_OP_TEXLD)
#define EXTERN_TEXGATHER (4)

#define OPCODE_TEXFETCH_MS (VIR_OP_TEXLD)
#define EXTERN_TEXFETCH_MS (5)

#define OPCODE_TEXPCF (VIR_OP_TEXLD)
#define EXTERN_TEXPCF (6)

#define OPCODE_TEXU    (VIR_OP_TEXLD)
#define EXTERN_TEXU    (7)

#define OPCODE_TEXU_LOD (VIR_OP_TEXLD)
#define EXTERN_TEXU_LOD (8)

#define OPCODE_BYTEREV (VIR_OP_BYTEREV)
#define EXTERN_BYTEREV (1)

#define OPCODE_STORE   (VIR_OP_STARR)
#define EXTERN_STORE   (1)

#define OPCODE_STORE1  (VIR_OP_STARR)
#define EXTERN_STORE1  (2)

#define OPCODE_IMG_STORE  (VIR_OP_STARR)
#define EXTERN_IMG_STORE  (3)

#define OPCODE_IMG_STORE_3D  (VIR_OP_STARR)
#define EXTERN_IMG_STORE_3D  (4)

#define OPCODE_STORE_L (VIR_OP_STARR)
#define EXTERN_STORE_L (5)

#define OPCODE_I2F     (VIR_OP_CONVERT)
#define EXTERN_I2F     (1)

#define OPCODE_F2I     (VIR_OP_CONVERT)
#define EXTERN_F2I     (2)

#define OPCODE_FMA_MUL (VIR_OP_MUL)
#define EXTERN_FMA_MUL (1)

#define OPCODE_ARCTRIG0 (VIR_OP_ARCTRIG)
#define EXTERN_ARCTRIG0 (1)

#define OPCODE_ARCTRIG1 (VIR_OP_ARCTRIG)
#define EXTERN_ARCTRIG1 (2)

#define UNUSED_FUNCTION  ((VIR_Function *)-1)

typedef VIR_OpCode
(* CONV_VIR_OPCODE_FUNC_PTR)(
    IN gcSHADER         Shader,
    IN gctUINT32        CodeIndex
    );

VIR_OpCode _ConvTEXLOD(
    IN gcSHADER         Shader,
    IN gctUINT32        CodeIndex
    )
{
    VIR_OpCode          virOpcode = OPCODE_TEXLOD;
    gcSL_INSTRUCTION    nextCode;

    if (CodeIndex < Shader->codeCount - 1)
    {
        nextCode = &Shader->code[CodeIndex + 1];

        if (gcmSL_OPCODE_GET(nextCode->opcode, Opcode) == gcSL_TEXLD_U)
        {
            virOpcode = OPCODE_TEXLOD_U;
        }
    }

    return virOpcode;
}

VIR_OpCode _ConvTEXBIAS(
    IN gcSHADER         Shader,
    IN gctUINT32        CodeIndex
    )
{
    VIR_OpCode          virOpcode = OPCODE_TEXBIAS;
    gcSL_INSTRUCTION    nextCode;

    if (CodeIndex < Shader->codeCount - 1)
    {
        nextCode = &Shader->code[CodeIndex + 1];

        if (gcmSL_OPCODE_GET(nextCode->opcode, Opcode) == gcSL_TEXLD_U)
        {
            virOpcode = OPCODE_TEXBIAS_U;
        }
    }

    return virOpcode;
}

typedef struct _conv2VirsVirOpcodeMap
{
    VIR_OpCode                  virOpcode;
    VIR_Modifier                modifier;
    gctUINT                     externOpcode;
    gctINT                      srcComponents;
    CONV_VIR_OPCODE_FUNC_PTR    convVirOpCode;

}
conv2VirsVirOpcodeMap;

conv2VirsVirOpcodeMap _virOpcodeMap[] =
{
    {VIR_OP_NOP, /* gcSL_NOP, 0x00 */ VIR_MOD_NONE, 0, 0},
    {VIR_OP_MOV, /* gcSL_MOV, 0x01 */ VIR_MOD_NONE, 0, 0},
    {VIR_OP_SAT, /* gcSL_SAT, 0x02 */ VIR_MOD_NONE, 0, 0},
    {VIR_OP_DP3, /* gcSL_DP3, 0x03 */ VIR_MOD_NONE, 0, 3},
    {VIR_OP_DP4, /* gcSL_DP4, 0x04 */ VIR_MOD_NONE, 0, 4},
    {VIR_OP_ABS, /* gcSL_ABS, 0x05 */ VIR_MOD_NONE, 0, 0},
    {VIR_OP_JMP, /* gcSL_JMP, 0x06 */ VIR_MOD_NONE, 0, 0},
    {VIR_OP_ADD, /* gcSL_ADD, 0x07 */ VIR_MOD_NONE, 0, 0},
    {VIR_OP_MUL, /* gcSL_MUL, 0x08 */ VIR_MOD_NONE, 0, 0},
    {VIR_OP_RCP, /* gcSL_RCP, 0x09 */ VIR_MOD_NONE, 0, 0},
    {VIR_OP_SUB, /* gcSL_SUB, 0x0A */ VIR_MOD_NONE, 0, 0},
    {VIR_OP_KILL, /* gcSL_KILL, 0x0B */ VIR_MOD_NONE, 0, 0},
    {VIR_OP_TEXLD, /* gcSL_TEXLD, 0x0C */ VIR_MOD_NONE, 0, -1},
    {VIR_OP_CALL, /* gcSL_CALL, 0x0D */ VIR_MOD_NONE, 0, 0},
    {VIR_OP_RET, /* gcSL_RET, 0x0E */ VIR_MOD_NONE, 0, 0},
    {VIR_OP_NORM, /* gcSL_NORM, 0x0F */ VIR_MOD_NONE, 0, 0},
    {VIR_OP_MAX, /* gcSL_MAX, 0x10 */ VIR_MOD_NONE, 0, 0},
    {VIR_OP_MIN, /* gcSL_MIN, 0x11 */ VIR_MOD_NONE, 0, 0},
    {VIR_OP_POW, /* gcSL_POW, 0x12 */ VIR_MOD_NONE, 0, 0},
    {VIR_OP_RSQ, /* gcSL_RSQ, 0x13 */ VIR_MOD_NONE, 0, 0},
    {VIR_OP_LOG2, /* gcSL_LOG, 0x14 */ VIR_MOD_NONE, 0, 0},
    {VIR_OP_FRAC, /* gcSL_FRAC, 0x15 */ VIR_MOD_NONE, 0, 0},
    {VIR_OP_FLOOR, /* gcSL_FLOOR, 0x16 */ VIR_MOD_NONE, 0, 0},
    {VIR_OP_CEIL, /* gcSL_CEIL, 0x17 */ VIR_MOD_NONE, 0, 0},
    {VIR_OP_CROSS, /* gcSL_CROSS, 0x18 */ VIR_MOD_NONE, 0, 0},
    {VIR_OP_TEXLDPROJ, /* gcSL_TEXLDPROJ, 0x19 */ VIR_MOD_NONE, 0, -1},
    {OPCODE_TEXBIAS, /* gcSL_TEXBIAS, 0x1A */ VIR_MOD_NONE, EXTERN_TEXBIAS, -1, _ConvTEXBIAS},
    {OPCODE_TEXGRAD, /* gcSL_TEXGRAD, 0x1B */ VIR_MOD_NONE, EXTERN_TEXGRAD, -1},
    {OPCODE_TEXLOD, /* gcSL_TEXLOD, 0x1C */ VIR_MOD_NONE, EXTERN_TEXLOD, -1, _ConvTEXLOD},
    {VIR_OP_SIN, /* gcSL_SIN, 0x1D */ VIR_MOD_NONE, 0, 0},
    {VIR_OP_COS, /* gcSL_COS, 0x1E */ VIR_MOD_NONE, 0, 0},
    {VIR_OP_TAN, /* gcSL_TAN, 0x1F */ VIR_MOD_NONE, 0, 0},
    {VIR_OP_EXP2, /* gcSL_EXP, 0x20 */ VIR_MOD_NONE, 0, 0},
    {VIR_OP_SIGN, /* gcSL_SIGN, 0x21 */ VIR_MOD_NONE, 0, 0},
    {VIR_OP_STEP, /* gcSL_STEP, 0x22 */ VIR_MOD_NONE, 0, 0},
    {VIR_OP_SQRT, /* gcSL_SQRT, 0x23 */ VIR_MOD_NONE, 0, 0},
    {VIR_OP_ACOS, /* gcSL_ACOS, 0x24 */ VIR_MOD_NONE, 0, 0},
    {VIR_OP_ASIN, /* gcSL_ASIN, 0x25 */ VIR_MOD_NONE, 0, 0},
    {VIR_OP_ATAN, /* gcSL_ATAN, 0x26 */ VIR_MOD_NONE, 0, 0},
    {VIR_OP_SET,/* gcSL_SET, 0x27 */ VIR_MOD_NONE, 0, 0},
    {VIR_OP_DSX, /* gcSL_DSX, 0x28 */ VIR_MOD_NONE, 0, 0},
    {VIR_OP_DSY, /* gcSL_DSY, 0x29 */ VIR_MOD_NONE, 0, 0},
    {VIR_OP_FWIDTH, /* gcSL_FWIDTH, 0x2A */ VIR_MOD_NONE, 0, 0},
    {VIR_OP_DIV, /* gcSL_DIV, 0x2B */ VIR_MOD_NONE, 0, 0},
    {VIR_OP_MOD, /* gcSL_MOD, 0x2C */ VIR_MOD_NONE, 0, 0},
    {VIR_OP_AND_BITWISE, /* gcSL_AND_BITWISE, 0x2D */ VIR_MOD_NONE, 0, 0},
    {VIR_OP_OR_BITWISE, /* gcSL_OR_BITWISE, 0x2E */ VIR_MOD_NONE, 0, 0},
    {VIR_OP_XOR_BITWISE, /* gcSL_XOR_BITWISE, 0x2F */ VIR_MOD_NONE, 0, 0},
    {VIR_OP_NOT_BITWISE, /* gcSL_NOT_BITWISE, 0x30 */ VIR_MOD_NONE, 0, 0},
    {VIR_OP_LSHIFT, /* gcSL_LSHIFT, 0x31 */ VIR_MOD_NONE, 0, 0},
    {VIR_OP_RSHIFT, /* gcSL_RSHIFT, 0x32 */ VIR_MOD_NONE, 0, 0},
    {VIR_OP_ROTATE, /* gcSL_ROTATE, 0x33 */ VIR_MOD_NONE, 0, 0},
    {VIR_OP_BITSEL, /* gcSL_BITSEL, 0x34 */ VIR_MOD_NONE, 0, 0},
    {VIR_OP_LEADZERO, /* gcSL_LEADZERO, 0x35 */ VIR_MOD_NONE, 0, 0},
    {VIR_OP_LOAD, /* gcSL_LOAD, 0x36 */ VIR_MOD_NONE, 0, 0},
    {OPCODE_STORE, /* gcSL_STORE, 0x37 */ VIR_MOD_NONE, EXTERN_STORE, -1},
    {VIR_OP_BARRIER, /* gcSL_BARRIER, 0x38 */ VIR_MOD_NONE, 0, 0},
    {OPCODE_STORE1, /* gcSL_STORE1, 0x39 */ VIR_MOD_NONE, EXTERN_STORE1, 1},
    {VIR_OP_ATOMADD, /* gcSL_ATOMADD, 0x3A */ VIR_MOD_NONE, 0, 0},
    {VIR_OP_ATOMSUB, /* gcSL_ATOMSUB, 0x3B */ VIR_MOD_NONE, 0, 0},
    {VIR_OP_ATOMXCHG, /* gcSL_ATOMXCHG, 0x3C */ VIR_MOD_NONE, 0, 0},
    {VIR_OP_ATOMCMPXCHG, /* gcSL_ATOMCMPXCHG, 0x3D */ VIR_MOD_NONE, 0, 0},
    {VIR_OP_ATOMMIN, /* gcSL_ATOMMIN, 0x3E */ VIR_MOD_NONE, 0, 0},
    {VIR_OP_ATOMMAX, /* gcSL_ATOMMAX, 0x3F */ VIR_MOD_NONE, 0, 0},
    {VIR_OP_ATOMOR, /* gcSL_ATOMOR, 0x40 */ VIR_MOD_NONE, 0, 0},
    {VIR_OP_ATOMAND, /* gcSL_ATOMAND, 0x41 */ VIR_MOD_NONE, 0, 0},
    {VIR_OP_ATOMXOR, /* gcSL_ATOMXOR, 0x42 */ VIR_MOD_NONE, 0, 0},
    {VIR_OP_TEXLDPCF, /* gcSL_TEXLDPCF, 0x43 */ VIR_MOD_NONE, 0, -1},
    {VIR_OP_TEXLDPCFPROJ, /* gcSL_TEXLDPCFPROJ, 0x44 */ VIR_MOD_NONE, 0, -1},
    {VIR_OP_NOP, /* gcSL_TEXLODQ, 0x45 */ VIR_MOD_NONE, 0, 0},
    {VIR_OP_NOP, /* gcSL_FLUSH, 0x46 */ VIR_MOD_NONE, 0, 0},
    {VIR_OP_JMP_ANY, /* gcSL_JMP_ANY, 0x47 */ VIR_MOD_NONE, 0, 0},
    {VIR_OP_BITRANGE, /* gcSL_BITRANGE, 0x48 */ VIR_MOD_NONE, 0, 0},
    {VIR_OP_BITRANGE1, /* gcSL_BITRANGE1, 0x49 */ VIR_MOD_NONE, 0, 0},
    {VIR_OP_BITEXTRACT, /* gcSL_BITEXTRACT, 0x4A */ VIR_MOD_NONE, 0, 0},
    {VIR_OP_BITINSERT, /* gcSL_BITINSERT, 0x4B */ VIR_MOD_NONE, 0, 0},
    {VIR_OP_BITFIND_LSB, /* gcSL_FINDLSB, 0x4C */ VIR_MOD_NONE, 0, 0},
    {VIR_OP_BITFIND_MSB, /* gcSL_FINDMSB, 0x4D */ VIR_MOD_NONE, 0, 0},
    {VIR_OP_NOP, /* gcSL_IMAGE_OFFSET, 0x4E */ VIR_MOD_NONE, 0, 0},
    {VIR_OP_IMG_ADDR, /* gcSL_IMAGE_ADDR, 0x4F */ VIR_MOD_NONE, 0, -1},
    {VIR_OP_SINPI, /* gcSL_SINPI, 0x50 */ VIR_MOD_NONE, 0, 0},
    {VIR_OP_COSPI, /* gcSL_COSPI, 0x51 */ VIR_MOD_NONE, 0, 0},
    {VIR_OP_TANPI, /* gcSL_TANPI, 0x52 */ VIR_MOD_NONE, 0, 0},
    {VIR_OP_ADDLO, /* gcSL_ADDLO, 0x53 */ VIR_MOD_NONE, 0, 0},
    {VIR_OP_MULLO, /* gcSL_MULLO, 0x54 */ VIR_MOD_NONE, 0, 0},
    {VIR_OP_CONV0, /* gcSL_CONV, 0x55 */ VIR_MOD_NONE, 0, 0},
    {VIR_OP_GETEXP, /* gcSL_GETEXP, 0x56 */ VIR_MOD_NONE, 0, 0},
    {VIR_OP_GETMANT, /* gcSL_GETMANT, 0x57 */ VIR_MOD_NONE, 0, 0},
    {VIR_OP_MULHI, /* gcSL_MULHI, 0x58 */ VIR_MOD_NONE, 0, 0},
    {VIR_OP_COMPARE, /* gcSL_CMP, 0x59 */ VIR_MOD_NONE, 0, 0},
    {OPCODE_I2F, /* gcSL_I2F, 0x5A */ VIR_MOD_NONE, EXTERN_I2F, 0},
    {OPCODE_F2I, /* gcSL_F2I, 0x5B */ VIR_MOD_NONE, EXTERN_F2I, 0},
    {VIR_OP_ADDSAT, /* gcSL_ADDSAT, 0x5C */ VIR_MOD_NONE, 0, 0},
    {VIR_OP_SUBSAT, /* gcSL_SUBSAT, 0x5D */ VIR_MOD_NONE, 0, 0},
    {VIR_OP_MULSAT, /* gcSL_MULSAT, 0x5E */ VIR_MOD_NONE, 0, 0},
    {VIR_OP_DP2, /* gcSL_DP2, 0x5F */ VIR_MOD_NONE, 0, 2},
    {VIR_OP_NOP, /* gcSL_UNPACK, 0x60 */ VIR_MOD_NONE, 0, 0},
    {OPCODE_IMG_STORE, /* gcSL_IMAGE_STORE, 0x61 */ VIR_MOD_NONE, EXTERN_IMG_STORE, 0},
    {VIR_OP_NOP, /* gcSL_SAMPLER_ADD, 0x62 */ VIR_MOD_NONE, 0, 0},
    {VIR_OP_MOVA, /* gcSL_MOVA, 0x63 */ VIR_MOD_NONE, 0, 0},
    {VIR_OP_IMG_LOAD, /* gcSL_IMAGE_RD, 0x64 */ VIR_MOD_NONE, 0, 0},
    {VIR_OP_IMG_SAMPLER, /* gcSL_IMAGE_SAMPLER, 0x65 */ VIR_MOD_NONE, 0, 0},
    {VIR_OP_NORM_MUL, /* gcSL_NORM_MUL, 0x66 */ VIR_MOD_NONE, 0, 0},
    {VIR_OP_NORM_DP2, /* gcSL_NORM_DP2, 0x67 */ VIR_MOD_NONE, 0, 0},
    {VIR_OP_NORM_DP3, /* gcSL_NORM_DP3, 0x68 */ VIR_MOD_NONE, 0, 0},
    {VIR_OP_NORM_DP4, /* gcSL_NORM_DP4, 0x69 */ VIR_MOD_NONE, 0, 0},
    {VIR_OP_PRE_DIV, /* gcSL_PRE_DIV, 0x6A */ VIR_MOD_NONE, 0, 0},
    {VIR_OP_PRE_LOG2, /* gcSL_PRE_LOG2, 0x6B */ VIR_MOD_NONE, 0, 0},
    {OPCODE_TEXGATHER, /* gcSL_TEXGATHER, 0x6C */ VIR_MOD_NONE, EXTERN_TEXGATHER, -1},
    {OPCODE_TEXFETCH_MS, /* gcSL_TEXFETCH_MS, 0x6D */ VIR_MOD_NONE, EXTERN_TEXFETCH_MS, -1},
    {VIR_OP_POPCOUNT, /* gcSL_POPCOUNT, 0x6E */ VIR_MOD_NONE, 0, 1},
    {VIR_OP_BITREV, /* gcSL_BIT_REVERSAL, 0x6F */ VIR_MOD_NONE, 0, 1},
    {OPCODE_BYTEREV, /* gcSL_BYTE_REVERSAL, 0x70 */ VIR_MOD_NONE, EXTERN_BYTEREV, -1},
    {OPCODE_TEXPCF, /* gcSL_TEXPCF, 0x71 */ VIR_MOD_NONE, EXTERN_TEXPCF, -1},
    {VIR_OP_UCARRY, /* gcSL_UCARRY, 0x72 */ VIR_MOD_NONE, 0, 2},
    {OPCODE_TEXU, /* gcSL_TEXU, 0x73 */ VIR_MOD_NONE, EXTERN_TEXU, -1},
    {OPCODE_TEXU_LOD, /* gcSL_TEXU_LOD, 0x74 */ VIR_MOD_NONE, EXTERN_TEXU_LOD, -1},
    {VIR_OP_MEM_BARRIER, /* gcS_BARRIER, 0x75 */ VIR_MOD_NONE, 0, 0},
    {VIR_OP_NOP, /* gcSL_SAMPLER_ASSIGN, 0x76 */ VIR_MOD_NONE, 0, 0},
    {VIR_OP_GET_SAMPLER_IDX,/* gcSL_GET_SAMPLER_IDX, 0x77 */ VIR_MOD_NONE, 0, 0},
    {VIR_OP_IMG_LOAD_3D, /* gcSL_IMAGE_RD_3D, 0x78 */ VIR_MOD_NONE, 0, 0},
    {OPCODE_IMG_STORE_3D, /* gcSL_IMAGE_WR_3D, 0x79 */ VIR_MOD_NONE, EXTERN_IMG_STORE_3D, 0},
    {VIR_OP_CLAMP0MAX, /* gcSL_CLAMP0MAX, 0x7A */ VIR_MOD_NONE, 0, 0},
    {OPCODE_FMA_MUL, /* gcSL_FMA_MUL, 0x7B */ VIR_MOD_NONE, EXTERN_FMA_MUL, 0},
    {VIR_OP_ADD, /* gcSL_FMA_ADD, 0x7C */ VIR_MOD_NONE, 0, 0},
    {VIR_OP_ATTR_ST, /* gcSL_ATTR_ST, 0x7D */ VIR_MOD_NONE, 0, 0},
    {VIR_OP_ATTR_LD, /* gcSL_ATTR_LD, 0x7E */ VIR_MOD_NONE, 0, 0},
    {VIR_OP_EMIT0, /* gcSL_EMIT_VERTEX, 0x7F */ VIR_MOD_NONE, 0, 0},
    {VIR_OP_RESTART0, /* gcSL_END_PRIMITIVE, 0x80 */ VIR_MOD_NONE, 0, 0},
    {OPCODE_ARCTRIG0, /* gcSL_ARCTRIG0, 0x81 */ VIR_MOD_NONE, EXTERN_ARCTRIG0, 0},
    {OPCODE_ARCTRIG1, /* gcSL_ARCTRIG1, 0x82 */ VIR_MOD_NONE, EXTERN_ARCTRIG1, 0},
    {VIR_OP_MUL_Z, /* gcSL_MUL_Z, 0x83 */ VIR_MOD_NONE, 0, 0},
    {VIR_OP_NEG, /* gcSL_NEG, 0x84 */ VIR_MOD_NONE, 0, 0},
    {VIR_OP_LONGLO, /* gcSL_LONGLO, 0x85 */ VIR_MOD_NONE, 0, 0},
    {VIR_OP_LONGHI, /* gcSL_LONGHI, 0x86 */ VIR_MOD_NONE, 0, 0},
    {VIR_OP_MOV_LONG, /* gcSL_MOV_LONG, 0x87 */ VIR_MOD_NONE, 0, 0},
    {VIR_OP_MADSAT, /* gcSL_MADSAT, 0x88 */ VIR_MOD_NONE, 0, 0},
    {VIR_OP_COPY, /* gcSL_COPY, 0x89 */ VIR_MOD_NONE, 0, 0},
    {VIR_OP_LOAD_L, /* gcSL_LOAD_L, 0x8A */ VIR_MOD_NONE, 0, 0},
    {OPCODE_STORE_L, /* gcSL_STORE1_L, 0x8B */ VIR_MOD_NONE, EXTERN_STORE_L, 1},
    {VIR_OP_IMG_ADDR_3D, /* gcSL_IMAGE_ADDR_3D, 0x8C */ VIR_MOD_NONE, 0, -1},
    {VIR_OP_GET_SAMPLER_LMM,/* gcSL_GET_SAMPLER_LMM, 0x8D */ VIR_MOD_NONE, 0, 0},
    {VIR_OP_GET_SAMPLER_LBS,/* gcSL_GET_SAMPLER_LBS, 0x8E */ VIR_MOD_NONE, 0, 0},
    {VIR_OP_TEXLD_U, /* gcSL_TEXLD_U, 0x8F */ VIR_MOD_NONE, 0, -1},
    {VIR_OP_PARAM_CHAIN, /* gcSL_PARAM_CHAIN, 0x90 */ VIR_MOD_NONE, 0, -1},
    {VIR_OP_INTRINSIC, /* gcSL_INTRINSIC, 0x91 */ VIR_MOD_NONE, 0, -1},
    {VIR_OP_INTRINSIC, /* gcSL_INTRINSIC_ST, 0x92 */ VIR_MOD_NONE, 0, -1},
    {VIR_OP_CMAD, /* gcSL_CMAD, 0x93 */ VIR_MOD_NONE, 0, -1},
    {VIR_OP_CONJ, /* gcSL_CONJ, 0x94 */ VIR_MOD_NONE, 0, -1},
    {VIR_OP_CMUL, /* gcSL_CMUL, 0x95 */ VIR_MOD_NONE, 0, -1},
    {VIR_OP_CMADCJ, /* gcSL_CMADCJ, 0x96 */ VIR_MOD_NONE, 0, -1},
    {VIR_OP_CMULCJ, /* gcSL_CMULCJ, 0x97 */ VIR_MOD_NONE, 0, -1},
    {VIR_OP_CADDCJ, /* gcSL_CADDCJ, 0x98 */ VIR_MOD_NONE, 0, -1},
    {VIR_OP_CSUBCJ, /* gcSL_CSUBCJ, 0x99 */ VIR_MOD_NONE, 0, -1},
    {VIR_OP_CADD, /* gcSL_CADD, 0x9A */ VIR_MOD_NONE, 0, -1},
    {VIR_OP_IMG_TYPE, /* gcSL_GET_IMAGE_TYPE, 0x9B */ VIR_MOD_NONE, 0, -1},
    {VIR_OP_CLAMPCOORD, /* gcSL_CLAMPCOORD, */     VIR_MOD_NONE, 0, 0},
    {VIR_OP_MAXOPCODE, /* gcSL_MAXOPCODE */                    VIR_MOD_NONE, 0, 0},
};

char _check_virOpcodeMap_size[sizeof(_virOpcodeMap)/sizeof(_virOpcodeMap[0]) == gcSL_MAXOPCODE+1];

VIR_ConditionOp virConditionMap[] =
{
    VIR_COP_ALWAYS, /* gcSL_ALWAYS, 0x0  */
    VIR_COP_NOT_EQUAL, /* gcSL_NOT_EQUAL, 0x1  */
    VIR_COP_LESS_OR_EQUAL, /* gcSL_LESS_OR_EQUAL, 0x2  */
    VIR_COP_LESS, /* gcSL_LESS, 0x3  */
    VIR_COP_EQUAL, /* gcSL_EQUAL, 0x4  */
    VIR_COP_GREATER, /* gcSL_GREATER, 0x5  */
    VIR_COP_GREATER_OR_EQUAL, /* gcSL_GREATER_OR_EQUAL, 0x6  */
    VIR_COP_AND, /* gcSL_AND, 0x7  */
    VIR_COP_OR, /* gcSL_OR, 0x8  */
    VIR_COP_XOR, /* gcSL_XOR, 0x9  */
    VIR_COP_NOT_ZERO, /* gcSL_NOT_ZERO, 0xA  */
    VIR_COP_NOT, /* gcSL_ZERO, 0xB  */
    VIR_COP_GREATER_OR_EQUAL_ZERO, /* gcSL_GREATER_OR_EQUAL_ZERO, 0xC  */
    VIR_COP_GREATER_ZERO, /* gcSL_GREATER_ZERO, 0xD  */
    VIR_COP_LESS_OREQUAL_ZERO, /* gcSL_LESS_OREQUAL_ZERO, 0xE  */
    VIR_COP_LESS_ZERO, /* gcSL_LESS_ZERO, 0xF  */
    VIR_COP_ALLMSB, /* gcSL_ALLMSB, 0x10  */
    VIR_COP_ANYMSB, /* gcSL_ANYMSB, 0x11  */
    VIR_COP_SELMSB, /* gcSL_SELMSB, 0x12  */
};

VIR_TypeId _virFormatTypeId[] =
{
    VIR_TYPE_FLOAT32, /*gcSL_FLOAT = 0, 0 */
    VIR_TYPE_INT32, /*gcSL_INTEGER = 1, 1 */
    VIR_TYPE_BOOLEAN, /*gcSL_BOOLEAN = 2, 2 */
    VIR_TYPE_UINT32, /*gcSL_UINT32 = 3, 3 */
    VIR_TYPE_INT8, /*gcSL_INT8, 4 */
    VIR_TYPE_UINT8, /*gcSL_UINT8, 5 */
    VIR_TYPE_INT16, /*gcSL_INT16, 6 */
    VIR_TYPE_UINT16, /*gcSL_UINT16, 7 */
    VIR_TYPE_INT64, /*gcSL_INT64, 8 */
    VIR_TYPE_UINT64, /*gcSL_UINT64, 9 */
    VIR_TYPE_SNORM8, /*gcSL_SNORM8, 10 */
    VIR_TYPE_UNORM8, /*gcSL_UNORM8, 11 */
    VIR_TYPE_FLOAT16, /*gcSL_FLOAT16, 12 */
    VIR_TYPE_FLOAT64, /*gcSL_FLOAT64, 13 */   /* Reserved for future enhancement. */
    VIR_TYPE_SNORM16, /*gcSL_SNORM16, 14 */
    VIR_TYPE_UNORM16  /*gcSL_UNORM16, 15 */
};

typedef struct _conv2VirsVirRelAddrModeMap
{
    VIR_Indexed indexed;
    VIR_Swizzle swizzle;
}
conv2VirsVirRelAddrModeMap;

conv2VirsVirRelAddrModeMap _virRelAddrModeMap[] =
{
    {VIR_INDEXED_NONE, VIR_SWIZZLE_INVALID}, /* gcSL_NOT_INDEXED, 0 */
    {VIR_INDEXED_X, VIR_SWIZZLE_XXXX}, /* gcSL_INDEXED_X, 1 */
    {VIR_INDEXED_Y, VIR_SWIZZLE_YYYY}, /* gcSL_INDEXED_Y, 2 */
    {VIR_INDEXED_Z, VIR_SWIZZLE_ZZZZ}, /* gcSL_INDEXED_Z, 3 */
    {VIR_INDEXED_W, VIR_SWIZZLE_WWWW}         /* gcSL_INDEXED_W, 4 */
};

#define _gcmMakeSWIZZLE(Component1, Component2, Component3, Component4) \
    (\
    ((Component1) << 0) | \
    ((Component2) << 2) | \
    ((Component3) << 4) | \
    ((Component4) << 6)   \
    )

VIR_Swizzle _virSwizzleMap[] =
{
    VIR_SWIZZLE_X, /* gcSL_SWIZZLE_X, 0x0 */
    VIR_SWIZZLE_Y, /* gcSL_SWIZZLE_Y, 0x1 */
    VIR_SWIZZLE_Z, /* gcSL_SWIZZLE_Z, 0x2 */
    VIR_SWIZZLE_W, /* gcSL_SWIZZLE_W, 0x3 */
};

VIR_Enable _virEnableMap[] =
{
    VIR_ENABLE_NONE, /* gcSL_ENABLE_NONE                    = 0x0, none is enabled, error/uninitialized state */
    VIR_ENABLE_X, /* gcSL_ENABLE_X                        = 0x1 */
    VIR_ENABLE_Y, /* gcSL_ENABLE_Y                        = 0x2 */
    VIR_ENABLE_Z, /* gcSL_ENABLE_Z                        = 0x4 */
    VIR_ENABLE_W, /* gcSL_ENABLE_W                        = 0x8 */
};

typedef struct _conv2VirsVirRegMap
{
    VIR_Function *inFunction;
    VIR_SymId     virRegId;   /* virreg symbol id */
    gctUINT       components : 10;
    gctBOOL       packed : 2;
}
conv2VirsVirRegMap;

typedef struct _conv2VirsInstMap
{
    VIR_Function *inFunction;
    VIR_LabelId    labelId;
    VIR_Instruction *inst;
}
conv2VirsInstMap;

typedef struct _conv2VirsVirBuiltinMap
{
    gctSTRING  builtinName;
    VIR_StorageClass storageClass[gcSHADER_KIND_COUNT];
}
conv2VirsVirBuiltinMap;

conv2VirsVirBuiltinMap _virBuiltinMap[] =
{
    { gcvNULL, {0, }},
    { "gl_Position", {0, VIR_STORAGE_OUTPUT, VIR_STORAGE_INPUT, 0,}},
    { "gl_PointSize", {0, VIR_STORAGE_OUTPUT, 0, }},
    { "gl_Color", {0, 0, VIR_STORAGE_OUTPUT, 0,}},
    { "gl_FrontFacing", {0, 0, VIR_STORAGE_INPUT, 0,}},
    { "gl_PointCoord", {0, VIR_STORAGE_OUTPUT, VIR_STORAGE_INPUT, 0,}},
    { "gl_Position.w", {0, VIR_STORAGE_OUTPUT, VIR_STORAGE_INPUT, 0,}},
    { "gl_FragDepth", {0, 0, VIR_STORAGE_OUTPUT,}},
    { "gl_FogFragCoord",{0, 0, VIR_STORAGE_INPUT, 0,}},
    { "gl_VertexID", {0, VIR_STORAGE_INPUT, 0,}},
    { "gl_InstanceID", {0, VIR_STORAGE_INPUT, 0,}},
    { "gl_Work_group_id", {0, 0, 0, VIR_STORAGE_INPUT,}},
    { "gl_Local_invocation_id", {0, 0, 0, VIR_STORAGE_INPUT,}},
    { "gl_Global_invocation_id",{0, 0, 0, VIR_STORAGE_INPUT,}},
    { "gl_HelperInvocation", {0, 0, VIR_STORAGE_INPUT,}},
    { "gl_Front_color", {0, VIR_STORAGE_INPUT, 0,}},
    { "gl_Back_color", {0, VIR_STORAGE_INPUT, 0,}},
    { "gl_Front_secondary_color", {0, VIR_STORAGE_INPUT, 0,}},
    { "gl_Back_secondary_color", {0, VIR_STORAGE_INPUT, 0,}},
    { "gl_Tex_coord", {0, VIR_STORAGE_INPUT, 0,}},
    { "#subsample_depth", {0, 0, VIR_STORAGE_INOUTPUT, 0,}},
    { "gl_PerVertex", {0, VIR_STORAGE_UNKNOWN, 0,}},
    { "gl_in", {0, VIR_STORAGE_INPUT, 0,}},
    { "gl_out", {0, VIR_STORAGE_OUTPUT, 0,}},
    { "gl_InvocationID", {0, VIR_STORAGE_INPUT, 0,}},
    { "gl_PatchVerticesIn", {0, VIR_STORAGE_INPUT, 0,}},
    { "gl_PrimitiveID", {0, VIR_STORAGE_INPUT, 0,}},
    { "gl_TessLevelOuter", {0, VIR_STORAGE_PERPATCH_INOUT, 0,}},
    { "gl_TessLevelInner", {0, VIR_STORAGE_PERPATCH_INOUT, 0,}},
    { "gl_Layer", {0, VIR_STORAGE_INPUT, 0,}},
    { "gl_PrimitiveIDIn", {0, VIR_STORAGE_INPUT, 0,}},
    { "gl_TessCoord", {0, VIR_STORAGE_INPUT, 0,}},
    { "gl_SampleID", {0, 0, VIR_STORAGE_INPUT,}},
    { "gl_SamplePosition", {0, 0, VIR_STORAGE_INPUT,}},
    { "gl_SampleMaskIn", {0, 0, VIR_STORAGE_INPUT,}},
    { "gl_SampleMask", {0, 0, VIR_STORAGE_INOUTPUT,}},
    { "gl_in.gl_Position", {0, 0, VIR_STORAGE_INPUT,}},
    { "gl_in.gl_PointSize", {0, 0, VIR_STORAGE_INPUT,}},
    { "gl_BoundingBox", {0, VIR_STORAGE_PERPATCH_INOUT, 0,}},
    { "gl_LastFragData", {0, VIR_STORAGE_INPUT, 0,}},
    { "#cluster_id", {0, 0, 0, VIR_STORAGE_INPUT,}},
};

#define _gcdBuiltinMapCount (sizeof(_virBuiltinMap)/sizeof(conv2VirsVirBuiltinMap))
/* compile-time assertion to make sure all gcSL builtin names
 * are included in _virBuiltinMap */
extern char _verifyBuiltinNameArray[_gcdBuiltinMapCount == gcSL_BUILTINNAME_COUNT];

static gctSTRING
_ConstructVariableName(
    IN gctSTRING Prefix,
    IN gctSTRING VariableName
    );

static VSC_ErrCode
_ResolveNameClash(
    IN VIR_Shader * VirShader,
    IN VIR_NameId   OrgNameId,
    IN gctSTRING    Separator,
    IN gctUINT      Resolution,
    OUT VIR_NameId *NewNameId
    );

static VIR_TypeId
_ConvScalarFormatToVirVectorTypeId(
    IN gcSL_FORMAT Format,
    IN gctUINT Components,
    IN gctBOOL Packed
    )
{
    switch (Format)
    {
    case gcSL_FLOAT:
        switch (Components)
        {
        case 1: return VIR_TYPE_FLOAT32;
        case 2: return VIR_TYPE_FLOAT_X2;
        case 3: return VIR_TYPE_FLOAT_X3;
        case 4: return VIR_TYPE_FLOAT_X4;
        case 8: return VIR_TYPE_FLOAT_X8;
        case 16: return VIR_TYPE_FLOAT_X16;
        case 32: return VIR_TYPE_FLOAT_X32;

        default:
            return VIR_TYPE_FLOAT_X4;
        }

    case gcSL_FLOAT64:
        switch (Components)
        {
        case 1: return VIR_TYPE_FLOAT32;
        case 2: return VIR_TYPE_FLOAT_X2;
        case 3: return VIR_TYPE_FLOAT_X3;
        case 4: return VIR_TYPE_FLOAT_X4;
        case 8: return VIR_TYPE_FLOAT_X8;
        case 16: return VIR_TYPE_FLOAT_X16;
        case 32: return VIR_TYPE_FLOAT_X32;

        default:
            return VIR_TYPE_FLOAT_X4;
        }

    case gcSL_FLOAT16:
        if(Packed)
        {
            switch (Components)
            {
            case 1: return VIR_TYPE_FLOAT16;
            case 2: return VIR_TYPE_FLOAT16_P2;
            case 3: return VIR_TYPE_FLOAT16_P3;
            case 4: return VIR_TYPE_FLOAT16_P4;
            case 8: return VIR_TYPE_FLOAT16_P8;
            case 16: return VIR_TYPE_FLOAT16_P16;
            case 32: return VIR_TYPE_FLOAT16_P32;

            default:
                return VIR_TYPE_FLOAT16_P8;
            }
        }
        else
        {
            switch (Components)
            {
            case 1: return VIR_TYPE_FLOAT16;
            case 2: return VIR_TYPE_FLOAT16_X2;
            case 3: return VIR_TYPE_FLOAT16_X3;
            case 4: return VIR_TYPE_FLOAT16_X4;
            case 8: return VIR_TYPE_FLOAT16_X8;
            case 16: return VIR_TYPE_FLOAT16_X16;
            case 32: return VIR_TYPE_FLOAT16_X32;

            default:
                return VIR_TYPE_FLOAT16_X4;
            }
        }

    case gcSL_BOOLEAN:
        if(Packed)
        {
            switch (Components)
            {
            case 1: return VIR_TYPE_BOOLEAN;
            case 2: return VIR_TYPE_BOOLEAN_P2;
            case 3: return VIR_TYPE_BOOLEAN_P3;
            case 4: return VIR_TYPE_BOOLEAN_P4;
            case 8: return VIR_TYPE_BOOLEAN_P8;
            case 16: return VIR_TYPE_BOOLEAN_P16;
            case 32: return VIR_TYPE_BOOLEAN_P32;

            default:
                return VIR_TYPE_BOOLEAN_P16;
            }
        }
        else {
            switch (Components)
            {
            case 1: return VIR_TYPE_BOOLEAN;
            case 2: return VIR_TYPE_BOOLEAN_X2;
            case 3: return VIR_TYPE_BOOLEAN_X3;
            case 4: return VIR_TYPE_BOOLEAN_X4;
            case 8: return VIR_TYPE_BOOLEAN_X8;
            case 16: return VIR_TYPE_BOOLEAN_X16;
            case 32: return VIR_TYPE_BOOLEAN_X32;

            default:
                return VIR_TYPE_BOOLEAN_X4;
            }
        }

    case gcSL_INTEGER:
        switch (Components)
        {
        case 1: return VIR_TYPE_INT32;
        case 2: return VIR_TYPE_INTEGER_X2;
        case 3: return VIR_TYPE_INTEGER_X3;
        case 4: return VIR_TYPE_INTEGER_X4;
        case 8: return VIR_TYPE_INTEGER_X8;
        case 16: return VIR_TYPE_INTEGER_X16;
        case 32: return VIR_TYPE_INTEGER_X32;

        default:
            return VIR_TYPE_INTEGER_X4;
        }

    case gcSL_INT16:
        if(Packed)
        {
            switch (Components)
            {
            case 1: return VIR_TYPE_INT16;
            case 2: return VIR_TYPE_INT16_P2;
            case 3: return VIR_TYPE_INT16_P3;
            case 4: return VIR_TYPE_INT16_P4;
            case 8: return VIR_TYPE_INT16_P8;
            case 16: return VIR_TYPE_INT16_P16;
            case 32: return VIR_TYPE_INT16_P32;

            default:
                return VIR_TYPE_INT16_P8;
            }
        }
        else {
            switch (Components)
            {
            case 1: return VIR_TYPE_INT16;
            case 2: return VIR_TYPE_INT16_X2;
            case 3: return VIR_TYPE_INT16_X3;
            case 4: return VIR_TYPE_INT16_X4;
            case 8: return VIR_TYPE_INT16_X8;
            case 16: return VIR_TYPE_INT16_X16;
            case 32: return VIR_TYPE_INT16_X32;

            default:
                return VIR_TYPE_INT16_X4;
            }
        }

    case gcSL_INT8:
        if(Packed)
        {
            switch (Components)
            {
            case 1: return VIR_TYPE_INT8;
            case 2: return VIR_TYPE_INT8_P2;
            case 3: return VIR_TYPE_INT8_P3;
            case 4: return VIR_TYPE_INT8_P4;
            case 8: return VIR_TYPE_INT8_P8;
            case 16: return VIR_TYPE_INT8_P16;
            case 32: return VIR_TYPE_INT8_P32;

            default:
                return VIR_TYPE_INT8_P16;
            }
        }
        else
        {
            switch (Components)
            {
            case 1: return VIR_TYPE_INT8;
            case 2: return VIR_TYPE_INT8_X2;
            case 3: return VIR_TYPE_INT8_X3;
            case 4: return VIR_TYPE_INT8_X4;
            case 8: return VIR_TYPE_INT8_X8;
            case 16: return VIR_TYPE_INT8_X16;
            case 32: return VIR_TYPE_INT8_X32;

            default:
                return VIR_TYPE_INT8_X4;
            }
        }

    case gcSL_UINT32:
        switch (Components)
        {
        case 1: return VIR_TYPE_UINT32;
        case 2: return VIR_TYPE_UINT_X2;
        case 3: return VIR_TYPE_UINT_X3;
        case 4: return VIR_TYPE_UINT_X4;
        case 8: return VIR_TYPE_UINT_X8;
        case 16: return VIR_TYPE_UINT_X16;
        case 32: return VIR_TYPE_UINT_X32;

        default:
            return VIR_TYPE_UINT_X4;
        }

    case gcSL_UINT16:
        if(Packed)
        {
            switch (Components)
            {
            case 1: return VIR_TYPE_UINT16;
            case 2: return VIR_TYPE_UINT16_P2;
            case 3: return VIR_TYPE_UINT16_P3;
            case 4: return VIR_TYPE_UINT16_P4;
            case 8: return VIR_TYPE_UINT16_P8;
            case 16: return VIR_TYPE_UINT16_P16;
            case 32: return VIR_TYPE_UINT16_P32;

            default:
                return VIR_TYPE_UINT16_P8;
            }
        }
        else {
            switch (Components)
            {
            case 1: return VIR_TYPE_UINT16;
            case 2: return VIR_TYPE_UINT16_X2;
            case 3: return VIR_TYPE_UINT16_X3;
            case 4: return VIR_TYPE_UINT16_X4;
            case 8: return VIR_TYPE_UINT16_X8;
            case 16: return VIR_TYPE_UINT16_X16;
            case 32: return VIR_TYPE_UINT16_X32;

            default:
                return VIR_TYPE_UINT16_X4;
            }
        }

    case gcSL_UINT8:
        if(Packed)
        {
            switch (Components)
            {
            case 1: return VIR_TYPE_UINT8;
            case 2: return VIR_TYPE_UINT8_P2;
            case 3: return VIR_TYPE_UINT8_P3;
            case 4: return VIR_TYPE_UINT8_P4;
            case 8: return VIR_TYPE_UINT8_P8;
            case 16: return VIR_TYPE_UINT8_P16;
            case 32: return VIR_TYPE_UINT8_P32;

            default:
                return VIR_TYPE_UINT8_P16;
            }
        }
        else
        {
            switch (Components)
            {
            case 1: return VIR_TYPE_UINT8;
            case 2: return VIR_TYPE_UINT8_X2;
            case 3: return VIR_TYPE_UINT8_X3;
            case 4: return VIR_TYPE_UINT8_X4;
            case 8: return VIR_TYPE_UINT8_X8;
            case 16: return VIR_TYPE_UINT8_X16;
            case 32: return VIR_TYPE_UINT8_X32;

            default:
                return VIR_TYPE_UINT8_X4;
            }
        }

    case gcSL_SNORM8:
        switch (Components)
        {
        case 1: return VIR_TYPE_SNORM8;

        default:
            return VIR_TYPE_SNORM8;
        }

    case gcSL_INT64:
        switch (Components)
        {
        case 1: return VIR_TYPE_INT64;
        case 2: return VIR_TYPE_INT64_X2;
        case 3: return VIR_TYPE_INT64_X3;
        case 4: return VIR_TYPE_INT64_X4;
        case 8: return VIR_TYPE_INT64_X8;
        case 16: return VIR_TYPE_INT64_X16;
        case 32: return VIR_TYPE_INT64_X32;

        default:
            return VIR_TYPE_INT64_X2;
        }

    case gcSL_UNORM8:
        switch (Components)
        {
        case 1: return VIR_TYPE_UNORM8;

        default:
            return VIR_TYPE_UNORM8;
        }

    case gcSL_UINT64:
        switch (Components)
        {
        case 1: return VIR_TYPE_UINT64;
        case 2: return VIR_TYPE_UINT64_X2;
        case 3: return VIR_TYPE_UINT64_X3;
        case 4: return VIR_TYPE_UINT64_X4;
        case 8: return VIR_TYPE_UINT64_X8;
        case 16: return VIR_TYPE_UINT64_X16;
        case 32: return VIR_TYPE_UINT64_X32;

        default:
            return VIR_TYPE_UINT64_X2;
        }

    case gcSL_VOID:
        return VIR_TYPE_VOID;

    case gcSL_SAMPLER_T:             /* OCL sampler_t */
        return VIR_TYPE_SAMPLER_T;
    case gcSL_EVENT_T:               /* OCL event */
        return VIR_TYPE_EVENT_T;
    case gcSL_SIZE_T:                /* OCL size_t */
    case gcSL_PTRDIFF_T:             /* OCL prtdiff_t */
        return VIR_TYPE_UINT32;      /* to uint64 if HW support 64 bit */

    case gcSL_INTPTR_T:              /* OCL intptr_t */
    case gcSL_UINTPTR_T:             /* OCL uintptr_t */
        return VIR_TYPE_UINT32;

    case gcSL_STRUCT:                /* OCL struct */
    case gcSL_UNION:                 /* OCL union */
    case gcSL_ENUM:                  /* OCL enum */
    case gcSL_TYPEDEF:               /* OCL typedef */

    default:
        gcmASSERT(0);
        return VIR_TYPE_FLOAT_X4;
    }
}
/*******************************************************************************
**  _gcVIR_SHADER_Clean
**
**  Clean up memory used in a VIR_Shader object.
**
**  INPUT:
**
**      VIR_Shader VirShader
**          Pointer to a VIR_Shader object.
**
**  OUTPUT:
**
**      Nothing.
*/
static gceSTATUS
_gcVIR_SHADER_Clean(
    IN VIR_Shader *VirShader
    )
{
    gcmHEADER_ARG("VirShader=0x%x", VirShader);
    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

static VIR_VirRegId
_GetVirRegId(
    VIR_Shader *VirShader,
    conv2VirsVirRegMap *VirRegMapArr,
    gctINT Index,
    gcSL_FORMAT Format,
    gctBOOL IsFromSampler,
    gctUINT Components,
    gctBOOL Packed,
    VIR_Precision   Precision
    );

static VIR_TyQualifier
_ConvTypeQualifier2Vir(
    gceTYPE_QUALIFIER qualifier
    )
{
    VIR_TyQualifier virQualifier = VIR_TYQUAL_NONE;

    if (qualifier & gcvTYPE_QUALIFIER_VOLATILE)
    {
        virQualifier |= VIR_TYQUAL_VOLATILE;
    }
    if (qualifier & gcvTYPE_QUALIFIER_RESTRICT)
    {
        virQualifier |= VIR_TYQUAL_RESTRICT;
    }
    if (qualifier & gcvTYPE_QUALIFIER_READ_ONLY)
    {
        virQualifier |= VIR_TYQUAL_READ_ONLY;
    }
    if (qualifier & gcvTYPE_QUALIFIER_WRITE_ONLY)
    {
        virQualifier |= VIR_TYQUAL_WRITE_ONLY;
    }
    if (qualifier & gcvTYPE_QUALIFIER_CONST)
    {
        virQualifier |= VIR_TYQUAL_CONST;
    }

    return virQualifier;
}

static VIR_AddrSpace
_ConvAddrSpaceQualifier2Vir(
    gceTYPE_QUALIFIER qualifier
    )
{
    VIR_AddrSpace addrSpace = VIR_AS_PRIVATE;

    if (qualifier & gcvTYPE_QUALIFIER_PRIVATE)
    {
        addrSpace = VIR_AS_PRIVATE;
    }
    else if (qualifier & gcvTYPE_QUALIFIER_GLOBAL)
    {
        addrSpace = VIR_AS_GLOBAL;
    }
    else if (qualifier & gcvTYPE_QUALIFIER_CONSTANT)
    {
        addrSpace = VIR_AS_CONSTANT;
    }
    else if (qualifier & gcvTYPE_QUALIFIER_LOCAL)
    {
        addrSpace = VIR_AS_LOCAL;
    }

    return addrSpace;
}

#define _gcmConvConstVal2VirConstValue(FromVal, ToVal) \
    gcoOS_MemCopy(ToVal, FromVal, sizeof(gcsValue))

static VIR_LayoutQual
_gcmConvMemoryLayout2Vir(
    gceINTERFACE_BLOCK_LAYOUT_ID MemoryLayout
    )
{
    VIR_LayoutQual virLayout = VIR_LAYQUAL_NONE;

    if (MemoryLayout & gcvINTERFACE_BLOCK_SHARED)
    {
        virLayout |= VIR_LAYQUAL_SHARED;
    }
    if (MemoryLayout & gcvINTERFACE_BLOCK_STD140)
    {
        virLayout |= VIR_LAYQUAL_STD140;
    }
    if (MemoryLayout & gcvINTERFACE_BLOCK_PACKED)
    {
        virLayout |= VIR_LAYQUAL_PACKED;
    }
    if (MemoryLayout & gcvINTERFACE_BLOCK_STD430)
    {
        virLayout |= VIR_LAYQUAL_STD430;
    }
    if (MemoryLayout & gcvINTERFACE_BLOCK_ROW_MAJOR)
    {
        virLayout |= VIR_LAYQUAL_ROW_MAJOR;
    }

    return virLayout;
}

static VIR_Precision
_gcmConvPrecision2Vir(
    gcSHADER_PRECISION Precision)
{
    VIR_Precision   retPrecision = VIR_PRECISION_MEDIUM;

    switch (Precision)
    {
    case gcSHADER_PRECISION_DEFAULT:
        retPrecision = VIR_PRECISION_DEFAULT;
        break;
    case gcSHADER_PRECISION_LOW:
        retPrecision = VIR_PRECISION_LOW;
        break;
    case gcSHADER_PRECISION_MEDIUM:
        retPrecision = VIR_PRECISION_MEDIUM;
        break;
    case gcSHADER_PRECISION_HIGH:
        retPrecision = VIR_PRECISION_HIGH;
        break;
    case gcSHADER_PRECISION_ANY:
        retPrecision = VIR_PRECISION_ANY;
        break;
    default:
        gcmASSERT(0);
        break;
    }

    return retPrecision;
}

static VIR_Swizzle
_ConvSwizzle2Vir(
    IN gctUINT Swizzle
    )
{
    gctUINT i;
    VIR_Swizzle virSwizzle = VIR_SWIZZLE_X;

    for(i = 0; i < 8; i+=2) {
        virSwizzle |= _virSwizzleMap[(Swizzle >> i) & 0x3] << i;
    }
    return  virSwizzle;
}

static VIR_ImageFormat
_ConvImageFormat2Vir(
    IN gceIMAGE_FORMAT ImageFormat
    )
{
    VIR_ImageFormat virImageFormat = VIR_IMAGE_FORMAT_NONE;

    switch (ImageFormat)
    {
    case gcIMAGE_FORMAT_RGBA32F:
        virImageFormat = VIR_IMAGE_FORMAT_RGBA32F;
        break;
    case gcIMAGE_FORMAT_RGBA16F:
        virImageFormat = VIR_IMAGE_FORMAT_RGBA16F;
        break;
    case gcIMAGE_FORMAT_R32F:
        virImageFormat = VIR_IMAGE_FORMAT_R32F;
        break;
    case gcIMAGE_FORMAT_RGBA8:
        virImageFormat = VIR_IMAGE_FORMAT_RGBA8;
        break;
    case gcIMAGE_FORMAT_RGBA8_SNORM:
        virImageFormat = VIR_IMAGE_FORMAT_RGBA8_SNORM;
        break;
    case gcIMAGE_FORMAT_RGBA32I:
        virImageFormat = VIR_IMAGE_FORMAT_RGBA32I;
        break;
    case gcIMAGE_FORMAT_RGBA16I:
        virImageFormat = VIR_IMAGE_FORMAT_RGBA16I;
        break;
    case gcIMAGE_FORMAT_RGBA8I:
        virImageFormat = VIR_IMAGE_FORMAT_RGBA8I;
        break;
    case gcIMAGE_FORMAT_R32I:
        virImageFormat = VIR_IMAGE_FORMAT_R32I;
        break;
    case gcIMAGE_FORMAT_RGBA32UI:
        virImageFormat = VIR_IMAGE_FORMAT_RGBA32UI;
        break;
    case gcIMAGE_FORMAT_RGBA16UI:
        virImageFormat = VIR_IMAGE_FORMAT_RGBA16UI;
        break;
    case gcIMAGE_FORMAT_RGBA8UI:
        virImageFormat = VIR_IMAGE_FORMAT_RGBA8UI;
        break;
    case gcIMAGE_FORMAT_R32UI:
        virImageFormat = VIR_IMAGE_FORMAT_R32UI;
        break;
    default:
        break;
    }

    return  virImageFormat;
}

static VIR_Enable
_ConvEnable2Vir(
    IN gctUINT Enable
    )
{
    VIR_Enable virEnable = VIR_ENABLE_NONE;

    if(Enable & gcSL_ENABLE_X) {
        virEnable |= VIR_ENABLE_X;
    }
    if(Enable & gcSL_ENABLE_Y) {
        virEnable |= VIR_ENABLE_Y;
    }
    if(Enable & gcSL_ENABLE_Z) {
        virEnable |= VIR_ENABLE_Z;
    }
    if(Enable & gcSL_ENABLE_W) {
        virEnable |= VIR_ENABLE_W;
    }
    return  virEnable;
}

static VIR_RoundMode
_ConvRound2Vir(
    IN gcSL_ROUND Round
    )
{
    switch(Round)
    {
    case gcSL_ROUND_DEFAULT:
        return VIR_ROUND_DEFAULT;
    case gcSL_ROUND_RTNE:
        return VIR_ROUND_RTE;
    case gcSL_ROUND_RTZ:
        return VIR_ROUND_RTZ;
    case gcSL_ROUND_RTP:
        return VIR_ROUND_RTP;
    case gcSL_ROUND_RTN:
        return VIR_ROUND_RTN;
    default:
        gcmASSERT(0);
    }
    return VIR_ROUND_DEFAULT;
}
static gctUINT
_GetEnableComponentCount(
    IN gctUINT Enable
    )
{
    gctUINT components = 0;

    if(Enable & gcSL_ENABLE_X) {
        components++;
    }
    if(Enable & gcSL_ENABLE_Y) {
        components++;
    }
    if(Enable & gcSL_ENABLE_Z) {
        components++;
    }
    if(Enable & gcSL_ENABLE_W) {
        components++;
    }
    return  components;
}

static gctUINT
_GetEnableMaxComponentCount(
    IN gctUINT Enable
    )
{
    if(Enable & gcSL_ENABLE_W) {
        return 4;
    }
    if(Enable & gcSL_ENABLE_Z) {
        return 3;
    }
    if(Enable & gcSL_ENABLE_Y) {
        return 2;
    }
    if(Enable & gcSL_ENABLE_X) {
        return 1;
    }
    return  0;
}


#define _gcmGetBuiltinNameString(BuiltinKind) \
   (((gctINT)(BuiltinKind) < 0 &&  (-1 * (gctINT)(BuiltinKind)) < (gctINT)_gcdBuiltinMapCount) \
       ? _virBuiltinMap[-1 * (gctINT)(BuiltinKind)].builtinName \
       : gcvNULL)

#define _gcmGetVirBuiltinStorageClass(ShaderType, BuiltinKind) \
   (((gctINT)(BuiltinKind) < 0 &&  (-1 * (gctINT)(BuiltinKind)) < (gctINT)_gcdBuiltinMapCount) \
       ? _virBuiltinMap[-1 * (gctINT)(BuiltinKind)].storageClass[(ShaderType)] \
       : VIR_STORAGE_UNKNOWN)

static VIR_UniformKind
_ConvUniformKind2Vir(
    IN gcUNIFORM Uniform
    )
{
    VIR_UniformKind virUniformKind = VIR_UNIFORM_NORMAL;

    switch(GetUniformCategory(Uniform))
    {
    case gcSHADER_VAR_CATEGORY_BLOCK_ADDRESS:
        if(isUniformStorageBlockAddress(Uniform))
        {
            virUniformKind = VIR_UNIFORM_STORAGE_BLOCK_ADDRESS;
        }
        else
        {
            virUniformKind = VIR_UNIFORM_UNIFORM_BLOCK_ADDRESS;
        }
        break;

    case gcSHADER_VAR_CATEGORY_BLOCK_MEMBER:
        virUniformKind = VIR_UNIFORM_BLOCK_MEMBER;
        break;

    case gcSHADER_VAR_CATEGORY_LOD_MIN_MAX:
        virUniformKind = VIR_UNIFORM_LOD_MIN_MAX;
        break;

    case gcSHADER_VAR_CATEGORY_LEVEL_BASE_SIZE:
        virUniformKind = VIR_UNIFORM_LEVEL_BASE_SIZE;
        break;

    case gcSHADER_VAR_CATEGORY_VIEW_INDEX:
        virUniformKind = VIR_UNIFORM_VIEW_INDEX;
        break;

    case gcSHADER_VAR_CATEGORY_NORMAL:
        switch(GetUniformKind(Uniform))
        {
        case gcvUNIFORM_KIND_KERNEL_ARG:
            virUniformKind = VIR_UNIFORM_KERNEL_ARG;
            break;
        case gcvUNIFORM_KIND_KERNEL_ARG_LOCAL:
            virUniformKind = VIR_UNIFORM_KERNEL_ARG_LOCAL;
            break;
        case gcvUNIFORM_KIND_KERNEL_ARG_SAMPLER:
            virUniformKind = VIR_UNIFORM_KERNEL_ARG_SAMPLER;
            break;
        case gcvUNIFORM_KIND_KERNEL_ARG_CONSTANT:
            virUniformKind = VIR_UNIFORM_KERNEL_ARG_CONSTANT;
            break;
        case gcvUNIFORM_KIND_KERNEL_ARG_LOCAL_MEM_SIZE:
            virUniformKind = VIR_UNIFORM_KERNEL_ARG_LOCAL_MEM_SIZE;
            break;
        case gcvUNIFORM_KIND_KERNEL_ARG_PRIVATE:
            virUniformKind = VIR_UNIFORM_KERNEL_ARG_PRIVATE;
            break;
        case gcvUNIFORM_KIND_LOCAL_ADDRESS_SPACE:
            virUniformKind = VIR_UNIFORM_LOCAL_ADDRESS_SPACE;
            break;
        case gcvUNIFORM_KIND_PRIVATE_ADDRESS_SPACE:
            virUniformKind = VIR_UNIFORM_PRIVATE_ADDRESS_SPACE;
            break;
        case gcvUNIFORM_KIND_CONSTANT_ADDRESS_SPACE:
            virUniformKind = VIR_UNIFORM_CONSTANT_ADDRESS_SPACE;
            break;
        case gcvUNIFORM_KIND_GLOBAL_SIZE:
            virUniformKind = VIR_UNIFORM_GLOBAL_SIZE;
            break;
        case gcvUNIFORM_KIND_GLOBAL_WORK_SCALE:
            virUniformKind = VIR_UNIFORM_GLOBAL_WORK_SCALE;
            break;
        case gcvUNIFORM_KIND_LOCAL_SIZE:
            virUniformKind = VIR_UNIFORM_LOCAL_SIZE;
            break;
        case gcvUNIFORM_KIND_NUM_GROUPS:
            virUniformKind = VIR_UNIFORM_NUM_GROUPS;
            break;
        case gcvUNIFORM_KIND_NUM_GROUPS_FOR_SINGLE_GPU:
            virUniformKind = VIR_UNIFORM_NUM_GROUPS_FOR_SINGLE_GPU;
            break;
        case gcvUNIFORM_KIND_GLOBAL_OFFSET:
            virUniformKind = VIR_UNIFORM_GLOBAL_OFFSET;
            break;
        case gcvUNIFORM_KIND_WORK_DIM:
            virUniformKind = VIR_UNIFORM_WORK_DIM;
            break;
        case gcvUNIFORM_KIND_TRANSFORM_FEEDBACK_BUFFER:
            virUniformKind = VIR_UNIFORM_TRANSFORM_FEEDBACK_BUFFER;
            break;
        case gcvUNIFORM_KIND_TRANSFORM_FEEDBACK_STATE:
            virUniformKind = VIR_UNIFORM_TRANSFORM_FEEDBACK_STATE;
            break;
        case gcvUNIFORM_KIND_PRINTF_ADDRESS:
            virUniformKind = VIR_UNIFORM_PRINTF_ADDRESS;
            break;
        case gcvUNIFORM_KIND_WORKITEM_PRINTF_BUFFER_SIZE:
            virUniformKind = VIR_UNIFORM_WORKITEM_PRINTF_BUFFER_SIZE;
            break;
        case gcvUNIFORM_KIND_STORAGE_BLOCK_ADDRESS:
            virUniformKind = VIR_UNIFORM_STORAGE_BLOCK_ADDRESS;
            break;
        case gcvUNIFORM_KIND_GENERAL_PATCH:
            virUniformKind = VIR_UNIFORM_GENERAL_PATCH;
            break;
        case gcvUNIFORM_KIND_IMAGE_EXTRA_LAYER:
            virUniformKind = VIR_UNIFORM_EXTRA_LAYER;
            break;
        default:
            virUniformKind = VIR_UNIFORM_NORMAL;
        }
        if (Uniform->nameLength == 11 &&
            gcmIS_SUCCESS(gcoOS_StrNCmp(Uniform->name, "#num_groups", 11)))
        {
            virUniformKind = VIR_UNIFORM_NUM_GROUPS;
        }
        else if (Uniform->nameLength == 17 &&
                 gcmIS_SUCCESS(gcoOS_StrCmp(Uniform->name, "$ConstBorderValue")))
        {
            virUniformKind = VIR_UNIFORM_CONST_BORDER_VALUE;
        }
        break;

    case gcSHADER_VAR_CATEGORY_STRUCT:
    case gcSHADER_VAR_CATEGORY_TOP_LEVEL_STRUCT:
        virUniformKind = VIR_UNIFORM_STRUCT;
        break;

    case gcSHADER_VAR_CATEGORY_SAMPLE_LOCATION:
        virUniformKind = VIR_UNIFORM_SAMPLE_LOCATION;
        break;

    case gcSHADER_VAR_CATEGORY_ENABLE_MULTISAMPLE_BUFFERS:
        virUniformKind = VIR_UNIFORM_ENABLE_MULTISAMPLE_BUFFERS;
        break;

    case gcSHADER_VAR_CATEGORY_GL_SAMPLER_FOR_IMAGE_T:
        virUniformKind = VIR_UNIFORM_GL_SAMPLER_FOR_IMAGE_T;
        break;

    case gcSHADER_VAR_CATEGORY_GL_IMAGE_FOR_IMAGE_T:
        virUniformKind = VIR_UNIFORM_GL_IMAGE_FOR_IMAGE_T;
        break;

    case gcSHADER_VAR_CATEGORY_WORK_THREAD_COUNT:
        virUniformKind = VIR_UNIFORM_WORK_THREAD_COUNT;
        break;

    case gcSHADER_VAR_CATEGORY_WORK_GROUP_COUNT:
        virUniformKind = VIR_UNIFORM_WORK_GROUP_COUNT;

    case gcSHADER_VAR_CATEGORY_WORK_GROUP_ID_OFFSET:
        virUniformKind = VIR_UNIFORM_WORK_GROUP_ID_OFFSET;
        break;

    default:
        gcmASSERT(gcvFALSE);
        break;
    }

    return virUniformKind;
}

static VIR_IB_FLAG
_ConvIBFlag2Vir(
    IN gcsINTERFACE_BLOCK_INFO* IBInfo
    )
{
    VIR_IB_FLAG virIBFlag = VIR_IB_NONE;

    if (HasIBIFlag(IBInfo, gceIB_FLAG_UNSIZED))
    {
        virIBFlag |= VIR_IB_UNSIZED;
    }

    if (HasIBIFlag(IBInfo, gceIB_FLAG_FOR_SHARED_VARIABLE))
    {
        virIBFlag |= VIR_IB_FOR_SHARED_VARIABLE;
    }

    if (HasIBIFlag(IBInfo, gceIB_FLAG_WITH_INSTANCE_NAME))
    {
        virIBFlag |= VIR_IB_WITH_INSTANCE_NAME;
    }

    return virIBFlag;
}

static VIR_SymFlag
_ConvUniformFlag2Vir(
    IN gcUNIFORM Uniform
    )
{
    VIR_SymFlag virSymFlag = VIR_SYMFLAG_NONE;

    if(UniformHasFlag(Uniform, gcvUNIFORM_FLAG_ATOMIC_COUNTER))
    {
        virSymFlag = (VIR_SymFlag)((gctUINT)virSymFlag | VIR_SYMUNIFORMFLAG_ATOMIC_COUNTER);
    }
    if(UniformHasFlag(Uniform, gcvUNIFORM_FLAG_BUILTIN))
    {
        virSymFlag = (VIR_SymFlag)((gctUINT)virSymFlag | VIR_SYMFLAG_BUILTIN);
    }
    if(UniformHasFlag(Uniform, gcvUNIFORM_FLAG_COMPILER_GEN))
    {
        virSymFlag = (VIR_SymFlag)((gctUINT)virSymFlag | VIR_SYMFLAG_COMPILER_GEN);
    }
    if(UniformHasFlag(Uniform, gcvUNIFORM_FLAG_USED_IN_SHADER))
    {
        virSymFlag = (VIR_SymFlag)((gctUINT)virSymFlag | VIR_SYMUNIFORMFLAG_USED_IN_SHADER);
    }
    if(UniformHasFlag(Uniform, gcvUNIFORM_FLAG_USED_IN_LTC))
    {
        virSymFlag = (VIR_SymFlag)((gctUINT)virSymFlag | VIR_SYMUNIFORMFLAG_USED_IN_LTC);
    }
    if(UniformHasFlag(Uniform, gcvUNIFORM_FLAG_STD140_SHARED) || isUniformForceActive(Uniform))
    {
        virSymFlag = (VIR_SymFlag)((gctUINT)virSymFlag | VIR_SYMUNIFORMFLAG_FORCE_ACTIVE);
    }
    if(UniformHasFlag(Uniform, gcvUNIFORM_FLAG_TREAT_SAMPLER_AS_CONST))
    {
        virSymFlag = (VIR_SymFlag)((gctUINT)virSymFlag | VIR_SYMUNIFORMFLAG_TREAT_SAMPLER_AS_CONST);
    }

    return virSymFlag;
}

/******************************************************************************
**  _ConvBuiltinNameKindToVirNameId
**
**  Convert builtin name kind to VIR_NameId.
**
**  INPUT:
**
**      gctSIZE_T Kind
**          Builtin name kind.
**
**  OUTPUT:
**
**      VIR_NameId * VirNameId
**          Pointer to a variable receiving the vir name id.
*/
gceSTATUS
_ConvBuiltinNameKindToVirNameId(
    IN gctINT        Kind,
    OUT VIR_NameId * VirNameId
    )
{
    VIR_NameId nameId;
#define Case(name1, name2)  case name1: nameId = name2; break
    switch(Kind) {
    Case(gcSL_POSITION, VIR_NAME_POSITION);
    Case(gcSL_POINT_SIZE, VIR_NAME_POINT_SIZE);
    Case(gcSL_COLOR, VIR_NAME_COLOR);
    Case(gcSL_FRONT_FACING, VIR_NAME_FRONT_FACING);
    Case(gcSL_POINT_COORD, VIR_NAME_POINT_COORD);
    Case(gcSL_POSITION_W, VIR_NAME_POSITION_W);
    Case(gcSL_FOG_COORD, VIR_NAME_FOG_COORD);
    Case(gcSL_VERTEX_ID, VIR_NAME_VERTEX_ID);
    Case(gcSL_INSTANCE_ID, VIR_NAME_INSTANCE_ID);
    Case(gcSL_DEPTH, VIR_NAME_DEPTH);
    Case(gcSL_FRONT_COLOR, VIR_NAME_FRONT_COLOR);
    Case(gcSL_BACK_COLOR, VIR_NAME_BACK_COLOR);
    Case(gcSL_FRONT_SECONDARY_COLOR, VIR_NAME_FRONT_SECONDARY_COLOR);
    Case(gcSL_BACK_SECONDARY_COLOR, VIR_NAME_BACK_SECONDARY_COLOR);
    Case(gcSL_TEX_COORD, VIR_NAME_TEX_COORD);
    Case(gcSL_WORK_GROUP_ID, VIR_NAME_WORK_GROUP_ID);
    Case(gcSL_LOCAL_INVOCATION_ID, VIR_NAME_LOCAL_INVOCATION_ID);
    Case(gcSL_GLOBAL_INVOCATION_ID, VIR_NAME_GLOBAL_INVOCATION_ID);
    Case(gcSL_HELPER_INVOCATION, VIR_NAME_HELPER_INVOCATION);
    Case(gcSL_SUBSAMPLE_DEPTH, VIR_NAME_SUBSAMPLE_DEPTH);
    Case(gcSL_PERVERTEX, VIR_NAME_PERVERTEX);
    Case(gcSL_IN, VIR_NAME_IN);
    Case(gcSL_OUT, VIR_NAME_OUT);
    Case(gcSL_INVOCATION_ID, VIR_NAME_INVOCATION_ID);
    Case(gcSL_PATCH_VERTICES_IN, VIR_NAME_PATCH_VERTICES_IN);
    Case(gcSL_PRIMITIVE_ID, VIR_NAME_PRIMITIVE_ID);
    Case(gcSL_TESS_LEVEL_OUTER, VIR_NAME_TESS_LEVEL_OUTER);
    Case(gcSL_TESS_LEVEL_INNER, VIR_NAME_TESS_LEVEL_INNER);
    Case(gcSL_LAYER, VIR_NAME_LAYER);
    Case(gcSL_PRIMITIVE_ID_IN, VIR_NAME_PRIMITIVE_ID_IN);
    Case(gcSL_TESS_COORD, VIR_NAME_TESS_COORD);
    Case(gcSL_SAMPLE_ID, VIR_NAME_SAMPLE_ID);
    Case(gcSL_SAMPLE_POSITION, VIR_NAME_SAMPLE_POSITION);
    Case(gcSL_SAMPLE_MASK_IN, VIR_NAME_SAMPLE_MASK_IN);
    Case(gcSL_SAMPLE_MASK, VIR_NAME_SAMPLE_MASK);
    Case(gcSL_IN_POSITION, VIR_NAME_IN_POSITION);
    Case(gcSL_IN_POINT_SIZE, VIR_NAME_IN_POINT_SIZE);
    Case(gcSL_BOUNDING_BOX, VIR_NAME_BOUNDING_BOX);
    Case(gcSL_LAST_FRAG_DATA, VIR_NAME_LAST_FRAG_DATA);
    Case(gcSL_CLUSTER_ID, VIR_NAME_CLUSTER_ID);
    default:
        if((gctINT) Kind < 0) {
            gcmASSERT(0);
        }
        return gcvSTATUS_NOT_FOUND;
    }
#undef Case

    *VirNameId = nameId;

    return gcvSTATUS_OK;
}

#define _gcmConvShaderOpcodeToVirOpcode(Shader, CodeIndex, Opcode) \
    (((Opcode) < gcSL_MAXOPCODE) ? \
        (_virOpcodeMap[(Opcode)].convVirOpCode ? (*_virOpcodeMap[(Opcode)].convVirOpCode)(Shader, CodeIndex) : _virOpcodeMap[(Opcode)].virOpcode) : VIR_OP_MAXOPCODE)

#define _gcmGetShaderOpcodeVirModifier(Opcode) \
    (((Opcode) < gcSL_MAXOPCODE) ? _virOpcodeMap[(Opcode)].modifier : VIR_MOD_NONE)

#define _gcmGetShaderOpcodeVirExternOpcode(Opcode) \
    (((Opcode) < gcSL_MAXOPCODE) ? _virOpcodeMap[(Opcode)].externOpcode : 0)

VIR_PrimitiveTypeId  gcSLType2VIRTypeMapping[] =
{
    VIR_TYPE_FLOAT32, /* gcSHADER_FLOAT_X1   = 0, 0x00 */
    VIR_TYPE_FLOAT_X2, /* gcSHADER_FLOAT_X2, 0x01 */
    VIR_TYPE_FLOAT_X3, /* gcSHADER_FLOAT_X3, 0x02 */
    VIR_TYPE_FLOAT_X4, /* gcSHADER_FLOAT_X4, 0x03 */
    VIR_TYPE_FLOAT_2X2, /* gcSHADER_FLOAT_2X2, 0x04 */
    VIR_TYPE_FLOAT_3X3, /* gcSHADER_FLOAT_3X3, 0x05 */
    VIR_TYPE_FLOAT_4X4, /* gcSHADER_FLOAT_4X4, 0x06 */
    VIR_TYPE_BOOLEAN, /* gcSHADER_BOOLEAN_X1, 0x07 */
    VIR_TYPE_BOOLEAN_X2, /* gcSHADER_BOOLEAN_X2, 0x08 */
    VIR_TYPE_BOOLEAN_X3, /* gcSHADER_BOOLEAN_X3, 0x09 */
    VIR_TYPE_BOOLEAN_X4, /* gcSHADER_BOOLEAN_X4, 0x0A */
    VIR_TYPE_INT32, /* gcSHADER_INTEGER_X1, 0x0B */
    VIR_TYPE_INTEGER_X2, /* gcSHADER_INTEGER_X2, 0x0C */
    VIR_TYPE_INTEGER_X3, /* gcSHADER_INTEGER_X3, 0x0D */
    VIR_TYPE_INTEGER_X4, /* gcSHADER_INTEGER_X4, 0x0E */
    VIR_TYPE_SAMPLER_1D, /* gcSHADER_SAMPLER_1D, 0x0F */
    VIR_TYPE_SAMPLER_2D, /* gcSHADER_SAMPLER_2D, 0x10 */
    VIR_TYPE_SAMPLER_3D, /* gcSHADER_SAMPLER_3D, 0x11 */
    VIR_TYPE_SAMPLER_CUBIC, /* gcSHADER_SAMPLER_CUBIC, 0x12 */
    VIR_TYPE_UNKNOWN, /* gcSHADER_FIXED_X1, 0x13 */
    VIR_TYPE_UNKNOWN, /* gcSHADER_FIXED_X2, 0x14 */
    VIR_TYPE_UNKNOWN, /* gcSHADER_FIXED_X3, 0x15 */
    VIR_TYPE_UNKNOWN, /* gcSHADER_FIXED_X4, 0x16 */

    /* For OCL. */
    VIR_TYPE_IMAGE_1D_T, /* gcSHADER_IMAGE_1D_T, 0x17 */
    VIR_TYPE_IMAGE_1D_BUFFER_T, /* gcSHADER_IMAGE_1D_BUFFER_T, 0x18 */
    VIR_TYPE_IMAGE_1D_ARRAY_T, /* gcSHADER_IMAGE_1D_ARRAY_T, 0x19 */
    VIR_TYPE_IMAGE_2D_T, /* gcSHADER_IMAGE_2D_T, 0x1A */
    VIR_TYPE_IMAGE_2D_ARRAY_T, /* gcSHADER_IMAGE_2D_ARRAY_T, 0x1B */
    VIR_TYPE_IMAGE_3D_T, /* gcSHADER_IMAGE_3D_T, 0x1C */
    VIR_TYPE_VIV_GENERIC_IMAGE_T, /* gcSHADER_VIV_GENERIC_IMAGE_T, 0x1D */
    VIR_TYPE_SAMPLER_T, /* gcSHADER_SAMPLER_T, 0x1E */

    VIR_TYPE_FLOAT_2X3, /* gcSHADER_FLOAT_2X3, 0x1F */
    VIR_TYPE_FLOAT_2X4, /* gcSHADER_FLOAT_2X4, 0x20 */
    VIR_TYPE_FLOAT_3X2, /* gcSHADER_FLOAT_3X2, 0x21 */
    VIR_TYPE_FLOAT_3X4, /* gcSHADER_FLOAT_3X4, 0x22 */
    VIR_TYPE_FLOAT_4X2, /* gcSHADER_FLOAT_4X2, 0x23 */
    VIR_TYPE_FLOAT_4X3, /* gcSHADER_FLOAT_4X3, 0x24 */
    VIR_TYPE_ISAMPLER_2D, /* gcSHADER_ISAMPLER_2D, 0x25 */
    VIR_TYPE_ISAMPLER_3D, /* gcSHADER_ISAMPLER_3D, 0x26 */
    VIR_TYPE_ISAMPLER_CUBIC, /* gcSHADER_ISAMPLER_CUBIC, 0x27 */
    VIR_TYPE_USAMPLER_2D, /* gcSHADER_USAMPLER_2D, 0x28 */
    VIR_TYPE_USAMPLER_3D, /* gcSHADER_USAMPLER_3D, 0x29 */
    VIR_TYPE_USAMPLER_CUBIC, /* gcSHADER_USAMPLER_CUBIC, 0x2A */
    VIR_TYPE_SAMPLER_EXTERNAL_OES, /* gcSHADER_SAMPLER_EXTERNAL_OES, 0x2B */

    VIR_TYPE_UINT32, /* gcSHADER_UINT_X1, 0x2C */
    VIR_TYPE_UINT_X2, /* gcSHADER_UINT_X2, 0x2D */
    VIR_TYPE_UINT_X3, /* gcSHADER_UINT_X3, 0x2E */
    VIR_TYPE_UINT_X4, /* gcSHADER_UINT_X4, 0x2F */

    VIR_TYPE_SAMPLER_2D_SHADOW, /* gcSHADER_SAMPLER_2D_SHADOW, 0x30 */
    VIR_TYPE_SAMPLER_CUBE_SHADOW, /* gcSHADER_SAMPLER_CUBE_SHADOW, 0x31 */

    VIR_TYPE_SAMPLER_1D_ARRAY, /* gcSHADER_SAMPLER_1D_ARRAY, 0x32 */
    VIR_TYPE_SAMPLER_1D_ARRAY_SHADOW, /* gcSHADER_SAMPLER_1D_ARRAY_SHADOW, 0x33 */
    VIR_TYPE_SAMPLER_2D_ARRAY, /* gcSHADER_SAMPLER_2D_ARRAY, 0x34 */
    VIR_TYPE_ISAMPLER_2D_ARRAY, /* gcSHADER_ISAMPLER_2D_ARRAY, 0x35 */
    VIR_TYPE_USAMPLER_2D_ARRAY, /* gcSHADER_USAMPLER_2D_ARRAY, 0x36 */
    VIR_TYPE_SAMPLER_2D_ARRAY_SHADOW, /* gcSHADER_SAMPLER_2D_ARRAY_SHADOW, 0x37 */

    VIR_TYPE_SAMPLER_2D_MS, /* gcSHADER_SAMPLER_2D_MS, 0x38 */
    VIR_TYPE_ISAMPLER_2D_MS, /* gcSHADER_ISAMPLER_2D_MS, 0x39 */
    VIR_TYPE_USAMPLER_2D_MS, /* gcSHADER_USAMPLER_2D_MS, 0x3A */
    VIR_TYPE_SAMPLER_2D_MS_ARRAY, /* gcSHADER_SAMPLER_2D_MS_ARRAY, 0x3B */
    VIR_TYPE_ISAMPLER_2D_MS_ARRAY, /* gcSHADER_ISAMPLER_2D_MS_ARRAY, 0x3C */
    VIR_TYPE_USAMPLER_2D_MS_ARRAY, /* gcSHADER_USAMPLER_2D_MS_ARRAY, 0x3D */

    VIR_TYPE_IMAGE_2D, /* gcSHADER_IMAGE_2D, 0x3E */
    VIR_TYPE_IIMAGE_2D, /* gcSHADER_IIMAGE_2D, 0x3F */
    VIR_TYPE_UIMAGE_2D, /* gcSHADER_UIMAGE_2D, 0x40 */
    VIR_TYPE_IMAGE_3D, /* gcSHADER_IMAGE_3D, 0x41 */
    VIR_TYPE_IIMAGE_3D, /* gcSHADER_IIMAGE_3D, 0x42 */
    VIR_TYPE_UIMAGE_3D, /* gcSHADER_UIMAGE_3D, 0x43 */
    VIR_TYPE_IMAGE_CUBE, /* gcSHADER_IMAGE_CUBE, 0x44 */
    VIR_TYPE_IIMAGE_CUBE, /* gcSHADER_IIMAGE_CUBE, 0x45 */
    VIR_TYPE_UIMAGE_CUBE, /* gcSHADER_UIMAGE_CUBE, 0x46 */
    VIR_TYPE_IMAGE_2D_ARRAY, /* gcSHADER_IMAGE_2D_ARRAY, 0x47 */
    VIR_TYPE_IIMAGE_2D_ARRAY, /* gcSHADER_IIMAGE_2D_ARRAY, 0x48 */
    VIR_TYPE_UIMAGE_2D_ARRAY, /* gcSHADER_UIMAGE_2D_ARRAY, 0x49 */
    VIR_TYPE_VIV_GENERIC_GL_IMAGE, /* gcSHADER_VIV_GENERIC_GL_IMAGE, 0x4A */

    VIR_TYPE_ATOMIC_UINT, /* gcSHADER_ATOMIC_UINT, 0x4B */

    /* GL_EXT_texture_cube_map_array */
    VIR_TYPE_SAMPLER_CUBE_ARRAY, /* gcSHADER_SAMPLER_CUBEMAP_ARRAY, 0x4C */
    VIR_TYPE_SAMPLER_CUBE_ARRAY_SHADOW, /* gcSHADER_SAMPLER_CUBEMAP_ARRAY_SHADOW, 0x4D */
    VIR_TYPE_ISAMPLER_CUBE_ARRAY, /* gcSHADER_ISAMPLER_CUBEMAP_ARRAY, 0x4E */
    VIR_TYPE_USAMPLER_CUBE_ARRAY, /* gcSHADER_USAMPLER_CUBEMAP_ARRAY, 0x4F */
    VIR_TYPE_IMAGE_CUBE_ARRAY, /* gcSHADER_IMAGE_CUBEMAP_ARRAY, 0x50 */
    VIR_TYPE_IIMAGE_CUBE_ARRAY, /* gcSHADER_IIMAGE_CUBEMAP_ARRAY, 0x51 */
    VIR_TYPE_UIMAGE_CUBE_ARRAY, /* gcSHADER_UIMAGE_CUBEMAP_ARRAY, 0x52 */

    VIR_TYPE_INT64, /* gcSHADER_INT64_X1, 0x53 */
    VIR_TYPE_INT64_X2, /* gcSHADER_INT64_X2, 0x54 */
    VIR_TYPE_INT64_X3, /* gcSHADER_INT64_X3, 0x55 */
    VIR_TYPE_INT64_X4, /* gcSHADER_INT64_X4, 0x56 */
    VIR_TYPE_UINT64, /* gcSHADER_UINT64_X1, 0x57 */
    VIR_TYPE_UINT64_X2, /* gcSHADER_UINT64_X2, 0x58 */
    VIR_TYPE_UINT64_X3, /* gcSHADER_UINT64_X3, 0x59 */
    VIR_TYPE_UINT64_X4, /* gcSHADER_UINT64_X4, 0x5A */

    /* texture buffer extension type. */
    VIR_TYPE_SAMPLER_BUFFER, /* gcSHADER_SAMPLER_BUFFER, 0x5B */
    VIR_TYPE_ISAMPLER_BUFFER, /* gcSHADER_ISAMPLER_BUFFER, 0x5C */
    VIR_TYPE_USAMPLER_BUFFER, /* gcSHADER_USAMPLER_BUFFER, 0x5D */
    VIR_TYPE_IMAGE_BUFFER, /* gcSHADER_IMAGE_BUFFER, 0x5E */
    VIR_TYPE_IIMAGE_BUFFER, /* gcSHADER_IIMAGE_BUFFER, 0x5F */
    VIR_TYPE_UIMAGE_BUFFER, /* gcSHADER_UIMAGE_BUFFER, 0x60 */
    VIR_TYPE_VIV_GENERIC_GL_SAMPLER, /* gcSHADER_VIV_GENERIC_GL_SAMPLER, 0x61 */

    /* float16 */
    VIR_TYPE_FLOAT16, /* gcSHADER_FLOAT16_X1, 0x62 half2 */
    VIR_TYPE_FLOAT16_X2, /* gcSHADER_FLOAT16_X2, 0x63 half2 */
    VIR_TYPE_FLOAT16_X3, /* gcSHADER_FLOAT16_X3, 0x64 half3 */
    VIR_TYPE_FLOAT16_X4, /* gcSHADER_FLOAT16_X4, 0x65 half4 */
    VIR_TYPE_FLOAT16_X8, /* gcSHADER_FLOAT16_X8, 0x66 half8 */
    VIR_TYPE_FLOAT16_X16, /* gcSHADER_FLOAT16_X16, 0x67 half16 */
    VIR_TYPE_FLOAT16_X32, /* gcSHADER_FLOAT16_X32, 0x68 half32 */

    /*  boolean  */
    VIR_TYPE_BOOLEAN_X8, /* gcSHADER_BOOLEAN_X8, 0x69 */
    VIR_TYPE_BOOLEAN_X16, /* gcSHADER_BOOLEAN_X16, 0x6A */
    VIR_TYPE_BOOLEAN_X32, /* gcSHADER_BOOLEAN_X32, 0x6B */

    /* uchar vectors  */
    VIR_TYPE_UINT8,
    VIR_TYPE_UINT8_X2,
    VIR_TYPE_UINT8_X3,
    VIR_TYPE_UINT8_X4,
    VIR_TYPE_UINT8_X8,
    VIR_TYPE_UINT8_X16,
    VIR_TYPE_UINT8_X32,

    /* char vectors  */
    VIR_TYPE_INT8,
    VIR_TYPE_INT8_X2,
    VIR_TYPE_INT8_X3,
    VIR_TYPE_INT8_X4,
    VIR_TYPE_INT8_X8,
    VIR_TYPE_INT8_X16,
    VIR_TYPE_INT8_X32,

    /* ushort vectors */
    VIR_TYPE_UINT16,
    VIR_TYPE_UINT16_X2,
    VIR_TYPE_UINT16_X3,
    VIR_TYPE_UINT16_X4,
    VIR_TYPE_UINT16_X8,
    VIR_TYPE_UINT16_X16,
    VIR_TYPE_UINT16_X32,

    /* short vectors */
    VIR_TYPE_INT16,
    VIR_TYPE_INT16_X2,
    VIR_TYPE_INT16_X3,
    VIR_TYPE_INT16_X4,
    VIR_TYPE_INT16_X8,
    VIR_TYPE_INT16_X16,
    VIR_TYPE_INT16_X32,

    /* packed data type */
    /* packed float16 (2 bytes per element) */
    VIR_TYPE_FLOAT16_P2, /* half2 */
    VIR_TYPE_FLOAT16_P3, /* half3 */
    VIR_TYPE_FLOAT16_P4, /* half4 */
    VIR_TYPE_FLOAT16_P8, /* half8 */
    VIR_TYPE_FLOAT16_P16, /* half16 */
    VIR_TYPE_FLOAT16_P32, /* half32 */

    /* packed boolean (1 byte per element) */
    VIR_TYPE_BOOLEAN_P2, /* bool2, bvec2 */
    VIR_TYPE_BOOLEAN_P3,
    VIR_TYPE_BOOLEAN_P4,
    VIR_TYPE_BOOLEAN_P8,
    VIR_TYPE_BOOLEAN_P16,
    VIR_TYPE_BOOLEAN_P32,

    /* uchar vectors (1 byte per element) */
    VIR_TYPE_UINT8_P2,
    VIR_TYPE_UINT8_P3,
    VIR_TYPE_UINT8_P4,
    VIR_TYPE_UINT8_P8,
    VIR_TYPE_UINT8_P16,
    VIR_TYPE_UINT8_P32,

    /* char vectors (1 byte per element) */
    VIR_TYPE_INT8_P2,
    VIR_TYPE_INT8_P3,
    VIR_TYPE_INT8_P4,
    VIR_TYPE_INT8_P8,
    VIR_TYPE_INT8_P16,
    VIR_TYPE_INT8_P32,

    /* ushort vectors (2 bytes per element) */
    VIR_TYPE_UINT16_P2,
    VIR_TYPE_UINT16_P3,
    VIR_TYPE_UINT16_P4,
    VIR_TYPE_UINT16_P8,
    VIR_TYPE_UINT16_P16,
    VIR_TYPE_UINT16_P32,

    /* short vectors (2 bytes per element) */
    VIR_TYPE_INT16_P2,
    VIR_TYPE_INT16_P3,
    VIR_TYPE_INT16_P4,
    VIR_TYPE_INT16_P8,
    VIR_TYPE_INT16_P16,
    VIR_TYPE_INT16_P32,

    VIR_TYPE_INTEGER_X8,
    VIR_TYPE_INTEGER_X16,
    VIR_TYPE_UINT_X8,
    VIR_TYPE_UINT_X16,
    VIR_TYPE_FLOAT_X8,
    VIR_TYPE_FLOAT_X16,
    VIR_TYPE_INT64_X8,
    VIR_TYPE_INT64_X16,
    VIR_TYPE_UINT64_X8,
    VIR_TYPE_UINT64_X16,

    VIR_TYPE_FLOAT64, /* gcSHADER_FLOAT64_X1   0xB6 */
    VIR_TYPE_FLOAT64_X2, /* gcSHADER_FLOAT64_X2   0xB7 */
    VIR_TYPE_FLOAT64_X3, /* gcSHADER_FLOAT64_X3   0xB8 */
    VIR_TYPE_FLOAT64_X4, /* gcSHADER_FLOAT64_X4   0xB9 */
    VIR_TYPE_FLOAT64_2X2, /* gcSHADER_FLOAT64_2X2  0xBA */
    VIR_TYPE_FLOAT64_3X3, /* gcSHADER_FLOAT64_3X3  0xBB */
    VIR_TYPE_FLOAT64_4X4, /* gcSHADER_FLOAT64_4X4  0xBC */
    VIR_TYPE_FLOAT64_2X3, /* gcSHADER_FLOAT64_2X3  0xBD */
    VIR_TYPE_FLOAT64_2X4, /* gcSHADER_FLOAT64_2X4  0xBE */
    VIR_TYPE_FLOAT64_3X2, /* gcSHADER_FLOAT64_3X2  0xBF */
    VIR_TYPE_FLOAT64_3X4, /* gcSHADER_FLOAT64_3X4  0xC0 */
    VIR_TYPE_FLOAT64_4X2, /* gcSHADER_FLOAT64_4X2  0xC1 */
    VIR_TYPE_FLOAT64_4X3, /* gcSHADER_FLOAT64_4X3  0xC2 */
    VIR_TYPE_FLOAT64_X8, /* gcSHADER_FLOAT64_X8   0xC3 */
    VIR_TYPE_FLOAT64_X16, /* gcSHADER_FLOAT64_X16  0xC4 */

    /* OpenGL 4.0 types */
    VIR_TYPE_SAMPLER_2D_RECT, /* gcSHADER_SAMPLER_2D_RECT, 0xC5 */
    VIR_TYPE_ISAMPLER_2D_RECT, /* gcSHADER_ISAMPLER_2D_RECT, 0xC6 */
    VIR_TYPE_USAMPLER_2D_RECT, /* gcSHADER_USAMPLER_2D_RECT, 0xC7 */
    VIR_TYPE_SAMPLER_2D_RECT_SHADOW, /* gcSHADER_SAMPLER_2D_RECT_SHADOW, 0xC8 */
    VIR_TYPE_ISAMPLER_1D_ARRAY, /* gcSHADER_ISAMPLER_1D_ARRAY, 0xC9 */
    VIR_TYPE_USAMPLER_1D_ARRAY, /* gcSHADER_USAMPLER_1D_ARRAY, 0xCA */
    VIR_TYPE_ISAMPLER_1D, /* gcSHADER_ISAMPLER_1D, 0xCB */
    VIR_TYPE_USAMPLER_1D, /* gcSHADER_USAMPLER_1D, 0xCC */
    VIR_TYPE_SAMPLER_1D_SHADOW, /* gcSHADER_SAMPLER_1D_SHADOW, 0xCD */

    VIR_TYPE_UNKNOWN
};
/* compile-time assertion if the gcSLType2VIRTypeMapping is not the same length as gcSHADER_TYPE_COUNT */
const gctINT _verify_TypeMappingTable[sizeof(gcSLType2VIRTypeMapping)/sizeof(VIR_PrimitiveTypeId) == gcSHADER_TYPE_COUNT] = { 0 };

/*******************************************************************************
**  _ConvTypeToVirPrimitiveTypeId
**
**  Convert shader type to VIR type id.
**
**  INPUT:
**
**      gcSHADER_TYPE Type
**          Shader data type.
**
**  OUTPUT:
**
**      VIR_TypeId * TypeId
**          Pointer to a variable receiving the VIR type id.
*/
static gceSTATUS
_ConvShaderTypeToVirTypeId(
    IN gcSHADER_TYPE Type,
    OUT VIR_TypeId * TypeId
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    VIR_PrimitiveTypeId primitiveId;

    if (Type < gcSHADER_TYPE_COUNT)
    {
        primitiveId = gcSLType2VIRTypeMapping[Type];
    }
    else
    {
        gcmASSERT(0);
        primitiveId = VIR_TYPE_UNKNOWN;
        status = gcvSTATUS_NOT_FOUND;
    }

    if(TypeId)
    {
        *TypeId = (VIR_TypeId)primitiveId;
    }

    return status;
}

/* lower high level type which cannot supported by HW to types supported
 * by HW, such as
 *    VIR_TYPE_FLOAT_X8 to VIR_TYPE_FLOAT_X2[2]
 *    VIR_TYPE_INTEGER_X16[16] to VIR_TYPE_INTEGER_X4[4*16]
 */
VIR_Type *
VIR_Type_LowerToMachineLevel(
    IN VIR_Shader *      Shader,
    IN VIR_Type   *      Type
    )
{
    /* */
    if (VIR_Type_isPrimitive(Type))
    {
        /* VIR_Type   * newType;*/
        if (VIR_GetTypeRows(VIR_Type_GetIndex(Type)) <= 1)
        {
            return Type;
        }
        /* create a new type with RowType[Rows] */
    }
    else
    {
        switch (VIR_Type_GetKind(Type)) {
        case VIR_TY_ARRAY:
            break;
        case VIR_TY_POINTER:
            break;
        case VIR_TY_STRUCT:
            break;
        case VIR_TY_IMAGE:
            break;
        case VIR_TY_FUNCTION:
            break;
        default:
            break;
        }
    }
    return Type;
}

static gctCHAR *
_GetBaseVariableName(
    IN gctSTRING  VariableName
    )
{
    gctSIZE_T len, i;

    gcmASSERT(VariableName);
    len = gcoOS_StrLen(VariableName, gcvNULL);

    if (len == 0) return VariableName;
    for(i = len - 1; i > 0; i--) {
       if(VariableName[i] == '.') return (VariableName + i + 1);
    }
    return VariableName;
}

static gctUINT16
_GetStartUniformIndex(
    gcSHADER Shader,
    gcUNIFORM Uniform
    )
{
    if(isUniformStruct(Uniform)) {
        if(Uniform->firstChild != -1) {
            gcUNIFORM firstChild;

            gcmVERIFY_OK(gcSHADER_GetUniform(Shader, Uniform->firstChild, &firstChild));
            return _GetStartUniformIndex(Shader, firstChild);
        }
        else return (gctUINT16) -1;
    }
    else return (gctUINT16) Uniform->index;
}

gctUINT
_gcGetTypeComponentCount(
    IN VIR_Shader *     VirShader,
    IN VIR_Type *       Type,
    IN gctBOOL          IsLogicalComponentCount);

static VSC_ErrCode
_InitializeUniformFromBuffer(
VIR_Shader * VirShader,
VIR_TypeId TypeId,
gcSL_FORMAT Format,
gctSTRING Buffer,
VIR_Uniform  *VirUniform
)
{
    VSC_ErrCode virErrCode;
    VIR_Type *type;
    VIR_ConstVal value[1];
    VIR_ConstId *initializerPtr;
    gctUINT vectorSize, i, j;
    gctUINT arrayLength = 1;
    VIR_TypeId baseTypeId;
    VIR_TypeId elemTypeId;
    gctBOOL isPacked;
    gctUINT components;
    gctSTRING ptr, curPtr;

    type = VIR_Shader_GetTypeFromId(VirShader, TypeId);
    if(VIR_Type_isArray(type)) {
        VSC_MM *memPool = &VirShader->pmp.mmWrapper;

        arrayLength = VIR_Type_GetArrayLength(type);
        initializerPtr = (VIR_ConstId *)vscMM_Alloc(memPool, sizeof(VIR_ConstId) * arrayLength);
        VIR_Uniform_SetInitializerPtr(VirUniform, initializerPtr);
    }
    else {
        initializerPtr = &VirUniform->u.initializer;
    }

    gcmASSERT(VIR_Uniform_GetOffset(VirUniform) >= 0);
    ptr = Buffer + VIR_Uniform_GetOffset(VirUniform);
    vectorSize =  _gcGetTypeComponentCount(VirShader,
                                           type,
                                           gcvTRUE);
    baseTypeId = VIR_Type_GetBaseTypeId(type);
    isPacked = VIR_TypeId_isPacked(baseTypeId);
    elemTypeId = VIR_GetTypeComponentType(baseTypeId);  /* get the element type */
    for(j = 0; j < arrayLength; j++, initializerPtr++) {
        gcoOS_ZeroMemory(value, gcmSIZEOF(VIR_ConstVal));
        curPtr = ptr;
        switch(elemTypeId) {
        case VIR_TYPE_INT8:
            if(isPacked) {
                switch(vectorSize) {
                case 1:
                    value->scalarVal.uValue = *((gctUINT8 *) curPtr);
                    curPtr++;
                    break;

                case 2:
                    value->vecVal.u32Value[0] = *((gctUINT16 *) curPtr);
                    curPtr += 2;
                    break;

                default:
                    components = (vectorSize + 3)/ 4;
                    for(i = 0; i < components; i++, curPtr += 4) {
                        value->vecVal.u32Value[i] = *((gctUINT32 *) curPtr);
                    }
                    break;
                }
            }
            else {
                for(i = 0; i < vectorSize; i++, curPtr++) {
                    value->vecVal.i32Value[i] = *((gctINT8 *) curPtr);
                }
            }
            break;

        case VIR_TYPE_UINT8:
        case VIR_TYPE_BOOLEAN:
            if(isPacked) {
                switch(vectorSize) {
                case 1:
                    value->scalarVal.uValue = *((gctUINT8 *) curPtr);
                    curPtr++;
                    break;

                case 2:
                    value->vecVal.u32Value[0] = *((gctUINT16 *) curPtr);
                    curPtr += 2;
                    break;

                default:
                    components = (vectorSize + 3)/ 4;
                    for(i = 0; i < components; i++, curPtr += 4) {
                        value->vecVal.u32Value[i] = *((gctUINT32 *) curPtr);
                    }
                    break;
                }
            }
            else {
                for(i = 0; i < vectorSize; i++, curPtr++) {
                    value->vecVal.u32Value[i] = *((gctUINT8 *) curPtr);
                }
            }
            break;

        case VIR_TYPE_INT16:
            if(isPacked) {
                if(vectorSize == 1) {
                    value->scalarVal.iValue = *((gctINT16 *) curPtr);
                    curPtr += 2;
                }
                else {
                    components = (vectorSize + 1)/ 2;
                    for(i = 0; i < components; i++, curPtr += 4) {
                        value->vecVal.u32Value[i] = *((gctUINT32 *) curPtr);
                    }
                }
            }
            else {
                for(i = 0; i < vectorSize; i++, curPtr += 2) {
                    value->vecVal.i32Value[i] = *((gctINT16 *) curPtr);
                }
            }
            break;

        case VIR_TYPE_UINT16:
            if(isPacked) {
                if(vectorSize == 1) {
                    value->scalarVal.uValue = *((gctUINT16 *) curPtr);
                    curPtr += 2;
                }
                else {
                    components = (vectorSize + 1)/ 2;
                    for(i = 0; i < components; i++, curPtr += 4) {
                        value->vecVal.u32Value[i] = *((gctUINT32 *) curPtr);
                    }
                }
            }
            else {
                for(i = 0; i < vectorSize; i++, curPtr += 2) {
                    value->vecVal.u32Value[i] = *((gctUINT16 *) curPtr);
                }
            }
            break;

        case VIR_TYPE_INT32:
            switch(Format) {
            case gcSL_INT8:
                if(isPacked) {
                    switch(vectorSize) {
                    case 1:
                        value->scalarVal.iValue = *((gctINT8 *) curPtr);
                        curPtr++;
                        break;

                    case 2:
                        value->vecVal.u32Value[0] = *((gctUINT16 *) curPtr);
                        curPtr += 2;
                        break;

                    default:
                        components = (vectorSize + 3)/ 4;
                        for(i = 0; i < components; i++, curPtr += 4) {
                            value->vecVal.u32Value[i] = *((gctUINT32 *) curPtr);
                        }
                        break;
                    }
                }
                else {
                    for(i = 0; i < vectorSize; i++, curPtr++) {
                        value->vecVal.i32Value[i] = *((gctINT8 *) curPtr);
                    }
                }
                break;

            case gcSL_INT16:
                if(isPacked) {
                    if(vectorSize == 1) {
                        value->scalarVal.iValue = *((gctINT16 *) curPtr);
                        curPtr += 2;
                    }
                    else {
                        components = (vectorSize + 1)/ 2;
                        for(i = 0; i < components; i++, curPtr += 4) {
                            value->vecVal.u32Value[i] = *((gctUINT32 *) curPtr);
                        }
                    }
                }
                else {
                    for(i = 0; i < vectorSize; i++, curPtr += 2) {
                        value->vecVal.i32Value[i] = *((gctINT16 *) curPtr);
                    }
                }
                break;

            case gcSL_INT32:
            case gcSL_FLOAT:
                for(i = 0; i < vectorSize; i++, curPtr += 4) {
                    value->vecVal.i32Value[i] = *((gctINT32 *) curPtr);
                }
                break;

            default:
                gcmASSERT(0);
                break;
            }
            break;

        case VIR_TYPE_UINT32:
            switch(Format) {
            case gcSL_UINT8:
                if(isPacked) {
                    switch(vectorSize) {
                    case 1:
                        value->scalarVal.uValue = *((gctUINT8 *) curPtr);
                        curPtr++;
                        break;

                    case 2:
                        value->vecVal.u32Value[0] = *((gctUINT16 *) curPtr);
                        curPtr += 2;
                        break;

                    default:
                        components = (vectorSize + 3)/ 4;
                        for(i = 0; i < components; i++, curPtr += 4) {
                            value->vecVal.u32Value[i] = *((gctUINT32 *) curPtr);
                        }
                        break;
                    }
                }
                else {
                    for(i = 0; i < vectorSize; i++, curPtr++) {
                        value->vecVal.u32Value[i] = *((gctUINT8 *) curPtr);
                    }
                }
                break;

            case gcSL_UINT16:
                if(isPacked) {
                    if(vectorSize == 1) {
                        value->scalarVal.uValue = *((gctUINT16 *) curPtr);
                        curPtr += 2;
                    }
                    else {
                        components = (vectorSize + 1)/ 2;
                        for(i = 0; i < components; i++, curPtr += 4) {
                            value->vecVal.u32Value[i] = *((gctUINT32 *) curPtr);
                        }
                    }
                }
                else {
                    for(i = 0; i < vectorSize; i++, curPtr += 2) {
                        value->vecVal.u32Value[i] = *((gctUINT16 *) curPtr);
                    }
                }
                break;

            case gcSL_UINT32:
            case gcSL_FLOAT:
                for(i = 0; i < vectorSize; i++, curPtr += 4) {
                    value->vecVal.u32Value[i] = *((gctUINT32 *) curPtr);
                }
                break;

            default:
                gcmASSERT(0);
                break;
            }
            break;

        case VIR_TYPE_UINT64:
            for(i = 0; i < vectorSize; i++, curPtr += 8) {
                value->vecVal.u64Value[i] = *((gctUINT64 *) curPtr);
            }
            break;

        case VIR_TYPE_INT64:
            for(i = 0; i < vectorSize; i++, curPtr += 8) {
                value->vecVal.i64Value[i] = *((gctINT64 *) curPtr);
            }
            break;

        case VIR_TYPE_FLOAT32:
            for(i = 0; i < vectorSize; i++, curPtr += 4) {
                value->vecVal.f32Value[i] = *((gctFLOAT *) curPtr);
            }
            break;

        default:
            gcmASSERT(0);
            return VSC_ERR_INVALID_DATA;
        }
        virErrCode = VIR_Shader_AddConstant(VirShader,
                                            baseTypeId,
                                            value,
                                            initializerPtr);
        if (virErrCode != VSC_ERR_NONE) return virErrCode;
        ptr += VIR_Type_GetArrayStride(type);
    }
    return VSC_ERR_NONE;
}

static gctINT
_GetTrueUniformArraySize(
    IN VIR_Shader *VirShader,
    IN gcUNIFORM  Uniform,
    IN VIR_TypeId *TypeId
    )
{
    gctINT arraySize = GetUniformArraySize(Uniform);
    gcSL_FORMAT format;

    if(!VIR_Shader_IsCL(VirShader) ||
       !isNormalType(Uniform->u.type)) return arraySize;

    format = GetUniformFormat(Uniform);
    if(isFormatSpecialType(format)) return arraySize;

    if (arraySize > 1)
    {
        gctUINT components = GetUniformVectorSize(Uniform);

        if(components == 0) components = 1;
        if(components > VIR_GetTypeComponents(*TypeId)) /* is extended vector type */
        {
            gctINT dim, index;

            index = GetUniformArrayLengthCount(Uniform);
            if(index > 0)
            {
                dim = Uniform->arrayLengthList[index - 1];
            }
            else dim = arraySize;
            *TypeId = _ConvScalarFormatToVirVectorTypeId(format, components, gcvFALSE);
            arraySize /= dim;
        }
    }
    return arraySize;
}

/* promote (u)char/(u)short component type of vector to (u)int */
static VIR_TypeId
_gcmConvConstIntType2VirConstIntType(
    VIR_TypeId    typeId)
{
    VIR_TypeId newtypeId = typeId;
    gcmASSERT(VIR_TypeId_isPrimitive(typeId)); /*const type should be primitive*/
    switch(newtypeId)
    {
        case VIR_TYPE_UINT8_X16:
        case VIR_TYPE_UINT16_X16:
            newtypeId = VIR_TYPE_UINT_X16;
            break;
        case VIR_TYPE_UINT8_X8:
        case VIR_TYPE_UINT16_X8:
            newtypeId = VIR_TYPE_UINT_X8;
            break;
        case VIR_TYPE_INT8_X8:
        case VIR_TYPE_INT16_X8:
            newtypeId = VIR_TYPE_INTEGER_X8;
            break;
        case VIR_TYPE_INT8_X16:
        case VIR_TYPE_INT16_X16:
            newtypeId = VIR_TYPE_INTEGER_X16;
            break;
        default:
            gcmASSERT(0); /*unsupport vector type*/
    }
    return newtypeId;
}

static VSC_ErrCode
_ConvShaderUniformIdx2Vir(
    IN gcSHADER Shader,
    IN gctINT UniformIndex,
    IN gctUINT16 StartUniformIndex,
    IN gctINT16 BlockIndex,
    IN VIR_Shader *VirShader,
    IN VIR_Type *StructType,
    OUT gctUINT16 *StructStartUniformIndex,
    OUT VIR_SymId *SymId
    )
{
    gceSTATUS status;
    VSC_ErrCode virErrCode;
    gcUNIFORM uniform;
    VIR_Uniform *virUniform;
    gctINT uniformIndex;
    VIR_SymbolKind symbolKind;
    VIR_UniformKind uniformKind;
    VIR_SymId symId = VIR_INVALID_ID;
    VIR_Symbol *sym;
    VIR_NameId nameId;
    VIR_TypeId typeId;
    VIR_TypeId structTypeId;
    VIR_ConstVal value[1];
    gctUINT16 currUniformIndex = (gctUINT16) - 1;
    gctUINT16 startUniformIndex = (gctUINT16) - 1;
    gctBOOL firstTime = gcvTRUE;
    gctINT  lastIndexingIndex = -1;
    gctINT  arraySize;

    gcmHEADER_ARG("Shader=0x%x UniformIndex=%d StartUniformIndex=%d VirShader=0x%x StructType=0x%x",
                  Shader, UniformIndex, StartUniformIndex, VirShader, StructType);

    gcmASSERT(Shader);
    gcmASSERT(VirShader);

    memset(value, 0, sizeof(value));

    uniformIndex = UniformIndex;
    while (uniformIndex != -1)
    {
        status = gcSHADER_GetUniform(Shader,
                                     uniformIndex,
                                     &uniform);
        if (gcmIS_ERROR(status))
        {
            /* Error. */
            gcmFOOTER();
            return VSC_ERR_INVALID_DATA;
        }

        virErrCode = VIR_Shader_AddString(VirShader,
                                          uniform->name,
                                          &nameId);
        if (virErrCode != VSC_ERR_NONE) return virErrCode;

        structTypeId = VIR_INVALID_ID;
        /* If this uniform is a struct, add a struct type. */
        if (isUniformStruct(uniform))
        {
            symbolKind = VIR_SYM_UNIFORM;

            virErrCode = VIR_Shader_AddStructType(VirShader,
                                                  gcvFALSE,
                                                  nameId,
                                                  gcvFALSE,
                                                  &typeId);
            if (virErrCode != VSC_ERR_NONE) return virErrCode;
            structTypeId = typeId;
        }
        else
        {
            virErrCode = (VSC_ErrCode)_ConvShaderTypeToVirTypeId(uniform->u.type, &typeId);

            symbolKind = (gcmType_Kind(uniform->u.type) == gceTK_SAMPLER) ? VIR_SYM_SAMPLER
                             : (gcmType_Kind(uniform->u.type) == gceTK_SAMPLER_T) ? VIR_SYM_SAMPLER_T
                             : (gcmType_Kind(uniform->u.type) == gceTK_IMAGE) ? VIR_SYM_IMAGE
                             : (gcmType_Kind(uniform->u.type) == gceTK_IMAGE_T) ? VIR_SYM_IMAGE_T
                             : VIR_SYM_UNIFORM;
            currUniformIndex = uniform->index;
            /* promote (u)char/(u)short component type of vector to (u)int
             * 1) gcsValue only has u32_v16/i32_v16 to store char/short/int vector and memory copy to VIR_VecConstVal
             * 2) const value related passes, cpf/put imm to uniform/ operate on VIR_VecConstVal->u32Value
             */
            if (VIR_GetTypeRows(typeId) > 1 &&
                (VIR_GetTypeFlag(typeId)&VIR_TYFLAG_ISINTEGER) &&
                (VIR_GetTypeSize(VIR_GetTypeComponentType(typeId)) < VIR_GetTypeSize(VIR_TYPE_INT32)))
            {
                typeId = _gcmConvConstIntType2VirConstIntType(typeId);
            }
        }
        CHECK_ERROR(virErrCode, "Failed to conv shader type to VIR type id");

        /* If this uniform is an array, update the type into an array type. */
        arraySize = _GetTrueUniformArraySize(VirShader, uniform, &typeId);
        if (isUniformArray(uniform) || arraySize > 1)
        {
            gctINT arrayStride = GetUniformArrayStride(uniform);

            if(arrayStride == -1) arrayStride = 0;
            virErrCode = VIR_Shader_AddArrayType(VirShader,
                                                 typeId,
                                                 arraySize,
                                                 arrayStride,
                                                 &typeId);
            CHECK_ERROR(virErrCode, "Failed to add VIR array type Id");
        }

        /* Set the start uniform index. */
        if (firstTime)
        {
            startUniformIndex = currUniformIndex;
            firstTime = gcvFALSE;
        }

        /* If this uniform is a member of a struct or a UBO, then treat this uniform as a field. */
        if (StructType)
        {
           gctSTRING fieldName;
           fieldName = _GetBaseVariableName(uniform->name);

           virErrCode = VIR_Shader_AddString(VirShader,
                                             fieldName,
                                             &nameId);
           if (virErrCode != VSC_ERR_NONE) return virErrCode;
           symbolKind = VIR_SYM_FIELD;
           virErrCode = VIR_Shader_AddFieldSymbol(VirShader,
                                                  nameId,
                                                  VIR_Shader_GetTypeFromId(VirShader, typeId),
                                                  StructType,
                                                  VIR_STORAGE_UNKNOWN,
                                                  &symId);
        }
        else
        {
           virErrCode = VIR_Shader_AddSymbol(VirShader,
                                             symbolKind,
                                             nameId,
                                             VIR_Shader_GetTypeFromId(VirShader, typeId),
                                             VIR_STORAGE_UNKNOWN,
                                             &symId);
        }

        /* We may add this uniform before. */
        if (virErrCode == VSC_ERR_REDEFINITION)
        {
            if (uniform->blockIndex != -1)
            {
                gcsUNIFORM_BLOCK uniformBlock;

                status = gcSHADER_GetUniformBlock(Shader,
                                                  uniform->blockIndex,
                                                  &uniformBlock);
                if (gcmIS_ERROR(status))
                {
                    virErrCode = VSC_ERR_INVALID_ARGUMENT;
                }
                else
                {
                    gctSTRING uniformName;

                    uniformName = _ConstructVariableName(uniformBlock->name,
                                                         uniform->name);
                    if (!uniformName)
                    {
                        virErrCode = VSC_ERR_INVALID_ARGUMENT;
                    }
                    else
                    {
                        virErrCode = VIR_Shader_AddString(VirShader,
                                                          uniformName,
                                                          &nameId);
                        if (virErrCode == VSC_ERR_NONE)
                        {
                            virErrCode = VIR_Shader_AddSymbol(VirShader,
                                                              symbolKind,
                                                              nameId,
                                                              VIR_Shader_GetTypeFromId(VirShader, typeId),
                                                              VIR_STORAGE_UNKNOWN,
                                                              &symId);
                        }
                        gcmOS_SAFE_FREE(gcvNULL, uniformName);
                    }
                }
            }
            else if (!StructType)
            {
                /* OCL created many same uniforms :-(*/
                virErrCode = _ResolveNameClash(VirShader,
                                               nameId,
                                               "$$",
                                               uniform->index,
                                               &nameId);
                if (virErrCode == VSC_ERR_NONE)
                {
                   virErrCode = VIR_Shader_AddSymbol(VirShader,
                                                     symbolKind,
                                                     nameId,
                                                     VIR_Shader_GetTypeFromId(VirShader, typeId),
                                                     VIR_STORAGE_UNKNOWN,
                                                     &symId);
                }
            }
        }

        CHECK_ERROR(virErrCode, "Failed to add uniform symbol");

        if (SymId)
        {
            *SymId = symId;
        }
        /* Set the sym info. */
        sym = VIR_Shader_GetSymFromId(VirShader, symId);
        VIR_Symbol_SetPrecision(sym, _gcmConvPrecision2Vir(uniform->precision));
        VIR_Symbol_SetTyQualifier(sym, _ConvTypeQualifier2Vir(GetUniformQualifier(uniform)));
        uniformKind = _ConvUniformKind2Vir(uniform);
        VIR_Symbol_SetUniformKind(sym, uniformKind);
        /* set address space based on uniform kind */
        switch (uniformKind) {
        case VIR_UNIFORM_KERNEL_ARG_LOCAL:
        case VIR_UNIFORM_LOCAL_ADDRESS_SPACE:
            VIR_Symbol_SetAddrSpace(sym, VIR_AS_LOCAL);
            break;
        case VIR_UNIFORM_KERNEL_ARG_PRIVATE:
        case VIR_UNIFORM_PRIVATE_ADDRESS_SPACE:
            VIR_Symbol_SetAddrSpace(sym, VIR_AS_PRIVATE);
            break;
        case VIR_UNIFORM_KERNEL_ARG:
        case VIR_UNIFORM_KERNEL_ARG_SAMPLER:
            VIR_Symbol_SetAddrSpace(sym, VIR_AS_GLOBAL);
            break;
        default:
        VIR_Symbol_SetAddrSpace(sym, VIR_AS_CONSTANT);
            break;
        }

        if (isUniformBlockAddress(uniform))
        {
            VIR_Symbol_SetFlag(sym, VIR_SYMUNIFORMFLAG_ALWAYS_IN_DUB);
        }
        VIR_Symbol_SetFlag(sym, _ConvUniformFlag2Vir(uniform));
        if (isUniformCompiletimeInitialized(uniform))
        {
            VIR_Symbol_SetFlag(sym, VIR_SYMUNIFORMFLAG_COMPILETIME_INITIALIZED);
        }
        else if (isUniformLoadtimeConstant(uniform))
        {
            VIR_Symbol_SetFlag(sym, VIR_SYMUNIFORMFLAG_LOADTIME_CONSTANT);
        }

        /* If this uniform is a struct, add its members. */
        if (isUniformStruct(uniform))
        {
            gctINT firstChild;

            firstChild = uniform->firstChild;
            /* If this uniform is an array struct and the array size is not 1, then we need to get the first element.*/
            if (firstChild != -1 && isUniformArray(uniform) && GetUniformArraySize(uniform) > 1)
            {
               gcUNIFORM arrayStructElem;

               gcmVERIFY_OK(gcSHADER_GetUniform(Shader, firstChild, &arrayStructElem));
               firstChild = arrayStructElem->firstChild;
            }

            gcmASSERT((gctUINT32) structTypeId != VIR_INVALID_ID);
            if (firstChild != -1)
            {
                virErrCode =_ConvShaderUniformIdx2Vir(Shader,
                                                      (gctINT)uniform->firstChild,
                                                      StartUniformIndex,
                                                      BlockIndex,
                                                      VirShader,
                                                      VIR_Shader_GetTypeFromId(VirShader, structTypeId),
                                                      &currUniformIndex,
                                                      gcvNULL);
                if (virErrCode != VSC_ERR_NONE) return virErrCode;
            }
        }

        /* Set the uniform info. */
        switch(symbolKind)
        {
        case VIR_SYM_UNIFORM:
        case VIR_SYM_SAMPLER:
        case VIR_SYM_SAMPLER_T:
        case VIR_SYM_IMAGE:
        case VIR_SYM_IMAGE_T:
            virUniform = sym->u2.uniform;
            gcmASSERT(virUniform);

            if (!isUniformStruct(uniform))
            {
                gcSHADER_GetUniformIndexingRange(Shader,
                                                 uniform->index,
                                                 -1,
                                                 (gctINT*)&lastIndexingIndex,
                                                 gcvNULL,
                                                 gcvNULL);
            }

            virUniform->sym = symId;
            VIR_Uniform_SetBlockIndex(virUniform, BlockIndex);
            if(BlockIndex != -1)
            {
                /* In recompilation, uniform already in DUBO but not so in IR. Need to move it to DUBO again */
                VIR_UBOIdList* uboList = VIR_Shader_GetUniformBlocks(VirShader);
                VIR_Symbol* uboSym = VIR_Shader_GetSymFromId(VirShader, VIR_IdList_GetId(uboList, BlockIndex));

                if(isSymUBODUBO(uboSym))
                {
                    VIR_Symbol_AddFlag(sym, VIR_SYMUNIFORMFLAG_MOVING_TO_DUBO);
                }
            }
            if (VIR_Shader_IsCL(VirShader) && Shader->currentKernelFunction &&
                uniform->index < Shader->currentKernelFunction->uniformArgumentCount)
            {
                virUniform->kernelArgIndex = uniform->index;
            }
            else
            {
                virUniform->kernelArgIndex = -1;
            }
            virUniform->gcslIndex = uniform->index;
            virUniform->lastIndexingIndex = (gctINT16)lastIndexingIndex;
            virUniform->glUniformIndex = uniform->glUniformIndex;
            if (GetUniformSamplerPhysical(uniform) != -1)
            {
                VIR_Uniform_SetOrigPhysical(virUniform, GetUniformSamplerPhysical(uniform));
            }
            else
            {
                VIR_Uniform_SetOrigPhysical(virUniform, GetUniformPhysical(uniform));
            }
            VIR_Uniform_SetPhysical(virUniform, -1);
            VIR_Uniform_SetSamplerPhysical(virUniform, -1);
            if(symbolKind == VIR_SYM_UNIFORM)
            {
                VIR_Uniform_SetIsPointer(virUniform, GetUniformIsPointer(uniform));
            }
            if (isUniformPointer(uniform))
            {
                VIR_Uniform_SetTrulyDefinedPointer(virUniform);
            }
            if (VIR_Uniform_isPointer(virUniform) || isFormatSpecialType(GetUniformFormat(uniform)))
            {
                VIR_TypeId  pointerTypeId = VIR_TYPE_UNKNOWN, baseTypeId = VIR_TYPE_UNKNOWN, derivedTypeId = VIR_TYPE_UNKNOWN;

                gctSIZE_T   nameOffset = GetUniformTypeNameOffset(uniform);

                baseTypeId = _ConvScalarFormatToVirVectorTypeId(GetFormatBasicType(GetUniformFormat(uniform)),
                                                                GetUniformVectorSize(uniform) ? GetUniformVectorSize(uniform) : 1,
                                                                gcvFALSE);

                if (nameOffset != -1)
                {
                    gctCHAR*    name = GetShaderTypeNameBuffer(Shader) + nameOffset;
                    VIR_NameId  nameId = VIR_INVALID_ID;

                    VIR_Shader_AddString(VirShader, name, &nameId);

                    switch (GetFormatSpecialType(GetUniformFormat(uniform)))
                    {
                    case gcSL_TYPEDEF:
                        VIR_Shader_AddTypeDefType(VirShader,
                                                  baseTypeId,
                                                  nameId,
                                                  VIR_TYQUAL_NONE,
                                                  VIR_AS_PRIVATE,
                                                  &baseTypeId);
                        break;

                    case gcSL_STRUCT:
                        VIR_Shader_AddStructType(VirShader,
                                                 gcvFALSE,
                                                 nameId,
                                                 gcvFALSE,
                                                 &baseTypeId);
                        break;

                    case gcSL_UNION:
                        VIR_Shader_AddStructType(VirShader,
                                                 gcvTRUE,
                                                 nameId,
                                                 gcvFALSE,
                                                 &baseTypeId);
                        break;

                    case gcSL_ENUM:
                        VIR_Shader_AddEnumType(VirShader,
                                               nameId,
                                               &baseTypeId);
                        break;

                    default:
                        break;
                    }

                    derivedTypeId = baseTypeId;
                }

                if (VIR_Uniform_isPointer(virUniform))
                {
                    VIR_Shader_AddPointerType(VirShader,
                                              baseTypeId,
                                              VIR_TYQUAL_NONE,
                                              VIR_AS_PRIVATE,
                                              &pointerTypeId);
                    derivedTypeId = pointerTypeId;
                }
                VIR_Uniform_SetDerivedType(virUniform, derivedTypeId);
            }
            else
            {
                VIR_Uniform_SetDerivedType(virUniform, VIR_TYPE_UNKNOWN);
            }
            virUniform->swizzle = uniform->swizzle;
            virUniform->address = uniform->address;
            virUniform->imageSamplerIndex = uniform->imageSamplerIndex;
            virUniform->offset = uniform->offset;

            if(gcmType_Kind(uniform->u.type) != gceTK_SAMPLER &&
               gcmType_Kind(uniform->u.type) != gceTK_IMAGE &&
               gcmType_Kind(uniform->u.type) != gceTK_IMAGE_T &&
               isUniformCompiletimeInitialized(uniform))
            {
                VIR_TypeId typeId;

                typeId = VIR_Symbol_GetTypeId(sym);
                if(uniform->offset >= 0) {
                    virErrCode = _InitializeUniformFromBuffer(VirShader,
                                                              typeId,
                                                              GetUniformFormat(uniform),
                                                              GetShaderConstantMemoryBuffer(Shader),
                                                              virUniform);
                }
                else {
                    _gcmConvConstVal2VirConstValue(&uniform->initializer, value);
                    virErrCode = VIR_Shader_AddConstant(VirShader,
                                                        typeId,
                                                        value,
                                                        &virUniform->u.initializer);
                }
                CHECK_ERROR(virErrCode, "Failed to add constant");
                if (virErrCode != VSC_ERR_NONE) return virErrCode;
            }

            {
                /* set layout info */
                VIR_LayoutQual qual = ((!isUniformStruct(uniform) && gcmType_isMatrix(uniform->u.type)) ?
                                              (uniform->isRowMajor ? VIR_LAYQUAL_ROW_MAJOR : VIR_LAYQUAL_COLUMN_MAJOR) :
                                              VIR_LAYQUAL_NONE);
                VIR_ImageFormat imageFormat = VIR_IMAGE_FORMAT_NONE;

                if (isUniformImage(uniform))
                {
                    imageFormat = _ConvImageFormat2Vir(uniform->imageFormat);
                }
                else if (isUniformSamplerBuffer(uniform))
                {
                    if (GetUniformType(uniform) == gcSHADER_SAMPLER_BUFFER)
                    {
                        imageFormat =  VIR_IMAGE_FORMAT_RGBA32F;
                    }
                    else if (GetUniformType(uniform) == gcSHADER_ISAMPLER_BUFFER)
                    {
                        imageFormat =  VIR_IMAGE_FORMAT_RGBA32I;
                    }
                    else
                    {
                        imageFormat =  VIR_IMAGE_FORMAT_RGBA32UI;
                    }
                }
                VIR_Symbol_SetLayoutQualifier(sym, qual);
                VIR_Symbol_SetBinding(sym, GetUniformBinding(uniform));
                VIR_Symbol_SetLocation(sym, GetUniformLayoutLocation(uniform));
                VIR_Symbol_SetImageFormat(sym, imageFormat);
            }
            break;

        case VIR_SYM_FIELD:
            gcmASSERT(StructType);

            virErrCode = VIR_Type_AddField(VirShader,
                                           StructType,
                                           symId);
            if (virErrCode != VSC_ERR_NONE) return virErrCode;
            {
                /* block member can override row_major/column_major. */
                VIR_LayoutQual qual = (!isUniformStruct(uniform)) &&
                                      gcmType_isMatrix(uniform->u.type) &&
                                      uniform->isRowMajor ? VIR_LAYQUAL_ROW_MAJOR : VIR_LAYQUAL_NONE;

                /* Inherit layout from UBO. */
                if (GetUniformBlockID(uniform) != -1)
                {
                    gcsUNIFORM_BLOCK uniformBlock;
                    uniformBlock = Shader->uniformBlocks[GetUniformBlockID(uniform)];
                    qual |= _gcmConvMemoryLayout2Vir(GetUBMemoryLayout(uniformBlock));
                }
                VIR_Symbol_SetLayoutQualifier(sym, qual);
                if (isUniformImage(uniform))
                {
                    VIR_Symbol_SetImageFormat(sym, _ConvImageFormat2Vir(uniform->imageFormat));
                }
            }
            VIR_FieldInfo_SetTempRegOrUniformOffset(VIR_Symbol_GetFieldInfo(sym), currUniformIndex - StartUniformIndex);
            break;

        default:
            gcmASSERT(0);
            break;
        }

        if (StructType)
        {
            uniformIndex = uniform->nextSibling;
        }
        else break;
    }

    if (StructStartUniformIndex)
    {
        *StructStartUniformIndex = startUniformIndex;
    }

    gcmFOOTER_ARG("*SymId=%d", symId);
    return VSC_ERR_NONE;
}

static gctSTRING
_ConstructVariableName(
    IN gctSTRING Prefix,
    IN gctSTRING VariableName
    )
{
    gceSTATUS status;
    gctSTRING name;
    gctSIZE_T length;
    gctUINT offset = 0;
    gctPOINTER pointer;

    length = gcoOS_StrLen(Prefix, gcvNULL) +
             gcoOS_StrLen(VariableName, gcvNULL) + 5;

    status = gcoOS_Allocate(gcvNULL,
                            length,
                            &pointer);
    if (gcmIS_ERROR(status))
    {
        return gcvNULL;
    }
    name = pointer;

    gcmVERIFY_OK(gcoOS_PrintStrSafe(name,
                                    length,
                                    &offset,
                                    "%s::%s",
                                    Prefix,
                                    VariableName));
    return name;
}

static VSC_ErrCode
_ResolveNameClash(
    IN VIR_Shader * VirShader,
    IN VIR_NameId   OrgNameId,
    IN gctSTRING    Separator,
    IN gctUINT      Resolution,
    OUT VIR_NameId *NewNameId
    )
{
    gceSTATUS status;
    VSC_ErrCode virErrCode;
    gctSTRING symName;
    gctSTRING redefinedName;
    VIR_NameId nameId;
    gctSIZE_T length;
    gctUINT offset = 0;
    gctPOINTER pointer;

    symName = VIR_Shader_GetStringFromId(VirShader, OrgNameId);
    length = gcoOS_StrLen(symName, gcvNULL) +
             gcoOS_StrLen(Separator, gcvNULL) + 16;

    status = gcoOS_Allocate(gcvNULL,
                            length,
                            &pointer);
    if (gcmIS_ERROR(status)) {
         CHECK_ERROR(VSC_ERR_OUT_OF_MEMORY,
                     "Ran out of memory during duplicate symbol name \"%s\" construction",
                     symName);
        return VSC_ERR_OUT_OF_MEMORY;
    }
    redefinedName = pointer;

    gcmVERIFY_OK(gcoOS_PrintStrSafe(redefinedName,
                                    length,
                                    &offset,
                                    "%s%s%u",
                                    symName,
                                    Separator,
                                    Resolution));
    virErrCode = VIR_Shader_AddString(VirShader,
                                      redefinedName,
                                      &nameId);
    gcmOS_SAFE_FREE(gcvNULL, redefinedName);
    if(virErrCode != VSC_ERR_NONE) return virErrCode;
    *NewNameId = nameId;
    return VSC_ERR_NONE;
}

static gctINT
_GetTrueVariableArraySize(
    IN VIR_Shader *VirShader,
    IN gcVARIABLE Variable,
    IN VIR_TypeId *TypeId
    )
{
    gctINT arraySize = GetVariableArraySize(Variable);

    if(!VIR_Shader_IsCL(VirShader)) return arraySize;
    if (arraySize != -1)
    {
        if (GetVariableIsExtendedVector(Variable))
        {
            VIR_TypeId typeId;
            VIR_PrimitiveTypeId  format;
            gctUINT components;
            gctINT dim;
            gctINT index;

            typeId = *TypeId;
            format = VIR_GetTypeComponentType(typeId);
            components = VIR_GetTypeComponents(typeId);

            index = GetVariableArrayLengthCount(Variable);
            if(index > 0)
            {
                dim = Variable->arrayLengthList[index - 1];
            }
            else dim = arraySize;
            components *= dim;
            typeId = VIR_TypeId_ComposeNonOpaqueType(format,
                                                     components,
                                                     1);
            *TypeId = typeId;
            arraySize /= dim;
        }
    }
    return arraySize;
}

static VSC_ErrCode
_ConvShaderVariable2Vir(
    IN gcSHADER Shader,
    IN gcVARIABLE Variable,
    IN conv2VirsVirRegMap *VirRegMapArr,
    IN gctUINT StartRegIndex,
    IN VIR_Shader *VirShader,
    IN VIR_Type *StructType,
    IN VIR_Symbol *StructVariable,
    OUT gctUINT *StructStartRegIndex,
    OUT gctUINT *StructEndRegIndex
    )
{
    gceSTATUS status;
    VSC_ErrCode virErrCode = VSC_ERR_NONE;
    gcVARIABLE variable;
    VIR_SymbolKind symbolKind;
    VIR_StorageClass storageClass = VIR_STORAGE_UNKNOWN;
    VIR_SymId symId = VIR_INVALID_ID;
    VIR_Symbol *sym;
    VIR_NameId nameId;
    VIR_TypeId typeId;
    VIR_TypeId structTypeId;
    gctUINT currStartRegIndex = (gctUINT32) - 1;
    gctUINT currEndRegIndex = (gctUINT32) - 1;
    gctUINT startRegIndex = (gctUINT32) - 1;
    VIR_Precision precision;
    gctBOOL firstTime = gcvTRUE;

    gcmASSERT(Shader);
    gcmASSERT(VirShader);

    variable = Variable;
    while(variable)
    {
        gctUINT startIndex, endIndex;
        gctINT arraySize;

        gcSHADER_GetVariableIndexingRange(Shader, variable, gcvFALSE,
                                            &startIndex, &endIndex);

        structTypeId = VIR_INVALID_ID;
        status = _ConvBuiltinNameKindToVirNameId(variable->nameLength,
                                                 &nameId);

        if (status == gcvSTATUS_NOT_FOUND) {
            gctSTRING variableName = variable->name;
            gctBOOL nameConstructed = gcvFALSE;

            if(VirRegMapArr[StartRegIndex].inFunction) {
                gctSTRING functionName;

                functionName = VIR_Shader_GetSymNameStringFromId(VirShader, VirRegMapArr[StartRegIndex].inFunction->funcSym);
                variableName = _ConstructVariableName(functionName,
                                                      variableName);
                if(!variableName) {
                    ON_ERROR(VSC_ERR_INVALID_ARGUMENT,
                                "Failed to construct variable name \"%s\" in function \"%s\"",
                                variableName, functionName);
                }
                nameConstructed = gcvTRUE;
            }

            virErrCode = VIR_Shader_AddString(VirShader,
                                              variableName,
                                              &nameId);
            if(nameConstructed) {
                gcmOS_SAFE_FREE(gcvNULL, variableName);
            }
            ON_ERROR(virErrCode, "Failed to add string %s", variableName);
        }
        else {
            /* the name is already in string table, */
            storageClass = _gcmGetVirBuiltinStorageClass(Shader->type,
                                                         variable->nameLength);
        }

        if(isVariableStruct(variable)) {
            virErrCode = VIR_Shader_AddStructType(VirShader,
                                                  gcvFALSE,
                                                  nameId,
                                                  gcvFALSE,
                                                  &typeId);
            structTypeId = typeId;
        }
        else {
            currStartRegIndex = variable->tempIndex;

            virErrCode = _ConvShaderTypeToVirTypeId(variable->u.type,
                                                    &typeId);
        }
        symbolKind = VIR_SYM_VARIABLE;
        ON_ERROR(virErrCode, "Failed to conv shader type to VIR type id");

        arraySize = _GetTrueVariableArraySize(VirShader, variable, &typeId);
        if(GetVariableArrayLengthCount(variable) > 0) {
            if (arraySize != -1) {
                virErrCode = VIR_Shader_AddArrayType(VirShader,
                                                     typeId,
                                                     arraySize,
                                                     0,
                                                     &typeId);
                ON_ERROR(virErrCode, "Failed to add VIR array type Id %d", typeId);
            }
            else {
                gcmASSERT(0);
            }
        }

        if(storageClass == VIR_STORAGE_UNKNOWN) {
            storageClass = GetVariableIsOtput(variable) ? VIR_STORAGE_OUTPUT
                             : (GetVariableIsLocal(variable) ? VIR_STORAGE_LOCAL
                                 : VIR_STORAGE_GLOBAL);
        }

        if (GetVariableIsPointer(variable))
        {
            VIR_Shader_AddPointerType(VirShader,
                                      typeId,
                                      VIR_TYQUAL_NONE,
                                      VIR_AS_PRIVATE,
                                      &typeId);
        }
        if(StructType) {
           gctSTRING fieldName;
           fieldName = _GetBaseVariableName(variable->name);

           virErrCode = VIR_Shader_AddString(VirShader,
                                             fieldName,
                                             &nameId);
           ON_ERROR(virErrCode, "Failed to add string %s", fieldName);

           symbolKind = VIR_SYM_FIELD;
           virErrCode = VIR_Shader_AddFieldSymbol(VirShader,
                                                  nameId,
                                                  VIR_Shader_GetTypeFromId(VirShader, typeId),
                                                  StructType,
                                                  storageClass,
                                                  &symId);
        }
        else {
            if (VIR_Shader_IsCL(VirShader) && GetShaderMaxLocalTempRegCount(Shader) > 0 &&
                variable->tempIndex < _gcdOCL_MaxLocalTempRegs)
            {
                /* For LocalMemoryAddressReg, we only need to add the baseAddress, no need to add the __local variables. */
                if (GetVariableTempIndex(variable) == _gcdOCL_LocalMemoryAddressRegIndex)
                {
                    gctSTRING name = VIR_Shader_GetStringFromId(VirShader, nameId);

                    if (!gcmIS_SUCCESS(gcoOS_StrNCmp(name, _sldLocalMemoryAddressName, 22)))
                    {
                        return VSC_ERR_NONE;
                    }
                }
                /* OCL has predefined variables which may be defined multiple times,
                 * for each tempIndex, just find the first one and use it for all
                 */
                virErrCode = VIR_Shader_FindSymbol(VirShader,
                                                 symbolKind,
                                                 nameId,
                                                 VIR_Shader_GetTypeFromId(VirShader, typeId),
                                                 storageClass,
                                                 &symId);
                if (virErrCode == VSC_ERR_NOT_FOUND)
                {
                    /* no symbol added yet, add it now. */
                    virErrCode = VIR_Shader_AddSymbol(VirShader,
                                                     symbolKind,
                                                     nameId,
                                                     VIR_Shader_GetTypeFromId(VirShader, typeId),
                                                     storageClass,
                                                     &symId);

                }
                else
                {
                    /* found previous defined symbol, reuse it */
                    sym = VIR_Shader_GetSymFromId(VirShader, symId);

                    *StructStartRegIndex = VIR_Symbol_GetVariableVregIndex(sym);
                    *StructEndRegIndex = VIR_Symbol_GetIndexRange(sym);

                    return VSC_ERR_NONE;
                }
            }
            else
            {
                virErrCode = VIR_Shader_AddSymbol(VirShader,
                                                 symbolKind,
                                                 nameId,
                                                 VIR_Shader_GetTypeFromId(VirShader, typeId),
                                                 storageClass,
                                                 &symId);
            }
        }

        if(virErrCode == VSC_ERR_REDEFINITION) {
            if (VIR_Shader_IsCL(VirShader) &&
                variable->tempIndex == _gcdOCL_PrivateMemoryAddressRegIndex)
            {
                gcmASSERT(variable->offset != -1);
                    virErrCode = _ResolveNameClash(VirShader,
                                                nameId,
                                                "@@",
                                                variable->offset,
                                                &nameId);
            }
            else
            {
                virErrCode = _ResolveNameClash(VirShader,
                                               nameId,
                                               "$$",
                                               variable->tempIndex,
                                               &nameId);
            }
            ON_ERROR(virErrCode, "Failed to add variable symbol: ");
            if(StructType) {
                gcmASSERT(symbolKind == VIR_SYM_FIELD);
                virErrCode = VIR_Shader_AddFieldSymbol(VirShader,
                                                       nameId,
                                                       VIR_Shader_GetTypeFromId(VirShader, typeId),
                                                       StructType,
                                                       storageClass,
                                                       &symId);
            }
            else {
                virErrCode = VIR_Shader_AddSymbol(VirShader,
                                                  symbolKind,
                                                  nameId,
                                                  VIR_Shader_GetTypeFromId(VirShader, typeId),
                                                  storageClass,
                                                  &symId);
            }
        }

        ON_ERROR(virErrCode, "Failed to add variable symbol");

        sym = VIR_Shader_GetSymFromId(VirShader, symId);
        if(StructVariable)
        {
            VIR_Symbol_SetParentId(sym, VIR_Symbol_GetIndex(StructVariable));
        }

        if (symbolKind == VIR_SYM_VARIABLE)
        {
            /* set layout info */
            VIR_Symbol_SetLayoutQualifier(sym, VIR_LAYQUAL_NONE);
            VIR_Symbol_SetLocation(sym, 0);
        }

        precision = _gcmConvPrecision2Vir(variable->precision);
        VIR_Symbol_SetPrecision(sym, precision);
        VIR_Symbol_SetAddrSpace(sym, _ConvAddrSpaceQualifier2Vir(variable->qualifier));
        VIR_Symbol_SetTyQualifier(sym, _ConvTypeQualifier2Vir(variable->qualifier));

        if (gcdTARGETHOST_BIGENDIAN && IsVariableHostEndian(variable))
        {
            VIR_Symbol_SetBigEndian(sym, gcvTRUE);
        }
        if (IsVariablePrecise(variable))
        {
            VIR_Symbol_SetFlag(sym, VIR_SYMFLAG_PRECISE);
        }
        if(isVariableStruct(variable))
        {
            gcVARIABLE firstChild;

            firstChild = gcvNULL;
            if(variable->firstChild != -1)
            {
                if(GetVariableArraySize(variable) > 1)
                {
                    gcVARIABLE arrayStructElem;
                    gcmVERIFY_OK(gcSHADER_GetVariable(Shader, variable->firstChild, &arrayStructElem));

                    while (GetVariableArraySize(arrayStructElem) > 1)
                    {
                        gcmVERIFY_OK(gcSHADER_GetVariable(Shader, arrayStructElem->firstChild, &arrayStructElem));
                    }

                    if(arrayStructElem->firstChild != -1) {
                        gcmVERIFY_OK(gcSHADER_GetVariable(Shader, arrayStructElem->firstChild, &firstChild));
                    }
                }
                else
                {
                    gcmVERIFY_OK(gcSHADER_GetVariable(Shader, variable->firstChild, &firstChild));
                }
            }

            if(firstChild)
            {
                gctINT i;
                gctUINT j;
                VIR_SymId virRegId;
                VIR_Symbol *fromVirReg, *toVirReg;
                gctUINT endRegIndex;

                gcmASSERT((gctUINT32) structTypeId != VIR_INVALID_ID);
                virErrCode =_ConvShaderVariable2Vir(Shader,
                                                    firstChild,
                                                    VirRegMapArr,
                                                    StartRegIndex,
                                                    VirShader,
                                                    VIR_Shader_GetTypeFromId(VirShader, structTypeId),
                                                    sym,
                                                    &currStartRegIndex,
                                                    &currEndRegIndex);
                if(virErrCode != VSC_ERR_NONE) return virErrCode;

                endRegIndex = currEndRegIndex;
                for(i = 1; i < GetVariableKnownArraySize(variable); i++)
                {
                    for(j = currStartRegIndex; j < endRegIndex; j++, currEndRegIndex++)
                    {
                        gcmASSERT(VirRegMapArr[currEndRegIndex].virRegId == VIR_INVALID_ID);
                        fromVirReg= VIR_Shader_GetSymFromId(VirShader, VirRegMapArr[j].virRegId);
                        virErrCode = VIR_Shader_AddSymbol(VirShader,
                                                          VIR_SYM_VIRREG,
                                                          currEndRegIndex,
                                                          VIR_Symbol_GetType(fromVirReg),
                                                          VIR_Symbol_GetStorageClass(fromVirReg),
                                                          &virRegId);
                        ON_ERROR(virErrCode, "Failed to add virtual register symbol for array");

                        VirRegMapArr[currEndRegIndex].virRegId = virRegId;
                        VirRegMapArr[currEndRegIndex].components = VirRegMapArr[j].components;
                        VirRegMapArr[currEndRegIndex].packed = VirRegMapArr[j].packed;
                        toVirReg = VIR_Shader_GetSymFromId(VirShader, virRegId);
                        VIR_Symbol_SetVregVariable(toVirReg, VIR_Symbol_GetVregVariable(fromVirReg));
                        VIR_Symbol_SetPrecision(toVirReg, VIR_Symbol_GetPrecision(fromVirReg));
                    }
                }
            }
        }
        else if(!isVariableBlockMember(variable))   /* storage block member has no coresponding temp register */
        {

            gcSL_FORMAT regFormat;
            gctUINT32 components = 0, rows = 0;
            VIR_SymId virRegId;
            VIR_Symbol *virRegSym;
            gctUINT32 j;

            gcTYPE_GetTypeInfo(variable->u.type, &components, &rows, 0);
            rows *= GetVariableKnownArraySize(variable);
            regFormat = gcGetFormatFromType(variable->u.type);
            for(j = 0, currEndRegIndex = currStartRegIndex; j < rows; j++, currEndRegIndex++)
            {
               virRegId = _GetVirRegId(VirShader,
                                       VirRegMapArr,
                                       currEndRegIndex,
                                       regFormat,
                                       gcvFALSE,
                                       components,
                                       gcTYPE_IsTypePacked(variable->u.type),
                                       precision);
                if(virRegId == VIR_INVALID_ID)
                {
                    return VSC_ERR_INVALID_ARGUMENT;
                }
                gcmASSERT(VirRegMapArr[currEndRegIndex].virRegId != VIR_INVALID_ID);

                virRegSym = VIR_Shader_GetSymFromId(VirShader, virRegId);
                VIR_Symbol_SetVregVariable(virRegSym, sym);
                VIR_Symbol_SetOffsetInVar(virRegSym, j);
                VIR_Symbol_SetPrecision(virRegSym, precision);
                VIR_Symbol_SetIndexRange(virRegSym, endIndex);
            }
        }

        if(symbolKind == VIR_SYM_FIELD)
        {
            gcmASSERT(StructType);
            virErrCode = VIR_Type_AddField(VirShader,
                                           StructType,
                                           symId);
            if(virErrCode != VSC_ERR_NONE) return virErrCode;
            gcmASSERT(sym->u2.fieldInfo);
            VIR_FieldInfo_SetTempRegOrUniformOffset(VIR_Symbol_GetFieldInfo(sym), currStartRegIndex - StartRegIndex);
        }
        else
        {
            VIR_Symbol_SetVariableVregIndex(sym, currStartRegIndex);
            VIR_Symbol_SetIndexRange(sym, endIndex);
        }

        if(firstTime)
        {
            startRegIndex = currStartRegIndex;
            firstTime = gcvFALSE;
        }
        if(variable->nextSibling != -1) {
            gcmVERIFY_OK(gcSHADER_GetVariable(Shader, variable->nextSibling, &variable));
        }
        else break;
    }

    *StructStartRegIndex = startRegIndex;
    *StructEndRegIndex = currEndRegIndex;

    return VSC_ERR_NONE;

OnError:
    return virErrCode;
}

static VSC_ErrCode
_ConvShaderLocalVariable2Vir(
    IN gctUINT32            FunctionLocalVariableCount,
    IN gcVARIABLE *         FunctionLocalVariables,
    IN gcVARIABLE           LocalVariable,
    IN conv2VirsVirRegMap * VirRegMapArr,
    IN gctUINT32            StartRegIndex,
    IN VIR_Shader *         VirShader,
    IN VIR_Function *       VirFunction,
    IN VIR_Type *           StructType,
    IN VIR_Symbol *         StructVariable,
    OUT gctUINT32 *         StructStartRegIndex,
    OUT gctUINT32 *         StructEndRegIndex
    )
{
    VSC_ErrCode virErrCode;
    gcVARIABLE localVariable;
    VIR_SymId symId = VIR_INVALID_ID;
    VIR_NameId nameId = VIR_INVALID_ID;
    VIR_Symbol *sym;
    VIR_TypeId typeId;
    VIR_TypeId structTypeId;
    gctUINT32 currStartRegIndex = (gctUINT32) - 1;
    gctUINT32 currEndRegIndex = (gctUINT32) - 1;
    gctUINT32 startRegIndex = (gctUINT32) - 1;
    gctSTRING localVariableName;
    VIR_Precision precision;
    gctBOOL firstTime = gcvTRUE;
    gctINT  arraySize;

    gcmHEADER_ARG("LocalVariable=0x%x StartRegIndex=%u VirShader=0x%x "
                  "VirFunction=0x%x StructType=0x%x",
                  LocalVariable, StartRegIndex, VirShader, VirFunction, StructType);

    gcmASSERT(FunctionLocalVariables);
    gcmASSERT(VirShader);
    gcmASSERT(VirFunction);

    localVariable = LocalVariable;
    while(localVariable)
    {
        structTypeId = VIR_INVALID_ID;
        localVariableName = _gcmGetBuiltinNameString(localVariable->nameLength);

        if (localVariableName == gcvNULL) {
            gctSTRING functionName;

            functionName = VIR_Shader_GetSymNameStringFromId(VirShader, VirFunction->funcSym);
            localVariableName = _ConstructVariableName(functionName,
                                                       localVariable->name);
            if(!localVariableName) {
                CHECK_ERROR(VSC_ERR_INVALID_ARGUMENT,
                            "Failed to construct local variable name \"%s\" in function \"%s\"",
                            localVariableName, functionName);
                return VSC_ERR_INVALID_ARGUMENT;
            }

            virErrCode = VIR_Shader_AddString(VirShader,
                                              localVariableName,
                                              &nameId);
            gcmOS_SAFE_FREE(gcvNULL, localVariableName);
            if(virErrCode != VSC_ERR_NONE) {
                return virErrCode;
            }
        }
        else {
            gcmASSERT(StructType == gcvNULL);
            return VSC_ERR_INVALID_ARGUMENT;
        }

        if(isVariableStruct(localVariable)) {
            virErrCode = VIR_Shader_AddStructType(VirShader,
                                                  gcvFALSE,
                                                  nameId,
                                                  gcvFALSE,
                                                  &typeId);
            structTypeId = typeId;
        }
        else {
            currStartRegIndex = localVariable->tempIndex;

            virErrCode = _ConvShaderTypeToVirTypeId(localVariable->u.type,
                                                    &typeId);
        }

        CHECK_ERROR(virErrCode, "Failed to conv shader type to VIR type id");
        if(virErrCode != VSC_ERR_NONE) return virErrCode;

        arraySize = _GetTrueVariableArraySize(VirShader, localVariable, &typeId);
        if(GetVariableArrayLengthCount(localVariable) > 0) {
            virErrCode = VIR_Shader_AddArrayType(VirShader,
                                                 typeId,
                                                 arraySize,
                                                 0,
                                                 &typeId);
            CHECK_ERROR(virErrCode, "Failed to add VIR array type Id");
            if(virErrCode != VSC_ERR_NONE) return virErrCode;
        }

        if(StructType) {
            gctSTRING fieldName;
            fieldName = _GetBaseVariableName(localVariable->name);
            virErrCode = VIR_Shader_AddString(VirShader,
                                              fieldName,
                                              &nameId);
            if(virErrCode != VSC_ERR_NONE) return virErrCode;
            virErrCode = VIR_Shader_AddFieldSymbol(VirShader,
                                                   nameId,
                                                   VIR_Shader_GetTypeFromId(VirShader, typeId),
                                                   StructType,
                                                   VIR_STORAGE_LOCAL,
                                                   &symId);
        }
        else {
            virErrCode = VIR_Function_AddLocalVar(VirFunction,
                                                  VIR_Shader_GetStringFromId(VirShader, nameId),
                                                  typeId,
                                                  &symId);
            if(virErrCode != VSC_ERR_NONE &&
               virErrCode == VSC_ERR_REDEFINITION) {
                virErrCode = _ResolveNameClash(VirShader,
                                               nameId,
                                               "$$L_",
                                               localVariable->tempIndex,
                                               &nameId);
                if(virErrCode != VSC_ERR_NONE) return virErrCode;

                virErrCode = VIR_Function_AddLocalVar(VirFunction,
                                                      VIR_Shader_GetStringFromId(VirShader, nameId),
                                                      typeId,
                                                      &symId);
            }
        }

        CHECK_ERROR(virErrCode, "Failed to add local variable symbol");
        if(virErrCode != VSC_ERR_NONE) return virErrCode;

        sym = VIR_Shader_GetSymFromId(VirShader, symId);

        precision = _gcmConvPrecision2Vir(localVariable->precision);
        VIR_Symbol_SetPrecision(sym, precision);
        VIR_Symbol_SetTyQualifier(sym, _ConvTypeQualifier2Vir(localVariable->qualifier));
        if(IsVariablePrecise(localVariable))
        {
            VIR_Symbol_SetFlag(sym, VIR_SYMFLAG_PRECISE);
        }

        if(isVariableStruct(localVariable)) {
            gcVARIABLE firstChild;

            firstChild = gcvNULL;
            if(localVariable->firstChild != -1) {
                gcmASSERT((gctUINT16) localVariable->firstChild < FunctionLocalVariableCount);
                if(GetVariableArraySize(localVariable) > 1) {
                    gcVARIABLE arrayStructElem;
                    arrayStructElem = FunctionLocalVariables[localVariable->firstChild];
                    if(arrayStructElem->firstChild != -1) {
                        gcmASSERT((gctUINT16) arrayStructElem->firstChild < FunctionLocalVariableCount);
                        firstChild = FunctionLocalVariables[arrayStructElem->firstChild];
                    }
                }
                else {
                    firstChild = FunctionLocalVariables[localVariable->firstChild];
                }
            }

            if(firstChild) {
                gctINT i;
                gctUINT j;
                VIR_SymId virRegId;
                VIR_Symbol *fromVirReg, *toVirReg;
                gctUINT endRegIndex;

                gcmASSERT((gctUINT32) structTypeId != VIR_INVALID_ID);
                virErrCode =_ConvShaderLocalVariable2Vir(FunctionLocalVariableCount,
                                                         FunctionLocalVariables,
                                                         firstChild,
                                                         VirRegMapArr,
                                                         StartRegIndex,
                                                         VirShader,
                                                         VirFunction,
                                                         VIR_Shader_GetTypeFromId(VirShader, structTypeId),
                                                         StructVariable ? StructVariable : sym,
                                                         &currStartRegIndex,
                                                         &currEndRegIndex);
                if(virErrCode != VSC_ERR_NONE) return virErrCode;

                endRegIndex = currEndRegIndex;
                for(i = 1; i < GetVariableArraySize(localVariable); i++) {
                    for(j = currStartRegIndex; j < endRegIndex; j++, currEndRegIndex++) {
                        gcmASSERT(VirRegMapArr[currEndRegIndex].virRegId == VIR_INVALID_ID);
                        fromVirReg= VIR_Shader_GetSymFromId(VirShader, VirRegMapArr[j].virRegId);
                        virErrCode = VIR_Shader_AddSymbol(VirShader,
                                                          VIR_SYM_VIRREG,
                                                          currEndRegIndex,
                                                          VIR_Symbol_GetType(fromVirReg),
                                                          VIR_Symbol_GetStorageClass(fromVirReg),
                                                          &virRegId);
                        CHECK_ERROR(virErrCode, "Failed to add virtual register symbol for array");

                        if(virErrCode == VSC_ERR_NONE) {
                            VirRegMapArr[currEndRegIndex].virRegId = virRegId;
                            VirRegMapArr[currEndRegIndex].components = VirRegMapArr[j].components;
                            VirRegMapArr[currEndRegIndex].packed = VirRegMapArr[j].packed;
                            toVirReg = VIR_Shader_GetSymFromId(VirShader, virRegId);
                            VIR_Symbol_SetVregVariable(toVirReg, VIR_Symbol_GetVregVariable(fromVirReg));
                        }
                    }
                }
            }
        }
        else {
            gcSL_FORMAT regFormat;
            gctUINT32 components = 0, rows = 0;
            VIR_SymId virRegId;
            VIR_Symbol *virRegSym;
            gctUINT32 j;

            gcTYPE_GetTypeInfo(localVariable->u.type, &components, &rows, 0);
            rows *= GetVariableKnownArraySize(localVariable);
            regFormat = gcGetFormatFromType(localVariable->u.type);
            for(j = 0, currEndRegIndex = currStartRegIndex; j < rows; j++, currEndRegIndex++) {
               virRegId = _GetVirRegId(VirShader,
                                       VirRegMapArr,
                                       currEndRegIndex,
                                       regFormat,
                                       gcvFALSE,
                                       components,
                                       gcTYPE_IsTypePacked(localVariable->u.type),
                                       precision);
                if(virRegId == VIR_INVALID_ID) {
                    return VSC_ERR_INVALID_ARGUMENT;
                }
                gcmASSERT(VirRegMapArr[currEndRegIndex].virRegId != VIR_INVALID_ID);

                virRegSym = VIR_Shader_GetSymFromId(VirShader, virRegId);
                VIR_Symbol_SetVregVariable(virRegSym, StructVariable ? StructVariable : sym);
                VIR_Symbol_SetPrecision(virRegSym, precision);
            }
        }


        if(StructType) {
            gcmASSERT(sym->u2.fieldInfo);
            VIR_FieldInfo_SetTempRegOrUniformOffset(VIR_Symbol_GetFieldInfo(sym), currStartRegIndex - StartRegIndex);

            virErrCode = VIR_Type_AddField(VirShader,
                                           StructType,
                                           symId);
            if(virErrCode != VSC_ERR_NONE) return virErrCode;

            gcmASSERT(sym->u2.fieldInfo);
        }
        else {
            sym->u2.tempIndex = currStartRegIndex;
        }

        if(firstTime) {
            startRegIndex = currStartRegIndex;
            firstTime = gcvFALSE;
        }

        if(localVariable->nextSibling != -1) {
            gcmASSERT((gctUINT16) localVariable->nextSibling < FunctionLocalVariableCount);
            localVariable = FunctionLocalVariables[localVariable->nextSibling];
        }
        else break;
    }

    *StructStartRegIndex = startRegIndex;
    *StructEndRegIndex = currEndRegIndex;

    gcmFOOTER_ARG("*SymId=%d", symId);

    return VSC_ERR_NONE;
}

static gctUINT32
_GetStartRegIndex(
    gcSHADER Shader,
    gcVARIABLE Variable
    )
{
    if(isVariableStruct(Variable)) {
        if(Variable->firstChild != -1) {
            gcVARIABLE firstChild;

            gcmVERIFY_OK(gcSHADER_GetVariable(Shader, Variable->firstChild, &firstChild));
            return _GetStartRegIndex(Shader, firstChild);
        }
        else {
            return (gctUINT32) -1;
        }
    }
    else return Variable->tempIndex;
}

static gctUINT32
_GetLocalStartRegIndex(
    IN gctUINT32            FunctionLocalVariableCount,
    IN gcVARIABLE *         FunctionLocalVariables,
    IN gcVARIABLE           LocalVariable
    )
{
    if(isVariableStruct(LocalVariable)) {
        if(LocalVariable->firstChild != -1) {
            gcVARIABLE firstChild;

            gcmASSERT((gctUINT16) LocalVariable->firstChild < FunctionLocalVariableCount);
            firstChild = FunctionLocalVariables[LocalVariable->firstChild];

            return _GetLocalStartRegIndex(FunctionLocalVariableCount, FunctionLocalVariables, firstChild);
        }
        else return (gctUINT32) -1;
    }
    else return LocalVariable->tempIndex;
}

#define _gcmConvParamQualifier2Vir(Qualifier) \
    ((Qualifier) == gcvFUNCTION_INPUT ? VIR_STORAGE_INPARM \
       : ((Qualifier) == gcvFUNCTION_OUTPUT ? VIR_STORAGE_OUTPARM \
          : VIR_STORAGE_INOUTPARM))

typedef struct _conv2VirsFunctionFlagMap
{
    VIR_FunctionFlag virFunctionFlag;
    gceFUNCTION_FLAG functionFlag;
}
conv2VirsFunctionFlagMap;

conv2VirsFunctionFlagMap virFunctionFlagMap[] =
{
  {VIR_FUNCFLAG_NOATTR, gcvFUNC_NOATTR                  }, /*  0x00, */
  {VIR_FUNCFLAG_INTRINSICS, gcvFUNC_INTRINSICS              }, /*  0x01, Function is openCL/OpenGL builtin function */
  {VIR_FUNCFLAG_ALWAYSINLINE, gcvFUNC_ALWAYSINLINE            }, /*  0x02, Always inline */
  {VIR_FUNCFLAG_NOINLINE, gcvFUNC_NOINLINE                }, /*  0x04, Neve inline */
  {VIR_FUNCFLAG_INLINEHINT, gcvFUNC_INLINEHINT              }, /*  0x08, Inline is desirable */
  {VIR_FUNCFLAG_READNONE, gcvFUNC_READNONE                }, /*  0x10, Function does not access memory */
  {VIR_FUNCFLAG_READONLY, gcvFUNC_READONLY                }, /*  0x20, Function only reads from memory */
  {VIR_FUNCFLAG_STRUCTRET, gcvFUNC_STRUCTRET               }, /*  0x40, Hidden pointer to structure to return */
  {VIR_FUNCFLAG_NORETURN, gcvFUNC_NORETURN                }, /*  0x80, Function is not returning */
  {VIR_FUNCFLAG_INREG, gcvFUNC_INREG                   }, /*  0x100, Force argument to be passed in register */
  {VIR_FUNCFLAG_BYVAL, gcvFUNC_BYVAL                   }, /*  0x200, Pass structure by value */
  {VIR_FUNCFLAG_STATIC, gcvFUNC_STATIC                  }, /*  0x400, OPENCL static function */
  {VIR_FUNCFLAG_EXTERN, gcvFUNC_EXTERN                  }, /*  0x800, OPENCL extern function with no body */
  {VIR_FUNCFLAG_NAME_MANGLED, gcvFUNC_NAME_MANGLED            }, /*  0x1000, name mangled */
  {VIR_FUNCFLAG_RECOMPILER, gcvFUNC_RECOMPILER              }, /*  0x2000, A recompile function */
  {VIR_FUNCFLAG_RECOMPILER_STUB, gcvFUNC_RECOMPILER_STUB         }, /*  0x4000, The function to stub a recompile function */
  {VIR_FUNCFLAG_HAS_SAMPLER_INDEXINED, gcvFUNC_HAS_SAMPLER_INDEXINED   }, /*  0x8000, This function has sampler indexing used */
};

static gctUINT  virFunctionFlagCount   = sizeof(virFunctionFlagMap) / sizeof(conv2VirsFunctionFlagMap);

static VIR_FunctionFlag
_ConvFunctionFlagsToVir(
    IN gceFUNCTION_FLAG Flags
    )
{
    VIR_FunctionFlag virFunctionFlags = VIR_FUNCFLAG_NOATTR;

    gctUINT i;
    for(i = 0; i < virFunctionFlagCount; i++) {
        if(virFunctionFlagMap[i].functionFlag & Flags) {
            virFunctionFlags |= virFunctionFlagMap[i].virFunctionFlag;
        }
    }
    return virFunctionFlags;
}

static VIR_TypeId
_getRowTypeId(
    IN VIR_TypeId TypeId
    )
{
    if (VIR_TypeId_isPrimitive(TypeId))
    {
        gctUINT rows = VIR_GetTypeRows(TypeId);
        if (rows > 1)
        {
            return VIR_GetTypeRowType(TypeId);
        }
        else
        {
            return TypeId;
        }
    }
    else
        return VIR_TYPE_UNKNOWN;
}

static gctUINT
_getRealRowsUsedByParam(
    IN gcSHADER   Shader,
    IN gcFUNCTION Function,
    IN gctUINT    ParamNo,
    OUT gctUINT * EndIndex)
{
    gcsFUNCTION_ARGUMENT *argument;
    gcVARIABLE variable = gcvNULL;
    gctUINT32 startIndex = 0, endIndex;
    gctUINT32 components = 0, rows = 1;

    gcmASSERT(ParamNo < Function->argumentCount);
    argument = &Function->arguments[ParamNo];
    gcmASSERT(argument->variableIndex < Shader->variableCount);
    variable = Shader->variables[argument->variableIndex];
    if (variable)
        {
        gcTYPE_GetTypeInfo(variable->u.type, &components, &rows, 0);
        rows *= variable->arraySize;

        gcSHADER_GetVariableIndexingRange(Shader,
                                            variable,
                                            gcvFALSE,
                                            &startIndex,
                                            &endIndex);
        /* check if the variable is already expanded, if so, do not expand it again */
        if (rows > 1)
        {
            /* if the start index of variable is not the same as argument's
             * tempIndex, or if the next arg share the same variable as this arg,
             * that means the variable is expanded to multiple rows,
             */
            if (startIndex != argument->index ||
                ((ParamNo+1 < Function->argumentCount) &&
                 (Function->arguments[ParamNo+1].variableIndex == argument->variableIndex)))
            {
                /* check if the next arg share the same variable as this arg*/
                rows = 1;
                endIndex = argument->index + 1;
            }
        }
    }
    else
    {
        endIndex = argument->index + 1;
    }
    *EndIndex = endIndex;
    return rows;
}

static VSC_ErrCode
_ConvShaderFunction2Vir(
    IN gcSHADER Shader,
    IN gcFUNCTION Function,
    IN VIR_Shader *VirShader,
    IN conv2VirsVirRegMap *VirRegMapArr,
    OUT VIR_Function **VirFunction
    )
{
    VSC_ErrCode virErrCode;
    VIR_Function *virFunction;
    VIR_SymId paramId;
    gctUINT i;

    gcmASSERT(Shader);
    gcmASSERT(VirShader);
    gcmASSERT(Function);

    virErrCode = VIR_Shader_AddFunction(VirShader,
                                        gcvFALSE, /* IsKernel */
                                        Function->name,
                                        VIR_TYPE_UNKNOWN,
                                        &virFunction);
    if(virErrCode != VSC_ERR_NONE &&
       virErrCode == VSC_ERR_REDEFINITION) {
        VIR_NameId nameId;

        virErrCode = VIR_Shader_AddString(VirShader,
                                          Function->name,
                                          &nameId);
        if(virErrCode != VSC_ERR_NONE) return virErrCode;

        virErrCode = _ResolveNameClash(VirShader,
                                       nameId,
                                       "$$",
                                       Function->label,
                                       &nameId);
        if(virErrCode != VSC_ERR_NONE) return virErrCode;
        virErrCode = VIR_Shader_AddFunction(VirShader,
                                            gcvFALSE, /* IsKernel */
                                            VIR_Shader_GetStringFromId(VirShader, nameId),
                                            VIR_TYPE_UNKNOWN,
                                            &virFunction);
    }

    CHECK_ERROR(virErrCode, "Failed to add VIR Function");
    if(virErrCode != VSC_ERR_NONE) return virErrCode;

    gcmASSERT(virFunction);
    virFunction->flags = _ConvFunctionFlagsToVir(Function->flags);
    if(VirFunction) {
        *VirFunction = virFunction;
    }

    /* Function arguments */
    for (i = 0; i < Function->argumentCount; i++)
    {
        gcsFUNCTION_ARGUMENT *argument;
        gctCHAR   parameterName[30];
        gctUINT   offset = 0;
        VIR_Symbol *sym, *param;
        VIR_SymId virRegId;
        VIR_TypeId typeId = VIR_TYPE_UNKNOWN;
        VIR_TypeId rowTypeId;
        gcVARIABLE variable = gcvNULL;
        VIR_StorageClass virStorage;
        gctUINT32 components = 0, rows = 0;
        gctUINT32 endIndex = 0;
        gctBOOL packed = gcvFALSE;

        /* Point to arguments. */
        argument = &Function->arguments[i];

        /*
        ** We need to save the argument type so that
        ** we can get the correct type for a sampler argument.
        */
        if (argument->qualifier == gcvFUNCTION_INPUT)
        {
            if (argument->variableIndex < Shader->variableCount)
            {
                variable = Shader->variables[argument->variableIndex];
                _ConvShaderTypeToVirTypeId(variable->u.type, &typeId);
            }
            else
            {
                gcmASSERT(gcvFALSE || argument->variableIndex == 0xFFFF);
            }
        }

        rowTypeId = _getRowTypeId(typeId);
        if (variable)
        {
            rows = _getRealRowsUsedByParam(Shader, Function, i, &endIndex);
            if (rows == 1)
            {
                typeId = rowTypeId;
            }
        }
        else
        {
            components = _GetEnableComponentCount(argument->enable);
            endIndex = argument->index + 1;
        }

        /* set the function as always inline if any parameter is image or sampler type */
        if (VIR_TypeId_isSamplerOrImage(rowTypeId) || VIR_TypeId_isImageT(rowTypeId))
        {
            VIR_Function_SetFlag(virFunction, VIR_FUNCFLAG_ALWAYSINLINE);
        }
        /* make parameter name */
        virStorage = _gcmConvParamQualifier2Vir (argument->qualifier);
        gcoOS_PrintStrSafe(parameterName, sizeof(parameterName), &offset, "@param_%d", i);
        virErrCode = VIR_Function_AddParameter(virFunction,
                                               parameterName,
                                               typeId,
                                               virStorage,
                                               &paramId);
        CHECK_ERROR(virErrCode, "Failed to add VIR Parameter");
        if(virErrCode != VSC_ERR_NONE) return virErrCode;

        param = VIR_Function_GetSymFromId(virFunction, paramId);
        param->u2.tempIndex = argument->index;
        VIR_Symbol_SetPrecision(param, _gcmConvPrecision2Vir(argument->precision));
        VIR_Symbol_SetIndexRange(param, (gctINT)endIndex);

        /* virReg symbol of the parameter is added to global symbol table
         * so it can be used outside of the function to pass argument
         */
        virErrCode = VIR_Shader_AddSymbol(VirShader,
                                          VIR_SYM_VIRREG,
                                          argument->index,
                                          VIR_Shader_GetTypeFromId(VirShader, rowTypeId),
                                          virStorage,
                                          &virRegId);
        if(virErrCode != VSC_ERR_NONE && virErrCode != VSC_ERR_REDEFINITION) {
            CHECK_ERROR(virErrCode, "Failed to add virtual register symbol");
            return virErrCode;
        }

        if (virErrCode != VSC_ERR_REDEFINITION)
        {
            gcmASSERT(VirRegMapArr[param->u2.tempIndex].virRegId == VIR_INVALID_ID);
            VirRegMapArr[argument->index].virRegId = virRegId;
            VirRegMapArr[argument->index].components = components;
            VirRegMapArr[argument->index].packed = packed;

            sym = VIR_Shader_GetSymFromId(VirShader, virRegId);
            VIR_Symbol_SetVregVariable(sym, param);
            VIR_Symbol_SetParamFuncSymId(sym, VIR_Function_GetSymId(virFunction));
            VIR_Symbol_SetPrecision(sym, VIR_Symbol_GetPrecision(param));
            VIR_Symbol_SetIndexRange(sym, (gctINT)endIndex);
            if(argument->flags & gceFUNCTION_ARGUMENT_FLAG_IS_PRECISE)
            {
                VIR_Symbol_SetFlag(sym, VIR_SYMFLAG_PRECISE);
            }
            /* create extra virreg for multi row type */
            if (variable)
            {
                gctUINT         j;
                gctUINT         virRegIndex = argument->index + 1;

                for (j = 1; j < rows; j++, virRegIndex++)
                {
                    virErrCode = VIR_Shader_AddSymbol(VirShader,
                                                      VIR_SYM_VIRREG,
                                                      virRegIndex,
                                                      VIR_Shader_GetTypeFromId(VirShader, rowTypeId),
                                                      virStorage,
                                                      &virRegId);
                    CHECK_ERROR(virErrCode, "Failed to add virtual register symbol");
                    if(virErrCode != VSC_ERR_NONE) {
                        return virErrCode;
                    }
                    sym = VIR_Shader_GetSymFromId(VirShader, virRegId);
                    VIR_Symbol_SetVregVariable(sym, param);
                    VIR_Symbol_SetParamFuncSymId(sym, VIR_Function_GetSymId(virFunction));
                    VIR_Symbol_SetPrecision(sym, VIR_Symbol_GetPrecision(param));
                    VIR_Symbol_SetIndexRange(sym, (gctINT)endIndex);
                    gcmASSERT(VirRegMapArr[virRegIndex].virRegId == VIR_INVALID_ID);
                    VirRegMapArr[virRegIndex].virRegId = virRegId;
                    VirRegMapArr[virRegIndex].components = components;
                    VirRegMapArr[virRegIndex].packed = packed;
                }
            }
        }
        else
        {
            gcmASSERT(VirRegMapArr[argument->index].virRegId == virRegId);
        }

    }

    /* Function variables */
    for (i = 0; i < Function->localVariableCount; i++)
    {
        gcVARIABLE localVariable;
        gctUINT32 startRegIndex;
        gctUINT32 endRegIndex;

        if (Function->localVariables[i] == gcvNULL) continue;

        localVariable = Function->localVariables[i];

        if (localVariable->parent != -1) {
            continue;
        }
        virErrCode =_ConvShaderLocalVariable2Vir(Function->localVariableCount,
                                                 Function->localVariables,
                                                 localVariable,
                                                 VirRegMapArr,
                                                 _GetLocalStartRegIndex(Function->localVariableCount,
                                                                        Function->localVariables, localVariable),
                                                 VirShader,
                                                 virFunction,
                                                 gcvNULL,
                                                 gcvNULL,
                                                 &startRegIndex,
                                                 &endRegIndex);
        CHECK_ERROR(virErrCode, "Failed to Convert Shader Local Variable");

        if(virErrCode != VSC_ERR_NONE) return virErrCode;
    }

    virFunction->tempIndexStart = Function->tempIndexStart;
    virFunction->tempIndexCount = Function->tempIndexCount;

    virFunction->die = Function->die;
    virFunction->debugInfo = VirShader->debugInfo;

    return VSC_ERR_NONE;
}

static VSC_ErrCode
_ConvKernelInfo(
    IN gcSHADER             Shader,
    IN gcKERNEL_FUNCTION    Function,
    IN VIR_Shader *         VirShader,
    IN conv2VirsVirRegMap * VirRegMapArr,
    IN VIR_SymId   *        VirUniformIdArr,
    OUT VIR_Function *      VirFunction
    )
{
    VSC_ErrCode     virErrCode = VSC_ERR_NONE;
    VSC_MM *        memPool    = &VirShader->pmp.mmWrapper;
    VIR_KernelInfo *kInfo;
    VIR_IdList *    idList;
    VIR_ValueList * valueList;
    gctUINT         i;

    /* allocate Shader object from memory pool */
    kInfo = (VIR_KernelInfo *)vscMM_Alloc(memPool, sizeof(VIR_KernelInfo));
    if (kInfo == gcvNULL)
    {
        return VSC_ERR_OUT_OF_MEMORY;
    }

    /* set default vaules */
    memset(kInfo, 0, sizeof(VIR_KernelInfo));

    kInfo->kernelName       = VIR_Function_GetNameID(VirFunction);
    kInfo->localMemorySize  = Function->localMemorySize;
    kInfo->samplerIndex     = Function->samplerIndex;
    kInfo->isMain           = Function->isMain;

    VirFunction->kernelInfo = kInfo;

    if(!Function->isCalledByEntryKernel)
    {
        /* Convert uniformArguments:
         * uniform should already converted, get the converted uniform symbol ID */
        idList = &kInfo->uniformArguments;
        virErrCode = VIR_IdList_Init(memPool, Function->uniformArgumentCount, &idList);
        CHECK_ERROR(virErrCode, "InitIdList");
        for (i=0; i < Function->uniformArgumentCount; i++)
        {
            VIR_SymId uId = VirUniformIdArr[Function->uniformArguments[i]->index];
            gcmASSERT(VIR_Id_isValid(uId));
            virErrCode = VIR_IdList_Add(idList, uId);
            CHECK_ERROR(virErrCode, "Failed to add uniformArguments[%d] to VIR Kernel Function", i);
        }
    }

    /* Convert imageSamplers  */
    valueList = &kInfo->imageSamplers;
    virErrCode = VIR_ValueList_Init(memPool, Function->imageSamplerCount, sizeof(VIR_ImageSampler), &valueList);
    CHECK_ERROR(virErrCode, "VIR_ValueList_Init");
    for (i=0; i < Function->imageSamplerCount; i++)
    {
        VIR_ImageSampler imgSampler;
        gcsIMAGE_SAMPLER_PTR smpl = & Function->imageSamplers[i];
        imgSampler.imageNum                 = smpl->imageNum;
        imgSampler.isConstantSamplerType    = smpl->isConstantSamplerType;
        imgSampler.samplerType              = smpl->samplerType;

        virErrCode = VIR_ValueList_Add(valueList, (gctCHAR *)&imgSampler);
        CHECK_ERROR(virErrCode, "Failed to add imageSamplers[%d] to VIR Kernel Function", i);
    }

    /* Convert properties  */
    valueList = &kInfo->properties;
    virErrCode = VIR_ValueList_Init(memPool, Function->propertyCount, sizeof(VIR_KernelProperty), &valueList);
    CHECK_ERROR(virErrCode, "VIR_ValueList_Init");
    for (i=0; i < Function->propertyCount; i++)
    {
        VIR_KernelProperty property;

        gcKERNEL_FUNCTION_GetProperty(Function, i, &property.propertySize, &property.propertyType, property.propertyValue);

        virErrCode = VIR_ValueList_Add(valueList, (gctCHAR *)&property);
        CHECK_ERROR(virErrCode, "Failed to add properties[%d] to VIR Kernel Function", i);
    }

    return virErrCode;
}

/* convert kernel function to VIR Function */
static VSC_ErrCode
_ConvShaderKernelFunction2Vir(
    IN gcSHADER             Shader,
    IN gcKERNEL_FUNCTION    KernelFunction,
    IN VIR_Shader *         VirShader,
    IN conv2VirsVirRegMap * VirRegMapArr,
    IN VIR_SymId   *        VirUniformIdArr,
    OUT VIR_Function **     VirFunction
    )
{
    VSC_ErrCode virErrCode = VSC_ERR_NONE;
    VIR_Function *virFunction;
    VIR_SymId paramId;
    gctUINT i;

    gcmASSERT(Shader);
    gcmASSERT(VirShader);
    gcmASSERT(KernelFunction);

    virErrCode = VIR_Shader_AddFunction(VirShader,
                                        gcvTRUE, /* IsKernel */
                                        KernelFunction->name,
                                        VIR_TYPE_UNKNOWN,
                                        &virFunction);
    if(virErrCode == VSC_ERR_REDEFINITION) {
        VIR_NameId nameId;

        virErrCode = VIR_Shader_AddString(VirShader,
                                          KernelFunction->name,
                                          &nameId);
        if(virErrCode != VSC_ERR_NONE) return virErrCode;

        virErrCode = _ResolveNameClash(VirShader,
                                       nameId,
                                       "$$",
                                       KernelFunction->label,
                                       &nameId);
        if(virErrCode != VSC_ERR_NONE) return virErrCode;
        virErrCode = VIR_Shader_AddFunction(VirShader,
                                            gcvTRUE, /* IsKernel */
                                            VIR_Shader_GetStringFromId(VirShader, nameId),
                                            VIR_TYPE_UNKNOWN,
                                            &virFunction);
    }

    CHECK_ERROR(virErrCode, "Failed to add VIR KernelFunction");
    if(virErrCode != VSC_ERR_NONE) return virErrCode;

    gcmASSERT(virFunction);
    virFunction->flags = _ConvFunctionFlagsToVir(KernelFunction->flags);

    if (Shader->currentKernelFunction == KernelFunction)
    {
        VIR_Shader_SetCurrentKernelFunction(VirShader, virFunction);
        VIR_Shader_SetKernelNameId(VirShader, VIR_Function_GetNameID(virFunction));
    }
    if (KernelFunction->isMain)
    {
        virFunction->flags |= VIR_FUNCFLAG_KERNEL_MERGED_MAIN;
    }

    virFunction->die = KernelFunction->die;
    virFunction->debugInfo = VirShader->debugInfo;

    if(VirFunction) {
        *VirFunction = virFunction;
    }

    /* KernelFunction arguments */
    for (i = 0; i < KernelFunction->argumentCount; i++)
    {
        gcsFUNCTION_ARGUMENT *argument;
        gctCHAR   parameterName[30];
        gctUINT   offset = 0;
        VIR_Symbol *sym, *param;
        VIR_SymId virRegId;
        VIR_TypeId typeId = VIR_TYPE_UNKNOWN;
        VIR_TypeId rowTypeId;
        gcVARIABLE variable = gcvNULL;
        VIR_StorageClass virStorage;

        /* Point to arguments. */
        argument = &KernelFunction->arguments[i];

        if (argument->qualifier == gcvFUNCTION_INPUT)
        {
            if (argument->variableIndex < Shader->variableCount)
            {
                variable = Shader->variables[argument->variableIndex];
                _ConvShaderTypeToVirTypeId(variable->u.type, &typeId);
            }
            else
            {
                gcmASSERT(gcvFALSE || argument->variableIndex == 0xFFFF);
            }
        }

        rowTypeId = _getRowTypeId(typeId);
        /* make parameter name */
        virStorage = _gcmConvParamQualifier2Vir (argument->qualifier);
        gcoOS_PrintStrSafe(parameterName, sizeof(parameterName), &offset, "@param_%d", i);
        virErrCode = VIR_Function_AddParameter(virFunction,
                                               parameterName,
                                               typeId,
                                               virStorage,
                                               &paramId);
        CHECK_ERROR(virErrCode, "Failed to add VIR Parameter");
        if(virErrCode != VSC_ERR_NONE) return virErrCode;

        param = VIR_Function_GetSymFromId(virFunction, paramId);
        param->u2.tempIndex = argument->index;
        virErrCode = VIR_Shader_AddSymbol(VirShader,
                                          VIR_SYM_VIRREG,
                                          argument->index,
                                          VIR_Shader_GetTypeFromId(VirShader, rowTypeId),
                                          virStorage,
                                          &virRegId);
        if(virErrCode != VSC_ERR_NONE && virErrCode != VSC_ERR_REDEFINITION) {
            CHECK_ERROR(virErrCode, "Failed to add virtual register symbol");
            return virErrCode;
        }

        if (virErrCode != VSC_ERR_REDEFINITION) {
            gctUINT32       components = 0, rows = 0;
            gctBOOL packed = gcvFALSE;

            gcmASSERT(VirRegMapArr[param->u2.tempIndex].virRegId == VIR_INVALID_ID);
            VirRegMapArr[argument->index].virRegId = virRegId;
            if(variable) {
                gcTYPE_GetTypeInfo(variable->u.type, &components, &rows, 0);
                packed = gcTYPE_IsTypePacked(variable->u.type);
            }
            VirRegMapArr[argument->index].packed = packed;

            if(packed) {
                VirRegMapArr[argument->index].components = components;
            }
            else {
                VirRegMapArr[argument->index].components = _GetEnableComponentCount(argument->enable);
            }
            sym = VIR_Shader_GetSymFromId(VirShader, virRegId);
            VIR_Symbol_SetVregVariable(sym, param);
            VIR_Symbol_SetParamFuncSymId(sym, VIR_Function_GetSymId(virFunction));
            VIR_Symbol_SetPrecision(sym, VIR_Symbol_GetPrecision(param));
            if(argument->flags & gceFUNCTION_ARGUMENT_FLAG_IS_PRECISE)
            {
                VIR_Symbol_SetFlag(sym, VIR_SYMFLAG_PRECISE);
            }
            /* create extra virreg for multi row type */
            if (variable)
            {
                gctUINT         j;
                gctUINT         virRegIndex = argument->index + 1;

                gcTYPE_GetTypeInfo(variable->u.type, &components, &rows, 0);
                rows *= variable->arraySize;
                for (j = 1; j < rows; j++, virRegIndex++)
                {
                    virErrCode = VIR_Shader_AddSymbol(VirShader,
                                                      VIR_SYM_VIRREG,
                                                      virRegIndex,
                                                      VIR_Shader_GetTypeFromId(VirShader, rowTypeId),
                                                      virStorage,
                                                      &virRegId);
                    CHECK_ERROR(virErrCode, "Failed to add virtual register symbol");
                    if(virErrCode != VSC_ERR_NONE) {
                        return virErrCode;
                    }
                    sym = VIR_Shader_GetSymFromId(VirShader, virRegId);
                    VIR_Symbol_SetPrecision(sym, VIR_Symbol_GetPrecision(param));
                    gcmASSERT(VirRegMapArr[virRegIndex].virRegId == VIR_INVALID_ID);
                    VirRegMapArr[virRegIndex].virRegId = virRegId;
                    VirRegMapArr[virRegIndex].components = _GetEnableComponentCount(argument->enable);
                    VirRegMapArr[virRegIndex].packed = packed;
                }
            }
        }
        else
        {
            gcmASSERT(VirRegMapArr[argument->index].virRegId == virRegId);
        }
    }

    /* KernelFunction variables */
    for (i = 0; i < KernelFunction->localVariableCount; i++)
    {
        gcVARIABLE localVariable;
        gctUINT32 startRegIndex;
        gctUINT32 endRegIndex;

        if (KernelFunction->localVariables[i] == gcvNULL) continue;

        localVariable = KernelFunction->localVariables[i];

        if (localVariable->parent != -1) {
            continue;
        }
        virErrCode =_ConvShaderLocalVariable2Vir(KernelFunction->localVariableCount,
                                                 KernelFunction->localVariables,
                                                 localVariable,
                                                 VirRegMapArr,
                                                 _GetLocalStartRegIndex(KernelFunction->localVariableCount,
                                                                        KernelFunction->localVariables, localVariable),
                                                 VirShader,
                                                 virFunction,
                                                 gcvNULL,
                                                 gcvNULL,
                                                 &startRegIndex,
                                                 &endRegIndex);
        CHECK_ERROR(virErrCode, "Failed to Convert Shader Local Variable");

        if(virErrCode != VSC_ERR_NONE) return virErrCode;
    }

    /* iF kernel function not being called, convert kernel info */
    return _ConvKernelInfo(Shader,
                           KernelFunction,
                           VirShader,
                           VirRegMapArr,
                           VirUniformIdArr,
                           virFunction);
}

static VSC_ErrCode
_ConvUniformBlock2Vir(
    IN gcSHADER Shader,
    IN gcsUNIFORM_BLOCK UniformBlock,
    IN VIR_Shader *VirShader,
    IN OUT VIR_TypeId *StructTypeId,
    IN OUT VIR_UniformBlock **VirUniformBlock
    )
{
    gceSTATUS status;
    VSC_ErrCode virErrCode;
    gcUNIFORM uniform;
    VIR_UniformBlock *virUniformBlock;
    gctINT uniformIndex;
    VIR_SymId symId;
    VIR_Symbol *sym;
    VIR_NameId nameId;
    VIR_TypeId typeId;
    VIR_Type *structType;
    gctUINT16 firstUniformIndex;
    gctINT16 virBlockIndex;
    gctBOOL createType = gcvFALSE;

    gcmHEADER_ARG("Shader=0x%x UniformBlock=0x%x VirShader=0x%x StructTypeId=0x%x",
                  Shader, UniformBlock, VirShader, StructTypeId);

    gcmASSERT(Shader);
    gcmASSERT(VirShader);
    gcmASSERT(UniformBlock);

    /* Add name string. */
    virErrCode = VIR_Shader_AddString(VirShader,
                                      UniformBlock->name,
                                      &nameId);
    if (virErrCode != VSC_ERR_NONE) return virErrCode;

    /* Add type. */
    typeId = *StructTypeId;
    createType = (typeId == VIR_TYPE_UNKNOWN);

    if (createType)
    {
        virErrCode = VIR_Shader_AddStructType(VirShader,
                                              gcvFALSE,
                                              nameId,
                                              gcvFALSE,
                                              &typeId);
        if (virErrCode != VSC_ERR_NONE) return virErrCode;
    }

    /* Add ubo. */
    virErrCode = VIR_Shader_AddSymbol(VirShader,
                                      VIR_SYM_UBO,
                                      nameId,
                                      VIR_Shader_GetTypeFromId(VirShader, typeId),
                                      VIR_STORAGE_UNKNOWN,
                                      &symId);
    if (virErrCode != VSC_ERR_NONE) return virErrCode;

    virBlockIndex = (gctINT16)(VIR_IdList_Count(&VirShader->uniformBlocks) - 1);

    /* If this type is new, we need to create its members. */
    if (createType)
    {
        structType = VIR_Shader_GetTypeFromId(VirShader, typeId);
        uniformIndex = (gctINT)GetUBFirstChild(UniformBlock);
        if (uniformIndex != -1)
        {
            status = gcSHADER_GetUniform(Shader,
                                         uniformIndex,
                                         &uniform);
            if (gcmIS_ERROR(status))
            {
                /* Error. */
                gcmFOOTER();
                return VSC_ERR_INVALID_DATA;
            }

            virErrCode =_ConvShaderUniformIdx2Vir(Shader,
                                                  uniformIndex,
                                                  _GetStartUniformIndex(Shader, uniform),
                                                  virBlockIndex,
                                                  VirShader,
                                                  structType,
                                                  &firstUniformIndex,
                                                  gcvNULL);
            if (virErrCode != VSC_ERR_NONE) return virErrCode;
        }
    }

    sym = VIR_Shader_GetSymFromId(VirShader, symId);

    VIR_Symbol_SetPrecision(sym, _gcmConvPrecision2Vir(GetUBPrecision(UniformBlock)));
    VIR_Symbol_SetAddrSpace(sym, VIR_AS_CONSTANT);
    VIR_Symbol_SetTyQualifier(sym, VIR_TYQUAL_CONST);
    if(gcoOS_StrCmp(UniformBlock->name, "#DefaultUBO") == gcvSTATUS_OK)
    {
        VIR_Symbol_SetFlag(sym, VIR_SYMUBOFLAG_IS_DUBO);
        VIR_Shader_SetDefaultUBOIndex(VirShader, virBlockIndex);
    }
    if(gcoOS_StrCmp(UniformBlock->name, "#ConstantUBO") == gcvSTATUS_OK)
    {
        VIR_Symbol_SetFlag(sym, VIR_SYMUBOFLAG_IS_CUBO);
        VIR_Shader_SetConstantUBOIndex(VirShader, virBlockIndex);
    }

    virUniformBlock = sym->u2.ubo;
    gcmASSERT(virUniformBlock);
    virUniformBlock->sym = symId;
    virUniformBlock->blockSize = GetUBBlockSize(UniformBlock);
    VIR_IB_SetFlag(virUniformBlock, _ConvIBFlag2Vir(GetUBInterfaceBlockInfo(UniformBlock)));

    /* ubo array share the same address uniform array, so convert it once */
    if (GetUBArrayIndex(UniformBlock) == 0 || GetUBArrayIndex(UniformBlock) == -1)
    {
        virErrCode =_ConvShaderUniformIdx2Vir(Shader,
                                              (gctINT)GetUBIndex(UniformBlock),
                                              (gctINT)GetUBIndex(UniformBlock),
                                              -1,
                                              VirShader,
                                              gcvNULL,
                                              &firstUniformIndex,
                                              &virUniformBlock->baseAddr);
        if (virErrCode != VSC_ERR_NONE) return virErrCode;
    }

    /* set layout info */
    VIR_Symbol_SetLayoutQualifier(sym,
        _gcmConvMemoryLayout2Vir(GetUBMemoryLayout(UniformBlock)));
    VIR_Symbol_SetLocation(sym, 0);
    VIR_Symbol_SetBinding(sym, GetUBBinding(UniformBlock));

    if (StructTypeId)
    {
        *StructTypeId = typeId;
    }
    if (VirUniformBlock)
    {
        *VirUniformBlock = virUniformBlock;
    }

    gcmFOOTER_NO();
    return VSC_ERR_NONE;
}

static VSC_ErrCode
_ConvShaderOutput2Vir(
    IN gcSHADER             Shader,
    IN gctUINT32*           Index,
    IN gcOUTPUT             Output,
    IN VIR_SymId*           VirOutIdArr,
    IN conv2VirsVirRegMap*  VirRegMapArr,
    IN VIR_Shader*          VirShader,
    IN VIR_SymId            IoBlockIndex,
    OUT VIR_Symbol**        OutputSymbol
    )
{
    gceSTATUS           status = gcvSTATUS_OK;
    VSC_ErrCode         virErrCode = VSC_ERR_NONE;
    gctUINT32           components = 0, rows = 0;
    gcSL_FORMAT         regFormat = gcSL_INVALID;
    VIR_TypeId          typeId;
    VIR_SymId           symId;
    VIR_Symbol          *sym;
    VIR_NameId          nameId;
    gctUINT16           j;
    VIR_SymId           virRegId;
    VIR_StorageClass    storage;
    gctINT              outputIdx = *Index;

    status = _ConvBuiltinNameKindToVirNameId(Output->nameLength,
                                             &nameId);

    if (status == gcvSTATUS_NOT_FOUND)
    {
        virErrCode = VIR_Shader_AddString(VirShader,
                                          Output->name,
                                          &nameId);
        ON_ERROR2STATUS(virErrCode, "Failed to AddString");
    }
    /* reset status */
    status = gcvSTATUS_OK;
    virErrCode = _ConvShaderTypeToVirTypeId(Output->type,
                                            &typeId);
    ON_ERROR2STATUS(virErrCode, "Failed to conv shader type to VIR type Id");

    if (gcmOUTPUT_isArray(Output))
    {
        virErrCode = VIR_Shader_AddArrayType(VirShader,
                                             typeId,
                                             Output->arraySize,
                                             0,
                                             &typeId);
        ON_ERROR2STATUS(virErrCode, "Failed to add VIR array type Id");
        /* advance index to output to account for arrayed outputs being expanded */
        *Index += (gctUINT32) Output->arraySize - 1;
    }

    /* the builtin output variable gl_SampleMask is a special variables, it is
     * allocated to r0.z last 4 bits, the other bits are input values for other
     * purpose which cannot be changed, so we need to mark it as in/out variable
     */
    if (nameId == VIR_NAME_SUBSAMPLE_DEPTH || nameId == VIR_NAME_SAMPLE_MASK)
    {
        storage = VIR_STORAGE_INOUTPUT;
    }
    else
    {
        storage = gcmOUTPUT_isPerPatch(Output) ? VIR_STORAGE_PERPATCH_OUTPUT : VIR_STORAGE_OUTPUT;
    }
    virErrCode = VIR_Shader_AddSymbol(VirShader,
                                      VIR_SYM_VARIABLE,
                                      nameId,
                                      VIR_Shader_GetTypeFromId(VirShader, typeId),
                                      storage,
                                      &symId);
    ON_ERROR2STATUS(virErrCode, "Failed to add output symbol");
    if (virErrCode != VSC_ERR_NONE)
    {
        status = gcvSTATUS_INVALID_ARGUMENT;
        goto OnError;
    }
    sym = VIR_Shader_GetSymFromId(VirShader, symId);
    VirOutIdArr[outputIdx] = symId;

    VIR_Symbol_SetTyQualifier(sym, VIR_TYQUAL_NONE);
    VIR_Symbol_SetPrecision(sym, _gcmConvPrecision2Vir(Output->precision));

    if (gcmOUTPUT_isArray(Output) &&
        VIR_Symbol_GetPrecision(sym) == VIR_PRECISION_HIGH)
    {
        VIR_Shader_SetFlag(VirShader, VIR_SHFLAG_HAS_OUTPUT_ARRAY_HIGHP);
    }

    sym->u2.tempIndex = Output->tempIndex;

    sym->ioBlockIndex = IoBlockIndex;

    VIR_Symbol_SetFlag(sym, VIR_SYMFLAG_ENABLED);

    sym->flags = VIR_SYMFLAG_ENABLED |
                    (gcmOUTPUT_packedAway(Output)
                        ? VIR_SYMFLAG_PACKEDAWAY
                        : 0) |
                    (gcmOUTPUT_alwaysUsed(Output)
                        ? VIR_SYMFLAG_ALWAYSUSED
                        : 0) |
                    (gcmOUTPUT_isPosition(Output)
                        ? VIR_SYMFLAG_POSITION
                        : 0) |
                    (gcmOUTPUT_isInvariant(Output)
                        ? VIR_SYMFLAG_INVARIANT
                        : 0) |
                    (gcmOUTPUT_isPerVertexArray(Output)
                        ? VIR_SYMFLAG_ARRAYED_PER_VERTEX
                        : 0) |
                    (gcmOUTPUT_isPrecise(Output)
                        ? VIR_SYMFLAG_PRECISE
                        : 0) |
                    (Output->shaderMode == gcSHADER_SHADER_FLAT
                        ? VIR_SYMFLAG_FLAT
                        : 0) |
                    (gcmOUTPUT_isStaticallyUsed(Output)
                        ? VIR_SYMFLAG_STATICALLY_USED
                        : 0) |
                    (gcmOUTPUT_isIOBLockMember(Output)
                        ? VIR_SYMFLAG_IS_IOBLOCK_MEMBER
                        : 0) |
                    (gcmOUTPUT_isInstanceMember(Output)
                        ? VIR_SYMFLAG_IS_INSTANCE_MEMBER
                        : 0) |
                    (gcmOUTPUT_isCentroid(Output)
                        ? VIR_SYMFLAG_ISCENTROID
                        : 0) |
                    (gcmOUTPUT_isSample(Output)
                        ? VIR_SYMFLAG_ISSAMPLE
                        : 0);

    sym->flagsExt = (Output->shaderMode == gcSHADER_SHADER_NOPERSPECTIVE
                        ? VIR_SYMFLAGEXT_NOPERSPECTIVE
                        : VIR_SYMFLAGEXT_NONE);

    {
        /* set layout info */
        VIR_Symbol_SetLayoutQualifier(sym, VIR_LAYQUAL_NONE);
        VIR_Symbol_SetLocation(sym, Output->location);
    }

    gcTYPE_GetTypeInfo(Output->type, &components, &rows, 0);
    rows *= Output->arraySize;
    regFormat = gcGetFormatFromType(Output->type);
    for (j = 0; j < rows; j++)
    {
        virRegId = _GetVirRegId(VirShader,
                                VirRegMapArr,
                                Output->tempIndex + j,
                                regFormat,
                                gcvFALSE,
                                components,
                                gcTYPE_IsTypePacked(Output->type),
                                VIR_Symbol_GetPrecision(sym));
        if (virRegId == VIR_INVALID_ID)
        {
            status = gcvSTATUS_INVALID_ARGUMENT;
            goto OnError;
        }

        gcmASSERT(VirRegMapArr[Output->tempIndex + j].virRegId != VIR_INVALID_ID);
        sym = VIR_Shader_GetSymFromId(VirShader, virRegId);
        VIR_Symbol_SetVregVarSymId(sym, symId);
        VIR_Symbol_SetStorageClass(sym, storage);
        if (gcmOUTPUT_isPerPatch(Output))
        {
            VIR_IdList_Add(&VirShader->perpatchOutputVregs, virRegId);
        }
        else
        {
            VIR_IdList_Add(&VirShader->outputVregs, virRegId);
        }
    }

    if (OutputSymbol)
    {
        *OutputSymbol = sym;
    }

OnError:
    return virErrCode;
}

static VSC_ErrCode
_ConvShaderAttribute2Vir(
    IN gcSHADER             Shader,
    IN gctUINT32            Index,
    IN gcATTRIBUTE          Attribute,
    IN VIR_SymId*           VirAttrIdArr,
    IN conv2VirsVirRegMap*  VirRegMapArr,
    IN gctUINT*             AttrVirRegCount,
    IN VIR_Shader*          VirShader,
    IN VIR_SymId            IoBlockIndex,
    IN VSC_HW_CONFIG*       hwCfg,
    OUT VIR_Symbol**        AttributeSymbol
    )
{
    gceSTATUS           status = gcvSTATUS_OK;
    VSC_ErrCode         virErrCode = VSC_ERR_NONE;
    gctUINT32           components = 0, rows = 0;
    gcSL_FORMAT         regFormat = gcSL_INVALID;
    VIR_TypeId          typeId;
    VIR_SymId           symId;
    VIR_Symbol          *sym;
    VIR_NameId          nameId;
    gctUINT             j;
    gctUINT             attrVirRegCount = *AttrVirRegCount;
    VIR_SymId           virRegId;

    status = _ConvBuiltinNameKindToVirNameId(Attribute->nameLength,
                                             &nameId);

    if (status == gcvSTATUS_NOT_FOUND)
    {
        virErrCode = VIR_Shader_AddString(VirShader,
                                          Attribute->name,
                                          &nameId);
        ON_ERROR2STATUS(virErrCode, "Failed to AddString");
    }
    /* reset status */
    status = gcvSTATUS_OK;
    gcmONERROR(_ConvShaderTypeToVirTypeId(Attribute->type, &typeId));

    if (GetATTRIsArray(Attribute) > 0)
    {
        virErrCode = VIR_Shader_AddArrayType(VirShader,
                                             typeId,
                                             Attribute->arraySize,
                                             0,
                                             &typeId);
        ON_ERROR2STATUS(virErrCode, "Failed to add VIR array type Id");
    }
    virErrCode = VIR_Shader_AddSymbol(VirShader,
                                      VIR_SYM_VARIABLE,
                                      nameId,
                                      VIR_Shader_GetTypeFromId(VirShader, typeId),
                                      gcmATTRIBUTE_isPerPatch(Attribute) ? VIR_STORAGE_PERPATCH_INPUT
                                                                           : VIR_STORAGE_INPUT,
                                      &symId);

    if (virErrCode == VSC_ERR_REDEFINITION)
    {
        virErrCode = _ResolveNameClash(VirShader,
                                       nameId,
                                       "$$",
                                       Index,
                                       &nameId);
        if (virErrCode == VSC_ERR_NONE)
        {
            virErrCode = VIR_Shader_AddSymbol(VirShader,
                                        VIR_SYM_VARIABLE,
                                        nameId,
                                        VIR_Shader_GetTypeFromId(VirShader, typeId),
                                        gcmATTRIBUTE_isPerPatch(Attribute) ? VIR_STORAGE_PERPATCH_INPUT
                                                                            : VIR_STORAGE_INPUT,
                                        &symId);
        }
    }
    ON_ERROR2STATUS(virErrCode, "Failed to add attribute symbol");

    sym = VIR_Shader_GetSymFromId(VirShader, symId);

    sym->ioBlockIndex = IoBlockIndex;

    VIR_Symbol_SetPrecision(sym, _gcmConvPrecision2Vir(Attribute->precision));
    VIR_Symbol_SetTyQualifier(sym, VIR_TYQUAL_CONST);

    sym->flags = VIR_SYMFLAG_ENABLED |
                    (gcmATTRIBUTE_isTexture(Attribute)
                        ? VIR_SYMFLAG_ISTEXTURE
                        : 0) |
                    (gcmATTRIBUTE_packedAway(Attribute)
                        ? VIR_SYMFLAG_PACKEDAWAY
                        : 0) |
                    (gcmATTRIBUTE_alwaysUsed(Attribute)
                        ? VIR_SYMFLAG_ALWAYSUSED
                        : 0) |
                    (gcmATTRIBUTE_pointspriteTc(Attribute)
                        ? VIR_SYMFLAG_POINTSPRITE_TC
                        : 0) |
                    (gcmATTRIBUTE_isZWTexture(Attribute)
                        ? VIR_SYMFLAG_ZWTEXTURE
                        : 0) |
                    (gcmATTRIBUTE_isPosition(Attribute)
                        ? VIR_SYMFLAG_POSITION
                        : 0) |
                    (gcmATTRIBUTE_isInvariant(Attribute)
                        ? VIR_SYMFLAG_INVARIANT
                        : 0) |
                    (gcmATTRIBUTE_isPerVertexArray(Attribute)
                        ? VIR_SYMFLAG_ARRAYED_PER_VERTEX
                        : 0) |
                    (gcmATTRIBUTE_isPrecise(Attribute)
                        ? VIR_SYMFLAG_PRECISE
                        : 0) |
                    (gcmATTRIBUTE_enabled(Attribute)
                        ? 0
                        : VIR_SYMFLAG_UNUSED) |
                    (Attribute->shaderMode == gcSHADER_SHADER_FLAT
                        ? VIR_SYMFLAG_FLAT
                        : 0) |
                    (gcmATTRIBUTE_isStaticallyUsed(Attribute)
                        ? VIR_SYMFLAG_STATICALLY_USED
                        : 0) |
                    (gcmATTRIBUTE_isIOBlockMember(Attribute)
                        ? VIR_SYMFLAG_IS_IOBLOCK_MEMBER
                        : 0) |
                    (gcmATTRIBUTE_isInstanceMember(Attribute)
                        ? VIR_SYMFLAG_IS_INSTANCE_MEMBER
                        : 0) |
                    (gcmATTRIBUTE_isCentroid(Attribute)
                        ? VIR_SYMFLAG_ISCENTROID
                        : 0) |
                    (gcmATTRIBUTE_isSample(Attribute)
                        ? VIR_SYMFLAG_ISSAMPLE
                        : 0) |
                    (gcmATTRIBUTE_isCompilerGen(Attribute)
                        ? VIR_SYMFLAG_COMPILER_GEN
                        : 0) |
                    (gcmATTRIBUTE_isLocSetByDriver(Attribute)
                        ? VIR_SYMFLAG_LOC_SET_BY_DRIVER
                        : 0);

    sym->flagsExt = (Attribute->shaderMode == gcSHADER_SHADER_NOPERSPECTIVE
                        ? VIR_SYMFLAGEXT_NOPERSPECTIVE
                        : VIR_SYMFLAGEXT_NONE);

    if (sym->flags & VIR_SYMFLAG_ALWAYSUSED)
    {
        sym->flags &= ~VIR_SYMFLAG_UNUSED;
    }

    {
        /* set layout info */
        VIR_Symbol_SetLayoutQualifier(sym, VIR_LAYQUAL_NONE);
        VIR_Symbol_SetLocation(sym, Attribute->location);
    }
    sym->u2.tempIndex = attrVirRegCount;
    VirAttrIdArr[Index] = symId;

    /* create the virReg */
    gcTYPE_GetTypeInfo(Attribute->type, &components, &rows, 0);
    rows *= Attribute->arraySize;
    regFormat = gcGetFormatFromType(Attribute->type);
    for (j = 0; j < rows; j++, attrVirRegCount++)
    {
        VIR_Symbol *virRegSym;

        virRegId = _GetVirRegId(VirShader,
                                VirRegMapArr,
                                attrVirRegCount,
                                regFormat,
                                gcvFALSE,
                                components,
                                gcTYPE_IsTypePacked(Attribute->type),
                                VIR_Symbol_GetPrecision(sym));
        if (virRegId == VIR_INVALID_ID)
        {
            status = gcvSTATUS_INVALID_ARGUMENT;
            goto OnError;
        }
        gcmASSERT(VirRegMapArr[attrVirRegCount].virRegId != VIR_INVALID_ID);

        virRegSym = VIR_Shader_GetSymFromId(VirShader, virRegId);
        VIR_Symbol_SetVregVariable(virRegSym, sym);
    }
    /* set the shader flag for GS. the flags will be used in RA */
    if (VirShader->shaderKind == VIR_SHADER_GEOMETRY)
    {
        if (VIR_Symbol_GetName(sym) == VIR_NAME_PRIMITIVE_ID_IN)
        {
            VIR_Shader_SetFlag(VirShader, VIR_SHFLAG_HAS_PRIMITIVEID);
        }
        else if (VIR_Symbol_GetName(sym) == VIR_NAME_INVOCATION_ID)
        {
            VIR_Shader_SetFlag(VirShader, VIR_SHFLAG_HAS_INSTANCEID);
        }
    }

    if ((nameId == VIR_NAME_SAMPLE_ID || nameId == VIR_NAME_SAMPLE_POSITION) &&
        VIR_Symbol_HasFlag(sym, VIR_SYMFLAG_STATICALLY_USED) &&
        !VIR_Symbol_HasFlag(sym, VIR_SYMFLAG_COMPILER_GEN))
    {
        VIR_Shader_SetFlag(VirShader, VIR_SHFLAG_PS_SAMPLE_SHADING);
    }

    if (AttributeSymbol)
    {
        *AttributeSymbol = sym;
    }

    if (AttrVirRegCount)
    {
        *AttrVirRegCount = attrVirRegCount;
    }

OnError:
    return virErrCode;
}

static VSC_ErrCode
_ConvSSBlockMember(
    IN gcSHADER                 Shader,
    IN gcsSTORAGE_BLOCK         SSBlock,
    IN VIR_Shader*              VirShader,
    IN VIR_Type*                StructType,
    IN gctINT                   ChildIndex
    )
{
    gceSTATUS           status = gcvSTATUS_OK;
    VSC_ErrCode         virErrCode = VSC_ERR_NONE;
    VIR_NameId          fieldNameId;
    VIR_SymId           fieldSymId;
    VIR_TypeId          fieldTypeId;
    VIR_Type*           fieldType;
    gcVARIABLE          childVariable, structChildVariable;
    gctINT              structChildIndex;
    gctINT              arraySize;

    gcmONERROR(gcSHADER_GetVariable(Shader, (gctUINT)ChildIndex, &childVariable));

    /* Add the member name. */
    virErrCode = VIR_Shader_AddString(VirShader,
                                      GetVariableName(childVariable),
                                      &fieldNameId);
    if (virErrCode != VSC_ERR_NONE) return virErrCode;

    /* If this member is struct, fill this struct first. */
    if (isVariableStruct(childVariable))
    {
        virErrCode = VIR_Shader_AddStructType(VirShader,
                                              gcvFALSE,
                                              fieldNameId,
                                              gcvFALSE,
                                              &fieldTypeId);
        if (virErrCode != VSC_ERR_NONE) return virErrCode;

        fieldType = VIR_Shader_GetTypeFromId(VirShader, fieldTypeId);

        structChildIndex = GetVariableFirstChild(childVariable);

        while (structChildIndex != -1)
        {
            virErrCode = _ConvSSBlockMember(Shader,
                                            SSBlock,
                                            VirShader,
                                            fieldType,
                                            structChildIndex);

            gcmONERROR(gcSHADER_GetVariable(Shader, (gctUINT)structChildIndex, &structChildVariable));
            structChildIndex = GetVariableNextSibling(structChildVariable);
        }
    }
    else
    {
        /* Get the member type. */
        _ConvShaderTypeToVirTypeId(GetVariableType(childVariable), &fieldTypeId);
    }

    /* If this member is an array, create a new type. */
    arraySize = _GetTrueVariableArraySize(VirShader, childVariable, &fieldTypeId);
    if (GetVariableArrayLengthCount(childVariable) > 0)
    {
        virErrCode = VIR_Shader_AddArrayType(VirShader,
                                                fieldTypeId,
                                                arraySize,
                                                0,
                                                &fieldTypeId);
        if (virErrCode != VSC_ERR_NONE) return virErrCode;
    }

    /* Create the member symbol. */
    virErrCode = VIR_Shader_AddFieldSymbol(VirShader,
                                           fieldNameId,
                                           VIR_Shader_GetTypeFromId(VirShader, fieldTypeId),
                                           StructType,
                                           VIR_STORAGE_UNKNOWN,
                                           &fieldSymId);
    if (virErrCode != VSC_ERR_NONE) return virErrCode;

    /* Add member. */
    virErrCode = VIR_Type_AddField(VirShader,
                                   StructType,
                                   fieldSymId);
    if (virErrCode != VSC_ERR_NONE) return virErrCode;

OnError:
    return virErrCode;
}

static VSC_ErrCode
_ConvSSBlock2Vir(
    IN gcSHADER                 Shader,
    IN gcsSTORAGE_BLOCK         SSBlock,
    IN VIR_Shader*              VirShader,
    IN VIR_SymId*               VirUniformIdArr,
    IN OUT VIR_StorageBlock**   VirSSBlock
    )
{
    gceSTATUS           status = gcvSTATUS_OK;
    VSC_ErrCode         virErrCode = VSC_ERR_NONE;
    VIR_NameId          nameId;
    VIR_TypeId          typeId;
    VIR_Type*           ssboType;
    VIR_SymId           symId;
    VIR_Symbol*         sym;
    VIR_Symbol*         addressSym;
    VIR_StorageBlock*   virSSBlock;
    gctINT16            childIndex;
    gcUNIFORM           baseAddressUniform;
    gcVARIABLE          childVariable;

    /* Add ssbo name. */
    virErrCode = VIR_Shader_AddString(VirShader,
                                      GetSBName(SSBlock),
                                      &nameId);
    if (virErrCode != VSC_ERR_NONE) return virErrCode;

    /* Add a new type for this ssbo. */
    virErrCode = VIR_Shader_AddStructType(VirShader,
                                          gcvFALSE,
                                          nameId,
                                          gcvFALSE,
                                          &typeId);
    if (virErrCode != VSC_ERR_NONE) return virErrCode;

    /* Get the ssbo type. */
    ssboType = VIR_Shader_GetTypeFromId(VirShader, typeId);

    /* Add all members. */
    childIndex = GetIBIFirstChild(GetSBInterfaceBlockInfo(SSBlock));

    while (childIndex != -1)
    {
        virErrCode = _ConvSSBlockMember(Shader,
                                        SSBlock,
                                        VirShader,
                                        ssboType,
                                        childIndex);

        gcmONERROR(gcSHADER_GetVariable(Shader, (gctUINT)childIndex, &childVariable));
        childIndex = GetVariableNextSibling(childVariable);
    }

    /* If this SSBO is an array. */
    if (GetIBIArraySize(GetSBInterfaceBlockInfo(SSBlock)) > 0)
    {
        virErrCode = VIR_Shader_AddArrayType(VirShader,
                                             typeId,
                                             GetIBIArraySize(GetSBInterfaceBlockInfo(SSBlock)),
                                             0,
                                             &typeId);
        if (virErrCode != VSC_ERR_NONE) return virErrCode;
    }

    /* Create the symbol for this ssbo. */
    virErrCode = VIR_Shader_AddSymbol(VirShader,
                                      VIR_SYM_SBO,
                                      nameId,
                                      VIR_Shader_GetTypeFromId(VirShader, typeId),
                                      VIR_STORAGE_UNKNOWN,
                                      &symId);
    CHECK_ERROR(virErrCode, "Failed to add SSBO symbol");

    /* Fill symbol data. */
    sym = VIR_Shader_GetSymFromId(VirShader, symId);

    virSSBlock = sym->u2.sbo;
    gcmASSERT(virSSBlock);
    virSSBlock->sym = symId;
    virSSBlock->blockSize = GetSBBlockSize(SSBlock);
    VIR_IB_SetFlag(virSSBlock, _ConvIBFlag2Vir(GetSBInterfaceBlockInfo(SSBlock)));
    /* If this SBO is for shared variables, save its size for local memory size. */
    if (VIR_IB_GetFlags(virSSBlock) & VIR_IB_FOR_SHARED_VARIABLE)
    {
        VIR_Shader_SetLocalMemorySize(VirShader, GetSBBlockSize(SSBlock));
    }

    /* Convert the base address uniform. */
    virErrCode =_ConvShaderUniformIdx2Vir(Shader,
                                          (gctINT)GetSBIndex(SSBlock),
                                          (gctINT)GetSBIndex(SSBlock),
                                          -1,
                                          VirShader,
                                          gcvNULL,
                                          gcvNULL,
                                          &virSSBlock->baseAddr);
    if (virErrCode != VSC_ERR_NONE) return virErrCode;

    /* set base address uniform. */
    baseAddressUniform = Shader->uniforms[GetSBIndex(SSBlock)];
    addressSym = VIR_Shader_GetSymFromId(VirShader, virSSBlock->baseAddr);
    gcmASSERT(VirUniformIdArr[GetSBIndex(SSBlock)] == VIR_INVALID_ID);
    VirUniformIdArr[GetSBIndex(SSBlock)] = virSSBlock->baseAddr;
    if (isUniformStaticallyUsed(baseAddressUniform))
    {
        VIR_Symbol_SetFlag(addressSym, VIR_SYMFLAG_STATICALLY_USED);
    }

    /* set layout info */
    VIR_Symbol_SetLayoutQualifier(sym,
        _gcmConvMemoryLayout2Vir(GetUBMemoryLayout(SSBlock)));
    VIR_Symbol_SetLocation(sym, 0);
    VIR_Symbol_SetBinding(sym, GetSBBinding(SSBlock));

    if (VirSSBlock)
    {
        *VirSSBlock = virSSBlock;
    }

OnError:
    return virErrCode;
}

static VSC_ErrCode
_ConvIOBlock2Vir(
    IN gcSHADER             Shader,
    IN gcsIO_BLOCK          IOBlock,
    IN VIR_SymId*           VirAttrIdArr,
    IN VIR_SymId*           VirOutputIdArr,
    IN conv2VirsVirRegMap*  VirRegMapArr,
    IN gctUINT*             AttrVirRegCount,
    IN VIR_Shader*          VirShader,
    IN VSC_HW_CONFIG*       hwCfg,
    IN OUT VIR_IOBlock**    VirIOBlock
    )
{
    gceSTATUS           status = gcvSTATUS_OK;
    VSC_ErrCode         virErrCode = VSC_ERR_NONE;
    VIR_NameId          nameId;
    VIR_TypeId          structTypeId;
    VIR_SymId           symId;
    VIR_Symbol          *sym;
    VIR_IOBlock         *virIOBlock;
    gctINT32            childIndex;
    gcOUTPUT            output = gcvNULL;
    gcATTRIBUTE         attribute = gcvNULL;
    gctBOOL             hasInstanceName = GetSBInstanceNameLength(IOBlock) > 0;
    gctBOOL             isOutput = GetSVIIsOutput(GetSBShaderVarInfo(IOBlock));

    gcmHEADER_ARG("Shader=0x%x IOBlock=0x%x VirShader=0x%x",
                  Shader, IOBlock, VirShader);

    gcmASSERT(Shader);
    gcmASSERT(VirShader);
    gcmASSERT(IOBlock);

    status = _ConvBuiltinNameKindToVirNameId(GetSBNameLength(IOBlock),
                                             &nameId);

    if (status == gcvSTATUS_NOT_FOUND)
    {
        /* Create IO block name, it is "BlockName{.InstanceName}". */
        virErrCode = VIR_Shader_AddString(VirShader,
                                          IOBlock->name,
                                          &nameId);
        if (virErrCode != VSC_ERR_NONE) return virErrCode;
    }

    /* Create a struct type for IO block. */
    virErrCode = VIR_Shader_AddStructType(VirShader,
                                          gcvFALSE,
                                          nameId,
                                          gcvFALSE,
                                          &structTypeId);
    if (virErrCode != VSC_ERR_NONE) return virErrCode;

    /* Find its members. */
    childIndex = GetIBIFirstChild(GetSBInterfaceBlockInfo(IOBlock));
    while (childIndex != -1)
    {
        gctSTRING           fieldName;
        VIR_NameId          fieldNameId;
        VIR_SymId           fieldSymId;
        VIR_TypeId          fieldTypeId;

        /* Get the nameID and typeID of IO block member. */
        if (isOutput)
        {
            gcmONERROR(gcSHADER_GetOutput(Shader,
                                          (gctUINT)childIndex,
                                          &output));

            status = _ConvBuiltinNameKindToVirNameId(output->nameLength,
                                                     &fieldNameId);

            if (status == gcvSTATUS_NOT_FOUND)
            {
                if (hasInstanceName)
                {
                    fieldName = &output->name[GetSBInstanceNameLength(IOBlock) + 1];
                }
                else
                {
                    fieldName = output->name;
                }
                virErrCode = VIR_Shader_AddString(VirShader,
                                                  fieldName,
                                                  &fieldNameId);
                if (virErrCode != VSC_ERR_NONE) return virErrCode;
            }

            _ConvShaderTypeToVirTypeId(output->type, &fieldTypeId);
            childIndex = output->nextSibling;
        }
        else
        {
            gcmONERROR(gcSHADER_GetAttribute(Shader,
                                             (gctUINT)childIndex,
                                             &attribute));

            status = _ConvBuiltinNameKindToVirNameId(attribute->nameLength,
                                                     &fieldNameId);

            if (status == gcvSTATUS_NOT_FOUND)
            {
                if (hasInstanceName)
                {
                    fieldName = &attribute->name[GetSBInstanceNameLength(IOBlock) + 1];
                }
                else
                {
                    fieldName = attribute->name;
                }
                virErrCode = VIR_Shader_AddString(VirShader,
                                                  fieldName,
                                                  &fieldNameId);
                if (virErrCode != VSC_ERR_NONE) return virErrCode;
            }

            _ConvShaderTypeToVirTypeId(attribute->type, &fieldTypeId);
            childIndex = attribute->nextSibling;
        }

        /* Create the filed symbol. */
        virErrCode = VIR_Shader_AddFieldSymbol(VirShader,
                                               fieldNameId,
                                               VIR_Shader_GetTypeFromId(VirShader, fieldTypeId),
                                               VIR_Shader_GetTypeFromId(VirShader, structTypeId),
                                               VIR_STORAGE_UNKNOWN,
                                               &fieldSymId);
        if (virErrCode != VSC_ERR_NONE) return virErrCode;

        virErrCode = VIR_Type_AddField(VirShader,
                                       VIR_Shader_GetTypeFromId(VirShader, structTypeId),
                                       fieldSymId);
        if (virErrCode != VSC_ERR_NONE) return virErrCode;
    }

    /* If this struct is an array. */
    if (GetIBIArraySize(GetSBInterfaceBlockInfo(IOBlock)) > 0)
    {
        virErrCode = VIR_Shader_AddArrayType(VirShader,
                                             structTypeId,
                                             GetIBIArraySize(GetSBInterfaceBlockInfo(IOBlock)),
                                             0,
                                             &structTypeId);
        if (virErrCode != VSC_ERR_NONE) return virErrCode;
    }

    /* Create the symbol for this IO block. */
    virErrCode = VIR_Shader_AddSymbol(VirShader,
                                      VIR_SYM_IOBLOCK,
                                      nameId,
                                      VIR_Shader_GetTypeFromId(VirShader, structTypeId),
                                      VIR_STORAGE_UNKNOWN,
                                      &symId);
    CHECK_ERROR(virErrCode, "Failed to add IO block symbol");

    /* Fill symbol data. */
    sym = VIR_Shader_GetSymFromId(VirShader, symId);
    VIR_Symbol_SetLocation(sym, GetIBILocation(GetSBInterfaceBlockInfo(IOBlock)));

    /* Fill the IO block data. */
    virIOBlock = sym->u2.ioBlock;
    gcmASSERT(virIOBlock);
    VIR_IOBLOCK_SetSymId(virIOBlock, symId);
    VIR_IOBLOCK_SetBlockNameLength(virIOBlock, GetSBNameLength(IOBlock));
    VIR_IOBLOCK_SetInstanceNameLength(virIOBlock, GetSBInstanceNameLength(IOBlock));
    if (isOutput)
    {
        VIR_IOBLOCK_SetStorage(virIOBlock, VIR_STORAGE_OUTPUT);
    }
    else
    {
        VIR_IOBLOCK_SetStorage(virIOBlock, VIR_STORAGE_INPUT);
    }
    VIR_IB_SetFlag(virIOBlock, _ConvIBFlag2Vir(GetSBInterfaceBlockInfo(IOBlock)));

    /* Map its members to this IOBlock. */
    childIndex = GetIBIFirstChild(GetSBInterfaceBlockInfo(IOBlock));
    while (childIndex != -1)
    {
        /* Get the nameID and typeID of IO block member. */
        if (isOutput)
        {
            gcmONERROR(gcSHADER_GetOutput(Shader,
                                          (gctUINT)childIndex,
                                          &output));

            virErrCode = _ConvShaderOutput2Vir(Shader,
                                               (gctUINT32 *)&childIndex,
                                               output,
                                               VirOutputIdArr,
                                               VirRegMapArr,
                                               VirShader,
                                               symId,
                                               gcvNULL);
            if (virErrCode != VSC_ERR_NONE) return virErrCode;

            childIndex = output->nextSibling;
        }
        else
        {
            gcmONERROR(gcSHADER_GetAttribute(Shader,
                                             (gctUINT)childIndex,
                                             &attribute));

            virErrCode = _ConvShaderAttribute2Vir(Shader,
                                                  (gctUINT32)childIndex,
                                                  attribute,
                                                  VirAttrIdArr,
                                                  VirRegMapArr,
                                                  AttrVirRegCount,
                                                  VirShader,
                                                  symId,
                                                  hwCfg,
                                                  gcvNULL);
            if (virErrCode != VSC_ERR_NONE) return virErrCode;

            childIndex = attribute->nextSibling;
        }
    }

    if (VirIOBlock)
    {
        *VirIOBlock = virIOBlock;
    }

OnError:
    gcmFOOTER_NO();
    return virErrCode;
}

#define _gcdVirIsUnknownBasicTypeId(TypeId) ((TypeId) == VIR_TYPE_UNKNOWN)

static VIR_VirRegId
_GetVirRegId(
    VIR_Shader *VirShader,
    conv2VirsVirRegMap *VirRegMapArr,
    gctINT Index,
    gcSL_FORMAT Format,
    gctBOOL IsFromSampler,
    gctUINT Components,
    gctBOOL Packed,
    VIR_Precision precision
    )
{
    VIR_Symbol *sym;
    VIR_TypeId typeId;
    VIR_VirRegId virRegId = VIR_INVALID_ID;
    VSC_ErrCode virErrCode = VSC_ERR_NONE;

    virRegId = VirRegMapArr[Index].virRegId;
    if(virRegId == VIR_INVALID_ID) {
        virErrCode = VIR_Shader_AddSymbol(VirShader,
                                          VIR_SYM_VIRREG,
                                          Index,
                                          VIR_Shader_GetTypeFromId(VirShader,
                                          _ConvScalarFormatToVirVectorTypeId(Format, Components, Packed)),
                                          VIR_STORAGE_UNKNOWN,
                                          &virRegId);
        CHECK_ERROR(virErrCode, "Failed to add virtual register symbol");

        if(virErrCode == VSC_ERR_NONE) {
            sym = VIR_Shader_GetSymFromId(VirShader, virRegId);
            VirRegMapArr[Index].virRegId = virRegId;
            /* set the precision */
            VIR_Symbol_SetPrecision(sym, precision);
        }

    }
    else {
        sym = VIR_Shader_GetSymFromId(VirShader, virRegId);
        if(VIR_Symbol_GetPrecision(sym) == VIR_PRECISION_ANY)
        {
            VIR_Symbol_SetPrecision(sym, precision);
        }
        gcmASSERT(precision == VIR_PRECISION_ANY || VIR_Symbol_GetPrecision(sym) == precision
                  || !VIR_Shader_IsFS(VirShader) || VIR_Shader_IsDesktopGL(VirShader));
        /* need to update type of function arguments */
        if(_gcdVirIsUnknownBasicTypeId(VIR_Symbol_GetTypeId(sym)) && VIR_Symbol_GetVregVariable(sym)) {
            VIR_Symbol *variable;

            variable = VIR_Symbol_GetVregVariable(sym);
            if(_gcdVirIsUnknownBasicTypeId(VIR_Symbol_GetTypeId(variable))) {
                gcmASSERT(VirRegMapArr[Index].components);
                typeId = _ConvScalarFormatToVirVectorTypeId(Format,
                                                            VirRegMapArr[Index].components,
                                                            VirRegMapArr[Index].packed);
                VIR_Symbol_SetTypeId(variable, typeId);
            }
            else {
                VIR_Type * type;

                typeId = VIR_Symbol_GetTypeId(variable);
                type = VIR_Shader_GetTypeFromId(VirShader, typeId);
                while (VIR_Type_isArray(type))
                {
                   type = VIR_Shader_GetTypeFromId(VirShader, VIR_Type_GetBaseTypeId(type));
                }
                typeId = VIR_Type_GetBaseTypeId(type);
            }

            gcmASSERT(VIR_TypeId_isPrimitive(typeId));
            if (IsFromSampler)
            {
                gcmASSERT(VirRegMapArr[Index].components);
                typeId = _ConvScalarFormatToVirVectorTypeId(Format,
                                                            VirRegMapArr[Index].components,
                                                            VirRegMapArr[Index].packed);
            }
        }
        else if (!VIR_Symbol_GetVregVariable(sym)) {
            if(Components > VIR_GetTypeComponents(VIR_Symbol_GetTypeId(sym)))
            {
                typeId = _ConvScalarFormatToVirVectorTypeId(Format,
                                                            Components,
                                                            Packed);
                VIR_Symbol_SetTypeId(sym, typeId);
            }
        }
    }
    return virRegId;
}

static VSC_ErrCode
_ConvSourceConst2VirScalerConstVal(
    gctUINT Index,
    gctUINT Indexed,
    VIR_ScalarConstVal *VirScalarConstValue
    )
{
    gcmASSERT(VirScalarConstValue);
    VirScalarConstValue->uValue = (Index & 0xFFFF) | (Indexed << 16);
    return VSC_ERR_NONE;
}

static VSC_ErrCode
_FindInstLabel(
    IN gcSHADER Shader,
    IN gctUINT Label,
    IN conv2VirsInstMap *VirInstMap,
    IN VIR_Shader *VirShader,
    OUT VIR_LabelId * VirLabelId
    )
{
    VSC_ErrCode virErrCode = VSC_ERR_NONE;
    gctUINT offset = 0;
    gctCHAR labelName[20];

    gcmASSERT(Label < Shader->codeCount);

    if(VirInstMap[Label].labelId == VIR_INVALID_ID) {
        gcmVERIFY_OK(gcoOS_PrintStrSafe(labelName,
                                        20,
                                        &offset,
                                        "%u",
                                        Label));

        gcmASSERT(VirInstMap[Label].inFunction);
        virErrCode = VIR_Function_AddLabel(VirInstMap[Label].inFunction,
                                           labelName,
                                           &(VirInstMap[Label].labelId));
        if(virErrCode != VSC_ERR_NONE) return virErrCode;

        if(VirInstMap[Label].inst) {
            VIR_Label *virLabel;
            VIR_Instruction *labelInst;

            virErrCode = VIR_Function_AddInstructionBefore(VirInstMap[Label].inFunction,
                                                           VIR_OP_LABEL,
                                                           VIR_TYPE_UNKNOWN,
                                                           VirInstMap[Label].inst,
                                                           gcvTRUE,
                                                           &labelInst);
            if(virErrCode != VSC_ERR_NONE) return virErrCode;
            virLabel = VIR_GetLabelFromId(VirInstMap[Label].inFunction,
                                          VirInstMap[Label].labelId);
            virLabel->defined = labelInst;
            VIR_Operand_SetLabel(labelInst->dest, virLabel);
        }
    }
    *VirLabelId = VirInstMap[Label].labelId;

    return virErrCode;
}

/* get the type's component count:
 *   for primitive type: it is from preset array
 *   for composite types: get its base type's component count
 */
gctUINT
_gcGetTypeComponentCount(
    IN VIR_Shader *     VirShader,
    IN VIR_Type *       Type,
    IN gctBOOL          IsLogicalComponentCount)
{
    if (VIR_Type_GetIndex(Type) <= VIR_TYPE_LAST_PRIMITIVETYPE)
    {
        if (IsLogicalComponentCount)
        {
            return VIR_GetTypeLogicalComponents(VIR_Type_GetIndex(Type));
        }
        else
        {
        return VIR_GetTypeComponents(VIR_Type_GetIndex(Type));
    }
    }
    else {
        VIR_Type * baseType = VIR_Shader_GetTypeFromId(VirShader,
                                     VIR_Type_GetBaseTypeId(Type));
        return _gcGetTypeComponentCount(VirShader, baseType, IsLogicalComponentCount);
    }
}


static gctUINT
_gcComputeSymComponentCount(
    IN VIR_Symbol *Sym,
    IN gctUINT Opcode,
    IN gcSL_ENABLE Enable,
    IN gcSL_SWIZZLE Swizzle
    )
{
    gctINT components;

    components = _virOpcodeMap[Opcode].srcComponents;
    switch(components) {
    case -1:
        if ((VIR_Symbol_isVreg((Sym)) && !VIR_Symbol_GetVregVariable(Sym)))
        {
            components = 4;
        }
        else if (VIR_Symbol_GetTypeId(Sym) <= VIR_TYPE_LAST_PRIMITIVETYPE)
        {
            components = VIR_GetTypeComponents(VIR_Symbol_GetTypeId(Sym));
        }
        else
        {
            /* no meaning to get composite type's component count */
            components = 0;
        }
        break;

    case 0:
        if(Swizzle == gcSL_SWIZZLE_X || Swizzle == gcSL_SWIZZLE_XXXX ||
           Swizzle == gcSL_SWIZZLE_Y || Swizzle == gcSL_SWIZZLE_YYYY ||
           Swizzle == gcSL_SWIZZLE_Z || Swizzle == gcSL_SWIZZLE_ZZZZ ||
           Swizzle == gcSL_SWIZZLE_W || Swizzle == gcSL_SWIZZLE_WWWW)
        {
            components = 1;
        }
        else
        {
            components = (gctINT) _GetEnableComponentCount(Enable);
        }
        break;

    default:
        break;
    }
    return (gctUINT)components;
}

static void
_updateOperandTypeByVariable(
    IN VIR_OpCode          OpCode,
    IN VIR_Operand *       Operand,
    IN VIR_Symbol  *       VarSymbol
    )
{
    VIR_TypeId   opndTypeId = VIR_Operand_GetTypeId(Operand);
    VIR_TypeId   varTypeId  = VIR_Type_GetIndex(VIR_Symbol_GetType(VarSymbol));
    /* adjust operand's type according to coresponding variable's type */
    gcmASSERT(VIR_TypeId_isPrimitive(opndTypeId));
    if (opndTypeId == varTypeId)
        return;

    if (VIR_TypeId_isPrimitive(varTypeId) &&
         ((VIR_TypeId_isInteger(varTypeId) && VIR_TypeId_isInteger(opndTypeId)) ||
          (VIR_TypeId_isFloat(varTypeId)   && VIR_TypeId_isFloat(opndTypeId))     ) &&
         VIR_TypeId_isPacked(varTypeId))
    {
        VIR_Operand_SetTypeId(Operand, VIR_GetTypeRows(varTypeId) > 1 ?
                                        VIR_GetTypeRowType(varTypeId) : varTypeId);
    }
    VIR_Operand_SetBigEndian(Operand, VIR_Symbol_GetBigEndian(VarSymbol));
}

static VSC_ErrCode
_ConvSource2VirOperand(
    IN  gcSHADER            Shader,
    IN  VIR_Shader *        VirShader,
    IN  VIR_Function *      VirFunction,
    IN  gcSL_INSTRUCTION    Code,
    IN  gctUINT32           CodeIndex,
    IN  gctINT              SrcNo,
    IN  conv2VirsVirRegMap *VirRegMapArr,
    IN  VIR_SymId *         VirAttrIdArr,
    IN  VIR_SymId *         VirOutIdArr,
    IN  VIR_SymId *         VirUniformIdArr,
    IN  VIR_Instruction *   VirInst,
    OUT VIR_Operand *       VirSrc
    )
{
    VSC_ErrCode         virErrCode = VSC_ERR_NONE;
    gctSOURCE_t         source  = (SrcNo == 0) ? Code->source0
                                               : Code->source1;
    gctUINT             index = (SrcNo == 0) ? Code->source0Index
                                             : Code->source1Index;
    gctUINT             indexed = (SrcNo == 0) ? Code->source0Indexed
                                               : Code->source1Indexed;
    gctUINT             swizzle = gcmSL_SOURCE_GET(source, Swizzle);
    gcSL_TYPE           srcType = gcmSL_SOURCE_GET(source, Type);
    gcSL_FORMAT         srcFormat = gcmSL_SOURCE_GET(source, Format);
    gcSL_INDEXED_LEVEL  srcIndexedLevel = (gcSL_INDEXED_LEVEL)gcmSL_SOURCE_GET(source, Indexed_Level);
    VIR_Precision       srcPrecision = _gcmConvPrecision2Vir(gcmSL_SOURCE_GET(source, Precision));
    VIR_SymId           virRegId;
    VIR_SymId           symId;
    VIR_Symbol          *sym, *varSym = gcvNULL;
    gctUINT             constIdx = 0;
    VIR_ScalarConstVal  virScalarVal;
    gctUINT             effectiveIndex;
    gctUINT             components;
    gctBOOL             packed = gcvFALSE;
    VIR_TypeId          typeId;
    gcSL_ENABLE         enable;
    gctUINT             mode;
    gcSL_OPCODE         opcode;
    gctBOOL             packedBasicType;

    enable = gcmSL_TARGET_GET(Code->temp, Enable);
    opcode = gcmSL_OPCODE_GET(Code->opcode, Opcode);
    VIR_Operand_SetPrecision(VirSrc, srcPrecision);

    packedBasicType = (gcmOPT_oclPackedBasicType() || gcShaderHasVivVxExtension(Shader));

    switch(srcType) {
    case gcSL_ATTRIBUTE:
    case gcSL_OUTPUT:
        effectiveIndex = gcmSL_INDEX_GET(index, Index);
        constIdx = gcmSL_INDEX_GET(index, ConstValue);
        if (srcType == gcSL_ATTRIBUTE)
        {
            gcmASSERT(effectiveIndex < Shader->attributeCount);
            symId = VirAttrIdArr[effectiveIndex];
        }
        else
        {
            gcmASSERT(effectiveIndex < Shader->outputCount);
            symId = VirOutIdArr[effectiveIndex];
        }
        sym = VIR_Shader_GetSymFromId(VirShader, symId);

        /* After adjusting precision, the operand precision may be changed, so we may hit this assertion in recompiler. */

        components = gcmSL_SOURCE_GET(source, PackedComponents);
        packed = gcvFALSE;
        if(components) {
            if(packedBasicType) {
                packed = gcvTRUE;
            }
        }
        else {
            components = _gcComputeSymComponentCount(sym, opcode, enable, swizzle);
        }
        typeId = _ConvScalarFormatToVirVectorTypeId(srcFormat,
                                                    components,
                                                    packed);

        mode = gcmSL_SOURCE_GET(source, Indexed);
        if (mode != gcSL_NOT_INDEXED) {
            VIR_VirRegId target;
            VIR_VirRegId indexId;

            target = VirRegMapArr[-(SrcNo + 1)].virRegId;
            if (target == VIR_INVALID_ID)
            {
                gcmASSERT(gcvFALSE);
            }

            gcmASSERT(indexed < Shader->_tempRegCount);
            indexId = _GetVirRegId(VirShader,
                                   VirRegMapArr,
                                   indexed,
                                   srcFormat,
                                   gcvFALSE,
                                   1,
                                   gcvFALSE,
                                   gcSHADER_PRECISION_ANY);

            if (!VIR_Type_isArray(VIR_Symbol_GetType(sym)))
            {
                typeId = VIR_Symbol_GetTypeId(sym);
            }

            VIR_Operand_SetSym(VirSrc, sym);
            VIR_Operand_SetTypeId(VirSrc, typeId);
            VIR_Operand_SetOpKind(VirSrc, VIR_OPND_SYMBOL);
            VIR_Operand_SetMatrixConstIndex(VirSrc, constIdx);

            VIR_Operand_SetRelIndexing(VirSrc, indexId);
            VIR_Operand_SetRelAddrMode(VirSrc, mode);
            VIR_Operand_SetRelAddrLevel(VirSrc, srcIndexedLevel);
        }
        else{
            if(indexed > 0)
            {
                VIR_Operand_SetRelIndexingImmed(VirSrc, indexed);
            }
            VIR_Operand_SetSym(VirSrc, sym);
            VIR_Operand_SetTypeId(VirSrc, typeId);
            VIR_Operand_SetOpKind(VirSrc, VIR_OPND_SYMBOL);
            VIR_Operand_SetMatrixConstIndex(VirSrc, constIdx);
        }

         /* change operand's precision based on symbol */
        VIR_Operand_SetPrecision(VirSrc, srcPrecision);

        VIR_Operand_SetSwizzle(VirSrc, _ConvSwizzle2Vir(swizzle));
        VIR_Operand_SetBigEndian(VirSrc, VIR_Symbol_GetBigEndian(sym));
        break;

    case gcSL_UNIFORM:
        effectiveIndex = gcmSL_INDEX_GET(index, Index);
        constIdx = gcmSL_INDEX_GET(index, ConstValue);

        gcmASSERT(effectiveIndex < Shader->uniformCount);
        symId = VirUniformIdArr[effectiveIndex];
        sym = VIR_Shader_GetSymFromId(VirShader, symId);
        if (srcPrecision != VIR_Symbol_GetPrecision(sym) &&
            srcPrecision == VIR_PRECISION_DEFAULT)
        {
            srcPrecision = VIR_Symbol_GetPrecision(sym);
        }
        gcmASSERT(srcPrecision == VIR_Symbol_GetPrecision(sym));
        if(VIR_Symbol_GetKind(sym) == VIR_SYM_SAMPLER) {
            typeId = VIR_Symbol_GetTypeId(sym);
        }
        else {
            components = gcmSL_SOURCE_GET(source, PackedComponents);
            packed = gcvFALSE;
            if(components) {
                if(packedBasicType) {
                    packed = gcvTRUE;
                }
            }
            else {
                components = _gcComputeSymComponentCount(sym, opcode, enable, swizzle);
            }
            typeId = _ConvScalarFormatToVirVectorTypeId(srcFormat,
                                                        components,
                                                        packed);
        }

        mode = gcmSL_SOURCE_GET(source, Indexed);
        if (mode != gcSL_NOT_INDEXED) {
            VIR_VirRegId target;
            VIR_VirRegId indexId;

            target = VirRegMapArr[-(SrcNo + 1)].virRegId;
            if (target == VIR_INVALID_ID)
            {
                gcmASSERT(gcvFALSE);
            }

            gcmASSERT(indexed < Shader->_tempRegCount);
            indexId = _GetVirRegId(VirShader,
                VirRegMapArr,
                indexed,
                srcFormat,
                gcvFALSE,
                1,
                gcvFALSE,
                VIR_PRECISION_ANY);

            VIR_Operand_SetSym(VirSrc, sym);
            VIR_Operand_SetTypeId(VirSrc, typeId);
            VIR_Operand_SetOpKind(VirSrc, VIR_OPND_SYMBOL);
            VIR_Operand_SetMatrixConstIndex(VirSrc, constIdx);

            VIR_Operand_SetRelIndexing(VirSrc, indexId);
            VIR_Operand_SetRelAddrMode(VirSrc, mode);
            VIR_Operand_SetRelAddrLevel(VirSrc, srcIndexedLevel);
        }
        else {
            if (VIR_Shader_IsCL(VirShader))
            {
                VIR_Type *symType = VIR_Symbol_GetType(sym);
                VIR_TypeId typeId = VIR_Type_GetBaseTypeId(symType);

                if (VIR_Type_isArray(symType) ||
                    typeId == VIR_TYPE_INT64 ||
                    typeId == VIR_TYPE_UINT64)
                {
                    gctUINT rows = VIR_GetTypeRows(typeId);
                    gctINT totalIdx = indexed + constIdx;

                    constIdx = totalIdx % rows;
                    indexed = totalIdx - constIdx;
                }
            }

            if(indexed > 0)
            {
                VIR_Operand_SetRelIndexingImmed(VirSrc, indexed);
            }
            VIR_Operand_SetSym(VirSrc, sym);
            VIR_Operand_SetTypeId(VirSrc, typeId);
            VIR_Operand_SetOpKind(VirSrc, VIR_OPND_SYMBOL);
            VIR_Operand_SetMatrixConstIndex(VirSrc, constIdx);
        }
        VIR_Operand_SetSwizzle(VirSrc, _ConvSwizzle2Vir(swizzle));
        VIR_Operand_SetPrecision(VirSrc, srcPrecision);
        VIR_Operand_SetBigEndian(VirSrc, VIR_Symbol_GetBigEndian(sym));
        break;

    case gcSL_SAMPLER:
        {
            VIR_SymId baseSamplerId = VIR_Shader_GetBaseSamplerId(VirShader);
            VIR_Symbol *baseSamplerSym = gcvNULL;
            VIR_Symbol *argumentSym = gcvNULL;
            VIR_VirRegId indexId;

            mode = gcmSL_SOURCE_GET(source, Indexed);

            /* Create a symbol to hold the base sampler. */
            if (baseSamplerId == VIR_INVALID_ID)
            {
                VIR_NameId nameId;
                VIR_Uniform *baseSampler = gcvNULL;

                virErrCode = VIR_Shader_AddString(VirShader,
                                                  "#BaseSamplerSym",
                                                  &nameId);
                if(virErrCode != VSC_ERR_NONE) return virErrCode;

                virErrCode = VIR_Shader_AddSymbol(VirShader,
                                                  VIR_SYM_SAMPLER,
                                                  nameId,
                                                  VIR_Shader_GetTypeFromId(VirShader, VIR_TYPE_VIV_GENERIC_GL_SAMPLER),
                                                  VIR_STORAGE_UNKNOWN,
                                                  &baseSamplerId);
                if(virErrCode != VSC_ERR_NONE) return virErrCode;

                VIR_Shader_SetBaseSamplerId(VirShader, baseSamplerId);

                /* Set the symbol. */
                baseSamplerSym = VIR_Shader_GetSymFromId(VirShader, baseSamplerId);
                VIR_Symbol_SetPrecision(baseSamplerSym, VIR_PRECISION_ANY);
                VIR_Symbol_SetAddrSpace(baseSamplerSym, VIR_AS_CONSTANT);
                VIR_Symbol_SetTyQualifier(baseSamplerSym, VIR_TYQUAL_CONST);
                VIR_Symbol_SetFlag(baseSamplerSym, VIR_SYMFLAG_COMPILER_GEN);

                /* Set the uniform. */
                baseSampler = baseSamplerSym->u2.uniform;
                baseSampler->sym = baseSamplerId;
            }
            else
            {
                baseSamplerSym = VIR_Shader_GetSymFromId(VirShader, baseSamplerId);
            }

            gcmASSERT(mode != gcSL_NOT_INDEXED);

            indexId = _GetVirRegId(VirShader,
                    VirRegMapArr,
                    indexed,
                    (VirShader->clientApiVersion == gcvAPI_OPENGL_ES30) ? gcSL_INT32 : gcSL_FLOAT,
                    gcvTRUE,
                    1,
                    gcvFALSE,
                    srcPrecision);

            argumentSym = VIR_Shader_GetSymFromId(VirShader, VirRegMapArr[indexed].virRegId);

            VIR_Operand_SetRelIndexing(VirSrc, indexId);
            VIR_Operand_SetRelAddrMode(VirSrc, mode);
            VIR_Operand_SetRelAddrLevel(VirSrc, srcIndexedLevel);
            VIR_Operand_SetSamplerIndexing(VirSrc, VirShader, baseSamplerSym);
            VIR_Operand_SetSwizzle(VirSrc, _ConvSwizzle2Vir(swizzle));

            if (VIR_Symbol_GetVregVariable(argumentSym))
            {
                VIR_Symbol* vregvar = VIR_Symbol_GetVregVariable(argumentSym);

                VIR_Operand_SetTypeId(VirSrc, VIR_Type_GetBaseTypeId(VIR_Symbol_GetType(vregvar)));
            }
            else
            {
                VIR_Operand_SetTypeId(VirSrc, VIR_TYPE_VIV_GENERIC_GL_SAMPLER);
            }

            VIR_Operand_SetPrecision(VirSrc, VIR_Symbol_GetPrecision(argumentSym));
            break;
        }
    case gcSL_CONSTANT:
        virErrCode = _ConvSourceConst2VirScalerConstVal(index,
                                                        indexed,
                                                        &virScalarVal);
        if(virErrCode != VSC_ERR_NONE) return virErrCode;

        if (VIR_Inst_GetOpcode(VirInst) == VIR_OP_INTRINSIC && SrcNo == 0)
        {
            VIR_Operand_SetIntrinsic(VirSrc, (VIR_IntrinsicsKind)virScalarVal.uValue);
        }
        else
        {
            VIR_Operand_SetImmediate(VirSrc,
                                     _virFormatTypeId[srcFormat],
                                     virScalarVal);
        }
        break;

    case gcSL_TEMP:
        gcmASSERT(index < Shader->_tempRegCount);
        constIdx = gcmSL_INDEX_GET(index, ConstValue);
        components = gcmSL_SOURCE_GET(source, PackedComponents);

        virRegId = _GetVirRegId(VirShader,
                                VirRegMapArr,
                                index,
                                srcFormat,
                                gcvFALSE,
                                components ? components : 4,
                                gcvFALSE,
                                srcPrecision);
        sym = VIR_Shader_GetSymFromId(VirShader, virRegId);
        packed = gcvFALSE;
        if(components) {
            if(packedBasicType) {
                packed = gcvTRUE;
            }
        }
        else {
            components = _gcComputeSymComponentCount(sym, opcode, enable, swizzle);
        }
        typeId = _ConvScalarFormatToVirVectorTypeId(srcFormat, components, packed);

        /* update temp variable type if src operand using more components, like following case
         * virreg type of "temp(12)" is created according to enable component which is 1 but the usage is used as vec4
         * 020: ADD                hp temp(12).hp.x,...
         * 022: ADD                hp temp(12).hp.y,...
         * 024: ADD                hp temp(12).hp.z,...
         * 026: ADD                hp temp(12).hp.w,...
         * 027: MOV                ..., vec4 hp  temp(12).hp.yxzw
         */
        if (VIR_Symbol_isVreg(sym) && (components > VIR_Symbol_GetComponents(sym)))
        {
            VIR_Symbol_SetTypeId(sym, typeId);
        }

        /* change operand's precision based on symbol */
        varSym = VIR_Symbol_GetVregVariable(sym);
        if (varSym)
        {
            gcmASSERT(VIR_Symbol_GetPrecision(sym) == VIR_Symbol_GetPrecision(varSym) ||
                      VIR_Symbol_GetPrecision(varSym) == VIR_PRECISION_ANY ||
                      !VIR_Shader_IsFS(VirShader) ||
                      VIR_Shader_IsDesktopGL(VirShader));
        }
        VIR_Operand_SetPrecision(VirSrc, srcPrecision);

        mode = gcmSL_SOURCE_GET(source, Indexed);
        if (mode != gcSL_NOT_INDEXED) {
            VIR_VirRegId target;
            VIR_VirRegId indexId;

            target = VirRegMapArr[-(SrcNo + 1)].virRegId;
            if (target == VIR_INVALID_ID)
            {
                gcmASSERT(gcvFALSE);
            }

            gcmASSERT(indexed < Shader->_tempRegCount);
            indexId = _GetVirRegId(VirShader,
                                   VirRegMapArr,
                                   indexed,
                                   srcFormat,
                                   gcvFALSE,
                                   1,
                                   gcvFALSE,
                                   VIR_PRECISION_ANY);

            if (!VIR_Type_isArray(VIR_Symbol_GetType(sym)))
            {
                typeId = VIR_Symbol_GetTypeId(sym);
            }

            VIR_Operand_SetSym(VirSrc, sym);
            VIR_Operand_SetTypeId(VirSrc, typeId);
            VIR_Operand_SetOpKind(VirSrc, VIR_OPND_SYMBOL);
            VIR_Operand_SetMatrixConstIndex(VirSrc, constIdx);

            VIR_Operand_SetRelIndexing(VirSrc, indexId);
            VIR_Operand_SetRelAddrMode(VirSrc, mode);
            VIR_Operand_SetRelAddrLevel(VirSrc, srcIndexedLevel);
        }
        else {
            if(indexed > 0)
            {
                VIR_Operand_SetRelIndexingImmed(VirSrc, indexed);
            }
            VIR_Operand_SetTempRegister(VirSrc,
                                        VirFunction,
                                        virRegId,
                                        typeId);
            VIR_Operand_SetMatrixConstIndex(VirSrc, constIdx);
        }
        if (varSym)
        {
            _updateOperandTypeByVariable(VIR_Inst_GetOpcode(VirInst), VirSrc, varSym);
        }
        VIR_Operand_SetSwizzle(VirSrc, _ConvSwizzle2Vir(swizzle));
        break;

    default:
        break;
    }

    return virErrCode;
}

static VSC_ErrCode
_ConvTexLdForShadow(
    IN  gcSHADER            Shader,
    IN  gcSL_INSTRUCTION    Code,
    IN  gcSL_OPCODE *       NewOpcode
    )
{
    gceSTATUS           status = gcvSTATUS_OK;
    VSC_ErrCode         virErrCode = VSC_ERR_NONE;
    gcSL_TYPE           srcType = gcmSL_SOURCE_GET(Code->source0, Type);
    gcUNIFORM           uniform = gcvNULL;
    gcVARIABLE          variable = gcvNULL;

    /*
    ** If the source type is UNIFORM, then we get the sampler type from uniform;
    ** if the source type is SAMPLER, then we get the sampler type from the function argument.
    */
    if (srcType == gcSL_UNIFORM)
    {
        gcSHADER_GetUniform(Shader, gcmSL_INDEX_GET(Code->source0Index, Index), &uniform);
        gcmASSERT(uniform);
    }
    else if (srcType == gcSL_SAMPLER)
    {
        gctUINT32 i, j;

        for (i = 0; i < Shader->functionCount; i++)
        {
            gcFUNCTION function = Shader->functions[i];

            if (function == gcvNULL) continue;

            for (j = 0; j < function->argumentCount; j++)
            {
                gcsFUNCTION_ARGUMENT_PTR argument = &function->arguments[j];

                if (argument == gcvNULL ||
                    argument->qualifier != gcvFUNCTION_INPUT ||
                    (gctINT16)argument->variableIndex == -1 ||
                    argument->index != Code->source0Indexed)
                {
                    continue;
                }
                status = gcSHADER_GetVariable(Shader,
                                              argument->variableIndex,
                                              &variable);
                if (gcmIS_ERROR(status))
                {
                    gcmASSERT(gcvFALSE);
                    return virErrCode;
                }
                break;
            }
            if (variable != gcvNULL) break;
        }
    }

    if ((uniform &&
         (isUniformBasicType(uniform)) &&
         isShadowSampler(uniform->u.type) &&
         !UniformHasFlag(uniform, gcvUNIFORM_FLAG_IS_DEPTH_COMPARISON))
        ||
        (variable && isShadowSampler(GetVariableType(variable))))
    {
        if (*NewOpcode == gcSL_TEXLD)
        {
            *NewOpcode = gcSL_TEXLDPCF;
        }
        else
        {
            gcmASSERT(*NewOpcode == gcSL_TEXLDPROJ);
            *NewOpcode = gcSL_TEXLDPCFPROJ;
        }
    }

    return virErrCode;
}

#define MAP_INSTRINSIC(Name, Op, Src2Add)  {Name, sizeof(Name)-1, VIR_OP_##Op, Src2Add, -1}
#define MAP_INSTRINSIC_COMP(Name, Op, Src2Add, Components)  {Name, sizeof(Name)-1, VIR_OP_##Op, Src2Add, Components}

typedef struct _IntrinsicNameMap IntrinsicNameMap;
struct _IntrinsicNameMap
{
    gctSTRING       name;
    gctINT          nameLen;
    VIR_OpCode      virOpcode;
    gctINT          srcToAdd;           /* # of sources need to add  to the IR */
    gctINT          destComponents;     /* -1 means do not care */
} theIntrinsicNameMap[] =
{
/* The definition of vload* intrinsic reside in the cl_viv_vx_ext.h */
    MAP_INSTRINSIC("vx_read_image", VX_IMG_LOAD, 1),
    MAP_INSTRINSIC("vx_write_image", VX_IMG_STORE, 1),
    MAP_INSTRINSIC("vx_icastD", VX_ICASTD, 0),
    MAP_INSTRINSIC("vx_icastP", VX_ICASTP, 0),
    MAP_INSTRINSIC("vx_AbsDiff", VX_ABSDIFF, 1),
    MAP_INSTRINSIC("vx_IAdd", VX_IADD, 1),
    MAP_INSTRINSIC("vx_IAccSq", VX_IACCSQ, 1),
    MAP_INSTRINSIC("vx_Lerp", VX_LERP, 1),
    MAP_INSTRINSIC("vx_Filter", VX_FILTER, 1),
    MAP_INSTRINSIC("vx_MagPhase", VX_MAGPHASE, 1),
    MAP_INSTRINSIC("vx_MulShift", VX_MULSHIFT, 1),
    MAP_INSTRINSIC("vx_DP16x1", VX_DP16X1, 2),
    MAP_INSTRINSIC("vx_DP8x2", VX_DP8X2, 2),
    MAP_INSTRINSIC("vx_DP4x4", VX_DP4X4, 2),
    MAP_INSTRINSIC("vx_DP2x8", VX_DP2X8, 2),
    MAP_INSTRINSIC("vx_Clamp", VX_CLAMP, 1),
    MAP_INSTRINSIC("vx_BiLinear", VX_BILINEAR, 1),
    MAP_INSTRINSIC("vx_SelectAdd", VX_SELECTADD, 1),
    MAP_INSTRINSIC("vx_AtomicAdd", VX_ATOMICADD, 1),
    MAP_INSTRINSIC("vx_BitExtract", VX_BITEXTRACT, 1),
    MAP_INSTRINSIC("vx_BitReplace", VX_BITREPLACE, 1),
    MAP_INSTRINSIC("vxmc_read_image", VX_IMG_LOAD, 0),
    MAP_INSTRINSIC("vxmc_write_image", VX_IMG_STORE, 0),
    MAP_INSTRINSIC("vxmc_AbsDiff", VX_ABSDIFF, 0),
    MAP_INSTRINSIC("vxmc_IAdd", VX_IADD, 0),
    MAP_INSTRINSIC("vxmc_IAccSq", VX_IACCSQ, 0),
    MAP_INSTRINSIC("vxmc_Lerp", VX_LERP, 0),
    MAP_INSTRINSIC("vxmc_Filter", VX_FILTER, 0),
    MAP_INSTRINSIC("vxmc_MagPhase", VX_MAGPHASE, 0),
    MAP_INSTRINSIC("vxmc_MulShift", VX_MULSHIFT, 0),
    MAP_INSTRINSIC("vxmc_DP16x1", VX_DP16X1, 0),
    MAP_INSTRINSIC("vxmc_DP8x2", VX_DP8X2, 0),
    MAP_INSTRINSIC("vxmc_DP4x4", VX_DP4X4, 0),
    MAP_INSTRINSIC("vxmc_DP2x8", VX_DP2X8, 0),
    MAP_INSTRINSIC("vxmc_DP32x1_b", VX_DP32X1_B, 0),
    MAP_INSTRINSIC("vxmc_DP16x2_b", VX_DP16X2_B, 0),
    MAP_INSTRINSIC("vxmc_DP8x4_b", VX_DP8X4_B, 0),
    MAP_INSTRINSIC("vxmc_DP4x8_b", VX_DP4X8_B, 0),
    MAP_INSTRINSIC("vxmc_DP2x16_b", VX_DP2X16_B, 0),
    MAP_INSTRINSIC("vxmc_DP32x1", VX_DP32X1, 0),
    MAP_INSTRINSIC("vxmc_DP16x2", VX_DP16X2, 0),
    MAP_INSTRINSIC("vxmc_DP8x4", VX_DP8X4, 0),
    MAP_INSTRINSIC("vxmc_DP4x8", VX_DP4X8, 0),
    MAP_INSTRINSIC("vxmc_DP2x16", VX_DP2X16, 0),
    MAP_INSTRINSIC("vxmc_Clamp", VX_CLAMP, 0),
    MAP_INSTRINSIC("vxmc_BiLinear", VX_BILINEAR, 0),
    MAP_INSTRINSIC("vxmc_SelectAdd", VX_SELECTADD, 0),
    MAP_INSTRINSIC("vxmc_AtomicAdd", VX_ATOMICADD, 0),
    MAP_INSTRINSIC("vxmc_BitExtract", VX_BITEXTRACT, 0),
    MAP_INSTRINSIC("vxmc_BitReplace", VX_BITREPLACE, 0),
    {0, 0, VIR_OP_NOP, 0}
};
#undef  MAP_INSTRINSIC
#undef MAP_INSTRINSIC_COMP

/* return ture if the Name (excluding type mangleing part after "__"
 * if exists) is a known intrinsic, otherwise return false
 * VIR opCode corresponding to the instrinsic is returned in VirOpcode
 */
IntrinsicNameMap *
_ConvIntrinsicName2VirOp(
    gctSTRING           Name
    )
{
    gctSTRING  mangledNameStart = gcvNULL;
    gctINT     len;
    gctINT     i;
    if (gcoOS_StrStr(Name, "__", &mangledNameStart))
    {
        len = (gctINT)(mangledNameStart - Name);
    }
    else
    {
        len = (gctINT)strlen(Name);
    }
    i = 0;
    while (theIntrinsicNameMap[i].name != gcvNULL)
    {
        gctINT nl = theIntrinsicNameMap[i].nameLen;
        if (len >= nl &&
            Name[nl-1] == theIntrinsicNameMap[i].name[nl-1] &&
            gcoOS_StrNCmp(theIntrinsicNameMap[i].name, Name, nl) == 0)
        {
            return &theIntrinsicNameMap[i];
        }
        i++;
    }
    return gcvNULL;
}

VX_SrcFormat
VIR_GetOpernadVXFormat(
    IN  VIR_Operand * VirOperand
    )
{
    VIR_TypeId          tyId, elemTyId;
    VX_SrcFormat        fmt = VX_FMT_FP32;
    tyId = VIR_Operand_GetTypeId(VirOperand);
    gcmASSERT(tyId < VIR_TYPE_PRIMITIVETYPE_COUNT);
    elemTyId = VIR_GetTypeComponentType(tyId);  /* get the element type */
    switch(elemTyId) {
    case VIR_TYPE_INT8:
        fmt = VX_FMT_S8;
        break;
    case VIR_TYPE_UINT8:
        fmt = VX_FMT_U8;
        break;
    case VIR_TYPE_INT16:
        fmt = VX_FMT_S16;
        break;
    case VIR_TYPE_UINT16:
        fmt = VX_FMT_U16;
        break;
    case VIR_TYPE_INT32:
        fmt = VX_FMT_S32;
        break;
    case VIR_TYPE_UINT32:
        fmt = VX_FMT_U32;
        break;
    case VIR_TYPE_FLOAT16:
        fmt = VX_FMT_FP16;
        break;
    case VIR_TYPE_FLOAT32:
        fmt = VX_FMT_FP32;
        break;
    case VIR_TYPE_UNKNOWN:
        break;
    default:
        gcmASSERT(gcvFALSE);
        break;
    }
    return fmt;
}

typedef struct _Evis512BitsData
{
    VIR_OpCode   opcode;
    VIR_TypeId   elemTypeId;
    gctUINT      data[16];
} Evis512BitsData;

static Evis512BitsData evisDefault512BitsUniformValue[] =
{
    /* DP16x1 */
    {VIR_OP_VX_DP16X1, VIR_TYPE_UINT8,
       {0x155, /* 5x5 window */ 0x0, 0x76543210, 0x0, 0x55555555, 0x76543210, 0x0, 0xa, 0, 0, 0, 0, 0, 0, 0, 0, /* total 16 x 32bits */}},
    {VIR_OP_VX_DP16X1, VIR_TYPE_INT8,
       {0x155, /* 5x5 window */ 0x0, 0x76543210, 0x0, 0x55555555, 0x76543210, 0x0, 0xa, 0, 0, 0, 0, 0, 0, 0, 0, /* total 16 x 32bits */}},
    {VIR_OP_VX_DP16X1, VIR_TYPE_UINT16,
       {0x155, /* 5x5 window */ 0x0, 0x76543210, 0x0, 0x55555555, 0x76543210, 0x0, 0xa, 0, 0, 0, 0, 0, 0, 0, 0, /* total 16 x 32bits */}},
    {VIR_OP_VX_DP16X1, VIR_TYPE_INT16,
       {0x155, /* 5x5 window */ 0x0, 0x76543210, 0x0, 0x55555555, 0x76543210, 0x0, 0xa, 0, 0, 0, 0, 0, 0, 0, 0, /* total 16 x 32bits */}},
    {VIR_OP_VX_DP16X1, VIR_TYPE_FLOAT16,
       {0x155, /* 5x5 window */ 0x0, 0x76543210, 0x0, 0x55555555, 0x76543210, 0x0, 0xa, 0, 0, 0, 0, 0, 0, 0, 0, /* total 16 x 32bits */}},
    /* DP8x2  */
    {VIR_OP_VX_DP8X2, VIR_TYPE_UINT8,
       {0x155, /* 5x5 window */ 0x0, 0x76543210, 0x0, 0x55555555, 0x76543210, 0x0, 0xa, 0, 0, 0, 0, 0, 0, 0, 0, /* total 16 x 32bits */}},
    {VIR_OP_VX_DP8X2, VIR_TYPE_INT8,
       {0x155, /* 5x5 window */ 0x0, 0x76543210, 0x0, 0x55555555, 0x76543210, 0x0, 0xa, 0, 0, 0, 0, 0, 0, 0, 0, /* total 16 x 32bits */}},
    {VIR_OP_VX_DP8X2, VIR_TYPE_UINT16,
       {0x155, /* 5x5 window */ 0x0, 0x76543210, 0x0, 0x55555555, 0x76543210, 0x0, 0xa, 0, 0, 0, 0, 0, 0, 0, 0, /* total 16 x 32bits */}},
    {VIR_OP_VX_DP8X2, VIR_TYPE_INT16,
       {0x155, /* 5x5 window */ 0x0, 0x76543210, 0x0, 0x55555555, 0x76543210, 0x0, 0xa, 0, 0, 0, 0, 0, 0, 0, 0, /* total 16 x 32bits */}},
    {VIR_OP_VX_DP8X2, VIR_TYPE_FLOAT16,
       {0x155, /* 5x5 window */ 0x0, 0x76543210, 0x0, 0x55555555, 0x76543210, 0x0, 0xa, 0, 0, 0, 0, 0, 0, 0, 0, /* total 16 x 32bits */}},
    /* DP4x4  */
    {VIR_OP_VX_DP4X4, VIR_TYPE_UINT8,
       {0x155, /* 5x5 window */ 0x0, 0x76543210, 0x0, 0x55555555, 0x76543210, 0x0, 0xa, 0, 0, 0, 0, 0, 0, 0, 0, /* total 16 x 32bits */}},
    {VIR_OP_VX_DP4X4, VIR_TYPE_INT8,
       {0x155, /* 5x5 window */ 0x0, 0x76543210, 0x0, 0x55555555, 0x76543210, 0x0, 0xa, 0, 0, 0, 0, 0, 0, 0, 0, /* total 16 x 32bits */}},
    {VIR_OP_VX_DP4X4, VIR_TYPE_UINT16,
       {0x155, /* 5x5 window */ 0x0, 0x76543210, 0x0, 0x55555555, 0x76543210, 0x0, 0xa, 0, 0, 0, 0, 0, 0, 0, 0, /* total 16 x 32bits */}},
    {VIR_OP_VX_DP4X4, VIR_TYPE_INT16,
       {0x155, /* 5x5 window */ 0x0, 0x76543210, 0x0, 0x55555555, 0x76543210, 0x0, 0xa, 0, 0, 0, 0, 0, 0, 0, 0, /* total 16 x 32bits */}},
    {VIR_OP_VX_DP4X4, VIR_TYPE_FLOAT16, {0x00000000, 0, 0 /* total 16 x 32bits */}},
    /* DP2x8  */
    {VIR_OP_VX_DP2X8, VIR_TYPE_UINT8,
       {0x155, /* 5x5 window */ 0x0, 0x76543210, 0x0, 0x55555555, 0x76543210, 0x0, 0xa, 0, 0, 0, 0, 0, 0, 0, 0, /* total 16 x 32bits */}},
    {VIR_OP_VX_DP2X8, VIR_TYPE_INT8,
       {0x155, /* 5x5 window */ 0x0, 0x76543210, 0x0, 0x55555555, 0x76543210, 0x0, 0xa, 0, 0, 0, 0, 0, 0, 0, 0, /* total 16 x 32bits */}},
    {VIR_OP_VX_DP2X8, VIR_TYPE_UINT16,
       {0x155, /* 5x5 window */ 0x0, 0x76543210, 0x0, 0x55555555, 0x76543210, 0x0, 0xa, 0, 0, 0, 0, 0, 0, 0, 0, /* total 16 x 32bits */}},
    {VIR_OP_VX_DP2X8, VIR_TYPE_INT16,
       {0x155, /* 5x5 window */ 0x0, 0x76543210, 0x0, 0x55555555, 0x76543210, 0x0, 0xa, 0, 0, 0, 0, 0, 0, 0, 0, /* total 16 x 32bits */}},
    {VIR_OP_VX_DP2X8, VIR_TYPE_FLOAT16, {0x00000000, 0, 0 /* total 16 x 32bits */}},
};

static VSC_ErrCode
_ConvCode2VirInstruction(
    IN  gcSHADER            Shader,
    IN  VIR_Shader *        VirShader,
    IN  VIR_Function *      VirFunction,
    IN  gcSL_INSTRUCTION    Code,
    IN  gctUINT32           CodeIndex,
    IN  conv2VirsVirRegMap *VirRegMapArr,
    IN  VIR_SymId *         VirAttrIdArr,
    IN  VIR_SymId *         VirOutIdArr,
    IN  VIR_SymId *         VirUniformIdArr,
    IN  conv2VirsInstMap *  VirInstMap,
    IN  VSC_HASH_TABLE     *ExternOpcodeTable,
    OUT VIR_Instruction **  VirInst
    )
{
    gcSL_OPCODE         opcode;
    gctSIZE_T           externOpcode;
    VIR_OpCode          virOpcode;
    VIR_Modifier        virModifier;
    VIR_Instruction *   virInst;
    VIR_Operand *       virDest;
    gcSL_FORMAT         format;
    gcSL_ENABLE         enable;
    VIR_Enable          virEnable;
    gctUINT             mode;
    gctUINT             constIdx = 0;
    VIR_VirRegId        virRegId;
    VSC_ErrCode         virErrCode = VSC_ERR_NONE;
    gctUINT32           i;
    gctUINT             components = 0, rows = 0;
    gctBOOL             packed = gcvFALSE;
    VIR_TypeId          typeId;
    VIR_Instruction    *headerInst = gcvNULL;
    VIR_ConditionOp     virCond    = VIR_COP_ALWAYS;
    gcFUNCTION          intrinsicFunc = gcvNULL;
    gctINT              src2Add = 0;
    IntrinsicNameMap *  intrinsicNameMapPtr = gcvNULL;

    gcmASSERT(VirInst);

    opcode       = (gcSL_OPCODE) gcmSL_OPCODE_GET(Code->opcode, Opcode);

    if (opcode == gcSL_TEXLD || opcode == gcSL_TEXLDPROJ)
    {
        virErrCode = _ConvTexLdForShadow(Shader, Code, &opcode);
    }

    if (opcode == gcSL_BARRIER)
    {
        VIR_Shader_SetFlag(VirShader, VIR_SHFLAG_HAS_BARRIER);
    }

    virOpcode    = _gcmConvShaderOpcodeToVirOpcode(Shader, CodeIndex, opcode);
    externOpcode = _gcmGetShaderOpcodeVirExternOpcode(opcode);

    /* handling viv_intrinsic */
    if (virOpcode == VIR_OP_CALL)
    {
        gcFUNCTION func = gcvNULL;

        gcSHADER_FindFunctionByLabel(Shader, gcmSL_CALL_TARGET(Code), &func);
        if (func &&
            gcoOS_StrNCmp(func->name, "viv_intrinsic_", 14) == 0)
        {
            intrinsicNameMapPtr = _ConvIntrinsicName2VirOp(func->name+14);
            if (intrinsicNameMapPtr)
            {
                intrinsicFunc = func;
                virOpcode = intrinsicNameMapPtr->virOpcode;
                src2Add   = intrinsicNameMapPtr->srcToAdd;
            }
        }
    }
    if (virOpcode == VIR_OP_JMP &&
        gcmSL_TARGET_GET(Code->temp, Condition) != gcSL_ALWAYS) {
        virOpcode = VIR_OP_JMPC;
    }

    if(virOpcode == VIR_OP_MAXOPCODE) {
        return VSC_ERR_INVALID_ARGUMENT;
    }

    format = gcmSL_TARGET_GET(Code->temp, Format);
    enable = gcmSL_TARGET_GET(Code->temp, Enable);

    virErrCode = VIR_Function_AddInstruction(VirFunction,
        virOpcode,
        _virFormatTypeId[format],
        &virInst);
    if(virErrCode != VSC_ERR_NONE) return virErrCode;

    if (intrinsicFunc)
    {
        gctUINT srcNo = 0;
        gctBOOL destSeen = gcvFALSE;
        VIR_Operand *       virOperand;
        /* convert funciton arguments to source operands
         *   convert output parameter to dest operand
         *   convert input parameters to source operands
         */
        for (i = 0; i < intrinsicFunc->argumentCount; i++)
        {
            gcsFUNCTION_ARGUMENT_PTR arg = &(intrinsicFunc->arguments[i]);
            VIR_Precision  precision =VIR_PRECISION_MEDIUM;
            VIR_Symbol      *sym, *varSym;
            VIR_TypeId     opndTyId;
            gcSHADER_UpdateTempRegCount(Shader, arg->index);
            enable = (gcSL_ENABLE)arg->enable;
            components = _GetEnableComponentCount(enable);

            packed = gcvFALSE;
            if(arg->variableIndex != 0xFFFF)
            {
                gcVARIABLE variable = gcvNULL;

                if (arg->variableIndex < Shader->variableCount)
                {
                    variable = Shader->variables[arg->variableIndex];
                }
                gcTYPE_GetTypeInfo(variable->u.type, &components, &rows, 0);
                if(variable && gcTYPE_IsTypePacked(variable->u.type))
                {
                    packed = gcvTRUE;
                }
            }
            virRegId = _GetVirRegId(VirShader,
                                    VirRegMapArr,
                                    arg->index,
                                    format,
                                    gcvFALSE,
                                    components,
                                    packed,
                                    precision);
            virEnable = _ConvEnable2Vir(enable);
            typeId = _ConvScalarFormatToVirVectorTypeId(format,
                                                        components,
                                                        packed);

            if (arg->qualifier == gcvFUNCTION_OUTPUT )
            {
                if (destSeen)
                {
                    /* cannot have two output! */
                    gcmASSERT(gcvFALSE);
                }
                destSeen = gcvTRUE;
                virOperand = VIR_Inst_GetDest(virInst);
                VIR_Operand_SetEnable(virOperand, virEnable);
            }
            else
            {
                gcmASSERT(arg->qualifier != gcvFUNCTION_INOUT);
                gcmASSERT(srcNo < VIR_Inst_GetSrcNum(virInst));
                virOperand = VIR_Inst_GetSource(virInst, srcNo);
                VIR_Operand_SetSwizzle(virOperand,
                             VIR_NormalizeSwizzleByEnable(_ConvEnable2Vir(enable), VIR_SWIZZLE_XYZW));

                srcNo++;  /* go to next source operand */
            }

            /* change operand's precision based on symbol */
            sym = VIR_Shader_GetSymFromId(VirShader, virRegId);
            precision = VIR_Symbol_GetPrecision(sym);
            opndTyId = VIR_Type_GetIndex(VIR_Symbol_GetType(sym));
            if (VIR_Symbol_isVreg(sym))
            {
                varSym = VIR_Symbol_GetVregVariable(sym);
                if (varSym)
                {
                    precision = VIR_Symbol_GetPrecision(varSym);
                    opndTyId = VIR_Type_GetIndex(VIR_Symbol_GetType(varSym));
                }
            }
            VIR_Operand_SetPrecision(virOperand, precision);

            if (destSeen && virOperand == VIR_Inst_GetDest(virInst) &&
                intrinsicNameMapPtr->destComponents > 0)
            {
                VIR_TypeId compTypeId;
                /* figure out the dest type by it components count and base type */
                gcmASSERT(VIR_TypeId_isPrimitive(opndTyId ));
                compTypeId = VIR_GetTypeComponentType(opndTyId);
                switch (compTypeId) {
                case VIR_TYPE_UINT8:
                    switch (intrinsicNameMapPtr->destComponents) {
                    case 2:
                        opndTyId = VIR_TYPE_UINT8_P2;
                        break;
                    case 3:
                        opndTyId = VIR_TYPE_UINT8_P3;
                        break;
                    case 4:
                        opndTyId = VIR_TYPE_UINT8_P4;
                        break;
                    case 8:
                        opndTyId = VIR_TYPE_UINT8_P8;
                        break;
                    case 16:
                        opndTyId = VIR_TYPE_UINT8_P16;
                        break;
                    default:
                        gcmASSERT(gcvFALSE);
                        break;
                    }
                    break;
                case VIR_TYPE_INT8:
                    switch (intrinsicNameMapPtr->destComponents) {
                    case 2:
                        opndTyId = VIR_TYPE_INT8_P2;
                        break;
                    case 3:
                        opndTyId = VIR_TYPE_INT8_P3;
                        break;
                    case 4:
                        opndTyId = VIR_TYPE_INT8_P4;
                        break;
                    case 8:
                        opndTyId = VIR_TYPE_INT8_P8;
                        break;
                    case 16:
                        opndTyId = VIR_TYPE_INT8_P16;
                        break;
                    default:
                        gcmASSERT(gcvFALSE);
                        break;
                    }
                    break;
                case VIR_TYPE_UINT16:
                    switch (intrinsicNameMapPtr->destComponents) {
                    case 2:
                        opndTyId = VIR_TYPE_UINT16_P2;
                        break;
                    case 3:
                        opndTyId = VIR_TYPE_UINT16_P3;
                        break;
                    case 4:
                        opndTyId = VIR_TYPE_UINT16_P4;
                        break;
                    case 8:
                        opndTyId = VIR_TYPE_UINT16_P8;
                        break;
                    default:
                        gcmASSERT(gcvFALSE);
                        break;
                    }
                    break;
                case VIR_TYPE_INT16:
                    switch (intrinsicNameMapPtr->destComponents) {
                    case 2:
                        opndTyId = VIR_TYPE_INT16_P2;
                        break;
                    case 3:
                        opndTyId = VIR_TYPE_INT16_P3;
                        break;
                    case 4:
                        opndTyId = VIR_TYPE_INT16_P4;
                        break;
                    case 8:
                        opndTyId = VIR_TYPE_INT16_P8;
                        break;
                    default:
                        gcmASSERT(gcvFALSE);
                        break;
                    }
                    break;
                case VIR_TYPE_FLOAT16:
                    switch (intrinsicNameMapPtr->destComponents) {
                    case 2:
                        opndTyId = VIR_TYPE_FLOAT16_P2;
                        break;
                    case 3:
                        opndTyId = VIR_TYPE_FLOAT16_P3;
                        break;
                    case 4:
                        opndTyId = VIR_TYPE_FLOAT16_P4;
                        break;
                    case 8:
                        opndTyId = VIR_TYPE_FLOAT16_P8;
                        break;
                    default:
                        gcmASSERT(gcvFALSE);
                        break;
                    }
                    break;
                default:
                    gcmASSERT(gcvFALSE);
                    break;
                }
            }
            VIR_Operand_SetTempRegister(virOperand,
                                        VirFunction,
                                        virRegId,
                                        opndTyId);

            if (VIR_OPCODE_Src0Src1Temp256(virOpcode))
            {
                if (virOperand == VIR_Inst_GetSource(virInst, 0))
                {
                    /* set the src to be higher part of temp 256 register pair */
                    VIR_Operand_SetFlag(virOperand, VIR_OPNDFLAG_TEMP256_HIGH);
                }
                else if (virOperand == VIR_Inst_GetSource(virInst, 1))
                {
                    /* set the src to be lower part of temp 256 register pair */
                    VIR_Operand_SetFlag(virOperand, VIR_OPNDFLAG_TEMP256_LOW);
                }
            }
            else if (VIR_OPCODE_Src1Src2Temp256(virOpcode))
            {
                if (virOperand == VIR_Inst_GetSource(virInst, 1))
                {
                    /* set the src to be higher part of temp 256 register pair */
                    VIR_Operand_SetFlag(virOperand, VIR_OPNDFLAG_TEMP256_HIGH);
                }
                else if (virOperand == VIR_Inst_GetSource(virInst, 2))
                {
                    /* set the src to be lower part of temp 256 register pair */
                    VIR_Operand_SetFlag(virOperand, VIR_OPNDFLAG_TEMP256_LOW);
                }
            }
        }
        if (src2Add > 0)
        {
            /* some intrinsics has two version: high level interface and machine level interface
             * machine level can control more details which cannot be specified in HL API,
             * so HL API need to provide the machine level detail to the IR
             * e.g.
             *    vx_uchar16 viv_intrinsic_vxmc_DP16x1_uc(vx_uchar16 a,
             *                                            vx_uchar16 b,
             *                                            VX_Modifier_t modifier,
             *                                            VX_512Bits u);
             *
             *   vx_uchar16 viv_intrinsic_vx_DP16x1_uc(vx_uchar16 a, vx_uchar16 b);
             */

            VIR_TypeId srcTyId = VIR_TYPE_UNKNOWN;
            VIR_EVIS_Modifier mi;

            mi.u1 = 0;  /* init to all zero */

            switch (virOpcode) {
            case VIR_OP_VX_IMG_LOAD:
            case VIR_OP_VX_IMG_LOAD_3D:
            case VIR_OP_VX_IMG_STORE:
            case VIR_OP_VX_IMG_STORE_3D:
            case VIR_OP_VX_ABSDIFF:
            case VIR_OP_VX_IADD:
            case VIR_OP_VX_IACCSQ:
            case VIR_OP_VX_LERP:
            case VIR_OP_VX_FILTER:
            case VIR_OP_VX_MAGPHASE:
            case VIR_OP_VX_MULSHIFT:
            case VIR_OP_VX_DP16X1:
            case VIR_OP_VX_DP8X2:
            case VIR_OP_VX_DP4X4:
            case VIR_OP_VX_DP2X8:
            case VIR_OP_VX_CLAMP:
            case VIR_OP_VX_BILINEAR:
            case VIR_OP_VX_SELECTADD:
            case VIR_OP_VX_ATOMICADD:
            case VIR_OP_VX_BITEXTRACT:
            case VIR_OP_VX_BITREPLACE:
                /* add default vx_modifier operand */
                /* handle 1st added operand */
                /* get source type from first operand */
                if (virOpcode == VIR_OP_VX_IMG_LOAD ||
                    virOpcode == VIR_OP_VX_IMG_LOAD_3D)
                {
                    /* get operand type from img_load dest */
                    virOperand = VIR_Inst_GetDest(virInst);
                }
                else if (virOpcode == VIR_OP_VX_IMG_STORE ||
                         virOpcode == VIR_OP_VX_IMG_STORE_3D)
                {
                    /* get operand type from img_load dest */
                    virOperand = VIR_Inst_GetSource(virInst, 2);
                }
                else

                {
                    virOperand = VIR_Inst_GetSource(virInst, 0);
                }
                srcTyId = VIR_Operand_GetTypeId(virOperand);

                mi.u0.startBin = 0;
                mi.u0.endBin = VIR_GetTypePackedComponents(srcTyId) - 1;
                mi.u0.sourceBin = 0;

                /* filter mode will be set at VIR->mcodec */
                virOperand = VIR_Inst_GetSource(virInst, VIR_Inst_GetSrcNum(virInst) - src2Add);
                VIR_Operand_SetOpKind(virOperand, VIR_OPND_EVIS_MODIFIER);
                VIR_Operand_SetEvisModifier(virOperand, mi.u1);
                VIR_Operand_SetTypeId(virOperand, VIR_TYPE_UINT32);
                break;
            case VIR_OP_VX_DP32X1:
            case VIR_OP_VX_DP16X2:
            case VIR_OP_VX_DP8X4:
            case VIR_OP_VX_DP4X8:
            case VIR_OP_VX_DP2X16:
            case VIR_OP_VX_DP32X1_B:
            case VIR_OP_VX_DP16X2_B:
            case VIR_OP_VX_DP8X4_B:
            case VIR_OP_VX_DP4X8_B:
            case VIR_OP_VX_DP2X16_B:
            default:
                gcmASSERT(gcvFALSE);
                break;
            }
            /* handle 2nd added operand */
            switch (virOpcode) {
            case VIR_OP_VX_DP16X1:
            case VIR_OP_VX_DP8X2:
            case VIR_OP_VX_DP4X4:
            case VIR_OP_VX_DP2X8:
            {
                gctUINT           j;

                /* search the evis default 512bit uniform  */
                j = 0;
                while (evisDefault512BitsUniformValue[j].elemTypeId != VIR_TYPE_UNKNOWN)
                {
                    VIR_Operand*      opnd ;
                    VIR_Const         virConst;
                    VIR_Uniform*      virUniform;
                    VIR_Symbol*       sym;
                    VIR_Swizzle       swizzle = VIR_SWIZZLE_XXXX;
                    Evis512BitsData * pEvisData = gcvNULL;

                    if (evisDefault512BitsUniformValue[j].opcode == virOpcode &&
                        VIR_GetTypeComponentType(srcTyId) == evisDefault512BitsUniformValue[j].elemTypeId)
                    {
                        pEvisData = &evisDefault512BitsUniformValue[j];
                        /* add 512bits data to uniform table */
                        virConst.index = VIR_INVALID_ID;
                        virConst.type = VIR_TYPE_UINT_X16;
                        virConst.value.vecVal = *(VIR_VecConstVal *)pEvisData->data;
                        VIR_Shader_AddInitializedUniform(VirShader, &virConst, &virUniform, &swizzle);
                        sym = VIR_Shader_GetSymFromId(VirShader, virUniform->sym);
                        opnd = VIR_Inst_GetSource(virInst, VIR_Inst_GetSrcNum(virInst) - src2Add + 1);
                        VIR_Operand_SetOpKind(opnd, VIR_OPND_SYMBOL);
                        VIR_Operand_SetSym(opnd, sym);
                        VIR_Operand_SetSwizzle(opnd, swizzle);

                        break;
                    }
                    j++;
                }
            }
                break;
            case VIR_OP_VX_ABSDIFF:
            case VIR_OP_VX_IADD:
            case VIR_OP_VX_IACCSQ:
            case VIR_OP_VX_LERP:
            case VIR_OP_VX_FILTER:
            case VIR_OP_VX_MAGPHASE:
            case VIR_OP_VX_MULSHIFT:
            case VIR_OP_VX_BILINEAR:
            case VIR_OP_VX_SELECTADD:
            case VIR_OP_VX_ATOMICADD:
            case VIR_OP_VX_BITEXTRACT:
            case VIR_OP_VX_BITREPLACE:
            case VIR_OP_VX_CLAMP:
            case VIR_OP_VX_DP32X1:
            case VIR_OP_VX_DP16X2:
            case VIR_OP_VX_DP8X4:
            case VIR_OP_VX_DP4X8:
            case VIR_OP_VX_DP2X16:
            case VIR_OP_VX_DP16X1_B:
            case VIR_OP_VX_DP8X2_B:
            case VIR_OP_VX_DP4X4_B:
            case VIR_OP_VX_DP2X8_B:
            case VIR_OP_VX_DP32X1_B:
            case VIR_OP_VX_DP16X2_B:
            case VIR_OP_VX_DP8X4_B:
            case VIR_OP_VX_DP4X8_B:
            case VIR_OP_VX_DP2X16_B:
            default:
                gcmASSERT(src2Add == 1);
                break;
            }
        }
    }
    else
    {
        virCond = virConditionMap[gcmSL_TARGET_GET(Code->temp, Condition)];
        VIR_Inst_SetConditionOp(virInst, virCond);
        if(externOpcode != 0)
        {
            vscHTBL_DirectSet(ExternOpcodeTable, (void *)(gctUINTPTR_T)virInst, (void *)externOpcode);
        }

        if(gcmSL_OPCODE_GET(Code->opcode, Round))
        {
            VIR_Operand_SetRoundMode(VIR_Inst_GetDest(virInst), _ConvRound2Vir(gcmSL_OPCODE_GET(Code->opcode, Round)));
        }

        /* check sources */
        for (i=0; i < 2; i++)
        {
            VIR_Operand *virSrc = virInst->src[i];

            if(virSrc == gcvNULL)
            {
                continue;
            }

            virErrCode = _ConvSource2VirOperand(Shader,
                                                VirShader,
                                                VirFunction,
                                                Code,
                                                CodeIndex,
                                                i,
                                                VirRegMapArr,
                                                VirAttrIdArr,
                                                VirOutIdArr,
                                                VirUniformIdArr,
                                                virInst,
                                                virSrc
                                                );
        }

        /* Update the modifier. */
        if(gcmSL_SOURCE_GET(Code->source0, Neg))
        {
            VIR_Operand_NegateOperand(VirShader, VIR_Inst_GetSource(virInst, 0));
        }

        if(gcmSL_SOURCE_GET(Code->source0, Abs))
        {
            VIR_Operand_SetModifier(VIR_Inst_GetSource(virInst, 0),
                VIR_MOD_ABS | VIR_Operand_GetModifier(VIR_Inst_GetSource(virInst, 0)));
        }

        if(gcmSL_SOURCE_GET(Code->source1, Neg))
        {
            VIR_Operand_NegateOperand(VirShader, VIR_Inst_GetSource(virInst, 1));
        }

        if(gcmSL_SOURCE_GET(Code->source1, Abs))
        {
            VIR_Operand_SetModifier(VIR_Inst_GetSource(virInst, 1),
                VIR_MOD_ABS | VIR_Operand_GetModifier(VIR_Inst_GetSource(virInst, 1)));
        }

        if(VIR_OPCODE_hasDest(virOpcode)) {
            /* check destination */
            virDest = VIR_Inst_GetDest(virInst);
            gcmASSERT(virDest);

            virModifier =  _gcmGetShaderOpcodeVirModifier(opcode);
            VIR_Operand_SetModifier(virDest, virModifier);

            if (gcmSL_OPCODE_GET(Code->opcode, Sat) != 0)
            {
                /* set modifier from gcSL IR's SAT bit */
                VIR_Operand_SetModifier(virDest, VIR_MOD_SAT_0_TO_1);
            }

            switch(opcode) {
            case gcSL_JMP:
            case gcSL_CALL:
              {
                gctUINT inst;
                VIR_LabelId virLabelId = VIR_INVALID_ID;
                VIR_Label * virLabel;
                inst = Code->tempIndex;
                /* find/create vir label */
                _FindInstLabel(Shader,
                              inst,
                              VirInstMap,
                              VirShader,
                              &virLabelId);
                CHECK_ERROR(virErrCode, "Failed to find/create VIR label");
                if(virErrCode != VSC_ERR_NONE) return virErrCode;

                if(opcode == gcSL_CALL) {
                    gcmASSERT(VirInstMap[inst].inFunction);
                    VIR_Operand_SetFunction(virDest, VirInstMap[inst].inFunction);
                }
                else {  /* gcSL_JMP */
                    VIR_Link* link;
                    gcmASSERT(VirInstMap[inst].labelId != VIR_INVALID_ID);
                    if(virCond != VIR_COP_ALWAYS) {
                        VIR_Inst_SetOpcode(virInst, VIR_OP_JMPC);
                    }
                    virLabel = VIR_Function_GetLabelFromId(VirFunction, virLabelId);
                    VIR_Operand_SetLabel(virDest, virLabel);
                    VIR_Function_NewLink(VirFunction, &link);
                    VIR_Link_SetReference(link, (gctUINTPTR_T)virInst);
                    VIR_Link_AddLink(&(virLabel->referenced), link);
                }
                break;
              }

            case gcSL_NOP:
            case gcSL_KILL:
            case gcSL_RET:
            /*case gcSL_TEXBIAS:*/
            case gcSL_TEXGRAD:
            case gcSL_TEXLOD:
            case gcSL_TEXGATHER:
            case gcSL_TEXFETCH_MS:
                break;
            default:
                {
                    VIR_Precision  destPrecision = _gcmConvPrecision2Vir(gcmSL_TARGET_GET(Code->temp, Precision));
                    VIR_Symbol      *sym, *varSym = gcvNULL;
                    gctUINT virRegComponents;

                    gcSHADER_UpdateTempRegCount(Shader, Code->tempIndex );
                    enable = gcmSL_TARGET_GET(Code->temp, Enable);
                    components = gcmSL_TARGET_GET(Code->temp, PackedComponents);
                    packed = gcvFALSE;
                    if(components) {
                        if(gcmOPT_oclPackedBasicType() || gcShaderHasVivVxExtension(Shader)) {
                            packed = gcvTRUE;
                        }
                        virRegComponents = components;
                    }
                    else {
                        components = _GetEnableComponentCount(enable);
                        virRegComponents = _GetEnableMaxComponentCount(enable);
                    }
                    virRegId = _GetVirRegId(VirShader,
                                            VirRegMapArr,
                                            Code->tempIndex,
                                            format,
                                            gcvFALSE,
                                            virRegComponents,
                                            packed,
                                            destPrecision);

                    constIdx = gcmSL_INDEX_GET(Code->tempIndex, ConstValue);
                    mode = gcmSL_TARGET_GET(Code->temp, Indexed);
                    virEnable = _ConvEnable2Vir(enable);
                    typeId = _ConvScalarFormatToVirVectorTypeId(format,
                                                                components,
                                                                packed);

                    if (mode != gcSL_NOT_INDEXED)
                    {
                        VIR_VirRegId virRelRegId = _GetVirRegId(
                            VirShader,
                            VirRegMapArr,
                            Code->tempIndexed,
                            format,
                            gcvFALSE,
                            1,
                            gcvFALSE,
                            VIR_PRECISION_ANY);

                        VIR_Operand_SetMatrixConstIndex(virDest, constIdx);

                        VIR_Operand_SetRelIndexing(virDest, virRelRegId);
                        VIR_Operand_SetRelAddrMode(virDest, mode);
                    }
                    else if(Code->tempIndexed > 0)
                    {
                        VIR_Operand_SetMatrixConstIndex(virDest, constIdx);
                        VIR_Operand_SetRelIndexingImmed(virDest, Code->tempIndexed);
                    }

                    VIR_Operand_SetTempRegister(virDest,
                                                VirFunction,
                                                virRegId,
                                                typeId);
                    VIR_Operand_SetEnable(virDest, virEnable);
                    VIR_Operand_SetPrecision(virDest, destPrecision);

                    /* change operand's precision and type based on symbol */
                    sym = VIR_Shader_GetSymFromId(VirShader, virRegId);
                    if (VIR_Symbol_isVreg(sym))
                    {
                        varSym = VIR_Symbol_GetVregVariable(sym);
                        if (varSym)
                        {
                            _updateOperandTypeByVariable(virOpcode, virDest, varSym);
                        }
                    }
                }
                break;
            }
        }
    }

    *VirInst = (headerInst != NULL) ? headerInst : virInst;
    (*VirInst)->sourceLoc.fileId = 0;
    (*VirInst)->sourceLoc.colNo = GCSL_SRC_LOC_COLNO(Code->srcLoc);
    (*VirInst)->sourceLoc.lineNo= GCSL_SRC_LOC_LINENO(Code->srcLoc);
    return virErrCode;
}

typedef struct _VIR_PATTERN_GCSL2VIR_CONTEXT
{
    VIR_PatternContext       header;
    VSC_HASH_TABLE          *externOpcodeTable;
    conv2VirsVirRegMap      *virRegMapArr;
}VIR_PatternGCSL2VirContext;

static VIR_Pattern*
_GetgcSL2VirPatterns(
    IN VIR_PatternContext      *Context,
    IN VIR_Instruction         *Inst
    );

static gctBOOL
_CmpInstruction(
    IN VIR_PatternContext   *Context,
    IN VIR_PatternMatchInst *Inst0,
    IN VIR_Instruction      *Inst1
    )
{
    gctUINT externOp = 0;

    if((Inst0->opcode & 0xffff)  != Inst1->_opcode)
        return gcvFALSE;

    if(!Inst1->_isPatternRep)
    {
        externOp = (gctUINT)((VIR_OpCode)(gctUINTPTR_T)vscHTBL_DirectGet(((VIR_PatternGCSL2VirContext *)Context)->externOpcodeTable, (void *)(gctUINTPTR_T)Inst1));
    }

    return externOp == (gctUINT)((Inst0->opcode & 0xf0000) >> 16);
}

/* convert input gcShader Shader to VIR shader, the output vir shader
 * is created with gcoOS_Allocate in global memory */
gceSTATUS
vscConvertGcShader2VirShader(
    IN  SHADER_HANDLE           GcShader,
    OUT SHADER_HANDLE*          phVirShader
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gcSHADER  shader = (gcSHADER) GcShader;

    gcmVERIFY_OBJECT(shader, gcvOBJ_SHADER);

    do
    {
        VIR_Shader *              virShader;
        VIR_ShaderKind            shaderKind = VIR_SHADER_UNKNOWN;
        gctBOOL                   dumpCGV = gcSHADER_DumpCodeGenVerbose(shader);
        VSC_HW_CONFIG *           hwCfg = &gcHWCaps;

        switch (shader->type) {
        case gcSHADER_TYPE_VERTEX:
            shaderKind = VIR_SHADER_VERTEX;
            break;
        case gcSHADER_TYPE_FRAGMENT:
            shaderKind = VIR_SHADER_FRAGMENT;
            break;
        case gcSHADER_TYPE_CL:
        case gcSHADER_TYPE_COMPUTE:
            shaderKind = VIR_SHADER_COMPUTE;
            break;
        case gcSHADER_TYPE_TCS:
            shaderKind = VIR_SHADER_TESSELLATION_CONTROL;
            break;
        case gcSHADER_TYPE_TES:
            shaderKind = VIR_SHADER_TESSELLATION_EVALUATION;
            break;
        case gcSHADER_TYPE_GEOMETRY:
            shaderKind = VIR_SHADER_GEOMETRY;
            break;
        default:
            gcmASSERT(gcvFALSE);
            break;
        }

        gcmONERROR(gcoOS_Allocate(gcvNULL,
                                  sizeof(VIR_Shader),
                                  (gctPOINTER*)&virShader));

        ON_ERROR2STATUS(VIR_Shader_Construct(gcvNULL, shaderKind, virShader), "Failed to call construct VIR_Shader" );
        vscReferenceShader(virShader);
        if (dumpCGV)
        {
            gcDump_Shader(gcvNULL, "Incoming gcSL shader IR.", gcvNULL, shader, gcvTRUE);
        }

        gcmONERROR(gcSHADER_Conv2VIR(shader, hwCfg, virShader));
#if _DEBUG_VIR_IO_COPY
        {
            VSC_ErrCode               errCode;
            VIR_Shader *copiedShader;

            gcmONERROR(gcoOS_Allocate(gcvNULL,
                                      sizeof(VIR_Shader),
                                      (gctPOINTER*)&copiedShader));

            gcmASSERT(copiedShader != gcvNULL);

            VIR_Shader_Copy(copiedShader, virShader);
            VIR_Shader_Destroy(virShader);
            gcoOS_Free(gcvNULL, virShader);
            if (dumpCGV)
            {
                VIR_Shader_Dump(gcvNULL, "Converted and Copied VIR Shader", copiedShader, gcvTRUE);
            }
            {
                VIR_Shader_IOBuffer buf;
                VIR_Shader * readShader;

                VIR_Shader_IOBuffer_Initialize(&buf);

                VIR_Shader_Save(copiedShader, &buf);

                gcmONERROR(gcoOS_Allocate(gcvNULL,
                                          sizeof(VIR_Shader),
                                          (gctPOINTER*)&readShader));

                errCode = VIR_Shader_Construct(gcvNULL,
                                               VIR_SHADER_UNKNOWN,
                                               readShader);
                buf.shader = readShader;
                buf.curPos = 0;

                VIR_Shader_Read(readShader, &buf);

                VIR_Shader_Destroy(copiedShader);
                gcoOS_Free(gcvNULL, copiedShader);
                VIR_IO_Finalize(&buf);
                VIR_Shader_IOBuffer_Finalize(&buf);

                *VirShader = readShader;
            }
        }
#else
        if (dumpCGV)
        {
            VIR_Shader_Dump(gcvNULL, "Converted VIR shader IR.", virShader, gcvTRUE);
        }

        *phVirShader = (SHADER_HANDLE)virShader;
#endif

    } while (gcvFALSE);

    /* Success. */

OnError:
    /* Return the status. */
    return status;
}

/*******************************************************************************
**  gcSHADER_Conv2VIR
**
**  Convert gcSHADER in old IR to VIR
**
**  INPUT:
**
**      gcSHADER Shader
**          Pointer to a gcSHADER object.
**
**  OUTPUT:
**
**      VIR_Shader * VirShader
**          Pointer to VIR Shader variable to receive the converted data
*/
gceSTATUS
gcSHADER_Conv2VIR(
    IN gcSHADER Shader,
    IN VSC_HW_CONFIG * hwCfg,
    IN OUT SHADER_HANDLE hVirShader
    )
{
    gceSTATUS   status = gcvSTATUS_OK;
    gctUINT32   i, j, k;
    VIR_Function *virFunction;
    VIR_SymId   symId;
    VIR_Symbol  *sym;
    VSC_ErrCode virErrCode = VSC_ERR_NONE;
    conv2VirsVirRegMap *virRegMapStart = gcvNULL;
    conv2VirsVirRegMap *virRegMapArr = gcvNULL;
    gctUINT attrVirRegCount = 0;
    gctUINT totalVirRegCount = 0;
    VIR_SymId   *virAttrIdArr = gcvNULL;
    VIR_SymId   *virOutIdArr = gcvNULL;
    gctSIZE_T   allShaderUniformCount;
    VIR_SymId   *virUniformIdArr = gcvNULL;
    conv2VirsInstMap *virInstMap = gcvNULL;
    VIR_SymId   virRegId;
    VIR_Instruction *virInst;
    gctPOINTER  pointer;
    gctSIZE_T   size;
    VSC_HASH_TABLE          *externOpcodeTable;
    VSC_PRIMARY_MEM_POOL     mp;
    VIR_Shader*   VirShader = (VIR_Shader*)hVirShader;
    VIR_Function * kernelFunc = gcvNULL;

    gcmHEADER_ARG("Shader=0x%x VirShader=0x%x", Shader, VirShader);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Shader, gcvOBJ_SHADER);
    gcmDEBUG_VERIFY_ARGUMENT(VirShader);

    if(!VirShader) {
       gcmFATAL("gcSHADER_Conv2VIR: null VIR shader handle passed");
       gcmFOOTER_ARG("status=%d", gcvSTATUS_INVALID_ARGUMENT);
       return gcvSTATUS_INVALID_ARGUMENT;
    }

    vscPMP_Intialize(&mp, gcvNULL, 1024, sizeof(void *), gcvTRUE);
    externOpcodeTable = vscHTBL_Create(&mp.mmWrapper,
                                       vscHFUNC_DefaultPtr, vscHKCMP_Default, 256);

    _gcVIR_SHADER_Clean(VirShader);

    VirShader->clientApiVersion = Shader->clientApiVersion;

    VIR_Shader_SetId(VirShader, Shader->_id);
    VirShader->_constVectorId = Shader->_constVectorId;
    VirShader->_dummyUniformCount = Shader->_dummyUniformCount;

    VirShader->debugInfo = Shader->debugInfo;

    if (gcmOPT_DriverVIRPath())
    {
        /* Suppose vir is under pre-ML now */
        VIR_Shader_SetLevel(VirShader, VIR_SHLEVEL_Pre_Medium);
    }
    else
    {
        /* Suppose vir is under post-ML now */
        VIR_Shader_SetLevel(VirShader, VIR_SHLEVEL_Post_Medium);
    }

    /* initialize the TCS shaderLayout data */
    if (VirShader->shaderKind == VIR_SHADER_TESSELLATION_CONTROL)
    {
        /* we assume the input vertex count equals to the output vertex count here.
           later on, we collect all the input vertex access, if we find larger
           const access index, we will change tcsPatchInputVertices. In the run time,
           the driver will compare the assumed tcsPatchInputVertices with the real one,
           if not the same, the recompilation will needed.

           When recompiling, useDriverTcsPatchInputVertices is set to be true,
           the real input vertex count is inside tcsPatchInputVertices */
        if (Shader->useDriverTcsPatchInputVertices)
        {
            VirShader->shaderLayout.tcs.tcsPatchInputVertices = Shader->shaderLayout.tcs.tcsPatchInputVertices;
            VIR_Shader_SetFlag(VirShader, VIR_SHFLAG_TCS_USE_DRIVER_INPUT);
        }
        else
        {
            VirShader->shaderLayout.tcs.tcsPatchInputVertices = Shader->shaderLayout.tcs.tcsPatchOutputVertices;
            VIR_Shader_ClrFlag(VirShader, VIR_SHFLAG_TCS_USE_DRIVER_INPUT);
        }
        VirShader->shaderLayout.tcs.hasOutputVertexAccess = gcvFALSE;
    }

    attrVirRegCount = 0;
    for (i = 0; i < Shader->attributeCount; i++) {
        gctUINT32 components = 0, rows = 0;

        if (Shader->attributes[i] == gcvNULL) continue;
        /* Get # of rows in attribute type */
        gcTYPE_GetTypeInfo(Shader->attributes[i]->type, &components, &rows, 0);
        rows *= Shader->attributes[i]->arraySize;

        attrVirRegCount += rows;
    }

    totalVirRegCount = attrVirRegCount + Shader->_tempRegCount + 256;
    VIR_Shader_SetOrgRegCount(VirShader, attrVirRegCount + Shader->_tempRegCount);

    size = (totalVirRegCount + 4) * gcmSIZEOF(conv2VirsVirRegMap);
    gcmONERROR(gcoOS_Allocate(gcvNULL,
                            size,
                            &pointer));

    virRegMapStart = pointer;
    gcmASSERT(totalVirRegCount + 4 < 0x00fffff);  /* < 2M temp registers */
    for(i = 0; i < totalVirRegCount + 4; i++) {
        virRegMapStart[i].inFunction = gcvNULL;
        virRegMapStart[i].virRegId = VIR_INVALID_ID;
        virRegMapStart[i].components = 0;
        virRegMapStart[i].packed = gcvFALSE;
    }

    /* the first three are reserved for temp virtual register for the operands
       when used in array indexing */
    for(i = 0; i < 3; i++) {
        virErrCode = VIR_Shader_AddSymbol(VirShader,
                                          VIR_SYM_VIRREG,
                                          totalVirRegCount + i,
                                          VIR_Shader_GetTypeFromId(VirShader, VIR_TYPE_UNKNOWN),
                                          VIR_STORAGE_UNKNOWN,
                                          &virRegId);

        if (virErrCode == VSC_ERR_NONE)
        {
            sym = VIR_Shader_GetSymFromId(VirShader, virRegId);
            virRegMapStart[i].virRegId = virRegId;
        }
        else
        {
            gcmFATAL("Failed to add virtual register symbol");
            status = gcvSTATUS_INVALID_ARGUMENT;
            goto OnError;
        }
    }

    virRegMapArr = virRegMapStart + 3;

    allShaderUniformCount = Shader->uniformCount + Shader->uniformBlockCount + Shader->_dummyUniformCount;
    size = (allShaderUniformCount + 1) * gcmSIZEOF(VIR_SymId);
    gcmONERROR(gcoOS_Allocate(gcvNULL,
                            size,
                            &pointer));

    virUniformIdArr = pointer;

    for(i = 0; i < allShaderUniformCount + 1; i++) {
        virUniformIdArr[i] = VIR_INVALID_ID;
    }

    size = (Shader->attributeCount + 1) * gcmSIZEOF(VIR_SymId);
    gcmONERROR(gcoOS_Allocate(gcvNULL,
                            size,
                            &pointer));

    virAttrIdArr = pointer;

    for(i = 0; i < Shader->attributeCount + 1; i++) {
        virAttrIdArr[i] = VIR_INVALID_ID;
    }

    size = (Shader->outputCount + 1) * gcmSIZEOF(VIR_SymId);
    gcmONERROR(gcoOS_Allocate(gcvNULL,
                            size,
                            &pointer));

    virOutIdArr = pointer;

    for(i = 0; i < Shader->outputCount + 1; i++) {
        virOutIdArr[i] = VIR_INVALID_ID;
    }

    switch(Shader->type) {
    case gcSHADER_TYPE_PRECOMPILED:
        VirShader->shaderKind = VIR_SHADER_PRECOMPILED;
        break;

    case gcSHADER_TYPE_VERTEX:
        VirShader->shaderKind = VIR_SHADER_VERTEX;
        break;

    case gcSHADER_TYPE_FRAGMENT:
        VirShader->shaderKind = VIR_SHADER_FRAGMENT;
        break;

    case gcSHADER_TYPE_CL:
    case gcSHADER_TYPE_COMPUTE:
        VirShader->shaderKind = VIR_SHADER_COMPUTE;
        break;

    case gcSHADER_TYPE_TCS:
        VirShader->shaderKind = VIR_SHADER_TESSELLATION_CONTROL;
        break;

    case gcSHADER_TYPE_TES:
        VirShader->shaderKind = VIR_SHADER_TESSELLATION_EVALUATION;
        break;

    case gcSHADER_TYPE_GEOMETRY:
        VirShader->shaderKind = VIR_SHADER_GEOMETRY;
        break;

    case gcSHADER_TYPE_LIBRARY:
        VirShader->shaderKind = VIR_SHADER_LIBRARY;
        break;

    default:
       gcmFATAL("gcSHADER_Conv2VIR: unknown shader type passed");
       status = gcvSTATUS_INVALID_ARGUMENT;
       goto OnError;
    }

    /* compiler version */
    VirShader->compilerVersion[0] = Shader->compilerVersion[0];
    VirShader->compilerVersion[1] = Shader->compilerVersion[1];

    VirShader->useEarlyFragTest = Shader->useEarlyFragTest;
    VirShader->useLastFragData = Shader->useLastFragData;

    /* convert shader flags */
    if (gcShaderIsOldHeader(Shader))
        VIR_Shader_SetFlag(VirShader, VIR_SHFLAG_OLDHEADER);

    if (gcShaderHasUnsizedSBO(Shader))
        VIR_Shader_SetFlag(VirShader, VIR_SHFLAG_HAS_UNSIZED_SBO);

    if (gcShaderHasImageQuery(Shader))
    {
        gcmASSERT(GetShaderType(Shader) == gcSHADER_TYPE_CL);
        VIR_Shader_SetFlag(VirShader, VIR_SHFLAG_HAS_IMAGE_QUERY);
    }

    if (gcShaderHasInt64(Shader))
        VIR_Shader_SetFlag(VirShader, VIR_SHFLAG_HAS_INT64);

    if (gcShaderHasVivVxExtension(Shader))
    {
        VIR_Shader_SetFlag(VirShader, VIR_SHFLAG_HAS_VIV_VX_EXTENSION);
    }

    if (gcShaderHasVivGcslDriverImage(Shader))
    {
        VIR_Shader_SetFlag(VirShader, VIR_SHFLAG_HAS_VIV_GCSL_DRIVER_IMAGE);
    }

    if (gcShaderHasDefineMainFunc(Shader))
    {
        VIR_Shader_SetFlag(VirShader, VIR_SHFLAG_HAS_DEFINE_MAIN_FUNC);
    }

     size = (Shader->codeCount + 1) * gcmSIZEOF(conv2VirsInstMap);
     gcmONERROR(gcoOS_Allocate(gcvNULL,
                            size,
                            &pointer));

    gcoOS_ZeroMemory(pointer, size);
    virInstMap = pointer;

    for(i = 0; i < Shader->codeCount + 1; i++) {
        virInstMap[i].labelId = VIR_INVALID_ID;
    }

    /* Functions. */
    for (i = 0; i < Shader->functionCount; i++)
    {
        gcFUNCTION function = Shader->functions[i];

        if (function == gcvNULL)
        {
            continue;
        }
        else if (IsFunctionNotUsed(function))
        {
            /* mark the instructions in the unused function as UNUSED */
            for (j = 0; j < function->codeCount; j++)
            {
                virInstMap[function->codeStart + j].inFunction = UNUSED_FUNCTION;
            }
            continue;
        }
        virErrCode =_ConvShaderFunction2Vir(Shader,
                                            function,
                                            VirShader,
                                            virRegMapArr,
                                            &virFunction);

        if(virErrCode != VSC_ERR_NONE) {
            status = gcvSTATUS_INVALID_ARGUMENT;
            goto OnError;
        }
        for (j = 0; j < function->codeCount; j++)
        {
            virInstMap[function->codeStart + j].inFunction = virFunction;
        }

        for (j = 0; j < function->tempIndexCount; j++)
        {
            if(virRegMapArr[function->tempIndexStart + j].inFunction) continue;
            virRegMapArr[function->tempIndexStart + j].inFunction = virFunction;
        }
    }

    attrVirRegCount = Shader->_tempRegCount;
    /* handle attributes */
    for (i = 0; i < Shader->attributeCount; i++)
    {
        gcATTRIBUTE attribute = Shader->attributes[i];

        if (attribute == gcvNULL ||
            gcmATTRIBUTE_isIOBlockMember(attribute))
        {
            continue;
        }

        virErrCode = _ConvShaderAttribute2Vir(Shader,
                                              i,
                                              attribute,
                                              virAttrIdArr,
                                              virRegMapArr,
                                              &attrVirRegCount,
                                              VirShader,
                                              VIR_INVALID_ID,
                                              hwCfg,
                                              gcvNULL);
        ON_ERROR2STATUS(virErrCode, "Failed to normal attributes: _ConvShaderAttribute2Vir");
    }

    /* Uniform blocks. */
    for (i = 0; i < Shader->uniformBlockCount; i++)
    {
        gcsUNIFORM_BLOCK uniformBlock;
        VIR_UniformBlock *virUniformBlock;

        uniformBlock = Shader->uniformBlocks[i];
        if (uniformBlock == gcvNULL) continue;

        if (GetUBPrevSibling(uniformBlock) == -1)
        {
            VIR_TypeId uniformBlockTypeId[1] = {VIR_TYPE_UNKNOWN};

            virErrCode =_ConvUniformBlock2Vir(Shader,
                                              uniformBlock,
                                              VirShader,
                                              uniformBlockTypeId,
                                              &virUniformBlock);
            ON_ERROR2STATUS(virErrCode, "Failed to _ConvUniformBlock2Vir");
            virUniformIdArr[GetUBIndex(uniformBlock)] = virUniformBlock->baseAddr;

            while (GetUBNextSibling(uniformBlock) != -1)
            {
                gcmONERROR(gcSHADER_GetUniformBlock(Shader,
                                                  GetUBNextSibling(uniformBlock),
                                                  &uniformBlock));

                virErrCode =_ConvUniformBlock2Vir(Shader,
                                                  uniformBlock,
                                                  VirShader,
                                                  uniformBlockTypeId,
                                                  &virUniformBlock);
                ON_ERROR2STATUS(virErrCode, "Failed to _ConvUniformBlock2Vir");
                virUniformBlock->baseAddr = virUniformIdArr[GetUBIndex(uniformBlock)];
            }
        }

        if (virErrCode != VSC_ERR_NONE)
        {
           status = gcvSTATUS_INVALID_ARGUMENT;
           goto OnError;
        }
    }

    /* handle uniforms */
    for (i = 0; i < Shader->uniformCount; i++)
    {
        gctUINT16 firstUniformIndex;
        gcUNIFORM uniform = Shader->uniforms[i];

        if ((uniform == gcvNULL) ||
            /* This uniform is not the first element of a struct. */
            (isUniformStruct(uniform) && uniform->parent != -1) ||
            /* This uniform is the block address for a SSBO or a UBO. */
            (uniform->blockIndex != -1 && !isUniformBlockMember(uniform)))
        {
            continue;
        }

        virErrCode =_ConvShaderUniformIdx2Vir(Shader,
                                              (gctINT)uniform->index,
                                              _GetStartUniformIndex(Shader, uniform),
                                              uniform->blockIndex,
                                              VirShader,
                                              gcvNULL,
                                              &firstUniformIndex,
                                              &symId);
        ON_ERROR2STATUS(virErrCode, "Failed to _ConvShaderUniformIdx2Vir");

        sym = VIR_Shader_GetSymFromId(VirShader, symId);
        gcmASSERT(virUniformIdArr[uniform->index] == VIR_INVALID_ID);
        virUniformIdArr[uniform->index] = symId;
        if (uniform->parent != -1)
        {
            VIR_Symbol_SetFlag(sym, VIR_SYMFLAG_IS_FIELD);
        }
        if (isUniformStaticallyUsed(uniform))
        {
            VIR_Symbol_SetFlag(sym, VIR_SYMFLAG_STATICALLY_USED);
        }
        if (isUniformLodMinMax(uniform) || isUniformLevelBaseSize(uniform))
        {
            gcUNIFORM gcslSampler = gcvNULL;
            VIR_Uniform* child = VIR_Symbol_GetUniform(sym);
            VIR_Uniform* virSampler;

            gcmASSERT(uniform->parent != -1);

            gcSHADER_GetUniform(Shader, uniform->parent, &gcslSampler);
            virSampler = VIR_Shader_GetUniformFromGCSLIndex(VirShader, gcslSampler->index);
            gcmASSERT(virSampler);
            if(isUniformLodMinMax(uniform))
            {
                virSampler->u.samplerOrImageAttr.lodMinMax = VIR_Symbol_GetIndex(sym);
            }
            if(isUniformLevelBaseSize(uniform))
            {
                virSampler->u.samplerOrImageAttr.levelBaseSize = VIR_Symbol_GetIndex(sym);
            }
            child->u.samplerOrImageAttr.parentSamplerSymId = VIR_Uniform_GetSymID(virSampler);
            child->u.samplerOrImageAttr.arrayIdxInParent = NOT_ASSIGNED;
        }
    }

    for (i = 0; i < Shader->uniformCount; i++)
    {
        gcUNIFORM uniform = Shader->uniforms[i];

        if (!uniform)
        {
            continue;
        }
        if (uniform->baseBindingIdx != -1)
        {
            VIR_Id      virUniformID, virBaseUniformID;
            VIR_Symbol *virBaseUniformSym = gcvNULL;
            VIR_Symbol *virUniformSym = gcvNULL;
            VIR_Uniform *virUniform = gcvNULL;
            gctUINT     k;

            for (k = 0; k < VIR_IdList_Count(&VirShader->uniforms); k ++)
            {
                virUniformID  = VIR_IdList_GetId(&VirShader->uniforms, k);
                virUniformSym = VIR_Shader_GetSymFromId(VirShader, virUniformID);

                if (uniform->index == (gctINT16)virUniformSym->u2.uniform->gcslIndex)
                {
                    virUniform = virUniformSym->u2.uniform;
                    break;
                }
            }

            for (k = 0; k < VIR_IdList_Count(&VirShader->uniforms); k ++)
            {
                virBaseUniformID  = VIR_IdList_GetId(&VirShader->uniforms, k);
                virBaseUniformSym = VIR_Shader_GetSymFromId(VirShader, virBaseUniformID);

                if (uniform->baseBindingIdx == (gctINT16)virBaseUniformSym->u2.uniform->gcslIndex)
                {
                    virUniform->baseBindingUniform = VIR_Symbol_GetIndex(virBaseUniformSym);
                    break;
                }
            }
        }

        if (isUniformLodMinMax(uniform) || isUniformLevelBaseSize(uniform))
        {
            gcUNIFORM   sampler;
            gctUINT     k;
            VIR_Id      virSamplerUniformID;
            VIR_Symbol *virSamplerUniformSym = gcvNULL;

            sampler = Shader->uniforms[uniform->parent];
            gcmASSERT(uniform->parent < (gctINT16)Shader->uniformCount && sampler);

            /* Mark this sampler as use to calucate texture size.  */
            SetUniformFlag(sampler, gcvUNIFORM_FLAG_SAMPLER_CALCULATE_TEX_SIZE);

            if (!isUniformUsedInShader(sampler) && !isUniformUsedInLTC(sampler))
            {
                gcUNIFORM childUniform = gcvNULL;
                VIR_Id      virUniformID;
                VIR_Symbol *virUniformSym = gcvNULL;

                gcmASSERT(sampler->firstChild != -1 &&
                            sampler->firstChild < (gctUINT16)Shader->uniformCount);

                childUniform = Shader->uniforms[sampler->firstChild];

                gcmASSERT(childUniform);

                while (childUniform)
                {
                    if (isUniformUsedInShader(childUniform) ||
                        isUniformUsedInLTC(childUniform))
                    {
                        for (k = 0; k < VIR_IdList_Count(&VirShader->uniforms); k ++)
                        {
                            virUniformID  = VIR_IdList_GetId(&VirShader->uniforms, k);
                            virUniformSym = VIR_Shader_GetSymFromId(VirShader, virUniformID);

                            if (sampler->index == (gctINT16)virUniformSym->u2.uniform->gcslIndex)
                            {
                                break;
                            }
                        }

                        VIR_Symbol_SetFlag(virUniformSym, VIR_SYMUNIFORMFLAG_IMPLICITLY_USED);

                        break;
                    }

                    if (childUniform->nextSibling != -1)
                    {
                        childUniform = Shader->uniforms[childUniform->nextSibling];
                    }
                    else
                    {
                        childUniform = gcvNULL;
                    }
                }
            }

            for (k = 0; k < VIR_IdList_Count(&VirShader->uniforms); k ++)
            {
                virSamplerUniformID  = VIR_IdList_GetId(&VirShader->uniforms, k);
                virSamplerUniformSym = VIR_Shader_GetSymFromId(VirShader, virSamplerUniformID);

                if (sampler->index == (gctINT16)virSamplerUniformSym->u2.uniform->gcslIndex)
                {
                    VIR_Symbol_SetFlag(virSamplerUniformSym, VIR_SYMUNIFORMFLAG_SAMPLER_CALCULATE_TEX_SIZE);
                    break;
                }
            }
        }
    }

    /* SSBO. */
    for (i = 0; i < Shader->storageBlockCount; i++)
    {
        gcsSTORAGE_BLOCK ssbo;
        VIR_StorageBlock *virSSBO;

        ssbo = Shader->storageBlocks[i];

        virErrCode = _ConvSSBlock2Vir(Shader,
                                      ssbo,
                                      VirShader,
                                      virUniformIdArr,
                                      &virSSBO);
        ON_ERROR2STATUS(virErrCode, "Failed to _ConvSSBlock2Vir");
    }

    /* Outputs. */
    for (i = 0; i < Shader->outputCount; i++)
    {
        gcOUTPUT output = Shader->outputs[i];

        if (output == gcvNULL ||
            gcmOUTPUT_isIOBLockMember(output))
        {
            continue;
        }

        virErrCode = _ConvShaderOutput2Vir(Shader,
                                           &i,
                                           output,
                                           virOutIdArr,
                                           virRegMapArr,
                                           VirShader,
                                           VIR_INVALID_ID,
                                           gcvNULL);
        ON_ERROR2STATUS(virErrCode, "Failed to add normal output: _ConvShaderOutput2Vir");
    }

    /* IO blocks. */
    for (i = 0; i < Shader->ioBlockCount; i++)
    {
        gcsIO_BLOCK ioBlock;
        VIR_IOBlock *virIOBlock;

        ioBlock = Shader->ioBlocks[i];
        if (ioBlock == gcvNULL) continue;

        virErrCode = _ConvIOBlock2Vir(Shader,
                                      ioBlock,
                                      virAttrIdArr,
                                      virOutIdArr,
                                      virRegMapArr,
                                      &attrVirRegCount,
                                      VirShader,
                                      hwCfg,
                                      &virIOBlock);
        ON_ERROR2STATUS(virErrCode, "Failed to _ConvUniformBlock2Vir");
    }

    /* Transform feedback */
    if (Shader->transformFeedback.varyingCount > 0)
    {
        VIR_OutputIdList *pOutputs;
        gcSHADER_ComputeTotalFeedbackVaryingsSize(Shader);
        VIR_IdList_Init(&VirShader->pmp.mmWrapper,
                        Shader->transformFeedback.varyingCount,
                        &VirShader->transformFeedback.varyings);

        VIR_ValueList_Init(&VirShader->pmp.mmWrapper,
                           Shader->transformFeedback.varyingCount,
                           sizeof(VIR_VarTempRegInfo),
                           &VirShader->transformFeedback.varRegInfos);

        for (i = 0; i < Shader->transformFeedback.varyingCount; i++)
        {
            gcOUTPUT varying = Shader->transformFeedback.varyings[i].output;
            gctBOOL isWholeTFBed = Shader->transformFeedback.varyings[i].isWholeTFBed;
            /* find the gcSL output from appropriate VIR output list */
            if (gcmOUTPUT_isPerPatch(varying))
            {
                pOutputs = isWholeTFBed ? VIR_Shader_GetPerpatchOutputs(VirShader)
                                        : VIR_Shader_GetPerpatchOutputVregs(VirShader);
            }
            else
            {
                pOutputs = isWholeTFBed ? VIR_Shader_GetOutputs(VirShader)
                                        : VIR_Shader_GetOutputVregs(VirShader);
            }
            sym = gcvNULL;
            for (j = 0; j < Shader->outputCount; j++)
            {
                gcOUTPUT output = Shader->outputs[j];
                if (output == gcvNULL)
                {
                    continue;
                }

                if (varying == output)
                {
                    for (k = 0; k < VIR_IdList_Count(pOutputs); k ++)
                    {
                        VIR_VirRegId    outputVirRegId;
                        sym = VIR_Shader_GetSymFromId(VirShader, VIR_IdList_GetId(pOutputs, k));

                        outputVirRegId = VIR_Symbol_GetVregIndex(sym);
                        /* the output tempIndex is in the vriReg index range of the Vir output */
                        if (output->tempIndex == outputVirRegId)
                        {
                            gcsVarTempRegInfo * gv = &(Shader->transformFeedback.varRegInfos[i]);
                            VIR_VarTempRegInfo  vv;
                            vv.streamoutSize = gv->streamoutSize;
                            vv.tempRegCount  = Shader->transformFeedback.varyings[i].isArray ?
                                               gv->tempRegCount * varying->arraySize :
                                               gv->tempRegCount;
                            vv.tempRegTypes  = gcvNULL;
                            vv.variable      = VIR_Symbol_GetIndex(sym);
                            VIR_IdList_Add(VirShader->transformFeedback.varyings, VIR_IdList_GetId(pOutputs, k));
                            VIR_ValueList_Add(VirShader->transformFeedback.varRegInfos, (gctCHAR *)&vv);
                            break;
                        }
                    }

                    /* it must find the match */
                    gcmASSERT(sym != gcvNULL && k < VIR_IdList_Count(pOutputs));

                    break;
                }
            }
            /* it must find the match */
            gcmASSERT(sym != gcvNULL);
        }

        if (Shader->transformFeedback.bufferMode == gcvFEEDBACK_INTERLEAVED)
        {
            VirShader->transformFeedback.bufferMode = VIR_FEEDBACK_INTERLEAVED;
        }
        else if (Shader->transformFeedback.bufferMode == gcvFEEDBACK_SEPARATE)
        {
            VirShader->transformFeedback.bufferMode = VIR_FEEDBACK_SEPARATE;
        }
        else
        {
            gcmASSERT(gcvFALSE);
        }
    }

    /* Global variables. */
    for (i = 0; i < Shader->variableCount; i++)
    {
        gctUINT startRegIndex, endRegIndex, initialRegIndex;
        gcVARIABLE variable;

        variable = Shader->variables[i];
        if (variable == gcvNULL ||
            variable->parent != -1) continue;

        if (IsVariableIsNotUsed(variable))
            continue;
        if (GetVariableIsOtput(variable))
        {
            /* the output should be already added to symbol table.
               if it is not, then this is a dead output*/
            continue;
        }

        /* already process in storage block */
        if (GetVariableBlockID(variable) != gcvBLOCK_INDEX_DEFAULT)
        {
            continue;
        }

        initialRegIndex = _GetStartRegIndex(Shader, variable);
        if(initialRegIndex != (gctUINT32) -1 &&
           variable->prevSibling == -1) {
            virErrCode =_ConvShaderVariable2Vir(Shader,
                                                Shader->variables[i],
                                                virRegMapArr,
                                                initialRegIndex,
                                                VirShader,
                                                gcvNULL,
                                                gcvNULL,
                                                &startRegIndex,
                                                &endRegIndex);
            ON_ERROR2STATUS(virErrCode, "Failed to _ConvShaderVariable2Vir");
            gcmASSERT(Shader->variables[i]->offset >= 0 || Shader->variables[i]->tempIndex == startRegIndex);
        }
    }

    /* Kernel Functions. */
    for (i = 0; i < Shader->kernelFunctionCount; i++)
    {
        gcKERNEL_FUNCTION kfunction = Shader->kernelFunctions[i];

        if (kfunction == gcvNULL)
        {
            continue;
        }
        else if (IsFunctionNotUsed(kfunction) ||
            (kfunction != Shader->currentKernelFunction && !kfunction->isCalledByEntryKernel))
        {
            /* mark the instructions in the unused function as UNUSED */
            for (j = 0; j < kfunction->codeCount; j++)
            {
                virInstMap[kfunction->codeStart + j].inFunction = UNUSED_FUNCTION;
            }
            continue;
        }

        virErrCode =_ConvShaderKernelFunction2Vir(Shader,
                                            kfunction,
                                            VirShader,
                                            virRegMapArr,
                                            virUniformIdArr,
                                            &virFunction);

        if (virErrCode != VSC_ERR_NONE)
        {
            status = gcvSTATUS_INVALID_ARGUMENT;
            goto OnError;
        }

        if (!kfunction->isMain)
        {
            for (j = 0; j < kfunction->codeCount; j++)
            {
                virInstMap[kfunction->codeStart + j].inFunction = virFunction;
            }

            for (j = 0; j < kfunction->tempIndexCount; j++)
            {
                if(virRegMapArr[kfunction->tempIndexStart + j].inFunction) continue;
                virRegMapArr[kfunction->tempIndexStart + j].inFunction = virFunction;
            }
        }
        else
        {
            /* Do we always only have one kernel Function? */
            kernelFunc = virFunction;
        }
    }

    /* Do main function */
    virErrCode = VIR_Shader_AddFunction(VirShader,
                                        gcvFALSE, /* IsKernel */
                                        "main",
                                        VIR_TYPE_UNKNOWN,
                                        &virFunction);

    ON_ERROR2STATUS(virErrCode, "Failed to add VIR Function");
    gcmASSERT(virFunction);

    virFunction->flags |= VIR_FUNCFLAG_MAIN;

    if (kernelFunc != gcvNULL)
    {
        virFunction->die = kernelFunc->die;
    }

    /* Mark all instructions in main */
    for (i = 0; i < Shader->codeCount; i ++) {
        if(virInstMap[i].inFunction) continue;
        virInstMap[i].inFunction = virFunction;
    }

    /* go through all code */
    virInst = gcvNULL;
    for (i = 0; i < Shader->codeCount; i++)
    {
        if (virInstMap[i].inFunction == UNUSED_FUNCTION)
        {
            continue;
        }
        virErrCode =_ConvCode2VirInstruction(Shader,
                                             VirShader,
                                             virInstMap[i].inFunction,
                                             Shader->code + i,
                                             i,
                                             virRegMapArr,
                                             virAttrIdArr,
                                             virOutIdArr,
                                             virUniformIdArr,
                                             virInstMap,
                                             externOpcodeTable,
                                             &virInst);
        ON_ERROR2STATUS(virErrCode, "Failed to ConvCode2VirInstruction");

        virInstMap[i].inst = virInst;
        if(virInstMap[i].labelId != VIR_INVALID_ID) {
            VIR_Label *virLabel;
            VIR_Instruction *labelInst;

            virLabel = VIR_GetLabelFromId(virInstMap[i].inFunction,
                                          virInstMap[i].labelId);
            virErrCode = VIR_Function_AddInstructionBefore(virInstMap[i].inFunction,
                                                           VIR_OP_LABEL,
                                                           VIR_TYPE_UNKNOWN,
                                                           virInst,
                                                           gcvTRUE,
                                                           &labelInst);
            ON_ERROR2STATUS(virErrCode, "Failed to AddInstructionBefore");

            virLabel->defined = labelInst;
            VIR_Operand_SetLabel(labelInst->dest, virLabel);
        }
    }

    {
        /* transform patterns in the VIR */
        VIR_PatternGCSL2VirContext context;
        VSC_CONTEXT vscContext = {0};
        VSC_CORE_SYS_CONTEXT coreSysCtx;
        VSC_SYS_CONTEXT sysCtx;
        sysCtx.pCoreSysCtx = &coreSysCtx;
        coreSysCtx.hwCfg = *hwCfg;
        vscContext.pSysCtx = &sysCtx;
        VIR_PatternContext_Initialize(&context.header, &vscContext, VirShader, &mp.mmWrapper, VIR_PATN_CONTEXT_FLAG_NONE, _GetgcSL2VirPatterns, _CmpInstruction, 0);
        context.externOpcodeTable = externOpcodeTable;
        context.virRegMapArr = virRegMapArr;
        if (gcSHADER_DumpCodeGenVerbose(Shader))
        {
            VIR_Shader_Dump(gcvNULL, "Converted VIR shader before patternMatch IR.", VirShader, gcvTRUE);
        }
        virErrCode = VIR_Pattern_Transform((VIR_PatternContext *)&context);
        VIR_PatternContext_Finalize(&context.header);

        virErrCode = VIR_Lower_MiddleLevel_Process_Intrinsics(VirShader);
        ON_ERROR2STATUS(virErrCode, "Failed to process intrinsics at middle level");
    }

    {
        /* check dual16 mode */
        gcShaderCodeInfo    codeInfo;
        gcoOS_ZeroMemory(&codeInfo, gcmSIZEOF(codeInfo));
        gcSHADER_CountCode(Shader, &codeInfo);

        /* Determine if shader is in dual-16 mode. */
        VirShader->__IsDual16Shader = gcvFALSE;
        if(!gcUseFullNewLinker(hwCfg->hwFeatureFlags.hasHalti2) &&
           !VIR_Shader_IsCL(VirShader) &&
           (GC_ENABLE_DUAL_FP16 > 0 && hwCfg->hwFeatureFlags.supportDual16))
        {
            VirShader->__IsDual16Shader = gcSHADER_IsDual16Shader(Shader, &codeInfo);
        }

        VirShader->__IsMasterDual16Shader = Shader->isMasterDual16Shader;
        VirShader->_enableDefaultUBO = Shader->enableDefaultUBO;

    }
    /* handling ES3.1 data */

    switch (VirShader->shaderKind)
    {
    case VIR_SHADER_COMPUTE:
        for (i = 0; i < 3; i++)
        {
            VirShader->shaderLayout.compute.workGroupSize[i] = Shader->shaderLayout.compute.workGroupSize[i];
        }
        VIR_Shader_SetWorkGroupSizeFixed(VirShader, Shader->shaderLayout.compute.isWorkGroupSizeFixed);
        break;
    case VIR_SHADER_TESSELLATION_CONTROL:
        VirShader->shaderLayout.tcs.tcsPatchOutputVertices = Shader->shaderLayout.tcs.tcsPatchOutputVertices;
        break;
    case VIR_SHADER_TESSELLATION_EVALUATION:
        VirShader->shaderLayout.tes.tessPrimitiveMode =
            (VIR_TessPrimitiveMode)Shader->shaderLayout.tes.tessPrimitiveMode;
        VirShader->shaderLayout.tes.tessVertexSpacing =
            (VIR_TessVertexSpacing)Shader->shaderLayout.tes.tessVertexSpacing;
        VirShader->shaderLayout.tes.tessOrdering =
            (VIR_TessOrdering)Shader->shaderLayout.tes.tessOrdering;
        VirShader->shaderLayout.tes.tessPointMode =
            Shader->shaderLayout.tes.tessPointMode;
        VirShader->shaderLayout.tes.tessPatchInputVertices =
            Shader->shaderLayout.tes.tessPatchInputVertices;
        break;
    case VIR_SHADER_GEOMETRY:
        VirShader->shaderLayout.geo.geoInvocations =
            Shader->shaderLayout.geo.geoInvocations;
        VirShader->shaderLayout.geo.geoMaxVertices =
            Shader->shaderLayout.geo.geoMaxVertices;
        VirShader->shaderLayout.geo.geoInPrimitive =
            (VIR_GeoPrimitive)Shader->shaderLayout.geo.geoInPrimitive;
        VirShader->shaderLayout.geo.geoOutPrimitive =
            (VIR_GeoPrimitive)Shader->shaderLayout.geo.geoOutPrimitive;
        break;
    default:
        break;
    }

    /* Save the private memory size. */
    VIR_Shader_SetPrivateMemorySize(VirShader, GetShaderPrivateMemorySize(Shader));

    /* Save the constant memory size. */
    VIR_Shader_SetConstantMemorySize(VirShader,GetShaderConstantMemorySize(Shader));
    VirShader->constantMemoryBuffer = Shader->constantMemoryBuffer;


OnError:
    vscPMP_Finalize(&mp);
    /* finailize, free memory */
    if (virInstMap != gcvNULL)
    {
        gcmOS_SAFE_FREE(gcvNULL, virInstMap);
    }
    if (virAttrIdArr != gcvNULL)
    {
        gcmOS_SAFE_FREE(gcvNULL, virAttrIdArr);
    }
    if (virOutIdArr != gcvNULL)
    {
        gcmOS_SAFE_FREE(gcvNULL, virOutIdArr);
    }
    if (virRegMapStart != gcvNULL)
    {
        gcmOS_SAFE_FREE(gcvNULL, virRegMapStart);
    }
    if (virUniformIdArr != gcvNULL)
    {
        gcmOS_SAFE_FREE(gcvNULL, virUniformIdArr);
    }
    /* Success. */
    gcmFOOTER_ARG("VirShader=0x%x", VirShader);
    return status;
}

static void
_Conv2FloatType(
    IN VIR_Shader  *Shader,
    IN VIR_Operand *Opnd
    )
{
    VIR_Type           *type     = VIR_Shader_GetTypeFromId(Shader,
            VIR_Operand_GetTypeId(Opnd));

    VIR_PrimitiveTypeId baseType;
    VIR_PrimitiveTypeId convType = VIR_TYPE_FLOAT32;

    baseType = VIR_Type_GetBaseTypeId(type);

    if(VIR_GetTypeFlag(baseType) & VIR_TYFLAG_ISFLOAT)
    {
        return;
    }

    if(VIR_GetTypeFlag(baseType) & VIR_TYFLAG_ISINTEGER)
    {
        switch(baseType)
        {
        case VIR_TYPE_INT32:
        case VIR_TYPE_BOOLEAN:
        case VIR_TYPE_UINT32:
            convType = VIR_TYPE_FLOAT32;
            break;
        case VIR_TYPE_INTEGER_X2:
        case VIR_TYPE_BOOLEAN_X2:
        case VIR_TYPE_UINT_X2:
            convType = VIR_TYPE_FLOAT_X2;
            break;
        case VIR_TYPE_INTEGER_X3:
        case VIR_TYPE_BOOLEAN_X3:
        case VIR_TYPE_UINT_X3:
            convType = VIR_TYPE_FLOAT_X3;
            break;
        case VIR_TYPE_INTEGER_X4:
        case VIR_TYPE_BOOLEAN_X4:
        case VIR_TYPE_UINT_X4:
            convType = VIR_TYPE_FLOAT_X4;
            break;
        case VIR_TYPE_INTEGER_X8:
        case VIR_TYPE_BOOLEAN_X8:
        case VIR_TYPE_UINT_X8:
            convType = VIR_TYPE_FLOAT_X8;
            break;
        case VIR_TYPE_INTEGER_X16:
        case VIR_TYPE_BOOLEAN_X16:
        case VIR_TYPE_UINT_X16:
            convType = VIR_TYPE_FLOAT_X16;
            break;
        case VIR_TYPE_INT8:
        case VIR_TYPE_UINT8:
        case VIR_TYPE_INT16:
        case VIR_TYPE_UINT16:
            convType = VIR_TYPE_FLOAT16;
            break;
        case VIR_TYPE_INT8_X2:
        case VIR_TYPE_UINT8_X2:
        case VIR_TYPE_INT16_X2:
        case VIR_TYPE_UINT16_X2:
            convType = VIR_TYPE_FLOAT16_X2;
            break;
        case VIR_TYPE_INT8_X3:
        case VIR_TYPE_UINT8_X3:
        case VIR_TYPE_INT16_X3:
        case VIR_TYPE_UINT16_X3:
            convType = VIR_TYPE_FLOAT16_X3;
            break;
        case VIR_TYPE_INT8_X4:
        case VIR_TYPE_UINT8_X4:
        case VIR_TYPE_INT16_X4:
        case VIR_TYPE_UINT16_X4:
            convType = VIR_TYPE_FLOAT16_X4;
            break;
        case VIR_TYPE_INT8_X8:
        case VIR_TYPE_UINT8_X8:
        case VIR_TYPE_INT16_X8:
        case VIR_TYPE_UINT16_X8:
            convType = VIR_TYPE_FLOAT16_X8;
            break;
        case VIR_TYPE_INT8_X16:
        case VIR_TYPE_UINT8_X16:
        case VIR_TYPE_INT16_X16:
        case VIR_TYPE_UINT16_X16:
            convType = VIR_TYPE_FLOAT16_X16;
            break;
        case VIR_TYPE_INT64:
        case VIR_TYPE_UINT64:
            convType = VIR_TYPE_FLOAT64;
            break;
        case VIR_TYPE_SNORM8:
        case VIR_TYPE_UNORM8:
        case VIR_TYPE_SNORM16:
        case VIR_TYPE_UNORM16:
            convType = VIR_TYPE_FLOAT32;
            break;
        default:
            gcmASSERT(0);
            break;
        }
    }
    VIR_Operand_SetTypeId(Opnd, convType);
}

static void
_Conv2IntegerType(
    IN VIR_Shader  *Shader,
    IN VIR_Operand *Opnd
    )
{
    VIR_Type           *type     = VIR_Shader_GetTypeFromId(Shader,
            VIR_Operand_GetTypeId(Opnd));

    VIR_PrimitiveTypeId baseType;
    VIR_PrimitiveTypeId convType = VIR_TYPE_FLOAT32;

    gcmASSERT(VIR_Type_isPrimitive(type));

    baseType = VIR_Type_GetBaseTypeId(type);

    if(VIR_GetTypeFlag(baseType) & VIR_TYFLAG_ISINTEGER)
    {
        return;
    }

    if(VIR_GetTypeFlag(baseType) & VIR_TYFLAG_ISFLOAT)
    {
        switch(baseType)
        {
        case VIR_TYPE_FLOAT32:
            convType = VIR_TYPE_INT32;
            break;
        case VIR_TYPE_FLOAT_X2:
            convType = VIR_TYPE_INTEGER_X2;
            break;
        case VIR_TYPE_FLOAT_X3:
            convType = VIR_TYPE_INTEGER_X3;
            break;
        case VIR_TYPE_FLOAT_X4:
            convType = VIR_TYPE_INTEGER_X4;
            break;
        case VIR_TYPE_FLOAT_X8:
            convType = VIR_TYPE_INTEGER_X8;
            break;
        case VIR_TYPE_FLOAT_X16:
            convType = VIR_TYPE_INTEGER_X16;
            break;
        case VIR_TYPE_FLOAT16:
            convType = VIR_TYPE_INT16;
            break;
        case VIR_TYPE_FLOAT16_X2:
            convType = VIR_TYPE_INT16_X2;
            break;
        case VIR_TYPE_FLOAT16_X3:
            convType = VIR_TYPE_INT16_X3;
            break;
        case VIR_TYPE_FLOAT16_X4:
            convType = VIR_TYPE_INT16_X4;
            break;
        case VIR_TYPE_FLOAT16_X8:
            convType = VIR_TYPE_INT16_X8;
            break;
        case VIR_TYPE_FLOAT16_X16:
            convType = VIR_TYPE_INT16_X16;
            break;
        case VIR_TYPE_FLOAT64:
            convType = VIR_TYPE_INT64;
            break;
        default:
            gcmASSERT(0);
            break;
        }
    }
    VIR_Operand_SetTypeId(Opnd, convType);
}


static gctBOOL
_SetTexBias(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_Instruction *biasInst  = VIR_Inst_GetNext(Inst);

    gcmASSERT(biasInst != gcvNULL);

    VIR_Operand_SetTexldBias(Inst->src[2], biasInst->src[1]);
    biasInst->src[1] = gcvNULL;

    return gcvTRUE;
}

static gctBOOL
_SetTexGrad(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_Instruction *gradInst = VIR_Inst_GetNext(Inst);
    gcmASSERT(gradInst != gcvNULL);

    VIR_Operand_SetTexldGradient(Inst->src[2], gradInst->src[0], gradInst->src[1]);
    gradInst->src[0] = gcvNULL;
    gradInst->src[1] = gcvNULL;
    return gcvTRUE;
}

static gctBOOL
_SetTexLod(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_Instruction *lodInst   = VIR_Inst_GetNext(Inst);

    gcmASSERT(lodInst != gcvNULL);

    VIR_Operand_SetTexldLod(Inst->src[2], lodInst->src[1]);
    lodInst->src[1] = gcvNULL;

    return gcvTRUE;
}

static gctBOOL
_SetTexGather(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_Instruction *gatherInst = VIR_Inst_GetNext(Inst);
    gcmASSERT(gatherInst != gcvNULL);

    VIR_Operand_SetTexldGather(Inst->src[2], gatherInst->src[0], gatherInst->src[1]);
    gatherInst->src[0] = gatherInst->src[1] = gcvNULL;

    return gcvTRUE;
}

static gctBOOL
_SetTexFetchMS(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_Instruction *fetchMSInst = VIR_Inst_GetNext(Inst);
    gcmASSERT(fetchMSInst != gcvNULL);

    VIR_Operand_SetTexldFetchMS(Inst->src[2], fetchMSInst->src[1]);
    fetchMSInst->src[1] = gcvNULL;

    return gcvTRUE;
}

#define MERGE_OPCODE(Op, EXTERN_OP) ((Op) | ((EXTERN_OP) << 16))

/*
        CMP 1, 2, 3
        CMP.z  1, 1, 4
        CMP.nz 1, 1, 3
            select 4, 2, 5, 3, 0

static VIR_PatternInst _cmpPatInst0[] = {
    { VIR_OP_COMPARE, VIR_PATTERN_ANYCOND, { 1, 2, 3, 0 }, 0 },
    { VIR_OP_COMPARE, VIR_COP_NOT, { 1, 1, 4, 0 }, 0 },
    { VIR_OP_COMPARE, VIR_COP_NOT_ZERO, { 1, 1, 3, 0 }, 0 },
};

static VIR_PatternInst _cmpRepInst0[] = {
    { VIR_OP_CSELECT, -1, { 1, 2, 3, 4 }, 0 },
};
*/

     /*
        SET 1, 2, 3
        CMP.z  4, 1, 4
        CMP.nz 4, 1, 3
            select 4, 2, 3, 4
    */
static VIR_PatternMatchInst _setPatInst0[] = {
    { VIR_OP_SET, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_COMPARE, VIR_COP_NOT, 0, { 4, 1, 4, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_COMPARE, VIR_COP_NOT_ZERO, 0, { 4, 1, 3, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _setRepInst0[] = {
    { VIR_OP_SELECT, -1, 0, { 4, 2, 3, 4 }, { 0 } },
};

     /*
        SET 1, 2, 3
        CMP.z  4, 1, 2
        CMP.nz 4, 1, 3
            select 4, 2, 3, 2
    */
static VIR_PatternMatchInst _setPatInst1[] = {
    { VIR_OP_SET, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_COMPARE, VIR_COP_NOT, 0, { 4, 1, 2, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_COMPARE, VIR_COP_NOT_ZERO, 0, { 4, 1, 3, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _setRepInst1[] = {
    { VIR_OP_SELECT, -1, 0, { 4, 2, 3, 2 }, { 0 } },
};

     /*
        SET 1, 2, 3
        CMP.z  4, 1, 5
        CMP.nz 4, 1, 3
            select 4, 2, 3, 5
    */
static VIR_PatternMatchInst _setPatInst2[] = {
    { VIR_OP_SET, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_COMPARE, VIR_COP_NOT, 0, { 4, 1, 5, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_COMPARE, VIR_COP_NOT_ZERO, 0, { 4, 1, 3, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _setRepInst2[] = {
    { VIR_OP_SELECT, -1, 0, { 4, 2, 3, 5 }, { 0 } },
};

     /*
        SET    1, 2, 3
        MOV    4, 5
        CMP.z  5, 1, 4
        CMP.nz 5, 1, 3
            select 5, 2, 3, 5
    */
static VIR_PatternMatchInst _setPatInst3[] = {
    { VIR_OP_SET, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_MOV, VIR_PATTERN_ANYCOND, 0, { 4, 5, 0, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_COMPARE, VIR_COP_NOT, 0, { 5, 1, 4, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_COMPARE, VIR_COP_NOT_ZERO, 0, { 5, 1, 3, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _setRepInst3[] = {
    { VIR_OP_MOV, 0, 0, { 4, 5, 0, 0 }, { 0 } },
    { VIR_OP_SELECT, -1, 0, { 5, 2, 3, 5 }, { 0 } },
};

     /*
        SET    1, 2, 3
        CMP.z  4, 1, 5
        MOV    6, 4
        CMP.nz 4, 1, 6
            select 4, 2, 3, 5
    */
static VIR_PatternMatchInst _setPatInst4[] = {
    { VIR_OP_SET, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_COMPARE, VIR_COP_NOT, 0, { 4, 1, 5, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_MOV, VIR_PATTERN_ANYCOND, 0, { 6, 4, 0, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_COMPARE, VIR_COP_NOT_ZERO, 0, { 4, 1, 6, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _setRepInst4[] = {
    { VIR_OP_MOV, 0, 0, { 6, 4, 0, 0 }, { 0 } },
    { VIR_OP_SELECT, -1, 0, { 4, 2, 3, 5 }, { 0 } },
};

/*
set.z  1, 2, 3
set.nz 1, 2, 4
    select.nz 1, 2, 3, 4, 0
*/
static VIR_PatternMatchInst _setPatInst5[] = {
    { VIR_OP_SET, VIR_COP_NOT, 0, { 1, 2, 3, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_SET, VIR_COP_NOT_ZERO, 0, { 1, 2, 4, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _setRepInst5[] = {
    { VIR_OP_CSELECT, VIR_COP_NOT_ZERO, 0, { 1, 2, 4, 3 }, { 0 } },
};

static VIR_Pattern _setPattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_set, 0) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_set, 1) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_set, 2) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_set, 3) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_set, 4) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_set, 5) },
    { VIR_PATN_FLAG_NONE }
};

static
gctBOOL _ConditionWithZero(
    IN VIR_PatternContext   *Context,
    IN VIR_Instruction      *Inst
    )
{
    return VIR_ConditionOp_ComparingWithZero(VIR_Inst_GetConditionOp(Inst));
}

static
gctBOOL _ConditionReversedWithPrev(
    IN VIR_PatternContext   *Context,
    IN VIR_Instruction      *Inst
    )
{
    return VIR_Inst_GetConditionOp(Inst) == VIR_ConditionOp_Reverse(VIR_Inst_GetConditionOp(VIR_Inst_GetPrev(Inst)));
}
/*
cmp.z  1, 2, 3
cmp.nz 1, 2, 4
    select.nz 1, 2, 3, 4, 0
*/
static VIR_PatternMatchInst _cmpPatInst0[] = {
    { VIR_OP_COMPARE, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { _ConditionWithZero }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_COMPARE, VIR_PATTERN_ANYCOND, 0, { 1, 2, 4, 0 }, { _ConditionReversedWithPrev }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _cmpRepInst0[] = {
    { VIR_OP_CSELECT, -1, 0, { 1, 2, 3, 4 }, { 0 } },
};

static gctBOOL
_isSrc1ConstInteger31(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    VIR_Operand * src1 = VIR_Inst_GetSource(Inst, 1);
    VIR_TypeId    tyId;

    if (VIR_Operand_GetOpKind(src1) == VIR_OPND_IMMEDIATE)
    {
        tyId = VIR_Operand_GetTypeId(src1);
        if (VIR_TypeId_isPrimitive(tyId))
        {
            if (VIR_TypeId_isSignedInteger(tyId) || VIR_TypeId_isUnSignedInteger(tyId))
            {
                gctUINT uValue = VIR_Operand_GetImmediateUint(src1);
                return uValue == 31;
            }
        }
    }

    return gcvFALSE;
}

/*
cmp.z  1, 2, 3
cmp.nz 1, 2, 4
    select.nz 1, 2, 3, 4, 0
*/
static VIR_PatternMatchInst _cmpPatInst1[] = {
    { VIR_OP_COMPARE, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_RSHIFT, VIR_PATTERN_ANYCOND, 0, { 4, 1, 5, 0 }, { 0, 0, _isSrc1ConstInteger31 }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_SET, VIR_COP_NOT, 0, { 6, 4, 7, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_SET, VIR_COP_NOT_ZERO, 0, { 6, 4, 3, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _cmpRepInst1[] = {
    { VIR_OP_SELECT, -1, 0, { 6, 2, 3, 7 }, { 0 } },
};

static VIR_PatternMatchInst _cmpPatInst2[] = {
    { VIR_OP_COMPARE, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_RSHIFT, VIR_PATTERN_ANYCOND, 0, { 4, 1, 5, 0 }, { 0, 0, _isSrc1ConstInteger31 }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_SET, VIR_COP_NOT, 0, { 6, 4, 3, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_SET, VIR_COP_NOT_ZERO, 0, { 6, 4, 2, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _cmpRepInst2[] = {
    { VIR_OP_SELECT, -1, 0, { 6, 3, 2, 3 }, { VIR_Lower_ReverseCondOp } },
};

static VIR_Pattern _cmpPattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_cmp, 0) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_cmp, 1) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_cmp, 2) },
    { VIR_PATN_FLAG_NONE }
};

/*
texbias *0, 1, 2
texld    3, *1, 4
    texld 3, 1, 4, 2
*/

static VIR_PatternMatchInst _texldPatInst0[] = {
    { MERGE_OPCODE(VIR_OP_TEXLD, EXTERN_TEXBIAS), VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_TEXLD, VIR_PATTERN_ANYCOND, 0, { 4, 2, 5, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _texldRepInst0[] = {
    { VIR_OP_TEXLD, -2, 0, { 4, 2, 5, 0 }, { _SetTexBias } },
};

/*
texgrad *0, 1, 2
texld    3, 4, 5
    texld 3, 4, 5, [ 1, 2 ]
*/

static VIR_PatternMatchInst _texldPatInst1[] = {
    { MERGE_OPCODE(VIR_OP_TEXLD, EXTERN_TEXGRAD), VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_TEXLD, VIR_PATTERN_ANYCOND, 0, { 4, 5, 6, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _texldRepInst1[] = {
    { VIR_OP_TEXLD, -2, 0, { 4, 5, 6, 0 }, { _SetTexGrad } },
};

/*
texlod  *0, 1, 2
texld    3, *1, 4
    texld 3, 1, 4, 2
*/

static VIR_PatternMatchInst _texldPatInst2[] = {
    { MERGE_OPCODE(VIR_OP_TEXLD, EXTERN_TEXLOD), VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_TEXLD, VIR_PATTERN_ANYCOND, 0, {4, 2, 5, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _texldRepInst2[] = {
    { VIR_OP_TEXLD, -2, 0, { 4, 2, 5, 0 }, { _SetTexLod } },
};

/*
texgather *0, 1, 2
texld      3, 4, 5
    texld 3, 4, 5, [ 1, 2 ]
*/
static VIR_PatternMatchInst _texldPatInst3[] = {
    { MERGE_OPCODE(VIR_OP_TEXLD, EXTERN_TEXGATHER), VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_TEXLD, VIR_PATTERN_ANYCOND, 0, { 4, 5, 6, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _texldRepInst3[] = {
    { VIR_OP_TEXLD, -2, 0, { 4, 5, 6, 0 }, { _SetTexGather } },
};

/*
texfetchMS *0, 1, 2
texld      3, *1, 4
    texld 3, 1, 4, 2
*/
static VIR_PatternMatchInst _texldPatInst4[] = {
    { MERGE_OPCODE(VIR_OP_TEXLD, EXTERN_TEXFETCH_MS), VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_TEXLD, VIR_PATTERN_ANYCOND, 0, { 4, 2, 5, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _texldRepInst4[] = {
    { VIR_OP_TEXLD, -2, 0, { 4, 2, 5, 0 }, { _SetTexFetchMS } },
};

/*
texu      1, 2
texld     3, 4, 5
    texld_u_plain 3, 4, 5, 2
*/
static VIR_PatternMatchInst _texldPatInst5[] = {
    { MERGE_OPCODE(VIR_OP_TEXLD, EXTERN_TEXU), VIR_PATTERN_ANYCOND, 0, { 1, 2, 0, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_TEXLD, VIR_PATTERN_ANYCOND, 0, { 3, 4, 5, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _texldRepInst5[] = {
    { VIR_OP_TEXLD_U_PLAIN, -2, 0, { 3, 4, 5, 2 }, { 0 } },
};

/*
texu_lod   1, 2, 3
texld      1, 4, 5
    texld_u_lod 1, 4, 5, 2, 3
*/
static VIR_PatternMatchInst _texldPatInst6[] = {
    { MERGE_OPCODE(VIR_OP_TEXLD, EXTERN_TEXU_LOD), VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_TEXLD, VIR_PATTERN_ANYCOND, 0, { 1, 4, 5, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _texldRepInst6[] = {
    { VIR_OP_TEXLD_U_LOD, -2, 0, { 1, 4, 5, 2, 3}, { 0 } },
};

/*
texbias *0, 1, 2
texldpcf 3, *1, 4
    texld 3, 1, 4, 2
*/
static VIR_PatternMatchInst _texldpcfPatInst0[] = {
    { MERGE_OPCODE(VIR_OP_TEXLD, EXTERN_TEXBIAS), VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_TEXLDPCF, VIR_PATTERN_ANYCOND, 0, { 4, 2, 5, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _texldpcfRepInst0[] = {
    { VIR_OP_TEXLDPCF, -2, 0, { 4, 2, 5, 0 }, { _SetTexBias } },
};

/*
texgrad *0, 1, 2
texldpcf 3, 4, 5
    texld 3, 4, 5, [ 1, 2 ]
*/
static VIR_PatternMatchInst _texldpcfPatInst1[] = {
    { MERGE_OPCODE(VIR_OP_TEXLD, EXTERN_TEXGRAD), VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_TEXLDPCF, VIR_PATTERN_ANYCOND, 0, { 4, 5, 6, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _texldpcfRepInst1[] = {
    { VIR_OP_TEXLDPCF, -2, 0, { 4, 5, 6, 0 }, { _SetTexGrad } },
};

/*
texlod  *0, 1, 2
texldpcf 3, *1, 4
    texld 3, 1, 4, 2
*/
static VIR_PatternMatchInst _texldpcfPatInst2[] = {
    { MERGE_OPCODE(VIR_OP_TEXLD, EXTERN_TEXLOD), VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_TEXLDPCF, VIR_PATTERN_ANYCOND, 0, { 4, 2, 5, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _texldpcfRepInst2[] = {
    { VIR_OP_TEXLDPCF, -2, 0, { 4, 2, 5, 0 }, { _SetTexLod } },
};

/*
texgather *0, 1, 2
texldpcf 3, 4, 5
    texld 3, 4, 5, [ 1, 2 ]
*/
static VIR_PatternMatchInst _texldpcfPatInst3[] = {
    { MERGE_OPCODE(VIR_OP_TEXLD, EXTERN_TEXGATHER), VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_TEXLDPCF, VIR_PATTERN_ANYCOND, 0, { 4, 5, 6, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _texldpcfRepInst3[] = {
    { VIR_OP_TEXLDPCF, -2, 0, { 4, 5, 6, 0 }, { _SetTexGather } },
};

/*
texbias *0, 1, 2
texldproj 3, *1, 4
    texld 3, 1, 4, 2
*/
static VIR_PatternMatchInst _texldprojPatInst0[] = {
    { MERGE_OPCODE(VIR_OP_TEXLD, EXTERN_TEXBIAS), VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_TEXLDPROJ, VIR_PATTERN_ANYCOND, 0, { 4, 2, 5, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _texldprojRepInst0[] = {
    { VIR_OP_TEXLDPROJ, -2, 0, { 4, 2, 5, 0 }, { _SetTexBias } },
};

/*
texgrad *0, 1, 2
texldproj 3, 4, 5
    texld 3, 4, 5, [ 1, 2 ]
*/
static VIR_PatternMatchInst _texldprojPatInst1[] = {
    { MERGE_OPCODE(VIR_OP_TEXLD, EXTERN_TEXGRAD), VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_TEXLDPROJ, VIR_PATTERN_ANYCOND, 0, { 4, 5, 6, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _texldprojRepInst1[] = {
    { VIR_OP_TEXLDPROJ, -2, 0, { 4, 5, 6, 0 }, { _SetTexGrad } },
};

/*
texlod  *0, 1, 2
texldproj 3, *1, 4
    texld 3, 1, 4, 2
*/
static VIR_PatternMatchInst _texldprojPatInst2[] = {
    { MERGE_OPCODE(VIR_OP_TEXLD, EXTERN_TEXLOD), VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_TEXLDPROJ, VIR_PATTERN_ANYCOND, 0, { 4, 2, 5, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _texldprojRepInst2[] = {
    { VIR_OP_TEXLDPROJ, -2, 0, { 4, 2, 5, 0 }, { _SetTexLod } },
};

/*
texbias *0, 1, 2
texldpcfproj 3, *1, 4
    texld 3, 1, 4, 2
*/
static VIR_PatternMatchInst _texldpcfprojPatInst0[] = {
    { MERGE_OPCODE(VIR_OP_TEXLD, EXTERN_TEXBIAS), VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_TEXLDPCFPROJ, VIR_PATTERN_ANYCOND, 0, { 4, 2, 5, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _texldpcfprojRepInst0[] = {
    { VIR_OP_TEXLDPCFPROJ, -2, 0, { 4, 2, 5, 0 }, { _SetTexBias } },
};

/*
texgrad *0, 1, 2
texldpcfproj 3, 4, 5
    texld 3, 4, 5, [ 1, 2 ]
*/
static VIR_PatternMatchInst _texldpcfprojPatInst1[] = {
    { MERGE_OPCODE(VIR_OP_TEXLD, EXTERN_TEXGRAD), VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_TEXLDPCFPROJ, VIR_PATTERN_ANYCOND, 0, { 4, 5, 6, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _texldpcfprojRepInst1[] = {
    { VIR_OP_TEXLDPCFPROJ, -2, 0, { 4, 5, 6, 0 }, { _SetTexGrad } },
};

/*
texlod  *0, 1, 2
texldpcfproj 3, *1, 4
    texld 3, 1, 4, 2
*/
static VIR_PatternMatchInst _texldpcfprojPatInst2[] = {
    { MERGE_OPCODE(VIR_OP_TEXLD, EXTERN_TEXLOD), VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_TEXLDPCFPROJ, VIR_PATTERN_ANYCOND, 0, { 4, 2, 5, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _texldpcfprojRepInst2[] = {
    { VIR_OP_TEXLDPCFPROJ, -2, 0, { 4, 2, 5, 0 }, { _SetTexLod } },
};

static VIR_Pattern _texldPattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_texld, 0) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_texld, 1) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_texld, 2) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_texld, 3) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_texld, 4) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_texld, 5) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_texld, 6) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_texldpcf, 0) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_texldpcf, 1) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_texldpcf, 2) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_texldpcf, 3) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_texldproj, 0) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_texldproj, 1) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_texldproj, 2) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_texldpcfproj, 0) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_texldpcfproj, 1) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_texldpcfproj, 2) },
    { VIR_PATN_FLAG_NONE }
};

/*
texlod  *0, 1, 2
texld_u    3, *1, 4
    texld_u 3, 1, 4, 2
*/
static VIR_PatternMatchInst _texlduPatInst0[] = {
    { MERGE_OPCODE(VIR_OP_TEXLD_U, EXTERN_TEXLOD), VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_TEXLD_U, VIR_PATTERN_ANYCOND, 0, {4, 2, 5, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _texlduRepInst0[] = {
    { VIR_OP_TEXLD_U, -2, 0, { 4, 2, 5, 0 }, { _SetTexLod } },
};

/*
texbias  *0, 1, 2
texld_u    3, *1, 4
    texld_u 3, 1, 4, 2
*/
static VIR_PatternMatchInst _texlduPatInst1[] = {
    { MERGE_OPCODE(VIR_OP_TEXLD_U, EXTERN_TEXBIAS), VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_TEXLD_U, VIR_PATTERN_ANYCOND, 0, {4, 2, 5, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _texlduRepInst1[] = {
    { VIR_OP_TEXLD_U, -2, 0, { 4, 2, 5, 0 }, { _SetTexBias } },
};

static VIR_Pattern _texlduPattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_texldu, 0) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_texldu, 1) },
    { VIR_PATN_FLAG_NONE }
};

static gctBOOL
_SetI2F(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    _Conv2IntegerType(Context->shader, Inst->src[0]);
    _Conv2FloatType(Context->shader, Inst->dest);
    return gcvTRUE;
}
/*
i2f   1, 2
    conv 1, 2
*/
static VIR_PatternMatchInst _convPatInst0[] = {
    { MERGE_OPCODE(VIR_OP_CONVERT, EXTERN_I2F), VIR_PATTERN_ANYCOND, 0, { 1, 2, 0, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _convRepInst0[] = {
    { VIR_OP_CONVERT, -1, 0, { 1, 2 }, { _SetI2F } },
};

static gctBOOL
_SetF2I(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    _Conv2FloatType(Context->shader, Inst->src[0]);
    _Conv2IntegerType(Context->shader, Inst->dest);
    return gcvTRUE;
}

/*
f2i   1, 2
    conv 1, 2
*/
static VIR_PatternMatchInst _convPatInst1[] = {
    { MERGE_OPCODE(VIR_OP_CONVERT, EXTERN_F2I), VIR_PATTERN_ANYCOND, 0, { 1, 2, 0, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _convRepInst1[] = {
    { VIR_OP_CONVERT, -1, 0, { 1, 2 }, { _SetF2I } },
};

static VIR_Pattern _convPattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_conv, 0) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_conv, 1) },
    { VIR_PATN_FLAG_NONE }
};

static gctBOOL
_SetConvType(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_Instruction *conv0Inst = VIR_Inst_GetNext(Inst);
    VIR_TypeId opndTyp = VIR_Operand_GetTypeId(Opnd);

    if (VIR_TypeId_isPrimitive(opndTyp) && VIR_TypeId_isPacked(opndTyp))
    {
        /* if it's packed type, the components is already in operand type */
        return gcvTRUE;
    }
    else
    {
        VIR_TypeId typeId;
        gctUINT components;
        VIR_Operand *dest = VIR_Inst_GetDest(Inst);

        opndTyp = VIR_Operand_GetTypeId(dest);
        if (VIR_TypeId_isPrimitive(opndTyp) && VIR_TypeId_isPacked(opndTyp))
        {
            components = VIR_GetTypePackedComponents(opndTyp);
        }
        else components = _GetEnableComponentCount(VIR_Operand_GetEnable(dest));
        typeId = _ConvScalarFormatToVirVectorTypeId((gcSL_FORMAT)VIR_Operand_GetImmediateUint(conv0Inst->src[1]),
                                                     components, gcvFALSE);
        VIR_Operand_SetTypeId(Opnd, typeId);
    }
    return gcvTRUE;
}

/*
conv0   1, 2, 3
    conv 1, 2
*/
static VIR_PatternMatchInst _conv0PatInst0[] = {
    { VIR_OP_CONV0, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _conv0RepInst0[] = {
    { VIR_OP_CONVERT, -1, 0, { 1, 2 }, { 0, _SetConvType } },
};

static VIR_Pattern _conv0Pattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_conv0, 0) },
    { VIR_PATN_FLAG_NONE }
};

static gctBOOL
int_zero_2(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_ScalarConstVal imm0;
    imm0.iValue = 0;

    VIR_Operand_SetImmediate(Inst->src[1],
        VIR_TYPE_INT32,
        imm0);
    return gcvTRUE;
}


static gctBOOL
_normalize_src2_swiz(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_Enable  enable;
    VIR_Swizzle swizzle;
    VIR_OperandInfo operInfo;

    VIR_Operand_GetOperandInfo(Inst, Inst->src[2], &operInfo);

    if(VIR_OpndInfo_Is_Virtual_Reg(&operInfo))
    {
        enable  = VIR_Inst_GetEnable(Inst);
        swizzle = VIR_Operand_GetSwizzle(Inst->src[2]);
        swizzle = VIR_NormalizeSwizzleByEnable(enable, swizzle);
        VIR_Operand_SetSwizzle(Inst->src[2], swizzle);
    }

    return gcvTRUE;
}

static gctBOOL
_fix_src_type_to_dest_type(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    {
        VIR_Operand *dest;
        VIR_TypeId typeId;
        VIR_BuiltinTypeInfo * typeInfo;

        dest = VIR_Inst_GetDest(Inst);
        gcmASSERT(dest);

        typeId = VIR_Operand_GetTypeId(dest);
        typeInfo = VIR_Shader_GetBuiltInTypes(typeId);

        if(typeInfo->componentType != VIR_TYPE_FLOAT16)
        {
            VIR_Operand_SetTypeId(Opnd, typeId);
        }
    }
    return gcvTRUE;
}

/* image_store, the dest must be VIR_ENABLE_XYZW */
static gctBOOL
set_dest_xyzw(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_Operand_SetEnable(Inst->dest, VIR_ENABLE_XYZW);

    return gcvTRUE;
}

static gctBOOL
_SetTypeIdFromVariable(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_Symbol    *sym;

    sym = VIR_Operand_GetSymbol(Opnd);
    if (sym && VIR_Symbol_GetKind(sym) == VIR_SYM_VIRREG && VIR_Symbol_GetVregVariable(sym))
    {
        VIR_TypeId typeId;

        sym = VIR_Symbol_GetVregVariable(sym); /* sym corresponding to variable */

        typeId = VIR_Symbol_GetTypeId(sym);
        if(VIR_TypeId_isImage(typeId) || VIR_TypeId_isImageT(typeId))
        {
            gctUINT typeSize = VIR_GetTypeSize(typeId);
            VIR_PrimitiveTypeId  format;
            gctUINT components;
            format = VIR_GetTypeComponentType(typeId);
            components = typeSize / VIR_GetTypeSize(format),
            typeId = VIR_TypeId_ComposeNonOpaqueType(format,
                                                     components,
                                                     1);
            VIR_Operand_SetTypeId(Opnd, typeId);
        }
    }
    return gcvTRUE;
}

/*
store   1, 2, 3
    store 0, 1, 2, 3
*/
static VIR_PatternMatchInst _storePatInst0[] = {
    { MERGE_OPCODE(VIR_OP_STARR, EXTERN_STORE), VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _storeRepInst0[] = {
    { VIR_OP_STORE, -1, 0, { 1, 2, 3, 1 }, { _normalize_src2_swiz } },
};

static gctBOOL
_isOCL_VXMode(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    return VIR_Shader_IsCL(Context->shader) &&
           (VIR_Shader_HasVivVxExtension(Context->shader) || VIR_Shader_HasVivGcslDriverImage(Context->shader)) ;
}

/*
store1   1, 2, 3
    store 0, 1, 2, 3
*/
static VIR_PatternMatchInst _storePatInst1[] = {
    { MERGE_OPCODE(VIR_OP_STARR, EXTERN_STORE1), VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _storeRepInst1[] = {
    { VIR_OP_STORE, -1, VIR_PATN_FLAG_COPY_ROUNDING_MODE, { 1, 2, 0, 3 }, { int_zero_2, _normalize_src2_swiz, 0, _fix_src_type_to_dest_type} },};

static VIR_PatternMatchInst _storePatInst2[] = {
    { MERGE_OPCODE(VIR_OP_STARR, EXTERN_IMG_STORE), VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { _isOCL_VXMode }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _storeRepInst2[] = {
    { VIR_OP_IMG_WRITE, 0, VIR_PATTERN_TEMP_TYPE_XYZW | VIR_PATN_FLAG_COPY_ROUNDING_MODE, { -1, 2, 3, 1 }, { set_dest_xyzw, 0, _fix_src_type_to_dest_type, 0 } },
};

static VIR_PatternMatchInst _storePatInst3[] = {
    { MERGE_OPCODE(VIR_OP_STARR, EXTERN_IMG_STORE), VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _storeRepInst3[] = {
    { VIR_OP_IMG_STORE, 0, VIR_PATTERN_TEMP_TYPE_XYZW | VIR_PATN_FLAG_COPY_ROUNDING_MODE, { -1, 2, 3, 1 }, { set_dest_xyzw, 0, _fix_src_type_to_dest_type, 0 } },
};

static VIR_PatternMatchInst _storePatInst4[] = {
    { MERGE_OPCODE(VIR_OP_STARR, EXTERN_IMG_STORE_3D), VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { _isOCL_VXMode }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _storeRepInst4[] = {
    { VIR_OP_IMG_WRITE_3D, 0, VIR_PATTERN_TEMP_TYPE_XYZW | VIR_PATN_FLAG_COPY_ROUNDING_MODE, { -1, 2, 3, 1 }, { set_dest_xyzw } },
};

static VIR_PatternMatchInst _storePatInst5[] = {
    { MERGE_OPCODE(VIR_OP_STARR, EXTERN_IMG_STORE_3D), VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _storeRepInst5[] = {
    { VIR_OP_IMG_STORE_3D, 0, VIR_PATTERN_TEMP_TYPE_XYZW | VIR_PATN_FLAG_COPY_ROUNDING_MODE, { -1, 2, 3, 1 }, { set_dest_xyzw } },
};

/*
store_l:   1, 2, 3
    store 0, 1, 2, 3
*/
static VIR_PatternMatchInst _storePatInst6[] = {
    { MERGE_OPCODE(VIR_OP_STARR, OPCODE_STORE_L), VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _storeRepInst6[] = {
    { VIR_OP_STORE_L, -1, 0, { 1, 2, 0, 3 }, { int_zero_2, _normalize_src2_swiz, 0, _fix_src_type_to_dest_type} },};

static VIR_Pattern _storePattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_store, 0) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_store, 1) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_store, 2) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_store, 3) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_store, 4) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_store, 5) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_store, 6) },
    { VIR_PATN_FLAG_NONE }
};

static VIR_PatternMatchInst _vxImgStorePatInst0[] = {
    { VIR_OP_VX_IMG_STORE, VIR_PATTERN_ANYCOND, 0, { 0, 1, 2, 3, 4 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _vxImgStoreRepInst0[] = {
    { VIR_OP_VX_IMG_STORE, 0, VIR_PATTERN_TEMP_TYPE_XYZW, { -1, 1, 2, 3, 4 }, { set_dest_xyzw, _SetTypeIdFromVariable } },
};

static VIR_Pattern _vxImgStorePattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_vxImgStore, 0) },
    { VIR_PATN_FLAG_NONE }
};

static VIR_PatternMatchInst _vxImgStore3DPatInst0[] = {
    { VIR_OP_VX_IMG_STORE_3D, VIR_PATTERN_ANYCOND, 0, { 0, 1, 2, 3, 4 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _vxImgStore3DRepInst0[] = {
    { VIR_OP_VX_IMG_STORE_3D, 0, VIR_PATTERN_TEMP_TYPE_XYZW, { -1, 1, 2, 3, 4 }, { set_dest_xyzw, _SetTypeIdFromVariable } },
};

static VIR_Pattern _vxImgStore3DPattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_vxImgStore3D, 0) },
    { VIR_PATN_FLAG_NONE }
};

static VIR_PatternMatchInst _vxImgLoadPatInst0[] = {
    { VIR_OP_VX_IMG_LOAD, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 4, 5 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _vxImgLoadRepInst0[] = {
    { VIR_OP_VX_IMG_LOAD, 0, 0, { 1, 2, 3, 4, 5 }, { 0, _SetTypeIdFromVariable } },
};

static VIR_Pattern _vxImgLoadPattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_vxImgLoad, 0) },
    { VIR_PATN_FLAG_NONE }
};

static VIR_PatternMatchInst _vxImgLoad3DPatInst0[] = {
    { VIR_OP_VX_IMG_LOAD_3D, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 4, 5 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _vxImgLoad3DRepInst0[] = {
    { VIR_OP_VX_IMG_LOAD_3D, 0, 0, { 1, 2, 3, 4, 5 }, { 0, _SetTypeIdFromVariable } },
};

static VIR_Pattern _vxImgLoad3DPattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_vxImgLoad3D, 0) },
    { VIR_PATN_FLAG_NONE }
};


/*
img_sampler  1, 2, 3
img_load      1, 2, 5
    img_read 1, 2, 5, 0, 3   // they stand for high level image read function
*/
static VIR_PatternMatchInst _imgSamplerPatInst0[] = {
    { VIR_OP_IMG_SAMPLER, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_IMG_LOAD, VIR_PATTERN_ANYCOND, 0, { 1, 2, 5, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _imgSamplerRepInst0[] = {
    { VIR_OP_IMG_READ, 0, 0, { 1, 2, 5, 3 }, { 0, 0, 0 } },
};

static VIR_PatternMatchInst _imgSamplerPatInst1[] = {
    { VIR_OP_IMG_SAMPLER, VIR_PATTERN_ANYCOND, 0, { 4, 2, 3, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_IMG_LOAD, VIR_PATTERN_ANYCOND, 0, { 1, 2, 5, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _imgSamplerRepInst1[] = {
    { VIR_OP_IMG_READ, 0, 0, { 1, 2, 5, 3 }, { 0, 0, 0 } },
};

/*
img_sampler  1, 2, 3
img_load_3d      1, 2, 5
    img_read_3d 1, 2, 5, 0, 3
*/
static VIR_PatternMatchInst _imgSamplerPatInst2[] = {
    { VIR_OP_IMG_SAMPLER, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_IMG_LOAD_3D, VIR_PATTERN_ANYCOND, 0, { 1, 2, 5, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _imgSamplerRepInst2[] = {
    { VIR_OP_IMG_READ_3D, 0, 0, { 1, 2, 5, 3 }, { 0, 0, 0 } },
};

static VIR_PatternMatchInst _imgSamplerPatInst3[] = {
    { VIR_OP_IMG_SAMPLER, VIR_PATTERN_ANYCOND, 0, { 4, 2, 3, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_IMG_LOAD_3D, VIR_PATTERN_ANYCOND, 0, { 1, 2, 5, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _imgSamplerRepInst3[] = {
    { VIR_OP_IMG_READ_3D, 0, 0, { 1, 2, 5, 3 }, { 0, 0, 0 } },
};

static VIR_Pattern _imgSamplerPattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_imgSampler, 0) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_imgSampler, 1) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_imgSampler, 2) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_imgSampler, 3) },
    { VIR_PATN_FLAG_NONE }
};

static gctBOOL
_isSrc1ConstFit5Bits(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    VIR_Operand * src1 = VIR_Inst_GetSource(Inst, 1);
    return VIR_Operand_isValueFit5Bits(Context->shader, src1);
}

static gctBOOL
_noOffsetAndPrevInstUseAllComponents(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    VIR_Operand * src2 = VIR_Inst_GetSource(Inst, 2);
    VIR_Instruction * prevInst;
    if (src2 && !VIR_Operand_isUndef(src2))
    {
        return gcvFALSE;
    }
    prevInst = VIR_Inst_GetPrev(Inst);
    if (prevInst)
    {
        /* check inst components are the same as coordinate components */
        VIR_Enable    prevEnable = VIR_Inst_GetEnable(prevInst);
        VIR_Operand * src1 = VIR_Inst_GetSource(Inst, 1);
        VIR_Swizzle   src1Swizzle = VIR_Operand_GetSwizzle(src1);
        VIR_Enable    src1Enable = VIR_Swizzle_2_Enable(src1Swizzle);
        gcmASSERT(VIR_Inst_GetOpcode(prevInst) == VIR_OP_ADD && src1);
        return VIR_Enable_Covers(prevEnable, src1Enable);
   }
    return gcvTRUE;
}

static gctBOOL
_SetImmOffset(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    return VIR_IMG_LOAD_SetImmOffset(Context->shader, Inst, Opnd, gcvFALSE);
}

/* mul/add pattern can be changed to mad, but this pattern makes mad not happening
   mul t1, t2, t3
   add t4, t1, base
   store t0, t1, base, data (t1 is used here) */
static gctBOOL
_isPrevInstNotMul(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    VIR_Instruction *prevInst = VIR_Inst_GetPrev(Inst);

    if (prevInst != gcvNULL &&
        VIR_Inst_GetOpcode(prevInst) == VIR_OP_MUL)
    {
        if (VIR_Operand_SameLocation(prevInst, VIR_Inst_GetDest(prevInst), Inst, VIR_Inst_GetSource(Inst, 0)) ||
            VIR_Operand_SameLocation(prevInst, VIR_Inst_GetDest(prevInst), Inst, VIR_Inst_GetSource(Inst, 1)))
        {
            return gcvFALSE;
        }
    }

    return gcvTRUE;
}

static gctBOOL
_conv_enable_to_swizzle(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_Enable enable = VIR_Inst_GetEnable(Inst);
    VIR_Swizzle swizzle = VIR_SWIZZLE_XYZW;
    gctINT i, channel = 0;
    VIR_Swizzle components[4] = {VIR_SWIZZLE_X, VIR_SWIZZLE_X, VIR_SWIZZLE_X, VIR_SWIZZLE_X};

    for(i = 0; i < VIR_CHANNEL_COUNT; i++)
    {
        if(enable & (1 << i))
        {
            components[i] = VIR_Swizzle_GetChannel(swizzle, channel);
            channel++;
        }
    }
    swizzle = (components[0] << 0) | (components[1] << 2) | (components[2] << 4) | (components[3] << 6);
    VIR_Operand_SetSwizzle(Opnd, swizzle);
    return gcvTRUE;
}

static gctBOOL
_conv_1st_enable_to_swizzle(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_Enable enable = VIR_Inst_GetEnable(Inst);

    if(enable & VIR_ENABLE_X)
    {
        VIR_Operand_SetSwizzle(Opnd, VIR_SWIZZLE_XXXX);
        return gcvTRUE;
    }
    if(enable & VIR_ENABLE_Y)
    {
        VIR_Operand_SetSwizzle(Opnd, VIR_SWIZZLE_YYYY);
        return gcvTRUE;
    }
    if(enable & VIR_ENABLE_Z)
    {
        VIR_Operand_SetSwizzle(Opnd, VIR_SWIZZLE_ZZZZ);
        return gcvTRUE;
    }
    if(enable & VIR_ENABLE_W)
    {
        VIR_Operand_SetSwizzle(Opnd, VIR_SWIZZLE_WWWW);
        return gcvTRUE;
    }
    gcmASSERT(0);
    return gcvFALSE;
}

static gctBOOL
_conv_2nd_enable_to_swizzle(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    switch(VIR_Inst_GetEnable(Inst))
    {
    case VIR_ENABLE_XY:
    case VIR_ENABLE_XYZ:
    case VIR_ENABLE_XYW:
    case VIR_ENABLE_XYZW:
        VIR_Operand_SetSwizzle(Opnd, VIR_SWIZZLE_YYYY);
        break;

    case VIR_ENABLE_XZ:
    case VIR_ENABLE_XZW:
    case VIR_ENABLE_YZ:
    case VIR_ENABLE_YZW:
        VIR_Operand_SetSwizzle(Opnd, VIR_SWIZZLE_ZZZZ);
        break;

    case VIR_ENABLE_XW:
    case VIR_ENABLE_YW:
    case VIR_ENABLE_ZW:
        VIR_Operand_SetSwizzle(Opnd, VIR_SWIZZLE_WWWW);
        break;

    default:
       gcmASSERT(0);
       return gcvFALSE;
    }
    return gcvTRUE;
}

static gctBOOL
_conv_3rd_enable_to_swizzle(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    switch(VIR_Inst_GetEnable(Inst))
    {
    case VIR_ENABLE_XYZ:
    case VIR_ENABLE_XYZW:
        VIR_Operand_SetSwizzle(Opnd, VIR_SWIZZLE_ZZZZ);
        break;

    case VIR_ENABLE_XYW:
    case VIR_ENABLE_XZW :
    case VIR_ENABLE_YZW :
        VIR_Operand_SetSwizzle(Opnd, VIR_SWIZZLE_WWWW);
        break;

    default:
        gcmASSERT(0);
        return gcvFALSE;
    }

    return gcvTRUE;
}

static gctBOOL
_conv_4th_enable_to_swizzle(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    if(VIR_Inst_GetEnable(Inst) == VIR_ENABLE_XYZW)
    {
        VIR_Operand_SetSwizzle(Opnd, VIR_SWIZZLE_WWWW);
        return gcvTRUE;
    }
    gcmASSERT(0);
    return gcvFALSE;
}

static gctBOOL
_set_int_zero_enableXY(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_Operand * dest = VIR_Inst_GetDest(Inst);
    VIR_ScalarConstVal imm0;
    imm0.iValue = 0;

    VIR_Operand_SetImmediate(Opnd,
        VIR_TYPE_INT32,
        imm0);

    VIR_Operand_SetEnable(dest, VIR_ENABLE_XY);
    return gcvTRUE;
}

static gctBOOL
_set_int_one_enableX(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_Operand * dest = VIR_Inst_GetDest(Inst);
    VIR_ScalarConstVal imm0;
    imm0.iValue = 1;

    VIR_Operand_SetImmediate(Opnd,
        VIR_TYPE_INT32,
        imm0);

    VIR_Operand_SetEnable(dest, VIR_ENABLE_X);
    return gcvTRUE;
}

static gctBOOL
_set_int_one_enableXY(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_Operand * dest = VIR_Inst_GetDest(Inst);
    VIR_ScalarConstVal imm0;
    imm0.iValue = 1;

    VIR_Operand_SetImmediate(Opnd,
        VIR_TYPE_INT32,
        imm0);

    VIR_Operand_SetEnable(dest, VIR_ENABLE_XY);
    return gcvTRUE;
}

static gctBOOL
_set_1st_enable(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_Enable enable = VIR_Inst_GetEnable(Inst);

    if(enable & VIR_ENABLE_X)
    {
        VIR_Operand_SetEnable(Opnd, VIR_ENABLE_X);
        return gcvTRUE;
    }
    if(enable & VIR_ENABLE_Y)
    {
        VIR_Operand_SetEnable(Opnd, VIR_ENABLE_Y);
        return gcvTRUE;
    }
    if(enable & VIR_ENABLE_Z)
    {
        VIR_Operand_SetEnable(Opnd, VIR_ENABLE_Z);
        return gcvTRUE;
    }
    if(enable & VIR_ENABLE_W)
    {
        VIR_Operand_SetEnable(Opnd, VIR_ENABLE_W);
        return gcvTRUE;
    }
    gcmASSERT(0);
    return gcvFALSE;
}

static gctBOOL
_set_2nd_enable(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    switch(VIR_Inst_GetEnable(Inst))
    {
    case VIR_ENABLE_XY:
    case VIR_ENABLE_XYZ:
    case VIR_ENABLE_XYW:
    case VIR_ENABLE_XYZW:
        VIR_Operand_SetEnable(Opnd, VIR_ENABLE_Y);
        break;

    case VIR_ENABLE_XZ:
    case VIR_ENABLE_XZW:
    case VIR_ENABLE_YZ:
    case VIR_ENABLE_YZW:
        VIR_Operand_SetEnable(Opnd, VIR_ENABLE_Z);
        break;

    case VIR_ENABLE_XW:
    case VIR_ENABLE_YW:
    case VIR_ENABLE_ZW:
        VIR_Operand_SetEnable(Opnd, VIR_ENABLE_W);
        break;

    default:
        gcmASSERT(0);
        return gcvFALSE;
    }

    return gcvTRUE;
}

static gctBOOL
_set_3rd_enable(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    switch(VIR_Inst_GetEnable(Inst))
    {
    case VIR_ENABLE_XYZ:
    case VIR_ENABLE_XYZW:
        VIR_Operand_SetEnable(Opnd, VIR_ENABLE_Z);
        break;

    case VIR_ENABLE_XYW:
    case VIR_ENABLE_XZW:
    case VIR_ENABLE_YZW:
        VIR_Operand_SetEnable(Opnd, VIR_ENABLE_W);
        break;

    default:
        gcmASSERT(0);
        return gcvFALSE;
    }

    return gcvTRUE;
}

static gctBOOL
_set_4th_enable(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    if(VIR_Inst_GetEnable(Inst) == VIR_ENABLE_XYZW)
    {
        VIR_Operand_SetEnable(Opnd, VIR_ENABLE_W);
    }
    else
    {
        gcmASSERT(0);
        return gcvFALSE;
    }

    return gcvTRUE;
}

static gctBOOL
_set_1st_enable_swizzle(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_Enable enable = VIR_Inst_GetEnable(Inst);
    VIR_Swizzle swizzle = VIR_Operand_GetSwizzle(Opnd);

    if(enable & VIR_ENABLE_X)
    {
        swizzle = VIR_Swizzle_GetChannel(swizzle, 0);
        swizzle = (swizzle << 0) | (swizzle << 2) | (swizzle << 4) | (swizzle << 6);
        VIR_Operand_SetSwizzle(Opnd, swizzle);
        return gcvTRUE;
    }
    if(enable & VIR_ENABLE_Y)
    {
        swizzle = VIR_Swizzle_GetChannel(swizzle, 1);
        swizzle = (swizzle << 0) | (swizzle << 2) | (swizzle << 4) | (swizzle << 6);
        VIR_Operand_SetSwizzle(Opnd, swizzle);
        return gcvTRUE;
    }
    if(enable & VIR_ENABLE_Z)
    {
        swizzle = VIR_Swizzle_GetChannel(swizzle, 2);
        swizzle = (swizzle << 0) | (swizzle << 2) | (swizzle << 4) | (swizzle << 6);
        VIR_Operand_SetSwizzle(Opnd, swizzle);
        return gcvTRUE;
    }
    if(enable & VIR_ENABLE_W)
    {
        swizzle = VIR_Swizzle_GetChannel(swizzle, 3);
        swizzle = (swizzle << 0) | (swizzle << 2) | (swizzle << 4) | (swizzle << 6);
        VIR_Operand_SetSwizzle(Opnd, swizzle);
        return gcvTRUE;
    }
    gcmASSERT(0);
    return gcvFALSE;
}

static gctBOOL
_set_2nd_enable_swizzle(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_Swizzle swizzle = VIR_Operand_GetSwizzle(Opnd);

    switch(VIR_Inst_GetEnable(Inst))
    {
    case VIR_ENABLE_XY:
    case VIR_ENABLE_XYZ:
    case VIR_ENABLE_XYW:
    case VIR_ENABLE_XYZW:
        swizzle = VIR_Swizzle_GetChannel(swizzle, 1);
        swizzle = (swizzle << 0) | (swizzle << 2) | (swizzle << 4) | (swizzle << 6);
        VIR_Operand_SetSwizzle(Opnd, swizzle);
        break;

    case VIR_ENABLE_XZ:
    case VIR_ENABLE_XZW:
    case VIR_ENABLE_YZ:
    case VIR_ENABLE_YZW:
        swizzle = VIR_Swizzle_GetChannel(swizzle, 2);
        swizzle = (swizzle << 0) | (swizzle << 2) | (swizzle << 4) | (swizzle << 6);
        VIR_Operand_SetSwizzle(Opnd, swizzle);
        break;

    case VIR_ENABLE_XW:
    case VIR_ENABLE_YW:
    case VIR_ENABLE_ZW:
        swizzle = VIR_Swizzle_GetChannel(swizzle, 3);
        swizzle = (swizzle << 0) | (swizzle << 2) | (swizzle << 4) | (swizzle << 6);
        VIR_Operand_SetSwizzle(Opnd, swizzle);
        break;

    default:
       gcmASSERT(0);
       return gcvFALSE;
    }

    return gcvTRUE;
}

static gctBOOL
_set_3rd_enable_swizzle(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_Swizzle swizzle = VIR_Operand_GetSwizzle(Opnd);

    switch(VIR_Inst_GetEnable(Inst))
    {
    case VIR_ENABLE_XYZ:
    case VIR_ENABLE_XYZW:
        swizzle = VIR_Swizzle_GetChannel(swizzle, 2);
        swizzle = (swizzle << 0) | (swizzle << 2) | (swizzle << 4) | (swizzle << 6);
        VIR_Operand_SetSwizzle(Opnd, swizzle);
        break;

    case VIR_ENABLE_XYW:
    case VIR_ENABLE_XZW :
    case VIR_ENABLE_YZW :
        swizzle = VIR_Swizzle_GetChannel(swizzle, 3);
        swizzle = (swizzle << 0) | (swizzle << 2) | (swizzle << 4) | (swizzle << 6);
        VIR_Operand_SetSwizzle(Opnd, swizzle);
        break;

    default:
       gcmASSERT(0);
       return gcvFALSE;
    }

    return gcvTRUE;
}

static gctBOOL
_set_4th_enable_swizzle(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_Swizzle swizzle = VIR_Operand_GetSwizzle(Opnd);

    if(VIR_Inst_GetEnable(Inst) == VIR_ENABLE_XYZW)
    {
        swizzle = VIR_Swizzle_GetChannel(swizzle, 3);
        swizzle = (swizzle << 0) | (swizzle << 2) | (swizzle << 4) | (swizzle << 6);
        VIR_Operand_SetSwizzle(Opnd, swizzle);
    }
    else
    {
       gcmASSERT(0);
       return gcvFALSE;
    }

    return gcvTRUE;
}

/*
add      1, 2, 3
store1   4, 1, 5
    store 4, 2, 3, 5
*/
static VIR_PatternMatchInst _addPatInst0[] = {
    { VIR_OP_ADD, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { _isPrevInstNotMul }, VIR_PATN_MATCH_FLAG_OR },
    { MERGE_OPCODE(VIR_OP_STARR, EXTERN_STORE1), VIR_PATTERN_ANYCOND, 0, { 4, 1, 5, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _addRepInst0[] = {
    { VIR_OP_ADD, 0, 0, { 1, 2, 3, 0 }, { 0 } }, /* keep the add in case it is used by other inst */
    { VIR_OP_STORE, -1, 0, { 4, 2, 3, 5 }, { 0, _normalize_src2_swiz, 0, _fix_src_type_to_dest_type} },
};

/*
add      1, 2, 3
store_l   4, 1, 5
    store 4, 2, 3, 5
*/
static VIR_PatternMatchInst _addPatInst1[] = {
    { VIR_OP_ADD, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
    { MERGE_OPCODE(VIR_OP_STARR, EXTERN_STORE_L), VIR_PATTERN_ANYCOND, 0, { 4, 1, 5, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _addRepInst1[] = {
    { VIR_OP_ADD, 0, 0, { 1, 2, 3, 0 }, { 0 } }, /* keep the add in case it is used by other inst */
    { VIR_OP_STORE_L, -1, 0, { 4, 2, 3, 5 }, { 0, _normalize_src2_swiz, 0, _fix_src_type_to_dest_type} },
};

/*
add      1, 2, 3
img_load   4, 5, 1, 6
    img_load 4, 5, 2, 3
*/
static VIR_PatternMatchInst _addPatInst2[] = {
    { VIR_OP_ADD, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { _isSrc1ConstFit5Bits }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_IMG_LOAD, VIR_PATTERN_ANYCOND, 0, { 4, 5, 1, 6 }, { _noOffsetAndPrevInstUseAllComponents }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _addRepInst2[] = {
    { VIR_OP_ADD, 0, 0, { 1, 2, 3, 0 }, { 0 } }, /* keep the add in case it is used by other inst */
    { VIR_OP_IMG_LOAD, 0, 0, { 4, 5, 2, 3 }, { 0, 0, 0, _SetImmOffset } }, /* keep the add in case it is used by other inst */
};

/*
add      1, 2, 3
img_load_3d   4, 5, 1, 6
    img_load_3d 4, 5, 2, 3
*/
static VIR_PatternMatchInst _addPatInst3[] = {
    { VIR_OP_ADD, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { _isSrc1ConstFit5Bits }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_IMG_LOAD_3D, VIR_PATTERN_ANYCOND, 0, { 4, 5, 1, 6 }, { _noOffsetAndPrevInstUseAllComponents }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _addRepInst3[] = {
    { VIR_OP_ADD, 0, 0, { 1, 2, 3, 0 }, { 0 } }, /* keep the add in case it is used by other inst */
    { VIR_OP_IMG_LOAD_3D, 0, 0, { 4, 5, 2, 3 }, { 0, 0, 0, _SetImmOffset } },
};

/*
add      1, 2, 3
vx_img_load   4, 5, 1, 6, 7
    vx_img_load 4, 5, 2, 3
*/
static VIR_PatternMatchInst _addPatInst4[] = {
    { VIR_OP_ADD, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { _isSrc1ConstFit5Bits }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_VX_IMG_LOAD, VIR_PATTERN_ANYCOND, 0, { 4, 5, 1, 6, 7 }, { _noOffsetAndPrevInstUseAllComponents }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _addRepInst4[] = {
    { VIR_OP_ADD, 0, 0, { 1, 2, 3, 0 }, { 0 } }, /* keep the add in case it is used by other inst */
    { VIR_OP_VX_IMG_LOAD, 0, 0, { 4, 5, 2, 3, 7 }, {0, 0, 0, _SetImmOffset } },
};

/*
add      1, 2, 3
vx_img_load_3d   4, 5, 1, 6, 7
    vx_img_load_3d 4, 5, 2, 3
*/
static VIR_PatternMatchInst _addPatInst5[] = {
    { VIR_OP_ADD, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { _isSrc1ConstFit5Bits }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_VX_IMG_LOAD_3D, VIR_PATTERN_ANYCOND, 0, { 4, 5, 1, 6, 7 }, { _noOffsetAndPrevInstUseAllComponents }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _addRepInst5[] = {
    { VIR_OP_ADD, 0, 0, { 1, 2, 3, 0 }, { 0 } }, /* keep the add in case it is used by other inst */
    { VIR_OP_VX_IMG_LOAD_3D, 0, 0, { 4, 5, 2, 3, 7 }, { 0, 0, 0, _SetImmOffset } },
};

static VIR_Pattern _addPattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_add, 0) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_add, 1) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_add, 2) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_add, 3) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_add, 4) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_add, 5) },
    { VIR_PATN_FLAG_NONE }
};

/*
load    1, 2, 3
atomadd 4, 1, 5
    atomadd 4, 2, 3, 5
*/
static VIR_PatternMatchInst _loadPatInst0[] = {
    { VIR_OP_LOAD, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_ATOMADD, VIR_PATTERN_ANYCOND, 0, { 4, 1, 5, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _loadRepInst0[] = {
    { VIR_OP_ATOMADD, -2, 0, { 4, 2, 3, 5 }, { 0 } },
};

static VIR_PatternMatchInst _loadPatInst1[] = {
    { VIR_OP_LOAD, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_ATOMSUB, VIR_PATTERN_ANYCOND, 0, { 4, 1, 5, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _loadRepInst1[] = {
    { VIR_OP_ATOMSUB, -2, 0, { 4, 2, 3, 5 }, { 0 } },
};

static VIR_PatternMatchInst _loadPatInst2[] = {
    { VIR_OP_LOAD, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_ATOMXCHG, VIR_PATTERN_ANYCOND, 0, { 4, 1, 5, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _loadRepInst2[] = {
    { VIR_OP_ATOMXCHG, -2, 0, { 4, 2, 3, 5 }, { 0 } },
};

static VIR_PatternMatchInst _loadPatInst3[] = {
    { VIR_OP_LOAD, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_ATOMCMPXCHG, VIR_PATTERN_ANYCOND, 0, { 4, 1, 5, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _loadRepInst3[] = {
    { VIR_OP_ATOMCMPXCHG, -2, 0, { 4, 2, 3, 5 }, { 0 } },
};

static VIR_PatternMatchInst _loadPatInst4[] = {
    { VIR_OP_LOAD, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_ATOMMIN, VIR_PATTERN_ANYCOND, 0, { 4, 1, 5, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _loadRepInst4[] = {
    { VIR_OP_ATOMMIN, -2, 0, { 4, 2, 3, 5 }, { 0 } },
};

static VIR_PatternMatchInst _loadPatInst5[] = {
    { VIR_OP_LOAD, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_ATOMMAX, VIR_PATTERN_ANYCOND, 0, { 4, 1, 5, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _loadRepInst5[] = {
    { VIR_OP_ATOMMAX, -2, 0, { 4, 2, 3, 5 }, { 0 } },
};

static VIR_PatternMatchInst _loadPatInst6[] = {
    { VIR_OP_LOAD, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_ATOMOR, VIR_PATTERN_ANYCOND, 0, { 4, 1, 5, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _loadRepInst6[] = {
    { VIR_OP_ATOMOR, -2, 0, { 4, 2, 3, 5 }, { 0 } },
};

static VIR_PatternMatchInst _loadPatInst7[] = {
    { VIR_OP_LOAD, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_ATOMAND, VIR_PATTERN_ANYCOND, 0, { 4, 1, 5, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _loadRepInst7[] = {
    { VIR_OP_ATOMAND, -2, 0, { 4, 2, 3, 5 }, { 0 } },
};

static VIR_PatternMatchInst _loadPatInst8[] = {
    { VIR_OP_LOAD, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_ATOMXOR, VIR_PATTERN_ANYCOND, 0, { 4, 1, 5, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _loadRepInst8[] = {
    { VIR_OP_ATOMXOR, -2, 0, { 4, 2, 3, 5 }, { 0 } },
};

static VIR_Pattern _loadPattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_load, 0) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_load, 1) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_load, 2) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_load, 3) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_load, 4) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_load, 5) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_load, 6) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_load, 7) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_load, 8) },
    { VIR_PATN_FLAG_NONE }
};

static VIR_PatternMatchInst _atomADDPatInst0[] = {
    { VIR_OP_ATOMADD, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _atomADDRepInst0[] = {
    { VIR_OP_ATOMADD, -1, 0, { 1, 2, 3, 0 }, { 0 } },
};

static VIR_Pattern _atomADDPattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_atomADD, 0) },
    { VIR_PATN_FLAG_NONE }
};

static VIR_PatternMatchInst _atomSUBPatInst0[] = {
    { VIR_OP_ATOMSUB, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _atomSUBRepInst0[] = {
    { VIR_OP_ATOMSUB, -1, 0, { 1, 2, 3, 0 }, { 0 } },
};

static VIR_Pattern _atomSUBPattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_atomSUB, 0) },
    { VIR_PATN_FLAG_NONE }
};

static VIR_PatternMatchInst _atomXCHGPatInst0[] = {
    { VIR_OP_ATOMXCHG, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _atomXCHGRepInst0[] = {
    { VIR_OP_ATOMXCHG, -1, 0, { 1, 2, 3, 0 }, { 0 } },
};

static VIR_Pattern _atomXCHGPattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_atomXCHG, 0) },
    { VIR_PATN_FLAG_NONE }
};

static VIR_PatternMatchInst _atomCMPXCHGPatInst0[] = {
    { VIR_OP_ATOMCMPXCHG, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _atomCMPXCHGRepInst0[] = {
    { VIR_OP_ATOMCMPXCHG, -1, 0, { 1, 2, 3, 0 }, { 0 } },
};

static VIR_Pattern _atomCMPXCHGPattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_atomCMPXCHG, 0) },
    { VIR_PATN_FLAG_NONE }
};

static VIR_PatternMatchInst _atomMINPatInst0[] = {
    { VIR_OP_ATOMMIN, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _atomMINRepInst0[] = {
    { VIR_OP_ATOMMIN, -1, 0, { 1, 2, 3, 0 }, { 0 } },
};

static VIR_Pattern _atomMINPattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_atomMIN, 0) },
    { VIR_PATN_FLAG_NONE }
};

static VIR_PatternMatchInst _atomMAXPatInst0[] = {
    { VIR_OP_ATOMMAX, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _atomMAXRepInst0[] = {
    { VIR_OP_ATOMMAX, -1, 0, { 1, 2, 3, 0 }, { 0 } },
};

static VIR_Pattern _atomMAXPattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_atomMAX, 0) },
    { VIR_PATN_FLAG_NONE }
};

static VIR_PatternMatchInst _atomORPatInst0[] = {
    { VIR_OP_ATOMOR, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _atomORRepInst0[] = {
    { VIR_OP_ATOMOR, -1, 0, { 1, 2, 3, 0 }, { 0 } },
};

static VIR_Pattern _atomORPattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_atomOR, 0) },
    { VIR_PATN_FLAG_NONE }
};

static VIR_PatternMatchInst _atomANDPatInst0[] = {
    { VIR_OP_ATOMAND, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _atomANDRepInst0[] = {
    { VIR_OP_ATOMAND, -1, 0, { 1, 2, 3, 0 }, { 0 } },
};

static VIR_Pattern _atomANDPattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_atomAND, 0) },
    { VIR_PATN_FLAG_NONE }
};

static VIR_PatternMatchInst _atomXORPatInst0[] = {
    { VIR_OP_ATOMXOR, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _atomXORRepInst0[] = {
    { VIR_OP_ATOMXOR, -1, 0, { 1, 2, 3, 0 }, { 0 } },
};

static VIR_Pattern _atomXORPattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_atomXOR, 0) },
    { VIR_PATN_FLAG_NONE }
};


static VIR_PatternMatchInst _bitrangePatInst0[] = {
    { VIR_OP_BITRANGE, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_LOAD, VIR_PATTERN_ANYCOND, 0, { 5, 6, 7, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_BITEXTRACT, VIR_PATTERN_ANYCOND, 0, { 1, 4, 0, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _bitrangeRepInst0[] = {
    { VIR_OP_LOAD, VIR_PATTERN_ANYCOND, 0, { 5, 6, 7, 0 }, { 0 } },
    { VIR_OP_BITEXTRACT, VIR_PATTERN_ANYCOND, 0, { 1, 4, 2, 3 }, { 0 } },
};

static VIR_PatternMatchInst _bitrangePatInst1[] = {
    { VIR_OP_BITRANGE, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_BITEXTRACT, VIR_PATTERN_ANYCOND, 0, { 1, 4, 0, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _bitrangeRepInst1[] = {
    { VIR_OP_BITEXTRACT, VIR_PATTERN_ANYCOND, 0, { 1, 4, 2, 3 }, { 0 } },
};

static VIR_PatternMatchInst _bitrangePatInst2[] = {
    { VIR_OP_BITRANGE, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_LOAD, VIR_PATTERN_ANYCOND, 0, { 6, 7, 8, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_LOAD, VIR_PATTERN_ANYCOND, 0, { 9, 10, 11, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_BITINSERT, VIR_PATTERN_ANYCOND, 0, { 1, 4, 5, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _bitrangeRepInst2[] = {
    { VIR_OP_LOAD, VIR_PATTERN_ANYCOND, 0, { 6, 7, 8, 0 }, { 0 } },
    { VIR_OP_LOAD, VIR_PATTERN_ANYCOND, 0, { 9, 10, 11, 0 }, { 0 } },
    { VIR_OP_BITINSERT, VIR_PATTERN_ANYCOND, 0, { 1, 4, 5, 2, 3 }, { 0 } },
};

static VIR_PatternMatchInst _bitrangePatInst3[] = {
    { VIR_OP_BITRANGE, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_BITINSERT, VIR_PATTERN_ANYCOND, 0, { 1, 4, 5, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _bitrangeRepInst3[] = {
    { VIR_OP_BITINSERT, VIR_PATTERN_ANYCOND, 0, { 1, 4, 5, 2, 3 }, { 0 } },
};

static VIR_Pattern _bitrangePattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_bitrange, 0) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_bitrange, 1) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_bitrange, 2) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_bitrange, 3) },
    { VIR_PATN_FLAG_NONE }
};

static VIR_PatternMatchInst _bitrange1PatInst0[] = {
    { VIR_OP_BITRANGE1, VIR_PATTERN_ANYCOND, 0, { 1, 2, 0, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_BITINSERT, VIR_PATTERN_ANYCOND, 0, { 1, 4, 5, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _bitrange1RepInst0[] = {
    { VIR_OP_BITINSERT2, VIR_PATTERN_ANYCOND, 0, { 1, 4, 5, 2 }, { 0 } },
};

static VIR_Pattern _bitrange1Pattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_bitrange1, 0) },
    { VIR_PATN_FLAG_NONE }
};

/*
fma_mul 1, 2, 3
fma_add 4, 1, 5
    fma 4, 2, 3, 5
*/

static VIR_PatternMatchInst _mulPatInst0[] = {
    { MERGE_OPCODE(VIR_OP_MUL, EXTERN_FMA_MUL), VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_ADD, VIR_PATTERN_ANYCOND, 0, { 4, 1, 5, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _mulRepInst0[] = {
    { VIR_OP_FMA, -2, 0, { 4, 2, 3, 5 }, { 0 } },
};

static VIR_Pattern _mulPattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_mul, 0) },
    { VIR_PATN_FLAG_NONE }
};

/*
arctrig0 1, 2, 3
arctrig1 4, 1, 5
    arctrig 4, 2, 3, 5
*/

static VIR_PatternMatchInst _arctrigPatInst0[] = {
    { MERGE_OPCODE(VIR_OP_ARCTRIG, EXTERN_ARCTRIG0), VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
    { MERGE_OPCODE(VIR_OP_ARCTRIG, EXTERN_ARCTRIG1), VIR_PATTERN_ANYCOND, 0, { 4, 1, 5, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _arctrigRepInst0[] = {
    { VIR_OP_ARCTRIG, -2, 0, { 4, 2, 3, 5 }, { 0 } },
};

/*
arctrig0 1, 2, 3
mov      4, 1
arctrig1 6, 4, 5
    arctrig 6, 2, 3, 5
*/

static VIR_PatternMatchInst _arctrigPatInst1[] = {
    { MERGE_OPCODE(VIR_OP_ARCTRIG, EXTERN_ARCTRIG0), VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_MOV, VIR_PATTERN_ANYCOND, 0, { 4, 1, 0, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
    { MERGE_OPCODE(VIR_OP_ARCTRIG, EXTERN_ARCTRIG1), VIR_PATTERN_ANYCOND, 0, { 6, 4, 5, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _arctrigRepInst1[] = {
    { VIR_OP_ARCTRIG, -2, 0, { 6, 2, 3, 5 }, { 0 } },
};

static VIR_Pattern _arctrigPattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_arctrig, 0) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_arctrig, 1) },
    { VIR_PATN_FLAG_NONE }
};

/*
asin 1, 2
    NEG         t1, 2
    FMA         t2, 2, t1, imm(1.0)
    SQRT        t3, t2
    ARCTRIG     t4.xy, 2, t3, imm(1)
    MUL         1, 2, t4.x
*/

static VIR_PatternMatchInst _asinPatInst0[] = {
    { VIR_OP_ASIN, VIR_PATTERN_ANYCOND, 0, { 1, 2 }, { VIR_Lower_IsDstOneEnable }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _asinRepInst0[] = {
    { VIR_OP_NEG, 0, 0, { -1, 2, 0, 0 }, { 0 } },
    { VIR_OP_FMA, 0, 0, { -2, 2, -1, 0 }, { VIR_Lower_SetPrecisionBaseOnSrc0, 0, 0, VIR_Lower_SetOne } },
    { VIR_OP_SQRT, 0, 0, { -3, -2, 0, 0 }, { 0 } },
    { VIR_OP_ARCTRIG, 0, 0, { -4, -3, -3, 0 }, { VIR_Lower_SetPrecisionBaseOnSrc0, 0, 0, _set_int_one_enableXY } },
    { VIR_OP_MUL, 0, 0, { 1, 2, -4, 0 }, { 0, 0, VIR_Lower_SetSwizzleX } },
};

static VIR_PatternMatchInst _asinPatInst1[] = {
    { VIR_OP_ASIN, VIR_PATTERN_ANYCOND, 0, { 1, 2 }, { VIR_Lower_IsDstTwoEnables }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _asinRepInst1[] = {
    { VIR_OP_NEG, 0, 0, { -1, 2, 0, 0 }, { 0 } },
    { VIR_OP_FMA, 0, 0, { -2, 2, -1, 0 }, { VIR_Lower_SetPrecisionBaseOnSrc0, 0, 0, VIR_Lower_SetOne } },
    { VIR_OP_SQRT, 0, 0, { -3, -2, 0, 0 }, { 0 } },
    { VIR_OP_ARCTRIG, 0, 0, { -4, -3, -3, 0 }, { VIR_Lower_SetPrecisionBaseOnSrc0, _set_1st_enable_swizzle, _conv_1st_enable_to_swizzle, _set_int_one_enableX } },
    { VIR_OP_ARCTRIG, 0, 0, { -5, -3, -3, 0 }, { VIR_Lower_SetPrecisionBaseOnSrc0, _set_2nd_enable_swizzle, _conv_2nd_enable_to_swizzle, _set_int_one_enableXY } },
    { VIR_OP_MOV, 0, 0, { -4, -5, 0, 0 }, { VIR_Lower_SetEnableY, VIR_Lower_SetSwizzleX } },
    { VIR_OP_MUL, 0, 0, { 1, 2, -4, 0 }, { 0, 0, _conv_enable_to_swizzle } },
};

static VIR_PatternMatchInst _asinPatInst2[] = {
    { VIR_OP_ASIN, VIR_PATTERN_ANYCOND, 0, { 1, 2 }, { VIR_Lower_IsDstThreeEnables }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _asinRepInst2[] = {
    { VIR_OP_NEG, 0, 0, { -1, 2, 0, 0 }, { 0 } },
    { VIR_OP_FMA, 0, 0, { -2, 2, -1, 0 }, { VIR_Lower_SetPrecisionBaseOnSrc0, 0, 0, VIR_Lower_SetOne } },
    { VIR_OP_SQRT, 0, 0, { -3, -2, 0, 0 }, { 0 } },
    { VIR_OP_ARCTRIG, 0, 0, { -4, -3, -3, 0 }, { VIR_Lower_SetPrecisionBaseOnSrc0, _set_1st_enable_swizzle, _conv_1st_enable_to_swizzle, _set_int_one_enableX } },
    { VIR_OP_ARCTRIG, 0, 0, { -5, -3, -3, 0 }, { VIR_Lower_SetPrecisionBaseOnSrc0, _set_2nd_enable_swizzle, _conv_2nd_enable_to_swizzle, _set_int_one_enableXY } },
    { VIR_OP_ARCTRIG, 0, 0, { -6, -3, -3, 0 }, { VIR_Lower_SetPrecisionBaseOnSrc0, _set_3rd_enable_swizzle, _conv_3rd_enable_to_swizzle, _set_int_one_enableXY } },
    { VIR_OP_MOV, 0, 0, { -4, -5, 0, 0 }, { VIR_Lower_SetEnableY, VIR_Lower_SetSwizzleX } },
    { VIR_OP_MOV, 0, 0, { -4, -6, 0, 0 }, { VIR_Lower_SetEnableZ, VIR_Lower_SetSwizzleX } },
    { VIR_OP_MUL, 0, 0, { 1, 2, -4, 0 }, { 0, 0, _conv_enable_to_swizzle } },
};

static VIR_PatternMatchInst _asinPatInst3[] = {
    { VIR_OP_ASIN, VIR_PATTERN_ANYCOND, 0, { 1, 2 }, { VIR_Lower_IsDstFourEnables }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _asinRepInst3[] = {
    { VIR_OP_NEG, 0, 0, { -1, 2, 0, 0 }, { 0 } },
    { VIR_OP_FMA, 0, 0, { -2, 2, -1, 0 }, { VIR_Lower_SetPrecisionBaseOnSrc0, 0, 0, VIR_Lower_SetOne } },
    { VIR_OP_SQRT, 0, 0, { -3, -2, 0, 0 }, { 0 } },
    { VIR_OP_ARCTRIG, 0, 0, { -4, -3, -3, 0 }, { VIR_Lower_SetPrecisionBaseOnSrc0, _set_1st_enable_swizzle, _conv_1st_enable_to_swizzle, _set_int_zero_enableXY } },
    { VIR_OP_ARCTRIG, 0, 0, { -5, -3, -3, 0 }, { VIR_Lower_SetPrecisionBaseOnSrc0, _set_2nd_enable_swizzle, _conv_2nd_enable_to_swizzle, _set_int_zero_enableXY } },
    { VIR_OP_ARCTRIG, 0, 0, { -6, -3, -3, 0 }, { VIR_Lower_SetPrecisionBaseOnSrc0, _set_3rd_enable_swizzle, _conv_3rd_enable_to_swizzle, _set_int_zero_enableXY } },
    { VIR_OP_ARCTRIG, 0, 0, { -7, -3, -3, 0 }, { VIR_Lower_SetPrecisionBaseOnSrc0, _set_4th_enable_swizzle, _conv_4th_enable_to_swizzle, _set_int_zero_enableXY } },
    { VIR_OP_MOV, 0, 0, { -4, -5, 0, 0 }, { VIR_Lower_SetEnableY, VIR_Lower_SetSwizzleX } },
    { VIR_OP_MOV, 0, 0, { -4, -6, 0, 0 }, { VIR_Lower_SetEnableZ, VIR_Lower_SetSwizzleX } },
    { VIR_OP_MOV, 0, 0, { -4, -7, 0, 0 }, { VIR_Lower_SetEnableW, VIR_Lower_SetSwizzleX } },
    { VIR_OP_MUL, 0, 0, { 1, 2, -4, 0 }, { 0, 0, _conv_enable_to_swizzle } },
};

static VIR_Pattern _asinPattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_asin, 0) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_asin, 1) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_asin, 2) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_asin, 3) },
    { VIR_PATN_FLAG_NONE }
};

/*
acos 1, 2
    NEG         t1, 2
    FMA         t2, 2, t1, imm(1.0)
    SQRT        t3, t2
    ARCTRIG     t4.xy, 2, t3, imm(0)
    FMA         1, t3, t4.x, t4.y
*/

static VIR_PatternMatchInst _acosPatInst0[] = {
    { VIR_OP_ACOS, VIR_PATTERN_ANYCOND, 0, { 1, 2 }, { VIR_Lower_IsDstOneEnable }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _acosRepInst0[] = {
    { VIR_OP_NEG, 0, 0, { -1, 2, 0, 0 }, { 0 } },
    { VIR_OP_FMA, 0, 0, { -2, 2, -1, 0 }, { VIR_Lower_SetPrecisionBaseOnSrc0, 0, 0, VIR_Lower_SetOne } },
    { VIR_OP_SQRT, 0, 0, { -3, -2, 0, 0 }, { 0 } },
    { VIR_OP_ARCTRIG, 0, 0, { -4, 2, -3, 0 }, { VIR_Lower_SetHighp, 0, 0, _set_int_zero_enableXY } },
    { VIR_OP_FMA, 0, 0, { 1, -3, -4, -4 }, { 0, 0, VIR_Lower_SetSwizzleX, VIR_Lower_SetSwizzleY } },
};

static VIR_PatternMatchInst _acosPatInst1[] = {
    { VIR_OP_ACOS, VIR_PATTERN_ANYCOND, 0, { 1, 2 }, { VIR_Lower_IsDstTwoEnables }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _acosRepInst1[] = {
    { VIR_OP_NEG, 0, 0, { -1, 2, 0, 0 }, { 0 } },
    { VIR_OP_FMA, 0, 0, { -2, 2, -1, 0 }, { VIR_Lower_SetPrecisionBaseOnSrc0, 0, 0, VIR_Lower_SetOne } },
    { VIR_OP_SQRT, 0, 0, { -3, -2, 0, 0 }, { 0 } },
    { VIR_OP_ARCTRIG, 0, 0, { -4, 2, -3, 0 }, { VIR_Lower_SetHighp, _set_1st_enable_swizzle, _conv_1st_enable_to_swizzle, _set_int_zero_enableXY } },
    { VIR_OP_ARCTRIG, 0, 0, { -5, 2, -3, 0 }, { VIR_Lower_SetHighp, _set_2nd_enable_swizzle, _conv_2nd_enable_to_swizzle, _set_int_zero_enableXY } },
    { VIR_OP_FMA, 0, 0, { 1, -3, -4, -4 }, { _set_1st_enable, 0, VIR_Lower_SetSwizzleX, VIR_Lower_SetSwizzleY } },
    { VIR_OP_FMA, 0, 0, { 1, -3, -5, -5 }, { _set_2nd_enable, 0, VIR_Lower_SetSwizzleX, VIR_Lower_SetSwizzleY } },
};

static VIR_PatternMatchInst _acosPatInst2[] = {
    { VIR_OP_ACOS, VIR_PATTERN_ANYCOND, 0, { 1, 2 }, { VIR_Lower_IsDstThreeEnables }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _acosRepInst2[] = {
    { VIR_OP_NEG, 0, 0, { -1, 2, 0, 0 }, { 0 } },
    { VIR_OP_FMA, 0, 0, { -2, 2, -1, 0 }, { VIR_Lower_SetPrecisionBaseOnSrc0, 0, 0, VIR_Lower_SetOne } },
    { VIR_OP_SQRT, 0, 0, { -3, -2, 0, 0 }, { 0 } },
    { VIR_OP_ARCTRIG, 0, 0, { -4, 2, -3, 0 }, { VIR_Lower_SetHighp, _set_1st_enable_swizzle, _conv_1st_enable_to_swizzle, _set_int_zero_enableXY } },
    { VIR_OP_ARCTRIG, 0, 0, { -5, 2, -3, 0 }, { VIR_Lower_SetHighp, _set_2nd_enable_swizzle, _conv_2nd_enable_to_swizzle, _set_int_zero_enableXY } },
    { VIR_OP_ARCTRIG, 0, 0, { -6, 2, -3, 0 }, { VIR_Lower_SetHighp, _set_3rd_enable_swizzle, _conv_3rd_enable_to_swizzle, _set_int_zero_enableXY } },
    { VIR_OP_FMA, 0, 0, { 1, -3, -4, -4 }, { _set_1st_enable, 0, VIR_Lower_SetSwizzleX, VIR_Lower_SetSwizzleY } },
    { VIR_OP_FMA, 0, 0, { 1, -3, -5, -5 }, { _set_2nd_enable, 0, VIR_Lower_SetSwizzleX, VIR_Lower_SetSwizzleY } },
    { VIR_OP_FMA, 0, 0, { 1, -3, -6, -6 }, { _set_3rd_enable, 0, VIR_Lower_SetSwizzleX, VIR_Lower_SetSwizzleY } },
};

static VIR_PatternMatchInst _acosPatInst3[] = {
    { VIR_OP_ACOS, VIR_PATTERN_ANYCOND, 0, { 1, 2 }, { VIR_Lower_IsDstFourEnables }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _acosRepInst3[] = {
    { VIR_OP_NEG, 0, 0, { -1, 2, 0, 0 }, { 0 } },
    { VIR_OP_FMA, 0, 0, { -2, 2, -1, 0 }, { VIR_Lower_SetPrecisionBaseOnSrc0, 0, 0, VIR_Lower_SetOne } },
    { VIR_OP_SQRT, 0, 0, { -3, -2, 0, 0 }, { 0 } },
    { VIR_OP_ARCTRIG, 0, 0, { -4, 2, -3, 0 }, { VIR_Lower_SetHighp, _set_1st_enable_swizzle, _conv_1st_enable_to_swizzle, _set_int_zero_enableXY } },
    { VIR_OP_ARCTRIG, 0, 0, { -5, 2, -3, 0 }, { VIR_Lower_SetHighp, _set_2nd_enable_swizzle, _conv_2nd_enable_to_swizzle, _set_int_zero_enableXY } },
    { VIR_OP_ARCTRIG, 0, 0, { -6, 2, -3, 0 }, { VIR_Lower_SetHighp, _set_3rd_enable_swizzle, _conv_3rd_enable_to_swizzle, _set_int_zero_enableXY } },
    { VIR_OP_ARCTRIG, 0, 0, { -7, 2, -3, 0 }, { VIR_Lower_SetHighp, _set_4th_enable_swizzle, _conv_4th_enable_to_swizzle, _set_int_zero_enableXY } },
    { VIR_OP_FMA, 0, 0, { 1, -3, -4, -4 }, { _set_1st_enable, 0, VIR_Lower_SetSwizzleX, VIR_Lower_SetSwizzleY } },
    { VIR_OP_FMA, 0, 0, { 1, -3, -5, -5 }, { _set_2nd_enable, 0, VIR_Lower_SetSwizzleX, VIR_Lower_SetSwizzleY } },
    { VIR_OP_FMA, 0, 0, { 1, -3, -6, -6 }, { _set_3rd_enable, 0, VIR_Lower_SetSwizzleX, VIR_Lower_SetSwizzleY } },
    { VIR_OP_FMA, 0, 0, { 1, -3, -7, -7 }, { _set_4th_enable, 0, VIR_Lower_SetSwizzleX, VIR_Lower_SetSwizzleY } },
};

static VIR_Pattern _acosPattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_acos, 0) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_acos, 1) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_acos, 2) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_acos, 3) },
    { VIR_PATN_FLAG_NONE }
};


static gctBOOL
_copy16ByteOrLess(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    VIR_Operand * src1 = VIR_Inst_GetSource(Inst, 1);
    gctUINT     copySize;
    gcmASSERT(VIR_Operand_GetOpKind(src1) == VIR_OPND_IMMEDIATE);

    copySize = VIR_Operand_GetImmediateUint(src1);

    return copySize <= 16;
}

static gctBOOL
_copy32ByteOrLess(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    VIR_Operand * src1 = VIR_Inst_GetSource(Inst, 1);
    gctUINT     copySize;
    gcmASSERT(VIR_Operand_GetOpKind(src1) == VIR_OPND_IMMEDIATE);

    copySize = VIR_Operand_GetImmediateUint(src1);

    return copySize > 16 && copySize <= 32;
}

static gctBOOL
_copy64ByteOrLess(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    VIR_Operand * src1 = VIR_Inst_GetSource(Inst, 1);
    gctUINT     copySize;
    gcmASSERT(VIR_Operand_GetOpKind(src1) == VIR_OPND_IMMEDIATE);

    copySize = VIR_Operand_GetImmediateUint(src1);

    return copySize >32 && copySize <= 64;
}

VIR_TypeId
_getArrayElemTypeId(
    IN VIR_PatternContext * Context,
    IN VIR_TypeId           TypeId
    )
{
    VIR_Type * ty  =  VIR_Shader_GetTypeFromId(Context->shader, TypeId);
    while (VIR_Type_isArray(ty))
    {
        ty = VIR_Shader_GetTypeFromId(Context->shader, VIR_Type_GetBaseTypeId(ty));
    }
    gcmASSERT(ty != gcvNULL);

    return VIR_Type_GetIndex(ty);
}

static VIR_TypeId
_getOpndRowTypeId(
    IN VIR_PatternContext *Context,
    IN VIR_Operand        *Opnd
    )
{
    VIR_Symbol *  sym;
    VIR_TypeId    tyId;

    gcmASSERT(VIR_Operand_isVirReg(Opnd) || VIR_Operand_isSymbol(Opnd));

    if (VIR_Operand_isVirReg(Opnd) || VIR_Operand_isSymbol(Opnd))
    {
        sym = VIR_Operand_GetSymbol(Opnd);
        if (VIR_Symbol_GetKind(sym) == VIR_SYM_VIRREG)
        {
            sym = VIR_Symbol_GetVregVariable(sym); /* set the sym to corresponding variable */
        }
        tyId = _getArrayElemTypeId(Context, VIR_Symbol_GetTypeId(sym));
    }
    else
    {
        tyId = _getArrayElemTypeId(Context, VIR_Operand_GetTypeId(Opnd));
    }
    gcmASSERT(VIR_TypeId_isPrimitive(tyId));
    tyId = VIR_GetTypeRowType(tyId);
    return tyId;
}

static gctBOOL
_expand_first_mov(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_Operand * dest = VIR_Inst_GetDest(Inst);
    VIR_TypeId    tyId = _getOpndRowTypeId(Context, Opnd);
    VIR_Operand_SetTypeId(dest, tyId);
    VIR_Operand_SetTypeId(Opnd, tyId);
    return gcvTRUE;
}

static gctBOOL
_expand_nth_mov(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd,
    IN gctINT              rowOffset
    )
{
    VSC_ErrCode   virErrCode = VSC_ERR_NONE;
    VIR_Symbol    *sym = VIR_Operand_GetSymbol(Opnd);
    VIR_VirRegId  regId;
    VIR_SymId     symId;
    VIR_Operand   *dest;
    VIR_TypeId    elemTyId = _getOpndRowTypeId(Context, Opnd);

    /* handle source operand */
    virErrCode = VIR_Lower_ChangeOperandByOffset(Context,
                                   Inst,
                                   Opnd,
                                   rowOffset);
    if(virErrCode != VSC_ERR_NONE) return gcvFALSE;

    /* handle dest operand */
    dest = VIR_Inst_GetDest(Inst);
    sym = VIR_Operand_GetSymbol(dest);
    regId = VIR_Symbol_GetOffsetTempIndex(sym, rowOffset);
    virErrCode = VIR_Shader_GetVirRegSymByVirRegId(Context->shader,
                                                   regId,
                                                   &symId);
    if(virErrCode != VSC_ERR_NONE)
    {
        return gcvFALSE;
    }
    if(symId == VIR_INVALID_ID)
    {
        /* create a virreg if it is not created before */
        virErrCode = VIR_Shader_AddSymbol(Context->shader,
                                          VIR_SYM_VIRREG,
                                          regId,
                                          VIR_Shader_GetTypeFromId(Context->shader, elemTyId),
                                          VIR_STORAGE_UNKNOWN,
                                          &symId);
        if(virErrCode != VSC_ERR_NONE) return gcvFALSE;
    }

    if(VIR_Lower_SetLongUlongInstType(Context, Inst, Opnd))
    {
        VIR_Operand_SetTempRegister(dest,
                                    VIR_Inst_GetFunction(Inst),
                                    symId,
                                    VIR_Operand_GetTypeId(dest));
        return gcvTRUE;
    }
    return gcvFALSE;
}

static gctBOOL
_expand_second_mov(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    return _expand_nth_mov(Context, Inst, Opnd, 1);
}

static gctBOOL
_expand_third_mov(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    return _expand_nth_mov(Context, Inst, Opnd, 2);
}

static gctBOOL
_expand_fourth_mov(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    return _expand_nth_mov(Context, Inst, Opnd, 3);
}

/*
copy 1, 2, 3,
    mov 1, 2
*/
static VIR_PatternMatchInst _copyPatInst0[] = {
    { VIR_OP_COPY, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { _copy16ByteOrLess }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _copyRepInst0[] = {
    { VIR_OP_MOV, -1, 0, { 1, 2, 0, 0 }, { 0 } },
};

static VIR_PatternMatchInst _copyPatInst1[] = {
    { VIR_OP_COPY, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { _copy32ByteOrLess }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _copyRepInst1[] = {
    { VIR_OP_MOV, 0, 0, {  1, 2, 0, 0 }, { 0, _expand_first_mov } },
    { VIR_OP_MOV, 0, 0, {  1, 2, 0, 0 }, { 0, _expand_second_mov } },
};

static VIR_PatternMatchInst _copyPatInst2[] = {
    { VIR_OP_COPY, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { _copy64ByteOrLess }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _copyRepInst2[] = {
    { VIR_OP_MOV, 0, 0, {  1, 2, 0, 0 }, { 0, _expand_first_mov } },
    { VIR_OP_MOV, 0, 0, {  1, 2, 0, 0 }, { 0, _expand_second_mov } },
    { VIR_OP_MOV, 0, 0, {  1, 2, 0, 0 }, { 0, _expand_third_mov } },
    { VIR_OP_MOV, 0, 0, {  1, 2, 0, 0 }, { 0, _expand_fourth_mov } },
};

static VIR_Pattern _copyPattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_copy, 0) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_copy, 1) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_copy, 2) },
    { VIR_PATN_FLAG_NONE }
};

static gctBOOL
_NonScalarOperandOnNECond(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    VIR_Swizzle swizzle0 = VIR_Operand_GetSwizzle(Inst->src[0]);
    VIR_Swizzle swizzle1 = VIR_Operand_GetSwizzle(Inst->src[1]);

    if (gcUseFullNewLinker(Context->vscContext->pSysCtx->pCoreSysCtx->hwCfg.hwFeatureFlags.hasHalti2))
    {
        if (VIR_Inst_GetConditionOp(Inst) == VIR_COP_NOT_EQUAL)
        {
            if ((swizzle0 != VIR_SWIZZLE_XXXX &&
                 swizzle0 != VIR_SWIZZLE_YYYY &&
                 swizzle0 != VIR_SWIZZLE_ZZZZ &&
                 swizzle0 != VIR_SWIZZLE_WWWW) ||
                (swizzle1 != VIR_SWIZZLE_XXXX &&
                 swizzle1 != VIR_SWIZZLE_YYYY &&
                 swizzle1 != VIR_SWIZZLE_ZZZZ &&
                 swizzle1 != VIR_SWIZZLE_WWWW))
            {
                return gcvTRUE;
            }
        }
    }

    return gcvFALSE;
}

/*
jmp 1, 2, 3,
    jmp_any 1, 2, 3
*/

static VIR_PatternMatchInst _jmpcPatInst0[] = {
    { VIR_OP_JMPC, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { _NonScalarOperandOnNECond }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _jmpcRepInst0[] = {
    { VIR_OP_JMP_ANY, -1, 0, { 1, 2, 3, 0 }, { 0 } },
};

static VIR_Pattern _jmpcPattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_jmpc, 0) },
    { VIR_PATN_FLAG_NONE }
};


static VIR_PatternMatchInst _cmadPatInst0[] = {
    { VIR_OP_CMAD, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_CMAD, VIR_PATTERN_ANYCOND, 0, { 1, 2, 4, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _cmadRepInst0[] = {
    { VIR_OP_CMAD, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 4 }, { 0 } },
};

static VIR_Pattern _cmadPattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_cmad, 0) },
    { VIR_PATN_FLAG_NONE }
};

static VIR_PatternMatchInst _cmadcjPatInst0[] = {
    { VIR_OP_CMADCJ, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_CMADCJ, VIR_PATTERN_ANYCOND, 0, { 1, 2, 4, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _cmadcjRepInst0[] = {
    { VIR_OP_CMADCJ, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 4 }, { 0 } },
};

static VIR_Pattern _cmadcjPattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_cmadcj, 0) },
    { VIR_PATN_FLAG_NONE }
};

static VIR_Pattern*
_GetgcSL2VirPatterns(
    IN VIR_PatternContext      *Context,
    IN VIR_Instruction         *Inst
    )
{
    switch(VIR_Inst_GetOpcode(Inst))
    {
    case VIR_OP_ADD:
        return _addPattern;
    case VIR_OP_ACOS:
        return _acosPattern;
    case VIR_OP_ASIN:
        return _asinPattern;
    case VIR_OP_ARCTRIG:
        return _arctrigPattern;
    case VIR_OP_MUL:
        return _mulPattern;
    case VIR_OP_COMPARE:
        return _cmpPattern;
    case VIR_OP_SET:
        return _setPattern;
    case VIR_OP_CONVERT:
        return _convPattern;
    case VIR_OP_CONV0:
        return _conv0Pattern;
    case VIR_OP_STARR:
        return _storePattern;
    case VIR_OP_TEXLD:
        return _texldPattern;
    case VIR_OP_TEXLD_U:
        return _texlduPattern;
    case VIR_OP_LOAD:
        return _loadPattern;
    case VIR_OP_ATOMADD:
        return _atomADDPattern;
    case VIR_OP_ATOMSUB:
        return _atomSUBPattern;
    case VIR_OP_ATOMXCHG:
        return _atomXCHGPattern;
    case VIR_OP_ATOMCMPXCHG:
        return _atomCMPXCHGPattern;
    case VIR_OP_ATOMMIN:
        return _atomMINPattern;
    case VIR_OP_ATOMMAX:
        return _atomMAXPattern;
    case VIR_OP_ATOMOR:
        return _atomORPattern;
    case VIR_OP_ATOMAND:
        return _atomANDPattern;
    case VIR_OP_ATOMXOR:
        return _atomXORPattern;
    case VIR_OP_BITRANGE:
        return _bitrangePattern;
    case VIR_OP_BITRANGE1:
        return _bitrange1Pattern;
    case VIR_OP_COPY:
        return _copyPattern;
    case VIR_OP_VX_IMG_STORE:
        return _vxImgStorePattern;
    case VIR_OP_VX_IMG_STORE_3D:
        return _vxImgStore3DPattern;
    case VIR_OP_VX_IMG_LOAD:
        return _vxImgLoadPattern;
    case VIR_OP_VX_IMG_LOAD_3D:
        return _vxImgLoad3DPattern;
    case VIR_OP_IMG_SAMPLER:
        return _imgSamplerPattern;
    case VIR_OP_JMPC:
        return _jmpcPattern;
    case VIR_OP_CMAD:
        return _cmadPattern;
    case VIR_OP_CMADCJ:
        return _cmadcjPattern;
    default:
        break;
    }
    return gcvNULL;
}

#endif


