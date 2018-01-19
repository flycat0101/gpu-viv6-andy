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


#ifndef __gc_vsc_asm_al_codec_h_
#define __gc_vsc_asm_al_codec_h_

BEGIN_EXTERN_C()


   /* !!!!!!!!!!!!NEED TO REDEFINE BASED ON GEN's ASSEMBLER !!!!!!!!!!!!! */



/******************************************************************************\
|******************************* SHADER ASM CODE ******************************|
\******************************************************************************/
typedef enum _vscMC_OPCODE
{
#define vscMC_DEF(code, val, f32Ok, f16Ok, i8Ok, i16Ok, i32Ok, isTrans, isCtrl, desc) VSC_MC_##code,
#define vscEMC_DEF(code, val, f32Ok, f16Ok, i8Ok, i16Ok, i32Ok, isTrans, isCtrl, desc)
#include "asm/gc_vsc_asm_opcode.def.h"
#undef vscMC_DEF
#undef vscEMC_DEF
/* define extended code */
#define vscMC_DEF(code, val, f32Ok, f16Ok, i8Ok, i16Ok, i32Ok, isTrans, isCtrl, desc)
#define vscEMC_DEF(code, val, f32Ok, f16Ok, i8Ok, i16Ok, i32Ok, isTrans, isCtrl, desc) VSC_MC_##code,
#include "asm/gc_vsc_asm_opcode.def.h"
#undef vscMC_DEF
#undef vscEMC_DEF
} vscMC_OPCODE;

/* gcMC_OPCODE  Value  F32   F16   I8    I16   I32  Trans  Ctrl Desc */
typedef struct _vscMC_Info
{
    vscMC_OPCODE   opcode;
    gctUINT        chipOpcodeVal  : 8;
    gctUINT        chipExtOpcode  : 8;
    gctUINT        isExtOpcode    : 1;
    gctUINT        F32Ok          : 1;
    gctUINT        F16Ok          : 1;
    gctUINT        I8Ok           : 1;
    gctUINT        I16Ok          : 1;
    gctUINT        I32Ok          : 1;
    gctSTRING      desc;
} vscMC_Info;

typedef enum _vscMC_InstType
{
    /* Instruction type. */
    VSC_MC_INST_TYPE_FLOAT32    = 0x0,
    VSC_MC_INST_TYPE_FLOAT16    = 0x1,
    VSC_MC_INST_TYPE_SIGNED32   = 0x2,
    VSC_MC_INST_TYPE_SIGNED16   = 0x3,
    VSC_MC_INST_TYPE_SIGNED8    = 0x4,
    VSC_MC_INST_TYPE_UNSIGNED32 = 0x5,
    VSC_MC_INST_TYPE_UNSIGNED16 = 0x6,
    VSC_MC_INST_TYPE_UNSIGNED8  = 0x7,

} vscMC_InstType;

typedef enum _vscMC_ThreadMode
{
    /* single 32 mode */
    VSC_MC_ThreadMode_Single32 = 0,

    /* dual 16 mode */
    VSC_MC_ThreadMode_T0T1 = 0, /* dual 16 two threads */
    VSC_MC_ThreadMode_T0   = 1, /* single 32 Thread 0  */
    VSC_MC_ThreadMode_T1   = 2, /* single 32 Thread 1  */
} vscMC_ThreadMode;

typedef enum _vscMC_SrcRegType
{
    VSC_MC_REG_TYPE_TEMP            = 0,
    VSC_MC_REG_TYPE_MEDIUMP_TEMP    = 0, /* for dual-16 */
    VSC_MC_REG_TYPE_FACE            = 1,
    VSC_MC_REG_TYPE_UNBOUNDED_CONST = 2,
    VSC_MC_REG_TYPE_BOUNDED_CONST   = 3,
    VSC_MC_REG_TYPE_HIGHP_TEMP      = 4, /* In dual-16 mode, highp
                                           * temporary registers*/
    VSC_MC_REG_TYPE_EXTENDED        = 5,
    VSC_MC_REG_TYPE_VERTEX_ID       = 4, /* deprecated as of 5.4.2,
                                           * Moved into the attributes */
    VSC_MC_REG_TYPE_INSTANCE_ID     = 5, /* deprecated as of 5.4.2,
                                           * Moved into the attributes */
    VSC_MC_REG_TYPE_LOCAL           = 6,
    VSC_MC_REG_TYPE_IMMEDIATE       = 7,
    VSC_MC_REG_TYPE_SAMPLER         = 8,
    VSC_MC_REG_TYPE_TARGET          = 9, /* call/branch target PC */
    VSC_MC_REG_TYPE_NEXTPC          = 10,


} vscMC_SrcRegType;

typedef struct _vscMC_Dest
{
    gctUINT             destValid        : 1;
    vscMC_SrcRegType    destRegType      : 5;   /* destination register type */
    gctUINT             destEnable       : 4;
    gctUINT             destModifier     : 2;   /* dest modifier */
    gctUINT             destAddr         : 9;   /* dest register address */
    gctUINT             destRelativeAddr : 3;   /* dest relative addressing mode:
                                                 * 0: none.
                                                 * 1:a0.x, 2:a0.y, 3:a0.z, 4:a0.w
                                                 * 5:aL
                                                 * 6:Instance ID (deprecated from 5.4.2)*/
} vscMC_Dest;

typedef struct _vscMC_SourceReg
{
    gctUINT             srcValid        : 1;
    vscMC_SrcRegType    srcRegType      : 5;   /* source register type */
    gctUINT             srcValueNegate  : 1;
    gctUINT             srcValueAbs     : 1;
    gctUINT             srcAddr         : 9;   /* source address */
    gctUINT             srcSwizzle      : 8;   /* source register swizzle */
    gctUINT             srcRelativeAddr : 3;   /* dest relative addressing mode:
                                                * 0: none.
                                                * 1:a0.x, 2:a0.y, 3:a0.z, 4:a0.w
                                                * 5:aL
                                                * 6:Instance ID (deprecated from 5.4.2)*/
} vsc_MC_SourceReg;

typedef struct _vscMC_SourceImm20
{
    gctUINT             srcValid        : 1;
    vscMC_SrcRegType    srcRegType      : 5;   /* source register type */
    vscMC_InstType      immType         : 4;
    gctUINT             immBits         : 20;  /* 20 bits immediate value */
} vscMC_SourceImm20;

typedef struct _vscMC_SourceSampler
{
    gctUINT             srcValid        : 1;
    vscMC_SrcRegType    srcRegType      : 5;   /* source register type */
    gctUINT             samplerId       : 5;   /* texture sampler number */
    gctUINT             samplerRelativeAddr : 3;   /* dest relative addressing mode:
                                                * 0: none.
                                                * 1:a0.x, 2:a0.y, 3:a0.z, 4:a0.w
                                                * 5:aL
                                                * 6:Instance ID (deprecated from 5.4.2)*/
    gctUINT             texureSwizzle   : 8;   /* texure swizzle */
} vscMC_SourceSampler;

typedef struct _vscMC_SourceTarget
{
    gctUINT             srcValid        : 1;
    vscMC_SrcRegType    srcRegType      : 5;   /* source register type */
    gctUINT             targetPC        : 26;
} vscMC_SourceTarget;

typedef struct _vscMC_SourceCommon
{
    gctUINT             srcValid        : 1;
    vscMC_SrcRegType    srcRegType      : 5;   /* source register type */
    gctUINT             bits            : 26;
} vscMC_SourceCommon;


typedef union _vscMC_Source
{
    vscMC_SourceCommon      common;
    vsc_MC_SourceReg        reg;
    vscMC_SourceImm20       imm;
    vscMC_SourceSampler     sampler;
    vscMC_SourceTarget      targetPC;
} vscMC_Source;

typedef struct _vscMC_Instr
{
    vscMC_OPCODE        opcode       : 9;
    VIR_ConditionOp     compFunc     : 6;
    gctUINT             saturate     : 2;
    gctUINT             rndMode      : 2;   /* ??? */
    vscMC_InstType      instType     : 5;   /* instruction result type */
    vscMC_ThreadMode    threadType   : 3;

    vscMC_Dest          dest;
    vscMC_Source        src[3];
}
vscMC_Instr;


END_EXTERN_C()

#endif /* __gc_vsc_asm_al_codec_h_ */

