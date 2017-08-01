/****************************************************************************
*
*    Copyright (c) 2005 - 2017 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


#ifndef __gc_cl_ir_h_
#define __gc_cl_ir_h_

#include "gc_cl_compiler_int.h"
#include "debug/gc_vsc_debug.h"

#define cldMAX_VECTOR_COMPONENT  16    /*maximum number of components in a vector */
#define cldBASIC_VECTOR_SIZE    4      /*Size of a basic vector object: any sized vector
                                         will need to be a multiple of this */
#define cldBASIC_VECTOR_BYTE_SIZE (cldBASIC_VECTOR_SIZE * cldMachineBytesPerWord)
#define cldMAX_ARRAY_DIMENSION  4      /*maximum number of dimension for array allowed */

#ifndef _GEN_UNIFORMS_FOR_CONSTANT_ADDRESS_SPACE_VARIABLES
#define _GEN_UNIFORMS_FOR_CONSTANT_ADDRESS_SPACE_VARIABLES 0
#endif

/* Compute aligned value in bytes */
#define clmALIGN(N, Alignment, Packed) \
   ((Packed) ? (N) : gcmALIGN(N, Alignment))

#define clmF2B(f)        (((f) != (gctFLOAT)0.0)? gcvTRUE : gcvFALSE)
#define clmF2I(f)        ((gctINT32)(f))
#define clmF2U(f)        ((gctUINT32)(f))
#define clmF2L(f)        ((gctINT64)(f))
#define clmF2UL(f)       ((gctUINT64)(f))
#define clmF2C(f)        ((gctCHAR)(f))
#define clmF2H(f)        ((gctUINT)clConvFloatToHalf((f)))

#define clmI2F(i)        ((gctFLOAT)(i))
#define clmI2U(i)        ((gctUINT32)(i))
#define clmI2L(i)        ((gctINT64)(i))
#define clmI2UL(i)       ((gctUINT64)(i))
#define clmI2B(i)        (((i) != (gctINT32)0)? gcvTRUE : gcvFALSE)
#define clmI2C(i)        ((gctCHAR)(i))

#define clmU2F(i)        ((gctFLOAT)(i))
#define clmU2I(i)        ((gctINT32)(i))
#define clmU2L(i)        ((gctINT64)(i))
#define clmU2UL(i)       ((gctUINT64)(i))
#define clmU2B(i)        (((i) != (gctUINT32)0)? gcvTRUE : gcvFALSE)
#define clmU2C(i)        ((gctCHAR)(i))

#define clmL2F(i)        ((gctFLOAT)(i))
#define clmL2U(i)        ((gctUINT32)(i))
#define clmL2I(i)        ((gctINT32)(i))
#define clmL2UL(i)       ((gctUINT64)(i))
#define clmL2B(i)        (((i) != (gctINT64)0)? gcvTRUE : gcvFALSE)
#define clmL2C(i)        ((gctCHAR)(i))

#define clmUL2F(i)       ((gctFLOAT)(i))
#define clmUL2I(i)       ((gctINT32)(i))
#define clmUL2U(i)       ((gctUINT32)(i))
#define clmUL2L(i)       ((gctINT64)(i))
#define clmUL2B(i)       (((i) != (gctUINT64)0)? gcvTRUE : gcvFALSE)
#define clmUL2C(i)       ((gctCHAR)(i))

#define clmB2F(b)        ((b)? (gctFLOAT)1.0 : (gctFLOAT)0.0)
#define clmB2I(b)        ((b)? (gctINT32)1 : (gctINT32)0)
#define clmB2U(b)        ((b)? (gctUINT32)1 : (gctUINT32)0)
#define clmB2L(b)        ((b)? (gctINT64)1 : (gctINT64)0)
#define clmB2UL(b)       ((b)? (gctUINT64)1 : (gctUINT64)0)
#define clmB2C(b)        ((b)? (gctCHAR)1 : (gctCHAR)0)

#define clmC2F(c)        ((gctFLOAT)(c))
#define clmC2U(c)        ((gctUINT32)(c))
#define clmC2B(c)        (((c) != (gctCHAR)0)? gcvTRUE : gcvFALSE)
#define clmC2I(c)        ((gctINT32)(c))
#define clmC2L(c)        ((gctINT64)(c))
#define clmC2UL(c)       ((gctUINT64)(c))

struct _clsDECL; /* Decl */
struct _clsNAME; /* Name */

struct _clsARRAY;

/* BUILTIN VARIABLE TYPES */
typedef enum _cleBUILTIN_VARIABLE
{
    clvBUILTIN_NONE,
    clvBUILTIN_GLOBAL_ID,
    clvBUILTIN_LOCAL_ID,
    clvBUILTIN_GROUP_ID,
    clvBUILTIN_WORK_DIM,
    clvBUILTIN_GLOBAL_SIZE,
    clvBUILTIN_LOCAL_SIZE,
    clvBUILTIN_NUM_GROUPS,
    clvBUILTIN_GLOBAL_OFFSET,
    clvBUILTIN_LOCAL_ADDRESS_SPACE,
    clvBUILTIN_PRIVATE_ADDRESS_SPACE,
    clvBUILTIN_CONSTANT_ADDRESS_SPACE,
    clvBUILTIN_ARG_LOCAL_MEM_SIZE,
    clvBUILTIN_PRINTF_ADDRESS,
    clvBUILTIN_WORKITEM_PRINTF_BUFFER_SIZE,
    clvBUILTIN_KERNEL_ARG,
    clvBUILTIN_LAST_ONE   /* This is a fake one to mark the last of all builtin's
                                 New builtins have to be added before this one and the
                                 order has to be maintained - please refer to corresponding
                                 definitions in gc_cl_built_ins.c */
}
cleBUILTIN_VARIABLE;
#define cldNumBuiltinVariables  (clvBUILTIN_LAST_ONE - clvBUILTIN_NONE)

enum _cleQUALIFIER
{
    clvQUALIFIER_NONE    = 0,

    clvQUALIFIER_CONSTANT,
    clvQUALIFIER_GLOBAL,
    clvQUALIFIER_LOCAL,
    clvQUALIFIER_PRIVATE,

    clvQUALIFIER_CONST,
    clvQUALIFIER_UNIFORM,
    clvQUALIFIER_ATTRIBUTE,

    clvQUALIFIER_CONST_IN,

    clvQUALIFIER_READ_ONLY,
    clvQUALIFIER_WRITE_ONLY,

    clvQUALIFIER_OUT,

    clvQUALIFIER_END  /* Do not define past this. Use to compute the number
                         of qualifier types = clvQUALIFIER_END - clvQUALIFIER_NONE */
};

enum _cleSTORAGE_QUALIFIER
{
    clvSTORAGE_QUALIFIER_NONE      = 0x0,
    clvSTORAGE_QUALIFIER_RESTRICT  = 0x1,
    clvSTORAGE_QUALIFIER_VOLATILE  = 0x2,

    clvSTORAGE_QUALIFIER_STATIC    = 0x4,
    clvSTORAGE_QUALIFIER_EXTERN    = 0x8,
};

#define clvSTORAGE_QUALIFIER_LAST_ONE  clvSTORAGE_QUALIFIER_EXTERN

#define cldQUALIFIER_COUNT   (clvQUALIFIER_END - clvQUALIFIER_NONE)
#define cldQUALIFIER_ADDRESS_SPACE_COUNT  (clvQUALIFIER_PRIVATE - clvQUALIFIER_NONE + 1)
#define cldQUALIFIER_ACCESS_COUNT  (cldQUALIFIER_COUNT)
/** KLC, TO optimize later to compute actual count
#define cldQUALIFIER_ACCESS_COUNT  (cldQUALIFIER_COUNT - cldQUALIFIER_ADDRESS_SPACE_COUNT + 1)
**/

typedef gctUINT32 cltQUALIFIER;

gctCONST_STRING
clGetStorageQualifierName(
IN cltQUALIFIER Qualifier
);

gctCONST_STRING
clGetQualifierName(
IN cltQUALIFIER Qualifier
);

typedef enum _cleELEMENT_TYPE
{
    clvTYPE_VOID        = 0,

    clvTYPE_BOOL,
    clvTYPE_CHAR,
    clvTYPE_UCHAR,
    clvTYPE_SHORT,
    clvTYPE_USHORT,
    clvTYPE_INT,
    clvTYPE_UINT,
    clvTYPE_LONG,
    clvTYPE_ULONG,
    clvTYPE_HALF,
    clvTYPE_FLOAT,
    clvTYPE_DOUBLE,
    clvTYPE_QUAD,

    clvTYPE_SAMPLER_T,
    clvTYPE_IMAGE1D_T,
    clvTYPE_IMAGE1D_ARRAY_T,
    clvTYPE_IMAGE1D_BUFFER_T,
    clvTYPE_IMAGE2D_T,
    clvTYPE_IMAGE2D_ARRAY_T,
    clvTYPE_IMAGE3D_T,
    clvTYPE_SAMPLER2D,
    clvTYPE_SAMPLER3D,
    clvTYPE_EVENT_T,
    clvTYPE_STRUCT,
    clvTYPE_UNION,
    clvTYPE_TYPEDEF,
    clvTYPE_ENUM,
    clvTYPE_BOOL_PACKED,
    clvTYPE_CHAR_PACKED,
    clvTYPE_UCHAR_PACKED,
    clvTYPE_SHORT_PACKED,
    clvTYPE_USHORT_PACKED,
    clvTYPE_HALF_PACKED,
    clvTYPE_SIU_GEN,  /* All non generic types should be defined before this */
    clvTYPE_I_GEN,
    clvTYPE_U_GEN,
    clvTYPE_IU_GEN,
    clvTYPE_F_GEN,
    clvTYPE_GEN,
    clvTYPE_GEN_PACKED,
} cltELEMENT_TYPE;

#define cldFirstGenType clvTYPE_SIU_GEN
#define cldLastGenType clvTYPE_GEN
#define cldLowestRankedInteger  clvTYPE_BOOL
#define cldHighestRankedInteger clvTYPE_ULONG
#define cldLowestRankedFloat clvTYPE_HALF
#define cldHighestRankedFloat clvTYPE_QUAD
#define cldFirstPackedType  clvTYPE_BOOL_PACKED
#define cldLastPackedType   clvTYPE_HALF_PACKED
#define cldLowestRankedPackedInteger  clvTYPE_BOOL_PACKED
#define cldHighestRankedPackedInteger clvTYPE_USHORT_PACKED

#define cldArithmeticTypeCount (cldHighestRankedFloat - cldLowestRankedInteger + 1)

#define clmIsElementTypePackedGenType(EType)    \
    ((EType) == clvTYPE_GEN_PACKED)

#define clmIsElementTypeBoolean(EType) \
   (((EType) == clvTYPE_BOOL) || \
    (EType) == clvTYPE_BOOL_PACKED)

#define clmIsElementTypePackedChar(EType) \
   ((EType) == clvTYPE_CHAR_PACKED || \
    (EType) == clvTYPE_UCHAR_PACKED)

#define clmIsElementTypeChar(EType) \
   ((EType) == clvTYPE_CHAR || \
    (EType) == clvTYPE_UCHAR || \
    clmIsElementTypePackedChar(EType))

#define clmIsElementTypePackedInteger(EType) \
 ((EType) >= cldLowestRankedPackedInteger && (EType) <= cldHighestRankedPackedInteger)

#define clmIsElementTypeInteger(EType) \
 (((EType) >= cldLowestRankedInteger && (EType) <= cldHighestRankedInteger) || \
  clmIsElementTypePackedInteger(EType))

#define clmIsElementTypePackedUnsigned(EType) \
   ((EType) == clvTYPE_UCHAR_PACKED || \
    (EType) == clvTYPE_USHORT_PACKED)

#define clmIsElementTypeUnsigned(EType) \
   ((EType) == clvTYPE_UINT || \
    (EType) == clvTYPE_USHORT || \
    (EType) == clvTYPE_UCHAR || \
    (EType) == clvTYPE_ULONG || \
    clmIsElementTypePackedUnsigned(EType))

#define clmIsElementTypePackedSigned(EType) \
   ((EType) == clvTYPE_CHAR_PACKED || \
    (EType) == clvTYPE_SHORT_PACKED)

#define clmIsElementTypeSigned(EType) \
   ((EType) == clvTYPE_INT || \
    (EType) == clvTYPE_SHORT || \
    (EType) == clvTYPE_CHAR || \
    (EType) == clvTYPE_LONG || \
    clmIsElementTypePackedSigned(EType))

#define clmIsElementTypeFloating(EType) \
 (((EType) >= cldLowestRankedFloat && (EType) <= cldHighestRankedFloat) || \
  (EType) == clvTYPE_HALF_PACKED)

#define clmIsElementTypePacked(EType) \
 (((EType) >= cldFirstPackedType && (EType) <= cldLastPackedType) || \
  clmIsElementTypePackedGenType(EType))


#define clmIsElementTypeArithmetic(EType) \
 (((EType) >= cldLowestRankedInteger && (EType) <= cldHighestRankedFloat) || \
  clmIsElementTypePacked(EType))

#define clmIsElementTypeSampler(EType) \
 ((EType) == clvTYPE_SAMPLER_T)

/* Check if it is a image type. */
#define clmIsElementTypeImage1D(EType) \
    ((EType) == clvTYPE_IMAGE1D_T)

#define clmIsElementTypeImage2D(EType) \
    ((EType) == clvTYPE_IMAGE2D_T || \
     (EType) == clvTYPE_IMAGE1D_ARRAY_T)

#define clmIsElementTypeImage3D(EType) \
    ((EType) == clvTYPE_IMAGE3D_T || \
     (EType) == clvTYPE_IMAGE2D_ARRAY_T)

#define clmIsElementTypeImageBuffer(EType) \
    ((EType) == clvTYPE_IMAGE1D_BUFFER_T)

#define clmIsElementTypeImage(EType) \
    (clmIsElementTypeImage1D(EType) || \
     clmIsElementTypeImage2D(EType) || \
     clmIsElementTypeImage3D(EType) || \
     clmIsElementTypeImageBuffer(EType))

#define clmIsElementTypeEvent(EType) \
 ((EType) == clvTYPE_EVENT_T)

#define clmIsElementTypeCompatibleInteger(EType) \
  (clmIsElementTypeInteger(EType) || \
   clmIsElementTypeEvent(EType))

#define clmIsElementTypeHighPrecision(EType) \
    ((EType) == clvTYPE_LONG || \
     (EType) == clvTYPE_ULONG || \
     (EType) == clvTYPE_DOUBLE)

#define clmDECL_IsGenType(Decl) \
  clmDATA_TYPE_IsGenType((Decl)->dataType)

#define clmDECL_IsPackedGenType(Decl) \
  ((Decl)->ptrDscr == gcvNULL && \
   clmDATA_TYPE_IsPackedGenType((Decl)->dataType))

#define clmDECL_IsIntegerType(Decl) \
 ((Decl)->array.numDim == 0  && \
  (Decl)->ptrDscr == gcvNULL && \
  clmIsElementTypeInteger((Decl)->dataType->elementType))

#define clmDECL_IsCompatibleIntegerType(Decl) \
 ((Decl)->array.numDim == 0  && \
  (Decl)->ptrDscr == gcvNULL && \
  clmIsElementTypeCompatibleInteger((Decl)->dataType->elementType))

#define clmDECL_IsFloatingType(Decl) \
 ((Decl)->array.numDim == 0  && \
  (Decl)->ptrDscr == gcvNULL && \
  clmIsElementTypeFloating((Decl)->dataType->elementType))

#define clmDECL_IsArithmeticType(Decl) \
 (clmDECL_IsPointerType(Decl) || \
  ((Decl)->array.numDim == 0  && \
  clmIsElementTypeArithmetic((Decl)->dataType->elementType)))

#define clmDECL_IsPointerType(Decl) \
 (((Decl)->ptrDominant && (Decl)->ptrDscr != gcvNULL) || \
  ((Decl)->array.numDim == 0  && \
   (Decl)->ptrDscr != gcvNULL))

#define clmDECL_IsVectorType(Decl) \
 ((Decl)->array.numDim == 0  &&  \
  (Decl)->ptrDscr == gcvNULL && \
  clmIsElementTypeArithmetic((Decl)->dataType->elementType) && \
  (clmDATA_TYPE_vectorSize_GET((Decl)->dataType) != 0 || \
   clmIsElementTypePackedGenType((Decl)->dataType->elementType)))

#define clmDECL_IsBasicVectorType(Decl) \
 (clmDECL_IsVectorType(Decl) && \
  clmDATA_TYPE_vectorSize_NOCHECK_GET((Decl)->dataType) <= cldBASIC_VECTOR_SIZE)

#define clmDECL_IsExtendedVectorType(Decl) \
 (clmDECL_IsPackedType(Decl) || \
  (clmDECL_IsVectorType(Decl) && \
   clmDATA_TYPE_vectorSize_NOCHECK_GET((Decl)->dataType) > cldBASIC_VECTOR_SIZE))

#define clmDECL_IsGeneralVectorType(Decl) \
  (clmDECL_IsVectorType(Decl) || clmDECL_IsPackedType(Decl))

#define clmDECL_IsVectorPointerType(Decl) \
 ((Decl)->array.numDim == 0  &&  \
  (Decl)->ptrDscr && \
  clmDATA_TYPE_vectorSize_GET((Decl)->dataType) != 0)

#define clmDECL_IsUnderlyingVectorType(Decl) \
    clmDATA_TYPE_IsVector((Decl)->dataType)

#define clmDECL_IsPackedType(Decl) \
 ((Decl)->ptrDscr == gcvNULL && \
  (Decl)->array.numDim == 0  &&  \
  clmIsElementTypePacked((Decl)->dataType->elementType))

#define clmDECL_IsSameVectorType(Decl1, Decl2) \
 ((Decl1)->dataType->elementType == (Decl2)->dataType->elementType \
  && clmDATA_TYPE_vectorSize_GET((Decl1)->dataType) ==  \
     clmDATA_TYPE_vectorSize_GET((Decl2)->dataType))

#define clmDECL_IsSameMatrixType(Decl1, Decl2) \
 ((Decl1)->dataType->elementType == (Decl2)->dataType->elementType \
  &&(clmDATA_TYPE_matrixRowCount_GET((Decl1)->dataType) == \
     clmDATA_TYPE_matrixRowCount_GET((Decl2)->dataType)) \
  && (clmDATA_TYPE_matrixColumnCount_GET((Decl1)->dataType) == \
     clmDATA_TYPE_matrixColumnCount_GET((Decl2)->dataType)))

/* cloIR_POLYNARY_EXPR object. */
typedef enum _clePOLYNARY_EXPR_TYPE
{
    clvPOLYNARY_CONSTRUCT_NONE,
    clvPOLYNARY_CONSTRUCT_SCALAR,
    clvPOLYNARY_CONSTRUCT_VECTOR,
    clvPOLYNARY_CONSTRUCT_MATRIX,
    clvPOLYNARY_CONSTRUCT_STRUCT,
    clvPOLYNARY_CONSTRUCT_ARRAY,
    clvPOLYNARY_FUNC_CALL,
    clvPOLYNARY_BUILT_IN_ASM_CALL,
}
clePOLYNARY_EXPR_TYPE;

struct _clsNAME_SPACE;

struct _clsMATRIX_SIZE;
typedef struct _clsMATRIX_SIZE
{
  gctUINT8  rowCount;      /* vector if rowCount >0 and
                                     columnCount = 0 */
  gctUINT8  columnCount;  /* 0 means not matrix, column dimension */
} clsMATRIX_SIZE;

typedef struct _clsDATA_TYPE
{
    slsDLINK_NODE    node;
    gctINT        type;
    VIR_TypeId    virPrimitiveType;
    cltQUALIFIER    addrSpaceQualifier;
    cltQUALIFIER    accessQualifier;
    cltELEMENT_TYPE    elementType;
    clsMATRIX_SIZE  matrixSize;
    union {
       struct _clsNAME_SPACE *fieldSpace;     /* Only for struct or union*/
       struct _clsNAME *typeDef;    /* for typedef */
       struct _clsNAME *enumerator; /* for enum */
       gctPOINTER generic; /* generic to all */
    } u;
}
clsDATA_TYPE;

typedef struct _clsARRAY
{
    gctINT numDim;          /* Number of dimensions: 0 means not array */
    gctINT length[cldMAX_ARRAY_DIMENSION];  /* length for each dimension */
} clsARRAY;

typedef struct _clsDECL
{
    clsDATA_TYPE *dataType;    /*data type*/
    cltQUALIFIER storageQualifier;  /* storage qualifiers : static/extern/volatile/restrict */
    clsARRAY  array;
    slsSLINK_LIST *ptrDscr;    /* for pointer variables */
    gctBOOL ptrDominant;       /* This is to indicate the declaration is a pointer
                                  when it is an array and ptrDscr is not null */
} clsDECL;

gctUINT
clConvFloatToHalf(
IN gctFLOAT F
);

gctUINT
clPermissibleAlignment(
cloCOMPILER Compiler,
clsDECL *Decl
);

gceSTATUS
cloCOMPILER_CloneDataType(
IN cloCOMPILER Compiler,
IN cltQUALIFIER AccessQualifier,
IN cltQUALIFIER AddrSpaceQualifier,
IN clsDATA_TYPE * Source,
OUT clsDATA_TYPE **DataType
);

gceSTATUS
cloCOMPILER_ClonePtrDscr(
IN cloCOMPILER Compiler,
IN slsSLINK_LIST *Source,
OUT slsSLINK_LIST **Destination
);

gceSTATUS
clsDATA_TYPE_Destroy(
IN cloCOMPILER Compiler,
IN clsDATA_TYPE * DataType
);

gceSTATUS
clsDATA_TYPE_Dump(
IN cloCOMPILER Compiler,
IN clsDATA_TYPE * DataType
);

gctBOOL
clsDECL_IsEqual(
IN clsDECL * Decl1,
IN clsDECL * Decl2
);

gctSIZE_T
clsDECL_GetSize(
IN clsDECL * Decl
);

gctSIZE_T
clsDECL_GetElementSize(
IN clsDECL * Decl
);

gctSIZE_T
clsDECL_GetByteSize(
IN cloCOMPILER Compiler,
IN clsDECL * Decl
);

gctSIZE_T
clsDECL_GetElementByteSize(
IN cloCOMPILER Compiler,
IN clsDECL * Decl,
OUT gctUINT * Alignment,
OUT gctBOOL * Packed
);

gctSIZE_T
clsDECL_GetPointedToByteSize(
IN cloCOMPILER Compiler,
IN clsDECL * Decl
);

#define clGetVectorElementByteSize  clGetElementTypeByteSize

gctSIZE_T
clGetVectorElementByteSize(
IN cloCOMPILER Compiler,
IN cltELEMENT_TYPE ElementType
);

gctSIZE_T
clsDECL_GetFieldOffset(
IN clsDECL * StructDecl,
IN struct _clsNAME * FieldName
);

gctSIZE_T
clGetFieldByteOffset(
    IN cloCOMPILER Compiler,
    IN clsDECL * StructDecl,
    IN struct _clsNAME * FieldName,
    OUT gctUINT * Alignment
    );

gctBOOL
clsDECL_IsAssignableAndComparable(
IN clsDECL * Decl
);

gctBOOL
clsDECL_IsInitializable(
IN clsDECL * Decl
);

gctBOOL
clsDECL_IsAssignableTo(
IN clsDECL * LDecl,
IN clsDECL * RDecl
);

gctBOOL
clsDECL_IsInitializableTo(
IN clsDECL * LDecl,
IN clsDECL * RDecl
);

#define clmPromoteIntToVector(LongValue, Vz, Res)  \
  do {  \
    int i; \
    for(i=0; i < (Vz); i++) { \
      (Res)[i].longValue = (LongValue); \
    } \
  } while (gcvFALSE)

#define clmPromoteScalarConstantToVector(ScalarConstant, Vz, Res)  \
  do {  \
    int i; \
    if(clmDECL_IsFloatingType(&((ScalarConstant)->exprBase.decl))) { \
        for(i=0; i < (Vz); i++) { \
          (Res)[i].floatValue = (ScalarConstant)->values[0].floatValue; \
        } \
    } \
    else { \
        for(i=0; i < (Vz); i++) { \
          (Res)[i].longValue = (ScalarConstant)->values[0].longValue; \
        } \
    } \
  } while (gcvFALSE)

#define clmDATA_TYPE_elementType_GET(d) ((d)->elementType)

#define clmIsElementTypeGenType(EType) \
  ((EType) >= cldFirstGenType && (EType) <= cldLastGenType)

#define clmDATA_TYPE_IsGenType(D) \
  clmIsElementTypeGenType((D)->elementType)

#define clmDATA_TYPE_IsPackedGenType(D) \
  ((D)->elementType == clvTYPE_GEN_PACKED)

#define clmDATA_TYPE_vectorSize_GET(d) ((d)->matrixSize.columnCount? (gctUINT)0 : (d)->matrixSize.rowCount)
#define clmDATA_TYPE_vectorSize_NOCHECK_GET(d) ((d)->matrixSize.rowCount)
#define clmDATA_TYPE_vectorSize_SET(d, s) do \
        { (d)->matrixSize.rowCount = s; \
       (d)->matrixSize.columnCount = 0; \
        } \
        while (gcvFALSE)

#define clmDATA_TYPE_matrixSize_GET(d) ((d)->matrixSize.columnCount)
#define clmDATA_TYPE_matrixRowCount_GET(d) ((d)->matrixSize.rowCount)
#define clmDATA_TYPE_matrixColumnCount_GET(d) ((d)->matrixSize.columnCount)
#define clmDATA_TYPE_matrixSize_SET(d, r, c) do \
        { (d)->matrixSize.rowCount = (r); \
       (d)->matrixSize.columnCount = (c); \
        } \
        while (gcvFALSE)

#define clmDATA_TYPE_matrixRowCount_SET(d, c) (d)->matrixSize.rowCount = (c)
#define clmDATA_TYPE_matrixColumnCount_SET(d, c) (d)->matrixSize.columnCount = (c)

#define clmDATA_TYPE_IsVoid(DataType) \
    ((DataType)->elementType == clvTYPE_VOID)

#define clmDATA_TYPE_IsStruct(DataType) \
    ((DataType)->elementType == clvTYPE_STRUCT)

#define clmDATA_TYPE_IsUnion(DataType) \
    ((DataType)->elementType == clvTYPE_UNION)

#define clmDATA_TYPE_IsStructOrUnion(DataType) \
    ((DataType)->elementType == clvTYPE_STRUCT || \
     (DataType)->elementType == clvTYPE_UNION)

#define clmDATA_TYPE_IsImage(DataType) \
    clmIsElementTypeImage((DataType)->elementType)

#define clmDATA_TYPE_IsSampler(DataType) \
    ((DataType)->type == T_SAMPLER_T)

#define clmDATA_TYPE_IsHighPrecision(DataType) \
    clmIsElementTypeHighPrecision(clmDATA_TYPE_elementType_GET(DataType))

#define clmDATA_TYPE_IsVector(DataType) \
 (clmIsElementTypeArithmetic((DataType)->elementType) && \
  clmDATA_TYPE_vectorSize_GET(DataType) != 0)

#define clmGetArrayElementCount(Array, dim, ElementCount) \
    do { \
    if((Array).numDim > (dim)) { \
       int i; \
       (ElementCount) = (Array).length[dim]; \
       for(i = (dim) + 1; i < (Array).numDim; i++) { \
          (ElementCount) *= (Array).length[i]; \
       } \
    } \
    else (ElementCount) = 0; \
    } while(gcvFALSE)

#define clmDECL_Initialize(Decl, DataType, Array, PtrDscr, PtrDominant, StorageQualifier) \
    do { \
       if((Array) != gcvNULL) { \
          (Decl)->array= *(Array); \
           } \
       else { \
         (Decl)->array.numDim = 0; \
         (Decl)->array.length[0] = 0; \
       } \
       (Decl)->dataType = (DataType); \
       (Decl)->storageQualifier = (StorageQualifier); \
       (Decl)->ptrDscr = (PtrDscr); \
       (Decl)->ptrDominant = (PtrDominant); \
    } while(gcvFALSE)

#define clmDECL_IsVoid(Decl) \
    (!clmDECL_IsPointerType(Decl) && \
     clmDATA_TYPE_IsVoid((Decl)->dataType))

#define clmDECL_IsBVecOrIVecOrVec(Decl) \
    clmDECL_IsVectorType(Decl)

#define clmDECL_IsMat(Decl) \
    ((Decl)->array.numDim == 0 && \
         (Decl)->ptrDscr == gcvNULL && \
         (Decl)->dataType->matrixSize.columnCount != 0)

#define clmDECL_IsSquareMat(Decl) \
    ((Decl)->dataType->matrixSize.columnCount != 0 && \
         ((Decl)->dataType->matrixSize.rowCount == (Decl)->dataType->matrixSize.columnCount))

#define clmDECL_IsArray(Decl) \
    (!(Decl)->ptrDominant && (Decl)->array.numDim != 0)

#define clmDECL_IsArrayOfPointers(Decl) \
    (clmDECL_IsArray(Decl) && (Decl)->ptrDscr != gcvNULL)

/* This is different from clmDECL_IsArray() in that the declared array may
   not be primary and/or being pointed to */
#define clmDECL_IsUnderlyingArray(Decl) \
    ((Decl)->array.numDim != 0)

#define clmDECL_IsAggregateType(Decl) \
    (clmDECL_IsArray(Decl) || \
      clmDECL_IsStructOrUnion(Decl))

#define clmDECL_IsScalar(Decl) \
    (clmDECL_IsPointerType(Decl) || \
     (clmDECL_IsArithmeticType(Decl) && \
      (Decl)->dataType->matrixSize.rowCount == 0 && \
      !clmDECL_IsPackedGenType(Decl)))

#define clmDECL_IsElementScalar(Decl) \
  ((clmIsElementTypeArithmetic((Decl)->dataType->elementType) && \
   (Decl)->dataType->matrixSize.rowCount == 0 && \
   !clmDECL_IsPackedGenType(Decl)) || \
   clmIsElementTypeSampler((Decl)->dataType->elementType) || \
   clmIsElementTypeEvent((Decl)->dataType->elementType))

#define clmDECL_IsBool(Decl) \
    (clmIsElementTypeBoolean((Decl)->dataType->elementType) && \
        (Decl)->dataType->matrixSize.rowCount == 0 && \
                (Decl)->ptrDscr == gcvNULL && \
        (Decl)->array.numDim == 0)

#define clmDECL_IsScalarInteger(Decl) \
  (clmIsElementTypeInteger((Decl)->dataType->elementType) && \
   clmDECL_IsScalar(Decl))

#define clmDECL_IsInt(Decl) clmDECL_IsScalarInteger(Decl)

#define clmDECL_IsScalarFloating(Decl) \
  (clmIsElementTypeFloating((Decl)->dataType->elementType) && \
   clmDECL_IsScalar(Decl))

#define clmDECL_IsFloat(Decl) clmDECL_IsScalarFloating(Decl)

#define clmDECL_IsBVec(Decl) \
    (clmIsElementTypeBoolean((Decl)->dataType->elementType) && \
         && clmDATA_TYPE_vectorSize_GET((Decl)->dataType) !=0 && \
                (Decl)->ptrDscr == gcvNULL && \
        (Decl)->array.numDim == 0)

#define clmDECL_IsIVec(Decl) \
    (clmIsElementTypeInteger((Decl)->dataType->elementType) && \
         clmDATA_TYPE_vectorSize_GET((Decl)->dataType) !=0 && \
                (Decl)->ptrDscr == gcvNULL && \
        (Decl)->array.numDim == 0)

#define clmDECL_IsPackedIVec(Decl) \
    (clmIsElementTypePackedInteger((Decl)->dataType->elementType) && \
         clmDATA_TYPE_vectorSize_GET((Decl)->dataType) !=0 && \
                (Decl)->ptrDscr == gcvNULL && \
        (Decl)->array.numDim == 0)

#define clmDECL_IsVec(Decl) \
        (clmIsElementTypeFloating((Decl)->dataType->elementType) && \
          clmDATA_TYPE_vectorSize_GET((Decl)->dataType) !=0  && \
                (Decl)->ptrDscr == gcvNULL && \
         (Decl)->array.numDim == 0)

#define clmDECL_IsPackedVec(Decl) \
        (clmIsElementTypePackedFloating((Decl)->dataType->elementType) && \
          clmDATA_TYPE_vectorSize_GET((Decl)->dataType) !=0  && \
                (Decl)->ptrDscr == gcvNULL && \
         (Decl)->array.numDim == 0)

#define clmDECL_IsBoolOrBVec(Decl) \
    (clmIsElementTypeBoolean((Decl)->dataType->elementType) && \
         clmDATA_TYPE_matrixSize_GET((Decl)->dataType) == 0 && \
                (Decl)->ptrDscr == gcvNULL && \
         (Decl)->array.numDim == 0)

#define clmDECL_IsIntOrIVec(Decl) \
    (clmIsElementTypeInteger((Decl)->dataType->elementType) && \
         clmDATA_TYPE_matrixSize_GET((Decl)->dataType) == 0  && \
                (Decl)->ptrDscr == gcvNULL && \
        (Decl)->array.numDim == 0)

#define clmDECL_IsFloatOrVec(Decl) \
        (clmIsElementTypeFloating((Decl)->dataType->elementType) && \
         clmDATA_TYPE_matrixSize_GET((Decl)->dataType) == 0 && \
                (Decl)->ptrDscr == gcvNULL && \
        (Decl)->array.numDim == 0)

#define clmDECL_IsFloatOrVecOrMat(Decl) \
    clmDECL_IsFloatingType(Decl)

#define clmDECL_IsVecOrMat(Decl) \
        (clmIsElementTypeFloating((Decl)->dataType->elementType) && \
        ((Decl)->dataType->matrixSize.rowCount != 0 || \
                 (Decl)->dataType->matrixSize.columnCount != 0) && \
                (Decl)->ptrDscr == gcvNULL && \
        (Decl)->array.numDim == 0)

#define clmDECL_IsStruct(Decl) \
    ((Decl)->dataType->elementType == clvTYPE_STRUCT && \
                (Decl)->ptrDscr == gcvNULL && \
        (Decl)->array.numDim == 0)

#define clmDECL_IsUnion(Decl) \
    ((Decl)->dataType->elementType == clvTYPE_UNION && \
                (Decl)->ptrDscr == gcvNULL && \
        (Decl)->array.numDim == 0)


#define clmDECL_IsUnderlyingStructOrUnion(Decl) \
    (((Decl)->dataType->elementType == clvTYPE_STRUCT || \
     (Decl)->dataType->elementType == clvTYPE_UNION) && \
     !clmDECL_IsPointerType(Decl))

#define clmDECL_IsStructOrUnion(Decl) \
    (((Decl)->dataType->elementType == clvTYPE_STRUCT || \
     (Decl)->dataType->elementType == clvTYPE_UNION) && \
                (Decl)->ptrDscr == gcvNULL && \
        (Decl)->array.numDim == 0)

#define clmDECL_IsImage(Decl) \
    (clmDATA_TYPE_IsImage((Decl)->dataType) && \
     (Decl)->ptrDscr == gcvNULL && \
     (Decl)->array.numDim == 0)

#define clmDECL_IsSampler(Decl) \
    (clmDATA_TYPE_IsSampler((Decl)->dataType) && \
     (Decl)->ptrDscr == gcvNULL && \
     (Decl)->array.numDim == 0)

#define clmDECL_IsTypeDef(Decl) \
    ((Decl)->dataType->elementType == clvTYPE_TYPEDEF)

#define clmNAME_InitializeTypeInfo(Name) \
    do { \
        (Name)->u.typeInfo.specifiedAttr = 0;     \
        (Name)->u.typeInfo.packed    = gcvFALSE;  \
        (Name)->u.typeInfo.alignment = 0;         \
        (Name)->u.typeInfo.needMemory = gcvFALSE; \
        (Name)->u.typeInfo.typeNameOffset = -1;   \
    } while (gcvFALSE)

/* Name and Name space. */
typedef enum _cleNAME_TYPE
{
    clvVARIABLE_NAME,
    clvPARAMETER_NAME,
    clvFUNC_NAME,
    clvKERNEL_FUNC_NAME,
    clvTYPE_NAME,
    clvLABEL_NAME,
    clvSTRUCT_NAME,
    clvUNION_NAME,
    clvENUM_NAME,
    clvFIELD_NAME,
    clvENUM_TAG_NAME,
}
cleNAME_TYPE;

struct _clsNAME_SPACE;
struct _cloIR_SET;
struct _cloIR_CONSTANT;
struct _cloIR_POLYNARY_EXPR;
struct _clsLOGICAL_REG;
struct _cloIR_LABEL;

struct _clsTYPE_QUALIFIER;

typedef struct _clsTYPE_QUALIFIER
{
    slsSLINK_NODE node;
    gctINT type;
    cltQUALIFIER  qualifier;
}
clsTYPE_QUALIFIER;

struct _clsTYPE_QUALIFIER;

typedef struct _clsSAMPLER_TYPES
{
    slsSLINK_NODE node;
    struct _clsNAME *member;
    gcUNIFORM imageSampler;
}
clsSAMPLER_TYPES;

typedef struct _clsENUMERATOR
{
    slsSLINK_NODE node;
    struct _clsNAME *member;
}
clsENUMERATOR;

/* attributes */
typedef enum _cleATTR_FLAGS
{
    clvATTR_PACKED     = 0x1,
    clvATTR_ALIGNED = 0x2,
    clvATTR_ENDIAN = 0x4,
    clvATTR_VEC_TYPE_HINT = 0x8,
    clvATTR_REQD_WORK_GROUP_SIZE = 0x10,
    clvATTR_WORK_GROUP_SIZE_HINT = 0x20,
    clvATTR_ALWAYS_INLINE = 0x40,
    clvATTR_KERNEL_SCALE_HINT = 0x80,
}
cleATTR_FLAGS;

typedef gctUINT32 cltATTR_FLAGS;

typedef struct _clsATTRIBUTE
{
    cltATTR_FLAGS  specifiedAttr;
    gctUINT16 alignment;
    gctBOOL  packed;
    gctBOOL  alwaysInline;
    gctBOOL  hostEndian; /*default 0: follow device's endian NOT host */
    gctINT vecTypeHint; /*default is int*/
    gctSIZE_T reqdWorkGroupSize[3]; /*default {0,0,0} */
    gctSIZE_T workGroupSizeHint[3]; /*default {0,0,0} */
    gctSIZE_T kernelScaleHint[3];   /*default {1,1,1} */
} clsATTRIBUTE;

struct _cloIR_EXPR;

typedef struct _clsNAME
{
    slsDLINK_NODE    node;
    struct _clsNAME_SPACE *    mySpace;
    gctUINT         fileNo;
    gctUINT         lineNo;
    gctUINT         stringNo;
    cleNAME_TYPE    type;
    struct _clsDECL decl;
    struct _clsNAME *derivedType;
    cltPOOL_STRING  symbol;
    gctBOOL         isBuiltin;
    cleEXTENSION    extension;
    gctUINT16       die;
    union {
      struct {
        cltATTR_FLAGS  specifiedAttr;
        union {
           struct _cloIR_CONSTANT *constant; /* Only for constant variables */
           struct _clsNAME *aliasName; /* for parameter */
        } u;
        struct _clsNAME *alias;  /* for optimization: alias to memory location with aliasOffset */
        gctINT aliasOffset;
        gctINT padding;  /* temporary variable for data alignment of struct*/
        struct _clsDECL effectiveDecl; /* for parameter, the effective declaration with gen_type */
        slsSLINK_LIST *samplers; /* for images and samplers only */
        cleBUILTIN_VARIABLE variableType;
        gctBOOL  hostEndian; /*default 0: follow device's endian NOT host */
        gctBOOL  isAddressed; /* has it been addressed through an '&' operator*/
        gctBOOL  isConvertibleType; /* for parameter names in builtin functions, that the corresponding
                                          argument type can be converted to this type for matching of the
                                          builtin function prototype*/
        gctBOOL isDirty; /* Variable is dirty, if it has been written into (location pointed by it)
                                or its value has been changed */
        gctBOOL  hasGenType;
        gctBOOL  allocated;  /* variable's memory has been allocated */
        gctBOOL  inInterfaceBlock;  /* variable has put into interface block */
        gctBOOL  isUnnamedConstant;  /* variable is an unnamed constant */
        gctBOOL  isInitializedWithExtendedVectorConstant;  /* variable is initialized with constant */
        gctBOOL  indirectlyAddressed;  /* variable whose elements are indirectly addressed */
      } variableInfo;
      struct { /* enum, struct, union or typedefs */
        cltATTR_FLAGS  specifiedAttr;
        gctUINT16 alignment;
        gctBOOL packed;
        gctBOOL needMemory;
        gctINT  typeNameOffset;
      } typeInfo;
      struct {
        struct _clsNAME_SPACE *localSpace;
        struct _cloIR_SET *funcBody;
        gctINT vecTypeHint;
        gctSIZE_T reqdWorkGroupSize[3];
        gctSIZE_T workGroupSizeHint[3];
        gctSIZE_T kernelScaleHint[3];
        gctSIZE_T localMemorySize;
        gctUINT  refCount;
        gctBOOL  needLocalMemory;
        gctBOOL  isIntrinsicCall;
        gceINTRINSICS_KIND  intrinsicKind;
        cltPOOL_STRING  mangledName;
        gctBOOL  isFuncDef;
        gctBOOL  isInline;
        gctBOOL  hasWriteArg;
        gctBOOL  passArgByRef;
        gctBOOL  hasVarArg;
        gctBOOL  hasGenType;
      } funcInfo;    /* Only for function names */
      struct {
        struct _cloIR_LABEL *label;
        gctBOOL isReferenced;
      } labelInfo;
    } u;
    struct {
      gctUINT16 alignment;
      gctBOOL  packed;
      union {
        struct {
          gctUINT  logicalRegCount;
          struct _clsLOGICAL_REG *logicalRegs;
          gctINT  memoryOffset;

          /* Only for the function/parameter names */
          gctBOOL isKernel;  /* belong to kernel function if true */
          union {
            gcFUNCTION    function;
            gcKERNEL_FUNCTION kernelFunction;
          }u;
        } variable;
        clsDECL typeDef;
        slsSLINK_LIST *enumerator;   /* for enum */
      } u;
    } context;
}
clsNAME;

#define clmNAME_VariableMemoryOffset_NOCHECK_GET(Name) \
     (Name)->context.u.variable.memoryOffset

#define clmNAME_VariableMemoryOffset_SET(Name, Offset) \
     (Name)->context.u.variable.memoryOffset = (Offset)

#define clmNAME_VariableHasMemoryOffset(Name) \
     (clmNAME_VariableMemoryOffset_NOCHECK_GET(Name) >=  0)

#define clmNAME_VariableMemoryOffset_GET(Name) \
     (clmNAME_VariableHasMemoryOffset(Name) \
      ? clmNAME_VariableMemoryOffset_NOCHECK_GET(Name) \
      : 0)

#define clmNAME_DerivedType_GET(Name) \
     ((Name)->derivedType)

gceSTATUS
cloNAME_BindFuncBody(
IN cloCOMPILER Compiler,
IN clsNAME * FuncName,
IN struct _cloIR_SET * FuncBody
);

gceSTATUS
cloNAME_GetParamCount(
IN cloCOMPILER Compiler,
IN clsNAME * FuncName,
OUT gctUINT * ParamCount
);

gceSTATUS
clsNAME_BindAliasParamNames(
IN cloCOMPILER Compiler,
IN OUT clsNAME * FuncDefName,
IN clsNAME * FuncDeclName
);

gceSTATUS
clsNAME_SetVariableAddressed(
IN cloCOMPILER Compiler,
IN clsNAME *Name
);

gceSTATUS
clsNAME_Dump(
IN cloCOMPILER Compiler,
IN clsNAME * Name
);

gceSTATUS
cloCOMPILER_RegisterBuiltinVariable(
IN cloCOMPILER Compiler,
IN gctINT VariableNum,
IN clsNAME *Unnamed
);

clsNAME *
cloCOMPILER_GetBuiltinVariable(
IN cloCOMPILER Compiler,
IN gctINT VariableNum
);

typedef struct _clsNAME_SPACE
{
slsDLINK_NODE    node;
struct _clsNAME *scopeName;
cltPOOL_STRING symbol;
struct _clsNAME_SPACE *    parent;
slsDLINK_LIST    names;
slsDLINK_LIST    subSpaces;
gctUINT16 die;
}
clsNAME_SPACE;

gceSTATUS
clsNAME_SPACE_ReleaseName(
IN cloCOMPILER Compiler,
IN clsNAME_SPACE * NameSpace,
IN clsNAME *Name
);

gceSTATUS
clsNAME_SPACE_Construct(
IN cloCOMPILER Compiler,
IN clsNAME_SPACE * Parent,
OUT clsNAME_SPACE ** NameSpace
);

gceSTATUS
clsNAME_SPACE_Destroy(
IN cloCOMPILER Compiler,
IN clsNAME_SPACE * NameSpace
);

gceSTATUS
clsNAME_SPACE_Dump(
IN cloCOMPILER Compiler,
IN clsNAME_SPACE * NameSpace
);

gceSTATUS
clsNAME_SPACE_Search(
IN cloCOMPILER Compiler,
IN clsNAME_SPACE * NameSpace,
IN cltPOOL_STRING Symbol,
IN gctBOOL Recursive,
OUT clsNAME ** Name
);

gceSTATUS
clsNAME_SPACE_CheckNewFuncName(
IN cloCOMPILER Compiler,
IN clsNAME_SPACE * NameSpace,
IN clsNAME * FuncName,
OUT clsNAME ** FirstFuncName
);

gceSTATUS
clsNAME_SPACE_BindFuncName(
IN cloCOMPILER Compiler,
IN clsNAME_SPACE * NameSpace,
IN OUT struct _cloIR_POLYNARY_EXPR * PolynaryExpr
);

clsNAME_SPACE *
cloCOMPILER_GetBuiltinSpace(
IN cloCOMPILER Compiler
);

clsNAME_SPACE *
cloCOMPILER_GetCurrentSpace(
IN cloCOMPILER Compiler
);

clsNAME_SPACE *
cloCOMPILER_GetGlobalSpace(
IN cloCOMPILER Compiler
);

clsNAME_SPACE *
cloCOMPILER_GetUnnamedSpace(
IN cloCOMPILER Compiler
);

gctBOOL
cloCOMPILER_GenDebugInfo(
IN cloCOMPILER Compiler
);

void
cloCOMPILER_ChangeUniformDebugInfo(
    IN cloCOMPILER Compiler,
    gctUINT tmpStart,
    gctUINT tmpEnd,
    gctUINT uniformIdx
);

void
cloCOMPILER_SetCollectDIE(
IN cloCOMPILER Compiler,
gctBOOL collect
);

gctUINT16
cloCOMPILER_AddDIEWithName(
IN cloCOMPILER Compiler,
IN clsNAME * Variable
);

gctUINT16
cloCOMPILER_AddDIE(
IN cloCOMPILER Compiler,
IN VSC_DIE_TAG Tag,
IN gctUINT16 Parent,
IN gctCONST_STRING Name,
IN gctUINT FileNo,
IN gctUINT LineNo,
IN gctUINT EndLineNo,
IN gctUINT ColNo
);

void
cloCOMPILER_DumpDIETree(
IN cloCOMPILER Compiler
);

void
cloCOMPILER_SetDIESourceLoc(
IN cloCOMPILER Compiler,
IN gctUINT16 Id,
IN gctUINT FileNo,
IN gctUINT LineNo,
IN gctUINT EndLineNo,
IN gctUINT ColNo
);

void
cloCOMPILER_SetDIELogicalReg(
IN cloCOMPILER Compiler,
IN gctUINT16 Id,
IN gctUINT32 regIndex,
IN gctUINT num,
IN gctUINT mask
);

void
cloCOMPILER_SetStructDIELogicalReg(
IN cloCOMPILER Compiler,
IN gctUINT16 ParentId,
IN gctCONST_STRING Symbol,
IN gctUINT32 regIndex,
IN gctUINT num,
IN gctUINT mask
);

gctBOOL
cloCOMPILER_InGlobalSpace(
IN cloCOMPILER Compiler
);
gceSTATUS
clsNAME_SPACE_CreateName(
IN cloCOMPILER Compiler,
IN clsNAME_SPACE *NameSpace,
IN gctUINT LineNo,
IN gctUINT StringNo,
IN cleNAME_TYPE Type,
IN clsDECL *Decl,
IN cltPOOL_STRING Symbol,
IN slsSLINK_LIST *PtrDscr,
IN gctBOOL IsBuiltin,
IN cleEXTENSION Extension,
OUT clsNAME **Name
);

gceSTATUS
cloCOMPILER_CreateName(
IN cloCOMPILER Compiler,
IN gctUINT LineNo,
IN gctUINT StringNo,
IN cleNAME_TYPE Type,
IN clsDECL *Decl,
IN cltPOOL_STRING Symbol,
IN slsSLINK_LIST *PtrDscr,
IN cleEXTENSION Extension,
OUT clsNAME **Name
);

gceSTATUS
cloCOMPILER_SearchName(
IN cloCOMPILER Compiler,
IN cltPOOL_STRING Symbol,
IN gctBOOL Recursive,
OUT clsNAME ** Name
);

gceSTATUS
cloCOMPILER_CheckNewFuncName(
IN cloCOMPILER Compiler,
IN clsNAME * FuncName,
OUT clsNAME ** FirstFuncName
);

gceSTATUS
cloCOMPILER_CreateNameSpace(
IN cloCOMPILER Compiler,
OUT clsNAME_SPACE ** NameSpace
);

gceSTATUS
cloCOMPILER_PopCurrentNameSpace(
IN cloCOMPILER Compiler,
OUT clsNAME_SPACE ** PrevNameSpace
);

gceSTATUS
cloCOMPILER_PushUnnamedSpace(
IN cloCOMPILER Compiler,
OUT clsNAME_SPACE ** UnnamedSpace
);

gceSTATUS
cloCOMPILER_AtGlobalNameSpace(
IN cloCOMPILER Compiler,
OUT gctBOOL * AtGlobalNameSpace
);

gctUINT
cloCOMPILER_FindTopKernelFunc(
IN cloCOMPILER Compiler,
OUT clsNAME **TopKernelFunc
);

gceSTATUS
cloCOMPILER_AllocateVariableMemory(
cloCOMPILER Compiler,
clsNAME *Variable
);

/* Type of IR objects. */
typedef enum _cleIR_OBJECT_TYPE
{
    clvIR_UNKNOWN     = 0,
    clvIR_SET    = gcmCC('S','E','T','\0'),
    clvIR_ITERATION    = gcmCC('I','T','E','R'),
    clvIR_JUMP    = gcmCC('J','U','M','P'),
    clvIR_LABEL    = gcmCC('L','A','B','\0'),
    clvIR_VARIABLE    = gcmCC('V','A','R','\0'),
    clvIR_CONSTANT    = gcmCC('C','N','S','T'),
    clvIR_UNARY_EXPR = gcmCC('U','N','R','Y'),
    clvIR_BINARY_EXPR = gcmCC('B','N','R','Y'),
    clvIR_SELECTION    = gcmCC('S','E','L','T'),
    clvIR_SWITCH    = gcmCC('S','W','I','T'),
    clvIR_POLYNARY_EXPR = gcmCC('P','O','L','Y'),
    clvIR_TYPECAST_ARGS = gcmCC('C','A','S','T')
}
cleIR_OBJECT_TYPE;

/* cloIR_BASE object. */
struct _clsVTAB;
typedef struct _clsVTAB *cltVPTR;

struct _cloIR_BASE
{
    slsDLINK_NODE    node;
    cltVPTR        vptr;
    gctUINT        lineNo;
    gctUINT        stringNo;
    gctUINT        endLineNo;
};

typedef struct _cloIR_BASE *cloIR_BASE;

typedef gceSTATUS
(* cltDESTROY_FUNC_PTR)(
  IN cloCOMPILER Compiler,
  IN cloIR_BASE This
);

typedef gceSTATUS
(* cltDUMP_FUNC_PTR)(
  IN cloCOMPILER Compiler,
  IN cloIR_BASE This
);

struct _clsVISITOR;

typedef gceSTATUS
(* cltACCEPT_FUNC_PTR)(
  IN cloCOMPILER Compiler,
  IN cloIR_BASE This,
  IN struct _clsVISITOR * Visitor,
  IN OUT gctPOINTER Parameters
);

typedef struct _clsVTAB
{
  cleIR_OBJECT_TYPE    type;
  cltDESTROY_FUNC_PTR    destroy;
  cltDUMP_FUNC_PTR    dump;
  cltACCEPT_FUNC_PTR    accept;
}
clsVTAB;

#define cloIR_BASE_Initialize(base, finalVPtr, finalLineNo, finalStringNo) \
    do { \
        (base)->vptr        = (finalVPtr); \
        (base)->lineNo        = (finalLineNo); \
        (base)->stringNo    = (finalStringNo); \
        (base)->endLineNo   = (finalLineNo);\
    } while (gcvFALSE)

#if gcdDEBUG
#define clmVERIFY_IR_OBJECT(obj, objType) \
    do { \
        if (((obj) == gcvNULL) || (((cloIR_BASE)(obj))->vptr->type != (objType))) { \
      gcmASSERT(((obj) != gcvNULL) && (((cloIR_BASE)(obj))->vptr->type == (objType))); \
      return gcvSTATUS_INVALID_OBJECT; \
    } \
    } while (gcvFALSE)
#else
#define clmVERIFY_IR_OBJECT(obj, objType) do {} while (gcvFALSE)
#endif

#define cloIR_OBJECT_GetType(obj) \
    ((obj)->vptr->type)

#define cloIR_OBJECT_Destroy(compiler, obj) \
    ((obj)->vptr->destroy((compiler), (obj)))

#define cloIR_OBJECT_Dump(compiler, obj) \
    ((obj)->vptr->dump((compiler), (obj)))

#define cloIR_OBJECT_Accept(compiler, obj, visitor, parameters) \
    ((obj)->vptr->accept((compiler), (obj), (visitor), (parameters)))

/* cloIR_SET object. */
typedef enum _cleSET_TYPE
{
    clvDECL_SET,
    clvSTATEMENT_SET,
    clvEXPR_SET
}
cleSET_TYPE;

struct _cloIR_SET
{
    struct _cloIR_BASE  base;
    cleSET_TYPE         type;
    slsDLINK_LIST       members;
    clsNAME *           funcName;    /* Only for the function definition */
};

typedef struct _cloIR_SET *cloIR_SET;

/* cloIR_EXPR object. */
struct _cloIR_EXPR
{
    struct _cloIR_BASE base;
    clsDECL decl;
};

typedef struct _cloIR_EXPR *cloIR_EXPR;

gceSTATUS
cloIR_SET_Destroy(
IN cloCOMPILER Compiler,
IN cloIR_BASE This
);

gceSTATUS
cloIR_SET_Empty(
IN cloCOMPILER Compiler,
IN cloIR_BASE This
);

gceSTATUS
cloIR_SET_Construct(
    IN cloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN cleSET_TYPE Type,
    OUT cloIR_SET * Set
    );

gceSTATUS
cloIR_SET_AddMember(
    IN cloCOMPILER Compiler,
    IN cloIR_SET Set,
    IN cloIR_BASE Member
    );

gceSTATUS
cloIR_SET_GetMemberCount(
    IN cloCOMPILER Compiler,
    IN cloIR_SET Set,
    OUT gctUINT * MemberCount
    );

gceSTATUS
cloIR_SET_GetMember(
    IN cloCOMPILER Compiler,
    IN cloIR_SET Set,
    gctUINT I,
    cloIR_BASE *Member
    );

gctBOOL
cloIR_SET_IsEmpty(
    IN cloCOMPILER Compiler,
    IN cloIR_SET Set
    );

gceSTATUS
cloIR_SET_IsRoot(
    IN cloCOMPILER Compiler,
    IN cloIR_SET Set,
    OUT gctBOOL * IsRoot
    );

gceSTATUS
cloCOMPILER_AddExternalDecl(
    IN cloCOMPILER Compiler,
    IN cloIR_BASE ExternalDecl
    );

gceSTATUS
cloCOMPILER_AddStatementPlaceHolder(
IN cloCOMPILER Compiler,
IN clsNAME *FuncName
);

#define cloIR_EXPR_Initialize(expr, finalVPtr, finalLineNo, finalStringNo, exprDecl) \
    do { \
      cloIR_BASE_Initialize(&(expr)->base, (finalVPtr), (finalLineNo), (finalStringNo)); \
      (expr)->decl = (exprDecl); \
    } while (gcvFALSE)

#define clmIR_EXPR_IsUnaryType(expr, unaryType) \
    (cloIR_OBJECT_GetType(&((expr)->base)) == clvIR_UNARY_EXPR && \
        ((cloIR_UNARY_EXPR) &((expr)->base))->type == (unaryType))

#define clmIR_EXPR_IsBinaryType(expr, binaryType) \
    (cloIR_OBJECT_GetType(&((expr)->base)) == clvIR_BINARY_EXPR && \
        ((cloIR_BINARY_EXPR) &((expr)->base))->type == (binaryType))

gceSTATUS
cloCOMPILER_CreateDecl(
IN cloCOMPILER Compiler,
IN gctINT TokenType,
IN gctPOINTER Generic,
IN cltQUALIFIER AccessQualifier,
IN cltQUALIFIER AddrSpaceQualifier,
OUT clsDECL *Decl
);

gceSTATUS
cloCOMPILER_CreateArrayDecl(
IN cloCOMPILER Compiler,
IN clsDATA_TYPE *ElementDataType,
IN clsARRAY *Array,
IN slsSLINK_LIST *PtrDscr,
OUT clsDECL *Decl
);

gceSTATUS
cloCOMPILER_CreateElementDecl(
IN cloCOMPILER Compiler,
IN clsDECL * CompoundDecl,
OUT clsDECL * Decl
);

gceSTATUS
cloCOMPILER_CloneDataType(
IN cloCOMPILER Compiler,
IN cltQUALIFIER AccessQualifier,
IN cltQUALIFIER AddrSpaceQualifier,
IN clsDATA_TYPE * Source,
OUT clsDATA_TYPE **DataType
);

gceSTATUS
cloCOMPILER_CloneDecl(
IN cloCOMPILER Compiler,
IN cltQUALIFIER AccessQualifier,
IN cltQUALIFIER AddrSpaceQualifier,
IN clsDECL *Source,
OUT clsDECL *Decl
);

gceSTATUS
cloCOMPILER_CreateDataType(
IN cloCOMPILER Compiler,
IN gctINT TokenType,
IN gctPOINTER Generic,
IN cltQUALIFIER AccessQualifier,
IN cltQUALIFIER AddrSpaceQualifier,
OUT clsDATA_TYPE **DataType
);

/* cloIR_ITERATION object. */
typedef enum _cleITERATION_TYPE
{
    clvFOR,
    clvWHILE,
    clvDO_WHILE
}
cleITERATION_TYPE;

struct _cloIR_ITERATION
{
    struct _cloIR_BASE    base;
    cleITERATION_TYPE    type;
    cloIR_EXPR        condExpr;
    cloIR_BASE        loopBody;
    clsNAME_SPACE *        forSpace;
    cloIR_BASE        forInitStatement;
    cloIR_EXPR        forRestExpr;
};

typedef struct _cloIR_ITERATION *cloIR_ITERATION;

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
    );

/* cloIR_JUMP object. */
typedef enum _cleJUMP_TYPE
{
    clvCONTINUE,
    clvBREAK,
    clvRETURN,
    clvGOTO
}
cleJUMP_TYPE;

gctCONST_STRING
clGetIRJumpTypeName(
    IN cleJUMP_TYPE JumpType
    );

struct _cloIR_JUMP
{
    struct _cloIR_BASE    base;
    cleJUMP_TYPE        type;
    union {
      cloIR_EXPR    returnExpr;
      clsNAME    *label;
    } u;
    clsNAME_SPACE * nameSpace;
};

typedef struct _cloIR_JUMP *    cloIR_JUMP;

gceSTATUS
cloIR_JUMP_Construct(
    IN cloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN cleJUMP_TYPE Type,
    IN cloIR_EXPR ReturnExpr,
    OUT cloIR_JUMP * Jump
    );

gceSTATUS
cloIR_GOTO_Construct(
    IN cloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN clsNAME *GotoLabel,
    OUT cloIR_JUMP *Goto
    );

/* cloIR_JUMP object. */
typedef enum _cleLABEL_TYPE
{
    clvNAMED,
    clvCASE,
    clvDEFAULT
}
cleLABEL_TYPE;

/* cloIR_LABEL object. */
struct _cloIR_LABEL
{
    struct _cloIR_BASE base;
    cleLABEL_TYPE type;
    union {
      clsNAME *name;
      struct _cloIR_LABEL *nextCase;
    } u;
    gctLABEL programCounter;
    struct _cloIR_CONSTANT *caseValue;
    gctUINT16 caseNumber;
};

typedef struct _cloIR_LABEL *cloIR_LABEL;

void
cloIR_LABEL_Initialize(
IN gctUINT LineNo,
IN gctUINT StringNo,
IN OUT cloIR_LABEL Label
);

gceSTATUS
cloIR_LABEL_Construct(
    IN cloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    OUT cloIR_LABEL *Label
    );

/* cloIR_VARIABLE object. */
struct _cloIR_VARIABLE
{
    struct _cloIR_EXPR    exprBase;
    clsNAME *        name;
};

typedef struct _cloIR_VARIABLE *cloIR_VARIABLE;

cloIR_VARIABLE
cloCOMPILER_GetParamChainVariable(
IN cloCOMPILER Compiler,
IN gctINT LineNo,
IN gctINT StringNo,
IN gctINT VariableNum
);

gceSTATUS
cloIR_VARIABLE_Construct(
    IN cloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN clsNAME * Name,
    OUT cloIR_VARIABLE * Variable
    );

/* cloIR_CONSTANT object. */
typedef union _cluCONSTANT_VALUE
{
    gctCHAR     charValue;
    gctBOOL     boolValue;
    gctINT32    intValue;
    gctINT64    longValue;
    gctUINT32   uintValue;
    gctUINT64   ulongValue;
    gctFLOAT    floatValue;
    gctUINT32   uintArray[2];
}
cluCONSTANT_VALUE;

struct _cloIR_CONSTANT
{
    struct _cloIR_EXPR    exprBase;
    gctUINT            valueCount;
    cluCONSTANT_VALUE * values;
    gctSTRING buffer;
    clsNAME * variable;
    gctUINT uniformCount;

    union {
       gcUNIFORM *uniformArr;
       gcUNIFORM uniform;
    } u;
    gctBOOL allValuesEqual;
};

typedef struct _cloIR_CONSTANT *cloIR_CONSTANT;

gceSTATUS
clConvFieldConstantToConstantValues(
cluCONSTANT_VALUE *Values,
clsDECL *Decl,
gctSTRING Buffer
);

gceSTATUS
cloIR_CONSTANT_Allocate(
    IN cloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN clsDECL * Decl,
    OUT cloIR_CONSTANT * Constant
    );

gceSTATUS
cloIR_CONSTANT_Construct(
    IN cloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN clsDECL * Decl,
    OUT cloIR_CONSTANT * Constant
    );

gceSTATUS
cloIR_CONSTANT_Clone(
    IN cloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN cloIR_CONSTANT Source,
    OUT cloIR_CONSTANT * Constant
    );

gceSTATUS
cloIR_CONSTANT_AddValues(
    IN cloCOMPILER Compiler,
    IN cloIR_CONSTANT Constant,
    IN gctUINT ValueCount,
    IN cluCONSTANT_VALUE * Values
    );

gceSTATUS
cloIR_CONSTANT_Int_Construct(
IN cloCOMPILER Compiler,
IN gctUINT LineNo,
IN gctUINT StringNo,
IN gctINT  IntConstant,
OUT cloIR_CONSTANT *Constant
);

gceSTATUS
cloIR_CONSTANT_AddCharValues(
IN cloCOMPILER Compiler,
IN cloIR_CONSTANT Constant,
IN gctUINT ValueCount,
IN gctSTRING Values
);

gceSTATUS
cloIR_CONSTANT_GetBoolValue(
    IN cloCOMPILER Compiler,
    IN cloIR_CONSTANT Constant,
    IN gctUINT ValueNo,
    OUT cluCONSTANT_VALUE * Value
    );

gceSTATUS
cloIR_CONSTANT_GetIntValue(
    IN cloCOMPILER Compiler,
    IN cloIR_CONSTANT Constant,
    IN gctUINT ValueNo,
    OUT cluCONSTANT_VALUE * Value
    );

gceSTATUS
cloIR_CONSTANT_GetUintValue(
    IN cloCOMPILER Compiler,
    IN cloIR_CONSTANT Constant,
    IN gctUINT ValueNo,
    OUT cluCONSTANT_VALUE * Value
    );

gceSTATUS
cloIR_CONSTANT_GetLongValue(
    IN cloCOMPILER Compiler,
    IN cloIR_CONSTANT Constant,
    IN gctUINT ValueNo,
    OUT cluCONSTANT_VALUE * Value
    );

gceSTATUS
cloIR_CONSTANT_GetULongValue(
    IN cloCOMPILER Compiler,
    IN cloIR_CONSTANT Constant,
    IN gctUINT ValueNo,
    OUT cluCONSTANT_VALUE * Value
    );

gceSTATUS
cloIR_CONSTANT_GetCharValue(
    IN cloCOMPILER Compiler,
    IN cloIR_CONSTANT Constant,
    IN gctUINT ValueNo,
    OUT cluCONSTANT_VALUE * Value
    );

gceSTATUS
cloIR_CONSTANT_GetFloatValue(
    IN cloCOMPILER Compiler,
    IN cloIR_CONSTANT Constant,
    IN gctUINT ValueNo,
    OUT cluCONSTANT_VALUE * Value
    );

typedef gceSTATUS
(* cltEVALUATE_FUNC_PTR)(
    IN cltELEMENT_TYPE Type,
    IN OUT cluCONSTANT_VALUE * Value
    );

gctINT
cloIR_CONSTANT_GetIntegerValue(
IN cloIR_CONSTANT Constant
);

gctBOOL
cloIR_CONSTANT_CheckAndSetAllValuesEqual(
IN cloCOMPILER Compiler,
IN cloIR_CONSTANT Constant
);

gceSTATUS
cloIR_CONSTANT_Evaluate(
    IN cloCOMPILER Compiler,
    IN OUT cloIR_CONSTANT Constant,
    IN cltEVALUATE_FUNC_PTR Evaluate
    );

/* cloIR_UNARY_EXPR object. */
typedef enum _cleUNARY_EXPR_TYPE
{
    clvUNARY_NULL = 0,         /* null expression */
    clvUNARY_NON_LVAL,         /* to indictate non-lval on variable -
                    e.g &*A in which A is a pointer variable*/
    clvUNARY_FIELD_SELECTION,
    clvUNARY_COMPONENT_SELECTION,

    clvUNARY_POST_INC,
    clvUNARY_POST_DEC,
    clvUNARY_PRE_INC,
    clvUNARY_PRE_DEC,

    clvUNARY_NEG,
    clvUNARY_INDIRECTION,
    clvUNARY_ADDR,
    clvUNARY_CAST,
    clvUNARY_NOT_BITWISE,
    clvUNARY_NOT
}
cleUNARY_EXPR_TYPE;

typedef enum _cleCOMPONENT
{
    clvCOMPONENT_X    = 0x0,
    clvCOMPONENT_Y    = 0x1,
    clvCOMPONENT_Z    = 0x2,
    clvCOMPONENT_W    = 0x3,
    clvCOMPONENT_4  = 0x4,
    clvCOMPONENT_5  = 0x5,
    clvCOMPONENT_6  = 0x6,
    clvCOMPONENT_7  = 0x7,
    clvCOMPONENT_8  = 0x8,
    clvCOMPONENT_9  = 0x9,
    clvCOMPONENT_10  = 0x0A,
    clvCOMPONENT_11  = 0x0B,
    clvCOMPONENT_12  = 0x0C,
    clvCOMPONENT_13  = 0x0D,
    clvCOMPONENT_14  = 0x0E,
    clvCOMPONENT_15  = 0x0F,
    clvCOMPONENT_16  = 0x10,
    clvCOMPONENT_17  = 0x11,
    clvCOMPONENT_18  = 0x12,
    clvCOMPONENT_19  = 0x13,
    clvCOMPONENT_20  = 0x14,
    clvCOMPONENT_21  = 0x15,
    clvCOMPONENT_22  = 0x16,
    clvCOMPONENT_23  = 0x17,
    clvCOMPONENT_24  = 0x18,
    clvCOMPONENT_25  = 0x19,
    clvCOMPONENT_26  = 0x1A,
    clvCOMPONENT_27  = 0x1B,
    clvCOMPONENT_28  = 0x1C,
    clvCOMPONENT_29  = 0x1D,
    clvCOMPONENT_30  = 0x1E,
    clvCOMPONENT_31  = 0x1F
}
cleCOMPONENT;

/* Maximum number component per vector component */
#define cldMaxComponentPerVectorElement 2
/* Maximum number of components */
#define cldMaxComponentCount (cldMAX_VECTOR_COMPONENT * 2)

typedef struct _clsCOMPONENT_SELECTION
{
    gctUINT8 components;
    gctUINT8 selection[cldMaxComponentCount];
}
clsCOMPONENT_SELECTION;

extern clsCOMPONENT_SELECTION    ComponentSelection_X;
extern clsCOMPONENT_SELECTION    ComponentSelection_Y;
extern clsCOMPONENT_SELECTION    ComponentSelection_Z;
extern clsCOMPONENT_SELECTION    ComponentSelection_W;
extern clsCOMPONENT_SELECTION    ComponentSelection_XY;
extern clsCOMPONENT_SELECTION    ComponentSelection_XYZ;
extern clsCOMPONENT_SELECTION    ComponentSelection_XYZW;
extern clsCOMPONENT_SELECTION    ComponentSelection_4;
extern clsCOMPONENT_SELECTION    ComponentSelection_5;
extern clsCOMPONENT_SELECTION    ComponentSelection_6;
extern clsCOMPONENT_SELECTION    ComponentSelection_7;
extern clsCOMPONENT_SELECTION    ComponentSelection_8;
extern clsCOMPONENT_SELECTION    ComponentSelection_VECTOR8;
extern clsCOMPONENT_SELECTION    ComponentSelection_9;
extern clsCOMPONENT_SELECTION    ComponentSelection_10;
extern clsCOMPONENT_SELECTION    ComponentSelection_11;
extern clsCOMPONENT_SELECTION    ComponentSelection_12;
extern clsCOMPONENT_SELECTION    ComponentSelection_13;
extern clsCOMPONENT_SELECTION    ComponentSelection_14;
extern clsCOMPONENT_SELECTION    ComponentSelection_15;
extern clsCOMPONENT_SELECTION    ComponentSelection_VECTOR16;

gctBOOL
clIsRepeatedComponentSelection(
    IN clsCOMPONENT_SELECTION * ComponentSelection
    );

gctCONST_STRING
clGetIRUnaryExprTypeName(
    IN cleUNARY_EXPR_TYPE UnaryExprType
    );

struct _cloIR_UNARY_EXPR
{
    struct _cloIR_EXPR    exprBase;
    cleUNARY_EXPR_TYPE    type;
    cloIR_EXPR        operand;
    union {
      clsNAME *    fieldName;            /* Only for the field selection */
      clsCOMPONENT_SELECTION componentSelection;    /* Only for the component selection */
      clsNAME *    generated;            /* For & generated for reference to an array */
    } u;
};

typedef struct _cloIR_UNARY_EXPR *cloIR_UNARY_EXPR;

gceSTATUS
cloIR_CAST_EXPR_Evaluate(
IN cloCOMPILER Compiler,
IN clsDECL  *TypeCast,
IN OUT cloIR_CONSTANT Constant
);

gceSTATUS
cloIR_CAST_EXPR_Construct(
IN cloCOMPILER Compiler,
IN gctUINT LineNo,
IN gctUINT StringNo,
IN clsDECL *TypeCast,
IN cloIR_EXPR Operand,
OUT cloIR_EXPR * Expr
);

gceSTATUS
cloIR_UNARY_EXPR_Construct(
IN cloCOMPILER Compiler,
IN gctUINT LineNo,
IN gctUINT StringNo,
IN cleUNARY_EXPR_TYPE Type,
IN cloIR_EXPR Operand,
IN clsNAME * FieldName,        /* Only for the field selection */
IN clsCOMPONENT_SELECTION * ComponentSelection,    /* Only for the component selection */
OUT cloIR_UNARY_EXPR * UnaryExpr
);

gceSTATUS
cloIR_UNARY_EXPR_Evaluate(
IN cloCOMPILER Compiler,
IN cleUNARY_EXPR_TYPE Type,
IN cloIR_CONSTANT Constant,
IN clsNAME * FieldName,    /* Only for the field selection */
IN clsCOMPONENT_SELECTION * ComponentSelection,    /* Only for the component selection */
OUT cloIR_CONSTANT * ResultConstant
);

gceSTATUS
clConstructScalarIntegerConstant(
IN cloCOMPILER Compiler,
IN gctUINT LineNo,
IN gctUINT StringNo,
IN gctINT IntegerValue,
OUT cloIR_CONSTANT * Constant
);

/* cloIR_BINARY_EXPR object. */
typedef enum _cleBINARY_EXPR_TYPE
{
    clvBINARY_SUBSCRIPT,
            /* The following five encodings ADD, SUB, MUL, DIV, MOD cannot be changed,
                           Values relied upon in function clSetFloatOpUsed, clGetFloatOpUsed
            */
    clvBINARY_ADD = 0x01,
    clvBINARY_SUB = 0x02,
    clvBINARY_MUL = 0x04,
    clvBINARY_DIV = 0x08,
    clvBINARY_MOD = 0x10,

    clvBINARY_AND_BITWISE,
    clvBINARY_OR_BITWISE,
    clvBINARY_XOR_BITWISE,

    clvBINARY_LSHIFT,
    clvBINARY_RSHIFT,

    clvBINARY_GREATER_THAN,
    clvBINARY_LESS_THAN,
    clvBINARY_GREATER_THAN_EQUAL,
    clvBINARY_LESS_THAN_EQUAL,

    clvBINARY_EQUAL,
    clvBINARY_NOT_EQUAL,

    clvBINARY_AND,
    clvBINARY_OR,
    clvBINARY_XOR,

    clvBINARY_SEQUENCE,

    clvBINARY_ASSIGN,

    clvBINARY_LEFT_ASSIGN,
    clvBINARY_RIGHT_ASSIGN,
    clvBINARY_AND_ASSIGN,
    clvBINARY_XOR_ASSIGN,
    clvBINARY_OR_ASSIGN,

    clvBINARY_MUL_ASSIGN,
    clvBINARY_DIV_ASSIGN,
    clvBINARY_ADD_ASSIGN,
    clvBINARY_MOD_ASSIGN,
    clvBINARY_SUB_ASSIGN,
    clvBINARY_MULTI_DIM_SUBSCRIPT
}
cleBINARY_EXPR_TYPE;

struct _cloIR_BINARY_EXPR
{
    struct _cloIR_EXPR    exprBase;
    cleBINARY_EXPR_TYPE    type;
    cloIR_EXPR        leftOperand;
    cloIR_EXPR        rightOperand;
};

typedef struct _cloIR_BINARY_EXPR *cloIR_BINARY_EXPR;

gceSTATUS
cloIR_ArrayDeclarator_Construct(
IN cloCOMPILER Compiler,
IN gctUINT LineNo,
IN gctUINT StringNo,
IN cloIR_EXPR ArrayDecl,
IN cloIR_EXPR ArraySize,
OUT cloIR_BINARY_EXPR *BinaryExpr
);

gceSTATUS
cloIR_NULL_EXPR_Construct(
IN cloCOMPILER Compiler,
IN gctUINT LineNo,
IN gctUINT StringNo,
IN clsDECL *Decl,
OUT cloIR_UNARY_EXPR *NullExpr
);

gceSTATUS
cloIR_BINARY_EXPR_Construct(
    IN cloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN cleBINARY_EXPR_TYPE Type,
    IN cloIR_EXPR LeftOperand,
    IN cloIR_EXPR RightOperand,
    OUT cloIR_BINARY_EXPR * BinaryExpr
    );

gceSTATUS
cloIR_BINARY_EXPR_Evaluate(
    IN cloCOMPILER Compiler,
    IN cleBINARY_EXPR_TYPE Type,
    IN cloIR_CONSTANT LeftConstant,
    IN cloIR_CONSTANT RightConstant,
    IN clsDECL *ResultType,
    OUT cloIR_CONSTANT * ResultConstant
    );

gctUINT
clGetFloatOpsUsed(
IN cloCOMPILER Compiler,
IN gctBOOL GtThanTwo
);

gctINT
clGetVectorTerminalToken(
IN cltELEMENT_TYPE ElementType,
IN gctINT8 NumComponents
);

gctBOOL
clAreElementTypeInRankOrder(
IN cltELEMENT_TYPE HighRank,
IN cltELEMENT_TYPE LowRank
);

void
clSetFloatOpsUsed(
IN cloCOMPILER Compiler,
IN cleBINARY_EXPR_TYPE BinaryOp
);

/* cloIR_SELECTION object. */
struct _cloIR_SELECTION
{
    struct _cloIR_EXPR    exprBase;
    cloIR_EXPR        condExpr;
    cloIR_BASE        trueOperand;
    cloIR_BASE        falseOperand;
};

typedef struct _cloIR_SELECTION *cloIR_SELECTION;

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
    );

/* cloIR_SWITCH object. */
struct _cloIR_SWITCH
{
    struct _cloIR_EXPR    exprBase;
    cloIR_EXPR        condExpr;
    cloIR_BASE        switchBody;
    cloIR_LABEL        cases;
};

typedef struct _cloIR_SWITCH *cloIR_SWITCH;

gceSTATUS
cloIR_SWITCH_Construct(
    IN cloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN clsDECL * Decl,
    IN cloIR_EXPR CondExpr,
    IN cloIR_BASE SwitchBody,
    IN cloIR_LABEL Cases,
    OUT cloIR_SWITCH * Selection
    );

struct _cloIR_POLYNARY_EXPR
{
    struct _cloIR_EXPR    exprBase;
    clePOLYNARY_EXPR_TYPE    type;
    cltPOOL_STRING        funcSymbol;    /* Only for the function call */
    clsNAME *        funcName;    /* Only for the function call */
    cloIR_SET        operands;
};

typedef struct _cloIR_POLYNARY_EXPR *cloIR_POLYNARY_EXPR;

struct _cloIR_TYPECAST_ARGS
{
    struct _cloIR_EXPR    exprBase;
    cloIR_EXPR        lhs;
    cloIR_SET        operands;
};

typedef struct _cloIR_TYPECAST_ARGS *cloIR_TYPECAST_ARGS;

/* Parser State */
typedef enum _clePARSER_STATE
{
    clvPARSER_NORMAL    = 0,
    clvPARSER_IN_TYPE_CAST,
    clvPARSER_IN_TYPEDEF
}
clePARSER_STATE;

typedef gctUINT8 cltPARSER_STATE;

struct _clsPARSER_STATE;
typedef struct _clsPARSER_STATE
{
    slsSLINK_NODE node;
    clsDECL castTypeDecl;
    cltPARSER_STATE state;
}
clsPARSER_STATE;

void
cloIR_InitializeVecCompSelTypes();

gctCONST_STRING
clGetIRPolynaryExprTypeName(
    IN clePOLYNARY_EXPR_TYPE PolynaryExprType
    );

gctCONST_STRING
clGetElementTypeName(IN cltELEMENT_TYPE ElementType);

gceSTATUS
cloIR_POLYNARY_EXPR_Construct(
    IN cloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN clePOLYNARY_EXPR_TYPE Type,
    IN clsDECL * Decl,
    IN cltPOOL_STRING FuncSymbol,
    OUT cloIR_POLYNARY_EXPR * PolynaryExpr
    );

gceSTATUS
cloIR_TYPECAST_ARGS_Construct(
    IN cloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    OUT cloIR_TYPECAST_ARGS *TypeCastArgs
    );

gceSTATUS
cloIR_CreateVectorType(
IN cloCOMPILER Compiler,
IN clsDATA_TYPE *DataType,
IN gctUINT8  VectorSize,
IN clsDATA_TYPE **VecDataType
);

gceSTATUS
cloIR_GetArithmeticExprDecl(
IN cloCOMPILER Compiler,
IN gctBOOL IsMul,
IN cloIR_EXPR LeftOperand,
IN cloIR_EXPR RightOperand,
OUT clsDECL *Decl
);

gceSTATUS
cloIR_ScalarizeFuncCall(
IN cloCOMPILER Compiler,
IN cloIR_POLYNARY_EXPR VectorFuncCall,
IN clsNAME *RefFuncName,
IN gctBOOL IsLookUp,
OUT cloIR_POLYNARY_EXPR *ScalarFuncCall
);

gceSTATUS
cloCOMPILER_BindFuncCall(
    IN cloCOMPILER Compiler,
    IN OUT cloIR_POLYNARY_EXPR PolynaryExpr
    );

gceSTATUS
cloCOMPILER_BindBuiltinFuncCall(
IN cloCOMPILER Compiler,
IN OUT cloIR_POLYNARY_EXPR PolynaryExpr
);

gceSTATUS
cloIR_POLYNARY_EXPR_Evaluate(
    IN cloCOMPILER Compiler,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    OUT cloIR_CONSTANT * ResultConstant
    );

gctSIZE_T
clAlignMemory(
cloCOMPILER Compiler,
clsNAME *Variable,
gctSIZE_T MemorySize
);

cltPOOL_STRING
clCreateMangledFuncName(
IN cloCOMPILER Compiler,
IN clsNAME *FuncName
);

gceSTATUS
clMergePtrDscrToDecl(
IN cloCOMPILER Compiler,
IN slsSLINK_LIST *PtrDscr,
IN clsDECL *Decl,
IN gctBOOL PtrDominant
);

/* Visitor */
typedef gceSTATUS
(* cltVISIT_SET_FUNC_PTR)(
    IN cloCOMPILER Compiler,
    IN struct _clsVISITOR * Visitor,
    IN cloIR_SET Set,
    IN OUT gctPOINTER Parameters
    );

typedef gceSTATUS
(* cltVISIT_ITERATION_FUNC_PTR)(
    IN cloCOMPILER Compiler,
    IN struct _clsVISITOR * Visitor,
    IN cloIR_ITERATION Iteration,
    IN OUT gctPOINTER Parameters
    );

typedef gceSTATUS
(* cltVISIT_JUMP_FUNC_PTR)(
    IN cloCOMPILER Compiler,
    IN struct _clsVISITOR * Visitor,
    IN cloIR_JUMP Jump,
    IN OUT gctPOINTER Parameters
    );

typedef gceSTATUS
(* cltVISIT_LABEL_FUNC_PTR)(
    IN cloCOMPILER Compiler,
    IN struct _clsVISITOR * Visitor,
    IN cloIR_LABEL Label,
    IN OUT gctPOINTER Parameters
    );

typedef gceSTATUS
(* cltVISIT_VARIABLE_FUNC_PTR)(
    IN cloCOMPILER Compiler,
    IN struct _clsVISITOR * Visitor,
    IN cloIR_VARIABLE Variable,
    IN OUT gctPOINTER Parameters
    );

typedef gceSTATUS
(* cltVISIT_CONSTANT_FUNC_PTR)(
    IN cloCOMPILER Compiler,
    IN struct _clsVISITOR * Visitor,
    IN cloIR_CONSTANT Constant,
    IN OUT gctPOINTER Parameters
    );

typedef gceSTATUS
(* cltVISIT_UNARY_EXPR_FUNC_PTR)(
    IN cloCOMPILER Compiler,
    IN struct _clsVISITOR * Visitor,
    IN cloIR_UNARY_EXPR UnaryExpr,
    IN OUT gctPOINTER Parameters
    );

typedef gceSTATUS
(* cltVISIT_BINARY_EXPR_FUNC_PTR)(
    IN cloCOMPILER Compiler,
    IN struct _clsVISITOR * Visitor,
    IN cloIR_BINARY_EXPR BinaryExpr,
    IN OUT gctPOINTER Parameters
    );

typedef gceSTATUS
(* cltVISIT_SELECTION_FUNC_PTR)(
    IN cloCOMPILER Compiler,
    IN struct _clsVISITOR * Visitor,
    IN cloIR_SELECTION Selection,
    IN OUT gctPOINTER Parameters
    );

typedef gceSTATUS
(* cltVISIT_SWITCH_FUNC_PTR)(
    IN cloCOMPILER Compiler,
    IN struct _clsVISITOR * Visitor,
    IN cloIR_SWITCH Selection,
    IN OUT gctPOINTER Parameters
    );

typedef gceSTATUS
(* cltVISIT_POLYNARY_EXPR_FUNC_PTR)(
    IN cloCOMPILER Compiler,
    IN struct _clsVISITOR * Visitor,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN OUT gctPOINTER Parameters
    );

typedef gceSTATUS
(* cltVISIT_TYPECAST_ARGS_FUNC_PTR)(
    IN cloCOMPILER Compiler,
    IN struct _clsVISITOR * Visitor,
    IN cloIR_TYPECAST_ARGS TypeCastArgs,
    IN OUT gctPOINTER Parameters
    );

typedef struct _clsVISITOR
{
    clsOBJECT            object;
    cltVISIT_SET_FUNC_PTR        visitSet;
    cltVISIT_ITERATION_FUNC_PTR    visitIteration;
    cltVISIT_JUMP_FUNC_PTR        visitJump;
    cltVISIT_LABEL_FUNC_PTR        visitLabel;
    cltVISIT_VARIABLE_FUNC_PTR    visitVariable;
    cltVISIT_CONSTANT_FUNC_PTR    visitConstant;
    cltVISIT_UNARY_EXPR_FUNC_PTR    visitUnaryExpr;
    cltVISIT_BINARY_EXPR_FUNC_PTR    visitBinaryExpr;
    cltVISIT_SELECTION_FUNC_PTR    visitSelection;
    cltVISIT_SWITCH_FUNC_PTR    visitSwitch;
    cltVISIT_POLYNARY_EXPR_FUNC_PTR    visitPolynaryExpr;
    cltVISIT_TYPECAST_ARGS_FUNC_PTR    visitTypeCastArgs;
}
clsVISITOR;

#endif /* __gc_cl_ir_h_ */
