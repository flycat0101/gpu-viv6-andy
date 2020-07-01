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


#include "gc_cl_emit_code.h"

gceSTATUS
cloCODE_EMITTER_NewBasicBlock(
IN cloCOMPILER Compiler,
IN cloCODE_EMITTER CodeEmitter
);

gceSTATUS
cloCODE_EMITTER_EndBasicBlock(
IN cloCOMPILER Compiler,
IN cloCODE_EMITTER CodeEmitter
);

gceSTATUS
cloCODE_EMITTER_EmitCode1(
IN cloCOMPILER Compiler,
IN cloCODE_EMITTER CodeEmitter,
IN gctUINT LineNo,
IN gctUINT StringNo,
IN cleOPCODE Opcode,
IN gcsTARGET * Target,
IN gcsSOURCE * Source
);

gceSTATUS
cloCODE_EMITTER_EmitCode2(
IN cloCOMPILER Compiler,
IN cloCODE_EMITTER CodeEmitter,
IN gctUINT LineNo,
IN gctUINT StringNo,
IN cleOPCODE Opcode,
IN gcsTARGET * Target,
IN gcsSOURCE * Source0,
IN gcsSOURCE * Source1
);

gctSIZE_T
gcGetDataTypeRegSize(
IN clsGEN_CODE_DATA_TYPE DataType
)
{
    gctUINT8 size;

    size = clmGEN_CODE_matrixColumnCount_GET(DataType);
    if(size == 0) size = 1;
    if(clmIsElementTypePacked(DataType.elementType)) {
        switch(DataType.elementType) {
        case clvTYPE_BOOL_PACKED:
        case clvTYPE_CHAR_PACKED:
        case clvTYPE_UCHAR_PACKED:
            if(clmGEN_CODE_matrixRowCount_GET(DataType) == 32) {
                size <<= 1;
            }
            break;

        case clvTYPE_SHORT_PACKED:
        case clvTYPE_USHORT_PACKED:
        case clvTYPE_HALF_PACKED:
            switch(clmGEN_CODE_matrixRowCount_GET(DataType)) {
            case 16:
                size <<= 1;
                break;

            case 32:
                size <<= 2;
                break;
            }
            break;

        case clvTYPE_GEN_PACKED:
            size <<= 2;
            break;

        default:
            break;
        }
    }
    else {
        switch(clmGEN_CODE_matrixRowCount_GET(DataType)) {
        case 8:
            size <<= 1;
            break;

        case 16:
            size <<= 2;
            break;
        }
    }

    return size;
}

gctUINT8
gcGetDataTypeComponentCount(
IN clsGEN_CODE_DATA_TYPE DataType
)
{
      gctUINT8 rowCount;
      gctUINT8 columnCount;

      rowCount = clmGEN_CODE_matrixRowCount_GET(DataType);
      if(rowCount) {
         columnCount = clmGEN_CODE_matrixColumnCount_GET(DataType);
         if(columnCount) return rowCount * columnCount;
         else return rowCount;
      }
      else return 1;
}

/* Machine targeted data type component count - high precision will be doubled */
gctUINT8
gcGetDataTypeTargetComponentCount(
IN clsGEN_CODE_DATA_TYPE DataType
)
{
    gctUINT8 rowCount;
    gctUINT8 columnCount;
    gctUINT8 componentCount;

    rowCount = clmGEN_CODE_matrixRowCount_GET(DataType);
    if(rowCount) {
        columnCount = clmGEN_CODE_matrixColumnCount_GET(DataType);
        if(columnCount) componentCount = rowCount * columnCount;
        else componentCount = rowCount;
    }
    else componentCount = 1;

    return componentCount;
}


gctSIZE_T
gcGetAddressableUnitSize(
IN clsGEN_CODE_DATA_TYPE DataType
)
{
    gctUINT componentCount;
    gctUINT byteCount = 0;

    componentCount = gcGetDataTypeComponentCount(DataType);

    switch(clmGEN_CODE_elementType_GET(DataType)) {
    case clvTYPE_CHAR:
    case clvTYPE_UCHAR:
    case clvTYPE_CHAR_PACKED:
    case clvTYPE_UCHAR_PACKED:
       byteCount = 1;
       break;

    case clvTYPE_HALF:
    case clvTYPE_SHORT:
    case clvTYPE_USHORT:
    case clvTYPE_HALF_PACKED:
    case clvTYPE_SHORT_PACKED:
    case clvTYPE_USHORT_PACKED:
       byteCount = cldMachineBytesPerWord >> 1;
       break;

    case clvTYPE_LONG:
    case clvTYPE_ULONG:
    case clvTYPE_DOUBLE:
       byteCount = cldMachineBytesPerWord << 1;
       break;

    default:
       byteCount = cldMachineBytesPerWord;
    }

    return componentCount * byteCount;
}

clsGEN_CODE_DATA_TYPE
gcGetComponentDataType(
IN clsGEN_CODE_DATA_TYPE DataType
)
{
    clmGEN_CODE_vectorSize_SET(DataType, 0);
    return DataType;
}

gctBOOL
gcIsElementTypeEqual(
IN clsGEN_CODE_DATA_TYPE DataType1,
IN clsGEN_CODE_DATA_TYPE DataType2
)
{
   cltELEMENT_TYPE type1, type2;

   type1 = clmGEN_CODE_elementType_GET(DataType1);
   type2 = clmGEN_CODE_elementType_GET(DataType2);

   return (clmIsElementTypeFloating(type1) && clmIsElementTypeFloating(type2)) ||
          (clmIsElementTypeBoolean(type1) && clmIsElementTypeBoolean(type2)) ||
          (clmIsElementTypeInteger(type1) && clmIsElementTypeInteger(type2)) ||
      (clmIsElementTypeSampler(type1) && clmIsElementTypeSampler(type2)) ||
      (clmIsElementTypePacked(type1) && clmIsElementTypePacked(type2)) ||
      (clmIsElementTypeEvent(type1) && clmIsElementTypeEvent(type2));
}

gctBOOL
gcIsDataTypeEqual(
IN clsGEN_CODE_DATA_TYPE DataType1,
IN clsGEN_CODE_DATA_TYPE DataType2
)
{
   if((clmIsElementTypePackedGenType(DataType1.elementType) &&
       clmIsElementTypePacked(DataType2.elementType)) ||
      (clmIsElementTypePackedGenType(DataType2.elementType) &&
       clmIsElementTypePacked(DataType1.elementType))) return gcvTRUE;
   return gcIsElementTypeEqual(DataType1, DataType2)
          && (clmGEN_CODE_matrixRowCount_GET(DataType1) == clmGEN_CODE_matrixRowCount_GET(DataType2))
          && (clmGEN_CODE_matrixColumnCount_GET(DataType1) == clmGEN_CODE_matrixColumnCount_GET(DataType2));
}

static gctCONST_STRING
_GetOpcodeName(
IN gcSL_OPCODE Opcode
)
{
    switch (Opcode) {
    case gcSL_NOP:      return "gcSL_NOP";
    case gcSL_CMP:      return "gcSL_CMP";
    case gcSL_MOV:      return "gcSL_MOV";
    case gcSL_CONV:     return "gcSL_CONV";
    case gcSL_SAT:      return "gcSL_SAT";
    case gcSL_DP2:      return "gcSL_DP2";
    case gcSL_DP3:      return "gcSL_DP3";
    case gcSL_DP4:      return "gcSL_DP4";
    case gcSL_ABS:      return "gcSL_ABS";
    case gcSL_JMP:      return "gcSL_JMP";
    case gcSL_ADD:      return "gcSL_ADD";
    case gcSL_MUL:      return "gcSL_MUL";
    case gcSL_MUL_Z:    return "gcSL_MUL_Z";
    case gcSL_MOD:      return "gcSL_MOD";
    case gcSL_DIV:      return "gcSL_DIV";
    case gcSL_RSHIFT:   return "gcSL_RSHIFT";
    case gcSL_LSHIFT:   return "gcSL_LSHIFT";
    case gcSL_NOT_BITWISE:   return "gcSL_NOT_BITWISE";
    case gcSL_AND_BITWISE:   return "gcSL_AND_BITWISE";
    case gcSL_OR_BITWISE:    return "gcSL_OR_BITWISE";
    case gcSL_XOR_BITWISE:   return "gcSL_XOR_BITWISE";
    case gcSL_LOAD:     return "gcSL_LOAD";
    case gcSL_STORE:    return "gcSL_STORE";
#if cldUseSTORE1
    case gcSL_STORE1:   return "gcSL_STORE1";
#endif
    case gcSL_BARRIER:  return "gcSL_BARRIER";
    case gcSL_MEM_BARRIER:  return "gcSL_MEM_BARRIER";
    case gcSL_RCP:      return "gcSL_RCP";
    case gcSL_SUB:      return "gcSL_SUB";
    case gcSL_NEG:      return "gcSL_NEG";
    case gcSL_KILL:     return "gcSL_KILL";
    case gcSL_TEXLD:    return "gcSL_TEXLD";
    case gcSL_IMAGE_WR: return "gcSL_IMAGE_WR";
    case gcSL_IMAGE_WR_3D: return "gcSL_IMAGE_WR_3D";
    case gcSL_IMAGE_RD: return "gcSL_IMAGE_RD";
    case gcSL_IMAGE_RD_3D: return "gcSL_IMAGE_RD_3D";
    case gcSL_IMAGE_SAMPLER: return "gcSL_IMAGE_SAMPLER";
    case gcSL_CALL:     return "gcSL_CALL";
    case gcSL_RET:      return "gcSL_RET";
    case gcSL_NORM:     return "gcSL_NORM";
    case gcSL_MAX:      return "gcSL_MAX";
    case gcSL_MIN:      return "gcSL_MIN";
    case gcSL_POW:      return "gcSL_POW";
    case gcSL_RSQ:      return "gcSL_RSQ";
    case gcSL_LOG:      return "gcSL_LOG";
    case gcSL_FRAC:     return "gcSL_FRAC";
    case gcSL_FLOOR:    return "gcSL_FLOOR";
    case gcSL_CEIL:     return "gcSL_CEIL";
    case gcSL_CROSS:    return "gcSL_CROSS";
    case gcSL_TEXLDPROJ:return "gcSL_TEXLDPROJ";
    case gcSL_TEXBIAS:  return "gcSL_TEXBIAS";
    case gcSL_TEXGRAD:    return "gcSL_TEXGRAD";
    case gcSL_TEXLOD:    return "gcSL_TEXLOD";
    case gcSL_SIN:        return "gcSL_SIN";
    case gcSL_COS:        return "gcSL_COS";
    case gcSL_TAN:        return "gcSL_TAN";
    case gcSL_EXP:        return "gcSL_EXP";
    case gcSL_SIGN:        return "gcSL_SIGN";
    case gcSL_STEP:        return "gcSL_STEP";
    case gcSL_SQRT:        return "gcSL_SQRT";
    case gcSL_ACOS:        return "gcSL_ACOS";
    case gcSL_ASIN:        return "gcSL_ASIN";
    case gcSL_ATAN:        return "gcSL_ATAN";
    case gcSL_SET:        return "gcSL_SET";
    case gcSL_DSX:        return "gcSL_DSX";
    case gcSL_DSY:        return "gcSL_DSY";
    case gcSL_FWIDTH:    return "gcSL_FWIDTH";

    case gcSL_MULLO:    return "gcSL_MULLO";
    case gcSL_ADDLO:    return "gcSL_ADDLO";
    case gcSL_MULHI:    return "gcSL_MULHI";
    case gcSL_I2F:        return "gcSL_I2F";
    case gcSL_F2I:        return "gcSL_F2I";

    case gcSL_ROTATE:    return "gcSL_ROTATE";
    case gcSL_LEADZERO: return "gcSL_LEADZERO";
    case gcSL_GETEXP:    return "gcSL_GETEXP";
    case gcSL_GETMANT:    return "gcSL_GETMANT";
    case gcSL_ADDSAT:    return "gcSL_ADDSAT";
    case gcSL_SUBSAT:    return "gcSL_SUBSAT";
    case gcSL_MULSAT:    return "gcSL_MULSAT";
    case gcSL_MADSAT:    return "gcSL_MADSAT";

    case gcSL_CTZ:       return "gcSL_CTZ";

    case gcSL_ATOMADD:    return "gcSL_ATOMADD";
    case gcSL_ATOMSUB:    return "gcSL_ATOMSUB";
    case gcSL_ATOMXCHG:    return "gcSL_ATOMXCHG";
    case gcSL_ATOMCMPXCHG:    return "gcSL_ATOMCMPXCHG";
    case gcSL_ATOMMIN:    return "gcSL_ATOMMIN";
    case gcSL_ATOMMAX:    return "gcSL_ATOMMAX";
    case gcSL_ATOMOR:    return "gcSL_ATOMOR";
    case gcSL_ATOMAND:    return "gcSL_ATOMAND";
    case gcSL_ATOMXOR:    return "gcSL_ATOMXOR";

    case gcSL_UNPACK:     return "gcSL_UNPACK";

    case gcSL_TEXLDPCF: return "gcSL_TEXLDPCF";
    case gcSL_TEXLDPCFPROJ: return "gcSL_TEXLDPCFPROJ";

    case gcSL_SINPI:    return "gcSL_SINPI";
    case gcSL_COSPI:    return "gcSL_COSPI";
    case gcSL_TANPI:    return "gcSL_TANPI";

    case gcSL_POPCOUNT: return "gcSL_POPCOUNT";
    case gcSL_TEXU:     return "gcSL_TEXU";
    case gcSL_ARCTRIG0:      return "gcSL_ARCTRIG0";
    case gcSL_ARCTRIG1:      return "gcSL_ARCTRIG1";

    case gcSL_LONGLO:        return "gcSL_LONGLO";
    case gcSL_LONGHI:        return "gcSL_LONGHI";
    case gcSL_MOV_LONG:      return "gcSL_MOV_LONG";
    case gcSL_COPY:          return "gcSL_COPY";
    case gcSL_TEXLD_U:       return "gcSL_TEXLD_U";
    case gcSL_PARAM_CHAIN:   return "gcSL_PARAM_CHAIN";
    case gcSL_INTRINSIC:     return "gcSL_INTRINSIC";
    case gcSL_INTRINSIC_ST:  return "gcSL_INTRINSIC_ST";
    case gcSL_CLAMP0MAX:     return "gcSL_CLAMP0MAX";
    case gcSL_CLAMPCOORD:    return "gcSL_CLAMPCOORD";
    case gcSL_FMA_MUL:       return "gcSL_FMA_MUL";
    case gcSL_FMA_ADD:       return "gcSL_FMA_ADD";

    case gcSL_CMAD:         return "gcSL_CMAD";
    case gcSL_CONJ:         return "gcSL_CONJ";
    case gcSL_CMUL:         return "gcSL_CMUL";
    case gcSL_CMADCJ:       return "gcSL_CMADCJ";
    case gcSL_CMULCJ:       return "gcSL_CMULCJ";
    case gcSL_CADDCJ:       return "gcSL_CADDCJ";
    case gcSL_CSUBCJ:       return "gcSL_CSUBCJ";

    case gcSL_GET_IMAGE_TYPE: return "gcSL_GET_IMAGE_TYPE";

    case gcSL_FINDLSB:      return "gcSL_FINDLSB";
    case gcSL_FINDMSB:      return "gcSL_FINDMSB";
    case gcSL_BIT_REVERSAL: return "gcSL_BIT_REVERSAL";
    case gcSL_BYTE_REVERSAL:  return "gcSL_BYTE_REVERSAL";
    default:
    gcmASSERT(0);
    return "Invalid";
    }
}

static gctCONST_STRING
_GetConditionName(
IN gcSL_CONDITION Condition
)
{
    switch (Condition) {
    case gcSL_ALWAYS:           return "gcSL_ALWAYS";
    case gcSL_NOT_EQUAL:        return "gcSL_NOT_EQUAL";
    case gcSL_LESS_OR_EQUAL:    return "gcSL_LESS_OR_EQUAL";
    case gcSL_LESS:             return "gcSL_LESS";
    case gcSL_EQUAL:            return "gcSL_EQUAL";
    case gcSL_GREATER:          return "gcSL_GREATER";
    case gcSL_GREATER_OR_EQUAL: return "gcSL_GREATER_OR_EQUAL";
    case gcSL_AND:              return "gcSL_AND";
    case gcSL_OR:               return "gcSL_OR";
    case gcSL_XOR:              return "gcSL_XOR";
    case gcSL_NOT_ZERO:         return "gcSL_NOT_ZERO";
    case gcSL_ZERO:             return "gcSL_ZERO";

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

    buf[i] = '\0';

    return buf;
}

static gctCHAR
_GetSwizzleChar(
IN gctUINT8 Swizzle
)
{
    switch (Swizzle) {
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

static gctCONST_STRING
_GetIndexModeName(
IN gcSL_INDEXED IndexMode
)
{
    switch (IndexMode) {
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

static gcSL_INDEXED
_ConvSwizzleToIndexMode(
IN gcSL_SWIZZLE Swizzle
)
{
    switch(Swizzle) {
    case gcSL_SWIZZLE_X: return gcSL_INDEXED_X;
    case gcSL_SWIZZLE_Y: return gcSL_INDEXED_Y;
    case gcSL_SWIZZLE_Z: return gcSL_INDEXED_Z;
    case gcSL_SWIZZLE_W: return gcSL_INDEXED_W;

    default:
        gcmASSERT(0);
        return gcSL_INDEXED_X;
    }
}

static gctCONST_STRING
_GetTypeName(
IN gcSL_TYPE Type
)
{
    switch (Type) {
    case gcSL_NONE:        return "gcSL_NONE";
    case gcSL_TEMP:        return "gcSL_TEMP";
    case gcSL_ATTRIBUTE:    return "gcSL_ATTRIBUTE";
    case gcSL_UNIFORM:        return "gcSL_UNIFORM";
    case gcSL_SAMPLER:        return "gcSL_SAMPLER";
    case gcSL_CONSTANT:        return "gcSL_CONSTANT";

    default:
    gcmASSERT(0);
    return "Invalid";
    }
}

gcsTYPE_SIZE
clConvToShaderDataType(
cloCOMPILER Compiler,
clsGEN_CODE_DATA_TYPE DataType
)
{
    gcsTYPE_SIZE typeSize;

    typeSize.type = gcSHADER_FLOAT_X4;
    typeSize.length = 1;
    switch (clmGEN_CODE_elementType_GET(DataType)) {
    case clvTYPE_BOOL:
        switch (clmGEN_CODE_vectorSize_GET(DataType))
        {
        case 0:
                   typeSize.type = gcSHADER_BOOLEAN_X1;
                   break;

        case 2:
                   typeSize.type = gcSHADER_BOOLEAN_X2;
                   break;

        case 3:
                   typeSize.type = gcSHADER_BOOLEAN_X3;
                   break;

        case 4:
                   typeSize.type = gcSHADER_BOOLEAN_X4;
                   break;

        case 8:

                   if(cloCOMPILER_ExtensionEnabled(Compiler, clvEXTENSION_VIV_VX)) {
                       typeSize.type = gcSHADER_BOOLEAN_X8;
                   }
                   else {
                       typeSize.type = gcSHADER_BOOLEAN_X4;
                       typeSize.length = 2;
                   }
                   break;

        case 16:
                   if(cloCOMPILER_ExtensionEnabled(Compiler, clvEXTENSION_VIV_VX)) {
                       typeSize.type = gcSHADER_BOOLEAN_X16;
                   }
                   else {
                       typeSize.type = gcSHADER_BOOLEAN_X4;
                       typeSize.length = 4;
                   }
                   break;

        default:
           gcmASSERT(0);
        }
        break;

    case clvTYPE_BOOL_PACKED:
        switch (clmGEN_CODE_vectorSize_GET(DataType))
        {
        case 0:
                   typeSize.type = gcSHADER_BOOLEAN_X1;
                   break;

        case 2:
                   typeSize.type = gcSHADER_BOOLEAN_P2;
                   break;

        case 3:
                   typeSize.type = gcSHADER_BOOLEAN_P3;
                   break;

        case 4:
                   typeSize.type = gcSHADER_BOOLEAN_P4;
                   break;

        case 8:
                   typeSize.type = gcSHADER_BOOLEAN_P8;
                   break;

        case 16:
                   typeSize.type = gcSHADER_BOOLEAN_P16;
                   break;

        case 32:
                   typeSize.type = gcSHADER_BOOLEAN_P32;
                   break;

        default:
           gcmASSERT(0);
        }
        break;

    case clvTYPE_LONG:
        switch (clmGEN_CODE_vectorSize_GET(DataType))
        {
        case 0:
                   typeSize.type = gcSHADER_INT64_X1;
                   break;

        case 2:
                   typeSize.type = gcSHADER_INT64_X2;
                   break;

        case 3:
                   typeSize.type = gcSHADER_INT64_X3;
                   break;

        case 4:
                   typeSize.type = gcSHADER_INT64_X4;
                   break;

        case 8:
                   typeSize.type = gcSHADER_INT64_X4;
                   typeSize.length = 2;
                   break;

        case 16:
                   typeSize .type = gcSHADER_INT64_X4;
                   typeSize.length = 4;
                   break;

        default:
           gcmASSERT(0);
        }
        break;

    case clvTYPE_INT:
        switch (clmGEN_CODE_vectorSize_GET(DataType))
        {
        case 0:
                   typeSize.type = gcSHADER_INTEGER_X1;
                   break;

        case 2:
                   typeSize.type = gcSHADER_INTEGER_X2;
                   break;

        case 3:
                   typeSize.type = gcSHADER_INTEGER_X3;
                   break;

        case 4:
                   typeSize.type = gcSHADER_INTEGER_X4;
                   break;

        case 8:
                   if(cloCOMPILER_ExtensionEnabled(Compiler, clvEXTENSION_VIV_VX)) {
                       typeSize.type = gcSHADER_INTEGER_X8;
                   }
                   else {
                       typeSize.type = gcSHADER_INTEGER_X4;
                       typeSize.length = 2;
                   }
                   break;

        case 16:
                   if(cloCOMPILER_ExtensionEnabled(Compiler, clvEXTENSION_VIV_VX)) {
                       typeSize.type = gcSHADER_INTEGER_X16;
                   }
                   else {
                       typeSize .type = gcSHADER_INTEGER_X4;
                       typeSize.length = 4;
                   }
                   break;

        default:
           gcmASSERT(0);
        }
        break;

    case clvTYPE_SHORT:
        switch (clmGEN_CODE_vectorSize_GET(DataType))
        {
        case 0:
                   typeSize.type = gcSHADER_INT16_X1;
                   break;

        case 2:
                   typeSize.type = gcSHADER_INT16_X2;
                   break;

        case 3:
                   typeSize.type = gcSHADER_INT16_X3;
                   break;

        case 4:
                   typeSize.type = gcSHADER_INT16_X4;
                   break;

        case 8:
                   if(cloCOMPILER_ExtensionEnabled(Compiler, clvEXTENSION_VIV_VX)) {
                       typeSize.type = gcSHADER_INT16_X8;
                   }
                   else {
                       typeSize.type = gcSHADER_INT16_X4;
                       typeSize.length = 2;
                   }
                   break;

        case 16:
                   if(cloCOMPILER_ExtensionEnabled(Compiler, clvEXTENSION_VIV_VX)) {
                       typeSize.type = gcSHADER_INT16_X16;
                   }
                   else {
                       typeSize .type = gcSHADER_INT16_X4;
                       typeSize.length = 4;
                   }
                   break;

        default:
           gcmASSERT(0);
        }
        break;

    case clvTYPE_CHAR:
        switch (clmGEN_CODE_vectorSize_GET(DataType))
        {
        case 0:
                   typeSize.type = gcSHADER_INT8_X1;
                   break;

        case 2:
                   typeSize.type = gcSHADER_INT8_X2;
                   break;

        case 3:
                   typeSize.type = gcSHADER_INT8_X3;
                   break;

        case 4:
                   typeSize.type = gcSHADER_INT8_X4;
                   break;

        case 8:
                   if(cloCOMPILER_ExtensionEnabled(Compiler, clvEXTENSION_VIV_VX)) {
                       typeSize.type = gcSHADER_INT8_X8;
                   }
                   else {
                       typeSize.type = gcSHADER_INT8_X4;
                       typeSize.length = 2;
                   }
                   break;

        case 16:
                   if(cloCOMPILER_ExtensionEnabled(Compiler, clvEXTENSION_VIV_VX)) {
                       typeSize.type = gcSHADER_INT8_X16;
                   }
                   else {
                       typeSize .type = gcSHADER_INT8_X4;
                       typeSize.length = 4;
                   }
                   break;

        default:
           gcmASSERT(0);
        }
        break;

    case clvTYPE_CHAR_PACKED:
        switch (clmGEN_CODE_vectorSize_GET(DataType))
        {
        case 0:
                   typeSize.type = gcSHADER_INTEGER_X1;
                   break;

        case 2:
                   typeSize.type = gcSHADER_INT8_P2;
                   break;

        case 3:
                   typeSize.type = gcSHADER_INT8_P3;
                   break;

        case 4:
                   typeSize.type = gcSHADER_INT8_P4;
                   break;

        case 8:
                   typeSize.type = gcSHADER_INT8_P8;
                   break;

        case 16:
                   typeSize .type = gcSHADER_INT8_P16;
                   break;

        case 32:
                   typeSize .type = gcSHADER_INT8_P32;
                   break;

        default:
           gcmASSERT(0);
        }
        break;

    case clvTYPE_UCHAR_PACKED:
        switch (clmGEN_CODE_vectorSize_GET(DataType))
        {
        case 0:
                   typeSize.type = gcSHADER_UINT_X1;
                   break;

        case 2:
                   typeSize.type = gcSHADER_UINT8_P2;
                   break;

        case 3:
                   typeSize.type = gcSHADER_UINT8_P3;
                   break;

        case 4:
                   typeSize.type = gcSHADER_UINT8_P4;
                   break;

        case 8:
                   typeSize.type = gcSHADER_UINT8_P8;
                   break;

        case 16:
                   typeSize .type = gcSHADER_UINT8_P16;
                   break;

        case 32:
                   typeSize .type = gcSHADER_UINT8_P32;
                   break;

        default:
           gcmASSERT(0);
        }
        break;

    case clvTYPE_HALF_PACKED:
        switch (clmGEN_CODE_vectorSize_GET(DataType))
        {
        case 0:
                   typeSize.type = gcSHADER_FLOAT_X1;
                   break;

        case 2:
                   typeSize.type = gcSHADER_FLOAT16_P2;
                   break;

        case 3:
                   typeSize.type = gcSHADER_FLOAT16_P3;
                   break;

        case 4:
                   typeSize.type = gcSHADER_FLOAT16_P4;
                   break;

        case 8:
                   typeSize.type = gcSHADER_FLOAT16_P8;
                   break;

        case 16:
                   typeSize .type = gcSHADER_FLOAT16_P16;
                   break;

        case 32:
                   typeSize .type = gcSHADER_FLOAT16_P32;
                   break;

        default:
           gcmASSERT(0);
        }
        break;

    case clvTYPE_SHORT_PACKED:
        switch (clmGEN_CODE_vectorSize_GET(DataType))
        {
        case 0:
                   typeSize.type = gcSHADER_INTEGER_X1;
                   break;

        case 2:
                   typeSize.type = gcSHADER_INT16_P2;
                   break;

        case 3:
                   typeSize.type = gcSHADER_INT16_P3;
                   break;

        case 4:
                   typeSize.type = gcSHADER_INT16_P4;
                   break;

        case 8:
                   typeSize.type = gcSHADER_INT16_P8;
                   break;

        case 16:
                   typeSize .type = gcSHADER_INT16_P16;
                   break;

        case 32:
                   typeSize .type = gcSHADER_INT16_P32;
                   break;

        default:
           gcmASSERT(0);
        }
        break;

    case clvTYPE_USHORT_PACKED:
        switch (clmGEN_CODE_vectorSize_GET(DataType))
        {
        case 0:
                   typeSize.type = gcSHADER_UINT_X1;
                   break;

        case 2:
                   typeSize.type = gcSHADER_UINT16_P2;
                   break;

        case 3:
                   typeSize.type = gcSHADER_UINT16_P3;
                   break;

        case 4:
                   typeSize.type = gcSHADER_UINT16_P4;
                   break;

        case 8:
                   typeSize.type = gcSHADER_UINT16_P8;
                   break;

        case 16:
                   typeSize .type = gcSHADER_UINT16_P16;
                   break;

        case 32:
                   typeSize .type = gcSHADER_UINT16_P32;
                   break;

        default:
           gcmASSERT(0);
        }
        break;

    case clvTYPE_ULONG:
        switch (clmGEN_CODE_vectorSize_GET(DataType))
        {
        case 0:
                   typeSize.type = gcSHADER_UINT64_X1;
                   break;

        case 2:
                   typeSize.type = gcSHADER_UINT64_X2;
                   break;

        case 3:
                   typeSize.type = gcSHADER_UINT64_X3;
                   break;

        case 4:
                   typeSize.type = gcSHADER_UINT64_X4;
                   break;

        case 8:
                   typeSize.type = gcSHADER_UINT64_X4;
                   typeSize.length = 2;
                   break;

        case 16:
                   typeSize .type = gcSHADER_UINT64_X4;
                   typeSize.length = 4;
                   break;

        default:
           gcmASSERT(0);
        }
        break;

    case clvTYPE_UINT:
        switch (clmGEN_CODE_vectorSize_GET(DataType))
        {
        case 0:
                   typeSize.type = gcSHADER_UINT_X1;
                   break;

        case 2:
                   typeSize.type = gcSHADER_UINT_X2;
                   break;

        case 3:
                   typeSize.type = gcSHADER_UINT_X3;
                   break;

        case 4:
                   typeSize.type = gcSHADER_UINT_X4;
                   break;

        case 8:
                   if(cloCOMPILER_ExtensionEnabled(Compiler, clvEXTENSION_VIV_VX)) {
                       typeSize.type = gcSHADER_UINT_X8;
                   }
                   else {
                       typeSize.type = gcSHADER_UINT_X4;
                       typeSize.length = 2;
                   }
                   break;

        case 16:
                   if(cloCOMPILER_ExtensionEnabled(Compiler, clvEXTENSION_VIV_VX)) {
                       typeSize.type = gcSHADER_UINT_X16;
                   }
                   else {
                       typeSize.type = gcSHADER_UINT_X4;
                       typeSize.length = 4;
                   }
                   break;

        default:
           gcmASSERT(0);
        }
        break;

    case clvTYPE_USHORT:
        switch (clmGEN_CODE_vectorSize_GET(DataType))
        {
        case 0:
                   typeSize.type = gcSHADER_UINT16_X1;
                   break;

        case 2:
                   typeSize.type = gcSHADER_UINT16_X2;
                   break;

        case 3:
                   typeSize.type = gcSHADER_UINT16_X3;
                   break;

        case 4:
                   typeSize.type = gcSHADER_UINT16_X4;
                   break;

        case 8:
                   if(cloCOMPILER_ExtensionEnabled(Compiler, clvEXTENSION_VIV_VX)) {
                       typeSize.type = gcSHADER_UINT16_X8;
                   }
                   else {
                       typeSize.type = gcSHADER_UINT16_X4;
                       typeSize.length = 2;
                   }
                   break;

        case 16:
                   if(cloCOMPILER_ExtensionEnabled(Compiler, clvEXTENSION_VIV_VX)) {
                       typeSize.type = gcSHADER_UINT16_X16;
                   }
                   else {
                       typeSize .type = gcSHADER_UINT16_X4;
                       typeSize.length = 4;
                   }
                   break;

        default:
           gcmASSERT(0);
        }
        break;

    case clvTYPE_UCHAR:
        switch (clmGEN_CODE_vectorSize_GET(DataType))
        {
        case 0:
                   typeSize.type = gcSHADER_UINT8_X1;
                   break;

        case 2:
                   typeSize.type = gcSHADER_UINT8_X2;
                   break;

        case 3:
                   typeSize.type = gcSHADER_UINT8_X3;
                   break;

        case 4:
                   typeSize.type = gcSHADER_UINT8_X4;
                   break;

        case 8:
                   if(cloCOMPILER_ExtensionEnabled(Compiler, clvEXTENSION_VIV_VX)) {
                       typeSize.type = gcSHADER_UINT8_X8;
                   }
                   else {
                       typeSize.type = gcSHADER_UINT8_X4;
                       typeSize.length = 2;
                   }
                   break;

        case 16:
                   if(cloCOMPILER_ExtensionEnabled(Compiler, clvEXTENSION_VIV_VX)) {
                       typeSize.type = gcSHADER_UINT8_X16;
                   }
                   else {
                       typeSize .type = gcSHADER_UINT8_X4;
                       typeSize.length = 4;
                   }
                   break;

        default:
           gcmASSERT(0);
        }
        break;

    case clvTYPE_HALF:
        switch (clmGEN_CODE_vectorSize_GET(DataType)) {
        case 0:
            typeSize.type = gcSHADER_FLOAT16_X1;
            break;

        case 2:
            typeSize.type = gcSHADER_FLOAT16_X2;
            break;

        case 3:
            typeSize.type = gcSHADER_FLOAT16_X3;
            break;

        case 4:
            typeSize.type = gcSHADER_FLOAT16_X4;
            break;

        case 8:
            if(cloCOMPILER_ExtensionEnabled(Compiler, clvEXTENSION_VIV_VX)) {
                typeSize.type = gcSHADER_FLOAT16_X8;
            }
            else {
                typeSize.type = gcSHADER_FLOAT16_X4;
                typeSize.length = 2;
            }
            break;

        case 16:
            if(cloCOMPILER_ExtensionEnabled(Compiler, clvEXTENSION_VIV_VX)) {
                typeSize.type = gcSHADER_FLOAT16_X16;
            }
            else {
                typeSize.type = gcSHADER_FLOAT16_X4;
                typeSize.length = 4;
            }
            break;

        default:
            gcmASSERT(0);
        }
        break;

    case clvTYPE_DOUBLE:
        switch (clmGEN_CODE_vectorSize_GET(DataType)) {
        case 0:
            typeSize.type = gcSHADER_FLOAT_X1;
            break;

        case 2:
            typeSize.type = gcSHADER_FLOAT_X2;
            break;

        case 3:
            typeSize.type = gcSHADER_FLOAT_X3;
            break;

        case 4:
            typeSize.type = gcSHADER_FLOAT_X4;
            break;

        case 8:
            if(cloCOMPILER_ExtensionEnabled(Compiler, clvEXTENSION_VIV_VX)) {
                typeSize.type = gcSHADER_FLOAT_X8;
            }
            else {
                typeSize.type = gcSHADER_FLOAT_X4;
                typeSize.length = 2;
            }
            break;

        case 16:
            if(cloCOMPILER_ExtensionEnabled(Compiler, clvEXTENSION_VIV_VX)) {
                typeSize.type = gcSHADER_FLOAT_X16;
            }
            else {
                typeSize.type = gcSHADER_FLOAT_X4;
                typeSize.length = 4;
            }
            break;

        default:
            gcmASSERT(0);
        }
        break;

    case clvTYPE_FLOAT:
        switch (clmGEN_CODE_matrixColumnCount_GET(DataType)) {
        case 0:
           switch (clmGEN_CODE_vectorSize_GET(DataType)) {
           case 0:
                      typeSize.type = gcSHADER_FLOAT_X1;
                      break;

           case 2:
                      typeSize.type = gcSHADER_FLOAT_X2;
                      break;

           case 3:
                      typeSize.type = gcSHADER_FLOAT_X3;
                      break;

           case 4:
                      typeSize.type = gcSHADER_FLOAT_X4;
                      break;

           case 8:
                   if(cloCOMPILER_ExtensionEnabled(Compiler, clvEXTENSION_VIV_VX)) {
                       typeSize.type = gcSHADER_FLOAT_X8;
                   }
                   else {
                       typeSize.type = gcSHADER_FLOAT_X4;
                       typeSize.length = 2;
                   }
                   break;

           case 16:
                   if(cloCOMPILER_ExtensionEnabled(Compiler, clvEXTENSION_VIV_VX)) {
                       typeSize.type = gcSHADER_FLOAT_X16;
                   }
                   else {
                       typeSize.type = gcSHADER_FLOAT_X4;
                       typeSize.length = 4;
                   }
                   break;

           default:
              gcmASSERT(0);
           }
           break;

        case 2:
           switch (clmGEN_CODE_matrixRowCount_GET(DataType)) {
           case 2:
              typeSize.type = gcSHADER_FLOAT_2X2;
              break;

           case 3:
              typeSize.type = gcSHADER_FLOAT_2X3;
              break;

           case 4:
              typeSize.type = gcSHADER_FLOAT_2X4;
              break;

           case 8:
              typeSize.type = gcSHADER_FLOAT_2X4;
              typeSize.length = 2;
              break;

           case 16:
              typeSize.type = gcSHADER_FLOAT_2X4;
              typeSize.length = 4;
              break;

           default:
              gcmASSERT(0);
           }
           break;

        case 3:
           switch (clmGEN_CODE_matrixRowCount_GET(DataType)) {
           case 2:
              typeSize.type = gcSHADER_FLOAT_3X2;
              break;

           case 3:
              typeSize.type = gcSHADER_FLOAT_3X3;
              break;

           case 4:
              typeSize.type = gcSHADER_FLOAT_3X4;
              break;

           case 8:
              typeSize.type = gcSHADER_FLOAT_3X4;
              typeSize.length = 2;
              break;

           case 16:
              typeSize.type = gcSHADER_FLOAT_3X4;
              typeSize.length = 4;
              break;

           default:
              gcmASSERT(0);
           }
           break;

        case 4:
           switch (clmGEN_CODE_matrixRowCount_GET(DataType)) {
           case 2:
              typeSize.type = gcSHADER_FLOAT_4X2;
              break;

           case 3:
              typeSize.type = gcSHADER_FLOAT_4X3;
              break;

           case 4:
              typeSize.type = gcSHADER_FLOAT_4X4;
              break;

           case 8:
              typeSize.type = gcSHADER_FLOAT_4X4;
              typeSize.length = 2;
              break;

           case 16:
              typeSize.type = gcSHADER_FLOAT_4X4;
              typeSize.length = 4;
              break;

           default:
              gcmASSERT(0);
           }
           break;

        case 5:
        case 6:
        case 7:
        case 8:
           switch (clmGEN_CODE_matrixRowCount_GET(DataType)) {
           case 2:
              typeSize.type = gcSHADER_FLOAT_4X2;
              typeSize.length = 2;
              break;

           case 3:
              typeSize.type = gcSHADER_FLOAT_4X3;
              typeSize.length = 2;
              break;

           case 4:
              typeSize.type = gcSHADER_FLOAT_4X4;
              typeSize.length = 2;
              break;

           case 8:
              typeSize.type = gcSHADER_FLOAT_4X4;
              typeSize.length = 4;
              break;

           case 16:
              typeSize.type = gcSHADER_FLOAT_4X4;
              typeSize.length = 8;
              break;

           default:
              gcmASSERT(0);
           }
           break;

        case 9:
        case 10:
        case 11:
        case 12:
           switch (clmGEN_CODE_matrixRowCount_GET(DataType)) {
           case 2:
              typeSize.type = gcSHADER_FLOAT_4X2;
              typeSize.length = 3;
              break;

           case 3:
              typeSize.type = gcSHADER_FLOAT_4X3;
              typeSize.length = 3;
              break;

           case 4:
              typeSize.type = gcSHADER_FLOAT_4X4;
              typeSize.length = 3;
              break;

           case 8:
              typeSize.type = gcSHADER_FLOAT_4X4;
              typeSize.length = 6;
              break;

           case 16:
              typeSize.type = gcSHADER_FLOAT_4X4;
              typeSize.length = 12;
              break;

           default:
              gcmASSERT(0);
           }
           break;

        case 13:
        case 14:
        case 15:
        case 16:
           switch (clmGEN_CODE_matrixRowCount_GET(DataType)) {
           case 2:
              typeSize.type = gcSHADER_FLOAT_4X2;
              typeSize.length = 4;
              break;

           case 3:
              typeSize.type = gcSHADER_FLOAT_4X3;
              typeSize.length = 4;
              break;

           case 4:
              typeSize.type = gcSHADER_FLOAT_4X4;
              typeSize.length = 4;
              break;

           case 8:
              typeSize.type = gcSHADER_FLOAT_4X4;
              typeSize.length = 8;
              break;

           case 16:
              typeSize.type = gcSHADER_FLOAT_4X4;
              typeSize.length = 16;
              break;

           default:
              gcmASSERT(0);
           }
           break;

        default:
           gcmASSERT(0);
        }
        break;

    case clvTYPE_EVENT_T:
        typeSize.type = gcSHADER_INTEGER_X1;
        break;

    case clvTYPE_SAMPLER_T:
        typeSize.type = gcSHADER_SAMPLER_T;
        break;

    case clvTYPE_IMAGE2D_T:
        typeSize.type = gcSHADER_IMAGE_2D_T;
        break;

    case clvTYPE_IMAGE3D_T:
        typeSize.type = gcSHADER_IMAGE_3D_T;
        break;

    case clvTYPE_IMAGE2D_ARRAY_T:
        typeSize.type = gcSHADER_IMAGE_2D_ARRAY_T;
        break;

    case clvTYPE_IMAGE1D_T:
        typeSize.type = gcSHADER_IMAGE_1D_T;
        break;

    case clvTYPE_IMAGE1D_ARRAY_T:
        typeSize.type = gcSHADER_IMAGE_1D_ARRAY_T;
        break;

    case clvTYPE_IMAGE1D_BUFFER_T:
        typeSize.type = gcSHADER_IMAGE_1D_BUFFER_T;
        break;

    case clvTYPE_VIV_GENERIC_IMAGE_T:
        typeSize.type = gcSHADER_VIV_GENERIC_IMAGE_T;
        break;

    case clvTYPE_SAMPLER2D:
        typeSize.type = gcSHADER_SAMPLER_2D;
        break;

    case clvTYPE_SAMPLER3D:
        typeSize.type = gcSHADER_SAMPLER_3D;
        break;

    case clvTYPE_STRUCT:
    case clvTYPE_UNION:
        typeSize.type = gcSHADER_INTEGER_X4;
        break;

    default:
        typeSize.type = gcSHADER_INTEGER_X1;
        break;
    }
    return typeSize;
}

gctCONST_STRING
gcGetShaderDataTypeName(
IN gcSHADER_TYPE Type
)
{
    switch (Type) {
    case gcSHADER_FLOAT_X1:                        return "gcSHADER_FLOAT_X1";                      /* 0x00 */
    case gcSHADER_FLOAT_X2:                        return "gcSHADER_FLOAT_X2";                      /* 0x01 */
    case gcSHADER_FLOAT_X3:                        return "gcSHADER_FLOAT_X3";                      /* 0x02 */
    case gcSHADER_FLOAT_X4:                        return "gcSHADER_FLOAT_X4";                      /* 0x03 */
    case gcSHADER_FLOAT_2X2:                       return "gcSHADER_FLOAT_2X2";                     /* 0x04 */
    case gcSHADER_FLOAT_3X3:                       return "gcSHADER_FLOAT_3X3";                     /* 0x05 */
    case gcSHADER_FLOAT_4X4:                       return "gcSHADER_FLOAT_4X4";                     /* 0x06 */
    case gcSHADER_BOOLEAN_X1:                      return "gcSHADER_BOOLEAN_X1";                    /* 0x07 */
    case gcSHADER_BOOLEAN_X2:                      return "gcSHADER_BOOLEAN_X2";                    /* 0x08 */
    case gcSHADER_BOOLEAN_X3:                      return "gcSHADER_BOOLEAN_X3";                    /* 0x09 */
    case gcSHADER_BOOLEAN_X4:                      return "gcSHADER_BOOLEAN_X4";                    /* 0x0A */
    case gcSHADER_INTEGER_X1:                      return "gcSHADER_INTEGER_X1";                    /* 0x0B */
    case gcSHADER_INTEGER_X2:                      return "gcSHADER_INTEGER_X2";                    /* 0x0C */
    case gcSHADER_INTEGER_X3:                      return "gcSHADER_INTEGER_X3";                    /* 0x0D */
    case gcSHADER_INTEGER_X4:                      return "gcSHADER_INTEGER_X4";                    /* 0x0E */
    case gcSHADER_SAMPLER_1D:                      return "gcSHADER_SAMPLER_1D";                    /* 0x0F */
    case gcSHADER_SAMPLER_2D:                      return "gcSHADER_SAMPLER_2D";                    /* 0x10 */
    case gcSHADER_SAMPLER_3D:                      return "gcSHADER_SAMPLER_3D";                    /* 0x11 */
    case gcSHADER_SAMPLER_CUBIC:                   return "gcSHADER_SAMPLER_CUBIC";                 /* 0x12 */
    case gcSHADER_FIXED_X1:                        return "gcSHADER_FIXED_X1";                      /* 0x13 */
    case gcSHADER_FIXED_X2:                        return "gcSHADER_FIXED_X2";                      /* 0x14 */
    case gcSHADER_FIXED_X3:                        return "gcSHADER_FIXED_X3";                      /* 0x15 */
    case gcSHADER_FIXED_X4:                        return "gcSHADER_FIXED_X4";                      /* 0x16 */

    /* For OCL. */
    case gcSHADER_IMAGE_1D_T:                      return "gcSHADER_IMAGE_1D_T";                    /* 0x17 */
    case gcSHADER_IMAGE_1D_BUFFER_T:               return "gcSHADER_IMAGE_1D_BUFFER_T";             /* 0x18 */
    case gcSHADER_IMAGE_1D_ARRAY_T:                return "gcSHADER_IMAGE_1D_ARRAY_T";              /* 0x19 */
    case gcSHADER_IMAGE_2D_T:                      return "gcSHADER_IMAGE_2D_T";                    /* 0x1A */
    case gcSHADER_IMAGE_2D_ARRAY_T:                return "gcSHADER_IMAGE_2D_ARRAY_T";              /* 0x1B */
    case gcSHADER_IMAGE_3D_T:                      return "gcSHADER_IMAGE_3D_T";                    /* 0x1C */
    case gcSHADER_VIV_GENERIC_IMAGE_T:             return "gcSHADER_VIV_GENERIC_IMAGE_T";           /* 0x1D */
    case gcSHADER_SAMPLER_T:                       return "gcSHADER_SAMPLER_T";                     /* 0x1E */

    case gcSHADER_FLOAT_2X3:                       return "gcSHADER_FLOAT_2X3";                     /* 0x1F */
    case gcSHADER_FLOAT_2X4:                       return "gcSHADER_FLOAT_2X4";                     /* 0x20 */
    case gcSHADER_FLOAT_3X2:                       return "gcSHADER_FLOAT_3X2";                     /* 0x21 */
    case gcSHADER_FLOAT_3X4:                       return "gcSHADER_FLOAT_3X4";                     /* 0x22 */
    case gcSHADER_FLOAT_4X2:                       return "gcSHADER_FLOAT_4X2";                     /* 0x23 */
    case gcSHADER_FLOAT_4X3:                       return "gcSHADER_FLOAT_4X3";                     /* 0x24 */
    case gcSHADER_ISAMPLER_2D:                     return "gcSHADER_ISAMPLER_2D";                   /* 0x25 */
    case gcSHADER_ISAMPLER_3D:                     return "gcSHADER_ISAMPLER_3D";                   /* 0x26 */
    case gcSHADER_ISAMPLER_CUBIC:                  return "gcSHADER_ISAMPLER_CUBIC";                /* 0x27 */
    case gcSHADER_USAMPLER_2D:                     return "gcSHADER_USAMPLER_2D";                   /* 0x28 */
    case gcSHADER_USAMPLER_3D:                     return "gcSHADER_USAMPLER_3D";                   /* 0x29 */
    case gcSHADER_USAMPLER_CUBIC:                  return "gcSHADER_USAMPLER_CUBIC";                /* 0x2A */
    case gcSHADER_SAMPLER_EXTERNAL_OES:            return "gcSHADER_SAMPLER_EXTERNAL_OES";          /* 0x2B */

    case gcSHADER_UINT_X1:                         return "gcSHADER_UINT_X1";                       /* 0x2C */
    case gcSHADER_UINT_X2:                         return "gcSHADER_UINT_X2";                       /* 0x2D */
    case gcSHADER_UINT_X3:                         return "gcSHADER_UINT_X3";                       /* 0x2E */
    case gcSHADER_UINT_X4:                         return "gcSHADER_UINT_X4";                       /* 0x2F */

    case gcSHADER_SAMPLER_2D_SHADOW:               return "gcSHADER_SAMPLER_2D_SHADOW";             /* 0x30 */
    case gcSHADER_SAMPLER_CUBE_SHADOW:             return "gcSHADER_SAMPLER_CUBE_SHADOW";           /* 0x31 */

    case gcSHADER_SAMPLER_1D_ARRAY:                return "gcSHADER_SAMPLER_1D_ARRAY";              /* 0x32 */
    case gcSHADER_SAMPLER_1D_ARRAY_SHADOW:         return "gcSHADER_SAMPLER_1D_ARRAY_SHADOW";       /* 0x33 */
    case gcSHADER_SAMPLER_2D_ARRAY:                return "gcSHADER_SAMPLER_2D_ARRAY";              /* 0x34 */
    case gcSHADER_ISAMPLER_2D_ARRAY:               return "gcSHADER_ISAMPLER_2D_ARRAY";             /* 0x35 */
    case gcSHADER_USAMPLER_2D_ARRAY:               return "gcSHADER_USAMPLER_2D_ARRAY";             /* 0x36 */
    case gcSHADER_SAMPLER_2D_ARRAY_SHADOW:         return "gcSHADER_SAMPLER_2D_ARRAY_SHADOW";       /* 0x37 */

    case gcSHADER_SAMPLER_2D_MS:                   return "gcSHADER_SAMPLER_2D_MS";                 /* 0x38 */
    case gcSHADER_ISAMPLER_2D_MS:                  return "gcSHADER_ISAMPLER_2D_MS";                /* 0x39 */
    case gcSHADER_USAMPLER_2D_MS:                  return "gcSHADER_USAMPLER_2D_MS";                /* 0x3A */
    case gcSHADER_SAMPLER_2D_MS_ARRAY:             return "gcSHADER_SAMPLER_2D_MS_ARRAY";           /* 0x3B */
    case gcSHADER_ISAMPLER_2D_MS_ARRAY:            return "gcSHADER_ISAMPLER_2D_MS_ARRAY";          /* 0x3C */
    case gcSHADER_USAMPLER_2D_MS_ARRAY:            return "gcSHADER_USAMPLER_2D_MS_ARRAY";          /* 0x3D */

    case gcSHADER_IMAGE_2D:                        return "gcSHADER_IMAGE_2D";                      /* 0x3E */
    case gcSHADER_IIMAGE_2D:                       return "gcSHADER_IIMAGE_2D";                     /* 0x3F */
    case gcSHADER_UIMAGE_2D:                       return "gcSHADER_UIMAGE_2D";                     /* 0x40 */
    case gcSHADER_IMAGE_3D:                        return "gcSHADER_IMAGE_3D";                      /* 0x41 */
    case gcSHADER_IIMAGE_3D:                       return "gcSHADER_IIMAGE_3D";                     /* 0x42 */
    case gcSHADER_UIMAGE_3D:                       return "gcSHADER_UIMAGE_3D";                     /* 0x43 */
    case gcSHADER_IMAGE_CUBE:                      return "gcSHADER_IMAGE_CUBE";                    /* 0x44 */
    case gcSHADER_IIMAGE_CUBE:                     return "gcSHADER_IIMAGE_CUBE";                   /* 0x45 */
    case gcSHADER_UIMAGE_CUBE:                     return "gcSHADER_UIMAGE_CUBE";                   /* 0x46 */
    case gcSHADER_IMAGE_2D_ARRAY:                  return "gcSHADER_IMAGE_2D_ARRAY";                /* 0x47 */
    case gcSHADER_IIMAGE_2D_ARRAY:                 return "gcSHADER_IIMAGE_2D_ARRAY";               /* 0x48 */
    case gcSHADER_UIMAGE_2D_ARRAY:                 return "gcSHADER_UIMAGE_2D_ARRAY";               /* 0x49 */
    case gcSHADER_VIV_GENERIC_GL_IMAGE:            return "gcSHADER_VIV_GENERIC_GL_IMAGE";          /* 0x4A */

    case gcSHADER_ATOMIC_UINT:                     return "gcSHADER_ATOMIC_UINT";                   /* 0x4B */

    /* GL_EXT_texture_cube_map_array */
    case gcSHADER_SAMPLER_CUBEMAP_ARRAY:           return "gcSHADER_SAMPLER_CUBEMAP_ARRAY";         /* 0x4C */
    case gcSHADER_SAMPLER_CUBEMAP_ARRAY_SHADOW:    return "gcSHADER_SAMPLER_CUBEMAP_ARRAY_SHADOW";  /* 0x4D */
    case gcSHADER_ISAMPLER_CUBEMAP_ARRAY:          return "gcSHADER_ISAMPLER_CUBEMAP_ARRAY";        /* 0x4E */
    case gcSHADER_USAMPLER_CUBEMAP_ARRAY:          return "gcSHADER_USAMPLER_CUBEMAP_ARRAY";        /* 0x4F */
    case gcSHADER_IMAGE_CUBEMAP_ARRAY:             return "gcSHADER_IMAGE_CUBEMAP_ARRAY";           /* 0x50 */
    case gcSHADER_IIMAGE_CUBEMAP_ARRAY:            return "gcSHADER_IIMAGE_CUBEMAP_ARRAY";          /* 0x51 */
    case gcSHADER_UIMAGE_CUBEMAP_ARRAY:            return "gcSHADER_UIMAGE_CUBEMAP_ARRAY";          /* 0x52 */

    case gcSHADER_INT64_X1:                        return "gcSHADER_INT64_X1";                      /* 0x53 */
    case gcSHADER_INT64_X2:                        return "gcSHADER_INT64_X2";                      /* 0x54 */
    case gcSHADER_INT64_X3:                        return "gcSHADER_INT64_X3";                      /* 0x55 */
    case gcSHADER_INT64_X4:                        return "gcSHADER_INT64_X4";                      /* 0x56 */
    case gcSHADER_UINT64_X1:                       return "gcSHADER_UINT64_X1";                     /* 0x57 */
    case gcSHADER_UINT64_X2:                       return "gcSHADER_UINT64_X2";                     /* 0x58 */
    case gcSHADER_UINT64_X3:                       return "gcSHADER_UINT64_X3";                     /* 0x59 */
    case gcSHADER_UINT64_X4:                       return "gcSHADER_UINT64_X4";                     /* 0x5A */

    /* texture buffer extension type. */
    case gcSHADER_SAMPLER_BUFFER:                  return "gcSHADER_SAMPLER_BUFFER";                /* 0x5B */
    case gcSHADER_ISAMPLER_BUFFER:                 return "gcSHADER_ISAMPLER_BUFFER";               /* 0x5C */
    case gcSHADER_USAMPLER_BUFFER:                 return "gcSHADER_USAMPLER_BUFFER";               /* 0x5D */
    case gcSHADER_IMAGE_BUFFER:                    return "gcSHADER_IMAGE_BUFFER";                  /* 0x5E */
    case gcSHADER_IIMAGE_BUFFER:                   return "gcSHADER_IIMAGE_BUFFER";                 /* 0x5F */
    case gcSHADER_UIMAGE_BUFFER:                   return "gcSHADER_UIMAGE_BUFFER";                 /* 0x60 */
    case gcSHADER_VIV_GENERIC_GL_SAMPLER:          return "gcSHADER_VIV_GENERIC_GL_SAMPLER";        /* 0x61 */

    /* float16 */
    case gcSHADER_FLOAT16_X1:                      return "gcSHADER_FLOAT16_X1";                    /* 0x62 half2 */
    case gcSHADER_FLOAT16_X2:                      return "gcSHADER_FLOAT16_X2";                    /* 0x63 half2 */
    case gcSHADER_FLOAT16_X3:                      return "gcSHADER_FLOAT16_X3";                    /* 0x64 half3 */
    case gcSHADER_FLOAT16_X4:                      return "gcSHADER_FLOAT16_X4";                    /* 0x65 half4 */
    case gcSHADER_FLOAT16_X8:                      return "gcSHADER_FLOAT16_X8";                    /* 0x66 half8 */
    case gcSHADER_FLOAT16_X16:                     return "gcSHADER_FLOAT16_X16";                   /* 0x67 half16 */
    case gcSHADER_FLOAT16_X32:                     return "gcSHADER_FLOAT16_X32";                   /* 0x68 half32 */

    /*  boolean  */
    case gcSHADER_BOOLEAN_X8:                      return "gcSHADER_BOOLEAN_X8";                    /* 0x69 */
    case gcSHADER_BOOLEAN_X16:                     return "gcSHADER_BOOLEAN_X16";                   /* 0x6A */
    case gcSHADER_BOOLEAN_X32:                     return "gcSHADER_BOOLEAN_X32";                   /* 0x6B */

    /* uchar vectors  */
    case gcSHADER_UINT8_X1:                        return "gcSHADER_UINT8_X1";                      /* 0x6C */
    case gcSHADER_UINT8_X2:                        return "gcSHADER_UINT8_X2";                      /* 0x6D */
    case gcSHADER_UINT8_X3:                        return "gcSHADER_UINT8_X3";                      /* 0x6E */
    case gcSHADER_UINT8_X4:                        return "gcSHADER_UINT8_X4";                      /* 0x6F */
    case gcSHADER_UINT8_X8:                        return "gcSHADER_UINT8_X8";                      /* 0x70 */
    case gcSHADER_UINT8_X16:                       return "gcSHADER_UINT8_X16";                     /* 0x71 */
    case gcSHADER_UINT8_X32:                       return "gcSHADER_UINT8_X32";                     /* 0x72 */

    /* char vectors  */
    case gcSHADER_INT8_X1:                         return "gcSHADER_INT8_X1";                       /* 0x73 */
    case gcSHADER_INT8_X2:                         return "gcSHADER_INT8_X2";                       /* 0x74 */
    case gcSHADER_INT8_X3:                         return "gcSHADER_INT8_X3";                       /* 0x75 */
    case gcSHADER_INT8_X4:                         return "gcSHADER_INT8_X4";                       /* 0x76 */
    case gcSHADER_INT8_X8:                         return "gcSHADER_INT8_X8";                       /* 0x77 */
    case gcSHADER_INT8_X16:                        return "gcSHADER_INT8_X16";                      /* 0x78 */
    case gcSHADER_INT8_X32:                        return "gcSHADER_INT8_X32";                      /* 0x79 */

    /* ushort vectors */
    case gcSHADER_UINT16_X1:                       return "gcSHADER_UINT16_X1";                     /* 0x7A */
    case gcSHADER_UINT16_X2:                       return "gcSHADER_UINT16_X2";                     /* 0x7B */
    case gcSHADER_UINT16_X3:                       return "gcSHADER_UINT16_X3";                     /* 0x7C */
    case gcSHADER_UINT16_X4:                       return "gcSHADER_UINT16_X4";                     /* 0x7D */
    case gcSHADER_UINT16_X8:                       return "gcSHADER_UINT16_X8";                     /* 0x7E */
    case gcSHADER_UINT16_X16:                      return "gcSHADER_UINT16_X16";                    /* 0x7F */
    case gcSHADER_UINT16_X32:                      return "gcSHADER_UINT16_X32";                    /* 0x80 */

    /* short vectors */
    case gcSHADER_INT16_X1:                        return "gcSHADER_INT16_X1";                      /* 0x81 */
    case gcSHADER_INT16_X2:                        return "gcSHADER_INT16_X2";                      /* 0x82 */
    case gcSHADER_INT16_X3:                        return "gcSHADER_INT16_X3";                      /* 0x83 */
    case gcSHADER_INT16_X4:                        return "gcSHADER_INT16_X4";                      /* 0x84 */
    case gcSHADER_INT16_X8:                        return "gcSHADER_INT16_X8";                      /* 0x85 */
    case gcSHADER_INT16_X16:                       return "gcSHADER_INT16_X16";                     /* 0x86 */
    case gcSHADER_INT16_X32:                       return "gcSHADER_INT16_X32";                     /* 0x87 */

    /* packed data type */
    /* packed float16 (2 bytes per element) */
    case gcSHADER_FLOAT16_P2:                      return "gcSHADER_FLOAT16_P2";                    /* 0x88 half2 */
    case gcSHADER_FLOAT16_P3:                      return "gcSHADER_FLOAT16_P3";                    /* 0x89 half3 */
    case gcSHADER_FLOAT16_P4:                      return "gcSHADER_FLOAT16_P4";                    /* 0x8A half4 */
    case gcSHADER_FLOAT16_P8:                      return "gcSHADER_FLOAT16_P8";                    /* 0x8B half8 */
    case gcSHADER_FLOAT16_P16:                     return "gcSHADER_FLOAT16_P16";                   /* 0x8C half16 */
    case gcSHADER_FLOAT16_P32:                     return "gcSHADER_FLOAT16_P32";                   /* 0x8D half32 */

    /* packed boolean (1 byte per element) */
    case gcSHADER_BOOLEAN_P2:                      return "gcSHADER_BOOLEAN_P2";                    /* 0x8E bool2 bvec2 */
    case gcSHADER_BOOLEAN_P3:                      return "gcSHADER_BOOLEAN_P3";                    /* 0x8F */
    case gcSHADER_BOOLEAN_P4:                      return "gcSHADER_BOOLEAN_P4";                    /* 0x90 */
    case gcSHADER_BOOLEAN_P8:                      return "gcSHADER_BOOLEAN_P8";                    /* 0x91 */
    case gcSHADER_BOOLEAN_P16:                     return "gcSHADER_BOOLEAN_P16";                   /* 0x92 */
    case gcSHADER_BOOLEAN_P32:                     return "gcSHADER_BOOLEAN_P32";                   /* 0x93 */

    /* uchar vectors (1 byte per element) */
    case gcSHADER_UINT8_P2:                        return "gcSHADER_UINT8_P2";                      /* 0x94 */
    case gcSHADER_UINT8_P3:                        return "gcSHADER_UINT8_P3";                      /* 0x95 */
    case gcSHADER_UINT8_P4:                        return "gcSHADER_UINT8_P4";                      /* 0x96 */
    case gcSHADER_UINT8_P8:                        return "gcSHADER_UINT8_P8";                      /* 0x97 */
    case gcSHADER_UINT8_P16:                       return "gcSHADER_UINT8_P16";                     /* 0x98 */
    case gcSHADER_UINT8_P32:                       return "gcSHADER_UINT8_P32";                     /* 0x99 */

    /* char vectors (1 byte per element) */
    case gcSHADER_INT8_P2:                         return "gcSHADER_INT8_P2";                       /* 0x9A */
    case gcSHADER_INT8_P3:                         return "gcSHADER_INT8_P3";                       /* 0x9B */
    case gcSHADER_INT8_P4:                         return "gcSHADER_INT8_P4";                       /* 0x9C */
    case gcSHADER_INT8_P8:                         return "gcSHADER_INT8_P8";                       /* 0x9D */
    case gcSHADER_INT8_P16:                        return "gcSHADER_INT8_P16";                      /* 0x9E */
    case gcSHADER_INT8_P32:                        return "gcSHADER_INT8_P32";                      /* 0x9F */

    /* ushort vectors (2 bytes per element) */
    case gcSHADER_UINT16_P2:                       return "gcSHADER_UINT16_P2";                     /* 0xA0 */
    case gcSHADER_UINT16_P3:                       return "gcSHADER_UINT16_P3";                     /* 0xA1 */
    case gcSHADER_UINT16_P4:                       return "gcSHADER_UINT16_P4";                     /* 0xA2 */
    case gcSHADER_UINT16_P8:                       return "gcSHADER_UINT16_P8";                     /* 0xA3 */
    case gcSHADER_UINT16_P16:                      return "gcSHADER_UINT16_P16";                    /* 0xA4 */
    case gcSHADER_UINT16_P32:                      return "gcSHADER_UINT16_P32";                    /* 0xA5 */

    /* short vectors (2 bytes per element) */
    case gcSHADER_INT16_P2:                        return "gcSHADER_INT16_P2";                      /* 0xA6 */
    case gcSHADER_INT16_P3:                        return "gcSHADER_INT16_P3";                      /* 0xA7 */
    case gcSHADER_INT16_P4:                        return "gcSHADER_INT16_P4";                      /* 0xA8 */
    case gcSHADER_INT16_P8:                        return "gcSHADER_INT16_P8";                      /* 0xA9 */
    case gcSHADER_INT16_P16:                       return "gcSHADER_INT16_P16";                     /* 0xAA */
    case gcSHADER_INT16_P32:                       return "gcSHADER_INT16_P32";                     /* 0xAB */

    case gcSHADER_INTEGER_X8:                      return "gcSHADER_INTEGER_X8";                    /* 0xAC */
    case gcSHADER_INTEGER_X16:                     return "gcSHADER_INTEGER_X16";                   /* 0xAD */
    case gcSHADER_UINT_X8:                         return "gcSHADER_UINT_X8";                       /* 0xAE */
    case gcSHADER_UINT_X16:                        return "gcSHADER_UINT_X16";                      /* 0xAF */
    case gcSHADER_FLOAT_X8:                        return "gcSHADER_FLOAT_X8";                      /* 0xB0 */
    case gcSHADER_FLOAT_X16:                       return "gcSHADER_FLOAT_X16";                     /* 0xB1 */
    case gcSHADER_INT64_X8:                        return "gcSHADER_INT64_X8";                      /* 0xB2 */
    case gcSHADER_INT64_X16:                       return "gcSHADER_INT64_X16";                     /* 0xB3 */
    case gcSHADER_UINT64_X8:                       return "gcSHADER_UINT64_X8";                     /* 0xB4 */
    case gcSHADER_UINT64_X16:                      return "gcSHADER_UINT64_X16";                    /* 0xB5 */

    default:
    gcmASSERT(0);
    return "Invalid";
    }
}

gctCONST_STRING
gcGetDataTypeName(
IN cloCOMPILER Compiler,
IN clsGEN_CODE_DATA_TYPE DataType
)
{
  gcsTYPE_SIZE typeSize;

  typeSize = clConvToShaderDataType(Compiler, DataType);
  return gcGetShaderDataTypeName(typeSize.type);
}

static gctCONST_STRING
_GetFormatName(
IN gcSL_FORMAT Format
)
{
    gcSL_FORMAT format;

    format = GetFormatBasicType(Format);
    if(isFormatSpecialType(Format))
    {
        format = GetFormatSpecialType(Format);
    }

    switch (format) {
    case gcSL_FLOAT:    return "gcSL_FLOAT";
    case gcSL_FLOAT16:  return "gcSL_FLOAT16";
    case gcSL_FLOAT64:  return "gcSL_FLOAT64";
    case gcSL_INTEGER:  return "gcSL_INTEGER";
    case gcSL_INT64:    return "gcSL_INT64";
    case gcSL_INT8:     return "gcSL_INT8";
    case gcSL_INT16:    return "gcSL_INT16";
    case gcSL_UINT64:   return "gcSL_UINT64";
    case gcSL_UINT32:   return "gcSL_UINT32";
    case gcSL_UINT8:    return "gcSL_UINT8";
    case gcSL_UINT16:   return "gcSL_UINT16";
    case gcSL_SNORM8:   return "gcSL_SNORM8";
    case gcSL_UNORM8:   return "gcSL_UNORM8";
    case gcSL_SNORM16:  return "gcSL_SNORM16";
    case gcSL_UNORM16:  return "gcSL_UNORM16";
    case gcSL_BOOLEAN:  return "gcSL_BOOLEAN";
    case gcSL_VOID:     return "gcSL_VOID";
    case gcSL_SAMPLER_T:  return "gcSL_SAMPLER_T";
    case gcSL_EVENT_T:  return "gcSL_EVENT_T";
    case gcSL_SIZE_T:   return "gcSL_SIZE_T";
    case gcSL_PTRDIFF_T:  return "gcSL_PTRDIFF_T";
    case gcSL_INTPTR_T:   return "gcSL_INTPTR_T";
    case gcSL_UINTPTR_T:  return "gcSL_UINTPTR_T";
    case gcSL_STRUCT:   return "gcSL_STRUCT";
    case gcSL_UNION:    return "gcSL_UNION";
    case gcSL_ENUM:     return "gcSL_ENUM";
    case gcSL_TYPEDEF:  return "gcSL_TYPEDEF";

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
    switch (Qualifier) {
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
    IN clsGEN_CODE_DATA_TYPE DataType
    )
{
    switch (clmGEN_CODE_elementType_GET(DataType))
    {
    case clvTYPE_SAMPLER2D:
    case clvTYPE_SAMPLER3D:
        return gcvTRUE;

    default:
        return gcvFALSE;
    }
}

gctBOOL
gcIsScalarDataType(
IN clsGEN_CODE_DATA_TYPE DataType
)
{
    return clmGEN_CODE_IsScalarDataType(DataType);
}

gctBOOL
gcIsVectorDataType(
IN clsGEN_CODE_DATA_TYPE DataType
)
{
    return clmGEN_CODE_IsVectorDataType(DataType);
}

gctBOOL
gcIsMatrixDataType(
IN clsGEN_CODE_DATA_TYPE DataType
)
{
    return clmGEN_CODE_IsMatrixDataType(DataType);
}

clsGEN_CODE_DATA_TYPE
gcGetVectorComponentDataType(
IN clsGEN_CODE_DATA_TYPE DataType
)
{
    clmGEN_CODE_vectorSize_SET(DataType, 0);
    return DataType;
}

clsGEN_CODE_DATA_TYPE
gcGetVectorSliceDataType(
IN clsGEN_CODE_DATA_TYPE DataType,
IN gctUINT8 Components
)
{
    if(!gcIsVectorDataType(DataType) || Components > cldMaxComponentCount) {
    gcmASSERT(0);
        clmGEN_CODE_vectorSize_SET(DataType, 0);
    }
    else if(Components == 1) {
        clmGEN_CODE_vectorSize_SET(DataType, 0);
    }
    else {
        clmGEN_CODE_vectorSize_SET(DataType, Components);
    }
    return DataType;
}

clsGEN_CODE_DATA_TYPE
gcConvScalarToVectorDataType(
IN clsGEN_CODE_DATA_TYPE DataType,
IN gctUINT8 Components
)
{
    if(!gcIsScalarDataType(DataType) || Components == 1 || Components > cldMaxComponentCount) {
       gcmASSERT(0);
       clmGEN_CODE_vectorSize_SET(DataType, 4);
    }
    else {
        clmGEN_CODE_vectorSize_SET(DataType, Components);
    }
    return DataType;
}

gctUINT8
gcGetVectorDataTypeComponentCount(
IN clsGEN_CODE_DATA_TYPE DataType
)
{
    if(!gcIsVectorDataType(DataType)) {
      gcmASSERT(0);
      return 4;
    }
    else {
      return clmGEN_CODE_vectorSize_NOCHECK_GET(DataType);
    }
}

clsGEN_CODE_DATA_TYPE
gcGetVectorComponentSelectionDataType(
IN clsGEN_CODE_DATA_TYPE DataType,
IN gctUINT8 Components
)
{
    if(!gcIsVectorDataType(DataType) || Components > cldMaxComponentCount) {
        gcmASSERT(0);
        clmGEN_CODE_vectorSize_SET(DataType, 0);
    }
    else if(Components == 1) {
        clmGEN_CODE_vectorSize_SET(DataType, 0);
    }
    else {
        clmGEN_CODE_vectorSize_SET(DataType, Components);
    }
    return DataType;
}

clsGEN_CODE_DATA_TYPE
gcGetMatrixColumnDataType(
IN clsGEN_CODE_DATA_TYPE DataType
)
{
    if(!gcIsMatrixDataType(DataType)) {
    gcmASSERT(0);
        clmGEN_CODE_vectorSize_SET(DataType, 4);
    }
    else {
        gctUINT8 vectorSize = clmGEN_CODE_matrixRowCount_GET(DataType);
        clmGEN_CODE_vectorSize_SET(DataType, vectorSize);
    }
    return DataType;
}

gctUINT
gcGetMatrixDataTypeColumnCount(
IN clsGEN_CODE_DATA_TYPE DataType
)
{
    if(!gcIsMatrixDataType(DataType)) {
       gcmASSERT(0);
       return 4;
    }
    else {
       return clmGEN_CODE_matrixColumnCount_GET(DataType);
    }
}

gctUINT
gcGetMatrixDataTypeRowCount(
IN clsGEN_CODE_DATA_TYPE DataType
)
{
    if(!gcIsMatrixDataType(DataType)) {
       gcmASSERT(0);
       return 4;
    }
    else {
       return clmGEN_CODE_matrixRowCount_GET(DataType);
    }
}

gctSIZE_T
gcGetMatrixColumnRegSize(
IN clsGEN_CODE_DATA_TYPE DataType
)
{
    return gcGetDataTypeRegSize(gcGetMatrixColumnDataType(DataType));
}

static gceSTATUS
_AddAttribute(
IN cloCOMPILER Compiler,
IN gctCONST_STRING Name,
IN gcSHADER_TYPE Type,
IN gctSIZE_T Length,
IN gctBOOL IsTexture,
OUT gcATTRIBUTE * Attribute
)
{
    gcSHADER binary;

    gcmASSERT(Attribute);
    gcmVERIFY_OK(cloCOMPILER_GetBinary(Compiler, &binary));

    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                      clvDUMP_CODE_EMITTER,
                      "gcSHADER_AddAttribute(Shader, \"%s\", %s, %d, %s);",
                      Name,
                      gcGetShaderDataTypeName(Type),
                      Length,
                      (IsTexture)? "true" : "false"));
    return gcSHADER_AddAttribute(binary, Name, Type, Length, IsTexture, gcSHADER_SHADER_DEFAULT, gcSHADER_PRECISION_DEFAULT, Attribute);
}

static gceSTATUS
_AddOutput(
IN cloCOMPILER Compiler,
IN gctCONST_STRING Name,
IN gcSHADER_TYPE Type,
IN gctSIZE_T Length,
IN gctREG_INDEX TempRegister
)
{
    gcSHADER    binary;

    gcmVERIFY_OK(cloCOMPILER_GetBinary(Compiler, &binary));
    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                      clvDUMP_CODE_EMITTER,
                      "gcSHADER_AddOutput(Shader, \"%s\", %s, %d, %d);",
                      Name,
                      gcGetShaderDataTypeName(Type),
                      Length,
                      TempRegister));

    return gcSHADER_AddOutputWithLocation(binary,
                                          Name,
                                          Type,
                                          gcSHADER_PRECISION_DEFAULT,
                                          Length > 1,
                                          Length,
                                          TempRegister,
                                          gcSHADER_SHADER_DEFAULT,
                                          gcSHADER_GetOutputDefaultLocation(binary),
                                          -1,
                                          gcvFALSE,
                                          gcvFALSE,
                                          gcvNULL);



}

static gceSTATUS
_AddOutputIndexed(
IN cloCOMPILER Compiler,
IN gctCONST_STRING Name,
IN gctUINT Index,
IN gctREG_INDEX TempIndex
)
{
    gcSHADER    binary;

    gcmVERIFY_OK(cloCOMPILER_GetBinary(Compiler, &binary));
    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                      clvDUMP_CODE_EMITTER,
                      "gcSHADER_AddOutputIndexed(Shader, \"%s\", %d, %d);",
                      Name,
                      Index,
                      TempIndex));
    return gcSHADER_AddOutputIndexed(binary, Name, Index, TempIndex);
}

static gceSTATUS
_AddOpcode(
    IN cloCOMPILER Compiler,
    IN gcSL_OPCODE Opcode,
    IN gcSL_FORMAT Format,
    IN gctREG_INDEX TempRegister,
    IN gctUINT8 Enable,
    IN gctUINT32 SrcLoc
    )
{
    gcSHADER binary;
    gctCHAR  buf[5];

    gcmVERIFY_OK(cloCOMPILER_GetBinary(Compiler, &binary));

    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                                  clvDUMP_CODE_EMITTER,
                                  "gcSHADER_AddOpcode(Shader, %s, %d, gcSL_ENABLE_%s, %s);",
                                  _GetOpcodeName(Opcode),
                                  TempRegister,
                                  _GetEnableName(Enable, buf),
                                  _GetFormatName(Format)));

    return gcSHADER_AddOpcode(binary, Opcode, TempRegister, Enable, Format, gcSHADER_PRECISION_DEFAULT, SrcLoc);
}

static gceSTATUS
_AddOpcodeCondition(
    IN cloCOMPILER Compiler,
    IN gcSL_OPCODE Opcode,
    IN gcSL_CONDITION Condition,
    IN gcSL_FORMAT Format,
    IN gctREG_INDEX TempRegister,
    IN gctUINT8 Enable,
    IN gctUINT32 SrcLoc
    )
{
    gcSHADER binary;
    gctCHAR  buf[5];

    gcmVERIFY_OK(cloCOMPILER_GetBinary(Compiler, &binary));

    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                                  clvDUMP_CODE_EMITTER,
                                  "gcSHADER_AddOpcode2(Shader, %s, %s, %d, gcSL_ENABLE_%s, %s);",
                                  _GetOpcodeName(Opcode),
                                  _GetConditionName(Condition),
                                  TempRegister,
                                  _GetEnableName(Enable, buf),
                                  _GetFormatName(Format)));

    return gcSHADER_AddOpcode2(binary, Opcode, Condition, TempRegister, Enable, Format, gcSHADER_PRECISION_DEFAULT, SrcLoc);
}

static gceSTATUS
_AddOpcodeIndexed(
    IN cloCOMPILER Compiler,
    IN gcSL_OPCODE Opcode,
    IN gcSL_FORMAT Format,
    IN gctREG_INDEX TempRegister,
    IN gctUINT8 Enable,
    IN gcSL_INDEXED Mode,
    IN gctREG_INDEX IndexRegister,
    IN gctUINT32 SrcLoc
    )
{
   gcSHADER binary;
   gctCHAR  buf[5];

   gcmVERIFY_OK(cloCOMPILER_GetBinary(Compiler, &binary));

   gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                                 clvDUMP_CODE_EMITTER,
                                 "gcSHADER_AddOpcodeIndexed(Shader, %s, %d, gcSL_ENABLE_%s, %s, %d, %s);",
                                 _GetOpcodeName(Opcode),
                                 TempRegister,
                                 _GetEnableName(Enable, buf),
                                 _GetIndexModeName(Mode),
                                 IndexRegister,
                                 _GetFormatName(Format)));

    return gcSHADER_AddOpcodeIndexed(binary, Opcode, TempRegister, Enable, Mode, (gctUINT16)IndexRegister, Format, gcSHADER_PRECISION_DEFAULT, SrcLoc);
}

static gceSTATUS
_AddOpcodeConditionIndexed(
    IN cloCOMPILER Compiler,
    IN gcSL_OPCODE Opcode,
    IN gcSL_CONDITION Condition,
    IN gcSL_FORMAT Format,
    IN gctREG_INDEX TempRegister,
    IN gctUINT8 Enable,
    IN gcSL_INDEXED Mode,
    IN gctREG_INDEX IndexRegister,
    IN gctUINT32 SrcLoc
    )
{
   gcSHADER binary;
   gctCHAR  buf[5];

   gcmVERIFY_OK(cloCOMPILER_GetBinary(Compiler, &binary));

   gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                                 clvDUMP_CODE_EMITTER,
                                 "gcSHADER_AddOpcodeConditionIndexed(Shader, %s, %s, %d, gcSL_ENABLE_%s, %s, %d, %s);",
                                 _GetOpcodeName(Opcode),
                                 _GetConditionName(Condition),
                                 TempRegister,
                                 _GetEnableName(Enable, buf),
                                 _GetIndexModeName(Mode),
                                 IndexRegister,
                                 _GetFormatName(Format)));

    return gcSHADER_AddOpcodeConditionIndexed(binary, Opcode, Condition, TempRegister, Enable, Mode,
                                              (gctUINT16)IndexRegister, Format, gcSHADER_PRECISION_DEFAULT, SrcLoc);
}

static gceSTATUS
_AddRoundingMode(
    IN cloCOMPILER Compiler,
    IN gcSL_ROUND Round
    )
{
    gcSHADER binary;

    gcmVERIFY_OK(cloCOMPILER_GetBinary(Compiler, &binary));

    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                                  clvDUMP_CODE_EMITTER,
                                  "gcSHADER_AddRoundingMode(Shader, %d);",
                                  Round));

    return gcSHADER_AddRoundingMode(binary, Round);
}

static gceSTATUS
_AddSource(
    IN cloCOMPILER Compiler,
    IN gcSL_TYPE Type,
    IN gcSL_FORMAT Format,
    IN gctREG_INDEX SourceIndex,
    IN gctUINT8 Swizzle
    )
{
   gcSHADER    binary;
   gctCHAR    buf[5];

   gcmVERIFY_OK(cloCOMPILER_GetBinary(Compiler, &binary));

   gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                                 clvDUMP_CODE_EMITTER,
                                 "gcSHADER_AddSource(Shader, %s, %d, gcSL_SWIZZLE_%s, %s);",
                                 _GetTypeName(Type),
                                 SourceIndex,
                                 _GetSwizzleName(Swizzle, buf),
                                 _GetFormatName(Format)));

    return gcSHADER_AddSource(binary, Type, SourceIndex, Swizzle, Format, gcSHADER_PRECISION_DEFAULT);
}

static gceSTATUS
_AddSourceIndexed(
    IN cloCOMPILER Compiler,
    IN gcSL_TYPE Type,
    IN gcSL_FORMAT Format,
    IN gctREG_INDEX SourceIndex,
    IN gctUINT8 Swizzle,
    IN gcSL_INDEXED Mode,
    IN gctREG_INDEX IndexRegister
    )
{
   gcSHADER    binary;
   gctCHAR    buf[5];

   gcmVERIFY_OK(cloCOMPILER_GetBinary(Compiler, &binary));

   gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                                 clvDUMP_CODE_EMITTER,
                                 "gcSHADER_AddSourceIndexed(Shader, %s, %d, gcSL_SWIZZLE_%s, %s, %d, %s);",
                                 _GetTypeName(Type),
                                 SourceIndex,
                                 _GetSwizzleName(Swizzle, buf),
                                 _GetIndexModeName(Mode),
                                 IndexRegister,
                                 _GetFormatName(Format)));

    return gcSHADER_AddSourceIndexed(binary, Type, (gctUINT16)SourceIndex, Swizzle, Mode, (gctUINT16)IndexRegister, Format, gcSHADER_PRECISION_DEFAULT);
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

static gceSTATUS
_AddSourceAttribute(
    IN cloCOMPILER Compiler,
    IN gcATTRIBUTE Attribute,
    IN gcSL_FORMAT Format,
    IN gctUINT8 Swizzle,
    IN gctINT Index
    )
{
    gcSHADER    binary;
    gctCHAR     buf[5];

    gcmVERIFY_OK(cloCOMPILER_GetBinary(Compiler, &binary));

    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                      clvDUMP_CODE_EMITTER,
                      "gcSHADER_AddSourceAttributeFormatted(Shader, \"%s\","
                      " gcSL_SWIZZLE_%s, %d, %u);",
                      gcGetAttributeName(binary, Attribute),
                      _GetSwizzleName(Swizzle, buf),
                      Index,
                      Format));

    return gcSHADER_AddSourceAttributeFormatted(binary, Attribute, Swizzle, Index, Format);
}

static gceSTATUS
_AddSourceAttributeIndexed(
    IN cloCOMPILER Compiler,
    IN gcATTRIBUTE Attribute,
    IN gcSL_FORMAT Format,
    IN gctUINT8 Swizzle,
    IN gctINT Index,
    IN gcSL_INDEXED Mode,
    IN gctREG_INDEX IndexRegister
    )
{
    gcSHADER    binary;
    gctCHAR     buf[5];

    gcmVERIFY_OK(cloCOMPILER_GetBinary(Compiler, &binary));

    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                clvDUMP_CODE_EMITTER,
                "gcSHADER_AddSourceAttributeIndexedFormatted(Shader, \"%s\","
                " gcSL_SWIZZLE_%s, %d, %s, %d, %u);",
                gcGetAttributeName(binary, Attribute),
                _GetSwizzleName(Swizzle, buf),
                Index,
                _GetIndexModeName(Mode),
                IndexRegister,
                Format));

    return gcSHADER_AddSourceAttributeIndexedFormatted(binary, Attribute, Swizzle, Index, Mode, (gctUINT16)IndexRegister, Format);
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
    IN cloCOMPILER Compiler,
    IN gcUNIFORM Uniform,
    OUT gctUINT32 * Sampler
    )
{
    gcmASSERT(Uniform);

    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                      clvDUMP_CODE_EMITTER,
                      "gcUNIFORM_GetSampler(\"%s\", Sampler);",
                      gcGetUniformName(Uniform)));

    return gcUNIFORM_GetSampler(Uniform, Sampler);
}

static gceSTATUS
_AddSourceUniform(
    IN cloCOMPILER Compiler,
    IN gcUNIFORM Uniform,
    IN gcSL_FORMAT Format,
    IN gctUINT8 Swizzle,
    IN gctINT Index
    )
{
    gcSHADER    binary;
    gctCHAR        buf[5];

    gcmVERIFY_OK(cloCOMPILER_GetBinary(Compiler, &binary));

    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                      clvDUMP_CODE_EMITTER,
                      "gcSHADER_AddSourceUniform(Shader, \"%s\","
                      " gcSL_SWIZZLE_%s, %d, %u);",
                      gcGetUniformName(Uniform),
                      _GetSwizzleName(Swizzle, buf),
                      Index,
                      Format));

    return gcSHADER_AddSourceUniformFormatted(binary, Uniform, Swizzle, Index, Format);
}

static gceSTATUS
_AddSourceUniformIndexed(
    IN cloCOMPILER Compiler,
    IN gcUNIFORM Uniform,
    IN gcSL_FORMAT Format,
    IN gctUINT8 Swizzle,
    IN gctINT Index,
    IN gcSL_INDEXED Mode,
    IN gctREG_INDEX IndexRegister
    )
{
    gcSHADER    binary;
    gctCHAR        buf[5];

    gcmVERIFY_OK(cloCOMPILER_GetBinary(Compiler, &binary));

    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                      clvDUMP_CODE_EMITTER,
                      "gcSHADER_AddSourceUniformIndexed(Shader, \"%s\","
                      " gcSL_SWIZZLE_%s, %d, %s, %d, %u);",
                      gcGetUniformName(Uniform),
                      _GetSwizzleName(Swizzle, buf),
                      Index,
                      _GetIndexModeName(Mode),
                      IndexRegister,
                      Format));

    return gcSHADER_AddSourceUniformIndexedFormatted(binary, Uniform, Swizzle, Index, Mode, (gctUINT16)IndexRegister, Format);
}

static gceSTATUS
_AddSourceConstant(
    IN cloCOMPILER Compiler,
    IN void *Constant,
    IN gcSL_FORMAT Format
    )
{
   gcSHADER    binary;

   gcmVERIFY_OK(cloCOMPILER_GetBinary(Compiler, &binary));

   gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                clvDUMP_CODE_EMITTER,
                "gcSHADER_AddSourceConstantFormatted(Shader, 0x%x, \"%s\");",
                Constant, _GetFormatName(Format)));

   return gcSHADER_AddSourceConstantFormatted(binary, Constant, Format);
}

static gceSTATUS
_AddLabel(
    IN cloCOMPILER Compiler,
    IN gctUINT Label
    )
{
    gcSHADER    binary;

    gcmVERIFY_OK(cloCOMPILER_GetBinary(Compiler, &binary));

    gcmVERIFY_OK(cloCOMPILER_Dump(
                                Compiler,
                                clvDUMP_CODE_EMITTER,
                                "gcSHADER_AddLabel(Shader, %d);",
                                Label));

    return gcSHADER_AddLabel(binary, Label);
}

static gceSTATUS
_AddOpcodeConditionalFormatted(
    IN cloCOMPILER Compiler,
    IN gcSL_OPCODE Opcode,
    IN gcSL_CONDITION Condition,
    IN gcSL_FORMAT Format,
    IN gctUINT Label,
    IN gctUINT32 SrcLoc
    )
{
    gcSHADER    binary;

    gcmVERIFY_OK(cloCOMPILER_GetBinary(Compiler, &binary));

    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                      clvDUMP_CODE_EMITTER,
                      "gcSHADER_AddOpcodeConditionalFormatted(Shader, %s, %s, %s, %d);",
                      _GetOpcodeName(Opcode),
                      _GetConditionName(Condition),
                      _GetFormatName(Format),
                      Label));

    return gcSHADER_AddOpcodeConditionalFormatted(binary, Opcode, Condition, Format, Label, SrcLoc);
}

static gceSTATUS
_AddOpcodeConditional(
    IN cloCOMPILER Compiler,
    IN gcSL_OPCODE Opcode,
    IN gcSL_CONDITION Condition,
    IN gctUINT Label,
    IN gctUINT32 SrcLoc
    )
{
    gcSHADER    binary;

    gcmVERIFY_OK(cloCOMPILER_GetBinary(Compiler, &binary));

    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                      clvDUMP_CODE_EMITTER,
                      "gcSHADER_AddOpcodeConditional(Shader, %s, %s, %d);",
                      _GetOpcodeName(Opcode),
                      _GetConditionName(Condition),
                      Label));

    return gcSHADER_AddOpcodeConditional(binary, Opcode, Condition, Label, SrcLoc);
}

static gceSTATUS
_AddFunction(
    IN cloCOMPILER Compiler,
    IN gctCONST_STRING Name,
    OUT gcFUNCTION * Function
    )
{
    gcSHADER    binary;

    gcmASSERT(Function);

    gcmVERIFY_OK(cloCOMPILER_GetBinary(Compiler, &binary));

    gcmVERIFY_OK(cloCOMPILER_Dump(
                                Compiler,
                                clvDUMP_CODE_EMITTER,
                                "gcSHADER_AddFunction(Shader, \"%s\");",
                                Name));

    return gcSHADER_AddFunction(binary, Name, Function);
}

static gceSTATUS
_BeginFunction(
    IN cloCOMPILER Compiler,
    IN gcFUNCTION Function
    )
{
    gcSHADER    binary;

    gcmVERIFY_OK(cloCOMPILER_GetBinary(Compiler, &binary));

    gcmVERIFY_OK(cloCOMPILER_Dump(
                                Compiler,
                                clvDUMP_CODE_EMITTER,
                                "gcSHADER_BeginFunction(Shader);"));

    return gcSHADER_BeginFunction(binary, Function);
}

static gceSTATUS
_EndFunction(
    IN cloCOMPILER Compiler,
    IN gcFUNCTION Function
    )
{
    gcSHADER    binary;

    gcmVERIFY_OK(cloCOMPILER_GetBinary(Compiler, &binary));

    gcmVERIFY_OK(cloCOMPILER_Dump(
                                Compiler,
                                clvDUMP_CODE_EMITTER,
                                "gcSHADER_EndFunction(Shader);"));

    return gcSHADER_EndFunction(binary, Function);
}

static gceSTATUS
_AddKernelFunction(
    IN cloCOMPILER Compiler,
    IN gctCONST_STRING Name,
    OUT gcKERNEL_FUNCTION * Function
    )
{
    gcSHADER    binary;

    gcmASSERT(Function);

    gcmVERIFY_OK(cloCOMPILER_GetBinary(Compiler, &binary));

    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                      clvDUMP_CODE_EMITTER,
                      "gcSHADER_AddKernelFunction(Shader, \"%s\");",
                      Name));

    return gcSHADER_AddKernelFunction(binary, Name, Function);
}

static gceSTATUS
_BeginKernelFunction(
    IN cloCOMPILER Compiler,
    IN gcKERNEL_FUNCTION Function
    )
{
    gcSHADER    binary;

    gcmVERIFY_OK(cloCOMPILER_GetBinary(Compiler, &binary));

    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                      clvDUMP_CODE_EMITTER,
                      "gcSHADER_BeginKernelFunction(Shader);"));

    return gcSHADER_BeginKernelFunction(binary, Function);
}

static gceSTATUS
_EndKernelFunction(
    IN cloCOMPILER Compiler,
    IN gcKERNEL_FUNCTION Function,
    IN gctSIZE_T LocalMemorySize
    )
{
    gcSHADER    binary;

    gcmVERIFY_OK(cloCOMPILER_GetBinary(Compiler, &binary));

    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                      clvDUMP_CODE_EMITTER,
                      "gcSHADER_EndKernelFunction(Shader);"));

    return gcSHADER_EndKernelFunction(binary, Function, LocalMemorySize);
}

static gceSTATUS
_AddFunctionArgument(
    IN cloCOMPILER Compiler,
    IN gcFUNCTION Function,
    IN gctUINT16 VariableIndex,
    IN gctREG_INDEX TempIndex,
    IN gctUINT8 Enable,
    IN gctUINT8 Qualifier
    )
{
    gctCHAR        buf[5];

    gcmVERIFY_OK(cloCOMPILER_Dump(
                                Compiler,
                                clvDUMP_CODE_EMITTER,
                                "gcFUNCTION_AddArgument(Function, %d, gcSL_ENABLE_%s, %s);",
                                TempIndex,
                                _GetEnableName(Enable, buf),
                                _GetQualifierName((gceINPUT_OUTPUT) Qualifier)));

    return gcFUNCTION_AddArgument(Function, VariableIndex, TempIndex, Enable, Qualifier, gcSHADER_PRECISION_DEFAULT, gcvFALSE);
}

static gceSTATUS
_AddKernelFunctionArgument(
    IN cloCOMPILER Compiler,
    IN gcKERNEL_FUNCTION Function,
    IN gctUINT16 VariableIndex,
    IN gctREG_INDEX TempIndex,
    IN gctUINT8 Enable,
    IN gctUINT8 Qualifier
    )
{
    gctCHAR        buf[5];

    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                      clvDUMP_CODE_EMITTER,
                      "gcKERNEL_FUNCTION_AddArgument(Function, %d, gcSL_ENABLE_%s, %s);",
                      TempIndex,
                      _GetEnableName(Enable, buf),
                      _GetQualifierName((gceINPUT_OUTPUT) Qualifier)));

    return gcKERNEL_FUNCTION_AddArgument(Function, VariableIndex, TempIndex, Enable, Qualifier);
}

gceSTATUS
_GetKernelFunctionLabel(
    IN cloCOMPILER Compiler,
    IN gcKERNEL_FUNCTION Function,
    OUT gctUINT_PTR Label
    )
{
    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                      clvDUMP_CODE_EMITTER,
                      "gcKERNEL_FUNCTION_GetLabel(Function, Label);"));

    return gcKERNEL_FUNCTION_GetLabel(Function, Label);
}

gceSTATUS
_GetFunctionLabel(
    IN cloCOMPILER Compiler,
    IN gcFUNCTION Function,
    OUT gctUINT_PTR Label
    )
{
    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                      clvDUMP_CODE_EMITTER,
                      "gcFUNCTION_GetLabel(Function, Label);"));

    return gcFUNCTION_GetLabel(Function, Label);
}

gcSL_FORMAT
clConvDataTypeToFormat(
clsGEN_CODE_DATA_TYPE DataType
)
{
    switch (clmGEN_CODE_elementType_GET(DataType)) {
    case clvTYPE_BOOL:
    case clvTYPE_BOOL_PACKED:
        return gcSL_BOOLEAN;

    case clvTYPE_INT:
        return gcSL_INTEGER;

    case clvTYPE_UINT:
        return gcSL_UINT32;

    case clvTYPE_VOID:
        return gcSL_VOID;

    case clvTYPE_LONG:
        return gcSL_INT64;

    case clvTYPE_ULONG:
        return gcSL_UINT64;

    case clvTYPE_USHORT:
    case clvTYPE_USHORT_PACKED:
        return gcSL_UINT16;

    case clvTYPE_SHORT:
    case clvTYPE_SHORT_PACKED:
        return gcSL_INT16;

    case clvTYPE_UCHAR:
    case clvTYPE_UCHAR_PACKED:
        return gcSL_UINT8;

    case clvTYPE_CHAR:
    case clvTYPE_CHAR_PACKED:
        return gcSL_INT8;

    case clvTYPE_FLOAT:
        return gcSL_FLOAT;

    case clvTYPE_HALF:
    case clvTYPE_HALF_PACKED:
        return gcSL_FLOAT16;

    case clvTYPE_DOUBLE:
        return gcSL_FLOAT64;

    case clvTYPE_EVENT_T:
        return gcSL_UINT32;

    case clvTYPE_SAMPLER_T:
    case clvTYPE_IMAGE2D_T:
    case clvTYPE_IMAGE3D_T:
    case clvTYPE_IMAGE1D_T:
    case clvTYPE_IMAGE1D_ARRAY_T:
    case clvTYPE_IMAGE1D_BUFFER_T:
    case clvTYPE_IMAGE2D_ARRAY_T:
    case clvTYPE_VIV_GENERIC_GL_SAMPLER:
    case clvTYPE_VIV_GENERIC_IMAGE_T:
        return gcSL_UINT32;

    case clvTYPE_VIV_GENERIC_GL_IMAGE:
        return gcSL_UINT64;

    case clvTYPE_SAMPLER2D:
    case clvTYPE_SAMPLER3D:
        return gcSL_FLOAT;

    case clvTYPE_UNION:
    case clvTYPE_STRUCT:
        return gcSL_INT8;

    case clvTYPE_GEN_PACKED:
        return gcSL_UINT32;

    default:
        gcmASSERT(0);
        return gcSL_FLOAT;
    }
}

static gceSTATUS
_EmitOpcodeAndTarget(
    IN cloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gcSL_OPCODE Opcode,
    IN gcsTARGET * Target
    )
{
    gceSTATUS    status;
    gcSL_FORMAT format;
    gctUINT32 srcLoc;

    gcmASSERT(Target);

    format = clConvDataTypeToFormat(Target->dataType);

    srcLoc = GCSL_Build_SRC_LOC(LineNo, StringNo);

    if (Target->indexMode == gcSL_NOT_INDEXED) {
        status = _AddOpcode(Compiler,
                            Opcode,
                            format,
                            Target->tempRegIndex,
                            Target->enable,
                            srcLoc);
    }
    else {
        status = _AddOpcodeIndexed(Compiler,
                                   Opcode,
                                   format,
                                   Target->tempRegIndex,
                                   Target->enable,
                                   Target->indexMode,
                                   Target->indexRegIndex,
                                   srcLoc);
    }

    if(clmGEN_CODE_IsExtendedVectorType(Target->dataType))
    {
        gcSHADER binary;
        gcmVERIFY_OK(cloCOMPILER_GetBinary(Compiler, &binary));
        gcSHADER_UpdateTargetPacked(binary,
                                    clmGEN_CODE_vectorSize_GET(Target->dataType));
    }

    if (gcmIS_ERROR(status)) {
        cloCOMPILER_Report(Compiler,
                           LineNo,
                           StringNo,
                           clvREPORT_INTERNAL_ERROR,
                           "failed to add the opcode");
        return status;
    }

    return gcvSTATUS_OK;
}

static gceSTATUS
_EmitOpcodeConditionAndTarget(
    IN cloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gcSL_OPCODE Opcode,
    IN gcSL_CONDITION Condition,
    IN gcsTARGET * Target
    )
{
    gceSTATUS    status;
    gcSL_FORMAT format;
    gctUINT32 srcLoc;

    gcmASSERT(Target);

    srcLoc = GCSL_Build_SRC_LOC(LineNo, StringNo);
    format = clConvDataTypeToFormat(Target->dataType);

    if (Target->indexMode == gcSL_NOT_INDEXED) {
        status = _AddOpcodeCondition(Compiler,
                                     Opcode,
                                     Condition,
                                     format,
                                     Target->tempRegIndex,
                                     Target->enable,
                                     srcLoc);
    }
    else {
        status = _AddOpcodeConditionIndexed(Compiler,
                                            Opcode,
                                            Condition,
                                            format,
                                            Target->tempRegIndex,
                                            Target->enable,
                                            Target->indexMode,
                                            Target->indexRegIndex,
                                            srcLoc);
    }

    if(clmGEN_CODE_IsExtendedVectorType(Target->dataType))
    {
        gcSHADER binary;
        gcmVERIFY_OK(cloCOMPILER_GetBinary(Compiler, &binary));
        gcSHADER_UpdateTargetPacked(binary,
                                    clmGEN_CODE_vectorSize_GET(Target->dataType));
    }

    if (gcmIS_ERROR(status)) {
        cloCOMPILER_Report(Compiler,
                           LineNo,
                           StringNo,
                           clvREPORT_INTERNAL_ERROR,
                           "failed to add the opcode");

        return status;
    }

    return gcvSTATUS_OK;
}

static gceSTATUS
_EmitOpcodeConditional(
    IN cloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gcSL_OPCODE Opcode,
    IN gcSL_CONDITION Condition,
    IN clsGEN_CODE_DATA_TYPE *DataType,
    IN gctLABEL Label
    )
{
    gceSTATUS    status;
    gctUINT32 srcLoc;

    srcLoc = GCSL_Build_SRC_LOC(LineNo, StringNo);

    if(DataType) {
       status = _AddOpcodeConditionalFormatted(Compiler,
                                               Opcode,
                                               Condition,
                                               clConvDataTypeToFormat(*DataType),
                                               Label,
                                               srcLoc);
    }
    else {
       status = _AddOpcodeConditional(Compiler,
                                      Opcode,
                                      Condition,
                                      Label,
                                      srcLoc);
    }

    if (gcmIS_ERROR(status)) {
       cloCOMPILER_Report(Compiler,
                          LineNo,
                          StringNo,
                          clvREPORT_INTERNAL_ERROR,
                          "failed to add the conditional opcode");
       return status;
    }

    return gcvSTATUS_OK;
}

static gceSTATUS
_EmitSourceTemp(
    IN cloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN clsGEN_CODE_DATA_TYPE DataType,
    IN gcsSOURCE_REG * SourceReg
    )
{
   gceSTATUS status;
   gcSL_FORMAT format;
   gcSHADER binary;
   gctCHAR buf[5];

   gcmASSERT(SourceReg);

   format = clConvDataTypeToFormat(DataType);
   gcmVERIFY_OK(cloCOMPILER_GetBinary(Compiler, &binary));

   if (gcIsSamplerDataType(DataType)) {
        gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                                      clvDUMP_CODE_EMITTER,
                                      "gcSHADER_AddSourceSamplerIndexedFormatted(Shader,"
                                      " gcSL_SWIZZLE_%s, %s, %d, %s);",
                                      _GetSwizzleName(gcSL_SWIZZLE_XYZW, buf),
                                      _GetIndexModeName(_ConvSwizzleToIndexMode(SourceReg->swizzle)),
                                      SourceReg->regIndex,
                                      _GetFormatName(format)));

        status = gcSHADER_AddSourceSamplerIndexedFormatted(binary,
                                                           gcSL_SWIZZLE_XYZW,
                                                           _ConvSwizzleToIndexMode(SourceReg->swizzle),
                                                           (gctUINT16)SourceReg->regIndex,
                                                           format);
   }
   else {
       if (SourceReg->indexMode == gcSL_NOT_INDEXED) {
              status = _AddSource(Compiler,
                                  gcSL_TEMP,
                                  format,
                                  SourceReg->regIndex,
                                  SourceReg->swizzle);
       }
       else {
         status = _AddSourceIndexed(Compiler,
                                        gcSL_TEMP,
                                        format,
                                        SourceReg->regIndex,
                                        SourceReg->swizzle,
                                        SourceReg->indexMode,
                                        SourceReg->indexRegIndex);
       }
   }

   if (gcmIS_ERROR(status)) {
       cloCOMPILER_Report(Compiler,
                          LineNo,
                          StringNo,
                          clvREPORT_INTERNAL_ERROR,
                          "failed to add the source");
       return status;
   }

    return gcvSTATUS_OK;
}

static gceSTATUS
_EmitSourceAttribute(
    IN cloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN clsGEN_CODE_DATA_TYPE DataType,
    IN gcsSOURCE_REG * SourceReg
    )
{
    gceSTATUS  status;
    gcSL_FORMAT format;

    gcmASSERT(SourceReg);

    format = clConvDataTypeToFormat(DataType);

    if (SourceReg->indexMode == gcSL_NOT_INDEXED)
    {
        status = _AddSourceAttribute(Compiler,
                         SourceReg->u.attribute,
                         format,
                         SourceReg->swizzle,
                         SourceReg->regIndex);

    }
    else
    {
        status = _AddSourceAttributeIndexed(Compiler,
                                            SourceReg->u.attribute,
                                            format,
                                            SourceReg->swizzle,
                                            SourceReg->regIndex,
                                            SourceReg->indexMode,
                                            SourceReg->indexRegIndex);
    }

    if (gcmIS_ERROR(status))
    {
        gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                        LineNo,
                        StringNo,
                        clvREPORT_INTERNAL_ERROR,
                        "failed to add the source attribute"));

            return status;
    }

    return gcvSTATUS_OK;
}

static gceSTATUS
_EmitSourceUniform(
    IN cloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN clsGEN_CODE_DATA_TYPE DataType,
    IN gcsSOURCE_REG * SourceReg
    )
{
    gceSTATUS    status;

    gcSL_FORMAT format;

    gcmASSERT(SourceReg);

    format = clConvDataTypeToFormat(DataType);

    if (SourceReg->indexMode == gcSL_NOT_INDEXED)
    {
        status = _AddSourceUniform(Compiler,
                       SourceReg->u.uniform,
                       format,
                       SourceReg->swizzle,
                       SourceReg->regIndex);

    }
    else
    {
        status = _AddSourceUniformIndexed(Compiler,
                          SourceReg->u.uniform,
                          format,
                          SourceReg->swizzle,
                          SourceReg->regIndex,
                          SourceReg->indexMode,
                          SourceReg->indexRegIndex);
    }

    if (gcmIS_ERROR(status))
    {
        gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                        LineNo,
                        StringNo,
                        clvREPORT_INTERNAL_ERROR,
                        "failed to add the source uniform"));

            return status;
    }

    return gcvSTATUS_OK;
}

static gceSTATUS
_EmitSourceConstant(
    IN cloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN void * Constant,
    IN gcSL_FORMAT Format
    )
{
    gceSTATUS status;

    status = _AddSourceConstant(Compiler, Constant, Format);

    if (gcmIS_ERROR(status)) {
    gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                    LineNo,
                    StringNo,
                    clvREPORT_INTERNAL_ERROR,
                    "failed to add the source constant"));
        return status;
    }

    return gcvSTATUS_OK;
}

static gceSTATUS
_EmitSource(
    IN cloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gcsSOURCE * Source
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gcSHADER binary = gcvNULL;
    gcSHADER_INSTRUCTION_INDEX instrIndex = gcSHADER_OPCODE;
    gcmASSERT(Source);

    if(clmGEN_CODE_IsExtendedVectorType(Source->dataType))
    {
        gcmVERIFY_OK(cloCOMPILER_GetBinary(Compiler, &binary));
        instrIndex = binary->instrIndex;
    }

    switch (Source->type) {
    case gcvSOURCE_TEMP:
        status = _EmitSourceTemp(Compiler,
                                 LineNo,
                                 StringNo,
                                 Source->dataType,
                                 &Source->u.sourceReg);
        break;

    case gcvSOURCE_ATTRIBUTE:
        status = _EmitSourceAttribute(Compiler,
                                      LineNo,
                                      StringNo,
                                      Source->dataType,
                                      &Source->u.sourceReg);
        break;

    case gcvSOURCE_UNIFORM:
        status =  _EmitSourceUniform(Compiler,
                                     LineNo,
                                     StringNo,
                                     Source->dataType,
                                     &Source->u.sourceReg);
        break;

    case gcvSOURCE_CONSTANT:
        {
            void * constantPtr;
            gctFLOAT floatConstant[1];
            gctINT32 intConstant[1];
            gctUINT32 uintConstant[1];
            gctINT64 longConstant[1];
            gctUINT64 ulongConstant[1];
            gcSL_FORMAT format;
            cltELEMENT_TYPE elementType;

            elementType = clmGEN_CODE_elementType_GET(Source->dataType);
            format = clConvDataTypeToFormat(Source->dataType);

            if(clmIsElementTypeFloating(elementType)) {
                floatConstant[0] = Source->u.sourceConstant.floatValue;
                constantPtr = (void *)floatConstant;
            }
            else if(clmIsElementTypeBoolean(elementType)) {
                intConstant[0] = Source->u.sourceConstant.boolValue;
                constantPtr = (void *)intConstant;
            }
            else if(clmIsElementTypeInteger(elementType)) {
                if(clmIsElementTypeUnsigned(elementType)) {
                    if (format == gcSL_UINT64)
                    {
                        ulongConstant[0] = Source->u.sourceConstant.ulongValue;
                        constantPtr = (void *)ulongConstant;
                    }
                    else
                    {
                        uintConstant[0] = Source->u.sourceConstant.uintValue;
                        constantPtr = (void *)uintConstant;
                    }
                }
                else {
                    if (format == gcSL_INT64)
                    {
                        longConstant[0] = Source->u.sourceConstant.longValue;
                        constantPtr = (void *)longConstant;
                    }
                    else
                    {
                        intConstant[0] = Source->u.sourceConstant.intValue;
                        constantPtr = (void *)intConstant;
                    }
                }
            }
            else if(clmIsElementTypeEvent(elementType) ||
                    clmIsElementTypeSampler(elementType)) {
                uintConstant[0] = Source->u.sourceConstant.uintValue;
                constantPtr = (void *)uintConstant;
            }
            else {
                gcmASSERT(0);
                return gcvSTATUS_INVALID_ARGUMENT;
            }

            status =  _EmitSourceConstant(Compiler,
                                          LineNo,
                                          StringNo,
                                          constantPtr,
                                          format);
            break;
        }

    case gcvSOURCE_TARGET_FORMAT:
        {
            gctUINT32 format[1];

            format[0] = (gctUINT32)clConvDataTypeToFormat(Source->dataType);
            status =  _EmitSourceConstant(Compiler,
                                          LineNo,
                                          StringNo,
                                          (void *)format,
                                          gcSL_UINT32);
            break;

        }

    default:
        gcmASSERT(0);
        return gcvSTATUS_INVALID_ARGUMENT;
    }

    if (gcmIS_ERROR(status)) return status;

    if (instrIndex != gcSHADER_OPCODE)
    {
        gcmASSERT(binary);
        status =  gcSHADER_UpdateSourcePacked(binary,
                                              instrIndex,
                                              clmGEN_CODE_vectorSize_GET(Source->dataType));
    }

    return status;
}

gceSTATUS
clNewAttribute(
    IN cloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gctCONST_STRING Name,
    IN clsGEN_CODE_DATA_TYPE DataType,
    IN gctSIZE_T Length,
    IN gctBOOL IsTexture,
    OUT gcATTRIBUTE * Attribute
    )
{
    gceSTATUS status;
    gcsTYPE_SIZE typeSize;
    gcSHADER_TYPE type;

    typeSize = clConvToShaderDataType(Compiler, DataType);
    type = typeSize.type;

    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                      clvDUMP_CODE_EMITTER,
                      "<ATTRIBUTE line=\"%d\" string=\"%d\" name=\"%s\""
                      " dataType=\"%s\" length=\"%d\">",
                      LineNo,
                      StringNo,
                      Name,
                      gcGetShaderDataTypeName(type),
                      Length * typeSize.length));

    status = _AddAttribute(Compiler,
                Name,
                type,
                Length * typeSize.length,
                IsTexture,
                Attribute);

    if (gcmIS_ERROR(status)) {
       gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                       LineNo,
                       StringNo,
                       clvREPORT_INTERNAL_ERROR,
                       "failed to add the attribute"));
           return status;
    }

    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                      clvDUMP_CODE_EMITTER,
                      "</ATTRIBUTE>"));

    return gcvSTATUS_OK;
}

static gceUNIFORM_FLAGS
_GetUniformFlags(
gctINT Flags,
clsNAME *ParamName
)
{
   switch(Flags) {
   case clvBUILTIN_KERNEL_ARG:
      if(ParamName) {
          cltQUALIFIER addrSpaceQualifier;

          addrSpaceQualifier = clGetAddrSpaceQualifier(&ParamName->decl);

          if(addrSpaceQualifier == clvQUALIFIER_CONSTANT) {
             return gcvUNIFORM_KIND_KERNEL_ARG_CONSTANT;
          }
          else if(addrSpaceQualifier == clvQUALIFIER_LOCAL) {
             return gcvUNIFORM_KIND_KERNEL_ARG_LOCAL;
          }
          else if((addrSpaceQualifier == clvQUALIFIER_NONE
               || addrSpaceQualifier == clvQUALIFIER_PRIVATE)
               && ParamName->u.variableInfo.isAddressed) {
             return gcvUNIFORM_KIND_KERNEL_ARG_PRIVATE;
          }
          if(ParamName->decl.dataType->elementType == clvTYPE_SAMPLER_T) {
             return gcvUNIFORM_KIND_KERNEL_ARG_SAMPLER;
          }
      }
      return gcvUNIFORM_KIND_KERNEL_ARG;

   case clvBUILTIN_WORK_DIM:
      return gcvUNIFORM_KIND_WORK_DIM;

   case clvBUILTIN_GLOBAL_SIZE:
      return gcvUNIFORM_KIND_GLOBAL_SIZE;

   case clvBUILTIN_GLOBAL_WORK_SCALE:
       return gcvUNIFORM_KIND_GLOBAL_WORK_SCALE;

   case    clvBUILTIN_LOCAL_SIZE:
      return gcvUNIFORM_KIND_LOCAL_SIZE;

   case    clvBUILTIN_NUM_GROUPS:
      return gcvUNIFORM_KIND_NUM_GROUPS;

   case    clvBUILTIN_NUM_GROUPS_FOR_SINGLE_GPU:
      return gcvUNIFORM_KIND_NUM_GROUPS_FOR_SINGLE_GPU;

   case    clvBUILTIN_GLOBAL_OFFSET:
      return gcvUNIFORM_KIND_GLOBAL_OFFSET;

   case clvBUILTIN_LOCAL_ADDRESS_SPACE:
      return gcvUNIFORM_KIND_LOCAL_ADDRESS_SPACE;

   case clvBUILTIN_CONSTANT_ADDRESS_SPACE:
      return gcvUNIFORM_KIND_CONSTANT_ADDRESS_SPACE;

   case clvBUILTIN_PRIVATE_ADDRESS_SPACE:
      return gcvUNIFORM_KIND_PRIVATE_ADDRESS_SPACE;

   case clvBUILTIN_ARG_LOCAL_MEM_SIZE:
      return gcvUNIFORM_KIND_KERNEL_ARG_LOCAL_MEM_SIZE;

   case clvBUILTIN_PRINTF_ADDRESS:
       return gcvUNIFORM_KIND_PRINTF_ADDRESS;

   case clvBUILTIN_WORKITEM_PRINTF_BUFFER_SIZE:
       return gcvUNIFORM_KIND_WORKITEM_PRINTF_BUFFER_SIZE;
   }
   return gcvUNIFORM_KIND_NONE;
}

gceSTATUS
clNewUniform(
    IN cloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gctCONST_STRING Name,
    IN clsGEN_CODE_DATA_TYPE DataType,
    IN clsGEN_CODE_DATA_TYPE Format,
    IN gctUINT Flags,
    IN gctBOOL IsPointer,
    IN gctSIZE_T Length,
    IN clsARRAY *Array,
    OUT gcUNIFORM * Uniform
    )
{
    gceSTATUS status;
    gceUNIFORM_FLAGS flags;
    gcSL_FORMAT format;
    gcsTYPE_SIZE typeSize;
    gcSHADER_TYPE type;
    gcSHADER binary;
    gctINT length;

    typeSize = clConvToShaderDataType(Compiler, DataType);
    type = typeSize.type;
    format = clConvDataTypeToFormat(Format);
    flags = _GetUniformFlags(Flags, gcvNULL);

    length = Length * typeSize.length;
    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                      clvDUMP_CODE_EMITTER,
                      "<UNIFORM line=\"%d\" string=\"%d\" name=\"%s\""
                      " dataType=\"%s\"  format=\"%s\" length=\"%d\">",
                      LineNo,
                      StringNo,
                      Name,
                      gcGetShaderDataTypeName(type),
                      _GetFormatName(format),
                      length));

    gcmASSERT(Uniform);
    gcmVERIFY_OK(cloCOMPILER_GetBinary(Compiler, &binary));
    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                      clvDUMP_CODE_EMITTER,
                      "gcSHADER_AddUniform(Shader, \"%s\", %s, %d);",
                      Name,
                      gcGetShaderDataTypeName(type),
                      length));

    status = gcSHADER_AddUniform(binary,
                                 Name,
                                 type,
                                 length,
                                 gcSHADER_PRECISION_DEFAULT,
                                 Uniform);
    if (gcmIS_ERROR(status)) {
        gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                        LineNo,
                                        StringNo,
                                        clvREPORT_INTERNAL_ERROR,
                                        "failed to add the uniform"));
        return status;
    }

    /* update arrayLengthCount and arrayLengthList. */
    if (Array && Array->numDim)
    {
        gctPOINTER pointer;
        clsARRAY array = *Array;

        /* Put the compiler-generated array length to the bottom of arrayLengthList. */
        if (typeSize.length > 1)
        {
            gcmASSERT(Array->numDim < cldMAX_ARRAY_DIMENSION);
            array.length[Array->numDim] = typeSize.length;
            array.numDim = Array->numDim + 1;
        }

        if ((*Uniform)->arrayLengthList != gcvNULL)
        {
            gcmOS_SAFE_FREE(gcvNULL, (*Uniform)->arrayLengthList);
        }

        (*Uniform)->arrayLengthCount = array.numDim;
        status = gcoOS_Allocate(gcvNULL, gcmSIZEOF(gctINT) * array.numDim, &pointer);
        if(gcmIS_ERROR(status)) return status;

        gcoOS_MemCopy(pointer,
                      array.length,
                      gcmSIZEOF(gctINT) * array.numDim);
        (*Uniform)->arrayLengthList = (gctINT *)pointer;
    }
    else
    {
        ResetUniformFlag(*Uniform, gcvUNIFORM_FLAG_IS_ARRAY);
    }

    SetUniformFlag(*Uniform, flags);

    SetUniformVectorSize(*Uniform, clmGEN_CODE_vectorSize_GET(Format));
    status = gcUNIFORM_SetFormat(*Uniform, format, IsPointer);
    if(gcmIS_ERROR(status)) return status;

    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                    clvDUMP_CODE_EMITTER,
                    "</UNIFORM>"));

    return gcvSTATUS_OK;
}

static gceSTATUS
_AddKernelUniformArgument(
IN cloCOMPILER Compiler,
IN gcKERNEL_FUNCTION KernelFunction,
IN gctCONST_STRING Name,
IN gcSHADER_TYPE Type,
IN gcSL_FORMAT Format,
gceUNIFORM_FLAGS Flags,
IN gctBOOL IsPointer,
IN gctSIZE_T Length,
OUT gcUNIFORM * UniformArgument
)
{
    gceSTATUS status;

    gcmASSERT(KernelFunction);
    gcmASSERT(UniformArgument);
    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                      clvDUMP_CODE_EMITTER,
                      "gcKERNEL_FUNCTION_AddUniformArgument(KernelFunction, \"%s\", %s, %d);",
                      Name,
                      gcGetShaderDataTypeName(Type),
                      Length));

    status = gcKERNEL_FUNCTION_AddUniformArgument(KernelFunction,
                                                      Name,
                                                      Type,
                                                      Length,
                                                      UniformArgument);
    if(gcmIS_ERROR(status)) return status;

    status = gcUNIFORM_SetFlags(*UniformArgument, Flags);
    if(gcmIS_ERROR(status)) return status;

    return gcUNIFORM_SetFormat(*UniformArgument, Format, IsPointer);
}

gceSTATUS
clNewKernelUniformArgument(
    IN cloCOMPILER Compiler,
    IN clsNAME *KernelFuncName,
    IN gctCONST_STRING Name,
    IN clsGEN_CODE_DATA_TYPE DataType,
    IN clsGEN_CODE_DATA_TYPE Format,
    IN clsNAME *ParamName,
    IN gctSIZE_T Length,
    IN clsARRAY *Array,
    OUT gcUNIFORM * UniformArgument
    )
{
    gceSTATUS status;
    gcSL_FORMAT format;
    gceUNIFORM_FLAGS flags;
    gcsTYPE_SIZE typeSize;
    gcSHADER_TYPE type;

    typeSize = clConvToShaderDataType(Compiler, DataType);
    type = typeSize.type;
    format = clConvDataTypeToFormat(Format);
    flags = _GetUniformFlags(clvBUILTIN_KERNEL_ARG, ParamName);

    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                      clvDUMP_CODE_EMITTER,
                      "<UNIFORM line=\"%d\" string=\"%d\" name=\"%s\""
                      " dataType=\"%s\"  format=\"%s\" length=\"%d\">",
                      KernelFuncName->lineNo,
                      KernelFuncName->stringNo,
                      Name,
                      gcGetShaderDataTypeName(type),
                      _GetFormatName(format),
                      Length * typeSize.length));

    status = _AddKernelUniformArgument(Compiler,
                                       KernelFuncName->context.u.variable.u.kernelFunction,
                                       Name,
                                       type,
                                       format,
                                       flags,
                                       clmDECL_IsPointerType(&ParamName->decl) ||
                                       clmGEN_CODE_checkVariableForMemory(ParamName) ||
                                       clmDATA_TYPE_IsImage(ParamName->decl.dataType),
                                       Length * typeSize.length,
                                       UniformArgument);

    if (gcmIS_ERROR(status)) {
        gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                        KernelFuncName->lineNo,
                                        KernelFuncName->stringNo,
                                        clvREPORT_INTERNAL_ERROR,
                                        "failed to add the kernel uniform argument"));

        return status;
    }

    /* update arrayLengthCount and arrayLengthList. */
    if (Array && Array->numDim)
    {
        gctPOINTER pointer;
        clsARRAY array = *Array;

        /* Put the compiler-generated array length to the bottom of arrayLengthList. */
        if (typeSize.length > 1)
        {
            gcmASSERT(Array->numDim < cldMAX_ARRAY_DIMENSION);
            array.length[Array->numDim] = typeSize.length;
            array.numDim = Array->numDim + 1;
        }

        if ((*UniformArgument)->arrayLengthList != gcvNULL)
        {
            gcmOS_SAFE_FREE(gcvNULL, (*UniformArgument)->arrayLengthList);
        }

        (*UniformArgument)->arrayLengthCount = array.numDim;
        status = gcoOS_Allocate(gcvNULL, gcmSIZEOF(gctINT) * array.numDim, &pointer);
        if(gcmIS_ERROR(status)) return status;

        gcoOS_MemCopy(pointer,
                      array.length,
                      gcmSIZEOF(gctINT) * array.numDim);
        (*UniformArgument)->arrayLengthList = (gctINT *)pointer;
    }
    else
    {
        ResetUniformFlag(*UniformArgument, gcvUNIFORM_FLAG_IS_ARRAY);
    }

    SetUniformVectorSize(*UniformArgument, clmGEN_CODE_vectorSize_GET(Format));

    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                    clvDUMP_CODE_EMITTER,
                    "</UNIFORM>"));

    return gcvSTATUS_OK;
}

gceSTATUS
clGetUniformSamplerIndex(
    IN cloCOMPILER Compiler,
    IN gcUNIFORM UniformSampler,
    OUT gctREG_INDEX * Index
    )
{
    gceSTATUS    status;
    gctUINT32    sampler;

    status = _GetSampler(Compiler, UniformSampler, &sampler);

    if (gcmIS_ERROR(status)) {
        gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                        0,
                        0,
                        clvREPORT_INTERNAL_ERROR,
                        "failed to get the uniform index"));

            return status;
    }

    *Index = (gctREG_INDEX)sampler;
    return gcvSTATUS_OK;
}

gceSTATUS
clNewOutput(
IN cloCOMPILER Compiler,
IN gctUINT LineNo,
IN gctUINT StringNo,
IN gctCONST_STRING Name,
IN clsGEN_CODE_DATA_TYPE DataType,
IN gctSIZE_T Length,
IN gctREG_INDEX TempRegIndex
)
{
    gceSTATUS  status;
    gctSIZE_T  i;
    gcsTYPE_SIZE typeSize;
    gcSHADER_TYPE type;

    typeSize = clConvToShaderDataType(Compiler, DataType);
    type = typeSize.type;

    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                      clvDUMP_CODE_EMITTER,
                      "<OUTPUT line=\"%d\" string=\"%d\" name=\"%s\""
                      " dataType=\"%s\" length=\"%d\" tempRegIndex=\"%d\">",
                      LineNo,
                      StringNo,
                      Name,
                      gcGetShaderDataTypeName(type),
                      Length * typeSize.length,
                      TempRegIndex));

    status = _AddOutput(Compiler,
                        Name,
                        type,
                        Length * typeSize.length,
                        TempRegIndex);
    if (gcmIS_ERROR(status)) {
        gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                        LineNo,
                                        StringNo,
                                        clvREPORT_INTERNAL_ERROR,
                                        "failed to add the output"));
        return status;
    }

    for (i = 1; i < Length; i++) {
        status = _AddOutputIndexed(Compiler,
                                   Name,
                                   i,
                                   TempRegIndex + (gctREG_INDEX)i);

        if (gcmIS_ERROR(status)) {
            gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                            LineNo,
                                            StringNo,
                                            clvREPORT_INTERNAL_ERROR,
                                            "failed to add the indexed output"));
            return status;
        }
    }

    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                                  clvDUMP_CODE_EMITTER,
                                  "</OUTPUT>"));
    return gcvSTATUS_OK;
}

gctTYPE_QUALIFIER
clConvToShaderTypeQualifier(
cltQUALIFIER Qualifier
)
{
   switch(Qualifier) {
   case clvQUALIFIER_CONSTANT:
        return gcvTYPE_QUALIFIER_CONSTANT;

   case clvQUALIFIER_GLOBAL:
        return gcvTYPE_QUALIFIER_GLOBAL;

   case clvQUALIFIER_LOCAL:
        return gcvTYPE_QUALIFIER_LOCAL;

   case clvQUALIFIER_PRIVATE:
        return gcvTYPE_QUALIFIER_PRIVATE;

   case clvQUALIFIER_READ_ONLY:
        return gcvTYPE_QUALIFIER_READ_ONLY;

   case clvQUALIFIER_WRITE_ONLY:
        return gcvTYPE_QUALIFIER_WRITE_ONLY;

   case clvQUALIFIER_CONST:
        return gcvTYPE_QUALIFIER_CONST;

   default:
        return gcvTYPE_QUALIFIER_NONE;
   }
}

gctTYPE_QUALIFIER
clConvStorageQualifierToShaderTypeQualifier(
cltQUALIFIER Qualifier
)
{
   gctTYPE_QUALIFIER qualifier = gcvTYPE_QUALIFIER_NONE;

   if(Qualifier & clvSTORAGE_QUALIFIER_VOLATILE) {
       qualifier |= gcvTYPE_QUALIFIER_VOLATILE;
   }

   if(Qualifier & clvSTORAGE_QUALIFIER_RESTRICT) {
       qualifier |= gcvTYPE_QUALIFIER_RESTRICT;
   }

   return qualifier;
}

gceSTATUS
clNewVariable(
IN cloCOMPILER Compiler,
IN gctUINT LineNo,
IN gctUINT StringNo,
IN gctCONST_STRING Name,
IN cltQUALIFIER AccessQualifier,
IN cltQUALIFIER AddrSpaceQualifier,
IN cltQUALIFIER StorageQualifier,
IN clsGEN_CODE_DATA_TYPE DataType,
IN gctSIZE_T Length,
IN clsARRAY *Array,
IN gctBOOL   IsArray,
IN gctREG_INDEX TempRegIndex,
OUT gcVARIABLE *Variable,
IN gctINT16 parent,
IN gctINT16 prevSibling,
OUT gctINT16* ThisVarIndex
)
{
    gceSTATUS status;
    gcsTYPE_SIZE typeSize;
    gcSHADER_TYPE type;
    clsARRAY     array;
    gcSHADER     binary;
    gctUINT16     varIndex;
    gcVARIABLE   variable;
    gctINT       length;

    typeSize = clConvToShaderDataType(Compiler, DataType);
    type = typeSize.type;
    length = Length * typeSize.length;
    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                      clvDUMP_CODE_EMITTER,
                      "<VARIABLE line=\"%d\" string=\"%d\" name=\"%s\" "
                      "dataType=\"%s\" length=\"%d\" tempRegIndex=\"%d\">",
                      LineNo,
                      StringNo,
                      Name,
                      gcGetShaderDataTypeName(type),
                      length,
                      TempRegIndex));

    gcmVERIFY_OK(cloCOMPILER_GetBinary(Compiler, &binary));
    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                      clvDUMP_CODE_EMITTER,
                      "gcSHADER_AddVariableEx(Shader, \"%s\", %s, %d, %d);",
                      Name,
                      gcGetShaderDataTypeName(type),
                      length,
                      TempRegIndex));

    if (IsArray)
    {
        if(Array) {
            array = *Array;
            if(typeSize.length > 1) {
                array.length[Array->numDim] = typeSize.length;
                array.numDim = Array->numDim + 1;
            }
        }
        else {
            array.numDim = 1;
            array.length[0] = length;
        }
    }
    else
    {
        array.numDim = -1;
        array.length[0] = length;
    }

    status = gcSHADER_AddVariableEx(binary,
                                    Name,
                                    type,
                                    array.numDim,
                                    (gctINT *)array.length,
                                    TempRegIndex,
                                    gcSHADER_VAR_CATEGORY_NORMAL,
                                    gcSHADER_PRECISION_DEFAULT,
                                    0,
                                    parent,
                                    prevSibling,
                                    (gctINT16*)&varIndex);
    if(ThisVarIndex != gcvNULL)
        *ThisVarIndex = varIndex;
    if (gcmIS_ERROR(status)) {
        cloCOMPILER_Report(Compiler,
                           LineNo,
                           StringNo,
                           clvREPORT_INTERNAL_ERROR,
                           "failed to add variable");
        return status;
    }
    status = gcSHADER_GetVariable(binary,
                                  varIndex,
                                  &variable);
    if (gcmIS_ERROR(status)) return status;
    if(typeSize.length > 1 && clmGEN_CODE_IsExtendedVectorType(DataType)) {
        SetVariableFlag(variable, gceVARFLAG_IS_EXTENDED_VECTOR);
    }
    if(Variable) {
        *Variable = variable;
    }

    status = gcSHADER_UpdateVariable(binary,
                                     varIndex,
                                     gcvVARIABLE_UPDATE_TYPE_QUALIFIER,
                                     clConvToShaderTypeQualifier(AccessQualifier) |
                                     clConvToShaderTypeQualifier(AddrSpaceQualifier) |
                                     clConvStorageQualifierToShaderTypeQualifier(StorageQualifier));
    if (gcmIS_ERROR(status)) return status;

    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                      clvDUMP_CODE_EMITTER,
                      "</VARIABLE>"));
    return gcvSTATUS_OK;
}

gceSTATUS
clNewStructIntermediateElementSymbol(
IN cloCOMPILER Compiler,
IN gctCONST_STRING Name,
IN clsGEN_CODE_DATA_TYPE DataType,
IN gctSIZE_T Length,
IN clsARRAY *Array,
IN gctBOOL   IsArray,
IN gctREG_INDEX TempRegIndex,
IN gcSHADER_VAR_CATEGORY varCategory,
IN gctUINT16 numStructureElement,
IN gctINT16 parent,
IN gctINT16 prevSibling,
OUT gctINT16* ThisVarIndex
)
{
    gceSTATUS status;
    gcsTYPE_SIZE typeSize;
    gcSHADER_TYPE type;
    clsARRAY     array;
    gcSHADER     binary;
    gctUINT16     varIndex;
    gctINT       length;

    typeSize = clConvToShaderDataType(Compiler, DataType);
    type = typeSize.type;
    length = Length * typeSize.length;

    gcmVERIFY_OK(cloCOMPILER_GetBinary(Compiler, &binary));
    if (IsArray)
    {
        if(Array) {
            array = *Array;
            if(typeSize.length > 1) {
                array.length[Array->numDim] = typeSize.length;
                array.numDim = Array->numDim + 1;
            }
        }
        else {
            array.numDim = 1;
            array.length[0] = length;
        }
    }
    else
    {
        array.numDim = -1;
        array.length[0] = length;
    }

    status = gcSHADER_AddVariableEx(binary,
                                    Name,
                                    type,
                                    array.numDim,
                                    (gctINT *)array.length,
                                    TempRegIndex,
                                    varCategory,
                                    gcSHADER_PRECISION_DEFAULT,
                                    numStructureElement,
                                    parent,
                                    prevSibling,
                                    (gctINT16*)&varIndex);
    if(ThisVarIndex != gcvNULL)
        *ThisVarIndex = varIndex;
    return status;
}

gceSTATUS
clSetLabel(
IN cloCOMPILER Compiler,
IN gctUINT LineNo,
IN gctUINT StringNo,
IN gctLABEL Label
)
{
    gceSTATUS    status;
    cloCODE_EMITTER    codeEmitter;

    codeEmitter = cloCOMPILER_GetCodeEmitter(Compiler);
    gcmASSERT(codeEmitter);

    status = cloCODE_EMITTER_NewBasicBlock(Compiler, codeEmitter);
    if (gcmIS_ERROR(status)) return status;

    if (LineNo != 0) {
        gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                          clvDUMP_CODE_EMITTER,
                          "<LABEL line=\"%d\" string=\"%d\" no=\"%d\">",
                          LineNo,
                          StringNo,
                          Label));
    }
    else {
        gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                          clvDUMP_CODE_EMITTER,
                          "<LABEL no=\"%d\">",
                          Label));
    }

    status = _AddLabel(Compiler, Label);

    clGenClearCurrentVectorCreation(Compiler);

    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                      clvDUMP_CODE_EMITTER,
                      "</LABEL>"));

        if (gcmIS_ERROR(status)) {
        gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                        LineNo,
                        StringNo,
                        clvREPORT_INTERNAL_ERROR,
                        "failed to add the label"));
            return status;
    }
    return gcvSTATUS_OK;
}

gctUINT8
clConvPackedTypeToSwizzle(
    IN cloCOMPILER Compiler,
    IN clsGEN_CODE_DATA_TYPE PackedType
    )
{
    gcmASSERT(clmIsElementTypePacked(PackedType.elementType));
    switch(clGEN_CODE_DataTypeByteSize(Compiler, PackedType)) {
    case 1:
    case 2:
    case 3:
    case 4:
        return gcSL_SWIZZLE_XXXX;

    case 8:
        return gcSL_SWIZZLE_XYYY;

    case 16:
    case 32:
        return gcSL_SWIZZLE_XYZW;

    default:
        gcmASSERT(0);
        return gcSL_SWIZZLE_XYZW;
    }
}

gctUINT8
clConvPackedTypeToEnable(
    IN cloCOMPILER Compiler,
    IN clsGEN_CODE_DATA_TYPE PackedType
    )
{
    gcmASSERT(clmIsElementTypePacked(PackedType.elementType));
    switch(clGEN_CODE_DataTypeByteSize(Compiler, PackedType)) {
    case 1:
    case 2:
    case 3:
    case 4:
        return gcSL_ENABLE_X;

    case 8:
        return gcSL_ENABLE_XY;

    case 16:
    case 32:
        return gcSL_ENABLE_XYZW;

    default:
        gcmASSERT(0);
        return gcSL_ENABLE_XYZW;
    }
}

gctUINT8
gcGetDefaultEnable(
    IN cloCOMPILER Compiler,
    IN clsGEN_CODE_DATA_TYPE DataType
    )
{
    if(clmIsElementTypePacked(DataType.elementType)) {
        return clConvPackedTypeToEnable(Compiler, DataType);
    }

    if((cloCOMPILER_ExtensionEnabled(Compiler, clvEXTENSION_VIV_VX) ||
        gcmOPT_oclUseImgIntrinsicQuery()) &&
       clmIsElementTypeImage(DataType.elementType)) return gcSL_ENABLE_XYZW;

    switch(clmGEN_CODE_vectorSize_NOCHECK_GET(DataType)) {
    case 0:
      return gcSL_ENABLE_X;

    case 2:
      return gcSL_ENABLE_XY;

    case 3:
      return gcSL_ENABLE_XYZ;

    case 4:
    case 8:
    case 16:
      return gcSL_ENABLE_XYZW;

    case 32:
      if(!clmIsElementTypePacked(DataType.elementType)) {
          gcmASSERT(0);
      }
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
    gctUINT8    i;
    gcSL_ENABLE    enables[4]={gcSL_ENABLE_NONE,gcSL_ENABLE_NONE,
                                        gcSL_ENABLE_NONE,gcSL_ENABLE_NONE};

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

    return enables[Component];
}

gctUINT8
gcGetDefaultSwizzle(
    IN cloCOMPILER Compiler,
    IN clsGEN_CODE_DATA_TYPE DataType
    )
{
    if(clmIsElementTypePacked(DataType.elementType)) {
        return clConvPackedTypeToSwizzle(Compiler,
                                         DataType);
    }

    switch(clmGEN_CODE_vectorSize_NOCHECK_GET(DataType)) {
    case 0:
      return gcSL_SWIZZLE_XXXX;

    case 2:
      return gcSL_SWIZZLE_XYYY;

    case 3:
      return gcSL_SWIZZLE_XYZZ;

    case 4:
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
    IN cloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gcSL_OPCODE Opcode,
    IN gcsTARGET * Target,
    IN gcsSOURCE * Source0,
    IN gcsSOURCE * Source1
    );

static gceSTATUS
_MakeNewSource(
    IN cloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gcsSOURCE * Source,
    OUT gcsSOURCE * NewSource
    )
{
    gceSTATUS    status;
    gcsTARGET    tempTarget;

    gcsTARGET_Initialize(
                        &tempTarget,
                        Source->dataType,
                        clNewTempRegs(Compiler, 1, Source->dataType.elementType),
                        gcGetDefaultEnable(Compiler, Source->dataType),
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

    if (gcmIS_ERROR(status)) return status;

    gcsSOURCE_InitializeTempReg(
                                NewSource,
                                Source->dataType,
                                tempTarget.tempRegIndex,
                                gcGetDefaultSwizzle(Compiler, Source->dataType),
                                gcSL_NOT_INDEXED,
                                0);

    return gcvSTATUS_OK;
}

static gceSTATUS
_PrepareSource(
    IN cloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gcsTARGET * Target,
    IN gcsSOURCE * Source,
    OUT gcsSOURCE * NewSource
    )
{
    gceSTATUS    status;
    gctBOOL        insertAssign;

    gcmASSERT(Source);
    gcmASSERT(NewSource);

    if (Target != gcvNULL)
    {
        insertAssign = (Source->type == gcvSOURCE_TEMP
                        && Target->tempRegIndex == Source->u.sourceReg.regIndex);
        {
            gctBOOL     useFullNewLinker = gcvFALSE;
            gctBOOL     hasHalti2 = gcGetHWCaps()->hwFeatureFlags.hasHalti2;

            useFullNewLinker = gcUseFullNewLinker(hasHalti2);

            if (useFullNewLinker && insertAssign)
            {
                insertAssign = gcvFALSE;
            }
        }
    }
    else
    {
        insertAssign = (Source->type == gcvSOURCE_UNIFORM);
    }

    if (insertAssign)
    {
        status = _MakeNewSource(
                                Compiler,
                                LineNo,
                                StringNo,
                                Source,
                                NewSource);

        if (gcmIS_ERROR(status)) return status;
    }
    else
    {
        *NewSource = *Source;
    }

    return gcvSTATUS_OK;
}

static gceSTATUS
_PrepareAnotherSource(
    IN cloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gcsTARGET * Target,
    IN gcsSOURCE * Source0,
    IN gcsSOURCE * Source1,
    OUT gcsSOURCE * NewSource1
    )
{
    gceSTATUS    status;
    gctBOOL      insertAssign = gcvFALSE;

    gcmASSERT(Source0);
    gcmASSERT(Source1);
    gcmASSERT(NewSource1);


    if(!cloCOMPILER_ExtensionEnabled(Compiler, clvEXTENSION_VIV_VX))
    {
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
            gctBOOL     hasHalti2 = gcGetHWCaps()->hwFeatureFlags.hasHalti2;

            useFullNewLinker = gcUseFullNewLinker(hasHalti2);

            if (useFullNewLinker && insertAssign)
            {
                insertAssign = gcvFALSE;
            }
        }
    }

    if (insertAssign)
    {
        status = _MakeNewSource(
                                Compiler,
                                LineNo,
                                StringNo,
                                Source1,
                                NewSource1);

        if (gcmIS_ERROR(status)) return status;
    }
    else {
        *NewSource1 = *Source1;
    }

    return gcvSTATUS_OK;
}

static gcSL_OPCODE
_ConvOpcode(
    cleOPCODE opcode
    )
{
    switch (opcode)
    {
    case clvOPCODE_ASSIGN:           return gcSL_MOV;
    case clvOPCODE_CONV:             return gcSL_CONV;
    case clvOPCODE_CONV_SAT:         return gcSL_CONV;
    case clvOPCODE_CONV_SAT_RTE:     return gcSL_CONV;
    case clvOPCODE_CONV_SAT_RTZ:     return gcSL_CONV;
    case clvOPCODE_CONV_SAT_RTN:     return gcSL_CONV;
    case clvOPCODE_CONV_SAT_RTP:     return gcSL_CONV;
    case clvOPCODE_CONV_RTE:         return gcSL_CONV;
    case clvOPCODE_CONV_RTZ:         return gcSL_CONV;
    case clvOPCODE_CONV_RTN:         return gcSL_CONV;
    case clvOPCODE_CONV_RTP:         return gcSL_CONV;

    case clvOPCODE_ADD:              return gcSL_ADD;
    case clvOPCODE_SUB:              return gcSL_SUB;
    case clvOPCODE_IMUL:
    case clvOPCODE_MUL:              return gcSL_MUL;
    case clvOPCODE_MUL_Z:            return gcSL_MUL_Z;
    case clvOPCODE_DIV:              return gcSL_DIV;
    case clvOPCODE_IDIV:             return gcSL_DIV;
    case clvOPCODE_MOD:              return gcSL_MOD;

    case clvOPCODE_DP2:              return gcSL_DP2;
    case clvOPCODE_DP3:              return gcSL_DP3;
    case clvOPCODE_DP4:              return gcSL_DP4;

    case clvOPCODE_FADD:             return gcSL_ADD;
    case clvOPCODE_FSUB:             return gcSL_SUB;
    case clvOPCODE_FMUL:             return gcSL_MUL;
    case clvOPCODE_FMOD:             return gcSL_MOD;

    case clvOPCODE_RSHIFT:           return gcSL_RSHIFT;
    case clvOPCODE_LSHIFT:           return gcSL_LSHIFT;
    case clvOPCODE_NEG:              return gcSL_NEG;
    case clvOPCODE_NOT_BITWISE:      return gcSL_NOT_BITWISE;

    case clvOPCODE_AND_BITWISE:      return gcSL_AND_BITWISE;
    case clvOPCODE_OR_BITWISE:       return gcSL_OR_BITWISE;
    case clvOPCODE_XOR_BITWISE:      return gcSL_XOR_BITWISE;

    case clvOPCODE_BARRIER:          return gcSL_BARRIER;
    case clvOPCODE_MEM_FENCE:        return gcSL_MEM_BARRIER;
    case clvOPCODE_LOAD:             return gcSL_LOAD;
    case clvOPCODE_STORE:            return gcSL_STORE;
#if cldUseSTORE1
    case clvOPCODE_STORE1:           return gcSL_STORE1;
    case clvOPCODE_STORE1_RTE:       return gcSL_STORE1;
    case clvOPCODE_STORE1_RTZ:       return gcSL_STORE1;
    case clvOPCODE_STORE1_RTP:       return gcSL_STORE1;
    case clvOPCODE_STORE1_RTN:       return gcSL_STORE1;
#endif

    case clvOPCODE_TEXTURE_LOAD:     return gcSL_TEXLD;
    case clvOPCODE_IMAGE_WRITE:      return gcSL_IMAGE_WR;
    case clvOPCODE_IMAGE_WRITE_3D:   return gcSL_IMAGE_WR_3D;
    case clvOPCODE_IMAGE_READ:       return gcSL_IMAGE_RD;
    case clvOPCODE_IMAGE_READ_3D:    return gcSL_IMAGE_RD_3D;
    case clvOPCODE_IMAGE_SAMPLER:    return gcSL_IMAGE_SAMPLER;
    case clvOPCODE_CLAMP0MAX:        return gcSL_CLAMP0MAX;
    case clvOPCODE_CLAMPCOORD:       return gcSL_CLAMPCOORD;
    case clvOPCODE_TEXU:             return gcSL_TEXU;

    case clvOPCODE_FLOAT_TO_INT:     return gcSL_F2I;
    case clvOPCODE_FLOAT_TO_UINT:    return gcSL_F2I;
    case clvOPCODE_FLOAT_TO_BOOL:    gcmASSERT(0); return gcSL_NOP;
    case clvOPCODE_FLOAT_TO_HALF:    gcmASSERT(0); return gcSL_NOP;
    case clvOPCODE_HALF_TO_FLOAT:    gcmASSERT(0); return gcSL_NOP;
    case clvOPCODE_INT_TO_BOOL:      gcmASSERT(0); return gcSL_NOP;
    case clvOPCODE_UINT_TO_BOOL:     gcmASSERT(0); return gcSL_NOP;
    case clvOPCODE_INT_TO_INT:       return gcSL_CONV;
    case clvOPCODE_INT_TO_UINT:      return gcSL_CONV;
    case clvOPCODE_UINT_TO_UINT:     return gcSL_CONV;
    case clvOPCODE_INT_TO_FLOAT:     return gcSL_I2F;
    case clvOPCODE_UINT_TO_INT:      return gcSL_CONV;
    case clvOPCODE_UINT_TO_FLOAT:    return gcSL_I2F;
    case clvOPCODE_BOOL_TO_FLOAT:    return gcSL_I2F;
    case clvOPCODE_BOOL_TO_INT:      return gcSL_MOV;
    case clvOPCODE_BOOL_TO_UINT:     return gcSL_MOV;

    case clvOPCODE_IMPL_I2F:         gcmASSERT(0); return gcSL_NOP;
    case clvOPCODE_IMPL_U2F:         gcmASSERT(0); return gcSL_NOP;
    case clvOPCODE_IMPL_B2F:         gcmASSERT(0); return gcSL_NOP;

    case clvOPCODE_INVERSE:          return gcSL_RCP;

    case clvOPCODE_LESS_THAN:        gcmASSERT(0); return gcSL_NOP;
    case clvOPCODE_LESS_THAN_EQUAL:  gcmASSERT(0); return gcSL_NOP;
    case clvOPCODE_GREATER_THAN:     gcmASSERT(0); return gcSL_NOP;
    case clvOPCODE_GREATER_THAN_EQUAL:    gcmASSERT(0); return gcSL_NOP;
    case clvOPCODE_EQUAL:            gcmASSERT(0); return gcSL_NOP;
    case clvOPCODE_NOT_EQUAL:        gcmASSERT(0); return gcSL_NOP;

    case clvOPCODE_ANY:              gcmASSERT(0); return gcSL_NOP;
    case clvOPCODE_ALL:              gcmASSERT(0); return gcSL_NOP;
    case clvOPCODE_NOT:              gcmASSERT(0); return gcSL_NOP;

    case clvOPCODE_SIN:              return gcSL_SIN;
    case clvOPCODE_COS:              return gcSL_COS;
    case clvOPCODE_TAN:              return gcSL_TAN;

    case clvOPCODE_ASIN:             return gcSL_ASIN;
    case clvOPCODE_ACOS:             return gcSL_ACOS;
    case clvOPCODE_ATAN:             return gcSL_ATAN;
    case clvOPCODE_ATAN2:            gcmASSERT(0); return gcSL_NOP;

    case clvOPCODE_SINPI:            return gcSL_SINPI;
    case clvOPCODE_COSPI:            return gcSL_COSPI;
    case clvOPCODE_TANPI:            return gcSL_TANPI;

    case clvOPCODE_ARCTRIG0:         return gcSL_ARCTRIG0;
    case clvOPCODE_ARCTRIG1:         return gcSL_ARCTRIG1;

    case clvOPCODE_MULLO:            return gcSL_MULLO;
    case clvOPCODE_ADDLO:            return gcSL_ADDLO;

    case clvOPCODE_ROTATE:           return gcSL_ROTATE;
    case clvOPCODE_LEADZERO:         return gcSL_LEADZERO;
    case clvOPCODE_GETEXP:           return gcSL_GETEXP;
    case clvOPCODE_GETMANT:          return gcSL_GETMANT;
    case clvOPCODE_CTZ:              return gcSL_CTZ;

    case clvOPCODE_POW:              return gcSL_POW;
    case clvOPCODE_EXP2:             return gcSL_EXP;
    case clvOPCODE_LOG2:             return gcSL_LOG;
    case clvOPCODE_SQRT:             return gcSL_SQRT;

    case clvOPCODE_INVERSE_SQRT:     return gcSL_RSQ;

    case clvOPCODE_ABS:              return gcSL_ABS;
    case clvOPCODE_POPCOUNT:         return gcSL_POPCOUNT;
    case clvOPCODE_SIGN:             return gcSL_SIGN;
    case clvOPCODE_FLOOR:            return gcSL_FLOOR;
    case clvOPCODE_CEIL:             return gcSL_CEIL;
    case clvOPCODE_FRACT:            return gcSL_FRAC;
    case clvOPCODE_MIN:              return gcSL_MIN;
    case clvOPCODE_MAX:              return gcSL_MAX;
    case clvOPCODE_SATURATE:         return gcSL_SAT;
    case clvOPCODE_STEP:             return gcSL_STEP;
    case clvOPCODE_DOT:              gcmASSERT(0); return gcSL_NOP;
    case clvOPCODE_CROSS:            return gcSL_CROSS;
    case clvOPCODE_NORMALIZE:        gcmASSERT(0); return gcSL_NOP;

    case clvOPCODE_JUMP:             return gcSL_JMP;
    case clvOPCODE_CALL:             return gcSL_CALL;
    case clvOPCODE_RETURN:           return gcSL_RET;

    case clvOPCODE_DFDX:             return gcSL_DSX;
    case clvOPCODE_DFDY:             return gcSL_DSY;
    case clvOPCODE_FWIDTH:           return gcSL_FWIDTH;

    case clvOPCODE_MULHI:            return gcSL_MULHI;
    case clvOPCODE_CMP:              return gcSL_CMP;
    case clvOPCODE_SET:              return gcSL_SET;
    case clvOPCODE_SUBSAT:           return gcSL_SUBSAT;
    case clvOPCODE_ADDSAT:           return gcSL_ADDSAT;
    case clvOPCODE_MULSAT:           return gcSL_MULSAT;
    case clvOPCODE_MADSAT:           return gcSL_MADSAT;

    case clvOPCODE_ATOMADD:          return gcSL_ATOMADD;
    case clvOPCODE_ATOMSUB:          return gcSL_ATOMSUB;
    case clvOPCODE_ATOMXCHG:         return gcSL_ATOMXCHG;
    case clvOPCODE_ATOMCMPXCHG:      return gcSL_ATOMCMPXCHG;
    case clvOPCODE_ATOMMIN:          return gcSL_ATOMMIN;
    case clvOPCODE_ATOMMAX:          return gcSL_ATOMMAX;
    case clvOPCODE_ATOMOR:           return gcSL_ATOMOR;
    case clvOPCODE_ATOMAND:          return gcSL_ATOMAND;
    case clvOPCODE_ATOMXOR:          return gcSL_ATOMXOR;

    case clvOPCODE_UNPACK:           return gcSL_UNPACK;
    case clvOPCODE_ASTYPE:           return gcSL_CONV;
    case clvOPCODE_NOP:              return gcSL_NOP;
    case clvOPCODE_LONGLO:           return gcSL_LONGLO;
    case clvOPCODE_LONGHI:           return gcSL_LONGHI;
    case clvOPCODE_MOV_LONG:         return gcSL_MOV_LONG;
    case clvOPCODE_COPY:             return gcSL_COPY;
    case clvOPCODE_PARAM_CHAIN:      return gcSL_PARAM_CHAIN;
    case clvOPCODE_INTRINSIC:        return gcSL_INTRINSIC;
    case clvOPCODE_INTRINSIC_ST:     return gcSL_INTRINSIC_ST;
    case clvOPCODE_FMA_MUL:          return gcSL_FMA_MUL;
    case clvOPCODE_FMA_ADD:          return gcSL_FMA_ADD;

    case clvOPCODE_CMAD:             return gcSL_CMAD;
    case clvOPCODE_CONJ:             return gcSL_CONJ;
    case clvOPCODE_CMUL:             return gcSL_CMUL;
    case clvOPCODE_CMADCJ:           return gcSL_CMADCJ;
    case clvOPCODE_CMULCJ:           return gcSL_CMULCJ;
    case clvOPCODE_CADDCJ:           return gcSL_CADDCJ;
    case clvOPCODE_CSUBCJ:           return gcSL_CSUBCJ;

    case clvOPCODE_GET_IMAGE_TYPE:   return gcSL_GET_IMAGE_TYPE;
    case clvOPCODE_FINDLSB:          return gcSL_FINDLSB;
    case clvOPCODE_FINDMSB:          return gcSL_FINDMSB;
    case clvOPCODE_BIT_REVERSAL:     return gcSL_BIT_REVERSAL;
    case clvOPCODE_BYTE_REVERSAL:    return gcSL_BYTE_REVERSAL;

    default:
        gcmASSERT(0);
        return gcSL_NOP;
    }
}

static gceSTATUS
_EmitCode(
    IN cloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gcSL_OPCODE Opcode,
    IN gcsTARGET * Target,
    IN gcsSOURCE * Source0,
    IN gcsSOURCE * Source1
    )
{
    gceSTATUS    status;

    gcmASSERT(Target);
    gcmASSERT(!gcIsMatrixDataType(Target->dataType));
    gcmASSERT(Source0);
    gcmASSERT(!gcIsMatrixDataType(Source0->dataType));

    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                    clvDUMP_CODE_EMITTER,
                    "<INSTRUCTION line=\"%d\" string=\"%d\" opcode=\"%s\""
                    " targetDataType=\"%s\"",
                    LineNo,
                    StringNo,
                    _GetOpcodeName(Opcode),
                    gcGetDataTypeName(Compiler, Target->dataType)));

    if (Source1 == gcvNULL)
    {
        gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                          clvDUMP_CODE_EMITTER,
                          " sourceDataType=\"%s\">",
                          gcGetDataTypeName(Compiler, Source0->dataType)));
    }
    else
    {
        gcmASSERT(!gcIsMatrixDataType(Source1->dataType));

        gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                          clvDUMP_CODE_EMITTER,
                          " source0DataType=\"%s\" source1DataType=\"%s\">",
                          gcGetDataTypeName(Compiler, Source0->dataType),
                          gcGetDataTypeName(Compiler, Source1->dataType)));
    }

    status = _EmitOpcodeAndTarget(Compiler,
                      LineNo,
                      StringNo,
                      Opcode,
                      Target);

    if (gcmIS_ERROR(status)) return status;

    status = _EmitSource(Compiler,
                 LineNo,
                 StringNo,
                 Source0);

    if (gcmIS_ERROR(status)) return status;

    if (Source1 != gcvNULL)
    {
        status = _EmitSource(Compiler,
                     LineNo,
                     StringNo,
                     Source1);

        if (gcmIS_ERROR(status)) return status;
    }

    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                      clvDUMP_CODE_EMITTER,
                      "</INSTRUCTION>"));

    return gcvSTATUS_OK;
}

static gceSTATUS
_EmitCodeWRound(
    IN cloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gcSL_OPCODE Opcode,
    IN gcsTARGET * Target,
    IN gcsSOURCE * Source0,
    IN gcsSOURCE * Source1,
    IN gcSL_ROUND Round
    )
{
    gceSTATUS    status;

    gcmASSERT(Target);
    gcmASSERT(!gcIsMatrixDataType(Target->dataType));
    gcmASSERT(Source0);
    gcmASSERT(!gcIsMatrixDataType(Source0->dataType));

    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                    clvDUMP_CODE_EMITTER,
                    "<INSTRUCTION line=\"%d\" string=\"%d\" opcode=\"%s\""
                    " targetDataType=\"%s\"",
                    LineNo,
                    StringNo,
                    _GetOpcodeName(Opcode),
                    gcGetDataTypeName(Compiler, Target->dataType)));

    if (Source1 == gcvNULL)
    {
        gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                          clvDUMP_CODE_EMITTER,
                          " sourceDataType=\"%s\">",
                          gcGetDataTypeName(Compiler, Source0->dataType)));
    }
    else
    {
        gcmASSERT(!gcIsMatrixDataType(Source1->dataType));

        gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                          clvDUMP_CODE_EMITTER,
                          " source0DataType=\"%s\" source1DataType=\"%s\">",
                          gcGetDataTypeName(Compiler, Source0->dataType),
                          gcGetDataTypeName(Compiler, Source1->dataType)));
    }

    status = _EmitOpcodeAndTarget(Compiler,
                      LineNo,
                      StringNo,
                      Opcode,
                      Target);

    if (gcmIS_ERROR(status)) return status;

    status = _AddRoundingMode(Compiler, Round);

    if (gcmIS_ERROR(status)) return status;

    status = _EmitSource(Compiler,
                 LineNo,
                 StringNo,
                 Source0);

    if (gcmIS_ERROR(status)) return status;

    if (Source1 != gcvNULL)
    {
        status = _EmitSource(Compiler,
                     LineNo,
                     StringNo,
                     Source1);

        if (gcmIS_ERROR(status)) return status;
    }

    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                      clvDUMP_CODE_EMITTER,
                      "</INSTRUCTION>"));

    return gcvSTATUS_OK;
}

static gceSTATUS
_EmitCode0(
    IN cloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN cleOPCODE Opcode
    )
{
    gceSTATUS    status;
    gcSL_OPCODE opcode;

    opcode = _ConvOpcode(Opcode);

    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                    clvDUMP_CODE_EMITTER,
                    "<INSTRUCTION line=\"%d\" string=\"%d\" opcode=\"%s\"",
                    LineNo,
                    StringNo,
                    _GetOpcodeName(opcode)));

    status = _AddOpcode(Compiler,
                opcode,
                0,
                0,
                0,
                GCSL_Build_SRC_LOC(LineNo, StringNo));

    if (gcmIS_ERROR(status)) return status;

    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                      clvDUMP_CODE_EMITTER,
                      "</INSTRUCTION>"));

    return gcvSTATUS_OK;
}

static gceSTATUS
_EmitBranchCode(
    IN cloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gcSL_OPCODE Opcode,
    IN gcSL_CONDITION Condition,
    IN gctLABEL Label,
    IN gcsSOURCE * Source0,
    IN gcsSOURCE * Source1
    )
{
    gceSTATUS           status;
    cloCODE_EMITTER     codeEmitter;
    cloCODE_GENERATOR   codeGenerator;
    cltELEMENT_TYPE     sourceType;

    codeGenerator = cloCOMPILER_GetCodeGenerator(Compiler);
    codeEmitter = cloCOMPILER_GetCodeEmitter(Compiler);
    gcmASSERT(codeEmitter);

    status = cloCODE_EMITTER_EndBasicBlock(Compiler, codeEmitter);

    if (gcmIS_ERROR(status)) return status;

    gcmVERIFY_OK(cloCOMPILER_Dump(
                                Compiler,
                                clvDUMP_CODE_EMITTER,
                                "<INSTRUCTION line=\"%d\" string=\"%d\" opcode=\"%s\""
                                " condition=\"%s\" label=\"%d\"",
                                LineNo,
                                StringNo,
                                _GetOpcodeName(Opcode),
                                _GetConditionName(Condition),
                                Label));

    if (Source0 != gcvNULL)
    {
        gcmASSERT(!gcIsMatrixDataType(Source0->dataType));

        gcmVERIFY_OK(cloCOMPILER_Dump(
                                    Compiler,
                                    clvDUMP_CODE_EMITTER,
                                    " source0DataType=\"%s\"",
                                    gcGetDataTypeName(Compiler, Source0->dataType)));
    }

    if (Source1 != gcvNULL)
    {
        gcmASSERT(!gcIsMatrixDataType(Source1->dataType));

        gcmVERIFY_OK(cloCOMPILER_Dump(
                                    Compiler,
                                    clvDUMP_CODE_EMITTER,
                                    " source1DataType=\"%s\"",
                                    gcGetDataTypeName(Compiler, Source1->dataType)));
    }

    gcmVERIFY_OK(cloCOMPILER_Dump(
                                Compiler,
                                clvDUMP_CODE_EMITTER,
                                ">"));

    if (Source0 && !codeGenerator->fulllySupportIntegerBranch)
    {
        sourceType = Source0->dataType.elementType;

        if (sourceType == clvTYPE_CHAR || sourceType == clvTYPE_SHORT)
        {
            Source0->dataType.elementType = clvTYPE_INT;
        }
        else if (sourceType == clvTYPE_UCHAR || sourceType == clvTYPE_USHORT)
        {
            Source0->dataType.elementType = clvTYPE_UINT;
        }

    }
    if (Source1 && !codeGenerator->fulllySupportIntegerBranch)
    {
        sourceType = Source1->dataType.elementType;

        if (sourceType == clvTYPE_CHAR || sourceType == clvTYPE_SHORT)
        {
            Source1->dataType.elementType = clvTYPE_INT;
        }
        else if (sourceType == clvTYPE_UCHAR || sourceType == clvTYPE_USHORT)
        {
            Source1->dataType.elementType = clvTYPE_UINT;
        }
    }

    status = _EmitOpcodeConditional(Compiler,
                    LineNo,
                    StringNo,
                    Opcode,
                    Condition,
                    Source0 ? &Source0->dataType : gcvNULL,
                    Label);

    if (gcmIS_ERROR(status)) return status;

    if (Source0 != gcvNULL)
    {
        status = _EmitSource(
                            Compiler,
                            LineNo,
                            StringNo,
                            Source0);

        if (gcmIS_ERROR(status)) return status;
    }

    if (Source1 != gcvNULL)
    {
        status = _EmitSource(
                            Compiler,
                            LineNo,
                            StringNo,
                            Source1);

        if (gcmIS_ERROR(status)) return status;
    }

    gcmVERIFY_OK(cloCOMPILER_Dump(
                                Compiler,
                                clvDUMP_CODE_EMITTER,
                                "</INSTRUCTION>"));

    status = cloCODE_EMITTER_NewBasicBlock(Compiler, codeEmitter);

    if (gcmIS_ERROR(status)) return status;

    return gcvSTATUS_OK;
}

static gceSTATUS
_EmitNullTargetCode(
    IN cloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN cleOPCODE Opcode,
    IN gcsSOURCE * Source0,
    IN gcsSOURCE * Source1
    )
{
    gceSTATUS    status;
    cloCODE_EMITTER    codeEmitter;
        gcSL_OPCODE opcode;

    codeEmitter = cloCOMPILER_GetCodeEmitter(Compiler);
    gcmASSERT(codeEmitter);

    status = cloCODE_EMITTER_EndBasicBlock(Compiler, codeEmitter);
    if (gcmIS_ERROR(status)) return status;

    opcode = _ConvOpcode(Opcode);
    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                      clvDUMP_CODE_EMITTER,
                      "<INSTRUCTION line=\"%d\" string=\"%d\" opcode=\"%s\"",
                      LineNo,
                      StringNo,
                      _GetOpcodeName(opcode)));

    if (Source0 != gcvNULL)
    {
        gcmASSERT(!gcIsMatrixDataType(Source0->dataType));

        gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                          clvDUMP_CODE_EMITTER,
                          " source0DataType=\"%s\"",
                          gcGetDataTypeName(Compiler, Source0->dataType)));
    }

    if (Source1 != gcvNULL)
    {
        gcmASSERT(!gcIsMatrixDataType(Source1->dataType));

        gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                          clvDUMP_CODE_EMITTER,
                          " source1DataType=\"%s\"",
                          gcGetDataTypeName(Compiler, Source1->dataType)));
    }

    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                      clvDUMP_CODE_EMITTER,
                      ">"));

    status = _AddOpcode(Compiler,
                opcode,
                0,
                0,
                0,
                GCSL_Build_SRC_LOC(LineNo, StringNo));
    if (gcmIS_ERROR(status)) return status;

    if (Source0 != gcvNULL) {
        status = _EmitSource(Compiler,
                     LineNo,
                     StringNo,
                     Source0);
        if (gcmIS_ERROR(status)) return status;
    }

    if (Source1 != gcvNULL) {
        status = _EmitSource(Compiler,
                     LineNo,
                     StringNo,
                     Source1);
        if (gcmIS_ERROR(status)) return status;
    }

    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                clvDUMP_CODE_EMITTER,
                "</INSTRUCTION>"));

    return gcvSTATUS_OK;
}

gceSTATUS
clEmitNullTargetCode(
    IN cloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN cleOPCODE Opcode,
    IN gcsSOURCE * Source0,
    IN gcsSOURCE * Source1
    )
{
    gceSTATUS status;
    gcsSOURCE *source1;
    gcsSOURCE newSource1;

    gcmASSERT(Source0);

    if(Source0 && Source1) {
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
    else source1 = Source1;

    return _EmitNullTargetCode(Compiler,
                   LineNo,
                   StringNo,
                   Opcode,
                   Source0,
                   source1);
}

gceSTATUS
clEmitConvCode(
    IN cloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN cleOPCODE Opcode,
    IN gcsTARGET * Target,
    IN gcsSOURCE * Source,
    IN clsGEN_CODE_DATA_TYPE DataType
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gcsSOURCE source1[1];
    gcSHADER binary;

    gcsSOURCE_InitializeTargetFormat(source1, DataType);
    status =  clEmitCode2(Compiler,
                          LineNo,
                          StringNo,
                          Opcode,
                          Target,
                          Source,
                          source1);
    if (gcmIS_ERROR(status)) return status;

    if(clmIsElementTypePacked(Target->dataType.elementType) &&
       !clmIsElementTypePacked(Source->dataType.elementType)) {
        gcmVERIFY_OK(cloCOMPILER_GetBinary(Compiler, &binary));
        return gcSHADER_UpdateSourcePacked(binary,
                                           gcSHADER_SOURCE0,
                                           clmGEN_CODE_vectorSize_GET(Source->dataType));
    }
    else if(clmIsElementTypePacked(Source->dataType.elementType) &&
            !clmIsElementTypePacked(Target->dataType.elementType)) {
        gcmVERIFY_OK(cloCOMPILER_GetBinary(Compiler, &binary));
        return gcSHADER_UpdateTargetPacked(binary,
                                           clmGEN_CODE_vectorSize_GET(Target->dataType));
    }
    return status;
}

gceSTATUS
clEmitAssignCode(
    IN cloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gcsTARGET * Target,
    IN gcsSOURCE * Source,
    IN gctBOOL IsUnionMember
    )
{
    if(IsUnionMember) {
       return clEmitConvCode(Compiler,
                             LineNo,
                             StringNo,
                             clvOPCODE_CONV,
                             Target,
                             Source,
                             Target->dataType);
    }
    else {
       return clEmitCode1(Compiler,
                  LineNo,
                  StringNo,
                  clvOPCODE_ASSIGN,
                  Target,
                  Source);
    }
}

static gceSTATUS
_EmitIntToFloatCode(
    IN cloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gcsTARGET * Target,
    IN gcsSOURCE * Source
    )
{
    gceSTATUS    status;
    cloCODE_GENERATOR codeGenerator;
    cltELEMENT_TYPE elementType;
    clsSELECTION_CONTEXT selectionContextIntConv, selectionContextDiff, selectionContextHalfway, selectionContextZero;
    clsIOPERAND    intermIOperands[6];
    clsIOPERAND tempIOperand[2];
    clsROPERAND    intermROperands[6];
    clsROPERAND    oneROperand[1], zeroROperand[1];
    gcsTARGET    refTarget[1], intermTarget[1];
    gcsTARGET tempTarget[1], convTarget[1];
    gcsSOURCE   refSource[1], intermSource[1];
    gcsSOURCE tempSource[1], convSource[1];
    gcsSOURCE    zeroSource[1], oneSource[1];


    gcmASSERT(Target);
    gcmASSERT(Source);

    codeGenerator = cloCOMPILER_GetCodeGenerator(Compiler);

    if (codeGenerator->fpConfig & cldFpROUND_TO_NEAREST) {
        return _EmitCode(Compiler,
                         LineNo,
                         StringNo,
                         gcSL_I2F,
                         Target,
                         Source,
                         gcvNULL);
    }

    clsIOPERAND_New(Compiler, &intermIOperands[0], Source->dataType);
    clsROPERAND_InitializeUsingIOperand(&intermROperands[0], &intermIOperands[0]);

    clsIOPERAND_New(Compiler, &intermIOperands[1], clmGenCodeDataType(T_FLOAT));
    clsROPERAND_InitializeUsingIOperand(&intermROperands[1], &intermIOperands[1]);

    elementType = clmGEN_CODE_elementType_GET(Source->dataType);
    if(clmIsElementTypeSigned(elementType)) {
        clsIOPERAND_New(Compiler, &intermIOperands[2], clmGenCodeDataType(T_INT));
        clsROPERAND_InitializeUsingIOperand(&intermROperands[2], &intermIOperands[2]);

        clsIOPERAND_New(Compiler, &intermIOperands[3], clmGenCodeDataType(T_INT));
        clsROPERAND_InitializeUsingIOperand(&intermROperands[3], &intermIOperands[3]);

        clsIOPERAND_New(Compiler, &tempIOperand[0], clmGenCodeDataType(T_INT));

    } else {
        clsIOPERAND_New(Compiler, &intermIOperands[2], clmGenCodeDataType(T_UINT));
        clsROPERAND_InitializeUsingIOperand(&intermROperands[2], &intermIOperands[2]);

        clsIOPERAND_New(Compiler, &intermIOperands[3], clmGenCodeDataType(T_UINT));
        clsROPERAND_InitializeUsingIOperand(&intermROperands[3], &intermIOperands[3]);

        clsIOPERAND_New(Compiler, &tempIOperand[0], clmGenCodeDataType(T_UINT));
    }

    clsIOPERAND_New(Compiler, &intermIOperands[4], clmGenCodeDataType(T_FLOAT));
    clsROPERAND_InitializeUsingIOperand(&intermROperands[4], &intermIOperands[4]);

    clsIOPERAND_New(Compiler, &intermIOperands[5], clmGenCodeDataType(T_UINT));
    clsROPERAND_InitializeUsingIOperand(&intermROperands[5], &intermIOperands[5]);

    clsROPERAND_InitializeIntOrIVecConstant(oneROperand, clmGenCodeDataType(T_UINT), 0x00000001);
    clsROPERAND_InitializeIntOrIVecConstant(zeroROperand, clmGenCodeDataType(T_UINT), 0x00000000);

    gcsSOURCE_InitializeUintConstant(zeroSource, (gctUINT32) 0);
    gcsSOURCE_InitializeUintConstant(oneSource, (gctUINT32) 1);

    gcsTARGET_InitializeUsingIOperand(Compiler, tempTarget, &tempIOperand[0]);
    gcsSOURCE_InitializeUsingIOperand(Compiler, tempSource, &tempIOperand[0]);

    /* r0 */
    gcsTARGET_InitializeUsingIOperand(Compiler, refTarget, &intermIOperands[0]);
    gcsSOURCE_InitializeUsingIOperand(Compiler, refSource, &intermIOperands[0]);
    status = _EmitCode(Compiler,
                       LineNo,
                       StringNo,
                       gcSL_MOV,
                       refTarget,
                       Source,
                       gcvNULL);
    if (gcmIS_ERROR(status)) return status;

    /* r1 = float(r0) */
    gcsTARGET_InitializeUsingIOperand(Compiler, intermTarget, &intermIOperands[1]);
    status = _EmitCode(Compiler,
                       LineNo,
                       StringNo,
                       gcSL_I2F,
                       intermTarget,
                       refSource,
                       gcvNULL);
    if (gcmIS_ERROR(status)) return status;

    /* r2 = int(r1) */
    gcsTARGET_InitializeUsingIOperand(Compiler, intermTarget, &intermIOperands[2]);
    gcsSOURCE_InitializeUsingIOperand(Compiler, intermSource, &intermIOperands[1]);
    status = _EmitCode(Compiler,
                       LineNo,
                       StringNo,
                       gcSL_F2I,
                       intermTarget,
                       intermSource,
                       gcvNULL);
    if (gcmIS_ERROR(status)) return status;

    status = clDefineSelectionBegin(Compiler,
                                    codeGenerator,
                                    gcvTRUE,
                                    &selectionContextIntConv);
    if (gcmIS_ERROR(status)) return status;

    /* r0 == r2 ? */
    gcsSOURCE_InitializeUsingIOperand(Compiler, intermSource, &intermIOperands[2]);
    status = clEmitCompareBranchCode(Compiler,
                                     LineNo,
                                     StringNo,
                                     clvOPCODE_JUMP,
                                     clvCONDITION_EQUAL,
                                     clGetSelectionConditionLabel(&selectionContextIntConv),
                                     refSource,
                                     intermSource);
    if (gcmIS_ERROR(status)) return status;

    status = clDefineSelectionTrueOperandBegin(Compiler,
                                               codeGenerator,
                                               &selectionContextIntConv);
    if (gcmIS_ERROR(status)) return status;

    /* r5 = (uint) r1 */
    gcsTARGET_InitializeUsingIOperand(Compiler, intermTarget, &intermIOperands[5]);
    gcsSOURCE_InitializeUsingIOperand(Compiler, intermSource, &intermIOperands[1]);

    gcsSOURCE_InitializeTargetFormat(convSource, intermTarget->dataType);
    status = _EmitCode(Compiler,
               LineNo,
               StringNo,
               gcSL_CONV,
               intermTarget,
               intermSource,
               convSource);
    if (gcmIS_ERROR(status)) return status;

    /* r5 = ((uint)r1) + 0x1 */
    gcsSOURCE_InitializeUsingIOperand(Compiler, intermSource, &intermIOperands[5]);

    clsIOPERAND_New(Compiler, &tempIOperand[1], clmGenCodeDataType(T_UINT));
    gcsSOURCE_InitializeUsingIOperand(Compiler, convSource, &tempIOperand[1]);
    gcsTARGET_InitializeUsingIOperand(Compiler, convTarget, &tempIOperand[1]);

    status = _EmitCode(Compiler,
                       LineNo,
                       StringNo,
                       gcSL_ADD,
                       convTarget,
                       oneSource,
                       intermSource);
    if (gcmIS_ERROR(status)) return status;

    status = _EmitCode(Compiler,
                       LineNo,
                       StringNo,
                       gcSL_MOV,
                       intermTarget,
                       convSource,
                       gcvNULL);
    if (gcmIS_ERROR(status)) return status;

    /* r4 = (float) r5 */
    gcsTARGET_InitializeUsingIOperand(Compiler, intermTarget, &intermIOperands[4]);
    status = _EmitCode(Compiler,
                       LineNo,
                       StringNo,
                       gcSL_MOV,
                       intermTarget,
                       intermSource,
                       gcvNULL);
    if (gcmIS_ERROR(status)) return status;

    /* r3 = int(r4) */
    gcsTARGET_InitializeUsingIOperand(Compiler, intermTarget, &intermIOperands[3]);
    gcsSOURCE_InitializeUsingIOperand(Compiler, intermSource, &intermIOperands[4]);
    status = _EmitCode(Compiler,
                       LineNo,
                       StringNo,
                       gcSL_F2I,
                       intermTarget,
                       intermSource,
                       gcvNULL);
    if (gcmIS_ERROR(status)) return status;

    /* r3 = r3 - r0 */
    gcsSOURCE_InitializeUsingIOperand(Compiler, intermSource, &intermIOperands[3]);
    status = _EmitCode(Compiler,
                       LineNo,
                       StringNo,
                       gcSL_SUB,
                       tempTarget,
                       intermSource,
                       refSource);
    if (gcmIS_ERROR(status)) return status;

    status = _EmitCode(Compiler,
                       LineNo,
                       StringNo,
                       gcSL_MOV,
                       intermTarget,
                       tempSource,
                       gcvNULL);
    if (gcmIS_ERROR(status)) return status;

    /* r0 = r0 - r2 */
    gcsSOURCE_InitializeUsingIOperand(Compiler, intermSource, &intermIOperands[2]);
    status = _EmitCode(Compiler,
                       LineNo,
                       StringNo,
                       gcSL_SUB,
                       tempTarget,
                       refSource,
                       intermSource);
    if (gcmIS_ERROR(status)) return status;

    status = _EmitCode(Compiler,
                       LineNo,
                       StringNo,
                       gcSL_MOV,
                       refTarget,
                       tempSource,
                       gcvNULL);
    if (gcmIS_ERROR(status)) return status;

    status = clDefineSelectionBegin(Compiler,
                                    codeGenerator,
                                    gcvTRUE,
                                    &selectionContextHalfway);
    if (gcmIS_ERROR(status)) return status;

    /* r3 != r0 ? */
    gcsSOURCE_InitializeUsingIOperand(Compiler, intermSource, &intermIOperands[3]);
    status = clEmitCompareBranchCode(Compiler,
                                     LineNo,
                                     StringNo,
                                     clvOPCODE_JUMP,
                                     clvCONDITION_NOT_EQUAL,
                                     clGetSelectionConditionLabel(&selectionContextHalfway),
                                     intermSource,
                                     refSource);
    if (gcmIS_ERROR(status)) return status;

    status = clDefineSelectionTrueOperandBegin(Compiler,
                                               codeGenerator,
                                               &selectionContextHalfway);
    if (gcmIS_ERROR(status)) return status;

    /* r0 = r4 & 0x1 */
    gcsSOURCE_InitializeUsingIOperand(Compiler, intermSource, &intermIOperands[4]);
    status = _EmitCode(Compiler,
                       LineNo,
                       StringNo,
                       gcSL_AND_BITWISE,
                       refTarget,
                       intermSource,
                       oneSource);
    if (gcmIS_ERROR(status)) return status;

    status = clDefineSelectionBegin(Compiler,
                                    codeGenerator,
                                    gcvTRUE,
                                    &selectionContextZero);
    if (gcmIS_ERROR(status)) return status;

    /* r0 != 0 ? */
    status = clEmitCompareBranchCode(Compiler,
                                     LineNo,
                                     StringNo,
                                     clvOPCODE_JUMP,
                                     clvCONDITION_NOT_EQUAL,
                                     clGetSelectionConditionLabel(&selectionContextZero),
                                     refSource,
                                     zeroSource);
    if (gcmIS_ERROR(status)) return status;

    status = clDefineSelectionTrueOperandBegin(Compiler,
                                               codeGenerator,
                                               &selectionContextZero);
    if (gcmIS_ERROR(status)) return status;

    /* r4 is even, pick r4 */
    gcsTARGET_InitializeUsingIOperand(Compiler, intermTarget, &intermIOperands[1]);
    gcsSOURCE_InitializeUsingIOperand(Compiler, intermSource, &intermIOperands[4]);
    status = _EmitCode(Compiler,
                       LineNo,
                       StringNo,
                       gcSL_MOV,
                       intermTarget,
                       intermSource,
                       gcvNULL);
    if (gcmIS_ERROR(status)) return status;

    status = clDefineSelectionTrueOperandEnd(Compiler,
                                             LineNo,
                                             StringNo,
                                             codeGenerator,
                                             &selectionContextZero,
                                             gcvFALSE);
    if (gcmIS_ERROR(status)) return status;

    status = clDefineSelectionFalseOperandBegin(Compiler,
                                            codeGenerator,
                                            &selectionContextZero);
    if (gcmIS_ERROR(status)) return status;

    status = clDefineSelectionFalseOperandEnd(Compiler,
                                              codeGenerator,
                                              &selectionContextZero);
    if (gcmIS_ERROR(status)) return status;

    status = clDefineSelectionEnd(Compiler,
                                  codeGenerator,
                                  &selectionContextZero);
    if (gcmIS_ERROR(status)) return status;

    status = clDefineSelectionTrueOperandEnd(Compiler,
                                             LineNo,
                                             StringNo,
                                             codeGenerator,
                                             &selectionContextHalfway,
                                             gcvFALSE);
    if (gcmIS_ERROR(status)) return status;

    status = clDefineSelectionFalseOperandBegin(Compiler,
                                                codeGenerator,
                                                &selectionContextHalfway);
    if (gcmIS_ERROR(status)) return status;

    status = clDefineSelectionBegin(Compiler,
                                    codeGenerator,
                                    gcvTRUE,
                                    &selectionContextDiff);
    if (gcmIS_ERROR(status)) return status;

    if(clmIsElementTypeSigned(elementType)) {
       status = _EmitCode(Compiler,
                          LineNo,
                          StringNo,
                          gcSL_ABS,
                          tempTarget,
                          refSource,
                          gcvNULL);
       if (gcmIS_ERROR(status)) return status;

       status = _EmitCode(Compiler,
                          LineNo,
                          StringNo,
                          gcSL_MOV,
                          refTarget,
                          tempSource,
                          gcvNULL);
       if (gcmIS_ERROR(status)) return status;

       gcsSOURCE_InitializeUsingIOperand(Compiler, intermSource, &intermIOperands[3]);
       gcsTARGET_InitializeUsingIOperand(Compiler, intermTarget, &intermIOperands[3]);
       status = _EmitCode(Compiler,
                          LineNo,
                          StringNo,
                          gcSL_ABS,
                          tempTarget,
                          intermSource,
                          gcvNULL);
       if (gcmIS_ERROR(status)) return status;

       status = _EmitCode(Compiler,
                          LineNo,
                          StringNo,
                          gcSL_MOV,
                          intermTarget,
                          tempSource,
                          gcvNULL);
       if (gcmIS_ERROR(status)) return status;
    }

    /* r3 >= r0 ? */
    gcsSOURCE_InitializeUsingIOperand(Compiler, intermSource, &intermIOperands[3]);

    status = clEmitCompareBranchCode(Compiler,
                                     LineNo,
                                     StringNo,
                                     clvOPCODE_JUMP,
                                     clvCONDITION_GREATER_THAN_EQUAL,
                                     clGetSelectionConditionLabel(&selectionContextDiff),
                                     intermSource,
                                     refSource);
    if (gcmIS_ERROR(status)) return status;

    status = clDefineSelectionTrueOperandBegin(Compiler,
                                               codeGenerator,
                                               &selectionContextDiff);
    if (gcmIS_ERROR(status)) return status;

    /* r4 is nearer, pick r4 */
    gcsTARGET_InitializeUsingIOperand(Compiler, intermTarget, &intermIOperands[1]);
    gcsSOURCE_InitializeUsingIOperand(Compiler, intermSource, &intermIOperands[4]);
    status = _EmitCode(Compiler,
                       LineNo,
                       StringNo,
                       gcSL_MOV,
                       intermTarget,
                       intermSource,
                       gcvNULL);
    if (gcmIS_ERROR(status)) return status;

    status = clDefineSelectionTrueOperandEnd(Compiler,
                                             LineNo,
                                             StringNo,
                                             codeGenerator,
                                             &selectionContextDiff,
                                             gcvFALSE);
    if (gcmIS_ERROR(status)) return status;

    status = clDefineSelectionFalseOperandBegin(Compiler,
                                                codeGenerator,
                                                &selectionContextDiff);
    if (gcmIS_ERROR(status)) return status;

    status = clDefineSelectionFalseOperandEnd(Compiler,
                                              codeGenerator,
                                              &selectionContextDiff);
    if (gcmIS_ERROR(status)) return status;

    status = clDefineSelectionEnd(Compiler,
                                  codeGenerator,
                                  &selectionContextDiff);
    if (gcmIS_ERROR(status)) return status;

    status = clDefineSelectionFalseOperandEnd(Compiler,
                                              codeGenerator,
                                              &selectionContextHalfway);
    if (gcmIS_ERROR(status)) return status;

    status = clDefineSelectionEnd(Compiler,
                                  codeGenerator,
                                  &selectionContextHalfway);
    if (gcmIS_ERROR(status)) return status;

    status = clDefineSelectionTrueOperandEnd(Compiler,
                                             LineNo,
                                             StringNo,
                                             codeGenerator,
                                             &selectionContextIntConv,
                                             gcvFALSE);
    if (gcmIS_ERROR(status)) return status;

    status = clDefineSelectionFalseOperandBegin(Compiler,
                                                codeGenerator,
                                                &selectionContextIntConv);
    if (gcmIS_ERROR(status)) return status;

    status = clDefineSelectionFalseOperandEnd(Compiler,
                                              codeGenerator,
                                              &selectionContextIntConv);
    if (gcmIS_ERROR(status)) return status;

    status = clDefineSelectionEnd(Compiler,
                                  codeGenerator,
                                  &selectionContextIntConv);
    if (gcmIS_ERROR(status)) return status;

    gcsSOURCE_InitializeUsingIOperand(Compiler, intermSource, &intermIOperands[1]);
    return  _EmitCode(Compiler,
                      LineNo,
                      StringNo,
                      gcSL_MOV,
                      Target,
                      intermSource,
                      gcvNULL);
}

static gceSTATUS
_EmitFloatToIntCode(
    IN cloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gcsTARGET * Target,
    IN gcsSOURCE * Source
    )
{
    gceSTATUS    status = gcvSTATUS_OK;
    cltELEMENT_TYPE    elementType;
    gcsSOURCE    format[1];

    gcmASSERT(Target);
    gcmASSERT(Source);

    elementType = clmGEN_CODE_elementType_GET(Target->dataType);
    switch(elementType)
    {
    case clvTYPE_INT:
    case clvTYPE_UINT:
        status =  _EmitCode(Compiler,
                            LineNo,
                            StringNo,
                            gcSL_F2I,
                            Target,
                            Source,
                            gcvNULL);
        break;

    case clvTYPE_LONG:
    case clvTYPE_ULONG:
       gcsSOURCE_InitializeTargetFormat(format, Source->dataType);
       status = _EmitCode(Compiler,
                          LineNo,
                          StringNo,
                          gcSL_CONV,
                          Target,
                          Source,
                          format);
       break;

    default:
        {
           clsIOPERAND    intermIOperand[1];
           gcsTARGET    intermTarget[1];
           gcsSOURCE    intermSource[1];
           clsGEN_CODE_DATA_TYPE dataType;

           dataType = Target->dataType;
           if(clmIsElementTypeUnsigned(elementType)) {
              clmGEN_CODE_elementType_SET(dataType, clvTYPE_UINT);
           }
           else {
              clmGEN_CODE_elementType_SET(dataType, clvTYPE_INT);
           }
           clsIOPERAND_New(Compiler, intermIOperand, dataType);
           gcsTARGET_InitializeUsingIOperand(Compiler, intermTarget, intermIOperand);

           gcsTARGET_Initialize(intermTarget,
                                dataType,
                                intermIOperand->tempRegIndex,
                                Target->enable,
                                Target->indexMode,
                                Target->indexRegIndex);
           status = _EmitCode(Compiler,
                              LineNo,
                              StringNo,
                              gcSL_F2I,
                              intermTarget,
                              Source,
                              gcvNULL);
           if (gcmIS_ERROR(status)) return status;

           gcsSOURCE_InitializeUsingIOperand(Compiler, intermSource, intermIOperand);
           gcsSOURCE_InitializeTargetFormat(format, Target->dataType);
           status = _EmitCode(Compiler,
                              LineNo,
                              StringNo,
                              gcSL_CONV,
                              Target,
                              intermSource,
                              format);
        }
        break;
    }

    return status;
}

static gceSTATUS
_EmitFloatToHalfCode(
    IN cloCOMPILER Compiler,
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

    gcmVERIFY_OK(cloCOMPILER_GetBinary(Compiler, &binary));
    status = _EmitOpcodeAndTarget(Compiler,
                                  LineNo,
                                  StringNo,
                                  gcSL_CONV,
                                  Target);

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
    IN cloCOMPILER Compiler,
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

    gcmVERIFY_OK(cloCOMPILER_GetBinary(Compiler, &binary));
    status = _EmitOpcodeAndTarget(Compiler,
                                  LineNo,
                                  StringNo,
                                  gcSL_CONV,
                                  Target);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    status = _EmitSource(Compiler,
                         LineNo,
                         StringNo,
                         Source);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    format[0] = gcSL_FLOAT16;
    status = gcSHADER_AddSourceConstantFormatted(binary, format, gcSL_UINT32);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

static gceSTATUS
_EmitScalarFloatOrIntToBoolCode(
    IN cloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gcsTARGET * Target,
    IN gcsSOURCE * Source
    )
{
    gceSTATUS    status;
    gctLABEL    endLabel;
    gcsSOURCE    constSource[1];

    endLabel    = clNewLabel(Compiler);

    /* jump end if !(source) */
    status = clEmitTestBranchCode(Compiler,
                      LineNo,
                      StringNo,
                      clvOPCODE_JUMP,
                      endLabel,
                      gcvFALSE,
                      Source);

    if (gcmIS_ERROR(status)) return status;

    /* mov target, true */
    gcsSOURCE_InitializeBoolConstant(constSource, gcvTRUE);

    status = _EmitCode(Compiler,
               LineNo,
               StringNo,
               gcSL_MOV,
               Target,
               constSource,
               gcvNULL);

    if (gcmIS_ERROR(status)) return status;

    /* end: */
    status = clSetLabel(Compiler, LineNo, StringNo, endLabel);

    if (gcmIS_ERROR(status)) return status;

    return gcvSTATUS_OK;
}

static gceSTATUS
_EmitFloatOrIntToBoolCode(
    IN cloCOMPILER Compiler,
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
    cltELEMENT_TYPE elementType;
    gcsSOURCE    constSource[1];

    gcmASSERT(Target);
    gcmASSERT(Source);

    /*
    ** We need to move FALSE to the target first, because the result of bool(-0.0) is 0,
    ** if we don't move first, the result is 0x80000000, which is wrong.
    */
    /* mov target, source */
    gcsSOURCE_InitializeBoolConstant(constSource, gcvFALSE);

    status = _EmitCode(Compiler,
               LineNo,
               StringNo,
               gcSL_MOV,
               Target,
               constSource,
               gcvNULL);

    if (gcmIS_ERROR(status)) return status;

    elementType = clmGEN_CODE_elementType_GET(Target->dataType);
    gcmASSERT(clmIsElementTypeBoolean(elementType));
    if(gcIsScalarDataType(Target->dataType)) {
        status = _EmitScalarFloatOrIntToBoolCode(Compiler,
                             LineNo,
                             StringNo,
                             Target,
                             Source);

        if (gcmIS_ERROR(status)) return status;
    }
    else {
        for (i = 0; i < gcGetVectorDataTypeComponentCount(Target->dataType); i++)
        {
            gcsTARGET_InitializeAsVectorComponent(&componentTarget, Target, i);
            gcsSOURCE_InitializeAsVectorComponent(&componentSource, Source, i);

            status = _EmitScalarFloatOrIntToBoolCode(Compiler,
                                 LineNo,
                                 StringNo,
                                 &componentTarget,
                                 &componentSource);
            if (gcmIS_ERROR(status)) return status;
        }
    }

    return gcvSTATUS_OK;
}

static gceSTATUS
_EmitAnyCode(
    IN cloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gcsTARGET * Target,
    IN gcsSOURCE * Source
    )
{
    gceSTATUS    status;
    gctLABEL    elseLabel, endLabel;
    gcsSOURCE    constSource;

    elseLabel    = clNewLabel(Compiler);
    endLabel    = clNewLabel(Compiler);

    /* jump else if all components are false */
    status = clEmitTestBranchCode(
                                Compiler,
                                LineNo,
                                StringNo,
                                clvOPCODE_JUMP,
                                elseLabel,
                                gcvFALSE,
                                Source);

    if (gcmIS_ERROR(status)) return status;

    /* mov target, true */
    gcsSOURCE_InitializeBoolConstant(&constSource, gcvTRUE);

    status = _EmitCode(
                        Compiler,
                        LineNo,
                        StringNo,
                        gcSL_MOV,
                        Target,
                        &constSource,
                        gcvNULL);

    if (gcmIS_ERROR(status)) return status;

    /* jump end */
    status = clEmitAlwaysBranchCode(
                                    Compiler,
                                    LineNo,
                                    StringNo,
                                    clvOPCODE_JUMP,
                                    endLabel);

    if (gcmIS_ERROR(status)) return status;

    /* else: */
    status = clSetLabel(Compiler, LineNo, StringNo, elseLabel);

    if (gcmIS_ERROR(status)) return status;

    /* mov target, false */
    gcsSOURCE_InitializeBoolConstant(&constSource, gcvFALSE);

    status = _EmitCode(
                        Compiler,
                        LineNo,
                        StringNo,
                        gcSL_MOV,
                        Target,
                        &constSource,
                        gcvNULL);

    if (gcmIS_ERROR(status)) return status;

    /* end: */
    status = clSetLabel(Compiler, LineNo, StringNo, endLabel);

    if (gcmIS_ERROR(status)) return status;

    return gcvSTATUS_OK;
}

static gceSTATUS
_EmitAllCode(
    IN cloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gcsTARGET * Target,
    IN gcsSOURCE * Source
    )
{
    gceSTATUS    status;
    gctLABEL    elseLabel, endLabel;
    gcsSOURCE    constSource;

    elseLabel    = clNewLabel(Compiler);
    endLabel    = clNewLabel(Compiler);

    /* jump else if all components are true */
    status = clEmitTestBranchCode(
                                Compiler,
                                LineNo,
                                StringNo,
                                clvOPCODE_JUMP,
                                elseLabel,
                                gcvTRUE,
                                Source);

    if (gcmIS_ERROR(status)) return status;

    /* mov target, false */
    gcsSOURCE_InitializeBoolConstant(&constSource, gcvFALSE);

    status = _EmitCode(
                        Compiler,
                        LineNo,
                        StringNo,
                        gcSL_MOV,
                        Target,
                        &constSource,
                        gcvNULL);

    if (gcmIS_ERROR(status)) return status;

    /* jump end */
    status = clEmitAlwaysBranchCode(
                                    Compiler,
                                    LineNo,
                                    StringNo,
                                    clvOPCODE_JUMP,
                                    endLabel);

    if (gcmIS_ERROR(status)) return status;

    /* else: */
    status = clSetLabel(Compiler, LineNo, StringNo, elseLabel);

    if (gcmIS_ERROR(status)) return status;

    /* mov target, true */
    gcsSOURCE_InitializeBoolConstant(&constSource, gcvTRUE);

    status = _EmitCode(
                        Compiler,
                        LineNo,
                        StringNo,
                        gcSL_MOV,
                        Target,
                        &constSource,
                        gcvNULL);

    if (gcmIS_ERROR(status)) return status;

    /* end: */
    status = clSetLabel(Compiler, LineNo, StringNo, endLabel);

    if (gcmIS_ERROR(status)) return status;

    return gcvSTATUS_OK;
}

static gceSTATUS
_EmitVectorNotCode(
    IN cloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gcsTARGET * Target,
    IN gcsSOURCE * Source
    )
{
    gceSTATUS status;
    cltELEMENT_TYPE elementType;
    gcsSOURCE falseSource[1];
    cloCODE_GENERATOR   codeGenerator;
    cltELEMENT_TYPE     sourceType;

    codeGenerator = cloCOMPILER_GetCodeGenerator(Compiler);

    gcmASSERT(Source);

    status = _EmitOpcodeConditionAndTarget(Compiler,
                                           LineNo,
                                           StringNo,
                                           gcSL_CMP,
                                           gcSL_EQUAL,
                                           Target);
    if (gcmIS_ERROR(status)) return status;

    if (Source && !codeGenerator->fulllySupportIntegerBranch)
    {
        sourceType = Source->dataType.elementType;

        if (sourceType == clvTYPE_CHAR || sourceType == clvTYPE_SHORT)
        {
            Source->dataType.elementType = clvTYPE_INT;
        }
        else if (sourceType == clvTYPE_UCHAR || sourceType == clvTYPE_USHORT)
        {
            Source->dataType.elementType = clvTYPE_UINT;
        }
    }

    if(Source) {
        status = _EmitSource(Compiler,
                             LineNo,
                             StringNo,
                             Source);
        if (gcmIS_ERROR(status)) return status;
    }

    elementType = clmGEN_CODE_elementType_GET(Source->dataType);
    if(clmIsElementTypeBoolean(elementType)) {
        gcsSOURCE_InitializeBoolConstant(falseSource, gcvFALSE);
    }
    else if(clmIsElementTypeFloating(elementType)) {
        gcsSOURCE_InitializeFloatConstant(falseSource, (gctFLOAT)0.0);
    }
    else {
        gcsSOURCE_InitializeIntConstant(falseSource, (gctINT32) 0);
    }

    if (!codeGenerator->fulllySupportIntegerBranch)
    {
        sourceType = falseSource->dataType.elementType;

        if (sourceType == clvTYPE_CHAR || sourceType == clvTYPE_SHORT)
        {
            falseSource->dataType.elementType = clvTYPE_INT;
        }
        else if (sourceType == clvTYPE_UCHAR || sourceType == clvTYPE_USHORT)
        {
            falseSource->dataType.elementType = clvTYPE_UINT;
        }
    }

    status = _EmitSource(Compiler,
                         LineNo,
                         StringNo,
                         falseSource);
    if (gcmIS_ERROR(status)) return status;

    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                                  clvDUMP_CODE_EMITTER,
                                  "</INSTRUCTION>"));

    return gcvSTATUS_OK;
}

static gceSTATUS
_EmitScalarNotCode(
    IN cloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gcsTARGET * Target,
    IN gcsSOURCE * Source
    )
{
    gceSTATUS    status;
    gctLABEL    elseLabel, endLabel;
    gcsSOURCE    constSource;

    elseLabel    = clNewLabel(Compiler);
    endLabel    = clNewLabel(Compiler);

    /* jump else if (source == true) */
    status = clEmitTestBranchCode(Compiler,
                                  LineNo,
                                  StringNo,
                                  clvOPCODE_JUMP,
                                  elseLabel,
                                  gcvTRUE,
                                  Source);

    if (gcmIS_ERROR(status)) return status;

    /* mov target, 1 */
    gcsSOURCE_InitializeIntConstant(&constSource, 1);

    status = _EmitCode(Compiler,
                       LineNo,
                       StringNo,
                       gcSL_MOV,
                       Target,
                       &constSource,
                       gcvNULL);
    if (gcmIS_ERROR(status)) return status;

    /* jump end */
    status = clEmitAlwaysBranchCode(Compiler,
                                    LineNo,
                                    StringNo,
                                    clvOPCODE_JUMP,
                                    endLabel);

    if (gcmIS_ERROR(status)) return status;

    /* else: */
    status = clSetLabel(Compiler, LineNo, StringNo, elseLabel);

    if (gcmIS_ERROR(status)) return status;

    /* mov target, 0 */
    gcsSOURCE_InitializeIntConstant(&constSource, 0);

    status = _EmitCode(Compiler,
                       LineNo,
                       StringNo,
                       gcSL_MOV,
                       Target,
                       &constSource,
                       gcvNULL);
    if (gcmIS_ERROR(status)) return status;

    /* end: */
    status = clSetLabel(Compiler, LineNo, StringNo, endLabel);

    if (gcmIS_ERROR(status)) return status;

    return gcvSTATUS_OK;
}

static gceSTATUS
_EmitNotCode(
    IN cloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gcsTARGET * Target,
    IN gcsSOURCE * Source
    )
{
    gceSTATUS    status;

    gcmASSERT(Target);
    gcmASSERT(Source);

        if(gcIsScalarDataType(Target->dataType)) {
        status = _EmitScalarNotCode(Compiler,
                        LineNo,
                        StringNo,
                        Target,
                        Source);
        if (gcmIS_ERROR(status)) return status;
    }
    else {
        status = _EmitVectorNotCode(Compiler,
                        LineNo,
                        StringNo,
                        Target,
                        Source);
        if (gcmIS_ERROR(status)) return status;
    }

    return gcvSTATUS_OK;
}

static gceSTATUS
_EmitDP2Code(
    IN cloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gcsTARGET * Target,
    IN gcsSOURCE * Source0,
    IN gcsSOURCE * Source1
    );

static gceSTATUS
_EmitNORM2Code(
    IN cloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gcsTARGET * Target,
    IN gcsSOURCE * Source
    )
{
    gceSTATUS status;
    clsIOPERAND intermIOperands[2];
    gcsTARGET   intermTargets[2];
    gcsSOURCE  intermSources[2];

    gcmASSERT(Target);
    gcmASSERT(Source);

    /* dp2 t0, source, source */
    clsIOPERAND_New(Compiler, &intermIOperands[0], clmGenCodeDataType(T_FLOAT));
    gcsTARGET_InitializeUsingIOperand(Compiler, &intermTargets[0], &intermIOperands[0]);

    status = _EmitDP2Code(
                    Compiler,
                    LineNo,
                    StringNo,
                    &intermTargets[0],
                    Source,
                    Source);

    if (gcmIS_ERROR(status)) return status;

    /* rsq t1, t0 */
    clsIOPERAND_New(Compiler, &intermIOperands[1], clmGenCodeDataType(T_FLOAT));
    gcsTARGET_InitializeUsingIOperand(Compiler, &intermTargets[1], &intermIOperands[1]);
    gcsSOURCE_InitializeUsingIOperand(Compiler, &intermSources[0], &intermIOperands[0]);

    status = _EmitCode(Compiler,
               LineNo,
               StringNo,
               gcSL_RSQ,
               &intermTargets[1],
               &intermSources[0],
               gcvNULL);

    if (gcmIS_ERROR(status)) return status;

    /* mul target, source, t1 */
    gcsSOURCE_InitializeUsingIOperand(Compiler, &intermSources[1], &intermIOperands[1]);

    status = _EmitCode(
                    Compiler,
                    LineNo,
                    StringNo,
                    gcSL_MUL_Z,
                    Target,
                    Source,
                    &intermSources[1]);

    if (gcmIS_ERROR(status)) return status;

    return gcvSTATUS_OK;
}

static gceSTATUS
_EmitNORM4Code(
    IN cloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gcsTARGET * Target,
    IN gcsSOURCE * Source
    )
{
    gceSTATUS    status;
    clsIOPERAND    intermIOperands[2];
    gcsTARGET    intermTargets[2];
    gcsSOURCE    intermSources[2];

    gcmASSERT(Target);
    gcmASSERT(Source);

    /* dp4 t0, source, source */
    clsIOPERAND_New(Compiler, &intermIOperands[0], clmGenCodeDataType(T_FLOAT));
    gcsTARGET_InitializeUsingIOperand(Compiler, &intermTargets[0], &intermIOperands[0]);

    status = _EmitCode(
                    Compiler,
                    LineNo,
                    StringNo,
                    gcSL_DP4,
                    &intermTargets[0],
                    Source,
                    Source);

    if (gcmIS_ERROR(status)) return status;

    /* rsq t1, t0 */
    clsIOPERAND_New(Compiler, &intermIOperands[1], clmGenCodeDataType(T_FLOAT));
    gcsTARGET_InitializeUsingIOperand(Compiler, &intermTargets[1], &intermIOperands[1]);
    gcsSOURCE_InitializeUsingIOperand(Compiler, &intermSources[0], &intermIOperands[0]);

    status = _EmitCode(
                    Compiler,
                    LineNo,
                    StringNo,
                    gcSL_RSQ,
                    &intermTargets[1],
                    &intermSources[0],
                    gcvNULL);

    if (gcmIS_ERROR(status)) return status;

    /* mul target, source, t1 */
    gcsSOURCE_InitializeUsingIOperand(Compiler, &intermSources[1], &intermIOperands[1]);

    status = _EmitCode(
                    Compiler,
                    LineNo,
                    StringNo,
                    gcSL_MUL_Z,
                    Target,
                    Source,
                    &intermSources[1]);

    if (gcmIS_ERROR(status)) return status;

    return gcvSTATUS_OK;
}

static gceSTATUS
_EmitNormalizeCode(
    IN cloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gcsTARGET * Target,
    IN gcsSOURCE * Source
    )
{
    gcsSOURCE    sourceOne;
    cltELEMENT_TYPE elementType;

    gcmASSERT(Target);
    gcmASSERT(Source);

    elementType = clmGEN_CODE_elementType_GET(Source->dataType);

    if(clmIsElementTypeFloating(elementType) &&
       !gcIsMatrixDataType(Source->dataType)) {

          switch(clmGEN_CODE_vectorSize_NOCHECK_GET(Source->dataType)){
       case 0:
        gcsSOURCE_InitializeFloatConstant(&sourceOne, (gctFLOAT)1.0);

        return _EmitCode(Compiler,
                LineNo,
                StringNo,
                gcSL_MOV,
                Target,
                &sourceOne,
                gcvNULL);

       case 2:
        return _EmitNORM2Code(Compiler,
                LineNo,
                StringNo,
                Target,
                Source);

       case 3:
        return _EmitCode(Compiler,
                LineNo,
                StringNo,
                gcSL_NORM,
                Target,
                Source,
                gcvNULL);

       case 4:
        return _EmitNORM4Code(Compiler,
                      LineNo,
                      StringNo,
                      Target,
                      Source);

           }
    }
    gcmASSERT(0);

    return gcvSTATUS_INVALID_ARGUMENT;
}

static gceSTATUS
_EmitFractRTZCode(
    IN cloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gcsTARGET * Target,
    IN gcsSOURCE * Source
    )
{
    gcmASSERT(Target);
    gcmASSERT(Source);

    gcmASSERT(!gcIsMatrixDataType(Source->dataType));
    gcmASSERT(clmIsElementTypeFloating(clmGEN_CODE_elementType_GET(Source->dataType)));

    if (! cloCOMPILER_GetCodeGenerator(Compiler)->supportRTNE)
    {
        return _EmitCode(Compiler,
                    LineNo,
                    StringNo,
                    gcSL_FRAC,
                    Target,
                    Source,
                    gcvNULL);
    }
    else
    {
        return _EmitCodeWRound(Compiler,
                    LineNo,
                    StringNo,
                    gcSL_FRAC,
                    Target,
                    Source,
                    gcvNULL,
                    gcSL_ROUND_RTZ);
    }
}

static gceSTATUS
_EmitFractRTNECode(
    IN cloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gcsTARGET * Target,
    IN gcsSOURCE * Source
    )
{
    gcmASSERT(Target);
    gcmASSERT(Source);

    gcmASSERT(!gcIsMatrixDataType(Source->dataType));
    gcmASSERT(clmIsElementTypeFloating(clmGEN_CODE_elementType_GET(Source->dataType)));

    if (! cloCOMPILER_GetCodeGenerator(Compiler)->supportRTNE)
    {
        return _EmitCode(Compiler,
                    LineNo,
                    StringNo,
                    gcSL_FRAC,
                    Target,
                    Source,
                    gcvNULL);
    }
    else
    {
        return _EmitCodeWRound(Compiler,
                    LineNo,
                    StringNo,
                    gcSL_FRAC,
                    Target,
                    Source,
                    gcvNULL,
                    gcSL_ROUND_RTNE);
    }
}

static gceSTATUS
_EmitIntToFloatRTZCode(
    IN cloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gcsTARGET * Target,
    IN gcsSOURCE * Source
    )
{
    gcmASSERT(Target);
    gcmASSERT(Source);

    if (! cloCOMPILER_GetCodeGenerator(Compiler)->supportRTNE)
    {
        return _EmitCode(Compiler,
                      LineNo,
                      StringNo,
                      gcSL_I2F,
                      Target,
                      Source,
                      gcvNULL);
    }
    else
    {
        return _EmitCodeWRound(Compiler,
                      LineNo,
                      StringNo,
                      gcSL_I2F,
                      Target,
                      Source,
                      gcvNULL,
                      gcSL_ROUND_RTZ);
    }
}

static gceSTATUS
_EmitIntToFloatRTNECode(
    IN cloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gcsTARGET * Target,
    IN gcsSOURCE * Source
    )
{
    gcmASSERT(Target);
    gcmASSERT(Source);

    if (! cloCOMPILER_GetCodeGenerator(Compiler)->supportRTNE)
    {
        return _EmitCode(Compiler,
                      LineNo,
                      StringNo,
                      gcSL_I2F,
                      Target,
                      Source,
                      gcvNULL);
    }
    else
    {
        return _EmitCodeWRound(Compiler,
                      LineNo,
                      StringNo,
                      gcSL_I2F,
                      Target,
                      Source,
                      gcvNULL,
                      gcSL_ROUND_RTNE);
    }
}

typedef gceSTATUS
(* cltEMIT_SPECIAL_CODE_FUNC_PTR1)(
    IN cloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gcsTARGET * Target,/* Either a pointer or an array */
    IN gcsSOURCE * Source
    );

typedef struct _clsSPECIAL_CODE_EMITTER1
{
    cleOPCODE opcode;

    cltEMIT_SPECIAL_CODE_FUNC_PTR1    codeEmitter;
}
clsSPECIAL_CODE_EMITTER1;

static clsSPECIAL_CODE_EMITTER1 SpecialCodeEmitterTable1[] =
{
    {clvOPCODE_FLOAT_TO_INT,    _EmitFloatToIntCode},
    {clvOPCODE_FLOAT_TO_UINT,    _EmitFloatToIntCode},
    {clvOPCODE_FLOAT_TO_BOOL,    _EmitFloatOrIntToBoolCode},
    {clvOPCODE_FLOAT_TO_HALF,    _EmitFloatToHalfCode},
    {clvOPCODE_HALF_TO_FLOAT,    _EmitHalfToFloatCode},
    {clvOPCODE_INT_TO_BOOL,        _EmitFloatOrIntToBoolCode},
    {clvOPCODE_UINT_TO_BOOL,    _EmitFloatOrIntToBoolCode},
    {clvOPCODE_IMPL_I2F,        _EmitIntToFloatCode},
    {clvOPCODE_IMPL_U2F,        _EmitIntToFloatCode},
    {clvOPCODE_IMPL_B2F,        _EmitIntToFloatCode},

    {clvOPCODE_ANY,            _EmitAnyCode},
    {clvOPCODE_ALL,            _EmitAllCode},
    {clvOPCODE_NOT,            _EmitNotCode},

    {clvOPCODE_NORMALIZE,        _EmitNormalizeCode},

    {clvOPCODE_FRACT_RTZ,            _EmitFractRTZCode},
    {clvOPCODE_FRACT_RTNE,            _EmitFractRTNECode},
    {clvOPCODE_INT_TO_FLOAT_RTZ,    _EmitIntToFloatRTZCode},
    {clvOPCODE_INT_TO_FLOAT_RTNE,    _EmitIntToFloatRTNECode},
    {clvOPCODE_UINT_TO_FLOAT_RTZ,    _EmitIntToFloatRTZCode},
    {clvOPCODE_UINT_TO_FLOAT_RTNE,    _EmitIntToFloatRTNECode},
};

const gctUINT SpecialCodeEmitterCount1 =
                    sizeof(SpecialCodeEmitterTable1) / sizeof(clsSPECIAL_CODE_EMITTER1);

static gceSTATUS
_EmitCodeImpl1(
    IN cloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN cleOPCODE Opcode,
    IN gcsTARGET * Target,
    IN gcsSOURCE * Source
    )
{
    gceSTATUS    status;
    gcsSOURCE    newSource;
    gctUINT        i;
    cltEMIT_SPECIAL_CODE_FUNC_PTR1    specialCodeEmitter = gcvNULL;

    status = _PrepareSource(Compiler,
                LineNo,
                StringNo,
                Target,
                Source,
                &newSource);

    if (gcmIS_ERROR(status)) return status;

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
        status = (*specialCodeEmitter)(Compiler,
                           LineNo,
                           StringNo,
                           Target,
                           &newSource);

        if (gcmIS_ERROR(status)) return status;
    }
    else
    {
        status = _EmitCode(Compiler,
                   LineNo,
                   StringNo,
                   _ConvOpcode(Opcode),
                   Target,
                   &newSource,
                   gcvNULL);

        if (gcmIS_ERROR(status)) return status;
    }

    return gcvSTATUS_OK;
}

gceSTATUS
clEmitCode1(
    IN cloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN cleOPCODE Opcode,
    IN gcsTARGET * Target,
    IN gcsSOURCE * Source
    )
{
    cloCODE_EMITTER    codeEmitter;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);

    if(clmIsOpcodeConv(Opcode)) {
       return clEmitConvCode(Compiler,
                             LineNo,
                             StringNo,
                             Opcode,
                             Target,
                             Source,
                             Target->dataType);
    }
    codeEmitter = cloCOMPILER_GetCodeEmitter(Compiler);
    gcmASSERT(codeEmitter);

    return cloCODE_EMITTER_EmitCode1(Compiler,
                     codeEmitter,
                     LineNo,
                      StringNo,
                     Opcode,
                     Target,
                     Source);
}
static gceSTATUS
gcsSOURCE_CONSTANT_Inverse(
    IN OUT gcsSOURCE * Source
    )
{
    cltELEMENT_TYPE elementType;

    gcmASSERT(Source);
    gcmASSERT(Source->type == gcvSOURCE_CONSTANT);

        elementType = clmGEN_CODE_elementType_GET(Source->dataType);
        if(clmIsElementTypeArithmetic(elementType)) {
           if(clmIsElementTypeFloating(elementType)) {
          Source->u.sourceConstant.floatValue =
            (gctFLOAT)1.0 / Source->u.sourceConstant.floatValue;
       }
           else if(clmIsElementTypeInteger(elementType)) {
          gctINT divisor;

              if(gcIsScalarDataType(Source->dataType)) {
                 clmGEN_CODE_DATA_TYPE_Initialize(Source->dataType, 0, 0, clvTYPE_FLOAT);
          }
          else {
          Source->dataType = gcConvScalarToVectorDataType(clmGenCodeDataType(T_FLOAT),
                                gcGetDataTypeComponentCount(Source->dataType));
          }

              if(clmIsElementTypeBoolean(elementType)) {
             divisor = Source->u.sourceConstant.boolValue;
          }
          else {
             divisor = Source->u.sourceConstant.intValue;
          }
          if(divisor == 0) return gcvSTATUS_INVALID_ARGUMENT;
          Source->u.sourceConstant.floatValue = (gctFLOAT)1.0 / (gctFLOAT)divisor;
       }
    }
    else {
       gcmASSERT(0);
       return gcvSTATUS_INVALID_ARGUMENT;
    }
    return gcvSTATUS_OK;
}

static gceSTATUS
_EmitMulForDivCode(
    IN cloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gcsTARGET * Target,
    IN gcsSOURCE * Source0,
    IN gcsSOURCE * Source1
    )
{
    gceSTATUS    status;
    clsIOPERAND    intermIOperand;
    gcsTARGET    intermTarget;
    gcsSOURCE    intermSource;
    cltELEMENT_TYPE elementType;

    gcmASSERT(Target);
    gcmASSERT(Source0);
    gcmASSERT(Source1);

    elementType = clmGEN_CODE_elementType_GET(Target->dataType);
    if(clmIsElementTypeFloating(elementType)) {
        /* mul target, source0, source1 */
        status = _EmitCode(Compiler,
                   LineNo,
                   StringNo,
                   gcSL_MUL,
                   Target,
                   Source0,
                   Source1);
        if (gcmIS_ERROR(status)) return status;
    }
    else if(clmIsElementTypeInteger(elementType)) {
        /* mul t0, source0, source1 */
        clsIOPERAND_New(Compiler, &intermIOperand, Target->dataType);
        gcsTARGET_InitializeUsingIOperand(Compiler, &intermTarget, &intermIOperand);

        status = _EmitCode(Compiler,
                   LineNo,
                   StringNo,
                   gcSL_MUL,
                   &intermTarget,
                   Source0,
                   Source1);
        if (gcmIS_ERROR(status)) return status;

        /* floor target, t0 */
        gcsSOURCE_InitializeUsingIOperand(Compiler, &intermSource, &intermIOperand);

        status = _EmitCode(Compiler,
                   LineNo,
                   StringNo,
                   gcSL_FLOOR,
                   Target,
                   &intermSource,
                   gcvNULL);
        if (gcmIS_ERROR(status)) return status;
    }
    else {
        gcmASSERT(0);
    }

    return gcvSTATUS_OK;
}

#define _GEN_DIV_IN_BACKEND  1

static gceSTATUS
_EmitDivCode(
    IN cloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gcsTARGET * Target,
    IN gcsSOURCE * Source0,
    IN gcsSOURCE * Source1
    )
{
    gcmASSERT(Target);
    gcmASSERT(Source0);
    gcmASSERT(Source1);

    if(clmIsElementTypeInteger(clmGEN_CODE_elementType_GET(Target->dataType))) {
       return _EmitCode(Compiler,
                LineNo,
                StringNo,
                gcSL_DIV,
                Target,
                Source0,
                Source1);
    }
    else {
      gceSTATUS        status;
      gcsSOURCE        intermSource;

      if (Source1->type == gcvSOURCE_CONSTANT) {
        /* mul target, source0, 1 / constant_source1 */
        intermSource = *Source1;
        status = gcsSOURCE_CONSTANT_Inverse(&intermSource);
        if (gcmIS_ERROR(status)) {
            gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                            LineNo,
                            StringNo,
                            clvREPORT_INTERNAL_ERROR,
                            "divide by zero"));
            return status;
        }

        status = _EmitMulForDivCode(Compiler,
                        LineNo,
                        StringNo,
                        Target,
                        Source0,
                        &intermSource);

        if (gcmIS_ERROR(status)) return status;
      }
      else {

#if _GEN_DIV_IN_BACKEND
        return _EmitCode(Compiler,
                LineNo,
                StringNo,
                gcSL_DIV,
                Target,
                Source0,
                Source1);
#else
        clsIOPERAND    intermIOperand;
        gcsTARGET   intermTarget;

        /* rcp t0, source1 */
        clsIOPERAND_New(Compiler, &intermIOperand, Source1->dataType);
        gcsTARGET_InitializeUsingIOperand(Compiler, &intermTarget, &intermIOperand);

        status = _EmitCode(
                        Compiler,
                        LineNo,
                        StringNo,
                        gcSL_RCP,
                        &intermTarget,
                        Source1,
                        gcvNULL);

        if (gcmIS_ERROR(status)) return status;

        /* mul target, source0, t0 */
        gcsSOURCE_InitializeUsingIOperand(Compiler, &intermSource, &intermIOperand);

        status = _EmitMulForDivCode(
                                    Compiler,
                                    LineNo,
                                    StringNo,
                                    Target,
                                    Source0,
                                    &intermSource);

        if (gcmIS_ERROR(status)) return status;
#endif
      }
    }

    return gcvSTATUS_OK;
}

static gceSTATUS
_EmitScalarAtan2Code(
    IN cloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gcsTARGET * Target,
    IN gcsSOURCE * Source0,
    IN gcsSOURCE * Source1
    )
{
    gceSTATUS        status;
    gctLABEL        else0Label, else1Label, else2Label, else3Label, endLabel, jmp1Label;
    gcsSOURCE        constSource;
    clsIOPERAND        intermIOperands[5];
    gcsTARGET        intermTargets[5];
    gcsSOURCE        intermSources[5];

    else0Label    = clNewLabel(Compiler);
    else1Label    = clNewLabel(Compiler);
    else2Label    = clNewLabel(Compiler);
    else3Label    = clNewLabel(Compiler);
    endLabel    = clNewLabel(Compiler);
    jmp1Label    = clNewLabel(Compiler);

    /* sign t0, y (source0) */
    clsIOPERAND_New(Compiler, &intermIOperands[0], Source0->dataType);
    gcsTARGET_InitializeUsingIOperand(Compiler, &intermTargets[0], &intermIOperands[0]);

    status = _EmitCode(
                        Compiler,
                        LineNo,
                        StringNo,
                        gcSL_SIGN,
                        &intermTargets[0],
                        Source0,
                        gcvNULL);

    if (gcmIS_ERROR(status)) return status;

    /* jump else0 if x (source1) != 0.0 */
    gcsSOURCE_InitializeFloatConstant(&constSource, (gctFLOAT)0.0);

    status = clEmitCompareBranchCode(
                                    Compiler,
                                    LineNo,
                                    StringNo,
                                    clvOPCODE_JUMP,
                                    clvCONDITION_NOT_EQUAL,
                                    else0Label,
                                    Source1,
                                    &constSource);

    if (gcmIS_ERROR(status)) return status;

    /* mul target, t0, _HALF_PI */
    gcsSOURCE_InitializeUsingIOperand(Compiler, &intermSources[0], &intermIOperands[0]);
    gcsSOURCE_InitializeFloatConstant(&constSource, _HALF_PI);

    status = _EmitCode(
                        Compiler,
                        LineNo,
                        StringNo,
                        gcSL_MUL,
                        Target,
                        &intermSources[0],
                        &constSource);

    if (gcmIS_ERROR(status)) return status;

    /* jump end */
    status = clEmitAlwaysBranchCode(
                                    Compiler,
                                    LineNo,
                                    StringNo,
                                    clvOPCODE_JUMP,
                                    endLabel);

    if (gcmIS_ERROR(status)) return status;

    /* else0: */
    status = clSetLabel(Compiler, LineNo, StringNo, else0Label);

    if (gcmIS_ERROR(status)) return status;

    /* div t1, y (source0), x (source1) */
    clsIOPERAND_New(Compiler, &intermIOperands[1], Source0->dataType);
    gcsTARGET_InitializeUsingIOperand(Compiler, &intermTargets[1], &intermIOperands[1]);

    status = _EmitDivCode(
                        Compiler,
                        LineNo,
                        StringNo,
                        &intermTargets[1],
                        Source0,
                        Source1);

    if (gcmIS_ERROR(status)) return status;

    /* abs t2, t1 */
    clsIOPERAND_New(Compiler, &intermIOperands[2], Source0->dataType);
    gcsTARGET_InitializeUsingIOperand(Compiler, &intermTargets[2], &intermIOperands[2]);
    gcsSOURCE_InitializeUsingIOperand(Compiler, &intermSources[1], &intermIOperands[1]);

    status = _EmitCode(
                        Compiler,
                        LineNo,
                        StringNo,
                        gcSL_ABS,
                        &intermTargets[2],
                        &intermSources[1],
                        gcvNULL);

    if (gcmIS_ERROR(status)) return status;

    /* atan t3, t2 */
    clsIOPERAND_New(Compiler, &intermIOperands[3], Source0->dataType);
    gcsTARGET_InitializeUsingIOperand(Compiler, &intermTargets[3], &intermIOperands[3]);
    gcsSOURCE_InitializeUsingIOperand(Compiler, &intermSources[2], &intermIOperands[2]);

    status = _EmitCode(
                        Compiler,
                        LineNo,
                        StringNo,
                        gcSL_ATAN,
                        &intermTargets[3],
                        &intermSources[2],
                        gcvNULL);

    if (gcmIS_ERROR(status)) return status;

    /* jump else1 if x (source1) > 0.0 */
    gcsSOURCE_InitializeFloatConstant(&constSource, (gctFLOAT)0.0);

    status = clEmitCompareBranchCode(
                                    Compiler,
                                    LineNo,
                                    StringNo,
                                    clvOPCODE_JUMP,
                                    clvCONDITION_GREATER_THAN,
                                    else1Label,
                                    Source1,
                                    &constSource);

    if (gcmIS_ERROR(status)) return status;

    /* jump else2 if y (source0) != 0.0 */
    gcsSOURCE_InitializeFloatConstant(&constSource, (gctFLOAT)0.0);

    status = clEmitCompareBranchCode(
                                    Compiler,
                                    LineNo,
                                    StringNo,
                                    clvOPCODE_JUMP,
                                    clvCONDITION_NOT_EQUAL,
                                    else2Label,
                                    Source0,
                                    &constSource);

    if (gcmIS_ERROR(status)) return status;

    /* mov target, _PI */
    gcsSOURCE_InitializeFloatConstant(&constSource, _PI);

    status = _EmitCode(
                        Compiler,
                        LineNo,
                        StringNo,
                        gcSL_MOV,
                        Target,
                        &constSource,
                        gcvNULL);

    if (gcmIS_ERROR(status)) return status;

    /* jump jmp1 */
    status = clEmitAlwaysBranchCode(
                                    Compiler,
                                    LineNo,
                                    StringNo,
                                    clvOPCODE_JUMP,
                                    jmp1Label);

    if (gcmIS_ERROR(status)) return status;

    /* else2: */
    status = clSetLabel(Compiler, LineNo, StringNo, else2Label);

    if (gcmIS_ERROR(status)) return status;

    /* sub t4, _PI, t3 */
    clsIOPERAND_New(Compiler, &intermIOperands[4], Source0->dataType);
    gcsTARGET_InitializeUsingIOperand(Compiler, &intermTargets[4], &intermIOperands[4]);
    gcsSOURCE_InitializeFloatConstant(&constSource, _PI);
    gcsSOURCE_InitializeUsingIOperand(Compiler, &intermSources[3], &intermIOperands[3]);

    status = _EmitCode(
                        Compiler,
                        LineNo,
                        StringNo,
                        gcSL_SUB,
                        &intermTargets[4],
                        &constSource,
                        &intermSources[3]);

    if (gcmIS_ERROR(status)) return status;

    /* mul target, t0, t4 */
    gcsSOURCE_InitializeUsingIOperand(Compiler, &intermSources[0], &intermIOperands[0]);
    gcsSOURCE_InitializeUsingIOperand(Compiler, &intermSources[4], &intermIOperands[4]);

    status = _EmitCode(
                        Compiler,
                        LineNo,
                        StringNo,
                        gcSL_MUL,
                        Target,
                        &intermSources[0],
                        &intermSources[4]);

    if (gcmIS_ERROR(status)) return status;

    /* jmp1: */
    status = clSetLabel(Compiler, LineNo, StringNo, jmp1Label);

    if (gcmIS_ERROR(status)) return status;

    /* jump end */
    status = clEmitAlwaysBranchCode(
                                    Compiler,
                                    LineNo,
                                    StringNo,
                                    clvOPCODE_JUMP,
                                    endLabel);

    if (gcmIS_ERROR(status)) return status;

    /* else1: */
    status = clSetLabel(Compiler, LineNo, StringNo, else1Label);

    if (gcmIS_ERROR(status)) return status;

    /* jump else3 if y (source0) != 0.0 */
    gcsSOURCE_InitializeFloatConstant(&constSource, (gctFLOAT)0.0);

    status = clEmitCompareBranchCode(
                                    Compiler,
                                    LineNo,
                                    StringNo,
                                    clvOPCODE_JUMP,
                                    clvCONDITION_NOT_EQUAL,
                                    else3Label,
                                    Source0,
                                    &constSource);

    if (gcmIS_ERROR(status)) return status;

    /* mov target, 0 */
    gcsSOURCE_InitializeFloatConstant(&constSource, (gctFLOAT)0.0);

    status = _EmitCode(
                        Compiler,
                        LineNo,
                        StringNo,
                        gcSL_MOV,
                        Target,
                        &constSource,
                        gcvNULL);

    if (gcmIS_ERROR(status)) return status;

    /* jump end */
    status = clEmitAlwaysBranchCode(
                                    Compiler,
                                    LineNo,
                                    StringNo,
                                    clvOPCODE_JUMP,
                                    endLabel);

    if (gcmIS_ERROR(status)) return status;

    /* else3: */
    status = clSetLabel(Compiler, LineNo, StringNo, else3Label);

    if (gcmIS_ERROR(status)) return status;

    /* mul target, t0, t3 */
    gcsSOURCE_InitializeUsingIOperand(Compiler, &intermSources[0], &intermIOperands[0]);
    gcsSOURCE_InitializeUsingIOperand(Compiler, &intermSources[3], &intermIOperands[3]);

    status = _EmitCode(
                        Compiler,
                        LineNo,
                        StringNo,
                        gcSL_MUL,
                        Target,
                        &intermSources[0],
                        &intermSources[3]);

    if (gcmIS_ERROR(status)) return status;

    /* end: */
    status = clSetLabel(Compiler, LineNo, StringNo, endLabel);

    if (gcmIS_ERROR(status)) return status;

    return gcvSTATUS_OK;
}

static gceSTATUS
_EmitVectorComponentAtan2SelectionCode(
    IN cloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gcsTARGET * Target,
    IN gcsSOURCE * Source0,
    IN gcsSOURCE * Source1,
    IN gcsSOURCE IntermSources[5]
    )
{
    gceSTATUS        status;
    gctLABEL        else0Label, else1Label, else2Label, else3Label, endLabel;
    gcsSOURCE        constSource;

    else0Label    = clNewLabel(Compiler);
    else1Label    = clNewLabel(Compiler);
    else2Label    = clNewLabel(Compiler);
    else3Label    = clNewLabel(Compiler);
    endLabel    = clNewLabel(Compiler);

    /* jump else0 if x (source1) != 0.0 */
    gcsSOURCE_InitializeFloatConstant(&constSource, (gctFLOAT)0.0);

    status = clEmitCompareBranchCode(
                                    Compiler,
                                    LineNo,
                                    StringNo,
                                    clvOPCODE_JUMP,
                                    clvCONDITION_NOT_EQUAL,
                                    else0Label,
                                    Source1,
                                    &constSource);

    if (gcmIS_ERROR(status)) return status;

    /* mul target, t0, _HALF_PI */
    gcsSOURCE_InitializeFloatConstant(&constSource, _HALF_PI);

    status = _EmitCode(
                        Compiler,
                        LineNo,
                        StringNo,
                        gcSL_MUL,
                        Target,
                        &IntermSources[0],
                        &constSource);

    if (gcmIS_ERROR(status)) return status;

    /* jump end */
    status = clEmitAlwaysBranchCode(
                                    Compiler,
                                    LineNo,
                                    StringNo,
                                    clvOPCODE_JUMP,
                                    endLabel);

    if (gcmIS_ERROR(status)) return status;

    /* else0: */
    status = clSetLabel(Compiler, LineNo, StringNo, else0Label);

    if (gcmIS_ERROR(status)) return status;

    /* jump else1 if x (source1) > 0.0 */
    gcsSOURCE_InitializeFloatConstant(&constSource, (gctFLOAT)0.0);

    status = clEmitCompareBranchCode(
                                    Compiler,
                                    LineNo,
                                    StringNo,
                                    clvOPCODE_JUMP,
                                    clvCONDITION_GREATER_THAN,
                                    else1Label,
                                    Source1,
                                    &constSource);

    if (gcmIS_ERROR(status)) return status;

    /* jump else2 if y (source0) != 0.0 */
    gcsSOURCE_InitializeFloatConstant(&constSource, (gctFLOAT)0.0);

    status = clEmitCompareBranchCode(
                                    Compiler,
                                    LineNo,
                                    StringNo,
                                    clvOPCODE_JUMP,
                                    clvCONDITION_NOT_EQUAL,
                                    else2Label,
                                    Source0,
                                    &constSource);

    if (gcmIS_ERROR(status)) return status;

    /* mov target, _PI */
    gcsSOURCE_InitializeFloatConstant(&constSource, _PI);

    status = _EmitCode(
                        Compiler,
                        LineNo,
                        StringNo,
                        gcSL_MOV,
                        Target,
                        &constSource,
                        gcvNULL);

    if (gcmIS_ERROR(status)) return status;

    /* jump end */
    status = clEmitAlwaysBranchCode(
                                    Compiler,
                                    LineNo,
                                    StringNo,
                                    clvOPCODE_JUMP,
                                    endLabel);

    if (gcmIS_ERROR(status)) return status;

    /* else2: */
    status = clSetLabel(Compiler, LineNo, StringNo, else2Label);

    if (gcmIS_ERROR(status)) return status;

    /* mul target, t0, t4 */
    status = _EmitCode(
                        Compiler,
                        LineNo,
                        StringNo,
                        gcSL_MUL,
                        Target,
                        &IntermSources[0],
                        &IntermSources[4]);

    if (gcmIS_ERROR(status)) return status;

    /* jump end */
    status = clEmitAlwaysBranchCode(
                                    Compiler,
                                    LineNo,
                                    StringNo,
                                    clvOPCODE_JUMP,
                                    endLabel);

    if (gcmIS_ERROR(status)) return status;

    /* else1: */
    status = clSetLabel(Compiler, LineNo, StringNo, else1Label);

    if (gcmIS_ERROR(status)) return status;

    /* jump else3 if y (source0) != 0.0 */
    gcsSOURCE_InitializeFloatConstant(&constSource, (gctFLOAT)0.0);

    status = clEmitCompareBranchCode(
                                    Compiler,
                                    LineNo,
                                    StringNo,
                                    clvOPCODE_JUMP,
                                    clvCONDITION_NOT_EQUAL,
                                    else3Label,
                                    Source0,
                                    &constSource);

    if (gcmIS_ERROR(status)) return status;

    /* mov target, 0 */
    gcsSOURCE_InitializeFloatConstant(&constSource, (gctFLOAT)0.0);

    status = _EmitCode(
                        Compiler,
                        LineNo,
                        StringNo,
                        gcSL_MOV,
                        Target,
                        &constSource,
                        gcvNULL);

    if (gcmIS_ERROR(status)) return status;

    /* jump end */
    status = clEmitAlwaysBranchCode(
                                    Compiler,
                                    LineNo,
                                    StringNo,
                                    clvOPCODE_JUMP,
                                    endLabel);

    if (gcmIS_ERROR(status)) return status;

    /* else3: */
    status = clSetLabel(Compiler, LineNo, StringNo, else3Label);

    if (gcmIS_ERROR(status)) return status;

    /* mul target, t0, t3 */
    status = _EmitCode(
                        Compiler,
                        LineNo,
                        StringNo,
                        gcSL_MUL,
                        Target,
                        &IntermSources[0],
                        &IntermSources[3]);

    if (gcmIS_ERROR(status)) return status;

    /* end: */
    status = clSetLabel(Compiler, LineNo, StringNo, endLabel);

    if (gcmIS_ERROR(status)) return status;

    return gcvSTATUS_OK;
}

static gceSTATUS
_EmitAtan2Code(
    IN cloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gcsTARGET * Target,
    IN gcsSOURCE * Source0,
    IN gcsSOURCE * Source1
    )
{
    gceSTATUS    status;
    gctUINT        i, j;
    gcsTARGET    componentTarget;
    gcsSOURCE    componentSource0, componentSource1;
    gcsSOURCE    constSource;
    clsIOPERAND    intermIOperands[5];
    gcsTARGET    intermTargets[5];
    gcsSOURCE    intermSources[5];
    gcsSOURCE    componentIntermSources[5];

    gcmASSERT(Target);
    gcmASSERT(Source0);
    gcmASSERT(Source1);

    gcmASSERT(!gcIsMatrixDataType(Target->dataType));

        gcmASSERT(clmIsElementTypeFloating(clmGEN_CODE_elementType_GET(Target->dataType)));

    if(gcIsScalarDataType(Target->dataType)) {
        status = _EmitScalarAtan2Code(Compiler,
                          LineNo,
                          StringNo,
                          Target,
                          Source0,
                          Source1);
        if (gcmIS_ERROR(status)) return status;
    }
    else {
        /* sign t0, y (source0) */
        clsIOPERAND_New(Compiler, &intermIOperands[0], Source0->dataType);
        gcsTARGET_InitializeUsingIOperand(Compiler, &intermTargets[0], &intermIOperands[0]);

        status = _EmitCode(Compiler,
                   LineNo,
                   StringNo,
                   gcSL_SIGN,
                   &intermTargets[0],
                   Source0,
                   gcvNULL);
        if (gcmIS_ERROR(status)) return status;

        /* div t1, y (source0), x (source1) */
        clsIOPERAND_New(Compiler, &intermIOperands[1], Source0->dataType);
        gcsTARGET_InitializeUsingIOperand(Compiler, &intermTargets[1], &intermIOperands[1]);

        status = _EmitDivCode(Compiler,
                      LineNo,
                      StringNo,
                      &intermTargets[1],
                      Source0,
                      Source1);
        if (gcmIS_ERROR(status)) return status;

        /* abs t2, t1 */
        clsIOPERAND_New(Compiler, &intermIOperands[2], Source0->dataType);
        gcsTARGET_InitializeUsingIOperand(Compiler, &intermTargets[2], &intermIOperands[2]);
        gcsSOURCE_InitializeUsingIOperand(Compiler, &intermSources[1], &intermIOperands[1]);

        status = _EmitCode(Compiler,
                   LineNo,
                   StringNo,
                   gcSL_ABS,
                   &intermTargets[2],
                   &intermSources[1],
                   gcvNULL);
        if (gcmIS_ERROR(status)) return status;

        /* atan t3, t2 */
        clsIOPERAND_New(Compiler, &intermIOperands[3], Source0->dataType);
        gcsTARGET_InitializeUsingIOperand(Compiler, &intermTargets[3], &intermIOperands[3]);
        gcsSOURCE_InitializeUsingIOperand(Compiler, &intermSources[2], &intermIOperands[2]);

        status = _EmitCode(Compiler,
                   LineNo,
                   StringNo,
                   gcSL_ATAN,
                   &intermTargets[3],
                   &intermSources[2],
                   gcvNULL);
        if (gcmIS_ERROR(status)) return status;

        /* sub t4, _PI, t3 */
        clsIOPERAND_New(Compiler, &intermIOperands[4], Source0->dataType);
        gcsTARGET_InitializeUsingIOperand(Compiler, &intermTargets[4], &intermIOperands[4]);
        gcsSOURCE_InitializeFloatConstant(&constSource, _PI);
        gcsSOURCE_InitializeUsingIOperand(Compiler, &intermSources[3], &intermIOperands[3]);

        status = _EmitCode(Compiler,
                   LineNo,
                   StringNo,
                   gcSL_SUB,
                   &intermTargets[4],
                   &constSource,
                   &intermSources[3]);
        if (gcmIS_ERROR(status)) return status;

        for (i = 0; i < gcGetVectorDataTypeComponentCount(Target->dataType); i++)
        {
            gcsTARGET_InitializeAsVectorComponent(&componentTarget, Target, i);
            gcsSOURCE_InitializeAsVectorComponent(&componentSource0, Source0, i);
            gcsSOURCE_InitializeAsVectorComponent(&componentSource1, Source1, i);

            for (j = 0; j < 5; j++)
            {
                gcsSOURCE_InitializeAsVectorComponent(&componentIntermSources[j],
                                      &intermSources[j],
                                      i);
            }

            status = _EmitVectorComponentAtan2SelectionCode(Compiler,
                                    LineNo,
                                    StringNo,
                                    &componentTarget,
                                    &componentSource0,
                                    &componentSource1,
                                    componentIntermSources);
            if (gcmIS_ERROR(status)) return status;
        }
    }

    return gcvSTATUS_OK;
}

static gceSTATUS
_EmitDP2Code(
    IN cloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gcsTARGET * Target,
    IN gcsSOURCE * Source0,
    IN gcsSOURCE * Source1
    )
{
    gceSTATUS    status;
    clsIOPERAND    intermIOperand;
    gcsTARGET    intermTarget;
    gcsSOURCE    intermSourceX, intermSourceY;

    gcmASSERT(Target);
    gcmASSERT(Source0);
    gcmASSERT(Source1);

    /* mul t0, source0, source1 */
    clsIOPERAND_New(Compiler, &intermIOperand, clmGenCodeDataType(T_FLOAT2));
    gcsTARGET_InitializeUsingIOperand(Compiler, &intermTarget, &intermIOperand);

    status = _EmitCode(
                    Compiler,
                    LineNo,
                    StringNo,
                    gcSL_MUL,
                    &intermTarget,
                    Source0,
                    Source1);

    if (gcmIS_ERROR(status)) return status;

    /* add target, t0.x, t0.y */
    gcsSOURCE_InitializeUsingIOperandAsVectorComponent(&intermSourceX,
                            &intermIOperand,
                            gcSL_SWIZZLE_XXXX);

    gcsSOURCE_InitializeUsingIOperandAsVectorComponent(&intermSourceY,
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

    if (gcmIS_ERROR(status)) return status;

    return gcvSTATUS_OK;
}

static gceSTATUS
_EmitScalarCompareCode(
    IN cloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN cleCONDITION Condition,
    IN gcsTARGET * Target,
    IN gcsSOURCE * Source0,
    IN gcsSOURCE * Source1
    )
{
    gceSTATUS    status;
    gctLABEL    elseLabel, endLabel;
    gcsSOURCE    constSource;

    elseLabel    = clNewLabel(Compiler);
    endLabel    = clNewLabel(Compiler);

    /* jump else if true */
    status = clEmitCompareBranchCode(Compiler,
                     LineNo,
                     StringNo,
                     clvOPCODE_JUMP,
                     Condition,
                     elseLabel,
                     Source0,
                     Source1);
    if (gcmIS_ERROR(status)) return status;

    /* mov target, 0 */
    gcsSOURCE_InitializeIntConstant(&constSource, 0);

    status = _EmitCode(Compiler,
               LineNo,
               StringNo,
               gcSL_MOV,
               Target,
               &constSource,
               gcvNULL);
    if (gcmIS_ERROR(status)) return status;

    /* jump end */
    status = clEmitAlwaysBranchCode(Compiler,
                    LineNo,
                    StringNo,
                    clvOPCODE_JUMP,
                    endLabel);
    if (gcmIS_ERROR(status)) return status;

    /* else: */
    status = clSetLabel(Compiler, LineNo, StringNo, elseLabel);

    if (gcmIS_ERROR(status)) return status;

    /* mov target, 1 */
    gcsSOURCE_InitializeIntConstant(&constSource, 1);

    status = _EmitCode(Compiler,
               LineNo,
               StringNo,
               gcSL_MOV,
               Target,
               &constSource,
               gcvNULL);

    if (gcmIS_ERROR(status)) return status;

    /* end: */
    status = clSetLabel(Compiler, LineNo, StringNo, endLabel);

    if (gcmIS_ERROR(status)) return status;

    return gcvSTATUS_OK;
}

gcSL_CONDITION
_ConvCondition(
    IN cleCONDITION Condition
    )
{
    switch (Condition)
    {
    case clvCONDITION_EQUAL:        return gcSL_EQUAL;
    case clvCONDITION_NOT_EQUAL:        return gcSL_NOT_EQUAL;
    case clvCONDITION_LESS_THAN:        return gcSL_LESS;
    case clvCONDITION_LESS_THAN_EQUAL:    return gcSL_LESS_OR_EQUAL;
    case clvCONDITION_GREATER_THAN:        return gcSL_GREATER;
    case clvCONDITION_GREATER_THAN_EQUAL:    return gcSL_GREATER_OR_EQUAL;
    case clvCONDITION_AND:            return gcSL_AND;
    case clvCONDITION_OR:            return gcSL_OR;
    case clvCONDITION_XOR:            return gcSL_XOR;
    case clvCONDITION_NOT_ZERO:        return gcSL_NOT_ZERO;
    case clvCONDITION_ZERO:            return gcSL_ZERO;

    default:
        gcmASSERT(0);
        return gcSL_EQUAL;
    }
}

static gceSTATUS
_EmitVectorCompareCode(
    IN cloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN cleCONDITION Condition,
    IN gcsTARGET * Target,
    IN gcsSOURCE * Source0,
    IN gcsSOURCE * Source1
    )
{
    gceSTATUS status;
    cloCODE_GENERATOR   codeGenerator;
    cltELEMENT_TYPE     sourceType;

    codeGenerator = cloCOMPILER_GetCodeGenerator(Compiler);

    gcmASSERT(Source0);
    gcmASSERT(Source1);

    status = _EmitOpcodeConditionAndTarget(Compiler,
                           LineNo,
                           StringNo,
                           gcSL_CMP,
                           _ConvCondition(Condition),
                           Target);
    if (gcmIS_ERROR(status)) return status;

    /* CMP does not support int8 nor int16 */
    if (Source0)
    {
        sourceType = Source0->dataType.elementType;

        if (sourceType == clvTYPE_CHAR || sourceType == clvTYPE_SHORT)
        {
            Source0->dataType.elementType = clvTYPE_INT;
        }
        else if (sourceType == clvTYPE_UCHAR || sourceType == clvTYPE_USHORT)
        {
            Source0->dataType.elementType = clvTYPE_UINT;
        }
    }
    if (Source1)
    {
        sourceType = Source1->dataType.elementType;

        if (sourceType == clvTYPE_CHAR || sourceType == clvTYPE_SHORT)
        {
            Source1->dataType.elementType = clvTYPE_INT;
        }
        else if (sourceType == clvTYPE_UCHAR || sourceType == clvTYPE_USHORT)
        {
            Source1->dataType.elementType = clvTYPE_UINT;
        }
    }

    if(Source0) {
       status = _EmitSource(Compiler,
                    LineNo,
                    StringNo,
                    Source0);
       if (gcmIS_ERROR(status)) return status;
    }

    if(Source1) {
       status = _EmitSource(Compiler,
                    LineNo,
                    StringNo,
                    Source1);
       if (gcmIS_ERROR(status)) return status;
    }

    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                      clvDUMP_CODE_EMITTER,
                      "</INSTRUCTION>"));

    return gcvSTATUS_OK;
}

static gceSTATUS
_EmitCompareCode(
    IN cloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN cleCONDITION Condition,
    IN gcsTARGET * Target,
    IN gcsSOURCE * Source0,
    IN gcsSOURCE * Source1
    )
{
    gceSTATUS    status;

    gcmASSERT(Target);
    gcmASSERT(Source0);
    gcmASSERT(Source1);

        if(gcIsScalarDataType(Target->dataType)) {
        status = _EmitScalarCompareCode(Compiler,
                        LineNo,
                        StringNo,
                        Condition,
                        Target,
                        Source0,
                        Source1);
        if (gcmIS_ERROR(status)) return status;
    }
    else {
        status = _EmitVectorCompareCode(Compiler,
                        LineNo,
                        StringNo,
                        Condition,
                        Target,
                        Source0,
                        Source1);
        if (gcmIS_ERROR(status)) return status;
    }

    return gcvSTATUS_OK;
}

static gceSTATUS
_EmitLessThanCode(
    IN cloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gcsTARGET * Target,
    IN gcsSOURCE * Source0,
    IN gcsSOURCE * Source1
    )
{
    gcmASSERT(Target);
    gcmASSERT(Source0);
    gcmASSERT(Source1);

    return _EmitCompareCode(Compiler,
                LineNo,
                StringNo,
                clvCONDITION_LESS_THAN,
                Target,
                Source0,
                Source1);
}

static gceSTATUS
_EmitLessThanEqualCode(
    IN cloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gcsTARGET * Target,
    IN gcsSOURCE * Source0,
    IN gcsSOURCE * Source1
    )
{
    gcmASSERT(Target);
    gcmASSERT(Source0);
    gcmASSERT(Source1);

    return _EmitCompareCode(Compiler,
                LineNo,
                StringNo,
                clvCONDITION_LESS_THAN_EQUAL,
                Target,
                Source0,
                Source1);
}

static gceSTATUS
_EmitGreaterThanCode(
    IN cloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gcsTARGET * Target,
    IN gcsSOURCE * Source0,
    IN gcsSOURCE * Source1
    )
{
    gcmASSERT(Target);
    gcmASSERT(Source0);
    gcmASSERT(Source1);

    return _EmitCompareCode(Compiler,
                LineNo,
                StringNo,
                clvCONDITION_GREATER_THAN,
                Target,
                Source0,
                Source1);
}

static gceSTATUS
_EmitGreaterThanEqualCode(
    IN cloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gcsTARGET * Target,
    IN gcsSOURCE * Source0,
    IN gcsSOURCE * Source1
    )
{
    gcmASSERT(Target);
    gcmASSERT(Source0);
    gcmASSERT(Source1);

    return _EmitCompareCode(Compiler,
                LineNo,
                StringNo,
                clvCONDITION_GREATER_THAN_EQUAL,
                Target,
                Source0,
                Source1);
}

static gceSTATUS
_EmitEqualCode(
    IN cloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gcsTARGET * Target,
    IN gcsSOURCE * Source0,
    IN gcsSOURCE * Source1
    )
{
    gcmASSERT(Target);
    gcmASSERT(Source0);
    gcmASSERT(Source1);

    return _EmitCompareCode(Compiler,
                LineNo,
                StringNo,
                clvCONDITION_EQUAL,
                Target,
                Source0,
                Source1);
}

static gceSTATUS
_EmitNotEqualCode(
    IN cloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gcsTARGET * Target,
    IN gcsSOURCE * Source0,
    IN gcsSOURCE * Source1
    )
{
    gcmASSERT(Target);
    gcmASSERT(Source0);
    gcmASSERT(Source1);

    return _EmitCompareCode(Compiler,
                LineNo,
                StringNo,
                clvCONDITION_NOT_EQUAL,
                Target,
                Source0,
                Source1);
}

static gceSTATUS
_EmitDotCode(
    IN cloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gcsTARGET * Target,
    IN gcsSOURCE * Source0,
    IN gcsSOURCE * Source1
    )
{
    gctUINT8 vectorSize;

    gcmASSERT(Target);
    gcmASSERT(Source0);
    gcmASSERT(Source1);

    gcmASSERT(!gcIsMatrixDataType(Source0->dataType));
        gcmASSERT(clmIsElementTypeFloating(clmGEN_CODE_elementType_GET(Source0->dataType)));

        vectorSize = clmGEN_CODE_vectorSize_NOCHECK_GET(Source0->dataType);
    switch (vectorSize) {
    case 0:
        return _EmitCode(Compiler,
                 LineNo,
                 StringNo,
                 gcSL_MUL,
                 Target,
                 Source0,
                 Source1);

    case 2:
        return _EmitDP2Code(Compiler,
                    LineNo,
                    StringNo,
                    Target,
                    Source0,
                    Source1);

    case 3:
        return _EmitCode(Compiler,
                 LineNo,
                 StringNo,
                 gcSL_DP3,
                 Target,
                 Source0,
                 Source1);

    case 4:
        return _EmitCode(Compiler,
                 LineNo,
                 StringNo,
                 gcSL_DP4,
                 Target,
                 Source0,
                 Source1);
    }

    gcmASSERT(0);
    return gcvSTATUS_INVALID_ARGUMENT;
}

static gceSTATUS
_EmitAddRTZCode(
    IN cloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gcsTARGET * Target,
    IN gcsSOURCE * Source0,
    IN gcsSOURCE * Source1
    )
{
    cltELEMENT_TYPE elementType;

    gcmASSERT(Target);
    gcmASSERT(Source0);
    gcmASSERT(Source1);

    gcmASSERT(!gcIsMatrixDataType(Source0->dataType));

    elementType = clmGEN_CODE_elementType_GET(Target->dataType);
    if (! clmIsElementTypeFloating(elementType) ||
        ! cloCOMPILER_GetCodeGenerator(Compiler)->supportRTNE)
    {
        return _EmitCode(Compiler,
                    LineNo,
                    StringNo,
                    gcSL_ADD,
                    Target,
                    Source0,
                    Source1);
    }
    else
    {
        return _EmitCodeWRound(Compiler,
                    LineNo,
                    StringNo,
                    gcSL_ADD,
                    Target,
                    Source0,
                    Source1,
                    gcSL_ROUND_RTZ);
    }
}

static gceSTATUS
_EmitAddRTNECode(
    IN cloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gcsTARGET * Target,
    IN gcsSOURCE * Source0,
    IN gcsSOURCE * Source1
    )
{
    cltELEMENT_TYPE elementType;

    gcmASSERT(Target);
    gcmASSERT(Source0);
    gcmASSERT(Source1);

    gcmASSERT(!gcIsMatrixDataType(Source0->dataType));

    elementType = clmGEN_CODE_elementType_GET(Target->dataType);
    if (! clmIsElementTypeFloating(elementType) ||
        ! cloCOMPILER_GetCodeGenerator(Compiler)->supportRTNE)
    {
        return _EmitCode(Compiler,
                    LineNo,
                    StringNo,
                    gcSL_ADD,
                    Target,
                    Source0,
                    Source1);
    }
    else
    {
        return _EmitCodeWRound(Compiler,
                    LineNo,
                    StringNo,
                    gcSL_ADD,
                    Target,
                    Source0,
                    Source1,
                    gcSL_ROUND_RTNE);
    }
}

static gceSTATUS
_EmitAddLoRTZCode(
    IN cloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gcsTARGET * Target,
    IN gcsSOURCE * Source0,
    IN gcsSOURCE * Source1
    )
{
    cltELEMENT_TYPE elementType;

    gcmASSERT(Target);
    gcmASSERT(Source0);
    gcmASSERT(Source1);

    gcmASSERT(!gcIsMatrixDataType(Source0->dataType));

    elementType = clmGEN_CODE_elementType_GET(Target->dataType);
    if (! clmIsElementTypeFloating(elementType) ||
        ! cloCOMPILER_GetCodeGenerator(Compiler)->supportRTNE)
    {
        return _EmitCode(Compiler,
                    LineNo,
                    StringNo,
                    gcSL_ADDLO,
                    Target,
                    Source0,
                    Source1);
    }
    else
    {
        return _EmitCodeWRound(Compiler,
                    LineNo,
                    StringNo,
                    gcSL_ADDLO,
                    Target,
                    Source0,
                    Source1,
                    gcSL_ROUND_RTZ);
    }
}

static gceSTATUS
_EmitAddLoRTNECode(
    IN cloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gcsTARGET * Target,
    IN gcsSOURCE * Source0,
    IN gcsSOURCE * Source1
    )
{
    cltELEMENT_TYPE elementType;

    gcmASSERT(Target);
    gcmASSERT(Source0);
    gcmASSERT(Source1);

    gcmASSERT(!gcIsMatrixDataType(Source0->dataType));

    elementType = clmGEN_CODE_elementType_GET(Target->dataType);
    if (! clmIsElementTypeFloating(elementType) ||
        ! cloCOMPILER_GetCodeGenerator(Compiler)->supportRTNE)
    {
        return _EmitCode(Compiler,
                    LineNo,
                    StringNo,
                    gcSL_ADDLO,
                    Target,
                    Source0,
                    Source1);
    }
    else
    {
        return _EmitCodeWRound(Compiler,
                    LineNo,
                    StringNo,
                    gcSL_ADDLO,
                    Target,
                    Source0,
                    Source1,
                    gcSL_ROUND_RTNE);
    }
}

static gceSTATUS
_EmitSubRTZCode(
    IN cloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gcsTARGET * Target,
    IN gcsSOURCE * Source0,
    IN gcsSOURCE * Source1
    )
{
    cltELEMENT_TYPE elementType;

    gcmASSERT(Target);
    gcmASSERT(Source0);
    gcmASSERT(Source1);

    gcmASSERT(!gcIsMatrixDataType(Source0->dataType));

    elementType = clmGEN_CODE_elementType_GET(Target->dataType);
    if (! clmIsElementTypeFloating(elementType) ||
        ! cloCOMPILER_GetCodeGenerator(Compiler)->supportRTNE)
    {
        return _EmitCode(Compiler,
                    LineNo,
                    StringNo,
                    gcSL_SUB,
                    Target,
                    Source0,
                    Source1);
    }
    else
    {
        return _EmitCodeWRound(Compiler,
                    LineNo,
                    StringNo,
                    gcSL_SUB,
                    Target,
                    Source0,
                    Source1,
                    gcSL_ROUND_RTZ);
    }
}

static gceSTATUS
_EmitSubRTNECode(
    IN cloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gcsTARGET * Target,
    IN gcsSOURCE * Source0,
    IN gcsSOURCE * Source1
    )
{
    cltELEMENT_TYPE elementType;

    gcmASSERT(Target);
    gcmASSERT(Source0);
    gcmASSERT(Source1);

    gcmASSERT(!gcIsMatrixDataType(Source0->dataType));

    elementType = clmGEN_CODE_elementType_GET(Target->dataType);
    if (! clmIsElementTypeFloating(elementType) ||
        ! cloCOMPILER_GetCodeGenerator(Compiler)->supportRTNE)
    {
        return _EmitCode(Compiler,
                    LineNo,
                    StringNo,
                    gcSL_SUB,
                    Target,
                    Source0,
                    Source1);
    }
    else
    {
        return _EmitCodeWRound(Compiler,
                    LineNo,
                    StringNo,
                    gcSL_SUB,
                    Target,
                    Source0,
                    Source1,
                    gcSL_ROUND_RTNE);
    }
}

static gceSTATUS
_EmitMulRTZCode(
    IN cloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gcsTARGET * Target,
    IN gcsSOURCE * Source0,
    IN gcsSOURCE * Source1
    )
{
    cltELEMENT_TYPE elementType;

    gcmASSERT(Target);
    gcmASSERT(Source0);
    gcmASSERT(Source1);

    gcmASSERT(!gcIsMatrixDataType(Source0->dataType));

    elementType = clmGEN_CODE_elementType_GET(Target->dataType);
    if (! clmIsElementTypeFloating(elementType) ||
        ! cloCOMPILER_GetCodeGenerator(Compiler)->supportRTNE)
    {
        return _EmitCode(Compiler,
                    LineNo,
                    StringNo,
                    gcSL_MUL,
                    Target,
                    Source0,
                    Source1);
    }
    else
    {
        return _EmitCodeWRound(Compiler,
                    LineNo,
                    StringNo,
                    gcSL_MUL,
                    Target,
                    Source0,
                    Source1,
                    gcSL_ROUND_RTZ);
    }
}

static gceSTATUS
_EmitMulRTNECode(
    IN cloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gcsTARGET * Target,
    IN gcsSOURCE * Source0,
    IN gcsSOURCE * Source1
    )
{
    cltELEMENT_TYPE elementType;

    gcmASSERT(Target);
    gcmASSERT(Source0);
    gcmASSERT(Source1);

    gcmASSERT(!gcIsMatrixDataType(Source0->dataType));

    elementType = clmGEN_CODE_elementType_GET(Target->dataType);
    if (! clmIsElementTypeFloating(elementType) ||
        ! cloCOMPILER_GetCodeGenerator(Compiler)->supportRTNE)
    {
        return _EmitCode(Compiler,
                    LineNo,
                    StringNo,
                    gcSL_MUL,
                    Target,
                    Source0,
                    Source1);
    }
    else
    {
        return _EmitCodeWRound(Compiler,
                    LineNo,
                    StringNo,
                    gcSL_MUL,
                    Target,
                    Source0,
                    Source1,
                    gcSL_ROUND_RTNE);
    }
}

static gceSTATUS
_EmitMulLoRTZCode(
    IN cloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gcsTARGET * Target,
    IN gcsSOURCE * Source0,
    IN gcsSOURCE * Source1
    )
{
    cltELEMENT_TYPE elementType;

    gcmASSERT(Target);
    gcmASSERT(Source0);
    gcmASSERT(Source1);

    gcmASSERT(!gcIsMatrixDataType(Source0->dataType));

    elementType = clmGEN_CODE_elementType_GET(Target->dataType);
    if (! clmIsElementTypeFloating(elementType) ||
        ! cloCOMPILER_GetCodeGenerator(Compiler)->supportRTNE)
    {
        return _EmitCode(Compiler,
                    LineNo,
                    StringNo,
                    gcSL_MULLO,
                    Target,
                    Source0,
                    Source1);
    }
    else
    {
        return _EmitCodeWRound(Compiler,
                    LineNo,
                    StringNo,
                    gcSL_MULLO,
                    Target,
                    Source0,
                    Source1,
                    gcSL_ROUND_RTZ);
    }
}

static gceSTATUS
_EmitMulLoRTNECode(
    IN cloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gcsTARGET * Target,
    IN gcsSOURCE * Source0,
    IN gcsSOURCE * Source1
    )
{
    cltELEMENT_TYPE elementType;

    gcmASSERT(Target);
    gcmASSERT(Source0);
    gcmASSERT(Source1);

    gcmASSERT(!gcIsMatrixDataType(Source0->dataType));

    elementType = clmGEN_CODE_elementType_GET(Target->dataType);
    if (! clmIsElementTypeFloating(elementType) ||
        ! cloCOMPILER_GetCodeGenerator(Compiler)->supportRTNE)
    {
        return _EmitCode(Compiler,
                    LineNo,
                    StringNo,
                    gcSL_MULLO,
                    Target,
                    Source0,
                    Source1);
    }
    else
    {
        return _EmitCodeWRound(Compiler,
                    LineNo,
                    StringNo,
                    gcSL_MULLO,
                    Target,
                    Source0,
                    Source1,
                    gcSL_ROUND_RTNE);
    }
}

typedef gceSTATUS
(* cltEMIT_SPECIAL_CODE_FUNC_PTR2)(
    IN cloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gcsTARGET * Target,
    IN gcsSOURCE * Source0,
    IN gcsSOURCE * Source1
    );

typedef struct _clsSPECIAL_CODE_EMITTER2
{
    cleOPCODE opcode;

    cltEMIT_SPECIAL_CODE_FUNC_PTR2    codeEmitter;
}
clsSPECIAL_CODE_EMITTER2;

static clsSPECIAL_CODE_EMITTER2 SpecialCodeEmitterTable2[] =
{
#if _GEN_DIV_IN_BACKEND
    {clvOPCODE_DIV,         _EmitDivCode},
#endif

    {clvOPCODE_ATAN2,        _EmitAtan2Code},

    {clvOPCODE_LESS_THAN,    _EmitLessThanCode},
    {clvOPCODE_LESS_THAN_EQUAL,    _EmitLessThanEqualCode},
    {clvOPCODE_GREATER_THAN,    _EmitGreaterThanCode},
    {clvOPCODE_GREATER_THAN_EQUAL,    _EmitGreaterThanEqualCode},
    {clvOPCODE_EQUAL,        _EmitEqualCode},
    {clvOPCODE_NOT_EQUAL,    _EmitNotEqualCode},

    {clvOPCODE_DOT,        _EmitDotCode},

    {clvOPCODE_ADD_RTZ,        _EmitAddRTZCode},
    {clvOPCODE_ADD_RTNE,    _EmitAddRTNECode},
    {clvOPCODE_ADDLO_RTZ,    _EmitAddLoRTZCode},
    {clvOPCODE_ADDLO_RTNE,    _EmitAddLoRTNECode},
    {clvOPCODE_SUB_RTZ,        _EmitSubRTZCode},
    {clvOPCODE_SUB_RTNE,    _EmitSubRTNECode},
    {clvOPCODE_MUL_RTZ,        _EmitMulRTZCode},
    {clvOPCODE_MUL_RTNE,    _EmitMulRTNECode},
    {clvOPCODE_MULLO_RTZ,    _EmitMulLoRTZCode},
    {clvOPCODE_MULLO_RTNE,    _EmitMulLoRTNECode},
};

const gctUINT SpecialCodeEmitterCount2 =
                    sizeof(SpecialCodeEmitterTable2) / sizeof(clsSPECIAL_CODE_EMITTER2);

static gceSTATUS
_EmitCodeImpl2(
    IN cloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN cleOPCODE Opcode,
    IN gcsTARGET * Target,
    IN gcsSOURCE * Source0,
    IN gcsSOURCE * Source1
    )
{
    gceSTATUS    status;
    gcsSOURCE    newSource0, newSource1;
    gctUINT        i;
    cltEMIT_SPECIAL_CODE_FUNC_PTR2    specialCodeEmitter = gcvNULL;

    status = _PrepareSource(
                            Compiler,
                            LineNo,
                            StringNo,
                            Target,
                            Source0,
                            &newSource0);

    if (gcmIS_ERROR(status)) return status;

    status = _PrepareAnotherSource(
                                Compiler,
                                LineNo,
                                StringNo,
                                Target,
                                &newSource0,
                                Source1,
                                &newSource1);

    if (gcmIS_ERROR(status)) return status;

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

        if (gcmIS_ERROR(status)) return status;
    }
    else
    {
        gcSHADER binary;

        gcmVERIFY_OK(cloCOMPILER_GetBinary(Compiler, &binary));
        /* need to set the instruction modifiers first before emitting the instruction */
        switch(Opcode) {
        case clvOPCODE_STORE1_RTE:
            gcSHADER_AddRoundingMode(binary, gcSL_ROUND_RTNE);
            break;

        case clvOPCODE_STORE1_RTZ:
            gcSHADER_AddRoundingMode(binary, gcSL_ROUND_RTZ);
            break;

        case clvOPCODE_STORE1_RTP:
            gcSHADER_AddRoundingMode(binary, gcSL_ROUND_RTP);
            break;

        case clvOPCODE_STORE1_RTN:
            gcSHADER_AddRoundingMode(binary, gcSL_ROUND_RTN);
            break;

        case clvOPCODE_CONV_SAT_RTE:
            gcSHADER_AddSaturation(binary, gcSL_SATURATE);
            /* fall through */
        case clvOPCODE_CONV_RTE:
            gcSHADER_AddRoundingMode(binary, gcSL_ROUND_RTNE);
            break;

        case clvOPCODE_CONV_SAT_RTZ:
            gcSHADER_AddSaturation(binary, gcSL_SATURATE);
            /* fall through */
        case clvOPCODE_CONV_RTZ:
            gcSHADER_AddRoundingMode(binary, gcSL_ROUND_RTZ);
            break;

        case clvOPCODE_CONV_SAT_RTN:
            gcSHADER_AddSaturation(binary, gcSL_SATURATE);
            /* fall through */
        case clvOPCODE_CONV_RTN:
            gcSHADER_AddRoundingMode(binary, gcSL_ROUND_RTN);
            break;

        case clvOPCODE_CONV_SAT_RTP:
            gcSHADER_AddSaturation(binary, gcSL_SATURATE);
            /* fall through */
        case clvOPCODE_CONV_RTP:
            gcSHADER_AddRoundingMode(binary, gcSL_ROUND_RTP);
            break;

        case clvOPCODE_CONV_SAT:
            gcSHADER_AddSaturation(binary, gcSL_SATURATE);
            break;

        case clvOPCODE_IMAGE_READ_3D:
        case clvOPCODE_IMAGE_READ:
        case clvOPCODE_IMAGE_WRITE_3D:
        case clvOPCODE_IMAGE_WRITE:
        case clvOPCODE_IMAGE_SAMPLER:
            if(clmIsElementTypeFloating(Target->dataType.elementType))
            {
                /* according to OCL spec: set default rounding mode for floating point tpye to RTE */
                gcSHADER_AddRoundingMode(binary, gcSL_ROUND_RTNE);
            }
            else
            {
                gcSHADER_AddRoundingMode(binary, gcSL_ROUND_RTZ);
            }
            break;

        default:
            break;
        }

        status = _EmitCode(Compiler,
                           LineNo,
                           StringNo,
                           _ConvOpcode(Opcode),
                           Target,
                           &newSource0,
                           &newSource1);

        if (gcmIS_ERROR(status)) return status;
    }

    return gcvSTATUS_OK;
}

gceSTATUS
clEmitCode2(
    IN cloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN cleOPCODE Opcode,
    IN gcsTARGET * Target,
    IN gcsSOURCE * Source0,
    IN gcsSOURCE * Source1
    )
{
    cloCODE_EMITTER    codeEmitter;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);

    codeEmitter = cloCOMPILER_GetCodeEmitter(Compiler);
    gcmASSERT(codeEmitter);

    return cloCODE_EMITTER_EmitCode2(Compiler,
                                     codeEmitter,
                                     LineNo,
                                     StringNo,
                                     Opcode,
                                     Target,
                                     Source0,
                                     Source1);
}

gceSTATUS
clEmitAlwaysBranchCode(
    IN cloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN cleOPCODE Opcode,
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
clEmitTestBranchCode(
    IN cloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN cleOPCODE Opcode,
    IN gctLABEL Label,
    IN gctBOOL TrueBranch,
    IN gcsSOURCE * Source
    )
{
    gceSTATUS    status;
    gcsSOURCE    newSource[1];
    gcsSOURCE    falseSource[1];
    cltELEMENT_TYPE    elementType;

    gcmASSERT(Source);

    status = _PrepareSource(Compiler,
                LineNo,
                StringNo,
                gcvNULL,
                Source,
                newSource);
    if (gcmIS_ERROR(status)) return status;

    elementType = clmGEN_CODE_elementType_GET(newSource->dataType);
    if(clmIsElementTypeFloating(elementType)) {
       gcsSOURCE_InitializeFloatConstant(falseSource, (gctFLOAT) 0.0);
    }
    else if(clmIsElementTypeBoolean(elementType)) {
       gcsSOURCE_InitializeBoolConstant(falseSource, gcvFALSE);
    }
    else {
       gcsSOURCE_InitializeIntConstant(falseSource, (gctINT32) 0);
    }

    return _EmitBranchCode(Compiler,
                LineNo,
                StringNo,
                _ConvOpcode(Opcode),
                TrueBranch ? gcSL_NOT_EQUAL : gcSL_EQUAL,
                Label,
                newSource,
                falseSource);
}

gceSTATUS
clEmitCompareBranchCode(
    IN cloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN cleOPCODE Opcode,
    IN cleCONDITION Condition,
    IN gctLABEL Label,
    IN gcsSOURCE * Source0,
    IN gcsSOURCE * Source1
    )
{
    gceSTATUS        status;
    gcsSOURCE        newSource1;

    gcmASSERT(Source0);
    gcmASSERT(Source1);

    status = _PrepareAnotherSource(
                                Compiler,
                                LineNo,
                                StringNo,
                                gcvNULL,
                                Source0,
                                Source1,
                                &newSource1);

    if (gcmIS_ERROR(status)) return status;

    return _EmitBranchCode(
                        Compiler,
                        LineNo,
                        StringNo,
                        _ConvOpcode(Opcode),
                        _ConvCondition(Condition),
                        Label,
                        Source0,
                        &newSource1);
}

gceSTATUS
clEmitCompareSetCode(
    IN cloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN cleOPCODE Opcode,
    IN gcsTARGET *Target,
    IN gcsSOURCE * Cond,
    IN gcsSOURCE * Source0,
    IN gcsSOURCE * Source1
    )
{
   gceSTATUS status;
   gcsSOURCE newCond, newSource0, newSource1;
   gcSL_OPCODE opcode;
   cloCODE_EMITTER codeEmitter;

   gcmASSERT(Target);
   gcmASSERT(Cond);
   gcmASSERT(Source0);
   gcmASSERT(Source1);


   codeEmitter = cloCOMPILER_GetCodeEmitter(Compiler);
   gcmASSERT(codeEmitter);

   /* flush out previous instructions */
   status = cloCODE_EMITTER_EndBasicBlock(Compiler, codeEmitter);
   if (gcmIS_ERROR(status)) return status;

   opcode = _ConvOpcode(Opcode);
   status = _PrepareSource(Compiler,
                           LineNo,
                           StringNo,
                           Target,
                           Cond,
                           &newCond);
   if (gcmIS_ERROR(status)) return status;

   status = _PrepareAnotherSource(Compiler,
                                  LineNo,
                                  StringNo,
                                  Target,
                                  &newCond,
                                  Source0,
                                  &newSource0);
   if (gcmIS_ERROR(status)) return status;

   status = _PrepareAnotherSource(Compiler,
                                  LineNo,
                                  StringNo,
                                  Target,
                                  &newCond,
                                  Source1,
                                  &newSource1);
   if (gcmIS_ERROR(status)) return status;

   status = _EmitOpcodeConditionAndTarget(Compiler,
                                          LineNo,
                                          StringNo,
                                          opcode,
                                          gcSL_ZERO,
                                          Target);
   if (gcmIS_ERROR(status)) return status;

   status = _EmitSource(Compiler,
                        LineNo,
                        StringNo,
                        &newCond);
   if (gcmIS_ERROR(status)) return status;

   status = _EmitSource(Compiler,
                        LineNo,
                        StringNo,
                        &newSource0);
   if (gcmIS_ERROR(status)) return status;

   status = _EmitOpcodeConditionAndTarget(Compiler,
                                          LineNo,
                                          StringNo,
                                          opcode,
                                          gcSL_NOT_ZERO,
                                          Target);
   if (gcmIS_ERROR(status)) return status;

   status = _EmitSource(Compiler,
                        LineNo,
                        StringNo,
                        &newCond);
   if (gcmIS_ERROR(status)) return status;

   return _EmitSource(Compiler,
                      LineNo,
                      StringNo,
                      &newSource1);
}

gceSTATUS
clBeginMainFunction(
IN cloCOMPILER Compiler,
IN gctUINT LineNo,
IN gctUINT StringNo
)
{
    gceSTATUS    status;
    cloCODE_EMITTER    codeEmitter;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);

    codeEmitter = cloCOMPILER_GetCodeEmitter(Compiler);
    gcmASSERT(codeEmitter);

    status = cloCODE_EMITTER_NewBasicBlock(Compiler, codeEmitter);
    if (gcmIS_ERROR(status)) return status;
    return gcvSTATUS_OK;
}

gceSTATUS
clEndMainFunction(
IN cloCOMPILER Compiler
)
{
    gceSTATUS    status;
    cloCODE_EMITTER    codeEmitter;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);

    codeEmitter = cloCOMPILER_GetCodeEmitter(Compiler);
    gcmASSERT(codeEmitter);

    status = cloCODE_EMITTER_EndBasicBlock(Compiler, codeEmitter);
    if (gcmIS_ERROR(status)) return status;

    return gcvSTATUS_OK;
}

gceSTATUS
clNewKernelFunction(
IN cloCOMPILER Compiler,
IN gctUINT LineNo,
IN gctUINT StringNo,
IN gctCONST_STRING Name,
OUT gcKERNEL_FUNCTION * Function
)
{
    gceSTATUS  status;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    gcmASSERT(Name);
    gcmASSERT(Function);

    status = _AddKernelFunction(Compiler,
                        Name,
                        Function);

    if (gcmIS_ERROR(status)) {
        gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                        LineNo,
                        StringNo,
                        clvREPORT_INTERNAL_ERROR,
                        "failed to add the kernel function: '%s'",
                        Name));
        return status;
    }
    return gcvSTATUS_OK;
}

gceSTATUS
clNewFunction(
IN cloCOMPILER Compiler,
IN gctUINT LineNo,
IN gctUINT StringNo,
IN gctCONST_STRING Name,
OUT gcFUNCTION * Function
)
{
    gceSTATUS  status;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    gcmASSERT(Name);
    gcmASSERT(Function);

    status = _AddFunction(Compiler,
                  Name,
                  Function);

    if (gcmIS_ERROR(status)) {
        gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                        LineNo,
                        StringNo,
                        clvREPORT_INTERNAL_ERROR,
                        "failed to add the function: '%s'",
                        Name));
        return status;
    }
    return gcvSTATUS_OK;
}

gceSTATUS
clNewFunctionArgument(
    IN cloCOMPILER Compiler,
    IN gcFUNCTION Function,
    IN gcVARIABLE Variable,
    IN clsGEN_CODE_DATA_TYPE DataType,
    IN gctSIZE_T Length,
    IN gctREG_INDEX TempRegIndex,
    IN gctUINT8 Qualifier
    )
{
    gceSTATUS    status;
    gctSIZE_T    i, j, binaryDataTypeRegSize, regCount, regOffset;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    gcmASSERT(Function);

    binaryDataTypeRegSize = gcGetDataTypeRegSize(DataType);
    regCount = binaryDataTypeRegSize *
               _clmGetTempRegIndexOffset(binaryDataTypeRegSize,
                                         DataType.elementType);
    regOffset = _clmGetTempRegIndexOffset(1,
                                          DataType.elementType);

    for (i = 0; i < Length; i++)
    {
        gctSIZE_T index = i * regCount;

        for (j = 0; j < binaryDataTypeRegSize; j++)
        {
            status = _AddFunctionArgument(Compiler,
                                          Function,
                                          Variable ? GetVariableIndex(Variable) : 0xffff,
                                          TempRegIndex + (gctREG_INDEX)(index + j * regOffset),
                                          gcGetDefaultEnable(Compiler, DataType),
                                          Qualifier);

            if (gcmIS_ERROR(status))
            {
                gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                0,
                                0,
                                clvREPORT_INTERNAL_ERROR,
                                "failed to add the function argument"));
                return status;
            }
            /* only create one argument for packed type even if it occupies more than one vec4 register */
            if (clmIsElementTypePacked(DataType.elementType) ||
                (clmGEN_CODE_IsExtendedVectorType(DataType) && cloCOMPILER_ExtensionEnabled(Compiler, clvEXTENSION_VIV_VX)))
            {
                break;
            }
        }

    }

    return gcvSTATUS_OK;
}

gceSTATUS
clNewKernelFunctionArgument(
    IN cloCOMPILER Compiler,
    IN gcKERNEL_FUNCTION Function,
    IN gcVARIABLE Variable,
    IN clsGEN_CODE_DATA_TYPE DataType,
    IN gctSIZE_T Length,
    IN gctREG_INDEX TempRegIndex,
    IN gctUINT8 Qualifier
    )
{
    gceSTATUS    status;
    gctSIZE_T    i, j, binaryDataTypeRegSize, regCount, regOffset;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    gcmASSERT(Function);

    binaryDataTypeRegSize = gcGetDataTypeRegSize(DataType);
    regCount = binaryDataTypeRegSize *
               _clmGetTempRegIndexOffset(binaryDataTypeRegSize,
                                         DataType.elementType);
    regOffset = _clmGetTempRegIndexOffset(1,
                                          DataType.elementType);

    for (i = 0; i < Length; i++)
    {
        gctSIZE_T index = i * regCount;

        for (j = 0; j < binaryDataTypeRegSize; j++)
        {
            status = _AddKernelFunctionArgument(Compiler,
                                                Function,
                                                Variable ? GetVariableIndex(Variable) : 0xffff,
                                                TempRegIndex + (gctREG_INDEX)(index + j * regOffset),
                                                gcGetDefaultEnable(Compiler, DataType),
                                                Qualifier);

            if (gcmIS_ERROR(status))
            {
                gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                                0,
                                                0,
                                                clvREPORT_INTERNAL_ERROR,
                                                "failed to add the kernel function argument"));
                return status;
            }
            /* only create one argument for packed type even if it occupies more than one vec4 register */
            if (clmIsElementTypePacked(DataType.elementType) ||
                (clmGEN_CODE_IsExtendedVectorType(DataType) && cloCOMPILER_ExtensionEnabled(Compiler, clvEXTENSION_VIV_VX)))
            {
                break;
            }
        }
    }

    return gcvSTATUS_OK;
}

gceSTATUS
clBeginFunction(
IN cloCOMPILER Compiler,
IN gctUINT LineNo,
IN gctUINT StringNo,
IN gcFUNCTION Function
)
{
    gceSTATUS    status;
    cloCODE_EMITTER    codeEmitter;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    gcmASSERT(Function);

    codeEmitter = cloCOMPILER_GetCodeEmitter(Compiler);
    gcmASSERT(codeEmitter);

    status = cloCODE_EMITTER_NewBasicBlock(Compiler, codeEmitter);
    if (gcmIS_ERROR(status)) return status;

    status = _BeginFunction(Compiler, Function);

    if (gcmIS_ERROR(status)) {
        gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                        LineNo,
                        StringNo,
                        clvREPORT_INTERNAL_ERROR,
                        "failed to begin function"));
        return status;
    }
    return gcvSTATUS_OK;
}

gceSTATUS
clEndFunction(
    IN cloCOMPILER Compiler,
    IN gcFUNCTION Function
    )
{
    gceSTATUS            status;
    cloCODE_EMITTER        codeEmitter;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    gcmASSERT(Function);

    codeEmitter = cloCOMPILER_GetCodeEmitter(Compiler);
    gcmASSERT(codeEmitter);

    status = cloCODE_EMITTER_EndBasicBlock(Compiler, codeEmitter);

    if (gcmIS_ERROR(status)) return status;

    status = _EndFunction(Compiler, Function);

    if (gcmIS_ERROR(status))
    {
        cloCOMPILER_Report(Compiler,
                   0,
                   0,
                   clvREPORT_INTERNAL_ERROR,
                   "failed to end function");
        return status;
    }

    return gcvSTATUS_OK;
}

gceSTATUS
clBeginKernelFunction(
IN cloCOMPILER Compiler,
IN gctUINT LineNo,
IN gctUINT StringNo,
IN gcKERNEL_FUNCTION Function
)
{
    gceSTATUS    status;
    cloCODE_EMITTER    codeEmitter;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    gcmASSERT(Function);

    codeEmitter = cloCOMPILER_GetCodeEmitter(Compiler);
    gcmASSERT(codeEmitter);

    status = cloCODE_EMITTER_NewBasicBlock(Compiler, codeEmitter);
    if (gcmIS_ERROR(status)) return status;

    status = _BeginKernelFunction(Compiler, Function);

    if (gcmIS_ERROR(status)) {
        cloCOMPILER_Report(Compiler,
                   LineNo,
                   StringNo,
                   clvREPORT_INTERNAL_ERROR,
                   "failed to begin kernel function");
        return status;
    }
    return gcvSTATUS_OK;
}

static gctUINT
_FindParamNum(
IN clsNAME *FuncName,
IN clsNAME *ParamName
)
{
   gctUINT paramCount;
   clsNAME *paramName;

   for (paramCount = 0, paramName = (clsNAME *)FuncName->u.funcInfo.localSpace->names.next;
    (slsDLINK_NODE *)paramName != &FuncName->u.funcInfo.localSpace->names;
    paramName = (clsNAME *)((slsDLINK_NODE *)paramName)->next, paramCount++) {
    if (paramName->type != clvPARAMETER_NAME) break;
        if(paramName == ParamName) {
           return paramCount;
        }
   }
   gcmASSERT(0);
   return 0;
}

static gceSTATUS
_BindImageSamplers(
IN clsNAME *FuncName
)
{
   gceSTATUS status = gcvSTATUS_OK;
   clsNAME *paramName;
   gctBOOL isConstantSamplerType;
   gctUINT32 samplerType;
   gctUINT8 paramCount;

   for (paramCount = 0, paramName = (clsNAME *)FuncName->u.funcInfo.localSpace->names.next;
    (slsDLINK_NODE *)paramName != &FuncName->u.funcInfo.localSpace->names;
    paramName = (clsNAME *)((slsDLINK_NODE *)paramName)->next, paramCount++) {
    if (paramName->type != clvPARAMETER_NAME) break;
        if (clmDECL_IsImage(&paramName->decl)) {
           clsNAME *sampler;

           if(paramName->u.variableInfo.samplers) {
               clsSAMPLER_TYPES *prev;
               clsSAMPLER_TYPES *next;
               gctUINT32 imageSamplerIndex;


               FOR_EACH_SLINK_NODE(paramName->u.variableInfo.samplers, clsSAMPLER_TYPES, prev, next) {
                 sampler = next->member;

                 if(sampler == gcvNULL)
                     continue;

                 if(sampler->type == clvPARAMETER_NAME) {
                    samplerType = _FindParamNum(FuncName,
                                                sampler);
                    isConstantSamplerType = gcvFALSE;
                 }
                 else {
                    cloIR_CONSTANT constant;

                    constant = sampler->u.variableInfo.u.constant;
                    gcmASSERT(constant);
                    samplerType = constant->values[0].uintValue;
                    isConstantSamplerType = gcvTRUE;
                 }

                 status = gcKERNEL_FUNCTION_AddImageSampler(FuncName->context.u.variable.u.kernelFunction,
                                                            paramCount,
                                                            isConstantSamplerType,
                                                            samplerType);
                 if(gcmIS_ERROR(status)) return status;

                 status = gcKERNEL_FUNCTION_GetImageSamplerCount(FuncName->context.u.variable.u.kernelFunction,
                                                                 &imageSamplerIndex);
                 if(gcmIS_ERROR(status)) return status;
                 if(next->imageSampler) {
                     next->imageSampler->imageSamplerIndex = (gctUINT16) (imageSamplerIndex - 1);
                 }
              }
           }
        }
   }
   return status;
}

static gceSTATUS
_AddKernelFunctionProperties(
IN clsNAME *FuncName
)
{
   gceSTATUS status;

   /* Add required work group size */
   status = gcKERNEL_FUNCTION_AddKernelFunctionProperties(FuncName->context.u.variable.u.kernelFunction,
                                                          gcvPROPERTY_REQD_WORK_GRP_SIZE,
                                                          3,
                                                          (gctINT *)FuncName->u.funcInfo.attrQualifier.reqdWorkGroupSize
                                                          );
   if(gcmIS_ERROR(status)) return status;

   /* Add work group size hint */
   status = gcKERNEL_FUNCTION_AddKernelFunctionProperties(FuncName->context.u.variable.u.kernelFunction,
                                                          gcvPROPERTY_WORK_GRP_SIZE_HINT,
                                                          3,
                                                          (gctINT *)FuncName->u.funcInfo.attrQualifier.workGroupSizeHint
                                                          );
   if(gcmIS_ERROR(status)) return status;

   /* Add kernel scale hint */
   status = gcKERNEL_FUNCTION_AddKernelFunctionProperties(FuncName->context.u.variable.u.kernelFunction,
                                                          gcvPROPERTY_KERNEL_SCALE_HINT,
                                                          3,
                                                          (gctINT *)FuncName->u.funcInfo.attrQualifier.kernelScaleHint
                                                          );
   if(gcmIS_ERROR(status)) return status;

   return status;
}

gceSTATUS
clEndKernelFunction(
    IN cloCOMPILER Compiler,
    IN clsNAME *FuncName
    )
{
    gceSTATUS status;
        gcKERNEL_FUNCTION kernelFunction;
    cloCODE_EMITTER    codeEmitter;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    gcmASSERT(FuncName);

    codeEmitter = cloCOMPILER_GetCodeEmitter(Compiler);
    gcmASSERT(codeEmitter);

    status = cloCODE_EMITTER_EndBasicBlock(Compiler, codeEmitter);
    if (gcmIS_ERROR(status)) return status;

    kernelFunction = FuncName->context.u.variable.u.kernelFunction;

    status = _BindImageSamplers(FuncName);
    if (gcmIS_ERROR(status))
    {
        cloCOMPILER_Report(Compiler,
                   0,
                   0,
                   clvREPORT_INTERNAL_ERROR,
                   "failed to bind image to a sampler type");
        return status;
    }

    status = _AddKernelFunctionProperties(FuncName);
    if (gcmIS_ERROR(status))
    {
        cloCOMPILER_Report(Compiler,
                   0,
                   0,
                   clvREPORT_INTERNAL_ERROR,
                   "failed to add kernel function properties");
        return status;
    }

    status = _EndKernelFunction(Compiler,
                                    kernelFunction,
                                    FuncName->u.funcInfo.localMemorySize);
    if (gcmIS_ERROR(status))
    {
        cloCOMPILER_Report(Compiler,
                   0,
                   0,
                   clvREPORT_INTERNAL_ERROR,
                   "failed to end kernel function");
        return status;
    }

    return gcvSTATUS_OK;
}

gceSTATUS
clGetKernelFunctionLabel(
    IN cloCOMPILER Compiler,
    IN gcKERNEL_FUNCTION Function,
    OUT gctLABEL * Label
    )
{
    gceSTATUS    status;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    gcmASSERT(Function);

    status = _GetKernelFunctionLabel(Compiler, Function, Label);

    if (gcmIS_ERROR(status))
    {
        cloCOMPILER_Report(Compiler,
                   0,
                   0,
                   clvREPORT_INTERNAL_ERROR,
                   "failed to get kernel function label");
        return status;
    }

    return gcvSTATUS_OK;
}

gceSTATUS
clGetFunctionLabel(
    IN cloCOMPILER Compiler,
    IN gcFUNCTION Function,
    OUT gctLABEL * Label
    )
{
    gceSTATUS    status;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    gcmASSERT(Function);

    status = _GetFunctionLabel(Compiler, Function, Label);

    if (gcmIS_ERROR(status))
    {
        cloCOMPILER_Report(Compiler,
                   0,
                   0,
                   clvREPORT_INTERNAL_ERROR,
                   "failed to get function label");
        return status;
    }

    return gcvSTATUS_OK;
}

/* cloCODE_EMITTER */
typedef enum _cleCODE_TYPE
{
    clvCODE_INVALID,
    clvCODE_ONE_OPERAND,
    clvCODE_TWO_OPERANDS,
    clvCODE_NO_OPERANDS
}
cleCODE_TYPE;

typedef struct _clsCODE_INFO
{
    cleCODE_TYPE    type;
    gctUINT        lineNo;
    gctUINT        stringNo;
    cleOPCODE    opcode;
    gcsTARGET    target;
    gcsSOURCE    source0;
    gcsSOURCE    source1;
}
clsCODE_INFO;

struct _cloCODE_EMITTER
{
    clsOBJECT        object;

    clsCODE_INFO    currentCodeInfo;
};

gceSTATUS
cloCODE_EMITTER_Construct(
    IN cloCOMPILER Compiler,
    OUT cloCODE_EMITTER * CodeEmitter
    )
{
    gceSTATUS            status;
    cloCODE_EMITTER        codeEmitter;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    gcmASSERT(CodeEmitter);

    do
    {
        status = cloCOMPILER_Allocate(
                                    Compiler,
                                    (gctSIZE_T)sizeof(struct _cloCODE_EMITTER),
                                    (gctPOINTER *) &codeEmitter);

        if (gcmIS_ERROR(status)) break;

        /* Initialize the members */
        codeEmitter->object.type            = clvOBJ_CODE_EMITTER;

        codeEmitter->currentCodeInfo.type    = clvCODE_INVALID;

        *CodeEmitter = codeEmitter;

        return gcvSTATUS_OK;
    }
    while (gcvFALSE);

    *CodeEmitter = gcvNULL;

    return status;
}

gceSTATUS
cloCODE_EMITTER_Destroy(
    IN cloCOMPILER Compiler,
    IN cloCODE_EMITTER CodeEmitter
    )
{
    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_OBJECT(CodeEmitter, clvOBJ_CODE_EMITTER);;

    gcmVERIFY_OK(cloCOMPILER_Free(Compiler, CodeEmitter));

    return gcvSTATUS_OK;
}

gceSTATUS
cloCODE_EMITTER_EmitCurrentCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_EMITTER CodeEmitter
    )
{
    gceSTATUS status;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_OBJECT(CodeEmitter, clvOBJ_CODE_EMITTER);

    switch (CodeEmitter->currentCodeInfo.type)
    {
    case clvCODE_INVALID:
        break;

    case clvCODE_ONE_OPERAND:
        CodeEmitter->currentCodeInfo.type = clvCODE_INVALID;

        status = _EmitCodeImpl1(Compiler,
                    CodeEmitter->currentCodeInfo.lineNo,
                    CodeEmitter->currentCodeInfo.stringNo,
                    CodeEmitter->currentCodeInfo.opcode,
                    &CodeEmitter->currentCodeInfo.target,
                    &CodeEmitter->currentCodeInfo.source0);

        if (gcmIS_ERROR(status)) return status;

        break;

    case clvCODE_TWO_OPERANDS:
        CodeEmitter->currentCodeInfo.type = clvCODE_INVALID;

        status = _EmitCodeImpl2(Compiler,
                    CodeEmitter->currentCodeInfo.lineNo,
                    CodeEmitter->currentCodeInfo.stringNo,
                    CodeEmitter->currentCodeInfo.opcode,
                    &CodeEmitter->currentCodeInfo.target,
                    &CodeEmitter->currentCodeInfo.source0,
                    &CodeEmitter->currentCodeInfo.source1);

        if (gcmIS_ERROR(status)) return status;

        break;

    case clvCODE_NO_OPERANDS:
        CodeEmitter->currentCodeInfo.type = clvCODE_INVALID;
        status = _EmitCode0(Compiler,
                    CodeEmitter->currentCodeInfo.lineNo,
                    CodeEmitter->currentCodeInfo.stringNo,
                    CodeEmitter->currentCodeInfo.opcode);
        if (gcmIS_ERROR(status)) return status;

        break;

    default:
        gcmASSERT(0);
    }

    return gcvSTATUS_OK;
}

gceSTATUS
cloCODE_EMITTER_NewBasicBlock(
    IN cloCOMPILER Compiler,
    IN cloCODE_EMITTER CodeEmitter
    )
{
    gceSTATUS status;

    /* End the previous basic block */
    status = cloCODE_EMITTER_EndBasicBlock(Compiler, CodeEmitter);

    if (gcmIS_ERROR(status)) return status;

    return gcvSTATUS_OK;
}

gceSTATUS
cloCODE_EMITTER_EndBasicBlock(
    IN cloCOMPILER Compiler,
    IN cloCODE_EMITTER CodeEmitter
    )
{
    gceSTATUS status;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_OBJECT(CodeEmitter, clvOBJ_CODE_EMITTER);

    status = cloCODE_EMITTER_EmitCurrentCode(Compiler, CodeEmitter);

    if (gcmIS_ERROR(status)) return status;

    return gcvSTATUS_OK;
}

#define clmMERGE_DATA_TYPE(dataType0, dataType1) \
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
        if(!gcIsElementTypeEqual(Target0->dataType, Target1->dataType)) break;

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

    do {

        if (Source0->type != Source1->type
            || !gcIsElementTypeEqual(Source0->dataType, Source1->dataType)) break;

        if (Source0->type == gcvSOURCE_CONSTANT)
        {
            if (Source0->u.sourceConstant.intValue
                != Source1->u.sourceConstant.intValue) break;
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
cloCODE_EMITTER_TryToMergeCode1(
    IN cloCOMPILER Compiler,
    IN cloCODE_EMITTER CodeEmitter,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN cleOPCODE Opcode,
    IN gcsTARGET * Target,
    IN gcsSOURCE * Source,
    OUT gctBOOL * Merged
    )
{
    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_OBJECT(CodeEmitter, clvOBJ_CODE_EMITTER);
    gcmASSERT(Merged);

    do
    {
        /* Check the type and opcode */
        if (CodeEmitter->currentCodeInfo.type != clvCODE_ONE_OPERAND
            || CodeEmitter->currentCodeInfo.opcode != Opcode) break;

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
                clmMERGE_DATA_TYPE(CodeEmitter->currentCodeInfo.target.dataType, Target->dataType);

        CodeEmitter->currentCodeInfo.source0.dataType =
                clmMERGE_DATA_TYPE(CodeEmitter->currentCodeInfo.source0.dataType, Source->dataType);

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

        return gcvSTATUS_OK;
    }
    while (gcvFALSE);

    *Merged = gcvFALSE;

    return gcvSTATUS_OK;
}

gceSTATUS
cloCODE_EMITTER_EmitCode1(
    IN cloCOMPILER Compiler,
    IN cloCODE_EMITTER CodeEmitter,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN cleOPCODE Opcode,
    IN gcsTARGET * Target,
    IN gcsSOURCE * Source
    )
{
    gceSTATUS    status;
    gctBOOL        merged;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_OBJECT(CodeEmitter, clvOBJ_CODE_EMITTER);

    if (!cloCOMPILER_OptimizationEnabled(Compiler, clvOPTIMIZATION_DATA_FLOW))
    {
        return _EmitCodeImpl1(Compiler,
                      LineNo,
                      StringNo,
                      Opcode,
                      Target,
                      Source);
    }

    status = cloCODE_EMITTER_TryToMergeCode1(Compiler,
                        CodeEmitter,
                        LineNo,
                        StringNo,
                        Opcode,
                        Target,
                        Source,
                        &merged);

    if (gcmIS_ERROR(status)) return status;

    if (!merged)
    {
        status = cloCODE_EMITTER_EmitCurrentCode(Compiler, CodeEmitter);

        if (gcmIS_ERROR(status)) return status;

        CodeEmitter->currentCodeInfo.type    = clvCODE_ONE_OPERAND;
        CodeEmitter->currentCodeInfo.lineNo    = LineNo;
        CodeEmitter->currentCodeInfo.stringNo    = StringNo;
        CodeEmitter->currentCodeInfo.opcode    = Opcode;
        CodeEmitter->currentCodeInfo.target    = *Target;
        CodeEmitter->currentCodeInfo.source0    = *Source;
    }

    return gcvSTATUS_OK;
}

gceSTATUS
cloCODE_EMITTER_EmitCode0(
    IN cloCOMPILER Compiler,
    IN cloCODE_EMITTER CodeEmitter,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN cleOPCODE Opcode
    )
{
    gceSTATUS    status;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_OBJECT(CodeEmitter, clvOBJ_CODE_EMITTER);

    status = cloCODE_EMITTER_EmitCurrentCode(Compiler, CodeEmitter);

    if (gcmIS_ERROR(status)) return status;

    CodeEmitter->currentCodeInfo.type    = clvCODE_NO_OPERANDS;
    CodeEmitter->currentCodeInfo.lineNo    = LineNo;
    CodeEmitter->currentCodeInfo.stringNo    = StringNo;
    CodeEmitter->currentCodeInfo.opcode    = Opcode;
        gcoOS_ZeroMemory(&CodeEmitter->currentCodeInfo.target, gcmSIZEOF(gcsTARGET));
        gcoOS_ZeroMemory(&CodeEmitter->currentCodeInfo.source0, gcmSIZEOF(gcsSOURCE));

    return gcvSTATUS_OK;
}

gceSTATUS
clEmitCode0(
    IN cloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN cleOPCODE Opcode
    )
{
    cloCODE_EMITTER    codeEmitter;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);

    codeEmitter = cloCOMPILER_GetCodeEmitter(Compiler);
    gcmASSERT(codeEmitter);

    return cloCODE_EMITTER_EmitCode0(Compiler,
                     codeEmitter,
                     LineNo,
                      StringNo,
                     Opcode);
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
cloCODE_EMITTER_TryToMergeCode2(
    IN cloCOMPILER Compiler,
    IN cloCODE_EMITTER CodeEmitter,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN cleOPCODE Opcode,
    IN gcsTARGET * Target,
    IN gcsSOURCE * Source0,
    IN gcsSOURCE * Source1,
    OUT gctBOOL * Merged
    )
{
    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_OBJECT(CodeEmitter, clvOBJ_CODE_EMITTER);
    gcmASSERT(Merged);

    do
    {
        /* Check the type and opcode */
        if (CodeEmitter->currentCodeInfo.type != clvCODE_TWO_OPERANDS
            || CodeEmitter->currentCodeInfo.opcode != Opcode) break;

        /* Do not allow merging of target for load: Need more sophisticated checking */
        if (Opcode == clvOPCODE_LOAD) break;

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
                clmMERGE_DATA_TYPE(CodeEmitter->currentCodeInfo.target.dataType, Target->dataType);

        CodeEmitter->currentCodeInfo.source0.dataType =
                clmMERGE_DATA_TYPE(CodeEmitter->currentCodeInfo.source0.dataType, Source0->dataType);

        CodeEmitter->currentCodeInfo.source1.dataType =
                clmMERGE_DATA_TYPE(CodeEmitter->currentCodeInfo.source1.dataType, Source1->dataType);

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

        return gcvSTATUS_OK;
    }
    while (gcvFALSE);

    *Merged = gcvFALSE;

    return gcvSTATUS_OK;
}

gceSTATUS
cloCODE_EMITTER_EmitCode2(
    IN cloCOMPILER Compiler,
    IN cloCODE_EMITTER CodeEmitter,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN cleOPCODE Opcode,
    IN gcsTARGET * Target,
    IN gcsSOURCE * Source0,
    IN gcsSOURCE * Source1
    )
{
    gceSTATUS    status;
    gctBOOL        merged;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_OBJECT(CodeEmitter, clvOBJ_CODE_EMITTER);

    if (!cloCOMPILER_OptimizationEnabled(Compiler, clvOPTIMIZATION_DATA_FLOW))
    {
        return _EmitCodeImpl2(Compiler,
                              LineNo,
                              StringNo,
                              Opcode,
                              Target,
                              Source0,
                              Source1);
    }

    status = cloCODE_EMITTER_TryToMergeCode2(Compiler,
                        CodeEmitter,
                        LineNo,
                        StringNo,
                        Opcode,
                        Target,
                        Source0,
                        Source1,
                        &merged);

    if (gcmIS_ERROR(status)) return status;

    if (!merged)
    {
        status = cloCODE_EMITTER_EmitCurrentCode(Compiler, CodeEmitter);

        if (gcmIS_ERROR(status)) return status;

        CodeEmitter->currentCodeInfo.type    = clvCODE_TWO_OPERANDS;
        CodeEmitter->currentCodeInfo.lineNo    = LineNo;
        CodeEmitter->currentCodeInfo.stringNo    = StringNo;
        CodeEmitter->currentCodeInfo.opcode    = Opcode;
        CodeEmitter->currentCodeInfo.target    = *Target;
        CodeEmitter->currentCodeInfo.source0    = *Source0;
        CodeEmitter->currentCodeInfo.source1    = *Source1;
    }

    return gcvSTATUS_OK;
}
