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


#include "gc_vsc.h"

/* compiler time checks */
/* compiler time assert if sizeof(VIR_Operand_TexldModifier) larger than VIR_Operand */

/* VIR op code name string */
#define VIR_OPINFO(OPCODE, OPNDNUM, FLAGS, WRITE2DEST, LEVEL)   #OPCODE
const gctSTRING VIR_OpName[] =
{
#include "vir/ir/gc_vsc_vir_opcode.def.h"
};
#undef VIR_OPINFO

#define VIR_INTRINSIC_INFO(Intrinsic)   #Intrinsic
const gctSTRING VIR_IntrinsicName[] =
{
#include "vir/ir/gc_vsc_vir_intrinsic_kind.def.h"
};
#undef VIR_INTRINSIC_INFO

const gctSTRING VIR_CondOpName[] =
{
    "", /* 0x00     0x00 */
    ".gt", /* 0x01       0x01 */
    ".lt", /* 0x02       0x02 */
    ".ge", /* 0x03       0x03 */
    ".le", /* 0x04       0x04 */
    ".eq", /* 0x05       0x05 */
    ".ne", /* 0x06       0x06 */
    ".and", /* 0x07      0x07 */
    ".or", /* 0x08       0x08 */
    ".xor", /* 0x09      0x09 */
    ".not", /* 0x0A      0x0A */
    ".nz", /* 0x0B       0x0B */
    ".gez", /* 0x0C      0x0C */
    ".gz", /* 0x0D       0x0D */
    ".lez", /* 0x0E      0x0E */
    ".lz", /* 0x0F       0x0F */
    ".fin", /* 0x10   0x10 */
    ".infin", /* 0x11 0x11 */
    ".nan", /* 0x12      0x12 */
    ".normal", /* 0x13   0x13 */
    ".anymsb", /* 0x14   0x14 */
    ".allmsb", /* 0x15   0x15 */
    ".selmsb", /* 0x16   0x16 */
    ".ucarry", /* 0x17    0x17 */
    ".helper", /* 0x18    0x18 */
    ".nothelper"    /* 0x19 0x19 */
    "!$%!@$$"
};

const gctSTRING VIR_RoundModeName[] =
{
    "", /* VIR_ROUND_DEFAULT */
    ".rte", /* VIR_ROUND_RTE: Round to nearest even */
    ".rtz", /* VIR_ROUND_RTZ: Round toward zero */
    ".rtp", /* VIR_ROUND_RTP: Round toward positive infinity */
    ".rtn", /* VIR_ROUND_RTN: Round toward negative infinity */
};

/* destination modifiers */
const gctSTRING VIR_DestModifierName[] =
{
    "", /* VIR_MOD_NONE     */
    ".sat", /* VIR_MOD_SAT_0_TO_1: Satruate the value between [0.0, 1.0] */
    ".sat0", /* VIR_MOD_SAT_0_TO_INF: Satruate the value between [0.0, +inf) */
    ".sat1"         /* VIR_MOD_SAT_NINF_TO_1: Satruate the value between (-inf, 1.0] */
    "",
};

/* source modifiers */
const gctSTRING VIR_SrcModifierName[] =
{
    "", /* VIR_MOD_NONE     */
    ".neg", /* VIR_MOD_NEG: source negate modifier */
    ".abs", /* VIR_MOD_ABS: source absolute modfier */
    ".abs.neg"      /* VIR_MOD_ABS | VIR_MOD_NEG: -abs(src) */
};


/* type table operations */
VIR_TypeId vscAddPrimTypeToTable(VIR_TypeTable* pTypeTbl, const char* pStr, gctUINT len);

/* builtin special names */
VIR_NameId  VIR_NAME_UNKNOWN,
            VIR_NAME_POSITION,
            VIR_NAME_POINT_SIZE,
            VIR_NAME_COLOR,
            VIR_NAME_FRONT_FACING,
            VIR_NAME_POINT_COORD,
            VIR_NAME_POSITION_W,
            VIR_NAME_DEPTH,
            VIR_NAME_FOG_COORD,
            VIR_NAME_VERTEX_ID,
            VIR_NAME_FRONT_COLOR,
            VIR_NAME_BACK_COLOR,
            VIR_NAME_FRONT_SECONDARY_COLOR,
            VIR_NAME_BACK_SECONDARY_COLOR,
            VIR_NAME_TEX_COORD,
            VIR_NAME_INSTANCE_ID,
            VIR_NAME_NUM_GROUPS,
            VIR_NAME_WORKGROUPSIZE,
            VIR_NAME_WORK_GROUP_ID,
            VIR_NAME_LOCAL_INVOCATION_ID,
            VIR_NAME_GLOBAL_INVOCATION_ID,
            VIR_NAME_LOCALINVOCATIONINDEX,
            VIR_NAME_HELPER_INVOCATION,
            VIR_NAME_SUBSAMPLE_DEPTH,
            VIR_NAME_PERVERTEX, /* gl_PerVertex */
            VIR_NAME_IN, /* gl_in */
            VIR_NAME_OUT, /* gl_out */
            VIR_NAME_INVOCATION_ID, /* gl_InvocationID */
            VIR_NAME_PATCH_VERTICES_IN, /* gl_PatchVerticesIn */
            VIR_NAME_PRIMITIVE_ID, /* gl_PrimitiveID */
            VIR_NAME_TESS_LEVEL_OUTER, /* gl_TessLevelOuter */
            VIR_NAME_TESS_LEVEL_INNER, /* gl_TessLevelInner */
            VIR_NAME_LAYER, /* gl_Layer */
            VIR_NAME_PS_OUT_LAYER, /* gl_Layer only for ps's output */
            VIR_NAME_PRIMITIVE_ID_IN, /* gl_PrimitiveIDIn */
            VIR_NAME_TESS_COORD, /* gl_TessCoord */
            VIR_NAME_SAMPLE_ID, /* gl_SampleID */
            VIR_NAME_SAMPLE_POSITION, /* gl_SamplePosition */
            VIR_NAME_SAMPLE_MASK_IN, /* gl_SampleMaskIn */
            VIR_NAME_SAMPLE_MASK, /* gl_SampleMask */
            VIR_NAME_IN_POSITION, /* gl_in.gl_Position*/
            VIR_NAME_IN_POINT_SIZE, /* gl_in.gl_PointSize*/
            VIR_NAME_BOUNDING_BOX, /* gl_BoundingBox*/
            VIR_NAME_BUILTIN_LAST;

VIR_BuiltinTypeInfo VIR_builtinTypes[] =
{
#include "gc_vsc_vir_builtin_types.def.h"
};


gctUINT
VIR_Inst_GetSourceIndex(
    IN VIR_Instruction     *pInst,
    IN VIR_Operand         *pOpnd
    )
{
    gctUINT srcIndex;
    for (srcIndex = 0; srcIndex < VIR_MAX_SRC_NUM; srcIndex++)
    {
        if (VIR_Inst_GetSource(pInst, srcIndex) == pOpnd)
        {
            return srcIndex;
        }
    }

    return VIR_MAX_SRC_NUM;
}

extern gctUINT
VIR_Inst_GetEvisState(
    IN VIR_Instruction     *pInst,
    IN VIR_Operand         *pOpnd
    )
{
    VIR_EVIS_State state;
    VIR_EVIS_Modifier evisModifier;
    gctUINT i;

    evisModifier.u1 = 0;
    /* find EvisModifier operand  */
    for (i=0; i<VIR_Inst_GetSrcNum(pInst); i++)
    {
        VIR_Operand * opnd = VIR_Inst_GetSource(pInst, i);
        if (opnd && VIR_Operand_GetOpKind(opnd) == VIR_OPND_EVIS_MODIFIER)
        {
            evisModifier.u1 = VIR_Operand_GetEvisModifier(opnd);
            break;
        }
    }
    state.u1 = 0;
    if (i != VIR_Inst_GetSrcNum(pInst))
    {
        switch (VIR_Inst_GetOpcode(pInst)) {
        case VIR_OP_VX_ABSDIFF:
            state.u2.roundingMode = evisModifier.u0.roundingMode;
            break;
        case VIR_OP_VX_IADD:
            state.u3.src0Format = evisModifier.u0.src0Format;
            state.u3.src1Format = evisModifier.u0.src1Format;
            state.u3.src2Format = evisModifier.u0.src2Format;
            break;
        case VIR_OP_VX_IACCSQ:
            state.u4.src1Format = evisModifier.u0.src1Format;
            state.u4.signExt    = evisModifier.u0.signExt;
            break;
        case VIR_OP_VX_LERP:
            state.u5.src0Format = evisModifier.u0.src0Format;
            state.u5.src1Format = evisModifier.u0.src1Format;
            break;
        case VIR_OP_VX_FILTER:
            state.u6.srcFormat = evisModifier.u0.src0Format;
            state.u6.filter    = evisModifier.u0.filterMode;
            break;
        case VIR_OP_VX_MAGPHASE:
            state.u7.srcFormat     = evisModifier.u0.src0Format;
            state.u7.disablePreAdj = evisModifier.u0.preAdjust;
            state.u7.rangePi       = evisModifier.u0.rangePi;
            break;
        case VIR_OP_VX_MULSHIFT:
            state.u8.srcFormat0 = evisModifier.u0.src0Format;
            state.u8.srcFormat1 = evisModifier.u0.src1Format;
            state.u8.rounding   =
                (evisModifier.u0.roundingMode == VX_RM_ToNearestEven) ? 1 : 0;
            break;
        case VIR_OP_VX_DP16X1:
        case VIR_OP_VX_DP8X2:
        case VIR_OP_VX_DP4X4:
        case VIR_OP_VX_DP2X8:
            state.u9.srcFormat0 = evisModifier.u0.src0Format;
            state.u9.srcFormat1 = evisModifier.u0.src1Format;
            state.u9.rounding   = evisModifier.u0.roundingMode;
            break;
        case VIR_OP_VX_DP32X1:
        case VIR_OP_VX_DP16X2:
        case VIR_OP_VX_DP8X4:
        case VIR_OP_VX_DP4X8:
        case VIR_OP_VX_DP2X16:
            state.u10.srcFormat01 = evisModifier.u0.src0Format;
            state.u10.rounding    = evisModifier.u0.roundingMode;
            state.u10.srcFormat1  = evisModifier.u0.src1Format;
            break;
        case VIR_OP_VX_CLAMP:
            state.u11.srcFormat   = evisModifier.u0.src0Format;
            state.u11.enableBool  = evisModifier.u0.enableBool ;
            break;
        case VIR_OP_VX_BILINEAR:
            state.u12.srcFormat01 = evisModifier.u0.src0Format;
            state.u12.srcFormat2  = evisModifier.u0.src2Format;
            break;
        case VIR_OP_VX_SELECTADD:
            state.u13.srcFormat0  = evisModifier.u0.src0Format;
            state.u13.srcFormat1  = evisModifier.u0.src1Format;
            break;
        case VIR_OP_VX_ATOMICADD:
            state.u14.srcFormat2  = evisModifier.u0.src2Format;
            break;
        case VIR_OP_VX_BITEXTRACT:
        case VIR_OP_VX_BITREPLACE:
            /* nothing in state */
            break;
        default:
            gcmASSERT(gcvFALSE);
            break;
        }
    }
    return state.u1;
}

gctBOOL
VIR_ConditionOp_Reversable(
    IN VIR_ConditionOp cond_op
    )
{
    /*VIR_COP_ALWAYS, 0x00     0x00 */
    /*VIR_COP_GREATER, 0x01       0x01 */
    /*VIR_COP_LESS, 0x02       0x02 */
    /*VIR_COP_GREATER_OR_EQUAL, 0x03       0x03 */
    /*VIR_COP_LESS_OR_EQUAL, 0x04       0x04 */
    /*VIR_COP_EQUAL, 0x05       0x05 */
    /*VIR_COP_NOT_EQUAL, 0x06       0x06 */
    /*VIR_COP_AND, 0x07      0x07 */
    /*VIR_COP_OR, 0x08       0x08 */
    /*VIR_COP_XOR, 0x09      0x09 */
    /*VIR_COP_NOT, 0x0A      0x0A */
    /*VIR_COP_NOT_ZERO, 0x0B       0x0B */
    /*VIR_COP_GREATER_OR_EQUAL_ZERO, 0x0C      0x0C */
    /*VIR_COP_GREATER_ZERO, 0x0D       0x0D */
    /*VIR_COP_LESS_OREQUAL_ZERO, 0x0E      0x0E */
    /*VIR_COP_LESS_ZERO, 0x0F       0x0F */
    /*VIR_COP_FINITE, 0x10   0x10 */
    /*VIR_COP_INFINITE, 0x11 0x11 */
    /*VIR_COP_NAN, 0x12      0x12 */
    /*VIR_COP_NORMAL, 0x13   0x13 */
    /*VIR_COP_ANYMSB, 0x14   0x14 */
    /*VIR_COP_ALLMSB, 0x15   0x15 */
    /*VIR_COP_SELMSB, 0x16   0x16 */
    /*VIR_COP_UCARRY, 0x17   0x17 */
    /*VIR_COP_HELPER, 0x18   0x18 */
    /*VIR_COP_NOTHELPER, 0x19 0x19 */
    switch(cond_op)
    {
        case VIR_COP_GREATER:
        case VIR_COP_LESS:
        case VIR_COP_GREATER_OR_EQUAL:
        case VIR_COP_LESS_OR_EQUAL:
        case VIR_COP_EQUAL:
        case VIR_COP_NOT_EQUAL:
        case VIR_COP_GREATER_OR_EQUAL_ZERO:
        case VIR_COP_GREATER_ZERO:
        case VIR_COP_LESS_OREQUAL_ZERO:
        case VIR_COP_LESS_ZERO:
        case VIR_COP_NOT:
        case VIR_COP_NOT_ZERO:
        case VIR_COP_FINITE:
        case VIR_COP_INFINITE:
            return gcvTRUE;
        default:
            return gcvFALSE;
    }
}

VIR_ConditionOp
VIR_ConditionOp_Reverse(
    IN VIR_ConditionOp cond_op
    )
{
    gcmASSERT(VIR_ConditionOp_Reversable(cond_op));
    switch(cond_op)
    {
        case VIR_COP_GREATER:
            return VIR_COP_LESS_OR_EQUAL;
        case VIR_COP_LESS:
            return VIR_COP_GREATER_OR_EQUAL;
        case VIR_COP_GREATER_OR_EQUAL:
            return VIR_COP_LESS;
        case VIR_COP_LESS_OR_EQUAL:
            return VIR_COP_GREATER;
        case VIR_COP_EQUAL:
            return VIR_COP_NOT_EQUAL;
        case VIR_COP_NOT_EQUAL:
            return VIR_COP_EQUAL;
        case VIR_COP_GREATER_OR_EQUAL_ZERO:
            return VIR_COP_LESS_ZERO;
        case VIR_COP_GREATER_ZERO:
            return VIR_COP_LESS_OREQUAL_ZERO;
        case VIR_COP_LESS_OREQUAL_ZERO:
            return VIR_COP_GREATER_ZERO;
        case VIR_COP_LESS_ZERO:
            return VIR_COP_GREATER_OR_EQUAL_ZERO;
        case VIR_COP_NOT:
            return VIR_COP_NOT_ZERO;
        case VIR_COP_NOT_ZERO:
            return VIR_COP_NOT;
        case VIR_COP_FINITE:
            return VIR_COP_INFINITE;
        case VIR_COP_INFINITE:
            return VIR_COP_FINITE;
        default:
            gcmASSERT(0);
            return VIR_COP_ALWAYS;
    }
}

gctBOOL
VIR_ConditionOp_CouldCompareWithZero(
    IN VIR_ConditionOp cond_op
    )
{
    /*VIR_COP_ALWAYS, 0x00     0x00 */
    /*VIR_COP_GREATER, 0x01       0x01 */
    /*VIR_COP_LESS, 0x02       0x02 */
    /*VIR_COP_GREATER_OR_EQUAL, 0x03       0x03 */
    /*VIR_COP_LESS_OR_EQUAL, 0x04       0x04 */
    /*VIR_COP_EQUAL, 0x05       0x05 */
    /*VIR_COP_NOT_EQUAL, 0x06       0x06 */
    /*VIR_COP_AND, 0x07      0x07 */
    /*VIR_COP_OR, 0x08       0x08 */
    /*VIR_COP_XOR, 0x09      0x09 */
    /*VIR_COP_NOT, 0x0A      0x0A */
    /*VIR_COP_NOT_ZERO, 0x0B       0x0B */
    /*VIR_COP_GREATER_OR_EQUAL_ZERO, 0x0C      0x0C */
    /*VIR_COP_GREATER_ZERO, 0x0D       0x0D */
    /*VIR_COP_LESS_OREQUAL_ZERO, 0x0E      0x0E */
    /*VIR_COP_LESS_ZERO, 0x0F       0x0F */
    /*VIR_COP_FINITE, 0x10   0x10 */
    /*VIR_COP_INFINITE, 0x11 0x11 */
    /*VIR_COP_NAN, 0x12      0x12 */
    /*VIR_COP_NORMAL, 0x13   0x13 */
    /*VIR_COP_ANYMSB, 0x14   0x14 */
    /*VIR_COP_ALLMSB, 0x15   0x15 */
    /*VIR_COP_SELMSB, 0x16   0x16 */
    /*VIR_COP_UCARRY, 0x17   0x17 */
    /*VIR_COP_HELPER, 0x18   0x18 */
    /*VIR_COP_NOTHELPER, 0x19 0x19 */
    switch(cond_op)
    {
        case VIR_COP_GREATER:
        case VIR_COP_LESS:
        case VIR_COP_GREATER_OR_EQUAL:
        case VIR_COP_LESS_OR_EQUAL:
        case VIR_COP_EQUAL:
        case VIR_COP_NOT_EQUAL:
            return gcvTRUE;
        default:
            return gcvFALSE;
    }
}

VIR_ConditionOp
VIR_ConditionOp_SetCompareWithZero(
    IN VIR_ConditionOp cond_op
    )
{
    gcmASSERT(VIR_ConditionOp_CouldCompareWithZero(cond_op));

    switch(cond_op)
    {
        case VIR_COP_GREATER:
            return VIR_COP_GREATER_ZERO;
        case VIR_COP_LESS:
            return VIR_COP_LESS_ZERO;
        case VIR_COP_GREATER_OR_EQUAL:
            return VIR_COP_GREATER_OR_EQUAL_ZERO;
        case VIR_COP_LESS_OR_EQUAL:
            return VIR_COP_LESS_OREQUAL_ZERO;
        case VIR_COP_EQUAL:
            return VIR_COP_NOT;
        case VIR_COP_NOT_EQUAL:
            return VIR_COP_NOT_ZERO;
        default:
            gcmASSERT(0);
            return VIR_COP_ALWAYS;
    }
}

VIR_ConditionOp
VIR_ConditionOp_SwitchLeftRight(
    IN VIR_ConditionOp cond_op
    )
{
    gcmASSERT(VIR_ConditionOp_DoubleOperand(cond_op));
    switch(cond_op)
    {
        case VIR_COP_GREATER:
            return VIR_COP_LESS;
        case VIR_COP_LESS:
            return VIR_COP_GREATER;
        case VIR_COP_GREATER_OR_EQUAL:
            return VIR_COP_LESS_OR_EQUAL;
        case VIR_COP_LESS_OR_EQUAL:
            return VIR_COP_GREATER_OR_EQUAL;
        default:
            return cond_op;
    }
}

gctUINT
VIR_ShaderKind_Map2KindId(
    IN VIR_ShaderKind kind
    )
{
    switch(kind)
    {
        case VIR_SHADER_VERTEX:
            return VSC_GFX_SHADER_STAGE_VS;
        case VIR_SHADER_FRAGMENT:
            return VSC_GFX_SHADER_STAGE_PS;
        case VIR_SHADER_COMPUTE:
            return VSC_CPT_SHADER_STAGE_CS;
        case VIR_SHADER_TESSELLATION_CONTROL:
            return VSC_GFX_SHADER_STAGE_HS;
        case VIR_SHADER_TESSELLATION_EVALUATION:
            return VSC_GFX_SHADER_STAGE_DS;
        case VIR_SHADER_GEOMETRY:
            return VSC_GFX_SHADER_STAGE_GS;

        default:
            gcmASSERT(0);
    }
    return VSC_MAX_LINKABLE_SHADER_STAGE_COUNT;
}

gctSTRING
VIR_GetSymbolKindName(
    IN VIR_SymbolKind  SymbolKind
    )
{
    static const gctSTRING symKindStr[] =
    {
        "SYM_UNKNOWN",
        "SYM_UNIFORM",
        "SYM_UBO", /* uniform block object */
        "SYM_VARIABLE", /* global/local variables, input/output */
        "SYM_BUFFER", /* buffer variables, use memory to read/write */
        "SYM_FIELD", /* the field of class/struct/union/ubo */
        "SYM_FUNCTION", /* function */
        "SYM_SAMPLER",
        "SYM_TEXTURE",
        "SYM_IMAGE",
        "SYM_CONST",
        "SYM_VIRREG", /* virtual register */
        "SYM_TYPE", /* typedef */
        /*"SYM_STRUCT", */         /* struct type */
        /*"SYM_UNIFORMSTRUCT", */  /* each uniform struct has an unique uniform
                                   ** symbol defined for the struct, each field
                                   ** of the uniform struct is uniform itself
                                   */
        "SYM_LABEL",
        "SYM_IOBLOCK"
    };
    gcmASSERT(SymbolKind >= VIR_SYM_UNKNOWN && SymbolKind < VIR_SYMKIND_COUNT);

    return symKindStr[SymbolKind];
}

VIR_Symbol *
VIR_GetSymFromId(
    VIR_SymTable *  SymTable,
    VIR_SymId       SymId)
{
    VIR_Symbol * sym;
    VIR_SymId    unscopedSymId =  VIR_Id_GetIndex(SymId);

    gcmASSERT(VIR_Id_isValid(SymId));
    gcmASSERT(!VIR_Id_isFunctionScope(SymId) || BT_IS_FUNCTION_SCOPE(SymTable));

    gcmASSERT(VIR_SymTable_MaxValidId(SymTable) > unscopedSymId);
    sym = (VIR_Symbol *)VIR_GetEntryFromId(SymTable, unscopedSymId);
    return sym;
}

/* string table operation */
VIR_NameId vscStringTable_Find(
    VIR_StringTable* pStringTbl,
    const char* pStr,
    gctUINT len)
{
    VIR_NameId sid;

    /* search hash table in Block table */
    gcmASSERT(BT_HAS_HASHTABLE(pStringTbl));
    sid = (VIR_NameId)vscBT_HashSearch(pStringTbl, (void *)pStr);
    if (VIR_Id_isInvalid(sid))  /* not found */
    {
        sid = (VIR_NameId)vscBT_AddContinuousEntries(pStringTbl, (void *)pStr, len);
    }

    return sid;
}

#define _add_name(VAR, name_str)  do { VAR = vscStringTable_Find(StrTable, name_str, sizeof(name_str)); } while(0)

static void
_initOpenCLBuiltinNames(VIR_StringTable *StrTable)
{
    /* initialize builtin names */
    _add_name(VIR_NAME_UNKNOWN, "__unknown");

    VIR_NAME_POSITION       =
    VIR_NAME_POINT_SIZE     =
    VIR_NAME_COLOR          =
    VIR_NAME_FRONT_FACING   =
    VIR_NAME_POINT_COORD    =
    VIR_NAME_POSITION_W     =
    VIR_NAME_DEPTH          =
    VIR_NAME_FOG_COORD      =
    VIR_NAME_VERTEX_ID      =
    VIR_NAME_FRONT_COLOR    =
    VIR_NAME_BACK_COLOR     =
    VIR_NAME_FRONT_SECONDARY_COLOR =
    VIR_NAME_BACK_SECONDARY_COLOR  =
    VIR_NAME_TEX_COORD      =
    VIR_NAME_INSTANCE_ID    =
    VIR_NAME_HELPER_INVOCATION  =
    VIR_NAME_SUBSAMPLE_DEPTH    =
    VIR_NAME_PERVERTEX          =
    VIR_NAME_IN                 =
    VIR_NAME_OUT                =
    VIR_NAME_INVOCATION_ID      =
    VIR_NAME_PATCH_VERTICES_IN  =
    VIR_NAME_PRIMITIVE_ID       =
    VIR_NAME_TESS_LEVEL_OUTER   =
    VIR_NAME_TESS_LEVEL_INNER   =
    VIR_NAME_LAYER              = VIR_NAME_UNKNOWN;

    _add_name(VIR_NAME_NUM_GROUPS, "gl_NumWorkGroups");
    _add_name(VIR_NAME_WORKGROUPSIZE, "gl_WorkGroupSize");
    _add_name(VIR_NAME_WORK_GROUP_ID, "gl_WorkGroupID");
    _add_name(VIR_NAME_LOCAL_INVOCATION_ID, "gl_LocalInvocationID");
    _add_name(VIR_NAME_GLOBAL_INVOCATION_ID, "gl_GlobalInvocationID");
    _add_name(VIR_NAME_LOCALINVOCATIONINDEX, "gl_LocalInvocationIndex");

    VIR_NAME_BUILTIN_LAST = VIR_NAME_LOCALINVOCATIONINDEX;
}

static void
_initOpenGLBuiltinNames(VIR_StringTable *StrTable)
{
    /* initialize builtin names */
    _add_name(VIR_NAME_UNKNOWN, "__unknown");
    _add_name(VIR_NAME_POSITION, "gl_Position");
    _add_name(VIR_NAME_POINT_SIZE, "gl_PointSize");
    _add_name(VIR_NAME_COLOR, "gl_Color");
    _add_name(VIR_NAME_FRONT_FACING, "gl_FrontFacing");
    _add_name(VIR_NAME_POINT_COORD, "gl_PointCoord");
    _add_name(VIR_NAME_POSITION_W, "gl_Position.w");
    _add_name(VIR_NAME_DEPTH, "gl_FragDepth");
    _add_name(VIR_NAME_FOG_COORD, "gl_FogFragCoord");
    _add_name(VIR_NAME_VERTEX_ID, "gl_VertexID");
    _add_name(VIR_NAME_FRONT_COLOR, "gl_FrontColor");
    _add_name(VIR_NAME_BACK_COLOR, "gl_BackColor");
    _add_name(VIR_NAME_FRONT_SECONDARY_COLOR, "gl_FrontSecondaryColor");
    _add_name(VIR_NAME_BACK_SECONDARY_COLOR, "gl_BackSecondaryColor");
    _add_name(VIR_NAME_TEX_COORD, "gl_TexCoord");
    _add_name(VIR_NAME_INSTANCE_ID, "gl_InstanceID");
    _add_name(VIR_NAME_NUM_GROUPS, "gl_NumWorkGroups");
    _add_name(VIR_NAME_WORKGROUPSIZE, "gl_WorkGroupSize");
    _add_name(VIR_NAME_WORK_GROUP_ID, "gl_WorkGroupID");
    _add_name(VIR_NAME_LOCAL_INVOCATION_ID, "gl_LocalInvocationID");
    _add_name(VIR_NAME_GLOBAL_INVOCATION_ID, "gl_GlobalInvocationID");
    _add_name(VIR_NAME_LOCALINVOCATIONINDEX, "gl_LocalInvocationIndex");
    _add_name(VIR_NAME_HELPER_INVOCATION, "gl_HelperInvocation");
    _add_name(VIR_NAME_SUBSAMPLE_DEPTH, "#Subsample_Depth");
    _add_name(VIR_NAME_PERVERTEX, "gl_PerVertex");
    _add_name(VIR_NAME_IN, "gl_in");
    _add_name(VIR_NAME_OUT, "gl_out");
    _add_name(VIR_NAME_INVOCATION_ID, "gl_InvocationID");
    _add_name(VIR_NAME_PATCH_VERTICES_IN, "gl_PatchVerticesIn");
    _add_name(VIR_NAME_PRIMITIVE_ID, "gl_PrimitiveID");
    _add_name(VIR_NAME_TESS_LEVEL_OUTER, "gl_TessLevelOuter");
    _add_name(VIR_NAME_TESS_LEVEL_INNER, "gl_TessLevelInner");
    _add_name(VIR_NAME_LAYER, "gl_Layer");
    _add_name(VIR_NAME_PS_OUT_LAYER, "#ps_out_layer");
    _add_name(VIR_NAME_PRIMITIVE_ID_IN, "gl_PrimitiveIDIn");
    _add_name(VIR_NAME_TESS_COORD, "gl_TessCoord");
    _add_name(VIR_NAME_SAMPLE_ID, "gl_SampleID");
    _add_name(VIR_NAME_SAMPLE_POSITION, "gl_SamplePosition");
    _add_name(VIR_NAME_SAMPLE_MASK_IN, "gl_SampleMaskIn");
    _add_name(VIR_NAME_SAMPLE_MASK, "gl_SampleMask");
    _add_name(VIR_NAME_IN_POSITION, "gl_in.gl_Position");
    _add_name(VIR_NAME_IN_POINT_SIZE, "gl_in.gl_PointSize");
    _add_name(VIR_NAME_BOUNDING_BOX, "gl_BoundingBox");
    /* WARNING!!! change builtin_last if add new name !!! */
    VIR_NAME_BUILTIN_LAST = VIR_NAME_BOUNDING_BOX;
}
#undef _add_name

static void
_initBuiltinNames(VIR_Shader * Shader)
{
    if (Shader->shaderKind == VIR_SHADER_CL)
    {
        _initOpenCLBuiltinNames(&Shader->stringTable);
    }
    else
    {
        _initOpenGLBuiltinNames(&Shader->stringTable);
    }
}

static VSC_ErrCode
_VIR_InitStringTable(VIR_Shader * Shader)
{
    VSC_ErrCode errCode   = VSC_ERR_NONE;
    /* initialize block table */
    vscBT_Initialize(&Shader->stringTable,
                     &Shader->mempool,
                     VSC_BLOCK_TABLE_FLAG_HASH_ENTRIES | VSC_BLOCK_TABLE_FLAG_AUTO_HASH,
                     sizeof(char),
                     32*1024, /* 32KB */
                     10,
                     (PFN_VSC_HASH_FUNC)vscHFUNC_String,
                     (PFN_VSC_KEY_CMP)vcsHKCMP_String,
                     1024);

    /* initialize builtin names */
    _initBuiltinNames(Shader);

    return errCode;
}

static VSC_ErrCode
_VIR_InitSymbolTable(VIR_Shader * Shader)
{
    VSC_ErrCode errCode   = VSC_ERR_NONE;
    /* initialize block table */
    vscBT_Initialize(&Shader->symTable,
                     &Shader->mempool,
                     VSC_BLOCK_TABLE_FLAG_HASH_ENTRIES | VSC_BLOCK_TABLE_FLAG_AUTO_HASH,
                     sizeof(VIR_Symbol),
                     32*1024, /* 32KB */
                     10,
                     (PFN_VSC_HASH_FUNC)vscHFUNC_Symbol,
                     (PFN_VSC_KEY_CMP)vcsHKCMP_Symbol,
                     1024);

    return errCode;
}

static VSC_ErrCode
_VIR_InitConstTable(VIR_Shader * Shader)
{
    VSC_ErrCode errCode   = VSC_ERR_NONE;
    /* initialize block table */
    vscBT_Initialize(&Shader->constTable,
                     &Shader->mempool,
                     VSC_BLOCK_TABLE_FLAG_HASH_ENTRIES | VSC_BLOCK_TABLE_FLAG_AUTO_HASH,
                     sizeof(VIR_Const),
                     16*1024, /* 16KB */
                     10,
                     (PFN_VSC_HASH_FUNC)vscHFUNC_Const,
                     (PFN_VSC_KEY_CMP)vcsHKCMP_Const,
                     512);

    return errCode;
}


static VSC_ErrCode
_VIR_Shader_InitTypeTable(VIR_Shader *    Shader)
{
    VSC_ErrCode errCode   = VSC_ERR_NONE;
    gctUINT      i;
    /* initialize block table */
    vscBT_Initialize(&Shader->typeTable,
                     &Shader->mempool,
                     (VSC_BLOCK_TABLE_FLAG_HASH_ENTRIES
                      | VSC_BLOCK_TABLE_FLAG_AUTO_HASH
                      | VSC_BLOCK_TABLE_FLAG_FREE_ENTRY_LIST ),
                     sizeof(VIR_Type),
                     32*1024, /* 32KB */
                     10,
                     (PFN_VSC_HASH_FUNC)vscHFUNC_Type,
                     (PFN_VSC_KEY_CMP)vcsHKCMP_Type,
                     512);

    /* initialize builtin types */
    for (i=0; i < sizeof(VIR_builtinTypes); i++)
    {
        if (VIR_builtinTypes[i].name != gcvNULL)
        {
            VIR_TypeId typeId;
            errCode = VIR_Shader_AddBuiltinType(Shader,
                                      &VIR_builtinTypes[i],
                                      &typeId);
            CHECK_ERROR(errCode, "AddBuiltinType");
        }
        else
        {
            /* last builtin types */
            break;
        }
    }
    return errCode;
}

#if defined(_DEBUG)
VIR_Id VIR_IdList_GetId(VIR_IdList *IdList, gctUINT No)
{
    gcmASSERT(No < VIR_IdList_Count(IdList));
    return No < VIR_IdList_Count(IdList) ? (VIR_Id)(IdList)->ids[(No)] : VIR_INVALID_ID;
}
#endif

VSC_ErrCode
_VIR_Shader_DumperInit(
    IN  VIR_Shader *    Shader,
    IN  gctFILE         File,
    IN  gctUINT         BufferSize
    )
{
    VSC_ErrCode       errCode   = VSC_ERR_NONE;
    gctCHAR *         buffer;

    gcmASSERT(Shader != gcvNULL);

    Shader->dumper = (VIR_Dumper*)vscMM_Alloc(&Shader->mempool,
                                              sizeof(VIR_Dumper));
    if (Shader->dumper == gcvNULL)
    {
        return VSC_ERR_OUT_OF_MEMORY;
    }

    buffer = (gctCHAR *)vscMM_Alloc(&Shader->mempool,
                                    sizeof(gctCHAR) * BufferSize);
    if (buffer == gcvNULL)
    {
        return VSC_ERR_OUT_OF_MEMORY;
    }

    vscDumper_Initialize(&Shader->dumper->baseDumper,
                         gcvNULL,
                         gcvNULL,
                         buffer,
                         BufferSize);
    Shader->dumper->Shader = Shader;

    return errCode;
}

VSC_ErrCode
VIR_Shader_Construct(
    IN gcoHAL         Hal,
    IN VIR_ShaderKind ShaderKind,
    IN VSC_MM *       MemPool,
    OUT VIR_Shader ** Shader
    )
{
    VSC_ErrCode errCode   = VSC_ERR_NONE;
    VIR_Shader * shader;
    VIR_IdList * idList;

    /* allocate Shader object from memory pool */
    shader = (VIR_Shader *)vscMM_Alloc(MemPool, sizeof(VIR_Shader));
    if (shader == gcvNULL)
    {
        return VSC_ERR_OUT_OF_MEMORY;
    }
    /* get context from Hal */

    /* set default vaules */
    memset(shader, 0, sizeof(VIR_Shader));
    shader->object.type                 = gcvOBJ_VIR_SHADER;
    shader->mempool                     = *MemPool;
    shader->shaderKind                  = ShaderKind;
    shader->defaultUniformBlockIndex    = -1;
    shader->constUniformBlockIndex      = -1;
    shader->samplerBaseOffset           = -1;
    shader->baseSamplerId               = VIR_INVALID_ID;

    /* init dumper */
    errCode = _VIR_Shader_DumperInit(shader, VSC_GET_DUMPER_FILE(), 2048);
    CHECK_ERROR(errCode, "Shader_DumperInit");

    /* initialize tables */
    errCode = _VIR_InitStringTable(shader);
    CHECK_ERROR(errCode, "InitStringTable");

    /* initialize symbol table */
    errCode = _VIR_InitSymbolTable(shader);
    CHECK_ERROR(errCode, "_VIR_InitSymbolTable");

    /* initialize type table */
    errCode = _VIR_Shader_InitTypeTable(shader);
    CHECK_ERROR(errCode, "InitTypeTable");

    /* initialize const table */
    errCode = _VIR_InitConstTable(shader);
    CHECK_ERROR(errCode, "InitConstTable");

    /* init id lists */
    idList = &shader->attributes;
    errCode = VIR_IdList_Init(MemPool, 16, &idList);
    CHECK_ERROR(errCode, "InitIdList");

    idList = &shader->outputs;
    errCode = VIR_IdList_Init(MemPool, 4, &idList);
    CHECK_ERROR(errCode, "InitIdList");

    idList = &shader->perpatchInput;
    errCode = VIR_IdList_Init(MemPool, 8, &idList);
    CHECK_ERROR(errCode, "InitIdList");

    idList = &shader->perpatchOutput;
    errCode = VIR_IdList_Init(MemPool, 4, &idList);
    CHECK_ERROR(errCode, "InitIdList");

    idList = &shader->outputVregs;
    errCode = VIR_IdList_Init(MemPool, 4, &idList);
    CHECK_ERROR(errCode, "InitIdList");

    idList = &shader->perpatchOutputVregs;
    errCode = VIR_IdList_Init(MemPool, 4, &idList);
    CHECK_ERROR(errCode, "InitIdList");

    idList = &shader->uniforms;
    errCode = VIR_IdList_Init(MemPool, 32, &idList);
    CHECK_ERROR(errCode, "InitIdList");

    idList = &shader->variables;
    errCode = VIR_IdList_Init(MemPool, 64, &idList);
    CHECK_ERROR(errCode, "InitIdList");

    idList = &shader->uniformBlocks;
    errCode = VIR_IdList_Init(MemPool, 32, &idList);
    CHECK_ERROR(errCode, "InitIdList");

    idList = &shader->ioBlocks;
    errCode = VIR_IdList_Init(MemPool, 32, &idList);
    CHECK_ERROR(errCode, "InitIdList");

    vscBILST_Initialize(&shader->functions, gcvFALSE);

    vscBILST_Initialize(&shader->kernelFunctions, gcvFALSE);

    /* init transform feedback */

    *Shader = shader;
    return errCode;
}

VSC_ErrCode
VIR_Shader_Destroy(
    IN VIR_Shader * Shader
    )
{
    VSC_ErrCode errCode   = VSC_ERR_NONE;
    return errCode;
}

gctBOOL
VIR_Shader_IsESCompiler(
    IN VIR_Shader * Shader
    )
{
    gctBOOL isESCompiler;

    isESCompiler = (Shader->compilerVersion[0] & 0xFFFF) == _SHADER_GL_LANGUAGE_TYPE;

    return isESCompiler;
}

gctBOOL
VIR_Shader_IsES11Compiler(
    IN VIR_Shader * Shader
    )
{
    gctBOOL isES11Compiler;

    isES11Compiler = ((Shader->compilerVersion[0] & 0xFFFF) == _SHADER_GL_LANGUAGE_TYPE &&
                    Shader->compilerVersion[1] == _SHADER_ES11_VERSION);

    return isES11Compiler;
}

gctBOOL
VIR_Shader_IsES30Compiler(
    IN VIR_Shader * Shader
    )
{
    gctBOOL isES30Compiler;

    isES30Compiler = ((Shader->compilerVersion[0] & 0xFFFF) == _SHADER_GL_LANGUAGE_TYPE &&
                    Shader->compilerVersion[1] == _SHADER_HALTI_VERSION);

    return isES30Compiler;
}

gctBOOL
VIR_Shader_IsES31Compiler(
    IN VIR_Shader * Shader
    )
{
    gctBOOL isES31Compiler;

    isES31Compiler = ((Shader->compilerVersion[0] & 0xFFFF) == _SHADER_GL_LANGUAGE_TYPE &&
                    Shader->compilerVersion[1] == _SHADER_ES31_VERSION);

    return isES31Compiler;
}

VSC_ErrCode
VIR_Shader_AddFunction(
    IN  VIR_Shader *    Shader,
    IN  gctBOOL         IsKernel,
    IN  gctSTRING       Name,
    IN  VIR_TypeId      TypeId,
    OUT VIR_Function ** Function
    )
{
    VSC_ErrCode     errCode   = VSC_ERR_NONE;
    VIR_IdList *    idList;
    VSC_MM *        memPool   = &Shader->mempool;
    VIR_Function *  func;
    VIR_SymId       funcSymId;
    VIR_Symbol *    funcSym;
    VIR_FunctionNode *funcNode;

    /* creat function */
    /* do we need a separate mempool for function? */

    errCode = VIR_Shader_AddSymbolWithName(Shader,
                                           VIR_SYM_FUNCTION,
                                           Name,
                                           VIR_Shader_GetTypeFromId(Shader, TypeId),
                                           VIR_STORAGE_UNKNOWN,
                                           &funcSymId);
    if (errCode != VSC_ERR_NONE)
    {
        return errCode;
    }
    funcSym = VIR_Shader_GetSymFromId(Shader, funcSymId);

    /* allocate Shader object from memory pool */
    func = (VIR_Function *)vscMM_Alloc(memPool, sizeof(VIR_Function));
    if (func == gcvNULL)
    {
        return VSC_ERR_OUT_OF_MEMORY;
    }

    /* set default vaules */
    memset(func, 0, sizeof(VIR_Function));

    funcSym->u2.function    = func;
    func->hostShader        = Shader;
    func->funcSym           = funcSymId;
    func->pFuncBlock        = gcvNULL;

    /* initialize symbol table */
    vscBT_Initialize(&func->symTable,
                     memPool,
                     (VSC_BLOCK_TABLE_FLAG_HASH_ENTRIES
                     | VSC_BLOCK_TABLE_FLAG_AUTO_HASH
                     | VSC_BLOCK_TABLE_FLAG_FUNCTION_SCOPE),
                     sizeof(VIR_Symbol),
                     4*1024, /* 4KB */
                     10,
                     (PFN_VSC_HASH_FUNC)vscHFUNC_Symbol,
                     (PFN_VSC_KEY_CMP)vcsHKCMP_Symbol,
                     512);

    /* initialize label table */
    vscBT_Initialize(&func->labelTable,
                     memPool,
                     (VSC_BLOCK_TABLE_FLAG_FREE_ENTRY_LIST
                     | VSC_BLOCK_TABLE_FLAG_HASH_ENTRIES
                     | VSC_BLOCK_TABLE_FLAG_AUTO_HASH),
                     sizeof(VIR_Label),
                     1024, /* 1KB */
                     10,
                     (PFN_VSC_HASH_FUNC)vscHFUNC_Label,
                     (PFN_VSC_KEY_CMP)vcsHKCMP_Label,
                     64);

    /* initialize operands table */
    vscBT_Initialize(&func->operandTable,
                     memPool,
                     VSC_BLOCK_TABLE_FLAG_FREE_ENTRY_LIST,
                     sizeof(VIR_Operand),
                     16*1024, /* 16KB */
                     10,
                     gcvNULL,
                     gcvNULL,
                     0);

    /* init id lists */
    idList = &func->localVariables;
    errCode = VIR_IdList_Init(memPool, 16, &idList);
    CHECK_ERROR(errCode, "InitIdList");

    idList = &func->paramters;
    errCode = VIR_IdList_Init(memPool, 6, &idList);
    CHECK_ERROR(errCode, "InitIdList");

    idList = &func->temps;
    errCode = VIR_IdList_Init(memPool, 128, &idList);
    CHECK_ERROR(errCode, "InitIdList");

    /* init transform feedback */

    /* add function to functionList */
    funcNode = (VIR_FunctionNode *)vscMM_Alloc(memPool, sizeof(VIR_FunctionNode));
    gcmASSERT(funcNode != gcvNULL);
    funcNode->function = func;
    vscBILST_Append(&Shader->functions, (VSC_BI_LIST_NODE*)funcNode);

    if (IsKernel)
    {
        funcNode = vscMM_Alloc(memPool, sizeof(VIR_FunctionNode));
        gcmASSERT(funcNode != gcvNULL);
        funcNode->function = func;
        vscBILST_Append(&Shader->kernelFunctions, (VSC_BI_LIST_NODE*)funcNode);

        func->flags |= VIR_FUNCFLAG_KERNEL;
    }

    if (strcmp("main", Name) == 0)
    {
        /* is main function */
        Shader->mainFunction = func;
        func->flags |= VIR_FUNCFLAG_MAIN;
    }
    *Function = func;
    return errCode;
}

VSC_ErrCode
VIR_Shader_RemoveFunction(
    IN  VIR_Shader *    Shader,
    IN  VIR_Function *  Function
    )
{
    VSC_ErrCode     errCode   = VSC_ERR_NONE;
    VIR_FunctionNode *funcNode;
    VIR_FuncIterator   iter;

    VIR_FuncIterator_Init(&iter, &Shader->functions);
    funcNode = VIR_FuncIterator_First(&iter);
    for (; funcNode != gcvNULL; funcNode = VIR_FuncIterator_Next(&iter))
    {
        if (funcNode->function == Function)
        {
            vscBILST_Remove(&Shader->functions, (VSC_BI_LIST_NODE*)funcNode);
            break;
        }
    }

    /* TODO */
    return errCode;
}

gceSTATUS
VIR_Shader_GetFunctionByName(
    IN  VIR_Shader *    Shader,
    IN  gctCONST_STRING FunctionName,
    OUT VIR_Function **    Function
    )
{
    gceSTATUS status = gcvSTATUS_NAME_NOT_FOUND;
    VIR_FuncIterator func_iter;
    VIR_FunctionNode* func_node;

    VIR_FuncIterator_Init(&func_iter, VIR_Shader_GetFunctions(Shader));
    for (func_node = VIR_FuncIterator_First(&func_iter); func_node != gcvNULL; func_node = VIR_FuncIterator_Next(&func_iter))
    {
        VIR_Function* func = func_node->function;
        if(strcmp(VIR_Function_GetNameString(func), FunctionName) == 0)
        {
            *Function = func;
            status = gcvSTATUS_OK;
            break;
        }
    }
    return status;
}

gceSTATUS
VIR_Shader_CopyFunction(
    IN OUT  VIR_Shader *    ToShader,
    IN VIR_Shader *         FromShader,
    IN gctSTRING            FunctionName,
    OUT VIR_Function **     NewFunction
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    VIR_Function* src_func;
    VIR_Function* new_func = gcvNULL;

    VIR_Shader_GetFunctionByName(FromShader, FunctionName, &src_func);
    VIR_Shader_AddFunction(ToShader, VIR_Function_GetFlags(src_func) & VIR_FUNCFLAG_KERNEL, FunctionName, VIR_Type_GetBaseTypeId(VIR_Function_GetType(src_func)), &new_func);

    return status;
}


VSC_ErrCode
VIR_Shader_AddString(
    IN  VIR_Shader *    Shader,
    IN  gctSTRING       String,
    OUT VIR_NameId *    Name
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    VIR_NameId  id = vscStringTable_Find(&Shader->stringTable,
                                         String, strlen(String) + 1);

    if (VIR_Id_isInvalid(id))
    {
        errCode = VSC_ERR_OUT_OF_MEMORY;
    }
    else
    {
        *Name = id;
    }
    return errCode;
}

VSC_ErrCode
VIR_Shader_AddBuiltinType(
    IN  VIR_Shader *          Shader,
    IN  VIR_BuiltinTypeInfo * TypeInfo,
    OUT VIR_TypeId*           TypeId
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    VIR_TypeId  tyId;
    VIR_Type    type;
    VIR_Type *  newType;

    type._base          = TypeInfo->type;
    type._flags         = (gctUINT)(TypeInfo->flag | VIR_TYFLAG_BUILTIN);
    type._kind          = (gctUINT)TypeInfo->kind;
    type._alignment     = (gctUINT)TypeInfo->alignment;
    type._addrSpace     = (gctUINT)VIR_AS_PRIVATE;
    type._qualifier     = (gctUINT)VIR_TYQUAL_NONE;
    type.u1.nameId      = vscStringTable_Find(&Shader->stringTable,
                                              TypeInfo->name,
                                              strlen(TypeInfo->name)+1);
    type.u1.symId       = VIR_INVALID_ID;
    type.u2.size        = TypeInfo->sz;
    tyId                = vscBT_AddEntry(&Shader->typeTable, &type);
    *TypeId             = tyId;
    newType = VIR_Shader_GetTypeFromId(Shader, tyId);
    newType->_tyIndex   = tyId;
    if (tyId != TypeInfo->type)
    {
        errCode = VSC_ERR_OUT_OF_MEMORY;
        gcmASSERT(0);
    }

    return errCode;
}

VSC_ErrCode
VIR_Shader_AddArrayType(
    IN  VIR_Shader *    Shader,
    IN  VIR_TypeId      BaseTypeId,
    IN  gctUINT32       ArrayLength,
    OUT VIR_TypeId *    TypeId
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    VIR_TypeId  tyId;
    VIR_Type    type;
    VIR_Type *  baseType = VIR_Shader_GetTypeFromId(Shader, BaseTypeId);
    VIR_Type *aggregateType;

    type._base          = BaseTypeId;
    type._flags         = (gctUINT)VIR_TYFLAG_SIZED;
    type._kind          = (gctUINT)VIR_TY_ARRAY;
    type._alignment     = baseType->_alignment;
    type._addrSpace     = (gctUINT)VIR_AS_PRIVATE;
    type._qualifier     = (gctUINT)VIR_TYQUAL_NONE;
    type.u1.nameId      = VIR_NAME_UNKNOWN;
    type.u1.symId       = VIR_INVALID_ID;
    type.u2.arrayLength = ArrayLength;
    tyId                = vscBT_Find(&Shader->typeTable, &type);
    aggregateType = VIR_Shader_GetTypeFromId(Shader, tyId);
    aggregateType->_tyIndex = tyId;
    *TypeId             = tyId;

    return errCode;
}

VSC_ErrCode
VIR_Shader_AddPointerType(
    IN  VIR_Shader *    Shader,
    IN  VIR_TypeId      BaseTypeId,
    IN  VIR_TyQualifier Qualifier,
    IN  VIR_AddrSpace   AS,
    OUT VIR_TypeId *    TypeId
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    VIR_TypeId  tyId;
    VIR_Type    type;
    VIR_Type *  ptrType;
    VIR_Type *  baseType = VIR_Shader_GetTypeFromId(Shader, BaseTypeId);

    type._base           = BaseTypeId;
    type._flags          = (gctUINT)VIR_TYFLAG_SIZED;
    type._kind           = (gctUINT)VIR_TY_POINTER;
    type._alignment      = baseType->_alignment;
    type._addrSpace      = (gctUINT)AS;
    type._qualifier      = (gctUINT)Qualifier;
    type.u1.nameId      = VIR_NAME_UNKNOWN;
    type.u1.symId       = VIR_INVALID_ID;
    type.u2.size        = POINTER_SIZE;
    tyId                = vscBT_Find(&Shader->typeTable, &type);
    ptrType = VIR_Shader_GetTypeFromId(Shader, tyId);
    ptrType->_tyIndex = tyId;
    *TypeId             = tyId;

    return errCode;
}

VSC_ErrCode
VIR_Shader_AddFunctionType(
    IN  VIR_Shader *    Shader,
    IN  VIR_TypeId      ReturnType,
    IN  VIR_TypeIdList *Params,
    OUT VIR_TypeId *    TypeId
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    VIR_TypeId  tyId;
    VIR_Type    type;
    VIR_Type *  funcType;

    type._base          = ReturnType;
    type._flags         = (gctUINT)VIR_TYFLAG_NONE;
    type._kind          = (gctUINT)VIR_TY_FUNCTION;
    type._alignment     = 0; /* no alignment for function type */
    type._addrSpace     = (gctUINT)VIR_AS_PRIVATE;
    type._qualifier     = (gctUINT)VIR_TYQUAL_NONE;
    type.u1.nameId      = VIR_NAME_UNKNOWN;
    type.u1.symId       = VIR_INVALID_ID;
    type.u2.params      = Params;
    tyId                = vscBT_Find(&Shader->typeTable, &type);
    *TypeId             = tyId;
    funcType = VIR_Shader_GetTypeFromId(Shader, tyId);
    funcType->_tyIndex = tyId;

    return errCode;
}

VSC_ErrCode
VIR_Shader_AddStructType(
    IN  VIR_Shader *    Shader,
    IN  gctBOOL         IsUnion,
    IN  VIR_NameId      NameId,
    OUT VIR_TypeId *    TypeId
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    VIR_TypeId  tyId;
    VIR_Type    type;
    VIR_Type *  structType;

    type._base          = VIR_TYPE_UNKNOWN;
    type._flags         = IsUnion ? (gctUINT)VIR_TYFLAG_ISUNION
                                  : (gctUINT)VIR_TYFLAG_NONE;
    type._kind          = (gctUINT)VIR_TY_STRUCT;
    type._alignment     = 0; /* no alignment for function type */
    type._addrSpace     = (gctUINT)VIR_AS_PRIVATE;
    type._qualifier     = (gctUINT)VIR_TYQUAL_NONE;
    type.u1.nameId      = NameId;
    type.u1.symId       = VIR_INVALID_ID; /* need to set it with struct symbol id */
    type.u2.fields      = gcvNULL;
    tyId                = vscBT_Find(&Shader->typeTable, &type);
    *TypeId             = tyId;
    structType = VIR_Shader_GetTypeFromId(Shader, tyId);
    structType->_tyIndex = tyId;

    return errCode;
}

VSC_ErrCode
VIR_Shader_AddConstant(
    IN  VIR_Shader *      Shader,
    IN  VIR_TypeId        Type,
    IN  VIR_ConstVal *    Value,
    OUT VIR_ConstId *     ConstId
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    VIR_Const   cst;
    VIR_Const*  newCst;
    VIR_ConstId id;

    cst.type    = Type;
    cst.value   = *Value;

    id = vscBT_Find(&Shader->constTable, &cst);
    if (VIR_Id_isInvalid(id))
    {
        errCode = VSC_ERR_OUT_OF_MEMORY;
    }
    else
    {
        newCst = (VIR_Const*)VIR_GetSymFromId(&Shader->constTable, id);
        newCst->index = id;

        *ConstId = id;
    }

    return errCode;
}

gctBOOL
VIR_Shader_FindUniformByConstantValue(
    IN  VIR_Shader *      Shader,
    IN  VIR_Const *       Constant,
    OUT VIR_Uniform **    Uniform,
    OUT VIR_Swizzle *     Swizzle
    )
{
    gctINT          i, j, k, index = 0, count;
    VIR_Uniform *   uniform = gcvNULL;
    gctUINT         chnlSwizzle[4];

    gcmASSERT(Constant->type < VIR_TYPE_LAST_PRIMITIVETYPE);
    count = VIR_GetTypeComponents(Constant->type);
    gcmASSERT(VIR_GetTypeRows(Constant->type) == 1); /* not support matrix yet */

    for (i = 0; i < (gctINT) VIR_IdList_Count(&Shader->uniforms); ++i)
    {
        VIR_Id      id  = VIR_IdList_GetId(&Shader->uniforms, i);
        VIR_Symbol *sym = VIR_Shader_GetSymFromId(Shader, id);
        uniform = VIR_Symbol_GetUniform(sym);

        if (uniform == gcvNULL)
        {
            continue;
        }

        if (isSymUniformCompiletimeInitialized(sym))
        {
            VIR_Const *constVal = (VIR_Const *) VIR_GetSymFromId(&Shader->constTable, VIR_Uniform_GetInitializer(uniform));
            gctINT constCount = VIR_GetTypeComponents(constVal->type);

            gcmASSERT(constCount > 0);

            if (constCount < count)
            {
                continue;
            }

            for (j = 0; j < count; ++ j)
            {
                for (index = 0; index < constCount; ++ index)
                {
                    if (Constant->value.vecVal.u32Value[j] == constVal->value.vecVal.u32Value[index])
                    {
                        for (k = j; k < 4; k ++)
                        {
                            chnlSwizzle[k] = ((VIR_Swizzle)index << (k * 2));
                        }

                        break;
                    }
                }

                /* Not found */
                if (index == constCount)
                {
                    break;
                }
            }

            /* Found */
            if (j == count)
            {
                break;
            }
        }
    }

    *Uniform = uniform;
    *Swizzle = (chnlSwizzle[0] | chnlSwizzle[1] | chnlSwizzle[2] | chnlSwizzle[3]);

    return ((i != (gctINT) VIR_IdList_Count(&Shader->uniforms)) && uniform);
}

/* give a constant, add a new uniform to the shader initialized with the
 * constant if there is no uniform initialized with the constant value,
 * possible with some swizzle */
VSC_ErrCode
VIR_Shader_AddInitializedUniform(
    IN  VIR_Shader *      Shader,
    IN  VIR_Const *       Constant,
    OUT VIR_Uniform **    Uniform,
    OUT VIR_Swizzle *     Swizzle
    )
{
    VSC_ErrCode  errCode = VSC_ERR_NONE;
    VIR_Uniform *uniform;
    gctCHAR      name[64];
    gctUINT      offset   = 0, cstId;
    VIR_SymId    symId;
    VIR_Symbol * uniformSym;
    VIR_Swizzle  swizzle = VIR_SWIZZLE_XYZW;

    if (VIR_Shader_FindUniformByConstantValue(Shader,
                                              Constant,
                                              &uniform,
                                              &swizzle))
    {
        /* found */
        *Uniform = uniform;
        if (Swizzle) *Swizzle = swizzle;

        return errCode;
    }

    /* not found, add a new uniform to shader */

    /* construct const vector name, create and initialize constant uniform */
    gcoOS_PrintStrSafe(name, sizeof(name), &offset, "#sh%d_const_%d",
                       Shader->_id, Shader->_constVectorId++);

    errCode = VIR_Shader_AddSymbolWithName(Shader,
                                           VIR_SYM_UNIFORM,
                                           name,
                                           VIR_Shader_GetTypeFromId(Shader, Constant->type),
                                           VIR_STORAGE_UNKNOWN,
                                           &symId);
    if (errCode != VSC_ERR_NONE)
    {
        return errCode;
    }
    uniformSym = VIR_Shader_GetSymFromId(Shader, symId);
    gcmASSERT(VIR_Symbol_isUniform(uniformSym));

    uniform = VIR_Symbol_GetUniform(uniformSym);
    VIR_Shader_AddConstant(Shader, Constant->type, &Constant->value, &cstId);
    uniform->u.initializer = cstId;
    VIR_Symbol_SetLocation(uniformSym, -1);
    VIR_Symbol_SetFlag(uniformSym, VIR_SYMUNIFORMFLAG_COMPILETIME_INITIALIZED);
    VIR_Symbol_SetFlag(uniformSym, VIR_SYMFLAG_COMPILER_GEN);
    VIR_Symbol_SetFlag(uniformSym, VIR_SYMUNIFORMFLAG_VIR_NEW_ADDED);

    switch (VIR_GetTypeComponents(Constant->type))
    {
        case 32:
        case 16:
        case 8:
        case 4:
            swizzle  = VIR_SWIZZLE_XYZW;
            break;
        case 3:
            swizzle  = VIR_SWIZZLE_XYZZ;
            break;
        case 2:
            swizzle  = VIR_SWIZZLE_XYYY;
            break;
        case 1:
            swizzle  = VIR_SWIZZLE_XXXX;
            break;
        default:
            gcmASSERT(gcvFALSE);
    }

    *Uniform = uniform;
    if (Swizzle) *Swizzle = swizzle;

    return errCode;
}

/* give a constant vector, add a new uniform to the shader initialized with the
 * constant
 * to-do check whether constant vector exists or not */
VSC_ErrCode
VIR_Shader_AddInitializedConstUniform(
    IN  VIR_Shader *      Shader,
    IN  VIR_Const *       Constant,
    OUT VIR_Uniform **    Uniform
    )
{
    VSC_ErrCode  errCode = VSC_ERR_NONE;
    VIR_Uniform *uniform;
    gctCHAR      name[64];
    gctUINT      offset   = 0;
    VIR_SymId    symId;
    VIR_Symbol * uniformSym;

    /* construct const vector name, create and initialize constant uniform */
    gcoOS_PrintStrSafe(name, sizeof(name), &offset, "#sh%d_const_%d",
                       Shader->_id, Shader->_constVectorId++);

    errCode = VIR_Shader_AddSymbolWithName(Shader,
                                           VIR_SYM_UNIFORM,
                                           name,
                                           VIR_Shader_GetTypeFromId(Shader, Constant->type),
                                           VIR_STORAGE_UNKNOWN,
                                           &symId);
    if (errCode != VSC_ERR_NONE)
    {
        return errCode;
    }
    uniformSym = VIR_Shader_GetSymFromId(Shader, symId);
    gcmASSERT(VIR_Symbol_isUniform(uniformSym));

    uniform = VIR_Symbol_GetUniform(uniformSym);
    uniform->u.initializer = Constant->index;
    VIR_Symbol_SetLocation(uniformSym, -1);
    VIR_Symbol_SetFlag(uniformSym, VIR_SYMUNIFORMFLAG_COMPILETIME_INITIALIZED);
    VIR_Symbol_SetFlag(uniformSym, VIR_SYMFLAG_COMPILER_GEN);
    VIR_Symbol_SetFlag(uniformSym, VIR_SYMUNIFORMFLAG_VIR_NEW_ADDED);

    *Uniform = uniform;

    return errCode;
}

gctUINT
VIR_Shader_GetTypeByteSize(
    IN  VIR_Shader *    Shader,
    IN  VIR_Type *      Type
    )
{
    gctUINT size = 0;
    if (VIR_Type_isPrimitive(Type))
    {
        size = VIR_GetTypeSize(VIR_Type_GetIndex(Type));
    }
    else
    {
        switch (VIR_Type_GetKind(Type))
        {
            case VIR_TY_ARRAY:
            {
                VIR_TypeId base_type_id = VIR_Type_GetBaseTypeId(Type);
                VIR_Type * base_type = VIR_Shader_GetTypeFromId(Shader, base_type_id);
                size = VIR_Type_GetArrayLength(Type) * VIR_Shader_GetTypeByteSize(Shader, base_type);
                break;
            }
            case VIR_TY_STRUCT:
            {
                VIR_SymIdList* fields = VIR_Type_GetFields(Type);
                if(fields)
                {
                    gctUINT i;
                    for(i = 0; i < VIR_IdList_Count(fields); i++)
                    {
                        VIR_SymId field_id = VIR_IdList_GetId(fields, i);
                        VIR_Symbol* field_sym = VIR_Shader_GetSymFromId(Shader, field_id);
                        VIR_Type* field_type = VIR_Symbol_GetType(field_sym);
                        size += VIR_Shader_GetTypeByteSize(Shader, field_type);
                    }
                }
                else
                {
                    /* this struct is only symbolic and unused. in the future when VIR is
                       fully functional, this condition will be illegal */
                    size = 0;
                }
                break;
            }
            default:
                gcmASSERT(0);
        }
    }

    return size;
}

gctUINT
VIR_Shader_GetTypeLocationRange(
    IN  VIR_Shader *    Shader,
    IN  VIR_Type *      Type
    )
{
    gctUINT range = 0;
    if (VIR_Type_isPrimitive(Type))
    {
        range = 1;
    }
    else
    {
        switch (VIR_Type_GetKind(Type))
        {
            case VIR_TY_ARRAY:
            {
                VIR_TypeId base_type_id = VIR_Type_GetBaseTypeId(Type);
                VIR_Type * base_type = VIR_Shader_GetTypeFromId(Shader, base_type_id);
                range = VIR_Type_GetArrayLength(Type) * VIR_Shader_GetTypeLocationRange(Shader, base_type);
                break;
            }
            default:
                gcmASSERT(0);
        }
    }

    return range;
}

VIR_Symbol*
VIR_Shader_AddBuiltinAttribute(
    IN  VIR_Shader *    VirShader,
    IN  VIR_TypeId      TypeId,
    IN  gctBOOL         isPerpatch,
    IN  VIR_NameId      builtinName
    )
{
    VIR_Symbol *        sym = gcvNULL;
    VIR_SymId           symId;
    VSC_ErrCode         virErrCode;
    VIR_Type *          type;

    type = VIR_Shader_GetTypeFromId(VirShader, TypeId);

    virErrCode = VIR_Shader_AddSymbol(VirShader,
                                        VIR_SYM_VARIABLE,
                                        builtinName,
                                        type,
                                        isPerpatch ? VIR_STORAGE_PERPATCH_INPUT
                                                   : VIR_STORAGE_INPUT,
                                        &symId);
    if(virErrCode == VSC_ERR_NONE)
    {
        sym = VIR_Shader_GetSymFromId(VirShader, symId);
        VIR_Symbol_SetPrecision(sym, VIR_PRECISION_DEFAULT);
        VIR_Symbol_SetTyQualifier(sym, VIR_TYQUAL_CONST);
        VIR_Symbol_SetFlag(sym, VIR_SYMFLAG_ENABLED);

        if (VirShader->shaderKind == VIR_SHADER_FRAGMENT)
        {
            if (builtinName == VIR_NAME_LAYER ||
                builtinName == VIR_NAME_PRIMITIVE_ID)
            {
                VIR_Symbol_SetFlag(sym, VIR_SYMFLAG_FLAT);
            }
        }

        /* set layout info */
        VIR_Symbol_SetLayoutQualifier(sym, VIR_LAYQUAL_NONE);
        VIR_Symbol_SetFirstSlot(sym, NOT_ASSIGNED);
        VIR_Symbol_SetArraySlot(sym, NOT_ASSIGNED);
        VIR_Symbol_SetHwFirstCompIndex(sym, NOT_ASSIGNED);
        /* Caller To Do: VIR_Symbol_SetLocation */
    }

    return sym;
}

VIR_Symbol*
VIR_Shader_AddBuiltinOutput(
    IN  VIR_Shader *    VirShader,
    IN  VIR_TypeId      TypeId,
    IN  gctBOOL         isPerpatch,
    IN  VIR_NameId      builtinName
    )
{
    VIR_Symbol *     sym = gcvNULL;
    VIR_SymId        symId;
    VSC_ErrCode      virErrCode;
    VIR_Type *       type;

    type = VIR_Shader_GetTypeFromId(VirShader, TypeId);

    virErrCode = VIR_Shader_AddSymbol(VirShader,
                                        VIR_SYM_VARIABLE,
                                        builtinName,
                                        type,
                                        isPerpatch ? VIR_STORAGE_PERPATCH_OUTPUT
                                                   : VIR_STORAGE_OUTPUT,
                                        &symId);
    if(virErrCode == VSC_ERR_NONE)
    {
        sym = VIR_Shader_GetSymFromId(VirShader, symId);
        VIR_Symbol_SetPrecision(sym, VIR_PRECISION_DEFAULT);
        VIR_Symbol_SetTyQualifier(sym, VIR_TYQUAL_CONST);
        VIR_Symbol_SetFlag(sym, VIR_SYMFLAG_ENABLED);

        /* set layout info */
        VIR_Symbol_SetLayoutQualifier(sym, VIR_LAYQUAL_NONE);
        VIR_Symbol_SetFirstSlot(sym, NOT_ASSIGNED);
        VIR_Symbol_SetArraySlot(sym, NOT_ASSIGNED);
        VIR_Symbol_SetHwFirstCompIndex(sym, NOT_ASSIGNED);
        /* Caller To Do: VIR_Symbol_SetLocation */
    }

    return sym;
}

/* types */
void
VIR_Type_SetAlignment(
    IN OUT VIR_Type *   Type,
    IN  gctUINT         Alignment
    )
{
    gcmASSERT(Alignment == 1 ||
              Alignment == 2 ||
              Alignment == 4 ||
              Alignment == 8 ||
              Alignment == 16 ||
              Alignment == 32 ||
              Alignment == 64 ||
              Alignment == 128);
    switch (Alignment)
    {
    case 1:
        Type->_alignment = 0;
        break;
    case 2:
        Type->_alignment = 1;
        break;
    case 4:
        Type->_alignment = 2;
        break;
    case 8:
        Type->_alignment = 3;
        break;
    case 16:
        Type->_alignment = 4;
        break;
    case 32:
        Type->_alignment = 5;
        break;
    case 64:
        Type->_alignment = 6;
        break;
    case 128:
        Type->_alignment = 7;
        break;
    default:
        gcmASSERT(0);
        break;
    }
}

void
VIR_Type_AddQualifier(
    IN OUT VIR_Type *   Type,
    IN  VIR_TyQualifier Qualifier
    )
{
    Type->_qualifier = Qualifier;
}

void
VIR_Type_AddAddrSpace(
    IN OUT VIR_Type *   Type,
    IN  VIR_AddrSpace   AS
    )
{
    gcmASSERT(VIR_Type_GetKind(Type) == VIR_TY_POINTER);
    Type->_addrSpace = AS;
}


VSC_ErrCode
VIR_Type_AddField(
    IN  VIR_Shader *    Shader,
    IN OUT VIR_Type *   Type,
    IN VIR_SymId        Field
    )
{
    VSC_ErrCode errCode  = VSC_ERR_NONE;
    VIR_Symbol *    fieldSym;
    VIR_FieldInfo * fieldInfo;

    gcmASSERT(VIR_Type_GetKind(Type) == VIR_TY_STRUCT);
    if (Type->u2.fields == gcvNULL)
    {
        /* create a field list */
        errCode = VIR_IdList_Init(&Shader->mempool,
                                 8, /* init field count */
                                 &Type->u2.fields);
        CHECK_ERROR(errCode, "AddField");
    }

    fieldSym = VIR_Shader_GetSymFromId(Shader, Field);
    /* the field type should not be the same as struct type */
    gcmASSERT(VIR_Symbol_GetType(fieldSym) != Type);
    errCode = VIR_IdList_Add(Type->u2.fields, Field);
    CHECK_ERROR(errCode, "AddField");

    /* allocate Shader object from memory pool */
    fieldInfo = (VIR_FieldInfo *)vscMM_Alloc(&Shader->mempool,
                                             sizeof(VIR_FieldInfo));
    if (fieldInfo == gcvNULL)
    {
        return VSC_ERR_OUT_OF_MEMORY;
    }

    /* set default vaules */
    memset(fieldInfo, 0, sizeof(VIR_FieldInfo));

    fieldSym->u2.fieldInfo  = fieldInfo;
    return errCode;
}

VSC_ErrCode
VIR_Type_AddFieldAndInfo(
    IN  VIR_Shader *    Shader,
    IN  VIR_Type *      StructType,
    IN  gctSTRING       FieldName,
    IN  gctUINT32       Offset,
    IN  gctBOOL         isBitField,
    IN  gctUINT         StartBit,
    IN  gctUINT         BitSize,
    IN  gctUINT         TempRegOrUniformOffset,
    OUT VIR_SymId *     FieldSymId
    )
{
    VSC_ErrCode     errCode  = VSC_ERR_NONE;
    VIR_Symbol *    fieldSym;
    VIR_FieldInfo * fieldInfo;

    gcmASSERT(VIR_Type_GetKind(StructType) == VIR_TY_STRUCT);
    /* add field symbol */
    errCode = VIR_Shader_AddSymbolWithName(Shader,
                                  VIR_SYM_FIELD,
                                  FieldName,
                                  StructType,
                                  VIR_STORAGE_UNKNOWN,
                                  FieldSymId);
    if (errCode != VSC_ERR_NONE)
    {
        return errCode;
    }
    fieldSym = VIR_Shader_GetSymFromId(Shader, *FieldSymId);
     /* the field type should not be the same as struct type */
    gcmASSERT(VIR_Symbol_GetType(fieldSym) != StructType);

    /* allocate Shader object from memory pool */
    fieldInfo = (VIR_FieldInfo *)vscMM_Alloc(&Shader->mempool,
                                             sizeof(VIR_FieldInfo));
    if (fieldInfo == gcvNULL)
    {
        return VSC_ERR_OUT_OF_MEMORY;
    }

    /* set default vaules */
    fieldInfo->bitSize      = BitSize;
    fieldInfo->startBit     = StartBit;
    fieldInfo->isBitfield   = isBitField;
    fieldInfo->offset       = Offset;
    fieldInfo->tempRegOrUniformOffset = TempRegOrUniformOffset;

    fieldSym->u2.fieldInfo  = fieldInfo;

    /* add the field symbol to struct type */
    if (StructType->u2.fields == gcvNULL)
    {
        /* create a field list */
        errCode = VIR_IdList_Init(&Shader->mempool,
                                 8, /* init field count */
                                 &StructType->u2.fields);
        CHECK_ERROR(errCode, "AddField");
    }

    errCode = VIR_IdList_Add(StructType->u2.fields, *FieldSymId);
    CHECK_ERROR(errCode, "AddField");

    return errCode;
}

gctUINT VIR_Type_GetVirRegCount(VIR_Shader * Shader,
                                VIR_Type *  Type)
{
    gctUINT virRegCount = 0;
    if (VIR_Type_isPrimitive(Type))
    {
        virRegCount = VIR_GetTypeRows(VIR_Type_GetIndex(Type));
    }
    else
    {
        VIR_Type * baseType;
        switch (VIR_Type_GetKind(Type))
        {
        case VIR_TY_ARRAY:
            baseType = VIR_Shader_GetTypeFromId(Shader,
                                                VIR_Type_GetBaseTypeId(Type));
            return VIR_Type_GetArrayLength(Type) *
                        VIR_Type_GetVirRegCount(Shader, baseType);
        case VIR_TY_STRUCT:
            /* get the last field of struct*/
            if (VIR_Type_GetFields(Type) &&
                VIR_IdList_Count(VIR_Type_GetFields(Type)) > 0)
            {
                VIR_SymId fieldSymId = VIR_IdList_GetId(VIR_Type_GetFields(Type),
                                            VIR_IdList_Count(VIR_Type_GetFields(Type)) - 1 );
                VIR_Symbol * fieldSym = VIR_Shader_GetSymFromId(Shader, fieldSymId);
                return VIR_Symbol_GetFieldInfo(fieldSym)->tempRegOrUniformOffset + 1;
            }
            else
                return 1;
        case VIR_TY_POINTER:
            return 1;
        case VIR_TY_SCALAR:
        case VIR_TY_VECTOR:
        case VIR_TY_MATRIX:
            /* should not reach here, it is handle in primitive type */
            gcmASSERT(gcvFALSE);
            return 1;
        default:
            return 1;
        }
    }
    return virRegCount;
}

gctUINT VIR_Type_GetIndexingRange(VIR_Shader * Shader,
                                     VIR_Type *  Type)
{
    gctUINT indexingRange = 0;
    if (VIR_Type_isPrimitive(Type))
    {
        indexingRange = VIR_GetTypeRows(VIR_Type_GetIndex(Type));
        if (indexingRange == 0)
        {
            /* types which doesn't have row concept like image/sampler */
            indexingRange = 1;
        }
    }
    else
    {
        VIR_Type * baseType;
        switch (VIR_Type_GetKind(Type))
        {
        case VIR_TY_ARRAY:
            baseType = VIR_Shader_GetTypeFromId(Shader,
                                                VIR_Type_GetBaseTypeId(Type));
            return VIR_Type_GetArrayLength(Type) *
                        VIR_Type_GetIndexingRange(Shader, baseType);
        case VIR_TY_STRUCT:
            /* get the last field of struct*/
            if (VIR_Type_GetFields(Type) &&
                VIR_IdList_Count(VIR_Type_GetFields(Type)) > 0)
            {
                VIR_SymId fieldSymId = VIR_IdList_GetId(VIR_Type_GetFields(Type),
                                            VIR_IdList_Count(VIR_Type_GetFields(Type)) - 1 );
                VIR_Symbol * fieldSym = VIR_Shader_GetSymFromId(Shader, fieldSymId);
                return VIR_Symbol_GetFieldInfo(fieldSym)->tempRegOrUniformOffset + 1;
            }
            else
                return 1;
        case VIR_TY_POINTER:
            return 1;
        case VIR_TY_SCALAR:
        case VIR_TY_VECTOR:
        case VIR_TY_MATRIX:
            /* should not reach here, it is handle in primitive type */
            gcmASSERT(gcvFALSE);
            return 1;
        default:
            return 1;
        }
    }
    return indexingRange;
}
gctUINT VIR_Symbol_GetVirIoRegCount(VIR_Shader * Shader,
                                  VIR_Symbol*    Sym)
{
    return VIR_Type_GetVirRegCount(Shader, VIR_Symbol_GetType(Sym));
}

/* For dual16 shader, the input symbol could be in 2 registers */
gctINT
VIR_Symbol_GetRegSize(
    IN VIR_Shader       *pShader,
    IN VSC_HW_CONFIG    *pHwCfg,
    IN VIR_Symbol       *Sym)
{
    gctINT  retValue = 1;

    if (VIR_Shader_isDual16Mode(pShader) &&
        VIR_Symbol_isInput(Sym) &&
        VIR_Symbol_GetPrecision(Sym) == VIR_PRECISION_HIGH
        )
    {
        gctUINT         components = VIR_Symbol_GetComponents(Sym);

        if (pHwCfg->hwFeatureFlags.highpVaryingShift)
        {
            if (components > 2)
            {
                retValue = 2;
            }
        }
        else
        {
            retValue = 2;
        }
    }

    return retValue;
}

VIR_SymIndexingInfo
VIR_Symbol_GetIndexingInfo(
    VIR_Shader * Shader,
    VIR_Symbol *Sym
    )
{
    VIR_SymIndexingInfo symIndexingInfo;

    symIndexingInfo.virRegSym = Sym;
    if (VIR_Symbol_GetKind(Sym) == VIR_SYM_VIRREG)
    {
        gctINT          arrayVirRegStride   = 1;
        VIR_VirRegId    underlyingSymVirRegId;
        VIR_Type *      underlyingSymType;

        symIndexingInfo.underlyingSym = VIR_Symbol_GetVregVariable(Sym); /* set the sym to corresponding variable */
        underlyingSymVirRegId = VIR_Symbol_GetVariableVregIndex(symIndexingInfo.underlyingSym);
        underlyingSymType = VIR_Symbol_GetType(symIndexingInfo.underlyingSym);
        gcmASSERT(VIR_Symbol_GetVregIndex(Sym) >= underlyingSymVirRegId);

        /* get array indexing info */
        if (VIR_Type_GetKind(underlyingSymType) == VIR_TY_ARRAY)
        {
            VIR_TypeId base_type_id = VIR_Type_GetBaseTypeId(underlyingSymType);
            VIR_Type * base_type = VIR_Shader_GetTypeFromId(Shader, base_type_id);
            arrayVirRegStride = VIR_Type_GetVirRegCount(Shader, base_type);
            symIndexingInfo.arrayIndexing  =
                (VIR_Symbol_GetVregIndex(Sym) - underlyingSymVirRegId) / arrayVirRegStride ;
            symIndexingInfo.elemOffset     =
                (VIR_Symbol_GetVregIndex(Sym) - underlyingSymVirRegId) % arrayVirRegStride;
        }
        else
        {
            symIndexingInfo.arrayIndexing  = 0;
            symIndexingInfo.elemOffset     = (VIR_Symbol_GetVregIndex(Sym) - underlyingSymVirRegId);
        }
    }
    else
    {
        symIndexingInfo.underlyingSym  = Sym;
        symIndexingInfo.arrayIndexing  = 0;
        symIndexingInfo.elemOffset     = 0;
    }

    return symIndexingInfo;
}

VIR_Enable
VIR_Type_Conv2Enable(
    IN VIR_Type         *Type
    )
{
    VIR_PrimitiveTypeId baseType = VIR_Type_GetBaseTypeId(Type);

    return VIR_TypeId_Conv2Enable(baseType);
}

/* check if the type1 from shader1 is indentical to type2 from shader2 */
gctBOOL
VIR_Type_Identical(
    VIR_Shader* Shader1,
    VIR_Type*   Type1,
    VIR_Shader* Shader2,
    VIR_Type*   Type2
    )
{
    VIR_TypeId tyId1 = VIR_Type_GetIndex(Type1);
    VIR_TypeId tyId2 = VIR_Type_GetIndex(Type2);
    gctUINT     i;

    /* type index are the same */
    if (tyId1 == tyId2 &&
       (VIR_TypeId_isPrimitive(tyId1) ||
         Shader1 == Shader2 ))
    {
        /* they are same types if both are from the same shader or are
         * primitive type which index is the same across all shaders */
        return gcvTRUE;
    }
    if ((VIR_TypeId_isPrimitive(tyId1) || VIR_TypeId_isPrimitive(tyId2)) &&
        tyId1 != tyId2)
    {
        /* types are not the same if one is primitive type and type ids are the same */
        return gcvFALSE;
    }
    if (VIR_Type_GetKind(Type1) != VIR_Type_GetKind(Type2))
    {
        /* types are not the same if they are not the same kind */
        return gcvFALSE;
    }
    switch (VIR_Type_GetKind(Type1)) {
    case VIR_TY_ARRAY:
        return VIR_Type_GetArrayLength(Type1) == VIR_Type_GetArrayLength(Type2) &&
               VIR_Type_Identical(Shader1,
                                  VIR_Shader_GetTypeFromId(Shader1, VIR_Type_GetBaseTypeId(Type1)),
                                  Shader2,
                                  VIR_Shader_GetTypeFromId(Shader2, VIR_Type_GetBaseTypeId(Type2)));
    case VIR_TY_STRUCT:
        {
            /* check name first */
            VIR_SymIdList * fields1,
                          * fields2;
            if ((VIR_Type_GetFlags(Type1) & VIR_TYFLAG_ISUNION) !=  (VIR_Type_GetFlags(Type2) & VIR_TYFLAG_ISUNION) ||
                (VIR_Type_GetFlags(Type1) & VIR_TYFLAG_ANONYMOUS) !=  (VIR_Type_GetFlags(Type2) & VIR_TYFLAG_ANONYMOUS))
            {
                return gcvFALSE;
            }
            if ((VIR_Type_GetFlags(Type1) & VIR_TYFLAG_ANONYMOUS) !=0 &&
                !gcmIS_SUCCESS(gcoOS_StrCmp(VIR_Shader_GetTypeNameString(Shader1, Type1),
                                            VIR_Shader_GetTypeNameString(Shader2, Type2))))
            {
                return gcvFALSE;
            }

            /* check fields: name, type, offset*/
            fields1 = VIR_Type_GetFields(Type1);
            fields2 = VIR_Type_GetFields(Type2);
            if (VIR_IdList_Count(fields1) != VIR_IdList_Count(fields2))
            {
                return gcvFALSE;
            }
            for (i=0; i < VIR_IdList_Count(fields1); i++)
            {
                VIR_Id      id1       = VIR_IdList_GetId(VIR_Type_GetFields(Type1), i);
                VIR_Symbol *fieldSym1 = VIR_Shader_GetSymFromId(Shader1, id1);
                VIR_Type   *fieldTy1  = VIR_Symbol_GetType(fieldSym1);
                VIR_FieldInfo *fInfo1 = VIR_Symbol_GetFieldInfo(fieldSym1);
                gctSTRING name1       = VIR_Shader_GetSymNameString(Shader1, fieldSym1);
                VIR_Id      id2       = VIR_IdList_GetId(VIR_Type_GetFields(Type2), i);
                VIR_Symbol *fieldSym2 = VIR_Shader_GetSymFromId(Shader2, id2);
                VIR_Type   *fieldTy2  = VIR_Symbol_GetType(fieldSym2);
                VIR_FieldInfo *fInfo2 = VIR_Symbol_GetFieldInfo(fieldSym2);
                gctSTRING name2       = VIR_Shader_GetSymNameString(Shader2, fieldSym2);
                if (!gcmIS_SUCCESS(gcoOS_StrCmp(name1, name2)))
                {
                    /* field names are not the same */
                    return gcvFALSE;
                }
                if (!VIR_Type_Identical(Shader1, fieldTy1, Shader2, fieldTy2))
                {
                    /* field types are not the same */
                    return gcvFALSE;
                }
                if (fInfo1->offset != fInfo2->offset)
                {
                    /* field offsets are the same */
                    return gcvFALSE;
                }
            }
        }
        return gcvTRUE;
    case VIR_TY_FUNCTION:
        {
            VIR_Type * retType1,
                     * retType2;
            /* check return type */
            retType1 = VIR_Shader_GetTypeFromId(Shader1, VIR_Type_GetBaseTypeId(Type1));
            retType2 = VIR_Shader_GetTypeFromId(Shader2, VIR_Type_GetBaseTypeId(Type2));
            if (!VIR_Type_Identical(Shader1, retType1, Shader2, retType2))
            {
                /* return types are not the same */
                return gcvFALSE;
            }
            /* check parameter types */
            if (VIR_IdList_Count(VIR_Type_GetParameters(Type1)) !=  VIR_IdList_Count(VIR_Type_GetParameters(Type2)))
            {
                /* parameter counts are not the same */
                return gcvFALSE;
            }
            for(i = 0; i < VIR_IdList_Count(VIR_Type_GetParameters(Type1)); ++i)
            {
                VIR_Id    id1        = VIR_IdList_GetId(VIR_Type_GetParameters(Type1), i);
                VIR_Type *paramType1 = VIR_Shader_GetTypeFromId(Shader1, id1);
                VIR_Id    id2        = VIR_IdList_GetId(VIR_Type_GetParameters(Type2), i);
                VIR_Type *paramType2 = VIR_Shader_GetTypeFromId(Shader2, id2);

                if (!VIR_Type_Identical(Shader1, paramType1, Shader2, paramType2))
                {
                    /* parameter types are not the same */
                    return gcvFALSE;
                }
            }
        }
        return gcvTRUE;
    case VIR_TY_POINTER:
        return VIR_Type_GetQualifier(Type1) == VIR_Type_GetQualifier(Type2) &&
               VIR_Type_GetAddrSpace(Type1) == VIR_Type_GetAddrSpace(Type2) &&
               VIR_Type_GetAlignement(Type1) == VIR_Type_GetAlignement(Type2) &&
               VIR_Type_Identical(Shader1,
                                  VIR_Shader_GetTypeFromId(Shader1, VIR_Type_GetBaseTypeId(Type1)),
                                  Shader2,
                                  VIR_Shader_GetTypeFromId(Shader2, VIR_Type_GetBaseTypeId(Type2)));

    case VIR_TY_VOID:
        return gcvTRUE;
    case VIR_TY_VECTOR:  /* we only accept primitive vector now */
    case VIR_TY_MATRIX:  /* we only accept primitive matrix now */
    default:
        break;
    }
    return gcvFALSE;
}

static VIR_Precision
_fixUniformPrecision(
   IN VIR_Shader* Shader,
   IN VIR_Precision Precision,
   IN VIR_PrimitiveTypeId Type,
   IN VIR_ShaderKind ShaderKind
   )
{
    VIR_Precision precision = Precision;

    /* Get real precision if default.
    ** Expand "default" to one of "lowp", "mediump" or "highp".
    */
    if (precision == VIR_PRECISION_DEFAULT)
    {
        switch (Type)
        {
        case VIR_TYPE_FLOAT64:
        case VIR_TYPE_FLOAT32:
        case VIR_TYPE_FLOAT16:
        case VIR_TYPE_FLOAT_X2:
        case VIR_TYPE_FLOAT_X3:
        case VIR_TYPE_FLOAT_X4:
        case VIR_TYPE_FLOAT_X8:
        case VIR_TYPE_FLOAT_X16:
        case VIR_TYPE_FLOAT16_X2:
        case VIR_TYPE_FLOAT16_X3:
        case VIR_TYPE_FLOAT16_X4:
        case VIR_TYPE_FLOAT16_X8:
        case VIR_TYPE_FLOAT16_X16:
        case VIR_TYPE_FLOAT_2X2:
        case VIR_TYPE_FLOAT_3X3:
        case VIR_TYPE_FLOAT_4X4:
        case VIR_TYPE_FLOAT_2X3:
        case VIR_TYPE_FLOAT_2X4:
        case VIR_TYPE_FLOAT_3X2:
        case VIR_TYPE_FLOAT_3X4:
        case VIR_TYPE_FLOAT_4X2:
        case VIR_TYPE_FLOAT_4X3:
            if ((!VIR_Shader_IsES30Compiler(Shader) && !VIR_Shader_IsES31Compiler(Shader)) ||
                ShaderKind == VIR_SHADER_VERTEX)
            {
                precision = VIR_PRECISION_HIGH;
            }
            break;

        case VIR_TYPE_INT32:
        case VIR_TYPE_INT16:
        case VIR_TYPE_INT8:
        case VIR_TYPE_UINT32:
        case VIR_TYPE_UINT16:
        case VIR_TYPE_UINT8:
        case VIR_TYPE_INT64:
        case VIR_TYPE_UINT64:
        case VIR_TYPE_INTEGER_X2:
        case VIR_TYPE_INTEGER_X3:
        case VIR_TYPE_INTEGER_X4:
        case VIR_TYPE_INTEGER_X8:
        case VIR_TYPE_INTEGER_X16:
        case VIR_TYPE_UINT_X2:
        case VIR_TYPE_UINT_X3:
        case VIR_TYPE_UINT_X4:
        case VIR_TYPE_UINT_X8:
        case VIR_TYPE_UINT_X16:
        case VIR_TYPE_UINT8_X2:
        case VIR_TYPE_UINT8_X3:
        case VIR_TYPE_UINT8_X4:
        case VIR_TYPE_UINT8_X8:
        case VIR_TYPE_UINT8_X16:
        case VIR_TYPE_INT8_X2:
        case VIR_TYPE_INT8_X3:
        case VIR_TYPE_INT8_X4:
        case VIR_TYPE_INT8_X8:
        case VIR_TYPE_INT8_X16:
        case VIR_TYPE_UINT16_X2:
        case VIR_TYPE_UINT16_X3:
        case VIR_TYPE_UINT16_X4:
        case VIR_TYPE_UINT16_X8:
        case VIR_TYPE_UINT16_X16:
        case VIR_TYPE_INT16_X2:
        case VIR_TYPE_INT16_X3:
        case VIR_TYPE_INT16_X4:
        case VIR_TYPE_INT16_X8:
        case VIR_TYPE_INT16_X16:
        case VIR_TYPE_UINT64_X2:
        case VIR_TYPE_UINT64_X3:
        case VIR_TYPE_UINT64_X4:
        case VIR_TYPE_UINT64_X8:
        case VIR_TYPE_UINT64_X16:
        case VIR_TYPE_INT64_X2:
        case VIR_TYPE_INT64_X3:
        case VIR_TYPE_INT64_X4:
        case VIR_TYPE_INT64_X8:
        case VIR_TYPE_INT64_X16:
            if (ShaderKind == VIR_SHADER_VERTEX)
            {
                precision = VIR_PRECISION_HIGH;
            }
            else if (ShaderKind == VIR_SHADER_FRAGMENT)
            {
                precision = VIR_PRECISION_MEDIUM;
            }
            break;

        case VIR_TYPE_SAMPLER_2D:
        case VIR_TYPE_SAMPLER_CUBIC:
            precision = VIR_PRECISION_LOW;
            break;
        default:
            break;
        }
    }

    return precision;
}

VSC_ErrCode
VIR_Uniform_Identical(
    IN VIR_Shader* Shader1,
    IN VIR_Symbol* Sym1,
    IN VIR_Shader* Shader2,
    IN VIR_Symbol* Sym2,
    IN gctBOOL     CheckPrecision,
    OUT gctBOOL*   Matched
    )
{
    VSC_ErrCode                errCode = VSC_ERR_NONE;
    gctSTRING                  name1, name2;
    VIR_Type*                  type1;
    VIR_Type*                  type2;
    gctBOOL                    matched = gcvFALSE;
    VIR_Precision              precision1, precision2;

    do
    {
        name1 = VIR_Shader_GetSymNameString(Shader1, Sym1);
        name2 = VIR_Shader_GetSymNameString(Shader2, Sym2);

        /* If two uniforms have different name, skip these. */
        if (!gcmIS_SUCCESS(gcoOS_StrCmp(name1, name2)))
        {
            break;
        }

        /* name matched, start to check the other info. */
        if (VIR_Symbol_GetKind(Sym1) != VIR_Symbol_GetKind(Sym2))
        {
            errCode = VSC_ERR_UNIFORM_TYPE_MISMATCH;
            ON_ERROR(errCode, "Uniform \"%s\" type mismatch.", name1);
        }

        matched = gcvTRUE;

        type1 = VIR_Symbol_GetType(Sym1);
        type2 = VIR_Symbol_GetType(Sym2);

        if (Matched && !*Matched)
        {
            /* Check all uniform member type. */
            if (!VIR_Type_Identical(Shader1, type1, Shader2, type2))
            {
                errCode = VSC_ERR_UNIFORM_TYPE_MISMATCH;
                ON_ERROR(errCode, "Uniform \"%s\" type mismatch.", name1);
            }
        }

        if (VIR_Type_GetKind(type1) == VIR_TY_STRUCT)
        {
            gctUINT            i;
            VIR_SymIdList *    fields1;

            fields1 = VIR_Type_GetFields(type1);
            for (i = 0; i < VIR_IdList_Count(fields1); i++)
            {
                VIR_Symbol *fieldSym1 = VIR_Shader_GetSymFromId(Shader1, VIR_IdList_GetId(VIR_Type_GetFields(type1), i));
                VIR_Symbol *fieldSym2 = VIR_Shader_GetSymFromId(Shader2, VIR_IdList_GetId(VIR_Type_GetFields(type2), i));

                errCode = VIR_Uniform_Identical(Shader1,
                                                fieldSym1,
                                                Shader2,
                                                fieldSym2,
                                                CheckPrecision,
                                                &matched);
                ON_ERROR(errCode, "Uniform \"%s\" precision mismatch.", VIR_Shader_GetSymNameString(Shader1, fieldSym1));
            }
        }
        else
        {
            if (CheckPrecision)
            {
                /* Check all uniform member precision. */
                precision1 = _fixUniformPrecision(Shader1,
                                                  VIR_Symbol_GetPrecision(Sym1),
                                                  VIR_Type_GetBaseTypeId(type1),
                                                  VIR_Shader_GetKind(Shader1));
                precision2 = _fixUniformPrecision(Shader2,
                                                  VIR_Symbol_GetPrecision(Sym2),
                                                  VIR_Type_GetBaseTypeId(type2),
                                                  VIR_Shader_GetKind(Shader2));
                if (precision1 != precision2)
                {
                    errCode = VSC_ERR_UNIFORM_TYPE_MISMATCH;
                    ON_ERROR(errCode, "Uniform \"%s\" precision mismatch.", name1);
                }
            }
            if (VIR_Symbol_GetBinding(Sym1) != VIR_Symbol_GetBinding(Sym2))
            {
                errCode = VSC_ERR_UNIFORM_TYPE_MISMATCH;
                ON_ERROR(errCode, "Uniform \"%s\" binding mismatch.", name1);
            }
            /* Check the image format and binding. */
            if (VIR_Symbol_GetKind(Sym1) == VIR_SYM_IMAGE)
            {
                if ((VIR_Symbol_GetLayoutQualifier(Sym1) & VIR_LAYQUAL_IMAGE_FORMAT_MASK) !=
                    (VIR_Symbol_GetLayoutQualifier(Sym2) & VIR_LAYQUAL_IMAGE_FORMAT_MASK))
                {
                    errCode = VSC_ERR_UNIFORM_TYPE_MISMATCH;
                    ON_ERROR(errCode, "Uniform \"%s\" image format mismatch.", name1);
                }
            }
        }
    } while(gcvFALSE);

    if (Matched)
    {
        *Matched = matched;
    }

OnError:
    return errCode;
}

VIR_Enable
VIR_TypeId_Conv2Enable(
    IN VIR_TypeId       TypeId
    )
{
    gcmASSERT(VIR_TypeId_isPrimitive(TypeId));
    if (VIR_TypeId_isSamplerOrImage(TypeId))
    {
            return VIR_ENABLE_XYZW;
    }
    switch(VIR_GetTypeComponents(TypeId))
    {
        case 0:
            return VIR_ENABLE_NONE;
        case 1:
            return VIR_ENABLE_X;
        case 2:
            return VIR_ENABLE_XY;
        case 3:
            return VIR_ENABLE_XYZ;
        case 4:
            return VIR_ENABLE_XYZW;
        default:
            gcmASSERT(0);
    }
    return VIR_ENABLE_XYZW;
}

VIR_TypeId
VIR_TypeId_ComposeNonOpaqueType(
    IN VIR_TypeId       ComponentType,
    IN gctUINT          CompCount,
    IN gctUINT          RowCount
    )
{
    if (RowCount > 1)
    {
        gcmASSERT(ComponentType == VIR_TYPE_FLOAT32);

        if (RowCount == 2)
        {
            if (CompCount == 2)
            {
                return VIR_TYPE_FLOAT_2X2;
            }
            else if (CompCount == 3)
            {
                return VIR_TYPE_FLOAT_2X3;
            }
            else if (CompCount == 4)
            {
                return VIR_TYPE_FLOAT_2X4;
            }
            else
            {
                gcmASSERT(gcvFALSE);
            }
        }
        else if (RowCount == 3)
        {
            if (CompCount == 2)
            {
                return VIR_TYPE_FLOAT_3X2;
            }
            else if (CompCount == 3)
            {
                return VIR_TYPE_FLOAT_3X3;
            }
            else if (CompCount == 4)
            {
                return VIR_TYPE_FLOAT_3X4;
            }
            else
            {
                gcmASSERT(gcvFALSE);
            }
        }
        else if (RowCount == 4)
        {
            if (CompCount == 2)
            {
                return VIR_TYPE_FLOAT_4X2;
            }
            else if (CompCount == 3)
            {
                return VIR_TYPE_FLOAT_4X3;
            }
            else if (CompCount == 4)
            {
                return VIR_TYPE_FLOAT_4X4;
            }
            else
            {
                gcmASSERT(gcvFALSE);
            }
        }
        else
        {
            gcmASSERT(gcvFALSE);
        }
    }
    else
    {
        switch(ComponentType)
        {
        case VIR_TYPE_FLOAT32:
            switch(CompCount)
            {
                case 1:
                    return VIR_TYPE_FLOAT32;
                case 2:
                    return VIR_TYPE_FLOAT_X2;
                case 3:
                    return VIR_TYPE_FLOAT_X3;
                case 4:
                    return VIR_TYPE_FLOAT_X4;
                case 8:
                    return VIR_TYPE_FLOAT_X8;
                case 16:
                    return VIR_TYPE_FLOAT_X16;
                default:
                    gcmASSERT(0);
            }
        case VIR_TYPE_FLOAT16:
            switch(CompCount)
            {
                case 1:
                    return VIR_TYPE_FLOAT16;
                case 2:
                    return VIR_TYPE_FLOAT16_X2;
                case 3:
                    return VIR_TYPE_FLOAT16_X3;
                case 4:
                    return VIR_TYPE_FLOAT16_X4;
                case 8:
                    return VIR_TYPE_FLOAT16_X8;
                case 16:
                    return VIR_TYPE_FLOAT16_X16;
                default:
                    gcmASSERT(0);
            }
        case VIR_TYPE_INT32:
            switch(CompCount)
            {
                case 1:
                    return VIR_TYPE_INT32;
                case 2:
                    return VIR_TYPE_INTEGER_X2;
                case 3:
                    return VIR_TYPE_INTEGER_X3;
                case 4:
                    return VIR_TYPE_INTEGER_X4;
                case 8:
                    return VIR_TYPE_INTEGER_X8;
                case 16:
                    return VIR_TYPE_INTEGER_X16;
                default:
                    gcmASSERT(0);
            }
        case VIR_TYPE_INT16:
            switch(CompCount)
            {
                case 1:
                    return VIR_TYPE_INT16;
                case 2:
                    return VIR_TYPE_INT16_X2;
                case 3:
                    return VIR_TYPE_INT16_X3;
                case 4:
                    return VIR_TYPE_INT16_X4;
                case 8:
                    return VIR_TYPE_INT16_X8;
                case 16:
                    return VIR_TYPE_INT16_X16;
                default:
                    gcmASSERT(0);
            }
        case VIR_TYPE_INT8:
            switch(CompCount)
            {
                case 1:
                    return VIR_TYPE_INT8;
                case 2:
                    return VIR_TYPE_INT8_X2;
                case 3:
                    return VIR_TYPE_INT8_X3;
                case 4:
                    return VIR_TYPE_INT8_X4;
                case 8:
                    return VIR_TYPE_INT8_X8;
                case 16:
                    return VIR_TYPE_INT8_X16;
                default:
                    gcmASSERT(0);
            }
        case VIR_TYPE_UINT32:
            switch(CompCount)
            {
                case 1:
                    return VIR_TYPE_UINT32;
                case 2:
                    return VIR_TYPE_UINT_X2;
                case 3:
                    return VIR_TYPE_UINT_X3;
                case 4:
                    return VIR_TYPE_UINT_X4;
                case 8:
                    return VIR_TYPE_UINT_X8;
                case 16:
                    return VIR_TYPE_UINT_X16;
                default:
                    gcmASSERT(0);
            }
        case VIR_TYPE_UINT16:
            switch(CompCount)
            {
                case 1:
                    return VIR_TYPE_UINT16;
                case 2:
                    return VIR_TYPE_UINT16_X2;
                case 3:
                    return VIR_TYPE_UINT16_X3;
                case 4:
                    return VIR_TYPE_UINT16_X4;
                case 8:
                    return VIR_TYPE_UINT16_X8;
                case 16:
                    return VIR_TYPE_UINT16_X16;
                default:
                    gcmASSERT(0);
            }
        case VIR_TYPE_UINT8:
            switch(CompCount)
            {
                case 1:
                    return VIR_TYPE_UINT8;
                case 2:
                    return VIR_TYPE_UINT8_X2;
                case 3:
                    return VIR_TYPE_UINT8_X3;
                case 4:
                    return VIR_TYPE_UINT8_X4;
                case 8:
                    return VIR_TYPE_UINT8_X8;
                case 16:
                    return VIR_TYPE_UINT8_X16;
                default:
                    gcmASSERT(0);
            }
        case VIR_TYPE_INT64:
            switch(CompCount)
            {
                case 1:
                    return VIR_TYPE_INT64;
                case 2:
                    return VIR_TYPE_INT64_X2;
                case 3:
                    return VIR_TYPE_INT64_X3;
                case 4:
                    return VIR_TYPE_INT64_X4;
                case 8:
                    return VIR_TYPE_INT64_X8;
                case 16:
                    return VIR_TYPE_INT64_X16;
                default:
                    gcmASSERT(0);
            }
        case VIR_TYPE_UINT64:
            switch(CompCount)
            {
                case 1:
                    return VIR_TYPE_UINT64;
                case 2:
                    return VIR_TYPE_UINT64_X2;
                case 3:
                    return VIR_TYPE_UINT64_X3;
                case 4:
                    return VIR_TYPE_UINT64_X4;
                case 8:
                    return VIR_TYPE_UINT64_X8;
                case 16:
                    return VIR_TYPE_UINT64_X16;
                default:
                    gcmASSERT(0);
            }
        case VIR_TYPE_BOOLEAN:
            switch(CompCount)
            {
                case 1:
                    return VIR_TYPE_BOOLEAN;
                case 2:
                    return VIR_TYPE_BOOLEAN_X2;
                case 3:
                    return VIR_TYPE_BOOLEAN_X3;
                case 4:
                    return VIR_TYPE_BOOLEAN_X4;
                case 8:
                    return VIR_TYPE_BOOLEAN_X8;
                case 16:
                    return VIR_TYPE_BOOLEAN_X16;
                default:
                    gcmASSERT(0);
            }
        case VIR_TYPE_SNORM8:
            switch (CompCount)
            {
            case 1: return VIR_TYPE_SNORM8;

            default:
                return VIR_TYPE_SNORM8;
            }
        case VIR_TYPE_UNORM8:
            switch (CompCount)
            {
            case 1: return VIR_TYPE_UNORM8;

            default:
                return VIR_TYPE_UNORM8;
            }
        default:
            gcmASSERT(0);
        }
    }

    return VIR_TYPE_UNKNOWN;
}

VIR_TypeId
VIR_TypeId_ComposeNonOpaqueArrayedType(
    IN VIR_Shader *     Shader,
    IN VIR_TypeId       ComponentType,
    IN gctUINT          CompCount,
    IN gctUINT          RowCount,
    IN gctINT           arrayLength
    )
{
    VIR_TypeId typeId = VIR_TypeId_ComposeNonOpaqueType(ComponentType, CompCount, RowCount);

    if (arrayLength != -1)
    {
        if (VIR_Shader_AddArrayType(Shader,
                                    typeId,
                                    arrayLength,
                                    &typeId) != VSC_ERR_NONE)
        {
            typeId = VIR_TYPE_UNKNOWN;
        }
    }

    return typeId;
}

/* symbol tables */
/* find the symbol by nameId or constId and its kind, kind should not be field,
 * virtual register, return NULL if symbol is not found */
VIR_Symbol *
VIR_Shader_FindSymbolById(
    IN  VIR_Shader *    Shader,
    IN  VIR_SymbolKind  SymbolKind,
    IN  VIR_Id          NameOrConstId)
{
    VIR_Symbol  sym;
    VIR_SymId   id;
    VIR_Symbol *pSym;

    gcmASSERT(SymbolKind != VIR_SYM_FIELD
             && SymbolKind != VIR_SYM_VIRREG);

    VIR_Symbol_SetKind(&sym, SymbolKind);
    if (SymbolKind == VIR_SYM_CONST)
    {
        VIR_Symbol_SetConstId(&sym, NameOrConstId);
    }
    else
    {
        VIR_Symbol_SetSymbolName(&sym, NameOrConstId);
    }

    id = vscBT_HashSearch(&Shader->symTable, &sym);
    if (VIR_Id_isInvalid(id))
    {
        pSym = gcvNULL;
    }
    else
    {
        pSym = VIR_GetSymFromId(&Shader->symTable, id);
    }

    return pSym;
}

/* symbol tables */
/* find the vreg symbol by temp register index, return NULL if symbol is not found */
VIR_Symbol *
VIR_Shader_FindSymbolByTempIndex(
    IN  VIR_Shader *    Shader,
    IN  VIR_Id          TempIndex)
{
    VIR_Symbol  sym;
    VIR_SymId   id;
    VIR_Symbol *pSym;

    VIR_Symbol_SetKind(&sym, VIR_SYM_VIRREG);
    VIR_Symbol_SetVregIndex(&sym, TempIndex);

    id = vscBT_HashSearch(&Shader->symTable, &sym);
    if (VIR_Id_isInvalid(id))
    {
        pSym = gcvNULL;
    }
    else
    {
        pSym = VIR_GetSymFromId(&Shader->symTable, id);
    }

    return pSym;
}

/* find the symbol by name and its kind, kind should not be field,
 * virtual register, or Constant, return NULL if symbol is not found */
VIR_Symbol *
VIR_Shader_FindSymbolByName(
    IN  VIR_Shader *    Shader,
    IN  VIR_SymbolKind  SymbolKind,
    IN  gctSTRING       Name)
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    VIR_NameId  nameId;

    /* add name to shader string table */
    errCode = VIR_Shader_AddString(Shader, Name, &nameId);
    if (errCode != VSC_ERR_NONE)
    {
        gcmASSERT(gcvFALSE);
        return gcvNULL;
    }

    return VIR_Shader_FindSymbolById(Shader, SymbolKind, nameId);
}

VSC_ErrCode
VIR_SymTable_AddSymbol(
    IN  VIR_SymTable *  SymTable,
    IN  VIR_SymbolKind  SymbolKind,
    IN  VIR_Id          NameOrConstIdOrRegId, /* constId for VIR_SYM_CONST,
                                                  VirRegId for VIR_SYM_VIRREG,
                                                  otherwise nameId */
    IN  VIR_Type *       Type, /* for VIR_SYM_FIELD, use struct typeId */
    IN  VIR_StorageClass Storage,
    OUT VIR_SymId*      SymId)
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    VIR_Symbol  sym;
    VIR_SymId   id;
    VIR_Symbol *pSym;

    memset(&sym, 0, sizeof(VIR_Symbol));
    VIR_Symbol_SetKind(&sym, SymbolKind);
    VIR_Symbol_SetType(&sym, Type);
    if (SymbolKind == VIR_SYM_CONST)
    {
        VIR_Symbol_SetConstId(&sym, NameOrConstIdOrRegId);
    }
    else if (SymbolKind == VIR_SYM_VIRREG)
    {
        VIR_Symbol_SetVregIndex(&sym, NameOrConstIdOrRegId);
    }
    else
    {
        VIR_Symbol_SetSymbolName(&sym, NameOrConstIdOrRegId);
    }

    if (SymbolKind == VIR_SYM_FIELD)
    {
        VIR_Symbol_SetStructType(&sym, Type);
    }

    id = vscBT_HashSearch(SymTable, &sym);
    if (VIR_Id_isInvalid(id))
    {
        /* not found in table, add it */
        id = vscBT_AddEntry(SymTable, &sym);
        if (VIR_Id_isInvalid(id))
        {
            errCode = VSC_ERR_OUT_OF_MEMORY;
        }
        else
        {
            if (BT_IS_FUNCTION_SCOPE(SymTable))
            {
                VIR_Id_SetFunctionScope(id);
            }
            pSym = VIR_GetSymFromId(SymTable, id);
            VIR_Symbol_SetIndex(pSym, id);
            VIR_Symbol_SetStorageClass(pSym, Storage);
            VIR_Symbol_SetType(pSym, Type);
            *SymId = id;
        }
    }
    else
    {
        errCode = VSC_ERR_REDEFINITION;
    }

    return errCode;
}


/* shader symbols */
VSC_ErrCode
VIR_Shader_AddFieldSymbol(
    IN  VIR_Shader *    Shader,
    IN  VIR_Id          NameId, /* field nameId */
    IN  VIR_Type *      Type, /* field type */
    IN  VIR_Type *      StructType, /* struct type of the field */
    IN  VIR_StorageClass Storage,
    OUT VIR_SymId*      SymId)
{
    VSC_ErrCode     errCode = VSC_ERR_NONE;
    VIR_Symbol *    sym = gcvNULL;

    errCode = VIR_Shader_AddSymbol(Shader,
                                   VIR_SYM_FIELD,
                                   NameId,
                                   StructType,
                                   Storage,
                                   SymId);
    if (errCode == VSC_ERR_NONE)
    {
        sym = VIR_Shader_GetSymFromId(Shader, *SymId);
        VIR_Symbol_SetType(sym, Type);
        VIR_Symbol_SetStructType(sym, StructType);
    }
    return errCode;
}

gctBOOL _setStructTypeSymid(
    IN  VIR_Shader *    Shader,
    IN  VIR_Type *      Type,
    IN  VIR_SymId       SymId
    )
{
    gctBOOL retVal = gcvFALSE;
    /* for now every struct type variable has its own struct
        defined for the variable:

            struct S { int a; int b; float c[2]; } s1, s2;

        We will see two struct type be defined s1 and s2

        TODO: use one struct type S for both s1 and s2
    */
    if (VIR_Type_isStruct(Type))
    {
        gcmASSERT(Type->u1.symId == VIR_INVALID_ID);
        Type->u1.symId = SymId;
        retVal = gcvTRUE;
    }
    else if (VIR_Type_isArray(Type))
    {
        VIR_Type * baseTy = VIR_Shader_GetTypeFromId(Shader,
                                        VIR_Type_GetBaseTypeId(Type));
        if (VIR_Type_isStruct(baseTy))
        {
            gcmASSERT(baseTy->u1.symId == VIR_INVALID_ID);
            baseTy->u1.symId = SymId;
            retVal = gcvTRUE;
        }
    }
    return retVal;
}

VSC_ErrCode
VIR_Shader_AddSymbol(
    IN  VIR_Shader *    Shader,
    IN  VIR_SymbolKind  SymbolKind,
    IN  VIR_Id          NameOrConstIdOrRegId, /* constId for VIR_SYM_CONST,
                                                  VirRegId for VIR_SYM_VIRREG,
                                                  otherwise nameId */
    IN  VIR_Type *       Type, /* for VIR_SYM_FIELD, use struct type */
    IN  VIR_StorageClass Storage,
    OUT VIR_SymId*      SymId)
{
    VSC_ErrCode     errCode = VSC_ERR_NONE;
    VIR_Symbol *    sym = gcvNULL;

    errCode = VIR_SymTable_AddSymbol(&Shader->symTable,
                                  SymbolKind,
                                  NameOrConstIdOrRegId,
                                  Type,
                                  Storage,
                                  SymId);
    if (errCode == VSC_ERR_NONE)
    {
        sym = VIR_Shader_GetSymFromId(Shader, *SymId);
        switch (SymbolKind) {
        case VIR_SYM_UNIFORM:
        case VIR_SYM_SAMPLER:
        case VIR_SYM_IMAGE:
            {
                VIR_Uniform *uniform;
                /* allocate uniform struct */
                uniform = (VIR_Uniform *)vscMM_Alloc(&Shader->mempool,
                                                    sizeof(VIR_Uniform));
                if (uniform == gcvNULL)
                {
                    return VSC_ERR_OUT_OF_MEMORY;
                }
                memset(uniform, 0, sizeof(VIR_Uniform));
                VIR_Symbol_SetUniform(sym, uniform);
                VIR_Symbol_SetLocation(sym, -1);
                uniform->sym    = sym->index;
                uniform->index = Shader->uniformCount;
                uniform->gcslIndex = -1;
                uniform->blockIndex = -1;
                uniform->offset = -1;
                uniform->lastIndexingIndex = -1;
                uniform->realUseArraySize = -1;
                uniform->physical = -1;
                if(SymbolKind == VIR_SYM_UNIFORM)
                {
                    uniform->u.initializer = VIR_INVALID_ID;
                }
                Shader->uniformCount++;

                /* add uniform to shader uniform list */
                VIR_IdList_Add(&Shader->uniforms, *SymId);
                break;
            }
        case VIR_SYM_UBO:
            {
                VIR_UniformBlock *   ubo;
                /* allocate uniform struct */
                ubo = (VIR_UniformBlock *)vscMM_Alloc(&Shader->mempool,
                                                      sizeof(VIR_UniformBlock));
                if (ubo == gcvNULL)
                {
                    return VSC_ERR_OUT_OF_MEMORY;
                }
                memset(ubo, 0, sizeof(VIR_UniformBlock));
                sym->u2.ubo     = ubo;
                ubo->sym        = sym->index;
                ubo->baseAddr   = VIR_INVALID_ID;

                /* add uniform block to shader UBO list */
                VIR_IdList_Add(&Shader->uniformBlocks, *SymId);
                break;
            }
        case  VIR_SYM_SBO:
            {
                /* TODO */
                VIR_IdList_Add(&Shader->buffers, *SymId);
                break;
            }
        case VIR_SYM_IOBLOCK:
            {
                VIR_IOBlock *   ibo;
                /* allocate ioblock struct */
                ibo = (VIR_IOBlock *)vscMM_Alloc(&Shader->mempool,
                                                 sizeof(VIR_IOBlock));
                if (ibo == gcvNULL)
                {
                    return VSC_ERR_OUT_OF_MEMORY;
                }
                memset(ibo, 0, sizeof(VIR_IOBlock));
                sym->u2.ioBlock     = ibo;
                ibo->sym            = sym->index;

                /* add IO block to shader UBO list */
                VIR_IdList_Add(&Shader->ioBlocks, *SymId);
                break;
            }
        case  VIR_SYM_VARIABLE:
            {
                if (VIR_Symbol_isPerPatchInput(sym))
                {
                    /* add attribute to shader attribute list */
                    VIR_IdList_Add(&Shader->perpatchInput, *SymId);
                }
                if (VIR_Symbol_isPerPatchOutput(sym))
                {
                    /* add attribute to shader attribute list */
                    VIR_IdList_Add(&Shader->perpatchOutput, *SymId);
                }
                if (VIR_Symbol_isAttribute(sym))
                {
                    /* add attribute to shader attribute list */
                    VIR_IdList_Add(&Shader->attributes, *SymId);
                }
                if (VIR_Symbol_isOutput(sym))
                {
                    /* add outputs to shader output list */
                    VIR_IdList_Add(&Shader->outputs, *SymId);
                }
            }
            break;
        case VIR_SYM_VIRREG:
            {
                (void)VIR_Shader_UpdateVirRegCount(Shader,
                                                   NameOrConstIdOrRegId);
                break;
            }
        default:
            break;
        }
    }
    if (VirSHADER_DumpCodeGenVerbose(Shader->_id))
    {
        VIR_Dumper * dumper = Shader->dumper;
        if (errCode != VSC_ERR_NONE)
        {
            char *str = VIR_Shader_GetStringFromId(Shader, NameOrConstIdOrRegId);
            VIR_LOG(dumper, "Error %d on adding %s: (id:%d)%s ",
                                   errCode, VIR_GetSymbolKindName(SymbolKind),
                                   NameOrConstIdOrRegId, str);
        }
        else
        {
            if (sym == gcvNULL) sym = VIR_Shader_GetSymFromId(Shader, *SymId);
            VIR_LOG(dumper, "Added %s %d: ", VIR_GetSymbolKindName(SymbolKind), *SymId);
            VIR_Symbol_Dump(dumper, sym, gcvTRUE);
        }
        VIR_LOG_FLUSH(dumper);
    }

    return errCode;
}

VSC_ErrCode
VIR_Shader_AddSymbolWithName(
    IN  VIR_Shader *    Shader,
    IN  VIR_SymbolKind  SymbolKind,
    IN  gctSTRING       Name,
    IN  VIR_Type *      Type, /* for VIR_SYM_FIELD, use struct typeId */
    IN  VIR_StorageClass Storage,
    OUT VIR_SymId*      SymId)
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    VIR_NameId  nameId;

    /* add name to shader string table */
    errCode = VIR_Shader_AddString(Shader, Name, &nameId);
    CHECK_ERROR(errCode, "AddString");
    return VIR_Shader_AddSymbol(Shader,
                                  SymbolKind,
                                  nameId,
                                  Type,
                                  Storage,
                                  SymId);
}

/* shaders */

gctUINT32
VIR_Shader_GetTotalInstructionCount(
    IN  VIR_Shader *    shader
    )
{
    VIR_FunctionList* func_list = VIR_Shader_GetFunctions(shader);
    VIR_FuncIterator iter;
    VIR_FunctionNode *node;
    gctUINT32 count = 0;

    VIR_FuncIterator_Init(&iter, func_list);
    for(node = VIR_FuncIterator_First(&iter); node != gcvNULL; node = VIR_FuncIterator_Next(&iter))
    {
        VIR_Function* func = node->function;
        count += VIR_Function_GetInstCount(func);
    }

    return count;
}

gctUINT
VIR_Shader_RenumberInstId(
    IN  VIR_Shader *  Shader
    )
{
    VIR_FuncIterator    func_iter;
    VIR_FunctionNode   *func_node;
    gctINT              instId   = 0;

    VIR_FuncIterator_Init(&func_iter, VIR_Shader_GetFunctions(Shader));

    for (func_node = VIR_FuncIterator_First(&func_iter);
         func_node != gcvNULL; func_node = VIR_FuncIterator_Next(&func_iter))
    {
        VIR_Function    *func       = func_node->function;
        VIR_Instruction *inst       = gcvNULL;

        inst = func->instList.pHead;
        while (inst != gcvNULL)
        {
            VIR_Inst_SetId(inst, instId++);
            inst = VIR_Inst_GetNext(inst);
        }

        func->_lastInstId = instId;
    }

    return instId;
}

VIR_Uniform*
VIR_Shader_GetUniformFromGCSLIndex(
    IN  VIR_Shader *  Shader,
    IN  gctINT        GCSLIndex
    )
{
    VIR_UniformIdList* uniforms = VIR_Shader_GetUniforms(Shader);
    gctUINT uniformCount = VIR_Shader_GetUniformCount(Shader);
    gctUINT i;

    for(i = 0; i < uniformCount; i++)
    {
        VIR_UniformId uniformID = VIR_IdList_GetId(uniforms, i);
        VIR_Symbol* uniformSym = VIR_Shader_GetSymFromId(Shader, uniformID);
        VIR_Uniform* uniform = gcvNULL;

        if(VIR_Symbol_isUniform(uniformSym))
        {
            uniform = VIR_Symbol_GetUniform(uniformSym);
        }
        else if(VIR_Symbol_isSampler(uniformSym))
        {
            uniform = VIR_Symbol_GetSampler(uniformSym);
        }
        else if(VIR_Symbol_isImage(uniformSym))
        {
            uniform = VIR_Symbol_GetImage(uniformSym);
        }
        gcmASSERT(uniform);
        if(uniform->gcslIndex == GCSLIndex)
        {
            return uniform;
        }
    }

    return gcvNULL;
}

/* setters */
void
VIR_Symbol_SetName(
    IN OUT VIR_Symbol *     Symbol,
    IN  VIR_NameId          Name
    )
{
    Symbol->u1.name = Name;
}

gctSTRING
VIR_Symbol_GetAttrName(
    IN VIR_Shader* pShader,
    IN VIR_Symbol *     AttrSymbol
    )
{
    if (VIR_Symbol_GetName(AttrSymbol) == VIR_NAME_IN_POSITION)
    {
        return "gl_Position";
    }
    else if (VIR_Symbol_GetName(AttrSymbol) == VIR_NAME_IN_POINT_SIZE)
    {
        return "gl_PointSize";
    }
    else
    {
        /* Normal one */
        return VIR_Shader_GetSymNameString(pShader, AttrSymbol);
    }
}

void
VIR_Symbol_SetConst(
    IN OUT VIR_Symbol *     Symbol,
    IN  VIR_ConstId         Constant
    )
{
    Symbol->u1.constId = Constant;
}

/* get RegCount of VirReg from the Shader, return the first VirRegId
 * user needs to add VIRREG symbol with the VirRegId to shader symbol table
 */
VIR_VirRegId
VIR_Shader_NewVirRegId(
    IN VIR_Shader *       Shader,
    IN gctUINT            RegCount
    )
{
    VIR_VirRegId regId;

    regId = (VIR_VirRegId)Shader->_tempRegCount;
    Shader->_tempRegCount += RegCount;

    return regId;
}

VIR_VirRegId
VIR_Shader_UpdateVirRegCount(
    IN VIR_Shader *        Shader,
    IN VIR_VirRegId        RegIndex
    )
{
    if (Shader->_tempRegCount <= RegIndex)
        Shader->_tempRegCount = RegIndex + 1;
    return (VIR_VirRegId)Shader->_tempRegCount;
}


gctUINT
VIR_Shader_GetVirRegCount(
    IN VIR_Shader       *Shader
    )
{
    return Shader->_tempRegCount;
}

VIR_VarTempRegInfo *
VIR_Shader_GetXFBVaryingTempRegInfo(
    IN VIR_Shader *    Shader,
    IN gctUINT         VaryingIndex
    )
{
    VIR_VarTempRegInfo * v;
    if (VIR_IdList_Count(Shader->transformFeedback.varyings) <= 0 ||
        VaryingIndex >= VIR_IdList_Count(Shader->transformFeedback.varyings))
    {
        return gcvNULL;
    }

    gcmASSERT(Shader->transformFeedback.varRegInfos != gcvNULL &&
              VaryingIndex < VIR_ValueList_Count(Shader->transformFeedback.varRegInfos));

    v = (VIR_VarTempRegInfo *)VIR_ValueList_GetValue(Shader->transformFeedback.varRegInfos,
                                                     VaryingIndex);
    return v;
}


void
VIR_Symbol_AddFlag(
    IN OUT VIR_Symbol * Symbol,
    IN VIR_SymFlag      Flag)
{
    Symbol->flags |= Flag;
}

void
VIR_Symbol_RemoveFlag(
    IN OUT VIR_Symbol * Symbol,
    IN VIR_SymFlag      Flag
    )
{
    Symbol->flags &= ~Flag;
}

void
VIR_Symbol_SetOffset(
    IN OUT VIR_Symbol * Symbol,
    IN gctUINT32        Offset,
    IN gctUINT          TempRegOrUniformOffset
    )
{
    gcmASSERT(VIR_Symbol_GetKind(Symbol) == VIR_SYM_FIELD);
    Symbol->u2.fieldInfo->offset = Offset;
    Symbol->u2.fieldInfo->tempRegOrUniformOffset = TempRegOrUniformOffset;
}

/* getters */

/* return true if the name of symbol1 in shader1 matches
 * the name of symbol2 in shader2 */
gctBOOL
VIR_Symbol_isNameMatch(
    IN VIR_Shader *        Shader1,
    IN VIR_Symbol *        Symbol1,
    IN VIR_Shader *        Shader2,
    IN VIR_Symbol *        Symbol2
    )
{
    gctSTRING     name1, name2, name1AfterDot, name2AfterDot;
    VIR_NameId    nameId1, nameId2;
    nameId1 = VIR_Symbol_GetName(Symbol1);
    nameId2 = VIR_Symbol_GetName(Symbol2);

    if (nameId1 <= VIR_NAME_BUILTIN_LAST && nameId2 <= VIR_NAME_BUILTIN_LAST)
    {
        if (nameId1 == nameId2 ||
            (nameId1 == VIR_NAME_IN_POSITION && nameId2 == VIR_NAME_POSITION) ||
            (nameId2 == VIR_NAME_IN_POSITION && nameId1 == VIR_NAME_POSITION) ||
            (nameId1 == VIR_NAME_IN_POINT_SIZE && nameId2 == VIR_NAME_POINT_SIZE) ||
            (nameId2 == VIR_NAME_IN_POINT_SIZE && nameId1 == VIR_NAME_POINT_SIZE) ||
            (nameId1 == VIR_NAME_SAMPLE_MASK_IN && nameId2 == VIR_NAME_SAMPLE_MASK) ||
            (nameId2 == VIR_NAME_SAMPLE_MASK_IN && nameId1 == VIR_NAME_SAMPLE_MASK) ||
            (nameId1 == VIR_NAME_PRIMITIVE_ID_IN && nameId2 == VIR_NAME_PRIMITIVE_ID) ||
            (nameId2 == VIR_NAME_PRIMITIVE_ID_IN && nameId1 == VIR_NAME_PRIMITIVE_ID)
            )
        {
            return gcvTRUE;
        }
        else
        {
            return gcvFALSE;
        }
    }

    name1 = VIR_Shader_GetSymNameString(Shader1, Symbol1);
    name2 = VIR_Shader_GetSymNameString(Shader2, Symbol2);
    /*
    ** If these two symbols are IO block, we only need to check block name.
    ** If these two symbols are IO block members, we can skip instance name.
    */
    if (VIR_Symbol_GetKind(Symbol1) == VIR_SYM_IOBLOCK ||
        VIR_Symbol_GetKind(Symbol2) == VIR_SYM_IOBLOCK)
    {
        gctINT nameLength1, nameLength2;

        if (VIR_Symbol_GetKind(Symbol1) != VIR_Symbol_GetKind(Symbol2))
        {
            return gcvFALSE;
        }

        nameLength1 = VIR_IOBLOCK_GetBlockNameLength(VIR_Symbol_GetIOB(Symbol1));
        nameLength2 = VIR_IOBLOCK_GetBlockNameLength(VIR_Symbol_GetIOB(Symbol2));

        if (nameLength1 == nameLength2 &&
            gcmIS_SUCCESS(gcoOS_StrNCmp(name1, name2, nameLength1)))
        {
            return gcvTRUE;
        }
        else
        {
            return gcvFALSE;
        }
    }
    else if (isSymIOBlockMember(Symbol1) || isSymIOBlockMember(Symbol2))
    {
        if (isSymIOBlockMember(Symbol1) != isSymIOBlockMember(Symbol2))
        {
            return gcvFALSE;
        }

        if (isSymInstanceMember(Symbol1))
        {
            gcoOS_StrStr(name1, ".", &name1AfterDot);
            name1 = (name1AfterDot != gcvNULL) ? &name1AfterDot[1] : name1;
        }

        if (isSymInstanceMember(Symbol2))
        {
            gcoOS_StrStr(name2, ".", &name2AfterDot);
            name2 = (name2AfterDot != gcvNULL) ? &name2AfterDot[1] : name2;
        }

        if (gcmIS_SUCCESS(gcoOS_StrCmp(name1, name2)))
        {
            return gcvTRUE;
        }
    }
    else if (gcmIS_SUCCESS(gcoOS_StrCmp(name1, name2)))
    {
        /* names match */
        return gcvTRUE;
    }

    return gcvFALSE;
}

gctUINT VIR_Symbol_GetComponents(VIR_Symbol *pSym)
{
    gctUINT         components = 0;
    VIR_Type        *symType = VIR_Symbol_GetType(pSym);

    if (VIR_Type_isPrimitive(symType))
    {
        components = VIR_GetTypeComponents(VIR_Type_GetIndex(symType));
    }
    else
    {
        components = VIR_GetTypeComponents(VIR_Type_GetBaseTypeId(symType));
    }

    return components;
}

/* functions */
VSC_ErrCode
VIR_Function_AddSymbol(
    IN  VIR_Function *  Function,
    IN  VIR_SymbolKind  SymbolKind,
    IN  VIR_Id          NameOrConstIdOrRegId, /* constId for VIR_SYM_CONST,
                                                  VirRegId for VIR_SYM_VIRREG,
                                                  otherwise nameId */
    IN  VIR_Type *       Type, /* for VIR_SYM_FIELD, use struct typeId */
    IN  VIR_StorageClass Storage,
    OUT VIR_SymId *     SymId
    )
{
    VSC_ErrCode errCode;
    VIR_Symbol *    sym;
    errCode = VIR_SymTable_AddSymbol(&Function->symTable,
                                  SymbolKind,
                                  NameOrConstIdOrRegId,
                                  Type,
                                  Storage,
                                  SymId);
    if (VirSHADER_DumpCodeGenVerbose(Function->hostShader->_id))
    {
        VIR_Dumper * dumper = Function->hostShader->dumper;
        sym = VIR_Function_GetSymFromId(Function, *SymId);
        VIR_LOG(dumper, "Added function scope symbol %d: ", VIR_Id_GetIndex(*SymId));
        VIR_Symbol_Dump(dumper, sym, gcvTRUE);
        VIR_LOG_FLUSH(dumper);
    }

    return errCode;
}

VSC_ErrCode
VIR_Function_AddSymbolWithName(
    IN  VIR_Function *  Function,
    IN  VIR_SymbolKind  SymbolKind,
    IN  gctSTRING       Name,
    IN  VIR_Type *      Type, /* for VIR_SYM_FIELD, use struct typeId */
    IN  VIR_StorageClass Storage,
    OUT VIR_SymId*      SymId)
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    VIR_NameId  nameId;

    /* add name to shader string table */
    errCode = VIR_Shader_AddString(Function->hostShader, Name, &nameId);
    CHECK_ERROR(errCode, "AddString");
    return VIR_Function_AddSymbol(Function,
                                  SymbolKind,
                                  nameId,
                                  Type,
                                  Storage,
                                  SymId);
}

VSC_ErrCode
VIR_Shader_CreateAnonymousName(
    IN  VIR_Shader *    Shader,
    IN  gctSTRING       AnonymousKindStr,
    OUT VIR_NameId *    NameId
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    gctCHAR     name[128];

    gcmASSERT(strlen(AnonymousKindStr) < 100);
#if defined(_WINDOWS) || (defined(_WIN32) || defined(WIN32))
    sprintf_s(name, sizeof(name), "_anony_%s_%d", AnonymousKindStr, Shader->_anonymousNameId++);
#else
    snprintf(name, sizeof(name), "_anony_%s_%d", AnonymousKindStr, Shader->_anonymousNameId++);
#endif
    errCode = VIR_Shader_AddString(Shader, name, NameId);

    return errCode;
}

VSC_ErrCode
VIR_Function_AddLabel(
    IN  VIR_Function *  Function,
    IN  gctSTRING       LabelName,
    OUT VIR_LabelId *   LabelId
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    VIR_LabelId labelId;
    VIR_SymId   symId;
    VIR_Label   label;
    gctCHAR     name[64];
    gctUINT     offset = 0;

    if(LabelName == gcvNULL)
    {
        gctSTRING funcName = VIR_Function_GetNameString(Function);
        /* construct label name */
        gcoOS_PrintStrSafe(name, sizeof(name), &offset, "#%s_label_%d",
                           funcName, VIR_Function_GetAndIncressLabelId(Function));
        LabelName = name;
    }

    /* add label to function's symbol table */
    errCode = VIR_Function_AddSymbolWithName(Function,
                                             VIR_SYM_LABEL,
                                             LabelName,
                                             VIR_Shader_GetTypeFromId(VIR_Function_GetShader(Function),
                                                    VIR_TYPE_UNKNOWN),
                                             VIR_STORAGE_UNKNOWN,
                                             &symId);
    CHECK_ERROR(errCode, "AddLabel");

    /* set label date to find if it is already exist */
    label.sym = symId;
    /* add label to function's label table */
    labelId = vscBT_Find(&Function->labelTable, &label);
    if (VIR_Id_isInvalid(labelId))
    {
        errCode = VSC_ERR_OUT_OF_MEMORY;
    }
    else
    {
        VIR_Label *  theLabel;

        theLabel = VIR_Function_GetLabelFromId(Function, labelId);
        theLabel->index  = labelId;
        theLabel->defined  = gcvNULL;
        theLabel->referenced = gcvNULL;
        *LabelId = labelId;
    }

    return errCode;
}

VSC_ErrCode
VIR_Function_FreeLabel(
    IN  VIR_Function *  Function,
    IN  VIR_LabelId     Label
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    return errCode;
}


VSC_ErrCode
VIR_Function_AddParameter(
    IN  VIR_Function *  Function,
    IN  gctSTRING       ParamName,
    IN  VIR_TypeId      Type,
    IN  VIR_StorageClass Storage,
    OUT VIR_SymId *     SymId
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    VIR_SymId   symId;

    /* add ParamName to function's symbol table */
    errCode = VIR_Function_AddSymbolWithName(Function,
                                             VIR_SYM_VARIABLE,
                                             ParamName,
                                             VIR_Shader_GetTypeFromId(
                                                    VIR_Function_GetShader(Function),
                                                    Type),
                                             Storage,
                                             &symId);
    CHECK_ERROR(errCode, "AddParameter");

    /* add the symId to parameter list */
    *SymId = symId;

    /* add parameter to function parameter list */
    VIR_IdList_Add(&Function->paramters, symId);
    return errCode;
}

VSC_ErrCode
VIR_Function_AddLocalVar(
    IN  VIR_Function *  Function,
    IN  gctSTRING       VarName,
    IN  VIR_TypeId      Type,
    OUT VIR_SymId *     SymId
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    VIR_SymId   symId;

    /* add ParamName to function's symbol table */
    errCode = VIR_Function_AddSymbolWithName(Function,
                                             VIR_SYM_VARIABLE,
                                             VarName,
                                             VIR_Shader_GetTypeFromId(
                                                    VIR_Function_GetShader(Function),
                                                    Type),
                                             VIR_STORAGE_LOCAL,
                                             &symId);
    CHECK_ERROR(errCode, "AddLocalVar");

    /* add the symId to parameter list */
    *SymId = symId;
      return errCode;
}

#define VIR_OPINFO(OPCODE, OPNDNUM, FLAGS, WRITE2DEST, LEVEL)    {VIR_OP_##OPCODE, OPNDNUM, WRITE2DEST, LEVEL, FLAGS}

const VIR_Opcode_Info VIR_OpcodeInfo[] =
{
#include "vir/ir/gc_vsc_vir_opcode.def.h"
};
#undef VIR_OPINFO

const VIR_Opcode_Info* VIR_Opcode_GetInfo(IN VIR_OpCode opcode)
{
    gcmASSERT(opcode < sizeof(VIR_OpcodeInfo)/sizeof(VIR_Opcode_Info));

    return &VIR_OpcodeInfo[opcode];
}

VSC_ErrCode
VIR_Function_NewLink(
    IN  VIR_Function *  Function,
    OUT VIR_Link **  Link
    )
{
    VSC_ErrCode     errCode = VSC_ERR_NONE;

    VIR_Link *new_link;
    gcmASSERT(Link);

    new_link = (VIR_Link *)vscMM_Alloc(&Function->hostShader->mempool,
                                          sizeof(VIR_Link));
    if (new_link == gcvNULL)
    {
        errCode = VSC_ERR_OUT_OF_MEMORY;
    }
    else
    {
        new_link->referenced = 0;
        new_link->next = gcvNULL;
        *Link = new_link;
    }
    return errCode;
}

VSC_ErrCode
VIR_Function_FreeLink(
    IN  VIR_Function *  Function,
    OUT VIR_Link *      Link
    )
{
    VSC_ErrCode     errCode = VSC_ERR_NONE;
    vscMM_Free(&Function->hostShader->mempool, Link);
    return errCode;
}

VSC_ErrCode
VIR_Function_NewOperand(
    IN  VIR_Function *  Function,
    OUT VIR_Operand **  Operand
    )
{
    VSC_ErrCode     errCode = VSC_ERR_NONE;
    VIR_OperandId   operandId;
    operandId = (VIR_OperandId)vscBT_NewEntry(&Function->operandTable);
    if (VIR_Id_isInvalid(operandId))
    {
        errCode = VSC_ERR_OUT_OF_MEMORY;
    }
    else
    {
        VIR_Operand * opnd = VIR_GetOperandFromId(Function, operandId);
        gcmASSERT(opnd != gcvNULL);
        VIR_Operand_SetOpKind(opnd, VIR_OPND_UNDEF);    /* the operand is not defined yet */
        VIR_Operand_SetIndex(opnd, operandId);
        *Operand = opnd;
    }

    return errCode;
}

VSC_ErrCode
VIR_Function_DupOperand(
    IN  VIR_Function *  Function,
    IN  VIR_Operand *   Src,
    OUT VIR_Operand **  Dup
    )
{
    VSC_ErrCode     errCode = VSC_ERR_NONE;
    gctUINT         index   = 0;
    gcmASSERT(Dup);

    errCode = VIR_Function_NewOperand(Function, Dup);
    if(errCode != VSC_ERR_NONE)
    {
        return errCode;
    }

    index = VIR_Operand_GetIndex(*Dup);
    memcpy(*Dup, Src, sizeof(VIR_Operand));
    VIR_Operand_SetIndex(*Dup, index);

    return errCode;
}

VSC_ErrCode
VIR_Function_NewInstruction(
    IN  VIR_Function *  Function,
    IN  VIR_OpCode      Opcode,
    IN  VIR_TypeId      ResType,
    OUT VIR_Instruction **Inst
    )
{
    VSC_ErrCode     errCode = VSC_ERR_NONE;
    VIR_Instruction *inst;
    gctUINT         srcNum = VIR_OPCODE_GetSrcOperandNum(Opcode);
    inst = (VIR_Instruction *)vscMM_Alloc(&Function->hostShader->mempool,
                                          sizeof(VIR_Instruction));

    gcmASSERT(srcNum <= VIR_MAX_SRC_NUM);
    *Inst = inst;
    if (inst == gcvNULL)
    {
        errCode = VSC_ERR_OUT_OF_MEMORY;
    }
    else
    {
        VIR_Operand * dest;
        VIR_Operand * src;
        gctUINT i;
        memset(inst, 0, sizeof(VIR_Instruction));
        VIR_Inst_SetOpcode(inst, Opcode);
        VIR_Inst_SetSrcNum(inst, srcNum);
        VIR_Inst_SetInstType(inst, ResType);
        VIR_Inst_SetConditionOp(inst, VIR_COP_ALWAYS);
        VIR_Inst_SetFunction(inst, Function);
        VIR_Inst_SetId(inst, VIR_Function_GetAndIncressLastInstId(Function));
        VIR_Inst_SetDual16ExpandSeq(inst, NOT_ASSIGNED);

        /* allocate dest operand */
        if (VIR_OPCODE_hasDest(Opcode))
        {
            errCode = VIR_Function_NewOperand(Function, &dest);
            VIR_Operand_SetLvalue(dest, 1);
            VIR_Inst_SetDest(inst, dest);
        }
        /* allocate source operand */
        for (i=0; i<srcNum; i++)
        {
            errCode = VIR_Function_NewOperand(Function, &src);
            inst->src[i] = src;
        }
        /* reset rest source operands */
        for (i=srcNum; i<VIR_MAX_SRC_NUM; i++)
        {
            inst->src[i] = gcvNULL;
        }
    }
    return errCode;
}

VSC_ErrCode
VIR_Function_AddInstruction(
    IN  VIR_Function *  Function,
    IN  VIR_OpCode      Opcode,
    IN  VIR_TypeId      ResType,
    OUT VIR_Instruction **Inst
    )
{
    VSC_ErrCode         errCode;
    VIR_Instruction *   inst;
    errCode = VIR_Function_NewInstruction(Function, Opcode, ResType, &inst);
    if (errCode == VSC_ERR_NONE)
    {
        *Inst = inst;
        /* link nodes */
        vscBILST_Append((VSC_BI_LIST *)&Function->instList, (VSC_BI_LIST_NODE *)inst);
    }
    return errCode;
}

VSC_ErrCode
VIR_Function_PrependInstruction(
    IN  VIR_Function *  Function,
    IN  VIR_OpCode      Opcode,
    IN  VIR_TypeId      ResType,
    OUT VIR_Instruction **Inst
    )
{
    VSC_ErrCode         errCode;
    VIR_Instruction *   inst;
    errCode = VIR_Function_NewInstruction(Function, Opcode, ResType, &inst);
    if (errCode == VSC_ERR_NONE)
    {
        *Inst = inst;
        /* link nodes */
        vscBILST_Prepend((VSC_BI_LIST *)&Function->instList, (VSC_BI_LIST_NODE *)inst);
    }
    return errCode;
}

VSC_ErrCode
VIR_Function_AddInstructionAfter(
    IN  VIR_Function *  Function,
    IN  VIR_OpCode      Opcode,
    IN  VIR_TypeId      ResType,
    IN  VIR_Instruction *AfterMe,
    OUT VIR_Instruction **Inst
    )
{
    VSC_ErrCode errCode;
    VIR_Instruction * inst;
    errCode = VIR_Function_NewInstruction(Function, Opcode, ResType, &inst);
    if (errCode == VSC_ERR_NONE)
    {
        *Inst = inst;
        /* link nodes */
        vscBILST_InsertAfter((VSC_BI_LIST *)&Function->instList,
                             (VSC_BI_LIST_NODE *)AfterMe,
                             (VSC_BI_LIST_NODE *)inst);

        if (VIR_Inst_GetBasicBlock(AfterMe) &&
            AfterMe->parent.BB->pEndInst == AfterMe)
        {
            BB_SET_END_INST(AfterMe->parent.BB, *Inst);
        }

        if (VIR_Inst_GetBasicBlock(AfterMe))
        {
            VIR_Inst_SetBasicBlock(*Inst, AfterMe->parent.BB);
            BB_INC_LENGTH(AfterMe->parent.BB);
        }
    }
    return errCode;
}

VSC_ErrCode
VIR_Function_AddInstructionBefore(
    IN  VIR_Function *  Function,
    IN  VIR_OpCode      Opcode,
    IN  VIR_TypeId      ResType,
    IN  VIR_Instruction *BeforeMe,
    OUT VIR_Instruction **Inst
    )
{
    VSC_ErrCode errCode;
    VIR_Instruction * inst;
    errCode = VIR_Function_NewInstruction(Function, Opcode, ResType, &inst);
    if (errCode == VSC_ERR_NONE)
    {
        *Inst = inst;
        /* link nodes */
        vscBILST_InsertBefore((VSC_BI_LIST *)&Function->instList,
                             (VSC_BI_LIST_NODE *)BeforeMe,
                             (VSC_BI_LIST_NODE *)inst);

        if (VIR_Inst_GetBasicBlock(BeforeMe) &&
            BeforeMe->parent.BB->pStartInst == BeforeMe)
        {
            BB_SET_START_INST(BeforeMe->parent.BB, *Inst);
        }

        if (VIR_Inst_GetBasicBlock(BeforeMe))
        {
            VIR_Inst_SetBasicBlock(*Inst, BeforeMe->parent.BB);
            BB_INC_LENGTH(BeforeMe->parent.BB);
        }
    }
    return errCode;
}

VSC_ErrCode
VIR_Function_RemoveInstruction(
    IN VIR_Function *   Function,
    IN VIR_Instruction *Inst
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    vscBILST_Remove((VSC_BI_LIST *)&Function->instList, (VSC_BI_LIST_NODE *)Inst);
    if((VIR_Function *)Inst->parent.BB != Function)
    {
        VIR_BB* bb = Inst->parent.BB;
        if(Inst == BB_GET_START_INST(bb) && Inst == BB_GET_END_INST(bb))
        {
            BB_GET_START_INST(bb) = gcvNULL;
            BB_GET_END_INST(bb) = gcvNULL;
        }
        else if(Inst == BB_GET_START_INST(bb))
        {
            BB_GET_START_INST(bb) = VIR_Inst_GetNext(Inst);
        }
        else if(Inst == BB_GET_END_INST(bb))
        {
            BB_GET_END_INST(bb) = VIR_Inst_GetPrev(Inst);
        }
        BB_DEC_LENGTH(bb);
    }
    return errCode;
}

VSC_ErrCode
VIR_Function_MoveInstructionBefore(
    IN  VIR_Function *  Function,
    IN  VIR_Instruction *BeforeMe,
    OUT VIR_Instruction *Inst
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;

    errCode = VIR_Function_RemoveInstruction(Function, Inst);
    if (errCode == VSC_ERR_NONE)
    {
        /* link nodes */
        vscBILST_InsertBefore((VSC_BI_LIST *)&Function->instList,
                             (VSC_BI_LIST_NODE *)BeforeMe,
                             (VSC_BI_LIST_NODE *)Inst);

        if (VIR_Inst_GetBasicBlock(BeforeMe) &&
            BeforeMe->parent.BB->pStartInst == BeforeMe)
        {
            BB_SET_START_INST(BeforeMe->parent.BB, Inst);
        }

        if (VIR_Inst_GetBasicBlock(BeforeMe))
        {
            VIR_Inst_SetBasicBlock(Inst, BeforeMe->parent.BB);
            BB_INC_LENGTH(BeforeMe->parent.BB);
        }
    }

    return errCode;
}

VSC_ErrCode
VIR_Function_FreeOperand(
    IN  VIR_Function *  Function,
    IN  VIR_Operand *   Operand
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    vscBT_RemoveEntry(&Function->operandTable, VIR_Operand_GetIndex(Operand));
    return errCode;
}

VIR_Symbol *
VIR_Function_GetSymFromId(
    IN  VIR_Function *  Function,
    IN  VIR_SymId       SymId
    )
{
    if (VIR_Id_isFunctionScope(SymId))
    {
        return VIR_GetFuncSymFromId(Function, SymId);
    }
    else
    {
        /* the symbol is global, get it from shader symbol table */
        return VIR_Shader_GetSymFromId(Function->hostShader, SymId);
    }
}

void
VIR_Function_SetVirtualInstStart(
    IN VIR_Function *   Func,
    IN gctUINT          StartIndex)
{
}

VIR_Function *
VIR_Inst_GetCallee(VIR_Instruction * Inst)
{
    gcmASSERT(VIR_Inst_GetOpcode(Inst) == VIR_OP_CALL);
    return VIR_Operand_GetFunction(VIR_Inst_GetDest(Inst));
}

VIR_BASIC_BLOCK * VIR_Inst_GetBranchTargetBB(VIR_Instruction * Inst)
{
    VIR_Instruction*        pBranchTargetInst;

    gcmASSERT(VIR_OPCODE_isBranch(VIR_Inst_GetOpcode(Inst)));

    pBranchTargetInst = VIR_Label_GetDefInst(VIR_Operand_GetLabel(Inst->dest));

    return VIR_Inst_GetBasicBlock(pBranchTargetInst);
}

VSC_ErrCode
VIR_Inst_ConstructArg(
    IN OUT VIR_Instruction * Inst,
    IN VIR_Function * Function,
    IN gctUINT ParmIndex,
    IN VIR_Operand* Operand
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;

    VIR_Inst_SetOpcode(Inst, VIR_OP_MOV);
    return errCode;
}

VSC_ErrCode
VIR_Inst_ConstructCall(
    IN OUT VIR_Instruction * Inst,
    IN VIR_Function * Function
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;

    VIR_Inst_SetOpcode(Inst, VIR_OP_CALL);
    return errCode;
}

VSC_ErrCode
VIR_Inst_ConstructRetRetValue(
    IN OUT VIR_Instruction * Inst,
    IN VIR_Function * Callee,
    IN VIR_Function * Caller
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;

    VIR_Inst_SetOpcode(Inst, VIR_OP_MOV);
    return errCode;
}

VSC_ErrCode
VIR_Inst_ConstructRet(
    IN OUT VIR_Instruction * Inst
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;

    VIR_Inst_SetOpcode(Inst, VIR_OP_RET);
    return errCode;
}

gctBOOL
VIR_Inst_Store_Have_Dst(
    IN VIR_Instruction * Inst
    )
{
    VIR_Operand *pOpnd = gcvNULL;
    VIR_OpCode  opcode = VIR_Inst_GetOpcode(Inst);

    if (opcode == VIR_OP_STORE || opcode == VIR_OP_STORE_S ||
        opcode == VIR_OP_STORE_ATTR || opcode == VIR_OP_ATTR_ST ||
        opcode == VIR_OP_IMG_STORE || opcode == VIR_OP_IMG_STORE_3D ||
        opcode == VIR_OP_VX_IMG_STORE)
    {
        /* in v60, USC has constraint. If src2 is immediate/uniform/indirect,
           there must be a store destination. the client needs to check v60 */
        pOpnd = VIR_Inst_GetSource(Inst, 2);
        gcmASSERT(!VIR_Operand_isUndef(pOpnd));

        if (VIR_Operand_GetOpKind(pOpnd) == VIR_OPND_IMMEDIATE)
        {
            return gcvTRUE;
        }

        if (VIR_Operand_GetRelAddrMode(pOpnd) != VIR_INDEXED_NONE)
        {
            return gcvTRUE;
        }

        if (VIR_Operand_GetOpKind(pOpnd) == VIR_OPND_SYMBOL)
        {
            VIR_Symbol *sym = VIR_Operand_GetSymbol(pOpnd);
            if (VIR_Symbol_GetKind(sym) == VIR_SYM_UNIFORM)
            {
                return gcvTRUE;
            }
        }
    }
    else
    {
        /* must be store instruction */
        gcmASSERT(gcvFALSE);
    }

    return gcvFALSE;
}

VSC_ErrCode
VIR_Inst_CopyDest(
    IN OUT VIR_Instruction * Inst,
    IN VIR_Operand *         FromDest,
    IN gctBOOL               KeepOrgType
    )
{
    VSC_ErrCode   errCode = VSC_ERR_NONE;
    VIR_TypeId    tyId;
    VIR_Operand * operand = VIR_Inst_GetDest(Inst);

    gcmASSERT(operand != gcvNULL);
    tyId = VIR_Operand_GetType(operand);
    memcpy(operand, FromDest, sizeof(VIR_Operand));
    operand->header._lvalue = gcvTRUE;

    if (KeepOrgType)
    {
        VIR_Operand_SetType(operand, tyId);
    }
    return errCode;
}

VSC_ErrCode
VIR_Inst_CopySource(
    IN OUT VIR_Instruction * Inst,
    IN gctINT                SrcNum,
    IN VIR_Operand *         FromOperand,
    IN gctBOOL               KeepSrcType   /* keep original source type */
    )
{
    VSC_ErrCode   errCode = VSC_ERR_NONE;
    VIR_TypeId    tyId;
    gctUINT       index;
    VIR_Operand * operand = VIR_Inst_GetSource(Inst, SrcNum);

    gcmASSERT(operand != gcvNULL);
    tyId = VIR_Operand_GetType(operand);
    index = VIR_Operand_GetIndex(operand);
    memcpy(operand, FromOperand, sizeof(VIR_Operand));
    VIR_Operand_SetIndex(operand, index);
    operand->header._lvalue = gcvFALSE;

    if (KeepSrcType)
    {
        VIR_Operand_SetType(operand, tyId);
    }
    return errCode;
}

VSC_ErrCode
VIR_Inst_FreeOperand(
    IN OUT VIR_Instruction * Inst,
    IN gctINT                SrcNum
    )
{
    VIR_Function *  function = VIR_Inst_GetFunction(Inst);
    VIR_Operand *   operand;

    gcmASSERT(function != gcvNULL);
    operand = VIR_Inst_GetSource(Inst, SrcNum);

    return VIR_Function_FreeOperand(function, operand);
}

gctBOOL
VIR_Inst_IdenticalExpression(
    IN VIR_Instruction  *Inst0,
    IN VIR_Instruction  *Inst1,
    IN VIR_Shader       *Shader,
    IN gctBOOL          precisionMatters
    )
{
    if(Inst0 == gcvNULL || Inst1 == gcvNULL)
    {
        return gcvFALSE;
    }

    if(Inst0 == Inst1)
    {
        return gcvTRUE;
    }

    if(VIR_Inst_GetOpcode(Inst0) != VIR_Inst_GetOpcode(Inst1))
    {
        return gcvFALSE;
    }

    if(!VIR_OPCODE_IsExpr(VIR_Inst_GetOpcode(Inst0)) ||
       !VIR_OPCODE_IsExpr(VIR_Inst_GetOpcode(Inst1)))
    {
        return gcvFALSE;
    }

    if(VIR_Inst_GetConditionOp(Inst0) != VIR_Inst_GetConditionOp(Inst1))
    {
        return gcvFALSE;
    }

    if(precisionMatters &&
       VIR_Operand_GetPrecision(VIR_Inst_GetDest(Inst0)) != VIR_Operand_GetPrecision(VIR_Inst_GetDest(Inst1)))
    {
        return gcvFALSE;
    }

    {
        gctUINT i;
        for(i = 0; i < VIR_Inst_GetSrcNum(Inst0); i++)
        {
            if(!VIR_Operand_Identical(VIR_Inst_GetSource(Inst0, i), VIR_Inst_GetSource(Inst1, i), Shader))
            {
                return gcvFALSE;
            }
        }
    }

    return gcvTRUE;
}

VIR_TypeId
VIR_Inst_GetExpressionTypeID(
    IN VIR_Instruction  *Inst,
    IN VIR_Shader       *Shader
    )
{
    VIR_OpCode opcode = VIR_Inst_GetOpcode(Inst);
    VIR_Operand* dst = VIR_Inst_GetDest(Inst);
    VIR_TypeId dstTypeID = VIR_Operand_GetType(dst);
    switch(opcode)
    {
        case VIR_OP_LOAD:
        {
            return VIR_TypeId_ComposeNonOpaqueType(VIR_GetTypeComponentType(dstTypeID), 4, 0);
        }
        case VIR_OP_DP2:
        case VIR_OP_DP3:
        case VIR_OP_DP4:
        {
            return dstTypeID;
        }
        default:
        {
            if(VIR_OPCODE_isComponentwise(opcode))
            {
                return VIR_TypeId_ComposeNonOpaqueType(VIR_GetTypeComponentType(dstTypeID), 4, 0);
            }
            else
            {
                gcmASSERT(0);
                return VIR_TYPE_UNKNOWN;
            }
        }
    }
}

VIR_Precision
VIR_Inst_GetExpectedResultPrecision(
    IN VIR_Instruction  *Inst
    )
{
    VIR_Precision result = VIR_PRECISION_MEDIUM;
    VIR_OpCode opcode = VIR_Inst_GetOpcode(Inst);

    if(VIR_OPCODE_ExpectedResultPrecisionFromHighest(opcode))
    {
        gctUINT i;
        for(i = 0; i < VIR_Inst_GetSrcNum(Inst); i++)
        {
            VIR_Operand* src = VIR_Inst_GetSource(Inst, i);
            VIR_Precision precision = (VIR_Precision)VIR_Operand_GetPrecision(src);
            if(precision == VIR_PRECISION_HIGH)
            {
                result = VIR_PRECISION_HIGH;
                break;
            }
        }
    }
    else if(VIR_OPCODE_ExpectedResultPrecisionFromSrc0(opcode))
    {
        result = (VIR_Precision)VIR_Operand_GetPrecision(VIR_Inst_GetSource(Inst, 0));
    }
    else if(VIR_OPCODE_ExpectedResultPrecisionFromSrc2(opcode))
    {
        result = (VIR_Precision)VIR_Operand_GetPrecision(VIR_Inst_GetSource(Inst, 2));
    }
    else if(VIR_OPCODE_ExpectedResultPrecisionFromHighestInSrc1Src2(opcode))
    {
        if((VIR_Precision)VIR_Operand_GetPrecision(VIR_Inst_GetSource(Inst, 1)) == VIR_PRECISION_HIGH ||
           (VIR_Precision)VIR_Operand_GetPrecision(VIR_Inst_GetSource(Inst, 2)) == VIR_PRECISION_HIGH)
        {
            result = VIR_PRECISION_HIGH;
        }
    }

    return result;
}

/* swizzle */

gctBOOL
VIR_Swizzle_IsEnable(
    IN VIR_Swizzle swizzle
    )
{
    gctUINT i;

    for(i = 0; i < VIR_CHANNEL_COUNT - 1; i++)
    {
        if(VIR_Swizzle_GetChannel(swizzle, i) > VIR_Swizzle_GetChannel(swizzle, i + 1))
        {
            return gcvFALSE;
        }
    }
    return gcvTRUE;
}

VIR_Swizzle
VIR_Swizzle_GetSwizzlingSwizzle(
    IN VIR_Swizzle swizzle1,
    IN VIR_Swizzle swizzle2
    )
{
    VIR_Swizzle result = 0;
    int i, j;

    gcmASSERT(VIR_Enable_Covers(VIR_Swizzle_2_Enable(swizzle1), VIR_Swizzle_2_Enable(swizzle2)));

    if(swizzle1 == swizzle2)
    {
        return VIR_SWIZZLE_XYZW;
    }

    for(i = 0; i < VIR_CHANNEL_COUNT; i++)
    {
        if(VIR_Swizzle_GetChannel(swizzle2, i) == VIR_Swizzle_GetChannel(swizzle1, i))
        {
            VIR_Swizzle_SetChannel(result, i, i);
        }
        else
        {
            for(j = 0; j < VIR_CHANNEL_COUNT; j++)
            {
                if(VIR_Swizzle_GetChannel(swizzle2, i) == VIR_Swizzle_GetChannel(swizzle1, j))
                {
                    VIR_Swizzle_SetChannel(result, i, j);
                    break;
                }
            }
        }
    }

    return result;
}

VIR_Swizzle
VIR_Swizzle_ApplySwizzlingSwizzle(
    IN VIR_Swizzle swizzle,
    IN VIR_Swizzle trans
    )
{
    VIR_Swizzle result = 0;
    int i;

    for(i = 0; i < VIR_CHANNEL_COUNT; i++)
    {
        VIR_Swizzle_SetChannel(result, i, VIR_Swizzle_GetChannel(swizzle, VIR_Swizzle_GetChannel(trans, i)));
    }

    return result;
}

VIR_Swizzle
VIR_Swizzle_GetMappingSwizzle2Swizzle(
    IN VIR_Swizzle swizzle1,
    IN VIR_Swizzle swizzle2
    )
{
    VIR_Swizzle result = VIR_SWIZZLE_XYZW;
    int i;

    for(i = 0; i < VIR_CHANNEL_COUNT; i++)
    {
        VIR_Swizzle_SetChannel(result, VIR_Swizzle_GetChannel(swizzle1, i), VIR_Swizzle_GetChannel(swizzle2, i));
    }

    gcmASSERT(VIR_Swizzle_ApplyMappingSwizzle(swizzle1, result) == swizzle2);

    return result;
}

gctBOOL
VIR_Swizzle_GetMappingSwizzle2Enable(
    IN VIR_Swizzle swizzle,
    IN VIR_Enable enable,
    OUT VIR_Swizzle * mapping_swizzle
    )
{
    VIR_Swizzle result = VIR_SWIZZLE_XYZW;
    gctBOOL set[VIR_CHANNEL_COUNT] = {0};
    int i;
    gctBOOL retValue = gcvTRUE;

    for(i = 0; i < VIR_CHANNEL_COUNT; i++)
    {
        if(enable & (1 << i))
        {
            if(set[VIR_Swizzle_GetChannel(swizzle, i)])
            {
                retValue = gcvFALSE;
            }
            VIR_Swizzle_SetChannel(result, VIR_Swizzle_GetChannel(swizzle, i), i);
            set[VIR_Swizzle_GetChannel(swizzle, i)] = gcvTRUE;
        }
    }

    if(mapping_swizzle)
    {
        *mapping_swizzle = result;
    }

    return retValue;
}

VIR_Swizzle
VIR_Swizzle_MergeMappingSwizzles(
    IN VIR_Swizzle map1,
    IN VIR_Swizzle map2
    )
{
    VIR_Swizzle result = VIR_SWIZZLE_XYZW;
    int i;

    for(i = 0; i < VIR_CHANNEL_COUNT; i++)
    {
        VIR_Swizzle_SetChannel(result, i, VIR_Swizzle_GetChannel(map2, VIR_Swizzle_GetChannel(map1, i)));
    }

    return result;
}

VIR_Swizzle
VIR_Swizzle_ApplyMappingSwizzle(
    IN VIR_Swizzle swizzle,
    IN VIR_Swizzle map
    )
{
    VIR_Swizzle result = 0;
    int i;

    for(i = 0; i < VIR_CHANNEL_COUNT; i++)
    {
        VIR_Swizzle_SetChannel(result, i, VIR_Swizzle_GetChannel(map, VIR_Swizzle_GetChannel(swizzle, i)));
    }

    return result;
}

VIR_Swizzle
VIR_Swizzle_Trim(
    IN VIR_Swizzle swizzle,
    IN VIR_Enable enable
    )
{
    VIR_Swizzle result = swizzle;
    int i, j;

    for(i = 0; i < VIR_CHANNEL_COUNT; i++)
    {
        if(enable & (1 << i))
        {
            break;
        }
    }
    for(j = 0; j < VIR_CHANNEL_COUNT; j++)
    {
        if(!(enable & (1 << j)))
        {
            VIR_Swizzle_SetChannel(result, j, VIR_Swizzle_GetChannel(swizzle, i));
        }
    }

    return result;
}

VIR_Swizzle
VIR_Swizzle_MappingNewSwizzle(
    IN VIR_Enable Enable0,
    IN VIR_Enable Enable1,
    IN VIR_Swizzle Swizzle0,
    IN VIR_Swizzle Swizzle1
    )
{
    VIR_Swizzle result          = VIR_SWIZZLE_X;
    VIR_Swizzle currentSwizzle  = VIR_SWIZZLE_INVALID;
    VIR_Swizzle prevSwizzle     = VIR_SWIZZLE_INVALID;
    VIR_Swizzle swizzle0        = VIR_SWIZZLE_INVALID;
    VIR_Swizzle swizzle1        = VIR_SWIZZLE_INVALID;
    gctINT      i, j;

    for (i = 0; i < VIR_CHANNEL_COUNT; i++)
    {
        currentSwizzle  = VIR_SWIZZLE_INVALID;

        if (Enable0 & (1 << i))
        {
            swizzle0 = VIR_Swizzle_GetChannel(Swizzle0, i);

            for (j = 0; j < VIR_CHANNEL_COUNT; j++)
            {
                if (Enable1 & (1 << j))
                {
                    swizzle1 = VIR_Swizzle_GetChannel(Swizzle1, j);

                    if (swizzle0 == swizzle1)
                    {
                        currentSwizzle = (VIR_Swizzle)j;
                        break;
                    }
                }
            }
            gcmASSERT(currentSwizzle != VIR_SWIZZLE_INVALID);

            /* Update the swizzle .*/
            VIR_Swizzle_SetChannel(result, i, currentSwizzle);
            if (prevSwizzle == VIR_SWIZZLE_INVALID)
            {
                for (j = 0; j < i; j++)
                {
                    VIR_Swizzle_SetChannel(result, j, currentSwizzle);
                }
            }
            prevSwizzle = currentSwizzle;
        }
        else if (prevSwizzle != VIR_SWIZZLE_INVALID)
        {
            VIR_Swizzle_SetChannel(result, i, prevSwizzle);
        }
    }

    return result;
}

/* enable */

VIR_Swizzle
VIR_Enable_2_Swizzle(
    IN VIR_Enable enable
    )
{
    VIR_Swizzle result = 0;
    int i;
    int channel = 0;

    for(i = 0; i < VIR_CHANNEL_COUNT; i++)
    {
        if(enable & (1 << i))
        {
            VIR_Swizzle_SetChannel(result, channel, i);
            channel++;
        }
    }
    for(; channel < VIR_CHANNEL_COUNT; channel++)
    {
        VIR_Swizzle_SetChannel(result, channel, VIR_Swizzle_GetChannel(result, channel - 1));
    }

    return result;
}

VIR_Swizzle
VIR_Enable_2_Swizzle_WShift(
    IN VIR_Enable Enable
    )
{
    switch (Enable)
    {
    case VIR_ENABLE_NONE:
    case VIR_ENABLE_X:
        return VIR_SWIZZLE_XXXX;
    case VIR_ENABLE_Y:
        return VIR_SWIZZLE_YYYY;
    case VIR_ENABLE_Z:
        return VIR_SWIZZLE_ZZZZ;
    case VIR_ENABLE_W:
        return VIR_SWIZZLE_WWWW;
    case VIR_ENABLE_XY:
        return VIR_SWIZZLE_XYYY;
    case VIR_ENABLE_XZ:
        return VIR_SWIZZLE_XZZZ;
    case VIR_ENABLE_XW:
        return VIR_SWIZZLE_XWWW;
    case VIR_ENABLE_YZ:
        return VIR_SWIZZLE_YYZZ;
    case VIR_ENABLE_YW:
        return VIR_SWIZZLE_YYWW;
    case VIR_ENABLE_ZW:
        return VIR_SWIZZLE_ZZZW;
    case VIR_ENABLE_XYZ:
        return VIR_SWIZZLE_XYZZ;
    case VIR_ENABLE_XYW:
        return VIR_SWIZZLE_XYWW;
    case VIR_ENABLE_XZW:
        return VIR_SWIZZLE_XZZW;
    case VIR_ENABLE_YZW:
        return VIR_SWIZZLE_YYZW;
    case VIR_ENABLE_XYZW:
        return VIR_SWIZZLE_XYZW;
    default:
        break;
    }

    gcmASSERT(0);
    return VIR_SWIZZLE_XYZW;
}

VIR_Swizzle
VIR_Enable_GetMappingSwizzle(
    IN VIR_Enable enable,
    IN VIR_Swizzle swizzle
    )
{
    VIR_Swizzle result = VIR_SWIZZLE_XYZW;
    int i;

    for(i = 0; i < VIR_CHANNEL_COUNT; i++)
    {
        if(enable & (1 << i))
        {
            VIR_Swizzle_SetChannel(result, i, VIR_Swizzle_GetChannel(swizzle, i));
        }
    }

    return result;
}

VIR_Enable
VIR_Enable_ApplyMappingSwizzle(
    IN VIR_Enable enable,
    IN VIR_Swizzle mappingSwizzle
    )
{
    VIR_Enable result = 0;
    int i;

    for(i = 0; i < VIR_CHANNEL_COUNT; i++)
    {
        if(enable & (1 << i))
        {
            result |= (1 << VIR_Swizzle_GetChannel(mappingSwizzle, i));
        }
    }

    return result;
}

void
VIR_Operand_GetOperandInfo(
    IN  VIR_Instruction *   Inst,
    IN  VIR_Operand *       Operand,
    OUT VIR_OperandInfo *   Info)
{
    VIR_Symbol *        sym;
    VIR_Symbol *        virregSym = gcvNULL;
    VIR_OperandKind     opndKind = VIR_Operand_GetOpKind(Operand);
    VIR_Shader *        Shader   = VIR_Inst_GetShader(Inst);
    gctBOOL             needIndexRange = gcvFALSE;
    gcmASSERT(Info != gcvNULL);
    gcmASSERT(VIR_Operand_IsOwnerInst(Operand, Inst));
    memset(Info, 0, sizeof(VIR_OperandInfo));
    Info->opnd  = Operand;
    if ((VIR_Inst_GetOpcode(Inst) == VIR_OP_LDARR &&
         Operand == VIR_Inst_GetSource(Inst, 0) ) ||
        (VIR_Inst_GetOpcode(Inst) == VIR_OP_STARR &&
         Operand == VIR_Inst_GetDest(Inst) ) )
    {
        needIndexRange = gcvTRUE;
    }

    /* TODO: For dual16 case, set correct half-channel mask */
    Info->halfChannelMask = VIR_HALF_CHANNEL_MASK_FULL;

    if (opndKind == VIR_OPND_SYMBOL || opndKind == VIR_OPND_SAMPLER_INDEXING)
    {
        sym = VIR_Operand_GetSymbol(Operand);
        if (VIR_Symbol_GetKind(sym) == VIR_SYM_VIRREG)
        {
            Info->u1.virRegInfo.virReg = VIR_Symbol_GetVregIndex(sym);
            virregSym = sym;
            sym = VIR_Symbol_GetVregVariable(sym); /* set the sym to corresponding variable */
        }
        else if (VIR_Symbol_isVariable(sym))
        {
            Info->u1.virRegInfo.virReg = VIR_Symbol_GetVariableVregIndex(sym);
        }
        else if (VIR_Symbol_isUniform(sym))
        {
            Info->isUniform = 1;
            Info->u1.uniformIdx = sym->u2.uniform->index;
        }
        else
        {
            Info->u1.virRegInfo.virReg = VIR_INVALID_ID;
        }

        if (Info->u1.virRegInfo.virReg != VIR_INVALID_ID && !Info->isUniform)
        {
            if (sym != gcvNULL)
            {
                if (VIR_Symbol_isVariable(sym))
                {
                    Info->u1.virRegInfo.startVirReg   = VIR_Symbol_GetVariableVregIndex(sym);
                    Info->u1.virRegInfo.virRegCount =
                        needIndexRange ? VIR_Type_GetVirRegCount(Shader, VIR_Symbol_GetType(sym)) : 1;
                }
                else
                {
                    /* get the variable of struct type which this field is in */
                    gcmASSERT(VIR_Symbol_isField(sym) && virregSym != gcvNULL);
                    Info->u1.virRegInfo.startVirReg   =
                        Info->u1.virRegInfo.virReg - VIR_Symbol_GetOffsetInVar(virregSym);
                    Info->u1.virRegInfo.virRegCount = needIndexRange ?
                        (VIR_Symbol_GetIndexRange(virregSym) - Info->u1.virRegInfo.startVirReg) : 1;

                    /* !!Need to be removed!! */
                    if (Info->u1.virRegInfo.virRegCount & 0x80000000)
                    {
                        Info->u1.virRegInfo.virRegCount = 1;
                    }
                }

                /* Out sample-mask needs hw special def */
                if (VIR_Symbol_GetName(sym) == VIR_NAME_SAMPLE_MASK)
                {
                    Info->needHwSpecialDef = 1;
                }

                Info->isArray       =
                    VIR_Type_GetKind(VIR_Symbol_GetType(sym)) == VIR_TY_ARRAY ? 1 : 0;
                Info->isInput       = VIR_Symbol_isInput(sym) || VIR_Symbol_isPerPatchInput(sym);
                Info->isOutput      = VIR_Symbol_isOutput(sym) || VIR_Symbol_isPerPatchOutput(sym);
                Info->isTempVar     = 0;
                Info->isImmVal      = 0;
            }
            else
            {
                /* is temp variable which has no coresponding user defined variable  */
                Info->u1.virRegInfo.startVirReg   = Info->u1.virRegInfo.virReg;
                Info->isArray       = Info->isInput = Info->isOutput = 0;
                Info->isTempVar     = 1;
                Info->u1.virRegInfo.virRegCount   = 1;
                Info->isImmVal      = 0;
            }
            if (opndKind == VIR_OPND_SAMPLER_INDEXING)
            {
                Info->indexingKind = VIR_OPND_SAMPLERINDEXING;
            }
            else
            {
                Info->indexingKind = VIR_OPND_NOINDEXING;
            }
        }
    }
    else if (opndKind == VIR_OPND_IMMEDIATE)
    {
        Info->isImmVal      = 1;
        Info->u1.immValue.uValue = VIR_Operand_GetImmediateUint(Operand);
    }
    else if (opndKind == VIR_OPND_CONST)
    {
        Info->isVecConst      = 1;
        Info->u1.vecConstId   =  VIR_Operand_GetConstId(Operand);
    }
    else
    {
        Info->u1.virRegInfo.virReg = VIR_INVALID_ID;
        Info->isImmVal      = 0;
        Info->isVecConst    = 0;
        Info->isUniform     = 0;
    }
    return;
}


gctBOOL VIR_Operand_isInputVariable(VIR_Operand * Operand)
{
    gctBOOL isInput = gcvFALSE;
    if (VIR_Operand_GetOpKind(Operand) == VIR_OPND_SYMBOL)
    {
        VIR_Symbol * sym = VIR_Operand_GetSymbol(Operand);
        isInput = VIR_Symbol_isInput(sym);
    }
    else if (VIR_Operand_GetOpKind(Operand) == VIR_OPND_VIRREG)
    {
        VIR_Symbol * sym = VIR_Operand_GetSymbol(Operand);
        VIR_Symbol * varSym = VIR_Symbol_GetVregVariable(sym);
        isInput = VIR_Symbol_isInput(varSym);
    }
    return isInput;
}

VIR_Symbol *
VIR_Operand_GetUnderlyingSymbol(
    IN VIR_Operand * Operand
    )
{
    VIR_Symbol *        sym = gcvNULL;
    VIR_OperandKind     opndKind = VIR_Operand_GetOpKind(Operand);
    if (opndKind == VIR_OPND_VIRREG)
    {
        sym = VIR_Symbol_GetVregVariable(VIR_Operand_GetSymbol(Operand));
    }
    else if (opndKind == VIR_OPND_SYMBOL ||
             opndKind == VIR_OPND_SAMPLER_INDEXING)
    {
        sym = VIR_Operand_GetSymbol(Operand);
        if (VIR_Symbol_GetKind(sym) == VIR_SYM_VIRREG)
        {
            /* set the sym to corresponding variable */
            sym = VIR_Symbol_GetVregVariable(sym);
        }
    }
    return sym;
}

/* set operands */

void
VIR_Operand_SetSymbol(
    IN OUT VIR_Operand*    Operand,
    IN     VIR_Function *  Function,
    IN     VIR_SymId       SymId
    )
{
    VIR_Symbol * sym    = VIR_Function_GetSymFromId(Function, SymId);
    VIR_Operand_SetOpKind(Operand, VIR_OPND_SYMBOL);
    VIR_Operand_SetType(Operand, VIR_Type_GetIndex(sym->type));
    VIR_Operand_SetSym(Operand, sym);
}

void
VIR_Operand_SetImmediate(
    IN OUT VIR_Operand*    Operand,
    IN  VIR_TypeId         Type,
    IN  VIR_ScalarConstVal Immed
    )
{
    VIR_Operand_SetOpKind(Operand, VIR_OPND_IMMEDIATE);
    VIR_Operand_SetType(Operand, Type);
    Operand->u1.uConst  = Immed.uValue;
}

void
VIR_Operand_SetConst(
    IN OUT VIR_Operand *Operand,
    IN  VIR_TypeId      Type,
    IN  VIR_ConstId     ConstId
    )
{
    VIR_Operand_SetOpKind(Operand, VIR_OPND_CONST);
    VIR_Operand_SetType(Operand, Type);
    Operand->u1.constId = ConstId;
}

void
VIR_Operand_SetUniform(
    IN OUT VIR_Operand *Operand,
    IN  VIR_Uniform *   Uniform,
    IN  VIR_Shader *    Shader
    )
{
    VIR_Symbol* sym = VIR_Shader_GetSymFromId(Shader, Uniform->sym);
    VIR_Operand_SetOpKind(Operand, VIR_OPND_SYMBOL);
    VIR_Operand_SetType(Operand, VIR_Type_GetIndex(VIR_Symbol_GetType(sym)));
    Operand->u1.sym = sym;
}

void
VIR_Operand_SetParameters(
    IN OUT VIR_Operand *Operand,
    IN  VIR_ParmPassing *Parms
    )
{
    VIR_Operand_SetOpKind(Operand, VIR_OPND_PARAMETERS);
    VIR_Operand_SetType(Operand, VIR_TYPE_UNKNOWN);
    Operand->u1.argList = Parms;
}

void
VIR_Operand_SetLabel(
    IN OUT VIR_Operand *    Operand,
    IN  VIR_Label *         Label
    )
{
    VIR_Operand_SetOpKind(Operand, VIR_OPND_LABEL);
    VIR_Operand_SetType(Operand, VIR_TYPE_UNKNOWN);
    Operand->u1.label  = Label;
}

void
 VIR_Operand_SetImmediateInt(
     IN OUT VIR_Operand *    Operand,
     IN gctINT              Val
     )
{
    VIR_Operand_SetOpKind(Operand, VIR_OPND_IMMEDIATE);
    VIR_Operand_SetType(Operand, VIR_TYPE_INT32);
    Operand->u1.iConst = Val;
}

void
 VIR_Operand_SetImmediateUint(
     IN OUT VIR_Operand *    Operand,
     IN gctUINT              Val
     )
{
    VIR_Operand_SetOpKind(Operand, VIR_OPND_IMMEDIATE);
    VIR_Operand_SetType(Operand, VIR_TYPE_UINT32);
    Operand->u1.uConst = Val;
}

void
 VIR_Operand_SetImmediateFloat(
     IN OUT VIR_Operand *    Operand,
     IN gctFLOAT             Val
     )
{
    VIR_Operand_SetOpKind(Operand, VIR_OPND_IMMEDIATE);
    VIR_Operand_SetType(Operand, VIR_TYPE_FLOAT32);
    Operand->u1.fConst = Val;
}

void
VIR_Operand_SetTexldBias(
    IN OUT VIR_Operand *    Operand,
    IN  VIR_Operand    *    Bias
    )
{
    VIR_Operand_TexldModifier * texldModifier = (VIR_Operand_TexldModifier *)Operand;
    gcmASSERT(VIR_Operand_GetOpKind(Operand) == VIR_OPND_UNDEF ||
              VIR_Operand_GetOpKind(Operand) == VIR_OPND_TEXLDPARM);
    VIR_Operand_SetOpKind(Operand, VIR_OPND_TEXLDPARM);
    VIR_Operand_SetTexModifierFlag(texldModifier, VIR_TMFLAG_BIAS);
    texldModifier->tmodifier[VIR_TEXLDMODIFIER_BIAS] = Bias;
}

void
VIR_Operand_SetTexldLod(
    IN OUT VIR_Operand *    Operand,
    IN  VIR_Operand    *    Lod
    )
{
    VIR_Operand_TexldModifier * texldModifier = (VIR_Operand_TexldModifier *)Operand;
    gcmASSERT(VIR_Operand_GetOpKind(Operand) == VIR_OPND_UNDEF ||
              VIR_Operand_GetOpKind(Operand) == VIR_OPND_TEXLDPARM);
    VIR_Operand_SetOpKind(Operand, VIR_OPND_TEXLDPARM);
    VIR_Operand_SetTexModifierFlag(texldModifier, VIR_TMFLAG_LOD);
    texldModifier->tmodifier[VIR_TEXLDMODIFIER_LOD] = Lod;
}

void
VIR_Operand_SetTexldGradient(
    IN OUT VIR_Operand *    Operand,
    IN  VIR_Operand    *    Pdx,
    IN  VIR_Operand    *    Pdy
    )
{
    VIR_Operand_TexldModifier * texldModifier = (VIR_Operand_TexldModifier *)Operand;
    gcmASSERT(VIR_Operand_GetOpKind(Operand) == VIR_OPND_UNDEF ||
              VIR_Operand_GetOpKind(Operand) == VIR_OPND_TEXLDPARM);
    VIR_Operand_SetOpKind(Operand, VIR_OPND_TEXLDPARM);
    VIR_Operand_SetTexModifierFlag(texldModifier, VIR_TMFLAG_GRAD);
    texldModifier->tmodifier[VIR_TEXLDMODIFIER_DPDX] = Pdx;
    texldModifier->tmodifier[VIR_TEXLDMODIFIER_DPDY] = Pdy;
}

void
VIR_Operand_SetTexldOffset(
    IN OUT VIR_Operand *    Operand,
    IN  VIR_Operand    *    Offset
    )
{
    VIR_Operand_TexldModifier * texldModifier = (VIR_Operand_TexldModifier *)Operand;
    gcmASSERT(VIR_Operand_GetOpKind(Operand) == VIR_OPND_UNDEF ||
              VIR_Operand_GetOpKind(Operand) == VIR_OPND_TEXLDPARM);
    VIR_Operand_SetOpKind(Operand, VIR_OPND_TEXLDPARM);
    VIR_Operand_SetTexModifierFlag(texldModifier, VIR_TMFLAG_OFFSET);
    texldModifier->tmodifier[VIR_TEXLDMODIFIER_OFFSET] = Offset;
}

void
VIR_Operand_SetTexldGather(
    IN OUT VIR_Operand *    Operand,
    IN  VIR_Operand    *    Component,
    IN  VIR_Operand    *    RefZ
    )
{
    VIR_Operand_TexldModifier * texldModifier = (VIR_Operand_TexldModifier *)Operand;
    gcmASSERT(VIR_Operand_GetOpKind(Operand) == VIR_OPND_UNDEF ||
              VIR_Operand_GetOpKind(Operand) == VIR_OPND_TEXLDPARM);
    VIR_Operand_SetOpKind(Operand, VIR_OPND_TEXLDPARM);
    VIR_Operand_SetTexModifierFlag(texldModifier, VIR_TMFLAG_GATHER);
    texldModifier->tmodifier[VIR_TEXLDMODIFIER_GATHERCOMP] = Component;
    texldModifier->tmodifier[VIR_TEXLDMODIFIER_GATHERREFZ] = RefZ;
}

void
VIR_Operand_SetTexldFetchMS(
    IN OUT VIR_Operand *    Operand,
    IN  VIR_Operand    *    Sample
    )
{
    VIR_Operand_TexldModifier * texldModifier = (VIR_Operand_TexldModifier *)Operand;
    gcmASSERT(VIR_Operand_GetOpKind(Operand) == VIR_OPND_UNDEF ||
              VIR_Operand_GetOpKind(Operand) == VIR_OPND_TEXLDPARM);
    VIR_Operand_SetOpKind(Operand, VIR_OPND_TEXLDPARM);
    VIR_Operand_SetTexModifierFlag(texldModifier, VIR_TMFLAG_FETCHMS);
    texldModifier->tmodifier[VIR_TEXLDMODIFIER_FETCHMS_SAMPLE] = Sample;
}

void
VIR_Operand_SetFunction(
    IN OUT VIR_Operand *Operand,
    IN  VIR_Function *  Function
    )
{
    VIR_Operand_SetOpKind(Operand, VIR_OPND_FUNCTION);
    VIR_Operand_SetType(Operand, VIR_TYPE_UNKNOWN);
    VIR_Operand_SetFunc(Operand, Function);
}

void
VIR_Operand_SetIntrinsic(
    IN OUT VIR_Operand *    Operand,
    IN  VIR_IntrinsicsKind  Intrinsic
    )
{
    VIR_Operand_SetOpKind(Operand, VIR_OPND_INTRINSIC);
    VIR_Operand_SetType(Operand, VIR_TYPE_UNKNOWN);
    Operand->u1.intrinsic  = Intrinsic;
}

void
VIR_Operand_SetFieldAccess(
    IN OUT VIR_Operand *Operand,
    IN  VIR_Operand *   Base,
    IN  VIR_SymId       FieldId
    )
{
    VIR_Operand_SetOpKind(Operand, VIR_OPND_FIELD);
    VIR_Operand_SetType(Operand, VIR_TYPE_UNKNOWN);
    Operand->u1.base    = Base;
    Operand->u2.fieldId = FieldId;
}

void
VIR_Operand_SetArrayIndexing(
    IN OUT VIR_Operand * Operand,
    IN  VIR_Operand *   Base,
    IN  VIR_OperandList* ArrayIndex
    )
{
    VIR_Operand_SetOpKind(Operand, VIR_OPND_ARRAY);
    VIR_Operand_SetType(Operand, VIR_TYPE_UNKNOWN);
    Operand->u1.base    = Base;
    Operand->u2.arrayIndex  = ArrayIndex;
}

void
VIR_Operand_SetSamplerIndexing(
    IN OUT VIR_Operand * Operand,
    IN VIR_Shader *      Shader,
    IN  VIR_Symbol *     SamplerIndexing
    )
{
    VIR_Operand_SetOpKind(Operand, VIR_OPND_SAMPLER_INDEXING);
    VIR_Operand_SetSym(Operand, SamplerIndexing);
    VIR_Shader_SetFlag(Shader, VIR_SHFLAG_HAS_SAMPLER_INDEXING);
}


void
VIR_Operand_SetTempRegister(
    IN OUT VIR_Operand *    Operand,
    IN     VIR_Function *   Function,
    IN     VIR_SymId        TempSymId,
    IN     VIR_TypeId       OpernadType
    )
{
    VIR_Symbol * sym    = VIR_Function_GetSymFromId(Function, TempSymId);
    VIR_Operand_SetOpKind(Operand, VIR_OPND_SYMBOL);
    VIR_Operand_SetType(Operand, OpernadType);
    VIR_Operand_SetSym(Operand, sym);
}

/* for source operand only */
void
VIR_Operand_SetSwizzle(
    IN OUT VIR_Operand *Operand,
    IN  VIR_Swizzle     Swizzle
    )
{
    gcmASSERT(!VIR_Operand_isLvalue(Operand)); /* must be rvalue */
    Operand->_swizzleOrEnable  = (gctUINT)Swizzle;
}

void
VIR_Operand_ShrinkSwizzle(
    IN OUT VIR_Operand *Operand,
    IN  VIR_Swizzle     Swizzle
    )
{
    gcmASSERT(!VIR_Operand_isLvalue(Operand)); /* must be rvalue */
    /* Swizzle must be a subset of Operand's original one */
    gcmASSERT(VIR_Swizzle_Covers(VIR_Operand_GetSwizzle(Operand), Swizzle));
    if(VIR_Operand_GetSwizzle(Operand) != Swizzle)
    {
        Operand->_swizzleOrEnable  = (gctUINT)Swizzle;
    }
}

/* for  dest operand only */
void
VIR_Operand_SetEnable(
    IN OUT VIR_Operand *Operand,
    IN  VIR_Enable      Enable
    )
{
    gcmASSERT(VIR_Operand_isLvalue(Operand)); /* must be lvalue */
    Operand->_swizzleOrEnable  = (gctUINT)Enable;
}

void
VIR_Operand_ShrinkEnable(
    IN OUT VIR_Operand *Operand,
    IN  VIR_Enable      Enable
    )
{
    gcmASSERT(VIR_Operand_isLvalue(Operand)); /* must be lvalue */
    /* Enable must be a subset of Operand's original one */
    gcmASSERT(VIR_Enable_Covers(VIR_Operand_GetEnable(Operand), Enable));
    if(VIR_Operand_GetEnable(Operand) != Enable)
    {
        Operand->_swizzleOrEnable  = (gctUINT)Enable;
    }
}

void
VIR_Operand_SetRelIndexing(
    IN OUT VIR_Operand *Operand,
    IN VIR_SymId        IndexSym
    )
{
    gcmASSERT(!VIR_Operand_isHighLevel(Operand));
    Operand->u2.vlInfo._isConstIndexing = 0;
    Operand->u2.vlInfo._relIndexing = IndexSym;
}

void
VIR_Operand_SetRelIndexingImmed(
    IN OUT VIR_Operand *Operand,
    IN gctINT           IndexImmed
    )
{
    gcmASSERT(!VIR_Operand_isHighLevel(Operand));
    gcmASSERT((IndexImmed < INT20_MAX) && (IndexImmed > INT20_MIN));

    Operand->u2.vlInfo._isConstIndexing = 1;
    Operand->u2.vlInfo._relIndexing = IndexImmed;
}


gctBOOL
VIR_Operand_IsNegatable(
    IN  VIR_Shader *        Shader,
    IN  VIR_Operand *       Operand)
{
    VIR_TypeId tid;
    VIR_Type* type;

    tid = VIR_Operand_GetType(Operand);
    type = VIR_Shader_GetTypeFromId(Shader, tid);

    gcmASSERT((VIR_Type_GetFlags(type) & VIR_TYFLAG_ISINTEGER)
        || (VIR_Type_GetFlags(type) & VIR_TYFLAG_ISFLOAT));

    if(VIR_Type_GetFlags(type) & VIR_TYFLAG_ISFLOAT)
    {
        return gcvTRUE;
    }

    return (VIR_Operand_GetOpKind(Operand) == VIR_OPND_IMMEDIATE)
        || (VIR_Operand_GetOpKind(Operand) == VIR_OPND_CONST);
}

void
VIR_Operand_NegateOperand(
    IN  VIR_Shader *        Shader,
    IN  VIR_Operand *       Operand
    )
{
    switch(VIR_Operand_GetOpKind(Operand))
    {
        case VIR_OPND_IMMEDIATE:
        {
            VIR_PrimitiveTypeId type = VIR_Operand_GetType(Operand);
            VIR_ScalarConstVal_GetNeg(type, (VIR_ScalarConstVal*)&(Operand->u1), (VIR_ScalarConstVal*)&(Operand->u1));
            break;
        }
        case VIR_OPND_SYMBOL:
            if(VIR_Operand_GetModifier(Operand) & VIR_MOD_NEG)
            {
                VIR_Operand_SetModifier(Operand, VIR_MOD_NEG ^ VIR_Operand_GetModifier(Operand));
            }
            else
            {
                VIR_Operand_SetModifier(Operand, VIR_MOD_NEG | VIR_Operand_GetModifier(Operand));
            }
            break;
        case VIR_OPND_CONST:
        {
            VIR_PrimitiveTypeId type = VIR_Operand_GetType(Operand);
            VIR_Const* cur_const = (VIR_Const *)VIR_Shader_GetSymFromId(Shader, Operand->u1.constId);
            VIR_ConstVal new_const;
            VIR_ConstId new_const_id;

            memset(&new_const, 0, sizeof(VIR_ConstVal));

            VIR_VecConstVal_GetNeg(type, &cur_const->value.vecVal, &new_const.vecVal);
            VIR_Shader_AddConstant(Shader, type, &new_const, &new_const_id);
            VIR_Operand_SetConstId(Operand, new_const_id);
            break;
        }
        default:
            gcmASSERT(gcvFALSE);
    }
}

gctBOOL
VIR_Operand_SameLocation(
    IN  VIR_Instruction *   Inst1,
    IN  VIR_Operand *       Operand1,
    IN  VIR_Instruction *   Inst2,
    IN  VIR_Operand *       Operand2)
{
    VIR_OperandInfo op1_info, op2_info;

    if(Operand1 == Operand2)
    {
        return gcvTRUE;
    }

    if(Operand1 == gcvNULL || Operand2 == gcvNULL)
    {
        return gcvFALSE;
    }

    VIR_Operand_GetOperandInfo(Inst1, Operand1, &op1_info);
    VIR_Operand_GetOperandInfo(Inst2, Operand2, &op2_info);

    if(op1_info.indexingKind != VIR_OPND_NOINDEXING
        || op2_info.indexingKind != VIR_OPND_NOINDEXING)
    {
        return gcvTRUE;
    }

    if((op1_info.isImmVal || op1_info.isVecConst) ||
       (op2_info.isImmVal || op2_info.isVecConst))
    {
        return gcvFALSE;
    }

    if(op1_info.u1.virRegInfo.virReg == op2_info.u1.virRegInfo.virReg)
    {
        VIR_Enable enable1, enable2;
        enable1 = VIR_Operand_isLvalue(Operand1) ? VIR_Operand_GetEnable(Operand1) : VIR_Swizzle_2_Enable(VIR_Operand_GetSwizzle(Operand1));
        enable2 = VIR_Operand_isLvalue(Operand2) ? VIR_Operand_GetEnable(Operand2) : VIR_Swizzle_2_Enable(VIR_Operand_GetSwizzle(Operand2));
        if(enable1 & enable2)
        {
            return gcvTRUE;
        }
    }

    return gcvFALSE;
}

gctBOOL
VIR_Operand_SameSymbol(
    IN VIR_Operand  *Opnd0,
    IN VIR_Operand  *Opnd1
    )
{
    if(Opnd0 == Opnd1)
    {
        return gcvTRUE;
    }

    if(Opnd0 == NULL || Opnd1 == NULL)
    {
        return gcvFALSE;
    }

    if(VIR_Operand_GetOpKind(Opnd0) != VIR_Operand_GetOpKind(Opnd1))
    {
        return gcvFALSE;
    }

    switch(VIR_Operand_GetOpKind(Opnd0))
    {
    case VIR_OPND_SYMBOL:
    case VIR_OPND_SAMPLER_INDEXING:
        {
            VIR_Symbol *sym0  = VIR_Operand_GetSymbol(Opnd0);
            VIR_Symbol *sym1  = VIR_Operand_GetSymbol(Opnd1);

            return sym0 == sym1;
        }
    default:
        return gcvFALSE;
    }
}

gctBOOL
VIR_Operand_Identical(
    IN VIR_Operand  *Opnd0,
    IN VIR_Operand  *Opnd1,
    IN VIR_Shader   *Shader
    )
{
    if(Opnd0 == Opnd1)
    {
        return gcvTRUE;
    }

    if(Opnd0 == NULL || Opnd1 == NULL)
    {
        return gcvFALSE;
    }

    if(VIR_Operand_GetOpKind(Opnd0) == VIR_Operand_GetOpKind(Opnd1))
    {

        switch(VIR_Operand_GetOpKind(Opnd0))
        {
            case VIR_OPND_SYMBOL:
            case VIR_OPND_VIRREG:
            case VIR_OPND_SAMPLER_INDEXING:
            {
                VIR_Symbol *sym0  = VIR_Operand_GetSymbol(Opnd0);
                VIR_Symbol *sym1  = VIR_Operand_GetSymbol(Opnd1);

                return sym0 == sym1 &&
                       VIR_Operand_GetSwizzle(Opnd0) == VIR_Operand_GetSwizzle(Opnd1) &&
                       VIR_Operand_GetConstIndexingImmed(Opnd0) == VIR_Operand_GetConstIndexingImmed(Opnd1) &&
                       VIR_Operand_GetMatrixConstIndex(Opnd0) == VIR_Operand_GetMatrixConstIndex(Opnd1) &&
                       VIR_Operand_GetRelAddrMode(Opnd0) == VIR_Operand_GetRelAddrMode(Opnd1) &&
                       VIR_Operand_GetRelIndexing(Opnd0) == VIR_Operand_GetRelIndexing(Opnd1);
            }
            case VIR_OPND_IMMEDIATE:
            {
                return VIR_Operand_GetType(Opnd0) == VIR_Operand_GetType(Opnd1) &&
                       VIR_Operand_GetImmediateUint(Opnd0) == VIR_Operand_GetImmediateUint(Opnd1);
            }
            case VIR_OPND_CONST:
            {
                gctUINT channel;
                VIR_ConstId constID0 = VIR_Operand_GetConstId(Opnd0);
                VIR_ConstId constID1 = VIR_Operand_GetConstId(Opnd1);
                VIR_Const* const0 = VIR_Shader_GetConstFromId(Shader, constID0);
                VIR_Const* const1 = VIR_Shader_GetConstFromId(Shader, constID1);
                VIR_Swizzle swizzle0 = VIR_Operand_GetSwizzle(Opnd0);
                VIR_Swizzle swizzle1 = VIR_Operand_GetSwizzle(Opnd1);

                if(VIR_GetTypeComponentType(VIR_Operand_GetType(Opnd0)) != VIR_GetTypeComponentType(VIR_Operand_GetType(Opnd1)))
                {
                    return gcvFALSE;
                }

                for(channel = 0; channel < VIR_CHANNEL_NUM; channel++)
                {
                    if(const0->value.vecVal.u32Value[VIR_Swizzle_GetChannel(swizzle0, channel)] !=
                       const1->value.vecVal.u32Value[VIR_Swizzle_GetChannel(swizzle1, channel)])
                    {
                        return gcvFALSE;
                    }
                }
                return gcvTRUE;
            }
            case VIR_OPND_UNDEF:
            default:
                return gcvFALSE;
        }
    }
    else
    {
        VIR_Operand* immOpnd;
        VIR_Operand* constOpnd;
        if(VIR_Operand_GetOpKind(Opnd0) == VIR_OPND_IMMEDIATE &&
           VIR_Operand_GetOpKind(Opnd1) == VIR_OPND_CONST)
        {
            immOpnd = Opnd0;
            constOpnd = Opnd1;
        }
        else if(VIR_Operand_GetOpKind(Opnd0) == VIR_OPND_CONST &&
                VIR_Operand_GetOpKind(Opnd1) == VIR_OPND_IMMEDIATE)
        {
            constOpnd = Opnd0;
            immOpnd = Opnd1;
        }
        else
        {
            return gcvFALSE;
        }

        {
            VIR_Swizzle swizzle = VIR_Operand_GetSwizzle(constOpnd);
            VIR_ConstId constID = VIR_Operand_GetConstId(constOpnd);
            VIR_Const* constVal = VIR_Shader_GetConstFromId(Shader, constID);

            if(VIR_Swizzle_Channel_Count(swizzle) > 1)
            {
                return gcvFALSE;
            }
            if(VIR_Operand_GetType(immOpnd) != VIR_GetTypeComponentType(VIR_Operand_GetType(constOpnd)))
            {
                return gcvFALSE;
            }
            return VIR_Operand_GetImmediateUint(immOpnd) == constVal->value.vecVal.u32Value[VIR_Swizzle_GetChannel(swizzle, 0)];
        }
    }
}

gctBOOL
VIR_Operand_Defines(
    IN VIR_Operand  *Opnd0,
    IN VIR_Operand  *Opnd1
    )
{
    gcmASSERT(VIR_Operand_isLvalue(Opnd0));

    if(VIR_Operand_SameSymbol(Opnd0, Opnd1))
    {
        VIR_Enable enable0 = VIR_Operand_GetEnable(Opnd0);
        VIR_Enable enable1;
        if(VIR_Operand_isLvalue(Opnd1))
        {
            enable1 = VIR_Operand_GetEnable(Opnd1);
        }
        else
        {
            enable1 = VIR_Swizzle_2_Enable(VIR_Operand_GetSwizzle(Opnd1));
        }
        return enable0 & enable1;
    }
    return gcvFALSE;
}

void
VIR_Operand_Change2Dest(
    IN OUT VIR_Operand*     Operand)
{
    VIR_Enable enable;
    VIR_Swizzle swizzle;

    if(VIR_Operand_isLvalue(Operand))
    {
        return;
    }

    swizzle = VIR_Operand_GetSwizzle(Operand);
    enable = VIR_Swizzle_2_Enable(swizzle);
    VIR_Operand_SetLvalue(Operand, gcvTRUE);
    VIR_Operand_SetEnable(Operand, enable);
}

void
VIR_Operand_Change2Src(
    IN OUT VIR_Operand*     Operand)
{
    VIR_Enable enable;
    VIR_Swizzle swizzle;

    if(!VIR_Operand_isLvalue(Operand))
    {
        return;
    }

    enable = VIR_Operand_GetEnable(Operand);
    swizzle = VIR_Enable_2_Swizzle(enable);
    VIR_Operand_SetLvalue(Operand, gcvFALSE);
    VIR_Operand_SetSwizzle(Operand, swizzle);
}

void
VIR_Operand_ReplaceDefOperandWithDef(
    IN OUT VIR_Operand *    Def,
    IN VIR_Operand *        New_Def,
    IN VIR_Enable           New_Enable
    )
{
    gctUINT index = VIR_Operand_GetIndex(Def);
    gcmASSERT(VIR_Operand_isLvalue(Def) && VIR_Operand_isLvalue(New_Def));
    memcpy(Def, New_Def, sizeof(VIR_Operand));
    VIR_Operand_SetIndex(Def, index);
    VIR_Operand_SetEnable(Def, New_Enable);
}

void
VIR_Operand_ReplaceUseOperandWithDef(
    IN  VIR_Operand *       Def,
    IN OUT VIR_Operand *    Use
    )
{
    gcmASSERT(VIR_Operand_isLvalue(Def) && !VIR_Operand_isLvalue(Use));
}

void
VIR_Operand_ReplaceUseOperandWithUse(
    IN OUT VIR_Operand *    Tgt_Use,
    IN VIR_Operand *        New_Use,
    IN VIR_Swizzle          New_Swizzle
    )
{
    gcmASSERT(!VIR_Operand_isLvalue(Tgt_Use) && !VIR_Operand_isLvalue(New_Use));

    memcpy(Tgt_Use, New_Use, sizeof(VIR_Operand));
    VIR_Operand_ShrinkSwizzle(Tgt_Use, New_Swizzle);
}

gctBOOL
VIR_Operand_IsOwnerInst(
    IN VIR_Operand *     Operand,
    IN VIR_Instruction * Inst
    )
{
    gctUINT i, j;

    if (VIR_Operand_isLvalue(Operand))
    {
        return (VIR_Inst_GetDest(Inst) == Operand);
    }
    else
    {
        for (i = 0; i < VIR_Inst_GetSrcNum(Inst); i ++)
        {
            VIR_Operand * opnd = VIR_Inst_GetSource(Inst, i);
            if (opnd == Operand)
            {
                return gcvTRUE;
            }
            else if (opnd != gcvNULL && VIR_Operand_isTexldParm(opnd))
            {
                /* special handle texldparm */
                VIR_Operand_TexldModifier *TexldOperand =
                                    (VIR_Operand_TexldModifier*) opnd;

                for(j = 0; j < VIR_TEXLDMODIFIER_COUNT; ++j)
                {
                    if(TexldOperand->tmodifier[j] == Operand)
                    {
                        return gcvTRUE;
                    }
                }
            }
        }

        return gcvFALSE;
    }
}

/* normalize swizzle by enable so it will not access uninvited component:
 *   enable   swizzle   normalizeSwizzle
 *    x        xyzw       xxxx
 *    xz       xyzw       xxzz
 *    yw       xyzx       yyyx
 */
VIR_Swizzle
VIR_NormalizeSwizzleByEnable(
    IN VIR_Enable       Enable,
    IN VIR_Swizzle      Swizzle
    )
{
    VIR_Swizzle normalizedSwizzle = 0;
    gctINT i;
    VIR_Swizzle curSwizzle = VIR_SWIZZLE_X;

    if (Enable == VIR_ENABLE_NONE)
        return VIR_SWIZZLE_XXXX;

    /* find the first enabled channel  */
    for (i = 0; i < VIR_CHANNEL_COUNT; i++)
    {
        if ((((gctUINT)Enable) & (0x01 << i)) != 0)
        {
            curSwizzle = VIR_Swizzle_GetChannel(Swizzle, i);
            break;
        }
    }

    /* set each channel */
    for (i = 0; i < VIR_CHANNEL_COUNT; i++)
    {
        if ((((gctUINT)Enable) & (0x01 << i)) != 0)
        {
            curSwizzle = VIR_Swizzle_GetChannel(Swizzle, i);
        }
        VIR_Swizzle_SetChannel(normalizedSwizzle, i, curSwizzle);
    }
    return normalizedSwizzle;
}

VIR_Enable
VIR_Operand_GetRealUsedChannels(
    IN VIR_Operand *     Operand,
    IN VIR_Instruction * Inst,
    VIR_Swizzle*         RealSwizzle)
{
    VIR_Enable realEnable;
    VIR_Enable enable;
    VIR_OpCode opCode = VIR_Inst_GetOpcode(Inst);
    VIR_Swizzle normalizedSwizzle;

    gcmASSERT(Operand != gcvNULL);
    gcmASSERT(VIR_Operand_IsOwnerInst(Operand, Inst));

    if (VIR_OPCODE_isComponentwise(opCode))
    {
        enable = VIR_Inst_GetEnable(Inst);
    }
    else
    {
        /* special handle non-componentwise operations */
        switch(opCode) {
        case VIR_OP_DP2:
        case VIR_OP_NORM_DP2:
            enable = VIR_ENABLE_XY;
            break;
        case VIR_OP_DP3:
        case VIR_OP_NORM_DP3:
            enable = VIR_ENABLE_XYZ;
            break;
        case VIR_OP_DP4:
        case VIR_OP_NORM_DP4:
            enable = VIR_ENABLE_XYZW;
            break;
        case VIR_OP_CROSS:
            enable = VIR_ENABLE_XYZ;
            break;
        case VIR_OP_STORE:
            enable = VIR_ENABLE_XYZW;
            break;
        default:
            /* assume all other operands set their swizzle properly */
            realEnable = VIR_Swizzle_2_Enable(VIR_Operand_GetSwizzle(Operand));
            return realEnable;
        }
    }
    normalizedSwizzle = VIR_NormalizeSwizzleByEnable(enable,
                                         VIR_Operand_GetSwizzle(Operand));
    realEnable = VIR_Swizzle_2_Enable(normalizedSwizzle);

    if(RealSwizzle != gcvNULL)
    {
        *RealSwizzle = normalizedSwizzle;
    }
    return realEnable;
}

gctUINT
VIR_Opnd_GetCompWiseSrcChannelValue(
    IN VIR_Shader* pShader,
    IN VIR_Instruction* Inst,
    IN VIR_Operand *srcOpnd,
    IN gctUINT8 channel,
    OUT gctUINT* pValue
)
{
    VIR_OperandInfo         operandInfo;
    gctUINT                 virRegNo = NOT_ASSIGNED;
    VIR_Const*              pConstValue;
    VIR_Swizzle             swizzle;

    VIR_Operand_GetOperandInfo(Inst,
                               srcOpnd,
                               &operandInfo);

    if (VIR_OpndInfo_Is_Virtual_Reg(&operandInfo))
    {
        /* Only return first reg no currently */
        virRegNo = operandInfo.u1.virRegInfo.virReg;

        swizzle = VIR_Operand_GetSwizzle(srcOpnd);
        *pValue =  ((swizzle >> channel*2) & 0x3);
    }
    else if (operandInfo.isImmVal)
    {
        *pValue = operandInfo.u1.immValue.uValue;
    }
    else if (operandInfo.isVecConst)
    {
        pConstValue = (VIR_Const*)VIR_GetSymFromId(&pShader->constTable, VIR_Operand_GetConstId(srcOpnd));
        *pValue = pConstValue->value.vecVal.u32Value[channel];
    }
    else if (operandInfo.isUniform)
    {
        virRegNo = operandInfo.u1.uniformIdx;

        swizzle = VIR_Operand_GetSwizzle(srcOpnd);
        *pValue =  ((swizzle >> channel*2) & 0x3);
    }
    else
    {
        gcmASSERT(gcvFALSE);
    }

    return virRegNo;
}

gctBOOL
VIR_Operand_IsPerPatch(
    IN VIR_Operand *Operand)
{
    gctBOOL isPerPatch = gcvFALSE;
    VIR_Symbol *sym = VIR_Operand_GetUnderlyingSymbol(Operand);

    if (sym)
    {
        if (VIR_Symbol_isPerPatch(sym) && !isSymUnused(sym) && !isSymVectorizedOut(sym))
        {
            isPerPatch = gcvTRUE;
        }
    }

    return isPerPatch;
}

gctBOOL
VIR_Operand_IsArrayedPerVertex(
    IN VIR_Operand *Operand)
{
    gctBOOL isPerVertexArray = gcvFALSE;
    VIR_Symbol *sym = VIR_Operand_GetUnderlyingSymbol(Operand);

    if (sym)
    {
        if (isSymArrayedPerVertex(sym) && !isSymUnused(sym) && !isSymVectorizedOut(sym))
        {
            isPerVertexArray = gcvTRUE;
        }
    }

    return isPerVertexArray;
}

gctUINT
VIR_Operand_GetPrecision(
    IN VIR_Operand *Operand
)
{
    VIR_Symbol          *pSym = gcvNULL;
    VIR_OperandKind     opndKind = VIR_Operand_GetOpKind(Operand);
    /* deprecate the operand's precision, instead, using symbol's precision */
    if (opndKind == VIR_OPND_VIRREG ||
        opndKind == VIR_OPND_SYMBOL ||
        opndKind == VIR_OPND_SAMPLER_INDEXING)
    {
        pSym = VIR_Operand_GetSymbol(Operand);
        return VIR_Symbol_GetPrecision(pSym);
    }

    return Operand->_precision;
}

void
VIR_Link_AddLink(
    IN  VIR_Link **         Head,
    IN  VIR_Link *          Link
    )
{
    gcmASSERT(Head && Link && !Link->next);

    if(*Head == gcvNULL)
    {
        *Head = Link;
    }
    else
    {
        VIR_Link* tail = *Head;
        while(tail->next)
        {
            tail = tail->next;
        }
        tail->next = Link;
    }
}

void
VIR_Link_RemoveLink(
    IN  VIR_Link **         Head,
    IN  gctUINTPTR_T        Reference
    )
{
    gcmASSERT(Head);

    if(*Head == gcvNULL)
    {
        return;
    }
    else if((*Head)->referenced == Reference)
    {
        *Head = (*Head)->next;
    }
    else
    {
        VIR_Link* prev = *Head;
        VIR_Link* remove = prev->next;
        while(remove != gcvNULL && remove->referenced != Reference)
        {
            prev = prev->next;
            remove = remove->next;
        }
        if(remove != gcvNULL)
        {
            prev->next = remove->next;
        }
    }
}

gctUINT
VIR_Link_Count(
    IN  VIR_Link *          Head
    )
{
    gctUINT count = 0;

    if(Head == gcvNULL)
    {
        return 0;
    }
    else
    {
        while(Head)
        {
            ++count;
            Head = Head->next;
        }
        return count;
    }
}

gctBOOL
VIR_Link_IsLinkContained(
    IN  VIR_Link *          Head,
    IN  gctUINTPTR_T        Reference
    )
{
    if(Head == gcvNULL)
    {
        return gcvFALSE;
    }
    else
    {
        while(Head)
        {
            if(Head->referenced == Reference)
            {
                return gcvTRUE;
            }
            else
            {
                Head = Head->next;
            }
        }
        return gcvFALSE;
    }
}

void
VIR_ScalarConstVal_GetNeg(
    IN  VIR_PrimitiveTypeId type,
    IN  VIR_ScalarConstVal* in_imm,
    OUT VIR_ScalarConstVal* out_imm
    )
{
    switch(type)
    {
        case VIR_TYPE_FLOAT32:
        {
            gctFLOAT* p = (gctFLOAT*)in_imm;
            *(gctFLOAT*)out_imm = -*p;
            break;
        }
        case VIR_TYPE_INT32:
        case VIR_TYPE_INT16:
        case VIR_TYPE_INT8:
        case VIR_TYPE_UINT32:
        case VIR_TYPE_UINT16:
        case VIR_TYPE_UINT8:
        {
            gctINT* p = (gctINT*)in_imm;
            *(gctINT*)out_imm = -*p;
            break;
        }
        default:
            gcmASSERT(gcvFALSE);
    }
}



void
VIR_ScalarConstVal_AddScalarConstVal(
    IN  VIR_PrimitiveTypeId type,
    IN  VIR_ScalarConstVal* in_imm0,
    IN  VIR_ScalarConstVal* in_imm1,
    OUT VIR_ScalarConstVal* out_imm
    )
{
    switch(type)
    {
        case VIR_TYPE_FLOAT32:
        {
            gctFLOAT* p0 = (gctFLOAT*)in_imm0;
            gctFLOAT* p1 = (gctFLOAT*)in_imm1;
            *(gctFLOAT*)out_imm = *p0 + *p1;
            break;
        }
        case VIR_TYPE_INT32:
        case VIR_TYPE_INT16:
        case VIR_TYPE_INT8:
        case VIR_TYPE_UINT32:
        case VIR_TYPE_UINT16:
        case VIR_TYPE_UINT8:
        {
            gctINT* p0 = (gctINT*)in_imm0;
            gctINT* p1 = (gctINT*)in_imm1;
            *(gctINT*)out_imm = *p0 + *p1;
            break;
        }
        default:
            gcmASSERT(gcvFALSE);
    }
}

void
VIR_ScalarConstVal_MulScalarConstVal(
    IN  VIR_PrimitiveTypeId type,
    IN  VIR_ScalarConstVal* in_imm0,
    IN  VIR_ScalarConstVal* in_imm1,
    OUT VIR_ScalarConstVal* out_imm
    )
{
    switch(type)
    {
        case VIR_TYPE_FLOAT32:
        {
            gctFLOAT* p0 = (gctFLOAT*)in_imm0;
            gctFLOAT* p1 = (gctFLOAT*)in_imm1;
            *(gctFLOAT*)out_imm = *p0 * *p1;
            break;
        }
        case VIR_TYPE_INT32:
        case VIR_TYPE_INT16:
        case VIR_TYPE_INT8:
        case VIR_TYPE_UINT32:
        case VIR_TYPE_UINT16:
        case VIR_TYPE_UINT8:
        {
            gctINT* p0 = (gctINT*)in_imm0;
            gctINT* p1 = (gctINT*)in_imm1;
            *(gctINT*)out_imm = *p0 * *p1;
            break;
        }
        default:
            gcmASSERT(gcvFALSE);
    }
}

gctBOOL
VIR_ScalarConstVal_One(
    IN  VIR_PrimitiveTypeId type,
    IN  VIR_ScalarConstVal* in_imm
    )
{
    switch(type)
    {
        case VIR_TYPE_FLOAT32:
        {
            return *(gctFLOAT*)in_imm == 1.0;
        }
        case VIR_TYPE_INT32:
        case VIR_TYPE_UINT32:
        {
            return *(gctINT32*)in_imm == 1;
        }
        case VIR_TYPE_INT16:
        case VIR_TYPE_UINT16:
        {
            return *(gctINT16*)in_imm == 1;
        }
        case VIR_TYPE_INT8:
        case VIR_TYPE_UINT8:
        {
            return *(gctINT8*)in_imm == 1;
        }
        default:
            gcmASSERT(gcvFALSE);
    }
    return gcvFALSE;
}

void
VIR_VecConstVal_GetNeg(
    IN  VIR_PrimitiveTypeId type,
    IN  VIR_VecConstVal* in_const,
    OUT VIR_VecConstVal* out_const
    )
{
    switch(type)
    {
        case VIR_TYPE_FLOAT_X4:
        {
            int i;
            gctFLOAT* in = (gctFLOAT*)in_const;
            gctFLOAT* out = (gctFLOAT*)out_const;
            for(i = 0; i < 4; i++)
            {
                out[i] = -in[i];
            }
            break;
        }
        case VIR_TYPE_UINT_X4:
        case VIR_TYPE_INTEGER_X4:
        {
            int i;
            gctINT* in = (gctINT*)in_const;
            gctINT* out = (gctINT*)out_const;
            for(i = 0; i < 4; i++)
            {
                out[i] = -in[i];
            }
            break;
        }
        case VIR_TYPE_UINT16_X8:
        case VIR_TYPE_INT16_X8:
        {
            int i;
            gctINT16* in = (gctINT16*)in_const;
            gctINT16* out = (gctINT16*)out_const;
            for(i = 0; i < 8; i++)
            {
                out[i] = -in[i];
            }
            break;
        }
        case VIR_TYPE_UINT8_X16:
        case VIR_TYPE_INT8_X16:
        {
            int i;
            gctINT8* in = (gctINT8*)in_const;
            gctINT8* out = (gctINT8*)out_const;
            for(i = 0; i < 16; i++)
            {
                out[i] = -in[i];
            }
            break;
        }
        default:
            gcmASSERT(gcvFALSE);
    }
}

void
VIR_VecConstVal_AddScalarConstVal(
    IN  VIR_PrimitiveTypeId type,
    IN  VIR_VecConstVal* in_const,
    IN  VIR_ScalarConstVal* in_imm,
    OUT VIR_VecConstVal* out_const
    )
{
    switch(type)
    {
        case VIR_TYPE_FLOAT_X4:
        {
            int i;
            gctFLOAT* in = (gctFLOAT*)in_const;
            gctFLOAT v = *(gctFLOAT*)in_imm;
            gctFLOAT* out = (gctFLOAT*)out_const;
            for(i = 0; i < 4; i++)
            {
                out[i] = in[i] + v;
            }
            break;
        }
        case VIR_TYPE_UINT_X4:
        case VIR_TYPE_INTEGER_X4:
        {
            int i;
            gctINT* in = (gctINT*)in_const;
            gctINT v = *(gctINT*)in_imm;
            gctINT* out = (gctINT*)out_const;
            for(i = 0; i < 4; i++)
            {
                out[i] = in[i] + v;
            }
            break;
        }
        case VIR_TYPE_UINT16_X8:
        case VIR_TYPE_INT16_X8:
        {
            int i;
            gctINT16* in = (gctINT16*)in_const;
            gctINT16 v = *(gctINT16*)in_imm;
            gctINT16* out = (gctINT16*)out_const;
            for(i = 0; i < 8; i++)
            {
                out[i] = in[i] + v;
            }
            break;
        }
        case VIR_TYPE_UINT8_X16:
        case VIR_TYPE_INT8_X16:
        {
            int i;
            gctINT8* in = (gctINT8*)in_const;
            gctINT8 v = *(gctINT8*)in_imm;
            gctINT8* out = (gctINT8*)out_const;
            for(i = 0; i < 16; i++)
            {
                out[i] = in[i] + v;
            }
            break;
        }
        default:
            gcmASSERT(gcvFALSE);
    }
}

void
VIR_VecConstVal_MulScalarConstVal(
    IN  VIR_PrimitiveTypeId type,
    IN  VIR_VecConstVal* in_const,
    IN  VIR_ScalarConstVal* in_imm,
    OUT VIR_VecConstVal* out_const
    )
{
    switch(type)
    {
        case VIR_TYPE_FLOAT_X4:
        {
            int i;
            gctFLOAT* in = (gctFLOAT*)in_const;
            gctFLOAT v = *(gctFLOAT*)in_imm;
            gctFLOAT* out = (gctFLOAT*)out_const;
            for(i = 0; i < 4; i++)
            {
                out[i] = in[i] * v;
            }
            break;
        }
        case VIR_TYPE_UINT_X4:
        case VIR_TYPE_INTEGER_X4:
        {
            int i;
            gctINT* in = (gctINT*)in_const;
            gctINT v = *(gctINT*)in_imm;
            gctINT* out = (gctINT*)out_const;
            for(i = 0; i < 4; i++)
            {
                out[i] = in[i] * v;
            }
            break;
        }
        case VIR_TYPE_UINT16_X8:
        case VIR_TYPE_INT16_X8:
        {
            int i;
            gctINT16* in = (gctINT16*)in_const;
            gctINT16 v = *(gctINT16*)in_imm;
            gctINT16* out = (gctINT16*)out_const;
            for(i = 0; i < 8; i++)
            {
                out[i] = in[i] * v;
            }
            break;
        }
        case VIR_TYPE_UINT8_X16:
        case VIR_TYPE_INT8_X16:
        {
            int i;
            gctINT8* in = (gctINT8*)in_const;
            gctINT8 v = *(gctINT8*)in_imm;
            gctINT8* out = (gctINT8*)out_const;
            for(i = 0; i < 16; i++)
            {
                out[i] = in[i] * v;
            }
            break;
        }
        default:
            gcmASSERT(gcvFALSE);
    }
}

void
VIR_VecConstVal_AddVecConstVal(
    IN  VIR_PrimitiveTypeId type,
    IN  VIR_VecConstVal* in_const0,
    IN  VIR_VecConstVal* in_const1,
    OUT VIR_VecConstVal* out_const
    )
{
    switch(type)
    {
        case VIR_TYPE_FLOAT_X4:
        {
            int i;
            gctFLOAT* in0 = (gctFLOAT*)in_const0;
            gctFLOAT* in1 = (gctFLOAT*)in_const1;
            gctFLOAT* out = (gctFLOAT*)out_const;
            for(i = 0; i < 4; i++)
            {
                out[i] = in0[i] + in1[i];
            }
            break;
        }
        case VIR_TYPE_UINT_X4:
        case VIR_TYPE_INTEGER_X4:
        {
            int i;
            gctINT* in0 = (gctINT*)in_const0;
            gctINT* in1 = (gctINT*)in_const1;
            gctINT* out = (gctINT*)out_const;
            for(i = 0; i < 4; i++)
            {
                out[i] = in0[i] + in1[i];
            }
            break;
        }
        case VIR_TYPE_UINT16_X8:
        case VIR_TYPE_INT16_X8:
        {
            int i;
            gctINT16* in0 = (gctINT16*)in_const0;
            gctINT16* in1 = (gctINT16*)in_const1;
            gctINT16* out = (gctINT16*)out_const;
            for(i = 0; i < 8; i++)
            {
                out[i] = in0[i] + in1[i];
            }
            break;
        }
        case VIR_TYPE_UINT8_X16:
        case VIR_TYPE_INT8_X16:
        {
            int i;
            gctINT8* in0 = (gctINT8*)in_const0;
            gctINT8* in1 = (gctINT8*)in_const1;
            gctINT8* out = (gctINT8*)out_const;
            for(i = 0; i < 16; i++)
            {
                out[i] = in0[i] + in1[i];
            }
            break;
        }
        default:
            gcmASSERT(gcvFALSE);
    }
}

void
VIR_VecConstVal_MulVecConstVal(
    IN  VIR_PrimitiveTypeId type,
    IN  VIR_VecConstVal* in_const0,
    IN  VIR_VecConstVal* in_const1,
    OUT VIR_VecConstVal* out_const
    )
{
    switch(type)
    {
        case VIR_TYPE_FLOAT_X4:
        {
            int i;
            gctFLOAT* in0 = (gctFLOAT*)in_const0;
            gctFLOAT* in1 = (gctFLOAT*)in_const1;
            gctFLOAT* out = (gctFLOAT*)out_const;
            for(i = 0; i < 4; i++)
            {
                out[i] = in0[i] * in1[i];
            }
            break;
        }
        case VIR_TYPE_UINT_X4:
        case VIR_TYPE_INTEGER_X4:
        {
            int i;
            gctINT* in0 = (gctINT*)in_const0;
            gctINT* in1 = (gctINT*)in_const1;
            gctINT* out = (gctINT*)out_const;
            for(i = 0; i < 4; i++)
            {
                out[i] = in0[i] * in1[i];
            }
            break;
        }
        case VIR_TYPE_UINT16_X8:
        case VIR_TYPE_INT16_X8:
        {
            int i;
            gctINT16* in0 = (gctINT16*)in_const0;
            gctINT16* in1 = (gctINT16*)in_const1;
            gctINT16* out = (gctINT16*)out_const;
            for(i = 0; i < 8; i++)
            {
                out[i] = in0[i] * in1[i];
            }
            break;
        }
        case VIR_TYPE_UINT8_X16:
        case VIR_TYPE_INT8_X16:
        {
            int i;
            gctINT8* in0 = (gctINT8*)in_const0;
            gctINT8* in1 = (gctINT8*)in_const1;
            gctINT8* out = (gctINT8*)out_const;
            for(i = 0; i < 16; i++)
            {
                out[i] = in0[i] * in1[i];
            }
            break;
        }
        default:
            gcmASSERT(gcvFALSE);
    }
}

gctBOOL
VIR_VecConstVal_AllZero(
    IN  VIR_PrimitiveTypeId type,
    IN  VIR_VecConstVal* in_const
    )
{
    switch(type)
    {
        case VIR_TYPE_FLOAT_X4:
        {
            int i;
            gctFLOAT* in = (gctFLOAT*)in_const;
            for(i = 0; i < 4; i++)
            {
                if(in[i] != 0.0)
                {
                    return gcvFALSE;
                }
            }
            break;
        }
        case VIR_TYPE_UINT_X4:
        case VIR_TYPE_INTEGER_X4:
        {
            int i;
            gctINT* in = (gctINT*)in_const;
            for(i = 0; i < 4; i++)
            {
                if(in[i] != 0)
                {
                    return gcvFALSE;
                }
            }
            break;
        }
        case VIR_TYPE_UINT16_X8:
        case VIR_TYPE_INT16_X8:
        {
            int i;
            gctINT16* in = (gctINT16*)in_const;
            for(i = 0; i < 8; i++)
            {
                if(in[i] != 0)
                {
                    return gcvFALSE;
                }
            }
            break;
        }
        case VIR_TYPE_UINT8_X16:
        case VIR_TYPE_INT8_X16:
        {
            int i;
            gctINT8* in = (gctINT8*)in_const;
            for(i = 0; i < 16; i++)
            {
                if(in[i] != 0)
                {
                    return gcvFALSE;
                }
            }
            break;
        }
        default:
            gcmASSERT(gcvFALSE);
    }
    return gcvTRUE;
}

gctBOOL
VIR_VecConstVal_AllOne(
    IN  VIR_PrimitiveTypeId type,
    IN  VIR_VecConstVal* in_const
    )
{
    switch(type)
    {
        case VIR_TYPE_FLOAT_X4:
        {
            int i;
            gctFLOAT* in = (gctFLOAT*)in_const;
            for(i = 0; i < 4; i++)
            {
                if(in[i] != 1.0)
                {
                    return gcvFALSE;
                }
            }
            break;
        }
        case VIR_TYPE_UINT_X4:
        case VIR_TYPE_INTEGER_X4:
        {
            int i;
            gctINT* in = (gctINT*)in_const;
            for(i = 0; i < 4; i++)
            {
                if(in[i] != 1)
                {
                    return gcvFALSE;
                }
            }
            break;
        }
        case VIR_TYPE_UINT16_X8:
        case VIR_TYPE_INT16_X8:
        {
            int i;
            gctINT16* in = (gctINT16*)in_const;
            for(i = 0; i < 8; i++)
            {
                if(in[i] != 1)
                {
                    return gcvFALSE;
                }
            }
            break;
        }
        case VIR_TYPE_UINT8_X16:
        case VIR_TYPE_INT8_X16:
        {
            int i;
            gctINT8* in = (gctINT8*)in_const;
            for(i = 0; i < 16; i++)
            {
                if(in[i] != 1)
                {
                    return gcvFALSE;
                }
            }
            break;
        }
        default:
            gcmASSERT(gcvFALSE);
    }
    return gcvTRUE;
}

void _Reset_SrcOperand_Iterator(
    OUT VIR_SrcOperand_Iterator *   Iter
    )
{
    Iter->curNode   = gcvNULL;
    Iter->curSrcNo  = 0;
    Iter->inSrcNo   = 0;
    Iter->speicalNode = 0;
    Iter->useOpndList = 0;
}

void VIR_SrcOperand_Iterator_Init(
    IN  VIR_Instruction *           Inst,
    OUT VIR_SrcOperand_Iterator *   Iter
    )
{
    Iter->inst      = Inst;
    _Reset_SrcOperand_Iterator(Iter);
}

VIR_Operand *
VIR_SrcOperand_Iterator_First(
    IN OUT VIR_SrcOperand_Iterator *   Iter
    )
{
    VIR_Operand * opnd;

    _Reset_SrcOperand_Iterator(Iter);
    opnd = VIR_Inst_GetSource(Iter->inst, Iter->curSrcNo);
    Iter->curSrcNo++;
    /* assume the first node has no special node */
    return opnd;
}

#define MOVE_TO_NEXT_SRC(Iter)                    \
            do {                                  \
                (Iter)->speicalNode = 0;          \
                (Iter)->useOpndList = 0;          \
                (Iter)->curNode = gcvNULL;        \
                (Iter)->curSrcNo++;               \
            } while (0)

VIR_Operand *
VIR_SrcOperand_Iterator_Next(
    VIR_SrcOperand_Iterator *Iter
    )
{
    VIR_Operand * opnd;

    if (!Iter->speicalNode)
    {
        if (Iter->curSrcNo >= VIR_Inst_GetSrcNum(Iter->inst))
        {
            /* no more source operand remain */
            return gcvNULL;
        }
        opnd = VIR_Inst_GetSource(Iter->inst, Iter->curSrcNo);
        gcmASSERT(opnd != gcvNULL);
        /* check if it is special node */
        if (VIR_Operand_GetOpKind(opnd) == VIR_OPND_TEXLDPARM)
        {
            Iter->speicalNode = 1;
            Iter->useOpndList = 0;
            /* get the first effective modifier */
            return VIR_SrcOperand_Iterator_Next(Iter);
        }
        else if (VIR_Operand_GetOpKind(opnd) == VIR_OPND_ARRAY)
        {
            /* set operand info for next iteration */
            Iter->speicalNode = 1;
            Iter->useOpndList = 0;
            Iter->curNode     = VIR_Operand_GetArrayIndex(opnd);
            /* return the base operand */
            return VIR_Operand_GetArrayBase(opnd);
        }
        MOVE_TO_NEXT_SRC(Iter);
        /* check if the operand if unknown kind */
        if (VIR_Operand_GetOpKind(opnd) == VIR_OPND_UNDEF)
        {
            return VIR_SrcOperand_Iterator_Next(Iter);
        }

        return opnd;
    }
    else
    {
        VIR_Operand * opndInInst = VIR_Inst_GetSource(Iter->inst, Iter->curSrcNo);
        /* we are handling a special node which has multiple
         * oeprand in side */
        if (Iter->useOpndList)
        {
            gcmASSERT(Iter->curNode != gcvNULL &&
                VIR_Operand_GetOpKind(opndInInst) == VIR_OPND_ARRAY);
            opnd = Iter->curNode->value;
            /* move to next node in the list */
            Iter->curNode = Iter->curNode->next;
            if (Iter->curNode == gcvNULL)
            {
                /* no more node in the list, move to next src */
                MOVE_TO_NEXT_SRC(Iter);
            }
        }
        else
        {
            if (opndInInst == gcvNULL)
            {
                return gcvNULL;
            }
            gcmASSERT(VIR_Operand_GetOpKind(opndInInst) == VIR_OPND_TEXLDPARM);
            while (Iter->inSrcNo < VIR_TEXLDMODIFIER_COUNT)
            {
                opnd = VIR_Operand_GetTexldModifier(opndInInst, Iter->inSrcNo);
                Iter->inSrcNo++;
                if (opnd != gcvNULL)
                {
                    /* found the modifier operand */
                    return opnd;
                }
            }
            /* nothing left in the current operand, move to next src */
            MOVE_TO_NEXT_SRC(Iter);

            opnd = VIR_SrcOperand_Iterator_Next(Iter);
        }
        return opnd;
    }
}


void VIR_Operand_Iterator_Init(
    IN  VIR_Instruction *        Inst,
    OUT VIR_Operand_Iterator *   Iter
    )
{
    VIR_SrcOperand_Iterator_Init(Inst, &Iter->header);
    Iter->curNo = 0;
    Iter->texldModifierName = VIR_TEXLDMODIFIER_COUNT;
    Iter->dest = gcvFALSE;
}

VIR_Operand *
VIR_Operand_Iterator_First(
    IN OUT VIR_Operand_Iterator *   Iter
    )
{
    VIR_Operand *opnd = gcvNULL;

    Iter->texldModifierName = VIR_TEXLDMODIFIER_COUNT;
    Iter->curNo++;
    /* assume the first node has no special node */
    if(Iter->header.inst->dest)
    {
        Iter->dest = gcvTRUE;
        return Iter->header.inst->dest;
    }

    Iter->curNo++;
    Iter->dest = gcvFALSE;

    opnd =  VIR_SrcOperand_Iterator_First(&Iter->header);
    if(Iter->header.speicalNode &&
       !Iter->header.useOpndList)
    {
        Iter->texldModifierName = (Vir_TexldModifier_Name)(Iter->header.inSrcNo - 1);
    }

    return opnd;
}

VIR_Operand *
VIR_Operand_Iterator_Next(
   IN OUT VIR_Operand_Iterator *Iter
    )
{
    VIR_Operand *opnd = gcvNULL;

    gcmASSERT(Iter->curNo != 0);

    Iter->texldModifierName = VIR_TEXLDMODIFIER_COUNT;

    if(Iter->curNo == 1)
    {
        opnd = VIR_SrcOperand_Iterator_First(&Iter->header);
    }
    else
    {
        opnd =  VIR_SrcOperand_Iterator_Next(&Iter->header);
    }

    Iter->curNo++;

    if(Iter->header.speicalNode)
    {
        Iter->texldModifierName = (Vir_TexldModifier_Name)(Iter->header.inSrcNo - 1);
    }

    return opnd;
}

/* creat a new IdList struct if *IdList is null,
 * allocate id arrays with InitSize in MemPool */
VSC_ErrCode
VIR_IdList_Init(
    IN VSC_MM *         MemPool,
    IN gctUINT          InitSize,
    IN OUT VIR_IdList **IdList)
{
    VSC_ErrCode  errCode    = VSC_ERR_NONE;
    VIR_IdList * idList     = *IdList;

    /* allocate IdList struct if it is not created yet */
    if (idList == gcvNULL)
    {
        idList = (VIR_IdList *)vscMM_Alloc(MemPool, sizeof(VIR_IdList));

        if (idList == gcvNULL)
        {
            return  VSC_ERR_OUT_OF_MEMORY;
        }
        *IdList = idList;
    }

    idList->memPool   = MemPool;
    InitSize = (InitSize == 0 ? 1 : InitSize);
    /* allocate id array */
    idList->ids       = (VIR_Id *)vscMM_Alloc(MemPool, InitSize * sizeof(VIR_Id));
    if (idList->ids == gcvNULL)
    {
        errCode = VSC_ERR_OUT_OF_MEMORY;
        /* free IdList struct */
        vscMM_Free(MemPool, idList);
    }
    else
    {
        idList->allocated = InitSize;
        idList->count     = 0;
    }

    return errCode;
}

VSC_ErrCode
VIR_IdList_Add(
    IN VIR_IdList *     IdList,
    IN VIR_Id           Id)
{
    VSC_ErrCode  errCode    = VSC_ERR_NONE;
    /* check if Id List has enough space */
    if (IdList->count >= IdList->allocated)
    {
        gctUINT newEntries = VIR_ListResize(IdList->allocated);
        void * newIds = vscMM_Realloc(IdList->memPool,
                                      IdList->ids,
                                      newEntries * sizeof(VIR_Id));
        if (newIds == gcvNULL)
        {
            errCode = VSC_ERR_OUT_OF_MEMORY;
        }
        else
        {
            IdList->ids = (VIR_Id *)newIds;
            IdList->allocated = newEntries;
        }
    }
    if (errCode == VSC_ERR_NONE)
        IdList->ids[IdList->count++] = Id;

    return errCode;
}

/* creat a new IdList struct if *IdList is null,
 * allocate id arrays with InitSize in MemPool */
VSC_ErrCode
VIR_ValueList_Init(
    IN VSC_MM *             MemPool,
    IN gctUINT              InitSize,
    IN gctUINT              ElemSize, /* sizeof(typeof(value)) */
    IN OUT VIR_ValueList ** ValueList)
{
    VSC_ErrCode     errCode    = VSC_ERR_NONE;
    VIR_ValueList * valueList  = *ValueList;

    /* allocate ValueList struct if it is not created yet */
    if (valueList == gcvNULL)
    {
        valueList = (VIR_ValueList *)vscMM_Alloc(MemPool, sizeof(VIR_ValueList));

        if (valueList == gcvNULL)
        {
            return  VSC_ERR_OUT_OF_MEMORY;
        }
        *ValueList = valueList;
    }

    valueList->memPool   = MemPool;
    InitSize = (InitSize == 0 ? 1 : InitSize);
    /* allocate value array */
    valueList->values    = (gctCHAR *)vscMM_Alloc(MemPool, InitSize * ElemSize);
    if (valueList->values == gcvNULL)
    {
        errCode = VSC_ERR_OUT_OF_MEMORY;
        /* free ValueList struct */
        vscMM_Free(MemPool, valueList);
    }
    else
    {
        valueList->allocated = InitSize;
        valueList->elemSize  = ElemSize;
        valueList->count     = 0;
    }

    return errCode;
}

VSC_ErrCode
VIR_ValueList_Add(
    IN VIR_ValueList *     ValueList,
    IN gctCHAR *           Value)
{
    VSC_ErrCode  errCode    = VSC_ERR_NONE;
    /* check if Id List has enough space */
    if (ValueList->count >= ValueList->allocated)
    {
        gctUINT newEntries = VIR_ListResize(ValueList->allocated);
        void * newValues = vscMM_Realloc(ValueList->memPool,
                                         ValueList->values,
                                         newEntries * ValueList->elemSize);
        if (newValues == gcvNULL)
        {
            errCode = VSC_ERR_OUT_OF_MEMORY;
        }
        else
        {
            ValueList->values = (gctCHAR *)newValues;
            ValueList->allocated = newEntries;
        }
    }
    if (errCode == VSC_ERR_NONE)
    {
        gctCHAR *addr = ValueList->values + (ValueList->count++) * ValueList->elemSize;
        memcpy(addr, Value, ValueList->elemSize);
    }

    return errCode;
}

#if defined(_NDEBUG)
gctCHAR *
VIR_ValueList_GetValue(
     IN VIR_ValueList * ValueList,
     IN gctUINT         Index)
{
    return Index < VIR_ValueList_Count(ValueList) ?
                (gctCHAR *)(ValueList->values + Index * ValueList->elemSize) : gcvNULL;
}
#endif

VSC_ErrCode
VIR_ValueList_SetValue(
    IN VIR_ValueList *     ValueList,
    IN gctUINT             Index,
    IN gctCHAR *           Value)
{
    VSC_ErrCode  errCode    = VSC_ERR_NONE;
    gctCHAR *addr = ValueList->values + Index * ValueList->elemSize;

    gcmASSERT(Value != gcvNULL);
    gcmASSERT(Index < VIR_ValueList_Count(ValueList) );
    memcpy(addr, Value, ValueList->elemSize);

    return errCode;
}

/* Common compare functions are defined here */
gctUINT
vscHFUNC_Type(const char *Str)
{
    VIR_Type *type = (VIR_Type *)Str;
    gctUINT32 hashVal = type->_base | (type->_kind << 20);
    return (hashVal & 0x7FFFFFFF);
}

static gctBOOL
_sameParameterTypes(VIR_Type *Type1, VIR_Type *Type2)
{
    gctBOOL sameType = gcvFALSE;
    gcmASSERT(VIR_Type_GetKind(Type1) == VIR_TY_FUNCTION &&
              VIR_Type_GetKind(Type2) == VIR_TY_FUNCTION );
    if (VIR_Type_GetParameters(Type1)->count == VIR_Type_GetParameters(Type2)->count)
    {
        gctUINT i;
        for (i=0; i < VIR_Type_GetParameters(Type1)->count ; i++)
        {
            if (VIR_Type_GetParameters(Type1)->ids[i] !=
                    VIR_Type_GetParameters(Type2)->ids[i])
                break;
        }

        if (i == VIR_Type_GetParameters(Type2)->count)
            sameType = gcvTRUE;
    }

    return sameType;
}

gctBOOL
vcsHKCMP_Type(const char *Str1, const char *Str2)
{
    VIR_Type *type1 = (VIR_Type *)Str1;
    VIR_Type *type2 = (VIR_Type *)Str2;

    /* do we need to check alignment? */
    if (VIR_Type_GetKind(type1) == VIR_Type_GetKind(type2) &&
        VIR_Type_GetBaseTypeId(type1) == VIR_Type_GetBaseTypeId(type2))
    {
        switch (VIR_Type_GetKind(type1)) {
        case VIR_TY_ARRAY:
            /* array length must equal */
            return (VIR_Type_GetArrayLength(type1) == VIR_Type_GetArrayLength(type2));
        case VIR_TY_FUNCTION:
            /* function parameters type must equal */
            return _sameParameterTypes(type1, type2);
        case VIR_TY_STRUCT:
            /* struct must have the same symbol */
            return (VIR_Type_GetNameId(type1) == VIR_Type_GetNameId(type2));
        case VIR_TY_POINTER:
            /* pointer must point to same address space and same qualifier */
            return (VIR_Type_GetAddrSpace(type1) == VIR_Type_GetAddrSpace(type2)) &&
                   (VIR_Type_GetQualifier(type1) == VIR_Type_GetQualifier(type2));
        default:
            return gcvTRUE;
        }
    }

    /* not equal */
    return gcvFALSE;
}

gctUINT
vscHFUNC_Const(const char *Str)
{
    VIR_Const *c = (VIR_Const *)Str;
    gctUINT32 hashVal = ((c->value.vecVal.u32Value[3] << 15) +
                         (c->value.vecVal.u32Value[2] << 10) +
                         (c->value.vecVal.u32Value[1] << 5)  +
                         (c->value.vecVal.u32Value[0])          ) |
                        (c->type << 20);
    return (hashVal & 0x7FFFFFFF);

}

gctBOOL
vcsHKCMP_Const(const char *Str1, const char *Str2)
{
    VIR_Const *c1 = (VIR_Const *)Str1;
    VIR_Const *c2 = (VIR_Const *)Str2;

    if   (c1->type == c2->type)
    {
        return  (c1->value.vecVal.u32Value[3] == c2->value.vecVal.u32Value[3]) &&
                (c1->value.vecVal.u32Value[2] == c2->value.vecVal.u32Value[2]) &&
                (c1->value.vecVal.u32Value[1] == c2->value.vecVal.u32Value[1]) &&
                (c1->value.vecVal.u32Value[0] == c2->value.vecVal.u32Value[0]);
    }
    return gcvFALSE;
}

gctUINT
vscHFUNC_Symbol(const char *Str)
{
    VIR_Symbol *sym = (VIR_Symbol *)Str;
    VIR_Type *  type;
    gctUINT32 hashVal = 0;

    switch (VIR_Symbol_GetKind(sym))
    {
    case VIR_SYM_UNIFORM:
    case VIR_SYM_UBO:            /* uniform block object */
    case VIR_SYM_VARIABLE:       /* global/local variables, input/output */
    case VIR_SYM_SBO:         /* buffer variables */
    case VIR_SYM_FUNCTION:       /* function */
    case VIR_SYM_SAMPLER:
    case VIR_SYM_TEXTURE:
    case VIR_SYM_IMAGE:
    case VIR_SYM_TYPE:           /* typedef */
    case VIR_SYM_LABEL:
    case VIR_SYM_IOBLOCK:
        hashVal = VIR_Symbol_GetName(sym) | (VIR_Symbol_GetKind(sym) << 20);
        break;
    case VIR_SYM_FIELD:          /* the field of class/struct/union/ubo */
        type = VIR_Symbol_GetStructType(sym);
        gcmASSERT(type != gcvNULL);
        hashVal = VIR_Symbol_GetName(sym)               |
                  (VIR_Type_GetIndex(type) << 10) |
                  (VIR_Symbol_GetKind(sym) << 20);
        break;
    case VIR_SYM_CONST:          /* constant value with type */
        hashVal = VIR_Symbol_GetConstId(sym) | (VIR_Symbol_GetKind(sym) << 20);
        break;
    case VIR_SYM_VIRREG:         /* virtual register */
        hashVal = VIR_Symbol_GetVregIndex(sym) | (VIR_Symbol_GetKind(sym) << 20);
        break;
    default:
        gcmASSERT(0);
        break;
    }
    return (hashVal & 0x7FFFFFFF);
}

gctBOOL
vcsHKCMP_Symbol(const char *Str1, const char *Str2)
{
    VIR_Symbol *sym1 = (VIR_Symbol *)Str1;
    VIR_Symbol *sym2 = (VIR_Symbol *)Str2;

    if (VIR_Symbol_GetKind(sym1) == VIR_Symbol_GetKind(sym2))
    {
        switch (VIR_Symbol_GetKind(sym2)) {
        case VIR_SYM_UNIFORM:
        case VIR_SYM_UBO:
        case VIR_SYM_VARIABLE:
        case VIR_SYM_SBO:         /* buffer variables */
        case VIR_SYM_FUNCTION:
        case VIR_SYM_SAMPLER:
        case VIR_SYM_TEXTURE:
        case VIR_SYM_IMAGE:
        case VIR_SYM_TYPE:
        case VIR_SYM_LABEL:
        case VIR_SYM_IOBLOCK:
            /* these symbols are the same if they have the same name */
            return VIR_Symbol_GetName(sym1) == VIR_Symbol_GetName(sym2);
        case VIR_SYM_FIELD:
            /* name and enclosing type must be the same */
            return VIR_Symbol_GetName(sym1) == VIR_Symbol_GetName(sym2) &&
                   (VIR_Symbol_GetStructType(sym1) == VIR_Symbol_GetStructType(sym2));
        case VIR_SYM_VIRREG:
            /* index must be the same */
            return  VIR_Symbol_GetVregIndex(sym1) ==  VIR_Symbol_GetVregIndex(sym2);
        case VIR_SYM_CONST:          /* constant value with type */
            return VIR_Symbol_GetConstId(sym1) == VIR_Symbol_GetConstId(sym2);
        default:
            return gcvFALSE;
        }
    }

    /* not equal */
    return gcvFALSE;
}

gctUINT
vscHFUNC_Label(const char *Str)
{
    VIR_Label *label = (VIR_Label *)Str;
    gctUINT32 hashVal = label->sym;
    return (hashVal & 0x7FFFFFFF);

}

gctBOOL
vcsHKCMP_Label(const char *Str1, const char *Str2)
{
    VIR_Label *label1 = (VIR_Label *)Str1;
    VIR_Label *label2 = (VIR_Label *)Str2;

    return  (label1->sym == label2->sym);
}

/* return TRUE if JMPC condition can be evaluated to true or false;
   i.e., the condition is a compile-time constant.
   Otherwise return FALSE.
   The evaluated condition value is stored in checkingResult. */
gctBOOL
VIR_Evaluate_JMPC_Condition(
    IN VIR_Shader      *pShader,
    IN VIR_Instruction *inst,
    OUT gctBOOL         *checkingResult)
{
    VIR_Operand     *src0, *src1;
    VIR_TypeId      src0Type, src1Type;
    VIR_OperandInfo  src0Info, src1Info;

    gctUINT32 value0, value1;

    gcmASSERT(VIR_Inst_GetOpcode(inst) == VIR_OP_JMPC ||
              VIR_Inst_GetOpcode(inst) == VIR_OP_JMP_ANY);

    src0 = VIR_Inst_GetSource(inst, 0);
    src1 = VIR_Inst_GetSource(inst, 1);

    src0Type = VIR_Operand_GetType(src0);
    src1Type = VIR_Operand_GetType(src1);

    VIR_Operand_GetOperandInfo(inst, src0, &src0Info);
    VIR_Operand_GetOperandInfo(inst, src1, &src1Info);

    if (!src0Info.isImmVal || !src1Info.isImmVal)
    {
        return gcvFALSE;
    }

    if (src0Type == VIR_TYPE_FLOAT32 ||
        src1Type == VIR_TYPE_FLOAT32)
    {
        float f0, f1;

        if (src0Type == VIR_TYPE_FLOAT32)
        {
            f0 = src0Info.u1.immValue.fValue;
        }
        else if (src0Type == VIR_TYPE_INT32)
        {
            f0 = (gctFLOAT)(src0Info.u1.immValue.iValue);
        }
        else
        {
            return gcvFALSE;
        }

        if (src1Type == VIR_TYPE_FLOAT32)
        {
            f1 = src1Info.u1.immValue.fValue;
        }
        else if (src1Type == VIR_TYPE_INT32)
        {
            f1 = (gctFLOAT) src1Info.u1.immValue.iValue;
        }
        else
        {
            /* Error. */
            return gcvFALSE;
        }

        switch (VIR_Inst_GetConditionOp(inst))
        {
        case VIR_COP_ALWAYS:
            /* Error. */ return gcvFALSE;
        case VIR_COP_NOT_EQUAL:
            *checkingResult = (f0 != f1); break;
        case VIR_COP_LESS_OR_EQUAL:
            *checkingResult = (f0 <= f1); break;
        case VIR_COP_LESS:
            *checkingResult = (f0 < f1); break;
        case VIR_COP_EQUAL:
            *checkingResult = (f0 == f1); break;
        case VIR_COP_GREATER:
            *checkingResult = (f0 > f1); break;
        case VIR_COP_GREATER_OR_EQUAL:
            *checkingResult = (f0 >= f1); break;
        case VIR_COP_AND:
        case VIR_COP_OR:
        case VIR_COP_XOR:
            /* TODO - Error. */
            return gcvFALSE;
        case VIR_COP_NOT_ZERO:
            *checkingResult = (f0 != 0.0f); break;
        default:
            return gcvFALSE;
        }
    }
    else
    {
        value0 = src0Info.u1.immValue.uValue;
        value1 = src1Info.u1.immValue.uValue;

        switch (VIR_Inst_GetConditionOp(inst))
        {
        case VIR_COP_ALWAYS:
            /* Error. */ return gcvFALSE;
        case VIR_COP_NOT_EQUAL:
            *checkingResult = (value0 != value1); break;
        case VIR_COP_LESS_OR_EQUAL:
            *checkingResult = (value0 <= value1); break;
        case VIR_COP_LESS:
            *checkingResult = (value0 < value1); break;
        case VIR_COP_EQUAL:
            *checkingResult = (value0 == value1); break;
        case VIR_COP_GREATER:
            *checkingResult = (value0 > value1); break;
        case VIR_COP_GREATER_OR_EQUAL:
            *checkingResult = (value0 >= value1); break;
        case VIR_COP_AND:
            *checkingResult = (value0 & value1); break;
        case VIR_COP_OR:
            *checkingResult = (value0 | value1); break;
        case VIR_COP_XOR:
            *checkingResult = (value0 ^ value1); break;
        case VIR_COP_NOT_ZERO:
            *checkingResult = (value0 != 0); break;
        default:
            return gcvFALSE;
        }
    }

    return gcvTRUE;
}

VIR_TessOutputPrimitive
VIR_ConvertTESLayoutToOutputPrimitive(
    IN VIR_TESLayout* TesLayout)
{
    if (TesLayout->tessPrimitiveMode == VIR_TESS_PMODE_TRIANGLE ||
        TesLayout->tessPrimitiveMode == VIR_TESS_PMODE_QUAD)
    {
        if (TesLayout->tessPointMode)
        {
            return VIR_TESS_OUTPUT_PRIM_POINT;
        }
        else
        {
            if (TesLayout->tessOrdering == VIR_TESS_ORDER_CW)
            {
                return VIR_TESS_OUTPUT_PRIM_TRIANGLE_CW;
            }
            else
            {
                return VIR_TESS_OUTPUT_PRIM_TRIANGLE_CCW;
            }
        }
    }
    else
    {
        if (TesLayout->tessPointMode)
        {
            return VIR_TESS_OUTPUT_PRIM_POINT;
        }
        else
        {
            return VIR_TESS_OUTPUT_PRIM_LINE;
        }
    }
}

gctBOOL
VIR_Opnd_ValueFit16Bits(
    IN VIR_Operand *Operand)
{
    gcmASSERT(VIR_Operand_GetOpKind(Operand) == VIR_OPND_IMMEDIATE);

    switch (VIR_GetTypeComponentType(VIR_Operand_GetType(Operand)))
    {
    case VIR_TYPE_BOOLEAN:
    case VIR_TYPE_INT16:
    case VIR_TYPE_UINT16:
        /* fits 16bits */
        return gcvTRUE;
    case VIR_TYPE_INT32:
        return CAN_EXACTLY_CVT_S32_2_S16(Operand->u1.iConst);
    case VIR_TYPE_UINT32:
        return CAN_EXACTLY_CVT_U32_2_U16(Operand->u1.uConst);
    case VIR_TYPE_FLOAT32:
        /* float immediate will always be put into uniform */
        return gcvTRUE;
    default:
        gcmASSERT(gcvFALSE);
        break;
    }

    return gcvFALSE;
}

/*
 * For source operands:
 *   Supported sources for single-t instructions which are not supported for
 *   dual-t instructions:
 *   - Highp sources  (e.g. Position or MSAA subsample information)
 *   - Anything using relative addressing of any kind (a0, aL, or InstanceID).
 *   - Immediate source formats other than packed V16.
 *   - Local storage (lr*)   ....but this is useless since writes to local
 *     storage aren't supported in dual-16 mode.
 *   - uniform (int32/uint32)
 *
 * For dest operand:
 *   Supported destinations for single-t instructions which are not supported for
 *   dual-t instructions:
 *   - Highp destinations  (e.g. depth or MSAA subsample information)
 *   - Anything using relative addressing of any kind (a0, aL, or InstanceID).
 *
 *   Unsupported destinations:
 *   - Local storage (lr*)
 *
 */
VSC_ErrCode
VIR_Operand_Check4Dual16(
    IN  VIR_Shader          * Shader,
    IN VIR_Instruction      * VirInst,
    IN VIR_Operand          * Operand,
    OUT VIR_ShaderCodeInfo  * CodeInfo,
    OUT gctBOOL             * isHighPrecisionOperand
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    switch(VIR_Operand_GetOpKind(Operand))
    {
    case VIR_OPND_NONE:
        /* fall through */
    case VIR_OPND_UNDEF:
        /* fall through */
    case VIR_OPND_UNUSED:
        return errCode;
    case VIR_OPND_IMMEDIATE:
        {
            VIR_Type   *type = VIR_Shader_GetTypeFromId(Shader, VIR_Operand_GetType(Operand));

            if(type == gcvNULL)
            {
                return VSC_ERR_INVALID_TYPE;
            }

            /* if the immediate value cannot fit into packed V16,
               it needs to be put into uniform (highp) */
            if (!VIR_Opnd_ValueFit16Bits(Operand))
            {
                *isHighPrecisionOperand = gcvTRUE;
            }
            else
            {
                VIR_Operand_SetPrecision(Operand, VIR_PRECISION_MEDIUM);
            }

            break;
        }
    case VIR_OPND_SAMPLER_INDEXING:

        if(VIR_Operand_GetRelAddrMode(Operand) != VIR_INDEXED_NONE)
        {
        }
        else
        {
        }
        break;
    case VIR_OPND_SYMBOL:
        {
            VIR_Symbol      *sym        = VIR_Operand_GetSymbol(Operand);
            VIR_Symbol      *underlyingSym;
            VIR_TypeId       typeId = VIR_Operand_GetType(Operand);

            /* ldarr/starr operand could be array type */
            if (!VIR_TypeId_isPrimitive(typeId))
            {
                VIR_Type   *type = VIR_Shader_GetTypeFromId(Shader, typeId);
                while (type && !VIR_Type_isPrimitive(type))
                {
                    type = VIR_Shader_GetTypeFromId(Shader, VIR_Type_GetBaseTypeId(type));
                }
                gcmASSERT(type);
                typeId = VIR_Type_GetIndex(type);

            }
            gcmASSERT(VIR_TypeId_isPrimitive(typeId));

            underlyingSym = VIR_Operand_GetUnderlyingSymbol(Operand);
            if (underlyingSym)
            {
                sym = underlyingSym;
            }
            if (VIR_Symbol_isVariable(sym) &&
                (VIR_Symbol_GetName(sym) == VIR_NAME_POSITION ||
                 VIR_Symbol_GetName(sym) == VIR_NAME_SUBSAMPLE_DEPTH))
            {
                *isHighPrecisionOperand = gcvTRUE;
            }
            if (VIR_Symbol_GetStorageClass(sym) == VIR_STORAGE_LOCALSTORAGE)
            {
                if (VirInst->dest == Operand)
                {
                    CodeInfo->destUseLocalStorage = gcvTRUE;
                }
                else
                {
                    CodeInfo->srcUseLocalStorage = gcvTRUE;
                }
            }
            switch(VIR_Symbol_GetKind(sym))
            {
            case VIR_SYM_UNIFORM:
                {
                    if ((VIR_GetTypeFlag(typeId) & VIR_TYFLAG_ISINTEGER) != 0)
                    {
                        *isHighPrecisionOperand = gcvTRUE;
                    }
                }
                break;
            case VIR_SYM_CONST:
                break;
            case VIR_SYM_VIRREG:
            case VIR_SYM_SAMPLER:
            case VIR_SYM_IMAGE:
            case VIR_SYM_TEXTURE:
            case VIR_SYM_VARIABLE:
            case VIR_SYM_FIELD:
                {
                    VIR_TypeId componentType = VIR_GetTypeComponentType(typeId);
                    if (VIR_Operand_GetPrecision(Operand) == VIR_PRECISION_HIGH &&
                        VIR_GetTypeSize(componentType) == 4 /*32 bit */)
                    {
                        *isHighPrecisionOperand = gcvTRUE;
                    }
                }
                break;
            case VIR_SYM_LABEL:
                break;
            default:
            case VIR_SYM_UBO:
            case VIR_SYM_TYPE:
                gcmASSERT(0);
                break;
            }


            break;
        }
    case VIR_OPND_LABEL:
    case VIR_OPND_CONST:
        break;
    case VIR_OPND_FUNCTION:
    case VIR_OPND_PARAMETERS:
    case VIR_OPND_INTRINSIC:
    case VIR_OPND_FIELD:
    case VIR_OPND_ARRAY:
    default:
        gcmASSERT(0);
        break;
    }
    return errCode;
}

gctBOOL
VIR_Inst_Dual16NotSupported(
    IN VIR_Instruction *  pInst
    )
{
    VIR_OpCode  opcode  =  VIR_Inst_GetOpcode(pInst);

    if (VIR_OPCODE_isCall(opcode) || opcode == VIR_OP_RET ||
        opcode == VIR_OP_LOOP || opcode == VIR_OP_ENDLOOP ||
        opcode == VIR_OP_REP || opcode == VIR_OP_ENDREP)
    {
        return gcvTRUE;
    }
    return gcvFALSE;
}

gctBOOL
VIR_Inst_isIntType(
    IN VIR_Instruction *  pInst
    )
{
    VIR_Operand * dest = VIR_Inst_GetDest(pInst);

    if (dest)
    {
        VIR_PrimitiveTypeId tyId = VIR_Operand_GetType(dest);
        if (tyId > VIR_TYPE_LAST_PRIMITIVETYPE)
        {
            return gcvFALSE;
        }
        return (VIR_GetTypeFlag(tyId) & VIR_TYFLAG_ISINTEGER) != 0;
    }

    return gcvFALSE;
}

gctBOOL
VIR_Inst_Dual16NeedRunInSingleT(
    IN VIR_Instruction *  pInst
    )
{
    VIR_OpCode  opcode  =  VIR_Inst_GetOpcode(pInst);
    /* Supported single-t instructions which are unsupported dual-t instructions, by category:
    -* load/store: LOAD, STORE, ATOM_ADD, ATOM_XCHG, ATOM_CMP_XCHG, ATOM_MIN,
     *             ATOM_MAX, ATOM_OR, ATOM_AND, ATOM_XOR.
    -* a0:         MOVAR, MOVAF, MOVAI
    -* OpenCL:     ADDLO, MULLO, SWIZZLE (opcode=0x2b).
     *             (NOTE: Swizzling of source operands is still supported in dual-t instructions.)
    -* idiv/imod:  IDIV0, IMOD0
    */
    if (opcode == VIR_OP_LOAD         || opcode == VIR_OP_ILOAD           ||
        opcode == VIR_OP_IMG_LOAD     || opcode == VIR_OP_IMG_LOAD_3D     ||
        opcode == VIR_OP_VX_IMG_LOAD  || opcode == VIR_OP_VX_IMG_LOAD_3D  ||
        opcode == VIR_OP_STORE        || opcode == VIR_OP_ISTORE          ||
        opcode == VIR_OP_IMG_STORE    || opcode == VIR_OP_IMG_STORE_3D    ||
        opcode == VIR_OP_VX_IMG_STORE || opcode == VIR_OP_VX_IMG_STORE_3D ||
        VIR_OPCODE_isAtom(opcode)   ||
        opcode == VIR_OP_MOVA       ||
        opcode == VIR_OP_LDARR      ||
        opcode == VIR_OP_STARR      ||
        opcode == VIR_OP_ADDLO      ||
        opcode == VIR_OP_MULLO      ||
        opcode == VIR_OP_SWIZZLE    ||
        opcode == VIR_OP_AQ_IMADLO0 ||
        opcode == VIR_OP_AQ_IMADLO1 ||
        opcode == VIR_OP_AQ_IMADHI0 ||
        opcode == VIR_OP_AQ_IMADHI1 ||
        opcode == VIR_OP_ARCTRIG ||
        ((opcode == VIR_OP_DIV || opcode == VIR_OP_MOD) && VIR_Inst_isIntType(pInst))
        )
    {
        return gcvTRUE;
    }
    return gcvFALSE;
}

VSC_ErrCode
VIR_Shader_CheckDual16(
    IN  VIR_Shader *           Shader,
    OUT VIR_ShaderCodeInfo *   CodeInfo
    )
{
    VSC_ErrCode      errCode = VSC_ERR_NONE;
    VIR_FunctionNode*  pFuncNode;
    VIR_FuncIterator   funcIter;
    VIR_Instruction*   pInst;
    VIR_InstIterator   instIter;
    gctBOOL            dual16NotSupported  = gcvFALSE;
    gctINT             runSignleTInstCount = 0;

    /* Calculate HW inst count */
    VIR_FuncIterator_Init(&funcIter, &Shader->functions);
    pFuncNode = VIR_FuncIterator_First(&funcIter);

    for(; pFuncNode != gcvNULL; pFuncNode = VIR_FuncIterator_Next(&funcIter))
    {
        VIR_Function *pFunc = pFuncNode->function;

        VIR_InstIterator_Init(&instIter, &pFunc->instList);
        pInst = VIR_InstIterator_First(&instIter);

        for (; pInst != gcvNULL; pInst = VIR_InstIterator_Next(&instIter))
        {
            VIR_OpCode  opcode  =  VIR_Inst_GetOpcode(pInst);
            gctBOOL     needRunSingleT = gcvFALSE;
            gctBOOL     isHighPrecisionOperand = gcvFALSE;
            gctSIZE_T   i       = 0;
            /* check dest */
            if (VIR_Inst_Dual16NotSupported(pInst))
            {
                dual16NotSupported = gcvTRUE;
                break;
            }

            if (VIR_Inst_Dual16NeedRunInSingleT(pInst))
            {
                runSignleTInstCount++;
                needRunSingleT = gcvTRUE;
            }

            if (pInst->dest)
            {
                VIR_Operand_Check4Dual16(Shader, pInst, pInst->dest, CodeInfo, &isHighPrecisionOperand);
                needRunSingleT |=  isHighPrecisionOperand;
            }

            /* check source operands */
            for(i = 0; i < VIR_Inst_GetSrcNum(pInst); ++i)
            {
                VIR_Operand *operand =  pInst->src[i];
                VIR_Operand_Check4Dual16(Shader, pInst, operand, CodeInfo, &isHighPrecisionOperand);
                needRunSingleT |=  isHighPrecisionOperand;
            }
            gcmASSERT(VIR_OP_MAXOPCODE > opcode);
            CodeInfo->codeCounter[(gctINT)opcode]++;
            if (opcode != VIR_OP_LABEL /* exclude non executive IRs */)
            {
                CodeInfo->estimatedInst++;
            }
            if (needRunSingleT)
            {
                CodeInfo->highPInstCount++;
                VIR_Inst_SetThreadMode(pInst, VIR_THREAD_D16_DUAL_32);
            }
        }
    }

    CodeInfo->hasUnsupportDual16Inst = dual16NotSupported;
    CodeInfo->runSignleTInstCount    = runSignleTInstCount;

    return errCode;
}

static void _SortAttributesOfDual16Shader(VIR_Shader *pShader, VSC_HW_CONFIG *pHwCfg)
{
    VIR_AttributeIdList     *pAttrs = VIR_Shader_GetAttributes(pShader);
    gctUINT                 attrCount = VIR_IdList_Count(pAttrs);
    gctUINT                 i, j, tempIdx;

    /* in dual16, we need to put all HP attributes in front of MP attributes,
       r0/r1: highp position
       highp varying:
        1) all vec3 and vec4 at 2 register each
        2) all vec1 and vec2 at 1 register
       mediump varying: 1 register each
    */

    /* put all highp to the front */
    for (i = 0; i < attrCount; i ++)
    {
        VIR_Symbol  *attribute = VIR_Shader_GetSymFromId(
                           pShader, VIR_IdList_GetId(pAttrs, i));

        if (VIR_Symbol_GetPrecision(attribute) == VIR_PRECISION_HIGH)
        {
            continue;
        }
        else
        {
            for (j = i + 1; j < attrCount; j ++)
            {
                VIR_Symbol  *nextAttribute = VIR_Shader_GetSymFromId(pShader, VIR_IdList_GetId(pAttrs, j));

                if (VIR_Symbol_GetPrecision(nextAttribute) == VIR_PRECISION_HIGH)
                {
                    tempIdx = VIR_IdList_GetId(pAttrs, j);
                    VIR_IdList_SetId(pAttrs, j, VIR_IdList_GetId(pAttrs, i));
                    VIR_IdList_SetId(pAttrs, i, tempIdx);
                    break;
                }
            }
        }
    }

    if (pHwCfg->hwFeatureFlags.highpVaryingShift)
    {
        /* put all highp vec3/vec4 to the front */
        for (i = 0; i < attrCount; i ++)
        {
            VIR_Symbol  *attribute = VIR_Shader_GetSymFromId(pShader, VIR_IdList_GetId(pAttrs, i));

            if (VIR_Symbol_GetPrecision(attribute) == VIR_PRECISION_HIGH &&
                VIR_Symbol_GetComponents(attribute) > 2)
            {
                continue;
            }
            else
            {
                for (j = i + 1; j < attrCount; j ++)
                {
                    VIR_Symbol  *nextAttribute = VIR_Shader_GetSymFromId(
                        pShader, VIR_IdList_GetId(pAttrs, j));

                    if (VIR_Symbol_GetPrecision(nextAttribute) == VIR_PRECISION_HIGH &&
                        VIR_Symbol_GetComponents(nextAttribute) > 2)
                    {
                        tempIdx = VIR_IdList_GetId(pAttrs, j);
                        VIR_IdList_SetId(pAttrs, j, VIR_IdList_GetId(pAttrs, i));
                        VIR_IdList_SetId(pAttrs, i, tempIdx);
                        break;
                    }
                }
            }
        }
    }
}

gctBOOL
VIR_Shader_IsDual16able(
    IN OUT VIR_Shader       *Shader,
    IN  VSC_HW_CONFIG       *pHwCfg
    )
{
    VIR_ShaderCodeInfo codeInfo;
    gctUINT            dual16Mode = gcmOPT_DualFP16Mode();
    gctBOOL            autoMode = gcvFALSE;
    gctUINT            i = 0;
    VIR_Symbol         *pSym = gcvNULL;

    /* only fragment shader can be dual16 shader,
    ** and exclude OpenVG shader due to precision issue
    */
    if (!pHwCfg->hwFeatureFlags.supportDual16               ||
        (VIR_Shader_GetKind(Shader) != VIR_SHADER_FRAGMENT) ||
        (dual16Mode == DUAL16_FORCE_OFF)                    ||
        VIR_Shader_IsOpenVG(Shader)                         ||
        VIR_Shader_Has32BitModulus(Shader)                  || /* gcsl expand the integer mod and div using 32bit way
                                                                  (_gcConvert32BitModulus) */
        !VirSHADER_DoDual16(VIR_Shader_GetId(Shader))
        )
        return gcvFALSE;

    if (dual16Mode == DUAL16_AUTO_ALL || dual16Mode == DUAL16_FORCE_ON)
    {
        autoMode = gcvTRUE;
    }
    else if (dual16Mode == DUAL16_AUTO_BENCH)
    {
        gcePATCH_ID patchID = gcvPATCH_INVALID;

        gcoHAL_GetPatchID(gcvNULL, &patchID);

        /* Enable dual16 auto-on mode for following games. */
        switch (patchID)
        {
        case gcvPATCH_GLBM21:
        case gcvPATCH_GLBM25:
        case gcvPATCH_GLBM27:
        case gcvPATCH_GFXBENCH:
        case gcvPATCH_MM07:
        case gcvPATCH_NENAMARK2:
        case gcvPATCH_LEANBACK:
        case gcvPATCH_ANGRYBIRDS:
            autoMode = gcvTRUE;
            break;
        default:
            break;
        }
    }

    /*  currently, we disable dual16 if the output array is highp.
        it is not clear how to allocate two register arrays for T0 and T1.
    */
    if (gcvFALSE == autoMode ||
        VIR_Shader_HasOutputArrayHighp(Shader))
    {
        return gcvFALSE;
    }

    /* dual16 does not support attribute components larger than 60 */
    {
        gctUINT attrCount = 0, highpAttrCount = 0, totalAttrCount = 0;
        gctUINT components = 0;
        for (i = 0; i < VIR_IdList_Count(VIR_Shader_GetAttributes(Shader)); i++)
        {
            VIR_Type        *symType = gcvNULL;

            pSym = VIR_Shader_GetSymFromId(Shader, VIR_IdList_GetId(VIR_Shader_GetAttributes(Shader), i));

            /* Only consider used one */
            if (isSymUnused(pSym) || isSymVectorizedOut(pSym))
            {
                continue;
            }

            symType = VIR_Symbol_GetType(pSym);

            if (VIR_Type_isPrimitive(symType))
            {
                components = VIR_GetTypeComponents(VIR_Type_GetIndex(symType));
            }
            else
            {
                components = VIR_GetTypeComponents(VIR_Type_GetBaseTypeId(symType));
            }

            attrCount += VIR_Type_GetVirRegCount(Shader, symType) * components;
            if (VIR_Symbol_GetPrecision(pSym) == VIR_PRECISION_HIGH)
            {
                highpAttrCount += VIR_Type_GetVirRegCount(Shader, symType) * components;
            }
        }

        /* this formula is coming from cmodel */
        totalAttrCount = ((2 + 4 + attrCount + 3) >> 2) + ((2 + 4 + highpAttrCount + 3) >> 2);
        if (totalAttrCount > 15)
        {
            return gcvFALSE;
        }
    }

    {
        gcoOS_ZeroMemory(&codeInfo, gcmSIZEOF(codeInfo));

        /* check the shader code */
        VIR_Shader_CheckDual16(Shader, &codeInfo);
    }

    if (codeInfo.hasUnsupportDual16Inst     ||
        codeInfo.useInstanceID              ||
        codeInfo.useVertexID                ||
        codeInfo.destUseLocalStorage        ||
        (codeInfo.estimatedInst + codeInfo.runSignleTInstCount + 2 * codeInfo.highPInstCount) > 1023 ||
        (dual16Mode != DUAL16_FORCE_ON &&
         !Shader->__IsMasterDual16Shader &&     /*in recompilation, if the master shader is dual16, force the shader to be dual16 if possible */
         1.5 * (codeInfo.runSignleTInstCount + codeInfo.highPInstCount) > codeInfo.estimatedInst)
        )
        return gcvFALSE;

    /* In dual16, we need to put all HP attributes in front of MP attributes */
    _SortAttributesOfDual16Shader(Shader, pHwCfg);

    return gcvTRUE;
}

VSC_ErrCode
VIR_Shader_CalcSamplerCount(
    IN      VIR_Shader *         Shader,
    IN OUT  gctINT*              SamplerCount
    )
{
    VSC_ErrCode                errCode = VSC_ERR_NONE;
    gctINT                     i;
    gctINT                     samplers = 0;

    for (i = 0; i < (gctINT) VIR_IdList_Count(&Shader->uniforms); ++i)
    {
        VIR_Id      id  = VIR_IdList_GetId(&Shader->uniforms, i);
        VIR_Symbol  *sym = VIR_Shader_GetSymFromId(Shader, id);
        VIR_Uniform *symUniform = gcvNULL;

        if (!VIR_Symbol_isSampler(sym) ||
            (VIR_Symbol_GetIndex(sym) == VIR_Shader_GetBaseSamplerId(Shader)))
        {
            continue;
        }

        symUniform = VIR_Symbol_GetSampler(sym);

        if (symUniform == gcvNULL)
        {
            continue;
        }

        /* If this texture is not used on shader, we can skip it. */
        if (!(isSymUniformUsedInShader(sym)) &&
            !(isSymUniformUsedInTextureSize(sym)))
        {
            continue;
        }

        if (symUniform->realUseArraySize == -1)
        {
            VIR_Type    *symType = VIR_Symbol_GetType(sym);

            if (VIR_Type_GetKind(symType) == VIR_TY_ARRAY)
            {
                symUniform->realUseArraySize = VIR_Type_GetArrayLength(symType);
            }
            else
            {
                symUniform->realUseArraySize = 1;
            }
        }

        if (VIR_Type_GetKind(sym->type) == VIR_TY_ARRAY)
        {
            samplers += (gctINT)VIR_Type_GetArrayLength(sym->type);
        }
        else
        {
            samplers += 1;
        }
    }

    if (SamplerCount)
    {
        *SamplerCount = samplers;
    }

    return errCode;
}

