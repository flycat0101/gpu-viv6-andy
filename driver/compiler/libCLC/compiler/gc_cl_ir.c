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


#include "gc_cl_scanner.h"
#include "gc_cl_ir.h"
#include "gc_cl_built_ins.h"
#include "gc_cl_gen_code.h"
#include "gc_cl_compiler.h"

#define cldCONSTANT_minBufferSize 16 * 16;
static cluCONSTANT_VALUE * clResultBuffer = gcvNULL;
static gctUINT    clResultBufferSize = 0;

static gctUINT
clsDECL_GetElementAlignment(
cloCOMPILER Compiler,
clsDECL *Decl
)
{
   gctUINT alignment = 0;
   clsNAME *fieldName;

   if(clmDECL_IsPointerType(Decl)) {
      alignment = 4;
   }
   else {
      switch(Decl->dataType->elementType) {
      case clvTYPE_STRUCT:
      case clvTYPE_UNION:
         {
           gctUINT structAlignment = 0;

           gcmASSERT(Decl->dataType->u.fieldSpace);

           FOR_EACH_DLINK_NODE(&Decl->dataType->u.fieldSpace->names, clsNAME, fieldName) {
              gcmASSERT(fieldName->decl.dataType);

              if(fieldName->u.variableInfo.specifiedAttr & clvATTR_PACKED) {
                 alignment = 1;
              }
              else {
                 if(fieldName->u.variableInfo.specifiedAttr & clvATTR_ALIGNED) {
                    alignment = fieldName->context.alignment;
                 }
                 else {
                    if(clmDECL_IsUnderlyingStructOrUnion(&fieldName->decl)) {
                       clsNAME *subField;
                   subField = slsDLINK_LIST_First(&fieldName->decl.dataType->u.fieldSpace->names, struct _clsNAME);
                       if(subField->u.variableInfo.specifiedAttr & clvATTR_ALIGNED) {
                          alignment = subField->context.alignment;
                       }
                       else alignment = clsDECL_GetElementAlignment(Compiler, &fieldName->decl);
                    }
                    else alignment = clsDECL_GetElementAlignment(Compiler, &fieldName->decl);
                 }
              }
              if(structAlignment == 0) structAlignment = alignment;
              else {
                 structAlignment = clFindLCM(structAlignment, alignment);
              }
           }
           alignment = structAlignment;
         }
         break;

      case clvTYPE_VOID:
         alignment = 1;
         break;

      case clvTYPE_FLOAT:
      case clvTYPE_BOOL:
      case clvTYPE_INT:
      case clvTYPE_UINT:
         alignment = 4;
         break;

      case clvTYPE_LONG:
      case clvTYPE_ULONG:
         alignment = 8;
         break;

      case clvTYPE_CHAR:
      case clvTYPE_UCHAR:
         alignment = 1;
         break;

      case clvTYPE_BOOL_PACKED:
      case clvTYPE_CHAR_PACKED:
      case clvTYPE_UCHAR_PACKED:
         alignment = 1;
         break;

      case clvTYPE_SHORT:
      case clvTYPE_USHORT:
         alignment = 2;
         break;

      case clvTYPE_SHORT_PACKED:
      case clvTYPE_USHORT_PACKED:
         alignment = 2;
         break;

      case clvTYPE_HALF:
         alignment = 2;
         break;

      case clvTYPE_HALF_PACKED:
         alignment = 2;
         break;

      case clvTYPE_SAMPLER_T:
         alignment = 4;
         break;

      case clvTYPE_IMAGE2D_T:
      case clvTYPE_IMAGE3D_T:
      case clvTYPE_IMAGE1D_T:
      case clvTYPE_IMAGE2D_ARRAY_T:
      case clvTYPE_IMAGE1D_ARRAY_T:
      case clvTYPE_IMAGE1D_BUFFER_T:
      case clvTYPE_VIV_GENERIC_IMAGE_T:
         if (cloCOMPILER_ExtensionEnabled(Compiler, clvEXTENSION_VIV_VX) ||
             gcmOPT_oclUseImgIntrinsicQuery()) {
             alignment = 16;
         }
         else {
             alignment = 4;
         }
         break;

      case clvTYPE_DOUBLE:
         alignment = 8;
         break;

      case clvTYPE_EVENT_T:
         alignment = 4;
         break;

      default:
         gcmASSERT(0);
         return 1;
      }

      if (clmDATA_TYPE_vectorSize_GET(Decl->dataType) > 0) {
          if (clmDATA_TYPE_vectorSize_NOCHECK_GET(Decl->dataType) == 3) {
              /* 3-component vector data type must be aligned to a 4*sizeof(component) */
              alignment *=4;
          } else {
              alignment *= clmDATA_TYPE_vectorSize_NOCHECK_GET(Decl->dataType);
          }
      }
      else if (clmDATA_TYPE_matrixColumnCount_GET(Decl->dataType) > 0) {
          alignment *= clmDATA_TYPE_matrixRowCount_GET(Decl->dataType) * clmDATA_TYPE_matrixColumnCount_GET(Decl->dataType);
      }
   }

   return  alignment;
}

/*Determine the alignment #*/
gctUINT
clPermissibleAlignment(
cloCOMPILER Compiler,
clsDECL *Decl
)
{
   gctUINT alignment;

   alignment = clsDECL_GetElementAlignment(Compiler, Decl);
   return (alignment > cldMachineByteAlignment) ? cldMachineByteAlignment : alignment;
}

static cluCONSTANT_VALUE *
_GetResultBuffer(
IN gctUINT elemCount
)
{
    if(elemCount > clResultBufferSize) {
       if(clResultBufferSize) {
        clFree((gctPOINTER) clResultBuffer);
       }
       else clResultBufferSize = cldCONSTANT_minBufferSize;
           while (elemCount > clResultBufferSize ) clResultBufferSize *= 2;
       clResultBuffer = (cluCONSTANT_VALUE *)clMalloc((gctSIZE_T)sizeof(cluCONSTANT_VALUE) * clResultBufferSize);
    }
    return clResultBuffer;
}

gceSTATUS
clsDATA_TYPE_Destroy(
IN cloCOMPILER Compiler,
IN clsDATA_TYPE * DataType
)
{
        gcmHEADER_ARG("Compiler=0x%x DataType=0x%x", Compiler, DataType);

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    gcmVERIFY_ARGUMENT(DataType);
    gcmVERIFY_OK(cloCOMPILER_Free(Compiler, DataType));

        gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gctCONST_STRING
clGetElementTypeName(IN cltELEMENT_TYPE ElementType)
{
    switch (ElementType) {
    case clvTYPE_VOID:              return "void";

    case clvTYPE_BOOL:              return "bool";
    case clvTYPE_CHAR:              return "char";
    case clvTYPE_UCHAR:             return "uchar";
    case clvTYPE_INT:               return "int";
    case clvTYPE_UINT:              return "uint";
    case clvTYPE_SHORT:             return "short";
    case clvTYPE_USHORT:            return "ushort";
    case clvTYPE_FLOAT:             return "float";
    case clvTYPE_HALF:              return "half";

    case clvTYPE_BOOL_PACKED:       return "bool_packed";
    case clvTYPE_CHAR_PACKED:       return "char_packed";
    case clvTYPE_UCHAR_PACKED:      return "uchar_packed";
    case clvTYPE_SHORT_PACKED:      return "short_packed";
    case clvTYPE_USHORT_PACKED:     return "ushort_packed";
    case clvTYPE_HALF_PACKED:       return "half_packed";

    case clvTYPE_SAMPLER_T:         return "sampler_t";
    case clvTYPE_IMAGE2D_T:         return "image2d_t";
    case clvTYPE_IMAGE2D_ARRAY_T:   return "image2d_array_t";
    case clvTYPE_IMAGE3D_T:         return "image3d_t";
    case clvTYPE_IMAGE1D_T:         return "image1d_t";
    case clvTYPE_IMAGE1D_ARRAY_T:   return "image1d_array_t";
    case clvTYPE_IMAGE1D_BUFFER_T:  return "image1d_buffer_t";
    case clvTYPE_VIV_GENERIC_IMAGE_T: return "viv_generic_image_t";


    case clvTYPE_STRUCT:            return "struct";
    case clvTYPE_UNION:             return "union";

    case clvTYPE_TYPEDEF:           return "typedef";

    default:
        gcmASSERT(0);
        return "invalid";
    }
}

gctCONST_STRING
clGetStorageQualifierName(
IN cltQUALIFIER Qualifier
)
{
    static gctCONST_STRING storageQualifierName[clvSTORAGE_QUALIFIER_LAST_ONE << 1] = {
        "none",
        "restrict",
        "volatile",
        "volatile restrict",
        "static",
        "static restrict",
        "static volatile",
        "static volatile restrict",
        "extern",
        "extern restrict",
        "extern volatile",
        "extern volatile restrict",
        "extern static",
        "extern static restrict",
        "extern static volatile",
        "extern static volatile restrict"
        };

    if(Qualifier < (clvSTORAGE_QUALIFIER_LAST_ONE << 1)) {
        gcmASSERT(storageQualifierName[Qualifier]);
        return storageQualifierName[Qualifier];
    }
    else {
        gcmASSERT(0);
        return "invalid";
    }
}

gctCONST_STRING
clGetQualifierName(
IN cltQUALIFIER Qualifier
)
{
    switch (Qualifier) {
    case clvQUALIFIER_NONE:       return "none";
    case clvQUALIFIER_CONST:      return "const";

    case clvQUALIFIER_UNIFORM:    return "uniform";
    case clvQUALIFIER_ATTRIBUTE:  return "attribute";

    case clvQUALIFIER_CONST_IN:   return "const_in";

    case clvQUALIFIER_CONSTANT:   return "constant";
    case clvQUALIFIER_GLOBAL:     return "global";
    case clvQUALIFIER_LOCAL:      return "local";
    case clvQUALIFIER_PRIVATE:    return "private";

    case clvQUALIFIER_READ_ONLY:  return "read_only";
    case clvQUALIFIER_WRITE_ONLY: return "write_only";

    case clvQUALIFIER_OUT:        return "function_value";

    default:
        gcmASSERT(0);
        return "invalid";
    }
}

void
clGetPointedToAddrSpace(
clsDECL *Decl,
cltQUALIFIER *AddrSpaceQualifier
)
{
    if(clmDECL_IsPointerType(Decl)) {
        clsTYPE_QUALIFIER *nextDscr;
        clsTYPE_QUALIFIER *prevDscr;

        FOR_EACH_SLINK_NODE(Decl->ptrDscr, clsTYPE_QUALIFIER, prevDscr, nextDscr) {
            if(nextDscr->type == T_EOF) break;
            switch(nextDscr->type) {
            case T_CONSTANT:
            case T_LOCAL:
            case T_GLOBAL:
            case T_PRIVATE:
                *AddrSpaceQualifier = nextDscr->qualifier;
                break;

            default:
                break;
            }
        }
    }

    return;
}

cltQUALIFIER
clGetAddrSpaceQualifier(
clsDECL *Decl
)
{
    cltQUALIFIER addrSpaceQualifier = Decl->dataType->addrSpaceQualifier;

    clGetPointedToAddrSpace(Decl,
                            &addrSpaceQualifier);
    return addrSpaceQualifier;
}

gceSTATUS
clsDATA_TYPE_Dump(
IN cloCOMPILER Compiler,
IN clsDATA_TYPE * DataType
)
{
    gcmHEADER_ARG("Compiler=0x%x DataType=0x%x", Compiler, DataType);

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    gcmVERIFY_ARGUMENT(DataType);

    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                      clvDUMP_IR,
                    "<DATA_TYPE this=\"0x%x\" qualifier=\"%s\""
                    " elementType=\"%s\" vectorSize=\"%d\""
                    " matrixSize=\"%d\"x\"%d\" generic=\"0x%x\" />",
                    DataType,
                    clGetQualifierName(DataType->accessQualifier),
                    clGetElementTypeName(DataType->elementType),
                    clmDATA_TYPE_vectorSize_GET(DataType),
                    clmDATA_TYPE_matrixRowCount_GET(DataType),
                    clmDATA_TYPE_matrixColumnCount_GET(DataType),
                    DataType->u.generic));

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

static gctBOOL
_IsSameArraySize(
IN clsARRAY *array1,
IN clsARRAY *array2
)
{
   if(array1->numDim == array2->numDim) {
      int i;
      for(i = 0; i < array1->numDim; i++ ) {
         if(array1->length[i] != array2->length[i]) {
            return gcvFALSE;
         }
      }
      return gcvTRUE;
   }
   else return gcvFALSE;
}

gctBOOL
clsDECL_IsEqual(
IN clsDECL * Decl1,
IN clsDECL * Decl2
)
{
  gcmASSERT(Decl1);
  gcmASSERT(Decl2);

  if((Decl1->dataType->elementType == Decl2->dataType->elementType) &&
     (clmDATA_TYPE_vectorSize_NOCHECK_GET(Decl1->dataType) ==
      clmDATA_TYPE_vectorSize_NOCHECK_GET(Decl2->dataType)) &&
     (clmDATA_TYPE_matrixRowCount_GET(Decl1->dataType) ==
      clmDATA_TYPE_matrixRowCount_GET(Decl2->dataType)) &&
     (clmDATA_TYPE_matrixColumnCount_GET(Decl1->dataType) ==
      clmDATA_TYPE_matrixColumnCount_GET(Decl2->dataType)) &&
     _IsSameArraySize(&Decl1->array, &Decl2->array) &&
     (Decl1->dataType->u.generic == Decl2->dataType->u.generic)) {

     if(Decl1->ptrDscr) {
        if(Decl2->ptrDscr) {
           return clParseCountIndirectionLevel(Decl1->ptrDscr) ==
                  clParseCountIndirectionLevel(Decl2->ptrDscr);
        }
     }
     else return Decl2->ptrDscr == gcvNULL;
  }
  return gcvFALSE;
}

gctBOOL
clsDECL_IsAssignableTo(
IN clsDECL * LDecl,
IN clsDECL * RDecl
)
{
  gctBOOL sameType = gcvFALSE, sameElementType = gcvFALSE;
  gcmASSERT(LDecl);
  gcmASSERT(RDecl);

  if(clmDECL_IsScalar(RDecl)) {
      if(!clmDECL_IsPointerType(LDecl)) {
          if(LDecl->dataType->elementType == clvTYPE_BOOL) {
              return gcvTRUE;
          }
          else if((!clmDECL_IsPointerType(RDecl) &&
                   (clmDECL_IsArithmeticType(LDecl) ||
                    clmDECL_IsPackedGenType(LDecl))) ||
                  (clmDECL_IsIntegerType(RDecl) &&
                   clmIsElementTypeEvent(LDecl->dataType->elementType))) {/*implicit conversion */
              return gcvTRUE;
          }
      }
      else if((LDecl->dataType->elementType == clvTYPE_VOID &&
              clmDECL_IsPointerType(RDecl)) ||
              (clmDECL_IsPointerType(RDecl) &&
              RDecl->dataType->elementType == clvTYPE_VOID)) {
          return gcvTRUE;
      }
  }
  else if((clmDECL_IsPackedGenType(LDecl) &&
      clmIsElementTypePacked(RDecl->dataType->elementType)) ||
      (clmDECL_IsFloatingType(RDecl) && clmDECL_IsFloatingType(LDecl) &&
       (RDecl->dataType->elementType == clvTYPE_HALF || LDecl->dataType->elementType == clvTYPE_HALF))) {
      sameType = gcvTRUE;
  }
  else if(clmDECL_IsPackedType(RDecl))
  {
      clsBUILTIN_DATATYPE_INFO *typeInfo;

      typeInfo = clGetBuiltinDataTypeInfo(LDecl->dataType->type);
      sameElementType = typeInfo->dualType == RDecl->dataType->type;
  }
  else if(clmDECL_IsPackedType(LDecl))
  {
      clsBUILTIN_DATATYPE_INFO *typeInfo;

      typeInfo = clGetBuiltinDataTypeInfo(RDecl->dataType->type);
      sameElementType = typeInfo->dualType == LDecl->dataType->type;
  }
  sameElementType = sameElementType || (LDecl->dataType->elementType == RDecl->dataType->elementType);

  if(sameType ||
     (sameElementType &&
     (clmDATA_TYPE_matrixRowCount_GET(LDecl->dataType) == clmDATA_TYPE_matrixRowCount_GET(RDecl->dataType)) &&
     (clmDATA_TYPE_matrixColumnCount_GET(LDecl->dataType) == clmDATA_TYPE_matrixColumnCount_GET(RDecl->dataType)) &&
     (LDecl->dataType->u.generic == RDecl->dataType->u.generic))) {
    if(!clmDECL_IsArray(LDecl)) {
       if(clmDECL_IsArray(RDecl)) {
          return (clParseCountIndirectionLevel(LDecl->ptrDscr) == 1);
       }
       else {
          gctINT lCount, rCount;

          lCount = clParseCountIndirectionLevel(LDecl->ptrDscr);
          rCount = clParseCountIndirectionLevel(RDecl->ptrDscr);

          if(lCount == rCount) return gcvTRUE;
          else return gcvFALSE;
       }
    }
  }
  return gcvFALSE;
}

gctBOOL
clsDECL_IsInitializableTo(
IN clsDECL * LDecl,
IN clsDECL * RDecl
)
{
  gctBOOL sameType = gcvFALSE, sameElementType = gcvFALSE;
  gcmASSERT(LDecl);
  gcmASSERT(RDecl);

  if(clmDECL_IsScalar(RDecl)) {
    if(!clmDECL_IsPointerType(LDecl)) {
       if(LDecl->dataType->elementType == clvTYPE_BOOL) {
         return gcvTRUE;
       }
       else if((!clmDECL_IsPointerType(RDecl)
               && clmDECL_IsArithmeticType(LDecl)) ||
               (clmDECL_IsIntegerType(RDecl) &&
                clmIsElementTypeEvent(LDecl->dataType->elementType))) {/*implicit conversion */
         return gcvTRUE;
       }
    }
    else if(clmDECL_IsPointerType(RDecl) ||
            clmDECL_IsInt(RDecl)) {
       return gcvTRUE;
    }
  }
  else if(clmDECL_IsPackedGenType(RDecl) &&
     clmIsElementTypePacked(LDecl->dataType->elementType)) {
      sameType = gcvTRUE;
  }
  else if(clmDECL_IsPackedType(RDecl))
  {
      clsBUILTIN_DATATYPE_INFO *typeInfo;

      typeInfo = clGetBuiltinDataTypeInfo(LDecl->dataType->type);
      sameElementType = typeInfo->dualType == RDecl->dataType->type;
  }
  else if(clmDECL_IsPackedType(LDecl))
  {
      clsBUILTIN_DATATYPE_INFO *typeInfo;

      typeInfo = clGetBuiltinDataTypeInfo(RDecl->dataType->type);
      sameElementType = typeInfo->dualType == LDecl->dataType->type;
  }
  sameElementType = sameElementType || (LDecl->dataType->elementType == RDecl->dataType->elementType);

  if(sameType ||
     (sameElementType &&
     (clmDATA_TYPE_vectorSize_NOCHECK_GET(LDecl->dataType) == clmDATA_TYPE_vectorSize_NOCHECK_GET(RDecl->dataType)) &&
     (clmDATA_TYPE_matrixRowCount_GET(LDecl->dataType) == clmDATA_TYPE_matrixRowCount_GET(RDecl->dataType)) &&
     (clmDATA_TYPE_matrixColumnCount_GET(LDecl->dataType) == clmDATA_TYPE_matrixColumnCount_GET(RDecl->dataType)) &&
     (LDecl->dataType->u.generic == RDecl->dataType->u.generic))) {
    if(!clmDECL_IsArray(LDecl)) {
       if(clmDECL_IsArray(RDecl)) {
          return clParseCountIndirectionLevel(LDecl->ptrDscr) == 1;
       }
       else {
          return clParseCountIndirectionLevel(LDecl->ptrDscr) ==
                 clParseCountIndirectionLevel(RDecl->ptrDscr);
       }
    }
    else if(_IsSameArraySize(&LDecl->array, &RDecl->array)) {
          return clParseCountIndirectionLevel(LDecl->ptrDscr) ==
                 clParseCountIndirectionLevel(RDecl->ptrDscr);

    }
  }
  return gcvFALSE;
}

gctSIZE_T
clAlignMemory(
cloCOMPILER Compiler,
clsNAME *Variable,
gctSIZE_T MemorySize
)
{
   gctUINT alignment;

   gcmASSERT(Variable);
   if(Variable->u.variableInfo.specifiedAttr & clvATTR_ALIGNED) {
       alignment = Variable->context.alignment;
   }
   else {
       alignment = clPermissibleAlignment(Compiler, &Variable->decl);
   }
   return gcmALIGN(MemorySize, alignment);
}

gctSIZE_T
clsDECL_GetByteSize(
IN cloCOMPILER Compiler,
IN clsDECL *Decl
)
{
   gctUINT size = 0;
   clsNAME *fieldName;
   gctBOOL packed = gcvFALSE;

   if(clmDECL_IsPointerType(Decl) ||
      clmDECL_IsPointerArray(Decl)) {
      size = 4;
   }
   else {
      switch(Decl->dataType->elementType) {
      case clvTYPE_STRUCT:
      case clvTYPE_UNION:
         {
           gctUINT localSize = 0;
           gctUINT curSize;
           gctUINT structAlignment = 0;
           gctUINT alignment;

           gcmASSERT(Decl->dataType->u.fieldSpace);


           FOR_EACH_DLINK_NODE(&Decl->dataType->u.fieldSpace->names, clsNAME, fieldName) {
              gcmASSERT(fieldName->decl.dataType);

              if(fieldName->u.variableInfo.specifiedAttr & clvATTR_PACKED) {
                packed = gcvTRUE;
              }
              else {
                packed = gcvFALSE;
              }
              if(fieldName->u.variableInfo.specifiedAttr & clvATTR_ALIGNED) {
                 alignment = fieldName->context.alignment;
              }
              else {
                 if(clmDECL_IsUnderlyingStructOrUnion(&fieldName->decl)) {
                    clsNAME *subField;
                    subField = slsDLINK_LIST_First(&fieldName->decl.dataType->u.fieldSpace->names, struct _clsNAME);
                    if(subField->u.variableInfo.specifiedAttr & clvATTR_ALIGNED) {
                       alignment = subField->context.alignment;
                    }
                    else alignment = clPermissibleAlignment(Compiler, &fieldName->decl);
                 }
                 else alignment = clPermissibleAlignment(Compiler, &fieldName->decl);
              }
              if(structAlignment == 0) structAlignment = alignment;
              else {
                 structAlignment = clFindLCM(structAlignment, alignment);
              }

              curSize = clsDECL_GetByteSize(Compiler, &fieldName->decl);
              localSize = clmALIGN(localSize, alignment, packed);

              if(Decl->dataType->elementType == clvTYPE_UNION) {
                  if(curSize > localSize) localSize = curSize;
              }
              else
                  localSize += curSize;
           }

           size += clmALIGN(localSize, structAlignment, packed);
         }
         break;

      case clvTYPE_VOID:
         size = 0;
         break;

      case clvTYPE_FLOAT:
      case clvTYPE_BOOL:
      case clvTYPE_INT:
      case clvTYPE_UINT:
         size = 4;
         break;

      case clvTYPE_LONG:
      case clvTYPE_ULONG:
         size = 8;
         break;

      case clvTYPE_CHAR:
      case clvTYPE_UCHAR:
         size = 1;
         break;

      case clvTYPE_BOOL_PACKED:
      case clvTYPE_CHAR_PACKED:
      case clvTYPE_UCHAR_PACKED:
         size = 1;
         break;

      case clvTYPE_SHORT:
      case clvTYPE_USHORT:
      case clvTYPE_HALF:
         size = 2;
         break;

      case clvTYPE_SHORT_PACKED:
      case clvTYPE_USHORT_PACKED:
      case clvTYPE_HALF_PACKED:
         size = 2;
         break;

      case clvTYPE_SAMPLER_T:
         size = 4;
         break;

      case clvTYPE_IMAGE2D_T:
      case clvTYPE_IMAGE3D_T:
      case clvTYPE_IMAGE1D_T:
      case clvTYPE_IMAGE2D_ARRAY_T:
      case clvTYPE_IMAGE1D_ARRAY_T:
      case clvTYPE_IMAGE1D_BUFFER_T:
      case clvTYPE_VIV_GENERIC_IMAGE_T:
         if (cloCOMPILER_ExtensionEnabled(Compiler, clvEXTENSION_VIV_VX) ||
             gcmOPT_oclUseImgIntrinsicQuery()) {
             size = 32;
         }
         else size = 4;
         break;

      case clvTYPE_DOUBLE:
         size = 8;
         break;

      case clvTYPE_EVENT_T:
         size = 4;
         break;

      default:
         gcmASSERT(0);
         return 0;
      }

      if (clmDATA_TYPE_vectorSize_GET(Decl->dataType) > 0) {
          if (clmDATA_TYPE_vectorSize_NOCHECK_GET(Decl->dataType) == 3) {
              /* 3-component vector data type must be aligned to a 4*sizeof(component) */
              size*=4;
          } else {
              size *= clmDATA_TYPE_vectorSize_NOCHECK_GET(Decl->dataType);
          }
      }
      else if (clmDATA_TYPE_matrixColumnCount_GET(Decl->dataType) > 0) {
         size *= clmDATA_TYPE_matrixRowCount_GET(Decl->dataType) * clmDATA_TYPE_matrixColumnCount_GET(Decl->dataType);
      }
   }

   if (clmDECL_IsArray(Decl)) {
      gctUINT elementCount;
      clmGetArrayElementCount(Decl->array, 0, elementCount);
      size *= elementCount;
   }
   return  size;
}

gctSIZE_T
clsDECL_GetElementByteSize(
IN cloCOMPILER Compiler,
IN clsDECL *Decl,
OUT gctUINT *Alignment,
OUT gctBOOL *Packed
)
{
   gctUINT size = 0;
   clsNAME *fieldName;
   gctUINT alignment = 0;
   gctBOOL packed = gcvFALSE;

   if(clmDECL_IsPointerType(Decl) ||
      clmDECL_IsPointerArray(Decl)) {
      size = 4;
      alignment = 4;
   }
   else {
      switch(Decl->dataType->elementType) {
      case clvTYPE_STRUCT:
      case clvTYPE_UNION:
         {
           gctUINT localSize = 0;
           gctUINT curSize;
           gctUINT structAlignment = 0;

           gcmASSERT(Decl->dataType->u.fieldSpace);

           FOR_EACH_DLINK_NODE(&Decl->dataType->u.fieldSpace->names, clsNAME, fieldName) {
              gcmASSERT(fieldName->decl.dataType);

              if(fieldName->u.variableInfo.specifiedAttr & clvATTR_PACKED) {
                 packed = gcvTRUE;
              }
              else {
                 packed = gcvFALSE;
              }
              if(fieldName->u.variableInfo.specifiedAttr & clvATTR_ALIGNED) {
                 alignment = fieldName->context.alignment;
              }
              else {
                 if(clmDECL_IsUnderlyingStructOrUnion(&fieldName->decl)) {
                    clsNAME *subField;
                    subField = slsDLINK_LIST_First(&fieldName->decl.dataType->u.fieldSpace->names, struct _clsNAME);
                    if(subField->u.variableInfo.specifiedAttr & clvATTR_ALIGNED) {
                       alignment = subField->context.alignment;
                    }
                    else alignment = clPermissibleAlignment(Compiler, &fieldName->decl);
                 }
                 else alignment = clPermissibleAlignment(Compiler, &fieldName->decl);
              }
              if(structAlignment == 0) structAlignment = alignment;
              else {
                 structAlignment = clFindLCM(structAlignment, alignment);
              }

              curSize = clsDECL_GetByteSize(Compiler, &fieldName->decl);
              localSize = clmALIGN(localSize, alignment, packed);

              if(Decl->dataType->elementType == clvTYPE_UNION) {
                 if(curSize > localSize) localSize = curSize;
              }
              else localSize += curSize;
           }

           size += clmALIGN(localSize, structAlignment, packed);
           alignment = structAlignment;
         }
         break;

      case clvTYPE_VOID:
         size = 0;
         alignment = 1;
         break;

      case clvTYPE_FLOAT:
      case clvTYPE_BOOL:
      case clvTYPE_INT:
      case clvTYPE_UINT:
         size = 4;
         alignment = 4;
         break;

      case clvTYPE_LONG:
      case clvTYPE_ULONG:
         size = 8;
         alignment = 8;
         break;

      case clvTYPE_CHAR:
      case clvTYPE_UCHAR:
         size = 1;
         alignment = 1;
         break;

      case clvTYPE_BOOL_PACKED:
      case clvTYPE_CHAR_PACKED:
      case clvTYPE_UCHAR_PACKED:
         size = 1;
         alignment = 1;
         break;

      case clvTYPE_SHORT:
      case clvTYPE_USHORT:
      case clvTYPE_HALF:
         size = 2;
         alignment = 2;
         break;

      case clvTYPE_SHORT_PACKED:
      case clvTYPE_USHORT_PACKED:
      case clvTYPE_HALF_PACKED:
         size = 2;
         alignment = 2;
         break;

      case clvTYPE_SAMPLER_T:
         size = 4;
         alignment = 4;
         break;

      case clvTYPE_IMAGE2D_T:
      case clvTYPE_IMAGE3D_T:
      case clvTYPE_IMAGE1D_T:
      case clvTYPE_IMAGE2D_ARRAY_T:
      case clvTYPE_IMAGE1D_ARRAY_T:
      case clvTYPE_IMAGE1D_BUFFER_T:
      case clvTYPE_VIV_GENERIC_IMAGE_T:
         if (cloCOMPILER_ExtensionEnabled(Compiler, clvEXTENSION_VIV_VX) ||
             gcmOPT_oclUseImgIntrinsicQuery()) {
             size = 32;
             alignment = 16;
         }
         else {
             size = 4;
             alignment = 4;
         }
         break;

      case clvTYPE_DOUBLE:
         size = 8;
         alignment = 8;
         break;

      case clvTYPE_EVENT_T:
         size = 4;
         alignment = 4;
         break;

      default:
         gcmASSERT(0);
         return 0;
      }

      if (clmDATA_TYPE_vectorSize_GET(Decl->dataType) > 0) {
          if (clmDATA_TYPE_vectorSize_NOCHECK_GET(Decl->dataType) == 3) {
              /* 3-component vector data type must be aligned to a 4*sizeof(component) */
              size*=4;
              alignment *=4;
          } else {
              size *= clmDATA_TYPE_vectorSize_NOCHECK_GET(Decl->dataType);
              alignment *= clmDATA_TYPE_vectorSize_NOCHECK_GET(Decl->dataType);
          }
      }
      else if (clmDATA_TYPE_matrixColumnCount_GET(Decl->dataType) > 0) {
         gctUINT8 rowCount;
         rowCount = clmDATA_TYPE_matrixRowCount_GET(Decl->dataType);
         if(rowCount == 3) rowCount++;
         size *= rowCount * clmDATA_TYPE_matrixColumnCount_GET(Decl->dataType);
         alignment *= rowCount;
      }
   }

   if(Alignment)
   {
       *Alignment = alignment;
   }
   if(Packed)
   {
       *Packed = packed;
   }
   return  size;
}

gctSIZE_T
clsDECL_GetPointedToByteSize(
IN cloCOMPILER Compiler,
IN clsDECL *Decl
)
{
   gctUINT size = 0;
   clsNAME *fieldName;
   gctBOOL packed = gcvFALSE;

   gcmASSERT(clmDECL_IsPointerType(Decl));

   if(clParseCountIndirectionLevel(Decl->ptrDscr) > 1) {
    return 4;
   }

   switch(Decl->dataType->elementType) {
   case clvTYPE_STRUCT:
   case clvTYPE_UNION:
      {
        gctUINT localSize = 0;
        gctUINT curSize;
        gctUINT structAlignment = 0;
        gctUINT alignment;

        gcmASSERT(Decl->dataType->u.fieldSpace);


        FOR_EACH_DLINK_NODE(&Decl->dataType->u.fieldSpace->names, clsNAME, fieldName) {
           gcmASSERT(fieldName->decl.dataType);

           if(fieldName->u.variableInfo.specifiedAttr & clvATTR_PACKED) {
              packed = gcvTRUE;
           }
           else {
              packed = gcvFALSE;
           }
           if(fieldName->u.variableInfo.specifiedAttr & clvATTR_ALIGNED) {
              alignment = fieldName->context.alignment;
           }
           else {
              if(clmDECL_IsUnderlyingStructOrUnion(&fieldName->decl)) {
                 clsNAME *subField;
                 subField = slsDLINK_LIST_First(&fieldName->decl.dataType->u.fieldSpace->names, struct _clsNAME);
                 if(subField->u.variableInfo.specifiedAttr & clvATTR_ALIGNED) {
                    alignment = subField->context.alignment;
                 }
                 else alignment = clPermissibleAlignment(Compiler, &fieldName->decl);
              }
              else alignment = clPermissibleAlignment(Compiler, &fieldName->decl);
           }
           if(structAlignment == 0) structAlignment = alignment;
           else {
              structAlignment = clFindLCM(structAlignment, alignment);
           }

           curSize = clsDECL_GetByteSize(Compiler, &fieldName->decl);
           localSize = clmALIGN(localSize, alignment, packed);

           if(Decl->dataType->elementType == clvTYPE_UNION) {
              if(curSize > localSize) localSize = curSize;
           }
           else localSize += curSize;
        }
        size += clmALIGN(localSize, structAlignment, packed);
      }
      break;

   case clvTYPE_VOID:
      size = 0;
      break;

   case clvTYPE_FLOAT:
   case clvTYPE_BOOL:
   case clvTYPE_INT:
   case clvTYPE_UINT:
      size = 4;
      break;

   case clvTYPE_LONG:
   case clvTYPE_ULONG:
      size = 8;
      break;

   case clvTYPE_CHAR:
   case clvTYPE_UCHAR:
   case clvTYPE_CHAR_PACKED:
   case clvTYPE_UCHAR_PACKED:
   case clvTYPE_BOOL_PACKED:
      size = 1;
      break;

   case clvTYPE_SHORT:
   case clvTYPE_USHORT:
   case clvTYPE_SHORT_PACKED:
   case clvTYPE_USHORT_PACKED:
      size = 2;
      break;

   case clvTYPE_HALF:
   case clvTYPE_HALF_PACKED:
      size = 2;
      break;

   case clvTYPE_SAMPLER_T:
      size = 4;
      break;

   case clvTYPE_IMAGE2D_T:
   case clvTYPE_IMAGE3D_T:
   case clvTYPE_IMAGE1D_T:
   case clvTYPE_IMAGE2D_ARRAY_T:
   case clvTYPE_IMAGE1D_ARRAY_T:
   case clvTYPE_IMAGE1D_BUFFER_T:
   case clvTYPE_VIV_GENERIC_IMAGE_T:
      if (cloCOMPILER_ExtensionEnabled(Compiler, clvEXTENSION_VIV_VX) ||
          gcmOPT_oclUseImgIntrinsicQuery()) {
          size = 32;
      }
      else size = 4;
      break;

   case clvTYPE_DOUBLE:
      size = 8;
      break;

   default:
      gcmASSERT(0);
      return 0;
   }

   if (clmDATA_TYPE_vectorSize_GET(Decl->dataType) > 0) {
      if (clmDATA_TYPE_vectorSize_NOCHECK_GET(Decl->dataType) == 3) {
      /* 3-component vector data type must be aligned to a 4*sizeof(component) */
         size*=4;
      } else {
         size *= clmDATA_TYPE_vectorSize_NOCHECK_GET(Decl->dataType);
      }
   }
   else if (clmDATA_TYPE_matrixColumnCount_GET(Decl->dataType) > 0) {
      size *= clmDATA_TYPE_matrixRowCount_GET(Decl->dataType) * clmDATA_TYPE_matrixColumnCount_GET(Decl->dataType);
   }

   if (clmDECL_IsUnderlyingArray(Decl)) {
      gctUINT elementCount;
      clmGetArrayElementCount(Decl->array, 0, elementCount);
      size *= elementCount;
   }
   return  size;
}

gctSIZE_T
clGetElementTypeByteSize(
cloCOMPILER Compiler,
cltELEMENT_TYPE ElementType
)
{
      gctSIZE_T size = 0;

      switch(ElementType) {
      case clvTYPE_VOID:
         size = 0;
         break;

      case clvTYPE_FLOAT:
      case clvTYPE_BOOL:
      case clvTYPE_INT:
      case clvTYPE_UINT:
         size = 4;
         break;

      case clvTYPE_LONG:
      case clvTYPE_ULONG:
         size = 8;
         break;

      case clvTYPE_CHAR:
      case clvTYPE_UCHAR:
         size = 1;
         break;

      case clvTYPE_BOOL_PACKED:
      case clvTYPE_CHAR_PACKED:
      case clvTYPE_UCHAR_PACKED:
         size = 1;
         break;

      case clvTYPE_SHORT:
      case clvTYPE_USHORT:
      case clvTYPE_HALF:
         size = 2;
         break;

      case clvTYPE_SHORT_PACKED:
      case clvTYPE_USHORT_PACKED:
      case clvTYPE_HALF_PACKED:
         size = 2;
         break;

      case clvTYPE_SAMPLER_T:
         size = 4;
         break;

      case clvTYPE_IMAGE2D_T:
      case clvTYPE_IMAGE3D_T:
      case clvTYPE_IMAGE1D_T:
      case clvTYPE_IMAGE2D_ARRAY_T:
      case clvTYPE_IMAGE1D_ARRAY_T:
      case clvTYPE_IMAGE1D_BUFFER_T:
      case clvTYPE_VIV_GENERIC_IMAGE_T:
        if (cloCOMPILER_ExtensionEnabled(Compiler, clvEXTENSION_VIV_VX) ||
            gcmOPT_oclUseImgIntrinsicQuery()) {
             size = 32;
         }
         else size = 4;
         break;

      case clvTYPE_DOUBLE:
         size = 8;
         break;

      case clvTYPE_EVENT_T:
         size = 4;
         break;

      case clvTYPE_GEN_PACKED:
         size = 4;
         break;

      default:
         gcmASSERT(0);
         return 0;
      }

      return size;
}

cltELEMENT_TYPE
clGenElementTypeByByteSizeAndBaseType(
    IN cloCOMPILER Compiler,
    IN cltELEMENT_TYPE BaseElementType,
    IN gctBOOL IsPacked,
    IN gctUINT Size,
    OUT VIR_TypeId * VirPrimitiveType
    )
{
    cltELEMENT_TYPE elementType = BaseElementType;
    VIR_TypeId virPrimitiveType = VIR_INVALID_ID;

    if (clmIsElementTypeUnsigned(BaseElementType))
    {
        switch (Size)
        {
        case 1:
            elementType = clvTYPE_UCHAR;
            virPrimitiveType = VIR_TYPE_UINT8;
            break;

        case 2:
            elementType = clvTYPE_USHORT;
            virPrimitiveType = VIR_TYPE_UINT16;
            break;

        case 4:
            elementType = clvTYPE_UINT;
            virPrimitiveType = VIR_TYPE_UINT32;
            break;

        default:
            gcmASSERT(8);
            elementType = clvTYPE_ULONG;
            virPrimitiveType = VIR_TYPE_UINT64;
            break;
        }
    }
    else if (clmIsElementTypeSigned(BaseElementType))
    {
        switch (Size)
        {
        case 1:
            elementType = clvTYPE_CHAR;
            virPrimitiveType = VIR_TYPE_INT8;
            break;

        case 2:
            elementType = clvTYPE_SHORT;
            virPrimitiveType = VIR_TYPE_INT16;
            break;

        case 4:
            elementType = clvTYPE_INT;
            virPrimitiveType = VIR_TYPE_INT32;
            break;

        default:
            gcmASSERT(8);
            elementType = clvTYPE_LONG;
            virPrimitiveType = VIR_TYPE_INT64;
            break;
        }
    }
    else if (clmIsElementTypeFloating(BaseElementType))
    {
        switch (Size)
        {
        case 2:
            elementType = clvTYPE_HALF;
            virPrimitiveType = VIR_TYPE_FLOAT16;
            break;

        case 4:
            elementType = clvTYPE_FLOAT;
            virPrimitiveType = VIR_TYPE_FLOAT32;
            break;

        default:
            gcmASSERT(8);
            elementType = clvTYPE_DOUBLE;
            virPrimitiveType = VIR_TYPE_FLOAT64;
            break;
        }
    }

    if (VirPrimitiveType)
    {
        *VirPrimitiveType = virPrimitiveType;
    }

    return elementType;
}

gctSIZE_T
clsDECL_GetSize(
IN clsDECL * Decl
)
{
    gctSIZE_T    size = 0;
    clsNAME *    fieldName;

    gcmASSERT(Decl->dataType);

    if(clmDECL_IsPointerType(Decl)) {
           return 1;
    }
    switch (Decl->dataType->elementType) {
    case clvTYPE_VOID:
        size = 0;
        break;

    case clvTYPE_BOOL:
    case clvTYPE_INT:
    case clvTYPE_UINT:
    case clvTYPE_FLOAT:
    case clvTYPE_CHAR:
    case clvTYPE_UCHAR:
    case clvTYPE_SHORT:
    case clvTYPE_USHORT:
    case clvTYPE_DOUBLE:
    case clvTYPE_HALF:
        size = 1;
        break;

    case clvTYPE_BOOL_PACKED:
    case clvTYPE_CHAR_PACKED:
    case clvTYPE_UCHAR_PACKED:
    case clvTYPE_SHORT_PACKED:
    case clvTYPE_USHORT_PACKED:
    case clvTYPE_HALF_PACKED:
        size = 1;
        break;

    case clvTYPE_LONG:
    case clvTYPE_ULONG:
        size = 1;
        break;

    case clvTYPE_SAMPLER_T:
        size = 1;
        break;

    case clvTYPE_IMAGE2D_T:
    case clvTYPE_IMAGE3D_T:
    case clvTYPE_IMAGE1D_T:
    case clvTYPE_IMAGE2D_ARRAY_T:
    case clvTYPE_IMAGE1D_ARRAY_T:
    case clvTYPE_IMAGE1D_BUFFER_T:
    case clvTYPE_VIV_GENERIC_IMAGE_T:
        size = 1;
        break;

    case clvTYPE_STRUCT:
    case clvTYPE_UNION:
        {
           gctUINT localSize = 0;
           gctUINT curSize;

           gcmASSERT(Decl->dataType->u.fieldSpace);

           FOR_EACH_DLINK_NODE(&Decl->dataType->u.fieldSpace->names, clsNAME, fieldName) {
            gcmASSERT(fieldName->decl.dataType);
            curSize = clsDECL_GetSize(&fieldName->decl);
                if(Decl->dataType->elementType == clvTYPE_UNION) {
                   if(curSize > localSize) localSize = curSize;
                }
                else localSize += curSize;
           }
           size += localSize;
        }
        break;

    default:
        gcmASSERT(0);
        return 0;
    }

    if (clmDATA_TYPE_vectorSize_GET(Decl->dataType) > 0) {
        size *= clmDATA_TYPE_vectorSize_NOCHECK_GET(Decl->dataType);
    }
    else if (clmDATA_TYPE_matrixColumnCount_GET(Decl->dataType) > 0) {
        size *= clmDATA_TYPE_matrixRowCount_GET(Decl->dataType) * clmDATA_TYPE_matrixColumnCount_GET(Decl->dataType);
    }

    if (clmDECL_IsArray(Decl)) {
        gctUINT elementCount;
        clmGetArrayElementCount(Decl->array, 0, elementCount);
        size *= elementCount;
    }

    return size;
}

gctSIZE_T
clsDECL_GetElementSize(
IN clsDECL * Decl
)
{
    gctSIZE_T    size = 0;
    clsNAME *    fieldName;

    gcmASSERT(Decl->dataType);

    switch (Decl->dataType->elementType) {
    case clvTYPE_VOID:
        size = 0;
        break;

    case clvTYPE_BOOL:
    case clvTYPE_INT:
    case clvTYPE_UINT:
    case clvTYPE_FLOAT:
    case clvTYPE_CHAR:
    case clvTYPE_UCHAR:
    case clvTYPE_SHORT:
    case clvTYPE_USHORT:
    case clvTYPE_DOUBLE:
    case clvTYPE_HALF:
    case clvTYPE_LONG:
    case clvTYPE_ULONG:
        size = 1;
        break;

    case clvTYPE_BOOL_PACKED:
    case clvTYPE_CHAR_PACKED:
    case clvTYPE_UCHAR_PACKED:
    case clvTYPE_SHORT_PACKED:
    case clvTYPE_USHORT_PACKED:
    case clvTYPE_HALF_PACKED:
        size = 1;
        break;

    case clvTYPE_SAMPLER_T:
    case clvTYPE_IMAGE2D_T:
    case clvTYPE_IMAGE3D_T:
    case clvTYPE_IMAGE1D_T:
    case clvTYPE_IMAGE2D_ARRAY_T:
    case clvTYPE_IMAGE1D_ARRAY_T:
    case clvTYPE_IMAGE1D_BUFFER_T:
    case clvTYPE_VIV_GENERIC_IMAGE_T:
        size = 1;
        break;

    case clvTYPE_STRUCT:
    case clvTYPE_UNION:
        {
           gctUINT localSize = 0;
           gctUINT curSize;

           gcmASSERT(Decl->dataType->u.fieldSpace);

           FOR_EACH_DLINK_NODE(&Decl->dataType->u.fieldSpace->names, clsNAME, fieldName) {
            gcmASSERT(fieldName->decl.dataType);
            curSize = clsDECL_GetSize(&fieldName->decl);
                if(Decl->dataType->elementType == clvTYPE_UNION) {
                   if(curSize > localSize) localSize = curSize;
                }
                else localSize += curSize;
           }
           size += localSize;
        }
        break;

    default:
        gcmASSERT(0);
        return 0;
    }

    if (clmDATA_TYPE_vectorSize_GET(Decl->dataType) > 0) {
        size *= clmDATA_TYPE_vectorSize_NOCHECK_GET(Decl->dataType);
    }
    else if (clmDATA_TYPE_matrixColumnCount_GET(Decl->dataType) > 0) {
        size *= clmDATA_TYPE_matrixRowCount_GET(Decl->dataType) * clmDATA_TYPE_matrixColumnCount_GET(Decl->dataType);
    }

    return size;
}

gctSIZE_T
clsDECL_GetFieldOffset(
    IN clsDECL * StructDecl,
    IN clsNAME * FieldName
    )
{
    gctSIZE_T    offset = 0;
    clsNAME *    fieldName;

    gcmASSERT(StructDecl->dataType);
    gcmASSERT(clmDECL_IsStructOrUnion(StructDecl));
    gcmASSERT(FieldName);

    gcmASSERT(StructDecl->dataType->u.fieldSpace);

    if(!clmDECL_IsUnion(StructDecl)) {
       FOR_EACH_DLINK_NODE(&StructDecl->dataType->u.fieldSpace->names, clsNAME, fieldName)
       {
        if (fieldName == FieldName) break;

        gcmASSERT(fieldName->decl.dataType);
        offset += clsDECL_GetSize(&fieldName->decl);
       }
       gcmASSERT(fieldName == FieldName);
    }

    return offset;
}

static gctBOOL
_IsDeclAssignableAndComparable(
IN clsDECL *Decl
)
{
  clsNAME *fieldName;

  if(clmDECL_IsPointerType(Decl) ||
     clmIsElementTypeArithmetic(Decl->dataType->elementType) ||
     clmIsElementTypePacked(Decl->dataType->elementType)) {
    return gcvTRUE;
  }
  else switch (Decl->dataType->elementType) {
  case clvTYPE_VOID:

  case clvTYPE_SAMPLER_T:
  case clvTYPE_IMAGE2D_T:
  case clvTYPE_IMAGE3D_T:
  case clvTYPE_IMAGE1D_T:
  case clvTYPE_IMAGE2D_ARRAY_T:
  case clvTYPE_IMAGE1D_ARRAY_T:
  case clvTYPE_IMAGE1D_BUFFER_T:
  case clvTYPE_VIV_GENERIC_IMAGE_T:
    return gcvFALSE;

  case clvTYPE_STRUCT:
  case clvTYPE_UNION:
    gcmASSERT(Decl->dataType->u.fieldSpace);

    FOR_EACH_DLINK_NODE(&Decl->dataType->u.fieldSpace->names, clsNAME, fieldName) {
      gcmASSERT(fieldName->decl.dataType);
      if (!_IsDeclAssignableAndComparable(&fieldName->decl)) return gcvFALSE;
    }
    return gcvTRUE;

  case clvTYPE_EVENT_T:
    return gcvTRUE;

default:
    gcmASSERT(0);
    return gcvFALSE;
  }
}

gctBOOL
clsDECL_IsAssignableAndComparable(
IN clsDECL *Decl
)
{

   gcmASSERT(Decl);
   if (clmDECL_IsArray(Decl)) return gcvFALSE;
   return _IsDeclAssignableAndComparable(Decl);

}

gctBOOL
clsDECL_IsInitializable(
IN clsDECL *Decl
)
{
   gcmASSERT(Decl);
   return _IsDeclAssignableAndComparable(Decl);
}

gceSTATUS
clsNAME_SetVariableAddressed(
IN cloCOMPILER Compiler,
IN clsNAME *Name
)
{
   gcmASSERT(Name->type == clvVARIABLE_NAME ||
             Name->type == clvENUM_NAME ||
             Name->type == clvFIELD_NAME ||
             Name->type == clvPARAMETER_NAME);
   if(!clmDECL_IsPointerType(&Name->decl)) {
      Name->u.variableInfo.isAddressed = gcvTRUE;
      if(Name->decl.dataType->addrSpaceQualifier == clvQUALIFIER_LOCAL) {
         cloCOMPILER_SetNeedLocalMemory(Compiler);
      }
      else if(Name->decl.dataType->addrSpaceQualifier == clvQUALIFIER_CONSTANT) {
         cloCOMPILER_SetNeedConstantMemory(Compiler);
      }
      else {
         cloCOMPILER_SetNeedPrivateMemory(Compiler);
      }
   }
   return gcvSTATUS_OK;
}

static gctINT
_GetTypeQualifierTokenType(
IN cltQUALIFIER Qualifier
)
{
    gctINT   tokenType = T_EOF;

    switch (Qualifier) {
    case clvQUALIFIER_CONSTANT:
        tokenType = T_CONSTANT;
        break;
    case clvQUALIFIER_UNIFORM:
        tokenType = T_UNIFORM;
        break;
    case clvQUALIFIER_LOCAL:
        tokenType = T_LOCAL;
        break;
    case clvQUALIFIER_GLOBAL:
        tokenType = T_GLOBAL;
        break;
    case clvQUALIFIER_PRIVATE:
        tokenType = T_PRIVATE;
        break;

    case clvQUALIFIER_CONST:
        tokenType = T_CONST;
        break;

    case clvQUALIFIER_READ_ONLY:
        tokenType = T_READ_ONLY;
        break;

    case clvQUALIFIER_WRITE_ONLY:
        tokenType = T_WRITE_ONLY;
        break;

    default:
        break;

    }

    return tokenType;
}

gceSTATUS
clMergePtrDscrToDecl(
IN cloCOMPILER Compiler,
IN slsSLINK_LIST *PtrDscr,
IN clsDECL *Decl,
IN gctBOOL PtrDominant
)
{
    gceSTATUS status = gcvSTATUS_OK;

    if(!slmSLINK_LIST_IsEmpty(PtrDscr)) {
        if(slmSLINK_LIST_IsEmpty(Decl->ptrDscr) && Decl->dataType) {
            gctPOINTER pointer;
            clsTYPE_QUALIFIER *typeQualifier;
            gctINT tokenType;

            tokenType = _GetTypeQualifierTokenType(Decl->dataType->addrSpaceQualifier);
            if(tokenType != T_EOF) {
                status = cloCOMPILER_Allocate(Compiler,
                                              (gctSIZE_T)sizeof(clsTYPE_QUALIFIER),
                                              (gctPOINTER *) &pointer);
                if(gcmIS_ERROR(status)) return status;
                typeQualifier = pointer;

                typeQualifier->type = tokenType;
                typeQualifier->qualifier = Decl->dataType->addrSpaceQualifier;
                slmSLINK_LIST_InsertFirst(PtrDscr, &typeQualifier->node);

                gcmVERIFY_OK(cloCOMPILER_CloneDataType(Compiler,
                                                       Decl->dataType->accessQualifier,
                                                       clvQUALIFIER_NONE,
                                                       Decl->dataType,
                                                       &Decl->dataType));
            }
        }

        slmSLINK_LIST_Append(PtrDscr, Decl->ptrDscr);
        if(PtrDominant && clmDECL_IsArray(Decl) && PtrDscr) {
            Decl->ptrDominant = gcvTRUE;
        }
    }
    return status;
}

/* Note: if any fields of a built-in function name or parameter name are changed during compilation,
         need to reset them here.*/
gceSTATUS
clsNAME_Reset(
    IN cloCOMPILER Compiler,
    IN clsNAME * Name
)
{
    gceSTATUS status= gcvSTATUS_OK;
    gctUINT builtinSpecificValue;
    gcmHEADER();

    do
    {
        switch (Name->type)
        {
        case clvPARAMETER_NAME:
            builtinSpecificValue = Name->u.variableInfo.builtinSpecific.value;
            gcoOS_ZeroMemory(&Name->u.variableInfo, sizeof(clsVARIABLE_INFO));
            Name->u.variableInfo.builtinSpecific.value = builtinSpecificValue;
            slmSLINK_LIST_Initialize(Name->u.variableInfo.samplers);
            gcoOS_ZeroMemory(&Name->context, sizeof(clsNAME_CONTEXT));
            clmNAME_VariableMemoryOffset_SET(Name, -1);
            break;

        case clvFUNC_NAME:
            Name->u.funcInfo.refCount = 0;
            Name->u.funcInfo.funcBody = gcvNULL;
            gcoOS_ZeroMemory(&Name->context, sizeof(clsNAME_CONTEXT));
            break;

        default:
            break;
        }
    }
    while (gcvFALSE);

    gcmFOOTER();
    return status;
}

static gctBOOL
_IsParamAddress(
    IN cloCOMPILER      Compiler,
    IN clsNAME_SPACE*   MySpace,
    IN clsNAME*         Name
    )
{
    if (!Name->decl.dataType)
    {
        return gcvFALSE;
    }

    if (clmDECL_IsAggregateTypeOverRegLimit(&Name->decl)
        &&
        (!clmDECL_IsStructOrUnion(&Name->decl) || !Name->decl.dataType->u.fieldSpace->scopeName->isBuiltin))
    {
        return gcvTRUE;
    }

    if (!(cloCOMPILER_ExtensionEnabled(Compiler, clvEXTENSION_VIV_VX) || gcmOPT_oclPassKernelStructArgByValue())
        &&
        clmDECL_IsAggregateType(&Name->decl)
        &&
        (MySpace && MySpace->scopeName && MySpace->scopeName->type == clvKERNEL_FUNC_NAME))
    {
        return gcvTRUE;
    }

    return gcvFALSE;
}

/* Name and Name space. */
static gceSTATUS
_clsNAME_Construct(
IN cloCOMPILER Compiler,
IN clsNAME_SPACE * MySpace,
IN gctUINT LineNo,
IN gctUINT StringNo,
IN cleNAME_TYPE Type,
IN clsDECL *Decl,
IN cltPOOL_STRING Symbol,
IN slsSLINK_LIST *PtrDscr,
IN gctBOOL IsBuiltin,
IN cleEXTENSION Extension,
OUT clsNAME **Name
)
{
    gceSTATUS status;
    clsNAME *name;

    gcmHEADER_ARG("Compiler=0x%x MySpace=0x%x LineNo=%u StringNo=%u Type=%d "
              "Decl=0x%x Symbol=0x%x IsBuiltIn=%d Extension=%d",
              Compiler, MySpace, LineNo, StringNo, Type, Decl, Symbol,
              IsBuiltin, Extension);
    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    gcmVERIFY_ARGUMENT(MySpace);
    gcmVERIFY_ARGUMENT(Symbol);
    gcmVERIFY_ARGUMENT(Name);

    do {
        gctPOINTER pointer;
#if (__USE_VSC_MP__ || __USE_VSC_BMS__)
        status = cloCOMPILER_ZeroMemoryAllocate(Compiler,
                                                (gctSIZE_T)sizeof(clsNAME),
                                                (gctPOINTER *) &pointer);
        if (gcmIS_ERROR(status)) break;
#else
        status = cloCOMPILER_AllocateName(Compiler,
                                          (gctPOINTER *) &pointer);
        if (gcmIS_ERROR(status)) break;
#endif

        name = pointer;

        name->mySpace    = MySpace;
        name->fileNo    = 0;
        name->lineNo    = LineNo;
        name->stringNo    = StringNo;
        name->type    = Type;
        if(Decl) {
             name->decl = *Decl;
        }
        else {
             clmDECL_Initialize(&name->decl, gcvNULL, (clsARRAY *)0, gcvNULL, gcvFALSE, clvSTORAGE_QUALIFIER_NONE);
        }

        gcmONERROR(clMergePtrDscrToDecl(Compiler,
                                        PtrDscr,
                                        &name->decl,
                                        PtrDscr != gcvNULL));

        name->derivedType  = gcvNULL;
        name->symbol       = Symbol;
        name->isBuiltin    = IsBuiltin;
        name->extension    = Extension;
        name->die          = VSC_DI_INVALIDE_DIE;

        switch (Type) {
        case clvVARIABLE_NAME:
        case clvENUM_NAME:
        case clvFIELD_NAME:
            slmSLINK_LIST_Initialize(name->u.variableInfo.samplers);
            name->u.variableInfo.u.constant    = gcvNULL;
            name->u.variableInfo.specifiedAttr = 0;
            name->u.variableInfo.hostEndian = gcvFALSE;
            name->u.variableInfo.isAddressed = gcvFALSE;
            name->u.variableInfo.builtinSpecific.s.isConvertibleType = gcvFALSE;
            name->u.variableInfo.isDirty = gcvFALSE;
            name->u.variableInfo.alias = gcvNULL;
            name->u.variableInfo.aliasOffset = 0;
            name->u.variableInfo.padding = 0;
            name->u.variableInfo.builtinSpecific.s.variableType = 0;
            name->u.variableInfo.builtinSpecific.s.hasGenType = gcvFALSE;
            name->u.variableInfo.allocated = gcvFALSE;
            name->u.variableInfo.inInterfaceBlock = gcvFALSE;
            name->u.variableInfo.isInitializedWithExtendedVectorConstant = gcvFALSE;
            name->u.variableInfo.indirectlyAddressed = gcvFALSE;
            break;

        case clvPARAMETER_NAME:
            slmSLINK_LIST_Initialize(name->u.variableInfo.samplers);
            name->u.variableInfo.u.aliasName = gcvNULL;
            name->u.variableInfo.specifiedAttr = 0;
            name->u.variableInfo.hostEndian = gcvFALSE;
            name->u.variableInfo.builtinSpecific.s.isConvertibleType = gcvFALSE;
            name->u.variableInfo.isDirty = gcvFALSE;
            name->u.variableInfo.alias = gcvNULL;
            name->u.variableInfo.aliasOffset = 0;
            name->u.variableInfo.builtinSpecific.s.variableType = 0;
            name->u.variableInfo.builtinSpecific.s.hasGenType = gcvFALSE;
            name->u.variableInfo.allocated = gcvFALSE;
            name->u.variableInfo.inInterfaceBlock = gcvFALSE;
            if (_IsParamAddress(Compiler, MySpace, name))
            {
                clsNAME_SetVariableAddressed(Compiler, name);
            }
            else
            {
                name->u.variableInfo.isAddressed = gcvFALSE;
            }
            break;

        case clvFUNC_NAME:
        case clvKERNEL_FUNC_NAME:
            name->u.funcInfo.localSpace    = gcvNULL;
            name->u.funcInfo.isFuncDef    = gcvFALSE;
            name->u.funcInfo.isIntrinsicCall = gcvFALSE;
            name->u.funcInfo.intrinsicKind = gceINTRIN_NONE;
            name->u.funcInfo.mangledName = gcvNULL;
            name->u.funcInfo.refCount       = 0;
            name->u.funcInfo.hasWriteArg       = gcvFALSE;
            name->u.funcInfo.passArgByRef   = gcvFALSE;
            name->u.funcInfo.isInline    = gcvFALSE;
            name->u.funcInfo.funcBody    = gcvNULL;
            name->u.funcInfo.hasVarArg   = gcvFALSE;
            name->u.funcInfo.hasGenType   = gcvFALSE;
            name->u.funcInfo.needLocalMemory = gcvFALSE;
            name->u.funcInfo.localMemorySize = 0;

            /* Those are the optional function attribute qualifiers. */
            name->u.funcInfo.attrQualifier.attrFlags = clvATTR_NONE;
            name->u.funcInfo.attrQualifier.vecTypeHint = T_INT;
            name->u.funcInfo.attrQualifier.reqdWorkGroupSize[0] =
            name->u.funcInfo.attrQualifier.reqdWorkGroupSize[1] =
            name->u.funcInfo.attrQualifier.reqdWorkGroupSize[2] = 0;
            name->u.funcInfo.attrQualifier.workGroupSizeHint[0] =
            name->u.funcInfo.attrQualifier.workGroupSizeHint[1] =
            name->u.funcInfo.attrQualifier.workGroupSizeHint[2] = 0;
            name->u.funcInfo.attrQualifier.kernelScaleHint[0] =
            name->u.funcInfo.attrQualifier.kernelScaleHint[1] =
            name->u.funcInfo.attrQualifier.kernelScaleHint[2] = 1;
            break;

        case clvLABEL_NAME:
            name->u.labelInfo.label = gcvNULL;
            name->u.labelInfo.isReferenced = gcvFALSE;
            break;

        case clvSTRUCT_NAME:
        case clvTYPE_NAME:
        case clvUNION_NAME:
        case clvENUM_TAG_NAME:
            clmNAME_InitializeTypeInfo(name);
            break;

        default:
            break;
        }

        name->context.packed    = gcvFALSE;
        name->context.alignment = 0;
        switch(Type) {
        case clvTYPE_NAME:
            (void)gcoOS_ZeroMemory((gctPOINTER)&name->context.u.typeDef, sizeof(clsDECL));
            break;

        case clvENUM_TAG_NAME:
            slmSLINK_LIST_Initialize(name->context.u.enumerator);
            name->decl.dataType->u.enumerator = name;
            break;

        default:
            name->context.u.variable.logicalRegCount = 0;
            name->context.u.variable.logicalRegs = gcvNULL;
            clmNAME_VariableMemoryOffset_SET(name, -1);
            name->context.u.variable.isKernel = gcvFALSE;
            name->context.u.variable.u.function = gcvNULL;
            name->context.u.variable.u.kernelFunction = gcvNULL;
            break;
        }
        *Name = name;
        gcmFOOTER_NO();
        return gcvSTATUS_OK;
    } while (gcvFALSE);

OnError:
    *Name = gcvNULL;
    gcmFOOTER();
    return status;
}

static void
_clFreePtrDscr(
IN cloCOMPILER Compiler,
IN slsSLINK_LIST *ptrDscr
)
{
    clsTYPE_QUALIFIER *typeQualifier;

    while (!slmSLINK_LIST_IsEmpty(ptrDscr)) {
        slmSLINK_LIST_DetachFirst(ptrDscr, clsTYPE_QUALIFIER, &typeQualifier);

        gcmVERIFY_OK(cloCOMPILER_Free(Compiler, typeQualifier));
    }
}

gceSTATUS
clsNAME_Destroy(
IN cloCOMPILER Compiler,
IN clsNAME *Name
)
{
    gcmHEADER_ARG("Compiler=0x%x Name=0x%x", Compiler, Name);
    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    gcmVERIFY_ARGUMENT(Name);

    clsNAME_FreeLogicalRegs(Compiler, Name);

    _clFreePtrDscr(Compiler, Name->decl.ptrDscr);

    /* do not free name here */
#if (__USE_VSC_MP__ || __USE_VSC_BMS__)
    gcmVERIFY_OK(cloCOMPILER_Free(Compiler, Name));
#endif

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

static gctCONST_STRING
_GetNameTypeName(
IN cleNAME_TYPE NameType
)
{
    switch (NameType) {
    case clvVARIABLE_NAME:    return "variable";
    case clvPARAMETER_NAME:   return "parameter";
    case clvFUNC_NAME:        return "function";
    case clvKERNEL_FUNC_NAME: return "__kernel";
    case clvSTRUCT_NAME:      return "struct";
    case clvUNION_NAME:       return "union";
    case clvFIELD_NAME:       return "field";
    case clvLABEL_NAME:       return "label";
    case clvTYPE_NAME:        return "typedef";
    case clvENUM_NAME:        return "enumerator";
    case clvENUM_TAG_NAME:    return "enum";

    default:
        gcmASSERT(0);
        return "invalid";
    }
}

gceSTATUS
cloNAME_BindFuncBody(
IN cloCOMPILER Compiler,
IN clsNAME * FuncName,
IN cloIR_SET FuncBody
)
{
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    gcmASSERT(FuncName);
    gcmASSERT(FuncName->type == clvFUNC_NAME ||
              FuncName->type == clvKERNEL_FUNC_NAME);
    clmVERIFY_IR_OBJECT(FuncBody, clvIR_SET);

    FuncName->u.funcInfo.funcBody    = FuncBody;
    FuncBody->funcName    = FuncName;

    return gcvSTATUS_OK;
}

gceSTATUS
cloNAME_GetParamCount(
IN cloCOMPILER Compiler,
IN clsNAME * FuncName,
OUT gctUINT * ParamCount
)
{
    gctUINT        count = 0;
    clsNAME *    paramName;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    gcmASSERT(FuncName);
    gcmASSERT(FuncName->type == clvFUNC_NAME ||
              FuncName->type == clvKERNEL_FUNC_NAME);
    gcmASSERT(FuncName->u.funcInfo.localSpace);
    gcmASSERT(ParamCount);

    FOR_EACH_DLINK_NODE(&FuncName->u.funcInfo.localSpace->names, clsNAME, paramName) {
        if (paramName->type != clvPARAMETER_NAME) break;
        count++;
    }

    *ParamCount = count;
    return gcvSTATUS_OK;
}

static gctBOOL
_IsSameFuncName(
    IN cloCOMPILER Compiler,
    IN clsNAME * FuncName1,
    IN clsNAME * FuncName2,
    OUT gctBOOL * AreAllParamQualifiersEqual
    )
{
    clsNAME *paramName1;
    clsNAME *paramName2;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    gcmASSERT(FuncName1);
    gcmASSERT(FuncName1->type == clvFUNC_NAME ||
              FuncName1->type == clvKERNEL_FUNC_NAME);
    gcmASSERT(FuncName1->u.funcInfo.localSpace);
    gcmASSERT(FuncName2);
    gcmASSERT(FuncName2->type == clvFUNC_NAME ||
              FuncName2->type == clvKERNEL_FUNC_NAME);
    gcmASSERT(FuncName2->u.funcInfo.localSpace);

    if (FuncName1->symbol != FuncName2->symbol) return gcvFALSE;

    if (AreAllParamQualifiersEqual != gcvNULL) *AreAllParamQualifiersEqual = gcvTRUE;

    for (paramName1 = (clsNAME *)FuncName1->u.funcInfo.localSpace->names.next,
            paramName2 = (clsNAME *)FuncName2->u.funcInfo.localSpace->names.next;
        (slsDLINK_NODE *)paramName1 != &FuncName1->u.funcInfo.localSpace->names
            && (slsDLINK_NODE *)paramName2 != &FuncName2->u.funcInfo.localSpace->names;
        paramName1 = (clsNAME *)((slsDLINK_NODE *)paramName1)->next,
            paramName2 = (clsNAME *)((slsDLINK_NODE *)paramName2)->next)
    {
        if (paramName1->type != clvPARAMETER_NAME || paramName2->type != clvPARAMETER_NAME) break;

        if (!clsDECL_IsEqual(&paramName1->decl, &paramName2->decl)) return gcvFALSE;

        if (AreAllParamQualifiersEqual != gcvNULL
            && paramName1->decl.dataType->accessQualifier != paramName2->decl.dataType->accessQualifier
            && paramName1->decl.dataType->addrSpaceQualifier != paramName2->decl.dataType->addrSpaceQualifier) {
            *AreAllParamQualifiersEqual = gcvFALSE;
        }
    }

    if ((slsDLINK_NODE *)paramName1 != &FuncName1->u.funcInfo.localSpace->names
        && paramName1->type == clvPARAMETER_NAME)
    {
        return gcvFALSE;
    }

    if ((slsDLINK_NODE *)paramName2 != &FuncName2->u.funcInfo.localSpace->names
        && paramName2->type == clvPARAMETER_NAME)
    {
        return gcvFALSE;
    }

    return gcvTRUE;
}

gceSTATUS
clsNAME_BindAliasParamNames(
    IN cloCOMPILER Compiler,
    IN OUT clsNAME * FuncDefName,
    IN clsNAME * FuncDeclName
    )
{
    clsNAME *paramName;
    clsNAME *aliasParamName;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    gcmASSERT(FuncDefName);
    gcmASSERT(FuncDefName->type == clvFUNC_NAME ||
              FuncDefName->type == clvKERNEL_FUNC_NAME);
    gcmASSERT(FuncDefName->u.funcInfo.localSpace);
    gcmASSERT(FuncDeclName);
    gcmASSERT(FuncDeclName->type == clvFUNC_NAME ||
              FuncDeclName->type == clvKERNEL_FUNC_NAME);
    gcmASSERT(FuncDeclName->u.funcInfo.localSpace);

    gcmASSERT(_IsSameFuncName(Compiler, FuncDefName, FuncDeclName, gcvNULL));

    for (paramName = (clsNAME *)FuncDefName->u.funcInfo.localSpace->names.next,
            aliasParamName = (clsNAME *)FuncDeclName->u.funcInfo.localSpace->names.next;
        (slsDLINK_NODE *)paramName != &FuncDefName->u.funcInfo.localSpace->names
            && (slsDLINK_NODE *)aliasParamName != &FuncDeclName->u.funcInfo.localSpace->names;
        paramName = (clsNAME *)((slsDLINK_NODE *)paramName)->next,
            aliasParamName = (clsNAME *)((slsDLINK_NODE *)aliasParamName)->next)
    {
        if (paramName->type != clvPARAMETER_NAME
            || aliasParamName->type != clvPARAMETER_NAME) break;

        paramName->u.variableInfo.u.aliasName = aliasParamName;
    }

    return gcvSTATUS_OK;
}

gceSTATUS
clsNAME_Dump(
IN cloCOMPILER Compiler,
IN clsNAME * Name
)
{
    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    gcmASSERT(Name);

    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                      clvDUMP_IR,
                      "<NAME this=\"0x%x\" mySpace=\"0x%x\" line=\"%d\" string=\"%d\""
                      " type=\"%s\" dataType=\"0x%x\" symbol=\"%s\" isBuiltin=\"%s\"",
                      Name,
                      Name->mySpace,
                      Name->lineNo,
                      Name->stringNo,
                      _GetNameTypeName(Name->type),
                      Name->decl.dataType,
                      Name->symbol,
                      (Name->isBuiltin)? "true" : "false"));

    if (Name->die != VSC_DI_INVALIDE_DIE)
    {
        cloCOMPILER_DumpDIE(Compiler, clvDUMP_IR, Name->die);
    }

    switch (Name->type) {
    case clvVARIABLE_NAME:
        gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                          clvDUMP_IR,
                          " constant=\"0x%x\" />",
                          Name->u.variableInfo.u.constant));
        break;

    case clvENUM_NAME:
        gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                          clvDUMP_IR,
                          " enumerator=\"0x%x\" />",
                          Name->u.variableInfo.u.constant));
        break;

    case clvPARAMETER_NAME:
        gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                        clvDUMP_IR,
                        " aliasName=\"0x%x\" />",
                        Name->u.variableInfo.u.aliasName));
        break;

    case clvFUNC_NAME:
    case clvKERNEL_FUNC_NAME:
        gcmASSERT(Name->u.funcInfo.localSpace);

        /* gcmVERIFY_OK(clsNAME_SPACE_Dump(Compiler, Name->localSpace)); */

        gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                          clvDUMP_IR,
                          " localSpace=\"0x%x\" isFuncDef=\"%s\" funcBody=\"0x%x\" />",
                          Name->u.funcInfo.localSpace,
                          (Name->u.funcInfo.isFuncDef)? "true" : "false",
                          Name->u.funcInfo.funcBody));
        break;

    default:
        gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                        clvDUMP_IR,
                        " />"));
    }
    return gcvSTATUS_OK;
}

gceSTATUS
clsNAME_SPACE_Construct(
IN cloCOMPILER Compiler,
IN clsNAME_SPACE * Parent,
OUT clsNAME_SPACE ** NameSpace
)
{
    gceSTATUS    status;
    clsNAME_SPACE *    nameSpace;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    gcmASSERT(NameSpace);

    do {
        status = cloCOMPILER_ZeroMemoryAllocate(Compiler,
                        (gctSIZE_T)sizeof(clsNAME_SPACE),
                        (gctPOINTER *) &nameSpace);
        if (gcmIS_ERROR(status)) break;

        nameSpace->parent = Parent;
        nameSpace->scopeName = gcvNULL;
        nameSpace->die = VSC_DI_INVALIDE_DIE;
        slsDLINK_LIST_Initialize(&nameSpace->names);
        slsDLINK_LIST_Initialize(&nameSpace->subSpaces);

        if (Parent != gcvNULL) slsDLINK_LIST_InsertLast(&Parent->subSpaces, &nameSpace->node);
        *NameSpace = nameSpace;
        return gcvSTATUS_OK;
    } while (gcvFALSE);

    *NameSpace = gcvNULL;
    return status;
}

gceSTATUS
clsNAME_SPACE_Destroy(
IN cloCOMPILER Compiler,
IN clsNAME_SPACE * NameSpace
)
{
    clsNAME *name;
    clsNAME_SPACE *    subSpace;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    gcmASSERT(NameSpace);

    /* Destroy all names */
    while (!slsDLINK_LIST_IsEmpty(&NameSpace->names)) {
        slsDLINK_LIST_DetachFirst(&NameSpace->names, clsNAME, &name);

        gcmVERIFY_OK(clsNAME_Destroy(Compiler, name));
    }

    /* Destroy all sub name spaces */
    while (!slsDLINK_LIST_IsEmpty(&NameSpace->subSpaces)) {
        slsDLINK_LIST_DetachFirst(&NameSpace->subSpaces, clsNAME_SPACE, &subSpace);

        gcmVERIFY_OK(clsNAME_SPACE_Destroy(Compiler, subSpace));
    }

    gcmVERIFY_OK(cloCOMPILER_Free(Compiler, NameSpace));
    return gcvSTATUS_OK;
}

gceSTATUS
clsNAME_SPACE_Dump(
IN cloCOMPILER Compiler,
IN clsNAME_SPACE * NameSpace
)
{
    clsNAME *name;
    clsNAME_SPACE *    subSpace;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    gcmASSERT(NameSpace);

    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                      clvDUMP_IR,
                      "<NAME_SPACE this=\"0x%x\" parent=\"0x%x\">",
                      NameSpace,
                      NameSpace->parent));

    if (NameSpace->die != VSC_DI_INVALIDE_DIE)
    {
        cloCOMPILER_DumpDIE(
            Compiler,
            clvDUMP_IR,
            NameSpace->die
            );
    }

    /* Dump all names */
    FOR_EACH_DLINK_NODE(&NameSpace->names, clsNAME, name) {
        gcmVERIFY_OK(clsNAME_Dump(Compiler, name));
    }

    /* Dump all sub name spaces */
    FOR_EACH_DLINK_NODE(&NameSpace->subSpaces, clsNAME_SPACE, subSpace) {
        gcmVERIFY_OK(clsNAME_SPACE_Dump(Compiler, subSpace));
    }

    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                      clvDUMP_IR,
                      "</NAME_SPACE>"));
    return gcvSTATUS_OK;
}

gceSTATUS
clsNAME_SPACE_Search(
IN cloCOMPILER Compiler,
IN clsNAME_SPACE * NameSpace,
IN cltPOOL_STRING Symbol,
IN gctBOOL Recursive,
OUT clsNAME ** Name
)
{
    clsNAME *name;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    gcmASSERT(NameSpace);

    FOR_EACH_DLINK_NODE(&NameSpace->names, clsNAME, name) {
        if (name->symbol == Symbol) {
            if (name->extension != clvEXTENSION_NONE) {
               if (!cloCOMPILER_ExtensionEnabled(Compiler,
                                 name->extension)) continue;
            }
            *Name = name;
            return gcvSTATUS_OK;
        }
    }

    if (Recursive && NameSpace->parent != gcvNULL) {
        return clsNAME_SPACE_Search(Compiler,
                        NameSpace->parent,
                        Symbol,
                        Recursive,
                        Name);
    }
    else {
        *Name = gcvNULL;
        return gcvSTATUS_NAME_NOT_FOUND;
    }
}

gceSTATUS
clsNAME_SPACE_SearchFieldSpaceWithUnnamedField(
IN cloCOMPILER Compiler,
IN clsNAME_SPACE * FieldSpace,
IN cltPOOL_STRING Symbol,
IN gctBOOL Recursive,
OUT clsNAME ** Name
)
{
    clsNAME *name;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    gcmASSERT(FieldSpace);

    FOR_EACH_DLINK_NODE(&FieldSpace->names, clsNAME, name) {
        if (name->symbol == Symbol) {
            if (name->extension != clvEXTENSION_NONE) {
               if (!cloCOMPILER_ExtensionEnabled(Compiler,
                                 name->extension)) continue;
            }
            *Name = name;
            return gcvSTATUS_OK;
        }
        else if(Recursive && name->symbol[0] == '\0') { /* unnamed field */
            if (clmDATA_TYPE_IsStructOrUnion(name->decl.dataType)) {
                gceSTATUS status;
                clsNAME *fieldName;
                status = clsNAME_SPACE_SearchFieldSpaceWithUnnamedField(Compiler,
                                                                        name->decl.dataType->u.fieldSpace,
                                                                        Symbol,
                                                                        gcvFALSE,
                                                                        &fieldName);
                if(status == gcvSTATUS_NAME_NOT_FOUND) continue;
                *Name = fieldName;
                return gcvSTATUS_OK;
            }
        }
    }

    *Name = gcvNULL;
    return gcvSTATUS_NAME_NOT_FOUND;
}

gceSTATUS
clsNAME_SPACE_CheckNewFuncName(
IN cloCOMPILER Compiler,
IN clsNAME_SPACE * NameSpace,
IN clsNAME * FuncName,
OUT clsNAME ** FirstFuncName
)
{
    gctUINT    paramCount;
    clsNAME    *name;
    clsNAME    *paramName;
    gctBOOL    areAllParamQualifiersEqual;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    gcmASSERT(NameSpace);
    gcmASSERT(FuncName);

    gcmVERIFY_OK(cloNAME_GetParamCount(Compiler,
                                       FuncName,
                                       &paramCount));
    if (paramCount == 1)
    {
        paramName = slsDLINK_LIST_First(&FuncName->u.funcInfo.localSpace->names, clsNAME);
        if(clmDECL_IsVoid(&paramName->decl))
        {
            slsDLINK_LIST_DetachFirst(&FuncName->u.funcInfo.localSpace->names, clsNAME, &paramName);
            gcmVERIFY_OK(clsNAME_Destroy(Compiler, paramName));
            paramCount = 0;
        }
    }

    if(gcmIS_SUCCESS(gcoOS_StrCmp(FuncName->symbol, "main"))) {
        if (!clmDECL_IsVoid(&FuncName->decl)) {
            gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                            FuncName->lineNo,
                                            FuncName->stringNo,
                                            clvREPORT_ERROR,
                                            "The return type of the kernel function"
                                            " %s must be 'void'",
                                            FuncName->symbol));
            return gcvSTATUS_INVALID_ARGUMENT;
        }

        if (paramCount != 0) {
            gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                            FuncName->lineNo,
                                            FuncName->stringNo,
                                            clvREPORT_ERROR,
                                            "The function 'main' must have no parameters"));
            return gcvSTATUS_INVALID_ARGUMENT;
        }

        if (FirstFuncName != gcvNULL) *FirstFuncName = FuncName;
        return gcvSTATUS_OK;
    }
    if (FuncName->type == clvKERNEL_FUNC_NAME) {
        if (!clmDECL_IsVoid(&FuncName->decl)) {
            gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                            FuncName->lineNo,
                                            FuncName->stringNo,
                                            clvREPORT_ERROR,
                                            "The return type of the kernel function"
                                            " %s must be 'void'",
                                            FuncName->symbol));
            return gcvSTATUS_INVALID_ARGUMENT;
        }
    }
    if (FirstFuncName != gcvNULL) *FirstFuncName = gcvNULL;

    FOR_EACH_DLINK_NODE(&NameSpace->names, clsNAME, name) {
        if (name != FuncName) {
            if (((name->type == clvFUNC_NAME) || (name->type == clvKERNEL_FUNC_NAME))
                && _IsSameFuncName(Compiler, name, FuncName, &areAllParamQualifiersEqual)) {
               if (name->u.funcInfo.isFuncDef && FuncName->u.funcInfo.isFuncDef) {
                   gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                                   FuncName->lineNo,
                                                   FuncName->stringNo,
                                                   clvREPORT_ERROR,
                                                   "%s function: '%s'",
                                                   "redefined",
                                                   FuncName->symbol));
                   return gcvSTATUS_INVALID_ARGUMENT;
               }
               else if (!clsDECL_IsEqual(&name->decl, &FuncName->decl)) {
                   gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                                   FuncName->lineNo,
                                                   FuncName->stringNo,
                                                   clvREPORT_ERROR,
                                                   "the inconsistent return type of"
                                                   " function: '%s'",
                                                   FuncName->symbol));
                   return gcvSTATUS_INVALID_ARGUMENT;
               }

               if (!areAllParamQualifiersEqual) {
                   gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                                   FuncName->lineNo,
                                                   FuncName->stringNo,
                                                   clvREPORT_ERROR,
                                                   "the function: '%s' has different"
                                                   " parameter qualifier(s)",
                                                   FuncName->symbol));
                   return gcvSTATUS_INVALID_ARGUMENT;
               }
               if (FirstFuncName != gcvNULL && *FirstFuncName == gcvNULL) {
                   *FirstFuncName = name;
               }
           }
      }
      else {
           if (FirstFuncName != gcvNULL && *FirstFuncName == gcvNULL) {
              *FirstFuncName = name;
           }
      }
    }
    return gcvSTATUS_OK;
}


/* Table of resulting type of vector component selection  --
   padded with void for zeroth column and row for efficient access*/

typedef struct _clsVecCompSelType {
   cltELEMENT_TYPE elementType;
   gctINT compSel[cldMAX_VECTOR_COMPONENT + 1];
}
clsVecCompSelType;


static  clsVecCompSelType _BuiltinVectorTypes[] =
{
  {clvTYPE_VOID, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}},
  {clvTYPE_BOOL, {T_BOOL, T_BOOL, T_BOOL2, T_BOOL3, T_BOOL4, 0, 0, 0, T_BOOL8, 0, 0, 0, 0, 0, 0, 0, T_BOOL16}},
  {clvTYPE_CHAR, {T_CHAR, T_CHAR, T_CHAR2, T_CHAR3, T_CHAR4, 0, 0, 0, T_CHAR8, 0, 0, 0, 0, 0, 0, 0, T_CHAR16}},
  {clvTYPE_UCHAR, {T_UCHAR, T_UCHAR, T_UCHAR2, T_UCHAR3, T_UCHAR4, 0, 0, 0, T_UCHAR8, 0, 0, 0, 0, 0, 0, 0, T_UCHAR16}},
  {clvTYPE_SHORT, {T_SHORT, T_SHORT, T_SHORT2, T_SHORT3, T_SHORT4, 0, 0, 0, T_SHORT8, 0, 0, 0, 0, 0, 0, 0, T_SHORT16}},
  {clvTYPE_USHORT, {T_USHORT, T_USHORT, T_USHORT2, T_USHORT3, T_USHORT4, 0, 0, 0, T_USHORT8, 0, 0, 0, 0, 0, 0, 0, T_USHORT16}},
  {clvTYPE_INT, {T_INT, T_INT, T_INT2, T_INT3, T_INT4, 0, 0, 0, T_INT8, 0, 0, 0, 0, 0, 0, 0, T_INT16}},
  {clvTYPE_UINT, {T_UINT, T_UINT, T_UINT2, T_UINT3, T_UINT4, 0, 0, 0, T_UINT8, 0, 0, 0, 0, 0, 0, 0, T_UINT16}},
  {clvTYPE_LONG, {T_LONG, T_LONG, T_LONG2, T_LONG3, T_LONG4, 0, 0, 0, T_LONG8, 0, 0, 0, 0, 0, 0, 0, T_LONG16}},
  {clvTYPE_ULONG, {T_ULONG, T_ULONG, T_ULONG2, T_ULONG3, T_ULONG4, 0, 0, 0, T_ULONG8, 0, 0, 0, 0, 0, 0, 0, T_ULONG16}},
  {clvTYPE_HALF, {T_HALF, T_HALF, T_HALF2, T_HALF3, T_HALF4, 0, 0, 0, T_HALF8, 0, 0, 0, 0, 0, 0, 0, T_HALF16}},
  {clvTYPE_FLOAT, {T_FLOAT, T_FLOAT, T_FLOAT2, T_FLOAT3, T_FLOAT4, 0, 0, 0, T_FLOAT8, 0, 0, 0, 0, 0, 0, 0, T_FLOAT16}},
  {clvTYPE_DOUBLE, {T_DOUBLE, T_DOUBLE, T_DOUBLE2, T_DOUBLE3, T_DOUBLE4, 0, 0, 0, T_DOUBLE8, 0, 0, 0, 0, 0, 0, 0, T_DOUBLE16}},
};

static const gctUINT  _BuiltinVectorTypeCount = sizeof(_BuiltinVectorTypes) / sizeof(clsVecCompSelType);

static  clsVecCompSelType _BuiltinPackedVectorTypes[] =
{
  {clvTYPE_BOOL_PACKED, {T_BOOL, T_BOOL_PACKED, T_BOOL2_PACKED, T_BOOL3_PACKED, T_BOOL4_PACKED, 0, 0, 0, T_BOOL8_PACKED, 0, 0, 0, 0, 0, 0, 0, T_BOOL16_PACKED}},
  {clvTYPE_CHAR_PACKED, {T_CHAR, T_CHAR_PACKED, T_CHAR2_PACKED, T_CHAR3_PACKED, T_CHAR4_PACKED, 0, 0, 0, T_CHAR8_PACKED, 0, 0, 0, 0, 0, 0, 0, T_CHAR16_PACKED}},
  {clvTYPE_UCHAR_PACKED, {T_UCHAR, T_UCHAR_PACKED, T_UCHAR2_PACKED, T_UCHAR3_PACKED, T_UCHAR4_PACKED, 0, 0, 0, T_UCHAR8_PACKED, 0, 0, 0, 0, 0, 0, 0, T_UCHAR16_PACKED}},
  {clvTYPE_SHORT_PACKED, {T_SHORT, T_SHORT_PACKED, T_SHORT2_PACKED, T_SHORT3_PACKED, T_SHORT4_PACKED, 0, 0, 0, T_SHORT8_PACKED, 0, 0, 0, 0, 0, 0, 0, T_SHORT16_PACKED}},
  {clvTYPE_USHORT_PACKED, {T_USHORT, T_USHORT_PACKED, T_USHORT2_PACKED, T_USHORT3_PACKED, T_USHORT4_PACKED, 0, 0, 0, T_USHORT8_PACKED, 0, 0, 0, 0, 0, 0, 0, T_USHORT16_PACKED}},
  {clvTYPE_HALF_PACKED, {T_HALF, T_HALF_PACKED, T_HALF2_PACKED, T_HALF3_PACKED, T_HALF4_PACKED, 0, 0, 0, T_HALF8_PACKED, 0, 0, 0, 0, 0, 0, 0, T_HALF16_PACKED}},
};

static const gctUINT  _BuiltinPackedVectorTypeCount = sizeof(_BuiltinPackedVectorTypes) / sizeof(clsVecCompSelType);


#if gcdDEBUG
static gctBOOL
_IsBuiltinVecTypeInfoSorted(
IN cloCOMPILER Compiler
)
{
   gctUINT i;

   for(i = 0; i < _BuiltinVectorTypeCount; i++) {
      if(_BuiltinVectorTypes[i].elementType == (gctINT)i)
          continue;
      else {
         if(Compiler) {
        gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                        0,
                        0,
                        clvREPORT_FATAL_ERROR,
                        "builtin vector data type info not sorted at %d"
                                            " with type %d",
                                            i, clBuiltinDataTypes[i].type));
         }
         return gcvFALSE;
      }
   }
   return gcvTRUE;
}
#endif

void
cloIR_InitializeVecCompSelTypes(IN cloCOMPILER Compiler)
{
#if gcdDEBUG
    gcmASSERT(_IsBuiltinVecTypeInfoSorted(Compiler));
#endif
}

#define _clmDATA_TYPE_IsValidGenType(dataType) \
    (clmIsElementTypeArithmetic((dataType)->elementType) && \
     (clmDATA_TYPE_matrixColumnCount_GET(dataType) ==  0))

#define _clmDATA_TYPE_IsValidPackedGenType(dataType) \
    (((dataType)->elementType >= cldFirstPackedType && (dataType)->elementType <= cldLastPackedType) && \
      (clmDATA_TYPE_matrixColumnCount_GET(dataType) ==  0))

#define _clmDATA_TYPE_InitializeGenType(to, from) \
   do { \
       (to)->type = (from)->type; \
       (to)->elementType = (from)->elementType; \
       (to)->matrixSize = (from)->matrixSize; \
   } while (gcvFALSE)

#define _clmDATA_TYPE_InitializeVoid(d) \
   do { \
       (d)->type = T_VOID; \
       (d)->elementType = clvTYPE_VOID; \
       clmDATA_TYPE_vectorSize_SET((d), 0); \
   } while (gcvFALSE)

#define _clmDATA_TYPE_InitializeGenTypeWithCheck(compiler, to, from, ref, refParamName, updateRef) \
   do { \
       gctUINT8 vectorSize; \
       vectorSize =  clmDATA_TYPE_vectorSize_NOCHECK_GET(from); \
       if(vectorSize == 0 && /* scalar */ \
          (vectorSize != clmDATA_TYPE_vectorSize_NOCHECK_GET(ref) || \
           (ref)->elementType > (from)->elementType)) { \
           (to)->type = (ref)->type; \
           (to)->elementType = (ref)->elementType; \
           (to)->matrixSize = (ref)->matrixSize; \
           gcmVERIFY_OK(cloCOMPILER_CloneDataType((Compiler), \
                                                  (to)->accessQualifier, \
                                                  (to)->addrSpaceQualifier, \
                                                  (to), \
                                                  &(refParamName)->u.variableInfo.effectiveDecl.dataType)); \
           (updateRef) = gcvTRUE; \
       } \
       else { \
           (to)->type = (from)->type; \
           (to)->elementType = (from)->elementType; \
           (to)->matrixSize = (from)->matrixSize; \
       } \
   } while (gcvFALSE)

#define _clmConvElementTypeToSigned(et) \
   (et == clvTYPE_UINT ? clvTYPE_INT : \
    (et == clvTYPE_USHORT ? clvTYPE_SHORT : \
       (et == clvTYPE_USHORT_PACKED ? clvTYPE_SHORT_PACKED : \
          (et == clvTYPE_UCHAR ? clvTYPE_CHAR : \
             (et == clvTYPE_UCHAR_PACKED ? clvTYPE_CHAR_PACKED : \
                (et == clvTYPE_ULONG ? clvTYPE_LONG : et))))))

#define _clmConvElementTypeToUnsigned(et) \
   (et == clvTYPE_INT ? clvTYPE_UINT : \
    (et == clvTYPE_SHORT ? clvTYPE_USHORT : \
      (et == clvTYPE_SHORT_PACKED ? clvTYPE_USHORT_PACKED : \
        (et == clvTYPE_CHAR ? clvTYPE_UCHAR : \
          (et == clvTYPE_CHAR_PACKED ? clvTYPE_UCHAR_PACKED : \
             (et == clvTYPE_LONG ? clvTYPE_ULONG : et))))))

#define _cldDoGentypeArgTypeChecking 1

static gctINT _clGentypeArgCheck = _cldDoGentypeArgTypeChecking;

gctINT
clGetVectorTerminalToken(
IN cltELEMENT_TYPE ElementType,
IN gctINT8 NumComponents
)
{
   clsVecCompSelType *vectorSel;

   if(clmIsElementTypePackedGenType(ElementType)) {
       switch(ElementType) {
       case clvTYPE_I_GEN_PACKED:
           return T_I_GENTYPE_PACKED;

       case clvTYPE_U_GEN_PACKED:
           return T_U_GENTYPE_PACKED;

       case clvTYPE_IU_GEN_PACKED:
           return T_IU_GENTYPE_PACKED;

       case clvTYPE_F_GEN_PACKED:
           return T_F_GENTYPE_PACKED;

       case clvTYPE_GEN:
           return T_GENTYPE_PACKED;

       default:
           gcmASSERT(0);
           return T_GENTYPE_PACKED;
       }
   }
   else if(clmIsElementTypeGenType(ElementType)) {
       switch(ElementType) {
       case clvTYPE_SIU_GEN:
           return T_SIU_GENTYPE;

       case clvTYPE_I_GEN:
           return T_I_GENTYPE;

       case clvTYPE_U_GEN:
           return T_U_GENTYPE;

       case clvTYPE_IU_GEN:
           return T_IU_GENTYPE;

       case clvTYPE_F_GEN:
           return T_F_GENTYPE;

       case clvTYPE_GEN:
           return T_GENTYPE;

       default:
           gcmASSERT(0);
           return T_GENTYPE;
       }
   }
   if(clmIsElementTypePacked(ElementType) && !clmIsElementTypePackedGenType(ElementType)) {
       gctINT index;
       index = ElementType - _BuiltinPackedVectorTypes[0].elementType;
       if (!(index >= 0 && (gctUINT)index < _BuiltinPackedVectorTypeCount))
       {
           gcmASSERT(gcvFALSE);
       }
       vectorSel = _BuiltinPackedVectorTypes + index;
   }
   else {
       if (!((gctUINT)ElementType < _BuiltinVectorTypeCount))
       {
           gcmASSERT(gcvFALSE);
       }
       vectorSel = _BuiltinVectorTypes + ElementType;
   }

   gcmASSERT(vectorSel->elementType == ElementType);

   return vectorSel->compSel[NumComponents];
}

#define _clmInitGenType(ElementType, NumComponents, DataType)  \
   do { \
      (DataType)->type = clGetVectorTerminalToken((ElementType), (NumComponents)); \
      (DataType)->elementType = clmGenCodeDataType((DataType)->type).elementType; \
      (DataType)->matrixSize = clmGenCodeDataType((DataType)->type).matrixSize; \
   } while (gcvFALSE)

gceSTATUS
cloIR_CreateVectorType(
IN cloCOMPILER Compiler,
IN clsDATA_TYPE *DataType,
IN gctUINT8  VectorSize,
IN clsDATA_TYPE **VecDataType
)
{
   clsDATA_TYPE dataType[1];
   gcmASSERT(DataType);
   gcmASSERT(VecDataType);

   _clmInitGenType(DataType->elementType,
                   VectorSize,
                   dataType);

   return cloCOMPILER_CreateDataType(Compiler,
                                     dataType->type,
                                     DataType->u.generic,
                                     DataType->accessQualifier,
                                     DataType->addrSpaceQualifier,
                                     VecDataType);
}

gctBOOL
clAreElementTypeInRankOrder(
IN cloCOMPILER Compiler,
IN cltELEMENT_TYPE HighRank,
IN cltELEMENT_TYPE LowRank,
IN gctBOOL IsConst
)
{
   gctINT index;
   clsBUILTIN_DATATYPE_INFO *typeInfo;
   cltELEMENT_TYPE highRank, lowRank;
   gctBOOL noCheck;
   gcePATCH_ID patchId  = gcvPATCH_INVALID;

   patchId = *gcGetPatchId();

   if(clmIsElementTypePacked(HighRank)) {
       index = HighRank - _BuiltinPackedVectorTypes[0].elementType;
       if (!(index >= 0 && (gctUINT)index < _BuiltinPackedVectorTypeCount))
       {
           gcmASSERT(gcvFALSE);
       }
       typeInfo = clGetBuiltinDataTypeInfo((_BuiltinPackedVectorTypes + index)->compSel[0]);
       highRank = typeInfo->dataType.elementType;
   }
   else highRank = HighRank;

   if(clmIsElementTypePacked(LowRank)) {
       index = LowRank - _BuiltinPackedVectorTypes[0].elementType;
       if (!(index >= 0 && (gctUINT)index < _BuiltinPackedVectorTypeCount))
       {
           gcmASSERT(gcvFALSE);
       }
       typeInfo = clGetBuiltinDataTypeInfo((_BuiltinPackedVectorTypes + index)->compSel[0]);
       lowRank = typeInfo->dataType.elementType;
   }
   else lowRank = LowRank;

   noCheck = IsConst && clmIsElementTypeInteger(highRank) && clmIsElementTypeInteger(lowRank);
   if (patchId == gcvPATCH_OCLCTS)
   {
       return !noCheck && (highRank > lowRank);
   }
   else
   {
       return (highRank > lowRank) &&
              !(noCheck ||
                (highRank == clvTYPE_UINT && lowRank == clvTYPE_INT) ||
                (highRank == clvTYPE_USHORT && lowRank == clvTYPE_SHORT) ||
                (highRank == clvTYPE_UCHAR && lowRank == clvTYPE_CHAR));
   }
}


#define cldCHAR_MAX    ((int)0x0000007F)  /* 127 */
#define cldCHAR_MIN    ((int)0xFFFFFF80)  /* (-127-1) */
#define cldUCHAR_MAX   0x000000FF  /* 255 */
#define cldSHRT_MAX    ((int) 0x00007FFF)  /* 32767 */
#define cldSHRT_MIN    ((int)0xFFFF8000)  /* (-32767-1) */
#define cldUSHRT_MAX   0x0000FFFF  /* 65535 */
#define cldINT_MAX     ((int) 0x7FFFFFFF)  /* 2147483647 */
#define cldINT_MIN     ((int) 0x80000000)  /* (-2147483647-1) */
#define cldUINT_MAX    0xFFFFFFFF  /* 4294967295 */
#define cldFLT_MAX     0x7F7FFFFF  /* 340282346638528859811704183484516925440.0f */
#define cldFLT_MIN     0x00800000  /* 1.175494350822287507969e-38f */


static void
_GetGenTypeCandidateType(
IN cloCOMPILER Compiler,
IN clsNAME *GenTypeParam,
IN clsDECL *RefDecl,
IN OUT clsDECL *CandidateType
)
{
   clsDECL *genTypeDecl;
   cltELEMENT_TYPE elementType;

   gcmASSERT(GenTypeParam);

   genTypeDecl = &GenTypeParam->decl;
   _clmDATA_TYPE_InitializeVoid(CandidateType->dataType);
   if((!clmDECL_IsPackedGenType(genTypeDecl) &&
       _clmDATA_TYPE_IsValidGenType(RefDecl->dataType)) ||
      (clmDECL_IsPackedGenType(genTypeDecl) &&
       _clmDATA_TYPE_IsValidPackedGenType(RefDecl->dataType))) {
       _clmDATA_TYPE_InitializeGenType(CandidateType->dataType, RefDecl->dataType);
   }
   else return;

   switch(genTypeDecl->dataType->elementType) {
   case clvTYPE_SIU_GEN:
        if(!clmDECL_IsIntegerType(RefDecl)) {
           elementType = clvTYPE_UINT;
        }
        else elementType = RefDecl->dataType->elementType;
        _clmInitGenType(elementType, 1, CandidateType->dataType);
        break;

   case clvTYPE_IU_GEN:
        if(clmDECL_IsIntegerType(RefDecl)) {
             ;
        }
        else {
           if(!clmDECL_IsPointerType(RefDecl)) {
              _clmInitGenType(clvTYPE_UINT,
                              clmDATA_TYPE_vectorSize_NOCHECK_GET(RefDecl->dataType),
                              CandidateType->dataType);
           }
        }
        break;

   case clvTYPE_I_GEN:
        if(clmDECL_IsIntegerType(RefDecl)) {
           if(clmIsElementTypeUnsigned(RefDecl->dataType->elementType)) {
              _clmInitGenType(_clmConvElementTypeToSigned(RefDecl->dataType->elementType),
                              clmDATA_TYPE_vectorSize_NOCHECK_GET(RefDecl->dataType),
                              CandidateType->dataType);
           }
        }
        else {
           if(!clmDECL_IsPointerType(RefDecl)) {
              _clmInitGenType(clvTYPE_INT,
                              clmDATA_TYPE_vectorSize_NOCHECK_GET(RefDecl->dataType),
                              CandidateType->dataType);
           }
        }
        break;

   case clvTYPE_U_GEN:
        if(clmDECL_IsIntegerType(RefDecl)) {
           if(clmIsElementTypeSigned(RefDecl->dataType->elementType)) {
              _clmInitGenType(_clmConvElementTypeToUnsigned(RefDecl->dataType->elementType),
                              clmDATA_TYPE_vectorSize_NOCHECK_GET(RefDecl->dataType),
                              CandidateType->dataType);
           }
        }
        else {
           if(!clmDECL_IsPointerType(RefDecl)) {
              _clmInitGenType(clvTYPE_UINT,
                              clmDATA_TYPE_vectorSize_NOCHECK_GET(RefDecl->dataType),
                              CandidateType->dataType);
           }
        }
        break;

   case clvTYPE_F_GEN:
        if(!cloCOMPILER_ExtensionEnabled(Compiler, clvEXTENSION_CL_KHR_FP16) &&
           (!clmDECL_IsFloat(RefDecl) ||
            (RefDecl->dataType->elementType == clvTYPE_HALF &&
            GenTypeParam->u.variableInfo.builtinSpecific.s.isConvertibleType))) {
            _clmInitGenType(clvTYPE_FLOAT,
                            clmDATA_TYPE_vectorSize_NOCHECK_GET(RefDecl->dataType),
                            CandidateType->dataType);
        }
        break;

   case clvTYPE_GEN:
        if(RefDecl->dataType->elementType == clvTYPE_HALF &&
           !cloCOMPILER_ExtensionEnabled(Compiler, clvEXTENSION_CL_KHR_FP16) &&
           GenTypeParam->u.variableInfo.builtinSpecific.s.isConvertibleType) {
           _clmInitGenType(clvTYPE_FLOAT,
                           clmDATA_TYPE_vectorSize_NOCHECK_GET(RefDecl->dataType),
                           CandidateType->dataType);
        }
        break;

   case clvTYPE_IU_GEN_PACKED:
        if(clmDECL_IsIntegerType(RefDecl)) {
             ;
        }
        else {
           if(!clmDECL_IsPointerType(RefDecl)) {
              _clmInitGenType(clvTYPE_UINT,
                              clmDATA_TYPE_vectorSize_NOCHECK_GET(RefDecl->dataType),
                              CandidateType->dataType);
           }
        }
        break;

   case clvTYPE_I_GEN_PACKED:
        if(clmDECL_IsIntegerType(RefDecl)) {
           if(clmIsElementTypeUnsigned(RefDecl->dataType->elementType)) {
              _clmInitGenType(_clmConvElementTypeToSigned(RefDecl->dataType->elementType),
                              clmDATA_TYPE_vectorSize_NOCHECK_GET(RefDecl->dataType),
                              CandidateType->dataType);
           }
        }
        else {
           if(!clmDECL_IsPointerType(RefDecl)) {
              _clmInitGenType(clvTYPE_INT,
                              clmDATA_TYPE_vectorSize_NOCHECK_GET(RefDecl->dataType),
                              CandidateType->dataType);
           }
        }
        break;

   case clvTYPE_U_GEN_PACKED:
        if(clmDECL_IsIntegerType(RefDecl)) {
           if(clmIsElementTypeSigned(RefDecl->dataType->elementType)) {
              _clmInitGenType(_clmConvElementTypeToUnsigned(RefDecl->dataType->elementType),
                              clmDATA_TYPE_vectorSize_NOCHECK_GET(RefDecl->dataType),
                              CandidateType->dataType);
           }
        }
        else {
           if(!clmDECL_IsPointerType(RefDecl)) {
              _clmInitGenType(clvTYPE_UINT,
                              clmDATA_TYPE_vectorSize_NOCHECK_GET(RefDecl->dataType),
                              CandidateType->dataType);
           }
        }
        break;

   case clvTYPE_F_GEN_PACKED:
        if(!cloCOMPILER_ExtensionEnabled(Compiler, clvEXTENSION_CL_KHR_FP16) &&
           (!clmDECL_IsFloat(RefDecl) ||
            (RefDecl->dataType->elementType == clvTYPE_HALF_PACKED &&
            GenTypeParam->u.variableInfo.builtinSpecific.s.isConvertibleType))) {
            _clmInitGenType(clvTYPE_FLOAT,
                            clmDATA_TYPE_vectorSize_NOCHECK_GET(RefDecl->dataType),
                            CandidateType->dataType);
        }
        break;

   case clvTYPE_GEN_PACKED:
        if(RefDecl->dataType->elementType == clvTYPE_HALF_PACKED &&
           !cloCOMPILER_ExtensionEnabled(Compiler, clvEXTENSION_CL_KHR_FP16) &&
           GenTypeParam->u.variableInfo.builtinSpecific.s.isConvertibleType) {
           _clmInitGenType(clvTYPE_FLOAT,
                           clmDATA_TYPE_vectorSize_NOCHECK_GET(RefDecl->dataType),
                           CandidateType->dataType);
        }
        break;

   default:
        break;
   }
}

#define _CONVERT_HALF_TO_FLOAT 0

gctBOOL
clsDECL_IsMatchingBuiltinArg(
IN cloCOMPILER Compiler,
IN gctBOOL NotFirstParam,
IN clsNAME * ParamName,
IN cloIR_EXPR Argument,
IN OUT clsNAME **RefParamName
)
{
#if _CONVERT_HALF_TO_FLOAT
  gceSTATUS status = gcvSTATUS_OK;
  clsDECL refDecl[1];
#endif
  clsDECL lDecl[1];
  clsDECL *paramDecl;
  clsDECL *rDecl;
  clsDATA_TYPE dataType[1];
  clsDATA_TYPE *refDataType = gcvNULL;
  cltELEMENT_TYPE refElementType = clvTYPE_VOID;
  gctBOOL updateRefDataType = gcvFALSE;
  gctBOOL matched = gcvFALSE;
  gctBOOL sameType = gcvFALSE;

  gcmASSERT(ParamName);
  gcmASSERT(Argument);
  gcmASSERT(RefParamName);

  if(*RefParamName) {
      refDataType = (*RefParamName)->u.variableInfo.effectiveDecl.dataType;
      refElementType = (*RefParamName)->decl.dataType->elementType;
  }
  paramDecl = &ParamName->decl;
  rDecl = &Argument->decl;
#if _CONVERT_HALF_TO_FLOAT
  if (rDecl->dataType->elementType == clvTYPE_HALF &&
      !clmDECL_IsPointerType(rDecl) && !clmDECL_IsArray(rDecl) &&
      ParamName->u.variableInfo.builtinSpecific.s.isConvertibleType) {
      gctINT resultType;
      resultType = clGetVectorTerminalToken(clvTYPE_FLOAT,
                                            clmDATA_TYPE_vectorSize_GET(rDecl->dataType));

      if(resultType) {
          status = cloCOMPILER_CreateDecl(Compiler,
                                          resultType,
                                          rDecl->dataType->u.generic,
                                          rDecl->dataType->accessQualifier,
                                          rDecl->dataType->addrSpaceQualifier,
                                          refDecl);
          if (gcmIS_ERROR(status)) return status;
          rDecl = refDecl;
      }
  }
#endif
  if(clmDECL_IsScalar(rDecl) && (ParamName->u.variableInfo.builtinSpecific.s.isConvertibleType ||
     clmDECL_IsScalar(paramDecl) || !ParamName->u.variableInfo.builtinSpecific.s.hasGenType)) {
      ParamName->u.variableInfo.effectiveDecl = ParamName->decl;
      if(!clmDECL_IsPointerType(paramDecl)) {
          if(clmDECL_IsScalar(paramDecl)) {
              if(paramDecl->dataType->elementType == clvTYPE_BOOL) {
                  return gcvTRUE;
              }
              else if(!clmDECL_IsPointerType(rDecl)
                      && !clmDECL_IsGenType(paramDecl)
                      && ParamName->u.variableInfo.builtinSpecific.s.isConvertibleType) {
                  if(clmDECL_IsIntegerType(rDecl)
                     && clmDECL_IsCompatibleIntegerType(paramDecl)) {  /*implicit integer conversion */
                       return gcvTRUE;
                  }
                  else if(clmDECL_IsHalfType(rDecl)
                     && clmDECL_IsFloatingType(paramDecl)) {  /*implicit half type to float conversion */
                       return gcvTRUE;
                  }
                  else if(clmDECL_IsArithmeticType(paramDecl)
                          && rDecl->dataType->elementType <= paramDecl->dataType->elementType) {
                       return gcvTRUE;
                  }
              }
          }
          else if (clmDECL_IsVectorType(paramDecl)) {
              if(NotFirstParam &&
                 !clmDECL_IsPointerType(rDecl) &&
                 clmDECL_IsVectorType(paramDecl) &&
                 (rDecl->dataType->elementType <= paramDecl->dataType->elementType) &&
                 ParamName->u.variableInfo.builtinSpecific.s.isConvertibleType) { /*implicit conversion */
                  return gcvTRUE;
              }
          }
     }
  }

  *dataType = *(paramDecl->dataType);
  clmDECL_Initialize(lDecl, dataType, &paramDecl->array, paramDecl->ptrDscr, gcvFALSE, clvSTORAGE_QUALIFIER_NONE);
  if(clmDECL_IsGenType(paramDecl)) {
     if(refDataType == gcvNULL) {
        _GetGenTypeCandidateType(Compiler,
                                 ParamName,
                                 rDecl,
                                 lDecl);
        updateRefDataType = gcvTRUE;
     }
     else switch(paramDecl->dataType->elementType) {
     case clvTYPE_SIU_GEN:
        switch(refElementType) {
        case clvTYPE_SIU_GEN:
           _clmDATA_TYPE_InitializeGenTypeWithCheck(compiler,
                                                    lDecl->dataType,
                                                    refDataType,
                                                    rDecl->dataType,
                                                    *RefParamName,
                                                    updateRefDataType);
           break;

        case clvTYPE_I_GEN:
        case clvTYPE_U_GEN:
        case clvTYPE_IU_GEN:
           _clmInitGenType(refDataType->elementType,
                           1,
                           dataType);
           break;

        case clvTYPE_F_GEN:
        case clvTYPE_GEN:
           if(clmIsElementTypeInteger(rDecl->dataType->elementType)) {
              _clmInitGenType(rDecl->dataType->elementType,
                              1,
                              dataType);
           }
           break;
        default:
           break;
        }
        break;

     case clvTYPE_GEN:
        switch(refElementType) {
        case clvTYPE_SIU_GEN:
           if(_clmDATA_TYPE_IsValidGenType(rDecl->dataType)) {
              _clmDATA_TYPE_InitializeGenType(lDecl->dataType, refDataType);
           }
           updateRefDataType = gcvTRUE;
           break;

        case clvTYPE_I_GEN:
        case clvTYPE_U_GEN:
        case clvTYPE_IU_GEN:
        case clvTYPE_F_GEN:
           if(_clmDATA_TYPE_IsValidGenType(rDecl->dataType)) {
              _clmInitGenType(rDecl->dataType->elementType,
                              clmDATA_TYPE_vectorSize_NOCHECK_GET(refDataType),
                              dataType);
           }
           updateRefDataType = gcvTRUE;
           break;

        case clvTYPE_GEN:
           _clmDATA_TYPE_InitializeGenTypeWithCheck(Compiler,
                                                    lDecl->dataType,
                                                    refDataType,
                                                    rDecl->dataType,
                                                    *RefParamName,
                                                    updateRefDataType);
           break;
        default:
           break;
        }
        break;

     case clvTYPE_I_GEN:
        switch(refElementType) {
        case clvTYPE_IU_GEN:
        case clvTYPE_U_GEN:
           _clmInitGenType(_clmConvElementTypeToSigned(refDataType->elementType),
                           clmDATA_TYPE_vectorSize_NOCHECK_GET(refDataType),
                           dataType);
           break;

        case clvTYPE_SIU_GEN:
           if(_clmDATA_TYPE_IsValidGenType(rDecl->dataType)) {
              _clmInitGenType(_clmConvElementTypeToSigned(refDataType->elementType),
                              clmDATA_TYPE_vectorSize_NOCHECK_GET(rDecl->dataType),
                              dataType);
           }
           updateRefDataType = gcvTRUE;
           break;

        case clvTYPE_I_GEN:
           _clmDATA_TYPE_InitializeGenTypeWithCheck(Compiler,
                                                    lDecl->dataType,
                                                    refDataType,
                                                    rDecl->dataType,
                                                    *RefParamName,
                                                    updateRefDataType);
           break;

        case clvTYPE_F_GEN:
        case clvTYPE_GEN:
           if(_clmDATA_TYPE_IsValidGenType(rDecl->dataType) &&
              clmIsElementTypeInteger(rDecl->dataType->elementType)) {
              _clmInitGenType(_clmConvElementTypeToSigned(rDecl->dataType->elementType),
                              clmDATA_TYPE_vectorSize_NOCHECK_GET(refDataType),
                              dataType);
           }
           break;
        default:
           break;
        }
        break;

     case clvTYPE_U_GEN:
        switch(refElementType) {
        case clvTYPE_IU_GEN:
        case clvTYPE_I_GEN:
           _clmInitGenType(_clmConvElementTypeToUnsigned(refDataType->elementType),
                           clmDATA_TYPE_vectorSize_NOCHECK_GET(refDataType),
                           dataType);
           break;

        case clvTYPE_U_GEN:
           _clmDATA_TYPE_InitializeGenTypeWithCheck(Compiler,
                                                    lDecl->dataType,
                                                    refDataType,
                                                    rDecl->dataType,
                                                    *RefParamName,
                                                    updateRefDataType);
           break;

        case clvTYPE_SIU_GEN:
           if(_clmDATA_TYPE_IsValidGenType(rDecl->dataType)) {
              _clmInitGenType(_clmConvElementTypeToUnsigned(refDataType->elementType),
                              clmDATA_TYPE_vectorSize_NOCHECK_GET(rDecl->dataType),
                              dataType);
           }
           updateRefDataType = gcvTRUE;
           break;

        case clvTYPE_F_GEN:
        case clvTYPE_GEN:
           if(_clmDATA_TYPE_IsValidGenType(rDecl->dataType) &&
              clmIsElementTypeInteger(rDecl->dataType->elementType)) {
              _clmInitGenType(_clmConvElementTypeToUnsigned(rDecl->dataType->elementType),
                              clmDATA_TYPE_vectorSize_NOCHECK_GET(refDataType),
                              dataType);
           }
           break;
        default:
           break;
        }
        break;

     case clvTYPE_IU_GEN:
        switch(refElementType) {
        case clvTYPE_IU_GEN:
           _clmDATA_TYPE_InitializeGenTypeWithCheck(Compiler,
                                                    lDecl->dataType,
                                                    refDataType,
                                                    rDecl->dataType,
                                                    *RefParamName,
                                                    updateRefDataType);
           break;

        case clvTYPE_I_GEN:
           if(_clmDATA_TYPE_IsValidGenType(rDecl->dataType)) {
              if(clmIsElementTypeUnsigned(rDecl->dataType->elementType)) {
                 dataType->elementType =_clmConvElementTypeToUnsigned(refDataType->elementType);
              }
              else {
                 dataType->elementType = refDataType->elementType;
              }
              _clmInitGenType(dataType->elementType,
                              clmDATA_TYPE_vectorSize_NOCHECK_GET(refDataType),
                              dataType);
           }
           updateRefDataType = gcvTRUE;
           break;

        case clvTYPE_U_GEN:
           if(_clmDATA_TYPE_IsValidGenType(rDecl->dataType)) {
              if(clmIsElementTypeSigned(rDecl->dataType->elementType)) {
                 dataType->elementType =_clmConvElementTypeToSigned(refDataType->elementType);
              }
              else {
                 dataType->elementType = refDataType->elementType;
              }
              _clmInitGenType(dataType->elementType,
                              clmDATA_TYPE_vectorSize_NOCHECK_GET(refDataType),
                              dataType);
           }
           updateRefDataType = gcvTRUE;
           break;

        case clvTYPE_SIU_GEN:
           if(_clmDATA_TYPE_IsValidGenType(rDecl->dataType)) {
              _clmInitGenType(refDataType->elementType,
                              clmDATA_TYPE_vectorSize_NOCHECK_GET(rDecl->dataType),
                              dataType);
           }
           updateRefDataType = gcvTRUE;
           break;

        case clvTYPE_F_GEN:
        case clvTYPE_GEN:
           if(_clmDATA_TYPE_IsValidGenType(rDecl->dataType) &&
              clmIsElementTypeInteger(rDecl->dataType->elementType)) {
              _clmInitGenType(rDecl->dataType->elementType,
                              clmDATA_TYPE_vectorSize_NOCHECK_GET(refDataType),
                              dataType);
           }
           break;
        default:
           break;
        }
        break;

     case clvTYPE_F_GEN:
        switch(refElementType) {
        case clvTYPE_SIU_GEN:
           if(_clmDATA_TYPE_IsValidGenType(rDecl->dataType) &&
              clmIsElementTypeFloating(rDecl->dataType->elementType)) {
              _clmDATA_TYPE_InitializeGenType(lDecl->dataType, rDecl->dataType);
           }
           updateRefDataType = gcvTRUE;
           break;

        case clvTYPE_I_GEN:
        case clvTYPE_U_GEN:
        case clvTYPE_IU_GEN:
           if(clmIsElementTypeFloating(rDecl->dataType->elementType)) {
              _clmInitGenType(rDecl->dataType->elementType,
                              clmDATA_TYPE_vectorSize_NOCHECK_GET(refDataType),
                              dataType);
           }
           updateRefDataType = gcvTRUE;
           break;

        case clvTYPE_F_GEN:
           _clmDATA_TYPE_InitializeGenTypeWithCheck(Compiler,
                                                    lDecl->dataType,
                                                    refDataType,
                                                    rDecl->dataType,
                                                    *RefParamName,
                                                    updateRefDataType);
           break;

        case clvTYPE_GEN:
           if(clmIsElementTypeFloating(rDecl->dataType->elementType)) {
              _clmInitGenType(rDecl->dataType->elementType,
                              clmDATA_TYPE_vectorSize_NOCHECK_GET(refDataType),
                              dataType);
           }
           break;
        default:
           break;
        }
        break;

     case clvTYPE_GEN_PACKED:
        switch(refElementType) {
        case clvTYPE_SIU_GEN:
           if(_clmDATA_TYPE_IsValidGenType(rDecl->dataType)) {
              _clmDATA_TYPE_InitializeGenType(lDecl->dataType, refDataType);
           }
           updateRefDataType = gcvTRUE;
           break;

        case clvTYPE_I_GEN_PACKED:
        case clvTYPE_U_GEN_PACKED:
        case clvTYPE_IU_GEN_PACKED:
        case clvTYPE_F_GEN_PACKED:
           if(_clmDATA_TYPE_IsValidGenType(rDecl->dataType)) {
              _clmInitGenType(rDecl->dataType->elementType,
                              clmDATA_TYPE_vectorSize_NOCHECK_GET(refDataType),
                              dataType);
           }
           updateRefDataType = gcvTRUE;
           break;

        case clvTYPE_GEN_PACKED:
           _clmDATA_TYPE_InitializeGenTypeWithCheck(Compiler,
                                                    lDecl->dataType,
                                                    refDataType,
                                                    rDecl->dataType,
                                                    *RefParamName,
                                                    updateRefDataType);
           break;
        default:
           break;
        }
        break;

     case clvTYPE_I_GEN_PACKED:
        switch(refElementType) {
        case clvTYPE_IU_GEN_PACKED:
        case clvTYPE_U_GEN_PACKED:
           _clmInitGenType(_clmConvElementTypeToSigned(refDataType->elementType),
                           clmDATA_TYPE_vectorSize_NOCHECK_GET(refDataType),
                           dataType);
           break;

        case clvTYPE_SIU_GEN:
           if(_clmDATA_TYPE_IsValidGenType(rDecl->dataType)) {
              _clmInitGenType(_clmConvElementTypeToSigned(refDataType->elementType),
                              clmDATA_TYPE_vectorSize_NOCHECK_GET(rDecl->dataType),
                              dataType);
           }
           updateRefDataType = gcvTRUE;
           break;

        case clvTYPE_I_GEN_PACKED:
           _clmDATA_TYPE_InitializeGenTypeWithCheck(Compiler,
                                                    lDecl->dataType,
                                                    refDataType,
                                                    rDecl->dataType,
                                                    *RefParamName,
                                                    updateRefDataType);
           break;

        case clvTYPE_F_GEN_PACKED:
        case clvTYPE_GEN_PACKED:
           if(_clmDATA_TYPE_IsValidGenType(rDecl->dataType) &&
              clmIsElementTypeInteger(rDecl->dataType->elementType)) {
              _clmInitGenType(_clmConvElementTypeToSigned(rDecl->dataType->elementType),
                              clmDATA_TYPE_vectorSize_NOCHECK_GET(refDataType),
                              dataType);
           }
           break;
        default:
           break;
        }
        break;

     case clvTYPE_U_GEN_PACKED:
        switch(refElementType) {
        case clvTYPE_IU_GEN_PACKED:
        case clvTYPE_I_GEN_PACKED:
           _clmInitGenType(_clmConvElementTypeToUnsigned(refDataType->elementType),
                           clmDATA_TYPE_vectorSize_NOCHECK_GET(refDataType),
                           dataType);
           break;

        case clvTYPE_U_GEN_PACKED:
           _clmDATA_TYPE_InitializeGenTypeWithCheck(Compiler,
                                                    lDecl->dataType,
                                                    refDataType,
                                                    rDecl->dataType,
                                                    *RefParamName,
                                                    updateRefDataType);
           break;

        case clvTYPE_SIU_GEN:
           if(_clmDATA_TYPE_IsValidGenType(rDecl->dataType)) {
              _clmInitGenType(_clmConvElementTypeToUnsigned(refDataType->elementType),
                              clmDATA_TYPE_vectorSize_NOCHECK_GET(rDecl->dataType),
                              dataType);
           }
           updateRefDataType = gcvTRUE;
           break;

        case clvTYPE_F_GEN_PACKED:
        case clvTYPE_GEN_PACKED:
           if(_clmDATA_TYPE_IsValidGenType(rDecl->dataType) &&
              clmIsElementTypeInteger(rDecl->dataType->elementType)) {
              _clmInitGenType(_clmConvElementTypeToUnsigned(rDecl->dataType->elementType),
                              clmDATA_TYPE_vectorSize_NOCHECK_GET(refDataType),
                              dataType);
           }
           break;
        default:
           break;
        }
        break;

     case clvTYPE_IU_GEN_PACKED:
        switch(refElementType) {
        case clvTYPE_IU_GEN_PACKED:
           _clmDATA_TYPE_InitializeGenTypeWithCheck(Compiler,
                                                    lDecl->dataType,
                                                    refDataType,
                                                    rDecl->dataType,
                                                    *RefParamName,
                                                    updateRefDataType);
           break;

        case clvTYPE_I_GEN_PACKED:
           if(_clmDATA_TYPE_IsValidGenType(rDecl->dataType)) {
              if(clmIsElementTypeUnsigned(rDecl->dataType->elementType)) {
                 dataType->elementType =_clmConvElementTypeToUnsigned(refDataType->elementType);
              }
              else {
                 dataType->elementType = refDataType->elementType;
              }
              _clmInitGenType(dataType->elementType,
                              clmDATA_TYPE_vectorSize_NOCHECK_GET(refDataType),
                              dataType);
           }
           updateRefDataType = gcvTRUE;
           break;

        case clvTYPE_U_GEN_PACKED:
           if(_clmDATA_TYPE_IsValidGenType(rDecl->dataType)) {
              if(clmIsElementTypeSigned(rDecl->dataType->elementType)) {
                 dataType->elementType =_clmConvElementTypeToSigned(refDataType->elementType);
              }
              else {
                 dataType->elementType = refDataType->elementType;
              }
              _clmInitGenType(dataType->elementType,
                              clmDATA_TYPE_vectorSize_NOCHECK_GET(refDataType),
                              dataType);
           }
           updateRefDataType = gcvTRUE;
           break;

        case clvTYPE_SIU_GEN:
           if(_clmDATA_TYPE_IsValidGenType(rDecl->dataType)) {
              _clmInitGenType(refDataType->elementType,
                              clmDATA_TYPE_vectorSize_NOCHECK_GET(rDecl->dataType),
                              dataType);
           }
           updateRefDataType = gcvTRUE;
           break;

        case clvTYPE_F_GEN_PACKED:
        case clvTYPE_GEN_PACKED:
           if(_clmDATA_TYPE_IsValidGenType(rDecl->dataType) &&
              clmIsElementTypeInteger(rDecl->dataType->elementType)) {
              _clmInitGenType(rDecl->dataType->elementType,
                              clmDATA_TYPE_vectorSize_NOCHECK_GET(refDataType),
                              dataType);
           }
           break;
        default:
           break;
        }
        break;

     case clvTYPE_F_GEN_PACKED:
        switch(refElementType) {
        case clvTYPE_SIU_GEN:
           if(_clmDATA_TYPE_IsValidGenType(rDecl->dataType) &&
              clmIsElementTypeFloating(rDecl->dataType->elementType)) {
              _clmDATA_TYPE_InitializeGenType(lDecl->dataType, rDecl->dataType);
           }
           updateRefDataType = gcvTRUE;
           break;

        case clvTYPE_I_GEN_PACKED:
        case clvTYPE_U_GEN_PACKED:
        case clvTYPE_IU_GEN_PACKED:
           if(clmIsElementTypeHalf(rDecl->dataType->elementType)) {
              _clmInitGenType(rDecl->dataType->elementType,
                              clmDATA_TYPE_vectorSize_NOCHECK_GET(refDataType),
                              dataType);
           }
           updateRefDataType = gcvTRUE;
           break;

        case clvTYPE_F_GEN_PACKED:
           _clmDATA_TYPE_InitializeGenTypeWithCheck(Compiler,
                                                    lDecl->dataType,
                                                    refDataType,
                                                    rDecl->dataType,
                                                    *RefParamName,
                                                    updateRefDataType);
           break;

        case clvTYPE_GEN_PACKED:
           if(clmIsElementTypeHalf(rDecl->dataType->elementType)) {
              _clmInitGenType(rDecl->dataType->elementType,
                              clmDATA_TYPE_vectorSize_NOCHECK_GET(refDataType),
                              dataType);
           }
           break;
        default:
           break;
        }
        break;

     default:
        break;
     }

     if(clmDECL_IsScalar(rDecl) &&
        !clmDECL_IsPointerType(rDecl) &&
        !clmDECL_IsVoid(lDecl) &&
        ParamName->u.variableInfo.builtinSpecific.s.isConvertibleType) {  /*implicit conversion */
        sameType = gcvTRUE;
     }
  }
  else if(clmDECL_IsPackedGenType(paramDecl) &&
     clmIsElementTypePacked(rDecl->dataType->elementType)) {
      sameType = gcvTRUE;
  }
  else if(clmIsElementTypeImageGeneric(paramDecl->dataType->elementType) &&
     clmIsElementTypeImage(rDecl->dataType->elementType)) {
      sameType = gcvTRUE;
  }
  else if(!cloCOMPILER_ExtensionEnabled(Compiler, clvEXTENSION_CL_KHR_FP16) &&
     clmIsElementTypeFloating(paramDecl->dataType->elementType) &&
     rDecl->dataType->elementType == clvTYPE_HALF) {
      sameType = gcvTRUE;
  }

  if(sameType ||
     (lDecl->dataType->elementType == rDecl->dataType->elementType &&
     (clmDATA_TYPE_vectorSize_NOCHECK_GET(lDecl->dataType) == clmDATA_TYPE_vectorSize_NOCHECK_GET(rDecl->dataType)) &&
     (clmDATA_TYPE_matrixRowCount_GET(lDecl->dataType) == clmDATA_TYPE_matrixRowCount_GET(rDecl->dataType)) &&
     (clmDATA_TYPE_matrixColumnCount_GET(lDecl->dataType) == clmDATA_TYPE_matrixColumnCount_GET(rDecl->dataType)) &&
     (lDecl->dataType->u.generic == rDecl->dataType->u.generic))) {
    if(!clmDECL_IsArray(lDecl)) {
       if(clmDECL_IsArray(rDecl)) {
          matched = (clParseCountIndirectionLevel(lDecl->ptrDscr) == 1);
       }
       else {
          matched = (clParseCountIndirectionLevel(lDecl->ptrDscr) ==
                     clParseCountIndirectionLevel(rDecl->ptrDscr));
       }
    }
    else if(_IsSameArraySize(&lDecl->array, &rDecl->array)) {
          matched = (clParseCountIndirectionLevel(lDecl->ptrDscr) ==
                     clParseCountIndirectionLevel(rDecl->ptrDscr));
    }
  }

  if(matched) {
    gceSTATUS status;
    if(clmDECL_IsGenType(paramDecl)) {
       ParamName->u.variableInfo.effectiveDecl = *lDecl;
       status = cloCOMPILER_CloneDataType(Compiler,
                                          lDecl->dataType->accessQualifier,
                                          lDecl->dataType->addrSpaceQualifier,
                                          lDecl->dataType,
                                          &ParamName->u.variableInfo.effectiveDecl.dataType);
       gcmASSERT(gcmNO_ERROR(status));

    }
    else {
      ParamName->u.variableInfo.effectiveDecl = ParamName->decl;
    }
    if(updateRefDataType) {
       *RefParamName = ParamName;
    }
  }
  if(clmDECL_IsGenType(paramDecl) && !_clGentypeArgCheck) return gcvTRUE;
  return matched;
}

static void
_UpdateGentypeDataType(
IN clsNAME *RefParamName,
IN clsDATA_TYPE *OrgFuncDataType,
IN OUT clsDATA_TYPE *FuncDataType
)
{
   clsDATA_TYPE *refDataType = gcvNULL;
   cltELEMENT_TYPE refElementType = clvTYPE_VOID;

   if(!clmDATA_TYPE_IsGenType(OrgFuncDataType)) return;

   if(RefParamName) {
       refDataType = RefParamName->u.variableInfo.effectiveDecl.dataType;
       refElementType = RefParamName->decl.dataType->elementType;
   }
   if(refDataType && OrgFuncDataType->elementType == refElementType) {
       _clmDATA_TYPE_InitializeGenType(FuncDataType, refDataType);
   }
   else if(clmDATA_TYPE_IsGenType(FuncDataType)) {
     cltELEMENT_TYPE elementType;

     switch(refElementType) {
     case clvTYPE_I_GEN:
     case clvTYPE_U_GEN:
     case clvTYPE_IU_GEN:
     case clvTYPE_GEN:
       if(FuncDataType->elementType == clvTYPE_I_GEN) {
          if(clmIsElementTypeUnsigned(refDataType->elementType)) {
             elementType =_clmConvElementTypeToSigned(refDataType->elementType);
          }
          else {
             elementType = refDataType->elementType;
          }
       }
       else if(FuncDataType->elementType == clvTYPE_U_GEN) {
          if(clmIsElementTypeSigned(refDataType->elementType)) {
             elementType =_clmConvElementTypeToUnsigned(refDataType->elementType);
          }
          else {
             elementType = refDataType->elementType;
          }
       }
       else if(FuncDataType->elementType == clvTYPE_F_GEN) {
          if(clmIsElementTypeFloating(refDataType->elementType)) {
             elementType = refDataType->elementType;
          }
          else elementType = clvTYPE_FLOAT;
       }
       else return;
       break;

     case clvTYPE_F_GEN:
       if(FuncDataType->elementType == clvTYPE_I_GEN) {
          elementType = clvTYPE_INT;
       }
       else if(FuncDataType->elementType == clvTYPE_U_GEN ||
               FuncDataType->elementType == clvTYPE_IU_GEN) {
          elementType = clvTYPE_UINT;
       }
       else elementType = clvTYPE_FLOAT;
       break;

     default:
       return;
     }
     _clmInitGenType(elementType,
                     clmDATA_TYPE_vectorSize_NOCHECK_GET(refDataType),
                     FuncDataType);
  }
}

gceSTATUS
_CloneBuiltinFuncName(
IN cloCOMPILER Compiler,
IN cloIR_POLYNARY_EXPR FuncCall,
cltPOOL_STRING Symbol,
IN clsNAME *FuncName,
OUT clsNAME **NewFuncName
)
{
   gceSTATUS status = gcvSTATUS_OK;
   clsNAME *newFuncName = gcvNULL;
   clsNAME_SPACE *builtinSpace;
   clsNAME_SPACE *orgSpace;

   /* Verify the arguments. */
   clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);

   /* Create a new name in the FuncName name space */
   gcmASSERT(FuncName && FuncName->mySpace);
   builtinSpace = cloCOMPILER_GetBuiltinSpace(Compiler);
   orgSpace = cloCOMPILER_GetCurrentSpace(Compiler);

   cloCOMPILER_SetCurrentSpace(Compiler, builtinSpace);

   status = _clsNAME_Construct(Compiler,
                               builtinSpace,
                               FuncCall->exprBase.base.lineNo,
                               FuncCall->exprBase.base.stringNo,
                               clvFUNC_NAME,
                               &FuncCall->exprBase.decl,
                               Symbol,
                               gcvNULL,
                               gcvTRUE,
                               FuncName->extension,
                               &newFuncName);

   if (gcmIS_ERROR(status)) {
       cloCOMPILER_SetCurrentSpace(Compiler, orgSpace);
       return status;
   }

   newFuncName->die = cloCOMPILER_AddDIEWithName(Compiler, newFuncName);

   slsDLINK_LIST_InsertFirst(&builtinSpace->names, &newFuncName->node);

   newFuncName->u.funcInfo.isInline = FuncName->u.funcInfo.isInline;
   newFuncName->u.funcInfo.hasVarArg = FuncName->u.funcInfo.hasVarArg;
   newFuncName->u.funcInfo.hasWriteArg = FuncName->u.funcInfo.hasWriteArg;
   newFuncName->u.funcInfo.passArgByRef = FuncName->u.funcInfo.passArgByRef;
   newFuncName->u.funcInfo.hasGenType = FuncName->u.funcInfo.hasGenType;
   newFuncName->u.funcInfo.isIntrinsicCall = FuncName->u.funcInfo.isIntrinsicCall;
   newFuncName->u.funcInfo.intrinsicKind = FuncName->u.funcInfo.intrinsicKind;
   newFuncName->u.funcInfo.mangledName = gcvNULL;


   if(FuncCall->operands) {
      clsNAME *paramName;
      clsNAME *newParamName;
      clsDECL decl[1];

      status = cloCOMPILER_CreateNameSpace(Compiler,
                                           &newFuncName->u.funcInfo.localSpace);
      if (gcmIS_ERROR(status)) return status;
      newFuncName->u.funcInfo.localSpace->scopeName = newFuncName;
      newFuncName->u.funcInfo.localSpace->die = newFuncName->die;

      gcmASSERT(FuncName->u.funcInfo.localSpace);
      FOR_EACH_DLINK_NODE(&FuncName->u.funcInfo.localSpace->names, struct _clsNAME, paramName) {
        /* Create parameter name */
        gcmONERROR(cloCOMPILER_CloneDecl(Compiler,
                                         paramName->decl.dataType->accessQualifier,
                                         clvQUALIFIER_PRIVATE,
                                         &paramName->u.variableInfo.effectiveDecl,
                                         decl));

        gcmONERROR(cloCOMPILER_CreateName(Compiler,
                                          FuncCall->exprBase.base.lineNo,
                                          FuncCall->exprBase.base.stringNo,
                                          clvPARAMETER_NAME,
                                          decl,
                                          "",
                                          decl->ptrDscr,
                                          clvEXTENSION_NONE,
                                          &newParamName));
        newParamName->u.variableInfo.effectiveDecl = newParamName->decl;
        newParamName->u.variableInfo.builtinSpecific.s.isConvertibleType = paramName->u.variableInfo.builtinSpecific.s.isConvertibleType;
        newParamName->u.variableInfo.builtinSpecific.s.hasGenType = paramName->u.variableInfo.builtinSpecific.s.hasGenType;
      }
OnError:
      cloCOMPILER_PopCurrentNameSpace(Compiler, gcvNULL);
   }

   cloCOMPILER_SetCurrentSpace(Compiler, orgSpace);

   if(NewFuncName) *NewFuncName = newFuncName;
   return status;
}

static gctBOOL
_IsDeclParameterizableTo(
IN clsDECL * LDecl,
IN clsDECL * RDecl
)
{
  gcmASSERT(LDecl);
  gcmASSERT(RDecl);

  if(clmDECL_IsPackedType(RDecl) &&
     LDecl->dataType->elementType == clvTYPE_GEN_PACKED) {
      return gcvTRUE;
  }
  else if(clmIsElementTypeImageGeneric(LDecl->dataType->elementType) &&
     clmIsElementTypeImage(RDecl->dataType->elementType)) {
      return gcvTRUE;
  }
  if(clmDECL_IsScalar(RDecl)) {
    if(!clmDECL_IsPointerType(LDecl)) {
       if(LDecl->dataType->elementType == clvTYPE_BOOL) {
         return gcvTRUE;
       }
       else if((!clmDECL_IsPointerType(RDecl) &&
                clmDECL_IsArithmeticType(LDecl)) ||
                (clmDECL_IsIntegerType(RDecl) &&
                 clmIsElementTypeEvent(LDecl->dataType->elementType))) {/*implicit conversion */
         return gcvTRUE;
       }
    }
    else {
       if(clmDECL_IsPointerType(RDecl)) {
           if(clmDATA_TYPE_IsVoid(LDecl->dataType) ||
              clmDATA_TYPE_IsVoid(RDecl->dataType) ||
              LDecl->dataType->elementType == RDecl->dataType->elementType) {
              return gcvTRUE;
           }
           else if(clmIsElementTypePacked(RDecl->dataType->elementType)) {
              clsBUILTIN_DATATYPE_INFO *typeInfo;

              typeInfo = clGetBuiltinDataTypeInfo(RDecl->dataType->type);
              if(typeInfo->dualType == LDecl->dataType->type) return gcvTRUE;
           }
           else if(clmIsElementTypePacked(LDecl->dataType->elementType)) {
              clsBUILTIN_DATATYPE_INFO *typeInfo;

              typeInfo = clGetBuiltinDataTypeInfo(LDecl->dataType->type);
              if(typeInfo->dualType == RDecl->dataType->type) return gcvTRUE;
           }
       }
       else if(clmDECL_IsInt(RDecl)) {
           return gcvTRUE;
       }
    }
  }

  if((LDecl->dataType->elementType == RDecl->dataType->elementType) &&
     (clmDATA_TYPE_vectorSize_NOCHECK_GET(LDecl->dataType) == clmDATA_TYPE_vectorSize_NOCHECK_GET(RDecl->dataType)) &&
     (clmDATA_TYPE_matrixRowCount_GET(LDecl->dataType) == clmDATA_TYPE_matrixRowCount_GET(RDecl->dataType)) &&
     (clmDATA_TYPE_matrixColumnCount_GET(LDecl->dataType) == clmDATA_TYPE_matrixColumnCount_GET(RDecl->dataType)) &&
     (LDecl->dataType->u.generic == RDecl->dataType->u.generic)) {
    if(!clmDECL_IsArray(LDecl)) {
       if(clmDECL_IsArray(RDecl)) {
          return clParseCountIndirectionLevel(LDecl->ptrDscr) == RDecl->array.numDim;
       }
       return clParseCountIndirectionLevel(LDecl->ptrDscr) ==
              clParseCountIndirectionLevel(RDecl->ptrDscr);
    }
    else {
       if(clmDECL_IsPointerType(RDecl)) {
          return clParseCountIndirectionLevel(RDecl->ptrDscr) == LDecl->array.numDim;
       }

       if(_IsSameArraySize(&LDecl->array, &RDecl->array)) {
          return clParseCountIndirectionLevel(LDecl->ptrDscr) ==
                 clParseCountIndirectionLevel(RDecl->ptrDscr);
       }
    }
  }
  return gcvFALSE;
}

static gctBOOL
_IsCorrespondingFuncName(
IN cloCOMPILER Compiler,
IN clsNAME * FuncName,
IN cloIR_POLYNARY_EXPR PolynaryExpr,
OUT gctBOOL *HasGenType,
OUT clsDATA_TYPE *FuncDataType
)
{
    gceSTATUS   status;
    gctUINT     paramCount;
    clsNAME *   paramName;
    cloIR_EXPR  argument;

    gcmASSERT(FuncName);
    gcmASSERT(FuncName->type == clvFUNC_NAME ||
              FuncName->type == clvKERNEL_FUNC_NAME);
    gcmASSERT(FuncName->u.funcInfo.localSpace);
    gcmASSERT(FuncDataType);
    gcmASSERT(HasGenType);

    clmVERIFY_IR_OBJECT(PolynaryExpr, clvIR_POLYNARY_EXPR);
    gcmASSERT(PolynaryExpr->type == clvPOLYNARY_FUNC_CALL);
    gcmASSERT(PolynaryExpr->funcSymbol);

    if (FuncName->symbol != PolynaryExpr->funcSymbol) return gcvFALSE;


    *FuncDataType = *FuncName->decl.dataType;
    *HasGenType = clmDATA_TYPE_IsGenType(FuncName->decl.dataType);
    if (PolynaryExpr->operands == gcvNULL) {

       gcmVERIFY_OK(cloNAME_GetParamCount(Compiler,
                                          FuncName,
                                          &paramCount));
       return (paramCount == 0);
    }

    if(FuncName->isBuiltin) {
       clsNAME *refParamName = gcvNULL;
       gctBOOL notFirstParam = gcvFALSE;

       for (paramName = (clsNAME *)FuncName->u.funcInfo.localSpace->names.next,
            argument = (cloIR_EXPR)PolynaryExpr->operands->members.next;
            (slsDLINK_NODE *)paramName != &FuncName->u.funcInfo.localSpace->names
            && (slsDLINK_NODE *)argument != &PolynaryExpr->operands->members;
            paramName = (clsNAME *)((slsDLINK_NODE *)paramName)->next,
            argument = (cloIR_EXPR)((slsDLINK_NODE *)argument)->next) {
            if (paramName->type != clvPARAMETER_NAME) break;

            if (!clsDECL_IsMatchingBuiltinArg(Compiler,
                                              notFirstParam,
                                              paramName,
                                              argument,
                                              &refParamName)) {
                return gcvFALSE;
            }

            notFirstParam = gcvTRUE;
            if (clmDECL_IsPointerType(&paramName->decl) && clmDECL_IsArray(&argument->decl)) {
                slsDLINK_NODE *nextArgument = ((slsDLINK_NODE *)argument)->next;

                /* edit array argument to the form of &A[0] */
                slsDLINK_NODE_Detach((slsDLINK_NODE *)argument);
                status = clParseMakeArrayPointerExpr(Compiler,
                                                     argument,
                                                     &argument);
                if(gcmIS_ERROR(status)) return gcvFALSE;

                slsDLINK_NODE_InsertPrev(nextArgument, (slsDLINK_NODE *)argument);
            }

            if(!*HasGenType && refParamName &&
               clmIsElementTypeGenType(refParamName->decl.dataType->elementType)) {
                *HasGenType = gcvTRUE;
            }

            _UpdateGentypeDataType(refParamName, FuncName->decl.dataType, FuncDataType);
       }
       if(clmDATA_TYPE_IsGenType(FuncDataType)) return gcvFALSE;
    }
    else {
       for (paramName = (clsNAME *)FuncName->u.funcInfo.localSpace->names.next,
            argument = (cloIR_EXPR)PolynaryExpr->operands->members.next;
            (slsDLINK_NODE *)paramName != &FuncName->u.funcInfo.localSpace->names
            && (slsDLINK_NODE *)argument != &PolynaryExpr->operands->members;
            paramName = (clsNAME *)((slsDLINK_NODE *)paramName)->next,
            argument = (cloIR_EXPR)((slsDLINK_NODE *)argument)->next) {
            if (paramName->type != clvPARAMETER_NAME) break;

            if (!_IsDeclParameterizableTo(&paramName->decl, &argument->decl)) return gcvFALSE;
            if (clmDECL_IsPointerType(&paramName->decl) && clmDECL_IsArray(&argument->decl)) {
                slsDLINK_NODE *nextArgument = ((slsDLINK_NODE *)argument)->next;

                /* edit array argument to the form of &A[0] */
                slsDLINK_NODE_Detach((slsDLINK_NODE *)argument);
                status = clParseMakeArrayPointerExpr(Compiler,
                                                     argument,
                                                     &argument);
                if(gcmIS_ERROR(status)) return gcvFALSE;

                slsDLINK_NODE_InsertPrev(nextArgument, (slsDLINK_NODE *)argument);
            }

       }
    }

    if (((slsDLINK_NODE *)paramName != &FuncName->u.funcInfo.localSpace->names
          && paramName->type == clvPARAMETER_NAME) ||
        ((slsDLINK_NODE *)argument != &PolynaryExpr->operands->members
          && !FuncName->u.funcInfo.hasVarArg)) {
        return gcvFALSE;
    }
    return gcvTRUE;
}

static gceSTATUS
_FindFuncName(
IN cloCOMPILER Compiler,
IN clsNAME_SPACE * NameSpace,
IN OUT cloIR_POLYNARY_EXPR PolynaryExpr
)
{
    clsNAME *name;
    clsDATA_TYPE dataType[1];
    gctBOOL hasGenType;

    gcmASSERT(PolynaryExpr->exprBase.decl.dataType);
    FOR_EACH_DLINK_NODE(&NameSpace->names, clsNAME, name) {
        hasGenType = gcvFALSE;
        if (((name->type == clvFUNC_NAME) || (name->type == clvKERNEL_FUNC_NAME))
            && clsDECL_IsEqual(&name->decl, &PolynaryExpr->exprBase.decl)
            && _IsCorrespondingFuncName(Compiler, name, PolynaryExpr, &hasGenType, dataType)) {

            if (name->extension != clvEXTENSION_NONE) {
              if (!cloCOMPILER_ExtensionEnabled(Compiler, name->extension)) {
                continue;
              }
            }

            PolynaryExpr->funcName    = name;
            return gcvSTATUS_OK;
        }
    }

    if (NameSpace->parent != gcvNULL) {
        return _FindFuncName(Compiler,
                     NameSpace->parent,
                     PolynaryExpr);
    }
    return gcvSTATUS_NOT_FOUND;
}

gceSTATUS
clConstructScalarIntegerConstant(
IN cloCOMPILER Compiler,
IN gctUINT LineNo,
IN gctUINT StringNo,
IN gctINT IntegerValue,
OUT cloIR_CONSTANT * Constant
)
{
  gceSTATUS status;
  cloIR_CONSTANT constant = gcvNULL;
  cluCONSTANT_VALUE values[1];
  clsDATA_TYPE *dataType;
  clsDECL decl[1];
  gctPOINTER pointer;

  /* Verify the arguments. */
  clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);

  do {
     status = cloCOMPILER_Allocate(Compiler,
                                   (gctSIZE_T)sizeof(struct _cloIR_CONSTANT),
                                   (gctPOINTER *) &pointer);
     if (gcmIS_ERROR(status)) break;
     constant = pointer;

     status = cloCOMPILER_CreateDataType(Compiler,
                                         T_INT,
                                         gcvNULL,
                                         clvQUALIFIER_CONST,
                                         clvQUALIFIER_NONE,
                                         &dataType);
     if (gcmIS_ERROR(status)) break;

     clmDECL_Initialize(decl, dataType, (clsARRAY *)0, gcvNULL, gcvFALSE, clvSTORAGE_QUALIFIER_NONE);

     status = cloIR_CONSTANT_Construct(Compiler,
                                       LineNo,
                                       StringNo,
                                       decl,
                                       &constant);
     if (gcmIS_ERROR(status)) break;

     values->intValue = IntegerValue;
     status = cloIR_CONSTANT_AddValues(Compiler,
                                       constant,
                                       1,
                                       values);
     if (gcmIS_ERROR(status)) break;

     *Constant = constant;
     return gcvSTATUS_OK;
   } while (gcvFALSE);

   *Constant = gcvNULL;
   return status;
}

static gceSTATUS
_MapFuncCallToPassByRef(
IN cloCOMPILER Compiler,
IN clsNAME_SPACE * NameSpace,
IN OUT gctBOOL *HasGenType,
IN OUT cloIR_POLYNARY_EXPR FuncCall
)
{
   gceSTATUS status = gcvSTATUS_OK;
   cloIR_EXPR argument;
   gctSIZE_T argumentCount = 0;
   cloIR_SET argumentSet = gcvNULL;
   clsNAME *variableName;
   clsNAME *name;
   gctBOOL hasGenType;
   cloIR_SET argumentSaved;
   cltPOOL_STRING funcSymbolSaved;
   clsNAME *funcNameSaved;
   clsDATA_TYPE dataType[1];
   gctUINT offset = 0;
   gctSIZE_T funcNameLength;
   gctSTRING nameSuffix;
   gctSTRING symbol;
   cltPOOL_STRING symbolInPool;
   clsNAME_SPACE * nameSpace;
   gctUINT operandCount;
   gctBOOL mapLastArg = gcvTRUE;

   gcmVERIFY_OK(cloIR_SET_GetMemberCount(Compiler,
                     FuncCall->operands,
                     &operandCount));
   FOR_EACH_DLINK_NODE(&FuncCall->operands->members, struct _cloIR_EXPR, argument) {
       if(cloIR_OBJECT_GetType(&argument->base) != clvIR_VARIABLE) {
         status = gcvSTATUS_NOT_FOUND;
         goto OnError;
       }
       variableName = ((cloIR_VARIABLE) &argument->base)->name;
       if(variableName->u.variableInfo.alias &&
          (clmDECL_IsPointerType(&variableName->u.variableInfo.alias->decl) ||
           variableName->u.variableInfo.alias->u.variableInfo.isAddressed ||
           variableName->u.variableInfo.isInitializedWithExtendedVectorConstant) &&
          !variableName->u.variableInfo.alias->u.variableInfo.isDirty) {
          cloIR_VARIABLE variable;
          cloIR_CONSTANT constant;
          cloIR_EXPR leftOperand;
          cloIR_EXPR rightOperand;
          cloIR_EXPR resExpr;

          if(variableName->u.variableInfo.isInitializedWithExtendedVectorConstant &&
             !variableName->u.variableInfo.alias->u.variableInfo.isDirty) {
               status =  clsNAME_SetVariableAddressed(Compiler,
                                                      variableName);
               if (gcmIS_ERROR(status)) return status;
          }

          if(argumentCount == 0) {
             status = cloIR_SET_Construct(Compiler,
                                          FuncCall->exprBase.base.lineNo,
                                          FuncCall->exprBase.base.stringNo,
                                          clvEXPR_SET,
                                          &argumentSet);
             if (gcmIS_ERROR(status)) return status;
          }

          gcmONERROR(cloIR_VARIABLE_Construct(Compiler,
                                              FuncCall->exprBase.base.lineNo,
                                              FuncCall->exprBase.base.stringNo,
                                              variableName->u.variableInfo.alias,
                                              &variable));
          leftOperand = &variable->exprBase;
          if(!clmDECL_IsPointerType(&variableName->u.variableInfo.alias->decl)) {
              gcmONERROR(cloIR_UNARY_EXPR_Construct(Compiler,
                                                    FuncCall->exprBase.base.lineNo,
                                                    FuncCall->exprBase.base.stringNo,
                                                    clvUNARY_ADDR,
                                                    leftOperand,
                                                    gcvNULL,
                                                    gcvNULL,
                                                    &leftOperand));
          }

          gcmONERROR(clConstructScalarIntegerConstant(Compiler,
                                                      FuncCall->exprBase.base.lineNo,
                                                      FuncCall->exprBase.base.stringNo,
                                                      variableName->u.variableInfo.aliasOffset,
                                                      &constant));
          rightOperand = &constant->exprBase;

          /* Create subscript expression */
          gcmONERROR(cloIR_BINARY_EXPR_Construct(Compiler,
                                                 FuncCall->exprBase.base.lineNo,
                                                 FuncCall->exprBase.base.stringNo,
                                                 clvBINARY_ADD,
                                                 leftOperand,
                                                 rightOperand,
                                                 &resExpr));

          gcmONERROR(cloIR_SET_AddMember(Compiler,
                                         argumentSet,
                                         &resExpr->base));
          argumentCount++;
       }
       else if(++argumentCount != operandCount) {
          status = gcvSTATUS_NOT_FOUND;
          goto OnError;
       }
       else {
          cloIR_VARIABLE variable;
          gcmONERROR(cloIR_VARIABLE_Construct(Compiler,
                                              FuncCall->exprBase.base.lineNo,
                                              FuncCall->exprBase.base.stringNo,
                                              variableName,
                                              &variable));

          gcmONERROR(cloIR_SET_AddMember(Compiler,
                                         argumentSet,
                                         &variable->exprBase.base));
          mapLastArg = gcvFALSE;
       }
   }
   funcNameLength = gcoOS_StrLen(FuncCall->funcSymbol, gcvNULL);
   if(mapLastArg) {
      funcNameLength += 3;
      nameSuffix = "#";
   }
   else {
      funcNameLength += 4;
      nameSuffix = "#1";
   }
   gcmONERROR(cloCOMPILER_Allocate(Compiler,
                                   funcNameLength,
                                   (gctPOINTER *) &symbol));

   gcmONERROR(gcoOS_PrintStrSafe(symbol,
                                 funcNameLength,
                                 &offset,
                                 "%s%s",
                                 FuncCall->funcSymbol,
                                 nameSuffix));

   gcmONERROR(cloCOMPILER_FindGeneralPoolString(Compiler,
                                                symbol,
                                                &symbolInPool));
   gcmONERROR(cloCOMPILER_Free(Compiler,
                               symbol));

   argumentSaved = FuncCall->operands;
   funcSymbolSaved = FuncCall->funcSymbol;
   funcNameSaved = FuncCall->funcName;
   FuncCall->operands = argumentSet;
   FuncCall->funcSymbol = symbolInPool;
   nameSpace = cloCOMPILER_GetBuiltinSpace(Compiler);
   while (nameSpace != gcvNULL) {
      FOR_EACH_DLINK_NODE(&nameSpace->names, clsNAME, name) {
         hasGenType = gcvFALSE;
         if (((name->type == clvFUNC_NAME) || (name->type == clvKERNEL_FUNC_NAME))
             && _IsCorrespondingFuncName(Compiler, name, FuncCall, &hasGenType, dataType)) {
             if (name->extension != clvEXTENSION_NONE) {
                if (!cloCOMPILER_ExtensionEnabled(Compiler, name->extension)) {
                   continue;
                }
             }
             /*found*/
             gcmVERIFY_OK(cloIR_OBJECT_Destroy(Compiler,
                                               &argumentSaved->base));
             *HasGenType = hasGenType;

/* update found function name */
             FuncCall->funcName = name;
             return gcvSTATUS_OK;
         }
      }
      nameSpace = nameSpace->parent;
   }
   status = gcvSTATUS_NOT_FOUND;
   FuncCall->operands = argumentSaved;
   FuncCall->funcSymbol = funcSymbolSaved;
   FuncCall->funcName = funcNameSaved;

OnError:
   if(argumentSet) {
      gcmVERIFY_OK(cloIR_OBJECT_Destroy(Compiler,
                                        &argumentSet->base));
   }
   return status;
}

#define _cldMangledNameBufferSize  1024

static gceSTATUS
_BindFuncName(
IN cloCOMPILER Compiler,
IN clsNAME_SPACE * NameSpace,
IN OUT cloIR_POLYNARY_EXPR PolynaryExpr
);


cltPOOL_STRING
clCreateMangledFuncName(
IN cloCOMPILER Compiler,
IN clsNAME *FuncName
)
{
    gceSTATUS status;
    cltPOOL_STRING symbol = gcvNULL;
    gctCHAR mangledNameBuffer[_cldMangledNameBufferSize] = {'\0'};
    clsNAME *paramName;
    clsBUILTIN_DATATYPE_INFO *typeInfo;
    gctBOOL firstTime = gcvTRUE;

    gcmVERIFY_OK(gcoOS_StrCopySafe(mangledNameBuffer, _cldMangledNameBufferSize, FuncName->symbol));
    gcmVERIFY_OK(gcoOS_StrCatSafe(mangledNameBuffer, _cldMangledNameBufferSize, "__"));

    FOR_EACH_DLINK_NODE(&FuncName->u.funcInfo.localSpace->names, struct _clsNAME, paramName) {
        clsNAME *derivedType;
        gctUINT offset, nameLength;

        if (paramName->type != clvPARAMETER_NAME) {
            if(firstTime) {
                typeInfo = clGetBuiltinDataTypeInfo(T_VOID);
                gcmVERIFY_OK(gcoOS_StrCatSafe(mangledNameBuffer, _cldMangledNameBufferSize, typeInfo->mangledName));
            }
            break;
        }
        if(clmDECL_IsPointerType(&paramName->decl)) {
            gcmVERIFY_OK(gcoOS_StrCatSafe(mangledNameBuffer, _cldMangledNameBufferSize, "P"));
        }

        offset = gcoOS_StrLen(mangledNameBuffer, gcvNULL);
        derivedType = clmNAME_DerivedType_GET(paramName);
        if(derivedType) {
            nameLength =  gcoOS_StrLen(derivedType->symbol, gcvNULL);
            switch(derivedType->type) {
            case clvTYPE_NAME:
                gcmVERIFY_OK(gcoOS_PrintStrSafe(mangledNameBuffer,
                                                _cldMangledNameBufferSize,
                                                &offset,
                                                "%d%s",
                                                nameLength, derivedType->symbol));
                break;
            case clvENUM_TAG_NAME:
                gcmVERIFY_OK(gcoOS_PrintStrSafe(mangledNameBuffer,
                                                _cldMangledNameBufferSize,
                                                &offset,
                                                "Te%d%s",
                                                nameLength, derivedType->symbol));
                break;
            default:
                gcmASSERT(0);
                break;
            }
        }
        else if(clmDECL_IsStructOrUnion(&paramName->decl)) {
            clsNAME_SPACE *nameSpace;

            nameSpace = paramName->decl.dataType->u.fieldSpace;
            nameLength =  gcoOS_StrLen(nameSpace->scopeName->symbol, gcvNULL);
            if(clmDATA_TYPE_IsStruct(paramName->decl.dataType)) {
                gcmVERIFY_OK(gcoOS_PrintStrSafe(mangledNameBuffer,
                                                _cldMangledNameBufferSize,
                                                &offset,
                                                "Ts%d%s",
                                                nameLength, nameSpace->scopeName->symbol));
            }
            else {
                gcmVERIFY_OK(gcoOS_PrintStrSafe(mangledNameBuffer,
                                                _cldMangledNameBufferSize,
                                                &offset,
                                                "Tu%d%s",
                                                nameLength, nameSpace->scopeName->symbol));
            }
        }
        else {
            typeInfo = clGetBuiltinDataTypeInfo(paramName->decl.dataType->type);
            gcmASSERT(typeInfo);
            gcmVERIFY_OK(gcoOS_PrintStrSafe(mangledNameBuffer,
                                            _cldMangledNameBufferSize,
                                            &offset,
                                            "%s",
                                            typeInfo->mangledName));
        }
        firstTime = gcvFALSE;
    }

    status = cloCOMPILER_AllocatePoolString(Compiler,
                                            mangledNameBuffer,
                                            &symbol);
    if (gcmIS_ERROR(status)) return gcvNULL;
    return symbol;
}

static clsNAME *
_CreateMangledBuiltinFuncName(
IN cloCOMPILER Compiler,
IN cloIR_POLYNARY_EXPR PolynaryExpr,
IN clsNAME *BuiltinName
)
{
   gceSTATUS status;
   cltPOOL_STRING symbol;
   clsBUILTIN_FUNCTION_INFO *functionInfo;

   if(BuiltinName->isBuiltin) {
       if (!cldBUILT_IN_NAME_MANGLING_ENABLED) return BuiltinName;
       functionInfo = clGetBuiltinFunctionInfo(PolynaryExpr->funcSymbol);
       gcmASSERT(functionInfo);
       if(functionInfo == gcvNULL) return gcvNULL;

       if(!functionInfo->nameMangled) return BuiltinName;

       symbol = clCreateMangledFuncName(Compiler,
                                        BuiltinName);
       if(!symbol) return gcvNULL;

       PolynaryExpr->funcSymbol = symbol;
       status = _BindFuncName(Compiler,
                              cloCOMPILER_GetGlobalSpace(Compiler),
                              PolynaryExpr);
       if(gcmIS_ERROR(status)) return gcvNULL;

       return PolynaryExpr->funcName;
   }
   return BuiltinName;
}

static gceSTATUS
_BindFuncName(
IN cloCOMPILER Compiler,
IN clsNAME_SPACE * NameSpace,
IN OUT cloIR_POLYNARY_EXPR PolynaryExpr
)
{
    gceSTATUS  status;
    clsNAME *name;
    clsDATA_TYPE dataType[1];
    gctBOOL hasGenType;
    gctBOOL nameCloned = gcvFALSE;

    FOR_EACH_DLINK_NODE(&NameSpace->names, clsNAME, name) {
        hasGenType = gcvFALSE;
        if (((name->type == clvFUNC_NAME) || (name->type == clvKERNEL_FUNC_NAME))
            && _IsCorrespondingFuncName(Compiler, name, PolynaryExpr, &hasGenType, dataType)) {
            clsDECL funcDecl[1];

            if (name->extension != clvEXTENSION_NONE) {
               if (!cloCOMPILER_ExtensionEnabled(Compiler, name->extension)) {
                   continue;
               }
            }

            dataType->accessQualifier = clvQUALIFIER_NONE; /* force data type creation if it is not already created */
            clmDECL_Initialize(funcDecl, dataType, &name->decl.array, name->decl.ptrDscr, gcvFALSE, clvSTORAGE_QUALIFIER_NONE);
            status = cloCOMPILER_CloneDecl(Compiler,
                            clvQUALIFIER_CONST,
                            name->decl.dataType->addrSpaceQualifier,
                            funcDecl,
                            &PolynaryExpr->exprBase.decl);
            if (gcmIS_ERROR(status)) return status;

            if(name->u.funcInfo.passArgByRef &&
               !clmDECL_IsPackedType(&PolynaryExpr->exprBase.decl)) {
               status = _MapFuncCallToPassByRef(Compiler,
                                                NameSpace,
                                                &hasGenType,
                                                PolynaryExpr);
               if (status == gcvSTATUS_OK) {
                  name = PolynaryExpr->funcName;
               }
            }

            if(name->isBuiltin) {
                if(hasGenType && name->u.funcInfo.refCount == 0) {
/** KLC - may want to rebind func name */
                     status = _CloneBuiltinFuncName(Compiler,
                                                    PolynaryExpr,
                                                    name->symbol,
                                                    name,
                                                    &name);
                     if (gcmIS_ERROR(status)) return status;
                     nameCloned = gcvTRUE;
                 }
                 if(!name->u.funcInfo.isIntrinsicCall &&
                    !name->u.funcInfo.isInline) {
#if cldNoInlineVectorToScalar
                     if(clmDECL_IsVectorType(&PolynaryExpr->exprBase.decl)) {
                         clsBUILTIN_FUNCTION_INFO *functionInfo;

                         functionInfo = clGetBuiltinFunctionInfo(PolynaryExpr->funcSymbol);
                         if(functionInfo == gcvNULL) return gcvSTATUS_INVALID_ARGUMENT;
                         if(functionInfo->handleVector) {
                             cloIR_POLYNARY_EXPR scalarFuncCall;

                             status = cloIR_ScalarizeFuncCall(Compiler,
                                                              PolynaryExpr,
                                                              name,
                                                              gcvFALSE,
                                                              &scalarFuncCall);
                             if (gcmIS_ERROR(status)) return status;
                             if(scalarFuncCall->funcName->u.funcInfo.refCount == 1) {
                                status = cloCOMPILER_AddReferencedBuiltinFunc(Compiler,
                                                                              scalarFuncCall);
                                if (gcmIS_ERROR(status)) return status;
                             }

                             scalarFuncCall->funcName->u.funcInfo.refCount += 1;
                         }
                     }
#endif
                     if(name->u.funcInfo.refCount == 1) {
                          if(!PolynaryExpr->funcName)PolynaryExpr->funcName = name;
                          status = cloCOMPILER_AddReferencedBuiltinFunc(Compiler,
                                                                        PolynaryExpr);
                          if (gcmIS_ERROR(status)) return status;
                     }
                 }
             }
             if(name->u.funcInfo.mangledName == gcvNULL) {
                 name = _CreateMangledBuiltinFuncName(Compiler,
                                                      PolynaryExpr,
                                                      name);
                 if(!name) return gcvSTATUS_NOT_FOUND;
             }

             name->u.funcInfo.refCount += 1;
             PolynaryExpr->funcName = name;
             if(nameCloned) return gcvSTATUS_NAME_NOT_FOUND;
             else return gcvSTATUS_OK;
        }
    }

    if (NameSpace->parent != gcvNULL) {
        return _BindFuncName(Compiler,
                             NameSpace->parent,
                             PolynaryExpr);
    }
    return gcvSTATUS_NOT_FOUND;
}

gceSTATUS
clsNAME_SPACE_BindFuncName(
IN cloCOMPILER Compiler,
IN clsNAME_SPACE * NameSpace,
IN OUT cloIR_POLYNARY_EXPR PolynaryExpr
)
{
    gceSTATUS  status;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    gcmASSERT(NameSpace);

    status = _BindFuncName(Compiler,
                   NameSpace,
                   PolynaryExpr);

    if (gcmIS_ERROR(status)) {
       cloCOMPILER_Report(Compiler,
                  PolynaryExpr->exprBase.base.lineNo,
                  PolynaryExpr->exprBase.base.stringNo,
                  clvREPORT_ERROR,
                  "function: '%s' hasn't the corresponding declaration",
                  PolynaryExpr->funcSymbol);
       return gcvSTATUS_INVALID_ARGUMENT;
    }
    return status;
}

gceSTATUS
cloIR_ScalarizeFuncCall(
IN cloCOMPILER Compiler,
IN cloIR_POLYNARY_EXPR VectorFuncCall,
IN clsNAME *RefFuncName,
IN gctBOOL IsLookUp,
OUT cloIR_POLYNARY_EXPR *ScalarFuncCall
)
{
   gceSTATUS status = gcvSTATUS_OK;
   cloIR_POLYNARY_EXPR    scalarFuncCall;
   cloIR_EXPR argument;
   cloIR_UNARY_EXPR nullExpr;
   clsDECL funcDecl[1];
   clsDECL argDecl[1];
   clsDECL *declPtr;

   gcmASSERT(VectorFuncCall);
   gcmASSERT(ScalarFuncCall);
   *ScalarFuncCall = gcvNULL;

   declPtr = &VectorFuncCall->exprBase.decl;
   if(!clmDECL_IsVoid(declPtr) &&
      (clmDECL_IsVectorType(declPtr) ||
       clmDECL_IsVectorPointerType(declPtr))) {
      clsDATA_TYPE *scalarDataType;

      status =  cloIR_CreateVectorType(Compiler,
                                       declPtr->dataType,
                                       1,
                                       &scalarDataType);
      if (gcmIS_ERROR(status)) return status;

      clmDECL_Initialize(funcDecl, scalarDataType, (clsARRAY *)0, gcvNULL, gcvFALSE, clvSTORAGE_QUALIFIER_NONE);
      status = cloCOMPILER_ClonePtrDscr(Compiler,
                    declPtr->ptrDscr,
                    &funcDecl->ptrDscr);
      if (gcmIS_ERROR(status)) return status;
      declPtr = funcDecl;
   }

   /* Create a new polynary expression */
   status = cloIR_POLYNARY_EXPR_Construct(Compiler,
                                          VectorFuncCall->exprBase.base.lineNo,
                                          VectorFuncCall->exprBase.base.stringNo,
                                          clvPOLYNARY_FUNC_CALL,
                                          declPtr,
                                          VectorFuncCall->funcSymbol,
                                          &scalarFuncCall);
   if (gcmIS_ERROR(status)) return status;

   status = cloIR_SET_Construct(Compiler,
                                VectorFuncCall->exprBase.base.lineNo,
                                VectorFuncCall->exprBase.base.stringNo,
                                clvEXPR_SET,
                                &scalarFuncCall->operands);
   if (gcmIS_ERROR(status)) return status;

   gcmASSERT(scalarFuncCall->operands);

   FOR_EACH_DLINK_NODE(&VectorFuncCall->operands->members, struct _cloIR_EXPR, argument) {
      if(!clmDECL_IsVoid(&argument->decl) &&
         (clmDECL_IsVectorType(&argument->decl) ||
          clmDECL_IsVectorPointerType(&argument->decl))) {
         clsDATA_TYPE *scalarDataType;

         status =  cloIR_CreateVectorType(Compiler,
                                          argument->decl.dataType,
                                          1,
                                          &scalarDataType);
         if (gcmIS_ERROR(status)) return status;
         clmDECL_Initialize(argDecl, scalarDataType, (clsARRAY *)0, gcvNULL, gcvFALSE, clvSTORAGE_QUALIFIER_NONE);
         status = cloCOMPILER_ClonePtrDscr(Compiler,
                       argument->decl.ptrDscr,
                       &argDecl->ptrDscr);
         if (gcmIS_ERROR(status)) return status;
         declPtr = argDecl;
      }
      else {
         declPtr = &argument->decl;
      }
      status = cloIR_NULL_EXPR_Construct(Compiler,
                                         VectorFuncCall->exprBase.base.lineNo,
                                         VectorFuncCall->exprBase.base.stringNo,
                                         declPtr,
                                         &nullExpr);
      if (gcmIS_ERROR(status)) return status;
      gcmVERIFY_OK(cloIR_SET_AddMember(Compiler,
                                       scalarFuncCall->operands,
                                       &nullExpr->exprBase.base));
   }

   status = _FindFuncName(Compiler,
                          cloCOMPILER_GetBuiltinSpace(Compiler),
                          scalarFuncCall);
   if(!IsLookUp) {
      if(status == gcvSTATUS_NOT_FOUND) {
         clsNAME *paramName;

         /*inherit the type from the scalarized function call */
         argument = (cloIR_EXPR)scalarFuncCall->operands->members.next;
         FOR_EACH_DLINK_NODE(&RefFuncName->u.funcInfo.localSpace->names, struct _clsNAME, paramName) {
           if(!clmDECL_IsVoid(&paramName->decl) &&
              (clmDECL_IsVectorType(&paramName->decl) ||
              clmDECL_IsVectorPointerType(&paramName->decl))) {
              paramName->u.variableInfo.effectiveDecl = argument->decl;
           }
           else {
              paramName->u.variableInfo.effectiveDecl = paramName->decl;
           }
           argument = (cloIR_EXPR)((slsDLINK_NODE *)argument)->next;
         }

         status = _CloneBuiltinFuncName(Compiler,
                                        scalarFuncCall,
                                        RefFuncName->symbol,
                                        RefFuncName,
                                        &scalarFuncCall->funcName);
      }
      else if(status == gcvSTATUS_NAME_NOT_FOUND) { /* name was cloned */
         scalarFuncCall->funcName->u.funcInfo.refCount = 0;
         status = gcvSTATUS_OK;
      }
   }

   if (gcmIS_ERROR(status)) return status;
   *ScalarFuncCall = scalarFuncCall;
   return status;
}

static gceSTATUS
_CheckNameScope(
IN cloCOMPILER Compiler,
IN gctUINT LineNo,
IN gctUINT StringNo,
IN clsNAME_SPACE *NameSpace,
IN clsDECL *Decl,
IN slsSLINK_LIST *PtrDscr
)
{
   if(Decl == gcvNULL) {
     return gcvSTATUS_OK;
   }

   if(cloCOMPILER_IsNameSpaceUnnamed(Compiler,
                                     NameSpace)) return gcvSTATUS_OK;

   if(cloCOMPILER_GetParserState(Compiler) != clvPARSER_IN_TYPEDEF) {
      if(cloCOMPILER_GetGlobalSpace(Compiler) == NameSpace) { /* Name space is global */
         if(Decl->dataType->addrSpaceQualifier != clvQUALIFIER_CONSTANT &&
            Decl->dataType->accessQualifier != clvQUALIFIER_UNIFORM) {
             gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                             LineNo,
                                             StringNo,
                                             clvREPORT_ERROR,
                                             "program scope variable not in constant address space"));
             return gcvSTATUS_INVALID_ARGUMENT;
         }
         else return gcvSTATUS_OK;
      }
   }

   if(clmDECL_IsPointerType(Decl) || PtrDscr ||
      clmDECL_IsArrayOfPointers(Decl)) {
      return gcvSTATUS_OK;
   }
   switch(Decl->dataType->addrSpaceQualifier) {
   case clvQUALIFIER_LOCAL:
     if(NameSpace->scopeName == gcvNULL) {
        if(!gcmOPT_oclOcvLocalAddressSpace()) {
            gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                            LineNo,
                                            StringNo,
                                            clvREPORT_ERROR,
                                            "local variable not in kernel function scope"));
            return gcvSTATUS_INVALID_ARGUMENT;
        }
     }
     else if(NameSpace->scopeName->type != clvKERNEL_FUNC_NAME) {
        return gcvSTATUS_INVALID_ARGUMENT;
     }
     else {
        NameSpace->scopeName->u.funcInfo.needLocalMemory = gcvTRUE;
        cloCOMPILER_SetNeedLocalMemory(Compiler);
     }
     break;

   case clvQUALIFIER_GLOBAL:
     gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                     LineNo,
                                     StringNo,
                                     clvREPORT_ERROR,
                                     "invalid global address space qualifier specified for variable type"));
     return gcvSTATUS_INVALID_ARGUMENT;

   default:
     break;
   }
   return gcvSTATUS_OK;
}

/* klc - to be completed */
gceSTATUS
clsNAME_SPACE_ReleaseName(
IN cloCOMPILER Compiler,
IN clsNAME_SPACE * NameSpace,
IN clsNAME *Name
)
{
   return gcvSTATUS_OK;
}

gceSTATUS
clsNAME_SPACE_CreateName(
IN cloCOMPILER Compiler,
IN clsNAME_SPACE * NameSpace,
IN gctUINT LineNo,
IN gctUINT StringNo,
IN cleNAME_TYPE Type,
IN clsDECL *Decl,
IN cltPOOL_STRING Symbol,
IN slsSLINK_LIST *PtrDscr,
IN gctBOOL IsBuiltin,
IN cleEXTENSION Extension,
OUT clsNAME **Name
)
{
    gceSTATUS status;
    clsNAME *name;
    cltPOOL_STRING symbol;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    gcmASSERT(NameSpace);

    symbol = Symbol;
    switch (Type) {
    case clvFUNC_NAME:
    case clvKERNEL_FUNC_NAME:

    case clvLABEL_NAME:
       break;

    default:
       if(Decl != gcvNULL && Decl->dataType != gcvNULL &&
          slmSLINK_LIST_IsEmpty(PtrDscr) &&
          clmDECL_IsVoid(Decl)) {
            if (Type != clvPARAMETER_NAME) {
                gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                                LineNo,
                                                StringNo,
                                                clvREPORT_ERROR,
                                                "\"%s\" can not use the void type",
                                                Symbol));
            }
            else if(!clmHasRightLanguageVersion(Compiler, _cldCL1Dot2)) {
                gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                                LineNo,
                                                StringNo,
                                                clvREPORT_ERROR,
                                                "the parameter declaration can not use the void type"));
                status = gcvSTATUS_INVALID_ARGUMENT;
                goto OnError;
            }
        }

        if(Symbol[0] != '\0') {
            symbol = Symbol;
            if(Type == clvSTRUCT_NAME || Type == clvUNION_NAME) {
                gctSIZE_T length, prefixLength;
                gctSTRING prefix;
                gctSTRING nameBuffer = gcvNULL;
                gctPOINTER pointer;

                if(Type == clvSTRUCT_NAME)
                {
                    prefix = cldSTRUCT_NAME_PREFIX;
                }
                else prefix = cldUNION_NAME_PREFIX;

                gcoOS_StrLen(prefix, &prefixLength);
                gcoOS_StrLen(Symbol, &length);

                length += prefixLength + 1;
                status = cloCOMPILER_Allocate(Compiler,
                                              length,
                                              &pointer);
                if (gcmIS_ERROR(status))  return status;
                nameBuffer = pointer;

                gcmVERIFY_OK(gcoOS_StrCopySafe(nameBuffer, length, prefix));
                gcmVERIFY_OK(gcoOS_StrCatSafe(nameBuffer, length, Symbol));

                status = cloCOMPILER_AllocatePoolString(Compiler,
                                                        nameBuffer,
                                                        &symbol);
                gcmVERIFY_OK(cloCOMPILER_Free(Compiler, pointer));
                if (gcmIS_ERROR(status)) return status;
            }

            if (!cloCOMPILER_IsLoadingBuiltin(Compiler))
            {
                status = clsNAME_SPACE_Search(Compiler,
                                              NameSpace,
                                              symbol,
                                              gcvFALSE,
                                              &name);

                if (status == gcvSTATUS_OK) {
                    gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                                    LineNo,
                                                    StringNo,
                                                    clvREPORT_ERROR,
                                                    "redefined identifier: '%s'",
                                                    Symbol));

                    status = gcvSTATUS_INVALID_ARGUMENT;
                    goto OnError;
                }
            }

            if(Type == clvVARIABLE_NAME) {
                gcmONERROR(_CheckNameScope(Compiler,
                                           LineNo,
                                           StringNo,
                                           NameSpace,
                                           Decl,
                                           PtrDscr));
            }
        }
    }
    /* Create a new name */
    do {
        gcmONERROR(_clsNAME_Construct(Compiler,
                                      NameSpace,
                                      LineNo,
                                      StringNo,
                                      Type,
                                      Decl,
                                      symbol,
                                      PtrDscr,
                                      IsBuiltin,
                                      Extension,
                                      &name));

        if((Type == clvSTRUCT_NAME ||
            Type == clvUNION_NAME ||
            Type == clvENUM_TAG_NAME) &&
            Symbol[0] == '\0') {
            clsNAME_SPACE *unnamedSpace;
            unnamedSpace = cloCOMPILER_GetUnnamedSpace(Compiler);
            gcmASSERT(unnamedSpace);
            slsDLINK_LIST_InsertLast(&unnamedSpace->names, &name->node);
        }
        else {
           slsDLINK_LIST_InsertLast(&NameSpace->names, &name->node);
        }
        if (Name != gcvNULL) {
            name->die = cloCOMPILER_AddDIEWithName(Compiler, name);
            *Name = name;
        }

        return gcvSTATUS_OK;
    } while (gcvFALSE);
OnError:
    if (Name != gcvNULL) *Name = gcvNULL;
    return status;
}

/* cloIR_BASE object. */
static gctCONST_STRING
_GetIRObjectTypeName(
IN cleIR_OBJECT_TYPE IRObjectType
)
{
    switch (IRObjectType) {
    case clvIR_SET:            return "IR_SET";
    case clvIR_ITERATION:        return "IR_ITERATION";
    case clvIR_JUMP:        return "IR_JUMP";
    case clvIR_LABEL:        return "IR_LABEL";
    case clvIR_VARIABLE:        return "IR_VARIABLE";
    case clvIR_CONSTANT:        return "IR_CONSTANT";
    case clvIR_UNARY_EXPR:        return "IR_UNARY_EXPR";
    case clvIR_BINARY_EXPR:        return "IR_BINARY_EXPR";
    case clvIR_SELECTION:        return "IR_SELECTION";
    case clvIR_POLYNARY_EXPR:    return "IR_POLYNARY_EXPR";
    case clvIR_TYPECAST_ARGS:    return "IR_TYPECAST_ARGS";

    default:
        gcmASSERT(0);
        return "invalid";
    }
}

gceSTATUS
cloIR_BASE_Dump(
IN cloCOMPILER Compiler,
IN cloIR_BASE This
)
{
    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(This, clvIR_SET);

    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                      clvDUMP_IR,
                      "<IR_BASE line=\"%d\" string=\"%d\" realType=\"%s\" />",
                      This->lineNo,
                      This->stringNo,
                      _GetIRObjectTypeName(This->vptr->type)));
    return gcvSTATUS_OK;
}

/* cloIR_SET object. */
gceSTATUS
cloIR_SET_Destroy(
IN cloCOMPILER Compiler,
IN cloIR_BASE This
)
{
    cloIR_SET    set = (cloIR_SET)This;
    slsDLINK_LIST *    members;
    cloIR_BASE    member;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(set, clvIR_SET);

    members = &set->members;

    while (!slsDLINK_LIST_IsEmpty(members)) {
        slsDLINK_LIST_DetachFirst(members, struct _cloIR_BASE, &member);

        gcmVERIFY_OK(cloIR_OBJECT_Destroy(Compiler, member));
    }

    gcmVERIFY_OK(cloCOMPILER_Free(Compiler, set));
    return gcvSTATUS_OK;
}

/* Empty a cloIR_SET object. */
gceSTATUS
cloIR_SET_Empty(
IN cloCOMPILER Compiler,
IN cloIR_BASE This
)
{
    cloIR_SET    set = (cloIR_SET)This;
    slsDLINK_LIST *    members;
    cloIR_BASE    member;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(set, clvIR_SET);

    members = &set->members;

    while (!slsDLINK_LIST_IsEmpty(members)) {
        slsDLINK_LIST_DetachFirst(members, struct _cloIR_BASE, &member);

        gcmVERIFY_OK(cloIR_OBJECT_Destroy(Compiler, member));
    }

    return gcvSTATUS_OK;
}

/* cloIR_BASE object. */
static gctCONST_STRING
_GetIRSetTypeName(
IN cleSET_TYPE SetType
)
{
    switch (SetType) {
    case clvDECL_SET:    return "declSet";
    case clvSTATEMENT_SET:    return "statementSet";
    case clvEXPR_SET:    return "exprSet";

    default:
        gcmASSERT(0);
        return "invalid";
    }
}

gceSTATUS
cloIR_SET_Dump(
IN cloCOMPILER Compiler,
IN cloIR_BASE This
)
{
    cloIR_SET    set = (cloIR_SET)This;
    cloIR_BASE    member;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(set, clvIR_SET);

    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                      clvDUMP_IR,
                      "<IR_SET this=\"0x%x\" line=\"%d\" string=\"%d\""
                      " type=\"%s\" asFunc=\"%s\">",
                      set,
                      set->base.lineNo,
                      set->base.stringNo,
                      _GetIRSetTypeName(set->type),
                      (set->funcName == gcvNULL)? "none" : set->funcName->symbol));

    if (set->funcName != gcvNULL) {
        gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                          clvDUMP_IR,
                          "<!-- Function Name -->"));

        gcmVERIFY_OK(clsNAME_Dump(Compiler, set->funcName));
    }

    FOR_EACH_DLINK_NODE(&set->members, struct _cloIR_BASE, member) {
        gcmVERIFY_OK(cloIR_OBJECT_Dump(Compiler, member));
    }

    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                      clvDUMP_IR,
                      "</IR_SET>"));
    return gcvSTATUS_OK;
}

gceSTATUS
cloIR_SET_Accept(
IN cloCOMPILER Compiler,
IN cloIR_BASE This,
IN clsVISITOR * Visitor,
IN OUT gctPOINTER Parameters
)
{
    cloIR_SET    set = (cloIR_SET)This;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(set, clvIR_SET);
    gcmASSERT(Visitor);

    if (Visitor->visitSet == gcvNULL) return gcvSTATUS_OK;

    return Visitor->visitSet(Compiler, Visitor, set, Parameters);
}

static clsVTAB s_setVTab =
{
    clvIR_SET,
    cloIR_SET_Destroy,
    cloIR_SET_Dump,
    cloIR_SET_Accept
};

gceSTATUS
cloIR_SET_Construct(
IN cloCOMPILER Compiler,
IN gctUINT LineNo,
IN gctUINT StringNo,
IN cleSET_TYPE Type,
OUT cloIR_SET * Set
)
{
    gceSTATUS status;
    cloIR_SET set;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    gcmASSERT(Set);

    do {
        status = cloCOMPILER_Allocate(Compiler,
                          (gctSIZE_T)sizeof(struct _cloIR_SET),
                          (gctPOINTER *) &set);
        if (gcmIS_ERROR(status)) break;

        cloIR_BASE_Initialize(&set->base, &s_setVTab, LineNo, StringNo);
        set->type        = Type;
        slsDLINK_LIST_Initialize(&set->members);

        set->funcName    = gcvNULL;
        *Set = set;
        return gcvSTATUS_OK;
    } while (gcvFALSE);

    *Set = gcvNULL;
    return status;
}

gceSTATUS
cloIR_SET_AddMember(
    IN cloCOMPILER Compiler,
    IN cloIR_SET Set,
    IN cloIR_BASE Member
    )
{
    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(Set, clvIR_SET);

    slsDLINK_LIST_InsertLast(&Set->members, &Member->node);

    return gcvSTATUS_OK;
}

gceSTATUS
cloIR_SET_GetMember(
    IN cloCOMPILER Compiler,
    IN cloIR_SET Set,
    gctUINT I,
    cloIR_BASE *Member
    )
{
   gctUINT count = 0;
   cloIR_BASE member;

   /* Verify the arguments. */
   clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
   clmVERIFY_IR_OBJECT(Set, clvIR_SET);

   gcmASSERT(Member);
   FOR_EACH_DLINK_NODE(&Set->members, struct _cloIR_BASE, member) {
      count++;
      if(I == count) {
         *Member = member;
         return gcvSTATUS_OK;
      }
   }

   *Member = gcvNULL;
   return gcvSTATUS_NOT_FOUND;
}

gceSTATUS
cloIR_SET_GetMemberCount(
    IN cloCOMPILER Compiler,
    IN cloIR_SET Set,
    OUT gctUINT * MemberCount
    )
{
    gctUINT        count = 0;
    cloIR_BASE    member;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(Set, clvIR_SET);

    FOR_EACH_DLINK_NODE(&Set->members, struct _cloIR_BASE, member)
    {
        count++;
    }

    *MemberCount = count;

    return gcvSTATUS_OK;
}

gctBOOL
cloIR_SET_IsEmpty(
    IN cloCOMPILER Compiler,
    IN cloIR_SET Set
    )
{
/* Verify the arguments. */
  clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
  clmVERIFY_IR_OBJECT(Set, clvIR_SET);

  return slsDLINK_LIST_IsEmpty(&Set->members);
}

/* cloIR_ITERATION object. */
gceSTATUS
cloIR_ITERATION_Destroy(
IN cloCOMPILER Compiler,
IN cloIR_BASE This
)
{
    cloIR_ITERATION    iteration = (cloIR_ITERATION)This;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(iteration, clvIR_ITERATION);

    if (iteration->condExpr != gcvNULL) {
        gcmVERIFY_OK(cloIR_OBJECT_Destroy(Compiler, &iteration->condExpr->base));
    }

    if (iteration->loopBody != gcvNULL) {
        gcmVERIFY_OK(cloIR_OBJECT_Destroy(Compiler, iteration->loopBody));
    }

    if (iteration->forInitStatement != gcvNULL) {
        gcmVERIFY_OK(cloIR_OBJECT_Destroy(Compiler, iteration->forInitStatement));
    }

    if (iteration->forRestExpr != gcvNULL) {
        gcmVERIFY_OK(cloIR_OBJECT_Destroy(Compiler, &iteration->forRestExpr->base));
    }

    gcmVERIFY_OK(cloCOMPILER_Free(Compiler, iteration));
    return gcvSTATUS_OK;
}

static gctCONST_STRING
_GetIRIterationTypeName(
IN cleITERATION_TYPE IterationType
)
{
    switch (IterationType) {
    case clvFOR:        return "for";
    case clvWHILE:        return "while";
    case clvDO_WHILE:    return "do-while";

    default:
        gcmASSERT(0);
        return "invalid";
    }
}

gceSTATUS
cloIR_ITERATION_Dump(
IN cloCOMPILER Compiler,
IN cloIR_BASE This
)
{
    cloIR_ITERATION    iteration = (cloIR_ITERATION)This;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(iteration, clvIR_ITERATION);

    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                      clvDUMP_IR,
                      "<IR_ITERATION line=\"%d\" string=\"%d\" type=\"%s\"",
                      iteration->base.lineNo,
                      iteration->base.stringNo,
                      _GetIRIterationTypeName(iteration->type)));

    if (iteration->forSpace != gcvNULL) {
        gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                          clvDUMP_IR,
                          " forSpace=\"0x%x\"",
                          iteration->forSpace));
    }

    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                      clvDUMP_IR,
                      ">"));

    if (iteration->condExpr != gcvNULL) {
        gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                          clvDUMP_IR,
                          "<!-- Condition Expression -->"));
        gcmVERIFY_OK(cloIR_OBJECT_Dump(Compiler, &iteration->condExpr->base));
    }

    if (iteration->loopBody != gcvNULL) {
        gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                          clvDUMP_IR,
                          "<!-- Loop Body -->"));
        gcmVERIFY_OK(cloIR_OBJECT_Dump(Compiler, iteration->loopBody));
    }

    if (iteration->forInitStatement != gcvNULL) {
        gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                          clvDUMP_IR,
                          "<!-- For Init Statement -->"));
        gcmVERIFY_OK(cloIR_OBJECT_Dump(Compiler, iteration->forInitStatement));
    }

    if (iteration->forRestExpr != gcvNULL) {
        gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                          clvDUMP_IR,
                          "<!-- For Rest Expression -->"));
        gcmVERIFY_OK(cloIR_OBJECT_Dump(Compiler, &iteration->forRestExpr->base));
    }

    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                      clvDUMP_IR,
                      "</IR_ITERATION>"));
    return gcvSTATUS_OK;
}

gceSTATUS
cloIR_ITERATION_Accept(
IN cloCOMPILER Compiler,
IN cloIR_BASE This,
IN clsVISITOR * Visitor,
IN OUT gctPOINTER Parameters
)
{
    cloIR_ITERATION    iteration = (cloIR_ITERATION)This;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(iteration, clvIR_ITERATION);
    gcmASSERT(Visitor);

    if (Visitor->visitIteration == gcvNULL) return gcvSTATUS_OK;

    return Visitor->visitIteration(Compiler, Visitor, iteration, Parameters);
}

static clsVTAB s_iterationVTab =
{
    clvIR_ITERATION,
    cloIR_ITERATION_Destroy,
    cloIR_ITERATION_Dump,
    cloIR_ITERATION_Accept
};

gceSTATUS
cloIR_ITERATION_Construct(
IN cloCOMPILER Compiler,
IN gctUINT LineNo,
IN gctUINT StringNo,
IN cleITERATION_TYPE Type,
IN cloIR_EXPR CondExpr,
IN cloIR_BASE LoopBody,
IN clsNAME_SPACE * ForSpace,
IN cloIR_BASE ForInitStatement,
IN cloIR_EXPR ForRestExpr,
OUT cloIR_ITERATION * Iteration
)
{
    gceSTATUS    status;
    cloIR_ITERATION    iteration;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);

    do {
        status = cloCOMPILER_Allocate(Compiler,
                          (gctSIZE_T)sizeof(struct _cloIR_ITERATION),
                          (gctPOINTER *) &iteration);
        if (gcmIS_ERROR(status)) break;

        cloIR_BASE_Initialize(&iteration->base, &s_iterationVTab, LineNo, StringNo);

        iteration->type            = Type;
        iteration->condExpr        = CondExpr;
        iteration->loopBody        = LoopBody;
        iteration->forSpace        = ForSpace;
        iteration->forInitStatement    = ForInitStatement;
        iteration->forRestExpr        = ForRestExpr;

        if (CondExpr)
            iteration->base.endLineNo = gcmMAX(CondExpr->base.endLineNo,iteration->base.endLineNo);
        if (LoopBody)
            iteration->base.endLineNo = gcmMAX(LoopBody->endLineNo,iteration->base.endLineNo);
        else if (ForInitStatement)
            iteration->base.endLineNo = gcmMAX(ForInitStatement->endLineNo,iteration->base.endLineNo);

        *Iteration = iteration;
        return gcvSTATUS_OK;
    } while (gcvFALSE);

    *Iteration = gcvNULL;
    return status;
}

/* cloIR_JUMP object. */
gceSTATUS
cloIR_JUMP_Destroy(
IN cloCOMPILER Compiler,
IN cloIR_BASE This
)
{
    cloIR_JUMP    jump = (cloIR_JUMP)This;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(jump, clvIR_JUMP);

    if (jump->type == clvRETURN &&
        jump->u.returnExpr != gcvNULL) {
        gcmVERIFY_OK(cloIR_OBJECT_Destroy(Compiler,
                          &jump->u.returnExpr->base));
    }

    gcmVERIFY_OK(cloCOMPILER_Free(Compiler, jump));
    return gcvSTATUS_OK;
}

gctCONST_STRING
clGetIRJumpTypeName(
IN cleJUMP_TYPE JumpType
)
{
    switch (JumpType) {
    case clvCONTINUE:    return "continue";
    case clvBREAK:        return "break";
    case clvRETURN:        return "return";
    case clvGOTO:        return "goto";

    default:
        gcmASSERT(0);
        return "invalid";
    }
}

gceSTATUS
cloIR_JUMP_Dump(
IN cloCOMPILER Compiler,
IN cloIR_BASE This
)
{
    cloIR_JUMP    jump = (cloIR_JUMP)This;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(jump, clvIR_JUMP);

    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                      clvDUMP_IR,
                      "<IR_JUMP line=\"%d\" string=\"%d\" type=\"%s\">",
                      jump->base.lineNo,
                      jump->base.stringNo,
                      clGetIRJumpTypeName(jump->type)));

    if (jump->type == clvRETURN) {
        if (jump->u.returnExpr != gcvNULL) {
            gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                              clvDUMP_IR,
                              "<!-- Return Expression -->"));

            gcmVERIFY_OK(cloIR_OBJECT_Dump(Compiler, &jump->u.returnExpr->base));
        }
    }

    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                      clvDUMP_IR,
                      "</IR_JUMP>"));
    return gcvSTATUS_OK;
}

gceSTATUS
cloIR_JUMP_Accept(
IN cloCOMPILER Compiler,
IN cloIR_BASE This,
IN clsVISITOR * Visitor,
IN OUT gctPOINTER Parameters
)
{
    cloIR_JUMP    jump = (cloIR_JUMP)This;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(jump, clvIR_JUMP);
    gcmASSERT(Visitor);

    if (Visitor->visitJump == gcvNULL) return gcvSTATUS_OK;

    return Visitor->visitJump(Compiler, Visitor, jump, Parameters);
}

static clsVTAB s_jumpVTab =
{
    clvIR_JUMP,
    cloIR_JUMP_Destroy,
    cloIR_JUMP_Dump,
    cloIR_JUMP_Accept
};

gceSTATUS
cloIR_JUMP_Construct(
IN cloCOMPILER Compiler,
IN gctUINT LineNo,
IN gctUINT StringNo,
IN cleJUMP_TYPE Type,
IN cloIR_EXPR ReturnExpr,
OUT cloIR_JUMP *Jump
)
{
    gceSTATUS    status;
    cloIR_JUMP    jump;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);

    do {
        status = cloCOMPILER_ZeroMemoryAllocate(Compiler,
                          (gctSIZE_T)sizeof(struct _cloIR_JUMP),
                          (gctPOINTER *) &jump);
        if (gcmIS_ERROR(status)) break;

        cloIR_BASE_Initialize(&jump->base, &s_jumpVTab, LineNo, StringNo);

        jump->type    = Type;
        jump->u.returnExpr= ReturnExpr;
        *Jump = jump;
        return gcvSTATUS_OK;
    } while (gcvFALSE);

    *Jump = gcvNULL;
    return status;
}

gceSTATUS
cloIR_GOTO_Construct(
IN cloCOMPILER Compiler,
IN gctUINT LineNo,
IN gctUINT StringNo,
IN clsNAME *Label,
OUT cloIR_JUMP *GotoStmt
)
{
    gceSTATUS    status;
    cloIR_JUMP    jump;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);

    do {
        status = cloCOMPILER_Allocate(Compiler,
                          (gctSIZE_T)sizeof(struct _cloIR_JUMP),
                          (gctPOINTER *) &jump);
        if (gcmIS_ERROR(status)) break;

        cloIR_BASE_Initialize(&jump->base, &s_jumpVTab, LineNo, StringNo);

        Label->u.labelInfo.isReferenced = gcvTRUE;
        jump->type = clvGOTO;
        jump->u.label = Label;
        jump->nameSpace = cloCOMPILER_GetCurrentSpace(Compiler);
        *GotoStmt = jump;
        return gcvSTATUS_OK;
    } while (gcvFALSE);

    *GotoStmt = gcvNULL;
    return status;
}

/* cloIR_LABEL object. */
gceSTATUS
cloIR_LABEL_Destroy(
IN cloCOMPILER Compiler,
IN cloIR_BASE This
)
{
    cloIR_LABEL    label = (cloIR_LABEL)This;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(label, clvIR_LABEL);

    gcmVERIFY_OK(cloCOMPILER_Free(Compiler, label));
    return gcvSTATUS_OK;
}

gceSTATUS
cloIR_LABEL_Dump(
IN cloCOMPILER Compiler,
IN cloIR_BASE This
)
{
    cloIR_LABEL label = (cloIR_LABEL)This;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(label, clvIR_LABEL);

        switch(label->type) {
    case clvNAMED:
       gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                         clvDUMP_IR,
                         "<IR_LABEL line=\"%d\" string=\"%d\" type=\"%s\">",
                         label->base.lineNo,
                         label->base.stringNo,
                         label->u.name->symbol));
       break;
    case clvCASE:
       gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                         clvDUMP_IR,
                         "<IR_LABEL line=\"%d\" string=\"%d\" type=\"%s\" %d:>",
                         label->base.lineNo,
                         label->base.stringNo,
                         "case", label->caseValue));
       break;

    case clvDEFAULT:
       gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                         clvDUMP_IR,
                         "<IR_LABEL line=\"%d\" string=\"%d\" type=\"%s\">",
                         label->base.lineNo,
                         label->base.stringNo,
                         "default:"));
       break;
    }

    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                      clvDUMP_IR,
                      "</IR_LABEL>"));
    return gcvSTATUS_OK;
}

gceSTATUS
cloIR_LABEL_Accept(
IN cloCOMPILER Compiler,
IN cloIR_BASE This,
IN clsVISITOR * Visitor,
IN OUT gctPOINTER Parameters
)
{
    cloIR_LABEL  label = (cloIR_LABEL)This;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(label, clvIR_LABEL);
    gcmASSERT(Visitor);

    if (Visitor->visitLabel == gcvNULL) return gcvSTATUS_OK;

    return Visitor->visitLabel(Compiler, Visitor, label, Parameters);
}

static clsVTAB s_LabelVTab =
{
    clvIR_LABEL,
    cloIR_LABEL_Destroy,
    cloIR_LABEL_Dump,
    cloIR_LABEL_Accept
};

void
cloIR_LABEL_Initialize(
IN gctUINT LineNo,
IN gctUINT StringNo,
IN OUT cloIR_LABEL Label
)
{
    gcmASSERT(Label);

          (void)gcoOS_ZeroMemory((gctPOINTER)Label, sizeof(struct _cloIR_LABEL));
    cloIR_BASE_Initialize(&Label->base, &s_LabelVTab, LineNo, StringNo);
}

gceSTATUS
cloIR_LABEL_Construct(
IN cloCOMPILER Compiler,
IN gctUINT LineNo,
IN gctUINT StringNo,
OUT cloIR_LABEL *Label
)
{
    gceSTATUS    status;
    cloIR_LABEL    label;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);

    do {
        status = cloCOMPILER_Allocate(Compiler,
                          (gctSIZE_T)sizeof(struct _cloIR_LABEL),
                          (gctPOINTER *) &label);
        if (gcmIS_ERROR(status)) break;
              (void)gcoOS_ZeroMemory((gctPOINTER)label, sizeof(struct _cloIR_LABEL));
        cloIR_BASE_Initialize(&label->base, &s_LabelVTab, LineNo, StringNo);
        *Label = label;
        return gcvSTATUS_OK;
    } while (gcvFALSE);

    *Label = gcvNULL;
    return status;
}

/* cloIR_VARIABLE object. */
gceSTATUS
cloIR_VARIABLE_Destroy(
IN cloCOMPILER Compiler,
IN cloIR_BASE This
)
{
    cloIR_VARIABLE    variable = (cloIR_VARIABLE)This;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(variable, clvIR_VARIABLE);

    if (variable->exprBase.asmMods != gcvNULL)
    {
        gcmVERIFY_OK(cloCOMPILER_Free(Compiler, variable->exprBase.asmMods));
    }

    gcmVERIFY_OK(cloCOMPILER_Free(Compiler, variable));

    return gcvSTATUS_OK;
}

gceSTATUS
cloIR_VARIABLE_Dump(
IN cloCOMPILER Compiler,
IN cloIR_BASE This
)
{
    cloIR_VARIABLE    variable = (cloIR_VARIABLE)This;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(variable, clvIR_VARIABLE);

    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                      clvDUMP_IR,
                      "<IR_VARIABLE line=\"%d\" string=\"%d\">",
                      variable->exprBase.base.lineNo,
                      variable->exprBase.base.stringNo));

    gcmASSERT(variable->name);
    gcmVERIFY_OK(clsNAME_Dump(Compiler, variable->name));

    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                      clvDUMP_IR,
                      "</IR_VARIABLE>"));
    return gcvSTATUS_OK;
}

gceSTATUS
cloIR_VARIABLE_Accept(
IN cloCOMPILER Compiler,
IN cloIR_BASE This,
IN clsVISITOR * Visitor,
IN OUT gctPOINTER Parameters
)
{
    cloIR_VARIABLE    variable = (cloIR_VARIABLE)This;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(variable, clvIR_VARIABLE);
    gcmASSERT(Visitor);

    if (Visitor->visitVariable == gcvNULL) return gcvSTATUS_OK;

    return Visitor->visitVariable(Compiler, Visitor, variable, Parameters);
}

static clsVTAB s_variableVTab =
{
    clvIR_VARIABLE,
    cloIR_VARIABLE_Destroy,
    cloIR_VARIABLE_Dump,
    cloIR_VARIABLE_Accept
};

gceSTATUS
cloIR_VARIABLE_Construct(
IN cloCOMPILER Compiler,
IN gctUINT LineNo,
IN gctUINT StringNo,
IN clsNAME * Name,
OUT cloIR_VARIABLE * Variable
)
{
    gceSTATUS status;
    cloIR_VARIABLE variable;
    clsDECL decl;
    gctPOINTER pointer;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    gcmASSERT(Name);

    do {
        if (Name->decl.dataType == gcvNULL) {
            gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                            LineNo,
                            StringNo,
                            clvREPORT_ERROR,
                            "'%s' has no data type",
                            Name->symbol));
            status = gcvSTATUS_INVALID_ARGUMENT;
            break;
        }

        status = cloCOMPILER_Allocate(Compiler,
                          (gctSIZE_T)sizeof(struct _cloIR_VARIABLE),
                          (gctPOINTER *) &pointer);
        if (gcmIS_ERROR(status)) break;
        variable = pointer;

        decl = Name->decl;
        status = cloCOMPILER_ClonePtrDscr(Compiler,
                          Name->decl.ptrDscr,
                          &decl.ptrDscr);
            if (gcmIS_ERROR(status)) break;

        cloIR_EXPR_Initialize(&variable->exprBase, &s_variableVTab, LineNo, StringNo,
                      decl);

        variable->name    = Name;
        *Variable = variable;
        return gcvSTATUS_OK;
    } while (gcvFALSE);

    *Variable = gcvNULL;
    return status;
}

/* cloIR_CONSTANT object. */
gceSTATUS
cloIR_CONSTANT_Destroy(
IN cloCOMPILER Compiler,
IN cloIR_BASE This
)
{
    cloIR_CONSTANT    constant = (cloIR_CONSTANT)This;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(constant, clvIR_CONSTANT);

    if (constant->valueCount > 0) {
        gcmASSERT(constant->values || constant->buffer);

        if(constant->values) {
           gcmVERIFY_OK(cloCOMPILER_Free(Compiler, constant->values));
        }
        if(constant->buffer) {
           gcmVERIFY_OK(cloCOMPILER_Free(Compiler, constant->buffer));
        }
    }
    if (constant->uniformCount > 0) {
        gcmVERIFY_OK(cloCOMPILER_Free(Compiler, constant->u.uniformArr));
    }

    if (constant->exprBase.asmMods != gcvNULL)
    {
        gcmVERIFY_OK(cloCOMPILER_Free(Compiler, constant->exprBase.asmMods));
    }

    gcmVERIFY_OK(cloCOMPILER_Free(Compiler, constant));
    return gcvSTATUS_OK;
}

gceSTATUS
cloIR_CONSTANT_Dump(
IN cloCOMPILER Compiler,
IN cloIR_BASE This
)
{
    gctUINT        i;
    cloIR_CONSTANT    constant = (cloIR_CONSTANT)This;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(constant, clvIR_CONSTANT);

    if(constant->values) {
       gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                         clvDUMP_IR,
                         "<IR_CONSTANT line=\"%d\" string=\"%d\" dataType=\"0x%x\""
                         " valueCount=\"%d\" value=\"0x%x\" >",
                         constant->exprBase.base.lineNo,
                         constant->exprBase.base.stringNo,
                         constant->exprBase.decl.dataType,
                         constant->valueCount,
                         constant->values));

       for (i = 0; i < constant->valueCount; i++) {
        gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                          clvDUMP_IR,
                          "<VALUE bool=\"%s\" int=\"%d\" float=\"%f\" />",
                          (constant->values[i].boolValue) ? "true" : "false",
                          constant->values[i].intValue,
                          constant->values[i].floatValue));
       }
    }

    if(constant->buffer) {
       gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                         clvDUMP_IR,
                         "<IR_CONSTANT line=\"%d\" string=\"%d\" dataType=\"0x%x\""
                         " valueCount=\"%d\" buffer=\"0x%x\" >",
                         constant->exprBase.base.lineNo,
                         constant->exprBase.base.stringNo,
                         constant->exprBase.decl.dataType,
                         constant->valueCount,
                         constant->buffer));
    }

    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                      clvDUMP_IR,
                      "</IR_CONSTANT>"));
    return gcvSTATUS_OK;
}

gceSTATUS
cloIR_CONSTANT_Accept(
IN cloCOMPILER Compiler,
IN cloIR_BASE This,
IN clsVISITOR * Visitor,
IN OUT gctPOINTER Parameters
)
{
    cloIR_CONSTANT    constant = (cloIR_CONSTANT)This;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(constant, clvIR_CONSTANT);
    gcmASSERT(Visitor);

    if (Visitor->visitConstant == gcvNULL) return gcvSTATUS_OK;

    return Visitor->visitConstant(Compiler, Visitor, constant, Parameters);
}

static clsVTAB s_constantVTab =
{
    clvIR_CONSTANT,
    cloIR_CONSTANT_Destroy,
    cloIR_CONSTANT_Dump,
    cloIR_CONSTANT_Accept
};

gceSTATUS
cloIR_CONSTANT_Allocate(
IN cloCOMPILER Compiler,
IN gctUINT LineNo,
IN gctUINT StringNo,
IN clsDECL *Decl,
OUT cloIR_CONSTANT *Constant
)
{
  gceSTATUS status;
  clsDECL decl;
  cloIR_CONSTANT constant = gcvNULL;

/* Verify the arguments. */
  clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
  gcmASSERT(Decl && Decl->dataType);

  status = cloCOMPILER_CloneDecl(Compiler,
                                 clvQUALIFIER_CONST,
                                 clvQUALIFIER_CONSTANT,
                                 Decl,
                                 &decl);
  if (gcmIS_ERROR(status)) return status;

  do {
     gctPOINTER pointer;
     gctSIZE_T size;

     status = cloCOMPILER_Allocate(Compiler,
                       (gctSIZE_T)sizeof(struct _cloIR_CONSTANT),
                       (gctPOINTER *) &pointer);
     if (gcmIS_ERROR(status)) break;
     (void)gcoOS_ZeroMemory(pointer, sizeof(struct _cloIR_CONSTANT));
     constant = pointer;

     cloIR_EXPR_Initialize(&constant->exprBase, &s_constantVTab, LineNo, StringNo,
                           decl);

     if((_GEN_UNIFORMS_FOR_CONSTANT_ADDRESS_SPACE_VARIABLES && clmDECL_IsAggregateType(&decl)) ||
        (clmDECL_IsUnderlyingStructOrUnion(&decl) &&
         (clGetOperandCountForRegAlloc(&decl) > _clmMaxOperandCountToUseMemory(&decl)))) {
         constant->valueCount = clsDECL_GetByteSize(Compiler, &decl);
         size = (gctSIZE_T)(sizeof(gctCHAR) * constant->valueCount);
         status = cloCOMPILER_ZeroMemoryAllocate(Compiler,
                                                 size,
                                                 (gctPOINTER *) &pointer);
         if (gcmIS_ERROR(status)) break;

         constant->buffer = pointer;
     }
     else {
         constant->valueCount = clsDECL_GetSize(&decl);
         size = (gctSIZE_T)(sizeof(cluCONSTANT_VALUE) * constant->valueCount);
         status = cloCOMPILER_Allocate(Compiler,
                                       size,
                                       (gctPOINTER *) &pointer);
         if (gcmIS_ERROR(status)) break;
         (void) gcoOS_ZeroMemory(pointer, size);

         constant->values = pointer;
     }

     *Constant = constant;
     return gcvSTATUS_OK;
  } while (gcvFALSE);

  if(constant) {
    if (constant->values != gcvNULL) {
        gcmVERIFY_OK(cloCOMPILER_Free(Compiler, constant->values));
    }
    if (constant->buffer != gcvNULL) {
        gcmVERIFY_OK(cloCOMPILER_Free(Compiler, constant->buffer));
    }
    if (constant->uniformCount > 0) {
        gcmVERIFY_OK(cloCOMPILER_Free(Compiler, constant->u.uniformArr));
    }
    gcmVERIFY_OK(cloCOMPILER_Free(Compiler, constant));
  }
  *Constant = gcvNULL;
  return status;
}

gceSTATUS
cloIR_CONSTANT_Construct(
IN cloCOMPILER Compiler,
IN gctUINT LineNo,
IN gctUINT StringNo,
IN clsDECL * Decl,
OUT cloIR_CONSTANT * Constant
)
{
    gceSTATUS    status;
    cloIR_CONSTANT    constant = gcvNULL;
    clsDECL decl;
    gctPOINTER pointer;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    gcmASSERT(Decl && Decl->dataType);
    gcmASSERT(Decl->dataType->accessQualifier == clvQUALIFIER_CONST);

    do {
        status = cloCOMPILER_Allocate(Compiler,
                          (gctSIZE_T)sizeof(struct _cloIR_CONSTANT),
                          (gctPOINTER *) &pointer);
        if (gcmIS_ERROR(status)) break;
        (void)gcoOS_ZeroMemory(pointer, sizeof(struct _cloIR_CONSTANT));
        constant = pointer;

        clmDECL_Initialize(&decl, Decl->dataType, &Decl->array, gcvNULL, gcvFALSE, clvSTORAGE_QUALIFIER_NONE);
        cloIR_EXPR_Initialize(&constant->exprBase, &s_constantVTab, LineNo, StringNo,
                      decl);

        *Constant = constant;
        return gcvSTATUS_OK;
    } while (gcvFALSE);

    *Constant = gcvNULL;
    return status;
}

gceSTATUS
cloIR_CONSTANT_Clone(
IN cloCOMPILER Compiler,
IN gctUINT LineNo,
IN gctUINT StringNo,
IN cloIR_CONSTANT Source,
OUT cloIR_CONSTANT * Constant
)
{
    gceSTATUS    status;
    cluCONSTANT_VALUE *values = gcvNULL;
    gctSTRING buffer = gcvNULL;
    cloIR_CONSTANT    constant = gcvNULL;
    clsDECL decl;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(Source, clvIR_CONSTANT);

    do {
        gctPOINTER pointer;

        if (Source->valueCount > 0) {
            if(Source->values) {
                status = cloCOMPILER_Allocate(Compiler,
                                  (gctSIZE_T)sizeof(cluCONSTANT_VALUE) * Source->valueCount,
                                  (gctPOINTER *) &pointer);
                if (gcmIS_ERROR(status)) break;
                values = pointer;
                gcoOS_MemCopy(values,
                          Source->values,
                          (gctSIZE_T)sizeof(cluCONSTANT_VALUE) * Source->valueCount);
            }
            else {
                status = cloCOMPILER_Allocate(Compiler,
                                  (gctSIZE_T)sizeof(gctCHAR) * Source->valueCount,
                                  (gctPOINTER *) &pointer);
                if (gcmIS_ERROR(status)) break;
                buffer = pointer;
                gcoOS_MemCopy(buffer,
                          Source->buffer,
                          (gctSIZE_T)sizeof(gctCHAR) * Source->valueCount);
            }
        }

        status = cloCOMPILER_ZeroMemoryAllocate(Compiler,
                                                (gctSIZE_T)sizeof(struct _cloIR_CONSTANT),
                                                (gctPOINTER *) &pointer);
        if (gcmIS_ERROR(status)) break;
        constant = pointer;

        if (Source->uniformCount > 0) {
            status = cloCOMPILER_Allocate(Compiler,
                                          (gctSIZE_T)sizeof(gcUNIFORM) * Source->uniformCount,
                                          (gctPOINTER *) &pointer);
            if (gcmIS_ERROR(status)) break;
            constant->u.uniformArr = pointer;
            gcoOS_MemCopy(constant->u.uniformArr,
                          Source->u.uniformArr,
                          (gctSIZE_T)sizeof(gcUNIFORM) * Source->uniformCount);
            constant->uniformCount = Source->uniformCount;
        }

        clmDECL_Initialize(&decl,
                           Source->exprBase.decl.dataType,
                           &Source->exprBase.decl.array,
                           gcvNULL,
                           gcvFALSE,
                           clvSTORAGE_QUALIFIER_NONE);
        cloIR_EXPR_Initialize(&constant->exprBase,
                              &s_constantVTab,
                              LineNo,
                              StringNo,
                              decl);

        constant->valueCount = Source->valueCount;
        constant->values = values;
        constant->buffer = buffer;
        constant->variable = Source->variable;
        constant->allValuesEqual = Source->allValuesEqual;
        *Constant = constant;
        return gcvSTATUS_OK;
    } while (gcvFALSE);

    if(constant) {
        gcmVERIFY_OK(cloCOMPILER_Free(Compiler, constant));
    }
    if (values != gcvNULL) {
        gcmVERIFY_OK(cloCOMPILER_Free(Compiler, values));
    }
    if (buffer != gcvNULL) {
        gcmVERIFY_OK(cloCOMPILER_Free(Compiler, buffer));
    }

    *Constant = gcvNULL;
    return status;
}

gceSTATUS
cloIR_CONSTANT_AddValues(
IN cloCOMPILER Compiler,
IN cloIR_CONSTANT Constant,
IN gctUINT ValueCount,
IN cluCONSTANT_VALUE * Values
)
{
    gceSTATUS status;
    cluCONSTANT_VALUE * newValues = gcvNULL;
    gctUINT        i;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(Constant, clvIR_CONSTANT);
    gcmASSERT(ValueCount > 0);

    do {
        if (Constant->valueCount == 0) {
            gcmASSERT(Constant->values == gcvNULL);

            status = cloCOMPILER_Allocate(Compiler,
                              (gctSIZE_T)sizeof(cluCONSTANT_VALUE) * ValueCount,
                              (gctPOINTER *) &newValues);
            if (gcmIS_ERROR(status)) break;

            for (i = 0; i < ValueCount; i++) {
                newValues[i] = Values[i];
            }

            Constant->valueCount    = ValueCount;
            Constant->values    = newValues;
        }
        else {
            gcmASSERT(Constant->values);

            status = cloCOMPILER_Allocate(Compiler,
                              (gctSIZE_T)sizeof(cluCONSTANT_VALUE) *
                              (Constant->valueCount + ValueCount),
                              (gctPOINTER *) &newValues);
            if (gcmIS_ERROR(status)) break;

            gcoOS_MemCopy(newValues,
                      Constant->values,
                      (gctSIZE_T)sizeof(cluCONSTANT_VALUE) * Constant->valueCount);

            for (i = 0; i < ValueCount; i++) {
                newValues[Constant->valueCount + i] = Values[i];
            }

            gcmVERIFY_OK(cloCOMPILER_Free(Compiler, Constant->values));

            Constant->valueCount    += ValueCount;
            Constant->values    = newValues;
        }

        return gcvSTATUS_OK;
    } while (gcvFALSE);

    if (newValues != gcvNULL) {
        gcmVERIFY_OK(cloCOMPILER_Free(Compiler, newValues));
    }
    return status;
}

gceSTATUS
cloIR_CONSTANT_AddCharValues(
IN cloCOMPILER Compiler,
IN cloIR_CONSTANT Constant,
IN gctUINT ValueCount,
IN gctSTRING Values
)
{
    gceSTATUS status;
    cluCONSTANT_VALUE * newValues = gcvNULL;
    gctUINT        i;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(Constant, clvIR_CONSTANT);
    gcmASSERT(ValueCount > 0);

    do {
        if (Constant->valueCount == 0) {
            gcmASSERT(Constant->values == gcvNULL);

            status = cloCOMPILER_Allocate(Compiler,
                              (gctSIZE_T)sizeof(cluCONSTANT_VALUE) * ValueCount,
                              (gctPOINTER *) &newValues);
            if (gcmIS_ERROR(status)) break;

            for (i = 0; i < ValueCount; i++) {
                newValues[i].intValue = Values[i];
            }

            Constant->valueCount    = ValueCount;
            Constant->values    = newValues;
        }
        else {
            gcmASSERT(Constant->values);

            status = cloCOMPILER_Allocate(Compiler,
                              (gctSIZE_T)sizeof(cluCONSTANT_VALUE) *
                              (Constant->valueCount + ValueCount),
                              (gctPOINTER *) &newValues);
            if (gcmIS_ERROR(status)) break;

            gcoOS_MemCopy(newValues,
                      Constant->values,
                      (gctSIZE_T)sizeof(cluCONSTANT_VALUE) * Constant->valueCount);

            for (i = 0; i < ValueCount; i++) {
                newValues[Constant->valueCount + i].intValue = Values[i];
            }

            gcmVERIFY_OK(cloCOMPILER_Free(Compiler, Constant->values));

            Constant->valueCount    += ValueCount;
            Constant->values    = newValues;
        }

        return gcvSTATUS_OK;
    } while (gcvFALSE);

    if (newValues != gcvNULL) {
        gcmVERIFY_OK(cloCOMPILER_Free(Compiler, newValues));
    }
    return status;
}

gceSTATUS
cloIR_CONSTANT_GetBoolValue(
    IN cloCOMPILER Compiler,
    IN cloIR_CONSTANT Constant,
    IN gctUINT ValueNo,
    OUT cluCONSTANT_VALUE * Value
    )
{
    /* Verify the arguments. */
    clmVERIFY_IR_OBJECT(Constant, clvIR_CONSTANT);
    gcmASSERT(Constant->valueCount > ValueNo);
    gcmASSERT(Value);

    switch (Constant->exprBase.decl.dataType->elementType)
    {
    case clvTYPE_BOOL:
        Value->boolValue = Constant->values[ValueNo].boolValue;
        break;

    case clvTYPE_INT:
    case clvTYPE_SHORT:
    case clvTYPE_LONG:
        Value->boolValue = clmI2B(Constant->values[ValueNo].intValue);
        break;

    case clvTYPE_UINT:
    case clvTYPE_USHORT:
    case clvTYPE_ULONG:
    case clvTYPE_UCHAR:
        Value->boolValue = clmU2B(Constant->values[ValueNo].uintValue);
        break;

    case clvTYPE_CHAR:
        Value->boolValue = clmC2B(Constant->values[ValueNo].intValue);
        break;

    case clvTYPE_FLOAT:
        Value->boolValue = clmF2B(Constant->values[ValueNo].floatValue);
        break;

    default:
        gcmASSERT(0);
    }

    return gcvSTATUS_OK;
}

gceSTATUS
cloIR_CONSTANT_GetIntValue(
IN cloCOMPILER Compiler,
IN cloIR_CONSTANT Constant,
IN gctUINT ValueNo,
OUT cluCONSTANT_VALUE * Value
)
{
    /* Verify the arguments. */
    clmVERIFY_IR_OBJECT(Constant, clvIR_CONSTANT);
    gcmASSERT(Constant->valueCount > ValueNo);
    gcmASSERT(Value);

    switch (Constant->exprBase.decl.dataType->elementType) {
    case clvTYPE_BOOL:
        Value->intValue = clmB2I(Constant->values[ValueNo].boolValue);
        break;

    case clvTYPE_CHAR:
        Value->intValue = clmC2I(Constant->values[ValueNo].intValue);
        break;

    case clvTYPE_INT:
    case clvTYPE_SHORT:
    case clvTYPE_LONG:
        Value->intValue = Constant->values[ValueNo].intValue;
        break;

    case clvTYPE_UINT:
    case clvTYPE_USHORT:
    case clvTYPE_UCHAR:
    case clvTYPE_ULONG:
        Value->intValue = clmU2I(Constant->values[ValueNo].uintValue);
        break;

    case clvTYPE_FLOAT:
        Value->intValue = clmF2I(Constant->values[ValueNo].floatValue);
        break;

    default:
        gcmASSERT(0);
    }

    return gcvSTATUS_OK;
}

gceSTATUS
cloIR_CONSTANT_GetUintValue(
IN cloCOMPILER Compiler,
IN cloIR_CONSTANT Constant,
IN gctUINT ValueNo,
OUT cluCONSTANT_VALUE * Value
)
{
    /* Verify the arguments. */
    clmVERIFY_IR_OBJECT(Constant, clvIR_CONSTANT);
    gcmASSERT(Constant->valueCount > ValueNo);
    gcmASSERT(Value);

    switch (Constant->exprBase.decl.dataType->elementType) {
    case clvTYPE_BOOL:
    case clvTYPE_BOOL_PACKED:
        Value->uintValue = clmB2U(Constant->values[ValueNo].boolValue);
        break;

    case clvTYPE_CHAR:
    case clvTYPE_CHAR_PACKED:
        Value->uintValue = clmC2U(Constant->values[ValueNo].intValue);
        break;

    case clvTYPE_INT:
    case clvTYPE_SHORT:
    case clvTYPE_SHORT_PACKED:
    case clvTYPE_LONG:
        Value->uintValue = clmI2U(Constant->values[ValueNo].intValue);
        break;

    case clvTYPE_UINT:
    case clvTYPE_USHORT:
    case clvTYPE_USHORT_PACKED:
    case clvTYPE_ULONG:
    case clvTYPE_UCHAR:
    case clvTYPE_UCHAR_PACKED:
        Value->uintValue = Constant->values[ValueNo].uintValue;
        break;

    case clvTYPE_FLOAT:
        Value->uintValue = clmF2U(Constant->values[ValueNo].floatValue);
        break;

    default:
        gcmASSERT(0);
    }

    return gcvSTATUS_OK;
}

gceSTATUS
cloIR_CONSTANT_GetLongValue(
IN cloCOMPILER Compiler,
IN cloIR_CONSTANT Constant,
IN gctUINT ValueNo,
OUT cluCONSTANT_VALUE * Value
)
{
    /* Verify the arguments. */
    clmVERIFY_IR_OBJECT(Constant, clvIR_CONSTANT);
    gcmASSERT(Constant->valueCount > ValueNo);
    gcmASSERT(Value);

    switch (Constant->exprBase.decl.dataType->elementType) {
    case clvTYPE_BOOL:
        Value->longValue = clmB2L(Constant->values[ValueNo].boolValue);
        break;

    case clvTYPE_CHAR:
        Value->longValue = clmC2L(Constant->values[ValueNo].intValue);
        break;

    case clvTYPE_INT:
    case clvTYPE_SHORT:
        Value->longValue = clmI2L(Constant->values[ValueNo].intValue);
        break;

    case clvTYPE_LONG:
        Value->longValue = Constant->values[ValueNo].longValue;
        break;

    case clvTYPE_UINT:
    case clvTYPE_USHORT:
    case clvTYPE_UCHAR:
        Value->longValue = clmU2L(Constant->values[ValueNo].uintValue);
        break;

    case clvTYPE_ULONG:
        Value->longValue = clmUL2L(Constant->values[ValueNo].ulongValue);
        break;

    case clvTYPE_FLOAT:
        Value->longValue = clmF2L(Constant->values[ValueNo].floatValue);
        break;

    default:
        gcmASSERT(0);
    }

    return gcvSTATUS_OK;
}

gceSTATUS
cloIR_CONSTANT_GetULongValue(
IN cloCOMPILER Compiler,
IN cloIR_CONSTANT Constant,
IN gctUINT ValueNo,
OUT cluCONSTANT_VALUE * Value
)
{
    /* Verify the arguments. */
    clmVERIFY_IR_OBJECT(Constant, clvIR_CONSTANT);
    gcmASSERT(Constant->valueCount > ValueNo);
    gcmASSERT(Value);

    switch (Constant->exprBase.decl.dataType->elementType) {
    case clvTYPE_BOOL:
        Value->ulongValue = clmB2UL(Constant->values[ValueNo].boolValue);
        break;

    case clvTYPE_CHAR:
        Value->ulongValue = clmC2UL(Constant->values[ValueNo].intValue);
        break;

    case clvTYPE_INT:
    case clvTYPE_SHORT:
        Value->ulongValue = clmI2UL(Constant->values[ValueNo].intValue);
        break;

    case clvTYPE_LONG:
        Value->ulongValue = clmL2UL(Constant->values[ValueNo].intValue);
        break;

    case clvTYPE_UINT:
    case clvTYPE_USHORT:
    case clvTYPE_UCHAR:
        Value->ulongValue = clmU2UL(Constant->values[ValueNo].uintValue);
        break;

    case clvTYPE_ULONG:
        Value->ulongValue = Constant->values[ValueNo].ulongValue;
        break;

    case clvTYPE_FLOAT:
        Value->ulongValue = clmF2UL(Constant->values[ValueNo].floatValue);
        break;

    default:
        gcmASSERT(0);
    }

    return gcvSTATUS_OK;
}

gceSTATUS
cloIR_CONSTANT_GetCharValue(
IN cloCOMPILER Compiler,
IN cloIR_CONSTANT Constant,
IN gctUINT ValueNo,
OUT cluCONSTANT_VALUE * Value
)
{
    /* Verify the arguments. */
    clmVERIFY_IR_OBJECT(Constant, clvIR_CONSTANT);
    gcmASSERT(Constant->valueCount > ValueNo);
    gcmASSERT(Value);

    switch (Constant->exprBase.decl.dataType->elementType) {
    case clvTYPE_BOOL:
        Value->intValue = clmB2C(Constant->values[ValueNo].boolValue);
        break;

    case clvTYPE_CHAR:
        Value->intValue = Constant->values[ValueNo].intValue;
        break;

    case clvTYPE_INT:
    case clvTYPE_SHORT:
    case clvTYPE_LONG:
        Value->intValue = clmI2C(Constant->values[ValueNo].intValue);
        break;

    case clvTYPE_UINT:
    case clvTYPE_USHORT:
    case clvTYPE_ULONG:
    case clvTYPE_UCHAR:
        Value->uintValue = clmU2C(Constant->values[ValueNo].uintValue);
        break;

    case clvTYPE_FLOAT:
        Value->intValue = clmF2C(Constant->values[ValueNo].floatValue);
        break;

    default:
        gcmASSERT(0);
    }

    return gcvSTATUS_OK;
}

gceSTATUS
cloIR_CONSTANT_GetFloatValue(
    IN cloCOMPILER Compiler,
    IN cloIR_CONSTANT Constant,
    IN gctUINT ValueNo,
    OUT cluCONSTANT_VALUE * Value
    )
{
    /* Verify the arguments. */
    clmVERIFY_IR_OBJECT(Constant, clvIR_CONSTANT);
    gcmASSERT(Constant->valueCount > ValueNo);
    gcmASSERT(Value);

    switch (Constant->exprBase.decl.dataType->elementType)
    {
    case clvTYPE_BOOL:
        Value->floatValue = clmB2F(Constant->values[ValueNo].boolValue);
        break;

    case clvTYPE_CHAR:
        Value->floatValue = clmC2F(Constant->values[ValueNo].intValue);
        break;

    case clvTYPE_UINT:
    case clvTYPE_USHORT:
    case clvTYPE_ULONG:
    case clvTYPE_UCHAR:
        Value->floatValue = clmU2F(Constant->values[ValueNo].uintValue);
        break;

    case clvTYPE_INT:
    case clvTYPE_SHORT:
    case clvTYPE_LONG:
        Value->floatValue = clmI2F(Constant->values[ValueNo].intValue);
        break;

    case clvTYPE_FLOAT:
        Value->floatValue = Constant->values[ValueNo].floatValue;
        break;

    default:
        gcmASSERT(0);
    }

    return gcvSTATUS_OK;
}

static gceSTATUS
_EvaluateConstantValues(
IN cloCOMPILER Compiler,
IN clsDECL * Decl,
IN OUT gctUINT * Offset,
IN OUT cluCONSTANT_VALUE * Values,
IN cltEVALUATE_FUNC_PTR Evaluate
)
{
    gceSTATUS    status;
    clsDECL    subDecl;
    clsDATA_TYPE dataType;
    gctUINT8    i;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    gcmASSERT(Decl && Decl->dataType);
    gcmASSERT(Offset);
    gcmASSERT(Values);
    gcmASSERT(Evaluate);

    clmDECL_Initialize(&subDecl, &dataType, &Decl->array, Decl->ptrDscr, gcvFALSE, clvSTORAGE_QUALIFIER_NONE);
    if (clmDECL_IsArray(Decl)) {
        gctINT elementCount;

        *subDecl.dataType = *Decl->dataType;
        subDecl.array.numDim = 0;

        clmGetArrayElementCount(Decl->array, 0, elementCount);
        for (i = 0; i < elementCount; i++) {
            status = _EvaluateConstantValues(Compiler,
                                             &subDecl,
                                             Offset,
                                             Values,
                                             Evaluate);
            if (gcmIS_ERROR(status)) return status;
        }
    }
    else if (clmDATA_TYPE_matrixColumnCount_GET(Decl->dataType) != 0) {
        *subDecl.dataType = *Decl->dataType;
    clmDATA_TYPE_matrixSize_SET(subDecl.dataType, 0, 0);

        for (i = 0; i < clmDATA_TYPE_matrixRowCount_GET(Decl->dataType) *
                         clmDATA_TYPE_matrixColumnCount_GET(Decl->dataType); i++) {
            status = _EvaluateConstantValues(Compiler,
                                             &subDecl,
                                             Offset,
                                             Values,
                                             Evaluate);
            if (gcmIS_ERROR(status)) return status;
        }
    }
    else if (clmDATA_TYPE_vectorSize_GET(Decl->dataType) != 0) {
        *subDecl.dataType = *Decl->dataType;
    clmDATA_TYPE_vectorSize_SET(subDecl.dataType, 0);

    for (i = 0; i < clmDATA_TYPE_vectorSize_NOCHECK_GET(Decl->dataType); i++) {
            status = _EvaluateConstantValues(Compiler,
                                             &subDecl,
                                             Offset,
                                             Values,
                                             Evaluate);
            if (gcmIS_ERROR(status)) return status;
        }
    }
    else {
        if(clmDECL_IsArithmeticType(Decl)) {
            status = (*Evaluate)(Decl->dataType->elementType, Values + *Offset);

            if (gcmIS_ERROR(status)) return status;

            (*Offset)++;
    }
    else {
            gcmASSERT(0);
            return gcvSTATUS_INVALID_ARGUMENT;
        }
    }

    return gcvSTATUS_OK;
}

gctINT
cloIR_CONSTANT_GetIntegerValue(
IN cloIR_CONSTANT Constant
)
{
   gcmASSERT(clmDECL_IsScalar(&Constant->exprBase.decl));

   switch(clmDATA_TYPE_elementType_GET(Constant->exprBase.decl.dataType)) {
   case clvTYPE_UINT:
   case clvTYPE_UCHAR:
   case clvTYPE_USHORT:
        return Constant->values[0].uintValue;

   case clvTYPE_INT:
   case clvTYPE_CHAR:
   case clvTYPE_SHORT:
        return Constant->values[0].intValue;

   case clvTYPE_BOOL:
        return (int)Constant->values[0].boolValue;

   case clvTYPE_FLOAT:
        return (int)Constant->values[0].floatValue;

   default:
        gcmASSERT(0);
   }
   return 0;
}

gctBOOL
cloIR_CONSTANT_CheckAndSetAllValuesEqual(
IN cloCOMPILER Compiler,
IN cloIR_CONSTANT Constant
)
{
    gctUINT    i;
    cltELEMENT_TYPE elementType;

    gcmASSERT(Constant);

    if (Constant->allValuesEqual) return gcvTRUE;
    if (clmDECL_IsUnderlyingStructOrUnion(&Constant->exprBase.decl)) return gcvFALSE;
    elementType = clmDATA_TYPE_elementType_GET(Constant->exprBase.decl.dataType);

    if(clmIsElementTypeFloating(elementType)) {
        for (i = 1; i < Constant->valueCount; i++) {
            if (Constant->values[i].floatValue
                != Constant->values[0].floatValue) {
                return gcvFALSE;
            }
        }
    }
    else if(clmIsElementTypeBoolean(elementType)) {
        for (i = 1; i < Constant->valueCount; i++) {
            if (Constant->values[i].boolValue
                != Constant->values[0].boolValue) {
                return gcvFALSE;
            }
        }
    }
    else if(clmIsElementTypeInteger(elementType)) {
        if (clmIsElementTypeHighPrecision(elementType)) {
            for (i = 1; i < Constant->valueCount; i++) {
                if (Constant->values[i].longValue
                    != Constant->values[0].longValue) {
                    return gcvFALSE;
                }
            }
        }
        else {
            for (i = 1; i < Constant->valueCount; i++) {
                if (Constant->values[i].intValue
                    != Constant->values[0].intValue) {
                    return gcvFALSE;
                }
            }
        }
    }
    else return gcvFALSE;

    Constant->allValuesEqual = gcvTRUE;
    return gcvTRUE;
}

gceSTATUS
cloIR_CONSTANT_Evaluate(
IN cloCOMPILER Compiler,
IN OUT cloIR_CONSTANT Constant,
IN cltEVALUATE_FUNC_PTR Evaluate
)
{
    gceSTATUS    status;
    gctUINT        offset = 0;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(Constant, clvIR_CONSTANT);
    gcmASSERT(Evaluate);

    gcmASSERT(Constant->exprBase.decl.dataType);
    gcmASSERT(Constant->values);

    status = _EvaluateConstantValues(Compiler,
                     &Constant->exprBase.decl,
                     &offset,
                     Constant->values,
                     Evaluate);

    if (gcmIS_ERROR(status)) return status;

    gcmASSERT(offset == Constant->valueCount);
    Constant->variable = gcvNULL;

    return gcvSTATUS_OK;
}

static gceSTATUS
_cloIR_CONSTANT_Mat_Mul_Mat(
    IN cloCOMPILER Compiler,
    IN cleBINARY_EXPR_TYPE ExprType,
    IN cloIR_CONSTANT LeftConstant,
    IN cloIR_CONSTANT RightConstant,
    OUT cloIR_CONSTANT * ResultConstant
    )
{
    gctUINT8    col, row, i;
    gctUINT8    leftColumnCount, leftRowCount;
    gctUINT8    rightColumnCount, rightRowCount;
        gctUINT        resultValueCount;
    cluCONSTANT_VALUE    *resultValues;


    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(LeftConstant, clvIR_CONSTANT);
    clmVERIFY_IR_OBJECT(RightConstant, clvIR_CONSTANT);
    gcmASSERT(ResultConstant);

    gcmASSERT(LeftConstant->exprBase.decl.dataType);
    gcmASSERT(RightConstant->exprBase.decl.dataType);
    gcmASSERT(clmDECL_IsMat(&LeftConstant->exprBase.decl));

    leftColumnCount = clmDATA_TYPE_matrixColumnCount_GET(LeftConstant->exprBase.decl.dataType);
    rightRowCount = clmDATA_TYPE_matrixRowCount_GET(RightConstant->exprBase.decl.dataType);
    gcmASSERT(leftColumnCount == rightRowCount);
    leftRowCount = clmDATA_TYPE_matrixRowCount_GET(LeftConstant->exprBase.decl.dataType);
    rightColumnCount = clmDATA_TYPE_matrixColumnCount_GET(RightConstant->exprBase.decl.dataType);

    gcmASSERT(LeftConstant->valueCount == (gctUINT) (leftRowCount * leftColumnCount));
    gcmASSERT(RightConstant->valueCount == (gctUINT) (rightRowCount * rightColumnCount));

        resultValueCount = leftRowCount * rightColumnCount;
        resultValues = _GetResultBuffer(resultValueCount);
        if (resultValues == gcvNULL) return gcvSTATUS_OUT_OF_MEMORY;
    /* Calculate the result */
    for (col = 0; col < rightColumnCount; col++)
    {
        cluCONSTANT_VALUE  *resultPtr = resultValues + (col * leftRowCount);
        for (row = 0; row < leftRowCount; row++, resultPtr++)
        {
            resultPtr->floatValue = 0;

            for (i = 0; i < leftColumnCount; i++)
            {
                resultPtr->floatValue +=
                        LeftConstant->values[i * leftRowCount + row].floatValue
                            * RightConstant->values[col * rightRowCount + i].floatValue;
            }
        }
    }

        /**** klc
       May need to do more work here if the data type is shared by other objects;
           What that means we cannot change the data inside the data type and will need to allocate a new one
         ****/
        clmDATA_TYPE_matrixSize_SET(LeftConstant->exprBase.decl.dataType, leftRowCount, rightColumnCount);
    /* Save the result back */
    gcmVERIFY_OK(cloIR_CONSTANT_AddValues(Compiler, LeftConstant, leftRowCount * rightColumnCount, resultValues));

    gcmVERIFY_OK(cloIR_OBJECT_Destroy(Compiler, &RightConstant->exprBase.base));

    *ResultConstant = LeftConstant;
    (*ResultConstant)->variable = gcvNULL;
    (*ResultConstant)->allValuesEqual = gcvFALSE;

    return gcvSTATUS_OK;
}

static gceSTATUS
_cloIR_CONSTANT_Vec_Mul_Mat(
IN cloCOMPILER Compiler,
IN cleBINARY_EXPR_TYPE ExprType,
IN cloIR_CONSTANT LeftConstant,
IN cloIR_CONSTANT RightConstant,
OUT cloIR_CONSTANT * ResultConstant
)
{
    gctUINT8    i, j;
    gctUINT8    vectorSize;
    gctFLOAT    resultVector[4];

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(LeftConstant, clvIR_CONSTANT);
    clmVERIFY_IR_OBJECT(RightConstant, clvIR_CONSTANT);
    gcmASSERT(ResultConstant);

    gcmASSERT(LeftConstant->exprBase.decl.dataType);
    gcmASSERT(RightConstant->exprBase.decl.dataType);
    gcmASSERT(clmDECL_IsVec(&LeftConstant->exprBase.decl));
    gcmASSERT(clmDECL_IsSquareMat(&RightConstant->exprBase.decl));
    gcmASSERT(clmDATA_TYPE_vectorSize_GET(LeftConstant->exprBase.decl.dataType)
            == clmDATA_TYPE_matrixRowCount_GET(RightConstant->exprBase.decl.dataType));

    vectorSize = clmDATA_TYPE_vectorSize_GET(LeftConstant->exprBase.decl.dataType);

    gcmASSERT(LeftConstant->valueCount == (gctUINT) vectorSize);
    gcmASSERT(RightConstant->valueCount == (gctUINT) (vectorSize * vectorSize));

    /* Calculate the result */
    for (i = 0; i < vectorSize; i++) {
        resultVector[i] = 0;

        for (j = 0; j < vectorSize; j++) {
            resultVector[i] +=
                    LeftConstant->values[j].floatValue
                        * RightConstant->values[i * vectorSize + j].floatValue;
        }
    }

    /* Save the result back */
    for (i = 0; i < vectorSize; i++) {
        LeftConstant->values[i].floatValue = resultVector[i];
    }

    gcmVERIFY_OK(cloIR_OBJECT_Destroy(Compiler, &RightConstant->exprBase.base));

    *ResultConstant = LeftConstant;
    (*ResultConstant)->variable = gcvNULL;
    (*ResultConstant)->allValuesEqual = gcvFALSE;
    return gcvSTATUS_OK;
}

static gceSTATUS
_cloIR_CONSTANT_Mat_Mul_Vec(
    IN cloCOMPILER Compiler,
    IN cleBINARY_EXPR_TYPE ExprType,
    IN cloIR_CONSTANT LeftConstant,
    IN cloIR_CONSTANT RightConstant,
    OUT cloIR_CONSTANT * ResultConstant
    )
{
    gctUINT8    i, j;
    gctUINT8    vectorSize;
    gctFLOAT    resultVector[4];

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(LeftConstant, clvIR_CONSTANT);
    clmVERIFY_IR_OBJECT(RightConstant, clvIR_CONSTANT);
    gcmASSERT(ResultConstant);

    gcmASSERT(LeftConstant->exprBase.decl.dataType);
    gcmASSERT(RightConstant->exprBase.decl.dataType);
    gcmASSERT(clmDECL_IsSquareMat(&LeftConstant->exprBase.decl));
    gcmASSERT(clmDECL_IsVec(&RightConstant->exprBase.decl));
    gcmASSERT(clmDATA_TYPE_matrixColumnCount_GET(LeftConstant->exprBase.decl.dataType)
                == clmDATA_TYPE_vectorSize_GET(RightConstant->exprBase.decl.dataType));

    vectorSize = clmDATA_TYPE_vectorSize_GET(RightConstant->exprBase.decl.dataType);

    gcmASSERT(LeftConstant->valueCount == (gctUINT) (vectorSize * vectorSize));
    gcmASSERT(RightConstant->valueCount == (gctUINT) vectorSize);

    /* Calculate the result */
    for (i = 0; i < vectorSize; i++) {
        resultVector[i] = 0;

        for (j = 0; j < vectorSize; j++) {
            resultVector[i] +=
                    LeftConstant->values[j * vectorSize + i].floatValue
                        * RightConstant->values[j].floatValue;
        }
    }

    /* Save the result back */
    RightConstant->exprBase.base.lineNo        = LeftConstant->exprBase.base.lineNo;
    RightConstant->exprBase.base.stringNo    = LeftConstant->exprBase.base.stringNo;

    for (i = 0; i < vectorSize; i++) {
        RightConstant->values[i].floatValue = resultVector[i];
    }

    gcmVERIFY_OK(cloIR_OBJECT_Destroy(Compiler, &LeftConstant->exprBase.base));

    *ResultConstant = RightConstant;
    (*ResultConstant)->variable = gcvNULL;
    (*ResultConstant)->allValuesEqual = gcvFALSE;
    return gcvSTATUS_OK;
}

static gceSTATUS
_cloIR_CONSTANT_ArithmeticOperateBySameTypes(
IN cloCOMPILER Compiler,
IN cleBINARY_EXPR_TYPE ExprType,
IN cloIR_CONSTANT LeftConstant,
IN cloIR_CONSTANT RightConstant,
OUT cloIR_CONSTANT * ResultConstant
)
{
    gctUINT        i;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(LeftConstant, clvIR_CONSTANT);
    clmVERIFY_IR_OBJECT(RightConstant, clvIR_CONSTANT);
    gcmASSERT(ResultConstant);

    gcmASSERT(LeftConstant->exprBase.decl.dataType);
    gcmASSERT(RightConstant->exprBase.decl.dataType);
    gcmASSERT(clsDECL_IsEqual(&LeftConstant->exprBase.decl,
                       &RightConstant->exprBase.decl));
    gcmASSERT(RightConstant->valueCount == LeftConstant->valueCount);

#if gcdDEBUG
    if (clmDECL_IsInt(&LeftConstant->exprBase.decl)
        || clmDECL_IsFloat(&LeftConstant->exprBase.decl)) {
        gcmASSERT(LeftConstant->valueCount == 1);
    }
    else if (clmDECL_IsIVec(&LeftConstant->exprBase.decl)
        || clmDECL_IsVec(&LeftConstant->exprBase.decl)) {
        gcmASSERT(LeftConstant->valueCount == clmDATA_TYPE_vectorSize_NOCHECK_GET(LeftConstant->exprBase.decl.dataType));
    }
    else if (clmDECL_IsMat(&LeftConstant->exprBase.decl)) {
        gcmASSERT(LeftConstant->valueCount ==
                        (gctUINT) (clmDATA_TYPE_matrixRowCount_GET(LeftConstant->exprBase.decl.dataType)
                            * clmDATA_TYPE_matrixColumnCount_GET(LeftConstant->exprBase.decl.dataType)));
    }
    else {
        gcmASSERT(0);
        return gcvSTATUS_INVALID_ARGUMENT;
    }
#endif /* gcdDEBUG */

    if (ExprType == clvBINARY_MUL && clmDECL_IsSquareMat(&LeftConstant->exprBase.decl))
    {
        return _cloIR_CONSTANT_Mat_Mul_Mat(Compiler,
                           ExprType,
                           LeftConstant,
                           RightConstant,
                           ResultConstant);
    }

    switch (LeftConstant->exprBase.decl.dataType->elementType) {
    case clvTYPE_INT:
    case clvTYPE_CHAR:
    case clvTYPE_SHORT:
        for (i = 0; i < LeftConstant->valueCount; i++) {
            switch (ExprType)
            {
            case clvBINARY_ADD:
                LeftConstant->values[i].intValue += RightConstant->values[i].intValue;
                break;

            case clvBINARY_SUB:
                LeftConstant->values[i].intValue -= RightConstant->values[i].intValue;
                break;

            case clvBINARY_MUL:
                LeftConstant->values[i].intValue *= RightConstant->values[i].intValue;
                break;

            case clvBINARY_DIV:
                LeftConstant->values[i].intValue /= RightConstant->values[i].intValue;
                break;

            case clvBINARY_MOD:
                LeftConstant->values[i].intValue %= RightConstant->values[i].intValue;
                break;

            default:
                gcmASSERT(0);
                return gcvSTATUS_INVALID_ARGUMENT;
            }
        }
        break;

    case clvTYPE_UINT:
    case clvTYPE_UCHAR:
    case clvTYPE_USHORT:
        for (i = 0; i < LeftConstant->valueCount; i++) {
            switch (ExprType)
            {
            case clvBINARY_ADD:
                LeftConstant->values[i].uintValue += RightConstant->values[i].uintValue;
                break;

            case clvBINARY_SUB:
                LeftConstant->values[i].uintValue -= RightConstant->values[i].uintValue;
                break;

            case clvBINARY_MUL:
                LeftConstant->values[i].uintValue *= RightConstant->values[i].uintValue;
                break;

            case clvBINARY_DIV:
                LeftConstant->values[i].uintValue /= RightConstant->values[i].uintValue;
                break;

            case clvBINARY_MOD:
                LeftConstant->values[i].uintValue %= RightConstant->values[i].uintValue;
                break;

            default:
                gcmASSERT(0);
                return gcvSTATUS_INVALID_ARGUMENT;
            }
        }
        break;

    case clvTYPE_LONG:
        for (i = 0; i < LeftConstant->valueCount; i++) {
            switch (ExprType)
            {
            case clvBINARY_ADD:
                LeftConstant->values[i].longValue += RightConstant->values[i].longValue;
                break;

            case clvBINARY_SUB:
                LeftConstant->values[i].longValue -= RightConstant->values[i].longValue;
                break;

            case clvBINARY_MUL:
                LeftConstant->values[i].longValue *= RightConstant->values[i].longValue;
                break;

            case clvBINARY_DIV:
                LeftConstant->values[i].longValue /= RightConstant->values[i].longValue;
                break;

            case clvBINARY_MOD:
                LeftConstant->values[i].longValue %= RightConstant->values[i].longValue;
                break;

            default:
                gcmASSERT(0);
                return gcvSTATUS_INVALID_ARGUMENT;
            }
        }
        break;

    case clvTYPE_ULONG:
        for (i = 0; i < LeftConstant->valueCount; i++) {
            switch (ExprType)
            {
            case clvBINARY_ADD:
                LeftConstant->values[i].ulongValue += RightConstant->values[i].ulongValue;
                break;

            case clvBINARY_SUB:
                LeftConstant->values[i].ulongValue -= RightConstant->values[i].ulongValue;
                break;

            case clvBINARY_MUL:
                LeftConstant->values[i].ulongValue *= RightConstant->values[i].ulongValue;
                break;

            case clvBINARY_DIV:
                LeftConstant->values[i].ulongValue /= RightConstant->values[i].ulongValue;
                break;

            case clvBINARY_MOD:
                LeftConstant->values[i].ulongValue %= RightConstant->values[i].ulongValue;
                break;

            default:
                gcmASSERT(0);
                return gcvSTATUS_INVALID_ARGUMENT;
            }
        }
        break;

    case clvTYPE_FLOAT:
    case clvTYPE_DOUBLE:
    case clvTYPE_HALF:
        for (i = 0; i < LeftConstant->valueCount; i++) {
            switch (ExprType)
            {
            case clvBINARY_ADD:
                LeftConstant->values[i].floatValue += RightConstant->values[i].floatValue;
                break;

            case clvBINARY_SUB:
                LeftConstant->values[i].floatValue -= RightConstant->values[i].floatValue;
                break;

            case clvBINARY_MUL:
                LeftConstant->values[i].floatValue *= RightConstant->values[i].floatValue;
                break;

            case clvBINARY_DIV:
                LeftConstant->values[i].floatValue /= RightConstant->values[i].floatValue;
                break;

            default:
                gcmASSERT(0);
                return gcvSTATUS_INVALID_ARGUMENT;
            }
        }
        break;

    case clvTYPE_BOOL:
        for (i = 0; i < LeftConstant->valueCount; i++) {
            switch (ExprType)
            {
            case clvBINARY_ADD:
                LeftConstant->values[i].boolValue += RightConstant->values[i].boolValue;
                break;

            case clvBINARY_SUB:
                LeftConstant->values[i].boolValue -= RightConstant->values[i].boolValue;
                break;

            case clvBINARY_MUL:
                LeftConstant->values[i].boolValue *= RightConstant->values[i].boolValue;
                break;

            case clvBINARY_DIV:
                LeftConstant->values[i].boolValue /= RightConstant->values[i].boolValue;
                break;

            default:
                gcmASSERT(0);
                return gcvSTATUS_INVALID_ARGUMENT;
            }
        }
        break;

    default:
        gcmASSERT(0);
        return gcvSTATUS_INVALID_ARGUMENT;
    }

    gcmVERIFY_OK(cloIR_OBJECT_Destroy(Compiler, &RightConstant->exprBase.base));

    *ResultConstant = LeftConstant;
    (*ResultConstant)->variable = gcvNULL;
    (*ResultConstant)->allValuesEqual = gcvFALSE;

    return gcvSTATUS_OK;
}

static gceSTATUS
_cloIR_CONSTANT_Scalar_ArithmeticOperate_VectorOrMatrix(
    IN cloCOMPILER Compiler,
    IN cleBINARY_EXPR_TYPE ExprType,
    IN cloIR_CONSTANT LeftConstant,
    IN cloIR_CONSTANT RightConstant,
    OUT cloIR_CONSTANT * ResultConstant
    )
{
    gctUINT        i;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(LeftConstant, clvIR_CONSTANT);
    clmVERIFY_IR_OBJECT(RightConstant, clvIR_CONSTANT);
    gcmASSERT(ResultConstant);

    gcmASSERT(LeftConstant->exprBase.decl.dataType);
    gcmASSERT(RightConstant->exprBase.decl.dataType);

    gcmASSERT(LeftConstant->valueCount == 1);

#if gcdDEBUG
    if (clmDECL_IsIVec(&RightConstant->exprBase.decl)
        || clmDECL_IsVec(&RightConstant->exprBase.decl))
    {
        gcmASSERT(RightConstant->valueCount == clmDATA_TYPE_vectorSize_GET(RightConstant->exprBase.decl.dataType));
    }
    else if (clmDECL_IsSquareMat(&RightConstant->exprBase.decl))
    {
        gcmASSERT(RightConstant->valueCount ==
                (gctUINT) (clmDATA_TYPE_matrixSize_GET(RightConstant->exprBase.decl.dataType)
                * clmDATA_TYPE_matrixSize_GET(RightConstant->exprBase.decl.dataType)));
    }
    else
    {
        gcmASSERT(0);
        return gcvSTATUS_INVALID_ARGUMENT;
    }
#endif /* gcdDEBUG */

    switch (RightConstant->exprBase.decl.dataType->elementType)
    {
    case clvTYPE_INT:
        for (i = 0; i < RightConstant->valueCount; i++)
        {
            switch (ExprType)
            {
            case clvBINARY_ADD:
                RightConstant->values[i].intValue =
                        LeftConstant->values[0].intValue + RightConstant->values[i].intValue;
                break;

            case clvBINARY_SUB:
                RightConstant->values[i].intValue =
                        LeftConstant->values[0].intValue - RightConstant->values[i].intValue;
                break;

            case clvBINARY_MUL:
                RightConstant->values[i].intValue =
                        LeftConstant->values[0].intValue * RightConstant->values[i].intValue;
                break;

            case clvBINARY_DIV:
                RightConstant->values[i].intValue =
                        LeftConstant->values[0].intValue / RightConstant->values[i].intValue;
                break;

            case clvBINARY_MOD:
                RightConstant->values[i].intValue =
                        LeftConstant->values[0].intValue % RightConstant->values[i].intValue;
                break;
            default:
                gcmASSERT(0);
                return gcvSTATUS_INVALID_ARGUMENT;
            }
        }
        break;

    case clvTYPE_UINT:
        for (i = 0; i < RightConstant->valueCount; i++)
        {
            switch (ExprType)
            {
            case clvBINARY_ADD:
                RightConstant->values[i].uintValue =
                        LeftConstant->values[0].uintValue + RightConstant->values[i].uintValue;
                break;

            case clvBINARY_SUB:
                RightConstant->values[i].intValue =
                        LeftConstant->values[0].uintValue - RightConstant->values[i].uintValue;
                break;

            case clvBINARY_MUL:
                RightConstant->values[i].uintValue =
                        LeftConstant->values[0].uintValue * RightConstant->values[i].uintValue;
                break;

            case clvBINARY_DIV:
                RightConstant->values[i].uintValue =
                        LeftConstant->values[0].uintValue / RightConstant->values[i].uintValue;
                break;

            case clvBINARY_MOD:
                RightConstant->values[i].uintValue =
                        LeftConstant->values[0].uintValue % RightConstant->values[i].uintValue;
                break;
            default:
                gcmASSERT(0);
                return gcvSTATUS_INVALID_ARGUMENT;
            }
        }
        break;

    case clvTYPE_LONG:
        for (i = 0; i < RightConstant->valueCount; i++)
        {
            switch (ExprType)
            {
            case clvBINARY_ADD:
                RightConstant->values[i].longValue =
                        LeftConstant->values[0].longValue + RightConstant->values[i].longValue;
                break;

            case clvBINARY_SUB:
                RightConstant->values[i].longValue =
                        LeftConstant->values[0].longValue - RightConstant->values[i].longValue;
                break;

            case clvBINARY_MUL:
                RightConstant->values[i].longValue =
                        LeftConstant->values[0].longValue * RightConstant->values[i].longValue;
                break;

            case clvBINARY_DIV:
                RightConstant->values[i].longValue =
                        LeftConstant->values[0].longValue / RightConstant->values[i].longValue;
                break;

            case clvBINARY_MOD:
                RightConstant->values[i].longValue =
                        LeftConstant->values[0].longValue % RightConstant->values[i].longValue;
                break;
            default:
                gcmASSERT(0);
                return gcvSTATUS_INVALID_ARGUMENT;
            }
        }
        break;

    case clvTYPE_ULONG:
        for (i = 0; i < RightConstant->valueCount; i++)
        {
            switch (ExprType)
            {
            case clvBINARY_ADD:
                RightConstant->values[i].ulongValue =
                        LeftConstant->values[0].ulongValue + RightConstant->values[i].ulongValue;
                break;

            case clvBINARY_SUB:
                RightConstant->values[i].longValue =
                        LeftConstant->values[0].ulongValue - RightConstant->values[i].ulongValue;
                break;

            case clvBINARY_MUL:
                RightConstant->values[i].ulongValue =
                        LeftConstant->values[0].ulongValue * RightConstant->values[i].ulongValue;
                break;

            case clvBINARY_DIV:
                RightConstant->values[i].ulongValue =
                        LeftConstant->values[0].ulongValue / RightConstant->values[i].ulongValue;
                break;

            case clvBINARY_MOD:
                RightConstant->values[i].ulongValue =
                        LeftConstant->values[0].ulongValue % RightConstant->values[i].ulongValue;
                break;
            default:
                gcmASSERT(0);
                return gcvSTATUS_INVALID_ARGUMENT;
            }
        }
        break;

    case clvTYPE_FLOAT:
        for (i = 0; i < RightConstant->valueCount; i++)
        {
            switch (ExprType)
            {
            case clvBINARY_ADD:
                RightConstant->values[i].floatValue =
                        LeftConstant->values[0].floatValue + RightConstant->values[i].floatValue;
                break;

            case clvBINARY_SUB:
                RightConstant->values[i].floatValue =
                        LeftConstant->values[0].floatValue - RightConstant->values[i].floatValue;
                break;

            case clvBINARY_MUL:
                RightConstant->values[i].floatValue =
                        LeftConstant->values[0].floatValue * RightConstant->values[i].floatValue;
                break;

            case clvBINARY_DIV:
                RightConstant->values[i].floatValue =
                        LeftConstant->values[0].floatValue / RightConstant->values[i].floatValue;
                break;

            default:
                gcmASSERT(0);
                return gcvSTATUS_INVALID_ARGUMENT;
            }
        }
        break;

    default:
        gcmASSERT(0);
        return gcvSTATUS_INVALID_ARGUMENT;
    }

    RightConstant->exprBase.base.lineNo        = LeftConstant->exprBase.base.lineNo;
    RightConstant->exprBase.base.stringNo    = LeftConstant->exprBase.base.stringNo;

    gcmVERIFY_OK(cloIR_OBJECT_Destroy(Compiler, &LeftConstant->exprBase.base));

    *ResultConstant = RightConstant;
    (*ResultConstant)->variable = gcvNULL;
    (*ResultConstant)->allValuesEqual = gcvFALSE;

    return gcvSTATUS_OK;
}

static gceSTATUS
_cloIR_CONSTANT_VectorOrMatrix_ArithmeticOperate_Scalar(
    IN cloCOMPILER Compiler,
    IN cleBINARY_EXPR_TYPE ExprType,
    IN cloIR_CONSTANT LeftConstant,
    IN cloIR_CONSTANT RightConstant,
    OUT cloIR_CONSTANT * ResultConstant
    )
{
    gctUINT        i;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(LeftConstant, clvIR_CONSTANT);
    clmVERIFY_IR_OBJECT(RightConstant, clvIR_CONSTANT);
    gcmASSERT(ResultConstant);

    gcmASSERT(LeftConstant->exprBase.decl.dataType);
    gcmASSERT(RightConstant->exprBase.decl.dataType);

#if gcdDEBUG
    if (clmDECL_IsIVec(&LeftConstant->exprBase.decl)
        || clmDECL_IsVec(&LeftConstant->exprBase.decl))
    {
        gcmASSERT(LeftConstant->valueCount == clmDATA_TYPE_vectorSize_GET(LeftConstant->exprBase.decl.dataType));
    }
    else if (clmDECL_IsMat(&LeftConstant->exprBase.decl))
    {
        gcmASSERT(LeftConstant->valueCount ==
                    (gctUINT) (clmDATA_TYPE_matrixRowCount_GET(LeftConstant->exprBase.decl.dataType)
                    * clmDATA_TYPE_matrixColumnCount_GET(LeftConstant->exprBase.decl.dataType)));
    }
    else
    {
        gcmASSERT(0);
        return gcvSTATUS_INVALID_ARGUMENT;
    }
#endif /* gcdDEBUG */

    gcmASSERT(RightConstant->valueCount == 1);


    switch (LeftConstant->exprBase.decl.dataType->elementType)
    {
    case clvTYPE_INT:
        for (i = 0; i < LeftConstant->valueCount; i++)
        {
            switch (ExprType)
            {
            case clvBINARY_ADD:
                LeftConstant->values[i].intValue += RightConstant->values[0].intValue;
                break;

            case clvBINARY_SUB:
                LeftConstant->values[i].intValue -= RightConstant->values[0].intValue;
                break;

            case clvBINARY_MUL:
                LeftConstant->values[i].intValue *= RightConstant->values[0].intValue;
                break;

            case clvBINARY_DIV:
                LeftConstant->values[i].intValue /= RightConstant->values[0].intValue;
                break;

            case clvBINARY_MOD:
                LeftConstant->values[i].intValue %= RightConstant->values[0].intValue;
                break;

            default:
                gcmASSERT(0);
                return gcvSTATUS_INVALID_ARGUMENT;
            }
        }
        break;

    case clvTYPE_UINT:
        for (i = 0; i < LeftConstant->valueCount; i++)
        {
            switch (ExprType)
            {
            case clvBINARY_ADD:
                LeftConstant->values[i].uintValue += RightConstant->values[0].uintValue;
                break;

            case clvBINARY_SUB:
                LeftConstant->values[i].uintValue -= RightConstant->values[0].uintValue;
                break;

            case clvBINARY_MUL:
                LeftConstant->values[i].uintValue *= RightConstant->values[0].uintValue;
                break;

            case clvBINARY_DIV:
                LeftConstant->values[i].uintValue /= RightConstant->values[0].uintValue;
                break;

            case clvBINARY_MOD:
                LeftConstant->values[i].uintValue %= RightConstant->values[0].uintValue;
                break;

            default:
                gcmASSERT(0);
                return gcvSTATUS_INVALID_ARGUMENT;
            }
        }
        break;

    case clvTYPE_LONG:
        for (i = 0; i < LeftConstant->valueCount; i++)
        {
            switch (ExprType)
            {
            case clvBINARY_ADD:
                LeftConstant->values[i].longValue += RightConstant->values[0].longValue;
                break;

            case clvBINARY_SUB:
                LeftConstant->values[i].longValue -= RightConstant->values[0].longValue;
                break;

            case clvBINARY_MUL:
                LeftConstant->values[i].longValue *= RightConstant->values[0].longValue;
                break;

            case clvBINARY_DIV:
                LeftConstant->values[i].longValue /= RightConstant->values[0].longValue;
                break;

            case clvBINARY_MOD:
                LeftConstant->values[i].longValue %= RightConstant->values[0].longValue;
                break;

            default:
                gcmASSERT(0);
                return gcvSTATUS_INVALID_ARGUMENT;
            }
        }
        break;

    case clvTYPE_ULONG:
        for (i = 0; i < LeftConstant->valueCount; i++)
        {
            switch (ExprType)
            {
            case clvBINARY_ADD:
                LeftConstant->values[i].ulongValue += RightConstant->values[0].ulongValue;
                break;

            case clvBINARY_SUB:
                LeftConstant->values[i].ulongValue -= RightConstant->values[0].ulongValue;
                break;

            case clvBINARY_MUL:
                LeftConstant->values[i].ulongValue *= RightConstant->values[0].ulongValue;
                break;

            case clvBINARY_DIV:
                LeftConstant->values[i].ulongValue /= RightConstant->values[0].ulongValue;
                break;

            case clvBINARY_MOD:
                LeftConstant->values[i].ulongValue %= RightConstant->values[0].ulongValue;
                break;

            default:
                gcmASSERT(0);
                return gcvSTATUS_INVALID_ARGUMENT;
            }
        }
        break;

    case clvTYPE_FLOAT:
        for (i = 0; i < LeftConstant->valueCount; i++)
        {
            switch (ExprType)
            {
            case clvBINARY_ADD:
                LeftConstant->values[i].floatValue += RightConstant->values[0].floatValue;
                break;

            case clvBINARY_SUB:
                LeftConstant->values[i].floatValue -= RightConstant->values[0].floatValue;
                break;

            case clvBINARY_MUL:
                LeftConstant->values[i].floatValue *= RightConstant->values[0].floatValue;
                break;

            case clvBINARY_DIV:
                LeftConstant->values[i].floatValue /= RightConstant->values[0].floatValue;
                break;

            default:
                gcmASSERT(0);
                return gcvSTATUS_INVALID_ARGUMENT;
            }
        }
        break;

    default:
        gcmASSERT(0);
        return gcvSTATUS_INVALID_ARGUMENT;
    }

    gcmVERIFY_OK(cloIR_OBJECT_Destroy(Compiler, &RightConstant->exprBase.base));

    *ResultConstant = LeftConstant;
    (*ResultConstant)->variable = gcvNULL;
    (*ResultConstant)->allValuesEqual = gcvFALSE;

    return gcvSTATUS_OK;
}

static gceSTATUS
_cloIR_CONSTANT_Subscript(
IN cloCOMPILER Compiler,
IN cloIR_CONSTANT LeftConstant,
IN cloIR_CONSTANT RightConstant,
IN clsDECL *Decl,
OUT cloIR_CONSTANT * ResultConstant
)
{
  gceSTATUS    status;
  gctSIZE_T    index;
  gctUINT8    i;
  gctSIZE_T elementCount = 1;
  cluCONSTANT_VALUE values[cldMAX_VECTOR_COMPONENT];
  cluCONSTANT_VALUE *valuePtr;

/* Verify the arguments. */
  clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
  clmVERIFY_IR_OBJECT(LeftConstant, clvIR_CONSTANT);
  clmVERIFY_IR_OBJECT(RightConstant, clvIR_CONSTANT);
  gcmASSERT(ResultConstant);
  gcmASSERT(Decl);

  gcmASSERT(LeftConstant->exprBase.decl.dataType);
  gcmASSERT(RightConstant->exprBase.decl.dataType);

  gcmASSERT(clmDECL_IsInt(&RightConstant->exprBase.decl));
  index = (gctSIZE_T)RightConstant->values[0].intValue;

  if (clmDECL_IsArray(&LeftConstant->exprBase.decl)) {
      elementCount = clsDECL_GetSize(Decl);
  }
  else if (!clmDECL_IsVectorType(&LeftConstant->exprBase.decl)) {
      gcmASSERT(!clmDECL_IsScalar(&LeftConstant->exprBase.decl));

      elementCount = clmDATA_TYPE_matrixRowCount_GET(LeftConstant->exprBase.decl.dataType);
  }

  if(LeftConstant->buffer) {
      gctINT elementByteSize;
      clsDECL elementDecl[1];

      gcmASSERT(LeftConstant->values == gcvNULL);
      gcmVERIFY_OK(cloCOMPILER_CreateElementDecl(Compiler,
                                              &LeftConstant->exprBase.decl,
                                              elementDecl));

      elementByteSize = clsDECL_GetElementByteSize(Compiler,
                                                   &LeftConstant->exprBase.decl,
                                                   gcvNULL,
                                                   gcvNULL);
      status = clConvFieldConstantToConstantValues(values,
                                                   elementDecl,
                                                   LeftConstant->buffer + elementByteSize * index);
      if(gcmIS_ERROR(status)) return status;
      valuePtr = values;

      gcmVERIFY_OK(cloCOMPILER_Free(Compiler, LeftConstant->buffer));
      LeftConstant->buffer = gcvNULL;
      gcmVERIFY_OK(cloCOMPILER_Allocate(Compiler,
                                        (gctSIZE_T)(sizeof(cluCONSTANT_VALUE) * elementCount),
                                        (gctPOINTER *) &LeftConstant->values));
  }
  else {
      valuePtr = LeftConstant->values + (index * elementCount);
  }

  gcmASSERT(LeftConstant->values);
  for (i = 0; i < elementCount; i++) {
      LeftConstant->values[i] = valuePtr[i];
  }
  LeftConstant->valueCount = elementCount;

  gcmVERIFY_OK(cloIR_OBJECT_Destroy(Compiler, &RightConstant->exprBase.base));

  status = cloCOMPILER_CloneDecl(Compiler,
                                 LeftConstant->exprBase.decl.dataType->accessQualifier,
                                 LeftConstant->exprBase.decl.dataType->addrSpaceQualifier,
                                 Decl,
                                 &LeftConstant->exprBase.decl);
  if(gcmIS_ERROR(status)) return status;

  *ResultConstant = LeftConstant;
  (*ResultConstant)->variable = gcvNULL;
  return gcvSTATUS_OK;
}

static gceSTATUS
_cloIR_GetTargetCastDecl(
IN cloCOMPILER Compiler,
IN clsDECL *NewDecl,
IN OUT clsDECL *ResultDecl
)
{
   gceSTATUS status;
   clsDATA_TYPE *orgDataType;
   gctINT resType = 0;

   gcmASSERT(ResultDecl);
   orgDataType = ResultDecl->dataType;
   if(!clmIsElementTypeArithmetic(NewDecl->dataType->elementType)) {
      if(clmDECL_IsPackedType(NewDecl) && clmDECL_IsScalar(ResultDecl)) {
          clsBUILTIN_DATATYPE_INFO *typeInfo;
          typeInfo = clGetBuiltinDataTypeInfo(NewDecl->dataType->type);
          resType = typeInfo->componentType;
      }
      else resType  = NewDecl->dataType->type;
   }
   else {
      gctINT8 vectorSize;
      gcmASSERT(orgDataType->matrixSize.columnCount == 0);

      vectorSize = clmDATA_TYPE_vectorSize_NOCHECK_GET(orgDataType);

      switch(vectorSize) {
      case 0: /* Scalar */
      case 2:
      case 3:
      case 4:
      case 8:
      case 16:
        resType = clGetVectorTerminalToken(NewDecl->dataType->elementType,
                                           vectorSize);
        break;

      default:
        break;
      }
   }

   gcmASSERT(resType);

   if(resType != orgDataType->type) {
      clsDATA_TYPE *newDataType;

      status =  cloCOMPILER_CreateDataType(Compiler,
                                           resType,
                                           orgDataType->u.generic,
                                           orgDataType->accessQualifier,
                                           orgDataType->addrSpaceQualifier,
                                           &newDataType);
      if(gcmIS_ERROR(status)) return status;
      ResultDecl->dataType = newDataType;
   }
   ResultDecl->ptrDscr = NewDecl->ptrDscr;
   return gcvSTATUS_OK;
}

static gceSTATUS
_cloIR_CONSTANT_EqualizeElementType(
    IN cloCOMPILER Compiler,
    IN cloIR_CONSTANT LeftConstant,
    IN cloIR_CONSTANT RightConstant
)
{
   gceSTATUS status = gcvSTATUS_OK;

   if(LeftConstant->exprBase.decl.dataType->elementType !=
      RightConstant->exprBase.decl.dataType->elementType) {
      if(LeftConstant->exprBase.decl.dataType->elementType > RightConstant->exprBase.decl.dataType->elementType) {
          status = clParseConstantTypeConvert(RightConstant,
                                              LeftConstant->exprBase.decl.dataType->elementType,
                                              RightConstant->values);
          if(gcmIS_ERROR(status)) return status;
          status = _cloIR_GetTargetCastDecl(Compiler,
                                            &LeftConstant->exprBase.decl,
                                            &RightConstant->exprBase.decl);
      }
      else {
          status = clParseConstantTypeConvert(LeftConstant,
                                              RightConstant->exprBase.decl.dataType->elementType,
                                              LeftConstant->values);
          if(gcmIS_ERROR(status)) return status;
          status = _cloIR_GetTargetCastDecl(Compiler,
                                            &RightConstant->exprBase.decl,
                                            &LeftConstant->exprBase.decl);
      }
   }
   if(gcmIS_ERROR(status)) return status;

   return gcvSTATUS_OK;
}


static gceSTATUS
_FetchLeftAndRightOperandValues(
    IN cloCOMPILER Compiler,
    IN cloIR_CONSTANT LeftConstant,
    IN cloIR_CONSTANT RightConstant,
    IN OUT cluCONSTANT_VALUE ** LeftValues,
    IN OUT cluCONSTANT_VALUE ** RightValues,
    OUT cloIR_CONSTANT *ResultConstant
    )
{
    gceSTATUS status;
    cluCONSTANT_VALUE *leftValuesPtr, *rightValuesPtr;
    cluCONSTANT_VALUE *valuesPtr = gcvNULL;
    cloIR_CONSTANT scalarConstant = gcvNULL;
    cloIR_CONSTANT resultConstant = gcvNULL;

    status = _cloIR_CONSTANT_EqualizeElementType(Compiler, LeftConstant, RightConstant);
    if(gcmIS_ERROR(status)) return status;

    leftValuesPtr = LeftConstant->values;
    rightValuesPtr = RightConstant->values;
    resultConstant = LeftConstant;
    if(clmDECL_IsScalar(&LeftConstant->exprBase.decl) ||
       clmDECL_IsVectorType(&RightConstant->exprBase.decl)) {
       scalarConstant = LeftConstant;
       resultConstant = RightConstant;
       valuesPtr = leftValuesPtr = *LeftValues;
    }
    else if(clmDECL_IsScalar(&RightConstant->exprBase.decl) ||
       clmDECL_IsVectorType(&LeftConstant->exprBase.decl)) {
       scalarConstant = RightConstant;
       valuesPtr = rightValuesPtr = *RightValues;
    }

    if(scalarConstant) {
        gctINT vectorSize;
        vectorSize = clsDECL_GetSize(&resultConstant->exprBase.decl);

        clmPromoteScalarConstantToVector(scalarConstant,
                                         vectorSize,
                                         valuesPtr);
    }
    *LeftValues = leftValuesPtr;
    *RightValues = rightValuesPtr;
    *ResultConstant = resultConstant;
    return gcvSTATUS_OK;
}

static gceSTATUS
_cloIR_CONSTANT_ArithmeticOperate(
    IN cloCOMPILER Compiler,
    IN cleBINARY_EXPR_TYPE ExprType,
    IN cloIR_CONSTANT LeftConstant,
    IN cloIR_CONSTANT RightConstant,
    OUT cloIR_CONSTANT * ResultConstant
    )
{
    gceSTATUS status;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(LeftConstant, clvIR_CONSTANT);
    clmVERIFY_IR_OBJECT(RightConstant, clvIR_CONSTANT);
    gcmASSERT(ResultConstant);


    status = _cloIR_CONSTANT_EqualizeElementType(Compiler, LeftConstant, RightConstant);
    if(gcmIS_ERROR(status)) return status;

    if (clsDECL_IsEqual(&LeftConstant->exprBase.decl, &RightConstant->exprBase.decl))
    {
        return _cloIR_CONSTANT_ArithmeticOperateBySameTypes(Compiler,
                                    ExprType,
                                    LeftConstant,
                                    RightConstant,
                                    ResultConstant);
    }
    else
    {
        if (clmDECL_IsInt(&LeftConstant->exprBase.decl))
        {
            gcmASSERT(clmDECL_IsIVec(&RightConstant->exprBase.decl));

            return _cloIR_CONSTANT_Scalar_ArithmeticOperate_VectorOrMatrix(Compiler,
                                               ExprType,
                                               LeftConstant,
                                               RightConstant,
                                               ResultConstant);
        }
        else if (clmDECL_IsIVec(&LeftConstant->exprBase.decl))
        {
            gcmASSERT(clmDECL_IsInt(&RightConstant->exprBase.decl));

            return _cloIR_CONSTANT_VectorOrMatrix_ArithmeticOperate_Scalar(Compiler,
                                            ExprType,
                                            LeftConstant,
                                            RightConstant,
                                            ResultConstant);
        }
        else if (clmDECL_IsFloat(&LeftConstant->exprBase.decl))
        {
            gcmASSERT(clmDECL_IsVecOrMat(&RightConstant->exprBase.decl));

            return _cloIR_CONSTANT_Scalar_ArithmeticOperate_VectorOrMatrix(Compiler,
                                            ExprType,
                                            LeftConstant,
                                            RightConstant,
                                            ResultConstant);
        }
        else if (clmDECL_IsVec(&LeftConstant->exprBase.decl))
        {
            if (ExprType != clvBINARY_MUL)
            {
                gcmASSERT(clmDECL_IsFloat(&RightConstant->exprBase.decl));

                return _cloIR_CONSTANT_VectorOrMatrix_ArithmeticOperate_Scalar(Compiler,
                                                ExprType,
                                                LeftConstant,
                                                RightConstant,
                                                ResultConstant);
            }
            else
            {
                if (clmDECL_IsFloat(&RightConstant->exprBase.decl))
                {
                    return _cloIR_CONSTANT_VectorOrMatrix_ArithmeticOperate_Scalar(Compiler,
                                                    ExprType,
                                                    LeftConstant,
                                                    RightConstant,
                                                    ResultConstant);
                }
                else
                {
                    gcmASSERT(clmDECL_IsSquareMat(&RightConstant->exprBase.decl)
                        && clmDATA_TYPE_vectorSize_GET(LeftConstant->exprBase.decl.dataType)
                                    == clmDATA_TYPE_matrixRowCount_GET(RightConstant->exprBase.decl.dataType));

                    return _cloIR_CONSTANT_Vec_Mul_Mat(Compiler,
                                    ExprType,
                                    LeftConstant,
                                    RightConstant,
                                    ResultConstant);
                }
            }
        }
        else if (clmDECL_IsMat(&LeftConstant->exprBase.decl))
        {
            if (ExprType != clvBINARY_MUL)
            {
                gcmASSERT(clmDECL_IsFloat(&RightConstant->exprBase.decl));

                return _cloIR_CONSTANT_VectorOrMatrix_ArithmeticOperate_Scalar(Compiler,
                                                ExprType,
                                                LeftConstant,
                                                RightConstant,
                                                ResultConstant);
            }
            else
            {
                if (clmDECL_IsFloat(&RightConstant->exprBase.decl))
                {
                    return _cloIR_CONSTANT_VectorOrMatrix_ArithmeticOperate_Scalar(Compiler,
                                                    ExprType,
                                                    LeftConstant,
                                                    RightConstant,
                                                    ResultConstant);
                }
                else
                {
                    gcmASSERT(clmDECL_IsVec(&RightConstant->exprBase.decl)
                        && clmDATA_TYPE_matrixColumnCount_GET(LeftConstant->exprBase.decl.dataType)
                                == clmDATA_TYPE_vectorSize_GET(RightConstant->exprBase.decl.dataType));

                    return _cloIR_CONSTANT_Mat_Mul_Vec(Compiler,
                                    ExprType,
                                    LeftConstant,
                                    RightConstant,
                                    ResultConstant);
                }
            }
        }
        else
        {
            gcmASSERT(0);
            return gcvSTATUS_INVALID_ARGUMENT;
        }
    }
}

static gceSTATUS
_cloIR_CONSTANT_RelationalOperate(
IN cloCOMPILER Compiler,
IN cleBINARY_EXPR_TYPE ExprType,
IN cloIR_CONSTANT LeftConstant,
IN cloIR_CONSTANT RightConstant,
OUT cloIR_CONSTANT * ResultConstant
)
{
    gceSTATUS    status;
    cluCONSTANT_VALUE leftValueBuf[cldMAX_VECTOR_COMPONENT];
    cluCONSTANT_VALUE rightValueBuf[cldMAX_VECTOR_COMPONENT];
    cluCONSTANT_VALUE *leftValues, *rightValues;
    int vectorSize, i;
    cloIR_CONSTANT resultConstant;
    cloIR_CONSTANT otherConstant;
    gctINT trueValue;
    clsDATA_TYPE resDataType[1];

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(LeftConstant, clvIR_CONSTANT);
    clmVERIFY_IR_OBJECT(RightConstant, clvIR_CONSTANT);
    gcmASSERT(ResultConstant);

    gcmASSERT(LeftConstant->exprBase.decl.dataType);
    gcmASSERT(RightConstant->exprBase.decl.dataType);
    gcmASSERT(clmDECL_IsIntOrIVec(&LeftConstant->exprBase.decl)
            || clmDECL_IsFloatOrVec(&LeftConstant->exprBase.decl));

    leftValues = leftValueBuf;
    rightValues = rightValueBuf;
    status = _FetchLeftAndRightOperandValues(Compiler,
                                             LeftConstant,
                                             RightConstant,
                                             &leftValues,
                                             &rightValues,
                                             &resultConstant);
    if(gcmIS_ERROR(status)) return status;
    if(resultConstant == LeftConstant) {
        otherConstant = RightConstant;
    }
    else {
        otherConstant = LeftConstant;
    }

    if(clmDECL_IsScalar(&resultConstant->exprBase.decl)) {
        trueValue = 1;
    }
    else {
        trueValue = -1;
    }
    /* Calculate the value */
    vectorSize = clsDECL_GetSize(&resultConstant->exprBase.decl);
    if (clmDECL_IsInt(&resultConstant->exprBase.decl)) {
        if (clmDATA_TYPE_IsHighPrecision(resultConstant->exprBase.decl.dataType)) {
            switch (ExprType) {
            case clvBINARY_GREATER_THAN:
                for(i=0; i < vectorSize; i++) {
                    resultConstant->values[i].longValue =
                        (leftValues[i].longValue > rightValues[i].longValue) ? trueValue : 0;
                }
                break;

            case clvBINARY_LESS_THAN:
                for(i=0; i < vectorSize; i++) {
                    resultConstant->values[i].longValue =
                        (leftValues[i].longValue < rightValues[i].longValue) ? trueValue : 0;
                }
                break;

            case clvBINARY_GREATER_THAN_EQUAL:
                for(i=0; i < vectorSize; i++) {
                    resultConstant->values[i].longValue =
                        (leftValues[i].longValue >= rightValues[i].longValue) ? trueValue : 0;
                }
                break;

            case clvBINARY_LESS_THAN_EQUAL:
                for(i=0; i < vectorSize; i++) {
                    resultConstant->values[i].longValue =
                        (leftValues[i].longValue <= rightValues[i].longValue) ? trueValue : 0;
                }

                break;

            default:
                gcmASSERT(0);
                return gcvSTATUS_INVALID_ARGUMENT;
            }
        }
        else {
            switch (ExprType) {
            case clvBINARY_GREATER_THAN:
                for(i=0; i < vectorSize; i++) {
                    resultConstant->values[i].longValue =
                        (leftValues[i].intValue > rightValues[i].intValue) ? trueValue : 0;
                }
                break;

            case clvBINARY_LESS_THAN:
                for(i=0; i < vectorSize; i++) {
                    resultConstant->values[i].longValue =
                        (leftValues[i].intValue < rightValues[i].intValue) ? trueValue : 0;
                }
                break;

            case clvBINARY_GREATER_THAN_EQUAL:
                for(i=0; i < vectorSize; i++) {
                    resultConstant->values[i].longValue =
                        (leftValues[i].intValue >= rightValues[i].intValue) ? trueValue : 0;
                }
                break;

            case clvBINARY_LESS_THAN_EQUAL:
                for(i=0; i < vectorSize; i++) {
                    resultConstant->values[i].longValue =
                        (leftValues[i].intValue <= rightValues[i].intValue) ? trueValue : 0;
                }
                break;

            default:
                gcmASSERT(0);
                return gcvSTATUS_INVALID_ARGUMENT;
            }
        }
    }
    else { /* floating point type */
        switch (ExprType) {
        case clvBINARY_GREATER_THAN:
            for (i = 0; i < vectorSize; i++) {
                /* Check for NaN special case */
                if ((leftValues[i].intValue  == 0x7fc00000)
                 || (rightValues[i].intValue == 0x7fc00000)) {
                    resultConstant->values[i].longValue = 0;
                    continue;
                }
                resultConstant->values[i].longValue =
                    (leftValues[i].floatValue > rightValues[i].floatValue) ? trueValue : 0;
            }
            break;

        case clvBINARY_LESS_THAN:
            for (i = 0; i < vectorSize; i++) {
                /* Check for NaN special case */
                if ((leftValues[i].intValue  == 0x7fc00000)
                 || (rightValues[i].intValue == 0x7fc00000)) {
                    resultConstant->values[i].longValue = 0;
                    continue;
                }
                resultConstant->values[i].longValue =
                    (leftValues[i].floatValue < rightValues[i].floatValue) ? trueValue : 0;
            }
            break;

        case clvBINARY_GREATER_THAN_EQUAL:
            for (i = 0; i < vectorSize; i++) {
                /* Check for NaN special case */
                if ((leftValues[i].intValue  == 0x7fc00000)
                 || (rightValues[i].intValue == 0x7fc00000)) {
                    resultConstant->values[i].longValue = 0;
                    continue;
                }
                resultConstant->values[i].longValue =
                    (leftValues[i].floatValue >= rightValues[i].floatValue) ? trueValue : 0;
            }
            break;

        case clvBINARY_LESS_THAN_EQUAL:
            for (i = 0; i < vectorSize; i++) {
                /* Check for NaN special case */
                if ((leftValues[i].intValue  == 0x7fc00000)
                 || (rightValues[i].intValue == 0x7fc00000)) {
                    resultConstant->values[i].longValue = 0;
                    continue;
                }
                resultConstant->values[i].longValue =
                    (leftValues[i].floatValue <= rightValues[i].floatValue) ? trueValue : 0;
            }
            break;

        default:
            gcmASSERT(0);
            return gcvSTATUS_INVALID_ARGUMENT;
        }
    }

    /* set resulting type to integer */
    if (clmDATA_TYPE_IsHighPrecision(resultConstant->exprBase.decl.dataType)) {
        _clmInitGenType(clvTYPE_LONG,
                        clmDATA_TYPE_vectorSize_NOCHECK_GET(resultConstant->exprBase.decl.dataType),
                        resDataType);
    }
    else {
        _clmInitGenType(clvTYPE_INT,
                        clmDATA_TYPE_vectorSize_NOCHECK_GET(resultConstant->exprBase.decl.dataType),
                        resDataType);
    }

    status = cloCOMPILER_CloneDataType(Compiler,
                                       resultConstant->exprBase.decl.dataType->accessQualifier,
                                       resultConstant->exprBase.decl.dataType->addrSpaceQualifier,
                                       resDataType,
                                       &resultConstant->exprBase.decl.dataType);

    gcmVERIFY_OK(cloIR_OBJECT_Destroy(Compiler, &otherConstant->exprBase.base));

    *ResultConstant = resultConstant;
    (*ResultConstant)->variable = gcvNULL;
    (*ResultConstant)->allValuesEqual = gcvFALSE;

    return status;
}

static gceSTATUS
_cloIR_CONSTANT_EqualityOperate(
IN cloCOMPILER Compiler,
IN cleBINARY_EXPR_TYPE ExprType,
IN cloIR_CONSTANT LeftConstant,
IN cloIR_CONSTANT RightConstant,
OUT cloIR_CONSTANT * ResultConstant
)
{
    gceSTATUS    status;
    cluCONSTANT_VALUE leftValueBuf[cldMAX_VECTOR_COMPONENT];
    cluCONSTANT_VALUE rightValueBuf[cldMAX_VECTOR_COMPONENT];
    cluCONSTANT_VALUE *leftValues, *rightValues;
    int vectorSize, i;
    cloIR_CONSTANT resultConstant;
    cloIR_CONSTANT otherConstant;
    gctINT trueValue;
    clsDATA_TYPE resDataType[1];

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(LeftConstant, clvIR_CONSTANT);
    clmVERIFY_IR_OBJECT(RightConstant, clvIR_CONSTANT);
    gcmASSERT(ResultConstant);

    gcmASSERT(LeftConstant->exprBase.decl.dataType);
    gcmASSERT(RightConstant->exprBase.decl.dataType);
    gcmASSERT(clsDECL_IsAssignableAndComparable(&LeftConstant->exprBase.decl));
    gcmASSERT(clsDECL_IsEqual(&LeftConstant->exprBase.decl,
                       &RightConstant->exprBase.decl));
    gcmASSERT(LeftConstant->valueCount == RightConstant->valueCount);

    leftValues = leftValueBuf;
    rightValues = rightValueBuf;
    status = _FetchLeftAndRightOperandValues(Compiler,
                                             LeftConstant,
                                             RightConstant,
                                             &leftValues,
                                             &rightValues,
                                             &resultConstant);
    if(gcmIS_ERROR(status)) return status;
    if(resultConstant == LeftConstant) {
        otherConstant = RightConstant;
    }
    else {
        otherConstant = LeftConstant;
    }

    if(clmDECL_IsScalar(&resultConstant->exprBase.decl)) {
        trueValue = 1;
    }
    else {
        trueValue = -1;
    }

    /* Calculate the value */
    vectorSize = clsDECL_GetSize(&resultConstant->exprBase.decl);
    if (clmDECL_IsInt(&resultConstant->exprBase.decl)) {
        if (clmDATA_TYPE_IsHighPrecision(resultConstant->exprBase.decl.dataType)) {
           switch (ExprType) {
           case clvBINARY_EQUAL:
               for (i = 0; i < vectorSize; i++) {
                   resultConstant->values[i].longValue =
                       (leftValues[i].longValue == rightValues[i].longValue) ? trueValue : 0;
               }
               break;

           case clvBINARY_NOT_EQUAL:
               for (i = 0; i < vectorSize; i++) {
                   resultConstant->values[i].longValue =
                       (leftValues[i].longValue != rightValues[i].longValue) ? trueValue : 0;
               }
               break;

           default:
               gcmASSERT(0);
               return gcvSTATUS_INVALID_ARGUMENT;
           }
        }
        else {
           switch (ExprType) {
           case clvBINARY_EQUAL:
               for (i = 0; i < vectorSize; i++) {
                   resultConstant->values[i].longValue =
                       (leftValues[i].intValue == rightValues[i].intValue) ? trueValue : 0;
               }
               break;

           case clvBINARY_NOT_EQUAL:
               for (i = 0; i < vectorSize; i++) {
                   resultConstant->values[i].longValue =
                       (leftValues[i].intValue != rightValues[i].intValue) ? trueValue : 0;
               }
               break;

           default:
               gcmASSERT(0);
               return gcvSTATUS_INVALID_ARGUMENT;
           }
        }
    }
    else { /* floating point type */
       switch (ExprType) {
       case clvBINARY_EQUAL:
           for (i = 0; i < vectorSize; i++) {
               resultConstant->values[i].longValue =
                   (leftValues[i].floatValue == rightValues[i].floatValue) ? trueValue : 0;
           }
           break;

       case clvBINARY_NOT_EQUAL:
           for (i = 0; i < vectorSize; i++) {
               resultConstant->values[i].longValue =
                   (leftValues[i].floatValue != rightValues[i].floatValue) ? trueValue : 0;
           }
           break;

       default:
           gcmASSERT(0);
           return gcvSTATUS_INVALID_ARGUMENT;
       }
    }

    /* set resulting type to integer */
    if (clmDATA_TYPE_IsHighPrecision(resultConstant->exprBase.decl.dataType)) {
        _clmInitGenType(clvTYPE_LONG,
                        clmDATA_TYPE_vectorSize_NOCHECK_GET(resultConstant->exprBase.decl.dataType),
                        resDataType);
    }
    else {
        _clmInitGenType(clvTYPE_INT,
                        clmDATA_TYPE_vectorSize_NOCHECK_GET(resultConstant->exprBase.decl.dataType),
                        resDataType);
    }

    status = cloCOMPILER_CloneDataType(Compiler,
                                       resultConstant->exprBase.decl.dataType->accessQualifier,
                                       resultConstant->exprBase.decl.dataType->addrSpaceQualifier,
                                       resDataType,
                                       &resultConstant->exprBase.decl.dataType);

    gcmVERIFY_OK(cloIR_OBJECT_Destroy(Compiler, &otherConstant->exprBase.base));

    *ResultConstant = resultConstant;
    (*ResultConstant)->variable = gcvNULL;
    (*ResultConstant)->allValuesEqual = gcvFALSE;

    return status;
}

static gceSTATUS
_cloIR_CONSTANT_LogicalOperate(
    IN cloCOMPILER Compiler,
    IN cleBINARY_EXPR_TYPE ExprType,
    IN cloIR_CONSTANT LeftConstant,
    IN cloIR_CONSTANT RightConstant,
    OUT cloIR_CONSTANT * ResultConstant
    )
{
    gceSTATUS status;
    cluCONSTANT_VALUE leftValueBuf[cldMAX_VECTOR_COMPONENT];
    cluCONSTANT_VALUE rightValueBuf[cldMAX_VECTOR_COMPONENT];
    cluCONSTANT_VALUE *leftValues, *rightValues;
    int vectorSize, i;
    cloIR_CONSTANT resultConstant;
    cloIR_CONSTANT otherConstant;
    gctINT trueValue;
    clsDATA_TYPE resDataType[1];

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(LeftConstant, clvIR_CONSTANT);
    clmVERIFY_IR_OBJECT(RightConstant, clvIR_CONSTANT);
    gcmASSERT(ResultConstant);

    gcmASSERT(LeftConstant->exprBase.decl.dataType);
    gcmASSERT(RightConstant->exprBase.decl.dataType);

    gcmVERIFY_OK(clParseConstantTypeConvert(LeftConstant,
                                            clvTYPE_BOOL,
                                            LeftConstant->values));
    gcmVERIFY_OK(clParseConstantTypeConvert(RightConstant,
                                            clvTYPE_BOOL,
                                            RightConstant->values));

    leftValues = leftValueBuf;
    rightValues = rightValueBuf;
    status = _FetchLeftAndRightOperandValues(Compiler,
                                             LeftConstant,
                                             RightConstant,
                                             &leftValues,
                                             &rightValues,
                                             &resultConstant);
    if(gcmIS_ERROR(status)) return status;
    if(resultConstant == LeftConstant) {
        otherConstant = RightConstant;
    }
    else {
        otherConstant = LeftConstant;
    }

    if(clmDECL_IsScalar(&resultConstant->exprBase.decl)) {
        trueValue = 1;
    }
    else {
        trueValue = -1;
    }

    /* Calculate the value */
    vectorSize = clsDECL_GetSize(&resultConstant->exprBase.decl);
    switch (ExprType)
    {
    case clvBINARY_AND:
        for(i=0; i < vectorSize; i++) {
            resultConstant->values[i].longValue =
                (leftValues[i].boolValue && rightValues[i].boolValue) ? trueValue : 0;
        }
        break;

    case clvBINARY_OR:
        for(i=0; i < vectorSize; i++) {
            resultConstant->values[i].longValue =
                (leftValues[i].boolValue || rightValues[i].boolValue) ? trueValue : 0;
        }
        break;

    case clvBINARY_XOR:
        for(i=0; i < vectorSize; i++) {
            resultConstant->values[i].longValue =
                (leftValues[i].boolValue != rightValues[i].boolValue) ? trueValue : 0;
        }
        break;

    default:
        gcmASSERT(0);
        return gcvSTATUS_INVALID_ARGUMENT;
    }

    /* set resulting type to integer */
    if (clmDATA_TYPE_IsHighPrecision(resultConstant->exprBase.decl.dataType)) {
        _clmInitGenType(clvTYPE_LONG,
                        clmDATA_TYPE_vectorSize_NOCHECK_GET(resultConstant->exprBase.decl.dataType),
                        resDataType);
    }
    else {
        _clmInitGenType(clvTYPE_INT,
                        clmDATA_TYPE_vectorSize_NOCHECK_GET(resultConstant->exprBase.decl.dataType),
                        resDataType);
    }

    status = cloCOMPILER_CloneDataType(Compiler,
                                       resultConstant->exprBase.decl.dataType->accessQualifier,
                                       resultConstant->exprBase.decl.dataType->addrSpaceQualifier,
                                       resDataType,
                                       &resultConstant->exprBase.decl.dataType);

    gcmVERIFY_OK(cloIR_OBJECT_Destroy(Compiler, &otherConstant->exprBase.base));

    *ResultConstant = resultConstant;
    (*ResultConstant)->variable = gcvNULL;
    (*ResultConstant)->allValuesEqual = gcvFALSE;

    return status;
}

static gceSTATUS
_cloIR_CONSTANT_BitwiseLogical(
    IN cloCOMPILER Compiler,
    IN cleBINARY_EXPR_TYPE ExprType,
    IN cloIR_CONSTANT LeftConstant,
    IN cloIR_CONSTANT RightConstant,
    OUT cloIR_CONSTANT * ResultConstant
    )
{
    gceSTATUS status;
    cluCONSTANT_VALUE leftValueBuf[cldMAX_VECTOR_COMPONENT];
    cluCONSTANT_VALUE rightValueBuf[cldMAX_VECTOR_COMPONENT];
    cluCONSTANT_VALUE *leftValues, *rightValues;
    int vectorSize, i;
    cloIR_CONSTANT resultConstant;
    cloIR_CONSTANT otherConstant;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(LeftConstant, clvIR_CONSTANT);
    clmVERIFY_IR_OBJECT(RightConstant, clvIR_CONSTANT);
    gcmASSERT(ResultConstant);

    gcmASSERT(LeftConstant->exprBase.decl.dataType);
    gcmASSERT(RightConstant->exprBase.decl.dataType);
    gcmASSERT(clmDECL_IsIntOrIVec(&LeftConstant->exprBase.decl) ||
          clmDECL_IsSampler(&LeftConstant->exprBase.decl));
    gcmASSERT(clmDECL_IsIntOrIVec(&RightConstant->exprBase.decl) ||
          clmDECL_IsSampler(&RightConstant->exprBase.decl));

    leftValues = leftValueBuf;
    rightValues = rightValueBuf;
    status = _FetchLeftAndRightOperandValues(Compiler,
                                             LeftConstant,
                                             RightConstant,
                                             &leftValues,
                                             &rightValues,
                                             &resultConstant);
    if(gcmIS_ERROR(status)) return status;
    if(resultConstant == LeftConstant) {
        otherConstant = RightConstant;
    }
    else {
        otherConstant = LeftConstant;
    }

    vectorSize = clsDECL_GetSize(&resultConstant->exprBase.decl);
    if (clmDATA_TYPE_IsHighPrecision(resultConstant->exprBase.decl.dataType)) {
       switch (ExprType) {
       case clvBINARY_AND_BITWISE:
           for(i=0; i < vectorSize; i++) {
              resultConstant->values[i].longValue = leftValues[i].longValue & rightValues[i].longValue;
           }
           break;

       case clvBINARY_OR_BITWISE:
           for(i=0; i < vectorSize; i++) {
              resultConstant->values[i].longValue = leftValues[i].longValue | rightValues[i].longValue;
           }
           break;

       case clvBINARY_XOR_BITWISE:
           for(i=0; i < vectorSize; i++) {
              resultConstant->values[i].longValue = leftValues[i].longValue ^ rightValues[i].longValue;
           }
           break;

       default:
           gcmASSERT(0);
           return gcvSTATUS_INVALID_ARGUMENT;
       }
    }
    else {
       switch (ExprType) {
       case clvBINARY_AND_BITWISE:
           for(i=0; i < vectorSize; i++) {
              resultConstant->values[i].intValue = leftValues[i].intValue & rightValues[i].intValue;
           }
           break;

       case clvBINARY_OR_BITWISE:
           for(i=0; i < vectorSize; i++) {
              resultConstant->values[i].intValue = leftValues[i].intValue | rightValues[i].intValue;
           }
           break;

       case clvBINARY_XOR_BITWISE:
           for(i=0; i < vectorSize; i++) {
              resultConstant->values[i].intValue = leftValues[i].intValue ^ rightValues[i].intValue;
           }
           break;

       default:
           gcmASSERT(0);
           return gcvSTATUS_INVALID_ARGUMENT;
       }
    }

    gcmVERIFY_OK(cloIR_OBJECT_Destroy(Compiler, &otherConstant->exprBase.base));

    *ResultConstant = resultConstant;
    (*ResultConstant)->variable = gcvNULL;
    (*ResultConstant)->allValuesEqual = gcvFALSE;

    return gcvSTATUS_OK;
}

static gceSTATUS
_cloIR_CONSTANT_BitwiseShift(
    IN cloCOMPILER Compiler,
    IN cleBINARY_EXPR_TYPE ExprType,
    IN cloIR_CONSTANT LeftConstant,
    IN cloIR_CONSTANT RightConstant,
    OUT cloIR_CONSTANT * ResultConstant
    )
{
    cluCONSTANT_VALUE values[cldMAX_VECTOR_COMPONENT];
    cluCONSTANT_VALUE *valuesPtr;
    int vectorSize, i;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(LeftConstant, clvIR_CONSTANT);
    clmVERIFY_IR_OBJECT(RightConstant, clvIR_CONSTANT);
    gcmASSERT(ResultConstant);

    gcmASSERT(LeftConstant->exprBase.decl.dataType);
    gcmASSERT(RightConstant->exprBase.decl.dataType);
    gcmASSERT(clmDECL_IsIntOrIVec(&LeftConstant->exprBase.decl));
    gcmASSERT(clmDECL_IsIntOrIVec(&RightConstant->exprBase.decl));

    vectorSize = clsDECL_GetSize(&LeftConstant->exprBase.decl);
    if(clmDECL_IsIVec(&LeftConstant->exprBase.decl) &&
       !clmDECL_IsIVec(&RightConstant->exprBase.decl)) {
       clmPromoteIntToVector(RightConstant->values[0].longValue,
                             vectorSize,
                             values);
       valuesPtr = values;
    }
    else {
       valuesPtr = RightConstant->values;
    }

    if(clmIsElementTypeUnsigned(LeftConstant->exprBase.decl.dataType->elementType)) {
      switch (ExprType) {
      case clvBINARY_RSHIFT:
        for(i=0; i < vectorSize; i++) {
           LeftConstant->values[i].ulongValue >>= valuesPtr[i].intValue;
        }
        break;

      case clvBINARY_LSHIFT:
        for(i=0; i < vectorSize; i++) {
           LeftConstant->values[i].ulongValue <<= valuesPtr[i].intValue;
        }
        break;

      default:
        gcmASSERT(0);
        return gcvSTATUS_INVALID_ARGUMENT;
      }
    }
    else {
      switch (ExprType) {
      case clvBINARY_RSHIFT:
        for(i=0; i < vectorSize; i++) {
           LeftConstant->values[i].longValue >>= valuesPtr[i].intValue;
        }
        break;

      case clvBINARY_LSHIFT:
        for(i=0; i < vectorSize; i++) {
           LeftConstant->values[i].longValue <<= valuesPtr[i].intValue;
        }
        break;

      default:
        gcmASSERT(0);
        return gcvSTATUS_INVALID_ARGUMENT;
      }
    }

    gcmVERIFY_OK(cloIR_OBJECT_Destroy(Compiler, &RightConstant->exprBase.base));

    *ResultConstant = LeftConstant;
    (*ResultConstant)->variable = gcvNULL;
    (*ResultConstant)->allValuesEqual = gcvFALSE;

    return gcvSTATUS_OK;
}

gceSTATUS
clConvFieldConstantToConstantValues(
cluCONSTANT_VALUE *Values,
clsDECL *Decl,
gctSTRING Buffer
)
{
    gctUINT8 vectorSize, i;
    gctSTRING ptr;

    gcoOS_ZeroMemory(Values, gcmSIZEOF(cluCONSTANT_VALUE) * cldMAX_VECTOR_COMPONENT);

    ptr = Buffer;
    vectorSize = clmDATA_TYPE_vectorSize_GET(Decl->dataType);
    if(vectorSize == 0) vectorSize = 1;
    switch(Decl->dataType->elementType) {
    case clvTYPE_CHAR:
    case clvTYPE_CHAR_PACKED:
        for(i = 0; i < vectorSize; i++, ptr++) {
            Values[i].intValue = *((gctINT8 *) ptr);
        }
        break;

    case clvTYPE_UCHAR:
    case clvTYPE_UCHAR_PACKED:
    case clvTYPE_BOOL_PACKED:
        for(i = 0; i < vectorSize; i++, ptr++) {
            Values[i].uintValue = *((gctUINT8 *) ptr);
        }
        break;

    case clvTYPE_SHORT:
    case clvTYPE_SHORT_PACKED:
        for(i = 0; i < vectorSize; i++, ptr += 2) {
            Values[i].intValue = *((gctINT16 *) ptr);
        }
        break;

    case clvTYPE_USHORT:
    case clvTYPE_USHORT_PACKED:
        for(i = 0; i < vectorSize; i++, ptr += 2) {
            Values[i].uintValue = *((gctUINT16 *) ptr);
        }
        break;

    case clvTYPE_INT:
        for(i = 0; i < vectorSize; i++, ptr += 4) {
            Values[i].intValue = *((gctINT32 *) ptr);
        }
        break;

    case clvTYPE_UINT:
    case clvTYPE_BOOL:
        for(i = 0; i < vectorSize; i++, ptr += 4) {
            Values[i].uintValue = *((gctUINT32 *) ptr);
        }
        break;

    case clvTYPE_FLOAT:
        for(i = 0; i < vectorSize; i++, ptr += 4) {
            Values[i].floatValue = *((gctFLOAT *) ptr);
        }
        break;

    default:
        gcmASSERT(0);
        return gcvSTATUS_INVALID_ARGUMENT;
    }
    return gcvSTATUS_OK;
}

gctSIZE_T
clGetFieldByteOffset(
    IN cloCOMPILER Compiler,
    IN clsDECL * StructDecl,
    IN clsNAME * FieldName,
    OUT gctUINT * Alignment
    )
{
   gctSIZE_T    offset = 0;
   clsNAME *    fieldName;
   gctUINT alignment;
   gctBOOL packed = gcvFALSE;
   clsNAME_SPACE *fieldSpace;

   gcmASSERT(StructDecl);
   gcmASSERT(clmDATA_TYPE_IsStructOrUnion(StructDecl->dataType));
   gcmASSERT(FieldName);

   fieldSpace = StructDecl->dataType->u.fieldSpace;
   gcmASSERT(fieldSpace);

   if(fieldSpace->scopeName->u.typeInfo.hasUnnamedFields) {
       FOR_EACH_DLINK_NODE(&fieldSpace->names, clsNAME, fieldName)
       {
          if (fieldName == FieldName) break;
          gcmASSERT(fieldName->decl.dataType);

          if(fieldName->symbol[0] == '\0') { /* unnamed field */
              if (clmDATA_TYPE_IsStructOrUnion(fieldName->decl.dataType)) {
                  clsNAME * fld;

                  FOR_EACH_DLINK_NODE(&fieldName->decl.dataType->u.fieldSpace->names, clsNAME, fld)
                  {
                      if (fld == FieldName) {
                          fieldName = fld;
                          goto Found;
                      }

                      if(fld->u.variableInfo.specifiedAttr & clvATTR_PACKED) {
                          packed = gcvTRUE;
                      }
                      else {
                          packed = gcvFALSE;
                      }
                      if(fld->decl.dataType->elementType == clvTYPE_UNION) continue;
                      else {
                          if(fld->u.variableInfo.specifiedAttr & clvATTR_ALIGNED) {
                             alignment = fld->context.alignment;
                          }
                          else {
                             if(clmDECL_IsUnderlyingStructOrUnion(&fld->decl)) {
                                 clsNAME *subField;
                                 subField = slsDLINK_LIST_First(&fld->decl.dataType->u.fieldSpace->names, struct _clsNAME);
                                 if(subField->u.variableInfo.specifiedAttr & clvATTR_ALIGNED) {
                                    alignment = subField->context.alignment;
                                 }
                                 else alignment = clPermissibleAlignment(Compiler, &fld->decl);
                             }
                             else alignment = clPermissibleAlignment(Compiler, &fld->decl);
                          }
                          offset = clmALIGN(offset, alignment, packed) + clsDECL_GetByteSize(Compiler, &fld->decl);
                      }
                  }
              }
          }
          else {
              if(fieldName->u.variableInfo.specifiedAttr & clvATTR_PACKED) {
                 packed = gcvTRUE;
              }
              else {
                 packed = gcvFALSE;
              }
              if(StructDecl->dataType->elementType == clvTYPE_UNION) continue;
              else {
                  if(fieldName->u.variableInfo.specifiedAttr & clvATTR_ALIGNED) {
                     alignment = fieldName->context.alignment;
                  }
                  else {
                     if(clmDECL_IsUnderlyingStructOrUnion(&fieldName->decl)) {
                        clsNAME *subField;
                        subField = slsDLINK_LIST_First(&fieldName->decl.dataType->u.fieldSpace->names, struct _clsNAME);
                        if(subField->u.variableInfo.specifiedAttr & clvATTR_ALIGNED) {
                           alignment = subField->context.alignment;
                        }
                        else alignment = clPermissibleAlignment(Compiler, &fieldName->decl);
                     }
                     else alignment = clPermissibleAlignment(Compiler, &fieldName->decl);
                  }
                  offset = clmALIGN(offset, alignment, packed) + clsDECL_GetByteSize(Compiler, &fieldName->decl);
              }
          }
       }
Found:;
   }
   else {
       FOR_EACH_DLINK_NODE(&fieldSpace->names, clsNAME, fieldName)
       {
          if (fieldName == FieldName) break;
          gcmASSERT(fieldName->decl.dataType);

          if(fieldName->u.variableInfo.specifiedAttr & clvATTR_PACKED) {
             packed = gcvTRUE;
          }
          else {
             packed = gcvFALSE;
          }
          if(StructDecl->dataType->elementType == clvTYPE_UNION) continue;
          else {
            if(fieldName->u.variableInfo.specifiedAttr & clvATTR_ALIGNED) {
               alignment = fieldName->context.alignment;
            }
            else {
               if(clmDECL_IsUnderlyingStructOrUnion(&fieldName->decl)) {
                  clsNAME *subField;
                  subField = slsDLINK_LIST_First(&fieldName->decl.dataType->u.fieldSpace->names, struct _clsNAME);
                  if(subField->u.variableInfo.specifiedAttr & clvATTR_ALIGNED) {
                     alignment = subField->context.alignment;
                  }
                  else alignment = clPermissibleAlignment(Compiler, &fieldName->decl);
               }
               else alignment = clPermissibleAlignment(Compiler, &fieldName->decl);
            }
            offset = clmALIGN(offset, alignment, packed) + clsDECL_GetByteSize(Compiler, &fieldName->decl);
          }
       }
   }

   gcmASSERT(fieldName == FieldName);

   if(FieldName->u.variableInfo.specifiedAttr & clvATTR_PACKED) {
      packed = gcvTRUE;
   }
   else {
      packed = gcvFALSE;
   }
   if(FieldName->u.variableInfo.specifiedAttr & clvATTR_ALIGNED) {
       alignment = FieldName->context.alignment;
   }
   else {
       if(clmDECL_IsUnderlyingStructOrUnion(&FieldName->decl)) {
          clsNAME *subField;
          subField = slsDLINK_LIST_First(&fieldName->decl.dataType->u.fieldSpace->names, struct _clsNAME);
          if(subField->u.variableInfo.specifiedAttr & clvATTR_ALIGNED) {
             alignment = subField->context.alignment;
          }
          else alignment = clPermissibleAlignment(Compiler, &FieldName->decl);
       }
       else alignment = clPermissibleAlignment(Compiler, &FieldName->decl);
   }

   if(Alignment) {
       *Alignment = alignment;
   }
   return clmALIGN(offset, alignment, packed);
}

static gceSTATUS
_cloIR_CONSTANT_SelectField(
IN cloCOMPILER Compiler,
IN cloIR_CONSTANT Constant,
IN clsNAME * FieldName,
OUT cloIR_CONSTANT * FieldConstant
)
{
    gceSTATUS status;
    clsDECL    decl;
    cloIR_CONSTANT    fieldConstant = gcvNULL;
    gctSIZE_T    size, offset;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(Constant, clvIR_CONSTANT);
    gcmASSERT(Constant->exprBase.decl.dataType);
    gcmASSERT(clmDATA_TYPE_IsStructOrUnion(Constant->exprBase.decl.dataType));
    gcmASSERT(FieldName);
    gcmASSERT(FieldName->decl.dataType);
    gcmASSERT(FieldConstant);

    do {
        status = cloCOMPILER_CloneDecl(Compiler,
                        clvQUALIFIER_CONST,
                        FieldName->decl.dataType->addrSpaceQualifier,
                        &FieldName->decl,
                        &decl);
        if (gcmIS_ERROR(status)) break;

        status = cloIR_CONSTANT_Allocate(Compiler,
                        Constant->exprBase.base.lineNo,
                        Constant->exprBase.base.stringNo,
                        &decl,
                        &fieldConstant);
        if (gcmIS_ERROR(status)) break;

        if(_GEN_UNIFORMS_FOR_CONSTANT_ADDRESS_SPACE_VARIABLES) {
            if(clmDECL_IsAggregateType(&FieldName->decl)) {
                offset = clGetFieldByteOffset(Compiler, &Constant->exprBase.decl, FieldName, gcvNULL);
                gcoOS_MemCopy(fieldConstant->buffer,
                              Constant->buffer + offset,
                              clsDECL_GetByteSize(Compiler,
                                                  &FieldName->decl));
            }
            else {
                cluCONSTANT_VALUE values[cldMAX_VECTOR_COMPONENT];

                size = clsDECL_GetSize(&FieldName->decl);
                gcmASSERT(size > 0);

                offset = clsDECL_GetFieldOffset(&Constant->exprBase.decl, FieldName);
                status = clConvFieldConstantToConstantValues(values,
                                                             &FieldName->decl,
                                                             Constant->buffer + offset);
                if (gcmIS_ERROR(status)) break;
                gcoOS_MemCopy(fieldConstant->values,
                              values,
                              (gctSIZE_T)sizeof(cluCONSTANT_VALUE) * size);
            }
        }
        else {
            size = clsDECL_GetSize(&FieldName->decl);
            gcmASSERT(size > 0);

            offset = clsDECL_GetFieldOffset(&Constant->exprBase.decl, FieldName);
            gcoOS_MemCopy(fieldConstant->values,
                          Constant->values + offset,
                          (gctSIZE_T)sizeof(cluCONSTANT_VALUE) * size);
        }

        gcmVERIFY_OK(cloIR_OBJECT_Destroy(Compiler, &Constant->exprBase.base));

        *FieldConstant = fieldConstant;
        return gcvSTATUS_OK;
    } while (gcvFALSE);

    if(fieldConstant) {
        gcmVERIFY_OK(cloIR_OBJECT_Destroy(Compiler, &fieldConstant->exprBase.base));
    }
    *FieldConstant = gcvNULL;
    return status;
}

static gceSTATUS
_cloIR_CONSTANT_SelectComponents(
IN cloCOMPILER Compiler,
IN cloIR_CONSTANT Constant,
IN clsCOMPONENT_SELECTION * ComponentSelection,
OUT cloIR_CONSTANT * ResultConstant
)
{
    gceSTATUS status;
    gctUINT8 i;
    cluCONSTANT_VALUE  values[4];
    gctINT resultType;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(Constant, clvIR_CONSTANT);
    gcmASSERT(Constant->exprBase.decl.dataType);
    gcmASSERT(clmDECL_IsBVecOrIVecOrVec(&Constant->exprBase.decl));
    gcmASSERT(ComponentSelection);
    gcmASSERT(clmDATA_TYPE_vectorSize_GET(Constant->exprBase.decl.dataType) >= ComponentSelection->components);
    gcmASSERT(ResultConstant);

    /* Swizzle the components */
    gcmASSERT(ComponentSelection->components <= cldMaxComponentCount);
    for (i = 0; i < ComponentSelection->components; i++) {
       values[i] = Constant->values[ComponentSelection->selection[i]];
    }

    /* Change to the result constant */
    if (!((gctUINT)Constant->exprBase.decl.dataType->elementType <  _BuiltinVectorTypeCount))
    {
        gcmASSERT(gcvFALSE);
    }
    resultType = clGetVectorTerminalToken(Constant->exprBase.decl.dataType->elementType,
                                          ComponentSelection->components);

    if(resultType) {
       status = cloCOMPILER_CreateDecl(Compiler,
                                       resultType,
                                       Constant->exprBase.decl.dataType->u.generic,
                                       clvQUALIFIER_CONST,
                                       clvQUALIFIER_NONE,
                                       &Constant->exprBase.decl);
       if (gcmIS_ERROR(status)) return status;
    }
    else {
      /* klc to do */
/*Function to create data type for internal vector types */
    }

    Constant->valueCount = ComponentSelection->components;

    for (i = 0; i < ComponentSelection->components; i++) {
        Constant->values[i] = values[i];
    }

    *ResultConstant = Constant;
    (*ResultConstant)->variable = gcvNULL;
    return gcvSTATUS_OK;
}

/* cloIR_UNARY_EXPR object. */
gceSTATUS
cloIR_UNARY_EXPR_Destroy(
    IN cloCOMPILER Compiler,
    IN cloIR_BASE This
    )
{
    cloIR_UNARY_EXPR    unaryExpr = (cloIR_UNARY_EXPR)This;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(unaryExpr, clvIR_UNARY_EXPR);

    gcmASSERT(unaryExpr->operand);
    gcmVERIFY_OK(cloIR_OBJECT_Destroy(Compiler, &unaryExpr->operand->base));

    if (unaryExpr->exprBase.asmMods != gcvNULL)
    {
        gcmVERIFY_OK(cloCOMPILER_Free(Compiler, unaryExpr->exprBase.asmMods));
    }

    gcmVERIFY_OK(cloCOMPILER_Free(Compiler, unaryExpr));

    return gcvSTATUS_OK;
}

clsCOMPONENT_SELECTION    ComponentSelection_X =
        {1, {clvCOMPONENT_X, clvCOMPONENT_X, clvCOMPONENT_X, clvCOMPONENT_X,
             clvCOMPONENT_X, clvCOMPONENT_X, clvCOMPONENT_X, clvCOMPONENT_X,
             clvCOMPONENT_X, clvCOMPONENT_X, clvCOMPONENT_X, clvCOMPONENT_X,
             clvCOMPONENT_X, clvCOMPONENT_X, clvCOMPONENT_X, clvCOMPONENT_X,
             clvCOMPONENT_X, clvCOMPONENT_X, clvCOMPONENT_X, clvCOMPONENT_X,
             clvCOMPONENT_X, clvCOMPONENT_X, clvCOMPONENT_X, clvCOMPONENT_X,
             clvCOMPONENT_X, clvCOMPONENT_X, clvCOMPONENT_X, clvCOMPONENT_X,
             clvCOMPONENT_X, clvCOMPONENT_X, clvCOMPONENT_X, clvCOMPONENT_X}};

clsCOMPONENT_SELECTION    ComponentSelection_Y =
        {1, {clvCOMPONENT_Y, clvCOMPONENT_Y, clvCOMPONENT_Y, clvCOMPONENT_Y,
             clvCOMPONENT_Y, clvCOMPONENT_Y, clvCOMPONENT_Y, clvCOMPONENT_Y,
             clvCOMPONENT_Y, clvCOMPONENT_Y, clvCOMPONENT_Y, clvCOMPONENT_Y,
             clvCOMPONENT_Y, clvCOMPONENT_Y, clvCOMPONENT_Y, clvCOMPONENT_Y,
             clvCOMPONENT_Y, clvCOMPONENT_Y, clvCOMPONENT_Y, clvCOMPONENT_Y,
             clvCOMPONENT_Y, clvCOMPONENT_Y, clvCOMPONENT_Y, clvCOMPONENT_Y,
             clvCOMPONENT_Y, clvCOMPONENT_Y, clvCOMPONENT_Y, clvCOMPONENT_Y,
             clvCOMPONENT_Y, clvCOMPONENT_Y, clvCOMPONENT_Y, clvCOMPONENT_Y}};

clsCOMPONENT_SELECTION    ComponentSelection_Z =
        {1, {clvCOMPONENT_Z, clvCOMPONENT_Z, clvCOMPONENT_Z, clvCOMPONENT_Z,
                     clvCOMPONENT_Z, clvCOMPONENT_Z, clvCOMPONENT_Z, clvCOMPONENT_Z,
                     clvCOMPONENT_Z, clvCOMPONENT_Z, clvCOMPONENT_Z, clvCOMPONENT_Z,
                     clvCOMPONENT_Z, clvCOMPONENT_Z, clvCOMPONENT_Z, clvCOMPONENT_Z,
                     clvCOMPONENT_Z, clvCOMPONENT_Z, clvCOMPONENT_Z, clvCOMPONENT_Z,
                     clvCOMPONENT_Z, clvCOMPONENT_Z, clvCOMPONENT_Z, clvCOMPONENT_Z,
                     clvCOMPONENT_Z, clvCOMPONENT_Z, clvCOMPONENT_Z, clvCOMPONENT_Z,
                     clvCOMPONENT_Z, clvCOMPONENT_Z, clvCOMPONENT_Z, clvCOMPONENT_Z}};

clsCOMPONENT_SELECTION    ComponentSelection_W =
        {1, {clvCOMPONENT_W, clvCOMPONENT_W, clvCOMPONENT_W, clvCOMPONENT_W,
                     clvCOMPONENT_W, clvCOMPONENT_W, clvCOMPONENT_W, clvCOMPONENT_W,
                     clvCOMPONENT_W, clvCOMPONENT_W, clvCOMPONENT_W, clvCOMPONENT_W,
                     clvCOMPONENT_W, clvCOMPONENT_W, clvCOMPONENT_W, clvCOMPONENT_W,
                     clvCOMPONENT_W, clvCOMPONENT_W, clvCOMPONENT_W, clvCOMPONENT_W,
                     clvCOMPONENT_W, clvCOMPONENT_W, clvCOMPONENT_W, clvCOMPONENT_W,
                     clvCOMPONENT_W, clvCOMPONENT_W, clvCOMPONENT_W, clvCOMPONENT_W,
                     clvCOMPONENT_W, clvCOMPONENT_W, clvCOMPONENT_W, clvCOMPONENT_W}};

clsCOMPONENT_SELECTION    ComponentSelection_XY =
        {2, {clvCOMPONENT_X, clvCOMPONENT_Y, clvCOMPONENT_Y, clvCOMPONENT_Y,
                     clvCOMPONENT_Y, clvCOMPONENT_Y, clvCOMPONENT_Y, clvCOMPONENT_Y,
                     clvCOMPONENT_Y, clvCOMPONENT_Y, clvCOMPONENT_Y, clvCOMPONENT_Y,
                     clvCOMPONENT_Y, clvCOMPONENT_Y, clvCOMPONENT_Y, clvCOMPONENT_Y,
                     clvCOMPONENT_Y, clvCOMPONENT_Y, clvCOMPONENT_Y, clvCOMPONENT_Y,
                     clvCOMPONENT_Y, clvCOMPONENT_Y, clvCOMPONENT_Y, clvCOMPONENT_Y,
                     clvCOMPONENT_Y, clvCOMPONENT_Y, clvCOMPONENT_Y, clvCOMPONENT_Y,
                     clvCOMPONENT_Y, clvCOMPONENT_Y, clvCOMPONENT_Y, clvCOMPONENT_Y}};

clsCOMPONENT_SELECTION    ComponentSelection_XYZ =
        {3, {clvCOMPONENT_X, clvCOMPONENT_Y, clvCOMPONENT_Z, clvCOMPONENT_Z,
                     clvCOMPONENT_Z, clvCOMPONENT_Z, clvCOMPONENT_Z, clvCOMPONENT_Z,
                     clvCOMPONENT_Z, clvCOMPONENT_Z, clvCOMPONENT_Z, clvCOMPONENT_Z,
                     clvCOMPONENT_Z, clvCOMPONENT_Z, clvCOMPONENT_Z, clvCOMPONENT_Z,
                     clvCOMPONENT_Z, clvCOMPONENT_Z, clvCOMPONENT_Z, clvCOMPONENT_Z,
                     clvCOMPONENT_Z, clvCOMPONENT_Z, clvCOMPONENT_Z, clvCOMPONENT_Z,
                     clvCOMPONENT_Z, clvCOMPONENT_Z, clvCOMPONENT_Z, clvCOMPONENT_Z,
                     clvCOMPONENT_Z, clvCOMPONENT_Z, clvCOMPONENT_Z, clvCOMPONENT_Z}};

clsCOMPONENT_SELECTION    ComponentSelection_XYZW =
        {4, {clvCOMPONENT_X, clvCOMPONENT_Y, clvCOMPONENT_Z, clvCOMPONENT_W,
                     clvCOMPONENT_W, clvCOMPONENT_W, clvCOMPONENT_W, clvCOMPONENT_W,
                     clvCOMPONENT_W, clvCOMPONENT_W, clvCOMPONENT_W, clvCOMPONENT_W,
                     clvCOMPONENT_W, clvCOMPONENT_W, clvCOMPONENT_W, clvCOMPONENT_W,
                     clvCOMPONENT_W, clvCOMPONENT_W, clvCOMPONENT_W, clvCOMPONENT_W,
                     clvCOMPONENT_W, clvCOMPONENT_W, clvCOMPONENT_W, clvCOMPONENT_W,
                     clvCOMPONENT_W, clvCOMPONENT_W, clvCOMPONENT_W, clvCOMPONENT_W,
                     clvCOMPONENT_W, clvCOMPONENT_W, clvCOMPONENT_W, clvCOMPONENT_W}};

clsCOMPONENT_SELECTION    ComponentSelection_4 =
        {1, {clvCOMPONENT_4, clvCOMPONENT_4, clvCOMPONENT_4, clvCOMPONENT_4,
                     clvCOMPONENT_4, clvCOMPONENT_4, clvCOMPONENT_4, clvCOMPONENT_4,
                     clvCOMPONENT_4, clvCOMPONENT_4, clvCOMPONENT_4, clvCOMPONENT_4,
                     clvCOMPONENT_4, clvCOMPONENT_4, clvCOMPONENT_4, clvCOMPONENT_4,
                     clvCOMPONENT_4, clvCOMPONENT_4, clvCOMPONENT_4, clvCOMPONENT_4,
                     clvCOMPONENT_4, clvCOMPONENT_4, clvCOMPONENT_4, clvCOMPONENT_4,
                     clvCOMPONENT_4, clvCOMPONENT_4, clvCOMPONENT_4, clvCOMPONENT_4,
                     clvCOMPONENT_4, clvCOMPONENT_4, clvCOMPONENT_4, clvCOMPONENT_4}};

clsCOMPONENT_SELECTION    ComponentSelection_5 =
        {1, {clvCOMPONENT_5, clvCOMPONENT_5, clvCOMPONENT_5, clvCOMPONENT_5,
                     clvCOMPONENT_5, clvCOMPONENT_5, clvCOMPONENT_5, clvCOMPONENT_5,
                     clvCOMPONENT_5, clvCOMPONENT_5, clvCOMPONENT_5, clvCOMPONENT_5,
                     clvCOMPONENT_5, clvCOMPONENT_5, clvCOMPONENT_5, clvCOMPONENT_5,
                     clvCOMPONENT_5, clvCOMPONENT_5, clvCOMPONENT_5, clvCOMPONENT_5,
                     clvCOMPONENT_5, clvCOMPONENT_5, clvCOMPONENT_5, clvCOMPONENT_5,
                     clvCOMPONENT_5, clvCOMPONENT_5, clvCOMPONENT_5, clvCOMPONENT_5,
                     clvCOMPONENT_5, clvCOMPONENT_5, clvCOMPONENT_5, clvCOMPONENT_5}};

clsCOMPONENT_SELECTION    ComponentSelection_6 =
        {1, {clvCOMPONENT_6, clvCOMPONENT_6, clvCOMPONENT_6, clvCOMPONENT_6,
                     clvCOMPONENT_6, clvCOMPONENT_6, clvCOMPONENT_6, clvCOMPONENT_6,
                     clvCOMPONENT_6, clvCOMPONENT_6, clvCOMPONENT_6, clvCOMPONENT_6,
                     clvCOMPONENT_6, clvCOMPONENT_6, clvCOMPONENT_6, clvCOMPONENT_6,
                     clvCOMPONENT_6, clvCOMPONENT_6, clvCOMPONENT_6, clvCOMPONENT_6,
                     clvCOMPONENT_6, clvCOMPONENT_6, clvCOMPONENT_6, clvCOMPONENT_6,
                     clvCOMPONENT_6, clvCOMPONENT_6, clvCOMPONENT_6, clvCOMPONENT_6,
                     clvCOMPONENT_6, clvCOMPONENT_6, clvCOMPONENT_6, clvCOMPONENT_6}};

clsCOMPONENT_SELECTION    ComponentSelection_7 =
        {1, {clvCOMPONENT_7, clvCOMPONENT_7, clvCOMPONENT_7, clvCOMPONENT_7,
                     clvCOMPONENT_7, clvCOMPONENT_7, clvCOMPONENT_7, clvCOMPONENT_7,
                     clvCOMPONENT_7, clvCOMPONENT_7, clvCOMPONENT_7, clvCOMPONENT_7,
                     clvCOMPONENT_7, clvCOMPONENT_7, clvCOMPONENT_7, clvCOMPONENT_7,
                     clvCOMPONENT_7, clvCOMPONENT_7, clvCOMPONENT_7, clvCOMPONENT_7,
                     clvCOMPONENT_7, clvCOMPONENT_7, clvCOMPONENT_7, clvCOMPONENT_7,
                     clvCOMPONENT_7, clvCOMPONENT_7, clvCOMPONENT_7, clvCOMPONENT_7,
                     clvCOMPONENT_7, clvCOMPONENT_7, clvCOMPONENT_7, clvCOMPONENT_7}};

clsCOMPONENT_SELECTION    ComponentSelection_8 =
        {1, {clvCOMPONENT_8, clvCOMPONENT_8, clvCOMPONENT_8, clvCOMPONENT_8,
                     clvCOMPONENT_8, clvCOMPONENT_8, clvCOMPONENT_8, clvCOMPONENT_8,
                     clvCOMPONENT_8, clvCOMPONENT_8, clvCOMPONENT_8, clvCOMPONENT_8,
                     clvCOMPONENT_8, clvCOMPONENT_8, clvCOMPONENT_8, clvCOMPONENT_8,
                     clvCOMPONENT_8, clvCOMPONENT_8, clvCOMPONENT_8, clvCOMPONENT_8,
                     clvCOMPONENT_8, clvCOMPONENT_8, clvCOMPONENT_8, clvCOMPONENT_8,
                     clvCOMPONENT_8, clvCOMPONENT_8, clvCOMPONENT_8, clvCOMPONENT_8,
                     clvCOMPONENT_8, clvCOMPONENT_8, clvCOMPONENT_8, clvCOMPONENT_8}};

clsCOMPONENT_SELECTION    ComponentSelection_9 =
        {1, {clvCOMPONENT_9, clvCOMPONENT_9, clvCOMPONENT_9, clvCOMPONENT_9,
                     clvCOMPONENT_9, clvCOMPONENT_9, clvCOMPONENT_9, clvCOMPONENT_9,
                     clvCOMPONENT_9, clvCOMPONENT_9, clvCOMPONENT_9, clvCOMPONENT_9,
                     clvCOMPONENT_9, clvCOMPONENT_9, clvCOMPONENT_9, clvCOMPONENT_9,
                     clvCOMPONENT_9, clvCOMPONENT_9, clvCOMPONENT_9, clvCOMPONENT_9,
                     clvCOMPONENT_9, clvCOMPONENT_9, clvCOMPONENT_9, clvCOMPONENT_9,
                     clvCOMPONENT_9, clvCOMPONENT_9, clvCOMPONENT_9, clvCOMPONENT_9,
                     clvCOMPONENT_9, clvCOMPONENT_9, clvCOMPONENT_9, clvCOMPONENT_9}};

clsCOMPONENT_SELECTION    ComponentSelection_10 =
        {1, {clvCOMPONENT_10, clvCOMPONENT_10, clvCOMPONENT_10, clvCOMPONENT_10,
                     clvCOMPONENT_10, clvCOMPONENT_10, clvCOMPONENT_10, clvCOMPONENT_10,
                     clvCOMPONENT_10, clvCOMPONENT_10, clvCOMPONENT_10, clvCOMPONENT_10,
                     clvCOMPONENT_10, clvCOMPONENT_10, clvCOMPONENT_10, clvCOMPONENT_10,
                     clvCOMPONENT_10, clvCOMPONENT_10, clvCOMPONENT_10, clvCOMPONENT_10,
                     clvCOMPONENT_10, clvCOMPONENT_10, clvCOMPONENT_10, clvCOMPONENT_10,
                     clvCOMPONENT_10, clvCOMPONENT_10, clvCOMPONENT_10, clvCOMPONENT_10,
                     clvCOMPONENT_10, clvCOMPONENT_10, clvCOMPONENT_10, clvCOMPONENT_10}};

clsCOMPONENT_SELECTION    ComponentSelection_11 =
        {1, {clvCOMPONENT_11, clvCOMPONENT_11, clvCOMPONENT_11, clvCOMPONENT_11,
                     clvCOMPONENT_11, clvCOMPONENT_11, clvCOMPONENT_11, clvCOMPONENT_11,
                     clvCOMPONENT_11, clvCOMPONENT_11, clvCOMPONENT_11, clvCOMPONENT_11,
                     clvCOMPONENT_11, clvCOMPONENT_11, clvCOMPONENT_11, clvCOMPONENT_11,
                     clvCOMPONENT_11, clvCOMPONENT_11, clvCOMPONENT_11, clvCOMPONENT_11,
                     clvCOMPONENT_11, clvCOMPONENT_11, clvCOMPONENT_11, clvCOMPONENT_11,
                     clvCOMPONENT_11, clvCOMPONENT_11, clvCOMPONENT_11, clvCOMPONENT_11,
                     clvCOMPONENT_11, clvCOMPONENT_11, clvCOMPONENT_11, clvCOMPONENT_11}};

clsCOMPONENT_SELECTION    ComponentSelection_12 =
        {1, {clvCOMPONENT_12, clvCOMPONENT_12, clvCOMPONENT_12, clvCOMPONENT_12,
                     clvCOMPONENT_12, clvCOMPONENT_12, clvCOMPONENT_12, clvCOMPONENT_12,
                     clvCOMPONENT_12, clvCOMPONENT_12, clvCOMPONENT_12, clvCOMPONENT_12,
                     clvCOMPONENT_12, clvCOMPONENT_12, clvCOMPONENT_12, clvCOMPONENT_12,
                     clvCOMPONENT_12, clvCOMPONENT_12, clvCOMPONENT_12, clvCOMPONENT_12,
                     clvCOMPONENT_12, clvCOMPONENT_12, clvCOMPONENT_12, clvCOMPONENT_12,
                     clvCOMPONENT_12, clvCOMPONENT_12, clvCOMPONENT_12, clvCOMPONENT_12,
                     clvCOMPONENT_12, clvCOMPONENT_12, clvCOMPONENT_12, clvCOMPONENT_12}};

clsCOMPONENT_SELECTION    ComponentSelection_13 =
        {1, {clvCOMPONENT_13, clvCOMPONENT_13, clvCOMPONENT_13, clvCOMPONENT_13,
                     clvCOMPONENT_13, clvCOMPONENT_13, clvCOMPONENT_13, clvCOMPONENT_13,
                     clvCOMPONENT_13, clvCOMPONENT_13, clvCOMPONENT_13, clvCOMPONENT_13,
                     clvCOMPONENT_13, clvCOMPONENT_13, clvCOMPONENT_13, clvCOMPONENT_13,
                     clvCOMPONENT_13, clvCOMPONENT_13, clvCOMPONENT_13, clvCOMPONENT_13,
                     clvCOMPONENT_13, clvCOMPONENT_13, clvCOMPONENT_13, clvCOMPONENT_13,
                     clvCOMPONENT_13, clvCOMPONENT_13, clvCOMPONENT_13, clvCOMPONENT_13,
                     clvCOMPONENT_13, clvCOMPONENT_13, clvCOMPONENT_13, clvCOMPONENT_13}};

clsCOMPONENT_SELECTION    ComponentSelection_14 =
        {1, {clvCOMPONENT_14, clvCOMPONENT_14, clvCOMPONENT_14, clvCOMPONENT_14,
                     clvCOMPONENT_14, clvCOMPONENT_14, clvCOMPONENT_14, clvCOMPONENT_14,
                     clvCOMPONENT_14, clvCOMPONENT_14, clvCOMPONENT_14, clvCOMPONENT_14,
                     clvCOMPONENT_14, clvCOMPONENT_14, clvCOMPONENT_14, clvCOMPONENT_14,
                     clvCOMPONENT_14, clvCOMPONENT_14, clvCOMPONENT_14, clvCOMPONENT_14,
                     clvCOMPONENT_14, clvCOMPONENT_14, clvCOMPONENT_14, clvCOMPONENT_14,
                     clvCOMPONENT_14, clvCOMPONENT_14, clvCOMPONENT_14, clvCOMPONENT_14,
                     clvCOMPONENT_14, clvCOMPONENT_14, clvCOMPONENT_14, clvCOMPONENT_14}};

clsCOMPONENT_SELECTION    ComponentSelection_15 =
        {1, {clvCOMPONENT_15, clvCOMPONENT_15, clvCOMPONENT_15, clvCOMPONENT_15,
                     clvCOMPONENT_15, clvCOMPONENT_15, clvCOMPONENT_15, clvCOMPONENT_15,
                     clvCOMPONENT_15, clvCOMPONENT_15, clvCOMPONENT_15, clvCOMPONENT_15,
                     clvCOMPONENT_15, clvCOMPONENT_15, clvCOMPONENT_15, clvCOMPONENT_15,
                     clvCOMPONENT_15, clvCOMPONENT_15, clvCOMPONENT_15, clvCOMPONENT_15,
                     clvCOMPONENT_15, clvCOMPONENT_15, clvCOMPONENT_15, clvCOMPONENT_15,
                     clvCOMPONENT_15, clvCOMPONENT_15, clvCOMPONENT_15, clvCOMPONENT_15,
                     clvCOMPONENT_15, clvCOMPONENT_15, clvCOMPONENT_15, clvCOMPONENT_15}};

clsCOMPONENT_SELECTION    ComponentSelection_X_DBL =
        {2, {clvCOMPONENT_X, clvCOMPONENT_Y, clvCOMPONENT_Y, clvCOMPONENT_Y,
             clvCOMPONENT_Y, clvCOMPONENT_Y, clvCOMPONENT_Y, clvCOMPONENT_Y,
             clvCOMPONENT_Y, clvCOMPONENT_Y, clvCOMPONENT_Y, clvCOMPONENT_Y,
             clvCOMPONENT_Y, clvCOMPONENT_Y, clvCOMPONENT_Y, clvCOMPONENT_Y,
             clvCOMPONENT_Y, clvCOMPONENT_Y, clvCOMPONENT_Y, clvCOMPONENT_Y,
             clvCOMPONENT_Y, clvCOMPONENT_Y, clvCOMPONENT_Y, clvCOMPONENT_Y,
             clvCOMPONENT_Y, clvCOMPONENT_Y, clvCOMPONENT_Y, clvCOMPONENT_Y,
             clvCOMPONENT_Y, clvCOMPONENT_Y, clvCOMPONENT_Y, clvCOMPONENT_Y}};

clsCOMPONENT_SELECTION    ComponentSelection_Y_DBL =
        {2, {clvCOMPONENT_X, clvCOMPONENT_Y, clvCOMPONENT_Z, clvCOMPONENT_Z,
             clvCOMPONENT_Z, clvCOMPONENT_Z, clvCOMPONENT_Z, clvCOMPONENT_Z,
             clvCOMPONENT_Z, clvCOMPONENT_Z, clvCOMPONENT_Z, clvCOMPONENT_Z,
             clvCOMPONENT_Z, clvCOMPONENT_Z, clvCOMPONENT_Z, clvCOMPONENT_Z,
             clvCOMPONENT_Z, clvCOMPONENT_Z, clvCOMPONENT_Z, clvCOMPONENT_Z,
             clvCOMPONENT_Z, clvCOMPONENT_Z, clvCOMPONENT_Z, clvCOMPONENT_Z,
             clvCOMPONENT_Z, clvCOMPONENT_Z, clvCOMPONENT_Z, clvCOMPONENT_Z,
             clvCOMPONENT_Z, clvCOMPONENT_Z, clvCOMPONENT_Z, clvCOMPONENT_Z}};

clsCOMPONENT_SELECTION    ComponentSelection_Z_DBL =
        {2, {clvCOMPONENT_4, clvCOMPONENT_5, clvCOMPONENT_5, clvCOMPONENT_5,
                     clvCOMPONENT_5, clvCOMPONENT_5, clvCOMPONENT_5, clvCOMPONENT_5,
                     clvCOMPONENT_5, clvCOMPONENT_5, clvCOMPONENT_5, clvCOMPONENT_5,
                     clvCOMPONENT_5, clvCOMPONENT_5, clvCOMPONENT_5, clvCOMPONENT_5,
             clvCOMPONENT_5, clvCOMPONENT_5, clvCOMPONENT_5, clvCOMPONENT_5,
                     clvCOMPONENT_5, clvCOMPONENT_5, clvCOMPONENT_5, clvCOMPONENT_5,
                     clvCOMPONENT_5, clvCOMPONENT_5, clvCOMPONENT_5, clvCOMPONENT_5,
                     clvCOMPONENT_5, clvCOMPONENT_5, clvCOMPONENT_5, clvCOMPONENT_5}};

clsCOMPONENT_SELECTION    ComponentSelection_W_DBL =
        {2, {clvCOMPONENT_6, clvCOMPONENT_7, clvCOMPONENT_7, clvCOMPONENT_7,
                     clvCOMPONENT_7, clvCOMPONENT_7, clvCOMPONENT_7, clvCOMPONENT_7,
                     clvCOMPONENT_7, clvCOMPONENT_7, clvCOMPONENT_7, clvCOMPONENT_7,
                     clvCOMPONENT_7, clvCOMPONENT_7, clvCOMPONENT_7, clvCOMPONENT_7,
             clvCOMPONENT_7, clvCOMPONENT_7, clvCOMPONENT_7, clvCOMPONENT_7,
                     clvCOMPONENT_7, clvCOMPONENT_7, clvCOMPONENT_7, clvCOMPONENT_7,
                     clvCOMPONENT_7, clvCOMPONENT_7, clvCOMPONENT_7, clvCOMPONENT_7,
                     clvCOMPONENT_7, clvCOMPONENT_7, clvCOMPONENT_7, clvCOMPONENT_7}};

clsCOMPONENT_SELECTION    ComponentSelection_XY_DBL =
        {4, {clvCOMPONENT_X, clvCOMPONENT_Y, clvCOMPONENT_Z, clvCOMPONENT_W,
                     clvCOMPONENT_W, clvCOMPONENT_W, clvCOMPONENT_W, clvCOMPONENT_W,
                     clvCOMPONENT_W, clvCOMPONENT_W, clvCOMPONENT_W, clvCOMPONENT_W,
                     clvCOMPONENT_W, clvCOMPONENT_W, clvCOMPONENT_W, clvCOMPONENT_W,
             clvCOMPONENT_W, clvCOMPONENT_W, clvCOMPONENT_W, clvCOMPONENT_W,
                     clvCOMPONENT_W, clvCOMPONENT_W, clvCOMPONENT_W, clvCOMPONENT_W,
                     clvCOMPONENT_W, clvCOMPONENT_W, clvCOMPONENT_W, clvCOMPONENT_W,
                     clvCOMPONENT_W, clvCOMPONENT_W, clvCOMPONENT_W, clvCOMPONENT_W}};

clsCOMPONENT_SELECTION    ComponentSelection_XYZ_DBL =
        {6, {clvCOMPONENT_X, clvCOMPONENT_Y, clvCOMPONENT_Z, clvCOMPONENT_W,
                     clvCOMPONENT_4, clvCOMPONENT_5, clvCOMPONENT_5, clvCOMPONENT_5,
                     clvCOMPONENT_5, clvCOMPONENT_5, clvCOMPONENT_5, clvCOMPONENT_5,
                     clvCOMPONENT_5, clvCOMPONENT_5, clvCOMPONENT_5, clvCOMPONENT_5,
             clvCOMPONENT_5, clvCOMPONENT_5, clvCOMPONENT_5, clvCOMPONENT_5,
                     clvCOMPONENT_5, clvCOMPONENT_5, clvCOMPONENT_5, clvCOMPONENT_5,
                     clvCOMPONENT_5, clvCOMPONENT_5, clvCOMPONENT_5, clvCOMPONENT_5,
                     clvCOMPONENT_5, clvCOMPONENT_5, clvCOMPONENT_5, clvCOMPONENT_5}};

clsCOMPONENT_SELECTION    ComponentSelection_XYZW_DBL =
        {8, {clvCOMPONENT_X, clvCOMPONENT_Y, clvCOMPONENT_Z, clvCOMPONENT_W,
                     clvCOMPONENT_4, clvCOMPONENT_5, clvCOMPONENT_6, clvCOMPONENT_7,
                     clvCOMPONENT_7, clvCOMPONENT_7, clvCOMPONENT_7, clvCOMPONENT_7,
                     clvCOMPONENT_7, clvCOMPONENT_7, clvCOMPONENT_7, clvCOMPONENT_7,
             clvCOMPONENT_7, clvCOMPONENT_7, clvCOMPONENT_7, clvCOMPONENT_7,
                     clvCOMPONENT_7, clvCOMPONENT_7, clvCOMPONENT_7, clvCOMPONENT_7,
                     clvCOMPONENT_7, clvCOMPONENT_7, clvCOMPONENT_7, clvCOMPONENT_7,
                     clvCOMPONENT_7, clvCOMPONENT_7, clvCOMPONENT_7, clvCOMPONENT_7}};

clsCOMPONENT_SELECTION    ComponentSelection_4_DBL =
        {2, {clvCOMPONENT_8, clvCOMPONENT_9, clvCOMPONENT_9, clvCOMPONENT_9,
                     clvCOMPONENT_9, clvCOMPONENT_9, clvCOMPONENT_9, clvCOMPONENT_9,
                     clvCOMPONENT_9, clvCOMPONENT_9, clvCOMPONENT_9, clvCOMPONENT_9,
                     clvCOMPONENT_9, clvCOMPONENT_9, clvCOMPONENT_9, clvCOMPONENT_9,
             clvCOMPONENT_9, clvCOMPONENT_9, clvCOMPONENT_9, clvCOMPONENT_9,
                     clvCOMPONENT_9, clvCOMPONENT_9, clvCOMPONENT_9, clvCOMPONENT_9,
                     clvCOMPONENT_9, clvCOMPONENT_9, clvCOMPONENT_9, clvCOMPONENT_9,
                     clvCOMPONENT_9, clvCOMPONENT_9, clvCOMPONENT_9, clvCOMPONENT_9}};

clsCOMPONENT_SELECTION    ComponentSelection_5_DBL =
        {2, {clvCOMPONENT_10, clvCOMPONENT_11, clvCOMPONENT_11, clvCOMPONENT_11,
                     clvCOMPONENT_11, clvCOMPONENT_11, clvCOMPONENT_11, clvCOMPONENT_11,
                     clvCOMPONENT_11, clvCOMPONENT_11, clvCOMPONENT_11, clvCOMPONENT_11,
                     clvCOMPONENT_11, clvCOMPONENT_11, clvCOMPONENT_11, clvCOMPONENT_11,
             clvCOMPONENT_11, clvCOMPONENT_11, clvCOMPONENT_11, clvCOMPONENT_11,
                     clvCOMPONENT_11, clvCOMPONENT_11, clvCOMPONENT_11, clvCOMPONENT_11,
                     clvCOMPONENT_11, clvCOMPONENT_11, clvCOMPONENT_11, clvCOMPONENT_11,
                     clvCOMPONENT_11, clvCOMPONENT_11, clvCOMPONENT_11, clvCOMPONENT_11}};

clsCOMPONENT_SELECTION    ComponentSelection_6_DBL =
        {2, {clvCOMPONENT_12, clvCOMPONENT_13, clvCOMPONENT_13, clvCOMPONENT_13,
                     clvCOMPONENT_13, clvCOMPONENT_13, clvCOMPONENT_13, clvCOMPONENT_13,
                     clvCOMPONENT_13, clvCOMPONENT_13, clvCOMPONENT_13, clvCOMPONENT_13,
                     clvCOMPONENT_13, clvCOMPONENT_13, clvCOMPONENT_13, clvCOMPONENT_13,
             clvCOMPONENT_13, clvCOMPONENT_13, clvCOMPONENT_13, clvCOMPONENT_13,
                     clvCOMPONENT_13, clvCOMPONENT_13, clvCOMPONENT_13, clvCOMPONENT_13,
                     clvCOMPONENT_13, clvCOMPONENT_13, clvCOMPONENT_13, clvCOMPONENT_13,
                     clvCOMPONENT_13, clvCOMPONENT_13, clvCOMPONENT_13, clvCOMPONENT_13}};

clsCOMPONENT_SELECTION    ComponentSelection_7_DBL =
        {2, {clvCOMPONENT_14, clvCOMPONENT_15, clvCOMPONENT_15, clvCOMPONENT_15,
                     clvCOMPONENT_15, clvCOMPONENT_15, clvCOMPONENT_15, clvCOMPONENT_15,
                     clvCOMPONENT_15, clvCOMPONENT_15, clvCOMPONENT_15, clvCOMPONENT_15,
                     clvCOMPONENT_15, clvCOMPONENT_15, clvCOMPONENT_15, clvCOMPONENT_15,
             clvCOMPONENT_15, clvCOMPONENT_15, clvCOMPONENT_15, clvCOMPONENT_15,
                     clvCOMPONENT_15, clvCOMPONENT_15, clvCOMPONENT_15, clvCOMPONENT_15,
                     clvCOMPONENT_15, clvCOMPONENT_15, clvCOMPONENT_15, clvCOMPONENT_15,
                     clvCOMPONENT_15, clvCOMPONENT_15, clvCOMPONENT_15, clvCOMPONENT_15}};

clsCOMPONENT_SELECTION    ComponentSelection_8_DBL =
        {2, {clvCOMPONENT_16, clvCOMPONENT_17, clvCOMPONENT_17, clvCOMPONENT_17,
                     clvCOMPONENT_17, clvCOMPONENT_17, clvCOMPONENT_17, clvCOMPONENT_17,
                     clvCOMPONENT_17, clvCOMPONENT_17, clvCOMPONENT_17, clvCOMPONENT_17,
                     clvCOMPONENT_17, clvCOMPONENT_17, clvCOMPONENT_17, clvCOMPONENT_17,
             clvCOMPONENT_17, clvCOMPONENT_17, clvCOMPONENT_17, clvCOMPONENT_17,
                     clvCOMPONENT_17, clvCOMPONENT_17, clvCOMPONENT_17, clvCOMPONENT_17,
                     clvCOMPONENT_17, clvCOMPONENT_17, clvCOMPONENT_17, clvCOMPONENT_17,
                     clvCOMPONENT_17, clvCOMPONENT_17, clvCOMPONENT_17, clvCOMPONENT_17}};

clsCOMPONENT_SELECTION    ComponentSelection_9_DBL =
        {2, {clvCOMPONENT_18, clvCOMPONENT_19, clvCOMPONENT_19, clvCOMPONENT_19,
                     clvCOMPONENT_19, clvCOMPONENT_19, clvCOMPONENT_19, clvCOMPONENT_19,
                     clvCOMPONENT_19, clvCOMPONENT_19, clvCOMPONENT_19, clvCOMPONENT_19,
                     clvCOMPONENT_19, clvCOMPONENT_19, clvCOMPONENT_19, clvCOMPONENT_19,
             clvCOMPONENT_19, clvCOMPONENT_19, clvCOMPONENT_19, clvCOMPONENT_19,
                     clvCOMPONENT_19, clvCOMPONENT_19, clvCOMPONENT_19, clvCOMPONENT_19,
                     clvCOMPONENT_19, clvCOMPONENT_19, clvCOMPONENT_19, clvCOMPONENT_19,
                     clvCOMPONENT_19, clvCOMPONENT_19, clvCOMPONENT_19, clvCOMPONENT_19}};

clsCOMPONENT_SELECTION    ComponentSelection_10_DBL =
        {2, {clvCOMPONENT_20, clvCOMPONENT_21, clvCOMPONENT_21, clvCOMPONENT_21,
                     clvCOMPONENT_21, clvCOMPONENT_21, clvCOMPONENT_21, clvCOMPONENT_21,
                     clvCOMPONENT_21, clvCOMPONENT_21, clvCOMPONENT_21, clvCOMPONENT_21,
                     clvCOMPONENT_21, clvCOMPONENT_21, clvCOMPONENT_21, clvCOMPONENT_21,
             clvCOMPONENT_21, clvCOMPONENT_21, clvCOMPONENT_21, clvCOMPONENT_21,
                     clvCOMPONENT_21, clvCOMPONENT_21, clvCOMPONENT_21, clvCOMPONENT_21,
                     clvCOMPONENT_21, clvCOMPONENT_21, clvCOMPONENT_21, clvCOMPONENT_21,
                     clvCOMPONENT_21, clvCOMPONENT_21, clvCOMPONENT_21, clvCOMPONENT_21}};

clsCOMPONENT_SELECTION    ComponentSelection_11_DBL =
        {2, {clvCOMPONENT_22, clvCOMPONENT_23, clvCOMPONENT_23, clvCOMPONENT_23,
                     clvCOMPONENT_23, clvCOMPONENT_23, clvCOMPONENT_23, clvCOMPONENT_23,
                     clvCOMPONENT_23, clvCOMPONENT_23, clvCOMPONENT_23, clvCOMPONENT_23,
                     clvCOMPONENT_23, clvCOMPONENT_23, clvCOMPONENT_23, clvCOMPONENT_23,
             clvCOMPONENT_23, clvCOMPONENT_23, clvCOMPONENT_23, clvCOMPONENT_23,
                     clvCOMPONENT_23, clvCOMPONENT_23, clvCOMPONENT_23, clvCOMPONENT_23,
                     clvCOMPONENT_23, clvCOMPONENT_23, clvCOMPONENT_23, clvCOMPONENT_23,
                     clvCOMPONENT_23, clvCOMPONENT_23, clvCOMPONENT_23, clvCOMPONENT_23}};

clsCOMPONENT_SELECTION    ComponentSelection_12_DBL =
        {2, {clvCOMPONENT_24, clvCOMPONENT_25, clvCOMPONENT_25, clvCOMPONENT_25,
                     clvCOMPONENT_25, clvCOMPONENT_25, clvCOMPONENT_25, clvCOMPONENT_25,
                     clvCOMPONENT_25, clvCOMPONENT_25, clvCOMPONENT_25, clvCOMPONENT_25,
                     clvCOMPONENT_25, clvCOMPONENT_25, clvCOMPONENT_25, clvCOMPONENT_25,
             clvCOMPONENT_25, clvCOMPONENT_25, clvCOMPONENT_25, clvCOMPONENT_25,
                     clvCOMPONENT_25, clvCOMPONENT_25, clvCOMPONENT_25, clvCOMPONENT_25,
                     clvCOMPONENT_25, clvCOMPONENT_25, clvCOMPONENT_25, clvCOMPONENT_25,
                     clvCOMPONENT_25, clvCOMPONENT_25, clvCOMPONENT_25, clvCOMPONENT_25}};

clsCOMPONENT_SELECTION    ComponentSelection_13_DBL =
        {2, {clvCOMPONENT_26, clvCOMPONENT_27, clvCOMPONENT_27, clvCOMPONENT_27,
                     clvCOMPONENT_27, clvCOMPONENT_27, clvCOMPONENT_27, clvCOMPONENT_27,
                     clvCOMPONENT_27, clvCOMPONENT_27, clvCOMPONENT_27, clvCOMPONENT_27,
                     clvCOMPONENT_27, clvCOMPONENT_27, clvCOMPONENT_27, clvCOMPONENT_27,
             clvCOMPONENT_27, clvCOMPONENT_27, clvCOMPONENT_27, clvCOMPONENT_27,
                     clvCOMPONENT_27, clvCOMPONENT_27, clvCOMPONENT_27, clvCOMPONENT_27,
                     clvCOMPONENT_27, clvCOMPONENT_27, clvCOMPONENT_27, clvCOMPONENT_27,
                     clvCOMPONENT_27, clvCOMPONENT_27, clvCOMPONENT_27, clvCOMPONENT_27}};

clsCOMPONENT_SELECTION    ComponentSelection_14_DBL =
        {2, {clvCOMPONENT_28, clvCOMPONENT_29, clvCOMPONENT_29, clvCOMPONENT_29,
                     clvCOMPONENT_29, clvCOMPONENT_29, clvCOMPONENT_29, clvCOMPONENT_29,
                     clvCOMPONENT_29, clvCOMPONENT_29, clvCOMPONENT_29, clvCOMPONENT_29,
                     clvCOMPONENT_29, clvCOMPONENT_29, clvCOMPONENT_29, clvCOMPONENT_29,
             clvCOMPONENT_29, clvCOMPONENT_29, clvCOMPONENT_29, clvCOMPONENT_29,
                     clvCOMPONENT_29, clvCOMPONENT_29, clvCOMPONENT_29, clvCOMPONENT_29,
                     clvCOMPONENT_29, clvCOMPONENT_29, clvCOMPONENT_29, clvCOMPONENT_29,
                     clvCOMPONENT_29, clvCOMPONENT_29, clvCOMPONENT_29, clvCOMPONENT_29}};

clsCOMPONENT_SELECTION    ComponentSelection_15_DBL =
        {2, {clvCOMPONENT_30, clvCOMPONENT_31, clvCOMPONENT_31, clvCOMPONENT_31,
                     clvCOMPONENT_31, clvCOMPONENT_31, clvCOMPONENT_31, clvCOMPONENT_31,
                     clvCOMPONENT_31, clvCOMPONENT_31, clvCOMPONENT_31, clvCOMPONENT_31,
                     clvCOMPONENT_31, clvCOMPONENT_31, clvCOMPONENT_31, clvCOMPONENT_31,
             clvCOMPONENT_31, clvCOMPONENT_31, clvCOMPONENT_31, clvCOMPONENT_31,
                     clvCOMPONENT_31, clvCOMPONENT_31, clvCOMPONENT_31, clvCOMPONENT_31,
                     clvCOMPONENT_31, clvCOMPONENT_31, clvCOMPONENT_31, clvCOMPONENT_31,
                     clvCOMPONENT_31, clvCOMPONENT_31, clvCOMPONENT_31, clvCOMPONENT_31}};


clsCOMPONENT_SELECTION    ComponentSelection_VECTOR8 =
        {8, {0, 1, 2, 3, 4, 5, 6, 7, 7, 7, 7, 7, 7, 7, 7, 7,
             7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7}};

clsCOMPONENT_SELECTION    ComponentSelection_VECTOR16 =
        {16, {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
              15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15}};

clsCOMPONENT_SELECTION    ComponentSelection_VECTOR8_DBL =
        {16, {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
             15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15}};

clsCOMPONENT_SELECTION    ComponentSelection_VECTOR16_DBL =
        {32, {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
              16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31}};

gctBOOL
clIsRepeatedComponentSelection(
    IN clsCOMPONENT_SELECTION * ComponentSelection
    )
{
  gctUINT8 i, j;

  gcmASSERT(ComponentSelection);

  for (i = 0; i < ComponentSelection->components - 1; i++) {
    for (j = i + 1; j < ComponentSelection->components; j++) {
      if (ComponentSelection->selection[i] == ComponentSelection->selection[j]) return gcvTRUE;
    }
  }

  return gcvFALSE;
}

gctCONST_STRING
clGetIRUnaryExprTypeName(
IN cleUNARY_EXPR_TYPE UnaryExprType
)
{
    switch (UnaryExprType) {
    case clvUNARY_FIELD_SELECTION:        return ".";
    case clvUNARY_COMPONENT_SELECTION:    return ".";

    case clvUNARY_POST_INC:            return "x++";
    case clvUNARY_POST_DEC:            return "x--";
    case clvUNARY_PRE_INC:            return "++x";
    case clvUNARY_PRE_DEC:            return "--x";
    case clvUNARY_NEG:            return "-";
    case clvUNARY_NOT:            return "!";
    case clvUNARY_NOT_BITWISE:        return "~";
    case clvUNARY_INDIRECTION:        return "*";
    case clvUNARY_ADDR:            return "&";
    case clvUNARY_CAST:            return "type_cast";
    case clvUNARY_NON_LVAL:            return "non_lval";
    case clvUNARY_NULL:            return "null";

    default:
        gcmASSERT(0);
        return "invalid";
    }
}

gceSTATUS
cloIR_UNARY_EXPR_Dump(
    IN cloCOMPILER Compiler,
    IN cloIR_BASE This
    )
{
    cloIR_UNARY_EXPR    unaryExpr = (cloIR_UNARY_EXPR)This;
    gctUINT8            i, component;
    const gctCHAR        componentNames[4] = {'x', 'y', 'z', 'w'};

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(unaryExpr, clvIR_UNARY_EXPR);

    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                      clvDUMP_IR,
                      "<IR_UNARY_EXPR line=\"%d\" string=\"%d\""
                      " dataType=\"0x%x\" type=\"%s\">",
                      unaryExpr->exprBase.base.lineNo,
                      unaryExpr->exprBase.base.stringNo,
                      unaryExpr->exprBase.decl.dataType,
                      clGetIRUnaryExprTypeName(unaryExpr->type)));

    gcmASSERT(unaryExpr->operand);

    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                      clvDUMP_IR,
                      "<!-- Operand -->"));

    gcmVERIFY_OK(cloIR_OBJECT_Dump(Compiler, &unaryExpr->operand->base));

    switch (unaryExpr->type) {
    case clvUNARY_FIELD_SELECTION:
        gcmASSERT(unaryExpr->u.fieldName);

        gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                          clvDUMP_IR,
                          "<!-- Field -->"));

        gcmVERIFY_OK(clsNAME_Dump(Compiler, unaryExpr->u.fieldName));
        break;

    case clvUNARY_COMPONENT_SELECTION:
        gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                          clvDUMP_IR,
                          "<COMPONMENT_SELECTION value=\""));

            gcmASSERT(unaryExpr->u.componentSelection.components <= cldMaxComponentCount);
        for (i = 0; i < unaryExpr->u.componentSelection.components; i++) {
           component = unaryExpr->u.componentSelection.selection[i];
           gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                             clvDUMP_IR,
                             "%c",
                             componentNames[component]));
        }

        gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                          clvDUMP_IR,
                          "\" />"));
        break;

    default:
        break;
    }

    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                      clvDUMP_IR,
                      "</IR_UNARY_EXPR>"));
    return gcvSTATUS_OK;
}

gceSTATUS
cloIR_UNARY_EXPR_Accept(
    IN cloCOMPILER Compiler,
    IN cloIR_BASE This,
    IN clsVISITOR * Visitor,
    IN OUT gctPOINTER Parameters
    )
{
    cloIR_UNARY_EXPR    unaryExpr = (cloIR_UNARY_EXPR)This;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(unaryExpr, clvIR_UNARY_EXPR);
    gcmASSERT(Visitor);

    if (Visitor->visitUnaryExpr == gcvNULL) return gcvSTATUS_OK;

    return Visitor->visitUnaryExpr(Compiler, Visitor, unaryExpr, Parameters);
}

static clsVTAB s_unaryExprVTab =
{
    clvIR_UNARY_EXPR,
    cloIR_UNARY_EXPR_Destroy,
    cloIR_UNARY_EXPR_Dump,
    cloIR_UNARY_EXPR_Accept
};

#define _GetFloatSign(f)      (((f) >> 31) & 0x01)
#define _GetFloatExp(f)       (((f) >> 23) & 0xFF)
#define _GetFloatMantissa(f)  (((f) & 0x7FFFFF) | 0x800000)

gctUINT
clConvFloatToHalf(
IN gctFLOAT F
)
{
    union {
       gctFLOAT f;
       gctUINT  u;
    } value;

    value.f = F;
    return ((_GetFloatSign(value.u) << 15 |
            (_GetFloatExp(value.u) & 0x1F) << 10) |
            ((_GetFloatMantissa(value.u) >> 13) & 0x3FF));
}

static gctINT
_ConvUnsignedToSigned(
IN gctINT intType
)
{
   switch(intType) {
   case T_UINT:
   case T_INT:
   case T_FLOAT:
   case T_DOUBLE:
   case T_HALF:
   case T_BOOL:
   case T_HALF_PACKED:
   case T_BOOL_PACKED:
   case T_ENUM:
   case T_EVENT_T:
      return T_INT;

   case T_UINT2:
   case T_INT2:
   case T_FLOAT2:
   case T_DOUBLE2:
   case T_HALF2:
   case T_BOOL2:
      return T_INT2;

   case T_UINT3:
   case T_INT3:
   case T_FLOAT3:
   case T_DOUBLE3:
   case T_HALF3:
   case T_BOOL3:
      return T_INT3;

   case T_UINT4:
   case T_INT4:
   case T_FLOAT4:
   case T_DOUBLE4:
   case T_HALF4:
   case T_BOOL4:
      return T_INT4;

   case T_UINT8:
   case T_INT8:
   case T_FLOAT8:
   case T_DOUBLE8:
   case T_HALF8:
   case T_BOOL8:
      return T_INT8;

   case T_UINT16:
   case T_INT16:
   case T_FLOAT16:
   case T_DOUBLE16:
   case T_HALF16:
   case T_BOOL16:
      return T_INT16;

   case T_HALF2_PACKED:
      return T_SHORT2_PACKED;

   case T_BOOL2_PACKED:
      return T_CHAR2_PACKED;

   case T_HALF3_PACKED:
      return T_SHORT3_PACKED;

   case T_BOOL3_PACKED:
      return T_CHAR3_PACKED;

   case T_HALF4_PACKED:
      return T_SHORT4_PACKED;

   case T_BOOL4_PACKED:
      return T_CHAR4_PACKED;

   case T_HALF8_PACKED:
      return T_SHORT8_PACKED;

   case T_BOOL8_PACKED:
      return T_CHAR8_PACKED;

   case T_HALF16_PACKED:
      return T_SHORT16_PACKED;

   case T_BOOL16_PACKED:
      return T_CHAR16_PACKED;

   case T_USHORT:
   case T_SHORT:
   case T_USHORT_PACKED:
   case T_SHORT_PACKED:
      return T_SHORT;

   case T_USHORT2:
   case T_SHORT2:
      return T_SHORT2;

   case T_USHORT3:
   case T_SHORT3:
      return T_SHORT3;

   case T_USHORT4:
   case T_SHORT4:
      return T_SHORT4;

   case T_USHORT8:
   case T_SHORT8:
      return T_SHORT8;

   case T_USHORT16:
   case T_SHORT16:
      return T_SHORT16;

   case T_USHORT2_PACKED:
   case T_SHORT2_PACKED:
      return T_SHORT2_PACKED;

   case T_USHORT3_PACKED:
   case T_SHORT3_PACKED:
      return T_SHORT3_PACKED;

   case T_USHORT4_PACKED:
   case T_SHORT4_PACKED:
      return T_SHORT4_PACKED;

   case T_USHORT8_PACKED:
   case T_SHORT8_PACKED:
      return T_SHORT8_PACKED;

   case T_USHORT16_PACKED:
   case T_SHORT16_PACKED:
      return T_SHORT16_PACKED;

   case T_UCHAR:
   case T_CHAR:
   case T_UCHAR_PACKED:
   case T_CHAR_PACKED:
      return T_CHAR;

   case T_UCHAR2:
   case T_CHAR2:
      return T_CHAR2;

   case T_UCHAR2_PACKED:
   case T_CHAR2_PACKED:
      return T_CHAR2_PACKED;

   case T_UCHAR3:
   case T_CHAR3:
      return T_CHAR3;

   case T_UCHAR3_PACKED:
   case T_CHAR3_PACKED:
      return T_CHAR3_PACKED;

   case T_UCHAR4:
   case T_CHAR4:
      return T_CHAR4;

   case T_UCHAR4_PACKED:
   case T_CHAR4_PACKED:
      return T_CHAR4_PACKED;

   case T_UCHAR8:
   case T_CHAR8:
      return T_CHAR8;

   case T_UCHAR8_PACKED:
   case T_CHAR8_PACKED:
      return T_CHAR8_PACKED;

   case T_UCHAR16:
   case T_CHAR16:
      return T_CHAR16;

   case T_UCHAR16_PACKED:
   case T_CHAR16_PACKED:
      return T_CHAR16_PACKED;

   case T_ULONG:
   case T_LONG:
      return T_LONG;

   case T_ULONG2:
   case T_LONG2:
      return T_LONG2;

   case T_ULONG3:
   case T_LONG3:
      return T_LONG3;

   case T_ULONG4:
   case T_LONG4:
      return T_LONG4;

   case T_ULONG8:
   case T_LONG8:
      return T_LONG8;

   case T_ULONG16:
   case T_LONG16:
      return T_LONG16;

   default:
      gcmASSERT(0);
      return T_INT;
   }
}

static gceSTATUS
_GetUnaryExprDecl(
IN cloCOMPILER Compiler,
IN cleUNARY_EXPR_TYPE Type,
IN cloIR_EXPR Operand,
IN clsNAME * FieldName,
IN clsCOMPONENT_SELECTION * ComponentSelection,
OUT clsDECL * Decl,
OUT cloIR_UNARY_EXPR *ModifierExpr
)
{
    gceSTATUS status;
    gctINT resultType;
    clsDECL decl;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    gcmASSERT(Operand);
    gcmASSERT(Decl);

    switch (Type) {
    case clvUNARY_FIELD_SELECTION:
        gcmASSERT(FieldName);

        if (FieldName->decl.dataType == gcvNULL) return gcvSTATUS_INVALID_ARGUMENT;

        status = cloCOMPILER_CloneDecl(Compiler,
                                       Operand->decl.dataType->accessQualifier == clvQUALIFIER_CONST
                                                                               ? clvQUALIFIER_NONE
                                                                               : Operand->decl.dataType->accessQualifier,
                                       Operand->decl.dataType->addrSpaceQualifier,
                                       &FieldName->decl,
                                       Decl);
        if (gcmIS_ERROR(status)) return status;

        if (Decl->dataType->elementType == clvTYPE_HALF &&
            !clmDECL_IsPointerType(Decl) && !clmDECL_IsArray(Decl)) {
            gctBOOL addNullExpr = gcvFALSE;
            clsNAME *variable;

            if(cloIR_OBJECT_GetType(&Operand->base) == clvIR_BINARY_EXPR) {
                cloIR_BINARY_EXPR binaryExpr = (cloIR_BINARY_EXPR) &Operand->base;
                if(binaryExpr->type == clvBINARY_SUBSCRIPT &&
                   clmDECL_IsPointerType(&binaryExpr->leftOperand->decl)) {
                    addNullExpr = gcvTRUE;
                }
                else {
                    variable = clParseFindLeafName(Compiler, Operand);
                    if(variable && clmGEN_CODE_checkVariableForMemory(variable)) {
                        addNullExpr = gcvTRUE;
                    }
                }
            }
            else {
                variable = clParseFindLeafName(Compiler, Operand);
                if(variable && clmGEN_CODE_checkVariableForMemory(variable)) {
                    addNullExpr = gcvTRUE;
                }
            }
            if(addNullExpr) {
                gctINT resultType;
                resultType = clGetVectorTerminalToken(clvTYPE_FLOAT,
                                                      clmDATA_TYPE_vectorSize_GET(Decl->dataType));
                if(resultType) {
                    clsDECL decl[1];
                    status = cloCOMPILER_CreateDecl(Compiler,
                                                    resultType,
                                                    Decl->dataType->u.generic,
                                                    Decl->dataType->accessQualifier,
                                                    Decl->dataType->addrSpaceQualifier,
                                                    decl);
                    if (gcmIS_ERROR(status)) return status;
                    status = cloIR_NULL_EXPR_Construct(Compiler,
                                                       Operand->base.lineNo,
                                                       Operand->base.stringNo,
                                                       decl,
                                                       ModifierExpr);
                    if (gcmIS_ERROR(status)) return status;
                }
            }
        }
        break;

    case clvUNARY_COMPONENT_SELECTION:
        gcmASSERT(ComponentSelection);
        gcmASSERT(clmDECL_IsBVecOrIVecOrVec(&Operand->decl));

        resultType = clGetVectorTerminalToken(Operand->decl.dataType->elementType, ComponentSelection->components);
        if(resultType) {
            status = cloCOMPILER_CreateDecl(Compiler,
                                            resultType,
                                            Operand->decl.dataType->u.generic,
                                            Operand->decl.dataType->accessQualifier,
                                            Operand->decl.dataType->addrSpaceQualifier,
                                            Decl);
            if (gcmIS_ERROR(status)) return status;
        }
        else {
            /* call new function to create  internal vector data type */
        }
        break;

    case clvUNARY_POST_INC:
    case clvUNARY_POST_DEC:
    case clvUNARY_PRE_INC:
    case clvUNARY_PRE_DEC:
        status = cloCOMPILER_CloneDecl(Compiler,
                                       Operand->decl.dataType->accessQualifier,
                                       Operand->decl.dataType->addrSpaceQualifier,
                                       &Operand->decl,
                                       Decl);
        if (gcmIS_ERROR(status)) return status;
        break;

    case clvUNARY_NEG:
    case clvUNARY_NON_LVAL:
    case clvUNARY_NOT_BITWISE:
        status = cloCOMPILER_CloneDecl(Compiler,
                                       clvQUALIFIER_CONST,
                                       Operand->decl.dataType->addrSpaceQualifier,
                                       &Operand->decl,
                                       Decl);
        if (gcmIS_ERROR(status)) return status;
        break;

    case clvUNARY_ADDR:
        status = cloCOMPILER_CloneDecl(Compiler,
                                       clvQUALIFIER_CONST,
                                       Operand->decl.dataType->addrSpaceQualifier,
                                       &Operand->decl,
                                       Decl);
        if (gcmIS_ERROR(status)) return status;
        if(cloIR_OBJECT_GetType(&Operand->base) == clvIR_BINARY_EXPR) {
            cloIR_BINARY_EXPR binaryExpr = (cloIR_BINARY_EXPR) &Operand->base;
            if(binaryExpr->type == clvBINARY_SUBSCRIPT &&
               clmDECL_IsPointerType(&binaryExpr->leftOperand->decl)) {
               /* expression is of the form P[i] where P is of pointer type and
                  therefore P need not be set as addressed */
                ;
            }
            else {
                clParseSetOperandAddressed(Compiler, Operand);
            }
        }
        else clParseSetOperandAddressed(Compiler, Operand);

        if(clmDECL_IsArray(Decl)) {
           Decl->ptrDominant = gcvTRUE;
        }
        status = clParseAddIndirectionOneLevel(Compiler, &Decl->ptrDscr);
        if (gcmIS_ERROR(status)) return status;
        break;

    case clvUNARY_NOT:
        decl = Operand->decl;
        resultType = _ConvUnsignedToSigned(Operand->decl.dataType->type);
        if(resultType != Operand->decl.dataType->type) {
            status = cloCOMPILER_CreateDataType(Compiler,
                                                resultType,
                                                gcvNULL,
                                                clvQUALIFIER_CONST,
                                                clvQUALIFIER_NONE,
                                                &decl.dataType);
            if (gcmIS_ERROR(status)) return status;
        }

        status = cloCOMPILER_CloneDecl(Compiler,
                                       clvQUALIFIER_CONST,
                                       Operand->decl.dataType->addrSpaceQualifier,
                                       &decl,
                                       Decl);
        if (gcmIS_ERROR(status)) return status;
        break;

    case clvUNARY_INDIRECTION:
        status = cloCOMPILER_CreateElementDecl(Compiler,
                                               &Operand->decl,
                                               Decl);
        if (gcmIS_ERROR(status)) return status;
        if (Decl->dataType->elementType == clvTYPE_HALF &&
            !clmDECL_IsPointerType(Decl) && !clmDECL_IsArray(Decl)) {
            gctINT resultType;

            resultType = clGetVectorTerminalToken(clvTYPE_FLOAT,
                                                  clmDATA_TYPE_vectorSize_GET(Decl->dataType));

            if(resultType) {
                clsDECL decl[1];

                status = cloCOMPILER_CreateDecl(Compiler,
                                                resultType,
                                                Decl->dataType->u.generic,
                                                Decl->dataType->accessQualifier,
                                                Decl->dataType->addrSpaceQualifier,
                                                decl);
                if (gcmIS_ERROR(status)) return status;
                status = cloIR_NULL_EXPR_Construct(Compiler,
                                                   Operand->base.lineNo,
                                                   Operand->base.stringNo,
                                                   decl,
                                                   ModifierExpr);
                if (gcmIS_ERROR(status)) return status;
            }
        }
        break;

    default: gcmASSERT(0);
    }
    return gcvSTATUS_OK;
}

gceSTATUS
cloIR_CAST_EXPR_Construct(
IN cloCOMPILER Compiler,
IN gctUINT LineNo,
IN gctUINT StringNo,
IN clsDECL *TypeCast,
IN cloIR_EXPR Operand,
OUT cloIR_EXPR *Expr
)
{
  gceSTATUS  status = gcvSTATUS_OK;
  cloIR_UNARY_EXPR unaryExpr = gcvNULL;

/* Verify the arguments. */
  clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
  gcmASSERT(Operand);
  gcmASSERT(Expr);

  if (clsDECL_IsEqual(TypeCast, &Operand->decl)) {
    *Expr = Operand;
    return gcvSTATUS_OK;
  }
  do {
    clsDECL effectiveDecl;
    gctPOINTER pointer;

    status = cloCOMPILER_Allocate(Compiler,
                                  (gctSIZE_T)sizeof(struct _cloIR_UNARY_EXPR),
                                  (gctPOINTER *) &pointer);
    if (gcmIS_ERROR(status)) break;
    unaryExpr = pointer;

    effectiveDecl = *TypeCast;
    status = cloCOMPILER_CloneDataType(Compiler,
                                       TypeCast->dataType->accessQualifier,
                                       Operand->decl.dataType->addrSpaceQualifier,
                                       TypeCast->dataType,
                                       &effectiveDecl.dataType);
    if (gcmIS_ERROR(status)) break;

    cloIR_EXPR_Initialize(&unaryExpr->exprBase, &s_unaryExprVTab, LineNo, StringNo, effectiveDecl);
    unaryExpr->type = clvUNARY_CAST;
    unaryExpr->operand = Operand;

    *Expr = &unaryExpr->exprBase;
    return gcvSTATUS_OK;
  } while (gcvFALSE);

  if(unaryExpr != gcvNULL)  cloCOMPILER_Free(Compiler,unaryExpr);
  *Expr = gcvNULL;
  return status;
}

gceSTATUS
cloIR_UNARY_EXPR_Construct(
IN cloCOMPILER Compiler,
IN gctUINT LineNo,
IN gctUINT StringNo,
IN cleUNARY_EXPR_TYPE Type,
IN cloIR_EXPR Operand,
IN clsNAME *FieldName,
IN clsCOMPONENT_SELECTION *ComponentSelection,
OUT cloIR_EXPR *ResExpr
)
{
    gceSTATUS  status = gcvSTATUS_OK;
    cloIR_UNARY_EXPR unaryExpr;
    clsDECL decl;
    cloIR_UNARY_EXPR modifierExpr = gcvNULL;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    gcmASSERT(Operand);
    gcmASSERT(ResExpr);

    do {
        gctPOINTER pointer;

        status = _GetUnaryExprDecl(Compiler,
                                   Type,
                                   Operand,
                                   FieldName,
                                   ComponentSelection,
                                   &decl,
                                   &modifierExpr);
        if (gcmIS_ERROR(status)) break;
        gcmASSERT(decl.dataType);

        status = cloCOMPILER_Allocate(Compiler,
                          (gctSIZE_T)sizeof(struct _cloIR_UNARY_EXPR),
                          (gctPOINTER *) &pointer);
        if (gcmIS_ERROR(status)) break;
        unaryExpr = pointer;

        cloIR_EXPR_Initialize(&unaryExpr->exprBase, &s_unaryExprVTab, LineNo, StringNo,
                      decl);

        unaryExpr->type    = Type;
        unaryExpr->operand = Operand;

        switch (Type) {
        case clvUNARY_FIELD_SELECTION:
            gcmASSERT(FieldName);
            unaryExpr->u.fieldName    = FieldName;
            break;

        case clvUNARY_COMPONENT_SELECTION:
            gcmASSERT(ComponentSelection);
            unaryExpr->u.componentSelection    = *ComponentSelection;
            break;

        default:
            unaryExpr->u.generated = gcvNULL;
            break;
        }

        if(modifierExpr) {
            modifierExpr->operand = &unaryExpr->exprBase;
            *ResExpr = &modifierExpr->exprBase;
        }
        else {
            *ResExpr = &unaryExpr->exprBase;
        }

        return gcvSTATUS_OK;
    } while (gcvFALSE);

    *ResExpr = gcvNULL;
    return status;
}

gceSTATUS
cloIR_NULL_EXPR_Construct(
IN cloCOMPILER Compiler,
IN gctUINT LineNo,
IN gctUINT StringNo,
IN clsDECL *Decl,
OUT cloIR_UNARY_EXPR *NullExpr
)
{
    gceSTATUS  status = gcvSTATUS_OK;
    cloIR_UNARY_EXPR nullExpr;
    clsDECL decl;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);

    do {
        gctPOINTER pointer;

        status = cloCOMPILER_CloneDecl(Compiler,
                           Decl->dataType->accessQualifier,
                           Decl->dataType->addrSpaceQualifier,
                           Decl,
                           &decl);
        if (gcmIS_ERROR(status)) return status;

        status = cloCOMPILER_Allocate(Compiler,
                          (gctSIZE_T)sizeof(struct _cloIR_UNARY_EXPR),
                          (gctPOINTER *) &pointer);
        if (gcmIS_ERROR(status)) break;
        nullExpr = pointer;

        cloIR_EXPR_Initialize(&nullExpr->exprBase, &s_unaryExprVTab, LineNo, StringNo,
                      decl);

        nullExpr->type    = clvUNARY_NULL;
        nullExpr->operand = gcvNULL;

        *NullExpr = nullExpr;

        return gcvSTATUS_OK;
    } while (gcvFALSE);

    *NullExpr = gcvNULL;
    return status;
}

static gceSTATUS
_NegConstantValue(
IN cltELEMENT_TYPE Type,
IN OUT cluCONSTANT_VALUE * Value
)
{
    gcmASSERT(Value);

    switch (Type) {
    case clvTYPE_INT:
        Value->intValue = -Value->intValue;
        break;

    case clvTYPE_FLOAT:
        Value->floatValue = -Value->floatValue;
        break;

    default:
        gcmASSERT(0);
        return gcvSTATUS_INVALID_ARGUMENT;
    }

    return gcvSTATUS_OK;
}

static gceSTATUS
_NotConstantValue(
IN cltELEMENT_TYPE Type,
IN OUT cluCONSTANT_VALUE * Value
)
{
    gcmASSERT(Value);

    switch (Type) {
    case clvTYPE_BOOL:
        Value->boolValue = !Value->boolValue;
        break;

    case clvTYPE_INT:
    case clvTYPE_CHAR:
    case clvTYPE_SHORT:
        Value->boolValue = !clmI2B(Value->intValue);
        break;

    case clvTYPE_UINT:
    case clvTYPE_UCHAR:
    case clvTYPE_USHORT:
        Value->boolValue = !clmU2B(Value->uintValue);
        break;

    case clvTYPE_FLOAT:
    case clvTYPE_DOUBLE:
    case clvTYPE_HALF:
        Value->boolValue = !clmF2B(Value->floatValue);
        break;

    default:
        gcmASSERT(0);
        return gcvSTATUS_INVALID_ARGUMENT;
    }

    return gcvSTATUS_OK;
}

gceSTATUS
cloIR_CAST_EXPR_Evaluate(
IN cloCOMPILER Compiler,
IN clsDECL *Decl,
IN OUT cloIR_CONSTANT Constant
)
{
   gceSTATUS status;

   if(Decl->dataType->elementType != Constant->exprBase.decl.dataType->elementType) {
      status = clParseConstantTypeConvert(Constant, Decl->dataType->elementType, Constant->values);
      if(gcmIS_ERROR(status)) return status;

      status = _cloIR_GetTargetCastDecl(Compiler,
                                        Decl,
                                        &Constant->exprBase.decl);
      if(gcmIS_ERROR(status)) return status;
   }
   return gcvSTATUS_OK;
}

static gceSTATUS
_BitwiseNotConstantValue(
IN cltELEMENT_TYPE Type,
IN OUT cluCONSTANT_VALUE * Value
)
{
    gcmASSERT(Value);

    switch (Type) {
    case clvTYPE_INT:
        Value->intValue = ~Value->intValue;
        break;

    default:
        gcmASSERT(0);
        return gcvSTATUS_INVALID_ARGUMENT;
    }

    return gcvSTATUS_OK;
}

gceSTATUS
cloIR_UNARY_EXPR_Evaluate(
IN cloCOMPILER Compiler,
IN cleUNARY_EXPR_TYPE Type,
IN cloIR_CONSTANT Constant,
IN clsNAME * FieldName,
IN clsCOMPONENT_SELECTION * ComponentSelection,
OUT cloIR_CONSTANT * ResultConstant
)
{
    gceSTATUS    status;
    cltEVALUATE_FUNC_PTR    evaluate;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(Constant, clvIR_CONSTANT);

    switch (Type) {
    case clvUNARY_FIELD_SELECTION:
        return _cloIR_CONSTANT_SelectField(Compiler,
                           Constant,
                           FieldName,
                           ResultConstant);

    case clvUNARY_COMPONENT_SELECTION:
        return _cloIR_CONSTANT_SelectComponents(Compiler,
                            Constant,
                            ComponentSelection,
                            ResultConstant);
    case clvUNARY_NEG:
        evaluate = &_NegConstantValue;
        break;

    case clvUNARY_NOT:
        evaluate = &_NotConstantValue;
        break;

    case clvUNARY_NOT_BITWISE:
        evaluate = &_BitwiseNotConstantValue;
        break;

    default:
        gcmASSERT(0);
        return gcvSTATUS_INVALID_ARGUMENT;
    }

    do {
        status = cloIR_CONSTANT_Evaluate(Compiler,
                         Constant,
                         evaluate);
        if (gcmIS_ERROR(status)) break;

        *ResultConstant = Constant;
        return gcvSTATUS_OK;
    } while (gcvFALSE);

    *ResultConstant = gcvNULL;
    return status;
}

/* cloIR_BINARY_EXPR object. */
gceSTATUS
cloIR_BINARY_EXPR_Destroy(
IN cloCOMPILER Compiler,
IN cloIR_BASE This
)
{
    cloIR_BINARY_EXPR  binaryExpr = (cloIR_BINARY_EXPR)This;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(binaryExpr, clvIR_BINARY_EXPR);

    gcmASSERT(binaryExpr->leftOperand);
    gcmVERIFY_OK(cloIR_OBJECT_Destroy(Compiler, &binaryExpr->leftOperand->base));

    gcmASSERT(binaryExpr->rightOperand);
    gcmVERIFY_OK(cloIR_OBJECT_Destroy(Compiler, &binaryExpr->rightOperand->base));

    if (binaryExpr->exprBase.asmMods != gcvNULL)
    {
        gcmVERIFY_OK(cloCOMPILER_Free(Compiler, binaryExpr->exprBase.asmMods));
    }

    gcmVERIFY_OK(cloCOMPILER_Free(Compiler, binaryExpr));
    return gcvSTATUS_OK;
}

static gctCONST_STRING
_GetIRBinaryExprTypeName(
IN cleBINARY_EXPR_TYPE BinaryExprType
)
{
    switch (BinaryExprType) {
    case clvBINARY_SUBSCRIPT:    return "subscript";

    case clvBINARY_ADD:        return "add";
    case clvBINARY_SUB:        return "sub";
    case clvBINARY_MUL:        return "mul";
    case clvBINARY_DIV:        return "div";
    case clvBINARY_MOD:        return "mod";

    case clvBINARY_GREATER_THAN:    return "greater_than";
    case clvBINARY_LESS_THAN:    return "less_than";
    case clvBINARY_GREATER_THAN_EQUAL: return "greater_than_equal";
    case clvBINARY_LESS_THAN_EQUAL:    return "less_than_equal";

    case clvBINARY_EQUAL:        return "equal";
    case clvBINARY_NOT_EQUAL:    return "not_equal";

    case clvBINARY_AND:        return "and";
    case clvBINARY_OR:        return "or";
    case clvBINARY_XOR:        return "xor";

    case clvBINARY_SEQUENCE:    return "sequence";

    case clvBINARY_ASSIGN:        return "assign";

    case clvBINARY_MUL_ASSIGN:    return "mul_assign";
    case clvBINARY_DIV_ASSIGN:    return "div_assign";
    case clvBINARY_ADD_ASSIGN:    return "add_assign";
    case clvBINARY_SUB_ASSIGN:    return "sub_assign";
    case clvBINARY_MOD_ASSIGN:    return "mod_assign";

    case clvBINARY_LEFT_ASSIGN:     return "<<=";
    case clvBINARY_RIGHT_ASSIGN:    return ">>=";
    case clvBINARY_AND_ASSIGN:    return "&=";
    case clvBINARY_XOR_ASSIGN:    return "^=";
    case clvBINARY_OR_ASSIGN:    return "|=";

    case clvBINARY_AND_BITWISE:     return "&";
    case clvBINARY_OR_BITWISE:    return "|";
    case clvBINARY_XOR_BITWISE:    return "^";

    case clvBINARY_LSHIFT:        return "<<";
    case clvBINARY_RSHIFT:        return ">>";

    case clvBINARY_MULTI_DIM_SUBSCRIPT: return "multi_dim_subscript";
    default:
        gcmASSERT(0);
        return "invalid";
    }
}

gceSTATUS
cloIR_BINARY_EXPR_Dump(
IN cloCOMPILER Compiler,
IN cloIR_BASE This
)
{
    cloIR_BINARY_EXPR    binaryExpr = (cloIR_BINARY_EXPR)This;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(binaryExpr, clvIR_BINARY_EXPR);

    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                      clvDUMP_IR,
                      "<IR_BINARY_EXPR line=\"%d\" string=\"%d\""
                      " dataType=\"0x%x\" type=\"%s\">",
                      binaryExpr->exprBase.base.lineNo,
                      binaryExpr->exprBase.base.stringNo,
                      binaryExpr->exprBase.decl.dataType,
                      _GetIRBinaryExprTypeName(binaryExpr->type)));
    gcmASSERT(binaryExpr->leftOperand);

    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                      clvDUMP_IR,
                      "<!-- Left Operand -->"));

    gcmVERIFY_OK(cloIR_OBJECT_Dump(Compiler, &binaryExpr->leftOperand->base));
    gcmASSERT(binaryExpr->rightOperand);

    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                      clvDUMP_IR,
                      "<!-- Right Operand -->"));

    gcmVERIFY_OK(cloIR_OBJECT_Dump(Compiler, &binaryExpr->rightOperand->base));
    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                      clvDUMP_IR,
                      "</IR_BINARY_EXPR>"));
    return gcvSTATUS_OK;
}

gceSTATUS
cloIR_BINARY_EXPR_Accept(
IN cloCOMPILER Compiler,
IN cloIR_BASE This,
IN clsVISITOR * Visitor,
IN OUT gctPOINTER Parameters
)
{
    cloIR_BINARY_EXPR    binaryExpr = (cloIR_BINARY_EXPR)This;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(binaryExpr, clvIR_BINARY_EXPR);
    gcmASSERT(Visitor);

    if (Visitor->visitBinaryExpr == gcvNULL) return gcvSTATUS_OK;

    return Visitor->visitBinaryExpr(Compiler, Visitor, binaryExpr, Parameters);
}

static clsVTAB s_binaryExprVTab =
{
    clvIR_BINARY_EXPR,
    cloIR_BINARY_EXPR_Destroy,
    cloIR_BINARY_EXPR_Dump,
    cloIR_BINARY_EXPR_Accept
};

gceSTATUS
cloIR_GetArithmeticExprDecl(
IN cloCOMPILER Compiler,
IN gctBOOL IsMul,
IN cloIR_EXPR LeftOperand,
IN cloIR_EXPR RightOperand,
OUT clsDECL *Decl
)
{
  gceSTATUS status;
  clsDECL *exprDecl;
  clsDECL *leftDecl;
  clsDECL *rightDecl;
  gctINT resultType = 0;
  cltQUALIFIER accessQualifier = clvQUALIFIER_CONST;

  /* Verify the arguments. */
  clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
  gcmASSERT(LeftOperand);
  gcmASSERT(RightOperand);
  gcmASSERT(RightOperand->decl.dataType);
  gcmASSERT(Decl);

  leftDecl = &LeftOperand->decl;
  rightDecl = &RightOperand->decl;
  gcmASSERT(leftDecl->dataType);
  gcmASSERT(rightDecl->dataType);

  exprDecl = &LeftOperand->decl;
  if(clmDECL_IsPointerType(&RightOperand->decl)) { /* right operand a pointer */
    exprDecl = &RightOperand->decl;
    if(clmDECL_IsPointerType(&LeftOperand->decl)) { /* both are pointers */
       resultType = T_INT;
    }
  }
  else if(!clmDECL_IsPointerType(&LeftOperand->decl)) { /* left operand not a pointer */
    if (!clsDECL_IsEqual(&LeftOperand->decl, &RightOperand->decl)) {
      if(clmDECL_IsScalar(leftDecl)) {
         if(clmDECL_IsVectorType(rightDecl)
            || clmDECL_IsMat(rightDecl)
            || (rightDecl->dataType->elementType > leftDecl->dataType->elementType)) {
            exprDecl = rightDecl;
         }
      }
      else if(clmDECL_IsVectorType(leftDecl)) {
         if (clmDECL_IsMat(rightDecl)) { /* vector and matrix */
             resultType = clGetVectorTerminalToken(rightDecl->dataType->elementType,
                                                   clmDATA_TYPE_matrixColumnCount_GET(rightDecl->dataType));
             gcmASSERT(resultType);
         }
      }
      else if(clmDECL_IsMat(leftDecl)) { /* Left operand must be a matrix */
         if(clmDECL_IsVectorType(rightDecl)) {
             resultType = clGetVectorTerminalToken(leftDecl->dataType->elementType,
                                                   clmDATA_TYPE_matrixRowCount_GET(leftDecl->dataType));
             gcmASSERT(resultType);
         }
         else if(clmDECL_IsMat(rightDecl)) { /* both are matrices */
             status = cloCOMPILER_CreateDecl(Compiler,
                                             T_FLOATNXM,
                                             gcvNULL,
                                             accessQualifier,
                                             exprDecl->dataType->addrSpaceQualifier,
                                             Decl);
             if (gcmIS_ERROR(status)) return status;
             clmDATA_TYPE_matrixSize_SET(Decl->dataType,
                                         clmDATA_TYPE_matrixRowCount_GET(leftDecl->dataType),
                                         clmDATA_TYPE_matrixColumnCount_GET(rightDecl->dataType));

             return gcvSTATUS_OK;
         }
      }
    }
  }

  if(resultType) {
    status = cloCOMPILER_CreateDecl(Compiler,
                                    resultType,
                                    gcvNULL,
                                    accessQualifier,
                                    exprDecl->dataType->addrSpaceQualifier,
                                    Decl);
  }
  else {
    status = cloCOMPILER_CloneDecl(Compiler,
                                   accessQualifier,
                                   exprDecl->dataType->addrSpaceQualifier,
                                   exprDecl,
                                   Decl);
  }
  if (gcmIS_ERROR(status)) return status;
  return gcvSTATUS_OK;
}

static gceSTATUS
_GetBitwiseLogicalExprDecl(
IN cloCOMPILER Compiler,
IN cloIR_EXPR LeftOperand,
IN cloIR_EXPR RightOperand,
OUT clsDECL *Decl
)
{
  gceSTATUS status;
  clsDECL *exprDecl;

  /* Verify the arguments. */
  clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
  gcmASSERT(LeftOperand);
  gcmASSERT(LeftOperand->decl.dataType);
  gcmASSERT(RightOperand);
  gcmASSERT(RightOperand->decl.dataType);
  gcmASSERT(Decl);

  gcmASSERT(clmDECL_IsIntOrIVec(&LeftOperand->decl) ||
            clmDECL_IsSampler(&LeftOperand->decl));
  gcmASSERT(clmDECL_IsIntOrIVec(&RightOperand->decl) ||
            clmDECL_IsSampler(&RightOperand->decl));

  if(clmDECL_IsIVec(&LeftOperand->decl) ||
     clmDECL_IsSampler(&LeftOperand->decl)) {
     exprDecl = &LeftOperand->decl;
  }
  else if(clmDECL_IsIVec(&RightOperand->decl) ||
          clmDECL_IsSampler(&RightOperand->decl)) {
     exprDecl = &RightOperand->decl;
  }
  else { /* both are scalar */
      if(LeftOperand->decl.dataType->elementType < RightOperand->decl.dataType->elementType) {
         exprDecl = &RightOperand->decl;
      }
      else {
         exprDecl = &LeftOperand->decl;
      }
  }

  status = cloCOMPILER_CloneDecl(Compiler,
                       clvQUALIFIER_CONST,
                       exprDecl->dataType->addrSpaceQualifier,
                       exprDecl,
                       Decl);
  if (gcmIS_ERROR(status)) return status;
  return gcvSTATUS_OK;
}

static gceSTATUS
_GetLogicalExprDecl(
IN cloCOMPILER Compiler,
IN cloIR_EXPR LeftOperand,
IN cloIR_EXPR RightOperand,
OUT clsDECL *Decl
)
{
  gctINT8 vectorSize;
  clsDATA_TYPE resDataType[1];
  clsDECL *exprDecl;
  gctINT type;

  /* Verify the arguments. */
  clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
  gcmASSERT(LeftOperand);
  gcmASSERT(LeftOperand->decl.dataType);
  gcmASSERT(RightOperand);
  gcmASSERT(RightOperand->decl.dataType);
  gcmASSERT(Decl);

  exprDecl = &LeftOperand->decl;
  vectorSize = clmDATA_TYPE_vectorSize_NOCHECK_GET(LeftOperand->decl.dataType);
  if(clmDECL_IsVectorType(&RightOperand->decl)) {
      exprDecl = &RightOperand->decl;
      vectorSize = clmDATA_TYPE_vectorSize_NOCHECK_GET(RightOperand->decl.dataType);
  }

  if (vectorSize == 0 || !clmDECL_IsIntegerType(exprDecl)) {
      _clmInitGenType(clvTYPE_INT,
                      vectorSize,
                      resDataType);
      type = resDataType->type;
  }
  else if (clmDATA_TYPE_IsHighPrecision(LeftOperand->decl.dataType) ||
           clmDATA_TYPE_IsHighPrecision(RightOperand->decl.dataType)) {
      _clmInitGenType(clvTYPE_LONG,
                      vectorSize,
                      resDataType);
      type = resDataType->type;
  }
  else {
      type = _ConvUnsignedToSigned(exprDecl->dataType->type);
  }

  return cloCOMPILER_CreateDecl(Compiler,
                                type,
                                gcvNULL,
                                clvQUALIFIER_CONST,
                                clvQUALIFIER_NONE,
                                Decl);
}

static gceSTATUS
_GetRelationalExprDecl(
IN cloCOMPILER Compiler,
IN cloIR_EXPR LeftOperand,
IN cloIR_EXPR RightOperand,
OUT clsDECL *Decl
)
{
  gceSTATUS status;
  clsDECL *exprDecl;
  gctINT type;
  clsDECL decl;

  /* Verify the arguments. */
  clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
  gcmASSERT(LeftOperand);
  gcmASSERT(LeftOperand->decl.dataType);
  gcmASSERT(RightOperand);
  gcmASSERT(RightOperand->decl.dataType);
  gcmASSERT(Decl);

  gcmASSERT(clmDECL_IsScalar(&LeftOperand->decl) || clmDECL_IsVectorType(&LeftOperand->decl));
  gcmASSERT(clmDECL_IsScalar(&RightOperand->decl) || clmDECL_IsVectorType(&RightOperand->decl));

  if(clmDECL_IsVectorType(&LeftOperand->decl)) {
     exprDecl = &LeftOperand->decl;
  }
  else if(clmDECL_IsVectorType(&RightOperand->decl)) {
     exprDecl = &RightOperand->decl;
  }
  else { /* both are scalar */
     gctINT type = T_INT;

     status = cloCOMPILER_CreateDecl(Compiler,
                                     type,
                                     gcvNULL,
                                     clvQUALIFIER_CONST,
                                     clvQUALIFIER_NONE,
                                     Decl);
     if (gcmIS_ERROR(status)) return status;
     return gcvSTATUS_OK;
  }

  decl = *exprDecl;
  type = _ConvUnsignedToSigned(exprDecl->dataType->type);
  if(type != exprDecl->dataType->type) {
     status = cloCOMPILER_CreateDataType(Compiler,
                                         type,
                                         gcvNULL,
                                         clvQUALIFIER_CONST,
                                         clvQUALIFIER_NONE,
                                         &decl.dataType);
     if (gcmIS_ERROR(status)) return status;
  }

  status = cloCOMPILER_CloneDecl(Compiler,
                                 clvQUALIFIER_CONST,
                                 clvQUALIFIER_NONE,
                                 &decl,
                                 Decl);
  if (gcmIS_ERROR(status)) return status;
  return gcvSTATUS_OK;
}

static gceSTATUS
_GetBinaryExprDecl(
    IN cloCOMPILER Compiler,
    IN cleBINARY_EXPR_TYPE Type,
    IN cloIR_EXPR LeftOperand,
    IN cloIR_EXPR RightOperand,
    OUT clsDECL *Decl,
    OUT cloIR_UNARY_EXPR *ModifierExpr
    )
{
    gceSTATUS status;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    gcmASSERT(LeftOperand);
    gcmASSERT(LeftOperand->decl.dataType);
    gcmASSERT(RightOperand);
    gcmASSERT(RightOperand->decl.dataType);
    gcmASSERT(Decl);

    switch (Type) {
    case clvBINARY_SUBSCRIPT:
        status = cloCOMPILER_CreateElementDecl(Compiler,
                                               &LeftOperand->decl,
                                               Decl);
        if (gcmIS_ERROR(status)) return status;
        if (Decl->dataType->elementType == clvTYPE_HALF &&
            !clmDECL_IsPointerType(Decl) && !clmDECL_IsArray(Decl)) {
            gctBOOL addNullExpr = gcvFALSE;

            if(clmDECL_IsPointerType(&LeftOperand->decl)) addNullExpr = gcvTRUE;
            else {
               clsNAME *variable;
               variable = clParseFindLeafName(Compiler,
                                              LeftOperand);
               if(variable && clmGEN_CODE_checkVariableForMemory(variable)) addNullExpr = gcvTRUE;
            }
            if(addNullExpr) {
                gctINT resultType;
                resultType = clGetVectorTerminalToken(clvTYPE_FLOAT,
                                                      clmDATA_TYPE_vectorSize_GET(Decl->dataType));
                if(resultType) {
                    clsDECL decl[1];
                    status = cloCOMPILER_CreateDecl(Compiler,
                                                    resultType,
                                                    Decl->dataType->u.generic,
                                                    Decl->dataType->accessQualifier,
                                                    Decl->dataType->addrSpaceQualifier,
                                                    decl);
                    if (gcmIS_ERROR(status)) return status;
                    status = cloIR_NULL_EXPR_Construct(Compiler,
                                                       LeftOperand->base.lineNo,
                                                       LeftOperand->base.stringNo,
                                                       decl,
                                                       ModifierExpr);
                    if (gcmIS_ERROR(status)) return status;
                }
            }
        }
        break;

    case clvBINARY_ADD:
    case clvBINARY_SUB:
    case clvBINARY_MUL:
    case clvBINARY_DIV:
    case clvBINARY_MOD:
        status = cloIR_GetArithmeticExprDecl(Compiler,
                                        (Type == clvBINARY_MUL),
                                        LeftOperand,
                                        RightOperand,
                                        Decl);
        if (gcmIS_ERROR(status)) return status;
        break;

    case clvBINARY_GREATER_THAN:
    case clvBINARY_LESS_THAN:
    case clvBINARY_GREATER_THAN_EQUAL:
    case clvBINARY_LESS_THAN_EQUAL:

    case clvBINARY_EQUAL:
    case clvBINARY_NOT_EQUAL:
        status = _GetRelationalExprDecl(Compiler,
                                        LeftOperand,
                                        RightOperand,
                                        Decl);
        if (gcmIS_ERROR(status)) return status;
        break;

    case clvBINARY_AND:
    case clvBINARY_OR:
    case clvBINARY_XOR:
        status = _GetLogicalExprDecl(Compiler,
                                     LeftOperand,
                                     RightOperand,
                                     Decl);
        if (gcmIS_ERROR(status)) return status;
        break;

    case clvBINARY_AND_BITWISE:
    case clvBINARY_OR_BITWISE:
    case clvBINARY_XOR_BITWISE:
        status = _GetBitwiseLogicalExprDecl(Compiler,
                                            LeftOperand,
                                            RightOperand,
                                            Decl);
        if (gcmIS_ERROR(status)) return status;
        break;

    case clvBINARY_LSHIFT:
    case clvBINARY_RSHIFT:
        if (clmDECL_IsScalar(&LeftOperand->decl) &&
            !clmIsElementTypeHighPrecision(LeftOperand->decl.dataType->elementType)) {
            clsDATA_TYPE *dataType;
            gctINT typeToken;

            if(clmIsElementTypeUnsigned(LeftOperand->decl.dataType->elementType)) {
                typeToken = T_UINT;
            }
            else {
                typeToken = T_INT;
            }
            status = cloCOMPILER_CreateDataType(Compiler,
                                                typeToken,
                                                gcvNULL,
                                                clvQUALIFIER_CONST,
                                                LeftOperand->decl.dataType->addrSpaceQualifier,
                                                &dataType);
            if (gcmIS_ERROR(status)) return status;

            clmDECL_Initialize(Decl,
                               dataType,
                               &LeftOperand->decl.array,
                               LeftOperand->decl.ptrDscr,
                               LeftOperand->decl.ptrDominant,
                               LeftOperand->decl.storageQualifier);
        }
        else {
            status = cloCOMPILER_CloneDecl(Compiler,
                                           clvQUALIFIER_CONST,
                                           LeftOperand->decl.dataType->addrSpaceQualifier,
                                           &LeftOperand->decl,
                                           Decl);
            if (gcmIS_ERROR(status)) return status;
        }
        break;

    case clvBINARY_SEQUENCE:
        status = cloCOMPILER_CloneDecl(Compiler,
                                       clvQUALIFIER_CONST,
                                       RightOperand->decl.dataType->addrSpaceQualifier,
                                       &RightOperand->decl,
                                       Decl);
        if (gcmIS_ERROR(status)) return status;
        break;

    case clvBINARY_ASSIGN:

    case clvBINARY_MUL_ASSIGN:
    case clvBINARY_DIV_ASSIGN:
    case clvBINARY_ADD_ASSIGN:
    case clvBINARY_SUB_ASSIGN:
    case clvBINARY_MOD_ASSIGN:

    case clvBINARY_LEFT_ASSIGN:
    case clvBINARY_RIGHT_ASSIGN:
    case clvBINARY_AND_ASSIGN:
    case clvBINARY_XOR_ASSIGN:
    case clvBINARY_OR_ASSIGN:
    case clvBINARY_MULTI_DIM_SUBSCRIPT:
        status = cloCOMPILER_CloneDecl(Compiler,
                                       clvQUALIFIER_CONST,
                                       LeftOperand->decl.dataType->addrSpaceQualifier,
                                       &LeftOperand->decl,
                                       Decl);
        if (gcmIS_ERROR(status)) return status;
        break;

    default: gcmASSERT(0);
        return gcvSTATUS_INVALID_ARGUMENT;
    }

    return gcvSTATUS_OK;
}

gceSTATUS
cloIR_ArrayDeclarator_Construct(
IN cloCOMPILER Compiler,
IN gctUINT LineNo,
IN gctUINT StringNo,
IN cloIR_EXPR ArrayDecl,
IN cloIR_EXPR ArraySize,
OUT cloIR_BINARY_EXPR *BinaryExpr
)
{
   gceSTATUS status;
   clsDECL decl;
   cloIR_BINARY_EXPR  binaryExpr;

   /* Verify the arguments. */
   clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
   gcmASSERT(BinaryExpr);

   do {
        gctPOINTER pointer;

        status = cloCOMPILER_CreateDecl(Compiler,
                                        T_INT,
                                        gcvNULL,
                                        clvQUALIFIER_NONE,
                                        clvQUALIFIER_NONE,
                                        &decl);
        if (gcmIS_ERROR(status)) break;

        status = cloCOMPILER_Allocate(Compiler,
                          (gctSIZE_T)sizeof(struct _cloIR_BINARY_EXPR),
                          (gctPOINTER *) &pointer);
        if (gcmIS_ERROR(status)) break;
        binaryExpr = pointer;

        cloIR_EXPR_Initialize(&binaryExpr->exprBase, &s_binaryExprVTab, LineNo, StringNo,
                      decl);
        binaryExpr->type = clvBINARY_MULTI_DIM_SUBSCRIPT;
        binaryExpr->leftOperand    = ArrayDecl;
        binaryExpr->rightOperand = ArraySize;

        *BinaryExpr = binaryExpr;
        return gcvSTATUS_OK;
   } while (gcvFALSE);

   *BinaryExpr = gcvNULL;
   return status;
}

gceSTATUS
cloIR_BINARY_EXPR_Construct(
IN cloCOMPILER Compiler,
IN gctUINT LineNo,
IN gctUINT StringNo,
IN cleBINARY_EXPR_TYPE Type,
IN cloIR_EXPR LeftOperand,
IN cloIR_EXPR RightOperand,
OUT cloIR_EXPR *ResExpr
)
{
   gceSTATUS status;
   clsDECL decl;
   cloIR_BINARY_EXPR  binaryExpr;
   cloIR_UNARY_EXPR modifierExpr = gcvNULL;

   /* Verify the arguments. */
   clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
   gcmASSERT(LeftOperand);
   gcmASSERT(RightOperand);
   gcmASSERT(ResExpr);

   do {
    gctPOINTER pointer;

    clmDECL_Initialize(&decl, gcvNULL, (clsARRAY *)0, gcvNULL, gcvFALSE, clvSTORAGE_QUALIFIER_NONE);
    status = _GetBinaryExprDecl(Compiler,
                                Type,
                                LeftOperand,
                                RightOperand,
                                &decl,
                                &modifierExpr);

    if (gcmIS_ERROR(status)) break;

    gcmASSERT(decl.dataType);

    if(clmDECL_IsFloatingType(&decl)) {
       clSetFloatOpsUsed(Compiler, Type);
    }

    status = cloCOMPILER_Allocate(Compiler,
                      (gctSIZE_T)sizeof(struct _cloIR_BINARY_EXPR),
                      (gctPOINTER *) &pointer);
    if (gcmIS_ERROR(status)) break;
    binaryExpr = pointer;

    cloIR_EXPR_Initialize(&binaryExpr->exprBase, &s_binaryExprVTab, LineNo, StringNo,
                  decl);
    binaryExpr->type = Type;
    binaryExpr->leftOperand    = LeftOperand;
    binaryExpr->rightOperand = RightOperand;

    /* Do the implicit type conversion. */
    status = cloIR_BINARY_EXPR_ImplicitTypeConv(Compiler,
                                                binaryExpr);

    if(modifierExpr) {
        modifierExpr->operand = &binaryExpr->exprBase;
        *ResExpr = &modifierExpr->exprBase;
    }
    else {
        *ResExpr = &binaryExpr->exprBase;
    }
    return gcvSTATUS_OK;
   } while (gcvFALSE);

   *ResExpr = gcvNULL;
   return status;
}

gceSTATUS
cloIR_BINARY_EXPR_Evaluate(
IN cloCOMPILER Compiler,
IN cleBINARY_EXPR_TYPE Type,
IN cloIR_CONSTANT LeftConstant,
IN cloIR_CONSTANT RightConstant,
IN clsDECL *ResultType,
OUT cloIR_CONSTANT * ResultConstant
)
{
    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(LeftConstant, clvIR_CONSTANT);
    clmVERIFY_IR_OBJECT(RightConstant, clvIR_CONSTANT);

    switch (Type) {
    case clvBINARY_SUBSCRIPT:
        return _cloIR_CONSTANT_Subscript(Compiler,
                         LeftConstant,
                         RightConstant,
                         ResultType,
                         ResultConstant);

    case clvBINARY_ADD:
    case clvBINARY_SUB:
    case clvBINARY_MUL:
    case clvBINARY_DIV:
    case clvBINARY_MOD:
        return _cloIR_CONSTANT_ArithmeticOperate(Compiler,
                             Type,
                             LeftConstant,
                             RightConstant,
                             ResultConstant);

    case clvBINARY_GREATER_THAN:
    case clvBINARY_LESS_THAN:
    case clvBINARY_GREATER_THAN_EQUAL:
    case clvBINARY_LESS_THAN_EQUAL:
        return _cloIR_CONSTANT_RelationalOperate(Compiler,
                             Type,
                             LeftConstant,
                             RightConstant,
                             ResultConstant);

    case clvBINARY_EQUAL:
    case clvBINARY_NOT_EQUAL:
        return _cloIR_CONSTANT_EqualityOperate(Compiler,
                               Type,
                               LeftConstant,
                               RightConstant,
                               ResultConstant);

    case clvBINARY_AND:
    case clvBINARY_OR:
    case clvBINARY_XOR:
        return _cloIR_CONSTANT_LogicalOperate(Compiler,
                              Type,
                              LeftConstant,
                              RightConstant,
                              ResultConstant);

    case clvBINARY_AND_BITWISE:
    case clvBINARY_OR_BITWISE:
    case clvBINARY_XOR_BITWISE:
        return _cloIR_CONSTANT_BitwiseLogical(Compiler,
                              Type,
                              LeftConstant,
                              RightConstant,
                              ResultConstant);

    case clvBINARY_LSHIFT:
    case clvBINARY_RSHIFT:
        return _cloIR_CONSTANT_BitwiseShift(Compiler,
                            Type,
                            LeftConstant,
                            RightConstant,
                            ResultConstant);

    case clvBINARY_SEQUENCE:
        gcmVERIFY_OK(cloIR_OBJECT_Destroy(Compiler, &LeftConstant->exprBase.base));

        *ResultConstant = RightConstant;
        (*ResultConstant)->allValuesEqual = gcvFALSE;
        return gcvSTATUS_OK;

    default:
        gcmASSERT(0);

        return gcvSTATUS_INVALID_ARGUMENT;
    }
}

static gctBOOL
_checkNeedImplicitTypeConvForAssignment(
    IN cloCOMPILER  Compiler,
    IN clsDATA_TYPE *LeftExprDataType,
    IN clsDATA_TYPE *RightExprDataType,
    OUT clsDATA_TYPE ** DataType
    )
{
    gctBOOL         converted = gcvFALSE;
    cltELEMENT_TYPE leftElementType = LeftExprDataType->elementType;
    cltELEMENT_TYPE rightElementType = RightExprDataType->elementType;
    gctSIZE_T       leftElementTypeByteSize = clGetElementTypeByteSize(Compiler, leftElementType);
    gctSIZE_T       rightElementTypeByteSize = clGetElementTypeByteSize(Compiler, rightElementType);
    clsDATA_TYPE   *newDataType = gcvNULL;
    cltELEMENT_TYPE newElementType;
    VIR_TypeId      newVirPrimitiveType = VIR_INVALID_ID;

    if (leftElementTypeByteSize < rightElementTypeByteSize)
    {
        return gcvFALSE;
    }

    if (clmIsElementTypeFloating(leftElementType))
    {
        if (clmIsElementTypeInteger(rightElementType))
        {
            newElementType = clGenElementTypeByByteSizeAndBaseType(Compiler,
                                                                   rightElementType,
                                                                   gcvFALSE,
                                                                   leftElementTypeByteSize,
                                                                   &newVirPrimitiveType);

            if (newElementType != rightElementType)
            {
                cloCOMPILER_CloneDataTypeExplicit(Compiler,
                                                  RightExprDataType->accessQualifier,
                                                  RightExprDataType->addrSpaceQualifier,
                                                  RightExprDataType,
                                                  &newDataType);
                gcmASSERT(newDataType);

                newDataType->elementType = newElementType;
                newDataType->virPrimitiveType = newVirPrimitiveType;
                converted = gcvTRUE;
            }
        }
    }

    if (converted && DataType)
    {
        gcmASSERT(newDataType);
        *DataType = newDataType;
    }

    return converted;
}

static gctBOOL
_NeedImplicitTypeConv(
    IN cloIR_BINARY_EXPR BinaryExpr
    )
{
    gctBOOL need = gcvFALSE;
    clsDATA_TYPE *leftExprDataType;
    clsDATA_TYPE *rightExprDataType;
    cloIR_BINARY_EXPR binaryExpr = BinaryExpr;

    if(BinaryExpr->type == clvBINARY_ASSIGN)
    {
        /* Right now only check arithmetic binary. */
        if (cloIR_OBJECT_GetType(&BinaryExpr->rightOperand->base) != clvIR_BINARY_EXPR)
        {
            return gcvFALSE;
        }
        binaryExpr = (cloIR_BINARY_EXPR) &BinaryExpr->rightOperand->base;
    }
    else if(!(clmDECL_IsScalarInteger(&BinaryExpr->leftOperand->decl) &&
              clmDECL_IsScalarInteger(&BinaryExpr->rightOperand->decl))) /*for integer promotion */
    {
        return gcvFALSE;
    }
    if (!(binaryExpr->type == clvBINARY_ADD  ||
          binaryExpr->type == clvBINARY_SUB  ||
          binaryExpr->type == clvBINARY_MUL  ||
          binaryExpr->type == clvBINARY_AND  ||
          binaryExpr->type == clvBINARY_OR   ||
          binaryExpr->type == clvBINARY_XOR))
    {
        return gcvFALSE;
    }

    leftExprDataType = BinaryExpr->leftOperand->decl.dataType;
    rightExprDataType = BinaryExpr->rightOperand->decl.dataType;

    /* Check floating and integer only. */
    if ((clmIsElementTypeFloating(leftExprDataType->elementType) || clmIsElementTypeInteger(leftExprDataType->elementType))
        &&
        (clmIsElementTypeFloating(rightExprDataType->elementType) || clmIsElementTypeInteger(rightExprDataType->elementType)))
    {
        need = gcvTRUE;
    }

    /* Skip packed type now. */
    if (clmIsElementTypePacked(leftExprDataType->elementType) || clmIsElementTypePacked(rightExprDataType->elementType))
    {
        need = gcvFALSE;
    }

    return need;
}

gceSTATUS
cloIR_BINARY_EXPR_ImplicitTypeConv(
    IN cloCOMPILER Compiler,
    IN cloIR_BINARY_EXPR BinaryExpr
    )
{
    gceSTATUS       status = gcvSTATUS_OK;
    clsDATA_TYPE    *newRightExprDataType = gcvNULL;

    if (!_NeedImplicitTypeConv(BinaryExpr))
    {
        return status;
    }

    switch (BinaryExpr->type)
    {
    case clvBINARY_ASSIGN:
        if (_checkNeedImplicitTypeConvForAssignment(Compiler,
                                                    BinaryExpr->leftOperand->decl.dataType,
                                                    BinaryExpr->rightOperand->decl.dataType,
                                                    &newRightExprDataType))
        {
            /* Update the dataType. */
            BinaryExpr->rightOperand->decl.dataType = newRightExprDataType;
        }
        break;

    default:
       if(clmDECL_IsScalarInteger(&BinaryExpr->leftOperand->decl) &&
          clmDECL_IsScalarInteger(&BinaryExpr->rightOperand->decl)) /*for integer promotion */
       {
           clsDECL decl[1];
           clsDATA_TYPE dataType[1];
           clsDATA_TYPE    *leftExprDataType = BinaryExpr->leftOperand->decl.dataType;
           clsDATA_TYPE    *rightExprDataType = BinaryExpr->rightOperand->decl.dataType;

           *dataType = *leftExprDataType;
           clmDECL_Initialize(decl, dataType, (clsARRAY *)0, gcvNULL, gcvFALSE, clvSTORAGE_QUALIFIER_NONE);
           dataType->type = T_INT;
           dataType->elementType = clvTYPE_INT;

           if((!clmDECL_IsPointerType(&BinaryExpr->leftOperand->decl) && leftExprDataType->elementType < clvTYPE_INT) &&
              (!clmDECL_IsPointerType(&BinaryExpr->rightOperand->decl) && rightExprDataType->elementType < clvTYPE_INT))
           {
               status = cloCOMPILER_CloneDecl(Compiler,
                                              BinaryExpr->exprBase.decl.dataType->accessQualifier,
                                              BinaryExpr->exprBase.decl.dataType->addrSpaceQualifier,
                                              decl,
                                              &BinaryExpr->exprBase.decl);
               if (gcmIS_ERROR(status)) return status;
           }
        }
        break;
    }

    return status;
}

/* cloIR_SELECTION object. */
gceSTATUS
cloIR_SELECTION_Destroy(
IN cloCOMPILER Compiler,
IN cloIR_BASE This
)
{
    cloIR_SELECTION    selection = (cloIR_SELECTION)This;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(selection, clvIR_SELECTION);

    gcmASSERT(selection->condExpr);
    gcmVERIFY_OK(cloIR_OBJECT_Destroy(Compiler, &selection->condExpr->base));

    if (selection->trueOperand != gcvNULL) {
        gcmVERIFY_OK(cloIR_OBJECT_Destroy(Compiler, selection->trueOperand));
    }

    if (selection->falseOperand != gcvNULL) {
        gcmVERIFY_OK(cloIR_OBJECT_Destroy(Compiler, selection->falseOperand));
    }

    if (selection->exprBase.asmMods != gcvNULL)
    {
        gcmVERIFY_OK(cloCOMPILER_Free(Compiler, selection->exprBase.asmMods));
    }

    gcmVERIFY_OK(cloCOMPILER_Free(Compiler, selection));

    return gcvSTATUS_OK;
}

gceSTATUS
cloIR_SELECTION_Dump(
IN cloCOMPILER Compiler,
IN cloIR_BASE This
)
{
    cloIR_SELECTION    selection = (cloIR_SELECTION)This;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(selection, clvIR_SELECTION);

    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                      clvDUMP_IR,
                      "<IR_SELECTION line=\"%d\" string=\"%d\" dataType=\"0x%x\">",
                      selection->exprBase.base.lineNo,
                      selection->exprBase.base.stringNo,
                      selection->exprBase.decl.dataType));

    gcmASSERT(selection->condExpr);

    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                      clvDUMP_IR,
                      "<!-- Condition Expression -->"));

    gcmVERIFY_OK(cloIR_OBJECT_Dump(Compiler, &selection->condExpr->base));

    if (selection->trueOperand != gcvNULL) {
        gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                          clvDUMP_IR,
                          "<!-- True Operand -->"));

        gcmVERIFY_OK(cloIR_OBJECT_Dump(Compiler, selection->trueOperand));
    }

    if (selection->falseOperand != gcvNULL) {
        gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                          clvDUMP_IR,
                          "<!-- False Operand -->"));

        gcmVERIFY_OK(cloIR_OBJECT_Dump(Compiler, selection->falseOperand));
    }

    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                      clvDUMP_IR,
                      "</IR_SELECTION>"));
    return gcvSTATUS_OK;
}

gceSTATUS
cloIR_SELECTION_Accept(
IN cloCOMPILER Compiler,
IN cloIR_BASE This,
IN clsVISITOR * Visitor,
IN OUT gctPOINTER Parameters
)
{
    cloIR_SELECTION    selection = (cloIR_SELECTION)This;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(selection, clvIR_SELECTION);
    gcmASSERT(Visitor);

    if (Visitor->visitSelection == gcvNULL) return gcvSTATUS_OK;

    return Visitor->visitSelection(Compiler, Visitor, selection, Parameters);
}

static clsVTAB s_selectionVTab =
{
    clvIR_SELECTION,
    cloIR_SELECTION_Destroy,
    cloIR_SELECTION_Dump,
    cloIR_SELECTION_Accept
};

gceSTATUS
cloIR_SELECTION_Construct(
IN cloCOMPILER Compiler,
IN gctUINT LineNo,
IN gctUINT StringNo,
IN clsDECL * Decl,
IN cloIR_EXPR CondExpr,
IN cloIR_BASE TrueOperand,
IN cloIR_BASE FalseOperand,
OUT cloIR_SELECTION * Selection
)
{
    gceSTATUS    status;
    cloIR_SELECTION    selection;
    gctPOINTER pointer;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    gcmASSERT(CondExpr);

    do {
        status = cloCOMPILER_Allocate(Compiler,
                          (gctSIZE_T)sizeof(struct _cloIR_SELECTION),
                          (gctPOINTER *) &pointer);
        if (gcmIS_ERROR(status)) break;

        selection = pointer;
        cloIR_EXPR_Initialize(&selection->exprBase, &s_selectionVTab, LineNo, StringNo,
                      *Decl);

        selection->condExpr    = CondExpr;
        selection->trueOperand    = TrueOperand;
        selection->falseOperand    = FalseOperand;

        *Selection = selection;
        return gcvSTATUS_OK;
    } while (gcvFALSE);

    *Selection = gcvNULL;
    return status;
}

/* cloIR_SWITCH object. */
gceSTATUS
cloIR_SWITCH_Destroy(
IN cloCOMPILER Compiler,
IN cloIR_BASE This
)
{
    cloIR_SWITCH    selection = (cloIR_SWITCH)This;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(selection, clvIR_SWITCH);

    gcmASSERT(selection->condExpr);
    gcmVERIFY_OK(cloIR_OBJECT_Destroy(Compiler, &selection->condExpr->base));

    if (selection->switchBody != gcvNULL) {
        gcmVERIFY_OK(cloIR_OBJECT_Destroy(Compiler, selection->switchBody));
    }

    if (selection->exprBase.asmMods != gcvNULL)
    {
        gcmVERIFY_OK(cloCOMPILER_Free(Compiler, selection->exprBase.asmMods));
    }

    gcmVERIFY_OK(cloCOMPILER_Free(Compiler, selection));

    return gcvSTATUS_OK;
}

gceSTATUS
cloIR_SWITCH_Dump(
IN cloCOMPILER Compiler,
IN cloIR_BASE This
)
{
    cloIR_SWITCH    selection = (cloIR_SWITCH)This;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(selection, clvIR_SWITCH);

    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                      clvDUMP_IR,
                      "<IR_SWITCH line=\"%d\" string=\"%d\" dataType=\"0x%x\">",
                      selection->exprBase.base.lineNo,
                      selection->exprBase.base.stringNo,
                      selection->exprBase.decl.dataType));

    gcmASSERT(selection->condExpr);

    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                      clvDUMP_IR,
                      "<!-- Condition Expression -->"));

    gcmVERIFY_OK(cloIR_OBJECT_Dump(Compiler, &selection->condExpr->base));

    if (selection->switchBody != gcvNULL) {
        gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                          clvDUMP_IR,
                          "<!-- Switch Body -->"));

        gcmVERIFY_OK(cloIR_OBJECT_Dump(Compiler, selection->switchBody));
    }

    if (selection->cases != gcvNULL) {
        gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                          clvDUMP_IR,
                          "<!-- cases -->"));
/** to do **

 ** Print the cases **/
    }

    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                      clvDUMP_IR,
                      "</IR_SWITCH>"));
    return gcvSTATUS_OK;
}

gceSTATUS
cloIR_SWITCH_Accept(
IN cloCOMPILER Compiler,
IN cloIR_BASE This,
IN clsVISITOR * Visitor,
IN OUT gctPOINTER Parameters
)
{
    cloIR_SWITCH    selection = (cloIR_SWITCH)This;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(selection, clvIR_SWITCH);
    gcmASSERT(Visitor);

    if (Visitor->visitSwitch == gcvNULL) return gcvSTATUS_OK;

    return Visitor->visitSwitch(Compiler, Visitor, selection, Parameters);
}

static clsVTAB s_switchVTab =
{
    clvIR_SWITCH,
    cloIR_SWITCH_Destroy,
    cloIR_SWITCH_Dump,
    cloIR_SWITCH_Accept
};

gceSTATUS
cloIR_SWITCH_Construct(
IN cloCOMPILER Compiler,
IN gctUINT LineNo,
IN gctUINT StringNo,
IN clsDECL * Decl,
IN cloIR_EXPR CondExpr,
IN cloIR_BASE SwitchBody,
IN cloIR_LABEL Cases,
OUT cloIR_SWITCH * SwitchSelect
)
{
    gceSTATUS    status;
    cloIR_SWITCH    switchSelect;
    gctPOINTER pointer;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    gcmASSERT(CondExpr);

    do {
        status = cloCOMPILER_Allocate(Compiler,
                          (gctSIZE_T)sizeof(struct _cloIR_SWITCH),
                          (gctPOINTER *) &pointer);
        if (gcmIS_ERROR(status)) break;

        switchSelect = pointer;
        cloIR_EXPR_Initialize(&switchSelect->exprBase, &s_switchVTab, LineNo, StringNo,
                      *Decl);

        switchSelect->condExpr    = CondExpr;
        switchSelect->switchBody = SwitchBody;
        switchSelect->cases = Cases;

        *SwitchSelect = switchSelect;
        return gcvSTATUS_OK;
    } while (gcvFALSE);

    *SwitchSelect = gcvNULL;
    return status;
}

/* cloIR_POLYNARY_EXPR object. */
gceSTATUS
cloIR_POLYNARY_EXPR_Destroy(
IN cloCOMPILER Compiler,
IN cloIR_BASE This
)
{
    cloIR_POLYNARY_EXPR    polynaryExpr = (cloIR_POLYNARY_EXPR)This;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(polynaryExpr, clvIR_POLYNARY_EXPR);

    if (polynaryExpr->operands != gcvNULL) {
        gcmVERIFY_OK(cloIR_OBJECT_Destroy(Compiler, &polynaryExpr->operands->base));
    }

    if (polynaryExpr->exprBase.asmMods != gcvNULL)
    {
        gcmVERIFY_OK(cloCOMPILER_Free(Compiler, polynaryExpr->exprBase.asmMods));
    }

    gcmVERIFY_OK(cloCOMPILER_Free(Compiler, polynaryExpr));
    return gcvSTATUS_OK;
}

gctCONST_STRING
clGetIRPolynaryExprTypeName(
IN clePOLYNARY_EXPR_TYPE PolynaryExprType
)
{
    switch (PolynaryExprType) {
    case clvPOLYNARY_CONSTRUCT_SCALAR:    return "construct_scalar";
    case clvPOLYNARY_CONSTRUCT_ARRAY:    return "construct_array";
    case clvPOLYNARY_CONSTRUCT_VECTOR:    return "construct_vector";
    case clvPOLYNARY_CONSTRUCT_MATRIX:    return "construct_matrix";
    case clvPOLYNARY_CONSTRUCT_STRUCT:    return "construct_struct";

    case clvPOLYNARY_FUNC_CALL:           return "function_call";
    case clvPOLYNARY_BUILT_IN_ASM_CALL:   return "built_in_asm_call";
    default:
        gcmASSERT(0);
        return "invalid";
    }
}

gceSTATUS
cloIR_POLYNARY_EXPR_Dump(
IN cloCOMPILER Compiler,
IN cloIR_BASE This
)
{
    cloIR_POLYNARY_EXPR    polynaryExpr = (cloIR_POLYNARY_EXPR)This;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(polynaryExpr, clvIR_POLYNARY_EXPR);

    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                      clvDUMP_IR,
                      "<IR_POLYNARY_EXPR line=\"%d\" string=\"%d\""
                      " dataType=\"0x%x\" type=\"%s\"",
                    polynaryExpr->exprBase.base.lineNo,
                    polynaryExpr->exprBase.base.stringNo,
                    polynaryExpr->exprBase.decl.dataType,
                    clGetIRPolynaryExprTypeName(polynaryExpr->type)));

    if (polynaryExpr->type == clvPOLYNARY_FUNC_CALL) {
        gcmASSERT(polynaryExpr->funcName);

        gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                          clvDUMP_IR,
                          " funcSymbol=\"%s\">",
                          polynaryExpr->funcSymbol));
    }
    else {
        gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                          clvDUMP_IR,
                          ">"));
    }

    if (polynaryExpr->funcName != gcvNULL) {
        gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                          clvDUMP_IR,
                          "<!-- Function Name -->"));
        gcmVERIFY_OK(clsNAME_Dump(Compiler, polynaryExpr->funcName));
    }

    if (polynaryExpr->operands != gcvNULL) {
        gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                          clvDUMP_IR,
                          "<!-- Operands -->"));
        gcmVERIFY_OK(cloIR_OBJECT_Dump(Compiler, &polynaryExpr->operands->base));
    }

    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                      clvDUMP_IR,
                      "</IR_POLYNARY_EXPR>"));
    return gcvSTATUS_OK;
}

gceSTATUS
cloIR_POLYNARY_EXPR_Accept(
IN cloCOMPILER Compiler,
IN cloIR_BASE This,
IN clsVISITOR * Visitor,
IN OUT gctPOINTER Parameters
)
{
    cloIR_POLYNARY_EXPR    polynaryExpr = (cloIR_POLYNARY_EXPR)This;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(polynaryExpr, clvIR_POLYNARY_EXPR);
    gcmASSERT(Visitor);

    if (Visitor->visitPolynaryExpr == gcvNULL) return gcvSTATUS_OK;

    return Visitor->visitPolynaryExpr(Compiler, Visitor, polynaryExpr, Parameters);
}

static clsVTAB s_polynaryExprVTab =
{
    clvIR_POLYNARY_EXPR,
    cloIR_POLYNARY_EXPR_Destroy,
    cloIR_POLYNARY_EXPR_Dump,
    cloIR_POLYNARY_EXPR_Accept
};

gceSTATUS
cloIR_POLYNARY_EXPR_Construct(
IN cloCOMPILER Compiler,
IN gctUINT LineNo,
IN gctUINT StringNo,
IN clePOLYNARY_EXPR_TYPE Type,
IN clsDECL * Decl,
IN cltPOOL_STRING FuncSymbol,
OUT cloIR_POLYNARY_EXPR * PolynaryExpr
)
{
    gceSTATUS        status;
    cloIR_POLYNARY_EXPR    polynaryExpr;
    gctPOINTER pointer;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);

    do {
        status = cloCOMPILER_Allocate(Compiler,
                          (gctSIZE_T)sizeof(struct _cloIR_POLYNARY_EXPR),
                          (gctPOINTER *) &pointer);
        if (gcmIS_ERROR(status)) break;

        polynaryExpr = pointer;
        cloIR_EXPR_Initialize(&polynaryExpr->exprBase, &s_polynaryExprVTab, LineNo, StringNo,
                      *Decl);

        polynaryExpr->type    = Type;
        polynaryExpr->funcSymbol= FuncSymbol;
        polynaryExpr->funcName    = gcvNULL;
        polynaryExpr->operands    = gcvNULL;

        *PolynaryExpr = polynaryExpr;
        return gcvSTATUS_OK;
    } while (gcvFALSE);

    *PolynaryExpr = gcvNULL;
    return status;
}


gceSTATUS
cloIR_TYPECAST_ARGS_Destroy(
IN cloCOMPILER Compiler,
IN cloIR_BASE This
)
{
    cloIR_TYPECAST_ARGS    typeCastArgs = (cloIR_TYPECAST_ARGS)This;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(typeCastArgs, clvIR_TYPECAST_ARGS);

    if (typeCastArgs->operands != gcvNULL) {
        gcmVERIFY_OK(cloIR_OBJECT_Destroy(Compiler, &typeCastArgs->operands->base));
    }

    if (typeCastArgs->exprBase.asmMods != gcvNULL)
    {
        gcmVERIFY_OK(cloCOMPILER_Free(Compiler, typeCastArgs->exprBase.asmMods));
    }

    gcmVERIFY_OK(cloCOMPILER_Free(Compiler, typeCastArgs));
    return gcvSTATUS_OK;
}

gceSTATUS
cloIR_TYPECAST_ARGS_Dump(
IN cloCOMPILER Compiler,
IN cloIR_BASE This
)
{
    cloIR_TYPECAST_ARGS    typeCastArgs = (cloIR_TYPECAST_ARGS)This;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(typeCastArgs, clvIR_TYPECAST_ARGS);

    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                      clvDUMP_IR,
                      "<IR_TYPECAST_ARGS line=\"%d\" string=\"%d\""
                      " args=\"0x%x\">",
                    typeCastArgs->exprBase.base.lineNo,
                    typeCastArgs->exprBase.base.stringNo,
                    typeCastArgs));

    if (typeCastArgs->operands != gcvNULL) {
        gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                          clvDUMP_IR,
                          "<!-- Operands -->"));
        gcmVERIFY_OK(cloIR_OBJECT_Dump(Compiler, &typeCastArgs->operands->base));
    }

    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                      clvDUMP_IR,
                      "</IR_TYPECAST_ARGS>"));
    return gcvSTATUS_OK;
}

gceSTATUS
cloIR_TYPECAST_ARGS_Accept(
IN cloCOMPILER Compiler,
IN cloIR_BASE This,
IN clsVISITOR * Visitor,
IN OUT gctPOINTER Parameters
)
{
    cloIR_TYPECAST_ARGS    typeCastArgs = (cloIR_TYPECAST_ARGS)This;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(typeCastArgs, clvIR_TYPECAST_ARGS);
    gcmASSERT(Visitor);

    if (Visitor->visitTypeCastArgs == gcvNULL) return gcvSTATUS_OK;

    return Visitor->visitTypeCastArgs(Compiler, Visitor, typeCastArgs, Parameters);
}

static clsVTAB s_typeCastArgsVTab =
{
    clvIR_TYPECAST_ARGS,
    cloIR_TYPECAST_ARGS_Destroy,
    cloIR_TYPECAST_ARGS_Dump,
    cloIR_TYPECAST_ARGS_Accept
};

gceSTATUS
cloIR_TYPECAST_ARGS_Construct(
IN cloCOMPILER Compiler,
IN gctUINT LineNo,
IN gctUINT StringNo,
OUT cloIR_TYPECAST_ARGS * TypeCastArgs
)
{
    gceSTATUS  status;
    cloIR_TYPECAST_ARGS typeCastArgs;
    gctPOINTER pointer;
    clsDECL decl;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmDECL_Initialize(&decl, gcvNULL, (clsARRAY *)0, gcvNULL, gcvFALSE, clvSTORAGE_QUALIFIER_NONE);
    do {
        status = cloCOMPILER_Allocate(Compiler,
                          (gctSIZE_T)sizeof(struct _cloIR_TYPECAST_ARGS),
                          (gctPOINTER *) &pointer);
        if (gcmIS_ERROR(status)) break;

        typeCastArgs = pointer;
        cloIR_EXPR_Initialize(&typeCastArgs->exprBase, &s_typeCastArgsVTab, LineNo, StringNo,
                      decl);

        typeCastArgs->operands    = gcvNULL;
        typeCastArgs->lhs = gcvNULL;
        *TypeCastArgs = typeCastArgs;
        return gcvSTATUS_OK;
    } while (gcvFALSE);

    *TypeCastArgs = gcvNULL;
    return status;
}

gceSTATUS
cloIR_POLYNARY_EXPR_ConstructScalarConstant(
IN cloCOMPILER Compiler,
IN cloIR_POLYNARY_EXPR PolynaryExpr,
OUT cloIR_CONSTANT * ResultConstant
)
{
    gceSTATUS        status;
    cloIR_CONSTANT        resultConstant, operandConstant;
    cloIR_EXPR        operand;
    cluCONSTANT_VALUE    value;
    clsDECL decl;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(PolynaryExpr, clvIR_POLYNARY_EXPR);
    gcmASSERT(ResultConstant);

    gcmASSERT(PolynaryExpr->operands);

    operand = slsDLINK_LIST_First(&PolynaryExpr->operands->members, struct _cloIR_EXPR);
    gcmASSERT(operand);

    /*init value*/
    value.floatValue = (gctFLOAT)0.0;

    do {
        /* Check if the operand is constant */
        if (cloIR_OBJECT_GetType(&operand->base) != clvIR_CONSTANT) break;

        operandConstant = (cloIR_CONSTANT)operand;

        /* Create the constant */
            status = cloCOMPILER_CloneDecl(Compiler,
                               clvQUALIFIER_CONST,
                               PolynaryExpr->exprBase.decl.dataType->addrSpaceQualifier,
                               &PolynaryExpr->exprBase.decl,
                               &decl);
            if (gcmIS_ERROR(status)) return status;

        status = cloIR_CONSTANT_Construct(Compiler,
                          PolynaryExpr->exprBase.base.lineNo,
                          PolynaryExpr->exprBase.base.stringNo,
                          &decl,
                          &resultConstant);
        if (gcmIS_ERROR(status)) return status;

        /* Get the converted constant value */
        switch (PolynaryExpr->exprBase.decl.dataType->elementType) {
        case clvTYPE_BOOL:
            gcmVERIFY_OK(cloIR_CONSTANT_GetBoolValue(Compiler,
                                 operandConstant,
                                 0,
                                 &value));
            break;

        case clvTYPE_INT:
        case clvTYPE_SHORT:
            gcmVERIFY_OK(cloIR_CONSTANT_GetIntValue(Compiler,
                                operandConstant,
                                0,
                                &value));
            break;

        case clvTYPE_LONG:
            gcmVERIFY_OK(cloIR_CONSTANT_GetLongValue(Compiler,
                                operandConstant,
                                0,
                                &value));
            break;

        case clvTYPE_UINT:
        case clvTYPE_USHORT:
        case clvTYPE_UCHAR:
            gcmVERIFY_OK(cloIR_CONSTANT_GetUintValue(Compiler,
                                operandConstant,
                                0,
                                &value));
            break;

        case clvTYPE_ULONG:
            gcmVERIFY_OK(cloIR_CONSTANT_GetULongValue(Compiler,
                                operandConstant,
                                0,
                                &value));
            break;

        case clvTYPE_FLOAT:
            gcmVERIFY_OK(cloIR_CONSTANT_GetFloatValue(Compiler,
                                  operandConstant,
                                  0,
                                  &value));
            break;

        case clvTYPE_CHAR:
            gcmVERIFY_OK(cloIR_CONSTANT_GetCharValue(Compiler,
                                 operandConstant,
                                 0,
                                 &value));
            break;

        default:
            gcmASSERT(0);
        }

        /* Add the constant value */
        status = cloIR_CONSTANT_AddValues(Compiler,
                          resultConstant,
                          1,
                          &value);
        if (gcmIS_ERROR(status)) return status;

        gcmVERIFY_OK(cloIR_OBJECT_Destroy(Compiler, &PolynaryExpr->exprBase.base));
        *ResultConstant = resultConstant;
        return gcvSTATUS_OK;
    } while (gcvFALSE);

    *ResultConstant = gcvNULL;
    return gcvSTATUS_OK;
}

static gctBOOL
_AreAllOperandsConstant(
    IN cloIR_POLYNARY_EXPR PolynaryExpr
    )
{
    cloIR_EXPR            operand;

    /* Verify the arguments. */
    clmVERIFY_IR_OBJECT(PolynaryExpr, clvIR_POLYNARY_EXPR);

    if(PolynaryExpr->operands) {
       FOR_EACH_DLINK_NODE(&PolynaryExpr->operands->members, struct _cloIR_EXPR, operand) {
        if (cloIR_OBJECT_GetType(&operand->base) != clvIR_CONSTANT) return gcvFALSE;
       }
    }

    return gcvTRUE;
}


static gceSTATUS
_SetVectorConstantValuesByOneScalarValue(
    IN cloCOMPILER Compiler,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN cloIR_CONSTANT OperandConstant,
    IN OUT cloIR_CONSTANT ResultConstant
    )
{
    gceSTATUS            status;
    gctUINT                i;
    cluCONSTANT_VALUE    value;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(PolynaryExpr, clvIR_POLYNARY_EXPR);

    /* Get the converted constant value */
    switch (ResultConstant->exprBase.decl.dataType->elementType)
    {
    case clvTYPE_BOOL:
    case clvTYPE_BOOL_PACKED:
        gcmVERIFY_OK(cloIR_CONSTANT_GetBoolValue(Compiler,
                            OperandConstant,
                            0,
                            &value));
        break;

    case clvTYPE_INT:
    case clvTYPE_SHORT:
    case clvTYPE_SHORT_PACKED:
        gcmVERIFY_OK(cloIR_CONSTANT_GetIntValue(Compiler,
                            OperandConstant,
                            0,
                            &value));
        break;

    case clvTYPE_LONG:
        gcmVERIFY_OK(cloIR_CONSTANT_GetLongValue(Compiler,
                            OperandConstant,
                            0,
                            &value));
        break;

    case clvTYPE_UINT:
    case clvTYPE_USHORT:
    case clvTYPE_UCHAR:
    case clvTYPE_USHORT_PACKED:
    case clvTYPE_UCHAR_PACKED:
        gcmVERIFY_OK(cloIR_CONSTANT_GetUintValue(Compiler,
                            OperandConstant,
                            0,
                            &value));
        break;

    case clvTYPE_ULONG:
        gcmVERIFY_OK(cloIR_CONSTANT_GetULongValue(Compiler,
                            OperandConstant,
                            0,
                            &value));
        break;

    case clvTYPE_FLOAT:
    case clvTYPE_DOUBLE:
    case clvTYPE_HALF:
    case clvTYPE_HALF_PACKED:
        gcmVERIFY_OK(cloIR_CONSTANT_GetFloatValue(Compiler,
                              OperandConstant,
                              0,
                              &value));
        break;

    case clvTYPE_CHAR:
    case clvTYPE_CHAR_PACKED:
        gcmVERIFY_OK(cloIR_CONSTANT_GetCharValue(Compiler,
                             OperandConstant,
                             0,
                             &value));
        break;

    default:
        gcmASSERT(0);
    }

    for (i = 0; i < clsDECL_GetSize(&ResultConstant->exprBase.decl); i++) {
        /* Add the constant value */
        status = cloIR_CONSTANT_AddValues(Compiler,
                          ResultConstant,
                          1,
                          &value);
        if (gcmIS_ERROR(status)) return status;
    }
    ResultConstant->allValuesEqual = gcvTRUE;
    return gcvSTATUS_OK;
}

static gceSTATUS
_SetMatrixConstantValuesByOneScalarValue(
    IN cloCOMPILER Compiler,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN cloIR_CONSTANT OperandConstant,
    IN OUT cloIR_CONSTANT ResultConstant
    )
{
    gceSTATUS    status;
    gctUINT        matrixRowCount, matrixColumnCount, i, j;
    cluCONSTANT_VALUE value, valueZero;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(PolynaryExpr, clvIR_POLYNARY_EXPR);

    /* Get the converted constant value */
    switch (ResultConstant->exprBase.decl.dataType->elementType)
    {
    case clvTYPE_BOOL:
        gcmVERIFY_OK(cloIR_CONSTANT_GetBoolValue(Compiler,
                             OperandConstant,
                             0,
                             &value));
        break;

    case clvTYPE_INT:
    case clvTYPE_SHORT:
        gcmVERIFY_OK(cloIR_CONSTANT_GetIntValue(Compiler,
                            OperandConstant,
                            0,
                            &value));
        break;

    case clvTYPE_LONG:
        gcmVERIFY_OK(cloIR_CONSTANT_GetLongValue(Compiler,
                            OperandConstant,
                            0,
                            &value));
        break;

    case clvTYPE_UINT:
    case clvTYPE_USHORT:
    case clvTYPE_UCHAR:
        gcmVERIFY_OK(cloIR_CONSTANT_GetUintValue(Compiler,
                            OperandConstant,
                            0,
                            &value));
        break;

    case clvTYPE_ULONG:
        gcmVERIFY_OK(cloIR_CONSTANT_GetULongValue(Compiler,
                            OperandConstant,
                            0,
                            &value));
        break;

    case clvTYPE_FLOAT:
        gcmVERIFY_OK(cloIR_CONSTANT_GetFloatValue(Compiler,
                              OperandConstant,
                              0,
                              &value));
        break;

    case clvTYPE_CHAR:
        gcmVERIFY_OK(cloIR_CONSTANT_GetCharValue(Compiler,
                             OperandConstant,
                             0,
                             &value));
        break;

    default:
        gcmASSERT(0);
    }

    valueZero.floatValue = (gctFLOAT)0.0;

    matrixRowCount = clmDATA_TYPE_matrixRowCount_GET(ResultConstant->exprBase.decl.dataType);
    matrixColumnCount = clmDATA_TYPE_matrixColumnCount_GET(ResultConstant->exprBase.decl.dataType);

    for (i = 0; i < matrixColumnCount; i++) {
        for (j = 0; j < matrixRowCount; j++) {
            if (i == j) {
                /* Add the specified constant value */
                status = cloIR_CONSTANT_AddValues(Compiler,
                                  ResultConstant,
                                  1,
                                  &value);
                if (gcmIS_ERROR(status)) return status;
            }
            else {
                /* Add the zero constant value */
                status = cloIR_CONSTANT_AddValues(Compiler,
                                  ResultConstant,
                                  1,
                                  &valueZero);
                if (gcmIS_ERROR(status)) return status;
            }
        }
    }

    return gcvSTATUS_OK;
}

static gceSTATUS
_SetMatrixConstantValuesByOneMatrixValue(
IN cloCOMPILER Compiler,
IN cloIR_POLYNARY_EXPR PolynaryExpr,
IN cloIR_CONSTANT OperandConstant,
IN OUT cloIR_CONSTANT ResultConstant
)
{
    gceSTATUS    status;
    gctUINT        matrixRowCount, matrixColumnCount;
    gctUINT        operandMatrixRowCount, operandMatrixColumnCount;
    gctUINT i, j;
    cluCONSTANT_VALUE    value, valueZero, valueOne;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(PolynaryExpr, clvIR_POLYNARY_EXPR);

    valueZero.floatValue    = (gctFLOAT)0.0;
    valueOne.floatValue    = (gctFLOAT)1.0;

    matrixColumnCount = clmDATA_TYPE_matrixColumnCount_GET(ResultConstant->exprBase.decl.dataType);
    matrixRowCount = clmDATA_TYPE_matrixRowCount_GET(ResultConstant->exprBase.decl.dataType);

    operandMatrixColumnCount = clmDATA_TYPE_matrixColumnCount_GET(OperandConstant->exprBase.decl.dataType);
    operandMatrixRowCount = clmDATA_TYPE_matrixRowCount_GET(OperandConstant->exprBase.decl.dataType);
    for (i = 0; i < matrixColumnCount; i++) {
        for (j = 0; j < matrixRowCount; j++) {
            if (i < operandMatrixColumnCount && j < operandMatrixRowCount) {
                /* Get the converted constant value */
                switch (ResultConstant->exprBase.decl.dataType->elementType) {
                case clvTYPE_BOOL:
                    gcmVERIFY_OK(cloIR_CONSTANT_GetBoolValue(Compiler,
                                         OperandConstant,
                                         i * operandMatrixColumnCount + j,
                                         &value));
                    break;

                case clvTYPE_INT:
                case clvTYPE_SHORT:
                    gcmVERIFY_OK(cloIR_CONSTANT_GetIntValue(Compiler,
                                        OperandConstant,
                                        i * operandMatrixColumnCount + j,
                                        &value));
                    break;

                case clvTYPE_LONG:
                    gcmVERIFY_OK(cloIR_CONSTANT_GetLongValue(Compiler,
                                        OperandConstant,
                                        i * operandMatrixColumnCount + j,
                                        &value));
                    break;

                case clvTYPE_UINT:
                case clvTYPE_USHORT:
                case clvTYPE_UCHAR:
                    gcmVERIFY_OK(cloIR_CONSTANT_GetUintValue(Compiler,
                                        OperandConstant,
                                        i * operandMatrixColumnCount + j,
                                        &value));
                    break;

                case clvTYPE_ULONG:
                    gcmVERIFY_OK(cloIR_CONSTANT_GetULongValue(Compiler,
                                        OperandConstant,
                                        i * operandMatrixColumnCount + j,
                                        &value));
                    break;

                case clvTYPE_FLOAT:
                    gcmVERIFY_OK(cloIR_CONSTANT_GetFloatValue(Compiler,
                                          OperandConstant,
                                          i * operandMatrixColumnCount + j,
                                          &value));
                    break;

                case clvTYPE_CHAR:
                    gcmVERIFY_OK(cloIR_CONSTANT_GetCharValue(Compiler,
                                         OperandConstant,
                                         i * operandMatrixColumnCount + j,
                                         &value));
                    break;

                default:
                    gcmASSERT(0);
                }

                /* Add the specified constant value */
                status = cloIR_CONSTANT_AddValues(Compiler,
                                  ResultConstant,
                                  1,
                                  &value);
                if (gcmIS_ERROR(status)) return status;
            }
            else if (i == j) {
                /* Add the one constant value */
                status = cloIR_CONSTANT_AddValues(Compiler,
                                  ResultConstant,
                                  1,
                                  &valueOne);
                if (gcmIS_ERROR(status)) return status;
            }
            else {
                /* Add the zero constant value */
                status = cloIR_CONSTANT_AddValues(Compiler,
                                  ResultConstant,
                                  1,
                                  &valueZero);
                if (gcmIS_ERROR(status)) return status;
            }
        }
    }

    return gcvSTATUS_OK;
}

static gceSTATUS
_SetVectorOrMatrixConstantValues(
IN cloCOMPILER Compiler,
IN cloIR_POLYNARY_EXPR PolynaryExpr,
IN gctBOOL IsVectorConstant,
IN OUT cloIR_CONSTANT ResultConstant
)
{
    gceSTATUS        status;
    gctUINT            operandCount, i, valueCount = 0;
    cloIR_EXPR        operand;
    cloIR_CONSTANT        operandConstant;
    cluCONSTANT_VALUE    value;
    gctUINT         operandSize, resultSize;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(PolynaryExpr, clvIR_POLYNARY_EXPR);
    gcmASSERT(PolynaryExpr->operands);

    gcmVERIFY_OK(cloIR_SET_GetMemberCount(Compiler,
                          PolynaryExpr->operands,
                          &operandCount));

    if (operandCount == 1) {
        operand = slsDLINK_LIST_First(&PolynaryExpr->operands->members, struct _cloIR_EXPR);
        gcmASSERT(operand);

        gcmASSERT(cloIR_OBJECT_GetType(&operand->base) == clvIR_CONSTANT);
        operandConstant = (cloIR_CONSTANT)operand;

        if (IsVectorConstant) {
            return _SetVectorConstantValuesByOneScalarValue(Compiler,
                                    PolynaryExpr,
                                    operandConstant,
                                    ResultConstant);
        }
        else if (clmDECL_IsScalar(&operandConstant->exprBase.decl)) {
            return _SetMatrixConstantValuesByOneScalarValue(Compiler,
                                    PolynaryExpr,
                                    operandConstant,
                                    ResultConstant);
        }
        else {
            gcmASSERT(clmDECL_IsMat(&operandConstant->exprBase.decl));

            return _SetMatrixConstantValuesByOneMatrixValue(Compiler,
                                    PolynaryExpr,
                                    operandConstant,
                                    ResultConstant);
        }
    }

    FOR_EACH_DLINK_NODE(&PolynaryExpr->operands->members, struct _cloIR_EXPR, operand) {
        gcmASSERT(cloIR_OBJECT_GetType(&operand->base) == clvIR_CONSTANT);
        operandConstant = (cloIR_CONSTANT)operand;

        operandSize = clsDECL_GetSize(&operandConstant->exprBase.decl);
        resultSize = clsDECL_GetSize(&ResultConstant->exprBase.decl);
        for (i = 0; i < operandSize; i++) {
            /* Get the converted constant value */
            switch (ResultConstant->exprBase.decl.dataType->elementType) {
            case clvTYPE_BOOL:
            case clvTYPE_BOOL_PACKED:
                gcmVERIFY_OK(cloIR_CONSTANT_GetBoolValue(Compiler,
                                    operandConstant,
                                    i,
                                    &value));
                break;

            case clvTYPE_INT:
            case clvTYPE_SHORT:
            case clvTYPE_SHORT_PACKED:
                gcmVERIFY_OK(cloIR_CONSTANT_GetIntValue(Compiler,
                                    operandConstant,
                                    i,
                                    &value));
                break;

            case clvTYPE_LONG:
                gcmVERIFY_OK(cloIR_CONSTANT_GetLongValue(Compiler,
                                    operandConstant,
                                    i,
                                    &value));
                break;

            case clvTYPE_UINT:
            case clvTYPE_USHORT:
            case clvTYPE_USHORT_PACKED:
            case clvTYPE_UCHAR:
            case clvTYPE_UCHAR_PACKED:
                gcmVERIFY_OK(cloIR_CONSTANT_GetUintValue(Compiler,
                                    operandConstant,
                                    i,
                                    &value));
                break;

            case clvTYPE_ULONG:
                gcmVERIFY_OK(cloIR_CONSTANT_GetULongValue(Compiler,
                                    operandConstant,
                                    i,
                                    &value));
                break;

            case clvTYPE_FLOAT:
            case clvTYPE_DOUBLE:
            case clvTYPE_HALF:
            case clvTYPE_HALF_PACKED:
                gcmVERIFY_OK(cloIR_CONSTANT_GetFloatValue(Compiler,
                                      operandConstant,
                                      i,
                                      &value));
                break;

            case clvTYPE_CHAR:
            case clvTYPE_CHAR_PACKED:
                gcmVERIFY_OK(cloIR_CONSTANT_GetCharValue(Compiler,
                                     operandConstant,
                                     i,
                                     &value));
                break;

            default:
                gcmASSERT(0);
            }

            /* Add the constant value */
            status = cloIR_CONSTANT_AddValues(Compiler,
                              ResultConstant,
                              1,
                              &value);
            if (gcmIS_ERROR(status)) return status;

            /* Move to the next component */
            valueCount++;
            if (valueCount == resultSize) {
                cloIR_CONSTANT_CheckAndSetAllValuesEqual(Compiler,
                                     ResultConstant);
                goto Exit;
            }
        }
    }

    gcmASSERT(0);

Exit:
    return gcvSTATUS_OK;
}

gceSTATUS
cloIR_POLYNARY_EXPR_ConstructVectorOrMatrixConstant(
IN cloCOMPILER Compiler,
IN cloIR_POLYNARY_EXPR PolynaryExpr,
IN gctBOOL IsVectorConstant,
OUT cloIR_CONSTANT * ResultConstant
)
{
    gceSTATUS        status;
    cloIR_CONSTANT   resultConstant;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(PolynaryExpr, clvIR_POLYNARY_EXPR);
    gcmASSERT(ResultConstant);

    gcmASSERT(PolynaryExpr->operands);

    do {
        clsDECL decl;

        /* Check if all operands are constant */
        if (!_AreAllOperandsConstant(PolynaryExpr)) break;

        /* Create the constant */
        status = cloCOMPILER_CloneDecl(Compiler,
                                       clvQUALIFIER_CONST,
                                       PolynaryExpr->exprBase.decl.dataType->addrSpaceQualifier,
                                       &PolynaryExpr->exprBase.decl,
                                       &decl);
        if (gcmIS_ERROR(status)) return status;

        status = cloIR_CONSTANT_Construct(Compiler,
                                          PolynaryExpr->exprBase.base.lineNo,
                                          PolynaryExpr->exprBase.base.stringNo,
                                          &decl,
                                          &resultConstant);
        if (gcmIS_ERROR(status)) return status;

        /* Convert all operand constant values */
        status = _SetVectorOrMatrixConstantValues(Compiler,
                                                  PolynaryExpr,
                                                  IsVectorConstant,
                                                  resultConstant);
        if (gcmIS_ERROR(status)) return status;
        gcmVERIFY_OK(cloIR_OBJECT_Destroy(Compiler, &PolynaryExpr->exprBase.base));

        *ResultConstant = resultConstant;
        return gcvSTATUS_OK;
    } while (gcvFALSE);

    *ResultConstant = gcvNULL;
    return gcvSTATUS_OK;
}

gceSTATUS
cloIR_POLYNARY_EXPR_ConstructStructConstant(
IN cloCOMPILER Compiler,
IN cloIR_POLYNARY_EXPR PolynaryExpr,
OUT cloIR_CONSTANT * ResultConstant
)
{
    gceSTATUS    status;
    cloIR_CONSTANT    resultConstant;
    cloIR_EXPR    operand;
    cloIR_CONSTANT    operandConstant;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(PolynaryExpr, clvIR_POLYNARY_EXPR);
    gcmASSERT(ResultConstant);

    gcmASSERT(PolynaryExpr->operands);

    do {
        clsDECL decl;

        /* Check if all operands are constant */
        if (!_AreAllOperandsConstant(PolynaryExpr)) break;

        /* Create the constant */
            status = cloCOMPILER_CloneDecl(Compiler,
                               clvQUALIFIER_CONST,
                               PolynaryExpr->exprBase.decl.dataType->addrSpaceQualifier,
                                               &PolynaryExpr->exprBase.decl,
                               &decl);
            if (gcmIS_ERROR(status)) return status;

        status = cloIR_CONSTANT_Construct(Compiler,
                          PolynaryExpr->exprBase.base.lineNo,
                          PolynaryExpr->exprBase.base.stringNo,
                          &decl,
                          &resultConstant);
        if (gcmIS_ERROR(status)) return status;

        /* Set all operand constant values */
        FOR_EACH_DLINK_NODE(&PolynaryExpr->operands->members, struct _cloIR_EXPR, operand) {
            gcmASSERT(cloIR_OBJECT_GetType(&operand->base) == clvIR_CONSTANT);
            operandConstant = (cloIR_CONSTANT)operand;

            /* Add the constant value */
            status = cloIR_CONSTANT_AddValues(Compiler,
                              resultConstant,
                              operandConstant->valueCount,
                              operandConstant->values);
            if (gcmIS_ERROR(status)) return status;
        }

        gcmVERIFY_OK(cloIR_OBJECT_Destroy(Compiler, &PolynaryExpr->exprBase.base));

        *ResultConstant = resultConstant;
        return gcvSTATUS_OK;
    }
    while (gcvFALSE);

    *ResultConstant = gcvNULL;
    return gcvSTATUS_OK;
}

gceSTATUS
cloIR_POLYNARY_EXPR_ConstructArrayConstant(
IN cloCOMPILER Compiler,
IN cloIR_POLYNARY_EXPR PolynaryExpr,
OUT cloIR_CONSTANT * ResultConstant
)
{
    gceSTATUS    status;
    cloIR_CONSTANT    resultConstant;
    cloIR_EXPR    operand;
    cloIR_CONSTANT    operandConstant;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(PolynaryExpr, clvIR_POLYNARY_EXPR);
    gcmASSERT(ResultConstant);

    gcmASSERT(PolynaryExpr->operands);

    do {
        clsDECL decl;

        /* Check if all operands are constant */
        if (!_AreAllOperandsConstant(PolynaryExpr)) break;

        /* Create the constant */
            status = cloCOMPILER_CloneDecl(Compiler,
                               clvQUALIFIER_CONST,
                               PolynaryExpr->exprBase.decl.dataType->addrSpaceQualifier,
                                               &PolynaryExpr->exprBase.decl,
                               &decl);
            if (gcmIS_ERROR(status)) return status;

        status = cloIR_CONSTANT_Construct(Compiler,
                          PolynaryExpr->exprBase.base.lineNo,
                          PolynaryExpr->exprBase.base.stringNo,
                          &decl,
                          &resultConstant);
        if (gcmIS_ERROR(status)) return status;

        /* Set all operand constant values */
        FOR_EACH_DLINK_NODE(&PolynaryExpr->operands->members, struct _cloIR_EXPR, operand) {
            gcmASSERT(cloIR_OBJECT_GetType(&operand->base) == clvIR_CONSTANT);
            operandConstant = (cloIR_CONSTANT)operand;

            /* Add the constant value */
            status = cloIR_CONSTANT_AddValues(Compiler,
                              resultConstant,
                              operandConstant->valueCount,
                              operandConstant->values);
            if (gcmIS_ERROR(status)) return status;
        }

        gcmVERIFY_OK(cloIR_OBJECT_Destroy(Compiler, &PolynaryExpr->exprBase.base));

        *ResultConstant = resultConstant;
        return gcvSTATUS_OK;
    }
    while (gcvFALSE);

    *ResultConstant = gcvNULL;
    return gcvSTATUS_OK;
}

gceSTATUS
cloIR_POLYNARY_EXPR_EvaluateBuiltin(
IN cloCOMPILER Compiler,
IN cloIR_POLYNARY_EXPR PolynaryExpr,
OUT cloIR_CONSTANT * ResultConstant
)
{
    gceSTATUS    status;
    cloIR_EXPR    operand;
    gctUINT        i;
    cloIR_CONSTANT    operandConstants[clmMAX_BUILT_IN_PARAMETER_COUNT];

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(PolynaryExpr, clvIR_POLYNARY_EXPR);
    gcmASSERT(ResultConstant);

    do {
        /* Check if all operands are constant */
        if (!_AreAllOperandsConstant(PolynaryExpr)) break;

        /* Collect all operand constants */
        i = 0;
        if(PolynaryExpr->operands) {
           FOR_EACH_DLINK_NODE(&PolynaryExpr->operands->members, struct _cloIR_EXPR, operand) {
            gcmASSERT(cloIR_OBJECT_GetType(&operand->base) == clvIR_CONSTANT);
            operandConstants[i] = (cloIR_CONSTANT)operand;

            i++;
           }
        }

        gcmASSERT(i <= clmMAX_BUILT_IN_PARAMETER_COUNT);

        /* Try to evaluate the built-in */
        status = clEvaluateBuiltinFunction(Compiler,
                           PolynaryExpr,
                           i,
                           operandConstants,
                           ResultConstant);
        if (gcmIS_ERROR(status)) return status;

        if (*ResultConstant != gcvNULL) {
            gcmVERIFY_OK(cloIR_OBJECT_Destroy(Compiler, &PolynaryExpr->exprBase.base));
        }

        return gcvSTATUS_OK;
    } while (gcvFALSE);

    *ResultConstant = gcvNULL;
    return gcvSTATUS_OK;
}

gceSTATUS
cloIR_POLYNARY_EXPR_Evaluate(
IN cloCOMPILER Compiler,
IN cloIR_POLYNARY_EXPR PolynaryExpr,
OUT cloIR_CONSTANT * ResultConstant
)
{
    gceSTATUS    status;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(PolynaryExpr, clvIR_POLYNARY_EXPR);
    gcmASSERT(ResultConstant);

    switch (PolynaryExpr->type) {
    case clvPOLYNARY_CONSTRUCT_SCALAR:
        status = cloIR_POLYNARY_EXPR_ConstructScalarConstant(Compiler,
                                     PolynaryExpr,
                                     ResultConstant);
        if (gcmIS_ERROR(status)) return status;
        break;

    case clvPOLYNARY_CONSTRUCT_ARRAY:
        status = cloIR_POLYNARY_EXPR_ConstructArrayConstant(Compiler,
                                     PolynaryExpr,
                                     ResultConstant);
        if (gcmIS_ERROR(status)) return status;
        break;

    case clvPOLYNARY_CONSTRUCT_VECTOR:
        status = cloIR_POLYNARY_EXPR_ConstructVectorOrMatrixConstant(Compiler,
                                         PolynaryExpr,
                                         gcvTRUE,
                                         ResultConstant);
        if (gcmIS_ERROR(status)) return status;
        break;

    case clvPOLYNARY_CONSTRUCT_MATRIX:
        status = cloIR_POLYNARY_EXPR_ConstructVectorOrMatrixConstant(Compiler,
                                         PolynaryExpr,
                                         gcvFALSE,
                                         ResultConstant);
        if (gcmIS_ERROR(status)) return status;
        break;

    case clvPOLYNARY_CONSTRUCT_STRUCT:
        status = cloIR_POLYNARY_EXPR_ConstructStructConstant(Compiler,
                                     PolynaryExpr,
                                     ResultConstant);
        if (gcmIS_ERROR(status)) return status;
        break;

    case clvPOLYNARY_FUNC_CALL:
        gcmASSERT(PolynaryExpr->funcName);

        if (PolynaryExpr->funcName->isBuiltin) {
            status = cloIR_POLYNARY_EXPR_EvaluateBuiltin(Compiler,
                                     PolynaryExpr,
                                     ResultConstant);
            if (gcmIS_ERROR(status)) return status;
        }
        else {
            *ResultConstant = gcvNULL;
        }
        break;
    case clvPOLYNARY_BUILT_IN_ASM_CALL:
        *ResultConstant = gcvNULL;
        break;
    default:
        gcmASSERT(0);
    }
    return gcvSTATUS_OK;
}
