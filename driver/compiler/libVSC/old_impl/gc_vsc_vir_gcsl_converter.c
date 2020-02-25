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
#if gcdENABLE_3D

/* Zone used for header/footer. */
#define _GC_OBJ_ZONE    gcdZONE_COMPILER

#define SHADER_LASTINST(CONV) ((CONV)->Shader->lastInstruction)
#define TEMP_REGISTER_COUNT (1)
#define SAVE_TEMP_REGISTER 0
typedef struct _CONVERTER
{
    VIR_Shader           *VirShader;
    gcSHADER              Shader;
    VSC_HASH_TABLE       *InstTable;
    VSC_HASH_TABLE       *FuncTable;
    VSC_HASH_TABLE       *UniformTable;
#if SAVE_TEMP_REGISTER
    gctUINT32             TmpRegs[TEMP_REGISTER_COUNT];
#endif
    VSC_PRIMARY_MEM_POOL  MemPool;
}
Converter;

typedef struct _conv2VirsVirBuiltinMap
{
    gctSTRING  builtinName;
    VIR_StorageClass storageClass[gcSHADER_KIND_COUNT];
}
conv2VirsVirBuiltinMap;

extern conv2VirsVirBuiltinMap _virBuiltinMap[11];

#define _gcdBuiltinMapCount (sizeof(_virBuiltinMap)/sizeof(conv2VirsVirBuiltinMap))

#define _gcmGetBuiltinNameString(BuiltinKind) \
   (((gctINT)(BuiltinKind) < 0 &&  (-1 * (gctINT)(BuiltinKind)) < _gcdBuiltinMapCount) \
       ? _virBuiltinMap[-1 * (gctINT)(BuiltinKind)].builtinName \
       : gcvNULL)

#define _gcmGetVirBuiltinStorageClass(ShaderType, BuiltinKind) \
   (((gctINT)(BuiltinKind) < 0 &&  (-1 * (gctINT)(BuiltinKind)) < (gctINT)_gcdBuiltinMapCount) \
       ? _virBuiltinMap[-1 * (gctINT)(BuiltinKind)].storageClass[(ShaderType)] \
       : VIR_STORAGE_UNKNOWN)

static gcSL_SWIZZLE
_GetRegisterSwizzle(
    IN Converter        *Converter,
    IN VIR_Operand      *Opnd,
    IN VIR_Instruction  *VirInst
    );

VIR_Enable
VIR_Inst_GetRelEnable(
    IN Converter       *Converter,
    IN VIR_Instruction *Inst,
    IN VIR_Operand     *Opnd
    );

static void
_InitialConverter(
    IN OUT Converter *Converter
    )
{
    vscPMP_Intialize(&Converter->MemPool, gcvNULL, 1024, sizeof(void *), gcvTRUE/*pooling*/);

    Converter->InstTable = vscHTBL_Create(&Converter->MemPool.mmWrapper,
        vscHFUNC_Default, vscHKCMP_Default, 256);
    Converter->FuncTable = vscHTBL_Create(&Converter->MemPool.mmWrapper,
        vscHFUNC_Default, vscHKCMP_Default, 32);
    Converter->UniformTable = vscHTBL_Create(&Converter->MemPool.mmWrapper,
        vscHFUNC_Default, vscHKCMP_Default, 32);
#if SAVE_TEMP_REGISTER
    {
        gctSIZE_T i = 0;
        for(i = 0; i < TEMP_REGISTER_COUNT; ++i)
        {
            Converter->TmpRegs[i] = 0xffff;
        }
    }
#endif
}

static void
_FinalizeConverter(
    IN OUT Converter *Converter
    )
{
    vscPMP_Finalize(&Converter->MemPool);
}

static gctSIZE_T
_FindValue(
    IN OUT VSC_HASH_TABLE  *Table,
    IN void                *Key
    )
{
    gcmASSERT(Table != gcvNULL);

    return (gctSIZE_T)vscHTBL_DirectGet(Table, Key);
}

static void
_AddValue(
    IN OUT VSC_HASH_TABLE *Table,
    IN void          *Key,
    IN gctSIZE_T      Value
    )
{
    gcmASSERT(Table != gcvNULL);

    vscHTBL_DirectSet(Table, Key, (void *)Value);
}

static gctBOOL
_ExpandSelectNeedMove(
    IN Converter        *Converter,
    IN VIR_Instruction  *VirInst
    )
{
    return gcvFALSE;
}

static gctINT32
_CalculateInstCount(
    IN Converter    *Converter,
    IN VIR_Instruction *VirInst
    )
{
    VIR_OpCode       opcode    = VIR_OP_NOP;
    gcmASSERT(VirInst != gcvNULL);

    opcode = VIR_Inst_GetOpcode(VirInst);
    switch(opcode)
    {
    case VIR_OP_IMG_LOAD:   case VIR_OP_IMG_STORE:
    case VIR_OP_VX_IMG_LOAD: case VIR_OP_VX_IMG_STORE:
    case VIR_OP_IMG_LOAD_3D: case VIR_OP_IMG_STORE_3D:
    case VIR_OP_VX_IMG_LOAD_3D: case VIR_OP_VX_IMG_STORE_3D:
    case VIR_OP_IMG_READ:
    case VIR_OP_VX_IMG_READ:
    case VIR_OP_IMG_READ_3D:
    case VIR_OP_VX_IMG_READ_3D:
    case VIR_OP_IMG_WRITE:
    case VIR_OP_VX_IMG_WRITE:
    case VIR_OP_IMG_WRITE_3D:
    case VIR_OP_VX_IMG_WRITE_3D:
    case VIR_OP_IMG_ADDR:
    case VIR_OP_CLAMP0MAX:
    case VIR_OP_CLAMPCOORD:
    case VIR_OP_NOP:        case VIR_OP_MOV:
    case VIR_OP_COMPARE:        case VIR_OP_SAT:
    case VIR_OP_ABS:
    case VIR_OP_FLOOR:      case VIR_OP_CEIL:
    case VIR_OP_POW:        case VIR_OP_LOG2:
    case VIR_OP_EXP2:        case VIR_OP_SIGN:
    case VIR_OP_FRAC:       case VIR_OP_RCP:
    case VIR_OP_RSQ:        case VIR_OP_SQRT:
    case VIR_OP_SIN:        case VIR_OP_COS:
    case VIR_OP_TAN:        case VIR_OP_ACOS:
    case VIR_OP_ASIN:       case VIR_OP_ATAN:
    case VIR_OP_SINPI:      case VIR_OP_COSPI:
    case VIR_OP_TANPI:      case VIR_OP_ADD:
    case VIR_OP_SUB:        case VIR_OP_MUL:
    case VIR_OP_ADDSAT:     case VIR_OP_MADSAT:
    case VIR_OP_SUBSAT:     case VIR_OP_MULSAT:
    case VIR_OP_DIV:        case VIR_OP_MOD:
    case VIR_OP_MAX:        case VIR_OP_MIN:
    case VIR_OP_ADDLO:      case VIR_OP_MULLO:
    case VIR_OP_MULHI:      case VIR_OP_DP2:
    case VIR_OP_DP3:        case VIR_OP_DP4:
    case VIR_OP_NORM_MUL:   case VIR_OP_NORM_DP2:
    case VIR_OP_NORM_DP3:   case VIR_OP_NORM_DP4:
    case VIR_OP_NORM:       case VIR_OP_CROSS:
    case VIR_OP_STEP:       case VIR_OP_DSX:
    case VIR_OP_DSY:        case VIR_OP_FWIDTH:
    case VIR_OP_AND_BITWISE:case VIR_OP_OR_BITWISE:
    case VIR_OP_NOT_BITWISE:case VIR_OP_LSHIFT:
    case VIR_OP_RSHIFT:     case VIR_OP_ROTATE:
    case VIR_OP_LOAD:       case VIR_OP_LOAD_L:
    case VIR_OP_KILL:       case VIR_OP_XOR_BITWISE:
    case VIR_OP_BARRIER:
    case VIR_OP_MEM_BARRIER:
    case VIR_OP_BITSEL:     case VIR_OP_LEADZERO:
    case VIR_OP_GETEXP:     case VIR_OP_GETMANT:
    case VIR_OP_JMP:        case VIR_OP_CALL:
    case VIR_OP_RET:        case VIR_OP_CONVERT:
    case VIR_OP_JMPC:
    case VIR_OP_JMP_ANY:
    case VIR_OP_NEG:
    case VIR_OP_MOVA:       case VIR_OP_PRE_DIV:
    case VIR_OP_PRE_LOG2:   case VIR_OP_SET:
    case VIR_OP_POPCOUNT:
    case VIR_OP_BITFIND_MSB:
    case VIR_OP_BITFIND_LSB:
    case VIR_OP_BITREV:
    case VIR_OP_UCARRY:
    case VIR_OP_BITRANGE:
    case VIR_OP_BITRANGE1:
    case VIR_OP_GET_SAMPLER_IDX:
    case VIR_OP_GET_SAMPLER_LMM:
    case VIR_OP_GET_SAMPLER_LBS:
    case VIR_OP_F2I:
    case VIR_OP_I2F:
    case VIR_OP_CMP:
        return 1;
    case VIR_OP_BITEXTRACT:
    case VIR_OP_BITINSERT1:
    case VIR_OP_BITINSERT2:
    case VIR_OP_TEXLD_U_PLAIN:
    case VIR_OP_TEXLD_U_LOD:
        return 2;
    case VIR_OP_ATOMADD:    case VIR_OP_ATOMSUB:
    case VIR_OP_ATOMXCHG:   case VIR_OP_ATOMMIN:
    case VIR_OP_ATOMMAX:    case VIR_OP_ATOMOR:
    case VIR_OP_ATOMAND:    case VIR_OP_ATOMXOR:
    case VIR_OP_ATOMCMPXCHG:
    case VIR_OP_ATOMCMPXCHG_L:
        return 2;

    case VIR_OP_BITINSERT:
        return 2;

    case VIR_OP_STORE:
    case VIR_OP_STORE_L:
        {
            VIR_OperandInfo src1Info;
            VIR_Operand_GetOperandInfo(VirInst, VIR_Inst_GetSource(VirInst, 1), &src1Info);

            if(src1Info.isImmVal && src1Info.u1.immValue.iValue == 0)
            {
                return 1;
            }
            else
            {
                return 2;
            }
        }

    case VIR_OP_TEXLD:
    case VIR_OP_TEXLD_U:
    case VIR_OP_TEXLDPCF:
    case VIR_OP_TEXLDPCFPROJ:
    case VIR_OP_TEXLDPROJ:
        {
            gctSIZE_T                  i            = 0;
            gctSIZE_T                  count        = 0;
            VIR_Operand *texldOperand = gcvNULL;

            gcmASSERT(VIR_Inst_GetSrcNum(VirInst) == 4 &&
                (VIR_Operand_GetOpKind(VIR_Inst_GetSource(VirInst, 2)) == VIR_OPND_TEXLDPARM ||
                VIR_Operand_GetOpKind(VIR_Inst_GetSource(VirInst, 2)) == VIR_OPND_UNDEF) &&
                VIR_Inst_GetDest(VirInst) != gcvNULL);

            texldOperand = ((VIR_Operand *)VIR_Inst_GetSource(VirInst, 2));

            if (VIR_Operand_GetOpKind(VIR_Inst_GetSource(VirInst, 2)) == VIR_OPND_TEXLDPARM)
            {
                for(i = 0; i < VIR_TEXLDMODIFIER_COUNT; ++i)
                {
                    if(VIR_Operand_GetTexldModifier(texldOperand, i) != gcvNULL)
                    {
                        ++count;
                        break;
                    }
                }
            }

            return count + 1;
        }
    case VIR_OP_SELECT:
        return 3;
    case VIR_OP_CSELECT:
        if(_ExpandSelectNeedMove(Converter, VirInst))
        {
            return 3;
        }
        else
        {
            return 2;
        }
    case VIR_OP_MIX:
        return 3;
    case VIR_OP_CMOV:
        return 3;
    case VIR_OP_MAD:
    case VIR_OP_SAD:
        return 2;
    case VIR_OP_STARR:
        {
            VIR_OperandInfo src0Info;
            VIR_Operand_GetOperandInfo(VirInst, VIR_Inst_GetSource(VirInst, 0), &src0Info);

            if(VIR_OpndInfo_Is_Virtual_Reg(&src0Info) && !src0Info.isInput)
            {
                return 1;
            }
            /* Add a mov instruction. */
            else
            {
                return 2;
            }
        }
    case VIR_OP_LDARR:
        {
            VIR_OperandInfo src1Info;
            VIR_Operand_GetOperandInfo(VirInst, VIR_Inst_GetSource(VirInst, 1), &src1Info);

            if(VIR_OpndInfo_Is_Virtual_Reg(&src1Info) && !src1Info.isInput)
            {
                return 1;
            }
            /* Add a mov instruction. */
            else
            {
                return 2;
            }
        }
    case VIR_OP_LABEL:
        return 0;
    case VIR_OP_BITCAST:    case VIR_OP_SETP:
    case VIR_OP_SWIZZLE:
    case VIR_OP_PACK:       case VIR_OP_SHUFFLE2:
    case VIR_OP_MAD_CC:     case VIR_OP_MADC:
    case VIR_OP_IJMP:
    case VIR_OP_SETMANT:    case VIR_OP_SETEXP:
    case VIR_OP_ROUNDAWAY:  case VIR_OP_ROUNDEVEN:
    case VIR_OP_NEXTAFTER:  case VIR_OP_NAN:
    case VIR_OP_PREFETCH:   case VIR_OP_RGB2YUV:
    case VIR_OP_CVTA:       case VIR_OP_QAS:
    case VIR_OP_ALLOCA:     case VIR_OP_SUBC:
    case VIR_OP_SUB_CC:     case VIR_OP_ADD_CC:
    case VIR_OP_ADDC:       case VIR_OP_FENCE:
    case VIR_OP_SURRED:     case VIR_OP_SURLD:
    case VIR_OP_GETLOD:     case VIR_OP_LDA:
    case VIR_OP_SURQUERY:   case VIR_OP_SURSTORE:
    case VIR_OP_TEXQUERY:   case VIR_OP_ILOAD:
    case VIR_OP_ISTORE:     case VIR_OP_SMOOTH:
    case VIR_OP_ICALL:
    case VIR_OP_LITP:
    case VIR_OP_LENGTH:     case VIR_OP_DST:
    case VIR_OP_COPYSIGN:   case VIR_OP_BYTEREV:
    case VIR_OP_STORE_ATTR:
    case VIR_OP_LOAD_ATTR:
    case VIR_OP_LOAD_ATTR_O:
    case VIR_OP_SELECT_MAP:
    case VIR_OP_EMIT0:
    case VIR_OP_RESTART0:
    case VIR_OP_EMIT_STREAM:
    case VIR_OP_RESTART_STREAM:

    case VIR_OP_PARM:       case VIR_OP_UNREACHABLE:
    case VIR_OP_INTRINSIC:  case VIR_OP_THREADEXIT:
    case VIR_OP_GETPC:      case VIR_OP_BLOCK:
    case VIR_OP_ENTRY:      case VIR_OP_COMMENT:
    case VIR_OP_PRAGMA:     case VIR_OP_SHUFFLE:
    case VIR_OP_BREAK:
    case VIR_OP_ASSERT:     case VIR_OP_TRAP:
    case VIR_OP_EXIT:       case VIR_OP_PHI:
    case VIR_OP_COMBINE_TEX_SAMPL:
    default:
        gcmASSERT(0);
        break;
    }

    return 1;
}

static gceINPUT_OUTPUT
_ConvVirParamQualifier2ParamQualifier(
    VIR_StorageClass Qualifier
    )
{
    switch(Qualifier)
    {
    case VIR_STORAGE_INPARM:
        return gcvFUNCTION_INPUT;
    case VIR_STORAGE_OUTPARM:
        return gcvFUNCTION_OUTPUT;
    case VIR_STORAGE_INOUTPARM:
        return gcvFUNCTION_INOUT;
    default:
        gcmASSERT(0);
    }
    return gcvFUNCTION_INPUT;
}

static gcSL_FORMAT
_ConvBuiltinVirType2Format(
    IN VIR_Type *Type
    )
{
    VIR_PrimitiveTypeId baseType = VIR_Type_GetBaseTypeId(Type);

    switch(VIR_GetTypeTypeKind(baseType))
    {
    case VIR_TY_MATRIX:
    case VIR_TY_VECTOR:
    case VIR_TY_SCALAR:
    case VIR_TY_VOID:
    case VIR_TY_INVALID:
        switch(VIR_GetTypeComponentType(baseType))
        {
        case VIR_TYPE_FLOAT32:
            return gcSL_FLOAT;
        case VIR_TYPE_INT32:
            return gcSL_INTEGER;
        case VIR_TYPE_BOOLEAN:
            return gcSL_BOOLEAN;
        case VIR_TYPE_UINT32:
            return gcSL_UINT32;
        case VIR_TYPE_INT8:
            return gcSL_INT8;
        case VIR_TYPE_UINT8:
            return gcSL_UINT8;
        case VIR_TYPE_INT16:
            return gcSL_INT16;
        case VIR_TYPE_UINT16:
            return gcSL_UINT16;
        case VIR_TYPE_INT64:
            return gcSL_INT64;
        case VIR_TYPE_UINT64:
            return gcSL_UINT64;
        case VIR_TYPE_SNORM8:
            return gcSL_SNORM8;
        case VIR_TYPE_UNORM8:
            return gcSL_UNORM8;
        case VIR_TYPE_SNORM16:
            return gcSL_SNORM16;
        case VIR_TYPE_UNORM16:
            return gcSL_UNORM16;
        case VIR_TYPE_FLOAT16:
            return gcSL_FLOAT16;
        case VIR_TYPE_FLOAT64:
            return gcSL_FLOAT64;
        case VIR_TYPE_UNKNOWN:
            return gcSL_INVALID;
        default:
            gcmASSERT(0);
        }
        break;
    case VIR_TY_SAMPLER:
        switch(baseType)
        {
        case VIR_TYPE_VIV_GENERIC_GL_SAMPLER:
        case VIR_TYPE_SAMPLER_1D:
        case VIR_TYPE_SAMPLER_2D:
        case VIR_TYPE_SAMPLER_3D:
        case VIR_TYPE_SAMPLER_CUBIC:
        case VIR_TYPE_SAMPLER_CUBE_ARRAY:
        case VIR_TYPE_SAMPLER:
        case VIR_TYPE_SAMPLER_EXTERNAL_OES:
        case VIR_TYPE_SAMPLER_2D_SHADOW:
        case VIR_TYPE_SAMPLER_CUBE_SHADOW:
        case VIR_TYPE_SAMPLER_CUBE_ARRAY_SHADOW:
        case VIR_TYPE_SAMPLER_1D_ARRAY:
        case VIR_TYPE_SAMPLER_1D_ARRAY_SHADOW:
        case VIR_TYPE_SAMPLER_2D_ARRAY:
        case VIR_TYPE_SAMPLER_2D_ARRAY_SHADOW:
        case VIR_TYPE_SAMPLER_2D_MS:
        case VIR_TYPE_SAMPLER_2D_MS_ARRAY:
        case VIR_TYPE_SAMPLER_2D_RECT:
        case VIR_TYPE_SAMPLER_2D_RECT_SHADOW:
        case VIR_TYPE_SAMPLER_1D_SHADOW:
            return gcSL_FLOAT;

        case VIR_TYPE_ISAMPLER_2D:
        case VIR_TYPE_ISAMPLER_3D:
        case VIR_TYPE_ISAMPLER_CUBIC:
        case VIR_TYPE_ISAMPLER_CUBE_ARRAY:
        case VIR_TYPE_ISAMPLER_2D_ARRAY:
        case VIR_TYPE_ISAMPLER_2D_MS:
        case VIR_TYPE_ISAMPLER_2D_MS_ARRAY:
        case VIR_TYPE_ISAMPLER_2D_RECT:
        case VIR_TYPE_ISAMPLER_1D_ARRAY:
        case VIR_TYPE_ISAMPLER_1D:
            return gcSL_INTEGER;

        case VIR_TYPE_USAMPLER_2D:
        case VIR_TYPE_USAMPLER_3D:
        case VIR_TYPE_USAMPLER_CUBIC:
        case VIR_TYPE_USAMPLER_CUBE_ARRAY:
        case VIR_TYPE_USAMPLER_2D_ARRAY:
        case VIR_TYPE_USAMPLER_2D_MS:
        case VIR_TYPE_USAMPLER_2D_MS_ARRAY:
        case VIR_TYPE_USAMPLER_2D_RECT:
        case VIR_TYPE_USAMPLER_1D_ARRAY:
        case VIR_TYPE_USAMPLER_1D:
            return gcSL_UINT32;
        default:
            gcmASSERT(0);
        }
        break;

    /* Unsupported OCL */
    case VIR_TY_IMAGE:
    default:
        gcmASSERT(0);
    }

    return gcSL_FLOAT;
}

static gcSL_FORMAT
_ConvVirType2Format(
    IN Converter *Converter,
    IN VIR_Type *Type
    )
{
    if (VIR_Type_isPrimitive(Type))
    {
        return _ConvBuiltinVirType2Format(Type);
    }
    else
    {
        VIR_Type * baseType = gcvNULL;
        switch (VIR_Type_GetKind(Type))
        {
        case VIR_TY_ARRAY:
        case VIR_TY_POINTER:
            baseType = VIR_Shader_GetTypeFromId(Converter->VirShader,
                                                VIR_Type_GetBaseTypeId(Type));
            return _ConvVirType2Format(Converter, baseType);
        default:
        case VIR_TY_STRUCT:
        case VIR_TY_SCALAR:
        case VIR_TY_VECTOR:
        case VIR_TY_MATRIX:
            gcmASSERT(0);
            return gcSL_FLOAT;
        }
    }
}

static gcSHADER_TYPE
_ConvVirType2ShaderType(
    IN Converter *Converter,
    IN VIR_Type *Type
    )
{
    VIR_PrimitiveTypeId baseType = VIR_Type_GetBaseTypeId(Type);
    gcmASSERT(VIR_Type_isPrimitive(Type));

    switch(baseType) {
    case VIR_TYPE_FLOAT32:
        return gcSHADER_FLOAT_X1;
    case VIR_TYPE_FLOAT_X2:
        return gcSHADER_FLOAT_X2;
    case VIR_TYPE_FLOAT_X3:
        return gcSHADER_FLOAT_X3;
    case VIR_TYPE_FLOAT_X4:
        return gcSHADER_FLOAT_X4;
    case VIR_TYPE_FLOAT_2X2:
        return gcSHADER_FLOAT_2X2;
    case VIR_TYPE_FLOAT_3X3:
        return gcSHADER_FLOAT_3X3;
    case VIR_TYPE_FLOAT_4X4:
        return gcSHADER_FLOAT_4X4;
    case VIR_TYPE_FLOAT_2X3:
        return gcSHADER_FLOAT_2X3;
    case VIR_TYPE_FLOAT_2X4:
        return gcSHADER_FLOAT_2X4;
    case VIR_TYPE_FLOAT_3X2:
        return gcSHADER_FLOAT_3X2;
    case VIR_TYPE_FLOAT_3X4:
        return gcSHADER_FLOAT_3X4;
    case VIR_TYPE_FLOAT_4X2:
        return gcSHADER_FLOAT_4X2;
    case VIR_TYPE_FLOAT_4X3:
        return gcSHADER_FLOAT_4X3;
    case VIR_TYPE_BOOLEAN:
        return gcSHADER_BOOLEAN_X1;
    case VIR_TYPE_BOOLEAN_X2:
        return gcSHADER_BOOLEAN_X2;
    case VIR_TYPE_BOOLEAN_X3:
        return gcSHADER_BOOLEAN_X3;
    case VIR_TYPE_BOOLEAN_X4:
        return gcSHADER_BOOLEAN_X4;
    case VIR_TYPE_INT32:
        return gcSHADER_INTEGER_X1;
    case VIR_TYPE_INTEGER_X2:
        return gcSHADER_INTEGER_X2;
    case VIR_TYPE_INTEGER_X3:
        return gcSHADER_INTEGER_X3;
    case VIR_TYPE_INTEGER_X4:
        return gcSHADER_INTEGER_X4;
    case VIR_TYPE_SAMPLER_1D:
        return gcSHADER_SAMPLER_1D;
    case VIR_TYPE_SAMPLER_2D:
        return gcSHADER_SAMPLER_2D;
    case VIR_TYPE_SAMPLER_3D:
        return gcSHADER_SAMPLER_3D;
    case VIR_TYPE_SAMPLER_CUBIC:
        return gcSHADER_SAMPLER_CUBIC;
    case VIR_TYPE_SAMPLER_CUBE_ARRAY:
        return gcSHADER_SAMPLER_CUBEMAP_ARRAY;
    case VIR_TYPE_IMAGE_3D:
        return gcSHADER_IMAGE_3D;
    case VIR_TYPE_SAMPLER_T:
        return gcSHADER_SAMPLER_T;
    case VIR_TYPE_ISAMPLER_2D:
        return gcSHADER_ISAMPLER_2D;
    case VIR_TYPE_ISAMPLER_3D:
        return gcSHADER_ISAMPLER_3D;
    case VIR_TYPE_ISAMPLER_CUBIC:
        return gcSHADER_ISAMPLER_CUBIC;
    case VIR_TYPE_ISAMPLER_CUBE_ARRAY:
        return gcSHADER_ISAMPLER_CUBEMAP_ARRAY;
    case VIR_TYPE_USAMPLER_2D:
        return gcSHADER_USAMPLER_2D;
    case VIR_TYPE_USAMPLER_3D:
        return gcSHADER_USAMPLER_3D;
    case VIR_TYPE_USAMPLER_CUBIC:
        return gcSHADER_USAMPLER_CUBIC;
    case VIR_TYPE_USAMPLER_CUBE_ARRAY:
        return gcSHADER_USAMPLER_CUBEMAP_ARRAY;
    case VIR_TYPE_SAMPLER_EXTERNAL_OES:
        return gcSHADER_SAMPLER_EXTERNAL_OES;
    case VIR_TYPE_UINT32:
        return gcSHADER_UINT_X1;
    case VIR_TYPE_UINT_X2:
        return gcSHADER_UINT_X2;
    case VIR_TYPE_UINT_X3:
        return gcSHADER_UINT_X3;
    case VIR_TYPE_UINT_X4:
        return gcSHADER_UINT_X4;
    case VIR_TYPE_SAMPLER_2D_SHADOW:
        return gcSHADER_SAMPLER_2D_SHADOW;
    case VIR_TYPE_SAMPLER_CUBE_SHADOW:
        return gcSHADER_SAMPLER_CUBE_SHADOW;
    case VIR_TYPE_SAMPLER_CUBE_ARRAY_SHADOW:
        return gcSHADER_SAMPLER_CUBEMAP_ARRAY_SHADOW;
    case VIR_TYPE_SAMPLER_1D_ARRAY:
        return gcSHADER_SAMPLER_1D_ARRAY;
    case VIR_TYPE_SAMPLER_1D_ARRAY_SHADOW:
        return gcSHADER_SAMPLER_1D_ARRAY_SHADOW;
    case VIR_TYPE_SAMPLER_2D_ARRAY:
        return gcSHADER_SAMPLER_2D_ARRAY;
    case VIR_TYPE_ISAMPLER_2D_ARRAY:
        return gcSHADER_ISAMPLER_2D_ARRAY;
    case VIR_TYPE_USAMPLER_2D_ARRAY:
        return gcSHADER_USAMPLER_2D_ARRAY;
    case VIR_TYPE_SAMPLER_2D_ARRAY_SHADOW:
        return gcSHADER_SAMPLER_2D_ARRAY_SHADOW;
    /* OpenGL 4.0 types */

    case VIR_TYPE_SAMPLER_2D_RECT:
        return gcSHADER_SAMPLER_2D_RECT;
    case VIR_TYPE_ISAMPLER_2D_RECT:
        return gcSHADER_ISAMPLER_2D_RECT;
    case VIR_TYPE_USAMPLER_2D_RECT:
        return gcSHADER_USAMPLER_2D_RECT;
    case VIR_TYPE_SAMPLER_2D_RECT_SHADOW:
        return gcSHADER_SAMPLER_2D_RECT_SHADOW;
    case VIR_TYPE_ISAMPLER_1D_ARRAY:
        return gcSHADER_ISAMPLER_1D_ARRAY;
    case VIR_TYPE_USAMPLER_1D_ARRAY:
        return gcSHADER_USAMPLER_1D_ARRAY;
    default:
        break;
    }

    gcmASSERT(0);
    return gcSHADER_FLOAT_X4;
}

static gcSL_SWIZZLE
_ConvVirSwizzle2Swizzle(
    IN VIR_Swizzle Swizzle
    )
{
    return (gcSL_SWIZZLE)Swizzle;
}

static gcSL_ENABLE
_ConvVirEnable2Enable(
    IN VIR_Enable Enable
    )
{
    return (gcSL_ENABLE)Enable;
}

static gcSL_ROUND
_ConvVirRound2Round(
    IN VIR_RoundMode Round
    )
{
    switch(Round)
    {
    case VIR_ROUND_DEFAULT:
        return gcSL_ROUND_DEFAULT;
    case VIR_ROUND_RTE:
        return gcSL_ROUND_RTNE;
    case VIR_ROUND_RTZ:
        return gcSL_ROUND_RTZ;
    case VIR_ROUND_RTP:
        return gcSL_ROUND_RTP;
    case VIR_ROUND_RTN:
        return gcSL_ROUND_RTN;
    default:
        gcmASSERT(0);
    }
    return gcSL_ROUND_DEFAULT;
}

static gcSL_MODIFIER_SAT
_ConvVirSaturation2Saturation(
    IN VIR_Modifier Saturation
    )
{
    switch(Saturation)
    {
    case VIR_MOD_NONE:
        return gcSL_NO_SATURATE;
    case VIR_MOD_SAT_0_TO_1:
        return gcSL_SATURATE;
    case VIR_MOD_SAT_0_TO_INF:
    case VIR_MOD_SAT_NINF_TO_1:
    case VIR_MOD_SAT_TO_MAX_UINT:
    default:
        gcmASSERT(0);
    }
    return gcSL_NO_SATURATE;
}

extern gceSTATUS
_ConvBuiltinNameKindToVirNameId(
    IN gctINT        Kind,
    OUT VIR_NameId * VirNameId
    );

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
static gceSTATUS
_ConvVirNameIdToBuiltinNameKind(
    IN  VIR_NameId      VirNameId,
    OUT gctINT *        Kind
    )
{
    gceBuiltinNameKind kind;

    if (VirNameId > VIR_NAME_BUILTIN_LAST)
    {
        *Kind = gcSL_NONBUILTINGNAME;
        return gcvSTATUS_NOT_FOUND;
    }

    if (VirNameId == VIR_NAME_POSITION)
    {
        kind = gcSL_POSITION;
    } else if (VirNameId == VIR_NAME_POINT_SIZE) {
        kind = gcSL_POINT_SIZE;
    } else if (VirNameId == VIR_NAME_COLOR) {
        kind = gcSL_COLOR;
    } else if (VirNameId == VIR_NAME_FRONT_FACING) {
        kind = gcSL_FRONT_FACING;
    } else if (VirNameId ==  VIR_NAME_POINT_COORD) {
        kind = gcSL_POINT_COORD;
    } else if (VirNameId == VIR_NAME_POSITION_W) {
        kind = gcSL_POSITION_W;
    } else if (VirNameId == VIR_NAME_FOG_COORD) {
        kind = gcSL_FOG_COORD;
    } else if (VirNameId == VIR_NAME_VERTEX_ID ||
               VirNameId == VIR_NAME_VERTEX_INDEX) {
        kind = gcSL_VERTEX_ID;
    } else if (VirNameId == VIR_NAME_INSTANCE_ID) {
        kind = gcSL_INSTANCE_ID;
    } else if (VirNameId == VIR_NAME_DEPTH) {
        kind = gcSL_DEPTH;
    }  else if (VirNameId == VIR_NAME_WORK_GROUP_ID) {
        kind = gcSL_WORK_GROUP_ID;
    }  else if (VirNameId == VIR_NAME_LOCAL_INVOCATION_ID) {
        kind = gcSL_LOCAL_INVOCATION_ID;
    }  else if (VirNameId == VIR_NAME_GLOBAL_INVOCATION_ID) {
        kind = gcSL_GLOBAL_INVOCATION_ID;
    }  else if (VirNameId == VIR_NAME_CLUSTER_ID) {
        kind = gcSL_CLUSTER_ID;
    }  else if (VirNameId == VIR_NAME_CLIP_DISTANCE) {
        kind = gcSL_CLIP_DISTANCE;
    }  else if (VirNameId == VIR_NAME_HELPER_INVOCATION) {
        kind = gcSL_HELPER_INVOCATION;
    } else if (VirNameId == VIR_NAME_FRONT_COLOR) {
        kind = gcSL_FRONT_COLOR;
    } else if (VirNameId == VIR_NAME_BACK_COLOR) {
        kind = gcSL_BACK_COLOR;
    } else if (VirNameId == VIR_NAME_FRONT_SECONDARY_COLOR) {
        kind = gcSL_FRONT_SECONDARY_COLOR;
    } else if (VirNameId == VIR_NAME_BACK_SECONDARY_COLOR) {
        kind = gcSL_BACK_SECONDARY_COLOR;
    } else if (VirNameId == VIR_NAME_TEX_COORD) {
        kind = gcSL_TEX_COORD;
    } else if (VirNameId == VIR_NAME_SUBSAMPLE_DEPTH) {
        kind = gcSL_SUBSAMPLE_DEPTH;
    } else if (VirNameId == VIR_NAME_PERVERTEX) {
        kind = gcSL_PERVERTEX;
    } else if (VirNameId == VIR_NAME_IN) {
        kind = gcSL_IN;
    } else if (VirNameId == VIR_NAME_OUT) {
        kind = gcSL_OUT;
    } else if (VirNameId == VIR_NAME_INVOCATION_ID) {
        kind = gcSL_INVOCATION_ID;
    } else if (VirNameId == VIR_NAME_PATCH_VERTICES_IN) {
        kind = gcSL_PATCH_VERTICES_IN;
    } else if (VirNameId == VIR_NAME_PRIMITIVE_ID) {
        kind = gcSL_PRIMITIVE_ID;
    } else if (VirNameId == VIR_NAME_TESS_LEVEL_OUTER) {
        kind = gcSL_TESS_LEVEL_OUTER;
    } else if (VirNameId == VIR_NAME_TESS_LEVEL_INNER) {
        kind = gcSL_TESS_LEVEL_INNER;
    } else if (VirNameId == VIR_NAME_LAYER) {
        kind = gcSL_LAYER;
    } else if (VirNameId == VIR_NAME_PRIMITIVE_ID_IN) {
        kind = gcSL_PRIMITIVE_ID_IN;
    } else if (VirNameId == VIR_NAME_TESS_COORD) {
        kind = gcSL_TESS_COORD;
    } else if (VirNameId == VIR_NAME_SAMPLE_ID) {
        kind = gcSL_SAMPLE_ID;
    } else if (VirNameId == VIR_NAME_SAMPLE_POSITION) {
        kind = gcSL_SAMPLE_POSITION;
    } else if (VirNameId == VIR_NAME_SAMPLE_MASK_IN) {
        kind = gcSL_SAMPLE_MASK_IN;
    } else if (VirNameId == VIR_NAME_SAMPLE_MASK) {
        kind = gcSL_SAMPLE_MASK;
    } else if (VirNameId == VIR_NAME_IN_POSITION) {
        kind = gcSL_IN_POSITION;
    } else if (VirNameId == VIR_NAME_IN_POINT_SIZE) {
        kind = gcSL_IN_POINT_SIZE;
    } else if (VirNameId == VIR_NAME_BOUNDING_BOX) {
        kind = gcSL_BOUNDING_BOX;
    } else if (VirNameId == VIR_NAME_LAST_FRAG_DATA) {
        kind = gcSL_LAST_FRAG_DATA;
    } else {
        *Kind = gcSL_NONBUILTINGNAME;
        return gcvSTATUS_NOT_FOUND;
    }

    *Kind = kind;

    return gcvSTATUS_OK;
}

static gcATTRIBUTE
_FindAttributeFromVirSym(
    IN VIR_Shader * VirShader,
    IN gcSHADER     Shader,
    IN VIR_Symbol *Symbol
    )
{
    gcATTRIBUTE attribute = gcvNULL;
    if (VIR_Symbol_isAttribute(Symbol))
    {
        gctUINT i;
        gctSTRING name = VIR_Shader_GetSymNameString(VirShader, Symbol);
        gctINT kind = 0;
        gceSTATUS status =_ConvVirNameIdToBuiltinNameKind(VIR_Symbol_GetName(Symbol), &kind);
        /* search the same name attribute */
        for (i = 0; i < Shader->attributeCount; i++)
        {
            attribute = Shader->attributes[i];
            if (attribute == gcvNULL)
            {
                continue;
            }

            if (status == gcvSTATUS_OK)
            {
                /* the attribute has builtin name */
                gcmASSERT(kind < gcSL_NONBUILTINGNAME);

                if (attribute->nameLength == kind)
                    return attribute;
            }
            else
            {
                gcmASSERT(kind == gcSL_NONBUILTINGNAME);

                /* Skip builtin ones as they have no real name string */
                if (attribute->nameLength < gcSL_NONBUILTINGNAME)
                {
                    continue;
                }

                /* compare name */
                if (gcmIS_SUCCESS(gcoOS_StrCmp(attribute->name, name)))
                {
                    /* found it */
                    return attribute;
                }
            }
        }
    }

    return attribute;
}

static gcSL_TYPE
_ConvVirSymbol2Type(
    IN VIR_Symbol *Symbol
    )
{
    switch(VIR_Symbol_GetKind(Symbol))
    {
    case VIR_SYM_UNIFORM:
    case VIR_SYM_SAMPLER:
    case VIR_SYM_SAMPLER_T:
    case VIR_SYM_IMAGE:
    case VIR_SYM_IMAGE_T:
        return gcSL_UNIFORM;
    case VIR_SYM_CONST:
        return gcSL_CONSTANT;
    case VIR_SYM_VIRREG:
        return gcSL_TEMP;
    case VIR_SYM_TEXTURE:
    case VIR_SYM_VARIABLE:
        switch(VIR_Symbol_GetStorageClass(Symbol))
        {
        case VIR_STORAGE_INPUT:
            return gcSL_ATTRIBUTE;
        case VIR_STORAGE_OUTPUT:
            return gcSL_OUTPUT;
        default:
            break;
        }
        return gcSL_TEMP;
    default:
    case VIR_SYM_UBO:
    case VIR_SYM_LABEL:
    case VIR_SYM_TYPE:
    case VIR_SYM_FIELD:
        gcmASSERT(0);
        break;
    }
    return gcSL_NONE;
}

static gcSL_CONDITION
_ConvVirCondition2Condition(
    IN VIR_ConditionOp Condition
    )
{
    switch(Condition)
    {
    case VIR_COP_ALWAYS:
        return gcSL_ALWAYS;
    case VIR_COP_NOT_EQUAL:
        return gcSL_NOT_EQUAL;
    case VIR_COP_LESS:
        return gcSL_LESS;
    case VIR_COP_LESS_OR_EQUAL:
        return gcSL_LESS_OR_EQUAL;
    case VIR_COP_EQUAL:
        return gcSL_EQUAL;
    case VIR_COP_GREATER:
        return gcSL_GREATER;
    case VIR_COP_GREATER_OR_EQUAL:
        return gcSL_GREATER_OR_EQUAL;
    case VIR_COP_AND:
        return gcSL_AND;
    case VIR_COP_OR:
        return gcSL_OR;
    case VIR_COP_XOR:
        return gcSL_XOR;
    case VIR_COP_NOT_ZERO:
        return gcSL_NOT_ZERO;
    case VIR_COP_NOT:
        return gcSL_ZERO;
    case VIR_COP_GREATER_OR_EQUAL_ZERO:
        return gcSL_GREATER_OR_EQUAL_ZERO;
    case VIR_COP_GREATER_ZERO:
        return gcSL_GREATER_ZERO;
    case VIR_COP_LESS_OREQUAL_ZERO:
        return gcSL_LESS_OREQUAL_ZERO;
    case VIR_COP_LESS_ZERO:
        return gcSL_LESS_ZERO;
    case VIR_COP_ALLMSB:
        return gcSL_ALLMSB;
    case VIR_COP_ANYMSB:
        return gcSL_ANYMSB;
    case VIR_COP_SELMSB:
        return gcSL_SELMSB;
    default:
        break;
    }
    gcmASSERT(0);
    return gcSL_ALWAYS;
}

static gcSL_INDEXED
_ConvVirOpndSwizzle2Indexd(
    IN VIR_Operand *Operand
    )
{
    gcmASSERT(Operand != gcvNULL &&
        !VIR_Operand_isLvalue(Operand));

    switch(VIR_Operand_GetSwizzle(Operand))
    {
    case VIR_SWIZZLE_X:
     /*  VIR_SWIZZLE_XXXX: */
        return gcSL_INDEXED_X;
    case VIR_SWIZZLE_YYYY:
        return gcSL_INDEXED_Y;
    case VIR_SWIZZLE_ZZZZ:
        return gcSL_INDEXED_Z;
    case VIR_SWIZZLE_WWWW:
        return gcSL_INDEXED_W;
    default:
        gcmASSERT(0);
        break;
    }
    gcmASSERT(0);
    return gcSL_NOT_INDEXED;
}

static gcSL_INDEXED
_ConvVirIndexed2Indexed(
    IN VIR_Indexed Indexed
    )
{
    switch(Indexed)
    {
    case VIR_INDEXED_NONE:
        return gcSL_NOT_INDEXED;
    case VIR_INDEXED_X:
        return gcSL_INDEXED_X;
    case VIR_INDEXED_Y:
        return gcSL_INDEXED_Y;
    case VIR_INDEXED_Z:
        return gcSL_INDEXED_Z;
    case VIR_INDEXED_W:
        return gcSL_INDEXED_W;
    case VIR_INDEXED_AL:
    case VIR_INDEXED_VERTEX_ID:
    default:
        gcmASSERT(0);
        break;
    }
    gcmASSERT(0);
    return gcSL_NOT_INDEXED;
}



static gcSHADER_PRECISION
_ConvVirOpndPrec2Prec(
    IN VIR_Operand *Opnd
    )
{
    switch(VIR_Operand_GetPrecision(Opnd))
    {
    case VIR_PRECISION_DEFAULT:
        return gcSHADER_PRECISION_DEFAULT;
    case VIR_PRECISION_HIGH:
        return gcSHADER_PRECISION_HIGH;
    case VIR_PRECISION_MEDIUM:
        return gcSHADER_PRECISION_MEDIUM;
    case VIR_PRECISION_LOW:
        return gcSHADER_PRECISION_LOW;
    case VIR_PRECISION_ANY:
        return gcSHADER_PRECISION_ANY;
    default:
        gcmASSERT(0);
        return gcSHADER_PRECISION_MEDIUM;
    }
}

static gctUINT8
_ConvgcSLSwizzle2gcSLEnable(
    IN gcSL_SWIZZLE X,
    IN gcSL_SWIZZLE Y,
    IN gcSL_SWIZZLE Z,
    IN gcSL_SWIZZLE W
    )
{
    static const gctUINT8 _enable[] =
    {
        gcSL_ENABLE_X,
        gcSL_ENABLE_Y,
        gcSL_ENABLE_Z,
        gcSL_ENABLE_W
    };

    /* Return combined enables for each swizzle. */
    return _enable[X] | _enable[Y] | _enable[Z] | _enable[W];
}

static gcSL_SWIZZLE
_GetRegisterSwizzle(
    IN Converter        *Converter,
    IN VIR_Operand      *Opnd,
    IN VIR_Instruction  *VirInst
    )
{
    gcSL_SWIZZLE    swizzle     = VIR_Operand_isLvalue(Opnd) ?
                _ConvVirSwizzle2Swizzle(VIR_Enable_2_Swizzle_WShift(VIR_Operand_GetEnable(Opnd))):
                _ConvVirSwizzle2Swizzle(VIR_Operand_GetSwizzle(Opnd));

    gcSL_SWIZZLE newSwizzle = swizzle;

    if (VIR_Shader_isRegAllocated(Converter->VirShader))
    {
        gctUINT8 shift = (gctUINT8) VIR_Operand_GetHwShift(Opnd);
        VIR_OpCode  opcode = VIR_Inst_GetOpcode(VirInst);

        /* compnent shift */
        newSwizzle  = (((swizzle >> 6) & 0x3) + shift) << 6;
        newSwizzle |= (((swizzle >> 4) & 0x3) + shift) << 4;
        newSwizzle |= (((swizzle >> 2) & 0x3) + shift) << 2;
        newSwizzle |= (((swizzle >> 0) & 0x3) + shift) << 0;

        if (!((opcode == VIR_OP_DP2) ||
              (opcode == VIR_OP_DP3) ||
              (opcode == VIR_OP_DP4) ||
              (opcode == VIR_OP_NORM_DP2) ||
              (opcode == VIR_OP_NORM_DP3) ||
              (opcode == VIR_OP_NORM_DP4) ||
              (opcode == VIR_OP_NORM) ||
              (opcode == VIR_OP_CROSS)))
        {
            gctUINT8 enableShift = 0;

            if (VIR_Inst_GetDest(VirInst))
            {
                enableShift = (gctUINT8) VIR_Operand_GetHwShift(VIR_Inst_GetDest(VirInst));

            }
            while (enableShift-- >0)
            {
                newSwizzle = (newSwizzle << 2 | (newSwizzle & 0x3));
            }
        }
    }

    return newSwizzle;
}

/* this is to get inst enable considering RA shift information */
VIR_Enable
VIR_Inst_GetRelEnable(
    IN Converter       *Converter,
    IN VIR_Instruction *Inst,
    IN VIR_Operand     *Opnd
    )
{
    VIR_Enable  vEnable =  VIR_ENABLE_XYZW;

    if (Opnd)
    {
        if(VIR_Operand_isLvalue(Opnd))
        {
            vEnable = VIR_Operand_GetEnable(Opnd);
            if (VIR_Shader_isRegAllocated(Converter->VirShader))
            {
                VIR_Operand *DestOpnd = Opnd;
                if (DestOpnd &&
                    ((VIR_Operand_GetOpKind(DestOpnd) == VIR_OPND_SYMBOL) ||
                    (VIR_Operand_GetOpKind(DestOpnd) == VIR_OPND_VIRREG)))
                {
                    gcmASSERT(VIR_Operand_isRegAllocated(DestOpnd));
                    vEnable = vEnable << VIR_Operand_GetHwShift(DestOpnd);
                }
            }
        }
        else
        {
            gcSL_SWIZZLE swizzle = _GetRegisterSwizzle(Converter, Opnd, Inst);
            vEnable = _ConvgcSLSwizzle2gcSLEnable((swizzle >> 0) & 0x3,
                (swizzle >> 2) & 0x3,
                (swizzle >> 4) & 0x3,
                (swizzle >> 6) & 0x3);
        }
    }

    return vEnable;
}

static gctUINT
_GetRegisterIndex(
    IN Converter  *Converter,
    IN VIR_Symbol *Sym,
    IN VIR_Operand *Opnd
    )
{
    gctUINT index;
    switch(VIR_Symbol_GetKind(Sym))
    {
    case VIR_SYM_IMAGE:
        return VIR_Symbol_GetImage(Sym)->gcslIndex;
    case VIR_SYM_UNIFORM:
#ifdef ADD_UNIFORM_CONSTANT
        return _FindValue(Converter->UniformTable, VIR_Symbol_GetUniform(Sym));
#else
        return VIR_Symbol_GetUniform(Sym)->gcslIndex;
#endif
    case VIR_SYM_SAMPLER:
        return VIR_Symbol_GetSampler(Sym)->gcslIndex;
    case VIR_SYM_SAMPLER_T:
        return VIR_Symbol_GetSamplerT(Sym)->gcslIndex;
    case VIR_SYM_IMAGE_T:
        return VIR_Symbol_GetImageT(Sym)->gcslIndex;
    case VIR_SYM_TEXTURE:
    case VIR_SYM_VARIABLE:       /* global/local variables, input/output */
    {
        if (VIR_Shader_isRegAllocated(Converter->VirShader))
        {
            if(Opnd != gcvNULL)
            {
                /* need to use hw register infor generated by register alloc */
                gcmASSERT(VIR_Operand_isRegAllocated(Opnd));
                index = VIR_Operand_GetHwRegId(Opnd);
            }
            else
            {
                index = VIR_Symbol_GetHwRegId(Sym);
            }
        }
        else
        {
            index = VIR_Symbol_GetVariableVregIndex(Sym);
            gcSHADER_UpdateTempRegCount(Converter->Shader, index);
        }
        return index;
    }
    case VIR_SYM_VIRREG:
        if (VIR_Shader_isRegAllocated(Converter->VirShader))
        {
            if(Opnd != gcvNULL)
            {
                /* need to use hw register infor generated by register alloc
                vreg must have regAllocated info */
                gcmASSERT(VIR_Operand_isRegAllocated(Opnd));
                index = VIR_Operand_GetHwRegId(Opnd);
            }
            else
            {
                index = VIR_Symbol_GetHwRegId(Sym);
            }
        }
        else
        {
            index = VIR_Symbol_GetVregIndex(Sym);
            gcSHADER_UpdateTempRegCount(Converter->Shader, index);
        }
        return index;
    case VIR_SYM_CONST:
        {
            /* const symbol cannot be register index!
             * if there is any need to have const symbol, handle it at
             * the place it is used.
             */
            gcmASSERT(0);
            break;
        }
    case VIR_SYM_UBO:
        /* NOTE */
        gcmASSERT(0);
        break;
    default:
        gcmASSERT(0);
        break;
    }
    return 0;
}

static gctUINT32
_GetIndexedRegisterIndex(
    IN Converter        *Converter,
    IN VIR_Instruction  *VirInst,
    IN VIR_Operand      *Operand,
    IN gctUINT          relIndexing
    )
{
    gctUINT32       indexRegister;

    if (VIR_Shader_isRegAllocated(Converter->VirShader))
    {
        indexRegister =  relIndexing;
    }
    else
    {
        if (VIR_Operand_GetRelAddrMode(Operand) != VIR_INDEXED_NONE)
        {
            gcmASSERT(!VirInst->_parentUseBB &&
                      VIR_Inst_GetFunction(VirInst) != gcvNULL);

            indexRegister = (gctUINT32)_GetRegisterIndex(Converter,
                VIR_Function_GetSymFromId(VIR_Inst_GetFunction(VirInst), relIndexing), Operand);
        }
        else
        {
            indexRegister = (gctUINT32) relIndexing;
        }

        if (VIR_Operand_GetRelAddrMode(Operand) != VIR_INDEXED_NONE)
        {
            VIR_Symbol  *indexSym = VIR_Shader_GetSymFromId(
                Converter->VirShader,
                relIndexing);
            gcSL_TYPE   srcKind = _ConvVirSymbol2Type(indexSym);
            /* index symbol should not be attribute */
            if (srcKind == gcSL_ATTRIBUTE)
            {
                gcmASSERT(gcvFALSE);
            }
        }
    }

    return indexRegister;
}

static gcSL_OPCODE
_ConvVirOpcode2Opcode(
    IN VIR_OpCode Opcode
    )
{
    switch(Opcode)
    {
    case VIR_OP_IMG_LOAD:       return gcSL_IMAGE_RD;
    case VIR_OP_IMG_STORE:      return gcSL_IMAGE_WR;
    case VIR_OP_IMG_LOAD_3D:    return gcSL_IMAGE_RD_3D;
    case VIR_OP_IMG_STORE_3D:   return gcSL_IMAGE_WR_3D;
    case VIR_OP_IMG_ADDR:       return gcSL_IMAGE_ADDR;
    case VIR_OP_IMG_ADDR_3D:       return gcSL_IMAGE_ADDR_3D;
    case VIR_OP_CLAMP0MAX:      return gcSL_CLAMP0MAX;
    case VIR_OP_CLAMPCOORD:     return gcSL_CLAMPCOORD;
    case VIR_OP_NOP:            return gcSL_NOP;
    case VIR_OP_MOV:            return gcSL_MOV;
    case VIR_OP_SET:         return gcSL_SET;
    case VIR_OP_COMPARE:            return gcSL_CMP;
    case VIR_OP_SAT:            return gcSL_SAT;
    case VIR_OP_ABS:            return gcSL_ABS;
    case VIR_OP_FLOOR:          return gcSL_FLOOR;
    case VIR_OP_CEIL:           return gcSL_CEIL;
    case VIR_OP_POW:            return gcSL_POW;
    case VIR_OP_LOG2:           return gcSL_LOG;
    case VIR_OP_EXP2:           return gcSL_EXP;
    case VIR_OP_SIGN:           return gcSL_SIGN;
    case VIR_OP_FRAC:           return gcSL_FRAC;
    case VIR_OP_RCP:            return gcSL_RCP;
    case VIR_OP_RSQ:            return gcSL_RSQ;
    case VIR_OP_SQRT:           return gcSL_SQRT;
    case VIR_OP_SIN:            return gcSL_SIN;
    case VIR_OP_COS:            return gcSL_COS;
    case VIR_OP_TAN:            return gcSL_TAN;
    case VIR_OP_ACOS:           return gcSL_ACOS;
    case VIR_OP_ASIN:           return gcSL_ASIN;
    case VIR_OP_ATAN:           return gcSL_ATAN;
    case VIR_OP_SINPI:          return gcSL_SINPI;
    case VIR_OP_COSPI:          return gcSL_COSPI;
    case VIR_OP_TANPI:          return gcSL_TANPI;
    case VIR_OP_ADD:            return gcSL_ADD;
    case VIR_OP_SUB:            return gcSL_SUB;
    case VIR_OP_MUL:            return gcSL_MUL;
    case VIR_OP_ADDSAT:         return gcSL_ADDSAT;
    case VIR_OP_SUBSAT:         return gcSL_SUBSAT;
    case VIR_OP_MULSAT:         return gcSL_MULSAT;
    case VIR_OP_MADSAT:         return gcSL_MADSAT;
    case VIR_OP_DIV:            return gcSL_DIV;
    case VIR_OP_MOD:            return gcSL_MOD;
    case VIR_OP_MAX:            return gcSL_MAX;
    case VIR_OP_MIN:            return gcSL_MIN;
    case VIR_OP_ADDLO:          return gcSL_ADDLO;
    case VIR_OP_MULLO:          return gcSL_MULLO;
    case VIR_OP_MULHI:          return gcSL_MULHI;
    case VIR_OP_DP2:            return gcSL_DP2;
    case VIR_OP_DP3:            return gcSL_DP3;
    case VIR_OP_DP4:            return gcSL_DP4;
    case VIR_OP_NORM_MUL:       return gcSL_NORM_MUL;
    case VIR_OP_NORM_DP2:       return gcSL_NORM_DP2;
    case VIR_OP_NORM_DP3:       return gcSL_NORM_DP3;
    case VIR_OP_NORM_DP4:       return gcSL_NORM_DP4;
    case VIR_OP_NORM:           return gcSL_NORM;
    case VIR_OP_CROSS:          return gcSL_CROSS;
    case VIR_OP_STEP:           return gcSL_STEP;
    case VIR_OP_DSX:            return gcSL_DSX;
    case VIR_OP_DSY:            return gcSL_DSY;
    case VIR_OP_FWIDTH:         return gcSL_FWIDTH;
    case VIR_OP_AND_BITWISE:    return gcSL_AND_BITWISE;
    case VIR_OP_OR_BITWISE:     return gcSL_OR_BITWISE;
    case VIR_OP_NOT_BITWISE:    return gcSL_NOT_BITWISE;
    case VIR_OP_LSHIFT:         return gcSL_LSHIFT;
    case VIR_OP_RSHIFT:         return gcSL_RSHIFT;
    case VIR_OP_ROTATE:         return gcSL_ROTATE;
    case VIR_OP_STORE:          return gcSL_STORE1;
    case VIR_OP_STORE_L:        return gcSL_STORE_L;
    case VIR_OP_LOAD:           return gcSL_LOAD;
    case VIR_OP_LOAD_L:         return gcSL_LOAD_L;
    case VIR_OP_KILL:           return gcSL_KILL;
    case VIR_OP_XOR_BITWISE:    return gcSL_XOR_BITWISE;
    case VIR_OP_BARRIER:        return gcSL_BARRIER;
    case VIR_OP_ATOMADD:        return gcSL_ATOMADD;
    case VIR_OP_ATOMSUB:        return gcSL_ATOMSUB;
    case VIR_OP_ATOMXCHG:       return gcSL_ATOMXCHG;
    case VIR_OP_ATOMCMPXCHG:    return gcSL_ATOMCMPXCHG;
    case VIR_OP_ATOMCMPXCHG_L:  return gcSL_ATOMCMPXCHG;
    case VIR_OP_ATOMMIN:        return gcSL_ATOMMIN;
    case VIR_OP_ATOMMAX:        return gcSL_ATOMMAX;
    case VIR_OP_ATOMOR:         return gcSL_ATOMOR;
    case VIR_OP_ATOMAND:        return gcSL_ATOMAND;
    case VIR_OP_ATOMXOR:        return gcSL_ATOMXOR;
    case VIR_OP_BITSEL:         return gcSL_BITSEL;
    case VIR_OP_LEADZERO:       return gcSL_LEADZERO;
    case VIR_OP_GETEXP:         return gcSL_GETEXP;
    case VIR_OP_GETMANT:        return gcSL_GETMANT;
    case VIR_OP_JMP:            return gcSL_JMP;
    case VIR_OP_JMPC:           return gcSL_JMP;
    case VIR_OP_JMP_ANY:        return gcSL_JMP_ANY;
    case VIR_OP_CALL:           return gcSL_CALL;
    case VIR_OP_RET:            return gcSL_RET;
    case VIR_OP_CONVERT:           return gcSL_CONV;
    case VIR_OP_TEXLD:          return gcSL_TEXLD;
    case VIR_OP_TEXLD_U:        return gcSL_TEXLD_U;
    case VIR_OP_TEXLDPCF:       return gcSL_TEXLDPCF;
    case VIR_OP_TEXLDPCFPROJ:   return gcSL_TEXLDPCFPROJ;
    case VIR_OP_TEXLDPROJ:      return gcSL_TEXLDPROJ;
    case VIR_OP_MOVA:           return gcSL_MOVA;
    case VIR_OP_NEG:            return gcSL_SUB;
    case VIR_OP_PRE_DIV:        return gcSL_PRE_DIV;
    case VIR_OP_PRE_LOG2:       return gcSL_PRE_LOG2;
    case VIR_OP_POPCOUNT:       return gcSL_POPCOUNT;
    case VIR_OP_BITFIND_LSB:    return gcSL_FINDLSB;
    case VIR_OP_BITFIND_MSB:    return gcSL_FINDMSB;
    case VIR_OP_BITREV:         return gcSL_BIT_REVERSAL;
    case VIR_OP_BITEXTRACT:     return gcSL_BITEXTRACT;
    case VIR_OP_BITINSERT:      return gcSL_BITINSERT;
    case VIR_OP_BITINSERT1:     return gcSL_BITINSERT;
    case VIR_OP_BITINSERT2:     return gcSL_BITINSERT;
    case VIR_OP_UCARRY:         return gcSL_UCARRY;
    case VIR_OP_TEXLD_U_PLAIN:  return gcSL_TEXU;
    case VIR_OP_TEXLD_U_LOD:    return gcSL_TEXU_LOD;
    case VIR_OP_BITRANGE:       return gcSL_BITRANGE;
    case VIR_OP_BITRANGE1:      return gcSL_BITRANGE1;
    case VIR_OP_F2I:            return gcSL_F2I;
    case VIR_OP_I2F:            return gcSL_I2F;
    case VIR_OP_CMP:            return gcSL_CMP;
    case VIR_OP_MEM_BARRIER:
        {
            /* Currently HW can't support memory barrier. */
            if (1)
            {
                return gcSL_NOP;
            }
            else
            {
                return gcSL_MEM_BARRIER;
            }
        }
    case VIR_OP_CMAD:           return gcSL_CMAD;
    case VIR_OP_CMUL:           return gcSL_CMUL;
    case VIR_OP_CADD:           return gcSL_ADD;
    case VIR_OP_CONJ:           return gcSL_CONJ;
    case VIR_OP_CMADCJ:         return gcSL_CMADCJ;
    case VIR_OP_CMULCJ:         return gcSL_CMULCJ;
    case VIR_OP_CADDCJ:         return gcSL_CADDCJ;
    case VIR_OP_CSUBCJ:         return gcSL_CSUBCJ;

    default:
        gcmASSERT(0);
        return gcSL_NOP;
    }
}

static gceSTATUS
_ConvVirOperand2Target(
    IN OUT Converter        *Converter,
    IN VIR_OpCode           Opcode,
    IN VIR_Operand          *Operand,
    IN VIR_Instruction      *VirInst,
    IN gcSL_CONDITION       Condition,
    IN gctUINT32            SrcLoc
    )
{
    gceSTATUS   status = gcvSTATUS_OK;

    gcmASSERT(Converter->Shader != gcvNULL &&
        Converter->VirShader != gcvNULL &&
        VirInst != gcvNULL);

    if(Operand == gcvNULL)
    {
        gcSHADER_AddOpcode2(
            Converter->Shader,
            _ConvVirOpcode2Opcode(Opcode),
            Condition,
            0,
            gcSL_ENABLE_NONE,
            gcSL_INVALID,
            gcSHADER_PRECISION_DEFAULT,
            SrcLoc);
        return status;
    }
    switch(VIR_Operand_GetOpKind(Operand))
    {
    case VIR_OPND_NONE:
        /* fall through */
    case VIR_OPND_UNUSED:
        /* fall through */
    case VIR_OPND_UNDEF:
        gcSHADER_AddOpcode2(
            Converter->Shader,
            _ConvVirOpcode2Opcode(Opcode),
            Condition,
            0,
            gcSL_ENABLE_NONE,
            gcSL_INVALID,
            gcSHADER_PRECISION_DEFAULT,
            SrcLoc
            );
        return status;

    case VIR_OPND_LABEL:
        {
            VIR_Label    *label = gcvNULL;
            VIR_Type     *type  = gcvNULL;

            gctUINT32     defIndex = 0;

            gcmASSERT(!VirInst->_parentUseBB &&
                      VIR_Inst_GetFunction(VirInst) != gcvNULL &&
                      VIR_Operand_GetLabel(Operand) != gcvNULL);

            label = VIR_Operand_GetLabel(Operand);
            type = VIR_Shader_GetTypeFromId(Converter->VirShader, VIR_Operand_GetTypeId(Operand));
            defIndex = _FindValue(Converter->InstTable, label->defined);

            gcSHADER_AddOpcodeConditionalFormattedEnable(
                Converter->Shader,
                _ConvVirOpcode2Opcode(Opcode),
                Condition,
                _ConvVirType2Format(Converter, type),
                _ConvVirEnable2Enable(VIR_Inst_GetRelEnable(Converter, VirInst, Operand)),
                defIndex,
                SrcLoc);

            break;
        }

    case VIR_OPND_FUNCTION:
        {
            VIR_Function    *func;
            VIR_Type        *type;
            gctUINT32        funcLabel = 0;

            gcmASSERT(!VirInst->_parentUseBB &&
                      VIR_Inst_GetFunction(VirInst) != gcvNULL);

            type = VIR_Shader_GetTypeFromId(Converter->VirShader, VIR_Operand_GetTypeId(Operand));
            if(type == gcvNULL)
            {
                return gcvSTATUS_NOT_FOUND;
            }

            func = VIR_Operand_GetFunction(Operand);
            if(func == gcvNULL)
            {
                return gcvSTATUS_NOT_FOUND;
            }

            funcLabel = _FindValue(Converter->FuncTable, func);

            gcSHADER_AddOpcodeConditionalFormattedEnable(
                Converter->Shader,
                _ConvVirOpcode2Opcode(Opcode),
                Condition,
                _ConvVirType2Format(Converter, type),
                _ConvVirEnable2Enable(VIR_Inst_GetRelEnable(Converter, VirInst, Operand)),
                funcLabel,
                SrcLoc);
            break;
        }

    case VIR_OPND_SYMBOL:
        {
            VIR_Type       *type   = VIR_Shader_GetTypeFromId(Converter->VirShader,
                VIR_Operand_GetTypeId(Operand));

            VIR_Symbol     *sym    = VIR_Operand_GetSymbol(Operand);
            gcSL_OPCODE     opcode = _ConvVirOpcode2Opcode(Opcode);

            gctUINT         index  = _GetRegisterIndex(Converter, sym, Operand);
            VIR_Enable      vEnable = VIR_Inst_GetRelEnable(Converter, VirInst, Operand);
            gcSL_ENABLE     enable =  _ConvVirEnable2Enable(vEnable);

            gcSL_INDEXED    indexed =
                _ConvVirIndexed2Indexed(VIR_Operand_GetRelAddrMode(Operand));

            gctINT          relIndexing = VIR_Operand_GetRelIndexing(Operand);

            gctUINT32       indexRegister =
                _GetIndexedRegisterIndex(Converter, VirInst, Operand, relIndexing);

            gcSL_FORMAT         format      = _ConvVirType2Format(Converter, type);
            gcSHADER_PRECISION  precision   = _ConvVirOpndPrec2Prec(Operand);

            index = gcmSL_INDEX_SET(index, ConstValue, VIR_Operand_GetMatrixConstIndex(Operand));

            gcSHADER_AddOpcodeConditionIndexedWithPrecision(
                Converter->Shader,
                opcode,
                Condition,
                index,
                enable,
                indexed,
                (gctUINT16)indexRegister,
                format,
                precision,
                SrcLoc);

            if (VIR_Operand_isLvalue(Operand))
            {
                gcSHADER_AddRoundingMode(
                    Converter->Shader,
                    _ConvVirRound2Round(VIR_Operand_GetRoundMode(Operand))
                    );

                gcSHADER_AddSaturation(
                    Converter->Shader,
                    _ConvVirSaturation2Saturation(VIR_Operand_GetModifier(Operand))
                    );
            }
            break;
        }
    case VIR_OPND_SAMPLER_INDEXING:
    case VIR_OPND_IMMEDIATE:
    case VIR_OPND_CONST:
    case VIR_OPND_PARAMETERS:
    case VIR_OPND_NAME:
    case VIR_OPND_INTRINSIC:
    case VIR_OPND_FIELD:
    case VIR_OPND_ARRAY:
    default:
        gcmASSERT(0);
        break;
    }
    return status;
}

static gcSL_ENABLE
_ConvVirType2Enable(
    IN VIR_Type         *Type
    )
{
    VIR_PrimitiveTypeId baseType = VIR_Type_GetBaseTypeId(Type);

    switch(VIR_GetTypeComponents(baseType))
    {
    case 0:
        return gcSL_ENABLE_NONE;
    case 1:
        return gcSL_ENABLE_X;
    case 2:
        return gcSL_ENABLE_XY;
    case 3:
        return gcSL_ENABLE_XYZ;
    case 4:
        return gcSL_ENABLE_XYZW;
    default:
        gcmASSERT(0);
    }
    return gcSL_ENABLE_XYZW;
}

static gceSTATUS
_ConvVirOperand2Source(
    IN OUT Converter        *Converter,
    IN VIR_Operand          *Operand,
    IN VIR_Instruction      *VirInst,
    IN gctUINT32            SourceIndex
    )
{
    gceSTATUS   status         = gcvSTATUS_OK;
    gctSOURCE_t *source        = gcvNULL;
    gctUINT32   *sourceIndex   = gcvNULL;
    gcSL_INSTRUCTION inst      = gcvNULL;

    gcmASSERT(Converter->Shader != gcvNULL &&
        Operand != gcvNULL &&
        Converter->VirShader != gcvNULL &&
        VirInst != gcvNULL);

    inst          = &Converter->Shader->code[SHADER_LASTINST(Converter)];
    source        = (SourceIndex == 0) ? &inst->source0 : &inst->source1;
    sourceIndex   = (SourceIndex == 0) ? &inst->source0Index : &inst->source1Index;

    switch(VIR_Operand_GetOpKind(Operand))
    {
    case VIR_OPND_NONE:
        /* fall through */
    case VIR_OPND_UNDEF:
        /* fall through */
    case VIR_OPND_UNUSED:
        return status;
    case VIR_OPND_IMMEDIATE:
        {
            VIR_Type   *type = VIR_Shader_GetTypeFromId(Converter->VirShader,
                VIR_Operand_GetTypeId(Operand));

            gcmASSERT(!VIR_Operand_isLvalue(Operand));
            if(type == gcvNULL)
            {
                return gcvSTATUS_NOT_FOUND;
            }

            gcSHADER_AddSourceConstantFormattedWithPrecision(
                Converter->Shader,
                &VIR_Operand_GetImmediateUint(Operand),
                _ConvVirType2Format(Converter, type),
                gcSHADER_PRECISION_HIGH
                );
            break;
        }
    case VIR_OPND_SAMPLER_INDEXING:
        if(VIR_Operand_GetRelAddrMode(Operand) != VIR_INDEXED_NONE)
        {
            gcSL_INDEXED    indexed     =
                _ConvVirIndexed2Indexed(VIR_Operand_GetRelAddrMode(Operand));

            gctINT          relIndexing = VIR_Operand_GetRelIndexing(Operand);

            gctUINT32       indexRegister =
                _GetIndexedRegisterIndex(Converter, VirInst, Operand, relIndexing);
            gcSL_SWIZZLE    swizzle     = _GetRegisterSwizzle(Converter, Operand, VirInst);
            gcSHADER_AddSourceSamplerIndexed(
                Converter->Shader,
                swizzle,
                indexed,
                (gctUINT16)indexRegister);

            *sourceIndex = gcmSL_INDEX_SET(*sourceIndex, ConstValue,
                VIR_Operand_GetMatrixConstIndex(Operand));
        }
        else
        {
            gctUINT32           indexRegister =
                _GetIndexedRegisterIndex(Converter, VirInst, Operand, VIR_Operand_GetRelIndexing(Operand));
            gcUNIFORM           uniform = gcvNULL;
            gctINT              arrayIndex = 0;

            uniform = gcSHADER_GetUniformBySamplerIndex(Converter->Shader,
                                                        indexRegister,
                                                        &arrayIndex);

            gcmASSERT(uniform);

            gcSHADER_AddSourceUniformIndexedFormattedWithPrecision(
                Converter->Shader,
                uniform,
                gcSL_SWIZZLE_XYZW,
                arrayIndex,
                gcSL_NOT_INDEXED,
                gcSL_NONE_INDEXED,
                0,
                GetUniformFormat(uniform),
                GetUniformPrecision(uniform));
        }
        break;
    case VIR_OPND_SYMBOL:
        {
            VIR_Symbol      *sym        = VIR_Operand_GetSymbol(Operand);

            VIR_Type        *type       =
                VIR_Shader_GetTypeFromId(Converter->VirShader, VIR_Operand_GetTypeId(Operand));

            gcSL_TYPE       srcKind     = _ConvVirSymbol2Type(sym);
            gctUINT         index       = _GetRegisterIndex(Converter, sym, Operand);
            gcSL_SWIZZLE    swizzle     = _GetRegisterSwizzle(Converter, Operand, VirInst);

            gcSL_INDEXED    indexed     =
                _ConvVirIndexed2Indexed(VIR_Operand_GetRelAddrMode(Operand));
            gcSL_INDEXED_LEVEL indexedLevel = (gcSL_INDEXED_LEVEL)VIR_Operand_GetRelAddrLevel(Operand);

            gctINT          relIndexing = VIR_Operand_GetRelIndexing(Operand);

            gctUINT32       indexRegister =
                _GetIndexedRegisterIndex(Converter, VirInst, Operand, relIndexing);

            gcSL_FORMAT         format      = _ConvVirType2Format(Converter, type);
            gcSHADER_PRECISION  precision   = _ConvVirOpndPrec2Prec(Operand);

            /* need adjust attribute index, the VIR shader uses virreg id */
            if (srcKind == gcSL_ATTRIBUTE)
            {
                /* find the attribute the symbol associated with */
                gcATTRIBUTE attribute =
                    _FindAttributeFromVirSym(Converter->VirShader,
                                             Converter->Shader, sym);
                gcmASSERT(attribute != gcvNULL);
                index = attribute->index;
            }

            gcSHADER_AddSourceIndexedWithPrecision(
                Converter->Shader,
                srcKind,
                index,
                swizzle,
                indexed,
                (gctUINT16)indexRegister,
                format,
                precision);

            *sourceIndex = gcmSL_INDEX_SET(*sourceIndex, ConstValue,
                VIR_Operand_GetMatrixConstIndex(Operand));

            if(!VIR_Operand_isLvalue(Operand) &&
                (VIR_Operand_GetModifier(Operand) & VIR_MOD_NEG))
            {
                *source = gcmSL_SOURCE_SET(*source, Neg, 1);
            }

            if(!VIR_Operand_isLvalue(Operand) &&
                (VIR_Operand_GetModifier(Operand) & VIR_MOD_ABS))
            {
                *source = gcmSL_SOURCE_SET(*source, Abs, 1);
            }

            *source = gcmSL_SOURCE_SET(*source, Indexed_Level, indexedLevel);

            break;
        }
    case VIR_OPND_CONST:
    case VIR_OPND_LABEL:
    case VIR_OPND_FUNCTION:
    case VIR_OPND_PARAMETERS:
    case VIR_OPND_NAME:
    case VIR_OPND_INTRINSIC:
    case VIR_OPND_FIELD:
    case VIR_OPND_ARRAY:
    default:
        gcmASSERT(0);
        break;
    }
    return status;
}

static gcSL_OPCODE
_GetOpcodeByTexldModifier(
    IN VIR_Operand *  Operand,
    IN Vir_TexldModifier_Name       Name
    )
{
    switch(Name)
    {
    case VIR_TEXLDMODIFIER_BIAS:
        return gcSL_TEXBIAS;
    case VIR_TEXLDMODIFIER_LOD:
        if (VIR_Operand_GetTexModifierFlag(Operand) & VIR_TMFLAG_LOD)
        {
            return gcSL_TEXLOD;
        }
        else
        {
            gcmASSERT(VIR_Operand_GetTexModifierFlag(Operand) & VIR_TMFLAG_FETCHMS);
            return gcSL_TEXFETCH_MS;
        }
    case VIR_TEXLDMODIFIER_DPDX:
    case VIR_TEXLDMODIFIER_DPDY:
        if (VIR_Operand_GetTexModifierFlag(Operand) & VIR_TMFLAG_GRAD)
        {
            return gcSL_TEXGRAD;
        }
        else
        {
            gcmASSERT(VIR_Operand_GetTexModifierFlag(Operand) & VIR_TMFLAG_GATHER);
            return gcSL_TEXGATHER;
        }
    case VIR_TEXLDMODIFIER_OFFSET:
    default:
        gcmASSERT(0);
        return gcSL_NOP;
    }
}

static gceSTATUS
_CloneVirOpnd2TmpOpnd(
    IN OUT Converter            *Converter,
    IN     VIR_Instruction      *Inst,
    IN     VIR_Operand          *Opnd,
    IN     gctUINT32            TempRegIndex,
    IN OUT gctUINT32            *TempReg,
    IN OUT VIR_Enable           *Enable,
    IN OUT gcSL_FORMAT          *Format,
    IN OUT gcSHADER_PRECISION   *Precision
    )
{
    gceSTATUS       status      = gcvSTATUS_OK;
    VIR_Type        *type       =
        VIR_Shader_GetTypeFromId(Converter->VirShader, VIR_Operand_GetTypeId(Opnd));
    gcSHADER_TYPE   gcShaderTy  = _ConvVirType2ShaderType(Converter, type);

    gcmASSERT(TempRegIndex < TEMP_REGISTER_COUNT);
#if SAVE_TEMP_REGISTER
    if(Converter->TmpRegs[TempRegIndex] == (gctUINT32)-1)
    {
        *TempReg = gcSHADER_NewTempRegs(Converter->Shader, 1, gcShaderTy);
        Converter->TmpRegs[TempRegIndex] = *TempReg;
    }
    else
    {
        *TempReg = Converter->TmpRegs[TempRegIndex];
    }
#else
    *TempReg = gcSHADER_NewTempRegs(Converter->Shader, 1, gcShaderTy);
#endif
    *Enable      = VIR_Inst_GetRelEnable(Converter, Inst, Opnd);
    *Format      = _ConvVirType2Format(Converter, type);
    *Precision   = _ConvVirOpndPrec2Prec(Opnd);

    return status;
}

extern gctBOOL
isConditionReversible(
    IN  gcSL_CONDITION   Condition,
    OUT gcSL_CONDITION * ReversedCondition
    );

static gceSTATUS
_ConvVirInst2Inst(
    IN OUT Converter   *Converter,
    IN VIR_Instruction *VirInst
    )
{
    gceSTATUS       status    = gcvSTATUS_OK;
    VIR_OpCode      opcode    = VIR_OP_NOP;
    gcSL_CONDITION  condition;
    gctUINT32       srcLoc;

    gcmASSERT(Converter->VirShader != gcvNULL &&
        VirInst != gcvNULL &&
        Converter->Shader != gcvNULL);

    srcLoc = GCSL_Build_SRC_LOC(VIR_Inst_GetSrcLocLine(VirInst),VIR_Inst_GetSrcLocCol(VirInst));

    opcode = VIR_Inst_GetOpcode(VirInst);
    condition = _ConvVirCondition2Condition(VIR_Inst_GetConditionOp(VirInst));
    switch(opcode)
    {
    case VIR_OP_IMG_LOAD:
    case VIR_OP_IMG_ADDR:
    case VIR_OP_IMG_LOAD_3D:
    case VIR_OP_CLAMP0MAX:
    case VIR_OP_CLAMPCOORD:
    case VIR_OP_NOP:        case VIR_OP_MOV:
    case VIR_OP_COMPARE:        case VIR_OP_SAT:
    case VIR_OP_ABS:
    case VIR_OP_FLOOR:      case VIR_OP_CEIL:
    case VIR_OP_POW:        case VIR_OP_LOG2:
    case VIR_OP_EXP2:       case VIR_OP_SIGN:
    case VIR_OP_FRAC:       case VIR_OP_RCP:
    case VIR_OP_RSQ:        case VIR_OP_SQRT:
    case VIR_OP_SIN:        case VIR_OP_COS:
    case VIR_OP_TAN:        case VIR_OP_ACOS:
    case VIR_OP_ASIN:       case VIR_OP_ATAN:
    case VIR_OP_SINPI:      case VIR_OP_COSPI:
    case VIR_OP_TANPI:      case VIR_OP_ADD:
    case VIR_OP_SUB:        case VIR_OP_MUL:
    case VIR_OP_DIV:        case VIR_OP_MOD:
    case VIR_OP_ADDSAT:
    case VIR_OP_SUBSAT:     case VIR_OP_MULSAT:
    case VIR_OP_MAX:        case VIR_OP_MIN:
    case VIR_OP_ADDLO:      case VIR_OP_MULLO:
    case VIR_OP_MULHI:
    case VIR_OP_NORM_MUL:
    case VIR_OP_NORM_DP2:
    case VIR_OP_NORM_DP3:   case VIR_OP_NORM_DP4:
    case VIR_OP_DP2:
    case VIR_OP_DP3:        case VIR_OP_DP4:
    case VIR_OP_NORM:       case VIR_OP_CROSS:
    case VIR_OP_STEP:       case VIR_OP_DSX:
    case VIR_OP_DSY:        case VIR_OP_FWIDTH:
    case VIR_OP_AND_BITWISE:case VIR_OP_OR_BITWISE:
    case VIR_OP_NOT_BITWISE:case VIR_OP_LSHIFT:
    case VIR_OP_RSHIFT:     case VIR_OP_ROTATE:
    case VIR_OP_LOAD:       case VIR_OP_LOAD_L:
    case VIR_OP_KILL:       case VIR_OP_XOR_BITWISE:
    case VIR_OP_BARRIER:
    case VIR_OP_MEM_BARRIER:
    case VIR_OP_BITSEL:     case VIR_OP_LEADZERO:
    case VIR_OP_GETEXP:     case VIR_OP_GETMANT:
    case VIR_OP_JMP:        case VIR_OP_CALL:
    case VIR_OP_JMPC:       case VIR_OP_JMP_ANY:
    case VIR_OP_RET:        case VIR_OP_PRE_DIV:
    case VIR_OP_PRE_LOG2:   case VIR_OP_SET:
    case VIR_OP_UCARRY:
    case VIR_OP_POPCOUNT:
    case VIR_OP_BITFIND_LSB:
    case VIR_OP_BITFIND_MSB:
    case VIR_OP_BITREV:
    case VIR_OP_GET_SAMPLER_IDX:
    case VIR_OP_GET_SAMPLER_LMM:
    case VIR_OP_GET_SAMPLER_LBS:
    case VIR_OP_F2I:
    case VIR_OP_I2F:
    case VIR_OP_CMP:
        {
            gctSIZE_T i = 0;

            _ConvVirOperand2Target(Converter, opcode, VIR_Inst_GetDest(VirInst), VirInst, condition, srcLoc);

            if (_ConvVirOpcode2Opcode(opcode) != gcSL_NOP)
            {
                for(i = 0; i < VIR_Inst_GetSrcNum(VirInst); ++i)
                {
                    _ConvVirOperand2Source(Converter, VirInst->src[i], VirInst, i);
                }
            }
            break;
        }
    case VIR_OP_IMG_STORE:
    case VIR_OP_IMG_STORE_3D:
        {
            /* For gcSL_IMAGE_STORE:
                    target is the data, src0 is the base address, src1 is the offset.
               For VIR_OP_IMG_STORE:
                    src0 is the base address, src1 is the offset, src2 is the data.
            */
            _ConvVirOperand2Target(Converter, opcode, VIR_Inst_GetSource(VirInst, 2), VirInst, condition, srcLoc);
            _ConvVirOperand2Source(Converter, VIR_Inst_GetSource(VirInst, 0), VirInst, 0);
            _ConvVirOperand2Source(Converter, VIR_Inst_GetSource(VirInst, 1), VirInst, 1);
            break;
        }
    case VIR_OP_ATOMADD:    case VIR_OP_ATOMSUB:
    case VIR_OP_ATOMXCHG:   case VIR_OP_ATOMMIN:
    case VIR_OP_ATOMMAX:    case VIR_OP_ATOMOR:
    case VIR_OP_ATOMAND:    case VIR_OP_ATOMXOR:
    case VIR_OP_ATOMCMPXCHG:
    case VIR_OP_ATOMCMPXCHG_L:
        {
            /* VIR_OP_ATOMOP 1, 2, 3, 4 */
                /* gcSL_LOAD   t, 2, 3 */
                /* gcSL_ATOMOP 1, t, 4 */

            gctUINT32       tmpReg;
            gcSL_FORMAT     format;
            VIR_Enable      enable;
            gcSL_TYPE       srcKind  = _ConvVirSymbol2Type(VIR_Operand_GetSymbol(VIR_Inst_GetDest(VirInst)));
            gcSL_SWIZZLE    swizzle;
            gcSHADER_PRECISION precision;

            _CloneVirOpnd2TmpOpnd(Converter, VirInst, VIR_Inst_GetDest(VirInst), 0, &tmpReg, &enable, &format, &precision);

            /* load t0, src0, src2 */
            gcSHADER_AddOpcode2(Converter->Shader, gcSL_LOAD, gcSL_ALWAYS, tmpReg, (gcSL_ENABLE)enable, format, precision, srcLoc);
            _ConvVirOperand2Source(Converter, VIR_Inst_GetSource(VirInst, 0), VirInst, 0);
            _ConvVirOperand2Source(Converter, VIR_Inst_GetSource(VirInst, 1), VirInst, 1);

            swizzle     = _ConvVirSwizzle2Swizzle(VIR_Enable_2_Swizzle_WShift(enable));

            /* atomop dest, t0, src1 */
            _ConvVirOperand2Target(Converter, opcode, VIR_Inst_GetDest(VirInst), VirInst, condition, srcLoc);
            gcSHADER_AddSource(Converter->Shader, srcKind, tmpReg, swizzle, format, precision);
            _ConvVirOperand2Source(Converter, VIR_Inst_GetSource(VirInst, 2), VirInst, 1);
            break;
        }

    case VIR_OP_STORE:
    case VIR_OP_STORE_L:
        {
            VIR_OperandInfo src1Info;

            VIR_Operand_GetOperandInfo(VirInst, VIR_Inst_GetSource(VirInst, 1), &src1Info);
            /* VIR_OP_STORE 1, 2, '0', 4 */
                /* gcSL_STORE1 1, 2, 4 */

            if(src1Info.isImmVal && src1Info.u1.immValue.iValue == 0)
            {
                _ConvVirOperand2Target(Converter, opcode, VIR_Inst_GetDest(VirInst), VirInst, condition, srcLoc);
                _ConvVirOperand2Source(Converter, VIR_Inst_GetSource(VirInst, 0), VirInst, 0);
                _ConvVirOperand2Source(Converter, VIR_Inst_GetSource(VirInst, 2), VirInst, 1);
            }
            else
            {
                /* VIR_OP_STORE 1, 2, 3, 4 */
                    /* gcSL_ADD    t, 2, 3 */
                    /* gcSL_STORE1 1, t, 4 */

                gctUINT32       tmpReg;
                gcSL_FORMAT     format;
                VIR_Enable      enable;
                gcSL_TYPE       srcKind  = _ConvVirSymbol2Type(VIR_Operand_GetSymbol(VIR_Inst_GetDest(VirInst)));
                gcSL_SWIZZLE    swizzle;
                gcSHADER_PRECISION precision;

                gcmASSERT(VIR_OpndInfo_Is_Virtual_Reg(&src1Info) || src1Info.u1.immValue.iValue != 0);
                _CloneVirOpnd2TmpOpnd(Converter, VirInst, VIR_Inst_GetSource(VirInst, 1), 0, &tmpReg, &enable, &format, &precision);

                /* add t0, src0, src2 */
                gcSHADER_AddOpcode2(Converter->Shader, gcSL_ADD, gcSL_ALWAYS, tmpReg, (gcSL_ENABLE)enable, format, precision, srcLoc);
                _ConvVirOperand2Source(Converter, VIR_Inst_GetSource(VirInst, 0), VirInst, 0);
                _ConvVirOperand2Source(Converter, VIR_Inst_GetSource(VirInst, 1), VirInst, 1);

                swizzle     = _ConvVirSwizzle2Swizzle(VIR_Enable_2_Swizzle_WShift(enable));

                /* store1 dest, t0, src1 */
                _ConvVirOperand2Target(Converter, opcode, VIR_Inst_GetDest(VirInst), VirInst, condition, srcLoc);
                gcSHADER_AddSource(Converter->Shader, srcKind, tmpReg, swizzle, format, precision);
                _ConvVirOperand2Source(Converter, VIR_Inst_GetSource(VirInst, 2), VirInst, 1);
            }
        }

        break;
    case VIR_OP_LABEL:
        break;
        /* mad dest, src0, src1, src2 */
    case VIR_OP_MAD:
        /* sad dest, src0, src1, src2 */
    case VIR_OP_SAD:
        {
            gctUINT32       tmpReg;
            VIR_Enable      enable;
            gcSL_FORMAT     format;

            gcSL_SWIZZLE    swizzle;

            gcSL_TYPE       srcKind  = _ConvVirSymbol2Type(VIR_Operand_GetSymbol(VIR_Inst_GetDest(VirInst)));
            gcSL_OPCODE     opcode0  = (opcode == VIR_OP_MAD) ? gcSL_MUL : gcSL_SUB;
            gcSHADER_PRECISION precision;

            _CloneVirOpnd2TmpOpnd(Converter, VirInst, VIR_Inst_GetDest(VirInst), 0, &tmpReg, &enable, &format, &precision);

            /* mul/sub newtmp src0 src1 */
            gcSHADER_AddOpcode2(Converter->Shader, opcode0, condition, tmpReg, enable, format, precision, srcLoc);
            _ConvVirOperand2Source(Converter, VIR_Inst_GetSource(VirInst, 0), VirInst, 0);
            _ConvVirOperand2Source(Converter, VIR_Inst_GetSource(VirInst, 1), VirInst, 1);

            swizzle     = _ConvVirSwizzle2Swizzle(VIR_Enable_2_Swizzle_WShift(enable));

            /* add dest newtmp src2*/
            _ConvVirOperand2Target(Converter, VIR_OP_ADD, VIR_Inst_GetDest(VirInst), VirInst, condition, srcLoc);
            gcSHADER_AddSource(Converter->Shader, srcKind, tmpReg, swizzle, format, precision);
            _ConvVirOperand2Source(Converter, VIR_Inst_GetSource(VirInst, 2), VirInst, 1);
        }
        break;
        /* madsat dest, src0, src1, src2 */
    case VIR_OP_MADSAT:
        {
            gctUINT32       tmpReg;
            VIR_Enable      enable;
            gcSL_FORMAT     format;

            gcSL_SWIZZLE    swizzle;
            gcSHADER_PRECISION precision;

            gcSL_TYPE       srcKind  = _ConvVirSymbol2Type(VIR_Operand_GetSymbol(VIR_Inst_GetDest(VirInst)));

            _CloneVirOpnd2TmpOpnd(Converter, VirInst, VIR_Inst_GetDest(VirInst), 0, &tmpReg, &enable, &format, &precision);

            /* mulsat newtmp src0 src1 */
            gcSHADER_AddOpcode2(Converter->Shader, gcSL_MULSAT, condition, tmpReg, enable, format, precision, srcLoc);
            _ConvVirOperand2Source(Converter, VIR_Inst_GetSource(VirInst, 0), VirInst, 0);
            _ConvVirOperand2Source(Converter, VIR_Inst_GetSource(VirInst, 1), VirInst, 1);

            swizzle     = _ConvVirSwizzle2Swizzle(VIR_Enable_2_Swizzle_WShift(enable));

            /* addsat dest newtmp src2*/
            _ConvVirOperand2Target(Converter, VIR_OP_ADDSAT, VIR_Inst_GetDest(VirInst), VirInst, condition, srcLoc);
            gcSHADER_AddSource(Converter->Shader, srcKind, tmpReg, swizzle, format, precision);
            _ConvVirOperand2Source(Converter, VIR_Inst_GetSource(VirInst, 2), VirInst, 1);
        }
        break;

        /* mix dest, src0, src1, src2 */
    case VIR_OP_MIX:
        {
            gctUINT32       tmpReg;
            VIR_Enable      enable;
            gcSL_FORMAT     format;

            gcSL_SWIZZLE    swizzle;
            gcSHADER_PRECISION precision;
            gcSL_TYPE       srcKind  = _ConvVirSymbol2Type(VIR_Operand_GetSymbol(VIR_Inst_GetDest(VirInst)));

            _CloneVirOpnd2TmpOpnd(Converter, VirInst, VIR_Inst_GetDest(VirInst), 0, &tmpReg, &enable, &format, &precision);

            /* sub newtmp src1 src0 */
            gcSHADER_AddOpcode2(Converter->Shader, gcSL_SUB, condition, tmpReg, enable, format, precision, srcLoc);
            _ConvVirOperand2Source(Converter, VIR_Inst_GetSource(VirInst, 1), VirInst, 0);
            _ConvVirOperand2Source(Converter, VIR_Inst_GetSource(VirInst, 0), VirInst, 1);

            swizzle     = _ConvVirSwizzle2Swizzle(VIR_Enable_2_Swizzle_WShift(enable));

            /* mul newtmp newtmp src2 */
            gcSHADER_AddOpcode2(Converter->Shader, gcSL_MUL, condition, tmpReg, enable, format, precision, srcLoc);
            gcSHADER_AddSource(Converter->Shader, srcKind, tmpReg, swizzle, format, precision);
            _ConvVirOperand2Source(Converter, VIR_Inst_GetSource(VirInst, 2), VirInst, 1);

            /* add dest newtmp src0 */
            _ConvVirOperand2Target(Converter, VIR_OP_ADD, VIR_Inst_GetDest(VirInst), VirInst, condition, srcLoc);
            gcSHADER_AddSource(Converter->Shader, srcKind, tmpReg, swizzle, format, precision);
            _ConvVirOperand2Source(Converter, VIR_Inst_GetSource(VirInst, 0), VirInst, 1);
        }
        break;
        /* select.condition dest, src0, src1, src2 */
    case VIR_OP_SELECT:
        {
            gctUINT32       dest0;
            gcSL_FORMAT     format;
            VIR_Enable      enable;
            gcSHADER_PRECISION precision;

            VIR_Symbol      *sym        = VIR_Operand_GetSymbol(VIR_Inst_GetDest(VirInst));
            _CloneVirOpnd2TmpOpnd(Converter, VirInst, VIR_Inst_GetSource(VirInst, 0), 0, &dest0, &enable, &format, &precision);

            /* SET.cond dest0, src0, src1 */
            gcSHADER_AddOpcode2(Converter->Shader, gcSL_SET, condition, dest0, gcSL_ENABLE_XYZW, format, precision, srcLoc);
            _ConvVirOperand2Source(Converter, VIR_Inst_GetSource(VirInst, 0), VirInst, 0);
            _ConvVirOperand2Source(Converter, VIR_Inst_GetSource(VirInst, 1), VirInst, 1);

            /* CMP.z dest, dest0, src2 */
            _ConvVirOperand2Target(Converter, VIR_OP_COMPARE, VIR_Inst_GetDest(VirInst), VirInst, gcSL_ZERO, srcLoc);
            gcSHADER_AddSource(Converter->Shader, _ConvVirSymbol2Type(sym), dest0, gcSL_SWIZZLE_XYZW, format, precision);
            _ConvVirOperand2Source(Converter, VIR_Inst_GetSource(VirInst, 2), VirInst, 1);

            /* CMP.nz dest, dest0, src1 */
            _ConvVirOperand2Target(Converter, VIR_OP_COMPARE, VIR_Inst_GetDest(VirInst), VirInst, gcSL_NOT_ZERO, srcLoc);
            gcSHADER_AddSource(Converter->Shader, _ConvVirSymbol2Type(sym), dest0, gcSL_SWIZZLE_XYZW, format, precision);
            _ConvVirOperand2Source(Converter, VIR_Inst_GetSource(VirInst, 1), VirInst, 1);
        }
        break;
        /* select.condition dest, src0, src1, src2 */
    case VIR_OP_CSELECT:
        {
            gctUINT32       tmpReg = 0;
            VIR_Enable      enable = VIR_ENABLE_NONE;
            gcSL_FORMAT     format = gcSL_INVALID;
            gcSL_SWIZZLE    swizzle = gcSL_SWIZZLE_INVALID;
            gcSHADER_PRECISION precision = gcSHADER_PRECISION_ANY;
            gcSL_TYPE       srcKind = gcSL_NONE;
            gcSL_CONDITION  reversed = gcSL_ALWAYS;
            gctBOOL         needMove = gcvFALSE;
            gctBOOL         canReverse = gcvFALSE;

            gcmASSERT(VIR_Inst_GetSrcNum(VirInst) == 3 &&
                VIR_Inst_GetDest(VirInst) != gcvNULL);
            gcmASSERT(condition == gcSL_ZERO || condition == gcSL_NOT_ZERO ||
                      condition == gcSL_LESS_ZERO || condition == gcSL_GREATER_OR_EQUAL_ZERO ||
                      condition == gcSL_LESS_OREQUAL_ZERO || condition == gcSL_GREATER_ZERO ||
                      condition == gcSL_ALLMSB || condition == gcSL_ANYMSB || condition == gcSL_SELMSB
                      );

            needMove = _ExpandSelectNeedMove(Converter, VirInst);

            if(needMove)
            {
                _CloneVirOpnd2TmpOpnd(Converter, VirInst, VIR_Inst_GetDest(VirInst), 0, &tmpReg, &enable, &format, &precision);
                swizzle  = _ConvVirSwizzle2Swizzle(VIR_Enable_2_Swizzle_WShift(enable));
                srcKind  = _ConvVirSymbol2Type(VIR_Operand_GetSymbol(VIR_Inst_GetDest(VirInst)));
            }

            canReverse = isConditionReversible(condition, &reversed);
            if(needMove)
            {
                /* CMP.z dest0, src0, src1 */
                gcSHADER_AddOpcode2(Converter->Shader, gcSL_CMP, condition, tmpReg, enable, format, precision, srcLoc);
                _ConvVirOperand2Source(Converter, VIR_Inst_GetSource(VirInst, 0), VirInst, 0);
                _ConvVirOperand2Source(Converter, VIR_Inst_GetSource(VirInst, 1), VirInst, 1);

                /* CMP.nz dest0, src0, src2 */
                gcSHADER_AddOpcode2(Converter->Shader, gcSL_CMP, canReverse ? reversed : condition, tmpReg, enable, format, precision, srcLoc);
                _ConvVirOperand2Source(Converter, VIR_Inst_GetSource(VirInst, 0), VirInst, 0);
                _ConvVirOperand2Source(Converter, VIR_Inst_GetSource(VirInst, 2), VirInst, 1);

                /* MOV    dest, dest0 */
                _ConvVirOperand2Target(Converter, VIR_OP_MOV, VIR_Inst_GetDest(VirInst), VirInst, gcSL_ALWAYS, srcLoc);
                gcSHADER_AddSource(Converter->Shader, srcKind, tmpReg, swizzle, format, precision);
            }
            else
            {
                /* CMP.z dest, src0, src1 */
                _ConvVirOperand2Target(Converter, VIR_OP_COMPARE, VIR_Inst_GetDest(VirInst), VirInst, condition, srcLoc);
                _ConvVirOperand2Source(Converter, VIR_Inst_GetSource(VirInst, 0), VirInst, 0);
                _ConvVirOperand2Source(Converter, VIR_Inst_GetSource(VirInst, 1), VirInst, 1);

                /* CMP.nz dest, src0, src2 */
                _ConvVirOperand2Target(Converter, VIR_OP_COMPARE, VIR_Inst_GetDest(VirInst), VirInst, canReverse ? reversed : condition, srcLoc);
                _ConvVirOperand2Source(Converter, VIR_Inst_GetSource(VirInst, 0), VirInst, 0);
                _ConvVirOperand2Source(Converter, VIR_Inst_GetSource(VirInst, 2), VirInst, 1);
            }
        }
        break;
        /* cmov.cond    dst, src0, src1, src2 */
            /* cmp.cond     dst0, src0, src1       */
            /* cmp.z     dst, dst0, dst  */
            /* cmp.nz    dst, dst0, src2 */
    case VIR_OP_CMOV:
        {
            gctUINT32       tmpReg;
            gcSL_FORMAT     format;
            VIR_Enable      enable;
            gcSL_SWIZZLE    swizzle;
            gcSHADER_PRECISION precision = gcSHADER_PRECISION_ANY;

            VIR_Symbol      *sym        = VIR_Operand_GetSymbol(VIR_Inst_GetDest(VirInst));

            _CloneVirOpnd2TmpOpnd(Converter, VirInst, VIR_Inst_GetDest(VirInst), 0, &tmpReg, &enable, &format, &precision);

            enable   = VIR_ENABLE_X;
            swizzle  = gcSL_SWIZZLE_X;

            /* cmp.cond     dst0, src0, src1       */
            gcSHADER_AddOpcode2(Converter->Shader, gcSL_CMP, condition, tmpReg, enable, format, precision, srcLoc);
            _ConvVirOperand2Source(Converter, VIR_Inst_GetSource(VirInst, 0), VirInst, 0);
            _ConvVirOperand2Source(Converter, VIR_Inst_GetSource(VirInst, 1), VirInst, 1);

            /* cmp.z     dst, dst0, dst  */
            _ConvVirOperand2Target(Converter, VIR_OP_COMPARE, VIR_Inst_GetDest(VirInst), VirInst, gcSL_ZERO, srcLoc);
            gcSHADER_AddSource(Converter->Shader, _ConvVirSymbol2Type(sym), tmpReg, swizzle, format, precision);
            _ConvVirOperand2Source(Converter, VIR_Inst_GetDest(VirInst), VirInst, 1);

            /* cmp.nz    dst, dst0, src2 */
            _ConvVirOperand2Target(Converter, VIR_OP_COMPARE, VIR_Inst_GetDest(VirInst), VirInst, gcSL_NOT_ZERO, srcLoc);
            gcSHADER_AddSource(Converter->Shader, _ConvVirSymbol2Type(sym), tmpReg, swizzle, format, precision);
            _ConvVirOperand2Source(Converter, VIR_Inst_GetSource(VirInst, 2), VirInst, 1);

            break;
        }
        /* texld dest, src0, src1, src2 */
    case VIR_OP_TEXLD:
    case VIR_OP_TEXLD_U:
    case VIR_OP_TEXLDPCF:
    case VIR_OP_TEXLDPCFPROJ:
    case VIR_OP_TEXLDPROJ:
        {
            VIR_Operand *texldOperand = gcvNULL;
            gctSIZE_T                  i            = 0;

            gcmASSERT(VIR_Inst_GetSrcNum(VirInst) == 4 &&
                (VIR_Operand_GetOpKind(VIR_Inst_GetSource(VirInst, 2)) == VIR_OPND_TEXLDPARM ||
                       VIR_Operand_GetOpKind(VIR_Inst_GetSource(VirInst, 2)) == VIR_OPND_UNDEF) &&
                VIR_Inst_GetDest(VirInst) != gcvNULL);

            texldOperand = ((VIR_Operand *)VIR_Inst_GetSource(VirInst, 2));

            /* texbias src0, texldModifier */
            if (VIR_Operand_GetOpKind(VIR_Inst_GetSource(VirInst, 2)) == VIR_OPND_TEXLDPARM)
            {
                for (i = 0; i < VIR_TEXLDMODIFIER_COUNT; ++i)
                {
                    if (VIR_Operand_GetTexldModifier(texldOperand, i) != gcvNULL)
                    {
                        gcSHADER_AddOpcode(
                            Converter->Shader,
                            _GetOpcodeByTexldModifier(texldOperand,
                                                      (Vir_TexldModifier_Name)i),
                            0,
                            gcSL_ENABLE_NONE,
                            gcSL_FLOAT,
                            gcSHADER_PRECISION_DEFAULT,
                            0);

                        if ((VIR_Operand_GetTexModifierFlag(texldOperand) & VIR_TMFLAG_GRAD ||
                             VIR_Operand_GetTexModifierFlag(texldOperand) & VIR_TMFLAG_GATHER) &&
                            (Vir_TexldModifier_Name)i == VIR_TEXLDMODIFIER_DPDX &&
                            VIR_Operand_GetTexldModifier(texldOperand, i+1) != gcvNULL)
                        {
                            gcmASSERT((Vir_TexldModifier_Name)(i + 1) == VIR_TEXLDMODIFIER_DPDY);
                            _ConvVirOperand2Source(Converter, VIR_Operand_GetTexldModifier(texldOperand, i), VirInst, 0);
                            _ConvVirOperand2Source(Converter, VIR_Operand_GetTexldModifier(texldOperand, i), VirInst, 1);
                            i++;
                        }
                        else
                        {
                            _ConvVirOperand2Source(Converter, VIR_Inst_GetSource(VirInst, 0), VirInst, 0);
                            _ConvVirOperand2Source(Converter, VIR_Operand_GetTexldModifier(texldOperand, i), VirInst, 1);
                        }
                        break;
                    }
                }
            }

            /* texld dest src0, src1 */
            _ConvVirOperand2Target(Converter, opcode, VIR_Inst_GetDest(VirInst), VirInst, gcSL_ALWAYS, srcLoc);
            _ConvVirOperand2Source(Converter, VIR_Inst_GetSource(VirInst, 0), VirInst, 0);
            _ConvVirOperand2Source(Converter, VIR_Inst_GetSource(VirInst, 1), VirInst, 1);
            break;
        }
    case VIR_OP_LDARR:
        {
            /* indexed array load:
            LDARR dest, src0, src1
            ==>
            MOV dest, src0[src1]
            */
            gcSL_INSTRUCTION inst       = gcvNULL;

            VIR_Symbol      *sym        = VIR_Operand_GetSymbol(VIR_Inst_GetSource(VirInst, 1));
            VIR_OperandInfo src1Info;
            gcSL_FORMAT     format = _ConvVirType2Format(Converter,
                VIR_Shader_GetTypeFromId(Converter->VirShader, VIR_Operand_GetTypeId(VIR_Inst_GetSource(VirInst, 1))));

            VIR_Operand_GetOperandInfo(VirInst, VIR_Inst_GetSource(VirInst, 1), &src1Info);

            if(VIR_OpndInfo_Is_Virtual_Reg(&src1Info) && !src1Info.isInput)
            {
                gctUINT         index       = _GetRegisterIndex(Converter, sym, VIR_Inst_GetSource(VirInst, 1));
                gcSL_INDEXED    indexed     =
                    _ConvVirOpndSwizzle2Indexd(VIR_Inst_GetSource(VirInst, 1));

                /* mov dest, src0[src1] */
                _ConvVirOperand2Target(Converter, VIR_OP_MOV, VIR_Inst_GetDest(VirInst), VirInst, gcSL_ALWAYS, srcLoc);
                if (VIR_Operand_GetOpKind(VIR_Inst_GetSource(VirInst, 0)) == VIR_OPND_SAMPLER_INDEXING)
                {
                    gcSL_SWIZZLE swizzle = (gcSL_SWIZZLE)gcmComposeSwizzle(indexed - 1,
                                                                           indexed - 1,
                                                                           indexed - 1,
                                                                           indexed - 1);

                    inst = &Converter->Shader->code[SHADER_LASTINST(Converter)];
                    inst->source0 = gcmSL_SOURCE_SET(0, Format, format)
                                  | gcmSL_SOURCE_SET(0, Indexed, gcSL_NOT_INDEXED)
                                  | gcmSL_SOURCE_SET(0, Precision, gcSHADER_PRECISION_DEFAULT);

                    inst->source0 = gcmSL_SOURCE_SET(inst->source0, Swizzle, swizzle);
                    inst->source0 = gcmSL_SOURCE_SET(inst->source0, Type, gcSL_TEMP);
                    inst->source0Index = (gctUINT32)index;
                }
                else
                {
                    _ConvVirOperand2Source(Converter, VIR_Inst_GetSource(VirInst, 0), VirInst, 0);
                    inst = &Converter->Shader->code[SHADER_LASTINST(Converter)];
                    inst->source0 = gcmSL_SOURCE_SET(inst->source0,
                        Indexed, indexed);
                    inst->source0Indexed = (gctUINT16)index;
                }
            }
            else
            {
                gctUINT32       tmpReg;
                VIR_Enable      enable;
                gcSHADER_PRECISION precision;

                _CloneVirOpnd2TmpOpnd(Converter, VirInst, VIR_Inst_GetSource(VirInst, 1), 0, &tmpReg, &enable, &format, &precision);

                /* mov t0, src1 */
                gcSHADER_AddOpcode2(Converter->Shader, gcSL_MOV, gcSL_ALWAYS, tmpReg, gcSL_ENABLE_X, format, precision, srcLoc);
                _ConvVirOperand2Source(Converter, VIR_Inst_GetSource(VirInst, 1), VirInst, 0);

                /* mov dest, src0[t0] */
                _ConvVirOperand2Target(Converter, VIR_OP_MOV, VIR_Inst_GetDest(VirInst), VirInst, gcSL_ALWAYS, srcLoc);
                if (VIR_Operand_GetOpKind(VIR_Inst_GetSource(VirInst, 0)) == VIR_OPND_SAMPLER_INDEXING)
                {
                    inst = &Converter->Shader->code[SHADER_LASTINST(Converter)];
                    inst->source0 = gcmSL_SOURCE_SET(0, Format, format)
                                  | gcmSL_SOURCE_SET(0, Indexed, gcSL_NOT_INDEXED)
                                  | gcmSL_SOURCE_SET(0, Precision, gcSHADER_PRECISION_DEFAULT);

                    inst->source0 = gcmSL_SOURCE_SET(inst->source0, Swizzle, gcSL_SWIZZLE_XXXX);
                    inst->source0 = gcmSL_SOURCE_SET(inst->source0, Type, gcSL_TEMP);
                    inst->source0Index = tmpReg;
                }
                else
                {
                    _ConvVirOperand2Source(Converter, VIR_Inst_GetSource(VirInst, 0), VirInst, 0);
                    inst = &Converter->Shader->code[SHADER_LASTINST(Converter)];
                    inst->source0 = gcmSL_SOURCE_SET(inst->source0,
                        Indexed, gcSL_INDEXED_X);
                    inst->source0Indexed = (gctUINT16)tmpReg;
                }
            }
        }
        break;
        /* indexed array store: STARR dest, src0, src1 ==> dest[src0] = src1 */
    case VIR_OP_STARR:
        {
            gcSL_INSTRUCTION inst       = gcvNULL;
            VIR_Symbol      *sym        = VIR_Operand_GetSymbol(VIR_Inst_GetSource(VirInst, 0));

            VIR_OperandInfo src0Info;

            VIR_Operand_GetOperandInfo(VirInst, VIR_Inst_GetSource(VirInst, 0), &src0Info);

            if(VIR_OpndInfo_Is_Virtual_Reg(&src0Info) && !src0Info.isInput)
            {
                gctUINT         index       = _GetRegisterIndex(Converter, sym, VIR_Inst_GetSource(VirInst, 0));
                gcSL_INDEXED    indexed     =
                    _ConvVirOpndSwizzle2Indexd(VIR_Inst_GetSource(VirInst, 0));

                /* mov dest[src0], src1 */
                _ConvVirOperand2Target(Converter, VIR_OP_MOV, VIR_Inst_GetDest(VirInst), VirInst, gcSL_ALWAYS, srcLoc);
                inst = &Converter->Shader->code[SHADER_LASTINST(Converter)];
                inst->temp = gcmSL_TARGET_SET(inst->temp, Indexed, indexed);
                inst->tempIndexed = (gctUINT16)index;
                _ConvVirOperand2Source(Converter, VIR_Inst_GetSource(VirInst, 1), VirInst, 0);
            }
            else
            {
                gctUINT32       tmpReg;
                gcSL_FORMAT     format;
                VIR_Enable      enable;
                gcSHADER_PRECISION precision;

                _CloneVirOpnd2TmpOpnd(Converter, VirInst, VIR_Inst_GetSource(VirInst, 0), 0, &tmpReg, &enable, &format, &precision);

                /* mov t0, src0 */
                gcSHADER_AddOpcode2(Converter->Shader, gcSL_MOV, gcSL_ALWAYS, tmpReg, gcSL_ENABLE_X, format, precision, srcLoc);
                _ConvVirOperand2Source(Converter, VIR_Inst_GetSource(VirInst, 0), VirInst, 0);

                /* mov dest[t0], src1 */
                _ConvVirOperand2Target(Converter, VIR_OP_MOV, VIR_Inst_GetDest(VirInst), VirInst, gcSL_ALWAYS, srcLoc);
                inst = &Converter->Shader->code[SHADER_LASTINST(Converter)];
                inst->temp = gcmSL_TARGET_SET(inst->temp, Indexed, gcSL_INDEXED_X);
                inst->tempIndexed = (gctUINT16)tmpReg;
                _ConvVirOperand2Source(Converter, VIR_Inst_GetSource(VirInst, 1), VirInst, 0);
            }
        }
        break;
    case VIR_OP_CONVERT:
        {
            VIR_Type   *type = VIR_Shader_GetTypeFromId(Converter->VirShader,
                VIR_Operand_GetTypeId(VIR_Inst_GetSource(VirInst, 0)));
            gcSL_FORMAT format = _ConvVirType2Format(Converter, type);

            _ConvVirOperand2Target(Converter, opcode, VIR_Inst_GetDest(VirInst), VirInst, condition, srcLoc);
            _ConvVirOperand2Source(Converter, VIR_Inst_GetSource(VirInst, 0), VirInst, 0);

            gcSHADER_AddSourceConstantFormattedWithPrecision(
                Converter->Shader,
                &format,
                gcSL_UINT32,
                gcSHADER_PRECISION_HIGH
                );
        }
        break;
    case VIR_OP_MOVA:
        {
            VIR_Operand     *src0;
            VIR_OperandInfo  src0Info;

            gcmASSERT(VIR_Shader_isRegAllocated(Converter->VirShader));

            gcmASSERT(VIR_Inst_GetSrcNum(VirInst) == 1);
            src0 = VIR_Inst_GetSource(VirInst, 0);
            VIR_Operand_GetOperandInfo(VirInst,
                    src0,
                    &src0Info);

            if (src0Info.isImmVal)
            {
                if (VIR_Operand_GetTypeId(src0) == VIR_TYPE_FLOAT32)
                {
                    VIR_Operand_SetTypeId(VIR_Inst_GetDest(VirInst), VIR_TYPE_FLOAT32);
                }
                else
                {
                    VIR_Operand_SetTypeId(VIR_Inst_GetDest(VirInst), VIR_TYPE_INT32);
                }
            }

            _ConvVirOperand2Target(Converter, opcode, VIR_Inst_GetDest(VirInst), VirInst, condition, srcLoc);
            _ConvVirOperand2Source(Converter, src0, VirInst, 0);

        }
        break;
        /* neg dest, src0 */
        /*    sub dest, 0, src0 */
    case VIR_OP_NEG:
        {
            VIR_Type *ty = VIR_Shader_GetTypeFromId(Converter->VirShader, VIR_Operand_GetTypeId(VIR_Inst_GetDest(VirInst)));
            gctUINT   zero = 0;

            _ConvVirOperand2Target(Converter, opcode, VIR_Inst_GetDest(VirInst), VirInst, condition, srcLoc);

            gcSHADER_AddSourceConstantFormattedWithPrecision(
                Converter->Shader, &zero,
                _ConvVirType2Format(Converter, ty),
                gcSHADER_PRECISION_HIGH);

            _ConvVirOperand2Source(Converter, VIR_Inst_GetSource(VirInst, 0), VirInst, 1);
        }
        break;
    case VIR_OP_BITEXTRACT:
        {
            /* bitextract dest, src0 ==>
               bitrange t1, src1, src2
               bitextract dest, src0 */

            _ConvVirOperand2Target(Converter, VIR_OP_BITRANGE, VIR_Inst_GetDest(VirInst), VirInst, gcSL_ALWAYS, srcLoc);
            _ConvVirOperand2Source(Converter, VIR_Inst_GetSource(VirInst, 1), VirInst, 0);
            _ConvVirOperand2Source(Converter, VIR_Inst_GetSource(VirInst, 2), VirInst, 1);

            _ConvVirOperand2Target(Converter, opcode, VIR_Inst_GetDest(VirInst), VirInst, condition, srcLoc);
            _ConvVirOperand2Source(Converter, VIR_Inst_GetSource(VirInst, 0), VirInst, 0);
        }
        break;

        case VIR_OP_BITINSERT:
        {
            /* bitinsert dest, src0, src1, src2, src3 ==>
               bitrange1 dest, src2, src3
               bitinsert dest, src0, src1 */

            _ConvVirOperand2Target(Converter, VIR_OP_BITRANGE, VIR_Inst_GetDest(VirInst), VirInst, condition, srcLoc);
            _ConvVirOperand2Source(Converter, VIR_Inst_GetSource(VirInst, 2), VirInst, 0);
            _ConvVirOperand2Source(Converter, VIR_Inst_GetSource(VirInst, 3), VirInst, 1);

            _ConvVirOperand2Target(Converter, opcode, VIR_Inst_GetDest(VirInst), VirInst, condition, srcLoc);
            _ConvVirOperand2Source(Converter, VIR_Inst_GetSource(VirInst, 0), VirInst, 0);
            _ConvVirOperand2Source(Converter, VIR_Inst_GetSource(VirInst, 1), VirInst, 1);
        }
        break;

        case VIR_OP_BITINSERT1:
        case VIR_OP_BITINSERT2:
        {
            /* bitinsert1 dest, src0, src1, src2 ==>

               bitrange1 t2, src2
               bitinsert dest, src0, src1 */

            _ConvVirOperand2Target(Converter, VIR_OP_BITRANGE1, VIR_Inst_GetDest(VirInst), VirInst, gcSL_ALWAYS, srcLoc);
            _ConvVirOperand2Source(Converter, VIR_Inst_GetSource(VirInst, 2), VirInst, 0);

            /* bitinsert dest, src0, src1 */
            _ConvVirOperand2Target(Converter, opcode, VIR_Inst_GetDest(VirInst), VirInst, condition, srcLoc);
            _ConvVirOperand2Source(Converter, VIR_Inst_GetSource(VirInst, 0), VirInst, 0);
            _ConvVirOperand2Source(Converter, VIR_Inst_GetSource(VirInst, 1), VirInst, 1);
        }
        break;

        case VIR_OP_TEXLD_U_PLAIN:
        {
            /* texld_u_plain dest, src0, src1, src2 ==>

               texu  dest, src2
               texld dest, src0, src1 */

            /* texu  dest, src2 */
            _ConvVirOperand2Target(Converter, opcode, VIR_Inst_GetDest(VirInst), VirInst, condition, srcLoc);
            _ConvVirOperand2Source(Converter, VIR_Inst_GetSource(VirInst, 2), VirInst, 0);

            /* texld dest, src0, src1 */
            _ConvVirOperand2Target(Converter, VIR_OP_TEXLD, VIR_Inst_GetDest(VirInst), VirInst, condition, srcLoc);
            _ConvVirOperand2Source(Converter, VIR_Inst_GetSource(VirInst, 0), VirInst, 0);
            _ConvVirOperand2Source(Converter, VIR_Inst_GetSource(VirInst, 1), VirInst, 1);
        }
        break;

        case VIR_OP_TEXLD_U_LOD:
        {
            /* texld_u_lod dest, src0, src1, src2, src3 ==>

               texu_lod  dest, src2, src3
               texld dest, src0, src1 */

            /* texu_lod  dest, src2_modifier, src2 */
            _ConvVirOperand2Target(Converter, opcode, VIR_Inst_GetDest(VirInst), VirInst, condition, srcLoc);
            _ConvVirOperand2Source(Converter, VIR_Inst_GetSource(VirInst, 2), VirInst, 0);
            _ConvVirOperand2Source(Converter, VIR_Inst_GetSource(VirInst, 3), VirInst, 1);

            /* texld dest src0, src1 */
            _ConvVirOperand2Target(Converter, VIR_OP_TEXLD, VIR_Inst_GetDest(VirInst), VirInst, gcSL_ALWAYS, srcLoc);
            _ConvVirOperand2Source(Converter, VIR_Inst_GetSource(VirInst, 0), VirInst, 0);
            _ConvVirOperand2Source(Converter, VIR_Inst_GetSource(VirInst, 1), VirInst, 1);

        }
        break;

    default:
        gcmASSERT(0);
        break;
    }

    return status;
}

static gceSTATUS
_ConvVirFunction2Function(
    IN OUT Converter   *Converter,
    IN OUT gcFUNCTION   Func,
    IN VIR_Function    *VirFunc
    )
{
    gceSTATUS    status = gcvSTATUS_OK;
    gctSIZE_T    i = 0;
    /* VIR_Symbol  *funcSymbol = gcvNULL; */

    if(!(VirFunc->flags & VIR_FUNCFLAG_MAIN))
    {
        gcmASSERT(Func != gcvNULL);

        gcSHADER_BeginFunction(Converter->Shader,
            Func);

        /* Convert paramters */
        for(i = 0; i < VIR_IdList_Count(&VirFunc->paramters); ++i)
        {
            VIR_Id      id   = VIR_IdList_GetId(&VirFunc->paramters, i);
            VIR_Symbol *sym  = VIR_Function_GetSymFromId(VirFunc, id);
            VIR_Type   *type = gcvNULL;
            VIR_StorageClass qualifier = VIR_Symbol_GetStorageClass(sym);
            gceINPUT_OUTPUT io = _ConvVirParamQualifier2ParamQualifier(qualifier);

            if(sym == gcvNULL)
            {
                return gcvSTATUS_NOT_FOUND;
            }

            type = VIR_Symbol_GetType(sym);
            if(type == gcvNULL)
            {
                return gcvSTATUS_NOT_FOUND;
            }

            gcFUNCTION_AddArgument(Func, 0xffff,
                (gctUINT32)_GetRegisterIndex(Converter, sym, gcvNULL),
                _ConvVirType2Enable(type),
                io,
                VIR_Symbol_GetPrecision(sym),
                VIR_Symbol_HasFlag(sym, VIR_SYMFLAG_PRECISE));
        }

        /* Flags */
#if gcvFUNC_NOATTR
        if(VirFunc->flags & VIR_FUNCFLAG_NOATTR)
        {
            Func->flags |= gcvFUNC_NOATTR;
        }
#endif

        if(VirFunc->flags & VIR_FUNCFLAG_INTRINSICS)
        {
            Func->flags |= gcvFUNC_INTRINSICS;
        }

        if(VirFunc->flags & VIR_FUNCFLAG_STATIC)
        {
            Func->flags |= gcvFUNC_STATIC;
        }

        if(VirFunc->flags & VIR_FUNCFLAG_EXTERN)
        {
            Func->flags |= gcvFUNC_EXTERN;
        }

        if(VirFunc->flags & VIR_FUNCFLAG_ALWAYSINLINE)
        {
            Func->flags |= gcvFUNC_ALWAYSINLINE;
        }

        if(VirFunc->flags & VIR_FUNCFLAG_NOINLINE)
        {
            Func->flags |= gcvFUNC_NOINLINE;
        }

        if(VirFunc->flags & VIR_FUNCFLAG_INLINEHINT)
        {
            Func->flags |= gcvFUNC_INLINEHINT;
        }

        if(VirFunc->flags & VIR_FUNCFLAG_READNONE)
        {
            Func->flags |= gcvFUNC_READNONE;
        }

        if(VirFunc->flags & VIR_FUNCFLAG_READONLY)
        {
            Func->flags |= gcvFUNC_READONLY;
        }

        if(VirFunc->flags & VIR_FUNCFLAG_STRUCTRET)
        {
            Func->flags |= gcvFUNC_STRUCTRET;
        }

        if(VirFunc->flags & VIR_FUNCFLAG_NORETURN)
        {
            Func->flags |= gcvFUNC_NORETURN;
        }

        if(VirFunc->flags & VIR_FUNCFLAG_INREG)
        {
            Func->flags |= gcvFUNC_INREG;
        }

        if(VirFunc->flags & VIR_FUNCFLAG_BYVAL)
        {
            Func->flags |= gcvFUNC_BYVAL;
        }

        if(VirFunc->flags & VIR_FUNCFLAG_RECURSIVE)
        {
            Func->isRecursion = gcvTRUE;
        }
    }

    {
        VIR_Instruction  *inst;
        VIR_InstIterator instIter;

        VIR_InstIterator_Init(&instIter, &VirFunc->instList);
        inst = (VIR_Instruction *)VIR_InstIterator_First(&instIter);

        for (; inst != gcvNULL; inst = (VIR_Instruction *)VIR_InstIterator_Next(&instIter))
        {
            _ConvVirInst2Inst(Converter, inst);
        }
    }

    if(!(VirFunc->flags & VIR_FUNCFLAG_MAIN))
    {
        gcmASSERT(Func != gcvNULL);
        gcSHADER_EndFunction(Converter->Shader, Func);
        /* copy function temp index start and count from VirFunction */
        Func->tempIndexStart = VirFunc->tempIndexStart;
        Func->tempIndexCount = VirFunc->tempIndexCount;
    }
    return status;
}

static gceSTATUS
_BuildLabelByFunction(
    IN Converter    *Converter,
    IN VIR_Function *Func,
    IN OUT gctUINT  *InstCount
    )
{
    VIR_Instruction   *inst = gcvNULL;
    VIR_InstIterator   instIter;
    gceSTATUS status    = gcvSTATUS_OK;

    gcmASSERT(Converter != gcvNULL &&
        Func != gcvNULL &&
        InstCount != gcvNULL);

    VIR_InstIterator_Init(&instIter, &Func->instList);
    inst = (VIR_Instruction *)VIR_InstIterator_First(&instIter);

    for (; inst != gcvNULL; inst = (VIR_Instruction *)VIR_InstIterator_Next(&instIter))
    {
        VIR_OpCode opCode = VIR_Inst_GetOpcode(inst);

        if (opCode == VIR_OP_LABEL)
        {
            gcSHADER_LABEL label;
            _AddValue(Converter->InstTable, inst, *InstCount);

            /* Allocate a new gcSHADER_LABEL structure.  */
            status = gcoOS_Allocate(gcvNULL,
                sizeof(struct _gcSHADER_LABEL),
                (gctPOINTER *)&label);

            if (gcmIS_ERROR(status))
            {
                /* Error. */
                return status;
            }

            /* Initialize the gcSHADER_LABEL structure. */
            label->next       = Converter->Shader->labels;
            label->label      = *InstCount;
            label->defined    = *InstCount;
            label->referenced = gcvNULL;
            label->function   = gcvNULL;

            /* Move gcSHADER_LABEL structure to head of list. */
            Converter->Shader->labels = label;
        }

        *InstCount += _CalculateInstCount(Converter, inst);
    }

    return status;
}

/* add default ubo item */
static gceSTATUS
_gcCreateAuxUBOLite(
    IN gcSHADER Shader,
    IN gctSTRING Name,
    OUT gcsUNIFORM_BLOCK* UB
    )
{
    gcsSHADER_VAR_INFO blockInfo[1];
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("Shader=0x%x", Shader);

    gcoOS_ZeroMemory(blockInfo, sizeof(blockInfo[0]));
    blockInfo->varCategory = gcSHADER_VAR_CATEGORY_BLOCK;
    blockInfo->format = gcSL_FLOAT;
    blockInfo->precision = gcSHADER_PRECISION_DEFAULT;
    blockInfo->arraySize = 1;
    blockInfo->u.numBlockElement = 0;
    blockInfo->prevSibling = -1;
    blockInfo->nextSibling = -1;
    blockInfo->parent = -1;

    gcmONERROR(gcSHADER_AddUniformBlock(Shader,
                                        Name,
                                        blockInfo,
                                        gcvINTERFACE_BLOCK_PACKED,
                                        -1,
                                        0,
                                        UB));

OnError:
    gcmFOOTER();
    return status;
}

static gceSTATUS
_MarkFunctionFlag(
    IN VIR_Shader*      pShader
    )
{
    VIR_FunctionNode*   pFuncNode;
    VIR_FuncIterator    funcIter;

    VIR_FuncIterator_Init(&funcIter, VIR_Shader_GetFunctions(pShader));

    for (pFuncNode = VIR_FuncIterator_First(&funcIter);
         pFuncNode != gcvNULL;
         pFuncNode =  VIR_FuncIterator_Next(&funcIter))
    {
        if (pFuncNode->function != gcvNULL)
        {
            VIR_Function*       pFunc = pFuncNode->function;
            VIR_Instruction*    pInst = gcvNULL;
            VIR_InstIterator    instIter;

            /* So far we only mark flag HAS_CALL_OP. */
            if (VIR_Function_HasFlag(pFunc, VIR_FUNCFLAG_HAS_CALL_OP))
            {
                continue;
            }

            VIR_InstIterator_Init(&instIter, VIR_Function_GetInstList(pFunc));

            for (pInst = (VIR_Instruction *)VIR_InstIterator_First(&instIter);
                 pInst != gcvNULL;
                 pInst = (VIR_Instruction *)VIR_InstIterator_Next(&instIter))
            {
                VIR_OpCode opCode = VIR_Inst_GetOpcode(pInst);

                if (opCode == VIR_OP_CALL)
                {
                    VIR_Function_SetFlag(pFunc, VIR_FUNCFLAG_HAS_CALL_OP);
                    break;
                }
            }
        }
    }

    return gcvSTATUS_OK;
}

static void
_SortFunctions(
    IN VIR_Shader*      pShader
    )
{
    VSC_MM*             pMM = &pShader->pmp.mmWrapper;
    VIR_FunctionNode*   pFuncNode;
    VIR_FuncIterator    funcIter;
    VIR_FunctionList    newFunctionList;
    VIR_FunctionNode*   pNewFuncNode = gcvNULL;

    if (VIR_Shader_GetFunctionCount(pShader) == 1)
    {
        return;
    }

    vscBILST_Initialize(&newFunctionList, gcvFALSE);

    /*
    ** I: add those functions that don't call the other function,
    ** so when add the caller function, the label is already set.
    */
    VIR_FuncIterator_Init(&funcIter, VIR_Shader_GetFunctions(pShader));
    for (pFuncNode = VIR_FuncIterator_First(&funcIter);
         pFuncNode != gcvNULL;
         pFuncNode = VIR_FuncIterator_Next(&funcIter))
    {
        VIR_Function*   pVirFunc;

        pVirFunc = pFuncNode->function;

        /* Skip the main function and the functions use CALL instruction. */
        if (pVirFunc == VIR_Shader_GetMainFunction(pShader) || VIR_Function_HasFlag(pVirFunc, VIR_FUNCFLAG_HAS_CALL_OP))
        {
            continue;
        }

        /* add function to functionList */
        pNewFuncNode = (VIR_FunctionNode *)vscMM_Alloc(pMM, sizeof(VIR_FunctionNode));
        gcmASSERT(pNewFuncNode != gcvNULL);
        pNewFuncNode->function = pVirFunc;
        vscBILST_Append(&newFunctionList, (VSC_BI_LIST_NODE*)pNewFuncNode);
    }

    /* II: add the other non-main functions that use CALL instruction. */
    VIR_FuncIterator_Init(&funcIter, VIR_Shader_GetFunctions(pShader));
    for (pFuncNode = VIR_FuncIterator_First(&funcIter);
         pFuncNode != gcvNULL;
         pFuncNode = VIR_FuncIterator_Next(&funcIter))
    {
        VIR_Function*   pVirFunc;

        pVirFunc = pFuncNode->function;

        /* Skip the main function and the functions don't use CALL instruction. */
        if (pVirFunc == VIR_Shader_GetMainFunction(pShader) || !VIR_Function_HasFlag(pVirFunc, VIR_FUNCFLAG_HAS_CALL_OP))
        {
            continue;
        }

        /* add function to functionList */
        pNewFuncNode = (VIR_FunctionNode *)vscMM_Alloc(pMM, sizeof(VIR_FunctionNode));
        gcmASSERT(pNewFuncNode != gcvNULL);
        pNewFuncNode->function = pVirFunc;
        vscBILST_Append(&newFunctionList, (VSC_BI_LIST_NODE*)pNewFuncNode);
    }

    /* III: add the main function. */
    pNewFuncNode = (VIR_FunctionNode *)vscMM_Alloc(pMM, sizeof(VIR_FunctionNode));
    gcmASSERT(pNewFuncNode != gcvNULL);
    pNewFuncNode->function = VIR_Shader_GetMainFunction(pShader);
    vscBILST_Append(&newFunctionList, (VSC_BI_LIST_NODE*)pNewFuncNode);

    /* Make sure that every single function is processed. */
    gcmASSERT(VIR_Shader_GetFunctionCount(pShader) == vscBILST_GetNodeCount(&newFunctionList));

    /* Update the function list. */
    pShader->functions = newFunctionList;
}

/*

*/
gceSTATUS
gcSHADER_ConvFromVIR(
    IN OUT gcSHADER Shader,
    IN SHADER_HANDLE hVirShader,
    IN  gceSHADER_FLAGS Flags
    )
{
    gceSTATUS status    = gcvSTATUS_OK;
    VIR_Shader* VirShader = (VIR_Shader*)hVirShader;
    Converter converter = { VirShader, Shader, gcvNULL, gcvNULL };
    gctUINT    i, duboMemberIndex;
    gctBOOL seperatedShader = Flags & gcvSHADER_SEPERATED_PROGRAM;
    gcsUNIFORM_BLOCK dubo = gcvNULL, cubo = gcvNULL;
    gctBOOL useFullNewLinker = gcUseFullNewLinker(gcHWCaps.hwFeatureFlags.hasHalti2);
    gcKERNEL_FUNCTION  *kernelFunctions = gcvNULL;

    gcmHEADER_ARG("Shader=0x%x VirShader=0x%x", Shader, VirShader);

    /* Verify the arguments. */
    /*
    gcmVERIFY_OBJECT(Shader, gcvOBJ_SHADER);
    gcmDEBUG_VERIFY_ARGUMENT(VirShader);
    */
    if(!Shader) {
        gcmFATAL("gcSHADER_ConvFromVIR: null shader handle passed");
        gcmFOOTER_ARG("status=%d", gcvSTATUS_INVALID_ARGUMENT);
        return gcvSTATUS_INVALID_ARGUMENT;
    }

    _InitialConverter(&converter);

    /* convert shader flags */
    if (VIR_Shader_IsOldHeader(VirShader))
    {
        gcShaderSetIsOldHeader(Shader);
    }

    if (VIR_Shader_HasUnsizedSBO(VirShader))
    {
        gcShaderSetHasUnsizedSBO(Shader);
    }

    if (VIR_Shader_IsEnableMultiGPU(VirShader))
    {
        gcShaderSetEnableMultiGPU(Shader);
    }

    Shader->_constVectorId      = VirShader->_constVectorId;
    Shader->_dummyUniformCount  = VirShader->_dummyUniformCount;
    Shader->_id                 = VirShader->_id;

    /* handling ES3.1 data */
    switch (VirShader->shaderKind)
    {
    case VIR_SHADER_COMPUTE:
        for (i = 0; i < 3; i++)
        {
            Shader->shaderLayout.compute.workGroupSize[i] = VirShader->shaderLayout.compute.workGroupSize[i];
            Shader->shaderLayout.compute.workGroupSizeFactor[i] = VirShader->shaderLayout.compute.workGroupSizeFactor[i];
        }
        Shader->shaderLayout.compute.isWorkGroupSizeFixed = VirShader->shaderLayout.compute.isWorkGroupSizeFixed;
        Shader->shaderLayout.compute.isWorkGroupSizeAdjusted = VirShader->shaderLayout.compute.isWorkGroupSizeAdjusted;
        Shader->shaderLayout.compute.adjustedWorkGroupSize = VirShader->shaderLayout.compute.adjustedWorkGroupSize;
        break;
    case VIR_SHADER_TESSELLATION_CONTROL:
        Shader->shaderLayout.tcs.tcsPatchOutputVertices = VirShader->shaderLayout.tcs.tcsPatchOutputVertices;
        Shader->shaderLayout.tcs.tcsPatchInputVertices = VirShader->shaderLayout.tcs.tcsPatchInputVertices;
        break;
    case VIR_SHADER_TESSELLATION_EVALUATION:
        Shader->shaderLayout.tes.tessPrimitiveMode =
            (gcTessPrimitiveMode)VirShader->shaderLayout.tes.tessPrimitiveMode;
        Shader->shaderLayout.tes.tessVertexSpacing =
            (gcTessVertexSpacing)VirShader->shaderLayout.tes.tessVertexSpacing;
        Shader->shaderLayout.tes.tessOrdering =
            (gcTessOrdering)VirShader->shaderLayout.tes.tessOrdering;
        Shader->shaderLayout.tes.tessPointMode =
            VirShader->shaderLayout.tes.tessPointMode;
        break;
    case VIR_SHADER_GEOMETRY:
        Shader->shaderLayout.geo.geoInvocations =
            VirShader->shaderLayout.geo.geoInvocations;
        Shader->shaderLayout.geo.geoMaxVertices =
            VirShader->shaderLayout.geo.geoMaxVertices;
        Shader->shaderLayout.geo.geoInPrimitive =
            (gcGeoPrimitive)VirShader->shaderLayout.geo.geoInPrimitive;
        Shader->shaderLayout.geo.geoOutPrimitive =
            (gcGeoPrimitive)VirShader->shaderLayout.geo.geoOutPrimitive;
        break;
    default:
        break;
    }

    if (VirShader->shaderKind == VIR_SHADER_FRAGMENT)
    {
        Shader->isDual16Shader = VirShader->__IsDual16Shader;
    }

    if (VIR_Shader_UseLocalMem(VirShader))
    {
        gcShaderSetFlag(Shader, gcSHADER_FLAG_USE_LOCAL_MEM);
    }

    /* Add uniforms. Assume only inserting const values. */

#ifdef ADD_UNIFORM_CONSTANT
    {
        gctSIZE_T     i       = 0;
        gctSIZE_T     j       = 0;
        gctSIZE_T     count   = 0;

        VIR_Uniform  *virUniform = gcvNULL;
        gcUNIFORM    gcUniform   = gcvNULL;

        for(i = 0; i < Shader->uniformCount; ++i)
        {
            gcUniform = Shader->uniforms[i];
            gcmASSERT(gcUniform->index == i);
            if(!isUniformStruct(Shader->uniforms[i]))
            {
                ++count;
            }
        }
        j = 0;
        for(i = 0; i < count; ++i)
        {
            VIR_Id      id  = VIR_IdList_GetId(&VirShader->uniforms, i);
            VIR_Symbol *sym = VIR_Shader_GetSymFromId(VirShader, id);

            virUniform = sym->u2.uniform;

            while(virUniform->index != Shader->uniforms[j]->index)
            {
                ++j;
                gcmASSERT(Shader->uniformCount > j);
            }

            _AddValue(converter.UniformTable, virUniform, virUniform->index);
        }
        /******************************************************************************

        Original gcUniform like this:
index:       0         1         2         3
        -----------------------------------------
category| struct1 | normal1 | normal2 | struct2 |
        -----------------------------------------

        VirUniform like this:
index:       1         2         3         4         5
        ---------------------------------------------------
category| normal1 | normal2 | number1 | number2 | number3 |
        ---------------------------------------------------

        Converted gcUniform like this:
index:        0        1         2         3         4         5        6
        -----------------------------------------------------------------------
category| struct1 | normal1 | normal2 | struct2 | number1 | number2 | number3 |
        -----------------------------------------------------------------------

        *******************************************************************************/
        if(VIR_IdList_Count(&VirShader->uniforms) > count)
        {
            for(i = count; i < VIR_IdList_Count(&VirShader->uniforms); ++i)
            {
                VIR_Id      id  = VIR_IdList_GetId(&VirShader->uniforms, i);
                VIR_Symbol *sym = VIR_Shader_GetSymFromId(VirShader, id);
                VIR_Type   *type= VIR_Symbol_GetType(sym);

                gcsValue    value;
                VIR_Const  *virConst;

                virUniform = sym->u2.uniform;

                virConst = (VIR_Const *)VIR_GetSymFromId(&VirShader->constTable,
                    virUniform->initializer);

                gcmASSERT(virConst != gcvNULL);

                _ConvVirConstVal2gcsConst(virConst->value, &value);

                gcSHADER_CreateConstantUniform(Shader,
                    _ConvVirType2ShaderType(&converter, type),
                    &value,
                    &gcUniform
                    );

                _AddValue(converter.UniformTable, virUniform, gcUniform->index);
            }
        }
    }
#else

#endif
    /* set _defaultUniformBlockSize if default ubo is constructed in VIR */
    if (VIR_Shader_GetDefaultUBOIndex(VirShader) != -1)
    {
        VIR_UBOIdList* ubo_idlist = VIR_Shader_GetUniformBlocks(VirShader);
        VIR_Id dubo_id = VIR_IdList_GetId(ubo_idlist, VIR_Shader_GetDefaultUBOIndex(VirShader));
        VIR_Symbol* virDUBOSym = VIR_Shader_GetSymFromId(VirShader, dubo_id);
        VIR_Symbol* baseAddress_sym;
        VIR_Uniform* baseAddress_uniform;
        VIR_UniformBlock* virDUBO = VIR_Symbol_GetUBO(virDUBOSym);
        gcUNIFORM baseAddressUniform;

        if (virDUBO->uniformCount > 0)
        {
            _gcCreateAuxUBOLite(Shader, "#DefaultUBO", &dubo);
            SetUBBlockSize(dubo, virDUBO->blockSize);
            SetUBUniformCount(dubo, virDUBO->uniformCount);
            gcmONERROR(gcoOS_Allocate(gcvNULL,
                       gcmSIZEOF(gcUNIFORM) * GetUBUniformCount(dubo),
                       (gctPOINTER *)&dubo->uniforms));

            /* Mapping the base address. */
            gcSHADER_GetUniform(Shader, GetUBIndex(dubo), &baseAddressUniform);
            baseAddress_sym = VIR_Shader_GetSymFromId(VirShader, virDUBO->baseAddr);
            gcmASSERT(VIR_Symbol_isUniform(baseAddress_sym));
            baseAddress_uniform = VIR_Symbol_GetUniform(baseAddress_sym);
            baseAddress_uniform->gcslIndex = baseAddressUniform->index;

            while(VIR_Uniform_GetAuxAddrSymId(baseAddress_uniform) != VIR_INVALID_ID)
            {
                VIR_SymId auxAddrSymId = VIR_Uniform_GetAuxAddrSymId(baseAddress_uniform);
                VIR_Symbol* auxAddrSym = VIR_Shader_GetSymFromId(VirShader, auxAddrSymId);
                VIR_Uniform* auxAddrUniform = VIR_Symbol_GetUniform(auxAddrSym);
                gcUNIFORM gcslAuxAddrUniform;

                gcSHADER_AddUniform(Shader,
                                    VIR_Shader_GetSymNameString(VirShader, auxAddrSym),
                                    gcSHADER_UINT_X1,
                                    1,
                                    gcSHADER_PRECISION_HIGH,
                                    &gcslAuxAddrUniform);

                /* Link back to each other */
                SetUniformFollowingOffset(gcslAuxAddrUniform, VIR_Uniform_GetOffset(auxAddrUniform));
                SetUniformCategory(gcslAuxAddrUniform, gcSHADER_VAR_CATEGORY_BLOCK_ADDRESS);
                gcslAuxAddrUniform->blockIndex   = GetUBBlockIndex(dubo);
                SetUniformFlag(gcslAuxAddrUniform, gcvUNIFORM_FLAG_USED_IN_SHADER);
                SetUniformFlag(gcslAuxAddrUniform, gcvUNIFORM_FLAG_COMPILER_GEN);


                SetUniformFollowingAddr(baseAddressUniform, gcslAuxAddrUniform);
                baseAddress_uniform = auxAddrUniform;
                baseAddressUniform = gcslAuxAddrUniform;
            }
        }
    }

    /* set _defaultConstantUniformBlockSize if default cubo is constructed in VIR */
    if (VIR_Shader_GetConstantUBOIndex(VirShader) != -1)
    {
        VIR_UBOIdList* ubo_idlist = VIR_Shader_GetUniformBlocks(VirShader);
        VIR_Id cubo_id = VIR_IdList_GetId(ubo_idlist, VIR_Shader_GetConstantUBOIndex(VirShader));
        VIR_Symbol* virCUBOSym = VIR_Shader_GetSymFromId(VirShader, cubo_id);
        VIR_Symbol* virCUBOAddressSym;
        VIR_Uniform* virCUBOAddressUniform;
        VIR_UniformBlock* virCUBO = VIR_Symbol_GetUBO(virCUBOSym);
        gcUNIFORM baseAddressUniform;
        gctPOINTER pointer = gcvNULL;

        _gcCreateAuxUBOLite(Shader, "#ConstantUBO", &cubo);
        SetUBBlockSize(cubo, virCUBO->blockSize);
        gcSHADER_GetUniform(Shader, GetUBIndex(cubo), &baseAddressUniform);
        virCUBOAddressSym = VIR_Shader_GetSymFromId(VirShader, virCUBO->baseAddr);
        gcmASSERT(VIR_Symbol_isUniform(virCUBOAddressSym));
        virCUBOAddressUniform = VIR_Symbol_GetUniform(virCUBOAddressSym);
        virCUBOAddressUniform->gcslIndex = baseAddressUniform->index;

        /* Allocate a memory to hold these constants. */
        gcmASSERT(Shader->constUBOData == gcvNULL);
        gcmONERROR(gcoOS_Allocate(gcvNULL,
                                    virCUBO->blockSize,
                                    &pointer));
        /* Zero this memory. */
        gcoOS_ZeroMemory(pointer, virCUBO->blockSize);

        Shader->constUBOData = pointer;
    }

    duboMemberIndex = 0;

    /* update uniforms in gcsl IR */
    for (i = 0; i < VIR_IdList_Count(VIR_Shader_GetUniforms(VirShader)); i++)
    {
        VIR_Id virUniformSymId = VIR_IdList_GetId(VIR_Shader_GetUniforms(VirShader), i);
        VIR_Symbol* virUniformSym = VIR_Shader_GetSymFromId(VirShader, virUniformSymId);
        VIR_Uniform* virUniform = VIR_Symbol_GetUniformPointer(VirShader, virUniformSym);

        if (virUniform)
        {
            if(virUniform->gcslIndex == -1)
            {
                if(VIR_Symbol_GetUniformKind(virUniformSym) == VIR_UNIFORM_LOD_MIN_MAX)
                {
                    gctINT16 lodMinMaxIndex;
                    gcUNIFORM lodMinMaxUniform;
                    gcUNIFORM samplerUniform;
                    gctINT16 lastChildIndex;
                    VIR_Symbol * pSym = VIR_Shader_GetSymFromId(VirShader, virUniform->u.samplerOrImageAttr.parentSamplerSymId);
                    VIR_Uniform * pUniform = VIR_Symbol_GetSampler(pSym);

                    gcmASSERT(pUniform && pUniform->gcslIndex != -1);
                    gcSHADER_GetUniform(Shader, pUniform->gcslIndex, &samplerUniform);
                    lastChildIndex = samplerUniform->firstChild;
                    while (lastChildIndex != -1)
                    {
                        gcUNIFORM temp= gcvNULL;
                        gcSHADER_GetUniform(Shader, (gctUINT)lastChildIndex, &temp);
                        gcmASSERT(temp);
                        lastChildIndex = temp->index;
                        if (temp->nextSibling != -1)
                        {
                            lastChildIndex = temp->nextSibling;
                        }
                        else
                        {
                            break;
                        }
                    }
                    /* Create lodMinMax. */
                    gcmONERROR(gcSHADER_AddUniformEx1(Shader,
                                                      VIR_Shader_GetSymNameString(VirShader, virUniformSym),
                                                      gcSHADER_FLOAT_X4,
                                                      gcSHADER_PRECISION_MEDIUM,
                                                      -1,
                                                      -1,
                                                      -1,
                                                      0,
                                                      gcvNULL,
                                                      gcSHADER_VAR_CATEGORY_LOD_MIN_MAX,
                                                      0,
                                                      samplerUniform->index,
                                                      lastChildIndex,
                                                      gcIMAGE_FORMAT_DEFAULT,
                                                      &lodMinMaxIndex,
                                                      &lodMinMaxUniform));
                    virUniform->gcslIndex = lodMinMaxIndex;
                }
                else if(VIR_Symbol_GetUniformKind(virUniformSym) == VIR_UNIFORM_LEVEL_BASE_SIZE)
                {
                    gctINT16 levelBaseSizeIndex;
                    gcUNIFORM levelBaseSizeUniform;
                    gcUNIFORM samplerUniform;
                    gctINT16 lastChildIndex;
                    VIR_Symbol * pSym = VIR_Shader_GetSymFromId(VirShader, virUniform->u.samplerOrImageAttr.parentSamplerSymId);
                    VIR_Uniform * pUniform = VIR_Symbol_GetSampler(pSym);

                    gcmASSERT(pUniform && pUniform->gcslIndex != -1);
                    gcSHADER_GetUniform(Shader, pUniform->gcslIndex, &samplerUniform);
                    lastChildIndex = samplerUniform->firstChild;
                    while (lastChildIndex != -1)
                    {
                        gcUNIFORM temp= gcvNULL;
                        gcSHADER_GetUniform(Shader, (gctUINT)lastChildIndex, &temp);
                        gcmASSERT(temp);
                        lastChildIndex = temp->index;
                        if (temp->nextSibling != -1)
                        {
                            lastChildIndex = temp->nextSibling;
                        }
                        else
                        {
                            break;
                        }
                    }
                    /* Create levelBaseSize. */
                    gcmONERROR(gcSHADER_AddUniformEx1(Shader,
                                                      VIR_Shader_GetSymNameString(VirShader, virUniformSym),
                                                      gcSHADER_INTEGER_X4,
                                                      gcSHADER_PRECISION_MEDIUM,
                                                      -1,
                                                      -1,
                                                      -1,
                                                      0,
                                                      gcvNULL,
                                                      gcSHADER_VAR_CATEGORY_LEVEL_BASE_SIZE,
                                                      0,
                                                      samplerUniform->index,
                                                      lastChildIndex,
                                                      gcIMAGE_FORMAT_DEFAULT,
                                                      &levelBaseSizeIndex,
                                                      &levelBaseSizeUniform));
                    virUniform->gcslIndex = levelBaseSizeIndex;
                }
                else if(VIR_Symbol_GetUniformKind(virUniformSym) == VIR_UNIFORM_CONST_BORDER_VALUE)
                {
                    gcUNIFORM uniform;

                    gcmONERROR(gcSHADER_AddUniformEx(Shader,
                                                     VIR_Shader_GetSymNameString(VirShader, virUniformSym),
                                                     gcSHADER_UINT_X4,
                                                     gcSHADER_PRECISION_HIGH,
                                                     1,
                                                     &uniform));

                    virUniform->gcslIndex = GetUniformIndex(uniform);
                }
                else if(VIR_Symbol_GetUniformKind(virUniformSym) == VIR_UNIFORM_WORK_THREAD_COUNT)
                {
                    gctINT16 workThreadCountIndex;
                    gcUNIFORM workThreadCountUniform;

                    /* Create workThreadCount. */
                    gcmONERROR(gcSHADER_AddUniformEx1(Shader,
                                                      VIR_Shader_GetSymNameString(VirShader, virUniformSym),
                                                      gcSHADER_UINT16_X1,
                                                      gcSHADER_PRECISION_MEDIUM,
                                                      -1,
                                                      -1,
                                                      -1,
                                                      0,
                                                      gcvNULL,
                                                      gcSHADER_VAR_CATEGORY_WORK_THREAD_COUNT,
                                                      0,
                                                      -1,
                                                      -1,
                                                      gcIMAGE_FORMAT_DEFAULT,
                                                      &workThreadCountIndex,
                                                      &workThreadCountUniform));
                    virUniform->gcslIndex = workThreadCountIndex;
                }
                else if(VIR_Symbol_GetUniformKind(virUniformSym) == VIR_UNIFORM_WORK_GROUP_COUNT)
                {
                    gctINT16 workGroupCountIndex;
                    gcUNIFORM workGroupCountUniform;

                    /* Create workGroupCount. */
                    gcmONERROR(gcSHADER_AddUniformEx1(Shader,
                                                      VIR_Shader_GetSymNameString(VirShader, virUniformSym),
                                                      gcSHADER_UINT16_X1,
                                                      gcSHADER_PRECISION_MEDIUM,
                                                      -1,
                                                      -1,
                                                      -1,
                                                      0,
                                                      gcvNULL,
                                                      gcSHADER_VAR_CATEGORY_WORK_GROUP_COUNT,
                                                      0,
                                                      -1,
                                                      -1,
                                                      gcIMAGE_FORMAT_DEFAULT,
                                                      &workGroupCountIndex,
                                                      &workGroupCountUniform));
                    virUniform->gcslIndex = workGroupCountIndex;
                }
                else if(VIR_Symbol_GetUniformKind(virUniformSym) == VIR_UNIFORM_WORK_GROUP_ID_OFFSET)
                {
                    gctINT16 workGroupIdOffsetIndex;
                    gcUNIFORM workGroupIdOffsetUniform;

                    /* Create workGroupIdOffset. */
                    gcmONERROR(gcSHADER_AddUniformEx1(Shader,
                                                      VIR_Shader_GetSymNameString(VirShader, virUniformSym),
                                                      gcSHADER_UINT_X3,
                                                      gcSHADER_PRECISION_HIGH,
                                                      -1,
                                                      -1,
                                                      -1,
                                                      0,
                                                      gcvNULL,
                                                      gcSHADER_VAR_CATEGORY_WORK_GROUP_ID_OFFSET,
                                                      0,
                                                      -1,
                                                      -1,
                                                      gcIMAGE_FORMAT_DEFAULT,
                                                      &workGroupIdOffsetIndex,
                                                      &workGroupIdOffsetUniform));
                    virUniform->gcslIndex = workGroupIdOffsetIndex;
                }
                else if(VIR_Symbol_GetUniformKind(virUniformSym) == VIR_UNIFORM_GLOBAL_INVOCATION_ID_OFFSET)
                {
                    gctINT16 globalIdOffsetIndex;
                    gcUNIFORM globalIdOffsetUniform;

                    /* Create globalInvocationId. */
                    gcmONERROR(gcSHADER_AddUniformEx1(Shader,
                                                      VIR_Shader_GetSymNameString(VirShader, virUniformSym),
                                                      gcSHADER_UINT_X3,
                                                      gcSHADER_PRECISION_HIGH,
                                                      -1,
                                                      -1,
                                                      -1,
                                                      0,
                                                      gcvNULL,
                                                      gcSHADER_VAR_CATEGORY_GLOBAL_INVOCATION_ID_OFFSET,
                                                      0,
                                                      -1,
                                                      -1,
                                                      gcIMAGE_FORMAT_DEFAULT,
                                                      &globalIdOffsetIndex,
                                                      &globalIdOffsetUniform));
                    virUniform->gcslIndex = globalIdOffsetIndex;
                }
                else if(VIR_Symbol_GetUniformKind(virUniformSym) == VIR_UNIFORM_VIEW_INDEX)
                {
                    gctINT16 viewIndexIndex;
                    gcUNIFORM viewIndexUniform;

                    /* Create viewIndex. */
                    gcmONERROR(gcSHADER_AddUniformEx1(Shader,
                                                      VIR_Shader_GetSymNameString(VirShader, virUniformSym),
                                                      gcSHADER_INTEGER_X1,
                                                      gcSHADER_PRECISION_HIGH,
                                                      -1,
                                                      -1,
                                                      -1,
                                                      0,
                                                      gcvNULL,
                                                      gcSHADER_VAR_CATEGORY_VIEW_INDEX,
                                                      0,
                                                      -1,
                                                      -1,
                                                      gcIMAGE_FORMAT_DEFAULT,
                                                      &viewIndexIndex,
                                                      &viewIndexUniform));
                    virUniform->gcslIndex = viewIndexIndex;
                }
                else if (VIR_Symbol_GetUniformKind(virUniformSym) == VIR_UNIFORM_THREAD_ID_MEM_ADDR)
                {
                    gctINT16 threadIdIndex;
                    gcUNIFORM threadIdUniform;

                    /* Create threadID. */
                    gcmONERROR(gcSHADER_AddUniformEx1(Shader,
                                                        VIR_Shader_GetSymNameString(VirShader, virUniformSym),
                                                        gcSHADER_UINT_X3,
                                                        gcSHADER_PRECISION_HIGH,
                                                        -1,
                                                        -1,
                                                        -1,
                                                        0,
                                                        gcvNULL,
                                                        gcSHADER_VAR_CATEGORY_THREAD_ID_MEM_ADDR,
                                                        0,
                                                        -1,
                                                        -1,
                                                        gcIMAGE_FORMAT_DEFAULT,
                                                        &threadIdIndex,
                                                        &threadIdUniform));
                    virUniform->gcslIndex = threadIdIndex;
                }
                else if (VIR_Symbol_GetUniformKind(virUniformSym) == VIR_UNIFORM_TEMP_REG_SPILL_MEM_ADDRESS)
                {
                    /* Convert the tempRegSpillMem only when multiGPU is enabled. */
                    if (VirShader->hasRegisterSpill && gcShaderEnableMultiGPU(Shader))
                    {
                        gcsSHADER_VAR_INFO      blockInfo[1];
                        gctSTRING               blockName = VIR_Shader_GetSymNameString(VirShader, virUniformSym);
                        gcsSTORAGE_BLOCK        pTempRegSpillSbo = gcvNULL;
                        gcUNIFORM               pTempRegSpillUniform = gcvNULL;

                        gcmASSERT(VirShader->vidmemSizeOfSpill > 0);

                        gcoOS_ZeroMemory(blockInfo, gcmSIZEOF(gcsSHADER_VAR_INFO));

                        blockInfo->varCategory = gcSHADER_VAR_CATEGORY_BLOCK;
                        blockInfo->format = gcSL_FLOAT;
                        blockInfo->precision = gcSHADER_PRECISION_HIGH;
                        blockInfo->arraySize = 1;
                        blockInfo->u.numBlockElement = 1;
                        blockInfo->firstChild = -1;
                        blockInfo->nextSibling = -1;
                        blockInfo->prevSibling = -1;
                        blockInfo->parent = -1;

                        gcmONERROR(gcSHADER_AddStorageBlock(Shader,
                                                            blockName,
                                                            blockInfo,
                                                            gcvINTERFACE_BLOCK_SHARED,
                                                            &pTempRegSpillSbo));
                        SetSBBlockSize(pTempRegSpillSbo, VirShader->vidmemSizeOfSpill);
                        gcmONERROR(gcSHADER_GetUniform(Shader, GetSBIndex(pTempRegSpillSbo), &pTempRegSpillUniform));
                        SetUniformKind(pTempRegSpillUniform, gcvUNIFORM_KIND_TEMP_REG_SPILL_ADDRESS);
                        virUniform->gcslIndex = GetUniformIndex(pTempRegSpillUniform);
                    }
                }
                else if (VIR_Symbol_GetUniformKind(virUniformSym) == VIR_UNIFORM_CLIP_DISTANCE_ENABLE)
                {
                    gctINT16 clipDistanceIndex;
                    gcUNIFORM clipDistanceUniform;

                    /* Create viewIndex. */
                    gcmONERROR(gcSHADER_AddUniformEx1(Shader,
                                                        VIR_Shader_GetSymNameString(VirShader, virUniformSym),
                                                        gcSHADER_INTEGER_X1,
                                                        gcSHADER_PRECISION_HIGH,
                                                        -1,
                                                        -1,
                                                        -1,
                                                        0,
                                                        gcvNULL,
                                                        gcSHADER_VAR_CATEGORY_CLIP_DISTANCE_ENABLE,
                                                        0,
                                                        -1,
                                                        -1,
                                                        gcIMAGE_FORMAT_DEFAULT,
                                                        &clipDistanceIndex,
                                                        &clipDistanceUniform));
                    virUniform->gcslIndex = clipDistanceIndex;
                }
                else
                {
                }
            }

            if(virUniform->gcslIndex != -1)
            {
                gcUNIFORM mappingGCSLUniform = Shader->uniforms[virUniform->gcslIndex];
                /* if uniform is put into aubo by VIR, update its attributes */
                if (VIR_Symbol_HasFlag(virUniformSym, VIR_SYMUNIFORMFLAG_MOVED_TO_DUBO))
                {
                    gcSHADER_TYPE base_type;

                    base_type = GetUniformType(mappingGCSLUniform);
                    dubo->uniforms[duboMemberIndex++] = mappingGCSLUniform;
                    SetUniformCategory(mappingGCSLUniform, gcSHADER_VAR_CATEGORY_BLOCK_MEMBER); /* becuase VIR is not converted back to gcSL, load instructions generated by DUBO are not there in gcSL, so skip this setting  */
                    mappingGCSLUniform->blockIndex = (gctINT16)Shader->_defaultUniformBlockIndex;
                    mappingGCSLUniform->offset = virUniform->offset;
                    if (mappingGCSLUniform->arraySize > 1)
                    {
                        gctUINT base_type_size = gcmType_Comonents(base_type) * gcmType_Rows(base_type) * gcmType_ComponentByteSize;
                        SetUniformArrayStride(mappingGCSLUniform, base_type_size);
                    }
                    if (gcmType_Rows(base_type) > 1)
                    {
                        if (mappingGCSLUniform->isRowMajor)
                        {
                            SetUniformMatrixStride(mappingGCSLUniform, (gctINT16)gcmType_Rows(base_type) * gcmType_ComponentByteSize);
                        }
                        else
                        {
                            SetUniformMatrixStride(mappingGCSLUniform, (gctINT16)gcmType_Comonents(base_type) * gcmType_ComponentByteSize);
                        }
                    }
                    SetUniformFlag(mappingGCSLUniform, gcvUNIFORM_FLAG_MOVED_TO_DUBO);
                }

                if (VIR_Symbol_HasFlag(virUniformSym, VIR_SYMUNIFORMFLAG_USED_IN_SHADER))
                {
                    SetUniformFlag(mappingGCSLUniform, gcvUNIFORM_FLAG_USED_IN_SHADER);
                }
                else
                {
                    ResetUniformFlag(mappingGCSLUniform, gcvUNIFORM_FLAG_USED_IN_SHADER);
                }

                if (VIR_Symbol_HasFlag(virUniformSym, VIR_SYMUNIFORMFLAG_COMPILETIME_INITIALIZED))
                {
                    VIR_Type *type = VIR_Symbol_GetType(virUniformSym);

                    if(!VIR_Type_isArray(type)) {
                        VIR_ConstId initializerId = VIR_Uniform_GetInitializer(virUniform);
                        VIR_Const* initializerConst = VIR_Shader_GetConstFromId(VirShader, initializerId);
                        gctUINT i;
                        for(i = 0; i < 4; i++)
                        {
                            mappingGCSLUniform->initializer.u32_v4[i] = initializerConst->value.vecVal.u32Value[i];
                        }
                    }
                }
            }

            if (VIR_Symbol_HasFlag(virUniformSym, VIR_SYMUNIFORMFLAG_MOVED_TO_CUBO))
            {
                VIR_Type *type = VIR_Symbol_GetType(virUniformSym);

                if(VIR_Type_isArray(type)) {
                    gcmASSERT(0);
                }
                else {
                    VIR_Const* initializer = VIR_Shader_GetConstFromId(VirShader, VIR_Uniform_GetInitializer(virUniform));

                    gcoOS_MemCopy(Shader->constUBOData + VIR_Uniform_GetOffset(virUniform) / sizeof(gctUINT32), &initializer->value, VIR_GetTypeSize(initializer->type));
                }
            }
        }
    }

    if (VIR_Shader_isRegAllocated(VirShader))
    {
        gcShaderSetHwRegAllocated(Shader);
        Shader->RARegWaterMark = VIR_Shader_GetRegWatermark(VirShader);

        /* change attributes tempindex based on register allocation info*/
        {
            gctSIZE_T   i = 0;

            for (i = 0; i < Shader->attributeCount; ++i)
            {
                gcATTRIBUTE attribute = Shader->attributes[i];
                VIR_Symbol  *sym = gcvNULL;
                VIR_NameId  nameId;

                if (attribute == gcvNULL)
                {
                    continue;
                }

                if (attribute->nameLength < 0)
                {
                    _ConvBuiltinNameKindToVirNameId(attribute->nameLength,
                                                    &nameId);
                    sym = VIR_Shader_FindSymbolById(VirShader, VIR_SYM_VARIABLE, nameId);
                }
                else
                {
                    sym = VIR_Shader_FindSymbolByName(VirShader, VIR_SYM_VARIABLE, attribute->name);
                }

                gcmASSERT(sym != gcvNULL);

                attribute->inputIndex =  VIR_Symbol_GetHwRegId(sym);

                if (isSymUnused(sym))
                {
                    gcmATTRIBUTE_SetNotUsed(attribute, gcvTRUE);

                    if (useFullNewLinker &&
                        !gcmATTRIBUTE_alwaysUsed(attribute) &&
                        !seperatedShader)
                    {
                        gcmATTRIBUTE_SetEnabled(attribute, gcvFALSE);
                    }
                }
                else
                {
                    gcmATTRIBUTE_SetAlwaysUsed(attribute, gcvTRUE);
                }
            }
        }

        if (!useFullNewLinker)
        {
            /* change outputs tempindex based on register allocation info. */
            {
                gctUINT32   i = 0, outputIndex = 0, perpatchOutputIndex = 0;
                gctUINT32 components = 0, rows = 0;

                for (i = 0; i < Shader->outputCount; ++i)
                {
                    gcOUTPUT output = Shader->outputs[i];
                    VIR_Symbol  *sym = gcvNULL;
                    VIR_NameId  nameId;

                    if (output == gcvNULL)
                    {
                        continue;
                    }

                    if (output->nameLength < 0)
                    {
                        _ConvBuiltinNameKindToVirNameId(output->nameLength,
                                                        &nameId);
                        sym = VIR_Shader_FindSymbolById(VirShader, VIR_SYM_VARIABLE, nameId);
                    }
                    else
                    {
                        sym = VIR_Shader_FindSymbolByName(VirShader, VIR_SYM_VARIABLE, output->name);
                    }

                    gcmASSERT(sym != gcvNULL);

                    gcTYPE_GetTypeInfo(output->type, &components, &rows, 0);

                    if (gcmOUTPUT_isPerPatch(output))
                    {
                        perpatchOutputIndex += rows;
                    }
                    else
                    {
                        outputIndex += rows;
                    }

                    Shader->outputs[i]->tempIndex = VIR_Symbol_GetHwRegId(sym);
                }
                gcmASSERT(outputIndex ==
                          VIR_IdList_Count(VIR_Shader_GetOutputVregs(VirShader)));
                gcmASSERT(perpatchOutputIndex ==
                          VIR_IdList_Count(VIR_Shader_GetPerpatchOutputVregs(VirShader)));
            }

            /* change variables index who are also as attributes based on register allocation info */
            {
                gctSIZE_T           i = 0, attId = 0;
                VIR_Symbol          *sym;
                VIR_StorageClass    storageClass = VIR_STORAGE_UNKNOWN;
                gctINT              kind = 0;
                VIR_AttributeIdList *pAttrs = VIR_Shader_GetAttributes(VirShader);

                for (i = 0; i < Shader->variableCount; i++)
                {
                    gcVARIABLE variable;

                    variable = Shader->variables[i];
                    if (variable == gcvNULL ||
                        variable->parent != -1) continue;

                    if (GetVariableIsOtput(variable))
                    {
                        /* the output should be already added to symbol table.
                           if it is not, then this is a dead output*/
                        continue;
                    }

                    storageClass = _gcmGetVirBuiltinStorageClass(Shader->type, variable->nameLength);
                    if (storageClass == VIR_STORAGE_INPUT)
                    {
                        for (attId = 0; attId < VIR_IdList_Count(pAttrs); attId ++)
                        {
                            sym = VIR_Shader_GetSymFromId(VirShader,
                                        VIR_IdList_GetId(pAttrs, attId));

                            _ConvVirNameIdToBuiltinNameKind(VIR_Symbol_GetName(sym), &kind);

                            if (kind == variable->nameLength)
                            {
                                variable->tempIndex = VIR_Symbol_GetHwRegId(sym);
                            }
                        }
                    }
                }
            }
        }
    }

    if (VIR_Shader_isConstRegAllocated(VirShader))
    {
        /* change uniform physical/swizzle/address based on VIR_CG_MapUniform information */
        gctSIZE_T   i;
        VIR_Symbol  *sym;
        VIR_UniformIdList     *pUniforms = VIR_Shader_GetUniforms(VirShader);

        gcShaderSetConstHwRegAllocated(Shader);

        if (gcmOPT_DUMP_UNIFORM())
        {
            gcoOS_Print("VIR Uniform Mapping(id:%d): \n", VirShader->_id);
        }

        for (i = 0; i < VIR_IdList_Count(pUniforms); ++i)
        {
            VIR_Uniform *symUniform = gcvNULL;
            /*VIR_Type    *pSymType = gcvNULL;*/
            gcUNIFORM    gcslUniform;

            sym = VIR_Shader_GetSymFromId(VirShader,
                                VIR_IdList_GetId(pUniforms, i));

            /*pSymType = VIR_Symbol_GetType(sym);*/
            symUniform = VIR_Symbol_GetUniformPointer(VirShader, sym);

            if (symUniform == gcvNULL)
            {
                continue;
            }

            if (symUniform->gcslIndex == -1)
            {
                continue;
            }

            gcslUniform = Shader->uniforms[symUniform->gcslIndex];
            gcslUniform->physical = symUniform->physical;
            gcslUniform->samplerPhysical = symUniform->samplerPhysical;
            gcslUniform->swizzle = symUniform->swizzle;
            gcslUniform->address = symUniform->address;
            gcslUniform->baseBindingIdx = -1;
            SetUniformUsedArraySize(gcslUniform, VIR_Uniform_GetRealUseArraySize(symUniform));

            if (isSymInactive(sym))
            {
                /* Don't change the usage of user-defined UBO when doing recompiler. */
                if (!(Flags & gcvSHADER_RECOMPILER &&
                      !isUniformLoadtimeConstant(gcslUniform) &&
                      !isUniformCompiletimeInitialized(gcslUniform) &&
                      !isUniformCompilerGen(gcslUniform)))
                {
                    SetUniformFlag(gcslUniform, gcvUNIFORM_FLAG_IS_INACTIVE);
                }
            }

            SetUniformShaderKind(gcslUniform, Shader->type);

            if (symUniform->baseBindingUniform != VIR_INVALID_ID)
            {
                gctSIZE_T j = 0;

                for (j = 0; j <= i; ++j)
                {
                    VIR_Symbol *baseSym = VIR_Shader_GetSymFromId(VirShader,
                        VIR_IdList_GetId(pUniforms, j));
                    if (symUniform->baseBindingUniform == VIR_Symbol_GetIndex(baseSym))
                    {
                        gcslUniform->baseBindingIdx = (gctINT16)j;
                        break;
                    }
                }
            }

            if (VIR_Symbol_isUniform(sym))
            {
                gcslUniform->location = VIR_Symbol_GetLocation(sym);
            }
            else if (VIR_Symbol_isSampler(sym) || VIR_Symbol_isImage(sym))
            {
                gcslUniform->binding = VIR_Symbol_GetBinding(sym);
            }

            if (gcmOPT_DUMP_UNIFORM())
            {
                gctCHAR   buffer[512];
                gctUINT   offset      = 0;

                gcoOS_PrintStrSafe(buffer, gcmSIZEOF(buffer), &offset,
                                    "uniform %d name %s physical %d swizzle %d address %d\n",
                                    symUniform->index,
                                    gcslUniform->name,
                                    gcslUniform->physical,
                                    gcslUniform->swizzle,
                                    gcslUniform->address);
                gcoOS_Print("%s", buffer);
                offset = 0;
            }
        }

        if (gcmOPT_DUMP_UNIFORM())
        {
            gcoOS_Print("\n");
        }
    }

    if (!useFullNewLinker)
    {
        /* Sort the function list first. */
        _MarkFunctionFlag(VirShader);
        _SortFunctions(VirShader);

        if(VIR_Shader_isRegAllocated(VirShader))
        {
            Shader->_tempRegCount   = VIR_Shader_GetRegWatermark(VirShader);
        }
        else
        {
            Shader->_tempRegCount   = VirShader->_tempRegCount;
        }

        Shader->lastInstruction     = 0;
        Shader->instrIndex          = gcSHADER_OPCODE;

        /* Free any labels. */
        while (Shader->labels != gcvNULL)
        {
            gcSHADER_LABEL label = Shader->labels;
            Shader->labels = label->next;

            while (label->referenced != gcvNULL)
            {
                /* Remove gcSHADER_LINK structure from head of list. */
                gcSHADER_LINK link = label->referenced;
                label->referenced = link->next;

                /* Free the gcSHADER_LINK structure. */
                gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, link));
            }

            /* Free the gcSHADER_LABEL structure. */
            gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, label));
        }

        /* Build labels */
        {
            gctUINT           instCount = 0;
            VIR_FunctionNode *funcNode;
            VIR_FuncIterator  funcIter;

            VIR_FuncIterator_Init(&funcIter, &converter.VirShader->functions);
            funcNode =  VIR_FuncIterator_First(&funcIter);
            for (; funcNode != gcvNULL; funcNode =  VIR_FuncIterator_Next(&funcIter))
            {
                if (funcNode->function != gcvNULL)
                {
                    _BuildLabelByFunction(&converter, funcNode->function, &instCount);
                }
            }
        }

        /* Free the code buffer. */
        if (Shader->code != gcvNULL)
        {
            gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, Shader->code));
        }
        Shader->codeCount = 0;

        /* Free any functions. */
        if (Shader->functions != gcvNULL)
        {
            gctUINT i         = 0;
            for (i = 0; i < Shader->functionCount; i++)
            {
                if (Shader->functions[i] != gcvNULL)
                {
                    if (Shader->functions[i]->arguments != gcvNULL)
                    {
                        /* Free the gcSHADER_FUNCTION_ARGUMENT structure. */
                        gcmVERIFY_OK(
                            gcmOS_SAFE_FREE(gcvNULL, Shader->functions[i]->arguments));
                    }

                    /* Free the gcSHADER_FUNCTION structure. */
                    gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, Shader->functions[i]));
                }
            }

            /* Free the array of gcSHADER_FUNCTION pointers. */
            gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, Shader->functions));
            Shader->functionArraySize = 0;
            Shader->functionCount = 0;
            Shader->currentFunction = gcvNULL;
        }

        /* Free any kernel functions. */
        if (Shader->kernelFunctions != gcvNULL)
        {
            gctUINT             i = 0;
            gctPOINTER          pointer = gcvNULL;
            gctUINT32           length = Shader->currentKernelFunction->nameLength;

            /* Allocate a new kernel function list. */
            gcmONERROR(gcoOS_Allocate(gcvNULL,
                                      gcmSIZEOF(gcKERNEL_FUNCTION),
                                      &pointer));
            gcoOS_ZeroMemory(pointer, gcmSIZEOF(gcKERNEL_FUNCTION));
            kernelFunctions = (gcKERNEL_FUNCTION*)pointer;

            /* Copy the current kernel function. */
            gcmONERROR(gcoOS_Allocate(gcvNULL,
                                      gcmOFFSETOF(_gcsKERNEL_FUNCTION, name) + length + 1,
                                      &pointer));
            gcoOS_ZeroMemory(pointer, gcmOFFSETOF(_gcsKERNEL_FUNCTION, name) + length + 1);
            kernelFunctions[0] = (gcKERNEL_FUNCTION)pointer;

            gcoOS_MemCopy(kernelFunctions[0], Shader->currentKernelFunction, gcmSIZEOF(struct _gcsKERNEL_FUNCTION));
            gcoOS_MemCopy(kernelFunctions[0]->name, Shader->currentKernelFunction->name, length + 1);

            for (i = 0; i < Shader->kernelFunctionCount; i++)
            {
                if (Shader->kernelFunctions[i] != gcvNULL)
                {
                    if (Shader->kernelFunctions[i]->arguments != gcvNULL &&
                        Shader->kernelFunctions[i] != Shader->currentKernelFunction)
                    {
                        /* Free the gcSHADER_FUNCTION_ARGUMENT structure. */
                        gcmVERIFY_OK(
                            gcmOS_SAFE_FREE(gcvNULL, Shader->kernelFunctions[i]->arguments));
                    }

                    /* Free the gcSHADER_FUNCTION structure. */
                    gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, Shader->kernelFunctions[i]));
                }
            }

            /* Free the array of gcSHADER_FUNCTION pointers. */
            gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, Shader->kernelFunctions));

            /* Reset kernel functionns */
            Shader->kernelFunctionArraySize = 1;
            Shader->kernelFunctionCount = 1;
            Shader->kernelFunctions = kernelFunctions;
            Shader->currentKernelFunction = kernelFunctions[0];
            kernelFunctions = gcvNULL;
        }

        if(converter.VirShader->functions.info.count > 0)
        {
            VIR_FunctionNode *funcNode;
            VIR_FuncIterator  funcIter;

            VIR_FuncIterator_Init(&funcIter, &converter.VirShader->functions);
            funcNode = VIR_FuncIterator_First(&funcIter);

            for (;funcNode != gcvNULL; funcNode = VIR_FuncIterator_Next(&funcIter))
            {
                gctSTRING     name;
                VIR_Symbol   *funcSym;
                VIR_Function *virFunc;
                gcFUNCTION    func = gcvNULL;

                virFunc = funcNode->function;

                if(virFunc != VirShader->mainFunction)
                {
                    funcSym = VIR_Function_GetSymFromId(virFunc, virFunc->funcSym);
                    name    = VIR_Shader_GetSymNameString(VirShader, funcSym);

                    gcSHADER_AddFunction(Shader, name, &func);

                    /* Save the gcSL label. */
                    _AddValue(converter.FuncTable, virFunc, GetFunctionLable(func));

                    _ConvVirFunction2Function(&converter, func, virFunc);
                }
                else
                {
                    if (Shader->currentKernelFunction)
                    {
                        Shader->currentKernelFunction->codeStart = Shader->lastInstruction;
                    }

                    _ConvVirFunction2Function(&converter, gcvNULL, VirShader->mainFunction);

                    if (Shader->currentKernelFunction)
                    {
                        Shader->currentKernelFunction->codeEnd = Shader->lastInstruction + 1;
                        Shader->currentKernelFunction->codeCount =
                            Shader->currentKernelFunction->codeEnd - Shader->currentKernelFunction->codeStart;
                    }
                }
            }
        }
    }

    /* Pack the shader to update the label. */
    gcSHADER_Pack(Shader);

    if (gcSHADER_DumpCodeGenVerbose(Shader))
    {
        gcDump_Shader(gcvNULL, "Converted gcSL shader IR (from VIR).", gcvNULL, Shader, gcvTRUE);
    }

OnError:
    if (kernelFunctions != gcvNULL)
    {
        gcmOS_SAFE_FREE(gcvNULL, kernelFunctions);
    }
    _FinalizeConverter(&converter);

    gcmFOOTER_ARG("Shader=0x%x", Shader);
    return status;
}

#endif


