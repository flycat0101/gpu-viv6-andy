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


#include "gc_cl_gen_code.h"
#include "gc_cl_emit_code.h"

#define _cldIncludeComponentOffset 1
#define _cldHandleULongStore 0

#define _GEN_FOR_BETTER_LOOP_RECOGNITION  1

#define clmIsPointerOperand(Operand)  \
   (clmDECL_IsPointerType(&(Operand)->decl) || \
    (clmDECL_IsArray(&(Operand)->decl) && \
     (Operand)->decl.dataType->addrSpaceQualifier == clvQUALIFIER_LOCAL))

#define _clmInitializeSuperEnable(superEnable) \
  do { \
     for(i = 0; i < cldMaxFourComponentCount; i++) { \
        (superEnable)[i]= 0; \
     } \
  } while (gcvFALSE)

#define _clmInitializeSuperSwizzle(superSwizzle) \
  do { \
     for(i = 0; i < cldMaxComponentCount; i++) { \
        (superSwizzle)[i]= 0; \
     } \
  } while (gcvFALSE)

#define _clmGetComponentSectionIndex(component) \
    ((gctREG_INDEX) ((component) >> 2))

#define _clmGetSectionalComponent(component) ((component) & 0x3)

#define _clmConvROperandToSuperSourceConstant(Compiler, ROperand, SuperSource, Status) \
   do { \
    Status = _ConvROperandToSourceConstant(Compiler, ROperand, (SuperSource)->sources); \
    (SuperSource)->numSources = 1; \
   } while (gcvFALSE)

#define _clmConvROperandToSpecialVectorSuperSourceConstant(Compiler, ROperand, SuperSource, Status) \
   do { \
    Status =_ConvROperandToSpecialVectorSourceConstant(Compiler, ROperand, (SuperSource)->sources); \
    (SuperSource)->numSources = 1; \
   } while (gcvFALSE)

#define _clmConvLOperandToSuperTarget(Compiler, LOperand, SuperTarget, ComponentSelection, Status) \
   do { \
     Status = _ConvLOperandToTarget(Compiler, LOperand, (SuperTarget)->targets, ComponentSelection); \
     (SuperTarget)->numTargets = 1; \
   } while (gcvFALSE)

#define _clmConvNormalROperandToSuperSource(Compiler, LineNo, StringNo, ROperand, SuperSource, Status) \
   do { \
      Status = _ConvNormalROperandToSource(Compiler, LineNo, StringNo, ROperand, (SuperSource)->sources); \
      (SuperSource)->numSources = 1; \
   } while (gcvFALSE)

#define _clmConvLOperandToSuperSourceReg(Compiler, LOperand, ComponentSelection, SuperSource, Status) \
   do { \
      Status = _ConvLOperandToSourceReg(Compiler, \
                                        LOperand, \
                                        clGetDefaultComponentSelection(LOperand->dataType), \
                                        (SuperSource)->sources); \
      (SuperSource)->numSources = 1; \
   } while (gcvFALSE)

#define _clmConvIOperandToVectorComponentSuperTarget(Compiler, IOperand, VectorIndex, SuperTarget, Status) \
   do { \
      Status = _ConvIOperandToVectorComponentSuperTarget(Compiler, \
                                                 IOperand, \
                                 VectorIndex, \
                             (SuperTarget)->targets); \
      (SuperTarget)->numTargets = 1; \
   } while (gcvFALSE)

#define _clmIsROperandConstantZero(ROperand) \
   (!((ROperand)->isReg) ? ((ROperand)->u.constant.values[0].intValue == 0) : gcvFALSE)

#define _clmAssignROperandToPreallocatedTempReg(Compiler, Parameters, ROperand, Status)  \
   do { \
    clsLOPERAND lOperand[1]; \
    clsIOPERAND iOperand[1]; \
    gcmASSERT((Parameters)->hasIOperand); \
        clmGEN_CODE_GetParametersIOperand((Compiler), iOperand, (Parameters), (Parameters)->dataTypes[0].def); \
    clsLOPERAND_InitializeUsingIOperand(lOperand, iOperand); \
    (Status) = clGenAssignCode((Compiler), 0, 0, lOperand, (ROperand)); \
   } while (gcvFALSE)

#define _clmGenStructAssign(Compiler, Expr, Lhs, LOffset, RightParameters, ElementSize, Status) \
   do { \
      if((RightParameters)->hint & (clvGEN_DEREF_CODE | clvGEN_DEREF_STRUCT_CODE)) { \
         (Status) = _GenAssignBytes(Compiler, \
                                    (Expr)->base.lineNo, \
                                    (Expr)->base.stringNo, \
                                    (Lhs), \
                                    (LOffset), \
                                    (RightParameters)->rOperands, \
                                    (RightParameters)->dataTypes[0].byteOffset, \
                                    (ElementSize)); \
     } \
     else { \
         gctINT curOperand = -1; \
         gctINT size; \
         size = _AssignStructOrUnionInMemory(Compiler, \
                                             (Expr)->base.lineNo, \
                                             (Expr)->base.stringNo, \
                                             &((Expr)->decl), \
                                             (Lhs), \
                                             (LOffset), \
                                             (RightParameters), \
                                             &curOperand, \
                                             gcvTRUE); \
         if(size < 0) { \
            (Status) = gcvSTATUS_INVALID_DATA; \
         } \
    } \
  } while (gcvFALSE)

#define clmNewUniform(Compiler, Name, Symbol, Decl, Offset,\
          Array, VarCategory, NumStructElement, Parent, PrevSibling, \
          Uniform, ThisIndex, Status)  \
  do { \
    gctSIZE_T length = 1; \
    gcSHADER binary; \
    gcUNIFORM siblingUniform; \
    clsGEN_CODE_DATA_TYPE format; \
    clsGEN_CODE_DATA_TYPE binaryDataType; \
    gctBOOL isPointer; \
    clmGEN_CODE_ConvDirectElementDataType((Decl)->dataType, format); \
    if(Array) { \
        clmGetArrayElementCount(*(Array), 0, length); \
    } \
    if((VarCategory) == gcSHADER_VAR_CATEGORY_BLOCK_MEMBER) { \
        binaryDataType = _ConvElementDataType(Decl); \
        isPointer = clmDECL_IsPointerType(Decl); \
    } \
    else { \
        binaryDataType = _ConvElementDataTypeForRegAlloc(Name); \
        isPointer = clmDECL_IsPointerType(Decl) || clmGEN_CODE_checkVariableForMemory(Name); \
    } \
    (Status) = clNewUniform((Compiler), \
                            (Name)->lineNo, \
                            (Name)->stringNo, \
                            (Symbol), \
                            binaryDataType, \
                            format, \
                            (Name)->u.variableInfo.variableType, \
                            isPointer, \
                            length, \
                            &(Uniform)); \
    if (gcmIS_ERROR(Status)) break; \
    SetUniformCategory((Uniform), (VarCategory)); \
    SetUniformOffset((Uniform), (Offset)); \
    SetUniformNumStructureElement((Uniform), (NumStructElement)); \
    SetUniformParent((Uniform), (Parent)); \
    SetUniformPrevSibling((Uniform), (PrevSibling)); \
    *(ThisIndex) = GetUniformIndex(Uniform); \
    gcmVERIFY_OK(cloCOMPILER_GetBinary(Compiler, &binary)); \
    if((PrevSibling) != -1) { \
       clmGetUniform(binary, (PrevSibling), siblingUniform, (Status)); \
       siblingUniform->nextSibling = *(ThisIndex); \
    } \
  } while (gcvFALSE)

#define clmGetUniform(Shader, Index, Uniform, Status) \
    do { \
        (status) = gcSHADER_GetUniform((Shader), \
                                       (Index) - GetShaderMaxKernelFunctionArgs(Shader), \
                                       &(Uniform)); \
    } while (gcvFALSE)

static void
_InitializeROperandConstant(
IN clsROPERAND *Constant,
IN clsGEN_CODE_DATA_TYPE DataType,
IN gctINT Value
);

static gceSTATUS
_GenAssignBytes(
IN cloCOMPILER Compiler,
IN gctINT LineNo,
IN gctINT StringNo,
IN clsLOPERAND *LOperand,
IN gctINT LOffset,
IN clsROPERAND *ROperand,
IN gctINT ROffset,
IN gctSIZE_T NumBytes
);

static gceSTATUS
_ConvNormalROperandToSource(
    IN cloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN clsROPERAND * ROperand,
    OUT gcsSOURCE * Source
    );

static cltELEMENT_TYPE
_ConvPackedTypeToBasicType(
    cltELEMENT_TYPE PackedType
    )
{
    cltELEMENT_TYPE basicType = clvTYPE_CHAR;

    switch(PackedType) {
    case clvTYPE_BOOL_PACKED:
        basicType = clvTYPE_BOOL;
        break;

    case clvTYPE_CHAR_PACKED:
        basicType = clvTYPE_CHAR;
        break;

    case clvTYPE_UCHAR_PACKED:
        basicType = clvTYPE_UCHAR;
        break;

    case clvTYPE_SHORT_PACKED:
        basicType = clvTYPE_SHORT;
        break;

    case clvTYPE_USHORT_PACKED:
        basicType = clvTYPE_USHORT;
        break;

    case clvTYPE_HALF_PACKED:
        basicType = clvTYPE_HALF;
        break;

    default:
        gcmASSERT(0);
        break;
    }
    return basicType;
}

clsGEN_CODE_DATA_TYPE
clGetSubsetDataType(
IN clsGEN_CODE_DATA_TYPE Base,
IN gctUINT8 NumComponents
)
{
   clsGEN_CODE_DATA_TYPE result;

   result = Base;
   if(NumComponents == 1) {
      if(clmIsElementTypePacked(result.elementType)) {
          result.elementType = _ConvPackedTypeToBasicType(result.elementType);
      }
      clmGEN_CODE_scalarDataType_SET(result);
   }
   else {
      clmGEN_CODE_vectorSize_SET(result, NumComponents);
   }
   return result;
}

gceSTATUS
cloIR_POLYNARY_EXPR_GenOperandsCodeForFuncCall(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    OUT gctUINT * OperandCount,
    OUT clsGEN_CODE_PARAMETERS * * OperandsParameters
    );

static gceSTATUS
_ConvDerefPackedPointerExprToFuncCall(
IN cloCOMPILER Compiler,
IN cloIR_EXPR Expr,
OUT cloIR_POLYNARY_EXPR *FuncCall
);

#define _clmGetSubsetDataType(BaseType, NumComponents, ResultType)  do { \
    (ResultType) = clGetSubsetDataType((BaseType), (NumComponents));  \
  } while(gcvFALSE)

static gctSIZE_T
_ElementTypeByteSize(
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
      case clvTYPE_IMAGE2D_T:
      case clvTYPE_IMAGE3D_T:
         size = 4;
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

      return size;
}

/* Machine targeted elment type byte size - maximum 4 bytes */
static gctSIZE_T
_TargetElementTypeByteSize(
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
      case clvTYPE_IMAGE2D_T:
      case clvTYPE_IMAGE3D_T:
         size = 4;
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

      return size;
}

static gctSIZE_T
_DataTypeByteSize(
clsGEN_CODE_DATA_TYPE DataType
)
{
      gctSIZE_T size = _ElementTypeByteSize(DataType.elementType);

      if (clmGEN_CODE_vectorSize_GET(DataType) > 0) {
          if (clmGEN_CODE_vectorSize_NOCHECK_GET(DataType) == 3) {
              /* 3-component vector data type must be aligned to a 4*sizeof(component) */
              size*=4;
          } else {
              size *= clmGEN_CODE_vectorSize_NOCHECK_GET(DataType);
          }
      }
      else if (clmGEN_CODE_matrixColumnCount_GET(DataType) > 0) {
         size *= clmGEN_CODE_matrixRowCount_GET(DataType) * clmGEN_CODE_matrixColumnCount_GET(DataType);
      }

      return size;
}

static gctSIZE_T
_SectionalDataTypeByteSize(
clsGEN_CODE_DATA_TYPE DataType
)
{
    gctSIZE_T size = _ElementTypeByteSize(DataType.elementType);

    if (clmGEN_CODE_vectorSize_GET(DataType) > 0) {
        switch(clmGEN_CODE_vectorSize_NOCHECK_GET(DataType)) {
        case 2:
            size <<= 1;
            break;

        default:
            if(clmGEN_CODE_IsHighPrecisionDataType(DataType)) {
                size = 16;
            }
            else {
                size <<= 2;
            }
            break;
        }
    }

    return size;
}

static gctUINT8 _EnableOrder[4] = {gcSL_ENABLE_X, gcSL_ENABLE_Y, gcSL_ENABLE_Z, gcSL_ENABLE_W};

static gctUINT8
_GetEnableComponentCount(
IN gctUINT8 Enable
)
{
   gctUINT8 count = 0;
   gcmASSERT(Enable);

   do {
     if(Enable & 0x1) count++;
     Enable >>= 1;
   } while(Enable);

   gcmASSERT(count <= 4);
   return count;
}

static gctBOOL
_IsComponentSelectionSameSubVector(
IN clsCOMPONENT_SELECTION *ComponentSelection
)
{
   gctUINT8 i;
   gctUINT startComponent;

   startComponent = (ComponentSelection->selection[0] >> 2) << 2;
   for(i = 1; i < ComponentSelection->components; i++) {
     if(ComponentSelection->selection[i] < startComponent ||
        ComponentSelection->selection[i] > (startComponent + 3)) {
        return gcvFALSE;
     }
   }
   return gcvTRUE;
}

static gceSTATUS
_IsCommonExprObject(
IN cloIR_BASE ExprBase
)
{
   switch(cloIR_OBJECT_GetType(ExprBase)) {
   case clvIR_VARIABLE:
   case clvIR_CONSTANT:
   case clvIR_UNARY_EXPR:
   case clvIR_BINARY_EXPR:
   case clvIR_POLYNARY_EXPR:
      return gcvTRUE;

   case clvIR_SELECTION:
      {
        cloIR_SELECTION selection;

        selection = (cloIR_SELECTION) ExprBase;
    return (selection->trueOperand && _IsCommonExprObject(selection->trueOperand) &&
            selection->falseOperand &&  _IsCommonExprObject(selection->falseOperand));
      }

   default:
      return gcvFALSE;
   }
}

static cloIR_POLYNARY_EXPR
_CreateFuncCall(
IN cloCOMPILER Compiler,
cleOPCODE Opcode,
IN cloIR_EXPR Expr
)
{
   gceSTATUS status;
   cloIR_POLYNARY_EXPR    newFuncCall = gcvNULL;
   cltPOOL_STRING symbolInPool;

   status = cloCOMPILER_FindPoolString(Compiler,
                                       clGetOpcodeName(Opcode),
                                       &symbolInPool);
   if (gcmIS_ERROR(status)) return gcvNULL;

   /* Create a new function call expression */
   status = cloIR_POLYNARY_EXPR_Construct(Compiler,
                                          Expr->base.lineNo,
                                          Expr->base.stringNo,
                                          clvPOLYNARY_FUNC_CALL,
                                          &Expr->decl,
                                          symbolInPool,
                                          &newFuncCall);
   if (gcmIS_ERROR(status)) return gcvNULL;

   status = cloIR_SET_Construct(Compiler,
                                Expr->base.lineNo,
                                Expr->base.stringNo,
                                clvEXPR_SET,
                                &newFuncCall->operands);
   if (gcmIS_ERROR(status)) return gcvNULL;
   return newFuncCall;
}

static cloIR_POLYNARY_EXPR
_CreateFuncCallByName(
IN cloCOMPILER Compiler,
IN gctUINT LineNo,
IN gctUINT StringNo,
IN gctCONST_STRING Name,
IN cloIR_EXPR Expr
)
{
   gceSTATUS status;
   cloIR_POLYNARY_EXPR    newFuncCall = gcvNULL;
   cltPOOL_STRING symbolInPool;
   clsDECL declBuf[1];
   clsDECL *declPtr;

   status = cloCOMPILER_FindPoolString(Compiler,
                                       Name,
                                       &symbolInPool);
   if (gcmIS_ERROR(status)) return gcvNULL;

   if(Expr) {
      declPtr = &Expr->decl;
   }
   else { /* void function */
      clsDATA_TYPE *dataType;
      status = cloCOMPILER_CreateDataType(Compiler,
                                          T_VOID,
                                          gcvNULL,
                                          clvQUALIFIER_NONE,
                                          clvQUALIFIER_NONE,
                                          &dataType);
      if (gcmIS_ERROR(status)) return gcvNULL;
      declPtr = declBuf;
      clmDECL_Initialize(declPtr, dataType, (clsARRAY *)0, gcvNULL, gcvFALSE, clvSTORAGE_QUALIFIER_NONE);
   }

   /* Create a new function call expression */
   status = cloIR_POLYNARY_EXPR_Construct(Compiler,
                                          LineNo,
                                          StringNo,
                                          clvPOLYNARY_FUNC_CALL,
                                          declPtr,
                                          symbolInPool,
                                          &newFuncCall);
   if (gcmIS_ERROR(status)) return gcvNULL;

   status = cloIR_SET_Construct(Compiler,
                                LineNo,
                                StringNo,
                                clvEXPR_SET,
                                &newFuncCall->operands);
   if (gcmIS_ERROR(status)) return gcvNULL;
   return newFuncCall;
}

static gceSTATUS
_ConvExprToFuncCall(
IN cloCOMPILER Compiler,
cleOPCODE Opcode,
IN cloIR_EXPR Expr,
OUT cloIR_POLYNARY_EXPR *FuncCall
)
{
   gceSTATUS status = gcvSTATUS_OK;
   cloIR_POLYNARY_EXPR    newFuncCall;
   cloIR_UNARY_EXPR unaryExpr;
   cloIR_BINARY_EXPR binaryExpr;
   cloIR_SELECTION selectionExpr;

   gcmASSERT(FuncCall);
   *FuncCall = gcvNULL;

   gcmVERIFY_OK(cloCOMPILER_Lock(Compiler));
   newFuncCall = _CreateFuncCall(Compiler,
                                 Opcode,
                                 Expr);
   if(!newFuncCall) {
     status = gcvSTATUS_INVALID_ARGUMENT;
     gcmONERROR(status);
   }

   gcmASSERT(newFuncCall->operands);

   switch(cloIR_OBJECT_GetType(&Expr->base)) {
   case clvIR_UNARY_EXPR:
      unaryExpr = (cloIR_UNARY_EXPR) &Expr->base;
      gcmONERROR(cloIR_SET_AddMember(Compiler,
                                     newFuncCall->operands,
                                     &unaryExpr->operand->base));
      break;

   case clvIR_BINARY_EXPR:
      binaryExpr = (cloIR_BINARY_EXPR) &Expr->base;
      gcmONERROR(cloIR_SET_AddMember(Compiler,
                                     newFuncCall->operands,
                                     &binaryExpr->leftOperand->base));
      gcmONERROR(cloIR_SET_AddMember(Compiler,
                                     newFuncCall->operands,
                                     &binaryExpr->rightOperand->base));
      break;

   case clvIR_SELECTION:
      selectionExpr = (cloIR_SELECTION) &Expr->base;
      gcmONERROR(cloIR_SET_AddMember(Compiler,
                                     newFuncCall->operands,
                                     selectionExpr->falseOperand));
      gcmONERROR(cloIR_SET_AddMember(Compiler,
                                     newFuncCall->operands,
                                     selectionExpr->trueOperand));
      gcmONERROR(cloIR_SET_AddMember(Compiler,
                                     newFuncCall->operands,
                                     &selectionExpr->condExpr->base));
      break;

   default:
      gcmASSERT(0);
      status = gcvSTATUS_INVALID_ARGUMENT;
      gcmONERROR(status);
   }
   gcmONERROR(cloCOMPILER_BindBuiltinFuncCall(Compiler,
                                              newFuncCall));
   *FuncCall = newFuncCall;

OnError:
   gcmVERIFY_OK(cloCOMPILER_Unlock(Compiler));
   return status;
}

void
clsOPERAND_CONSTANT_ChangeFloatFamilyDataType(
IN clsGEN_CODE_DATA_TYPE NewDataType,
IN OUT clsOPERAND_CONSTANT * OperandConstant
)
{
    cltELEMENT_TYPE sourceType;
    cltELEMENT_TYPE elementType;

    gcmASSERT(OperandConstant);

    sourceType = clmGEN_CODE_elementType_GET(OperandConstant->dataType);
    elementType = clmGEN_CODE_elementType_GET(NewDataType);
    clParseConvertConstantValues(OperandConstant->valueCount,
                                 sourceType,
                                 OperandConstant->values,
                                 elementType,
                                 OperandConstant->values);

    clmGEN_CODE_elementType_SET(OperandConstant->dataType, elementType);
}

void
clsOPERAND_CONSTANT_ChangeIntegerFamilyDataType(
IN clsGEN_CODE_DATA_TYPE NewDataType,
IN OUT clsOPERAND_CONSTANT * OperandConstant
)
{
   cltELEMENT_TYPE sourceType;
   cltELEMENT_TYPE elementType;

   gcmASSERT(OperandConstant);

   sourceType = clmGEN_CODE_elementType_GET(OperandConstant->dataType);
   elementType = clmGEN_CODE_elementType_GET(NewDataType);
   clParseConvertConstantValues(OperandConstant->valueCount,
                                sourceType,
                                OperandConstant->values,
                                elementType,
                                OperandConstant->values);

   clmGEN_CODE_elementType_SET(OperandConstant->dataType, elementType);
}

void
clsOPERAND_CONSTANT_ChangeUnsignedIntegerFamilyDataType(
IN clsGEN_CODE_DATA_TYPE NewDataType,
IN OUT clsOPERAND_CONSTANT * OperandConstant
)
{
    cltELEMENT_TYPE sourceType;
    cltELEMENT_TYPE elementType;

    gcmASSERT(OperandConstant);

    sourceType = clmGEN_CODE_elementType_GET(OperandConstant->dataType);
    elementType = clmGEN_CODE_elementType_GET(NewDataType);
    clParseConvertConstantValues(OperandConstant->valueCount,
                                 sourceType,
                                 OperandConstant->values,
                                 elementType,
                                 OperandConstant->values);

    clmGEN_CODE_elementType_SET(OperandConstant->dataType, elementType);
}

void
clsOPERAND_CONSTANT_ChangeLongFamilyDataType(
IN clsGEN_CODE_DATA_TYPE NewDataType,
IN OUT clsOPERAND_CONSTANT * OperandConstant
)
{
   cltELEMENT_TYPE sourceType;
   cltELEMENT_TYPE elementType;

   gcmASSERT(OperandConstant);

   sourceType = clmGEN_CODE_elementType_GET(OperandConstant->dataType);
   elementType = clmGEN_CODE_elementType_GET(NewDataType);
   clParseConvertConstantValues(OperandConstant->valueCount,
                                sourceType,
                                OperandConstant->values,
                                elementType,
                                OperandConstant->values);

   clmGEN_CODE_elementType_SET(OperandConstant->dataType, elementType);
}

void
clsOPERAND_CONSTANT_ChangeUlongFamilyDataType(
IN clsGEN_CODE_DATA_TYPE NewDataType,
IN OUT clsOPERAND_CONSTANT * OperandConstant
)
{
    gctUINT    i;
    cltELEMENT_TYPE elementType;

    gcmASSERT(OperandConstant);

    elementType = clmGEN_CODE_elementType_GET(NewDataType);
    if(clmIsElementTypeFloating(elementType)) {
        for (i = 0; i < OperandConstant->valueCount; i++) {
            OperandConstant->values[i].floatValue = clmUL2F(OperandConstant->values[i].ulongValue);
        }
    }
    else if(clmIsElementTypeBoolean(elementType)) {
        for (i = 0; i < OperandConstant->valueCount; i++) {
            OperandConstant->values[i].boolValue = clmUL2B(OperandConstant->values[i].ulongValue);
        }
    }
    else if (clmIsElementTypeHighPrecision(elementType)) {
        if(clmIsElementTypeSigned(elementType)) {
            for (i = 0; i < OperandConstant->valueCount; i++) {
                OperandConstant->values[i].longValue = clmUL2L(OperandConstant->values[i].ulongValue);
            }
        }
        else {
            gcmASSERT(clmIsElementTypeUnsigned(elementType));
        }
    }
    else if(clmIsElementTypeSigned(elementType)) {
        for (i = 0; i < OperandConstant->valueCount; i++) {
            OperandConstant->values[i].intValue = clmUL2I(OperandConstant->values[i].ulongValue);
        }
    }
    else {
        gcmASSERT(clmIsElementTypeUnsigned(elementType)); /* has to be unsigned integer */
        for (i = 0; i < OperandConstant->valueCount; i++) {
            OperandConstant->values[i].uintValue = clmUL2U(OperandConstant->values[i].ulongValue);
        }
    }

    clmGEN_CODE_elementType_SET(OperandConstant->dataType, elementType);
}

void
clsOPERAND_CONSTANT_ChangeBooleanFamilyDataType(
IN clsGEN_CODE_DATA_TYPE NewDataType,
IN OUT clsOPERAND_CONSTANT * OperandConstant
)
{
    gctUINT    i;
    cltELEMENT_TYPE elementType;

    gcmASSERT(OperandConstant);

        elementType = clmGEN_CODE_elementType_GET(NewDataType);
    if(clmIsElementTypeFloating(elementType)) {
       for (i = 0; i < OperandConstant->valueCount; i++) {
            OperandConstant->values[i].floatValue = clmB2F(OperandConstant->values[i].boolValue);
       }
    }
    else if(clmIsElementTypeInteger(elementType)) {
       if(clmIsElementTypeUnsigned(elementType)) {
           for (i = 0; i < OperandConstant->valueCount; i++) {
               OperandConstant->values[i].ulongValue = clmB2U(OperandConstant->values[i].boolValue);
           }
       }
       else {
           for (i = 0; i < OperandConstant->valueCount; i++) {
               OperandConstant->values[i].longValue = clmB2I(OperandConstant->values[i].boolValue);
           }
       }
    }
    gcmASSERT(clmIsElementTypeBoolean(elementType));  /* error if not boolean type */

    clmGEN_CODE_elementType_SET(OperandConstant->dataType, elementType);
}

gceSTATUS
clsROPERAND_ChangeDataTypeFamily(
IN cloCOMPILER Compiler,
IN gctUINT LineNo,
IN gctUINT StringNo,
IN gctBOOL TreatFloatAsInt,
IN clsGEN_CODE_DATA_TYPE NewDataType,
IN OUT clsROPERAND * ROperand
)
{
    gceSTATUS  status;
    cleOPCODE  opcode;
    clsIOPERAND intermIOperand[1];

    gcmASSERT(ROperand);

    if (ROperand->isReg) {
       cltELEMENT_TYPE elementType;
       cltELEMENT_TYPE newElementType;

       opcode = clvOPCODE_INVALID;

       elementType = clmGEN_CODE_elementType_GET(ROperand->dataType);
       newElementType = clmGEN_CODE_elementType_GET(NewDataType);
       if(clmIsElementTypePacked(newElementType)) {
           newElementType = _ConvPackedTypeToBasicType(newElementType);
       }

       if(newElementType != elementType) {
          cloCODE_GENERATOR codeGenerator;

          codeGenerator = cloCOMPILER_GetCodeGenerator(Compiler);
          if(clmIsElementTypeFloating(newElementType)) {
             if(clmIsElementTypeBoolean(elementType)) {
                if(codeGenerator->fpConfig & cldFpROUND_TO_NEAREST) {
                   opcode = clvOPCODE_IMPL_B2F;
                }
                else {
                   opcode = clvOPCODE_BOOL_TO_FLOAT;
                }
             }
             else if(clmIsElementTypeInteger(elementType)) {
                if(clmIsElementTypeUnsigned(elementType)) {
                   if(codeGenerator->fpConfig & cldFpROUND_TO_NEAREST) {
                      opcode = clvOPCODE_IMPL_U2F;
                   }
                   else {
                      opcode = clvOPCODE_UINT_TO_FLOAT;
                   }
                }
                else {
                   if(codeGenerator->fpConfig & cldFpROUND_TO_NEAREST) {
                      opcode = clvOPCODE_IMPL_I2F;
                   }
                   else {
                      opcode = clvOPCODE_INT_TO_FLOAT;
                   }
                }
             }
          }
          else if(clmIsElementTypeBoolean(newElementType)) {
             if(clmIsElementTypeFloating(elementType)) {
                opcode = clvOPCODE_FLOAT_TO_BOOL;
             }
             else if(clmIsElementTypeInteger(elementType)) {
                if(clmIsElementTypeUnsigned(elementType)) {
                   opcode = clvOPCODE_UINT_TO_BOOL;
                }
                else {
                   opcode = clvOPCODE_INT_TO_BOOL;
                }
             }
          }
          else if(clmIsElementTypeInteger(newElementType)) {
             if(clmIsElementTypeUnsigned(newElementType)) {
                if(clmIsElementTypeBoolean(elementType)) {
                   opcode = clvOPCODE_BOOL_TO_UINT;
                }
                else if(clmIsElementTypeFloating(elementType)) {
                   opcode = clvOPCODE_FLOAT_TO_UINT;
                }
                else if(clmIsElementTypeUnsigned(elementType)) {
                   opcode = clvOPCODE_CONV;
                }
                else opcode = clvOPCODE_CONV;
             }
             else {
                if(clmIsElementTypeBoolean(elementType)) {
                   opcode = clvOPCODE_BOOL_TO_INT;
                }
                else if(clmIsElementTypeFloating(elementType)) {
                   if (!TreatFloatAsInt) opcode = clvOPCODE_FLOAT_TO_INT;
                }
                else if(clmIsElementTypeUnsigned(elementType)) {
                   opcode = clvOPCODE_CONV;
                }
                else opcode = clvOPCODE_CONV;
             }
          }
       }

       if (opcode != clvOPCODE_INVALID) {
           clsGEN_CODE_DATA_TYPE newDataType;

           gcmASSERT(clmGEN_CODE_IsScalarDataType(ROperand->dataType));
           if(gcIsScalarDataType(ROperand->dataType)) {
                newDataType = clGetSubsetDataType(NewDataType, 1);
           }
           else newDataType = NewDataType;
           clsIOPERAND_New(Compiler, intermIOperand, newDataType);

           status = clGenGenericCode1(Compiler,
                                      LineNo,
                                      StringNo,
                                      opcode,
                                      intermIOperand,
                                      ROperand);
           if (gcmIS_ERROR(status)) return status;

           clsROPERAND_InitializeUsingIOperand(ROperand, intermIOperand);
       }

       if(gcIsScalarDataType(ROperand->dataType) && gcIsVectorDataType(NewDataType)) {
           clsLOPERAND lOperand[1];

           clsIOPERAND_New(Compiler, intermIOperand, NewDataType);
           clsLOPERAND_InitializeUsingIOperand(lOperand, intermIOperand);
           status = clGenAssignCode(Compiler,
                                    LineNo,
                                    StringNo,
                                    lOperand,
                                    ROperand);
          if (gcmIS_ERROR(status)) return status;

          clsROPERAND_InitializeUsingIOperand(ROperand, intermIOperand);
       }
    }
    else {
        cltELEMENT_TYPE elementType;
        elementType = clmGEN_CODE_elementType_GET(ROperand->dataType);

        if(clmIsElementTypeBoolean(elementType)) {
            clsOPERAND_CONSTANT_ChangeBooleanFamilyDataType(NewDataType, &ROperand->u.constant);
        }
        else if(clmIsElementTypeFloating(elementType)) {
            clsOPERAND_CONSTANT_ChangeFloatFamilyDataType(NewDataType, &ROperand->u.constant);
        }
        else if (clmIsElementTypeHighPrecision(elementType)) {
            if(clmIsElementTypeUnsigned(elementType)) {
                clsOPERAND_CONSTANT_ChangeUlongFamilyDataType(NewDataType, &ROperand->u.constant);
            }
            else {
                clsOPERAND_CONSTANT_ChangeLongFamilyDataType(NewDataType, &ROperand->u.constant);
            }
        }
        else if(clmIsElementTypeUnsigned(elementType)) {
            clsOPERAND_CONSTANT_ChangeUnsignedIntegerFamilyDataType(NewDataType, &ROperand->u.constant);
        }
        else if(clmIsElementTypeSigned(elementType)) {
            clsOPERAND_CONSTANT_ChangeIntegerFamilyDataType(NewDataType, &ROperand->u.constant);
        }
        else {
            gcmASSERT(0);
        }
    }
    ROperand->dataType = NewDataType;

    return gcvSTATUS_OK;
}

static gceSTATUS
_ImplicitConvertOperand(
IN cloCOMPILER Compiler,
IN gctUINT LineNo,
IN gctUINT StringNo,
IN clsGEN_CODE_DATA_TYPE NewDataType,
IN OUT clsROPERAND *Operand
);

#define clmGEN_CODE_ConvDirectElementDataType(DataType, ResType) \
   clmGEN_CODE_DATA_TYPE_Initialize(ResType, \
                                    clmDATA_TYPE_matrixRowCount_GET(DataType), \
                                    clmDATA_TYPE_matrixColumnCount_GET(DataType), \
                                    (DataType)->elementType);

gceSTATUS
clGenCheckAndImplicitConvertOperand(
IN cloCOMPILER Compiler,
IN gctUINT LineNo,
IN gctUINT StringNo,
IN clsDECL *TargetDecl,
IN OUT clsROPERAND *Operand
)
{
   gceSTATUS status;
   clsGEN_CODE_DATA_TYPE resultType;
   cltELEMENT_TYPE fromElementType, toElementType;

   if(clmDECL_IsPointerType(TargetDecl)) return gcvSTATUS_OK;

   clmGEN_CODE_ConvDirectElementDataType(TargetDecl->dataType, resultType);
   toElementType = clmGEN_CODE_elementType_GET(resultType);
   fromElementType = clmGEN_CODE_elementType_GET(Operand->dataType);

   if(((fromElementType == toElementType) &&
        clmGEN_CODE_IsScalarDataType(Operand->dataType) &&
        clmGEN_CODE_IsVectorDataType(resultType)) ||
      (fromElementType != toElementType &&
       clmGEN_CODE_IsScalarDataType(Operand->dataType))) {
      status = clsROPERAND_ChangeDataTypeFamily(Compiler,
                                                LineNo,
                                                StringNo,
                                                gcvFALSE,
                                                resultType,
                                                Operand);
      if(gcmIS_ERROR(status)) return status;
   }
   return gcvSTATUS_OK;
}

static gceSTATUS
_ConvScalarToVector(
IN cloCOMPILER Compiler,
IN gctUINT8 VectorSize,
IN OUT cloIR_EXPR ScalarExpr,
IN OUT clsGEN_CODE_PARAMETERS * Parameters
)
{
   gceSTATUS status;
   clsDATA_TYPE *vectorDataType;
   clsGEN_CODE_DATA_TYPE dataType;

   status = cloIR_CreateVectorType(Compiler,
                                   ScalarExpr->decl.dataType,
                                   VectorSize,
                                   &vectorDataType);
   if(gcmIS_ERROR(status)) return status;

   clmGEN_CODE_ConvDirectElementDataType(vectorDataType, dataType);
   status = _ImplicitConvertOperand(Compiler,
                                    ScalarExpr->base.lineNo,
                                    ScalarExpr->base.stringNo,
                                    dataType,
                                    Parameters->rOperands);
   if(gcmIS_ERROR(status)) return status;

   Parameters->dataTypes[0].def = dataType;
   ScalarExpr->decl.dataType = vectorDataType;
   return gcvSTATUS_OK;
}

void
clsROPERAND_CONSTANT_ConvScalarToVector(
    IN cloCOMPILER Compiler,
    IN clsGEN_CODE_DATA_TYPE NewDataType,
    IN OUT clsROPERAND * ROperand
    )
{
    gctUINT        i;
    gctUINT8 vectorComponentCount;
    cltELEMENT_TYPE elementType;

    gcmASSERT(ROperand);
    gcmASSERT(gcIsScalarDataType(ROperand->dataType));
    gcmASSERT(!ROperand->isReg);

    vectorComponentCount = gcGetDataTypeComponentCount(NewDataType);

    elementType = clmGEN_CODE_elementType_GET(ROperand->dataType);
    if(clmIsElementTypeFloating(elementType)) {
        for (i = 1; i < vectorComponentCount; i++)
        {
            ROperand->u.constant.values[i].floatValue = ROperand->u.constant.values[0].floatValue;
        }
    }
    else if(clmIsElementTypeBoolean(elementType)) {
        for (i = 1; i < vectorComponentCount; i++)
        {
            ROperand->u.constant.values[i].boolValue = ROperand->u.constant.values[0].boolValue;
        }
    }
    else if(clmIsElementTypeInteger(elementType)) {
        for (i = 1; i < vectorComponentCount; i++)
        {
            ROperand->u.constant.values[i].intValue = ROperand->u.constant.values[0].intValue;
        }
    }
    else {
        gcmASSERT(0);
    }

    ROperand->u.constant.valueCount = vectorComponentCount;

    ROperand->dataType = gcConvScalarToVectorDataType(ROperand->dataType, vectorComponentCount);

    gcmVERIFY_OK(clsROPERAND_ChangeDataTypeFamily(Compiler,
                            0,
                            0,
                            gcvFALSE,
                            NewDataType,
                            ROperand));
}

gctBOOL
clsROPERAND_CONSTANT_IsAllVectorComponentsEqual(
    IN clsROPERAND * ROperand
    )
{
    gctUINT        i;
    clsGEN_CODE_DATA_TYPE dataType;
    cltELEMENT_TYPE elementType;

    gcmASSERT(ROperand);
    gcmASSERT(!ROperand->isReg);

    if(ROperand->u.constant.valueCount == 1) return gcvTRUE;
    dataType = gcGetVectorComponentDataType(ROperand->dataType);
    elementType = clmGEN_CODE_elementType_GET(dataType);
    if(clmIsElementTypePacked(elementType)) {
        elementType = _ConvPackedTypeToBasicType(elementType);
    }

    if(clmIsElementTypeFloating(elementType)) {
        for (i = 1; i < ROperand->u.constant.valueCount; i++)
        {
            if (ROperand->u.constant.values[i].floatValue
                != ROperand->u.constant.values[0].floatValue)
            {
                return gcvFALSE;
            }
        }
    }
    else if(clmIsElementTypeBoolean(elementType)) {
        for (i = 1; i < ROperand->u.constant.valueCount; i++)
        {
            if (ROperand->u.constant.values[i].boolValue
                != ROperand->u.constant.values[0].boolValue)
            {
                return gcvFALSE;
            }
        }
    }
    else if(clmIsElementTypeInteger(elementType)) {
        for (i = 1; i < ROperand->u.constant.valueCount; i++)
        {
            if (ROperand->u.constant.values[i].intValue
                != ROperand->u.constant.values[0].intValue)
            {
                return gcvFALSE;
            }
        }
    }
    else {
        gcmASSERT(0);
    }

    return gcvTRUE;
}

gctCONST_STRING
clGetOpcodeName(
    IN cleOPCODE Opcode
    )
{
    switch (Opcode)
    {
    case clvOPCODE_ASSIGN:   return "=";
    case clvOPCODE_CONV:     return "conv";
    case clvOPCODE_CONV_SAT: return "conv_sat";
    case clvOPCODE_CONV_SAT_RTE: return "conv_sat_rte";
    case clvOPCODE_CONV_SAT_RTZ: return "conv_sat_rtz";
    case clvOPCODE_CONV_SAT_RTN: return "conv_sat_rtn";
    case clvOPCODE_CONV_SAT_RTP: return "conv_sat_rtp";
    case clvOPCODE_CONV_RTE: return "conv_sat_rte";
    case clvOPCODE_CONV_RTZ: return "conv_sat_rtz";
    case clvOPCODE_CONV_RTN: return "conv_sat_rtn";
    case clvOPCODE_CONV_RTP: return "conv_sat_rtp";
    case clvOPCODE_ASTYPE:   return "astype";
    case clvOPCODE_ADD:         return "+";
    case clvOPCODE_SUB:         return "-";
    case clvOPCODE_MUL:         return "*";
    case clvOPCODE_DIV:         return "divide#";
    case clvOPCODE_IDIV:     return "divide_int#";
    case clvOPCODE_IMUL:     return "mul_int#";
    case clvOPCODE_FMUL:     return "mul#";
    case clvOPCODE_FADD:     return "add#";
    case clvOPCODE_FSUB:     return "sub#";
    case clvOPCODE_MOD:      return "mod#";
    case clvOPCODE_FMOD:     return "viv_fmod";
    case clvOPCODE_SELECT:   return "viv_select";
    case clvOPCODE_FMA:      return "fma#";

    case clvOPCODE_TEXTURE_LOAD:     return "texture_load";
    case clvOPCODE_IMAGE_SAMPLER:    return "image_sampler";
    case clvOPCODE_IMAGE_READ:       return "image_read";
    case clvOPCODE_IMAGE_WRITE:      return "image_write";

    case clvOPCODE_FLOAT_TO_INT:     return "float_to_int";
    case clvOPCODE_FLOAT_TO_UINT:    return "float_to_uint";
    case clvOPCODE_FLOAT_TO_BOOL:    return "float_to_bool";
    case clvOPCODE_INT_TO_UINT:      return "int_to_uint";
    case clvOPCODE_INT_TO_INT:       return "int_to_int";
    case clvOPCODE_INT_TO_BOOL:      return "int_to_bool";
    case clvOPCODE_INT_TO_FLOAT:     return "int_to_float";
    case clvOPCODE_BOOL_TO_INT:      return "bool_to_int";
    case clvOPCODE_BOOL_TO_UINT:     return "bool_to_uint";
    case clvOPCODE_BOOL_TO_FLOAT:    return "bool_to_float";
    case clvOPCODE_UINT_TO_UINT:     return "uint_to_uint";
    case clvOPCODE_UINT_TO_INT:      return "uint_to_int";
    case clvOPCODE_UINT_TO_BOOL:     return "uint_to_bool";
    case clvOPCODE_UINT_TO_FLOAT:    return "uint_to_float";

    case clvOPCODE_IMPL_B2F:         return "impl_B2F";
    case clvOPCODE_IMPL_U2F:         return "impl_U2F";
    case clvOPCODE_IMPL_I2F:         return "impl_I2F";

    case clvOPCODE_INVERSE:          return "viv_reciprocal";
    case clvOPCODE_LESS_THAN:        return "isless";
    case clvOPCODE_LESS_THAN_EQUAL:  return "islessequal";
    case clvOPCODE_GREATER_THAN:     return "isgreater";
    case clvOPCODE_GREATER_THAN_EQUAL: return "isgreaterequal";
    case clvOPCODE_EQUAL:              return "isequal";
    case clvOPCODE_NOT_EQUAL:          return "isnotequal";

    case clvOPCODE_BITWISE_AND:        return "bitwise_and";
    case clvOPCODE_BITWISE_OR:         return "bitwise_or";
    case clvOPCODE_BITWISE_XOR:        return "bitwise_xor";
    case clvOPCODE_BITWISE_NOT:        return "bitwise_not";

    case clvOPCODE_RSHIFT:             return ">>";
    case clvOPCODE_LSHIFT:             return "<<";

    case clvOPCODE_RIGHT_SHIFT:        return "right_shift#";
    case clvOPCODE_LEFT_SHIFT:         return "left_shift#";

    case clvOPCODE_BARRIER:            return "barrier";
    case clvOPCODE_LOAD:               return "load";
    case clvOPCODE_STORE:              return "store";
    case clvOPCODE_STORE1:             return "store1";

    case clvOPCODE_ANY:                return "any";
    case clvOPCODE_ALL:                return "all";
    case clvOPCODE_NOT:                return "!";
    case clvOPCODE_NEG:                return "-";

    case clvOPCODE_SIN:                return "native_sin";
    case clvOPCODE_COS:                return "native_cos";
    case clvOPCODE_TAN:                return "native_tan";

    case clvOPCODE_ASIN:        return "native#asin";
    case clvOPCODE_ACOS:        return "native#acos";
    case clvOPCODE_ATAN:        return "native#atan";
    case clvOPCODE_ATAN2:       return "atan2";

    case clvOPCODE_SINPI:       return "sinpi";
    case clvOPCODE_COSPI:       return "cospi";
    case clvOPCODE_TANPI:       return "tanpi";

    case clvOPCODE_MULLO:       return "viv_mul_lo";
    case clvOPCODE_ADDLO:       return "viv_add_lo";

    case clvOPCODE_ROTATE:      return "rotate";
    case clvOPCODE_LEADZERO:    return "leadzero";
    case clvOPCODE_GETEXP:      return "getexp";
    case clvOPCODE_GETMANT:     return "getmant";

    case clvOPCODE_POW:         return "viv_pow";
    case clvOPCODE_EXP2:        return "viv_exp2";
    case clvOPCODE_LOG2:        return "viv_log2";
    case clvOPCODE_SQRT:        return "viv_sqrt";
    case clvOPCODE_INVERSE_SQRT: return "viv_rsqrt";

    case clvOPCODE_ABS:         return "viv_abs";
    case clvOPCODE_POPCOUNT:    return "viv_popcount";
    case clvOPCODE_SIGN:        return "viv_sign";
    case clvOPCODE_FLOOR:       return "viv_floor";
    case clvOPCODE_CEIL:        return "viv_ceil";
    case clvOPCODE_FRACT:       return "viv_fract";
    case clvOPCODE_MIN:         return "viv_min";
    case clvOPCODE_MAX:         return "viv_max";
    case clvOPCODE_SATURATE:    return "viv_saturate";
    case clvOPCODE_STEP:        return "viv_step";
    case clvOPCODE_DOT:         return "viv_dot";
    case clvOPCODE_CROSS:       return "viv_cross";
    case clvOPCODE_NORMALIZE:   return "viv_normalize";

    case clvOPCODE_JUMP:        return "jump";
    case clvOPCODE_CALL:        return "call";
    case clvOPCODE_RETURN:      return "return";

    case clvOPCODE_DFDX:        return "dFdx";
    case clvOPCODE_DFDY:        return "dFdy";
    case clvOPCODE_FWIDTH:      return "fwidth";
    case clvOPCODE_MULHI:       return "mulIntHi";
    case clvOPCODE_CMP:         return "cmp";
    case clvOPCODE_ADDSAT:      return "addsat";
    case clvOPCODE_SUBSAT:      return "subsat";
    case clvOPCODE_MULSAT:      return "mulsat";
    case clvOPCODE_MADSAT:      return "madsat";

    case clvOPCODE_ATOMADD:             return "atomadd";
    case clvOPCODE_ATOMSUB:             return "atomsub";
    case clvOPCODE_ATOMXCHG:            return "atomxchg";
    case clvOPCODE_ATOMCMPXCHG:         return "atomcmpxchg";
    case clvOPCODE_ATOMMIN:             return "atommin";
    case clvOPCODE_ATOMMAX:             return "atommax";
    case clvOPCODE_ATOMOR:              return "atomor";
    case clvOPCODE_ATOMAND:             return "atomand";
    case clvOPCODE_ATOMXOR:             return "atomxor";

    case clvOPCODE_UNPACK:              return "unpack";

    case clvOPCODE_ADD_RTZ:             return "add#rtz";
    case clvOPCODE_ADD_RTNE:            return "add#rtne";
    case clvOPCODE_ADDLO_RTZ:           return "addlo#rtz";
    case clvOPCODE_ADDLO_RTNE:          return "addlo#rtne";
    case clvOPCODE_SUB_RTZ:             return "sub#rtz";
    case clvOPCODE_SUB_RTNE:            return "sub#rtne";
    case clvOPCODE_MUL_RTZ:             return "mul#rtz";
    case clvOPCODE_MUL_RTNE:            return "mul#rtne";
    case clvOPCODE_MULLO_RTZ:           return "mullo#rtz";
    case clvOPCODE_MULLO_RTNE:          return "mullo#rtne";
    case clvOPCODE_FRACT_RTZ:           return "fract#rtz";
    case clvOPCODE_FRACT_RTNE:          return "fract#rtne";
    case clvOPCODE_INT_TO_FLOAT_RTZ:    return "int_to_float#rtz";
    case clvOPCODE_INT_TO_FLOAT_RTNE:   return "int_to_float#rtne";
    case clvOPCODE_UINT_TO_FLOAT_RTZ:   return "uint_to_float#rtz";
    case clvOPCODE_UINT_TO_FLOAT_RTNE:  return "uint_to_float#rtne";

    case clvOPCODE_LONGLO:              return "longlo";
    case clvOPCODE_LONGHI:              return "longhi";
    case clvOPCODE_MOV_LONG:            return "mov_long";
    case clvOPCODE_COPY:                return "copy";
    default:
        gcmASSERT(0);
        return "Invalid";
    }
}

gctCONST_STRING
clGetConditionName(
    IN cleCONDITION Condition
    )
{
    switch (Condition)
    {
    case clvCONDITION_EQUAL:                return "equal";
    case clvCONDITION_NOT_EQUAL:            return "not_equal";
    case clvCONDITION_LESS_THAN:            return "less_than";
    case clvCONDITION_LESS_THAN_EQUAL:        return "less_than_equal";
    case clvCONDITION_GREATER_THAN:            return "greater_than";
    case clvCONDITION_GREATER_THAN_EQUAL:    return "greater_than_equal";
    case clvCONDITION_XOR:                    return "xor";

    default:
        gcmASSERT(0);
        return "Invalid";
    }
}

cleCONDITION
clGetNotCondition(
    IN cleCONDITION Condition
    )
{
    switch (Condition)
    {
    case clvCONDITION_EQUAL:            return clvCONDITION_NOT_EQUAL;
    case clvCONDITION_NOT_EQUAL:
    case clvCONDITION_XOR:                return clvCONDITION_EQUAL;
    case clvCONDITION_LESS_THAN:            return clvCONDITION_GREATER_THAN_EQUAL;
    case clvCONDITION_LESS_THAN_EQUAL:        return clvCONDITION_GREATER_THAN;
    case clvCONDITION_GREATER_THAN:            return clvCONDITION_LESS_THAN_EQUAL;
    case clvCONDITION_GREATER_THAN_EQUAL:        return clvCONDITION_LESS_THAN;
    case clvCONDITION_ZERO:                return clvCONDITION_NOT_ZERO;
    case clvCONDITION_NOT_ZERO:            return clvCONDITION_ZERO;

    default:
        gcmASSERT(0);
        return clvCONDITION_NOT_EQUAL;
    }
}

gctBOOL
clsROPERAND_IsFloatOrVecConstant(
    IN clsROPERAND * ROperand,
    IN gctFLOAT FloatValue
    )
{
    gcmASSERT(ROperand);

    if (ROperand->isReg) return gcvFALSE;

    if(clmIsElementTypeFloating(clmGEN_CODE_elementType_GET(ROperand->dataType)) &&
       (gcIsScalarDataType(ROperand->dataType) || gcIsVectorDataType(ROperand->dataType))) {

       switch(clmGEN_CODE_vectorSize_GET(ROperand->dataType)) {
       case 0:
        return (ROperand->u.constant.values[0].floatValue == FloatValue);

       case 2:
        return (ROperand->u.constant.values[0].floatValue == FloatValue
                && ROperand->u.constant.values[1].floatValue == FloatValue);

       case 3:
        return (ROperand->u.constant.values[0].floatValue == FloatValue
                && ROperand->u.constant.values[1].floatValue == FloatValue
                && ROperand->u.constant.values[2].floatValue == FloatValue);

       case 4:
        return (ROperand->u.constant.values[0].floatValue == FloatValue
                && ROperand->u.constant.values[1].floatValue == FloatValue
                && ROperand->u.constant.values[2].floatValue == FloatValue
                && ROperand->u.constant.values[3].floatValue == FloatValue);
       }
    }
    return gcvFALSE;
}

gctSIZE_T
_GetDataTypeRegSize(
IN clsDATA_TYPE *DataType
)
{
    gctUINT8 size;

    size = clmDATA_TYPE_matrixColumnCount_GET(DataType);
    if(size == 0) size = 1;
    switch(clmDATA_TYPE_matrixRowCount_GET(DataType)) {
    case 8:
      size <<= 1;
      break;

    case 16:
      size <<= 2;
      break;
    }
    return size;
}

static gctUINT
_GetNameRegSize(
IN clsNAME * Name
);

static gctUINT
_GetDeclRegSize(
IN clsDECL * Decl
)
{
   gctUINT count = 0;
   clsNAME *fieldName;

   if(clmDECL_IsPointerType(Decl)) {
      count = 1;
   }
   else {
     if (Decl->dataType->elementType == clvTYPE_STRUCT ||
         Decl->dataType->elementType == clvTYPE_UNION) {
         gctUINT localCount = 0;
         gctUINT curCount;

         gcmASSERT(Decl->dataType->u.fieldSpace);

         FOR_EACH_DLINK_NODE(&Decl->dataType->u.fieldSpace->names, clsNAME, fieldName) {
            gcmASSERT(fieldName->decl.dataType);
            curCount = _GetNameRegSize(fieldName);
            if(Decl->dataType->elementType == clvTYPE_UNION) {
                if(curCount > localCount) localCount = curCount;
            }
            else localCount += curCount;
        }
        count += localCount;
     }
     else {
        count += _GetDataTypeRegSize(Decl->dataType);
     }
   }

   if (clmDECL_IsArray(Decl)) {
      gctINT elementCount;
      clmGetArrayElementCount(Decl->array, 0, elementCount);
      count *= elementCount;
   }
   return count;
}

static gctUINT
_GetNameRegSize(
IN clsNAME * Name
)
{
   gcmASSERT(Name);

   if(clmGEN_CODE_checkVariableForMemory(Name)) {
    return 1;
   }
   else return _GetDeclRegSize(&Name->decl);
}

static gctUINT
_GetLogicalOperandCount(
IN clsDECL * Decl
)
{
   gctUINT count = 0;
   clsNAME *fieldName;

   gcmASSERT(Decl);

   if(clmDECL_IsPointerType(Decl) ||
      (clmDECL_IsArray(Decl) && Decl->dataType->addrSpaceQualifier == clvQUALIFIER_LOCAL)) {
      return 1;
   }
   if (Decl->dataType->elementType == clvTYPE_STRUCT ||
       Decl->dataType->elementType == clvTYPE_UNION) {
      gctUINT localCount = 0;
      gctUINT curCount;

      gcmASSERT(Decl->dataType->u.fieldSpace);

      FOR_EACH_DLINK_NODE(&Decl->dataType->u.fieldSpace->names, clsNAME, fieldName) {
         gcmASSERT(fieldName->decl.dataType);
         curCount = _GetLogicalOperandCount(&fieldName->decl);
         if(Decl->dataType->elementType == clvTYPE_UNION) {
            if(curCount > localCount) localCount = curCount;
         }
         else localCount += curCount;
      }
      count += localCount;
   }
   else {
      count = 1;
   }

   if (clmDECL_IsArray(Decl)) {
      gctINT elementCount;
      clmGetArrayElementCount(Decl->array, 0, elementCount);
      count *= elementCount;
   }
   return count;
}

gctUINT
clGetOperandCountForRegAlloc(
IN clsNAME * Name
)
{
   gctUINT count = 0;
   clsNAME *fieldName;
   clsDECL *decl;

   gcmASSERT(Name);

   decl = &Name->decl;
   if(clmDECL_IsPointerType(decl)) {
      return 1;
   }

   if (decl->dataType->elementType == clvTYPE_STRUCT ||
       decl->dataType->elementType == clvTYPE_UNION) {
      gctUINT localCount = 0;
      gctUINT curCount;

      gcmASSERT(decl->dataType->u.fieldSpace);

      FOR_EACH_DLINK_NODE(&decl->dataType->u.fieldSpace->names, clsNAME, fieldName) {
         gcmASSERT(fieldName->decl.dataType);
         curCount = _GetLogicalOperandCount(&fieldName->decl);
         if(decl->dataType->elementType == clvTYPE_UNION) {
            if(curCount > localCount) localCount = curCount;
         }
         else localCount += curCount;
      }
      count += localCount;
   }
   else {
      count = 1;
   }

   if (clmDECL_IsArray(decl)) {
      gctINT elementCount;
      clmGetArrayElementCount(decl->array, 0, elementCount);
      count *= elementCount;
   }
   return count;
}

gctUINT
_GetOperandCountForRegAlloc(
IN clsNAME * Name
)
{
   clsDECL *decl;

   gcmASSERT(Name);

   decl = &Name->decl;
   if(clmDECL_IsPointerType(decl) || clmGEN_CODE_checkVariableForMemory(Name)) {
      return 1;
   }
   return clGetOperandCountForRegAlloc(Name);
}

static gctSIZE_T
_GetLogicalOperandFieldOffset(
    IN clsDECL * StructDecl,
    IN clsNAME * FieldName
    )
{
    gctSIZE_T    offset = 0;
    clsNAME *    fieldName;

    gcmASSERT(StructDecl);
    gcmASSERT(clmDECL_IsStructOrUnion(StructDecl));
    gcmASSERT(FieldName);

    gcmASSERT(StructDecl->dataType->u.fieldSpace);

    FOR_EACH_DLINK_NODE(&StructDecl->dataType->u.fieldSpace->names, clsNAME, fieldName)
    {
        if (fieldName == FieldName) break;
        gcmASSERT(fieldName->decl.dataType);

             if(StructDecl->dataType->elementType == clvTYPE_UNION) continue;
        else offset += _GetLogicalOperandCount(&fieldName->decl);
    }

    gcmASSERT(fieldName == FieldName);

    return offset;
}

static gctUINT
_GetOperandByteSize(
IN clsNAME * Operand
)
{
   gcmASSERT(Operand);

   return  clsDECL_GetByteSize(&Operand->decl);
}

static gctSIZE_T
_GetFieldByteOffset(
    IN clsDECL * StructDecl,
    IN clsNAME * FieldName,
    OUT gctUINT * Alignment
    )
{
   gctSIZE_T    offset = 0;
   clsNAME *    fieldName;
   gctUINT alignment;
   gctBOOL packed = gcvFALSE;

   gcmASSERT(StructDecl);
   gcmASSERT(clmDATA_TYPE_IsStructOrUnion(StructDecl->dataType));
   gcmASSERT(FieldName);

   gcmASSERT(StructDecl->dataType->u.fieldSpace);

   FOR_EACH_DLINK_NODE(&StructDecl->dataType->u.fieldSpace->names, clsNAME, fieldName)
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
              else alignment = clPermissibleAlignment(&fieldName->decl);
           }
           else alignment = clPermissibleAlignment(&fieldName->decl);
        }
        offset = clmALIGN(offset, alignment, packed) + _GetOperandByteSize(fieldName);
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
          else alignment = clPermissibleAlignment(&FieldName->decl);
       }
       else alignment = clPermissibleAlignment(&FieldName->decl);
   }

   if(Alignment) {
       *Alignment = alignment;
   }
   return clmALIGN(offset, alignment, packed);
}

static gceSTATUS
_GetBaseAlignmentForStruct(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN clsDECL * StructDecl,
    OUT gctUINT * StructAlignment
    )
{
    clsNAME* fieldName;
    gctUINT alignment = 0;

    gcmASSERT(clmDATA_TYPE_IsStructOrUnion(StructDecl->dataType));
    FOR_EACH_DLINK_NODE(&StructDecl->dataType->u.fieldSpace->names, clsNAME, fieldName)
    {
        gctUINT childAlignment = 0;

        gcmASSERT(fieldName->decl.dataType);

        if(clmDECL_IsUnderlyingStructOrUnion(&fieldName->decl))
        {
            _GetBaseAlignmentForStruct(Compiler,
                                       CodeGenerator,
                                       &fieldName->decl,
                                       &childAlignment);
        }
        else
        {
            _GetFieldByteOffset(StructDecl,
                                fieldName,
                                &childAlignment);
        }

        if (childAlignment > alignment)
            alignment = childAlignment;
    }
    if (StructAlignment)
        *StructAlignment = alignment;
    return gcvSTATUS_OK;
}

static gctINT
_AssignStructOrUnionInMemory(
    IN cloCOMPILER Compiler,
    IN gctINT LineNo,
    IN gctINT StringNo,
    IN clsDECL * StructDecl,
    IN clsLOPERAND *LOperand,
    IN gctINT LOffset,
    IN clsGEN_CODE_PARAMETERS *Rhs,
    IN OUT gctINT *CurOperand,
    IN gctBOOL DoAssign
    )
{
   gctINT size = 0;
   gctUINT i;
   gctUINT elementCount = 1;
   gctUINT structAlignment = 0;
   gctBOOL packed = gcvFALSE;
   gctINT lOffset;
   clsROPERAND offset[1];
   gceSTATUS status;

   if (clmDECL_IsArray(StructDecl)) {
      clmGetArrayElementCount(StructDecl->array, 0, elementCount);
   }

   lOffset = LOffset;
   for(i = 0; i < elementCount; i++) {
      if(clmDECL_IsPointerType(StructDecl)) {
         size = 4;
      }
      else {
         switch(StructDecl->dataType->elementType) {
         case clvTYPE_STRUCT:
         case clvTYPE_UNION:
            {
              clsNAME *fieldName;
              gctUINT localSize = 0;
              gctUINT newLocalSize;
              gctUINT curSize;
              gctUINT alignment;
              gctUINT localCount = 0;
              gctUINT curCount;
              gctBOOL doAssign = DoAssign;
              gctUINT curOperandIndex;

              gcmASSERT(StructDecl->dataType->u.fieldSpace);

              curOperandIndex = *CurOperand;
              FOR_EACH_DLINK_NODE(&StructDecl->dataType->u.fieldSpace->names, clsNAME, fieldName) {
                 gcmASSERT(fieldName->decl.dataType);

                 if(i == 0) {
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
                          subField = slsDLINK_LIST_First(&fieldName->decl.dataType->u.fieldSpace->names,
                                                         struct _clsNAME);
                          if(subField->u.variableInfo.specifiedAttr & clvATTR_ALIGNED) {
                             alignment = subField->context.alignment;
                          }
                          else alignment = clPermissibleAlignment(&fieldName->decl);
                       }
                       else alignment = clPermissibleAlignment(&fieldName->decl);
                    }
                    if(structAlignment == 0) structAlignment = alignment;
                    else {
                       structAlignment = clFindLCM(structAlignment, alignment);
                    }

                    newLocalSize = clmALIGN(localSize, alignment, packed);
                    fieldName->u.variableInfo.padding = newLocalSize - localSize;
                 }
                 localSize += fieldName->u.variableInfo.padding;
                 curSize = _AssignStructOrUnionInMemory(Compiler,
                                                        LineNo,
                                                        StringNo,
                                                        &fieldName->decl,
                                                        LOperand,
                                                        localSize + lOffset,
                                                        Rhs,
                                                        CurOperand,
                                                        doAssign);

                 if(StructDecl->dataType->elementType == clvTYPE_UNION) {
                if(curSize > localSize) localSize = curSize;
                    curCount = *CurOperand - curOperandIndex;
                    if(curCount > localCount) localCount = curCount;
                    doAssign = gcvFALSE;
                    *CurOperand = curOperandIndex;
                 }
                 else {
                    localSize += curSize;
                 }
              }

              if(StructDecl->dataType->elementType == clvTYPE_UNION) {
                 curOperandIndex += localCount;
              }

              newLocalSize = clmALIGN(localSize, structAlignment, packed);
              lOffset += newLocalSize - localSize;
              continue;
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
            size = 2;
            break;

        case clvTYPE_SHORT_PACKED:
        case clvTYPE_USHORT_PACKED:
        case clvTYPE_HALF_PACKED:
            size = 2;
            break;

        case clvTYPE_HALF:
            size = 2;
            break;

        case clvTYPE_SAMPLER_T:
        case clvTYPE_IMAGE2D_T:
        case clvTYPE_IMAGE3D_T:
            size = 4;
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

         if (clmDATA_TYPE_vectorSize_GET(StructDecl->dataType) > 0) {
             if (clmDATA_TYPE_vectorSize_NOCHECK_GET(StructDecl->dataType) == 3) {
                 /* 3-component vector data type must be aligned to a 4*sizeof(component) */
                 size*=4;
             } else {
                 size *= clmDATA_TYPE_vectorSize_NOCHECK_GET(StructDecl->dataType);
             }
         }
         else if (clmDATA_TYPE_matrixColumnCount_GET(StructDecl->dataType) > 0) {
            size *= clmDATA_TYPE_matrixRowCount_GET(StructDecl->dataType) * clmDATA_TYPE_matrixColumnCount_GET(StructDecl->dataType);
         }
      }
      _InitializeROperandConstant(offset, clmGenCodeDataType(T_UINT), lOffset);
      (*CurOperand)++;
      if(DoAssign) {
         status = clGenStoreCode(Compiler,
                                 LineNo,
                                 StringNo,
                                 Rhs->rOperands + *CurOperand,
                                 LOperand,
                                 Rhs->dataTypes[*CurOperand].def,
                                 offset);
         if (gcmIS_ERROR(status)) return -1;
      }
      lOffset += size;
   }

   return lOffset - LOffset;
}

clsCOMPONENT_SELECTION
clGetDefaultComponentSelection(
    IN clsGEN_CODE_DATA_TYPE DataType
    )
{
    switch(clmGEN_CODE_elementType_GET(DataType)) {
    case clvTYPE_SAMPLER2D:
    case clvTYPE_SAMPLER3D:
        return ComponentSelection_XYZW;
    default:
        break;
    }
    switch(clmGEN_CODE_vectorSize_NOCHECK_GET(DataType)) {
    case 0:
      return ComponentSelection_X;

    case 2:
      return ComponentSelection_XY;

    case 3:
      return ComponentSelection_XYZ;

    case 4:
      return ComponentSelection_XYZW;

    case 8:
      return ComponentSelection_VECTOR8;

    case 16:
      return ComponentSelection_VECTOR16;

    default:
      if(!clmIsElementTypePacked(DataType.elementType)) {
          gcmASSERT(0);
      }
      return ComponentSelection_XYZW;
    }
}

static void
_FillDefaultComponentSelection(
    IN clsGEN_CODE_DATA_TYPE DataType,
    OUT clsCOMPONENT_SELECTION *DefaultSelection
    )
{

    gctUINT8 i;
    clsCOMPONENT_SELECTION componentSelection;


    componentSelection = clGetDefaultComponentSelection(DataType);
    for(i=0; i < cldMaxFourComponentCount; i++) {
    DefaultSelection[i] = componentSelection;
    }
}

static clsGEN_CODE_DATA_TYPE
_ConvElementDataTypeForRegAlloc(
IN clsNAME * Name
)
{
   IN clsDECL * decl;
   gcmASSERT(Name && Name->decl.dataType);

   decl = &Name->decl;
   if(clmDECL_IsPointerType(decl) || clmGEN_CODE_checkVariableForMemory(Name)) {
      return clmGenCodeDataType(T_UINT);
   }
   else {
      clsGEN_CODE_DATA_TYPE resultType;

      gcmASSERT(decl->dataType);
      clmGEN_CODE_ConvDirectElementDataType(decl->dataType, resultType);
      return resultType;
   }
}

static clsGEN_CODE_DATA_TYPE
_ConvElementDataType(
IN clsDECL * Decl
)
{
   gcmASSERT(Decl && Decl->dataType);

   if(clmDECL_IsPointerType(Decl)) {
      return clmGenCodeDataType(T_UINT);
   }
   else {
      clsGEN_CODE_DATA_TYPE resultType;

      clmGEN_CODE_ConvDirectElementDataType(Decl->dataType, resultType);
      return resultType;
   }
}


static gceSTATUS
_ConvDataType(
IN clsDECL * Decl,
OUT clsGEN_CODE_PARAMETER_DATA_TYPE * TargetDataTypes,
IN OUT gctUINT * Start
)
{
   gceSTATUS status;
   gctUINT count, i, curStart, prevStart;
   clsNAME *fieldName;

   gcmASSERT(Decl);
   gcmASSERT(TargetDataTypes);
   gcmASSERT(Start);

   if(clmDECL_IsArray(Decl) && Decl->dataType->addrSpaceQualifier == clvQUALIFIER_LOCAL) {
    TargetDataTypes[*Start].def = _ConvElementDataType(Decl);
    (*Start)++;
    return gcvSTATUS_OK;
   }

   if(clmDECL_IsPointerType(Decl)) {
    TargetDataTypes[*Start].def = _ConvElementDataType(Decl);
    (*Start)++;
    return gcvSTATUS_OK;
   }

   if (clmDECL_IsArray(Decl)) {
      clmGetArrayElementCount(Decl->array, 0, count);
   }
   else count = 1;

   for (i = 0; i < count; i++) {
      if (Decl->dataType->elementType == clvTYPE_STRUCT ||
          Decl->dataType->elementType == clvTYPE_UNION) {
        gcmASSERT(Decl->dataType->u.fieldSpace);

        prevStart = curStart = *Start;
        FOR_EACH_DLINK_NODE(&Decl->dataType->u.fieldSpace->names, clsNAME, fieldName) {
           gcmASSERT(fieldName->decl.dataType);

           status = _ConvDataType(&fieldName->decl,
                                  TargetDataTypes,
                                  Start);
           if(Decl->dataType->elementType == clvTYPE_UNION) {
          if(*Start > curStart) curStart = *Start;
          *Start = prevStart;
       }
       else curStart = *Start;
           if (gcmIS_ERROR(status)) return status;
    }
    *Start = curStart;
      }
      else {
    TargetDataTypes[*Start].def = _ConvElementDataType(Decl);
    (*Start)++;
      }
  }

  return gcvSTATUS_OK;
}

static gceSTATUS
_ConvDataTypeByName(
IN clsNAME * Name,
OUT clsGEN_CODE_PARAMETER_DATA_TYPE * TargetDataTypes,
IN OUT gctUINT * Start
)
{
   gceSTATUS status;
   gctUINT count, i, curStart, prevStart;
   clsNAME *fieldName;
   clsDECL * decl;
   gcmASSERT(Name && Name->decl.dataType);

   gcmASSERT(TargetDataTypes);
   gcmASSERT(Start);
   decl = &Name->decl;
   if(clmDECL_IsPointerType(decl) || clmGEN_CODE_checkVariableForMemory(Name)) {
    TargetDataTypes[*Start].def = clmGenCodeDataType(T_UINT);
    (*Start)++;
    return gcvSTATUS_OK;
   }

   if (clmDECL_IsArray(decl)) {
      clmGetArrayElementCount(decl->array, 0, count);
   }
   else count = 1;

   for (i = 0; i < count; i++) {
      if (decl->dataType->elementType == clvTYPE_STRUCT ||
          decl->dataType->elementType == clvTYPE_UNION) {
        gcmASSERT(decl->dataType->u.fieldSpace);

        prevStart = curStart = *Start;
        FOR_EACH_DLINK_NODE(&decl->dataType->u.fieldSpace->names, clsNAME, fieldName) {
           gcmASSERT(fieldName->decl.dataType);

           status = _ConvDataType(&fieldName->decl,
                                  TargetDataTypes,
                                  Start);
           if(decl->dataType->elementType == clvTYPE_UNION) {
          if(*Start > curStart) curStart = *Start;
          *Start = prevStart;
       }
       else curStart = *Start;
           if (gcmIS_ERROR(status)) return status;
    }
    *Start = curStart;
      }
      else {
    TargetDataTypes[*Start].def = _ConvElementDataType(decl);
    (*Start)++;
      }
  }

  return gcvSTATUS_OK;
}

static gctBOOL
_IsTempRegQualifier(
    IN cltQUALIFIER Qualifier
    )
{
    switch (Qualifier)
    {
    case clvQUALIFIER_NONE:
    case clvQUALIFIER_CONST_IN:
    case clvQUALIFIER_OUT:
        return gcvTRUE;

    default:
        return gcvFALSE;
    }
}

static cltQUALIFIER
_MapQualifierToShaderQualifier(
cltQUALIFIER Qualifier
)
{
   cltQUALIFIER qualifier;

   switch(Qualifier) {
   case clvQUALIFIER_READ_ONLY:
        qualifier = clvQUALIFIER_READ_ONLY;
        break;

   case clvQUALIFIER_WRITE_ONLY:
        qualifier = clvQUALIFIER_WRITE_ONLY;
        break;

   default :
        qualifier = Qualifier;
        break;
   }
   return qualifier;
}

static void
_SetPointerUniformQualifiers(
    IN gcUNIFORM Uniform,
    IN clsDECL * Decl
    )
{
    if(clmDECL_IsPointerType(Decl)) {
        clsTYPE_QUALIFIER *nextDscr;
        clsTYPE_QUALIFIER *prevDscr;

        FOR_EACH_SLINK_NODE(Decl->ptrDscr, clsTYPE_QUALIFIER, prevDscr, nextDscr) {
            if(nextDscr->type == T_EOF) break;
            switch(nextDscr->type) {
            case T_RESTRICT:
            case T_VOLATILE:
            case T_STATIC:
            case T_EXTERN:
                 SetUniformQualifier(Uniform, clConvStorageQualifierToShaderTypeQualifier(nextDscr->qualifier));
                 break;

            default:
                 SetUniformQualifier(Uniform, clConvToShaderTypeQualifier(nextDscr->qualifier));
                 break;
            }
        }
        SetUniformFlag(Uniform, gcvUNIFORM_FLAG_IS_POINTER);
    }
}

struct _clsDERIVED_TYPE_VARIABLE
{
    slsSLINK_NODE node;
    gctUINT typeNameLength;
    struct _clsNAME *name;
};

static gcSL_FORMAT
_ConvDerivedTypeToFormat(
clsNAME *DerivedType
)
{
    switch (DerivedType->type) {
    case clvUNION_NAME:
        return gcSL_UNION;

    case clvSTRUCT_NAME:
        return gcSL_STRUCT;

    case clvENUM_TAG_NAME:
        return gcSL_ENUM;

    case clvTYPE_NAME:
        return gcSL_TYPEDEF;

    default:
        return gcSL_VOID;
    }
}

static void
_AddDerivedTypeVariable(
IN cloCOMPILER Compiler,
IN cloCODE_GENERATOR CodeGenerator,
IN clsNAME *Variable,
IN gcUNIFORM Uniform
)
{
   clsNAME *derivedType;

   if(!Variable->derivedType &&
      Variable->decl.dataType->elementType != clvTYPE_STRUCT &&
      Variable->decl.dataType->elementType != clvTYPE_UNION) return;

   derivedType = Variable->derivedType;
   if(!derivedType) {
       clsNAME_SPACE *fieldNameSpace;

       switch (Variable->decl.dataType->elementType) {
       case clvTYPE_STRUCT:
       case clvTYPE_UNION:
           fieldNameSpace = Variable->decl.dataType->u.fieldSpace;
           derivedType = fieldNameSpace->scopeName;
           break;

       default:
           break;
       }
   }
   gcmASSERT(derivedType);

   gcmASSERT(derivedType->type == clvSTRUCT_NAME ||
             derivedType->type == clvUNION_NAME ||
             derivedType->type == clvTYPE_NAME ||
             derivedType->type == clvENUM_TAG_NAME);

   if(derivedType->u.typeInfo.typeNameOffset == -1) {
       gceSTATUS status;
       struct _clsDERIVED_TYPE_VARIABLE *derivedTypeVariable;
       gctUINT nameLength;
       gctPOINTER pointer;

       status = cloCOMPILER_Allocate(Compiler,
                                     (gctSIZE_T)sizeof(struct _clsDERIVED_TYPE_VARIABLE),
                                     (gctPOINTER *) &pointer);
       if (gcmIS_ERROR(status)) return;

       derivedTypeVariable = pointer;

       derivedTypeVariable->name = derivedType;
       nameLength = gcoOS_StrLen(derivedType->symbol, gcvNULL) + 1;
       derivedTypeVariable->typeNameLength = nameLength;
       slmSLINK_LIST_InsertLast(CodeGenerator->derivedTypeVariables, &derivedTypeVariable->node);
       derivedType->u.typeInfo.typeNameOffset = CodeGenerator->derivedTypeNameBufferSize;
       CodeGenerator->derivedTypeNameBufferSize += nameLength;
   }

   SetUniformTypeNameOffset(Uniform, derivedType->u.typeInfo.typeNameOffset);

   SetFormatSpecialType(Uniform->format, _ConvDerivedTypeToFormat(derivedType));
   return;
}

#if cldSupportMultiKernelFunction
static gceSTATUS
_NewBlockIntermediateElementSymbol(
    IN cloCOMPILER Compiler,
    IN clsNAME * Name,
    IN gctCONST_STRING Symbol,
    IN clsDECL *Decl,
    IN clsARRAY *Array,
    IN gcsINTERFACE_BLOCK_INFO  *InterfaceBlock,
    IN gcSHADER_VAR_CATEGORY VarCategory,
    IN gctUINT16 NumStructureElement,
    IN gctINT16 Parent,
    IN gctINT16 PrevSibling,
    OUT gctINT16* ThisVarIndex
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER();
    if(isIBIUniformBlock(InterfaceBlock))
    {
        gcUNIFORM uniform;
        clmNewUniform(Compiler,
                      Name,
                      Symbol,
                      Decl,
                      -1,
                      Array,
                      VarCategory,
                      NumStructureElement,
                      Parent,
                      PrevSibling,
                      uniform,
                      ThisVarIndex,
                      status);
        if (gcmIS_ERROR(status))
        {
           gcmFOOTER();
           return status;
        }
        uniform->blockIndex = GetIBIBlockIndex(InterfaceBlock);
    }
    else
    {
        gcmASSERT(0);
        status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
    }
    gcmFOOTER_NO();
    return status;
}

static gceSTATUS
_AllocStructElementAggregatedSymbol(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN clsDECL * StructDecl,
    IN gctUINT ArrayIdx,
    IN gctCONST_STRING StructSymbol,
    IN gctCONST_STRING FieldSymbol,
    OUT gctSTRING *Symbol
    )
{
    gceSTATUS       status;
    gctSIZE_T       symbolLength = 0, fieldLength = 0, len;
    gctSTRING       symbol = gcvNULL;
    gctUINT         offset;
    gctPOINTER      pointer = gcvNULL;
    gctBOOL         skipArrayIndex = (gctINT)ArrayIdx == -1 ? gcvTRUE : gcvFALSE;

    gcmHEADER_ARG("Compiler=0x%x CodeGenerator=0x%x StructDecl=0x%x ArrayIdx=%d"
                  "StructSymbol=%s FieldSymbol=%s Symbol=0x%x",
                  Compiler, CodeGenerator, StructDecl, ArrayIdx, StructSymbol, FieldSymbol, Symbol);

    symbolLength = gcoOS_StrLen(StructSymbol, gcvNULL);

    if (FieldSymbol)
    {
        fieldLength = gcoOS_StrLen(FieldSymbol, gcvNULL);
    }

    len = symbolLength + fieldLength + 20;

    status = cloCOMPILER_Allocate(Compiler,
                                  len,
                                  &pointer);

    if (gcmIS_ERROR(status))
    {
        if (Symbol) *Symbol = gcvNULL;

        gcmFOOTER();
        return status;
    }

    symbol = pointer;

    if (!clmDECL_IsArray(StructDecl) || skipArrayIndex)
    {
        offset = 0;
        if (FieldSymbol)
            gcmVERIFY_OK(gcoOS_PrintStrSafe(symbol, len,
                                            &offset,
                                            "%s.%s",
                                            StructSymbol,
                                            FieldSymbol));
        else
            gcmVERIFY_OK(gcoOS_PrintStrSafe(symbol, len,
                                            &offset,
                                            "%s",
                                            StructSymbol));

    }
    else
    {
        offset = 0;
        if (FieldSymbol)
            gcmVERIFY_OK(gcoOS_PrintStrSafe(symbol, len,
                                            &offset,
                                            "%s[%d].%s",
                                            StructSymbol,
                                            ArrayIdx,
                                            FieldSymbol));
        else
            gcmVERIFY_OK(gcoOS_PrintStrSafe(symbol, len,
                                            &offset,
                                            "%s[%d]",
                                            StructSymbol,
                                            ArrayIdx));
    }

    if (Symbol)
        *Symbol = symbol;

    gcmFOOTER_ARG("*Symbol=%s", *Symbol);
    return gcvSTATUS_OK;
}

static gceSTATUS
_FreeStructElementAggregatedSymbol(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN gctSTRING Symbol
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gcmHEADER_ARG("Compiler=0x%x CodeGenerator=0x%x Symbol=%s",
                  Compiler, CodeGenerator, Symbol);

    gcmVERIFY_OK(cloCOMPILER_Free(Compiler, Symbol));

    gcmFOOTER();
    return status;
}

static gceSTATUS
_AllocMemoryOffsets(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN clsNAME * Name,
    IN gctCONST_STRING Symbol,
    IN clsDECL * Decl,
    IN gcsINTERFACE_BLOCK_INFO  *InterfaceBlock,
    IN gcUNIFORM StructParent,
    IN OUT gctINT16 *PrevSibling,
    IN OUT gctINT32 *Offset
    );

static gceSTATUS
_AllocMemoryOffsetsForNormalStruct(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN clsNAME * Name,
    IN gctCONST_STRING Symbol,
    IN clsDECL * Decl,
    IN gcsINTERFACE_BLOCK_INFO *InterfaceBlock,
    IN gctINT16 ParentIndex,
    IN OUT gctINT16 *PrevSibling,
    IN OUT gctINT32 *Offset
    )
{
    gceSTATUS       status;
    gctUINT         count, i;
    clsNAME *       fieldName;
    gctSTRING       symbol = gcvNULL;
    gctINT16        mainIdx = -1, structEleParent;
    gctINT16        arrayElePrevSibling;
    gctINT16        structElePrevSibling;
    gctUINT16       structEleCount;
    gcsINTERFACE_BLOCK_INFO  *interfaceBlock = gcvNULL;
    gcUNIFORM structParent;
    gcSHADER binary;
    gctUINT alignment = 1;
    gcUNIFORM mainParent;

    interfaceBlock = InterfaceBlock;

    gcmASSERT(Name->decl.dataType->u.fieldSpace);
    gcmVERIFY_OK(cloCOMPILER_GetBinary(Compiler, &binary));

    mainParent = gcvNULL;

    if (clmDECL_IsArray(Decl)) {
       clmGetArrayElementCount(Decl->array, 0, count);
    }
    else count = 1;

    slsDLINK_NODE_COUNT(&Decl->dataType->u.fieldSpace->names, structEleCount);

    status = _NewBlockIntermediateElementSymbol(Compiler,
                                                Name,
                                                Symbol,
                                                Decl,
                                                &Decl->array,
                                                interfaceBlock,
                                                gcSHADER_VAR_CATEGORY_STRUCT,
                                                (count > 1) ? 0 : structEleCount,
                                                ParentIndex,
                                                *PrevSibling,
                                                &mainIdx);
    if (gcmIS_ERROR(status)) return status;

    if(GetIBIFirstChild(interfaceBlock) == -1) {
        SetIBIFirstChild(interfaceBlock, mainIdx);
    }

    arrayElePrevSibling = -1;

    gcmVERIFY_OK(_GetBaseAlignmentForStruct(Compiler, CodeGenerator, Decl, &alignment));

    for (i = 0; i < count; i++)
    {
        gcUNIFORM structChild;
        structEleParent = mainIdx;

        /* If this interface block member is a structure, we need to update the offset. */
        *Offset = gcmALIGN(*Offset, alignment);

        if (count > 1)
        {
            gcmVERIFY_OK(_AllocStructElementAggregatedSymbol(Compiler,
                                                             CodeGenerator,
                                                             Decl,
                                                             i,
                                                             Symbol,
                                                             gcvNULL,
                                                             &symbol));

            status = _NewBlockIntermediateElementSymbol(Compiler,
                                                        Name,
                                                        symbol,
                                                        Decl,
                                                        (clsARRAY *)0,
                                                        interfaceBlock,
                                                        gcSHADER_VAR_CATEGORY_STRUCT,
                                                        structEleCount,
                                                        mainIdx,
                                                        arrayElePrevSibling,
                                                        &arrayElePrevSibling);

            if (gcmIS_ERROR(status)) return status;

            structEleParent = arrayElePrevSibling;

            gcmVERIFY_OK(_FreeStructElementAggregatedSymbol(Compiler, CodeGenerator, symbol));

            clmGetUniform(binary, mainIdx, mainParent, status);
            if (gcmIS_ERROR(status)) return status;
        }

        clmGetUniform(binary, structEleParent, structParent, status);
        if (gcmIS_ERROR(status)) return status;

        structParent->offset = -1;

        structElePrevSibling = -1;

        FOR_EACH_DLINK_NODE(&Decl->dataType->u.fieldSpace->names, clsNAME, fieldName)
        {
            gcmASSERT(fieldName->decl.dataType);
            gcmVERIFY_OK(_AllocStructElementAggregatedSymbol(Compiler,
                                                             CodeGenerator,
                                                             Decl,
                                                             i,
                                                             Symbol,
                                                             fieldName->symbol,
                                                             &symbol));

            status = _AllocMemoryOffsets(Compiler,
                                         CodeGenerator,
                                         Name,
                                         symbol,
                                         &fieldName->decl,
                                         interfaceBlock,
                                         structParent,
                                         &structElePrevSibling,
                                         Offset);
            if (gcmIS_ERROR(status)) return status;

            gcmVERIFY_OK(_FreeStructElementAggregatedSymbol(Compiler, CodeGenerator, symbol));

            /* The offset of a struct is equal to the offset of its first member. */
            if(structParent->offset == -1 )
            {
                clmGetUniform(binary, structElePrevSibling, structChild, status);
                if (gcmIS_ERROR(status)) return status;

                if (isUniformArray(structChild) && structChild->arraySize > 1)
                {
                    gcmASSERT(structChild->firstChild != -1);

                    clmGetUniform(binary, structChild->firstChild, structChild, status);
                    gcmVERIFY_OK(status);
                }

                structParent->offset = structChild->offset;
            }

            if(mainParent && i == 0)
            {
                mainParent->offset = structParent->offset;
            }
        }
        if(i == 0)
        {
            if(mainParent && isUniformArray(mainParent))
            {
                mainParent->arrayStride = gcmALIGN(*Offset, alignment) - mainParent->offset;
            }
        }

        /* add struct padding part */
        *Offset = gcmALIGN(*Offset, alignment);
    }
    *PrevSibling = mainIdx;

    return status;
}

static gceSTATUS
_AllocMemoryOffsetsForStruct(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN clsNAME * Name,
    IN gctCONST_STRING Symbol,
    IN clsDECL * Decl,
    IN gcsINTERFACE_BLOCK_INFO  *InterfaceBlock,
    IN gctINT16 ParentIndex,
    IN OUT gctINT16 *PrevSibling,
    IN OUT gctINT32 *Offset
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gcSHADER binary = gcvNULL;

    if (Decl->array.numDim < 2)
    {
        status = _AllocMemoryOffsetsForNormalStruct(Compiler,
                                                    CodeGenerator,
                                                    Name,
                                                    Symbol,
                                                    Decl,
                                                    InterfaceBlock,
                                                    ParentIndex,
                                                    PrevSibling,
                                                    Offset);
        if (gcmIS_ERROR(status))
        {
            return status;
        }
    }
    else
    {
        clsDECL elementDecl[1];
        gctINT i, arrayLength, count;
        gctSTRING symbol = gcvNULL;
        gctINT16 parent = 0;
        gctINT16 prevSibling = -1;
        gcUNIFORM parentUniform = gcvNULL, childUniform = gcvNULL;
        gcsINTERFACE_BLOCK_INFO  *interfaceBlock = InterfaceBlock;

        status = _NewBlockIntermediateElementSymbol(Compiler,
                                                    Name,
                                                    Symbol,
                                                    Decl,
                                                    &Decl->array,
                                                    interfaceBlock,
                                                    gcSHADER_VAR_CATEGORY_STRUCT,
                                                    0,
                                                    ParentIndex,
                                                    *PrevSibling,
                                                    &parent);

        if(GetIBIFirstChild(interfaceBlock) == -1) {
            SetIBIFirstChild(interfaceBlock, parent);
        }

        if (gcmIS_ERROR(status)) return status;

        if (clmDECL_IsArray(Decl)) {
           clmGetArrayElementCount(Decl->array, 0, arrayLength);
        }
        else arrayLength = 1;

        clmDECL_Initialize(elementDecl, Decl->dataType, (clsARRAY *) 0, Decl->ptrDscr, Decl->ptrDominant, Decl->storageQualifier);
        for (i = 0; i < arrayLength; i++)
        {
            gcmVERIFY_OK(_AllocStructElementAggregatedSymbol(Compiler,
                                                             CodeGenerator,
                                                             Decl,
                                                             i,
                                                             Symbol,
                                                             gcvNULL,
                                                             &symbol));

            if (gcmIS_ERROR(status)) return status;

            gcmVERIFY_OK(_AllocMemoryOffsetsForStruct(Compiler,
                                                      CodeGenerator,
                                                      Name,
                                                      symbol,
                                                      elementDecl,
                                                      interfaceBlock,
                                                      parent,
                                                      &prevSibling,
                                                      Offset));

            gcmVERIFY_OK(_FreeStructElementAggregatedSymbol(Compiler, CodeGenerator, symbol));
        }

        gcmVERIFY_OK(cloCOMPILER_GetBinary(Compiler, &binary));

        clmGetUniform(binary, parent, parentUniform, status);
        gcmVERIFY_OK(status);

        clmGetUniform(binary, parentUniform->firstChild, childUniform, status);
        gcmVERIFY_OK(status);

        clmGetArrayElementCount(Decl->array, 1, count);
        SetUniformArrayStride(parentUniform, count * childUniform->arrayStride);
        SetUniformOffset(parentUniform, childUniform->offset);

        if (PrevSibling)
        {
            *PrevSibling = parent;
        }
    }

    return status;
}

static gctINT32
_GetDataTypeByteOffset(
    IN gctINT32 BaseOffset,
    IN clsDECL * Decl,
    OUT gctINT32 *ArrayStride
)
{
   gctSIZE_T size;
   gctUINT alignment;
   gctBOOL packed;

   size = clsDECL_GetElementByteSize(Decl,
                                     &alignment,
                                     &packed);

   if(ArrayStride) *ArrayStride = size;
   return  clmALIGN(BaseOffset, alignment, packed);
}

static gceSTATUS
_AllocMemoryOffsetOrArray(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN clsNAME * MemberName,
    IN gctCONST_STRING Symbol,
    IN clsDECL * Decl,
    IN gcsINTERFACE_BLOCK_INFO  *InterfaceBlock,
    IN gcUNIFORM StructParent,
    IN OUT gctINT16 *PrevSibling,
    IN OUT gctINT32 *Offset
    )
{
    gceSTATUS          status;
    gctUINT            logicalRegCount;
    gcUNIFORM          member = gcvNULL;
    gcUNIFORM          structParent;
    clsGEN_CODE_DATA_TYPE binaryDataType;
    gctINT16 thisVarIndex = -1;
    gctINT32 offset, arrayStride;
    gctINT isArray = clmDECL_IsArray(Decl);

    gcmHEADER_ARG("Compiler=0x%x CodeGenerator=0x%x MemberName=0x%x Symbol=%s "
                  "Decl=0x%x InterfaceBlock=0x%x StructParent=0x%x",
                  Compiler, CodeGenerator, MemberName, Symbol, Decl, InterfaceBlock, StructParent);

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_OBJECT(CodeGenerator, clvOBJ_CODE_GENERATOR);
    gcmVERIFY_ARGUMENT(MemberName);
    gcmASSERT(Decl);
    gcmASSERT(!clmDATA_TYPE_IsStructOrUnion(Decl->dataType));
    gcmASSERT(Offset);

    logicalRegCount = _GetLogicalOperandCount(Decl);

    if(isIBIUniformBlock(InterfaceBlock))
    {
        clmNewUniform(Compiler,
                      MemberName,
                      Symbol,
                      Decl,
                      -1,
                      clmDECL_IsArray(Decl) ? &Decl->array : (clsARRAY *) 0,
                      gcSHADER_VAR_CATEGORY_BLOCK_MEMBER,
                      0,
                      StructParent ? StructParent->index : -1,
                      *PrevSibling,
                      member,
                      &thisVarIndex,
                      status);
        if (gcmIS_ERROR(status))
        {
           gcmFOOTER();
           return status;
        }
        member->blockIndex = GetIBIBlockIndex(InterfaceBlock);
    }
    else
    {
        gcmASSERT(0);
        status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
    }

    if (gcmIS_ERROR(status))
    {
        gcmFOOTER();
        return status;
    }

    structParent = StructParent;

    *PrevSibling = thisVarIndex;

    member->blockIndex = GetIBIBlockIndex(InterfaceBlock);

    offset = _GetDataTypeByteOffset(*Offset,
                                    Decl,
                                    &arrayStride);
    member->offset = offset;
    member->arrayStride = arrayStride;

    if(structParent &&
       isUniformStruct(structParent)) {
       if(structParent->offset == -1) {
           structParent->offset = offset;
       }
       if(structParent->firstChild == -1) {
           structParent->firstChild = thisVarIndex;
       }
    }

    binaryDataType = _ConvElementDataType(Decl);

    if(isArray) {
        arrayStride *= logicalRegCount;
    }

    *Offset = offset + arrayStride;

    if(GetIBIFirstChild(InterfaceBlock) == -1) {
        gcmASSERT(thisVarIndex != -1);
        SetIBIFirstChild(InterfaceBlock, thisVarIndex);
    }

    gcmFOOTER_ARG("*PrevSibling=%d *Offset=%u", *PrevSibling, *Offset);
    return gcvSTATUS_OK;
}

static gceSTATUS
_AllocMemoryOffsets(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN clsNAME * Name,
    IN gctCONST_STRING Symbol,
    IN clsDECL * Decl,
    IN gcsINTERFACE_BLOCK_INFO  *InterfaceBlock,
    IN gcUNIFORM StructParent,
    IN OUT gctINT16 *PrevSibling,
    IN OUT gctINT32 *Offset
    )
{
    gceSTATUS       status;
    gcsINTERFACE_BLOCK_INFO  *interfaceBlock = gcvNULL;
    gctINT16        structParentIndex = -1;

    gcmHEADER_ARG("Compiler=0x%x codeGenerator=0x%x Name=0x%x Symbol=%s "
                  "Decl=0x%x InterfaceBlock=0x%x StructParent=0x%x",
                  Compiler, CodeGenerator, Name, Symbol, Decl, InterfaceBlock, StructParent);

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_OBJECT(CodeGenerator, clvOBJ_CODE_GENERATOR);
    gcmVERIFY_ARGUMENT(Name);
    gcmASSERT(Decl);

    gcmASSERT(InterfaceBlock);
    interfaceBlock = InterfaceBlock;
    if(!interfaceBlock) {
        gcmFOOTER_ARG("status=%d", gcvSTATUS_COMPILER_FE_PARSER_ERROR);
        return gcvSTATUS_COMPILER_FE_PARSER_ERROR;
    }

    structParentIndex = StructParent ? ((gcVARIABLE)StructParent)->index : -1;

    if (clmDECL_IsUnderlyingStructOrUnion(Decl))
    {
        status = _AllocMemoryOffsetsForStruct(Compiler,
                                              CodeGenerator,
                                              Name,
                                              Symbol,
                                              Decl,
                                              interfaceBlock,
                                              structParentIndex,
                                              PrevSibling,
                                              Offset);

        if (gcmIS_ERROR(status))
        {
            gcmFOOTER();
            return status;
        }
    }
    else
    {
        status = _AllocMemoryOffsetOrArray(Compiler,
                                           CodeGenerator,
                                           Name,
                                           Symbol,
                                           Decl,
                                           interfaceBlock,
                                           StructParent,
                                           PrevSibling,
                                           Offset);

        if (gcmIS_ERROR(status))
        {
            gcmFOOTER();
            return status;
        }
    }

    gcmFOOTER_ARG("*PrevSibling=%d *Offset=%u", *PrevSibling, *Offset);
    return gcvSTATUS_OK;
}

static gceSTATUS
_AllocLogicalRegForInterfaceBlock(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN clsNAME * VarName
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gcsSHADER_VAR_INFO blockInfo[1];
    gctINT32 offset = 0;
    gctCONST_STRING symbol = gcvNULL;
    gcSHADER binary;
    gcsINTERFACE_BLOCK_INFO *interfaceBlock;

    gcmHEADER_ARG("Compiler=0x%x CodeGenerator=0x%x VarName=0x%x",
                  Compiler, CodeGenerator, VarName);

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_OBJECT(CodeGenerator, clvOBJ_CODE_GENERATOR);
    gcmVERIFY_ARGUMENT(VarName);

    if(VarName->u.variableInfo.inInterfaceBlock) {
        gcmFOOTER_NO();
        return gcvSTATUS_OK;
    }

    gcmVERIFY_OK(cloCOMPILER_GetBinary(Compiler, &binary));


    /* create uniform block */
    if(CodeGenerator->uniformBlock == gcvNULL) {
        clsNAME *constantAddrSpace;
        gctINT blockAddrIndex;
        gcUNIFORM blockAddrUniform;

        gcoOS_ZeroMemory(blockInfo, gcmSIZEOF(gcsSHADER_VAR_INFO));

        blockInfo->varCategory = gcSHADER_VAR_CATEGORY_BLOCK;
        blockInfo->format = gcSL_FLOAT;
        blockInfo->precision = gcSHADER_PRECISION_DEFAULT;
        blockInfo->arraySize = 1;
        blockInfo->u.numBlockElement = 0;
        blockInfo->parent= -1;

        blockInfo->prevSibling = -1;

        constantAddrSpace = cloCOMPILER_GetBuiltinVariable(Compiler, clvBUILTIN_CONSTANT_ADDRESS_SPACE);
        if (gcmIS_ERROR(status)) {
            gcmFOOTER();
            return status;
        }

        gcmVERIFY_OK(clGetBuiltinVariableImplSymbol(Compiler, constantAddrSpace, "", &symbol));
        status = gcSHADER_AddUniformBlock(binary,
                                          symbol,
                                          blockInfo,
                                          gcvINTERFACE_BLOCK_NONE,
                                          -1,
                                          0,
                                          &CodeGenerator->uniformBlock);
        if (gcmIS_ERROR(status)) {
            gcmFOOTER();
            return status;
        }

        blockAddrIndex = GetUBIndex(CodeGenerator->uniformBlock);
        clmGetUniform(binary, blockAddrIndex, blockAddrUniform, status);
        if (gcmIS_ERROR(status)) {
            gcmFOOTER();
            return status;
        }
        gcUNIFORM_SetFlags(blockAddrUniform, gcvUNIFORM_KIND_CONSTANT_ADDRESS_SPACE);

        gcmASSERT(constantAddrSpace->context.u.variable.logicalRegCount == 0);
        constantAddrSpace->context.u.variable.logicalRegCount = 1;
        status = cloCOMPILER_Allocate(Compiler,
                                      (gctSIZE_T)sizeof(clsLOGICAL_REG),
                                      (gctPOINTER *) &constantAddrSpace->context.u.variable.logicalRegs);
        if (gcmIS_ERROR(status)) {
            gcmFOOTER();
            return status;
        }

        clsLOGICAL_REG_InitializeUniform(constantAddrSpace->context.u.variable.logicalRegs,
                                         clvQUALIFIER_UNIFORM,
                                         clmGenCodeDataType(T_UINT),
                                         blockAddrUniform,
                                         0,
                                         gcvFALSE);
    }

    interfaceBlock = GetUBInterfaceBlockInfo(CodeGenerator->uniformBlock);
    gcmASSERT(interfaceBlock);
    offset = GetIBIBlockSize(interfaceBlock);
    gcmASSERT(VarName->decl.dataType);

    symbol = VarName->symbol;

    status = _AllocMemoryOffsets(Compiler,
                                 CodeGenerator,
                                 VarName,
                                 symbol,
                                 &VarName->decl,
                                 interfaceBlock,
                                 gcvNULL,
                                 &CodeGenerator->currentUniformBlockMember,
                                 &offset);
    if (gcmIS_ERROR(status)) {
        gcmFOOTER();
        return status;
    }

    SetIBIBlockSize(interfaceBlock, offset);

    VarName->u.variableInfo.inInterfaceBlock = gcvTRUE;

    gcmFOOTER();
    return status;
}

static gceSTATUS
_AllocLogicalRegOrArray(
IN cloCOMPILER Compiler,
IN cloCODE_GENERATOR CodeGenerator,
IN clsNAME * Name,
IN clsNAME * ParentName,
IN gctCONST_STRING Symbol,
IN clsDECL * Decl,
OUT clsLOGICAL_REG * LogicalRegs,
IN OUT gctUINT * Start,
IN OUT gctUINT * Available,
OUT gctUINT *NumRegNeeded
)
{
    gceSTATUS    status;
    cltQUALIFIER accessQualifier;
    cltQUALIFIER orgAccessQualifier;
    cltQUALIFIER addrSpaceQualifier;
    cltQUALIFIER storageQualifier;
    clsGEN_CODE_DATA_TYPE binaryDataType;
    gctSIZE_T    binaryDataTypeRegSize;
    gctUINT      logicalRegCount, i;
    gctREG_INDEX tempRegIndex;
    gcUNIFORM    uniform;
    gctBOOL      isUniformForAddressSpace = gcvFALSE;
    gcATTRIBUTE  attribute;
    gcVARIABLE   variable = gcvNULL;
    gctUINT numRegNeeded;
    gctUINT available = Available ? *Available: 0;
    gctSIZE_T    regOffset;

    /*gcmHEADER_ARG("Compiler=0x%x CodeGenerator=0x%x Name=0x%x Symbol=%s "
              "DataType=0x%x",
              Compiler, CodeGenerator, Name, Symbol?Symbol:"",
              Decl);*/
    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_OBJECT(CodeGenerator, clvOBJ_CODE_GENERATOR);
    gcmASSERT(Name);
    gcmASSERT(Decl && Decl->dataType);
    gcmASSERT(LogicalRegs);
    gcmASSERT(Start);

    if (Name->isBuiltin && Name->type == clvVARIABLE_NAME) {
        gcmVERIFY_OK(clGetBuiltinVariableImplSymbol(Compiler, Name, Symbol, &Symbol));
    }

    binaryDataType        = _ConvElementDataTypeForRegAlloc(Name);
    binaryDataTypeRegSize    = gcGetDataTypeRegSize(binaryDataType);
    regOffset = _clmGetTempRegIndexOffset(binaryDataTypeRegSize,
                                          binaryDataType.elementType);

    addrSpaceQualifier = Name->decl.dataType->addrSpaceQualifier;
    storageQualifier = Name->decl.storageQualifier;
    if(Name->type != clvPARAMETER_NAME &&
       !clmDECL_IsPointerType(&Name->decl) &&
       clmGEN_CODE_checkVariableForMemory(Name)) {
       orgAccessQualifier = clvQUALIFIER_UNIFORM;
       isUniformForAddressSpace = gcvTRUE;
       logicalRegCount = 1;
    }
    else  {
       orgAccessQualifier = Name->decl.dataType->accessQualifier;
       if(!clmGEN_CODE_checkVariableForMemory(Name) &&
          clmDECL_IsArray(Decl)) {
          clmGetArrayElementCount(Decl->array, 0, logicalRegCount);
       }
       else logicalRegCount = 1;
    }

    if(isUniformForAddressSpace ||
       (cloCOMPILER_IsExternSymbolsAllowed(Compiler) &&
        !clmDECL_IsPointerType(&Name->decl) &&
        (Name->type == clvVARIABLE_NAME || Name->type == clvPARAMETER_NAME) &&
        Name->decl.dataType->addrSpaceQualifier == clvQUALIFIER_CONSTANT)) {

        status = cloCOMPILER_AllocateVariableMemory(Compiler,
                                                    Name);
        if (gcmIS_ERROR(status)) return status;

        if (Name->type == clvVARIABLE_NAME && clmIsVariableInConstantAddressSpaceMemory(Name)) {
            status =  _AllocLogicalRegForInterfaceBlock(Compiler,
                                                        CodeGenerator,
                                                        Name);
            if (gcmIS_ERROR(status)) return status;
        }
    }

    *NumRegNeeded = 0;

    accessQualifier = _MapQualifierToShaderQualifier(orgAccessQualifier);
    switch (accessQualifier) {
    case clvQUALIFIER_NONE:
    case clvQUALIFIER_CONST:
    case clvQUALIFIER_READ_ONLY:
    case clvQUALIFIER_WRITE_ONLY:
    case clvQUALIFIER_CONST_IN:
    case clvQUALIFIER_OUT:
        numRegNeeded = logicalRegCount * binaryDataTypeRegSize;
        if(numRegNeeded > available) {
           tempRegIndex = clNewTempRegs(Compiler,
                                        numRegNeeded - available,
                                        binaryDataType.elementType);
           tempRegIndex -= (gctREG_INDEX) available;
           if(Available) {
             *Available = numRegNeeded;
           }
        }
        else {
           tempRegIndex = clNewTempRegs(Compiler,
                                        0,
                                        binaryDataType.elementType);
           tempRegIndex -= (gctREG_INDEX) available;
        }
        *NumRegNeeded = numRegNeeded;

        for (i = 0; i < logicalRegCount; i++) {
            clsLOGICAL_REG_InitializeTemp(LogicalRegs + *Start + i,
                                          accessQualifier,
                                          binaryDataType,
                                          tempRegIndex + (gctREG_INDEX)(i * regOffset),
                                          (i < available) ? gcvTRUE : gcvFALSE);
        }

        if (ParentName->type == clvFUNC_NAME || ParentName->type == clvKERNEL_FUNC_NAME
            || ParentName->type == clvPARAMETER_NAME) {
            gctUINT8 argumentQualifier = gcvFUNCTION_INPUT;

            switch (accessQualifier) {
            case clvQUALIFIER_NONE:
            case clvQUALIFIER_CONST_IN:
                argumentQualifier = gcvFUNCTION_INPUT;
                break;

            case clvQUALIFIER_OUT:
                argumentQualifier = gcvFUNCTION_OUTPUT;
                break;
            }

            if(ParentName->context.u.variable.isKernel) {
               gcmASSERT(ParentName->context.u.variable.u.kernelFunction);

               status = clNewKernelFunctionArgument(Compiler,
                                                    ParentName->context.u.variable.u.kernelFunction,
                                                    gcvNULL,
                                                    binaryDataType,
                                                    logicalRegCount,
                                                    tempRegIndex,
                                                    argumentQualifier);
               if (gcmIS_ERROR(status)) return status;
            }
            else {
               gcmASSERT(ParentName->context.u.variable.u.function);

               if(ParentName->type == clvPARAMETER_NAME) {
                   gctSIZE_T length, functionLength, totalLength;
                   gctUINT offset = 0;
                   gctPOINTER pointer = gcvNULL;
                   gctSTRING symbol = gcvNULL;
                   gctSTRING funcName;
                   gcoOS_StrLen(Symbol, &length);

                   gcmASSERT(ParentName->context.u.variable.u.function);
                   funcName = ParentName->context.u.variable.u.function->name;
                   if(length > 0 && !clGetBuiltinFunctionInfo(funcName)) {
                       gcoOS_StrLen(funcName, &functionLength);

                       totalLength = length + functionLength + 10;

                       status = cloCOMPILER_Allocate(Compiler,
                                                     totalLength,
                                                     &pointer);
                       if (gcmIS_ERROR(status))  return status;
                       symbol = pointer;

                       gcmVERIFY_OK(gcoOS_PrintStrSafe(symbol, totalLength,
                                                       &offset,
                                                       (argumentQualifier == gcvFUNCTION_INPUT) ? ("%s_input_%s") :
                                                       (argumentQualifier == gcvFUNCTION_OUTPUT ? ("%s_output_%s") : ("%s_inout_%s")),
                                                       funcName,
                                                       Symbol));

                       status = clNewVariable(Compiler,
                                              ParentName->lineNo,
                                              ParentName->stringNo,
                                              symbol,
                                              orgAccessQualifier,
                                              addrSpaceQualifier,
                                              storageQualifier,
                                              binaryDataType,
                                              logicalRegCount,
                                              tempRegIndex,
                                              &variable);
                       if (gcmIS_ERROR(status)) return status;

                       if (symbol != gcvNULL) {
                           gcmVERIFY_OK(cloCOMPILER_Free(Compiler, symbol));
                       }
                   }
               }
               else variable = gcvNULL;
               status = clNewFunctionArgument(Compiler,
                                              ParentName->context.u.variable.u.function,
                                              variable,
                                              binaryDataType,
                                              logicalRegCount,
                                              tempRegIndex,
                                              argumentQualifier);
               if (gcmIS_ERROR(status)) return status;
            }
        }
        else {
            status = clNewVariable(Compiler,
                                   ParentName->lineNo,
                                   ParentName->stringNo,
                                   Symbol,
                                   orgAccessQualifier,
                                   addrSpaceQualifier,
                                   storageQualifier,
                                   binaryDataType,
                                   logicalRegCount,
                                   tempRegIndex,
                                   &variable);
            if (gcmIS_ERROR(status)) return status;
            if(storageQualifier & clvSTORAGE_QUALIFIER_EXTERN) {
                gcSHADER shader;

                /* set shader to have extern variables */
                gcmVERIFY_OK(cloCOMPILER_GetBinary(Compiler, &shader));
                SetShaderHasExternVariable(shader, gcvTRUE);
                SetVariableIsExtern(variable);
            }
            else if(storageQualifier & clvSTORAGE_QUALIFIER_STATIC) {
                SetVariableIsStatic(variable);
            }
            if(clmDECL_IsPointerType(&Name->decl)) {
                SetVariableIsPointer(variable);
            }
            else if(addrSpaceQualifier == clvQUALIFIER_NONE ||
                    addrSpaceQualifier == clvQUALIFIER_PRIVATE ||
                    addrSpaceQualifier == clvQUALIFIER_LOCAL) {
                SetVariableIsLocal(variable);
            }
        }
        break;

    case clvQUALIFIER_UNIFORM:
        if(isUniformForAddressSpace) {
           gctINT arrayStride;

           switch(addrSpaceQualifier) {
           case clvQUALIFIER_GLOBAL:
           case clvQUALIFIER_PRIVATE:
               tempRegIndex = cldPrivateMemoryAddressRegIndex;
               break;

           case clvQUALIFIER_LOCAL:
               tempRegIndex = cldLocalMemoryAddressRegIndex;
               break;

           case clvQUALIFIER_CONSTANT:
               tempRegIndex = cldConstantMemoryAddressRegIndex;
               break;

           default:
               if ((Name)->type == clvFUNC_NAME && clmDECL_IsAggregateType(&((Name)->decl)))
               {
                   tempRegIndex = clNewTempRegs(Compiler, 1,
                                (Name)->decl.dataType->elementType);
               }
               else
               {
                    tempRegIndex = cldPrivateMemoryAddressRegIndex;
               }
               break;
           }
           gcmASSERT(logicalRegCount == 1);
           clsLOGICAL_REG_InitializeTemp(LogicalRegs + *Start,
                                         clvQUALIFIER_NONE,
                                         binaryDataType,
                                         tempRegIndex,
                                         gcvFALSE);
           status = clNewVariable(Compiler,
                                  Name->lineNo,
                                  Name->stringNo,
                                  Symbol,
                                  orgAccessQualifier,
                                  addrSpaceQualifier,
                                  storageQualifier,
                                  binaryDataType,
                                  logicalRegCount,
                                  tempRegIndex,
                                  &variable);
           if (gcmIS_ERROR(status)) return status;

           arrayStride = clsDECL_GetByteSize(&Name->decl);
           arrayStride /= GetVariableKnownArraySize(variable);

           SetVariableArrayStride(variable, arrayStride);
           SetVariableOffset(variable, clmNAME_VariableMemoryOffset_GET(Name));
           if(storageQualifier & clvSTORAGE_QUALIFIER_EXTERN) {
               gcSHADER shader;

               /* set shader to have extern variables */
               gcmVERIFY_OK(cloCOMPILER_GetBinary(Compiler, &shader));
               SetShaderHasExternVariable(shader, gcvTRUE);
               SetVariableIsExtern(variable);
           }
           else if(storageQualifier & clvSTORAGE_QUALIFIER_STATIC) {
               SetVariableIsStatic(variable);
           }
           if(clmDECL_IsPointerType(&Name->decl)) {
               SetVariableIsPointer(variable);
           }
           else if(addrSpaceQualifier == clvQUALIFIER_NONE ||
                   addrSpaceQualifier == clvQUALIFIER_PRIVATE ||
                   addrSpaceQualifier == clvQUALIFIER_LOCAL) {
               SetVariableIsLocal(variable);
           }
        }
        else {
            clsGEN_CODE_DATA_TYPE format;

            clmGEN_CODE_ConvDirectElementDataType(Decl->dataType, format);
            status = clNewUniform(Compiler,
                                  Name->lineNo,
                                  Name->stringNo,
                                  Symbol,
                                  binaryDataType,
                                  format,
                                  Name->u.variableInfo.variableType,
                                  clmDECL_IsPointerType(Decl) || clmGEN_CODE_checkVariableForMemory(Name),
                                  logicalRegCount,
                                  &uniform);
            if (gcmIS_ERROR(status)) return status;
            SetUniformQualifier(uniform, clConvToShaderTypeQualifier(accessQualifier));
            SetUniformQualifier(uniform, clConvToShaderTypeQualifier(addrSpaceQualifier));
            SetUniformQualifier(uniform, clConvStorageQualifierToShaderTypeQualifier(storageQualifier));
            _SetPointerUniformQualifiers(uniform, Decl);
            _AddDerivedTypeVariable(Compiler,
                                    CodeGenerator,
                                    Name,
                                    uniform);

            for (i = 0; i < logicalRegCount; i++) {
                clsLOGICAL_REG_InitializeUniform(LogicalRegs + *Start + i,
                                                 accessQualifier,
                                                 binaryDataType,
                                                 uniform,
                                                 (gctREG_INDEX)(i * regOffset),
                                                 (i < available) ? gcvTRUE : gcvFALSE);
            }
        }
        break;

    case clvQUALIFIER_ATTRIBUTE:
        status = clNewAttribute(Compiler,
                                Name->lineNo,
                                Name->stringNo,
                                Symbol,
                                binaryDataType,
                                logicalRegCount,
                                gcvFALSE,
                                &attribute);

        if (gcmIS_ERROR(status)) return status;

        for (i = 0; i < logicalRegCount; i++) {
            clsLOGICAL_REG_InitializeAttribute(LogicalRegs + *Start + i,
                                               accessQualifier,
                                               binaryDataType,
                                               attribute,
                                               (gctREG_INDEX)(i * regOffset),
                                               (i < available) ? gcvTRUE : gcvFALSE);
        }
        break;

    default:
        gcmASSERT(0);
        return gcvSTATUS_INVALID_ARGUMENT;
    }

    (*Start) += logicalRegCount;
    return gcvSTATUS_OK;
}

#else
static gceSTATUS
_AllocLogicalRegOrArray(
IN cloCOMPILER Compiler,
IN cloCODE_GENERATOR CodeGenerator,
IN clsNAME * Name,
IN clsNAME * ParentName,
IN gctCONST_STRING Symbol,
IN clsDECL * Decl,
OUT clsLOGICAL_REG * LogicalRegs,
IN OUT gctUINT * Start,
IN OUT gctUINT * Available,
OUT gctUINT *NumRegNeeded
)
{
    gceSTATUS    status;
    cltQUALIFIER accessQualifier;
    cltQUALIFIER addrSpaceQualifier;
    cltQUALIFIER storageQualifier;
    clsGEN_CODE_DATA_TYPE    binaryDataType;
    gctSIZE_T    binaryDataTypeRegSize;
    gctUINT      logicalRegCount, i;
    gctREG_INDEX tempRegIndex;
    gcUNIFORM    uniform;
    gcATTRIBUTE  attribute;
    gcVARIABLE   variable;
    gctUINT numRegNeeded;
    gctUINT available = Available ? *Available: 0;

    /*gcmHEADER_ARG("Compiler=0x%x CodeGenerator=0x%x Name=0x%x Symbol=%s "
              "DataType=0x%x",
              Compiler, CodeGenerator, Name, Symbol?Symbol:"",
              Decl); */
    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_OBJECT(CodeGenerator, clvOBJ_CODE_GENERATOR);
    gcmVERIFY_ARGUMENT(Name);
    gcmASSERT(Decl && Decl->dataType);
    gcmASSERT(LogicalRegs);
    gcmASSERT(Start);

    if (Name->isBuiltin && Name->type == clvVARIABLE_NAME) {
        gcmVERIFY_OK(clGetBuiltinVariableImplSymbol(Compiler, Name, Symbol, &Symbol));
    }

    binaryDataType        = _ConvElementDataTypeForRegAlloc(Name);
    binaryDataTypeRegSize    = gcGetDataTypeRegSize(binaryDataType);

    addrSpaceQualifier = Name->decl.dataType->addrSpaceQualifier;
    storageQualifier = Name->decl.storageQualifier;
    if(!clmDECL_IsPointerType(&Name->decl) &&
       clmGEN_CODE_checkVariableForMemory(Name)) {
       accessQualifier = clvQUALIFIER_UNIFORM;
       logicalRegCount = 1;
    }
    else  {
       orgAccessQualifier = Name->decl.dataType->accessQualifier;
       if(!clmGEN_CODE_checkVariableForMemory(Name) &&
          clmDECL_IsArray(Decl)) {
          clmGetArrayElementCount(Decl->array, 0, logicalRegCount);
       }
       else logicalRegCount = 1;
    }

    *NumRegNeeded = 0;
    switch (accessQualifier) {
    case clvQUALIFIER_NONE:
    case clvQUALIFIER_CONST:
    case clvQUALIFIER_CONST_IN:
        numRegNeeded = logicalRegCount * binaryDataTypeRegSize;
        if(numRegNeeded > available) {
           tempRegIndex = clNewTempRegs(Compiler,
                                        numRegNeeded - available,
                                        binaryDataType.elementType);
           tempRegIndex -= (gctREG_INDEX) available;
           if(Available) {
             *Available = numRegNeeded;
           }
        }
        else {
           tempRegIndex = clNewTempRegs(Compiler,
                                        0,
                                        binaryDataType.elementType);
           tempRegIndex -= (gctREG_INDEX) available;
        }
        *NumRegNeeded = numRegNeeded;

        for (i = 0; i < logicalRegCount; i++) {
            clsLOGICAL_REG_InitializeTemp(LogicalRegs + *Start + i,
                              accessQualifier,
                              binaryDataType,
                              tempRegIndex + (gctREG_INDEX)(i * binaryDataTypeRegSize),
                                                      (i < available) ? gcvTRUE : gcvFALSE);
        }

        if (ParentName->type == clvFUNC_NAME || ParentName->type == clvKERNEL_FUNC_NAME
            || ParentName->type == clvPARAMETER_NAME) {
            gctUINT8 argumentQualifier = gcvFUNCTION_INPUT;

            gcmASSERT(ParentName->context.u.variable.u.function);

            switch (accessQualifier) {
            case clvQUALIFIER_NONE:
            case clvQUALIFIER_CONST_IN:
                argumentQualifier = gcvFUNCTION_INPUT;    break;
            }

            status = clNewFunctionArgument(Compiler,
                                           ParentName->context.u.variable.u.function,
                                           binaryDataType,
                                           logicalRegCount,
                                           tempRegIndex,
                                           argumentQualifier);
            if (gcmIS_ERROR(status)) return status;
        }
        else {
            status = clNewVariable(Compiler,
                                   ParentName->lineNo,
                                   ParentName->stringNo,
                                   Symbol,
                                   accessQualifier,
                                   addrSpaceQualifier,
                                   storageQualifier,
                                   binaryDataType,
                                   logicalRegCount,
                                   tempRegIndex,
                                   &variable);
            if (gcmIS_ERROR(status)) return status;
            if(storageQualifier & clvSTORAGE_QUALIFIER_EXTERN) {
                gcSHADER shader;

                /* set shader to have extern variables */
                gcmVERIFY_OK(cloCOMPILER_GetBinary(Compiler, &shader));
                SetShaderHasExternVariable(shader, gcvTRUE);
                SetVariableIsExtern(variable);
            }
            else if(storageQualifier & clvSTORAGE_QUALIFIER_STATIC) {
                SetVariableIsStatic(variable);
            }
            if(clmDECL_IsPointerType(&Name->decl)) {
                SetVariableIsPointer(variable);
            }
            else if(addrSpaceQualifier == clvQUALIFIER_NONE ||
                    addrSpaceQualifier == clvQUALIFIER_PRIVATE ||
                    addrSpaceQualifier == clvQUALIFIER_LOCAL) {
                SetVariableIsLocal(variable);
            }
        }
        break;

    case clvQUALIFIER_UNIFORM:
        {
            clsGEN_CODE_DATA_TYPE format;

            clmGEN_CODE_ConvDirectElementDataType(Decl->dataType, format);
            status = clNewUniform(Compiler,
                                  Name->lineNo,
                                  Name->stringNo,
                                  Symbol,
                                  binaryDataType,
                                  format,
                                  Name->u.variableInfo.variableType,
                                  clmDECL_IsPointerType(Decl) || clmGEN_CODE_checkVariableForMemory(Name),
                                  logicalRegCount,
                                  &uniform);
           if (gcmIS_ERROR(status)) return status;
           SetUniformQualifier(uniform, clConvToShaderTypeQualifier(accessQualifier));
           SetUniformQualifier(uniform, clConvToShaderTypeQualifier(addrSpaceQualifier));
           SetUniformQualifier(uniform, clConvStorageQualifierToShaderTypeQualifier(storageQualifier));
           _SetPointerUniformQualifiers(uniform, Decl);

           _AddDerivedTypeVariable(Compiler,
                                   CodeGenerator,
                                   Name,
                                   uniform);

           for (i = 0; i < logicalRegCount; i++) {
               clsLOGICAL_REG_InitializeUniform(LogicalRegs + *Start + i,
                                                accessQualifier,
                                                binaryDataType,
                                                uniform,
                                                (gctREG_INDEX)(i * binaryDataTypeRegSize),
                                                (i < available) ? gcvTRUE : gcvFALSE);
           }
        }
        break;

    case clvQUALIFIER_ATTRIBUTE:
        status = clNewAttribute(Compiler,
                    Name->lineNo,
                    Name->stringNo,
                    Symbol,
                    binaryDataType,
                    logicalRegCount,
                    gcvFALSE,
                    &attribute);

        if (gcmIS_ERROR(status)) return status;

        for (i = 0; i < logicalRegCount; i++) {
            clsLOGICAL_REG_InitializeAttribute(LogicalRegs + *Start + i,
                                               accessQualifier,
                                               binaryDataType,
                                               attribute,
                                               (gctREG_INDEX)(i * binaryDataTypeRegSize),
                                               (i < available) ? gcvTRUE : gcvFALSE);
        }
        break;

    default:
        gcmASSERT(0);
        return gcvSTATUS_INVALID_ARGUMENT;
    }

    (*Start) += logicalRegCount;
    return gcvSTATUS_OK;
}
#endif

static gceSTATUS
_AllocLogicalRegs(
IN cloCOMPILER Compiler,
IN cloCODE_GENERATOR CodeGenerator,
IN clsNAME * Name,
IN clsNAME * ParentName,
IN gctCONST_STRING Symbol,
IN clsDECL * Decl,
IN OUT clsLOGICAL_REG * LogicalRegs,
IN OUT gctUINT * Start,
IN OUT gctUINT * Available,
IN OUT gctUINT *NumTempRegNeeded
)
{
    gceSTATUS status;
    gctUINT    count, i;
    clsNAME *fieldName;
    gctUINT    offset;
    gctUINT regAllocated;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_OBJECT(CodeGenerator, clvOBJ_CODE_GENERATOR);
    gcmASSERT(Name);
    gcmASSERT(Decl);
    gcmASSERT(LogicalRegs);
    gcmASSERT(Start);

/* KLC CHANGE FOR local memory allocation */
    if(!(clmDECL_IsPointerType(Decl) || clmGEN_CODE_checkVariableForMemory(Name)) &&
       (Decl->dataType->elementType == clvTYPE_STRUCT || Decl->dataType->elementType == clvTYPE_UNION)) {
        gctUINT curStart;
        gctUINT newStart;
        gctUINT available[1] = {0};
        gctSIZE_T symbolLength = 0;
        gctSIZE_T len;
        gctSIZE_T fieldLength = 1;
        gctSIZE_T maxFieldLength = 1;
        gctSTRING symbol;
        clsARRAY arrayRef;

        arrayRef = Decl->array;
        if (clmDECL_IsArray(Decl)) {
           int j;
           clmGetArrayElementCount(Decl->array, 0, count);
           for(j = 0; j < Decl->array.numDim; j++) {
              arrayRef.length[j] = 0;
           }
           arrayRef.numDim = Decl->array.numDim - 1;
        }
        else count = 1;

        if(Available == gcvNULL && Decl->dataType->elementType == clvTYPE_UNION) {
           Available = available;
        }
        symbolLength = gcoOS_StrLen(Symbol, gcvNULL);
        FOR_EACH_DLINK_NODE(&Decl->dataType->u.fieldSpace->names, clsNAME, fieldName) {
            fieldLength = gcoOS_StrLen(fieldName->symbol, gcvNULL);
            if(fieldLength > maxFieldLength) maxFieldLength = fieldLength;
        }
        len = symbolLength + fieldLength + 20 * cldMAX_ARRAY_DIMENSION;

        status = cloCOMPILER_Allocate(Compiler,
                          len,
                          (gctPOINTER *) &symbol);
        if (gcmIS_ERROR(status)) return status;

        for (i = 0; i < count; i++) {
            gcmASSERT(Name->decl.dataType->u.fieldSpace);

            curStart = *Start;
            newStart = *Start;
            regAllocated = 0;
            FOR_EACH_DLINK_NODE(&Decl->dataType->u.fieldSpace->names, clsNAME, fieldName) {
                gcmASSERT(fieldName->decl.dataType);
                fieldLength = gcoOS_StrLen(fieldName->symbol, gcvNULL);

                if (clmDECL_IsArray(Decl)) {
                    int j;

                    offset = 0;
                    gcmVERIFY_OK(gcoOS_PrintStrSafe(symbol,
                                    len,
                                    &offset,
                                    "%s[%d]",
                                    Symbol,
                                    arrayRef.length[0]));
                    for(j = 1; j < Decl->array.numDim; j++) {
                       gcmVERIFY_OK(gcoOS_PrintStrSafe(symbol,
                                       len,
                                       &offset,
                                       "[%d]",
                                       arrayRef.length[i]));
                    }
                    gcmVERIFY_OK(gcoOS_PrintStrSafe(symbol,
                                    len,
                                    &offset,
                                    ".%s",
                                    fieldName->symbol));
                }
                else {
                    offset = 0;
                    gcmVERIFY_OK(gcoOS_PrintStrSafe(symbol, len,
                                    &offset,
                                    "%s.%s",
                                    Symbol,
                                    fieldName->symbol));
                }

                status = _AllocLogicalRegs(Compiler,
                               CodeGenerator,
                               fieldName,
                               ParentName,
                               symbol,
                               &fieldName->decl,
                               LogicalRegs,
                               Start,
                               Available,
                               NumTempRegNeeded);
                if (gcmIS_ERROR(status)) return status;
                if(Decl->dataType->elementType == clvTYPE_UNION) {
                   if(*Start > newStart) newStart = *Start;
                   *Start = curStart;
                }
                else {
                   regAllocated += *NumTempRegNeeded;
                   if(Available && *Available) {
                      gcmASSERT(*Available >= *NumTempRegNeeded);
                      *Available -= *NumTempRegNeeded;
                   }
                   newStart = *Start;
                }

            }
            *Start = newStart;
            if(Available) {
               *Available += regAllocated;
               *NumTempRegNeeded = regAllocated;
            }

            if(count > 1) {
               do {
                  if(arrayRef.numDim < 0) {
                     arrayRef.numDim = Decl->array.numDim - 1;
                  }
                  arrayRef.length[arrayRef.numDim] += 1;
                  if(arrayRef.length[arrayRef.numDim] <
                     Decl->array.length[arrayRef.numDim]) {
                     break;
                  }
                  arrayRef.length[arrayRef.numDim] = 0;
                  arrayRef.numDim--;
               } while (gcvTRUE);
            }
        }
        gcmVERIFY_OK(cloCOMPILER_Free(Compiler, symbol));
    }
    else {
        status = _AllocLogicalRegOrArray(Compiler,
                        CodeGenerator,
                        Name,
                        ParentName,
                        Symbol,
                        Decl,
                        LogicalRegs,
                        Start,
                        Available,
                        NumTempRegNeeded);
        if (gcmIS_ERROR(status)) return status;
    }

    return gcvSTATUS_OK;
}

gceSTATUS
clsNAME_CloneContext(
IN cloCOMPILER Compiler,
IN clsNAME * ActualParamName,
IN clsNAME * FormalParamName
)
{
    gceSTATUS status;
    gctUINT      i;

    gcmHEADER_ARG("Compiler=0x%x ActualParamName=0x%x FormalParamName=0x%x",
              Compiler, ActualParamName, FormalParamName);
    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    gcmVERIFY_ARGUMENT(ActualParamName);
    gcmVERIFY_ARGUMENT(FormalParamName);

    ActualParamName->context = FormalParamName->context;

    status = cloCOMPILER_Allocate(Compiler,
                      (gctSIZE_T)sizeof(clsLOGICAL_REG)
                      * FormalParamName->context.u.variable.logicalRegCount,
                      (gctPOINTER *) &ActualParamName->context.u.variable.logicalRegs);
    if (gcmIS_ERROR(status))
        {
            gcmFOOTER();
            return status;
        }

    for (i = 0; i < FormalParamName->context.u.variable.logicalRegCount; i++) {
        ActualParamName->context.u.variable.logicalRegs[i] = FormalParamName->context.u.variable.logicalRegs[i];
    }
    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
clsNAME_AllocLogicalRegs(
IN cloCOMPILER Compiler,
IN cloCODE_GENERATOR CodeGenerator,
IN clsNAME * Name
)
{
    gceSTATUS status;
    gctUINT    logicalRegCount;
    clsLOGICAL_REG *logicalRegs = gcvNULL;
    gctUINT    start = 0;
    gctUINT numTempRegNeeded = 0;

    /*gcmHEADER_ARG("Compiler=0x%x CodeGenerator=0x%x Name=0x%x",
              Compiler, CodeGenerator, Name);*/
    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_OBJECT(CodeGenerator, clvOBJ_CODE_GENERATOR);
    gcmASSERT(Name);
    gcmASSERT(Name->decl.dataType);
    gcmASSERT(Name->type == clvVARIABLE_NAME ||
              Name->type == clvENUM_NAME ||
              Name->type == clvPARAMETER_NAME ||
              Name->type == clvFUNC_NAME ||
              Name->type == clvKERNEL_FUNC_NAME);

    if (Name->context.u.variable.logicalRegCount != 0) return gcvSTATUS_OK;

    if (Name->type == clvPARAMETER_NAME && Name->u.variableInfo.u.aliasName != gcvNULL) {
        return clsNAME_CloneContext(Compiler,
                                    Name,
                                    Name->u.variableInfo.u.aliasName);
    }

    if(!clmDECL_IsPointerType(&Name->decl) &&
       (Name->type == clvVARIABLE_NAME || Name->type == clvPARAMETER_NAME) &&
       Name->decl.dataType->addrSpaceQualifier == clvQUALIFIER_CONSTANT &&
       Name->decl.storageQualifier & clvSTORAGE_QUALIFIER_EXTERN) {
        status = clsNAME_SetVariableAddressed(Compiler,
                                              Name);
        if (gcmIS_ERROR(status)) return status;
    }

    logicalRegCount = _GetOperandCountForRegAlloc(Name);
    gcmASSERT(logicalRegCount > 0);

    do {
        status = cloCOMPILER_Allocate(Compiler,
                                      (gctSIZE_T)sizeof(clsLOGICAL_REG) * logicalRegCount,
                                      (gctPOINTER *) &logicalRegs);
        if (gcmIS_ERROR(status)) break;

        status = _AllocLogicalRegs(Compiler,
                                   CodeGenerator,
                                   Name,
                                   Name,
                                   Name->symbol,
                                   &Name->decl,
                                   logicalRegs,
                                   &start,
                                   gcvNULL,
                                   &numTempRegNeeded);
        if (gcmIS_ERROR(status)) break;

        gcmASSERT(start == logicalRegCount);
        Name->context.u.variable.logicalRegCount    = logicalRegCount;
        Name->context.u.variable.logicalRegs    = logicalRegs;
        return gcvSTATUS_OK;
    } while (gcvFALSE);

    if (logicalRegs != gcvNULL) {
        gcmVERIFY_OK(cloCOMPILER_Free(Compiler, logicalRegs));
    }

    return status;
}

gceSTATUS
clsNAME_FreeLogicalRegs(
IN cloCOMPILER Compiler,
IN clsNAME * Name
)
{
    /*gcmHEADER_ARG("Compiler=0x%x Name=0x%x", Compiler, Name);*/

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    gcmASSERT(Name);

    if (Name->context.u.variable.logicalRegs != gcvNULL) {
        gcmVERIFY_OK(cloCOMPILER_Free(Compiler, Name->context.u.variable.logicalRegs));
    }

    return gcvSTATUS_OK;
}

static gceSTATUS
_AllocateKernelFuncResources(
IN cloCOMPILER Compiler,
IN cloCODE_GENERATOR CodeGenerator,
IN clsNAME * FuncName
)
{
    gceSTATUS status;
    clsNAME *paramName;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_OBJECT(CodeGenerator, clvOBJ_CODE_GENERATOR);
    gcmASSERT(FuncName);

    if (FuncName->context.u.variable.u.kernelFunction != gcvNULL) return gcvSTATUS_OK;

    status = clNewKernelFunction(Compiler,
                                 FuncName->lineNo,
                                 FuncName->stringNo,
                                 FuncName->u.funcInfo.mangledName ?
                                 FuncName->u.funcInfo.mangledName : FuncName->symbol,
                                 &FuncName->context.u.variable.u.kernelFunction);
    if (gcmIS_ERROR(status)) return status;

    /* set kernel function attributes */
    SetKFunctionFlags(FuncName->context.u.variable.u.kernelFunction,
                     !FuncName->u.funcInfo.isFuncDef
                         ? gcvFUNC_EXTERN
                         : gcvFUNC_NOATTR);

    /* Allocate registers for all parameters */
    FOR_EACH_DLINK_NODE(&FuncName->u.funcInfo.localSpace->names, clsNAME, paramName) {
        if (paramName->type != clvPARAMETER_NAME) break;

        paramName->context.u.variable.isKernel = gcvTRUE;
        paramName->context.u.variable.u.kernelFunction = FuncName->context.u.variable.u.kernelFunction;
        paramName->u.variableInfo.variableType = clvBUILTIN_KERNEL_ARG;

        status = clsNAME_AllocLogicalRegs(Compiler,
                          CodeGenerator,
                          paramName);
        if (gcmIS_ERROR(status)) return status;
    }

    /* Allocate registers for return value */
    if (!clmDECL_IsVoid(&FuncName->decl)) {
        /* Return registers are output */

        status = cloCOMPILER_CloneDataType(Compiler,
                                           clvQUALIFIER_NONE,
                                           FuncName->decl.dataType->addrSpaceQualifier,
                                           FuncName->decl.dataType,
                                           &FuncName->decl.dataType);

        status = clsNAME_AllocLogicalRegs(Compiler,
                                          CodeGenerator,
                                          FuncName);
        if (gcmIS_ERROR(status)) return status;
    }
    return gcvSTATUS_OK;
}

static gceSTATUS
_AllocateFuncResources(
IN cloCOMPILER Compiler,
IN cloCODE_GENERATOR CodeGenerator,
IN clsNAME * FuncName
)
{
    gceSTATUS status;
    clsNAME *paramName;
    cltPOOL_STRING symbol;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_OBJECT(CodeGenerator, clvOBJ_CODE_GENERATOR);
    gcmASSERT(FuncName);


#if cldSupportMultiKernelFunction
    if (FuncName->type == clvKERNEL_FUNC_NAME) {
       return _AllocateKernelFuncResources(Compiler,
                                           CodeGenerator,
                                           FuncName);
    }
#endif

    if (FuncName->context.u.variable.u.function != gcvNULL) return gcvSTATUS_OK;

    if(FuncName->u.funcInfo.isIntrinsicCall)
    {
        symbol = FuncName->u.funcInfo.mangledName;
        if(symbol == gcvNULL)
        {
            symbol = clCreateMangledFuncName(Compiler,
                                             FuncName);
        }

        FuncName->u.funcInfo.mangledName = FuncName->symbol = symbol;
    }
    else if(!FuncName->isBuiltin &&
            cloCOMPILER_IsExternSymbolsAllowed(Compiler)) {
        symbol = clCreateMangledFuncName(Compiler,
                                         FuncName);
        FuncName->u.funcInfo.mangledName = symbol;
    }

    status = clNewFunction(Compiler,
                           FuncName->lineNo,
                           FuncName->stringNo,
                           FuncName->u.funcInfo.mangledName ?
                           FuncName->u.funcInfo.mangledName : FuncName->symbol,
                           &FuncName->context.u.variable.u.function);
    if (gcmIS_ERROR(status)) return status;

    if(!FuncName->isBuiltin) {
        /* set function attributes */
        SetFunctionFlags(FuncName->context.u.variable.u.function,
                         !FuncName->u.funcInfo.isFuncDef
                             ? gcvFUNC_EXTERN
                             : ((FuncName->decl.storageQualifier & clvSTORAGE_QUALIFIER_STATIC)
                                 ? gcvFUNC_STATIC
                                 : gcvFUNC_NOATTR));
        if(cloCOMPILER_IsExternSymbolsAllowed(Compiler)) {
            SetFunctionFlags(FuncName->context.u.variable.u.function,
                             gcvFUNC_NAME_MANGLED);
        }
    }

    /* Allocate registers for all parameters */
    FOR_EACH_DLINK_NODE(&FuncName->u.funcInfo.localSpace->names, clsNAME, paramName) {
        if (paramName->type != clvPARAMETER_NAME) break;

        paramName->context.u.variable.isKernel = gcvFALSE;
        paramName->context.u.variable.u.function = FuncName->context.u.variable.u.function;
        status = clsNAME_AllocLogicalRegs(Compiler,
                          CodeGenerator,
                          paramName);
        if (gcmIS_ERROR(status)) return status;
    }

    /* Allocate registers for return value */
    if (!clmDECL_IsVoid(&FuncName->decl)) {
        /* Return registers are output */

        status = cloCOMPILER_CloneDataType(Compiler,
                                           clvQUALIFIER_OUT,
                                           FuncName->decl.dataType->addrSpaceQualifier,
                                           FuncName->decl.dataType,
                                           &FuncName->decl.dataType);

        status = clsNAME_AllocLogicalRegs(Compiler,
                                          CodeGenerator,
                                          FuncName);
        if (gcmIS_ERROR(status)) return status;
    }
    return gcvSTATUS_OK;
}

gceSTATUS
clsGEN_CODE_PARAMETERS_CopyOperands(
IN cloCOMPILER Compiler,
IN OUT clsGEN_CODE_PARAMETERS *ToParameters,
IN clsGEN_CODE_PARAMETERS *FromParameters
)
{
    gceSTATUS status = gcvSTATUS_OK;
    gctUINT    i;
    gctPOINTER pointer;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    gcmASSERT(ToParameters);
    gcmASSERT(FromParameters);

    gcmASSERT(FromParameters->operandCount);
    gcmASSERT(FromParameters->needLOperand || FromParameters->needROperand);

    *ToParameters = *FromParameters;

    status = cloCOMPILER_Allocate(Compiler,
                      (gctSIZE_T)sizeof(clsGEN_CODE_PARAMETER_DATA_TYPE) * FromParameters->operandCount,
                      (gctPOINTER *) &pointer);
    if (gcmIS_ERROR(status)) return status;
        ToParameters->dataTypes = pointer;

    for(i=0; i < FromParameters->operandCount; i++) {
       ToParameters->dataTypes[i] = FromParameters->dataTypes[i];
    }

    if (FromParameters->needLOperand)
    {
        status = cloCOMPILER_Allocate(Compiler,
                    (gctSIZE_T)sizeof(clsLOPERAND) * FromParameters->operandCount,
                    (gctPOINTER *) &pointer);
        if (gcmIS_ERROR(status)) return status;
                ToParameters->lOperands = pointer;
        for(i=0; i < FromParameters->operandCount; i++) {
           ToParameters->lOperands[i] = FromParameters->lOperands[i];
        }
    }

    if (FromParameters->needROperand)
    {
        status = cloCOMPILER_Allocate(Compiler,
                    (gctSIZE_T)sizeof(clsROPERAND) * FromParameters->operandCount,
                    (gctPOINTER *) &pointer);
        if (gcmIS_ERROR(status)) return status;
            ToParameters->rOperands = pointer;
        for(i=0; i < FromParameters->operandCount; i++) {
           ToParameters->rOperands[i] = FromParameters->rOperands[i];
        }
    }

    if (FromParameters->elementIndex)
    {
        status = cloCOMPILER_Allocate(Compiler,
                    (gctSIZE_T)sizeof(clsROPERAND),
                    (gctPOINTER *) &pointer);
        if (gcmIS_ERROR(status)) return status;
            ToParameters->elementIndex = pointer;
        ToParameters->elementIndex[0] = FromParameters->elementIndex[0];
    }
    return gcvSTATUS_OK;
}

static gceSTATUS
clsLOGICAL_REG_Dump(
IN cloCOMPILER Compiler,
IN clsLOGICAL_REG * LogicalReg
)
{
    gctCONST_STRING    name;
    gctUINT8    i, component;
    gcSHADER binary;
    const gctCHAR    componentNames[4] = {'x', 'y', 'z', 'w'};

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    gcmASSERT(LogicalReg);

    switch (LogicalReg->qualifier) {
    case clvQUALIFIER_UNIFORM:
        name = gcGetUniformName(LogicalReg->u.uniform);
        break;

    case clvQUALIFIER_ATTRIBUTE:
        gcmVERIFY_OK(cloCOMPILER_GetBinary(Compiler, &binary));
        name = gcGetAttributeName(binary, LogicalReg->u.attribute);
        break;

    default:
        name = "";
    }

    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                      clvDUMP_CODE_GENERATOR,
                      "<LOGICAL_REG qualifier=\"%s\" dataType=\"%s\""
                      " name=\"%s\" regIndex=\"%d\" componentSelection=\"",
                      clGetQualifierName(LogicalReg->qualifier),
                      gcGetDataTypeName(LogicalReg->dataType),
                      name,
                      LogicalReg->regIndex));

    gcmASSERT(LogicalReg->componentSelection.components <= cldMaxComponentCount);
    for (i = 0; i < LogicalReg->componentSelection.components; i++) {
        component = LogicalReg->componentSelection.selection[i];
        gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                          clvDUMP_CODE_GENERATOR,
                          "%c",
                          componentNames[component]));
    }

    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                      clvDUMP_CODE_GENERATOR,
                      "\" />"));
    return gcvSTATUS_OK;
}

static gceSTATUS
_DumpIndex(
IN cloCOMPILER Compiler,
IN gctCONST_STRING Name,
IN clsINDEX * Index
)
{
    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    gcmASSERT(Name);
    gcmASSERT(Index);

    switch (Index->mode) {
    case clvINDEX_NONE:
        return gcvSTATUS_OK;

    case clvINDEX_REG:
        gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                          clvDUMP_CODE_GENERATOR,
                          "<%s_REG_INDEX indexRegIndex=\"%d\" />",
                          Name,
                          Index->u.indexRegIndex));
        break;

    case clvINDEX_CONSTANT:
        gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                          clvDUMP_CODE_GENERATOR,
                          "<%s_CONSTANT_INDEX index=\"%d\" />",
                          Name,
                          Index->u.constant));
        break;
    }
    return gcvSTATUS_OK;
}

static gceSTATUS
clsLOPERAND_Dump(
IN cloCOMPILER Compiler,
IN clsLOPERAND * LOperand
)
{
    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    gcmASSERT(LOperand);

    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                      clvDUMP_CODE_GENERATOR,
                      "<LOPERAND dataType=\"%s\">",
                      gcGetDataTypeName(LOperand->dataType)));

    gcmVERIFY_OK(clsLOGICAL_REG_Dump(Compiler, &LOperand->reg));
    gcmVERIFY_OK(_DumpIndex(Compiler, "ARRAY", &LOperand->arrayIndex));
    gcmVERIFY_OK(_DumpIndex(Compiler, "MATRIX", &LOperand->matrixIndex));
    gcmVERIFY_OK(_DumpIndex(Compiler, "VECTOR", &LOperand->vectorIndex));
    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                      clvDUMP_CODE_GENERATOR,
                      "</LOPERAND>"));
    return gcvSTATUS_OK;
}

static gceSTATUS
clsROPERAND_Dump(
IN cloCOMPILER Compiler,
IN clsROPERAND * ROperand
)
{
    gctUINT    i;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    gcmASSERT(ROperand);

    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                      clvDUMP_CODE_GENERATOR,
                      "<ROPERAND dataType=\"%s\">",
                      gcGetDataTypeName(ROperand->dataType)));

    if (ROperand->isReg) {
        gcmVERIFY_OK(clsLOGICAL_REG_Dump(Compiler, &ROperand->u.reg));
    }
    else {
        gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                          clvDUMP_CODE_GENERATOR,
                          "<CONSTANT dataType=\"%s\" valueCount=\"%d\">",
                          gcGetDataTypeName(ROperand->u.constant.dataType),
                          ROperand->u.constant.valueCount));

        for (i = 0; i < ROperand->u.constant.valueCount; i++) {
            gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                              clvDUMP_CODE_GENERATOR,
                              "<VALUE bool=\"%s\" int=\"%d\" float=\"%f\" />",
                              (ROperand->u.constant.values[i].boolValue) ?
                              "true" : "false",
                              ROperand->u.constant.values[i].intValue,
                              ROperand->u.constant.values[i].floatValue));
        }

        gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                          clvDUMP_IR,
                          "</CONSTANT>"));
    }

    gcmVERIFY_OK(_DumpIndex(Compiler, "ARRAY", &ROperand->arrayIndex));
    gcmVERIFY_OK(_DumpIndex(Compiler, "MATRIX", &ROperand->matrixIndex));
    gcmVERIFY_OK(_DumpIndex(Compiler, "VECTOR", &ROperand->vectorIndex));
    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                      clvDUMP_CODE_GENERATOR,
                      "</ROPERAND>"));
    return gcvSTATUS_OK;
}

static gceSTATUS
clsIOPERAND_Dump(
IN cloCOMPILER Compiler,
IN clsIOPERAND * IOperand
)
{
    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    gcmASSERT(IOperand);

    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                      clvDUMP_CODE_GENERATOR,
                      "<IOPERAND dataType=\"%s\" tempRegIndex=\"%d\" />",
                      gcGetDataTypeName(IOperand->dataType),
                      IOperand->tempRegIndex));
    return gcvSTATUS_OK;
}

static gctUINT8
_ConvComponentToVector4Component(
IN gctUINT8 Component
)
{
    if(Component < cldMaxComponentCount) {
        return (Component & 0x3);
    }
    else {
        gcmASSERT(0);
        return clvCOMPONENT_X;
    }
}

static void
_ReverseComponentSelection(
IN clsCOMPONENT_SELECTION *Source,
OUT clsCOMPONENT_SELECTION *Dest
)
{
    gctUINT8 i;

    *Dest = ComponentSelection_XYZW;
    gcmASSERT(Source->components <= cldMaxComponentCount);
    for(i = 0; i < Source->components; i++) {
        gcmASSERT(Source->selection[i] < cldMaxComponentCount);
        Dest->selection[_ConvComponentToVector4Component(Source->selection[i])] = i;
    }
}

static void
_ReverseSuperComponentSelection(
IN clsCOMPONENT_SELECTION *Source,
OUT clsCOMPONENT_SELECTION *Dest
)
{
    gctUINT8 i;

    *Dest = ComponentSelection_VECTOR16;
    gcmASSERT(Source->components <= cldMaxComponentCount);
    for(i = 0; i < Source->components; i++) {
        Dest->selection[Source->selection[i]] = i;
    }
}

static void
_ReverseComponents(
IN gctUINT8 StartComponent,
IN gctUINT8 NumComponent,
OUT clsCOMPONENT_SELECTION *Dest
)
{
    gctUINT8  i;

    *Dest = ComponentSelection_XYZW;
    Dest->components = NumComponent;
    gcmASSERT(StartComponent < cldMaxComponentCount);
    gcmASSERT(NumComponent <= 4);

    for(i = 0; i < NumComponent; i++) {
        Dest->selection[_ConvComponentToVector4Component(StartComponent + i)] = i;
    }
}

static clsCOMPONENT_SELECTION
_ConvVectorIndexToComponentSelection(
IN gctUINT VectorIndex
)
{
    switch (VectorIndex) {
    case 0: return ComponentSelection_X;
    case 1: return ComponentSelection_Y;
    case 2: return ComponentSelection_Z;
    case 3: return ComponentSelection_W;

    case 4: return ComponentSelection_4;
    case 5: return ComponentSelection_5;
    case 6: return ComponentSelection_6;
    case 7: return ComponentSelection_7;

    case 8: return ComponentSelection_8;
    case 9: return ComponentSelection_9;
    case 10: return ComponentSelection_10;
    case 11: return ComponentSelection_11;

    case 12: return ComponentSelection_12;
    case 13: return ComponentSelection_13;
    case 14: return ComponentSelection_14;
    case 15: return ComponentSelection_15;

    default:
    gcmASSERT(0);
    return ComponentSelection_X;
    }
}

static clsCOMPONENT_SELECTION
_ConvVectorIndexToHighPrecisionComponentSelection(
IN gctUINT VectorIndex
)
{
    switch (VectorIndex) {
    case 0: return ComponentSelection_X;
    case 1: return ComponentSelection_Y;
    case 2: return ComponentSelection_Z;
    case 3: return ComponentSelection_W;

    case 4: return ComponentSelection_4;
    case 5: return ComponentSelection_5;
    case 6: return ComponentSelection_6;
    case 7: return ComponentSelection_7;

    case 8: return ComponentSelection_8;
    case 9: return ComponentSelection_9;
    case 10: return ComponentSelection_10;
    case 11: return ComponentSelection_11;

    case 12: return ComponentSelection_12;
    case 13: return ComponentSelection_13;
    case 14: return ComponentSelection_14;
    case 15: return ComponentSelection_15;

    default:
    gcmASSERT(0);
    return ComponentSelection_X;
    }
}

#define _clmConvVectorIndexToComponentSelection(DataType, VectorIndex)  \
    (clmGEN_CODE_IsHighPrecisionDataType((DataType)) ? _ConvVectorIndexToHighPrecisionComponentSelection((VectorIndex)) \
                                                   : _ConvVectorIndexToComponentSelection((VectorIndex)))

static gceSTATUS
_CompleteComponentSelection(
IN clsCOMPONENT_SELECTION *ComponentSelection,
IN gctUINT8 StartComponent
)
{
   if(StartComponent < cldMaxComponentCount) {
      gctUINT8 component, i;

      gcmASSERT(StartComponent > 0);
      component = ComponentSelection->selection[StartComponent - 1];
      for(i = StartComponent; i < cldMaxComponentCount; i++) {
        ComponentSelection->selection[i] = component;
      }
      return gcvSTATUS_OK;
   }
   else {
      return (StartComponent == cldMaxComponentCount) ? gcvSTATUS_OK : gcvSTATUS_INVALID_DATA;
   }
}

gceSTATUS
clGetStartComponentDefaultComponentSelection(
    IN gctUINT8 StartComponent,
    IN OUT clsCOMPONENT_SELECTION *ComponentSelection
    )
{
    gctUINT8 i, j;

    gcmASSERT(StartComponent < cldMaxComponentCount);

    for(i= 0, j = StartComponent; j < cldMaxComponentCount; i++, j++) {
       ComponentSelection->selection[i] = j;
    }
    return _CompleteComponentSelection(ComponentSelection,
                                       i);
}

static clsCOMPONENT_SELECTION
_SwizzleComponentSelection(
IN clsCOMPONENT_SELECTION *Source1,
IN clsCOMPONENT_SELECTION *Source2
)
{
#if defined __GNUC__
    clsCOMPONENT_SELECTION    result = {};
#else
    clsCOMPONENT_SELECTION    result = { 0 };
#endif
    gctUINT8 i;
    gceSTATUS status;

    result.components = Source1->components;

    status = _CompleteComponentSelection(Source2, Source2->components);
    gcmASSERT(status == gcvSTATUS_OK);

    for(i = 0; i < Source1->components; i++) {
        gcmASSERT(Source1->selection[i] < cldMaxComponentCount);
        result.selection[i] = Source2->selection[Source1->selection[i]];
    }

    return result;
}

static void
_ReshuffleComponentSelection(
IN clsCOMPONENT_SELECTION *ComponentSelection,
IN clsCOMPONENT_SELECTION *ReversedComponentSelection,
OUT clsCOMPONENT_SELECTION *Result
)
{
    gctUINT8 maxComponentIx, position;
    gctUINT8 i, j;
    gctUINT8 component, previous;

    gcmASSERT(Result);
    *Result = ComponentSelection_X;
    for(i = 0; i < ComponentSelection->components; i++) {
       Result->selection[i] = cldMaxComponentCount;
    }
    Result->components = ComponentSelection->components;
        maxComponentIx = ComponentSelection->components - 1;
    for(i = 0; i < ReversedComponentSelection->components; i++) {
        j = ReversedComponentSelection->selection[i];
        if(j > maxComponentIx) continue;
        position = i;
        if(i > maxComponentIx) {
          position = maxComponentIx;
        }
        component = ComponentSelection->selection[j];
        do {
          previous = Result->selection[position];
          if(component == previous) break;
          Result->selection[position] = component;
          if(previous == cldMaxComponentCount) break;
          component = previous;
        } while (position--);
    }
}

static clsCOMPONENT_SELECTION
_SwizzleSingleComponent(
IN gctUINT8 Component,
IN clsCOMPONENT_SELECTION *Source
)
{
    clsCOMPONENT_SELECTION    result;

    result = _ConvVectorIndexToComponentSelection(Component);
    result.selection[0] = Source->selection[result.selection[0]];

    return result;
}

static gctUINT8
_ConvOneComponentToEnable(
IN gctUINT8 Component
)
{
    switch (Component) {
    case clvCOMPONENT_X: return gcSL_ENABLE_X;
    case clvCOMPONENT_Y: return gcSL_ENABLE_Y;
    case clvCOMPONENT_Z: return gcSL_ENABLE_Z;
    case clvCOMPONENT_W: return gcSL_ENABLE_W;

    case 4: return gcSL_ENABLE_X;
    case 5: return gcSL_ENABLE_Y;
    case 6: return gcSL_ENABLE_Z;
    case 7: return gcSL_ENABLE_W;

    case 8: return gcSL_ENABLE_X;
    case 9: return gcSL_ENABLE_Y;
    case 10: return gcSL_ENABLE_Z;
    case 11: return gcSL_ENABLE_W;

    case 12: return gcSL_ENABLE_X;
    case 13: return gcSL_ENABLE_Y;
    case 14: return gcSL_ENABLE_Z;
    case 15: return gcSL_ENABLE_W;

    case 16: return gcSL_ENABLE_X;
    case 17: return gcSL_ENABLE_Y;
    case 18: return gcSL_ENABLE_Z;
    case 19: return gcSL_ENABLE_W;

    case 20: return gcSL_ENABLE_X;
    case 21: return gcSL_ENABLE_Y;
    case 22: return gcSL_ENABLE_Z;
    case 23: return gcSL_ENABLE_W;

    case 24: return gcSL_ENABLE_X;
    case 25: return gcSL_ENABLE_Y;
    case 26: return gcSL_ENABLE_Z;
    case 27: return gcSL_ENABLE_W;

    case 28: return gcSL_ENABLE_X;
    case 29: return gcSL_ENABLE_Y;
    case 30: return gcSL_ENABLE_Z;
    case 31: return gcSL_ENABLE_W;

    default:
    gcmASSERT(0);
    return gcSL_ENABLE_X;
    }
}

static gctUINT8
_ConvComponentsToEnable(
IN gctUINT8 StartComponent,
IN gctUINT8 NumComponent
)
{
    gctUINT8  enable = 0;
    gctUINT8 i;

    for(i = 0; i < NumComponent; i++) {
        enable |= _ConvOneComponentToEnable(StartComponent + i);
    }
    return enable;
}

static gctUINT8
_ConvComponentToSectionalEnable(
IN gctUINT8 Component,
OUT gctUINT8 *SectionId
)
{
    switch (Component) {
    case clvCOMPONENT_X:
       *SectionId = 0;
       return gcSL_ENABLE_X;
    case clvCOMPONENT_Y:
       *SectionId = 0;
       return gcSL_ENABLE_Y;
    case clvCOMPONENT_Z:
       *SectionId = 0;
       return gcSL_ENABLE_Z;
    case clvCOMPONENT_W:
       *SectionId = 0;
       return gcSL_ENABLE_W;

    case 4:
       *SectionId = 1;
       return gcSL_ENABLE_X;
    case 5:
       *SectionId = 1;
       return gcSL_ENABLE_Y;
    case 6:
       *SectionId = 1;
       return gcSL_ENABLE_Z;
    case 7:
       *SectionId = 1;
       return gcSL_ENABLE_W;

    case 8:
       *SectionId = 2;
       return gcSL_ENABLE_X;
    case 9:
       *SectionId = 2;
       return gcSL_ENABLE_Y;
    case 10:
       *SectionId = 2;
       return gcSL_ENABLE_Z;
    case 11:
       *SectionId = 2;
       return gcSL_ENABLE_W;

    case 12:
       *SectionId = 3;
       return gcSL_ENABLE_X;
    case 13:
       *SectionId = 3;
       return gcSL_ENABLE_Y;
    case 14:
       *SectionId = 3;
       return gcSL_ENABLE_Z;
    case 15:
       *SectionId = 3;
       return gcSL_ENABLE_W;

    default:
    gcmASSERT(0);
       *SectionId = 0;
       return gcSL_ENABLE_X;
    }
}

static gctSIZE_T
_ConvComponentSelectionToSuperEnable(
IN clsCOMPONENT_SELECTION *ComponentSelection,
OUT gctUINT8 *SuperEnable
)
{
    gctUINT8  enable = 0;
    gctUINT8 i, j;
    gctSIZE_T numSections = 0;

    _clmInitializeSuperEnable(SuperEnable);
    for(i = 0; i < ComponentSelection->components; i++) {
        enable = _ConvComponentToSectionalEnable(ComponentSelection->selection[i], &j);
        SuperEnable[j] |= enable;
        if(numSections < j) {
            numSections = j;
        }
    }
    return enable ? (numSections + 1) : 0;
}

static gctUINT8
_ConvComponentSelectionToEnable(
IN clsCOMPONENT_SELECTION *ComponentSelection
)
{
    gctUINT8  enable = 0;
    gctUINT8 i;

    for(i = 0; i < ComponentSelection->components; i++) {
        enable |= _ConvOneComponentToEnable(ComponentSelection->selection[i]);
    }
    return enable;
}

#define _clmConvEnableToComponentCount(Enable) \
        ((((Enable) & gcSL_ENABLE_X) ? 1 : 0) + \
         (((Enable) & gcSL_ENABLE_Y) ? 1 : 0) + \
         (((Enable) & gcSL_ENABLE_W) ? 1 : 0) + \
         (((Enable) & gcSL_ENABLE_Z) ? 1 : 0))

#define _clmConvEnableToComponentStart(Enable) \
        ((((Enable) & gcSL_ENABLE_X) ? 0 : \
         (((Enable) & gcSL_ENABLE_Y) ? 1 : \
         (((Enable) & gcSL_ENABLE_Z) ? 2 : 3))))


static gceSTATUS
_GetSectionalComponentSelection(
IN clsCOMPONENT_SELECTION *ComponentSelection,
IN gctUINT8 SectionId,
OUT clsCOMPONENT_SELECTION *SectionComponentSelection
)
{
   gctUINT8 i, j;
   gctUINT8 startComponent;
   gctUINT8 lastComponent;

   if((SectionId << 2) > ComponentSelection->components) {
     SectionComponentSelection->components = 0;
     return gcvSTATUS_OK;
   }
   if(ComponentSelection->components > 4) {
     gcmASSERT(SectionId < (cldMaxComponentCount >> 2));
     startComponent = SectionId << 2;
     gcmASSERT(startComponent < ComponentSelection->components);
     lastComponent = startComponent + 4;
     lastComponent = (lastComponent > ComponentSelection->components) ?
                                      ComponentSelection->components : lastComponent;
   }
   else {
     startComponent = 0;
     lastComponent = ComponentSelection->components;
   }

   for(i = startComponent, j = 0; i < lastComponent; i++, j++) {
      SectionComponentSelection->selection[j] = ComponentSelection->selection[i];
   }
   SectionComponentSelection->components = lastComponent - startComponent;
   return gcvSTATUS_OK;
}

static clsCOMPONENT_SELECTION *
_CheckHighPrecisionComponentSelection(
IN clsGEN_CODE_DATA_TYPE DataType,
IN clsCOMPONENT_SELECTION *ComponentSelection,
IN clsCOMPONENT_SELECTION *NewComponentSelection
)
{
   if(clmGEN_CODE_IsHighPrecisionDataType(DataType)) {
       gctUINT8 i, j;
       gctUINT8 component;

       NewComponentSelection->components = ComponentSelection->components << 1;
       for(i = 0, j = 0;  i < ComponentSelection->components; i++) {
           component = ComponentSelection->selection[i] << 1;
           NewComponentSelection->selection[j++] = component;
           NewComponentSelection->selection[j++] = component + 1;
       }
       return NewComponentSelection;
   }
   else {
       return ComponentSelection;
   }
}

static gceSTATUS
_ConvLOperandToSuperTarget(
IN cloCOMPILER Compiler,
IN clsLOPERAND * LOperand,
OUT gcsSUPER_TARGET *SuperTarget,
OUT clsCOMPONENT_SELECTION * ReversedComponentSelection
)
{
    gctREG_INDEX    tempRegIndex;
    gctUINT8 superEnable[cldMaxFourComponentCount], i;
    gcSL_INDEXED    indexMode;
    gctREG_INDEX    indexRegIndex;
    clsCOMPONENT_SELECTION    currentComponentSelection;
    clsCOMPONENT_SELECTION    reversedComponentSelection;

    SuperTarget->numTargets = 0;
    reversedComponentSelection.components = 0;
    _clmInitializeSuperEnable(superEnable);

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    gcmASSERT(LOperand);
    gcmASSERT(!gcIsMatrixDataType(LOperand->dataType));
    gcmASSERT(SuperTarget);
    gcmASSERT(ReversedComponentSelection);

    tempRegIndex = LOperand->reg.regIndex;

    switch (LOperand->arrayIndex.mode) {
    case clvINDEX_NONE:
        indexMode = gcSL_NOT_INDEXED;
        indexRegIndex = 0;
        break;

    case clvINDEX_REG:
        gcmASSERT(LOperand->matrixIndex.mode != clvINDEX_REG);

        indexMode = gcSL_INDEXED_X;
        indexRegIndex = LOperand->arrayIndex.u.indexRegIndex;
        break;

    default:
        gcmASSERT(0);
        return gcvSTATUS_INVALID_ARGUMENT;
    }

    if (gcIsScalarDataType(LOperand->dataType)) {
        if (gcIsScalarDataType(LOperand->reg.dataType)) {
            superEnable[0] = gcGetDefaultEnable(LOperand->dataType);
            reversedComponentSelection = clGetDefaultComponentSelection(LOperand->dataType);
        }
        else {
            gcmASSERT(gcIsVectorDataType(LOperand->reg.dataType)
                      || gcIsMatrixDataType(LOperand->reg.dataType));

            switch (LOperand->vectorIndex.mode) {
            case clvINDEX_CONSTANT:
                currentComponentSelection = _clmConvVectorIndexToComponentSelection(LOperand->dataType,
                                                                                    LOperand->vectorIndex.u.constant);

                currentComponentSelection = _SwizzleComponentSelection(&currentComponentSelection,
                                                                       &LOperand->reg.componentSelection);
                _ConvComponentSelectionToSuperEnable(&currentComponentSelection, superEnable);

                _ReverseSuperComponentSelection(&currentComponentSelection,
                                                &reversedComponentSelection);
                break;

            default:
                gcmASSERT(0);
                return gcvSTATUS_INVALID_ARGUMENT;
            }

            if (gcIsMatrixDataType(LOperand->reg.dataType)) {
                switch (LOperand->matrixIndex.mode) {
                case clvINDEX_CONSTANT:
                    tempRegIndex += (gctREG_INDEX) (LOperand->matrixIndex.u.constant
                                     * gcGetMatrixColumnRegSize(LOperand->reg.dataType));
                    break;

                case clvINDEX_REG:
                    gcmASSERT(LOperand->arrayIndex.mode != clvINDEX_REG);

                    indexMode = gcSL_INDEXED_X;
                    indexRegIndex = LOperand->matrixIndex.u.indexRegIndex;
                    break;

                default:
                    gcmASSERT(0);
                    return gcvSTATUS_INVALID_ARGUMENT;
                }
            }
        }
    }
    else if (gcIsVectorDataType(LOperand->dataType)) {
        gcmASSERT(gcIsVectorDataType(LOperand->reg.dataType)
                  || gcIsMatrixDataType(LOperand->reg.dataType));

        _ConvComponentSelectionToSuperEnable(&LOperand->reg.componentSelection,
                                             superEnable);

        _ReverseSuperComponentSelection(&LOperand->reg.componentSelection,
                                        &reversedComponentSelection);

        if (gcIsMatrixDataType(LOperand->reg.dataType)) {
            switch (LOperand->matrixIndex.mode) {
            case clvINDEX_CONSTANT:
                tempRegIndex += (gctREG_INDEX) (LOperand->matrixIndex.u.constant
                                * gcGetMatrixColumnRegSize(LOperand->reg.dataType));
                break;

            case clvINDEX_REG:
                gcmASSERT(LOperand->arrayIndex.mode != clvINDEX_REG);

                indexMode = gcSL_INDEXED_X;
                indexRegIndex = LOperand->matrixIndex.u.indexRegIndex;
                break;

            default:
                gcmASSERT(0);
                return gcvSTATUS_INVALID_ARGUMENT;
            }
        }
    }
    else {
        gcmASSERT(0);
    }

    for(i = 0; i < cldMaxFourComponentCount; i++) {
        if(superEnable[i]) {
            clsGEN_CODE_DATA_TYPE dataType;

            _clmGetSubsetDataType(LOperand->dataType,
                                  _GetEnableComponentCount(superEnable[i]),
                                  dataType);
            gcsTARGET_Initialize(SuperTarget->targets + SuperTarget->numTargets,
                                 dataType,
                                 tempRegIndex +
                                 _clmGetTempRegIndexOffset(i, clmGEN_CODE_elementType_GET(LOperand->dataType)),
                                 superEnable[i],
                                 indexMode,
                                 indexRegIndex);
            SuperTarget->numTargets++;
       }
    }

    *ReversedComponentSelection = reversedComponentSelection;
    gcmASSERT(SuperTarget->numTargets);
    return gcvSTATUS_OK;
}

static gceSTATUS
_ConvLOperandToTarget(
IN cloCOMPILER Compiler,
IN clsLOPERAND * LOperand,
OUT gcsTARGET * Target,
OUT clsCOMPONENT_SELECTION * ReversedComponentSelection
)
{
    gctREG_INDEX    tempRegIndex;
    gctUINT8    enable = 0;
    gcSL_INDEXED    indexMode;
    gctREG_INDEX    indexRegIndex;
    clsCOMPONENT_SELECTION    currentComponentSelection;
    clsCOMPONENT_SELECTION    reversedComponentSelection;

    reversedComponentSelection.components = 0;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    gcmASSERT(LOperand);
    gcmASSERT(!gcIsMatrixDataType(LOperand->dataType));
    gcmASSERT(Target);
    gcmASSERT(ReversedComponentSelection);

    tempRegIndex    = LOperand->reg.regIndex;

    switch (LOperand->arrayIndex.mode) {
    case clvINDEX_NONE:
        indexMode    = gcSL_NOT_INDEXED;
        indexRegIndex    = 0;
        break;

    case clvINDEX_REG:
        gcmASSERT(LOperand->matrixIndex.mode != clvINDEX_REG);

        indexMode    = gcSL_INDEXED_X;
        indexRegIndex    = LOperand->arrayIndex.u.indexRegIndex;
        break;

    default:
        gcmASSERT(0);
        return gcvSTATUS_INVALID_ARGUMENT;
    }

    if (gcIsScalarDataType(LOperand->dataType)) {
        if (gcIsScalarDataType(LOperand->reg.dataType)) {
            enable    = gcGetDefaultEnable(LOperand->dataType);
            reversedComponentSelection    = clGetDefaultComponentSelection(LOperand->dataType);
        }
        else {
            gcmASSERT(gcIsVectorDataType(LOperand->reg.dataType)
                   || gcIsMatrixDataType(LOperand->reg.dataType));

            switch (LOperand->vectorIndex.mode) {
            case clvINDEX_CONSTANT:
                currentComponentSelection =
                 _clmConvVectorIndexToComponentSelection(LOperand->dataType, LOperand->vectorIndex.u.constant);

                currentComponentSelection = _SwizzleComponentSelection(&currentComponentSelection,
                                                                       &LOperand->reg.componentSelection);
                tempRegIndex += _clmGetTempRegIndexOffset(_clmGetComponentSectionIndex(currentComponentSelection.selection[0]),
                                                          clmGEN_CODE_elementType_GET(LOperand->dataType));
                enable = _ConvComponentSelectionToEnable(&currentComponentSelection);

                _ReverseComponentSelection(&currentComponentSelection,
                                           &reversedComponentSelection);
                break;

            default:
                gcmASSERT(0);
                return gcvSTATUS_INVALID_ARGUMENT;
            }

            if (gcIsMatrixDataType(LOperand->reg.dataType)) {
                switch (LOperand->matrixIndex.mode) {
                case clvINDEX_CONSTANT:
                    tempRegIndex += (gctREG_INDEX) (LOperand->matrixIndex.u.constant
                                    * gcGetMatrixColumnRegSize(LOperand->reg.dataType));
                    break;

                case clvINDEX_REG:
                    gcmASSERT(LOperand->arrayIndex.mode != clvINDEX_REG);

                    indexMode    = gcSL_INDEXED_X;
                    indexRegIndex    = LOperand->matrixIndex.u.indexRegIndex;
                    break;

                default:
                    gcmASSERT(0);
                    return gcvSTATUS_INVALID_ARGUMENT;
                }
            }
        }
    }
    else if (gcIsVectorDataType(LOperand->dataType)) {
        gcmASSERT(gcIsVectorDataType(LOperand->reg.dataType)
                || gcIsMatrixDataType(LOperand->reg.dataType));

        enable = _ConvComponentSelectionToEnable(&LOperand->reg.componentSelection);

        _ReverseComponentSelection(&LOperand->reg.componentSelection,
                                   &reversedComponentSelection);

        if (gcIsMatrixDataType(LOperand->reg.dataType)) {
            switch (LOperand->matrixIndex.mode) {
            case clvINDEX_CONSTANT:
                tempRegIndex += (gctREG_INDEX) (LOperand->matrixIndex.u.constant
                        * gcGetMatrixColumnRegSize(LOperand->reg.dataType));
                break;

            case clvINDEX_REG:
                gcmASSERT(LOperand->arrayIndex.mode != clvINDEX_REG);

                indexMode    = gcSL_INDEXED_X;
                indexRegIndex    = LOperand->matrixIndex.u.indexRegIndex;
                break;

            default:
                gcmASSERT(0);
                return gcvSTATUS_INVALID_ARGUMENT;
            }
        }
    }
    else {
        gcmASSERT(0);
    }

    gcsTARGET_Initialize(Target,
                 LOperand->dataType,
                 tempRegIndex,
                 enable,
                 indexMode,
                 indexRegIndex);

    *ReversedComponentSelection = reversedComponentSelection;
    return gcvSTATUS_OK;
}

static gctUINT8
_ChooseSingleComponentSwizzle(
    IN gctUINT8 Swizzle,
    IN gctUINT8 Component
    )
{
    gctUINT8 mask = 0x03;
    gctUINT8 shift = Component << 1;
    gctUINT8 swizzle = (Swizzle & (mask << shift)) >> shift;

    swizzle |= ((swizzle << 2) | (swizzle << 4) | (swizzle << 6));

    return swizzle;
}

static void
_SplitTargets(
IN gcsSUPER_TARGET *SuperTarget,
IN gctUINT8 NumTargets
)
{
   if(SuperTarget->numTargets != NumTargets && NumTargets != 1) {
      gcsSUPER_TARGET tempSuperTarget[1];
      gcsTARGET tempTarget;
      gctUINT8 i, j, componentCount, enableIndex, total;

      *tempSuperTarget = *SuperTarget;

      total = 0;
      for(i = 0; i < tempSuperTarget->numTargets; i++) {
         tempTarget = tempSuperTarget->targets[i];

     componentCount = gcGetDataTypeComponentCount(tempTarget.dataType);
         _clmGetSubsetDataType(tempTarget.dataType, 1, tempTarget.dataType);
         gcmASSERT(componentCount <= NumTargets);
         enableIndex = 0;
         for(j=0; j < componentCount; j++, total++) {
        SuperTarget->targets[total] = tempTarget;
        while(enableIndex < 4) {
               if(_EnableOrder[enableIndex] & tempTarget.enable) {
                   SuperTarget->targets[total].enable = _EnableOrder[enableIndex];
               enableIndex++;
                   break;
               }
           enableIndex++;
            }
        gcmASSERT(enableIndex <= 4);
         }
      }
      gcmASSERT(total >= NumTargets);
      SuperTarget->numTargets = total;
   }
}

#define _clmSplitOneTarget(SuperTarget, Index, SplittedTarget)  \
   do { \
     (SplittedTarget)->targets[0] = (SuperTarget)->targets[Index]; \
     (SplittedTarget)->numTargets = 1; \
     _SplitTargets((SplittedTarget), gcGetDataTypeComponentCount(SplittedTarget->targets[0].dataType)); \
   } while (gcvFALSE)


static void
_ExtractTarget(
IN gctUINT8 NumEnables,
IN OUT gcsTARGET *Target,
OUT gcsTARGET *ResTarget
)
{
   gctUINT8 i;
   gctUINT8 enable = 0;
   gctUINT8 enableCount = 0;
   gctUINT8 componentCount = gcGetDataTypeComponentCount(Target->dataType);

   gcmASSERT(componentCount >= NumEnables);

   for(i=0; i < 4; i++) {
      if(_EnableOrder[i] & Target->enable) {
         enable |= _EnableOrder[i];
         enableCount++;
         if(NumEnables == enableCount) break;
      }
   }

   *ResTarget = *Target;
   _clmGetSubsetDataType(Target->dataType, NumEnables, ResTarget->dataType);
   ResTarget->enable = enable;

   gcmASSERT(componentCount - NumEnables);
   _clmGetSubsetDataType(Target->dataType, componentCount - NumEnables, Target->dataType);
   Target->enable &= ~enable;
}

static void
_SplitSources(
IN gcsSUPER_SOURCE *SuperSource,
IN gctUINT8 NumSources
)
{
   if(SuperSource->numSources != NumSources && NumSources != 1) {
      gctUINT8 total;

      if(SuperSource->numSources == 1 &&
         gcIsScalarDataType(SuperSource->sources[0].dataType)) {
         for(total=0; total < NumSources; total++) {
        SuperSource->sources[total] = SuperSource->sources[0];
         }
      }
      else {
         gcsSUPER_SOURCE tempSuperSource[1];
         gcsSOURCE tempSource;
         gctUINT8 i, j, componentCount;
         *tempSuperSource = *SuperSource;

         total = 0;
         for(i = 0; i < tempSuperSource->numSources; i++) {
            tempSource = tempSuperSource->sources[i];

        componentCount = gcGetDataTypeComponentCount(tempSource.dataType);
            _clmGetSubsetDataType(tempSource.dataType, 1, tempSource.dataType);
            for(j=0; j < componentCount; j++, total++) {
           SuperSource->sources[total] = tempSource;
               SuperSource->sources[total].u.sourceReg.swizzle = _ChooseSingleComponentSwizzle(tempSource.u.sourceReg.swizzle, j);
            }
         }
      }
      gcmASSERT(total >= NumSources);
      SuperSource->numSources = total;
   }
}

static gceSTATUS
_ConvLOperandToSectionalTarget(
IN cloCOMPILER Compiler,
IN clsLOPERAND * LOperand,
IN gctUINT8 SectionIndex,
OUT gcsSUPER_TARGET *SuperTarget,
OUT clsCOMPONENT_SELECTION * ReversedComponentSelection
)
{
    gctREG_INDEX    tempRegIndex;
    gctUINT8    enable = 0;
    gcSL_INDEXED    indexMode;
    gctREG_INDEX    indexRegIndex;
#if defined __GNUC__
    clsCOMPONENT_SELECTION    currentComponentSelection = {};
#else
    clsCOMPONENT_SELECTION    currentComponentSelection = { 0 };
#endif
    clsCOMPONENT_SELECTION    reversedComponentSelection;
    gctBOOL inSameSubVector = gcvTRUE;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    gcmASSERT(LOperand);
    gcmASSERT(!gcIsMatrixDataType(LOperand->dataType));
    gcmASSERT(SuperTarget);
    gcmASSERT(ReversedComponentSelection);

    SuperTarget->numTargets = 0;
    reversedComponentSelection.components = 0;

    tempRegIndex = LOperand->reg.regIndex;

    switch (LOperand->arrayIndex.mode) {
    case clvINDEX_NONE:
        indexMode = gcSL_NOT_INDEXED;
        indexRegIndex = 0;
        break;

    case clvINDEX_REG:
        gcmASSERT(LOperand->matrixIndex.mode != clvINDEX_REG);

        indexMode = gcSL_INDEXED_X;
        indexRegIndex = LOperand->arrayIndex.u.indexRegIndex;
        break;

    default:
        gcmASSERT(0);
        return gcvSTATUS_INVALID_ARGUMENT;
    }

    if (gcIsScalarDataType(LOperand->dataType)) {
        if (gcIsScalarDataType(LOperand->reg.dataType)) {
            enable = gcGetDefaultEnable(LOperand->dataType);
            reversedComponentSelection = clGetDefaultComponentSelection(LOperand->dataType);
            currentComponentSelection = reversedComponentSelection;
        }
        else {
            gcmASSERT(gcIsVectorDataType(LOperand->reg.dataType) ||
                      gcIsMatrixDataType(LOperand->reg.dataType));

            switch (LOperand->vectorIndex.mode) {
            case clvINDEX_CONSTANT:
                currentComponentSelection =
                    _clmConvVectorIndexToComponentSelection(LOperand->dataType, LOperand->vectorIndex.u.constant);

                enable = _ConvComponentSelectionToEnable(&currentComponentSelection);

                _ReverseComponentSelection(&currentComponentSelection,
                                           &reversedComponentSelection);
                break;

            default:
                gcmASSERT(0);
                return gcvSTATUS_INVALID_ARGUMENT;
            }

            if (gcIsMatrixDataType(LOperand->reg.dataType)) {
                switch (LOperand->matrixIndex.mode) {
                case clvINDEX_CONSTANT:
                    tempRegIndex += (gctREG_INDEX)(LOperand->matrixIndex.u.constant
                            * gcGetMatrixColumnRegSize(LOperand->reg.dataType));
                    break;

                case clvINDEX_REG:
                    gcmASSERT(LOperand->arrayIndex.mode != clvINDEX_REG);

                    indexMode = gcSL_INDEXED_X;
                    indexRegIndex = LOperand->matrixIndex.u.indexRegIndex;
                    break;

                default:
                    gcmASSERT(0);
                    return gcvSTATUS_INVALID_ARGUMENT;
                }
            }
        }
    }
    else if (gcIsVectorDataType(LOperand->dataType)) {
        gcmASSERT(gcIsVectorDataType(LOperand->reg.dataType)
                || gcIsMatrixDataType(LOperand->reg.dataType));
        _GetSectionalComponentSelection(&LOperand->reg.componentSelection,
                                        SectionIndex,
                                        &currentComponentSelection);
        if(currentComponentSelection.components == 0) return gcvSTATUS_OK;
        enable = _ConvComponentSelectionToEnable(&currentComponentSelection);

        if(_IsComponentSelectionSameSubVector(&currentComponentSelection)) {
            _ReverseComponentSelection(&currentComponentSelection,
                                       &reversedComponentSelection);
        }
        else {
            inSameSubVector = gcvFALSE;
            _ReverseSuperComponentSelection(&currentComponentSelection,
                                            &reversedComponentSelection);
        }

        if (gcIsMatrixDataType(LOperand->reg.dataType)) {
            switch (LOperand->matrixIndex.mode) {
            case clvINDEX_CONSTANT:
                tempRegIndex += (gctREG_INDEX)(LOperand->matrixIndex.u.constant
                        * gcGetMatrixColumnRegSize(LOperand->reg.dataType));
                break;

            case clvINDEX_REG:
                gcmASSERT(LOperand->arrayIndex.mode != clvINDEX_REG);

                indexMode = gcSL_INDEXED_X;
                indexRegIndex = LOperand->matrixIndex.u.indexRegIndex;
                break;

            default:
                gcmASSERT(0);
                return gcvSTATUS_INVALID_ARGUMENT;
            }
        }
    }
    else {
        gcmASSERT(0);
        return gcvSTATUS_INVALID_ARGUMENT;
    }

    ReversedComponentSelection[0] = reversedComponentSelection;
    if(inSameSubVector) {
        clsGEN_CODE_DATA_TYPE dataType;

        _clmGetSubsetDataType(LOperand->dataType, currentComponentSelection.components, dataType);
        gcsTARGET_Initialize(SuperTarget->targets,
                             dataType,
                             tempRegIndex +
                             _clmGetTempRegIndexOffset(_clmGetComponentSectionIndex(currentComponentSelection.selection[0]),
                                                       clmGEN_CODE_elementType_GET(LOperand->dataType)),
                             enable,
                             indexMode,
                             indexRegIndex);
        SuperTarget->numTargets = 1;
    }
    else {
        clsGEN_CODE_DATA_TYPE dataType;
        gctUINT8 sectionId;
        gctUINT8 i, j, numComponent;

        if(clmGEN_CODE_IsHighPrecisionDataType(LOperand->dataType)) {
            numComponent = 2;
        }
        else numComponent = 1;

        _clmGetSubsetDataType(LOperand->dataType, numComponent, dataType);
        for(i = 0, j = 0; i < currentComponentSelection.components; i += numComponent, j++) {
           sectionId = _clmGetComponentSectionIndex(currentComponentSelection.selection[i]);
           gcsTARGET_Initialize(SuperTarget->targets + j,
                                dataType,
                                tempRegIndex +
                                _clmGetTempRegIndexOffset(sectionId, clmGEN_CODE_elementType_GET(LOperand->dataType)),
                                _ConvComponentsToEnable(currentComponentSelection.selection[i], numComponent),
                                indexMode,
                                indexRegIndex);
           gcmASSERT((numComponent == 1) ||
                     ((currentComponentSelection.selection[i + 1] - currentComponentSelection.selection[i]) == 1));
           _ReverseComponents(currentComponentSelection.selection[i], numComponent,
                              ReversedComponentSelection + sectionId);
       }
       SuperTarget->numTargets = j;
    }

    return gcvSTATUS_OK;
}

static gceSTATUS
_ConvLOperandToVectorComponentTarget(
IN cloCOMPILER Compiler,
IN clsLOPERAND * LOperand,
IN gctUINT VectorIndex,
OUT gcsTARGET * Target
)
{
    gctREG_INDEX    tempRegIndex;
    gctUINT8    enable;
    gcSL_INDEXED    indexMode;
    gctREG_INDEX    indexRegIndex;
    clsCOMPONENT_SELECTION    componentSelection;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    gcmASSERT(LOperand);
    gcmASSERT(gcIsVectorDataType(LOperand->dataType));
    gcmASSERT(Target);

    tempRegIndex    = LOperand->reg.regIndex;

    switch (LOperand->arrayIndex.mode)
    {
    case clvINDEX_NONE:
        indexMode    = gcSL_NOT_INDEXED;
        indexRegIndex    = 0;
        break;

    case clvINDEX_REG:
        gcmASSERT(LOperand->matrixIndex.mode != clvINDEX_REG);

        indexMode    = gcSL_INDEXED_X;
        indexRegIndex    = LOperand->arrayIndex.u.indexRegIndex;
        break;

    default:
        gcmASSERT(0);
        return gcvSTATUS_INVALID_ARGUMENT;
    }

    gcmASSERT(gcIsVectorDataType(LOperand->reg.dataType)
                || gcIsMatrixDataType(LOperand->reg.dataType));

    componentSelection = _clmConvVectorIndexToComponentSelection(LOperand->dataType, VectorIndex);

    componentSelection = _SwizzleComponentSelection(&componentSelection,
                                                    &LOperand->reg.componentSelection);
    tempRegIndex += _clmGetTempRegIndexOffset(_clmGetComponentSectionIndex(componentSelection.selection[0]),
                                              clmGEN_CODE_elementType_GET(LOperand->dataType));
    enable = _ConvComponentSelectionToEnable(&componentSelection);

    if (gcIsMatrixDataType(LOperand->reg.dataType)) {
        switch (LOperand->matrixIndex.mode) {
        case clvINDEX_CONSTANT:
            tempRegIndex += (gctREG_INDEX)(LOperand->matrixIndex.u.constant
                    * gcGetMatrixColumnRegSize(LOperand->reg.dataType));
            break;

        case clvINDEX_REG:
            gcmASSERT(LOperand->arrayIndex.mode != clvINDEX_REG);

            indexMode    = gcSL_INDEXED_X;
            indexRegIndex    = LOperand->matrixIndex.u.indexRegIndex;
            break;

        default:
            gcmASSERT(0);
            return gcvSTATUS_INVALID_ARGUMENT;
        }
    }

    gcsTARGET_Initialize(Target,
                 gcGetVectorComponentDataType(LOperand->dataType),
                 tempRegIndex,
                 enable,
                 indexMode,
                 indexRegIndex);
    return gcvSTATUS_OK;
}

static gceSTATUS
_ConvLOperandToMatrixColumnSuperTarget(
IN cloCOMPILER Compiler,
IN clsLOPERAND * LOperand,
IN gctUINT MatrixIndex,
OUT gcsSUPER_TARGET * SuperTarget
)
{
    clsLOPERAND columnLOperand[1];
    clsCOMPONENT_SELECTION    reversedComponentSelection[cldMaxFourComponentCount];

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    gcmASSERT(LOperand);
    gcmASSERT(gcIsMatrixDataType(LOperand->dataType));
    gcmASSERT(gcIsMatrixDataType(LOperand->reg.dataType));
    gcmASSERT(SuperTarget);

    clsLOPERAND_InitializeAsMatrixColumn(columnLOperand, LOperand, MatrixIndex);
        return _ConvLOperandToSuperTarget(Compiler,
                                          columnLOperand,
                                          SuperTarget,
                                          reversedComponentSelection);
}

static gceSTATUS
_ConvLOperandToMatrixComponentTarget(
    IN cloCOMPILER Compiler,
    IN clsLOPERAND * LOperand,
    IN gctUINT MatrixIndex,
    IN gctUINT VectorIndex,
    OUT gcsTARGET * Target
    )
{
    clsGEN_CODE_DATA_TYPE    dataType;
    gctREG_INDEX    tempRegIndex;
    gctUINT8    enable;
    gcSL_INDEXED    indexMode;
    gctREG_INDEX    indexRegIndex;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    gcmASSERT(LOperand);
    gcmASSERT(gcIsMatrixDataType(LOperand->dataType));
    gcmASSERT(gcIsMatrixDataType(LOperand->reg.dataType));
    gcmASSERT(Target);

    dataType = gcGetVectorComponentDataType(gcGetMatrixColumnDataType(LOperand->dataType));
    tempRegIndex = (gctREG_INDEX) (LOperand->reg.regIndex +
                      (MatrixIndex * gcGetDataTypeRegSize(dataType)));

    tempRegIndex += _clmGetTempRegIndexOffset(_clmGetComponentSectionIndex(VectorIndex),
                                              clmGEN_CODE_elementType_GET(dataType));

    enable = _ConvOneComponentToEnable((gctUINT8) VectorIndex);

    switch (LOperand->arrayIndex.mode)
    {
    case clvINDEX_NONE:
        indexMode    = gcSL_NOT_INDEXED;
        indexRegIndex    = 0;
        break;

    case clvINDEX_REG:
        gcmASSERT(LOperand->matrixIndex.mode != clvINDEX_REG);

        indexMode    = gcSL_INDEXED_X;
        indexRegIndex    = LOperand->arrayIndex.u.indexRegIndex;
        break;

    default:
        gcmASSERT(0);
        return gcvSTATUS_INVALID_ARGUMENT;
    }

    gcsTARGET_Initialize(Target,
                 dataType,
                 tempRegIndex,
                 enable,
                 indexMode,
                 indexRegIndex);
    return gcvSTATUS_OK;
}

static gctUINT8
_ConvComponentToSwizzle(
    IN gctUINT8 Component
    )
{
    switch (Component)
    {
    case clvCOMPONENT_X: return gcSL_SWIZZLE_X;
    case clvCOMPONENT_Y: return gcSL_SWIZZLE_Y;
    case clvCOMPONENT_Z: return gcSL_SWIZZLE_Z;
    case clvCOMPONENT_W: return gcSL_SWIZZLE_W;

    case clvCOMPONENT_4: return gcSL_SWIZZLE_X;
    case clvCOMPONENT_5: return gcSL_SWIZZLE_Y;
    case clvCOMPONENT_6: return gcSL_SWIZZLE_Z;
    case clvCOMPONENT_7: return gcSL_SWIZZLE_W;

    case clvCOMPONENT_8: return gcSL_SWIZZLE_X;
    case clvCOMPONENT_9: return gcSL_SWIZZLE_Y;
    case clvCOMPONENT_10: return gcSL_SWIZZLE_Z;
    case clvCOMPONENT_11: return gcSL_SWIZZLE_W;

    case clvCOMPONENT_12: return gcSL_SWIZZLE_X;
    case clvCOMPONENT_13: return gcSL_SWIZZLE_Y;
    case clvCOMPONENT_14: return gcSL_SWIZZLE_Z;
    case clvCOMPONENT_15: return gcSL_SWIZZLE_W;

    case clvCOMPONENT_16: return gcSL_SWIZZLE_X;
    case clvCOMPONENT_17: return gcSL_SWIZZLE_Y;
    case clvCOMPONENT_18: return gcSL_SWIZZLE_Z;
    case clvCOMPONENT_19: return gcSL_SWIZZLE_W;

    case clvCOMPONENT_20: return gcSL_SWIZZLE_X;
    case clvCOMPONENT_21: return gcSL_SWIZZLE_Y;
    case clvCOMPONENT_22: return gcSL_SWIZZLE_Z;
    case clvCOMPONENT_23: return gcSL_SWIZZLE_W;

    case clvCOMPONENT_24: return gcSL_SWIZZLE_X;
    case clvCOMPONENT_25: return gcSL_SWIZZLE_Y;
    case clvCOMPONENT_26: return gcSL_SWIZZLE_Z;
    case clvCOMPONENT_27: return gcSL_SWIZZLE_W;

    case clvCOMPONENT_28: return gcSL_SWIZZLE_X;
    case clvCOMPONENT_29: return gcSL_SWIZZLE_Y;
    case clvCOMPONENT_30: return gcSL_SWIZZLE_Z;
    case clvCOMPONENT_31: return gcSL_SWIZZLE_W;

    default:
    gcmASSERT(0);
    return gcSL_SWIZZLE_X;
    }
}

static gctUINT8
_ConvComponentSelectionToSwizzle(
    IN clsCOMPONENT_SELECTION *ComponentSelection
    )
{
    gctUINT8 swizzle = _ConvComponentToSwizzle(ComponentSelection->selection[clvCOMPONENT_X]);

    swizzle |= (ComponentSelection->components >= 2)
            ?  ( _ConvComponentToSwizzle(ComponentSelection->selection[clvCOMPONENT_Y]) << 2)
            :  ((swizzle & 0x03) << 2);

    swizzle |= (ComponentSelection->components >= 3)
            ?  (_ConvComponentToSwizzle(ComponentSelection->selection[clvCOMPONENT_Z]) << 4)
            :  ((swizzle & 0x0C) << 2);

    swizzle |= (ComponentSelection->components >= 4)
            ?  (_ConvComponentToSwizzle(ComponentSelection->selection[clvCOMPONENT_W]) << 6)
            :  ((swizzle & 0x30) << 2);

    return swizzle;
}

static gctUINT8
_CompleteSwizzle(
IN gctUINT8 Swizzle,
IN gctUINT8 Components
)
{
    Swizzle |= (Components < 2) ? ((Swizzle & 0x03) << 2) : 0;

    Swizzle |= (Components < 3) ? ((Swizzle & 0x0C) << 2) : 0;

    Swizzle |= (Components < 4) ? ((Swizzle & 0x30) << 2) : 0;

    return Swizzle;
}

static gctUINT8
_ConvComponentToSectionalSwizzle(
IN gctUINT8 Component
)
{
    switch (Component)
    {
    case clvCOMPONENT_X:
       return gcSL_SWIZZLE_X;
    case clvCOMPONENT_Y:
       return gcSL_SWIZZLE_Y;
    case clvCOMPONENT_Z:
       return gcSL_SWIZZLE_Z;
    case clvCOMPONENT_W:
       return gcSL_SWIZZLE_W;

    case 4:
       return (1 << 2) | gcSL_SWIZZLE_X;
    case 5:
       return (1 << 2) | gcSL_SWIZZLE_Y;
    case 6:
       return (1 << 2) | gcSL_SWIZZLE_Z;
    case 7:
       return (1 << 2) | gcSL_SWIZZLE_W;

    case 8:
       return (2 << 2) | gcSL_SWIZZLE_X;
    case 9:
       return (2 << 2) | gcSL_SWIZZLE_Y;
    case 10:
       return (2 << 2) | gcSL_SWIZZLE_Z;
    case 11:
       return (2 << 2) | gcSL_SWIZZLE_W;

    case 12:
       return (3 << 2) | gcSL_SWIZZLE_X;
    case 13:
       return (3 << 2) | gcSL_SWIZZLE_Y;
    case 14:
       return (3 << 2) | gcSL_SWIZZLE_Z;
    case 15:
       return (3 << 2) | gcSL_SWIZZLE_W;

    default:
    gcmASSERT(0);
       return gcSL_SWIZZLE_X;
    }
}

static void
_ConvComponentSelectionToSuperSwizzle(
IN clsCOMPONENT_SELECTION *ComponentSelection,
OUT gctUINT8 *SuperSwizzle
)
{
    gctUINT8 i, j;
    gctUINT8 repeat;

    _clmInitializeSuperSwizzle(SuperSwizzle);

    for(i = 0; i < ComponentSelection->components; i++) {
        SuperSwizzle[i] = _ConvComponentToSectionalSwizzle(ComponentSelection->selection[i]);
    }

    j = 4 - (ComponentSelection->components & 0x3);
    if(j < 4) {
       repeat = SuperSwizzle[ComponentSelection->components - 1];
       for(i = 0; i < j; i++) {
         SuperSwizzle[ComponentSelection->components + i] = repeat;
       }
    }
}

static gceSOURCE_TYPE
_ConvQualifierToSourceType(
    IN cltQUALIFIER Qualifier
    )
{
    switch (Qualifier)
    {
    case clvQUALIFIER_NONE:
    case clvQUALIFIER_CONST:
    case clvQUALIFIER_CONST_IN:
    case clvQUALIFIER_OUT:
    case clvQUALIFIER_READ_ONLY:
    case clvQUALIFIER_WRITE_ONLY:
        return gcvSOURCE_TEMP;

    case clvQUALIFIER_UNIFORM:
        return gcvSOURCE_UNIFORM;

    case clvQUALIFIER_ATTRIBUTE:
        return gcvSOURCE_ATTRIBUTE;

    default:
        gcmASSERT(0);
        return gcvSOURCE_TEMP;
    }
}

static gceSTATUS
_ConvROperandToSourceReg(
    IN cloCOMPILER Compiler,
    IN clsROPERAND * ROperand,
    IN clsCOMPONENT_SELECTION ReversedComponentSelection,
    OUT gcsSOURCE * Source
    )
{
#if defined __GNUC__
    gcsSOURCE_REG            sourceReg = {};
#else
    gcsSOURCE_REG            sourceReg = { 0 };
#endif
    clsCOMPONENT_SELECTION    componentSelection;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    gcmASSERT(ROperand);
    gcmASSERT(ROperand->isReg);
    gcmASSERT(!gcIsMatrixDataType(ROperand->dataType));
    gcmASSERT(Source);

    sourceReg.u.attribute    = ROperand->u.reg.u.attribute;
    sourceReg.regIndex    = ROperand->u.reg.regIndex;

    switch (ROperand->arrayIndex.mode)
    {
    case clvINDEX_NONE:
        sourceReg.indexMode = gcSL_NOT_INDEXED;
        sourceReg.indexRegIndex    = 0;
        break;

    case clvINDEX_REG:
        gcmASSERT(ROperand->matrixIndex.mode != clvINDEX_REG);

        sourceReg.indexMode    = gcSL_INDEXED_X;
        sourceReg.indexRegIndex    = ROperand->arrayIndex.u.indexRegIndex;
        break;

    default:
        gcmASSERT(0);
        return gcvSTATUS_INVALID_ARGUMENT;
    }

    if (clmGEN_CODE_IsSamplerDataType(ROperand->dataType))
    {
        gcmASSERT(clmGEN_CODE_IsSamplerDataType(ROperand->u.reg.dataType));

        componentSelection = clGetDefaultComponentSelection(ROperand->dataType);

        componentSelection = _SwizzleComponentSelection(&ReversedComponentSelection,
                                                        &componentSelection);

        sourceReg.swizzle = _ConvComponentSelectionToSwizzle(&componentSelection);
    }
    else if (gcIsScalarDataType(ROperand->dataType))
    {
        if (gcIsScalarDataType(ROperand->u.reg.dataType))
        {
            componentSelection = clGetDefaultComponentSelection(ROperand->dataType);

            componentSelection = _SwizzleComponentSelection(&ReversedComponentSelection,
                                                            &componentSelection);

            sourceReg.swizzle = _ConvComponentSelectionToSwizzle(&componentSelection);
        }
        else
        {
            gcmASSERT(gcIsVectorDataType(ROperand->u.reg.dataType)
                        || gcIsMatrixDataType(ROperand->u.reg.dataType));

            switch (ROperand->vectorIndex.mode)
            {
            case clvINDEX_CONSTANT:
                componentSelection = _clmConvVectorIndexToComponentSelection(ROperand->dataType,
                                                                             ROperand->vectorIndex.u.constant);

                componentSelection = _SwizzleComponentSelection(&componentSelection,
                                                                &ROperand->u.reg.componentSelection);

                sourceReg.regIndex += _clmGetTempRegIndexOffset(_clmGetComponentSectionIndex(componentSelection.selection[0]),
                                                                clmGEN_CODE_elementType_GET(ROperand->dataType));
                componentSelection = _SwizzleComponentSelection(&ReversedComponentSelection,
                                                                &componentSelection);
                sourceReg.swizzle = _ConvComponentSelectionToSwizzle(&componentSelection);
                break;

            default:
                gcmASSERT(0);
                return gcvSTATUS_INVALID_ARGUMENT;
            }

            if (gcIsMatrixDataType(ROperand->u.reg.dataType))
            {
                switch (ROperand->matrixIndex.mode)
                {
                case clvINDEX_CONSTANT:
                    sourceReg.regIndex += (gctREG_INDEX)(ROperand->matrixIndex.u.constant
                                          * gcGetMatrixColumnRegSize(ROperand->u.reg.dataType));
                    break;

                case clvINDEX_REG:
                    gcmASSERT(ROperand->arrayIndex.mode != clvINDEX_REG);

                    sourceReg.indexMode    = gcSL_INDEXED_X;
                    sourceReg.indexRegIndex    = ROperand->matrixIndex.u.indexRegIndex;
                    break;

                default:
                    gcmASSERT(0);
                    return gcvSTATUS_INVALID_ARGUMENT;
                }
            }
        }
    }
    else if (gcIsVectorDataType(ROperand->dataType))
    {
        gcmASSERT(gcIsVectorDataType(ROperand->u.reg.dataType)
                  || gcIsMatrixDataType(ROperand->u.reg.dataType));

        componentSelection = _SwizzleComponentSelection(&ReversedComponentSelection,
                                                        &ROperand->u.reg.componentSelection);

        sourceReg.swizzle = _ConvComponentSelectionToSwizzle(&componentSelection);

        if (gcIsMatrixDataType(ROperand->u.reg.dataType))
        {
            switch (ROperand->matrixIndex.mode)
            {
            case clvINDEX_CONSTANT:
                sourceReg.regIndex += (gctREG_INDEX)(ROperand->matrixIndex.u.constant
                                      * gcGetMatrixColumnRegSize(ROperand->u.reg.dataType));
                break;

            case clvINDEX_REG:
                gcmASSERT(ROperand->arrayIndex.mode != clvINDEX_REG);

                sourceReg.indexMode    = gcSL_INDEXED_X;
                sourceReg.indexRegIndex    = ROperand->matrixIndex.u.indexRegIndex;
                break;

            default:
                gcmASSERT(0);
                return gcvSTATUS_INVALID_ARGUMENT;
            }
        }
    }
    else {
        gcmASSERT(0);
    }

    gcsSOURCE_InitializeReg(Source,
                            _ConvQualifierToSourceType(ROperand->u.reg.qualifier),
                            ROperand->dataType,
                            sourceReg);

    return gcvSTATUS_OK;
}

static gceSTATUS
_ConvROperandToSuperSourceReg(
    IN cloCOMPILER Compiler,
    IN clsROPERAND * ROperand,
    IN clsCOMPONENT_SELECTION ReversedComponentSelection,
    OUT gcsSUPER_SOURCE *SuperSource
    )
{
#if defined __GNUC__
    gcsSOURCE_REG            sourceReg = {};
#else
    gcsSOURCE_REG            sourceReg = { 0 };
#endif
    clsCOMPONENT_SELECTION    componentSelection;
    clsCOMPONENT_SELECTION    currentComponentSelection;
    gctUINT8 superSwizzle[cldMaxComponentCount] = { 0 };
    gctUINT8 shift                              = 0;
    gctUINT8 i, j, k;
    gctUINT8 sectionId                          = 0;
    gctSIZE_T numSections                       = 0;
    gctBOOL sameSection                         = gcvFALSE;
    gcsSOURCE_REG currentSourceReg;
    gctUINT8 swizzle                            = 0;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    gcmASSERT(ROperand);
    gcmASSERT(ROperand->isReg);
    gcmASSERT(!gcIsMatrixDataType(ROperand->dataType));
    gcmASSERT(SuperSource);

    SuperSource->numSources = 0;
    sourceReg.u.attribute   = ROperand->u.reg.u.attribute;
    sourceReg.regIndex      = ROperand->u.reg.regIndex;

    switch (ROperand->arrayIndex.mode)
    {
    case clvINDEX_NONE:
        sourceReg.indexMode = gcSL_NOT_INDEXED;
        sourceReg.indexRegIndex    = 0;
        break;

    case clvINDEX_REG:
        gcmASSERT(ROperand->matrixIndex.mode != clvINDEX_REG);

        sourceReg.indexMode    = gcSL_INDEXED_X;
        sourceReg.indexRegIndex    = ROperand->arrayIndex.u.indexRegIndex;
        break;

    default:
        gcmASSERT(0);
        return gcvSTATUS_INVALID_ARGUMENT;
    }

    if (gcIsScalarDataType(ROperand->dataType))
    {
        if (gcIsScalarDataType(ROperand->u.reg.dataType))
        {
            componentSelection = clGetDefaultComponentSelection(ROperand->dataType);

            componentSelection = _SwizzleComponentSelection(&ReversedComponentSelection,
                                    &componentSelection);

            _ConvComponentSelectionToSuperSwizzle(&componentSelection, superSwizzle);
        }
        else
        {
            gcmASSERT(gcIsVectorDataType(ROperand->u.reg.dataType)
                        || gcIsMatrixDataType(ROperand->u.reg.dataType));

            switch (ROperand->vectorIndex.mode)
            {
            case clvINDEX_CONSTANT:
                componentSelection = _clmConvVectorIndexToComponentSelection(ROperand->dataType,
                                                                             ROperand->vectorIndex.u.constant);

                componentSelection = _SwizzleComponentSelection(&componentSelection,
                                        &ROperand->u.reg.componentSelection);

                componentSelection = _SwizzleComponentSelection(&ReversedComponentSelection,
                                        &componentSelection);

                _ConvComponentSelectionToSuperSwizzle(&componentSelection, superSwizzle);
                break;

            default:
                gcmASSERT(0);
                return gcvSTATUS_INVALID_ARGUMENT;
            }

            if (gcIsMatrixDataType(ROperand->u.reg.dataType))
            {
                switch (ROperand->matrixIndex.mode)
                {
                case clvINDEX_CONSTANT:
                    sourceReg.regIndex += (gctREG_INDEX)(ROperand->matrixIndex.u.constant
                                * gcGetMatrixColumnRegSize(ROperand->u.reg.dataType));
                    break;

                case clvINDEX_REG:
                    gcmASSERT(ROperand->arrayIndex.mode != clvINDEX_REG);

                    sourceReg.indexMode    = gcSL_INDEXED_X;
                    sourceReg.indexRegIndex    = ROperand->matrixIndex.u.indexRegIndex;
                    break;

                default:
                    gcmASSERT(0);
                    return gcvSTATUS_INVALID_ARGUMENT;
                }
            }
        }
    }
    else if (gcIsVectorDataType(ROperand->dataType))
    {
        gcmASSERT(gcIsVectorDataType(ROperand->u.reg.dataType)
                    || gcIsMatrixDataType(ROperand->u.reg.dataType));

        componentSelection = _SwizzleComponentSelection(&ReversedComponentSelection,
                                &ROperand->u.reg.componentSelection);

        _ConvComponentSelectionToSuperSwizzle(&componentSelection, superSwizzle);

        if (gcIsMatrixDataType(ROperand->u.reg.dataType))
        {
            switch (ROperand->matrixIndex.mode)
            {
            case clvINDEX_CONSTANT:
                sourceReg.regIndex += (gctREG_INDEX)(ROperand->matrixIndex.u.constant
                            * gcGetMatrixColumnRegSize(ROperand->u.reg.dataType));
                break;

            case clvINDEX_REG:
                gcmASSERT(ROperand->arrayIndex.mode != clvINDEX_REG);

                sourceReg.indexMode    = gcSL_INDEXED_X;
                sourceReg.indexRegIndex    = ROperand->matrixIndex.u.indexRegIndex;
                break;

            default:
                gcmASSERT(0);
                return gcvSTATUS_INVALID_ARGUMENT;
            }
        }
    }
    else {
        gcmASSERT(0);
    }

    numSections = gcGetDataTypeRegSize(ROperand->dataType);

    k = 0;
    for(i = 0; i < numSections; i++) {
       sectionId = superSwizzle[k] & 0xc;
       shift = 2;
       sameSection = gcvTRUE;
       swizzle = superSwizzle[k] & 0x3;
           for(j = k + 1; j < (k + 4); j++) {
          if(sectionId != (superSwizzle[j] & 0xc)) {
             sameSection = gcvFALSE;
             break;
              }
          else {
             swizzle |= (superSwizzle[j] & 0x3) << shift;
             shift += 2;
          }
       }
       if(sameSection) {
          clsGEN_CODE_DATA_TYPE dataType;

          _GetSectionalComponentSelection(&componentSelection,
                          i,
                          &currentComponentSelection);
          currentSourceReg = sourceReg;
          currentSourceReg.regIndex += _clmGetTempRegIndexOffset(sectionId >> 2,
                                                                 clmGEN_CODE_elementType_GET(ROperand->dataType));
          currentSourceReg.swizzle = swizzle;
          _clmGetSubsetDataType(ROperand->dataType, currentComponentSelection.components, dataType);
          gcsSOURCE_InitializeReg(SuperSource->sources + SuperSource->numSources,
                      _ConvQualifierToSourceType(ROperand->u.reg.qualifier),
                      dataType,
                      currentSourceReg);
          SuperSource->numSources++;
       }
       else {
          clsGEN_CODE_DATA_TYPE dataType;

          _clmGetSubsetDataType(ROperand->dataType, 1, dataType);
          for(j = k; j < (k + 4); j++) {
             currentSourceReg = sourceReg;
             currentSourceReg.regIndex += _clmGetTempRegIndexOffset((superSwizzle[j] & 0xc) >> 2,
                                                                    clmGEN_CODE_elementType_GET(ROperand->dataType));
             currentSourceReg.swizzle = _CompleteSwizzle(superSwizzle[j] & 0x3, 1);
             gcsSOURCE_InitializeReg(SuperSource->sources + SuperSource->numSources,
                                     _ConvQualifierToSourceType(ROperand->u.reg.qualifier),
                                     dataType,
                                     currentSourceReg);
             SuperSource->numSources++;
          }
       }

       k += 4;
    }

    gcmASSERT(SuperSource->numSources);
    return gcvSTATUS_OK;
}

static gceSTATUS
_ConvROperandToSectionalSourceReg(
IN cloCOMPILER Compiler,
IN clsROPERAND * ROperand,
IN gctUINT8 SectionIndex,
IN gctUINT8 NumTargets,
IN clsCOMPONENT_SELECTION *ReversedComponentSelection,
OUT gcsSUPER_SOURCE * SuperSource
)
{
#if defined __GNUC__
    gcsSOURCE_REG sourceReg = {};
    clsCOMPONENT_SELECTION componentSelection = {};
#else
    gcsSOURCE_REG sourceReg = { 0 };
    clsCOMPONENT_SELECTION componentSelection = { 0 };
#endif
    clsCOMPONENT_SELECTION currentComponentSelection;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    gcmASSERT(ROperand);
    gcmASSERT(ROperand->isReg);
    gcmASSERT(!gcIsMatrixDataType(ROperand->dataType));
    gcmASSERT(SuperSource);

    SuperSource->numSources = 0;
    sourceReg.u.attribute = ROperand->u.reg.u.attribute;
    sourceReg.regIndex = ROperand->u.reg.regIndex;

    switch (ROperand->arrayIndex.mode)
    {
    case clvINDEX_NONE:
        sourceReg.indexMode = gcSL_NOT_INDEXED;
        sourceReg.indexRegIndex    = 0;
        break;

    case clvINDEX_REG:
        gcmASSERT(ROperand->matrixIndex.mode != clvINDEX_REG);

        sourceReg.indexMode = gcSL_INDEXED_X;
        sourceReg.indexRegIndex = ROperand->arrayIndex.u.indexRegIndex;
        break;

    default:
        gcmASSERT(0);
        return gcvSTATUS_INVALID_ARGUMENT;
    }

    if (gcIsScalarDataType(ROperand->dataType))
    {
        if (gcIsScalarDataType(ROperand->u.reg.dataType))
        {
            componentSelection = clGetDefaultComponentSelection(ROperand->dataType);

            currentComponentSelection = _SwizzleComponentSelection(ReversedComponentSelection,
                                           &componentSelection);

            sourceReg.swizzle = _ConvComponentSelectionToSwizzle(&currentComponentSelection);
        }
        else
        {
            gcmASSERT(gcIsVectorDataType(ROperand->u.reg.dataType)
                        || gcIsMatrixDataType(ROperand->u.reg.dataType));

            switch (ROperand->vectorIndex.mode)
            {
            case clvINDEX_CONSTANT:
                componentSelection = _clmConvVectorIndexToComponentSelection(ROperand->dataType,
                                                                             ROperand->vectorIndex.u.constant);

                componentSelection = _SwizzleComponentSelection(&componentSelection,
                                                                &ROperand->u.reg.componentSelection);

                currentComponentSelection = _SwizzleComponentSelection(ReversedComponentSelection,
                                               &componentSelection);

                sourceReg.swizzle = _ConvComponentSelectionToSwizzle(&currentComponentSelection);
                break;

            default:
                gcmASSERT(0);
                return gcvSTATUS_INVALID_ARGUMENT;
            }

            if (gcIsMatrixDataType(ROperand->u.reg.dataType))
            {
                switch (ROperand->matrixIndex.mode)
                {
                case clvINDEX_CONSTANT:
                    sourceReg.regIndex += (gctREG_INDEX)(ROperand->matrixIndex.u.constant
                                              * gcGetMatrixColumnRegSize(ROperand->u.reg.dataType));
                    break;

                case clvINDEX_REG:
                    gcmASSERT(ROperand->arrayIndex.mode != clvINDEX_REG);

                    sourceReg.indexMode = gcSL_INDEXED_X;
                    sourceReg.indexRegIndex = ROperand->matrixIndex.u.indexRegIndex;
                    break;

                default:
                    gcmASSERT(0);
                    return gcvSTATUS_INVALID_ARGUMENT;
                }
            }
        }
    }
    else if (gcIsVectorDataType(ROperand->dataType))
    {
        gcmASSERT(gcIsVectorDataType(ROperand->u.reg.dataType)
                    || gcIsMatrixDataType(ROperand->u.reg.dataType));

        _GetSectionalComponentSelection(&ROperand->u.reg.componentSelection,
                                        SectionIndex,
                                        &componentSelection);
        if(componentSelection.components == 0) return gcvSTATUS_OK;
        currentComponentSelection = _SwizzleComponentSelection(ReversedComponentSelection,
                                                               &componentSelection);

        sourceReg.swizzle = _ConvComponentSelectionToSwizzle(&currentComponentSelection);

        if (gcIsMatrixDataType(ROperand->u.reg.dataType))
        {
            switch (ROperand->matrixIndex.mode)
            {
            case clvINDEX_CONSTANT:
                sourceReg.regIndex += (gctREG_INDEX)(ROperand->matrixIndex.u.constant
                            * gcGetMatrixColumnRegSize(ROperand->u.reg.dataType));
                break;

            case clvINDEX_REG:
                gcmASSERT(ROperand->arrayIndex.mode != clvINDEX_REG);

                sourceReg.indexMode = gcSL_INDEXED_X;
                sourceReg.indexRegIndex = ROperand->matrixIndex.u.indexRegIndex;
                break;

            default:
                gcmASSERT(0);
                return gcvSTATUS_INVALID_ARGUMENT;
            }
        }
    }
    else {
        gcmASSERT(0);
    }

    if(_IsComponentSelectionSameSubVector(&componentSelection)) {
        sourceReg.regIndex += _clmGetTempRegIndexOffset(_clmGetComponentSectionIndex(componentSelection.selection[0]),
                                                        clmGEN_CODE_elementType_GET(ROperand->dataType));

        if(NumTargets == 1 ) {
            clsGEN_CODE_DATA_TYPE dataType;

            _clmGetSubsetDataType(ROperand->dataType, componentSelection.components, dataType);
            gcsSOURCE_InitializeReg(SuperSource->sources,
                                    _ConvQualifierToSourceType(ROperand->u.reg.qualifier),
                                    dataType,
                                    sourceReg);
            SuperSource->numSources = 1;
        }
        else {
            clsGEN_CODE_DATA_TYPE dataType;
            gctUINT8 i;

            gcmASSERT(NumTargets == componentSelection.components ||
                      componentSelection.components == 1);

            _clmGetSubsetDataType(ROperand->dataType, 1, dataType);
            for(i = 0; i < NumTargets; i++) {
                if(componentSelection.components == 1) {
                    currentComponentSelection = _SwizzleSingleComponent(0,
                                                                        &componentSelection);
                }
                else {
                    currentComponentSelection = _SwizzleSingleComponent(i,
                                                                        &componentSelection);
                }
                sourceReg.swizzle = _ConvComponentSelectionToSwizzle(&currentComponentSelection);
                gcsSOURCE_InitializeReg(SuperSource->sources + i,
                                        _ConvQualifierToSourceType(ROperand->u.reg.qualifier),
                                        dataType,
                                        sourceReg);
            }
            SuperSource->numSources = i;
        }
    }
    else {
       gctUINT8 i;
       gcsSOURCE_REG tempSourceReg;
       clsCOMPONENT_SELECTION orderedComponentSelection;
       clsGEN_CODE_DATA_TYPE dataType;

       if(NumTargets > 1) {
           gcmASSERT((!clmGEN_CODE_IsHighPrecisionDataType(ROperand->dataType) &&
                         componentSelection.components == NumTargets) ||
                         (clmGEN_CODE_IsHighPrecisionDataType(ROperand->dataType) &&
                         (componentSelection.components == (NumTargets << 1))));
       }
       else {
           _ReshuffleComponentSelection(&componentSelection,
                                        ReversedComponentSelection,
                                        &orderedComponentSelection);
       }

       _clmGetSubsetDataType(ROperand->dataType, 1, dataType);
       for(i = 0; i < componentSelection.components; i++) {
          tempSourceReg = sourceReg;
          if(NumTargets > 1) {
             currentComponentSelection = _SwizzleSingleComponent(i, &componentSelection);
             tempSourceReg.swizzle = _ConvComponentSelectionToSwizzle(&currentComponentSelection);
             tempSourceReg.regIndex += _clmGetTempRegIndexOffset(_clmGetComponentSectionIndex(componentSelection.selection[i]),
                                                                 clmGEN_CODE_elementType_GET(ROperand->dataType));
          }
          else {
             tempSourceReg.regIndex += _clmGetTempRegIndexOffset(_clmGetComponentSectionIndex(orderedComponentSelection.selection[i]),
                                                                 clmGEN_CODE_elementType_GET(ROperand->dataType));
          }
          gcsSOURCE_InitializeReg(SuperSource->sources + i,
                                  _ConvQualifierToSourceType(ROperand->u.reg.qualifier),
                                  dataType,
                                  tempSourceReg);
       }

       SuperSource->numSources = i;
    }

    return gcvSTATUS_OK;
}

static gceSTATUS
_ConvROperandToMatrixColumnSuperSourceReg(
    IN cloCOMPILER Compiler,
    IN clsROPERAND * ROperand,
    IN gctUINT MatrixIndex,
    OUT gcsSUPER_SOURCE * SuperSource
    )
{
    clsROPERAND columnROperand[1];

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    gcmASSERT(ROperand);
    gcmASSERT(ROperand->isReg);
    gcmASSERT(gcIsMatrixDataType(ROperand->dataType));
    gcmASSERT(gcIsMatrixDataType(ROperand->u.reg.dataType));
    gcmASSERT(SuperSource);

    clsROPERAND_InitializeAsMatrixColumn(columnROperand, ROperand, MatrixIndex);

    return _ConvROperandToSuperSourceReg(Compiler,
                         columnROperand,
                         clGetDefaultComponentSelection(columnROperand->dataType),
                         SuperSource);
}

static gceSTATUS
_ConvLongConstantToSource(
    IN cloCOMPILER Compiler,
    cluCONSTANT_VALUE * ConstantValue,
    clsGEN_CODE_DATA_TYPE DataType,
    OUT gcsSOURCE * Source
    )
{
    gceSTATUS status;
    clsIOPERAND iOperand[1];
    clsROPERAND rOperand[1];
    clsROPERAND lower[1], upper[1];
    clsGEN_CODE_DATA_TYPE type = clmGenCodeDataType(T_UINT);

    clsIOPERAND_New(Compiler, iOperand, DataType);
    clsROPERAND_InitializeUsingIOperand(rOperand, iOperand);

    clsROPERAND_InitializeScalarConstant(lower,
                                         type,
                                         uint,
                                         ConstantValue->uintArray[0]);
    clsROPERAND_InitializeScalarConstant(upper,
                                         type,
                                         uint,
                                         ConstantValue->uintArray[1]);
    status = clGenGenericCode2(Compiler,
                               0,
                               0,
                               clvOPCODE_MOV_LONG,
                               iOperand,
                               lower,
                               upper);
    if (gcmIS_ERROR(status)) return status;

    return _ConvNormalROperandToSource(Compiler,
                                       0,
                                       0,
                                       rOperand,
                                       Source);
}

static gceSTATUS
_ConvROperandToSourceConstant(
    IN cloCOMPILER Compiler,
    IN clsROPERAND * ROperand,
    OUT gcsSOURCE * Source
    )
{
    gcsSOURCE_CONSTANT sourceConstant;
    cltELEMENT_TYPE elementType;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    gcmASSERT(ROperand);
    gcmASSERT(!ROperand->isReg);
    gcmASSERT(gcIsScalarDataType(ROperand->dataType));
    gcmASSERT(Source);

    elementType = clmGEN_CODE_elementType_GET(ROperand->u.constant.dataType);
    if(gcIsScalarDataType(ROperand->u.constant.dataType)) {
       if(clmIsElementTypeHighPrecision(elementType)) {
           return _ConvLongConstantToSource(Compiler,
                                            &ROperand->u.constant.values[0],
                                            ROperand->u.constant.dataType,
                                            Source);
       }
       if(clmIsElementTypeFloating(elementType)) {
          sourceConstant.floatValue =
                                    ROperand->u.constant.values[0].floatValue;
       }
       else if(clmIsElementTypeBoolean(elementType)) {
          sourceConstant.boolValue =
                                    ROperand->u.constant.values[0].boolValue;
       }
       else if(clmIsElementTypeInteger(elementType)) {
          if(clmIsElementTypeUnsigned(elementType)) {
              sourceConstant.uintValue =
                  ROperand->u.constant.values[0].uintValue;
          }
          else {
              sourceConstant.intValue = ROperand->u.constant.values[0].intValue;
          }
       }
       else if(clmIsElementTypeEvent(elementType) ||
               clmIsElementTypeSampler(elementType)) {
          sourceConstant.uintValue =
              ROperand->u.constant.values[0].uintValue;
       }
       else {
          gcmASSERT(0);
          return gcvSTATUS_INVALID_ARGUMENT;
       }
    }
    else if(gcIsVectorDataType(ROperand->u.constant.dataType)) {
       gcmASSERT(ROperand->vectorIndex.mode == clvINDEX_CONSTANT);
       if(clmIsElementTypeHighPrecision(elementType)) {
           return _ConvLongConstantToSource(Compiler,
                                            &ROperand->u.constant.values[ROperand->vectorIndex.u.constant],
                                            ROperand->dataType,
                                            Source);
       }
       if(clmIsElementTypeFloating(elementType)) {
          sourceConstant.floatValue =
              ROperand->u.constant.values[ROperand->vectorIndex.u.constant].floatValue;
       }
       else if(clmIsElementTypeBoolean(elementType)) {
          sourceConstant.boolValue =
              ROperand->u.constant.values[ROperand->vectorIndex.u.constant].boolValue;
       }
       else if(clmIsElementTypeInteger(elementType)) {
          if(clmIsElementTypeUnsigned(elementType)) {
              sourceConstant.uintValue =
                  ROperand->u.constant.values[ROperand->vectorIndex.u.constant].uintValue;
          }
          else {
              sourceConstant.intValue    =
                  ROperand->u.constant.values[ROperand->vectorIndex.u.constant].intValue;
          }
       }
       else {
          gcmASSERT(0);
          return gcvSTATUS_INVALID_ARGUMENT;
       }
    }
    else if(gcIsMatrixDataType(ROperand->u.constant.dataType)) {
       gcmASSERT(ROperand->matrixIndex.mode == clvINDEX_CONSTANT);
       gcmASSERT(ROperand->vectorIndex.mode == clvINDEX_CONSTANT);
       sourceConstant.floatValue =
           ROperand->u.constant.values[ROperand->matrixIndex.u.constant
                    * gcGetMatrixDataTypeColumnCount(ROperand->u.constant.dataType)
                    + ROperand->vectorIndex.u.constant].floatValue;
    }
    else {
       gcmASSERT(0);
       return gcvSTATUS_INVALID_ARGUMENT;
    }

    gcsSOURCE_InitializeConstant(Source, ROperand->dataType, sourceConstant);

    return gcvSTATUS_OK;
}

static gceSTATUS
_ConvROperandToSpecialVectorSourceConstant(
    IN cloCOMPILER Compiler,
    IN clsROPERAND * ROperand,
    OUT gcsSOURCE * Source
    )
{
    gcsSOURCE_CONSTANT    sourceConstant;
    clsGEN_CODE_DATA_TYPE dataType;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    gcmASSERT(ROperand);
    gcmASSERT(!ROperand->isReg);
    gcmASSERT(ROperand->u.constant.allValuesEqual);
    gcmASSERT(Source);

    _clmGetSubsetDataType(ROperand->dataType, 1, dataType);
    if(gcIsVectorDataType(ROperand->dataType) ||
           gcIsScalarDataType(ROperand->dataType)) {
       cltELEMENT_TYPE elementType;
       elementType = clmGEN_CODE_elementType_GET(ROperand->dataType);
       if(clmIsElementTypePacked(elementType)) {
           elementType = _ConvPackedTypeToBasicType(elementType);
       }

       if(clmIsElementTypeHighPrecision(elementType)) {
           return _ConvLongConstantToSource(Compiler,
                                            &ROperand->u.constant.values[0],
                                            dataType,
                                            Source);
       }
       if(clmIsElementTypeFloating(elementType)) {
        sourceConstant.floatValue = ROperand->u.constant.values[0].floatValue;
       }
       else if(clmIsElementTypeBoolean(elementType)) {
        sourceConstant.boolValue = ROperand->u.constant.values[0].boolValue;
       }
       else if(clmIsElementTypeInteger(elementType)) {
          if(clmIsElementTypeUnsigned(elementType)) {
              sourceConstant.uintValue = ROperand->u.constant.values[0].uintValue;
          }
          else {
              sourceConstant.intValue = ROperand->u.constant.values[0].intValue;
          }
       }
       else if(clmIsElementTypeEvent(elementType) ||
               clmIsElementTypeSampler(elementType)) {
          sourceConstant.uintValue =
              ROperand->u.constant.values[0].uintValue;
       }
       else {
          gcmASSERT(0);
          return gcvSTATUS_INVALID_ARGUMENT;
       }
    }
    else {
       gcmASSERT(0);
       return gcvSTATUS_INVALID_ARGUMENT;
    }

    gcsSOURCE_InitializeConstant(Source, dataType, sourceConstant);

    return gcvSTATUS_OK;
}

static gceSTATUS
_ConvROperandToVectorComponentSourceConstant(
    IN cloCOMPILER Compiler,
    IN clsROPERAND * ROperand,
    IN gctUINT VectorIndex,
    OUT gcsSOURCE * Source
    )
{
    clsGEN_CODE_DATA_TYPE    dataType;
    gcsSOURCE_CONSTANT    sourceConstant;
    gctUINT    columnCount;
    cltELEMENT_TYPE elementType;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    gcmASSERT(ROperand);
    gcmASSERT(!ROperand->isReg);
    gcmASSERT(gcIsVectorDataType(ROperand->dataType));
    gcmASSERT(ROperand->u.constant.valueCount > VectorIndex);
    gcmASSERT(Source);

    dataType = gcGetVectorComponentDataType(ROperand->dataType);
    elementType = clmGEN_CODE_elementType_GET(dataType);

    if (gcIsMatrixDataType(ROperand->u.constant.dataType)) {
        gcmASSERT(gcIsDataTypeEqual(ROperand->dataType,
                                               gcGetMatrixColumnDataType(ROperand->u.constant.dataType)));
        gcmASSERT(ROperand->matrixIndex.mode == clvINDEX_CONSTANT);

        columnCount = gcGetMatrixDataTypeColumnCount(ROperand->u.constant.dataType);

        if(clmIsElementTypeFloating(elementType)) {
           sourceConstant.floatValue =
            ROperand->u.constant.values[ROperand->matrixIndex.u.constant * columnCount
                                                 + VectorIndex].floatValue;
        }
        else if(clmIsElementTypeBoolean(elementType)) {
           sourceConstant.boolValue =
            ROperand->u.constant.values[ROperand->matrixIndex.u.constant * columnCount
                                        + VectorIndex].boolValue;
        }
        else if(clmIsElementTypeInteger(elementType)) {
               if(clmIsElementTypeUnsigned(elementType)) {
             sourceConstant.uintValue    =
            ROperand->u.constant.values[ROperand->matrixIndex.u.constant * columnCount
                                        + VectorIndex].uintValue;
           }
           else {
             sourceConstant.intValue    =
            ROperand->u.constant.values[ROperand->matrixIndex.u.constant * columnCount
                                        + VectorIndex].intValue;
           }
        }
        else {
            gcmASSERT(0);
            return gcvSTATUS_INVALID_ARGUMENT;
        }
    }
    else
    {
        gcmASSERT(gcIsVectorDataType(ROperand->u.constant.dataType));

        if(clmIsElementTypePacked(elementType)) {
            elementType = _ConvPackedTypeToBasicType(elementType);
        }

        if(clmIsElementTypeFloating(elementType)) {
           sourceConstant.floatValue = ROperand->u.constant.values[VectorIndex].floatValue;
        }
        else if(clmIsElementTypeBoolean(elementType)) {
           sourceConstant.boolValue = ROperand->u.constant.values[VectorIndex].boolValue;
        }
        else if(clmIsElementTypeInteger(elementType)) {
              if(clmIsElementTypeUnsigned(elementType)) {
           sourceConstant.uintValue = ROperand->u.constant.values[VectorIndex].uintValue;
          }
          else {
           sourceConstant.intValue = ROperand->u.constant.values[VectorIndex].intValue;
          }
        }
        else {
           gcmASSERT(0);
           return gcvSTATUS_INVALID_ARGUMENT;
        }
    }

    gcsSOURCE_InitializeConstant(Source, dataType, sourceConstant);

    return gcvSTATUS_OK;
}

static gceSTATUS
_ConvROperandToMatrixComponentSourceConstant(
    IN cloCOMPILER Compiler,
    IN clsROPERAND * ROperand,
    IN gctUINT MatrixIndex,
    IN gctUINT VectorIndex,
    OUT gcsSOURCE * Source
    )
{
    clsGEN_CODE_DATA_TYPE    dataType;
    gcsSOURCE_CONSTANT    sourceConstant;
    gctUINT    index;
    cltELEMENT_TYPE elementType;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    gcmASSERT(ROperand);
    gcmASSERT(!ROperand->isReg);
    gcmASSERT(gcIsMatrixDataType(ROperand->dataType));
    gcmASSERT(ROperand->u.constant.valueCount > VectorIndex);
    gcmASSERT(Source);

    dataType = gcGetVectorComponentDataType(gcGetMatrixColumnDataType(ROperand->dataType));
    elementType = clmGEN_CODE_elementType_GET(dataType);
    index = MatrixIndex * gcGetMatrixDataTypeColumnCount(ROperand->dataType) + VectorIndex;

    if(clmIsElementTypeFloating(elementType)) {
       sourceConstant.floatValue = ROperand->u.constant.values[index].floatValue;
    }
    else if(clmIsElementTypeBoolean(elementType)) {
       sourceConstant.boolValue = ROperand->u.constant.values[index].boolValue;
    }
    else if(clmIsElementTypeInteger(elementType)) {
       if(clmIsElementTypeUnsigned(elementType)) {
         sourceConstant.uintValue = ROperand->u.constant.values[index].uintValue;
       }
       else {
         sourceConstant.intValue = ROperand->u.constant.values[index].intValue;
       }
    }
    else {
       gcmASSERT(0);
       return gcvSTATUS_INVALID_ARGUMENT;
    }

    gcsSOURCE_InitializeConstant(Source, dataType, sourceConstant);

    return gcvSTATUS_OK;
}

static gceSTATUS
_SpecialGenAssignCode(
    IN cloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN clsLOPERAND * LOperand,
    IN clsROPERAND * ROperand
    )
{
    gceSTATUS status;
    gcsSUPER_TARGET superTarget;
    gcsSUPER_SOURCE superSource;
#if defined __GNUC__
    clsCOMPONENT_SELECTION    reversedComponentSelection[cldMaxFourComponentCount] = {};
#else
    clsCOMPONENT_SELECTION    reversedComponentSelection[cldMaxFourComponentCount] = { 0 };
#endif
    gctUINT    i, j;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    gcmASSERT(LOperand);
    gcmASSERT(ROperand);

    if(!gcIsScalarDataType(ROperand->dataType)) {
       gcmASSERT(gcIsDataTypeEqual(LOperand->dataType, ROperand->dataType));
    }
    else if(clmIsElementTypeBoolean(clmGEN_CODE_elementType_GET(LOperand->dataType))
                || (clmIsElementTypeInteger(clmGEN_CODE_elementType_GET(LOperand->dataType))
                    && !clmIsElementTypeInteger(clmGEN_CODE_elementType_GET(ROperand->dataType)))
                || (clmIsElementTypeFloating(clmGEN_CODE_elementType_GET(LOperand->dataType))
                    && !clmIsElementTypeFloating(clmGEN_CODE_elementType_GET(ROperand->dataType)))) {
       status = clsROPERAND_ChangeDataTypeFamily(Compiler,
                                                 LineNo,
                                                 StringNo,
                                                 gcvFALSE,
                                                 LOperand->dataType,
                                                 ROperand);
    }

    if (gcIsScalarDataType(LOperand->dataType)) {
        status = _ConvLOperandToTarget(Compiler,
                                       LOperand,
                                       superTarget.targets,
                                       reversedComponentSelection);
        if (gcmIS_ERROR(status)) return status;

        if (ROperand->isReg) {
            status = _ConvROperandToSourceReg(Compiler,
                                              ROperand,
                                              reversedComponentSelection[0],
                                              superSource.sources);
            if (gcmIS_ERROR(status)) return status;
        }
        else {
            status = _ConvROperandToSourceConstant(Compiler,
                                                   ROperand,
                                                   superSource.sources);
            if (gcmIS_ERROR(status)) return status;
        }

        status = clEmitAssignCode(Compiler,
                                  LineNo,
                                  StringNo,
                                  superTarget.targets,
                                  superSource.sources,
                                  LOperand->reg.isUnionMember);

        if (gcmIS_ERROR(status)) return status;
    }
    else if (gcIsVectorDataType(LOperand->dataType)) {
        if(clmIsElementTypePacked(ROperand->dataType.elementType)) {
            clsCOMPONENT_SELECTION reversedComponentSelection[1];
            gcsTARGET target[1];
            gcsSOURCE source0[1];
            gcsSOURCE source1[1];
            gcsSOURCE_CONSTANT sourceConstant;

            status =  _ConvLOperandToTarget(Compiler,
                                            LOperand,
                                            target,
                                            reversedComponentSelection);
            if (gcmIS_ERROR(status)) return status;

            if(!ROperand->isReg && ROperand->u.constant.allValuesEqual) {
                status = _ConvROperandToSpecialVectorSourceConstant(Compiler,
                                                                    ROperand,
                                                                    source0);
                if(gcmIS_ERROR(status)) return status;
            }
            else {
                status = _ConvROperandToSourceReg(Compiler,
                                                  ROperand,
                                                  clGetDefaultComponentSelection(ROperand->dataType),
                                                  source0);
                if (gcmIS_ERROR(status)) return status;
            }

            sourceConstant.intValue = (gctINT)_DataTypeByteSize(LOperand->dataType);
            gcsSOURCE_InitializeConstant(source1, clmGenCodeDataType(T_INT), sourceConstant);
            status = clEmitCode2(Compiler,
                                 LineNo,
                                 StringNo,
                                 clvOPCODE_COPY,
                                 target,
                                 source0,
                                 source1);
            if (gcmIS_ERROR(status)) return status;
        }
        else if (ROperand->isReg) {
            gctSIZE_T lSize, rSize;
            gctUINT8 i, j;

            lSize = gcGetDataTypeRegSize(LOperand->dataType);
            rSize = gcGetDataTypeRegSize(ROperand->dataType);

            gcmASSERT(rSize == lSize || rSize == 1);
            for(i = 0; i < lSize; i++) {
                status = _ConvLOperandToSectionalTarget(Compiler,
                                                        LOperand,
                                                        i,
                                                        &superTarget,
                                                        reversedComponentSelection);
                if (gcmIS_ERROR(status)) return status;

/* KLC - may be able to break out from the loop instead of continue */
                if(superTarget.numTargets == 0) continue;
                if(lSize == 1) {
                    status = _ConvROperandToSectionalSourceReg(Compiler,
                                                               ROperand,
                                                               0,
                                                               superTarget.numTargets,
                                                               reversedComponentSelection,
                                                               &superSource);
                    if (gcmIS_ERROR(status)) return status;
                }
                else {
                    status = _ConvROperandToSectionalSourceReg(Compiler,
                                                               ROperand,
                                                               i,
                                                               superTarget.numTargets,
                                                               reversedComponentSelection,
                                                               &superSource);
                    if (gcmIS_ERROR(status)) return status;
                }
                _SplitTargets(&superTarget, superSource.numSources);
                _SplitSources(&superSource, superTarget.numTargets);
                gcmASSERT(superTarget.numTargets == superSource.numSources);

                for(j=0; j < superTarget.numTargets; j++) {
                    status = clEmitAssignCode(Compiler,
                                              LineNo,
                                              StringNo,
                                              superTarget.targets + j,
                                              superSource.sources + j,
                                                LOperand->reg.isUnionMember);
                    if (gcmIS_ERROR(status)) return status;
                }
            }
        }
        else {
            if (ROperand->u.constant.allValuesEqual) {
#if cldVector8And16
                status = _ConvLOperandToSuperTarget(Compiler,
                                                    LOperand,
                                                    &superTarget,
                                                    reversedComponentSelection);
                if (gcmIS_ERROR(status)) return status;
#else
                _clmConvLOperandToSuperTarget(Compiler,
                                              LOperand,
                                              &superTarget,
                                              reversedComponentSelection,
                                              status);
                if(gcmIS_ERROR(status)) return status;
#endif
                status = _ConvROperandToSpecialVectorSourceConstant(Compiler,
                                                                    ROperand,
                                                                    superSource.sources);
                if(gcmIS_ERROR(status)) return status;

                for(i = 0; i < superTarget.numTargets; i++) {
                    status = clEmitAssignCode(Compiler,
                                              LineNo,
                                              StringNo,
                                              superTarget.targets + i,
                                              superSource.sources,
                                              LOperand->reg.isUnionMember);
                    if (gcmIS_ERROR(status)) return status;
                }
            }
            else {
                gctUINT limit;

                limit = gcGetVectorDataTypeComponentCount(LOperand->dataType);
                for (i = 0; i < limit; i++) {
                    status = _ConvLOperandToVectorComponentTarget(Compiler,
                                                                  LOperand,
                                                                  i,
                                                                  superTarget.targets);
                    if (gcmIS_ERROR(status)) return status;

                    status = _ConvROperandToVectorComponentSourceConstant(Compiler,
                                                                          ROperand,
                                                                          i,
                                                                          superSource.sources);
                    if (gcmIS_ERROR(status)) return status;

                    status = clEmitAssignCode(Compiler,
                                              LineNo,
                                              StringNo,
                                              superTarget.targets,
                                              superSource.sources,
                                              LOperand->reg.isUnionMember);
                    if (gcmIS_ERROR(status)) return status;
                }
            }
        }
    }
    else
    {
        gcmASSERT(gcIsMatrixDataType(LOperand->dataType));

        if (ROperand->isReg) {
            for (i = 0; i < gcGetMatrixDataTypeColumnCount(LOperand->dataType); i++) {
                status = _ConvLOperandToMatrixColumnSuperTarget(Compiler,
                                                                LOperand,
                                                                i,
                                                                &superTarget);
                if (gcmIS_ERROR(status)) return status;

                status = _ConvROperandToMatrixColumnSuperSourceReg(Compiler,
                                                                   ROperand,
                                                                   i,
                                                                   &superSource);
                if (gcmIS_ERROR(status)) return status;

                _SplitTargets(&superTarget, superSource.numSources);
                _SplitSources(&superSource, superTarget.numTargets);
                gcmASSERT(superTarget.numTargets == superSource.numSources);

                for(j=0; j < superTarget.numTargets; j++) {
                    status = clEmitAssignCode(Compiler,
                                              LineNo,
                                              StringNo,
                                              superTarget.targets + j,
                                              superSource.sources + j,
                                              LOperand->reg.isUnionMember);
                    if (gcmIS_ERROR(status)) return status;
                }
            }
        }
        else {
            for (i = 0; i < gcGetMatrixDataTypeColumnCount(LOperand->dataType); i++) {
                for (j = 0; j < gcGetMatrixDataTypeRowCount(LOperand->dataType); j++) {
                    status = _ConvLOperandToMatrixComponentTarget(Compiler,
                                                                  LOperand,
                                                                  i,
                                                                  j,
                                                                  superTarget.targets);
                    if (gcmIS_ERROR(status)) return status;

                    status = _ConvROperandToMatrixComponentSourceConstant(Compiler,
                                                                          ROperand,
                                                                          i,
                                                                          j,
                                                                          superSource.sources);
                    if (gcmIS_ERROR(status)) return status;

                    status = clEmitAssignCode(Compiler,
                                              LineNo,
                                              StringNo,
                                              superTarget.targets,
                                              superSource.sources,
                                              LOperand->reg.isUnionMember);
                    if (gcmIS_ERROR(status)) return status;
                }
            }
        }
    }

    return gcvSTATUS_OK;
}

static gceSTATUS
_GenIndexAddCode(
    IN cloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gctREG_INDEX TargetIndexRegIndex,
    IN gctREG_INDEX SourceIndexRegIndex,
    IN clsROPERAND * ROperand
    );

static gctBOOL
_IsDefaultComponentSelection(
IN clsCOMPONENT_SELECTION  *ComponentSelection
)
{
    gctUINT i;

    for(i = 0; i < ComponentSelection->components; i++) {
        if(ComponentSelection->selection[i] != i) return gcvFALSE;
    }
    return gcvTRUE;
}

static gceSTATUS
_ConvROperandToSuperTarget(
    IN cloCOMPILER Compiler,
    IN clsROPERAND *ROperand,
    OUT gcsSUPER_TARGET *SuperTarget
    )
{
    clsLOPERAND lOperand[1];
    clsCOMPONENT_SELECTION    reversedComponentSelection[1];

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    gcmASSERT(ROperand);
    gcmASSERT(!gcIsMatrixDataType(ROperand->dataType));
    gcmASSERT(SuperTarget);

    if(ROperand->isReg &&
       !_IsDefaultComponentSelection(&ROperand->u.reg.componentSelection)) {
         gceSTATUS status;
         clsIOPERAND intermIOperand[1];
         clsIOPERAND_New(Compiler, intermIOperand, ROperand->dataType);
         clsLOPERAND_InitializeUsingIOperand(lOperand, intermIOperand);

         status = clGenAssignCode(Compiler,
                      0,
                      0,
                      lOperand,
                      ROperand);
         if (gcmIS_ERROR(status)) return status;
    }
    else clsLOPERAND_InitializeUsingROperand(lOperand, ROperand);

    return _ConvLOperandToSuperTarget(Compiler,
                          lOperand,
                          SuperTarget,
                          reversedComponentSelection);
}


static gceSTATUS
_ConvIOperandToTarget(
    IN cloCOMPILER Compiler,
    IN clsIOPERAND * IOperand,
    OUT gcsTARGET * Target
    )
{
    clsLOPERAND lOperand;
    clsCOMPONENT_SELECTION    reversedComponentSelection;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    gcmASSERT(IOperand);
    gcmASSERT(!gcIsMatrixDataType(IOperand->dataType));
    gcmASSERT(Target);

    clsLOPERAND_InitializeUsingIOperand(&lOperand, IOperand);

    return _ConvLOperandToTarget(Compiler,
                     &lOperand,
                     Target,
                     &reversedComponentSelection);
}

static gceSTATUS
_ConvIOperandToSuperTarget(
    IN cloCOMPILER Compiler,
    IN clsIOPERAND * IOperand,
    OUT gcsSUPER_TARGET * SuperTarget
    )
{
    clsLOPERAND lOperand;
    clsCOMPONENT_SELECTION    reversedComponentSelection;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    gcmASSERT(IOperand);
    gcmASSERT(!gcIsMatrixDataType(IOperand->dataType));
    gcmASSERT(SuperTarget);

    clsLOPERAND_InitializeUsingIOperand(&lOperand, IOperand);

    return _ConvLOperandToSuperTarget(Compiler,
                          &lOperand,
                          SuperTarget,
                          &reversedComponentSelection);
}

static gceSTATUS
_ConvIOperandToSectionalTarget(
    IN cloCOMPILER Compiler,
    IN clsIOPERAND * IOperand,
    IN gctUINT8 SectionIndex,
    OUT gcsSUPER_TARGET * SuperTarget
    )
{
    clsLOPERAND lOperand[1];
    clsCOMPONENT_SELECTION    reversedComponentSelection[cldMaxFourComponentCount];

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    gcmASSERT(IOperand);
    gcmASSERT(!gcIsMatrixDataType(IOperand->dataType));
    gcmASSERT(SuperTarget);

    clsLOPERAND_InitializeUsingIOperand(lOperand, IOperand);

    return _ConvLOperandToSectionalTarget(Compiler,
                                          lOperand,
                                          SectionIndex,
                                          SuperTarget,
                                          reversedComponentSelection);
}

static gceSTATUS
_ConvIOperandToVectorComponentTarget(
    IN cloCOMPILER Compiler,
    IN clsIOPERAND * IOperand,
    IN gctUINT VectorIndex,
    OUT gcsTARGET *Target
    )
{
    clsLOPERAND lOperand[1];

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    gcmASSERT(IOperand);
    gcmASSERT(gcIsVectorDataType(IOperand->dataType));
    gcmASSERT(Target);

    clsLOPERAND_InitializeUsingIOperand(lOperand, IOperand);

    return  _ConvLOperandToVectorComponentTarget(Compiler,
                             lOperand,
                             VectorIndex,
                             Target);
}

static gceSTATUS
_ConvIOperandToMatrixColumnTarget(
    IN cloCOMPILER Compiler,
    IN clsIOPERAND * IOperand,
    IN gctUINT MatrixIndex,
    OUT gcsTARGET * Target
    )
{
    clsIOPERAND    columnIOperand[1];

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    gcmASSERT(IOperand);
    gcmASSERT(gcIsMatrixDataType(IOperand->dataType));
    gcmASSERT(Target);

    clsIOPERAND_InitializeAsMatrixColumn(columnIOperand, IOperand, MatrixIndex);

    return _ConvIOperandToTarget(Compiler,
                     columnIOperand,
                     Target);
}

static gceSTATUS
_ConvIOperandToMatrixColumnSuperTarget(
    IN cloCOMPILER Compiler,
    IN clsIOPERAND * IOperand,
    IN gctUINT MatrixIndex,
    OUT gcsSUPER_TARGET * SuperTarget
    )
{
    clsIOPERAND    columnIOperand[1];

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    gcmASSERT(IOperand);
    gcmASSERT(gcIsMatrixDataType(IOperand->dataType));
    gcmASSERT(SuperTarget);

    clsIOPERAND_InitializeAsMatrixColumn(columnIOperand, IOperand, MatrixIndex);

    return _ConvIOperandToSuperTarget(Compiler,
                          columnIOperand,
                          SuperTarget);
}

static gceSTATUS
_ConvNormalROperandToSource(
    IN cloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN clsROPERAND * ROperand,
    OUT gcsSOURCE * Source
    )
{
    gceSTATUS    status;
    clsIOPERAND    intermIOperand;
    clsLOPERAND    intermLOperand;
    clsROPERAND    intermROperand;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    gcmASSERT(ROperand);
    gcmASSERT(!gcIsMatrixDataType(ROperand->dataType));
    gcmASSERT(Source);

    /* Assign the non-scalar constant operand to a new register operand */
    if (!ROperand->isReg && gcIsScalarDataType(ROperand->dataType))
    {
        status = _ConvROperandToSourceConstant(Compiler,
                            ROperand,
                            Source);
        if (gcmIS_ERROR(status)) return status;
    }
    else if (!ROperand->isReg &&
             (gcIsScalarDataType(ROperand->dataType) ||
              (gcIsVectorDataType(ROperand->dataType) && ROperand->u.constant.allValuesEqual)))
    {
        status = _ConvROperandToSpecialVectorSourceConstant(Compiler,
                                                            ROperand,
                                                            Source);
        if (gcmIS_ERROR(status)) return status;
    }
    else
    {
        if (!ROperand->isReg)
        {
            gcmASSERT(gcIsVectorDataType(ROperand->dataType));

            clsIOPERAND_New(Compiler, &intermIOperand, ROperand->dataType);
            clsLOPERAND_InitializeUsingIOperand(&intermLOperand, &intermIOperand);

            status = clGenAssignCode(Compiler,
                        LineNo,
                        StringNo,
                        &intermLOperand,
                        ROperand);

            if (gcmIS_ERROR(status)) return status;

            clsROPERAND_InitializeUsingIOperand(&intermROperand, &intermIOperand);
            ROperand = &intermROperand;
        }

        status = _ConvROperandToSourceReg(Compiler,
                              ROperand,
                              clGetDefaultComponentSelection(ROperand->dataType),
                              Source);
        if (gcmIS_ERROR(status)) return status;
    }

    return gcvSTATUS_OK;
}

static gceSTATUS
_ConvNormalROperandToSuperSource(
    IN cloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN clsROPERAND * ROperand,
    OUT gcsSUPER_SOURCE * SuperSource
    )
{
    gceSTATUS    status;
    clsIOPERAND    intermIOperand;
    clsLOPERAND    intermLOperand;
    clsROPERAND    intermROperand;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    gcmASSERT(ROperand);
    gcmASSERT(!gcIsMatrixDataType(ROperand->dataType));
    gcmASSERT(SuperSource);

    /* Assign the non-scalar constant operand to a new register operand */
    if (!ROperand->isReg && gcIsScalarDataType(ROperand->dataType))
    {
        _clmConvROperandToSuperSourceConstant(Compiler,
                                              ROperand,
                                              SuperSource,
                                              status);
        if(gcmIS_ERROR(status)) return status;
    }
    else if (!ROperand->isReg &&
             (gcIsScalarDataType(ROperand->dataType) ||
              (gcIsVectorDataType(ROperand->dataType) && ROperand->u.constant.allValuesEqual)))
    {
        _clmConvROperandToSpecialVectorSuperSourceConstant(Compiler,
                                                           ROperand,
                                                           SuperSource,
                                                           status);
        if(gcmIS_ERROR(status)) return status;
    }
    else
    {
        if (!ROperand->isReg)
        {
            gcmASSERT(gcIsVectorDataType(ROperand->dataType));

            clsIOPERAND_New(Compiler, &intermIOperand, ROperand->dataType);
            clsLOPERAND_InitializeUsingIOperand(&intermLOperand, &intermIOperand);

            status = clGenAssignCode(Compiler,
                                     LineNo,
                                     StringNo,
                                     &intermLOperand,
                                     ROperand);

            if (gcmIS_ERROR(status)) return status;

            clsROPERAND_InitializeUsingIOperand(&intermROperand, &intermIOperand);
            ROperand = &intermROperand;
        }

        status = _ConvROperandToSuperSourceReg(Compiler,
                                               ROperand,
                                               clGetDefaultComponentSelection(ROperand->dataType),
                                               SuperSource);
        if (gcmIS_ERROR(status)) return status;
    }

    return gcvSTATUS_OK;
}

static gceSTATUS
_ConvNormalROperandToSectionalSource(
IN cloCOMPILER Compiler,
IN gctUINT LineNo,
IN gctUINT StringNo,
IN clsROPERAND * ROperand,
IN gctUINT8 SectionIndex,
IN gctUINT8 NumTargets,
OUT gcsSUPER_SOURCE * SuperSource
)
{
    gceSTATUS    status;
    clsIOPERAND    intermIOperand;
    clsLOPERAND    intermLOperand;
    clsROPERAND    intermROperand;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    gcmASSERT(ROperand);
    gcmASSERT(!gcIsMatrixDataType(ROperand->dataType));
    gcmASSERT(SuperSource);

    /* Assign the non-scalar constant operand to a new register operand */
    if (!ROperand->isReg && gcIsScalarDataType(ROperand->dataType))
    {
        _clmConvROperandToSuperSourceConstant(Compiler,
                              ROperand,
                              SuperSource,
                              status);
        if (gcmIS_ERROR(status)) return status;
    }
    else if (!ROperand->isReg &&
             (gcIsScalarDataType(ROperand->dataType) ||
              (gcIsVectorDataType(ROperand->dataType) && ROperand->u.constant.allValuesEqual)))
    {
        _clmConvROperandToSpecialVectorSuperSourceConstant(Compiler,
                                                           ROperand,
                                                           SuperSource,
                                                       status);
        if (gcmIS_ERROR(status)) return status;
    }
    else
    {
        clsCOMPONENT_SELECTION    defaultSelection[cldMaxFourComponentCount];

        if (!ROperand->isReg)
        {
            gcmASSERT(gcIsVectorDataType(ROperand->dataType));

            clsIOPERAND_New(Compiler, &intermIOperand, ROperand->dataType);
            clsLOPERAND_InitializeUsingIOperand(&intermLOperand, &intermIOperand);

            status = clGenAssignCode(Compiler,
                        LineNo,
                        StringNo,
                        &intermLOperand,
                        ROperand);

            if (gcmIS_ERROR(status)) return status;

            clsROPERAND_InitializeUsingIOperand(&intermROperand, &intermIOperand);
            ROperand = &intermROperand;
        }

        _FillDefaultComponentSelection(ROperand->dataType, defaultSelection);
        gcmVERIFY_OK(_ConvROperandToSectionalSourceReg(Compiler,
                                       ROperand,
                                   SectionIndex,
                                   NumTargets,
                                   defaultSelection,
                                       SuperSource));
    }

    return gcvSTATUS_OK;
}

static gceSTATUS
_ConvNormalROperandToMatrixColumnSource(
    IN cloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN clsROPERAND * ROperand,
    IN gctUINT MatrixIndex,
    OUT gcsSOURCE * Source
    )
{
    clsROPERAND columnROperand[1];

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    gcmASSERT(ROperand);
    gcmASSERT(gcIsMatrixDataType(ROperand->dataType));
    gcmASSERT(ROperand->matrixIndex.mode == clvINDEX_NONE);
    gcmASSERT(Source);

    clsROPERAND_InitializeAsMatrixColumn(columnROperand,
                         ROperand,
                         MatrixIndex);

    return _ConvNormalROperandToSource(Compiler,
                       LineNo,
                       StringNo,
                       columnROperand,
                       Source);
}

static gceSTATUS
_ConvNormalROperandToMatrixColumnSuperSource(
    IN cloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN clsROPERAND * ROperand,
    IN gctUINT MatrixIndex,
    OUT gcsSUPER_SOURCE * SuperSource
    )
{
    clsROPERAND columnROperand[1];

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    gcmASSERT(ROperand);
    gcmASSERT(gcIsMatrixDataType(ROperand->dataType));
    gcmASSERT(ROperand->matrixIndex.mode == clvINDEX_NONE);
    gcmASSERT(SuperSource);

    clsROPERAND_InitializeAsMatrixColumn(columnROperand,
                         ROperand,
                         MatrixIndex);

    return _ConvNormalROperandToSuperSource(Compiler,
                            LineNo,
                            StringNo,
                            columnROperand,
                            SuperSource);
}

static gceSTATUS
_GenMatrixMulVectorCode(
    IN cloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN clsIOPERAND * IOperand,
    IN clsROPERAND * ROperand0,
    IN clsROPERAND * ROperand1
    );

static gctBOOL
_IsConstantValue(
IN clsROPERAND *Operand,
IN gctINT Value
)
{
   gctBOOL yes = gcvFALSE;
   gcmASSERT(Operand);

   if(!Operand->isReg) {
      switch(clmGEN_CODE_elementType_GET(Operand->dataType)) {
      case clvTYPE_UINT:
      case clvTYPE_UCHAR:
      case clvTYPE_USHORT:
           yes = Operand->u.constant.values[0].uintValue == (gctUINT)Value;
           break;

      case clvTYPE_INT:
      case clvTYPE_CHAR:
      case clvTYPE_SHORT:
           yes = Operand->u.constant.values[0].intValue == (gctINT)Value;
           break;

      case clvTYPE_FLOAT:
           yes = Operand->u.constant.values[0].floatValue == (gctFLOAT)Value;
           break;

      case clvTYPE_BOOL:
           yes = Operand->u.constant.values[0].boolValue == (gctBOOL)Value;
           break;
      default:
           break;
      }
      if(yes) {
         if(clmGEN_CODE_IsVectorDataType(Operand->dataType)) {
             if(!Operand->u.constant.allValuesEqual) {
                return gcvFALSE;
             }
         }
         else if(!clmGEN_CODE_IsScalarDataType(Operand->dataType)) {
                return gcvFALSE;
         }
      }
   }
   return yes;
}

#if cldVector8And16
gceSTATUS
clGenArithmeticExprCode(
    IN cloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN cleOPCODE Opcode,
    IN clsIOPERAND * IOperand,
    IN clsROPERAND * ROperand0,
    IN clsROPERAND * ROperand1
    )
{
    gceSTATUS    status = gcvSTATUS_MORE_DATA;
    gcsSUPER_TARGET    superTarget;
    gcsSUPER_SOURCE    superSource0;
    gcsSUPER_SOURCE    superSource1;
    gctUINT        i, j;
    clsIOPERAND    columnIOperand[1];
    clsIOPERAND    intermIOperand[1];
    clsROPERAND    columnROperand1[1];
    clsROPERAND    intermROperand[1];
    clsLOPERAND    lOperand[1];

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    gcmASSERT(IOperand);
    gcmASSERT(ROperand0);
    gcmASSERT(ROperand1);

    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                    clvDUMP_CODE_GENERATOR,
                    "<OPERATION line=\"%d\" string=\"%d\" type=\"%s\">",
                    LineNo,
                    StringNo,
                    clGetOpcodeName(Opcode)));

    gcmVERIFY_OK(clsIOPERAND_Dump(Compiler, IOperand));

    gcmVERIFY_OK(clsROPERAND_Dump(Compiler, ROperand0));

    gcmVERIFY_OK(clsROPERAND_Dump(Compiler, ROperand1));

    do {
      /* check for matrix multiply */
      if (Opcode == clvOPCODE_MUL || Opcode == clvOPCODE_FMUL) {
        if(gcIsVectorDataType(ROperand0->dataType) && gcIsMatrixDataType(ROperand1->dataType)) {
          gctUINT8 numComponent;

          numComponent = gcGetVectorDataTypeComponentCount(IOperand->dataType);
          gcmASSERT(numComponent == gcGetVectorDataTypeComponentCount(ROperand0->dataType));
          gcmASSERT(numComponent == gcGetMatrixDataTypeColumnCount(ROperand1->dataType));

          if(numComponent > 4) {
             clsGEN_CODE_DATA_TYPE dataType;
             gctUINT limit;

             dataType = clGetSubsetDataType(IOperand->dataType, 1);
             clsIOPERAND_New(Compiler, intermIOperand, dataType);
             clsROPERAND_InitializeUsingIOperand(intermROperand, intermIOperand);

             limit = gcGetVectorDataTypeComponentCount(IOperand->dataType);
             for (i = 0; i < limit; i++) {
               clsROPERAND_InitializeAsMatrixColumn(columnROperand1, ROperand1, i);
               status = clGenDotCode(Compiler,
                                     LineNo,
                                     StringNo,
                                     intermIOperand,
                                     ROperand0,
                                     columnROperand1);

               _ConvIOperandToVectorComponentTarget(Compiler,
                                                    IOperand,
                                                    i,
                                                    superTarget.targets);
               if (gcmIS_ERROR(status)) return status;

               status = _ConvNormalROperandToSource(Compiler,
                                                    LineNo,
                                                    StringNo,
                                                    intermROperand,
                                                    superSource0.sources);
               if (gcmIS_ERROR(status)) return status;

               status = clEmitAssignCode(Compiler,
                                         LineNo,
                                         StringNo,
                                         superTarget.targets,
                                         superSource0.sources,
                                         gcvFALSE);
               if (gcmIS_ERROR(status)) return status;
            }
          }
          else {
            gctUINT limit;

            status = _ConvNormalROperandToSuperSource(Compiler,
                                                      LineNo,
                                                      StringNo,
                                                      ROperand0,
                                                      &superSource0);
            if (gcmIS_ERROR(status)) return status;

            limit = gcGetVectorDataTypeComponentCount(IOperand->dataType);
            for (i = 0; i < limit; i++) {
              status = _ConvIOperandToVectorComponentTarget(Compiler,
                                                            IOperand,
                                                            i,
                                                            superTarget.targets);
              if (gcmIS_ERROR(status)) return status;

              status = _ConvNormalROperandToMatrixColumnSource(Compiler,
                                                               LineNo,
                                                               StringNo,
                                                               ROperand1,
                                                               i,
                                                               superSource1.sources);
              if (gcmIS_ERROR(status)) return status;

              status = clEmitCode2(Compiler,
                                   LineNo,
                                   StringNo,
                                   clvOPCODE_DOT,
                                   superTarget.targets,
                                   superSource0.sources,
                                   superSource1.sources);
              if (gcmIS_ERROR(status)) return status;
            }
          }
        }
        else if (gcIsMatrixDataType(ROperand0->dataType) && gcIsVectorDataType(ROperand1->dataType)) {
          status = _GenMatrixMulVectorCode(Compiler,
                                           LineNo,
                                           StringNo,
                                           IOperand,
                                           ROperand0,
                                           ROperand1);
          if (gcmIS_ERROR(status)) return status;
        }
        else if (gcIsMatrixDataType(ROperand0->dataType) && gcIsMatrixDataType(ROperand1->dataType)) {
          gctUINT limit;
          gcmASSERT(gcGetMatrixDataTypeRowCount(IOperand->dataType)
                    == gcGetMatrixDataTypeRowCount(ROperand0->dataType));
          gcmASSERT(gcGetMatrixDataTypeColumnCount(IOperand->dataType)
                    == gcGetMatrixDataTypeColumnCount(ROperand1->dataType));
          gcmASSERT(gcGetMatrixDataTypeColumnCount(ROperand0->dataType)
                    == gcGetMatrixDataTypeRowCount(ROperand1->dataType));

          limit = gcGetMatrixDataTypeColumnCount(IOperand->dataType);
          for (i = 0; i < limit; i++) {
            clsIOPERAND_InitializeAsMatrixColumn(columnIOperand, IOperand, i);
            clsROPERAND_InitializeAsMatrixColumn(columnROperand1, ROperand1, i);

            status = _GenMatrixMulVectorCode(Compiler,
                                             LineNo,
                                             StringNo,
                                             columnIOperand,
                                             ROperand0,
                                             columnROperand1);
            if (gcmIS_ERROR(status)) return status;
          }
        }
      }

      if(status != gcvSTATUS_MORE_DATA) break;

          /* Not matrix multiply: proceed forward */
      switch(Opcode) {
      case clvOPCODE_MUL:
      case clvOPCODE_FMUL:
      case clvOPCODE_MUL_RTZ:
      case clvOPCODE_MUL_RTNE:
            if(clmGEN_CODE_IsScalarDataType(ROperand0->dataType)) {
               if(_IsConstantValue(ROperand0, 1)) {
                  clsLOPERAND_InitializeUsingIOperand(lOperand, IOperand);
                  status =  clGenAssignCode(Compiler,
                                            LineNo,
                                            StringNo,
                                            lOperand,
                                        ROperand1);
              if (gcmIS_ERROR(status)) return status;
               }
               else if(_IsConstantValue(ROperand0, 0)) {
                  clsLOPERAND_InitializeUsingIOperand(lOperand, IOperand);
                  status =  clGenAssignCode(Compiler,
                                            LineNo,
                                            StringNo,
                                            lOperand,
                                        ROperand0);
              if (gcmIS_ERROR(status)) return status;
               }
            }
            else if(clmGEN_CODE_IsScalarDataType(ROperand1->dataType)) {
               if(_IsConstantValue(ROperand1, 1)) {
                  clsLOPERAND_InitializeUsingIOperand(lOperand, IOperand);
                  status =  clGenAssignCode(Compiler,
                                            LineNo,
                                            StringNo,
                                            lOperand,
                                        ROperand0);
              if (gcmIS_ERROR(status)) return status;
               }
               else if(_IsConstantValue(ROperand1, 0)) {
                  clsLOPERAND_InitializeUsingIOperand(lOperand, IOperand);
                  status =  clGenAssignCode(Compiler,
                                            LineNo,
                                            StringNo,
                                            lOperand,
                                        ROperand1);
              if (gcmIS_ERROR(status)) return status;
               }
        }
        break;

      case clvOPCODE_ADD:
      case clvOPCODE_FADD:
      case clvOPCODE_ADD_RTZ:
      case clvOPCODE_ADD_RTNE:
             if(_IsConstantValue(ROperand0, 0)) {
                clsLOPERAND_InitializeUsingIOperand(lOperand, IOperand);
                status = clGenAssignCode(Compiler,
                                         LineNo,
                                         StringNo,
                                         lOperand,
                                     ROperand1);
            if (gcmIS_ERROR(status)) return status;
             }
             else if(_IsConstantValue(ROperand1, 0)) {
                clsLOPERAND_InitializeUsingIOperand(lOperand, IOperand);
                status = clGenAssignCode(Compiler,
                                         LineNo,
                                         StringNo,
                                         lOperand,
                                     ROperand0);
            if (gcmIS_ERROR(status)) return status;
             }
             break;

      case clvOPCODE_SUB:
      case clvOPCODE_FSUB:
      case clvOPCODE_SUB_RTZ:
      case clvOPCODE_SUB_RTNE:
             if(_IsConstantValue(ROperand1, 0)) {
                clsLOPERAND_InitializeUsingIOperand(lOperand, IOperand);
                status = clGenAssignCode(Compiler,
                                         LineNo,
                                         StringNo,
                                         lOperand,
                                     ROperand0);
            if (gcmIS_ERROR(status)) return status;
             }
             break;

      case clvOPCODE_ADDLO:
      case clvOPCODE_ADDLO_RTZ:
      case clvOPCODE_ADDLO_RTNE:
      case clvOPCODE_MULLO:
      case clvOPCODE_MULLO_RTZ:
      case clvOPCODE_MULLO_RTNE:
      case clvOPCODE_DIV:
      case clvOPCODE_IDIV:
      case clvOPCODE_IMUL:
      case clvOPCODE_MOD:
      case clvOPCODE_FMOD:
      case clvOPCODE_BITWISE_AND:
      case clvOPCODE_BITWISE_OR:
      case clvOPCODE_BITWISE_XOR:

      case clvOPCODE_MULHI:
      case clvOPCODE_ADDSAT:
      case clvOPCODE_SUBSAT:
      case clvOPCODE_MULSAT:
      case clvOPCODE_MADSAT:
         break;

      default: gcmASSERT(0);
         return gcvSTATUS_INVALID_DATA;
      }

      if(status != gcvSTATUS_MORE_DATA) break;

      /* both of same type or both being scalar*/
      if (gcIsDataTypeEqual(ROperand0->dataType, ROperand1->dataType) ||
          (gcIsScalarDataType(ROperand0->dataType) && gcIsScalarDataType(ROperand1->dataType))) {
       /* first operand scalar or vector */
       if (gcIsScalarDataType(ROperand0->dataType) || gcIsVectorDataType(ROperand0->dataType)) {
           gcmVERIFY_OK(_ConvIOperandToSuperTarget(Compiler,
                                   IOperand,
                                   &superTarget));

           status = _ConvNormalROperandToSuperSource(Compiler,
                                     LineNo,
                                     StringNo,
                                     ROperand0,
                                     &superSource0);
           if (gcmIS_ERROR(status)) return status;

           status = _ConvNormalROperandToSuperSource(Compiler,
                                     LineNo,
                                     StringNo,
                                     ROperand1,
                                     &superSource1);
           if (gcmIS_ERROR(status)) return status;
           _SplitTargets(&superTarget,
                     superSource0.numSources > superSource1.numSources
                     ? superSource0.numSources
                     : superSource1.numSources);
           _SplitSources(&superSource0, superTarget.numTargets);
           _SplitSources(&superSource1, superTarget.numTargets);
           gcmASSERT(superTarget.numTargets == superSource0.numSources &&
                 superTarget.numTargets == superSource1.numSources);

           for(i=0; i < superTarget.numTargets; i++) {
               status = clEmitCode2(Compiler,
                        LineNo,
                        StringNo,
                        Opcode,
                        superTarget.targets + i,
                        superSource0.sources + i,
                        superSource1.sources + i);
               if (gcmIS_ERROR(status)) return status;
           }
          }
          else { /* first and second operands being matrix type */
            gcmASSERT(gcIsMatrixDataType(ROperand0->dataType));

            for (i = 0; i < gcGetMatrixDataTypeColumnCount(ROperand0->dataType); i++)
            {
                gcmVERIFY_OK(_ConvIOperandToMatrixColumnSuperTarget(Compiler, IOperand, i, &superTarget));

                status = _ConvNormalROperandToMatrixColumnSuperSource(Compiler,
                                              LineNo,
                                              StringNo,
                                              ROperand0,
                                              i,
                                              &superSource0);

                if (gcmIS_ERROR(status)) return status;

                status = _ConvNormalROperandToMatrixColumnSuperSource(Compiler,
                                              LineNo,
                                              StringNo,
                                              ROperand1,
                                              i,
                                              &superSource1);

                if (gcmIS_ERROR(status)) return status;

                _SplitTargets(&superTarget,
                          superSource0.numSources > superSource1.numSources
                          ? superSource0.numSources
                          : superSource1.numSources);
                _SplitSources(&superSource0, superTarget.numTargets);
                _SplitSources(&superSource1, superTarget.numTargets);
                gcmASSERT(superTarget.numTargets == superSource0.numSources &&
                      superTarget.numTargets == superSource1.numSources);
                for(j=0; j < superTarget.numTargets; j++) {
                    status = clEmitCode2(Compiler,
                                 LineNo,
                                 StringNo,
                                 Opcode,
                                 superTarget.targets + j,
                                 superSource0.sources + j,
                                 superSource1.sources + j);
                    if (gcmIS_ERROR(status)) return status;
                }
            }
          }
      }
      else  /* unequal typed operands */
      {
        if (gcIsMatrixDataType(ROperand0->dataType))
        {
            clsROPERAND rOperand[1];
            clsROPERAND *scalarOperand;

            gcmASSERT(gcIsScalarDataType(ROperand1->dataType));
            if(!clmIsElementTypeFloating(clmGEN_CODE_elementType_GET(ROperand1->dataType))) {
                *rOperand = *ROperand1;
                status = _ImplicitConvertOperand(Compiler,
                                 LineNo,
                                 StringNo,
                                 clmGenCodeDataType(T_FLOAT),
                                 rOperand);
                if (gcmIS_ERROR(status)) return status;
                scalarOperand = rOperand;
            }
            else scalarOperand = ROperand1;

            status = _ConvNormalROperandToSource(Compiler,
                                 LineNo,
                                 StringNo,
                                 scalarOperand,
                                 superSource1.sources);
            if (gcmIS_ERROR(status)) return status;

            for (i = 0; i < gcGetMatrixDataTypeColumnCount(ROperand0->dataType); i++)
            {
                gcmVERIFY_OK(_ConvIOperandToMatrixColumnSuperTarget(Compiler, IOperand, i, &superTarget));

                status = _ConvNormalROperandToMatrixColumnSuperSource(Compiler,
                                              LineNo,
                                              StringNo,
                                              ROperand0,
                                              i,
                                              &superSource0);

                if (gcmIS_ERROR(status)) return status;

                _SplitTargets(&superTarget, superSource0.numSources);
                _SplitSources(&superSource0, superTarget.numTargets);
                gcmASSERT(superTarget.numTargets == superSource0.numSources);

                for(j=0; j < superTarget.numTargets; j++) {
                    status = clEmitCode2(Compiler,
                                 LineNo,
                                 StringNo,
                                 Opcode,
                                 superTarget.targets + j,
                                 superSource0.sources + j,
                                 superSource1.sources);
                    if (gcmIS_ERROR(status)) return status;
                }
            }
        }
        else if (gcIsMatrixDataType(ROperand1->dataType))
        {
            clsROPERAND rOperand[1];
            clsROPERAND *scalarOperand;

            gcmASSERT(gcIsScalarDataType(ROperand0->dataType));
            if(!clmIsElementTypeFloating(clmGEN_CODE_elementType_GET(ROperand0->dataType))) {
                *rOperand = *ROperand0;
                status = _ImplicitConvertOperand(Compiler,
                                 LineNo,
                                 StringNo,
                                 clmGenCodeDataType(T_FLOAT),
                                 rOperand);
                if (gcmIS_ERROR(status)) return status;
                scalarOperand = rOperand;
            }
            else scalarOperand = ROperand0;


            status = _ConvNormalROperandToSuperSource(Compiler,
                                      LineNo,
                                      StringNo,
                                      scalarOperand,
                                      &superSource0);
            if (gcmIS_ERROR(status)) return status;

            for (i = 0; i < gcGetMatrixDataTypeColumnCount(ROperand1->dataType); i++) {
                gcmVERIFY_OK(_ConvIOperandToMatrixColumnSuperTarget(Compiler, IOperand, i, &superTarget));

                status = _ConvNormalROperandToMatrixColumnSuperSource(Compiler,
                                              LineNo,
                                              StringNo,
                                              ROperand1,
                                              i,
                                              &superSource1);
                if (gcmIS_ERROR(status)) return status;

                _SplitTargets(&superTarget, superSource1.numSources);
                _SplitSources(&superSource1, superTarget.numTargets);
                gcmASSERT(superTarget.numTargets == superSource1.numSources);

                for(j=0; j < superTarget.numTargets; j++) {
                    status = clEmitCode2(Compiler,
                                 LineNo,
                                 StringNo,
                                 Opcode,
                                 superTarget.targets + j,
                                 superSource0.sources,
                                 superSource1.sources + j);
                    if (gcmIS_ERROR(status)) return status;
                }
            }
        }
        else {
            gcmASSERT((gcIsVectorDataType(ROperand0->dataType)
                    && gcIsScalarDataType(ROperand1->dataType)) ||
                  (gcIsScalarDataType(ROperand0->dataType)
                    && gcIsVectorDataType(ROperand1->dataType)));

            gcmVERIFY_OK(_ConvIOperandToSuperTarget(Compiler,
                                    IOperand,
                                    &superTarget));

            status = _ConvNormalROperandToSuperSource(Compiler,
                                      LineNo,
                                      StringNo,
                                      ROperand0,
                                           &superSource0);
            if (gcmIS_ERROR(status)) return status;

            status = _ConvNormalROperandToSuperSource(Compiler,
                                      LineNo,
                                      StringNo,
                                      ROperand1,
                                      &superSource1);
            if (gcmIS_ERROR(status)) return status;
            if(gcIsScalarDataType(ROperand0->dataType)) {
                _SplitTargets(&superTarget, superSource1.numSources);
                _SplitSources(&superSource1, superTarget.numTargets);
                gcmASSERT(superTarget.numTargets == superSource1.numSources);

                for(j=0; j < superTarget.numTargets; j++) {
                    status = clEmitCode2(Compiler,
                                 LineNo,
                                 StringNo,
                                 Opcode,
                                 superTarget.targets + j,
                                 superSource0.sources,
                                 superSource1.sources + j);
                    if (gcmIS_ERROR(status)) return status;
                }
            }
            else {
                _SplitTargets(&superTarget, superSource0.numSources);
                _SplitSources(&superSource0, superTarget.numTargets);
                gcmASSERT(superTarget.numTargets == superSource0.numSources);

                for(j=0; j < superTarget.numTargets; j++) {
                    status = clEmitCode2(Compiler,
                                 LineNo,
                                 StringNo,
                                 Opcode,
                                 superTarget.targets + j,
                                 superSource0.sources + j,
                                 superSource1.sources);
                    if (gcmIS_ERROR(status)) return status;
                }
            }
         }
      }

    } while (gcvFALSE);

    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                clvDUMP_CODE_GENERATOR,
                "</OPERATION>"));
    return gcvSTATUS_OK;
}

gceSTATUS
clGenShiftExprCode(
    IN cloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN cleOPCODE Opcode,
    IN clsIOPERAND * IOperand,
    IN clsROPERAND * ROperand0,
    IN clsROPERAND * ROperand1
    )
{
    gceSTATUS    status;
    gcsSUPER_TARGET    superTarget;
    gcsSUPER_SOURCE    superSource0;
    gcsSUPER_SOURCE    superSource1;
    gctUINT8 i;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    gcmASSERT(IOperand);
    gcmASSERT(ROperand0);
    gcmASSERT(ROperand1);

    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                    clvDUMP_CODE_GENERATOR,
                    "<OPERATION line=\"%d\" string=\"%d\" type=\"%s\">",
                    LineNo,
                    StringNo,
                    clGetOpcodeName(Opcode)));

    gcmVERIFY_OK(clsIOPERAND_Dump(Compiler, IOperand));
    gcmVERIFY_OK(clsROPERAND_Dump(Compiler, ROperand0));
    gcmVERIFY_OK(clsROPERAND_Dump(Compiler, ROperand1));

    gcmASSERT(Opcode == clvOPCODE_RSHIFT || Opcode == clvOPCODE_LSHIFT);

    if (gcIsScalarDataType(ROperand0->dataType) || gcIsVectorDataType(ROperand0->dataType))
    {
        gcmVERIFY_OK(_ConvIOperandToSuperTarget(Compiler,
                                IOperand,
                                &superTarget));

        status = _ConvNormalROperandToSuperSource(Compiler,
                              LineNo,
                              StringNo,
                              ROperand0,
                              &superSource0);
        if (gcmIS_ERROR(status)) return status;

        status = _ConvNormalROperandToSuperSource(Compiler,
                              LineNo,
                              StringNo,
                              ROperand1,
                              &superSource1);
        if (gcmIS_ERROR(status)) return status;
        _SplitTargets(&superTarget,
                  superSource0.numSources > superSource1.numSources
                  ? superSource0.numSources
                  : superSource1.numSources);
        _SplitSources(&superSource0, superTarget.numTargets);
        _SplitSources(&superSource1, superTarget.numTargets);
        gcmASSERT(superTarget.numTargets == superSource0.numSources &&
                superTarget.numTargets == superSource1.numSources);

        for(i=0; i < superTarget.numTargets; i++) {
            status = clEmitCode2(Compiler,
                         LineNo,
                         StringNo,
                         Opcode,
                         superTarget.targets + i,
                         superSource0.sources + i,
                         superSource1.sources + i);
            if (gcmIS_ERROR(status)) return status;
        }
    }
    else
    {
        gcmASSERT(0);
    }

    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                clvDUMP_CODE_GENERATOR,
                "</OPERATION>"));

    return gcvSTATUS_OK;
}

gceSTATUS
clGenBitwiseExprCode(
    IN cloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN cleOPCODE Opcode,
    IN clsIOPERAND * IOperand,
    IN clsROPERAND * ROperand0,
    IN clsROPERAND * ROperand1
    )
{
    gceSTATUS    status;
    gcsSUPER_TARGET    superTarget;
    gcsSUPER_SOURCE    superSource0;
    gcsSUPER_SOURCE    superSource1;


    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    gcmASSERT(IOperand);
    gcmASSERT(ROperand0);
    gcmASSERT(ROperand1);

    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                    clvDUMP_CODE_GENERATOR,
                    "<OPERATION line=\"%d\" string=\"%d\" type=\"%s\">",
                    LineNo,
                    StringNo,
                    clGetOpcodeName(Opcode)));

    gcmVERIFY_OK(clsIOPERAND_Dump(Compiler, IOperand));
    gcmVERIFY_OK(clsROPERAND_Dump(Compiler, ROperand0));
    gcmVERIFY_OK(clsROPERAND_Dump(Compiler, ROperand1));

    gcmASSERT(Opcode == clvOPCODE_BITWISE_AND || Opcode == clvOPCODE_BITWISE_OR || Opcode == clvOPCODE_BITWISE_XOR);

    if (gcIsDataTypeEqual(ROperand0->dataType, ROperand1->dataType) ||
        (gcIsScalarDataType(ROperand0->dataType) && gcIsScalarDataType(ROperand1->dataType))) {
        if (gcIsScalarDataType(ROperand0->dataType) || gcIsVectorDataType(ROperand0->dataType)) {
            gctUINT i;

            gcmVERIFY_OK(_ConvIOperandToSuperTarget(Compiler,
                                    IOperand,
                                    &superTarget));

            status = _ConvNormalROperandToSuperSource(Compiler,
                                  LineNo,
                                  StringNo,
                                  ROperand0,
                                  &superSource0);
            if (gcmIS_ERROR(status)) return status;

            status = _ConvNormalROperandToSuperSource(Compiler,
                                  LineNo,
                                  StringNo,
                                  ROperand1,
                                  &superSource1);
            if (gcmIS_ERROR(status)) return status;

            _SplitTargets(&superTarget,
                      superSource0.numSources > superSource1.numSources
                      ? superSource0.numSources
                      : superSource1.numSources);
            _SplitSources(&superSource0, superTarget.numTargets);
            _SplitSources(&superSource1, superTarget.numTargets);

            gcmASSERT(superTarget.numTargets == superSource0.numSources &&
                  superTarget.numTargets == superSource1.numSources);
            for(i=0; i < superTarget.numTargets; i++) {
                status = clEmitCode2(Compiler,
                                 LineNo,
                                 StringNo,
                                 Opcode,
                                 superTarget.targets + i,
                                 superSource0.sources + i,
                                 superSource1.sources + i);
                if (gcmIS_ERROR(status)) return status;
            }
        }
        else {
            gctUINT i;
            gcmASSERT(gcIsMatrixDataType(ROperand0->dataType));

            for (i = 0; i < gcGetMatrixDataTypeColumnCount(ROperand0->dataType); i++)
            {
                gcmVERIFY_OK(_ConvIOperandToMatrixColumnTarget(Compiler, IOperand, i, superTarget.targets));

                status = _ConvNormalROperandToMatrixColumnSource(Compiler,
                                         LineNo,
                                         StringNo,
                                         ROperand0,
                                         i,
                                         superSource0.sources);
                if (gcmIS_ERROR(status)) return status;

                status = _ConvNormalROperandToMatrixColumnSource(Compiler,
                                         LineNo,
                                         StringNo,
                                         ROperand1,
                                         i,
                                         superSource1.sources);
                if (gcmIS_ERROR(status)) return status;

                status = clEmitCode2(Compiler,
                             LineNo,
                             StringNo,
                             Opcode,
                             superTarget.targets,
                             superSource0.sources,
                             superSource1.sources);
                if (gcmIS_ERROR(status)) return status;
            }
        }
    }
    else {
      if (gcIsMatrixDataType(ROperand0->dataType)) {
        gctUINT i;

        gcmASSERT(gcIsScalarDataType(ROperand1->dataType));

        status = _ConvNormalROperandToSource(Compiler,
                             LineNo,
                             StringNo,
                             ROperand1,
                             superSource1.sources);

        if (gcmIS_ERROR(status)) return status;

        for (i = 0; i < gcGetMatrixDataTypeColumnCount(ROperand0->dataType); i++)
        {
            gcmVERIFY_OK(_ConvIOperandToMatrixColumnTarget(Compiler, IOperand, i, superTarget.targets));

            status = _ConvNormalROperandToMatrixColumnSource(Compiler,
                                     LineNo,
                                     StringNo,
                                     ROperand0,
                                     i,
                                     superSource0.sources);
            if (gcmIS_ERROR(status)) return status;

            status = clEmitCode2(Compiler,
                         LineNo,
                         StringNo,
                         Opcode,
                         superTarget.targets,
                         superSource0.sources,
                         superSource1.sources);
            if (gcmIS_ERROR(status)) return status;
        }
      }
      else if (gcIsMatrixDataType(ROperand1->dataType)) {
        gctUINT i;
        gcmASSERT(gcIsScalarDataType(ROperand0->dataType));

        status = _ConvNormalROperandToSource(Compiler,
                             LineNo,
                             StringNo,
                             ROperand0,
                             superSource0.sources);
        if (gcmIS_ERROR(status)) return status;

        for (i = 0; i < gcGetMatrixDataTypeColumnCount(ROperand1->dataType); i++) {
            gcmVERIFY_OK(_ConvIOperandToMatrixColumnTarget(Compiler, IOperand, i, superTarget.targets));

            status = _ConvNormalROperandToMatrixColumnSource(Compiler,
                                     LineNo,
                                     StringNo,
                                     ROperand1,
                                     i,
                                     superSource1.sources);
            if (gcmIS_ERROR(status)) return status;

            status = clEmitCode2(Compiler,
                         LineNo,
                         StringNo,
                         Opcode,
                         superTarget.targets,
                         superSource0.sources,
                         superSource1.sources);
            if (gcmIS_ERROR(status)) return status;
        }
      }
      else {
        gctUINT i;

        /*  Non-necessary check for GC4000*/
/*          gcmASSERT((gcIsVectorDataType(ROperand0->dataType)
                          && gcIsScalarDataType(ROperand1->dataType))
                              || (gcIsScalarDataType(ROperand0->dataType)
                          && gcIsVectorDataType(ROperand1->dataType)));
*/
        gcmVERIFY_OK(_ConvIOperandToSuperTarget(Compiler,
                            IOperand,
                            &superTarget));

        status = _ConvNormalROperandToSuperSource(Compiler,
                              LineNo,
                              StringNo,
                              ROperand0,
                              &superSource0);
        if (gcmIS_ERROR(status)) return status;

        status = _ConvNormalROperandToSuperSource(Compiler,
                              LineNo,
                              StringNo,
                              ROperand1,
                              &superSource1);
        if (gcmIS_ERROR(status)) return status;

        _SplitTargets(&superTarget,
                  superSource0.numSources > superSource1.numSources
                  ? superSource0.numSources
                  : superSource1.numSources);
        _SplitSources(&superSource0, superTarget.numTargets);
        _SplitSources(&superSource1, superTarget.numTargets);
        gcmASSERT(superTarget.numTargets == superSource0.numSources &&
              superTarget.numTargets == superSource1.numSources);
        for(i=0; i < superTarget.numTargets; i++) {
            status = clEmitCode2(Compiler,
                         LineNo,
                         StringNo,
                         Opcode,
                         superTarget.targets + i,
                         superSource0.sources + i,
                         superSource1.sources + i);
            if (gcmIS_ERROR(status)) return status;
        }

      }
    }

    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                clvDUMP_CODE_GENERATOR,
                "</OPERATION>"));

    return gcvSTATUS_OK;
}
#else
gceSTATUS
clGenArithmeticExprCode(
    IN cloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN cleOPCODE Opcode,
    IN clsIOPERAND * IOperand,
    IN clsROPERAND * ROperand0,
    IN clsROPERAND * ROperand1
    )
{
    gceSTATUS    status;
    gcsTARGET    target;
    gcsSOURCE    source0;
    gcsSOURCE    source1;
    gctUINT        i, j;
    clsIOPERAND    columnIOperand;
    clsROPERAND    columnROperand1;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    gcmASSERT(IOperand);
    gcmASSERT(ROperand0);
    gcmASSERT(ROperand1);

    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                    clvDUMP_CODE_GENERATOR,
                    "<OPERATION line=\"%d\" string=\"%d\" type=\"%s\">",
                    LineNo,
                    StringNo,
                    clGetOpcodeName(Opcode)));

    gcmVERIFY_OK(clsIOPERAND_Dump(Compiler, IOperand));

    gcmVERIFY_OK(clsROPERAND_Dump(Compiler, ROperand0));

    gcmVERIFY_OK(clsROPERAND_Dump(Compiler, ROperand1));

    switch (Opcode)
    {
    case clvOPCODE_ADD:
    case clvOPCODE_FADD:
    case clvOPCODE_ADDLO:
    case clvOPCODE_SUB:
    case clvOPCODE_FSUB:
    case clvOPCODE_MUL:
    case clvOPCODE_FMUL:
    case clvOPCODE_MULLO:
    case clvOPCODE_DIV:
    case clvOPCODE_IDIV:
    case clvOPCODE_IMUL:
    case clvOPCODE_MOD:
    case clvOPCODE_FMOD:
    case clvOPCODE_SELECT:
    case clvOPCODE_BITWISE_AND:
    case clvOPCODE_BITWISE_OR:
    case clvOPCODE_BITWISE_XOR:

    case clvOPCODE_MULHI:
    case clvOPCODE_CMP:
    case clvOPCODE_ADDSAT:
    case clvOPCODE_SUBSAT:
    case clvOPCODE_MULSAT:
    case clvOPCODE_MADSAT:
        break;

    default: gcmASSERT(0);
    }

    if ((Opcode == clvOPCODE_MUL || Opcode == clvOPCODE_FMUL) &&
        gcIsVectorDataType(ROperand0->dataType) && gcIsMatrixDataType(ROperand1->dataType))
    {
        gctUINT limit;

        limit = gcGetVectorDataTypeComponentCount(IOperand->dataType);
        gcmASSERT(limit == gcGetVectorDataTypeComponentCount(ROperand0->dataType));
        gcmASSERT(limit == gcGetMatrixDataTypeColumnCount(ROperand1->dataType));

        status = _ConvNormalROperandToSource(Compiler,
                             LineNo,
                             StringNo,
                             ROperand0,
                             &source0);
        if (gcmIS_ERROR(status)) return status;

        for (i = 0; i < limit; i++)
        {
            status = _ConvIOperandToVectorComponentTarget(Compiler, IOperand, i, &target);
            if (gcmIS_ERROR(status)) return status;

            status = _ConvNormalROperandToMatrixColumnSource(Compiler,
                                    LineNo,
                                    StringNo,
                                    ROperand1,
                                    i,
                                    &source1);

            if (gcmIS_ERROR(status)) return status;

            status = clEmitCode2(Compiler,
                         LineNo,
                         StringNo,
                         clvOPCODE_DOT,
                         &target,
                         &source0,
                         &source1);

            if (gcmIS_ERROR(status)) return status;
        }
    }
    else if ((Opcode == clvOPCODE_MUL || Opcode == clvOPCODE_FMUL) &&
        gcIsMatrixDataType(ROperand0->dataType) && gcIsVectorDataType(ROperand1->dataType))
    {
        status = _GenMatrixMulVectorCode(Compiler,
                        LineNo,
                        StringNo,
                        IOperand,
                        ROperand0,
                        ROperand1);

        if (gcmIS_ERROR(status)) return status;
    }
    else if ((Opcode == clvOPCODE_MUL || Opcode == clvOPCODE_FMUL) &&
        gcIsMatrixDataType(ROperand0->dataType) && gcIsMatrixDataType(ROperand1->dataType))
    {
        gcmASSERT(gcGetMatrixDataTypeColumnCount(IOperand->dataType)
                    == gcGetMatrixDataTypeColumnCount(ROperand0->dataType));
        gcmASSERT(gcGetMatrixDataTypeColumnCount(IOperand->dataType)
                    == gcGetMatrixDataTypeColumnCount(ROperand1->dataType));
        for (i = 0; i < gcGetMatrixDataTypeColumnCount(IOperand->dataType); i++) {
            clsIOPERAND_InitializeAsMatrixColumn(&columnIOperand, IOperand, i);
            clsROPERAND_InitializeAsMatrixColumn(&columnROperand1, ROperand1, i);

            status = _GenMatrixMulVectorCode(Compiler,
                            LineNo,
                            StringNo,
                            &columnIOperand,
                            ROperand0,
                            &columnROperand1);
            if (gcmIS_ERROR(status)) return status;
        }
    }
    else {
        if (gcIsDataTypeEqual(ROperand0->dataType, ROperand1->dataType) ||
            (gcIsScalarDataType(ROperand0->dataType) && gcIsScalarDataType(ROperand1->dataType))) {
            if (gcIsScalarDataType(ROperand0->dataType) || gcIsVectorDataType(ROperand0->dataType)) {
                status = _ConvIOperandToTarget(Compiler,
                                   IOperand,
                                   &target);
                if (gcmIS_ERROR(status)) return status;

                status = _ConvNormalROperandToSource(Compiler,
                                LineNo,
                                StringNo,
                                ROperand0,
                                &source0);

                if (gcmIS_ERROR(status)) return status;

                status = _ConvNormalROperandToSource(Compiler,
                                LineNo,
                                StringNo,
                                ROperand1,
                                &source1);

                if (gcmIS_ERROR(status)) return status;

                status = clEmitCode2(Compiler,
                             LineNo,
                                  StringNo,
                             Opcode,
                             &target,
                             &source0,
                             &source1);

                if (gcmIS_ERROR(status)) return status;
            }
            else {
                gcmASSERT(gcIsMatrixDataType(ROperand0->dataType));

                for (i = 0; i < gcGetMatrixDataTypeColumnCount(ROperand0->dataType); i++)
                {
                    status = _ConvIOperandToMatrixColumnTarget(Compiler, IOperand, i, &target);
                    if (gcmIS_ERROR(status)) return status;

                    status = _ConvNormalROperandToMatrixColumnSource(Compiler,
                                            LineNo,
                                            StringNo,
                                            ROperand0,
                                            i,
                                            &source0);

                    if (gcmIS_ERROR(status)) return status;

                    status = _ConvNormalROperandToMatrixColumnSource(Compiler,
                                            LineNo,
                                            StringNo,
                                            ROperand1,
                                            i,
                                            &source1);

                    if (gcmIS_ERROR(status)) return status;

                    status = clEmitCode2(Compiler,
                            LineNo,
                            StringNo,
                            Opcode,
                            &target,
                            &source0,
                            &source1);

                    if (gcmIS_ERROR(status)) return status;
                }
            }
        }
        else
        {
            if (gcIsMatrixDataType(ROperand0->dataType))
            {
                gcmASSERT(gcIsScalarDataType(ROperand1->dataType));

                status = _ConvNormalROperandToSource(Compiler,
                                LineNo,
                                StringNo,
                                ROperand1,
                                &source1);

                if (gcmIS_ERROR(status)) return status;

                for (i = 0; i < gcGetMatrixDataTypeColumnCount(ROperand0->dataType); i++)
                {
                    status = _ConvIOperandToMatrixColumnTarget(Compiler, IOperand, i, &target);
                    if (gcmIS_ERROR(status)) return status;

                    status = _ConvNormalROperandToMatrixColumnSource(Compiler,
                                            LineNo,
                                            StringNo,
                                            ROperand0,
                                            i,
                                            &source0);

                    if (gcmIS_ERROR(status)) return status;

                    status = clEmitCode2(Compiler,
                            LineNo,
                            StringNo,
                            Opcode,
                            &target,
                            &source0,
                            &source1);

                    if (gcmIS_ERROR(status)) return status;
                }
            }
            else if (gcIsMatrixDataType(ROperand1->dataType))
            {
                gcmASSERT(gcIsScalarDataType(ROperand0->dataType));

                status = _ConvNormalROperandToSource(Compiler,
                                LineNo,
                                StringNo,
                                ROperand0,
                                &source0);

                if (gcmIS_ERROR(status)) return status;

                for (i = 0; i < gcGetMatrixDataTypeColumnCount(ROperand1->dataType); i++) {
                    status = _ConvIOperandToMatrixColumnTarget(Compiler, IOperand, i, &target);
                    if (gcmIS_ERROR(status)) return status;

                    status = _ConvNormalROperandToMatrixColumnSource(Compiler,
                                            LineNo,
                                            StringNo,
                                            ROperand1,
                                            i,
                                            &source1);

                    if (gcmIS_ERROR(status)) return status;

                    status = clEmitCode2(Compiler,
                                 LineNo,
                                 StringNo,
                                 Opcode,
                                 &target,
                                 &source0,
                                 &source1);
                    if (gcmIS_ERROR(status)) return status;
                }
            }
            else {
                if(!((gcIsVectorDataType(ROperand0->dataType)
                                  && gcIsScalarDataType(ROperand1->dataType))
                                      || (gcIsScalarDataType(ROperand0->dataType)
                                  && gcIsVectorDataType(ROperand1->dataType)))) {
                  gcmASSERT((gcIsVectorDataType(ROperand0->dataType)
                                  && gcIsScalarDataType(ROperand1->dataType))
                                      || (gcIsScalarDataType(ROperand0->dataType)
                                  && gcIsVectorDataType(ROperand1->dataType)));
                }

                status = _ConvIOperandToTarget(Compiler,
                                   IOperand,
                                   &target);
                if (gcmIS_ERROR(status)) return status;

                status = _ConvNormalROperandToSource(Compiler,
                                     LineNo,
                                     StringNo,
                                     ROperand0,
                                          &source0);

                if (gcmIS_ERROR(status)) return status;

                status = _ConvNormalROperandToSource(Compiler,
                                     LineNo,
                                     StringNo,
                                     ROperand1,
                                     &source1);
                if (gcmIS_ERROR(status)) return status;

                status = clEmitCode2(Compiler,
                             LineNo,
                             StringNo,
                             Opcode,
                             &target,
                             &source0,
                             &source1);
                if (gcmIS_ERROR(status)) return status;
            }
        }
    }

    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                clvDUMP_CODE_GENERATOR,
                "</OPERATION>"));

    return gcvSTATUS_OK;
}

gceSTATUS
clGenShiftExprCode(
    IN cloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN cleOPCODE Opcode,
    IN clsIOPERAND * IOperand,
    IN clsROPERAND * ROperand0,
    IN clsROPERAND * ROperand1
    )
{
    gceSTATUS    status;
    gcsTARGET    target;
    gcsSOURCE    source0;
    gcsSOURCE    source1;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    gcmASSERT(IOperand);
    gcmASSERT(ROperand0);
    gcmASSERT(ROperand1);

    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                    clvDUMP_CODE_GENERATOR,
                    "<OPERATION line=\"%d\" string=\"%d\" type=\"%s\">",
                    LineNo,
                    StringNo,
                    clGetOpcodeName(Opcode)));

    gcmVERIFY_OK(clsIOPERAND_Dump(Compiler, IOperand));
    gcmVERIFY_OK(clsROPERAND_Dump(Compiler, ROperand0));
    gcmVERIFY_OK(clsROPERAND_Dump(Compiler, ROperand1));

    gcmASSERT(Opcode == clvOPCODE_RSHIFT || Opcode == clvOPCODE_LSHIFT);

    if (gcIsScalarDataType(ROperand0->dataType) || gcIsVectorDataType(ROperand0->dataType))
    {
        gcmVERIFY_OK(_ConvIOperandToTarget(Compiler,
                        IOperand,
                        &target));

        status = _ConvNormalROperandToSource(Compiler,
                        LineNo,
                        StringNo,
                        ROperand0,
                        &source0);

        if (gcmIS_ERROR(status)) return status;

        status = _ConvNormalROperandToSource(Compiler,
                        LineNo,
                        StringNo,
                        ROperand1,
                        &source1);

        if (gcmIS_ERROR(status)) return status;

        status = clEmitCode2(Compiler,
                     LineNo,
                     StringNo,
                     Opcode,
                     &target,
                     &source0,
                     &source1);
        if (gcmIS_ERROR(status)) return status;
    }
    else
    {
        gcmASSERT(0);
    }

    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                clvDUMP_CODE_GENERATOR,
                "</OPERATION>"));

    return gcvSTATUS_OK;
}

gceSTATUS
clGenBitwiseExprCode(
    IN cloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN cleOPCODE Opcode,
    IN clsIOPERAND * IOperand,
    IN clsROPERAND * ROperand0,
    IN clsROPERAND * ROperand1
    )
{
    gceSTATUS    status;
    gcsTARGET    target;
    gcsSOURCE    source0;
    gcsSOURCE    source1;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    gcmASSERT(IOperand);
    gcmASSERT(ROperand0);
    gcmASSERT(ROperand1);

    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                    clvDUMP_CODE_GENERATOR,
                    "<OPERATION line=\"%d\" string=\"%d\" type=\"%s\">",
                    LineNo,
                    StringNo,
                    clGetOpcodeName(Opcode)));

    gcmVERIFY_OK(clsIOPERAND_Dump(Compiler, IOperand));
    gcmVERIFY_OK(clsROPERAND_Dump(Compiler, ROperand0));
    gcmVERIFY_OK(clsROPERAND_Dump(Compiler, ROperand1));

    gcmASSERT(Opcode == clvOPCODE_BITWISE_AND || Opcode == clvOPCODE_BITWISE_OR || Opcode == clvOPCODE_BITWISE_XOR);

    if (gcIsDataTypeEqual(ROperand0->dataType, ROperand1->dataType) ||
        (gcIsScalarDataType(ROperand0->dataType) && gcIsScalarDataType(ROperand1->dataType))) {
        if (gcIsScalarDataType(ROperand0->dataType) || gcIsVectorDataType(ROperand0->dataType)) {
            gcmVERIFY_OK(_ConvIOperandToTarget(Compiler,
                            IOperand,
                            &target));

            status = _ConvNormalROperandToSource(Compiler,
                            LineNo,
                            StringNo,
                            ROperand0,
                            &source0);

            if (gcmIS_ERROR(status)) return status;

            status = _ConvNormalROperandToSource(Compiler,
                            LineNo,
                            StringNo,
                            ROperand1,
                            &source1);

            if (gcmIS_ERROR(status)) return status;

            status = clEmitCode2(Compiler,
                         LineNo,
                         StringNo,
                         Opcode,
                         &target,
                         &source0,
                         &source1);

            if (gcmIS_ERROR(status)) return status;
        }
        else {
            gctUINT i;
            gcmASSERT(gcIsMatrixDataType(ROperand0->dataType));

            for (i = 0; i < gcGetMatrixDataTypeColumnCount(ROperand0->dataType); i++)
            {
                gcmVERIFY_OK(_ConvIOperandToMatrixColumnTarget(Compiler, IOperand, i, &target));

                status = _ConvNormalROperandToMatrixColumnSource(Compiler,
                                        LineNo,
                                        StringNo,
                                        ROperand0,
                                        i,
                                        &source0);

                if (gcmIS_ERROR(status)) return status;

                status = _ConvNormalROperandToMatrixColumnSource(Compiler,
                                        LineNo,
                                        StringNo,
                                        ROperand1,
                                        i,
                                        &source1);

                if (gcmIS_ERROR(status)) return status;

                status = clEmitCode2(Compiler,
                        LineNo,
                        StringNo,
                        Opcode,
                        &target,
                        &source0,
                        &source1);

                if (gcmIS_ERROR(status)) return status;
            }
        }
    }
    else {
      if (gcIsMatrixDataType(ROperand0->dataType)) {
        gctUINT i;

        gcmASSERT(gcIsScalarDataType(ROperand1->dataType));

        status = _ConvNormalROperandToSource(Compiler,
                        LineNo,
                        StringNo,
                        ROperand1,
                        &source1);

        if (gcmIS_ERROR(status)) return status;

        for (i = 0; i < gcGetMatrixDataTypeColumnCount(ROperand0->dataType); i++)
        {
            gcmVERIFY_OK(_ConvIOperandToMatrixColumnTarget(Compiler, IOperand, i, &target));

            status = _ConvNormalROperandToMatrixColumnSource(Compiler,
                                    LineNo,
                                    StringNo,
                                    ROperand0,
                                    i,
                                    &source0);

            if (gcmIS_ERROR(status)) return status;

            status = clEmitCode2(Compiler,
                    LineNo,
                    StringNo,
                    Opcode,
                    &target,
                    &source0,
                    &source1);

            if (gcmIS_ERROR(status)) return status;
        }
      }
      else if (gcIsMatrixDataType(ROperand1->dataType)) {
        gctUINT i;
        gcmASSERT(gcIsScalarDataType(ROperand0->dataType));

        status = _ConvNormalROperandToSource(Compiler,
                        LineNo,
                        StringNo,
                        ROperand0,
                        &source0);

        if (gcmIS_ERROR(status)) return status;

        for (i = 0; i < gcGetMatrixDataTypeColumnCount(ROperand1->dataType); i++) {
            gcmVERIFY_OK(_ConvIOperandToMatrixColumnTarget(Compiler, IOperand, i, &target));

            status = _ConvNormalROperandToMatrixColumnSource(Compiler,
                                    LineNo,
                                    StringNo,
                                    ROperand1,
                                    i,
                                    &source1);

            if (gcmIS_ERROR(status)) return status;

            status = clEmitCode2(Compiler,
                         LineNo,
                         StringNo,
                         Opcode,
                         &target,
                         &source0,
                         &source1);
            if (gcmIS_ERROR(status)) return status;
        }
      }
      else {
          gcmASSERT((gcIsVectorDataType(ROperand0->dataType)
                          && gcIsScalarDataType(ROperand1->dataType))
                              || (gcIsScalarDataType(ROperand0->dataType)
                          && gcIsVectorDataType(ROperand1->dataType)));

        gcmVERIFY_OK(_ConvIOperandToTarget(Compiler,
                           IOperand,
                           &target));

        status = _ConvNormalROperandToSource(Compiler,
                             LineNo,
                             StringNo,
                             ROperand0,
                                  &source0);

        if (gcmIS_ERROR(status)) return status;

        status = _ConvNormalROperandToSource(Compiler,
                             LineNo,
                             StringNo,
                             ROperand1,
                             &source1);
        if (gcmIS_ERROR(status)) return status;

        status = clEmitCode2(Compiler,
                     LineNo,
                     StringNo,
                     Opcode,
                     &target,
                     &source0,
                     &source1);
        if (gcmIS_ERROR(status)) return status;
      }
    }

    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                clvDUMP_CODE_GENERATOR,
                "</OPERATION>"));

    return gcvSTATUS_OK;
}
#endif

#define clmIsOdd(N)      ((N) & 0x1)

static gctINT
_ConvValueToPowerOfTwo(
IN gctUINT Value
)
{
   gctUINT powerOfTwo = 0;
   gctUINT remainder = Value;

   while(remainder) {
     if(remainder == 1) break;
     if(clmIsOdd(remainder)) return -1;
     remainder >>= 1;
     powerOfTwo++;
   }

   return powerOfTwo;
}

static gceSTATUS
_GenScaledIndexOperand(
    IN cloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN clsROPERAND *IndexOperand,
    IN clsROPERAND *SizeOperand,
    OUT clsROPERAND *ScaledOperand
    )
{
   clsIOPERAND result[1];
   gceSTATUS status = gcvSTATUS_OK;

   gcmASSERT(gcIsScalarDataType(IndexOperand->dataType) &&
       clmIsElementTypeInteger(clmGEN_CODE_elementType_GET(IndexOperand->dataType)));

   if (!SizeOperand->isReg) { /*size is constant */
      if(SizeOperand->u.constant.values[0].intValue == 1) { /* is a constant 1 */
         *ScaledOperand = *IndexOperand;
         return gcvSTATUS_OK;
      }
      else {
         gctINT shiftValue;
         clsROPERAND size[1];

         shiftValue = _ConvValueToPowerOfTwo(SizeOperand->u.constant.values[0].intValue);
         if(shiftValue > 0) {
            clsROPERAND_InitializeScalarConstant(size,
                                                 clmGenCodeDataType(T_INT),
                                                 long,
                                                 shiftValue);
            clsIOPERAND_New(Compiler, result, IndexOperand->dataType);
            status = clGenShiftExprCode(Compiler,
                                        LineNo,
                                        StringNo,
                                        clvOPCODE_LSHIFT,
                                        result,
                                        IndexOperand,
                                        size);
            clsROPERAND_InitializeUsingIOperand(ScaledOperand, result);
            return gcvSTATUS_OK;
         }
      }
   }

   if (!IndexOperand->isReg && /* is a constant 0 */
       IndexOperand->u.constant.values[0].intValue == 0) {
      *ScaledOperand = *IndexOperand;
      return gcvSTATUS_OK;
   }

   clsIOPERAND_New(Compiler, result, IndexOperand->dataType);
   status = clGenArithmeticExprCode(Compiler,
                                    LineNo,
                                    StringNo,
                                    clvOPCODE_MUL,
                                    result,
                                    IndexOperand,
                                    SizeOperand);
   clsROPERAND_InitializeUsingIOperand(ScaledOperand, result);

   return status;
}

gceSTATUS
clGenScaledIndexOperand(
    IN cloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN clsROPERAND *IndexOperand,
    IN gctINT32 ElementDataTypeSize,
    OUT clsROPERAND *ScaledOperand
    )
{
   gceSTATUS status = gcvSTATUS_OK;

   gcmASSERT(gcIsScalarDataType(IndexOperand->dataType) &&
       clmIsElementTypeInteger(clmGEN_CODE_elementType_GET(IndexOperand->dataType)));

   if(ElementDataTypeSize == 1) {
     *ScaledOperand = *IndexOperand;
     return gcvSTATUS_OK;
   }

   if (!IndexOperand->isReg) { /* is a constant */
     gctUINT index;

     index = IndexOperand->u.constant.values[0].intValue;
     if(index == 0) {
       *ScaledOperand = *IndexOperand;
     }
     else {
       clsROPERAND_InitializeScalarConstant(ScaledOperand,
                                            clmGenCodeDataType(T_INT),
                                            long,
                                            index * ElementDataTypeSize);
     }
     return gcvSTATUS_OK;
   }
   else {
     clsROPERAND scale[1];
     clsIOPERAND result[1];
     gctINT shiftValue;

     clsIOPERAND_New(Compiler, result, IndexOperand->dataType);
     shiftValue = _ConvValueToPowerOfTwo(ElementDataTypeSize);
     if(shiftValue > 0) {
        clsROPERAND_InitializeScalarConstant(scale,
                                             clmGenCodeDataType(T_INT),
                                             long,
                                             shiftValue);
        status = clGenShiftExprCode(Compiler,
                                    LineNo,
                                    StringNo,
                                    clvOPCODE_LSHIFT,
                                    result,
                                    IndexOperand,
                                    scale);
     }
     else {
        clsROPERAND_InitializeScalarConstant(scale,
                                             clmGenCodeDataType(T_INT),
                                             long,
                                             ElementDataTypeSize);
        status = clGenArithmeticExprCode(Compiler,
                                         LineNo,
                                         StringNo,
                                         clvOPCODE_MUL,
                                         result,
                                         IndexOperand,
                                         scale);
     }
     clsROPERAND_InitializeUsingIOperand(ScaledOperand, result);
   }
   return status;
}

gceSTATUS
clGenAddToOffset(
IN clsROPERAND *Offset,
IN gctINT Incr
)
{
   if(Incr == 0) return gcvSTATUS_OK;
   if(!Offset->isReg) {
      gctINT offset;

      switch(clmGEN_CODE_elementType_GET(Offset->dataType)) {
      case clvTYPE_UINT:
      case clvTYPE_UCHAR:
      case clvTYPE_USHORT:
           offset = (gctINT) Offset->u.constant.values[0].uintValue + Incr;
           Offset->u.constant.values[0].uintValue = (gctUINT)offset;
       return gcvSTATUS_OK;

      case clvTYPE_INT:
      case clvTYPE_CHAR:
      case clvTYPE_SHORT:
           Offset->u.constant.values[0].intValue += Incr;
       return gcvSTATUS_OK;

      default:
       break;
      }
   }
   return gcvSTATUS_INVALID_ARGUMENT;
}

static gceSTATUS
_UpdateAddressOffset(
IN cloCOMPILER Compiler,
IN gctUINT LineNo,
IN gctUINT StringNo,
IN gctINT Incr,
IN OUT clsROPERAND *Offset,
OUT clsROPERAND *NewOffset
)
{
    gceSTATUS status = gcvSTATUS_OK;
    clsROPERAND step[1];
    clsROPERAND *offset;

    offset = Offset;
    if(offset->isReg) {
       clsIOPERAND iOperand[1];

       if(NewOffset) {
          clsIOPERAND_New(Compiler, iOperand, clmGenCodeDataType(T_INT));
          clsROPERAND_InitializeUsingIOperand(NewOffset, iOperand);
       }
       else {
          clsIOPERAND_Initialize(iOperand, offset->dataType, offset->u.reg.regIndex);
       }
       clsROPERAND_InitializeScalarConstant(step,
                                            clmGenCodeDataType(T_INT),
                                            long,
                                            Incr);
       status = clGenGenericCode2(Compiler,
                                  LineNo,
                                  StringNo,
                                  clvOPCODE_ADD,
                                  iOperand,
                                  offset,
                                  step);
       if (gcmIS_ERROR(status)) return status;
    }
    else {
       if(NewOffset) {
           clsROPERAND_InitializeConstant(NewOffset,
                                          clmGenCodeDataType(T_INT),
                                          1,
                                          Offset->u.constant.values);
           offset = NewOffset;
       }
       status = clGenAddToOffset(offset, Incr);
       if (gcmIS_ERROR(status)) return status;
    }
    return status;
}

gceSTATUS
clGenScaledIndexOperandWithOffset(
    IN cloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN clsROPERAND *IndexOperand,
    IN gctINT32 ElementDataTypeSize,
    IN gctINT ByteOffset,
    OUT clsROPERAND *ScaledOperand
    )
{
   gceSTATUS status = gcvSTATUS_OK;

   gcmASSERT(IndexOperand);
   gcmASSERT(gcIsScalarDataType(IndexOperand->dataType) &&
       clmIsElementTypeInteger(clmGEN_CODE_elementType_GET(IndexOperand->dataType)));

   if(ElementDataTypeSize == 1 && ByteOffset == 0) {
      *ScaledOperand = *IndexOperand;
      return gcvSTATUS_OK;
   }

   if (!IndexOperand->isReg) { /* is a constant */
     gctUINT index;

     index = IndexOperand->u.constant.values[0].intValue;
     if(index == 0 && ByteOffset == 0) {
        *ScaledOperand = *IndexOperand;
     }
     else {
        clsROPERAND_InitializeScalarConstant(ScaledOperand,
                                             clmGenCodeDataType(T_INT),
                                             long,
                                             index * ElementDataTypeSize + ByteOffset);
     }
     return gcvSTATUS_OK;
   }
   else {
     gctINT shiftValue;

     shiftValue = _ConvValueToPowerOfTwo(ElementDataTypeSize);
     if(shiftValue == 0) {
        *ScaledOperand = *IndexOperand;
     }
     else {
        clsROPERAND scale[1];
        clsIOPERAND result[1];

        clsIOPERAND_New(Compiler, result, IndexOperand->dataType);
        if(shiftValue > 0) {
           clsROPERAND_InitializeScalarConstant(scale,
                                                clmGenCodeDataType(T_INT),
                                                long,
                                                shiftValue);
           status = clGenShiftExprCode(Compiler,
                                       LineNo,
                                       StringNo,
                                       clvOPCODE_LSHIFT,
                                       result,
                                       IndexOperand,
                                       scale);
        }
        else {
           clsROPERAND_InitializeScalarConstant(scale,
                                                clmGenCodeDataType(T_INT),
                                                long,
                                                ElementDataTypeSize);
           status = clGenArithmeticExprCode(Compiler,
                                            LineNo,
                                            StringNo,
                                            clvOPCODE_MUL,
                                            result,
                                            IndexOperand,
                                            scale);
        }
        if (gcmIS_ERROR(status)) return status;
        clsROPERAND_InitializeUsingIOperand(ScaledOperand, result);
     }

     if(ByteOffset) {
        status = _UpdateAddressOffset(Compiler,
                                      LineNo,
                                      StringNo,
                                      ByteOffset,
                                      ScaledOperand,
                                      gcvNULL);
        if (gcmIS_ERROR(status)) return status;
     }
   }
   return status;
}

static void
_InitializeROperandConstant(
IN clsROPERAND *Constant,
IN clsGEN_CODE_DATA_TYPE DataType,
IN gctINT Value
)
{
   cltELEMENT_TYPE elementType;

   elementType = clmGEN_CODE_elementType_GET(DataType);
   if(clmIsElementTypeFloating(elementType)) {
      clsROPERAND_InitializeFloatOrVecOrMatConstant(Constant, DataType, (float) Value);
   }
   else if(clmIsElementTypeChar(elementType)) {
      clsROPERAND_InitializeCharConstant(Constant, DataType, (char)Value);
   }
   else if(clmIsElementTypeBoolean(elementType)) {
      clsROPERAND_InitializeBoolOrBVecConstant(Constant, DataType, clmI2B(Value));
   }
   else if(clmIsElementTypeUnsigned(elementType)) {
      clsROPERAND_InitializeUintConstant(Constant, DataType, (gctUINT)Value);
   }
   else {
      clsROPERAND_InitializeIntOrIVecConstant(Constant, DataType, (gctINT)Value);
   }
}

gctBOOL
_IsIntegerZero(
IN clsROPERAND *Operand
)
{
   gcmASSERT(Operand);

   if(!Operand->isReg) {
      switch(clmGEN_CODE_elementType_GET(Operand->dataType)) {
      case clvTYPE_UINT:
      case clvTYPE_UCHAR:
      case clvTYPE_USHORT:
           return Operand->u.constant.values[0].uintValue == 0;

      case clvTYPE_INT:
      case clvTYPE_CHAR:
      case clvTYPE_SHORT:
           return Operand->u.constant.values[0].intValue == 0;

      case clvTYPE_BOOL:
           return Operand->u.constant.values[0].boolValue == gcvFALSE;
      default:
           break;
      }
   }
   return gcvFALSE;
}

gctINT
_GetIntegerValue(
IN clsROPERAND *Operand
)
{
   gcmASSERT(Operand && !Operand->isReg);

   if(!Operand) return 0;

   switch(clmGEN_CODE_elementType_GET(Operand->dataType)) {
   case clvTYPE_UINT:
   case clvTYPE_UCHAR:
   case clvTYPE_USHORT:
        return Operand->u.constant.values[0].uintValue;

   case clvTYPE_INT:
   case clvTYPE_CHAR:
   case clvTYPE_SHORT:
        return Operand->u.constant.values[0].intValue;

   case clvTYPE_BOOL:
        return (int)Operand->u.constant.values[0].boolValue;

   case clvTYPE_FLOAT:
        return (int)Operand->u.constant.values[0].floatValue;

   default:
        gcmASSERT(0);
   }
   return 0;
}

static gceSTATUS
_AddROperandOffset(
IN cloCOMPILER Compiler,
IN gctUINT LineNo,
IN gctUINT StringNo,
IN clsROPERAND *ROperand,
IN clsROPERAND *Offset,
IN OUT clsROPERAND *Res
)
{
    if(_IsIntegerZero(Offset)) {
       *Res = *ROperand;
    }
    else if(_IsIntegerZero(ROperand)) {
       *Res = *Offset;
    }
    else if(!ROperand->isReg && !Offset->isReg) {
       clsROPERAND_InitializeScalarConstant(Res,
                                            clmGenCodeDataType(T_INT),
                                            int,
                                           _GetIntegerValue(ROperand) + _GetIntegerValue(Offset));
    }
    else {
       gceSTATUS status;
       clsIOPERAND intermIOperand[1];

       clsIOPERAND_New(Compiler, intermIOperand, ROperand->dataType);
       status = clGenGenericCode2(Compiler,
                                  LineNo,
                                  StringNo,
                                  clvOPCODE_ADD,
                                  intermIOperand,
                                  ROperand,
                                  Offset);
       if (gcmIS_ERROR(status)) return status;
       clsROPERAND_InitializeUsingIOperand(Res, intermIOperand);
    }
    return gcvSTATUS_OK;
}

static gceSTATUS
_AddLOperandOffset(
IN cloCOMPILER Compiler,
IN gctUINT LineNo,
IN gctUINT StringNo,
IN clsLOPERAND *LOperand,
IN clsROPERAND *Offset,
IN OUT clsLOPERAND *Res
)
{
    if(_IsIntegerZero(Offset)) {
       *Res = *LOperand;
    }
    else {
       gceSTATUS status;
       clsIOPERAND intermIOperand[1];
       clsROPERAND intermROperand[1];

       clsIOPERAND_New(Compiler, intermIOperand, LOperand->dataType);
       clsROPERAND_InitializeUsingLOperand(intermROperand, LOperand);

       clsLOPERAND_InitializeUsingIOperand(Res, intermIOperand);
       status = clGenGenericCode2(Compiler,
                                  LineNo,
                                  StringNo,
                                  clvOPCODE_ADD,
                                  intermIOperand,
                                  intermROperand,
                                  Offset);
       if (gcmIS_ERROR(status)) return status;
    }

    return gcvSTATUS_OK;
}

/* Optimize offset usage when it is a constant */
#ifdef _cldIncludeComponentOffset
static gceSTATUS
_GenSelectiveLoadCode(
IN cloCOMPILER Compiler,
IN gctUINT LineNo,
IN gctUINT StringNo,
IN clsIOPERAND *IOperand,
IN clsROPERAND *ROperand,
IN clsROPERAND *Offset,
IN clsGEN_CODE_DATA_TYPE StorageType,
IN clsCOMPONENT_SELECTION *ComponentSelection
)
{
  gceSTATUS status;
  gcsSUPER_TARGET superTarget;
  gcsSOURCE source0[1];
  gcsSOURCE source1[1];
  gctSIZE_T iSize, i, j;
  clsROPERAND *addressOperand;
  clsROPERAND addressBuffer[1];
  clsIOPERAND columnIOperand[1];
  gctSIZE_T numSections;
  gctUINT numColumns;
  clsROPERAND *offset;
  clsROPERAND offsetBuffer[1];
  gctINT incr;
  gctUINT8 superEnable[cldMaxFourComponentCount];
  clsCOMPONENT_SELECTION componentSelection[1];

/* Verify the arguments. */
  clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
  gcmASSERT(ROperand);
  gcmASSERT(IOperand);
  gcmASSERT(Offset);

  gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                                clvDUMP_CODE_GENERATOR,
                                "<OPERATION line=\"%d\" string=\"%d\" type=\"store\">",
                                LineNo,
                                StringNo));

  gcmVERIFY_OK(clsROPERAND_Dump(Compiler, ROperand));
  gcmVERIFY_OK(clsIOPERAND_Dump(Compiler, IOperand));
  gcmVERIFY_OK(clsROPERAND_Dump(Compiler, Offset));

  if(gcIsMatrixDataType(IOperand->dataType)) {
     clsIOPERAND_InitializeAsMatrixColumn(columnIOperand, IOperand, 0);
     numColumns = gcGetMatrixDataTypeColumnCount(IOperand->dataType);
  }
  else {
     *columnIOperand = *IOperand;
     numColumns = 1;
  }

  iSize = gcGetDataTypeRegSize(StorageType);
  numSections = _ConvComponentSelectionToSuperEnable(_CheckHighPrecisionComponentSelection(StorageType,
                                                                                           ComponentSelection,
                                                                                           componentSelection),
                                                                                           superEnable);

  if(Offset->isReg) {
     if(numColumns > 1 || ((iSize > 1) && !(numSections == 1 && superEnable[0]))) {
        status = _AddROperandOffset(Compiler,
                                    LineNo,
                                    StringNo,
                                    ROperand,
                                    Offset,
                                    addressBuffer);
        if (gcmIS_ERROR(status)) return status;

        addressOperand = addressBuffer;
        _InitializeROperandConstant(offsetBuffer, clmGenCodeDataType(T_UINT), 0);
        offset = offsetBuffer;
     }
     else {
        addressOperand = ROperand;
        offset = Offset;
     }
  }
  else {
     addressOperand = ROperand;
     _InitializeROperandConstant(offsetBuffer,
                                 Offset->dataType,
                                 _GetIntegerValue(Offset));
     offset = offsetBuffer;
  }

  status = _ConvNormalROperandToSource(Compiler,
                       LineNo,
                       StringNo,
                       addressOperand,
                       source0);
  if (gcmIS_ERROR(status)) return status;

  incr = 0;
  do {
     for(i = 0; i < iSize; i++) {
       if(superEnable[i]) {
           if(gcIsScalarDataType(IOperand->dataType)) {
              incr += _clmConvEnableToComponentStart(superEnable[i]) *
                      _TargetElementTypeByteSize(IOperand->dataType.elementType);
           }

           gcmVERIFY_OK(_ConvIOperandToSectionalTarget(Compiler,
                                  columnIOperand,
                               (gctUINT8)i,
                                  &superTarget));

           for(j=0; j < superTarget.numTargets; j++) {

              if(offset->isReg && incr) {
                 status = _AddROperandOffset(Compiler,
                                             LineNo,
                                             StringNo,
                                             addressOperand,
                                             offset,
                                             addressBuffer);
                 if (gcmIS_ERROR(status)) return status;

                 addressOperand = addressBuffer;
                 status = _ConvNormalROperandToSource(Compiler,
                                      LineNo,
                                      StringNo,
                                      addressOperand,
                                         source0);
                 if (gcmIS_ERROR(status)) return status;
                 _InitializeROperandConstant(offsetBuffer, clmGenCodeDataType(T_UINT), 0);
                 offset = offsetBuffer;
              }
              status = clGenAddToOffset(offset,
                                        incr);
              if (gcmIS_ERROR(status)) return status;

              status = _ConvNormalROperandToSource(Compiler,
                                                   LineNo,
                                                   StringNo,
                                                   offset,
                                                   source1);
              if (gcmIS_ERROR(status)) return status;

              status = clEmitCode2(Compiler,
                                   LineNo,
                                   StringNo,
                                   clvOPCODE_LOAD,
                                   superTarget.targets + j,
                                   source0,
                                   source1);
              if (gcmIS_ERROR(status)) return status;
              incr = (gctINT)_DataTypeByteSize(superTarget.targets[j].dataType);
           }
        }
        else {
           incr += (gctINT)_SectionalDataTypeByteSize(StorageType);
        }
     }
     columnIOperand->tempRegIndex += (gctREG_INDEX)iSize;
  } while(--numColumns);

  gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                                clvDUMP_CODE_GENERATOR,
                                "</OPERATION>"));

  return gcvSTATUS_OK;
}
#else
static gceSTATUS
_GenSelectiveLoadCode(
IN cloCOMPILER Compiler,
IN gctUINT LineNo,
IN gctUINT StringNo,
IN clsIOPERAND *IOperand,
IN clsROPERAND *ROperand,
IN clsROPERAND *Offset,
IN clsGEN_CODE_DATA_TYPE StorageType,
IN clsCOMPONENT_SELECTION *ComponentSelection
)
{
  gceSTATUS status;
  gcsSUPER_TARGET superTarget;
  gcsSOURCE source0[1];
  gcsSOURCE source1[1];
  gctSIZE_T iSize, i, j;
  clsROPERAND *addressOperand;
  clsROPERAND addressBuffer[1];
  clsIOPERAND columnIOperand[1];
  gctSIZE_T numSections;
  gctUINT numColumns;
  clsROPERAND *offset;
  clsROPERAND offsetBuffer[1];
  gctINT incr;
  gctUINT8 superEnable[cldMaxFourComponentCount];

/* Verify the arguments. */
  clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
  gcmASSERT(ROperand);
  gcmASSERT(IOperand);
  gcmASSERT(Offset);

  gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                                clvDUMP_CODE_GENERATOR,
                                "<OPERATION line=\"%d\" string=\"%d\" type=\"store\">",
                                LineNo,
                                StringNo));

  gcmVERIFY_OK(clsROPERAND_Dump(Compiler, ROperand));
  gcmVERIFY_OK(clsIOPERAND_Dump(Compiler, IOperand));
  gcmVERIFY_OK(clsROPERAND_Dump(Compiler, Offset));

  if(gcIsMatrixDataType(IOperand->dataType)) {
     clsIOPERAND_InitializeAsMatrixColumn(columnIOperand, IOperand, 0);
     numColumns = gcGetMatrixDataTypeColumnCount(IOperand->dataType);
  }
  else {
     *columnIOperand = *IOperand;
     numColumns = 1;
  }

  iSize = gcGetDataTypeRegSize(columnIOperand->dataType);
  numSections = _ConvComponentSelectionToSuperEnable(ComponentSelection, superEnable);

  if(Offset->isReg) {
     if(numColumns > 1 || ((iSize > 1) && !(numSections == 1 && superEnable[0]))) {
        status = _AddROperandOffset(Compiler,
                                    LineNo,
                                    StringNo,
                                    ROperand,
                                    Offset,
                                    addressBuffer);
        if (gcmIS_ERROR(status)) return status;

        addressOperand = addressBuffer;
        _InitializeROperandConstant(offsetBuffer, clmGenCodeDataType(T_UINT), 0);
        offset = offsetBuffer;
     }
     else {
        addressOperand = ROperand;
        offset = Offset;
     }
  }
  else {
     addressOperand = ROperand;
     _InitializeROperandConstant(offsetBuffer,
                                 Offset->dataType,
                                 _GetIntegerValue(Offset));
     offset = offsetBuffer;
  }

  status = _ConvNormalROperandToSource(Compiler,
                       LineNo,
                       StringNo,
                       addressOperand,
                       source0);
  if (gcmIS_ERROR(status)) return status;

  incr = 0;
  do {
     for(i = 0; i < iSize; i++) {
    if(superEnable[i]) {
           gcmVERIFY_OK(_ConvIOperandToSectionalTarget(Compiler,
                                  columnIOperand,
                               (gctUINT8)i,
                                  &superTarget));

           for(j=0; j < superTarget.numTargets; j++) {

              if(offset->isReg && incr) {
                 status = _AddROperandOffset(Compiler,
                                             LineNo,
                                             StringNo,
                                             addressOperand,
                                             offset,
                                             addressBuffer);
                 if (gcmIS_ERROR(status)) return status;

                 addressOperand = addressBuffer;
                 status = _ConvNormalROperandToSource(Compiler,
                                      LineNo,
                                      StringNo,
                                      addressOperand,
                                         source0);
                 if (gcmIS_ERROR(status)) return status;
                 _InitializeROperandConstant(offsetBuffer, clmGenCodeDataType(T_UINT), 0);
                 offset = offsetBuffer;
              }
              status = clGenAddToOffset(offset,
                                        incr);
              if (gcmIS_ERROR(status)) return status;

              status = _ConvNormalROperandToSource(Compiler,
                                                   LineNo,
                                                   StringNo,
                                                   offset,
                                                   source1);
              if (gcmIS_ERROR(status)) return status;

              status = clEmitCode2(Compiler,
                                   LineNo,
                                   StringNo,
                                   clvOPCODE_LOAD,
                                   superTarget.targets + j,
                                   source0,
                                   source1);
              if (gcmIS_ERROR(status)) return status;
              incr = (gctINT)_DataTypeByteSize(superTarget.targets[j].dataType);
           }
        }
        else {
           incr += (gctINT)_SectionalDataTypeByteSize(columnIOperand->dataType);
        }
     }
     columnIOperand->tempRegIndex += (gctREG_INDEX)iSize;
  } while(--numColumns);

  gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                                clvDUMP_CODE_GENERATOR,
                                "</OPERATION>"));

  return gcvSTATUS_OK;
}
#endif

void
clGenClearCurrentVectorCreation(
IN cloCOMPILER Compiler
)
{
   cloCODE_GENERATOR codeGenerator;

   codeGenerator = cloCOMPILER_GetCodeGenerator(Compiler);
   gcoOS_ZeroMemory(&codeGenerator->currentVector, gcmSIZEOF(clsVECTOR_CREATION));
}


gctBOOL
clGenNeedVectorUpdate(
IN cloCOMPILER Compiler,
IN cloCODE_GENERATOR CodeGenerator,
IN cloIR_EXPR Expr,
IN clsGEN_CODE_DATA_TYPE DataType,
IN clsROPERAND *ScaledIndex,
IN clsIOPERAND *IOperand
)
{
    clsNAME *variable;
    gctUINT8 i;
    gctUINT8 selectedComponent;
    gctBOOL matched = gcvTRUE;
    gctSIZE_T size;
    gctINT offset;

    if(IOperand->tempRegIndex != CodeGenerator->currentVector.tempRegIndex) return gcvTRUE;
    if(ScaledIndex->isReg) {
       gcoOS_ZeroMemory(&CodeGenerator->currentVector, gcmSIZEOF(clsVECTOR_CREATION));
       return gcvTRUE;
    }
    else {
       offset = _GetIntegerValue(ScaledIndex);
    }

    variable = clParseFindPointerVariable(Compiler,
                                          Expr);

    if(!variable ||
       variable->u.variableInfo.isDirty) {
       gcoOS_ZeroMemory(&CodeGenerator->currentVector, gcmSIZEOF(clsVECTOR_CREATION));
       return gcvTRUE;
    }

    i = IOperand->componentSelection.components;
    gcmASSERT(i == gcGetDataTypeComponentCount(DataType));
    size = _TargetElementTypeByteSize(DataType.elementType);
    for(i = 0; i < IOperand->componentSelection.components; i++, offset += size) {
       selectedComponent = IOperand->componentSelection.selection[i];
       if(!CodeGenerator->currentVector.component[selectedComponent].pointer ||
          CodeGenerator->currentVector.component[selectedComponent].pointer != variable ||
          CodeGenerator->currentVector.component[selectedComponent].offset != offset) {
          matched = gcvFALSE;
       }
       CodeGenerator->currentVector.component[selectedComponent].pointer = variable;
       CodeGenerator->currentVector.component[selectedComponent].offset = offset;
    }
    return !matched;
}

gceSTATUS
clGenDerefPointerCode(
    IN cloCOMPILER Compiler,
    IN cloIR_EXPR Expr,
    IN clsGEN_CODE_PARAMETERS *LeftParameters,
    IN clsGEN_CODE_PARAMETERS *RightParameters,
    IN OUT clsGEN_CODE_PARAMETERS *Parameters
    )
{
  gceSTATUS status;
  gctUINT i;

/* Verify the arguments. */
  clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
  gcmASSERT(LeftParameters);
  gcmASSERT(RightParameters);
  gcmASSERT(Parameters);

/** Delay code generation when the left hand side is needed.
    To do this, we have to copy the subscript and the left parameter's
    roperands if any onto the return Parameters. This is somewhat a kluge **/
  if (Parameters->needLOperand &&
      (Parameters->hint & clvGEN_LEFT_ASSIGN_CODE)) {
     if(!Parameters->elementIndex) {
        gctPOINTER pointer;

        status = cloCOMPILER_Allocate(Compiler,
                                      (gctSIZE_T)sizeof(clsROPERAND),
                                      (gctPOINTER *) &pointer);
        if (gcmIS_ERROR(status)) return status;
        Parameters->elementIndex = pointer;
     }
     Parameters->elementIndex[0] = RightParameters->rOperands[0];
     Parameters->dataTypes[0].byteOffset = LeftParameters->dataTypes[0].byteOffset;
     for (i = 0; i < Parameters->operandCount; i++) {
         Parameters->lOperands[i] = LeftParameters->lOperands[0];
     }
     if (Parameters->needROperand) {
       for (i = 0; i < Parameters->operandCount; i++) {
         Parameters->rOperands[i] = LeftParameters->rOperands[0];
       }
     }
     Parameters->hint = clvGEN_DEREF_CODE;
     return gcvSTATUS_OK;
  }

  if (Parameters->needROperand || Parameters->needLOperand) {
    clsIOPERAND iOperand[1];
    clsROPERAND scaledIndex[1];
    gctUINT elementDataTypeSize = 0;

    elementDataTypeSize = clsDECL_GetByteSize(&Expr->decl);
    status = clGenScaledIndexOperandWithOffset(Compiler,
                                               Expr->base.lineNo,
                                               Expr->base.stringNo,
                                               &RightParameters->rOperands[0],
                                               elementDataTypeSize,
                                               LeftParameters->dataTypes[0].byteOffset,
                                               scaledIndex);
    if (gcmIS_ERROR(status)) return status;

    if(Parameters->hint & clvGEN_FIELD_SELECT_CODE) {
       if (_clmIsROperandConstantZero(scaledIndex)) {
          if(Parameters->needLOperand) {
             Parameters->lOperands[0] = LeftParameters->lOperands[0];
          }
          if(Parameters->needROperand) {
             Parameters->rOperands[0] = LeftParameters->rOperands[0];
          }
       }
       else {
          if(Parameters->needLOperand) {
            clsROPERAND rOperand[1];

            clsIOPERAND_New(Compiler, iOperand, LeftParameters->lOperands[0].dataType);
            clsROPERAND_InitializeUsingLOperand(rOperand, &LeftParameters->lOperands[0]);
            status = clGenGenericCode2(Compiler,
                                       Expr->base.lineNo,
                                       Expr->base.stringNo,
                                       clvOPCODE_ADD,
                                       iOperand,
                                       rOperand,
                                       scaledIndex);
             if (gcmIS_ERROR(status)) return status;
             clsLOPERAND_InitializeUsingIOperand(&Parameters->lOperands[0], iOperand);
          }
          if(Parameters->needROperand) {
            clsIOPERAND_New(Compiler, iOperand, LeftParameters->rOperands[0].dataType);
            status = clGenGenericCode2(Compiler,
                                       Expr->base.lineNo,
                                       Expr->base.stringNo,
                                       clvOPCODE_ADD,
                                       iOperand,
                                       &LeftParameters->rOperands[0],
                                       scaledIndex);
             if (gcmIS_ERROR(status)) return status;
             clsROPERAND_InitializeUsingIOperand(&Parameters->rOperands[0], iOperand);
          }
       }

       Parameters->hint = clvGEN_DEREF_STRUCT_CODE;
    }
    else {
      if(Parameters->needLOperand) {
        if(clmDECL_IsUnderlyingStructOrUnion(&Expr->decl)) {
          if(scaledIndex->isReg) {
            clsROPERAND rOperand[1];

            clsIOPERAND_New(Compiler, iOperand, LeftParameters->lOperands[0].dataType);
            clsROPERAND_InitializeUsingLOperand(rOperand, &LeftParameters->lOperands[0]);
            status = clGenGenericCode2(Compiler,
                                       Expr->base.lineNo,
                                       Expr->base.stringNo,
                                       clvOPCODE_ADD,
                                       iOperand,
                                       rOperand,
                                       scaledIndex);
             if (gcmIS_ERROR(status)) return status;
             Parameters->dataTypes[0].byteOffset = 0;
             clsLOPERAND_InitializeUsingIOperand(&Parameters->lOperands[0], iOperand);
          }
          else {
             Parameters->dataTypes[0].byteOffset = _GetIntegerValue(scaledIndex);
             for (i = 0; i < Parameters->operandCount; i++) {
               Parameters->lOperands[i] = LeftParameters->lOperands[0];
             }
          }
          Parameters->hint = clvGEN_DEREF_CODE;
        }
        else {
          clsROPERAND rOperand[1];

          for (i = 0; i < Parameters->operandCount; i++) {
            cloIR_UNARY_EXPR unaryExpr = gcvNULL;

            if(Parameters->hint & clvGEN_COMPONENT_SELECT_CODE) {
              clsGEN_CODE_DATA_TYPE selectionDataType;

              unaryExpr = (cloIR_UNARY_EXPR) &Parameters->expr->base;
#ifdef _cldIncludeComponentOffset
              clmGEN_CODE_ConvDirectElementDataType(unaryExpr->exprBase.decl.dataType,
                                                    selectionDataType);
              if(gcIsScalarDataType(selectionDataType)) {
                 clmGEN_CODE_GetParametersIOperand(Compiler, iOperand, Parameters, selectionDataType);
              }
              else {
                 clmGEN_CODE_GetParametersIOperand(Compiler, iOperand, Parameters, Parameters->dataTypes[i].def);
              }
#else
              clmGEN_CODE_GetParametersIOperand(Compiler, iOperand, Parameters, Parameters->dataTypes[i].def);
#endif
            }
            else {
              clmGEN_CODE_GetParametersIOperand(Compiler, iOperand, Parameters, Parameters->dataTypes[i].def);
            }

            clsROPERAND_InitializeUsingLOperand(rOperand, LeftParameters->lOperands);

            if(Parameters->hint & clvGEN_COMPONENT_SELECT_CODE) {
               status = _GenSelectiveLoadCode(Compiler,
                                              Expr->base.lineNo,
                                              Expr->base.stringNo,
                                              iOperand,
                                              rOperand,
                                              scaledIndex,
                                              Parameters->dataTypes[i].def,
                                              &unaryExpr->u.componentSelection);
               if (gcmIS_ERROR(status)) return status;
#ifdef _cldIncludeComponentOffset
               if(gcIsScalarDataType(iOperand->dataType)) {
                  Parameters->hint |= clvGEN_SELECTIVE_LOADED;
               }
#endif
            }
            else {
               status = clGenGenericCode2(Compiler,
                                          Expr->base.lineNo,
                                          Expr->base.stringNo,
                                          clvOPCODE_LOAD,
                                          iOperand,
                                          rOperand,
                                          scaledIndex);
               if (gcmIS_ERROR(status)) return status;
            }

/* KLC - to do
   Need to adjust scaledIndex for each operand */
            clsLOPERAND_InitializeUsingIOperand(Parameters->lOperands + i, iOperand);
          }
        }
      }

      if(Parameters->needROperand) {
        if(clmDECL_IsUnderlyingStructOrUnion(&Expr->decl)) {
          if(scaledIndex->isReg) {
            clsIOPERAND_New(Compiler, iOperand, LeftParameters->rOperands[0].dataType);
            status = clGenGenericCode2(Compiler,
                                       Expr->base.lineNo,
                                       Expr->base.stringNo,
                                       clvOPCODE_ADD,
                                       iOperand,
                                       &LeftParameters->rOperands[0],
                                       scaledIndex);
             if (gcmIS_ERROR(status)) return status;
             Parameters->dataTypes[0].byteOffset = 0;
             clsROPERAND_InitializeUsingIOperand(&Parameters->rOperands[0], iOperand);
          }
          else {
             Parameters->dataTypes[0].byteOffset = _GetIntegerValue(scaledIndex);
             for (i = 0; i < Parameters->operandCount; i++) {
               Parameters->rOperands[i] = LeftParameters->rOperands[0];
             }
          }
          Parameters->hint = clvGEN_DEREF_CODE;
        }
        else {
          for (i = 0; i < Parameters->operandCount; i++) {
               cloIR_UNARY_EXPR unaryExpr = gcvNULL;

               if(Parameters->hint & clvGEN_COMPONENT_SELECT_CODE) {
                  clsGEN_CODE_DATA_TYPE selectionDataType;

                  unaryExpr = (cloIR_UNARY_EXPR) &Parameters->expr->base;
#ifdef _cldIncludeComponentOffset
                  clmGEN_CODE_ConvDirectElementDataType(unaryExpr->exprBase.decl.dataType,
                                                        selectionDataType);

                  if(Parameters->hasIOperand) {
                     clmGEN_CODE_GetParametersIOperand(Compiler, iOperand, Parameters, selectionDataType);
                  }
                  else {
                      if(gcIsScalarDataType(selectionDataType)) {
                         clmGEN_CODE_GetParametersIOperand(Compiler, iOperand, Parameters, selectionDataType);
                      }
                      else {
                         clmGEN_CODE_GetParametersIOperand(Compiler, iOperand, Parameters, Parameters->dataTypes[i].def);
                      }
                  }
#else
                  clmGEN_CODE_GetParametersIOperand(Compiler, iOperand, Parameters, Parameters->dataTypes[i].def);
#endif
                  if(Parameters->operandCount == 1) {
                     gctBOOL loadNeeded;
                     cloCODE_GENERATOR codeGenerator;

                     codeGenerator = cloCOMPILER_GetCodeGenerator(Compiler);
                     loadNeeded = clGenNeedVectorUpdate(Compiler,
                                                        codeGenerator,
                                                        Expr,
                                                        Parameters->dataTypes[0].def,
                                                        scaledIndex,
                                                        iOperand);
                     if(!loadNeeded) break;
                  }
                  if(Parameters->hasIOperand &&
                     gcIsVectorDataType(selectionDataType) &&
                     !_IsDefaultComponentSelection(&unaryExpr->u.componentSelection)) {
                     clsIOPERAND intermIOperand[1];
                     clsROPERAND intermROperand[1];
                     clsLOPERAND intermLOperand[1];

                     clsIOPERAND_New(Compiler, intermIOperand, Parameters->dataTypes[i].def);
                     status = _GenSelectiveLoadCode(Compiler,
                                                    Expr->base.lineNo,
                                                    Expr->base.stringNo,
                                                    intermIOperand,
                                                    LeftParameters->rOperands,
                                                    scaledIndex,
                                                    Parameters->dataTypes[i].def,
                                                    &unaryExpr->u.componentSelection);
                     if (gcmIS_ERROR(status)) return status;

                     clsLOPERAND_InitializeUsingIOperand(intermLOperand, iOperand);
                     clsROPERAND_InitializeUsingIOperand(intermROperand, intermIOperand);
                     _clmGetSubsetDataType(intermROperand->dataType,
                                           unaryExpr->u.componentSelection.components,
                                           intermROperand->dataType);
                     intermROperand->u.reg.componentSelection = unaryExpr->u.componentSelection;

                     status = clGenAssignCode(Compiler,
                                              Expr->base.lineNo,
                                              Expr->base.stringNo,
                                              intermLOperand,
                                              intermROperand);
                     if (gcmIS_ERROR(status)) return status;
                     Parameters->hint |= clvGEN_SELECTIVE_LOADED;  /* set to selectively loaded */
                  }
                  else {
                     status = _GenSelectiveLoadCode(Compiler,
                                                    Expr->base.lineNo,
                                                    Expr->base.stringNo,
                                                    iOperand,
                                                    LeftParameters->rOperands,
                                                    scaledIndex,
                                                    Parameters->dataTypes[i].def,
                                                    &unaryExpr->u.componentSelection);
                     if (gcmIS_ERROR(status)) return status;
                  }

#ifdef _cldIncludeComponentOffset
                  if(gcIsScalarDataType(iOperand->dataType)) {
                     Parameters->hint |= clvGEN_SELECTIVE_LOADED;
                  }
#endif
               }
               else {
                  clmGEN_CODE_GetParametersIOperand(Compiler, iOperand, Parameters, Parameters->dataTypes[i].def);
                  if(Parameters->operandCount == 1) {
                     gctBOOL loadNeeded;
                     cloCODE_GENERATOR codeGenerator;

                     codeGenerator = cloCOMPILER_GetCodeGenerator(Compiler);
                     loadNeeded = clGenNeedVectorUpdate(Compiler,
                                                        codeGenerator,
                                                        Expr,
                                                        Parameters->dataTypes[0].def,
                                                        scaledIndex,
                                                        iOperand);
                     if(!loadNeeded) break;
                  }
                  status = clGenGenericCode2(Compiler,
                                             Expr->base.lineNo,
                                             Expr->base.stringNo,
                                             clvOPCODE_LOAD,
                                             iOperand,
                                             LeftParameters->rOperands,
                                             scaledIndex);
                  if (gcmIS_ERROR(status)) return status;
               }
/* KLC - to do
   Need to adjust scaledIndex for each operand */
               clsROPERAND_InitializeUsingIOperand(Parameters->rOperands + i, iOperand);
            }
          }
       }
     }
  }

  return gcvSTATUS_OK;
}

static clsROPERAND *
_ResolveComponentSelection(
    IN cloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN clsROPERAND *ROperand,
    IN clsROPERAND *ResultBuf
)
{
   if(ROperand->isReg &&
      !_IsDefaultComponentSelection(&ROperand->u.reg.componentSelection)) {
       clsIOPERAND intermIOperand[1];
       clsLOPERAND lOperand[1];
       gceSTATUS status;

       clsIOPERAND_New(Compiler, intermIOperand, ROperand->dataType);
       clsLOPERAND_InitializeUsingIOperand(lOperand, intermIOperand);
       status = clGenAssignCode(Compiler,
                                LineNo,
                                StringNo,
                                lOperand,
                                ROperand);
       if (gcmIS_ERROR(status)) return gcvNULL;
       clsROPERAND_InitializeUsingIOperand(ResultBuf, intermIOperand);
       return ResultBuf;
   }
   else return ROperand;
}

static cleOPCODE
_AdjustAstypeOpcode(
    cleOPCODE Opcode,
    cltELEMENT_TYPE TargetElementType
    )
{
    cleOPCODE opcode = Opcode;

    gcmASSERT(opcode == clvOPCODE_ASTYPE);

    switch(TargetElementType) {
    case clvTYPE_INT:
    case clvTYPE_UINT:
    case clvTYPE_FLOAT:
        opcode = clvOPCODE_ASSIGN;
        break;

    default:
        break;
    }

    return opcode;
}

#if cldVector8And16
gceSTATUS
clGenGenericCode1(
    IN cloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN cleOPCODE Opcode,
    IN clsIOPERAND * IOperand,
    IN clsROPERAND * ROperand
    )
{
    gceSTATUS    status;
    gcsSUPER_TARGET superTarget;
    gcsSUPER_SOURCE superSource;
    clsROPERAND rOperand[1];
    gctSIZE_T iSize, rSize, i, j;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    gcmASSERT(IOperand);
    gcmASSERT(ROperand);

    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                 clvDUMP_CODE_GENERATOR,
                 "<OPERATION line=\"%d\" string=\"%d\" type=\"%s\">",
                 LineNo,
                 StringNo,
                 clGetOpcodeName(Opcode)));

    gcmVERIFY_OK(clsIOPERAND_Dump(Compiler, IOperand));

    gcmVERIFY_OK(clsROPERAND_Dump(Compiler, ROperand));

    switch (Opcode) {
    case clvOPCODE_NORMALIZE:
        ROperand = _ResolveComponentSelection(Compiler,
                                              LineNo,
                                              StringNo,
                                              ROperand,
                                              rOperand);
        gcmASSERT(ROperand);
        break;

    default:
        break;
    }

    iSize = gcGetDataTypeRegSize(IOperand->dataType);
    if(iSize > 1) {
       rSize = gcGetDataTypeRegSize(ROperand->dataType);

       gcmASSERT((iSize == rSize || rSize == 1));

       for(i = 0; i < iSize; i++) {
           status = _ConvIOperandToSectionalTarget(Compiler,
                                                   IOperand,
                                                   (gctUINT8)i,
                                                   &superTarget);
           if (gcmIS_ERROR(status)) return status;

/* KLC - may be able to break out from the loop instead of continue */
           if(superTarget.numTargets == 0) continue;

           status = _ConvNormalROperandToSectionalSource(Compiler,
                                                         LineNo,
                                                         StringNo,
                                                         ROperand,
                                                         (gctUINT8)i,
                                                         superTarget.numTargets,
                                                         &superSource);
           if (gcmIS_ERROR(status)) return status;

           _SplitTargets(&superTarget, superSource.numSources);
           _SplitSources(&superSource, superTarget.numTargets);
           gcmASSERT(superTarget.numTargets == superSource.numSources);

           if(Opcode == clvOPCODE_ASTYPE) {
               Opcode = _AdjustAstypeOpcode(Opcode,
                                            IOperand->dataType.elementType);
           }

           if(clmIsOpcodeConv(Opcode)) {
               clsGEN_CODE_DATA_TYPE srcType = ROperand->dataType;

               if(Opcode == clvOPCODE_ASTYPE) {
                   Opcode = clvOPCODE_CONV;
                   srcType = IOperand->dataType;
               }

               for(j=0; j < superTarget.numTargets; j++) {
                   status = clEmitConvCode(Compiler,
                                           LineNo,
                                           StringNo,
                                           Opcode,
                                           superTarget.targets + j,
                                           superSource.sources + j,
                                           srcType);
                    if (gcmIS_ERROR(status)) return status;
                }
           }
           else {
               for(j=0; j < superTarget.numTargets; j++) {
                   status = clEmitCode1(Compiler,
                                        LineNo,
                                        StringNo,
                                        Opcode,
                                        superTarget.targets + j,
                                        superSource.sources + j);
                   if (gcmIS_ERROR(status)) return status;
               }
           }
        }
    }
    else {
        gcmVERIFY_OK(_ConvIOperandToSuperTarget(Compiler,
                                                IOperand,
                                                &superTarget));

        status = _ConvNormalROperandToSuperSource(Compiler,
                                                  LineNo,
                                                  StringNo,
                                                  ROperand,
                                                  &superSource);
        if (gcmIS_ERROR(status)) return status;


        _SplitTargets(&superTarget, superSource.numSources);
        _SplitSources(&superSource, superTarget.numTargets);
        gcmASSERT(superTarget.numTargets == superSource.numSources);

        if(Opcode == clvOPCODE_ASTYPE) {
            Opcode = _AdjustAstypeOpcode(Opcode,
                                         IOperand->dataType.elementType);
        }

        if(clmIsOpcodeConv(Opcode)) {
           clsGEN_CODE_DATA_TYPE srcType = ROperand->dataType;

           if(Opcode == clvOPCODE_ASTYPE) {
               Opcode = clvOPCODE_CONV;
               srcType = IOperand->dataType;
           }

           for(j=0; j < superTarget.numTargets; j++) {
               status = clEmitConvCode(Compiler,
                                       LineNo,
                                       StringNo,
                                       Opcode,
                                       superTarget.targets + j,
                                       superSource.sources + j,
                                       srcType);
               if (gcmIS_ERROR(status)) return status;
           }
        }
        else {
           for(j=0; j < superTarget.numTargets; j++) {
               status = clEmitCode1(Compiler,
                                    LineNo,
                                    StringNo,
                                    Opcode,
                                    superTarget.targets + j,
                                    superSource.sources + j);
               if (gcmIS_ERROR(status)) return status;
           }
        }
    }

    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                 clvDUMP_CODE_GENERATOR,
                 "</OPERATION>"));

    return gcvSTATUS_OK;
}

#else
gceSTATUS
clGenGenericCode1(
    IN cloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN cleOPCODE Opcode,
    IN clsIOPERAND * IOperand,
    IN clsROPERAND * ROperand
    )
{
    gceSTATUS    status;
    gcsTARGET    target;
    gcsSOURCE    source;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    gcmASSERT(IOperand);
    gcmASSERT(ROperand);

    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                clvDUMP_CODE_GENERATOR,
                "<OPERATION line=\"%d\" string=\"%d\" type=\"%s\">",
                LineNo,
                StringNo,
                clGetOpcodeName(Opcode)));

    gcmVERIFY_OK(clsIOPERAND_Dump(Compiler, IOperand));

    gcmVERIFY_OK(clsROPERAND_Dump(Compiler, ROperand));

    switch (Opcode)
    {
    case clvOPCODE_ASSIGN:
     case clvOPCODE_FLOAT_TO_INT:
     case clvOPCODE_FLOAT_TO_UINT:
    case clvOPCODE_FLOAT_TO_BOOL:
    case clvOPCODE_INT_TO_INT:
    case clvOPCODE_INT_TO_BOOL:
    case clvOPCODE_INT_TO_FLOAT:
    case clvOPCODE_INT_TO_UINT:
    case clvOPCODE_UINT_TO_UINT:
    case clvOPCODE_UINT_TO_FLOAT:
    case clvOPCODE_UINT_TO_INT:
    case clvOPCODE_UINT_TO_BOOL:
    case clvOPCODE_BOOL_TO_FLOAT:
    case clvOPCODE_BOOL_TO_INT:
    case clvOPCODE_BOOL_TO_UINT:

    case clvOPCODE_IMPL_B2F:
    case clvOPCODE_IMPL_U2F:
    case clvOPCODE_IMPL_I2F:

    case clvOPCODE_BARRIER:

    case clvOPCODE_INVERSE:

    case clvOPCODE_ANY:
    case clvOPCODE_ALL:
    case clvOPCODE_NOT:
    case clvOPCODE_NEG:
    case clvOPCODE_BITWISE_NOT:

    case clvOPCODE_SIN:
    case clvOPCODE_COS:
    case clvOPCODE_TAN:

    case clvOPCODE_ASIN:
    case clvOPCODE_ACOS:
    case clvOPCODE_ATAN:

    case clvOPCODE_EXP2:
    case clvOPCODE_LOG2:
    case clvOPCODE_SQRT:
    case clvOPCODE_INVERSE_SQRT:

    case clvOPCODE_ABS:
    case clvOPCODE_SIGN:
    case clvOPCODE_FLOOR:
    case clvOPCODE_CEIL:
    case clvOPCODE_FRACT:
    case clvOPCODE_SATURATE:

    case clvOPCODE_NORMALIZE:

    case clvOPCODE_LEADZERO:
    case clvOPCODE_GETEXP:
    case clvOPCODE_GETMANT:
    case clvOPCODE_CONV:
    case clvOPCODE_CONV_SAT:
    case clvOPCODE_CONV_SAT_RTE:
    case clvOPCODE_CONV_SAT_RTZ:
    case clvOPCODE_CONV_SAT_RTN:
    case clvOPCODE_CONV_SAT_RTP:
    case clvOPCODE_CONV_RTE:
    case clvOPCODE_CONV_RTZ:
    case clvOPCODE_CONV_RTN:
    case clvOPCODE_CONV_RTP:

    case clvOPCODE_MULLO:
    case clvOPCODE_MULHI:
    case clvOPCODE_ADDLO:

    case clvOPCODE_DFDX:
    case clvOPCODE_DFDY:
    case clvOPCODE_FWIDTH:

    case clvOPCODE_SINPI:
    case clvOPCODE_COSPI:
    case clvOPCODE_TANPI:

    case clvOPCODE_ASTYPE:

    case clvOPCODE_LONGLO:
    case clvOPCODE_LONGHI:
        break;

    default: gcmASSERT(0);
    }

    gcmVERIFY_OK(_ConvIOperandToTarget(Compiler,
                       IOperand,
                       &target));

    status = _ConvNormalROperandToSource(Compiler,
                         LineNo,
                         StringNo,
                         ROperand,
                         &source);

    if (gcmIS_ERROR(status)) return status;

    switch(Opcode) {
    case clvOPCODE_IMPL_B2F:
    case clvOPCODE_IMPL_U2F:
    case clvOPCODE_IMPL_I2F:
       break;

    }

    if(Opcode == clvOPCODE_ASTYPE) {
        Opcode = _AdjustAstypeOpcode(Opcode,
                                     IOperand->dataType.elementType);
    }

    if(clmIsOpcodeConv(Opcode)) {
       clsGEN_CODE_DATA_TYPE srcType = ROperand->dataType;

       if(Opcode == clvOPCODE_ASTYPE) {
           Opcode = clvOPCODE_CONV;
           srcType = IOperand->dataType;
       }

       status = clEmitConvCode(Compiler,
                               LineNo,
                               StringNo,
                               Opcode,
                               &target,
                               &source,
                               srcType);
    }
    else {
       status = clEmitCode1(Compiler,
                            LineNo,
                            StringNo,
                            Opcode,
                            &target,
                            &source);
    }
    if (gcmIS_ERROR(status)) return status;

    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                clvDUMP_CODE_GENERATOR,
                "</OPERATION>"));

    return gcvSTATUS_OK;
}
#endif

gceSTATUS
clGenGenericNullTargetCode(
    IN cloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN cleOPCODE Opcode,
    IN clsROPERAND * ROperand0,
    IN clsROPERAND * ROperand1
    )
{
   gceSTATUS status;
   gcsSUPER_SOURCE superSource0;
   gcsSUPER_SOURCE superSource1;
   int j;

   /* Verify the arguments. */
   clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);

   gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                                 clvDUMP_CODE_GENERATOR,
                                 "<OPERATION line=\"%d\" string=\"%d\" type=\"%s\">",
                                 LineNo,
                                 StringNo,
                                 clGetOpcodeName(Opcode)));

   switch (Opcode) {
   case clvOPCODE_BARRIER:
      break;

   default:
      gcmASSERT(0);
      return gcvSTATUS_INVALID_DATA;
   }

   superSource0.numSources = 0;
   superSource1.numSources = 0;
   if(ROperand0) {
      gcmVERIFY_OK(clsROPERAND_Dump(Compiler, ROperand0));
      status = _ConvNormalROperandToSuperSource(Compiler,
                                                LineNo,
                                                StringNo,
                                                ROperand0,
                                                &superSource0);
      if (gcmIS_ERROR(status)) return status;
   }

   if(ROperand1) {
      gcmVERIFY_OK(clsROPERAND_Dump(Compiler, ROperand1));
      status = _ConvNormalROperandToSuperSource(Compiler,
                            LineNo,
                            StringNo,
                            ROperand1,
                            &superSource1);
      if (gcmIS_ERROR(status)) return status;
   }

   if(ROperand0 && ROperand1) {
      _SplitSources(&superSource0, superSource1.numSources);
      _SplitSources(&superSource1, superSource0.numSources);
      gcmASSERT(superSource0.numSources == superSource1.numSources);

      for(j = 0; j < superSource0.numSources; j++) {
     status = clEmitNullTargetCode(Compiler,
                                       LineNo,
                                       StringNo,
                                       Opcode,
                                       superSource0.sources + j,
                                       superSource1.sources + j);
         if (gcmIS_ERROR(status)) return status;
      }
   }
   else if(ROperand0) {
      for(j = 0; j < superSource0.numSources; j++) {
     status = clEmitNullTargetCode(Compiler,
                                       LineNo,
                                       StringNo,
                                       Opcode,
                                       superSource0.sources + j,
                                       gcvNULL);
         if (gcmIS_ERROR(status)) return status;
      }
   }
   else if(ROperand1) {
      for(j = 0; j < superSource1.numSources; j++) {
         status = clEmitNullTargetCode(Compiler,
                                       LineNo,
                                       StringNo,
                                       Opcode,
                                       gcvNULL,
                           superSource1.sources + j);
     if (gcmIS_ERROR(status)) return status;
      }
   }
   else {
      status = clEmitNullTargetCode(Compiler,
                                    LineNo,
                                    StringNo,
                                    Opcode,
                                    gcvNULL,
                                    gcvNULL);
      if (gcmIS_ERROR(status)) return status;
   }

   gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                     clvDUMP_CODE_GENERATOR,
                     "</OPERATION>"));

    return gcvSTATUS_OK;
}

/* Optimize offset usage when it is a constant */
gceSTATUS
clGenLoadCode(
IN cloCOMPILER Compiler,
IN gctUINT LineNo,
IN gctUINT StringNo,
IN clsIOPERAND *IOperand,
IN clsROPERAND *ROperand,
IN clsROPERAND *Offset
)
{
  gceSTATUS status;
  gcsSUPER_TARGET superTarget;
  gcsSOURCE source0[1];
  gcsSOURCE source1[1];
  gctSIZE_T iSize, i, j;
  clsROPERAND *addressOperand;
  clsROPERAND addressBuffer[1];
  clsIOPERAND columnIOperand[1];
  gctUINT numColumns;
  clsROPERAND *offset;
  clsROPERAND offsetBuffer[1];
  gctINT incr;

/* Verify the arguments. */
  clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
  gcmASSERT(ROperand);
  gcmASSERT(IOperand);
  gcmASSERT(Offset);

  gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                                clvDUMP_CODE_GENERATOR,
                                "<OPERATION line=\"%d\" string=\"%d\" type=\"store\">",
                                LineNo,
                                StringNo));

  gcmVERIFY_OK(clsROPERAND_Dump(Compiler, ROperand));
  gcmVERIFY_OK(clsIOPERAND_Dump(Compiler, IOperand));
  gcmVERIFY_OK(clsROPERAND_Dump(Compiler, Offset));

  if(gcIsMatrixDataType(IOperand->dataType)) {
     clsIOPERAND_InitializeAsMatrixColumn(columnIOperand, IOperand, 0);
     numColumns = gcGetMatrixDataTypeColumnCount(IOperand->dataType);
  }
  else {
     *columnIOperand = *IOperand;
     numColumns = 1;
  }

  iSize = gcGetDataTypeRegSize(columnIOperand->dataType);

  if(Offset->isReg) {
     if(iSize > 1 || numColumns > 1) {
        status = _AddROperandOffset(Compiler,
                                    LineNo,
                                    StringNo,
                                    ROperand,
                                    Offset,
                                    addressBuffer);
        if (gcmIS_ERROR(status)) return status;

        addressOperand = addressBuffer;
        _InitializeROperandConstant(offsetBuffer, clmGenCodeDataType(T_UINT), 0);
        offset = offsetBuffer;
     }
     else {
        addressOperand = ROperand;
        offset = Offset;
     }
  }
  else {
     addressOperand = ROperand;
     _InitializeROperandConstant(offsetBuffer,
                                 Offset->dataType,
                                 _GetIntegerValue(Offset));
     offset = offsetBuffer;
  }

  status = _ConvNormalROperandToSource(Compiler,
                       LineNo,
                       StringNo,
                       addressOperand,
                       source0);
  if (gcmIS_ERROR(status)) return status;

  incr = 0;
  do {
     for(i = 0; i < iSize; i++) {
        gcmVERIFY_OK(_ConvIOperandToSectionalTarget(Compiler,
                                                    columnIOperand,
                                                    (gctUINT8)i,
                                                    &superTarget));

        for(j=0; j < superTarget.numTargets; j++) {
           if(offset->isReg && incr) {
              status = _AddROperandOffset(Compiler,
                                          LineNo,
                                          StringNo,
                                          addressOperand,
                                          offset,
                                          addressBuffer);
              if (gcmIS_ERROR(status)) return status;

              addressOperand = addressBuffer;
              status = _ConvNormalROperandToSource(Compiler,
                                   LineNo,
                                   StringNo,
                                   addressOperand,
                                   source0);
              if (gcmIS_ERROR(status)) return status;
              _InitializeROperandConstant(offsetBuffer, clmGenCodeDataType(T_UINT), 0);
              offset = offsetBuffer;
           }
           status = clGenAddToOffset(offset,
                                     incr);
           if (gcmIS_ERROR(status)) return status;

           status = _ConvNormalROperandToSource(Compiler,
                                                LineNo,
                                                StringNo,
                                                offset,
                                                source1);
           if (gcmIS_ERROR(status)) return status;

           status = clEmitCode2(Compiler,
                                LineNo,
                                StringNo,
                                clvOPCODE_LOAD,
                                superTarget.targets + j,
                                source0,
                                source1);
           if (gcmIS_ERROR(status)) return status;
           incr = (gctINT)_DataTypeByteSize(superTarget.targets[j].dataType);
        }
     }
     columnIOperand->tempRegIndex += (gctREG_INDEX)iSize;
  } while(--numColumns);

  gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                                clvDUMP_CODE_GENERATOR,
                                "</OPERATION>"));

  return gcvSTATUS_OK;
}

#if cldVector8And16
gceSTATUS
clGenGenericCode2(
    IN cloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN cleOPCODE Opcode,
    IN clsIOPERAND * IOperand,
    IN clsROPERAND * ROperand0,
    IN clsROPERAND * ROperand1
    )
{
    gceSTATUS status;
    gcsSUPER_TARGET superTarget;
    gcsSUPER_SOURCE superSource0;
    gcsSUPER_SOURCE superSource1;
    gctSIZE_T iSize, rSize0, rSize1, i, j;
    clsIOPERAND iOperand[1];
    clsROPERAND rOperand0[1], rOperand1[1];
    clsIOPERAND *compareIOperand = gcvNULL;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    gcmASSERT(IOperand);
    gcmASSERT(ROperand0);
    gcmASSERT(ROperand1);

    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                 clvDUMP_CODE_GENERATOR,
                 "<OPERATION line=\"%d\" string=\"%d\" type=\"%s\">",
                 LineNo,
                 StringNo,
                 clGetOpcodeName(Opcode)));

    gcmVERIFY_OK(clsIOPERAND_Dump(Compiler, IOperand));

    gcmVERIFY_OK(clsROPERAND_Dump(Compiler, ROperand0));

    gcmVERIFY_OK(clsROPERAND_Dump(Compiler, ROperand1));

    switch (Opcode) {
    case clvOPCODE_LESS_THAN:
    case clvOPCODE_LESS_THAN_EQUAL:
    case clvOPCODE_GREATER_THAN:
    case clvOPCODE_GREATER_THAN_EQUAL:
    case clvOPCODE_EQUAL:
    case clvOPCODE_NOT_EQUAL:
        {
            cltELEMENT_TYPE elementType;
            clsGEN_CODE_DATA_TYPE newDataType;

            elementType = clmGEN_CODE_elementType_GET(IOperand->dataType);
            if(clmIsElementTypeInteger(elementType)) {
                switch(elementType) {
                case clvTYPE_INT:
                case clvTYPE_UINT:
                    break;

                case clvTYPE_LONG:
                case clvTYPE_ULONG:
                    if(gcIsVectorDataType(IOperand->dataType))
                    {
                       IOperand->dataType.elementType = clvTYPE_LONG;
                    }
                    else
                    {
                       IOperand->dataType.elementType = clvTYPE_INT;
                    }
                    break;

                default:
                    compareIOperand = IOperand;
                    newDataType = IOperand->dataType;
                    if(clmIsElementTypeUnsigned(elementType)) {
                        newDataType.elementType = clvTYPE_UINT;
                    }
                    else {
                        newDataType.elementType = clvTYPE_INT;
                    }
                    clsIOPERAND_New(Compiler, iOperand, newDataType);
                    IOperand = iOperand;
                    break;
                }
            }
        }
        break;

    case clvOPCODE_LOAD:
       return clGenLoadCode(Compiler,
                            LineNo,
                            StringNo,
                            IOperand,
                            ROperand0,
                            ROperand1);

    case clvOPCODE_TEXTURE_LOAD:
    case clvOPCODE_IMAGE_SAMPLER:
    case clvOPCODE_IMAGE_READ:
    case clvOPCODE_IMAGE_WRITE:
    case clvOPCODE_ATAN2:

    case clvOPCODE_POW:

    case clvOPCODE_ROTATE:
    case clvOPCODE_MIN:
    case clvOPCODE_MAX:
    case clvOPCODE_STEP:
    case clvOPCODE_CMP:
    case clvOPCODE_FLOAT_TO_UINT:
    case clvOPCODE_DIV:
    case clvOPCODE_IDIV:
    case clvOPCODE_IMUL:
    case clvOPCODE_MOD:
    case clvOPCODE_FMOD:
    case clvOPCODE_SELECT:
    case clvOPCODE_RSHIFT:
    case clvOPCODE_LSHIFT:
    case clvOPCODE_RIGHT_SHIFT:
    case clvOPCODE_LEFT_SHIFT:

    case clvOPCODE_ADD:
    case clvOPCODE_ADDLO:
    case clvOPCODE_FADD:
    case clvOPCODE_SUB:
    case clvOPCODE_FSUB:
    case clvOPCODE_MUL:
    case clvOPCODE_MULLO:
    case clvOPCODE_FMUL:

    case clvOPCODE_ADD_RTZ:
    case clvOPCODE_ADD_RTNE:
    case clvOPCODE_SUB_RTZ:
    case clvOPCODE_SUB_RTNE:
    case clvOPCODE_MUL_RTZ:
    case clvOPCODE_MUL_RTNE:

    case clvOPCODE_ATOMADD:
    case clvOPCODE_ATOMSUB:
    case clvOPCODE_ATOMXCHG:
    case clvOPCODE_ATOMCMPXCHG:
    case clvOPCODE_ATOMMIN:
    case clvOPCODE_ATOMMAX:
    case clvOPCODE_ATOMOR:
    case clvOPCODE_ATOMAND:
    case clvOPCODE_ATOMXOR:

    case clvOPCODE_MOV_LONG:
    case clvOPCODE_COPY:
    case clvOPCODE_UNPACK:
        break;

    case clvOPCODE_DOT:
    case clvOPCODE_CROSS:
        ROperand0 = _ResolveComponentSelection(Compiler,
                                               LineNo,
                                               StringNo,
                                               ROperand0,
                                               rOperand0);
        gcmASSERT(ROperand0);
        ROperand1 = _ResolveComponentSelection(Compiler,
                                               LineNo,
                                               StringNo,
                                               ROperand1,
                                               rOperand1);
        gcmASSERT(ROperand1);
        break;

    default: gcmASSERT(0);
    }

    iSize = gcGetDataTypeRegSize(IOperand->dataType);
    if(iSize > 1) {
       rSize0 = gcGetDataTypeRegSize(ROperand0->dataType);
       rSize1 = gcGetDataTypeRegSize(ROperand1->dataType);

       gcmASSERT((iSize == rSize0 || rSize0 == 1) &&
                 (iSize == rSize1 || rSize1 == 1));

       for(i = 0; i < iSize; i++) {
           status = _ConvIOperandToSectionalTarget(Compiler,
                                                   IOperand,
                                                   (gctUINT8)i,
                                                    &superTarget);
           if (gcmIS_ERROR(status)) return status;

/* KLC - may be able to break out from the loop instead of continue */
           if(superTarget.numTargets == 0) continue;

           status = _ConvNormalROperandToSectionalSource(Compiler,
                                                         LineNo,
                                                         StringNo,
                                                         ROperand0,
                                                         (gctUINT8)i,
                                                         superTarget.numTargets,
                                                         &superSource0);
           if (gcmIS_ERROR(status)) return status;

           status = _ConvNormalROperandToSectionalSource(Compiler,
                                                         LineNo,
                                                         StringNo,
                                                         ROperand1,
                                                         (gctUINT8)i,
                                                         superTarget.numTargets,
                                                         &superSource1);
           if (gcmIS_ERROR(status)) return status;
           _SplitTargets(&superTarget,
                         superSource0.numSources > superSource1.numSources
                         ? superSource0.numSources
                         : superSource1.numSources);
           _SplitSources(&superSource0, superTarget.numTargets);
           _SplitSources(&superSource1, superTarget.numTargets);
           gcmASSERT(superTarget.numTargets == superSource0.numSources &&
                     superTarget.numTargets == superSource1.numSources);

           for(j=0; j < superTarget.numTargets; j++) {
               status = clEmitCode2(Compiler,
                                    LineNo,
                                    StringNo,
                                    Opcode,
                                    superTarget.targets + j,
                                    superSource0.sources + j,
                                    superSource1.sources + j);
                if (gcmIS_ERROR(status)) return status;
            }
        }
    }
    else {
        gcmVERIFY_OK(_ConvIOperandToSuperTarget(Compiler,
                                                IOperand,
                                                &superTarget));

        status = _ConvNormalROperandToSuperSource(Compiler,
                                                  LineNo,
                                                  StringNo,
                                                  ROperand0,
                                                  &superSource0);
        if (gcmIS_ERROR(status)) return status;
        if(Opcode == clvOPCODE_TEXTURE_LOAD) {
           gcmASSERT(superSource0.numSources == 1);
           gcmASSERT(gcIsSamplerDataType(ROperand0->dataType));

           /* adjust source swizzle if it is a sampler index */
           if(superSource0.sources[0].type == gcvSOURCE_TEMP) {
               superSource0.sources[0].u.sourceReg.swizzle =
                   _ConvComponentSelectionToSwizzle(&ROperand0->u.reg.componentSelection);
           }
        }

        status = _ConvNormalROperandToSuperSource(Compiler,
                                                  LineNo,
                                                  StringNo,
                                                  ROperand1,
                                                  &superSource1);
        if (gcmIS_ERROR(status)) return status;

        _SplitTargets(&superTarget,
                      superSource0.numSources > superSource1.numSources
                      ? superSource0.numSources
                      : superSource1.numSources);
        _SplitSources(&superSource0, superTarget.numTargets);
        _SplitSources(&superSource1, superTarget.numTargets);
        gcmASSERT(superTarget.numTargets == superSource0.numSources &&
                  superTarget.numTargets == superSource1.numSources);

        for(j=0; j < superTarget.numTargets; j++) {
            status = clEmitCode2(Compiler,
                                 LineNo,
                                 StringNo,
                                 Opcode,
                                 superTarget.targets + j,
                                 superSource0.sources + j,
                                 superSource1.sources + j);
            if (gcmIS_ERROR(status)) return status;
        }
    }

    if(compareIOperand) {
        clsROPERAND rOperand[1];
        clsLOPERAND lOperand[1];

        clsROPERAND_InitializeUsingIOperand(rOperand, IOperand);
        clsLOPERAND_InitializeUsingIOperand(lOperand, compareIOperand);

        status = clGenAssignCode(Compiler,
                                 LineNo,
                                 StringNo,
                                 lOperand,
                                 rOperand);
        if (gcmIS_ERROR(status)) return status;
    }

    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                 clvDUMP_CODE_GENERATOR,
                 "</OPERATION>"));

    return gcvSTATUS_OK;
}

#else
gceSTATUS
clGenGenericCode2(
    IN cloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN cleOPCODE Opcode,
    IN clsIOPERAND * IOperand,
    IN clsROPERAND * ROperand0,
    IN clsROPERAND * ROperand1
    )
{
    gceSTATUS status;
    gcsTARGET target;
    gcsSOURCE source0;
    gcsSOURCE source1;
    clsIOPERAND iOperand[1];
    clsIOPERAND *compareIOperand = gcvNULL;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    gcmASSERT(IOperand);
    gcmASSERT(ROperand0);
    gcmASSERT(ROperand1);

    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                clvDUMP_CODE_GENERATOR,
                "<OPERATION line=\"%d\" string=\"%d\" type=\"%s\">",
                LineNo,
                StringNo,
                clGetOpcodeName(Opcode)));

    gcmVERIFY_OK(clsIOPERAND_Dump(Compiler, IOperand));

    gcmVERIFY_OK(clsROPERAND_Dump(Compiler, ROperand0));

    gcmVERIFY_OK(clsROPERAND_Dump(Compiler, ROperand1));

    switch (Opcode)
    {
    case clvOPCODE_LESS_THAN:
    case clvOPCODE_LESS_THAN_EQUAL:
    case clvOPCODE_GREATER_THAN:
    case clvOPCODE_GREATER_THAN_EQUAL:
    case clvOPCODE_EQUAL:
    case clvOPCODE_NOT_EQUAL:
       {
          cltELEMENT_TYPE elementType;
          clsGEN_CODE_DATA_TYPE newDataType;

          elementType = clmGEN_CODE_elementType_GET(IOperand->dataType);
          if(clmIsElementTypeInteger(elementType) &&
                 (elementType != clvTYPE_INT || elementType != clvTYPE_UINT)) {
         compareIOperand = IOperand;
         newDataType = IOperand->dataType;
         if(clmIsElementTypeUnsigned(elementType)) {
            newDataType.elementType = clvTYPE_UINT;
             }
             else {
            newDataType.elementType = clvTYPE_INT;
             }
             clsIOPERAND_New(Compiler, iOperand, newDataType);
             IOperand = iOperand;
          }
       }
       break;

    case clvOPCODE_TEXTURE_LOAD:
    case clvOPCODE_IMAGE_SAMPLER:
    case clvOPCODE_IMAGE_READ:
    case clvOPCODE_IMAGE_WRITE:
    case clvOPCODE_LOAD:

    case clvOPCODE_ATAN2:

    case clvOPCODE_POW:

    case clvOPCODE_ROTATE:
    case clvOPCODE_MIN:
    case clvOPCODE_MAX:
    case clvOPCODE_STEP:
    case clvOPCODE_DOT:
    case clvOPCODE_CROSS:
    case clvOPCODE_CMP:
    case clvOPCODE_FLOAT_TO_UINT:
    case clvOPCODE_DIV:
    case clvOPCODE_IDIV:
    case clvOPCODE_IMUL:
    case clvOPCODE_MOD:
    case clvOPCODE_FMOD:
    case clvOPCODE_SELECT:
    case clvOPCODE_RSHIFT:
    case clvOPCODE_LSHIFT:
    case clvOPCODE_RIGHT_SHIFT:
    case clvOPCODE_LEFT_SHIFT:

    case clvOPCODE_ADD:
    case clvOPCODE_ADDLO:
    case clvOPCODE_FADD:
    case clvOPCODE_ADD_RTZ:
    case clvOPCODE_ATAN2PI:
    case clvOPCODE_SUB:
    case clvOPCODE_FSUB:
    case clvOPCODE_SUB_RTZ:
    case clvOPCODE_MUL:
    case clvOPCODE_MULLO:
    case clvOPCODE_FMUL:
    case clvOPCODE_MUL_RTZ:

    case clvOPCODE_ATOMADD:
    case clvOPCODE_ATOMSUB:
    case clvOPCODE_ATOMXCHG:
    case clvOPCODE_ATOMCMPXCHG:
    case clvOPCODE_ATOMMIN:
    case clvOPCODE_ATOMMAX:
    case clvOPCODE_ATOMOR:
    case clvOPCODE_ATOMAND:
    case clvOPCODE_ATOMXOR:

    case clvOPCODE_UNPACK:
        break;

    default: gcmASSERT(0);
    }

    gcmVERIFY_OK(_ConvIOperandToTarget(Compiler,
                       IOperand,
                       &target));

    status = _ConvNormalROperandToSource(Compiler,
                         LineNo,
                         StringNo,
                         ROperand0,
                         &source0);
    if (gcmIS_ERROR(status)) return status;

    status = _ConvNormalROperandToSource(Compiler,
                         LineNo,
                         StringNo,
                         ROperand1,
                         &source1);
    if (gcmIS_ERROR(status)) return status;

    status = clEmitCode2(Compiler,
                 LineNo,
                 StringNo,
                 Opcode,
                 &target,
                 &source0,
                 &source1);
    if (gcmIS_ERROR(status)) return status;

    if(compareIOperand) {
           clsROPERAND rOperand[1];
           clsLOPERAND lOperand[1];

       clsROPERAND_InitializeUsingIOperand(rOperand, IOperand);
           clsLOPERAND_InitializeUsingIOperand(lOperand, compareIOperand);

           status = clGenAssignCode(Compiler,
                                    LineNo,
                                    StringNo,
                                    lOperand,
                                rOperand);
           if (gcmIS_ERROR(status)) return status;
    }

    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                clvDUMP_CODE_GENERATOR,
                "</OPERATION>"));

    return gcvSTATUS_OK;
}
#endif

gceSTATUS
clGenDotCode(
    IN cloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN clsIOPERAND * IOperand,
    IN clsROPERAND * ROperand0,
    IN clsROPERAND * ROperand1
)
{
    gceSTATUS status;
    gctUINT8 vectorComponentCount;
    gctUINT8 startComponent;
    gctUINT8 sliceComponentCount;
    gctUINT8 rSize, i;
    clsROPERAND  rOperandSlice0[1];
    clsROPERAND  rOperandSlice1[1];
    clsIOPERAND intermIOperand[1];
    clsIOPERAND *iOperand;
    clsROPERAND intermROperand[1];
    clsROPERAND resultROperand[1];

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    gcmASSERT(ROperand0);
    gcmASSERT(ROperand1);
    gcmASSERT(IOperand);

        if(gcIsScalarDataType(ROperand0->dataType)) {
            status = clGenGenericCode2(Compiler,
                                       LineNo,
                                       StringNo,
                                       clvOPCODE_DOT,
                                       IOperand,
                                       ROperand0,
                                       ROperand1);
           return status;
        }

    vectorComponentCount = gcGetDataTypeComponentCount(ROperand0->dataType);
    rSize = (vectorComponentCount + 3) >> 2;
    gcmASSERT(rSize <= cldMaxFourComponentCount);

    if(rSize > 1) {
      clsIOPERAND_New(Compiler, intermIOperand, IOperand->dataType);
      clsROPERAND_InitializeUsingIOperand(intermROperand, intermIOperand);
      clsROPERAND_InitializeUsingIOperand(resultROperand, IOperand);
    }

    startComponent = 0;
    for(i = 0; i < rSize; i++) {
       sliceComponentCount = (startComponent + 4) > vectorComponentCount ?
                 (vectorComponentCount - startComponent) : 4;
       clGetVectorROperandSlice(ROperand0,
                    startComponent,
                    sliceComponentCount,
                    rOperandSlice0);

       clGetVectorROperandSlice(ROperand1,
                    startComponent,
                    sliceComponentCount,
                    rOperandSlice1);

       if(i == 0) {
        iOperand = IOperand;
       }
       else iOperand = intermIOperand;
       status = clGenGenericCode2(Compiler,
                      LineNo,
                      StringNo,
                      clvOPCODE_DOT,
                      iOperand,
                      rOperandSlice0,
                      rOperandSlice1);
       if (gcmIS_ERROR(status)) return status;

       if(i != 0) {
          status = clGenGenericCode2(Compiler,
                         LineNo,
                         StringNo,
                         clvOPCODE_ADD,
                         IOperand,
                     resultROperand,
                     intermROperand);
       }
       startComponent += 4;
    }

    return gcvSTATUS_OK;
}

gceSTATUS
clGenTestJumpCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gctLABEL Label,
    IN gctBOOL TrueJump,
    IN clsROPERAND * ROperand
    )
{
    gceSTATUS    status;
    gcsSOURCE    source;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_OBJECT(CodeGenerator, clvOBJ_CODE_GENERATOR);
    gcmASSERT(ROperand);

    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                      clvDUMP_CODE_GENERATOR,
                      "<TEST line=\"%d\" string=\"%d\" trueJump=\"%s\">",
                      LineNo,
                      StringNo,
                      TrueJump ? "true" : "false"));

    gcmVERIFY_OK(clsROPERAND_Dump(Compiler, ROperand));

    status = _ConvNormalROperandToSource(Compiler,
                         LineNo,
                         StringNo,
                         ROperand,
                         &source);

    if (gcmIS_ERROR(status)) return status;

    status = clEmitTestBranchCode(Compiler,
                      LineNo,
                      StringNo,
                      clvOPCODE_JUMP,
                      Label,
                      TrueJump,
                      &source);

    if (gcmIS_ERROR(status)) return status;

    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                      clvDUMP_CODE_GENERATOR,
                      "</TEST>"));

    return gcvSTATUS_OK;
}

gceSTATUS
clGenCompareJumpCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gctLABEL Label,
    IN gctBOOL TrueJump,
    IN cleCONDITION CompareCondition,
    IN clsROPERAND * ROperand0,
    IN clsROPERAND * ROperand1
    )
{
    gceSTATUS status;
    gcsSUPER_SOURCE superSource0;
    gcsSUPER_SOURCE superSource1;
    int j;
    gctLABEL endLabel = 0;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_OBJECT(CodeGenerator, clvOBJ_CODE_GENERATOR);
    gcmASSERT(ROperand0);
    gcmASSERT(ROperand1);

    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                  clvDUMP_CODE_GENERATOR,
                  "<CONDITION line=\"%d\" string=\"%d\""
                  " trueJump=\"%s\" compareType=\"%s\">",
                  LineNo,
                  StringNo,
                  TrueJump ? "true" : "false",
                  clGetConditionName(CompareCondition)));

    gcmVERIFY_OK(clsROPERAND_Dump(Compiler, ROperand0));
    gcmVERIFY_OK(clsROPERAND_Dump(Compiler, ROperand1));

    status = _ConvNormalROperandToSuperSource(Compiler,
                                              LineNo,
                                              StringNo,
                                              ROperand0,
                                              &superSource0);
    if (gcmIS_ERROR(status)) return status;

    status = _ConvNormalROperandToSuperSource(Compiler,
                                              LineNo,
                                              StringNo,
                                              ROperand1,
                                              &superSource1);
    if (gcmIS_ERROR(status)) return status;

    _SplitSources(&superSource0, superSource1.numSources);
    _SplitSources(&superSource1, superSource0.numSources);
    gcmASSERT(superSource0.numSources == superSource1.numSources);

    j = 0;
    if(superSource0.numSources > 1) {
        cleCONDITION falseCondition,

        endLabel = clNewLabel(Compiler);
        falseCondition = TrueJump ? clGetNotCondition(CompareCondition) : CompareCondition;
        for(; j < (superSource0.numSources - 1); j++) {
            status = clEmitCompareBranchCode(Compiler,
                                             LineNo,
                                             StringNo,
                                             clvOPCODE_JUMP,
                                             falseCondition,
                                             endLabel,
                                             superSource0.sources + j,
                                             superSource1.sources + j);
             if (gcmIS_ERROR(status)) return status;
        }
    }
    status = clEmitCompareBranchCode(Compiler,
                                     LineNo,
                                     StringNo,
                                     clvOPCODE_JUMP,
                                     TrueJump ?
                                     CompareCondition : clGetNotCondition(CompareCondition),
                                     Label,
                                     superSource0.sources + j,
                                     superSource1.sources + j);
    if (gcmIS_ERROR(status)) return status;
    if(endLabel) {
        status = clSetLabel(Compiler,
                            LineNo,
                            StringNo,
                            endLabel);
        if (gcmIS_ERROR(status)) return status;
    }

    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                                  clvDUMP_CODE_GENERATOR,
                                  "</CONDITION>"));

    return gcvSTATUS_OK;
}

static gceSTATUS
_GenConditionCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_EXPR CondExpr,
    IN gctLABEL Label,
    IN gctBOOL TrueJump
    );

static gceSTATUS
_GenSplitOperandConditionCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cleCONDITION Condition,
    IN clsGEN_CODE_PARAMETERS *CondParameters,
    IN cloIR_EXPR LeftExpr,
    IN cloIR_EXPR RightExpr,
    IN gctLABEL Label,
    IN gctBOOL TrueJump
    );

static gceSTATUS
_ImplicitConvertOperand(
IN cloCOMPILER Compiler,
IN gctUINT LineNo,
IN gctUINT StringNo,
IN clsGEN_CODE_DATA_TYPE NewDataType,
IN OUT clsROPERAND *Operand
)
{
      return clsROPERAND_ChangeDataTypeFamily(Compiler,
                                              LineNo,
                                              StringNo,
                                              gcvFALSE,
                                              NewDataType,
                                              Operand);
}

gceSTATUS
clGenImplicitConversion(
IN cloCOMPILER Compiler,
IN OUT cloIR_EXPR LeftExpr,
IN OUT cloIR_EXPR RightExpr,
IN OUT clsGEN_CODE_PARAMETERS *LeftParameters,
IN OUT clsGEN_CODE_PARAMETERS *RightParameters
)
{
   gceSTATUS status;
   gctUINT i;
   cltELEMENT_TYPE leftElementType, rightElementType;

   gcmASSERT(LeftParameters->operandCount == RightParameters->operandCount);

   for (i = 0; i < LeftParameters->operandCount; i++) {
       if(gcIsMatrixDataType(LeftParameters->dataTypes[i].def) ||
          gcIsMatrixDataType(RightParameters->dataTypes[i].def)) continue;
       leftElementType = clmGEN_CODE_elementType_GET(LeftParameters->dataTypes[i].def);
       rightElementType = clmGEN_CODE_elementType_GET(RightParameters->dataTypes[i].def);

       if(leftElementType == rightElementType) {
          if(gcIsScalarDataType(LeftParameters->dataTypes[i].def) &&
             gcIsScalarDataType(RightParameters->dataTypes[i].def)) continue;

          if(gcIsScalarDataType(RightParameters->dataTypes[i].def)) {
             gcmASSERT(gcIsVectorDataType(LeftParameters->dataTypes[i].def));
             status = _ImplicitConvertOperand(Compiler,
                                              RightExpr->base.lineNo,
                                              RightExpr->base.stringNo,
                                              LeftParameters->dataTypes[i].def,
                                              &RightParameters->rOperands[i]);
             if(gcmIS_ERROR(status)) return status;
             RightParameters->dataTypes[i].def = LeftParameters->dataTypes[i].def;
             RightExpr->decl.dataType = LeftExpr->decl.dataType;
          }
          else if(gcIsScalarDataType(LeftParameters->dataTypes[i].def) &&
                  LeftParameters->needROperand) {
             gcmASSERT(gcIsVectorDataType(RightParameters->dataTypes[i].def));
             status = _ImplicitConvertOperand(Compiler,
                                              LeftExpr->base.lineNo,
                                              LeftExpr->base.stringNo,
                                              RightParameters->dataTypes[i].def,
                                              &LeftParameters->rOperands[i]);
             if(gcmIS_ERROR(status)) return status;
             LeftParameters->dataTypes[i].def = RightParameters->dataTypes[i].def;
             LeftExpr->decl.dataType = RightExpr->decl.dataType;
          }
       }
       else {
          if(leftElementType > rightElementType) { /* convert right */
             gcmASSERT(gcIsScalarDataType(RightParameters->dataTypes[i].def));

             status = _ImplicitConvertOperand(Compiler,
                                              RightExpr->base.lineNo,
                                              RightExpr->base.stringNo,
                                              LeftParameters->dataTypes[i].def,
                                              &RightParameters->rOperands[i]);
             if(gcmIS_ERROR(status)) return status;
             RightParameters->dataTypes[i].def = LeftParameters->dataTypes[i].def;
             RightExpr->decl.dataType = LeftExpr->decl.dataType;
         }
         else if(LeftParameters->needROperand) { /* convert left */
             gcmASSERT(gcIsScalarDataType(LeftParameters->dataTypes[i].def));

             status = _ImplicitConvertOperand(Compiler,
                                              LeftExpr->base.lineNo,
                                              LeftExpr->base.stringNo,
                                              RightParameters->dataTypes[i].def,
                                              &LeftParameters->rOperands[i]);
             if(gcmIS_ERROR(status)) return status;
             LeftParameters->dataTypes[i].def = RightParameters->dataTypes[i].def;
             LeftExpr->decl.dataType = RightExpr->decl.dataType;
         }
      }
   }
   return gcvSTATUS_OK;
}

static gceSTATUS
_GenImplicitConvParametersToType(
IN cloCOMPILER Compiler,
IN OUT cloIR_EXPR FromExpr,
IN OUT cloIR_EXPR ToExpr,
IN OUT clsGEN_CODE_PARAMETERS *FromParameters,
IN OUT clsGEN_CODE_PARAMETERS *ToParameters
)
{
   gceSTATUS status;
   gctUINT i;
   cltELEMENT_TYPE fromElementType, toElementType;

   gcmASSERT(FromParameters->operandCount == ToParameters->operandCount);

   for (i = 0; i < FromParameters->operandCount; i++) {
       fromElementType = clmGEN_CODE_elementType_GET(FromParameters->dataTypes[i].def);
       toElementType = clmGEN_CODE_elementType_GET(ToParameters->dataTypes[i].def);

       if(fromElementType == toElementType) {
          if(gcIsScalarDataType(FromParameters->dataTypes[i].def) &&
             gcIsScalarDataType(ToParameters->dataTypes[i].def)) continue;

          if(gcIsScalarDataType(FromParameters->dataTypes[i].def)) {
             if(gcIsVectorDataType(ToParameters->dataTypes[i].def)) {
                status = clsROPERAND_ChangeDataTypeFamily(Compiler,
                                                          FromExpr->base.lineNo,
                                                          FromExpr->base.stringNo,
                                                          gcvFALSE,
                                                          ToParameters->dataTypes[i].def,
                                                          &FromParameters->rOperands[i]);
                FromParameters->dataTypes[i].def = ToParameters->dataTypes[i].def;
                FromExpr->decl.dataType = ToExpr->decl.dataType;
             }
          }
       }
       else {
          gcmASSERT(gcIsScalarDataType(FromParameters->dataTypes[i].def));

          status = clsROPERAND_ChangeDataTypeFamily(Compiler,
                                                    FromExpr->base.lineNo,
                                                    FromExpr->base.stringNo,
                                                    gcvFALSE,
                                                    ToParameters->dataTypes[i].def,
                                                    &FromParameters->rOperands[i]);
          if(gcmIS_ERROR(status)) return status;
          FromParameters->dataTypes[i].def = ToParameters->dataTypes[i].def;
          FromExpr->decl.dataType = ToExpr->decl.dataType;
       }
   }
   return gcvSTATUS_OK;
}

static gceSTATUS
_GenImplicitConvToType(
IN cloCOMPILER Compiler,
IN clsDECL *ToDecl,
IN OUT cloIR_EXPR FromExpr,
IN OUT clsGEN_CODE_PARAMETERS *FromParameters
)
{
   if(clmDECL_IsGeneralArithmeticType(ToDecl) &&
      clmDECL_IsArithmeticType(&FromExpr->decl)) {
      gceSTATUS status;
      cltELEMENT_TYPE fromElementType, toElementType;

      fromElementType = clmDATA_TYPE_elementType_GET(FromExpr->decl.dataType);
      toElementType = clmDATA_TYPE_elementType_GET(ToDecl->dataType);
      if(clmIsElementTypePacked(toElementType)) {
          toElementType = _ConvPackedTypeToBasicType(toElementType);
      }

      if(((fromElementType == toElementType) &&
          clmDECL_IsScalar(&FromExpr->decl) &&
          clmDECL_IsGeneralVectorType(ToDecl)) ||
         (fromElementType != toElementType &&
          clmDECL_IsScalar(&FromExpr->decl))) {
          clsGEN_CODE_DATA_TYPE format;
          clmGEN_CODE_ConvDirectElementDataType(ToDecl->dataType, format);

          status = clsROPERAND_ChangeDataTypeFamily(Compiler,
                                                    FromExpr->base.lineNo,
                                                    FromExpr->base.stringNo,
                                                    gcvFALSE,
                                                    format,
                                                    &FromParameters->rOperands[0]);
          if(gcmIS_ERROR(status)) return status;

          FromParameters->dataTypes[0].def = format;
          FromExpr->decl.dataType = ToDecl->dataType;
      }
   }
   return gcvSTATUS_OK;
}

static gceSTATUS
_GenConvROperandForAssign(
IN cloCOMPILER Compiler,
IN gctUINT LineNo,
IN gctUINT StringNo,
IN clsGEN_CODE_DATA_TYPE NewDataType,
IN clsROPERAND *ROperand
)
{
   gceSTATUS status;
   cltELEMENT_TYPE fromElementType, toElementType;

   fromElementType = clmGEN_CODE_elementType_GET(ROperand->dataType);
   toElementType = clmGEN_CODE_elementType_GET(NewDataType);

   if(fromElementType == toElementType) {
      if(gcIsScalarDataType(ROperand->dataType) &&
         gcIsScalarDataType(NewDataType)) return gcvSTATUS_OK;

      if(gcIsScalarDataType(ROperand->dataType)) {
         gcmASSERT(gcIsVectorDataType(NewDataType));
         status = clsROPERAND_ChangeDataTypeFamily(Compiler,
                                                   LineNo,
                                                   StringNo,
                                                   gcvFALSE,
                                                   NewDataType,
                                                   ROperand);
         if(gcmIS_ERROR(status)) return status;
      }
   }
   else {
      gcmASSERT(gcIsScalarDataType(ROperand->dataType));

      status = clsROPERAND_ChangeDataTypeFamily(Compiler,
                                                LineNo,
                                                StringNo,
                                                gcvFALSE,
                                                NewDataType,
                                                ROperand);
      if(gcmIS_ERROR(status)) return status;
   }
   return gcvSTATUS_OK;
}

gceSTATUS
cloIR_BINARY_EXPR_GenRelationalConditionCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_BINARY_EXPR BinaryExpr,
    IN gctLABEL Label,
    IN gctBOOL TrueJump
    )
{
    gceSTATUS  status;
    clsGEN_CODE_PARAMETERS    leftParameters, rightParameters;
    cleCONDITION condition;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_OBJECT(CodeGenerator, clvOBJ_CODE_GENERATOR);
    clmVERIFY_IR_OBJECT(BinaryExpr, clvIR_BINARY_EXPR);

    /* Generate the code of the left operand */
    gcmASSERT(BinaryExpr->leftOperand);

    clsGEN_CODE_PARAMETERS_Initialize(
                                    &leftParameters,
                                    gcvFALSE,
                                    gcvTRUE);

    status = cloIR_OBJECT_Accept(
                                Compiler,
                                &BinaryExpr->leftOperand->base,
                                &CodeGenerator->visitor,
                                &leftParameters);

    if (gcmIS_ERROR(status)) return status;

    gcmASSERT(leftParameters.operandCount == 1);

    /* Generate the code of the right operand */
    gcmASSERT(BinaryExpr->rightOperand);

    clsGEN_CODE_PARAMETERS_Initialize(&rightParameters,
                      gcvFALSE,
                      gcvTRUE);

    status = cloIR_OBJECT_Accept(Compiler,
                     &BinaryExpr->rightOperand->base,
                     &CodeGenerator->visitor,
                     &rightParameters);
    if (gcmIS_ERROR(status)) return status;

    gcmASSERT(rightParameters.operandCount == 1);

    status = clGenImplicitConversion(Compiler,
                                         BinaryExpr->leftOperand,
                                         BinaryExpr->rightOperand,
                                         &leftParameters,
                                         &rightParameters);
    if(gcmIS_ERROR(status)) return status;

    /* Generate the condition code */
    switch (BinaryExpr->type)
    {
    case clvBINARY_GREATER_THAN:
        condition = clvCONDITION_GREATER_THAN;
        break;

    case clvBINARY_LESS_THAN:
        condition = clvCONDITION_LESS_THAN;
        break;

    case clvBINARY_GREATER_THAN_EQUAL:
        condition = clvCONDITION_GREATER_THAN_EQUAL;
        break;

    case clvBINARY_LESS_THAN_EQUAL:
        condition = clvCONDITION_LESS_THAN_EQUAL;
        break;

    default:
        gcmASSERT(0);
        return gcvSTATUS_INVALID_ARGUMENT;
    }

    status = clGenCompareJumpCode(
                                Compiler,
                                CodeGenerator,
                                BinaryExpr->exprBase.base.lineNo,
                                BinaryExpr->exprBase.base.stringNo,
                                Label,
                                TrueJump,
                                condition,
                                &leftParameters.rOperands[0],
                                &rightParameters.rOperands[0]);

    if (gcmIS_ERROR(status)) return status;

    clsGEN_CODE_PARAMETERS_Finalize(&leftParameters);
    clsGEN_CODE_PARAMETERS_Finalize(&rightParameters);

    return gcvSTATUS_OK;
}

static gceSTATUS
_GenSplitOperandConditionCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cleCONDITION Condition,
    IN clsGEN_CODE_PARAMETERS *CondParameters,
    IN cloIR_EXPR LeftExpr,
    IN cloIR_EXPR RightExpr,
    IN gctLABEL Label,
    IN gctBOOL TrueJump
    )
{
    gceSTATUS  status;
    clsGEN_CODE_PARAMETERS    rightParameters;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_OBJECT(CodeGenerator, clvOBJ_CODE_GENERATOR);

    /* Generate the code of the right expression */
    gcmASSERT(RightExpr);

    clsGEN_CODE_PARAMETERS_Initialize(&rightParameters,
                      gcvFALSE,
                      gcvTRUE);

    status = cloIR_OBJECT_Accept(Compiler,
                     &RightExpr->base,
                     &CodeGenerator->visitor,
                     &rightParameters);
    if (gcmIS_ERROR(status)) return status;

    gcmASSERT(rightParameters.operandCount == 1);

    status = clGenImplicitConversion(Compiler,
                                         LeftExpr,
                                         RightExpr,
                                         CondParameters,
                                         &rightParameters);
    if (gcmIS_ERROR(status)) return status;

    /* Generate the condition code */
    status = clGenCompareJumpCode(Compiler,
                      CodeGenerator,
                      RightExpr->base.lineNo,
                      RightExpr->base.stringNo,
                      Label,
                      TrueJump,
                      Condition,
                      &CondParameters->rOperands[0],
                      &rightParameters.rOperands[0]);

    if (gcmIS_ERROR(status)) return status;

    clsGEN_CODE_PARAMETERS_Finalize(&rightParameters);

    return gcvSTATUS_OK;
}

static gceSTATUS
_GenMultiplyEqualityConditionCode(
IN cloCOMPILER Compiler,
IN cloCODE_GENERATOR CodeGenerator,
IN gctUINT LineNo,
IN gctUINT StringNo,
IN gctLABEL Label,
IN gctBOOL TrueJump,
IN cleCONDITION CompareCondition,
IN gctUINT OperandCount,
IN clsGEN_CODE_PARAMETER_DATA_TYPE * DataTypes,
IN clsROPERAND * ROperands0,
IN clsROPERAND * ROperands1
)
{
    gceSTATUS        status;
    gctUINT            i, j, k;
    clsROPERAND        rOperand0;
    clsROPERAND        rOperand1;
    gctLABEL        endLabel;
    gctLABEL        targetLabel;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_OBJECT(CodeGenerator, clvOBJ_CODE_GENERATOR);
    gcmASSERT(CompareCondition == clvCONDITION_EQUAL
                || CompareCondition == clvCONDITION_NOT_EQUAL);
    gcmASSERT(OperandCount >= 1);
    gcmASSERT(DataTypes);
    gcmASSERT(ROperands0);
    gcmASSERT(ROperands1);

    if (!TrueJump)    CompareCondition = clGetNotCondition(CompareCondition);

    if (CompareCondition == clvCONDITION_NOT_EQUAL) {
        for (i = 0; i < OperandCount; i++) {
            if (gcIsScalarDataType(DataTypes[i].def)) {
                status = clGenCompareJumpCode(Compiler,
                                  CodeGenerator,
                                  LineNo,
                                  StringNo,
                                  Label,
                                  gcvTRUE,
                                  CompareCondition,
                                  &ROperands0[i],
                                  &ROperands1[i]);
                if (gcmIS_ERROR(status)) return status;
            }
            else if (gcIsVectorDataType(DataTypes[i].def)) {
                    gctUINT limit;

                limit =  gcGetVectorDataTypeComponentCount(DataTypes[i].def);
                for (j = 0; j < limit; j++) {
                    clsROPERAND_InitializeAsVectorComponent(&rOperand0, &ROperands0[i], j);
                    clsROPERAND_InitializeAsVectorComponent(&rOperand1, &ROperands1[i], j);

                    status = clGenCompareJumpCode(Compiler,
                                      CodeGenerator,
                                      LineNo,
                                      StringNo,
                                      Label,
                                      gcvTRUE,
                                      CompareCondition,
                                      &rOperand0,
                                      &rOperand1);
                    if (gcmIS_ERROR(status)) return status;
                }
            }
            else {
                gcmASSERT(gcIsMatrixDataType(DataTypes[i].def));
                for (j = 0; j < gcGetMatrixDataTypeColumnCount(DataTypes[i].def); j++) {
                  for (k = 0; k < gcGetMatrixDataTypeColumnCount(DataTypes[i].def); k++) {
                    clsROPERAND_InitializeAsMatrixComponent(&rOperand0, &ROperands0[i], j, k);
                    clsROPERAND_InitializeAsMatrixComponent(&rOperand1, &ROperands1[i], j, k);
                    status = clGenCompareJumpCode(Compiler,
                                      CodeGenerator,
                                      LineNo,
                                      StringNo,
                                      Label,
                                      gcvTRUE,
                                      CompareCondition,
                                      &rOperand0,
                                      &rOperand1);
                    if (gcmIS_ERROR(status)) return status;
                  }
                }
            }
        }    /* for (i = 0; i < OperandCount; i++) */
    }
    else {
        endLabel = clNewLabel(Compiler);
        for (i = 0; i < OperandCount; i++) {
            if (gcIsScalarDataType(DataTypes[i].def)) {
                if (i == OperandCount - 1) {
                    targetLabel    = Label;
                    TrueJump    = gcvTRUE;
                }
                else {
                    targetLabel    = endLabel;
                    TrueJump    = gcvFALSE;
                }

                status = clGenCompareJumpCode(Compiler,
                                  CodeGenerator,
                                  LineNo,
                                  StringNo,
                                  targetLabel,
                                  TrueJump,
                                  CompareCondition,
                                  &ROperands0[i],
                                  &ROperands1[i]);
                if (gcmIS_ERROR(status)) return status;
            }
            else if (gcIsVectorDataType(DataTypes[i].def)) {
                    gctUINT limit;

                limit =  gcGetVectorDataTypeComponentCount(DataTypes[i].def);
                for (j = 0; j < limit; j++) {
                    if (i == OperandCount - 1
                        && j ==  limit - 1) {
                        targetLabel    = Label;
                        TrueJump          = gcvTRUE;
                    }
                    else {
                        targetLabel    = endLabel;
                        TrueJump    = gcvFALSE;
                    }

                    clsROPERAND_InitializeAsVectorComponent(&rOperand0, &ROperands0[i], j);
                    clsROPERAND_InitializeAsVectorComponent(&rOperand1, &ROperands1[i], j);

                    status = clGenCompareJumpCode(Compiler,
                                      CodeGenerator,
                                      LineNo,
                                      StringNo,
                                      targetLabel,
                                      TrueJump,
                                      CompareCondition,
                                      &rOperand0,
                                      &rOperand1);
                    if (gcmIS_ERROR(status)) return status;
                }
            }
            else {
                gcmASSERT(gcIsMatrixDataType(DataTypes[i].def));

                for (j = 0; j < gcGetMatrixDataTypeColumnCount(DataTypes[i].def); j++) {
                    for (k = 0; k < gcGetMatrixDataTypeColumnCount(DataTypes[i].def); k++) {
                      if (i == OperandCount - 1
                          && j == gcGetMatrixDataTypeColumnCount(DataTypes[i].def) - 1
                          && k == gcGetMatrixDataTypeColumnCount(DataTypes[i].def) - 1) {
                        targetLabel = Label;
                        TrueJump = gcvTRUE;
                        }
                      else {
                        targetLabel = endLabel;
                        TrueJump    = gcvFALSE;
                      }

                      clsROPERAND_InitializeAsMatrixComponent(&rOperand0, &ROperands0[i], j, k);
                      clsROPERAND_InitializeAsMatrixComponent(&rOperand1, &ROperands1[i], j, k);

                      status = clGenCompareJumpCode(Compiler,
                                    CodeGenerator,
                                    LineNo,
                                    StringNo,
                                    targetLabel,
                                    TrueJump,
                                    CompareCondition,
                                    &rOperand0,
                                    &rOperand1);
                      if (gcmIS_ERROR(status)) return status;
                    }
                }
            }
        }    /* for (i = 0; i < OperandCount; i++) */

        /* end: */
        status = clSetLabel(Compiler,
                    LineNo,
                    StringNo,
                    endLabel);
        if (gcmIS_ERROR(status)) return status;
    }
    return gcvSTATUS_OK;
}

gceSTATUS
cloIR_BINARY_EXPR_GenEqualityConditionCode(
IN cloCOMPILER Compiler,
IN cloCODE_GENERATOR CodeGenerator,
IN cloIR_BINARY_EXPR BinaryExpr,
IN gctLABEL Label,
IN gctBOOL TrueJump
)
{
    gceSTATUS  status;
    clsGEN_CODE_PARAMETERS    leftParameters, rightParameters;
    cleCONDITION    condition;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_OBJECT(CodeGenerator, clvOBJ_CODE_GENERATOR);
    clmVERIFY_IR_OBJECT(BinaryExpr, clvIR_BINARY_EXPR);

    /* Generate the code of the left operand */
    gcmASSERT(BinaryExpr->leftOperand);

    clsGEN_CODE_PARAMETERS_Initialize(&leftParameters,
                      gcvFALSE,
                      gcvTRUE);

    status = cloIR_OBJECT_Accept(Compiler,
                     &BinaryExpr->leftOperand->base,
                     &CodeGenerator->visitor,
                     &leftParameters);
    if (gcmIS_ERROR(status)) return status;

    /* Generate the code of the right operand */
    gcmASSERT(BinaryExpr->rightOperand);

    clsGEN_CODE_PARAMETERS_Initialize(&rightParameters,
                      gcvFALSE,
                      gcvTRUE);

    status = cloIR_OBJECT_Accept(Compiler,
                     &BinaryExpr->rightOperand->base,
                     &CodeGenerator->visitor,
                     &rightParameters);
    if (gcmIS_ERROR(status)) return status;

    status = clGenImplicitConversion(Compiler,
                                         BinaryExpr->leftOperand,
                                         BinaryExpr->rightOperand,
                                         &leftParameters,
                                         &rightParameters);
    if (gcmIS_ERROR(status)) return status;

    /* Get the condition code */
    switch (BinaryExpr->type) {
    case clvBINARY_EQUAL:
        condition = clvCONDITION_EQUAL;
        break;

    case clvBINARY_NOT_EQUAL:
    case clvBINARY_XOR:
        condition = clvCONDITION_NOT_EQUAL;
        break;

    default:
        gcmASSERT(0);
        return gcvSTATUS_INVALID_ARGUMENT;
    }

    if (clmDECL_IsScalar(&BinaryExpr->leftOperand->decl)) {
        gcmASSERT(leftParameters.operandCount == 1);
        gcmASSERT(rightParameters.operandCount == 1);

        status = clGenCompareJumpCode(Compiler,
                          CodeGenerator,
                          BinaryExpr->exprBase.base.lineNo,
                          BinaryExpr->exprBase.base.stringNo,
                          Label,
                          TrueJump,
                          condition,
                          &leftParameters.rOperands[0],
                          &rightParameters.rOperands[0]);
        if (gcmIS_ERROR(status)) return status;
    }
    else {
        gcmASSERT(leftParameters.operandCount == rightParameters.operandCount);

        status = _GenMultiplyEqualityConditionCode(Compiler,
                               CodeGenerator,
                               BinaryExpr->exprBase.base.lineNo,
                               BinaryExpr->exprBase.base.stringNo,
                               Label,
                               TrueJump,
                               condition,
                               leftParameters.operandCount,
                               leftParameters.dataTypes,
                               leftParameters.rOperands,
                               rightParameters.rOperands);
        if (gcmIS_ERROR(status)) return status;
    }

    clsGEN_CODE_PARAMETERS_Finalize(&leftParameters);
    clsGEN_CODE_PARAMETERS_Finalize(&rightParameters);
    return gcvSTATUS_OK;
}

gceSTATUS
cloIR_BINARY_EXPR_GenAndConditionCode(
IN cloCOMPILER Compiler,
IN cloCODE_GENERATOR CodeGenerator,
IN cloIR_BINARY_EXPR BinaryExpr,
IN gctLABEL Label,
IN gctBOOL TrueJump
)
{
    gceSTATUS    status;
    gctLABEL    endLabel;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_OBJECT(CodeGenerator, clvOBJ_CODE_GENERATOR);
    clmVERIFY_IR_OBJECT(BinaryExpr, clvIR_BINARY_EXPR);

    if (TrueJump) {
        endLabel = clNewLabel(Compiler);

        /* jump end if !(left) */
        gcmASSERT(BinaryExpr->leftOperand);

        status = _GenConditionCode(Compiler,
                       CodeGenerator,
                       BinaryExpr->leftOperand,
                       endLabel,
                       gcvFALSE);
        if (gcmIS_ERROR(status)) return status;

        /* jump label if (right) */
        gcmASSERT(BinaryExpr->rightOperand);

        status = _GenConditionCode(Compiler,
                       CodeGenerator,
                       BinaryExpr->rightOperand,
                       Label,
                       gcvTRUE);
        if (gcmIS_ERROR(status)) return status;

        /* end: */
        status = clSetLabel(Compiler,
                    BinaryExpr->exprBase.base.lineNo,
                    BinaryExpr->exprBase.base.stringNo,
                    endLabel);
        if (gcmIS_ERROR(status)) return status;
    }
    else {
        /* jump label if !(left) */
        gcmASSERT(BinaryExpr->leftOperand);

        status = _GenConditionCode(Compiler,
                       CodeGenerator,
                       BinaryExpr->leftOperand,
                       Label,
                       gcvFALSE);
        if (gcmIS_ERROR(status)) return status;

        /* jump label if !(right) */
        gcmASSERT(BinaryExpr->rightOperand);

        status = _GenConditionCode(Compiler,
                       CodeGenerator,
                       BinaryExpr->rightOperand,
                       Label,
                       gcvFALSE);
        if (gcmIS_ERROR(status)) return status;
    }
    return gcvSTATUS_OK;
}

gceSTATUS
cloIR_BINARY_EXPR_GenOrConditionCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_BINARY_EXPR BinaryExpr,
    IN gctLABEL Label,
    IN gctBOOL TrueJump
    )
{
    gceSTATUS    status;
    gctLABEL    endLabel;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_OBJECT(CodeGenerator, clvOBJ_CODE_GENERATOR);
    clmVERIFY_IR_OBJECT(BinaryExpr, clvIR_BINARY_EXPR);

    if (TrueJump)
    {
        /* jump label if (left) */
        gcmASSERT(BinaryExpr->leftOperand);

        status = _GenConditionCode(
                                    Compiler,
                                    CodeGenerator,
                                    BinaryExpr->leftOperand,
                                    Label,
                                    gcvTRUE);

        if (gcmIS_ERROR(status)) return status;

        /* jump label if (right) */
        gcmASSERT(BinaryExpr->rightOperand);

        status = _GenConditionCode(
                                    Compiler,
                                    CodeGenerator,
                                    BinaryExpr->rightOperand,
                                    Label,
                                    gcvTRUE);

        if (gcmIS_ERROR(status)) return status;
    }
    else {
        endLabel = clNewLabel(Compiler);

        /* jump end if (left) */
        gcmASSERT(BinaryExpr->leftOperand);

        status = _GenConditionCode(
                                    Compiler,
                                    CodeGenerator,
                                    BinaryExpr->leftOperand,
                                    endLabel,
                                    gcvTRUE);

        if (gcmIS_ERROR(status)) return status;

        /* jump label if !(right) */
        gcmASSERT(BinaryExpr->rightOperand);

        status = _GenConditionCode(
                                    Compiler,
                                    CodeGenerator,
                                    BinaryExpr->rightOperand,
                                    Label,
                                    gcvFALSE);

        if (gcmIS_ERROR(status)) return status;

        /* end: */
        status = clSetLabel(
                            Compiler,
                            BinaryExpr->exprBase.base.lineNo,
                            BinaryExpr->exprBase.base.stringNo,
                            endLabel);

        if (gcmIS_ERROR(status)) return status;
    }

    return gcvSTATUS_OK;
}

static gceSTATUS
_GenConditionCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_EXPR CondExpr,
    IN gctLABEL Label,
    IN gctBOOL TrueJump
    )
{
    gceSTATUS        status;
    cloIR_BINARY_EXPR    binaryExpr;
    cloIR_UNARY_EXPR    unaryExpr;
    clsGEN_CODE_PARAMETERS    condParameters;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_OBJECT(CodeGenerator, clvOBJ_CODE_GENERATOR);
    gcmASSERT(CondExpr);

    switch (cloIR_OBJECT_GetType(&CondExpr->base))
    {
    case clvIR_BINARY_EXPR:
        binaryExpr = (cloIR_BINARY_EXPR)CondExpr;

        switch (binaryExpr->type)
        {
        case clvBINARY_GREATER_THAN:
        case clvBINARY_LESS_THAN:
        case clvBINARY_GREATER_THAN_EQUAL:
        case clvBINARY_LESS_THAN_EQUAL:
            return cloIR_BINARY_EXPR_GenRelationalConditionCode(Compiler,
                                        CodeGenerator,
                                        binaryExpr,
                                        Label,
                                        TrueJump);

        case clvBINARY_EQUAL:
        case clvBINARY_NOT_EQUAL:
        case clvBINARY_XOR:
            return cloIR_BINARY_EXPR_GenEqualityConditionCode(Compiler,
                                      CodeGenerator,
                                      binaryExpr,
                                      Label,
                                      TrueJump);

        case clvBINARY_AND:
            return cloIR_BINARY_EXPR_GenAndConditionCode(Compiler,
                                     CodeGenerator,
                                     binaryExpr,
                                     Label,
                                     TrueJump);

        case clvBINARY_OR:
            return cloIR_BINARY_EXPR_GenOrConditionCode(Compiler,
                                    CodeGenerator,
                                    binaryExpr,
                                    Label,
                                    TrueJump);

        default: break;
        }
        break;

    case clvIR_UNARY_EXPR:
        unaryExpr = (cloIR_UNARY_EXPR)CondExpr;
        gcmASSERT(unaryExpr->operand);

        switch (unaryExpr->type)
        {
        case clvUNARY_NOT:
            return _GenConditionCode(Compiler,
                         CodeGenerator,
                         unaryExpr->operand,
                         Label,
                         !TrueJump);

        default: break;
        }
        break;

    default: break;
    }

    clsGEN_CODE_PARAMETERS_Initialize(&condParameters, gcvFALSE, gcvTRUE);

    status = cloIR_OBJECT_Accept(
                                Compiler,
                                &CondExpr->base,
                                &CodeGenerator->visitor,
                                &condParameters);

    if (gcmIS_ERROR(status)) return status;

    status = clGenTestJumpCode(
                                Compiler,
                                CodeGenerator,
                                CondExpr->base.lineNo,
                                CondExpr->base.stringNo,
                                Label,
                                TrueJump,
                                &condParameters.rOperands[0]);

    if (gcmIS_ERROR(status)) return status;

    clsGEN_CODE_PARAMETERS_Finalize(&condParameters);

    return gcvSTATUS_OK;
}

gceSTATUS
clDefineSelectionBegin(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN gctBOOL HasFalseOperand,
    OUT clsSELECTION_CONTEXT * SelectionContext
    )
{
    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_OBJECT(CodeGenerator, clvOBJ_CODE_GENERATOR);
    gcmASSERT(SelectionContext);

    gcmVERIFY_OK(cloCOMPILER_Dump(
                            Compiler,
                            clvDUMP_CODE_GENERATOR,
                            "<SELECTION hasFalseOperand=\"%s\">",
                            HasFalseOperand ? "true" : "false"));

    SelectionContext->hasFalseOperand        = HasFalseOperand;
    SelectionContext->endLabel            = clNewLabel(Compiler);
    SelectionContext->isNegativeCond        = gcvFALSE;

    if (HasFalseOperand)
    {
        SelectionContext->beginLabelOfFalseOperand    = clNewLabel(Compiler);
    }

    return gcvSTATUS_OK;
}

gceSTATUS
clDefineSelectionEnd(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN clsSELECTION_CONTEXT * SelectionContext
    )
{
    gceSTATUS        status;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_OBJECT(CodeGenerator, clvOBJ_CODE_GENERATOR);
    gcmASSERT(SelectionContext);

    status = clSetLabel(
                        Compiler,
                        0,
                        0,
                        SelectionContext->endLabel);

    if (gcmIS_ERROR(status)) return status;

    gcmVERIFY_OK(cloCOMPILER_Dump(
                            Compiler,
                            clvDUMP_CODE_GENERATOR,
                            "</SELECTION>"));

    return gcvSTATUS_OK;
}

gctLABEL
clGetSelectionConditionLabel(
    IN clsSELECTION_CONTEXT * SelectionContext
    )
{
    gcmASSERT(SelectionContext);

    return (SelectionContext->hasFalseOperand) ?
                SelectionContext->beginLabelOfFalseOperand : SelectionContext->endLabel;
}

gceSTATUS
clGenSelectionTestConditionCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN clsSELECTION_CONTEXT * SelectionContext,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN clsROPERAND * ROperand
    )
{
    gceSTATUS    status;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_OBJECT(CodeGenerator, clvOBJ_CODE_GENERATOR);
    gcmASSERT(SelectionContext);
    gcmASSERT(ROperand);

    status = clGenTestJumpCode(
                            Compiler,
                            CodeGenerator,
                            LineNo,
                            StringNo,
                            clGetSelectionConditionLabel(SelectionContext),
                            gcvFALSE,
                            ROperand);

    if (gcmIS_ERROR(status)) return status;

    return gcvSTATUS_OK;
}

gceSTATUS
clGenSelectionCompareConditionCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN clsSELECTION_CONTEXT * SelectionContext,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN cleCONDITION CompareCondition,
    IN clsROPERAND * ROperand0,
    IN clsROPERAND * ROperand1
    )
{
    gceSTATUS    status;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_OBJECT(CodeGenerator, clvOBJ_CODE_GENERATOR);
    gcmASSERT(SelectionContext);
    gcmASSERT(ROperand0);
    gcmASSERT(ROperand1);

    status = clGenCompareJumpCode(
                                Compiler,
                                CodeGenerator,
                                LineNo,
                                StringNo,
                                clGetSelectionConditionLabel(SelectionContext),
                                gcvFALSE,
                                CompareCondition,
                                ROperand0,
                                ROperand1);

    if (gcmIS_ERROR(status)) return status;

    return gcvSTATUS_OK;
}

gceSTATUS
clDefineSelectionTrueOperandBegin(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN clsSELECTION_CONTEXT * SelectionContext
    )
{
    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_OBJECT(CodeGenerator, clvOBJ_CODE_GENERATOR);
    gcmASSERT(SelectionContext);

    gcmVERIFY_OK(cloCOMPILER_Dump(
                            Compiler,
                            clvDUMP_CODE_GENERATOR,
                            "<TRUE_OPERAND>"));

    return gcvSTATUS_OK;
}

gceSTATUS
clDefineSelectionTrueOperandEnd(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN clsSELECTION_CONTEXT * SelectionContext,
    IN gctBOOL HasReturn
    )
{
    gceSTATUS    status;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_OBJECT(CodeGenerator, clvOBJ_CODE_GENERATOR);
    gcmASSERT(SelectionContext);

    if (SelectionContext->hasFalseOperand && !HasReturn)
    {
        status = clEmitAlwaysBranchCode(Compiler,
                        0,
                        0,
                        clvOPCODE_JUMP,
                        SelectionContext->endLabel);

        if (gcmIS_ERROR(status)) return status;
    }

    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                      clvDUMP_CODE_GENERATOR,
                      "</TRUE_OPERAND>"));

    return gcvSTATUS_OK;
}

gceSTATUS
clDefineSelectionFalseOperandBegin(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN clsSELECTION_CONTEXT * SelectionContext
    )
{
    gceSTATUS    status;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_OBJECT(CodeGenerator, clvOBJ_CODE_GENERATOR);
    gcmASSERT(SelectionContext);
    gcmASSERT(SelectionContext->hasFalseOperand);

    gcmVERIFY_OK(cloCOMPILER_Dump(
                            Compiler,
                            clvDUMP_CODE_GENERATOR,
                            "<FALSE_OPERAND>"));

    status = clSetLabel(
                        Compiler,
                        0,
                        0,
                        SelectionContext->beginLabelOfFalseOperand);

    if (gcmIS_ERROR(status)) return status;

    return gcvSTATUS_OK;
}

gceSTATUS
clDefineSelectionFalseOperandEnd(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN clsSELECTION_CONTEXT * SelectionContext
    )
{
    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_OBJECT(CodeGenerator, clvOBJ_CODE_GENERATOR);
    gcmASSERT(SelectionContext);
    gcmASSERT(SelectionContext->hasFalseOperand);

    gcmVERIFY_OK(cloCOMPILER_Dump(
                            Compiler,
                            clvDUMP_CODE_GENERATOR,
                            "</FALSE_OPERAND>"));

    return gcvSTATUS_OK;
}

static gceSTATUS
_GenMatrixMulVectorCode(
    IN cloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN clsIOPERAND * IOperand,
    IN clsROPERAND * ROperand0,
    IN clsROPERAND * ROperand1
    )
{
    gceSTATUS    status;
    clsIOPERAND    intermIOperands[3];
    clsROPERAND    resROperand[1];
    clsROPERAND    rOperand0[1];
    clsROPERAND    rOperand1[1];
    clsROPERAND    rOperand2[1];
    gctUINT    matrixColumnCount, i;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    gcmASSERT(IOperand);
    gcmASSERT(ROperand0);
    gcmASSERT(ROperand1);

    /* mul t0, m[0], v.x */
    clsIOPERAND_New(Compiler, &intermIOperands[0], ROperand1->dataType);
    clsROPERAND_InitializeAsMatrixColumn(rOperand0, ROperand0, 0);
    clsROPERAND_InitializeAsVectorComponent(rOperand1, ROperand1, 0);

    status = clGenArithmeticExprCode(Compiler,
                    LineNo,
                    StringNo,
                    clvOPCODE_MUL,
                    &intermIOperands[0],
                    rOperand0,
                    rOperand1);
    if (gcmIS_ERROR(status)) return status;

    /* mul t1, m[1], v.y */
    clsIOPERAND_New(Compiler, &intermIOperands[1], ROperand1->dataType);
    clsROPERAND_InitializeAsMatrixColumn(rOperand0, ROperand0, 1);
    clsROPERAND_InitializeAsVectorComponent(rOperand1, ROperand1, 1);

    status = clGenArithmeticExprCode(Compiler,
                    LineNo,
                    StringNo,
                    clvOPCODE_MUL,
                    &intermIOperands[1],
                    rOperand0,
                    rOperand1);
    if (gcmIS_ERROR(status)) return status;

    /* add result, t0, t1 */
    clsROPERAND_InitializeUsingIOperand(rOperand0, &intermIOperands[0]);
    clsROPERAND_InitializeUsingIOperand(rOperand1, &intermIOperands[1]);

    status = clGenArithmeticExprCode(Compiler,
                     LineNo,
                     StringNo,
                     clvOPCODE_ADD,
                     IOperand,
                     rOperand0,
                     rOperand1);
    if (gcmIS_ERROR(status)) return status;

    clsROPERAND_InitializeUsingIOperand(resROperand, IOperand);
        matrixColumnCount = gcGetMatrixDataTypeColumnCount(ROperand0->dataType);
    gcmASSERT(matrixColumnCount ==
              gcGetVectorDataTypeComponentCount(ROperand1->dataType));
    for(i = 2; i < matrixColumnCount; i++) {
        /* mul t2, m[i], v.i */
        clsIOPERAND_New(Compiler, &intermIOperands[2], ROperand1->dataType);
        clsROPERAND_InitializeAsMatrixColumn(rOperand0, ROperand0, i);
        clsROPERAND_InitializeAsVectorComponent(rOperand1, ROperand1, i);

        status = clGenArithmeticExprCode(Compiler,
                        LineNo,
                        StringNo,
                        clvOPCODE_MUL,
                        &intermIOperands[2],
                        rOperand0,
                        rOperand1);
        if (gcmIS_ERROR(status)) return status;

        /* add result, I(new), I(old), t2 */
        clsROPERAND_InitializeUsingIOperand(rOperand2, &intermIOperands[2]);

        status = clGenArithmeticExprCode(Compiler,
                         LineNo,
                         StringNo,
                         clvOPCODE_ADD,
                         IOperand,
                         resROperand,
                         rOperand2);
        if (gcmIS_ERROR(status)) return status;
    }

    return gcvSTATUS_OK;
}

#define MAX_NAME_TABLE_SIZE            10
#define INVALID_VECTOR_INDEX        (0xffffffff)
#define MAX_LEVEL                    (0xffffffff)

typedef struct _clsUSING_SINGLE_VECTOR_INDEX_PARAMETERS
{
    gctBOOL                usingSingleVectorIndex;

    gctUINT                vectorIndex;

    gctUINT                currentLevel;

    gctUINT                maxLevel;

    gctUINT                inputNameCount;

    clsNAME *            inputNameTable[MAX_NAME_TABLE_SIZE];

    gctUINT                outputNameCount;

    clsNAME *            outputNameTable[MAX_NAME_TABLE_SIZE];

    gctUINT                outputNameLevelTable[MAX_NAME_TABLE_SIZE];
}
clsUSING_SINGLE_VECTOR_INDEX_PARAMETERS;

#define clsUSING_SINGLE_VECTOR_INDEX_PARAMETERS_Initialize(parameters) \
    do \
    { \
        (parameters)->usingSingleVectorIndex    = gcvTRUE; \
        (parameters)->vectorIndex                = INVALID_VECTOR_INDEX; \
        (parameters)->currentLevel                = 0; \
        (parameters)->maxLevel                    = 0; \
        (parameters)->inputNameCount            = 0; \
        (parameters)->outputNameCount            = 0; \
    } \
    while (gcvFALSE)

static gctBOOL
_IsNameListEqual(
    IN gctUINT NameCount0,
    IN clsNAME * NameTable0[MAX_NAME_TABLE_SIZE],
    IN gctUINT NameCount1,
    IN clsNAME * NameTable1[MAX_NAME_TABLE_SIZE]
    )
{
    gctUINT        i;

    /* Verify the arguments. */
    gcmASSERT(NameTable0);
    gcmASSERT(NameTable1);

    if (NameCount0 != NameCount1)    return gcvFALSE;

    for (i = 0; i < NameCount0; i++)
    {
        if (NameTable0[i] != NameTable1[i])    return gcvFALSE;
    }

    return gcvTRUE;
}

static gctUINT
_FindNameInList(
    IN clsNAME * Name,
    IN gctUINT NameCount,
    IN clsNAME * NameTable[MAX_NAME_TABLE_SIZE]
    )
{
    gctUINT        i;

    /* Verify the arguments. */
    gcmASSERT(Name);
    gcmASSERT(NameTable);

    for (i = 0; i < NameCount; i++)
    {
        if (NameTable[i] == Name) return i;
    }

    return MAX_NAME_TABLE_SIZE;
}

static gceSTATUS
_AddNameToList(
    IN clsNAME * Name,
    IN gctUINT Level,
    IN OUT gctUINT * NameCount,
    IN OUT clsNAME * NameTable[MAX_NAME_TABLE_SIZE],
    IN OUT gctUINT NameLevelTable[MAX_NAME_TABLE_SIZE]
    )
{
    gctUINT        i;

    /* Verify the arguments. */
    gcmASSERT(Name);
    gcmASSERT(NameCount);
    gcmASSERT(NameTable);

    for (i = 0; i < *NameCount; i++)
    {
        if (Name == NameTable[i])
        {
            if (NameLevelTable != gcvNULL && Level < NameLevelTable[i])
            {
                NameLevelTable[i] = Level;
            }

            return gcvSTATUS_OK;
        }
    }

    if (*NameCount == MAX_NAME_TABLE_SIZE) return gcvSTATUS_BUFFER_TOO_SMALL;

    NameTable[*NameCount]    = Name;

    if (NameLevelTable != gcvNULL) NameLevelTable[*NameCount] = Level;

    (*NameCount)++;

    return gcvSTATUS_OK;
}

static gceSTATUS
_AddNameToParameters(
    IN cloCOMPILER Compiler,
    IN clsNAME * Name,
    IN gctUINT VectorIndex,
    IN gctBOOL NeedLValue,
    IN gctBOOL NeedRValue,
    IN OUT clsUSING_SINGLE_VECTOR_INDEX_PARAMETERS * Parameters
    )
{
    gceSTATUS    status;
    gctUINT        nameIndex;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    gcmASSERT(Name);
    gcmASSERT(Parameters);

    do
    {
        if (Parameters->vectorIndex == INVALID_VECTOR_INDEX)
        {
            Parameters->vectorIndex = VectorIndex;
        }
        else
        {
            if (Parameters->vectorIndex != VectorIndex) break;
        }

        if (NeedRValue)
        {
            nameIndex = _FindNameInList(
                                        Name,
                                        Parameters->outputNameCount,
                                        Parameters->outputNameTable);

            if (nameIndex == MAX_NAME_TABLE_SIZE
                || Parameters->currentLevel < Parameters->outputNameLevelTable[nameIndex])
            {
                status = _AddNameToList(
                                        Name,
                                        Parameters->currentLevel,
                                        &Parameters->inputNameCount,
                                        Parameters->inputNameTable,
                                        gcvNULL);

                if (gcmIS_ERROR(status)) break;
            }
        }

        if (NeedLValue)
        {
            status = _AddNameToList(
                                    Name,
                                    Parameters->currentLevel,
                                    &Parameters->outputNameCount,
                                    Parameters->outputNameTable,
                                    Parameters->outputNameLevelTable);

            if (gcmIS_ERROR(status)) break;
        }

        Parameters->usingSingleVectorIndex = gcvTRUE;

        return gcvSTATUS_OK;
    }
    while (gcvFALSE);

    Parameters->usingSingleVectorIndex = gcvFALSE;

    return gcvSTATUS_OK;
}

gceSTATUS
clGenAssignCode(
    IN cloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN clsLOPERAND * LOperand,
    IN clsROPERAND * ROperand
    )
{
    gceSTATUS  status;
    clsROPERAND  newROperand;
    clsIOPERAND  intermIOperand;
    clsLOPERAND  intermLOperand;
    clsROPERAND  intermROperand;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    gcmASSERT(LOperand);
    gcmASSERT(LOperand->vectorIndex.mode != clvINDEX_REG);
    gcmASSERT(ROperand);
    gcmASSERT(ROperand->vectorIndex.mode != clvINDEX_REG);

    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                 clvDUMP_CODE_GENERATOR,
                 "<OPERATION line=\"%d\" string=\"%d\" type=\"assign\">",
                 LineNo,
                 StringNo));

    gcmVERIFY_OK(clsLOPERAND_Dump(Compiler, LOperand));

    gcmVERIFY_OK(clsROPERAND_Dump(Compiler, ROperand));

    gcmASSERT(ROperand->isReg || (ROperand->arrayIndex.mode != clvINDEX_REG));

    if (!ROperand->isReg && (ROperand->matrixIndex.mode == clvINDEX_REG))
    {
        newROperand = *ROperand;
        newROperand.matrixIndex.mode = clvINDEX_NONE;

        clsIOPERAND_New(Compiler, &intermIOperand, ROperand->dataType);
        clsLOPERAND_InitializeUsingIOperand(&intermLOperand, &intermIOperand);

        status = _SpecialGenAssignCode(Compiler,
                                       LineNo,
                                       StringNo,
                                       &intermLOperand,
                                       &newROperand);

        if (gcmIS_ERROR(status)) return status;

        clsROPERAND_InitializeUsingIOperand(&intermROperand, &intermIOperand);
        intermROperand.matrixIndex = ROperand->matrixIndex;

        status = _SpecialGenAssignCode(Compiler,
                                       LineNo,
                                       StringNo,
                                       LOperand,
                                       &intermROperand);

        if (gcmIS_ERROR(status)) return status;
    }
    else
    {
        status = _SpecialGenAssignCode(Compiler,
                                       LineNo,
                                       StringNo,
                                       LOperand,
                                       ROperand);

        if (gcmIS_ERROR(status)) return status;
    }

    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                 clvDUMP_CODE_GENERATOR,
                 "</OPERATION>"));

    return gcvSTATUS_OK;
}

gceSTATUS
cloIR_BASE_UsingSingleVectorIndex(
IN cloCOMPILER Compiler,
IN cloIR_BASE Base,
IN gctBOOL NeedLValue,
IN gctBOOL NeedRValue,
IN OUT clsUSING_SINGLE_VECTOR_INDEX_PARAMETERS * Parameters
)
{
    gceSTATUS        status;
    cloIR_SET        set        = gcvNULL;
    cloIR_VARIABLE        variable    = gcvNULL;
    cloIR_CONSTANT        constant    = gcvNULL;
    cloIR_UNARY_EXPR    unaryExpr    = gcvNULL;
    cloIR_BINARY_EXPR    binaryExpr    = gcvNULL;
    cloIR_SELECTION        selection    = gcvNULL;
    cloIR_POLYNARY_EXPR    polynaryExpr    = gcvNULL;
    cloIR_BASE        member;
    gctBOOL            needLValue0 = gcvFALSE;
    gctBOOL            needLValue1 = gcvFALSE;
    gctBOOL            needRValue0 = gcvFALSE;
    gctBOOL            needRValue1 = gcvFALSE;
    clsNAME *        name;
    gctUINT            i, vectorIndex;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    gcmASSERT(Base);
    gcmASSERT(Parameters);

    switch (cloIR_OBJECT_GetType(Base)) {
    case clvIR_SET:
        set = (cloIR_SET)Base;

        FOR_EACH_DLINK_NODE(&set->members, struct _cloIR_BASE, member) {
            /* Check all members */
            status = cloIR_BASE_UsingSingleVectorIndex(Compiler,
                                   member,
                                   gcvFALSE,
                                   NeedRValue,
                                   Parameters);
            if (gcmIS_ERROR(status)) return status;

            if (!Parameters->usingSingleVectorIndex) break;
        }
        return gcvSTATUS_OK;

    case clvIR_VARIABLE:
        variable = (cloIR_VARIABLE)Base;

        Parameters->usingSingleVectorIndex =
                    clmDECL_IsScalar(&variable->exprBase.decl);
        return gcvSTATUS_OK;

    case clvIR_CONSTANT:
        constant = (cloIR_CONSTANT)Base;

        Parameters->usingSingleVectorIndex =
                    clmDECL_IsScalar(&constant->exprBase.decl);
        return gcvSTATUS_OK;

    case clvIR_UNARY_EXPR:
        unaryExpr = (cloIR_UNARY_EXPR)Base;
        gcmASSERT(unaryExpr->operand);

        switch (unaryExpr->type) {
        case clvUNARY_FIELD_SELECTION:
            Parameters->usingSingleVectorIndex =
                    clmDECL_IsScalar(&unaryExpr->exprBase.decl);
            return gcvSTATUS_OK;

        case clvUNARY_COMPONENT_SELECTION:
            if (!clmDECL_IsScalar(&unaryExpr->exprBase.decl)
                || cloIR_OBJECT_GetType(&unaryExpr->operand->base) != clvIR_VARIABLE) {
                Parameters->usingSingleVectorIndex = gcvFALSE;
                return gcvSTATUS_OK;
            }

            name = ((cloIR_VARIABLE)unaryExpr->operand)->name;
            gcmASSERT(name);

            vectorIndex = unaryExpr->u.componentSelection.selection[clvCOMPONENT_X];
            return _AddNameToParameters(Compiler,
                            name,
                            vectorIndex,
                            NeedLValue,
                            NeedRValue,
                            Parameters);

        case clvUNARY_POST_INC:
        case clvUNARY_POST_DEC:
        case clvUNARY_PRE_INC:
        case clvUNARY_PRE_DEC:
            needLValue0 = gcvTRUE;
            needRValue0 = gcvTRUE;
            break;

        case clvUNARY_NEG:
        case clvUNARY_NOT:
        case clvUNARY_NON_LVAL:
        case clvUNARY_CAST:
        case clvUNARY_ADDR:
        case clvUNARY_BITWISE_NOT:
            needLValue0 = gcvFALSE;
            needRValue0 = NeedRValue;
            break;

        case clvUNARY_INDIRECTION:
            needLValue0 = NeedLValue;
            needRValue0 = NeedRValue;
            break;

        default: gcmASSERT(0);
        }

        /* Check the operand */
        status = cloIR_BASE_UsingSingleVectorIndex(Compiler,
                               &unaryExpr->operand->base,
                               needLValue0,
                               needRValue0,
                               Parameters);
        if (gcmIS_ERROR(status)) return status;
        return gcvSTATUS_OK;

    case clvIR_BINARY_EXPR:
        binaryExpr = (cloIR_BINARY_EXPR)Base;
        gcmASSERT(binaryExpr->leftOperand);
        gcmASSERT(binaryExpr->rightOperand);

        switch (binaryExpr->type) {
        case clvBINARY_SUBSCRIPT:
            if (!clmDECL_IsBVecOrIVecOrVec(&binaryExpr->leftOperand->decl)
                || cloIR_OBJECT_GetType(&binaryExpr->leftOperand->base) != clvIR_VARIABLE
                || cloIR_OBJECT_GetType(&binaryExpr->rightOperand->base) != clvIR_CONSTANT) {
                Parameters->usingSingleVectorIndex = gcvFALSE;
                return gcvSTATUS_OK;
            }

            name = ((cloIR_VARIABLE)binaryExpr->leftOperand)->name;
            gcmASSERT(name);

            gcmASSERT(((cloIR_CONSTANT)binaryExpr->rightOperand)->valueCount == 1);
            vectorIndex = ((cloIR_CONSTANT)binaryExpr->rightOperand)->values[0].intValue;

            return _AddNameToParameters(Compiler,
                            name,
                            vectorIndex,
                            NeedLValue,
                            NeedRValue,
                            Parameters);

        case clvBINARY_ADD:
        case clvBINARY_SUB:
        case clvBINARY_MUL:
        case clvBINARY_DIV:
        case clvBINARY_MOD:

        case clvBINARY_GREATER_THAN:
        case clvBINARY_LESS_THAN:
        case clvBINARY_GREATER_THAN_EQUAL:
        case clvBINARY_LESS_THAN_EQUAL:

        case clvBINARY_EQUAL:
        case clvBINARY_NOT_EQUAL:

        case clvBINARY_AND:
        case clvBINARY_OR:
        case clvBINARY_XOR:

            case clvBINARY_BITWISE_AND:
            case clvBINARY_BITWISE_OR:
            case clvBINARY_BITWISE_XOR:

        case clvBINARY_LSHIFT:
        case clvBINARY_RSHIFT:
            needLValue0 = gcvFALSE;
            needRValue0 = NeedRValue;
            needLValue1 = gcvFALSE;
            needRValue1 = NeedRValue;
            break;

        case clvBINARY_SEQUENCE:
            needLValue0 = gcvFALSE;
            needRValue0 = gcvFALSE;
            needLValue1 = gcvFALSE;
            needRValue1 = NeedRValue;
            break;

        case clvBINARY_ASSIGN:
            needLValue0 = gcvTRUE;
            needRValue0 = gcvFALSE;
            needLValue1 = gcvFALSE;
            needRValue1 = gcvTRUE;
            break;

        case clvBINARY_LEFT_ASSIGN:
        case clvBINARY_RIGHT_ASSIGN:
        case clvBINARY_AND_ASSIGN:
        case clvBINARY_XOR_ASSIGN:
        case clvBINARY_OR_ASSIGN:
        case clvBINARY_MUL_ASSIGN:
        case clvBINARY_DIV_ASSIGN:
        case clvBINARY_ADD_ASSIGN:
        case clvBINARY_SUB_ASSIGN:
        case clvBINARY_MOD_ASSIGN:
            needLValue0 = gcvTRUE;
            needRValue0 = gcvTRUE;
            needLValue1 = gcvFALSE;
            needRValue1 = gcvTRUE;
            break;

        default:
            gcmASSERT(0);
        }

        /* Check the left operand */
        status = cloIR_BASE_UsingSingleVectorIndex(Compiler,
                               &binaryExpr->leftOperand->base,
                               needLValue0,
                               needRValue0,
                               Parameters);
        if (gcmIS_ERROR(status)) return status;

        if (!Parameters->usingSingleVectorIndex) return gcvSTATUS_OK;

        /* Check the right operand */
        status = cloIR_BASE_UsingSingleVectorIndex(Compiler,
                               &binaryExpr->rightOperand->base,
                               needLValue1,
                               needRValue1,
                               Parameters);
        if (gcmIS_ERROR(status)) return status;
        return gcvSTATUS_OK;

    case clvIR_SELECTION:
        selection = (cloIR_SELECTION)Base;
        gcmASSERT(selection->condExpr);

        /* Check the condition expression */
        status = cloIR_BASE_UsingSingleVectorIndex(Compiler,
                               &selection->condExpr->base,
                               gcvFALSE,
                               gcvTRUE,
                               Parameters);
        if (gcmIS_ERROR(status)) return status;
        if (!Parameters->usingSingleVectorIndex) return gcvSTATUS_OK;

        Parameters->currentLevel++;
        Parameters->maxLevel++;

        /* Check the true operand */
        if (selection->trueOperand != gcvNULL) {
            status = cloIR_BASE_UsingSingleVectorIndex(Compiler,
                                   selection->trueOperand,
                                   gcvFALSE,
                                   NeedRValue,
                                   Parameters);
            if (gcmIS_ERROR(status)) return status;
            if (!Parameters->usingSingleVectorIndex) return gcvSTATUS_OK;

            for (i = 0; i < Parameters->outputNameCount; i++) {
                if (Parameters->outputNameLevelTable[i] == i) {
                    Parameters->outputNameLevelTable[i] = MAX_LEVEL;
                }
            }
        }

        /* Check the false operand */
        if (selection->falseOperand != gcvNULL) {
            status = cloIR_BASE_UsingSingleVectorIndex(Compiler,
                                   selection->falseOperand,
                                   gcvFALSE,
                                   NeedRValue,
                                   Parameters);
            if (gcmIS_ERROR(status)) return status;
            if (!Parameters->usingSingleVectorIndex) return gcvSTATUS_OK;

            for (i = 0; i < Parameters->outputNameCount; i++) {
                if (Parameters->outputNameLevelTable[i] == i) {
                    Parameters->outputNameLevelTable[i] = MAX_LEVEL;
                }
            }
        }

        Parameters->currentLevel--;
        return gcvSTATUS_OK;

    case clvIR_POLYNARY_EXPR:
        polynaryExpr = (cloIR_POLYNARY_EXPR)Base;
        if (polynaryExpr->type == clvPOLYNARY_FUNC_CALL
            && !polynaryExpr->funcName->isBuiltin) {
            Parameters->usingSingleVectorIndex = gcvFALSE;
            return gcvSTATUS_OK;
        }

        if (polynaryExpr->operands != gcvNULL) {
            /* Check all operands */
            status = cloIR_BASE_UsingSingleVectorIndex(Compiler,
                                   &polynaryExpr->operands->base,
                                   gcvFALSE,
                                   gcvTRUE,
                                   Parameters);
            if (gcmIS_ERROR(status)) return status;
        }
        return gcvSTATUS_OK;

    default:
        Parameters->usingSingleVectorIndex = gcvFALSE;
        return gcvSTATUS_OK;
    }
}

gctBOOL
cloIR_BASE_IsEqualExceptVectorIndex(
    IN cloCOMPILER Compiler,
    IN cloIR_BASE Base0,
    IN cloIR_BASE Base1
    )
{
    cloIR_SET                set0, set1;
    cloIR_VARIABLE            variable0, variable1;
    cloIR_CONSTANT            constant0, constant1;
    cloIR_UNARY_EXPR        unaryExpr0, unaryExpr1;
    cloIR_BINARY_EXPR        binaryExpr0, binaryExpr1;
    cloIR_SELECTION            selection0, selection1;
    cloIR_POLYNARY_EXPR        polynaryExpr0, polynaryExpr1;
    cloIR_BASE                member0, member1;
    gctUINT                    i;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    gcmASSERT(Base0);
    gcmASSERT(Base1);

    if (cloIR_OBJECT_GetType(Base0) != cloIR_OBJECT_GetType(Base1)) return gcvFALSE;

    switch (cloIR_OBJECT_GetType(Base0))
    {
    case clvIR_SET:
        set0 = (cloIR_SET)Base0;
        set1 = (cloIR_SET)Base1;

        if (set0->type != set1->type) return gcvFALSE;

        for (member0 = slsDLINK_LIST_First(&set0->members, struct _cloIR_BASE),
                member1 = slsDLINK_LIST_First(&set1->members, struct _cloIR_BASE);
            (slsDLINK_NODE *)member0 != &set0->members
                && (slsDLINK_NODE *)member1 != &set1->members;
            member0 = slsDLINK_NODE_Next(&member0->node, struct _cloIR_BASE),
                member1 = slsDLINK_NODE_Next(&member1->node, struct _cloIR_BASE))
        {
            if (!cloIR_BASE_IsEqualExceptVectorIndex(
                                                    Compiler,
                                                    member0,
                                                    member1)) return gcvFALSE;
        }

        if ((slsDLINK_NODE *)member0 != &set0->members
            && (slsDLINK_NODE *)member1 != &set1->members) return gcvFALSE;

        return gcvTRUE;

    case clvIR_VARIABLE:
        variable0 = (cloIR_VARIABLE)Base0;
        variable1 = (cloIR_VARIABLE)Base1;

        return (variable0->name == variable1->name);

    case clvIR_CONSTANT:
        constant0 = (cloIR_CONSTANT)Base0;
        constant1 = (cloIR_CONSTANT)Base1;

        if (constant0->valueCount != constant1->valueCount) return gcvFALSE;

        for (i = 0; i < constant0->valueCount; i++)
        {
            if (constant0->values[i].intValue != constant1->values[i].intValue) return gcvFALSE;
        }

        return gcvTRUE;

    case clvIR_UNARY_EXPR:
        unaryExpr0 = (cloIR_UNARY_EXPR)Base0;
        unaryExpr1 = (cloIR_UNARY_EXPR)Base1;

        if (unaryExpr0->type != unaryExpr1->type) return gcvFALSE;

        if (unaryExpr0->type == clvUNARY_FIELD_SELECTION
            && unaryExpr0->u.fieldName != unaryExpr1->u.fieldName) return gcvFALSE;

        return cloIR_BASE_IsEqualExceptVectorIndex(
                                                Compiler,
                                                &unaryExpr0->operand->base,
                                                &unaryExpr1->operand->base);

    case clvIR_BINARY_EXPR:
        binaryExpr0 = (cloIR_BINARY_EXPR)Base0;
        binaryExpr1 = (cloIR_BINARY_EXPR)Base1;

        if (!cloIR_BASE_IsEqualExceptVectorIndex(
                                                Compiler,
                                                &binaryExpr0->leftOperand->base,
                                                &binaryExpr1->leftOperand->base)) return gcvFALSE;

        if (binaryExpr0->type != clvBINARY_SUBSCRIPT)
        {
            if (!cloIR_BASE_IsEqualExceptVectorIndex(
                                                    Compiler,
                                                    &binaryExpr0->rightOperand->base,
                                                    &binaryExpr1->rightOperand->base))
            {
                return gcvFALSE;
            }
        }

        return gcvTRUE;

    case clvIR_SELECTION:
        selection0 = (cloIR_SELECTION)Base0;
        selection1 = (cloIR_SELECTION)Base1;

        /* Check the condition expression */
        if (!cloIR_BASE_IsEqualExceptVectorIndex(
                                                Compiler,
                                                &selection0->condExpr->base,
                                                &selection1->condExpr->base)) return gcvFALSE;

        /* Check the true operand */
        if (selection0->trueOperand != gcvNULL)
        {
            if (selection1->trueOperand == gcvNULL) return gcvFALSE;

            if (!cloIR_BASE_IsEqualExceptVectorIndex(
                                                    Compiler,
                                                    selection0->trueOperand,
                                                    selection1->trueOperand)) return gcvFALSE;
        }
        else
        {
            if (selection1->trueOperand != gcvNULL) return gcvFALSE;
        }

        /* Check the false operand */
        if (selection0->falseOperand != gcvNULL)
        {
            if (selection1->falseOperand == gcvNULL) return gcvFALSE;

            if (!cloIR_BASE_IsEqualExceptVectorIndex(
                                                    Compiler,
                                                    selection0->falseOperand,
                                                    selection1->falseOperand)) return gcvFALSE;
        }
        else
        {
            if (selection1->falseOperand != gcvNULL) return gcvFALSE;
        }

        return gcvTRUE;

    case clvIR_POLYNARY_EXPR:
        polynaryExpr0 = (cloIR_POLYNARY_EXPR)Base0;
        polynaryExpr1 = (cloIR_POLYNARY_EXPR)Base1;

        if (polynaryExpr0->type != polynaryExpr1->type) return gcvFALSE;

        if (polynaryExpr0->operands != gcvNULL)
        {
            if (polynaryExpr1->operands == gcvNULL) return gcvFALSE;

            if (!cloIR_BASE_IsEqualExceptVectorIndex(
                                                    Compiler,
                                                    &polynaryExpr0->operands->base,
                                                    &polynaryExpr1->operands->base))
            {
                return gcvFALSE;
            }
        }
        else
        {
            if (polynaryExpr1->operands != gcvNULL) return gcvFALSE;
        }

        return gcvTRUE;

    default:
        return gcvFALSE;
    }
}

typedef struct _clsCOMPARE_ALL_NAMES_COMPONENT_PARAMETERS
{
    gctUINT            nameCount;

    clsNAME * *        nameTable;

    gctUINT            vectorIndex0;

    gctUINT            vectorIndex1;

    gctBOOL            compareResults[MAX_NAME_TABLE_SIZE];
}
clsCOMPARE_ALL_NAMES_COMPONENT_PARAMETERS;

#define clsCOMPARE_ALL_NAMES_COMPONENT_PARAMETERS_Initialize( \
                                                            parameters, \
                                                            _nameCount, \
                                                            _nameTable, \
                                                            _vectorIndex0, \
                                                            _vectorIndex1) \
    do \
    { \
        gctUINT    _i; \
        \
        (parameters)->nameCount        = (_nameCount); \
        (parameters)->nameTable        = (_nameTable); \
        (parameters)->vectorIndex0    = (_vectorIndex0); \
        (parameters)->vectorIndex1    = (_vectorIndex1); \
        \
        for (_i = 0; _i < (_nameCount); _i++) \
        { \
            (parameters)->compareResults[_i] = gcvFALSE; \
        } \
    } \
    while (gcvFALSE)

#define clsCOMPARE_ALL_NAMES_COMPONENT_PARAMETERS_ClearResults(parameters) \
    do \
    { \
        gctUINT    _i; \
        \
        for (_i = 0; _i < (parameters)->nameCount; _i++) \
        { \
            (parameters)->compareResults[_i] = gcvFALSE; \
        } \
    } \
    while (gcvFALSE)

gctBOOL
cloIR_BASE_CompareAllNamesComponent(
IN cloCOMPILER Compiler,
IN cloIR_BASE Base,
IN OUT clsCOMPARE_ALL_NAMES_COMPONENT_PARAMETERS * Parameters,
OUT gctBOOL * NeedClearResults
)
{
    cloIR_VARIABLE        variable    = gcvNULL;
    cloIR_CONSTANT        constant    = gcvNULL;
    cloIR_UNARY_EXPR    unaryExpr    = gcvNULL;
    cloIR_BINARY_EXPR    binaryExpr    = gcvNULL;
    cloIR_POLYNARY_EXPR    polynaryExpr    = gcvNULL;
    clsNAME *        name;
    gctUINT            nameIndex;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    gcmASSERT(Base);
    gcmASSERT(Parameters);
    gcmASSERT(NeedClearResults);

    *NeedClearResults = gcvFALSE;

    switch (cloIR_OBJECT_GetType(Base)) {
    case clvIR_VARIABLE:
        variable = (cloIR_VARIABLE)Base;
        return clmDECL_IsScalar(&variable->exprBase.decl);

    case clvIR_CONSTANT:
        constant = (cloIR_CONSTANT)Base;
        if (clmDECL_IsScalar(&constant->exprBase.decl)) return gcvTRUE;
        if (clmDECL_IsBVecOrIVecOrVec(&constant->exprBase.decl)) {
           if (clmDATA_TYPE_vectorSize_NOCHECK_GET(constant->exprBase.decl.dataType) > Parameters->vectorIndex0
               && clmDATA_TYPE_vectorSize_NOCHECK_GET(constant->exprBase.decl.dataType) > Parameters->vectorIndex1) {
                return (constant->values[Parameters->vectorIndex0].intValue
                            == constant->values[Parameters->vectorIndex1].intValue);
            }
        }
        return gcvFALSE;

    case clvIR_UNARY_EXPR:
        unaryExpr = (cloIR_UNARY_EXPR)Base;
        gcmASSERT(unaryExpr->operand);

        if (clmDECL_IsScalar(&unaryExpr->exprBase.decl)) return gcvTRUE;

        switch (unaryExpr->type) {
        case clvUNARY_FIELD_SELECTION:
            return gcvFALSE;

        case clvUNARY_COMPONENT_SELECTION:
            return (unaryExpr->u.componentSelection.selection[Parameters->vectorIndex0]
                                == unaryExpr->u.componentSelection.selection[Parameters->vectorIndex1]);

        case clvUNARY_POST_INC:
        case clvUNARY_POST_DEC:
        case clvUNARY_PRE_INC:
        case clvUNARY_PRE_DEC:

        case clvUNARY_NEG:

        case clvUNARY_NOT:
        case clvUNARY_BITWISE_NOT:
            return cloIR_BASE_CompareAllNamesComponent(Compiler,
                                   &unaryExpr->operand->base,
                                   Parameters,
                                   NeedClearResults);

        default:
            gcmASSERT(0);
            return gcvFALSE;
        }

    case clvIR_BINARY_EXPR:
        binaryExpr = (cloIR_BINARY_EXPR)Base;
        gcmASSERT(binaryExpr->leftOperand);
        gcmASSERT(binaryExpr->rightOperand);

        if (clmDECL_IsScalar(&binaryExpr->exprBase.decl)) return gcvTRUE;

        switch (binaryExpr->type) {
        case clvBINARY_SUBSCRIPT:
            return gcvFALSE;

        case clvBINARY_ADD:
        case clvBINARY_SUB:
        case clvBINARY_MUL:
        case clvBINARY_DIV:
        case clvBINARY_MOD:

            if (!cloIR_BASE_CompareAllNamesComponent(Compiler,
                                 &binaryExpr->leftOperand->base,
                                 Parameters,
                                 NeedClearResults)) return gcvFALSE;

            if (!cloIR_BASE_CompareAllNamesComponent(Compiler,
                                 &binaryExpr->rightOperand->base,
                                 Parameters,
                                 NeedClearResults)) return gcvFALSE;
            return gcvTRUE;

        case clvBINARY_GREATER_THAN:
        case clvBINARY_LESS_THAN:
        case clvBINARY_GREATER_THAN_EQUAL:
        case clvBINARY_LESS_THAN_EQUAL:

        case clvBINARY_EQUAL:
        case clvBINARY_NOT_EQUAL:

        case clvBINARY_AND:
        case clvBINARY_OR:
        case clvBINARY_XOR:

        case clvBINARY_BITWISE_AND:
        case clvBINARY_BITWISE_OR:
        case clvBINARY_BITWISE_XOR:
            return gcvTRUE;

        case clvBINARY_SEQUENCE:
            if (!cloIR_BASE_CompareAllNamesComponent(Compiler,
                                 &binaryExpr->rightOperand->base,
                                 Parameters,
                                 NeedClearResults)) return gcvFALSE;
            return gcvTRUE;

        case clvBINARY_ASSIGN:

        case clvBINARY_LEFT_ASSIGN:
        case clvBINARY_RIGHT_ASSIGN:
        case clvBINARY_AND_ASSIGN:
        case clvBINARY_XOR_ASSIGN:
        case clvBINARY_OR_ASSIGN:

        case clvBINARY_MUL_ASSIGN:
        case clvBINARY_DIV_ASSIGN:
        case clvBINARY_ADD_ASSIGN:
        case clvBINARY_SUB_ASSIGN:
        case clvBINARY_MOD_ASSIGN:

            if (binaryExpr->type != clvBINARY_ASSIGN
                && !cloIR_BASE_CompareAllNamesComponent(Compiler,
                                    &binaryExpr->leftOperand->base,
                                    Parameters,
                                    NeedClearResults)) return gcvFALSE;

            if (cloIR_OBJECT_GetType(&binaryExpr->leftOperand->base) == clvIR_VARIABLE) {
                name = ((cloIR_VARIABLE)binaryExpr->leftOperand)->name;
                gcmASSERT(name);

                nameIndex = _FindNameInList(name,
                                Parameters->nameCount,
                                Parameters->nameTable);

                if (nameIndex != MAX_NAME_TABLE_SIZE) {
                   Parameters->compareResults[nameIndex] =
                    cloIR_BASE_CompareAllNamesComponent(Compiler,
                                        &binaryExpr->rightOperand->base,
                                        Parameters,
                                        NeedClearResults);
                    return Parameters->compareResults[nameIndex];
                }
            }
            return cloIR_BASE_CompareAllNamesComponent(Compiler,
                                   &binaryExpr->rightOperand->base,
                                   Parameters,
                                   NeedClearResults);

        default:
            gcmASSERT(0);
            return gcvFALSE;
        }

    case clvIR_POLYNARY_EXPR:
        polynaryExpr = (cloIR_POLYNARY_EXPR)Base;

        if (polynaryExpr->type == clvPOLYNARY_FUNC_CALL
            && !polynaryExpr->funcName->isBuiltin) {
            *NeedClearResults = gcvTRUE;
            return gcvFALSE;
        }

        return clmDECL_IsScalar(&polynaryExpr->exprBase.decl);

    default:
        *NeedClearResults = gcvTRUE;
        return gcvFALSE;
    }
}

gctBOOL
cloIR_SET_CompareAllNamesComponent(
    IN cloCOMPILER Compiler,
    IN cloIR_SET StatementSet,
    IN cloIR_BASE StopStatement,
    IN gctUINT NameCount,
    IN clsNAME * NameTable[MAX_NAME_TABLE_SIZE],
    IN gctUINT VectorIndex0,
    IN gctUINT VectorIndex1
    )
{
    clsCOMPARE_ALL_NAMES_COMPONENT_PARAMETERS    parameters;
    gctUINT                                        i;
    cloIR_BASE                                    statement;
    gctBOOL                                        needClearResults;

    clsCOMPARE_ALL_NAMES_COMPONENT_PARAMETERS_Initialize(
                                                        &parameters,
                                                        NameCount,
                                                        NameTable,
                                                        VectorIndex0,
                                                        VectorIndex1);

    FOR_EACH_DLINK_NODE(&StatementSet->members, struct _cloIR_BASE, statement)
    {
        if (statement == StopStatement) break;

        cloIR_BASE_CompareAllNamesComponent(Compiler, statement, &parameters, &needClearResults);

        if (needClearResults)
        {
            clsCOMPARE_ALL_NAMES_COMPONENT_PARAMETERS_ClearResults(&parameters);
        }
    }

    for (i = 0; i < parameters.nameCount; i++)
    {
        if (!parameters.compareResults[i]) return gcvFALSE;
    }

    return gcvTRUE;
}

typedef struct _clsSPECIAL_STATEMENT_CONTEXT
{
    gctBOOL                                        codeGenerated;

    cloIR_BASE                                    prevStatement;

    clsUSING_SINGLE_VECTOR_INDEX_PARAMETERS        prevParameters;
}
clsSPECIAL_STATEMENT_CONTEXT;

#define clsSPECIAL_STATEMENT_CONTEXT_Initialize(context) \
    do \
    { \
        (context)->codeGenerated    = gcvFALSE; \
        (context)->prevStatement    = gcvNULL; \
        clsUSING_SINGLE_VECTOR_INDEX_PARAMETERS_Initialize(&(context)->prevParameters); \
    } \
    while (gcvFALSE)

gceSTATUS
cloIR_SET_TryToGenSpecialStatementCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_SET StatementSet,
    IN cloIR_BASE Statement,
    IN OUT clsSPECIAL_STATEMENT_CONTEXT * StatementContext
    )
{
    gceSTATUS status;
    clsUSING_SINGLE_VECTOR_INDEX_PARAMETERS    parameters;
    gctUINT    i;
    clsNAME *name;
    clsLOPERAND lOperand, componentLOperand;
    clsROPERAND rOperand, componentROperand;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_OBJECT(CodeGenerator, clvOBJ_CODE_GENERATOR);
    clmVERIFY_IR_OBJECT(StatementSet, clvIR_SET);
    gcmASSERT(Statement);
    gcmASSERT(StatementContext);

    do
    {
        /* Check the current statement */
        clsUSING_SINGLE_VECTOR_INDEX_PARAMETERS_Initialize(&parameters);

        status = cloIR_BASE_UsingSingleVectorIndex(Compiler,
                            Statement,
                            gcvFALSE,
                            gcvFALSE,
                            &parameters);

        if (gcmIS_ERROR(status)) return status;

        if (!parameters.usingSingleVectorIndex
            || parameters.vectorIndex == INVALID_VECTOR_INDEX) break;

        if (parameters.maxLevel == 0) break;

        /* Check the previous statement */
        if (StatementContext->prevStatement == gcvNULL) break;

        gcmASSERT(StatementContext->prevParameters.usingSingleVectorIndex);
        gcmASSERT(StatementContext->prevParameters.vectorIndex != INVALID_VECTOR_INDEX);

        /* Compare the results */
        if (parameters.vectorIndex == StatementContext->prevParameters.vectorIndex) break;

        if (!_IsNameListEqual(parameters.inputNameCount,
                    parameters.inputNameTable,
                    StatementContext->prevParameters.inputNameCount,
                    StatementContext->prevParameters.inputNameTable)) break;

        if (!_IsNameListEqual(parameters.outputNameCount,
                    parameters.outputNameTable,
                    StatementContext->prevParameters.outputNameCount,
                    StatementContext->prevParameters.outputNameTable)) break;

        /* Compare the statements */
        if (!cloIR_BASE_IsEqualExceptVectorIndex(Compiler,
                            Statement,
                            StatementContext->prevStatement)) break;

        /* Check all input names */
        if (!cloIR_SET_CompareAllNamesComponent(Compiler,
                            StatementSet,
                            StatementContext->prevStatement,
                            parameters.inputNameCount,
                            parameters.inputNameTable,
                            StatementContext->prevParameters.vectorIndex,
                            parameters.vectorIndex)) break;

        /* Generate the special assign code */
        for (i = 0; i < parameters.outputNameCount; i++) {
            name = parameters.outputNameTable[i];
            gcmASSERT(clmDECL_IsBVecOrIVecOrVec(&name->decl));
            gcmASSERT(name->context.u.variable.logicalRegCount == 1);

            clsLOPERAND_Initialize(&lOperand, &name->context.u.variable.logicalRegs[0]);
            clsROPERAND_InitializeReg(&rOperand, &name->context.u.variable.logicalRegs[0]);

            clsLOPERAND_InitializeAsVectorComponent(&componentLOperand,
                                &lOperand,
                                parameters.vectorIndex);
            clsROPERAND_InitializeAsVectorComponent(&componentROperand,
                                &rOperand,
                                StatementContext->prevParameters.vectorIndex);

            status = clGenAssignCode(Compiler,
                         Statement->lineNo,
                         Statement->stringNo,
                         &componentLOperand,
                         &componentROperand);

            if (gcmIS_ERROR(status)) return status;
        }

        StatementContext->codeGenerated = gcvTRUE;
        return gcvSTATUS_OK;
    } while (gcvFALSE);

    StatementContext->codeGenerated = gcvFALSE;

    if (parameters.usingSingleVectorIndex
        && parameters.vectorIndex != INVALID_VECTOR_INDEX) {
        StatementContext->prevStatement    = Statement;
        StatementContext->prevParameters= parameters;
    }
    else {
        StatementContext->prevStatement = gcvNULL;
    }
    return gcvSTATUS_OK;
}

gctBOOL
cloIR_BASE_HasReturn(
IN cloCOMPILER Compiler,
IN cloIR_BASE Statement
)
{
    cloIR_SELECTION    selection;
    cloIR_SET    set;
    cloIR_BASE    member;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    gcmASSERT(Statement);

    switch (cloIR_OBJECT_GetType(Statement)) {
    case clvIR_JUMP:
        return (((cloIR_JUMP)Statement)->type == clvRETURN);

    case clvIR_SELECTION:
        selection = (cloIR_SELECTION)Statement;

        if (selection->trueOperand == gcvNULL
            || selection->falseOperand == gcvNULL) return gcvFALSE;

        return cloIR_BASE_HasReturn(Compiler, selection->trueOperand)
                && cloIR_BASE_HasReturn(Compiler, selection->falseOperand);

    case clvIR_SET:
        set = (cloIR_SET)Statement;

        if (set->type != clvSTATEMENT_SET) return gcvFALSE;

        FOR_EACH_DLINK_NODE(&set->members, struct _cloIR_BASE, member) {
            if (cloIR_BASE_HasReturn(Compiler, member)) return gcvTRUE;
        }
        return gcvFALSE;

    default:
        return gcvFALSE;
    }
}

gceSTATUS
cloIR_POLYNARY_EXPR_FinalizeOperandsParameters(
    IN cloCOMPILER Compiler,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters
    )
{
    gctUINT        i;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);

    if (OperandCount == 0) return gcvSTATUS_OK;

    gcmASSERT(OperandsParameters);

    for (i = 0; i < OperandCount; i++)
    {
        clsGEN_CODE_PARAMETERS_Finalize(&OperandsParameters[i]);
    }

    gcmVERIFY_OK(cloCOMPILER_Free(Compiler, OperandsParameters));

    return gcvSTATUS_OK;
}

gceSTATUS
clsGEN_CODE_PARAMETERS_AllocateOperandsByName(
IN cloCOMPILER Compiler,
IN OUT clsGEN_CODE_PARAMETERS * Parameters,
IN clsNAME * Name
)
{
    gceSTATUS status;
    gctUINT    start = 0;
    gctPOINTER pointer;
        clsDECL * decl;
    gctBOOL forceNeedROperand = gcvFALSE;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    gcmASSERT(Parameters);
        gcmASSERT(Name && Name->decl.dataType);

    decl = &Name->decl;
    gcmASSERT(Parameters->operandCount == 0);

    if(!Parameters->needLOperand && !Parameters->needROperand) {
           forceNeedROperand = gcvTRUE;
        }

    Parameters->operandCount = _GetOperandCountForRegAlloc(Name);
    gcmASSERT(Parameters->operandCount > 0);

    status = cloCOMPILER_ZeroMemoryAllocate(Compiler,
                                (gctSIZE_T)sizeof(clsGEN_CODE_PARAMETER_DATA_TYPE) * Parameters->operandCount,
                                (gctPOINTER *) &pointer);
    if (gcmIS_ERROR(status)) return status;
        Parameters->dataTypes = pointer;

    status = _ConvDataTypeByName(Name,
                         Parameters->dataTypes,
                         &start);
    if (gcmIS_ERROR(status)) return status;

    gcmASSERT(start == Parameters->operandCount);

    if (Parameters->needLOperand)
    {
        status = cloCOMPILER_Allocate(Compiler,
                    (gctSIZE_T)sizeof(clsLOPERAND) * Parameters->operandCount,
                    (gctPOINTER *) &pointer);
        if (gcmIS_ERROR(status)) return status;
                Parameters->lOperands = pointer;
    }

    if (Parameters->needROperand || forceNeedROperand)
    {
        status = cloCOMPILER_Allocate(Compiler,
                    (gctSIZE_T)sizeof(clsROPERAND) * Parameters->operandCount,
                    (gctPOINTER *) &pointer);
        if (gcmIS_ERROR(status)) return status;
            Parameters->rOperands = pointer;
    }

    return gcvSTATUS_OK;
}

static gceSTATUS
_GenOperandsCodeForKernelFuncCall(
IN cloCOMPILER Compiler,
IN cloCODE_GENERATOR CodeGenerator,
IN clsNAME * KernelFuncName,
OUT gctUINT * OperandCount,
OUT clsGEN_CODE_PARAMETERS * * OperandsParameters
)
{
   gceSTATUS status = gcvSTATUS_OK;
   gctUINT   operandCount;
   clsGEN_CODE_PARAMETERS *operandsParameters;
   gctUINT  i, j;
   clsNAME *paramName;
   gctBOOL needLOperand, needROperand;
   gcUNIFORM uniform;

   /* Verify the arguments. */
   clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
   clmVERIFY_OBJECT(CodeGenerator, clvOBJ_CODE_GENERATOR);
   gcmASSERT(OperandCount);
   gcmASSERT(OperandsParameters);

   do {
      gcmVERIFY_OK(cloNAME_GetParamCount(Compiler,
                                         KernelFuncName,
                                         &operandCount));

      if (operandCount == 0) break;

      status = cloCOMPILER_Allocate(Compiler,
                    (gctSIZE_T)sizeof(clsGEN_CODE_PARAMETERS) * operandCount,
                    (gctPOINTER *) &operandsParameters);
      if (gcmIS_ERROR(status)) break;

/** Create uniform variables for calling arguments to the kernel function **/
      i = 0;
      FOR_EACH_DLINK_NODE(&KernelFuncName->u.funcInfo.localSpace->names, clsNAME, paramName) {
         if (paramName->type == clvPARAMETER_NAME) {
             clsGEN_CODE_DATA_TYPE binaryDataType;
             gctUINT binaryDataTypeRegSize;
             clsGEN_CODE_DATA_TYPE format;
             gctUINT logicalRegCount = 1;

             binaryDataType = _ConvElementDataTypeForRegAlloc(paramName);
             binaryDataTypeRegSize = gcGetDataTypeRegSize(binaryDataType);
             clmGEN_CODE_ConvDirectElementDataType(paramName->decl.dataType, format);

#if cldSupportMultiKernelFunction
             status = clNewKernelUniformArgument(Compiler,
                                                 KernelFuncName,
                                                 paramName->symbol,
                                                 binaryDataType,
                                                 format,
                                                 paramName,
                                                 logicalRegCount,
                                                 &uniform);
#else
             status = clNewUniform(Compiler,
                                   KernelFuncName->lineNo,
                                   KernelFuncName->stringNo,
                                   paramName->symbol,
                                   binaryDataType,
                                   format,
                                   clvBUILTIN_KERNEL_ARG,
                                   clmDECL_IsPointerType(&paramName->decl) ||
                                   clmGEN_CODE_checkVariableForMemory(paramName),
                                   logicalRegCount,
                                   &uniform);
#endif
             if (gcmIS_ERROR(status)) return status;
             SetUniformQualifier(uniform, clConvToShaderTypeQualifier(paramName->decl.dataType->accessQualifier));
             SetUniformQualifier(uniform, clConvToShaderTypeQualifier(paramName->decl.dataType->addrSpaceQualifier));
             SetUniformQualifier(uniform, clConvStorageQualifierToShaderTypeQualifier(paramName->decl.storageQualifier));
             _SetPointerUniformQualifiers(uniform, &paramName->decl);

             _AddDerivedTypeVariable(Compiler,
                                     CodeGenerator,
                                     paramName,
                                     uniform);

             needLOperand = gcvFALSE;
             needROperand = gcvTRUE;

             clsGEN_CODE_PARAMETERS_Initialize(&operandsParameters[i], needLOperand, needROperand);
             /* Allocate the operands */
             status = clsGEN_CODE_PARAMETERS_AllocateOperandsByName(Compiler,
                                                                    operandsParameters + i,
                                                                    paramName);
             if (gcmIS_ERROR(status)) return status;

             if(operandsParameters[i].needROperand) {
                 for (j = 0; j < logicalRegCount; j++) {
                     clsLOGICAL_REG reg[1];
                     clsLOGICAL_REG_InitializeUniform(reg,
                                                      clvQUALIFIER_UNIFORM,
                                                      binaryDataType,
                                                      uniform,
                                                      (gctREG_INDEX)(j * binaryDataTypeRegSize),
                                                      gcvFALSE);
                     clsROPERAND_InitializeReg(operandsParameters[i].rOperands + j, reg);
                 }
             }
             if(operandsParameters[i].needLOperand) {
                 for (j = 0; j < logicalRegCount; j++) {
                     clsLOGICAL_REG reg[1];
                     clsLOGICAL_REG_InitializeUniform(reg,
                                                      clvQUALIFIER_UNIFORM,
                                                      binaryDataType,
                                                      uniform,
                                                      (gctREG_INDEX)(j * binaryDataTypeRegSize),
                                                      gcvFALSE);
                     clsLOPERAND_Initialize(operandsParameters[i].lOperands + j, reg);
                 }
             }
             i++;
          }
          else break;
       }
       *OperandCount        = operandCount;
       *OperandsParameters    = operandsParameters;

       return gcvSTATUS_OK;
   }
   while (gcvFALSE);

   *OperandCount    = 0;
   *OperandsParameters    = gcvNULL;

   return status;
}


#define clmGetFunctionLabel(Compiler, FuncName, Label) \
    ((FuncName)->type == clvKERNEL_FUNC_NAME  \
      ?  clGetKernelFunctionLabel(Compiler, \
                                  (FuncName)->context.u.variable.u.kernelFunction, \
                                  Label)  \
      :  clGetFunctionLabel(Compiler, \
                            (FuncName)->context.u.variable.u.function, \
                            Label))

gceSTATUS
cloIR_GenKernelFuncCall(
IN cloCOMPILER Compiler,
IN cloCODE_GENERATOR CodeGenerator,
IN clsNAME *KernelFuncName
)
{
    gceSTATUS        status;
    gctUINT          operandCount = 0, i, j;
    clsGEN_CODE_PARAMETERS *operandsParameters = gcvNULL;
    clsNAME *        paramName;
    clsLOPERAND      lOperand[1];
    clsIOPERAND      addressOffset[1];
    clsROPERAND      rOperand1[1], rOperand2[1];
    clsROPERAND      localBaseAddr[1];
    clsLOGICAL_REG   *regs;
    gctLABEL         funcLabel;
    gctBOOL          hasArgPtrToLocal = gcvFALSE;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_OBJECT(CodeGenerator, clvOBJ_CODE_GENERATOR);

    /* Allocate the Kernel function resources */
    status = _AllocateKernelFuncResources(Compiler,
                                          CodeGenerator,
                                          KernelFuncName);
    if (gcmIS_ERROR(status)) return status;

    /* Generate the code of all operands */
    status = _GenOperandsCodeForKernelFuncCall(Compiler,
                                               CodeGenerator,
                                               KernelFuncName,
                                               &operandCount,
                                               &operandsParameters);
    if (gcmIS_ERROR(status)) return status;

    /* for all arguments */
    i = 0;
    FOR_EACH_DLINK_NODE(&KernelFuncName->u.funcInfo.localSpace->names, clsNAME, paramName) {
       if (paramName->type != clvPARAMETER_NAME) break;

       gcmASSERT(i < operandCount);
       gcmASSERT(operandsParameters[i].needROperand);

       for (j = 0; j < operandsParameters[i].operandCount; j++) {
            if(clmDECL_IsPointerType(&paramName->decl) &&
               paramName->decl.dataType->addrSpaceQualifier == clvQUALIFIER_LOCAL) {
                clsIOPERAND_Initialize(addressOffset,
                                   paramName->context.u.variable.logicalRegs[j].dataType,
                                   paramName->context.u.variable.logicalRegs[j].regIndex);
                if(!hasArgPtrToLocal) {
                    clResetLocalTempRegs(Compiler, 0);
                    clsROPERAND_InitializeTempReg(rOperand1,
                                                  clvQUALIFIER_NONE,
                                                  clmGenCodeDataType(T_UINT),
                                                  cldLocalMemoryAddressRegIndex);
/* compute base address */
                    regs = cloCOMPILER_GetBuiltinVariable(Compiler, clvBUILTIN_ARG_LOCAL_MEM_SIZE)
                                                                                ->context.u.variable.logicalRegs;
                    clsROPERAND_InitializeReg(rOperand2, regs);
                    status = _GenScaledIndexOperand(Compiler,
                                                    KernelFuncName->lineNo,
                                                    KernelFuncName->stringNo,
                                                    rOperand1,
                                                    rOperand2,
                                                    localBaseAddr);
                    if (gcmIS_ERROR(status)) return status;
                    hasArgPtrToLocal = gcvTRUE;
                }

                status = clGenArithmeticExprCode(Compiler,
                                                 KernelFuncName->lineNo,
                                                 KernelFuncName->stringNo,
                                                 clvOPCODE_ADD,
                                                 addressOffset,
                                                 localBaseAddr,
                                                 operandsParameters[i].rOperands + j);
                if (gcmIS_ERROR(status)) return status;
            }
            else {
                clsLOPERAND_Initialize(lOperand, paramName->context.u.variable.logicalRegs + j);

                status = clGenAssignCode(Compiler,
                                         KernelFuncName->lineNo,
                                         KernelFuncName->stringNo,
                                         lOperand,
                                         operandsParameters[i].rOperands + j);
                if (gcmIS_ERROR(status)) return status;
            }
       }
       i++;
    }

    /* Compute final base addresses to local allocated memory space */
    if(cloCOMPILER_IsLocalMemoryNeeded(Compiler)) {
       clsIOPERAND_Initialize(addressOffset, clmGenCodeDataType(T_UINT), cldLocalMemoryAddressRegIndex);
       clsROPERAND_InitializeUsingIOperand(rOperand1, addressOffset);

       regs = cloCOMPILER_GetBuiltinVariable(Compiler, clvBUILTIN_LOCAL_ADDRESS_SPACE)->context.u.variable.logicalRegs;
       clsROPERAND_InitializeReg(rOperand2, regs);

       status = clGenScaledIndexOperand(Compiler,
                                        KernelFuncName->lineNo,
                                        KernelFuncName->stringNo,
                                        rOperand1,
                                        KernelFuncName->u.funcInfo.localMemorySize,
                                        rOperand1);
       if (gcmIS_ERROR(status)) return status;
       status = clGenArithmeticExprCode(Compiler,
                                        KernelFuncName->lineNo,
                                        KernelFuncName->stringNo,
                                        clvOPCODE_ADD,
                                        addressOffset,
                                        rOperand1,
                                        rOperand2);
       if (gcmIS_ERROR(status)) return status;
    }
    /* Generate the call code */
#if cldSupportMultiKernelFunction
    status = clGetKernelFunctionLabel(Compiler,
                          KernelFuncName->context.u.variable.u.kernelFunction,
                          &funcLabel);
#else
    status = clGetFunctionLabel(Compiler,
                    KernelFuncName->context.u.variable.u.function,
                    &funcLabel);
#endif
    if (gcmIS_ERROR(status)) return status;

    status = clEmitAlwaysBranchCode(Compiler,
                    KernelFuncName->lineNo,
                    KernelFuncName->stringNo,
                    clvOPCODE_CALL,
                    funcLabel);
    if (gcmIS_ERROR(status)) return status;
    gcmVERIFY_OK(cloIR_POLYNARY_EXPR_FinalizeOperandsParameters(Compiler,
                                    operandCount,
                                    operandsParameters));
    return gcvSTATUS_OK;
}

#if cldSupportMultiKernelFunction
static gceSTATUS
_DefineFuncBegin(
IN cloCOMPILER Compiler,
IN cloCODE_GENERATOR CodeGenerator,
IN cloIR_SET FuncBody
)
{
    gceSTATUS    status;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_OBJECT(CodeGenerator, clvOBJ_CODE_GENERATOR);
    clmVERIFY_IR_OBJECT(FuncBody, clvIR_SET);
    gcmASSERT(FuncBody->funcName);

    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                      clvDUMP_CODE_GENERATOR,
                      "<FUNC_DEF line=\"%d\" string=\"%d\" name=\"%s\">",
                      FuncBody->base.lineNo,
                      FuncBody->base.stringNo,
                      FuncBody->funcName->symbol));

    CodeGenerator->currentFuncDefContext.isMain    = gcvFALSE;
    CodeGenerator->currentFuncDefContext.funcBody    = FuncBody;
    if(FuncBody->funcName->type == clvKERNEL_FUNC_NAME) {
        CodeGenerator->currentFuncDefContext.isKernel    = gcvTRUE;

        status = _AllocateKernelFuncResources(Compiler,
                              CodeGenerator,
                              FuncBody->funcName);
        if (gcmIS_ERROR(status)) return status;

        CodeGenerator->currentFuncDefContext.mainEndLabel    = clNewLabel(Compiler);
        status = clBeginKernelFunction(Compiler,
                           FuncBody->base.lineNo,
                           FuncBody->base.stringNo,
                           FuncBody->funcName->context.u.variable.u.kernelFunction);
        if (gcmIS_ERROR(status)) return status;
    }
    else {
        CodeGenerator->currentFuncDefContext.isKernel    = gcvFALSE;

        status = _AllocateFuncResources(Compiler,
                                        CodeGenerator,
                                        FuncBody->funcName);
        if (gcmIS_ERROR(status)) return status;

        status = clBeginFunction(Compiler,
                     FuncBody->base.lineNo,
                     FuncBody->base.stringNo,
                     FuncBody->funcName->context.u.variable.u.function);
        if (gcmIS_ERROR(status)) return status;
    }
    return gcvSTATUS_OK;
}

static gceSTATUS
_MakeKernelFunctionEpilog(
IN cloCOMPILER Compiler,
IN cloCODE_GENERATOR CodeGenerator,
IN clsNAME *FuncName
)
{
   gceSTATUS status = gcvSTATUS_OK;

   clResetLocalTempRegs(Compiler, 0);
   gcmONERROR(cloCOMPILER_InKernelFunctionEpilog(Compiler, gcvTRUE));
   gcmONERROR(gcKERNEL_FUNCTION_SetCodeEnd(FuncName->context.u.variable.u.kernelFunction));

   gcmONERROR(clGenBaseMemoryAddressCode(Compiler,
                                         CodeGenerator,
                                         FuncName));

   gcmONERROR(cloIR_GenKernelFuncCall(Compiler,
                                      CodeGenerator,
                                      FuncName));

   gcmONERROR(clEndKernelFunction(Compiler,
                  FuncName));

OnError:
   cloCOMPILER_InKernelFunctionEpilog(Compiler, gcvFALSE);
   return status;
}

static gceSTATUS
_DefineFuncEnd(
IN cloCOMPILER Compiler,
IN cloCODE_GENERATOR CodeGenerator,
IN cloIR_SET FuncBody,
IN gctBOOL HasReturn
)
{
    gceSTATUS  status;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_OBJECT(CodeGenerator, clvOBJ_CODE_GENERATOR);
    clmVERIFY_IR_OBJECT(FuncBody, clvIR_SET);
    gcmASSERT(FuncBody->funcName);

    gcmASSERT(CodeGenerator->currentFuncDefContext.funcBody == FuncBody);

    if (CodeGenerator->currentFuncDefContext.isKernel) {
        status = clSetLabel(Compiler,
                    0,
                    0,
                    CodeGenerator->currentFuncDefContext.mainEndLabel);
        if (gcmIS_ERROR(status)) return status;
        status = clEmitAlwaysBranchCode(Compiler,
                        FuncBody->base.lineNo,
                        FuncBody->base.stringNo,
                        clvOPCODE_RETURN,
                        0);
        if (gcmIS_ERROR(status)) return status;

        status = _MakeKernelFunctionEpilog(Compiler,
                                           CodeGenerator,
                                           FuncBody->funcName);
        if (gcmIS_ERROR(status))  {
            gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                            FuncBody->base.lineNo,
                                            FuncBody->base.stringNo,
                                            clvREPORT_ERROR,
                                            "internal error: failed to make kernel function epilog"));
            return status;
        }
    }
    else {
        if (!HasReturn) {
            if (!clmDECL_IsVoid(&FuncBody->funcName->decl)) {
                gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                                FuncBody->base.lineNo,
                                                FuncBody->base.stringNo,
                                                clvREPORT_WARN,
                                                "non-void function: '%s' must return a value",
                                                FuncBody->funcName->symbol));
            }
            status = clEmitAlwaysBranchCode(Compiler,
                                            FuncBody->base.lineNo,
                                            FuncBody->base.stringNo,
                                            clvOPCODE_RETURN,
                                            0);
            if (gcmIS_ERROR(status)) return status;
        }

        status = clEndFunction(Compiler, FuncBody->funcName->context.u.variable.u.function);
        if (gcmIS_ERROR(status)) return status;
    }

    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                                  clvDUMP_CODE_GENERATOR,
                                  "</FUNC_DEF>"));
    return gcvSTATUS_OK;
}

#else

static gceSTATUS
_DefineFuncBegin(
IN cloCOMPILER Compiler,
IN cloCODE_GENERATOR CodeGenerator,
IN cloIR_SET FuncBody
)
{
    gceSTATUS    status;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_OBJECT(CodeGenerator, clvOBJ_CODE_GENERATOR);
    clmVERIFY_IR_OBJECT(FuncBody, clvIR_SET);
    gcmASSERT(FuncBody->funcName);

    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                      clvDUMP_CODE_GENERATOR,
                      "<FUNC_DEF line=\"%d\" string=\"%d\" name=\"%s\">",
                      FuncBody->base.lineNo,
                      FuncBody->base.stringNo,
                      FuncBody->funcName->symbol));

    if (gcmIS_SUCCESS(gcoOS_StrCmp(FuncBody->funcName->symbol, "main"))) {
        CodeGenerator->currentFuncDefContext.isMain        = gcvTRUE;
        CodeGenerator->currentFuncDefContext.mainEndLabel    = clNewLabel(Compiler);

        status = clBeginMainFunction(Compiler,
                         FuncBody->base.lineNo,
                         FuncBody->base.stringNo);
        if (gcmIS_ERROR(status)) return status;
    }
    else {
        CodeGenerator->currentFuncDefContext.isMain    = gcvFALSE;
        CodeGenerator->currentFuncDefContext.funcBody    = FuncBody;

        status = _AllocateFuncResources(Compiler,
                                        CodeGenerator,
                                        FuncBody->funcName);
        if (gcmIS_ERROR(status)) return status;

        status = clBeginFunction(Compiler,
                     FuncBody->base.lineNo,
                     FuncBody->base.stringNo,
                     FuncBody->funcName->context.u.variable.u.function);
        if (gcmIS_ERROR(status)) return status;
    }
    return gcvSTATUS_OK;
}


static gceSTATUS
_DefineFuncEnd(
IN cloCOMPILER Compiler,
IN cloCODE_GENERATOR CodeGenerator,
IN cloIR_SET FuncBody,
IN gctBOOL HasReturn
)
{
    gceSTATUS    status;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_OBJECT(CodeGenerator, clvOBJ_CODE_GENERATOR);
    clmVERIFY_IR_OBJECT(FuncBody, clvIR_SET);
    gcmASSERT(FuncBody->funcName);

    if (CodeGenerator->currentFuncDefContext.isMain) {
        status = clSetLabel(Compiler,
                    0,
                    0,
                    CodeGenerator->currentFuncDefContext.mainEndLabel);
        if (gcmIS_ERROR(status)) return status;
        status = clEndMainFunction(Compiler);
        if (gcmIS_ERROR(status)) return status;
    }
    else {
        gcmASSERT(CodeGenerator->currentFuncDefContext.funcBody == FuncBody);

        if (!HasReturn) {
            if (!clmDECL_IsVoid(&FuncBody->funcName->decl)) {
                gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                                FuncBody->base.lineNo,
                                                FuncBody->base.stringNo,
                                                clvREPORT_WARN,
                                                "non-void function: '%s' must return a value",
                                                FuncBody->funcName->symbol));
            }
            status = clEmitAlwaysBranchCode(Compiler,
                                            FuncBody->base.lineNo,
                                            FuncBody->base.stringNo,
                                            clvOPCODE_RETURN,
                                            0);
            if (gcmIS_ERROR(status)) return status;
        }
        status = clEndFunction(Compiler, FuncBody->funcName->context.u.variable.u.function);
        if (gcmIS_ERROR(status)) return status;

#if cldNoMain
        if(FuncBody->funcName->type == clvKERNEL_FUNC_NAME) {
            status = cloIR_GenKernelFuncCall(Compiler,
                                             CodeGenerator,
                                             FuncBody->funcName);
            if (gcmIS_ERROR(status)) return status;
        }
#endif
    }

    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                      clvDUMP_CODE_GENERATOR,
                      "</FUNC_DEF>"));
    return gcvSTATUS_OK;
}
#endif

gceSTATUS
clsGEN_CODE_PARAMETERS_AllocateOperands(
IN cloCOMPILER Compiler,
IN OUT clsGEN_CODE_PARAMETERS * Parameters,
IN clsDECL * Decl
)
{
    gceSTATUS status;
    gctUINT    start = 0;
    gctPOINTER pointer;
    gctBOOL forceNeedROperand = gcvFALSE;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    gcmASSERT(Parameters);
    gcmASSERT(Decl && Decl->dataType);

    gcmASSERT(Parameters->operandCount == 0);

    if(!Parameters->needLOperand && !Parameters->needROperand) {
       forceNeedROperand = gcvTRUE;
    }
    Parameters->operandCount = _GetLogicalOperandCount(Decl);
    gcmASSERT(Parameters->operandCount > 0);

    status = cloCOMPILER_ZeroMemoryAllocate(Compiler,
                                (gctSIZE_T)sizeof(clsGEN_CODE_PARAMETER_DATA_TYPE) * Parameters->operandCount,
                                (gctPOINTER *) &pointer);
    if (gcmIS_ERROR(status)) return status;
    Parameters->dataTypes = pointer;

    status = _ConvDataType(Decl,
                           Parameters->dataTypes,
                           &start);
    if (gcmIS_ERROR(status)) return status;

    gcmASSERT(start == Parameters->operandCount);

    if (Parameters->needLOperand)
    {
        status = cloCOMPILER_Allocate(Compiler,
                                      (gctSIZE_T)sizeof(clsLOPERAND) * Parameters->operandCount,
                                      (gctPOINTER *) &pointer);
        if (gcmIS_ERROR(status)) return status;
        Parameters->lOperands = pointer;
    }

    if (Parameters->needROperand || forceNeedROperand)
    {
        status = cloCOMPILER_Allocate(Compiler,
                                      (gctSIZE_T)sizeof(clsROPERAND) * Parameters->operandCount,
                                      (gctPOINTER *) &pointer);
        if (gcmIS_ERROR(status)) return status;
        Parameters->rOperands = pointer;
    }

    return gcvSTATUS_OK;
}

gceSTATUS
_GenOperandsCodeForBuiltinFuncDef(
IN cloCOMPILER Compiler,
IN cloCODE_GENERATOR CodeGenerator,
IN clsNAME *FuncName,
OUT gctUINT * OperandCount,
OUT clsGEN_CODE_PARAMETERS * * OperandsParameters
)
{
   gceSTATUS status = gcvSTATUS_OK;
   clsGEN_CODE_PARAMETERS *operandsParameters;

   /* Verify the arguments. */
   clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
   clmVERIFY_OBJECT(CodeGenerator, clvOBJ_CODE_GENERATOR);

   gcmASSERT(FuncName);
   gcmASSERT(OperandCount);
   gcmASSERT(OperandsParameters);
   do {
      clsNAME *paramName;
      gctUINT operandCount;
      gctUINT    i, j;

      gcmVERIFY_OK(cloNAME_GetParamCount(Compiler,
                                         FuncName,
                                         &operandCount));

      if (operandCount == 0) break;

      status = cloCOMPILER_Allocate(Compiler,
                                    (gctSIZE_T)sizeof(clsGEN_CODE_PARAMETERS) * operandCount,
                                    (gctPOINTER *) &operandsParameters);
      if (gcmIS_ERROR(status)) break;

      i = 0;
      FOR_EACH_DLINK_NODE(&FuncName->u.funcInfo.localSpace->names, clsNAME, paramName) {
        if (paramName->type != clvPARAMETER_NAME) break;

        clsGEN_CODE_PARAMETERS_Initialize(&operandsParameters[i], gcvFALSE, gcvTRUE);

        status = clsGEN_CODE_PARAMETERS_AllocateOperandsByName(Compiler,
                                                               operandsParameters + i,
                                                               paramName);
        if (gcmIS_ERROR(status)) return status;
        for (j = 0; j < paramName->context.u.variable.logicalRegCount; j++) {
           clsROPERAND_InitializeReg(operandsParameters[i].rOperands + j,
                                     paramName->context.u.variable.logicalRegs + j);
        }
        i++;
      }
      *OperandCount = operandCount;
      *OperandsParameters = operandsParameters;

      return gcvSTATUS_OK;
   }
   while (gcvFALSE);

   *OperandCount    = 0;
   *OperandsParameters    = gcvNULL;
   return status;
}

static gceSTATUS
_GenBuiltinFuncDef(
IN cloCOMPILER Compiler,
IN cloCODE_GENERATOR CodeGenerator
)
{
   gceSTATUS status = gcvSTATUS_OK;
   slsSLINK_LIST *builtin;
   clsBUILTIN_FUNC_REFERENCED *prev;
   clsBUILTIN_FUNC_REFERENCED *next;
   cloIR_POLYNARY_EXPR funcCall;
   clsNAME *funcName;
   gctUINT operandCount, i;
   clsGEN_CODE_PARAMETERS  *operandsParameters;
   clsGEN_CODE_PARAMETERS  parameters[1];
   clsIOPERAND resOperand[1];
   clsIOPERAND *iOperand = gcvNULL;

   builtin = cloCOMPILER_GetReferencedBuiltinFuncList(Compiler);

   FOR_EACH_SLINK_NODE(builtin, clsBUILTIN_FUNC_REFERENCED, prev, next) {
      funcCall = next->member;
      funcName = funcCall->funcName;
      gcmASSERT(!funcName->u.funcInfo.isInline);

      CodeGenerator->currentFuncDefContext.isKernel = gcvFALSE;
      status = _AllocateFuncResources(Compiler,
                                      CodeGenerator,
                                      funcName);
      if (gcmIS_ERROR(status)) return status;

      /* set function attributes */
      funcName->context.u.variable.u.function->flags |= gcvFUNC_INTRINSICS | gcvFUNC_INLINEHINT;

      status = clBeginFunction(Compiler,
                               0,
                               cloCOMPILER_GetCurrentStringNo(Compiler),
                               funcName->context.u.variable.u.function);
      if (gcmIS_ERROR(status)) return status;
      status = _GenOperandsCodeForBuiltinFuncDef(Compiler,
                                                 CodeGenerator,
                                                 funcName,
                                                 &operandCount,
                                                 &operandsParameters);
      if (gcmIS_ERROR(status)) return status;
      gcmASSERT(operandCount > 0);


      /* Get the return value */
      if (!clmDECL_IsVoid(&funcName->decl)) {
          clsGEN_CODE_PARAMETERS_Initialize(parameters,
                                            gcvFALSE,
                                            gcvTRUE);

          status = clsGEN_CODE_PARAMETERS_AllocateOperandsByName(Compiler,
                                                                 parameters,
                                                                 funcName);
          if (gcmIS_ERROR(status)) return status;

          for (i = 0; i < parameters->operandCount; i++) {
             clsROPERAND_InitializeReg(parameters->rOperands + i,
                                       funcName->context.u.variable.logicalRegs + i);
          }
          clsIOPERAND_Initialize(resOperand,
                                 parameters->rOperands->dataType,
                                 parameters->rOperands->u.reg.regIndex);
          iOperand = resOperand;
      }
      else {
          clsGEN_CODE_PARAMETERS_Initialize(parameters,
                                            gcvFALSE,
                                            gcvFALSE);
          iOperand = gcvNULL;
      }
      gcmONERROR(clGenBuiltinFunctionCode(Compiler,
                                          CodeGenerator,
                                          funcCall,
                                          operandCount,
                                          operandsParameters,
                                          iOperand,
                                          parameters,
                                          gcvFALSE));

      gcmONERROR(clEmitAlwaysBranchCode(Compiler,
                                        0,
                                        cloCOMPILER_GetCurrentStringNo(Compiler),
                        clvOPCODE_RETURN,
                        0));

      gcmONERROR(clEndFunction(Compiler, funcName->context.u.variable.u.function));

OnError:
      clsGEN_CODE_PARAMETERS_Finalize(operandsParameters);
      clsGEN_CODE_PARAMETERS_Finalize(parameters);
      if(gcmIS_ERROR(status)) return status;
   }
   return status;
}

gceSTATUS
cloIR_SET_GenCode(
IN cloCOMPILER Compiler,
IN cloCODE_GENERATOR CodeGenerator,
IN cloIR_SET Set,
IN OUT clsGEN_CODE_PARAMETERS * Parameters
)
{
    gceSTATUS    status;
    gctBOOL        isRoot;
    cloIR_BASE    member;
    clsGEN_CODE_PARAMETERS    memberParameters;
    gctBOOL        hasReturn = gcvFALSE;
    clsSPECIAL_STATEMENT_CONTEXT    specialStatementContext;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_OBJECT(CodeGenerator, clvOBJ_CODE_GENERATOR);
    clmVERIFY_IR_OBJECT(Set, clvIR_SET);
    gcmASSERT(Parameters);

    switch (Set->type) {
    case clvDECL_SET:
        gcmVERIFY_OK(cloIR_SET_IsRoot(Compiler, Set, &isRoot));

        if (isRoot) {
            /* Generate the initalization code of all global variables */
            FOR_EACH_DLINK_NODE(&Set->members, struct _cloIR_BASE, member) {
                if (cloIR_OBJECT_GetType(member) == clvIR_BINARY_EXPR) {
                    clsGEN_CODE_PARAMETERS_Initialize(&memberParameters, gcvFALSE, gcvFALSE);

                    status = cloIR_OBJECT_Accept(Compiler,
                                     member,
                                     &CodeGenerator->visitor,
                                     &memberParameters);

                    clsGEN_CODE_PARAMETERS_Finalize(&memberParameters);
                    if (gcmIS_ERROR(status)) return status;
                }
            }
            status = _GenBuiltinFuncDef(Compiler, CodeGenerator);
            if(gcmIS_ERROR(status)) return status;
        }

        FOR_EACH_DLINK_NODE(&Set->members, struct _cloIR_BASE, member) {
            if (!isRoot || cloIR_OBJECT_GetType(member) != clvIR_BINARY_EXPR) {
                clsGEN_CODE_PARAMETERS_Initialize(&memberParameters, gcvFALSE, gcvFALSE);

                status = cloIR_OBJECT_Accept(Compiler,
                                 member,
                                 &CodeGenerator->visitor,
                                 &memberParameters);

                clsGEN_CODE_PARAMETERS_Finalize(&memberParameters);
                if (gcmIS_ERROR(status)) return status;
            }
        }
        break;

    case clvSTATEMENT_SET:
        if (Set->funcName != gcvNULL) {
            status = _DefineFuncBegin(Compiler, CodeGenerator, Set);
            if (gcmIS_ERROR(status)) return status;

            hasReturn = gcvFALSE;
        }

        clsSPECIAL_STATEMENT_CONTEXT_Initialize(&specialStatementContext);

        FOR_EACH_DLINK_NODE(&Set->members, struct _cloIR_BASE, member) {
            if (Set->funcName != gcvNULL) {
                if (cloIR_BASE_HasReturn(Compiler, member)) {
                    hasReturn = gcvTRUE;
                }
            }
            if (cloCOMPILER_OptimizationEnabled(Compiler, clvOPTIMIZATION_SPECIAL)) {
                status = cloIR_SET_TryToGenSpecialStatementCode(Compiler,
                                        CodeGenerator,
                                        Set,
                                        member,
                                        &specialStatementContext);
                if (gcmIS_ERROR(status)) return status;
                if (specialStatementContext.codeGenerated) continue;
            }

            clsGEN_CODE_PARAMETERS_Initialize(&memberParameters, gcvFALSE, gcvFALSE);

            status = cloIR_OBJECT_Accept(Compiler,
                             member,
                             &CodeGenerator->visitor,
                             &memberParameters);

            clsGEN_CODE_PARAMETERS_Finalize(&memberParameters);
            if (gcmIS_ERROR(status)) return status;
        }

        if (Set->funcName != gcvNULL) {
            status = _DefineFuncEnd(Compiler, CodeGenerator, Set, hasReturn);
            if (gcmIS_ERROR(status)) return status;
        }
        break;

    case clvEXPR_SET:
        gcmASSERT(0);
        break;

    default:
        gcmASSERT(0);
        return gcvSTATUS_INVALID_DATA;
    }

    return gcvSTATUS_OK;
}

static gceSTATUS
_DefineUnrolledIterationBegin(
IN cloCOMPILER Compiler,
IN cloCODE_GENERATOR CodeGenerator,
IN clsNAME * LoopIndexName,
OUT clsITERATION_CONTEXT * CurrentIterationContext
)
{
    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_OBJECT(CodeGenerator, clvOBJ_CODE_GENERATOR);
    gcmASSERT(LoopIndexName);
    gcmASSERT(CurrentIterationContext);

    CurrentIterationContext->prevContext    = CodeGenerator->currentIterationContext;
    CodeGenerator->currentIterationContext    = CurrentIterationContext;

    CurrentIterationContext->isUnrolled    = gcvTRUE;
    CurrentIterationContext->u.unrolledInfo.loopIndexName    = LoopIndexName;
    CurrentIterationContext->endLabel    = clNewLabel(Compiler);

    return gcvSTATUS_OK;
}

static gceSTATUS
_DefineUnrolledIterationEnd(
IN cloCOMPILER Compiler,
IN cloCODE_GENERATOR CodeGenerator
)
{
    gceSTATUS    status;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_OBJECT(CodeGenerator, clvOBJ_CODE_GENERATOR);

    gcmASSERT(CodeGenerator->currentIterationContext);
    gcmASSERT(CodeGenerator->currentIterationContext->isUnrolled);

    status = clSetLabel(Compiler,
                0,
                0,
                CodeGenerator->currentIterationContext->endLabel);
    if (gcmIS_ERROR(status)) return status;

    CodeGenerator->currentIterationContext = CodeGenerator->currentIterationContext->prevContext;
    return gcvSTATUS_OK;
}

static gceSTATUS
_DefineUnrolledIterationBodyBegin(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cluCONSTANT_VALUE LoopIndexValue
    )
{
    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_OBJECT(CodeGenerator, clvOBJ_CODE_GENERATOR);

    gcmASSERT(CodeGenerator->currentIterationContext);
    gcmASSERT(CodeGenerator->currentIterationContext->isUnrolled);

    CodeGenerator->currentIterationContext->u.unrolledInfo.loopIndexValue    = LoopIndexValue;
    CodeGenerator->currentIterationContext->u.unrolledInfo.bodyEndLabel    = clNewLabel(Compiler);

    return gcvSTATUS_OK;
}

static gceSTATUS
_DefineUnrolledIterationBodyEnd(
IN cloCOMPILER Compiler,
IN cloCODE_GENERATOR CodeGenerator
)
{
    gceSTATUS status;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_OBJECT(CodeGenerator, clvOBJ_CODE_GENERATOR);

    gcmASSERT(CodeGenerator->currentIterationContext);
    gcmASSERT(CodeGenerator->currentIterationContext->isUnrolled);

    status = clSetLabel(Compiler,
                0,
                0,
                CodeGenerator->currentIterationContext->u.unrolledInfo.bodyEndLabel);

    if (gcmIS_ERROR(status)) return status;
    return gcvSTATUS_OK;
}

static gceSTATUS
_CloneIterationContextForSwitch(
IN cloCOMPILER Compiler,
IN cloCODE_GENERATOR CodeGenerator,
OUT clsITERATION_CONTEXT * CurrentIterationContext
)
{
/* Verify the arguments. */
   clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
   clmVERIFY_OBJECT(CodeGenerator, clvOBJ_CODE_GENERATOR);
   gcmASSERT(CurrentIterationContext);

   if(CodeGenerator->currentIterationContext) {
       *CurrentIterationContext = *CodeGenerator->currentIterationContext;
   }
   else {
       gcoOS_ZeroMemory(CurrentIterationContext, gcmSIZEOF(clsITERATION_CONTEXT));
   }
   CurrentIterationContext->prevContext    = CodeGenerator->currentIterationContext;
   CodeGenerator->currentIterationContext    = CurrentIterationContext;

   CurrentIterationContext->endLabel = clNewLabel(Compiler);

   return gcvSTATUS_OK;
}

static gceSTATUS
_DefineIterationBegin(
IN cloCOMPILER Compiler,
IN cloCODE_GENERATOR CodeGenerator,
IN gctBOOL IsTestFirst,
IN gctBOOL HasRestExpr,
OUT clsITERATION_CONTEXT * CurrentIterationContext
)
{
    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_OBJECT(CodeGenerator, clvOBJ_CODE_GENERATOR);
    gcmASSERT(CurrentIterationContext);

    CurrentIterationContext->prevContext    = CodeGenerator->currentIterationContext;
    CodeGenerator->currentIterationContext    = CurrentIterationContext;

    CurrentIterationContext->isUnrolled    = gcvFALSE;

    CurrentIterationContext->u.genericInfo.isTestFirst    = IsTestFirst;
    CurrentIterationContext->u.genericInfo.hasRestExpr    = HasRestExpr;
    CurrentIterationContext->u.genericInfo.loopBeginLabel    = clNewLabel(Compiler);

    if (HasRestExpr) {
        CurrentIterationContext->u.genericInfo.restLabel    = clNewLabel(Compiler);
    }

    CurrentIterationContext->endLabel    = clNewLabel(Compiler);

    return gcvSTATUS_OK;
}

static gceSTATUS
_DefineIterationEnd(
IN cloCOMPILER Compiler,
IN cloCODE_GENERATOR CodeGenerator
)
{
    gceSTATUS    status;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_OBJECT(CodeGenerator, clvOBJ_CODE_GENERATOR);

    gcmASSERT(CodeGenerator->currentIterationContext);
    gcmASSERT(!CodeGenerator->currentIterationContext->isUnrolled);

    status = clSetLabel(Compiler,
                0,
                0,
                CodeGenerator->currentIterationContext->endLabel);

    if (gcmIS_ERROR(status)) return status;
    CodeGenerator->currentIterationContext = CodeGenerator->currentIterationContext->prevContext;

    return gcvSTATUS_OK;
}

#define _DefineSwitchEnd(Compiler, CodeGen)  _DefineIterationEnd(Compiler, CodeGen)

static gceSTATUS
_DefineIterationBodyBegin(
IN cloCOMPILER Compiler,
IN cloCODE_GENERATOR CodeGenerator
)
{
    gceSTATUS    status;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_OBJECT(CodeGenerator, clvOBJ_CODE_GENERATOR);

    gcmASSERT(CodeGenerator->currentIterationContext);
    gcmASSERT(!CodeGenerator->currentIterationContext->isUnrolled);

    if (!CodeGenerator->currentIterationContext->u.genericInfo.hasRestExpr) {
        status = clSetLabel(Compiler,
                    0,
                    0,
                    CodeGenerator->currentIterationContext->u.genericInfo.loopBeginLabel);

        if (gcmIS_ERROR(status)) return status;
    }

    return gcvSTATUS_OK;
}

static gceSTATUS
_DefineIterationBodyEnd(
IN cloCOMPILER Compiler,
IN cloCODE_GENERATOR CodeGenerator
)
{
    gceSTATUS    status;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_OBJECT(CodeGenerator, clvOBJ_CODE_GENERATOR);

    gcmASSERT(CodeGenerator->currentIterationContext);
    gcmASSERT(!CodeGenerator->currentIterationContext->isUnrolled);

    if (CodeGenerator->currentIterationContext->u.genericInfo.isTestFirst) {
        status = clEmitAlwaysBranchCode(Compiler,
                        0,
                        0,
                        clvOPCODE_JUMP,
                        CodeGenerator->currentIterationContext->
                        u.genericInfo.loopBeginLabel);
        if (gcmIS_ERROR(status)) return status;
    }

    return gcvSTATUS_OK;
}

static gctLABEL
_GetIterationContinueLabel(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator
    )
{
    /* Verify the arguments. */
    clmASSERT_OBJECT(Compiler, clvOBJ_COMPILER);
    clmASSERT_OBJECT(CodeGenerator, clvOBJ_CODE_GENERATOR);

    gcmASSERT(CodeGenerator->currentIterationContext);

    if (CodeGenerator->currentIterationContext->isUnrolled)
    {
        return CodeGenerator->currentIterationContext->u.unrolledInfo.bodyEndLabel;
    }
    else
    {
#if _GEN_FOR_BETTER_LOOP_RECOGNITION
        if (CodeGenerator->currentIterationContext->u.genericInfo.hasRestExpr)
        {
            return CodeGenerator->currentIterationContext->u.genericInfo.restLabel;
        }
        else
        {
            return CodeGenerator->currentIterationContext->u.genericInfo.loopBeginLabel;
        }
#else
        return CodeGenerator->currentIterationContext->u.genericInfo.loopBeginLabel;
#endif
    }
}

static gctLABEL
_GetIterationEndLabel(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator
    )
{
    /* Verify the arguments. */
    clmASSERT_OBJECT(Compiler, clvOBJ_COMPILER);
    clmASSERT_OBJECT(CodeGenerator, clvOBJ_CODE_GENERATOR);

    gcmASSERT(CodeGenerator->currentIterationContext);

    return CodeGenerator->currentIterationContext->endLabel;
}

static gctLABEL
_GetIterationLoopBeginLabel(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator
    )
{
    /* Verify the arguments. */
    clmASSERT_OBJECT(Compiler, clvOBJ_COMPILER);
    clmASSERT_OBJECT(CodeGenerator, clvOBJ_CODE_GENERATOR);

    gcmASSERT(CodeGenerator->currentIterationContext);

    return CodeGenerator->currentIterationContext->u.genericInfo.loopBeginLabel;
}

static gctBOOL
_IsUnrolledLoopIndexRecursively(
    IN cloCOMPILER Compiler,
    IN clsITERATION_CONTEXT * IterationContext,
    IN clsNAME * Name,
    OUT cluCONSTANT_VALUE * UnrolledLoopIndexValue
    )
{
    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    gcmASSERT(IterationContext);
    gcmASSERT(Name);
    gcmASSERT(UnrolledLoopIndexValue);

    if (IterationContext->isUnrolled && Name == IterationContext->u.unrolledInfo.loopIndexName)
    {
        *UnrolledLoopIndexValue = IterationContext->u.unrolledInfo.loopIndexValue;
        return gcvTRUE;
    }

    if (IterationContext->prevContext == gcvNULL)
    {
        return gcvFALSE;
    }
    else
    {
        return _IsUnrolledLoopIndexRecursively(
                                            Compiler,
                                            IterationContext->prevContext,
                                            Name,
                                            UnrolledLoopIndexValue);
    }
}

static gctBOOL
_IsUnrolledLoopIndex(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN clsNAME * Name,
    OUT cluCONSTANT_VALUE * UnrolledLoopIndexValue
    )
{
    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_OBJECT(CodeGenerator, clvOBJ_CODE_GENERATOR);
    gcmASSERT(Name);
    gcmASSERT(UnrolledLoopIndexValue);

    if (CodeGenerator->currentIterationContext == gcvNULL) return gcvFALSE;

    return _IsUnrolledLoopIndexRecursively(
                                        Compiler,
                                        CodeGenerator->currentIterationContext,
                                        Name,
                                        UnrolledLoopIndexValue);
}

typedef struct _clsITERATION_UNROLL_INFO
{
    gctBOOL                unrollable;

    clsNAME *            loopIndexName;

    cleCONDITION        condition;

    cluCONSTANT_VALUE    conditionConstantValue;

    cluCONSTANT_VALUE    initialConstantValue;

    cluCONSTANT_VALUE    incrementConstantValue;

    gctUINT                loopCount;
}
clsITERATION_UNROLL_INFO;

static gceSTATUS
_CheckAsUnrollableCondition(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_BINARY_EXPR BinaryExpr,
    IN clsNAME_SPACE * ForSpace,
    OUT clsITERATION_UNROLL_INFO * IterationUnrollInfo
    )
{
    gceSTATUS                status;
    cloIR_CONSTANT            conditionConstant    = gcvNULL;
    clsGEN_CODE_PARAMETERS    conditionParameters;

    /* Verify the arguments. */
    clmVERIFY_IR_OBJECT(BinaryExpr, clvIR_BINARY_EXPR);
    gcmASSERT(ForSpace);
    gcmASSERT(IterationUnrollInfo);

    do
    {
        /* Check the loop index */
        if (cloIR_OBJECT_GetType(&BinaryExpr->leftOperand->base) != clvIR_VARIABLE) break;

        IterationUnrollInfo->loopIndexName = ((cloIR_VARIABLE)BinaryExpr->leftOperand)->name;

        if (IterationUnrollInfo->loopIndexName->mySpace != ForSpace) break;

        if (!clmDECL_IsInt(&IterationUnrollInfo->loopIndexName->decl)
            && !clmDECL_IsFloat(&IterationUnrollInfo->loopIndexName->decl))
        {
            break;
        }

        /* Check the relation operator */
        switch (BinaryExpr->type)
        {
        case clvBINARY_GREATER_THAN:
            IterationUnrollInfo->condition = clvCONDITION_GREATER_THAN;
            break;

        case clvBINARY_LESS_THAN:
            IterationUnrollInfo->condition = clvCONDITION_LESS_THAN;
            break;

        case clvBINARY_GREATER_THAN_EQUAL:
            IterationUnrollInfo->condition = clvCONDITION_GREATER_THAN_EQUAL;
            break;

        case clvBINARY_LESS_THAN_EQUAL:
            IterationUnrollInfo->condition = clvCONDITION_LESS_THAN_EQUAL;
            break;

        case clvBINARY_EQUAL:
            IterationUnrollInfo->condition = clvCONDITION_EQUAL;
            break;

        case clvBINARY_NOT_EQUAL:
            IterationUnrollInfo->condition = clvCONDITION_NOT_EQUAL;
            break;

        default:
            gcmASSERT(0);
            IterationUnrollInfo->condition = clvCONDITION_INVALID;
        }

        if (IterationUnrollInfo->condition == clvCONDITION_INVALID) break;

        /* Try to evaluate the condition operand */
        gcmASSERT(BinaryExpr->rightOperand);

        clsGEN_CODE_PARAMETERS_Initialize(
                                        &conditionParameters,
                                        gcvFALSE,
                                        gcvTRUE);

        conditionParameters.hint = clvEVALUATE_ONLY;

        status = cloIR_OBJECT_Accept(
                                    Compiler,
                                    &BinaryExpr->rightOperand->base,
                                    &CodeGenerator->visitor,
                                    &conditionParameters);

        if (gcmIS_ERROR(status)) return status;

        conditionConstant                = conditionParameters.constant;
        conditionParameters.constant    = gcvNULL;

        clsGEN_CODE_PARAMETERS_Finalize(&conditionParameters);

        if (conditionConstant == gcvNULL) break;

        if (!clmDECL_IsInt(&conditionConstant->exprBase.decl)
            && !clmDECL_IsFloat(&conditionConstant->exprBase.decl))
        {
            break;
        }

        IterationUnrollInfo->conditionConstantValue = conditionConstant->values[0];

        /* OK */
        IterationUnrollInfo->unrollable = gcvTRUE;

        gcmVERIFY_OK(cloIR_OBJECT_Destroy(Compiler, &conditionConstant->exprBase.base));

        return gcvSTATUS_OK;
    }
    while (gcvFALSE);

    IterationUnrollInfo->unrollable = gcvFALSE;

    if (conditionConstant != gcvNULL)
    {
        gcmVERIFY_OK(cloIR_OBJECT_Destroy(Compiler, &conditionConstant->exprBase.base));
    }

    return gcvSTATUS_OK;
}

static gceSTATUS
_CheckAsUnrollableInitializer(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_BINARY_EXPR BinaryExpr,
    IN OUT clsITERATION_UNROLL_INFO * IterationUnrollInfo
    )
{
    gceSTATUS                status;
    cloIR_CONSTANT            initialConstant    = gcvNULL;
    clsGEN_CODE_PARAMETERS    initialParameters;

    /* Verify the arguments. */
    clmVERIFY_IR_OBJECT(BinaryExpr, clvIR_BINARY_EXPR);
    gcmASSERT(IterationUnrollInfo);

    do
    {
        /* Check the loop index */
        if (cloIR_OBJECT_GetType(&BinaryExpr->leftOperand->base) != clvIR_VARIABLE) break;

        if (((cloIR_VARIABLE)BinaryExpr->leftOperand)->name != IterationUnrollInfo->loopIndexName)
        {
            break;
        }

        /* Check the relation operator */
        if (BinaryExpr->type != clvBINARY_ASSIGN) break;

        /* Try to evaluate the initial operand */
        gcmASSERT(BinaryExpr->rightOperand);

        clsGEN_CODE_PARAMETERS_Initialize(
                                        &initialParameters,
                                        gcvFALSE,
                                        gcvTRUE);

        initialParameters.hint = clvEVALUATE_ONLY;

        status = cloIR_OBJECT_Accept(
                                    Compiler,
                                    &BinaryExpr->rightOperand->base,
                                    &CodeGenerator->visitor,
                                    &initialParameters);

        if (gcmIS_ERROR(status)) return status;

        initialConstant                = initialParameters.constant;
        initialParameters.constant    = gcvNULL;

        clsGEN_CODE_PARAMETERS_Finalize(&initialParameters);

        if (initialConstant == gcvNULL) break;

        if (!clmDECL_IsInt(&initialConstant->exprBase.decl)
            && !clmDECL_IsFloat(&initialConstant->exprBase.decl))
        {
            break;
        }

        IterationUnrollInfo->initialConstantValue = initialConstant->values[0];

        /* OK */
        IterationUnrollInfo->unrollable = gcvTRUE;

        gcmVERIFY_OK(cloIR_OBJECT_Destroy(Compiler, &initialConstant->exprBase.base));

        return gcvSTATUS_OK;
    }
    while (gcvFALSE);

    IterationUnrollInfo->unrollable = gcvFALSE;

    if (initialConstant != gcvNULL)
    {
        gcmVERIFY_OK(cloIR_OBJECT_Destroy(Compiler, &initialConstant->exprBase.base));
    }

    return gcvSTATUS_OK;
}

static gceSTATUS
_CheckAsUnrollableRestExpr1(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_UNARY_EXPR UnaryExpr,
    IN OUT clsITERATION_UNROLL_INFO * IterationUnrollInfo
    )
{
    /* Verify the arguments. */
    clmVERIFY_IR_OBJECT(UnaryExpr, clvIR_UNARY_EXPR);
    gcmASSERT(IterationUnrollInfo);

    do
    {
        /* Check the loop index */
        if (cloIR_OBJECT_GetType(&UnaryExpr->operand->base) != clvIR_VARIABLE) break;

        if (((cloIR_VARIABLE)&UnaryExpr->operand->base)->name != IterationUnrollInfo->loopIndexName)
        {
            break;
        }

        /* Check the relation operator */
        if (UnaryExpr->type == clvUNARY_POST_INC || UnaryExpr->type == clvUNARY_PRE_INC)
        {
            if (clmDECL_IsInt(&IterationUnrollInfo->loopIndexName->decl))
            {
                IterationUnrollInfo->incrementConstantValue.longValue = 1;
            }
            else
            {
                gcmASSERT(clmDECL_IsFloat(&IterationUnrollInfo->loopIndexName->decl));

                IterationUnrollInfo->incrementConstantValue.floatValue = (gctFLOAT)1.0;
            }
        }
        else if (UnaryExpr->type == clvUNARY_POST_DEC || UnaryExpr->type == clvUNARY_PRE_DEC)
        {
            if (clmDECL_IsInt(&IterationUnrollInfo->loopIndexName->decl))
            {
                IterationUnrollInfo->incrementConstantValue.longValue = -1;
            }
            else
            {
                gcmASSERT(clmDECL_IsFloat(&IterationUnrollInfo->loopIndexName->decl));

                IterationUnrollInfo->incrementConstantValue.floatValue = (gctFLOAT)-1.0;
            }
        }
        else
        {
            break;
        }

        /* OK */
        IterationUnrollInfo->unrollable = gcvTRUE;

        return gcvSTATUS_OK;
    }
    while (gcvFALSE);

    IterationUnrollInfo->unrollable = gcvFALSE;

    return gcvSTATUS_OK;
}

static gceSTATUS
_CheckAsUnrollableRestExpr2(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_BINARY_EXPR BinaryExpr,
    IN OUT clsITERATION_UNROLL_INFO * IterationUnrollInfo
    )
{
    gceSTATUS                status;
    cloIR_CONSTANT            incrementConstant    = gcvNULL;
    clsGEN_CODE_PARAMETERS    incrementParameters;

    /* Verify the arguments. */
    clmVERIFY_IR_OBJECT(BinaryExpr, clvIR_BINARY_EXPR);
    gcmASSERT(IterationUnrollInfo);

    do
    {
        /* Check the loop index */
        if (cloIR_OBJECT_GetType(&BinaryExpr->leftOperand->base) != clvIR_VARIABLE) break;

        if (((cloIR_VARIABLE)BinaryExpr->leftOperand)->name != IterationUnrollInfo->loopIndexName)
        {
            break;
        }

        /* Try to evaluate the increment operand */
        gcmASSERT(BinaryExpr->rightOperand);

        clsGEN_CODE_PARAMETERS_Initialize(
                                        &incrementParameters,
                                        gcvFALSE,
                                        gcvTRUE);

        incrementParameters.hint = clvEVALUATE_ONLY;

        status = cloIR_OBJECT_Accept(
                                    Compiler,
                                    &BinaryExpr->rightOperand->base,
                                    &CodeGenerator->visitor,
                                    &incrementParameters);

        if (gcmIS_ERROR(status)) return status;

        incrementConstant                = incrementParameters.constant;
        incrementParameters.constant    = gcvNULL;

        clsGEN_CODE_PARAMETERS_Finalize(&incrementParameters);

        if (incrementConstant == gcvNULL) break;

        if (!clmDECL_IsInt(&incrementConstant->exprBase.decl)
            && !clmDECL_IsFloat(&incrementConstant->exprBase.decl))
        {
            break;
        }

        IterationUnrollInfo->incrementConstantValue = incrementConstant->values[0];

        /* Check the operator */
        if (BinaryExpr->type == clvBINARY_ADD_ASSIGN)
        {
            /* Nothing to do */
        }
        else if (BinaryExpr->type == clvBINARY_SUB_ASSIGN)
        {
            if (clmDECL_IsInt(&IterationUnrollInfo->loopIndexName->decl))
            {
                IterationUnrollInfo->incrementConstantValue.longValue =
                        -IterationUnrollInfo->incrementConstantValue.intValue;
            }
            else
            {
                gcmASSERT(clmDECL_IsFloat(&IterationUnrollInfo->loopIndexName->decl));

                IterationUnrollInfo->incrementConstantValue.floatValue =
                        -IterationUnrollInfo->incrementConstantValue.floatValue;
            }
        }
        else
        {
            break;
        }

        /* OK */
        IterationUnrollInfo->unrollable = gcvTRUE;

        gcmVERIFY_OK(cloIR_OBJECT_Destroy(Compiler, &incrementConstant->exprBase.base));

        return gcvSTATUS_OK;
    }
    while (gcvFALSE);

    IterationUnrollInfo->unrollable = gcvFALSE;

    if (incrementConstant != gcvNULL)
    {
        gcmVERIFY_OK(cloIR_OBJECT_Destroy(Compiler, &incrementConstant->exprBase.base));
    }

    return gcvSTATUS_OK;
}

#define MAX_UNROLL_ITERATION_COUNT        3

static gceSTATUS
_CheckIterationCount(
    IN OUT clsITERATION_UNROLL_INFO * IterationUnrollInfo
    )
{
    gctUINT                i;
    cluCONSTANT_VALUE    loopIndexValue;
    gctBOOL                loopTestResult = gcvFALSE;

    loopIndexValue = IterationUnrollInfo->initialConstantValue;

    for (i = 0; gcvTRUE; i++)
    {
        /* Use the condition */
        if (clmDECL_IsInt(&IterationUnrollInfo->loopIndexName->decl))
        {
            switch (IterationUnrollInfo->condition)
            {
            case clvCONDITION_EQUAL:
                loopTestResult = (loopIndexValue.intValue
                                    == IterationUnrollInfo->conditionConstantValue.intValue);
                break;

            case clvCONDITION_NOT_EQUAL:
                loopTestResult = (loopIndexValue.intValue
                                    != IterationUnrollInfo->conditionConstantValue.intValue);
                break;

            case clvCONDITION_LESS_THAN:
                loopTestResult = (loopIndexValue.intValue
                                    < IterationUnrollInfo->conditionConstantValue.intValue);
                break;

            case clvCONDITION_LESS_THAN_EQUAL:
                loopTestResult = (loopIndexValue.intValue
                                    <= IterationUnrollInfo->conditionConstantValue.intValue);
                break;

            case clvCONDITION_GREATER_THAN:
                loopTestResult = (loopIndexValue.intValue
                                    > IterationUnrollInfo->conditionConstantValue.intValue);
                break;

            case clvCONDITION_GREATER_THAN_EQUAL:
                loopTestResult = (loopIndexValue.intValue
                                    >= IterationUnrollInfo->conditionConstantValue.intValue);
                break;

            default:
                gcmASSERT(0);
            }
        }
        else
        {
            gcmASSERT(clmDECL_IsFloat(&IterationUnrollInfo->loopIndexName->decl));

            switch (IterationUnrollInfo->condition)
            {
            case clvCONDITION_EQUAL:
                loopTestResult = (loopIndexValue.floatValue
                                    == IterationUnrollInfo->conditionConstantValue.floatValue);
                break;

            case clvCONDITION_NOT_EQUAL:
                loopTestResult = (loopIndexValue.floatValue
                                    != IterationUnrollInfo->conditionConstantValue.floatValue);
                break;

            case clvCONDITION_LESS_THAN:
                loopTestResult = (loopIndexValue.floatValue
                                    < IterationUnrollInfo->conditionConstantValue.floatValue);
                break;

            case clvCONDITION_LESS_THAN_EQUAL:
                loopTestResult = (loopIndexValue.floatValue
                                    <= IterationUnrollInfo->conditionConstantValue.floatValue);
                break;

            case clvCONDITION_GREATER_THAN:
                loopTestResult = (loopIndexValue.floatValue
                                    > IterationUnrollInfo->conditionConstantValue.floatValue);
                break;

            case clvCONDITION_GREATER_THAN_EQUAL:
                loopTestResult = (loopIndexValue.floatValue
                                    >= IterationUnrollInfo->conditionConstantValue.floatValue);
                break;

            default:
                gcmASSERT(0);
            }
        }

        if (!loopTestResult) break;

        /* Check the count */
        if (i >= MAX_UNROLL_ITERATION_COUNT)
        {
            IterationUnrollInfo->unrollable = gcvFALSE;

            return gcvSTATUS_OK;
        }

        /* Use the increment */
        if (clmDECL_IsInt(&IterationUnrollInfo->loopIndexName->decl))
        {
            loopIndexValue.intValue += IterationUnrollInfo->incrementConstantValue.intValue;
        }
        else
        {
            gcmASSERT(clmDECL_IsFloat(&IterationUnrollInfo->loopIndexName->decl));

            loopIndexValue.floatValue += IterationUnrollInfo->incrementConstantValue.floatValue;
        }
    }

    /* OK */
    IterationUnrollInfo->loopCount    = i;

    IterationUnrollInfo->unrollable = gcvTRUE;

    return gcvSTATUS_OK;
}

gceSTATUS
cloIR_ITERATION_GenUnrolledCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_ITERATION Iteration,
    IN clsITERATION_UNROLL_INFO * IterationUnrollInfo,
    IN OUT clsGEN_CODE_PARAMETERS * Parameters
    )
{
    gceSTATUS                status;
    gctUINT                    i;
    clsGEN_CODE_PARAMETERS    loopBodyParameters;
    clsITERATION_CONTEXT    iterationContext;
    cluCONSTANT_VALUE        unrolledLoopIndexValue;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(Iteration, clvIR_ITERATION);
    gcmASSERT(IterationUnrollInfo);
    gcmASSERT(Parameters);
    gcmASSERT(!Parameters->needLOperand && !Parameters->needROperand);

    if (Iteration->loopBody == gcvNULL) return gcvSTATUS_OK;

    status = _DefineUnrolledIterationBegin(
                                        Compiler,
                                        CodeGenerator,
                                        IterationUnrollInfo->loopIndexName,
                                        &iterationContext);

    if (gcmIS_ERROR(status)) return status;

    unrolledLoopIndexValue    = IterationUnrollInfo->initialConstantValue;

    for (i = 0; i < IterationUnrollInfo->loopCount; i++)
    {
        status = _DefineUnrolledIterationBodyBegin(
                                                Compiler,
                                                CodeGenerator,
                                                unrolledLoopIndexValue);

        if (gcmIS_ERROR(status)) return status;

        /* Generate the code of the loop body */
        clsGEN_CODE_PARAMETERS_Initialize(&loopBodyParameters, gcvFALSE, gcvFALSE);

        status = cloIR_OBJECT_Accept(
                                    Compiler,
                                    Iteration->loopBody,
                                    &CodeGenerator->visitor,
                                    &loopBodyParameters);

        clsGEN_CODE_PARAMETERS_Finalize(&loopBodyParameters);

        if (gcmIS_ERROR(status)) return status;

        status = _DefineUnrolledIterationBodyEnd(Compiler,
                            CodeGenerator);

        if (gcmIS_ERROR(status)) return status;

        /* Use the increment */
        if (clmDECL_IsInt(&IterationUnrollInfo->loopIndexName->decl))
        {
            unrolledLoopIndexValue.intValue +=
                    IterationUnrollInfo->incrementConstantValue.intValue;
        }
        else
        {
            gcmASSERT(clmDECL_IsFloat(&IterationUnrollInfo->loopIndexName->decl));

            unrolledLoopIndexValue.floatValue +=
                    IterationUnrollInfo->incrementConstantValue.floatValue;
        }
    }

    status = _DefineUnrolledIterationEnd(
                                        Compiler,
                                        CodeGenerator);

    if (gcmIS_ERROR(status)) return status;

    return gcvSTATUS_OK;
}

gceSTATUS
cloIR_ITERATION_TryToGenUnrolledCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_ITERATION Iteration,
    IN OUT clsGEN_CODE_PARAMETERS * Parameters,
    OUT gctBOOL * IsUnrolled
    )
{
    gceSTATUS                    status = gcvSTATUS_OK;
    clsITERATION_UNROLL_INFO    iterationUnrollInfo;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(Iteration, clvIR_ITERATION);
    gcmASSERT(Parameters);
    gcmASSERT(IsUnrolled);

    do
    {
        if (Iteration->type != clvFOR) break;

        gcmASSERT(Iteration->forSpace);

        /* Check the condition part */
        if (Iteration->condExpr == gcvNULL
            || cloIR_OBJECT_GetType(&Iteration->condExpr->base) != clvIR_BINARY_EXPR) break;

        status = _CheckAsUnrollableCondition(
                                            Compiler,
                                            CodeGenerator,
                                            (cloIR_BINARY_EXPR)Iteration->condExpr,
                                            Iteration->forSpace,
                                            &iterationUnrollInfo);

        if (gcmIS_ERROR(status)) return status;

        if (!iterationUnrollInfo.unrollable) break;

        /* Check the initial part */
        if (Iteration->forInitStatement == gcvNULL
            || cloIR_OBJECT_GetType(Iteration->forInitStatement) != clvIR_BINARY_EXPR) break;

        status = _CheckAsUnrollableInitializer(
                                            Compiler,
                                            CodeGenerator,
                                            (cloIR_BINARY_EXPR)Iteration->forInitStatement,
                                            &iterationUnrollInfo);

        if (gcmIS_ERROR(status)) return status;

        if (!iterationUnrollInfo.unrollable) break;

        /* Check the rest part */
        if (Iteration->forRestExpr == gcvNULL) break;

        if (cloIR_OBJECT_GetType(&Iteration->forRestExpr->base) == clvIR_UNARY_EXPR)
        {
            status = _CheckAsUnrollableRestExpr1(
                                                Compiler,
                                                CodeGenerator,
                                                (cloIR_UNARY_EXPR)Iteration->forRestExpr,
                                                &iterationUnrollInfo);

            if (gcmIS_ERROR(status)) return status;

            if (!iterationUnrollInfo.unrollable) break;
        }
        else if (cloIR_OBJECT_GetType(&Iteration->forRestExpr->base) == clvIR_BINARY_EXPR)
        {
            status = _CheckAsUnrollableRestExpr2(
                                                Compiler,
                                                CodeGenerator,
                                                (cloIR_BINARY_EXPR)Iteration->forRestExpr,
                                                &iterationUnrollInfo);

            if (gcmIS_ERROR(status)) return status;

            if (!iterationUnrollInfo.unrollable) break;
        }
        else
        {
            break;
        }

        /* TODO: Check the loop body */

        /* Check the iteration count */
        status = _CheckIterationCount(&iterationUnrollInfo);

        if (gcmIS_ERROR(status)) return status;

        if (!iterationUnrollInfo.unrollable) break;

        /* Generate the unrolled code */
        status = cloIR_ITERATION_GenUnrolledCode(
                                                Compiler,
                                                CodeGenerator,
                                                Iteration,
                                                &iterationUnrollInfo,
                                                Parameters);

        if (gcmIS_ERROR(status)) return status;

        /* OK */
        *IsUnrolled = gcvTRUE;

        return gcvSTATUS_OK;
    }
    while (gcvFALSE);

    *IsUnrolled = gcvFALSE;

    return status;
}

#if _GEN_FOR_BETTER_LOOP_RECOGNITION
static gceSTATUS
_DefineIterationRestExprBegin(
IN cloCOMPILER Compiler,
IN cloCODE_GENERATOR CodeGenerator
)
{
    gceSTATUS    status;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_OBJECT(CodeGenerator, clvOBJ_CODE_GENERATOR);

    gcmASSERT(CodeGenerator->currentIterationContext);
    gcmASSERT(!CodeGenerator->currentIterationContext->isUnrolled);
    gcmASSERT(CodeGenerator->currentIterationContext->u.genericInfo.hasRestExpr);

    status = clSetLabel(Compiler,
                        0,
                        0,
                        CodeGenerator->currentIterationContext->u.genericInfo.restLabel);

    if (gcmIS_ERROR(status)) return status;

    return gcvSTATUS_OK;
}

gceSTATUS
cloIR_ITERATION_GenForCode(
IN cloCOMPILER Compiler,
IN cloCODE_GENERATOR CodeGenerator,
IN cloIR_ITERATION Iteration,
IN OUT clsGEN_CODE_PARAMETERS * Parameters
)
{
    gceSTATUS        status;
    clsITERATION_CONTEXT    iterationContext;
    clsGEN_CODE_PARAMETERS    initParameters, restParameters, bodyParameters;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(Iteration, clvIR_ITERATION);
    gcmASSERT(Parameters);
    gcmASSERT(Iteration->type == clvFOR);

    /* The init part */
    if (Iteration->forInitStatement != gcvNULL) {
        clsGEN_CODE_PARAMETERS_Initialize(&initParameters, gcvFALSE, gcvFALSE);

        status = cloIR_OBJECT_Accept(Compiler,
                                     Iteration->forInitStatement,
                                     &CodeGenerator->visitor,
                                     &initParameters);
        if (gcmIS_ERROR(status)) return status;

        clsGEN_CODE_PARAMETERS_Finalize(&initParameters);
    }

    status = _DefineIterationBegin(Compiler,
                                   CodeGenerator,
                                   gcvFALSE,
                                   (Iteration->forRestExpr != gcvNULL),
                                   &iterationContext);
    if (gcmIS_ERROR(status)) return status;

    /* The condition part, first time into loop */
    if (Iteration->condExpr != gcvNULL) {
        status = _GenConditionCode(Compiler,
                                   CodeGenerator,
                                   Iteration->condExpr,
                                   _GetIterationEndLabel(Compiler, CodeGenerator),
                                   gcvFALSE);
        if (gcmIS_ERROR(status)) return status;
    }

    /* The loop body: body begin label */
    status = clSetLabel(Compiler,
                        0,
                        0,
                        CodeGenerator->currentIterationContext->u.genericInfo.loopBeginLabel);
    if (gcmIS_ERROR(status)) return status;

    /* The body part */
    if (Iteration->loopBody != gcvNULL) {
        clsGEN_CODE_PARAMETERS_Initialize(&bodyParameters, gcvFALSE, gcvFALSE);
        status = cloIR_OBJECT_Accept(Compiler,
                                     Iteration->loopBody,
                                     &CodeGenerator->visitor,
                                     &bodyParameters);
        if (gcmIS_ERROR(status)) return status;

        clsGEN_CODE_PARAMETERS_Finalize(&bodyParameters);
    }

    /* The rest part */
    if (Iteration->forRestExpr != gcvNULL) {
        status = _DefineIterationRestExprBegin(Compiler, CodeGenerator);
        if (gcmIS_ERROR(status)) return status;

        clsGEN_CODE_PARAMETERS_Initialize(&restParameters, gcvFALSE, gcvFALSE);

        status = cloIR_OBJECT_Accept(Compiler,
                                     &Iteration->forRestExpr->base,
                                     &CodeGenerator->visitor,
                                     &restParameters);
        if (gcmIS_ERROR(status)) return status;
    }

    /* The condition part if exists, check to loop back */
    if (Iteration->condExpr != gcvNULL) {
        status = _GenConditionCode(Compiler,
                                   CodeGenerator,
                                   Iteration->condExpr,
                                   _GetIterationLoopBeginLabel(Compiler, CodeGenerator),
                                   gcvTRUE);
        if (gcmIS_ERROR(status)) return status;
    }
    else {
        status = clEmitAlwaysBranchCode(Compiler,
                                    Iteration->loopBody->lineNo,
                                    Iteration->loopBody->stringNo,
                                    clvOPCODE_JUMP,
                                    _GetIterationLoopBeginLabel(Compiler, CodeGenerator));
        if (gcmIS_ERROR(status)) return status;
    }

    status = _DefineIterationEnd(Compiler,
                                 CodeGenerator);
    if (gcmIS_ERROR(status)) return status;
    return gcvSTATUS_OK;
}
#else
static gceSTATUS
_DefineIterationRestExprBegin(
IN cloCOMPILER Compiler,
IN cloCODE_GENERATOR CodeGenerator
)
{
    gceSTATUS    status;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_OBJECT(CodeGenerator, clvOBJ_CODE_GENERATOR);

    gcmASSERT(CodeGenerator->currentIterationContext);
    gcmASSERT(!CodeGenerator->currentIterationContext->isUnrolled);
    gcmASSERT(CodeGenerator->currentIterationContext->u.genericInfo.hasRestExpr);

    status = clEmitAlwaysBranchCode(Compiler,
                    0,
                    0,
                    clvOPCODE_JUMP,
                    CodeGenerator->currentIterationContext->
                    u.genericInfo.restLabel);

    if (gcmIS_ERROR(status)) return status;

    status = clSetLabel(Compiler,
                0,
                0,
                CodeGenerator->currentIterationContext->u.genericInfo.loopBeginLabel);

    if (gcmIS_ERROR(status)) return status;

    return gcvSTATUS_OK;
}

static gceSTATUS
_DefineIterationRestExprEnd(
IN cloCOMPILER Compiler,
IN cloCODE_GENERATOR CodeGenerator
)
{
    gceSTATUS    status;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_OBJECT(CodeGenerator, clvOBJ_CODE_GENERATOR);

    gcmASSERT(CodeGenerator->currentIterationContext);
    gcmASSERT(!CodeGenerator->currentIterationContext->isUnrolled);
    gcmASSERT(CodeGenerator->currentIterationContext->u.genericInfo.hasRestExpr);

    status = clSetLabel(Compiler,
                        0,
                        0,
                        CodeGenerator->currentIterationContext->u.genericInfo.restLabel);

    if (gcmIS_ERROR(status)) return status;

    return gcvSTATUS_OK;
}

gceSTATUS
cloIR_ITERATION_GenForCode(
IN cloCOMPILER Compiler,
IN cloCODE_GENERATOR CodeGenerator,
IN cloIR_ITERATION Iteration,
IN OUT clsGEN_CODE_PARAMETERS * Parameters
)
{
    gceSTATUS        status;
    clsITERATION_CONTEXT    iterationContext;
    clsGEN_CODE_PARAMETERS    initParameters, restParameters, bodyParameters;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(Iteration, clvIR_ITERATION);
    gcmASSERT(Parameters);
    gcmASSERT(Iteration->type == clvFOR);

    /* The init part */
    if (Iteration->forInitStatement != gcvNULL) {
        clsGEN_CODE_PARAMETERS_Initialize(&initParameters, gcvFALSE, gcvFALSE);

        status = cloIR_OBJECT_Accept(Compiler,
                         Iteration->forInitStatement,
                         &CodeGenerator->visitor,
                         &initParameters);
        if (gcmIS_ERROR(status)) return status;

        clsGEN_CODE_PARAMETERS_Finalize(&initParameters);
    }

    status = _DefineIterationBegin(Compiler,
                       CodeGenerator,
                       gcvTRUE,
                       (Iteration->forRestExpr != gcvNULL),
                       &iterationContext);
    if (gcmIS_ERROR(status)) return status;

    /* The rest part */
    if (Iteration->forRestExpr != gcvNULL) {
        status = _DefineIterationRestExprBegin(Compiler, CodeGenerator);
        if (gcmIS_ERROR(status)) return status;

        clsGEN_CODE_PARAMETERS_Initialize(&restParameters, gcvFALSE, gcvFALSE);

        status = cloIR_OBJECT_Accept(Compiler,
                         &Iteration->forRestExpr->base,
                         &CodeGenerator->visitor,
                         &restParameters);
        if (gcmIS_ERROR(status)) return status;

        status = _DefineIterationRestExprEnd(Compiler, CodeGenerator);
        if (gcmIS_ERROR(status)) return status;
    }

    /* The loop body */
    status = _DefineIterationBodyBegin(Compiler, CodeGenerator);
    if (gcmIS_ERROR(status)) return status;

    /* The condition part */
    if (Iteration->condExpr != gcvNULL) {
        status = _GenConditionCode(Compiler,
                       CodeGenerator,
                       Iteration->condExpr,
                       _GetIterationEndLabel(Compiler, CodeGenerator),
                       gcvFALSE);
        if (gcmIS_ERROR(status)) return status;
    }

    /* The body part */
    if (Iteration->loopBody != gcvNULL) {
        clsGEN_CODE_PARAMETERS_Initialize(&bodyParameters, gcvFALSE, gcvFALSE);
        status = cloIR_OBJECT_Accept(Compiler,
                         Iteration->loopBody,
                         &CodeGenerator->visitor,
                         &bodyParameters);
        if (gcmIS_ERROR(status)) return status;

        clsGEN_CODE_PARAMETERS_Finalize(&bodyParameters);
    }

    status = _DefineIterationBodyEnd(Compiler, CodeGenerator);
    if (gcmIS_ERROR(status)) return status;

    status = _DefineIterationEnd(Compiler,
                     CodeGenerator);
    if (gcmIS_ERROR(status)) return status;
    return gcvSTATUS_OK;
}
#endif


gceSTATUS
cloIR_ITERATION_GenWhileCode(
IN cloCOMPILER Compiler,
IN cloCODE_GENERATOR CodeGenerator,
IN cloIR_ITERATION Iteration,
IN OUT clsGEN_CODE_PARAMETERS * Parameters
    )
{
    gceSTATUS        status;
    clsITERATION_CONTEXT    iterationContext;
    clsGEN_CODE_PARAMETERS    bodyParameters;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(Iteration, clvIR_ITERATION);
    gcmASSERT(Parameters);
    gcmASSERT(Iteration->type == clvWHILE);

    status = _DefineIterationBegin(Compiler,
                       CodeGenerator,
                       gcvTRUE,
                       gcvFALSE,
                       &iterationContext);
    if (gcmIS_ERROR(status)) return status;

    /* The loop body */
    status = _DefineIterationBodyBegin(Compiler, CodeGenerator);
    if (gcmIS_ERROR(status)) return status;

    /* The condition part */
    if (Iteration->condExpr != gcvNULL) {
        status = _GenConditionCode(Compiler,
                       CodeGenerator,
                       Iteration->condExpr,
                       _GetIterationEndLabel(Compiler, CodeGenerator),
                       gcvFALSE);
        if (gcmIS_ERROR(status)) return status;
    }

    if (Iteration->loopBody != gcvNULL) {
        clsGEN_CODE_PARAMETERS_Initialize(&bodyParameters, gcvFALSE, gcvFALSE);
        status = cloIR_OBJECT_Accept(Compiler,
                         Iteration->loopBody,
                         &CodeGenerator->visitor,
                         &bodyParameters);
        if (gcmIS_ERROR(status)) return status;

        clsGEN_CODE_PARAMETERS_Finalize(&bodyParameters);
    }

    status = _DefineIterationBodyEnd(Compiler, CodeGenerator);
    if (gcmIS_ERROR(status)) return status;

    status = _DefineIterationEnd(Compiler,
                     CodeGenerator);
    if (gcmIS_ERROR(status)) return status;
    return gcvSTATUS_OK;
}

gceSTATUS
cloIR_ITERATION_GenDoWhileCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_ITERATION Iteration,
    IN OUT clsGEN_CODE_PARAMETERS * Parameters
    )
{
    gceSTATUS                status;
    clsITERATION_CONTEXT    iterationContext;
    clsGEN_CODE_PARAMETERS    bodyParameters;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(Iteration, clvIR_ITERATION);
    gcmASSERT(Parameters);
    gcmASSERT(Iteration->type == clvDO_WHILE);

    status = _DefineIterationBegin(
                                Compiler,
                                CodeGenerator,
                                gcvFALSE,
                                gcvFALSE,
                                &iterationContext);

    if (gcmIS_ERROR(status)) return status;

    /* The loop body */
    status = _DefineIterationBodyBegin(Compiler, CodeGenerator);

    if (gcmIS_ERROR(status)) return status;

    if (Iteration->loopBody != gcvNULL)
    {
        clsGEN_CODE_PARAMETERS_Initialize(&bodyParameters, gcvFALSE, gcvFALSE);

        status = cloIR_OBJECT_Accept(
                                    Compiler,
                                    Iteration->loopBody,
                                    &CodeGenerator->visitor,
                                    &bodyParameters);

        if (gcmIS_ERROR(status)) return status;

        clsGEN_CODE_PARAMETERS_Finalize(&bodyParameters);
    }

    status = _DefineIterationBodyEnd(Compiler, CodeGenerator);

    if (gcmIS_ERROR(status)) return status;

    /* The condition part */
    if (Iteration->condExpr != gcvNULL)
    {
        status = _GenConditionCode(
                                Compiler,
                                CodeGenerator,
                                Iteration->condExpr,
                                _GetIterationContinueLabel(Compiler, CodeGenerator),
                                gcvTRUE);

        if (gcmIS_ERROR(status)) return status;
    }

    status = _DefineIterationEnd(
                                Compiler,
                                CodeGenerator);

    if (gcmIS_ERROR(status)) return status;

    return gcvSTATUS_OK;
}

gceSTATUS
cloIR_ITERATION_GenCode(
IN cloCOMPILER Compiler,
IN cloCODE_GENERATOR CodeGenerator,
IN cloIR_ITERATION Iteration,
IN OUT clsGEN_CODE_PARAMETERS * Parameters
)
{
    gceSTATUS    status;
    gctBOOL        isUnrolled = gcvFALSE;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(Iteration, clvIR_ITERATION);
    gcmASSERT(Parameters);

    if (cloCOMPILER_OptimizationEnabled(Compiler, clvOPTIMIZATION_UNROLL_ITERATION)) {
        status = cloIR_ITERATION_TryToGenUnrolledCode(Compiler,
                                CodeGenerator,
                                Iteration,
                                Parameters,
                                &isUnrolled);
        if (gcmIS_ERROR(status)) return status;
    }

    if (isUnrolled) return gcvSTATUS_OK;

    switch (Iteration->type) {
    case clvFOR:
        return cloIR_ITERATION_GenForCode(Compiler,
                          CodeGenerator,
                          Iteration,
                          Parameters);

    case clvWHILE:
        return cloIR_ITERATION_GenWhileCode(Compiler,
                            CodeGenerator,
                            Iteration,
                            Parameters);

    case clvDO_WHILE:
        return cloIR_ITERATION_GenDoWhileCode(Compiler,
                              CodeGenerator,
                              Iteration,
                              Parameters);

    default:
        gcmASSERT(0);
        return gcvSTATUS_INVALID_ARGUMENT;
    }
}

gceSTATUS
cloIR_JUMP_GenContinueCode(
IN cloCOMPILER Compiler,
IN cloCODE_GENERATOR CodeGenerator,
IN cloIR_JUMP Jump,
IN OUT clsGEN_CODE_PARAMETERS * Parameters
)
{
    gceSTATUS    status;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(Jump, clvIR_JUMP);
    gcmASSERT(Jump->type == clvCONTINUE);
    gcmASSERT(Parameters);
    gcmASSERT(!Parameters->needLOperand && !Parameters->needROperand);

    if (CodeGenerator->currentIterationContext == gcvNULL) {
        gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                        Jump->base.lineNo,
                        Jump->base.stringNo,
                        clvREPORT_ERROR,
                        "'continue' is only allowed within loops"));
        return gcvSTATUS_INVALID_ARGUMENT;
    }

    status = clEmitAlwaysBranchCode(Compiler,
                    Jump->base.lineNo,
                    Jump->base.stringNo,
                    clvOPCODE_JUMP,
                    _GetIterationContinueLabel(Compiler, CodeGenerator));
    if (gcmIS_ERROR(status)) return status;
    return gcvSTATUS_OK;
}

gceSTATUS
cloIR_JUMP_GenBreakCode(
IN cloCOMPILER Compiler,
IN cloCODE_GENERATOR CodeGenerator,
IN cloIR_JUMP Jump,
IN OUT clsGEN_CODE_PARAMETERS * Parameters
)
{
    gceSTATUS    status;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(Jump, clvIR_JUMP);
    gcmASSERT(Jump->type == clvBREAK);
    gcmASSERT(Parameters);
    gcmASSERT(!Parameters->needLOperand && !Parameters->needROperand);

    if (CodeGenerator->currentIterationContext == gcvNULL) {
        gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                        Jump->base.lineNo,
                        Jump->base.stringNo,
                        clvREPORT_ERROR,
                        "'break' is only allowed within loops"));

        return gcvSTATUS_INVALID_ARGUMENT;
    }

    status = clEmitAlwaysBranchCode(Compiler,
                    Jump->base.lineNo,
                    Jump->base.stringNo,
                    clvOPCODE_JUMP,
                    _GetIterationEndLabel(Compiler, CodeGenerator));
    if (gcmIS_ERROR(status)) return status;

    return gcvSTATUS_OK;
}

gceSTATUS
cloIR_JUMP_GenGotoCode(
IN cloCOMPILER Compiler,
IN cloCODE_GENERATOR CodeGenerator,
IN cloIR_JUMP Jump,
IN OUT clsGEN_CODE_PARAMETERS * Parameters
)
{
    gceSTATUS status;
    cloIR_LABEL label;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(Jump, clvIR_JUMP);
    gcmASSERT(Jump->type == clvGOTO);
    gcmASSERT(Parameters);
    gcmASSERT(!Parameters->needLOperand && !Parameters->needROperand);

    gcmASSERT(Jump->u.label);

    label = Jump->u.label->u.labelInfo.label;
    if(!label) {
        gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                        Jump->base.lineNo,
                        Jump->base.stringNo,
                        clvREPORT_ERROR,
                        "goto label \"%s\" not defined",
                                                Jump->u.label->symbol));
        return gcvSTATUS_INVALID_ARGUMENT;

    }

    if(!label->programCounter) {
          label->programCounter = clNewLabel(Compiler);
    }

    status = clEmitAlwaysBranchCode(Compiler,
                    Jump->base.lineNo,
                    Jump->base.stringNo,
                    clvOPCODE_JUMP,
                    label->programCounter);
    if (gcmIS_ERROR(status)) return status;

    return gcvSTATUS_OK;
}

gceSTATUS
clGenGotoCode(
IN cloCOMPILER Compiler,
IN cloCODE_GENERATOR CodeGenerator,
IN gctBOOL IsNew,
IN OUT cloIR_LABEL Label
)
{
    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    gcmASSERT(Label);

    if(IsNew) {
       cloIR_LABEL_Initialize(0, 0, Label);
       Label->caseValue = gcvNULL;
       Label->caseNumber = 0;
       Label->u.name = gcvNULL;
       Label->type = clvDEFAULT;
           Label->programCounter = clNewLabel(Compiler);
    }
    else {
       gcmASSERT(Label->type == clvDEFAULT);
    }

    gcmASSERT(Label->programCounter);
    return clEmitAlwaysBranchCode(Compiler,
                      Label->base.lineNo,
                      Label->base.stringNo,
                      clvOPCODE_JUMP,
                      Label->programCounter);
}

gceSTATUS
cloIR_JUMP_GenReturnCode(
IN cloCOMPILER Compiler,
IN cloCODE_GENERATOR CodeGenerator,
IN cloIR_JUMP Jump,
IN OUT clsGEN_CODE_PARAMETERS * Parameters
)
{
    gceSTATUS    status;
    clsNAME *    funcName;
    clsGEN_CODE_PARAMETERS    returnExprParameters;
    gctUINT        i;
    clsLOPERAND    lOperand;


    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(Jump, clvIR_JUMP);
    gcmASSERT(Jump->type == clvRETURN);
    gcmASSERT(Parameters);
    gcmASSERT(!Parameters->needLOperand && !Parameters->needROperand);

    if (CodeGenerator->currentFuncDefContext.isMain) {
        if (Jump->u.returnExpr != gcvNULL) {
            gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                            Jump->base.lineNo,
                            Jump->base.stringNo,
                            clvREPORT_ERROR,
                            "'main' function returning a value"));

            return gcvSTATUS_INVALID_ARGUMENT;
        }

        status = clEmitAlwaysBranchCode(Compiler,
                        Jump->base.lineNo,
                        Jump->base.stringNo,
                        clvOPCODE_JUMP,
                        CodeGenerator->currentFuncDefContext.mainEndLabel);
        if (gcmIS_ERROR(status)) return status;
    }
    else if (CodeGenerator->currentFuncDefContext.isKernel) {
        if (Jump->u.returnExpr != gcvNULL) {
            gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                            Jump->base.lineNo,
                            Jump->base.stringNo,
                            clvREPORT_ERROR,
                            "kernel function \"%s\" returning a value",
                            CodeGenerator->currentFuncDefContext.funcBody->funcName));
            return gcvSTATUS_INVALID_ARGUMENT;
        }

        status = clEmitAlwaysBranchCode(Compiler,
                        Jump->base.lineNo,
                        Jump->base.stringNo,
                        clvOPCODE_JUMP,
                        CodeGenerator->currentFuncDefContext.mainEndLabel);
        if (gcmIS_ERROR(status)) return status;
    }
    else {
        funcName = CodeGenerator->currentFuncDefContext.funcBody->funcName;
        gcmASSERT(funcName);

        if (clmDECL_IsVoid(&funcName->decl)) {
            if (Jump->u.returnExpr != gcvNULL) {
                gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                                Jump->base.lineNo,
                                                Jump->base.stringNo,
                                                clvREPORT_ERROR,
                                                "'void' function: '%s' returning a value",
                                                funcName->symbol));
                return gcvSTATUS_INVALID_ARGUMENT;
            }
        }
        else if (Jump->u.returnExpr == gcvNULL) {
            gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                            Jump->base.lineNo,
                                            Jump->base.stringNo,
                                            clvREPORT_WARN,
                                            "non-void function: '%s' must return a value",
                                            funcName->symbol));
        }
        else {
            status = clParseCheckReturnExpr(Compiler,
                                            &funcName->decl,
                                            Jump->u.returnExpr);
            if (gcmIS_ERROR(status)) return status;

            /* Generate the code of the return expression */
            clsGEN_CODE_PARAMETERS_Initialize(&returnExprParameters,
                                              gcvFALSE,
                                              gcvTRUE);

            status = cloIR_OBJECT_Accept(Compiler,
                                         &Jump->u.returnExpr->base,
                                         &CodeGenerator->visitor,
                                         &returnExprParameters);
            if (gcmIS_ERROR(status)) return status;

            status = _GenImplicitConvToType(Compiler,
                                            &funcName->decl,
                                            Jump->u.returnExpr,
                                            &returnExprParameters);
            if (gcmIS_ERROR(status)) return status;

            /* Generate the assign code */
            gcmASSERT(funcName->context.u.variable.logicalRegs);
            if(clmDECL_IsUnderlyingStructOrUnion(&funcName->decl) &&
               clmNAME_VariableHasMemoryOffset(funcName)) {
               gctUINT elementDataTypeSize;

               gcmASSERT(clmDECL_IsUnderlyingStructOrUnion(&Jump->u.returnExpr->decl));
               gcmASSERT(funcName->context.u.variable.logicalRegCount == 1);
               elementDataTypeSize = clsDECL_GetByteSize(&funcName->decl);
               clsLOPERAND_Initialize(&lOperand,
                                      funcName->context.u.variable.logicalRegs);
               _clmGenStructAssign(Compiler,
                                   Jump->u.returnExpr,
                                   &lOperand,
                                   clmNAME_VariableMemoryOffset_NOCHECK_GET(funcName),
                                   &returnExprParameters,
                                   elementDataTypeSize,
                                   status);
               if (gcmIS_ERROR(status)) return status;
            }
            else {
               gcmASSERT(returnExprParameters.operandCount == funcName->context.u.variable.logicalRegCount);

               for (i = 0; i < returnExprParameters.operandCount; i++) {
                  clsLOPERAND_Initialize(&lOperand,
                                         funcName->context.u.variable.logicalRegs + i);

                  status = clGenAssignCode(Compiler,
                                           Jump->base.lineNo,
                                           Jump->base.stringNo,
                                           &lOperand,
                                           returnExprParameters.rOperands + i);
                  if (gcmIS_ERROR(status)) return status;
               }
            }
            clsGEN_CODE_PARAMETERS_Finalize(&returnExprParameters);
        }

        status = clEmitAlwaysBranchCode(Compiler,
                                        Jump->base.lineNo,
                                        Jump->base.stringNo,
                                        clvOPCODE_RETURN,
                                        0);
        if (gcmIS_ERROR(status)) return status;
    }
    return gcvSTATUS_OK;
}

gceSTATUS
cloIR_JUMP_GenCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_JUMP Jump,
    IN OUT clsGEN_CODE_PARAMETERS * Parameters
    )
{
    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(Jump, clvIR_JUMP);
    gcmASSERT(Parameters);

    switch (Jump->type) {
    case clvCONTINUE:
        return cloIR_JUMP_GenContinueCode(Compiler,
                          CodeGenerator,
                          Jump,
                          Parameters);

    case clvBREAK:
        return cloIR_JUMP_GenBreakCode(Compiler,
                           CodeGenerator,
                           Jump,
                           Parameters);

    case clvRETURN:
        return cloIR_JUMP_GenReturnCode(Compiler,
                        CodeGenerator,
                        Jump,
                        Parameters);

    case clvGOTO:
        return cloIR_JUMP_GenGotoCode(Compiler,
                       CodeGenerator,
                       Jump,
                       Parameters);

    default:
        gcmASSERT(0);
        return gcvSTATUS_INVALID_ARGUMENT;
    }
}

gceSTATUS
cloIR_LABEL_GenCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_LABEL Label,
    IN OUT clsGEN_CODE_PARAMETERS * Parameters
    )
{
   gceSTATUS status;

/* Verify the arguments. */
   clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
   clmVERIFY_IR_OBJECT(Label, clvIR_LABEL);
   gcmASSERT(Parameters);
   gcmASSERT(!Parameters->needLOperand && !Parameters->needROperand);

   switch(Label->type) {
   case clvNAMED:
      gcmASSERT(Label->u.name);

      if(Label->u.name->u.labelInfo.isReferenced) {
         if(!Label->programCounter) {
           Label->programCounter = clNewLabel(Compiler);
         }
         status = clSetLabel(Compiler,
                             Label->base.lineNo,
                             Label->base.stringNo,
                             Label->programCounter);
         if (gcmIS_ERROR(status)) return status;
      }
      break;

   case clvCASE:
   case clvDEFAULT:
      gcmASSERT(Label->programCounter);
      status = clSetLabel(Compiler,
                          Label->base.lineNo,
                          Label->base.stringNo,
                          Label->programCounter);
      if (gcmIS_ERROR(status)) return status;
      break;

   default:
      gcmASSERT(0);
   }

   return gcvSTATUS_OK;
}

gceSTATUS
clGenLabelCode(
IN cloCOMPILER Compiler,
IN cloCODE_GENERATOR CodeGenerator,
IN gctBOOL IsNew,
IN cloIR_LABEL Label
)
{
/* Verify the arguments. */
   clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
   clmVERIFY_IR_OBJECT(Label, clvIR_LABEL);
   gcmASSERT(Label);

   if(IsNew) {
      cloIR_LABEL_Initialize(0, 0, Label);
      Label->caseValue = gcvNULL;
      Label->caseNumber = 0;
      Label->u.name = gcvNULL;
      Label->type = clvDEFAULT;
      Label->programCounter = clNewLabel(Compiler);
   }
   else {
      gcmASSERT(Label->type == clvDEFAULT);
   }

   gcmASSERT(Label->programCounter);
   return clSetLabel(Compiler,
                     Label->base.lineNo,
                     Label->base.stringNo,
                     Label->programCounter);
}


static gceSTATUS
_PackedDerefMemory(
IN cloCOMPILER Compiler,
IN cloCODE_GENERATOR CodeGenerator,
IN cloIR_EXPR Expr,
IN OUT clsGEN_CODE_PARAMETERS *Parameters
)
{
    gceSTATUS status = gcvSTATUS_OK;
    cloIR_POLYNARY_EXPR funcCall;
    gctUINT operandCount;
    clsGEN_CODE_PARAMETERS *operandsParameters;

    gcmASSERT(clmDECL_IsPackedType(&Expr->decl));

    status =_ConvDerefPackedPointerExprToFuncCall(Compiler,
                                                  Expr,
                                                  &funcCall);
    if (gcmIS_ERROR(status)) return status;

    /* Generate the code of the operands */
    status = cloIR_POLYNARY_EXPR_GenOperandsCodeForFuncCall(Compiler,
                                                            CodeGenerator,
                                                            funcCall,
                                                            &operandCount,
                                                            &operandsParameters);
    if (gcmIS_ERROR(status)) return status;

    Parameters->operandCount = 0;
    status = clGenFuncCallCode(Compiler,
                               CodeGenerator,
                               funcCall,
                               operandsParameters,
                               Parameters);
    gcmVERIFY_OK(cloIR_POLYNARY_EXPR_FinalizeOperandsParameters(Compiler,
                                                                operandCount,
                                                                operandsParameters));
    return status;
}

static gceSTATUS
_DerefMemory(
IN cloCOMPILER Compiler,
cloIR_EXPR Expr,
IN clsGEN_CODE_PARAMETERS *OperandParameters,
IN OUT clsGEN_CODE_PARAMETERS *Parameters
)
{
  gceSTATUS status;
  clsGEN_CODE_PARAMETERS zeroParameters[1];
  clsDECL decl[1];

  if (Parameters->needLOperand || Parameters->needROperand) {
     status = clsGEN_CODE_PARAMETERS_AllocateOperands(Compiler,
                                                      Parameters,
                                                      &Expr->decl);
     if (gcmIS_ERROR(status)) return status;

     clsGEN_CODE_PARAMETERS_Initialize(zeroParameters,
                                       gcvFALSE,
                                       gcvTRUE);
/* Allocate the fake constant zero operand */
     status = cloCOMPILER_CreateDecl(Compiler,
                                     T_INT,
                                     gcvNULL,
                                     clvQUALIFIER_CONST,
                                     clvQUALIFIER_NONE,
                                     decl);
     if (gcmIS_ERROR(status)) return status;

     status = clsGEN_CODE_PARAMETERS_AllocateOperands(Compiler,
                                                      zeroParameters,
                                                      decl);
     if (gcmIS_ERROR(status)) return status;
     clsROPERAND_InitializeScalarConstant(&zeroParameters->rOperands[0],
                                          zeroParameters->dataTypes[0].def,
                                          int,
                                          0);

     status = clGenDerefPointerCode(Compiler,
                                    Expr,
                                    OperandParameters,
                                    zeroParameters,
                                    Parameters);

     clsGEN_CODE_PARAMETERS_Finalize(zeroParameters);
     if (gcmIS_ERROR(status)) return status;
  }
  return gcvSTATUS_OK;
}

static gceSTATUS
_GenAddressOffsetCode(
IN cloCOMPILER Compiler,
IN cloIR_EXPR Expr,
IN gctSIZE_T Offset,
IN OUT clsGEN_CODE_PARAMETERS *Parameters
)
{
  gceSTATUS status = gcvSTATUS_OK;
  clsIOPERAND iOperand[1];
  clsROPERAND offset[1];
  cluCONSTANT_VALUE constantValue[1];

  if(!Parameters->needLOperand && !Parameters->needROperand) return status;

  if(Parameters->hint & clvGEN_SAVE_ADDRESS_OFFSET) {
     Parameters->dataTypes[0].savedByteOffset += Offset;
  }

  if(Parameters->hint & clvGEN_ADDRESS_OFFSET) {
     if(Offset == 0) return status;
     Parameters->dataTypes[0].byteOffset += Offset;
     return gcvSTATUS_OK;
  }

  constantValue->longValue = Parameters->dataTypes[0].byteOffset + Offset;
  Parameters->dataTypes[0].byteOffset = 0;
  if(constantValue->longValue == 0) return gcvSTATUS_OK;
  clsROPERAND_InitializeConstant(offset,
                                 clmGenCodeDataType(T_INT),
                                 1,
                                 constantValue);

  if(Parameters->needLOperand) {
     clsROPERAND rOperand[1];

     clsIOPERAND_New(Compiler, iOperand, Parameters->lOperands[0].dataType);
     clsROPERAND_InitializeUsingLOperand(rOperand, &Parameters->lOperands[0]);
     gcmONERROR(clGenGenericCode2(Compiler,
                                  Expr->base.lineNo,
                                  Expr->base.stringNo,
                                  clvOPCODE_ADD,
                                  iOperand,
                                  rOperand,
                                  offset));
     if (gcmIS_ERROR(status)) return status;
     clsLOPERAND_InitializeUsingIOperand(&Parameters->lOperands[0], iOperand);
  }

  if(Parameters->needROperand) {
     clsIOPERAND_New(Compiler, iOperand, Parameters->rOperands[0].dataType);
     gcmONERROR(clGenGenericCode2(Compiler,
                                  Expr->base.lineNo,
                                  Expr->base.stringNo,
                                  clvOPCODE_ADD,
                                  iOperand,
                                  &Parameters->rOperands[0],
                                  offset));
     if (gcmIS_ERROR(status)) return status;
     clsROPERAND_InitializeUsingIOperand(&Parameters->rOperands[0], iOperand);
  }

OnError:
  return status;
}

static gceSTATUS
_GenLoadVariableCode(
IN cloCOMPILER Compiler,
IN gctUINT LineNo,
IN gctUINT StringNo,
IN clsNAME *Variable,
IN gctSIZE_T Offset,
IN OUT clsIOPERAND *IOperand
)
{
   clsROPERAND rOperand[1];
   clsROPERAND offset[1];

   clsROPERAND_InitializeReg(rOperand,
                             Variable->context.u.variable.logicalRegs);

   clsROPERAND_InitializeScalarConstant(offset,
                                        clmGenCodeDataType(T_INT),
                                        long,
                                        Offset + clmNAME_VariableMemoryOffset_GET(Variable));

   return clGenLoadCode(Compiler,
                        LineNo,
                        StringNo,
                        IOperand,
                        rOperand,
                        offset);
}

gceSTATUS
cloIR_VARIABLE_GenCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_VARIABLE Variable,
    IN OUT clsGEN_CODE_PARAMETERS * Parameters
    )
{
    gceSTATUS status;
    gctUINT    i;
    cluCONSTANT_VALUE unrolledLoopIndexValue;
    clsDECL    decl;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(Variable, clvIR_VARIABLE);
    gcmASSERT(Parameters);

    gcmASSERT(Variable->name);

    if (!Parameters->needLOperand && !Parameters->needROperand) return gcvSTATUS_OK;

    /* Check if it is the unrolled loop index */
    if (_IsUnrolledLoopIndex(Compiler,
                             CodeGenerator,
                             Variable->name,
                             &unrolledLoopIndexValue))
    {
        gcmASSERT(!Parameters->needLOperand);

        if (Parameters->hint == clvEVALUATE_ONLY)
        {
            /* Create the data type */
            status = cloCOMPILER_CreateDecl(Compiler,
                                            clmDECL_IsInt(&Variable->exprBase.decl) ?
                                            T_INT : T_FLOAT,
                                            gcvNULL,
                                            clvQUALIFIER_CONST,
                                            clvQUALIFIER_NONE,
                                            &decl);
            if (gcmIS_ERROR(status)) return status;

            /* Create the constant */
            status = cloIR_CONSTANT_Construct(Compiler,
                                              Variable->exprBase.base.lineNo,
                                              Variable->exprBase.base.stringNo,
                                              &decl,
                                              &Parameters->constant);
            if (gcmIS_ERROR(status)) return status;

            /* Add the constant value */
            status = cloIR_CONSTANT_AddValues(Compiler,
                                              Parameters->constant,
                                              1,
                                              &unrolledLoopIndexValue);
            if (gcmIS_ERROR(status)) return status;
        }
        else
        {
            /* Allocate the operands */
            status = clsGEN_CODE_PARAMETERS_AllocateOperandsByName(Compiler,
                                                                   Parameters,
                                                                   Variable->name);
            if (gcmIS_ERROR(status)) return status;

            gcmASSERT(Parameters->operandCount == 1);
            gcmASSERT(gcIsScalarDataType(Parameters->dataTypes[0].def));
            gcmASSERT(clmIsElementTypeFloating(clmGEN_CODE_elementType_GET(Parameters->dataTypes[0].def)) ||
                      clmIsElementTypeInteger(clmGEN_CODE_elementType_GET(Parameters->dataTypes[0].def)));

            clsROPERAND_InitializeConstant(&Parameters->rOperands[0],
                                           Parameters->dataTypes[0].def,
                                           1,
                                           &unrolledLoopIndexValue);
            if (Parameters->hasIOperand) {
                gceSTATUS status;

                gcmASSERT(Parameters->operandCount == 1);
                _clmAssignROperandToPreallocatedTempReg(Compiler,
                                                        Parameters,
                                                        &Parameters->rOperands[0],
                                                        status);
                if(gcmIS_ERROR(status)) return status;
            }
        }
    }
    else {
        gctBOOL isDeref = gcvFALSE;
        clsGEN_CODE_PARAMETERS operandParameters[1];
        clsGEN_CODE_PARAMETERS *activeParameters;

        if (Parameters->hint == clvEVALUATE_ONLY) return gcvSTATUS_OK;

        /* Allocate all logical registers */
        status = clsNAME_AllocLogicalRegs(Compiler,
                                          CodeGenerator,
                                          Variable->name);
        if (gcmIS_ERROR(status)) return status;

        if(!(Parameters->hint & clvGEN_ADDR_CODE) &&
           !clmDECL_IsPointerType(&Variable->exprBase.decl) &&
           clmGEN_CODE_checkVariableForMemory(Variable->name)) {
           if(!clmDECL_IsArray(&Variable->exprBase.decl)) {
               isDeref = gcvTRUE;

               if(clmDECL_IsPackedType(&Variable->exprBase.decl)) {
                   return _PackedDerefMemory(Compiler,
                                             CodeGenerator,
                                             &Variable->exprBase,
                                             Parameters);
               }
               activeParameters = operandParameters;
               clsGEN_CODE_PARAMETERS_Initialize(activeParameters,
                                                 Parameters->needLOperand,
                                                 Parameters->needROperand);
           }
           else {
               activeParameters = Parameters;
               if(Variable->name->type == clvVARIABLE_NAME &&
                  Variable->name->u.variableInfo.u.constant != gcvNULL) {
                  activeParameters->hint |= clvGEN_ARRAY_OF_CONSTANTS;
                  gcmASSERT(Variable->name->u.variableInfo.u.constant->variable == Variable->name);
                  activeParameters->constantArray = Variable->name->u.variableInfo.u.constant;
               }
           }
        }
        else {
            activeParameters = Parameters;
        }

        /* Allocate the operands */
        status = clsGEN_CODE_PARAMETERS_AllocateOperandsByName(Compiler,
                                                               activeParameters,
                                                               Variable->name);
        if (gcmIS_ERROR(status)) return status;

        if (activeParameters->needLOperand) {
            for (i = 0; i < activeParameters->operandCount; i++) {
                clsLOPERAND_Initialize(activeParameters->lOperands + i,
                                       Variable->name->context.u.variable.logicalRegs + i);
            }
        }

        if (activeParameters->needROperand) {
            for (i = 0; i < activeParameters->operandCount; i++) {
                clsROPERAND_InitializeReg(activeParameters->rOperands + i,
                                          Variable->name->context.u.variable.logicalRegs + i);
            }
        }
        if(clmNAME_VariableHasMemoryOffset(Variable->name)) {
            activeParameters->hint |= clvGEN_ADDRESS_OFFSET;
            status = _GenAddressOffsetCode(Compiler,
                                           &Variable->exprBase,
                                           clmNAME_VariableMemoryOffset_NOCHECK_GET(Variable->name),
                                           activeParameters);
            if(gcmIS_ERROR(status)) return status;
        }
        if(isDeref) {
            status = _DerefMemory(Compiler,
                                  &Variable->exprBase,
                                  activeParameters,
                                  Parameters);
            clsGEN_CODE_PARAMETERS_Finalize(operandParameters);
            if (gcmIS_ERROR(status)) return status;
        }
        else if (Parameters->needROperand && Parameters->hasIOperand) {
            gceSTATUS status;

            gcmASSERT(Parameters->operandCount == 1);
            _clmAssignROperandToPreallocatedTempReg(Compiler,
                                                    Parameters,
                                                    &activeParameters->rOperands[0],
                                                    status);
            if(gcmIS_ERROR(status)) return status;
        }
    }
    return gcvSTATUS_OK;
}

/*KLC : need work here */

static gceSTATUS
_SetOperandConstants(
IN cloCOMPILER Compiler,
IN clsDECL * Decl,
IN cloIR_CONSTANT Constant,
IN OUT clsGEN_CODE_PARAMETERS * Parameters,
IN OUT gctUINT * ByteOffset,
IN OUT gctUINT * ValueStart,
IN OUT gctUINT * Start
)
{
    gceSTATUS status;
    gctUINT    count, i;
    clsNAME *fieldName;
    clsGEN_CODE_DATA_TYPE binaryDataType;
    gctUINT    componentCount;
    gctUINT byteOffset;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    gcmASSERT(Decl  && Decl->dataType);
    gcmASSERT(Constant);
    gcmASSERT(Parameters);
    gcmASSERT(ByteOffset);
    gcmASSERT(ValueStart);
    gcmASSERT(Start);

    if (clmDECL_IsArray(Decl)) {
       clmGetArrayElementCount(Decl->array, 0, count);
    }
    else count = 1;

    binaryDataType    = _ConvElementDataType(Decl);
    componentCount    = gcGetDataTypeComponentCount(binaryDataType);
    byteOffset = clsDECL_GetElementByteSize(Decl, gcvNULL, gcvNULL);
    for (i = 0; i < count; i++) {
        if (clmDATA_TYPE_IsStructOrUnion(Decl->dataType)) {

            gcmASSERT(Decl->dataType->u.fieldSpace);

            FOR_EACH_DLINK_NODE(&Decl->dataType->u.fieldSpace->names, clsNAME, fieldName) {
                gcmASSERT(fieldName->decl.dataType);

                status = _SetOperandConstants(Compiler,
                                  &fieldName->decl,
                                  Constant,
                                  Parameters,
                                  ByteOffset +
                                  _GetFieldByteOffset(Decl, fieldName, gcvNULL),
                                  ValueStart,
                                  Start);
                if (gcmIS_ERROR(status)) return status;
            }
        }
        else
        {
            if(Constant->variable &&
               (clmDECL_IsAggregateType(&Constant->exprBase.decl) || clmDECL_IsExtendedVectorType(&Constant->exprBase.decl))) {

                clsIOPERAND iOperand[1];

                gcmASSERT(Constant->variable);
                clmGEN_CODE_GetParametersIOperand(Compiler, iOperand, Parameters, binaryDataType);
                clsROPERAND_InitializeUsingIOperand(Parameters->rOperands + *Start, iOperand);
                status = _GenLoadVariableCode(Compiler,
                                              Constant->exprBase.base.lineNo,
                                              Constant->exprBase.base.stringNo,
                                              Constant->variable,
                                              *ByteOffset,
                                              iOperand);
                if(gcmIS_ERROR(status)) return status;
                Parameters->dataTypes[*Start].def = binaryDataType;
                Parameters->dataTypes[*Start].byteOffset = 0;
            }
            else {
                clsROPERAND_InitializeConstant(Parameters->rOperands + *Start,
                                               binaryDataType,
                                               componentCount,
                                               Constant->values + *ValueStart);
                if(Parameters->hasIOperand) {
                   gceSTATUS status;

                   gcmASSERT(Parameters->operandCount == 1 && *Start == 0);
                   _clmAssignROperandToPreallocatedTempReg(Compiler,
                                                           Parameters,
                                                           &Parameters->rOperands[0],
                                                           status);
                   if(gcmIS_ERROR(status)) return status;
                }
            }

            (*Start)++;
            (*ValueStart) += componentCount;
            (*ByteOffset) += byteOffset;
        }
    }

    return gcvSTATUS_OK;
}

gceSTATUS
cloIR_CONSTANT_GenCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_CONSTANT Constant,
    IN OUT clsGEN_CODE_PARAMETERS * Parameters
    )
{
    gceSTATUS status;
    gctUINT    valueStart = 0, start = 0;
    gctUINT byteOffset = 0;
    gctINT isMemoryRef;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(Constant, clvIR_CONSTANT);
    gcmASSERT(Parameters);
    gcmASSERT(!Parameters->needLOperand);

    if (!Parameters->needROperand) return gcvSTATUS_OK;

    if (Parameters->hint == clvEVALUATE_ONLY) {
        return cloIR_CONSTANT_Clone(Compiler,
                        Constant->exprBase.base.lineNo,
                        Constant->exprBase.base.stringNo,
                        Constant,
                        &Parameters->constant);
    }

    /* Allocate the operands */
    isMemoryRef = (gctINT) (Constant->variable &&
                                clmGEN_CODE_checkVariableForMemory(Constant->variable));
    if(isMemoryRef) {
       if(Constant->variable->context.u.variable.logicalRegs == gcvNULL) {
          status = clsNAME_AllocLogicalRegs(Compiler,
                                                CodeGenerator,
                                                Constant->variable);
          if(gcmIS_ERROR(status)) return status;
       }
       status = clsGEN_CODE_PARAMETERS_AllocateOperandsByName(Compiler,
                                      Parameters,
                                      Constant->variable);
       if (gcmIS_ERROR(status)) return status;
    }
    else {
       status = clsGEN_CODE_PARAMETERS_AllocateOperands(Compiler,
                                Parameters,
                                &Constant->exprBase.decl);
       if (gcmIS_ERROR(status)) return status;
    }

    if(Parameters->hint & clvGEN_SUBSCRIPT_CODE) {
       gctUINT i;
       clsLOPERAND constantOperand[1];

       if(Constant->variable) {
          for (i = 0; i < Parameters->operandCount; i++) {
              clsROPERAND_InitializeReg(Parameters->rOperands + i,
                                        Constant->variable->context.u.variable.logicalRegs + isMemoryRef * i);
              Parameters->dataTypes[i].byteOffset = clmNAME_VariableMemoryOffset_GET(Constant->variable);
          }
       }
       else {
          gctREG_INDEX constantReg;

          status = _SetOperandConstants(Compiler,
                                        &Constant->exprBase.decl,
                                        Constant,
                                        Parameters,
                                        &byteOffset,
                                        &valueStart,
                                        &start);
          if (gcmIS_ERROR(status)) return status;

          constantReg = clNewTempRegs(Compiler,
                                      _GetDeclRegSize(&Constant->exprBase.decl),
                                      Constant->exprBase.decl.dataType->elementType);
          for (i = 0; i < Parameters->operandCount; i++) {
             clsLOPERAND_InitializeTempReg(constantOperand,
                           clvQUALIFIER_NONE,
                           Parameters->rOperands[i].dataType,
                           constantReg);

             status = clGenAssignCode(Compiler,
                          Constant->exprBase.base.lineNo,
                          Constant->exprBase.base.stringNo,
                          constantOperand,
                          &Parameters->rOperands[i]);
             if (gcmIS_ERROR(status)) return status;
             clsROPERAND_InitializeUsingLOperand(&Parameters->rOperands[i], constantOperand);
             constantReg += _clmGetTempRegIndexOffset(gcGetDataTypeRegSize(Parameters->rOperands[i].dataType),
                                                      clmGEN_CODE_elementType_GET(Parameters->rOperands[i].dataType));
         }
       }
    }
    else {
       clsDECL *declPtr;
       clsDECL decl[1];

       declPtr = &Constant->exprBase.decl;
       /* Correct vector type to scalar to correspond to scalar constant value */
       if(clmDECL_IsGeneralVectorType(declPtr) && Constant->valueCount == 1) {
           status = cloCOMPILER_CreateElementDecl(Compiler,
                                                  declPtr,
                                                  decl);
           if (gcmIS_ERROR(status)) return status;
           declPtr = decl;
       }
       status = _SetOperandConstants(Compiler,
                                     declPtr,
                                     Constant,
                                     Parameters,
                                     &byteOffset,
                                     &valueStart,
                                     &start);
       if (gcmIS_ERROR(status)) return status;

       gcmASSERT(valueStart == Constant->valueCount);
       gcmASSERT(start == Parameters->operandCount);
    }

    return gcvSTATUS_OK;
}

gceSTATUS
clGenDerefStructPointerCode(
    IN cloCOMPILER Compiler,
    IN cloIR_UNARY_EXPR UnaryExpr,
    IN clsGEN_CODE_PARAMETERS *LeftParameters,
    IN OUT clsGEN_CODE_PARAMETERS *Parameters
    )
{
  gceSTATUS status;
  gctUINT i;
  gctINT fieldByteOffset = 0;

/* Verify the arguments. */
  clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
  gcmASSERT(UnaryExpr);
  gcmASSERT(LeftParameters);
  gcmASSERT(Parameters);

  if (Parameters->needLOperand || Parameters->needROperand) {
     status = clsGEN_CODE_PARAMETERS_AllocateOperands(Compiler,
                                                      Parameters,
                                                      &UnaryExpr->exprBase.decl);
     if (gcmIS_ERROR(status)) return status;
  }

/** Delay code generation when the left hand side is needed.
    To do this, we have to copy the subscript and the left parameter's
    roperands if any onto the return Parameters. This is somewhat a kluge **/
  if (Parameters->needLOperand &&
      Parameters->hint & clvGEN_LEFT_ASSIGN_CODE) {
     fieldByteOffset = _GetFieldByteOffset(&UnaryExpr->operand->decl,
                                           UnaryExpr->u.fieldName,
                                           gcvNULL);
     if(!Parameters->elementIndex) {
        gctPOINTER pointer;

        status = cloCOMPILER_Allocate(Compiler,
                                      (gctSIZE_T)sizeof(clsROPERAND),
                                      (gctPOINTER *) &pointer);
        if (gcmIS_ERROR(status)) return status;
        Parameters->elementIndex = pointer;
     }

     clsROPERAND_InitializeScalarConstant(Parameters->elementIndex,
                                          clmGenCodeDataType(T_INT),
                                          long,
                                          fieldByteOffset + LeftParameters->dataTypes[0].byteOffset);

     Parameters->dataTypes[0].byteOffset = 0;
     for (i = 0; i < Parameters->operandCount; i++) {
         Parameters->lOperands[i] = LeftParameters->lOperands[0];
     }
     if (Parameters->needROperand) {
       for (i = 0; i < Parameters->operandCount; i++) {
         Parameters->rOperands[i] = LeftParameters->rOperands[0];
       }
     }
     Parameters->hint = clvGEN_DEREF_STRUCT_CODE;
     return gcvSTATUS_OK;
  }

  if (Parameters->needROperand || Parameters->needLOperand) {
     fieldByteOffset = _GetFieldByteOffset(&UnaryExpr->operand->decl,
                                           UnaryExpr->u.fieldName,
                                           gcvNULL);
     if(Parameters->needLOperand) {
        status = _GenAddressOffsetCode(Compiler,
                                       &UnaryExpr->exprBase,
                                       fieldByteOffset,
                                       LeftParameters);
        if (gcmIS_ERROR(status)) return status;
        Parameters->lOperands[0] = LeftParameters->lOperands[0];
        Parameters->hint = clvGEN_DEREF_CODE;
     }
     if(Parameters->needROperand) {
        /*klc: defer loading the struct to subsequent operation */
        if(clmDECL_IsUnderlyingStructOrUnion(&UnaryExpr->exprBase.decl)) {
           Parameters->dataTypes[0].byteOffset = fieldByteOffset + LeftParameters->dataTypes[0].byteOffset;
           Parameters->rOperands[0] = LeftParameters->rOperands[0];
           Parameters->hint = clvGEN_DEREF_CODE;
        }
        else {
           clsIOPERAND iOperand[1];
           clsROPERAND offset[1];
           clsGEN_CODE_DATA_TYPE dataType;

           gcmASSERT(Parameters->operandCount == 1);

           clsROPERAND_InitializeScalarConstant(offset,
                                                clmGenCodeDataType(T_INT),
                                                long,
                                                fieldByteOffset + LeftParameters->dataTypes[0].byteOffset);
           clmGEN_CODE_ConvDirectElementDataType(UnaryExpr->u.fieldName->decl.dataType, dataType);

           clmGEN_CODE_GetParametersIOperand(Compiler, iOperand, Parameters, dataType);
           status = clGenGenericCode2(Compiler,
                                      UnaryExpr->exprBase.base.lineNo,
                                      UnaryExpr->exprBase.base.stringNo,
                                      clvOPCODE_LOAD,
                                      iOperand,
                                      &LeftParameters->rOperands[0],
                                      offset);
           if (gcmIS_ERROR(status)) return status;
           clsROPERAND_InitializeUsingIOperand(&Parameters->rOperands[0], iOperand);
        }
     }
  }

  return gcvSTATUS_OK;
}

gceSTATUS
cloIR_UNARY_EXPR_GenFieldSelectionCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_UNARY_EXPR UnaryExpr,
    IN OUT clsGEN_CODE_PARAMETERS * Parameters
    )
{
   gceSTATUS    status;
   clsGEN_CODE_PARAMETERS operandParameters[1];
   gctSIZE_T    operandFieldOffset = 0;
   gctUINT i;

   /* Verify the arguments. */
   clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
   clmVERIFY_OBJECT(CodeGenerator, clvOBJ_CODE_GENERATOR);
   clmVERIFY_IR_OBJECT(UnaryExpr, clvIR_UNARY_EXPR);
   gcmASSERT(UnaryExpr->type == clvUNARY_FIELD_SELECTION);
   gcmASSERT(Parameters);

   /* Generate the code of the operand */
   gcmASSERT(UnaryExpr->operand);

   clsGEN_CODE_PARAMETERS_Initialize(operandParameters,
                        Parameters->needLOperand,
                        Parameters->needROperand);

   operandParameters->hint = clvGEN_FIELD_SELECT_CODE;
   status = cloIR_OBJECT_Accept(Compiler,
                       &UnaryExpr->operand->base,
                       &CodeGenerator->visitor,
                       operandParameters);
   if (gcmIS_ERROR(status)) return status;


   if(Parameters->hint & (clvGEN_ADDR_CODE | clvGEN_DEREF_STRUCT_CODE | clvGEN_SUBSCRIPT_CODE) ||
      ((operandParameters->hint & clvGEN_DEREF_STRUCT_CODE) &&
       (Parameters->hint & clvGEN_FIELD_SELECT_CODE))) {
        gctINT fieldByteOffset = 0;

        if (Parameters->needLOperand || Parameters->needROperand) {
           status = clsGEN_CODE_PARAMETERS_AllocateOperands(Compiler,
                                                            Parameters,
                                                            &UnaryExpr->exprBase.decl);

           if (gcmIS_ERROR(status)) return status;

           fieldByteOffset = _GetFieldByteOffset(&UnaryExpr->operand->decl,
                                                 UnaryExpr->u.fieldName,
                                                 gcvNULL);

           operandParameters->hint &= ~clvGEN_ADDRESS_OFFSET;
           status = _GenAddressOffsetCode(Compiler,
                                          &UnaryExpr->exprBase,
                                          fieldByteOffset,
                                          operandParameters);
           if (gcmIS_ERROR(status)) return status;

           Parameters->dataTypes[0].byteOffset = operandParameters->dataTypes[0].byteOffset;
           if(Parameters->needLOperand) {
              Parameters->lOperands[0] = operandParameters->lOperands[0];
           }
           if(Parameters->needROperand) {
              Parameters->rOperands[0] = operandParameters->rOperands[0];
           }
        }
        if(!(Parameters->hint & clvGEN_ADDR_CODE)) {
           Parameters->hint |= clvGEN_DEREF_STRUCT_CODE;
        }
   }
   else if(operandParameters->hint & clvGEN_DEREF_STRUCT_CODE) {
        status = clGenDerefStructPointerCode(Compiler,
                                             UnaryExpr,
                                             operandParameters,
                                             Parameters);
        if (gcmIS_ERROR(status)) return status;
   }
   else {
    /* Copy all field operands */
    if (Parameters->needLOperand || Parameters->needROperand) {
           status = clsGEN_CODE_PARAMETERS_AllocateOperands(Compiler,
                                                            Parameters,
                                                            &UnaryExpr->exprBase.decl);

           if (gcmIS_ERROR(status)) return status;

           operandFieldOffset = _GetLogicalOperandFieldOffset(&UnaryExpr->operand->decl,
                                                              UnaryExpr->u.fieldName);
    }

    if (Parameters->needLOperand) {
           for (i = 0; i < Parameters->operandCount; i++) {
              Parameters->lOperands[i] = operandParameters->lOperands[operandFieldOffset + i];
              Parameters->lOperands[i].dataType = Parameters->dataTypes[i].def;
           }
        }

        if (Parameters->needROperand) {
           for (i = 0; i < Parameters->operandCount; i++) {
              Parameters->rOperands[i] = operandParameters->rOperands[operandFieldOffset + i];
              Parameters->rOperands[i].dataType = Parameters->dataTypes[i].def;
           }
       if(Parameters->hasIOperand) {
          gceSTATUS status;

          gcmASSERT(Parameters->operandCount == 1);
          _clmAssignROperandToPreallocatedTempReg(Compiler,
                              Parameters,
                              &Parameters->rOperands[0],
                              status);
          if(gcmIS_ERROR(status)) return status;
           }
        }
   }

   clsGEN_CODE_PARAMETERS_Finalize(operandParameters);

   return gcvSTATUS_OK;
}

static gctREG_INDEX
_ConvComponentToVectorIndex(
    IN gctUINT8 Component
    )
{
    switch (Component) {
    case clvCOMPONENT_X: return 0;
    case clvCOMPONENT_Y: return 1;
    case clvCOMPONENT_Z: return 2;
    case clvCOMPONENT_W: return 3;

    case clvCOMPONENT_4: return 4;
    case clvCOMPONENT_5: return 5;
    case clvCOMPONENT_6: return 6;
    case clvCOMPONENT_7: return 7;

    case clvCOMPONENT_8: return 8;
    case clvCOMPONENT_9: return 9;
    case clvCOMPONENT_10: return 10;
    case clvCOMPONENT_11: return 11;

    case clvCOMPONENT_12: return 12;
    case clvCOMPONENT_13: return 13;
    case clvCOMPONENT_14: return 14;
    case clvCOMPONENT_15: return 15;
    default:
    gcmASSERT(0);
    return 0;
    }
}

static gctBOOL
_IsExprDeref(
cloCOMPILER Compiler,
cloIR_EXPR Expr
)
{
    cloIR_UNARY_EXPR unaryExpr;
    cloIR_BINARY_EXPR binaryExpr;
    clsNAME *variable;

    switch(cloIR_OBJECT_GetType(&Expr->base)) {
    case clvIR_UNARY_EXPR:
        unaryExpr = (cloIR_UNARY_EXPR) &Expr->base;
        if(unaryExpr->type == clvUNARY_INDIRECTION) return gcvTRUE;
        break;

    case clvIR_BINARY_EXPR:
        binaryExpr = (cloIR_BINARY_EXPR) &Expr->base;
        if(binaryExpr->type == clvBINARY_SUBSCRIPT) {
            if(clmDECL_IsPointerType(&binaryExpr->leftOperand->decl)) return gcvTRUE;
            variable = clParseFindLeafName(Compiler,
                                           binaryExpr->leftOperand);
            if(variable && clmGEN_CODE_checkVariableForMemory(variable)) {
               return gcvTRUE;
            }
        }
        break;

    default:
        variable = clParseFindLeafName(Compiler,
                                       Expr);
        if(variable && clmGEN_CODE_checkVariableForMemory(variable)) {
           return gcvTRUE;
        }
        break;
    }
    return gcvFALSE;
}

gceSTATUS
cloIR_UNARY_EXPR_GenComponentSelectionCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_UNARY_EXPR UnaryExpr,
    IN OUT clsGEN_CODE_PARAMETERS * Parameters
    )
{
    gceSTATUS  status;
    clsGEN_CODE_PARAMETERS    operandParameters;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_OBJECT(CodeGenerator, clvOBJ_CODE_GENERATOR);
    clmVERIFY_IR_OBJECT(UnaryExpr, clvIR_UNARY_EXPR);
    gcmASSERT(UnaryExpr->type == clvUNARY_COMPONENT_SELECTION);
    gcmASSERT(Parameters);

    /* Generate the code of the operand */
    gcmASSERT(UnaryExpr->operand);

    clsGEN_CODE_PARAMETERS_Initialize(&operandParameters,
                    Parameters->needLOperand,
                    Parameters->needROperand);

    if(Parameters->hint & clvGEN_LEFT_ASSIGN_CODE) {
       operandParameters.hint = clvGEN_COMPONENT_SELECT_CODE | clvGEN_LEFT_ASSIGN_CODE;
    }
    else {
       operandParameters.hint = clvGEN_COMPONENT_SELECT_CODE;
    }

/*KLC */
#if _cldIncludeComponentOffset
    if(Parameters->hasIOperand  &&
       (Parameters->hint & clvGEN_SELECTIVE_LOAD_TO ||
           _IsExprDeref(Compiler, UnaryExpr->operand))) { /* transfer ioperand to here */
        clsIOPERAND *iOperand;
        gctUINT8 componentIx = Parameters->iOperand[0].componentSelection.components;

        iOperand = &Parameters->iOperand[0];
        clmGEN_CODE_SetParametersIOperand(Compiler,
                                          &operandParameters,
                                          0,
                                          iOperand,
                                          componentIx);
        Parameters->hasIOperand = gcvFALSE;
    }
#endif
    operandParameters.expr = &UnaryExpr->exprBase;
    status = cloIR_OBJECT_Accept(Compiler,
                  &UnaryExpr->operand->base,
                  &CodeGenerator->visitor,
                  &operandParameters);

    if (gcmIS_ERROR(status)) return status;

    if((Parameters->hint & clvGEN_LEFT_ASSIGN_CODE) &&
        (operandParameters.hint & (clvGEN_DEREF_CODE | clvGEN_DEREF_STRUCT_CODE))) {
       clsGEN_CODE_PARAMETERS_MoveOperands(Parameters, &operandParameters);
       clmGEN_CODE_ConvDirectElementDataType(UnaryExpr->exprBase.decl.dataType, Parameters->dataTypes[0].def);
       Parameters->expr = &UnaryExpr->exprBase;
       Parameters->hint = (operandParameters.hint & (clvGEN_DEREF_CODE | clvGEN_DEREF_STRUCT_CODE)) |
                clvGEN_COMPONENT_SELECT_CODE;
    }
    else {
       /* Make the swizzled operand */
       if (Parameters->needLOperand || Parameters->needROperand) {
       status = clsGEN_CODE_PARAMETERS_AllocateOperands(Compiler,
                         Parameters,
                         &UnaryExpr->exprBase.decl);
       if (gcmIS_ERROR(status)) return status;
       }

       if(operandParameters.hint & clvGEN_SELECTIVE_LOADED) {
          if (Parameters->needLOperand) {
             Parameters->lOperands[0] = operandParameters.lOperands[0];
          }
          if (Parameters->needROperand) {
             Parameters->rOperands[0] = operandParameters.rOperands[0];
          }
       }
       else {
          clsCOMPONENT_SELECTION componentSelection[1];

          if (Parameters->needLOperand) {
             gcmASSERT(Parameters->operandCount == 1);

             Parameters->lOperands[0] = operandParameters.lOperands[0];

             Parameters->lOperands[0].dataType =
             gcGetVectorComponentSelectionDataType(operandParameters.lOperands[0].dataType,
                                                   UnaryExpr->u.componentSelection.components);

             if (UnaryExpr->u.componentSelection.components == 1)
             {
                gcmASSERT(Parameters->lOperands[0].vectorIndex.mode == clvINDEX_NONE);

                Parameters->lOperands[0].vectorIndex.mode = clvINDEX_CONSTANT;
                Parameters->lOperands[0].vectorIndex.u.constant =
                   _ConvComponentToVectorIndex(UnaryExpr->u.componentSelection.selection[clvCOMPONENT_X]);
             }
             else
             {
                Parameters->lOperands[0].reg.componentSelection =
                   _SwizzleComponentSelection(_CheckHighPrecisionComponentSelection(Parameters->lOperands[0].dataType,
                                                                                    &UnaryExpr->u.componentSelection,
                                                                                    componentSelection),
                                              &operandParameters.lOperands[0].reg.componentSelection);
             }
          }

          if (Parameters->needROperand) {
             gcmASSERT(Parameters->operandCount == 1);

             Parameters->rOperands[0] = operandParameters.rOperands[0];

             Parameters->rOperands[0].dataType =
                gcGetVectorComponentSelectionDataType(operandParameters.rOperands[0].dataType,
                                                      UnaryExpr->u.componentSelection.components);

             if (UnaryExpr->u.componentSelection.components == 1)
             {
                gcmASSERT(Parameters->rOperands[0].vectorIndex.mode == clvINDEX_NONE);

                Parameters->rOperands[0].vectorIndex.mode = clvINDEX_CONSTANT;
                Parameters->rOperands[0].vectorIndex.u.constant =
                _ConvComponentToVectorIndex(UnaryExpr->u.componentSelection.selection[clvCOMPONENT_X]);
                Parameters->rOperands[0].u.reg.componentSelection = UnaryExpr->u.componentSelection;
             }
             else
             {
                Parameters->rOperands[0].u.reg.componentSelection =
                   _SwizzleComponentSelection(_CheckHighPrecisionComponentSelection(Parameters->rOperands[0].dataType,
                                                                                    &UnaryExpr->u.componentSelection,
                                                                                    componentSelection),
                                              &operandParameters.rOperands[0].u.reg.componentSelection);
             }
             if(Parameters->hasIOperand) {
                gceSTATUS status;

                gcmASSERT(Parameters->operandCount == 1);
                _clmAssignROperandToPreallocatedTempReg(Compiler,
                                                        Parameters,
                                                        &Parameters->rOperands[0],
                                                        status);
                if(gcmIS_ERROR(status)) return status;
             }
          }
       }
    }

    clsGEN_CODE_PARAMETERS_Finalize(&operandParameters);

    return gcvSTATUS_OK;
}

gceSTATUS
clGenPointerArithmeticCode(
    IN cloCOMPILER Compiler,
    IN cloIR_EXPR Expr,
    IN gctINT Opcode,
    IN cleGEN_CODE_HINT Hint,
    IN clsGEN_CODE_PARAMETERS *LeftParameters,
    IN clsROPERAND *RightOperand,
    IN clsROPERAND *Res
)
{
    gceSTATUS status;
    clsIOPERAND iOperand[1];
    clsROPERAND scaledIndex[1];
    gctUINT elementDataTypeSize = 0;

    gcmASSERT(LeftParameters);
    gcmASSERT(RightOperand);
    gcmASSERT(Res);

    if(!clmDECL_IsPointerType(&Expr->decl)) {
       clsROPERAND rOperand[1];
       clsROPERAND size[1];
       gctINT shiftValue;
       clsDECL *pointerDecl;

       gcmASSERT(Opcode == clvOPCODE_SUB);
       pointerDecl = &(((cloIR_BINARY_EXPR)&Expr->base)->leftOperand->decl);
       clsIOPERAND_New(Compiler, iOperand, Res->dataType);
       status = clGenGenericCode2(Compiler,
                                  Expr->base.lineNo,
                                  Expr->base.stringNo,
                                  Opcode,
                                  iOperand,
                                  &LeftParameters->rOperands[0],
                                  RightOperand);
       if (gcmIS_ERROR(status)) return status;

       elementDataTypeSize = clsDECL_GetPointedToByteSize(pointerDecl);
       shiftValue = _ConvValueToPowerOfTwo(elementDataTypeSize);
       clsROPERAND_InitializeUsingIOperand(rOperand, iOperand);
       if(shiftValue > 0) {
          clsROPERAND_InitializeScalarConstant(size,
                                               clmGenCodeDataType(T_INT),
                                               long,
                                               shiftValue);
          clsIOPERAND_New(Compiler, iOperand, Res->dataType);
          status = clGenGenericCode2(Compiler,
                                     Expr->base.lineNo,
                                     Expr->base.stringNo,
                                     clvOPCODE_RSHIFT,
                                     iOperand,
                                     rOperand,
                                     size);
          if (gcmIS_ERROR(status)) return status;
       }
       else if(shiftValue < 0) {
          clsGEN_CODE_DATA_TYPE offsetType;
          clsROPERAND_InitializeScalarConstant(size,
                                               clmGenCodeDataType(T_INT),
                                               long,
                                               elementDataTypeSize);
/*KLC*/
/* cmodel supports only 16 bit integer */
          offsetType = clmGenCodeDataType(T_SHORT);
          clsIOPERAND_New(Compiler, iOperand, offsetType);
          status = clGenGenericCode2(Compiler,
                                     Expr->base.lineNo,
                                     Expr->base.stringNo,
                                     clvOPCODE_DIV,
                                     iOperand,
                                     rOperand,
                                     size);
          if (gcmIS_ERROR(status)) return status;
          clsROPERAND_InitializeUsingIOperand(rOperand, iOperand);

          clsIOPERAND_New(Compiler, iOperand, Res->dataType);
          status = clGenGenericCode1(Compiler,
                                     Expr->base.lineNo,
                                     Expr->base.stringNo,
                                     clvOPCODE_CONV,
                                     iOperand,
                                     rOperand);
          if (gcmIS_ERROR(status)) return status;
       }
       clsROPERAND_InitializeUsingIOperand(Res, iOperand);
       return gcvSTATUS_OK;
    }
    else {
       if(_IsIntegerZero(RightOperand)) {
         if(LeftParameters->dataTypes[0].byteOffset) {
           _InitializeROperandConstant(scaledIndex,
                                       clmGenCodeDataType(T_INT),
                                       LeftParameters->dataTypes[0].byteOffset);
         }
         else {
           *Res = LeftParameters->rOperands[0];
           return gcvSTATUS_OK;
         }
      }
      else {
         elementDataTypeSize = clsDECL_GetPointedToByteSize(&Expr->decl);
         status = clGenScaledIndexOperandWithOffset(Compiler,
                                                    Expr->base.lineNo,
                                                    Expr->base.stringNo,
                                                    RightOperand,
                                                    elementDataTypeSize,
                                                    LeftParameters->dataTypes[0].byteOffset,
                                                    scaledIndex);
         if (gcmIS_ERROR(status)) return status;
      }

      if(!scaledIndex->isReg &&
         Hint & clvGEN_ADDRESS_OFFSET) {
         gctINT offset;

         *Res = LeftParameters->rOperands[0];
         offset = _GetIntegerValue(scaledIndex);
         if(Opcode == clvOPCODE_SUB) {
            LeftParameters->dataTypes[0].byteOffset = -offset;
            if(Hint & clvGEN_SAVE_ADDRESS_OFFSET) {
                LeftParameters->dataTypes[0].savedByteOffset = -offset;
            }
         }
         else {
            LeftParameters->dataTypes[0].byteOffset = offset;
            if(Hint & clvGEN_SAVE_ADDRESS_OFFSET) {
                LeftParameters->dataTypes[0].savedByteOffset = offset;
            }
         }
      }
      else {
         clsIOPERAND_New(Compiler, iOperand, Res->dataType);
         status = clGenGenericCode2(Compiler,
                                    Expr->base.lineNo,
                                    Expr->base.stringNo,
                                    Opcode,
                                    iOperand,
                                    &LeftParameters->rOperands[0],
                                    scaledIndex);
         if (gcmIS_ERROR(status)) return status;

         clsROPERAND_InitializeUsingIOperand(Res, iOperand);
      }
      return gcvSTATUS_OK;
   }
}

gceSTATUS
cloIR_UNARY_EXPR_GenIncOrDecCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_UNARY_EXPR UnaryExpr,
    IN OUT clsGEN_CODE_PARAMETERS * Parameters
    )
{
    gceSTATUS    status;
    gctBOOL        isPost = gcvFALSE, isInc = gcvFALSE;
    clsGEN_CODE_PARAMETERS    operandParameters[1];
    cluCONSTANT_VALUE constantValue[1];
    clsROPERAND    constantROperand[1];
    cltELEMENT_TYPE elementType;
    clsIOPERAND intermIOperand[1];
    clsROPERAND intermROperand[1];
    clsROPERAND scaledIndex[1];

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_OBJECT(CodeGenerator, clvOBJ_CODE_GENERATOR);
    clmVERIFY_IR_OBJECT(UnaryExpr, clvIR_UNARY_EXPR);
    gcmASSERT(Parameters);

    /* Generate the code of the operand */
    gcmASSERT(UnaryExpr->operand);

    gcoOS_ZeroMemory(&constantValue, gcmSIZEOF(constantValue));
    clsGEN_CODE_PARAMETERS_Initialize(operandParameters, gcvTRUE, gcvTRUE);

    operandParameters->hint = clvGEN_LEFT_ASSIGN_CODE;
    status = cloIR_OBJECT_Accept(Compiler,
                                 &UnaryExpr->operand->base,
                                 &CodeGenerator->visitor,
                                 operandParameters);

    if (gcmIS_ERROR(status)) return status;

    gcmASSERT(operandParameters->operandCount == 1);

    if(operandParameters->hint & clvGEN_DEREF_CODE) {
       gctUINT elementDataTypeSize;

       elementDataTypeSize = clsDECL_GetByteSize(&UnaryExpr->operand->decl);
       gcmONERROR(clGenScaledIndexOperandWithOffset(Compiler,
                                                    UnaryExpr->exprBase.base.lineNo,
                                                    UnaryExpr->exprBase.base.stringNo,
                                                    operandParameters->elementIndex,
                                                    elementDataTypeSize,
                                                    operandParameters->dataTypes[0].byteOffset,
                                                    scaledIndex));

       if (operandParameters->needROperand) {
           clsIOPERAND iOperand[1];

           clsIOPERAND_New(Compiler, iOperand, operandParameters->dataTypes[0].def);

           gcmONERROR(clGenGenericCode2(Compiler,
                                        UnaryExpr->exprBase.base.lineNo,
                                        UnaryExpr->exprBase.base.stringNo,
                                        clvOPCODE_LOAD,
                                        iOperand,
                                        &operandParameters->rOperands[0],
                                        scaledIndex));
           clsROPERAND_InitializeUsingIOperand(operandParameters->rOperands, iOperand);
       }
    }
    else if(operandParameters->hint & clvGEN_DEREF_STRUCT_CODE) {
       if (operandParameters->needROperand) {
           clsIOPERAND iOperand[1];

           clsIOPERAND_New(Compiler, iOperand, operandParameters->dataTypes[0].def);

           gcmONERROR(clGenGenericCode2(Compiler,
                                        UnaryExpr->exprBase.base.lineNo,
                                        UnaryExpr->exprBase.base.stringNo,
                                        clvOPCODE_LOAD,
                                        iOperand,
                                        &operandParameters->rOperands[0],
                                        operandParameters->elementIndex));
           clsROPERAND_InitializeUsingIOperand(operandParameters->rOperands, iOperand);
       }
    }

    /* add/sub t0, operand, 1 */
    elementType = clmGEN_CODE_elementType_GET(operandParameters->dataTypes[0].def);
    if(clmIsElementTypeFloating(elementType)) {
        constantValue->floatValue = (gctFLOAT)1.0;
    }
    else if(clmIsElementTypeInteger(elementType)) {
        constantValue->intValue    = 1;
    }
    else {
        gcmASSERT(0);
        constantValue->intValue    = 0;
    }

    clsROPERAND_InitializeConstant(constantROperand,
                                   gcGetComponentDataType(operandParameters->dataTypes[0].def),
                                   1,
                                   constantValue);

    switch (UnaryExpr->type) {
    case clvUNARY_POST_INC:
       isPost = gcvTRUE;
       isInc = gcvTRUE;
       break;

    case clvUNARY_POST_DEC:
       isPost = gcvTRUE;
       break;

    case clvUNARY_PRE_INC:
       isInc = gcvTRUE;
       break;

    case clvUNARY_PRE_DEC:
       break;

    default: gcmASSERT(0);
    }

    if(clmDECL_IsPointerType(&UnaryExpr->operand->decl)) {
         intermROperand->dataType = operandParameters->dataTypes[0].def;
         gcmONERROR(clGenPointerArithmeticCode(Compiler,
                                               &UnaryExpr->exprBase,
                                               (isInc) ? clvOPCODE_ADD : clvOPCODE_SUB,
                                               clvGEN_NO_HINT,
                                               operandParameters,
                                               constantROperand,
                                               intermROperand));
         clsIOPERAND_Initialize(intermIOperand, intermROperand->dataType, intermROperand->u.reg.regIndex);
         /* Generate the assign code */
         gcmONERROR(clGenAssignCode(Compiler,
                                    UnaryExpr->exprBase.base.lineNo,
                                    UnaryExpr->exprBase.base.stringNo,
                                    operandParameters->lOperands,
                                    intermROperand));
    }
    else {
        /* Generate the inc or dec code */
        clsIOPERAND_New(Compiler, intermIOperand, operandParameters->dataTypes[0].def);

        gcmONERROR(clGenArithmeticExprCode(Compiler,
                                           UnaryExpr->exprBase.base.lineNo,
                                           UnaryExpr->exprBase.base.stringNo,
                                           (isInc) ? clvOPCODE_ADD : clvOPCODE_SUB,
                                           intermIOperand,
                                           operandParameters->rOperands,
                                           constantROperand));

        clsROPERAND_InitializeUsingIOperand(intermROperand, intermIOperand);

        if(operandParameters->hint & clvGEN_DEREF_CODE) {
           gcmONERROR(clGenStoreCode(Compiler,
                                     UnaryExpr->exprBase.base.lineNo,
                                     UnaryExpr->exprBase.base.stringNo,
                                     intermROperand,
                                     operandParameters->lOperands,
                                     operandParameters->dataTypes[0].def,
                                     scaledIndex));
        }
        else if(operandParameters->hint & clvGEN_DEREF_STRUCT_CODE) {
            gcmONERROR(clGenStoreCode(Compiler,
                                      UnaryExpr->exprBase.base.lineNo,
                                      UnaryExpr->exprBase.base.stringNo,
                                      intermROperand,
                                      operandParameters->lOperands,
                                      operandParameters->dataTypes[0].def,
                                      operandParameters->elementIndex));
        }
        else {
            /* Generate the assign code */
            gcmONERROR(clGenAssignCode(Compiler,
                                    UnaryExpr->exprBase.base.lineNo,
                                    UnaryExpr->exprBase.base.stringNo,
                                    operandParameters->lOperands,
                                    intermROperand));
        }
    }

          if (Parameters->needROperand || Parameters->needLOperand) {
              if (isPost) {
                  /* sub/add t0, operand, 1 */
                  if(clmDECL_IsPointerType(&UnaryExpr->operand->decl)) {
                      intermROperand->dataType = operandParameters->dataTypes[0].def;
                      gcmONERROR(clGenPointerArithmeticCode(Compiler,
                                                            &UnaryExpr->exprBase,
                                                            (isInc) ? clvOPCODE_SUB : clvOPCODE_ADD,
                                                            clvGEN_NO_HINT,
                                                            operandParameters,
                                                            constantROperand,
                                                            intermROperand));
                      clsIOPERAND_Initialize(intermIOperand, intermROperand->dataType, intermROperand->u.reg.regIndex);
                  }
                  else {
                      clsIOPERAND_New(Compiler, intermIOperand, operandParameters->dataTypes[0].def);
                      gcmONERROR(clGenArithmeticExprCode(Compiler,
                                                         UnaryExpr->exprBase.base.lineNo,
                                                         UnaryExpr->exprBase.base.stringNo,
                                                         (isInc) ? clvOPCODE_SUB : clvOPCODE_ADD,
                                                         intermIOperand,
                                                         &operandParameters->rOperands[0],
                                                         constantROperand));
                  }

                  /* Return t0 */
                  gcmONERROR(clsGEN_CODE_PARAMETERS_AllocateOperands(Compiler,
                                                                     Parameters,
                                                                     &UnaryExpr->exprBase.decl));

                  if(Parameters->needROperand) {
                      clsROPERAND_InitializeUsingIOperand(&Parameters->rOperands[0], intermIOperand);
                  }
                  if(Parameters->needLOperand) {
                      clsLOPERAND_InitializeUsingIOperand(&Parameters->lOperands[0], intermIOperand);
                  }
             }
             else {
                  /* Return the operand directly */
                  clsGEN_CODE_PARAMETERS_MoveOperands(Parameters, operandParameters);
             }
             if(Parameters->needROperand &&
                Parameters->hasIOperand) {
                 gceSTATUS status;

                 gcmASSERT(Parameters->operandCount == 1);
                 _clmAssignROperandToPreallocatedTempReg(Compiler,
                                                         Parameters,
                                                         &Parameters->rOperands[0],
                                                         status);
                 if(gcmIS_ERROR(status)) return status;
             }

          }

OnError:
    clsGEN_CODE_PARAMETERS_Finalize(operandParameters);

    return gcvSTATUS_OK;
}

gceSTATUS
cloIR_UNARY_EXPR_GenNegCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_UNARY_EXPR UnaryExpr,
    IN OUT clsGEN_CODE_PARAMETERS * Parameters
    )
{
    gceSTATUS status;
    clsGEN_CODE_PARAMETERS    operandParameters;
    clsIOPERAND intermIOperand;
    cluCONSTANT_VALUE constantValue;
    clsROPERAND constantROperand;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_OBJECT(CodeGenerator, clvOBJ_CODE_GENERATOR);
    clmVERIFY_IR_OBJECT(UnaryExpr, clvIR_UNARY_EXPR);
    gcmASSERT(UnaryExpr->type == clvUNARY_NEG);
    gcmASSERT(Parameters);
    gcmASSERT(!Parameters->needLOperand);

    /* Generate the code of the operand */
    gcmASSERT(UnaryExpr->operand);

    gcoOS_ZeroMemory(&constantValue, gcmSIZEOF(constantValue));
    clsGEN_CODE_PARAMETERS_Initialize(&operandParameters, gcvFALSE, Parameters->needROperand);

    status = cloIR_OBJECT_Accept(Compiler,
                     &UnaryExpr->operand->base,
                     &CodeGenerator->visitor,
                     &operandParameters);

    if (gcmIS_ERROR(status)) return status;

    if (Parameters->needROperand)
    {
        cltELEMENT_TYPE elementType;

        gcmASSERT(operandParameters.operandCount == 1);
        /* Return t0 */
        status = clsGEN_CODE_PARAMETERS_AllocateOperands(Compiler,
                                                         Parameters,
                                                         &UnaryExpr->exprBase.decl);
        if (gcmIS_ERROR(status)) return status;

        clmGEN_CODE_GetParametersIOperand(Compiler, &intermIOperand, Parameters, Parameters->dataTypes[0].def);

        if(gcmOPT_oclUseNeg())
        {
            status = clGenGenericCode1(Compiler,
                                       UnaryExpr->exprBase.base.lineNo,
                                       UnaryExpr->exprBase.base.stringNo,
                                       clvOPCODE_NEG,
                                       &intermIOperand,
                                       &operandParameters.rOperands[0]);
        }
        else
        {
            /* sub t0, 0, operand */
            elementType = clmGEN_CODE_elementType_GET(operandParameters.dataTypes[0].def);
            if(clmIsElementTypeFloating(elementType)) {
                constantValue.floatValue = (gctFLOAT)0.0;
            }
            else if(clmIsElementTypeInteger(elementType)) {
                constantValue.intValue = 0;
            }
            else {
                gcmASSERT(0);
            }

            clsROPERAND_InitializeConstant(&constantROperand,
                                           gcGetComponentDataType(operandParameters.dataTypes[0].def),
                                           1,
                                           &constantValue);

            status = clGenArithmeticExprCode(Compiler,
                                             UnaryExpr->exprBase.base.lineNo,
                                             UnaryExpr->exprBase.base.stringNo,
                                             clvOPCODE_SUB,
                                             &intermIOperand,
                                             &constantROperand,
                                             &operandParameters.rOperands[0]);
        }

        if (gcmIS_ERROR(status)) return status;

        clsROPERAND_InitializeUsingIOperand(&Parameters->rOperands[0], &intermIOperand);
    }

    clsGEN_CODE_PARAMETERS_Finalize(&operandParameters);

    return gcvSTATUS_OK;
}

gceSTATUS
cloIR_UNARY_EXPR_GenNotCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_UNARY_EXPR UnaryExpr,
    IN OUT clsGEN_CODE_PARAMETERS * Parameters
    )
{
    gceSTATUS status;
    clsGEN_CODE_PARAMETERS    operandParameters;
    clsIOPERAND intermIOperand;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_OBJECT(CodeGenerator, clvOBJ_CODE_GENERATOR);
    clmVERIFY_IR_OBJECT(UnaryExpr, clvIR_UNARY_EXPR);
    gcmASSERT(UnaryExpr->type == clvUNARY_NOT);
    gcmASSERT(Parameters);
    gcmASSERT(!Parameters->needLOperand);

    /* Generate the code of the operand */
    gcmASSERT(UnaryExpr->operand);

    clsGEN_CODE_PARAMETERS_Initialize(&operandParameters, gcvFALSE, Parameters->needROperand);

    status = cloIR_OBJECT_Accept(Compiler,
                     &UnaryExpr->operand->base,
                     &CodeGenerator->visitor,
                     &operandParameters);

    if (gcmIS_ERROR(status)) return status;

    if (Parameters->needROperand)
    {
        gcmASSERT(operandParameters.operandCount == 1);
        /* Return t0 */
        status = clsGEN_CODE_PARAMETERS_AllocateOperands(Compiler,
                                Parameters,
                                &UnaryExpr->exprBase.decl);

        if (gcmIS_ERROR(status)) return status;

        /* not t0, operand */
                clmGEN_CODE_GetParametersIOperand(Compiler, &intermIOperand, Parameters, Parameters->dataTypes[0].def);

        status = clGenGenericCode1(Compiler,
                       UnaryExpr->exprBase.base.lineNo,
                       UnaryExpr->exprBase.base.stringNo,
                       clvOPCODE_NOT,
                       &intermIOperand,
                       &operandParameters.rOperands[0]);
        if (gcmIS_ERROR(status)) return status;

        clsROPERAND_InitializeUsingIOperand(&Parameters->rOperands[0], &intermIOperand);
    }

    clsGEN_CODE_PARAMETERS_Finalize(&operandParameters);

    return gcvSTATUS_OK;
}

gceSTATUS
cloIR_UNARY_EXPR_GenBitwiseNotCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_UNARY_EXPR UnaryExpr,
    IN OUT clsGEN_CODE_PARAMETERS * Parameters
    )
{
    gceSTATUS status;
    clsGEN_CODE_PARAMETERS    operandParameters;
    clsIOPERAND intermIOperand;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_OBJECT(CodeGenerator, clvOBJ_CODE_GENERATOR);
    clmVERIFY_IR_OBJECT(UnaryExpr, clvIR_UNARY_EXPR);
    gcmASSERT(UnaryExpr->type == clvUNARY_BITWISE_NOT);
    gcmASSERT(Parameters);
    gcmASSERT(!Parameters->needLOperand);

    /* Generate the code of the operand */
    gcmASSERT(UnaryExpr->operand);

    clsGEN_CODE_PARAMETERS_Initialize(&operandParameters, gcvFALSE, Parameters->needROperand);

    status = cloIR_OBJECT_Accept(Compiler,
                     &UnaryExpr->operand->base,
                     &CodeGenerator->visitor,
                     &operandParameters);

    if (gcmIS_ERROR(status)) return status;

    if (Parameters->needROperand)
    {
        gcmASSERT(operandParameters.operandCount == 1);

        /* Return t0 */
        status = clsGEN_CODE_PARAMETERS_AllocateOperands(Compiler,
                                Parameters,
                                &UnaryExpr->exprBase.decl);

        if (gcmIS_ERROR(status)) return status;

        /* not t0, operand */
                clmGEN_CODE_GetParametersIOperand(Compiler, &intermIOperand, Parameters, Parameters->dataTypes[0].def);

        status = clGenGenericCode1(Compiler,
                       UnaryExpr->exprBase.base.lineNo,
                       UnaryExpr->exprBase.base.stringNo,
                       clvOPCODE_BITWISE_NOT,
                       &intermIOperand,
                       &operandParameters.rOperands[0]);

        if (gcmIS_ERROR(status)) return status;

        clsROPERAND_InitializeUsingIOperand(&Parameters->rOperands[0], &intermIOperand);
    }

    clsGEN_CODE_PARAMETERS_Finalize(&operandParameters);

    return gcvSTATUS_OK;
}

gceSTATUS
cloIR_UNARY_EXPR_GenNonLvalCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_UNARY_EXPR UnaryExpr,
    IN OUT clsGEN_CODE_PARAMETERS * Parameters
    )
{
    gceSTATUS status;
    clsGEN_CODE_PARAMETERS    operandParameters;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_OBJECT(CodeGenerator, clvOBJ_CODE_GENERATOR);
    clmVERIFY_IR_OBJECT(UnaryExpr, clvIR_UNARY_EXPR);
    gcmASSERT(UnaryExpr->type == clvUNARY_NON_LVAL);
    gcmASSERT(Parameters);
    gcmASSERT(!Parameters->needLOperand);

    /* Generate the code of the operand */
    gcmASSERT(UnaryExpr->operand);

    clsGEN_CODE_PARAMETERS_Initialize(&operandParameters, gcvFALSE, Parameters->needROperand);

    status = cloIR_OBJECT_Accept(Compiler,
                     &UnaryExpr->operand->base,
                     &CodeGenerator->visitor,
                     &operandParameters);
    if (gcmIS_ERROR(status)) return status;

    if (Parameters->needROperand)
    {
           gcmASSERT(operandParameters.operandCount == 1);
           clsGEN_CODE_PARAMETERS_MoveOperands(Parameters, &operandParameters);
           if(Parameters->hasIOperand) {
              gceSTATUS status;

              gcmASSERT(Parameters->operandCount == 1);
              _clmAssignROperandToPreallocatedTempReg(Compiler,
                                                      Parameters,
                                                      &Parameters->rOperands[0],
                                                      status);
          if(gcmIS_ERROR(status)) return status;
           }
    }

    clsGEN_CODE_PARAMETERS_Finalize(&operandParameters);

    return gcvSTATUS_OK;
}

gceSTATUS
cloIR_UNARY_EXPR_GenAddrCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_UNARY_EXPR UnaryExpr,
    IN OUT clsGEN_CODE_PARAMETERS * Parameters
    )
{
    gceSTATUS status;
    clsGEN_CODE_PARAMETERS operandParameters;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_OBJECT(CodeGenerator, clvOBJ_CODE_GENERATOR);
    clmVERIFY_IR_OBJECT(UnaryExpr, clvIR_UNARY_EXPR);
    gcmASSERT(UnaryExpr->type == clvUNARY_ADDR);
    gcmASSERT(Parameters);
    gcmASSERT(!Parameters->needLOperand);

    /* Generate the code of the operand */
    gcmASSERT(UnaryExpr->operand);

    clsGEN_CODE_PARAMETERS_Initialize(&operandParameters, gcvFALSE, Parameters->needROperand);

    operandParameters.hint = clvGEN_ADDR_CODE | (Parameters->hint & clvGEN_SAVE_ADDRESS_OFFSET);
    status = cloIR_OBJECT_Accept(Compiler,
                                 &UnaryExpr->operand->base,
                                 &CodeGenerator->visitor,
                                 &operandParameters);
    if (gcmIS_ERROR(status)) return status;

    if (Parameters->needROperand) {
        clsIOPERAND iOperand[1];
        clsLOPERAND lOperand[1];

        gcmASSERT(operandParameters.operandCount == 1);
        status = clsGEN_CODE_PARAMETERS_AllocateOperands(Compiler,
                                                         Parameters,
                                                         &UnaryExpr->exprBase.decl);
        if (gcmIS_ERROR(status)) return status;

        clmGEN_CODE_GetParametersIOperand(Compiler, iOperand, Parameters, Parameters->dataTypes[0].def);
        clsLOPERAND_InitializeUsingIOperand(lOperand, iOperand);

        if(operandParameters.dataTypes[0].byteOffset) {
            operandParameters.hint &= ~clvGEN_ADDRESS_OFFSET;
            status = _GenAddressOffsetCode(Compiler,
                                           &UnaryExpr->exprBase,
                                           0,
                                           &operandParameters);
            if(gcmIS_ERROR(status)) return status;
        }

        if(Parameters->hint & clvGEN_SAVE_ADDRESS_OFFSET) {
            Parameters->dataTypes[0].savedByteOffset += operandParameters.dataTypes[0].savedByteOffset;
        }

        status = clGenAssignCode(Compiler,
                                 UnaryExpr->exprBase.base.lineNo,
                                 UnaryExpr->exprBase.base.stringNo,
                                 lOperand,
                                 &operandParameters.rOperands[0]);
        if (gcmIS_ERROR(status)) return status;

        clsROPERAND_InitializeUsingIOperand(&Parameters->rOperands[0], iOperand);
    }

    clsGEN_CODE_PARAMETERS_Finalize(&operandParameters);

    return gcvSTATUS_OK;
}

gceSTATUS
cloIR_UNARY_EXPR_GenCastCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_UNARY_EXPR UnaryExpr,
    IN OUT clsGEN_CODE_PARAMETERS * Parameters
    )
{
    gceSTATUS status;
    clsGEN_CODE_PARAMETERS    operandParameters;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_OBJECT(CodeGenerator, clvOBJ_CODE_GENERATOR);
    clmVERIFY_IR_OBJECT(UnaryExpr, clvIR_UNARY_EXPR);
    gcmASSERT(UnaryExpr->type == clvUNARY_CAST);
    gcmASSERT(Parameters);

    /* Generate the code of the operand */
    gcmASSERT(UnaryExpr->operand);

    clsGEN_CODE_PARAMETERS_Initialize(&operandParameters, Parameters->needLOperand, Parameters->needROperand);

    status = cloIR_OBJECT_Accept(Compiler,
                     &UnaryExpr->operand->base,
                     &CodeGenerator->visitor,
                     &operandParameters);
    if (gcmIS_ERROR(status)) return status;

    if(clmDECL_IsArray(&UnaryExpr->operand->decl)) {
           operandParameters.operandCount = 1;
    }
    gcmASSERT(operandParameters.operandCount == 1);
    if(clmDECL_IsPointerType(&UnaryExpr->exprBase.decl)) {
       status = clsGEN_CODE_PARAMETERS_AllocateOperands(Compiler,
                                Parameters,
                                &UnaryExpr->exprBase.decl);
       if (gcmIS_ERROR(status)) return status;

       if(Parameters->needROperand) {
          Parameters->rOperands[0] = operandParameters.rOperands[0];
       }
       if(Parameters->needLOperand) {
          Parameters->lOperands[0] = operandParameters.lOperands[0];
       }
    }
    else {
       gcmASSERT(!Parameters->needLOperand);
       if(Parameters->needROperand) {
         clsGEN_CODE_DATA_TYPE newDataType;

         newDataType = _ConvElementDataType(&UnaryExpr->exprBase.decl);
         status = clsROPERAND_ChangeDataTypeFamily(Compiler,
                                                       UnaryExpr->exprBase.base.lineNo,
                                                       UnaryExpr->exprBase.base.stringNo,
                                                       gcvFALSE,
                                                       newDataType,
                                                       &operandParameters.rOperands[0]);
         if (gcmIS_ERROR(status)) return status;

         status = clsGEN_CODE_PARAMETERS_AllocateOperands(Compiler,
                                  Parameters,
                                  &UnaryExpr->exprBase.decl);
         if (gcmIS_ERROR(status)) return status;
             Parameters->rOperands[0] = operandParameters.rOperands[0];
       }
    }
    if(Parameters->needROperand &&
      Parameters->hasIOperand) {
      gceSTATUS status;

      gcmASSERT(Parameters->operandCount == 1);
      _clmAssignROperandToPreallocatedTempReg(Compiler,
                          Parameters,
                          &Parameters->rOperands[0],
                          status);
      if(gcmIS_ERROR(status)) return status;
    }

    clsGEN_CODE_PARAMETERS_Finalize(&operandParameters);

    return gcvSTATUS_OK;
}

static cloIR_EXPR
_CastPointerExprToComponentPointerExpr(
IN cloCOMPILER Compiler,
IN gctUINT LineNo,
IN gctUINT StringNo,
IN cloIR_EXPR PointerExpr)
{
    gceSTATUS status;
    clsDECL decl;
    clsBUILTIN_DATATYPE_INFO *typeInfo;
    gctINT componentType;
    cloIR_EXPR expr = gcvNULL;

    decl = PointerExpr->decl;
    typeInfo = clGetBuiltinDataTypeInfo(PointerExpr->decl.dataType->type);
    componentType = typeInfo->componentType;

    decl = PointerExpr->decl;
    status = cloCOMPILER_CreateDataType(Compiler,
                                        componentType,
                                        gcvNULL,
                                        PointerExpr->decl.dataType->accessQualifier,
                                        PointerExpr->decl.dataType->addrSpaceQualifier,
                                        &decl.dataType);
    if (gcmIS_ERROR(status)) return gcvNULL;

/* Create cast expression */
    status = cloIR_CAST_EXPR_Construct(Compiler,
                                       LineNo,
                                       StringNo,
                                       &decl,
                                       PointerExpr,
                                       &expr);
    if (gcmIS_ERROR(status)) return gcvNULL;
    return expr;
}

static gceSTATUS
_ConvDerefPackedPointerExprToFuncCall(
IN cloCOMPILER Compiler,
IN cloIR_EXPR Expr,
OUT cloIR_POLYNARY_EXPR *FuncCall
)
{
   gceSTATUS status = gcvSTATUS_OK;
   cloIR_POLYNARY_EXPR    newFuncCall;
   gctCHAR   nameBuf[32];
   gctSTRING funcNameString = nameBuf;
   gctUINT   offset = 0;
   cloIR_UNARY_EXPR unaryExpr;
   cloIR_BINARY_EXPR binaryExpr;
   cloIR_EXPR offsetExpr = gcvNULL;
   cloIR_EXPR pointerExpr = gcvNULL;
   clsNAME *variable;
   gctUINT8 vectorSize;

   gcmASSERT(FuncCall);
   *FuncCall = gcvNULL;

   gcmVERIFY_OK(cloCOMPILER_Lock(Compiler));
   gcmASSERT(clmDECL_IsPackedType(&Expr->decl));

   switch(cloIR_OBJECT_GetType(&Expr->base)) {
   case clvIR_UNARY_EXPR:
       unaryExpr = (cloIR_UNARY_EXPR) &Expr->base;
       if(unaryExpr->type == clvUNARY_INDIRECTION) {
           cloIR_CONSTANT constant;

           gcmASSERT(clmIsElementTypePacked(unaryExpr->operand->decl.dataType->elementType));

           pointerExpr = _CastPointerExprToComponentPointerExpr(Compiler,
                                                                Expr->base.lineNo,
                                                                Expr->base.stringNo,
                                                                unaryExpr->operand);
           gcmONERROR(clConstructScalarIntegerConstant(Compiler,
                                                       Expr->base.lineNo,
                                                       Expr->base.stringNo,
                                                       0,
                                                       &constant));
           offsetExpr = &constant->exprBase;
       }
       else {
           gcmASSERT(0);
           status = gcvSTATUS_INVALID_ARGUMENT;
           gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
       }
       break;

   case clvIR_BINARY_EXPR:
       binaryExpr = (cloIR_BINARY_EXPR) &Expr->base;
       if(binaryExpr->type == clvBINARY_SUBSCRIPT) {
           if(clmDECL_IsPointerType(&binaryExpr->leftOperand->decl)) {
               gcmASSERT(clmIsElementTypePacked(binaryExpr->leftOperand->decl.dataType->elementType));
               pointerExpr = _CastPointerExprToComponentPointerExpr(Compiler,
                                                                    binaryExpr->leftOperand->base.lineNo,
                                                                    binaryExpr->leftOperand->base.stringNo,
                                                                    binaryExpr->leftOperand);
               offsetExpr = binaryExpr->rightOperand;
           }
           else {
               variable = clParseFindLeafName(Compiler,
                                              binaryExpr->leftOperand);
               if(variable && clmGEN_CODE_checkVariableForMemory(variable) &&
                  clmDECL_IsPackedType(&variable->decl)) {
                   gcmONERROR(cloIR_UNARY_EXPR_Construct(Compiler,
                                                         Expr->base.lineNo,
                                                         Expr->base.stringNo,
                                                         clvUNARY_ADDR,
                                                         binaryExpr->leftOperand,
                                                         gcvNULL,
                                                         gcvNULL,
                                                         &unaryExpr));
                   pointerExpr = _CastPointerExprToComponentPointerExpr(Compiler,
                                                                        Expr->base.lineNo,
                                                                        Expr->base.stringNo,
                                                                        &unaryExpr->exprBase);
                   offsetExpr = binaryExpr->rightOperand;
               }
           }
           if(!pointerExpr) {
               gcmASSERT(0);
               status = gcvSTATUS_INVALID_ARGUMENT;
               gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
           }
       }
       break;

   default:
       variable = clParseFindLeafName(Compiler,
                                      Expr);
       if(variable && clmGEN_CODE_checkVariableForMemory(variable) &&
          clmDECL_IsPackedType(&variable->decl)) {
           cloIR_CONSTANT constant;

           gcmONERROR(cloIR_UNARY_EXPR_Construct(Compiler,
                                                 Expr->base.lineNo,
                                                 Expr->base.stringNo,
                                                 clvUNARY_ADDR,
                                                 Expr,
                                                 gcvNULL,
                                                 gcvNULL,
                                                 &unaryExpr));
           pointerExpr = _CastPointerExprToComponentPointerExpr(Compiler,
                                                                Expr->base.lineNo,
                                                                Expr->base.stringNo,
                                                                &unaryExpr->exprBase);
           gcmONERROR(clConstructScalarIntegerConstant(Compiler,
                                                       Expr->base.lineNo,
                                                       Expr->base.stringNo,
                                                       0,
                                                       &constant));
           offsetExpr = &constant->exprBase;
       }
       else {
           gcmASSERT(0);
           status = gcvSTATUS_INVALID_ARGUMENT;
           gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
       }
   }

   vectorSize = clmDATA_TYPE_vectorSize_GET(Expr->decl.dataType);

   gcmVERIFY_OK(gcoOS_PrintStrSafe(funcNameString,
                                   32,
                                   &offset,
                                   "viv_intrinsic_vx_vload%d",
                                   vectorSize));
   newFuncCall = _CreateFuncCallByName(Compiler,
                                       Expr->base.lineNo,
                                       Expr->base.stringNo,
                                       funcNameString,
                                       Expr);
   if(!newFuncCall) {
     status = gcvSTATUS_INVALID_ARGUMENT;
     gcmONERROR(status);
   }

   gcmASSERT(newFuncCall->operands);
   gcmONERROR(cloIR_SET_AddMember(Compiler,
                                  newFuncCall->operands,
                                  &offsetExpr->base));
   gcmONERROR(cloIR_SET_AddMember(Compiler,
                                  newFuncCall->operands,
                                  &pointerExpr->base));

   gcmONERROR(cloCOMPILER_BindFuncCall(Compiler,
                                       newFuncCall));

   *FuncCall = newFuncCall;

OnError:
   gcmVERIFY_OK(cloCOMPILER_Unlock(Compiler));
   return status;
}

static gceSTATUS
_ConvStorePackedDataToFuncCall(
IN cloCOMPILER Compiler,
IN cloIR_EXPR Expr,
OUT cloIR_POLYNARY_EXPR *FuncCall
)
{
   gceSTATUS status = gcvSTATUS_OK;
   cloIR_POLYNARY_EXPR    newFuncCall;
   gctCHAR   nameBuf[32];
   gctSTRING funcNameString = nameBuf;
   gctUINT   offset = 0;
   cloIR_UNARY_EXPR unaryExpr;
   cloIR_BINARY_EXPR binaryExpr;
   cloIR_EXPR leftExpr = gcvNULL;
   cloIR_EXPR rightExpr = gcvNULL;
   cloIR_EXPR offsetExpr = gcvNULL;
   cloIR_EXPR pointerExpr = gcvNULL;
   clsNAME *variable;
   gctUINT8 vectorSize;

   gcmASSERT(FuncCall);
   *FuncCall = gcvNULL;

   gcmVERIFY_OK(cloCOMPILER_Lock(Compiler));
   gcmASSERT(clmDECL_IsPackedType(&Expr->decl));

   if(cloIR_OBJECT_GetType(&Expr->base) == clvIR_BINARY_EXPR) {
       binaryExpr = (cloIR_BINARY_EXPR) &Expr->base;
       if(binaryExpr->type == clvBINARY_ASSIGN) {
           leftExpr = binaryExpr->leftOperand;
           rightExpr = binaryExpr->rightOperand;
       }
       else {
           gcmASSERT(0);
           status = gcvSTATUS_INVALID_ARGUMENT;
           gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
       }
   }
   else {
       gcmASSERT(0);
       status = gcvSTATUS_INVALID_ARGUMENT;
       gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
   }

   switch(cloIR_OBJECT_GetType(&leftExpr->base)) {
   case clvIR_UNARY_EXPR:
       unaryExpr = (cloIR_UNARY_EXPR) &leftExpr->base;
       if(unaryExpr->type == clvUNARY_INDIRECTION) {
           cloIR_CONSTANT constant;

           gcmASSERT(clmIsElementTypePacked(unaryExpr->operand->decl.dataType->elementType));

           pointerExpr = _CastPointerExprToComponentPointerExpr(Compiler,
                                                                Expr->base.lineNo,
                                                                Expr->base.stringNo,
                                                                unaryExpr->operand);
           gcmONERROR(clConstructScalarIntegerConstant(Compiler,
                                                       Expr->base.lineNo,
                                                       Expr->base.stringNo,
                                                       0,
                                                       &constant));
           offsetExpr = &constant->exprBase;
       }
       else {
           gcmASSERT(0);
           status = gcvSTATUS_INVALID_ARGUMENT;
           gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
       }

       break;

   case clvIR_BINARY_EXPR:
       binaryExpr = (cloIR_BINARY_EXPR) &leftExpr->base;
       if(binaryExpr->type == clvBINARY_SUBSCRIPT) {
           if(clmDECL_IsPointerType(&binaryExpr->leftOperand->decl)) {
               gcmASSERT(clmIsElementTypePacked(binaryExpr->leftOperand->decl.dataType->elementType));
               pointerExpr = _CastPointerExprToComponentPointerExpr(Compiler,
                                                                    binaryExpr->leftOperand->base.lineNo,
                                                                    binaryExpr->leftOperand->base.stringNo,
                                                                    binaryExpr->leftOperand);
               offsetExpr = binaryExpr->rightOperand;
           }
           else { /* is an array */
               variable = clParseFindLeafName(Compiler,
                                              binaryExpr->leftOperand);
               if(variable && clmGEN_CODE_checkVariableForMemory(variable)) {
                  gcmONERROR(clParseMakeArrayPointerExpr(Compiler,
                                                         binaryExpr->leftOperand,
                                                         &pointerExpr));

                  pointerExpr = _CastPointerExprToComponentPointerExpr(Compiler,
                                                                       binaryExpr->leftOperand->base.lineNo,
                                                                       binaryExpr->leftOperand->base.stringNo,
                                                                       pointerExpr);
                  offsetExpr = binaryExpr->rightOperand;
               }
           }
           if(!pointerExpr) {
               gcmASSERT(0);
               status = gcvSTATUS_INVALID_ARGUMENT;
               gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
           }
       }
       break;

   default:
       variable = clParseFindLeafName(Compiler,
                                      leftExpr);
       if(variable && clmGEN_CODE_checkVariableForMemory(variable) &&
          clmDECL_IsPackedType(&variable->decl)) {
           cloIR_CONSTANT constant;

           gcmONERROR(cloIR_UNARY_EXPR_Construct(Compiler,
                                                 leftExpr->base.lineNo,
                                                 leftExpr->base.stringNo,
                                                 clvUNARY_ADDR,
                                                 leftExpr,
                                                 gcvNULL,
                                                 gcvNULL,
                                                 &unaryExpr));
           pointerExpr = _CastPointerExprToComponentPointerExpr(Compiler,
                                                                leftExpr->base.lineNo,
                                                                leftExpr->base.stringNo,
                                                                &unaryExpr->exprBase);
           gcmONERROR(clConstructScalarIntegerConstant(Compiler,
                                                       Expr->base.lineNo,
                                                       Expr->base.stringNo,
                                                       0,
                                                       &constant));
           offsetExpr = &constant->exprBase;
       }
       else {
           gcmASSERT(0);
           status = gcvSTATUS_INVALID_ARGUMENT;
           gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
       }
   }

   vectorSize = clmDATA_TYPE_vectorSize_GET(leftExpr->decl.dataType);

   gcmVERIFY_OK(gcoOS_PrintStrSafe(funcNameString,
                                   32,
                                   &offset,
                                   "viv_intrinsic_vx_vstore%d",
                                   vectorSize));
   newFuncCall = _CreateFuncCallByName(Compiler,
                                       Expr->base.lineNo,
                                       Expr->base.stringNo,
                                       funcNameString,
                                       gcvNULL);
   if(!newFuncCall) {
     status = gcvSTATUS_INVALID_ARGUMENT;
     gcmONERROR(status);
   }

   gcmASSERT(newFuncCall->operands);
   gcmONERROR(cloIR_SET_AddMember(Compiler,
                                  newFuncCall->operands,
                                  &rightExpr->base));
   gcmONERROR(cloIR_SET_AddMember(Compiler,
                                  newFuncCall->operands,
                                  &offsetExpr->base));
   gcmONERROR(cloIR_SET_AddMember(Compiler,
                                  newFuncCall->operands,
                                  &pointerExpr->base));

   gcmONERROR(cloCOMPILER_BindFuncCall(Compiler,
                                       newFuncCall));

   *FuncCall = newFuncCall;

OnError:
   gcmVERIFY_OK(cloCOMPILER_Unlock(Compiler));
   return status;
}

gceSTATUS
cloIR_POLYNARY_EXPR_GenOperandsCodeForFuncCall(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    OUT gctUINT * OperandCount,
    OUT clsGEN_CODE_PARAMETERS * * OperandsParameters
    );

gceSTATUS
cloIR_UNARY_EXPR_GenIndirectionCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_UNARY_EXPR UnaryExpr,
    IN OUT clsGEN_CODE_PARAMETERS * Parameters
    )
{
  gceSTATUS status;

/* Verify the arguments. */
  clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
  clmVERIFY_OBJECT(CodeGenerator, clvOBJ_CODE_GENERATOR);
  clmVERIFY_IR_OBJECT(UnaryExpr, clvIR_UNARY_EXPR);
  gcmASSERT(UnaryExpr->type == clvUNARY_INDIRECTION);
  gcmASSERT(Parameters);

/* Generate the code of the operand */
  gcmASSERT(UnaryExpr->operand);

  if(clmDECL_IsPackedType(&UnaryExpr->exprBase.decl)) {
      return _PackedDerefMemory(Compiler,
                                CodeGenerator,
                                &UnaryExpr->exprBase,
                                Parameters);
  }
  else {
      clsGEN_CODE_PARAMETERS operandParameters[1];

      clsGEN_CODE_PARAMETERS_Initialize(operandParameters,
                                        Parameters->needLOperand,
                                        Parameters->needROperand);

      status = cloIR_OBJECT_Accept(Compiler,
                                   &UnaryExpr->operand->base,
                                   &CodeGenerator->visitor,
                                   operandParameters);
      if (gcmIS_ERROR(status)) return status;

      status = _DerefMemory(Compiler,
                            &UnaryExpr->exprBase,
                            operandParameters,
                            Parameters);
      clsGEN_CODE_PARAMETERS_Finalize(operandParameters);
      if (gcmIS_ERROR(status)) return status;
  }

  return gcvSTATUS_OK;
}

gceSTATUS
cloIR_UNARY_EXPR_GenCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_UNARY_EXPR UnaryExpr,
    IN OUT clsGEN_CODE_PARAMETERS * Parameters
    )
{
    gceSTATUS status;
    clsGEN_CODE_PARAMETERS    operandParameters;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_OBJECT(CodeGenerator, clvOBJ_CODE_GENERATOR);
    clmVERIFY_IR_OBJECT(UnaryExpr, clvIR_UNARY_EXPR);
    gcmASSERT(Parameters);

    /* Try to evaluate the operand */
    if (!Parameters->needLOperand && Parameters->needROperand &&
        UnaryExpr->type != clvUNARY_FIELD_SELECTION &&
        UnaryExpr->type != clvUNARY_ADDR)
    {
        gcmASSERT(UnaryExpr->operand);

        clsGEN_CODE_PARAMETERS_Initialize(&operandParameters,
                                          gcvFALSE,
                                          gcvTRUE);

        operandParameters.hint = clvEVALUATE_ONLY;

        status = cloIR_OBJECT_Accept(Compiler,
                                     &UnaryExpr->operand->base,
                                     &CodeGenerator->visitor,
                                     &operandParameters);
        if (gcmIS_ERROR(status)) return status;

        if (operandParameters.constant != gcvNULL)
        {
            status = cloIR_UNARY_EXPR_Evaluate(Compiler,
                                               UnaryExpr->type,
                                               operandParameters.constant,
                                               UnaryExpr->u.fieldName,
                                               &UnaryExpr->u.componentSelection,
                                               &Parameters->constant);
            if (gcmIS_ERROR(status)) return status;

            operandParameters.constant = gcvNULL;
        }

        clsGEN_CODE_PARAMETERS_Finalize(&operandParameters);

        if (Parameters->hint == clvEVALUATE_ONLY) return gcvSTATUS_OK;

        if (Parameters->constant != gcvNULL)
        {
            return cloIR_CONSTANT_GenCode(Compiler,
                                          CodeGenerator,
                                          Parameters->constant,
                                          Parameters);
        }
    }

    switch (UnaryExpr->type)
    {
    case clvUNARY_FIELD_SELECTION:
            if(Parameters->hint == clvEVALUATE_ONLY) return gcvSTATUS_OK;
            return cloIR_UNARY_EXPR_GenFieldSelectionCode(Compiler,
                                                          CodeGenerator,
                                                          UnaryExpr,
                                                          Parameters);

    case clvUNARY_COMPONENT_SELECTION:
        return cloIR_UNARY_EXPR_GenComponentSelectionCode(Compiler,
                                                          CodeGenerator,
                                                          UnaryExpr,
                                                          Parameters);

    case clvUNARY_POST_INC:
    case clvUNARY_POST_DEC:
    case clvUNARY_PRE_INC:
    case clvUNARY_PRE_DEC:
        return cloIR_UNARY_EXPR_GenIncOrDecCode(Compiler,
                                                CodeGenerator,
                                                UnaryExpr,
                                                Parameters);

    case clvUNARY_NEG:
        return cloIR_UNARY_EXPR_GenNegCode(Compiler,
                                           CodeGenerator,
                                           UnaryExpr,
                                           Parameters);

    case clvUNARY_NOT:
        return cloIR_UNARY_EXPR_GenNotCode(Compiler,
                                           CodeGenerator,
                                           UnaryExpr,
                                           Parameters);

    case clvUNARY_BITWISE_NOT:
        return cloIR_UNARY_EXPR_GenBitwiseNotCode(Compiler,
                                                  CodeGenerator,
                                                  UnaryExpr,
                                                  Parameters);

    case clvUNARY_NON_LVAL:
        if(Parameters->hint == clvEVALUATE_ONLY) return gcvSTATUS_OK;
        return cloIR_UNARY_EXPR_GenNonLvalCode(Compiler,
                                               CodeGenerator,
                                               UnaryExpr,
                                               Parameters);

    case clvUNARY_ADDR:
        if(Parameters->hint == clvEVALUATE_ONLY) return gcvSTATUS_OK;
        return cloIR_UNARY_EXPR_GenAddrCode(Compiler,
                                            CodeGenerator,
                                            UnaryExpr,
                                            Parameters);

    case clvUNARY_INDIRECTION:
        return cloIR_UNARY_EXPR_GenIndirectionCode(Compiler,
                                                   CodeGenerator,
                                                   UnaryExpr,
                                                   Parameters);

    case clvUNARY_CAST:
        return cloIR_UNARY_EXPR_GenCastCode(Compiler,
                                            CodeGenerator,
                                            UnaryExpr,
                                            Parameters);
    default:
        gcmASSERT(0);
        return gcvSTATUS_INVALID_ARGUMENT;
    }
}

static gceSTATUS
_GenConstantAssignCode(
    IN cloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN clsROPERAND *ROperand,
    IN clsGEN_CODE_DATA_TYPE ResType,
    OUT clsLOPERAND *ConstantOperand
    )
{
    gctREG_INDEX constantReg;

    gcmASSERT(!ROperand->isReg);
    constantReg = clNewTempRegs(Compiler,
                                gcGetDataTypeRegSize(ResType),
                                ResType.elementType);

    clsLOPERAND_InitializeTempReg(ConstantOperand,
                                  clvQUALIFIER_NONE,
                                  ResType,
                                  constantReg);

    return clGenAssignCode(Compiler,
                           LineNo,
                           StringNo,
                           ConstantOperand,
                           ROperand);
}

static gceSTATUS
_GenIndexAssignCode(
    IN cloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gctREG_INDEX IndexRegIndex,
    IN clsROPERAND * ROperand
    )
{
    clsLOPERAND lOperand;

    clsLOPERAND_InitializeTempReg(&lOperand,
                      clvQUALIFIER_NONE,
                      clmGenCodeDataType(T_INT),
                      IndexRegIndex);

    return clGenAssignCode(Compiler,
                LineNo,
                StringNo,
                &lOperand,
                ROperand);
}

gceSTATUS
_GetConstantSubscriptCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_BINARY_EXPR BinaryExpr,
    IN clsGEN_CODE_PARAMETERS * LeftParameters,
    IN clsGEN_CODE_PARAMETERS * RightParameters,
    IN OUT clsGEN_CODE_PARAMETERS * Parameters
    )
{
   gctUINT i;
   gctUINT offset;
   gctINT32 index;

/* Verify the arguments. */
   clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
   clmVERIFY_OBJECT(CodeGenerator, clvOBJ_CODE_GENERATOR);
   clmVERIFY_IR_OBJECT(BinaryExpr, clvIR_BINARY_EXPR);
   gcmASSERT(LeftParameters);
   gcmASSERT(RightParameters);
   gcmASSERT(Parameters);

   gcmASSERT(RightParameters->operandCount == 1);
   gcmASSERT(clmIsElementTypeInteger(clmGEN_CODE_elementType_GET(RightParameters->dataTypes[0].def)));
   gcmASSERT(!RightParameters->rOperands[0].isReg);

   index = RightParameters->rOperands[0].u.constant.values[0].intValue;
   if (clmDECL_IsVectorType(&BinaryExpr->leftOperand->decl)) {
      gcmASSERT(Parameters->operandCount == 1);

      if (Parameters->needLOperand) {
         Parameters->lOperands[0] = LeftParameters->lOperands[0];

         Parameters->lOperands[0].dataType =
                      gcGetVectorComponentDataType(LeftParameters->lOperands[0].dataType);
         Parameters->lOperands[0].vectorIndex.mode = clvINDEX_CONSTANT;
         Parameters->lOperands[0].vectorIndex.u.constant = (gctREG_INDEX) index;
      }

      if (Parameters->needROperand) {
         Parameters->rOperands[0] = LeftParameters->rOperands[0];

         Parameters->rOperands[0].dataType =
                      gcGetVectorComponentDataType(LeftParameters->rOperands[0].dataType);
         Parameters->rOperands[0].vectorIndex.mode = clvINDEX_CONSTANT;
         Parameters->rOperands[0].vectorIndex.u.constant = (gctREG_INDEX) index;
         if(Parameters->hasIOperand) {
            gceSTATUS status;

            gcmASSERT(Parameters->operandCount == 1);
            _clmAssignROperandToPreallocatedTempReg(Compiler,
                                Parameters,
                                &Parameters->rOperands[0],
                                status);
            if(gcmIS_ERROR(status)) return status;
         }
      }
   }
   else if (clmDECL_IsMat(&BinaryExpr->leftOperand->decl)) {
      gcmASSERT(Parameters->operandCount == 1);

      if (Parameters->needLOperand) {
          Parameters->lOperands[0] = LeftParameters->lOperands[0];
          Parameters->lOperands[0].dataType =
              gcGetMatrixColumnDataType(LeftParameters->lOperands[0].dataType);
          Parameters->lOperands[0].matrixIndex.mode = clvINDEX_CONSTANT;
          Parameters->lOperands[0].matrixIndex.u.constant    = (gctREG_INDEX) index;
      }

      if (Parameters->needROperand) {
          Parameters->rOperands[0] = LeftParameters->rOperands[0];

          Parameters->rOperands[0].dataType =
              gcGetMatrixColumnDataType(LeftParameters->rOperands[0].dataType);
          Parameters->rOperands[0].matrixIndex.mode = clvINDEX_CONSTANT;
          Parameters->rOperands[0].matrixIndex.u.constant    = (gctREG_INDEX) index;
          if(Parameters->hasIOperand) {
             gceSTATUS status;

             gcmASSERT(Parameters->operandCount == 1);
             _clmAssignROperandToPreallocatedTempReg(Compiler,
                                     Parameters,
                                     &Parameters->rOperands[0],
                                     status);
             if(gcmIS_ERROR(status)) return status;
          }
      }
   }
   else {
/* is an array */
      offset = Parameters->operandCount * index;

      gcmASSERT(offset < LeftParameters->operandCount);

      if (Parameters->needLOperand) {
          for (i = 0; i < Parameters->operandCount; i++) {
              gcmASSERT(LeftParameters->lOperands[offset + i].arrayIndex.mode == clvINDEX_NONE);
              Parameters->lOperands[i] = LeftParameters->lOperands[offset + i];
          }
      }

      if (Parameters->needROperand) {
         for (i = 0; i < Parameters->operandCount; i++) {
                 gcmASSERT(LeftParameters->rOperands[offset + i].arrayIndex.mode == clvINDEX_NONE);
                 Parameters->rOperands[i] = LeftParameters->rOperands[offset + i];
         }
         if(!(Parameters->hint & clvGEN_COMPONENT_SELECT_CODE) &&
            Parameters->hasIOperand) {
            gceSTATUS status;

            gcmASSERT(Parameters->operandCount == 1);
            _clmAssignROperandToPreallocatedTempReg(Compiler,
                                                    Parameters,
                                                    &Parameters->rOperands[0],
                                                    status);
            if(gcmIS_ERROR(status)) return status;
         }
      }
   }
   return gcvSTATUS_OK;
}

static gceSTATUS
_GenIndexAddCode(
    IN cloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gctREG_INDEX TargetIndexRegIndex,
    IN gctREG_INDEX SourceIndexRegIndex,
    IN clsROPERAND * ROperand
    )
{
    clsIOPERAND iOperand;
    clsROPERAND rOperand;

    clsIOPERAND_Initialize(&iOperand,
                   clmGenCodeDataType(T_INT),
                   TargetIndexRegIndex);

    clsROPERAND_InitializeTempReg(&rOperand,
                      clvQUALIFIER_NONE,
                      clmGenCodeDataType(T_INT),
                      SourceIndexRegIndex);

    return clGenArithmeticExprCode(Compiler,
                    LineNo,
                    StringNo,
                    clvOPCODE_ADD,
                    &iOperand,
                    &rOperand,
                    ROperand);
}

static gceSTATUS
_GenIndexScaleCode(
    IN cloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gctREG_INDEX IndexRegIndex,
    IN clsROPERAND * ROperand,
    IN gctINT32 ElementDataTypeSize
    )
{
    clsIOPERAND iOperand;
    clsROPERAND rOperand;

    clsIOPERAND_Initialize(&iOperand,
                   clmGenCodeDataType(T_INT),
                   IndexRegIndex);

    clsROPERAND_InitializeScalarConstant(&rOperand,
                                         clmGenCodeDataType(T_INT),
                                         long,
                                         ElementDataTypeSize);

    return clGenArithmeticExprCode(Compiler,
                       LineNo,
                       StringNo,
                       clvOPCODE_MUL,
                       &iOperand,
                       ROperand,
                       &rOperand);
}

gceSTATUS
_GetNonConstantSubscriptCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_BINARY_EXPR BinaryExpr,
    IN clsGEN_CODE_PARAMETERS * LeftParameters,
    IN clsGEN_CODE_PARAMETERS * RightParameters,
    IN OUT clsGEN_CODE_PARAMETERS * Parameters
    )
{
    gceSTATUS status;
    gctREG_INDEX indexRegIndex;
    gctUINT    i;
    gctINT32 elementDataTypeSize;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_OBJECT(CodeGenerator, clvOBJ_CODE_GENERATOR);
    clmVERIFY_IR_OBJECT(BinaryExpr, clvIR_BINARY_EXPR);
    gcmASSERT(LeftParameters);
    gcmASSERT(RightParameters);
    gcmASSERT(Parameters);

    gcmASSERT(cloIR_OBJECT_GetType(&BinaryExpr->rightOperand->base) != clvIR_CONSTANT);
    gcmASSERT(RightParameters->operandCount == 1);
    gcmASSERT(RightParameters->rOperands != gcvNULL && RightParameters->rOperands[0].isReg);

    if (!clmDECL_IsPointerType(&BinaryExpr->leftOperand->decl)) {
        if (clmDECL_IsVectorType(&BinaryExpr->leftOperand->decl)) {
            gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                            BinaryExpr->rightOperand->base.lineNo,
                                            BinaryExpr->rightOperand->base.stringNo,
                                            clvREPORT_ERROR,
                                            "not support for dynamic indexing of vectors"));

            return gcvSTATUS_NOT_SUPPORTED;
        }
        else if (clmDECL_IsMat(&BinaryExpr->leftOperand->decl)) {
            gcmASSERT(Parameters->operandCount == 1);

            if (LeftParameters->rOperands[0].arrayIndex.mode == clvINDEX_REG)
            {
                indexRegIndex = clNewTempRegs(Compiler, 1, clvTYPE_VOID);

                status = _GenIndexAddCode(Compiler,
                                          BinaryExpr->rightOperand->base.lineNo,
                                          BinaryExpr->rightOperand->base.stringNo,
                                          indexRegIndex,
                                          LeftParameters->rOperands[0].arrayIndex.u.indexRegIndex,
                                          &RightParameters->rOperands[0]);
                if (gcmIS_ERROR(status)) return status;

                LeftParameters->rOperands[0].arrayIndex.mode = clvINDEX_NONE;
            }
            else if (RightParameters->rOperands[0].arrayIndex.mode != clvINDEX_NONE ||
                     RightParameters->rOperands[0].matrixIndex.mode != clvINDEX_NONE ||
                     RightParameters->rOperands[0].vectorIndex.mode != clvINDEX_NONE ||
                     !_IsTempRegQualifier(RightParameters->rOperands[0].u.reg.qualifier))
            {
                indexRegIndex = clNewTempRegs(Compiler, 1, clvTYPE_VOID);

                status = _GenIndexAssignCode(Compiler,
                                             BinaryExpr->rightOperand->base.lineNo,
                                             BinaryExpr->rightOperand->base.stringNo,
                                             indexRegIndex,
                                             &RightParameters->rOperands[0]);

                if (gcmIS_ERROR(status)) return status;
            }
            else
            {
                indexRegIndex = RightParameters->rOperands[0].u.reg.regIndex;
            }

            if (Parameters->needLOperand)
            {
                Parameters->lOperands[0] = LeftParameters->lOperands[0];

                Parameters->lOperands[0].dataType =
                                gcGetMatrixColumnDataType(LeftParameters->lOperands[0].dataType);

                Parameters->lOperands[0].matrixIndex.mode = clvINDEX_REG;
                Parameters->lOperands[0].matrixIndex.u.indexRegIndex    = indexRegIndex;
            }

            if (Parameters->needROperand)
            {
                if(Parameters->hasIOperand) {
                    gceSTATUS status;
                    clsROPERAND rOperand;

                    rOperand = LeftParameters->rOperands[0];
                    rOperand.dataType = gcGetMatrixColumnDataType(LeftParameters->rOperands[0].dataType);

                    rOperand.matrixIndex.mode = clvINDEX_REG;
                    rOperand.matrixIndex.u.indexRegIndex = indexRegIndex;

                    _clmAssignROperandToPreallocatedTempReg(Compiler,
                                                            Parameters,
                                                            &rOperand,
                                                            status);
                    if(gcmIS_ERROR(status)) return status;
                }
                else {
                    Parameters->rOperands[0] = LeftParameters->rOperands[0];

                    Parameters->rOperands[0].dataType =
                                 gcGetMatrixColumnDataType(LeftParameters->rOperands[0].dataType);

                    Parameters->rOperands[0].matrixIndex.mode = clvINDEX_REG;
                    Parameters->rOperands[0].matrixIndex.u.indexRegIndex = indexRegIndex;
                }
            }
            return gcvSTATUS_OK;
        }
    }

/* Either a pointer or an array */
    if (Parameters->operandCount > 1 || clmDECL_IsMat(&BinaryExpr->exprBase.decl)) {
        gctSIZE_T dataTypeRegSize;

        indexRegIndex = clNewTempRegs(Compiler, 1, clvTYPE_VOID);
        for (i = 0, elementDataTypeSize = 0; i < Parameters->operandCount; i++) {
            dataTypeRegSize = gcGetDataTypeRegSize(Parameters->dataTypes[i].def);
            elementDataTypeSize += _clmGetTempRegIndexOffset(dataTypeRegSize,
                                                             clmGEN_CODE_elementType_GET(Parameters->dataTypes[i].def));
        }

        status = _GenIndexScaleCode(Compiler,
                                    BinaryExpr->rightOperand->base.lineNo,
                                    BinaryExpr->rightOperand->base.stringNo,
                                    indexRegIndex,
                                    &RightParameters->rOperands[0],
                                    elementDataTypeSize);
        if (gcmIS_ERROR(status)) return status;
    }
    else if (RightParameters->rOperands[0].arrayIndex.mode != clvINDEX_NONE ||
             RightParameters->rOperands[0].matrixIndex.mode != clvINDEX_NONE ||
             RightParameters->rOperands[0].vectorIndex.mode != clvINDEX_NONE ||
             !_IsTempRegQualifier(RightParameters->rOperands[0].u.reg.qualifier)) {
        indexRegIndex = clNewTempRegs(Compiler, 1, clvTYPE_VOID);

        status = _GenIndexAssignCode(Compiler,
                                     BinaryExpr->rightOperand->base.lineNo,
                                     BinaryExpr->rightOperand->base.stringNo,
                                     indexRegIndex,
                                     &RightParameters->rOperands[0]);
        if (gcmIS_ERROR(status)) return status;
    }
    else {
        gctSIZE_T dataTypeRegSize;

        indexRegIndex = clNewTempRegs(Compiler, 1, clvTYPE_VOID);
        dataTypeRegSize = gcGetDataTypeRegSize(Parameters->dataTypes[0].def);
        elementDataTypeSize = _clmGetTempRegIndexOffset(dataTypeRegSize,
                                                        clmGEN_CODE_elementType_GET(Parameters->dataTypes[0].def));
        status = _GenIndexScaleCode(Compiler,
                                    BinaryExpr->rightOperand->base.lineNo,
                                    BinaryExpr->rightOperand->base.stringNo,
                                    indexRegIndex,
                                    &RightParameters->rOperands[0],
                                    elementDataTypeSize);
        if (gcmIS_ERROR(status)) return status;
    }

    if (Parameters->needLOperand) {
        for (i = 0; i < Parameters->operandCount; i++) {
            gcmASSERT(LeftParameters->lOperands[i].matrixIndex.mode == clvINDEX_NONE);
            Parameters->lOperands[i] = LeftParameters->lOperands[i];
            Parameters->lOperands[i].arrayIndex.mode = clvINDEX_REG;
            Parameters->lOperands[i].arrayIndex.u.indexRegIndex = indexRegIndex;
        }
    }

    if (Parameters->needROperand) {
        if(!(Parameters->hint & clvGEN_COMPONENT_SELECT_CODE) &&
           Parameters->hasIOperand) {
            gceSTATUS status;
            clsROPERAND rOperand;

            gcmASSERT(Parameters->operandCount == 1);
            gcmASSERT(LeftParameters->rOperands[0].matrixIndex.mode == clvINDEX_NONE);
            rOperand = LeftParameters->rOperands[0];
            rOperand.arrayIndex.mode = clvINDEX_REG;
            rOperand.arrayIndex.u.indexRegIndex = indexRegIndex;

            _clmAssignROperandToPreallocatedTempReg(Compiler,
                                                    Parameters,
                                                    &rOperand,
                                                    status);
            if(gcmIS_ERROR(status)) return status;
        }
        else {
            for (i = 0; i < Parameters->operandCount; i++) {
                gcmASSERT(LeftParameters->rOperands[i].matrixIndex.mode == clvINDEX_NONE);
                Parameters->rOperands[i] = LeftParameters->rOperands[i];
                Parameters->rOperands[i].arrayIndex.mode = clvINDEX_REG;
                Parameters->rOperands[i].arrayIndex.u.indexRegIndex = indexRegIndex;
            }
        }
    }
    return gcvSTATUS_OK;
}

static gctINT
_GenArrayOffset(
IN cloCOMPILER Compiler,
IN cloCODE_GENERATOR CodeGenerator,
IN clsARRAY *Array,
IN cloIR_BINARY_EXPR Subscript,
OUT clsGEN_CODE_PARAMETERS *Offset
)
{
   gceSTATUS status;
   clsGEN_CODE_PARAMETERS leftParameters[1], rightParameters[1];
   gctUINT arrayOffset;
   gctINT dim = -1;

   clsGEN_CODE_PARAMETERS_Initialize(leftParameters,
                     gcvFALSE,
                     gcvTRUE);

   clsGEN_CODE_PARAMETERS_Initialize(rightParameters,
                     gcvFALSE,
                     gcvTRUE);

   if(clmIR_EXPR_IsBinaryType(Subscript->leftOperand, clvBINARY_MULTI_DIM_SUBSCRIPT)) {
      gcmONERROR(clsGEN_CODE_PARAMETERS_AllocateOperands(Compiler,
                                                         leftParameters,
                                                         &Subscript->leftOperand->decl));
      dim = _GenArrayOffset(Compiler,
                            CodeGenerator,
                            Array,
                            (cloIR_BINARY_EXPR)&Subscript->leftOperand->base,
                            leftParameters);
      if(dim < 0) goto OnError;
   }
   else {
      gcmONERROR(cloIR_OBJECT_Accept(Compiler,
                         &Subscript->leftOperand->base,
                         &CodeGenerator->visitor,
                         leftParameters));

      clmGetArrayElementCount(*Array, 1, arrayOffset);
      if(arrayOffset) {
         gcmONERROR(clGenScaledIndexOperand(Compiler,
                                            Subscript->exprBase.base.lineNo,
                                            Subscript->exprBase.base.stringNo,
                                            &leftParameters->rOperands[0],
                                            arrayOffset,
                                            &leftParameters->rOperands[0]));
      }
      dim = 1;
   }

   gcmONERROR(cloIR_OBJECT_Accept(Compiler,
                      &Subscript->rightOperand->base,
                      &CodeGenerator->visitor,
                      rightParameters));

   dim++;
   clmGetArrayElementCount(*Array, dim, arrayOffset);
   if(arrayOffset) {
         gcmONERROR(clGenScaledIndexOperand(Compiler,
                                            Subscript->exprBase.base.lineNo,
                                            Subscript->exprBase.base.stringNo,
                                            &rightParameters->rOperands[0],
                                            arrayOffset,
                                            &rightParameters->rOperands[0]));
   }

   gcmONERROR(_AddROperandOffset(Compiler,
                                 Subscript->exprBase.base.lineNo,
                                 Subscript->exprBase.base.stringNo,
                                 &leftParameters->rOperands[0],
                                 &rightParameters->rOperands[0],
                                 &Offset->rOperands[0]));

OnError:
   clsGEN_CODE_PARAMETERS_Finalize(leftParameters);
   clsGEN_CODE_PARAMETERS_Finalize(rightParameters);
   return dim;
}

static gctINT
_EvaluateArrayOffset(
IN cloCOMPILER Compiler,
IN cloCODE_GENERATOR CodeGenerator,
IN clsARRAY *Array,
IN gctINT Dim,
IN cloIR_BINARY_EXPR Subscript,
OUT clsGEN_CODE_PARAMETERS *Offset
)
{
   gceSTATUS status;
   cluCONSTANT_VALUE leftOffset[1], rightOffset[1];
   clsGEN_CODE_PARAMETERS leftParameters[1], rightParameters[1];
   gctUINT arrayOffset;
   gctINT dim = -1;

   clsGEN_CODE_PARAMETERS_Initialize(leftParameters,
                     gcvFALSE,
                     gcvTRUE);

   clsGEN_CODE_PARAMETERS_Initialize(rightParameters,
                     gcvFALSE,
                     gcvTRUE);

   leftParameters->hint = clvEVALUATE_ONLY;
   gcmONERROR(cloIR_OBJECT_Accept(Compiler,
                      &Subscript->leftOperand->base,
                      &CodeGenerator->visitor,
                      leftParameters));

   if (leftParameters->constant == gcvNULL) {
      dim = 0;
      goto OnError;
   }
   leftOffset->intValue = cloIR_CONSTANT_GetIntegerValue(leftParameters->constant);
   clmGetArrayElementCount(*Array, Dim, arrayOffset);
   if(arrayOffset) {
       leftOffset->intValue *= arrayOffset;
   }

   if(clmIR_EXPR_IsBinaryType(Subscript->rightOperand, clvBINARY_MULTI_DIM_SUBSCRIPT)) {
      dim = _EvaluateArrayOffset(Compiler,
                                 CodeGenerator,
                                 Array,
                                 Dim + 1,
                                 (cloIR_BINARY_EXPR)&Subscript->rightOperand->base,
                                 rightParameters);
      if(dim < 1) goto OnError;
   }
   else {
      rightParameters->hint = clvEVALUATE_ONLY;
      gcmONERROR(cloIR_OBJECT_Accept(Compiler,
                         &Subscript->rightOperand->base,
                         &CodeGenerator->visitor,
                         rightParameters));
      dim = Dim + 1;
   }
   if (rightParameters->constant == gcvNULL) {
      dim = 0;
      goto OnError;
   }
   rightOffset->intValue = cloIR_CONSTANT_GetIntegerValue(rightParameters->constant);
   clmGetArrayElementCount(*Array, dim, arrayOffset);
   if(arrayOffset) {
      rightOffset->intValue *= arrayOffset;
   }

   rightOffset->intValue += leftOffset->intValue;
   Offset->constant = rightParameters->constant;
   rightParameters->constant = gcvNULL;

   Offset->constant->values[0] = rightOffset[0];
OnError:
   clsGEN_CODE_PARAMETERS_Finalize(leftParameters);
   clsGEN_CODE_PARAMETERS_Finalize(rightParameters);
   return dim;
}

static
gceSTATUS
cloIR_BINARY_EXPR_GenSubscriptCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_BINARY_EXPR BinaryExpr,
    IN OUT clsGEN_CODE_PARAMETERS * Parameters
    )
{
   gceSTATUS status;
   clsGEN_CODE_PARAMETERS leftParameters, rightParameters;
   gctINT dim;

/* Verify the arguments. */
   clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
   clmVERIFY_OBJECT(CodeGenerator, clvOBJ_CODE_GENERATOR);
   clmVERIFY_IR_OBJECT(BinaryExpr, clvIR_BINARY_EXPR);
   gcmASSERT(Parameters);

/* Generate the code of the right operand */
   gcmASSERT(BinaryExpr->rightOperand);

   clsGEN_CODE_PARAMETERS_Initialize(&leftParameters,
                     Parameters->needLOperand,
                     Parameters->needROperand);

   clsGEN_CODE_PARAMETERS_Initialize(&rightParameters,
                     gcvFALSE,
                     Parameters->needLOperand || Parameters->needROperand);

   if(BinaryExpr->leftOperand->decl.ptrDominant && clmDECL_IsUnderlyingArray(&BinaryExpr->leftOperand->decl)) {
      dim = 0;
   }
   else dim = 1;

   if(clmIR_EXPR_IsBinaryType(BinaryExpr->rightOperand, clvBINARY_MULTI_DIM_SUBSCRIPT)) {
      gcmONERROR(clsGEN_CODE_PARAMETERS_AllocateOperands(Compiler,
                                                         &rightParameters,
                                                         &BinaryExpr->rightOperand->decl));

      dim = _GenArrayOffset(Compiler,
                            CodeGenerator,
                            &BinaryExpr->leftOperand->decl.array,
                            (cloIR_BINARY_EXPR)&BinaryExpr->rightOperand->base,
                            &rightParameters);

      if(dim < 0) {
     gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                     BinaryExpr->exprBase.base.lineNo,
                     BinaryExpr->exprBase.base.stringNo,
                     clvREPORT_ERROR,
                     "internal error: failed to compute array indices"));

         return gcvSTATUS_INVALID_DATA;
      }
   }
   else {
      gctINT32 arrayOffset;

      rightParameters.hint = clvGEN_INDEX_CODE;

      gcmONERROR(cloIR_OBJECT_Accept(Compiler,
                         &BinaryExpr->rightOperand->base,
                         &CodeGenerator->visitor,
                         &rightParameters));

      clmGetArrayElementCount(BinaryExpr->leftOperand->decl.array, dim, arrayOffset);
      if(arrayOffset) {
         gcmONERROR(clGenScaledIndexOperand(Compiler,
                                            BinaryExpr->exprBase.base.lineNo,
                                            BinaryExpr->exprBase.base.stringNo,
                                            &rightParameters.rOperands[0],
                                            arrayOffset,
                                            &rightParameters.rOperands[0]));
      }
   }

/* Generate the code of the left operand */
   gcmASSERT(BinaryExpr->leftOperand);

   leftParameters.hint = clvGEN_SUBSCRIPT_CODE;
   gcmONERROR(cloIR_OBJECT_Accept(Compiler,
                      &BinaryExpr->leftOperand->base,
                      &CodeGenerator->visitor,
                      &leftParameters));

   if (Parameters->needLOperand || Parameters->needROperand) {
     clsNAME *variable = gcvNULL;

     gcmONERROR(clsGEN_CODE_PARAMETERS_AllocateOperands(Compiler,
                                                        Parameters,
                                                        &BinaryExpr->exprBase.decl));

     gcmASSERT(rightParameters.operandCount == 1);
     gcmASSERT(rightParameters.rOperands != gcvNULL);

     variable = clParseFindLeafName(Compiler,
                                    BinaryExpr->leftOperand);

     if(Parameters->hint & clvGEN_ADDR_CODE) {
        clsIOPERAND iOperand[1];
        clsROPERAND scaledIndex[1];
        gctUINT elementDataTypeSize = 0;

        Parameters->operandCount = 1;
        elementDataTypeSize = clsDECL_GetByteSize(&BinaryExpr->exprBase.decl);
        gcmONERROR(clGenScaledIndexOperandWithOffset(Compiler,
                                                     BinaryExpr->exprBase.base.lineNo,
                                                     BinaryExpr->exprBase.base.stringNo,
                                                     &rightParameters.rOperands[0],
                                                     elementDataTypeSize,
                                                     leftParameters.dataTypes[0].byteOffset,
                                                     scaledIndex));

        if ((Parameters->hint & clvGEN_SAVE_ADDRESS_OFFSET) && !scaledIndex->isReg) { /* Save address offset && is a constant */
            if(Parameters->needLOperand || Parameters->needROperand) {
                Parameters->dataTypes[0].savedByteOffset = scaledIndex->u.constant.values[0].intValue;
            }
        }

        if (_clmIsROperandConstantZero(scaledIndex)) {
           if(Parameters->needLOperand) {
              Parameters->lOperands[0] = leftParameters.lOperands[0];
           }
           if(Parameters->needROperand) {
              Parameters->rOperands[0] = leftParameters.rOperands[0];
           }
        }
        else {
           if(clmIsElementTypeHighPrecision(scaledIndex->dataType.elementType)) {
               clsGEN_CODE_DATA_TYPE newDataType;

               if(clmIsElementTypeUnsigned(scaledIndex->dataType.elementType)) {
                    newDataType = clmGenCodeDataType(T_UINT);
               }
               else {
                    newDataType = clmGenCodeDataType(T_INT);
               }

               gcmONERROR(clsROPERAND_ChangeDataTypeFamily(Compiler,
                                                           BinaryExpr->exprBase.base.lineNo,
                                                           BinaryExpr->exprBase.base.stringNo,
                                                           gcvFALSE,
                                                           newDataType,
                                                           scaledIndex));
           }

           if(Parameters->needLOperand) {
              clsROPERAND rOperand[1];

              clsIOPERAND_New(Compiler, iOperand, leftParameters.lOperands[0].dataType);
              clsROPERAND_InitializeUsingLOperand(rOperand, &leftParameters.lOperands[0]);
              gcmONERROR(clGenGenericCode2(Compiler,
                                           BinaryExpr->exprBase.base.lineNo,
                                           BinaryExpr->exprBase.base.stringNo,
                                           clvOPCODE_ADD,
                                           iOperand,
                                           rOperand,
                                           scaledIndex));
              clsLOPERAND_InitializeUsingIOperand(&Parameters->lOperands[0], iOperand);
           }

           if(Parameters->needROperand) {
              clsIOPERAND_New(Compiler, iOperand, leftParameters.rOperands[0].dataType);
              gcmONERROR(clGenGenericCode2(Compiler,
                                           BinaryExpr->exprBase.base.lineNo,
                                           BinaryExpr->exprBase.base.stringNo,
                                           clvOPCODE_ADD,
                                           iOperand,
                                           &leftParameters.rOperands[0],
                                           scaledIndex));
               clsROPERAND_InitializeUsingIOperand(&Parameters->rOperands[0], iOperand);
           }
        }
     }
     else {
       if((leftParameters.hint & (clvGEN_DEREF_CODE | clvGEN_DEREF_STRUCT_CODE)) ||
          clmDECL_IsPointerType(&BinaryExpr->leftOperand->decl) ||
          (clmDECL_IsArray(&BinaryExpr->leftOperand->decl) &&
           BinaryExpr->leftOperand->decl.dataType->addrSpaceQualifier == clvQUALIFIER_LOCAL) ||
          (variable && clmGEN_CODE_checkVariableForMemory(variable))) {

          gcmONERROR(clGenDerefPointerCode(Compiler,
                       &BinaryExpr->exprBase,
                       &leftParameters,
                       &rightParameters,
                       Parameters));
       }
       else {
          /* Generate the subscripting code */
          if (!rightParameters.rOperands[0].isReg) {
              gcmONERROR(_GetConstantSubscriptCode(Compiler,
                                                   CodeGenerator,
                                                   BinaryExpr,
                                                   &leftParameters,
                                                   &rightParameters,
                                                   Parameters));
          }
          else {
              gcmONERROR(_GetNonConstantSubscriptCode(Compiler,
                                                      CodeGenerator,
                                                      BinaryExpr,
                                                      &leftParameters,
                                                      &rightParameters,
                                                      Parameters));
          }
       }
     }
   }

OnError:
   clsGEN_CODE_PARAMETERS_Finalize(&leftParameters);
   clsGEN_CODE_PARAMETERS_Finalize(&rightParameters);

   return gcvSTATUS_OK;
}

gceSTATUS
cloIR_BINARY_EXPR_GenArithmeticCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_BINARY_EXPR BinaryExpr,
    IN OUT clsGEN_CODE_PARAMETERS * Parameters
    )
{
    gceSTATUS    status = gcvSTATUS_OK;
    clsGEN_CODE_PARAMETERS leftParameters, rightParameters;
    gctUINT    i;
    clsIOPERAND    iOperand[1];
    cleOPCODE    opcode;
    cleGEN_CODE_HINT hint = clvGEN_NO_HINT;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_OBJECT(CodeGenerator, clvOBJ_CODE_GENERATOR);
    clmVERIFY_IR_OBJECT(BinaryExpr, clvIR_BINARY_EXPR);
    gcmASSERT(Parameters);

    if (Parameters->needROperand || Parameters->needLOperand) {
        status = clsGEN_CODE_PARAMETERS_AllocateOperands(Compiler,
                             Parameters,
                             &BinaryExpr->exprBase.decl);
        if (gcmIS_ERROR(status)) return status ;
    }

    /* Generate the code of the left operand */
    if(clmDECL_IsFloatOrVec(&BinaryExpr->exprBase.decl) &&
       (CodeGenerator->fpConfig & cldFpFMA) &&
       (BinaryExpr->type == clvBINARY_ADD || BinaryExpr->type == clvBINARY_SUB)) {
    hint = clvGEN_FMA;
    }
    gcmASSERT(BinaryExpr->leftOperand);

    clsGEN_CODE_PARAMETERS_Initialize(&leftParameters,
                      gcvFALSE,
                      Parameters->needLOperand || Parameters->needROperand);
    leftParameters.hint = hint;
    status = cloIR_OBJECT_Accept(Compiler,
                     &BinaryExpr->leftOperand->base,
                     &CodeGenerator->visitor,
                     &leftParameters);
    if (gcmIS_ERROR(status)) return status;

    clsGEN_CODE_PARAMETERS_Initialize(&rightParameters,
                      gcvFALSE,
                      Parameters->needLOperand || Parameters->needROperand);
    do {
      if(hint == clvGEN_FMA && leftParameters.hint ==  clvGEN_FOUND_MULTIPLICAND) {
             cloIR_POLYNARY_EXPR funcCall;
         cloIR_BINARY_EXPR multExpr;
         clsGEN_CODE_PARAMETERS operandsParameters[3];

         funcCall = _CreateFuncCall(Compiler,
                                    clvOPCODE_FMA,
                                    &BinaryExpr->exprBase);
         gcmASSERT(funcCall);
         if (!funcCall) {
            status = gcvSTATUS_INVALID_ARGUMENT;
            goto OnError;
         }
         gcmASSERT(cloIR_OBJECT_GetType(&BinaryExpr->leftOperand->base) == clvIR_BINARY_EXPR);
         multExpr = (cloIR_BINARY_EXPR) &BinaryExpr->leftOperand->base;

         gcmVERIFY_OK(cloIR_SET_AddMember(Compiler,
                                          funcCall->operands,
                                          &multExpr->leftOperand->base));

         gcmVERIFY_OK(cloIR_SET_AddMember(Compiler,
                                          funcCall->operands,
                                          &multExpr->rightOperand->base));

         gcmVERIFY_OK(cloIR_SET_AddMember(Compiler,
                                          funcCall->operands,
                                          &BinaryExpr->rightOperand->base));

         gcmONERROR(cloIR_OBJECT_Accept(Compiler,
                                        &BinaryExpr->rightOperand->base,
                                        &CodeGenerator->visitor,
                                        &rightParameters));

         if(BinaryExpr->type == clvBINARY_SUB) {
            clsROPERAND zeroROperand[1];
            clsIOPERAND intermIOperand[1];

            clsIOPERAND_New(Compiler, intermIOperand, Parameters->dataTypes[0].def);
            _InitializeROperandConstant(zeroROperand, clmGenCodeDataType(T_FLOAT), 0);
            gcmONERROR(clGenArithmeticExprCode(Compiler,
                                               BinaryExpr->exprBase.base.lineNo,
                                               BinaryExpr->exprBase.base.stringNo,
                                               clvOPCODE_SUB,
                                               intermIOperand,
                                               zeroROperand,
                                               rightParameters.rOperands));
            clsROPERAND_InitializeUsingIOperand(rightParameters.rOperands, intermIOperand);

            /* Update right operand type to result type due to subtraction above */
            BinaryExpr->rightOperand->decl.dataType = BinaryExpr->exprBase.decl.dataType;
         }
         operandsParameters[0] = leftParameters;
         operandsParameters[1] = leftParameters;
         operandsParameters[1].rOperands = leftParameters.rOperands + 1;
         operandsParameters[2] = rightParameters;

         status = _GenImplicitConvToType(Compiler,
                                         &BinaryExpr->exprBase.decl,
                                         multExpr->leftOperand,
                                         &operandsParameters[0]);
         if (gcmIS_ERROR(status)) return status;

         status = _GenImplicitConvToType(Compiler,
                                         &BinaryExpr->exprBase.decl,
                                         multExpr->rightOperand,
                                         &operandsParameters[1]);
         if (gcmIS_ERROR(status)) return status;

         status = _GenImplicitConvToType(Compiler,
                                         &BinaryExpr->exprBase.decl,
                                         BinaryExpr->rightOperand,
                                         &operandsParameters[2]);
         if (gcmIS_ERROR(status)) return status;


         gcmONERROR(cloCOMPILER_BindBuiltinFuncCall(Compiler,
                                                    funcCall));
         clmGEN_CODE_GetParametersIOperand(Compiler, iOperand, Parameters, Parameters->dataTypes[0].def);
             gcmONERROR(clGenBuiltinFunctionCode(Compiler,
                                                 CodeGenerator,
                                                 funcCall,
                                                 3,
                                                 operandsParameters,
                                                 iOperand,
                                                 Parameters,
                                                 gcvTRUE));

         clsROPERAND_InitializeUsingIOperand(Parameters->rOperands, iOperand);
         break;
    }

    /* Generate the code of the right operand */
    gcmASSERT(BinaryExpr->rightOperand);
    rightParameters.hint = hint;
    gcmONERROR(cloIR_OBJECT_Accept(Compiler,
                                   &BinaryExpr->rightOperand->base,
                                   &CodeGenerator->visitor,
                                   &rightParameters));

    if(hint == clvGEN_FMA && rightParameters.hint == clvGEN_FOUND_MULTIPLICAND) {
         cloIR_POLYNARY_EXPR funcCall;
         cloIR_BINARY_EXPR multExpr;
         clsGEN_CODE_PARAMETERS operandsParameters[3];
         clsROPERAND zeroROperand[1];
         clsIOPERAND intermIOperand[1];
         clsIOPERAND *resOperand;

         gcmONERROR(clGenImplicitConversion(Compiler,
                                            BinaryExpr->leftOperand,
                                            BinaryExpr->rightOperand,
                                            &leftParameters,
                                            &rightParameters));

         funcCall = _CreateFuncCall(Compiler,
                                    clvOPCODE_FMA,
                                    &BinaryExpr->exprBase);
         if (!funcCall) {
            status = gcvSTATUS_INVALID_ARGUMENT;
            goto OnError;
         }

         gcmASSERT(cloIR_OBJECT_GetType(&BinaryExpr->rightOperand->base) == clvIR_BINARY_EXPR);
         multExpr = (cloIR_BINARY_EXPR) &BinaryExpr->rightOperand->base;

         gcmONERROR(cloIR_SET_AddMember(Compiler,
                                        funcCall->operands,
                                        &multExpr->leftOperand->base));

         gcmONERROR(cloIR_SET_AddMember(Compiler,
                                        funcCall->operands,
                                        &multExpr->rightOperand->base));

         gcmONERROR(cloIR_SET_AddMember(Compiler,
                                        funcCall->operands,
                                        &BinaryExpr->leftOperand->base));

         gcmONERROR(cloCOMPILER_BindBuiltinFuncCall(Compiler,
                                                    funcCall));

         if(BinaryExpr->type == clvBINARY_SUB) {
            clsIOPERAND_New(Compiler, intermIOperand, Parameters->dataTypes[0].def);
            _InitializeROperandConstant(zeroROperand, clmGenCodeDataType(T_FLOAT), 0);
            gcmONERROR(clGenArithmeticExprCode(Compiler,
                                               BinaryExpr->exprBase.base.lineNo,
                                               BinaryExpr->exprBase.base.stringNo,
                                               clvOPCODE_SUB,
                                               intermIOperand,
                                               zeroROperand,
                                               leftParameters.rOperands));
            clsROPERAND_InitializeUsingIOperand(leftParameters.rOperands, intermIOperand);
         }
         operandsParameters[0] = rightParameters;
         operandsParameters[1] = rightParameters;
         operandsParameters[1].rOperands = rightParameters.rOperands + 1;
         operandsParameters[2] = leftParameters;

             clmGEN_CODE_GetParametersIOperand(Compiler, iOperand, Parameters, Parameters->dataTypes[0].def);
             gcmONERROR(clGenBuiltinFunctionCode(Compiler,
                                                 CodeGenerator,
                                                 funcCall,
                                                 3,
                                                 operandsParameters,
                                                 iOperand,
                                                 Parameters,
                                                 gcvTRUE));

         if(BinaryExpr->type == clvBINARY_SUB) {
            clsROPERAND intermROperand[1];

            clsIOPERAND_New(Compiler, intermIOperand, Parameters->dataTypes[0].def);
            clsROPERAND_InitializeUsingIOperand(intermROperand, iOperand);
            gcmONERROR(clGenArithmeticExprCode(Compiler,
                                               BinaryExpr->exprBase.base.lineNo,
                                               BinaryExpr->exprBase.base.stringNo,
                                               clvOPCODE_SUB,
                                               intermIOperand,
                                               zeroROperand,
                                               intermROperand));
            resOperand = intermIOperand;
         }
         else resOperand = iOperand;
         if(Parameters->needLOperand) {
            clsLOPERAND_InitializeUsingIOperand(Parameters->lOperands, resOperand);
         }
         if(Parameters->needROperand) {
            clsROPERAND_InitializeUsingIOperand(Parameters->rOperands, resOperand);
         }

         break;
    }

    /* Generate the arithmetic code */
    if (Parameters->needROperand || Parameters->needLOperand) {
       switch (BinaryExpr->type) {
       case clvBINARY_ADD:
           if(!clmDECL_IsMat(&BinaryExpr->exprBase.decl) &&
              clmDECL_IsFloatingType(&BinaryExpr->exprBase.decl) &&
              CodeGenerator->needFloatingPointAccuracy) {
              opcode = clvOPCODE_FADD;
           }
           else opcode = clvOPCODE_ADD;
           break;

       case clvBINARY_SUB:
           if(!clmDECL_IsMat(&BinaryExpr->exprBase.decl) &&
              clmDECL_IsFloatingType(&BinaryExpr->exprBase.decl) &&
              CodeGenerator->needFloatingPointAccuracy) {
              opcode = clvOPCODE_FSUB;
           }
           else opcode = clvOPCODE_SUB;
           break;

       case clvBINARY_MUL:
           if(clmDECL_IsIntegerType(&BinaryExpr->exprBase.decl)) {
              opcode = clvOPCODE_IMUL;
           }
           else {
              if(Parameters->hint == clvGEN_FMA) {
                 gctPOINTER pointer;
                 gcmONERROR(clGenImplicitConversion(Compiler,
                                                    BinaryExpr->leftOperand,
                                                    BinaryExpr->rightOperand,
                                                    &leftParameters,
                                                    &rightParameters));

                  if(Parameters->rOperands)  {
                     gcmVERIFY_OK(cloCOMPILER_Free(Compiler, Parameters->rOperands));
                  }
                  gcmONERROR(cloCOMPILER_Allocate(Compiler,
                                                  (gctSIZE_T)sizeof(clsROPERAND) << 1,
                                                  (gctPOINTER *) &pointer));
                  Parameters->rOperands = pointer;
                  Parameters->rOperands[0] = leftParameters.rOperands[0];
                  Parameters->rOperands[1] = rightParameters.rOperands[0];
                  clsGEN_CODE_PARAMETERS_Finalize(&leftParameters);
                  clsGEN_CODE_PARAMETERS_Finalize(&rightParameters);

                  Parameters->hint = clvGEN_FOUND_MULTIPLICAND;
                  return gcvSTATUS_OK;
               }
               else if(!clmDECL_IsMat(&BinaryExpr->exprBase.decl) &&
                        CodeGenerator->needFloatingPointAccuracy) {
                  opcode = clvOPCODE_FMUL;
               }
               else opcode = clvOPCODE_MUL;
           }
            break;

       case clvBINARY_DIV:
           if(clmDECL_IsIntegerType(&BinaryExpr->exprBase.decl)) {
              opcode = clvOPCODE_IDIV;
           }
           else {
              opcode = clvOPCODE_DIV;
           }
           break;

       case clvBINARY_MOD:
           if(clmDECL_IsIntegerType(&BinaryExpr->exprBase.decl)) {
              opcode = clvOPCODE_MOD;
           }
           else {
              opcode = clvOPCODE_FMOD;
           }
           break;

       default:
           gcmASSERT(0);
           status = gcvSTATUS_INVALID_ARGUMENT;
           goto OnError;
       }

       do {
          gctBOOL isPointerArithmetic;
          clsGEN_CODE_PARAMETERS *lParams = &leftParameters;
          clsGEN_CODE_PARAMETERS *rParams = &rightParameters;

          isPointerArithmetic = clmIsPointerOperand(BinaryExpr->leftOperand);
          if(!isPointerArithmetic) {
              if(clmIsPointerOperand(BinaryExpr->rightOperand)) {
                  lParams = &rightParameters;
                  rParams = &leftParameters;
                  isPointerArithmetic = gcvTRUE;
              }
          }
          if(isPointerArithmetic) {
             clsROPERAND intermROperand[1];

             intermROperand->dataType = Parameters->dataTypes[0].def;
             gcmONERROR(clGenPointerArithmeticCode(Compiler,
                                                   &BinaryExpr->exprBase,
                                                   opcode,
                                                   Parameters->hint,
                                                   lParams,
                                                   &rParams->rOperands[0],
                                                   intermROperand));
             Parameters->dataTypes[0].byteOffset = lParams->dataTypes[0].byteOffset;
             if(Parameters->needROperand) {
                   Parameters->rOperands[0] = intermROperand[0];
             }
             if(Parameters->needLOperand) {
               clsLOPERAND_InitializeUsingROperand(Parameters->lOperands, intermROperand);
             }
             break;
          }

          gcmONERROR(clGenImplicitConversion(Compiler,
                                             BinaryExpr->leftOperand,
                                             BinaryExpr->rightOperand,
                                             &leftParameters,
                                             &rightParameters));

          for (i = 0; i < Parameters->operandCount; i++) {
            clmGEN_CODE_GetParametersIOperand(Compiler, iOperand, Parameters, Parameters->dataTypes[i].def);
            if(!clmIsElementTypeHighPrecision(clmGEN_CODE_elementType_GET(iOperand->dataType)) &&
               !gcIsMatrixDataType(leftParameters.dataTypes[i].def) &&
               !gcIsMatrixDataType(rightParameters.dataTypes[i].def) &&
               (opcode == clvOPCODE_FMOD || opcode == clvOPCODE_DIV ||
                opcode == clvOPCODE_IDIV || opcode == clvOPCODE_MOD ||
                opcode == clvOPCODE_IMUL || opcode == clvOPCODE_FMUL ||
                opcode == clvOPCODE_FADD || opcode == clvOPCODE_FSUB)) {

                cloIR_POLYNARY_EXPR funcCall;
                clsGEN_CODE_PARAMETERS operandsParameters[2];
                operandsParameters[0] = leftParameters;
                operandsParameters[1] = rightParameters;

                gcmONERROR(_ConvExprToFuncCall(Compiler,
                                               opcode,
                                               &BinaryExpr->exprBase,
                                               &funcCall));

                gcmONERROR(clGenBuiltinFunctionCode(Compiler,
                                                    CodeGenerator,
                                                    funcCall,
                                                    2,
                                                    operandsParameters,
                                                    iOperand,
                                                    Parameters,
                                                    gcvTRUE));
            }
            else {
               gcmONERROR(clGenArithmeticExprCode(Compiler,
                                                  BinaryExpr->exprBase.base.lineNo,
                                                  BinaryExpr->exprBase.base.stringNo,
                                                  opcode                                                                                                ,
                                                  iOperand,
                                                  leftParameters.rOperands + i,
                                                  rightParameters.rOperands + i));
            }

            if(Parameters->needROperand) {
               clsROPERAND_InitializeUsingIOperand(Parameters->rOperands + i, iOperand);
            }
            if(Parameters->needLOperand) {
               clsLOPERAND_InitializeUsingIOperand(Parameters->lOperands + i, iOperand);
            }
          }
        } while(gcvFALSE);
      }
    } while (gcvFALSE);

OnError:
    clsGEN_CODE_PARAMETERS_Finalize(&leftParameters);
    clsGEN_CODE_PARAMETERS_Finalize(&rightParameters);

    return status;
}

gceSTATUS
cloIR_BINARY_EXPR_GenRelationalCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_BINARY_EXPR BinaryExpr,
    IN OUT clsGEN_CODE_PARAMETERS * Parameters
    )
{
    gceSTATUS    status;
    clsGEN_CODE_PARAMETERS    leftParameters, rightParameters;
    clsIOPERAND    intermIOperand;
    cleOPCODE    opcode;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_OBJECT(CodeGenerator, clvOBJ_CODE_GENERATOR);
    clmVERIFY_IR_OBJECT(BinaryExpr, clvIR_BINARY_EXPR);
    gcmASSERT(Parameters);
    gcmASSERT(!Parameters->needLOperand);

    /* Generate the code of the left operand */
    gcmASSERT(BinaryExpr->leftOperand);

    clsGEN_CODE_PARAMETERS_Initialize(&leftParameters,
                      gcvFALSE,
                      Parameters->needROperand);

    status = cloIR_OBJECT_Accept(Compiler,
                     &BinaryExpr->leftOperand->base,
                     &CodeGenerator->visitor,
                     &leftParameters);
    if (gcmIS_ERROR(status)) return status;

    /* Generate the code of the right operand */
    gcmASSERT(BinaryExpr->rightOperand);

    clsGEN_CODE_PARAMETERS_Initialize(&rightParameters,
                      gcvFALSE,
                      Parameters->needROperand);

    status = cloIR_OBJECT_Accept(Compiler,
                     &BinaryExpr->rightOperand->base,
                     &CodeGenerator->visitor,
                     &rightParameters);
    if (gcmIS_ERROR(status)) return status;

    if (Parameters->needROperand)
    {
        gcmASSERT(leftParameters.operandCount == 1);
        gcmASSERT(rightParameters.operandCount == 1);

        status = clGenImplicitConversion(Compiler,
                                                 BinaryExpr->leftOperand,
                                                 BinaryExpr->rightOperand,
                                                 &leftParameters,
                                                 &rightParameters);
        if(gcmIS_ERROR(status)) return status;

        switch (BinaryExpr->type)
        {
        case clvBINARY_LESS_THAN:        opcode = clvOPCODE_LESS_THAN;        break;
        case clvBINARY_LESS_THAN_EQUAL:        opcode = clvOPCODE_LESS_THAN_EQUAL;    break;
        case clvBINARY_GREATER_THAN:        opcode = clvOPCODE_GREATER_THAN;    break;
        case clvBINARY_GREATER_THAN_EQUAL:    opcode = clvOPCODE_GREATER_THAN_EQUAL;    break;

        default:
            gcmASSERT(0);
            return gcvSTATUS_INVALID_ARGUMENT;
        }

        /* Return t0 */
        status = clsGEN_CODE_PARAMETERS_AllocateOperands(Compiler,
                                 Parameters,
                                 &BinaryExpr->exprBase.decl);
        if (gcmIS_ERROR(status)) return status;

        /* less-than/less-than-equal/greater-than/greater-than-equal t0, operand */
                clmGEN_CODE_GetParametersIOperand(Compiler, &intermIOperand, Parameters, Parameters->dataTypes[0].def);

        status = clGenGenericCode2(Compiler,
                       BinaryExpr->exprBase.base.lineNo,
                       BinaryExpr->exprBase.base.stringNo,
                       opcode,
                       &intermIOperand,
                       &leftParameters.rOperands[0],
                       &rightParameters.rOperands[0]);
        if (gcmIS_ERROR(status)) return status;

        clsROPERAND_InitializeUsingIOperand(&Parameters->rOperands[0], &intermIOperand);
    }

    clsGEN_CODE_PARAMETERS_Finalize(&leftParameters);
    clsGEN_CODE_PARAMETERS_Finalize(&rightParameters);

    return gcvSTATUS_OK;
}

gceSTATUS
cloIR_BINARY_EXPR_GenEqualityCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_BINARY_EXPR BinaryExpr,
    IN OUT clsGEN_CODE_PARAMETERS * Parameters
    )
{
    gceSTATUS    status;
    clsGEN_CODE_PARAMETERS    leftParameters, rightParameters;
    clsIOPERAND    intermIOperand;
    cleOPCODE    opcode;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_OBJECT(CodeGenerator, clvOBJ_CODE_GENERATOR);
    clmVERIFY_IR_OBJECT(BinaryExpr, clvIR_BINARY_EXPR);
    gcmASSERT(Parameters);
    gcmASSERT(!Parameters->needLOperand);

    /* Generate the code of the left operand */
    gcmASSERT(BinaryExpr->leftOperand);

    clsGEN_CODE_PARAMETERS_Initialize(&leftParameters,
                      gcvFALSE,
                      Parameters->needROperand);

    status = cloIR_OBJECT_Accept(Compiler,
                     &BinaryExpr->leftOperand->base,
                     &CodeGenerator->visitor,
                     &leftParameters);
    if (gcmIS_ERROR(status)) return status;

    /* Generate the code of the right operand */
    gcmASSERT(BinaryExpr->rightOperand);

    clsGEN_CODE_PARAMETERS_Initialize(&rightParameters,
                      gcvFALSE,
                      Parameters->needROperand);

    status = cloIR_OBJECT_Accept(Compiler,
                     &BinaryExpr->rightOperand->base,
                     &CodeGenerator->visitor,
                     &rightParameters);
    if (gcmIS_ERROR(status)) return status;

    if (Parameters->needROperand)
    {
        status = clGenImplicitConversion(Compiler,
                                                 BinaryExpr->leftOperand,
                                                 BinaryExpr->rightOperand,
                                                 &leftParameters,
                                                 &rightParameters);
        if(gcmIS_ERROR(status)) return status;

        /* Return t0 */
        status = clsGEN_CODE_PARAMETERS_AllocateOperands(Compiler,
                                 Parameters,
                                 &BinaryExpr->exprBase.decl);
        if (gcmIS_ERROR(status)) return status;
                clmGEN_CODE_GetParametersIOperand(Compiler, &intermIOperand, Parameters, Parameters->dataTypes[0].def);

        /* Get the opcode */
        switch (BinaryExpr->type) {
        case clvBINARY_EQUAL:
           opcode = clvOPCODE_EQUAL;
           break;

        case clvBINARY_NOT_EQUAL:
        case clvBINARY_XOR:
           opcode = clvOPCODE_NOT_EQUAL;
           break;

        default:
            gcmASSERT(0);
            return gcvSTATUS_INVALID_ARGUMENT;
        }

        /* equal/not-equal t0, operand */
        status = clGenGenericCode2(Compiler,
                       BinaryExpr->exprBase.base.lineNo,
                       BinaryExpr->exprBase.base.stringNo,
                       opcode,
                       &intermIOperand,
                       &leftParameters.rOperands[0],
                       &rightParameters.rOperands[0]);
        if (gcmIS_ERROR(status)) return status;

        clsROPERAND_InitializeUsingIOperand(&Parameters->rOperands[0], &intermIOperand);
    }

    clsGEN_CODE_PARAMETERS_Finalize(&leftParameters);
    clsGEN_CODE_PARAMETERS_Finalize(&rightParameters);

    return gcvSTATUS_OK;
}

gceSTATUS
cloIR_BINARY_EXPR_GenAndCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_BINARY_EXPR BinaryExpr,
    IN OUT clsGEN_CODE_PARAMETERS * Parameters
    )
{
    gceSTATUS        status;
    clsGEN_CODE_PARAMETERS    rightParameters;
    clsGEN_CODE_PARAMETERS    leftParameters;
    clsIOPERAND        intermIOperand;
    clsLOPERAND        intermLOperand;
    clsROPERAND        zeroROperand[1];
    clsROPERAND        oneROperand[1];

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_OBJECT(CodeGenerator, clvOBJ_CODE_GENERATOR);
    clmVERIFY_IR_OBJECT(BinaryExpr, clvIR_BINARY_EXPR);
    gcmASSERT(BinaryExpr->leftOperand);
    gcmASSERT(BinaryExpr->rightOperand);
    gcmASSERT(Parameters);
    gcmASSERT(!Parameters->needLOperand);

    if (Parameters->needROperand) {
        gcmASSERT(BinaryExpr->exprBase.decl.dataType);

        /* Allocate the operand(s) */
        status = clsGEN_CODE_PARAMETERS_AllocateOperands(Compiler,
                                Parameters,
                                &BinaryExpr->exprBase.decl);
        if (gcmIS_ERROR(status)) return status;

        gcmASSERT(Parameters->operandCount == 1);

                clmGEN_CODE_GetParametersIOperand(Compiler, &intermIOperand, Parameters, Parameters->dataTypes[0].def);
        clsLOPERAND_InitializeUsingIOperand(&intermLOperand, &intermIOperand);
        clsROPERAND_InitializeUsingIOperand(&Parameters->rOperands[0], &intermIOperand);
    }

    _InitializeROperandConstant(zeroROperand, clmGenCodeDataType(T_INT), 0);
    _InitializeROperandConstant(oneROperand, clmGenCodeDataType(T_INT), 1);

        /* Generate the code of the left operand */
    gcmASSERT(BinaryExpr->leftOperand);

    clsGEN_CODE_PARAMETERS_Initialize(&leftParameters,
                      gcvFALSE,
                      Parameters->needROperand);

    status = cloIR_OBJECT_Accept(Compiler,
                     &BinaryExpr->leftOperand->base,
                     &CodeGenerator->visitor,
                     &leftParameters);
    if (gcmIS_ERROR(status)) return status;

    /* Generate the code of the right operand */
    gcmASSERT(BinaryExpr->rightOperand);

    clsGEN_CODE_PARAMETERS_Initialize(&rightParameters,
                      gcvFALSE,
                      Parameters->needROperand);

    status = cloIR_OBJECT_Accept(Compiler,
                     &BinaryExpr->rightOperand->base,
                     &CodeGenerator->visitor,
                     &rightParameters);
    if (gcmIS_ERROR(status)) {
      clsGEN_CODE_PARAMETERS_Finalize(&leftParameters);
          return status;
    }

    if (clmDECL_IsScalar(&BinaryExpr->exprBase.decl)) {
       /* Selection Begin */
       clmGEN_CODE_IF(Compiler,
              CodeGenerator,
              BinaryExpr->exprBase.base.lineNo,
              BinaryExpr->exprBase.base.stringNo,
              &leftParameters.rOperands[0],
              clvCONDITION_EQUAL,
              zeroROperand);

            if (Parameters->needROperand) {
           gcmONERROR(clGenAssignCode(Compiler,
                          BinaryExpr->exprBase.base.lineNo,
                          BinaryExpr->exprBase.base.stringNo,
                          &intermLOperand,
                          zeroROperand));
                }

                /* The false part, "!=0" */
        clmGEN_CODE_ELSE(Compiler,
                                CodeGenerator,
                                BinaryExpr->exprBase.base.lineNo,
                                BinaryExpr->exprBase.base.stringNo);
            /* Generate the code of the condition expression */
               clmGEN_CODE_IF(Compiler,
                      CodeGenerator,
                      BinaryExpr->exprBase.base.lineNo,
                      BinaryExpr->exprBase.base.stringNo,
                      &rightParameters.rOperands[0],
                      clvCONDITION_NOT_EQUAL,
                      zeroROperand);

                 if (Parameters->needROperand) {
                  gcmONERROR(clGenAssignCode(Compiler,
                                 BinaryExpr->exprBase.base.lineNo,
                                 BinaryExpr->exprBase.base.stringNo,
                                 &intermLOperand,
                                 oneROperand));
                }

           clmGEN_CODE_ELSE(Compiler,
                                   CodeGenerator,
                                   BinaryExpr->exprBase.base.lineNo,
                                   BinaryExpr->exprBase.base.stringNo);
                 if (Parameters->needROperand) {
                  gcmONERROR(clGenAssignCode(Compiler,
                                 BinaryExpr->exprBase.base.lineNo,
                                 BinaryExpr->exprBase.base.stringNo,
                                 &intermLOperand,
                                 zeroROperand));
                }

                     clmGEN_CODE_ENDIF(Compiler,
                                      CodeGenerator,
                                      BinaryExpr->exprBase.base.lineNo,
                                      BinaryExpr->exprBase.base.stringNo);

           clmGEN_CODE_ENDIF(Compiler,
                             CodeGenerator,
                             BinaryExpr->exprBase.base.lineNo,
                             BinaryExpr->exprBase.base.stringNo);
        }
    else { /*vector operation */
       clsIOPERAND    iOperand1, iOperand2;
       clsROPERAND    rOperand1, rOperand2;

       if (Parameters->needROperand) {
          gcmONERROR(clGenImplicitConversion(Compiler,
                                                 BinaryExpr->leftOperand,
                                                 BinaryExpr->rightOperand,
                                                 &leftParameters,
                                                 &rightParameters));

          clsIOPERAND_New(Compiler, &iOperand1, Parameters->dataTypes[0].def);

          /* left operand not-equal 0*/
          gcmONERROR(clGenGenericCode2(Compiler,
                           BinaryExpr->exprBase.base.lineNo,
                           BinaryExpr->exprBase.base.stringNo,
                           clvOPCODE_NOT_EQUAL,
                           &iOperand1,
                           &leftParameters.rOperands[0],
                           zeroROperand));

          clsIOPERAND_New(Compiler, &iOperand2, Parameters->dataTypes[0].def);
          /* right operand not-equal 0*/
          gcmONERROR(clGenGenericCode2(Compiler,
                           BinaryExpr->exprBase.base.lineNo,
                           BinaryExpr->exprBase.base.stringNo,
                           clvOPCODE_NOT_EQUAL,
                           &iOperand2,
                           &rightParameters.rOperands[0],
                           zeroROperand));

          clsROPERAND_InitializeUsingIOperand(&rOperand1, &iOperand1);
          clsROPERAND_InitializeUsingIOperand(&rOperand2, &iOperand2);
          gcmONERROR(clGenBitwiseExprCode(Compiler,
                              BinaryExpr->exprBase.base.lineNo,
                              BinaryExpr->exprBase.base.stringNo,
                              clvOPCODE_BITWISE_AND,
                              &intermIOperand,
                              &rOperand1,
                              &rOperand2));
           }
    }

OnError:
    clsGEN_CODE_PARAMETERS_Finalize(&leftParameters);
    clsGEN_CODE_PARAMETERS_Finalize(&rightParameters);
    return gcvSTATUS_OK;
}

gceSTATUS
cloIR_BINARY_EXPR_GenOrCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_BINARY_EXPR BinaryExpr,
    IN OUT clsGEN_CODE_PARAMETERS * Parameters
    )
{
    gceSTATUS        status;
    clsGEN_CODE_PARAMETERS    rightParameters;
    clsGEN_CODE_PARAMETERS    leftParameters;
    clsIOPERAND        intermIOperand;
    clsLOPERAND        intermLOperand;
    clsROPERAND        zeroROperand[1];
    clsROPERAND        oneROperand[1];

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_OBJECT(CodeGenerator, clvOBJ_CODE_GENERATOR);
    clmVERIFY_IR_OBJECT(BinaryExpr, clvIR_BINARY_EXPR);
    gcmASSERT(BinaryExpr->leftOperand);
    gcmASSERT(BinaryExpr->rightOperand);
    gcmASSERT(Parameters);
    gcmASSERT(!Parameters->needLOperand);

    if (Parameters->needROperand) {
        gcmASSERT(BinaryExpr->exprBase.decl.dataType);

        /* Allocate the operand(s) */
        status = clsGEN_CODE_PARAMETERS_AllocateOperands(Compiler,
                                Parameters,
                                &BinaryExpr->exprBase.decl);
        if (gcmIS_ERROR(status)) return status;

        gcmASSERT(Parameters->operandCount == 1);

        clmGEN_CODE_GetParametersIOperand(Compiler, &intermIOperand, Parameters, Parameters->dataTypes[0].def);
        clsLOPERAND_InitializeUsingIOperand(&intermLOperand, &intermIOperand);
        clsROPERAND_InitializeUsingIOperand(&Parameters->rOperands[0], &intermIOperand);
    }

    _InitializeROperandConstant(zeroROperand, clmGenCodeDataType(T_INT), 0);
    _InitializeROperandConstant(oneROperand, clmGenCodeDataType(T_INT), 1);

        /* Generate the code of the left operand */
    gcmASSERT(BinaryExpr->leftOperand);

    clsGEN_CODE_PARAMETERS_Initialize(&leftParameters,
                      gcvFALSE,
                      Parameters->needROperand);

    status = cloIR_OBJECT_Accept(Compiler,
                     &BinaryExpr->leftOperand->base,
                     &CodeGenerator->visitor,
                     &leftParameters);
    if (gcmIS_ERROR(status)) return status;

    /* Generate the code of the right operand */
    gcmASSERT(BinaryExpr->rightOperand);

    clsGEN_CODE_PARAMETERS_Initialize(&rightParameters,
                      gcvFALSE,
                      Parameters->needROperand);

    status = cloIR_OBJECT_Accept(Compiler,
                     &BinaryExpr->rightOperand->base,
                     &CodeGenerator->visitor,
                     &rightParameters);
    if (gcmIS_ERROR(status)) {
      clsGEN_CODE_PARAMETERS_Finalize(&leftParameters);
          return status;
    }

    if (clmDECL_IsScalar(&BinaryExpr->exprBase.decl)) {
       /* Selection Begin */
       clmGEN_CODE_IF(Compiler,
              CodeGenerator,
              BinaryExpr->exprBase.base.lineNo,
              BinaryExpr->exprBase.base.stringNo,
              &leftParameters.rOperands[0],
              clvCONDITION_NOT_EQUAL,
              zeroROperand);

            if (Parameters->needROperand) {
           gcmONERROR(clGenAssignCode(Compiler,
                          BinaryExpr->exprBase.base.lineNo,
                          BinaryExpr->exprBase.base.stringNo,
                          &intermLOperand,
                          oneROperand));
                }

                /* The false part, "==0" */
        clmGEN_CODE_ELSE(Compiler,
                                CodeGenerator,
                                BinaryExpr->exprBase.base.lineNo,
                                BinaryExpr->exprBase.base.stringNo);
            /* Generate the code of the condition expression */
               clmGEN_CODE_IF(Compiler,
                      CodeGenerator,
                      BinaryExpr->exprBase.base.lineNo,
                      BinaryExpr->exprBase.base.stringNo,
                      &rightParameters.rOperands[0],
                      clvCONDITION_NOT_EQUAL,
                      zeroROperand);

                 if (Parameters->needROperand) {
                  gcmONERROR(clGenAssignCode(Compiler,
                                 BinaryExpr->exprBase.base.lineNo,
                                 BinaryExpr->exprBase.base.stringNo,
                                 &intermLOperand,
                                 oneROperand));
                }

           clmGEN_CODE_ELSE(Compiler,
                                   CodeGenerator,
                                   BinaryExpr->exprBase.base.lineNo,
                                   BinaryExpr->exprBase.base.stringNo);
                 if (Parameters->needROperand) {
                  gcmONERROR(clGenAssignCode(Compiler,
                                 BinaryExpr->exprBase.base.lineNo,
                                 BinaryExpr->exprBase.base.stringNo,
                                 &intermLOperand,
                                 zeroROperand));
                }

                     clmGEN_CODE_ENDIF(Compiler,
                                      CodeGenerator,
                                      BinaryExpr->exprBase.base.lineNo,
                                      BinaryExpr->exprBase.base.stringNo);

           clmGEN_CODE_ENDIF(Compiler,
                             CodeGenerator,
                             BinaryExpr->exprBase.base.lineNo,
                             BinaryExpr->exprBase.base.stringNo);
        }
    else { /*vector operation */
       clsIOPERAND    iOperand1, iOperand2;
       clsROPERAND    rOperand1, rOperand2;

       if (Parameters->needROperand) {
          gcmONERROR(clGenImplicitConversion(Compiler,
                                                 BinaryExpr->leftOperand,
                                                 BinaryExpr->rightOperand,
                                                 &leftParameters,
                                                 &rightParameters));

          clsIOPERAND_New(Compiler, &iOperand1, Parameters->dataTypes[0].def);

          /* left operand not-equal 0*/
          gcmONERROR(clGenGenericCode2(Compiler,
                           BinaryExpr->exprBase.base.lineNo,
                           BinaryExpr->exprBase.base.stringNo,
                           clvOPCODE_NOT_EQUAL,
                           &iOperand1,
                           &leftParameters.rOperands[0],
                           zeroROperand));

          clsIOPERAND_New(Compiler, &iOperand2, Parameters->dataTypes[0].def);
          /* right operand not-equal 0*/
          gcmONERROR(clGenGenericCode2(Compiler,
                           BinaryExpr->exprBase.base.lineNo,
                           BinaryExpr->exprBase.base.stringNo,
                           clvOPCODE_NOT_EQUAL,
                           &iOperand2,
                           &rightParameters.rOperands[0],
                           zeroROperand));

          clsROPERAND_InitializeUsingIOperand(&rOperand1, &iOperand1);
          clsROPERAND_InitializeUsingIOperand(&rOperand2, &iOperand2);
          gcmONERROR(clGenBitwiseExprCode(Compiler,
                              BinaryExpr->exprBase.base.lineNo,
                              BinaryExpr->exprBase.base.stringNo,
                              clvOPCODE_BITWISE_OR,
                              &intermIOperand,
                              &rOperand1,
                              &rOperand2));
           }
    }

OnError:
    clsGEN_CODE_PARAMETERS_Finalize(&leftParameters);
    clsGEN_CODE_PARAMETERS_Finalize(&rightParameters);
    return gcvSTATUS_OK;
}

gceSTATUS
cloIR_BINARY_EXPR_GenBitwiseCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_BINARY_EXPR BinaryExpr,
    IN OUT clsGEN_CODE_PARAMETERS * Parameters
    )
{
    gceSTATUS    status;
    clsGEN_CODE_PARAMETERS    leftParameters, rightParameters;
    gctUINT        i;
    clsIOPERAND    iOperand;
    cleOPCODE    opcode;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_OBJECT(CodeGenerator, clvOBJ_CODE_GENERATOR);
    clmVERIFY_IR_OBJECT(BinaryExpr, clvIR_BINARY_EXPR);
    gcmASSERT(Parameters);
    gcmASSERT(!Parameters->needLOperand);

    /* Generate the code of the left operand */
    gcmASSERT(BinaryExpr->leftOperand);

    clsGEN_CODE_PARAMETERS_Initialize(&leftParameters,
                      gcvFALSE,
                      Parameters->needROperand);

    status = cloIR_OBJECT_Accept(Compiler,
                     &BinaryExpr->leftOperand->base,
                     &CodeGenerator->visitor,
                     &leftParameters);

    if (gcmIS_ERROR(status)) return status;

    /* Generate the code of the right operand */
    gcmASSERT(BinaryExpr->rightOperand);

    clsGEN_CODE_PARAMETERS_Initialize(&rightParameters,
                      gcvFALSE,
                      Parameters->needROperand);

    status = cloIR_OBJECT_Accept(Compiler,
                     &BinaryExpr->rightOperand->base,
                     &CodeGenerator->visitor,
                     &rightParameters);
    if (gcmIS_ERROR(status)) return status;

    /* Generate the shift code */
    if (Parameters->needROperand)
    {
        status = clsGEN_CODE_PARAMETERS_AllocateOperands(Compiler,
                                 Parameters,
                                 &BinaryExpr->exprBase.decl);
        if (gcmIS_ERROR(status)) return status;

        status = clGenImplicitConversion(Compiler,
                                                 BinaryExpr->leftOperand,
                                                 BinaryExpr->rightOperand,
                                                 &leftParameters,
                                                 &rightParameters);
        if(gcmIS_ERROR(status)) return status;

        for (i = 0; i < Parameters->operandCount; i++)
        {
                    clmGEN_CODE_GetParametersIOperand(Compiler, &iOperand, Parameters, Parameters->dataTypes[i].def);

            switch (BinaryExpr->type)
            {
            case clvBINARY_BITWISE_AND:    opcode = clvOPCODE_BITWISE_AND; break;
            case clvBINARY_BITWISE_OR:    opcode = clvOPCODE_BITWISE_OR; break;
            case clvBINARY_BITWISE_XOR:    opcode = clvOPCODE_BITWISE_XOR; break;

            default:
                gcmASSERT(0);
                return gcvSTATUS_INVALID_ARGUMENT;
            }

            status = clGenBitwiseExprCode(Compiler,
                              BinaryExpr->exprBase.base.lineNo,
                              BinaryExpr->exprBase.base.stringNo,
                              opcode,
                              &iOperand,
                              leftParameters.rOperands + i,
                              rightParameters.rOperands + i);
            if (gcmIS_ERROR(status)) return status;

            clsROPERAND_InitializeUsingIOperand(Parameters->rOperands + i, &iOperand);
        }
    }

    clsGEN_CODE_PARAMETERS_Finalize(&leftParameters);
    clsGEN_CODE_PARAMETERS_Finalize(&rightParameters);

    return gcvSTATUS_OK;
}

gceSTATUS
cloIR_BINARY_EXPR_GenShiftCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_BINARY_EXPR BinaryExpr,
    IN OUT clsGEN_CODE_PARAMETERS * Parameters
    )
{
    gceSTATUS    status;
    clsGEN_CODE_PARAMETERS operandsParameters[2];
    clsIOPERAND    iOperand;
    cleOPCODE    opcode;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_OBJECT(CodeGenerator, clvOBJ_CODE_GENERATOR);
    clmVERIFY_IR_OBJECT(BinaryExpr, clvIR_BINARY_EXPR);
    gcmASSERT(Parameters);
    gcmASSERT(!Parameters->needLOperand);

    /* Generate the code of the left operand */
    gcmASSERT(BinaryExpr->leftOperand);

    clsGEN_CODE_PARAMETERS_Initialize(&operandsParameters[0],
                      gcvFALSE,
                      Parameters->needROperand);

    status = cloIR_OBJECT_Accept(Compiler,
                     &BinaryExpr->leftOperand->base,
                     &CodeGenerator->visitor,
                     &operandsParameters[0]);
    if (gcmIS_ERROR(status)) return status;

    /* Generate the code of the right operand */
    gcmASSERT(BinaryExpr->rightOperand);

    clsGEN_CODE_PARAMETERS_Initialize(&operandsParameters[1],
                      gcvFALSE,
                      Parameters->needROperand);

    status = cloIR_OBJECT_Accept(Compiler,
                     &BinaryExpr->rightOperand->base,
                     &CodeGenerator->visitor,
                     &operandsParameters[1]);
    if (gcmIS_ERROR(status)) {
       clsGEN_CODE_PARAMETERS_Finalize(&operandsParameters[0]);
       return status;
    }

    /* Generate the shift code */
    if (Parameters->needROperand) {
                cloIR_POLYNARY_EXPR funcCall;

        gcmONERROR(clsGEN_CODE_PARAMETERS_AllocateOperands(Compiler,
                                   Parameters,
                                   &BinaryExpr->exprBase.decl));

        gcmASSERT(Parameters->operandCount == 1);
                clmGEN_CODE_GetParametersIOperand(Compiler, &iOperand, Parameters, Parameters->dataTypes[0].def);

        switch (BinaryExpr->type) {
        case clvBINARY_RSHIFT:
           opcode = clvOPCODE_RIGHT_SHIFT;
           break;

        case clvBINARY_LSHIFT:
           opcode = clvOPCODE_LEFT_SHIFT;
           break;

        default:
           gcmASSERT(0);
           return gcvSTATUS_INVALID_ARGUMENT;
        }

        if(clmDECL_IsVectorType(&BinaryExpr->leftOperand->decl) &&
           clmDECL_IsScalar(&BinaryExpr->rightOperand->decl)) {
            gcmONERROR(_ConvScalarToVector(Compiler,
                                           clmDATA_TYPE_vectorSize_NOCHECK_GET(BinaryExpr->leftOperand->decl.dataType),
                                           BinaryExpr->rightOperand,
                                           &operandsParameters[1]));
        }
        gcmONERROR(_ConvExprToFuncCall(Compiler,
                                       opcode,
                                       &BinaryExpr->exprBase,
                                       &funcCall));

        gcmONERROR(clGenBuiltinFunctionCode(Compiler,
                                            CodeGenerator,
                                            funcCall,
                                            2,
                                            operandsParameters,
                                            &iOperand,
                                            Parameters,
                                            gcvTRUE));

        clsROPERAND_InitializeUsingIOperand(Parameters->rOperands, &iOperand);
    }

OnError:
    clsGEN_CODE_PARAMETERS_Finalize(&operandsParameters[0]);
    clsGEN_CODE_PARAMETERS_Finalize(&operandsParameters[1]);

    return gcvSTATUS_OK;
}

gceSTATUS
cloIR_BINARY_EXPR_GenSequenceCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_BINARY_EXPR BinaryExpr,
    IN OUT clsGEN_CODE_PARAMETERS * Parameters
    )
{
    gceSTATUS    status;
    clsGEN_CODE_PARAMETERS    leftParameters, rightParameters;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_OBJECT(CodeGenerator, clvOBJ_CODE_GENERATOR);
    clmVERIFY_IR_OBJECT(BinaryExpr, clvIR_BINARY_EXPR);
    gcmASSERT(Parameters);
    gcmASSERT(!Parameters->needLOperand);

    /* Generate the code of the left operand */
    gcmASSERT(BinaryExpr->leftOperand);

    clsGEN_CODE_PARAMETERS_Initialize(
                                    &leftParameters,
                                    gcvFALSE,
                                    gcvFALSE);

    status = cloIR_OBJECT_Accept(
                                Compiler,
                                &BinaryExpr->leftOperand->base,
                                &CodeGenerator->visitor,
                                &leftParameters);

    if (gcmIS_ERROR(status)) return status;

    /* Generate the code of the right operand */
    gcmASSERT(BinaryExpr->rightOperand);

    clsGEN_CODE_PARAMETERS_Initialize(
                                    &rightParameters,
                                    gcvFALSE,
                                    Parameters->needROperand);

    status = cloIR_OBJECT_Accept(
                                Compiler,
                                &BinaryExpr->rightOperand->base,
                                &CodeGenerator->visitor,
                                &rightParameters);

    if (gcmIS_ERROR(status)) return status;

    if (Parameters->needROperand)
    {
          clsGEN_CODE_PARAMETERS_MoveOperands(Parameters, &rightParameters);
          if(Parameters->hasIOperand) {
             gceSTATUS status;

             gcmASSERT(Parameters->operandCount == 1);
             _clmAssignROperandToPreallocatedTempReg(Compiler,
                                 Parameters,
                                 &Parameters->rOperands[0],
                                 status);
             if(gcmIS_ERROR(status)) return status;
              }
    }

    clsGEN_CODE_PARAMETERS_Finalize(&leftParameters);
    clsGEN_CODE_PARAMETERS_Finalize(&rightParameters);

    return gcvSTATUS_OK;
}

static gceSTATUS
_ConvLOperandToSourceReg(
    IN cloCOMPILER Compiler,
    IN clsLOPERAND * LOperand,
    IN clsCOMPONENT_SELECTION ReversedComponentSelection,
    OUT gcsSOURCE * Source
    )
{
#if defined __GNUC__
    gcsSOURCE_REG    sourceReg = {};
#else
    gcsSOURCE_REG    sourceReg = { 0 };
#endif
    clsCOMPONENT_SELECTION    componentSelection;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    gcmASSERT(LOperand);
    gcmASSERT(Source);

    sourceReg.u.attribute    = LOperand->reg.u.attribute;
    sourceReg.regIndex    = LOperand->reg.regIndex;

    switch (LOperand->arrayIndex.mode)
    {
    case clvINDEX_NONE:
        sourceReg.indexMode = gcSL_NOT_INDEXED;
        sourceReg.indexRegIndex    = 0;
        break;

    case clvINDEX_REG:
        gcmASSERT(LOperand->matrixIndex.mode != clvINDEX_REG);

        sourceReg.indexMode = gcSL_INDEXED_X;
        sourceReg.indexRegIndex    = LOperand->arrayIndex.u.indexRegIndex;
        break;

    default:
        gcmASSERT(0);
        return gcvSTATUS_INVALID_ARGUMENT;
    }

    if (gcIsScalarDataType(LOperand->dataType)) {
        if (gcIsScalarDataType(LOperand->reg.dataType)) {
            componentSelection = clGetDefaultComponentSelection(LOperand->dataType);

            componentSelection = _SwizzleComponentSelection(&ReversedComponentSelection,
                                    &componentSelection);

            sourceReg.swizzle = _ConvComponentSelectionToSwizzle(&componentSelection);
        }
        else {
            gcmASSERT(gcIsVectorDataType(LOperand->reg.dataType)
                        || gcIsMatrixDataType(LOperand->reg.dataType));

            switch (LOperand->vectorIndex.mode) {
            case clvINDEX_CONSTANT:
                componentSelection = _clmConvVectorIndexToComponentSelection(LOperand->dataType,
                                                                                             LOperand->vectorIndex.u.constant);

                componentSelection = _SwizzleComponentSelection(&componentSelection,
                                        &LOperand->reg.componentSelection);

                sourceReg.regIndex += _clmGetTempRegIndexOffset(_clmGetComponentSectionIndex(componentSelection.selection[0]),
                                                                clmGEN_CODE_elementType_GET(LOperand->dataType)),
                componentSelection = _SwizzleComponentSelection(&ReversedComponentSelection,
                                        &componentSelection);

                sourceReg.swizzle = _ConvComponentSelectionToSwizzle(&componentSelection);
                break;

            default:
                gcmASSERT(0);
                return gcvSTATUS_INVALID_ARGUMENT;
            }

            if (gcIsMatrixDataType(LOperand->reg.dataType))
            {
                switch (LOperand->matrixIndex.mode)
                {
                case clvINDEX_CONSTANT:
                    sourceReg.regIndex += (gctREG_INDEX)(LOperand->matrixIndex.u.constant
                                * gcGetMatrixColumnRegSize(LOperand->reg.dataType));

                    break;

                case clvINDEX_REG:
                    gcmASSERT(LOperand->arrayIndex.mode != clvINDEX_REG);

                    sourceReg.indexMode = gcSL_INDEXED_X;
                    sourceReg.indexRegIndex    = LOperand->matrixIndex.u.indexRegIndex;
                    break;

                default:
                    gcmASSERT(0);
                    return gcvSTATUS_INVALID_ARGUMENT;
                }
            }
        }
    }
    else if (gcIsVectorDataType(LOperand->dataType))
    {
        gcmASSERT(gcIsVectorDataType(LOperand->reg.dataType)
                    || gcIsMatrixDataType(LOperand->reg.dataType));

        componentSelection = _SwizzleComponentSelection(&ReversedComponentSelection,
                                &LOperand->reg.componentSelection);

        sourceReg.swizzle = _ConvComponentSelectionToSwizzle(&componentSelection);

        if (gcIsMatrixDataType(LOperand->reg.dataType)) {
            switch (LOperand->matrixIndex.mode)
            {
            case clvINDEX_CONSTANT:
                sourceReg.regIndex += (gctREG_INDEX)(LOperand->matrixIndex.u.constant
                            * gcGetMatrixColumnRegSize(LOperand->reg.dataType));
                break;

            case clvINDEX_REG:
                gcmASSERT(LOperand->arrayIndex.mode != clvINDEX_REG);

                sourceReg.indexMode = gcSL_INDEXED_X;
                sourceReg.indexRegIndex    = LOperand->matrixIndex.u.indexRegIndex;
                break;

            default:
                gcmASSERT(0);
                return gcvSTATUS_INVALID_ARGUMENT;
            }
        }
    }
    else {
        gcmASSERT(0);
    }

    gcsSOURCE_InitializeReg(Source,
                _ConvQualifierToSourceType(LOperand->reg.qualifier),
                LOperand->dataType,
                sourceReg);

    return gcvSTATUS_OK;
}


#if cldVector8And16
#if cldUseSTORE1
gceSTATUS
clGenStoreCode(
IN cloCOMPILER Compiler,
IN gctUINT LineNo,
IN gctUINT StringNo,
IN clsROPERAND *ROperand,
IN clsLOPERAND *LOperand,
IN clsGEN_CODE_DATA_TYPE ResType,
IN clsROPERAND *Offset
)
{
  gceSTATUS status;
  gcsSUPER_TARGET superTarget;
  gcsSUPER_SOURCE superSource1;
  gcsSUPER_SOURCE superSource0;
  gcsSOURCE offsetSource[1];
  gcsTARGET addressTarget[1];
  gcsSOURCE addressSource[1];
  gctSIZE_T /*rSize, */j;
  clsROPERAND rOperand[1];
  clsROPERAND columnROperand[1];
  clsIOPERAND iOperand[1];
  gctUINT numColumns;
  clsROPERAND *newOffset;
  clsROPERAND *offset;
  gctINT incr;
  /*clsGEN_CODE_DATA_TYPE resType;*/

/* Verify the arguments. */
  clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
  gcmASSERT(ROperand);
  gcmASSERT(LOperand);
  gcmASSERT(Offset);

  gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                                clvDUMP_CODE_GENERATOR,
                                "<OPERATION line=\"%d\" string=\"%d\" type=\"store\">",
                                LineNo,
                                StringNo));

  gcmVERIFY_OK(clsROPERAND_Dump(Compiler, ROperand));
  gcmVERIFY_OK(clsLOPERAND_Dump(Compiler, LOperand));
  gcmVERIFY_OK(clsROPERAND_Dump(Compiler, Offset));

  _clmConvLOperandToSuperSourceReg(Compiler,
                                   LOperand,
                                   clGetDefaultComponentSelection(LOperand->dataType),
                                   &superSource0,
                                   status);
  if (gcmIS_ERROR(status)) return status;


  if(gcIsMatrixDataType(ResType)) {
     /*resType = gcGetMatrixColumnDataType(ResType);*/
     numColumns = gcGetMatrixDataTypeColumnCount(ResType);
  }
  else {
      /*resType = ResType;*/
      numColumns = 1;
  }

  if(gcIsMatrixDataType(ROperand->dataType)) {
     clsROPERAND_InitializeAsMatrixColumn(columnROperand, ROperand, 0);
  }
  else {
     *columnROperand = *ROperand;
  }

  /*rSize = gcGetDataTypeRegSize(resType);*/

  incr= 0;
  newOffset = gcvNULL;
  offset = Offset;

  do {
     if(!columnROperand->isReg) { /*constant*/
       gctREG_INDEX constantReg;
       clsLOPERAND constantOperand[1];
       clsCOMPONENT_SELECTION reversedComponentSelection;

       constantReg = clNewTempRegs(Compiler,
                                   gcGetDataTypeRegSize(ResType),
                                   ResType.elementType);
       clsLOPERAND_InitializeTempReg(constantOperand,
                                     clvQUALIFIER_NONE,
                                     ResType,
                                     constantReg);

       status = _ConvLOperandToSuperTarget(Compiler,
                                           constantOperand,
                                           &superTarget,
                                           &reversedComponentSelection);
       if(gcmIS_ERROR(status)) return status;
     }
     else {
        status = _ConvROperandToSuperTarget(Compiler,
                                            columnROperand,
                                            &superTarget);
        if(gcmIS_ERROR(status)) return status;
     }

     status = _ConvNormalROperandToSuperSource(Compiler,
                                               LineNo,
                                               StringNo,
                                               columnROperand,
                                               &superSource1);
     if(gcmIS_ERROR(status)) return status;

     _SplitSources(&superSource1, superTarget.numTargets);
     for(j=0; j < superTarget.numTargets; j++) {
         if(incr) {
             status = _UpdateAddressOffset(Compiler,
                                           LineNo,
                                           StringNo,
                                           incr,
                                           offset,
                                           newOffset ? gcvNULL: rOperand);
             if (gcmIS_ERROR(status)) return status;
             newOffset = rOperand;
             offset = rOperand;
         }

         status = _ConvNormalROperandToSource(Compiler,
                                              LineNo,
                                              StringNo,
                                              offset,
                                              offsetSource);
         if (gcmIS_ERROR(status)) return status;

         clsIOPERAND_New(Compiler, iOperand, LOperand->dataType);
         gcsTARGET_InitializeUsingIOperand(addressTarget, iOperand);
         gcsSOURCE_InitializeUsingIOperand(addressSource, iOperand);
         status = clEmitCode2(Compiler,
                              LineNo,
                              StringNo,
                              clvOPCODE_ADD,
                              addressTarget,
                              superSource0.sources,
                              offsetSource);
         if (gcmIS_ERROR(status)) return status;

         /* clear the tempRegIndex so that it is different from
            source to avoid the MOV*/
         superTarget.targets[j].tempRegIndex = 0;
         status = clEmitCode2(Compiler,
                              LineNo,
                              StringNo,
                              clvOPCODE_STORE1,
                              superTarget.targets + j,
                              addressSource,
                              superSource1.sources + j);
         if (gcmIS_ERROR(status)) return status;

#if _cldHandleULongStore
         if(!gcmOPT_oclHasLong()) {
           if(clmGEN_CODE_IsScalarDataType(superTarget.targets[j].dataType) &&
              clmIsElementTypeHighPrecision(superTarget.targets[j].dataType.elementType)) {
              gcsSOURCE zeroSource[1];
              gcsTARGET dummyTarget[1];
              gcsSOURCE_CONSTANT zeroConstant;
              clsGEN_CODE_DATA_TYPE dataType;
              clsROPERAND upperHalfOffset[1];

              dataType = clmGenCodeDataType(T_UINT);

              zeroConstant.uintValue = 0;
              gcsSOURCE_InitializeConstant(zeroSource, dataType, zeroConstant);

              status = _UpdateAddressOffset(Compiler,
                                            LineNo,
                                            StringNo,
                                            4,
                                            offset,
                                            upperHalfOffset);
              if (gcmIS_ERROR(status)) return status;

              status = _ConvNormalROperandToSource(Compiler,
                                                   LineNo,
                                                   StringNo,
                                                   upperHalfOffset,
                                                   offsetSource);
              if (gcmIS_ERROR(status)) return status;

              clsIOPERAND_New(Compiler, iOperand, LOperand->dataType);
              gcsTARGET_InitializeUsingIOperand(addressTarget, iOperand);
              gcsSOURCE_InitializeUsingIOperand(addressSource, iOperand);
              status = clEmitCode2(Compiler,
                                   LineNo,
                                   StringNo,
                                   clvOPCODE_ADD,
                                   addressTarget,
                                   superSource0.sources,
                                   offsetSource);
              if (gcmIS_ERROR(status)) return status;

              /* clear the tempRegIndex so that it is different from
                 source to avoid the MOV*/
              dummyTarget[0] = superTarget.targets[j];
              dummyTarget->tempRegIndex = 0;
              dummyTarget->dataType = dataType;
              status = clEmitCode2(Compiler,
                                   LineNo,
                                   StringNo,
                                   clvOPCODE_STORE1,
                                   dummyTarget,
                                   addressSource,
                                   zeroSource);
              if (gcmIS_ERROR(status)) return status;
           }
         }
#endif
         incr = (gctINT)_DataTypeByteSize(superTarget.targets[j].dataType);
     }
     if(gcIsMatrixDataType(ROperand->dataType)) {
           columnROperand->matrixIndex.u.constant += 1;
     }
  } while(--numColumns);

  gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                                clvDUMP_CODE_GENERATOR,
                                "</OPERATION>"));

  return gcvSTATUS_OK;
}

#else
gceSTATUS
clGenStoreCode(
IN cloCOMPILER Compiler,
IN gctUINT LineNo,
IN gctUINT StringNo,
IN clsROPERAND *ROperand,
IN clsLOPERAND *LOperand,
IN clsGEN_CODE_DATA_TYPE ResType,
IN clsROPERAND *Offset
)
{
  gceSTATUS status;
  gcsSUPER_TARGET superTarget;
  gcsSUPER_SOURCE superSource0;
  gcsSOURCE source1[1];
  gctSIZE_T /*rSize, */j;
  clsROPERAND rOperand[1];
  clsROPERAND columnROperand[1];
  gctUINT numColumns;
  clsROPERAND *newOffset;
  clsROPERAND *offset;
  gctINT incr;
  /*clsGEN_CODE_DATA_TYPE resType;*/

/* Verify the arguments. */
  clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
  gcmASSERT(ROperand);
  gcmASSERT(LOperand);
  gcmASSERT(Offset);

  gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                                clvDUMP_CODE_GENERATOR,
                                "<OPERATION line=\"%d\" string=\"%d\" type=\"store\">",
                                LineNo,
                                StringNo));

  gcmVERIFY_OK(clsROPERAND_Dump(Compiler, ROperand));
  gcmVERIFY_OK(clsLOPERAND_Dump(Compiler, LOperand));
  gcmVERIFY_OK(clsROPERAND_Dump(Compiler, Offset));

  _clmConvLOperandToSuperSourceReg(Compiler,
                                   LOperand,
                                   clGetDefaultComponentSelection(LOperand->dataType),
                                   &superSource0,
                                   status);
  if (gcmIS_ERROR(status)) return status;


  if(gcIsMatrixDataType(ResType)) {
     /*resType = gcGetMatrixColumnDataType(ResType);*/
     numColumns = gcGetMatrixDataTypeColumnCount(ResType);
  }
  else {
      /*resType = ResType;*/
      numColumns = 1;
  }

  if(gcIsMatrixDataType(ROperand->dataType)) {
     clsROPERAND_InitializeAsMatrixColumn(columnROperand, ROperand, 0);
  }
  else {
     *columnROperand = *ROperand;
  }

  /*rSize = gcGetDataTypeRegSize(resType);*/

  incr= 0;
  newOffset = gcvNULL;
  offset = Offset;

  do {
     if(!columnROperand->isReg) { /*constant*/
       clsLOPERAND constantOperand[1];
       clsCOMPONENT_SELECTION reversedComponentSelection;

        status = _GenConstantAssignCode(Compiler,
                                        LineNo,
                                        StringNo,
                                        columnROperand,
                                        resType,
                                        constantOperand);
        if(gcmIS_ERROR(status)) return status;

        status = _ConvLOperandToSuperTarget(Compiler,
                                                constantOperand,
                                                &superTarget,
                                                &reversedComponentSelection);
        if(gcmIS_ERROR(status)) return status;
     }
     else {
        status = _ConvROperandToSuperTarget(Compiler,
                                                columnROperand,
                                                &superTarget);
        if(gcmIS_ERROR(status)) return status;
     }

     for(j=0; j < superTarget.numTargets; j++) {
           if(incr) {
              status = _UpdateAddressOffset(Compiler,
                                            LineNo,
                                            StringNo,
                                            incr,
                                            offset,
                                            newOffset ? gcvNULL: rOperand);
              if (gcmIS_ERROR(status)) return status;
              newOffset = rOperand;
          offset = rOperand;
           }

           status = _ConvNormalROperandToSource(Compiler,
                                                LineNo,
                                                StringNo,
                                                offset,
                                                source1);
           if (gcmIS_ERROR(status)) return status;

           status = clEmitCode2(Compiler,
                                LineNo,
                                StringNo,
                                clvOPCODE_STORE,
                                superTarget.targets + j,
                                superSource0.sources,
                                source1);
           if (gcmIS_ERROR(status)) return status;
           incr = (gctINT)_DataTypeByteSize(superTarget.targets[j].dataType);
     }
     if(gcIsMatrixDataType(ROperand->dataType)) {
           columnROperand->matrixIndex.u.constant += 1;
     }
  } while(--numColumns);

  gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                                clvDUMP_CODE_GENERATOR,
                                "</OPERATION>"));

  return gcvSTATUS_OK;
}
#endif

#else
static gceSTATUS
_ConvROperandToTarget(
    IN cloCOMPILER Compiler,
    IN clsROPERAND *ROperand,
    OUT gcsTARGET *Target
    )
{
    clsLOPERAND lOperand;
    clsCOMPONENT_SELECTION    reversedComponentSelection;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    gcmASSERT(ROperand);
    gcmASSERT(!gcIsMatrixDataType(ROperand->dataType));
    gcmASSERT(Target);

    clsLOPERAND_InitializeUsingROperand(&lOperand, ROperand);

    return _ConvLOperandToTarget(Compiler,
                     &lOperand,
                     Target,
                     &reversedComponentSelection);
}

gceSTATUS
clGenStoreCode(
IN cloCOMPILER Compiler,
IN gctUINT LineNo,
IN gctUINT StringNo,
IN clsROPERAND *ROperand,
IN clsLOPERAND *LOperand,
IN clsGEN_CODE_DATA_TYPE ResType,
IN clsROPERAND *Offset
)
{
  gceSTATUS status;
  gcsTARGET target;
  gcsSOURCE source0;
  gcsSOURCE source1;

/* Verify the arguments. */
  clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
  gcmASSERT(ROperand);
  gcmASSERT(LOperand);
  gcmASSERT(Offset);

  gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                                clvDUMP_CODE_GENERATOR,
                                "<OPERATION line=\"%d\" string=\"%d\" type=\"store\">",
                                LineNo,
                                StringNo));

  gcmVERIFY_OK(clsROPERAND_Dump(Compiler, ROperand));
  gcmVERIFY_OK(clsLOPERAND_Dump(Compiler, LOperand));
  gcmVERIFY_OK(clsROPERAND_Dump(Compiler, Offset));

  if(!ROperand->isReg) { /*constant*/
    clsLOPERAND constantOperand[1];
    clsCOMPONENT_SELECTION reversedComponentSelection;

     status = _GenConstantAssignCode(Compiler,
                                     LineNo,
                                     StringNo,
                                     ROperand,
                                     ResType,
                                     constantOperand);
     _ConvLOperandToTarget(Compiler,
                           constantOperand,
                           &target,
                           &reversedComponentSelection);
  }
  else {
     gcmVERIFY_OK(_ConvROperandToTarget(Compiler,
                                        ROperand,
                                        &target));
  }

  status = _ConvLOperandToSourceReg(Compiler,
                                    LOperand,
                                    clGetDefaultComponentSelection(LOperand->dataType),
                                    &source0);
  if (gcmIS_ERROR(status)) return status;

  status = _ConvNormalROperandToSource(Compiler,
                                       LineNo,
                                       StringNo,
                                       Offset,
                                       &source1);
  if (gcmIS_ERROR(status)) return status;

  status = clEmitCode2(Compiler,
                       LineNo,
                       StringNo,
                       clvOPCODE_STORE,
                       &target,
                       &source0,
                       &source1);
  if (gcmIS_ERROR(status)) return status;

  gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                                clvDUMP_CODE_GENERATOR,
                                "</OPERATION>"));

  return gcvSTATUS_OK;
}
#endif

gceSTATUS
clGenAtomicCode(
IN cloCOMPILER Compiler,
IN gctUINT LineNo,
IN gctUINT StringNo,
IN cleOPCODE Opcode,
IN clsIOPERAND *IOperand,
IN clsROPERAND *ROperand,
IN clsROPERAND *CmpOperand,
IN clsROPERAND *ValOperand
)
{
    gceSTATUS status;
    clsIOPERAND iOperand[1];
    clsIOPERAND iOperandX[1];
    clsIOPERAND iOperandY[1];
    clsROPERAND rOperand[1];

    gcmASSERT(ValOperand);

    switch(Opcode) {
    case clvOPCODE_ATOMADD:
    case clvOPCODE_ATOMSUB:
    case clvOPCODE_ATOMMIN:
    case clvOPCODE_ATOMMAX:
    case clvOPCODE_ATOMOR:
    case clvOPCODE_ATOMAND:
    case clvOPCODE_ATOMXOR:
        return clGenGenericCode2(Compiler,
                                 LineNo,
                                 StringNo,
                                 Opcode,
                                 IOperand,
                                 ROperand,
                                 ValOperand);

    case clvOPCODE_ATOMXCHG:
        if (IOperand->dataType.elementType == clvTYPE_FLOAT &&
            ValOperand->dataType.elementType != clvTYPE_FLOAT)
        {
            clsIOPERAND_New(Compiler, iOperand, clmGenCodeDataType(T_FLOAT));
            clsROPERAND_InitializeUsingIOperand(rOperand, iOperand);

            if (ValOperand->dataType.elementType == clvTYPE_INT)
            {
                gcmONERROR(clGenGenericCode1(Compiler,
                                             LineNo,
                                             StringNo,
                                             clvOPCODE_INT_TO_FLOAT,
                                             iOperand,
                                             ValOperand));
            }
            else
            {
                gcmONERROR(clGenGenericCode1(Compiler,
                                             LineNo,
                                             StringNo,
                                             clvOPCODE_UINT_TO_FLOAT,
                                             iOperand,
                                             ValOperand));
            }

            return clGenGenericCode2(Compiler,
                                     LineNo,
                                     StringNo,
                                     Opcode,
                                     IOperand,
                                     ROperand,
                                     rOperand);
        }
        else
        {
            return clGenGenericCode2(Compiler,
                                     LineNo,
                                     StringNo,
                                     Opcode,
                                     IOperand,
                                     ROperand,
                                     ValOperand);
        }

    case clvOPCODE_ATOMCMPXCHG:
        gcmASSERT(CmpOperand);
        if (IOperand->dataType.elementType == clvTYPE_INT)
        {
            clsIOPERAND_New(Compiler, iOperand, clmGenCodeDataType(T_INT2));
        }
        else
        {
            clsIOPERAND_New(Compiler, iOperand, clmGenCodeDataType(T_UINT2));
        }
        clmIOPERAND_vectorComponent_GET(iOperandX, iOperand, clvCOMPONENT_X);
        clmIOPERAND_vectorComponent_GET(iOperandY, iOperand, clvCOMPONENT_Y);
        clsROPERAND_InitializeUsingIOperand(rOperand, iOperand);

        gcmONERROR(clGenGenericCode1(Compiler,
                                     LineNo,
                                     StringNo,
                                     clvOPCODE_ASSIGN,
                                     iOperandX,
                                     ValOperand));

        gcmONERROR(clGenGenericCode1(Compiler,
                                     LineNo,
                                     StringNo,
                                     clvOPCODE_ASSIGN,
                                     iOperandY,
                                     CmpOperand));

        return clGenGenericCode2(Compiler,
                                 LineNo,
                                 StringNo,
                                 Opcode,
                                 IOperand,
                                 ROperand,
                                 rOperand);

    default:
        return gcvSTATUS_INVALID_DATA;
    }

OnError:
    return status;
}

static gceSTATUS
_GenSelectiveStoreCode(
IN cloCOMPILER Compiler,
IN gctUINT LineNo,
IN gctUINT StringNo,
IN clsROPERAND *ROperand,
IN clsLOPERAND *LOperand,
IN clsGEN_CODE_DATA_TYPE ResType,
IN clsROPERAND *Offset,
IN clsGEN_CODE_DATA_TYPE StorageType,
IN clsCOMPONENT_SELECTION *ComponentSelection
)
{
  gceSTATUS status;
  gcsSUPER_TARGET superTarget;
  gcsSUPER_SOURCE superSource0;
  gcsSOURCE source1[1];
  gctSIZE_T numSections;
  gctSIZE_T componentCount;
  gctSIZE_T storageSize, elementSize;
  clsLOPERAND *addressOperand;
  clsLOPERAND addressBuffer[1];
  clsROPERAND columnROperand[1];
  gctUINT numColumns;
  clsROPERAND *offset;
  clsROPERAND offsetBuffer[1];
  clsGEN_CODE_DATA_TYPE resType;
  gctINT incr, byteOffset;
  gcsTARGET target[1];
  gcsTARGET *currentTarget;
  gctSIZE_T numTargets, targetComponentCount;
  gctUINT8 selectedComponent, consecutive;
  clsCOMPONENT_SELECTION componentSelectionBuf[1];
  clsCOMPONENT_SELECTION *componentSelection;

/* Verify the arguments. */
  clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
  gcmASSERT(ROperand);
  gcmASSERT(LOperand);
  gcmASSERT(Offset);

  gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                                clvDUMP_CODE_GENERATOR,
                                "<OPERATION line=\"%d\" string=\"%d\" type=\"store\">",
                                LineNo,
                                StringNo));

  gcmVERIFY_OK(clsROPERAND_Dump(Compiler, ROperand));
  gcmVERIFY_OK(clsLOPERAND_Dump(Compiler, LOperand));
  gcmVERIFY_OK(clsROPERAND_Dump(Compiler, Offset));

  if(gcIsMatrixDataType(ResType)) {
     resType = gcGetMatrixColumnDataType(ResType);
     numColumns = gcGetMatrixDataTypeColumnCount(ResType);
  }
  else {
      resType = ResType;
      numColumns = 1;
  }

  if(gcIsMatrixDataType(ROperand->dataType)) {
     clsROPERAND_InitializeAsMatrixColumn(columnROperand, ROperand, 0);
  }
  else {
     *columnROperand = *ROperand;
  }

  numSections = gcGetDataTypeRegSize(StorageType);

  if(numSections > 1 || numColumns > 1) {
     status = _AddLOperandOffset(Compiler,
                                 LineNo,
                                 StringNo,
                                 LOperand,
                                 Offset,
                                 addressBuffer);
     if (gcmIS_ERROR(status)) return status;
     addressOperand = addressBuffer;
     offset = offsetBuffer;
  }
  else {
     addressOperand = LOperand;
     offset = Offset;
  }
  _clmConvLOperandToSuperSourceReg(Compiler,
                                   addressOperand,
                                   clGetDefaultComponentSelection(LOperand->dataType),
                                   &superSource0,
                                   status);
  if (gcmIS_ERROR(status)) return status;

  storageSize = (gctINT)_DataTypeByteSize(StorageType);
  elementSize = _TargetElementTypeByteSize(StorageType.elementType);

  incr= 0;
  do {
     if(!columnROperand->isReg) { /*constant*/
       clsLOPERAND constantOperand[1];
       clsCOMPONENT_SELECTION reversedComponentSelection;

        status = _GenConstantAssignCode(Compiler,
                                        LineNo,
                                        StringNo,
                                        columnROperand,
                                        resType,
                                        constantOperand);
        if(gcmIS_ERROR(status)) return status;

        gcmVERIFY_OK(_ConvLOperandToSuperTarget(Compiler,
                                                constantOperand,
                                                &superTarget,
                                                &reversedComponentSelection));
     }
     else {
        gcmVERIFY_OK(_ConvROperandToSuperTarget(Compiler,
                                                columnROperand,
                                                &superTarget));
     }

     componentSelection = _CheckHighPrecisionComponentSelection(StorageType,
                                                                ComponentSelection,
                                                                componentSelectionBuf);

     componentCount = 0;
     selectedComponent = 0;
     numTargets = 0;

     while(numTargets < superTarget.numTargets) {
        currentTarget = superTarget.targets + numTargets;
        targetComponentCount = gcGetDataTypeComponentCount(currentTarget->dataType);

        /* get consecutive # of components */
        consecutive = 0;
        for(; componentCount < componentSelection->components; componentCount++) {
           if(consecutive) {
              if(componentSelection->selection[componentCount] != (selectedComponent + consecutive)) {
                 break;
              }
           }
           else {
              selectedComponent = componentSelection->selection[componentCount];
           }
           consecutive++;
           if(consecutive == targetComponentCount) {
              componentCount++;
              break;
           }
        }
        if(targetComponentCount > consecutive) {
           _ExtractTarget(consecutive,
                          currentTarget,
                          target);
           currentTarget = target;
        }
        else {
           numTargets++;
        }
        gcmASSERT(gcGetDataTypeComponentCount(currentTarget->dataType) == consecutive);

        byteOffset = incr + selectedComponent * elementSize;
        _InitializeROperandConstant(offsetBuffer,
                                    clmGenCodeDataType(T_UINT),
                                    byteOffset);

        if(byteOffset && offset == Offset) {
           status = _AddROperandOffset(Compiler,
                                       LineNo,
                                       StringNo,
                                       offset,
                                       offsetBuffer,
                                       offset);
        }
        status = _ConvNormalROperandToSource(Compiler,
                                             LineNo,
                                             StringNo,
                                             offset,
                                             source1);
        if (gcmIS_ERROR(status)) return status;

        status = clEmitCode2(Compiler,
                             LineNo,
                             StringNo,
                             clvOPCODE_STORE,
                             currentTarget,
                             superSource0.sources,
                             source1);
         if (gcmIS_ERROR(status)) return status;
         offset = offsetBuffer;
     }

     if(gcIsMatrixDataType(ROperand->dataType)) {
         columnROperand->matrixIndex.u.constant += 1;
         incr += storageSize;
     }
  } while(--numColumns);

  gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                                clvDUMP_CODE_GENERATOR,
                                "</OPERATION>"));

  return gcvSTATUS_OK;
}

static gceSTATUS
_GenAssignBytes(
IN cloCOMPILER Compiler,
IN gctINT LineNo,
IN gctINT StringNo,
IN clsLOPERAND *LOperand,
IN gctINT LOffset,
IN clsROPERAND *ROperand,
IN gctINT ROffset,
IN gctSIZE_T NumBytes
)
{
   gceSTATUS status;
   gctUINT bytesRemaining;
   gctUINT wordsRemaining;
   gctUINT numWords;
   gctUINT numQuads;
   clsIOPERAND iOperand[1];
   gcsSOURCE leftAddress[1], rightAddress[1];
   gcsSOURCE offset[1];
   gcsTARGET target[1];
   gcsSOURCE_CONSTANT lOffsetVal, rOffsetVal;
   clsGEN_CODE_DATA_TYPE offsetType;
   clsGEN_CODE_DATA_TYPE argType;

   bytesRemaining = NumBytes & 0x3;
   numWords = NumBytes >> 2;
   wordsRemaining = numWords & 0x3;
   numQuads = numWords >> 2;

   lOffsetVal.intValue = LOffset;
   rOffsetVal.intValue = ROffset;
   offsetType = clmGenCodeDataType(T_INT);
   status = _ConvNormalROperandToSource(Compiler,
                                        LineNo,
                                        StringNo,
                                        ROperand,
                                        rightAddress);
   if (gcmIS_ERROR(status)) return status;

   status = _ConvLOperandToSourceReg(Compiler,
                                     LOperand,
                                     clGetDefaultComponentSelection(clmGenCodeDataType(T_UINT)),
                                     leftAddress);
   if (gcmIS_ERROR(status)) return status;

   if(numQuads) {
      gctUINT i;

      argType = clmGenCodeDataType(T_INT4);
      clsIOPERAND_New(Compiler, iOperand, argType);

      status = _ConvIOperandToTarget(Compiler,
                                     iOperand,
                                     target);
      if (gcmIS_ERROR(status)) return status;

      for(i = 0; i < numQuads; i++,
                               lOffsetVal.intValue += 16,
                               rOffsetVal.intValue += 16) {
        gcsSOURCE_InitializeConstant(offset, offsetType, rOffsetVal);
        status = clEmitCode2(Compiler,
                             LineNo,
                             StringNo,
                             clvOPCODE_LOAD,
                             target,
                             rightAddress,
                             offset);
        if (gcmIS_ERROR(status)) return status;

        gcsSOURCE_InitializeConstant(offset, offsetType, lOffsetVal);
        status = clEmitCode2(Compiler,
                             LineNo,
                             StringNo,
                             clvOPCODE_STORE,
                             target,
                             leftAddress,
                             offset);
        if (gcmIS_ERROR(status)) return status;
      }
   }

   if (wordsRemaining) {
      gctUINT dataOffset = 0;

      argType = clmGenCodeDataType(T_INT);
      switch(wordsRemaining) {
      case 1:
         argType = clmGenCodeDataType(T_INT);
         dataOffset = 4;
         break;

      case 2:
         argType = clmGenCodeDataType(T_INT2);
         dataOffset = 8;
         break;

      case 3:
         argType = clmGenCodeDataType(T_INT3);
         dataOffset = 12;
         break;

      default:
         gcmASSERT(0);
         break;
      }
      clsIOPERAND_New(Compiler, iOperand, argType);
      gcsSOURCE_InitializeConstant(offset, offsetType, rOffsetVal);
      status = _ConvIOperandToTarget(Compiler,
                                     iOperand,
                                     target);
      if (gcmIS_ERROR(status)) return status;
      status = clEmitCode2(Compiler,
                           LineNo,
                           StringNo,
                           clvOPCODE_LOAD,
                           target,
                           rightAddress,
                           offset);
      if (gcmIS_ERROR(status)) return status;

      gcsSOURCE_InitializeConstant(offset, offsetType, lOffsetVal);
      status = clEmitCode2(Compiler,
                           LineNo,
                           StringNo,
                           clvOPCODE_STORE,
                           target,
                           leftAddress,
                           offset);
      if (gcmIS_ERROR(status)) return status;
      lOffsetVal.intValue += dataOffset;
      rOffsetVal.intValue += dataOffset;
   }

   if (bytesRemaining) {
      argType = clmGenCodeDataType(T_CHAR);
      switch(bytesRemaining) {
      case 1:
         argType = clmGenCodeDataType(T_CHAR);
         break;

      case 2:
         argType = clmGenCodeDataType(T_CHAR2);
         break;

      case 3:
         argType = clmGenCodeDataType(T_CHAR3);
         break;

      default:
         break;
      }
      clsIOPERAND_New(Compiler, iOperand, argType);
      gcsSOURCE_InitializeConstant(offset, offsetType, rOffsetVal);
      status = _ConvIOperandToTarget(Compiler,
                                     iOperand,
                                     target);
      if (gcmIS_ERROR(status)) return status;
      status = clEmitCode2(Compiler,
                           LineNo,
                           StringNo,
                           clvOPCODE_LOAD,
                           target,
                           rightAddress,
                           offset);
      if (gcmIS_ERROR(status)) return status;

      gcsSOURCE_InitializeConstant(offset, offsetType, lOffsetVal);
      status = clEmitCode2(Compiler,
                           LineNo,
                           StringNo,
                           clvOPCODE_STORE,
                           target,
                           leftAddress,
                           offset);
      if (gcmIS_ERROR(status)) return status;
   }

   return gcvSTATUS_OK;
}

static gceSTATUS
_MakeIOperandFromScalarLOperand(
IN clsLOPERAND *LOperand,
IN OUT clsIOPERAND *IOperand
)
{
   gcmASSERT(clmGEN_CODE_IsScalarDataType(LOperand->dataType));

   IOperand->dataType = LOperand->dataType;
   IOperand->tempRegIndex = LOperand->reg.regIndex;
   IOperand->regDataType = LOperand->reg.dataType;
   IOperand->componentSelection = LOperand->reg.componentSelection;

   return gcvSTATUS_OK;
}

static gctBOOL
_IsExprDerefComponentSelection(
cloCOMPILER Compiler,
cloIR_EXPR Expr
)
{
    cloIR_UNARY_EXPR unaryExpr;

    if(cloIR_OBJECT_GetType(&Expr->base) == clvIR_UNARY_EXPR) {
       unaryExpr = (cloIR_UNARY_EXPR) &Expr->base;
       if(unaryExpr->type == clvUNARY_COMPONENT_SELECTION) {
          cloIR_BINARY_EXPR binaryExpr;
          clsNAME *variable;

          switch(cloIR_OBJECT_GetType(&unaryExpr->operand->base)) {
          case clvIR_UNARY_EXPR:
             unaryExpr = (cloIR_UNARY_EXPR) &unaryExpr->operand->base;
             if(unaryExpr->type == clvUNARY_INDIRECTION) return gcvTRUE;
             break;

          case clvIR_BINARY_EXPR:
             binaryExpr = (cloIR_BINARY_EXPR) &unaryExpr->operand->base;
             if(binaryExpr->type == clvBINARY_SUBSCRIPT) {
                 if(clmDECL_IsPointerType(&binaryExpr->leftOperand->decl)) return gcvTRUE;
                 variable = clParseFindLeafName(Compiler,
                                                binaryExpr->leftOperand);
                 if(variable && clmGEN_CODE_checkVariableForMemory(variable)) {
                    return gcvTRUE;
                 }
             }
             break;

          default:
             variable = clParseFindLeafName(Compiler,
                                            unaryExpr->operand);
             if(variable && clmGEN_CODE_checkVariableForMemory(variable)) {
                return gcvTRUE;
             }
             break;
          }
       }
    }
    return gcvFALSE;
}

gceSTATUS
cloIR_BINARY_EXPR_GenAssignCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_BINARY_EXPR BinaryExpr,
    IN OUT clsGEN_CODE_PARAMETERS * Parameters
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_OBJECT(CodeGenerator, clvOBJ_CODE_GENERATOR);
    clmVERIFY_IR_OBJECT(BinaryExpr, clvIR_BINARY_EXPR);
    gcmASSERT(Parameters);
    gcmASSERT(!Parameters->needLOperand);

    /* Generate the code of the left operand */
    gcmASSERT(BinaryExpr->leftOperand);

    if(clmDECL_IsPackedType(&BinaryExpr->exprBase.decl) &&
       _IsExprDeref(Compiler, BinaryExpr->leftOperand)) {
        cloIR_POLYNARY_EXPR funcCall;
        gctUINT operandCount;
        clsGEN_CODE_PARAMETERS *operandsParameters;

        status =_ConvStorePackedDataToFuncCall(Compiler,
                                               &BinaryExpr->exprBase,
                                               &funcCall);
        if (gcmIS_ERROR(status)) return status;

        /* Generate the code of the operands */
        status = cloIR_POLYNARY_EXPR_GenOperandsCodeForFuncCall(Compiler,
                                                                CodeGenerator,
                                                                funcCall,
                                                                &operandCount,
                                                                &operandsParameters);
        if (gcmIS_ERROR(status)) return status;

        Parameters->operandCount = 0;
        status = clGenFuncCallCode(Compiler,
                                   CodeGenerator,
                                   funcCall,
                                   operandsParameters,
                                   Parameters);
        if (gcmIS_ERROR(status)) goto errorHandling;

        if (Parameters->needROperand) {
            clsGEN_CODE_PARAMETERS_MoveOperands(Parameters, &operandsParameters[0]);
            if(Parameters->hasIOperand) {
               gcmASSERT(Parameters->operandCount == 1);
               _clmAssignROperandToPreallocatedTempReg(Compiler,
                                                       Parameters,
                                                       &Parameters->rOperands[0],
                                                       status);
               if (gcmIS_ERROR(status)) goto errorHandling;
            }
        }

errorHandling:
        gcmVERIFY_OK(cloIR_POLYNARY_EXPR_FinalizeOperandsParameters(Compiler,
                                                                    operandCount,
                                                                    operandsParameters));
        return status;
    }
    else {
        clsGEN_CODE_PARAMETERS    leftParameters, rightParameters;
        gctUINT    i;

        clsGEN_CODE_PARAMETERS_Initialize(&leftParameters, gcvTRUE, Parameters->needROperand);
        clsGEN_CODE_PARAMETERS_Initialize(&rightParameters, gcvFALSE, gcvTRUE);

        leftParameters.hint = clvGEN_LEFT_ASSIGN_CODE;

        gcmONERROR(cloIR_OBJECT_Accept(Compiler,
                                       &BinaryExpr->leftOperand->base,
                                       &CodeGenerator->visitor,
                                       &leftParameters));
#ifdef _cldIncludeComponentOffset
        if(!(leftParameters.hint & clvGEN_DEREF_CODE) &&
           !(leftParameters.hint & clvGEN_DEREF_STRUCT_CODE) &&
           (leftParameters.operandCount == 1 &&
            clmGEN_CODE_IsScalarDataType(leftParameters.dataTypes[0].def) &&
                gcIsVectorDataType(leftParameters.lOperands[0].reg.dataType) &&
                (clmDATA_TYPE_elementType_GET(BinaryExpr->leftOperand->decl.dataType) ==
                 clmDATA_TYPE_elementType_GET(BinaryExpr->rightOperand->decl.dataType)) &&
                 _IsExprDerefComponentSelection(Compiler, BinaryExpr->rightOperand))) {
                clsIOPERAND iOperand[1];
                gctUINT8 componentIx;

                /* Generate the code of the right operand */
                gcmASSERT(BinaryExpr->rightOperand);

                status = _MakeIOperandFromScalarLOperand(leftParameters.lOperands,
                                                         iOperand);
                if (gcmIS_ERROR(status)) return status;

                componentIx = 0;
                if (gcIsVectorDataType(leftParameters.lOperands[0].reg.dataType)) {
                   gcmASSERT(leftParameters.lOperands[0].vectorIndex.mode == clvINDEX_CONSTANT);
                   componentIx = clmVectorIndexToTargetComponent(leftParameters.lOperands[0].dataType,
                                                                 leftParameters.lOperands[0].vectorIndex.u.constant);
                }

                clmGEN_CODE_SetParametersIOperand(Compiler, &rightParameters, 0, iOperand, componentIx);
                rightParameters.hint |= clvGEN_SELECTIVE_LOAD_TO;
                gcmONERROR(cloIR_OBJECT_Accept(Compiler,
                                               &BinaryExpr->rightOperand->base,
                                               &CodeGenerator->visitor,
                                               &rightParameters));
       }
       else
#endif
       {

            /* Generate the code of the right operand */
            gcmASSERT(BinaryExpr->rightOperand);
            gcmONERROR(cloIR_OBJECT_Accept(Compiler,
                           &BinaryExpr->rightOperand->base,
                           &CodeGenerator->visitor,
                           &rightParameters));

            /* Generate the assign code */
            if(leftParameters.hint & clvGEN_DEREF_CODE) {
               if(leftParameters.hint & clvGEN_COMPONENT_SELECT_CODE) {
                  clsROPERAND scaledIndex[1];
                  gctUINT elementDataTypeSize;
                  cloIR_UNARY_EXPR unaryExpr;
                  clsGEN_CODE_DATA_TYPE storageDataType;

                  gcmASSERT(cloIR_OBJECT_GetType(&BinaryExpr->leftOperand->base) == clvIR_UNARY_EXPR);
                  unaryExpr = (cloIR_UNARY_EXPR)(&BinaryExpr->leftOperand->base);
                  gcmASSERT(unaryExpr->type == clvUNARY_COMPONENT_SELECTION);

                  elementDataTypeSize = clsDECL_GetByteSize(&unaryExpr->operand->decl);
                  gcmONERROR(clGenScaledIndexOperandWithOffset(Compiler,
                                                               BinaryExpr->exprBase.base.lineNo,
                                                               BinaryExpr->exprBase.base.stringNo,
                                                               leftParameters.elementIndex,
                                                               elementDataTypeSize,
                                                               leftParameters.dataTypes[0].byteOffset,
                                                               scaledIndex));

                  gcmONERROR(_GenImplicitConvParametersToType(Compiler,
                                                              BinaryExpr->rightOperand,
                                                              BinaryExpr->leftOperand,
                                                              &rightParameters,
                                                              &leftParameters));

                  clmGEN_CODE_ConvDirectElementDataType(unaryExpr->operand->decl.dataType, storageDataType);
                  for (i = 0; i < leftParameters.operandCount; i++) {
                     gcmONERROR(_GenSelectiveStoreCode(Compiler,
                                                       BinaryExpr->exprBase.base.lineNo,
                                                       BinaryExpr->exprBase.base.stringNo,
                                                       rightParameters.rOperands + i,
                                                       leftParameters.lOperands + i,
                                                       leftParameters.dataTypes[i].def,
                                                       scaledIndex,
                                                       storageDataType,
                                                       &unaryExpr->u.componentSelection));
                  }
               }
               else { /* deref only, no component select */
                  clsROPERAND scaledIndex[1];
                  gctUINT elementDataTypeSize;

                  elementDataTypeSize = clsDECL_GetByteSize(&BinaryExpr->leftOperand->decl);
                  gcmONERROR(clGenScaledIndexOperandWithOffset(Compiler,
                                                               BinaryExpr->exprBase.base.lineNo,
                                                               BinaryExpr->exprBase.base.stringNo,
                                                               leftParameters.elementIndex,
                                                               elementDataTypeSize,
                                                               leftParameters.dataTypes[0].byteOffset,
                                                               scaledIndex));

                  if(clmDECL_IsUnderlyingStructOrUnion(&BinaryExpr->leftOperand->decl)) {
                      gctINT byteOffset = 0;
                      clsLOPERAND lOperand[1];
                      clsLOPERAND *lhs;

                      gcmASSERT(clmDECL_IsUnderlyingStructOrUnion(&BinaryExpr->rightOperand->decl));

                      if(scaledIndex->isReg) {
                         clsROPERAND rOperand[1];
                         clsIOPERAND iOperand[1];

                         clsIOPERAND_New(Compiler, iOperand, leftParameters.lOperands[0].dataType);
                         clsROPERAND_InitializeUsingLOperand(rOperand, leftParameters.lOperands);
                         status = clGenGenericCode2(Compiler,
                                                    BinaryExpr->exprBase.base.lineNo,
                                                    BinaryExpr->exprBase.base.stringNo,
                                                    clvOPCODE_ADD,
                                                    iOperand,
                                                    rOperand,
                                                    scaledIndex);
                          if (gcmIS_ERROR(status)) return status;
                          clsLOPERAND_InitializeUsingIOperand(lOperand, iOperand);
                          lhs = lOperand;
                      }
                      else {
                          byteOffset = _GetIntegerValue(scaledIndex);
                          lhs = leftParameters.lOperands;
                      }
                      _clmGenStructAssign(Compiler,
                                          &BinaryExpr->exprBase,
                                          lhs,
                                          byteOffset,
                                          &rightParameters,
                                          elementDataTypeSize,
                                          status);
                      gcmONERROR(status);
                  }
                  else {
                      gcmONERROR(_GenImplicitConvParametersToType(Compiler,
                                                                  BinaryExpr->rightOperand,
                                                                  BinaryExpr->leftOperand,
                                                                  &rightParameters,
                                                                  &leftParameters));

                      for (i = 0; i < leftParameters.operandCount; i++) {
                          gcmONERROR(clGenStoreCode(Compiler,
                                                    BinaryExpr->exprBase.base.lineNo,
                                                    BinaryExpr->exprBase.base.stringNo,
                                                    rightParameters.rOperands + i,
                                                    leftParameters.lOperands + i,
                                                    leftParameters.dataTypes[i].def,
                                                    scaledIndex));
                      }
                  }
               }
            }
            else if(leftParameters.hint & clvGEN_DEREF_STRUCT_CODE) {
               if(clmDECL_IsUnderlyingStructOrUnion(&BinaryExpr->leftOperand->decl)) {
                  gctUINT elementDataTypeSize = clsDECL_GetByteSize(&BinaryExpr->leftOperand->decl);

                  gcmASSERT(clmDECL_IsUnderlyingStructOrUnion(&BinaryExpr->rightOperand->decl));

                  _clmGenStructAssign(Compiler,
                                      &BinaryExpr->exprBase,
                                      leftParameters.lOperands,
                                      _GetIntegerValue(leftParameters.elementIndex),
                                      &rightParameters,
                                      elementDataTypeSize,
                                      status);
                  gcmONERROR(status);
               }
               else {
                  gcmONERROR(_GenImplicitConvParametersToType(Compiler,
                                                              BinaryExpr->rightOperand,
                                                              BinaryExpr->leftOperand,
                                                              &rightParameters,
                                                              &leftParameters));

                  for (i = 0; i < leftParameters.operandCount; i++) {
                      gcmONERROR(clGenStoreCode(Compiler,
                                                BinaryExpr->exprBase.base.lineNo,
                                                BinaryExpr->exprBase.base.stringNo,
                                                rightParameters.rOperands + i,
                                                leftParameters.lOperands + i,
                                                leftParameters.dataTypes[i].def,
                                                leftParameters.elementIndex));
                  }
               }
            }
            else {
               if(rightParameters.hint & clvGEN_ARRAY_OF_CONSTANTS) {
                   clsROPERAND constantOperand[1];
                   clsGEN_CODE_DATA_TYPE constantDataType;
                   gctBOOL allValuesEqual;
                   gctUINT8 componentCount;
                   gctUINT j;

                   gcmASSERT(rightParameters.constantArray);
                   gcmASSERT(!clmDECL_IsUnderlyingStructOrUnion(&BinaryExpr->leftOperand->decl));

                   clmGEN_CODE_ConvDirectElementDataType(rightParameters.constantArray->exprBase.decl.dataType, constantDataType);
                   allValuesEqual = rightParameters.constantArray->allValuesEqual;
                   if(allValuesEqual) {
                       clsROPERAND_InitializeConstant(constantOperand,
                                                      constantDataType,
                                                      1,
                                                      rightParameters.constantArray->values);
                       gcmONERROR(clsROPERAND_ChangeDataTypeFamily(Compiler,
                                                                   BinaryExpr->exprBase.base.lineNo,
                                                                   BinaryExpr->exprBase.base.stringNo,
                                                                   gcvFALSE,
                                                                   leftParameters.dataTypes[0].def,
                                                                   constantOperand));
                   }

                   j = 0;
                   componentCount = gcGetDataTypeComponentCount(constantDataType);
                   for (i = 0; i < leftParameters.operandCount; i++) {
                      if(!allValuesEqual) {
                          clsROPERAND_InitializeConstant(constantOperand,
                                                         constantDataType,
                                                         componentCount,
                                                         rightParameters.constantArray->values + j);
                          gcmONERROR(clsROPERAND_ChangeDataTypeFamily(Compiler,
                                                                      BinaryExpr->exprBase.base.lineNo,
                                                                      BinaryExpr->exprBase.base.stringNo,
                                                                      gcvFALSE,
                                                                      leftParameters.dataTypes[i].def,
                                                                      constantOperand));
                          j += componentCount;
                      }
                      gcmONERROR(clGenAssignCode(Compiler,
                                                 BinaryExpr->exprBase.base.lineNo,
                                                 BinaryExpr->exprBase.base.stringNo,
                                                 leftParameters.lOperands + i,
                                                 constantOperand));
                   }
               }
               else if(leftParameters.operandCount != rightParameters.operandCount) {
                  clsROPERAND scaledIndex[1];
                  clsLOPERAND addressBuffer[1];
                  clsLOPERAND *lOperand;
                  gctUINT incr = 0;

                  gctUINT elementDataTypeSize;

                  gcmASSERT(leftParameters.operandCount == 1); /* has to be a pointer */

                  elementDataTypeSize = clsDECL_GetByteSize(&BinaryExpr->leftOperand->decl);
                  if(leftParameters.elementIndex) {
                     gcmONERROR(clGenScaledIndexOperandWithOffset(Compiler,
                                                                  BinaryExpr->exprBase.base.lineNo,
                                                                  BinaryExpr->exprBase.base.stringNo,
                                                                  leftParameters.elementIndex,
                                                                  elementDataTypeSize,
                                                                  leftParameters.dataTypes[0].byteOffset,
                                                                  scaledIndex));

                     gcmONERROR(_AddLOperandOffset(Compiler,
                                                   BinaryExpr->exprBase.base.lineNo,
                                                   BinaryExpr->exprBase.base.stringNo,
                                                   leftParameters.lOperands,
                                                   scaledIndex,
                                                   addressBuffer));
                     lOperand = addressBuffer;
                  }
                  else {
                     lOperand = leftParameters.lOperands;
                  }

                  for (i = 0; i < rightParameters.operandCount; i++) {
                     clsROPERAND constantROperand[1];

                     clsROPERAND_InitializeScalarConstant(constantROperand,
                                                          clmGenCodeDataType(T_UINT),
                                                          uint,
                                                          incr);

                     gcmONERROR(clGenStoreCode(Compiler,
                                               BinaryExpr->exprBase.base.lineNo,
                                               BinaryExpr->exprBase.base.stringNo,
                                               rightParameters.rOperands + i,
                                               lOperand,
                                               rightParameters.dataTypes[i].def,
                                               constantROperand));
                     incr += _DataTypeByteSize(rightParameters.dataTypes[i].def);
                  }

               }
               else {
                  gcmONERROR(_GenImplicitConvParametersToType(Compiler,
                                      BinaryExpr->rightOperand,
                                      BinaryExpr->leftOperand,
                                      &rightParameters,
                                      &leftParameters));

                  if(clmDECL_IsUnderlyingStructOrUnion(&BinaryExpr->leftOperand->decl)) {
                      gctUINT elementDataTypeSize = clsDECL_GetByteSize(&BinaryExpr->rightOperand->decl);

                      gcmASSERT(clmDECL_IsUnderlyingStructOrUnion(&BinaryExpr->rightOperand->decl));
                      _clmGenStructAssign(Compiler,
                                          &BinaryExpr->exprBase,
                                          leftParameters.lOperands,
                                          leftParameters.dataTypes[0].byteOffset,
                                          &rightParameters,
                                          elementDataTypeSize,
                                          status);
                      gcmONERROR(status);
                  }
                  else {
                      for (i = 0; i < leftParameters.operandCount; i++) {
                          gcmONERROR(clGenAssignCode(Compiler,
                                                     BinaryExpr->exprBase.base.lineNo,
                                                     BinaryExpr->exprBase.base.stringNo,
                                                     leftParameters.lOperands + i,
                                                     rightParameters.rOperands + i));
                      }
                  }
               }
           }

           if (Parameters->needROperand) {
               clsGEN_CODE_PARAMETERS_MoveOperands(Parameters, &rightParameters);
               if(Parameters->hasIOperand) {
                  gcmASSERT(Parameters->operandCount == 1);
                  _clmAssignROperandToPreallocatedTempReg(Compiler,
                                                          Parameters,
                                                          &Parameters->rOperands[0],
                                                          status);
                  if(gcmIS_ERROR(status)) goto OnError;
               }
           }
        }

OnError:
        clsGEN_CODE_PARAMETERS_Finalize(&leftParameters);
        clsGEN_CODE_PARAMETERS_Finalize(&rightParameters);
    }

    return status;
}

gceSTATUS
cloIR_BINARY_EXPR_GenArithmeticAssignCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_BINARY_EXPR BinaryExpr,
    IN OUT clsGEN_CODE_PARAMETERS * Parameters
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    clsGEN_CODE_PARAMETERS    leftParameters, rightParameters;
    clsGEN_CODE_PARAMETERS    lOperandParameters;
    gctUINT    i;
    clsIOPERAND intermIOperand[1];
    clsROPERAND intermROperand[1];
        clsROPERAND scaledIndex[1];
    cleOPCODE opcode;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_OBJECT(CodeGenerator, clvOBJ_CODE_GENERATOR);
    clmVERIFY_IR_OBJECT(BinaryExpr, clvIR_BINARY_EXPR);
    gcmASSERT(Parameters);
    gcmASSERT(!Parameters->needLOperand);

    /* Generate the code of the left operand */
    gcmASSERT(BinaryExpr->leftOperand);

    clsGEN_CODE_PARAMETERS_Initialize(&leftParameters, gcvTRUE, gcvTRUE);
    clsGEN_CODE_PARAMETERS_Initialize(&lOperandParameters, gcvTRUE, gcvTRUE);
    clsGEN_CODE_PARAMETERS_Initialize(&rightParameters, gcvFALSE, gcvTRUE);

    leftParameters.hint = clvGEN_LEFT_ASSIGN_CODE;
    gcmONERROR(cloIR_OBJECT_Accept(Compiler,
                       &BinaryExpr->leftOperand->base,
                       &CodeGenerator->visitor,
                       &leftParameters));

    /* Generate the code of the right operand */
    gcmASSERT(BinaryExpr->rightOperand);


    gcmONERROR(cloIR_OBJECT_Accept(Compiler,
                       &BinaryExpr->rightOperand->base,
                       &CodeGenerator->visitor,
                       &rightParameters));

    /* Generate the arithmetic assign code */
    gcmASSERT(leftParameters.operandCount == rightParameters.operandCount);
    if(leftParameters.hint & clvGEN_DEREF_CODE) {
           gctUINT elementDataTypeSize;

       elementDataTypeSize = clsDECL_GetByteSize(&BinaryExpr->leftOperand->decl);
           gcmONERROR(clGenScaledIndexOperandWithOffset(Compiler,
                                                        BinaryExpr->exprBase.base.lineNo,
                                                        BinaryExpr->exprBase.base.stringNo,
                                                        leftParameters.elementIndex,
                                                        elementDataTypeSize,
                                                        leftParameters.dataTypes[0].byteOffset,
                                                        scaledIndex));

           if (leftParameters.needROperand) {
              clsIOPERAND iOperand[1];

              for (i = 0; i < leftParameters.operandCount; i++) {
                  clsIOPERAND_New(Compiler, iOperand, leftParameters.dataTypes[i].def);

                  gcmONERROR(clGenGenericCode2(Compiler,
                                               BinaryExpr->exprBase.base.lineNo,
                                               BinaryExpr->exprBase.base.stringNo,
                                               clvOPCODE_LOAD,
                                               iOperand,
                                               &leftParameters.rOperands[i],
                                               scaledIndex));
                  clsROPERAND_InitializeUsingIOperand(leftParameters.rOperands + i, iOperand);
              }
       }
        }
    else if(leftParameters.hint & clvGEN_DEREF_STRUCT_CODE) {
           if (leftParameters.needROperand) {
              clsIOPERAND iOperand[1];

          for (i = 0; i < leftParameters.operandCount; i++) {
                  clsIOPERAND_New(Compiler, iOperand, leftParameters.dataTypes[i].def);

                  gcmONERROR(clGenGenericCode2(Compiler,
                                               BinaryExpr->exprBase.base.lineNo,
                                               BinaryExpr->exprBase.base.stringNo,
                                               clvOPCODE_LOAD,
                                               iOperand,
                                               &leftParameters.rOperands[i],
                                               leftParameters.elementIndex));
                  clsROPERAND_InitializeUsingIOperand(leftParameters.rOperands + i, iOperand);
              }
       }
    }

/*klc*/
    if(!clmDECL_IsPointerType(&BinaryExpr->leftOperand->decl)) {
       gcmONERROR(clsGEN_CODE_PARAMETERS_CopyOperands(Compiler,
                                                      &lOperandParameters,
                                                      &leftParameters));

       gcmONERROR(clGenImplicitConversion(Compiler,
                                              BinaryExpr->leftOperand,
                                              BinaryExpr->rightOperand,
                                              &lOperandParameters,
                                              &rightParameters));
    }

    switch (BinaryExpr->type) {
    case clvBINARY_ADD_ASSIGN:
        if(clmDECL_IsFloatingType(&BinaryExpr->leftOperand->decl) &&
              CodeGenerator->needFloatingPointAccuracy) {
            opcode = clvOPCODE_FADD;
        }
        else {
            opcode = clvOPCODE_ADD;
        }
        break;
    case clvBINARY_SUB_ASSIGN:
        if(clmDECL_IsFloatingType(&BinaryExpr->leftOperand->decl) &&
              CodeGenerator->needFloatingPointAccuracy) {
            opcode = clvOPCODE_FSUB;
        }
        else {
            opcode = clvOPCODE_SUB;
        }
        break;
    case clvBINARY_MUL_ASSIGN:
        if(clmDECL_IsIntegerType(&BinaryExpr->leftOperand->decl)) {
             opcode = clvOPCODE_IMUL;
        }
        else if(CodeGenerator->needFloatingPointAccuracy) {
                     opcode = clvOPCODE_FMUL;
        }
        else {
                     opcode = clvOPCODE_MUL;
        }
        break;

    case clvBINARY_DIV_ASSIGN:
        if(clmDECL_IsIntegerType(&BinaryExpr->leftOperand->decl)) {
             opcode = clvOPCODE_IDIV;
        }
        else {
                     opcode = clvOPCODE_DIV;
        }
        break;

    case clvBINARY_MOD_ASSIGN:
        if(clmDECL_IsIntegerType(&BinaryExpr->leftOperand->decl)) {
             opcode = clvOPCODE_MOD;
        }
        else {
                     opcode = clvOPCODE_FMOD;
        }
        break;

    default:
        gcmASSERT(0);
        return gcvSTATUS_INVALID_ARGUMENT;
    }

    if(Parameters->needROperand) {
        gcmONERROR(clsGEN_CODE_PARAMETERS_AllocateOperands(Compiler,
                                   Parameters,
                                   &BinaryExpr->exprBase.decl));
    }

    if(clmDECL_IsPointerType(&BinaryExpr->leftOperand->decl)) {
          intermROperand->dataType = leftParameters.dataTypes[0].def;
              gcmONERROR(clGenPointerArithmeticCode(Compiler,
                                &BinaryExpr->exprBase,
                            opcode,
                            clvGEN_NO_HINT,
                            &leftParameters,
                            &rightParameters.rOperands[0],
                                                    intermROperand));
          clsIOPERAND_Initialize(intermIOperand, intermROperand->dataType, intermROperand->u.reg.regIndex);
              /* Generate the assign code */
          gcmONERROR(clGenAssignCode(Compiler,
                         BinaryExpr->exprBase.base.lineNo,
                         BinaryExpr->exprBase.base.stringNo,
                         leftParameters.lOperands,
                         intermROperand));
          if (Parameters->needROperand) {
          Parameters->rOperands[0] = *intermROperand;
          if (Parameters->hasIOperand) {
              _clmAssignROperandToPreallocatedTempReg(Compiler,
                                  Parameters,
                                  &Parameters->rOperands[0],
                                  status);
              if(gcmIS_ERROR(status)) goto OnError;
          }
          }
    }
    else {
       for (i = 0; i < leftParameters.operandCount; i++) {
          /* Generate the arithmetic code */
              clmGEN_CODE_GetParametersIOperand(Compiler, intermIOperand, Parameters, lOperandParameters.dataTypes[i].def);

          if(opcode == clvOPCODE_FMOD || opcode == clvOPCODE_DIV ||
                 opcode == clvOPCODE_IDIV || opcode == clvOPCODE_MOD ||
         opcode == clvOPCODE_IMUL || opcode == clvOPCODE_FMUL ||
         opcode == clvOPCODE_FADD || opcode == clvOPCODE_FSUB) {
                   cloIR_POLYNARY_EXPR funcCall;
           clsGEN_CODE_PARAMETERS operandsParameters[2];

           operandsParameters[0] = lOperandParameters;
           operandsParameters[1] = rightParameters;
                   gcmONERROR(_ConvExprToFuncCall(Compiler,
                                                  opcode,
                                                  &BinaryExpr->exprBase,
                                                  &funcCall));

                   gcmONERROR(clGenBuiltinFunctionCode(Compiler,
                                                       CodeGenerator,
                                                       funcCall,
                                                       2,
                                                       operandsParameters,
                                                       intermIOperand,
                               &lOperandParameters,
                               gcvTRUE));
           }
          else {
             gcmONERROR(clGenArithmeticExprCode(Compiler,
                                   BinaryExpr->exprBase.base.lineNo,
                                   BinaryExpr->exprBase.base.stringNo,
                                   opcode,
                                   intermIOperand,
                                   lOperandParameters.rOperands + i,
                                   rightParameters.rOperands + i));
          }

          clsROPERAND_InitializeUsingIOperand(intermROperand, intermIOperand);

          gcmONERROR(_GenConvROperandForAssign(Compiler,
                                  BinaryExpr->exprBase.base.lineNo,
                                  BinaryExpr->exprBase.base.stringNo,
                           leftParameters.dataTypes[i].def,
                           intermROperand));

          if(leftParameters.hint & clvGEN_DEREF_CODE) {
             gcmONERROR(clGenStoreCode(Compiler,
                                           BinaryExpr->exprBase.base.lineNo,
                                           BinaryExpr->exprBase.base.stringNo,
                                           intermROperand,
                                           leftParameters.lOperands + i,
                                 leftParameters.dataTypes[i].def,
                                           scaledIndex));
              }
          else if(leftParameters.hint & clvGEN_DEREF_STRUCT_CODE) {
             gcmONERROR(clGenStoreCode(Compiler,
                                           BinaryExpr->exprBase.base.lineNo,
                                           BinaryExpr->exprBase.base.stringNo,
                                           intermROperand,
                                           leftParameters.lOperands + i,
                                 leftParameters.dataTypes[i].def,
                                           leftParameters.elementIndex));
          }
              else {
           /* Generate the assign code */
            gcmONERROR(clGenAssignCode(Compiler,
                           BinaryExpr->exprBase.base.lineNo,
                           BinaryExpr->exprBase.base.stringNo,
                           leftParameters.lOperands + i,
                           intermROperand));
              }
          if (Parameters->needROperand) {
          Parameters->rOperands[i] = *intermROperand;
          }
       }
    }

OnError:
    clsGEN_CODE_PARAMETERS_Finalize(&lOperandParameters);
    clsGEN_CODE_PARAMETERS_Finalize(&leftParameters);
    clsGEN_CODE_PARAMETERS_Finalize(&rightParameters);

    return gcvSTATUS_OK;
}

gceSTATUS
cloIR_BINARY_EXPR_GenShiftAssignCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_BINARY_EXPR BinaryExpr,
    IN OUT clsGEN_CODE_PARAMETERS * Parameters
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    clsGEN_CODE_PARAMETERS leftParameters, rightParameters;
    clsGEN_CODE_PARAMETERS operandsParameters[2];
        cloIR_POLYNARY_EXPR funcCall;
    gctUINT    i;
    clsIOPERAND intermIOperand[1];
    clsROPERAND intermROperand[1];
        clsROPERAND scaledIndex[1];
    cleOPCODE opcode;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_OBJECT(CodeGenerator, clvOBJ_CODE_GENERATOR);
    clmVERIFY_IR_OBJECT(BinaryExpr, clvIR_BINARY_EXPR);
    gcmASSERT(Parameters);
    gcmASSERT(!Parameters->needLOperand);

    /* Generate the code of the left operand */
    gcmASSERT(BinaryExpr->leftOperand);

    clsGEN_CODE_PARAMETERS_Initialize(&leftParameters, gcvTRUE, gcvTRUE);
    clsGEN_CODE_PARAMETERS_Initialize(&rightParameters, gcvFALSE, gcvTRUE);

    leftParameters.hint = clvGEN_LEFT_ASSIGN_CODE;
    gcmONERROR(cloIR_OBJECT_Accept(Compiler,
                       &BinaryExpr->leftOperand->base,
                       &CodeGenerator->visitor,
                       &leftParameters));

    /* Generate the code of the right operand */
    gcmASSERT(BinaryExpr->rightOperand);


    gcmONERROR(cloIR_OBJECT_Accept(Compiler,
                       &BinaryExpr->rightOperand->base,
                       &CodeGenerator->visitor,
                       &rightParameters));

    /* Generate the shift assign code */
    gcmASSERT(leftParameters.operandCount == rightParameters.operandCount);
    if(leftParameters.hint & clvGEN_DEREF_CODE) {
           gctUINT elementDataTypeSize;

       elementDataTypeSize = clsDECL_GetByteSize(&BinaryExpr->leftOperand->decl);
           gcmONERROR(clGenScaledIndexOperandWithOffset(Compiler,
                                                        BinaryExpr->exprBase.base.lineNo,
                                                        BinaryExpr->exprBase.base.stringNo,
                                                        leftParameters.elementIndex,
                                                        elementDataTypeSize,
                                                        leftParameters.dataTypes[0].byteOffset,
                                                        scaledIndex));

           if (leftParameters.needROperand) {
              clsIOPERAND iOperand[1];

              for (i = 0; i < leftParameters.operandCount; i++) {
                  clsIOPERAND_New(Compiler, iOperand, leftParameters.dataTypes[i].def);

                  gcmONERROR(clGenGenericCode2(Compiler,
                                               BinaryExpr->exprBase.base.lineNo,
                                               BinaryExpr->exprBase.base.stringNo,
                                               clvOPCODE_LOAD,
                                               iOperand,
                                               &leftParameters.rOperands[i],
                                               scaledIndex));
                  clsROPERAND_InitializeUsingIOperand(leftParameters.rOperands + i, iOperand);
              }
       }
        }
    else if(leftParameters.hint & clvGEN_DEREF_STRUCT_CODE) {
           if (leftParameters.needROperand) {
              clsIOPERAND iOperand[1];

          for (i = 0; i < leftParameters.operandCount; i++) {
                  clsIOPERAND_New(Compiler, iOperand, leftParameters.dataTypes[i].def);

                  gcmONERROR(clGenGenericCode2(Compiler,
                                               BinaryExpr->exprBase.base.lineNo,
                                               BinaryExpr->exprBase.base.stringNo,
                                               clvOPCODE_LOAD,
                                               iOperand,
                                               &leftParameters.rOperands[i],
                                               leftParameters.elementIndex));
                  clsROPERAND_InitializeUsingIOperand(leftParameters.rOperands + i, iOperand);
              }
       }
    }

    switch (BinaryExpr->type) {
    case clvBINARY_LEFT_ASSIGN:
       opcode = clvOPCODE_LEFT_SHIFT;
       break;

    case clvBINARY_RIGHT_ASSIGN:
       opcode = clvOPCODE_RIGHT_SHIFT;
       break;

    default:
        gcmASSERT(0);
        return gcvSTATUS_INVALID_ARGUMENT;
    }

    if(clmDECL_IsVectorType(&BinaryExpr->leftOperand->decl) &&
       clmDECL_IsScalar(&BinaryExpr->rightOperand->decl)) {
        gcmONERROR(_ConvScalarToVector(Compiler,
                                   clmDATA_TYPE_vectorSize_NOCHECK_GET(BinaryExpr->leftOperand->decl.dataType),
                                   BinaryExpr->rightOperand,
                                   &rightParameters));
    }

    if(Parameters->needROperand) {
        gcmONERROR(clsGEN_CODE_PARAMETERS_AllocateOperands(Compiler,
                                   Parameters,
                                   &BinaryExpr->exprBase.decl));
    }

    operandsParameters[0] = leftParameters;
    operandsParameters[1] = rightParameters;
    for (i = 0; i < leftParameters.operandCount; i++) {
          /* Generate the shift code */
              clmGEN_CODE_GetParametersIOperand(Compiler, intermIOperand, Parameters, leftParameters.dataTypes[i].def);

              gcmONERROR(_ConvExprToFuncCall(Compiler,
                                             opcode,
                                             &BinaryExpr->exprBase,
                                             &funcCall));

              gcmONERROR(clGenBuiltinFunctionCode(Compiler,
                                                  CodeGenerator,
                                                  funcCall,
                                                  2,
                                                  operandsParameters,
                                                  intermIOperand,
                          &leftParameters,
                          gcvTRUE));

          clsROPERAND_InitializeUsingIOperand(intermROperand, intermIOperand);

          if(leftParameters.hint & clvGEN_DEREF_CODE) {
             gcmONERROR(clGenStoreCode(Compiler,
                                           BinaryExpr->exprBase.base.lineNo,
                                           BinaryExpr->exprBase.base.stringNo,
                                           intermROperand,
                                           leftParameters.lOperands + i,
                                 leftParameters.dataTypes[i].def,
                                           scaledIndex));
              }
          else if(leftParameters.hint & clvGEN_DEREF_STRUCT_CODE) {
             gcmONERROR(clGenStoreCode(Compiler,
                                           BinaryExpr->exprBase.base.lineNo,
                                           BinaryExpr->exprBase.base.stringNo,
                                           intermROperand,
                                           leftParameters.lOperands + i,
                                 leftParameters.dataTypes[i].def,
                                           leftParameters.elementIndex));
          }
              else {
           /* Generate the assign code */
            gcmONERROR(clGenAssignCode(Compiler,
                           BinaryExpr->exprBase.base.lineNo,
                           BinaryExpr->exprBase.base.stringNo,
                           leftParameters.lOperands + i,
                           intermROperand));
              }
          if (Parameters->needROperand) {
          Parameters->rOperands[i] = *intermROperand;
          }
    }

OnError:
    clsGEN_CODE_PARAMETERS_Finalize(&leftParameters);
    clsGEN_CODE_PARAMETERS_Finalize(&rightParameters);

    return status;
}

gceSTATUS
cloIR_BINARY_EXPR_GenBitwiseAssignCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_BINARY_EXPR BinaryExpr,
    IN OUT clsGEN_CODE_PARAMETERS * Parameters
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    clsGEN_CODE_PARAMETERS    leftParameters, rightParameters;
    gctUINT    i;
    clsIOPERAND intermIOperand[1];
    clsROPERAND intermROperand[1];
        clsROPERAND scaledIndex[1];
    cleOPCODE opcode;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_OBJECT(CodeGenerator, clvOBJ_CODE_GENERATOR);
    clmVERIFY_IR_OBJECT(BinaryExpr, clvIR_BINARY_EXPR);
    gcmASSERT(Parameters);
    gcmASSERT(!Parameters->needLOperand);

    /* Generate the code of the left operand */
    gcmASSERT(BinaryExpr->leftOperand);

    clsGEN_CODE_PARAMETERS_Initialize(&leftParameters, gcvTRUE, gcvTRUE);
    clsGEN_CODE_PARAMETERS_Initialize(&rightParameters, gcvFALSE, gcvTRUE);

    leftParameters.hint = clvGEN_LEFT_ASSIGN_CODE;
    gcmONERROR(cloIR_OBJECT_Accept(Compiler,
                       &BinaryExpr->leftOperand->base,
                       &CodeGenerator->visitor,
                       &leftParameters));

    /* Generate the code of the right operand */
    gcmASSERT(BinaryExpr->rightOperand);

    gcmONERROR(cloIR_OBJECT_Accept(Compiler,
                       &BinaryExpr->rightOperand->base,
                       &CodeGenerator->visitor,
                       &rightParameters));

    /* Generate the shift assign code */
    gcmASSERT(leftParameters.operandCount == rightParameters.operandCount);
    if(leftParameters.hint & clvGEN_DEREF_CODE) {
           gctUINT elementDataTypeSize;

       elementDataTypeSize = clsDECL_GetByteSize(&BinaryExpr->leftOperand->decl);
           gcmONERROR(clGenScaledIndexOperandWithOffset(Compiler,
                                                        BinaryExpr->exprBase.base.lineNo,
                                                        BinaryExpr->exprBase.base.stringNo,
                                                        leftParameters.elementIndex,
                                                        elementDataTypeSize,
                                                        leftParameters.dataTypes[0].byteOffset,
                                                        scaledIndex));

           if (leftParameters.needROperand) {
              clsIOPERAND iOperand[1];

              for (i = 0; i < leftParameters.operandCount; i++) {
                  clsIOPERAND_New(Compiler, iOperand, leftParameters.dataTypes[i].def);

                  gcmONERROR(clGenGenericCode2(Compiler,
                                               BinaryExpr->exprBase.base.lineNo,
                                               BinaryExpr->exprBase.base.stringNo,
                                               clvOPCODE_LOAD,
                                               iOperand,
                                               &leftParameters.rOperands[i],
                                               scaledIndex));
                  clsROPERAND_InitializeUsingIOperand(leftParameters.rOperands + i, iOperand);
              }
       }
        }
    else if(leftParameters.hint & clvGEN_DEREF_STRUCT_CODE) {
           if (leftParameters.needROperand) {
              clsIOPERAND iOperand[1];

          for (i = 0; i < leftParameters.operandCount; i++) {
                  clsIOPERAND_New(Compiler, iOperand, leftParameters.dataTypes[i].def);

                  gcmONERROR(clGenGenericCode2(Compiler,
                                               BinaryExpr->exprBase.base.lineNo,
                                               BinaryExpr->exprBase.base.stringNo,
                                               clvOPCODE_LOAD,
                                               iOperand,
                                               &leftParameters.rOperands[i],
                                               leftParameters.elementIndex));
                  clsROPERAND_InitializeUsingIOperand(leftParameters.rOperands + i, iOperand);
              }
       }
    }

    gcmONERROR(clGenImplicitConversion(Compiler,
                                           BinaryExpr->leftOperand,
                                           BinaryExpr->rightOperand,
                                           &leftParameters,
                                           &rightParameters));

    switch (BinaryExpr->type) {
    case clvBINARY_AND_ASSIGN: opcode = clvOPCODE_BITWISE_AND; break;
    case clvBINARY_XOR_ASSIGN: opcode = clvOPCODE_BITWISE_XOR; break;
    case clvBINARY_OR_ASSIGN: opcode = clvOPCODE_BITWISE_OR; break;

    default:
        gcmASSERT(0);
        status = gcvSTATUS_INVALID_ARGUMENT;
        goto OnError;
    }

    if(Parameters->needROperand) {
        gcmONERROR(clsGEN_CODE_PARAMETERS_AllocateOperands(Compiler,
                                   Parameters,
                                   &BinaryExpr->exprBase.decl));
    }

    for (i = 0; i < leftParameters.operandCount; i++) {
          /* Generate the shift code */
              clmGEN_CODE_GetParametersIOperand(Compiler, intermIOperand, Parameters, leftParameters.dataTypes[i].def);

          gcmONERROR(clGenBitwiseExprCode(Compiler,
                             BinaryExpr->exprBase.base.lineNo,
                             BinaryExpr->exprBase.base.stringNo,
                             opcode,
                             intermIOperand,
                             leftParameters.rOperands + i,
                             rightParameters.rOperands + i));

          clsROPERAND_InitializeUsingIOperand(intermROperand, intermIOperand);

          if(leftParameters.hint & clvGEN_DEREF_CODE) {
             gcmONERROR(clGenStoreCode(Compiler,
                                           BinaryExpr->exprBase.base.lineNo,
                                           BinaryExpr->exprBase.base.stringNo,
                                           intermROperand,
                                           leftParameters.lOperands + i,
                                 leftParameters.dataTypes[i].def,
                                           scaledIndex));
              }
          else if(leftParameters.hint & clvGEN_DEREF_STRUCT_CODE) {
             gcmONERROR(clGenStoreCode(Compiler,
                                           BinaryExpr->exprBase.base.lineNo,
                                           BinaryExpr->exprBase.base.stringNo,
                                           intermROperand,
                                           leftParameters.lOperands + i,
                                 leftParameters.dataTypes[i].def,
                                           leftParameters.elementIndex));
          }
              else {
            /* Generate the assign code */
            gcmONERROR(clGenAssignCode(Compiler,
                           BinaryExpr->exprBase.base.lineNo,
                           BinaryExpr->exprBase.base.stringNo,
                           leftParameters.lOperands + i,
                           intermROperand));
              }
          if (Parameters->needROperand) {
          Parameters->rOperands[i] = *intermROperand;
          }
    }

OnError:
    clsGEN_CODE_PARAMETERS_Finalize(&leftParameters);
    clsGEN_CODE_PARAMETERS_Finalize(&rightParameters);

    return status;
}

gceSTATUS
cloIR_BINARY_EXPR_GenCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_BINARY_EXPR BinaryExpr,
    IN OUT clsGEN_CODE_PARAMETERS * Parameters
    )
{
    gceSTATUS status;
    clsGEN_CODE_PARAMETERS    leftParameters, rightParameters;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_OBJECT(CodeGenerator, clvOBJ_CODE_GENERATOR);
    clmVERIFY_IR_OBJECT(BinaryExpr, clvIR_BINARY_EXPR);
    gcmASSERT(Parameters);

    /* Try to evaluate the operands */
    do {
      if (!Parameters->needLOperand && Parameters->needROperand &&
          !(Parameters->hint & clvGEN_ADDR_CODE))
      {
        /* Try to evaluate the left operand */
        gcmASSERT(BinaryExpr->leftOperand);

        clsGEN_CODE_PARAMETERS_Initialize(&leftParameters,
                          gcvFALSE,
                          gcvTRUE);

        leftParameters.hint = clvEVALUATE_ONLY;

        status = cloIR_OBJECT_Accept(Compiler,
                         &BinaryExpr->leftOperand->base,
                         &CodeGenerator->visitor,
                         &leftParameters);
        if (gcmIS_ERROR(status)) return status;

        if (leftParameters.constant == gcvNULL) {
           clsGEN_CODE_PARAMETERS_Finalize(&leftParameters);
           break;
        }

        /* Try to evaluate the right operand */
        gcmASSERT(BinaryExpr->rightOperand);
        clsGEN_CODE_PARAMETERS_Initialize(&rightParameters,
                          gcvFALSE,
                          gcvTRUE);

        rightParameters.hint = clvEVALUATE_ONLY;
        if(clmIR_EXPR_IsBinaryType(BinaryExpr->rightOperand, clvBINARY_MULTI_DIM_SUBSCRIPT)) {
           gctINT dim;

           dim = _EvaluateArrayOffset(Compiler,
                              CodeGenerator,
                              &BinaryExpr->leftOperand->decl.array,
                              1,
                              (cloIR_BINARY_EXPR)&BinaryExpr->rightOperand->base,
                              &rightParameters);

           if(dim < 0) {
              gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                              BinaryExpr->exprBase.base.lineNo,
                              BinaryExpr->exprBase.base.stringNo,
                              clvREPORT_ERROR,
                              "internal error: failed to compute array indices"));

              return gcvSTATUS_INVALID_DATA;
           }
        }
        else {
           status = cloIR_OBJECT_Accept(Compiler,
                            &BinaryExpr->rightOperand->base,
                            &CodeGenerator->visitor,
                            &rightParameters);
           if (gcmIS_ERROR(status)) return status;
        }

        if (leftParameters.constant != gcvNULL && rightParameters.constant != gcvNULL)
        {
            status = cloIR_BINARY_EXPR_Evaluate(Compiler,
                                BinaryExpr->type,
                                leftParameters.constant,
                                rightParameters.constant,
                                &BinaryExpr->exprBase.decl,
                                &Parameters->constant);
            if (gcmIS_ERROR(status)) return status;

            leftParameters.constant        = gcvNULL;
            rightParameters.constant    = gcvNULL;
        }

        clsGEN_CODE_PARAMETERS_Finalize(&leftParameters);
        clsGEN_CODE_PARAMETERS_Finalize(&rightParameters);

        if (Parameters->hint == clvEVALUATE_ONLY) return gcvSTATUS_OK;

        if (Parameters->constant != gcvNULL)
        {
            return cloIR_CONSTANT_GenCode(Compiler,
                              CodeGenerator,
                              Parameters->constant,
                              Parameters);
        }
      }
    } while (gcvFALSE);

    if (Parameters->hint == clvEVALUATE_ONLY) return gcvSTATUS_OK;

    switch (BinaryExpr->type)
    {
    case clvBINARY_SUBSCRIPT:
        return cloIR_BINARY_EXPR_GenSubscriptCode(Compiler,
                              CodeGenerator,
                              BinaryExpr,
                              Parameters);

    case clvBINARY_ADD:
    case clvBINARY_SUB:
    case clvBINARY_MUL:
    case clvBINARY_DIV:
    case clvBINARY_MOD:
        return cloIR_BINARY_EXPR_GenArithmeticCode(Compiler,
                               CodeGenerator,
                               BinaryExpr,
                               Parameters);

    case clvBINARY_GREATER_THAN:
    case clvBINARY_LESS_THAN:
    case clvBINARY_GREATER_THAN_EQUAL:
    case clvBINARY_LESS_THAN_EQUAL:
        return cloIR_BINARY_EXPR_GenRelationalCode(Compiler,
                               CodeGenerator,
                               BinaryExpr,
                               Parameters);

    case clvBINARY_EQUAL:
    case clvBINARY_NOT_EQUAL:
    case clvBINARY_XOR:
        return cloIR_BINARY_EXPR_GenEqualityCode(Compiler,
                             CodeGenerator,
                             BinaryExpr,
                             Parameters);

    case clvBINARY_AND:
        return cloIR_BINARY_EXPR_GenAndCode(Compiler,
                            CodeGenerator,
                            BinaryExpr,
                            Parameters);

    case clvBINARY_OR:
        return cloIR_BINARY_EXPR_GenOrCode(Compiler,
                           CodeGenerator,
                           BinaryExpr,
                           Parameters);
    case clvBINARY_BITWISE_AND:
    case clvBINARY_BITWISE_OR:
    case clvBINARY_BITWISE_XOR:
        return cloIR_BINARY_EXPR_GenBitwiseCode(Compiler,
                                CodeGenerator,
                                BinaryExpr,
                                Parameters);

    case clvBINARY_LSHIFT:
    case clvBINARY_RSHIFT:
        return cloIR_BINARY_EXPR_GenShiftCode(Compiler,
                              CodeGenerator,
                              BinaryExpr,
                              Parameters);

    case clvBINARY_SEQUENCE:
        return cloIR_BINARY_EXPR_GenSequenceCode(Compiler,
                            CodeGenerator,
                            BinaryExpr,
                            Parameters);

    case clvBINARY_ASSIGN:
        return cloIR_BINARY_EXPR_GenAssignCode(Compiler,
                               CodeGenerator,
                               BinaryExpr,
                               Parameters);

    case clvBINARY_LEFT_ASSIGN:
    case clvBINARY_RIGHT_ASSIGN:
        return cloIR_BINARY_EXPR_GenShiftAssignCode(Compiler,
                                    CodeGenerator,
                                    BinaryExpr,
                                    Parameters);

    case clvBINARY_AND_ASSIGN:
    case clvBINARY_XOR_ASSIGN:
    case clvBINARY_OR_ASSIGN:
        return cloIR_BINARY_EXPR_GenBitwiseAssignCode(Compiler,
                                      CodeGenerator,
                                      BinaryExpr,
                                      Parameters);

    case clvBINARY_MUL_ASSIGN:
    case clvBINARY_DIV_ASSIGN:
    case clvBINARY_ADD_ASSIGN:
    case clvBINARY_SUB_ASSIGN:
    case clvBINARY_MOD_ASSIGN:
        return cloIR_BINARY_EXPR_GenArithmeticAssignCode(Compiler,
                                 CodeGenerator,
                                 BinaryExpr,
                                 Parameters);

    default:
        gcmASSERT(0);
        return gcvSTATUS_INVALID_ARGUMENT;
    }
}

gceSTATUS
clGenSelectExprCode(
    IN cloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN cleOPCODE Opcode,
    IN clsIOPERAND * IOperand,
    IN clsROPERAND * Cond,
    IN clsROPERAND * ROperand0,
    IN clsROPERAND * ROperand1
    )
{
   gceSTATUS status;
   gctINT i;
   gcsSUPER_TARGET superTarget;
   gcsSUPER_SOURCE superCond;
   gcsSUPER_SOURCE superSource0;
   gcsSUPER_SOURCE superSource1;
   clsROPERAND lshiftROperand[1], rshiftROperand[1];
   clsIOPERAND    intermIOperand[1], iOperand[1];
   clsROPERAND    intermROperand[1], rOperand[1];
   cltELEMENT_TYPE copyCondType;
   cltELEMENT_TYPE copyType0 = 0;
   cltELEMENT_TYPE copyType1 = 0;
   cltELEMENT_TYPE copyIOperandType = 0;

   /* Verify the arguments. */
   clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
   gcmASSERT(IOperand);
   gcmASSERT(Cond);
   gcmASSERT(ROperand0);
   gcmASSERT(ROperand1);

   gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                                 clvDUMP_CODE_GENERATOR,
                                 "<OPERATION line=\"%d\" string=\"%d\" type=\"%s\">",
                                 LineNo,
                                 StringNo,
                                 clGetOpcodeName(Opcode)));

   gcmVERIFY_OK(clsIOPERAND_Dump(Compiler, IOperand));
   gcmVERIFY_OK(clsROPERAND_Dump(Compiler, Cond));
   gcmVERIFY_OK(clsROPERAND_Dump(Compiler, ROperand0));
   gcmVERIFY_OK(clsROPERAND_Dump(Compiler, ROperand1));

   gcmASSERT(Opcode == clvOPCODE_SELECT);

   /* Always use integer operation, avoid Nan, Inf, denorm*/
   copyCondType = Cond->dataType.elementType;
   Cond->dataType.elementType = clvTYPE_INT;
   clsIOPERAND_New(Compiler, intermIOperand, ROperand0->dataType);
   intermIOperand->dataType.elementType = clvTYPE_INT;
   clsIOPERAND_New(Compiler, iOperand, Cond->dataType);
   iOperand->dataType.elementType = clvTYPE_INT;
   clsROPERAND_InitializeUsingIOperand(intermROperand, intermIOperand);
   clsROPERAND_InitializeUsingIOperand(rOperand, iOperand);

   clsROPERAND_InitializeIntOrIVecConstant(rshiftROperand,
                                           clmGenCodeDataType(T_INT),
                                           (gctUINT)31);

   switch(copyCondType) {
   case clvTYPE_FLOAT:
   case clvTYPE_UINT:
   case clvTYPE_INT:
   case clvTYPE_SHORT:
   case clvTYPE_CHAR:
        gcmONERROR(clGenShiftExprCode(Compiler,
                                      LineNo,
                                      StringNo,
                                      clvOPCODE_RSHIFT,
                                      iOperand,
                                      Cond,
                                      rshiftROperand));
        break;

   case clvTYPE_USHORT:
        clsROPERAND_InitializeIntOrIVecConstant(lshiftROperand,
                                                clmGenCodeDataType(T_INT),
                                                (gctUINT)16);

        gcmONERROR(clGenShiftExprCode(Compiler,
                                      LineNo,
                                      StringNo,
                                      clvOPCODE_LSHIFT,
                                      intermIOperand,
                                      Cond,
                                      lshiftROperand));

        gcmONERROR(clGenShiftExprCode(Compiler,
                                      LineNo,
                                      StringNo,
                                      clvOPCODE_RSHIFT,
                                      iOperand,
                                      intermROperand,
                                      rshiftROperand));
        break;

   case clvTYPE_UCHAR:
        clsROPERAND_InitializeIntOrIVecConstant(lshiftROperand,
                                                clmGenCodeDataType(T_USHORT),
                                                (gctUINT)24);
        gcmONERROR(clGenShiftExprCode(Compiler,
                                      LineNo,
                                      StringNo,
                                      clvOPCODE_LSHIFT,
                                      intermIOperand,
                                      Cond,
                                      lshiftROperand));

        gcmONERROR(clGenShiftExprCode(Compiler,
                                      LineNo,
                                      StringNo,
                                      clvOPCODE_RSHIFT,
                                      iOperand,
                                      intermROperand,
                                      rshiftROperand));
        break;
   default:
        break;
   }
   copyType0 = ROperand0->dataType.elementType;
   if(clmIsElementTypeFloating(ROperand0->dataType.elementType)) {
       ROperand0->dataType.elementType = clvTYPE_INT;
   }
   copyType1 = ROperand1->dataType.elementType;
   if(clmIsElementTypeFloating(ROperand1->dataType.elementType)) {
       ROperand1->dataType.elementType = clvTYPE_INT;
   }
   copyIOperandType = IOperand->dataType.elementType;
   if(clmIsElementTypeFloating(IOperand->dataType.elementType)) {
       IOperand->dataType.elementType = clvTYPE_INT;
   }
   gcmVERIFY_OK(_ConvIOperandToSuperTarget(Compiler,
                                           IOperand,
                                           &superTarget));

   gcmONERROR(_ConvNormalROperandToSuperSource(Compiler,
                                               LineNo,
                                               StringNo,
                                               rOperand,
                                               &superCond));

   gcmONERROR(_ConvNormalROperandToSuperSource(Compiler,
                                               LineNo,
                                               StringNo,
                                               ROperand0,
                                               &superSource0));

   gcmONERROR(_ConvNormalROperandToSuperSource(Compiler,
                                               LineNo,
                                               StringNo,
                                               ROperand1,
                                               &superSource1));

   _SplitTargets(&superTarget,
                 superSource0.numSources > superSource1.numSources
                                         ? superSource0.numSources
                                         : superSource1.numSources);
   _SplitSources(&superSource0, superTarget.numTargets);
   _SplitSources(&superSource1, superTarget.numTargets);
   _SplitSources(&superCond, superTarget.numTargets);
   gcmASSERT(superTarget.numTargets == superSource0.numSources &&
             superTarget.numTargets == superSource1.numSources &&
             superTarget.numTargets == superCond.numSources);

   for(i=0; i < superTarget.numTargets; i++) {
      gcmONERROR(clEmitCompareSetCode(Compiler,
                                      LineNo,
                                      StringNo,
                                      clvOPCODE_SET,
                                      superTarget.targets + i,
                                      superCond.sources + i,
                                      superSource0.sources + i,
                                      superSource1.sources + i));
   }

   gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                                 clvDUMP_CODE_GENERATOR,
                                 "</OPERATION>"));

OnError:
   Cond->dataType.elementType = copyCondType;
   ROperand0->dataType.elementType = copyType0;
   ROperand1->dataType.elementType = copyType1;
   IOperand->dataType.elementType = copyIOperandType;
   return status;
}

gceSTATUS
cloIR_SELECTION_GenCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_SELECTION Selection,
    IN OUT clsGEN_CODE_PARAMETERS * Parameters
    )
{
    gceSTATUS    status;
    gctBOOL        emptySelection;
    clsGEN_CODE_PARAMETERS    condParameters, trueParameters, falseParameters;
    clsSELECTION_CONTEXT    selectionContext;
    gctBOOL        trueOperandHasReturn;
    clsLOPERAND    lOperand[1];
    clsIOPERAND    iOperand[1];
    clsIOPERAND    *intermIOperand = gcvNULL;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_OBJECT(CodeGenerator, clvOBJ_CODE_GENERATOR);
    clmVERIFY_IR_OBJECT(Selection, clvIR_SELECTION);
    gcmASSERT(Selection->condExpr);
    gcmASSERT(Parameters);
    gcmASSERT(!Parameters->needLOperand);

    gcoOS_ZeroMemory(&trueParameters, gcmSIZEOF(trueParameters));
    gcoOS_ZeroMemory(&falseParameters, gcmSIZEOF(falseParameters));

    if (Parameters->hint == clvEVALUATE_ONLY) return gcvSTATUS_OK;

    emptySelection = (Selection->trueOperand == gcvNULL && Selection->falseOperand == gcvNULL);

    if (emptySelection)
    {
        gcmASSERT(!Parameters->needROperand);

        /* Only generate the code of the condition expression */
        clsGEN_CODE_PARAMETERS_Initialize(&condParameters, gcvFALSE, gcvFALSE);

        status = cloIR_OBJECT_Accept(Compiler,
                         &Selection->condExpr->base,
                         &CodeGenerator->visitor,
                         &condParameters);
        if (gcmIS_ERROR(status)) return status;

        clsGEN_CODE_PARAMETERS_Finalize(&condParameters);

        return gcvSTATUS_OK;
    }

    if (Parameters->needROperand)
    {
        gcmASSERT(Selection->exprBase.decl.dataType);

        /* Allocate the operand(s) */
        status = clsGEN_CODE_PARAMETERS_AllocateOperands(Compiler,
                                 Parameters,
                                 &Selection->exprBase.decl);
        if (gcmIS_ERROR(status)) return status;

        gcmASSERT(Parameters->operandCount == 1);

                clmGEN_CODE_GetParametersIOperand(Compiler, iOperand, Parameters, Parameters->dataTypes[0].def);
        clsLOPERAND_InitializeUsingIOperand(lOperand, iOperand);
        clsROPERAND_InitializeUsingIOperand(&Parameters->rOperands[0], iOperand);
        intermIOperand = iOperand;
    }

    if (Selection->trueOperand && _IsCommonExprObject(Selection->trueOperand) &&
        Selection->falseOperand &&  _IsCommonExprObject(Selection->falseOperand) &&
        !clmDECL_IsScalar(&Selection->condExpr->decl)) {
           cloIR_POLYNARY_EXPR funcCall;
           clsGEN_CODE_PARAMETERS operandsParameters[3];

       /* Generate the code of the conditinal expression */
       clsGEN_CODE_PARAMETERS_Initialize(&operandsParameters[2],
                         gcvFALSE,
                         Parameters->needROperand);

       status = cloIR_OBJECT_Accept(Compiler,
                        &Selection->condExpr->base,
                        &CodeGenerator->visitor,
                        &operandsParameters[2]);
       if (gcmIS_ERROR(status)) return status;

       /* Generate the code of the left operand */
       clsGEN_CODE_PARAMETERS_Initialize(&operandsParameters[1],
                         gcvFALSE,
                         Parameters->needROperand);

       status = cloIR_OBJECT_Accept(Compiler,
                        Selection->trueOperand,
                        &CodeGenerator->visitor,
                        &operandsParameters[1]);
       if (gcmIS_ERROR(status)) return status;

       clsGEN_CODE_PARAMETERS_Initialize(&operandsParameters[0],
                         gcvFALSE,
                         Parameters->needROperand);

       status = cloIR_OBJECT_Accept(Compiler,
                        Selection->falseOperand,
                        &CodeGenerator->visitor,
                        &operandsParameters[0]);
       if (gcmIS_ERROR(status)) return status;

       if(clmDECL_IsScalar(&((cloIR_EXPR)Selection->trueOperand)->decl) &&
          clmDECL_IsScalar(&((cloIR_EXPR)Selection->falseOperand)->decl)) {
          gcmASSERT(clmDECL_IsVectorType(&Selection->condExpr->decl));
              status = _ConvScalarToVector(Compiler,
                                       clmDATA_TYPE_vectorSize_NOCHECK_GET(Selection->condExpr->decl.dataType),
                                           (cloIR_EXPR)Selection->trueOperand,
                                           &operandsParameters[1]);
          if(gcmIS_ERROR(status)) return status;
       }
       status = clGenImplicitConversion(Compiler,
                                            (cloIR_EXPR)Selection->trueOperand,
                                            (cloIR_EXPR)Selection->falseOperand,
                                            &operandsParameters[1],
                                            &operandsParameters[0]);
       if(gcmIS_ERROR(status)) return status;

           status = _ConvExprToFuncCall(Compiler,
                                        clvOPCODE_SELECT,
                                        &Selection->exprBase,
                                        &funcCall);
       if (gcmIS_ERROR(status)) return status;

           status = clGenBuiltinFunctionCode(Compiler,
                                             CodeGenerator,
                                             funcCall,
                                             3,
                                             operandsParameters,
                                             intermIOperand,
                         Parameters,
                         gcvTRUE);

       clsGEN_CODE_PARAMETERS_Finalize(&operandsParameters[0]);
       clsGEN_CODE_PARAMETERS_Finalize(&operandsParameters[1]);
       clsGEN_CODE_PARAMETERS_Finalize(&operandsParameters[2]);
       return status;
    }

    /* Selection Begin */
    status = clDefineSelectionBegin(Compiler,
                    CodeGenerator,
                    (Selection->falseOperand != gcvNULL),
                    &selectionContext);

    if (gcmIS_ERROR(status)) return status;


    /* Generate the code of the condition expression */
    status = _GenConditionCode(Compiler,
                   CodeGenerator,
                   Selection->condExpr,
                   clGetSelectionConditionLabel(&selectionContext),
                   gcvFALSE);
    if (gcmIS_ERROR(status)) return status;

    /* Generate the code of the true operand */
    status = clDefineSelectionTrueOperandBegin(Compiler,
                           CodeGenerator,
                           &selectionContext);

    if (gcmIS_ERROR(status)) return status;

    if (Selection->trueOperand != gcvNULL)
    {
        clsGEN_CODE_PARAMETERS_Initialize(&trueParameters, gcvFALSE, Parameters->needROperand);

        status = cloIR_OBJECT_Accept(Compiler,
                         Selection->trueOperand,
                         &CodeGenerator->visitor,
                         &trueParameters);
        if (gcmIS_ERROR(status)) return status;

        if (Parameters->needROperand)
        {
            status = clGenAssignCode(Compiler,
                        Selection->trueOperand->lineNo,
                        Selection->trueOperand->stringNo,
                        lOperand,
                        &trueParameters.rOperands[0]);
            if (gcmIS_ERROR(status)) return status;
        }
    }

    trueOperandHasReturn = (Selection->trueOperand != gcvNULL
                            && cloIR_BASE_HasReturn(Compiler, Selection->trueOperand));

    status = clDefineSelectionTrueOperandEnd(Compiler,
                        CodeGenerator,
                        &selectionContext,
                        trueOperandHasReturn);
    if (gcmIS_ERROR(status)) return status;

    /* Generate the code of the false operand */
    if (Selection->falseOperand != gcvNULL)
    {
        status = clDefineSelectionFalseOperandBegin(Compiler,
                                CodeGenerator,
                                &selectionContext);
        if (gcmIS_ERROR(status)) return status;

        clsGEN_CODE_PARAMETERS_Initialize(&falseParameters, gcvFALSE, Parameters->needROperand);

        status = cloIR_OBJECT_Accept(Compiler,
                         Selection->falseOperand,
                         &CodeGenerator->visitor,
                         &falseParameters);
        if (gcmIS_ERROR(status)) return status;

        if (Parameters->needROperand)
        {
            status = clGenAssignCode(Compiler,
                         Selection->falseOperand->lineNo,
                         Selection->falseOperand->stringNo,
                         lOperand,
                         &falseParameters.rOperands[0]);
            if (gcmIS_ERROR(status)) return status;
        }

        status = clDefineSelectionFalseOperandEnd(Compiler,
                            CodeGenerator,
                            &selectionContext);
        if (gcmIS_ERROR(status)) return status;
    }

    /* Selection End */
    status = clDefineSelectionEnd(Compiler,
                      CodeGenerator,
                      &selectionContext);
    if (gcmIS_ERROR(status)) return status;

    if (Selection->trueOperand != gcvNULL)    clsGEN_CODE_PARAMETERS_Finalize(&trueParameters);
    if (Selection->falseOperand != gcvNULL)    clsGEN_CODE_PARAMETERS_Finalize(&falseParameters);

    return gcvSTATUS_OK;
}

gceSTATUS
cloIR_SWITCH_GenCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_SWITCH Selection,
    IN OUT clsGEN_CODE_PARAMETERS * Parameters
    )
{
    gceSTATUS  status = gcvSTATUS_OK;
    cloIR_LABEL cases;
    clsITERATION_CONTEXT switchContext[1];
    clsGEN_CODE_PARAMETERS    condParameters, switchBodyParameters;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_OBJECT(CodeGenerator, clvOBJ_CODE_GENERATOR);
    clmVERIFY_IR_OBJECT(Selection, clvIR_SWITCH);
    gcmASSERT(Selection->condExpr);
    gcmASSERT(Parameters);
    gcmASSERT(!Parameters->needLOperand);

    gcoOS_ZeroMemory(&switchBodyParameters, gcmSIZEOF(switchBodyParameters));

    if (Parameters->hint == clvEVALUATE_ONLY) return gcvSTATUS_OK;

    clsGEN_CODE_PARAMETERS_Initialize(&condParameters, gcvFALSE, gcvTRUE);

    status = cloIR_OBJECT_Accept(Compiler,
                     &Selection->condExpr->base,
                     &CodeGenerator->visitor,
                     &condParameters);
    if (gcmIS_ERROR(status)) return status;

    gcmASSERT(condParameters.operandCount == 1);

    if(Selection->switchBody == gcvNULL) {
        gcmASSERT(!Parameters->needROperand);

        clsGEN_CODE_PARAMETERS_Finalize(&condParameters);
        return gcvSTATUS_OK;
    }

    clsGEN_CODE_PARAMETERS_Initialize(&switchBodyParameters, gcvFALSE, gcvFALSE);

    cases = Selection->cases;

    status = _CloneIterationContextForSwitch(Compiler, CodeGenerator, switchContext);
    if (gcmIS_ERROR(status)) return status;

        while(cases) {
       cases->programCounter = clNewLabel(Compiler);
    /* Generate the code of the condition expression */
       switch(cases->type) {
       case clvCASE:
          gcmONERROR(_GenSplitOperandConditionCode(Compiler,
                                                       CodeGenerator,
                                                       clvCONDITION_EQUAL,
                                                       &condParameters,
                                                       Selection->condExpr,
                                                       &cases->caseValue->exprBase,
                                                       cases->programCounter,
                                                       gcvTRUE));
          break;

       case clvDEFAULT:
          gcmONERROR(clEmitAlwaysBranchCode(Compiler,
                            0,
                            0,
                            clvOPCODE_JUMP,
                            cases->programCounter));
          gcmASSERT(!cases->u.nextCase);
          break;

       default:
          gcmASSERT(0);
          break;
       }
       cases = cases->u.nextCase;
    }

    gcmONERROR(cloIR_OBJECT_Accept(Compiler,
                       Selection->switchBody,
                       &CodeGenerator->visitor,
                       &switchBodyParameters));


    gcmONERROR(_DefineSwitchEnd(Compiler, CodeGenerator));

OnError:
    clsGEN_CODE_PARAMETERS_Finalize(&switchBodyParameters);
    clsGEN_CODE_PARAMETERS_Finalize(&condParameters);
    return status;
}

gceSTATUS
cloIR_POLYNARY_EXPR_GenOperandsCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    OUT gctUINT * OperandCount,
    OUT clsGEN_CODE_PARAMETERS * * OperandsParameters
    )
{
    gceSTATUS status;
    gctUINT    operandCount;
    clsGEN_CODE_PARAMETERS *operandsParameters;
    cloIR_EXPR operand;
    gctUINT    i = 0;
    cleGEN_CODE_HINT hint;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_OBJECT(CodeGenerator, clvOBJ_CODE_GENERATOR);
    clmVERIFY_IR_OBJECT(PolynaryExpr, clvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount);
    gcmASSERT(OperandsParameters);

    if (PolynaryExpr->operands == gcvNULL) {
        *OperandCount        = 0;
        *OperandsParameters    = gcvNULL;

        return gcvSTATUS_OK;
    }

    do {
        gcmVERIFY_OK(cloIR_SET_GetMemberCount(Compiler,
                              PolynaryExpr->operands,
                              &operandCount));

        gcmASSERT(operandCount > 0);

        status = cloCOMPILER_Allocate(Compiler,
                          (gctSIZE_T)sizeof(clsGEN_CODE_PARAMETERS) * operandCount,
                          (gctPOINTER *) &operandsParameters);

        if (gcmIS_ERROR(status)) break;

        if(PolynaryExpr->type == clvPOLYNARY_FUNC_CALL &&
           PolynaryExpr->funcName->isBuiltin) {
            hint = clvGEN_ADDRESS_OFFSET;
            if(PolynaryExpr->funcName->u.funcInfo.hasVarArg) {
                hint |= clvGEN_SAVE_ADDRESS_OFFSET;
            }
        }
        else hint = clvGEN_NO_HINT;

        FOR_EACH_DLINK_NODE(&PolynaryExpr->operands->members, struct _cloIR_EXPR, operand)
        {
            clsGEN_CODE_PARAMETERS_Initialize(&operandsParameters[i], gcvFALSE, gcvTRUE);

            if(clmDECL_IsPointerType(&operand->decl)) {
                operandsParameters[i].hint = hint;
            }

            status = cloIR_OBJECT_Accept(Compiler,
                                         &operand->base,
                                         &CodeGenerator->visitor,
                                         &operandsParameters[i]);
            if (gcmIS_ERROR(status)) break;

            i++;
        }

        *OperandCount        = operandCount;
        *OperandsParameters    = operandsParameters;

        return gcvSTATUS_OK;
    }
    while (gcvFALSE);

    *OperandCount        = 0;
    *OperandsParameters    = gcvNULL;

    return status;
}

gceSTATUS
cloIR_POLYNARY_EXPR_GenOperandsCodeForFuncCall(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    OUT gctUINT * OperandCount,
    OUT clsGEN_CODE_PARAMETERS * * OperandsParameters
    )
{
    gceSTATUS status;
    gctUINT    operandCount;
    clsGEN_CODE_PARAMETERS *operandsParameters;
    cloIR_EXPR operand;
    gctUINT    i = 0;
    clsNAME *paramName;
    gctBOOL    needLOperand, needROperand;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_OBJECT(CodeGenerator, clvOBJ_CODE_GENERATOR);
    clmVERIFY_IR_OBJECT(PolynaryExpr, clvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount);
    gcmASSERT(OperandsParameters);

    if (PolynaryExpr->operands == gcvNULL)
    {
        *OperandCount        = 0;
        *OperandsParameters    = gcvNULL;

        return gcvSTATUS_OK;
    }

    do
    {
        gcmVERIFY_OK(cloIR_SET_GetMemberCount(Compiler,
                            PolynaryExpr->operands,
                            &operandCount));

        gcmASSERT(operandCount > 0);

        status = cloCOMPILER_Allocate(Compiler,
                          (gctSIZE_T)sizeof(clsGEN_CODE_PARAMETERS) * operandCount,
                          (gctPOINTER *) &operandsParameters);

        if (gcmIS_ERROR(status)) break;

        for (paramName =
            slsDLINK_LIST_First(&PolynaryExpr->funcName->u.funcInfo.localSpace->names, clsNAME),
             operand = slsDLINK_LIST_First(&PolynaryExpr->operands->members, struct _cloIR_EXPR);
            (slsDLINK_NODE *)paramName != &PolynaryExpr->funcName->u.funcInfo.localSpace->names;
            paramName = slsDLINK_NODE_Next(&paramName->node, clsNAME),
            operand = slsDLINK_NODE_Next(&operand->base.node, struct _cloIR_EXPR))
        {
            if (paramName->type != clvPARAMETER_NAME) break;

            gcmASSERT((slsDLINK_NODE *)operand != &PolynaryExpr->operands->members);

            needLOperand = gcvFALSE;
            needROperand = gcvTRUE;

            clsGEN_CODE_PARAMETERS_Initialize(&operandsParameters[i], needLOperand, needROperand);

            if(clmDECL_IsAggregateType(&paramName->decl)) {
                           operandsParameters[i].hint |= clvGEN_ADDR_CODE;
            }
            status = cloIR_OBJECT_Accept(Compiler,
                             &operand->base,
                             &CodeGenerator->visitor,
                             &operandsParameters[i]);

            if (gcmIS_ERROR(status)) break;

            i++;
        }

        *OperandCount        = operandCount;
        *OperandsParameters    = operandsParameters;

        return gcvSTATUS_OK;
    }
    while (gcvFALSE);

    *OperandCount        = 0;
    *OperandsParameters    = gcvNULL;

    return status;
}

clsCOMPONENT_SELECTION
clGetComponentSelectionSlice(
    IN clsCOMPONENT_SELECTION ComponentSelection,
    IN gctUINT8 StartComponent,
    IN gctUINT8 SliceComponentCount
    )
{
    clsCOMPONENT_SELECTION resultComponentSelection = { 0 };

    gcmASSERT((StartComponent + SliceComponentCount) <= ComponentSelection.components);

    if(StartComponent < cldMaxComponentCount) {
      gctUINT8 i, j;

      resultComponentSelection.components = SliceComponentCount;

      for(i = clvCOMPONENT_X, j = StartComponent; i < SliceComponentCount; i++, j++) {
         resultComponentSelection.selection[i] = ComponentSelection.selection[j];
      }
    }
    else {
      gcmASSERT(0);
    }

    return resultComponentSelection;
}

#define _clmGetComponentSelectionSlice(DataType, ComponentSelection, StartComponent, SlicecomponentCount) \
    (clmGEN_CODE_IsHighPrecisionDataType((DataType)) \
        ? clGetComponentSelectionSlice((ComponentSelection), (StartComponent) << 1, (SliceComponentCount) << 1) \
        : clGetComponentSelectionSlice((ComponentSelection), (StartComponent), (SliceComponentCount)))

static void
_GetOperandConstantSlice(
    IN clsOPERAND_CONSTANT * OperandConstant,
    IN gctUINT8 StartComponent,
    IN gctUINT8 SliceComponentCount,
    IN clsGEN_CODE_DATA_TYPE SliceDataType,
    OUT clsOPERAND_CONSTANT * ResultOperandConstant
    )
{
    gctUINT        i;

    /* Verify the arguments. */
    gcmASSERT(OperandConstant);
    gcmASSERT(ResultOperandConstant);

    ResultOperandConstant->dataType        = SliceDataType;
    ResultOperandConstant->valueCount    = SliceComponentCount;

    if(clmGEN_CODE_IsScalarDataType(OperandConstant->dataType)) {
        for (i = 0; i < SliceComponentCount; i++)
        {
            ResultOperandConstant->values[i] = OperandConstant->values[0];
        }
    }
    else {
        gcmASSERT((StartComponent + SliceComponentCount) <= (gctUINT8)OperandConstant->valueCount);
        for (i = 0; i < SliceComponentCount; i++)
        {
            ResultOperandConstant->values[i] = OperandConstant->values[StartComponent + i];
        }
    }
}

void
clGetVectorROperandSlice(
    IN clsROPERAND * ROperand,
    IN gctUINT8 StartComponent,
    IN gctUINT8 SliceComponentCount,
    OUT clsROPERAND * ROperandSlice
    )
{
    clsGEN_CODE_DATA_TYPE sliceDataType;

    sliceDataType = gcGetVectorSliceDataType(ROperand->dataType, SliceComponentCount);

    *ROperandSlice = *ROperand;
    ROperandSlice->dataType = sliceDataType;

    if (SliceComponentCount == 1) {
        gcmASSERT(ROperandSlice->vectorIndex.mode == clvINDEX_NONE);

        ROperandSlice->vectorIndex.mode = clvINDEX_CONSTANT;
        ROperandSlice->vectorIndex.u.constant = (gctREG_INDEX)StartComponent;
    }
    else {
        if (ROperandSlice->isReg) {
            ROperandSlice->u.reg.componentSelection = _clmGetComponentSelectionSlice(ROperand->dataType,
                                                                                     ROperand->u.reg.componentSelection,
                                                                                     StartComponent,
                                                                                     SliceComponentCount);
        }
        else {
            _GetOperandConstantSlice(&ROperand->u.constant,
                                     StartComponent,
                                     SliceComponentCount,
                                     sliceDataType,
                                     &ROperandSlice->u.constant);
        }
    }
}

void
clGetVectorIOperandSlice(
    IN clsIOPERAND * IOperand,
    IN gctUINT8 StartComponent,
    IN gctUINT8 SliceComponentCount,
    OUT clsIOPERAND * IOperandSlice
    )
{
    clsGEN_CODE_DATA_TYPE sliceDataType;

    sliceDataType = gcGetVectorSliceDataType(IOperand->dataType, SliceComponentCount);

    *IOperandSlice = *IOperand;
    IOperandSlice->dataType = sliceDataType;

    /* IOperand associated with a register is assumed */
    IOperandSlice->componentSelection = _clmGetComponentSelectionSlice(IOperand->dataType,
                                                                       IOperand->componentSelection,
                                                                       StartComponent,
                                                                       SliceComponentCount);
}

static void
_GetVectorROperandSlice(
    IN clsROPERAND * ROperand,
    IN gctUINT8 StartComponent,
    IN gctUINT8 RequiredComponentCount,
    OUT clsROPERAND * ROperandSlice,
    OUT gctUINT8 * SliceComponentCount
    )
{
    gctUINT8 sliceComponentCount;

    /* Verify the arguments. */
    gcmASSERT(ROperand);
    gcmASSERT(gcIsVectorDataType(ROperand->dataType));
    gcmASSERT(ROperandSlice);
    gcmASSERT(SliceComponentCount);

    sliceComponentCount = gcGetVectorDataTypeComponentCount(ROperand->dataType) - StartComponent;

    if (sliceComponentCount > RequiredComponentCount) {
        sliceComponentCount = RequiredComponentCount;
    }

    clGetVectorROperandSlice(ROperand,
                             StartComponent,
                             sliceComponentCount,
                             ROperandSlice);

    *SliceComponentCount = sliceComponentCount;
}

static gctBOOL
_GetROperandSlice(
    IN clsROPERAND * ROperand,
    IN OUT gctUINT8 * StartComponent,
    IN OUT gctUINT8 * RequiredComponentCount,
    OUT clsROPERAND * ROperandSlice,
    OUT gctUINT8 * SliceComponentCount
    )
{
    gctUINT8        componentCount, sliceComponentCount;
    gctUINT            matrixColumnCount, matrixIndex;
    clsROPERAND        matrixColumnROperand;

    /* Verify the arguments. */
    gcmASSERT(ROperand);
    gcmASSERT(StartComponent);
    gcmASSERT(RequiredComponentCount);
    gcmASSERT(*RequiredComponentCount > 0);
    gcmASSERT(ROperandSlice);

    if (gcIsScalarDataType(ROperand->dataType))
    {
        if (*StartComponent > 0) return gcvFALSE;

        *ROperandSlice = *ROperand;

        sliceComponentCount = 1;
    }
    else if (gcIsVectorDataType(ROperand->dataType))
    {
        componentCount = gcGetVectorDataTypeComponentCount(ROperand->dataType);

        if (*StartComponent > componentCount - 1) return gcvFALSE;

        _GetVectorROperandSlice(ROperand,
                    *StartComponent,
                    *RequiredComponentCount,
                    ROperandSlice,
                    &sliceComponentCount);
    }
    else
    {
        gcmASSERT(gcIsMatrixDataType(ROperand->dataType));

        matrixColumnCount = gcGetMatrixDataTypeColumnCount(ROperand->dataType);

        if (*StartComponent > matrixColumnCount * matrixColumnCount - 1) return gcvFALSE;

        matrixIndex = *StartComponent / matrixColumnCount;

        clsROPERAND_InitializeAsMatrixColumn(&matrixColumnROperand,
                        ROperand,
                        matrixIndex);

        _GetVectorROperandSlice(&matrixColumnROperand,
                    *StartComponent - (gctUINT8) (matrixIndex * matrixColumnCount),
                    *RequiredComponentCount,
                    ROperandSlice,
                    &sliceComponentCount);
    }

    (*StartComponent)        += sliceComponentCount;
    (*RequiredComponentCount)    -= sliceComponentCount;

    if (SliceComponentCount != gcvNULL) *SliceComponentCount = sliceComponentCount;

    return gcvTRUE;
}

void
clGetVectorLOperandSlice(
IN clsLOPERAND * LOperand,
IN gctUINT8 StartComponent,
IN gctUINT8 SliceComponentCount,
OUT clsLOPERAND * LOperandSlice
)
{
   clsGEN_CODE_DATA_TYPE sliceDataType;

/* Verify the arguments. */
   gcmASSERT(LOperand);
   gcmASSERT(gcIsVectorDataType(LOperand->dataType));
   gcmASSERT(SliceComponentCount > 0);
   gcmASSERT(LOperandSlice);

   sliceDataType = gcGetVectorSliceDataType(LOperand->dataType, SliceComponentCount);

   *LOperandSlice = *LOperand;
   LOperandSlice->dataType = sliceDataType;

   if (SliceComponentCount == 1) {
      gcmASSERT(LOperandSlice->vectorIndex.mode == clvINDEX_NONE);

      LOperandSlice->vectorIndex.mode = clvINDEX_CONSTANT;
      LOperandSlice->vectorIndex.u.constant = (gctREG_INDEX)StartComponent;
   }
   else {
      LOperandSlice->reg.componentSelection = _clmGetComponentSelectionSlice(LOperand->dataType,
                                                                             LOperand->reg.componentSelection,
                                                                             StartComponent,
                                                                             SliceComponentCount);
   }
}

gceSTATUS
cloIR_POLYNARY_EXPR_GenConstructScalarCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN OUT clsGEN_CODE_PARAMETERS * Parameters
    )
{
    gceSTATUS status;
    gctUINT    operandCount;
    clsGEN_CODE_PARAMETERS *operandsParameters;
    gctBOOL treatFloatAsInt;
    gctUINT8 startComponent = 0;
    gctUINT8 requiredComponentCount = 1;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_OBJECT(CodeGenerator, clvOBJ_CODE_GENERATOR);
    clmVERIFY_IR_OBJECT(PolynaryExpr, clvIR_POLYNARY_EXPR);
    gcmASSERT(Parameters);
    gcmASSERT(!Parameters->needLOperand);

    status = cloIR_POLYNARY_EXPR_GenOperandsCode(Compiler,
                             CodeGenerator,
                             PolynaryExpr,
                             &operandCount,
                             &operandsParameters);
    if (gcmIS_ERROR(status)) return status;
    gcmASSERT(operandCount == 1);

    treatFloatAsInt = (gctBOOL) (Parameters->hint & clvGEN_INDEX_CODE);

    if (Parameters->needROperand)
    {
        status = clsGEN_CODE_PARAMETERS_AllocateOperands(Compiler,
                                Parameters,
                                &PolynaryExpr->exprBase.decl);

        if (gcmIS_ERROR(status)) return status;

        gcmASSERT(Parameters->operandCount == 1);

        _GetROperandSlice(&operandsParameters[0].rOperands[0],
                  &startComponent,
                  &requiredComponentCount,
                  &Parameters->rOperands[0],
                  gcvNULL);

        status = clsROPERAND_ChangeDataTypeFamily(Compiler,
                              PolynaryExpr->exprBase.base.lineNo,
                              PolynaryExpr->exprBase.base.stringNo,
                              treatFloatAsInt,
                              Parameters->dataTypes[0].def,
                              &Parameters->rOperands[0]);
        if (gcmIS_ERROR(status)) return status;
    }

    gcmVERIFY_OK(cloIR_POLYNARY_EXPR_FinalizeOperandsParameters(Compiler,
                                    operandCount,
                                    operandsParameters));
    return gcvSTATUS_OK;
}

gceSTATUS
cloIR_POLYNARY_EXPR_GenConstructArrayCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN OUT clsGEN_CODE_PARAMETERS * Parameters
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gctUINT    operandCount;
    clsGEN_CODE_PARAMETERS *operandsParameters;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_OBJECT(CodeGenerator, clvOBJ_CODE_GENERATOR);
    clmVERIFY_IR_OBJECT(PolynaryExpr, clvIR_POLYNARY_EXPR);
    gcmASSERT(Parameters);
    gcmASSERT(!Parameters->needLOperand);

    status = cloIR_POLYNARY_EXPR_GenOperandsCode(Compiler,
                             CodeGenerator,
                             PolynaryExpr,
                             &operandCount,
                             &operandsParameters);
    if (gcmIS_ERROR(status)) return status;

    if (Parameters->needROperand)
    {
        gctUINT i, j, k;

        /* Allocate the operand(s) */
        gcmONERROR(clsGEN_CODE_PARAMETERS_AllocateOperands(Compiler,
                                   Parameters,
                                   &PolynaryExpr->exprBase.decl));

        k = 0;
        for(i = 0; i < operandCount; i++) {
           for(j = 0; j < operandsParameters[i].operandCount; j++) {
            Parameters->rOperands[k++] = operandsParameters[i].rOperands[j];
           }
        }
        gcmASSERT(Parameters->operandCount == k);
    }

OnError:
    gcmVERIFY_OK(cloIR_POLYNARY_EXPR_FinalizeOperandsParameters(Compiler,
                                    operandCount,
                                    operandsParameters));
    return status;
}

typedef struct _clsOPERANDS_LOCATION
{
    gctUINT    currentOperand;

    gctUINT8 startComponent;
}
clsOPERANDS_LOCATION;

#define clsOPERANDS_LOCATION_Initialize(location) \
    do \
    { \
        (location)->currentOperand    = 0; \
        (location)->startComponent    = 0; \
    } \
    while (gcvFALSE)

gceSTATUS
cloIR_POLYNARY_EXPR_GenVectorComponentAssignCode(
    IN cloCOMPILER Compiler,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand,
    IN OUT clsOPERANDS_LOCATION * Location
    )
{
    gceSTATUS    status;
    gctUINT8    lOperandStartComponent = 0;
    gctUINT8    requiredComponentCount;
    clsROPERAND    rOperandSlice;
    gctUINT8    sliceComponentCount;
    clsLOPERAND    lOperand, lOperandSlice;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(PolynaryExpr, clvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount > 0);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);
    gcmASSERT(gcIsVectorDataType(IOperand->dataType));
    gcmASSERT(Location);

    requiredComponentCount = gcGetVectorDataTypeComponentCount(IOperand->dataType);

    clsLOPERAND_InitializeUsingIOperand(&lOperand, IOperand);

    while (requiredComponentCount > 0)
    {
        gcmASSERT(Location->currentOperand < OperandCount);
        gcmASSERT(OperandsParameters[Location->currentOperand].operandCount == 1);

        if (!_GetROperandSlice(&OperandsParameters[Location->currentOperand].rOperands[0],
                    &Location->startComponent,
                    &requiredComponentCount,
                    &rOperandSlice,
                    &sliceComponentCount))
        {
            Location->currentOperand++;
            Location->startComponent = 0;
            continue;
        }

        clGetVectorLOperandSlice(&lOperand,
                    lOperandStartComponent,
                    sliceComponentCount,
                    &lOperandSlice);

        lOperandStartComponent += sliceComponentCount;

        status = clsROPERAND_ChangeDataTypeFamily(Compiler,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            gcvFALSE,
                            lOperandSlice.dataType,
                            &rOperandSlice);

        if (gcmIS_ERROR(status)) return status;

        status = clGenAssignCode(Compiler,
                    PolynaryExpr->exprBase.base.lineNo,
                    PolynaryExpr->exprBase.base.stringNo,
                    &lOperandSlice,
                    &rOperandSlice);

        if (gcmIS_ERROR(status)) return status;
    }

    return gcvSTATUS_OK;
}

gceSTATUS
cloIR_POLYNARY_EXPR_GenMatrixComponentAssignCode(
    IN cloCOMPILER Compiler,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
    gceSTATUS    status;
    gctUINT        matrixColumnCount, i;
    clsOPERANDS_LOCATION    location;
    clsIOPERAND    matrixColumnIOperand[1];

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(PolynaryExpr, clvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount > 0);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);
    gcmASSERT(gcIsMatrixDataType(IOperand->dataType));

    matrixColumnCount = gcGetMatrixDataTypeColumnCount(IOperand->dataType);

    clsOPERANDS_LOCATION_Initialize(&location);

    for (i = 0; i < matrixColumnCount; i++)
    {
        clsIOPERAND_InitializeAsMatrixColumn(matrixColumnIOperand, IOperand, i);

        status = cloIR_POLYNARY_EXPR_GenVectorComponentAssignCode(Compiler,
                                      PolynaryExpr,
                                      OperandCount,
                                      OperandsParameters,
                                      matrixColumnIOperand,
                                      &location);
        if (gcmIS_ERROR(status)) return status;
    }

    return gcvSTATUS_OK;
}

static gceSTATUS
_GenScalarToVectorAssignCode(
    IN cloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN clsROPERAND * ScalarROperand,
    IN clsIOPERAND * VectorIOperand,
    OUT clsROPERAND * VectorROperand
    )
{
    gceSTATUS    status;
    clsROPERAND    scalarROperand;
    clsLOPERAND    vectorLOperand;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    gcmASSERT(ScalarROperand);
    gcmASSERT(VectorIOperand);
    gcmASSERT(VectorROperand);

    scalarROperand = *ScalarROperand;

    status = clsROPERAND_ChangeDataTypeFamily(Compiler,
                        LineNo,
                        StringNo,
                        gcvFALSE,
                        gcGetVectorComponentDataType(VectorIOperand->dataType),
                        &scalarROperand);

    if (gcmIS_ERROR(status)) return status;

    clsLOPERAND_InitializeTempReg(&vectorLOperand,
                    clvQUALIFIER_NONE,
                    scalarROperand.dataType,
                    VectorIOperand->tempRegIndex);

    status = clGenAssignCode(Compiler,
                LineNo,
                StringNo,
                &vectorLOperand,
                &scalarROperand);

    if (gcmIS_ERROR(status)) return status;

    clsROPERAND_InitializeUsingIOperand(VectorROperand, VectorIOperand);

    return _CompleteComponentSelection(&VectorROperand->u.reg.componentSelection, 1);
}

static gceSTATUS
_AssignIOperandWithTypeConversion(
IN cloCOMPILER Compiler,
IN cloCODE_GENERATOR CodeGenerator,
IN gctUINT LineNo,
IN gctUINT StringNo,
IN clsIOPERAND *IOperand,
IN gctUINT8 ComponentIx,
IN clsROPERAND *ROperand
)
{
   cleOPCODE  opcode;
   clsIOPERAND resIOperand[1];
   cltELEMENT_TYPE elementType;
   cltELEMENT_TYPE resElementType;

   gcmASSERT(IOperand);
   gcmASSERT(ROperand);

   opcode = clvOPCODE_ASSIGN;
   resElementType = clmGEN_CODE_elementType_GET(IOperand->dataType);
   elementType = clmGEN_CODE_elementType_GET(ROperand->dataType);
   if (!ROperand->isReg) { /*is a constant */
      clsGEN_CODE_DATA_TYPE resType;

      clmGEN_CODE_DATA_TYPE_Initialize(resType,
                                       clmGEN_CODE_matrixRowCount_GET(ROperand->dataType),
                                       clmGEN_CODE_matrixColumnCount_GET(ROperand->dataType),
                                       resElementType);

      if(clmIsElementTypeBoolean(elementType)) {
         clsOPERAND_CONSTANT_ChangeBooleanFamilyDataType(resType, &ROperand->u.constant);
      }
      else if(clmIsElementTypeFloating(elementType)) {
         clsOPERAND_CONSTANT_ChangeFloatFamilyDataType(resType, &ROperand->u.constant);
      }
      else if(clmIsElementTypeInteger(elementType)) {
         if(clmIsElementTypeUnsigned(elementType)) {
            clsOPERAND_CONSTANT_ChangeUnsignedIntegerFamilyDataType(resType, &ROperand->u.constant);
        }
        else {
            clsOPERAND_CONSTANT_ChangeIntegerFamilyDataType(resType, &ROperand->u.constant);
        }
      }
      else {
         gcmASSERT(0);
         return gcvSTATUS_INVALID_DATA;
      }
   }
   else {
      if(resElementType != elementType) {
        if(clmIsElementTypeFloating(resElementType)) {
           if(clmIsElementTypeBoolean(elementType)) {
              if(CodeGenerator->fpConfig & cldFpROUND_TO_NEAREST) {
                 opcode = clvOPCODE_IMPL_B2F;
              }
              else {
                 opcode = clvOPCODE_BOOL_TO_FLOAT;
              }
           }
           else if(clmIsElementTypeInteger(elementType)) {
              if(clmIsElementTypeUnsigned(elementType)) {
                 if(CodeGenerator->fpConfig & cldFpROUND_TO_NEAREST) {
                    opcode = clvOPCODE_IMPL_U2F;
                 }
                 else {
                    opcode = clvOPCODE_UINT_TO_FLOAT;
                 }
              }
              else {
                 if(CodeGenerator->fpConfig & cldFpROUND_TO_NEAREST) {
                    opcode = clvOPCODE_IMPL_I2F;
                 }
                 else {
                    opcode = clvOPCODE_INT_TO_FLOAT;
                 }
              }
           }
        }
        else if(clmIsElementTypeBoolean(resElementType)) {
           if(clmIsElementTypeFloating(elementType)) {
              opcode = clvOPCODE_FLOAT_TO_BOOL;
           }
           else if(clmIsElementTypeInteger(elementType)) {
              if(clmIsElementTypeUnsigned(elementType)) {
                 opcode = clvOPCODE_UINT_TO_BOOL;
              }
              else {
                 opcode = clvOPCODE_INT_TO_BOOL;
              }
           }
        }
        else if(clmIsElementTypeInteger(resElementType)) {
          if(clmIsElementTypeUnsigned(resElementType)) {
            if(clmIsElementTypeBoolean(elementType)) {
                opcode = clvOPCODE_BOOL_TO_UINT;
             }
            else if(clmIsElementTypeFloating(elementType)) {
                opcode = clvOPCODE_FLOAT_TO_UINT;
            }
             else if(clmIsElementTypeUnsigned(elementType)) {
                opcode = clvOPCODE_CONV;
            }
            else opcode = clvOPCODE_CONV;
          }
         else {
            if(clmIsElementTypeBoolean(elementType)) {
               opcode = clvOPCODE_BOOL_TO_INT;
             }
             else if(clmIsElementTypeFloating(elementType)) {
               opcode = clvOPCODE_FLOAT_TO_INT;
            }
             else if(clmIsElementTypeUnsigned(elementType)) {
               opcode = clvOPCODE_CONV;
             }
             else opcode = clvOPCODE_CONV;
          }
        }
      }
   }

   clGetVectorIOperandSlice(IOperand,
                            ComponentIx,
                            gcGetDataTypeComponentCount(ROperand->dataType),
                            resIOperand);

   return clGenGenericCode1(Compiler,
                            LineNo,
                            StringNo,
                            opcode,
                            resIOperand,
                            ROperand);
}

static gceSTATUS
_ReuseCreatedVector(
IN cloCOMPILER Compiler,
IN cloCODE_GENERATOR CodeGenerator,
IN clsGEN_CODE_PARAMETERS *Parameters,
IN OUT clsIOPERAND *IOperand
)
{
   if(Parameters->hasIOperand ||
      CodeGenerator->currentVector.tempRegIndex == 0 ||
      !gcIsDataTypeEqual(CodeGenerator->currentVector.dataType,
                        Parameters->dataTypes[0].def)) {
      gcoOS_ZeroMemory(&CodeGenerator->currentVector, gcmSIZEOF(clsVECTOR_CREATION));

      clmGEN_CODE_GetParametersIOperand(Compiler, IOperand, Parameters, Parameters->dataTypes[0].def);
      CodeGenerator->currentVector.dataType = IOperand->regDataType;
      CodeGenerator->currentVector.tempRegIndex = IOperand->tempRegIndex;
   }
   else {
      gcmASSERT(CodeGenerator->currentVector.tempRegIndex);

      clsIOPERAND_Initialize(IOperand, Parameters->dataTypes[0].def, CodeGenerator->currentVector.tempRegIndex);
   }
   return gcvSTATUS_OK;
}

gceSTATUS
cloIR_POLYNARY_EXPR_GenConstructVectorCode(
IN cloCOMPILER Compiler,
IN cloCODE_GENERATOR CodeGenerator,
IN cloIR_POLYNARY_EXPR PolynaryExpr,
IN OUT clsGEN_CODE_PARAMETERS * Parameters
)
{
    gceSTATUS    status;
    gctUINT      operandCount = 0;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_OBJECT(CodeGenerator, clvOBJ_CODE_GENERATOR);
    clmVERIFY_IR_OBJECT(PolynaryExpr, clvIR_POLYNARY_EXPR);
    gcmASSERT(Parameters);
    gcmASSERT(!Parameters->needLOperand);

    if (Parameters->needROperand) {
        clsGEN_CODE_PARAMETERS  operandParameters[1];
        clsIOPERAND iOperand[1];
        cloIR_EXPR operand;
        gctUINT8 componentIx;
        gctBOOL sameElementType;

        /* Allocate the operand(s) */
        status = clsGEN_CODE_PARAMETERS_AllocateOperands(Compiler,
                                                         Parameters,
                                                         &PolynaryExpr->exprBase.decl);
        if (gcmIS_ERROR(status)) return status;

        gcmASSERT(Parameters->operandCount == 1);
        _ReuseCreatedVector(Compiler,
                            CodeGenerator,
                            Parameters,
                            iOperand);
        clsROPERAND_InitializeUsingIOperand(&Parameters->rOperands[0], iOperand);
        componentIx = iOperand->componentSelection.selection[0];

        operandCount = 0;
        FOR_EACH_DLINK_NODE(&PolynaryExpr->operands->members, struct _cloIR_EXPR, operand)
        {
            if(operandCount) {
               clsGEN_CODE_PARAMETERS_Finalize(operandParameters);
            }
            clsGEN_CODE_PARAMETERS_Initialize(operandParameters, gcvFALSE, gcvTRUE);

            sameElementType = (clmDATA_TYPE_elementType_GET(PolynaryExpr->exprBase.decl.dataType) ==
                               clmDATA_TYPE_elementType_GET(operand->decl.dataType));
            if(sameElementType) {
               clmGEN_CODE_SetParametersIOperand(Compiler, operandParameters, 0, iOperand, componentIx);
            }

            status = cloIR_OBJECT_Accept(Compiler,
                                         &operand->base,
                                         &CodeGenerator->visitor,
                                         operandParameters);
            if (gcmIS_ERROR(status)) return status;

            if(!sameElementType) {
               status = _AssignIOperandWithTypeConversion(Compiler,
                                                          CodeGenerator,
                                                          PolynaryExpr->exprBase.base.lineNo,
                                                          PolynaryExpr->exprBase.base.stringNo,
                                                          iOperand,
                                                          componentIx,
                                                          operandParameters->rOperands);
               if (gcmIS_ERROR(status)) {
                   gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                                   PolynaryExpr->exprBase.base.lineNo,
                                                   PolynaryExpr->exprBase.base.stringNo,
                                                   clvREPORT_ERROR,
                                                   "internal error: failed to do data type conversion"));
                   return status;
               }
            }
            componentIx += gcGetDataTypeComponentCount(operandParameters->dataTypes[0].def);
            operandCount++;
        }

        if (operandCount == 1 && operandParameters[0].operandCount == 1
            && gcIsScalarDataType(operandParameters[0].dataTypes[0].def)) {
            if (!operandParameters[0].rOperands[0].isReg) {

                /* This should not happen as it should have been covered by
                cloIR_POLYNARY_EXPR_TryToEvaluate() in cloIR_POLYNARY_EXPR_GenCode() */
                gcmASSERT(0);
                /* Convert the scalar constant to the vector constant */
                Parameters->rOperands[0] = operandParameters[0].rOperands[0];

                clsROPERAND_CONSTANT_ConvScalarToVector(Compiler,
                                                        Parameters->dataTypes[0].def,
                                                        &Parameters->rOperands[0]);
            }
            else {
                /* Generate the special assignment */
                status = _GenScalarToVectorAssignCode(Compiler,
                                                      PolynaryExpr->exprBase.base.lineNo,
                                                      PolynaryExpr->exprBase.base.stringNo,
                                                      &operandParameters[0].rOperands[0],
                                                      iOperand,
                                                      &Parameters->rOperands[0]);
                if (gcmIS_ERROR(status)) return status;
            }
        }
        clsGEN_CODE_PARAMETERS_Finalize(operandParameters);
    }
    else {
        clsGEN_CODE_PARAMETERS  *operandsParameters;
        status = cloIR_POLYNARY_EXPR_GenOperandsCode(Compiler,
                                                     CodeGenerator,
                                                     PolynaryExpr,
                                                     &operandCount,
                                                     &operandsParameters);
        if (gcmIS_ERROR(status)) return status;
        gcmASSERT(operandCount > 0);
        gcmVERIFY_OK(cloIR_POLYNARY_EXPR_FinalizeOperandsParameters(Compiler,
                                                                    operandCount,
                                                                    operandsParameters));
    }

    return gcvSTATUS_OK;
}


static gceSTATUS
_GenScalarToMatrixAssignCode(
    IN cloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN clsROPERAND * ScalarROperand,
    IN clsIOPERAND * MatrixIOperand
    )
{
    gceSTATUS    status;
    gctUINT        i, j;
    clsROPERAND    scalarROperand;
    clsLOPERAND    matrixLOperand, componentLOperand;
    clsROPERAND    zeroROperand;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    gcmASSERT(ScalarROperand);
    gcmASSERT(MatrixIOperand);

    scalarROperand = *ScalarROperand;

    status = clsROPERAND_ChangeDataTypeFamily(Compiler,
                          LineNo,
                          StringNo,
                          gcvFALSE,
                          clmGenCodeDataType(T_FLOAT),
                          &scalarROperand);
    if (gcmIS_ERROR(status)) return status;

    clsLOPERAND_InitializeUsingIOperand(&matrixLOperand, MatrixIOperand);
    clsROPERAND_InitializeFloatOrVecOrMatConstant(&zeroROperand,
                              clmGenCodeDataType(T_FLOAT),
                              (gctFLOAT)0.0);

    for (i = 0; i < gcGetMatrixDataTypeColumnCount(MatrixIOperand->dataType); i++)
    {
        for (j = 0; j < gcGetMatrixDataTypeColumnCount(MatrixIOperand->dataType); j++)
        {
            clsLOPERAND_InitializeAsMatrixComponent(&componentLOperand, &matrixLOperand, i, j);

            if (i == j)
            {
                status = clGenAssignCode(Compiler,
                            LineNo,
                            StringNo,
                            &componentLOperand,
                            &scalarROperand);

                if (gcmIS_ERROR(status)) return status;
            }
            else
            {
                status = clGenAssignCode(Compiler,
                            LineNo,
                            StringNo,
                            &componentLOperand,
                            &zeroROperand);

                if (gcmIS_ERROR(status)) return status;
            }
        }
    }

    return gcvSTATUS_OK;
}

static gceSTATUS
_GenMatrixToMatrixAssignCode(
    IN cloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN clsROPERAND * MatrixROperand,
    IN clsIOPERAND * MatrixIOperand
    )
{
    gceSTATUS    status;
    gctUINT        targetMatrixColumnCount, sourceMatrixColumnCount;
    gctUINT        i, j;
    clsLOPERAND    matrixLOperand, columnLOperand, columnSliceLOperand, componentLOperand;
    clsROPERAND    columnROperand, columnSliceROperand, oneROperand, zeroROperand;
    gctUINT8    sliceComponentCount;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    gcmASSERT(MatrixROperand);
    gcmASSERT(MatrixIOperand);

    sourceMatrixColumnCount = gcGetMatrixDataTypeColumnCount(MatrixROperand->dataType);
    targetMatrixColumnCount = gcGetMatrixDataTypeColumnCount(MatrixIOperand->dataType);

    clsLOPERAND_InitializeUsingIOperand(&matrixLOperand, MatrixIOperand);
    clsROPERAND_InitializeFloatOrVecOrMatConstant(&oneROperand,
                              clmGenCodeDataType(T_FLOAT),
                              (gctFLOAT)1.0);
    clsROPERAND_InitializeFloatOrVecOrMatConstant(&zeroROperand,
                              clmGenCodeDataType(T_FLOAT),
                              (gctFLOAT)0.0);

    for (i = 0; i < targetMatrixColumnCount; i++)
    {
        clsLOPERAND_InitializeAsMatrixColumn(&columnLOperand, &matrixLOperand, i);
        clsROPERAND_InitializeAsMatrixColumn(&columnROperand, MatrixROperand, i);

        if (targetMatrixColumnCount == sourceMatrixColumnCount)
        {
            status = clGenAssignCode(Compiler,
                        LineNo,
                        StringNo,
                        &columnLOperand,
                        &columnROperand);

            if (gcmIS_ERROR(status)) return status;
        }
        else if (targetMatrixColumnCount < sourceMatrixColumnCount)
        {
            _GetVectorROperandSlice(&columnROperand,
                        0,
                        (gctUINT8) targetMatrixColumnCount,
                        &columnSliceROperand,
                        &sliceComponentCount);

            gcmASSERT(sliceComponentCount == targetMatrixColumnCount);

            status = clGenAssignCode(Compiler,
                        LineNo,
                        StringNo,
                        &columnLOperand,
                        &columnSliceROperand);

            if (gcmIS_ERROR(status)) return status;
        }
        else
        {
            if (i < sourceMatrixColumnCount)
            {
                clGetVectorLOperandSlice(&columnLOperand,
                             0,
                             (gctUINT8) sourceMatrixColumnCount,
                             &columnSliceLOperand);

                status = clGenAssignCode(Compiler,
                            LineNo,
                            StringNo,
                            &columnSliceLOperand,
                            &columnROperand);

                if (gcmIS_ERROR(status)) return status;

                j = sourceMatrixColumnCount;
            }
            else
            {
                j = 0;
            }

            for (; j < targetMatrixColumnCount; j++)
            {
                clsLOPERAND_InitializeAsMatrixComponent(&componentLOperand, &matrixLOperand, i, j);

                if (i == j)
                {
                    status = clGenAssignCode(Compiler,
                                LineNo,
                                StringNo,
                                &componentLOperand,
                                &oneROperand);

                    if (gcmIS_ERROR(status)) return status;
                }
                else
                {
                    status = clGenAssignCode(Compiler,
                                LineNo,
                                StringNo,
                                &componentLOperand,
                                &zeroROperand);

                    if (gcmIS_ERROR(status)) return status;
                }
            }    /* for */
        }    /* if */
    }    /* for */

    return gcvSTATUS_OK;
}

gceSTATUS
cloIR_POLYNARY_EXPR_GenConstructMatrixCode(
IN cloCOMPILER Compiler,
IN cloCODE_GENERATOR CodeGenerator,
IN cloIR_POLYNARY_EXPR PolynaryExpr,
IN OUT clsGEN_CODE_PARAMETERS * Parameters
)
{
    gceSTATUS        status;
    gctUINT            operandCount;
    clsGEN_CODE_PARAMETERS *operandsParameters;
    clsIOPERAND        iOperand;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_OBJECT(CodeGenerator, clvOBJ_CODE_GENERATOR);
    clmVERIFY_IR_OBJECT(PolynaryExpr, clvIR_POLYNARY_EXPR);
    gcmASSERT(Parameters);
    gcmASSERT(!Parameters->needLOperand);

    status = cloIR_POLYNARY_EXPR_GenOperandsCode(Compiler,
                             CodeGenerator,
                             PolynaryExpr,
                             &operandCount,
                             &operandsParameters);
    gcmASSERT(operandCount > 0);

    if (Parameters->needROperand) {
        /* Allocate the register(s) */
        status = clsGEN_CODE_PARAMETERS_AllocateOperands(Compiler,
                                 Parameters,
                                 &PolynaryExpr->exprBase.decl);
        if (gcmIS_ERROR(status)) return status;

        gcmASSERT(Parameters->operandCount == 1);

        clsIOPERAND_New(Compiler, &iOperand, Parameters->dataTypes[0].def);

        clsROPERAND_InitializeUsingIOperand(&Parameters->rOperands[0], &iOperand);

        /* Generate the assignment(s) */
        if (operandCount == 1 && operandsParameters[0].operandCount == 1
            && gcIsScalarDataType(operandsParameters[0].dataTypes[0].def)) {
            status = _GenScalarToMatrixAssignCode(Compiler,
                                  PolynaryExpr->exprBase.base.lineNo,
                                  PolynaryExpr->exprBase.base.stringNo,
                                  &operandsParameters[0].rOperands[0],
                                  &iOperand);
            if (gcmIS_ERROR(status)) return status;
        }
        else if (operandCount == 1 && operandsParameters[0].operandCount == 1
            && gcIsMatrixDataType(operandsParameters[0].dataTypes[0].def)) {
            status = _GenMatrixToMatrixAssignCode(Compiler,
                                  PolynaryExpr->exprBase.base.lineNo,
                                  PolynaryExpr->exprBase.base.stringNo,
                                  &operandsParameters[0].rOperands[0],
                                  &iOperand);
            if (gcmIS_ERROR(status)) return status;
        }
        else {
            status = cloIR_POLYNARY_EXPR_GenMatrixComponentAssignCode(Compiler,
                                          PolynaryExpr,
                                          operandCount,
                                          operandsParameters,
                                          &iOperand);
            if (gcmIS_ERROR(status)) return status;
        }
    }

    gcmVERIFY_OK(cloIR_POLYNARY_EXPR_FinalizeOperandsParameters(Compiler,
                                    operandCount,
                                    operandsParameters));
    return gcvSTATUS_OK;
}

gceSTATUS
cloIR_POLYNARY_EXPR_GenConstructStructCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN OUT clsGEN_CODE_PARAMETERS * Parameters
    )
{
    gceSTATUS                        status;
    gctUINT                            operandCount = 0;
    clsGEN_CODE_PARAMETERS *        operandsParameters;
    gctUINT                            i, j, k;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_OBJECT(CodeGenerator, clvOBJ_CODE_GENERATOR);
    clmVERIFY_IR_OBJECT(PolynaryExpr, clvIR_POLYNARY_EXPR);
    gcmASSERT(Parameters);
    gcmASSERT(!Parameters->needLOperand);

    status = cloIR_POLYNARY_EXPR_GenOperandsCode(Compiler,
                        CodeGenerator,
                        PolynaryExpr,
                        &operandCount,
                        &operandsParameters);

    gcmASSERT(operandCount > 0);

    if (Parameters->needROperand)
    {
        /* Allocate the register(s) */
        status = clsGEN_CODE_PARAMETERS_AllocateOperands(Compiler,
                                Parameters,
                                &PolynaryExpr->exprBase.decl);

        if (gcmIS_ERROR(status)) return status;

        /* Copy all operands */
        for (i = 0, j = 0, k = 0; i < Parameters->operandCount; i++, k++)
        {
            if (k == operandsParameters[j].operandCount)
            {
                k = 0;
                j++;
            }

            gcmASSERT(j < operandCount);
            gcmASSERT(k < operandsParameters[j].operandCount);
            Parameters->rOperands[i] = operandsParameters[j].rOperands[k];
        }
    }

    gcmVERIFY_OK(cloIR_POLYNARY_EXPR_FinalizeOperandsParameters(Compiler,
                                    operandCount,
                                    operandsParameters));

    return gcvSTATUS_OK;
}

gceSTATUS
cloIR_POLYNARY_EXPR_GenBuiltInAsmCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN OUT clsGEN_CODE_PARAMETERS * Parameters
    )
{
    gceSTATUS                       status = gcvSTATUS_OK;
    gctUINT                         operandCount;
    clsGEN_CODE_PARAMETERS *        operandsParameters;
    clsIOPERAND                     iOperand;
    cleOPCODE                       opcode = clvOPCODE_INVALID;
    gcmHEADER();

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_OBJECT(CodeGenerator, clvOBJ_CODE_GENERATOR);
    clmVERIFY_IR_OBJECT(PolynaryExpr, clvIR_POLYNARY_EXPR);
    gcmASSERT(Parameters);
    gcmASSERT(!Parameters->needLOperand);

    /* Generate the code of all operands */
    status = cloIR_POLYNARY_EXPR_GenOperandsCode(Compiler,
                                                 CodeGenerator,
                                                 PolynaryExpr,
                                                 &operandCount,
                                                 &operandsParameters);

    if(operandCount < 1)
    {
        gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
            PolynaryExpr->exprBase.base.lineNo,
            PolynaryExpr->exprBase.base.stringNo,
            clvREPORT_ERROR,
            "invalid builtin asm '%s'.", PolynaryExpr->funcSymbol));

        gcmFOOTER_ARG("status=%d", gcvSTATUS_INVALID_ARGUMENT);
        return gcvSTATUS_INVALID_ARGUMENT;
    }

    if(operandsParameters[0].rOperands[0].isReg)
    {
        gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
            PolynaryExpr->exprBase.base.lineNo,
            PolynaryExpr->exprBase.base.stringNo,
            clvREPORT_ERROR,
            "invalid builtin asm '%s', opcode must be a constant.", PolynaryExpr->funcSymbol));

        gcmFOOTER_ARG("status=%d", gcvSTATUS_INVALID_ARGUMENT);
        return gcvSTATUS_INVALID_ARGUMENT;
    }

    opcode = (gcSL_OPCODE)operandsParameters[0].rOperands[0].u.constant.values[0].intValue;

    if(operandCount >= 2 &&
        opcode != clvOPCODE_BARRIER &&
       !operandsParameters[1].rOperands[0].isReg)
    {
        gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
            PolynaryExpr->exprBase.base.lineNo,
            PolynaryExpr->exprBase.base.stringNo,
            clvREPORT_ERROR,
            "invalid builtin asm '%s', dest must be writeable.", PolynaryExpr->funcSymbol));

        gcmFOOTER_ARG("status=%d", gcvSTATUS_INVALID_ARGUMENT);
        return gcvSTATUS_INVALID_ARGUMENT;
    }

    switch(opcode)
    {
    case clvOPCODE_NOP:
        break;
    case clvOPCODE_BARRIER:
        if(operandCount != 2)
        {
            gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                PolynaryExpr->exprBase.base.lineNo,
                PolynaryExpr->exprBase.base.stringNo,
                clvREPORT_ERROR,
                "invalid builtin asm '%s'.", PolynaryExpr->funcSymbol));

            gcmFOOTER_ARG("status=%d", gcvSTATUS_INVALID_ARGUMENT);
            return gcvSTATUS_INVALID_ARGUMENT;
        }

        status = clGenGenericNullTargetCode(Compiler,
                                            PolynaryExpr->exprBase.base.lineNo,
                                            PolynaryExpr->exprBase.base.stringNo,
                                            opcode,
                                            operandsParameters[1].rOperands,
                                            gcvNULL);
        if (gcmIS_ERROR(status)) return status;
        break;

      case clvOPCODE_MUL:
      case clvOPCODE_FMUL:
      case clvOPCODE_MUL_RTZ:
      case clvOPCODE_MUL_RTNE:

      case clvOPCODE_ADD:
      case clvOPCODE_FADD:
      case clvOPCODE_ADD_RTZ:
      case clvOPCODE_ADD_RTNE:

      case clvOPCODE_SUB:
      case clvOPCODE_FSUB:
      case clvOPCODE_SUB_RTZ:
      case clvOPCODE_SUB_RTNE:
      case clvOPCODE_ADDLO:
      case clvOPCODE_ADDLO_RTZ:
      case clvOPCODE_ADDLO_RTNE:
      case clvOPCODE_MULLO:
      case clvOPCODE_MULLO_RTZ:
      case clvOPCODE_MULLO_RTNE:
      case clvOPCODE_DIV:
      case clvOPCODE_IDIV:
      case clvOPCODE_IMUL:
      case clvOPCODE_MOD:
      case clvOPCODE_FMOD:
      case clvOPCODE_BITWISE_AND:
      case clvOPCODE_BITWISE_OR:
      case clvOPCODE_BITWISE_XOR:

      case clvOPCODE_MULHI:
      case clvOPCODE_ADDSAT:
      case clvOPCODE_SUBSAT:
      case clvOPCODE_MULSAT:
      case clvOPCODE_MADSAT:

        if(operandCount != 4)
        {
            gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                PolynaryExpr->exprBase.base.lineNo,
                PolynaryExpr->exprBase.base.stringNo,
                clvREPORT_ERROR,
                "invalid builtin asm '%s'.", PolynaryExpr->funcSymbol));

            gcmFOOTER_ARG("status=%d", gcvSTATUS_INVALID_ARGUMENT);
            return gcvSTATUS_INVALID_ARGUMENT;
        }
        clsIOPERAND_Initialize(&iOperand,
            operandsParameters[1].rOperands[0].dataType,
            operandsParameters[1].rOperands[0].u.reg.regIndex);

        status = clGenArithmeticExprCode(
            Compiler,
            PolynaryExpr->exprBase.base.lineNo,
            PolynaryExpr->exprBase.base.stringNo,
            opcode,
            &iOperand,
            operandsParameters[2].rOperands,
            operandsParameters[3].rOperands);
        break;
    case clvOPCODE_FLOAT_TO_BOOL:
    case clvOPCODE_INT_TO_BOOL:
    case clvOPCODE_INT_TO_FLOAT:
    case clvOPCODE_INT_TO_UINT:
    case clvOPCODE_UINT_TO_FLOAT:
    case clvOPCODE_UINT_TO_INT:
    case clvOPCODE_UINT_TO_BOOL:
    case clvOPCODE_BOOL_TO_FLOAT:
    case clvOPCODE_BOOL_TO_INT:
    case clvOPCODE_BOOL_TO_UINT:
    case clvOPCODE_CONV:
    case clvOPCODE_CONV_RTE:
    case clvOPCODE_CONV_RTZ:
    case clvOPCODE_CONV_RTN:
    case clvOPCODE_CONV_RTP:
    case clvOPCODE_CONV_SAT:
    case clvOPCODE_CONV_SAT_RTE:
    case clvOPCODE_CONV_SAT_RTZ:
    case clvOPCODE_CONV_SAT_RTN:
    case clvOPCODE_CONV_SAT_RTP:
    case clvOPCODE_INVERSE:
    case clvOPCODE_ANY:
    case clvOPCODE_ALL:
    case clvOPCODE_NOT:
    case clvOPCODE_NEG:
    case clvOPCODE_BITWISE_NOT:
    case clvOPCODE_SIN:
    case clvOPCODE_COS:
    case clvOPCODE_TAN:
    case clvOPCODE_ASIN:
    case clvOPCODE_ACOS:
    case clvOPCODE_ATAN:
    case clvOPCODE_EXP2:
    case clvOPCODE_LOG2:
    case clvOPCODE_SQRT:
    case clvOPCODE_INVERSE_SQRT:
    case clvOPCODE_ABS:
    case clvOPCODE_SIGN:
    case clvOPCODE_FLOOR:
    case clvOPCODE_CEIL:
    case clvOPCODE_FRACT:
    case clvOPCODE_SATURATE:
    case clvOPCODE_NORMALIZE:
    case clvOPCODE_DFDX:
    case clvOPCODE_DFDY:
    case clvOPCODE_FWIDTH:
    case clvOPCODE_ASSIGN:

    case clvOPCODE_LONGLO:
    case clvOPCODE_LONGHI:
    case clvOPCODE_MOV_LONG:
    case clvOPCODE_COPY:
        if(operandCount != 3)
        {
            gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                PolynaryExpr->exprBase.base.lineNo,
                PolynaryExpr->exprBase.base.stringNo,
                clvREPORT_ERROR,
                "invalid builtin asm '%s'.", PolynaryExpr->funcSymbol));

            gcmFOOTER_ARG("status=%d", gcvSTATUS_INVALID_ARGUMENT);
            return gcvSTATUS_INVALID_ARGUMENT;
        }

        clsIOPERAND_Initialize(&iOperand,
            operandsParameters[1].rOperands[0].dataType,
            operandsParameters[1].rOperands[0].u.reg.regIndex);

        status = clGenGenericCode1(
            Compiler,
            PolynaryExpr->exprBase.base.lineNo,
            PolynaryExpr->exprBase.base.stringNo,
            opcode,
            &iOperand,
            operandsParameters[1].rOperands);
        break;
    case clvOPCODE_LESS_THAN:
    case clvOPCODE_LESS_THAN_EQUAL:
    case clvOPCODE_GREATER_THAN:
    case clvOPCODE_GREATER_THAN_EQUAL:
    case clvOPCODE_EQUAL:
    case clvOPCODE_NOT_EQUAL:

    case clvOPCODE_LOAD:

    case clvOPCODE_TEXTURE_LOAD:
    case clvOPCODE_IMAGE_SAMPLER:
    case clvOPCODE_IMAGE_READ:
    case clvOPCODE_IMAGE_WRITE:
    case clvOPCODE_ATAN2:

    case clvOPCODE_POW:

    case clvOPCODE_ROTATE:
    case clvOPCODE_MIN:
    case clvOPCODE_MAX:
    case clvOPCODE_STEP:
    case clvOPCODE_CMP:
    case clvOPCODE_FLOAT_TO_UINT:
    case clvOPCODE_SELECT:
    case clvOPCODE_RSHIFT:
    case clvOPCODE_LSHIFT:
    case clvOPCODE_RIGHT_SHIFT:
    case clvOPCODE_LEFT_SHIFT:

    case clvOPCODE_ATOMADD:
    case clvOPCODE_ATOMSUB:
    case clvOPCODE_ATOMXCHG:
    case clvOPCODE_ATOMCMPXCHG:
    case clvOPCODE_ATOMMIN:
    case clvOPCODE_ATOMMAX:
    case clvOPCODE_ATOMOR:
    case clvOPCODE_ATOMAND:
    case clvOPCODE_ATOMXOR:

    case clvOPCODE_UNPACK:

    case clvOPCODE_DOT:
    case clvOPCODE_CROSS:
        if(operandCount != 4)
        {
            gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                PolynaryExpr->exprBase.base.lineNo,
                PolynaryExpr->exprBase.base.stringNo,
                clvREPORT_ERROR,
                "invalid builtin asm '%s'.", PolynaryExpr->funcSymbol));

            gcmFOOTER_ARG("status=%d", gcvSTATUS_INVALID_ARGUMENT);
            return gcvSTATUS_INVALID_ARGUMENT;
        }

        clsIOPERAND_Initialize(&iOperand,
            operandsParameters[1].rOperands[0].dataType,
            operandsParameters[1].rOperands[0].u.reg.regIndex);

        status = clGenGenericCode2(
            Compiler,
            PolynaryExpr->exprBase.base.lineNo,
            PolynaryExpr->exprBase.base.stringNo,
            opcode,
            &iOperand,
            operandsParameters[2].rOperands,
            operandsParameters[3].rOperands);
        break;
    default:
        status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
        break;
    }

    if (gcmIS_ERROR(status)) {
        gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
            PolynaryExpr->exprBase.base.lineNo,
            PolynaryExpr->exprBase.base.stringNo,
            clvREPORT_ERROR,
            "invalid builtin asm '%s'", PolynaryExpr->funcSymbol));

        gcmFOOTER_ARG("status=%d", gcvSTATUS_COMPILER_FE_PARSER_ERROR);
        return gcvSTATUS_COMPILER_FE_PARSER_ERROR;
    }

    gcmVERIFY_OK(cloIR_POLYNARY_EXPR_FinalizeOperandsParameters(Compiler,
                                                                operandCount,
                                                                operandsParameters));

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
cloIR_POLYNARY_EXPR_GenBuiltinCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN OUT clsGEN_CODE_PARAMETERS * Parameters
    )
{
    gceSTATUS status;
    gctUINT    operandCount;
    clsGEN_CODE_PARAMETERS *operandsParameters;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_OBJECT(CodeGenerator, clvOBJ_CODE_GENERATOR);
    clmVERIFY_IR_OBJECT(PolynaryExpr, clvIR_POLYNARY_EXPR);
    gcmASSERT(Parameters);
    gcmASSERT(!Parameters->needLOperand);

    /* Generate the code of all operands */
    status = cloIR_POLYNARY_EXPR_GenOperandsCode(Compiler,
                             CodeGenerator,
                             PolynaryExpr,
                             &operandCount,
                             &operandsParameters);
    if (gcmIS_ERROR(status)) return status;

    /* make sure that operand count is set to 0 to signal operands to be allocated */
    Parameters->operandCount = 0;

    /* Generate the built-in code */
    status = clGenBuiltinFunctionCode(Compiler,
                      CodeGenerator,
                      PolynaryExpr,
                      operandCount,
                      operandsParameters,
                      gcvNULL,
                      Parameters,
                      gcvTRUE);
    if (gcmIS_ERROR(status)) return status;

    gcmVERIFY_OK(cloIR_POLYNARY_EXPR_FinalizeOperandsParameters(Compiler,
                                    operandCount,
                                    operandsParameters));

    return gcvSTATUS_OK;
}

gceSTATUS
clGenFuncCallCode(
IN cloCOMPILER Compiler,
IN cloCODE_GENERATOR CodeGenerator,
IN cloIR_POLYNARY_EXPR PolynaryExpr,
IN clsGEN_CODE_PARAMETERS * operandsParameters,
IN OUT clsGEN_CODE_PARAMETERS * Parameters
)
{
    gceSTATUS status = gcvSTATUS_OK;
    gctUINT    i, j;
    clsLOPERAND    lOperand[1];
    gctLABEL    funcLabel;
    clsNAME *    paramName;
    cltQUALIFIER accessQualifier;

    /* Set all 'in' arguments */
    i = 0;
    FOR_EACH_DLINK_NODE(&PolynaryExpr->funcName->u.funcInfo.localSpace->names, clsNAME, paramName) {
        if (paramName->type != clvPARAMETER_NAME) break;

        accessQualifier = _MapQualifierToShaderQualifier(paramName->decl.dataType->accessQualifier);
        switch (accessQualifier) {
        case clvQUALIFIER_NONE:
        case clvQUALIFIER_CONST:
        case clvQUALIFIER_READ_ONLY:
        case clvQUALIFIER_WRITE_ONLY:
        case clvQUALIFIER_CONST_IN:
            gcmASSERT(operandsParameters[i].needROperand);

            if(operandsParameters[i].hint & clvGEN_ADDRESS_OFFSET) {
                operandsParameters[i].hint &= ~clvGEN_ADDRESS_OFFSET;
                status = _GenAddressOffsetCode(Compiler,
                                               &PolynaryExpr->exprBase,
                                               0,
                                               operandsParameters + i);
                if(gcmIS_ERROR(status)) return status;
            }
            for (j = 0; j < operandsParameters[i].operandCount; j++) {
                clsLOPERAND_Initialize(lOperand,
                                       paramName->context.u.variable.logicalRegs + j);

                status = clGenAssignCode(Compiler,
                                         PolynaryExpr->exprBase.base.lineNo,
                                         PolynaryExpr->exprBase.base.stringNo,
                                         lOperand,
                                         operandsParameters[i].rOperands + j);
                if (gcmIS_ERROR(status)) return status;
            }
            break;

        default:
            gcmASSERT(0);
            break;
        }
        i++;
    }

    /* Generate the call code */
#if cldSupportMultiKernelFunction
    status = clmGetFunctionLabel(Compiler,
                                 PolynaryExpr->funcName,
                                 &funcLabel);
#else
    status = clGetFunctionLabel(Compiler,
                                PolynaryExpr->funcName->context.u.variable.u.function,
                                &funcLabel);
#endif
    if (gcmIS_ERROR(status)) return status;

    status = clEmitAlwaysBranchCode(Compiler,
                                    PolynaryExpr->exprBase.base.lineNo,
                                    PolynaryExpr->exprBase.base.stringNo,
                                    clvOPCODE_CALL,
                                    funcLabel);
    if (gcmIS_ERROR(status)) return status;

    /* Get all 'out' arguments */
/* There is no outputs in openCL */

    /* Get the return value */
    if (Parameters->needROperand) {
        clsIOPERAND intermIOperand[1];
        clsROPERAND rOperand[1];
        clsLOPERAND lOperand[1];

        if(Parameters->operandCount == 0 ) {
            status = clsGEN_CODE_PARAMETERS_AllocateOperandsByName(Compiler,
                                                                   Parameters,
                                                                   PolynaryExpr->funcName);
            if (gcmIS_ERROR(status)) return status;
        }

        for (i = 0; i < Parameters->operandCount; i++) {
            clmGEN_CODE_GetParametersIOperand(Compiler,
                                              intermIOperand,
                                              Parameters,
                                              Parameters->dataTypes[0].def);
            clsLOPERAND_InitializeUsingIOperand(lOperand, intermIOperand);

            clsROPERAND_InitializeReg(rOperand,
                                      PolynaryExpr->funcName->context.u.variable.logicalRegs + i);

            status = clGenAssignCode(Compiler,
                                     PolynaryExpr->exprBase.base.lineNo,
                                     PolynaryExpr->exprBase.base.stringNo,
                                     lOperand,
                                     rOperand);
            if (gcmIS_ERROR(status)) return status;

            clsROPERAND_InitializeUsingIOperand(&Parameters->rOperands[i], intermIOperand);
        }
        if(clmDECL_IsUnderlyingStructOrUnion(&PolynaryExpr->funcName->decl) &&
            clmNAME_VariableHasMemoryOffset(PolynaryExpr->funcName)) {
            Parameters->hint = clvGEN_DEREF_CODE;
        }
    }

    PolynaryExpr->funcName->context.u.variable.u.function->intrinsicsKind = gceINTRIN_NONE;
    if (PolynaryExpr->funcName->u.funcInfo.isIntrinsicCall ||
        IsFunctionExtern(PolynaryExpr->funcName->context.u.variable.u.function))
    {
        gcSHADER    shader;
        gcSHADER_LABEL  shaderLabel;

        gcmVERIFY_OK(cloCOMPILER_GetBinary(Compiler, &shader));

        /* set shader has the intrinsic builtin */
        if (PolynaryExpr->funcName->u.funcInfo.isIntrinsicCall)
        {
            PolynaryExpr->funcName->context.u.variable.u.function->intrinsicsKind =
                PolynaryExpr->funcName->u.funcInfo.intrinsicKind;
            SetShaderHasIntrinsicBuiltin(shader, gcvTRUE);
        }
        else /* set shader to have extern functions */
        {
            SetShaderHasExternFunction(shader, gcvTRUE);
        }

        /* set extern function label's function, these labels need to be resolved
           later at linking time */
        status = gcSHADER_FindLabel(shader, funcLabel, &shaderLabel);
        if (gcmIS_ERROR(status)) return status;

        shaderLabel->function = PolynaryExpr->funcName->context.u.variable.u.function;
    }

    return status;
}

gceSTATUS
cloIR_POLYNARY_EXPR_GenFuncCallCode(
IN cloCOMPILER Compiler,
IN cloCODE_GENERATOR CodeGenerator,
IN cloIR_POLYNARY_EXPR PolynaryExpr,
IN OUT clsGEN_CODE_PARAMETERS * Parameters
)
{
    gceSTATUS    status;
    gctUINT        operandCount;
    clsGEN_CODE_PARAMETERS *operandsParameters;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_OBJECT(CodeGenerator, clvOBJ_CODE_GENERATOR);
    clmVERIFY_IR_OBJECT(PolynaryExpr, clvIR_POLYNARY_EXPR);
    gcmASSERT(Parameters);
    gcmASSERT(!Parameters->needLOperand);

    if (!PolynaryExpr->funcName->isBuiltin &&
        !PolynaryExpr->funcName->u.funcInfo.isIntrinsicCall && !PolynaryExpr->funcName->u.funcInfo.isFuncDef &&
        (!cloCOMPILER_IsExternSymbolsAllowed(Compiler) || !(PolynaryExpr->funcName->decl.storageQualifier & clvSTORAGE_QUALIFIER_EXTERN)))
    {
        gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                        PolynaryExpr->exprBase.base.lineNo,
                                        PolynaryExpr->exprBase.base.stringNo,
                                        clvREPORT_ERROR,
                                        "function \'%s\' has not been defined",
                                        PolynaryExpr->funcName->symbol));
        return gcvSTATUS_COMPILER_FE_PARSER_ERROR;
    }

    /* Allocate the function resources */
    status = _AllocateFuncResources(Compiler,
                                    CodeGenerator,
                                    PolynaryExpr->funcName);
    if (gcmIS_ERROR(status)) return status;

    /* Generate the code of all operands */
    status = cloIR_POLYNARY_EXPR_GenOperandsCodeForFuncCall(Compiler,
                                                            CodeGenerator,
                                                            PolynaryExpr,
                                                            &operandCount,
                                                            &operandsParameters);
    if (gcmIS_ERROR(status)) return status;

    Parameters->operandCount = 0;
    status = clGenFuncCallCode(Compiler,
                               CodeGenerator,
                               PolynaryExpr,
                               operandsParameters,
                               Parameters);
    if (gcmIS_ERROR(status)) return status;

    gcmVERIFY_OK(cloIR_POLYNARY_EXPR_FinalizeOperandsParameters(Compiler,
                                                                operandCount,
                                                                operandsParameters));

    return gcvSTATUS_OK;
}

gceSTATUS
cloIR_POLYNARY_EXPR_TryToEvaluate(
IN cloCOMPILER Compiler,
IN cloCODE_GENERATOR CodeGenerator,
IN cloIR_POLYNARY_EXPR PolynaryExpr,
IN OUT clsGEN_CODE_PARAMETERS * Parameters
)
{
    gceSTATUS                status;
    cloIR_POLYNARY_EXPR        newPolynaryExpr;
    cloIR_EXPR                operand;
    clsGEN_CODE_PARAMETERS    operandParameters;
    cloIR_CONSTANT            operandConstant;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_OBJECT(CodeGenerator, clvOBJ_CODE_GENERATOR);
    clmVERIFY_IR_OBJECT(PolynaryExpr, clvIR_POLYNARY_EXPR);
    gcmASSERT(Parameters);

    if (PolynaryExpr->type == clvPOLYNARY_FUNC_CALL && (!PolynaryExpr->funcName->isBuiltin || !PolynaryExpr->operands)) {
        return gcvSTATUS_OK;
    }

    /* Create a new polynary expression */
    status = cloIR_POLYNARY_EXPR_Construct(Compiler,
                           PolynaryExpr->exprBase.base.lineNo,
                           PolynaryExpr->exprBase.base.stringNo,
                           PolynaryExpr->type,
                           &PolynaryExpr->exprBase.decl,
                           PolynaryExpr->funcSymbol,
                           &newPolynaryExpr);

    if (gcmIS_ERROR(status)) return status;
    status = cloIR_SET_Construct(Compiler,
                     PolynaryExpr->operands->base.lineNo,
                     PolynaryExpr->operands->base.stringNo,
                     PolynaryExpr->operands->type,
                     &newPolynaryExpr->operands);
    if (gcmIS_ERROR(status)) return status;
    if(PolynaryExpr->operands) {
       /* Try to evaluate all operands */
       FOR_EACH_DLINK_NODE(&PolynaryExpr->operands->members, struct _cloIR_EXPR, operand) {
        /* Try to evaluate the operand */
        clsGEN_CODE_PARAMETERS_Initialize(&operandParameters,
                          gcvFALSE,
                          gcvTRUE);

        operandParameters.hint = clvEVALUATE_ONLY;

        status = cloIR_OBJECT_Accept(Compiler,
                         &operand->base,
                         &CodeGenerator->visitor,
                         &operandParameters);
        if (gcmIS_ERROR(status)) return status;

        operandConstant            = operandParameters.constant;
        operandParameters.constant    = gcvNULL;

        clsGEN_CODE_PARAMETERS_Finalize(&operandParameters);

        if (operandConstant == gcvNULL) goto Exit;

        /* Add to the new polynary expression */
        gcmVERIFY_OK(cloIR_SET_AddMember(Compiler,
                         newPolynaryExpr->operands,
                         &operandConstant->exprBase.base));
       }
    }

    if (newPolynaryExpr->type == clvPOLYNARY_FUNC_CALL) {
            status = cloCOMPILER_BindBuiltinFuncCall(Compiler,
                                                         newPolynaryExpr);
        if (gcmIS_ERROR(status)) return status;
    }

    /* Try to evaluate the new polynary expression */
    status = cloIR_POLYNARY_EXPR_Evaluate(Compiler,
                          newPolynaryExpr,
                          &Parameters->constant);

    if (gcmIS_SUCCESS(status) && Parameters->constant != gcvNULL) {
        newPolynaryExpr = gcvNULL;
    }

Exit:
    if (newPolynaryExpr != gcvNULL) {
        gcmVERIFY_OK(cloIR_OBJECT_Destroy(Compiler, &newPolynaryExpr->exprBase.base));
    }
    return gcvSTATUS_OK;
}

gceSTATUS
cloIR_POLYNARY_EXPR_GenCode(
IN cloCOMPILER Compiler,
IN cloCODE_GENERATOR CodeGenerator,
IN cloIR_POLYNARY_EXPR PolynaryExpr,
IN OUT clsGEN_CODE_PARAMETERS * Parameters
)
{
    gceSTATUS    status;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_OBJECT(CodeGenerator, clvOBJ_CODE_GENERATOR);
    clmVERIFY_IR_OBJECT(PolynaryExpr, clvIR_POLYNARY_EXPR);
    gcmASSERT(Parameters);

    /* Try to evaluate the polynary expression */
    if (!Parameters->needLOperand && Parameters->needROperand) {
        status = cloIR_POLYNARY_EXPR_TryToEvaluate(Compiler,
                               CodeGenerator,
                               PolynaryExpr,
                               Parameters);
        if (gcmIS_ERROR(status)) return status;

        if (Parameters->hint == clvEVALUATE_ONLY) return gcvSTATUS_OK;

        if (Parameters->constant != gcvNULL) {
            return cloIR_CONSTANT_GenCode(Compiler,
                              CodeGenerator,
                              Parameters->constant,
                              Parameters);
        }
    }

    switch (PolynaryExpr->type) {
    case clvPOLYNARY_CONSTRUCT_SCALAR:
        return cloIR_POLYNARY_EXPR_GenConstructScalarCode(Compiler,
                                  CodeGenerator,
                                  PolynaryExpr,
                                  Parameters);

    case clvPOLYNARY_CONSTRUCT_ARRAY:
        return cloIR_POLYNARY_EXPR_GenConstructArrayCode(Compiler,
                                 CodeGenerator,
                                 PolynaryExpr,
                                 Parameters);

    case clvPOLYNARY_CONSTRUCT_VECTOR:
        return cloIR_POLYNARY_EXPR_GenConstructVectorCode(Compiler,
                                  CodeGenerator,
                                  PolynaryExpr,
                                  Parameters);

    case clvPOLYNARY_CONSTRUCT_MATRIX:
        return cloIR_POLYNARY_EXPR_GenConstructMatrixCode(Compiler,
                                  CodeGenerator,
                                  PolynaryExpr,
                                  Parameters);

    case clvPOLYNARY_CONSTRUCT_STRUCT:
        return cloIR_POLYNARY_EXPR_GenConstructStructCode(Compiler,
                                  CodeGenerator,
                                  PolynaryExpr,
                                  Parameters);

    case clvPOLYNARY_FUNC_CALL:
        gcmASSERT(PolynaryExpr->funcName);

        /* generate a normal function call for intrinsic builtin functions */
        if (PolynaryExpr->funcName->u.funcInfo.isIntrinsicCall)
        {
            return cloIR_POLYNARY_EXPR_GenFuncCallCode(Compiler,
                                                       CodeGenerator,
                                                       PolynaryExpr,
                                                       Parameters);

        }
        else if (PolynaryExpr->funcName->isBuiltin) {
            return cloIR_POLYNARY_EXPR_GenBuiltinCode(Compiler,
                                                      CodeGenerator,
                                                      PolynaryExpr,
                                                      Parameters);
        }
        else {
            return cloIR_POLYNARY_EXPR_GenFuncCallCode(Compiler,
                                                       CodeGenerator,
                                                       PolynaryExpr,
                                                       Parameters);
        }

    case clvPOLYNARY_BUILT_IN_ASM_CALL:
        return cloIR_POLYNARY_EXPR_GenBuiltInAsmCode(Compiler,
                                                     CodeGenerator,
                                                     PolynaryExpr,
                                                     Parameters);
    default:
        gcmASSERT(0);
        return gcvSTATUS_INVALID_ARGUMENT;
    }
}

gceSTATUS
cloCODE_GENERATOR_Construct(
IN cloCOMPILER Compiler,
OUT cloCODE_GENERATOR * CodeGenerator
)
{
    gceSTATUS        status;
    cloCODE_GENERATOR    codeGenerator;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    gcmASSERT(CodeGenerator);

    do {
        gcmONERROR(cloCOMPILER_ZeroMemoryAllocate(Compiler,
                              (gctSIZE_T)sizeof(struct _cloCODE_GENERATOR),
                              (gctPOINTER *) &codeGenerator));

        /* Initialize the visitor */
        codeGenerator->visitor.object.type    = clvOBJ_CODE_GENERATOR;

        codeGenerator->visitor.visitSet        =
                        (cltVISIT_SET_FUNC_PTR)cloIR_SET_GenCode;

        codeGenerator->visitor.visitIteration    =
                        (cltVISIT_ITERATION_FUNC_PTR)cloIR_ITERATION_GenCode;

        codeGenerator->visitor.visitJump    =
                        (cltVISIT_JUMP_FUNC_PTR)cloIR_JUMP_GenCode;

        codeGenerator->visitor.visitLabel    =
                        (cltVISIT_LABEL_FUNC_PTR)cloIR_LABEL_GenCode;

        codeGenerator->visitor.visitVariable    =
                        (cltVISIT_VARIABLE_FUNC_PTR)cloIR_VARIABLE_GenCode;

        codeGenerator->visitor.visitConstant    =
                        (cltVISIT_CONSTANT_FUNC_PTR)cloIR_CONSTANT_GenCode;

        codeGenerator->visitor.visitUnaryExpr    =
                        (cltVISIT_UNARY_EXPR_FUNC_PTR)cloIR_UNARY_EXPR_GenCode;

        codeGenerator->visitor.visitBinaryExpr    =
                        (cltVISIT_BINARY_EXPR_FUNC_PTR)cloIR_BINARY_EXPR_GenCode;

        codeGenerator->visitor.visitSelection    =
                        (cltVISIT_SELECTION_FUNC_PTR)cloIR_SELECTION_GenCode;

        codeGenerator->visitor.visitSwitch    =
                        (cltVISIT_SWITCH_FUNC_PTR)cloIR_SWITCH_GenCode;

        codeGenerator->visitor.visitPolynaryExpr=
                        (cltVISIT_POLYNARY_EXPR_FUNC_PTR)cloIR_POLYNARY_EXPR_GenCode;

        /* Initialize other data members */
        codeGenerator->currentIterationContext    = gcvNULL;

        gcmONERROR(gcoHAL_QueryChipIdentity(gcvNULL,
                            &codeGenerator->chipModel,
                            &codeGenerator->chipRevision,
                            gcvNULL,
                            gcvNULL));

        codeGenerator->hasNEW_SIN_COS_LOG_DIV =
            gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_SHADER_HAS_EXTRA_INSTRUCTIONS2);
        codeGenerator->supportRTNE =
            gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_SHADER_HAS_RTNE);
        codeGenerator->supportAtomic =
            gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_SHADER_HAS_ATOMIC);

        codeGenerator->fpConfig = cldFpCapsDefault;
        codeGenerator->fpConfig |= cloCOMPILER_GetFpConfig(Compiler);

        if(!cldNeedFpRoundToNearest && !codeGenerator->supportRTNE)
        {
           codeGenerator->fpConfig &= ~cldFpROUND_TO_NEAREST;
        }
        codeGenerator->needFloatingPointAccuracy = cldNeedFloatingPointAccuracy;

        /* Add INF handling for RTZ if needed. */
        if((codeGenerator->chipModel == gcv2100) ||
           ((codeGenerator->chipModel == gcv2000) && (codeGenerator->chipRevision == 0x5108))) {
            gctUINT floatOpsUsed;
            gctUINT count = 0;

            floatOpsUsed = clGetFloatOpsUsed(Compiler, gcvTRUE);
            if(!floatOpsUsed) {
                floatOpsUsed = clGetFloatOpsUsed(Compiler, gcvFALSE);
                if(floatOpsUsed) {
                    do {
                        count += (floatOpsUsed & 0x1);
                        floatOpsUsed >>= 1;
                    } while(floatOpsUsed);
                    if(count == 1) {
                        codeGenerator->needFloatingPointAccuracy = gcvTRUE;
                    }
                }
            }
        }
        if(codeGenerator->fpConfig & cldFpFAST_RELAXED_MATH) {
            codeGenerator->needFloatingPointAccuracy = gcvFALSE;
        }

/* To Do */
/* Query CL device info via gcoCL_QueryDeviceInfo to set the floating capabilty */

        codeGenerator->derivedTypeNameBufferSize = 0;
        slmSLINK_LIST_Initialize(codeGenerator->derivedTypeVariables);
        codeGenerator->currentUniformBlockMember = -1;
        codeGenerator->uniformBlock = gcvNULL;
        *CodeGenerator = codeGenerator;
        return gcvSTATUS_OK;
    } while (gcvFALSE);

OnError:
    *CodeGenerator = gcvNULL;
    return status;
}

gceSTATUS
cloCODE_GENERATOR_Destroy(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator
    )
{
    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_OBJECT(CodeGenerator, clvOBJ_CODE_GENERATOR);;

    if(CodeGenerator->derivedTypeNameBufferSize) {
        gceSTATUS status;
        gctCHAR *buffer;
        gctPOINTER pointer;
        struct _clsDERIVED_TYPE_VARIABLE *derivedTypeVariable;
        gcSHADER shader;

        status = cloCOMPILER_Allocate(Compiler,
                                      (gctSIZE_T)sizeof(gctCHAR) * CodeGenerator->derivedTypeNameBufferSize,
                                      &pointer);
        if (gcmIS_ERROR(status)) return gcvSTATUS_OUT_OF_MEMORY;

        buffer = pointer;

        while (!slmSLINK_LIST_IsEmpty(CodeGenerator->derivedTypeVariables)) {
            slmSLINK_LIST_DetachFirst(CodeGenerator->derivedTypeVariables,
                                      struct _clsDERIVED_TYPE_VARIABLE,
                                      &derivedTypeVariable);

            gcmVERIFY_OK(gcoOS_StrCopySafe(buffer,
                                           derivedTypeVariable->typeNameLength,
                                           derivedTypeVariable->name->symbol));
            buffer += derivedTypeVariable->typeNameLength;
            cloCOMPILER_Free(Compiler, derivedTypeVariable);
        }

        gcmVERIFY_OK(cloCOMPILER_GetBinary(Compiler, &shader));
        gcmONERROR(gcSHADER_AddTypeNameBuffer(shader,
                                              CodeGenerator->derivedTypeNameBufferSize,
                                              (gctSTRING)pointer));
OnError:
        cloCOMPILER_Free(Compiler, pointer);
    }
    gcmVERIFY_OK(cloCOMPILER_Free(Compiler, CodeGenerator));

    return gcvSTATUS_OK;
}
