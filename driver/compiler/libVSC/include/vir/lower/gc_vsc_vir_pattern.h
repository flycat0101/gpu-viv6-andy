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


#ifndef __gc_vsc_vir_pattern_
#define __gc_vsc_vir_pattern_

#include "gc_vsc.h"

BEGIN_EXTERN_C()

#define VIR_PATTERN_OPND_COUNT  (6)

#define VIR_PATTERN_ANYCOND     (-1)

#define VIR_PATTERN_TEMP_COUNT  (32)

#define CODEPATTERN(Inst, Idx) Inst##PatInst##Idx, sizeof(Inst##PatInst##Idx)/sizeof(VIR_PatternMatchInst), \
    Inst##RepInst##Idx, sizeof(Inst##RepInst##Idx)/sizeof(VIR_PatternReplaceInst)

typedef struct _VIR_PATTERN_CONTEXT     VIR_PatternContext;

typedef gctBOOL (*VIR_PATTERN_MATCH_FUNCTION_PTR)(
    IN VIR_PatternContext   *Context,
    IN VIR_Instruction      *Inst
    );

typedef gctBOOL (*VIR_PATTERN_REPLACE_FUNCTION_PTR)(
    IN VIR_PatternContext   *Context,
    IN VIR_Instruction      *Inst,
    IN VIR_Operand          *Opnd
    );

typedef enum _VIR_PATN_MATCH_INST_FLAG
{

    VIR_PATN_MATCH_FLAG_OR                      = 0x00,

    VIR_PATN_MATCH_FLAG_AND                     = 0x01,

} VIR_PatnMatchInstFlag;

typedef struct _VIR_PATTERN_MATCH_INST
{
    /* Opcode. */
    VIR_OpCode               opcode;
    /* On replaced mode, negative number is for copying the condition of condOp-th pattern instrution. */
    gctINT                   condOp       : 8;
    /* */
    gctINT                   reserve_     : 24;
    /* Operands dest, src0, src1, src2, src3 */
    gctINT                   opnd[VIR_PATTERN_OPND_COUNT];

    VIR_PATTERN_MATCH_FUNCTION_PTR function[VIR_PATTERN_OPND_COUNT];

    VIR_PatnMatchInstFlag    flag;

} VIR_PatternMatchInst;

typedef enum _VIR_PATN_REPLACE_FLAG
{
    VIR_PATN_REPLACE_FLAG_DEFAULT = 0, /* copy last inst dest info to temp reg */

    /* 1 ~ 255: copy operand by reference number to temp reg */
    VIR_PATN_REPLACE_FLAG_REFER_NO_MASK = 0xFF,

    /* special temp register */
    VIR_PATTERN_TEMP_TYPE_X         = 0x80000001,
    VIR_PATTERN_TEMP_TYPE_Y         = 0x80000002,
    VIR_PATTERN_TEMP_TYPE_Z         = 0x80000004,
    VIR_PATTERN_TEMP_TYPE_W         = 0x80000008,

    VIR_PATTERN_TEMP_TYPE_XY        = ((VIR_PATTERN_TEMP_TYPE_X) | (VIR_PATTERN_TEMP_TYPE_Y)),
    VIR_PATTERN_TEMP_TYPE_XZ        = ((VIR_PATTERN_TEMP_TYPE_X) | (VIR_PATTERN_TEMP_TYPE_Z)),
    VIR_PATTERN_TEMP_TYPE_XW        = ((VIR_PATTERN_TEMP_TYPE_X) | (VIR_PATTERN_TEMP_TYPE_W)),
    VIR_PATTERN_TEMP_TYPE_XYZ       = ((VIR_PATTERN_TEMP_TYPE_X) | (VIR_PATTERN_TEMP_TYPE_Y) | (VIR_PATTERN_TEMP_TYPE_Z)),
    VIR_PATTERN_TEMP_TYPE_XYW       = ((VIR_PATTERN_TEMP_TYPE_X) | (VIR_PATTERN_TEMP_TYPE_Y) | (VIR_PATTERN_TEMP_TYPE_W)),
    VIR_PATTERN_TEMP_TYPE_XZW       = ((VIR_PATTERN_TEMP_TYPE_X) | (VIR_PATTERN_TEMP_TYPE_Z) | (VIR_PATTERN_TEMP_TYPE_W)),
    VIR_PATTERN_TEMP_TYPE_XYZW      = ((VIR_PATTERN_TEMP_TYPE_X) | (VIR_PATTERN_TEMP_TYPE_Y) | (VIR_PATTERN_TEMP_TYPE_Z) | (VIR_PATTERN_TEMP_TYPE_W)),

    VIR_PATTERN_TEMP_TYPE_YZ        = ((VIR_PATTERN_TEMP_TYPE_Y) | (VIR_PATTERN_TEMP_TYPE_Z)),
    VIR_PATTERN_TEMP_TYPE_YW        = ((VIR_PATTERN_TEMP_TYPE_Y) | (VIR_PATTERN_TEMP_TYPE_W)),
    VIR_PATTERN_TEMP_TYPE_YZW       = ((VIR_PATTERN_TEMP_TYPE_Y) | (VIR_PATTERN_TEMP_TYPE_Z) | (VIR_PATTERN_TEMP_TYPE_W)),

    VIR_PATTERN_TEMP_TYPE_ZW        = ((VIR_PATTERN_TEMP_TYPE_Z) | (VIR_PATTERN_TEMP_TYPE_W)),

    VIR_PATTERN_TEMP_TYPE_MASK      = 0x8000001F,
    VIR_PATTERN_TEMP_TYPE_FLAG      = 0x80000000,
} VIR_PatnReplaceFlag;

typedef struct _VIR_PATTERN_REPLACE_INST
{
    /* Opcode. */
    VIR_OpCode               opcode;
    /* On replaced mode, negative number is for copying the condition of condOp-th pattern instrution. */
    gctINT                   condOp       : 8;
    /* */
    VIR_PatnReplaceFlag      flag;

    /* Operands dest, src0, src1, src2, src3 */
    gctINT                   opnd[VIR_PATTERN_OPND_COUNT];

    VIR_PATTERN_REPLACE_FUNCTION_PTR function[VIR_PATTERN_OPND_COUNT];

} VIR_PatternReplaceInst;

#define VIR_PATTERN_DefaultTempType(Patn) ((Patn)->flag == VIR_PATN_REPLACE_FLAG_DEFAULT)
#define VIR_PATTERN_GetTempType(Patn) ((Patn)->flag & VIR_PATTERN_TEMP_TYPE_MASK)
#define VIR_PATTERN_SpecialTempType(Patn) (((Patn)->flag & VIR_PATTERN_TEMP_TYPE_FLAG) == VIR_PATTERN_TEMP_TYPE_FLAG)
#define VIR_PATTERN_GetReferNo(Patn) ((Patn)->flag & VIR_PATN_REPLACE_FLAG_REFER_NO_MASK)

typedef enum _VIR_PATN_FLAG
{
    VIR_PATN_FLAG_NONE                    = 0x00,

    /* Manual delete unused operands. Some instructions like texld can not auto delete operands. */
    VIR_PATN_FLAG_DELETE_MANUAL           = 0x01,

    /* Expand sin, cos, div and so on. Only support 1 to n for now. */
    VIR_PATN_FLAG_EXPAND_COMPONENT_INLINE = 0x02,

    /**
     * Expand idiv, imod. The number of expanded instructions is too much.
     * In loop will improve instruction-cache hit radio. Only support 1 to n for now.
     */
    VIR_PATN_FLAG_EXPAND_COMPONENT_LOOP   = 0x04,

    /* Expand instruciton across function. Only support 1 to n for now. */
    VIR_PATN_FLAG_EXPAND_COMPONENT_CALL   = 0x08,

    VIR_PATN_FLAG_EXPAND                  = VIR_PATN_FLAG_EXPAND_COMPONENT_INLINE |
                                            VIR_PATN_FLAG_EXPAND_COMPONENT_LOOP |
                                            VIR_PATN_FLAG_EXPAND_COMPONENT_CALL,

    /**
     * Expand RCP, RSQ, EXP, LOG, SQRT, DIV, IDIV, IMOD, SIN, COS.
     * These instructions only can be computed one component everytime.
     * We need expand them now or later at ll.
     */
    VIR_PATN_FLAG_EXPAND_MODE_COMPONENT_O2O         = 0x10,

    /* Expand DP3, DP4. Reserved. */
    VIR_PATN_FLAG_EXPAND_MODE_COMPONENT_O2N         = 0x20,

    /* When generate temp, do not set its type, enable. */
    VIR_PATN_FLAG_SET_TEMP_IN_FUNC                  = 0x40,

    /* There is a circle when high level opcodes expand to low level opcode, enable. */
    VIR_PATN_FLAG_NO_RECURSION                      = 0x80,

    /* Generally, comp-inline relies on dst's enable, but sometimes, such as JMP, as
       dst is branch target, we will use src to determine */
    VIR_PATN_FLAG_EXPAND_COMP_INLINE_NOT_BY_DST     = 0x100,

    VIR_PATN_FLAG_EXPAND_COMP_O2O_SRC_ONLY_INLINE   =  VIR_PATN_FLAG_EXPAND_COMPONENT_INLINE |
                                                       VIR_PATN_FLAG_EXPAND_MODE_COMPONENT_O2O |
                                                       VIR_PATN_FLAG_EXPAND_COMP_INLINE_NOT_BY_DST,

    /*
    ** After expand current instruction, move the index to the original previous instruction
    ** and scan the expand instructions.
    */
    VIR_PATN_FLAG_RECURSIVE_SCAN                    = 0x200,

   /*
    ** After expand current instruction, move the index to the newly added instruction
    ** and scan the expand instructions.
    */
    VIR_PATN_FLAG_RECURSIVE_SCAN_NEWINST            = 0x400,

   /*
    ** After expand current instruction, copy the rounding mode to the newly added instruction
    */
    VIR_PATN_FLAG_COPY_ROUNDING_MODE                = 0x800,

   /*
    ** Don't expand a texld parameter operand.
    */
    VIR_PATN_FLAG_NOT_EXPAND_TEXLD_PARM_NODE        = 0x1000,

   /*
    ** Don't expand a parameter operand.
    */
    VIR_PATN_FLAG_NOT_EXPAND_PARAM_NODE             = 0x2000,

   /*
    ** Pattern already matched and replaced
    */
    VIR_PATN_FLAG_ALREADY_MATCHED_AND_REPLACED      = 0x4000,

} VIR_PatnFlag;

typedef struct _VIR_PATTERN
{
    VIR_PatnFlag                      flags;

    VIR_PatternMatchInst             *matchInsts;
    gctUINT                           matchCount;

    VIR_PatternReplaceInst           *replaceInsts;
    gctUINT                           repalceCount;
} VIR_Pattern;

typedef VIR_Pattern* (*VIR_PATTERN_GET_PATTERN_PTR)(
    IN  VIR_PatternContext      *Context,
    IN  VIR_Instruction         *Inst
    );

typedef gctBOOL      (*VIR_PATTERN_CMP_OPCODE_PTR)(
    IN VIR_PatternContext    *Context,
    IN VIR_PatternMatchInst  *Inst0,
    IN VIR_Instruction       *Inst1
    );

typedef enum _VIR_PATN_CONTEXT_FLAG
{
    VIR_PATN_CONTEXT_FLAG_NONE                    = 0x00,
} VIR_PatnContextFlag;

struct _VIR_PATTERN_CONTEXT
{
    PVSC_CONTEXT                vscContext;
    VIR_Shader                  *shader;
    VIR_Symbol                  *tmpRegSymbol[VIR_PATTERN_TEMP_COUNT];
    VIR_PatnContextFlag          flag;
    gctINT                       baseTmpRegNo;
    gctINT                       maxTmpRegNo;

    gctBOOL                      changed;
    VIR_PATTERN_GET_PATTERN_PTR  getPattern;
    VIR_PATTERN_CMP_OPCODE_PTR   cmpOpcode;

    VSC_MM                      *pMM;
};

typedef struct _VIR_PATTERN_LOWER_CONTEXT
{
    VIR_PatternContext          header;
    VSC_HW_CONFIG*              hwCfg;
    VSC_MM*                     pMM;
    gctBOOL                     generateImmediate;
    gctBOOL                     hasNEW_TEXLD;
    gctBOOL                     isCL_X;
    gctBOOL                     hasCL;
    gctBOOL                     hasHalti1;
    gctBOOL                     hasHalti2;
    gctBOOL                     hasHalti3;
    gctBOOL                     hasHalti4;
    gctBOOL                     hasSHEnhancements2;
} VIR_PatternLowerContext;

VSC_ErrCode
VIR_Pattern_Transform(
    IN VIR_PatternContext *Context
    );

VSC_ErrCode
VIR_PatternContext_Initialize(
    IN OUT VIR_PatternContext      *Context,
    IN PVSC_CONTEXT                vscContext,
    IN VIR_Shader                  *Shader,
    IN VSC_MM                      *pMM,
    IN VIR_PatnContextFlag          Flag,
    IN VIR_PATTERN_GET_PATTERN_PTR  GetPatrernPtr,
    IN VIR_PATTERN_CMP_OPCODE_PTR   CmpOpcodePtr,
    IN gctSIZE_T                    Size
    );

VSC_ErrCode
VIR_PatternContext_Finalize(
    IN OUT VIR_PatternContext      *Context
    );

END_EXTERN_C()
#endif



