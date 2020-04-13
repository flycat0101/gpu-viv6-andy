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

/* compiler time checks */
/* compiler time assert if sizeof(VIR_Operand) larger than VIR_Operand */
char _check_operand_size[sizeof(VIR_Operand) <= sizeof(VIR_Operand)];

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
    ".nothelper", /* 0x19 0x19 */
    ".eq_uq",
    ".ne_uq",
    ".lt_uq",
    ".gt_uq",
    ".le_uq",
    ".ge_uq",
    "!$%!@$$",
};

char _check_CondOpName_size[sizeof(VIR_CondOpName)/sizeof(gctSTRING) == VIR_COP_MAX + 1];

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
    ".neg", /* VIR_MOD_NEG: source negate modifier  0x1 */
    ".abs", /* VIR_MOD_ABS: source absolute modfier 0x2*/
    ".abs.neg", /* VIR_MOD_ABS | VIR_MOD_NEG: -abs(src) 0x3*/
    ".x3", /* VIR_MOD_X3                           0x4 */
    ".x3.neg", /* VIR_MOD_X3  | VIR_MOD_NEG            0x5 */
    ".x3.abs", /* VIR_MOD_X3  | VIR_MOD_ABS            0x6 */
    ".x3.abs.neg", /* VIR_MOD_X3  | VIR_MOD_NEG |  VIR_MOD_ABS   0x7 */
};

/* source modifiers */
const gctSTRING VIR_ComplexSrcModifierName[] =
{
    "", /* VIR_MOD_NONE     */
    ".neg", /* VIR_MOD_NEG: source negate modifier  0x1 */
    ".conj", /* VIR_MOD_ABS: source absolute modfier 0x2 */
    ".conj.neg", /* VIR_MOD_ABS | VIR_MOD_NEG: -conj(src) 0x3*/
    ".x3", /* VIR_MOD_X3                           0x4 */
    ".x3.neg", /* VIR_MOD_X3  | VIR_MOD_NEG            0x5 */
    ".x3.conj", /* VIR_MOD_X3  | VIR_MOD_ABS            0x6 */
    ".x3.conj.neg", /* VIR_MOD_X3  | VIR_MOD_NEG |  VIR_MOD_ABS   0x7 */
};

/* type table operations */
VIR_TypeId vscAddPrimTypeToTable(VIR_TypeTable* pTypeTbl, const char* pStr, gctUINT len);

/* builtin special names */
VIR_NameId  VIR_NAME_UNKNOWN,
    VIR_NAME_POSITION,
    VIR_NAME_POINT_SIZE,
    VIR_NAME_CLIP_DISTANCE,
    VIR_NAME_CULL_DISTANCE,
    VIR_NAME_COLOR,
    VIR_NAME_FRONT_FACING,
    VIR_NAME_POINT_COORD,
    VIR_NAME_POSITION_W,
    VIR_NAME_DEPTH,
    VIR_NAME_FOG_COORD,
    VIR_NAME_VERTEX_ID, /* gl_VertexID, for ES. */
    VIR_NAME_VERTEX_INDEX, /* gl_VertexIndex, for Vulkan only, where the indexing is relative to some base offset. */
    VIR_NAME_FRONT_COLOR,
    VIR_NAME_BACK_COLOR,
    VIR_NAME_FRONT_SECONDARY_COLOR,
    VIR_NAME_BACK_SECONDARY_COLOR,
    VIR_NAME_TEX_COORD,
    VIR_NAME_INSTANCE_ID, /* gl_InstanceID, for ES. */
    VIR_NAME_INSTANCE_INDEX, /* gl_InstanceIndex, for Vulkan only, where the indexing is relative to some base offset. */
    VIR_NAME_DEVICE_INDEX, /* gl_DeviceIndex, for Vulkan only. */
    VIR_NAME_NUM_GROUPS,
    VIR_NAME_WORKGROUPSIZE,
    VIR_NAME_WORK_GROUP_ID,
    VIR_NAME_WORK_GROUP_INDEX,
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
    VIR_NAME_IN_POSITION, /* gl_in.gl_Position */
    VIR_NAME_IN_POINT_SIZE, /* gl_in.gl_PointSize */
    VIR_NAME_IN_CLIP_DISTANCE, /* gl_in.gl_ClipDistance */
    VIR_NAME_IN_CULL_DISTANCE, /* gl_in.gl_CullDistance */
    VIR_NAME_BOUNDING_BOX, /* gl_BoundingBox */
    VIR_NAME_LAST_FRAG_DATA, /* gl_LastFragData */
    VIR_NAME_CLUSTER_ID, /* #cluster_id */
    VIR_NAME_SUBGROUP_NUM, /* gl_NumSubgroups */
    VIR_NAME_SUBGROUP_SIZE, /* gl_SubgroupSize */
    VIR_NAME_SUBGROUP_ID, /* gl_SubgroupID */
    VIR_NAME_SUBGROUP_INVOCATION_ID, /* gl_SubgroupInvocationID */
    VIR_NAME_VIEW_INDEX, /* gl_ViewIndex */
    VIR_NAME_BUILTIN_LAST;

VIR_BuiltinTypeInfo VIR_builtinTypes[] =
{
#include "gc_vsc_vir_builtin_types.def.h"
};
/* compile-time assertion if the VIR_builtinTypes is not the same length as VIR_TYPE_PRIMITIVETYPE_COUNT */
const gctINT _verify_builtinTypes[sizeof(VIR_builtinTypes)/sizeof(VIR_BuiltinTypeInfo) == VIR_TYPE_PRIMITIVETYPE_COUNT+1] = { 0 };

VIR_BuiltinTypeInfo*
    VIR_Shader_GetBuiltInTypes(
    IN  VIR_TypeId          TypeId
    )
{
    return &VIR_builtinTypes[TypeId];
}

/* should be synchronized with gcsl.std.450.h */
static VIR_IntrinsicsKind GLSL_STD_450[] =
{
    VIR_IK_NONE, /* GLSLstd450Bad = 0, */
    VIR_IK_roundEven, /* GLSLstd450Round = 1, and use roundEven to implement round.*/
    VIR_IK_roundEven, /* GLSLstd450RoundEven = 2,*/
    VIR_IK_trunc, /* GLSLstd450Trunc = 3, */
    VIR_IK_abs, /* GLSLstd450FAbs = 4, */
    VIR_IK_abs, /* GLSLstd450SAbs = 5, */
    VIR_IK_sign, /* GLSLstd450FSign = 6, */
    VIR_IK_sign, /* GLSLstd450SSign = 7, */
    VIR_IK_floor, /* GLSLstd450Floor = 8, */
    VIR_IK_ceil, /* GLSLstd450Ceil = 9, */
    VIR_IK_fract, /* GLSLstd450Fract = 10, */
    VIR_IK_radians, /* GLSLstd450Radians = 11, */
    VIR_IK_degrees, /* GLSLstd450Degrees = 12, */
    VIR_IK_sin, /* GLSLstd450Sin = 13, */
    VIR_IK_cos, /* GLSLstd450Cos = 14, */
    VIR_IK_tan, /* GLSLstd450Tan = 15, */
    VIR_IK_asin, /* GLSLstd450Asin = 16, */
    VIR_IK_acos, /* GLSLstd450Acos = 17, */
    VIR_IK_atan, /* GLSLstd450Atan = 18, */
    VIR_IK_sinh, /* GLSLstd450Sinh = 19, */
    VIR_IK_cosh, /* GLSLstd450Cosh = 20, */
    VIR_IK_tanh, /* GLSLstd450Tanh = 21, */
    VIR_IK_asinh, /* GLSLstd450Asinh = 22, */
    VIR_IK_acosh, /* GLSLstd450Acosh = 23, */
    VIR_IK_atanh, /* GLSLstd450Atanh = 24, */
    VIR_IK_atan2, /* GLSLstd450Atan2 = 25, */
    VIR_IK_pow, /* GLSLstd450Pow = 26, */
    VIR_IK_exp, /* GLSLstd450Exp = 27, */
    VIR_IK_log, /* GLSLstd450Log = 28, */
    VIR_IK_exp2, /* GLSLstd450Exp2 = 29, */
    VIR_IK_log2, /* GLSLstd450Log2 = 30, */
    VIR_IK_sqrt, /* GLSLstd450Sqrt = 31, */
    VIR_IK_inversesqrt, /* GLSLstd450InverseSqrt = 32, */
    VIR_IK_determinant, /* GLSLstd450Determinant = 33, */
    VIR_IK_matrixinverse, /* GLSLstd450MatrixInverse = 34, */
    VIR_IK_modf, /* GLSLstd450Modf = 35, */
    VIR_IK_modfstruct, /* GLSLstd450ModfStruct = 36, */
    VIR_IK_min, /* GLSLstd450FMin = 37, */
    VIR_IK_min, /* GLSLstd450UMin = 39, */
    VIR_IK_min, /* GLSLstd450SMin = 40, */
    VIR_IK_max, /* GLSLstd450FMax = 41, */
    VIR_IK_max, /* GLSLstd450UMax = 43, */
    VIR_IK_max, /* GLSLstd450SMax = 44, */
    VIR_IK_clamp, /* GLSLstd450FClamp = 45, */
    VIR_IK_clamp, /* GLSLstd450UClamp = 47, */
    VIR_IK_clamp, /* GLSLstd450SClamp = 48, */
    VIR_IK_mix, /* GLSLstd450FMix = 49, */
    VIR_IK_mix, /* GLSLstd450IMix = 50, */
    VIR_IK_step, /* GLSLstd450Step = 51, */
    VIR_IK_smoothstep, /* GLSLstd450SmoothStep = 52, */
    VIR_IK_fma, /* GLSLstd450Fma = 53, */
    VIR_IK_frexp, /* GLSLstd450Frexp = 54, */
    VIR_IK_frexpstruct, /* GLSLstd450FrexpStruct = 55, */
    VIR_IK_ldexp, /* GLSLstd450Ldexp = 56, */
    VIR_IK_packSnorm4x8, /* GLSLstd450PackSnorm4x8 = 57, */
    VIR_IK_packUnorm4x8, /* GLSLstd450PackUnorm4x8 = 58, */
    VIR_IK_packSnorm2x16, /* GLSLstd450PackSnorm2x16 = 59, */
    VIR_IK_packUnorm2x16, /* GLSLstd450PackUnorm2x16 = 60, */
    VIR_IK_packHalf2x16, /* GLSLstd450PackHalf2x16 = 61, */
    VIR_IK_packDouble2x32, /* GLSLstd450PackDouble2x32 = 62, */
    VIR_IK_unpackSnorm2x16,/* GLSLstd450UnpackSnorm2x16 = 63, */
    VIR_IK_unpackUnorm2x16,/* GLSLstd450UnpackUnorm2x16 = 64, */
    VIR_IK_unpackHalf2x16, /* GLSLstd450UnpackHalf2x16 = 65, */
    VIR_IK_unpackSnorm4x8, /* GLSLstd450UnpackSnorm4x8 = 66, */
    VIR_IK_unpackUnorm4x8, /* GLSLstd450UnpackUnorm4x8 = 67, */
    VIR_IK_unpackDouble2x32,/* GLSLstd450UnpackDouble2x32 = 68, */
    VIR_IK_length, /* GLSLstd450Length = 69, */
    VIR_IK_distance, /* GLSLstd450Distance = 70, */
    VIR_IK_cross, /* GLSLstd450Cross = 71, */
    VIR_IK_normalize, /* GLSLstd450Normalize = 72, */
    VIR_IK_faceforward, /* GLSLstd450FaceForward = 73, */
    VIR_IK_reflect, /* GLSLstd450Reflect = 74, */
    VIR_IK_refract, /* GLSLstd450Refract = 75, */
    VIR_IK_findlsb, /* GLSLstd450FindILsb = 76, */
    VIR_IK_findmsb, /* GLSLstd450FindSMsb = 77, */
    VIR_IK_findmsb, /* GLSLstd450FindUMsb = 78, */
    VIR_IK_interpolateAtCentroid, /* GLSLstd450InterpolateAtCentroid = 79, */
    VIR_IK_interpolateAtSample, /* GLSLstd450InterpolateAtSample = 80, */
    VIR_IK_interpolateAtOffset, /* GLSLstd450InterpolateAtOffset = 81, */
    VIR_IK_nmin, /* GLSLstd450NMin = 38, */
    VIR_IK_nmax, /* GLSLstd450NMax = 42, */
    VIR_IK_nclamp, /* GLSLstd450NClamp = 46, */


    /* GLSLstd450Count */
};
/*char _checkGLSLSTD450Size[(sizeof(GLSL_STD_450) / sizeof(VIR_IntrinsicsKind)) == GLSLstd450Count];*/

/* intrinsic from the SpvOp*, generated by compiler */
static VIR_IntrinsicsKind INTERNAL_INTRINSIC[] =
{
    VIR_IK_transpose, /* Transpose = 0, Transfered from SpvOpTranspose */
    VIR_IK_image_store,
    VIR_IK_image_load,
    VIR_IK_vecGet,
    VIR_IK_vecSet,
    VIR_IK_uaddCarry,
    VIR_IK_usubBorrow,
    VIR_IK_umulExtended,
    VIR_IK_imulExtended,
    VIR_IK_convF32ToF16,
    VIR_IK_quantizeToF16,
    VIR_IK_image_fetch,
    VIR_IK_image_addr,
    VIR_IK_image_query_format,
    VIR_IK_image_query_order,
    VIR_IK_image_query_size_lod,
    VIR_IK_image_query_size,
    VIR_IK_image_query_lod,
    VIR_IK_image_query_levels,
    VIR_IK_image_query_samples,
    /* Make sure add a new intrinsic kind from here. */
    VIR_IK_image_fetch_for_sampler,
    VIR_IK_texld,
    VIR_IK_texldpcf,
    VIR_IK_texld_proj,
    VIR_IK_texld_gather,
    VIR_IK_texld_fetch_ms,
    VIR_IK_image_query_size_for_sampler
};

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
            if ((VIR_Type_GetArrayLength(type1) == VIR_Type_GetArrayLength(type2)) &&
                (VIR_Type_GetArrayStride(type1) == VIR_Type_GetArrayStride(type2)))
            {
                return gcvTRUE;
            }
            else
            {
                return gcvFALSE;
            }
        case VIR_TY_FUNCTION:
            /* function parameters type must equal */
            return _sameParameterTypes(type1, type2);
        case VIR_TY_STRUCT:
        case VIR_TY_TYPEDEF:
        case VIR_TY_ENUM:
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
        if (VIR_GetTypeRows(c1->type) == 1)
        {
            return  (c1->value.vecVal.u32Value[3] == c2->value.vecVal.u32Value[3]) &&
                (c1->value.vecVal.u32Value[2] == c2->value.vecVal.u32Value[2]) &&
                (c1->value.vecVal.u32Value[1] == c2->value.vecVal.u32Value[1]) &&
                (c1->value.vecVal.u32Value[0] == c2->value.vecVal.u32Value[0]);
        }
        else if (VIR_GetTypeRows(c1->type) == 2)
        {
            return  (c1->value.vecVal.u32Value[7] == c2->value.vecVal.u32Value[7]) &&
                (c1->value.vecVal.u32Value[6] == c2->value.vecVal.u32Value[6]) &&
                (c1->value.vecVal.u32Value[5] == c2->value.vecVal.u32Value[5]) &&
                (c1->value.vecVal.u32Value[4] == c2->value.vecVal.u32Value[4]) &&
                (c1->value.vecVal.u32Value[3] == c2->value.vecVal.u32Value[3]) &&
                (c1->value.vecVal.u32Value[2] == c2->value.vecVal.u32Value[2]) &&
                (c1->value.vecVal.u32Value[1] == c2->value.vecVal.u32Value[1]) &&
                (c1->value.vecVal.u32Value[0] == c2->value.vecVal.u32Value[0]);
        }
        else if (VIR_GetTypeRows(c1->type) == 4)
        {
            return  (c1->value.vecVal.u32Value[15] == c2->value.vecVal.u32Value[15]) &&
                (c1->value.vecVal.u32Value[14] == c2->value.vecVal.u32Value[14]) &&
                (c1->value.vecVal.u32Value[13] == c2->value.vecVal.u32Value[13]) &&
                (c1->value.vecVal.u32Value[12] == c2->value.vecVal.u32Value[12]) &&
                (c1->value.vecVal.u32Value[11] == c2->value.vecVal.u32Value[11]) &&
                (c1->value.vecVal.u32Value[10] == c2->value.vecVal.u32Value[10]) &&
                (c1->value.vecVal.u32Value[9] == c2->value.vecVal.u32Value[9]) &&
                (c1->value.vecVal.u32Value[8] == c2->value.vecVal.u32Value[8]) &&
                (c1->value.vecVal.u32Value[7] == c2->value.vecVal.u32Value[7]) &&
                (c1->value.vecVal.u32Value[6] == c2->value.vecVal.u32Value[6]) &&
                (c1->value.vecVal.u32Value[5] == c2->value.vecVal.u32Value[5]) &&
                (c1->value.vecVal.u32Value[4] == c2->value.vecVal.u32Value[4]) &&
                (c1->value.vecVal.u32Value[3] == c2->value.vecVal.u32Value[3]) &&
                (c1->value.vecVal.u32Value[2] == c2->value.vecVal.u32Value[2]) &&
                (c1->value.vecVal.u32Value[1] == c2->value.vecVal.u32Value[1]) &&
                (c1->value.vecVal.u32Value[0] == c2->value.vecVal.u32Value[0]);
        }
        else
        {
            gctUINT i;

            /* Only 64bit type comes here. */
            gcmASSERT(VIR_GetTypeSize(VIR_GetTypeComponentType(c1->type)) >= 8);

            for (i = 0; i < VIR_GetTypeLogicalComponents(c1->type); i++)
            {
                if (c1->value.vecVal.u64Value[i] != c2->value.vecVal.u64Value[i])
                {
                    return gcvFALSE;
                }
            }

            return gcvTRUE;
        }
    }
    return gcvFALSE;
}

gctUINT
vscHFUNC_Symbol(const char *Str)
{
    VIR_Symbol *sym = (VIR_Symbol *)Str;
    VIR_TypeId  typeId;
    gctUINT32 hashVal = 0;

    switch (VIR_Symbol_GetKind(sym))
    {
    case VIR_SYM_UNIFORM:
    case VIR_SYM_UBO:            /* uniform block object */
    case VIR_SYM_VARIABLE:       /* global/local variables, input/output */
    case VIR_SYM_SBO:         /* buffer variables */
    case VIR_SYM_FUNCTION:       /* function */
    case VIR_SYM_SAMPLER:
    case VIR_SYM_SAMPLER_T:
    case VIR_SYM_TEXTURE:
    case VIR_SYM_IMAGE:
    case VIR_SYM_IMAGE_T:
    case VIR_SYM_TYPE:           /* typedef */
    case VIR_SYM_LABEL:
    case VIR_SYM_IOBLOCK:
        hashVal = VIR_Symbol_GetName(sym) | (VIR_Symbol_GetKind(sym) << 20);
        break;
    case VIR_SYM_FIELD:          /* the field of class/struct/union/ubo */
        typeId = VIR_Symbol_GetStructTypeId(sym);
        gcmASSERT(typeId != VIR_INVALID_ID);
        hashVal = VIR_Symbol_GetName(sym)          |
            (typeId << 10)                   |
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
        case VIR_SYM_SAMPLER_T:
        case VIR_SYM_TEXTURE:
        case VIR_SYM_IMAGE:
        case VIR_SYM_IMAGE_T:
        case VIR_SYM_TYPE:
        case VIR_SYM_LABEL:
        case VIR_SYM_IOBLOCK:
            /* these symbols are the same if they have the same name */
            return VIR_Symbol_GetName(sym1) == VIR_Symbol_GetName(sym2);
        case VIR_SYM_FIELD:
            /* name and enclosing type must be the same */
            return VIR_Symbol_GetName(sym1) == VIR_Symbol_GetName(sym2) &&
                (VIR_Symbol_GetStructTypeId(sym1) == VIR_Symbol_GetStructTypeId(sym2));
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

gctUINT
vscHFUNC_VirReg(const char *Str)
{
    gctUINT32 hashVal = (VIR_VirRegId)(gctUINTPTR_T)Str;
    return (hashVal & 0x7FFFFFFF);

}

gctBOOL
vcsHKCMP_VirReg(const char *Str1, const char *Str2)
{
    return  ((VIR_VirRegId)(gctUINTPTR_T)Str1 == (VIR_VirRegId)(gctUINTPTR_T)Str2);
}

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
    VIR_OpCode opCode = VIR_Inst_GetOpcode(pInst);

    gcmASSERT(VIR_Operand_GetOpKind(pOpnd) == VIR_OPND_EVIS_MODIFIER);
    evisModifier.u1 = VIR_Operand_GetEvisModifier(pOpnd);

    if (VIR_Inst_GetSrcNum(pInst) > 0 && !VIR_Operand_isUndef(VIR_Inst_GetSource(pInst, 0)))
    {
        evisModifier.u0.src0Format = VIR_GetOpernadVXFormat(VIR_Inst_GetSource(pInst, 0));
    }
    if (VIR_Inst_GetSrcNum(pInst) > 1 && !VIR_Operand_isUndef(VIR_Inst_GetSource(pInst, 1)))
    {
        evisModifier.u0.src1Format = VIR_GetOpernadVXFormat(VIR_Inst_GetSource(pInst, 1));
    }
    if (VIR_Inst_GetSrcNum(pInst) > 2 && !VIR_Operand_isUndef(VIR_Inst_GetSource(pInst, 2)))
    {
        evisModifier.u0.src2Format = VIR_GetOpernadVXFormat(VIR_Inst_GetSource(pInst, 2));
    }

    state.u1 = 0;
    switch (opCode) {
    case VIR_OP_VX_ABSDIFF:
        state.u2.roundingMode = evisModifier.u0.round_OfstMode;
        break;
    case VIR_OP_VX_IADD:
        state.u3.src0Format = evisModifier.u0.src0Format;
        state.u3.src1Format = evisModifier.u0.src1Format;
        state.u3.src2Format = evisModifier.u0.src2Format;
        if (evisModifier.u0.clamp)
        {
            /* vx_iadd use instruction sat bit for clamping */
            VIR_Operand_SetModifier(VIR_Inst_GetDest(pInst), VIR_MOD_SAT_0_TO_1);
        }
        break;
    case VIR_OP_VX_IACCSQ:
        state.u4.src1Format = evisModifier.u0.src1Format;
        state.u4.signExt    = evisModifier.u0.signExt;
        break;
    case VIR_OP_VX_LERP:
        state.u5.src0Format = evisModifier.u0.src0Format;
        state.u5.src1Format = evisModifier.u0.src1Format;
        state.u5.clampSrc2  = evisModifier.u0.clamp;
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
            (evisModifier.u0.round_OfstMode == VX_RM_ToNearestEven) ? 1 : 0;
        break;
    case VIR_OP_VX_DP16X1:
    case VIR_OP_VX_DP8X2:
    case VIR_OP_VX_DP4X4:
    case VIR_OP_VX_DP2X8:
        state.u9.srcFormat0 = evisModifier.u0.src0Format;
        state.u9.srcFormat1 = evisModifier.u0.src1Format;
        state.u9.rounding   = evisModifier.u0.round_OfstMode;
        break;
    case VIR_OP_VX_DP16X1_B:
    case VIR_OP_VX_DP8X2_B:
    case VIR_OP_VX_DP4X4_B:
    case VIR_OP_VX_DP2X8_B:
        state.u9.srcFormat0 = evisModifier.u0.src0Format;
        state.u9.srcFormat1 = evisModifier.u0.src2Format;
        state.u9.rounding   = evisModifier.u0.round_OfstMode;
        break;
    case VIR_OP_VX_DP32X1:
    case VIR_OP_VX_DP16X2:
    case VIR_OP_VX_DP8X4:
    case VIR_OP_VX_DP4X8:
    case VIR_OP_VX_DP2X16:
        state.u10.srcFormat01 = evisModifier.u0.src0Format;
        state.u10.rounding    = evisModifier.u0.round_OfstMode;
        state.u10.srcFormat1  = evisModifier.u0.src1Format;
        break;
    case VIR_OP_VX_DP32X1_B:
    case VIR_OP_VX_DP16X2_B:
    case VIR_OP_VX_DP8X4_B:
    case VIR_OP_VX_DP4X8_B:
    case VIR_OP_VX_DP2X16_B:
        state.u10.srcFormat01 = evisModifier.u0.src0Format;
        state.u10.rounding    = evisModifier.u0.round_OfstMode;
        state.u10.srcFormat1  = evisModifier.u0.src2Format;
        break;
    case VIR_OP_VX_CLAMP:
        state.u11.srcFormat   = evisModifier.u0.src0Format;
        state.u11.enableBool  = evisModifier.u0.enableBool ;
        break;
    case VIR_OP_VX_BILINEAR:
        state.u12.srcFormat01 = evisModifier.u0.src0Format;
        state.u12.srcFormat2  = evisModifier.u0.src2Format;
        state.u12.clampSrc2   = evisModifier.u0.clamp;
        break;
    case VIR_OP_VX_SELECTADD:
    case VIR_OP_VX_INDEXADD:
        state.u13.srcFormat0  = evisModifier.u0.src0Format;
        state.u13.srcFormat1  = evisModifier.u0.src1Format;
        break;
    case VIR_OP_VX_ATOMICADD:
        state.u14.srcFormat2  = evisModifier.u0.src2Format;
        break;
    case VIR_OP_VX_GATHER:
    case VIR_OP_VX_GATHER_B:
    case VIR_OP_VX_SCATTER:
    case VIR_OP_VX_SCATTER_B:
    case VIR_OP_VX_ATOMIC_S:
    case VIR_OP_VX_ATOMIC_S_B:
        state.u15.offsetType  = evisModifier.u0.filterMode & 0x7;
        if (opCode == VIR_OP_VX_ATOMIC_S ||
            opCode == VIR_OP_VX_ATOMIC_S_B)
        {
            state.u15.atomicOp = ((evisModifier.u0.filterMode & 0x8) >> 3)  |
                                 (evisModifier.u0.rangePi << 1)             |
                                 (evisModifier.u0.preAdjust << 2);
        }
        break;
    case VIR_OP_VX_BITEXTRACT:
    case VIR_OP_VX_BITREPLACE:
    case VIR_OP_VX_IMG_LOAD:
    case VIR_OP_VX_IMG_STORE:
    case VIR_OP_VX_IMG_LOAD_3D:
    case VIR_OP_VX_IMG_STORE_3D:
    case VIR_OP_VX_VERTMIN3:
    case VIR_OP_VX_VERTMAX3:
    case VIR_OP_VX_VERTMED3:
    case VIR_OP_VX_HORZMIN3:
    case VIR_OP_VX_HORZMAX3:
    case VIR_OP_VX_HORZMED3:
        /* nothing in state */
        break;
    default:
        gcmASSERT(gcvFALSE);
        break;
    }
    return state.u1;
}

VIR_Operand *
VIR_Inst_GetEvisModiferOpnd(
    IN VIR_Instruction *pInst
    )
{
    gctUINT i;

    if (VIR_OPCODE_isVX(VIR_Inst_GetOpcode(pInst)))
    {
        /* find EvisModifier operand  */
        for (i = 0; i < VIR_Inst_GetSrcNum(pInst); i ++)
        {
            VIR_Operand * opnd = VIR_Inst_GetSource(pInst, i);
            if (opnd && VIR_Operand_GetOpKind(opnd) == VIR_OPND_EVIS_MODIFIER)
            {
                return opnd;
            }
        }
    }
    return gcvNULL;
}

static VIR_RES_OP_TYPE
_CheckTexldParam(
    IN VIR_RES_OP_TYPE      ResOpType,
    IN VIR_Operand         *pOpnd
    )
{
    VIR_RES_OP_TYPE         resOpType = ResOpType;
    VIR_Operand *texldModifier;

    if (pOpnd == gcvNULL)
    {
        return resOpType;
    }

    texldModifier = (VIR_Operand *)pOpnd;

    if (VIR_Operand_hasBiasFlag(pOpnd))
    {
        switch (resOpType)
        {
        case VIR_RES_OP_TYPE_TEXLD:
            resOpType = VIR_RES_OP_TYPE_TEXLD_BIAS;
            break;

        case VIR_RES_OP_TYPE_TEXLD_PCF:
            resOpType = VIR_RES_OP_TYPE_TEXLD_BIAS_PCF;
            break;

        case VIR_RES_OP_TYPE_TEXLDP:
            resOpType = VIR_RES_OP_TYPE_TEXLDP_BIAS;
            break;

        default:
            break;
        }
    }
    else if (VIR_Operand_hasLodFlag(pOpnd))
    {
        switch (resOpType)
        {
        case VIR_RES_OP_TYPE_TEXLD:
            resOpType = VIR_RES_OP_TYPE_TEXLD_LOD;
            break;

        case VIR_RES_OP_TYPE_TEXLD_PCF:
            resOpType = VIR_RES_OP_TYPE_TEXLD_LOD_PCF;
            break;

        case VIR_RES_OP_TYPE_TEXLDP:
            resOpType = VIR_RES_OP_TYPE_TEXLDP_LOD;
            break;

        default:
            break;
        }
    }
    else if (VIR_Operand_hasGradFlag(pOpnd))
    {
        switch (resOpType)
        {
        case VIR_RES_OP_TYPE_TEXLD:
            resOpType = VIR_RES_OP_TYPE_TEXLD_GRAD;
            break;

        case VIR_RES_OP_TYPE_TEXLDP:
            resOpType = VIR_RES_OP_TYPE_TEXLDP_GRAD;
            break;

        default:
            break;
        }
    }
    else if (VIR_Operand_hasGatherFlag(pOpnd))
    {
        if (VIR_Operand_GetTexldModifier(texldModifier, VIR_TEXLDMODIFIER_GATHERREFZ) != gcvNULL)
        {
            resOpType = VIR_RES_OP_TYPE_GATHER_PCF;
        }
        else
        {
            resOpType = VIR_RES_OP_TYPE_GATHER;
        }
    }
    else if (VIR_Operand_hasFetchMSFlag(pOpnd))
    {
        resOpType = VIR_RES_OP_TYPE_FETCH_MS;
    }

    return resOpType;
}

VSC_ErrCode
VIR_Inst_UpdateResOpType(
    IN VIR_Instruction     *pInst
    )
{
    VSC_ErrCode             errCode = VSC_ERR_NONE;
    VIR_RES_OP_TYPE         resOpType = VIR_Inst_GetResOpType(pInst);
    VIR_OpCode              opCode = VIR_Inst_GetOpcode(pInst);
    VIR_Operand            *samplerOperand = gcvNULL;
    VIR_Operand            *texldParamOpnd = gcvNULL;
    VIR_ParmPassing        *paramOpnd = gcvNULL;
    VIR_IntrinsicsKind      intrinsicKind = VIR_IK_NONE;
    gctBOOL                 isIntrinsic = (opCode == VIR_OP_INTRINSIC);

    /* Already set, just skip. */
    if (resOpType != VIR_RES_OP_TYPE_UNKNOWN)
    {
        return errCode;
    }

    /* Skip none texld-related opcode. */
    if (isIntrinsic)
    {
        intrinsicKind = VIR_Operand_GetIntrinsicKind(VIR_Inst_GetSource(pInst, 0));

        if (!VIR_Intrinsics_isTexLdRelated(intrinsicKind) &&
            !VIR_Intrinsics_isImageFetch(intrinsicKind))
        {
            return errCode;
        }
    }
    else if (!VIR_OPCODE_isTexLd(opCode))
    {
        return errCode;
    }

    /* Get texld parameter and set resOpType by opcode. */
    if (VIR_OPCODE_isTexLd(opCode))
    {
        texldParamOpnd = VIR_Inst_GetSource(pInst, 2);

        switch (opCode)
        {
        case VIR_OP_TEXLD:
        case VIR_OP_TEXLD_U:
            resOpType = VIR_RES_OP_TYPE_TEXLD;
            break;

        case VIR_OP_TEXLDPCF:
            resOpType = VIR_RES_OP_TYPE_TEXLD_PCF;
            break;

        case VIR_OP_TEXLD_BIAS:
        case VIR_OP_TEXLD_U_BIAS:
            resOpType = VIR_RES_OP_TYPE_TEXLD_BIAS;
            break;

        case VIR_OP_TEXLD_BIAS_PCF:
            resOpType = VIR_RES_OP_TYPE_TEXLD_BIAS_PCF;
            break;

        case VIR_OP_TEXLDL:
        case VIR_OP_TEXLD_LOD:
        case VIR_OP_TEXLD_U_LOD:
            resOpType = VIR_RES_OP_TYPE_TEXLD_LOD;
            break;

        case VIR_OP_TEXLD_LOD_PCF:
            resOpType = VIR_RES_OP_TYPE_TEXLD_LOD_PCF;
            break;

        case VIR_OP_TEXLDPROJ:
        case VIR_OP_TEXLDPCFPROJ:
            resOpType = VIR_RES_OP_TYPE_TEXLDP;
            break;

        case VIR_OP_TEXLD_GATHER:
            resOpType = VIR_RES_OP_TYPE_GATHER;
            break;

        case VIR_OP_TEXLD_GATHER_PCF:
            resOpType = VIR_RES_OP_TYPE_GATHER_PCF;
            break;

        default:
            resOpType = VIR_RES_OP_TYPE_UNKNOWN;
            break;
        }
    }
    else
    {
        gcmASSERT(isIntrinsic);
        paramOpnd = VIR_Operand_GetParameters(VIR_Inst_GetSource(pInst, 1));
        if (paramOpnd->argNum > 2)
        {
            texldParamOpnd = paramOpnd->args[2];
        }
        samplerOperand = paramOpnd->args[1];

        switch (intrinsicKind)
        {
        case VIR_IK_texld:
        case VIR_IK_texldpcf:
            resOpType = VIR_RES_OP_TYPE_TEXLD;
            break;

        case VIR_IK_texld_proj:
            resOpType = VIR_RES_OP_TYPE_TEXLDP;
            break;

        case VIR_IK_texld_gather:
            resOpType = VIR_RES_OP_TYPE_GATHER;
            break;

        case VIR_IK_image_fetch:
            if (VIR_TypeId_isSamplerMS(VIR_Operand_GetTypeId(samplerOperand)))
            {
                resOpType = VIR_RES_OP_TYPE_FETCH_MS;
            }
            else
            {
                resOpType = VIR_RES_OP_TYPE_FETCH;
            }
            break;

        default:
            gcmASSERT(gcvFALSE);
            resOpType = VIR_RES_OP_TYPE_TEXLD;
            break;
        }
    }

    /* Update resOpType with texld parameter. */
    gcmASSERT(resOpType != VIR_RES_OP_TYPE_UNKNOWN);
    resOpType = _CheckTexldParam(resOpType, texldParamOpnd);

    /* Save the result. */
    gcmASSERT(resOpType != VIR_RES_OP_TYPE_UNKNOWN);
    VIR_Inst_SetResOpType(pInst, resOpType);

    return errCode;
}

/* Same logical to gcIsInstHWBarrier. */
gctBOOL
VIR_Inst_IsHWBarrier(
    IN VIR_Instruction*         pInst,
    IN gctBOOL                  bGenerateMC
    )
{
    VIR_Shader*                 pShader = VIR_Inst_GetShader(pInst);
    VIR_OpCode                  opCode = VIR_Inst_GetOpcode(pInst);
    VIR_Operand*                pScopeOpnd = VIR_Inst_GetSource(pInst, 0);
    VIR_Operand*                pMemorySemanticOpnd = VIR_Inst_GetSource(pInst, 1);
    VIR_MEMORY_SCOPE_TYPE       memoryScope = VIR_MEMORY_SCOPE_TYPE_WORKGROUP;
    VIR_MEMORY_SEMANTIC_FLAG    memorySemantic = VIR_MEMORY_SEMANTIC_ACQUIRERELEASE;

    if (!VIR_OPCODE_isBarrier(opCode))
    {
        return gcvFALSE;
    }

    /* Get the memory scope from source0. */
    if (pScopeOpnd && VIR_Operand_isImm(pScopeOpnd))
    {
        memoryScope = (VIR_MEMORY_SCOPE_TYPE)VIR_Operand_GetImmediateInt(pScopeOpnd);
    }

    /* Get the memory semantic from source1. */
    if (pMemorySemanticOpnd && VIR_Operand_isImm(pMemorySemanticOpnd))
    {
        memorySemantic = (VIR_MEMORY_SEMANTIC_FLAG)VIR_Operand_GetImmediateInt(pMemorySemanticOpnd);
    }

    /* So far always generate for BARRIER. */
    if (opCode == VIR_OP_BARRIER)
    {
        return gcvTRUE;
    }
    /* HW has logic to naturally insure memory access is in-order in queue scope. */
    else if (opCode == VIR_OP_MEM_BARRIER)
    {
        /* Only CS/CL and TCS can support BARRIER instruction. */
        if (bGenerateMC
            &&
            !(VIR_Shader_IsCL(pShader) || VIR_Shader_IsGlCompute(pShader) || VIR_Shader_IsTCS(pShader)))
        {
            return gcvFALSE;
        }

        if ((memoryScope == VIR_MEMORY_SCOPE_TYPE_WORKGROUP ||
             memoryScope == VIR_MEMORY_SCOPE_TYPE_DEVICE    ||
             memoryScope == VIR_MEMORY_SCOPE_TYPE_CROSS_DEVICE)
            &&
            ((memorySemantic & VIR_MEMORY_SEMANTIC_ACQUIRE) || (memorySemantic & VIR_MEMORY_SEMANTIC_ACQUIRERELEASE)))
        {
            return gcvTRUE;
        }
        else
        {
            return gcvFALSE;
        }
    }

    return gcvFALSE;
}

VSC_ErrCode
VIR_Uniform_UpdateResOpBitFromSampledImage(
    IN VIR_Shader* Shader,
    IN VIR_Uniform* SampledImageUniform,
    IN gctINT       Index,
    IN VIR_Uniform* DestUniform
    )
{
    VSC_ErrCode   errCode = VSC_ERR_NONE;
    VIR_Symbol*   sourceSym = VIR_Shader_GetSymFromId(Shader, SampledImageUniform->sym);
    VIR_Symbol*   destSym = VIR_Shader_GetSymFromId(Shader, DestUniform->sym);
    VIR_Type*     sourceType = VIR_Symbol_GetType(sourceSym);
    VIR_Type*     destType = VIR_Symbol_GetType(destSym);
    gctUINT       sampledImageArraySize, destArraySize, resOpBits;

    gcmASSERT(VIR_Uniform_isImage(DestUniform) || VIR_Uniform_isSampler(DestUniform));
    if (SampledImageUniform == gcvNULL || DestUniform == gcvNULL)
    {
        gcmASSERT(gcvFALSE);
        return errCode;
    }

    if (VIR_Uniform_GetResOpBitsArraySize(SampledImageUniform) == 0)
    {
        return errCode;
    }

    if (VIR_Type_GetKind(sourceType) == VIR_TY_ARRAY)
    {
        sampledImageArraySize = VIR_Type_GetArrayLength(sourceType);
    }
    else
    {
        sampledImageArraySize = 1;
    }

    if (VIR_Type_GetKind(destType) == VIR_TY_ARRAY)
    {
        destArraySize = VIR_Type_GetArrayLength(destType);
    }
    else
    {
        destArraySize = 1;
    }

    if (Index == NOT_ASSIGNED)
    {
        gcmASSERT(gcvFALSE);
        Index = 0;
    }
    else if (Index >= (gctINT)destArraySize)
    {
        gcmASSERT(gcvFALSE);
        Index = 0;
    }

    /*
    ** The array size of the sampled image can't be larger than the sampler/image.
    ** Actually from the spec, we can know that it can't be an array, the array size is always 1.
    */
    if (sampledImageArraySize > destArraySize || sampledImageArraySize != 1)
    {
        gcmASSERT(gcvFALSE);
    }

    gcmASSERT(sampledImageArraySize == VIR_Uniform_GetResOpBitsArraySize(SampledImageUniform));

    /* Create resOpBits. */
    if (VIR_Uniform_GetResOpBitsArray(DestUniform) == gcvNULL)
    {
        gctUINT32 * ptr = (gctUINT32*)vscMM_Alloc(&Shader->pmp.mmWrapper, sizeof(gctUINT32) * destArraySize);
        VIR_Uniform_SetResOpBitsArray(DestUniform, ptr);
        VIR_Uniform_SetResOpBitsArraySize(DestUniform, destArraySize);
        memset(ptr, 0, sizeof(gctUINT32) * destArraySize);
    }

    /* Update the resOpBits. */
    resOpBits =  VIR_Uniform_GetResOpBits(DestUniform, Index) | VIR_Uniform_GetResOpBits(SampledImageUniform, 0);
    VIR_Uniform_SetResOpBits(DestUniform, Index, resOpBits);

    return errCode;
}

VSC_ErrCode
VIR_Uniform_UpdateResOpBits(
    IN VIR_Shader* Shader,
    IN VIR_Uniform* Uniform,
    IN VIR_RES_OP_TYPE resOpType,
    IN gctUINT index
    )
{
    VSC_ErrCode   errCode = VSC_ERR_NONE;
    gctUINT       arraySize, i, resOpBits;
    VIR_Symbol*   sym = VIR_Shader_GetSymFromId(Shader, VIR_Uniform_GetSymID(Uniform));
    VIR_Type*     type = VIR_Symbol_GetType(sym);
    VIR_Symbol*   separateSamplerSym = gcvNULL;
    VIR_Symbol*   separateImageSym = gcvNULL;

    gcmASSERT(VIR_Uniform_isSampler(Uniform) || VIR_Uniform_isImage(Uniform));
    if (VIR_Type_GetKind(type) == VIR_TY_ARRAY)
    {
        arraySize = VIR_Type_GetArrayLength(type);
    }
    else
    {
        arraySize = 1;
    }

    if (VIR_Uniform_GetResOpBitsArray(Uniform) == gcvNULL)
    {
        gctUINT32 * ptr = (gctUINT32*)vscMM_Alloc(&Shader->pmp.mmWrapper, sizeof(gctUINT32) * arraySize);
        VIR_Uniform_SetResOpBitsArray(Uniform, ptr);
        VIR_Uniform_SetResOpBitsArraySize(Uniform, arraySize);
        memset(ptr, 0, sizeof(gctUINT32) * arraySize);
    }

    if (index == NOT_ASSIGNED)
    {
        for (i = 0; i < arraySize; i ++)
        {
            resOpBits =  VIR_Uniform_GetResOpBits(Uniform, i) | (1 << resOpType);
            VIR_Uniform_SetResOpBits(Uniform, i, resOpBits);
        }
    }
    else
    {
        gcmASSERT(index < arraySize);
        /* Update the resOpBits. */
        resOpBits =  VIR_Uniform_GetResOpBits(Uniform, index) | (1 << resOpType);
        VIR_Uniform_SetResOpBits(Uniform, index, resOpBits);
    }

    /* Copy the ResOpBits for sampled_image. */
    if (VIR_Symbol_GetUniformKind(sym) == VIR_UNIFORM_SAMPLED_IMAGE)
    {
        separateSamplerSym = VIR_Symbol_GetSeparateSampler(Shader, sym);
        if (separateSamplerSym != gcvNULL)
        {
            VIR_Uniform_UpdateResOpBitFromSampledImage(Shader,
                                                       Uniform,
                                                       VIR_Symbol_GetSamplerIdxRange(sym),
                                                       VIR_Symbol_GetSampler(separateSamplerSym));
        }

        separateImageSym = VIR_Symbol_GetSeparateImage(Shader, sym);
        if (separateImageSym != gcvNULL)
        {
            VIR_Uniform_UpdateResOpBitFromSampledImage(Shader,
                                                       Uniform,
                                                       VIR_Symbol_GetImgIdxRange(sym),
                                                       VIR_Symbol_GetImage(separateImageSym));
        }
    }

    return errCode;
}

VSC_ErrCode
VIR_Uniform_CheckImageFormatMismatch(
    IN VIR_Shader* Shader,
    IN VIR_Uniform* Uniform
    )
{
    VSC_ErrCode     errCode = VSC_ERR_NONE;
    VIR_Symbol*     pUniformSym = VIR_Shader_GetSymFromId(Shader, VIR_Uniform_GetSymID(Uniform));
    VIR_ImageFormat imageFormat = VIR_Symbol_GetImageFormat(pUniformSym);
    VIR_TypeId      imageSampledTypeId = VIR_Symbol_GetImageSampledType(pUniformSym);
    VIR_TypeId      imageComponentTypeId = VIR_TYPE_UNKNOWN;

    if (imageFormat == VIR_IMAGE_FORMAT_NONE)
    {
        return errCode;
    }

    if (imageSampledTypeId == VIR_TYPE_UNKNOWN)
    {
        return errCode;
    }

    imageComponentTypeId = VIR_ImageFormat_GetComponentTypeId(imageFormat);

    if ((VIR_TypeId_isFloat(imageSampledTypeId) && !VIR_TypeId_isFloat(imageComponentTypeId))
        ||
        (VIR_TypeId_isInteger(imageSampledTypeId) && !VIR_TypeId_isInteger(imageComponentTypeId)))
    {
        /*
        ** According vulkan spec:
        ** 14.5.2. Descriptor Set Interface
        ** ......
        ** The Sampled Type of an OpTypeImage declaration must match the numeric format of the
        ** corresponding resource in type and signedness, as shown in the SPIR-V Sampled Type column of the
        ** Interpretation of Numeric Format table, or the values obtained by reading or sampling from this
        ** image are undefined.
        */
        VIR_Symbol_SetFlagExt(pUniformSym, VIR_SYMUNIFORMFLAGEXT_IMAGE_FORMAT_MISMATCH);
        VIR_Shader_SetFlagExt1(Shader, VIR_SHFLAG_EXT1_IMAGE_FORMAT_MISMATCH);
    }

    return errCode;
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

gctBOOL
VIR_ConditionOp_EvaluateOneChannelConstantCondition(
    IN VIR_ConditionOp      COP,
    IN gctUINT              Src0Val,
    IN VIR_TypeId           Src0Type,
    IN gctUINT              Src1Val,
    IN VIR_TypeId           Src1Type
    )
{
    gctBOOL result = gcvFALSE;

    if(COP == VIR_COP_ALWAYS)
    {
        return gcvTRUE;
    }

    if (Src0Type == VIR_TYPE_FLOAT32 || Src1Type == VIR_TYPE_FLOAT32)
    {
        gctFLOAT f0, f1;

        f0 = *(gctFLOAT *)(&Src0Val);
        f1 = *(gctFLOAT *)(&Src1Val);

        switch (COP)
        {
        case VIR_COP_NOT_EQUAL:
            result = (f0 != f1); break;
        case VIR_COP_LESS_OR_EQUAL:
            result = (f0 <= f1); break;
        case VIR_COP_LESS:
            result = (f0 < f1); break;
        case VIR_COP_EQUAL:
            result = (f0 == f1); break;
        case VIR_COP_GREATER:
            result = (f0 > f1); break;
        case VIR_COP_GREATER_OR_EQUAL:
            result = (f0 >= f1); break;
        case VIR_COP_NOT_ZERO:
            result = (f0 != 0.0f); break;
        case VIR_COP_NOT:
            result = (f0 == 0.0f); break;
        case VIR_COP_GREATER_OR_EQUAL_ZERO:
            result = (f0 >= 0.0f); break;
        case VIR_COP_GREATER_ZERO:
            result = (f0 > 0.0f); break;
        case VIR_COP_LESS_OREQUAL_ZERO:
            result = (f0 <= 0.0f); break;
        case VIR_COP_LESS_ZERO:
            result = (f0 < 0.0f); break;
        case VIR_COP_SELMSB:
            result = Src0Val & 0x80000000; break;
        default:
            gcmASSERT(0);
        }
    }
    else if (VIR_TypeId_isSignedInteger(Src0Type) || VIR_TypeId_isSignedInteger(Src1Type))
    {
        gctINT32 i0 = *(gctINT *)(&Src0Val);
        gctINT32 i1 = *(gctINT *)(&Src1Val);

        switch (COP)
        {
        case VIR_COP_NOT_EQUAL:
            result = (i0 != i1); break;
        case VIR_COP_LESS_OR_EQUAL:
            result = (i0 <= i1); break;
        case VIR_COP_LESS:
            result = (i0 < i1); break;
        case VIR_COP_EQUAL:
            result = (i0 == i1); break;
        case VIR_COP_GREATER:
            result = (i0 > i1); break;
        case VIR_COP_GREATER_OR_EQUAL:
            result = (i0 >= i1); break;
        case VIR_COP_AND:
            result = (i0 & i1); break;
        case VIR_COP_OR:
            result = (i0 | i1); break;
        case VIR_COP_XOR:
            result = (i0 ^ i1); break;
        case VIR_COP_NOT_ZERO:
            result = (i0 != 0); break;
        case VIR_COP_NOT:
            result = (i0 == 0); break;
        case VIR_COP_GREATER_OR_EQUAL_ZERO:
            result = (i0 >= 0); break;
        case VIR_COP_GREATER_ZERO:
            result = (i0 > 0); break;
        case VIR_COP_LESS_OREQUAL_ZERO:
            result = (i0 <= 0); break;
        case VIR_COP_LESS_ZERO:
            result = (i0 < 0); break;
        case VIR_COP_SELMSB:
            result = Src0Val & 0x80000000; break;
        default:
            gcmASSERT(0);
        }
    }
    else
    {
        gcmASSERT(VIR_TypeId_isUnSignedInteger(Src0Type) || VIR_TypeId_isBoolean(Src0Type));
        gcmASSERT(VIR_ConditionOp_SingleOperand(COP) || VIR_TypeId_isUnSignedInteger(Src1Type) || VIR_TypeId_isBoolean(Src0Type));

        switch (COP)
        {
        case VIR_COP_NOT_EQUAL:
            result = (Src0Val != Src1Val); break;
        case VIR_COP_LESS_OR_EQUAL:
            result = (Src0Val <= Src1Val); break;
        case VIR_COP_LESS:
            result = (Src0Val < Src1Val); break;
        case VIR_COP_EQUAL:
            result = (Src0Val == Src1Val); break;
        case VIR_COP_GREATER:
            result = (Src0Val > Src1Val); break;
        case VIR_COP_GREATER_OR_EQUAL:
            result = (Src0Val >= Src1Val); break;
        case VIR_COP_AND:
            result = (Src0Val & Src1Val); break;
        case VIR_COP_OR:
            result = (Src0Val | Src1Val); break;
        case VIR_COP_XOR:
            result = (Src0Val ^ Src1Val); break;
        case VIR_COP_NOT_ZERO:
            result = (Src0Val != 0); break;
        case VIR_COP_NOT:
            result = (Src0Val == 0); break;
        case VIR_COP_GREATER_OR_EQUAL_ZERO:
            result = gcvTRUE; break;
        case VIR_COP_GREATER_ZERO:
            result = (Src0Val > 0); break;
        case VIR_COP_LESS_OREQUAL_ZERO:
            result = (Src0Val <= 0); break;
        case VIR_COP_LESS_ZERO:
            result = gcvFALSE; break;
        case VIR_COP_SELMSB:
        case VIR_COP_ANYMSB:
        case VIR_COP_ALLMSB:
            result = Src0Val & 0x80000000; break;
        default:
            gcmASSERT(0);
        }
    }

    return result;
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
        "SYM_SAMPLER_T",
        "SYM_TEXTURE",
        "SYM_IMAGE",
        "SYM_IMAGE_T",
        "SYM_CONST",
        "SYM_VIRREG", /* virtual register */
        "SYM_TYPE", /* typedef */
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

    VERIFY_EXPRESSION_BOOL_RET_BOPOT(VIR_Id_isValid(SymId));
    VERIFY_EXPRESSION_BOOL_RET_BOPOT(!VIR_Id_isFunctionScope(SymId) || BT_IS_FUNCTION_SCOPE(SymTable));

    VERIFY_EXPRESSION_BOOL_RET_BOPOT(VIR_SymTable_MaxValidId(SymTable) > unscopedSymId);
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
    _initOpenCLBuiltinNames(VIR_Shader * Shader, VIR_StringTable *StrTable)
{
    /* initialize builtin names */
    _add_name(VIR_NAME_UNKNOWN, "__unknown");

    VIR_NAME_POSITION       =
        VIR_NAME_POINT_SIZE     =
        VIR_NAME_CLIP_DISTANCE  =
        VIR_NAME_CULL_DISTANCE  =
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
        VIR_NAME_INSTANCE_INDEX     =
        VIR_NAME_DEVICE_INDEX       =
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
    _add_name(VIR_NAME_CLUSTER_ID, "#cluster_id");
    _add_name(VIR_NAME_SUBGROUP_NUM, "gl_NumSubgroups");
    _add_name(VIR_NAME_SUBGROUP_SIZE, "gl_SubgroupSize");
    _add_name(VIR_NAME_SUBGROUP_ID, "gl_SubgroupID");
    _add_name(VIR_NAME_SUBGROUP_INVOCATION_ID, "gl_SubgroupInvocationID");

    VIR_NAME_BUILTIN_LAST = VIR_NAME_SUBGROUP_INVOCATION_ID + sizeof("gl_SubgroupInvocationID");
}

static void
_initOpenGLBuiltinNames(VIR_Shader * Shader, VIR_StringTable *StrTable)
{
    /* initialize builtin names */
    _add_name(VIR_NAME_UNKNOWN, "__unknown");
    _add_name(VIR_NAME_POSITION, "gl_Position");
    _add_name(VIR_NAME_POINT_SIZE, "gl_PointSize");
    _add_name(VIR_NAME_CLIP_DISTANCE, "gl_ClipDistance");
    _add_name(VIR_NAME_CULL_DISTANCE, "gl_CullDistance");
    _add_name(VIR_NAME_COLOR, "gl_Color");
    _add_name(VIR_NAME_FRONT_FACING, "gl_FrontFacing");
    _add_name(VIR_NAME_POINT_COORD, "gl_PointCoord");
    _add_name(VIR_NAME_POSITION_W, "gl_Position.w");
    _add_name(VIR_NAME_DEPTH, "gl_FragDepth");
    _add_name(VIR_NAME_FOG_COORD, "gl_FogFragCoord");
    _add_name(VIR_NAME_VERTEX_ID, "gl_VertexID");
    _add_name(VIR_NAME_VERTEX_INDEX, "gl_VertexIndex");
    _add_name(VIR_NAME_FRONT_COLOR, "gl_FrontColor");
    _add_name(VIR_NAME_BACK_COLOR, "gl_BackColor");
    _add_name(VIR_NAME_FRONT_SECONDARY_COLOR, "gl_FrontSecondaryColor");
    _add_name(VIR_NAME_BACK_SECONDARY_COLOR, "gl_BackSecondaryColor");
    _add_name(VIR_NAME_TEX_COORD, "gl_TexCoord");
    _add_name(VIR_NAME_INSTANCE_ID, "gl_InstanceID");
    _add_name(VIR_NAME_INSTANCE_INDEX, "gl_InstanceIndex");
    _add_name(VIR_NAME_DEVICE_INDEX, "gl_DeviceIndex");
    _add_name(VIR_NAME_NUM_GROUPS, "gl_NumWorkGroups");
    _add_name(VIR_NAME_WORKGROUPSIZE, "gl_WorkGroupSize");
    _add_name(VIR_NAME_WORK_GROUP_ID, "gl_WorkGroupID");
    _add_name(VIR_NAME_WORK_GROUP_INDEX, "gl_WorkGroupIndex");
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
    _add_name(VIR_NAME_IN_CLIP_DISTANCE, "gl_in.gl_ClipDistance");
    _add_name(VIR_NAME_IN_CULL_DISTANCE, "gl_in.gl_CullDistance");
    _add_name(VIR_NAME_BOUNDING_BOX, "gl_BoundingBox");
    _add_name(VIR_NAME_LAST_FRAG_DATA, "gl_LastFragData");
    _add_name(VIR_NAME_CLUSTER_ID, "#cluster_id");
    _add_name(VIR_NAME_SUBGROUP_NUM, "gl_NumSubgroups");
    _add_name(VIR_NAME_SUBGROUP_SIZE, "gl_SubgroupSize");
    _add_name(VIR_NAME_SUBGROUP_ID, "gl_SubgroupID");
    _add_name(VIR_NAME_SUBGROUP_INVOCATION_ID, "gl_SubgroupInvocationID");
    _add_name(VIR_NAME_VIEW_INDEX, "gl_ViewIndex");

    /* WARNING!!! change builtin_last if add new name !!! */
    VIR_NAME_BUILTIN_LAST = VIR_NAME_VIEW_INDEX + sizeof("gl_SubgroupInvocationID");
}
#undef _add_name

static void
_initBuiltinNames(VIR_Shader * Shader)
{
    if (VIR_Shader_IsCL(Shader))
    {
        _initOpenCLBuiltinNames(Shader, &Shader->stringTable);
    }
    else
    {
        _initOpenGLBuiltinNames(Shader, &Shader->stringTable);
    }
}

static VSC_ErrCode
_VIR_InitStringTable(VIR_Shader * Shader)
{
    VSC_ErrCode errCode   = VSC_ERR_NONE;
    /* initialize block table */
    vscBT_Initialize(&Shader->stringTable,
        &Shader->pmp.mmWrapper,
        VSC_BLOCK_TABLE_FLAG_HASH_ENTRIES | VSC_BLOCK_TABLE_FLAG_AUTO_HASH,
        sizeof(char),
        32*1024, /* 32KB */
        10,
        gcvNULL,
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
        &Shader->pmp.mmWrapper,
        VSC_BLOCK_TABLE_FLAG_HASH_ENTRIES | VSC_BLOCK_TABLE_FLAG_AUTO_HASH,
        sizeof(VIR_Symbol),
        32*1024, /* 32KB */
        10,
        gcvNULL,
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
        &Shader->pmp.mmWrapper,
        VSC_BLOCK_TABLE_FLAG_HASH_ENTRIES | VSC_BLOCK_TABLE_FLAG_AUTO_HASH,
        sizeof(VIR_Const),
        16*1024, /* 16KB */
        10,
        gcvNULL,
        (PFN_VSC_HASH_FUNC)vscHFUNC_Const,
        (PFN_VSC_KEY_CMP)vcsHKCMP_Const,
        256);

    return errCode;
}

static VSC_ErrCode
_VIR_InitVirRegTable(VIR_Shader * Shader)
{
    VSC_ErrCode errCode   = VSC_ERR_NONE;
    /* initialize hash table */
    vscHTBL_Initialize(VIR_Shader_GetVirRegTable(Shader),
        &Shader->pmp.mmWrapper,
        (PFN_VSC_HASH_FUNC)vscHFUNC_VirReg,
        (PFN_VSC_KEY_CMP)vcsHKCMP_VirReg,
        512);

    return errCode;
}

static VSC_ErrCode
_VIR_InitInstTable(VIR_Shader * Shader)
{
    VSC_ErrCode errCode   = VSC_ERR_NONE;
    /* initialize block table */
    vscBT_Initialize(&Shader->instTable,
        &Shader->pmp.mmWrapper,
        VSC_BLOCK_TABLE_FLAG_FREE_ENTRY_LIST | VSC_BLOCK_TABLE_FLAG_FREE_ENTRY_USE_PTR,
        sizeof(VIR_Instruction),
        16*1024, /* 16KB */
        10,
        gcvNULL,
        gcvNULL,
        gcvNULL,
        0);

    return errCode;
}

static VSC_ErrCode
_VIR_Shader_InitTypeTable(VIR_Shader *    Shader)
{
    VSC_ErrCode errCode   = VSC_ERR_NONE;
    gctUINT      i;
    /* initialize block table */
    vscBT_Initialize(&Shader->typeTable,
        &Shader->pmp.mmWrapper,
        (VSC_BLOCK_TABLE_FLAG_HASH_ENTRIES
        | VSC_BLOCK_TABLE_FLAG_AUTO_HASH
        | VSC_BLOCK_TABLE_FLAG_FREE_ENTRY_LIST ),
        sizeof(VIR_Type),
        32*1024, /* 32KB */
        10,
        gcvNULL,
        (PFN_VSC_HASH_FUNC)vscHFUNC_Type,
        (PFN_VSC_KEY_CMP)vcsHKCMP_Type,
        512);

    /* initialize builtin types */
    for (i=0; i < sizeof(VIR_builtinTypes)/sizeof(VIR_builtinTypes[0]);  i++)
    {
        VIR_BuiltinTypeInfo * typeInfo = &VIR_builtinTypes[i];
        if (typeInfo->name != gcvNULL)
        {
            VIR_TypeId typeId;
            VIR_Type * type;
            errCode = VIR_Shader_AddBuiltinType(Shader, typeInfo, &typeId);
            CHECK_ERROR(errCode, "AddBuiltinType");

            /* initialize nameId and OCLNameId */
            type = VIR_Shader_GetTypeFromId(Shader, typeId);
            typeInfo->nameId = VIR_Type_GetNameId(type);
            if (typeInfo->OCLName == gcvNULL)
            {
                typeInfo->OCLNameId = VIR_Type_GetNameId(type);
            }
            else
            {
                typeInfo->OCLNameId = vscStringTable_Find(&Shader->stringTable,
                                                        typeInfo->name,
                                                        strlen(typeInfo->name)+1);
            }
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

    Shader->dumper = (VIR_Dumper*)vscMM_Alloc(&Shader->pmp.mmWrapper,
        sizeof(VIR_Dumper));
    if (Shader->dumper == gcvNULL)
    {
        return VSC_ERR_OUT_OF_MEMORY;
    }

    buffer = (gctCHAR *)vscMM_Alloc(&Shader->pmp.mmWrapper,
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
    Shader->dumper->dumpOperandId = VIR_DUMP_OPNDIDX;
    Shader->dumper->invalidCFG = gcvFALSE;
    return errCode;
}

VSC_ErrCode
VIR_Shader_Construct(
    IN gcoHAL         Hal,
    IN VIR_ShaderKind ShaderKind,
    OUT VIR_Shader *  Shader
    )
{
    return VIR_Shader_Construct0(Hal, ShaderKind, Shader, gcvTRUE);
}

VSC_ErrCode
VIR_Shader_Construct0(
    IN gcoHAL         Hal,
    IN VIR_ShaderKind ShaderKind,
    OUT VIR_Shader *  Shader,
    gctBOOL           InitTables
    )
{
    VSC_ErrCode errCode   = VSC_ERR_NONE;
    VIR_IdList * idList;

    gcmASSERT(Shader);

    /* get context from Hal */

    /* set default vaules */
    memset(Shader, 0, sizeof(VIR_Shader));

    vscPMP_Intialize(&Shader->pmp, gcvNULL, 8*1024, sizeof(void *), gcvTRUE /*pooling*/);

    Shader->object.type                 = gcvOBJ_VIR_SHADER;
    Shader->shaderKind                  = ShaderKind;
    VIR_Shader_SetDefaultUBOIndex(Shader, -1);
    Shader->constUniformBlockIndex      = -1;
    Shader->samplerBaseOffset           = -1;
    Shader->baseSamplerId               = VIR_INVALID_ID;
    Shader->kernelNameId                = VIR_INVALID_ID;
    Shader->optionsLen                  = 0;
    Shader->buildOptions                = gcvNULL;

    /* initialize the execution mode in tcs and tes to UNDEFINED */
    if (ShaderKind == VIR_SHADER_TESSELLATION_CONTROL)
    {
        Shader->shaderLayout.tcs.tessOrdering = VIR_TESS_ORDER_UNDEFINED;
        Shader->shaderLayout.tcs.tessPrimitiveMode = VIR_TESS_PMODE_UNDEFINED;
        Shader->shaderLayout.tcs.tessVertexSpacing = VIR_TESS_SPACING_UNDEFINED;
        Shader->shaderLayout.tcs.tessPointMode = gcvFALSE; /* default is disable */
    }
    else if (ShaderKind == VIR_SHADER_TESSELLATION_EVALUATION)
    {
        Shader->shaderLayout.tes.tessOrdering = VIR_TESS_ORDER_UNDEFINED;
        Shader->shaderLayout.tes.tessPrimitiveMode = VIR_TESS_PMODE_UNDEFINED;
        Shader->shaderLayout.tes.tessVertexSpacing = VIR_TESS_SPACING_UNDEFINED;
        Shader->shaderLayout.tes.tessPointMode = gcvFALSE; /* default is disable */
    }

    /* init dumper */
    errCode = _VIR_Shader_DumperInit(Shader, VSC_GET_DUMPER_FILE(), 2048);
    CHECK_ERROR(errCode, "Shader_DumperInit");

    /* always initialize instruction tables */
    errCode = _VIR_InitInstTable(Shader);
    CHECK_ERROR(errCode, "InitInstTable");

    /* initialize virReg table */
    errCode = _VIR_InitVirRegTable(Shader);
    CHECK_ERROR(errCode, "InitVirRegTable");

    /* initialize the default workGroupSize. */
    if (VIR_Shader_GetKind(Shader) == VIR_SHADER_COMPUTE)
    {
        /*
        ** 1) Use the DEVICE_MAX_WORK_GROUP_SIZE as the default workGroupSize for a shader.
        ** 2) When we need to use the workGroupSize to calculate the maxRegCount(e.g., use BARRIER in shader),
        **    use initWorkGroupSizeToCalcRegCount as the workGroupSize. And we may also reduce it to use more HW registers.
        */
        VIR_Shader_SetWorkGroupSizeAdjusted(Shader, gcvFALSE);
        VIR_Shader_SetAdjustedWorkGroupSize(Shader, GetHWMaxWorkGroupSize());

        /* Default, for CS, WorkGroupSize is fixed, for OCL, it is floating. */
        if (VIR_Shader_IsGlCompute(Shader))
        {
            VIR_Shader_SetWorkGroupSizeFixed(Shader, gcvTRUE);
        }
        else
        {
            VIR_Shader_SetWorkGroupSizeFixed(Shader, gcvFALSE);
        }
    }

    if (InitTables)
    {
        /* initialize tables */
        errCode = _VIR_InitStringTable(Shader);
        CHECK_ERROR(errCode, "InitStringTable");

        /* initialize symbol table */
        errCode = _VIR_InitSymbolTable(Shader);
        CHECK_ERROR(errCode, "_VIR_InitSymbolTable");

        /* initialize type table */
        errCode = _VIR_Shader_InitTypeTable(Shader);
        CHECK_ERROR(errCode, "InitTypeTable");

        /* initialize const table */
        errCode = _VIR_InitConstTable(Shader);
        CHECK_ERROR(errCode, "InitConstTable");

        /* init id lists */
        idList = &Shader->attributes;
        errCode = VIR_IdList_Init(&Shader->pmp.mmWrapper, 16, &idList);
        CHECK_ERROR(errCode, "InitIdList");

        idList = &Shader->outputs;
        errCode = VIR_IdList_Init(&Shader->pmp.mmWrapper, 4, &idList);
        CHECK_ERROR(errCode, "InitIdList");

        idList = &Shader->perpatchInput;
        errCode = VIR_IdList_Init(&Shader->pmp.mmWrapper, 8, &idList);
        CHECK_ERROR(errCode, "InitIdList");

        idList = &Shader->perpatchOutput;
        errCode = VIR_IdList_Init(&Shader->pmp.mmWrapper, 4, &idList);
        CHECK_ERROR(errCode, "InitIdList");

        idList = &Shader->outputVregs;
        errCode = VIR_IdList_Init(&Shader->pmp.mmWrapper, 4, &idList);
        CHECK_ERROR(errCode, "InitIdList");

        idList = &Shader->perpatchOutputVregs;
        errCode = VIR_IdList_Init(&Shader->pmp.mmWrapper, 4, &idList);
        CHECK_ERROR(errCode, "InitIdList");

        idList = &Shader->uniforms;
        errCode = VIR_IdList_Init(&Shader->pmp.mmWrapper, 32, &idList);
        CHECK_ERROR(errCode, "InitIdList");

        idList = &Shader->variables;
        errCode = VIR_IdList_Init(&Shader->pmp.mmWrapper, 64, &idList);
        CHECK_ERROR(errCode, "InitIdList");

        idList = &Shader->sharedVariables;
        errCode = VIR_IdList_Init(&Shader->pmp.mmWrapper, 64, &idList);
        CHECK_ERROR(errCode, "InitIdList");

        idList = &Shader->uniformBlocks;
        errCode = VIR_IdList_Init(&Shader->pmp.mmWrapper, 32, &idList);
        CHECK_ERROR(errCode, "InitIdList");

        idList = &Shader->storageBlocks;
        errCode = VIR_IdList_Init(&Shader->pmp.mmWrapper, 32, &idList);
        CHECK_ERROR(errCode, "InitIdList");

        idList = &Shader->ioBlocks;
        errCode = VIR_IdList_Init(&Shader->pmp.mmWrapper, 32, &idList);
        CHECK_ERROR(errCode, "InitIdList");

        idList = &Shader->moduleProcesses;
        errCode = VIR_IdList_Init(&Shader->pmp.mmWrapper, 32, &idList);
        CHECK_ERROR(errCode, "InitIdList");

        vscBILST_Initialize(&Shader->functions, gcvFALSE);

        vscBILST_Initialize(&Shader->kernelFunctions, gcvFALSE);

        /* init transform feedback */
    }

    return errCode;
}

VSC_ErrCode
VIR_Shader_Destroy(
    IN VIR_Shader * Shader
    )
{
    VSC_ErrCode errCode   = VSC_ERR_NONE;

    vscPMP_Finalize(&Shader->pmp);

    return errCode;
}

gctBOOL
VIR_Shader_IsNameBuiltIn(
    IN  VIR_Shader *    Shader,
    IN  VIR_NameId      NameId
    )
{
    if (NameId > VIR_NAME_UNKNOWN && NameId <= VIR_NAME_BUILTIN_LAST)
    {
        return gcvTRUE;
    }

    return gcvFALSE;
}

gctUINT
VIR_Shader_DecodeLangVersionToCompilerVersion(
    IN VIR_Shader *    Shader,
    IN gctBOOL         IsDeskTopGL,
    IN gctUINT         LanguageVersion
    )
{
    gctUINT            shMajorVer = 0;
    gctUINT            shMinorVer = 0;
    gctUINT            compilerVersion = 0;

    if (VIR_Shader_IsCL(Shader))
    {
        compilerVersion = _cldCL1Dot1;
    }
    else
    {
        shMajorVer = LanguageVersion / 100;
        shMinorVer = (LanguageVersion % 100) / 10;

        /* GL and ES have different version value. */
        if (IsDeskTopGL)
        {
            compilerVersion = gcmCC(0, _SHADER_GL_VERSION_SIG, shMinorVer, shMajorVer);
        }
        else
        {
            compilerVersion = gcmCC(0, 0, shMinorVer, shMajorVer);
        }
    }

    return compilerVersion;
}

void
VIR_Shader_DecodeCompilerVersionToShVersion(
    IN VIR_Shader *    Shader,
    IN gctUINT         CompilerVersion,
    OUT gctUINT *      MajorVersion,
    OUT gctUINT *      MinorVersion
    )
{
    gctUINT            shMajorVer = 0;
    gctUINT            shMinorVer = 0;

    shMajorVer = CompilerVersion >> 24;
    shMinorVer = CompilerVersion >> 16;

    if (VIR_Shader_IsCL(Shader) && shMinorVer == 0)
    {
        shMinorVer = 1;
    }

    if (MajorVersion)
    {
        *MajorVersion = shMajorVer;
    }

    if (MinorVersion)
    {
        *MinorVersion = shMinorVer;
    }
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
    if (VIR_Shader_GetClientApiVersion(Shader) == gcvAPI_OPENGL_ES11)
    {
        return gcvTRUE;
    }

    return gcvFALSE;
}

gctBOOL
VIR_Shader_IsES20Compiler(
    IN VIR_Shader * Shader
    )
{
    gctBOOL isES20Compiler;

    if (VIR_Shader_IsES11Compiler(Shader))
    {
        return gcvFALSE;
    }

    isES20Compiler = ((Shader->compilerVersion[0] & 0xFFFF) == _SHADER_GL_LANGUAGE_TYPE &&
        (Shader->compilerVersion[1] >= _SHADER_ES11_VERSION) &&
        (Shader->compilerVersion[1] < _SHADER_HALTI_VERSION));

    return isES20Compiler;
}

gctBOOL
VIR_Shader_IsES30Compiler(
    IN VIR_Shader * Shader
    )
{
    gctBOOL isES30Compiler;

    isES30Compiler = ((Shader->compilerVersion[0] & 0xFFFF) == _SHADER_GL_LANGUAGE_TYPE &&
        (Shader->compilerVersion[1] == _SHADER_HALTI_VERSION));

    return isES30Compiler;
}

gctBOOL
VIR_Shader_IsES31Compiler(
    IN VIR_Shader * Shader
    )
{
    gctBOOL isES31Compiler;

    isES31Compiler = ((Shader->compilerVersion[0] & 0xFFFF) == _SHADER_GL_LANGUAGE_TYPE &&
        (Shader->compilerVersion[1] == _SHADER_ES31_VERSION));

    return isES31Compiler;
}

gctBOOL
VIR_Shader_IsES32Compiler(
    IN VIR_Shader * Shader
    )
{
    gctBOOL isES32Compiler;

    isES32Compiler = ((Shader->compilerVersion[0] & 0xFFFF) == _SHADER_GL_LANGUAGE_TYPE &&
        (Shader->compilerVersion[1] == _SHADER_ES32_VERSION));

    return isES32Compiler;
}

gctBOOL
VIR_Shader_IsES40Compiler(
    IN VIR_Shader * Shader
    )
{
    gctBOOL isES40Compiler;

    isES40Compiler = ((Shader->compilerVersion[0] & 0xFFFF) == _SHADER_GL_LANGUAGE_TYPE &&
        (Shader->compilerVersion[1] == _SHADER_ES40_VERSION));

    return isES40Compiler;
}

gctBOOL
VIR_Shader_IsES31AndAboveCompiler(
    IN VIR_Shader * Shader
    )
{
    gctBOOL isES31Compiler;

    isES31Compiler = ((Shader->compilerVersion[0] & 0xFFFF) == _SHADER_GL_LANGUAGE_TYPE &&
        (Shader->compilerVersion[1] >= _SHADER_ES31_VERSION));

    return isES31Compiler;
}

gctBOOL
VIR_Shader_IsGL40(
    IN VIR_Shader * Shader
    )
{
    gctBOOL bMatch;

    bMatch = ((Shader->compilerVersion[0] & 0xFFFF) == _SHADER_OGL_LANGUAGE_TYPE &&
              (Shader->compilerVersion[1] == _SHADER_GL40_VERSION));

    return bMatch;
}

gctBOOL
VIR_Shader_IsGL40OrAbove(
    IN VIR_Shader * Shader
    )
{
    gctBOOL bMatch;
    bMatch = ((Shader->compilerVersion[0] & 0xFFFF) == _SHADER_OGL_LANGUAGE_TYPE &&
              (Shader->compilerVersion[1] >= _SHADER_GL40_VERSION));

    return bMatch;
}

gctBOOL
VIR_Shader_IsGL43(
    IN VIR_Shader * Shader
    )
{
    gctBOOL bMatch;

    bMatch = ((Shader->compilerVersion[0] & 0xFFFF) == _SHADER_OGL_LANGUAGE_TYPE &&
              (Shader->compilerVersion[1] == _SHADER_GL43_VERSION));

    return bMatch;
}

gctBOOL
VIR_Shader_IsGL44(
    IN VIR_Shader * Shader
    )
{
    gctBOOL bMatch;

    bMatch = ((Shader->compilerVersion[0] & 0xFFFF) == _SHADER_OGL_LANGUAGE_TYPE &&
              (Shader->compilerVersion[1] == _SHADER_GL44_VERSION));

    return bMatch;
}

gctBOOL
VIR_Shader_IsGL45(
    IN VIR_Shader * Shader
    )
{
    gctBOOL bMatch;

    bMatch = ((Shader->compilerVersion[0] & 0xFFFF) == _SHADER_OGL_LANGUAGE_TYPE &&
              (Shader->compilerVersion[1] == _SHADER_GL45_VERSION));

    return bMatch;
}

VSC_BT_FREE_ENTRY* VIR_Operand_GetFreeEntry(VIR_Operand * pOperand)
{
    /* set the operand kind to free entry kind */
    VIR_Operand_SetOpKind(pOperand, VIR_OPND_UNUSED);
    return (VSC_BT_FREE_ENTRY*)&pOperand->u.n._nextUse;
}

VSC_ErrCode
VIR_Shader_AddFunctionContent(
    IN  VIR_Shader *    Shader,
    IN  VIR_Symbol *    FuncSym,
    OUT VIR_Function ** Function,
    gctBOOL             Init
    )
{
    VSC_ErrCode     errCode   = VSC_ERR_NONE;
    VIR_IdList *    idList;
    VSC_MM *        memPool   = &Shader->pmp.mmWrapper;
    VIR_Function *  func;
    VIR_SymId       funcSymId = VIR_Symbol_GetIndex(FuncSym);
    VIR_FunctionNode *funcNode;

    /* allocate Shader object from memory pool */
    func = (VIR_Function *)vscMM_Alloc(memPool, sizeof(VIR_Function));
    if (func == gcvNULL)
    {
        return VSC_ERR_OUT_OF_MEMORY;
    }

    *Function = func;

    /* set default vaules */
    memset(func, 0, sizeof(VIR_Function));

    FuncSym->u2.function    = func;
    func->hostShader        = Shader;
    func->funcSym           = funcSymId;
    func->pFuncBlock        = gcvNULL;
    func->debugInfo         = Shader->debugInfo;
    func->die               = VSC_DI_INVALIDE_DIE;

    if (Init)
    {
        /* initialize symbol table */
        vscBT_Initialize(&func->symTable,
            memPool,
            (VSC_BLOCK_TABLE_FLAG_HASH_ENTRIES
            | VSC_BLOCK_TABLE_FLAG_AUTO_HASH
            | VSC_BLOCK_TABLE_FLAG_FUNCTION_SCOPE),
            sizeof(VIR_Symbol),
            VIR_FUNC_SYMTBL_SZ,
            10,
            gcvNULL,
            (PFN_VSC_HASH_FUNC)vscHFUNC_Symbol,
            (PFN_VSC_KEY_CMP)vcsHKCMP_Symbol,
            VIR_FUNC_SYMHSHTBL_SZ);

        /* initialize label table */
        vscBT_Initialize(&func->labelTable,
            memPool,
            (VSC_BLOCK_TABLE_FLAG_FREE_ENTRY_LIST
            | VSC_BLOCK_TABLE_FLAG_HASH_ENTRIES
            | VSC_BLOCK_TABLE_FLAG_AUTO_HASH),
            sizeof(VIR_Label),
            VIR_FUNC_LBLTBL_SZ,
            10,
            gcvNULL,
            (PFN_VSC_HASH_FUNC)vscHFUNC_Label,
            (PFN_VSC_KEY_CMP)vcsHKCMP_Label,
            VIR_FUNC_LBLHSHTBL_SZ);

        /* initialize operands table */
        vscBT_Initialize(&func->operandTable,
            memPool,
            VSC_BLOCK_TABLE_FLAG_FREE_ENTRY_LIST,
            sizeof(VIR_Operand),
            VIR_FUNC_OPNDTBL_SZ,
            10,
            (PFN_VSC_GET_FREE_ENTRY)VIR_Operand_GetFreeEntry,
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

        if (isSymKernelFunction(FuncSym))
        {
            funcNode = vscMM_Alloc(memPool, sizeof(VIR_FunctionNode));
            gcmASSERT(funcNode != gcvNULL);
            funcNode->function = func;
            vscBILST_Append(&Shader->kernelFunctions, (VSC_BI_LIST_NODE*)funcNode);

            func->flags |= VIR_FUNCFLAG_KERNEL;
        }

        if (isSymMainFunction(FuncSym))
        {
            /* is main function */
            Shader->mainFunction = func;
            func->flags |= VIR_FUNCFLAG_MAIN;
        }

        if (isSymInitFunction(FuncSym))
        {
            /* is init function */
            Shader->initFunction = func;
            func->flags |= VIR_FUNCFLAG_INITIALIZE_FUNC;
        }
    }
    return errCode;
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
    VIR_SymId       funcSymId;
    VIR_Symbol *    funcSym;

    /* creat function */
    /* do we need a separate mempool for function? */

    errCode = VIR_Shader_AddSymbolWithName(Shader,
        VIR_SYM_FUNCTION,
        Name,
        VIR_Shader_GetTypeFromId(Shader, TypeId),
        VIR_STORAGE_UNKNOWN,
        &funcSymId);

    if (errCode != VSC_ERR_NONE && errCode != VSC_ERR_REDEFINITION)
    {
        return errCode;
    }

    /* We can add a same function twice only if it has been removed before. */
    if (errCode == VSC_ERR_REDEFINITION)
    {
        funcSym = VIR_Shader_GetSymFromId(Shader, funcSymId);
        if (!isSymBeRemoved(funcSym))
        {
            return errCode;
        }

        /* Update the function type. */
        VIR_Symbol_SetType(funcSym, VIR_Shader_GetTypeFromId(Shader, TypeId));
        errCode = VSC_ERR_NONE;
    }
    funcSym = VIR_Shader_GetSymFromId(Shader, funcSymId);

    if (IsKernel)
    {
        VIR_Symbol_SetFlag(funcSym, VIR_SYMFLAG_ISKERNEL);
    }

    if (strcmp("main", Name) == 0)
    {
        VIR_Symbol_SetFlag(funcSym, VIR_SYMFLAG_ISMAIN);
    }

    ON_ERROR(VIR_Shader_AddFunctionContent(Shader, funcSym, Function, gcvTRUE), "VIR_Shader_AddFunctionContent");

OnError:
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
    VIR_Symbol          *pFunSym = VIR_Function_GetSymbol(Function);

    VIR_FuncIterator_Init(&iter, &Shader->functions);
    funcNode = VIR_FuncIterator_First(&iter);
    for (; funcNode != gcvNULL; funcNode = VIR_FuncIterator_Next(&iter))
    {
        if (funcNode->function == Function)
        {
            VIR_VariableIdList  *paramList = VIR_Function_GetParameters(Function);
            VIR_Symbol          *paramSym = gcvNULL;
            VIR_Symbol          *virRegSym = gcvNULL;
            VIR_VirRegId         virRegId = VIR_INVALID_ID;
            VIR_SymId            virRegSymId = VIR_INVALID_ID;
            gctUINT              i;

            /* Clean the storage class for the parameters. */
            for (i = 0; i < VIR_IdList_Count(paramList); i++)
            {
                /* Clean the storage class for a parameter variable itself. */
                paramSym = VIR_Function_GetSymFromId(Function, VIR_IdList_GetId(paramList, i));
                VIR_Symbol_SetStorageClass(paramSym, VIR_STORAGE_LOCAL);

                /* Clean the enclose function because it is not a parameter anymore. */
                VIR_Symbol_SetEncloseFuncSymId(paramSym, VIR_INVALID_ID);

                /* Set this symbol to a local variable. */
                VIR_Symbol_SetFlag(paramSym, VIR_SYMFLAG_LOCAL);
                VIR_Symbol_SetHostFunction(paramSym, Function);

                /* Clean the storage class for the vreg variable of a parameter variable if exist. */
                virRegId = VIR_Symbol_GetVregIndex(paramSym);
                if (virRegId != VIR_INVALID_ID)
                {
                    VIR_Shader_GetVirRegSymByVirRegId(Shader,
                                                      virRegId,
                                                      &virRegSymId);
                    if (virRegSymId != VIR_INVALID_ID)
                    {
                        virRegSym = VIR_Shader_GetSymFromId(Shader, virRegSymId);
                        gcmASSERT(virRegSym && VIR_Symbol_isVreg(virRegSym));

                        VIR_Symbol_SetStorageClass(virRegSym, VIR_STORAGE_UNKNOWN);

                        /* Clean the enclose function because it is not a parameter anymore. */
                        VIR_Symbol_SetEncloseFuncSymId(virRegSym, VIR_INVALID_ID);
                        /* reset the corresponding underlying variable of the virreg */
                        VIR_Symbol_SetVregVarSymId(virRegSym, VIR_INVALID_ID);
                    }
                }
            }

            /* Remove this function from function list. */
            vscBILST_Remove(&Shader->functions, (VSC_BI_LIST_NODE*)funcNode);
            break;
        }
    }

    VIR_Symbol_SetFlag(pFunSym, VIR_SYMFLAG_ISREMOVED);

    return errCode;
}

/* in SPIR-V, the function is mangled, the delimiter is '(' as of Jun. 1, 2016 */
#define VULKA_SPIRV_MANGLE_DELIMITER   '('

gctBOOL
isBaseNameMatched(
    gctCONST_STRING FuncName,
    gctSIZE_T       FuncNameLen,
    gctCONST_STRING BaseName,
    gctSIZE_T       BaseNameLen)
{
    if ((FuncNameLen > BaseNameLen) &&
        (FuncName[BaseNameLen] == VULKA_SPIRV_MANGLE_DELIMITER) &&
        gcoOS_StrNCmp(FuncName, BaseName, BaseNameLen) == gcvSTATUS_OK)
    {
        return gcvTRUE;
    }
    return gcvFALSE;

}

gceSTATUS
VIR_Shader_GetFunctionByName(
    IN  VIR_Shader *    Shader,
    IN  gctCONST_STRING FunctionName,
    OUT VIR_Function ** Function
    )
{
    gceSTATUS status = gcvSTATUS_NAME_NOT_FOUND;
    VIR_FuncIterator func_iter;
    VIR_FunctionNode* func_node;
    gctSIZE_T funcLength = gcoOS_StrLen(FunctionName, gcvNULL);

    gcmASSERT(Shader);
    VIR_FuncIterator_Init(&func_iter, VIR_Shader_GetFunctions(Shader));
    for (func_node = VIR_FuncIterator_First(&func_iter); func_node != gcvNULL; func_node = VIR_FuncIterator_Next(&func_iter))
    {
        VIR_Function* curFunc = func_node->function;
        gctSTRING curFuncName = VIR_Function_GetNameString(curFunc);
        gctSIZE_T curFuncLength;

        /* only compare the length of FunctionName */
        if (curFuncName &&
            gcoOS_StrNCmp(curFuncName, FunctionName, funcLength) == gcvSTATUS_OK &&
            ((curFuncLength = gcoOS_StrLen(curFuncName, gcvNULL)) == funcLength /* exact match */ ||
             isBaseNameMatched(curFuncName, curFuncLength, FunctionName, funcLength)))
        {
            *Function = curFunc;
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
    IN  gctCONST_STRING String,
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

gctBOOL
VIR_Shader_FindString(
    IN  VIR_Shader *    Shader,
    IN  gctCONST_STRING String
    )
{
    VIR_StringTable* pStringTbl = &Shader->stringTable;
    VIR_NameId sid;

    /* search hash table in Block table */
    gcmASSERT(BT_HAS_HASHTABLE(pStringTbl));
    sid = (VIR_NameId)vscBT_HashSearch(pStringTbl, (void *)String);

    if (VIR_Id_isInvalid(sid))
    {
        return gcvFALSE;
    }

    return gcvTRUE;
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
    type._flags         = (VIR_TyFlag)(TypeInfo->flag | VIR_TYFLAG_BUILTIN);
    type._kind          = (gctUINT)TypeInfo->kind;
    type._addrSpace     = (gctUINT)VIR_AS_PRIVATE;
    type._qualifier     = (gctUINT)VIR_TYQUAL_NONE;
    type.arrayStride    = 0;
    type.matrixStride   = 0;
    type.u1.nameId      = vscStringTable_Find(&Shader->stringTable,
        TypeInfo->name,
        strlen(TypeInfo->name)+1);
    type.u1.symId       = VIR_INVALID_ID;
    type.u2.size        = TypeInfo->sz;
    tyId                = vscBT_AddEntry(&Shader->typeTable, &type);
    *TypeId             = tyId;
    newType = VIR_Shader_GetTypeFromId(Shader, tyId);
    newType->_tyIndex   = tyId;
    VIR_Type_SetAlignment(newType, TypeInfo->alignment);
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
    IN  gctINT          ArrayStride,
    OUT VIR_TypeId *    TypeId
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    VIR_TypeId  tyId;
    VIR_Type    type;
    VIR_Type *  baseType = VIR_Shader_GetTypeFromId(Shader, BaseTypeId);
    VIR_Type *aggregateType;

    type._base          = BaseTypeId;
    type._flags         = VIR_TYFLAG_SIZED;
    type._kind          = (gctUINT)VIR_TY_ARRAY;
    type._alignment     = baseType->_alignment;
    type._addrSpace     = (gctUINT)VIR_AS_PRIVATE;
    type._qualifier     = (gctUINT)VIR_TYQUAL_NONE;
    type.arrayStride    = ArrayStride;
    type.matrixStride   = 0;
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
    IN  VSC_TyQualifier Qualifier,
    IN  VSC_AddrSpace   AS,
    OUT VIR_TypeId *    TypeId
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    VIR_TypeId  tyId;
    VIR_Type    type;
    VIR_Type *  ptrType;
    VIR_Type *  baseType = VIR_Shader_GetTypeFromId(Shader, BaseTypeId);

    type._base           = BaseTypeId;
    type._flags          = VIR_TYFLAG_SIZED;
    type._kind           = (gctUINT)VIR_TY_POINTER;
    type._alignment      = baseType->_alignment;
    type._addrSpace      = (gctUINT)AS;
    type._qualifier      = (gctUINT)Qualifier;
    type.arrayStride    = 0;
    type.matrixStride   = 0;
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
VIR_Shader_AddTypeDefType(
    IN  VIR_Shader *    Shader,
    IN  VIR_TypeId      BaseTypeId,
    IN  VIR_NameId      NameId,
    IN  VIR_TyQualifier Qualifier,
    IN  VIR_AddrSpace   AS,
    OUT VIR_TypeId*     TypeId
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    VIR_TypeId  tyId;
    VIR_Type    type;
    VIR_Type *  ptrType;
    VIR_Type *  baseType = VIR_Shader_GetTypeFromId(Shader, BaseTypeId);

    type._base           = BaseTypeId;
    type._flags          = VIR_TYFLAG_SIZED;
    type._kind           = (gctUINT)VIR_TY_TYPEDEF;
    type._alignment      = baseType->_alignment;
    type._addrSpace      = (gctUINT)AS;
    type._qualifier      = (gctUINT)Qualifier;
    type.arrayStride    = 0;
    type.matrixStride   = 0;
    type.u1.nameId      = NameId;
    type.u1.symId       = VIR_INVALID_ID;
    /* The size may be unknown if the base type is a undefined struct, please use VIR_Type_GetTypeByteSize to get the size. */
    type.u2.size        = 0;
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
    type._flags         = VIR_TYFLAG_NONE;
    type._kind          = (gctUINT)VIR_TY_FUNCTION;
    type._alignment     = 0; /* no alignment for function type */
    type._addrSpace     = (gctUINT)VIR_AS_PRIVATE;
    type._qualifier     = (gctUINT)VIR_TYQUAL_NONE;
    type.arrayStride    = 0;
    type.matrixStride   = 0;
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
    IN  gctBOOL         ForceDup,
    OUT VIR_TypeId *    TypeId
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    VIR_TypeId  tyId;
    VIR_TypeId  origTyId = VIR_INVALID_ID;
    VIR_Type    type;
    VIR_Type *  structType;

    type._base          = VIR_TYPE_UNKNOWN;
    type._flags         = IsUnion ? (gctUINT)VIR_TYFLAG_ISUNION : (gctUINT)VIR_TYFLAG_NONE;
    type._kind          = (gctUINT)VIR_TY_STRUCT;
    type._alignment     = 0; /* no alignment for struct type */
    type._addrSpace     = (gctUINT)VIR_AS_PRIVATE;
    type._qualifier     = (gctUINT)VIR_TYQUAL_NONE;
    type.arrayStride    = 0;
    type.matrixStride   = 0;
    type.u1.nameId      = NameId;
    type.u1.symId       = VIR_INVALID_ID; /* need to set it with struct symbol id */
    type.u2.fields      = gcvNULL;

    if (ForceDup)
    {
        /* Try to get existing id */
        origTyId = vscBT_HashSearch(&Shader->typeTable, &type);

        /* For a new id with same type */
        tyId = vscBT_AddEntry(&Shader->typeTable, &type);

        if (origTyId != VIR_INVALID_ID)
        {
            VIR_Shader_DuplicateType(Shader, origTyId, &tyId);
        }
    }
    else
    {
        tyId = vscBT_Find(&Shader->typeTable, &type);
    }

    *TypeId             = tyId;
    structType = VIR_Shader_GetTypeFromId(Shader, tyId);
    structType->_tyIndex = tyId;

    return errCode;
}

VSC_ErrCode
VIR_Shader_AddEnumType(
    IN  VIR_Shader *    Shader,
    IN  VIR_NameId      NameId,
    OUT VIR_TypeId *    TypeId
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    VIR_TypeId  tyId;
    VIR_Type    type;
    VIR_Type *  structType;

    type._base          = VIR_TYPE_UNKNOWN;
    type._flags         = (gctUINT)VIR_TYFLAG_NONE;
    type._kind          = (gctUINT)VIR_TY_ENUM;
    type._alignment     = 0; /* no alignment for struct type */
    type._addrSpace     = (gctUINT)VIR_AS_PRIVATE;
    type._qualifier     = (gctUINT)VIR_TYQUAL_NONE;
    type.arrayStride    = 0;
    type.matrixStride   = 0;
    type.u1.nameId      = NameId;
    type.u1.symId       = VIR_INVALID_ID; /* need to set it with struct symbol id */
    type.u2.fields      = gcvNULL;

    tyId = vscBT_Find(&Shader->typeTable, &type);

    *TypeId             = tyId;
    structType = VIR_Shader_GetTypeFromId(Shader, tyId);
    structType->_tyIndex = tyId;

    return errCode;
}

VSC_ErrCode
VIR_Shader_DuplicateType(
    IN  VIR_Shader *    Shader,
    IN  VIR_TypeId      OrigTypeId,
    OUT VIR_TypeId*     DupTypeId
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;

    VIR_Type* origType = VIR_Shader_GetTypeFromId(Shader, OrigTypeId);
    VIR_NameId origTypeNameId = VIR_Type_GetNameId(origType);
    gctSTRING origTypeName = VIR_Shader_GetStringFromId(Shader, origTypeNameId);
    VIR_Type* dupType;
    VIR_NameId dupTypeNameId;
    gctCHAR dupTypeName[256];
    gctUINT offset = 0;

    VIR_Type_IncDuplicationId(origType);
    gcoOS_PrintStrSafe(dupTypeName, 256, &offset, "%s_#dup%d", origTypeName, VIR_Type_GetDuplicationId(origType));

    if(*DupTypeId == VIR_INVALID_ID)
    {
        if(VIR_Type_isArray(origType))
        {
            /*TBD*/
        }
        else
        {
            /*TBD*/
        }
    }
    else
    {
        dupType = VIR_Shader_GetTypeFromId(Shader, *DupTypeId);
        VIR_Shader_AddString(Shader, dupTypeName, &dupTypeNameId);
        VIR_Type_SetNameId(dupType, dupTypeNameId);
    }

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
    gcmASSERT(VIR_TypeId_isPrimitive(Type));

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
    gctUINT         chnlSwizzle[4] = { 0, 0, 0, 0};

    gcmASSERT(Constant->type < VIR_TYPE_LAST_PRIMITIVETYPE);
    count = VIR_GetTypeComponents(Constant->type) * VIR_GetTypeRows(Constant->type);

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
            VIR_Type *type = VIR_Symbol_GetType(sym);
            VIR_Type *baseType = type;

            while (VIR_Type_isArray(baseType))
            {
                baseType = VIR_Shader_GetTypeFromId(Shader, VIR_Type_GetBaseTypeId(baseType));
            }

            if (VIR_GetTypeSize(VIR_GetTypeComponentType(VIR_Type_GetIndex(baseType))) >= 8)
            {
                continue;
            }

            if (!VIR_Type_isArray(type))
            {
                VIR_Const *constVal = (VIR_Const *) VIR_GetSymFromId(&Shader->constTable, VIR_Uniform_GetInitializer(uniform));
                gctINT constCount = VIR_GetTypeComponents(constVal->type) * VIR_GetTypeRows(constVal->type);

                gcmASSERT(constCount > 0);

                if (constCount < count || VIR_GetTypeRows(constVal->type) > 1)
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
        Shader->shaderKind, Shader->_constVectorId);
    Shader->_constVectorId++;

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
    VIR_Uniform_SetInitializer(uniform, cstId);
    VIR_Symbol_SetLocation(uniformSym, -1);
    VIR_Symbol_SetFlag(uniformSym, VIR_SYMUNIFORMFLAG_COMPILETIME_INITIALIZED);
    VIR_Symbol_SetFlag(uniformSym, VIR_SYMFLAG_COMPILER_GEN);

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

VSC_ErrCode
VIR_Shader_AddNamedUniform(
    IN  VIR_Shader *      Shader,
    IN  gctCONST_STRING   Name,
    IN  VIR_Type *        Type,
    OUT VIR_Symbol **     UniformSym
    )
{
    VSC_ErrCode  errCode = VSC_ERR_NONE;
    VIR_SymId    symId;
    VIR_Symbol * uniformSym;

    errCode = VIR_Shader_AddSymbolWithName(Shader,
        VIR_SYM_UNIFORM,
        Name,
        Type,
        VIR_STORAGE_UNKNOWN,
        &symId);
    if (errCode != VSC_ERR_NONE)
    {
        return errCode;
    }
    uniformSym = VIR_Shader_GetSymFromId(Shader, symId);
    gcmASSERT(VIR_Symbol_isUniform(uniformSym));

    VIR_Symbol_SetLocation(uniformSym, -1);
    VIR_Symbol_SetFlag(uniformSym, VIR_SYMFLAG_COMPILER_GEN);

    *UniformSym = uniformSym;

    return errCode;
}

VSC_ErrCode
VIR_Shader_ChangeAddressUniformTypeToFatPointer(
    VIR_Shader *   pShader,
    VIR_Symbol *   pSym)
{
    VSC_ErrCode         errCode = VSC_ERR_NONE;
    VIR_Type *          symType;
    VIR_TypeId          newSymTypeId;
    VIR_Type *          newSymType;

    /* set base address uniform's sym to use vector 4 for fat pointer,
     * so even the pointer is not used in shader, we still allocate correct
     * size for API specified bindings */

    if (VIR_Symbol_GetKind(pSym) == VIR_SYM_UNIFORM &&
        (VIR_Symbol_GetUniformKind(pSym) == VIR_UNIFORM_UNIFORM_BLOCK_ADDRESS ||
            VIR_Symbol_GetUniformKind(pSym) == VIR_UNIFORM_STORAGE_BLOCK_ADDRESS ||
            VIR_Symbol_GetUniformKind(pSym) == VIR_UNIFORM_TEMP_REG_SPILL_MEM_ADDRESS))
    {
        symType = VIR_Symbol_GetType(pSym);
        gcmASSERT((VIR_GetTypeFlag(VIR_Type_GetBaseTypeId(symType)) & VIR_TYFLAG_ISINTEGER));

        if (VIR_Type_isArray(symType))
        {
            errCode = VIR_Shader_AddArrayType(pShader, VIR_TYPE_UINT_X4,
                                                VIR_Type_GetArrayLength(symType), 0, &newSymTypeId);
            ON_ERROR(errCode, "Add array type");
            newSymType = VIR_Shader_GetTypeFromId(pShader, newSymTypeId);
            VIR_Symbol_SetType(pSym, newSymType);
        }
        else
        {
            VIR_Symbol_SetType(pSym, VIR_Shader_GetTypeFromId(pShader, VIR_TYPE_UINT_X4));
        }
    }

OnError:
    return errCode;
}

gctUINT
VIR_Shader_GetLogicalCount(
    IN  VIR_Shader *    Shader,
    IN  VIR_Type *      Type
    )
{
    gctUINT count = 0;

    if (VIR_Type_isPrimitive(Type))
    {
        count = 1;
    }
    else
    {
        switch (VIR_Type_GetKind(Type))
        {
        case VIR_TY_ARRAY:
            {
                VIR_TypeId base_type_id = VIR_Type_GetBaseTypeId(Type);
                VIR_Type * base_type = VIR_Shader_GetTypeFromId(Shader, base_type_id);
                count = VIR_Type_GetArrayLength(Type) * VIR_Shader_GetLogicalCount(Shader, base_type);
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
                        count += VIR_Shader_GetLogicalCount(Shader, field_type);
                    }
                }
                else
                {
                    /* this struct is only symbolic and unused. in the future when VIR is
                    fully functional, this condition will be illegal */
                    count = 0;
                }
                break;
            }
        default:
            gcmASSERT(0);
        }
    }

    return count;
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
        /* Caller To Do: VIR_Symbol_SetLocation */
    }

    return sym;
}

VSC_ErrCode
_ConstructComplexVariable(
    IN OUT  VIR_Shader     *Shader,
    IN  VIR_Function       *Function,
    IN  VIR_Instruction    *Inst,
    IN  VIR_Type           *DestType,
    IN  VIR_SymId           DestSymId,
    IN  VIR_SymId           SourceSymId,
    IN  gctUINT             DestOffset
    )
{
    VSC_ErrCode             errCode = VSC_ERR_NONE;
    VIR_Instruction        *movInst;
    VIR_Operand            *newOperand;
    VIR_TypeId              destTypeId = VIR_Type_GetIndex(DestType);

    /* MOV instruction */
    if (Inst != gcvNULL)
    {
        errCode = VIR_Function_AddInstructionBefore(Function,
            VIR_OP_MOV,
            destTypeId,
            Inst,
            gcvTRUE,
            &movInst);
        CHECK_ERROR(errCode, "VIR_Function_AddInstructionBefore failed.");
    }
    else
    {
        errCode = VIR_Function_AddInstruction(Function,
            VIR_OP_MOV,
            destTypeId,
            &movInst);
        if (errCode != VSC_ERR_NONE) return errCode;
    }

    /* Set DEST. */
    newOperand = VIR_Inst_GetDest(movInst);
    VIR_Operand_SetSymbol(newOperand, Function, DestSymId);
    VIR_Operand_SetTypeId(newOperand, destTypeId);
    if (DestOffset != 0)
    {
        VIR_Operand_SetIsConstIndexing(newOperand, gcvTRUE);
        VIR_Operand_SetRelIndexingImmed(newOperand, DestOffset);
    }
    VIR_Inst_SetDest(movInst, newOperand);

    /* Set Source. */
    newOperand = VIR_Inst_GetSource(movInst, 0);
    VIR_Operand_SetSymbol(newOperand, Function, SourceSymId);
    VIR_Operand_SetTypeId(newOperand, destTypeId);
    VIR_Inst_SetSource(movInst, 0, newOperand);

    return errCode;
}

VSC_ErrCode
VIR_Shader_GenNullForScalarAndVector(
    IN OUT  VIR_Shader     *Shader,
    IN  VIR_Function       *Function,
    IN  VIR_Instruction    *Inst,
    IN  VIR_SymId           SymId,
    IN  VIR_TypeId          TypeId,
    IN  gctUINT             MatrixIndex,
    IN  gctUINT             RegOffset
    )
{
    VSC_ErrCode             errCode   = VSC_ERR_NONE;
    VIR_Instruction        *movInst;
    VIR_Operand            *newOperand;
    VIR_Enable              enable;

    gcmASSERT(TypeId != VIR_TYPE_UNKNOWN);

    enable = VIR_TypeId_Conv2Enable(TypeId);

    /* MOV instruction */
    if (Inst != gcvNULL)
    {
        errCode = VIR_Function_AddInstructionBefore(Function,
            VIR_OP_MOV,
            TypeId,
            Inst,
            gcvTRUE,
            &movInst);
        CHECK_ERROR(errCode, "VIR_Function_AddInstructionBefore failed.");
    }
    else
    {
        errCode = VIR_Function_AddInstruction(Function,
            VIR_OP_MOV,
            TypeId,
            &movInst);
        if (errCode != VSC_ERR_NONE) return errCode;
    }

    /* Set DEST. */
    newOperand = VIR_Inst_GetDest(movInst);
    VIR_Operand_SetSymbol(newOperand, Function, SymId);
    VIR_Operand_SetTypeId(newOperand, TypeId);
    VIR_Operand_SetEnable(newOperand, enable);
    /* Set the offset. */
    if (RegOffset != 0)
    {
        VIR_Operand_SetIsConstIndexing(newOperand, gcvTRUE);
        VIR_Operand_SetRelIndexingImmed(newOperand, RegOffset);
    }
    VIR_Operand_SetMatrixConstIndex(newOperand, MatrixIndex);
    VIR_Inst_SetDest(movInst, newOperand);

    /* Set SOURCE0. */
    newOperand = VIR_Inst_GetSource(movInst, 0);
    if (VIR_GetTypeFlag(TypeId) & VIR_TYFLAG_ISFLOAT)
    {
        VIR_Operand_SetImmediateFloat(newOperand, 0.0f);
    }
    else if (VIR_GetTypeFlag(TypeId) & VIR_TYFLAG_IS_SIGNED_INT)
    {
        VIR_Operand_SetImmediateInt(newOperand, 0);
    }
    else if (VIR_GetTypeFlag(TypeId) & VIR_TYFLAG_IS_UNSIGNED_INT)
    {
        VIR_Operand_SetImmediateUint(newOperand, 0);
    }
    else if (VIR_GetTypeFlag(TypeId) & VIR_TYFLAG_IS_BOOLEAN)
    {
        VIR_Operand_SetImmediateBoolean(newOperand, 0);
    }

    VIR_Inst_SetSource(movInst, 0, newOperand);

    return errCode;
}

VSC_ErrCode
VIR_Shader_GenNullForMatrix(
    IN OUT  VIR_Shader     *Shader,
    IN  VIR_Function       *Function,
    IN  VIR_Instruction    *Inst,
    IN  VIR_SymId           SymId,
    IN  VIR_TypeId          TypeId,
    IN  gctUINT             RegOffset
    )
{
    VSC_ErrCode             errCode   = VSC_ERR_NONE;
    VIR_TypeId              vectorTypeId = VIR_GetTypeRowType(TypeId);
    gctINT                  rowCount = VIR_GetTypeRows(TypeId);
    gctINT                  i;

    for (i = 0; i < rowCount; i++)
    {
        errCode = VIR_Shader_GenNullForScalarAndVector(Shader,
            Function,
            Inst,
            SymId,
            vectorTypeId,
            i,
            RegOffset);
        CHECK_ERROR(errCode, "VIR_Shader_GenNullForScalarAndVector failed.");
    }
    return errCode;
}

VSC_ErrCode
VIR_Shader_GenNullForArray(
    IN OUT  VIR_Shader     *Shader,
    IN  VIR_Function       *Function,
    IN  VIR_Instruction    *Inst,
    IN  VIR_SymId           SymId,
    IN  VIR_TypeId          TypeId,
    IN  gctUINT             RegOffset
    )
{
    VSC_ErrCode             errCode   = VSC_ERR_NONE;
    VIR_Type               *type = VIR_Shader_GetTypeFromId(Shader, TypeId);
    VIR_Type               *baseType = VIR_Shader_GetTypeFromId(Shader, VIR_Type_GetBaseTypeId(type));
    VIR_TypeKind            baseTypeKind = VIR_Type_GetKind(baseType);
    VIR_TypeId              baseTypeId = VIR_Type_GetIndex(baseType);
    gctUINT                 arrayLength = VIR_Type_GetArrayLength(type);
    gctUINT                 i;
    gctUINT                 regOffset = RegOffset;
    gctUINT                 arrayRegCount = (gctUINT)VIR_Type_GetRegCount(Shader, baseType, gcvFALSE);

    for (i = 0; i < arrayLength; i++)
    {
        if (baseTypeKind == VIR_TY_SCALAR || baseTypeKind == VIR_TY_VECTOR)
        {
            errCode = VIR_Shader_GenNullForScalarAndVector(Shader,
                Function,
                Inst,
                SymId,
                baseTypeId,
                0,
                regOffset);
            CHECK_ERROR(errCode, "VIR_Shader_GenNullForScalarAndVector failed.");
        }
        else if (baseTypeKind == VIR_TY_ARRAY)
        {
            errCode = VIR_Shader_GenNullForArray(Shader,
                Function,
                Inst,
                SymId,
                baseTypeId,
                regOffset);
            CHECK_ERROR(errCode, "VIR_Shader_GenNullForArray failed.");
        }
        else if (baseTypeKind == VIR_TY_MATRIX)
        {
            errCode = VIR_Shader_GenNullForMatrix(Shader,
                Function,
                Inst,
                SymId,
                baseTypeId,
                regOffset);
            CHECK_ERROR(errCode, "VIR_Shader_GenNullForMatrix failed.");
        }
        else if (baseTypeKind == VIR_TY_STRUCT)
        {
            errCode = VIR_Shader_GenNullForStruct(Shader,
                Function,
                Inst,
                SymId,
                baseTypeId,
                regOffset);
            CHECK_ERROR(errCode, "VIR_Shader_GenNullForStruct failed.");
        }

        /* Update the regOffset. */
        regOffset += arrayRegCount;
    }

    return errCode;
}

VSC_ErrCode
VIR_Shader_GenNullForStruct(
    IN OUT  VIR_Shader     *Shader,
    IN  VIR_Function       *Function,
    IN  VIR_Instruction    *Inst,
    IN  VIR_SymId           SymId,
    IN  VIR_TypeId          TypeId,
    IN  gctUINT             RegOffset
    )
{
    VSC_ErrCode             errCode   = VSC_ERR_NONE;
    VIR_Type               *type = VIR_Shader_GetTypeFromId(Shader, TypeId);
    VIR_SymIdList          *fields = VIR_Type_GetFields(type);
    gctUINT                 fieldLength = VIR_IdList_Count(fields);
    gctUINT                 i;
    gctUINT                 regOffset = RegOffset;

    for (i = 0; i < fieldLength; i++)
    {
        VIR_Id              id = VIR_IdList_GetId(VIR_Type_GetFields(type), i);
        VIR_Symbol         *fieldSym = VIR_Shader_GetSymFromId(Shader, id);
        VIR_Type           *fieldType = VIR_Symbol_GetType(fieldSym);
        VIR_TypeId          fieldTypeId = VIR_Type_GetIndex(fieldType);
        VIR_TypeKind        fieldTypeKind = VIR_Type_GetKind(fieldType);
        gctUINT             fieldOffset = VIR_Type_GetRegCount(Shader, fieldType, gcvFALSE);

        if (fieldTypeKind == VIR_TY_SCALAR || fieldTypeKind == VIR_TY_VECTOR)
        {
            errCode = VIR_Shader_GenNullForScalarAndVector(Shader,
                Function,
                Inst,
                SymId,
                fieldTypeId,
                0,
                regOffset);
            CHECK_ERROR(errCode, "VIR_Shader_GenNullForScalarAndVector failed.");
        }
        else if (fieldTypeKind == VIR_TY_ARRAY)
        {
            errCode = VIR_Shader_GenNullForArray(Shader,
                Function,
                Inst,
                SymId,
                fieldTypeId,
                regOffset);
            CHECK_ERROR(errCode, "VIR_Shader_GenNullForArray failed.");
        }
        else if (fieldTypeKind == VIR_TY_MATRIX)
        {
            errCode = VIR_Shader_GenNullForMatrix(Shader,
                Function,
                Inst,
                SymId,
                fieldTypeId,
                regOffset);
            CHECK_ERROR(errCode, "VIR_Shader_GenNullForMatrix failed.");
        }
        else if (fieldTypeKind == VIR_TY_STRUCT)
        {
            errCode = VIR_Shader_GenNullForStruct(Shader,
                Function,
                Inst,
                SymId,
                fieldTypeId,
                regOffset);
            CHECK_ERROR(errCode, "VIR_Shader_GenNullForStruct failed.");
        }

        /* Update the regOffset. */
        regOffset += fieldOffset;
    }

    return errCode;
}


VSC_ErrCode
VIR_Shader_GenNullAssignment(
    IN OUT  VIR_Shader     *Shader,
    IN  VIR_Function       *Function,
    IN  VIR_Instruction    *Inst,
    IN  VIR_SymId           SymId,
    IN  VIR_TypeId          TypeId
    )
{
    VSC_ErrCode             errCode   = VSC_ERR_NONE;
    VIR_Type               *type = VIR_Shader_GetTypeFromId(Shader, TypeId);
    VIR_TypeKind            typeKind = VIR_Type_GetKind(type);

    switch (typeKind)
    {
    case VIR_TY_SCALAR:
    case VIR_TY_VECTOR:
        errCode = VIR_Shader_GenNullForScalarAndVector(Shader,
            Function,
            Inst,
            SymId,
            TypeId,
            0,
            0);
        CHECK_ERROR(errCode, "VIR_Shader_GenNullForScalarAndVector failed.");
        break;

    case VIR_TY_MATRIX:
        errCode = VIR_Shader_GenNullForMatrix(Shader,
            Function,
            Inst,
            SymId,
            TypeId,
            0);
        CHECK_ERROR(errCode, "VIR_Shader_GenNullForMatrix failed.");
        break;

    case VIR_TY_ARRAY:
        errCode = VIR_Shader_GenNullForArray(Shader,
            Function,
            Inst,
            SymId,
            TypeId,
            0);
        CHECK_ERROR(errCode, "VIR_Shader_GenNullForArray failed.");
        break;

    case VIR_TY_STRUCT:
        errCode = VIR_Shader_GenNullForStruct(Shader,
            Function,
            Inst,
            SymId,
            TypeId,
            0);
        CHECK_ERROR(errCode, "VIR_Shader_GenNullForStruct failed.");
        break;

    default:
        /* Don't support image/sampler/pointer so far. */
        /* gcmASSERT(gcvFALSE); */
        break;
    }

    return errCode;
}

VSC_ErrCode
VIR_Shader_GenSimpleAssignment(
    IN OUT  VIR_Shader     *Shader,
    IN  VIR_Function       *Function,
    IN  VIR_Instruction    *Inst,
    IN  VIR_SymId           DestSymId,
    IN  VIR_TypeId          DestTypeId,
    IN  VIR_SymbolKind      DestOffsetKind,
    IN  VIR_SymId           DestOffset,
    IN  VIR_SymbolKind      SourceSymKind,
    IN  VIR_SymId           SourceSymId,
    IN  VIR_SymbolKind      SourceOffsetKind,
    IN  VIR_SymId           SourceOffset,
    IN  gctUINT             DestVectorOffset,
    IN  gctUINT             DestMatrixIndex,
    IN  gctUINT             SourceMatrixIndex
    )
{
    VSC_ErrCode             errCode   = VSC_ERR_NONE;
    VIR_Instruction        *movInst;
    VIR_Operand            *newOperand;
    VIR_Enable              origEnable, enable;
    VIR_Swizzle             swizzle;
    VIR_Const              *constant;

    gcmASSERT(DestTypeId != VIR_TYPE_UNKNOWN);

    origEnable = VIR_TypeId_Conv2Enable(DestTypeId);

    /* MOV instruction */
    if (Inst != gcvNULL)
    {
        errCode = VIR_Function_AddInstructionBefore(Function,
            VIR_OP_MOV,
            DestTypeId,
            Inst,
            gcvTRUE,
            &movInst);
        CHECK_ERROR(errCode, "VIR_Function_AddInstructionBefore failed.");
    }
    else
    {
        errCode = VIR_Function_AddInstruction(Function,
            VIR_OP_MOV,
            DestTypeId,
            &movInst);
        if (errCode != VSC_ERR_NONE) return errCode;
    }

    /* Set DEST. */
    newOperand = VIR_Inst_GetDest(movInst);
    VIR_Operand_SetSymbol(newOperand, Function, DestSymId);
    VIR_Operand_SetTypeId(newOperand, DestTypeId);
    enable = origEnable << DestVectorOffset;
    VIR_Operand_SetEnable(newOperand, enable);
    /* Set the offset. */
    if (DestOffsetKind == VIR_SYM_CONST)
    {
        if (DestOffset != 0)
        {
            VIR_Operand_SetIsConstIndexing(newOperand, gcvTRUE);
            VIR_Operand_SetRelIndexingImmed(newOperand, DestOffset);
        }
    }
    else
    {
        VIR_Operand_SetRelIndexing(newOperand, DestOffset);
        VIR_Operand_SetRelAddrMode(newOperand, VIR_INDEXED_X);
    }
    VIR_Operand_SetMatrixConstIndex(newOperand, DestMatrixIndex);
    VIR_Inst_SetDest(movInst, newOperand);

    /* Set SOURCE0. */
    newOperand = VIR_Inst_GetSource(movInst, VIR_Operand_Src0);
    if (SourceSymKind == VIR_SYM_CONST)
    {
        constant = VIR_Shader_GetConstFromId(Shader, SourceSymId);
        VIR_Operand_SetConst(newOperand, constant->type, SourceSymId);
        swizzle = VIR_Swizzle_GenSwizzleByComponentCount(VIR_GetTypeComponents(constant->type));
        VIR_Operand_SetSwizzle(newOperand, swizzle);
    }
    else
    {
        VIR_Operand_SetSymbol(newOperand, Function, SourceSymId);
        swizzle = VIR_Enable_2_Swizzle_WShift(origEnable);
        VIR_Operand_SetSwizzle(newOperand, swizzle);
        VIR_Operand_SetMatrixConstIndex(newOperand, SourceMatrixIndex);
        /* Set the offset. */
        if (SourceOffsetKind == VIR_SYM_CONST)
        {
            if (SourceOffset != 0)
            {
                VIR_Operand_SetIsConstIndexing(newOperand, gcvTRUE);
                VIR_Operand_SetRelIndexingImmed(newOperand, SourceOffset);
            }
        }
        else
        {
            VIR_Operand_SetRelIndexing(newOperand, SourceOffset);
            VIR_Operand_SetRelAddrMode(newOperand, VIR_INDEXED_X);
        }
    }

    VIR_Operand_SetTypeId(newOperand, DestTypeId);
    VIR_Inst_SetSource(movInst, 0, newOperand);

    return errCode;
}

VSC_ErrCode
VIR_Shader_GenVectorAssignment(
    IN OUT  VIR_Shader*     pShader,
    IN  VIR_Function*       pFunction,
    IN  VIR_Instruction*    pInst,
    IN  VIR_SymId           DestSymId,
    IN  VIR_SymbolKind      DestOffsetKind,
    IN  VIR_SymId           DestOffset,
    IN  VIR_SymbolKind      SourceSymKind,
    IN  VIR_SymId           SourceSymId,
    IN  gctUINT             StartChannel
    )
{
    VSC_ErrCode             errCode   = VSC_ERR_NONE;
    VIR_TypeId              constituentTypeId;
    VIR_Instruction*        pMovInst;
    VIR_Operand*            pNewOpnd;
    VIR_Enable              enable = VIR_ENABLE_NONE;
    VIR_Swizzle             swizzle;
    gctUINT                 i, channelCount;
    VIR_Const*              pConst;
    VIR_Symbol*             pSym;

    if (SourceSymKind == VIR_SYM_CONST)
    {
        pConst = VIR_Shader_GetConstFromId(pShader, SourceSymId);
        constituentTypeId = pConst->type;
    }
    else
    {
        pSym = VIR_Shader_GetSymFromId(pShader, SourceSymId);
        constituentTypeId = VIR_Symbol_GetTypeId(pSym);
    }
    channelCount = VIR_GetTypeComponents(constituentTypeId);

    /* MOV instruction */
    if (pInst != gcvNULL)
    {
        errCode = VIR_Function_AddInstructionBefore(pFunction,
                                                    VIR_OP_MOV,
                                                    constituentTypeId,
                                                    pInst,
                                                    gcvTRUE,
                                                    &pMovInst);
        CHECK_ERROR(errCode, "Add instruction failed.");
    }
    else
    {
        errCode = VIR_Function_AddInstruction(pFunction,
                                              VIR_OP_MOV,
                                              constituentTypeId,
                                              &pMovInst);
        CHECK_ERROR(errCode, "Add instruction failed.");
    }

    /* Set DEST. */
    pNewOpnd = VIR_Inst_GetDest(pMovInst);
    VIR_Operand_SetSymbol(pNewOpnd, pFunction, DestSymId);
    VIR_Operand_SetTypeId(pNewOpnd, constituentTypeId);
    /* Set the enable. */
    for (i = StartChannel; i < StartChannel + channelCount; i++)
    {
        enable = (VIR_Enable)(enable | (VIR_ENABLE_X << i));
    }
    VIR_Operand_SetEnable(pNewOpnd, enable);

    /* Set the offset. */
    if (DestOffsetKind == VIR_SYM_CONST)
    {
        if (DestOffset != 0)
        {
            VIR_Operand_SetIsConstIndexing(pNewOpnd, gcvTRUE);
            VIR_Operand_SetRelIndexingImmed(pNewOpnd, DestOffset);
        }
    }
    else
    {
        VIR_Operand_SetRelIndexing(pNewOpnd, DestOffset);
        VIR_Operand_SetRelAddrMode(pNewOpnd, VIR_INDEXED_X);
    }

    /* Set SOURCE0. */
    pNewOpnd = VIR_Inst_GetSource(pMovInst, VIR_Operand_Src0);
    VIR_Operand_SetTypeId(pNewOpnd, constituentTypeId);
    if (SourceSymKind == VIR_SYM_CONST)
    {
        pConst = VIR_Shader_GetConstFromId(pShader, SourceSymId);
        VIR_Operand_SetConst(pNewOpnd, pConst->type, SourceSymId);
    }
    else
    {
        VIR_Operand_SetSymbol(pNewOpnd, pFunction, SourceSymId);
    }
    swizzle = VIR_TypeId_Conv2Swizzle(constituentTypeId);
    /* Set the swizzle. */
    swizzle = VIR_Swizzle_SwizzleWShiftEnable(swizzle, enable);
    VIR_Operand_SetSwizzle(pNewOpnd, swizzle);

    return errCode;
}

static gctBOOL
_IsSimpleConstruct(
    IN OUT  VIR_Shader     *Shader,
    IN  VIR_Type           *Type
    )
{
    VIR_TypeKind            typeKind = VIR_Type_GetKind(Type);

    if (typeKind == VIR_TY_VECTOR || typeKind == VIR_TY_SCALAR)
    {
        return gcvTRUE;
    }

    return gcvFALSE;
}

static VSC_ErrCode
_UpdateOffset(
    IN OUT  VIR_Shader     *Shader,
    IN  VIR_Function       *Function,
    IN  VIR_Instruction    *Inst,
    IN  VIR_SymbolKind      DataKind,
    IN  VIR_SymId           Data,
    IN  VIR_SymbolKind      OrigOffsetKind,
    IN  VIR_SymId           OrigOffset,
    IN  VIR_SymbolKind     *NewOffsetKind,
    IN  VIR_SymId          *NewOffset
    )
{
    VSC_ErrCode             errCode = VSC_ERR_NONE;
    VIR_VirRegId            regId;
    VIR_SymbolKind          newOffsetKind;
    VIR_SymId               newOffset;
    VIR_SymId               regSymId;
    VIR_Instruction        *newInst;
    VIR_Operand            *newOperand;

    if (DataKind == VIR_SYM_CONST && OrigOffsetKind == VIR_SYM_CONST)
    {
        newOffsetKind = VIR_SYM_CONST;
        newOffset = Data + OrigOffset;
    }
    else
    {
        /* Create a new temp register to save the offset. */
        regId = VIR_Shader_NewVirRegId(Shader, 1);
        errCode = VIR_Shader_AddSymbol(Shader,
            VIR_SYM_VIRREG,
            regId,
            VIR_Shader_GetTypeFromId(Shader, VIR_TYPE_UINT32),
            VIR_STORAGE_UNKNOWN,
            &regSymId);
        CHECK_ERROR(errCode, "VIR_Shader_AddSymbol failed.");

        /* Insert a ADD. */
        errCode = VIR_Function_AddInstructionBefore(Function,
            VIR_OP_ADD,
            VIR_TYPE_UINT32,
            Inst,
            gcvTRUE,
            &newInst);
        CHECK_ERROR(errCode, "VIR_Function_AddInstructionBefore failed.");

        /* Set DEST. */
        newOperand = VIR_Inst_GetDest(newInst);
        VIR_Operand_SetTempRegister(newOperand, Function, regSymId, VIR_TYPE_UINT32);
        VIR_Operand_SetEnable(newOperand, VIR_ENABLE_X);
        VIR_Inst_SetDest(newInst, newOperand);

        /* Set SOURCE0. */
        newOperand = VIR_Inst_GetSource(newInst, 0);
        if (DataKind == VIR_SYM_CONST)
        {
            VIR_Operand_SetImmediateUint(newOperand, Data);
        }
        else
        {
            gcmASSERT(DataKind == VIR_SYM_VIRREG);
            VIR_Operand_SetTempRegister(newOperand, Function, Data, VIR_TYPE_UINT32);
            VIR_Operand_SetSwizzle(newOperand, VIR_SWIZZLE_XXXX);
        }
        VIR_Inst_SetSource(newInst, 0, newOperand);

        /* Set SOURCE1. */
        newOperand = VIR_Inst_GetSource(newInst, 1);
        if (OrigOffsetKind == VIR_SYM_CONST)
        {
            VIR_Operand_SetImmediateUint(newOperand, OrigOffset);
        }
        else
        {
            gcmASSERT(OrigOffsetKind == VIR_SYM_VIRREG);
            VIR_Operand_SetTempRegister(newOperand, Function, OrigOffset, VIR_TYPE_UINT32);
            VIR_Operand_SetSwizzle(newOperand, VIR_SWIZZLE_XXXX);
        }
        VIR_Inst_SetSource(newInst, 1, newOperand);

        newOffset = regSymId;
        newOffsetKind = VIR_SYM_VIRREG;
    }

    if (NewOffset)
    {
        *NewOffset = newOffset;
    }
    if (NewOffsetKind)
    {
        *NewOffsetKind = newOffsetKind;
    }

    return errCode;
}

VSC_ErrCode
VIR_Shader_GenMatrixAssignment(
    IN OUT  VIR_Shader     *Shader,
    IN  VIR_Function       *Function,
    IN  VIR_Instruction    *Inst,
    IN  VIR_Type           *Type,
    IN  VIR_SymId           DestSymId,
    IN  VIR_SymId           SourceSymId,
    IN  VIR_SymbolKind      DestOffsetKind,
    IN  VIR_SymId           DestOffset,
    IN  VIR_SymbolKind      SourceOffsetKind,
    IN  VIR_SymId           SourceOffset
    )
{
    VSC_ErrCode             errCode = VSC_ERR_NONE;
    VIR_SymId               destSymId = DestSymId;
    VIR_SymId               sourceSymId = SourceSymId;
    VIR_Symbol             *destSym = VIR_Shader_GetSymFromId(Shader, destSymId);
    VIR_Symbol             *sourceSym = VIR_Shader_GetSymFromId(Shader, sourceSymId);
    VIR_SymbolKind          destOffsetKind = DestOffsetKind;
    VIR_SymId               destOffset = DestOffset;
    VIR_SymbolKind          sourceOffsetKind = SourceOffsetKind;
    VIR_SymId               sourceOffset = SourceOffset;
    VIR_VirRegId            destRegId = VIR_INVALID_ID;
    VIR_VirRegId            sourceRegId = VIR_INVALID_ID;
    gctINT                  rowCount = VIR_GetTypeRows(VIR_Type_GetIndex(Type));
    gctINT                  i;
    gctUINT                 destMatrixIndex = 0, sourceMatrixIndex = 0;
    gctBOOL                 updateDest = gcvFALSE, updateSource = gcvFALSE;

    /* Change DEST and SOURCE from variable to vir reg if needed. */
    if (VIR_Symbol_GetKind(destSym) != VIR_SYM_UNIFORM)
    {
        destRegId = VIR_Symbol_GetVregIndex(destSym);
        errCode = VIR_Shader_GetVirRegSymByVirRegId(Shader,
            destRegId,
            &destSymId);
        CHECK_ERROR(errCode, "VIR_Shader_GetVirRegSymByVirRegId");
        updateDest = gcvTRUE;
    }
    if (VIR_Symbol_NeedReplaceSymWithReg(sourceSym))
    {
        sourceRegId = VIR_Symbol_GetVregIndex(sourceSym);
        errCode = VIR_Shader_GetVirRegSymByVirRegId(Shader,
            sourceRegId,
            &sourceSymId);
        CHECK_ERROR(errCode, "VIR_Shader_GetVirRegSymByVirRegId");
        sourceSym = VIR_Shader_GetSymFromId(Shader, sourceSymId);
        updateSource = gcvTRUE;
    }

    /* Generate MOVs for all row assignment. */
    for (i = 0; i < rowCount; i++)
    {
        errCode = VIR_Shader_GenSimpleAssignment(Shader,
            Function,
            Inst,
            destSymId,
            VIR_GetTypeRowType(VIR_Type_GetIndex(Type)),
            destOffsetKind,
            destOffset,
            VIR_SYM_VARIABLE,
            sourceSymId,
            sourceOffsetKind,
            sourceOffset,
            0,
            destMatrixIndex,
            sourceMatrixIndex);
        CHECK_ERROR(errCode, "VIR_Shader_GenSimpleAssignment");

        if (updateDest)
        {
            destRegId++;
            errCode = VIR_Shader_GetVirRegSymByVirRegId(Shader,
                destRegId,
                &destSymId);
            CHECK_ERROR(errCode, "VIR_Shader_GetVirRegSymByVirRegId");
        }
        else
        {
            destMatrixIndex++;
        }

        if (updateSource)
        {
            sourceRegId++;
            errCode = VIR_Shader_GetVirRegSymByVirRegId(Shader,
                sourceRegId,
                &sourceSymId);
            CHECK_ERROR(errCode, "VIR_Shader_GetVirRegSymByVirRegId");
        }
        else
        {
            sourceMatrixIndex++;
        }
    }

    return errCode;
}

VSC_ErrCode
VIR_Shader_GenArrayAssignment(
    IN OUT  VIR_Shader     *Shader,
    IN  VIR_Function       *Function,
    IN  VIR_Instruction    *Inst,
    IN  VIR_Type           *Type,
    IN  VIR_SymId           DestSymId,
    IN  VIR_SymId           SourceSymId,
    IN  VIR_SymbolKind      DestOffsetKind,
    IN  VIR_SymId           DestOffset,
    IN  VIR_SymbolKind      SourceOffsetKind,
    IN  VIR_SymId           SourceOffset
    )
{
    VSC_ErrCode             errCode = VSC_ERR_NONE;
    VIR_SymId               destSymId = DestSymId;
    VIR_SymId               sourceSymId = SourceSymId;
    VIR_Symbol             *destSym = VIR_Shader_GetSymFromId(Shader, destSymId);
    VIR_Symbol             *sourceSym = VIR_Shader_GetSymFromId(Shader, sourceSymId);
    VIR_SymbolKind          destOffsetKind = DestOffsetKind;
    VIR_SymId               destOffset = DestOffset;
    VIR_SymbolKind          sourceOffsetKind = SourceOffsetKind;
    VIR_SymId               sourceOffset = SourceOffset;
    VIR_VirRegId            destRegId = VIR_INVALID_ID;
    VIR_VirRegId            sourceRegId = VIR_INVALID_ID;
    VIR_Type               *baseType = VIR_Shader_GetTypeFromId(Shader, VIR_Type_GetBaseTypeId(Type));
    VIR_TypeKind            baseTypeKind = VIR_Type_GetKind(baseType);
    gctUINT                 arrayLength = VIR_Type_GetArrayLength(Type);
    gctUINT                 i;
    gctINT                  arrayRegCount = VIR_Type_GetRegCount(Shader, baseType, gcvFALSE);
    gctBOOL                 updateDest = gcvFALSE, updateSource = gcvFALSE;

    /* Change DEST and SOURCE from variable to vir reg if needed. */
    if (VIR_Symbol_GetKind(destSym) != VIR_SYM_UNIFORM)
    {
        destRegId = VIR_Symbol_GetVregIndex(destSym);
        errCode = VIR_Shader_GetVirRegSymByVirRegId(Shader,
            destRegId,
            &destSymId);
        CHECK_ERROR(errCode, "VIR_Shader_GetVirRegSymByVirRegId");
        updateDest = gcvTRUE;
    }
    if (VIR_Symbol_NeedReplaceSymWithReg(sourceSym))
    {
        sourceRegId = VIR_Symbol_GetVregIndex(sourceSym);
        errCode = VIR_Shader_GetVirRegSymByVirRegId(Shader,
            sourceRegId,
            &sourceSymId);
        CHECK_ERROR(errCode, "VIR_Shader_GetVirRegSymByVirRegId");
        sourceSym = VIR_Shader_GetSymFromId(Shader, sourceSymId);
        updateSource = gcvTRUE;
    }

    for (i = 0; i < arrayLength; i++)
    {
        /* Generate MOVs for every array element. */
        if (_IsSimpleConstruct(Shader, baseType))
        {
            errCode = VIR_Shader_GenSimpleAssignment(Shader,
                Function,
                Inst,
                destSymId,
                VIR_Type_GetIndex(baseType),
                destOffsetKind,
                destOffset,
                VIR_SYM_VARIABLE,
                sourceSymId,
                sourceOffsetKind,
                sourceOffset,
                0,
                0,
                0);
            CHECK_ERROR(errCode, "VIR_Shader_GenSimpleAssignment");
        }
        else if (baseTypeKind == VIR_TY_MATRIX)
        {
            errCode = VIR_Shader_GenMatrixAssignment(Shader,
                Function,
                Inst,
                baseType,
                destSymId,
                sourceSymId,
                destOffsetKind,
                destOffset,
                sourceOffsetKind,
                sourceOffset);
            CHECK_ERROR(errCode, "VIR_Shader_GenMatrixAssignment");

        }
        else if (baseTypeKind == VIR_TY_STRUCT)
        {
            errCode = VIR_Shader_GenStructAssignment(Shader,
                Function,
                Inst,
                baseType,
                destSymId,
                sourceSymId,
                destOffsetKind,
                destOffset,
                sourceOffsetKind,
                sourceOffset);
            CHECK_ERROR(errCode, "VIR_Shader_GenStructAssignment");
        }
        else if (baseTypeKind == VIR_TY_ARRAY)
        {
            errCode = VIR_Shader_GenArrayAssignment(Shader,
                Function,
                Inst,
                baseType,
                destSymId,
                sourceSymId,
                destOffsetKind,
                destOffset,
                sourceOffsetKind,
                sourceOffset);
            CHECK_ERROR(errCode, "VIR_Shader_GenArrayAssignment");
        }

        /* Update DEST and SOURCE offset and offset kind. */
        if (updateDest)
        {
            destRegId += arrayRegCount;
            errCode = VIR_Shader_GetVirRegSymByVirRegId(Shader,
                destRegId,
                &destSymId);
            CHECK_ERROR(errCode, "VIR_Shader_GetVirRegSymByVirRegId");
        }
        else
        {
            errCode = _UpdateOffset(Shader,
                Function,
                Inst,
                VIR_SYM_CONST,
                arrayRegCount,
                destOffsetKind,
                destOffset,
                &destOffsetKind,
                &destOffset);
            CHECK_ERROR(errCode, "_UpdateOffset");
        }

        if (updateSource)
        {
            sourceRegId += arrayRegCount;
            errCode = VIR_Shader_GetVirRegSymByVirRegId(Shader,
                sourceRegId,
                &sourceSymId);
            CHECK_ERROR(errCode, "VIR_Shader_GetVirRegSymByVirRegId");
        }
        else
        {
            errCode = _UpdateOffset(Shader,
                Function,
                Inst,
                VIR_SYM_CONST,
                arrayRegCount,
                sourceOffsetKind,
                sourceOffset,
                &sourceOffsetKind,
                &sourceOffset);
            CHECK_ERROR(errCode, "_UpdateOffset");
        }
    }

    return errCode;
}

VSC_ErrCode
VIR_Shader_GenStructAssignment(
    IN OUT  VIR_Shader     *Shader,
    IN  VIR_Function       *Function,
    IN  VIR_Instruction    *Inst,
    IN  VIR_Type           *Type,
    IN  VIR_SymId           DestSymId,
    IN  VIR_SymId           SourceSymId,
    IN  VIR_SymbolKind      DestOffsetKind,
    IN  VIR_SymId           DestOffset,
    IN  VIR_SymbolKind      SourceOffsetKind,
    IN  VIR_SymId           SourceOffset
    )
{
    VSC_ErrCode             errCode = VSC_ERR_NONE;
    VIR_SymId               destSymId = DestSymId;
    VIR_SymId               sourceSymId = SourceSymId;
    VIR_Symbol             *destSym = VIR_Shader_GetSymFromId(Shader, destSymId);
    VIR_Symbol             *sourceSym = VIR_Shader_GetSymFromId(Shader, sourceSymId);
    VIR_SymbolKind          destOffsetKind = DestOffsetKind;
    VIR_SymId               destOffset = DestOffset;
    VIR_SymbolKind          sourceOffsetKind = SourceOffsetKind;
    VIR_SymId               sourceOffset = SourceOffset;
    VIR_VirRegId            destRegId = VIR_INVALID_ID;
    VIR_VirRegId            sourceRegId = VIR_INVALID_ID;
    VIR_SymIdList          *fields = VIR_Type_GetFields(Type);
    gctUINT                 fieldLength = VIR_IdList_Count(fields);
    gctUINT                 i;
    gctBOOL                 updateDest = gcvFALSE, updateSource = gcvFALSE;

    /* Change DEST and SOURCE from variable to vir reg if needed. */
    if (VIR_Symbol_GetKind(destSym) != VIR_SYM_UNIFORM)
    {
        destRegId = VIR_Symbol_GetVregIndex(destSym);
        errCode = VIR_Shader_GetVirRegSymByVirRegId(Shader,
            destRegId,
            &destSymId);
        CHECK_ERROR(errCode, "VIR_Shader_GetVirRegSymByVirRegId");
        updateDest = gcvTRUE;
    }
    if (VIR_Symbol_NeedReplaceSymWithReg(sourceSym))
    {
        sourceRegId = VIR_Symbol_GetVregIndex(sourceSym);
        errCode = VIR_Shader_GetVirRegSymByVirRegId(Shader,
            sourceRegId,
            &sourceSymId);
        CHECK_ERROR(errCode, "VIR_Shader_GetVirRegSymByVirRegId");
        sourceSym = VIR_Shader_GetSymFromId(Shader, sourceSymId);
        updateSource = gcvTRUE;
    }

    for (i = 0; i < fieldLength; i++)
    {
        VIR_Id              id = VIR_IdList_GetId(VIR_Type_GetFields(Type), i);
        VIR_Symbol         *fieldSym = VIR_Shader_GetSymFromId(Shader, id);
        VIR_Type           *fieldType = VIR_Symbol_GetType(fieldSym);
        VIR_TypeKind        fieldTypeKind = VIR_Type_GetKind(fieldType);
        gctUINT             fieldOffset = VIR_Type_GetRegCount(Shader, fieldType, gcvFALSE);

        /* Generate MOVs for every field. */
        if (_IsSimpleConstruct(Shader, fieldType))
        {
            errCode = VIR_Shader_GenSimpleAssignment(Shader,
                Function,
                Inst,
                destSymId,
                VIR_Type_GetIndex(fieldType),
                destOffsetKind,
                destOffset,
                VIR_SYM_VARIABLE,
                sourceSymId,
                sourceOffsetKind,
                sourceOffset,
                0,
                0,
                0);
            CHECK_ERROR(errCode, "VIR_Shader_GenSimpleAssignment");
        }
        else if (fieldTypeKind == VIR_TY_MATRIX)
        {
            errCode = VIR_Shader_GenMatrixAssignment(Shader,
                Function,
                Inst,
                fieldType,
                destSymId,
                sourceSymId,
                destOffsetKind,
                destOffset,
                sourceOffsetKind,
                sourceOffset);
            CHECK_ERROR(errCode, "VIR_Shader_GenMatrixAssignment");

        }
        else if (fieldTypeKind == VIR_TY_STRUCT)
        {
            errCode = VIR_Shader_GenStructAssignment(Shader,
                Function,
                Inst,
                fieldType,
                destSymId,
                sourceSymId,
                destOffsetKind,
                destOffset,
                sourceOffsetKind,
                sourceOffset);
            CHECK_ERROR(errCode, "VIR_Shader_GenStructAssignment");
        }
        else if (fieldTypeKind == VIR_TY_ARRAY)
        {
            errCode = VIR_Shader_GenArrayAssignment(Shader,
                Function,
                Inst,
                fieldType,
                destSymId,
                sourceSymId,
                destOffsetKind,
                destOffset,
                sourceOffsetKind,
                sourceOffset);
            CHECK_ERROR(errCode, "VIR_Shader_GenArrayAssignment");
        }

        /* Update DEST and SOURCE offset and offset kind. */
        if (updateDest)
        {
            destRegId += fieldOffset;
            errCode = VIR_Shader_GetVirRegSymByVirRegId(Shader,
                destRegId,
                &destSymId);
            CHECK_ERROR(errCode, "VIR_Shader_GetVirRegSymByVirRegId");
        }
        else
        {
            errCode = _UpdateOffset(Shader,
                Function,
                Inst,
                VIR_SYM_CONST,
                fieldOffset,
                destOffsetKind,
                destOffset,
                &destOffsetKind,
                &destOffset);
            CHECK_ERROR(errCode, "_UpdateOffset");
        }

        if (updateSource)
        {
            sourceRegId += fieldOffset;
            errCode = VIR_Shader_GetVirRegSymByVirRegId(Shader,
                sourceRegId,
                &sourceSymId);
            CHECK_ERROR(errCode, "VIR_Shader_GetVirRegSymByVirRegId");
        }
        else
        {
            errCode = _UpdateOffset(Shader,
                Function,
                Inst,
                VIR_SYM_CONST,
                fieldOffset,
                sourceOffsetKind,
                sourceOffset,
                &sourceOffsetKind,
                &sourceOffset);
            CHECK_ERROR(errCode, "_UpdateOffset");
        }
    }

    return errCode;
}

/* Gen codes to construct a composite variable. */
VSC_ErrCode
VIR_Shader_CompositeConstruct(
    IN OUT  VIR_Shader     *Shader,
    IN  VIR_Function       *Function,
    IN  VIR_Instruction    *Inst,
    IN  VIR_SymId           SymId,
    IN  VIR_TypeId          TypeId,
    IN  gctBOOL             ConstructWithNull,
    IN  VIR_SymId          *CompositeSymId,
    IN  VIR_SymbolKind     *CompositeSymKind,
    IN  gctUINT             CompositeSymLength
    )
{
    VSC_ErrCode             errCode = VSC_ERR_NONE;
    VIR_Type               *type = VIR_Shader_GetTypeFromId(Shader, TypeId);
    VIR_TypeKind            destTypeKind = VIR_Type_GetKind(type);
    gctUINT                 destOffset = 0;
    gctUINT                 i;

    /* Construct this variable with NULL. */
    if (ConstructWithNull)
    {
        errCode = VIR_Shader_GenNullAssignment(Shader,
            Function,
            Inst,
            SymId,
            TypeId);
        CHECK_ERROR(errCode, "VIR_Shader_GenNullAssignment");

        return errCode;
    }

    /* If this construction is a simple composite construct,
    ** generate MOVs for all composite syms.
    */
    if (_IsSimpleConstruct(Shader, type))
    {
        gctUINT StartChannel = 0;

        for (i = 0; i < CompositeSymLength; i++)
        {
            errCode = VIR_Shader_GenVectorAssignment(Shader,
                                                     Function,
                                                     Inst,
                                                     SymId,
                                                     VIR_SYM_CONST,
                                                     0,
                                                     CompositeSymKind[i],
                                                     CompositeSymId[i],
                                                     StartChannel);
            CHECK_ERROR(errCode, "Gen vector assignment error.");

            if (i != CompositeSymLength - 1)
            {
                if (CompositeSymKind[i] == VIR_SYM_CONST)
                {
                    VIR_Const*  pConst = VIR_Shader_GetConstFromId(Shader, CompositeSymId[i]);
                    StartChannel += VIR_GetTypeComponents(pConst->type);
                }
                else
                {
                    VIR_Symbol* pSym = VIR_Shader_GetSymFromId(Shader, CompositeSymId[i]);
                    StartChannel += VIR_GetTypeComponents(VIR_Symbol_GetTypeId(pSym));
                }
            }
        }
    }
    else
    {
        /* If this construct is for a matrix,
        ** the composite sym is a row element, generate MOVs for all rows.
        */
        if (destTypeKind == VIR_TY_MATRIX)
        {
            gcmASSERT((gctINT)CompositeSymLength == VIR_GetTypeRows(VIR_Type_GetIndex(type)));

            for (i = 0; i < CompositeSymLength; i++)
            {
                errCode = VIR_Shader_GenSimpleAssignment(Shader,
                    Function,
                    Inst,
                    SymId,
                    VIR_GetTypeRowType(VIR_Type_GetIndex(type)),
                    VIR_SYM_CONST,
                    0,
                    CompositeSymKind[i],
                    CompositeSymId[i],
                    VIR_SYM_CONST,
                    0,
                    0,
                    i,
                    0);
                CHECK_ERROR(errCode, "VIR_Shader_GenSimpleAssignment");
            }
        }
        /* If this construct is for an array,
        ** the composite sym is an array element, handle all array elements.
        */
        else if (destTypeKind == VIR_TY_ARRAY)
        {
            VIR_Type       *baseType = VIR_Shader_GetTypeFromId(Shader,
                VIR_Type_GetBaseTypeId(type));
            VIR_TypeKind    baseTypeKind = VIR_Type_GetKind(baseType);
            gctINT          arrayRegCount = VIR_Type_GetRegCount(Shader, baseType, gcvFALSE);

            gcmASSERT(VIR_Type_GetArrayLength(type) == CompositeSymLength);

            for (i = 0; i < CompositeSymLength; i++)
            {
                /* If the array element is a simple construct, generate MOVs for all elements. */
                if (baseTypeKind == VIR_TY_VECTOR || baseTypeKind == VIR_TY_SCALAR)
                {
                    errCode = VIR_Shader_GenSimpleAssignment(Shader,
                        Function,
                        Inst,
                        SymId,
                        VIR_Type_GetIndex(baseType),
                        VIR_SYM_CONST,
                        destOffset,
                        CompositeSymKind[i],
                        CompositeSymId[i],
                        VIR_SYM_CONST,
                        0,
                        0,
                        0,
                        0);
                    CHECK_ERROR(errCode, "VIR_Shader_GenSimpleAssignment");
                }
                /* If the array element is a complex construct, just a MOV, HL->ML will split this MOV. */
                else
                {
                    gcmASSERT(baseTypeKind == VIR_TY_MATRIX ||
                        baseTypeKind == VIR_TY_STRUCT ||
                        baseTypeKind == VIR_TY_ARRAY);

                    errCode = _ConstructComplexVariable(Shader,
                        Function,
                        Inst,
                        baseType,
                        SymId,
                        CompositeSymId[i],
                        destOffset);
                    CHECK_ERROR(errCode, "_ConstructComplexVariable");
                }

                destOffset += arrayRegCount;
            }
        }
        /* If this construct is for a struct,
        ** the composite sym is a struct field, handle all struct fields.
        */
        else
        {
            VIR_SymIdList *fields = VIR_Type_GetFields(type);

            gcmASSERT(destTypeKind == VIR_TY_STRUCT);
            gcmASSERT(VIR_IdList_Count(fields) == CompositeSymLength);

            for (i = 0; i < CompositeSymLength; i++)
            {
                VIR_Id          id = VIR_IdList_GetId(fields, i);
                VIR_Symbol     *fieldSym = VIR_Shader_GetSymFromId(Shader, id);
                VIR_Type       *fieldType = VIR_Symbol_GetType(fieldSym);

                /* If this field is a simple construct, generate MOV for this field. */
                if (_IsSimpleConstruct(Shader, fieldType))
                {
                    errCode = VIR_Shader_GenSimpleAssignment(Shader,
                        Function,
                        Inst,
                        SymId,
                        VIR_Type_GetIndex(fieldType),
                        VIR_SYM_CONST,
                        destOffset,
                        CompositeSymKind[i],
                        CompositeSymId[i],
                        VIR_SYM_CONST,
                        0,
                        0,
                        0,
                        0);
                    CHECK_ERROR(errCode, "VIR_Shader_GenSimpleAssignment");
                }
                /* If the filed is a complex construct, just a MOV, HL->ML will split this MOV. */
                else
                {
                    gcmASSERT(VIR_Type_GetKind(fieldType) == VIR_TY_MATRIX ||
                        VIR_Type_GetKind(fieldType) == VIR_TY_STRUCT ||
                        VIR_Type_GetKind(fieldType) == VIR_TY_ARRAY);

                    errCode = _ConstructComplexVariable(Shader,
                        Function,
                        Inst,
                        fieldType,
                        SymId,
                        CompositeSymId[i],
                        destOffset);
                    CHECK_ERROR(errCode, "_ConstructComplexVariable");
                }

                destOffset += VIR_Type_GetRegCount(Shader, fieldType, gcvFALSE);
            }
        }
    }


    return errCode;
}

VSC_ErrCode
VIR_Shader_CreateSymAliasTable(
    IN OUT  VIR_Shader      *Shader
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    VIR_SymAliasTable* table = VIR_Shader_GetSymAliasTable(Shader);

    VIR_SymAliasTable_SetMM(table, &Shader->pmp.mmWrapper);
    VIR_SymAliasTable_SetHashTable(table, vscHTBL_Create(&Shader->pmp.mmWrapper, vscHFUNC_Default, vscHKCMP_Default, 512));

    return errCode;
}

VSC_ErrCode
VIR_Shader_DestroySymAliasTable(
    IN OUT  VIR_Shader      *Shader
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    VIR_SymAliasTable* table = VIR_Shader_GetSymAliasTable(Shader);

    vscHTBL_Destroy(VIR_SymAliasTable_GetHashTable(table));
    VIR_SymAliasTable_SetHashTable(table, gcvNULL);
    VIR_SymAliasTable_SetMM(table, gcvNULL);

    return errCode;
}

VIR_SymAliasTable*
VIR_Shader_GetCreatedSymAliasTable(
    IN OUT  VIR_Shader      *Shader
    )
{
    VIR_SymAliasTable* table = VIR_Shader_GetSymAliasTable(Shader);

    if(!VIR_SymAliasTable_IsCreated(table))
    {
        VIR_Shader_CreateSymAliasTable(Shader);
    }

    return table;
}

VSC_ErrCode
VIR_Shader_UpdateCallParmAssignment(
    IN  VIR_Shader          *pShader,
    IN  VIR_Function        *pCalleeFunc,
    IN  VIR_Function        *pLibCalleeFunc,
    IN  VIR_Function        *pCallerFunc,
    IN  VIR_Instruction     *pCallerInParamInst, /* search IN param from this inst forward */
    IN  VIR_Instruction     *pCallerOutParamInst, /* search OUT param from this inst afterward */
    IN  gctBOOL             bMapTemp,
    IN  VSC_HASH_TABLE      *pTempSet
    )
{
    VSC_ErrCode             errCode = VSC_ERR_NONE;
    gctUINT                 i, j, k;
    VIR_SymId               parmSymId, parmVregId;
    VIR_Symbol              *parmSym, *parmVregSym;
    VIR_SymId               calleeParmSymId, calleeParmVregId;
    VIR_Symbol              *calleeParmSym = gcvNULL, *calleeParmVregSym = gcvNULL;
    VIR_Symbol              *newParmVregSym = gcvNULL;
    VIR_TypeId              parmTypeId, parmBaseTypeId;
    VIR_Type                *parmType;
    VIR_Instruction         *newInst;
    VIR_Instruction         *pInInstIter, *pOutInstIter;
    VIR_Operand             *pOpnd;
    gctUINT                 regCount;
    gctUINT                 compCount;
    gctBOOL                 bInCompMask[VIR_CHANNEL_COUNT], bOutCompMask[VIR_CHANNEL_COUNT];
    gctBOOL                 bUpdateParamWithLibCalleeFunc = (pCalleeFunc != pLibCalleeFunc);

    for (i = 0; i < VIR_IdList_Count(&pLibCalleeFunc->paramters); i++)
    {
        parmSymId = VIR_IdList_GetId(&pLibCalleeFunc->paramters, i);
        /* param symbol in func */
        parmSym = VIR_Function_GetSymFromId(pLibCalleeFunc, parmSymId);
        parmVregId = VIR_Symbol_GetVariableVregIndex(parmSym);

        parmTypeId = VIR_Symbol_GetTypeId(parmSym);
        parmBaseTypeId = VIR_Type_GetBaseTypeId(VIR_Shader_GetTypeFromId(pShader, parmTypeId));

        parmType = VIR_Shader_GetTypeFromId(pShader, parmTypeId);
        regCount = VIR_Type_GetRegOrOpaqueCount(pShader,
                                                parmType,
                                                VIR_TypeId_isSampler(parmBaseTypeId),
                                                VIR_TypeId_isImage(parmBaseTypeId),
                                                VIR_TypeId_isAtomicCounters(parmBaseTypeId),
                                                gcvFALSE);
        compCount = VIR_Type_isPointer(parmType) ? 1 : VIR_GetTypeComponents(parmBaseTypeId);
        gcmASSERT(compCount <= VIR_CHANNEL_COUNT);

        /* go through every virReg in the parameter (long/ulong has two)
         * before lowered to lower level, only first virReg is used in
         * argument passing, after that, both virReg are used, so we need
         * to adjust any/all of them
         */
        for (j = 0; j < regCount; j++)
        {
            gctBOOL bCheckInParm = gcvTRUE, bCheckOutParm = gcvTRUE;

            pInInstIter = pCallerInParamInst;
            pOutInstIter = pCallerOutParamInst;

            if (bUpdateParamWithLibCalleeFunc)
            {
                /* I: Get the parameter from the main shader. */
                calleeParmSymId = VIR_IdList_GetId(&pCalleeFunc->paramters, i);
                calleeParmSym = VIR_Function_GetSymFromId(pCalleeFunc, calleeParmSymId);
                calleeParmVregId = VIR_Symbol_GetVariableVregIndex(calleeParmSym);
                calleeParmVregSym = VIR_Shader_FindSymbolByTempIndex(pShader, calleeParmVregId + j);

                /* II: Get the parameter vreg symbol from the lib shader. */
                parmVregSym = VIR_Shader_FindSymbolByTempIndex(VIR_Function_GetShader(pLibCalleeFunc),
                                                               parmVregId + j);
                /* the virreg is not defined/expanded yet, skip it */
                if (parmVregSym == gcvNULL)
                {
                    continue;
                }

                /* III: Get the mapping vreg symbol. */
                gcmASSERT(pTempSet);
                if (!vscHTBL_DirectTestAndGet(pTempSet, (void*)parmVregSym, (void **)&parmVregSym))
                {
                    gcmASSERT(gcvFALSE);
                }
            }
            else
            {
                /* param vreg symbol */
                parmVregSym = VIR_Shader_FindSymbolByTempIndex(pShader, parmVregId + j);
                /* the virreg is not defined/expanded yet, skip it */
                if (parmVregSym == gcvNULL)
                {
                    continue;
                }
            }

            /* Initialize component mask. */
            for (k = 0; k < VIR_CHANNEL_COUNT; k++)
            {
                if (VIR_Symbol_isInParam(parmSym) && k < compCount)
                {
                    bInCompMask[k] = gcvFALSE;
                }
                else
                {
                    bInCompMask[k] = gcvTRUE;
                }

                if (VIR_Symbol_isOutParam(parmSym) && k < compCount)
                {
                    bOutCompMask[k] = gcvFALSE;
                }
                else
                {
                    bOutCompMask[k] = gcvTRUE;
                }
            }

            do
            {
                /* Check in/inout parameter. */
                if (VIR_Symbol_isInParam(parmSym) && bCheckInParm)
                {
                    newInst = VIR_Shader_FindParmInst(pCalleeFunc, pInInstIter, gcvTRUE, parmVregSym, &pOpnd);
                    if (newInst == gcvNULL)
                    {
                        break;
                    }

                    if (bMapTemp)
                    {
                        gcmASSERT(pTempSet);
                        if (!vscHTBL_DirectTestAndGet(pTempSet, (void*)parmVregSym, (void **)&newParmVregSym))
                        {
                            gcmASSERT(gcvFALSE);
                        }
                    }
                    else if (bUpdateParamWithLibCalleeFunc)
                    {
                        newParmVregSym = calleeParmVregSym;
                    }
                    else
                    {
                        newParmVregSym = parmVregSym;
                    }

                    gcmASSERT(newParmVregSym != gcvNULL);

                    VIR_Operand_SetTempRegister(pOpnd,
                                                pCallerFunc,
                                                VIR_Symbol_GetIndex(newParmVregSym),
                                                VIR_Operand_GetTypeId(pOpnd));

                    /* Updpate component mask. */
                    for (k = 0; k < VIR_CHANNEL_COUNT; k++)
                    {
                        if (VIR_Operand_GetEnable(pOpnd) & (1 << k))
                        {
                            bInCompMask[k] = gcvTRUE;
                        }
                    }

                    /* Update the instIter. */
                    pInInstIter = newInst;
                }
                /* Check out/inout parameter. */
                if (VIR_Symbol_isOutParam(parmSym) && bCheckOutParm)
                {
                    newInst = VIR_Shader_FindParmInst(pCalleeFunc, pOutInstIter, gcvFALSE, parmVregSym, &pOpnd);
                    if (newInst == gcvNULL)
                    {
                        break;
                    }

                    if (bMapTemp)
                    {
                        gcmASSERT(pTempSet);
                        if (!vscHTBL_DirectTestAndGet(pTempSet, (void*)parmVregSym, (void **)&newParmVregSym))
                        {
                            gcmASSERT(gcvFALSE);
                        }
                    }
                    else if (bUpdateParamWithLibCalleeFunc)
                    {
                        newParmVregSym = calleeParmVregSym;
                    }
                    else
                    {
                        newParmVregSym = parmVregSym;
                    }

                    gcmASSERT(newParmVregSym != gcvNULL);

                    VIR_Operand_SetTempRegister(pOpnd,
                                                pCallerFunc,
                                                VIR_Symbol_GetIndex(newParmVregSym),
                                                VIR_Operand_GetTypeId(pOpnd));
                    /* Update the instIter. */
                    pOutInstIter = newInst;
                }

                /* Check if all valid components are updated. */
                if (bInCompMask[0] && bInCompMask[1] && bInCompMask[2] && bInCompMask[3])
                {
                    bCheckInParm = gcvFALSE;
                }
                if (bOutCompMask[0] && bOutCompMask[1] && bOutCompMask[2] && bOutCompMask[3])
                {
                    bCheckOutParm = gcvFALSE;
                }
                /* Check the next reg if all valid components are updated. */
                if (!bCheckInParm && !bCheckOutParm)
                {
                    break;
                }
            } while(gcvTRUE);
        }
    }

    return errCode;
}

VSC_ErrCode
VIR_Shader_CreateAttributeAliasList(
    IN OUT  VIR_Shader*     pShader
    )
{
    VSC_ErrCode             errCode = VSC_ERR_NONE;
    VIR_IdList*             pArrayList = VIR_Shader_GetAttributeAliasList(pShader);
    VIR_IdList*             pList;
    gctUINT                 i;

    if (pArrayList != gcvNULL)
    {
        return errCode;
    }

    pArrayList = (VIR_IdList *)vscMM_Alloc(&pShader->pmp.mmWrapper,
                                           MAX_SHADER_IO_NUM * sizeof(VIR_IdList));
    if (pArrayList == gcvNULL)
    {
        return VSC_ERR_OUT_OF_MEMORY;
    }
    memset(pArrayList, 0, MAX_SHADER_IO_NUM * sizeof(VIR_IdList));
    VIR_Shader_SetAttributeAliasList(pShader, pArrayList);

    for (i = 0; i < MAX_SHADER_IO_NUM; i++)
    {
        pList = &pArrayList[i];

        VIR_IdList_Init(&pShader->pmp.mmWrapper, 2, &pList);
    }

    return errCode;
}

VSC_ErrCode
VIR_Shader_DestroyAttributeAliasList(
    IN OUT  VIR_Shader*     pShader
    )
{
    VSC_ErrCode             errCode = VSC_ERR_NONE;
    VIR_IdList*             pArrayList = VIR_Shader_GetAttributeAliasList(pShader);
    VIR_IdList*             pList;
    gctUINT                 i;

    if (pArrayList == gcvNULL)
    {
        return errCode;
    }

    for (i = 0; i < MAX_SHADER_IO_NUM; i++)
    {
        pList = &pArrayList[i];
        VIR_IdList_Finalize(pList);
    }

    vscMM_Free(&pShader->pmp.mmWrapper, pArrayList);

    VIR_Shader_SetAttributeAliasList(pShader, gcvNULL);

    return errCode;
}

VSC_ErrCode
VIR_Shader_CreateAttributeComponentMapList(
    IN OUT  VIR_Shader*     pShader
    )
{
    VSC_ErrCode             errCode = VSC_ERR_NONE;
    VIR_IdList*             pArrayList = VIR_Shader_GetAttributeComponentMapList(pShader);
    VIR_IdList*             pList;
    gctUINT                 i;

    if (pArrayList != gcvNULL)
    {
        return errCode;
    }

    pArrayList = (VIR_IdList *)vscMM_Alloc(&pShader->pmp.mmWrapper,
                                           MAX_SHADER_IO_NUM * sizeof(VIR_IdList));
    if (pArrayList == gcvNULL)
    {
        return VSC_ERR_OUT_OF_MEMORY;
    }
    memset(pArrayList, 0, MAX_SHADER_IO_NUM * sizeof(VIR_IdList));
    VIR_Shader_SetAttributeComponentMapList(pShader, pArrayList);

    for (i = 0; i < MAX_SHADER_IO_NUM; i++)
    {
        pList = &pArrayList[i];

        VIR_IdList_Init(&pShader->pmp.mmWrapper, 2, &pList);
    }

    return errCode;
}

VSC_ErrCode
VIR_Shader_DestroyAttributeComponentMapList(
    IN OUT  VIR_Shader*     pShader
    )
{
    VSC_ErrCode             errCode = VSC_ERR_NONE;
    VIR_IdList*             pArrayList = VIR_Shader_GetAttributeComponentMapList(pShader);
    VIR_IdList*             pList;
    gctUINT                 i;

    if (pArrayList == gcvNULL)
    {
        return errCode;
    }

    for (i = 0; i < MAX_SHADER_IO_NUM; i++)
    {
        pList = &pArrayList[i];
        VIR_IdList_Finalize(pList);
    }

    vscMM_Free(&pShader->pmp.mmWrapper, pArrayList);

    VIR_Shader_SetAttributeComponentMapList(pShader, gcvNULL);

    return errCode;
}

VSC_ErrCode
VIR_Shader_CreateOutputComponentMapList(
    IN OUT  VIR_Shader*     pShader
    )
{
    VSC_ErrCode             errCode = VSC_ERR_NONE;
    VIR_IdList*             pArrayList = VIR_Shader_GetOutputComponentMapList(pShader);
    VIR_IdList*             pList;
    gctUINT                 i;

    if (pArrayList != gcvNULL)
    {
        return errCode;
    }

    pArrayList = (VIR_IdList *)vscMM_Alloc(&pShader->pmp.mmWrapper,
                                           MAX_SHADER_IO_NUM * sizeof(VIR_IdList));
    if (pArrayList == gcvNULL)
    {
        return VSC_ERR_OUT_OF_MEMORY;
    }
    memset(pArrayList, 0, MAX_SHADER_IO_NUM * sizeof(VIR_IdList));
    VIR_Shader_SetOutputComponentMapList(pShader, pArrayList);

    for (i = 0; i < MAX_SHADER_IO_NUM; i++)
    {
        pList = &pArrayList[i];

        VIR_IdList_Init(&pShader->pmp.mmWrapper, 2, &pList);
    }

    return errCode;
}

VSC_ErrCode
VIR_Shader_DestroyOutputComponentMapList(
    IN OUT  VIR_Shader*     pShader
    )
{
    VSC_ErrCode             errCode = VSC_ERR_NONE;
    VIR_IdList*             pArrayList = VIR_Shader_GetOutputComponentMapList(pShader);
    VIR_IdList*             pList;
    gctUINT                 i;

    if (pArrayList == gcvNULL)
    {
        return errCode;
    }

    for (i = 0; i < MAX_SHADER_IO_NUM; i++)
    {
        pList = &pArrayList[i];
        VIR_IdList_Finalize(pList);
    }

    vscMM_Free(&pShader->pmp.mmWrapper, pArrayList);

    VIR_Shader_SetOutputComponentMapList(pShader, gcvNULL);

    return errCode;
}

/* types */
void
VIR_Type_SetAlignment(
    IN OUT VIR_Type *   Type,
    IN  gctUINT         Alignment
    )
{
    gcmASSERT(
        Alignment == 0 ||
        Alignment == 1 ||
        Alignment == 2 ||
        Alignment == 4 ||
        Alignment == 8 ||
        Alignment == 16 ||
        Alignment == 32 ||
        Alignment == 64 ||
        Alignment == 128);
    switch (Alignment)
    {
    case 0:
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
    IN  VSC_TyQualifier Qualifier
    )
{
    Type->_qualifier = Qualifier;
}

void
VIR_Type_AddAddrSpace(
    IN OUT VIR_Type *   Type,
    IN  VSC_AddrSpace   AS
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
        errCode = VIR_IdList_Init(&Shader->pmp.mmWrapper,
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
    fieldInfo = (VIR_FieldInfo *)vscMM_Alloc(&Shader->pmp.mmWrapper,
        sizeof(VIR_FieldInfo));
    if (fieldInfo == gcvNULL)
    {
        return VSC_ERR_OUT_OF_MEMORY;
    }

    /* set default vaules */
    VIR_FIELDINFO_Initialize(fieldInfo);

    fieldSym->u2.fieldInfo  = fieldInfo;
    return errCode;
}

VSC_ErrCode
VIR_Type_AddFieldAndInfo(
    IN  VIR_Shader *    Shader,
    IN  VIR_Type *      StructType,
    IN  gctSTRING       FieldName,
    IN  gctUINT32       Offset,
    IN  gctINT          ArrayStride,
    IN  gctINT          Matrixstride,
    IN  gctBOOL         IsBitField,
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
    fieldInfo = (VIR_FieldInfo *)vscMM_Alloc(&Shader->pmp.mmWrapper,
        sizeof(VIR_FieldInfo));
    if (fieldInfo == gcvNULL)
    {
        return VSC_ERR_OUT_OF_MEMORY;
    }

    /* set default vaules */
    VIR_FIELDINFO_Initialize(fieldInfo);

    /* set the value */
    fieldInfo->offset       = Offset;
    fieldInfo->arrayStride  = ArrayStride;
    fieldInfo->matrixStride = Matrixstride;
    fieldInfo->isBitfield   = IsBitField;
    fieldInfo->bitSize      = BitSize;
    fieldInfo->startBit     = StartBit;
    fieldInfo->tempRegOrUniformOffset = TempRegOrUniformOffset;

    fieldSym->u2.fieldInfo  = fieldInfo;

    /* add the field symbol to struct type */
    if (StructType->u2.fields == gcvNULL)
    {
        /* create a field list */
        errCode = VIR_IdList_Init(&Shader->pmp.mmWrapper,
            8, /* init field count */
            &StructType->u2.fields);
        CHECK_ERROR(errCode, "AddField");
    }

    errCode = VIR_IdList_Add(StructType->u2.fields, *FieldSymId);
    CHECK_ERROR(errCode, "AddField");

    return errCode;
}

gctUINT VIR_Type_GetVirRegCount(VIR_Shader * Shader,
                                VIR_Type *  Type,
                                gctINT realArraySize)
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
            return ((realArraySize == -1) ? VIR_Type_GetArrayLength(Type) : realArraySize) *
                VIR_Type_GetVirRegCount(Shader, baseType, VIR_Type_GetArrayLength(baseType));
        case VIR_TY_STRUCT:
            /* get the last field of struct*/
            if (VIR_Type_GetFields(Type) &&
                VIR_IdList_Count(VIR_Type_GetFields(Type)) > 0)
            {
                VIR_SymId fieldSymId = VIR_IdList_GetId(VIR_Type_GetFields(Type),
                    VIR_IdList_Count(VIR_Type_GetFields(Type)) - 1 );
                VIR_Symbol * fieldSym = VIR_Shader_GetSymFromId(Shader, fieldSymId);
                VIR_Type * fieldType = VIR_Symbol_GetType(fieldSym);
                return VIR_Symbol_GetFieldInfo(fieldSym)->tempRegOrUniformOffset + VIR_Type_GetVirRegCount(Shader, fieldType, -1);
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

VIR_Type* VIR_Type_GetRegIndexType(
    IN VIR_Shader * Shader,
    IN VIR_Type* Type,
    IN gctUINT RegIndex
    )
{
    if (VIR_Type_isPrimitive(Type))
    {
        return VIR_Shader_GetTypeFromId(Shader, VIR_GetTypeRowType(VIR_Type_GetIndex(Type)));
    }
    else
    {
        switch (VIR_Type_GetKind(Type))
        {
        case VIR_TY_ARRAY:
            {
                VIR_Type * baseType = VIR_Shader_GetTypeFromId(Shader,
                    VIR_Type_GetBaseTypeId(Type));
                gctUINT baseTypeRegCount = VIR_Type_GetVirRegCount(Shader, baseType, -1);
                return VIR_Type_GetRegIndexType(Shader, baseType, RegIndex % baseTypeRegCount);
            }
        case VIR_TY_STRUCT:
            {
                gctUINT lastRegIndex = 0;
                gctUINT fieldId = 0;

                while(lastRegIndex < RegIndex)
                {
                    VIR_Id fieldSymId = VIR_IdList_GetId(VIR_Type_GetFields(Type), fieldId);
                    VIR_Symbol *fieldSym = VIR_Shader_GetSymFromId(Shader, fieldSymId);
                    VIR_Type *fieldTy = VIR_Symbol_GetType(fieldSym);
                    gctUINT curRegCount = VIR_Type_GetVirRegCount(Shader, fieldTy, -1);

                    if(lastRegIndex + curRegCount >= RegIndex)
                    {
                        return VIR_Type_GetRegIndexType(Shader, fieldTy, RegIndex - lastRegIndex);
                    }
                    else
                    {
                        lastRegIndex += curRegCount;
                        fieldId++;

                        gcmASSERT(fieldId < VIR_IdList_Count(VIR_Type_GetFields(Type)));
                    }
                }
            }
        case VIR_TY_POINTER:
            return Type;
        default:
            return Type;
        }
    }
}

static VIR_Type *
_GetBaseTypeOrFieldSymbol(
    IN  VIR_Shader         *Shader,
    IN  VIR_Symbol         *ParentSym,
    IN  VIR_Type           *Type,
    IN  gctUINT             FieldIndex,
    IN  gctBOOL             IsMemoryOffset,
    IN  gctBOOL             IsLogicalReg,
    OUT VIR_Symbol        **FieldSymbol,
    OUT gctUINT            *FieldOffset,
    OUT gctINT             *ArrayStride,
    OUT gctINT             *MatrixStride,
    OUT VIR_LayoutQual     *LayoutQual
    )
{
    VIR_Symbol             *fieldSymbol = gcvNULL;
    VIR_Type               *type = Type;
    gctINT                  arrayStride = 0, matrixStride = 0;
    gctUINT                 fieldOffset = 0;
    gctBOOL                 updateStructMemberInfo = gcvFALSE;
    VIR_LayoutQual          layoutQual = VIR_LAYQUAL_NONE;
    VIR_TypeKind            typeKind;

    /* We need to get the array stride from VIR_Type,
    ** but we can't get the matrix stride from VIR_Type because we treat matrix as built-in type,
    ** so we can only get it from field info now.
    */

    if(VIR_Type_GetKind(type) == VIR_TY_POINTER)
    {
       type = VIR_Shader_GetTypeFromId(Shader, VIR_Type_GetBaseTypeId(type));
    }

    typeKind = VIR_Type_GetKind(type);
    switch (typeKind)
    {
    case VIR_TY_ARRAY:
        {
            type = VIR_Shader_GetTypeFromId(Shader,
            VIR_Type_GetBaseTypeId(Type));

            arrayStride = VIR_Type_GetArrayStride(Type);
            break;
        }

    case VIR_TY_STRUCT:
        {
            VIR_SymIdList *fields = VIR_Type_GetFields(Type);
            gctUINT i;

            updateStructMemberInfo = gcvTRUE;

            gcmASSERT(FieldIndex < VIR_IdList_Count(fields));

            for (i = 0; i < VIR_IdList_Count(fields); i++)
            {
                VIR_Id      id       = VIR_IdList_GetId(VIR_Type_GetFields(Type), i);
                VIR_Type   *fieldType;

                fieldSymbol = VIR_Shader_GetSymFromId(Shader, id);
                fieldType  = VIR_Symbol_GetType(fieldSymbol);
                layoutQual = VIR_Symbol_GetLayoutQualifier(fieldSymbol);

                if (i != FieldIndex)
                {
                    if (!IsMemoryOffset)
                    {
                        fieldOffset += VIR_Type_GetRegCount(Shader, fieldType, IsLogicalReg);
                    }
                }
                else
                {
                    if (IsMemoryOffset)
                    {
                        fieldOffset = VIR_FieldInfo_GetOffset(VIR_Symbol_GetFieldInfo(fieldSymbol));
                        arrayStride = VIR_Type_GetArrayStride(fieldType);
                        matrixStride = VIR_FieldInfo_GetMatrixStride(VIR_Symbol_GetFieldInfo(fieldSymbol));
                    }
                    type = fieldType;
                    break;
                }
            }

            gcmASSERT(i == FieldIndex);
            break;
        }

    case VIR_TY_MATRIX:
        if(*LayoutQual & VIR_LAYQUAL_ROW_MAJOR)
        {
            type = VIR_Shader_GetTypeFromId(Shader,
                VIR_TypeId_ComposeNonOpaqueType(VIR_GetTypeComponentType(VIR_Type_GetBaseTypeId(Type)), VIR_GetTypeRows(VIR_Type_GetBaseTypeId(Type)), 1));
        }
        else
        {

            type = VIR_Shader_GetTypeFromId(Shader,
                VIR_GetTypeRowType(VIR_Type_GetBaseTypeId(Type)));
        }
        break;

    case VIR_TY_VECTOR:
        type = VIR_Shader_GetTypeFromId(Shader,
            VIR_GetTypeComponentType(VIR_Type_GetBaseTypeId(Type)));
        break;

    default:
        gcmASSERT(0);
        break;
    }

    /* Save the result. */
    if (FieldOffset)
    {
        *FieldOffset = fieldOffset;
    }

    if (FieldSymbol && fieldSymbol)
    {
        *FieldSymbol = fieldSymbol;
    }

    if (ArrayStride)
    {
        *ArrayStride = arrayStride;
    }

    if (updateStructMemberInfo)
    {
        if (MatrixStride)
        {
            *MatrixStride = matrixStride;
        }

        if (LayoutQual)
        {
            *LayoutQual = layoutQual;
        }
    }

    return type;
}

gctUINT
VIR_Type_GetTypeByteSize(
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
                size = VIR_Type_GetArrayLength(Type) * VIR_Type_GetTypeByteSize(Shader, base_type);
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
                        size += VIR_Type_GetTypeByteSize(Shader, field_type);
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

        case VIR_TY_TYPEDEF:
            size = VIR_Type_GetTypeByteSize(Shader, VIR_Shader_GetTypeFromId(Shader, VIR_Type_GetBaseTypeId(Type)));
            break;

        default:
            gcmASSERT(0);
        }
    }

    return size;
}

gctINT
VIR_Type_GetRegOrOpaqueCount(
    IN  VIR_Shader         *Shader,
    IN  VIR_Type           *Type,
    IN  gctBOOL             CheckSampler,
    IN  gctBOOL             CheckImage,
    IN  gctBOOL             CheckAtomicCounter,
    IN  gctBOOL             IsLogicalReg
    )
{
    gctINT                  regCount = 0, baseCount;
    gctBOOL                 isValid = gcvFALSE;
    VIR_TypeId              typeId = VIR_Type_GetIndex(Type);

    /* If this is a primitive type, just check the rows. */
    if (VIR_Type_isPrimitive(Type))
    {
        if (VIR_TypeId_isOpaque(typeId))
        {
            gcmASSERT(VIR_TypeId_isSampler(typeId)  ||
                VIR_TypeId_isImage(typeId)    ||
                VIR_TypeId_isAtomicCounters(typeId));

            if ((CheckSampler && VIR_TypeId_isSampler(typeId))      ||
                (CheckImage && VIR_TypeId_isImage(typeId))          ||
                (CheckAtomicCounter && VIR_TypeId_isAtomicCounters(typeId)))
            {
                isValid = gcvTRUE;
            }
        }
        else if (!CheckSampler && !CheckImage && !CheckAtomicCounter)
        {
            isValid = gcvTRUE;
        }

        if (isValid)
        {
            regCount = IsLogicalReg ? 1 : VIR_GetTypeRows(typeId);
        }
    }
    else if (VIR_Type_isPointer(Type))
    {
        regCount = 1;
    }
    /* If this is an array or struct, check its element. */
    else
    {
        switch (VIR_Type_GetKind(Type))
        {
        case VIR_TY_ARRAY:
            {
                VIR_Type *baseType;

                baseType = VIR_Shader_GetTypeFromId(Shader,
                    VIR_Type_GetBaseTypeId(Type));
                baseCount = VIR_Type_GetRegOrOpaqueCount(Shader,
                    baseType,
                    CheckSampler,
                    CheckImage,
                    CheckAtomicCounter,
                    IsLogicalReg);
                regCount = VIR_Type_GetArrayLength(Type) * baseCount;
                break;
            }

        case VIR_TY_STRUCT:
            {
                VIR_SymIdList *fields = VIR_Type_GetFields(Type);
                gctUINT i;

                for (i = 0; i < VIR_IdList_Count(fields); i++)
                {
                    VIR_Id      id       = VIR_IdList_GetId(VIR_Type_GetFields(Type), i);
                    VIR_Symbol *fieldSym = VIR_Shader_GetSymFromId(Shader, id);
                    VIR_Type   *fieldType  = VIR_Symbol_GetType(fieldSym);

                    baseCount = VIR_Type_GetRegOrOpaqueCount(Shader,
                        fieldType,
                        CheckSampler,
                        CheckImage,
                        CheckAtomicCounter,
                        IsLogicalReg);
                    regCount += baseCount;
                }
                break;
            }

        default:
            gcmASSERT(0);
            break;
        }
    }

    return regCount;
}

/* Only calculate the non-Opaque type. */
gctINT
VIR_Type_GetRegCount(
    IN  VIR_Shader         *Shader,
    IN  VIR_Type           *Type,
    IN  gctBOOL             IsLogicalReg
    )
{
    gctINT                  regCount = 0;

    regCount = VIR_Type_GetRegOrOpaqueCount(Shader,
        Type,
        gcvFALSE,
        gcvFALSE,
        gcvFALSE,
        IsLogicalReg);

    return regCount;
}

gctBOOL
VIR_Type_IsBaseTypeStruct(
    IN  VIR_Shader         *Shader,
    IN  VIR_Type           *Type
    )
{
    if (VIR_Type_isPrimitive(Type))
    {
        return gcvFALSE;
    }
    else
    {
        switch (VIR_Type_GetKind(Type))
        {
        case VIR_TY_ARRAY:
            {
                VIR_Type *baseType;
                baseType = VIR_Shader_GetTypeFromId(Shader,
                    VIR_Type_GetBaseTypeId(Type));
                return VIR_Type_IsBaseTypeStruct(Shader, baseType);
            }

        case VIR_TY_STRUCT:
            return gcvTRUE;

        default:
            return gcvFALSE;
        }
    }
}

/*
** Slice type: array->base type, matrix->vector, vector->scalar.
*/
VIR_TypeId
VIR_Type_SliceType(
    IN  VIR_Shader         *Shader,
    IN  VIR_Type           *Type
    )
{
    VIR_TypeId              typeId = VIR_TYPE_UNKNOWN;
    VIR_TypeKind            typeKind = VIR_Type_GetKind(Type);

    switch (typeKind)
    {
    case VIR_TY_ARRAY:
        typeId = VIR_Type_GetBaseTypeId(Type);
        break;

    case VIR_TY_MATRIX:
        typeId = VIR_GetTypeRowType(VIR_Type_GetIndex(Type));
        break;

    case VIR_TY_VECTOR:
        typeId = VIR_GetTypeComponentType(VIR_Type_GetIndex(Type));
        break;

    default:
        gcmASSERT(gcvFALSE);
        break;
    }

    return typeId;
}

gctUINT VIR_Type_GetIndexingRange(
    VIR_Shader * Shader,
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
    return VIR_Type_GetVirRegCount(Shader, VIR_Symbol_GetType(Sym), -1);
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
            arrayVirRegStride = VIR_Type_GetVirRegCount(Shader, base_type, -1);
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

gctBOOL
VIR_Symbol_IsInArray(
    IN  VIR_Symbol              *Symbol
    )
{
    switch(VIR_Symbol_GetKind(Symbol))
    {
        case VIR_SYM_VIRREG:
        {
            gcmASSERT(VIR_Symbol_GetIndexRange(Symbol) >= 0);

            if((gctUINT) VIR_Symbol_GetIndexRange(Symbol) > VIR_Symbol_GetVregIndex(Symbol) + 1)
            {
                return gcvTRUE;
            }
            else
            {
                VIR_Symbol* underlyingSym = VIR_Symbol_GetVregVariable(Symbol);

                if(underlyingSym)
                {
                    return VIR_Symbol_IsInArray(underlyingSym);
                }
                else
                {
                    return gcvFALSE;
                }
            }
        }
        case VIR_SYM_VARIABLE:
        case VIR_SYM_UNIFORM:
        case VIR_SYM_IMAGE:
        case VIR_SYM_IMAGE_T:
        case VIR_SYM_FIELD:
        {
            VIR_Type* symType = VIR_Symbol_GetType(Symbol);

            if (VIR_Type_GetKind(symType) == VIR_TY_ARRAY ||
                VIR_Type_GetKind(symType) == VIR_TY_MATRIX)
            {
                return gcvTRUE;
            }
            else if(VIR_Symbol_GetKind(Symbol) == VIR_SYM_FIELD)
            {
                return VIR_Symbol_IsInArray(VIR_Id_isFunctionScope(VIR_Symbol_GetParentId(Symbol)) ?
                                            VIR_Function_GetSymFromId(VIR_Symbol_GetParamOrHostFunction(Symbol), VIR_Symbol_GetParentId(Symbol)) :
                                            VIR_Shader_GetSymFromId(VIR_Symbol_GetShader(Symbol), VIR_Symbol_GetParentId(Symbol)));
            }
            else
            {
                gcmASSERT(VIR_Symbol_GetIndexRange(Symbol) >= 0);

                return (gctUINT) VIR_Symbol_GetIndexRange(Symbol)  > (VIR_Symbol_GetVariableVregIndex(Symbol) + 1);
            }
        }
        default:
            return gcvFALSE;
    }
}

gctBOOL
VIR_Symbol_NeedReplaceSymWithReg(
    IN  VIR_Symbol              *Symbol
    )
{
    VIR_SymbolKind               symKind = VIR_Symbol_GetKind(Symbol);

    /* we don't need to replace with vreg for uniform/sampler/image/
    attribute (except:
               1) locaInvcationIndex, since it is changed to computation on localInvocationId.
               2) instance index, since it is changed to "instanceID + offset".
               3) vertex index, since it is changed to "vertexID + offset".
               4) work group index.
               )
    combinedSampler symbol, since it is similar to sampler */
    if (symKind == VIR_SYM_UNIFORM ||
        symKind == VIR_SYM_SAMPLER ||
        symKind == VIR_SYM_SAMPLER_T ||
        symKind == VIR_SYM_IMAGE   ||
        symKind == VIR_SYM_IMAGE_T ||
        VIR_Symbol_isPerPatchInput(Symbol) ||
        (VIR_Symbol_isAttribute(Symbol) &&
        VIR_Symbol_GetName(Symbol) != VIR_NAME_LOCALINVOCATIONINDEX &&
        VIR_Symbol_GetName(Symbol) != VIR_NAME_INSTANCE_INDEX &&
        VIR_Symbol_GetName(Symbol) != VIR_NAME_VERTEX_INDEX &&
        VIR_Symbol_GetName(Symbol) != VIR_NAME_WORK_GROUP_INDEX) ||
        isSymCombinedSampler(Symbol))
    {
        return gcvFALSE;
    }

    return gcvTRUE;
}

gctBOOL VIR_SymAliasTable_IsEmpty(
    IN VIR_SymAliasTable    *Table
    )
{
    VSC_HASH_TABLE* hashTable = VIR_SymAliasTable_GetHashTable(Table);

    return HTBL_GET_ITEM_COUNT(hashTable) == 0;
}

void VIR_SymAliasTable_Insert(
    IN OUT VIR_SymAliasTable    *Table,
    IN VIR_Symbol               *Sym,
    IN VIR_Symbol               *Alias
    )
{
    VSC_HASH_TABLE* hashTable = VIR_SymAliasTable_GetHashTable(Table);
    void* knownAlias = gcvNULL;

    gcmASSERT(Sym && Alias);

    if(vscHTBL_DirectTestAndGet(hashTable, Sym, &knownAlias))
    {
        gcmASSERT(knownAlias);

        if(knownAlias != Alias)
        {
            VIR_SymAliasTable_Insert(Table, (VIR_Symbol*)knownAlias, Alias);
        }
    }
    else
    {
        vscHTBL_DirectSet(hashTable, Sym, Alias);
    }
}

VIR_Symbol* VIR_SymAliasTable_GetAlias(
    IN VIR_SymAliasTable    *Table,
    IN VIR_Symbol           *Sym
    )
{
    VSC_HASH_TABLE* hashTable = VIR_SymAliasTable_GetHashTable(Table);
    void* result = gcvNULL;

    while(vscHTBL_DirectTestAndGet(hashTable, Sym, &result))
    {
        gcmASSERT(result);
        Sym = (VIR_Symbol*)result;
    }

    return (VIR_Symbol*)result;
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

                if (!isSymSkipNameCheck(fieldSym1) &&
                    !isSymSkipNameCheck(fieldSym2) &&
                    !gcmIS_SUCCESS(gcoOS_StrCmp(name1, name2)))
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
    gctSTRING                  name1 = gcvNULL, name2 = gcvNULL;
    VIR_Type*                  type1;
    VIR_Type*                  type2;
    gctBOOL                    sameUniform = gcvTRUE, matched = gcvFALSE;
    VIR_Precision              precision1, precision2;

    do
    {
        name1 = VIR_Shader_GetSymNameString(Shader1, Sym1);
        name2 = VIR_Shader_GetSymNameString(Shader2, Sym2);

        /* If input uniform has SkipNameCheck flag, then check DescriptorSet and binding, instead of checking name string. */
        if (isSymSkipNameCheck(Sym1))
        {
            if ((VIR_Symbol_GetBinding(Sym1) != VIR_Symbol_GetBinding(Sym2)) ||
                (VIR_Symbol_GetDescriptorSet(Sym1) != VIR_Symbol_GetDescriptorSet(Sym2)))
            {
                sameUniform = gcvFALSE;
            }
        }
        else
        {
            if (!gcmIS_SUCCESS(gcoOS_StrCmp(name1, name2)))
            {
                sameUniform = gcvFALSE;
            }

            /* If two uniforms have the same location, they must be the same uniform. */
            if (!sameUniform &&
                VIR_Symbol_GetLocation(Sym1) != -1 &&
                VIR_Symbol_GetLocation(Sym1) == VIR_Symbol_GetLocation(Sym2))
            {
                errCode = VSC_ERR_UNIFORM_TYPE_MISMATCH;
                ON_ERROR(errCode, "Uniform \"%s\" type mismatch.", name1);
            }
        }

        if (!sameUniform)
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
                ON_ERROR(errCode, "Uniform \"%s\" mismatch.", VIR_Shader_GetSymNameString(Shader1, fieldSym1));
            }
        }
        else
        {
            if (CheckPrecision && !VIR_Shader_IsDesktopGL(Shader1))
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

            /* check descriptorSet/binding. */
            if (!isSymSkipNameCheck(Sym1))
            {
                if (VIR_Symbol_GetBinding(Sym1) != VIR_Symbol_GetBinding(Sym2))
                {
                    errCode = VSC_ERR_UNIFORM_TYPE_MISMATCH;
                    ON_ERROR(errCode, "Uniform \"%s\" binding mismatch.", name1);
                }
                if (VIR_Symbol_GetDescriptorSet(Sym1) != VIR_Symbol_GetDescriptorSet(Sym2))
                {
                    errCode = VSC_ERR_UNIFORM_TYPE_MISMATCH;
                    ON_ERROR(errCode, "Uniform \"%s\" descriptorSet mismatch.", name1);
                }
            }

            /* Check the image format and binding. */
            if (VIR_Symbol_GetKind(Sym1) == VIR_SYM_IMAGE)
            {
                if (VIR_Symbol_GetImageFormat(Sym1) != VIR_Symbol_GetImageFormat(Sym2))
                {
                    errCode = VSC_ERR_UNIFORM_TYPE_MISMATCH;
                    ON_ERROR(errCode, "Uniform \"%s\" image format mismatch.", name1);
                }
            }
            /* Check uniform location. */
            if (VIR_Symbol_GetLocation(Sym1) != -1 &&
                VIR_Symbol_GetLocation(Sym2) != -1 &&
                VIR_Symbol_GetLocation(Sym1) != VIR_Symbol_GetLocation(Sym2))
            {
                errCode = VSC_ERR_UNIFORM_TYPE_MISMATCH;
                ON_ERROR(errCode, "Uniform \"%s\" location mismatch.", name1);
            }
            /* Check uniform kind: normal uniform cannot match ubo member
             * GLSL Spec: It is a link-time error if any particular shader interface contains
             *  o two different blocks, each having no instance name, and each having a member
             *    of the same name
             * or
             *  o a variable outside a block, and a block with no instance name, where the
             *    variable has the same name as a member in the block.
             */
            if (VIR_Symbol_GetUniformKind(Sym1) != VIR_Symbol_GetUniformKind(Sym2))
            {
                errCode = VSC_ERR_UNIFORM_TYPE_MISMATCH;
                ON_ERROR(errCode, "Uniform \"%s\" location mismatch.", name1);
            }
            else if (VIR_Symbol_GetUniformKind(Sym1) == VIR_UNIFORM_BLOCK_MEMBER &&
                     VIR_Symbol_GetUniformKind(Sym2) == VIR_UNIFORM_BLOCK_MEMBER )
            {
                /* two different non instanced ubo member having the same name */
                VIR_Uniform * uniform1 = VIR_Symbol_GetUniform(Sym1);
                VIR_Symbol *UBOSym1 = VIR_Shader_GetSymFromId(Shader1,
                                          VIR_IdList_GetId(VIR_Shader_GetUniformBlocks(Shader1),
                                                           VIR_Uniform_GetBlockIndex(uniform1)));
                VIR_Uniform * uniform2 = VIR_Symbol_GetUniform(Sym2);
                VIR_Symbol *UBOSym2 = VIR_Shader_GetSymFromId(Shader2,
                                          VIR_IdList_GetId(VIR_Shader_GetUniformBlocks(Shader2),
                                                           VIR_Uniform_GetBlockIndex(uniform2)));
                gctBOOL hasInstanceName1 = (VIR_IB_GetFlags(UBOSym1->u2.ubo) & VIR_IB_WITH_INSTANCE_NAME) != 0;
                gctBOOL hasInstanceName2 = (VIR_IB_GetFlags(UBOSym2->u2.ubo) & VIR_IB_WITH_INSTANCE_NAME) != 0;
                gctSTRING uboName1 = VIR_Shader_GetSymNameString(Shader1, UBOSym1);
                gctSTRING uboName2 = VIR_Shader_GetSymNameString(Shader2, UBOSym2);
                if(!hasInstanceName1 && !hasInstanceName2 && !gcmIS_SUCCESS(gcoOS_StrCmp(uboName1, uboName2)))
                {
                    errCode = VSC_ERR_UNIFORM_TYPE_MISMATCH;
                    ON_ERROR(errCode, "UBO member \"%s\" appeared in differnt named UBOs which has no instance name.", name1);
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

gctBOOL
VIR_Uniform_AlwaysAlloc(
    IN VIR_Shader* pShader,
    IN VIR_Symbol* pUniformSym
    )
{
    gctBOOL alwaysAlloc = gcvFALSE;

    if (VIR_Symbol_GetUniformKind(pUniformSym) == VIR_UNIFORM_NUM_GROUPS)
    {
        alwaysAlloc = gcvTRUE;
    }

    if (isSymUniformWithResLayout(pUniformSym))
    {
        alwaysAlloc = gcvTRUE;
    }

    return alwaysAlloc;
}

/* UBO-related functions. */
VSC_ErrCode
VIR_UBO_Member_Identical(
    IN VIR_Shader* Shader1,
    IN VIR_Symbol* Sym1,
    IN VIR_Shader* Shader2,
    IN VIR_Symbol* Sym2
    )
{
    VSC_ErrCode                errCode = VSC_ERR_NONE;
    gctBOOL                    matched = gcvFALSE;

    /* If input uniform has SkipNameCheck flag, then check DescriptorSet and binding, instead of checking name string. */
    if (isSymSkipNameCheck(Sym1))
    {
        if ((VIR_Symbol_GetBinding(Sym1) == VIR_Symbol_GetBinding(Sym2)) &&
            (VIR_Symbol_GetDescriptorSet(Sym1) == VIR_Symbol_GetDescriptorSet(Sym2)))
        {
            matched = gcvTRUE;
        }
    }
    else
    {
        matched = VIR_Symbol_isNameMatch(Shader1,
            Sym1,
            Shader2,
            Sym2);
    }

    if (matched == gcvFALSE)
    {
        errCode = VSC_ERR_NAME_MISMATCH;
    }

    if (errCode != VSC_ERR_NONE)
    {
        return errCode;
    }

    if (VIR_Symbol_GetLayoutQualifier(Sym1) != VIR_Symbol_GetLayoutQualifier(Sym2))
    {
        return VSC_ERR_UNIFORM_TYPE_MISMATCH;
    }

    return errCode;
}

VSC_ErrCode
VIR_UBO_Identical(
    IN VIR_Shader* Shader1,
    IN VIR_Symbol* Sym1,
    IN VIR_Shader* Shader2,
    IN VIR_Symbol* Sym2,
    OUT gctBOOL*   Matched
    )
{
    VSC_ErrCode                errCode = VSC_ERR_NONE;
    gctSTRING                  name1 = gcvNULL, name2 = gcvNULL;
    gctBOOL                    hasInstanceName1, hasInstanceName2;
    VIR_Type*                  type1;
    VIR_Type*                  type2;
    gctBOOL                    matched = gcvFALSE;
    VIR_Symbol*                addrSym1;
    VIR_Symbol*                addrSym2;

    do
    {
        name1 = VIR_Shader_GetSymNameString(Shader1, Sym1);
        name2 = VIR_Shader_GetSymNameString(Shader2, Sym2);

        /* If input UBO has SkipNameCheck flag, then check DescriptorSet and binding, instead of checking name string. */
        if (isSymSkipNameCheck(Sym1))
        {
            if ((VIR_Symbol_GetBinding(Sym1) != VIR_Symbol_GetBinding(Sym2)) ||
                (VIR_Symbol_GetDescriptorSet(Sym1) != VIR_Symbol_GetDescriptorSet(Sym2)))
            {
                break;
            }
        }
        else
        {
            /* If two ubos have different name, skip these. */
            if (!gcmIS_SUCCESS(gcoOS_StrCmp(name1, name2)))
            {
                break;
            }
        }

        /* name matched, start to check the other info. */
        if (VIR_Symbol_GetKind(Sym1) != VIR_Symbol_GetKind(Sym2))
        {
            errCode = VSC_ERR_UNIFORM_TYPE_MISMATCH;
            ON_ERROR(errCode, "UBO \"%s\" layout mismatch.", name1);
        }

        hasInstanceName1 = (VIR_IB_GetFlags(Sym1->u2.ubo) & VIR_IB_WITH_INSTANCE_NAME) != 0;
        hasInstanceName2 = (VIR_IB_GetFlags(Sym2->u2.ubo) & VIR_IB_WITH_INSTANCE_NAME) != 0;
        if(hasInstanceName1 != hasInstanceName2)
        {
            errCode = VSC_ERR_UNIFORM_TYPE_MISMATCH;
            ON_ERROR(errCode, "UBO \"%s\" instance name mismatch.", name1);
        }

        /* check base addr uniform. */
        matched = gcvFALSE;
        addrSym1 = VIR_Shader_GetSymFromId(Shader1, VIR_Symbol_GetUBO(Sym1)->baseAddr);
        addrSym2 = VIR_Shader_GetSymFromId(Shader2, VIR_Symbol_GetUBO(Sym2)->baseAddr);

        errCode = VIR_Uniform_Identical(Shader1,
            addrSym1,
            Shader2,
            addrSym2,
            gcvFALSE,
            &matched);
        ON_ERROR(errCode, "UBO \"%s\" type mismatch.", name1);

        /* check memory layout. */
        if (VIR_Symbol_GetLayoutQualifier(Sym1) != VIR_Symbol_GetLayoutQualifier(Sym2))
        {
            errCode = VSC_ERR_UNIFORM_TYPE_MISMATCH;
            ON_ERROR(errCode, "UBO \"%s\" layout mismatch.", name1);
        }

        /* check descriptorSet/binding. */
        if (!isSymSkipNameCheck(Sym1))
        {
            if (VIR_Symbol_GetBinding(Sym1) != VIR_Symbol_GetBinding(Sym2))
            {
                errCode = VSC_ERR_UNIFORM_TYPE_MISMATCH;
                ON_ERROR(errCode, "UBO \"%s\" binding mismatch.", name1);
            }
            if (VIR_Symbol_GetDescriptorSet(Sym1) != VIR_Symbol_GetDescriptorSet(Sym2))
            {
                errCode = VSC_ERR_UNIFORM_TYPE_MISMATCH;
                ON_ERROR(errCode, "UBO \"%s\" descriptorSet mismatch.", name1);
            }
        }

        matched = gcvTRUE;
        type1 = VIR_Symbol_GetType(Sym1);
        type2 = VIR_Symbol_GetType(Sym2);

        /* check block members. */
        if (Matched && !*Matched)
        {
            /* check name first */
            VIR_SymIdList * fields1,
                * fields2;
            gctUINT i;
            if (!isSymSkipNameCheck(Sym1) &&
                !gcmIS_SUCCESS(gcoOS_StrCmp(VIR_Shader_GetTypeNameString(Shader1, type1),
                                            VIR_Shader_GetTypeNameString(Shader2, type2))))
            {
                errCode = VSC_ERR_UNIFORM_TYPE_MISMATCH;
                ON_ERROR(errCode, "UBO \"%s\" mismatch.", name1);
            }

            /* check fields: name, type, offset*/
            fields1 = VIR_Type_GetFields(type1);
            fields2 = VIR_Type_GetFields(type2);
            if (VIR_IdList_Count(fields1) != VIR_IdList_Count(fields2))
            {
                errCode = VSC_ERR_UNIFORM_TYPE_MISMATCH;
                ON_ERROR(errCode, "UBO \"%s\" mismatch.", name1);
            }
            for (i = 0; i < VIR_IdList_Count(fields1); i++)
            {
                VIR_Id      id1       = VIR_IdList_GetId(VIR_Type_GetFields(type1), i);
                VIR_Symbol *fieldSym1 = VIR_Shader_GetSymFromId(Shader1, id1);
                VIR_Id      id2       = VIR_IdList_GetId(VIR_Type_GetFields(type2), i);
                VIR_Symbol *fieldSym2 = VIR_Shader_GetSymFromId(Shader2, id2);

                errCode = VIR_UBO_Member_Identical(Shader1,
                    fieldSym1,
                    Shader2,
                    fieldSym2);
                ON_ERROR(errCode, "UBO \"%s\" mismatch.", name1);
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

/* InterfaceBlock-related functions. */
static gctUINT
_AlignOffset(
    IN  gctUINT             Offset,
    IN  gctBOOL             IsPacked,
    IN  gctUINT             Alignment
    )
{
    if (IsPacked)
    {
        return Offset;
    }
    else
    {
        return gcmALIGN(Offset, Alignment);
    }
}

VSC_ErrCode
VIR_Type_CalcByteOffset(
    IN  VIR_Shader         *Shader,
    IN  VIR_Type           *BaseType,
    IN  gctBOOL             IsArray,
    IN  VIR_LayoutQual      LayoutQual,
    IN  gctUINT             BaseOffset,
    IN  gctUINT            *Offset,
    IN  gctINT             *ArrayStride,
    IN  gctINT             *MatrixStride,
    IN  gctINT             *Alignment
    )
{
    VSC_ErrCode             errCode   = VSC_ERR_NONE;
    VIR_TypeId              baseTypeId = VIR_Type_GetBaseTypeId(BaseType);
    gctINT                  componentCount = VIR_GetTypeComponents(baseTypeId);
    gctINT                  rowCount = VIR_GetTypeRows(baseTypeId);
    gctUINT                 offset = 0;
    gctINT                  arrayStride = -1, matrixStride = -1, alignment = -1;
    gctBOOL                 isSTD140 = LayoutQual & VIR_LAYQUAL_STD140;
    gctBOOL                 isRowMajor = LayoutQual & VIR_LAYQUAL_ROW_MAJOR;
    gctBOOL                 isPacked = LayoutQual & VIR_LAYQUAL_PACKED;

    /* Check matrix. */
    if (VIR_Type_isMatrix(BaseType))
    {
        switch (VIR_GetTypeType(baseTypeId))
        {
        /* FP32 matrix. */
        case VIR_TYPE_FLOAT_2X2:
            if (isSTD140)
            {
                alignment = 16;
            }
            else
            {
                alignment = 8;
            }
            break;

        case VIR_TYPE_FLOAT_2X3:
            if (isSTD140)
            {
                alignment = 16;
            }
            else if (isPacked)
            {
                if (isRowMajor)
                {
                    alignment = 8;
                }
                else
                {
                    alignment = 12;
                }
            }
            else
            {
                alignment = 16;
            }
            break;

        case VIR_TYPE_FLOAT_2X4:
            if (isSTD140)
            {
                alignment = 16;
            }
            else if (isPacked)
            {
                if (isRowMajor)
                {
                    alignment = 8;
                }
                else
                {
                    alignment = 16;
                }
            }
            else
            {
                alignment = 16;
            }
            break;

        case VIR_TYPE_FLOAT_3X2:
            if (isSTD140)
            {
                alignment = 16;
            }
            else if (isPacked)
            {
                if (isRowMajor)
                {
                    alignment = 12;
                }
                else
                {
                    alignment = 8;
                }
            }
            else
            {
                alignment = 8;
            }
            break;

        case VIR_TYPE_FLOAT_3X3:
            if (isSTD140)
            {
                alignment = 16;
            }
            else if (isPacked)
            {
                if (isRowMajor)
                {
                    alignment = 12;
                }
                else
                {
                    alignment = 12;
                }
            }
            else
            {
                alignment = 16;
            }
            break;

        case VIR_TYPE_FLOAT_3X4:
            if (isSTD140)
            {
                alignment = 16;
            }
            else if (isPacked)
            {
                if (isRowMajor)
                {
                    alignment = 12;
                }
                else
                {
                    alignment = 16;
                }
            }
            else
            {
                alignment = 16;
            }
            break;

        case VIR_TYPE_FLOAT_4X2:
            if (isSTD140)
            {
                alignment = 16;
            }
            else if (isPacked)
            {
                if (isRowMajor)
                {
                    alignment = 16;
                }
                else
                {
                    alignment = 8;
                }
            }
            else
            {
                alignment = 8;
            }
            break;

        case VIR_TYPE_FLOAT_4X3:
            if (isSTD140)
            {
                alignment = 16;
            }
            else if (isPacked)
            {
                if (isRowMajor)
                {
                    alignment = 16;
                }
                else
                {
                    alignment = 12;
                }
            }
            else
            {
                alignment = 16;
            }
            break;

        case VIR_TYPE_FLOAT_4X4:
            if (isSTD140)
            {
                alignment = 16;
            }
            else if (isPacked)
            {
                if (isRowMajor)
                {
                    alignment = 16;
                }
                else
                {
                    alignment = 16;
                }
            }
            else
            {
                alignment = 16;
            }
            break;

        /* FP16 matrix. */
        case VIR_TYPE_FLOAT16_2X2:
            alignment = 4;
            break;

        case VIR_TYPE_FLOAT16_2X3:
            alignment = 8;
            break;

        case VIR_TYPE_FLOAT16_2X4:
            alignment = 8;
            break;

        case VIR_TYPE_FLOAT16_3X2:
            alignment = 4;
            break;

        case VIR_TYPE_FLOAT16_3X3:
            alignment = 8;
            break;

        case VIR_TYPE_FLOAT16_3X4:
            alignment = 8;
            break;

        case VIR_TYPE_FLOAT16_4X2:
            alignment = 4;
            break;

        case VIR_TYPE_FLOAT16_4X3:
            alignment = 8;
            break;

        case VIR_TYPE_FLOAT16_4X4:
            alignment = 8;
            break;

        default:
            gcmASSERT(gcvFALSE);
            break;
        }

        if (isRowMajor)
        {
            arrayStride = componentCount * alignment;
        }
        else
        {
            arrayStride = rowCount * alignment;
        }
        matrixStride = alignment;
    }
    /* Check other types. */
    else
    {
        switch (VIR_GetTypeComponents(baseTypeId))
        {
        case 1:
            if (IsArray && isSTD140)
            {
                alignment = 16;
                arrayStride = 16;
            }
            else
            {
                alignment = VIR_GetTypeAlignment(baseTypeId);
                arrayStride = VIR_GetTypeSize(baseTypeId);
            }
            break;

        case 2:
            if (IsArray && isSTD140)
            {
                alignment = 16;
                arrayStride = 16;
            }
            else
            {
                alignment = VIR_GetTypeAlignment(baseTypeId);
                arrayStride = VIR_GetTypeSize(baseTypeId);
            }
            break;

        case 3:
            if (IsArray && isSTD140)
            {
                alignment = 16;
                arrayStride = 16;
            }
            else
            {
                alignment = VIR_GetTypeAlignment(baseTypeId);
                arrayStride = VIR_GetTypeSize(baseTypeId);
            }
            break;

        case 4:
            alignment = VIR_GetTypeAlignment(baseTypeId);
            arrayStride = VIR_GetTypeSize(baseTypeId);
            break;

        default:
            gcmASSERT(gcvFALSE);
            break;
        }
    }

    offset = _AlignOffset(BaseOffset, isPacked, alignment);

    /* Save the result. */
    if (Offset)
    {
        *Offset = offset;
    }
    if (ArrayStride)
    {
        *ArrayStride = arrayStride;
    }
    if (MatrixStride)
    {
        *MatrixStride = matrixStride;
    }
    if (Alignment)
    {
        *Alignment = alignment;
    }

    return errCode;
}

static VIR_LayoutQual
_MergeLayout(
    IN  VIR_LayoutQual      ParentLayoutQual,
    IN  VIR_LayoutQual      ChildLayoutQual
    )
{
    VIR_LayoutQual layoutQual = ParentLayoutQual;

    if (ChildLayoutQual & VIR_LAYQUAL_ROW_MAJOR)
    {
        layoutQual &= ~VIR_LAYQUAL_COLUMN_MAJOR;
        layoutQual |= VIR_LAYQUAL_ROW_MAJOR;
    }
    else if (ChildLayoutQual & VIR_LAYQUAL_COLUMN_MAJOR)
    {
        layoutQual &= ~VIR_LAYQUAL_ROW_MAJOR;
        layoutQual |= VIR_LAYQUAL_COLUMN_MAJOR;
    }

    return layoutQual;
}

static VSC_ErrCode
_CalcOffsetForNonStructField(
    IN  VIR_Shader         *Shader,
    IN  VIR_Symbol         *Symbol,
    IN  VIR_LayoutQual      ParentLayoutQual,
    IN  gctUINT            *BaseOffset,
    IN  gctBOOL             UpdateTypeOffset
    )
{
    VSC_ErrCode             errCode   = VSC_ERR_NONE;
    VIR_LayoutQual          mergeLayoutQual = _MergeLayout(ParentLayoutQual, VIR_Symbol_GetLayoutQualifier(Symbol));
    VIR_Type               *symType = VIR_Symbol_GetType(Symbol);
    VIR_Type               *baseType = symType;
    VIR_FieldInfo          *fieldInfo = VIR_Symbol_GetFieldInfo(Symbol);
    gctUINT                 offset = 0, topArrayLength = 1, totalArrayLength = 1;
    gctINT                  arrayStride = -1, matrixStride = -1, alignment = 0;

    /* Get the array length. */
    while (VIR_Type_isArray(baseType))
    {
        if (baseType == symType)
        {
            topArrayLength = VIR_Type_GetArrayLength(baseType);;
        }
        totalArrayLength *= VIR_Type_GetArrayLength(baseType);

        baseType = VIR_Shader_GetTypeFromId(Shader, VIR_Type_GetBaseTypeId(baseType));
    }

    /* Calc the offset and stride. */
    errCode = VIR_Type_CalcByteOffset(Shader,
                                      baseType,
                                      VIR_Type_isArray(symType),
                                      mergeLayoutQual,
                                      *BaseOffset,
                                      &offset,
                                      &arrayStride,
                                      &matrixStride,
                                      &alignment);
    CHECK_ERROR(errCode, "Calculate offset for non-struct field failed.");

    /* The calculated result should be same as the decoration. */
    if (VIR_FieldInfo_GetOffset(fieldInfo) != -1 &&
        VIR_FieldInfo_GetOffset(fieldInfo) != offset)
    {
        WARNING_REPORT(VSC_ERR_INVALID_DATA, "Field offset mismatch.");
        offset = VIR_FieldInfo_GetOffset(fieldInfo);
    }
    if (VIR_FieldInfo_GetArrayStride(fieldInfo) != -1 &&
        VIR_FieldInfo_GetArrayStride(fieldInfo) != (arrayStride * (gctINT)(totalArrayLength / topArrayLength)))
    {
        WARNING_REPORT(VSC_ERR_INVALID_DATA, "Field array stride mismatch.");
        arrayStride = VIR_FieldInfo_GetArrayStride(fieldInfo) / (gctINT)(totalArrayLength / topArrayLength);
    }
    if (VIR_FieldInfo_GetMatrixStride(fieldInfo) != -1 &&
        VIR_FieldInfo_GetMatrixStride(fieldInfo) != matrixStride)
    {
        WARNING_REPORT(VSC_ERR_INVALID_DATA, "Field matrix stride mismatch.");
        matrixStride = VIR_FieldInfo_GetMatrixStride(fieldInfo);
    }
    if (VIR_FieldInfo_GetAlignment(fieldInfo) != -1 &&
        VIR_FieldInfo_GetAlignment(fieldInfo) != alignment)
    {
        WARNING_REPORT(VSC_ERR_INVALID_DATA, "Field alignment mismatch.");
        alignment = VIR_FieldInfo_GetAlignment(fieldInfo);
    }

    /* Update the BaseOffset. */
    *BaseOffset = offset;
    *BaseOffset += totalArrayLength * arrayStride;

    /* Save the result. */
    if (UpdateTypeOffset)
    {
        VIR_FieldInfo_SetOffset(fieldInfo, offset);
        VIR_FieldInfo_SetArrayStride(fieldInfo, arrayStride);
        VIR_FieldInfo_SetMatrixStride(fieldInfo, matrixStride);
        VIR_FieldInfo_SetAlignment(fieldInfo, alignment);
    }

    return errCode;
}

VSC_ErrCode
_CalcBaseAlignmentForStruct(
    IN  VIR_Shader         *Shader,
    IN  VIR_Symbol         *Symbol,
    IN  VIR_LayoutQual      ParentLayoutQual,
    IN  VIR_Type           *BaseType,
    IN  gctINT             *Alignment
    )
{
    VSC_ErrCode             errCode   = VSC_ERR_NONE;
    VIR_SymIdList          *fields = VIR_Type_GetFields(BaseType);
    VIR_Id                  fieldSymId;
    VIR_Symbol             *fieldSym;
    VIR_Type               *fieldType;
    VIR_LayoutQual          fieldLayoutQual;
    VIR_Type               *baseFieldType;
    gctUINT                 i;
    gctINT                  alignment = 0, fieldAlignment = 0;

    for (i = 0; i < VIR_IdList_Count(fields); i++)
    {
        fieldSymId = VIR_IdList_GetId(fields, i);
        fieldSym = VIR_Shader_GetSymFromId(Shader, fieldSymId);
        fieldType = VIR_Symbol_GetType(fieldSym);
        fieldLayoutQual = VIR_Symbol_GetLayoutQualifier(fieldSym);
        baseFieldType = fieldType;

        /* Use the non-array struct type to calc. */
        while (VIR_Type_isArray(baseFieldType))
        {
            baseFieldType = VIR_Shader_GetTypeFromId(Shader, VIR_Type_GetBaseTypeId(baseFieldType));
        }

        if (VIR_Type_isStruct(baseFieldType))
        {
            errCode = _CalcBaseAlignmentForStruct(Shader,
                fieldSym,
                _MergeLayout(ParentLayoutQual, fieldLayoutQual),
                baseFieldType,
                &fieldAlignment);
            CHECK_ERROR(errCode, "_CalcBaseAlignmentForStruct failed.");
        }
        else
        {
            errCode = VIR_Type_CalcByteOffset(Shader,
                baseFieldType,
                VIR_Type_isArray(fieldType),
                _MergeLayout(ParentLayoutQual, fieldLayoutQual),
                0,
                gcvNULL,
                gcvNULL,
                gcvNULL,
                &fieldAlignment);
            CHECK_ERROR(errCode, "VIR_Type_CalcByteOffset failed.");
        }

        if (fieldAlignment > alignment)
        {
            alignment = fieldAlignment;
        }
    }

    if (Alignment)
    {
        *Alignment = alignment;
    }

    return errCode;
}

VSC_ErrCode
_CalcOffsetForStructField(
    IN  VIR_Shader         *Shader,
    IN  VIR_Symbol         *Symbol,
    IN  VIR_LayoutQual      ParentLayoutQual,
    IN  VIR_Type           *BaseType,
    IN  gctUINT            *BaseOffset,
    IN  gctBOOL             UpdateTypeOffset
    )
{
    VSC_ErrCode             errCode   = VSC_ERR_NONE;
    VIR_FieldInfo          *symbolFieldInfo = VIR_Symbol_GetFieldInfo(Symbol);
    VIR_LayoutQual          mergeLayoutQual = _MergeLayout(ParentLayoutQual, VIR_Symbol_GetLayoutQualifier(Symbol));
    VIR_Type               *symType = VIR_Symbol_GetType(Symbol);
    VIR_SymIdList          *fields = VIR_Type_GetFields(BaseType);
    VIR_Id                  fieldSymId;
    VIR_Symbol             *fieldSym;
    VIR_Type               *fieldType;
    VIR_Type               *baseFieldType;
    gctUINT                 i, totalArrayLength = 1, topArrayLength = 1;
    gctUINT                 offset = 0, structOffset = 0, structSize;
    gctINT                  arrayStride = -1, alignment = 0;

    /* Calc offset and stride for all field members. */
    for (i = 0; i < VIR_IdList_Count(fields); i++)
    {
        fieldSymId = VIR_IdList_GetId(fields, i);
        fieldSym = VIR_Shader_GetSymFromId(Shader, fieldSymId);
        fieldType = VIR_Symbol_GetType(fieldSym);
        baseFieldType = fieldType;

        /* Use the non-array struct type to calc and check if it is an arrays of arrays. */
        while (VIR_Type_isArray(baseFieldType))
        {
            baseFieldType = VIR_Shader_GetTypeFromId(Shader, VIR_Type_GetBaseTypeId(baseFieldType));
        }

        if (VIR_Type_isStruct(baseFieldType))
        {
            errCode = _CalcOffsetForStructField(Shader,
                                                fieldSym,
                                                mergeLayoutQual,
                                                baseFieldType,
                                                &structOffset,
                                                UpdateTypeOffset);
            CHECK_ERROR(errCode, "Calculate offset for struct field failed.");
        }
        else
        {
            errCode = _CalcOffsetForNonStructField(Shader,
                                                   fieldSym,
                                                   mergeLayoutQual,
                                                   &structOffset,
                                                   UpdateTypeOffset);
            CHECK_ERROR(errCode, "Calculate offset for non-struct field failed.");
        }
    }

    /* According to GLSL ES specs:
    ** If the member is a structure, the base alignment of the structure is N, where
    ** N is the largest base alignment value of any of its members, and rounded
    ** up to the base alignment of a vec4.
    */
    errCode = _CalcBaseAlignmentForStruct(Shader,
                                          Symbol,
                                          mergeLayoutQual,
                                          BaseType,
                                          &alignment);
    CHECK_ERROR(errCode, "_CalcBaseAlignmentForStruct failed.");

    if (mergeLayoutQual & VIR_LAYQUAL_STD140)
    {
        alignment = (alignment > 16) ? alignment : 16;
    }

    /* The structOffset is the size of this struct field. */
    structSize = _AlignOffset(structOffset, mergeLayoutQual & VIR_LAYQUAL_PACKED, alignment);

    /* Calc the offset for this struct field. */
    offset = _AlignOffset(*BaseOffset, mergeLayoutQual & VIR_LAYQUAL_PACKED, alignment);

    /* Update the BaseOffset and the array stride. */
    *BaseOffset = offset;
    while (VIR_Type_isArray(symType))
    {
        if (symType == VIR_Symbol_GetType(Symbol))
        {
            topArrayLength = VIR_Type_GetArrayLength(symType);
        }

        totalArrayLength *= VIR_Type_GetArrayLength(symType);
        symType = VIR_Shader_GetTypeFromId(Shader, VIR_Type_GetBaseTypeId(symType));
    }
    *BaseOffset += totalArrayLength * structSize;
    arrayStride = totalArrayLength * structSize / topArrayLength;

    /* The calculated result should be same as the decoration. */
    if (VIR_FieldInfo_GetOffset(symbolFieldInfo) != -1 &&
        VIR_FieldInfo_GetOffset(symbolFieldInfo) != offset)
    {
        WARNING_REPORT(VSC_ERR_INVALID_DATA, "Field offset mismatch.");
        offset = VIR_FieldInfo_GetOffset(symbolFieldInfo);
    }
    if (VIR_FieldInfo_GetArrayStride(symbolFieldInfo) != -1 &&
        VIR_FieldInfo_GetArrayStride(symbolFieldInfo) != arrayStride)
    {
        WARNING_REPORT(VSC_ERR_INVALID_DATA, "Field array stride mismatch.");
        arrayStride = VIR_FieldInfo_GetArrayStride(symbolFieldInfo);
    }
    if (VIR_FieldInfo_GetAlignment(symbolFieldInfo) != -1 &&
        VIR_FieldInfo_GetAlignment(symbolFieldInfo) != alignment)
    {
        WARNING_REPORT(VSC_ERR_INVALID_DATA, "Field alignment mismatch.");
        alignment = VIR_FieldInfo_GetAlignment(symbolFieldInfo);
    }

    /* Save the result. */
    if (UpdateTypeOffset)
    {
        VIR_FieldInfo_SetOffset(symbolFieldInfo, offset);
        VIR_FieldInfo_SetArrayStride(symbolFieldInfo, arrayStride);
        VIR_FieldInfo_SetAlignment(symbolFieldInfo, alignment);
    }

    return errCode;
}

/* Calculate the size for a interface block and arrayStride/matrixStride/offset for its members. */
VSC_ErrCode
VIR_InterfaceBlock_CalcDataByteSize(
    IN  VIR_Shader         *Shader,
    IN  VIR_Symbol         *Symbol,
    IN  gctBOOL            UpdateTypeOffset
    )
{
    VSC_ErrCode             errCode   = VSC_ERR_NONE;
    VIR_Type               *type = VIR_Symbol_GetType(Symbol);
    VIR_Type               *baseBlockType = type;
    VIR_LayoutQual          layoutQual = VIR_Symbol_GetLayoutQualifier(Symbol);
    VIR_SymIdList          *fields;
    VIR_Id                  fieldSymId;
    VIR_Symbol             *fieldSym;
    VIR_Type               *fieldType;
    VIR_Type               *baseFieldType;
    VIR_UniformBlock       *ubo;
    VIR_StorageBlock       *sbo;
    gctUINT                 i, offset = 0;

    /* Use the non-array struct type to calc. */
    while (VIR_Type_isArray(baseBlockType))
    {
        baseBlockType = VIR_Shader_GetTypeFromId(Shader, VIR_Type_GetBaseTypeId(baseBlockType));
    }

    gcmASSERT(VIR_Type_GetKind(baseBlockType) == VIR_TY_STRUCT);
    fields = VIR_Type_GetFields(baseBlockType);

    for (i = 0; i < VIR_IdList_Count(fields); i++)
    {
        fieldSymId = VIR_IdList_GetId(fields, i);
        fieldSym = VIR_Shader_GetSymFromId(Shader, fieldSymId);
        fieldType = VIR_Symbol_GetType(fieldSym);
        baseFieldType = fieldType;

        /* Use the non-array struct type to calc and check if it is an arrays of arrays. */
        while (VIR_Type_isArray(baseFieldType))
        {
            baseFieldType = VIR_Shader_GetTypeFromId(Shader, VIR_Type_GetBaseTypeId(baseFieldType));
        }

        if (VIR_Type_isStruct(baseFieldType))
        {
            errCode = _CalcOffsetForStructField(Shader,
                                                fieldSym,
                                                layoutQual,
                                                baseFieldType,
                                                &offset,
                                                UpdateTypeOffset);
            CHECK_ERROR(errCode, "Calculate offset for struct field failed.");
        }
        else
        {
            errCode = _CalcOffsetForNonStructField(Shader,
                                                   fieldSym,
                                                   layoutQual,
                                                   &offset,
                                                   UpdateTypeOffset);
            CHECK_ERROR(errCode, "Calculate offset for non-struct field failed.");
        }
    }

    if (VIR_Symbol_isUBO(Symbol))
    {
        ubo = VIR_Symbol_GetUBO(Symbol);
        VIR_UBO_SetBlockSize(ubo, offset);
    }

    if (VIR_Symbol_isSBO(Symbol))
    {
        sbo = VIR_Symbol_GetSBO(Symbol);
        VIR_SBO_SetBlockSize(sbo, offset);
    }

    return errCode;
}

/* TypeId-related functions. */
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
    if (VIR_TypeId_isPacked(TypeId))
    {
       switch(VIR_GetTypeSize(TypeId))
       {
        case 0:
            return VIR_ENABLE_NONE;
        case 1:
        case 2:
        case 3:
        case 4:
            return VIR_ENABLE_X;
        case 6:
        case 8:
            return VIR_ENABLE_XY;
        case 16:
            return VIR_ENABLE_XYZW;

        default:
            return VIR_ENABLE_XYZW;
        }
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
    default:
        return VIR_ENABLE_XYZW;
    }
}

VIR_Enable
VIR_TypeId_GetImplicitEnableForPackType(
    IN VIR_TypeId       TypeId
    )
{
    gcmASSERT(VIR_TypeId_isPrimitive(TypeId));
    gcmASSERT(VIR_TypeId_isPacked(TypeId));

    switch(VIR_GetTypePackedComponents(TypeId))
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
    default:
        return VIR_ENABLE_XYZW;
    }
}

VIR_Swizzle
VIR_TypeId_Conv2Swizzle(
    IN VIR_TypeId       TypeId
    )
{
    gcmASSERT(VIR_TypeId_isPrimitive(TypeId));
    if (VIR_TypeId_isSamplerOrImage(TypeId))
    {
        return VIR_SWIZZLE_XYZW;
    }

    if (VIR_TypeId_isPacked(TypeId))
    {
       switch(VIR_GetTypeSize(TypeId))
       {
        case 0:
        case 1:
        case 2:
        case 3:
        case 4:
            return VIR_SWIZZLE_XXXX;
        case 8:
            return VIR_SWIZZLE_XYYY;
        case 16:
        default:
            return VIR_SWIZZLE_XYZW;
        }
    }

    switch(VIR_GetTypeComponents(TypeId))
    {
    case 0:
        return VIR_SWIZZLE_XXXX;
    case 1:
        return VIR_SWIZZLE_XXXX;
    case 2:
        return VIR_SWIZZLE_XYYY;
    case 3:
        return VIR_SWIZZLE_XYZZ;
    case 4:
    default:
        return VIR_SWIZZLE_XYZW;
    }
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
            case 32:
                return VIR_TYPE_FLOAT_X32;
            default:
                gcmASSERT(0);
                break;
            }
            break;

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
            case 32:
                return VIR_TYPE_FLOAT16_X32;
            default:
                gcmASSERT(0);
                break;
            }
            break;

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
            case 32:
                return VIR_TYPE_INTEGER_X32;
            default:
                gcmASSERT(0);
                break;
            }
            break;

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
            case 32:
                return VIR_TYPE_INT16_X32;
            default:
                gcmASSERT(0);
                break;
            }
            break;

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
            case 32:
                return VIR_TYPE_INT8_X32;
            default:
                gcmASSERT(0);
                break;
            }
            break;

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
            case 32:
                return VIR_TYPE_UINT_X32;
            default:
                gcmASSERT(0);
                break;
            }
            break;

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
            case 32:
                return VIR_TYPE_UINT16_X32;
            default:
                gcmASSERT(0);
                break;
            }
            break;

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
            case 32:
                return VIR_TYPE_UINT8_X32;
            default:
                gcmASSERT(0);
                break;
            }
            break;

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
            case 32:
                return VIR_TYPE_INT64_X32;
            default:
                gcmASSERT(0);
                break;
            }
            break;

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
            case 32:
                return VIR_TYPE_UINT64_X32;
            default:
                gcmASSERT(0);
                break;
            }
            break;

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
            case 32:
                return VIR_TYPE_BOOLEAN_X32;
            default:
                gcmASSERT(0);
                break;
            }
            break;

        case VIR_TYPE_SNORM8:
            switch (CompCount)
            {
            case 1: return VIR_TYPE_SNORM8;

            default:
                return VIR_TYPE_SNORM8;
            }
            break;

        case VIR_TYPE_UNORM8:
            switch (CompCount)
            {
            case 1: return VIR_TYPE_UNORM8;

            default:
                return VIR_TYPE_UNORM8;
            }
            break;

        default:
            gcmASSERT(0);
            break;
        }
    }

    return VIR_TYPE_UNKNOWN;
}

VIR_TypeId
VIR_TypeId_ComposePackedNonOpaqueType(
    IN VIR_TypeId       ComponentType,
    IN gctUINT          CompCount
    )
{
    switch(ComponentType)
    {
    case VIR_TYPE_FLOAT32:
    case VIR_TYPE_INT32:
    case VIR_TYPE_UINT32:
    case VIR_TYPE_INT64:
    case VIR_TYPE_UINT64:
        return VIR_TypeId_ComposeNonOpaqueType(ComponentType, CompCount, 1);
    case VIR_TYPE_FLOAT16:
        switch(CompCount)
        {
        case 1:
            return VIR_TYPE_FLOAT16;
        case 2:
            return VIR_TYPE_FLOAT16_P2;
        case 3:
            return VIR_TYPE_FLOAT16_P3;
        case 4:
            return VIR_TYPE_FLOAT16_P4;
        case 8:
            return VIR_TYPE_FLOAT16_P8;
        case 16:
            return VIR_TYPE_FLOAT16_P16;
        case 32:
            return VIR_TYPE_FLOAT16_P32;
        default:
            gcmASSERT(0);
        }
    case VIR_TYPE_INT16:
        switch(CompCount)
        {
        case 1:
            return VIR_TYPE_INT16;
        case 2:
            return VIR_TYPE_INT16_P2;
        case 3:
            return VIR_TYPE_INT16_P3;
        case 4:
            return VIR_TYPE_INT16_P4;
        case 8:
            return VIR_TYPE_INT16_P8;
        case 16:
            return VIR_TYPE_INT16_P16;
        case 32:
            return VIR_TYPE_INT16_P32;
        default:
            gcmASSERT(0);
        }
    case VIR_TYPE_INT8:
        switch(CompCount)
        {
        case 1:
            return VIR_TYPE_INT8;
        case 2:
            return VIR_TYPE_INT8_P2;
        case 3:
            return VIR_TYPE_INT8_P3;
        case 4:
            return VIR_TYPE_INT8_P4;
        case 8:
            return VIR_TYPE_INT8_P8;
        case 16:
            return VIR_TYPE_INT8_P16;
        case 32:
            return VIR_TYPE_INT8_P32;
        default:
            gcmASSERT(0);
        }
    case VIR_TYPE_UINT16:
        switch(CompCount)
        {
        case 1:
            return VIR_TYPE_UINT16;
        case 2:
            return VIR_TYPE_UINT16_P2;
        case 3:
            return VIR_TYPE_UINT16_P3;
        case 4:
            return VIR_TYPE_UINT16_P4;
        case 8:
            return VIR_TYPE_UINT16_P8;
        case 16:
            return VIR_TYPE_UINT16_P16;
        case 32:
            return VIR_TYPE_UINT16_P32;
        default:
            gcmASSERT(0);
        }
    case VIR_TYPE_UINT8:
        switch(CompCount)
        {
        case 1:
            return VIR_TYPE_UINT8;
        case 2:
            return VIR_TYPE_UINT8_P2;
        case 3:
            return VIR_TYPE_UINT8_P3;
        case 4:
            return VIR_TYPE_UINT8_P4;
        case 8:
            return VIR_TYPE_UINT8_P8;
        case 16:
            return VIR_TYPE_UINT8_P16;
        case 32:
            return VIR_TYPE_UINT8_P32;
        default:
            gcmASSERT(0);
        }
    case VIR_TYPE_BOOLEAN:
        switch(CompCount)
        {
        case 1:
            return VIR_TYPE_BOOLEAN;
        case 2:
            return VIR_TYPE_BOOLEAN_P2;
        case 3:
            return VIR_TYPE_BOOLEAN_P3;
        case 4:
            return VIR_TYPE_BOOLEAN_P4;
        case 8:
            return VIR_TYPE_BOOLEAN_P8;
        case 16:
            return VIR_TYPE_BOOLEAN_P16;
        case 32:
            return VIR_TYPE_BOOLEAN_P32;
        default:
            gcmASSERT(0);
        }
    case VIR_TYPE_SNORM8:
    case VIR_TYPE_UNORM8:
    default:
        gcmASSERT(0);
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
            0,
            &typeId) != VSC_ERR_NONE)
        {
            typeId = VIR_TYPE_UNKNOWN;
        }
    }

    return typeId;
}

gctUINT
VIR_TypeId_GetSamplerCoordComponentCount(
    IN VIR_TypeId       SamplerType
    )
{
    if (VIR_TypeId_isSampler3D(SamplerType))
    {
        return 3;
    }
    else if (VIR_TypeId_isSamplerCube(SamplerType))
    {
        return 3;
    }
    else if (VIR_TypeId_isSampler2D(SamplerType))
    {
        return 2;
    }
    else if (VIR_TypeId_isSampler1D(SamplerType))
    {
        return 1;
    }
    else
    {
        gcmASSERT(gcvFALSE);
        return 2;
    }
}

VIR_TypeId
VIR_TypeId_ConvertSamplerTypeToImageType(
    IN VIR_Shader *     Shader,
    IN VIR_TypeId       SamplerType
    )
{
    VIR_TypeId          imageType = VIR_TYPE_IMAGE_2D;

    switch (SamplerType)
    {
    /* Floating point. */
    case VIR_TYPE_SAMPLER_1D:
        imageType = VIR_TYPE_IMAGE_1D;
        break;
    case VIR_TYPE_SAMPLER_1D_ARRAY:
        imageType = VIR_TYPE_IMAGE_1D_ARRAY;
        break;
    case VIR_TYPE_SAMPLER_2D:
        imageType = VIR_TYPE_IMAGE_2D;
        break;
    case VIR_TYPE_SAMPLER_2D_ARRAY:
        imageType = VIR_TYPE_IMAGE_2D_ARRAY;
        break;
    case VIR_TYPE_SAMPLER_3D:
        imageType = VIR_TYPE_IMAGE_3D;
        break;
    case VIR_TYPE_SAMPLER_BUFFER:
        imageType = VIR_TYPE_IMAGE_BUFFER;
        break;

    /* Signed integer. */
    case VIR_TYPE_ISAMPLER_1D:
        imageType = VIR_TYPE_IIMAGE_1D;
        break;
    case VIR_TYPE_ISAMPLER_1D_ARRAY:
        imageType = VIR_TYPE_IIMAGE_1D_ARRAY;
        break;
    case VIR_TYPE_ISAMPLER_2D:
        imageType = VIR_TYPE_IIMAGE_2D;
        break;
    case VIR_TYPE_ISAMPLER_2D_ARRAY:
        imageType = VIR_TYPE_IIMAGE_2D_ARRAY;
        break;
    case VIR_TYPE_ISAMPLER_3D:
        imageType = VIR_TYPE_IIMAGE_3D;
        break;
    case VIR_TYPE_ISAMPLER_BUFFER:
        imageType = VIR_TYPE_IIMAGE_BUFFER;
        break;

    /* Unsigned integer. */
    case VIR_TYPE_USAMPLER_1D:
        imageType = VIR_TYPE_UIMAGE_1D;
        break;
    case VIR_TYPE_USAMPLER_1D_ARRAY:
        imageType = VIR_TYPE_UIMAGE_1D_ARRAY;
        break;
    case VIR_TYPE_USAMPLER_2D:
        imageType = VIR_TYPE_UIMAGE_2D;
        break;
    case VIR_TYPE_USAMPLER_2D_ARRAY:
        imageType = VIR_TYPE_UIMAGE_2D_ARRAY;
        break;
    case VIR_TYPE_USAMPLER_3D:
        imageType = VIR_TYPE_UIMAGE_3D;
        break;
    case VIR_TYPE_USAMPLER_BUFFER:
        imageType = VIR_TYPE_UIMAGE_BUFFER;
        break;

    default:
        gcmASSERT(gcvFALSE);
        break;
    }

    return imageType;
}

VIR_TypeId
VIR_TypeId_ConvertIntegerType(
    IN VIR_Shader*      pShader,
    IN VIR_TypeId       origTypeId,
    IN gctBOOL          bSignedToUnsigned
    )
{
    VIR_TypeId          newTypeId = VIR_TYPE_UNKNOWN;
    VIR_TypeId          newCompTypeId = VIR_TYPE_UNKNOWN;
    VIR_TypeId          origCompTypeId = VIR_GetTypeComponentType(origTypeId);
    gctUINT32           componentCount = VIR_GetTypeComponents(origTypeId);
    gctUINT32           rowCount = VIR_GetTypeRows(origTypeId);

    gcmASSERT(VIR_TypeId_isPrimitive(origTypeId));

    if ((bSignedToUnsigned && VIR_TypeId_isUnSignedInteger(origTypeId))
        ||
        (!bSignedToUnsigned && VIR_TypeId_isSignedInteger(origTypeId)))
    {
        return origTypeId;
    }

    if (bSignedToUnsigned)
    {
        gcmASSERT(VIR_TypeId_isSignedInteger(origTypeId));

        switch (origCompTypeId)
        {
        case VIR_TYPE_INT64:
            newCompTypeId = VIR_TYPE_UINT64;
            break;
        case VIR_TYPE_INT32:
            newCompTypeId = VIR_TYPE_UINT32;
            break;
        case VIR_TYPE_INT16:
            newCompTypeId = VIR_TYPE_UINT16;
            break;
        case VIR_TYPE_INT8:
            newCompTypeId = VIR_TYPE_UINT8;
            break;
        default:
            gcmASSERT(gcvFALSE);
            newCompTypeId = origCompTypeId;
            break;
        }
    }
    else
    {
        gcmASSERT(VIR_TypeId_isUnSignedInteger(origTypeId));

        switch (origCompTypeId)
        {
        case VIR_TYPE_UINT64:
            newCompTypeId = VIR_TYPE_INT64;
            break;
        case VIR_TYPE_UINT32:
            newCompTypeId = VIR_TYPE_INT32;
            break;
        case VIR_TYPE_UINT16:
            newCompTypeId = VIR_TYPE_INT16;
            break;
        case VIR_TYPE_UINT8:
            newCompTypeId = VIR_TYPE_INT8;
            break;
        default:
            gcmASSERT(gcvFALSE);
            newCompTypeId = origCompTypeId;
            break;
        }
    }

    newTypeId = VIR_TypeId_ComposeNonOpaqueType(newCompTypeId, componentCount, rowCount);

    return newTypeId;
}

VIR_TypeId
VIR_TypeId_ConvertFP16Type(
    IN VIR_Shader*      pShader,
    IN VIR_TypeId       fp16TypeId
    )
{
    gctUINT             componentCount = VIR_GetTypeComponents(fp16TypeId);
    VIR_TypeId          newTypeId = fp16TypeId;

    if (VIR_GetTypeComponentType(fp16TypeId) == VIR_TYPE_FLOAT16)
    {
        newTypeId = VIR_TypeId_ComposeNonOpaqueType(VIR_TYPE_UINT16, componentCount, 1);
    }

    return newTypeId;
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
    IN  gctCONST_STRING Name)
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
    IN  void *          host, /* shader for global symbol, function for local symbol */
    IN  VIR_SymTable *  SymTable,
    IN  VIR_SymbolKind  SymbolKind,
    IN  VIR_Id          NameOrConstIdOrRegId, /* constId for VIR_SYM_CONST,VirRegId for VIR_SYM_VIRREG,
                                                 * otherwise nameId */
    IN  VIR_Type *      Type, /* for VIR_SYM_FIELD, use struct typeId */
    IN  VIR_StorageClass Storage,
    OUT VIR_SymId*      SymId
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    VIR_Symbol  sym;
    VIR_SymId   id;
    VIR_Symbol *pSym;

    memset(&sym, 0, sizeof(VIR_Symbol));
    VIR_Symbol_SetFirstElementId(&sym, VIR_INVALID_ID);
    VIR_Symbol_SetSeparateImageId(&sym, VIR_INVALID_ID, VIR_INVALID_ID);
    VIR_Symbol_SetKind(&sym, SymbolKind);
    VIR_Symbol_SetType(&sym, Type);
    VIR_Symbol_SetFixedTypeId(&sym, VIR_INVALID_ID);
    if (SymbolKind == VIR_SYM_CONST)
    {
        VIR_Symbol_SetConstId(&sym, NameOrConstIdOrRegId);
    }
    else if (SymbolKind == VIR_SYM_VIRREG)
    {
        VIR_Symbol_SetVregIndex(&sym, NameOrConstIdOrRegId);
        VIR_Symbol_SetVregVarSymId(&sym, VIR_INVALID_ID);
    }
    else
    {
        VIR_Symbol_SetSymbolName(&sym, NameOrConstIdOrRegId);
    }

    if (SymbolKind == VIR_SYM_FIELD)
    {
        VIR_Symbol_SetStructTypeId(&sym, VIR_Type_GetIndex(Type));
    }

    /* Initialize the layout. */
    VIR_LAYOUT_Initialize(VIR_Symbol_GetLayout(&sym));

    id = vscBT_HashSearch(SymTable, &sym);
    *SymId = id;
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
            if (BT_IS_FUNCTION_SCOPE(SymTable))
            {
                VIR_Symbol_SetFlag(pSym, VIR_SYMFLAG_LOCAL);
                VIR_Symbol_SetHostFunction(pSym, (VIR_Function *)host);
                VIR_Symbol_SetEncloseFuncSymId(pSym,
                         VIR_Function_GetSymId((VIR_Function *)host));
            }
            else
            {
                VIR_Symbol_SetHostShader(pSym, (VIR_Shader *)host);
            }
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
        VIR_Symbol_SetStructTypeId(sym, VIR_Type_GetIndex(StructType));
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
VIR_Shader_AddSymbolContents(
    IN  VIR_Shader *    Shader,
    IN  VIR_Symbol *    Sym,
    IN  VIR_Id          PresetId,
    IN  gctBOOL         UpdateIdList)
{
    VSC_ErrCode     errCode = VSC_ERR_NONE;
    VIR_SymbolKind  symKind = VIR_Symbol_GetKind(Sym);
    VIR_SymId       symId   = VIR_Symbol_GetIndex(Sym);

    {
        switch (symKind)
        {
        case VIR_SYM_UNIFORM:
        case VIR_SYM_SAMPLER:
        case VIR_SYM_SAMPLER_T:
        case VIR_SYM_IMAGE:
        case VIR_SYM_IMAGE_T:
            {
                VIR_Uniform *uniform;
                /* allocate uniform struct */
                uniform = (VIR_Uniform *)vscMM_Alloc(&Shader->pmp.mmWrapper,
                    sizeof(VIR_Uniform));
                if (uniform == gcvNULL)
                {
                    return VSC_ERR_OUT_OF_MEMORY;
                }
                memset(uniform, 0, sizeof(VIR_Uniform));
                VIR_Symbol_SetUniform(Sym, uniform);
                uniform->sym    = Sym->index;
                uniform->gcslIndex = -1;
                uniform->kernelArgIndex = -1;
                uniform->blockIndex = -1;
                uniform->offset = -1;
                uniform->lastIndexingIndex = -1;
                uniform->realUseArraySize = -1;
                VIR_Uniform_SetPhysical(uniform, -1);
                VIR_Uniform_SetSamplerPhysical(uniform, -1);

                if (VIR_Symbol_isImage(Sym) || VIR_Symbol_isImageT(Sym))
                {
                    VSC_ImageDesc desc = { { 0 } };
                    uniform->u.imageAttr.imageTSymId = VIR_INVALID_ID;
                    uniform->u.imageAttr.samplerTSymId = VIR_INVALID_ID;
                    uniform->u.imageAttr.samplerTValue = VSC_IMG_SAMPLER_INVALID_VALUE; /* not paired yet */
                    uniform->u.imageAttr.nextTandemSymId = VIR_INVALID_ID;
                    uniform->u.imageAttr.libFuncName = gcvNULL;
                    VIR_Uniform_SetImageDesc(uniform, desc);
                    uniform->isImage = gcvTRUE;
                }
                else if (VIR_Symbol_isSampler(Sym) || VIR_Symbol_isSamplerT(Sym))
                {
                    VIR_Uniform_SetImageSymId(uniform, VSC_IMG_SAMPLER_UNKNOWN_VALUE);
                    uniform->isSampler = gcvTRUE;
                    if (VIR_Symbol_isSampler(Sym))
                    {
                        uniform->u.samplerOrImageAttr.lodMinMax = VIR_INVALID_ID;
                        uniform->u.samplerOrImageAttr.levelBaseSize = VIR_INVALID_ID;
                        uniform->u.samplerOrImageAttr.levelsSamples = VIR_INVALID_ID;
                        uniform->u.samplerOrImageAttr.extraImageLayer = VIR_INVALID_ID;
                        uniform->u.samplerOrImageAttr.texelBufferToImageSymId   = VIR_INVALID_ID;
                        uniform->u.samplerOrImageAttr.sampledImageSymId   = VIR_INVALID_ID;
                    }
                }
                else
                {
                    uniform->u.samplerOrImageAttr.lodMinMax = VIR_INVALID_ID;
                    uniform->u.samplerOrImageAttr.levelBaseSize = VIR_INVALID_ID;
                    uniform->u.samplerOrImageAttr.levelsSamples = VIR_INVALID_ID;
                    uniform->u.samplerOrImageAttr.extraImageLayer = VIR_INVALID_ID;
                    uniform->u.samplerOrImageAttr.texelBufferToImageSymId   = VIR_INVALID_ID;
                    uniform->u.samplerOrImageAttr.sampledImageSymId   = VIR_INVALID_ID;
                }
                uniform->auxAddrSymId = VIR_INVALID_ID;
                if (PresetId == VIR_INVALID_ID)
                {
                    /* add uniform to shader uniform list */
                    VIR_IdList_Add(VIR_Shader_GetUniforms(Shader), symId);

                    VIR_Uniform_SetID(uniform, VIR_IdList_Count(VIR_Shader_GetUniforms(Shader)) - 1);
                }
                else if (UpdateIdList)
                {
                    /* use presetId */
                    VIR_Uniform_SetID(uniform, PresetId);

                    /* set uniform to shader uniform list */
                    VIR_IdList_Set(VIR_Shader_GetUniforms(Shader), PresetId, symId);
                    gcmASSERT(PresetId < VIR_IdList_Count(VIR_Shader_GetUniforms(Shader)));
                }

                VIR_Shader_SetConstRegAllocated(Shader, gcvFALSE);
                break;
            }
        case VIR_SYM_UBO:
            {
                VIR_UniformBlock *   ubo;
                /* allocate uniform struct */
                ubo = (VIR_UniformBlock *)vscMM_Alloc(&Shader->pmp.mmWrapper,
                    sizeof(VIR_UniformBlock));
                if (ubo == gcvNULL)
                {
                    return VSC_ERR_OUT_OF_MEMORY;
                }
                memset(ubo, 0, sizeof(VIR_UniformBlock));
                Sym->u2.ubo     = ubo;
                ubo->sym        = Sym->index;
                ubo->baseAddr   = VIR_INVALID_ID;

                if (PresetId == VIR_INVALID_ID)
                {
                    /* add uniform block to shader UBO list */
                    VIR_IdList_Add(VIR_Shader_GetUniformBlocks(Shader), symId);

                    VIR_UBO_SetBlockIndex(ubo, (gctINT16)(VIR_IdList_Count(&Shader->uniformBlocks) - 1));
                }
                else if (UpdateIdList)
                {
                    /* use presetId */
                    VIR_UBO_SetBlockIndex(ubo, (gctINT16)PresetId);
                    /* set blockIndex to shader uniform block list */
                    VIR_IdList_Set(VIR_Shader_GetUniformBlocks(Shader), PresetId, symId);
                    gcmASSERT(PresetId < VIR_IdList_Count(VIR_Shader_GetUniformBlocks(Shader)));
                }
                break;
            }
        case  VIR_SYM_SBO:
            {
                VIR_StorageBlock *   sbo;
                /* allocate sbo struct */
                sbo = (VIR_StorageBlock *)vscMM_Alloc(&Shader->pmp.mmWrapper,
                    sizeof(VIR_StorageBlock));
                if (sbo == gcvNULL)
                {
                    return VSC_ERR_OUT_OF_MEMORY;
                }
                memset(sbo, 0, sizeof(VIR_StorageBlock));
                Sym->u2.sbo     = sbo;
                sbo->sym        = Sym->index;
                sbo->baseAddr   = VIR_INVALID_ID;

                if (PresetId == VIR_INVALID_ID)
                {
                    /* add sbo to shader SBO list */
                    VIR_IdList_Add(VIR_Shader_GetSSBlocks(Shader), symId);

                    VIR_SBO_SetBlockIndex(sbo, (gctINT16)(VIR_IdList_Count(&Shader->storageBlocks) - 1));
                }
                else if (UpdateIdList)
                {
                    /* use presetId */
                    VIR_SBO_SetBlockIndex(sbo, (gctINT16)PresetId);
                    /* set ssbo to shader storage block list */
                    VIR_IdList_Set(VIR_Shader_GetSSBlocks(Shader), PresetId, symId);
                    gcmASSERT(PresetId < VIR_IdList_Count(VIR_Shader_GetSSBlocks(Shader)));
                }
                break;
            }
        case VIR_SYM_IOBLOCK:
            {
                VIR_IOBlock *   ibo;
                /* allocate ioblock struct */
                ibo = (VIR_IOBlock *)vscMM_Alloc(&Shader->pmp.mmWrapper,
                    sizeof(VIR_IOBlock));
                if (ibo == gcvNULL)
                {
                    return VSC_ERR_OUT_OF_MEMORY;
                }
                memset(ibo, 0, sizeof(VIR_IOBlock));
                Sym->u2.ioBlock     = ibo;
                ibo->sym            = Sym->index;

                if (PresetId == VIR_INVALID_ID)
                {
                    /* add IO block to shader UBO list */
                    VIR_IdList_Add(VIR_Shader_GetIOBlocks(Shader), symId);

                    VIR_IOBLOCK_SetBlockIndex(ibo, (gctINT16)(VIR_IdList_Count(&Shader->ioBlocks) - 1));
                }
                else if (UpdateIdList)
                {
                    /* use presetId */
                    VIR_IOBLOCK_SetBlockIndex(ibo, (gctINT16)PresetId);
                    /* set ssbo to shader storage block list */
                    VIR_IdList_Set(VIR_Shader_GetIOBlocks(Shader), PresetId, symId);
                    gcmASSERT(PresetId < VIR_IdList_Count(VIR_Shader_GetIOBlocks(Shader)));
                }
                break;
            }
        case  VIR_SYM_VARIABLE:
            if (PresetId == VIR_INVALID_ID)
            {
                if (VIR_Symbol_isPerPatchInput(Sym))
                {
                    /* add attribute to shader attribute list */
                    VIR_IdList_Add(VIR_Shader_GetPerpatchAttributes(Shader), symId);
                }
                else if (VIR_Symbol_isPerPatchOutput(Sym))
                {
                    /* add attribute to shader attribute list */
                    VIR_IdList_Add(VIR_Shader_GetPerpatchOutputs(Shader), symId);
                }
                /* "subsample_depth" is an input as well as a output,
                ** we need to add it to both input list and output list.
                */
                else if (VIR_Symbol_isAttribute(Sym) ||
                    VIR_Symbol_isOutput(Sym))
                {
                    if (VIR_Symbol_isAttribute(Sym))
                    {
                        /* add attribute to shader attribute list */
                        VIR_IdList_Add(VIR_Shader_GetAttributes(Shader), symId);
                    }
                    if (VIR_Symbol_isOutput(Sym))
                    {
                        /* add outputs to shader output list */
                        VIR_IdList_Add(VIR_Shader_GetOutputs(Shader), symId);
                    }
                }
                else if (VIR_Symbol_isLocalVar(Sym) ||
                    VIR_Symbol_isGlobalVar(Sym))
                {
                    /* add normal variables to shader variable list */
                    VIR_IdList_Add(VIR_Shader_GetVaribles(Shader), symId);
                }
                else if (VIR_Symbol_isSharedVariables(Sym))
                {
                    /* add normal variables to shader variable list */
                    VIR_IdList_Add(VIR_Shader_GetSharedVaribles(Shader), symId);
                }
            }
            break;
        case VIR_SYM_VIRREG:
            {
                VIR_VirRegId virRegId = VIR_Symbol_GetVregIndex(Sym);
                /* add <virregId, symId> to shader virreg hash table */
                vscHTBL_DirectSet(VIR_Shader_GetVirRegTable(Shader),
                    (void *)(gctUINTPTR_T)virRegId, (void *)(gctUINTPTR_T)symId);

                (void)VIR_Shader_UpdateVirRegCount(Shader, virRegId);
                break;
            }
        case VIR_SYM_FUNCTION:
            break;
        default:
            break;
        }
    }

    return errCode;
}

VSC_ErrCode
VIR_Shader_FindSymbol(
    IN  VIR_Shader *    Shader,
    IN  VIR_SymbolKind  SymbolKind,
    IN  VIR_Id          NameOrConstIdOrRegId, /* constId for VIR_SYM_CONST,
                                                   VirRegId for VIR_SYM_VIRREG,
                                                   otherwise nameId */
    IN  VIR_Type *       Type, /* for VIR_SYM_FIELD, use struct typeId */
    IN  VIR_StorageClass Storage,
    OUT VIR_SymId*      SymId
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    VIR_Symbol  sym;
    VIR_SymId   id;

    memset(&sym, 0, sizeof(VIR_Symbol));
    VIR_Symbol_SetFirstElementId(&sym, VIR_INVALID_ID);
    VIR_Symbol_SetSeparateImageId(&sym, VIR_INVALID_ID, VIR_INVALID_ID);
    VIR_Symbol_SetKind(&sym, SymbolKind);
    VIR_Symbol_SetType(&sym, Type);
    if (SymbolKind == VIR_SYM_CONST)
    {
        VIR_Symbol_SetConstId(&sym, NameOrConstIdOrRegId);
    }
    else if (SymbolKind == VIR_SYM_VIRREG)
    {
        VIR_Symbol_SetVregIndex(&sym, NameOrConstIdOrRegId);
        VIR_Symbol_SetVregVarSymId(&sym, VIR_INVALID_ID);
    }
    else
    {
        VIR_Symbol_SetSymbolName(&sym, NameOrConstIdOrRegId);
    }

    if (SymbolKind == VIR_SYM_FIELD)
    {
        VIR_Symbol_SetStructTypeId(&sym, VIR_Type_GetIndex(Type));
    }

    /* Initialize the layout. */
    VIR_LAYOUT_Initialize(VIR_Symbol_GetLayout(&sym));

    id = vscBT_HashSearch(&Shader->symTable, &sym);
    *SymId = id;
    if (VIR_Id_isInvalid(id))
    {
        errCode = VSC_ERR_NOT_FOUND;
    }

    return errCode;
}

VSC_ErrCode
VIR_Shader_AddSymbol(
    IN  VIR_Shader *    Shader,
    IN  VIR_SymbolKind  SymbolKind,
    IN  VIR_Id          NameOrConstIdOrRegId, /* constId for VIR_SYM_CONST,
                                                * VirRegId for VIR_SYM_VIRREG,
                                                * otherwise nameId */
    IN  VIR_Type *       Type, /* for VIR_SYM_FIELD, use struct type */
    IN  VIR_StorageClass Storage,
    OUT VIR_SymId*       SymId)
{
    VSC_ErrCode     errCode = VSC_ERR_NONE;
    VIR_Symbol *    sym = gcvNULL;

    errCode = VIR_SymTable_AddSymbol(Shader,
        &Shader->symTable,
        SymbolKind,
        NameOrConstIdOrRegId,
        Type,
        Storage,
        SymId);
    if (errCode == VSC_ERR_NONE)
    {
        sym = VIR_Shader_GetSymFromId(Shader, *SymId);
        errCode = VIR_Shader_AddSymbolContents(Shader, sym, VIR_INVALID_ID, gcvTRUE);
    }

    if (VirSHADER_DumpCodeGenVerbose(Shader))
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
VIR_Shader_DuplicateVariableFromSymbol(
    IN  VIR_Shader *    Shader,
    IN  VIR_Symbol*     Sym,
    OUT VIR_SymId*      DupSymId)
{
    VSC_ErrCode     errCode = VSC_ERR_NONE;
    char            name[256];
    VIR_NameId      nameId;
    gctUINT         offset = 0;
    VIR_SymId       id;
    VIR_Symbol      dupSym;
    VIR_Symbol*     newSym = gcvNULL;
    static gctUINT  dupId = 0;

    gcmASSERT (VIR_Symbol_GetKind(Sym) == VIR_SYM_VARIABLE);
    dupSym = *Sym;
    gcoOS_PrintStrSafe(name, 256, &offset, "%s_#dup%d", VIR_Shader_GetSymNameString(Shader, Sym), dupId++);
    VIR_Shader_AddString(Shader, name, &nameId);
    VIR_Symbol_SetName(&dupSym, nameId);

    id = vscBT_AddEntry(&Shader->symTable, &dupSym);
    if (VIR_Id_isInvalid(id))
    {
        errCode = VSC_ERR_OUT_OF_MEMORY;
    }
    else
    {
        if (BT_IS_FUNCTION_SCOPE(&Shader->symTable))
        {
            VIR_Id_SetFunctionScope(id);
        }
        newSym = VIR_GetSymFromId(&Shader->symTable, id);
        VIR_Symbol_SetIndex(newSym, id);
        *DupSymId = id;
    }

    if (errCode == VSC_ERR_NONE)
    {
        errCode = VIR_Shader_AddSymbolContents(Shader, newSym, VIR_INVALID_ID, gcvTRUE);
    }

    if (VirSHADER_DumpCodeGenVerbose(Shader))
    {
        VIR_Dumper * dumper = Shader->dumper;
        if (errCode != VSC_ERR_NONE)
        {
            VIR_LOG(dumper, "Error %d on adding %s: %s ",
                errCode, VIR_GetSymbolKindName(VIR_Symbol_GetKind(Sym)),
                name);
        }
        else
        {
            if (newSym == gcvNULL) newSym = VIR_Shader_GetSymFromId(Shader, *DupSymId);
            VIR_LOG(dumper, "Added %s %d: ", VIR_GetSymbolKindName(VIR_Symbol_GetKind(Sym)), *DupSymId);
            VIR_Symbol_Dump(dumper, newSym, gcvTRUE);
        }
        VIR_LOG_FLUSH(dumper);
    }

    return errCode;
}

VSC_ErrCode
VIR_Shader_DuplicateVariablelFromSymId(
    IN  VIR_Shader *    Shader,
    IN  VIR_SymId       SymId,
    OUT VIR_SymId*      DupSymId)
{
    VIR_Symbol* sym = VIR_Shader_GetSymFromId(Shader, SymId);

    return VIR_Shader_DuplicateVariableFromSymbol(Shader, sym, DupSymId);
}

VSC_ErrCode
VIR_Shader_AddSymbolWithName(
    IN  VIR_Shader *    Shader,
    IN  VIR_SymbolKind  SymbolKind,
    IN  gctCONST_STRING Name,
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

gctUINT
VIR_Shader_GetShareMemorySize(
    IN VIR_Shader *        pShader
    )
{
    gctUINT shareMemSizeInByte = 0;

    if (VIR_Shader_IsCL(pShader))
    {
        VIR_Function* pMainFunc = VIR_Shader_GetCurrentKernelFunction(pShader);

        if (pMainFunc != gcvNULL)
        {
            shareMemSizeInByte = pMainFunc->kernelInfo->localMemorySize;
        }
        gcmASSERT(shareMemSizeInByte == VSC_UTILS_ALIGN(shareMemSizeInByte, 256));
    }
    else
    {
        shareMemSizeInByte = VIR_Shader_GetLocalMemorySize(pShader);
    }

    return shareMemSizeInByte;
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

VSC_ErrCode
VIR_Shader_ReplaceBuiltInAttribute(
    IN VIR_Shader*          pShader
    )
{
    VSC_ErrCode             errCode  = VSC_ERR_NONE;
    VIR_AttributeIdList*    pAttrs = VIR_Shader_GetAttributes(pShader);
    gctUINT                 currAttr;

    /*
    **  1) Replace gl_DeviceIndex with immediate 0, as we will not be able to report multiple devices in a device group for the near future
    **  2) Change gl_ViewIndex from a attribute to a uniform as we can't directly support it.
    */

    for (currAttr = 0; currAttr < VIR_IdList_Count(pAttrs); currAttr++)
    {
        VIR_Symbol*         pAttributeSym = VIR_Shader_GetSymFromId(pShader, VIR_IdList_GetId(pAttrs, currAttr));
        gctBOOL             bRemoveAttr = gcvFALSE;

        if (VIR_Symbol_GetName(pAttributeSym) == VIR_NAME_DEVICE_INDEX)
        {
            VIR_FuncIterator    iter;
            VIR_FunctionNode*   funcNode;

            VIR_FuncIterator_Init(&iter, &pShader->functions);
            for (funcNode = VIR_FuncIterator_First(&iter); funcNode != gcvNULL; funcNode = VIR_FuncIterator_Next(&iter))
            {
                VIR_InstIterator    instIter;
                VIR_Instruction*    inst = gcvNULL;

                VIR_InstIterator_Init(&instIter, &funcNode->function->instList);
                for (inst = (VIR_Instruction *)VIR_InstIterator_First(&instIter);
                     inst != gcvNULL;
                     inst = (VIR_Instruction *)VIR_InstIterator_Next(&instIter))
                {
                    gctUINT srcId = 0;
                    for (srcId = 0 ; srcId < VIR_Inst_GetSrcNum(inst); ++srcId)
                    {
                        VIR_Operand  *operand = VIR_Inst_GetSource(inst, srcId);
                        if (VIR_Operand_GetOpKind(operand) == VIR_OPND_SYMBOL)
                        {
                            VIR_Symbol *srcSym  = VIR_Operand_GetSymbol(operand);
                            if (VIR_Symbol_GetName(srcSym) == VIR_NAME_DEVICE_INDEX)
                            {
                                /* replace it with immediate 0 */
                                VIR_Operand_SetImmediateInt(operand, 0);
                            }
                        }
                    }
                }
            }

            bRemoveAttr = gcvTRUE;
        }
        else if (VIR_Symbol_GetName(pAttributeSym) == VIR_NAME_VIEW_INDEX)
        {
            VIR_Symbol_SetKind(pAttributeSym, VIR_SYM_UNIFORM);
            VIR_Symbol_ClrFlag(pAttributeSym, VIR_SYMFLAG_WITHOUT_REG);
            VIR_Symbol_SetUniformKind(pAttributeSym, VIR_UNIFORM_VIEW_INDEX);
            VIR_Symbol_SetAddrSpace(pAttributeSym, VIR_AS_CONSTANT);
            VIR_Symbol_SetTyQualifier(pAttributeSym, VIR_TYQUAL_CONST);
            VIR_Symbol_SetLayoutQualifier(pAttributeSym, VIR_LAYQUAL_NONE);

            ON_ERROR(VIR_Shader_AddSymbolContents(pShader, pAttributeSym, VIR_INVALID_ID, gcvTRUE),
                "Add uniform content fail.");
            bRemoveAttr = gcvTRUE;
        }

        /* remove this attribute */
        if (bRemoveAttr)
        {
            VIR_IdList_DeleteByIndex(pAttrs, currAttr);
        }
    }

OnError:
    return errCode;
}

VIR_Uniform*
VIR_Shader_GetUniformFromGCSLIndex(
    IN  VIR_Shader *  Shader,
    IN  gctINT        GCSLIndex
    )
{
    VIR_UniformIdList* uniforms = VIR_Shader_GetUniforms(Shader);
    gctUINT uniformCount = VIR_IdList_Count(VIR_Shader_GetUniforms(Shader));
    gctUINT i;

    for(i = 0; i < uniformCount; i++)
    {
        VIR_UniformId uniformID = VIR_IdList_GetId(uniforms, i);
        VIR_Symbol* uniformSym = VIR_Shader_GetSymFromId(Shader, uniformID);
        VIR_Uniform* uniform = VIR_Symbol_GetUniformPointer(Shader, uniformSym);

        gcmASSERT(uniform);
        if(uniform->gcslIndex == GCSLIndex)
        {
            return uniform;
        }
    }

    return gcvNULL;
}

VIR_Uniform *
VIR_Shader_GetConstBorderValueUniform(
    IN VIR_Shader *  Shader
    )
{
    VSC_ErrCode  errCode = VSC_ERR_NONE;
    VIR_Symbol * sym;

    gctCONST_STRING constBorderValuename = "$ConstBorderValue";
    sym = VIR_Shader_FindSymbolByName(Shader, VIR_SYM_UNIFORM, constBorderValuename);
    if (sym != gcvNULL)
    {
        return VIR_Symbol_GetUniform(sym);
    }

    /* not found add a new one */
    errCode = VIR_Shader_AddNamedUniform(Shader, constBorderValuename, VIR_Shader_GetTypeFromId(Shader, VIR_TYPE_UINT_X4), &sym);
    if (errCode != VSC_ERR_NONE)
    {
        gcmASSERT(gcvFALSE);
        return gcvNULL;
    }
    VIR_Symbol_SetUniformKind(sym, VIR_UNIFORM_CONST_BORDER_VALUE);

    return VIR_Symbol_GetUniform(sym);
}

VSC_ErrCode
VIR_Shader_GetDUBO(
    IN VIR_Shader *     Shader,
    IN gctBOOL          CreateDUBO,
    OUT VIR_Symbol **   DUBO,
    OUT VIR_Symbol **   DUBOAddr
    )
{
    VSC_ErrCode virErrCode = VSC_ERR_NONE;
    VIR_UBOIdList* uboList = VIR_Shader_GetUniformBlocks(Shader);
    VIR_Symbol* duboSym = gcvNULL;

    if (VIR_Shader_GetDefaultUBOIndex(Shader) != -1)
    {
        duboSym = VIR_Shader_GetSymFromId(Shader, VIR_IdList_GetId(uboList, VIR_Shader_GetDefaultUBOIndex(Shader)));

        gcmASSERT(isSymUBODUBO(duboSym));
    }

    if (duboSym == gcvNULL && !CreateDUBO)
    {
        if (DUBO)
        {
            *DUBO = gcvNULL;
        }
        if (DUBOAddr)
        {
            *DUBOAddr = gcvNULL;
        }
    }
    else if (duboSym == gcvNULL)
    {
        /* varables used for creating default ubo */
        VIR_NameId dubo_nameId;
        VIR_TypeId dubo_typeId;
        VIR_SymId dubo_symId;
        VIR_Symbol* dubo_sym;
        VIR_UniformBlock* dubo_ub;
        /* varables used for creating default ubo address */
        VIR_NameId dubo_addr_nameId;
        VIR_SymId dubo_addr_symId;
        VIR_Symbol* dubo_addr_sym;
        VIR_Uniform* dubo_addr_uniform;

        /* create default ubo */
        {
            /* default ubo name */
            virErrCode = VIR_Shader_AddString(Shader,
                "#DefaultUBO",
                &dubo_nameId);
            if(virErrCode != VSC_ERR_NONE) return virErrCode;

            /* default ubo type */
            virErrCode = VIR_Shader_AddStructType(Shader,
                gcvFALSE,
                dubo_nameId,
                gcvFALSE,
                &dubo_typeId);
            if(virErrCode != VSC_ERR_NONE) return virErrCode;

            /* default ubo symbol */
            virErrCode = VIR_Shader_AddSymbol(Shader,
                VIR_SYM_UBO,
                dubo_nameId,
                VIR_Shader_GetTypeFromId(Shader, dubo_typeId),
                VIR_STORAGE_UNKNOWN,
                &dubo_symId);
            if(virErrCode != VSC_ERR_NONE) return virErrCode;

            dubo_sym = VIR_Shader_GetSymFromId(Shader, dubo_symId);
            VIR_Symbol_SetPrecision(dubo_sym, VIR_PRECISION_DEFAULT);
            VIR_Symbol_SetAddrSpace(dubo_sym, VIR_AS_CONSTANT);
            VIR_Symbol_SetTyQualifier(dubo_sym, VIR_TYQUAL_CONST);
            VIR_Symbol_SetLayoutQualifier(dubo_sym, VIR_LAYQUAL_PACKED);
            VIR_Symbol_SetFlag(dubo_sym, VIR_SYMFLAG_COMPILER_GEN);
            VIR_Symbol_SetFlag(dubo_sym, VIR_SYMUBOFLAG_IS_DUBO);

            dubo_ub = VIR_Symbol_GetUBO(dubo_sym);
            VIR_Shader_SetDefaultUBOIndex(Shader, dubo_ub->blockIndex);

            if(DUBO)
            {
                *DUBO = dubo_sym;
            }
        }

        /* create default ubo address */
        {
            virErrCode = VIR_Shader_AddString(Shader,
                "#DefaultUBO",
                &dubo_addr_nameId);
            if(virErrCode != VSC_ERR_NONE) return virErrCode;

            /* default ubo symbol */
            virErrCode = VIR_Shader_AddSymbol(Shader,
                VIR_SYM_UNIFORM,
                dubo_addr_nameId,
                VIR_Shader_GetTypeFromId(Shader, VIR_TYPE_UINT32),
                VIR_STORAGE_UNKNOWN,
                &dubo_addr_symId);

            dubo_addr_sym = VIR_Shader_GetSymFromId(Shader, dubo_addr_symId);
            VIR_Symbol_SetUniformKind(dubo_addr_sym, VIR_UNIFORM_UNIFORM_BLOCK_ADDRESS);
            VIR_Symbol_SetPrecision(dubo_addr_sym, VIR_PRECISION_HIGH);
            VIR_Symbol_SetFlag(dubo_addr_sym, VIR_SYMFLAG_COMPILER_GEN);

            dubo_addr_uniform = VIR_Symbol_GetUniform(dubo_addr_sym);
            dubo_addr_uniform->index = VIR_IdList_Count(VIR_Shader_GetUniforms(Shader)) - 1;
            dubo_addr_uniform->blockIndex = dubo_ub->blockIndex;

            if(DUBOAddr)
            {
                *DUBOAddr = dubo_addr_sym;
            }
        }

        dubo_ub->baseAddr = dubo_addr_symId;
    }
    else
    {
        if(DUBO)
        {
            *DUBO = duboSym;
        }
        if(DUBOAddr)
        {
            *DUBOAddr = VIR_Shader_GetSymFromId(Shader, VIR_Symbol_GetUBO(duboSym)->baseAddr);
        }
    }

    return virErrCode;
}

VSC_ErrCode
VIR_Shader_GetCUBO(
    IN VIR_Shader *     Shader,
    OUT VIR_Symbol **   CUBO,
    OUT VIR_Symbol **   CUBOAddr
    )
{
    VSC_ErrCode virErrCode = VSC_ERR_NONE;
    VIR_UBOIdList* uboList = VIR_Shader_GetUniformBlocks(Shader);
    gctUINT uboCount = VIR_IdList_Count(uboList);
    VIR_Symbol* cuboSym = gcvNULL;
    gctUINT i;

    for (i = 0; i < uboCount; i++)
    {
        VIR_Symbol*  uboSym = VIR_Shader_GetSymFromId(Shader, VIR_IdList_GetId(uboList, i));

        gcmASSERT(VIR_Symbol_isUBO(uboSym));
        if(isSymUBOCUBO(uboSym))
        {
            cuboSym = uboSym;
            break;
        }
    }

    if(cuboSym == gcvNULL)
    {
        /* varables used for creating default ubo */
        VIR_NameId cubo_nameId;
        VIR_TypeId cubo_typeId;
        VIR_SymId cubo_symId;
        VIR_Symbol* cubo_sym;
        VIR_UniformBlock* cubo_ub;
        /* varables used for creating default ubo address */
        VIR_NameId cubo_addr_nameId;
        VIR_SymId cubo_addr_symId;
        VIR_Symbol* cubo_addr_sym;
        VIR_Uniform* cubo_addr_uniform;

        /* create default ubo */
        {
            /* default ubo name */
            virErrCode = VIR_Shader_AddString(Shader,
                "#ConstantUBO",
                &cubo_nameId);
            if(virErrCode != VSC_ERR_NONE) return virErrCode;

            /* default ubo type */
            virErrCode = VIR_Shader_AddStructType(Shader,
                gcvFALSE,
                cubo_nameId,
                gcvFALSE,
                &cubo_typeId);
            if(virErrCode != VSC_ERR_NONE) return virErrCode;

            /* default ubo symbol */
            virErrCode = VIR_Shader_AddSymbol(Shader,
                VIR_SYM_UBO,
                cubo_nameId,
                VIR_Shader_GetTypeFromId(Shader, cubo_typeId),
                VIR_STORAGE_UNKNOWN,
                &cubo_symId);
            if(virErrCode != VSC_ERR_NONE) return virErrCode;

            cubo_sym = VIR_Shader_GetSymFromId(Shader, cubo_symId);
            VIR_Symbol_SetPrecision(cubo_sym, VIR_PRECISION_DEFAULT);
            VIR_Symbol_SetAddrSpace(cubo_sym, VIR_AS_CONSTANT);
            VIR_Symbol_SetTyQualifier(cubo_sym, VIR_TYQUAL_CONST);
            VIR_Symbol_SetLayoutQualifier(cubo_sym, VIR_LAYQUAL_PACKED);
            VIR_Symbol_SetFlag(cubo_sym, VIR_SYMFLAG_COMPILER_GEN);
            VIR_Symbol_SetFlag(cubo_sym, VIR_SYMUBOFLAG_IS_CUBO);

            cubo_ub = VIR_Symbol_GetUBO(cubo_sym);
            VIR_Shader_SetConstantUBOIndex(Shader, cubo_ub->blockIndex);
            cubo_ub->flags |= VIR_IB_WITH_CUBO;
            Shader->hasCRegSpill = gcvTRUE;

            if(CUBO)
            {
                *CUBO = cubo_sym;
            }
        }

        /* create default ubo address */
        {
            virErrCode = VIR_Shader_AddString(Shader,
                "#ConstantUBO_addr",
                &cubo_addr_nameId);
            if(virErrCode != VSC_ERR_NONE) return virErrCode;

            /* default ubo symbol */
            virErrCode = VIR_Shader_AddSymbol(Shader,
                VIR_SYM_UNIFORM,
                cubo_addr_nameId,
                VIR_Shader_IsEnableRobustCheck(Shader) ? VIR_Shader_GetTypeFromId(Shader, VIR_TYPE_UINT_X3)
                                                       : VIR_Shader_GetTypeFromId(Shader, VIR_TYPE_UINT32),
                VIR_STORAGE_UNKNOWN,
                &cubo_addr_symId);

            cubo_addr_sym = VIR_Shader_GetSymFromId(Shader, cubo_addr_symId);
            VIR_Symbol_SetUniformKind(cubo_addr_sym, VIR_UNIFORM_UNIFORM_BLOCK_ADDRESS);
            VIR_Symbol_SetPrecision(cubo_addr_sym, VIR_PRECISION_HIGH);
            VIR_Symbol_SetFlag(cubo_addr_sym, VIR_SYMFLAG_COMPILER_GEN);

            cubo_addr_uniform = VIR_Symbol_GetUniform(cubo_addr_sym);
            cubo_addr_uniform->index = VIR_IdList_Count(VIR_Shader_GetUniforms(Shader)) - 1;
            cubo_addr_uniform->blockIndex = cubo_ub->blockIndex;

            if(CUBOAddr)
            {
                *CUBOAddr = cubo_addr_sym;
            }
        }

        cubo_ub->baseAddr = cubo_addr_symId;
    }
    else
    {
        if(CUBO)
        {
            *CUBO = cuboSym;
        }
        if(CUBOAddr)
        {
            *CUBOAddr = VIR_Shader_GetSymFromId(Shader, VIR_Symbol_GetUBO(cuboSym)->baseAddr);
        }
    }

    return virErrCode;
}

gctBOOL
VIR_Shader_SupportImgLdSt(
    IN VIR_Shader*      pShader,
    IN VSC_HW_CONFIG*   pHwCfg,
    IN gctBOOL          bForGraphics
    )
{
    gctBOOL             bSupportImgLdSt = gcvFALSE;

    /* Check if chip can support IMG_LOAD/IMG_STORE. */
    bSupportImgLdSt = pHwCfg->hwFeatureFlags.supportImgAddr;

    /* For halti5 chips, if they don't have USC_GOS_ADDR_FIX feature, then they can't use IMG_LOAD/IMG_STORE for vs/ts/gs/ps. */
    if (bSupportImgLdSt &&
        pHwCfg->hwFeatureFlags.hasHalti5 &&
        !pHwCfg->hwFeatureFlags.hasUscGosAddrFix &&
        (bForGraphics || (pShader && VIR_Shader_IsGraphics(pShader))))
    {
        bSupportImgLdSt = gcvFALSE;
    }

    return bSupportImgLdSt;
}

gctBOOL
VIR_Shader_SupportAliasedAttribute(
    IN VIR_Shader*      pShader
    )
{
    /* The attribute aliasing is only allowed in OpenGLES2.0/OpenGL vertex shaders. */
    if ((VIR_Shader_IsDesktopGL(pShader) || VIR_Shader_IsES20Compiler(pShader))
        &&
        VIR_Shader_IsVS(pShader))
    {
        return gcvTRUE;
    }

    return gcvFALSE;
}

gctBOOL
VIR_Shader_SupportIoCommponentMapping(
    IN VIR_Shader*      pShader
    )
{
    if (VIR_Shader_IsVulkan(pShader) ||
        VIR_Shader_IsGL44(pShader) || VIR_Shader_IsGL45(pShader))
    {
        return gcvTRUE;
    }

    return gcvFALSE;
}

/* Bubble sort the symbol ID list, by default using the location to compare. */
void
VIR_Shader_BubbleSortSymIdList(
    IN VIR_Shader*      pShader,
    IN VIR_IdList*      pIdList,
    IN SortCompartFunc  pFunc,
    IN gctUINT          length
    )
{
    gctUINT             i, j;
    VIR_Id              temp;
    VIR_Symbol*         pSym1;
    VIR_Symbol*         pSym2;

    for (j = 0; j < length - 1; j++)
    {
        for (i = 0; i < length - 1 - j; i++)
        {
            gctBOOL bSwap = gcvFALSE;

            pSym1 = VIR_Shader_GetSymFromId(pShader, VIR_IdList_GetId(pIdList, i));
            pSym2 = VIR_Shader_GetSymFromId(pShader, VIR_IdList_GetId(pIdList, i + 1));

            if (pFunc != gcvNULL)
            {
                bSwap = (pFunc)(pSym1, pSym2);
            }
            else
            {
                bSwap = VIR_Symbol_GetLocation(pSym1) > VIR_Symbol_GetLocation(pSym2);
            }

            if (bSwap)
            {
                temp = VIR_IdList_GetId(pIdList, i);
                VIR_IdList_SetId(pIdList, (gctUINT)i, VIR_IdList_GetId(pIdList, i + 1));
                VIR_IdList_SetId(pIdList, (gctUINT)(i + 1), temp);
            }
        }
    }
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

VSC_ErrCode
VIR_Shader_GetVirRegSymByVirRegId(
    IN VIR_Shader *        Shader,
    IN VIR_VirRegId        VirRegId,
    OUT VIR_SymId *        pSymId
    )
{
    VSC_ErrCode            errCode = VSC_ERR_NONE;
    VIR_SymId              symId = VIR_INVALID_ID;
    void *                 result = gcvNULL;

    if (!vscHTBL_DirectTestAndGet(VIR_Shader_GetVirRegTable(Shader), (void*)(gctUINTPTR_T)VirRegId, &result))
    {
        symId = VIR_INVALID_ID;
    }
    else
    {
        gcmASSERT(result);
        symId = (VIR_SymId)(gctUINTPTR_T)result;
    }

    if (pSymId)
    {
        *pSymId = symId;
    }

    return errCode;
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

gctBOOL
VIR_Shader_TreatPushConstantAsBuffer(
    IN VIR_Shader*      pShader,
    IN VIR_Type*        pPushConstType
    )
{
    VIR_SymIdList*          pFields;
    VIR_Id                  fieldSymId;
    VIR_Symbol*             pFieldSym;
    VIR_Type*               pFieldType;
    gctBOOL                 bIsArray;
    gctUINT                 i;

    pFields = VIR_Type_GetFields(pPushConstType);

    for (i = 0; i < VIR_IdList_Count(pFields); i++)
    {
        fieldSymId = VIR_IdList_GetId(pFields, i);
        pFieldSym = VIR_Shader_GetSymFromId(pShader, fieldSymId);
        pFieldType = VIR_Symbol_GetType(pFieldSym);
        bIsArray = VIR_Type_isArray(pFieldType);

        /* Use the non-array struct type to calc and check if it is an arrays of arrays. */
        while (VIR_Type_isArray(pFieldType))
        {
            pFieldType = VIR_Shader_GetTypeFromId(pShader, VIR_Type_GetBaseTypeId(pFieldType));
        }

        if (VIR_Type_isStruct(pFieldType))
        {
            if (VIR_Shader_TreatPushConstantAsBuffer(pShader, pFieldType))
            {
                return gcvTRUE;
            }
        }
        else if ((bIsArray || VIR_Type_isMatrix(pFieldType))
                 &&
                 (VIR_GetTypeComponents(VIR_GetTypeRowType(VIR_Type_GetIndex(pFieldType))) <= 2))
        {
            return gcvTRUE;
        }
    }

    return gcvFALSE;
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

VIR_VirRegId
VIR_Symbol_GetFiledVregId(
    IN VIR_Symbol           *pFieldSym
    )
{
    VIR_VirRegId            vregId = VIR_Symbol_GetFieldVregOffset(pFieldSym);
    VIR_Shader              *pShader = VIR_Symbol_GetShader(pFieldSym);
    VIR_Symbol              *pParentSym = VIR_Shader_GetSymFromId(pShader, VIR_Symbol_GetParentId(pFieldSym));

    while (pParentSym && VIR_Symbol_isField(pParentSym))
    {
        vregId += VIR_Symbol_GetFieldVregOffset(pParentSym);
        pParentSym = VIR_Shader_GetSymFromId(pShader, VIR_Symbol_GetParentId(pParentSym));
    }

    vregId += VIR_Symbol_GetVregIndex(pParentSym);

    return vregId;
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
            (nameId2 == VIR_NAME_CLIP_DISTANCE && nameId1 == VIR_NAME_IN_CLIP_DISTANCE) ||
            (nameId2 == VIR_NAME_CULL_DISTANCE && nameId1 == VIR_NAME_IN_CULL_DISTANCE)
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

gctBOOL
VIR_Symbol_GetStartAndEndComponentForIO(
    IN VIR_Symbol*          pSym,
    IN gctBOOL              bLogicalIO,
    OUT gctUINT*            pStartComponent,
    OUT gctUINT*            pEndComponent
    )
{
    gctUINT                 startComponent, endComponent;

    if (VIR_Layout_HasComponent(VIR_Symbol_GetLayout(pSym)))
    {
        startComponent = VIR_Layout_GetComponent(VIR_Symbol_GetLayout(pSym));
        endComponent = startComponent + VIR_Symbol_GetComponents(pSym);
    }
    else
    {
        startComponent = 0;
        if (bLogicalIO)
        {
            endComponent = VIR_CHANNEL_NUM;
        }
        else
        {
            endComponent = VIR_Symbol_GetComponents(pSym);
        }
    }

    gcmASSERT(endComponent <= VIR_CHANNEL_NUM);

    if (pStartComponent)
    {
        *pStartComponent = startComponent;
    }

    if (pEndComponent)
    {
        *pEndComponent = endComponent;
    }

    return VIR_Layout_HasComponent(VIR_Symbol_GetLayout(pSym));
}

VIR_Uniform*
VIR_Symbol_GetHwMappingSeparateSamplerUniform(
    IN VSC_SHADER_RESOURCE_LAYOUT*  pResLayout,
    IN VIR_Shader*                  pShader,
    IN VIR_Symbol*                  pSym
    )
{
    VIR_SHADER_RESOURCE_ALLOC_LAYOUT*   pResAllocLayout = &pShader->shaderResAllocLayout;
    VIR_Symbol*                         pSeparateSamplerSym = VIR_Symbol_GetSeparateSampler(pShader, pSym);
    VIR_Symbol*                         pSeparateImageSym = VIR_Symbol_GetSeparateImage(pShader, pSym);

    if (pSeparateSamplerSym == gcvNULL || pSeparateImageSym == gcvNULL)
    {
        return gcvNULL;
    }

    gcmASSERT(pResAllocLayout || pResLayout);

    /*
    ** When the separate sampler and the separate image come from two different resources, and both resources are
    ** COMBINED_IMAGE_SAMPLER, we need to remap the sampler index based on the binding&set of the separate image.
    */
    if (VIR_Symbol_GetBinding(pSeparateSamplerSym) != VIR_Symbol_GetBinding(pSeparateImageSym)
        ||
        VIR_Symbol_GetDescriptorSet(pSeparateSamplerSym) != VIR_Symbol_GetDescriptorSet(pSeparateImageSym))
    {
        gctUINT32                   i, imageArraySize = 1, resCount, resEntryCount;
        VSC_SHADER_RESOURCE_BINDING imageBinding ={ VSC_SHADER_RESOURCE_TYPE_SAMPLER, 0, 0, 0 }, resBinding;
        VIR_Type*                   pType;
        VIR_UniformKind             uniformKind;
        VIR_Uniform*                pUniformArray[2] = { gcvNULL, gcvNULL };

        pType = VIR_Symbol_GetType(pSeparateImageSym);
        if (VIR_Type_isArray(pType))
        {
            imageArraySize = VIR_Type_GetArrayLength(pType);
        }

        if (pResLayout)
        {
            resEntryCount = pResLayout->resourceBindingCount;
        }
        else
        {
            resEntryCount = pResAllocLayout->resAllocEntryCount;
        }

        /* Find the resource bindings. */
        for (i = 0; i < resEntryCount; i++)
        {
            if (pResLayout)
            {
                resBinding = pResLayout->pResBindings[i];
            }
            else
            {
                resBinding = pResAllocLayout->pResAllocEntries[i].resBinding;
            }


            if (resBinding.binding == VIR_Symbol_GetBinding(pSeparateImageSym)    &&
                resBinding.set == VIR_Symbol_GetDescriptorSet(pSeparateImageSym)  &&
                resBinding.arraySize == imageArraySize)
            {
                imageBinding = resBinding;
                break;
            }
        }

        gcmASSERT(i < resEntryCount);

        /* For the other resources, like SAMPLED_IMAGE, we have already handled them in function "_CollectCompilerGeneatedCombinedSampler". */
        if (imageBinding.type == VSC_SHADER_RESOURCE_TYPE_COMBINED_IMAGE_SAMPLER)
        {
            uniformKind = VIR_Resouce_ResType2UniformKind(imageBinding.type);
            resCount = VIR_Resouce_FindResUniform(pShader, uniformKind, &imageBinding, VIR_FIND_RES_MODE_RES_ONLY, pUniformArray);

            if (resCount == 0)
            {
                gcmASSERT(gcvFALSE);
            }

            return pUniformArray[0];
        }
    }

    return gcvNULL;
}

VIR_Symbol*
VIR_Symbol_GetHwMappingSeparateSampler(
    IN VIR_Shader*                  pShader,
    IN VIR_Symbol*                  pSym
    )
{
    VIR_Uniform*                        pHwSeparateSamplerUniform;

    pHwSeparateSamplerUniform = VIR_Symbol_GetHwMappingSeparateSamplerUniform(gcvNULL, pShader, pSym);

    if (pHwSeparateSamplerUniform)
    {
        return VIR_Shader_GetSymFromId(pShader, VIR_Uniform_GetSymID(pHwSeparateSamplerUniform));
    }
    else
    {
        return VIR_Symbol_GetSeparateSampler(pShader, pSym);
    }
}

VIR_Symbol*
VIR_Symbol_GetSeparateSampler(
    IN VIR_Shader*          pShader,
    IN VIR_Symbol*          pSym
    )
{
    VIR_SymId               separateSamplerId = VIR_Symbol_GetSeparateSamplerId(pSym);
    VIR_SymId               funcId = VIR_Symbol_GetSeparateSamplerFuncId(pSym);

    gcmASSERT(VIR_Symbol_isVariable(pSym) ||
              (VIR_Symbol_isSampler(pSym) && VIR_Symbol_GetUniformKind(pSym) == VIR_UNIFORM_SAMPLED_IMAGE));

    if (separateSamplerId == VIR_INVALID_ID)
    {
        return gcvNULL;
    }

    if (funcId != VIR_INVALID_ID)
    {
        VIR_Function*       pFunc = VIR_Symbol_GetFunction(VIR_Shader_GetSymFromId(pShader, funcId));

        gcmASSERT(VIR_Id_isFunctionScope(separateSamplerId) && pFunc);

        return VIR_GetFuncSymFromId(pFunc, separateSamplerId);
    }
    else
    {
        return VIR_Shader_GetSymFromId(pShader, separateSamplerId);
    }
}

VIR_Symbol*
VIR_Symbol_GetSeparateImage(
    IN VIR_Shader*          pShader,
    IN VIR_Symbol*          pSym
    )
{
    VIR_SymId               separateImageId = VIR_Symbol_GetSeparateImageId(pSym);
    VIR_SymId               funcId = VIR_Symbol_GetSeparateImageFuncId(pSym);

    gcmASSERT(VIR_Symbol_isVariable(pSym) ||
              (VIR_Symbol_isSampler(pSym) && VIR_Symbol_GetUniformKind(pSym) == VIR_UNIFORM_SAMPLED_IMAGE));

    if (separateImageId == VIR_INVALID_ID)
    {
        return gcvNULL;
    }

    if (funcId != VIR_INVALID_ID)
    {
        VIR_Function*       pFunc = VIR_Symbol_GetFunction(VIR_Shader_GetSymFromId(pShader, funcId));

        gcmASSERT(VIR_Id_isFunctionScope(separateImageId) && pFunc);

        return VIR_GetFuncSymFromId(pFunc, separateImageId);
    }
    else
    {
        return VIR_Shader_GetSymFromId(pShader, separateImageId);
    }
}

VIR_Uniform*
VIR_Symbol_GetUniformPointer(
    IN VIR_Shader*          pShader,
    IN VIR_Symbol*          pSym
    )
{
    VIR_Uniform*            pUniform = gcvNULL;

    if (VIR_Symbol_isUniform(pSym))
    {
        pUniform = VIR_Symbol_GetUniform(pSym);
    }
    else if (VIR_Symbol_isSampler(pSym))
    {
        pUniform = VIR_Symbol_GetSampler(pSym);
    }
    else if (VIR_Symbol_isSamplerT(pSym))
    {
        pUniform = VIR_Symbol_GetSamplerT(pSym);
    }
    else if (VIR_Symbol_isImage(pSym))
    {
        pUniform = VIR_Symbol_GetImage(pSym);
    }
    else if (VIR_Symbol_isImageT(pSym))
    {
        pUniform = VIR_Symbol_GetImageT(pSym);
    }
    else if (isSymCombinedSampler(pSym))
    {
        pUniform = VIR_Symbol_GetSampler(pSym);
    }

    return pUniform;
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
    errCode = VIR_SymTable_AddSymbol(Function,
        &Function->symTable,
        SymbolKind,
        NameOrConstIdOrRegId,
        Type,
        Storage,
        SymId);
    if (VirSHADER_DumpCodeGenVerbose(Function->hostShader))
    {
        VIR_Dumper * dumper = Function->hostShader->dumper;
        sym = VIR_Function_GetSymFromId(Function, *SymId);
        VIR_LOG(dumper, "Added function scope %s %d: ",
                VIR_GetSymbolKindName(SymbolKind), VIR_Id_GetIndex(*SymId));
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
#if !defined(__STRICT_ANSI__)
    snprintf(name, sizeof(name), "_anony_%s_%d", AnonymousKindStr, Shader->_anonymousNameId++);
#endif
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
    gctCHAR     name[128];
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
VIR_Function_DuplicateLabel(
    IN  VIR_Function *  Function,
    IN  VIR_Label*      Label,
    OUT VIR_LabelId *   DupLabelId
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    VIR_SymId   labelSymId = VIR_Label_GetSymId(Label);
    VIR_Symbol* labelSym = VIR_Function_GetSymFromId(Function, labelSymId);
    gctSTRING   labelName = VIR_Shader_GetSymNameString(VIR_Function_GetShader(Function), labelSym);
    gctCHAR     name[128];
    gctUINT     offset = 0;
    static gctUINT dupId = 0;

    if (labelName != gcvNULL)
    {
        if (gcoOS_StrLen(labelName, gcvNULL) > 56)
        {
            gcoOS_PrintStrSafe(name, sizeof(name), &offset, "label_id_%d_dup%d",
                VIR_Symbol_GetIndex(labelSym), dupId++);
        }
        else
        {
            gcoOS_PrintStrSafe(name, sizeof(name), &offset, "%s_dup%d",
                labelName, dupId++);
        }
        labelName = name;
    }

    errCode = VIR_Function_AddLabel(Function, labelName, DupLabelId);

    return errCode;
}

VSC_ErrCode
VIR_Function_FreeLabel(
    IN  VIR_Function *  Function,
    IN  VIR_Label*      Label
    )
{
    VSC_ErrCode     errCode = VSC_ERR_NONE;
    VIR_Link *      link;

    /* set the symbol id to invalid */
    Label->sym = VIR_INVALID_ID;
    Label->defined = gcvNULL;
    link = Label->referenced;
    Label->referenced = gcvNULL;
    while (link)
    {
        VIR_Link * next = link->next;
        VIR_Function_FreeLink(Function, link);
        link = next;
    }
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

    /* add variable to function local variable list */
    VIR_IdList_Add(&Function->localVariables, symId);

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

    new_link = (VIR_Link *)vscMM_Alloc(&Function->hostShader->pmp.mmWrapper,
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
    vscMM_Free(&Function->hostShader->pmp.mmWrapper, Link);
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
    gcmASSERT(Dup);

    errCode = VIR_Function_NewOperand(Function, Dup);
    if(errCode != VSC_ERR_NONE)
    {
        return errCode;
    }

    VIR_Operand_Copy(*Dup, Src);

    return errCode;
}

VSC_ErrCode
VIR_Function_DupFullOperand(
    IN  VIR_Function *  Function,
    IN  VIR_Operand *   Src,
    OUT VIR_Operand **  Dup
    )
{
    VSC_ErrCode     errCode = VSC_ERR_NONE;
    VIR_Operand     *subOpnd = gcvNULL;
    gctUINT         i = 0;

    gcmASSERT(Dup);

    errCode = VIR_Function_NewOperand(Function, Dup);
    if(errCode != VSC_ERR_NONE)
    {
        return errCode;
    }

    VIR_Operand_Copy(*Dup, Src);

    if (VIR_Operand_isParameters(Src))
    {
        VIR_ParmPassing *srcParm = VIR_Operand_GetParameters(Src);
        VIR_ParmPassing *dupParm = VIR_Operand_GetParameters(*Dup);

        gcmASSERT(srcParm->argNum != 0);

        VIR_Function_NewParameters(Function, srcParm->argNum, &dupParm);
        VIR_Operand_SetParams(*Dup, dupParm);

        for (i = 0; i < srcParm->argNum; i++)
        {
            if (srcParm->args[i])
            {
                errCode = VIR_Function_DupOperand(Function, srcParm->args[i], &subOpnd);
                if(errCode != VSC_ERR_NONE)
                {
                    return errCode;
                }

                dupParm->args[i] = subOpnd;
            }
        }
    }
    else if (VIR_Operand_isTexldParm(Src))
    {
        VIR_Operand *srcTexldOperand = (VIR_Operand*)Src;
        VIR_Operand *dupTexldOperand = (VIR_Operand*)(*Dup);

        for (i = 0; i < VIR_TEXLDMODIFIER_COUNT; ++i)
        {
            if (VIR_Operand_GetTexldModifier(srcTexldOperand, i))
            {
                errCode = VIR_Function_DupOperand(Function, VIR_Operand_GetTexldModifier(srcTexldOperand,i), &subOpnd);
                if(errCode != VSC_ERR_NONE)
                {
                    return errCode;
                }

                VIR_Operand_SetTexldModifier(dupTexldOperand, i, subOpnd);
            }
        }
    }

    return errCode;
}

VSC_ErrCode
VIR_Function_NewParameters(
    IN  VIR_Function *  Function,
    IN  gctUINT         argsNum,
    OUT VIR_ParmPassing **Parameters
    )
{
    VSC_ErrCode     errCode = VSC_ERR_NONE;
    VIR_ParmPassing *parm;

    /* VIR_ParmPassing size: sizeof(count) + count * sizeof(Operand*) */
    gctUINT allocSize = gcmSIZEOF(VIR_ParmPassing);

    if (argsNum > 0)
    {
        allocSize += (argsNum - 1) * sizeof (VIR_Operand *);
    }

    parm = (VIR_ParmPassing *)vscMM_Alloc(&Function->hostShader->pmp.mmWrapper, allocSize);

    if (parm == gcvNULL)
    {
        errCode = VSC_ERR_OUT_OF_MEMORY;
    }
    else
    {
        gctUINT i;
        VIR_Operand * src;

        parm->argNum = argsNum;

        /* allocate parameter operands */
        for (i = 0; i < argsNum; i++)
        {
            errCode = VIR_Function_NewOperand(Function, &src);
            parm->args[i] = src;
        }

        *Parameters = parm;
    }

    return errCode;
}

VSC_ErrCode
VIR_Function_NewPhiOperandArray(
    IN  VIR_Function *          Function,
    IN  gctUINT                 Count,
    OUT VIR_PhiOperandArray **     PhiOperandArray
    )
{
    VSC_ErrCode     errCode = VSC_ERR_NONE;
    VIR_PhiOperandArray* phiOperands;

    gcmASSERT(Count > 0);
    phiOperands = (VIR_PhiOperandArray *)vscMM_Alloc(&Function->hostShader->pmp.mmWrapper, VIR_PhiOperandArray_ComputeSize(Count));

    if (phiOperands == gcvNULL)
    {
        errCode = VSC_ERR_OUT_OF_MEMORY;
    }
    else
    {
        memset(phiOperands, 0, VIR_PhiOperandArray_ComputeSize(Count));
        VIR_PhiOperandArray_SetCount(phiOperands, Count);
        VIR_PhiOperandArray_SetOperands(phiOperands, (VIR_PhiOperand*)(phiOperands + 1));
        *PhiOperandArray = phiOperands;
    }

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
    inst = (VIR_Instruction *)vscBT_NewEntryPtr(&Function->hostShader->instTable);

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

        /* zero all bits */
        memset(inst, 0, sizeof(VIR_Instruction));
        VIR_Inst_SetOpcode(inst, Opcode);
        VIR_Inst_SetSrcNum(inst, srcNum);
        VIR_Inst_SetInstType(inst, ResType);
        VIR_Inst_SetConditionOp(inst, VIR_COP_ALWAYS);
        VIR_Inst_SetFunction(inst, Function);
        VIR_Inst_SetId(inst, VIR_Function_GetAndIncressLastInstId(Function));
        VIR_Inst_SetDual16ExpandSeq(inst, NOT_ASSIGNED);
        VIR_Inst_SetMCInstPC(inst, -1);

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
            ON_ERROR0(VIR_Function_NewOperand(Function, &src));
            VIR_Inst_SetSource(inst, i, src);
        }
    }
OnError:
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

        if (Function->pFuncBlock && Function->pFuncBlock->cfg.pOwnerFuncBlk)
        {
            VIR_Inst_SetBasicBlock(inst, CFG_GET_EXIT_BB(VIR_Function_GetCFG(Function)));

            if (BB_GET_START_INST(inst->parent.BB) == gcvNULL)
            {
                BB_SET_START_INST(inst->parent.BB, *Inst);
            }

            BB_SET_END_INST(inst->parent.BB, *Inst);
            BB_INC_LENGTH(inst->parent.BB);
        }
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

        /* copy location */
        if (Function->instList.pHead)
        {
            VIR_Inst_CopySrcLoc(Function->instList.pHead->sourceLoc, inst->sourceLoc);
        }

        /* link nodes */
        vscBILST_Prepend((VSC_BI_LIST *)&Function->instList, (VSC_BI_LIST_NODE *)inst);

        if (Function->pFuncBlock && Function->pFuncBlock->cfg.pOwnerFuncBlk)
        {
            VIR_Inst_SetBasicBlock(inst, CFG_GET_ENTRY_BB(VIR_Function_GetCFG(Function)));

            if (BB_GET_END_INST(inst->parent.BB) == gcvNULL)
            {
                BB_SET_END_INST(inst->parent.BB, *Inst);
            }

            BB_SET_START_INST(inst->parent.BB, *Inst);
            BB_INC_LENGTH(inst->parent.BB);
        }

        /* set location */
        if (Function->debugInfo != gcvNULL)
        {
            VSC_DIE * die;
            die = vscDIGetDIE((VSC_DIContext *) (Function->debugInfo), Function->die);

            if (die)
            {
                inst->sourceLoc.fileId = die->fileNo;
                inst->sourceLoc.lineNo = die->lineNo;
                inst->sourceLoc.colNo  = die->colNo;
            }
        }
    }
    return errCode;
}

VSC_ErrCode
VIR_Function_AddInstructionAfter(
    IN  VIR_Function *  Function,
    IN  VIR_OpCode      Opcode,
    IN  VIR_TypeId      ResType,
    IN  VIR_Instruction *AfterMe,
    IN  gctBOOL         SameBB,
    OUT VIR_Instruction **Inst
    )
{
    VSC_ErrCode errCode;
    VIR_Instruction * inst;
    errCode = VIR_Function_NewInstruction(Function, Opcode, ResType, &inst);
    if (errCode == VSC_ERR_NONE)
    {
        /* link nodes */
        vscBILST_InsertAfter((VSC_BI_LIST *)&Function->instList,
            (VSC_BI_LIST_NODE *)AfterMe,
            (VSC_BI_LIST_NODE *)inst);

        if (VIR_Inst_GetBasicBlock(AfterMe))
        {
            if (SameBB)
            {
                if (AfterMe->parent.BB->pEndInst == AfterMe)
                {
                    BB_SET_END_INST(AfterMe->parent.BB, inst);
                }

                VIR_Inst_SetBasicBlock(inst, AfterMe->parent.BB);
                BB_INC_LENGTH(AfterMe->parent.BB);
            }
            else
            {
                gcmASSERT(AfterMe == BB_GET_END_INST(AfterMe->parent.BB));
            }
        }

        inst->sourceLoc = AfterMe->sourceLoc;
    }

    if(Inst)
    {
        *Inst = inst;
    }

    return errCode;
}

VSC_ErrCode
VIR_Function_AddCopiedInstructionAfter(
    IN  VIR_Function *  Function,
    IN  VIR_Instruction *CopyFrom,
    IN  VIR_Instruction *AfterMe,
    IN  gctBOOL         SameBB,
    OUT VIR_Instruction **Inst
    )
{
    VSC_ErrCode errCode;
    VIR_Instruction* inst;

    errCode = VIR_Function_AddInstructionAfter(Function, VIR_Inst_GetOpcode(CopyFrom), VIR_Inst_GetInstType(CopyFrom), AfterMe, SameBB, &inst);
    if(errCode == VSC_ERR_NONE)
    {
        VIR_Inst_Copy(inst, CopyFrom, gcvFALSE);

        if(Inst)
        {
            *Inst = inst;
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
    IN  gctBOOL         SameBB,
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

        if (VIR_Inst_GetBasicBlock(BeforeMe))
        {
            if (SameBB)
            {
                if (BeforeMe->parent.BB->pStartInst == BeforeMe)
                {
                    BB_SET_START_INST(BeforeMe->parent.BB, *Inst);
                }

                VIR_Inst_SetBasicBlock(*Inst, BeforeMe->parent.BB);
                BB_INC_LENGTH(BeforeMe->parent.BB);
            }
            else
            {
                gcmASSERT(BeforeMe == BB_GET_START_INST(BeforeMe->parent.BB));
            }
        }

        inst->sourceLoc = BeforeMe->sourceLoc;
    }
    return errCode;
}

VSC_ErrCode
VIR_Function_AddCopiedInstructionBefore(
    IN  VIR_Function *  Function,
    IN  VIR_Instruction *CopyFrom,
    IN  VIR_Instruction *BeforeMe,
    IN  gctBOOL         SameBB,
    OUT VIR_Instruction **Inst
    )
{
    VSC_ErrCode errCode;
    VIR_Instruction* inst;

    errCode = VIR_Function_AddInstructionBefore(Function, VIR_Inst_GetOpcode(CopyFrom), VIR_Inst_GetInstType(CopyFrom), BeforeMe, SameBB, &inst);
    if(errCode == VSC_ERR_NONE)
    {
        VIR_Inst_Copy(inst, CopyFrom, gcvFALSE);

        if(Inst)
        {
            *Inst = inst;
        }
    }
    return errCode;
}

VSC_ErrCode
VIR_Function_RemoveInstruction(
    IN VIR_Function *   Function,
    IN VIR_Instruction *Inst,
    IN gctBOOL          bRemoveLabelOrLink
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;

    vscBILST_Remove((VSC_BI_LIST *)&Function->instList, (VSC_BI_LIST_NODE *)Inst);

    if (bRemoveLabelOrLink)
    {
        if (VIR_Inst_GetOpcode(Inst) == VIR_OP_LABEL)
        {
            VIR_Operand* dest = VIR_Inst_GetDest(Inst);
            VIR_Label* label = VIR_Operand_GetLabel(dest);

            VIR_Function_FreeLabel(Function, label);
        }

        if (VIR_OPCODE_isBranch(VIR_Inst_GetOpcode(Inst)))
        {
            VIR_Operand* dest = VIR_Inst_GetDest(Inst);
            VIR_Label* label = VIR_Operand_GetLabel(dest);
            VIR_Link* link = VIR_Link_RemoveLink(VIR_Label_GetReferenceAddr(label), (gctUINTPTR_T)Inst);

            if (link)
            {
                VIR_Function_FreeLink(Function, link);
            }
        }
    }

    if (VIR_Inst_GetParentUseBB(Inst))
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
/* delete instruction and free its operands */
VSC_ErrCode
VIR_Function_DeleteInstruction(
    IN VIR_Function *     Function,
    IN VIR_Instruction *  Inst
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    errCode = VIR_Function_RemoveInstruction(Function, Inst, gcvTRUE);

    /* free operands */
    if (errCode == VSC_ERR_NONE)
    {
        gctUINT i;
        /* Free operands associated with this inst */
        for (i = 0; i < VIR_Inst_GetSrcNum(Inst); i ++)
        {
            VIR_Inst_FreeSource(Inst, i);
        }

        if (VIR_Inst_GetDest(Inst))
        {
            VIR_Inst_FreeDest(Inst);
        }
    }

    memset(Inst, 0xde, sizeof(VIR_Instruction));
    vscBT_RemoveEntryPtr(&Function->hostShader->instTable, Inst);

    return errCode;
}

VSC_ErrCode
VIR_Function_MoveInstructionBefore(
    IN  VIR_Function *  MoveFunction,
    IN  VIR_Instruction *BeforeMe,
    OUT VIR_Instruction *Inst
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;

    /* These two instruction must from the same function. */
    gcmASSERT(MoveFunction == VIR_Inst_GetFunction(BeforeMe));

    errCode = VIR_Function_RemoveInstruction(MoveFunction, Inst, gcvFALSE);
    if (errCode == VSC_ERR_NONE)
    {
        /* link nodes */
        vscBILST_InsertBefore((VSC_BI_LIST *)&MoveFunction->instList,
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
VIR_Function_AddPhiOperandArrayForInst(
    IN  VIR_Function *      Function,
    IN  VIR_Instruction *   Inst,
    IN  gctUINT             PhiOperandCount
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    VIR_PhiOperandArray* phiOperandArray;

    gcmASSERT(VIR_Inst_GetOpcode(Inst) == VIR_OP_PHI || VIR_Inst_GetOpcode(Inst) == VIR_OP_SPV_PHI);
    ON_ERROR(VIR_Function_NewPhiOperandArray(Function, PhiOperandCount, &phiOperandArray), "VIR_Function_NewPhiOperandArray");
    VIR_Operand_SetPhiOperands(VIR_Inst_GetSource(Inst, 0), phiOperandArray);
    VIR_Operand_SetOpKind(VIR_Inst_GetSource(Inst, 0), VIR_OPND_PHI);

OnError:
    return errCode;
}

VSC_ErrCode
VIR_Function_FreePhiOperandArray(
    IN  VIR_Function *      Function,
    IN  VIR_PhiOperandArray *  PhiOperandArray
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    gctUINT i;

    for(i = 0; i < VIR_PhiOperandArray_GetCount(PhiOperandArray); i++)
    {
        VIR_PhiOperand* phiOperand = VIR_PhiOperandArray_GetNthOperand(PhiOperandArray, i);
        ON_ERROR(VIR_Function_FreeOperand(Function, VIR_PhiOperand_GetValue(phiOperand)), "VIR_Function_FreeOperand");
    }
    /* free the whole PhiOperandArray which contains N PhiValues */
    vscMM_Free(&Function->hostShader->pmp.mmWrapper, PhiOperandArray);

OnError:
    return errCode;
}

VSC_ErrCode
VIR_Function_FreeOperandList(
    IN  VIR_Function *      Function,
    IN  VIR_OperandList *   pOperandList
    )
{
    VSC_ErrCode errCode   = VSC_ERR_NONE;
    VIR_OperandList * ptr = pOperandList;

    while (ptr)
    {
        VIR_OperandList * node = ptr;
        ptr = ptr->next;
        ON_ERROR(VIR_Function_FreeOperand(Function, node->value), "VIR_Function_FreeOperand");
        vscMM_Free(&Function->hostShader->pmp.mmWrapper, node);
    }

OnError:
    return errCode;
}

VSC_ErrCode
VIR_Function_FreeyParmPassing(
    IN  VIR_Function *      Function,
    IN  VIR_ParmPassing*    pParmPassing
    )
{
    VSC_ErrCode errCode   = VSC_ERR_NONE;
    gctUINT i;

    for (i=0; i<pParmPassing->argNum; i++)
    {
        if (pParmPassing->args[i])
        {
            ON_ERROR(VIR_Function_FreeOperand(Function, pParmPassing->args[i]), "VIR_Function_FreeOperand");
        }
    }
    /* free the whole PhiOperandArray which contains N PhiValues */
    vscMM_Free(&Function->hostShader->pmp.mmWrapper, pParmPassing);

OnError:
    return errCode;
}

VSC_ErrCode
VIR_Function_FreeOperand(
    IN  VIR_Function *  Function,
    IN  VIR_Operand *   pOperand
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    if (pOperand == gcvNULL)
    {
        return errCode;
    }
    switch (VIR_Operand_GetOpKind(pOperand))
    {
    case VIR_OPND_PHI:
        ON_ERROR0(VIR_Function_FreePhiOperandArray(Function, VIR_Operand_GetPhiOperands(pOperand)));
        break;
    case VIR_OPND_UNUSED:
        /* do not free a already freed operand again */
        return errCode;

    default:
        break;
    }
    /* mark the freed operand as unused, so when VIR_IO_Write iterating through the
    * operand table, the freed operand in the table will not be processed */
    VIR_Operand_SetOpKind(pOperand, VIR_OPND_UNUSED);
    vscBT_RemoveEntry(&Function->operandTable, VIR_Operand_GetIndex(pOperand));

OnError:
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

VSC_ErrCode
VIR_Function_BuildLabelLinks(
    VIR_Function *     pFunction
    )
{
    VSC_ErrCode         errCode = VSC_ERR_NONE;
    VIR_Instruction *   inst;
    VIR_InstIterator    instIter;

    VIR_InstIterator_Init(&instIter, VIR_Function_GetInstList(pFunction));
    inst = (VIR_Instruction *)VIR_InstIterator_First(&instIter);
    for (; inst != gcvNULL; inst = (VIR_Instruction *)VIR_InstIterator_Next(&instIter))
    {
        if (VIR_OPCODE_isBranch(VIR_Inst_GetOpcode(inst)))
        {
            VIR_Label *label     = VIR_Operand_GetLabel(VIR_Inst_GetDest(inst));
            VIR_Link  *pNewLink  = gcvNULL;
            ON_ERROR(VIR_Function_NewLink(pFunction, &pNewLink), "VIR_Function_NewLink");
            VIR_Link_SetReference(pNewLink, (gctUINTPTR_T)inst);
            VIR_Link_AddLink(&(label->referenced), pNewLink);
        }
    }
OnError:
    return errCode;
}

void
VIR_Function_ChangeInstToNop(
    IN  VIR_Function *      Function,
    IN OUT VIR_Instruction *   Inst
    )
{
    VIR_OpCode opcode = VIR_Inst_GetOpcode(Inst);
    gctUINT i;

    if(opcode == VIR_OP_LABEL)
    {
        VIR_Label* label = VIR_Operand_GetLabel(VIR_Inst_GetDest(Inst));

        gcmASSERT(VIR_Label_GetReference(label) == gcvNULL);
        VIR_Function_FreeLabel(Function, label);
    }

    if(VIR_OPCODE_isBranch(VIR_Inst_GetOpcode(Inst)))
    {
        VIR_Operand* dest = VIR_Inst_GetDest(Inst);
        VIR_Label* label = VIR_Operand_GetLabel(dest);
        VIR_Link* link = VIR_Link_RemoveLink(VIR_Label_GetReferenceAddr(label), (gctUINTPTR_T)Inst);

        if(link)
        {
            VIR_Function_FreeLink(Function, link);
        }
    }

    if(VIR_Inst_GetDest(Inst))
    {
        VIR_Function_FreeOperand(Function, VIR_Inst_GetDest(Inst));
        VIR_Inst_SetDest(Inst, gcvNULL);
    }

    for(i = 0; i < VIR_Inst_GetSrcNum(Inst); i++)
    {
        gcmASSERT(i < VIR_MAX_SRC_NUM);

        if (VIR_Inst_GetSource(Inst, i) != gcvNULL)
        {
            VIR_Function_FreeOperand(Function, VIR_Inst_GetSource(Inst, i));
            VIR_Inst_SetSource(Inst, i, gcvNULL);
        }
    }

    VIR_Inst_SetConditionOp(Inst, VIR_COP_ALWAYS);
    VIR_Inst_SetSrcNum(Inst, 0);
    VIR_Inst_SetOpcode(Inst, VIR_OP_NOP);
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
    VIR_Operand*            pDest;
    VIR_Label*              pLabel;

    gcmASSERT(VIR_OPCODE_isBranch(VIR_Inst_GetOpcode(Inst)));

    pDest = VIR_Inst_GetDest(Inst);
    pLabel = VIR_Operand_GetLabel(pDest);

    if(pLabel)
    {
        pBranchTargetInst = VIR_Label_GetDefInst(pLabel);

        gcmASSERT(pBranchTargetInst);

        return VIR_Inst_GetBasicBlock(pBranchTargetInst);
    }
    else
    {
        return gcvNULL;
    }
}

gctBOOL VIR_Inst_isComponentwise(VIR_Instruction *Inst)
{
    VIR_OpCode  opcode = VIR_Inst_GetOpcode(Inst);
    if (VIR_OPCODE_useCondCode(opcode))
    {
        /* ALLMSB/ANYMSB is not componentwise */
        VIR_ConditionOp condOp = VIR_Inst_GetConditionOp(Inst);
        if (condOp != VIR_COP_ANYMSB &&
            condOp != VIR_COP_ALLMSB)
        {
            return VIR_OPCODE_isComponentwise(opcode);
        }
    }
    else
    {
        return VIR_OPCODE_isComponentwise(opcode);
    }

    return gcvFALSE;
}

gctBOOL VIR_Inst_ConditionalWrite(VIR_Instruction *Inst)
{
    VIR_OpCode  opcode = VIR_Inst_GetOpcode(Inst);

    if (VIR_OPCODE_CONDITIONAL_WRITE(opcode)    ||
        /* vx_img_load has board color problem. We need a pre-assigned value for out-of-bound coordinates.
           thus vx_img_load should not kill the previous defined assignment */
        VIR_OPCODE_isVXImgLoad(opcode)          ||
        /* index_add - there is a implicit use of dest in this instruction
           dest = dest + ...
           thus it should always be may-def*/
        opcode == VIR_OP_VX_INDEXADD)
    {
        return gcvTRUE;
    }
    if (gcmOPT_hasFeature(FB_ENABLE_CONST_BORDER) &&
        (opcode == VIR_OP_IMG_LOAD ||
         opcode == VIR_OP_IMG_LOAD_3D ||
         opcode == VIR_OP_IMG_READ ||
         opcode == VIR_OP_IMG_READ_3D))
    {
        /* in constant border mode, the normal image reads become conditional write,
         * if the coordinate is our of boundary, the data is not write to the dest */
        return gcvTRUE;
    }
    else if (opcode == VIR_OP_SWIZZLE)
    {
        if ((VIR_Inst_GetFlags(Inst) & VIR_INSTFLAG_FULL_DEF) == 0)
        {
            return gcvTRUE;
        }
    }
    else if (VIR_OPCODE_isVX(opcode))
    {
        gctUINT         i = 0;
        VIR_Operand*    typeOpnd = gcvNULL;
        gctUINT         compCount = 0;
        VIR_TypeId      tyId;

        if (VIR_OPCODE_hasDest(opcode))
        {
            typeOpnd = VIR_Inst_GetDest(Inst);
        }
        else
        {
            if (VIR_OPCODE_useSrc2AsInstType(opcode))
            {
                typeOpnd = VIR_Inst_GetSource(Inst, 2);
            }
            else if (VIR_OPCODE_useSrc3AsInstType(opcode))
            {
                typeOpnd = VIR_Inst_GetSource(Inst, 3);
            }
            else
            {
                gcmASSERT(VIR_OPCODE_useSrc0AsInstType(opcode));
                typeOpnd = VIR_Inst_GetSource(Inst, 0);
            }
        }

        tyId = VIR_Operand_GetTypeId(typeOpnd);
        gcmASSERT(tyId < VIR_TYPE_PRIMITIVETYPE_COUNT);
        compCount = VIR_GetTypePackedComponents(tyId);

        for (i = 0; i < VIR_Inst_GetSrcNum(Inst); i++)
        {
            VIR_Operand *pOpnd = VIR_Inst_GetSource(Inst, i);
            VIR_EVIS_Modifier evisModifier;
            gctUINT binSize = 0;

            if (pOpnd && VIR_Operand_GetOpKind(pOpnd) == VIR_OPND_EVIS_MODIFIER)
            {
                evisModifier.u1 = VIR_Operand_GetEvisModifier(pOpnd);
                binSize = evisModifier.u0.endBin - evisModifier.u0.startBin;

                /* if the bin specifies all the dst will be written,
                this instr is a full write. Otherwise it is a conditional write */
                if (binSize != compCount - 1)
                {
                    return gcvTRUE;
                }
            }
        }
    }
    return gcvFALSE;
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

    if (VIR_OPCODE_hasStoreOperation(opcode))
    {
        /* in v60, USC has constraint. If src2 is immediate/uniform/indirect,
        there must be a store destination. the client needs to check v60,
        store includes: memory store, atomic, img_store */
        if (VIR_OPCODE_useSrc3AsInstType(opcode))
        {
            pOpnd = VIR_Inst_GetSource(Inst, 3);
        }
        else
        {
            pOpnd = VIR_Inst_GetSource(Inst, 2);
        }
        gcmASSERT(!VIR_Operand_isUndef(pOpnd));

        if (VIR_Operand_GetOpKind(pOpnd) == VIR_OPND_IMMEDIATE ||
            VIR_Operand_GetOpKind(pOpnd) == VIR_OPND_CONST)
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
            if (VIR_Symbol_GetKind(sym) == VIR_SYM_UNIFORM ||
                VIR_Symbol_GetKind(sym) == VIR_SYM_IMAGE)
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
    tyId = VIR_Operand_GetTypeId(operand);
    VIR_Operand_Copy(operand, FromDest);
    operand->header._lvalue = gcvTRUE;

    if (KeepOrgType)
    {
        VIR_Operand_SetTypeId(operand, tyId);
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
    tyId = VIR_Operand_GetTypeId(operand);
    index = VIR_Operand_GetIndex(operand);
    VIR_Operand_Copy(operand, FromOperand);
    VIR_Operand_SetIndex(operand, index);
    operand->header._lvalue = gcvFALSE;

    if (KeepSrcType)
    {
        VIR_Operand_SetTypeId(operand, tyId);
    }
    return errCode;
}

VSC_ErrCode
VIR_Inst_Copy(
    IN OUT VIR_Instruction * Dest,
    IN VIR_Instruction *     Source,
    IN gctBOOL               SameBB
    )
{
    VSC_ErrCode     errCode = VSC_ERR_NONE;
    gctUINT         i;

    Dest->sourceLoc = Source->sourceLoc;
    gcmASSERT(VIR_Inst_GetOpcode(Dest) == VIR_Inst_GetOpcode(Source));
    Dest->_isPrecise = Source->_isPrecise;
    VIR_Inst_SetPatched(Dest, VIR_Inst_GetPatched(Source));
    VIR_Inst_SetResOpType(Dest, VIR_Inst_GetResOpType(Source));
    VIR_Inst_SetConditionOp(Dest, VIR_Inst_GetConditionOp(Source));
    VIR_Inst_SetFlags(Dest, VIR_Inst_GetFlags(Source));
    VIR_Inst_SetSrcNum(Dest, VIR_Inst_GetSrcNum(Source));
    VIR_Inst_SetThreadMode(Dest, VIR_Inst_GetThreadMode(Source));

    VIR_Inst_CopySrcLoc(Source->sourceLoc, Dest->sourceLoc);

    if(SameBB && VIR_Inst_GetBasicBlock(Source))
    {
        VIR_Inst_SetBasicBlock(Dest, VIR_Inst_GetBasicBlock(Source));
    }
    gcmASSERT(VIR_Inst_GetInstType(Dest) == VIR_Inst_GetInstType(Source));

    if(VIR_Inst_GetDest(Source))
    {
        VIR_Operand_Copy(VIR_Inst_GetDest(Dest), VIR_Inst_GetDest(Source));
    }
    for(i = 0; i < VIR_Inst_GetSrcNum(Source); i++)
    {
        VIR_Operand_Copy(VIR_Inst_GetSource(Dest, i), VIR_Inst_GetSource(Source, i));
    }

    if(VIR_Inst_GetOpcode(Source) == VIR_OP_LABEL)
    {
        VIR_LabelId dupLabelId;
        VIR_Label* dupLabel;

        errCode = VIR_Function_DuplicateLabel(VIR_Inst_GetFunction(Dest), VIR_Operand_GetLabel(VIR_Inst_GetDest(Source)), &dupLabelId);
        if(errCode)
        {
            return errCode;
        }
        dupLabel = VIR_Function_GetLabelFromId(VIR_Inst_GetFunction(Dest), dupLabelId);
        VIR_Operand_SetLabel(VIR_Inst_GetDest(Dest), dupLabel);
        VIR_Label_SetDefInst(dupLabel, Dest);
    }
    else if(VIR_OPCODE_isBranch(VIR_Inst_GetOpcode(Source)))
    {
        if(VIR_Inst_GetFunction(Source) == VIR_Inst_GetFunction(Dest))
        {
            VIR_Label* label = VIR_Operand_GetLabel(VIR_Inst_GetDest(Source));
            VIR_Link* link;

            errCode = VIR_Function_NewLink(VIR_Inst_GetFunction(Source), &link);
            if(errCode)
            {
                return errCode;
            }
            VIR_Link_SetReference(link, (gctUINTPTR_T)Dest);
            VIR_Link_AddLink(VIR_Label_GetReferenceAddr(label), link);
        }
        else
        {
            WARNING_REPORT(errCode, "here we have a inter function branch");
        }
    }

    return errCode;
}

VSC_ErrCode
VIR_Inst_FreeSource(
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

VSC_ErrCode
VIR_Inst_FreeDest(
    IN OUT VIR_Instruction * Inst
    )
{
    VIR_Function *  function = VIR_Inst_GetFunction(Inst);
    VIR_Operand *   operand;

    gcmASSERT(function != gcvNULL);
    operand = VIR_Inst_GetDest(Inst);

    return VIR_Function_FreeOperand(function, operand);
}

void
VIR_Inst_ChangeDest(
    IN OUT VIR_Instruction * Inst,
    IN     VIR_Operand *     Dest
    )
{
        if (VIR_Inst_GetDest(Inst) != gcvNULL)
        {
            VIR_Inst_FreeDest((Inst));
        }
        VIR_Inst_SetDest((Inst), (Dest));
}

void
VIR_Inst_ChangeSource(
    IN OUT VIR_Instruction * Inst,
    IN     gctINT            SrcNo,
    IN     VIR_Operand *     Src
    )
{
    gcmASSERT(SrcNo < VIR_MAX_SRC_NUM);
    if (VIR_Inst_GetSource((Inst), (SrcNo)) != gcvNULL)
    {
        VIR_Inst_FreeSource((Inst), (SrcNo));
    }
    VIR_Inst_SetSource((Inst), (SrcNo), (Src));
}

void
VIR_Inst_ChangeSrcNum(
    IN OUT VIR_Instruction * Inst,
    IN     gctUINT           SrcNo
    )
{
    gcmASSERT(SrcNo < VIR_MAX_SRC_NUM);
    if (SrcNo < VIR_Inst_GetSrcNum((Inst)) )
    {
        gctUINT i;
        for (i = SrcNo; i<VIR_Inst_GetSrcNum(Inst); i++)
        {
            if (VIR_Inst_GetSource((Inst), i))
            {
                VIR_Inst_FreeSource((Inst), i);
                VIR_Inst_SetSource((Inst), i, gcvNULL);
            }
        }
    }
    VIR_Inst_SetSrcNum((Inst), SrcNo);
}

gctBOOL
VIR_Inst_IdenticalExpression(
    IN VIR_Instruction  *Inst0,
    IN VIR_Instruction  *Inst1,
    IN VIR_Shader       *Shader,
    IN gctBOOL          bPrecisionMatters,
    IN gctBOOL          bAllowCommutative
    )
{
    gctUINT             i;
    gctBOOL             bMatched = gcvTRUE;

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

    if (VIR_Operand_GetModifier(VIR_Inst_GetDest(Inst0)) != VIR_Operand_GetModifier(VIR_Inst_GetDest(Inst1)))
    {
        return gcvFALSE;
    }

    if(bPrecisionMatters &&
        VIR_Operand_GetPrecision(VIR_Inst_GetDest(Inst0)) != VIR_Operand_GetPrecision(VIR_Inst_GetDest(Inst1)))
    {
        return gcvFALSE;
    }

    /* Check all sources. */
    bMatched = gcvTRUE;
    for (i = 0; i < VIR_Inst_GetSrcNum(Inst0); i++)
    {
        if (!VIR_Operand_Identical(VIR_Inst_GetSource(Inst0, i), VIR_Inst_GetSource(Inst1, i), Shader))
        {
            bMatched = gcvFALSE;
            break;
        }
    }
    if (bMatched || !bAllowCommutative)
    {
        return bMatched;
    }

    /* If this OPCODE is commutative, check the commutative sources. */
    if (i < 2 && VIR_OPCODE_Src0Src1Commutative(VIR_Inst_GetOpcode(Inst0)))
    {
        /* Swap src0 and src1. */
        if (!VIR_Operand_Identical(VIR_Inst_GetSource(Inst0, 0), VIR_Inst_GetSource(Inst1, 1), Shader))
        {
            return gcvFALSE;
        }
        if (!VIR_Operand_Identical(VIR_Inst_GetSource(Inst0, 1), VIR_Inst_GetSource(Inst1, 0), Shader))
        {
            return gcvFALSE;
        }

        /* Check the left sources. */
        for (i = 2; i < VIR_Inst_GetSrcNum(Inst0); i++)
        {
            if (!VIR_Operand_Identical(VIR_Inst_GetSource(Inst0, i), VIR_Inst_GetSource(Inst1, i), Shader))
            {
                return gcvFALSE;
            }
        }

        return gcvTRUE;
    }

    return gcvFALSE;
}

VIR_TypeId
VIR_Inst_GetExpressionTypeID(
    IN VIR_Instruction  *Inst,
    IN VIR_Shader       *Shader
    )
{
    VIR_OpCode opcode = VIR_Inst_GetOpcode(Inst);
    VIR_Operand* dst = VIR_Inst_GetDest(Inst);
    VIR_TypeId dstTypeID = VIR_Operand_GetTypeId(dst);
    switch(opcode)
    {
    case VIR_OP_LOAD:
    case VIR_OP_LOAD_L:
    case VIR_OP_ATTR_LD:
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
            if(VIR_Inst_isComponentwise(Inst))
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

    VIR_Shader *pShader = VIR_Inst_GetShader(Inst);
    if (VIR_Shader_IsVulkan(pShader) || VIR_Shader_IsDesktopGL(pShader))
    {
        return result;
    }

    switch(VIR_OPCODE_ExpectedResultPrecision(opcode))
    {
    case VIR_OPFLAG_ExpdPrecFromHighest:
        {
            gctUINT i;
            for(i = 0; i < VIR_Inst_GetSrcNum(Inst); i++)
            {
                VIR_Operand* src = VIR_Inst_GetSource(Inst, i);
                VIR_Precision precision = (VIR_Precision)VIR_Operand_GetPrecision(src);

                gcmASSERT(precision != VIR_PRECISION_DEFAULT || !VIR_Shader_IsGPipe(VIR_Inst_GetShader(Inst)));
                if(precision > result)
                {
                    result = precision;
                }
            }
            break;
        }
    case VIR_OPFLAG_ExpdPrecFromSrc0:
        {
            gcmASSERT(VIR_Operand_GetPrecision(VIR_Inst_GetSource(Inst, 0)) != VIR_PRECISION_DEFAULT
                || !VIR_Shader_IsGPipe(VIR_Inst_GetShader(Inst)));
            result = VIR_Operand_GetPrecision(VIR_Inst_GetSource(Inst, 0));
            break;
        }
    case VIR_OPFLAG_ExpdPrecFromSrc2:
        {
            gcmASSERT(VIR_Operand_GetPrecision(VIR_Inst_GetSource(Inst, 2)) != VIR_PRECISION_DEFAULT
                || !VIR_Shader_IsGPipe(VIR_Inst_GetShader(Inst)));
            result = VIR_Operand_GetPrecision(VIR_Inst_GetSource(Inst, 2));
            break;
        }
    case VIR_OPFLAG_ExpdPrecFromSrc12:
        {
            VIR_Precision precision1 = VIR_Operand_GetPrecision(VIR_Inst_GetSource(Inst, 1));
            VIR_Precision precision2 = VIR_Operand_GetPrecision(VIR_Inst_GetSource(Inst, 2));

            gcmASSERT((precision1 != VIR_PRECISION_DEFAULT && precision2 != VIR_PRECISION_DEFAULT)
                || !VIR_Shader_IsGPipe(VIR_Inst_GetShader(Inst)));
            result = precision1 > precision2 ? precision1 : precision2;
            break;
        }
    case VIR_OPFLAG_ExpdPrecHP:
        {
            result = VIR_PRECISION_HIGH;
            break;
        }
    case VIR_OPFLAG_ExpdPrecMP:
        {
            result = VIR_PRECISION_MEDIUM;
            break;
        }
    default:
        result = VIR_PRECISION_DEFAULT;
        break;
    }

    return result;
}

void
VIR_Inst_InitMcInsts(
    IN VIR_Instruction  *Inst,
    IN VIR_Shader       *Shader,
    IN gctUINT          mcInstCount,
    IN gctINT32         mcInstPC,
    IN gctBOOL          bUpdatePC
    )
{
    if (Inst->mcInst != gcvNULL)
    {
        vscMM_Free(&Shader->pmp.mmWrapper, Inst->mcInst);
    }

    Inst->mcInstCount = mcInstCount;
    Inst->mcInst = (VSC_MC_RAW_INST *)vscMM_Alloc(&Shader->pmp.mmWrapper,
        sizeof(VSC_MC_RAW_INST) * Inst->mcInstCount);
    memset(Inst->mcInst, 0, sizeof(VSC_MC_RAW_INST) * Inst->mcInstCount);

    if (bUpdatePC)
    {
        VIR_Inst_SetMCInstPC(Inst, mcInstPC);
    }
}

VIR_Instruction*
VIR_Inst_GetJmpTarget(
    IN VIR_Instruction  *JmpInst
    )
{
    gcmASSERT(VIR_OPCODE_isBranch(VIR_Inst_GetOpcode(JmpInst)));

    return VIR_Label_GetDefInst(VIR_Operand_GetLabel(VIR_Inst_GetDest(JmpInst)));
}

void
VIR_Inst_ChangeJmpTarget(
    IN VIR_Instruction  *JmpInst,
    IN VIR_Instruction  *NewTargetInst)
{
    VIR_Label* oldLabel;
    VIR_Label* targetLabel;
    VIR_Function* func = VIR_Inst_GetFunction(JmpInst);
    VIR_Link* oldLink;
    VIR_Link *newLink = gcvNULL;

    gcmASSERT(VIR_OPCODE_isBranch(VIR_Inst_GetOpcode(JmpInst)));
    gcmASSERT(VIR_Inst_GetFunction(JmpInst) == VIR_Inst_GetFunction(NewTargetInst));

    /* No need to go on if new target is same as old one */
    if (VIR_Label_GetDefInst(VIR_Operand_GetLabel(VIR_Inst_GetDest(JmpInst))) == NewTargetInst)
    {
        return;
    }

    oldLabel = VIR_Operand_GetLabel(VIR_Inst_GetDest(JmpInst));
    oldLink = VIR_Link_RemoveLink(VIR_Label_GetReferenceAddr(oldLabel), (gctUINTPTR_T)JmpInst);
    VIR_Function_FreeLink(func, oldLink);

    if (VIR_Inst_GetOpcode(NewTargetInst) == VIR_OP_LABEL)
    {
        VIR_Operand* labelDest = VIR_Inst_GetDest(NewTargetInst);

        gcmASSERT((VIR_Operand_GetOpKind(labelDest) == VIR_OPND_LABEL));
        targetLabel = VIR_Operand_GetLabel(labelDest);
    }
    else
    {
        VIR_Instruction* labelInst;
        VIR_LabelId labelId;

        VIR_Function_AddLabel(func, gcvNULL, &labelId);
        VIR_Function_AddInstructionBefore(func, VIR_OP_LABEL, VIR_TYPE_UNKNOWN, NewTargetInst, gcvTRUE, &labelInst);
        targetLabel = VIR_GetLabelFromId(func, labelId);
        targetLabel->defined = labelInst;
        VIR_Operand_SetLabel(VIR_Inst_GetDest(labelInst), targetLabel);
    }

    VIR_Function_NewLink(func, &newLink);
    VIR_Link_SetReference(newLink, (gctUINTPTR_T)JmpInst);
    VIR_Link_AddLink(&(targetLabel->referenced), newLink);
    VIR_Operand_SetLabel(VIR_Inst_GetDest(JmpInst), targetLabel);
}

gctBOOL
VIR_Inst_CanGetConditionResult(
    IN VIR_Instruction *pInst
    )
{
    VIR_OpCode opcode = VIR_Inst_GetOpcode(pInst);

    if(VIR_OPCODE_useCondCode(opcode))
    {
        VIR_ConditionOp cop = VIR_Inst_GetConditionOp(pInst);

        if(cop == VIR_COP_ALWAYS)
        {
            return gcvTRUE;
        }
        else
        {
            VIR_Operand *src0 = VIR_Inst_GetSource(pInst, 0);

            if(VIR_ConditionOp_SingleOperand(cop))
            {
                return VIR_Operand_ContainsConstantValue(src0);
            }
            else
            {
                VIR_Operand *src1 = VIR_Inst_GetSource(pInst, 1);

                gcmASSERT(VIR_ConditionOp_DoubleOperand(cop));

                if(VIR_Operand_ContainsConstantValue(src0) &&
                   VIR_Operand_ContainsConstantValue(src1))
                {
                    return gcvTRUE;
                }

                if(VIR_ConditionOp_Reversable(cop) &&
                   VIR_Operand_Identical(src0, src1, VIR_Inst_GetShader(pInst)))
                {
                    if(VIR_TypeId_isFloat(VIR_Operand_GetTypeId(src0)))
                    {
                        if(cop == VIR_COP_EQUAL ||
                           cop == VIR_COP_GREATER_OR_EQUAL ||
                           cop == VIR_COP_LESS_OR_EQUAL ||
                           cop == VIR_COP_NOT_EQUAL_UQ ||
                           cop == VIR_COP_LESS_UQ ||
                           cop == VIR_COP_GREATER_UQ)
                        {
                            return gcvFALSE;
                        }
                    }

                    return gcvTRUE;
                }
            }
        }
    }

    return gcvFALSE;
}

gctBOOL
VIR_Inst_EvaluateConditionResult(
    IN VIR_Instruction  *pInst,
    OUT gctBOOL         *pChannelResults
    )
{
    VIR_Shader      *pShader = VIR_Inst_GetShader(pInst);
    VIR_OpCode      opc = VIR_Inst_GetOpcode(pInst);
    VIR_ConditionOp comp = VIR_Inst_GetConditionOp(pInst);
    gctBOOL         compDouble = VIR_ConditionOp_DoubleOperand(comp);
    VIR_Operand     *src0, *src1;

    gcmASSERT(VIR_Inst_CanGetConditionResult(pInst));


    src0 = VIR_Inst_GetSource(pInst, 0);
    src1 = VIR_Inst_GetSource(pInst, 1);

    if(VIR_Operand_ContainsConstantValue(src0))
    {
        VIR_Type        *src0Type, *src1Type;
        VIR_TypeId      src0TypeId, src1TypeId = VIR_TYPE_UNKNOWN;
        VIR_Swizzle     src0Swizzle, src1Swizzle = VIR_SWIZZLE_XXXX;
        gctBOOL         checkingResult[VIR_CHANNEL_COUNT] = { gcvTRUE, gcvTRUE, gcvTRUE, gcvTRUE};
        gctUINT         i;
        gctBOOL         branchAny = (opc == VIR_OP_JMP_ANY);

        /*
        ** For conditional branch, HW's normal branch is actually branch_all,
        ** however when condition opcode is NE, the logic should be branch_any.
        */
        if (opc == VIR_OP_JMPC &&
            VIR_Inst_GetConditionOp(pInst) == VIR_COP_NOT_EQUAL)
        {
            branchAny = gcvTRUE;
        }

        src0Swizzle = VIR_Operand_GetSwizzle(src0);
        src0Type = VIR_Shader_GetTypeFromId(pShader, VIR_Operand_GetTypeId(src0));
        src0TypeId = VIR_GetTypeComponentType(VIR_Type_GetBaseTypeId(src0Type));

        if(compDouble)
        {
            src1Swizzle = VIR_Operand_GetSwizzle(src1);
            src1Type = VIR_Shader_GetTypeFromId(pShader, VIR_Operand_GetTypeId(src1));
            src1TypeId = VIR_GetTypeComponentType(VIR_Type_GetBaseTypeId(src1Type));
        }

        /* Do the evaluation for all channels. */

        for (i = 0; i < VIR_CHANNEL_COUNT; i++)
        {
            gctUINT src0Val = VIR_Operand_ExtractOneChannelConstantValue(src0, pShader, VIR_Swizzle_GetChannel(src0Swizzle, i), gcvNULL);
            gctUINT src1Val = compDouble ?
                              VIR_Operand_ExtractOneChannelConstantValue(src1, pShader, VIR_Swizzle_GetChannel(src1Swizzle, i), gcvNULL) :
                              0;

            checkingResult[i] = VIR_ConditionOp_EvaluateOneChannelConstantCondition(VIR_Inst_GetConditionOp(pInst),
                                                                                    src0Val,
                                                                                    src0TypeId,
                                                                                    src1Val,
                                                                                    src1TypeId);
        }

        if(pChannelResults)
        {
            gcoOS_MemCopy(pChannelResults, checkingResult, sizeof(checkingResult));
        }

        if (branchAny || VIR_Inst_GetConditionOp(pInst) == VIR_COP_ANYMSB)
        {
            return checkingResult[0] | checkingResult[1] | checkingResult[2] | checkingResult[3];
        }
        else
        {
            return checkingResult[0] & checkingResult[1] & checkingResult[2] & checkingResult[3];
        }
    }
    else
    {
        gcmASSERT(VIR_Operand_isSymbol(src0) && VIR_Operand_Identical(src0, src1, pShader));

        switch (comp)
        {
        case VIR_COP_EQUAL:
        case VIR_COP_EQUAL_UQ:
        case VIR_COP_GREATER_OR_EQUAL:
        case VIR_COP_GREATER_OR_EQUAL_UQ:
        case VIR_COP_LESS_OR_EQUAL:
        case VIR_COP_LESS_OR_EQUAL_UQ:
            if(pChannelResults)
            {
                pChannelResults[0] = gcvTRUE;
                pChannelResults[1] = gcvTRUE;
                pChannelResults[2] = gcvTRUE;
                pChannelResults[3] = gcvTRUE;
            }
            return gcvTRUE;
        default:
            if(pChannelResults)
            {
                pChannelResults[0] = gcvFALSE;
                pChannelResults[1] = gcvFALSE;
                pChannelResults[2] = gcvFALSE;
                pChannelResults[3] = gcvFALSE;
            }
            return gcvFALSE;
        }
    }
}

gctBOOL
VIR_Inst_CanGetConstantResult(
    IN VIR_Instruction *pInst
    )
{
    VIR_OpCode opcode = VIR_Inst_GetOpcode(pInst);
    gctUINT i;

    switch(opcode)
    {
    case VIR_OP_ADD:
    case VIR_OP_DIV:
    case VIR_OP_MUL:
    case VIR_OP_SUB:
        break;
    default:
        return gcvFALSE;
    }

    for(i = 0; i < VIR_Inst_GetSrcNum(pInst); i++)
    {
        VIR_Operand* src = VIR_Inst_GetSource(pInst, i);

        if(!VIR_Operand_ContainsConstantValue(src))
        {
            break;
        }
    }
    if(i < VIR_Inst_GetSrcNum(pInst))
    {
        return gcvFALSE;
    }

    return gcvTRUE;
}

void
VIR_Inst_EvaluateConstantResult(
    IN VIR_Instruction *pInst,
    OUT gctUINT *pConstResults
    )
{
    gctUINT i;
    VIR_Shader* shader = VIR_Inst_GetShader(pInst);
    VIR_OpCode opcode = VIR_Inst_GetOpcode(pInst);
    VIR_Operand* dest = VIR_Inst_GetDest(pInst);
    VIR_Operand* src0 = VIR_Inst_GetSource(pInst, 0);
    VIR_TypeId src0TypeId = VIR_GetTypeComponentType(VIR_Operand_GetTypeId(src0));
    VIR_Operand* src1 = VIR_Inst_GetSource(pInst, 1);
    VIR_TypeId src1TypeId = VIR_GetTypeComponentType(VIR_Operand_GetTypeId(src1));

    gcmASSERT(VIR_Inst_CanGetConstantResult(pInst));

    if(VIR_OPCODE_isComponentwise(opcode))
    {
        VIR_Enable enable = VIR_Operand_GetEnable(dest);

        for(i = 0; i < VIR_CHANNEL_NUM; i++)
        {
            if(enable & (1 << i))
            {
                pConstResults[i] = VIR_OpCode_EvaluateOneChannelConstant(opcode,
                                                                         VIR_Operand_ExtractOneChannelConstantValue(src0, shader, i, gcvNULL),
                                                                         src0TypeId,
                                                                         VIR_Operand_ExtractOneChannelConstantValue(src1, shader, i, gcvNULL),
                                                                         src1TypeId,
                                                                         gcvNULL);
            }
        }
    }
}

void
VIR_Inst_CheckAndSetPakedMode(
    IN OUT VIR_Instruction  * Inst
    )
{
    VIR_OpCode   opCode = VIR_Inst_GetOpcode(Inst);
    gctBOOL      isPackedMode = gcvFALSE;
    if (VIR_Inst_isComponentwise(Inst))
    {
        if (VIR_OPCODE_hasDest(opCode))
        {
            VIR_Operand * dest = VIR_Inst_GetDest(Inst);
            VIR_TypeId tyId = VIR_Operand_GetTypeId(dest);
            gcmASSERT(VIR_OPCODE_hasDest(opCode) && VIR_TypeId_isPrimitive(tyId));
            /* if the dest is Packed type, the inst must be in packed mode */
            if (VIR_TypeId_isPacked(tyId))
            {
                isPackedMode = gcvTRUE;
                {
                    gctUINT i;
                    for (i = 0; i < VIR_Inst_GetSrcNum(Inst); i++)
                    {
                        VIR_Operand * opnd;
                        /* check operand for immediate value and adjust it to proper type
                         * so it can be set as 0x3 */
                        opnd = VIR_Inst_GetSource(Inst, i);
                        if (VIR_Operand_GetOpKind(opnd) == VIR_OPND_IMMEDIATE)
                        {
                            VIR_Operand_AdjustPackedImmValue(opnd, tyId);
                        }
                    }
                }
            }
        }
    }
    else
    {
        if (VIR_OPCODE_useCondCode(opCode))
        {
            /* ALLMSB/ANYMSB is not componentwise */
            VIR_ConditionOp condOp = VIR_Inst_GetConditionOp(Inst);
            if (condOp == VIR_COP_ANYMSB ||condOp == VIR_COP_ALLMSB)
            {
                VIR_Operand * src0  = VIR_Inst_GetSource(Inst, 0);
                VIR_TypeId    tyId0 = VIR_Operand_GetTypeId(src0);
                isPackedMode = VIR_TypeId_isPacked(tyId0);
            }
        }
    }

    if (isPackedMode)
    {
        VIR_Inst_SetFlag(Inst, VIR_INSTFLAG_PACKEDMODE);
    }
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

VIR_Swizzle VIR_Swizzle_Extract_Single_Channel_Swizzle(
    IN VIR_Swizzle orgSwizzle,
    IN gctUINT     channel)
{
    VIR_Swizzle result = VIR_SWIZZLE_XYZW;
    int i;

    for(i = 0; i < VIR_CHANNEL_COUNT; i++)
    {
        VIR_Swizzle_SetChannel(result, i, VIR_Swizzle_GetChannel(orgSwizzle, channel));
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

VIR_Swizzle
VIR_Swizzle_GenSwizzleByComponentCount(
    IN gctUINT ComponentCount
    )
{
    VIR_Swizzle result          = VIR_SWIZZLE_X;

    switch (ComponentCount)
    {
    case 32:
    case 16:
    case 8:
    case 4:
        result  = VIR_SWIZZLE_XYZW;
        break;

    case 3:
        result  = VIR_SWIZZLE_XYZZ;
        break;

    case 2:
        result  = VIR_SWIZZLE_XYYY;
        break;

    case 1:
        result  = VIR_SWIZZLE_XXXX;
        break;

    default:
        gcmASSERT(gcvFALSE);
        break;
    }

    return result;
}

VIR_Swizzle
VIR_Swizzle_SwizzleWShiftEnable(
    IN VIR_Swizzle swizzle,
    IN VIR_Enable enable
    )
{
    VIR_Swizzle result = swizzle;
    int i, j;

    for (i = 0; i < VIR_CHANNEL_COUNT; i++)
    {
        if (enable & (1 << i))
        {
            for(j = 0; j < i; j++)
            {
                VIR_Swizzle_SetChannel(result, j, VIR_Swizzle_GetChannel(swizzle, 0));
            }

            for(j = i; j < VIR_CHANNEL_COUNT; j++)
            {
                VIR_Swizzle_SetChannel(result, j, VIR_Swizzle_GetChannel(swizzle, j - i));
            }
            break;
        }
    }
    return result;
}

VIR_Swizzle
VIR_Swizzle_ComposeSwizzle(
    IN VIR_Swizzle channelX,
    IN VIR_Swizzle channelY,
    IN VIR_Swizzle channelZ,
    IN VIR_Swizzle channelW
    )
{
    VIR_Swizzle result = VIR_SWIZZLE_XYZW;

    VIR_Swizzle_SetChannel(result, 0, channelX);
    VIR_Swizzle_SetChannel(result, 1, channelY);
    VIR_Swizzle_SetChannel(result, 2, channelZ);
    VIR_Swizzle_SetChannel(result, 3, channelW);

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

VIR_Swizzle
VIR_Enable_GetMappingFullChannelSwizzle(
    IN VIR_Enable enable,
    IN VIR_Swizzle swizzle
    )
{
    VIR_Swizzle result = swizzle;
    gctUINT8 set[VIR_CHANNEL_COUNT] = { 0 };
    gctUINT8 i, j, lastChannel = 0;

    for (i = 0; i < VIR_CHANNEL_COUNT; i++)
    {
        if (enable & (1 << i))
        {
            lastChannel = VIR_Swizzle_GetChannel(swizzle, i);
            for (j = 0; j <= i; j++)
            {
                if (!set[j])
                {
                    VIR_Swizzle_SetChannel(result, j, lastChannel);
                    set[j] = 1;
                }
            }
        }
    }

    for (i = 0; i < VIR_CHANNEL_COUNT; i++)
    {
        if (!set[i])
        {
            VIR_Swizzle_SetChannel(result, i, lastChannel);
            set[i] = 1;
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

gctSTRING
VIR_Enable_2_String(
    IN VIR_Enable enable,
    IN gctBOOL upppercase
    )
{
    static gctSTRING uNames[1 << VIR_CHANNEL_NUM] = {
        "NONE",
        "X",
        "Y",
        "XY",
        "Z",
        "XZ",
        "YZ",
        "XYZ",
        "W",
        "XW",
        "YW",
        "XYW",
        "ZW",
        "XZW",
        "YZW",
        "XYZW"
    };

    static gctSTRING lNames[1 << VIR_CHANNEL_NUM] = {
        "none",
        "x",
        "y",
        "xy",
        "z",
        "xz",
        "yz",
        "xyz",
        "w",
        "xw",
        "yw",
        "xyw",
        "zw",
        "xzw",
        "yzw",
        "xyzw"
    };

    gcmASSERT(enable > 0 && enable < (1 << VIR_CHANNEL_NUM));

    return upppercase ? uNames[enable] : lNames[enable];
}

static gctBOOL _NeedIndexRange(
    IN  VIR_Instruction *   Inst,
    IN  VIR_Operand *       Operand,
    IN  gctUINT *           ConstRegOffset
    )
{
    VIR_Shader *            pShader = VIR_Inst_GetShader(Inst);
    gctBOOL                 result = gcvFALSE;
    gctUINT                 constRegOffset = 0;

    if ((VIR_Inst_GetOpcode(Inst) == VIR_OP_LDARR && Operand == VIR_Inst_GetSource(Inst, 0))
        ||
        (VIR_Inst_GetOpcode(Inst) == VIR_OP_STARR && Operand == VIR_Inst_GetDest(Inst)))
    {
        result = gcvTRUE;
    }
    if (VIR_Inst_GetOpcode(Inst) == VIR_OP_COPY)
    {
        VIR_Operand * src1 = VIR_Inst_GetSource(Inst, 1); /* src1 is the byte size */
        gctUINT     copySize;
        gcmASSERT(VIR_Operand_GetOpKind(src1) == VIR_OPND_IMMEDIATE);

        copySize = VIR_Operand_GetImmediateUint(src1);
        result =  copySize > 16;
    }

    if ((VIR_Inst_GetOpcode(Inst) == VIR_OP_ATTR_LD && Operand == VIR_Inst_GetSource(Inst, 0)))
    {
        VIR_Operand *       offsetOperand = VIR_Inst_GetSource(Inst, 2);
        VIR_Type *          offsetType = VIR_Shader_GetTypeFromId(pShader, VIR_Operand_GetTypeId(offsetOperand));
        VIR_Const *         constVal = gcvNULL;

        if (VIR_Operand_GetOpKind(offsetOperand) != VIR_OPND_IMMEDIATE &&
            VIR_Operand_GetOpKind(offsetOperand) != VIR_OPND_CONST)
        {
            result = gcvTRUE;
        }
        else if (VIR_TypeId_isInteger(VIR_Type_GetBaseTypeId(offsetType)))
        {
            if (VIR_Operand_GetOpKind(offsetOperand) == VIR_OPND_IMMEDIATE)
            {
                constRegOffset = VIR_Operand_GetImmediateUint(offsetOperand);
            }
            else
            {
                constVal = VIR_Shader_GetConstFromId(pShader, VIR_Operand_GetConstId(offsetOperand));
                constRegOffset = constVal->value.vecVal.u32Value[VIR_Operand_GetSwizzle(offsetOperand)];
            }
        }
    }

    /* Save the result. */
    if (ConstRegOffset)
    {
        *ConstRegOffset = constRegOffset;
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
    gctUINT             constRegOffset = 0;
    gctBOOL             needIndexRange = gcvFALSE;

    gcmASSERT(Info != gcvNULL);
    gcmASSERT(VIR_Operand_IsOwnerInst(Operand, Inst));
    memset(Info, 0, sizeof(VIR_OperandInfo));

    Info->opnd  = Operand;
    Info->indexingVirRegNo = VIR_INVALID_ID;
    Info->u1.virRegInfo.virReg = VIR_INVALID_ID;

    /* Check if we need index range. */
    needIndexRange = _NeedIndexRange(Inst, Operand, &constRegOffset);

    Info->halfChannelMask = VIR_HALF_CHANNEL_MASK_FULL;
    Info->halfChannelMaskOfIndexingVirRegNo = VIR_HALF_CHANNEL_MASK_FULL;

    if (opndKind == VIR_OPND_SYMBOL || opndKind == VIR_OPND_SAMPLER_INDEXING)
    {
        sym = VIR_Operand_GetSymbol(Operand);
        if (VIR_Symbol_GetKind(sym) == VIR_SYM_VIRREG)
        {
            Info->u1.virRegInfo.virReg = VIR_Symbol_GetVregIndex(sym);
            Info->u1.virRegInfo.virRegWithOffset = Info->u1.virRegInfo.virReg + constRegOffset;
            Info->isVreg = 1;
            virregSym = sym;
            sym = VIR_Symbol_GetVregVariable(sym); /* set the sym to corresponding variable */
        }
        else if (VIR_Symbol_isVariable(sym))
        {
            Info->u1.virRegInfo.virReg = VIR_Symbol_GetVariableVregIndex(sym);
            Info->u1.virRegInfo.virRegWithOffset = Info->u1.virRegInfo.virReg + constRegOffset;
            Info->isVreg = 1;
        }
        else if (VIR_Symbol_isUniform(sym))
        {
            Info->isUniform = 1;
            Info->u1.uniformIdx = sym->u2.uniform->index;
        }
        else if (VIR_Symbol_isImage(sym))
        {
            Info->u1.symId = VIR_Symbol_GetIndex(sym);
            Info->isImage = 1;
        }
        else if (VIR_Symbol_isImageT(sym))
        {
            Info->u1.symId = VIR_Symbol_GetIndex(sym);
            Info->isImageT = 1;
        }
        else if (VIR_Symbol_isSampler(sym))
        {
            Info->u1.symId = VIR_Symbol_GetIndex(sym);
            Info->isSampler = 1;
        }
        else if (VIR_Symbol_isSamplerT(sym))
        {
            Info->u1.symId = VIR_Symbol_GetIndex(sym);
            Info->isSamplerT = 1;
        }
        else if (VIR_Symbol_isTexure(sym))
        {
            Info->u1.symId = VIR_Symbol_GetIndex(sym);
            Info->isTexture = 1;
        }
        else
        {
            Info->u1.virRegInfo.virReg = VIR_INVALID_ID;
        }

        if (Info->isVreg)
        {
            if (sym != gcvNULL)
            {
                if (VIR_Symbol_isVariable(sym))
                {
                    /* for now, only indirect access needs multiple consecutive registers
                    others (e.g., matrix, uchar_x16 ...) don't need, since they are already
                    lowered to individual temp */
                    if (needIndexRange)
                    {
                        Info->u1.virRegInfo.startVirReg   = VIR_Symbol_GetVariableVregIndex(sym);
                        Info->u1.virRegInfo.virRegCount =  VIR_Symbol_GetIndexRange(sym) ?
                            VIR_Symbol_GetIndexRange(sym) - Info->u1.virRegInfo.startVirReg :
                            VIR_Type_GetVirRegCount(Shader, VIR_Symbol_GetType(sym), -1);
                    }
                    else
                    {
                        Info->u1.virRegInfo.startVirReg   = Info->u1.virRegInfo.virReg;
                        Info->u1.virRegInfo.virRegCount = 1;
                    }
                }
                else
                {
                    /* get the variable of struct type which this field is in */
                    gcmASSERT(VIR_Symbol_isField(sym) && virregSym != gcvNULL);

                    if (needIndexRange)
                    {
                        Info->u1.virRegInfo.startVirReg = Info->u1.virRegInfo.virReg - VIR_Symbol_GetOffsetInVar(virregSym);
                        Info->u1.virRegInfo.virRegCount = VIR_Symbol_GetIndexRange(virregSym) - Info->u1.virRegInfo.startVirReg;

                        /* to remove !! */
                        if(Info->u1.virRegInfo.virRegCount & 0x80000000)
                        {
                            Info->u1.virRegInfo.virRegCount = 1;
                        }
                    }
                    else
                    {
                        Info->u1.virRegInfo.startVirReg = Info->u1.virRegInfo.virReg;
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
                Info->isPerPrim = VIR_Operand_IsPerPatch(Operand);
                Info->isPerVtxCp = VIR_Operand_IsArrayedPerVertex(Operand);
                Info->isTempVar     = 0;
                Info->isImmVal      = 0;
                Info->isOutputParm  = VIR_Symbol_isOutParamVirReg(sym) || VIR_Symbol_isOutParam(sym);
            }
            else
            {
                /* is temp variable which has no coresponding user defined variable  */
                Info->u1.virRegInfo.startVirReg   = Info->u1.virRegInfo.virReg;
                Info->isArray = Info->isInput = Info->isOutput = Info->isPerPrim = Info->isPerVtxCp = 0;
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
    VIR_Operand_SetTypeId(Operand, VIR_Symbol_GetTypeId(sym));
    VIR_Operand_SetSym(Operand, sym);
    VIR_Operand_SetPrecision(Operand, VIR_Symbol_GetPrecision(sym));
}

void
VIR_Operand_SetImmediate(
    IN OUT VIR_Operand*    Operand,
    IN  VIR_TypeId         Type,
    IN  VIR_ScalarConstVal Immed
    )
{
    VIR_Operand_SetOpKind(Operand, VIR_OPND_IMMEDIATE);
    VIR_Operand_SetTypeId(Operand, Type);
    VIR_Operand_SetPrecision(Operand, VIR_PRECISION_HIGH);
    VIR_Operand_SetImmUint(Operand, Immed.uValue);
}

void
VIR_Operand_SetConst(
    IN OUT VIR_Operand *Operand,
    IN  VIR_TypeId      Type,
    IN  VIR_ConstId     ConstId
    )
{
    VIR_Operand_SetOpKind(Operand, VIR_OPND_CONST);
    VIR_Operand_SetTypeId(Operand, Type);
    VIR_Operand_SetPrecision(Operand, VIR_PRECISION_HIGH);
    VIR_Operand_SetConstId(Operand, ConstId);
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
    VIR_Operand_SetTypeId(Operand, VIR_Type_GetIndex(VIR_Symbol_GetType(sym)));
    VIR_Operand_SetSym(Operand, sym);
}

void
VIR_Operand_SetParameters(
    IN OUT VIR_Operand *Operand,
    IN  VIR_ParmPassing *Parms
    )
{
    VIR_Operand_SetOpKind(Operand, VIR_OPND_PARAMETERS);
    VIR_Operand_SetTypeId(Operand, VIR_TYPE_UNKNOWN);
    VIR_Operand_SetParams(Operand, Parms);
}

void
VIR_Operand_SetLabel(
    IN OUT VIR_Operand *    Operand,
    IN  VIR_Label *         Label
    )
{
    VIR_Operand_SetOpKind(Operand, VIR_OPND_LABEL);
    VIR_Operand_SetTypeId(Operand, VIR_TYPE_UNKNOWN);
    Operand->u.n.u1.label  = Label;
}

void
VIR_Operand_SetImmediateInt(
    IN OUT VIR_Operand *    Operand,
    IN gctINT              Val
    )
{
    VIR_Operand_SetOpKind(Operand, VIR_OPND_IMMEDIATE);
    VIR_Operand_SetTypeId(Operand, VIR_TYPE_INT32);
    VIR_Operand_SetPrecision(Operand, VIR_PRECISION_HIGH);
    VIR_Operand_SetSwizzle(Operand, VIR_SWIZZLE_XXXX);
    VIR_Operand_SetImmInt(Operand, Val);

    /* Clean vlinfo. */
    VIR_Operand_CleanVlInfo(Operand);
}

void
VIR_Operand_SetImmediateUint(
    IN OUT VIR_Operand *    Operand,
    IN gctUINT              Val
    )
{
    VIR_Operand_SetOpKind(Operand, VIR_OPND_IMMEDIATE);
    VIR_Operand_SetTypeId(Operand, VIR_TYPE_UINT32);
    VIR_Operand_SetPrecision(Operand, VIR_PRECISION_HIGH);
    VIR_Operand_SetSwizzle(Operand, VIR_SWIZZLE_XXXX);
    VIR_Operand_SetImmUint(Operand, Val);

    /* Clean vlinfo. */
    VIR_Operand_CleanVlInfo(Operand);
}

void
VIR_Operand_SetImmediateBoolean(
    IN OUT VIR_Operand *    Operand,
    IN gctUINT              Val
    )
{
    VIR_Operand_SetOpKind(Operand, VIR_OPND_IMMEDIATE);
    VIR_Operand_SetTypeId(Operand, VIR_TYPE_BOOLEAN);
    VIR_Operand_SetPrecision(Operand, VIR_PRECISION_HIGH);
    VIR_Operand_SetSwizzle(Operand, VIR_SWIZZLE_XXXX);
    VIR_Operand_SetImmUint(Operand, Val);

    /* Clean vlinfo. */
    VIR_Operand_CleanVlInfo(Operand);
}

void
VIR_Operand_SetImmediateFloat(
    IN OUT VIR_Operand *    Operand,
    IN gctFLOAT             Val
    )
{
    VIR_Operand_SetOpKind(Operand, VIR_OPND_IMMEDIATE);
    VIR_Operand_SetTypeId(Operand, VIR_TYPE_FLOAT32);
    VIR_Operand_SetPrecision(Operand, VIR_PRECISION_HIGH);
    VIR_Operand_SetSwizzle(Operand, VIR_SWIZZLE_XXXX);
    VIR_Operand_SetImmFloat(Operand, Val);

    /* Clean vlinfo. */
    VIR_Operand_CleanVlInfo(Operand);
}

void
VIR_Operand_SetTexldBias(
    IN OUT VIR_Operand *    Operand,
    IN  VIR_Operand    *    Bias
    )
{
    VIR_Operand * texldModifier = (VIR_Operand *)Operand;
    gcmASSERT(VIR_Operand_GetOpKind(Operand) == VIR_OPND_UNDEF ||
        VIR_Operand_GetOpKind(Operand) == VIR_OPND_TEXLDPARM);
    VIR_Operand_SetOpKind(Operand, VIR_OPND_TEXLDPARM);
    VIR_Operand_SetTexModifierFlag(texldModifier, VIR_TMFLAG_BIAS);
    VIR_Operand_SetTexldModifier(texldModifier, VIR_TEXLDMODIFIER_BIAS, Bias);
}

void
VIR_Operand_SetTexldLod(
    IN OUT VIR_Operand *    Operand,
    IN  VIR_Operand    *    Lod
    )
{
    VIR_Operand * texldModifier = (VIR_Operand *)Operand;
    gcmASSERT(VIR_Operand_GetOpKind(Operand) == VIR_OPND_UNDEF ||
        VIR_Operand_GetOpKind(Operand) == VIR_OPND_TEXLDPARM);
    VIR_Operand_SetOpKind(Operand, VIR_OPND_TEXLDPARM);
    VIR_Operand_SetTexModifierFlag(texldModifier, VIR_TMFLAG_LOD);
    VIR_Operand_SetTexldModifier(texldModifier, VIR_TEXLDMODIFIER_LOD, Lod);
}

void
VIR_Operand_SetTexldGradient(
    IN OUT VIR_Operand *    Operand,
    IN  VIR_Operand    *    Pdx,
    IN  VIR_Operand    *    Pdy
    )
{
    VIR_Operand * texldModifier = (VIR_Operand *)Operand;
    gcmASSERT(VIR_Operand_GetOpKind(Operand) == VIR_OPND_UNDEF ||
        VIR_Operand_GetOpKind(Operand) == VIR_OPND_TEXLDPARM);
    VIR_Operand_SetOpKind(Operand, VIR_OPND_TEXLDPARM);
    VIR_Operand_SetTexModifierFlag(texldModifier, VIR_TMFLAG_GRAD);
    VIR_Operand_SetTexldModifier(texldModifier, VIR_TEXLDMODIFIER_DPDX, Pdx);
    VIR_Operand_SetTexldModifier(texldModifier, VIR_TEXLDMODIFIER_DPDY, Pdy);
}

void
VIR_Operand_SetTexldGradientDx(
    IN OUT VIR_Operand *    Operand,
    IN  VIR_Operand    *    Pdx
    )
{
    VIR_Operand * texldModifier = (VIR_Operand *)Operand;
    gcmASSERT(VIR_Operand_GetOpKind(Operand) == VIR_OPND_UNDEF ||
        VIR_Operand_GetOpKind(Operand) == VIR_OPND_TEXLDPARM);
    VIR_Operand_SetOpKind(Operand, VIR_OPND_TEXLDPARM);
    VIR_Operand_SetTexModifierFlag(texldModifier, VIR_TMFLAG_GRAD);
    VIR_Operand_SetTexldModifier(texldModifier, VIR_TEXLDMODIFIER_DPDX, Pdx);
}

void
VIR_Operand_SetTexldGradientDy(
    IN OUT VIR_Operand *    Operand,
    IN  VIR_Operand    *    Pdy
    )
{
    VIR_Operand * texldModifier = (VIR_Operand *)Operand;
    gcmASSERT(VIR_Operand_GetOpKind(Operand) == VIR_OPND_UNDEF ||
        VIR_Operand_GetOpKind(Operand) == VIR_OPND_TEXLDPARM);
    VIR_Operand_SetOpKind(Operand, VIR_OPND_TEXLDPARM);
    VIR_Operand_SetTexModifierFlag(texldModifier, VIR_TMFLAG_GRAD);
    VIR_Operand_SetTexldModifier(texldModifier, VIR_TEXLDMODIFIER_DPDY, Pdy);
}

void
VIR_Operand_SetTexldOffset(
    IN OUT VIR_Operand *    Operand,
    IN  VIR_Operand    *    Offset
    )
{
    VIR_Operand * texldModifier = (VIR_Operand *)Operand;
    gcmASSERT(VIR_Operand_GetOpKind(Operand) == VIR_OPND_UNDEF ||
        VIR_Operand_GetOpKind(Operand) == VIR_OPND_TEXLDPARM);
    VIR_Operand_SetOpKind(Operand, VIR_OPND_TEXLDPARM);
    VIR_Operand_SetTexModifierFlag(texldModifier, VIR_TMFLAG_OFFSET);
    VIR_Operand_SetTexldModifier(texldModifier, VIR_TEXLDMODIFIER_OFFSET, Offset);
}

void
VIR_Operand_SetTexldGather(
    IN OUT VIR_Operand *    Operand,
    IN  VIR_Operand    *    Component,
    IN  VIR_Operand    *    RefZ
    )
{
    VIR_Operand * texldModifier = (VIR_Operand *)Operand;
    gcmASSERT(VIR_Operand_GetOpKind(Operand) == VIR_OPND_UNDEF ||
        VIR_Operand_GetOpKind(Operand) == VIR_OPND_TEXLDPARM);
    VIR_Operand_SetOpKind(Operand, VIR_OPND_TEXLDPARM);
    VIR_Operand_SetTexModifierFlag(texldModifier, VIR_TMFLAG_GATHER);
    VIR_Operand_SetTexldModifier(texldModifier, VIR_TEXLDMODIFIER_GATHERCOMP, Component);
    VIR_Operand_SetTexldModifier(texldModifier, VIR_TEXLDMODIFIER_GATHERREFZ, RefZ);
}

void
VIR_Operand_SetTexldGatherComp(
    IN OUT VIR_Operand *    Operand,
    IN  VIR_Operand    *    Component
    )
{
    VIR_Operand * texldModifier = (VIR_Operand *)Operand;
    gcmASSERT(VIR_Operand_GetOpKind(Operand) == VIR_OPND_UNDEF ||
        VIR_Operand_GetOpKind(Operand) == VIR_OPND_TEXLDPARM);
    VIR_Operand_SetOpKind(Operand, VIR_OPND_TEXLDPARM);
    VIR_Operand_SetTexModifierFlag(texldModifier, VIR_TMFLAG_GATHER);
    VIR_Operand_SetTexldModifier(texldModifier, VIR_TEXLDMODIFIER_GATHERCOMP, Component);
}

void
VIR_Operand_SetTexldGatherRefZ(
    IN OUT VIR_Operand *    Operand,
    IN  VIR_Operand    *    RefZ
    )
{
    VIR_Operand * texldModifier = (VIR_Operand *)Operand;
    gcmASSERT(VIR_Operand_GetOpKind(Operand) == VIR_OPND_UNDEF ||
        VIR_Operand_GetOpKind(Operand) == VIR_OPND_TEXLDPARM);
    VIR_Operand_SetOpKind(Operand, VIR_OPND_TEXLDPARM);
    VIR_Operand_SetTexModifierFlag(texldModifier, VIR_TMFLAG_GATHER);
    VIR_Operand_SetTexldModifier(texldModifier, VIR_TEXLDMODIFIER_GATHERREFZ, RefZ);
}

void
VIR_Operand_SetTexldFetchMS(
    IN OUT VIR_Operand *    Operand,
    IN  VIR_Operand    *    Sample
    )
{
    VIR_Operand * texldModifier = (VIR_Operand *)Operand;
    gcmASSERT(VIR_Operand_GetOpKind(Operand) == VIR_OPND_UNDEF ||
        VIR_Operand_GetOpKind(Operand) == VIR_OPND_TEXLDPARM);
    VIR_Operand_SetOpKind(Operand, VIR_OPND_TEXLDPARM);
    VIR_Operand_SetTexModifierFlag(texldModifier, VIR_TMFLAG_FETCHMS);
    VIR_Operand_SetTexldModifier(texldModifier, VIR_TEXLDMODIFIER_FETCHMS_SAMPLE, Sample);
}

void
VIR_Operand_SetFunction(
    IN OUT VIR_Operand *Operand,
    IN  VIR_Function *  Function
    )
{
    VIR_Operand_SetOpKind(Operand, VIR_OPND_FUNCTION);
    VIR_Operand_SetTypeId(Operand, VIR_TYPE_UNKNOWN);
    VIR_Operand_SetFunc(Operand, Function);
}

void
VIR_Operand_SetName(
    IN OUT VIR_Operand *    Operand,
    IN  VIR_NameId          Name
    )
{
    VIR_Operand_SetOpKind(Operand, VIR_OPND_NAME);
    VIR_Operand_SetTypeId(Operand, VIR_TYPE_UNKNOWN);
    Operand->u.n.u1.name  = Name;
}

void
VIR_Operand_SetIntrinsic(
    IN OUT VIR_Operand *    Operand,
    IN  VIR_IntrinsicsKind  Intrinsic
    )
{
    VIR_Operand_SetOpKind(Operand, VIR_OPND_INTRINSIC);
    VIR_Operand_SetTypeId(Operand, VIR_TYPE_UNKNOWN);
    Operand->u.n.u1.intrinsic  = Intrinsic;
}

void
VIR_Operand_SetFieldAccess(
    IN OUT VIR_Operand *Operand,
    IN  VIR_Operand *   Base,
    IN  VIR_SymId       FieldId
    )
{
    VIR_Operand_SetOpKind(Operand, VIR_OPND_FIELD);
    VIR_Operand_SetTypeId(Operand, VIR_TYPE_UNKNOWN);
    Operand->u.n.u1.base    = Base;
    Operand->u.n.u2.fieldId = FieldId;
}

void
VIR_Operand_SetArrayIndexing(
    IN OUT VIR_Operand * Operand,
    IN  VIR_Operand *   Base,
    IN  VIR_OperandList* ArrayIndex
    )
{
    VIR_Operand_SetOpKind(Operand, VIR_OPND_ARRAY);
    VIR_Operand_SetTypeId(Operand, VIR_TYPE_UNKNOWN);
    Operand->u.n.u1.base    = Base;
    Operand->u.n.u2.arrayIndex  = ArrayIndex;
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
    IN     VIR_TypeId       OperandType
    )
{
    VIR_Symbol * sym    = VIR_Function_GetSymFromId(Function, TempSymId);
    VIR_Operand_SetOpKind(Operand, VIR_OPND_SYMBOL);
    VIR_Operand_SetTypeId(Operand, OperandType);
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
    Operand->u.n._swizzleOrEnable  = (gctUINT)Swizzle;
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
        Operand->u.n._swizzleOrEnable  = (gctUINT)Swizzle;
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
    Operand->u.n._swizzleOrEnable  = (gctUINT)Enable;
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
        Operand->u.n._swizzleOrEnable  = (gctUINT)Enable;
    }
}

void
VIR_Operand_SetRelIndexing(
    IN OUT VIR_Operand *Operand,
    IN VIR_SymId        IndexSym
    )
{
    gcmASSERT(!VIR_Operand_isHighLevel(Operand));
    Operand->u.n.u2.vlInfo._isConstIndexing = 0;
    Operand->u.n.u2.vlInfo._relIndexing = IndexSym;
}

void
VIR_Operand_SetRelIndexingImmed(
    IN OUT VIR_Operand *Operand,
    IN gctINT           IndexImmed
    )
{
    gcmASSERT(!VIR_Operand_isHighLevel(Operand));
    gcmASSERT((IndexImmed < INT20_MAX) && (IndexImmed > INT20_MIN));

    Operand->u.n.u2.vlInfo._isConstIndexing = 1;
    Operand->u.n.u2.vlInfo._relIndexing = IndexImmed;
}

gctBOOL
VIR_Operand_IsNegatable(
    IN  VIR_Shader *        Shader,
    IN  VIR_Operand *       Operand)
{
    VIR_TypeId tid;
    VIR_Type* type;

    tid = VIR_Operand_GetTypeId(Operand);
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
    /* HW takes absolute values first, then takes negative, we need to follow this order. */
    gctBOOL                 bHasAbs = VIR_Operand_GetModifier(Operand) & VIR_MOD_ABS;

    switch(VIR_Operand_GetOpKind(Operand))
    {
    case VIR_OPND_IMMEDIATE:
        {
            VIR_PrimitiveTypeId type = VIR_Operand_GetTypeId(Operand);
            if (bHasAbs)
            {
                VIR_ScalarConstVal_GetAbs(type, &VIR_Operand_GetScalarImmediate(Operand), &VIR_Operand_GetScalarImmediate(Operand));
                VIR_Operand_SetModifier(Operand, VIR_MOD_ABS ^ VIR_Operand_GetModifier(Operand));
            }
            VIR_ScalarConstVal_GetNeg(type, &VIR_Operand_GetScalarImmediate(Operand), &VIR_Operand_GetScalarImmediate(Operand));
            break;
        }

    case VIR_OPND_SYMBOL:
        if(VIR_Operand_GetModifier(Operand) & VIR_MOD_NEG)
        {
            VIR_Operand_SetModifier(Operand, VIR_MOD_NEG ^ VIR_Operand_GetModifier(Operand));
            VIR_Operand_SetModOrder(Operand, VIR_MODORDER_NONE);
        }
        else
        {
            VIR_Operand_SetModifier(Operand, VIR_MOD_NEG | VIR_Operand_GetModifier(Operand));
            if (bHasAbs)
            {
                VIR_Operand_SetModOrder(Operand, VIR_MODORDER_ABS_NEG);
            }
        }
        break;

    case VIR_OPND_CONST:
        {
            VIR_Const* cur_const = (VIR_Const *)VIR_Shader_GetConstFromId(Shader,
                                                    VIR_Operand_GetConstId(Operand));
            VIR_ConstVal new_const;
            VIR_ConstId new_const_id;

            memset(&new_const, 0, sizeof(VIR_ConstVal));

            if (bHasAbs)
            {
                VIR_VecConstVal_GetAbs(cur_const->type, &cur_const->value.vecVal, &new_const.vecVal);
                VIR_VecConstVal_GetNeg(cur_const->type, &new_const.vecVal, &new_const.vecVal);
                VIR_Operand_SetModifier(Operand, VIR_MOD_ABS ^ VIR_Operand_GetModifier(Operand));
            }
            else
            {
                VIR_VecConstVal_GetNeg(cur_const->type, &cur_const->value.vecVal, &new_const.vecVal);
            }
            VIR_Shader_AddConstant(Shader, cur_const->type, &new_const, &new_const_id);
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
VIR_Operand_SameLocationByEnable(
    IN  VIR_Instruction *   Inst1,
    IN  VIR_Operand *       Operand1,
    IN  VIR_Enable          Enable,
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
        enable1 = Enable;
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
VIR_Operand_SameIndexedSymbol(
    IN VIR_Operand  *Opnd0,
    IN VIR_Operand  *Opnd1
    )
{
    if(VIR_Operand_SameSymbol(Opnd0, Opnd1))
    {
        if(VIR_Operand_GetMatrixConstIndex(Opnd0) != VIR_Operand_GetMatrixConstIndex(Opnd1))
        {
            return gcvFALSE;
        }

        if(VIR_Operand_GetIsConstIndexing(Opnd0))
        {
            if(VIR_Operand_GetIsConstIndexing(Opnd1))
            {
                gctUINT constIndexingImm0 = VIR_Operand_GetConstIndexingImmed(Opnd0);
                gctUINT constIndexingImm1 = VIR_Operand_GetConstIndexingImmed(Opnd1);

                if(constIndexingImm0 != constIndexingImm1)
                {
                    return gcvFALSE;
                }
            }
            else
            {
                return gcvFALSE;
            }
        }
        else if(VIR_Operand_GetIsConstIndexing(Opnd1))
        {
            return gcvFALSE;
        }
        else
        {
            gctUINT relIndexingMode0 = VIR_Operand_GetRelAddrMode(Opnd0);
            gctUINT relIndexingMode1 = VIR_Operand_GetRelAddrMode(Opnd1);

            if(relIndexingMode0 != relIndexingMode1)
            {
                return gcvFALSE;
            }
            else if(relIndexingMode0 != VIR_INDEXED_NONE)
            {
                gctUINT relIndexingSymId0 = VIR_Operand_GetRelIndexing(Opnd0);
                gctUINT relIndexingSymId1 = VIR_Operand_GetRelIndexing(Opnd1);

                if(relIndexingSymId0 != relIndexingSymId1)
                {
                    return gcvFALSE;
                }
            }
        }
    }
    else
    {
        return gcvFALSE;
    }

    return gcvTRUE;
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

    if (VIR_Operand_GetModifier(Opnd0) != VIR_Operand_GetModifier(Opnd1))
    {
        return gcvFALSE;
    }

    if (VIR_Operand_GetLShift(Opnd0) != VIR_Operand_GetLShift(Opnd1))
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
                gctBOOL sameSwizzleEnable = gcvTRUE;

                if(VIR_Operand_isLvalue(Opnd0) && VIR_Operand_isLvalue(Opnd1))
                {
                    sameSwizzleEnable = (VIR_Operand_GetEnable(Opnd0) == VIR_Operand_GetEnable(Opnd1));
                }
                else if(VIR_Operand_isLvalue(Opnd0) && !VIR_Operand_isLvalue(Opnd1))
                {
                    sameSwizzleEnable = (VIR_Operand_GetEnable(Opnd0) == VIR_Swizzle_2_Enable(VIR_Operand_GetSwizzle(Opnd1)));
                }
                else if(VIR_Operand_isLvalue(Opnd1) && !VIR_Operand_isLvalue(Opnd0))
                {
                    sameSwizzleEnable = (VIR_Operand_GetEnable(Opnd1) == VIR_Swizzle_2_Enable(VIR_Operand_GetSwizzle(Opnd0)));
                }
                else
                {
                    sameSwizzleEnable = (VIR_Operand_GetSwizzle(Opnd0) == VIR_Operand_GetSwizzle(Opnd1));
                }

                return sym0 == sym1 &&
                    sameSwizzleEnable &&
                    VIR_Operand_GetConstIndexingImmed(Opnd0) == VIR_Operand_GetConstIndexingImmed(Opnd1) &&
                    VIR_Operand_GetMatrixConstIndex(Opnd0) == VIR_Operand_GetMatrixConstIndex(Opnd1) &&
                    VIR_Operand_GetRelAddrMode(Opnd0) == VIR_Operand_GetRelAddrMode(Opnd1) &&
                    VIR_Operand_GetRelIndexing(Opnd0) == VIR_Operand_GetRelIndexing(Opnd1);
            }
        case VIR_OPND_IMMEDIATE:
            {
                return VIR_Operand_GetTypeId(Opnd0) == VIR_Operand_GetTypeId(Opnd1) &&
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

                if(VIR_GetTypeComponentType(VIR_Operand_GetTypeId(Opnd0)) != VIR_GetTypeComponentType(VIR_Operand_GetTypeId(Opnd1)))
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
            if(VIR_Operand_GetTypeId(immOpnd) != VIR_GetTypeComponentType(VIR_Operand_GetTypeId(constOpnd)))
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
VIR_Operand_Change2Src_WShift(
    IN OUT VIR_Operand*     Operand)
{
    VIR_Enable enable;
    VIR_Swizzle swizzle;

    if(!VIR_Operand_isLvalue(Operand))
    {
        return;
    }

    enable = VIR_Operand_GetEnable(Operand);
    swizzle = VIR_Enable_2_Swizzle_WShift(enable);
    VIR_Operand_SetLvalue(Operand, gcvFALSE);
    VIR_Operand_SetSwizzle(Operand, swizzle);
}

void
VIR_Operand_Copy(
    IN OUT VIR_Operand*     Dest,
    IN VIR_Operand*         Source)
{
    gctUINT index;

    gcmASSERT(Dest && Source);
    index = VIR_Operand_GetIndex(Dest);
    memcpy(Dest, Source, sizeof(VIR_Operand));
    VIR_Operand_SetIndex(Dest, index);
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
    VIR_Operand_Copy(Def, New_Def);
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

    VIR_Operand_Copy(Use, Def);
    VIR_Operand_SetLvalue(Use, gcvFALSE);
    VIR_Operand_SetSwizzle(Use, VIR_Enable_2_Swizzle(VIR_Operand_GetEnable(Def)));
}

void
VIR_Operand_ReplaceUseOperandWithUse(
    IN OUT VIR_Operand *    Tgt_Use,
    IN VIR_Operand *        New_Use,
    IN VIR_Swizzle          New_Swizzle
    )
{
    gcmASSERT(!VIR_Operand_isLvalue(Tgt_Use) && !VIR_Operand_isLvalue(New_Use));

    VIR_Operand_Copy(Tgt_Use, New_Use);
    VIR_Operand_ShrinkSwizzle(Tgt_Use, New_Swizzle);
}

static gctBOOL
_IsOwnerSourceOperand(
    IN VIR_Operand *     Operand,
    IN VIR_Operand *     SourceOperand
    )
{
    gctUINT i;

    if (SourceOperand == gcvNULL)
    {
        return gcvFALSE;
    }

    if (Operand == SourceOperand)
    {
        return gcvTRUE;
    }
    /* texld parameters and function parameters can be fixed. */
    else if (VIR_Operand_isTexldParm(SourceOperand))
    {
        VIR_Operand *TexldOperand = (VIR_Operand*)SourceOperand;

        for (i = 0; i < VIR_TEXLDMODIFIER_COUNT; i++)
        {
            if (_IsOwnerSourceOperand(Operand, VIR_Operand_GetTexldModifier(TexldOperand, i)))
            {
                return gcvTRUE;
            }
        }
    }
    else if (VIR_Operand_isParameters(SourceOperand))
    {
        VIR_ParmPassing *parm = VIR_Operand_GetParameters(SourceOperand);

        for (i = 0; i < parm->argNum; i++)
        {
            if (_IsOwnerSourceOperand(Operand, parm->args[i]))
            {
                return gcvTRUE;
            }
        }
    }

    return gcvFALSE;
}

gctBOOL
VIR_Operand_IsOwnerInst(
    IN VIR_Operand *     Operand,
    IN VIR_Instruction * Inst
    )
{
    gctUINT i;

    if (VIR_Operand_isLvalue(Operand))
    {
        return (VIR_Inst_GetDest(Inst) == Operand);
    }
    else
    {
        for (i = 0; i < VIR_Inst_GetSrcNum(Inst); i ++)
        {
            VIR_Operand * opnd = VIR_Inst_GetSource(Inst, i);

            if (_IsOwnerSourceOperand(Operand, opnd))
            {
                return gcvTRUE;
            }
        }
        return gcvFALSE;
    }
}

gctBOOL
VIR_Operand_ContainsConstantValue(
    IN VIR_Operand *     Operand
    )
{
    if(Operand == gcvNULL)
    {
        return gcvFALSE;
    }

    return VIR_Operand_isConst(Operand) ||
           VIR_Operand_isImm(Operand) ||
           (VIR_Operand_isSymbol(Operand) &&
            VIR_Symbol_isUniform(VIR_Operand_GetSymbol(Operand)) &&
            isSymUniformCompiletimeInitialized(VIR_Operand_GetSymbol(Operand)));
}

gctUINT
VIR_Operand_ExtractOneChannelConstantValue(
    IN VIR_Operand      *pOpnd,
    IN VIR_Shader       *pShader,
    IN gctUINT          Channel,
    OUT VIR_TypeId      *pTypeId
    )
{
    gctUINT result = 0;

    gcmASSERT(VIR_Operand_ContainsConstantValue(pOpnd));
    gcmASSERT(Channel < VIR_CHANNEL_NUM);

    if (VIR_Operand_isImm(pOpnd))
    {
        result = VIR_Operand_GetImmediateUint(pOpnd);
    }
    else if (VIR_Operand_isSymbol(pOpnd))
    {
        VIR_Symbol *uniformSym = VIR_Operand_GetSymbol(pOpnd);
        VIR_Uniform *uniform = VIR_Symbol_GetUniform(uniformSym);
        VIR_Type *symType;
        VIR_ConstId constId;
        VIR_Const *pConst;

        gcmASSERT(uniform && isSymUniformCompiletimeInitialized(uniformSym));
        symType = VIR_Symbol_GetType(uniformSym);
        if(VIR_Type_isArray(symType)) {
            gctINT arrayIndex;
            gcmASSERT(VIR_Operand_GetRelAddrMode(pOpnd) == VIR_INDEXED_NONE);

            arrayIndex = VIR_Operand_GetConstIndexingImmed(pOpnd) +
                         VIR_Operand_GetMatrixConstIndex(pOpnd);
            constId = *(VIR_Uniform_GetInitializerPtr(uniform) + arrayIndex);
        }
        else {
            constId = VIR_Uniform_GetInitializer(uniform);
        }

        pConst = VIR_Shader_GetConstFromId(pShader, constId);

        result = pConst->value.vecVal.u32Value[VIR_Swizzle_GetChannel(VIR_Operand_GetSwizzle(pOpnd), Channel)];
    }
    else if (VIR_Operand_isConst(pOpnd))
    {
        VIR_Const *pConst = VIR_Shader_GetConstFromId(pShader, VIR_Operand_GetConstId(pOpnd));

        result = pConst->value.vecVal.u32Value[VIR_Swizzle_GetChannel(VIR_Operand_GetSwizzle(pOpnd), Channel)];
    }

    if(pTypeId)
    {
        *pTypeId = VIR_GetTypeComponentType(VIR_Operand_GetTypeId(pOpnd));
    }

    return result;
}

/* check operand for immediate value and adjust it to proper type
 * so it can be set as 0x3 */
void VIR_Operand_AdjustPackedImmValue(
    IN VIR_Operand *     Opnd,
    IN VIR_TypeId        PackedTyId
    )
{
    VIR_TypeId opndTyId = VIR_Operand_GetTypeId(Opnd);
    gcmASSERT (VIR_Operand_GetOpKind(Opnd) == VIR_OPND_IMMEDIATE);
    gcmASSERT(VIR_TypeId_isPrimitive(opndTyId));
    if (!VIR_TypeId_isPacked(opndTyId))
    {
        if (!(VIR_TypeId_isInteger(opndTyId) && VIR_TypeId_isInteger(PackedTyId)) &&
                !(VIR_TypeId_isFloat(opndTyId) && VIR_TypeId_isFloat(PackedTyId)) )
        {
            /* need to convert the immediate to same format as dest type */
            if (VIR_TypeId_isSignedInteger(opndTyId))
            {
                VIR_Operand_SetImmediateFloat(Opnd, (gctFLOAT)VIR_Operand_GetImmediateInt(Opnd));
            }
            else if (VIR_TypeId_isUnSignedInteger(opndTyId) || VIR_TypeId_isBoolean(opndTyId))
            {
                VIR_Operand_SetImmediateFloat(Opnd, (gctFLOAT)VIR_Operand_GetImmediateUint(Opnd));
            }
            else
            {
                /* convert float value to int */
                VIR_Operand_SetImmediateInt(Opnd, (gctINT)VIR_Operand_GetImmediateFloat(Opnd));
            }
        }
        /* make the immediate operand type the same as dest type */
        VIR_Operand_SetTypeId(Opnd, PackedTyId);
    }
    else
    {
        /* immediate value is already packed type */
    }
}

/* return encoded value (each channel 5 bits) if the constant values fit into 5 bits,
* otherwise return 0xFFFFFFFF */
gctUINT
VIR_Const_EncodeValueIn5Bits(
    VIR_Const *      pConstVal
    )
{
    gctUINT imm = 0;
    VIR_TypeId tyId  = pConstVal->type;
    if (VIR_TypeId_isPrimitive(tyId))
    {
        gctINT components = VIR_GetTypeComponents(tyId);
        int i;
        if (components > 4)
            return 0xFFFFFFFF;

        for (i = 0; i < components; i++)
        {
            if (VIR_TypeId_isSignedInteger(tyId))
            {
                gctINT value = pConstVal->value.vecVal.i32Value[i];
                if (!((value >= 16) || (value < -16)))
                {
                    imm = (imm | (value & 0x1f) << (5*i));
                }
                else
                {
                    return 0xFFFFFFFF;
                }
            }
            else if (VIR_TypeId_isUnSignedInteger(tyId))
            {
                gctUINT value = pConstVal->value.vecVal.u32Value[i];
                if ((value < 16))
                {
                    imm = (imm | (value & 0x0f) << (5*i));
                }
                else
                {
                    return 0xFFFFFFFF;
                }
            }
            else
            {
                return 0xFFFFFFFF;
            }
        }
        /* all values are in 5 bits range */
    }
    return imm;
}

gctBOOL
VIR_Const_isValueZero(
    VIR_Const *      pConstVal
    )
{
    VIR_TypeId tyId  = pConstVal->type;
    if (VIR_TypeId_isPrimitive(tyId))
    {
        gctINT components = VIR_GetTypeComponents(tyId);
        int i;

        for (i = 0; i < components; i++)
        {
            if (VIR_TypeId_isSignedInteger(tyId))
            {
                gctINT value = pConstVal->value.vecVal.i32Value[i];
                if (value != 0)
                {
                    return gcvFALSE;
                }
            }
            else if (VIR_TypeId_isUnSignedInteger(tyId))
            {
                gctUINT value = pConstVal->value.vecVal.u32Value[i];
                if (value != 0)
                {
                    return gcvFALSE;
                }
            }
            else if (VIR_TypeId_isFloat(tyId))
            {
                gctFLOAT value = pConstVal->value.vecVal.f32Value[i];
                if (value != 0.0)
                {
                    return gcvFALSE;
                }
            }
        }
        /* all values are 0 */
        return gcvTRUE;
    }
    return gcvFALSE;
}
gctBOOL
VIR_Const_isValueFit5Bits(
    VIR_Const *      pConstVal
    )
{
    VIR_TypeId tyId  = pConstVal->type;
    if (VIR_TypeId_isPrimitive(tyId))
    {
        gctINT components = VIR_GetTypeComponents(tyId);
        int i;
        if (components > 4)
            return gcvFALSE;

        for (i = 0; i < components; i++)
        {
            if (VIR_TypeId_isSignedInteger(tyId))
            {
                gctINT value = pConstVal->value.vecVal.i32Value[i];
                if ((value >= 16) || (value < -16))
                {
                    return gcvFALSE;
                }
            }
            else if (VIR_TypeId_isUnSignedInteger(tyId))
            {
                gctUINT value = pConstVal->value.vecVal.u32Value[i];
                if ((value >= 16))
                {
                    return gcvFALSE;
                }
            }
        }
        /* all values are in 5 bits range */
        return gcvTRUE;
    }
    return gcvFALSE;
}

gctBOOL
VIR_Operand_isValueFit5Bits(
    IN VIR_Shader *        Shader,
    IN VIR_Operand *       Opnd
    )
{
    VIR_TypeId    tyId;
    if (VIR_Operand_GetOpKind(Opnd) == VIR_OPND_IMMEDIATE)
    {
        tyId = VIR_Operand_GetTypeId(Opnd);
        if (VIR_TypeId_isPrimitive(tyId))
        {
            if (VIR_TypeId_isSignedInteger(tyId))
            {
                gctINT value = VIR_Operand_GetImmediateInt(Opnd);
                return (value < 16) && (value >= -16);
            }
            else if (VIR_TypeId_isUnSignedInteger(tyId))
            {
                gctUINT uValue = VIR_Operand_GetImmediateUint(Opnd);
                return (uValue < 16);
            }
        }
    }
    else
    {
        if (VIR_Operand_GetOpKind(Opnd) == VIR_OPND_CONST)
        {
            VIR_ConstId constID = VIR_Operand_GetConstId(Opnd);
            VIR_Const*  cValue  = VIR_Shader_GetConstFromId(Shader, constID);
            return VIR_Const_isValueFit5Bits(cValue);
        }
        else if (VIR_Operand_isSymbol(Opnd))
        {
            VIR_Symbol *  sym = VIR_Operand_GetSymbol(Opnd);
            if (VIR_Symbol_isUniform(sym) && isSymUniformCompiletimeInitialized(sym) &&
                VIR_Operand_GetRelAddrMode(Opnd) == VIR_INDEXED_NONE)
            {
                VIR_Type *symType;
                VIR_ConstId constId;
                VIR_Const * cValue;

                symType = VIR_Symbol_GetType(sym);
                if(VIR_Type_isArray(symType)) {
                    gctINT arrayIndex;

                    arrayIndex = VIR_Operand_GetConstIndexingImmed(Opnd) +
                                 VIR_Operand_GetMatrixConstIndex(Opnd);
                    constId = *(VIR_Uniform_GetInitializerPtr(sym->u2.uniform) + arrayIndex);
                }
                else {
                    constId = VIR_Uniform_GetInitializer(sym->u2.uniform);
                }
                cValue = (VIR_Const *)VIR_GetSymFromId(&Shader->constTable,
                                                       constId);
                return VIR_Const_isValueFit5Bits(cValue);
            }
        }
    }
    return gcvFALSE;
}

gctBOOL
VIR_Operand_isValueZero(
    IN VIR_Shader *        Shader,
    IN VIR_Operand *       Opnd
    )
{
    VIR_TypeId    tyId;
    if (VIR_Operand_GetOpKind(Opnd) == VIR_OPND_IMMEDIATE)
    {
        tyId = VIR_Operand_GetTypeId(Opnd);
        if (VIR_TypeId_isPrimitive(tyId))
        {
            if (VIR_TypeId_isSignedInteger(tyId))
            {
                return VIR_Operand_GetImmediateInt(Opnd) == 0 ;
            }
            else if (VIR_TypeId_isUnSignedInteger(tyId))
            {
                return (VIR_Operand_GetImmediateUint(Opnd) == 0);
            }
            else if (VIR_TypeId_isFloat(tyId))
            {
                return (VIR_Operand_GetImmediateFloat(Opnd) == 0.0);
            }
        }
    }
    else
    {
        if (VIR_Operand_GetOpKind(Opnd) == VIR_OPND_CONST)
        {
            VIR_ConstId constID = VIR_Operand_GetConstId(Opnd);
            VIR_Const*  cValue  = VIR_Shader_GetConstFromId(Shader, constID);
            return VIR_Const_isValueZero(cValue);
        }
        else if (VIR_Operand_isSymbol(Opnd))
        {
            VIR_Symbol *  sym = VIR_Operand_GetSymbol(Opnd);
            if (VIR_Symbol_isUniform(sym) && isSymUniformCompiletimeInitialized(sym) &&
                VIR_Operand_GetRelAddrMode(Opnd) == VIR_INDEXED_NONE)
            {
                VIR_Type *symType;
                VIR_ConstId constId;
                VIR_Const * cValue;

                symType = VIR_Symbol_GetType(sym);
                if(VIR_Type_isArray(symType)) {
                    gctINT arrayIndex;

                    arrayIndex = VIR_Operand_GetConstIndexingImmed(Opnd) +
                                 VIR_Operand_GetMatrixConstIndex(Opnd);
                    constId = *(VIR_Uniform_GetInitializerPtr(sym->u2.uniform) + arrayIndex);
                }
                else {
                    constId = VIR_Uniform_GetInitializer(sym->u2.uniform);
                }
                cValue = (VIR_Const *)VIR_GetSymFromId(&Shader->constTable,
                                                       constId);
                return VIR_Const_isValueZero(cValue);
            }
        }
    }
    return gcvFALSE;
}

VSC_ErrCode
VIR_Operand_ReplaceSymbol(
    IN  VIR_Shader         *pShader,
    IN  VIR_Function       *pFunc,
    IN  VIR_Operand        *pOpnd,
    IN  VIR_Symbol         *pOrigSym,
    IN  VIR_Symbol         *pNewSym
    )
{
    VSC_ErrCode             errCode = VSC_ERR_NONE;
    VIR_Symbol             *sym = gcvNULL;
    gctUINT                 i;

    gcmASSERT(VIR_Symbol_GetTypeId(pOrigSym) == VIR_Symbol_GetTypeId(pNewSym));

    if (VIR_Operand_isParameters(pOpnd))
    {
        VIR_ParmPassing    *parm = VIR_Operand_GetParameters(pOpnd);
        for (i = 0; i < parm->argNum; i++)
        {
            errCode = VIR_Operand_ReplaceSymbol(pShader,
                                                pFunc,
                                                parm->args[i],
                                                pOrigSym,
                                                pNewSym);
            ON_ERROR(errCode, "Replace symbol");
        }
        return errCode;
    }
    else if (VIR_Operand_isTexldParm(pOpnd))
    {
        VIR_Operand *texldOperand = (VIR_Operand*)pOpnd;

        for (i = 0; i < VIR_TEXLDMODIFIER_COUNT; ++i)
        {
            errCode = VIR_Operand_ReplaceSymbol(pShader,
                                                pFunc,
                                                VIR_Operand_GetTexldModifier(texldOperand, i),
                                                pOrigSym,
                                                pNewSym);
            ON_ERROR(errCode, "Replace symbol");
        }
        return errCode;
    }

    /* Replace the symbol. */
    if (VIR_Operand_isSymbol(pOpnd))
    {
        sym = VIR_Operand_GetSymbol(pOpnd);

        if (sym == pOrigSym)
        {
            VIR_Operand_SetSym(pOpnd, pNewSym);
        }
    }

    /* Replace the index symbol. */
    if (VIR_Operand_GetRelAddrMode(pOpnd) != VIR_INDEXED_NONE)
    {
        VIR_Symbol *indexSym = VIR_Function_GetSymFromId(pFunc, VIR_Operand_GetRelIndexing(pOpnd));

        if (indexSym == pOrigSym)
        {
            VIR_Operand_SetRelIndexing(pOpnd, VIR_Symbol_GetIndex(pNewSym));
        }
    }

OnError:
    return errCode;
}

VIR_ConstId
VIR_Operand_GetConstValForUniform(
    IN  VIR_Shader         *pShader,
    IN  VIR_Operand        *pOpnd,
    IN  VIR_Symbol         *pUniformSym,
    IN  VIR_Uniform        *pUniform,
    IN  gctUINT             arrayOffset
    )
{
    VIR_ConstId             constId;
    VIR_Type               *pSymType;
    VIR_Type               *pBaseType;
    gctUINT                 arrayIndex;
    gctUINT                 index;

    gcmASSERT(pUniform && isSymUniformCompiletimeInitialized(pUniformSym));

    pBaseType = pSymType = VIR_Symbol_GetType(pUniformSym);

    gcmASSERT(arrayOffset == 0 || VIR_Type_isArray(pBaseType));

    while (VIR_Type_isArray(pBaseType))
    {
        pBaseType = VIR_Shader_GetTypeFromId(pShader, VIR_Type_GetBaseTypeId(pBaseType));
    }

    index = VIR_Operand_GetConstIndexingImmed(pOpnd) +
            VIR_Operand_GetMatrixConstIndex(pOpnd);
    if (VIR_Type_isArray(pSymType))
    {
        gctUINT rows = VIR_GetTypeRows(VIR_Type_GetIndex(pBaseType));

        arrayIndex = index / rows;
        constId = *(VIR_Uniform_GetInitializerPtr(pUniform) + arrayIndex + arrayOffset);
        index -= arrayIndex * 2;
    }
    else
    {
        constId = VIR_Uniform_GetInitializer(pUniform);
    }
    /*
    ** If it is a ulong/long, in MC level, it should be convert to a uint32/int32,
    ** we need to convert it to a uint32/int32 based on the indexing.
    */
    if(VIR_GetTypeSize(VIR_GetTypeComponentType(VIR_Type_GetIndex(pBaseType))) >= 8)
    {
        VIR_Const       *pConst;
        VIR_ConstVal    constVal;
        gctUINT64       data;
        gctINT          components;

        pConst = VIR_Shader_GetConstFromId(pShader, constId);
        pSymType = VIR_Shader_GetTypeFromId(pShader, pConst->type);
        arrayIndex = VIR_Operand_GetConstIndexingImmed(pOpnd) & 0x1;

        components = VIR_GetTypeLogicalComponents(pConst->type);
        if (VIR_Type_isVector(pSymType))
        {
            gctINT    i, j;
            gctINT    opndComponents = VIR_GetTypeLogicalComponents(VIR_Operand_GetTypeId(pOpnd));

            i = (index >> 1) * VIR_CHANNEL_COUNT;
            components -= i;
            gcmASSERT(components > 0);

            if(opndComponents > VIR_CHANNEL_COUNT)
            {
                if(components > opndComponents) components = opndComponents;
            }
            else
            {
                if(components > VIR_CHANNEL_COUNT) components = VIR_CHANNEL_COUNT;
            }

            for (j = 0; j < components; i++, j++)
            {
                if (VIR_TypeId_isUnSignedInteger(VIR_Type_GetIndex(pBaseType)))
                {
                    data = pConst->value.vecVal.u64Value[i];
                }
                else
                {
                    data = (gctUINT64)pConst->value.vecVal.i64Value[i];
                }

                if (arrayIndex == 0)
                {
                    constVal.vecVal.u32Value[j] = (gctUINT)(data& 0xFFFFFFFF);
                }
                else
                {
                    constVal.vecVal.u32Value[j] = (gctUINT)((data >> 32) & 0xFFFFFFFF);
                }
            }
        }
        else
        {
            if (VIR_TypeId_isUnSignedInteger(VIR_Type_GetIndex(pSymType)))
            {
                data = pConst->value.scalarVal.ulValue;
            }
            else
            {
                data = (gctUINT64)pConst->value.scalarVal.lValue;
            }

            if (arrayIndex == 0)
            {
                constVal.scalarVal.uValue = (gctUINT)(data & 0xFFFFFFFF);
            }
            else
            {
                constVal.scalarVal.uValue = (gctUINT)((data >> 32) & 0xFFFFFFFF);
            }
        }

        VIR_Shader_AddConstant(pShader,
                               VIR_TypeId_ComposeNonOpaqueType(VIR_TYPE_UINT32,
                                                               components,
                                                               1),
                               &constVal,
                               &constId);
    }

    return constId;
}

/* return true if the Opnd is fit into 5 bit offset and be changed to
* encoded 5bit offsets */
gctBOOL
VIR_IMG_LOAD_SetImmOffset(
    IN VIR_Shader *        Shader,
    IN VIR_Instruction *   Inst,
    IN VIR_Operand *       Opnd,
    IN gctBOOL             Encoded
    )
{
    VIR_TypeId    tyId;
    gctUINT       imm = 0;

    gcmASSERT(VIR_OPCODE_isImgLd(VIR_Inst_GetOpcode(Inst)));

    if (VIR_Operand_GetOpKind(Opnd) == VIR_OPND_IMMEDIATE)
    {
        tyId = VIR_Operand_GetTypeId(Opnd);
        if (VIR_TypeId_isPrimitive(tyId))
        {
            if (VIR_TypeId_isSignedInteger(tyId))
            {
                gctINT value = VIR_Operand_GetImmediateInt(Opnd);
                if (Encoded)
                {
                    imm = value;
                }
                else if ((value < 16) && (value >= -16))
                {
                    imm = ((value & 0x1f) << 5) | (value & 0x1f);
                }
            }
            else if (VIR_TypeId_isUnSignedInteger(tyId))
            {
                gctUINT uValue = VIR_Operand_GetImmediateUint(Opnd);
                if (Encoded)
                {
                    imm = uValue;
                }
                else if (uValue < 16)
                {
                    imm = ((uValue & 0x0f) << 5) | (uValue & 0x0f);
                }
            }
        }
    }
    else
    {
        if (VIR_Operand_GetOpKind(Opnd) == VIR_OPND_CONST)
        {
            VIR_ConstId constID = VIR_Operand_GetConstId(Opnd);
            VIR_Const*  cValue  = VIR_Shader_GetConstFromId(Shader, constID);
            imm = VIR_Const_EncodeValueIn5Bits(cValue);
        }
        else if (VIR_Operand_isSymbol(Opnd))
        {
            VIR_Symbol *  sym = VIR_Operand_GetSymbol(Opnd);
            if (VIR_Symbol_isUniform(sym) && isSymUniformCompiletimeInitialized(sym) &&
                VIR_Operand_GetRelAddrMode(Opnd) == VIR_INDEXED_NONE)
            {
                VIR_Type *symType;
                VIR_ConstId constId;
                VIR_Const * cValue;

                symType = VIR_Symbol_GetType(sym);
                if(VIR_Type_isArray(symType)) {
                    gctINT arrayIndex;

                    arrayIndex = VIR_Operand_GetConstIndexingImmed(Opnd) +
                                 VIR_Operand_GetMatrixConstIndex(Opnd);
                    constId = *(VIR_Uniform_GetInitializerPtr(sym->u2.uniform) + arrayIndex);
                }
                else {
                    constId = VIR_Uniform_GetInitializer(sym->u2.uniform);
                }
                cValue = (VIR_Const *)VIR_GetSymFromId(&Shader->constTable,
                                                       constId);
                imm = VIR_Const_EncodeValueIn5Bits(cValue);
            }
        }
    }
    if (imm != 0)
    {
        /* change operand to Immediate */
        VIR_Operand_SetImmediateInt(Opnd, imm);
        VIR_Operand_SetFlag(Opnd, VIR_OPNDFLAG_5BITOFFSET);
        return gcvTRUE;
    }

    return gcvFALSE;
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

    if (VIR_Inst_isComponentwise(Inst) &&
        !VIR_Operand_isRestrict(Operand))
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
        case VIR_OP_STORE_L:
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

static VSC_ErrCode
_CalculateStartOffset(
    IN OUT  VIR_Shader*     pShader,
    IN  VIR_Function*       pFunc,
    IN  VIR_Symbol*         pBaseSymbol,
    IN  VIR_SymId*          pBaseOffset,
    IN  VIR_SymbolKind      baseOffsetType
    )
{
    VSC_ErrCode             errCode   = VSC_ERR_NONE;
    VIR_Type*               pType = VIR_Symbol_GetType(pBaseSymbol);
    VIR_SymIdList*          pFields;
    VIR_Symbol*             pFirstFieldSym = gcvNULL;
    gctUINT                 startOffset = 0;
    gctUINT                 baseOffset = *pBaseOffset;

    /* Get the non-array type first. */
    while (VIR_Type_isArray(pType))
    {
        pType = VIR_Shader_GetTypeFromId(pShader, VIR_Type_GetBaseTypeId(pType));
    }

    gcmASSERT(VIR_Type_isStruct(pType));

    pFields = VIR_Type_GetFields(pType);
    pFirstFieldSym = VIR_Shader_GetSymFromId(pShader, VIR_IdList_GetId(pFields, 0));
    startOffset = VIR_FieldInfo_GetOffset(VIR_Symbol_GetFieldInfo(pFirstFieldSym));

    /* Nothing to do if it starts with 0. */
    if (startOffset == 0)
    {
        return errCode;
    }

    if (baseOffsetType == VIR_SYM_CONST)
    {
        baseOffset -= startOffset;
    }
    else
    {
        VIR_Instruction*    pNewInst;
        VIR_Operand*        pNewOpnd;

        errCode = VIR_Function_AddInstruction(pFunc,
                                              VIR_OP_SUB,
                                              VIR_TYPE_UINT16,
                                              &pNewInst);
        ON_ERROR(errCode, "Add SUB instruction failed.");

        /* Set DEST. */
        pNewOpnd = VIR_Inst_GetDest(pNewInst);
        VIR_Operand_SetTempRegister(pNewOpnd, pFunc, baseOffset, VIR_TYPE_UINT16);
        VIR_Operand_SetEnable(pNewOpnd, VIR_ENABLE_X);

        /* Set SRC0. */
        pNewOpnd = VIR_Inst_GetSource(pNewInst, 0);
        VIR_Operand_SetImmediateUint(pNewOpnd, startOffset);
    }

    /* Update the result. */
    if (pBaseOffset)
    {
        *pBaseOffset = baseOffset;
    }

OnError:
    return errCode;
}

static VSC_ErrCode
_ConvertAccessChainOffsetForPushConstant(
    IN OUT  VIR_Shader*     pShader,
    IN  VIR_Function*       pFunc,
    IN  gctUINT             resultId,
    IN  VIR_SymId           baseSymId,
    IN  VIR_Type*           pLastElementType,
    IN  VIR_SymId*          pBaseOffset,
    IN  VIR_SymbolKind*     pBaseOffsetType,
    IN  VIR_SymId*          pChannelOffset,
    IN  VIR_SymbolKind*     pChannelOffsetType,
    INOUT VIR_Symbol**      ppArraySym,
    INOUT VIR_Operand**     ppArrayOperand,
    INOUT VIR_SymId*        pArrayIndexSymId
    )
{
    VSC_ErrCode             errCode   = VSC_ERR_NONE;
    gctUINT                 perChannelSizeByByte = __PER_CHANNEL_DATA_MEMORY_SIZE__;
    gctUINT                 perRegSizeByByte = __PER_CHANNEL_DATA_MEMORY_SIZE__ * VIR_CHANNEL_COUNT;
    gctUINT                 elementSize;
    VIR_SymId               arrayIndexSymId = VIR_INVALID_ID;
    VIR_Symbol*             pArrayIndexSym = gcvNULL;
    VIR_SymId               baseOffset = *pBaseOffset, channelOffset = *pChannelOffset;
    VIR_SymbolKind          baseOffsetType = *pBaseOffsetType, channelOffsetType = *pChannelOffsetType;
    VIR_Instruction*        pNewInst = gcvNULL;
    VIR_Operand*            pNewOpnd = gcvNULL;
    VIR_TypeId              lastElementTypeId = VIR_Type_GetIndex(pLastElementType);
    VIR_NameId              nameId = VIR_INVALID_ID;
    gctUINT                 i, offset = 0;
    gctCHAR                 tempChar[128];

    /* If the byte-based offset is a constant, just convert it to reg-based offset.*/
    if (baseOffsetType == VIR_SYM_CONST)
    {
        channelOffset = (baseOffset % perRegSizeByByte) / perChannelSizeByByte;
        baseOffset = baseOffset / perRegSizeByByte;

        channelOffsetType = VIR_SYM_CONST;
        baseOffsetType = VIR_SYM_CONST;
    }
    /* If the byte-based offset is a symbol, we need to convert it to reg-based offset by checking element type. */
    else
    {
        elementSize = VIR_GetTypeSize(lastElementTypeId);
        /* I: Generate the arrayIndex symbol. */
        /* arrayIdx = (baseOffset % 16) / elementSize. */
        if (elementSize <= 8)
        {
            gcoOS_PrintStrSafe(tempChar, 32, &offset, "_spv_ac_arr_idx_%d", resultId);
            VIR_Shader_AddString(pShader, tempChar, &nameId);
            VIR_Shader_AddSymbol(pShader,
                                 VIR_SYM_VARIABLE,
                                 nameId,
                                 VIR_Shader_GetTypeFromId(pShader, VIR_TYPE_UINT16),
                                 VIR_STORAGE_GLOBAL,
                                 &arrayIndexSymId);
            pArrayIndexSym = VIR_Shader_GetSymFromId(pShader, arrayIndexSymId);
            VIR_Symbol_SetFlag(pArrayIndexSym, VIR_SYMFLAG_WITHOUT_REG);

            /* arrayIdx = baseOffset % 16 */
            errCode = VIR_Function_AddInstruction(pFunc,
                                                  VIR_OP_MOD,
                                                  VIR_TYPE_UINT16,
                                                  &pNewInst);
            ON_ERROR(errCode, "Add DIV instruction failed.");

            pNewOpnd = VIR_Inst_GetDest(pNewInst);
            VIR_Operand_SetTempRegister(pNewOpnd, pFunc, arrayIndexSymId, VIR_TYPE_UINT16);
            VIR_Operand_SetEnable(pNewOpnd, VIR_ENABLE_X);

            pNewOpnd = VIR_Inst_GetSource(pNewInst, 0);
            VIR_Operand_SetTempRegister(pNewOpnd, pFunc, baseOffset, VIR_TYPE_UINT16);
            VIR_Operand_SetSwizzle(pNewOpnd, VIR_SWIZZLE_XXXX);

            pNewOpnd = VIR_Inst_GetSource(pNewInst, 1);
            VIR_Operand_SetImmediateUint(pNewOpnd, perRegSizeByByte);

            /* arrayIdx = arrayIdx / elementSize */
            errCode = VIR_Function_AddInstruction(pFunc,
                                                  VIR_OP_DIV,
                                                  VIR_TYPE_UINT16,
                                                  &pNewInst);
            ON_ERROR(errCode, "Add DIV instruction failed.");

            pNewOpnd = VIR_Inst_GetDest(pNewInst);
            VIR_Operand_SetTempRegister(pNewOpnd, pFunc, arrayIndexSymId, VIR_TYPE_UINT16);
            VIR_Operand_SetEnable(pNewOpnd, VIR_ENABLE_X);

            pNewOpnd = VIR_Inst_GetSource(pNewInst, 0);
            VIR_Operand_SetTempRegister(pNewOpnd, pFunc, arrayIndexSymId, VIR_TYPE_UINT16);
            VIR_Operand_SetSwizzle(pNewOpnd, VIR_SWIZZLE_XXXX);

            pNewOpnd = VIR_Inst_GetSource(pNewInst, 1);
            VIR_Operand_SetImmediateUint(pNewOpnd, elementSize);
        }

        /* II: Callculate the new baseOffset. */
        errCode = VIR_Function_AddInstruction(pFunc,
                                              VIR_OP_DIV,
                                              VIR_TYPE_UINT16,
                                              &pNewInst);
        ON_ERROR(errCode, "Add DIV instruction failed.");

        pNewOpnd = VIR_Inst_GetDest(pNewInst);
        VIR_Operand_SetTempRegister(pNewOpnd, pFunc, baseOffset, VIR_TYPE_UINT16);
        VIR_Operand_SetEnable(pNewOpnd, VIR_ENABLE_X);

        pNewOpnd = VIR_Inst_GetSource(pNewInst, 0);
        VIR_Operand_SetTempRegister(pNewOpnd, pFunc, baseOffset, VIR_TYPE_UINT16);
        VIR_Operand_SetSwizzle(pNewOpnd, VIR_SWIZZLE_XXXX);

        pNewOpnd = VIR_Inst_GetSource(pNewInst, 1);
        VIR_Operand_SetImmediateUint(pNewOpnd, perRegSizeByByte);

        /* II: Create a variable to save the channel index if the element sizeInByte is less or equal than 8. */
        if (elementSize <= 8)
        {
            VIR_TypeId      arrayTypeId = VIR_TYPE_UNKNOWN;
            VIR_SymId       arraySymId = VIR_INVALID_ID;
            VIR_Symbol*     pArraySym = gcvNULL;
            VIR_Swizzle     srcSwizzle = VIR_SWIZZLE_X;
            gctUINT         arrayLength = perRegSizeByByte / elementSize;

            gcmASSERT(arrayLength == 2 || arrayLength == 4);

            offset = 0;
            gcoOS_PrintStrSafe(tempChar, 32, &offset, "_spv_ac_vector_dynamic_%d", resultId);
            VIR_Shader_AddString(pShader, tempChar, &nameId);
            VIR_Shader_AddArrayType(pShader,
                                    lastElementTypeId,
                                    arrayLength,
                                    1,
                                    &arrayTypeId);
            VIR_Shader_AddSymbol(pShader,
                                 VIR_SYM_VARIABLE,
                                 nameId,
                                 VIR_Shader_GetTypeFromId(pShader, arrayTypeId),
                                 VIR_STORAGE_GLOBAL,
                                 &arraySymId);
            pArraySym = VIR_Shader_GetSymFromId(pShader, arraySymId);
            VIR_Symbol_SetFlag(pArraySym, VIR_SYMFLAG_WITHOUT_REG);

            /* Generate MOV instructions to initialize the array symbol. */
            for (i = 0; i < arrayLength; i++)
            {
                VIR_Function_AddInstruction(pFunc,
                                            VIR_OP_MOV,
                                            lastElementTypeId,
                                            &pNewInst);

                /* Set DEST. */
                pNewOpnd = VIR_Inst_GetDest(pNewInst);
                VIR_Operand_SetSym(pNewOpnd, pArraySym);
                VIR_Operand_SetOpKind(pNewOpnd, VIR_OPND_SYMBOL);
                VIR_Operand_SetTypeId(pNewOpnd, lastElementTypeId);
                VIR_Operand_SetEnable(pNewOpnd, VIR_TypeId_Conv2Enable(lastElementTypeId));
                VIR_Operand_SetIsConstIndexing(pNewOpnd, 1);
                VIR_Operand_SetRelIndex(pNewOpnd, i);

                /* Set SRC0. */
                pNewOpnd = VIR_Inst_GetSource(pNewInst, 0);
                VIR_Operand_SetSymbol(pNewOpnd, pFunc, baseSymId);
                VIR_Operand_SetTypeId(pNewOpnd, lastElementTypeId);

                /* Set swizzle. */
                if (elementSize == 4)
                {
                    srcSwizzle = (VIR_Swizzle)(i | i << 2 | i << 4 | i << 6);
                }
                else if (i == 0)
                {
                    srcSwizzle = VIR_SWIZZLE_XYYY;
                }
                else
                {
                    gcmASSERT(i == 1);
                    srcSwizzle = VIR_SWIZZLE_ZWWW;
                }
                VIR_Operand_SetSwizzle(pNewOpnd, srcSwizzle);

                ppArrayOperand[i] = pNewOpnd;
            }

            if (ppArraySym)
            {
                *ppArraySym = pArraySym;
            }

            if (pArrayIndexSymId)
            {
                *pArrayIndexSymId = arrayIndexSymId;
            }
        }
    }

    /* Update the offset info. */
    if (pBaseOffset)
    {
        *pBaseOffset = baseOffset;
    }
    if (pBaseOffsetType)
    {
        *pBaseOffsetType = baseOffsetType;
    }
    if (pChannelOffset)
    {
        *pChannelOffset = channelOffset;
    }
    if (pChannelOffsetType)
    {
        *pChannelOffsetType = channelOffsetType;
    }

OnError:
    return errCode;
}

VSC_ErrCode
VIR_Operand_EvaluateOffsetByAccessChain(
    IN OUT  VIR_Shader         *Shader,
    IN  VIR_Function           *Function,
    IN  gctUINT                 ResultId,
    IN  VIR_Symbol             *BaseSymbol,
    IN  VIR_TypeId              BaseTypeId,
    IN  VIR_BASE_TYPE_INFO     *BaseTypeInfo,
    IN  gctUINT                *AccessChain,
    IN  VIR_SymbolKind         *AccessChainType,
    IN  gctUINT                 AccessChainLength,
    OUT VIR_AC_OFFSET_INFO     *AccessChainOffsetInfo
    )
{
    VSC_ErrCode             errCode   = VSC_ERR_NONE;
    VIR_BASE_TYPE_INFO      baseTypeInfo = *BaseTypeInfo;
    VIR_Symbol             *indexSymbol;
    VIR_NameId              nameId;
    VIR_Type               *type = VIR_Shader_GetTypeFromId(Shader, BaseTypeId);
    VIR_Symbol             *fieldSymbol = gcvNULL;
    VIR_Type               *baseType = gcvNULL;
    VIR_SymbolKind          blockIndexType = VIR_SYM_CONST, baseOffsetType = VIR_SYM_CONST, vectorIndexType = VIR_SYM_CONST;
    VIR_SymId               blockIndex = VIR_INVALID_ID;
    VIR_SymId               baseOffset = VIR_INVALID_ID;
    VIR_SymId               vectorIndex = VIR_INVALID_ID;
    VIR_SymId               arrayIndexSymId = VIR_INVALID_ID;
    VIR_Symbol*             arraySym = gcvNULL;
    VIR_LayoutQual          layoutQual = VIR_LAYQUAL_NONE;
    VIR_Instruction        *mulOrMadInst = gcvNULL;
    VIR_Instruction        *addInst = gcvNULL;
    VIR_Operand            *vectorArrayOperand[4] = {gcvNULL, gcvNULL, gcvNULL, gcvNULL};
    gctINT                  arrayStride = 0, matrixStride = 0;
    gctUINT                 i, stride = 0, totalConstantOffset = 0, fieldOffset = 0;
    gctUINT                 perChannelDataSize, offset = 0;
    gctCHAR                 name[32], arrayName[32];
    gctBOOL                 isBaseAllConstantIndex = gcvTRUE;
    gctBOOL                 treatPushConstAsMemory = gcvFALSE;
    gctBOOL                 isTypeStruct, isTypeArray, isTypeMatrix, isTypeVector, isRowMajorMatrixColumnIndexing = gcvFALSE;
    gctBOOL                 noNeedWShift = gcvFALSE;
    gctBOOL                 needToGenerateAttrLdForVector = gcvFALSE;
    VIR_Type*               vectorType = gcvNULL;
    VIR_Instruction*        initializeVectorInst = gcvNULL;

    /* Do the preprocessor. */
    for (i = 0; i < AccessChainLength; i++)
    {
        if (AccessChainType[i] == VIR_SYM_VARIABLE)
        {
            isBaseAllConstantIndex = gcvFALSE;
            break;
        }
    }

#if __USE_CONST_REG_SAVE_PUSH_CONST__
    /*
    ** In principle, a push constant is saved in the memory with layout std430,
    ** but in our implementation, we treat it as a normal uniform struct and save it in the constant registers,
    ** so we need to convert the byte-based offset to reg-based offset.
    **
    ** Right now we only need to consider dynamicly indexing for push constant cause.
    */
    if (!isBaseAllConstantIndex && baseTypeInfo.bIsBasePushConstant)
    {
        treatPushConstAsMemory = gcvTRUE;
        baseTypeInfo.bIsBaseVarMemory = gcvTRUE;
    }
#endif

    /* Init the index symbol name. */
    gcoOS_PrintStrSafe(name, 32, &offset, "_spv_ac_id_%d", ResultId);

    /* Check all access chain indexes. */
    if(VIR_Type_GetKind(type) == VIR_TY_POINTER)
    {
        type = VIR_Shader_GetTypeFromId(Shader, VIR_Type_GetBaseTypeId(type));
    }
    for (i = 0; i < AccessChainLength; i++)
    {
        isTypeStruct = (VIR_Type_GetKind(type) == VIR_TY_STRUCT);
        isTypeArray = (VIR_Type_GetKind(type) == VIR_TY_ARRAY);
        isTypeMatrix = (VIR_Type_GetKind(type) == VIR_TY_MATRIX);
        isTypeVector = (VIR_Type_GetKind(type) == VIR_TY_VECTOR);

        if(isTypeMatrix &&
           (layoutQual & VIR_LAYQUAL_ROW_MAJOR))
        {
            if(AccessChainLength - i >= 2)
            {
                /* switch matrix index if the matrix is row major */
                gctUINT tmp = AccessChain[i];
                VIR_SymbolKind tmpKind = AccessChainType[i];

                AccessChain[i] = AccessChain[i + 1];
                AccessChain[i + 1] = tmp;
                AccessChainType[i] = AccessChainType[i + 1];
                AccessChainType[i + 1] = tmpKind;
            }
            else
            {
                isRowMajorMatrixColumnIndexing = gcvTRUE;
            }
        }

        /* For an array, get the base type; for a struct, get the corresponding field symbol and offset. */
        baseType = _GetBaseTypeOrFieldSymbol(Shader,
            fieldSymbol,
            type,
            AccessChain[i],
            baseTypeInfo.bIsBaseVarMemory,
            baseTypeInfo.bIsLogicalReg,
            &fieldSymbol,
            &fieldOffset,
            &arrayStride,
            &matrixStride,
            &layoutQual);

        if (i == 0 && isTypeArray && !baseTypeInfo.bIsPtrAccessChain &&
            (baseTypeInfo.bIsBaseVarMemoryBlock || baseTypeInfo.bIsBasePerVertexArray))
        {
            blockIndexType = AccessChainType[i];
            blockIndex = AccessChain[i];
        }
        /* If this is a vector swizzle index:
        1) If this vector is a IB element, evaluate the offset.
        2) Otherwises, just return the swizzle index, SPIRV will do the indexed access.
        */
        else if (isTypeVector)
        {
            perChannelDataSize = VIR_GetTypeSize(VIR_GetTypeComponentType(VIR_Type_GetIndex(type)));

            if (baseTypeInfo.bIsBaseVarMemory)
            {
                if (AccessChainType[i] == VIR_SYM_CONST)
                {
                    totalConstantOffset += perChannelDataSize * AccessChain[i];
                }
                else
                {
                    if (baseOffset == VIR_INVALID_ID)
                    {
                        baseOffsetType = VIR_SYM_VARIABLE;

                        /* Create the index symbol if it is not existed. */
                        VIR_Shader_AddString(Shader, name, &nameId);
                        VIR_Shader_AddSymbol(Shader,
                            VIR_SYM_VARIABLE,
                            nameId,
                            VIR_Shader_GetTypeFromId(Shader, VIR_TYPE_UINT32),
                            VIR_STORAGE_GLOBAL,
                            &baseOffset);
                        indexSymbol = VIR_Shader_GetSymFromId(Shader, baseOffset);
                        VIR_Symbol_SetFlag(indexSymbol, VIR_SYMFLAG_WITHOUT_REG);

                        /* MUL instruction */
                        errCode = VIR_Function_AddInstruction(Function,
                            VIR_OP_MUL,
                            VIR_TYPE_UINT32,
                            &mulOrMadInst);
                        if (errCode != VSC_ERR_NONE) return errCode;

                        /* Set DEST. */
                        VIR_Operand_SetTempRegister(VIR_Inst_GetDest(mulOrMadInst),
                            Function,
                            baseOffset,
                            VIR_TYPE_UINT32);
                        VIR_Operand_SetEnable(VIR_Inst_GetDest(mulOrMadInst), VIR_ENABLE_X);

                        /* Set SOURCE0. */
                        VIR_Operand_SetTempRegister(VIR_Inst_GetSource(mulOrMadInst, VIR_Operand_Src0),
                            Function,
                            AccessChain[i],
                            VIR_TYPE_UINT32);
                        VIR_Operand_SetSwizzle(VIR_Inst_GetSource(mulOrMadInst, VIR_Operand_Src0), VIR_SWIZZLE_XXXX);

                        /* Set SOURCE1. */
                        VIR_Operand_SetImmediateUint(VIR_Inst_GetSource(mulOrMadInst, VIR_Operand_Src1), perChannelDataSize);
                    }
                    else
                    {
                        /* MAD instruction */
                        errCode = VIR_Function_AddInstruction(Function,
                            VIR_OP_MAD,
                            VIR_TYPE_UINT32,
                            &mulOrMadInst);
                        if (errCode != VSC_ERR_NONE) return errCode;

                        /* Set DEST. */
                        VIR_Operand_SetTempRegister(VIR_Inst_GetDest(mulOrMadInst),
                            Function,
                            baseOffset,
                            VIR_TYPE_UINT32);
                        VIR_Operand_SetEnable(VIR_Inst_GetDest(mulOrMadInst), VIR_ENABLE_X);

                        /* Set SOURCE0. */
                        VIR_Operand_SetTempRegister(VIR_Inst_GetSource(mulOrMadInst, VIR_Operand_Src0),
                            Function,
                            AccessChain[i],
                            VIR_TYPE_UINT32);
                        VIR_Operand_SetSwizzle(VIR_Inst_GetSource(mulOrMadInst, VIR_Operand_Src0), VIR_SWIZZLE_XXXX);

                        /* Set SOURCE1. */
                        VIR_Operand_SetImmediateUint(VIR_Inst_GetSource(mulOrMadInst, VIR_Operand_Src1), perChannelDataSize);

                        /* Set SOURCE2. */
                        VIR_Operand_SetTempRegister(VIR_Inst_GetSource(mulOrMadInst, VIR_Operand_Src2),
                            Function,
                            baseOffset,
                            VIR_TYPE_UINT32);
                        VIR_Operand_SetSwizzle(VIR_Inst_GetSource(mulOrMadInst, VIR_Operand_Src2), VIR_SWIZZLE_XXXX);
                    }
                }
            }
            else
            {
                vectorIndexType = AccessChainType[i];
                vectorIndex = AccessChain[i];

                if(vectorIndexType == VIR_SYM_VARIABLE)
                {
                    VIR_TypeId arrayTypeId;
                    VIR_SymId arraySymId;
                    VIR_Instruction* arrayInitializationInst;
                    gctUINT j;

                    if (baseTypeInfo.bIsBasePerVertexArray && blockIndex != VIR_INVALID_ID)
                    {
                        needToGenerateAttrLdForVector = gcvTRUE;
                        vectorType = type;
                    }
                    else
                    {
                        baseOffsetType = VIR_SYM_UNKNOWN;
                    }

                    offset = 0;
                    gcoOS_PrintStrSafe(arrayName, 32, &offset, "#spv_ac_vector_dynamic_%d", ResultId);

                    VIR_Shader_AddArrayType(Shader, VIR_Type_GetIndex(baseType), VIR_GetTypeComponents(VIR_Type_GetIndex(type)), 1, &arrayTypeId);
                    VIR_Shader_AddString(Shader, arrayName, &nameId);
                    VIR_Shader_AddSymbol(Shader,
                        VIR_SYM_VARIABLE,
                        nameId,
                        VIR_Shader_GetTypeFromId(Shader, arrayTypeId),
                        VIR_STORAGE_GLOBAL,
                        &arraySymId);

                    arraySym = VIR_Shader_GetSymFromId(Shader, arraySymId);
                    VIR_Symbol_SetFlag(arraySym,VIR_SYMFLAG_WITHOUT_REG);

                    for(j = 0; j < VIR_GetTypeComponents(VIR_Type_GetIndex(type)); j++)
                    {
                        VIR_Function_AddInstruction(Function, VIR_OP_MOV, VIR_Type_GetIndex(baseType), &arrayInitializationInst);
                        VIR_Operand_SetSym(VIR_Inst_GetDest(arrayInitializationInst), arraySym);
                        VIR_Operand_SetOpKind(VIR_Inst_GetDest(arrayInitializationInst), VIR_OPND_SYMBOL);
                        VIR_Operand_SetTypeId(VIR_Inst_GetDest(arrayInitializationInst), VIR_Type_GetIndex(baseType));
                        VIR_Operand_SetEnable(VIR_Inst_GetDest(arrayInitializationInst), VIR_ENABLE_X);
                        VIR_Operand_SetIsConstIndexing(VIR_Inst_GetDest(arrayInitializationInst), 1);
                        VIR_Operand_SetRelIndex(VIR_Inst_GetDest(arrayInitializationInst), j);
                        VIR_Operand_SetSymbol(VIR_Inst_GetSource(arrayInitializationInst, 0), Function, VIR_Symbol_GetIndex(BaseSymbol));
                        VIR_Operand_SetTypeId(VIR_Inst_GetSource(arrayInitializationInst, 0), VIR_Type_GetIndex(baseType));
                        VIR_Operand_SetSwizzle(VIR_Inst_GetSource(arrayInitializationInst, 0), (VIR_Swizzle)(j | j << 2 | j << 4 | j << 6));

                        vectorArrayOperand[j] = VIR_Inst_GetSource(arrayInitializationInst, 0);

                        if (j == 0)
                        {
                            initializeVectorInst = arrayInitializationInst;
                        }
                    }
                }
            }
        }
        /* If this is a struct/IB element, it must be a constant index. */
        else if (isTypeStruct)
        {
            gcmASSERT(AccessChainType[i] == VIR_SYM_CONST);
            totalConstantOffset += fieldOffset;
        }
        /* If this is an array/matrix index, evaluate base offset. */
        else if (isTypeArray || isTypeMatrix)
        {
            gcmASSERT(fieldOffset == 0);

            if (baseTypeInfo.bIsBaseVarMemory)
            {
                if (isTypeArray)
                {
                    stride = arrayStride;
                }
                else
                {
                    stride = matrixStride;
                }
            }
            else
            {
                stride = VIR_Type_GetRegOrOpaqueCount(Shader,
                    baseType,
                    VIR_TypeId_isSampler(VIR_Type_GetIndex(baseType)),
                    VIR_TypeId_isImage(VIR_Type_GetIndex(baseType)),
                    VIR_TypeId_isAtomicCounters(VIR_Type_GetIndex(baseType)),
                    baseTypeInfo.bIsLogicalReg);
            }

            /* If it is a constant index, just evaluate the constant offset. */
            if (AccessChainType[i] == VIR_SYM_CONST)
            {
                if(isRowMajorMatrixColumnIndexing)
                {
                    totalConstantOffset += 4 * AccessChain[i];
                }
                else
                {
                    totalConstantOffset += (stride * AccessChain[i]);
                }
            }
            /* If it is a register index, then MUL the array offset and ADD the prev array offset.*/
            else
            {
                baseOffsetType = VIR_SYM_VARIABLE;

                if (baseOffset == VIR_INVALID_ID)
                {
                    /* Create the index symbol if it is not existed. */
                    VIR_Shader_AddString(Shader, name, &nameId);
                    VIR_Shader_AddSymbol(Shader,
                        VIR_SYM_VARIABLE,
                        nameId,
                        VIR_Shader_GetTypeFromId(Shader, VIR_TYPE_UINT32),
                        VIR_STORAGE_GLOBAL,
                        &baseOffset);
                    indexSymbol = VIR_Shader_GetSymFromId(Shader, baseOffset);
                    VIR_Symbol_SetFlag(indexSymbol, VIR_SYMFLAG_WITHOUT_REG);

                    /* MUL instruction */
                    errCode = VIR_Function_AddInstruction(Function,
                        VIR_OP_MUL,
                        VIR_TYPE_UINT32,
                        &mulOrMadInst);
                    if (errCode != VSC_ERR_NONE) return errCode;

                    /* Set DEST. */
                    VIR_Operand_SetTempRegister(VIR_Inst_GetDest(mulOrMadInst),
                        Function,
                        baseOffset,
                        VIR_TYPE_UINT32);
                    VIR_Operand_SetEnable(VIR_Inst_GetDest(mulOrMadInst), VIR_ENABLE_X);

                    /* Set SOURCE0. */
                    VIR_Operand_SetTempRegister(VIR_Inst_GetSource(mulOrMadInst, VIR_Operand_Src0),
                        Function,
                        AccessChain[i],
                        VIR_TYPE_UINT32);
                    VIR_Operand_SetSwizzle(VIR_Inst_GetSource(mulOrMadInst, VIR_Operand_Src0), VIR_SWIZZLE_XXXX);

                    /* Set SOURCE1. */
                    if(isRowMajorMatrixColumnIndexing)
                    {
                        VIR_Operand_SetImmediateUint(VIR_Inst_GetSource(mulOrMadInst, VIR_Operand_Src1), 4);
                    }
                    else
                    {
                        VIR_Operand_SetImmediateUint(VIR_Inst_GetSource(mulOrMadInst, VIR_Operand_Src1), stride);
                    }
                }
                else
                {
                    /* MAD instruction */
                    errCode = VIR_Function_AddInstruction(Function,
                        VIR_OP_MAD,
                        VIR_TYPE_UINT32,
                        &mulOrMadInst);
                    if (errCode != VSC_ERR_NONE) return errCode;

                    /* Set DEST. */
                    VIR_Operand_SetTempRegister(VIR_Inst_GetDest(mulOrMadInst),
                        Function,
                        baseOffset,
                        VIR_TYPE_UINT32);
                    VIR_Operand_SetEnable(VIR_Inst_GetDest(mulOrMadInst), VIR_ENABLE_X);

                    /* Set SOURCE0. */
                    VIR_Operand_SetTempRegister(VIR_Inst_GetSource(mulOrMadInst, VIR_Operand_Src0),
                        Function,
                        AccessChain[i],
                        VIR_TYPE_UINT32);
                    VIR_Operand_SetSwizzle(VIR_Inst_GetSource(mulOrMadInst, VIR_Operand_Src0), VIR_SWIZZLE_XXXX);

                    /* Set SOURCE1. */
                    if(isRowMajorMatrixColumnIndexing)
                    {
                        VIR_Operand_SetImmediateUint(VIR_Inst_GetSource(mulOrMadInst, VIR_Operand_Src1), 4);
                    }
                    else
                    {
                        VIR_Operand_SetImmediateUint(VIR_Inst_GetSource(mulOrMadInst, VIR_Operand_Src1), stride);
                    }

                    /* Set SOURCE2. */
                    VIR_Operand_SetTempRegister(VIR_Inst_GetSource(mulOrMadInst, VIR_Operand_Src2),
                        Function,
                        baseOffset,
                        VIR_TYPE_UINT32);
                    VIR_Operand_SetSwizzle(VIR_Inst_GetSource(mulOrMadInst, VIR_Operand_Src2), VIR_SWIZZLE_XXXX);
                }
            }
        }
        else
        {
            gcmASSERT(gcvFALSE);
        }

        if (i != AccessChainLength)
        {
            type = baseType;
        }
    }

    /* Add the rest constant offset. */
    if (!isBaseAllConstantIndex && totalConstantOffset != 0 && baseOffset != VIR_INVALID_ID)
    {
        /* ADD instruction */
        errCode = VIR_Function_AddInstruction(Function,
            VIR_OP_ADD,
            VIR_TYPE_UINT32,
            &addInst);
        if (errCode != VSC_ERR_NONE) return errCode;

        /* Set DEST. */
        VIR_Operand_SetTempRegister(VIR_Inst_GetDest(addInst),
            Function,
            baseOffset,
            VIR_TYPE_UINT32);
        VIR_Operand_SetEnable(VIR_Inst_GetDest(addInst), VIR_ENABLE_X);

        /* Set SOURCE0. */
        VIR_Operand_SetTempRegister(VIR_Inst_GetSource(addInst, VIR_Operand_Src0),
            Function,
            baseOffset,
            VIR_TYPE_UINT32);
        VIR_Operand_SetSwizzle(VIR_Inst_GetSource(addInst, VIR_Operand_Src0), VIR_SWIZZLE_XXXX);

        /* Set SOURCE1. */
        VIR_Operand_SetImmediateUint(VIR_Inst_GetSource(addInst, VIR_Operand_Src1), totalConstantOffset);
    }

    /* For vectorIndex, we don't need to change baseOffset here. */
    if (isBaseAllConstantIndex || (baseOffset == VIR_INVALID_ID && baseOffsetType == VIR_SYM_CONST))
    {
        baseOffset = totalConstantOffset;
    }

    if (treatPushConstAsMemory)
    {
        /* First get the start offset, and if it is not 0, we need to minus this. */
        errCode = _CalculateStartOffset(Shader,
                                        Function,
                                        BaseSymbol,
                                        &baseOffset,
                                        baseOffsetType);

        errCode = _ConvertAccessChainOffsetForPushConstant(Shader,
                                                           Function,
                                                           ResultId,
                                                           VIR_Symbol_GetIndex(BaseSymbol),
                                                           baseType,
                                                           &baseOffset,
                                                           &baseOffsetType,
                                                           &vectorIndex,
                                                           &vectorIndexType,
                                                           &arraySym,
                                                           vectorArrayOperand,
                                                           &arrayIndexSymId);
        noNeedWShift = gcvTRUE;
    }

    /* If the vertex index is a PerVertex, we need to generate a ATTR_LD to load the data first. */
    if (needToGenerateAttrLdForVector)
    {
        VIR_SymId attrLdSymId;
        VIR_Symbol* attrLdSym = gcvNULL;
        VIR_Instruction* attrLdInst;
        VIR_Operand* opnd = gcvNULL;

        /* Currectly only TCS can support loading a attribute from a output. */
        gcmASSERT(VIR_Shader_IsTCS(Shader));

        /* Add a temp symbol to save the vector data. */
        offset = 0;
        gcoOS_PrintStrSafe(arrayName, 32, &offset, "#spv_ac_vector_attrld_%d", ResultId);

        VIR_Shader_AddString(Shader, arrayName, &nameId);
        VIR_Shader_AddSymbol(Shader,
            VIR_SYM_VARIABLE,
            nameId,
            vectorType,
            VIR_STORAGE_GLOBAL,
            &attrLdSymId);

        attrLdSym = VIR_Shader_GetSymFromId(Shader, attrLdSymId);
        VIR_Symbol_SetFlag(attrLdSym,VIR_SYMFLAG_WITHOUT_REG);

        VIR_Function_AddInstructionBefore(Function,
                                          VIR_OP_ATTR_LD,
                                          VIR_Type_GetIndex(vectorType),
                                          initializeVectorInst,
                                          gcvTRUE,
                                          &attrLdInst);

        opnd = VIR_Inst_GetDest(attrLdInst);
        VIR_Operand_SetSymbol(opnd, Function, attrLdSymId);
        VIR_Operand_SetEnable(opnd, VIR_TypeId_Conv2Enable(VIR_Type_GetIndex(vectorType)));

        opnd = VIR_Inst_GetSource(attrLdInst, 0);
        VIR_Operand_SetSymbol(opnd, Function, VIR_Symbol_GetIndex(BaseSymbol));
        VIR_Operand_SetSwizzle(opnd, VIR_SWIZZLE_XYZW);

        opnd = VIR_Inst_GetSource(attrLdInst, 1);
        if (blockIndexType == VIR_SYM_CONST)
        {
            VIR_Operand_SetImmediateUint(opnd, blockIndex);
        }
        else
        {
            VIR_Operand_SetSymbol(opnd, Function, blockIndex);
            VIR_Operand_SetSwizzle(opnd, VIR_SWIZZLE_XXXX);
        }

        opnd = VIR_Inst_GetSource(attrLdInst, 2);
        if (baseOffsetType == VIR_SYM_CONST)
        {
            VIR_Operand_SetImmediateUint(opnd, baseOffset);
        }
        else
        {
            VIR_Operand_SetSymbol(opnd, Function, baseOffset);
            VIR_Operand_SetSwizzle(opnd, VIR_SWIZZLE_XXXX);
        }

        for (i = 0; i < 4; i++)
        {
            if (vectorArrayOperand[i] == gcvNULL)
                continue;

            VIR_Operand_SetSymbol(vectorArrayOperand[i], Function, attrLdSymId);
            VIR_Operand_SetTypeId(vectorArrayOperand[i], VIR_GetTypeComponentType(VIR_Type_GetIndex(vectorType)));
        }
    }
    else
    {
        /* If there is vectorArrayOperand, set the baseOffset to the source.*/
        for (i = 0; i < 4; i++)
        {
            if (vectorArrayOperand[i] == gcvNULL)
                continue;

            if (isBaseAllConstantIndex && baseOffset != 0)
            {
                VIR_Operand_SetIsConstIndexing(vectorArrayOperand[i], gcvTRUE);
                VIR_Operand_SetRelIndex(vectorArrayOperand[i], baseOffset);
            }
            else if (!isBaseAllConstantIndex && baseOffset != VIR_INVALID_ID)
            {
                VIR_Operand_SetIsConstIndexing(vectorArrayOperand[i], gcvFALSE);
                VIR_Operand_SetRelIndex(vectorArrayOperand[i], baseOffset);
                VIR_Operand_SetRelAddrMode(vectorArrayOperand[i], VIR_INDEXED_X);
            }
        }
    }

    if (arrayIndexSymId != VIR_INVALID_ID)
    {
        gcmASSERT(baseOffsetType == VIR_SYM_VARIABLE);

        baseOffset = arrayIndexSymId;
    }

    /* Save the result. */
    if (AccessChainOffsetInfo)
    {
        AccessChainOffsetInfo->bHasAccessChain = gcvTRUE;
        AccessChainOffsetInfo->blockIndexType = blockIndexType;
        AccessChainOffsetInfo->blockIndex = blockIndex;
        AccessChainOffsetInfo->baseOffsetType = baseOffsetType;
        AccessChainOffsetInfo->baseOffset = baseOffset;
        AccessChainOffsetInfo->vectorIndexType = vectorIndexType;
        AccessChainOffsetInfo->vectorIndex = vectorIndex;
        AccessChainOffsetInfo->noNeedWShift = noNeedWShift;
        AccessChainOffsetInfo->arraySym = arraySym;
        AccessChainOffsetInfo->arrayStride = arrayStride;
        AccessChainOffsetInfo->matrixStride = matrixStride;
        AccessChainOffsetInfo->layoutQual = layoutQual;
        AccessChainOffsetInfo->isRowMajorMatrixColumnIndexing = isRowMajorMatrixColumnIndexing;
    }

    return errCode;
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
    VIR_Swizzle             swizzle = VIR_Operand_GetSwizzle(srcOpnd);
    gctUINT                 mapChannel = VIR_Swizzle_GetChannel(swizzle, channel);

    VIR_Operand_GetOperandInfo(Inst,
        srcOpnd,
        &operandInfo);

    if (VIR_OpndInfo_Is_Virtual_Reg(&operandInfo))
    {
        /* Only return first reg no currently */
        virRegNo = operandInfo.u1.virRegInfo.virReg;

        *pValue =  mapChannel;
    }
    else if (operandInfo.isImmVal)
    {
        *pValue = operandInfo.u1.immValue.uValue;
    }
    else if (operandInfo.isVecConst)
    {
        pConstValue = (VIR_Const*)VIR_GetSymFromId(&pShader->constTable, VIR_Operand_GetConstId(srcOpnd));

        *pValue = pConstValue->value.vecVal.u32Value[mapChannel];
    }
    else if (operandInfo.isUniform)
    {
        virRegNo = operandInfo.u1.uniformIdx;

        *pValue =  mapChannel;
    }
    else if (operandInfo.isImage || operandInfo.isImageT || operandInfo.isSampler || operandInfo.isSamplerT || operandInfo.isTexture)
    {
        virRegNo = operandInfo.u1.symId;

        *pValue =  mapChannel;
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

VIR_Precision
VIR_Operand_GetPrecision(
    IN VIR_Operand *Operand
    )
{
    VIR_OperandKind     opndKind = VIR_Operand_GetOpKind(Operand);
    /* deprecate the operand's precision, instead, using symbol's precision */
    if (opndKind == VIR_OPND_VIRREG ||
        opndKind == VIR_OPND_SYMBOL ||
        opndKind == VIR_OPND_SAMPLER_INDEXING)
    {
        VIR_Symbol* pSym = VIR_Operand_GetSymbol(Operand);
        if(VIR_Symbol_GetPrecision(pSym) != VIR_PRECISION_ANY)
        {
            return VIR_Symbol_GetPrecision(pSym);
        }
    }

    if (opndKind == VIR_OPND_INTRINSIC || opndKind == VIR_OPND_PARAMETERS || opndKind == VIR_OPND_NAME)
    {
        return VIR_PRECISION_HIGH;
    }

    return Operand->u.n._precision;
}

void
VIR_Operand_SetPrecision(
    IN OUT VIR_Operand *Operand,
    IN VIR_Precision Precision
    )
{
    VIR_OperandKind     opndKind = VIR_Operand_GetOpKind(Operand);
    /* deprecate the operand's precision, instead, using symbol's precision */
    if (opndKind == VIR_OPND_VIRREG ||
        opndKind == VIR_OPND_SYMBOL ||
        opndKind == VIR_OPND_SAMPLER_INDEXING)
    {
        VIR_Symbol* pSym = VIR_Operand_GetSymbol(Operand);
        if(VIR_Symbol_GetPrecision(pSym) != VIR_PRECISION_ANY)
        {
            VIR_Symbol_SetPrecision(pSym, Precision);
            return;
        }
    }

    Operand->u.n._precision = Precision;
}

VSC_ErrCode
VIR_Operand_SetIndexingFromOperand(
    IN  VIR_Shader         *pShader,
    IN  VIR_Operand        *pOperand,
    IN  VIR_Operand        *pIndexOperand
    )
{
    VSC_ErrCode             errCode = VSC_ERR_NONE;
    VIR_TypeId              indexType = VIR_Operand_GetTypeId(pIndexOperand);
    VIR_Swizzle             indexSwizzle = VIR_Operand_GetSwizzle(pIndexOperand);
    gctBOOL                 isValid = gcvFALSE;
    gctBOOL                 isRelIndex = gcvFALSE;
    gctINT                  immed = 0;
    VIR_Const              *cur_const = gcvNULL;
    gctUINT                 channel = 0;
    VIR_Symbol             *indexSym = gcvNULL;
    VIR_SymId               indexSymId = VIR_INVALID_ID;
    VIR_VirRegId            indexVirRegId = VIR_INVALID_ID;

    if (VIR_Operand_isImm(pIndexOperand))
    {
        isValid = gcvTRUE;

        if (VIR_TypeId_isFloat(indexType))
        {
            immed = (gctINT)VIR_Operand_GetImmediateFloat(pIndexOperand);
        }
        else if (VIR_TypeId_isSignedInteger(indexType))
        {
            immed = (gctINT)VIR_Operand_GetImmediateInt(pIndexOperand);
        }
        else
        {
            immed = (gctINT)VIR_Operand_GetImmediateUint(pIndexOperand);
        }
    }
    else if (VIR_Operand_isConst(pIndexOperand))
    {
        cur_const = VIR_Shader_GetConstFromId(pShader, VIR_Operand_GetConstId(pIndexOperand));
        channel = VIR_Swizzle_GetChannel(indexSwizzle, 0);

        if (VIR_Swizzle_Channel_Count(channel) == 1)
        {
            gcmASSERT(channel <= VIR_SWIZZLE_W);
            isValid = gcvTRUE;

            if (VIR_TypeId_isFloat(indexType))
            {
                immed = (gctINT)cur_const->value.vecVal.f32Value[channel];
            }
            else if (VIR_TypeId_isSignedInteger(indexType))
            {
                immed = (gctINT)cur_const->value.vecVal.i32Value[channel];
            }
            else
            {
                immed = (gctINT)cur_const->value.vecVal.u32Value[channel];
            }
        }

    }
    else
    {
        channel = VIR_Swizzle_GetChannel(indexSwizzle, 0);
        if (VIR_Swizzle_Channel_Count(channel) == 1)
        {
            gcmASSERT(channel <= VIR_SWIZZLE_W);
            isRelIndex = gcvTRUE;
            isValid = gcvTRUE;

            indexSym = VIR_Operand_GetSymbol(pIndexOperand);
            indexVirRegId = VIR_Symbol_GetVregIndex(indexSym);

            errCode = VIR_Shader_GetVirRegSymByVirRegId(pShader,
                                                        indexVirRegId,
                                                        &indexSymId);
            CHECK_ERROR(errCode, "VIR_Shader_GetVirRegSymByVirRegId failed.");
        }
    }

    if (isValid)
    {
        if (isRelIndex)
        {
            gcmASSERT(indexSymId != VIR_INVALID_ID);
            VIR_Operand_SetRelIndexing(pOperand, indexSymId);
            VIR_Operand_SetRelAddrMode(pOperand, channel + 1);
        }
        else
        {
            VIR_Operand_SetRelIndexingImmed(pOperand, immed);
        }
    }

    return errCode;
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

VIR_Link*
VIR_Link_RemoveLink(
    IN  VIR_Link **         Head,
    IN  gctUINTPTR_T        Reference
    )
{
    VIR_Link* ret = gcvNULL;

    gcmASSERT(Head);

    if(*Head == gcvNULL)
    {
        return ret;
    }
    else if((*Head)->referenced == Reference)
    {
        ret = *Head;
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
            ret = remove;
            prev->next = remove->next;
        }
    }

    return ret;
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
        out_imm->fValue = -in_imm->fValue;
        break;
    case VIR_TYPE_INT32:
    case VIR_TYPE_INT16:
    case VIR_TYPE_INT8:
    case VIR_TYPE_UINT32:
    case VIR_TYPE_UINT16:
    case VIR_TYPE_UINT8:
        out_imm->iValue = -in_imm->iValue;
        break;
    case VIR_TYPE_UINT64:
    case VIR_TYPE_INT64:
        out_imm->lValue = -in_imm->lValue;
        break;
    default:
        gcmASSERT(gcvFALSE);
    }
}

void
VIR_ScalarConstVal_GetAbs(
    IN  VIR_PrimitiveTypeId type,
    IN  VIR_ScalarConstVal* in_imm,
    OUT VIR_ScalarConstVal* out_imm
    )
{
    switch(type)
    {
    case VIR_TYPE_FLOAT32:
        out_imm->fValue = gcoMATH_Absolute(in_imm->fValue);
        break;

    case VIR_TYPE_INT64:
    case VIR_TYPE_INT32:
    case VIR_TYPE_INT16:
    case VIR_TYPE_INT8:
        if (in_imm->iValue > 0)
        {
            out_imm->iValue = in_imm->iValue;
        }
        else
        {
            out_imm->iValue = -in_imm->iValue;
        }
        break;

    case VIR_TYPE_UINT64:
    case VIR_TYPE_UINT32:
    case VIR_TYPE_UINT16:
    case VIR_TYPE_UINT8:
        out_imm->iValue = in_imm->iValue;
        break;

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
        out_imm->fValue = in_imm0->fValue + in_imm1->fValue;
        break;
    case VIR_TYPE_INT32:
    case VIR_TYPE_INT16:
    case VIR_TYPE_INT8:
    case VIR_TYPE_UINT32:
    case VIR_TYPE_UINT16:
    case VIR_TYPE_UINT8:
        out_imm->iValue = in_imm0->iValue + in_imm1->iValue;
        break;
    case VIR_TYPE_UINT64:
    case VIR_TYPE_INT64:
        out_imm->lValue = in_imm0->lValue + in_imm1->lValue;
        break;
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
        out_imm->fValue = in_imm0->fValue * in_imm1->fValue;
        break;
    case VIR_TYPE_INT32:
    case VIR_TYPE_INT16:
    case VIR_TYPE_INT8:
    case VIR_TYPE_UINT32:
    case VIR_TYPE_UINT16:
    case VIR_TYPE_UINT8:
        out_imm->iValue = in_imm0->iValue * in_imm1->iValue;
        break;
    case VIR_TYPE_UINT64:
    case VIR_TYPE_INT64:
        out_imm->lValue = in_imm0->lValue * in_imm1->lValue;
        break;
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
        return in_imm->fValue == 1.0;
    case VIR_TYPE_INT32:
    case VIR_TYPE_UINT32:
    case VIR_TYPE_INT16:
    case VIR_TYPE_UINT16:
    case VIR_TYPE_INT8:
    case VIR_TYPE_UINT8:
        return in_imm->iValue == 1;
    case VIR_TYPE_UINT64:
    case VIR_TYPE_INT64:
        return in_imm->lValue == 1;
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
    gctINT  componentCount = VIR_GetTypeComponents(type);
    gctINT  rowCount = VIR_GetTypeRows(type);
    gctINT  constCount = componentCount * rowCount;
    gctINT  i;

    switch (type)
    {
    case VIR_TYPE_FLOAT_X2:
    case VIR_TYPE_FLOAT_X3:
    case VIR_TYPE_FLOAT_X4:
        {
            gctFLOAT* in = &in_const->f32Value[0];
            gctFLOAT* out = &out_const->f32Value[0];
            for(i = 0; i < constCount; i++)
            {
                out[i] = -in[i];
            }
            break;
        }

    case VIR_TYPE_UINT_X2:
    case VIR_TYPE_UINT_X3:
    case VIR_TYPE_UINT_X4:
    case VIR_TYPE_INTEGER_X2:
    case VIR_TYPE_INTEGER_X3:
    case VIR_TYPE_INTEGER_X4:
        {
            gctINT* in = &in_const->i32Value[0];
            gctINT* out = &out_const->i32Value[0];
            for(i = 0; i < constCount; i++)
            {
                out[i] = -in[i];
            }
            break;
        }

    case VIR_TYPE_UINT16_X8:
    case VIR_TYPE_INT16_X8:
        {
            int i;
            gctINT16* in = &in_const->i16Value[0];
            gctINT16* out = &out_const->i16Value[0];
            for(i = 0; i < constCount; i++)
            {
                out[i] = -in[i];
            }
            break;
        }

    case VIR_TYPE_UINT8_X16:
    case VIR_TYPE_INT8_X16:
        {
            int i;
            gctINT8* in = &in_const->i8Value[0];
            gctINT8* out = &out_const->i8Value[0];
            for(i = 0; i < constCount; i++)
            {
                out[i] = -in[i];
            }
            break;
        }

    default:
        gcmASSERT(gcvFALSE);
        break;
    }
}

void
VIR_VecConstVal_GetAbs(
    IN  VIR_PrimitiveTypeId type,
    IN  VIR_VecConstVal* in_const,
    OUT VIR_VecConstVal* out_const
    )
{
    gctINT  componentCount = VIR_GetTypeComponents(type);
    gctINT  rowCount = VIR_GetTypeRows(type);
    gctINT  constCount = componentCount * rowCount;
    gctINT  i;

    switch (type)
    {
    case VIR_TYPE_FLOAT_X2:
    case VIR_TYPE_FLOAT_X3:
    case VIR_TYPE_FLOAT_X4:
        {
            gctFLOAT* in = &in_const->f32Value[0];
            gctFLOAT* out = &out_const->f32Value[0];
            for(i = 0; i < constCount; i++)
            {
                out[i] = gcoMATH_Absolute(in[i]);
            }
            break;
        }

    case VIR_TYPE_UINT_X2:
    case VIR_TYPE_UINT_X3:
    case VIR_TYPE_UINT_X4:
        {
            gctINT* in = &in_const->i32Value[0];
            gctINT* out = &out_const->i32Value[0];
            for(i = 0; i < constCount; i++)
            {
                out[i] = in[i];
            }
            break;
        }

    case VIR_TYPE_INTEGER_X2:
    case VIR_TYPE_INTEGER_X3:
    case VIR_TYPE_INTEGER_X4:
        {
            gctINT* in = &in_const->i32Value[0];
            gctINT* out = &out_const->i32Value[0];
            for(i = 0; i < constCount; i++)
            {
                out[i] = in[i] > 0 ? in[i] : -in[i];
            }
            break;
        }

    case VIR_TYPE_UINT16_X8:
        {
            int i;
            gctINT16* in = &in_const->i16Value[0];
            gctINT16* out = &out_const->i16Value[0];
            for(i = 0; i < constCount; i++)
            {
                out[i] = in[i];
            }
            break;
        }

    case VIR_TYPE_INT16_X8:
        {
            int i;
            gctINT16* in = &in_const->i16Value[0];
            gctINT16* out = &out_const->i16Value[0];
            for(i = 0; i < constCount; i++)
            {
                out[i] = in[i] > 0 ? in[i] : -in[i];
            }
            break;
        }

    case VIR_TYPE_UINT8_X16:
        {
            int i;
            gctINT8* in = &in_const->i8Value[0];
            gctINT8* out = &out_const->i8Value[0];
            for(i = 0; i < constCount; i++)
            {
                out[i] = in[i];
            }
            break;
        }

    case VIR_TYPE_INT8_X16:
        {
            int i;
            gctINT8* in = &in_const->i8Value[0];
            gctINT8* out = &out_const->i8Value[0];
            for(i = 0; i < constCount; i++)
            {
                out[i] = in[i] > 0 ? in[i] : -in[i];
            }
            break;
        }

    default:
        gcmASSERT(gcvFALSE);
        break;
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
    int i;
    switch(type)
    {
    case VIR_TYPE_FLOAT_X4:
        for(i = 0; i < 4; i++)
        {
            out_const->f32Value[i] = in_const->f32Value[i] + in_const->f32Value[i];
        }
        break;
    case VIR_TYPE_UINT_X4:
    case VIR_TYPE_INTEGER_X4:
        for(i = 0; i < 4; i++)
        {
            out_const->i32Value[i] = in_const->i32Value[i] + in_const->i32Value[i];
        }
        break;
    case VIR_TYPE_UINT16_X8:
    case VIR_TYPE_INT16_X8:
        for(i = 0; i < 8; i++)
        {
            out_const->i16Value[i] = in_const->i16Value[i] + in_const->i16Value[i];
        }
        break;
    case VIR_TYPE_UINT8_X16:
    case VIR_TYPE_INT8_X16:
        for(i = 0; i < 16; i++)
        {
            out_const->i8Value[i] = in_const->i8Value[i] + in_const->i8Value[i];
        }
        break;
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
    int i;
    switch(type)
    {
    case VIR_TYPE_FLOAT_X4:
        for(i = 0; i < 4; i++)
        {
            out_const->f32Value[i] = in_const->f32Value[i] * in_imm->fValue;
        }
        break;
    case VIR_TYPE_UINT_X4:
    case VIR_TYPE_INTEGER_X4:
        for(i = 0; i < 4; i++)
        {
            out_const->i32Value[i] = in_const->i32Value[i] * in_imm->iValue;
        }
        break;
    case VIR_TYPE_UINT16_X8:
    case VIR_TYPE_INT16_X8:
        for(i = 0; i < 8; i++)
        {
            out_const->i16Value[i] = in_const->i16Value[i] * (gctINT16)in_imm->iValue;
        }
        break;
    case VIR_TYPE_UINT8_X16:
    case VIR_TYPE_INT8_X16:
        for(i = 0; i < 16; i++)
        {
            out_const->i8Value[i] = in_const->i8Value[i] * (gctINT8)in_imm->iValue;
        }
        break;
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
    int i;
    switch(type)
    {
    case VIR_TYPE_FLOAT_X4:
        for(i = 0; i < 4; i++)
        {
            out_const->f32Value[i] = in_const0->f32Value[i] + in_const1->f32Value[i];
        }
        break;
    case VIR_TYPE_UINT_X4:
    case VIR_TYPE_INTEGER_X4:
        for(i = 0; i < 4; i++)
        {
            out_const->i32Value[i] = in_const0->i32Value[i] + in_const1->i32Value[i];
        }
        break;
    case VIR_TYPE_UINT16_X8:
    case VIR_TYPE_INT16_X8:
        for(i = 0; i < 8; i++)
        {
            out_const->i16Value[i] = in_const0->i16Value[i] + in_const1->i16Value[i];
        }
        break;
    case VIR_TYPE_UINT8_X16:
    case VIR_TYPE_INT8_X16:
        for(i = 0; i < 16; i++)
        {
            out_const->i8Value[i] = in_const0->i8Value[i] + in_const1->i8Value[i];
        }
        break;
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
    int i;
    switch(type)
    {
    case VIR_TYPE_FLOAT_X4:
        for(i = 0; i < 4; i++)
        {
            out_const->f32Value[i] = in_const0->f32Value[i] * in_const1->f32Value[i];
        }
        break;
    case VIR_TYPE_UINT_X4:
    case VIR_TYPE_INTEGER_X4:
        for(i = 0; i < 4; i++)
        {
            out_const->i32Value[i] = in_const0->i32Value[i] * in_const1->i32Value[i];
        }
        break;
    case VIR_TYPE_UINT16_X8:
    case VIR_TYPE_INT16_X8:
        for(i = 0; i < 8; i++)
        {
            out_const->i16Value[i] = in_const0->i16Value[i] * in_const1->i16Value[i];
        }
        break;
    case VIR_TYPE_UINT8_X16:
    case VIR_TYPE_INT8_X16:
        for(i = 0; i < 16; i++)
        {
            out_const->i8Value[i] = in_const0->i8Value[i] * in_const1->i8Value[i];
        }
        break;
    default:
        gcmASSERT(gcvFALSE);
    }
}

gctBOOL
VIR_VecConstVal_AllSameValue(
    IN  VIR_PrimitiveTypeId type,
    IN  VIR_VecConstVal     in_const,
    IN  VIR_ScalarConstVal  value
    )
{
    gctUINT i;
    VIR_TypeId componentType = VIR_GetTypeComponentType(type);

    for (i = 0; i < VIR_GetTypeComponents(type); i++)
    {
        if (VIR_TypeId_isFloat(componentType))
        {
            if (in_const.f32Value[i] != value.fValue)
            {
                return gcvFALSE;
            }
        }
        else if VIR_TypeId_isInteger(componentType)
        {
            if (in_const.i32Value[i] != value.iValue)
            {
                return gcvFALSE;
            }
        }
        else
        {
            gcmASSERT(gcvFALSE);

            return gcvFALSE;
        }
    }

    return gcvTRUE;
}

typedef union _gctBits
{
    float f;
    gctINT32 si;
    gctUINT32 ui;
} gctBits;

#define SHIFT       13
#define SHIFTSIGN   16

#define infN   0x7F800000        /* flt32 infinity */
#define maxN   0x477FE000        /* max flt16 normal as a flt32 */
#define minN   0x38800000        /* min flt16 normal as a flt32 */
#define signN  0x80000000        /* flt32 sign bit */

#define infC  (infN >> SHIFT)
#define nanN  ((infC + 1) << SHIFT)   /* minimum flt16 nan as a flt32 */
#define maxC  (maxN >> SHIFT)
#define minC  (minN >> SHIFT)
#define signC (signN >> SHIFTSIGN)    /* flt16 sign bit */

#define mulN  0x52000000              /* (1 << 23) / minN */
#define mulC  0x33800000              /* minN / (1 << (23 - shift)) */

#define subC  0x003FF                 /* max flt32 subnormal down shifted */
#define norC  0x00400                 /* min flt32 normal down shifted */

#define maxD  (infC - maxC - 1)
#define minD  (minC - subC - 1)

gctUINT16 VIR_ConvertF32ToFP16(gctFLOAT f)
{
    gctBits v, s;
    gctUINT32 sign;
    v.f = f;
    sign = v.si & signN;
    v.si ^= sign;
    sign >>= SHIFTSIGN; /* logical shift */
    s.si = mulN;
    s.si = (gctINT32)(s.f * v.f);   /* correct subnormals */
    v.si ^= (s.si ^ v.si) & -(minN > v.si);
    v.si ^= (infN ^ v.si) & -((infN > v.si) & (v.si > maxN));
    v.si ^= (nanN ^ v.si) & -((nanN > v.si) & (v.si > infN));
    v.ui >>= SHIFT;     /* logical shift */
    v.si ^= ((v.si - maxD) ^ v.si) & -(v.si > maxC);
    v.si ^= ((v.si - minD) ^ v.si) & -(v.si > subC);
    return (gctUINT16)((v.ui | sign) & 0xFFFF);
}

gctFLOAT VIR_ConvertF16ToFP32(gctUINT16 fp16Value)
{
    gctBits v, s;
    gctINT32 mask;
    gctINT32 sign;
    v.ui = fp16Value;
    sign = v.si & signC;
    v.si ^= sign;
    sign <<= SHIFTSIGN;
    v.si ^= ((v.si + minD) ^ v.si) & -(v.si > subC);
    v.si ^= ((v.si + maxD) ^ v.si) & -(v.si > maxC);
    s.si = mulC;
    s.f *= v.si;
    mask = -(norC > v.si);
    v.si <<= SHIFT;
    v.si ^= (s.si ^ v.si) & mask;
    v.si |= sign;
    return v.f;
}

void _Reset_SrcOperand_Iterator(
    OUT VIR_SrcOperand_Iterator *   Iter
    )
{
    Iter->curNode   = gcvNULL;
    Iter->curSrcNo  = 0;
    Iter->inSrcNo   = 0;
    Iter->specialNode = 0;
    Iter->useOpndList = 0;
}

void VIR_SrcOperand_Iterator_Init(
    IN  VIR_Instruction *           Inst,
    OUT VIR_SrcOperand_Iterator *   Iter
    )
{
    Iter->inst      = Inst;
    Iter->skipUndefs = gcvTRUE;
    _Reset_SrcOperand_Iterator(Iter);
    Iter->expandNodeFlag = VIR_SRCOPERAND_FLAG_EXPAND_ALL_NODE;
}

void VIR_SrcOperand_Iterator_Init1(
    IN  VIR_Instruction *           Inst,
    OUT VIR_SrcOperand_Iterator *   Iter,
    IN  VIR_SrcOperand_Iter_ExpandFlag ExpandFlag,
    IN  gctBOOL                     SkipUndef
    )
{
    Iter->inst      = Inst;
    Iter->skipUndefs = SkipUndef;
    _Reset_SrcOperand_Iterator(Iter);
    Iter->expandNodeFlag = ExpandFlag;
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
    (Iter)->specialNode = 0;          \
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

    if (!Iter->specialNode)
    {
        if (Iter->curSrcNo >= VIR_Inst_GetSrcNum(Iter->inst))
        {
            /* no more source operand remain */
            return gcvNULL;
        }
        opnd = VIR_Inst_GetSource(Iter->inst, Iter->curSrcNo);
        gcmASSERT(opnd != gcvNULL);
        /* check if it is special node */
        if ((Iter->expandNodeFlag & VIR_SRCOPERAND_FLAG_EXPAND_TEXLD_PARM_NODE)
            &&
            VIR_Operand_isTexldParm(opnd))
        {
            Iter->specialNode = 1;
            Iter->useOpndList = 0;
            /* get the first effective modifier */
            return VIR_SrcOperand_Iterator_Next(Iter);
        }
        else if ((Iter->expandNodeFlag & VIR_SRCOPERAND_FLAG_EXPAND_PARAM_NODE)
                 &&
                 VIR_Operand_isParameters(opnd))
        {
            Iter->specialNode = 1;
            Iter->useOpndList = 0;
            /* get the first effective modifier */
            return VIR_SrcOperand_Iterator_Next(Iter);
        }
        else if ((Iter->expandNodeFlag & VIR_SRCOPERAND_FLAG_EXPAND_ARRAY_NODE)
                 &&
                 VIR_Operand_isArray(opnd))
        {
            /* set operand info for next iteration */
            Iter->specialNode = 1;
            Iter->useOpndList = 0;
            Iter->curNode     = VIR_Operand_GetArrayIndex(opnd);
            /* return the base operand */
            return VIR_Operand_GetArrayBase(opnd);
        }
        MOVE_TO_NEXT_SRC(Iter);
        /* check if the operand if unknown kind */
        if (VIR_Operand_GetOpKind(opnd) == VIR_OPND_UNDEF && Iter->skipUndefs)
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

            if (VIR_Operand_isParameters(opndInInst))
            {
                VIR_ParmPassing *parm = VIR_Operand_GetParameters(opndInInst);

                gcmASSERT(Iter->expandNodeFlag & VIR_SRCOPERAND_FLAG_EXPAND_PARAM_NODE);

                while (Iter->inSrcNo < parm->argNum)
                {
                    opnd = parm->args[Iter->inSrcNo];
                    Iter->inSrcNo++;
                    if (opnd != gcvNULL)
                    {
                        /* found the modifier operand */
                        return opnd;
                    }
                }
            }
            else
            {
                gcmASSERT(VIR_Operand_isTexldParm(opndInInst) && (Iter->expandNodeFlag & VIR_SRCOPERAND_FLAG_EXPAND_TEXLD_PARM_NODE));

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
    OUT VIR_Operand_Iterator *   Iter,
    IN  VIR_SrcOperand_Iter_ExpandFlag ExpandFlag,
    IN  gctBOOL                  SkipUndef
    )
{
    VIR_SrcOperand_Iterator_Init1(Inst, &Iter->header, ExpandFlag, SkipUndef);
    Iter->curNo = 0;
    Iter->texldModifierName = VIR_TEXLDMODIFIER_COUNT;
    Iter->dest = gcvFALSE;
    Iter->skipUndefs = SkipUndef;
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
    if(VIR_Inst_GetDest(Iter->header.inst))
    {
        Iter->dest = gcvTRUE;
        return VIR_Inst_GetDest(Iter->header.inst);
    }

    Iter->curNo++;
    Iter->dest = gcvFALSE;

    opnd =  VIR_SrcOperand_Iterator_First(&Iter->header);
    if(Iter->header.specialNode &&
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

    if(Iter->header.specialNode)
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

void
VIR_IdList_Finalize(VIR_IdList *IdList)
{
    if (IdList->ids)
    {
        vscMM_Free(IdList->memPool, IdList->ids);
    }

    vscMM_Free(IdList->memPool, IdList);
}

VSC_ErrCode
VIR_IdList_Reserve(
    IN OUT VIR_IdList *     IdList,
    IN     gctUINT          Count
    )
{
    VSC_ErrCode  errCode    = VSC_ERR_NONE;

    /* check if Id List has enough space */
    if (IdList->allocated < Count)
    {
        gctUINT newEntries = Count;
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
        errCode = VIR_IdList_Reserve(IdList, newEntries);

    }
    if (errCode == VSC_ERR_NONE)
        IdList->ids[IdList->count++] = Id;

    return errCode;
}

VSC_ErrCode
VIR_IdList_Set(
    IN VIR_IdList *     IdList,
    IN gctUINT          Index,
    IN VIR_Id           Id)
{
    VSC_ErrCode  errCode    = VSC_ERR_NONE;
    /* check if Id List has enough space */
    if (Index >= IdList->allocated)
    {
        gctUINT newEntries = Index+1;
        errCode = VIR_IdList_Reserve(IdList, newEntries);

    }
    if (errCode == VSC_ERR_NONE)
    {
        IdList->ids[Index] = Id;
        if (Index > IdList->count)
        {
            IdList->count = Index;
        }
    }
    return errCode;
}

VSC_ErrCode
VIR_IdList_Copy(
    IN OUT VIR_IdList *     IdList,
    IN     VIR_IdList *     SourceIdList
    )
{
    VSC_ErrCode  errCode    = VSC_ERR_NONE;
    gctUINT i;
    /* check if Id List has enough space */
    if (IdList->allocated < SourceIdList->count)
    {
        gctUINT newEntries = SourceIdList->count + 1;
        errCode = VIR_IdList_Reserve(IdList, newEntries);
    }
    if (errCode == VSC_ERR_NONE)
    {
        for (i = 0; i < SourceIdList->count; i++)
        {
            IdList->ids[i] = SourceIdList->ids[i];
        }
        IdList->count = SourceIdList->count;
    }
    return errCode;
}

VSC_ErrCode
VIR_IdList_DeleteByIndex(
    IN VIR_IdList *     IdList,
    IN gctUINT          Index)
{
    VSC_ErrCode  errCode    = VSC_ERR_NONE;
    gctUINT i;

    gcmASSERT(Index < IdList->count);

    for (i = Index; i < IdList->count -1 ; i++)
    {
        IdList->ids[i] = IdList->ids[i + 1];
    }

    IdList->count--;

    return errCode;
}

VSC_ErrCode
VIR_IdList_DeleteByValue(
    IN VIR_IdList *     IdList,
    IN VIR_Id           Value)
{
    VSC_ErrCode  errCode    = VSC_ERR_NONE;
    gctUINT i;

    for (i = 0; i < IdList->count; i++)
    {
        if (IdList->ids[i] == Value)
        {
            errCode = VIR_IdList_DeleteByIndex(IdList,
                i);
            break;
        }
    }

    return errCode;
}

VIR_Id
VIR_IdList_FindByValue(
    IN VIR_IdList *     IdList,
    IN VIR_Id           Value)
{
    gctUINT i;

    for (i = 0; i < IdList->count; i++)
    {
        if (IdList->ids[i] == Value)
        {
            return i;
        }
    }

    return VIR_INVALID_ID;
}

VSC_ErrCode
VIR_IdList_RenumberIndex(
    IN VIR_Shader *     pShader,
    IN VIR_IdList *     IdList
    )
{
    VSC_ErrCode  errCode = VSC_ERR_NONE;
    VIR_Symbol * sym = gcvNULL;
    VIR_SymbolKind symKind;
    gctUINT i;

    for (i = 0; i < VIR_IdList_Count(IdList); i++)
    {
        sym = VIR_Shader_GetSymFromId(pShader, VIR_IdList_GetId(IdList, i));
        symKind = VIR_Symbol_GetKind(sym);

        switch (symKind)
        {
        case VIR_SYM_UNIFORM:
            {
                VIR_Uniform *uniform = VIR_Symbol_GetUniform(sym);
                if (uniform)
                    VIR_Uniform_SetID(uniform, i);
                break;
            }

        case VIR_SYM_SAMPLER:
            {
                VIR_Uniform *uniform = VIR_Symbol_GetSampler(sym);
                if (uniform)
                    VIR_Uniform_SetID(uniform, i);
                break;
            }

        case VIR_SYM_SAMPLER_T:
            {
                VIR_Uniform *uniform = VIR_Symbol_GetSamplerT(sym);
                if (uniform)
                    VIR_Uniform_SetID(uniform, i);
                break;
            }

        case VIR_SYM_IMAGE:
            {
                VIR_Uniform *uniform = VIR_Symbol_GetImage(sym);
                if (uniform)
                    VIR_Uniform_SetID(uniform, i);
                break;
            }

        case VIR_SYM_IMAGE_T:
            {
                VIR_Uniform *uniform = VIR_Symbol_GetImageT(sym);
                if (uniform)
                    VIR_Uniform_SetID(uniform, i);
                break;
            }

        case VIR_SYM_UBO:
            {
                VIR_UniformBlock *ubo = VIR_Symbol_GetUBO(sym);
                if (ubo)
                    VIR_UBO_SetBlockIndex(ubo, (gctINT16)i);
                break;
            }

        case VIR_SYM_SBO:
            {
                VIR_StorageBlock *sbo = VIR_Symbol_GetSBO(sym);
                if (sbo)
                    VIR_SBO_SetBlockIndex(sbo, (gctINT16)i);
                break;
            }

        case VIR_SYM_IOBLOCK:
            {
                VIR_IOBlock *iob = VIR_Symbol_GetIOB(sym);
                if (iob)
                    VIR_IOBLOCK_SetBlockIndex(iob, (gctINT16)i);
                break;
            }

        default:
            break;
        }
    }

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

void
VIR_ValueList_Finalize(VIR_ValueList *ValueList)
{
    if (ValueList->values)
    {
        vscMM_Free(ValueList->memPool, ValueList->values);
    }

    vscMM_Free(ValueList->memPool, ValueList);
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

    switch (VIR_GetTypeComponentType(VIR_Operand_GetTypeId(Operand)))
    {
    case VIR_TYPE_INT16:
    case VIR_TYPE_UINT16:
        /* fits 16bits */
        return gcvTRUE;
    /* we should treat boolean as int32 (since true can be any non-zero value) */
    case VIR_TYPE_BOOLEAN:
    case VIR_TYPE_INT32:
        return CAN_EXACTLY_CVT_S32_2_S16(VIR_Operand_GetImmediateInt(Operand));
    case VIR_TYPE_UINT32:
        return CAN_EXACTLY_CVT_U32_2_U16(VIR_Operand_GetImmediateUint(Operand));
    case VIR_TYPE_FLOAT32:
        /* float immediate will always be put into uniform */
        return gcvTRUE;
    default:
        gcmASSERT(gcvFALSE);
        break;
    }

    return gcvFALSE;
}

/* return true if sym is special for RA pass, like depth sym */
gctBOOL
VIR_Symbol_SpecialRegAlloc(
    VIR_Symbol     *sym)
{
    if (VIR_Symbol_isVariable(sym) && (VIR_Symbol_GetName(sym) == VIR_NAME_DEPTH))
    {
        return gcvTRUE;
    }
    else if (VIR_Symbol_isVreg(sym) && VIR_Symbol_GetVregVariable(sym))
    {
        VIR_Symbol *varSym = VIR_Symbol_GetVregVariable(sym);
        if (varSym && (VIR_Symbol_GetName(varSym) == VIR_NAME_DEPTH))
        {
            return gcvTRUE;
        }
    }

    return gcvFALSE;
}

static gctBOOL
_VIR_Sym_NeedRunSingleThreadInDual16HighpVec2(
    VIR_Symbol      *Sym)
{
    gcmASSERT(Sym);
    if (VIR_Symbol_GetName(Sym) == VIR_NAME_SAMPLE_ID ||
        VIR_Symbol_GetName(Sym) == VIR_NAME_SAMPLE_POSITION ||
        VIR_Symbol_GetName(Sym) == VIR_NAME_SAMPLE_MASK_IN ||
        VIR_Symbol_SpecialRegAlloc(Sym))
    {
        return gcvTRUE;
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
    IN VIR_Instruction      * VirInst,
    IN VIR_Operand          * Operand,
    OUT gctBOOL             * isHighPrecisionOperand,
    OUT gctBOOL             * isDual16NotSupported,
    OUT gctBOOL             * isVec2orless
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    VIR_OpCode opCode = VIR_Inst_GetOpcode(VirInst);
    VIR_Shader* shader = VIR_Inst_GetShader(VirInst);

    gcmASSERT(isHighPrecisionOperand && isHighPrecisionOperand && isVec2orless);
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
            VIR_Type   *type = VIR_Shader_GetTypeFromId(shader, VIR_Operand_GetTypeId(Operand));

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
            *isVec2orless = gcvTRUE;
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
            VIR_TypeId       typeId = VIR_Operand_GetTypeId(Operand);

            /* ldarr/starr operand could be array type */
            if (!VIR_TypeId_isPrimitive(typeId))
            {
                VIR_Type   *type = VIR_Shader_GetTypeFromId(shader, typeId);
                while (type && !VIR_Type_isPrimitive(type))
                {
                    type = VIR_Shader_GetTypeFromId(shader, VIR_Type_GetBaseTypeId(type));
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
            if (VIR_Symbol_GetStorageClass(sym) == VIR_STORAGE_LOCALSTORAGE)
            {
                if (VIR_Inst_GetDest(VirInst) == Operand)
                {
                    *isDual16NotSupported = gcvTRUE;
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
                    /*check uniform type is vec2-*/
                    if ((!_VIR_Sym_NeedRunSingleThreadInDual16HighpVec2(sym)) &&
                        (VIR_TypeId_isPrimitive(VIR_Symbol_GetTypeId(sym))))
                    {
                        /* now symbol type and operand type both component size <= 2,
                         * since zw channels will be used in highpvec2 dual16
                         */
                        *isVec2orless = ((VIR_GetTypeComponents(VIR_Symbol_GetTypeId(sym)) <= 2) &&
                                         (VIR_GetTypeComponents(typeId) <= 2));
                    }
                }
                break;
            case VIR_SYM_CONST:
                break;
            case VIR_SYM_VIRREG:
            case VIR_SYM_SAMPLER:
            case VIR_SYM_IMAGE:
            case VIR_SYM_IMAGE_T:       /* will be removed */
            case VIR_SYM_TEXTURE:
            case VIR_SYM_VARIABLE:
            case VIR_SYM_FIELD:
                {
                    if (VIR_Operand_GetPrecision(Operand) == VIR_PRECISION_HIGH
                        /*
                        ** We can't use type size to check here because in this instruction, it can be less than 32 bit,
                        ** but in other instruction, the type size of this symbol can be 32 bit, if so, the data is mess up.
                        ** For example:
                        ** 029: CONVERT            hvec2 hp temp(88).hp.xy, vec2 mp  temp(21).mp.xy    // The type size of temp(88) is 2 here.
                        ** 030: AND_BITWISE        uint hp temp(89).hp.x, uint hp  temp(88).hp.x, uint 65535   // The type size of temp(88) is 4 here.
                        */
                        )
                    {
                        *isHighPrecisionOperand = gcvTRUE;
                    }
                    if ((!_VIR_Sym_NeedRunSingleThreadInDual16HighpVec2(sym)) &&
                        (VIR_TypeId_isPrimitive(VIR_Symbol_GetTypeId(sym))))
                    {
                        /* now symbol type and operand type both component size <= 2,
                         * since zw channels will be used in highpvec2 dual16
                         */
                        *isVec2orless = ((VIR_GetTypeComponents(VIR_Symbol_GetTypeId(sym)) <= 2) &&
                                         (VIR_GetTypeComponents(typeId) <= 2));
                    }

                    /*
                    ** To enable DUAL16:
                    **  1) The SRC0 of a 2D/3D IMG instruction can't be a temp register.
                    **  2) The SRC1 of a 3D IMG instruction can't be a temp register.
                    */
                    if (VIR_OPCODE_isImgLd(opCode) || VIR_OPCODE_isImgSt(opCode) || VIR_OPCODE_isImgAddr(opCode))
                    {
                        VIR_OperandInfo opndInfo;

                        if (VIR_Inst_GetSource(VirInst, 0) == Operand)
                        {
                            VIR_Operand_GetOperandInfo(VirInst, Operand, &opndInfo);

                            if (opndInfo.isVreg)
                            {
                                *isDual16NotSupported = gcvTRUE;
                            }
                        }
                        else if (VIR_OPCODE_is3DImageRelated(opCode) && VIR_Inst_GetSource(VirInst, 1) == Operand)
                        {
                            VIR_Operand_GetOperandInfo(VirInst, Operand, &opndInfo);

                            if (opndInfo.isVreg)
                            {
                                *isDual16NotSupported = gcvTRUE;
                            }
                        }
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
    case VIR_OPND_NAME:
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
VIR_Inst_isIntType(
    IN VIR_Instruction *  pInst
    )
{
    VIR_Operand * dest = VIR_Inst_GetDest(pInst);

    if (dest)
    {
        VIR_PrimitiveTypeId tyId = VIR_Operand_GetTypeId(dest);
        if (tyId > VIR_TYPE_LAST_PRIMITIVETYPE)
        {
            return gcvFALSE;
        }
        return (VIR_GetTypeFlag(tyId) & VIR_TYFLAG_ISINTEGER) != 0;
    }

    return gcvFALSE;
}

gctBOOL
VIR_Opcode_Dual16NeedRunInSingleT(
    IN VIR_OpCode   Opcode,
    IN gctBOOL      IsIntType
    )
{
    /* Supported single-t instructions which are unsupported dual-t instructions, by category:
    -* load/store: LOAD, STORE, ATOM_ADD, ATOM_XCHG, ATOM_CMP_XCHG, ATOM_MIN,
    *             ATOM_MAX, ATOM_OR, ATOM_AND, ATOM_XOR.
    -* a0:         MOVAR, MOVAF, MOVAI
    -* OpenCL:     ADDLO, MULLO, SWIZZLE (Opcode=0x2b).
    *             (NOTE: Swizzling of source operands is still supported in dual-t instructions.)
    -* idiv/imod:  IDIV0, IMOD0
    */
    if (VIR_OPCODE_isMemLd(Opcode)    || Opcode == VIR_OP_ILOAD           ||
        Opcode == VIR_OP_IMG_LOAD     || Opcode == VIR_OP_IMG_LOAD_3D     ||
        Opcode == VIR_OP_VX_IMG_LOAD  || Opcode == VIR_OP_VX_IMG_LOAD_3D  ||
        VIR_OPCODE_isMemSt(Opcode)    || Opcode == VIR_OP_ISTORE          ||
        Opcode == VIR_OP_IMG_STORE    || Opcode == VIR_OP_IMG_STORE_3D    ||
        Opcode == VIR_OP_VX_IMG_STORE || Opcode == VIR_OP_VX_IMG_STORE_3D ||
        VIR_OPCODE_isAttrLd(Opcode)   ||
        VIR_OPCODE_isAtom(Opcode)     ||
        Opcode == VIR_OP_MOVA         ||
        Opcode == VIR_OP_LDARR        ||
        Opcode == VIR_OP_STARR        ||
        Opcode == VIR_OP_ADDLO        ||
        Opcode == VIR_OP_MULLO        ||
        Opcode == VIR_OP_SWIZZLE      ||
        Opcode == VIR_OP_IMADLO0      ||
        Opcode == VIR_OP_IMADLO1      ||
        Opcode == VIR_OP_IMADHI0      ||
        Opcode == VIR_OP_IMADHI1      ||
        Opcode == VIR_OP_ARCTRIG      ||
        Opcode == VIR_OP_IMOD         ||
        ((Opcode == VIR_OP_DIV || Opcode == VIR_OP_MOD || Opcode == VIR_OP_REM) && IsIntType)
        )
    {
        return gcvTRUE;
    }
    return gcvFALSE;
}

/* special inst in dual16 highpvec2
 * TODO: undo cases
 * IMULL0, IMULLOSAT0, IMULHI0, ... check VIR_Opcode_Dual16NeedRunInSingleT
 * PACK*
 */
static gctBOOL
_VIR_Inst_NeedRunSingleThreadInDual16HighpVec2(
    VIR_Instruction *pInst
    )
{
    /* 5) CLAMP0MAX with optional SRC2 */
    if(VIR_OP_CLAMP0MAX == VIR_Inst_GetOpcode(pInst) &&
       !(VIR_Inst_GetSource(pInst, 2)))
    {
        return gcvTRUE;
    }
    /* 6) conv src/dest shoudl be immediate */
    else if (VIR_Inst_GetOpcode(pInst) == VIR_OP_CONVERT ||
             VIR_Inst_GetOpcode(pInst) == VIR_OP_CONV0 ||
             VIR_Inst_GetOpcode(pInst) == VIR_OP_CONV)
    {
        VIR_Operand *dest = VIR_Inst_GetDest(pInst);
        gctUINT i;
        if (dest && VIR_Operand_GetOpKind(dest) != VIR_OPND_IMMEDIATE)
        {
            return gcvTRUE;
        }
        for (i = 0; i < VIR_Inst_GetSrcNum(pInst); i++)
        {
            VIR_Operand *src = VIR_Inst_GetSource(pInst, i);
            if (VIR_Operand_GetOpKind(src) != VIR_OPND_IMMEDIATE)
            {
                return gcvTRUE;
            }
        }
    }
    return gcvFALSE;
}

VSC_ErrCode
VIR_Inst_Check4Dual16(
    IN VIR_Instruction          *pInst,
    OUT gctBOOL                 *runSingleT,
    OUT gctBOOL                 *isDual16NotSupported,
    OUT gctBOOL                 *isDual16Highpvec2,
    IN  VSC_OPTN_DUAL16Options  *options,
    IN  VIR_Dumper              *dumper,
    IN  gctBOOL                 HwSupportHIGHVEC2
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    gctUINT     i;
    gctBOOL     needRunSingleT = gcvFALSE;
    gctBOOL     isDestHighPrecision = gcvFALSE;
    gctBOOL     isHighPrecisionOperand = gcvFALSE;
    gctBOOL     isDestVec2orless = gcvFALSE;
    gctBOOL     isSrcVec2orless = gcvFALSE;

    gcmASSERT(runSingleT && isDual16NotSupported);

    if (VIR_Opcode_Dual16NeedRunInSingleT(VIR_Inst_GetOpcode(pInst), VIR_Inst_isIntType(pInst)))
    {
        if(options && VSC_UTILS_MASK(VSC_OPTN_DUAL16Options_GetTrace(options), VSC_OPTN_DUAL16Options_TRACE_DETAIL))
        {
            VIR_LOG(dumper, "needs to run in singleT because of op.\n");
            VIR_LOG_FLUSH(dumper);
        }
        needRunSingleT = gcvTRUE;
    }

    if (VIR_Inst_GetDest(pInst))
    {
        errCode = VIR_Operand_Check4Dual16(pInst, VIR_Inst_GetDest(pInst),
                                           &isDestHighPrecision, isDual16NotSupported, &isDestVec2orless);
        if(isDestHighPrecision && (!HwSupportHIGHVEC2 || !isDestVec2orless))
        {
            /* run single thread for high precision dest if
             * not support dual16-highpvec2 or
             * not dest is not vec2 */
            if(options && VSC_UTILS_MASK(VSC_OPTN_DUAL16Options_GetTrace(options), VSC_OPTN_DUAL16Options_TRACE_DETAIL))
            {
                VIR_LOG(dumper, "needs to run in singleT because of dest.\n");
                VIR_LOG_FLUSH(dumper);
            }
            needRunSingleT |=  isDestHighPrecision;
        }
    }

    /* check source operands */

    for(i = 0; i < VIR_Inst_GetSrcNum(pInst); ++i)
    {
        VIR_Operand *operand =  VIR_Inst_GetSource(pInst, i);
        isHighPrecisionOperand = gcvFALSE;
        isSrcVec2orless = gcvFALSE;
        errCode = VIR_Operand_Check4Dual16(pInst, operand, &isHighPrecisionOperand, isDual16NotSupported, &isSrcVec2orless);
        if(isHighPrecisionOperand &&
           (!HwSupportHIGHVEC2 || !isSrcVec2orless || !isDestHighPrecision)) /* implicit conversion from highp to mediump is not supported in highpvec2 mode */
        {
            /* run single thread for high precision src operand if any of following
             *  1) not support dual16 highpvec2
             *  2) dest.hp.vec2- and src.mp.vec3+
             *  3) dest.mp and src.hp.vec2-
             */
            if(options && VSC_UTILS_MASK(VSC_OPTN_DUAL16Options_GetTrace(options), VSC_OPTN_DUAL16Options_TRACE_DETAIL))
            {
                VIR_LOG(dumper, "needs to run in singleT because of source%d.\n", i);
                VIR_LOG_FLUSH(dumper);
            }
            needRunSingleT |=  isHighPrecisionOperand;
        }
        else if (HwSupportHIGHVEC2 && isDestHighPrecision && !isSrcVec2orless)
        {
            /*run single thread if 4) dest is highpvec2 but one of the operand is mp vec3+ */
            if(options && VSC_UTILS_MASK(VSC_OPTN_DUAL16Options_GetTrace(options), VSC_OPTN_DUAL16Options_TRACE_DETAIL))
            {
                VIR_LOG(dumper, "needs to run in singleT because of source%d while dest is HIGHP.\n", i);
                VIR_LOG_FLUSH(dumper);
            }
            needRunSingleT |=  gcvTRUE;
        }
    }

    /* check special should run single thread in hihgpvec2 dual16 */
    if (HwSupportHIGHVEC2 && !needRunSingleT && isDestHighPrecision)
    {
        needRunSingleT |= _VIR_Inst_NeedRunSingleThreadInDual16HighpVec2(pInst);
        if (!needRunSingleT && isDual16Highpvec2)
        {
            *isDual16Highpvec2 = gcvTRUE;
        }
    }

    *runSingleT = needRunSingleT;
    return errCode;
}

gctUINT
VIR_OpCode_EvaluateOneChannelConstant(
    IN VIR_OpCode           Opcode,
    IN gctUINT              Src0Val,
    IN VIR_TypeId           Src0Type,
    IN gctUINT              Src1Val,
    IN VIR_TypeId           Src1Type,
    OUT VIR_TypeId          *ResultType
    )
{
    gctUINT result = 0;

    gcmASSERT(Src1Type == VIR_TYPE_UNKNOWN || VIR_OPCODE_GetSrcOperandNum(Opcode) == 2);

    if (Src0Type == VIR_TYPE_FLOAT32 || Src1Type == VIR_TYPE_FLOAT32)
    {
        gctFLOAT f0, f1, fres = 0.0;

        gcmASSERT(Src0Type == Src1Type);
        f0 = *(gctFLOAT *)(&Src0Val);
        f1 = *(gctFLOAT *)(&Src1Val);

        switch (Opcode)
        {
        case VIR_OP_ADD:
            fres = (f0 + f1); break;
        case VIR_OP_DIV:
            fres = (f0 / f1); break;
        case VIR_OP_MUL:
            fres = (f0 * f1); break;
        case VIR_OP_SUB:
            fres = (f0 - f1); break;
        default:
            gcmASSERT(0);
        }

        result = *((gctUINT*)&fres);
        if(ResultType)
        {
            *ResultType = VIR_TYPE_FLOAT32;
        }
    }
    else if (VIR_TypeId_isSignedInteger(Src0Type) || VIR_TypeId_isSignedInteger(Src1Type))
    {
        gctINT32 i0 = Src0Val;
        gctINT32 i1 = Src1Val;
        gctINT32 ires = 0;

        switch (Opcode)
        {
        case VIR_OP_ADD:
            ires = (i0 + i1); break;
        case VIR_OP_AND_BITWISE:
            ires = (i0 & i1); break;
        case VIR_OP_DIV:
            ires = (i0 / i1); break;
        case VIR_OP_MUL:
            ires = (i0 * i1); break;
        case VIR_OP_OR_BITWISE:
            ires = (i0 | i1); break;
        case VIR_OP_SUB:
            ires = (i0 - i1); break;
        case VIR_OP_XOR_BITWISE:
            ires = (i0 ^ i1); break;
        default:
            gcmASSERT(0);
        }

        result = ires;
        if(ResultType)
        {
            *ResultType = VIR_TYPE_INT32;
        }
    }
    else
    {
        gcmASSERT(VIR_TypeId_isUnSignedInteger(Src0Type) || VIR_TypeId_isBoolean(Src0Type));

        switch (Opcode)
        {
        case VIR_OP_ADD:
            result = (Src0Val + Src1Val); break;
        case VIR_OP_AND_BITWISE:
            result = (Src0Val & Src1Val); break;
        case VIR_OP_DIV:
            result = (Src0Val / Src1Val); break;
        case VIR_OP_MUL:
            result = (Src0Val * Src1Val); break;
        case VIR_OP_OR_BITWISE:
            result = (Src0Val | Src1Val); break;
        case VIR_OP_SUB:
            result = (Src0Val - Src1Val); break;
        case VIR_OP_XOR_BITWISE:
            result = (Src0Val ^ Src1Val); break;
        default:
            gcmASSERT(0);
        }

        if(ResultType)
        {
            *ResultType = VIR_TYPE_UINT32;
        }
    }

    return result;
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
        VIR_Type    *symType;

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
        if (!isSymUniformUsedInShader(sym) &&
            !isSymUniformUsedInTextureSize(sym) &&
            !isSymUniformUsedInLTC(sym) &&
            /* We can't skip a texture which is from the resource layout. */
            !VIR_Uniform_AlwaysAlloc(Shader, sym))
        {
            continue;
        }

        symType = VIR_Symbol_GetType(sym);

        if (VIR_Type_GetKind(symType) == VIR_TY_ARRAY)
        {
            samplers += (gctINT)VIR_Type_GetArrayLength(symType);
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

VIR_IntrinsicsKind
VIR_IntrinsicGetKind(
    IN  gctUINT setId,
    IN  gctUINT funcId
    )
{
    VIR_IntrinsicsKind intrinsicKind = VIR_IK_NONE;

    if (setId == VIR_INTRINSIC_SET_GLSL)
    {
        intrinsicKind = GLSL_STD_450[funcId];
    }
    else if (setId == VIR_INTRINSIC_SET_INTERNAL)
    {
        intrinsicKind = INTERNAL_INTRINSIC[funcId];
    }
    else if (setId == VIR_INTRINSIC_SET_CL)
    {
    }
    else
    {
        gcmASSERT(gcvFALSE);
    }

    return intrinsicKind;
}

gctUINT
VIR_Shader_GetWorkGroupSize(
    IN VIR_Shader      *pShader
    )
{
    gctUINT workGroupSize = 0;

    gcmASSERT(VIR_Shader_IsGlCompute(pShader) || VIR_Shader_IsCL(pShader));

    if (VIR_Shader_IsGlCompute(pShader))
    {
        gcmASSERT(!VIR_Shader_IsWorkGroupSizeAdjusted(pShader));

        workGroupSize = pShader->shaderLayout.compute.workGroupSize[0] *
                        pShader->shaderLayout.compute.workGroupSize[1] *
                        pShader->shaderLayout.compute.workGroupSize[2];
    }
    else
    {
        if (VIR_Shader_IsWorkGroupSizeFixed(pShader))
        {
            workGroupSize = pShader->shaderLayout.compute.workGroupSize[0] *
                            pShader->shaderLayout.compute.workGroupSize[1] *
                            pShader->shaderLayout.compute.workGroupSize[2];
        }
        else
        {
            workGroupSize = VIR_Shader_GetAdjustedWorkGroupSize(pShader);
        }
    }

    gcmASSERT(workGroupSize != 0);

    return workGroupSize;
}

gctUINT
VIR_Shader_GetMaxFreeRegCountPerThread(
    VIR_Shader      *pShader,
    VSC_HW_CONFIG   *pHwCfg
    )
{
    gctUINT          maxFreeReg = vscGetHWMaxFreeRegCount(pHwCfg);
    gctFLOAT         workGroupSize = 0, threadCount;

    if (VIR_Shader_CalcMaxRegBasedOnWorkGroupSize(pShader))
    {
        /* if compute shader has barrier, the temp count must follow
            ceiling(work_group_size/(shader_core_count*4*threads_per_register)) <= floor(maxFreeReg/temp_register_count)
            */

        /* VIR_SHADER_TESSELLATION_CONTROL should not have barrier if core == 8 */
        gcmASSERT(pShader->shaderKind == VIR_SHADER_COMPUTE || pShader->shaderKind == VIR_SHADER_TESSELLATION_CONTROL);
        threadCount = (gctFLOAT)(pHwCfg->maxCoreCount * 4 * (VIR_Shader_isDual16Mode(pShader) ? 2 : 1));

        if (VIR_Shader_IsGlCompute(pShader) || VIR_Shader_IsCL(pShader))
        {
            /* Use initWorkGroupSizeToCalcRegCount to calculate the maxRegCount if needed. */
            if (!VIR_Shader_IsWorkGroupSizeAdjusted(pShader) &&
                !VIR_Shader_IsWorkGroupSizeFixed(pShader))
            {
                VIR_Shader_SetWorkGroupSizeAdjusted(pShader, gcvTRUE);
                VIR_Shader_SetAdjustedWorkGroupSize(pShader, GetHWInitWorkGroupSizeToCalcRegCount());
            }
            workGroupSize = (gctFLOAT)VIR_Shader_GetWorkGroupSize(pShader);
            maxFreeReg = maxFreeReg / (gctUINT)(ceil(workGroupSize / threadCount));
        }
        else if (pShader->shaderKind == VIR_SHADER_TESSELLATION_CONTROL)
        {
            workGroupSize = (gctFLOAT)(pShader->shaderLayout.tcs.tcsPatchOutputVertices);
            maxFreeReg = maxFreeReg / (gctUINT)(ceil(workGroupSize / threadCount));
        }
    }

    return maxFreeReg;
}

gctUINT
VIR_Shader_NeedToCutDownWorkGroupSize(
    VIR_Shader      *pShader,
    VSC_HW_CONFIG   *pHwCfg
    )
{
    gctUINT         minRegCount = 3;

    if ((VIR_Shader_GetKind(pShader) == VIR_SHADER_COMPUTE) &&
        (VIR_Shader_GetMaxFreeRegCountPerThread(pShader, pHwCfg) <= minRegCount) &&
        (pHwCfg->maxCoreCount == 1 || pHwCfg->maxCoreCount == 2))
    {
        if (pShader->shaderLayout.compute.workGroupSize[0] == 0)
        {
            if (VIR_Shader_GetWorkGroupSize(pShader) % 2 == 0)
            {
                return gcvTRUE;
            }
        }
        else if ((pShader->shaderLayout.compute.workGroupSize[0] % 2 == 0)
                 ||
                 (pShader->shaderLayout.compute.workGroupSize[1] % 2 == 0)
                 ||
                 (pShader->shaderLayout.compute.workGroupSize[2] % 2 == 0))
        {
            return gcvTRUE;
        }
    }

    return gcvFALSE;
}

gctBOOL
VIR_Shader_CalcMaxRegBasedOnWorkGroupSize(
    VIR_Shader      *pShader
    )
{
    if (VIR_Shader_HasHWBarrier(pShader))
    {
        /* if compute shader has barrier, the temp count must follow
            ceiling(work_group_size/(shader_core_count*4*threads_per_register)) <= floor(maxFreeReg/temp_register_count)
            */
        return gcvTRUE;
    }

    /*
    ** If a shader uses the private memory, we use tempRegCount to calculate the concurrent workThreadCount,
    ** so we need to calculate the workGroupSize based on tempRegCount.
    */
    if (VIR_Shader_UsePrivateMem(pShader))
    {
        return gcvTRUE;
    }

    return gcvFALSE;
}

/* return the concurrent workThreadCount. */
gctUINT
VIR_Shader_ComputeWorkThreadNum(
    VIR_Shader      *pShader,
    VSC_HW_CONFIG   *pHwCfg,
    IN gctBOOL      bUse16BitMod
    )
{
    gctUINT maxFreeReg = pHwCfg->maxGPRCountPerCore / 4;
    gctUINT threadCount = pHwCfg->maxCoreCount * 4 * (VIR_Shader_isDual16Mode(pShader) ? 2 : 1);
    gctUINT hwRegCount = VIR_Shader_GetRegWatermark(pShader);
    gctUINT numWorkThread;

    gcmASSERT(VIR_Shader_GetKind(pShader) == VIR_SHADER_COMPUTE);

    /* Since we only use workThreadNum to calculate the private memory, it is OK that our result is larger than expect. */
    numWorkThread = (gctUINT)(floor((gctFLOAT)maxFreeReg / (gctFLOAT)hwRegCount)) * threadCount * pHwCfg->maxClusterCount;

    if (numWorkThread == 0)
    {
        numWorkThread = 1;
    }

    if (bUse16BitMod)
    {
        numWorkThread = vscAlignToPow2(numWorkThread, 16);
    }

    if (!bUse16BitMod || numWorkThread * 2 < 0x10000)
    {
        numWorkThread *= 2;
    }

    return numWorkThread;
}

/* return the number of workgroups launched at the same time */
gctUINT
VIR_Shader_ComputeWorkGroupNum(
    IN VIR_Shader      *pShader,
    IN VSC_HW_CONFIG   *pHwCfg,
    IN gctBOOL          bUse16BitMod
    )
{
    gctUINT numWorkGroup;
    gctUINT maxFreeReg = pHwCfg->maxGPRCountPerCore / 4;
    gctUINT hwRegCount = VIR_Shader_GetRegWatermark(pShader);
    gctUINT workGroupSize = 0;
    gctUINT threadCount = pHwCfg->maxCoreCount * 4 * (VIR_Shader_isDual16Mode(pShader) ? 2 : 1);

    gcmASSERT(VIR_Shader_IsGlCompute(pShader) || VIR_Shader_IsCL(pShader));

    workGroupSize = VIR_Shader_GetWorkGroupSize(pShader);

    /* Since we only use workGroupNumber to calculate the localMemory, it is OK that our result is larger than expect. */
    if (workGroupSize > threadCount)
    {
        gctFLOAT minHwThreadCountPerGroup = (gctFLOAT)ceil((gctFLOAT)workGroupSize / (gctFLOAT)threadCount);
        numWorkGroup = (gctUINT)(floor((gctFLOAT)maxFreeReg / (minHwThreadCountPerGroup * hwRegCount)) * minHwThreadCountPerGroup) * pHwCfg->maxClusterCount;
    }
    else
    {
        numWorkGroup = (gctUINT)ceil(floor((gctFLOAT)maxFreeReg / (gctFLOAT)hwRegCount) * (gctFLOAT)threadCount / (gctFLOAT)workGroupSize) * pHwCfg->maxClusterCount;
    }

    if (numWorkGroup == 0)
    {
        numWorkGroup = 1;
    }

    if (bUse16BitMod)
    {
        numWorkGroup = vscAlignToPow2(numWorkGroup, 16);
    }

    if (!bUse16BitMod || numWorkGroup * 2 < 0x10000)
    {
        numWorkGroup *= 2;
    }

    return numWorkGroup;
}

/* return the workGroupCount per shader group. */
gctUINT
VIR_Shader_ComputeWorkGroupNumPerShaderGroup(
    IN VIR_Shader      *pShader,
    IN VSC_HW_CONFIG   *pHwCfg
    )
{
    gctUINT             workGroupCountPerShaderGroup = 0;
    gctUINT             threadCount = pHwCfg->maxCoreCount * 4 * (VIR_Shader_isDual16Mode(pShader) ? 2 : 1);
    gctUINT             workGroupSize = VIR_Shader_GetWorkGroupSize(pShader);
    gctUINT             localStoargeSize = VIR_Shader_GetShareMemorySize(pShader);
    gctUINT             localStorageCount = 0;
    /*
    ** If local storage is used, the result is:
    **  min(
    **      floor(SHADER_CORE_COUNT * (dual16Enable ? 8 : 4) / WorkGroupSize),
    **      floor(LSSizeInHW / NumLSPerWorkGroup)
    **      )
    ** otherwise the result is:
    **  floor(SHADER_CORE_COUNT * (dual16Enable ? 8 : 4) / WorkGroupSize)
    */
    workGroupCountPerShaderGroup = (gctUINT)(floor((gctFLOAT)threadCount / (gctFLOAT)workGroupSize));

    if (VIR_Shader_UseLocalMem(pShader) && (localStoargeSize > 0))
    {
        localStorageCount = (gctUINT)(floor((gctFLOAT)pHwCfg->maxLocalMemSizeInByte / (gctFLOAT)localStoargeSize));
        workGroupCountPerShaderGroup = vscMIN(workGroupCountPerShaderGroup, localStorageCount);
    }

    return workGroupCountPerShaderGroup;
}

gctBOOL
VIR_Shader_CheckWorkGroupSizeFixed(
    IN VIR_Shader      *pShader
    )
{
    /* Check CS/CL only. */
    if (VIR_Shader_GetKind(pShader) == VIR_SHADER_COMPUTE)
    {
        if (!VIR_Shader_IsWorkGroupSizeFixed(pShader))
        {
            return gcvFALSE;
        }
    }

    return gcvTRUE;
}

gctBOOL
VIR_Shader_AdjustWorkGroupSize(
    IN VIR_Shader      *pShader,
    IN VSC_HW_CONFIG   *pHwCfg,
    IN gctBOOL          bReduceWorkGroupSize,
    IN gctUINT          adjustWorkGroupSize
    )
{
    gctBOOL             adjusted = gcvFALSE;
    gctUINT             workGroupSize = 0;
    gctUINT             maxWorkGroupSize = pHwCfg->maxWorkGroupSize;
    gctUINT             minWorkGroupSize = pHwCfg->minWorkGroupSize;

    /* Adjust the workGroupSize only it is not fixed. */
    if (!VIR_Shader_CheckWorkGroupSizeFixed(pShader))
    {
        if (VIR_Shader_IsCL(pShader))
        {
            workGroupSize = VIR_Shader_GetAdjustedWorkGroupSize(pShader);

            if (bReduceWorkGroupSize)
            {
                if (((gctINT)workGroupSize - (gctINT)adjustWorkGroupSize) >= (gctINT)minWorkGroupSize)
                {
                    workGroupSize -= adjustWorkGroupSize;
                    adjusted = gcvTRUE;
                }
            }
            else
            {
                if (workGroupSize + adjustWorkGroupSize <= maxWorkGroupSize)
                {
                    workGroupSize += adjustWorkGroupSize;
                    adjusted = gcvTRUE;
                }
            }
        }
    }

    if (adjusted)
    {
        VIR_Shader_SetAdjustedWorkGroupSize(pShader, workGroupSize);
        VIR_Shader_SetWorkGroupSizeAdjusted(pShader, gcvTRUE);
    }

    return adjusted;
}

static gctBOOL
_CheckIsTheParamOpndSym(
    IN VIR_Symbol      *pParmRegSym,
    IN VIR_Operand     *pOpnd
    )
{
    gctBOOL             bMatch = gcvFALSE;
    VIR_Symbol         *pSym = gcvNULL;

    if (VIR_Operand_isVirReg(pOpnd) || VIR_Operand_isSymbol(pOpnd) || (VIR_Operand_GetOpKind(pOpnd) == VIR_OPND_SAMPLER_INDEXING))
    {
        pSym = VIR_Operand_GetSymbol(pOpnd);

        if (pSym == pParmRegSym)
        {
            bMatch = gcvTRUE;
        }
        else if (VIR_Symbol_isVariable(pSym) && pSym == VIR_Symbol_GetVregVariable(pParmRegSym))
        {
            bMatch = gcvTRUE;
        }
    }

    return bMatch;
}

/*
** We can't use variable symbol to find the parmInst because a variable symbol can have more than one vreg symbol,
** we need to use vreg symbol to find the parmInst.
*/
VIR_Instruction*
VIR_Shader_FindParmInst(
    IN VIR_Function    *pCalleeFunc,
    IN VIR_Instruction *pCallInst,
    IN gctBOOL          bForward,
    IN VIR_Symbol      *parmRegSym,
    INOUT VIR_Operand  **ppOpnd
    )
{
    VIR_Instruction *pInst = pCallInst;
    VIR_Operand     *pOpnd;
    gctUINT32       i, j;

    gcmASSERT(VIR_Symbol_isVreg(parmRegSym));

    while (pInst != gcvNULL)
    {
        if (bForward)
        {
            pInst = VIR_Inst_GetPrev(pInst);
            if (pInst == gcvNULL ||
                (VIR_Inst_GetOpcode(pInst) == VIR_OP_CALL && VIR_Operand_GetFunction(VIR_Inst_GetDest(pInst)) == pCalleeFunc))
            {
                return gcvNULL;
            }

            if (VIR_OPCODE_hasDest(VIR_Inst_GetOpcode(pInst)))
            {
                pOpnd = VIR_Inst_GetDest(pInst);

                if (_CheckIsTheParamOpndSym(parmRegSym, pOpnd))
                {
                    if (ppOpnd)
                    {
                        *ppOpnd = pOpnd;
                    }
                    return pInst;
                }
            }
        }
        else
        {
            pInst = VIR_Inst_GetNext(pInst);
            if (pInst == gcvNULL ||
                (VIR_Inst_GetOpcode(pInst) == VIR_OP_CALL && VIR_Operand_GetFunction(VIR_Inst_GetDest(pInst)) == pCalleeFunc))
            {
                return gcvNULL;
            }

            for (i = 0; i < VIR_Inst_GetSrcNum(pInst); i++)
            {
                pOpnd = VIR_Inst_GetSource(pInst, i);

                if (VIR_Operand_isParameters(pOpnd))
                {
                    VIR_ParmPassing *pParm = VIR_Operand_GetParameters(pOpnd);

                    for (j = 0; j < pParm->argNum; j++)
                    {
                        if (_CheckIsTheParamOpndSym(parmRegSym, pParm->args[j]))
                        {
                            if (ppOpnd)
                            {
                                *ppOpnd = pParm->args[j];
                            }
                            return pInst;
                        }
                    }
                }
                else if (_CheckIsTheParamOpndSym(parmRegSym, pOpnd))
                {
                    if (ppOpnd)
                    {
                        *ppOpnd = pOpnd;
                    }
                    return pInst;
                }
            }
        }
    }

    /* pInst may be NULL if the parameter is not used or be optimized (by copy
     * propagation or DCE, etc.)
     */

    return pInst;
}

/* count the code in the shader (pShader):
 *   gctUINT pCodeCounter[VIR_OP_MAXOPCODE];
 */
void VIR_Shader_CountCode(
    IN VIR_Shader             *pShader,
    OUT gctUINT               *pCodeCounter
    )
{
    VIR_FunctionNode*  pFuncNode;
    VIR_FuncIterator   funcIter;
    VIR_Instruction*   pInst;
    VIR_InstIterator   instIter;

    VIR_FuncIterator_Init(&funcIter, &pShader->functions);
    pFuncNode = VIR_FuncIterator_First(&funcIter);

    for(; pFuncNode != gcvNULL; pFuncNode = VIR_FuncIterator_Next(&funcIter))
    {
        VIR_Function *pFunc = pFuncNode->function;

        VIR_InstIterator_Init(&instIter, &pFunc->instList);
        pInst = VIR_InstIterator_First(&instIter);

        for (; pInst != gcvNULL; pInst = VIR_InstIterator_Next(&instIter))
        {
            VIR_OpCode  opcode  =  VIR_Inst_GetOpcode(pInst);
            gcmASSERT(VIR_OP_MAXOPCODE > opcode);
            pCodeCounter[(gctINT)opcode]++;
        }
    }
}

gctBOOL
VIR_Shader_CanRemoveUnusedFunctions(
    IN  VIR_Shader*         pShader
    )
{
    gctBOOL                 bCanRemoveUnusedFunctions = gcvTRUE;

    if (VIR_Shader_IsPatchLib(pShader))
    {
        bCanRemoveUnusedFunctions = gcvFALSE;
    }

    return bCanRemoveUnusedFunctions;
}

/* if bound check the address has 3 components:
 *   .x: 32 bit address points to the memory to be accessed
 *   .y: start address of the valid memory block to access
 *   .z: end address of the valid memory block to access
 */
VIR_Uniform *
VIR_Shader_GetTempRegSpillAddrUniform(
    IN VIR_Shader *  Shader,
    IN gctBOOL       bNeedBoundsCheck
    )
{
    VSC_ErrCode  errCode = VSC_ERR_NONE;
    VIR_Symbol  *spillMemSym;
    VIR_Uniform *virUniform = gcvNULL;
    gctUINT      offset = 0;
    gctCHAR      spillMemAddrName[64];
    VIR_TypeId   addressTypeId = bNeedBoundsCheck ? VIR_TYPE_UINT_X3 : VIR_TYPE_UINT32;

    gcoOS_PrintStrSafe(spillMemAddrName,
        64,
        &offset,
        "#TempRegSpillMemAddr%d",
        Shader->_id);

    spillMemSym = VIR_Shader_FindSymbolByName(Shader, VIR_SYM_UNIFORM, spillMemAddrName);
    if (spillMemSym != gcvNULL)
    {
        gcmASSERT(VIR_Symbol_GetTypeId(spillMemSym) == addressTypeId);
        return VIR_Symbol_GetUniform(spillMemSym);
    }

    /* not found add a new one */
    errCode = VIR_Shader_AddNamedUniform(Shader, spillMemAddrName,
                             VIR_Shader_GetTypeFromId(Shader, addressTypeId), &spillMemSym);

    ON_ERROR(errCode, "Failed AddTempRegSpillMemAddrUniform");

    VIR_Symbol_SetUniformKind(spillMemSym, VIR_UNIFORM_TEMP_REG_SPILL_MEM_ADDRESS);
    VIR_Symbol_SetFlag(spillMemSym, VIR_SYMUNIFORMFLAG_USED_IN_SHADER);
    VIR_Symbol_SetFlag(spillMemSym, VIR_SYMFLAG_COMPILER_GEN);
    VIR_Symbol_SetLocation(spillMemSym, -1);
    VIR_Symbol_SetPrecision(spillMemSym, VIR_PRECISION_HIGH);

    virUniform = VIR_Symbol_GetUniform(spillMemSym);
    virUniform->index = VIR_IdList_Count(VIR_Shader_GetUniforms(Shader)) - 1;

OnError:
    return virUniform;
}

VIR_Uniform *
VIR_Shader_GetClipDistanceEnableUniform(
    IN VIR_Shader *  Shader
    )
{
    VSC_ErrCode  errCode = VSC_ERR_NONE;
    VIR_Symbol  *clipDistanceEnableSym;
    VIR_Uniform *virUniform = gcvNULL;

    clipDistanceEnableSym = VIR_Shader_FindSymbolByName(Shader, VIR_SYM_UNIFORM, "#clipDistanceEnable");
    if (clipDistanceEnableSym != gcvNULL)
    {
        return VIR_Symbol_GetUniform(clipDistanceEnableSym);
    }

    /* not found add a new one */
    errCode = VIR_Shader_AddNamedUniform(Shader, "#clipDistanceEnable",
                             VIR_Shader_GetTypeFromId(Shader, VIR_TYPE_INT32), &clipDistanceEnableSym);

    ON_ERROR(errCode, "Failed to add clipDistanceEnable uniform");

    VIR_Symbol_SetUniformKind(clipDistanceEnableSym, VIR_UNIFORM_CLIP_DISTANCE_ENABLE);
    VIR_Symbol_SetFlag(clipDistanceEnableSym, VIR_SYMUNIFORMFLAG_USED_IN_SHADER);
    VIR_Symbol_SetFlag(clipDistanceEnableSym, VIR_SYMFLAG_COMPILER_GEN);
    VIR_Symbol_SetLocation(clipDistanceEnableSym, -1);
    VIR_Symbol_SetPrecision(clipDistanceEnableSym, VIR_PRECISION_HIGH);

    virUniform = VIR_Symbol_GetUniform(clipDistanceEnableSym);
    virUniform->index = VIR_IdList_Count(VIR_Shader_GetUniforms(Shader)) - 1;

OnError:
    return virUniform;
}

gctBOOL VirSHADER_DumpCodeGenVerbose(void * Shader)
{
    gctINT ShaderId = ((VIR_Shader *)Shader)->_id;
    gcOPTIMIZER_OPTION * option = gcmGetOptimizerOption();

    if (!VIR_Shader_DisableIRDump((VIR_Shader *)Shader) && option->dumpBEVerbose &&
        !(VIR_Shader_IsLibraryShader((VIR_Shader *)Shader) && !option->includeLib))
    {
        gctINT   startId = option->_dumpStart;
        gctINT   endId   = option->_dumpEnd;
        return gcDoTriageForShaderId(ShaderId, startId, endId);
    }
    return gcvFALSE;
}

VSC_ErrCode
VIR_Shader_GenInvocationIndex(
    IN  VIR_Shader              *Shader,
    IN  VIR_Function            *pFunc,
    IN  VIR_Symbol              *VariableSym,
    IN  VIR_Instruction         *insertBeforeInst,
    IN  gctBOOL                 bUpdateSlot
    )
{
    VSC_ErrCode     errCode  = VSC_ERR_NONE;
    VIR_Instruction *mul1Inst = gcvNULL, *mul2Inst = gcvNULL;
    VIR_Instruction *add1Inst = gcvNULL, *add2Inst = gcvNULL;
    VIR_Operand     *src = gcvNULL;
    VIR_SymId        newVarSymId, tmpSymId1, tmpSymId2, tmpSymId3;
    VIR_SymId        tmpSymId = VIR_INVALID_ID, IndexSymId = VIR_INVALID_ID;
    VIR_Symbol       *newVarSym = gcvNULL;
    VIR_VirRegId     regId = VIR_INVALID_ID;
#if gcmIS_DEBUG(gcdDEBUG_ASSERT)
    gctSTRING        glLocalInvIndexStrName = "glLocalinvocationIndex";
#endif
    gctUINT         i;
    VIR_AttributeIdList *attIdList = VIR_Shader_GetAttributes(Shader);
    VIR_StorageClass     storageClass = VIR_Symbol_GetStorageClass(VariableSym);

    gcmASSERT(VIR_Symbol_GetName(VariableSym) == VIR_NAME_LOCALINVOCATIONINDEX ||
              (gcoOS_StrCmp(VIR_Shader_GetSymNameString(Shader, VariableSym), glLocalInvIndexStrName) == gcvSTATUS_OK));

    /* VIR_Shader_GenInvocationIndex also be called for atomic patch function,
     * temp varialbe "glLocalinvocationIndex" which storageClass is VIR_STORAGE_UNKNOWN
     * is used to represent gl_LocalInvocationIndex
     */
    gcmASSERT(storageClass == VIR_STORAGE_INPUT || storageClass == VIR_STORAGE_UNKNOWN);

    if (storageClass == VIR_STORAGE_UNKNOWN)
    {
        storageClass = VIR_STORAGE_INPUT;
    }

    if (bUpdateSlot)
    {
        VIR_Symbol  *pVregSym = VIR_Shader_FindSymbolByTempIndex(Shader, VIR_Symbol_GetVariableVregIndex(VariableSym));
        VIR_SymId  newSymId;
        IndexSymId = VIR_Symbol_GetIndex(pVregSym);
        /* original pVregSym will be used as a temp variable while VariableSym "gl_LocalInvocationIndex" will be tagged with UNUSED
         * create a temp for invocation index for VariableSym */
        regId = VIR_Shader_NewVirRegId(Shader, 1);
        errCode = VIR_Shader_AddSymbol(Shader,
                    VIR_SYM_VIRREG,
                    regId,
                    VIR_Shader_GetTypeFromId(Shader, VIR_TYPE_UINT32),
                    VIR_STORAGE_UNKNOWN,
                    &newSymId);
        VIR_Symbol_SetVariableVregIndex(VariableSym, regId);
        VIR_Symbol_SetVregVarSymId(pVregSym, VIR_INVALID_ID); /* VregSym is no attribute anymore */
    }
    else
    {
        /* create a temp for invocation index */
        regId = VIR_Shader_NewVirRegId(Shader, 1);
        errCode = VIR_Shader_AddSymbol(Shader,
                    VIR_SYM_VIRREG,
                    regId,
                    VIR_Shader_GetTypeFromId(Shader, VIR_TYPE_UINT32),
                    VIR_STORAGE_UNKNOWN,
                    &IndexSymId);
        VIR_Symbol_SetVariableVregIndex(VariableSym, regId);
    }

    VIR_Symbol_ClrFlag(VariableSym, VIR_SYMFLAG_ENABLED | VIR_SYMFLAG_STATICALLY_USED);
    VIR_Symbol_SetFlag(VariableSym, VIR_SYMFLAG_UNUSED);

    /* add an attribute if not found - LocalInvocationID */
    for (i = 0;  i< VIR_IdList_Count(attIdList); i++)
    {
        VIR_Symbol*attr = VIR_Shader_GetSymFromId(Shader, VIR_IdList_GetId(attIdList, i));
        if (VIR_Symbol_GetName(attr) == VIR_NAME_LOCAL_INVOCATION_ID)
        {
            newVarSym = attr;
            break;
        }
    }

    if (i == VIR_IdList_Count(attIdList))
    {
        gctUINT                     nextAttrLlSlot = 0;

        if (bUpdateSlot)
        {
            VIR_AttributeIdList*    pAttrIdLsts = VIR_Shader_GetAttributes(Shader);
            gctUINT                 attrCount = VIR_IdList_Count(pAttrIdLsts);
            gctUINT                 attrIdx;

            for (attrIdx = 0; attrIdx < attrCount; attrIdx ++)
            {
                VIR_SymId       attrSymId = VIR_IdList_GetId(pAttrIdLsts, attrIdx);
                VIR_Symbol      *pAttrSym = VIR_Shader_GetSymFromId(Shader, attrSymId);
                gctUINT         thisOutputRegCount;

                if (!isSymUnused(pAttrSym) && !isSymVectorizedOut(pAttrSym))
                {
                    gcmASSERT(VIR_Symbol_GetFirstSlot(pAttrSym) != NOT_ASSIGNED);

                    thisOutputRegCount = VIR_Symbol_GetVirIoRegCount(Shader, pAttrSym);
                    if (nextAttrLlSlot < (VIR_Symbol_GetFirstSlot(pAttrSym) + thisOutputRegCount))
                    {
                        nextAttrLlSlot = VIR_Symbol_GetFirstSlot(pAttrSym) + thisOutputRegCount;
                    }
                }
            }
        }

        errCode = VIR_Shader_AddSymbol(Shader,
                                       VIR_SYM_VARIABLE,
                                       VIR_NAME_LOCAL_INVOCATION_ID,
                                       VIR_Shader_GetTypeFromId(Shader, VIR_TYPE_UINT_X4),
                                       storageClass,
                                       &newVarSymId);
        CHECK_ERROR(errCode, "VIR_Shader_AddSymbol failed.");
        newVarSym = VIR_Shader_GetSymFromId(Shader, newVarSymId);
        VIR_Symbol_SetFlag(newVarSym, VIR_SYMFLAG_ENABLED | VIR_SYMFLAG_STATICALLY_USED);

        /* create a temp for invocation id */
        regId = VIR_Shader_NewVirRegId(Shader, 1);
        errCode = VIR_Shader_AddSymbol(Shader,
                    VIR_SYM_VIRREG,
                    regId,
                    VIR_Shader_GetTypeFromId(Shader, VIR_TYPE_UINT_X3),
                    VIR_STORAGE_UNKNOWN,
                    &tmpSymId);

        VIR_Symbol_SetVariableVregIndex(newVarSym, regId);
        VIR_Symbol_SetIndexRange(newVarSym, regId + 1);
        VIR_Symbol_SetVregVariable(VIR_Shader_GetSymFromId(Shader, tmpSymId), newVarSym);
        VIR_Symbol_SetIndexRange(VIR_Shader_GetSymFromId(Shader, tmpSymId), regId + 1);

        if (bUpdateSlot)
        {
            VIR_Symbol_SetFirstSlot(newVarSym, nextAttrLlSlot);
        }
    }

    /* Compute local invocation index :
           Z * I * J + Y * I + X
           where local Id = (X, Y, Z) and
                 work group size = (I, J, K)  */

    /* (Y, Z) * I */
    if (insertBeforeInst)
    {
        errCode = VIR_Function_AddInstructionBefore(pFunc,
                        VIR_OP_MUL,
                        VIR_TYPE_UINT_X2,
                        insertBeforeInst,
                        gcvTRUE,
                        &mul1Inst);
    }
    else
    {
        errCode = VIR_Function_PrependInstruction(pFunc,
                        VIR_OP_MUL,
                        VIR_TYPE_UINT_X2,
                        &mul1Inst);
    }

    CHECK_ERROR(errCode, "VIR_Function_PrependInstruction failed.");

    /* src0 - (Y, Z) */
    src = VIR_Inst_GetSource(mul1Inst, 0);
    VIR_Operand_SetOpKind(src, VIR_OPND_SYMBOL);
    VIR_Operand_SetTypeId(src, VIR_TYPE_UINT_X2);
    VIR_Operand_SetSym(src, newVarSym);
    VIR_Operand_SetSwizzle(src, VIR_SWIZZLE_YZZZ);

    /* src1 - workGroupSize[0] */
    src = VIR_Inst_GetSource(mul1Inst, 1);
    VIR_Operand_SetImmediateUint(src, Shader->shaderLayout.compute.workGroupSize[0]);

    /* dest */
    errCode = VIR_Shader_AddSymbol(Shader,
            VIR_SYM_VIRREG,
            VIR_Shader_NewVirRegId(Shader, 1),
            VIR_Shader_GetTypeFromId(Shader, VIR_TYPE_UINT_X2),
            VIR_STORAGE_UNKNOWN,
            &tmpSymId1);

    VIR_Operand_SetTempRegister(VIR_Inst_GetDest(mul1Inst),
                                pFunc,
                                tmpSymId1,
                                VIR_TYPE_UINT_X2);
    VIR_Operand_SetEnable(VIR_Inst_GetDest(mul1Inst), VIR_ENABLE_XY);

    /* (Z * I) * J */
    errCode = VIR_Function_AddInstructionAfter(pFunc,
                        VIR_OP_MUL,
                        VIR_TYPE_UINT32,
                        mul1Inst,
                        gcvTRUE,
                        &mul2Inst);

    CHECK_ERROR(errCode, "VIR_Function_PrependInstruction failed.");

    /* src0 */
    src = VIR_Inst_GetSource(mul2Inst, 0);
    VIR_Operand_SetTempRegister(src,
                                pFunc,
                                tmpSymId1,
                                VIR_TYPE_UINT32);
    VIR_Operand_SetSwizzle(src, VIR_SWIZZLE_YYYY);

    /* src1 - workGroupSize[1] */
    src = VIR_Inst_GetSource(mul2Inst, 1);
    VIR_Operand_SetImmediateUint(src, Shader->shaderLayout.compute.workGroupSize[1]);

    /* dest */
    errCode = VIR_Shader_AddSymbol(Shader,
            VIR_SYM_VIRREG,
            VIR_Shader_NewVirRegId(Shader, 1),
            VIR_Shader_GetTypeFromId(Shader, VIR_TYPE_UINT32),
            VIR_STORAGE_UNKNOWN,
            &tmpSymId2);

    VIR_Operand_SetTempRegister(VIR_Inst_GetDest(mul2Inst),
                                pFunc,
                                tmpSymId2,
                                VIR_TYPE_UINT32);
    VIR_Operand_SetEnable(VIR_Inst_GetDest(mul2Inst), VIR_ENABLE_X);

    /* (Z * I) * J + (Y * I) */
    errCode = VIR_Function_AddInstructionAfter(pFunc,
                        VIR_OP_ADD,
                        VIR_TYPE_UINT32,
                        mul2Inst,
                        gcvTRUE,
                        &add1Inst);

    CHECK_ERROR(errCode, "VIR_Function_PrependInstruction failed.");

    /* src0 */
    src = VIR_Inst_GetSource(add1Inst, 0);
    VIR_Operand_SetTempRegister(src,
                                pFunc,
                                tmpSymId2,
                                VIR_TYPE_UINT32);
    VIR_Operand_SetSwizzle(src, VIR_SWIZZLE_XXXX);

    /* src1 */
    src = VIR_Inst_GetSource(add1Inst, 1);
    VIR_Operand_SetTempRegister(src,
                                pFunc,
                                tmpSymId1,
                                VIR_TYPE_UINT32);
    VIR_Operand_SetSwizzle(src, VIR_SWIZZLE_XXXX);

    /* dest */
    errCode = VIR_Shader_AddSymbol(Shader,
            VIR_SYM_VIRREG,
            VIR_Shader_NewVirRegId(Shader, 1),
            VIR_Shader_GetTypeFromId(Shader, VIR_TYPE_UINT32),
            VIR_STORAGE_UNKNOWN,
            &tmpSymId3);

    VIR_Operand_SetTempRegister(VIR_Inst_GetDest(add1Inst),
                                pFunc,
                                tmpSymId3,
                                VIR_TYPE_UINT32);
    VIR_Operand_SetEnable(VIR_Inst_GetDest(add1Inst), VIR_ENABLE_X);

    /* (Z * I) * J + (Y * I)  + X*/
    errCode = VIR_Function_AddInstructionAfter(pFunc,
                        VIR_OP_ADD,
                        VIR_TYPE_UINT32,
                        add1Inst,
                        gcvTRUE,
                        &add2Inst);

    CHECK_ERROR(errCode, "VIR_Function_PrependInstruction failed.");

    /* src0 */
    src = VIR_Inst_GetSource(add2Inst, 0);
    VIR_Operand_SetTempRegister(src,
                                pFunc,
                                tmpSymId3,
                                VIR_TYPE_UINT32);
    VIR_Operand_SetSwizzle(src, VIR_SWIZZLE_XXXX);

    /* src1 */
    src = VIR_Inst_GetSource(add2Inst, 1);
    VIR_Operand_SetOpKind(src, VIR_OPND_SYMBOL);
    VIR_Operand_SetTypeId(src, VIR_TYPE_UINT32);
    VIR_Operand_SetSym(src, newVarSym);
    VIR_Operand_SetSwizzle(src, VIR_SWIZZLE_XXXX);

    /* dest */
    VIR_Operand_SetTempRegister(VIR_Inst_GetDest(add2Inst),
                                pFunc,
                                IndexSymId,
                                VIR_TYPE_UINT32);
    VIR_Operand_SetEnable(VIR_Inst_GetDest(add2Inst), VIR_ENABLE_X);

    return errCode;
}

/* Those functions are used in the PASS only. */
VSC_ErrCode
VIR_Pass_RemoveInstruction(
    IN VIR_Function*    pFunction,
    IN VIR_Instruction* pInst,
    INOUT gctBOOL*      pInvalidCFG
    )
{
    VSC_ErrCode         errCode  = VSC_ERR_NONE;
    VIR_BASIC_BLOCK*    pBB = VIR_Inst_GetBasicBlock(pInst);

    VIR_Function_RemoveInstruction(pFunction, pInst, gcvTRUE);

    /* If there is no instruction within one BB, we need to rebuild CFG. */
    if (pInvalidCFG && pBB && BB_GET_LENGTH(pBB) == 0)
    {
        *pInvalidCFG = gcvTRUE;
    }

    return errCode;
}

VSC_ErrCode
VIR_Pass_DeleteInstruction(
    IN VIR_Function*    pFunction,
    IN VIR_Instruction* pInst,
    INOUT gctBOOL*      pInvalidCFG
    )
{
    VSC_ErrCode         errCode  = VSC_ERR_NONE;
    VIR_BASIC_BLOCK*    pBB = VIR_Inst_GetBasicBlock(pInst);

    VIR_Function_DeleteInstruction(pFunction, pInst);

    /* If there is no instruction within one BB, we need to rebuild CFG. */
    if (pInvalidCFG && pBB && BB_GET_LENGTH(pBB) == 0)
    {
        *pInvalidCFG = gcvTRUE;
    }

    return errCode;
}

VSC_ErrCode
VIR_Pass_MoveInstructionBefore(
    IN VIR_Function*    pMoveFunction,
    IN VIR_Instruction* pBeforeMe,
    IN VIR_Instruction* pInst,
    INOUT gctBOOL*      pInvalidCFG
    )
{
    VSC_ErrCode         errCode  = VSC_ERR_NONE;
    VIR_BASIC_BLOCK*    pBB = VIR_Inst_GetBasicBlock(pInst);

    VIR_Function_MoveInstructionBefore(pMoveFunction, pBeforeMe, pInst);

    /* If there is no instruction within one BB, we need to rebuild CFG. */
    if (pInvalidCFG && pBB && BB_GET_LENGTH(pBB) == 0)
    {
        *pInvalidCFG = gcvTRUE;
    }

    return errCode;
}

VIR_UniformKind
VIR_Resouce_ResType2UniformKind(
    IN VSC_SHADER_RESOURCE_TYPE    resType
    )
{
    VIR_UniformKind uniformKind = VIR_UNIFORM_NORMAL;

    switch(resType)
    {
    case VSC_SHADER_RESOURCE_TYPE_SAMPLER:
    case VSC_SHADER_RESOURCE_TYPE_UNIFORM_TEXEL_BUFFER:
    case VSC_SHADER_RESOURCE_TYPE_COMBINED_IMAGE_SAMPLER:
    case VSC_SHADER_RESOURCE_TYPE_SAMPLED_IMAGE:
    case VSC_SHADER_RESOURCE_TYPE_STORAGE_IMAGE:
    case VSC_SHADER_RESOURCE_TYPE_STORAGE_TEXEL_BUFFER:
    case VSC_SHADER_RESOURCE_TYPE_INPUT_ATTACHMENT:
        uniformKind = VIR_UNIFORM_NORMAL;
        break;

    case VSC_SHADER_RESOURCE_TYPE_UNIFORM_BUFFER:
    case VSC_SHADER_RESOURCE_TYPE_UNIFORM_BUFFER_DYNAMIC:
        uniformKind = VIR_UNIFORM_UNIFORM_BLOCK_ADDRESS;
        break;

    case VSC_SHADER_RESOURCE_TYPE_STORAGE_BUFFER:
    case VSC_SHADER_RESOURCE_TYPE_STORAGE_BUFFER_DYNAMIC:
        uniformKind = VIR_UNIFORM_STORAGE_BLOCK_ADDRESS;
        break;

    default:
        break;
    }

    return uniformKind;
}

gctUINT
VIR_Resouce_FindResUniform(
    IN VIR_Shader*                  pShader,
    IN VIR_UniformKind              uniformKind,
    IN VSC_SHADER_RESOURCE_BINDING* pResBinding,
    IN VIR_FIND_RES_MODE            findResMode,
    INOUT VIR_Uniform**             ppUniformArray
    )
{
    VIR_Uniform*    pRetUniform[2] = { gcvNULL, gcvNULL };
    gctUINT         setNo = pResBinding->set;
    gctUINT         binding = pResBinding->binding;
    gctUINT         arraySize = pResBinding->arraySize;
    gctUINT         i, uniformCount = 0;
    gctBOOL         bGoThroughAllUniforms = gcvFALSE;

    if (pResBinding->type == VSC_SHADER_RESOURCE_TYPE_COMBINED_IMAGE_SAMPLER)
    {
        bGoThroughAllUniforms = gcvTRUE;
    }

    for (i = 0; i < (gctINT) VIR_IdList_Count(&pShader->uniforms); ++i)
    {
        VIR_Id      id  = VIR_IdList_GetId(&pShader->uniforms, i);
        VIR_Symbol  *sym = VIR_Shader_GetSymFromId(pShader, id);
        VIR_Uniform *symUniform = gcvNULL;
        VIR_Type    *symType = VIR_Symbol_GetType(sym);
        gctUINT     thisArraySize;

        if (VIR_Type_GetKind(symType) == VIR_TY_STRUCT)
        {
            continue;
        }

        symUniform = VIR_Symbol_GetUniformPointer(pShader, sym);

        if (symUniform == gcvNULL ||
            VIR_Symbol_GetUniformKind(sym) != uniformKind)
        {
            continue;
        }

        /* Skip unnecessary uniforms. */
        if (findResMode == VIR_FIND_RES_MODE_RES_ONLY)
        {
            if (!isSymUniformWithResLayout(sym))
            {
                continue;
            }
        }
        else if (findResMode == VIR_FIND_RES_MODE_COMPILE_GEN_ONLY)
        {
            if (isSymUniformWithResLayout(sym))
            {
                continue;
            }
        }

        if (VIR_Type_GetKind(symType) == VIR_TY_ARRAY)
        {
            thisArraySize = VIR_Type_GetArrayLength(symType);
        }
        else
        {
            thisArraySize = 1;
        }

        if (VIR_Symbol_GetDescriptorSet(sym) == setNo && VIR_Symbol_GetBinding(sym) == binding)
        {
            if (thisArraySize > arraySize)
            {
                gcmASSERT(gcvFALSE);
            }

            gcmASSERT(uniformCount < 2);
            pRetUniform[uniformCount] = symUniform;
            uniformCount++;

            if (!bGoThroughAllUniforms)
            {
                break;
            }
            if (uniformCount == 2)
            {
                break;
            }
        }
    }

    /*
    ** Make sure that:
    ** the first element is the SAMPLER variable and the second element is the SAMPLED IMAGE variable.
    */
    if (pResBinding->type == VSC_SHADER_RESOURCE_TYPE_COMBINED_IMAGE_SAMPLER)
    {
        if (uniformCount == 2)
        {
            if (VIR_Symbol_isImage(VIR_Shader_GetSymFromId(pShader, VIR_Uniform_GetSymID(pRetUniform[0]))))
            {
                VIR_Uniform* tempUniform = pRetUniform[0];

                pRetUniform[0] = pRetUniform[1];
                pRetUniform[1] = tempUniform;
            }
        }
    }

    if (ppUniformArray)
    {
        ppUniformArray[0] = pRetUniform[0];
        ppUniformArray[1] = pRetUniform[1];
    }

    return uniformCount;
}

VIR_TypeId
VIR_ImageFormat_GetComponentTypeId(
    IN VIR_ImageFormat              imageFormat
    )
{
    VIR_TypeId                      componentTypeId = VIR_TYPE_FLOAT32;

    switch (imageFormat)
    {
    case VIR_IMAGE_FORMAT_NONE:
        break;

    case VIR_IMAGE_FORMAT_RGBA32F:
    case VIR_IMAGE_FORMAT_RG32F:
    case VIR_IMAGE_FORMAT_R32F:
        componentTypeId = VIR_TYPE_FLOAT32;
        break;

    case VIR_IMAGE_FORMAT_RGBA32I:
    case VIR_IMAGE_FORMAT_RG32I:
    case VIR_IMAGE_FORMAT_R32I:
        componentTypeId = VIR_TYPE_INT32;
        break;

    case VIR_IMAGE_FORMAT_RGBA32UI:
    case VIR_IMAGE_FORMAT_RG32UI:
    case VIR_IMAGE_FORMAT_R32UI:
        componentTypeId = VIR_TYPE_UINT32;
        break;

    case VIR_IMAGE_FORMAT_RGBA16F:
    case VIR_IMAGE_FORMAT_RG16F:
    case VIR_IMAGE_FORMAT_R16F:
        componentTypeId = VIR_TYPE_FLOAT16;
        break;

    case VIR_IMAGE_FORMAT_RGBA16I:
    case VIR_IMAGE_FORMAT_RG16I:
    case VIR_IMAGE_FORMAT_R16I:
        componentTypeId = VIR_TYPE_INT16;
        break;

    case VIR_IMAGE_FORMAT_RGBA16UI:
    case VIR_IMAGE_FORMAT_RG16UI:
    case VIR_IMAGE_FORMAT_R16UI:
        componentTypeId = VIR_TYPE_UINT16;
        break;

    case VIR_IMAGE_FORMAT_RGBA16:
    case VIR_IMAGE_FORMAT_RGBA16_SNORM:
    case VIR_IMAGE_FORMAT_RG16:
    case VIR_IMAGE_FORMAT_RG16_SNORM:
    case VIR_IMAGE_FORMAT_R16:
    case VIR_IMAGE_FORMAT_R16_SNORM:
    case VIR_IMAGE_FORMAT_BGRA8_UNORM:
    case VIR_IMAGE_FORMAT_RGBA8:
    case VIR_IMAGE_FORMAT_RGBA8_SNORM:
    case VIR_IMAGE_FORMAT_RG8:
    case VIR_IMAGE_FORMAT_RG8_SNORM:
    case VIR_IMAGE_FORMAT_R8:
    case VIR_IMAGE_FORMAT_R8_SNORM:
        componentTypeId = VIR_TYPE_FLOAT16;
        break;

    case VIR_IMAGE_FORMAT_RGBA8I:
    case VIR_IMAGE_FORMAT_RG8I:
    case VIR_IMAGE_FORMAT_R8I:
        componentTypeId = VIR_TYPE_INT8;
        break;

    case VIR_IMAGE_FORMAT_RGBA8UI:
    case VIR_IMAGE_FORMAT_RG8UI:
    case VIR_IMAGE_FORMAT_R8UI:
        componentTypeId = VIR_TYPE_UINT8;
        break;

    case VIR_IMAGE_FORMAT_R5G6B5_UNORM_PACK16:
        componentTypeId = VIR_TYPE_FLOAT32;
        break;

    case VIR_IMAGE_FORMAT_ABGR8_UNORM_PACK32:
        componentTypeId = VIR_TYPE_FLOAT32;
        break;

    case VIR_IMAGE_FORMAT_ABGR8I_PACK32:
        componentTypeId = VIR_TYPE_INT8;
        break;

    case VIR_IMAGE_FORMAT_ABGR8UI_PACK32:
        componentTypeId = VIR_TYPE_UINT8;
        break;

    case VIR_IMAGE_FORMAT_A2R10G10B10_UNORM_PACK32:
        componentTypeId = VIR_TYPE_FLOAT32;
        break;

    case VIR_IMAGE_FORMAT_A2B10G10R10_UNORM_PACK32:
        componentTypeId = VIR_TYPE_FLOAT32;
        break;

    case VIR_IMAGE_FORMAT_A2B10G10R10UI_PACK32:
        componentTypeId = VIR_TYPE_UINT32;
        break;

    default:
        gcmASSERT(gcvFALSE);
        break;
    }

    return componentTypeId;
}

/* Intrinsic instruction related. */
VIR_IntrinsicsKind
VIR_Intrinsic_GetFinalIntrinsicKind(
    IN VIR_Instruction*             pIntrinsicInst
    )
{
    VIR_IntrinsicsKind              intrinsicKind = VIR_Operand_GetIntrinsicKind(VIR_Inst_GetSource(pIntrinsicInst, 0));
    VIR_ParmPassing*                pParmOpnd = VIR_Operand_GetParameters(VIR_Inst_GetSource(pIntrinsicInst, 1));
    VIR_TypeId                      opndTypeId = VIR_Operand_GetTypeId(pParmOpnd->args[0]);
    VIR_TypeId                      symTypeId = VIR_Type_GetBaseTypeId(VIR_Symbol_GetType(VIR_Operand_GetSymbol(pParmOpnd->args[0])));

    /*
    ** The source of image_fetch can be a image or a sampler:
    ** 1) If it is a image, we convert it to a image_load.
    ** 2) If it is a sampler, we convert it to a image_fetch_for_sampler.
    */
    if (VIR_Intrinsics_isImageFetch(intrinsicKind))
    {
        if (VIR_TypeId_isSampler(opndTypeId) || VIR_TypeId_isSampler(symTypeId))
        {
            intrinsicKind = VIR_IK_image_fetch_for_sampler;
        }
        else
        {
            intrinsicKind = VIR_IK_image_load;
        }
    }
    /* if the source of image_query_size is samplerMS */
    else if (VIR_Intrinsics_isImageQuerySize(intrinsicKind) && VIR_TypeId_isSamplerMS(opndTypeId))
    {
        intrinsicKind = VIR_IK_image_query_size_for_sampler;
    }

    return intrinsicKind;
}

