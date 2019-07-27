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


#include "gc_cl_parser.h"
extern int
yyparse(void *);

extern void
yyrestart(gctPOINTER);

#define _CREATE_UNNAMED_CONSTANT_IN_MEMORY  1

static gceSTATUS
_ParseSetAggregateTypedOperandAddressed(
IN cloCOMPILER Compiler,
IN cloIR_EXPR Operand,
IN OUT cloIR_EXPR *ResOperand
);

static gceSTATUS
_MakeConstantVariableExpr(
IN cloCOMPILER Compiler,
cloIR_CONSTANT ConstantOperand,
cloIR_EXPR *ConstVariableExpr
);

static gctCONST_STRING
_GetBinaryOperatorName(
IN gctINT TokenType
);

extern int
cloCOMPILER_Lex(YYSTYPE * pyylval, cloCOMPILER Compiler)
{
    int tokenType;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);

    tokenType = cloCOMPILER_Scan(Compiler, &(pyylval->token));
    if(clIsBuiltinDataType(tokenType) ||
       (tokenType == T_TYPE_NAME) ||
       (tokenType == T_IDENTIFIER && cloCOMPILER_GetScannerState(Compiler) == clvSCANNER_IN_TYPEOF)) {
       gcmVERIFY_OK(cloCOMPILER_SetScannerState(Compiler, clvSCANNER_AFTER_TYPE));
    }
    else if(tokenType == T_TYPEOF) {
       gcmVERIFY_OK(cloCOMPILER_SetScannerState(Compiler, clvSCANNER_IN_TYPEOF));
    }
    else {
       gcmVERIFY_OK(cloCOMPILER_SetScannerState(Compiler, clvSCANNER_NORMAL));
    }

    return tokenType;
}

gctCONST_STRING *_IndexKeywordStrings = gcvNULL;
gceSTATUS
cloCOMPILER_Parse(
IN cloCOMPILER Compiler,
IN gctUINT StringCount,
IN gctCONST_STRING Strings[],
IN gctCONST_STRING Options)
{
    gceSTATUS    status = gcvSTATUS_OK;

#ifdef SL_SCAN_ONLY
    clsLexToken    token;
#endif
/** Initialize index to keyword table entries **/
    if(_IndexKeywordStrings == gcvNULL) {
         _IndexKeywordStrings = clScanInitIndexToKeywordTableEntries();
    }

    status = cloCOMPILER_MakeCurrent(Compiler, StringCount, Strings, Options);
    clScanInitLanguageVersion(cloCOMPILER_GetLanguageVersion(Compiler),
                              cloCOMPILER_GetExtension(Compiler));
    if(gcmIS_ERROR(status)) return status;

/* Force basic vector types to be packed when VX extension is enabled */

#ifdef SL_SCAN_ONLY
    while (cloCOMPILER_Scan(Compiler, &token) != T_EOF);
#else
    clScanInitErrorHandler(Compiler);
    yyrestart(gcvNULL);
    if (yyparse(Compiler) != 0) status = gcvSTATUS_INVALID_ARGUMENT;
    clScanDeleteBuffer(Compiler);
#endif
    return status;
}

#define _clmExprIsConstantForEval(Expr) \
    (cloIR_OBJECT_GetType(&((Expr)->base)) == clvIR_CONSTANT &&  \
     (!_GEN_UNIFORMS_FOR_CONSTANT_ADDRESS_SPACE_VARIABLES ||  \
      !clmDECL_IsAggregateType(&((Expr)->decl))))

static gceSTATUS
_CheckConstantExpr(
IN cloCOMPILER Compiler,
IN cloIR_EXPR Expr
)
{
    gcmASSERT(Expr);

    if (cloIR_OBJECT_GetType(&Expr->base) != clvIR_CONSTANT) {
        gcmVERIFY_OK(cloCOMPILER_Report( Compiler, Expr->base.lineNo, Expr->base.stringNo,
                        clvREPORT_ERROR, "require a constant expression"));
        return gcvSTATUS_INVALID_ARGUMENT;
    }
    return gcvSTATUS_OK;
}

static gceSTATUS
_CheckIntConstantExpr(
IN cloCOMPILER Compiler,
IN cloIR_EXPR Expr
)
{
    gceSTATUS status;
    cloIR_CONSTANT constant;

    status = _CheckConstantExpr(Compiler, Expr);
    if (gcmIS_ERROR(status)) return status;

    constant = (cloIR_CONSTANT)Expr;
    if (constant->exprBase.decl.dataType == gcvNULL
        || !clmDECL_IsInt(&constant->exprBase.decl)) {
        gcmVERIFY_OK(cloCOMPILER_Report(Compiler, Expr->base.lineNo, Expr->base.stringNo,
                        clvREPORT_ERROR,
                        "require an integral constant expression"));
        return gcvSTATUS_INVALID_ARGUMENT;
    }
    return gcvSTATUS_OK;
}

static gceSTATUS
_CheckLValueExpr(
IN cloCOMPILER Compiler,
IN cloIR_EXPR Expr,
IN gctSTRING Msg
)
{
    cloIR_UNARY_EXPR unaryExpr;
    cloIR_BINARY_EXPR binaryExpr;

    gcmASSERT(Expr);
    gcmASSERT(Expr->decl.dataType);

    if(!clmDECL_IsPointerType(&Expr->decl)) {
       if (Expr->decl.dataType->addrSpaceQualifier == clvQUALIFIER_CONSTANT) {
              gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                              Expr->base.lineNo,
                                              Expr->base.stringNo,
                                              clvREPORT_ERROR,
                                              "require %s to be an l-value expression",
                                              Msg));

              return gcvSTATUS_INVALID_ARGUMENT;
       }
       switch (Expr->decl.dataType->accessQualifier) {
       case clvQUALIFIER_UNIFORM:
       case clvQUALIFIER_CONST:
       case clvQUALIFIER_ATTRIBUTE:
       case clvQUALIFIER_READ_ONLY:
        gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                                Expr->base.lineNo,
                                                Expr->base.stringNo,
                                                clvREPORT_ERROR,
                                                "require %s to be an l-value expression",
                                                Msg));
        return gcvSTATUS_INVALID_ARGUMENT;
       }
    }
    else { /* is a pointer */
       clsNAME *leafName;
       leafName = clParseFindLeafName(Compiler,
                                      Expr);

       if(!leafName) {
          switch (cloIR_OBJECT_GetType(&Expr->base)) {
          case clvIR_UNARY_EXPR:
            unaryExpr = (cloIR_UNARY_EXPR)Expr;
            if(unaryExpr->type != clvUNARY_INDIRECTION)
           gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                                   Expr->base.lineNo,
                                                   Expr->base.stringNo,
                                                   clvREPORT_ERROR,
                                                   "require %s to be an l-value expression",
                                                   Msg));
               return gcvSTATUS_INVALID_ARGUMENT;
            break;

          case clvIR_BINARY_EXPR:
            binaryExpr = (cloIR_BINARY_EXPR)Expr;
            if(binaryExpr->type != clvBINARY_SUBSCRIPT)
           gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                                   Expr->base.lineNo,
                                                   Expr->base.stringNo,
                                                   clvREPORT_ERROR,
                                                   "require %s to be an l-value expression",
                                                   Msg));
               return gcvSTATUS_INVALID_ARGUMENT;
            break;

          default:
            break;
          }
       }
    }
    if (cloIR_OBJECT_GetType(&Expr->base) == clvIR_UNARY_EXPR) {
            unaryExpr = (cloIR_UNARY_EXPR)Expr;
            switch(unaryExpr->type) {
            case clvUNARY_NON_LVAL:
            case clvUNARY_ADDR:
               gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                               Expr->base.lineNo,
                                               Expr->base.stringNo,
                                               clvREPORT_ERROR,
                                               "require %s to be an l-value expression",
                                               Msg));
               return gcvSTATUS_INVALID_ARGUMENT;

            case clvUNARY_COMPONENT_SELECTION:
               if(clIsRepeatedComponentSelection(&unaryExpr->u.componentSelection)) {
                   gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                                   Expr->base.lineNo,
                                                   Expr->base.stringNo,
                                                   clvREPORT_ERROR,
                                                   "%s expression select repeated"
                                                   " components or swizzles",
                                                   Msg));
                   return gcvSTATUS_INVALID_ARGUMENT;
               }
               break;

            default:
               break;
            }
    }
    return gcvSTATUS_OK;
}

cloIR_EXPR
clParseNullExpr(
IN cloCOMPILER Compiler,
IN clsLexToken *StartToken
)
{
   gceSTATUS status;
   clsDECL decl[1];
   cloIR_UNARY_EXPR nullExpr;

   status = cloCOMPILER_CreateDecl(Compiler,
                                   T_INT,
                                   gcvNULL,
                                   clvQUALIFIER_NONE,
                                   clvQUALIFIER_NONE,
                                   decl);
   if (gcmIS_ERROR(status)) return gcvNULL;

   status =  cloIR_NULL_EXPR_Construct(Compiler,
                                       StartToken->lineNo,
                                       StartToken->stringNo,
                                       decl,
                                       &nullExpr);
   if (gcmIS_ERROR(status)) return gcvNULL;
   return &nullExpr->exprBase;
}

static gceSTATUS
_EvaluateExprToArrayLength(
IN cloCOMPILER Compiler,
IN cloIR_EXPR Expr,
IN gctBOOL UnspecifiedOK,
OUT clsARRAY *Array
)
{
   gceSTATUS status;

   gcmASSERT(Expr);
   switch (cloIR_OBJECT_GetType(&Expr->base)) {
   case clvIR_UNARY_EXPR:
      gcmASSERT(((cloIR_UNARY_EXPR) &Expr->base)->type == clvUNARY_NULL);

      if(UnspecifiedOK) {
        Array->length[Array->numDim] = -1;
        Array->numDim++;
      }
      else {
         gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                         Expr->base.lineNo,
                                         Expr->base.stringNo,
                                         clvREPORT_ERROR,
                                         "unspecified array size not supported"));
         return gcvSTATUS_INVALID_DATA;
      }
      break;

   case clvIR_BINARY_EXPR:
      {
         cloIR_BINARY_EXPR binaryExpr;
         binaryExpr = (cloIR_BINARY_EXPR) &Expr->base;
         gcmASSERT(binaryExpr->type == clvBINARY_MULTI_DIM_SUBSCRIPT);
         status = _EvaluateExprToArrayLength(Compiler,
                                             binaryExpr->leftOperand,
                                             UnspecifiedOK,
                                             Array);
         if(gcmIS_ERROR(status)) return status;

         status = _EvaluateExprToArrayLength(Compiler,
                                             binaryExpr->rightOperand,
                                             UnspecifiedOK,
                                             Array);
         if(gcmIS_ERROR(status)) return status;
      }
      break;

   default:
      {
         cloIR_CONSTANT constant;

         gcmASSERT(Array->numDim >= 0 && Array->numDim < cldMAX_ARRAY_DIMENSION);

         status = _CheckIntConstantExpr(Compiler, Expr);
         if (gcmIS_ERROR(status)) return status;

         constant = (cloIR_CONSTANT)Expr;
         if (constant->valueCount > 1
             || constant->values == gcvNULL
             || constant->values[0].intValue <= 0) {
            gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                         Expr->base.lineNo,
                         Expr->base.stringNo,
                         clvREPORT_ERROR,
                         "the array length must be greater than zero"));
            return gcvSTATUS_INVALID_ARGUMENT;
    }
        Array->length[Array->numDim] = (gctUINT)constant->values[0].intValue;
        Array->numDim++;
    gcmVERIFY_OK(cloIR_OBJECT_Destroy(Compiler, &constant->exprBase.base));
     }
     break;
   }
   return gcvSTATUS_OK;
}

static gceSTATUS
_EvaluateExprToInteger(
IN cloCOMPILER Compiler,
IN cloIR_EXPR Expr,
OUT gctINT *IntValue
)
{
    gceSTATUS    status;
    cloIR_CONSTANT    constant;

    gcmASSERT(Expr);
    gcmASSERT(IntValue);

    *IntValue = 0;
    status = _CheckIntConstantExpr(Compiler, Expr);
    if (gcmIS_ERROR(status)) return status;

    constant = (cloIR_CONSTANT)Expr;
    if (constant->valueCount > 1
        || constant->values == gcvNULL) {
        gcmVERIFY_OK(cloCOMPILER_Report(Compiler, Expr->base.lineNo, Expr->base.stringNo,
                        clvREPORT_ERROR,
                        "invalid scalar integer"));
        return gcvSTATUS_INVALID_ARGUMENT;
    }
    *IntValue = constant->values[0].intValue;
    gcmVERIFY_OK(cloIR_OBJECT_Destroy(Compiler, &constant->exprBase.base));
    return gcvSTATUS_OK;
}

static cloIR_EXPR
_EvaluateIndirectionExpr(
IN cloCOMPILER Compiler,
IN cloIR_EXPR Operand
)
{
   if(!slmSLINK_LIST_IsEmpty(Operand->decl.ptrDscr) ||
      clmDECL_IsStructOrUnion(&Operand->decl) ||
      clmDECL_IsArray(&Operand->decl)) { /*create operand[0] */
        cloIR_EXPR arrayElem0;
        clsDECL indexDecl;
        cloIR_CONSTANT arrayIndex;
        cluCONSTANT_VALUE  value;
        gceSTATUS status;

        /* Create the const zero declaration */
        status = cloCOMPILER_CreateDecl(Compiler,
                                        T_INT,
                                        gcvNULL,
                                        clvQUALIFIER_CONST,
                                        clvQUALIFIER_NONE,
                                        &indexDecl);
        if (gcmIS_ERROR(status)) return gcvNULL;

        /* Create the constant */
        status = cloIR_CONSTANT_Construct(Compiler,
                                          Operand->base.lineNo,
                                          Operand->base.stringNo,
                                          &indexDecl,
                                          &arrayIndex);
        if (gcmIS_ERROR(status)) return gcvNULL;

        value.intValue = 0;
        /* Add the constant value */
        status = cloIR_CONSTANT_AddValues(Compiler,
                                          arrayIndex,
                                          1,
                                          &value);
        if (gcmIS_ERROR(status)) return gcvNULL;

        status = cloIR_BINARY_EXPR_Construct(Compiler,
                                             Operand->base.lineNo,
                                             Operand->base.stringNo,
                                             clvBINARY_SUBSCRIPT,
                                             Operand,
                                             &arrayIndex->exprBase,
                                             &arrayElem0);
        if (gcmIS_ERROR(status)) return gcvNULL;
        return arrayElem0;
   }
   return gcvNULL;
}

cloIR_EXPR
clParseVariableIdentifier(
IN cloCOMPILER Compiler,
IN clsLexToken * Identifier
)
{
    gceSTATUS    status;
    clsNAME *    name;
    cloIR_CONSTANT    constant;
    cloIR_VARIABLE    variable;
    cloIR_EXPR    expr;

    gcmASSERT(Identifier);

    status = cloCOMPILER_SearchName(Compiler, Identifier->u.identifier.name, gcvTRUE, &name);
    if (status != gcvSTATUS_OK) {
        gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                        Identifier->lineNo,
                                        Identifier->stringNo,
                                        clvREPORT_ERROR,
                                        "undefined identifier: '%s'",
                                        Identifier->u.identifier.name));
        return gcvNULL;
    }

    switch (name->type) {
    case clvENUM_NAME:
    case clvVARIABLE_NAME:
    case clvPARAMETER_NAME:
        break;

    case clvFUNC_NAME:
    case clvKERNEL_FUNC_NAME:
    case clvSTRUCT_NAME:
    case clvUNION_NAME:
    case clvFIELD_NAME:
    case clvENUM_TAG_NAME:
        gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                        Identifier->lineNo,
                                        Identifier->stringNo,
                                        clvREPORT_ERROR,
                                        "'%s' isn't a variable",
                                        name->symbol));
        return gcvNULL;

    default:
        gcmASSERT(0);
        return gcvNULL;
    }

    if ((name->type == clvVARIABLE_NAME || name->type == clvENUM_NAME)
        && name->u.variableInfo.u.constant != gcvNULL) {
        status = cloIR_CONSTANT_Clone(Compiler,
                                      Identifier->lineNo,
                                      Identifier->stringNo,
                                      name->u.variableInfo.u.constant, &constant);
        if (gcmIS_ERROR(status)) return gcvNULL;
        expr = &constant->exprBase;
    }
    else if(clmDECL_IsVoid(&name->decl)){
        gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                        Identifier->lineNo,
                                        Identifier->stringNo,
                                        clvREPORT_ERROR,
                                        "'%s' is a parameter of type void",
                                        name->symbol));
        return gcvNULL;
    }
    else {
       cloIR_UNARY_EXPR unaryExpr;

       status = cloIR_VARIABLE_Construct(Compiler,
                                         Identifier->lineNo,
                                         Identifier->stringNo,
                                         name,
                                         &variable);
       if (gcmIS_ERROR(status)) return gcvNULL;
       expr = &variable->exprBase;
       if(clmDECL_IsArray(&name->decl)) {
          status = clScanLookAheadWithSkip(Compiler, '[', ')');
          if(status == gcvSTATUS_NOT_FOUND &&
             clmDECL_IsAggregateTypeOverRegLimit(&name->decl)) { /* the next token is not '[' */
             /* Create the expression &A[0] for A being an array*/
             expr =  _EvaluateIndirectionExpr(Compiler,
                                              expr);
             gcmASSERT(expr);
             status = cloIR_UNARY_EXPR_Construct(Compiler,
                                                 Identifier->lineNo,
                                                 Identifier->stringNo,
                                                 clvUNARY_ADDR,
                                                 expr,
                                                 gcvNULL,
                                                 gcvNULL,
                                                 &expr);
             if (gcmIS_ERROR(status)) return gcvNULL;
             unaryExpr = (cloIR_UNARY_EXPR)&expr->base;
             unaryExpr->u.generated = name;
             status = clParseSetOperandAddressed(Compiler,
                                                 expr);
             if (gcmIS_ERROR(status)) return gcvNULL;
          }
       }
    }

    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                                  clvDUMP_PARSER,
                                  "<VARIABLE_IDENTIFIER line=\"%d\" string=\"%d\" name=\"%s\" />",
                                  Identifier->lineNo,
                                  Identifier->stringNo,
                                  Identifier->u.identifier.name));
    return expr;
}

static cloIR_CONSTANT
_ParseCreateConstant(
IN cloCOMPILER Compiler,
IN gctUINT LineNo,
IN gctUINT StringNo,
IN gctINT Type,
cluCONSTANT_VALUE *Value
)
{
  gceSTATUS status;
  clsDECL decl[1];
  cloIR_CONSTANT constant;

/* Create the data type */
  status = cloCOMPILER_CreateDecl(Compiler,
                                  Type,
                                  gcvNULL,
                                  clvQUALIFIER_CONST,
                                  clvQUALIFIER_NONE,
                                  decl);
  if (gcmIS_ERROR(status)) return gcvNULL;

/* Create the constant */
  status = cloIR_CONSTANT_Construct(Compiler,
                                    LineNo,
                                    StringNo,
                                    decl,
                                    &constant);
  if (gcmIS_ERROR(status)) return gcvNULL;

/* Add the constant value */
  status = cloIR_CONSTANT_AddValues(Compiler,
                                    constant,
                                    1,
                                    Value);
  if (gcmIS_ERROR(status)) return gcvNULL;
  return constant;
}

cloIR_EXPR
clParseScalarConstant(
IN cloCOMPILER Compiler,
IN clsLexToken *ScalarConstant
)
{
    cloIR_CONSTANT    constant;

    gcmASSERT(ScalarConstant);

    constant = _ParseCreateConstant(Compiler,
                                    ScalarConstant->lineNo,
                                    ScalarConstant->stringNo,
                                    ScalarConstant->type,
                                    &ScalarConstant->u.constant);
    if(!constant) return gcvNULL;

    switch(ScalarConstant->type) {
    case T_FLOAT:
       gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                     clvDUMP_PARSER,
                         "<FLOAT_CONSTANT line=\"%d\" string=\"%d\" value=\"%f\" />",
                     ScalarConstant->lineNo,
                     ScalarConstant->stringNo,
                     ScalarConstant->u.constant.floatValue));
       break;

    case T_BOOL:
       gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                         clvDUMP_PARSER,
                         "<BOOL_CONSTANT line=\"%d\" string=\"%d\" value=\"%s\" />",
                         ScalarConstant->lineNo,
                         ScalarConstant->stringNo,
                         (ScalarConstant->u.constant.boolValue)? "true" : "false"));
       break;

    case T_CHAR:
       gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                         clvDUMP_PARSER,
                         "<CHAR_CONSTANT line=\"%d\" string=\"%d\" value=\"%c\" />",
                         ScalarConstant->lineNo,
                         ScalarConstant->stringNo,
                         ScalarConstant->u.constant.intValue));
       break;

    case T_UINT:
       gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                         clvDUMP_PARSER,
                         "<UNSIGNED_CONSTANT line=\"%d\" string=\"%d\" value=\"%u\" />",
                         ScalarConstant->lineNo,
                         ScalarConstant->stringNo,
                         ScalarConstant->u.constant.uintValue));
       break;

    case T_LONG:
       gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                         clvDUMP_PARSER,
                         "<LONG_CONSTANT line=\"%d\" string=\"%d\" value=\"%ld\" />",
                         ScalarConstant->lineNo,
                         ScalarConstant->stringNo,
                         ScalarConstant->u.constant.longValue));
       break;

    case T_ULONG:
       gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                         clvDUMP_PARSER,
                         "<UNSIGNED_LONG_CONSTANT line=\"%d\" string=\"%d\" value=\"%lu\" />",
                         ScalarConstant->lineNo,
                         ScalarConstant->stringNo,
                         ScalarConstant->u.constant.ulongValue));
       break;

    default:
       gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                     clvDUMP_PARSER,
                     "<INT_CONSTANT line=\"%d\" string=\"%d\" value=\"%d\" />",
                     ScalarConstant->lineNo,
                     ScalarConstant->stringNo,
                     ScalarConstant->u.constant.intValue));
       break;
    }

    return &constant->exprBase;
}

cloIR_EXPR
clParseStringLiteral(
IN cloCOMPILER Compiler,
IN clsLexToken * StringLiteral
)
{
    gceSTATUS status;
    clsDECL    arrayDecl;
    clsARRAY array[1];
    clsDATA_TYPE *dataType;
    cloIR_CONSTANT    constant;

    gcmHEADER_ARG("Compiler=0x%x StringLiteral=0x%x",
                       Compiler, StringLiteral);

    gcmASSERT(StringLiteral);
    /* Create the data type */
    status = cloCOMPILER_CreateDataType(Compiler,
                                        T_CHAR,
                                        gcvNULL,
                                        clvQUALIFIER_CONST,
                                        clvQUALIFIER_NONE,
                                        &dataType);
    if (gcmIS_ERROR(status)) {
       gcmFOOTER();
       return gcvNULL;
    }

    array->numDim = 1;
    array->length[0] = StringLiteral->u.stringLiteral.len,
    status = cloCOMPILER_CreateArrayDecl(Compiler,
                                         dataType,
                                         array,
                                         gcvNULL,
                                         &arrayDecl);
    if (gcmIS_ERROR(status)) {
       gcmFOOTER();
       return gcvNULL;
    }

    /* Create the constant */
    status = cloIR_CONSTANT_Construct(Compiler,
                                      StringLiteral->lineNo,
                                      StringLiteral->stringNo,
                                      &arrayDecl,
                                      &constant);
    if (gcmIS_ERROR(status))
    {
        gcmFOOTER();
        return gcvNULL;
    }

    /* Add the constant string literal values */
    status = cloIR_CONSTANT_AddCharValues(Compiler,
                                          constant,
                                          StringLiteral->u.stringLiteral.len,
                                          StringLiteral->u.stringLiteral.value);
    if (gcmIS_ERROR(status))
    {
        gcmFOOTER();
        return gcvNULL;
    }

    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                      clvDUMP_PARSER,
                      "<STRING_LITERAL line=\"%d\" string=\"%d\" value=\"%s\""
                                      " length=\"%d\" />",
                      StringLiteral->lineNo,
                      StringLiteral->stringNo,
                      StringLiteral->u.stringLiteral.value ?
                      StringLiteral->u.stringLiteral.value : "",
                      StringLiteral->u.stringLiteral.len));
    gcmFOOTER();
    return &constant->exprBase;
}

clsLexToken
clParseCatStringLiteral(
IN cloCOMPILER Compiler,
IN clsLexToken *FirstStr,
IN clsLexToken *SecondStr
)
{
    gctSTRING strValue;
    gctINT len;
    gceSTATUS status = gcvSTATUS_OK;
    gctPOINTER pointer;

    gcmHEADER_ARG("Compiler=0x%x FirstStr=0x%x SecondStr=0x%x",
                       Compiler, FirstStr, SecondStr);

    gcmASSERT(FirstStr);
    gcmASSERT(SecondStr);

    if(FirstStr->u.stringLiteral.len == 0) {
      gcmFOOTER();
      return *SecondStr;
    }
    else if(SecondStr->u.stringLiteral.len == 0) {
      gcmFOOTER();
      return *FirstStr;
    }

    len = FirstStr->u.stringLiteral.len + SecondStr->u.stringLiteral.len - 1;
    status = cloCOMPILER_Allocate(Compiler, len, (gctPOINTER *) &pointer);

    if(gcmIS_ERROR(status)) {
       gcmFOOTER();
       return *FirstStr;
    }

    strValue = pointer;
    if(FirstStr->u.stringLiteral.len > 1) {
      gcoOS_MemCopy(strValue,
                     FirstStr->u.stringLiteral.value,
                     FirstStr->u.stringLiteral.len - 1);
    }

    gcoOS_MemCopy(strValue + FirstStr->u.stringLiteral.len - 1,
                   SecondStr->u.stringLiteral.value,
                   SecondStr->u.stringLiteral.len);

    gcmVERIFY_OK(cloCOMPILER_Free(Compiler, FirstStr->u.stringLiteral.value));
    gcmVERIFY_OK(cloCOMPILER_Free(Compiler, SecondStr->u.stringLiteral.value));

    FirstStr->u.stringLiteral.value = strValue;
    FirstStr->u.stringLiteral.len = len;

    gcmFOOTER();
    return *FirstStr;
}

clsATTRIBUTE *
clParseAttributeEndianType(
IN cloCOMPILER Compiler,
IN clsATTRIBUTE *Attr,
IN clsLexToken *EndianType
)
{
   gceSTATUS status;
   clsATTRIBUTE *attr;
   gctBOOL hostEndian;

   gcmASSERT(EndianType);

   if (gcmIS_SUCCESS(gcoOS_StrCmp(EndianType->u.identifier.name, "host"))) {
      hostEndian = gcvTRUE;
   }
   else if (gcmIS_SUCCESS(gcoOS_StrCmp(EndianType->u.identifier.name, "device"))) {
      hostEndian = gcvFALSE;
   }
   else {
      gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                      EndianType->lineNo,
                                      EndianType->stringNo,
                                      clvREPORT_ERROR,
                                      "invalid endian type \'%s\'",
                                      EndianType->u.identifier.name));
      return gcvNULL;
   }
   if(Attr == gcvNULL) {
     gctPOINTER pointer;

     status = cloCOMPILER_Allocate(Compiler,
                                   (gctSIZE_T)sizeof(clsATTRIBUTE),
                                   (gctPOINTER *) &pointer);
     if (gcmIS_ERROR(status)) return Attr;
     attr = pointer;
     (void)gcoOS_ZeroMemory((gctPOINTER)attr, sizeof(clsATTRIBUTE));
   }
   else {
     if(Attr->specifiedAttr & clvATTR_ENDIAN) {
       gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                       EndianType->lineNo,
                                       EndianType->stringNo,
                                       clvREPORT_ERROR,
                                       "endian type attribute already defined"));
       return Attr;
     }
     attr = Attr;
   }

   attr->specifiedAttr |= clvATTR_ENDIAN;
   attr->hostEndian = hostEndian;
   return attr;
}

static gctCONST_STRING
_GetTokenName(IN gctINT TokenType)
{
   int index;

   switch(TokenType) {
   case T_FLOATNXM:
    return "floatnxm";

   case T_DOUBLENXM:
    return "doublenxm";

   default:
    index = TokenType - T_VERY_FIRST_TERMINAL;
    if(index > 0 && index < cldNumTerminalTokens) {
      return _IndexKeywordStrings[index];
        }
        else {
      gcmASSERT(0);
      return "invalid";
    }
   }
}

clsATTRIBUTE *
clParseAttributeAligned(
IN cloCOMPILER Compiler,
IN clsATTRIBUTE *Attr,
IN cloIR_EXPR Alignment
)
{
   gceSTATUS status;
   clsATTRIBUTE *attr;
   gctINT alignment;

   if(Attr == gcvNULL) {
     gctPOINTER pointer;

     status = cloCOMPILER_Allocate(Compiler,
                                   (gctSIZE_T)sizeof(clsATTRIBUTE),
                                   (gctPOINTER *) &pointer);
     if (gcmIS_ERROR(status)) return Attr;
     attr = pointer;
     (void)gcoOS_ZeroMemory((gctPOINTER)attr, sizeof(clsATTRIBUTE));
   }
   else {
     if(Attr->specifiedAttr & clvATTR_ALIGNED) {
       if(Alignment) {
           gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                           Alignment->base.lineNo,
                                           Alignment->base.stringNo,
                                           clvREPORT_ERROR,
                                           "\'aligned\' attribute already defined"));
       }
       else {
           gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                           0,
                                           0,
                                           clvREPORT_ERROR,
                                           "\'aligned\' attribute already defined"));
       }
       return Attr;
     }
     attr = Attr;
   }

   if(Alignment) {
     status = _EvaluateExprToInteger(Compiler, Alignment, &alignment);
     if (gcmIS_ERROR(status)) return attr;

     if(!clIsPowerOf2(alignment)) {
       gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                       Alignment->base.lineNo,
                                       Alignment->base.stringNo,
                                       clvREPORT_ERROR,
                                       "\'aligned\' attribute's alignment value \'%d\' not a power of two",
                                       alignment));
       return attr;
     }
   }
   else {
     alignment = cldMachineByteAlignment;
   }
   attr->specifiedAttr |= clvATTR_ALIGNED;
   attr->alignment = (gctUINT16)alignment;
   return attr;
}

clsATTRIBUTE *
clParseAttributeReqdWorkGroupSize(
IN cloCOMPILER Compiler,
IN clsATTRIBUTE *Attr,
IN cloIR_EXPR X,
IN cloIR_EXPR Y,
IN cloIR_EXPR Z
)
{
   gceSTATUS status;
   clsATTRIBUTE *attr;
   gctINT x, y, z;

   status = _EvaluateExprToInteger(Compiler, X, &x);
   if (gcmIS_ERROR(status)) return Attr;

   status = _EvaluateExprToInteger(Compiler, Y, &y);
   if (gcmIS_ERROR(status)) return Attr;

   status = _EvaluateExprToInteger(Compiler, Z, &z);
   if (gcmIS_ERROR(status)) return Attr;

   if(Attr == gcvNULL) {
     gctPOINTER pointer;

     status = cloCOMPILER_Allocate(Compiler,
                                   (gctSIZE_T)sizeof(clsATTRIBUTE),
                                   (gctPOINTER *) &pointer);
     if (gcmIS_ERROR(status)) return Attr;
     attr = pointer;
     (void)gcoOS_ZeroMemory((gctPOINTER)attr, sizeof(clsATTRIBUTE));
   }
   else {
     if(Attr->specifiedAttr & clvATTR_REQD_WORK_GROUP_SIZE) {
       gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                       X->base.lineNo,
                                       X->base.stringNo,
                                       clvREPORT_ERROR,
                                       "reqd_work_group_size attribute already defined"));
       return Attr;
     }
     attr = Attr;
   }
   attr->reqdWorkGroupSize[0] = x;
   attr->reqdWorkGroupSize[1] = y;
   attr->reqdWorkGroupSize[2] = z;
   attr->specifiedAttr |= clvATTR_REQD_WORK_GROUP_SIZE;
   return attr;
}

clsATTRIBUTE *
clParseAttributeWorkGroupSizeHint(
IN cloCOMPILER Compiler,
IN clsATTRIBUTE *Attr,
IN cloIR_EXPR X,
IN cloIR_EXPR Y,
IN cloIR_EXPR Z
)
{
   gceSTATUS status;
   clsATTRIBUTE *attr;
   gctINT x, y, z;

   status = _EvaluateExprToInteger(Compiler, X, &x);
   if (gcmIS_ERROR(status)) return Attr;

   status = _EvaluateExprToInteger(Compiler, Y, &y);
   if (gcmIS_ERROR(status)) return Attr;

   status = _EvaluateExprToInteger(Compiler, Z, &z);
   if (gcmIS_ERROR(status)) return Attr;

   if(Attr == gcvNULL) {
     gctPOINTER pointer;

     status = cloCOMPILER_Allocate(Compiler,
                                   (gctSIZE_T)sizeof(clsATTRIBUTE),
                                   (gctPOINTER *) &pointer);
     if (gcmIS_ERROR(status)) return Attr;
     attr = pointer;
     (void)gcoOS_ZeroMemory((gctPOINTER)attr, sizeof(clsATTRIBUTE));
   }
   else {
     if(Attr->specifiedAttr & clvATTR_WORK_GROUP_SIZE_HINT) {
       gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                       X->base.lineNo,
                                       X->base.stringNo,
                                       clvREPORT_ERROR,
                                       "work_group_size_hint attribute already defined"));
       return Attr;
     }
     attr = Attr;
   }
   attr->workGroupSizeHint[0] = x;
   attr->workGroupSizeHint[1] = y;
   attr->workGroupSizeHint[2] = z;
   attr->specifiedAttr |= clvATTR_WORK_GROUP_SIZE_HINT;
   return attr;
}

clsATTRIBUTE *
clParseAttributeKernelScaleHint(
IN cloCOMPILER Compiler,
IN clsATTRIBUTE *Attr,
IN cloIR_EXPR X,
IN cloIR_EXPR Y,
IN cloIR_EXPR Z
)
{
   gceSTATUS status;
   clsATTRIBUTE *attr;
   gctINT x, y, z;

   status = _EvaluateExprToInteger(Compiler, X, &x);
   if (gcmIS_ERROR(status)) return Attr;

   status = _EvaluateExprToInteger(Compiler, Y, &y);
   if (gcmIS_ERROR(status)) return Attr;

   status = _EvaluateExprToInteger(Compiler, Z, &z);
   if (gcmIS_ERROR(status)) return Attr;

   if(Attr == gcvNULL) {
     gctPOINTER pointer;

     status = cloCOMPILER_Allocate(Compiler,
                                   (gctSIZE_T)sizeof(clsATTRIBUTE),
                                   (gctPOINTER *) &pointer);
     if (gcmIS_ERROR(status)) return Attr;
     attr = pointer;
     (void)gcoOS_ZeroMemory((gctPOINTER)attr, sizeof(clsATTRIBUTE));
   }
   else {
     if(Attr->specifiedAttr & clvATTR_KERNEL_SCALE_HINT) {
       gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                       X->base.lineNo,
                                       X->base.stringNo,
                                       clvREPORT_ERROR,
                                       "kernel_scale_hint attribute already defined"));
       return Attr;
     }
     attr = Attr;
   }
   attr->kernelScaleHint[0] = x;
   attr->kernelScaleHint[1] = y;
   attr->kernelScaleHint[2] = z;
   attr->specifiedAttr |= clvATTR_KERNEL_SCALE_HINT;
   return attr;
}

clsATTRIBUTE *
clParseAttributeVecTypeHint(
IN cloCOMPILER Compiler,
IN clsATTRIBUTE *Attr,
IN clsLexToken *DataType
)
{
   gceSTATUS status;
   clsATTRIBUTE *attr;

   gcmASSERT(DataType);

   if(Attr == gcvNULL) {
     gctPOINTER pointer;

     status = cloCOMPILER_Allocate(Compiler,
                                   (gctSIZE_T)sizeof(clsATTRIBUTE),
                                   (gctPOINTER *) &pointer);
     if (gcmIS_ERROR(status)) return Attr;
     attr = pointer;
     (void)gcoOS_ZeroMemory((gctPOINTER)attr, sizeof(clsATTRIBUTE));
   }
   else {
     if(Attr->specifiedAttr & clvATTR_VEC_TYPE_HINT) {
       gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                       DataType->lineNo,
                                       DataType->stringNo,
                                       clvREPORT_ERROR,
                                       "vec_type_hint attribute already defined"));
       return Attr;
     }
     attr = Attr;
   }

   attr->vecTypeHint = DataType->type;
   attr->specifiedAttr |= clvATTR_VEC_TYPE_HINT;
   return attr;
}

clsATTRIBUTE *
clParseSimpleAttribute(
IN cloCOMPILER Compiler,
IN clsLexToken *StartToken,
IN cltATTR_FLAGS AttrType,
IN clsATTRIBUTE *Attr
)
{
   gceSTATUS status;
   clsATTRIBUTE *attr;

   if(Attr == gcvNULL) {
     gctPOINTER pointer;

     status = cloCOMPILER_Allocate(Compiler,
                                   (gctSIZE_T)sizeof(clsATTRIBUTE),
                                   (gctPOINTER *) &pointer);
     if (gcmIS_ERROR(status)) return Attr;
     attr = pointer;
     (void)gcoOS_ZeroMemory((gctPOINTER)attr, sizeof(clsATTRIBUTE));
   }
   else {
     if(Attr->specifiedAttr & AttrType) {
       gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                       StartToken->lineNo,
                                       StartToken->stringNo,
                                       clvREPORT_ERROR,
                                       "\"%s\" attribute already defined",
                                       _GetTokenName(StartToken->type)));
       return Attr;
     }
     attr = Attr;
   }

   switch(AttrType) {
   case clvATTR_ALWAYS_INLINE:
      attr->alwaysInline = gcvTRUE;
      break;
   case clvATTR_PACKED:
      attr->packed = gcvTRUE;
      break;

   default:
      gcmASSERT(0);
   }
   attr->specifiedAttr |= AttrType;
   return attr;
}

clsATTRIBUTE *
clParseMergeAttributeSpecifier(
IN cloCOMPILER Compiler,
IN clsATTRIBUTE *Attr,
IN clsATTRIBUTE *ToAttr
)
{
   gctUINT lineNo = cloCOMPILER_GetCurrentLineNo(Compiler);
   gctUINT stringNo = cloCOMPILER_GetCurrentStringNo(Compiler);

   gcmASSERT(ToAttr);

/* check for PACKED attributes */
   if(Attr->specifiedAttr & clvATTR_PACKED) {
       if(ToAttr->specifiedAttr & clvATTR_PACKED) {
           gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                           lineNo,
                                           stringNo,
                                           clvREPORT_ERROR,
                                           "packed attribute already defined"));
       }
       else {
           ToAttr->packed = gcvTRUE;
           ToAttr->specifiedAttr |= clvATTR_PACKED;
       }
   }

/* check for ALWAYS_INLINE attributes */
   if(Attr->specifiedAttr & clvATTR_ALWAYS_INLINE) {
       if(ToAttr->specifiedAttr & clvATTR_ALWAYS_INLINE) {
           gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                           lineNo,
                                           stringNo,
                                           clvREPORT_ERROR,
                                           "always_inline attribute already defined"));
       }
       else {
           ToAttr->alwaysInline = gcvTRUE;
           ToAttr->specifiedAttr |= clvATTR_ALWAYS_INLINE;
       }
   }

/* check for ENDIAN TYPE attributes */
   if(Attr->specifiedAttr & clvATTR_ENDIAN) {
       if(ToAttr->specifiedAttr & clvATTR_ENDIAN) {
           gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                           lineNo,
                                           stringNo,
                                           clvREPORT_ERROR,
                                           "endian attribute already defined"));
       }
       else {
           ToAttr->hostEndian = Attr->hostEndian;
           ToAttr->specifiedAttr |= clvATTR_ENDIAN;
       }
   }

/* check for VEC_TYPE_HINT attributes */
   if(Attr->specifiedAttr & clvATTR_VEC_TYPE_HINT) {
       if(ToAttr->specifiedAttr & clvATTR_VEC_TYPE_HINT) {
           gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                           lineNo,
                                           stringNo,
                                           clvREPORT_ERROR,
                                           "vec_type_hint attribute already defined"));
       }
       else {
           ToAttr->vecTypeHint = Attr->vecTypeHint;
           ToAttr->specifiedAttr |= clvATTR_VEC_TYPE_HINT;
       }
   }

/* check for REQ_WORK_GROUP_SIZE attributes */
   if(Attr->specifiedAttr & clvATTR_REQD_WORK_GROUP_SIZE) {
       if(ToAttr->specifiedAttr & clvATTR_REQD_WORK_GROUP_SIZE) {
           gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                           lineNo,
                                           stringNo,
                                           clvREPORT_ERROR,
                                           "reqd_work_group_size attribute already defined"));
       }
       else {
           ToAttr->reqdWorkGroupSize[0] = Attr->reqdWorkGroupSize[0];
           ToAttr->reqdWorkGroupSize[1] = Attr->reqdWorkGroupSize[1];
           ToAttr->reqdWorkGroupSize[2] = Attr->reqdWorkGroupSize[2];
           ToAttr->specifiedAttr |= clvATTR_REQD_WORK_GROUP_SIZE;
       }
   }

/* check for WORK_GROUP_SIZE_HINT attributes */
   if(Attr->specifiedAttr & clvATTR_WORK_GROUP_SIZE_HINT) {
       if(ToAttr->specifiedAttr & clvATTR_WORK_GROUP_SIZE_HINT) {
           gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                           lineNo,
                                           stringNo,
                                           clvREPORT_ERROR,
                                           "work_group_size_hint attribute already defined"));
       }
       else {
           ToAttr->workGroupSizeHint[0] = Attr->workGroupSizeHint[0];
           ToAttr->workGroupSizeHint[1] = Attr->workGroupSizeHint[1];
           ToAttr->workGroupSizeHint[2] = Attr->workGroupSizeHint[2];
           ToAttr->specifiedAttr |= clvATTR_WORK_GROUP_SIZE_HINT;
       }
   }

/* check for KERNEL_SCALE_HINT attributes */
   if(Attr->specifiedAttr & clvATTR_KERNEL_SCALE_HINT) {
       if(ToAttr->specifiedAttr & clvATTR_KERNEL_SCALE_HINT) {
           gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                           lineNo,
                                           stringNo,
                                           clvREPORT_ERROR,
                                           "kernel_scale_hint attribute already defined"));
       }
       else {
           ToAttr->kernelScaleHint[0] = Attr->kernelScaleHint[0];
           ToAttr->kernelScaleHint[1] = Attr->kernelScaleHint[1];
           ToAttr->kernelScaleHint[2] = Attr->kernelScaleHint[2];
           ToAttr->specifiedAttr |= clvATTR_KERNEL_SCALE_HINT;
       }
   }

/* check for ALIGNED attributes */
   if(Attr->specifiedAttr & clvATTR_ALIGNED) {
       if(ToAttr->specifiedAttr & clvATTR_ALIGNED) {
           gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                           lineNo,
                                           stringNo,
                                           clvREPORT_ERROR,
                                           "aligned attribute already defined"));
       }
       else {
           ToAttr->alignment = Attr->alignment;
           ToAttr->specifiedAttr |= clvATTR_ALIGNED;
       }
   }

   return ToAttr;
}

gceSTATUS
_CheckSubscriptExpr(
IN cloCOMPILER Compiler,
IN cloIR_EXPR LeftOperand,
IN cloIR_EXPR RightOperand
)
{
    gctINT32  index;

    gcmASSERT(LeftOperand);
    gcmASSERT(LeftOperand->decl.dataType);
    gcmASSERT(RightOperand);
    gcmASSERT(RightOperand->decl.dataType);

    /* Check the left operand */
    if (!clmDECL_IsVectorType(&LeftOperand->decl)
        && !clmDECL_IsMat(&LeftOperand->decl)
        && !clmDECL_IsArray(&LeftOperand->decl)
        && !clmDECL_IsPointerType(&LeftOperand->decl)) {
        gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                        LeftOperand->base.lineNo,
                        LeftOperand->base.stringNo,
                        clvREPORT_ERROR,
                        "require an array or matrix or vector "
                        "or pointer typed expression"));
        return gcvSTATUS_INVALID_ARGUMENT;
    }

    /* Check the right operand */
    if (!clmDECL_IsScalarInteger(&RightOperand->decl)) {
        gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                        RightOperand->base.lineNo,
                        RightOperand->base.stringNo,
                        clvREPORT_ERROR,
                        "require a scalar integer expression"));
        return gcvSTATUS_INVALID_ARGUMENT;
    }

    /* Check constant index range */
    if(!clmDECL_IsPointerType(&LeftOperand->decl)
        && cloIR_OBJECT_GetType(&RightOperand->base) == clvIR_CONSTANT) {
        gcmASSERT(((cloIR_CONSTANT)RightOperand)->valueCount == 1);
        index = ((cloIR_CONSTANT)RightOperand)->values[0].intValue;
        if (index < 0) {
            gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                            RightOperand->base.lineNo,
                            RightOperand->base.stringNo,
                            clvREPORT_ERROR,
                            "require a nonnegative index"));
            return gcvSTATUS_INVALID_ARGUMENT;
        }
        if (clmDECL_IsVectorType(&LeftOperand->decl)) {
            if (index >= clmDATA_TYPE_vectorSize_NOCHECK_GET(LeftOperand->decl.dataType)) {
                gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                RightOperand->base.lineNo,
                                RightOperand->base.stringNo,
                                clvREPORT_ERROR,
                                "the index exceeds the vector type size"));
                return gcvSTATUS_INVALID_ARGUMENT;
            }
        }
        else if (clmDECL_IsMat(&LeftOperand->decl)) {
            if (index >= clmDATA_TYPE_matrixColumnCount_GET(LeftOperand->decl.dataType)) {
                gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                RightOperand->base.lineNo,
                                RightOperand->base.stringNo,
                                clvREPORT_ERROR,
                                "the index exceeds the matrix type size"));
                return gcvSTATUS_INVALID_ARGUMENT;
            }
        }
        else if (clmDECL_IsArray(&LeftOperand->decl)) {
            if ((gctINT)index >= LeftOperand->decl.array.length[0]) {
                gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                RightOperand->base.lineNo,
                                RightOperand->base.stringNo,
                                clvREPORT_ERROR,
                                "the index exceeds the array type size"));
                return gcvSTATUS_INVALID_ARGUMENT;
            }
        }
    }
    return gcvSTATUS_OK;
}

cloIR_EXPR
clParseSubscriptExpr(
IN cloCOMPILER Compiler,
IN cloIR_EXPR LeftOperand,
IN cloIR_EXPR RightOperand
)
{
    gceSTATUS  status;
    cloIR_CONSTANT resultConstant;
    cloIR_BINARY_EXPR binaryExpr;
    cloIR_EXPR resExpr;

    if (LeftOperand == gcvNULL || RightOperand == gcvNULL) return gcvNULL;

    /* Check error */
    status = _CheckSubscriptExpr(Compiler,
                                 LeftOperand,
                                 RightOperand);
    if (gcmIS_ERROR(status)) return gcvNULL;

    if (!clmDECL_IsPointerType(&LeftOperand->decl)) {
        clsNAME *leafName;
        leafName = clParseFindLeafName(Compiler,
                                       LeftOperand);
        if(leafName && !clmDECL_IsPointerType(&leafName->decl)) {
            if (_clmExprIsConstantForEval(RightOperand)) {
                /* Constant calculation */
                if (_clmExprIsConstantForEval(LeftOperand) &&
                   leafName->u.variableInfo.isUnnamedConstant &&  /* an unnamed constant variable */
                   (!clmDECL_IsArray(&leafName->decl) ||
                   leafName->decl.array.numDim == 1)) {
                    clsDECL decl[1];

                    status = cloCOMPILER_CreateElementDecl(Compiler,
                                                           &LeftOperand->decl,
                                                           decl);
                    if (gcmIS_ERROR(status)) return gcvNULL;

                    status = cloIR_BINARY_EXPR_Evaluate(Compiler,
                                                        clvBINARY_SUBSCRIPT,
                                                        (cloIR_CONSTANT)LeftOperand,
                                                        (cloIR_CONSTANT)RightOperand,
                                                        decl,
                                                        &resultConstant);
                    if (gcmIS_ERROR(status)) return gcvNULL;
                    return &resultConstant->exprBase;
                }
            }
            else { /* variable indexing to array or vector elements */
                if(clmDECL_IsArray(&LeftOperand->decl) &&
                   LeftOperand->decl.dataType->type != T_IMAGE2D_DYNAMIC_ARRAY_T) {
                    gctUINT elementCount;

                    clmGetArrayElementCount(LeftOperand->decl.array, 0, elementCount);
                    if(elementCount > _clmMaxOperandCountToUseMemory(&LeftOperand->decl)) {
                        status = clParseSetOperandAddressed(Compiler,
                                                            LeftOperand);
                        if (gcmIS_ERROR(status)) return gcvNULL;
                    }
                }
                leafName->u.variableInfo.indirectlyAddressed = gcvTRUE;
            }
        }
        /* Create the binary expression */
        if(clmIR_EXPR_IsBinaryType(LeftOperand, clvBINARY_SUBSCRIPT)) {
            binaryExpr = (cloIR_BINARY_EXPR) &LeftOperand->base;
            status = cloIR_BINARY_EXPR_Construct(Compiler,
                                                 LeftOperand->base.lineNo,
                                                 LeftOperand->base.stringNo,
                                                 clvBINARY_MULTI_DIM_SUBSCRIPT,
                                                 binaryExpr->rightOperand,
                                                 RightOperand,
                                                 &resExpr);
            if (gcmIS_ERROR(status)) return gcvNULL;

            binaryExpr->rightOperand = resExpr;
            status = cloCOMPILER_CreateElementDecl(Compiler,
                                                   &binaryExpr->exprBase.decl,
                                                   &binaryExpr->exprBase.decl);
            if (gcmIS_ERROR(status)) return gcvNULL;
            resExpr = &binaryExpr->exprBase;
        }
        else {
            status = cloIR_BINARY_EXPR_Construct(Compiler,
                                                 LeftOperand->base.lineNo,
                                                 LeftOperand->base.stringNo,
                                                 clvBINARY_SUBSCRIPT,
                                                 LeftOperand,
                                                 RightOperand,
                                                 &resExpr);
            if (gcmIS_ERROR(status)) return gcvNULL;
        }
    }
    else {
        status = cloIR_BINARY_EXPR_Construct(Compiler,
                                             LeftOperand->base.lineNo,
                                             LeftOperand->base.stringNo,
                                             clvBINARY_SUBSCRIPT,
                                             LeftOperand,
                                             RightOperand,
                                             &resExpr);
        if (gcmIS_ERROR(status)) return gcvNULL;
    }

    binaryExpr = (cloIR_BINARY_EXPR) &resExpr->base;
/* Reset resulting expression's access qualifier to none according to following criterion */
    if(binaryExpr->exprBase.decl.dataType->addrSpaceQualifier != clvQUALIFIER_CONSTANT &&
       cloIR_OBJECT_GetType(&LeftOperand->base) != clvIR_CONSTANT &&
       clmDECL_IsPointerType(&LeftOperand->decl) &&
       binaryExpr->exprBase.decl.dataType->accessQualifier == clvQUALIFIER_CONST) {
        status = cloCOMPILER_CloneDataType(Compiler,
                                           clvQUALIFIER_NONE,
                                           binaryExpr->exprBase.decl.dataType->addrSpaceQualifier,
                                           binaryExpr->exprBase.decl.dataType,
                                           &binaryExpr->exprBase.decl.dataType);
        if(gcmIS_ERROR(status)) return gcvNULL;
    }

    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                      clvDUMP_PARSER,
                      "<SUBSCRIPT_EXPR line=\"%d\" string=\"%d\" />",
                      LeftOperand->base.lineNo,
                      LeftOperand->base.stringNo));
    return resExpr;
}

static gceSTATUS
_CheckScalarConstructor(
IN cloCOMPILER Compiler,
IN cloIR_POLYNARY_EXPR PolynaryExpr
)
{
    gctUINT        operandCount;
    cloIR_EXPR    operand;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(PolynaryExpr, clvIR_POLYNARY_EXPR);

    if (PolynaryExpr->operands == gcvNULL) {
        operandCount = 0;
    }
    else {
        gcmVERIFY_OK(cloIR_SET_GetMemberCount(Compiler,
                              PolynaryExpr->operands,
                              &operandCount));
    }

    if (operandCount != 1) {
        gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                        PolynaryExpr->exprBase.base.lineNo,
                        PolynaryExpr->exprBase.base.stringNo,
                        clvREPORT_ERROR,
                        "require one expression"));
        return gcvSTATUS_INVALID_ARGUMENT;
    }

    operand = slsDLINK_LIST_First(&PolynaryExpr->operands->members, struct _cloIR_EXPR);
    gcmASSERT(operand);
    gcmASSERT(operand->decl.dataType);

    if (!clmDECL_IsBoolOrBVec(&operand->decl)
        && !clmDECL_IsIntOrIVec(&operand->decl)
        && !clmDECL_IsFloatOrVecOrMat(&operand->decl)) {
        gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                        operand->base.lineNo,
                        operand->base.stringNo,
                        clvREPORT_ERROR,
                        "require a boolean or integer or floating-point"
                        " typed expression"));
        return gcvSTATUS_INVALID_ARGUMENT;
    }
    return gcvSTATUS_OK;
}

static gceSTATUS
_CheckArrayConstructor(
IN cloCOMPILER Compiler,
IN cloIR_POLYNARY_EXPR PolynaryExpr
)
{
    gctUINT    operandCount, elementCount;
    cloIR_EXPR operand;
    clsDECL refDecl[1];

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(PolynaryExpr, clvIR_POLYNARY_EXPR);

    if (PolynaryExpr->operands == gcvNULL) {
        operandCount = 0;
    }
    else {
        gcmVERIFY_OK(cloIR_SET_GetMemberCount(Compiler,
                              PolynaryExpr->operands,
                              &operandCount));
    }

    gcmASSERT(clmDECL_IsArray(&PolynaryExpr->exprBase.decl));
    clmGetArrayElementCount(PolynaryExpr->exprBase.decl.array, 0, elementCount);
    if (operandCount != elementCount) {
        gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                        PolynaryExpr->exprBase.base.lineNo,
                        PolynaryExpr->exprBase.base.stringNo,
                        clvREPORT_ERROR,
                        "expected number of elements for array not matching"));
        return gcvSTATUS_INVALID_ARGUMENT;
    }
    if (PolynaryExpr->operands == gcvNULL) {
        gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                        PolynaryExpr->exprBase.base.lineNo,
                        PolynaryExpr->exprBase.base.stringNo,
                        clvREPORT_ERROR,
                        "null pointer not allow"));
        return gcvSTATUS_INVALID_ARGUMENT;
    }

    operand = slsDLINK_LIST_First(&PolynaryExpr->operands->members, struct _cloIR_EXPR);
    gcmASSERT(operand);
    gcmASSERT(operand->decl.dataType);

    refDecl[0] = PolynaryExpr->exprBase.decl;
    refDecl->array.numDim = 0;

    FOR_EACH_DLINK_NODE(&PolynaryExpr->operands->members, struct _cloIR_EXPR, operand) {
        gcmASSERT(operand);
        gcmASSERT(operand->decl.dataType);

        if (clmDECL_IsArithmeticType(refDecl) &&
                !clmDECL_IsArithmeticType(&operand->decl)) {
           gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                           operand->base.lineNo,
                           operand->base.stringNo,
                           clvREPORT_ERROR,
                           "require any boolean or integer or floating-point"
                           " typed expression"));
            return gcvSTATUS_INVALID_ARGUMENT;
        }
        if (!clsDECL_IsInitializableTo(refDecl, &operand->decl)) {
            gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                            operand->base.lineNo,
                            operand->base.stringNo,
                            clvREPORT_ERROR,
                            "require a matching typed expression"));
            return gcvSTATUS_INVALID_ARGUMENT;
        }
    }
    return gcvSTATUS_OK;
}

static gceSTATUS
_CheckVectorOrMatrixConstructor(
IN cloCOMPILER Compiler,
IN cloIR_POLYNARY_EXPR PolynaryExpr,
IN gctBOOL IsVectorConstructor
)
{
    gctUINT        operandCount;
    cloIR_EXPR    operand;
    gctSIZE_T    operandDataSizes = 0;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(PolynaryExpr, clvIR_POLYNARY_EXPR);

    if (PolynaryExpr->operands == gcvNULL) {
        operandCount = 0;
    }
    else {
        gcmVERIFY_OK(cloIR_SET_GetMemberCount(Compiler,
                        PolynaryExpr->operands,
                        &operandCount));
    }

    if (operandCount == 0) {
        gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                        PolynaryExpr->exprBase.base.lineNo,
                        PolynaryExpr->exprBase.base.stringNo,
                        clvREPORT_ERROR,
                        "require at least one expression"));
        return gcvSTATUS_INVALID_ARGUMENT;
    }

    FOR_EACH_DLINK_NODE(&PolynaryExpr->operands->members, struct _cloIR_EXPR, operand) {
        gcmASSERT(operand);
        gcmASSERT(operand->decl.dataType);
        if (!clmDECL_IsBoolOrBVec(&operand->decl)
            && !clmDECL_IsIntOrIVec(&operand->decl)
            && !clmDECL_IsFloatOrVecOrMat(&operand->decl)) {
            gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                            operand->base.lineNo,
                            operand->base.stringNo,
                            clvREPORT_ERROR,
                            "require any boolean or integer or floating-point"
                            " typed expression"));
            return gcvSTATUS_INVALID_ARGUMENT;
        }
        if(clmDECL_IsVectorType(&operand->decl)
                && (operand->decl.dataType->elementType != PolynaryExpr->exprBase.decl.dataType->elementType)) {
            gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                            operand->base.lineNo,
                            operand->base.stringNo,
                            clvREPORT_ERROR,
                            "conversion between vector types "
                            "not allowed"));
            return gcvSTATUS_INVALID_ARGUMENT;
        }
        if (operandDataSizes >= clsDECL_GetSize(&PolynaryExpr->exprBase.decl)) {
            gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            clvREPORT_ERROR,
                            "too many expressions in the constructor"));
            return gcvSTATUS_INVALID_ARGUMENT;
        }
        operandDataSizes += clsDECL_GetSize(&operand->decl);
    }

    if (operandCount == 1) {
        operand = slsDLINK_LIST_First(&PolynaryExpr->operands->members, struct _cloIR_EXPR);
        gcmASSERT(operand);

        if (IsVectorConstructor) {
           if (clmDECL_IsScalar(&operand->decl)) {
            return gcvSTATUS_OK;
           }
        }
        else {
              if (clmDECL_IsScalar(&operand->decl) || clmDECL_IsMat(&operand->decl)) {
             return gcvSTATUS_OK;
          }
        }
    }

    if (operandDataSizes < clsDECL_GetSize(&PolynaryExpr->exprBase.decl)) {
        gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                        PolynaryExpr->exprBase.base.lineNo,
                        PolynaryExpr->exprBase.base.stringNo,
                        clvREPORT_ERROR,
                        "require more expressions"));
        return gcvSTATUS_INVALID_ARGUMENT;
    }
    return gcvSTATUS_OK;
}

#define _clmIsFieldDeclEqual(Fld, Arg)  \
    ((clmDECL_IsIntegerType(Fld) && clmDECL_IsIntegerType(Arg) && \
      clmDECL_IsScalar(Fld) && clmDECL_IsScalar(Arg)) || \
     clsDECL_IsEqual((Fld), (Arg)))

static gceSTATUS
_CheckStructOrUnionMemberMatch(
IN cloCOMPILER Compiler,
IN cloIR_POLYNARY_EXPR PolynaryExpr,
IN clsDECL *Decl,
IN cloIR_EXPR  *Operand
)
{
    gceSTATUS status = gcvSTATUS_OK;
    clsNAME *   fieldName;
    gctBOOL matched = gcvFALSE;
    cloIR_EXPR operand;

    gcmASSERT(Operand);

    operand = *Operand;
    if (clmDATA_TYPE_IsStruct(Decl->dataType)) {
        for (fieldName = slsDLINK_LIST_First(&Decl->dataType->u.fieldSpace->names, clsNAME);
            (slsDLINK_NODE *)fieldName != &Decl->dataType->u.fieldSpace->names;
            fieldName = slsDLINK_NODE_Next(&fieldName->node, clsNAME),
            operand = slsDLINK_NODE_Next(&operand->base.node, struct _cloIR_EXPR)) {

            if ((slsDLINK_NODE *)Operand == &PolynaryExpr->operands->members) {
                gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                                PolynaryExpr->exprBase.base.lineNo,
                                                PolynaryExpr->exprBase.base.stringNo,
                                                clvREPORT_ERROR,
                                                "require more expressions"));
                return gcvSTATUS_INVALID_ARGUMENT;
            }

            gcmONERROR(_CheckStructOrUnionMemberMatch(Compiler,
                                                      PolynaryExpr,
                                                      &fieldName->decl,
                                                      &operand));
        }
        matched = gcvTRUE;
        *Operand = operand;
        return gcvSTATUS_OK;
    }
    else if (clmDATA_TYPE_IsUnion(Decl->dataType)) {
        matched = gcvFALSE;
        for (fieldName = slsDLINK_LIST_First(&Decl->dataType->u.fieldSpace->names, clsNAME);
            (slsDLINK_NODE *)fieldName != &Decl->dataType->u.fieldSpace->names;
            fieldName = slsDLINK_NODE_Next(&fieldName->node, clsNAME)) {
            if (_clmIsFieldDeclEqual(&fieldName->decl, &operand->decl)) {
                operand = slsDLINK_NODE_Next(&operand->base.node, struct _cloIR_EXPR);
                matched = gcvTRUE;
                break;
            }
            else {
                continue;
            }
        }
    }
    else matched = _clmIsFieldDeclEqual(Decl, &operand->decl);

OnError:
    if(matched) {
        *Operand = operand;
        return gcvSTATUS_OK;
    }

    gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                    operand->base.lineNo,
                                    operand->base.stringNo,
                                    clvREPORT_ERROR,
                                    "require the same typed expression"));
    return gcvSTATUS_INVALID_ARGUMENT;
}

static gceSTATUS
_CheckStructConstructor(
IN cloCOMPILER Compiler,
IN cloIR_POLYNARY_EXPR PolynaryExpr
)
{
    gceSTATUS    status;
    gctUINT      operandCount;
    cloIR_EXPR   operand;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(PolynaryExpr, clvIR_POLYNARY_EXPR);

    if (PolynaryExpr->operands == gcvNULL) {
        operandCount = 0;
    }
    else {
        gcmVERIFY_OK(cloIR_SET_GetMemberCount(Compiler,
                        PolynaryExpr->operands,
                        &operandCount));
    }
    if (operandCount == 0) {
        gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                        PolynaryExpr->exprBase.base.lineNo,
                        PolynaryExpr->exprBase.base.stringNo,
                        clvREPORT_ERROR,
                        "require at least one expression"));
        return gcvSTATUS_INVALID_ARGUMENT;
    }

    gcmASSERT(PolynaryExpr->exprBase.decl.dataType);
    gcmASSERT(clmDATA_TYPE_IsStructOrUnion(PolynaryExpr->exprBase.decl.dataType));

    operand = slsDLINK_LIST_First(&PolynaryExpr->operands->members, struct _cloIR_EXPR);
    status = _CheckStructOrUnionMemberMatch(Compiler,
                                            PolynaryExpr,
                                            &PolynaryExpr->exprBase.decl,
                                            &operand);

    if(status == gcvSTATUS_OK &&
       (slsDLINK_NODE *)operand != &PolynaryExpr->operands->members) {
        gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                        operand->base.lineNo,
                                        operand->base.stringNo,
                                        clvREPORT_ERROR,
                                        "too many expressions"));
        return gcvSTATUS_INVALID_ARGUMENT;
    }

    return status;
}

static gceSTATUS
_CheckFuncCall(
IN cloCOMPILER Compiler,
IN cloIR_POLYNARY_EXPR PolynaryExpr
)
{
    gctUINT        operandCount, paramCount;
    cloIR_EXPR    operand;
    clsNAME *    paramName;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(PolynaryExpr, clvIR_POLYNARY_EXPR);
    gcmASSERT(PolynaryExpr->funcName);
    gcmASSERT(PolynaryExpr->funcName->u.funcInfo.localSpace);

    if (PolynaryExpr->operands == gcvNULL) {
        operandCount = 0;
    }
    else {
        gcmVERIFY_OK(cloIR_SET_GetMemberCount(Compiler,
                              PolynaryExpr->operands,
                              &operandCount));
    }

    if (operandCount == 0) {
        gcmVERIFY_OK(cloNAME_GetParamCount(Compiler,
                        PolynaryExpr->funcName,
                        &paramCount));

        if (paramCount != 0) {
            gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            clvREPORT_ERROR,
                            "require %d argument(s)",
                            paramCount));
            return gcvSTATUS_INVALID_ARGUMENT;
        }
        else {
            return gcvSTATUS_OK;
        }
    }

    for (paramName =
               slsDLINK_LIST_First(&PolynaryExpr->funcName->u.funcInfo.localSpace->names, clsNAME),
               operand = slsDLINK_LIST_First(&PolynaryExpr->operands->members, struct _cloIR_EXPR);
         (slsDLINK_NODE *)paramName != &PolynaryExpr->funcName->u.funcInfo.localSpace->names;
         paramName = slsDLINK_NODE_Next(&paramName->node, clsNAME),
            operand = slsDLINK_NODE_Next(&operand->base.node, struct _cloIR_EXPR)) {
        if (paramName->type != clvPARAMETER_NAME) break;

        if ((slsDLINK_NODE *)operand == &PolynaryExpr->operands->members) {
            gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            clvREPORT_ERROR,
                            "require more arguments"));
            return gcvSTATUS_INVALID_ARGUMENT;
        }
    }

    if ((slsDLINK_NODE *)operand != &PolynaryExpr->operands->members
        && !PolynaryExpr->funcName->u.funcInfo.hasVarArg) {
        gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                        operand->base.lineNo,
                                        operand->base.stringNo,
                                        clvREPORT_ERROR,
                                        "too many arguments"));
        return gcvSTATUS_INVALID_ARGUMENT;
    }
    return gcvSTATUS_OK;
}

static gceSTATUS
_ParseSetFuncCallArgumentDirty(
IN cloCOMPILER Compiler,
IN cloIR_POLYNARY_EXPR FuncCall
)
{
   if (FuncCall->operands == gcvNULL) {
       return gcvSTATUS_OK;
   }
   if(!FuncCall->funcName->isBuiltin ||
      FuncCall->funcName->u.funcInfo.hasWriteArg) {
      gceSTATUS status;
      cloIR_EXPR argument;

      FOR_EACH_DLINK_NODE(&FuncCall->operands->members, struct _cloIR_EXPR, argument) {
         status = clParseSetOperandDirty(Compiler,
                                         gcvNULL,
                                         argument);
         if (gcmIS_ERROR(status)) return status;
      }
   }
   return gcvSTATUS_OK;
}

cloIR_EXPR
clParseFuncCallExprAsExpr(
IN cloCOMPILER Compiler,
IN cloIR_POLYNARY_EXPR FuncCall
)
{
    gceSTATUS    status;
    cloIR_CONSTANT    constant;

    if (FuncCall == gcvNULL) return gcvNULL;

    switch (FuncCall->type) {
    case clvPOLYNARY_CONSTRUCT_SCALAR:
        status = _CheckScalarConstructor(Compiler, FuncCall);
        if (gcmIS_ERROR(status)) return gcvNULL;
        break;

    case clvPOLYNARY_CONSTRUCT_ARRAY:
        status = _CheckArrayConstructor(Compiler, FuncCall);
        if (gcmIS_ERROR(status)) return gcvNULL;
        break;

    case clvPOLYNARY_CONSTRUCT_VECTOR:
        status = _CheckVectorOrMatrixConstructor(Compiler, FuncCall, gcvTRUE);
        if (gcmIS_ERROR(status)) return gcvNULL;
        break;

    case clvPOLYNARY_CONSTRUCT_MATRIX:
        status = _CheckVectorOrMatrixConstructor(Compiler, FuncCall, gcvFALSE);
        if (gcmIS_ERROR(status)) return gcvNULL;
        break;

    case clvPOLYNARY_CONSTRUCT_STRUCT:
        status = _CheckStructConstructor(Compiler, FuncCall);
        if (gcmIS_ERROR(status)) return gcvNULL;
        break;

    case clvPOLYNARY_FUNC_CALL:
        status = cloCOMPILER_BindFuncCall(Compiler, FuncCall);
        if (gcmIS_ERROR(status)) return gcvNULL;

        status = _CheckFuncCall(Compiler, FuncCall);
        if (gcmIS_ERROR(status)) return gcvNULL;

        status = _ParseSetFuncCallArgumentDirty(Compiler, FuncCall);
        if (gcmIS_ERROR(status)) return gcvNULL;
        break;
    case clvPOLYNARY_BUILT_IN_ASM_CALL:

        break;

    default:
        gcmASSERT(0);
    }

    /* Try to evaluate it */
    status = cloIR_POLYNARY_EXPR_Evaluate(Compiler,
                        FuncCall,
                        &constant);
    if (gcmIS_ERROR(status)) return gcvNULL;
    return (constant != gcvNULL)? &constant->exprBase : &FuncCall->exprBase;
}

#define clmEvaluateExprToArrayLength(Compiler, Expr, Array, UnspecifiedOK, Status)  do { \
   if(UnspecifiedOK) { \
      if(clmIR_EXPR_IsUnaryType(Expr, clvUNARY_NULL)) { \
         (Array)->numDim = 1; \
         (Array)->length[0] = -1; \
     (Status) = gcvSTATUS_OK; \
     break; \
      } \
   } \
   (Array)->numDim = 0; \
   (Array)->length[0] = 0; \
   (Status) = _EvaluateExprToArrayLength(Compiler, Expr, UnspecifiedOK, Array); \
   } while(gcvFALSE)

gceSTATUS
_ParseMergeArrayDecl(
IN cloCOMPILER Compiler,
IN clsDECL *OrigDecl,
IN clsARRAY *Array,
OUT clsDECL *NewDecl
)
{
   clsARRAY *newArray;
   clsARRAY arrayBuf[1];

   if(clmDECL_IsArray(OrigDecl)) {
      gctINT i, j;
      gcmASSERT(!OrigDecl->ptrDominant &&
                ((OrigDecl->array.numDim + Array->numDim) <= cldMAX_ARRAY_DIMENSION));
      newArray = arrayBuf;
      *newArray = OrigDecl->array;
      for(i = 0, j = newArray->numDim; i < Array->numDim; i++, j++) {
         newArray->length[j] = (Array)->length[i];
      }
      newArray->numDim = j;
   }
   else {
      newArray = (Array);
   }
   return cloCOMPILER_CreateArrayDecl(Compiler,
                      OrigDecl->dataType,
                      newArray,
                      OrigDecl->ptrDscr,
                      NewDecl);
}

#define _clmIsValidVectorType(N) ((N) <= 4 || (N) == 8 || (N) == 16)
#define _cldHexadecimalDigits "0123456789ABCDEFabcdef"

gceSTATUS
_ParseComponentSelection(
IN cloCOMPILER Compiler,
IN gctUINT8 VectorSize,
IN clsLexToken * FieldSelection,
OUT clsCOMPONENT_SELECTION * ComponentSelection
)
{
   gctUINT8    i;
   gctUINT8    nameSets[cldMaxComponentCount];
   cleCOMPONENT    components[cldMaxComponentCount];

   gcmASSERT(VectorSize <= cldMaxComponentCount);
   gcmASSERT(FieldSelection);
   gcmASSERT(FieldSelection->u.fieldSelection);
   gcmASSERT(ComponentSelection);

   if (gcmIS_SUCCESS(gcoOS_StrCmp(FieldSelection->u.fieldSelection, "hi"))) {
       gctUINT8 start;
       start = (VectorSize + 1) >> 1;
       for(i = 0; i < start; i++) {
          ComponentSelection->selection[i] = start + i;
       }
       ComponentSelection->components = i;
   }
   else if (gcmIS_SUCCESS(gcoOS_StrCmp(FieldSelection->u.fieldSelection, "lo"))) {
       for(i = 0; i < ((VectorSize + 1) >> 1); i++) {
          ComponentSelection->selection[i] = i;
       }
       ComponentSelection->components = i;
   }
   else if (gcmIS_SUCCESS(gcoOS_StrCmp(FieldSelection->u.fieldSelection, "even"))) {
       for(i = 0; i < ((VectorSize + 1) >> 1); i++) {
          ComponentSelection->selection[i] = i << 1;
       }
       ComponentSelection->components = i;
   }
   else if (gcmIS_SUCCESS(gcoOS_StrCmp(FieldSelection->u.fieldSelection, "odd"))) {
       for(i = 0; i < ((VectorSize + 1) >> 1); i++) {
          ComponentSelection->selection[i] = (i << 1) + 1;
       }
       ComponentSelection->components = i;
   }
   else {
       gctINT isNumericIndex = 0;

       do {
          if ((FieldSelection->u.fieldSelection[0] == 's' ||
               FieldSelection->u.fieldSelection[0] == 'S') &&
               FieldSelection->u.fieldSelection[1] != '\0') {
              gctUINT selectLen;
              selectLen= clScanStrspn(FieldSelection->u.fieldSelection + 1, _cldHexadecimalDigits);
              if(selectLen) {
                 for (i=0, selectLen= 1; FieldSelection->u.fieldSelection[selectLen] != '\0'; i++, selectLen++) {

                     if(i >= cldMaxComponentCount) {
                        gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                                        FieldSelection->lineNo,
                                                        FieldSelection->stringNo,
                                                        clvREPORT_ERROR,
                                                        "more than %d components are selected : \"%s\"",
                                                        cldMaxComponentCount,
                                                        FieldSelection->u.fieldSelection));
                        return gcvSTATUS_INVALID_ARGUMENT;
                     }

                     switch(FieldSelection->u.fieldSelection[selectLen]) {
                     case '0':
                       components[i] = 0;
                       break;

                     case '1':
                       components[i] = 1;
                       break;

                     case '2':
                           components[i] = 2;
                           break;

                     case '3':
                       components[i] = 3;
                       break;

                     case '4':
                       components[i] = 4;
                       break;

                     case '5':
                       components[i] = 5;
                       break;

                     case '6':
                       components[i] = 6;
                       break;

                     case '7':
                       components[i] = 7;
                       break;

                     case '8':
                       components[i] = 8;
                       break;

                     case '9':
                       components[i] = 9;
                       break;

                     case 'a':
                     case 'A':
                       components[i] = 10;
                       break;

                     case 'b':
                     case 'B':
                       components[i] = 11;
                       break;

                     case 'c':
                     case 'C':
                       components[i] = 12;
                       break;

                     case 'd':
                     case 'D':
                       components[i] = 13;
                       break;

                     case 'e':
                     case 'E':
                       components[i] = 14;
                       break;

                     case 'f':
                     case 'F':
                       components[i] = 15;
                       break;

                     default:
                       gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                       FieldSelection->lineNo,
                                       FieldSelection->stringNo,
                                       clvREPORT_ERROR,
                                       "invalid component selection: '%c'",
                                       FieldSelection->u.fieldSelection[selectLen]));
                       return gcvSTATUS_INVALID_ARGUMENT;
                     }
                  }
                  ComponentSelection->components = i;
                  isNumericIndex = 1;
                  break;
              }
          }
          for (i = 0; FieldSelection->u.fieldSelection[i] != '\0'; i++) {
              if(i >= cldMaxComponentCount) {
                    gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                                    FieldSelection->lineNo,
                                                    FieldSelection->stringNo,
                                                    clvREPORT_ERROR,
                                                    "more than %d components are selected : \"%s\"",
                                                    cldMaxComponentCount,
                                                    FieldSelection->u.fieldSelection));
                    return gcvSTATUS_INVALID_ARGUMENT;
              }
              switch (FieldSelection->u.fieldSelection[i]) {
              case 'x': nameSets[i] = 0; components[i] = clvCOMPONENT_X; break;
              case 'y': nameSets[i] = 0; components[i] = clvCOMPONENT_Y; break;
              case 'z': nameSets[i] = 0; components[i] = clvCOMPONENT_Z; break;
              case 'w': nameSets[i] = 0; components[i] = clvCOMPONENT_W; break;

              case 'r': nameSets[i] = 1; components[i] = clvCOMPONENT_X; break;
              case 'g': nameSets[i] = 1; components[i] = clvCOMPONENT_Y; break;
              case 'b': nameSets[i] = 1; components[i] = clvCOMPONENT_Z; break;
              case 'a': nameSets[i] = 1; components[i] = clvCOMPONENT_W; break;

              case 's': nameSets[i] = 2; components[i] = clvCOMPONENT_X; break;
              case 't': nameSets[i] = 2; components[i] = clvCOMPONENT_Y; break;
              case 'p': nameSets[i] = 2; components[i] = clvCOMPONENT_Z; break;
              case 'q': nameSets[i] = 2; components[i] = clvCOMPONENT_W; break;

              default:
                  gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                  FieldSelection->lineNo,
                                  FieldSelection->stringNo,
                                  clvREPORT_ERROR,
                                  "invalid component name: '%c'",
                                  FieldSelection->u.fieldSelection[i]));
                  return gcvSTATUS_INVALID_ARGUMENT;
              }
         }

         ComponentSelection->components = i;

         for (i = 1; i < ComponentSelection->components; i++) {
             if (nameSets[i] != nameSets[0]) {
                 gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                 FieldSelection->lineNo,
                                 FieldSelection->stringNo,
                                 clvREPORT_ERROR,
                                 "the component name: '%c'"
                                 " do not come from the same set",
                                 FieldSelection->u.fieldSelection[i]));
                 return gcvSTATUS_INVALID_ARGUMENT;
             }
         }
       } while (gcvFALSE);

       if(_clmIsValidVectorType(ComponentSelection->components)) {
           for (i = 0; i < ComponentSelection->components; i++) {
               if ((gctUINT8)components[i] >= VectorSize) {
                   gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                   FieldSelection->lineNo,
                                   FieldSelection->stringNo,
                                   clvREPORT_ERROR,
                                   "the component: '%c' beyond the specified vector type",
                                   FieldSelection->u.fieldSelection[i + isNumericIndex]));
                   return gcvSTATUS_INVALID_ARGUMENT;
               }

               ComponentSelection->selection[i] = components[i];
           }
       }
       else {
           gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                           FieldSelection->lineNo,
                           FieldSelection->stringNo,
                           clvREPORT_ERROR,
                           "vector type of component selection \"%s\" is invalid",
                           FieldSelection->u.fieldSelection));
           return gcvSTATUS_INVALID_ARGUMENT;
       }
   }
   return gcvSTATUS_OK;
}

static gceSTATUS
_ParseFlattenType(
IN cloCOMPILER Compiler,
IN clsDECL *TypeDecl,
OUT clsDECL *Decl
);

cloIR_POLYNARY_EXPR
clParseFuncCallHeaderExpr(
IN cloCOMPILER Compiler,
IN clsLexToken * FuncIdentifier,
IN clsARRAY *Array
)
{
  gceSTATUS    status;
  clePOLYNARY_EXPR_TYPE    exprType;
  clsDECL decl;
  cltPOOL_STRING    funcSymbol = gcvNULL;
  cloIR_POLYNARY_EXPR    polynaryExpr;
  clsBUILTIN_DATATYPE_INFO *typeInfo;

  gcmASSERT(FuncIdentifier);

  clmDECL_Initialize(&decl, gcvNULL, (clsARRAY *)0, gcvNULL, gcvFALSE, clvSTORAGE_QUALIFIER_NONE);

  switch (FuncIdentifier->type) {
  case T_STRUCT:
  case T_UNION:
    status = cloCOMPILER_CloneDecl(Compiler,
                       clvQUALIFIER_CONST,
                       FuncIdentifier->u.typeName->decl.dataType->addrSpaceQualifier,
                       &FuncIdentifier->u.typeName->decl,
                       &decl);
    if (gcmIS_ERROR(status)) return gcvNULL;
    if(Array) {
       decl.array = *Array;
       exprType = clvPOLYNARY_CONSTRUCT_ARRAY;
    }
    else {
       exprType = clvPOLYNARY_CONSTRUCT_STRUCT;
    }
    break;

  case T_IDENTIFIER:
    exprType   = clvPOLYNARY_FUNC_CALL;
    funcSymbol = FuncIdentifier->u.identifier.name;

    if(cloCOMPILER_ExtensionEnabled(Compiler, clvEXTENSION_VASM) &&
        gcmIS_SUCCESS(gcoOS_StrCmp(funcSymbol, "_viv_asm")))
    {
        exprType = clvPOLYNARY_BUILT_IN_ASM_CALL;
    }

    break;

  case T_TYPE_NAME:
    gcmASSERT(0);
    return gcvNULL;

  default:
    exprType = clvPOLYNARY_CONSTRUCT_NONE;

    typeInfo = clGetBuiltinDataTypeInfo(FuncIdentifier->type);
    if(typeInfo != gcvNULL) {
        exprType = typeInfo->constructorType;
    }
    if(exprType == clvPOLYNARY_CONSTRUCT_NONE) {
      gcmASSERT(0);
      return gcvNULL;
    }

    status = cloCOMPILER_CreateDecl(Compiler,
                                    FuncIdentifier->type,
                                    gcvNULL,
                                    clvQUALIFIER_CONST,
                                    clvQUALIFIER_NONE,
                                    &decl);
    if (gcmIS_ERROR(status)) return gcvNULL;
    if(Array) {
       decl.array = *Array;
       exprType = clvPOLYNARY_CONSTRUCT_ARRAY;
    }
    break;
  }

/* Create polynary expression */
  status = cloIR_POLYNARY_EXPR_Construct(Compiler,
                         FuncIdentifier->lineNo,
                         FuncIdentifier->stringNo,
                         exprType,
                         &decl,
                         funcSymbol,
                         &polynaryExpr);
  if (gcmIS_ERROR(status)) {
    return gcvNULL;
  }

  gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                    clvDUMP_PARSER,
                    "<FUNC_CALL_HEADER type=\"%s\" line=\"%d\" string=\"%d\" />",
                    clGetIRPolynaryExprTypeName(exprType),
                    FuncIdentifier->lineNo,
                    FuncIdentifier->stringNo));
  return polynaryExpr;
}

cloIR_EXPR
clParseTypeCastArgument(
IN cloCOMPILER Compiler,
IN cloIR_EXPR leftOperand,
IN cloIR_EXPR rightOperand
)
{
    gceSTATUS  status;
    cloIR_BASE base;
    cloIR_TYPECAST_ARGS typeCastArgs;

    if (leftOperand == gcvNULL) return gcvNULL;
        base = &leftOperand->base;
        if(cloIR_OBJECT_GetType(base) != clvIR_TYPECAST_ARGS) {
      /* Create type cast arguments */
      status = cloIR_TYPECAST_ARGS_Construct(Compiler,
                                             base->lineNo,
                                             base->stringNo,
                                             &typeCastArgs);
      if (gcmIS_ERROR(status)) return gcvNULL;
      status = cloIR_SET_Construct(Compiler,
                       base->lineNo,
                       base->stringNo,
                       clvEXPR_SET,
                       &typeCastArgs->operands);
      if (gcmIS_ERROR(status)) return gcvNULL;
      gcmVERIFY_OK(cloIR_SET_AddMember(Compiler,
                       typeCastArgs->operands,
                       &leftOperand->base));
      gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                        clvDUMP_PARSER,
                        "<TYPECAST__ARGUMENT />"));
    }
    else typeCastArgs = (cloIR_TYPECAST_ARGS) base;
    gcmASSERT(typeCastArgs->operands);
    if(rightOperand) {
       gcmVERIFY_OK(cloIR_SET_AddMember(Compiler,
                        typeCastArgs->operands,
                        &rightOperand->base));
    }

    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                      clvDUMP_PARSER,
                      "<TYPECAST__ARGUMENT />"));
    return &typeCastArgs->exprBase;
}

/*****************************************************************************************
 Find the pointer variable of an expr
******************************************************************************************/
clsNAME *
clParseFindPointerVariable(
IN cloCOMPILER Compiler,
IN cloIR_EXPR Expr
)
{
   clsNAME *variableName = gcvNULL;
   cloIR_UNARY_EXPR unaryExpr;
   cloIR_BINARY_EXPR binaryExpr;

   switch(cloIR_OBJECT_GetType(&Expr->base)) {
   case clvIR_UNARY_EXPR:
      unaryExpr = (cloIR_UNARY_EXPR) &Expr->base;

      if(clmDECL_IsPointerType(&unaryExpr->exprBase.decl)) {
         if (unaryExpr->type == clvUNARY_ADDR) {
            return clParseFindLeafName(Compiler,
                                       unaryExpr->operand);
         }
         return clParseFindPointerVariable(Compiler,
                                           unaryExpr->operand);
      }
      break;

   case clvIR_BINARY_EXPR:
      binaryExpr = (cloIR_BINARY_EXPR) &Expr->base;
      if(clmDECL_IsPointerType(&binaryExpr->leftOperand->decl)) {
         variableName =  clParseFindPointerVariable(Compiler,
                                                    binaryExpr->leftOperand);
      }
      if(!variableName && clmDECL_IsPointerType(&binaryExpr->rightOperand->decl)) {
         return clParseFindPointerVariable(Compiler,
                                           binaryExpr->rightOperand);
      }
      break;

   case clvIR_VARIABLE:
      variableName = ((cloIR_VARIABLE) &Expr->base)->name;
      break;

   default:
      break;
   }

   return variableName;
}

/*****************************************************************************************
 Set the associated variables of the left operand and right operand in an assignment flag
 to indicate it has been modified
******************************************************************************************/
gceSTATUS
clParseSetOperandDirty(
IN cloCOMPILER Compiler,
IN cloIR_EXPR LeftOperand,
IN cloIR_EXPR RightOperand
)
{
   clsNAME * variableName;
   cloIR_UNARY_EXPR unaryExpr;
   cloIR_BINARY_EXPR binaryExpr;

   if(LeftOperand) {
      variableName = clParseFindLeafName(Compiler,
                                         LeftOperand);

      if(variableName) {
          variableName->u.variableInfo.isDirty = gcvTRUE;
      }
      else {
         switch(cloIR_OBJECT_GetType(&LeftOperand->base)) {
         case clvIR_UNARY_EXPR:
            unaryExpr = (cloIR_UNARY_EXPR) &LeftOperand->base;
            if (unaryExpr->type == clvUNARY_INDIRECTION) {
               variableName =  clParseFindPointerVariable(Compiler,
                                                          unaryExpr->operand);
            }
            break;

         case clvIR_BINARY_EXPR:
            binaryExpr = (cloIR_BINARY_EXPR) &LeftOperand->base;
            if (binaryExpr->type == clvBINARY_SUBSCRIPT) {
               variableName =  clParseFindPointerVariable(Compiler,
                                                          binaryExpr->leftOperand);
            }
            break;

         default:
            variableName = gcvNULL;
            break;
         }

         if(variableName) {
             variableName->u.variableInfo.isDirty = gcvTRUE;
         }
      }
   }

   if(RightOperand) {
      if(clmDECL_IsPointerType(&RightOperand->decl)) {
         variableName = clParseFindLeafName(Compiler,
                                            RightOperand);

         if(variableName) {
             variableName->u.variableInfo.isDirty = gcvTRUE;
         }
         else {
             variableName =  clParseFindPointerVariable(Compiler,
                                                        RightOperand);
             if(variableName) {
                variableName->u.variableInfo.isDirty = gcvTRUE;
             }
         }
      }
   }

   return gcvSTATUS_OK;
}

cloIR_POLYNARY_EXPR
clParseFuncCallArgument(
IN cloCOMPILER Compiler,
IN cloIR_POLYNARY_EXPR FuncCall,
IN cloIR_EXPR Argument
)
{
    gceSTATUS  status;
    cloIR_EXPR argument;

    if (FuncCall == gcvNULL || Argument == gcvNULL) return gcvNULL;
    if (FuncCall->operands == gcvNULL) {
        status = cloIR_SET_Construct(Compiler,
                                     Argument->base.lineNo,
                                     Argument->base.stringNo,
                                     clvEXPR_SET,
                                     &FuncCall->operands);
        if (gcmIS_ERROR(status)) return gcvNULL;
    }

    gcmASSERT(FuncCall->operands);
    status = _ParseSetAggregateTypedOperandAddressed(Compiler,
                                                     Argument,
                                                     &argument);
    if (gcmIS_ERROR(status)) return gcvNULL;

    gcmVERIFY_OK(cloIR_SET_AddMember(Compiler,
                                     FuncCall->operands,
                                     &argument->base));

    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                                  clvDUMP_PARSER,
                                  "<FUNC_CALL_ARGUMENT />"));
    return FuncCall;
}

cloIR_EXPR
clParseFieldSelectionExpr(
IN cloCOMPILER Compiler,
IN cloIR_EXPR Operand,
IN clsLexToken * FieldSelection
)
{
    gceSTATUS  status;
    cleUNARY_EXPR_TYPE exprType;
    clsNAME *fieldName = gcvNULL;
    clsCOMPONENT_SELECTION    componentSelection;
    cloIR_CONSTANT    resultConstant;
    cloIR_UNARY_EXPR unaryExpr;
    cloIR_EXPR resExpr = gcvNULL;

    gcmASSERT(FieldSelection);

    if (Operand == gcvNULL) return gcvNULL;

    if (clmDATA_TYPE_IsStructOrUnion(Operand->decl.dataType)) {
        clsNAME_SPACE *fieldSpace = Operand->decl.dataType->u.fieldSpace;
        gcmASSERT(fieldSpace && fieldSpace->scopeName);

        exprType = clvUNARY_FIELD_SELECTION;
        if(fieldSpace->scopeName->u.typeInfo.hasUnnamedFields) {
            status = clsNAME_SPACE_SearchFieldSpaceWithUnnamedField(Compiler,
                                                                    fieldSpace,
                                                                    FieldSelection->u.fieldSelection,
                                                                    gcvTRUE,
                                                                    &fieldName);
        }
        else {
            status = clsNAME_SPACE_Search(Compiler,
                              Operand->decl.dataType->u.fieldSpace,
                              FieldSelection->u.fieldSelection,
                              gcvFALSE,
                              &fieldName);
        }
        if (status != gcvSTATUS_OK) {
            gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                            FieldSelection->lineNo,
                            FieldSelection->stringNo,
                            clvREPORT_ERROR,
                            "unknown field: '%s'",
                            FieldSelection->u.fieldSelection));
            return gcvNULL;
        }
        gcmASSERT(fieldName->type == clvFIELD_NAME);
    }
    else if (clmDECL_IsBVecOrIVecOrVec(&Operand->decl)) {
        exprType = clvUNARY_COMPONENT_SELECTION;
        status = _ParseComponentSelection(Compiler,
                              clmDATA_TYPE_vectorSize_NOCHECK_GET(Operand->decl.dataType),
                              FieldSelection,
                              &componentSelection);
        if (gcmIS_ERROR(status)) return gcvNULL;
    }
    else {
        gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                        Operand->base.lineNo,
                        Operand->base.stringNo,
                        clvREPORT_ERROR,
                        "require a struct/union or vector typed expression"));
        return gcvNULL;
    }

    /* Constant calculation */
    if (_clmExprIsConstantForEval(Operand)) {
        status = cloIR_UNARY_EXPR_Evaluate(Compiler,
                           exprType,
                           (cloIR_CONSTANT)Operand,
                           fieldName,
                           &componentSelection,
                           &resultConstant);
        if (gcmIS_ERROR(status)) return gcvNULL;
        return &resultConstant->exprBase;
    }

    /* Create unary expression */
    status = cloIR_UNARY_EXPR_Construct(Compiler,
                        Operand->base.lineNo,
                        Operand->base.stringNo,
                        exprType,
                        Operand,
                        fieldName,
                        &componentSelection,
                        &resExpr);
    if (gcmIS_ERROR(status)) return gcvNULL;

    if(fieldName && clmDECL_IsArray(&fieldName->decl)) {
       if(clScanLookAheadWithSkip(Compiler, '[', ')') == gcvSTATUS_NOT_FOUND) { /* the next token is not '[' */
          cloIR_EXPR expr;

          /* Create the expression &A[0] for A being an array*/
          expr =  _EvaluateIndirectionExpr(Compiler,
                                           resExpr);
          gcmASSERT(expr);
          status = cloIR_UNARY_EXPR_Construct(Compiler,
                                              Operand->base.lineNo,
                                              Operand->base.stringNo,
                                              clvUNARY_ADDR,
                                              expr,
                                              gcvNULL,
                                              gcvNULL,
                                              &resExpr);
          if (gcmIS_ERROR(status)) return gcvNULL;
          unaryExpr = (cloIR_UNARY_EXPR)&resExpr->base;
          unaryExpr->u.generated = fieldName;
          status = clParseSetOperandAddressed(Compiler,
                                              resExpr);
          if (gcmIS_ERROR(status)) return gcvNULL;
       }
    }

    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                      clvDUMP_PARSER,
                      "<UNARY_EXPR type=\"%s\" line=\"%d\" string=\"%d\""
                      " fieldSelection=\"%s\" />",
                      clGetIRUnaryExprTypeName(exprType),
                      Operand->base.lineNo,
                      Operand->base.stringNo,
                      FieldSelection->u.fieldSelection));
    return resExpr;
}

cloIR_EXPR
clParsePtrFieldSelectionExpr(
IN cloCOMPILER Compiler,
IN cloIR_EXPR Operand,
IN clsLexToken * FieldSelection
)
{
   gceSTATUS  status;
   cleUNARY_EXPR_TYPE exprType;
   clsNAME *fieldName = gcvNULL;
   clsCOMPONENT_SELECTION componentSelection;
   cloIR_EXPR resExpr;
   cloIR_EXPR derefExpr;
   clsLexToken token[1];

   gcmASSERT(FieldSelection);

   if (Operand == gcvNULL) return gcvNULL;

   (void) gcoOS_ZeroMemory((gctPOINTER)token, sizeof(clsLexToken));
   token->u.operator = T_STRUCT_UNION_PTR;

   derefExpr = clParseNormalUnaryExpr(Compiler,
                                      token,
                                      Operand);

   if(derefExpr == gcvNULL) return gcvNULL;

   gcmASSERT(Operand->decl.dataType->u.fieldSpace);

   exprType = clvUNARY_FIELD_SELECTION;
   gcmASSERT (clmDATA_TYPE_IsStructOrUnion(Operand->decl.dataType));

   if(Operand->decl.dataType->u.fieldSpace->scopeName->u.typeInfo.hasUnnamedFields) {
       status = clsNAME_SPACE_SearchFieldSpaceWithUnnamedField(Compiler,
                                                               Operand->decl.dataType->u.fieldSpace,
                                                               FieldSelection->u.fieldSelection,
                                                               gcvTRUE,
                                                               &fieldName);
   }
   else {
       status = clsNAME_SPACE_Search(Compiler,
                         Operand->decl.dataType->u.fieldSpace,
                         FieldSelection->u.fieldSelection,
                         gcvFALSE,
                         &fieldName);
   }

   if (status != gcvSTATUS_OK) {
      gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                      FieldSelection->lineNo,
                                      FieldSelection->stringNo,
                                      clvREPORT_ERROR,
                                      "unknown field: '%s'",
                                      FieldSelection->u.fieldSelection));
                                      return gcvNULL;
   }
   gcmASSERT(fieldName->type == clvFIELD_NAME);

   /* Create unary expression */
   status = cloIR_UNARY_EXPR_Construct(Compiler,
                                       Operand->base.lineNo,
                                       Operand->base.stringNo,
                                       exprType,
                                       derefExpr,
                                       fieldName,
                                       &componentSelection,
                                       &resExpr);
   if (gcmIS_ERROR(status)) return gcvNULL;
   gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                                 clvDUMP_PARSER,
                                 "<UNARY_EXPR type=\"%s\" line=\"%d\" string=\"%d\""
                                 " fieldSelection=\"%s\" />",
                                 "->",
                                 Operand->base.lineNo,
                                 Operand->base.stringNo,
                                 FieldSelection->u.fieldSelection));
   return resExpr;
}

gceSTATUS
_CheckIncOrDecExpr(
IN cloCOMPILER Compiler,
IN cloIR_EXPR Operand
)
{
    gceSTATUS status;

    gcmASSERT(Operand);
    gcmASSERT(Operand->decl.dataType);

    /* Check the operand */
    status = _CheckLValueExpr(Compiler, Operand, "inc or dec");
    if (gcmIS_ERROR(status)) return status;

        if(!clmDECL_IsPointerType(&Operand->decl)
       && !clmDECL_IsIntOrIVec(&Operand->decl)) {
        gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                        Operand->base.lineNo,
                        Operand->base.stringNo,
                        clvREPORT_ERROR,
                        "require an integer or pointer typed expression"));
        return gcvSTATUS_INVALID_ARGUMENT;
    }

    return gcvSTATUS_OK;
}

cloIR_EXPR
clParseIncOrDecExpr(
IN cloCOMPILER Compiler,
IN clsLexToken * StartToken,
IN cleUNARY_EXPR_TYPE ExprType,
IN cloIR_EXPR Operand
)
{
    gceSTATUS    status;
    gctUINT      lineNo;
    gctUINT      stringNo;
    cloIR_EXPR   resExpr;

    if (Operand == gcvNULL) return gcvNULL;

    if (StartToken != gcvNULL) {
       lineNo    = StartToken->lineNo;
       stringNo = StartToken->stringNo;
    }
    else {
       lineNo   = Operand->base.lineNo;
       stringNo = Operand->base.stringNo;
    }

    /* Check Error */
    status = _CheckIncOrDecExpr(Compiler    ,
                    Operand);

    if (gcmIS_ERROR(status)) return gcvNULL;

    if(ExprType != clvUNARY_POST_INC &&
       ExprType != clvUNARY_POST_DEC &&
       ExprType != clvUNARY_PRE_INC &&
       ExprType != clvUNARY_PRE_DEC)  return gcvNULL;

    /* Create unary expression */
    status = cloIR_UNARY_EXPR_Construct(Compiler,
                        lineNo,
                        stringNo,
                        ExprType,
                        Operand,
                        gcvNULL,
                        gcvNULL,
                        &resExpr);
    if (gcmIS_ERROR(status)) return gcvNULL;

    status = clParseSetOperandDirty(Compiler,
                                    Operand,
                                    gcvNULL);
    if (gcmIS_ERROR(status)) return gcvNULL;

    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                    clvDUMP_PARSER,
                    "<UNARY_EXPR type=\"%s\" line=\"%d\" string=\"%d\" />",
                    clGetIRUnaryExprTypeName(ExprType),
                    lineNo,
                    stringNo));
    return resExpr;
}

static gceSTATUS
_CheckPosOrNegExpr(
IN cloCOMPILER Compiler,
IN cloIR_EXPR Operand
)
{
    gcmASSERT(Operand);
    gcmASSERT(Operand->decl.dataType);

    /* Check the operand */
    if (!clmDECL_IsIntOrIVec(&Operand->decl)
        && !clmDECL_IsFloatOrVecOrMat(&Operand->decl)) {
        gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                        Operand->base.lineNo,
                        Operand->base.stringNo,
                        clvREPORT_ERROR,
                        "require an integer or floating-point typed expression"));
        return gcvSTATUS_INVALID_ARGUMENT;
    }
    return gcvSTATUS_OK;
}

static gceSTATUS
_CheckNotExpr(
IN cloCOMPILER Compiler,
IN cloIR_EXPR Operand
)
{
   gcmASSERT(Operand);
   gcmASSERT(Operand->decl.dataType);

   /* Check the operand */
   if (!clmDECL_IsIntOrIVec(&Operand->decl) &&
       !clmHasRightLanguageVersion(Compiler, _cldCL1Dot2)) {
       gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                       Operand->base.lineNo,
                                       Operand->base.stringNo,
                                       clvREPORT_ERROR,
                                       "require an integer expression"));
       return gcvSTATUS_INVALID_ARGUMENT;
   }

   return gcvSTATUS_OK;
}

static gceSTATUS
_CheckBitwiseNotExpr(
IN cloCOMPILER Compiler,
IN cloIR_EXPR Operand
)
{
    gcmASSERT(Operand);
    gcmASSERT(Operand->decl.dataType);

    /* Check the operand */
    if (!clmDECL_IsIntOrIVec(&Operand->decl) &&
            !clmDECL_IsSampler(&Operand->decl)) {
        gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                        Operand->base.lineNo,
                        Operand->base.stringNo,
                        clvREPORT_ERROR,
                        "require a scalar or vector int expression"));
        return gcvSTATUS_INVALID_ARGUMENT;
    }
    return gcvSTATUS_OK;
}

static gceSTATUS
_CheckIndirectionExpr(
IN cloCOMPILER Compiler,
IN cloIR_EXPR Operand
)
{
    gcmASSERT(Operand);

/* Check the operand */
    if(!(clmDECL_IsArray(&Operand->decl) || clmDECL_IsPointerType(&Operand->decl))) {
        gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                        Operand->base.lineNo,
                        Operand->base.stringNo,
                        clvREPORT_ERROR,
                        "indirection operator '*' requires a pointer expression"));
        return gcvSTATUS_INVALID_ARGUMENT;
    }
    return gcvSTATUS_OK;
}

static gceSTATUS
_CheckPtrStructUnionExpr(
IN cloCOMPILER Compiler,
IN cloIR_EXPR Operand
)
{
    gcmASSERT(Operand);

/* Check the operand */
    if (clmDATA_TYPE_IsStructOrUnion(Operand->decl.dataType)) {
       gctINT level;

       level = clParseCountIndirectionLevel(Operand->decl.ptrDscr);
       if(level == 1 ||
          (level == 0 && clmDECL_IsArray(&Operand->decl))) {
          return gcvSTATUS_OK;
       }
    }
    gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                    Operand->base.lineNo,
                    Operand->base.stringNo,
                    clvREPORT_ERROR,
                    "operator '->' requires a pointer expression to struct or union"));
    return gcvSTATUS_INVALID_ARGUMENT;
}

static gceSTATUS
_CheckAddrExpr(
IN cloCOMPILER Compiler,
IN cloIR_EXPR Operand
)
{
    cloIR_UNARY_EXPR unaryExpr;
    cloIR_BINARY_EXPR binaryExpr;
    cloIR_CONSTANT  constantExpr;

    gcmASSERT(Operand);

    switch(cloIR_OBJECT_GetType(&Operand->base)) {
    case clvIR_VARIABLE:
         return gcvSTATUS_OK;

    case clvIR_BINARY_EXPR:
         binaryExpr = (cloIR_BINARY_EXPR) &Operand->base;
         if(binaryExpr->type == clvBINARY_SUBSCRIPT) return gcvSTATUS_OK;
         break;

    case clvIR_UNARY_EXPR:
         unaryExpr = (cloIR_UNARY_EXPR) &Operand->base;
         switch(unaryExpr->type) { /* check the unary operator type */
         case clvUNARY_INDIRECTION:
         case clvUNARY_FIELD_SELECTION:
            return gcvSTATUS_OK;

         case clvUNARY_POST_INC:
         case clvUNARY_POST_DEC:
         case clvUNARY_PRE_INC:
         case clvUNARY_PRE_DEC:
            gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                            Operand->base.lineNo,
                                            Operand->base.stringNo,
                                            clvREPORT_ERROR,
                                            "address operator '&' requires an l-value"));
            return gcvSTATUS_INVALID_ARGUMENT;

         case clvUNARY_COMPONENT_SELECTION:
            gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                            Operand->base.lineNo,
                                            Operand->base.stringNo,
                                            clvREPORT_ERROR,
                                            "address operator '&' on component selection not allowed"));
            return gcvSTATUS_INVALID_ARGUMENT;

         case clvUNARY_ADDR:
            if(!unaryExpr->u.generated) {
               gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                               Operand->base.lineNo,
                                               Operand->base.stringNo,
                                               clvREPORT_ERROR,
                                               "address operation on another address expression not allowed"));
               return gcvSTATUS_INVALID_ARGUMENT;
            }
            else return gcvSTATUS_OK;

         default:
            break;
         }
         break;

    case clvIR_CONSTANT:
         constantExpr = (cloIR_CONSTANT) &Operand->base;
         if(constantExpr->variable) return gcvSTATUS_OK;
         break;

    default:
         break;
    }
    gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                    Operand->base.lineNo,
                                    Operand->base.stringNo,
                                    clvREPORT_ERROR,
                                    "address operator '&' requires an l-value"));
    return gcvSTATUS_INVALID_ARGUMENT;
}

gctINT
clParseCountIndirectionLevel(
IN slsSLINK_LIST *PtrDscr
)
{
    clsTYPE_QUALIFIER *prevDscr;
    clsTYPE_QUALIFIER *nextDscr;
    gctINT count = 0;

    FOR_EACH_SLINK_NODE(PtrDscr, clsTYPE_QUALIFIER, prevDscr, nextDscr) {
       if(nextDscr->type == T_EOF) count++;
    }
    return count;
}

void
clParseRemoveIndirectionOneLevel(
IN cloCOMPILER Compiler,
IN slsSLINK_LIST **PtrDscr
)
{
    clsTYPE_QUALIFIER *typeQualifier;
    gctINT qualType;
    slsSLINK_LIST *ptrDscr;

    ptrDscr = *PtrDscr;
    gcmASSERT(!slmSLINK_LIST_IsEmpty(ptrDscr));
    do {
        slmSLINK_LIST_DetachFirst(ptrDscr, clsTYPE_QUALIFIER, &typeQualifier);
        qualType = typeQualifier->type;
        gcmVERIFY_OK(cloCOMPILER_Free(Compiler, typeQualifier));
    } while (qualType != T_EOF);
    *PtrDscr = ptrDscr;
}

/*****************************************************************************************
 Find the leaf name of an expr
******************************************************************************************/
clsNAME *
clParseFindLeafName(
IN cloCOMPILER Compiler,
IN cloIR_EXPR Expr
)
{
   clsNAME *leafName = gcvNULL;
   cloIR_UNARY_EXPR unaryExpr;
   cloIR_BINARY_EXPR binaryExpr;

   switch(cloIR_OBJECT_GetType(&Expr->base)) {
   case clvIR_UNARY_EXPR:
      unaryExpr = (cloIR_UNARY_EXPR) &Expr->base;
      if (unaryExpr->type == clvUNARY_FIELD_SELECTION) {
         leafName = clParseFindLeafName(Compiler,
                                        unaryExpr->operand);
      }
      break;

   case clvIR_BINARY_EXPR:
      binaryExpr = (cloIR_BINARY_EXPR) &Expr->base;
      if(binaryExpr->type == clvBINARY_SUBSCRIPT) {
         leafName = clParseFindLeafName(Compiler,
                                        binaryExpr->leftOperand);
      }
      break;

   case clvIR_VARIABLE:
      leafName = ((cloIR_VARIABLE) &Expr->base)->name;
      break;

   case clvIR_CONSTANT:
      leafName = ((cloIR_CONSTANT) &Expr->base)->variable;
      break;

   default:
      break;
   }

   return leafName;
}

/*****************************************************************************************
 Set an operand variable to indicate it has been reference through the '&' operator
******************************************************************************************/
gceSTATUS
clParseSetOperandAddressed(
IN cloCOMPILER Compiler,
IN cloIR_EXPR Operand
)
{
   clsNAME * variableName;

   variableName = clParseFindLeafName(Compiler,
                                      Operand);

   if(variableName) {
       return clsNAME_SetVariableAddressed(Compiler,
                                           variableName);
   }
   return gcvSTATUS_OK;
}

/*****************************************************************************************
 Force an operand variable with aggregate type to be allocated from memory
******************************************************************************************/
static gceSTATUS
_ParseSetAggregateTypedOperandAddressed(
IN cloCOMPILER Compiler,
IN cloIR_EXPR Operand,
IN OUT cloIR_EXPR *ResOperand
)
{
   clsNAME * variableName;
   cloIR_EXPR resOperand;

   resOperand = Operand;
   if(Operand) {
       gceSTATUS status;

       if(cloIR_OBJECT_GetType(&Operand->base) == clvIR_CONSTANT &&
          clmDECL_IsArray(&Operand->decl) &&
          ((cloIR_CONSTANT)Operand)->valueCount > 1) {
          cloIR_EXPR constVariableExpr;

          status = _MakeConstantVariableExpr(Compiler,
                                             (cloIR_CONSTANT)Operand,
                                             &constVariableExpr);
          if (gcmIS_ERROR(status)) return status;

          resOperand = constVariableExpr;
       }
       else if(clmDECL_IsAggregateTypeOverRegLimit(&Operand->decl)) {
          variableName = clParseFindLeafName(Compiler,
                                             Operand);
          if(variableName && variableName->type != clvPARAMETER_NAME) {
             gcmASSERT(clmDECL_IsAggregateType(&variableName->decl));
             status = clsNAME_SetVariableAddressed(Compiler,
                                                   variableName);
             if (gcmIS_ERROR(status)) return status;
          }
       }
   }

   if(ResOperand) {
       *ResOperand = resOperand;
   }
   return gcvSTATUS_OK;
}

gceSTATUS
clParseAddIndirectionOneLevel(
IN cloCOMPILER Compiler,
IN slsSLINK_LIST **PtrDscr
)
{
    gceSTATUS status;
    clsTYPE_QUALIFIER *typeQualifier;
    slsSLINK_LIST *ptrDscr;
    gctPOINTER pointer;

    status = cloCOMPILER_Allocate(Compiler,
                                  (gctSIZE_T)sizeof(clsTYPE_QUALIFIER),
                                  (gctPOINTER *) &pointer);
    if (gcmIS_ERROR(status)) return status;
    typeQualifier = pointer;

    ptrDscr = *PtrDscr;
    typeQualifier->type = T_EOF;
    typeQualifier->qualifier = clvQUALIFIER_NONE;
    slmSLINK_LIST_InsertFirst(ptrDscr, &typeQualifier->node);
    *PtrDscr = ptrDscr;
    return gcvSTATUS_OK;
}

cloIR_EXPR
clParseNormalUnaryExpr(
IN cloCOMPILER Compiler,
IN clsLexToken * Operator,
IN cloIR_EXPR Operand
)
{
    gceSTATUS  status;
    cleUNARY_EXPR_TYPE exprType = clvUNARY_NEG;
    cloIR_CONSTANT  resultConstant;
    cloIR_UNARY_EXPR  unaryExpr;
    cloIR_EXPR  resExpr;

    gcmASSERT(Operator);

    if (Operand == gcvNULL) return gcvNULL;

    if (clmDECL_IsPackedGenType(&Operand->decl)) {
        gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                        Operator->lineNo,
                                        Operator->stringNo,
                                        clvREPORT_ERROR,
                                        "_viv_gentype_packed operands not allowed in unary operator '%s'",
                                        _GetBinaryOperatorName(Operator->u.operator)));
        return gcvNULL;
    }

    if (clmDECL_IsHalfType(&Operand->decl) && Operator->u.operator != '&') {
        if(!cloCOMPILER_ExtensionEnabled(Compiler, clvEXTENSION_CL_KHR_FP16)) {
            gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                            Operator->lineNo,
                                            Operator->stringNo,
                                            clvREPORT_ERROR,
                                            "operand of type half not supported in unary operator '%s'",
                                            _GetBinaryOperatorName(Operator->u.operator)));
            return gcvNULL;
        }
    }

    switch (Operator->u.operator) {
    case '-':
        exprType = clvUNARY_NEG; /* fall through */

    case '+':
        status = _CheckPosOrNegExpr(Compiler, Operand);
        if (gcmIS_ERROR(status)) return gcvNULL;
        if (Operator->u.operator == '+') {
            return Operand;
        }
        break;

    case '!':
        exprType = clvUNARY_NOT;
        status = _CheckNotExpr(Compiler, Operand);
        if (gcmIS_ERROR(status)) return gcvNULL;
        break;

    case T_STRUCT_UNION_PTR:
        status = _CheckPtrStructUnionExpr(Compiler, Operand);
        if (gcmIS_ERROR(status)) return gcvNULL;
        if(cloIR_OBJECT_GetType(&Operand->base) == clvIR_UNARY_EXPR) {
            unaryExpr = (cloIR_UNARY_EXPR) &Operand->base;
            if (unaryExpr->type == clvUNARY_ADDR ||
                unaryExpr->type == clvUNARY_NON_LVAL) {
                return unaryExpr->operand;
            }
        }
        return _EvaluateIndirectionExpr(Compiler,
                                        Operand);

    case '*':
        status = _CheckIndirectionExpr(Compiler, Operand);
        if (gcmIS_ERROR(status)) return gcvNULL;

        if(cloIR_OBJECT_GetType(&Operand->base) == clvIR_UNARY_EXPR) {
            unaryExpr = (cloIR_UNARY_EXPR) &Operand->base;
            if (unaryExpr->type == clvUNARY_ADDR ||
                unaryExpr->type == clvUNARY_NON_LVAL) {
                return unaryExpr->operand;
            }
        }
        exprType = clvUNARY_INDIRECTION;
        break;

    case '&':
        if(cloIR_OBJECT_GetType(&Operand->base) == clvIR_UNARY_EXPR)
        {
            cloIR_UNARY_EXPR unaryExpr = (cloIR_UNARY_EXPR) &Operand->base;
            if(unaryExpr->type == clvUNARY_NULL) {
                Operand = unaryExpr->operand;
                gcmASSERT(Operand);
            }
        }
        status = _CheckAddrExpr(Compiler, Operand);
        if (gcmIS_ERROR(status)) return gcvNULL;
        if(cloIR_OBJECT_GetType(&Operand->base) == clvIR_UNARY_EXPR) {
            unaryExpr = (cloIR_UNARY_EXPR)&Operand->base;
            if(unaryExpr->type == clvUNARY_INDIRECTION) {
                cloIR_EXPR operand = unaryExpr->operand;

                status = cloIR_UNARY_EXPR_Construct(Compiler,
                                                    Operator->lineNo,
                                                    Operator->stringNo,
                                                    clvUNARY_NON_LVAL,
                                                    operand,
                                                    gcvNULL,
                                                    gcvNULL,
                                                    &resExpr);
                if (gcmIS_ERROR(status)) return gcvNULL;
                return resExpr;
            }
            else if(unaryExpr->type == clvUNARY_ADDR &&
                    unaryExpr->u.generated) {
                unaryExpr->u.generated = gcvNULL;
                return &unaryExpr->exprBase;
            }
        }
        else if(cloIR_OBJECT_GetType(&Operand->base) == clvIR_CONSTANT) {
            cloIR_CONSTANT constantExpr = (cloIR_CONSTANT) &Operand->base;

            if(constantExpr->variable) {
                cloIR_VARIABLE variable;
                cloIR_EXPR expr;

                status = cloIR_VARIABLE_Construct(Compiler,
                                                  Operator->lineNo,
                                                  Operator->stringNo,
                                                  constantExpr->variable,
                                                  &variable);
                if (gcmIS_ERROR(status)) return gcvNULL;

                expr = &variable->exprBase;
                if(clmDECL_IsArray(&constantExpr->variable->decl)) {
                   /* Create the expression &A[0] for A being an array*/
                   expr =  _EvaluateIndirectionExpr(Compiler,
                                                    expr);
                   gcmASSERT(expr);
                }
                status = cloIR_UNARY_EXPR_Construct(Compiler,
                                                    Operator->lineNo,
                                                    Operator->stringNo,
                                                    clvUNARY_ADDR,
                                                    expr,
                                                    gcvNULL,
                                                    gcvNULL,
                                                    &resExpr);
                if (gcmIS_ERROR(status)) return gcvNULL;
                unaryExpr = (cloIR_UNARY_EXPR) &resExpr->base;
                unaryExpr->u.generated = constantExpr->variable;
                status = clsNAME_SetVariableAddressed(Compiler,
                                                      constantExpr->variable);
                if (gcmIS_ERROR(status)) return gcvNULL;
                return resExpr;
            }
        }

        exprType = clvUNARY_ADDR;
        break;

    case '~':
        exprType = clvUNARY_NOT_BITWISE;
        status = _CheckBitwiseNotExpr(Compiler, Operand);
        if (gcmIS_ERROR(status)) return gcvNULL;
        break;

    default:
        gcmASSERT(0);
        return gcvNULL;
    }

    /* Constant calculation */
    if (_clmExprIsConstantForEval(Operand)) {
        status = cloIR_UNARY_EXPR_Evaluate(Compiler,
                                           exprType,
                                           (cloIR_CONSTANT)Operand,
                                           gcvNULL,
                                           gcvNULL,
                                           &resultConstant);
        if (gcmIS_ERROR(status)) return gcvNULL;
        return &resultConstant->exprBase;
    }

    /* Create unary expression */
    status = cloIR_UNARY_EXPR_Construct(Compiler,
                                        Operator->lineNo,
                                        Operator->stringNo,
                                        exprType,
                                        Operand,
                                        gcvNULL,
                                        gcvNULL,
                                        &resExpr);
    if (gcmIS_ERROR(status)) return gcvNULL;
    unaryExpr = (cloIR_UNARY_EXPR) &resExpr->base;

/* Reset resulting expression's access qualifier to none according to following criterion
   if operator is an indirection
*/
    if(unaryExpr->exprBase.decl.dataType->addrSpaceQualifier != clvQUALIFIER_CONSTANT &&
        exprType == clvUNARY_INDIRECTION &&
        clmDECL_IsPointerType(&Operand->decl) &&
        unaryExpr->exprBase.decl.dataType->accessQualifier == clvQUALIFIER_CONST) {

        status = cloCOMPILER_CloneDataType(Compiler,
                                           clvQUALIFIER_NONE,
                                           unaryExpr->exprBase.decl.dataType->addrSpaceQualifier,
                                           unaryExpr->exprBase.decl.dataType,
                                           &unaryExpr->exprBase.decl.dataType);
        if(gcmIS_ERROR(status)) return gcvNULL;
    }
    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                                  clvDUMP_PARSER,
                                  "<UNARY_EXPR type=\"%c\" line=\"%d\" string=\"%d\" />",
                                  Operator->u.operator,
                                  Operator->lineNo,
                                  Operator->stringNo));
    return resExpr;
}

clsDECL
clParseCreateDeclFromDataType(
IN cloCOMPILER Compiler,
IN clsDATA_TYPE *DataType
)
{
   clsDECL decl;

   clmDECL_Initialize(&decl, DataType, (clsARRAY *)0, gcvNULL, gcvFALSE, clvSTORAGE_QUALIFIER_NONE);
   return decl;
}

clsDECL
clParseCreateDeclFromExpression(
IN cloCOMPILER Compiler,
IN cloIR_EXPR Expr
)
{
   return Expr->decl;
}

clsDECL
clParseCreateDecl(
IN cloCOMPILER Compiler,
IN clsDECL *Decl,
IN slsSLINK_LIST *PtrDscr,
IN cloIR_EXPR ArrayLengthExpr
)
{
    clsDECL decl;
    gceSTATUS status;

    decl = *Decl;
    if (Decl->dataType == gcvNULL) return decl;

    if(clmDATA_TYPE_IsImage(decl.dataType)) {
        gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                        cloCOMPILER_GetCurrentLineNo(Compiler),
                                        cloCOMPILER_GetCurrentStringNo(Compiler),
                                        clvREPORT_ERROR,
                                        "image cannot have pointer type"));
    }
    else {
        clsARRAY array[1];

        clMergePtrDscrToDecl(Compiler, PtrDscr, &decl, PtrDscr != gcvNULL);
        if(ArrayLengthExpr) {
            clmEvaluateExprToArrayLength(Compiler,
                                         ArrayLengthExpr,
                                         array,
                                         gcvFALSE,
                                         status);
            if (gcmIS_ERROR(status)) return decl;
            status = cloCOMPILER_CreateArrayDecl(Compiler,
                                                 decl.dataType,
                                                 array,
                                                 decl.ptrDscr,
                                                 &decl);
            if (gcmIS_ERROR(status)) return decl;
            decl.ptrDominant = gcvFALSE;
        }
    }
    return decl;
}

clsDECL
clParseTypeofArguments(
IN cloCOMPILER Compiler,
IN clsDECL *Decl,
IN slsSLINK_LIST *PtrDscr
)
{
   clsDECL decl;

   decl = *Decl;
   if(clmDATA_TYPE_IsImage(decl.dataType)) {
      gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                      cloCOMPILER_GetCurrentLineNo(Compiler),
                                      cloCOMPILER_GetCurrentStringNo(Compiler),
                                      clvREPORT_ERROR,
                                      "image cannot have pointer type"));
   }
   else {
      clMergePtrDscrToDecl(Compiler, PtrDscr, &decl, PtrDscr != gcvNULL);
   }
   return decl;
}

void
clParseCastExprBegin(
IN cloCOMPILER Compiler,
IN clsDECL *CastType
)
{
  if(CastType ||
     clScanLookAhead(Compiler, '(') == gcvSTATUS_OK) { /* Found the next token of '(' */
      cloCOMPILER_PushParserState(Compiler, clvPARSER_IN_TYPE_CAST);
      if(CastType) {
          clsPARSER_STATE *parserState;
          parserState = cloCOMPILER_GetParserStateHandle(Compiler);
          gcmASSERT(parserState);
          parserState->castTypeDecl = *CastType;
      }
  }
  else cloCOMPILER_PushParserState(Compiler, clvPARSER_NORMAL);

  return;
}

static cloIR_EXPR
_CreateCastExpr(
IN cloCOMPILER Compiler,
IN clsDECL *Decl,
IN cloIR_EXPR Operand
)
{
    gceSTATUS status;
    cloIR_EXPR expr = gcvNULL;

    gcmASSERT(Operand);
    gcmASSERT(Operand->decl.dataType);

    /* Check the operand */
    do {
        if(!clmDECL_IsScalar(&Operand->decl)) {
            if(clmDECL_IsArray(&Operand->decl)) {
               if(clmDECL_IsPointerType(Decl) &&
                  (Decl->dataType->addrSpaceQualifier == clvQUALIFIER_NONE ||
                   Decl->dataType->addrSpaceQualifier == Operand->decl.dataType->addrSpaceQualifier)) {

                   status =  clParseMakeArrayPointerExpr(Compiler,
                                                         Operand,
                                                         &Operand);
                   if (gcmIS_ERROR(status)) return gcvNULL;
                   break;
               }
               gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                               Operand->base.lineNo,
                                               Operand->base.stringNo,
                                               clvREPORT_ERROR,
                                               "pointer casting between different address spaces not allowed"));
               return gcvNULL;
            }
            else if(clmIsElementTypeSampler(Operand->decl.dataType->elementType) ||
                    clmIsElementTypeEvent(Operand->decl.dataType->elementType)) {
               break;
            }
            else if(gcmOPT_oclOpenCV()) {
               clsDECL decl[1];
               clePOLYNARY_EXPR_TYPE exprType = clvPOLYNARY_CONSTRUCT_NONE;
               clsBUILTIN_DATATYPE_INFO *typeInfo = clGetBuiltinDataTypeInfo(Operand->decl.dataType->type);
               cloIR_POLYNARY_EXPR polynaryExpr;

               if(typeInfo != gcvNULL) {
                   exprType = typeInfo->constructorType;
               }
               if(exprType == clvPOLYNARY_CONSTRUCT_NONE) {
                   gcmASSERT(0);
                   return gcvNULL;
               }

               status = cloCOMPILER_CreateDecl(Compiler,
                                               Operand->decl.dataType->type,
                                               gcvNULL,
                                               clvQUALIFIER_CONST,
                                               clvQUALIFIER_NONE,
                                               decl);
               if (gcmIS_ERROR(status)) return gcvNULL;

               if (clmDECL_IsArray(&Operand->decl)) {
                  decl->array = Operand->decl.array;
                  exprType = clvPOLYNARY_CONSTRUCT_ARRAY;
               }

               /* Create polynary expression */
               status = cloIR_POLYNARY_EXPR_Construct(Compiler,
                                                      Operand->base.lineNo,
                                                      Operand->base.stringNo,
                                                      exprType,
                                                      decl,
                                                      gcvNULL,
                                                      &polynaryExpr);
                if (gcmIS_ERROR(status)) return gcvNULL;

                status = cloIR_SET_Construct(Compiler,
                                             Operand->base.lineNo,
                                             Operand->base.stringNo,
                                             clvEXPR_SET,
                                             &polynaryExpr->operands);
                if (gcmIS_ERROR(status)) return gcvNULL;
                gcmASSERT(polynaryExpr->operands);

                gcmVERIFY_OK(cloIR_SET_AddMember(Compiler,
                                                 polynaryExpr->operands,
                                                 &Operand->base));
                return &polynaryExpr->exprBase;
            }
            else {
               gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                               Operand->base.lineNo,
                                               Operand->base.stringNo,
                                               clvREPORT_ERROR,
                                               "cast expression must be of scalar type"));
            }
            return gcvNULL;
        }
        else if(clmDECL_IsPointerType(&Operand->decl)) {
            if(clmDECL_IsPointerType(Decl)) {
               if(Decl->dataType->addrSpaceQualifier == clvQUALIFIER_NONE ||
                  Decl->dataType->addrSpaceQualifier == Operand->decl.dataType->addrSpaceQualifier) {
                  break;
               }
               gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                               Operand->base.lineNo,
                                               Operand->base.stringNo,
                                               clvREPORT_ERROR,
                                               "pointer casting between different address spaces not allowed"));
            }
            else if(clmDECL_IsIntegerType(Decl)) {
               break;
            }
            else {
               gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                               Operand->base.lineNo,
                                               Operand->base.stringNo,
                                               clvREPORT_ERROR,
                                               "pointer expression can be cast to either pointer or integer"));
            }
            return gcvNULL;
        }
        else if(clmDECL_IsPointerType(Decl) && !clmDECL_IsIntegerType(&Operand->decl)) {
          gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                          Operand->base.lineNo,
                                          Operand->base.stringNo,
                                          clvREPORT_ERROR,
                                          "pointer casting on a non integer"));
          return gcvNULL;
        }
    } while (gcvFALSE);

  /* Constant calculation */
    if (cloIR_OBJECT_GetType(&Operand->base) == clvIR_CONSTANT) {
       status = cloIR_CAST_EXPR_Evaluate(Compiler,
                                         Decl,
                                         (cloIR_CONSTANT)Operand);
       if (gcmIS_ERROR(status)) return gcvNULL;
       return Operand;
    }

 /* Create cast expression */
    status = cloIR_CAST_EXPR_Construct(Compiler,
                                        Operand->base.lineNo,
                                        Operand->base.stringNo,
                                        Decl,
                                        Operand,
                                        &expr);
    if (gcmIS_ERROR(status)) return gcvNULL;
    return expr;
}

static cloIR_EXPR
_ParseConvTypeCastToPolynaryExpr(
IN cloCOMPILER Compiler,
IN clsDECL *Decl,
IN cloIR_TYPECAST_ARGS TypeCastArgs
)
{
      clsNAME typeName[1];
      clsLexToken type;
      cloIR_POLYNARY_EXPR polynaryExpr;

      (void)gcoOS_ZeroMemory((gctPOINTER)&typeName, sizeof(clsLexToken));
      type.lineNo = TypeCastArgs->exprBase.base.lineNo;
      type.stringNo = TypeCastArgs->exprBase.base.stringNo;
      type.u.typeName = typeName;
      type.type = Decl->dataType->type;
      typeName->decl = *Decl;
      polynaryExpr = clParseFuncCallHeaderExpr(Compiler,
                                               &type,
                                               clmDECL_IsArray(Decl) ? &Decl->array : (clsARRAY *) 0);
      polynaryExpr->exprBase.decl.ptrDscr = Decl->ptrDscr;
      if(Decl->ptrDominant) {
          polynaryExpr->exprBase.decl.ptrDominant = gcvTRUE;
          polynaryExpr->exprBase.decl.array = Decl->array;
      }

      polynaryExpr->operands = TypeCastArgs->operands;
      TypeCastArgs->operands = gcvNULL; /* ensure that the operands not be freed */
      gcmVERIFY_OK(cloIR_OBJECT_Destroy(Compiler, &TypeCastArgs->exprBase.base));
      return clParseFuncCallExprAsExpr(Compiler, polynaryExpr);
}

struct _clsVARIABLE_NESTING;
typedef struct _clsVARIABLE_NESTING
{
    slsSLINK_NODE node;
    gctINT level;
    gctSIZE_T operandCount;
    gctSIZE_T maxOperandCount;
}
clsVARIABLE_NESTING;

struct _clsDATA_LOCATION_MAP;
typedef struct _clsDATA_LOCATION_MAP
{
    slsSLINK_LIST *nesting;
    clsDATA_TYPE *dataType;
    gctSIZE_T byteOffset;
}
clsDATA_LOCATION_MAP;

static clsDATA_LOCATION_MAP *_ParseLocationMap = gcvNULL;
static clsDATA_LOCATION_MAP *_ParseEndLocationMap = gcvNULL;
static clsDATA_LOCATION_MAP *_ParseSavedUnionLocationMap = gcvNULL;
static gctSIZE_T _ParseLocationMapSize = 0;
static gctSTRING _ParseConstantBuffer = gcvNULL;


static gceSTATUS
_ParsePushNestingLevel(
IN cloCOMPILER Compiler,
IN clsDATA_LOCATION_MAP *LocationMap,
IN gctSIZE_T OperandCount,
IN gctSIZE_T MaxOperandCount
)
{
    gceSTATUS status;
    clsVARIABLE_NESTING *nesting, *lastNesting;
    gctINT nestLevel;
    gctPOINTER pointer;

    /* Verify the arguments. */
    clmASSERT_OBJECT(Compiler, clvOBJ_COMPILER);
    status = cloCOMPILER_Allocate(Compiler,
                      (gctSIZE_T)sizeof(clsVARIABLE_NESTING),
                      (gctPOINTER *) &pointer);
    if (gcmIS_ERROR(status)) return gcvSTATUS_OUT_OF_MEMORY;
    nesting = pointer;

    lastNesting = slmSLINK_LIST_First(LocationMap->nesting, clsVARIABLE_NESTING);
    if(lastNesting) {
       nestLevel = lastNesting->level + 1;
    }
    else nestLevel = 1;
    nesting->level = nestLevel;
    nesting->operandCount = OperandCount ? OperandCount : MaxOperandCount;
    nesting->maxOperandCount = MaxOperandCount;
    slmSLINK_LIST_InsertFirst(LocationMap->nesting, &nesting->node);
    return gcvSTATUS_OK;
}

#define _clmParseGetLocationOperandCount(Location, OperandCount)  do { \
    clsVARIABLE_NESTING *variableNesting; \
    variableNesting = slmSLINK_LIST_First((Location)->nesting, clsVARIABLE_NESTING); \
    if(variableNesting) { \
      (OperandCount) = variableNesting->operandCount; \
    } \
    else (OperandCount) = 1; \
  } while(gcvFALSE)

#define _clmParseGetNextNesting(Location, CurrNesting)  do { \
    clsVARIABLE_NESTING *variableNesting; \
    if(CurrNesting) { \
       variableNesting = slmSLINK_NODE_Next(&((CurrNesting)->node), clsVARIABLE_NESTING); \
       if(variableNesting != slmSLINK_LIST_First((Location)->nesting, clsVARIABLE_NESTING)) { \
          (CurrNesting) = variableNesting; \
       } \
    } \
    else { \
       (CurrNesting) = slmSLINK_LIST_First((Location)->nesting, clsVARIABLE_NESTING); \
    } \
  } while(gcvFALSE)

static gctSIZE_T
_ParseFormVectorLocationMap(
cloCOMPILER Compiler,
clsDATA_LOCATION_MAP *Location,
gctUINT VectorSize,
gctSIZE_T *ByteOffset,
gctSIZE_T MaxOperandCount
)
{
   gceSTATUS status;
   gctUINT i;
   gctSIZE_T elementSize;
   gctSIZE_T byteOffset;
   clsDATA_LOCATION_MAP *currLocation;
   gctSIZE_T operandCount = VectorSize - 1;

   if(MaxOperandCount < operandCount) {
      /* issue error */
      return 0;
   }

   byteOffset = Location->byteOffset;
   elementSize = *ByteOffset - byteOffset;
   currLocation = Location + 1;
   for(i = 0; i < operandCount; i++) {
      byteOffset += elementSize;
      currLocation->dataType = Location->dataType;
      currLocation->byteOffset = byteOffset;
      currLocation++;
   }
   status = _ParsePushNestingLevel(Compiler,
                                   Location,
                                   VectorSize,
                                   VectorSize);
   if (gcmIS_ERROR(status)) return 0;

   *ByteOffset = byteOffset + elementSize;
   return operandCount;
}

static slsSLINK_LIST *
_ParseCopyVariableNesting(
cloCOMPILER Compiler,
slsSLINK_LIST *SourceList
)
{
   gceSTATUS status;
   clsVARIABLE_NESTING *prevNesting;
   clsVARIABLE_NESTING *nextNesting;
   clsVARIABLE_NESTING *nesting;
   slsSLINK_LIST *newList;
   gctPOINTER pointer;

   slmSLINK_LIST_Initialize(newList);
   FOR_EACH_SLINK_NODE(SourceList, clsVARIABLE_NESTING, prevNesting, nextNesting) {
      status = cloCOMPILER_Allocate(Compiler,
                                    (gctSIZE_T)sizeof(clsVARIABLE_NESTING),
                                    (gctPOINTER *) &pointer);
      if (gcmIS_ERROR(status)) return gcvNULL;
      nesting = pointer;

      *nesting = *nextNesting;
      slmSLINK_LIST_InsertLast(newList, &nesting->node);
   }
   return newList;
}

static gctINT
_ParseReplicateLocationMap(
cloCOMPILER Compiler,
clsDATA_LOCATION_MAP *Location,
gctINT Times,
gctSIZE_T *ByteOffset,
gctSIZE_T MaxOperandCount
)
{
   gceSTATUS status;
   gctINT i;
   gctSIZE_T j;
   gctSIZE_T elementSize;
   gctSIZE_T byteOffset;
   clsDATA_LOCATION_MAP *currLocation, *orgLocation;
   clsVARIABLE_NESTING *lastNesting;
   gctSIZE_T operandCount = Times - 1;
   gctSIZE_T maxOperandCount;

   gcmASSERT(Location);

   lastNesting = slmSLINK_LIST_First(Location->nesting, clsVARIABLE_NESTING);
   if(lastNesting) {
      operandCount *= lastNesting->maxOperandCount;
      maxOperandCount = lastNesting->maxOperandCount;
   }
   else maxOperandCount = 1;

   if(MaxOperandCount < operandCount) {
      /* issue error */
      return -1;
   }

   currLocation = Location + maxOperandCount;
   elementSize = *ByteOffset - Location->byteOffset;
   byteOffset = 0;
   for(i = 0; i < Times - 1; i++) {
      orgLocation = Location;
      byteOffset += elementSize;
      for(j = 0; j < maxOperandCount; j++) {
         currLocation->nesting = _ParseCopyVariableNesting(Compiler,
                                                           orgLocation->nesting);
         currLocation->dataType = orgLocation->dataType;
         currLocation->byteOffset = orgLocation->byteOffset + byteOffset;
         orgLocation++;
         currLocation++;
      }
   }

   status = _ParsePushNestingLevel(Compiler,
                                   Location,
                                   lastNesting ? ((gctINT)lastNesting->operandCount * Times) : Times,
                                   currLocation - Location);
   if (gcmIS_ERROR(status)) return 0;

   *ByteOffset += byteOffset;
   return (gctINT)operandCount;
}

static gctSIZE_T
_ParseFormMatrixLocationMap(
cloCOMPILER Compiler,
clsDATA_LOCATION_MAP *Location,
gctUINT RowCount,
gctUINT ColumnCount,
gctSIZE_T *ByteOffset,
gctSIZE_T MaxOperandCount
)
{
   gctINT numFilled;
   gctSIZE_T byteOffset;
   gctSIZE_T maxOperandCount;
   gctSIZE_T operandCount = RowCount * ColumnCount - 1;

   if(MaxOperandCount < operandCount) {
      /* issue error */
      return 0;
   }

   maxOperandCount = MaxOperandCount;
   byteOffset = *ByteOffset;
   numFilled = (gctINT)_ParseFormVectorLocationMap(Compiler,
                                                   Location,
                                                   RowCount,
                                                   &byteOffset,
                                                   maxOperandCount);
   if(numFilled == 0) {
      return 0;
   }
   else {
      maxOperandCount -= numFilled;
   }

   numFilled = _ParseReplicateLocationMap(Compiler,
                                          Location,
                                          ColumnCount,
                                          &byteOffset,
                                          maxOperandCount);
   if(numFilled == -1) {
      return 0;
   }
   else {
      maxOperandCount -= numFilled;
   }
   *ByteOffset = byteOffset;
   gcmASSERT(operandCount == (MaxOperandCount - maxOperandCount));
   return MaxOperandCount - maxOperandCount;
}

static gctINT
_ParseFormArrayLocationMap(
cloCOMPILER Compiler,
clsDATA_LOCATION_MAP *Location,
clsARRAY *Array,
gctSIZE_T *ByteOffset,
gctSIZE_T MaxOperandCount
)
{
   gctINT i;
   gctINT numFilled;
   gctSIZE_T byteOffset;
   gctSIZE_T operandCount;
   gctSIZE_T maxOperandCount;
   clsVARIABLE_NESTING *variableNesting;

   gcmASSERT(Array);
   clmGetArrayElementCount(*Array, 0, operandCount);
   operandCount--;
   variableNesting = slmSLINK_LIST_First(Location->nesting, clsVARIABLE_NESTING);
   if(variableNesting) {
      operandCount *= variableNesting->maxOperandCount;
   }

   if(MaxOperandCount < operandCount) {
      /* issue error */
      return -1;
   }

   maxOperandCount = MaxOperandCount;
   byteOffset = *ByteOffset;

   for(i = Array->numDim - 1; i >= 0; i--) {
      numFilled = _ParseReplicateLocationMap(Compiler,
                                             Location,
                                             Array->length[i],
                                             &byteOffset,
                                             maxOperandCount);
      if(numFilled == -1) {
         return -1;
      }
      else {
         maxOperandCount -= numFilled;
      }
   }
   *ByteOffset = byteOffset;
   gcmASSERT(operandCount == (MaxOperandCount - maxOperandCount));
   return MaxOperandCount - maxOperandCount;
}

static gctINT
_ParseFillLocationMapData(
cloCOMPILER Compiler,
clsDECL *Decl,
clsDATA_LOCATION_MAP *CurrLocation,
gctSIZE_T *ByteOffset,
gctSIZE_T MaxOperandCount
)
{
   gctSIZE_T byteOffset;
   gctSIZE_T operandCount = 1;
   gctSIZE_T maxOperandCount;
   gctINT numFilled;

   gcmASSERT(Decl && Decl->dataType);
   gcmASSERT(CurrLocation);
   gcmASSERT(ByteOffset);
   gcmASSERT(MaxOperandCount);

   byteOffset = *ByteOffset;
   maxOperandCount = MaxOperandCount;
   if(clmDECL_IsPointerType(Decl)) {
      CurrLocation->dataType = Decl->dataType;
      CurrLocation->byteOffset = byteOffset;
      byteOffset += 4;
      maxOperandCount--;
   }
   else {
      gceSTATUS status;
      clsNAME *fieldName;
      gctSIZE_T elementSize = 0;

      switch(Decl->dataType->elementType) {
      case clvTYPE_STRUCT:
      case clvTYPE_UNION:
         {
           clsDATA_LOCATION_MAP *currLocation;
           gctUINT localSize = 0;
           gctUINT curSize;
           gctUINT structAlignment = 0;
           gctUINT alignment;
           gctBOOL  packed = gcvFALSE;
           gctSIZE_T firstFieldOperandCount;

           gcmASSERT(Decl->dataType->u.fieldSpace);

           currLocation = CurrLocation;
           firstFieldOperandCount = 0;

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
                    else alignment = clPermissibleAlignment(Compiler, &subField->decl);
                 }
                 else alignment = clPermissibleAlignment(Compiler, &fieldName->decl);
              }
              if(structAlignment == 0) structAlignment = alignment;
              else {
                 structAlignment = clFindLCM(structAlignment, alignment);
              }

              byteOffset = clmALIGN(byteOffset, alignment, packed);

              curSize = clsDECL_GetByteSize(Compiler, &fieldName->decl);
              localSize = clmALIGN(localSize, alignment, packed);

              if(Decl->dataType->elementType == clvTYPE_UNION) {
                 if(!firstFieldOperandCount) {
                    numFilled = _ParseFillLocationMapData(Compiler,
                                                          &fieldName->decl,
                                                          currLocation,
                                                          &byteOffset,
                                                          maxOperandCount);
                    if(numFilled == -1) {
                       return -1;
                    }
                    else maxOperandCount -= numFilled;

                    firstFieldOperandCount = numFilled;
                 }
                 if(curSize > localSize) localSize = curSize;
              }
              else {
                 numFilled = _ParseFillLocationMapData(Compiler,
                                                       &fieldName->decl,
                                                       currLocation,
                                                       &byteOffset,
                                                       maxOperandCount);
                 if(numFilled == -1) {
                    return -1;
                 }
                 else {
                    maxOperandCount -= numFilled;
                    currLocation += numFilled;
                 }

                 localSize += curSize;
              }
           }

           numFilled = MaxOperandCount - maxOperandCount;
           operandCount = clsDECL_GetElementSize(Decl);
           gcmASSERT(numFilled <= (gctINT)operandCount);

           status = _ParsePushNestingLevel(Compiler,
                                           CurrLocation,
                                           firstFieldOperandCount,
                                           operandCount);
           if (gcmIS_ERROR(status)) return 0;

           maxOperandCount = MaxOperandCount - operandCount;
           byteOffset = clmALIGN(*ByteOffset, structAlignment, packed);
           byteOffset += clmALIGN(localSize, structAlignment, packed);
         }
         break;

      case clvTYPE_VOID:
         elementSize = 0;
         break;

      case clvTYPE_FLOAT:
      case clvTYPE_BOOL:
      case clvTYPE_INT:
      case clvTYPE_UINT:
         elementSize = 4;
         break;

      case clvTYPE_LONG:
      case clvTYPE_ULONG:
         elementSize = 8;
         break;

      case clvTYPE_CHAR:
      case clvTYPE_UCHAR:
         elementSize = 1;
         break;

      case clvTYPE_SHORT:
      case clvTYPE_USHORT:
         elementSize = 2;
         break;

      case clvTYPE_SAMPLER_T:
      case clvTYPE_IMAGE2D_T:
      case clvTYPE_IMAGE3D_T:
         elementSize = 4;
         break;

      case clvTYPE_DOUBLE:
         elementSize = 8;
     break;

      case clvTYPE_HALF:
         elementSize = 2;
     break;

      case clvTYPE_EVENT_T:
         elementSize = 4;
     break;

      default:
         gcmASSERT(0);
         return 0;
      }

      if(!clmDATA_TYPE_IsStructOrUnion(Decl->dataType)) {
         gctINT vectorSize;

         CurrLocation->dataType = Decl->dataType;
         CurrLocation->byteOffset = byteOffset;
         byteOffset += elementSize;
         maxOperandCount--;

         vectorSize = clmDATA_TYPE_vectorSize_GET(Decl->dataType);
         if (vectorSize > 0) {
            numFilled = (gctINT)_ParseFormVectorLocationMap(Compiler,
                                                            CurrLocation,
                                                            vectorSize,
                                                            &byteOffset,
                                                            maxOperandCount);
            if(numFilled == 0) {
               return -1;
            }
            else {
               maxOperandCount -= numFilled;
            }
            if(vectorSize == 3) {
                 /* 3-component vector data type must be aligned to a 4*sizeof(component) */
                 byteOffset += elementSize;
            }
         }
         else if (clmDATA_TYPE_matrixColumnCount_GET(Decl->dataType) > 0) {
            numFilled = _ParseFormMatrixLocationMap(Compiler,
                                                    CurrLocation,
                                                    clmDATA_TYPE_matrixRowCount_GET(Decl->dataType),
                                                    clmDATA_TYPE_matrixColumnCount_GET(Decl->dataType),
                                                    &byteOffset,
                                                    maxOperandCount);
            if(numFilled == 0) {
               return 0;
            }
            else {
               maxOperandCount -= numFilled;
            }
         }
      }
   }

   if (clmDECL_IsArray(Decl)) {
      numFilled = _ParseFormArrayLocationMap(Compiler,
                                             CurrLocation,
                                             &Decl->array,
                                             &byteOffset,
                                             maxOperandCount);
      if(numFilled == -1) {
         return -1;
      }
      else {
         maxOperandCount -= numFilled;
      }
   }
   *ByteOffset = byteOffset;
   return MaxOperandCount - maxOperandCount;
}

static void
_ParseFreeNestingLevels(
IN cloCOMPILER Compiler,
IN clsDATA_LOCATION_MAP *LocationMap,
IN gctSIZE_T LocationMapSize
)
{
   clsDATA_LOCATION_MAP *locationMap;
   clsDATA_LOCATION_MAP *locationMapEnd;

   locationMap = LocationMap;
   locationMapEnd = LocationMap + LocationMapSize;
   while(locationMap < locationMapEnd) {
      /* Destroy variable nesting */
      while (!slmSLINK_LIST_IsEmpty(locationMap->nesting)) {
         clsVARIABLE_NESTING *variableNesting;
         slmSLINK_LIST_DetachFirst(locationMap->nesting, clsVARIABLE_NESTING, &variableNesting);
         gcmVERIFY_OK(cloCOMPILER_Free(Compiler, variableNesting));
      }
      locationMap++;
   }
}

#define _clmParseFreeLocationMap(Compiler, LocationMap, LocationMapSize) do { \
     _ParseFreeNestingLevels(Compiler, LocationMap, LocationMapSize); \
     gcmVERIFY_OK(cloCOMPILER_Free(Compiler, LocationMap)); \
   } while(gcvFALSE)

static gctSIZE_T
_ParseGetLocationMap(
IN cloCOMPILER Compiler,
IN clsDECL * Decl,
OUT clsDATA_LOCATION_MAP **LocationMap
)
{
    gceSTATUS status;
    gctSIZE_T size;
    gctINT numFilled;
    gctSIZE_T byteOffset = 0;
    gctPOINTER pointer;
    clsDATA_LOCATION_MAP *locationMap;

    gcmASSERT(Decl->dataType);
    gcmASSERT(LocationMap);

    *LocationMap = gcvNULL;
    size = clsDECL_GetSize(Decl);
    if(size == 0) return 0;

    status = cloCOMPILER_ZeroMemoryAllocate(Compiler,
                        (gctSIZE_T)sizeof(clsDATA_LOCATION_MAP) * size,
                        (gctPOINTER *) &pointer);
    if (gcmIS_ERROR(status)) return 0;
    locationMap = pointer;

    numFilled = _ParseFillLocationMapData(Compiler,
                                          Decl,
                                          locationMap,
                                          &byteOffset,
                                          size);
    if(numFilled == -1) {
       _clmParseFreeLocationMap(Compiler,
                                locationMap,
                                size);
       size = 0;
    }
    else {
       gcmASSERT(numFilled == (gctINT)size);
       *LocationMap = locationMap;
    }

    return size;
}

static gctINT
_GetArraySize(
IN cloIR_TYPECAST_ARGS ArgList,
IN clsDECL *Decl,
IN gctINT Dim,
IN OUT clsARRAY *Array
)
{
    cloIR_BASE member;
    gctINT arraySize = 0;
    cloIR_POLYNARY_EXPR polynaryExpr;
    cloIR_BINARY_EXPR binaryExpr;
    cloIR_EXPR operand;
    gctINT isConstant;

    FOR_EACH_DLINK_NODE(&ArgList->operands->members, struct _cloIR_BASE, member) {
       switch(cloIR_OBJECT_GetType(member)) {
       case clvIR_CONSTANT:
           break;

       case clvIR_TYPECAST_ARGS:
           isConstant = _GetArraySize((cloIR_TYPECAST_ARGS) member,
                                      Decl,
                                      Dim + 1,
                                      Array);
          if(isConstant <= 0) {
              return isConstant;
          }
          break;

       case clvIR_BINARY_EXPR:
          binaryExpr = (cloIR_BINARY_EXPR) member;
          if(binaryExpr->type == clvBINARY_ASSIGN) {
             switch(cloIR_OBJECT_GetType(&binaryExpr->rightOperand->base)) {
             case clvIR_CONSTANT:
                break;

             case clvIR_TYPECAST_ARGS:
                isConstant = _GetArraySize((cloIR_TYPECAST_ARGS) &binaryExpr->rightOperand->base,
                                           Decl,
                                           0,
                                           (clsARRAY *) gcvNULL);
                if(isConstant <= 0) {
                   return isConstant;
                }
                break;

             default:
                return 0;
             }
          }
          else return 0;
          break;

       case clvIR_POLYNARY_EXPR:
          polynaryExpr = (cloIR_POLYNARY_EXPR) member;
          if(polynaryExpr->type == clvPOLYNARY_FUNC_CALL ||
             polynaryExpr->type == clvPOLYNARY_BUILT_IN_ASM_CALL)
          {
              return 0;
          }
          FOR_EACH_DLINK_NODE(&polynaryExpr->operands->members, struct _cloIR_EXPR, operand) {
             if(cloIR_OBJECT_GetType(&operand->base) == clvIR_CONSTANT) continue;
             else return 0;
          }
          break;

       default:
          return 0;
       }
       arraySize++;
    }

    if(Array && (Dim < Array->numDim)) {
       if(Array->length[Dim] < 0) {
          Array->length[Dim] = arraySize;
       }
       else if(Array->length[Dim] < arraySize) {
          gctINT elementCount;
          gctINT valueCount;

          clmGetArrayElementCount(*Array, Dim, elementCount);
          valueCount = clsDECL_GetElementSize(Decl) * elementCount;
          if(valueCount < arraySize) {
             return -1;
          }
       }
    }
    return arraySize;
}

static gceSTATUS
_MakeTypeCastArgsAsConstant(
cloCOMPILER Compiler,
cloIR_TYPECAST_ARGS TypeCast,
gctINT Dim,
clsDECL *ConstantDecl,
cluCONSTANT_VALUE *ValStart,
cluCONSTANT_VALUE *ValEnd
)
{
  gceSTATUS status = gcvSTATUS_OK;
  clsDECL *constantDecl;
  cloIR_CONSTANT constMember;
  gctUINT i;
  cluCONSTANT_VALUE *valStart, *valEnd;
  cloIR_BASE member;
  slsDLINK_LIST *structFields;
  clsNAME *fieldName = gcvNULL;
  clsDECL fieldDecl[1];
  gctINT arrayElementCount = 0;
  gctUINT valueCount;
  gctUINT elementCount = 1;
  gctBOOL braced = gcvFALSE;

  gcmASSERT(ConstantDecl);

  constantDecl = ConstantDecl;
  valStart = ValStart;
  valEnd = ValEnd;

  gcmASSERT(TypeCast->operands);
  if(clmDECL_IsArray(ConstantDecl)) {
    member = slsDLINK_LIST_First(&TypeCast->operands->members, struct _cloIR_BASE);
    if(cloIR_OBJECT_GetType(member) == clvIR_TYPECAST_ARGS) {
       if((Dim + 1) < ConstantDecl->array.numDim) {
          clmGetArrayElementCount(ConstantDecl->array, Dim + 1, elementCount);
          braced = gcvTRUE;
          gcmASSERT(elementCount);
       }
    }
  }
  valueCount = clsDECL_GetElementSize(ConstantDecl) * elementCount;
  structFields = gcvNULL;
  clmDECL_Initialize(fieldDecl, gcvNULL, (clsARRAY *)0, gcvNULL, gcvFALSE, clvSTORAGE_QUALIFIER_NONE);
  FOR_EACH_DLINK_NODE(&TypeCast->operands->members, struct _cloIR_BASE, member) {
     switch(cloIR_OBJECT_GetType(member)) {
     case clvIR_CONSTANT:
        constMember = (cloIR_CONSTANT)member;
        if((valStart + constMember->valueCount) > valEnd) {
           gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                           TypeCast->exprBase.base.lineNo,
                                           TypeCast->exprBase.base.stringNo,
                                           clvREPORT_ERROR,
                                           "number of initializers exceeds type defined"));
           return gcvSTATUS_INVALID_DATA;
        }

        if(clmDECL_IsStructOrUnion(ConstantDecl)) {
            if(!structFields) {
                structFields = &ConstantDecl->dataType->u.fieldSpace->names;
                fieldName = (clsNAME *) structFields;
            }

            fieldName = (clsNAME *) ((slsDLINK_NODE *) fieldName)->next;
            if((slsDLINK_NODE *)fieldName != structFields) {
                constantDecl = &fieldName->decl;
            }
            else {
                gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                                TypeCast->exprBase.base.lineNo,
                                                TypeCast->exprBase.base.stringNo,
                                                clvREPORT_ERROR,
                                                "number of initializers exceeds type defined"));
                return gcvSTATUS_INVALID_DATA;
            }

            if(clmDECL_IsStructOrUnion(constantDecl)) {
                gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                                TypeCast->exprBase.base.lineNo,
                                                TypeCast->exprBase.base.stringNo,
                                                clvREPORT_ERROR,
                                                "incorrect syntax in initializing struct elements"));
                return gcvSTATUS_INVALID_DATA;
            }
        }

        if(constMember->exprBase.decl.dataType->elementType == constantDecl->dataType->elementType) {
           for(i=0; i<constMember->valueCount; i++) {
              if(valStart < valEnd) {
                 *valStart++ = constMember->values[i];
              }
           }
        }
        else {
           status = clParseConstantTypeConvert(constMember, constantDecl->dataType->elementType, valStart);
           if (gcmIS_ERROR(status)) {
               gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                               TypeCast->exprBase.base.lineNo,
                                               TypeCast->exprBase.base.stringNo,
                                               clvREPORT_ERROR,
                                               "type mismatch between initializers and defined type"));
               return gcvSTATUS_INVALID_DATA;
           }
           valStart += constMember->valueCount;
        }
        break;

     case clvIR_TYPECAST_ARGS:
        if(clmDECL_IsUnderlyingStructOrUnion(constantDecl)) {
            if(!structFields) {
                structFields = &constantDecl->dataType->u.fieldSpace->names;
                fieldName = (clsNAME *) structFields;
            }

            gcmASSERT(fieldName);
            if(arrayElementCount == 0) {
                if(fieldDecl->dataType == gcvNULL) {
                    fieldName = (clsNAME *) ((slsDLINK_NODE *) fieldName)->next;
                    if((slsDLINK_NODE *)fieldName != structFields) {
                        *fieldDecl = fieldName->decl;
                        clmGetArrayElementCount(fieldDecl->array, 0, arrayElementCount);
                    }
                }
            }
            if(arrayElementCount) {
                clsDECL nonArrayDecl[1];
                clmDECL_Initialize(nonArrayDecl,
                                   fieldDecl->dataType,
                                   (clsARRAY *)0,
                                   fieldDecl->ptrDscr,
                                   fieldDecl->ptrDominant,
                                   fieldDecl->storageQualifier);

                if (clmDECL_IsScalar(nonArrayDecl)) {
                    status = _MakeTypeCastArgsAsConstant(Compiler,
                                                         (cloIR_TYPECAST_ARGS) member,
                                                         0,
                                                         fieldDecl,
                                                         valStart,
                                                         valEnd);
                    if (gcmIS_ERROR(status)) {
                        return gcvSTATUS_INVALID_DATA;
                    }
                    valStart += clsDECL_GetSize(fieldDecl);
                    gcmASSERT(valStart <= valEnd);
                    /* reduce one dimension */
                }
                else {
                    status = _MakeTypeCastArgsAsConstant(Compiler,
                                                         (cloIR_TYPECAST_ARGS) member,
                                                         0,
                                                         nonArrayDecl,
                                                         valStart,
                                                         valEnd);
                    if (gcmIS_ERROR(status)) {
                        return gcvSTATUS_INVALID_DATA;
                    }
                    valStart += clsDECL_GetSize(nonArrayDecl);
                    gcmASSERT(valStart <= valEnd);
                    /* reduce one array element */
                    gcmASSERT(arrayElementCount > 0);
                    arrayElementCount--;
                    if(arrayElementCount == 0) {
                        fieldDecl->dataType = gcvNULL;
                    }
                }
            }
            else {
                fieldName = (clsNAME *) ((slsDLINK_NODE *) fieldName)->next;
                *fieldDecl = fieldName->decl;
                if((slsDLINK_NODE *)fieldName != structFields) {
                    status = _MakeTypeCastArgsAsConstant(Compiler,
                                                         (cloIR_TYPECAST_ARGS) member,
                                                         0,
                                                         fieldDecl,
                                                         valStart,
                                                         valEnd);
                    if (gcmIS_ERROR(status)) {
                        return gcvSTATUS_INVALID_DATA;
                    }
                    valStart += clsDECL_GetSize(fieldDecl);
                    gcmASSERT(valStart <= valEnd);
                }
                else {
                    gcmASSERT(0);
                    /*error*/
                }
            }
        }
        else {
            cluCONSTANT_VALUE *expectedValEnd;

            expectedValEnd = valStart + valueCount;
            gcmASSERT(expectedValEnd <= ValEnd);
            status = _MakeTypeCastArgsAsConstant(Compiler,
                                                 (cloIR_TYPECAST_ARGS) member,
                                                 braced ? Dim + 1 : Dim,
                                                 constantDecl,
                                                 valStart,
                                                 expectedValEnd);
            if (gcmIS_ERROR(status)) {
                return gcvSTATUS_INVALID_DATA;
            }
            valStart = expectedValEnd;
        }

        break;

     case clvIR_POLYNARY_EXPR:
/** TO DO **/
        break;

     default:
        break;
     }
  }

/*klc*/
  return status;
}

static gctINT
_ParseGetArrayOffset(
IN clsARRAY *Array,
IN cloIR_EXPR Subscript,
OUT gctINT *Offset
)
{
   gctINT leftOffset;
   gctINT rightOffset;
   cloIR_BINARY_EXPR multiDimSubscript;
   gctUINT arrayOffset;
   gctINT offset = 0;
   gctINT dim = -1;

   switch(cloIR_OBJECT_GetType(&Subscript->base)) {
   case clvIR_CONSTANT:
      clmGetArrayElementCount(*Array, 1, arrayOffset);
      offset = cloIR_CONSTANT_GetIntegerValue((cloIR_CONSTANT) (&Subscript->base));
      if(arrayOffset) {
        offset *= arrayOffset;
      }
      break;

   case clvIR_BINARY_EXPR:
      multiDimSubscript = (cloIR_BINARY_EXPR) (&Subscript->base);
      if(clmIR_EXPR_IsBinaryType(multiDimSubscript->leftOperand, clvBINARY_MULTI_DIM_SUBSCRIPT)) {
         dim = _ParseGetArrayOffset(Array,
                                    multiDimSubscript->leftOperand,
                                    &leftOffset);
         if(dim < 0) goto OnError;
      }
      else {
         gcmASSERT(cloIR_OBJECT_GetType(&multiDimSubscript->leftOperand->base) == clvIR_CONSTANT);

         clmGetArrayElementCount(*Array, 1, arrayOffset);
         leftOffset = cloIR_CONSTANT_GetIntegerValue((cloIR_CONSTANT) (&multiDimSubscript->leftOperand->base));

         if(arrayOffset) {
            leftOffset *= arrayOffset;
         }
         dim = 1;
      }

      gcmASSERT(cloIR_OBJECT_GetType(&multiDimSubscript->rightOperand->base) == clvIR_CONSTANT);
      dim++;
      clmGetArrayElementCount(*Array, dim, arrayOffset);
      rightOffset = cloIR_CONSTANT_GetIntegerValue((cloIR_CONSTANT) (&multiDimSubscript->leftOperand->base));
      if(arrayOffset) {
         rightOffset *= arrayOffset;
      }

      offset = leftOffset + rightOffset;
      break;

   default:
      gcmASSERT(0);
      goto OnError;
   }

   *Offset = offset;

OnError:
   return dim;
}

static gctINT
_ParseDesignationOffset(
cloIR_EXPR Designation
)
{
   cloIR_UNARY_EXPR fieldSelection;
   cloIR_BINARY_EXPR subscript;
   gctUINT elementSize;
   gctINT leftOffset;
   gctINT offset;
   gctINT retVal;

   switch(cloIR_OBJECT_GetType(&Designation->base)) {
   case clvIR_UNARY_EXPR:
      fieldSelection = (cloIR_UNARY_EXPR)(&Designation->base);
      gcmASSERT(fieldSelection->type == clvUNARY_FIELD_SELECTION);
      leftOffset = _ParseDesignationOffset(fieldSelection->operand);
      offset = clsDECL_GetFieldOffset(&fieldSelection->operand->decl, fieldSelection->u.fieldName);
      offset += leftOffset;
      break;

   case clvIR_BINARY_EXPR:
      subscript = (cloIR_BINARY_EXPR)(&Designation->base);
      gcmASSERT(subscript->type == clvBINARY_SUBSCRIPT);
      leftOffset = _ParseDesignationOffset(subscript->leftOperand);
      if(leftOffset < 0) return leftOffset;
      elementSize = clsDECL_GetElementSize(&subscript->leftOperand->decl);

      retVal = _ParseGetArrayOffset(&subscript->leftOperand->decl.array,
                                    subscript->rightOperand,
                                    &offset);
      if(retVal < 0) {
          gcmASSERT(0);
          offset = -1;
          break;
      }

      offset = leftOffset + offset * elementSize;
      break;

   case clvIR_VARIABLE:
      offset = 0;
      break;

   default:
      gcmASSERT(0);
      offset = -1;
      break;
   }

   return offset;
}

#define _clmParseDesignationLocation(Designation, Location)  do { \
     gctINT offset; \
     (Location) = gcvNULL; \
     offset = _ParseDesignationOffset(Designation); \
     if(offset >= 0) { \
       (Location) = offset + _ParseLocationMap; \
     } \
  } while(gcvFALSE)

static gctINT
_ParseStoreToLocation(
cloCOMPILER Compiler,
gctINT LineNo,
gctINT StringNo,
cloIR_CONSTANT Constant,
clsDATA_LOCATION_MAP *Location,
gctSIZE_T OperandCount
)
{
   gctINT written;
   gcmASSERT(Location);

   if (OperandCount == 0) {
      written = -1;
   }
   else {
      gceSTATUS status;
      union {
        char * charPtr;
        unsigned char *ucharPtr;
        short * shortPtr;
        unsigned short * ushortPtr;
        int * intPtr;
        unsigned int * uintPtr;
        gctINT64 * longPtr;
        gctUINT64 * ulongPtr;
        gctBOOL * boolPtr;
        float * floatPtr;
      } bufPtr;

      do {
        if (!clmDECL_IsPointerType(&Constant->exprBase.decl) &&
           !clmDECL_IsElementScalar(&Constant->exprBase.decl)) {
           gctUINT vectorSize;

           vectorSize = clmDATA_TYPE_vectorSize_GET(Location->dataType);
           if(vectorSize != Constant->valueCount ||
              OperandCount < Constant->valueCount) {
              written = 0;
              break;
           }
        }

        if(Location->dataType->elementType != Constant->exprBase.decl.dataType->elementType) {
           status = clParseConstantTypeConvert(Constant,
                                               Location->dataType->elementType,
                                               Constant->values);
           if (gcmIS_ERROR(status)) {
               gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                               LineNo,
                                               StringNo,
                                               clvREPORT_ERROR,
                                               "type mismatch between initializers and defined type"));
               return 0;
           }
        }
        bufPtr.charPtr = _ParseConstantBuffer + Location->byteOffset;
        for(written = 0; written < (gctINT)Constant->valueCount; written++) {
           switch(Location->dataType->elementType) {
           case clvTYPE_CHAR:
              *(bufPtr.charPtr)++ = (char)Constant->values[written].intValue;
              break;

           case clvTYPE_UCHAR:
              *(bufPtr.ucharPtr)++ = (unsigned char)Constant->values[written].uintValue;
              break;

           case clvTYPE_SHORT:
              *(bufPtr.shortPtr)++ = (short)Constant->values[written].intValue;
              break;

           case clvTYPE_USHORT:
              *(bufPtr.ushortPtr)++ = (unsigned short)Constant->values[written].uintValue;
              break;

           case clvTYPE_BOOL:
              *(bufPtr.boolPtr)++ = Constant->values[written].boolValue;
              break;

           case clvTYPE_INT:
              *(bufPtr.intPtr)++ = Constant->values[written].intValue;
              break;

           case clvTYPE_UINT:
              *(bufPtr.uintPtr)++ = Constant->values[written].uintValue;
              break;

           case clvTYPE_LONG:
              *(bufPtr.longPtr)++ = Constant->values[written].longValue;
              break;

           case clvTYPE_ULONG:
              *(bufPtr.ulongPtr)++ = Constant->values[written].ulongValue;
              break;

           case clvTYPE_FLOAT:
              *(bufPtr.floatPtr)++ = Constant->values[written].floatValue;
              break;

           case clvTYPE_DOUBLE:
           case clvTYPE_HALF:
              written = 0;
              break;

           default:
              gcmASSERT(0);
              written = 0;
              break;
           }
        }
      } while(gcvFALSE);
   }
   if(written < 0) {
      gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                      LineNo,
                                      StringNo,
                                      clvREPORT_ERROR,
                                      "number of initializers exceeds type defined"));
   }
   else if(written == 0) {
      gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                      LineNo,
                                      StringNo,
                                      clvREPORT_ERROR,
                                      "type mismatch between initializers and defined type"));
   }
   return written;
}

static cloIR_UNARY_EXPR
_ParseFindNearestUnionField(
cloIR_EXPR Expr
)
{
   cloIR_UNARY_EXPR unionField = gcvNULL;
   cloIR_BINARY_EXPR binaryExpr;
   cloIR_UNARY_EXPR unaryExpr;

   switch(cloIR_OBJECT_GetType(&Expr->base)) {
   case clvIR_BINARY_EXPR:
      binaryExpr = (cloIR_BINARY_EXPR)(&Expr->base);
      unionField = _ParseFindNearestUnionField(binaryExpr->leftOperand);
      break;

   case clvIR_UNARY_EXPR:
      unaryExpr = (cloIR_UNARY_EXPR)(&Expr->base);
      gcmASSERT(unaryExpr->type == clvUNARY_FIELD_SELECTION);
      if(clmDECL_IsUnion(&unaryExpr->operand->decl)) {
         unionField = unaryExpr;
      }
      else {
         unionField = _ParseFindNearestUnionField(unaryExpr->operand);
      }
      break;

   default:
      break;
   }
   return unionField;
}

static clsDATA_LOCATION_MAP *
_ParseRestoreUnionLocationData(
cloCOMPILER Compiler,
clsDATA_LOCATION_MAP *UnionLocation
)
{
   gcmASSERT(_ParseSavedUnionLocationMap);

   if(_ParseSavedUnionLocationMap) {
      clsVARIABLE_NESTING *savedNesting;
      clsVARIABLE_NESTING *unionNesting;
      gctSIZE_T savedSize;

      gcmASSERT(_ParseSavedUnionLocationMap->nesting);
      savedNesting = slmSLINK_LIST_First(_ParseSavedUnionLocationMap->nesting, clsVARIABLE_NESTING);
      gcmASSERT(savedNesting);
      savedSize = savedNesting->maxOperandCount * sizeof(clsDATA_LOCATION_MAP);

      unionNesting = slmSLINK_LIST_First(UnionLocation->nesting, clsVARIABLE_NESTING);
      gcmASSERT(unionNesting);

      _ParseFreeNestingLevels(Compiler,
                              UnionLocation,
                              unionNesting->maxOperandCount);

      gcoOS_MemCopy(UnionLocation,
                                 _ParseSavedUnionLocationMap,
                                 savedSize);
      gcmVERIFY_OK(cloCOMPILER_Free(Compiler,
                                    _ParseSavedUnionLocationMap));
      _ParseSavedUnionLocationMap = gcvNULL;

      return UnionLocation + savedNesting->maxOperandCount;

   }
   else return UnionLocation;
}

static gctSIZE_T
_ParseEditLocationMapData(
IN cloCOMPILER Compiler,
IN clsDECL *Decl,
IN clsDATA_LOCATION_MAP *Location,
IN gctSIZE_T *ByteOffset,
IN gctSIZE_T MaxOperandCount
)
{
   gceSTATUS status;
   gctINT numEdited;

   (void)gcoOS_ZeroMemory((gctPOINTER)Location,
                          sizeof(clsDATA_LOCATION_MAP) * MaxOperandCount);


   numEdited = _ParseFillLocationMapData(Compiler,
                                         Decl,
                                         Location,
                                         ByteOffset,
                                         MaxOperandCount);
   if(numEdited == -1) {
      return 0;
   }
   gcmASSERT(numEdited <= (gctINT)MaxOperandCount);

   status = _ParsePushNestingLevel(Compiler,
                                   Location,
                                   numEdited,
                                   MaxOperandCount);
   if (gcmIS_ERROR(status)) return 0;
   return MaxOperandCount;
}

static clsDATA_LOCATION_MAP *
_ParseDesignationLocation(
IN cloCOMPILER Compiler,
IN cloIR_EXPR Designation,
clsDATA_LOCATION_MAP **UnionLocation
)
{
   clsDATA_LOCATION_MAP *location = gcvNULL;
   clsDATA_LOCATION_MAP *unionLocation = gcvNULL;
   cloIR_UNARY_EXPR unionField;
   gctINT offset;

   if(UnionLocation) {
      *UnionLocation = gcvNULL;
   }

   if((unionField = _ParseFindNearestUnionField(Designation)) != gcvNULL) {
      gceSTATUS status;
      clsVARIABLE_NESTING *variableNesting;
      gctSIZE_T savedSize;
      gctSIZE_T numEdited;
      gctSIZE_T byteOffset;
      gctPOINTER pointer;

      unionLocation = _ParseLocationMap + _ParseDesignationOffset(unionField->operand);

      gcmASSERT(unionLocation->nesting);
      variableNesting = slmSLINK_LIST_First(unionLocation->nesting, clsVARIABLE_NESTING);
      gcmASSERT(variableNesting);

      if(_ParseSavedUnionLocationMap) {
          gcmVERIFY_OK(cloCOMPILER_Free(Compiler,
                                        _ParseSavedUnionLocationMap));
          _ParseSavedUnionLocationMap = gcvNULL;
      }

      savedSize = (gctSIZE_T)sizeof(clsDATA_LOCATION_MAP) * variableNesting->maxOperandCount,
      status = cloCOMPILER_Allocate(Compiler,
                                    savedSize,
                                    (gctPOINTER *) &pointer);
      if (gcmIS_ERROR(status)) return gcvNULL;

      _ParseSavedUnionLocationMap = pointer;
      gcoOS_MemCopy(_ParseSavedUnionLocationMap,
                                 unionLocation,
                                 savedSize);

      byteOffset = unionLocation->byteOffset;
      numEdited = _ParseEditLocationMapData(Compiler,
                                            &unionField->u.fieldName->decl,
                                            unionLocation,
                                            &byteOffset,
                                            variableNesting->maxOperandCount);
      if(numEdited == 0) return gcvNULL;
   }

   offset = _ParseDesignationOffset(Designation);
   if(offset >= 0) {
     location = _ParseLocationMap + offset;
   }

   if(UnionLocation) {
     *UnionLocation = unionLocation;
   }
   return location;
}

static gctINT
_MakeStructOrUnionConstant(
cloCOMPILER Compiler,
cloIR_TYPECAST_ARGS TypeCast,
clsVARIABLE_NESTING *Nesting,
clsDATA_LOCATION_MAP *StartLocation,
gctSIZE_T MaxOperandCount
)
{
  cloIR_CONSTANT constMember;
  clsDATA_LOCATION_MAP *location;
  clsDATA_LOCATION_MAP *unionLocation;
  clsVARIABLE_NESTING *currNesting;
  cloIR_BINARY_EXPR designationExpr;
  gctSIZE_T maxOperandCount;
  gctSIZE_T operandCount;
  cloIR_BASE member;
  gctINT written;

  gcmASSERT(TypeCast);
  gcmASSERT(StartLocation);

  location = StartLocation;
  maxOperandCount = MaxOperandCount;

  currNesting = Nesting;

  FOR_EACH_DLINK_NODE(&TypeCast->operands->members, struct _cloIR_BASE, member) {
     if(location > _ParseEndLocationMap) {
        gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                        TypeCast->exprBase.base.lineNo,
                                        TypeCast->exprBase.base.stringNo,
                                        clvREPORT_ERROR,
                                        "number of initializers exceeds type defined"));
        return -1;
     }
     switch(cloIR_OBJECT_GetType(member)) {
     case clvIR_CONSTANT:
        constMember = (cloIR_CONSTANT)member;
        if(clmDECL_IsVectorType(&constMember->exprBase.decl)) {
           _clmParseGetNextNesting(location, currNesting);
            operandCount = currNesting ? currNesting->operandCount : 1;
        }
        else {
            operandCount = 1;
        }
        written = _ParseStoreToLocation(Compiler,
                                        TypeCast->exprBase.base.lineNo,
                                        TypeCast->exprBase.base.stringNo,
                                        constMember,
                                        location,
                                        operandCount);
        if(written <= 0) {
           return written;
        }
        location += written;
        break;

     case clvIR_TYPECAST_ARGS:
        _clmParseGetNextNesting(location, currNesting);
        written = _MakeStructOrUnionConstant(Compiler,
                                            (cloIR_TYPECAST_ARGS) member,
                                            currNesting,
                                            location,
                                            maxOperandCount);
        if(written <= 0) {
           return written;
        }
        location += written;
        break;

     case clvIR_BINARY_EXPR:
        designationExpr = (cloIR_BINARY_EXPR) member;
        location = _ParseDesignationLocation(Compiler,
                                             designationExpr->leftOperand,
                                             &unionLocation);
        if(location == gcvNULL) {
           gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                           TypeCast->exprBase.base.lineNo,
                                           TypeCast->exprBase.base.stringNo,
                                           clvREPORT_ERROR,
                                           "unrecognizable designation"));
           return 0;
        }
        currNesting = gcvNULL;
        _clmParseGetNextNesting(location, currNesting);
        operandCount = currNesting ? currNesting->operandCount : 1;
        switch(cloIR_OBJECT_GetType(&designationExpr->rightOperand->base)) {
        case clvIR_CONSTANT:
           written = _ParseStoreToLocation(Compiler,
                                           TypeCast->exprBase.base.lineNo,
                                           TypeCast->exprBase.base.stringNo,
                                           (cloIR_CONSTANT)(&designationExpr->rightOperand->base),
                                           location,
                                           operandCount);
           break;

        case clvIR_TYPECAST_ARGS:
           written = _MakeStructOrUnionConstant(Compiler,
                                                (cloIR_TYPECAST_ARGS)(&designationExpr->rightOperand->base),
                                                currNesting,
                                                location,
                                                operandCount);
           break;

        default:
           gcmASSERT(0);
           written = 0;
           break;
        }
        if(unionLocation) {
           location = _ParseRestoreUnionLocationData(Compiler,
                                                     unionLocation);
        }
        else {
           location += written;
        }
        if(written <= 0) {
           return written;
        }
        break;

     case clvIR_POLYNARY_EXPR:
/** TO DO **/
        break;

     default:
        break;
     }
     currNesting = gcvNULL;
  }

/* KLC : may not be the right count to return because of designators */
  return location - StartLocation;
}


#if _CREATE_UNNAMED_CONSTANT_IN_MEMORY
static gceSTATUS
_CreateUnnamedConstantExpr(
IN cloCOMPILER Compiler,
IN clsDECL *Decl,
IN cloIR_CONSTANT Constant,
OUT cloIR_EXPR *ConstantExpr
)
{
    gceSTATUS status = gcvSTATUS_OK;
    cloIR_VARIABLE constantVariable;

    gcmASSERT (ConstantExpr);
    if (clmDECL_IsScalar(Decl) ||
        Constant->variable || Constant->allValuesEqual) {
       *ConstantExpr = &Constant->exprBase;
       return gcvSTATUS_OK;
    }

    status = clMakeConstantVariableName(Compiler,
                                        Constant);
    if (gcmIS_ERROR(status)) return status;
    if(_GEN_UNIFORMS_FOR_CONSTANT_ADDRESS_SPACE_VARIABLES) {
        *ConstantExpr = &Constant->exprBase;

        status = cloCOMPILER_AllocateVariableMemory(Compiler,
                                                    Constant->variable);
        if (gcmIS_ERROR(status)) return status;
    }
    else {
        gcmONERROR(clsNAME_SetVariableAddressed(Compiler,
                                                Constant->variable));
        gcmONERROR(cloIR_VARIABLE_Construct(Compiler,
                                            Constant->exprBase.base.lineNo,
                                            Constant->exprBase.base.stringNo,
                                            Constant->variable,
                                            &constantVariable));
        *ConstantExpr = &constantVariable->exprBase;
    }
OnError:
    return status;
}
#endif

cloIR_EXPR
clParseCastExprEnd(
IN cloCOMPILER Compiler,
IN clsDECL *CastType,
IN cloIR_EXPR Operand
)
{
  gctINT parserState;
  cloIR_BASE base;
  cloIR_EXPR expr;
  clsDECL declBuf[1];
  clsDECL *decl = gcvNULL;

  if(Operand == gcvNULL) return gcvNULL;

  parserState = cloCOMPILER_GetParserState(Compiler);
  if(CastType) {
      decl = CastType;
  }
  else {
      clsPARSER_STATE *parserStateHandle;

      parserStateHandle = cloCOMPILER_GetParserStateHandle(Compiler);
      gcmASSERT(parserStateHandle);
      *declBuf = parserStateHandle->castTypeDecl;
      decl = declBuf;
  }

  if(decl->dataType->type == T_TYPE_NAME) {
      gceSTATUS status;

      status = _ParseFlattenType(Compiler, decl, declBuf);
      if(gcmIS_ERROR(status)) return gcvNULL;
      decl = declBuf;
  }

  cloCOMPILER_PopParserState(Compiler);
  base = &Operand->base;
  if(parserState == clvPARSER_IN_TYPE_CAST &&
     (cloIR_OBJECT_GetType(base) == clvIR_TYPECAST_ARGS ||
      clmDECL_IsVectorType(decl))) {
      cloIR_TYPECAST_ARGS typeCastArgs;

      if(clmDECL_IsPointerType(decl) || clmDECL_IsArray(decl)) {
          gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                          Operand->base.lineNo,
                          Operand->base.stringNo,
                          clvREPORT_ERROR,
                          "invalid specifying of vector literal"));
          return gcvNULL;
      }
      if(cloIR_OBJECT_GetType(base) != clvIR_TYPECAST_ARGS) {
          cloIR_EXPR typeCastOperand;

          typeCastOperand = clParseTypeCastArgument(Compiler, Operand, gcvNULL);
          typeCastArgs = (cloIR_TYPECAST_ARGS)&typeCastOperand->base;
      }
      else typeCastArgs = (cloIR_TYPECAST_ARGS)base;
      expr = _ParseConvTypeCastToPolynaryExpr(Compiler,
                                              decl,
                                              typeCastArgs);
/* Created unnamed constant in memory to optimize instruct count*/
#if _CREATE_UNNAMED_CONSTANT_IN_MEMORY
      if(expr) {
         if(cloIR_OBJECT_GetType(&expr->base) == clvIR_CONSTANT &&
            (clmDECL_IsAggregateTypeOverRegLimit(&expr->decl) ||
             (clmDECL_IsExtendedVectorType(&expr->decl) &&
              (clmDECL_IsPackedType(&expr->decl) || !cloCOMPILER_ExtensionEnabled(Compiler, clvEXTENSION_VIV_VX))))) {
            gceSTATUS status;

            status = _CreateUnnamedConstantExpr(Compiler,
                                                decl,
                                                (cloIR_CONSTANT)&expr->base,
                                                &expr);
            if (gcmIS_ERROR(status)) return gcvNULL;
         }
      }
#endif
      return expr;
  }
  else {
      if((decl->dataType->type == T_HALF ||
         Operand->decl.dataType->type == T_HALF) &&
         !clmDECL_IsPointerType(decl) &&
         !cloCOMPILER_ExtensionEnabled(Compiler, clvEXTENSION_VIV_VX | clvEXTENSION_CL_KHR_FP16)) {
           gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                           Operand->base.lineNo,
                                           Operand->base.stringNo,
                                           clvREPORT_ERROR,
                                           "explicit cast of half type not allowed"));
           return gcvNULL;
      }
      return _CreateCastExpr(Compiler, decl, Operand);
  }
}

cloIR_EXPR
clParseSizeofTypeDecl(
IN cloCOMPILER Compiler,
IN clsLexToken *StartToken,
IN clsDECL *Decl
)
{
   cloIR_CONSTANT dataSize;
   cluCONSTANT_VALUE constantValue[1];
   clsDECL declBuf[1];
   clsDECL *declPtr = Decl;

/* INITIALIZE THE CONSTANT WITH THE SIZE OF THE DECLARATION Decl */
   (void)gcoOS_ZeroMemory((gctPOINTER)constantValue, sizeof(cluCONSTANT_VALUE));

   if(declPtr->dataType->type == T_TYPE_NAME) {
       gceSTATUS status;

       status = _ParseFlattenType(Compiler, declPtr, declBuf);
       if(gcmIS_ERROR(status)) return 0;
       declPtr = declBuf;
   }
   constantValue->uintValue = clsDECL_GetByteSize(Compiler, declPtr);
   dataSize = _ParseCreateConstant(Compiler,
                                   StartToken->lineNo,
                                   StartToken->stringNo,
                                   T_SIZE_T,
                                   constantValue);
   if(!dataSize) return gcvNULL;
   else return &dataSize->exprBase;
}

cloIR_EXPR
clParseSizeofExpr(
IN cloCOMPILER Compiler,
IN clsLexToken *StartToken,
IN cloIR_EXPR Expr
)
{
   clsDECL *decl;

   decl = &Expr->decl;
   if(cloIR_OBJECT_GetType(&Expr->base) == clvIR_UNARY_EXPR) {
       cloIR_UNARY_EXPR unaryExpr = (cloIR_UNARY_EXPR) (&Expr->base);

       if(unaryExpr->type == clvUNARY_ADDR && unaryExpr->u.generated) {
           decl = &unaryExpr->u.generated->decl;
       }
   }
   return clParseSizeofTypeDecl(Compiler,
                                StartToken,
                                decl);
}

cloIR_EXPR
clParseVecStep(
IN cloCOMPILER Compiler,
IN clsLexToken *StartToken,
IN clsDECL *Decl
)
{
   cloIR_CONSTANT numElements = gcvNULL;
   clsDECL declBuf[1];
   clsDECL *decl;

   gcmASSERT(Decl);
/* INITIALIZE THE CONSTANT WITH THE NUMBER OF ELEMENTS IN THE DECLARATION Decl */
   do {

     decl = Decl;
     if(decl->dataType->type == T_TYPE_NAME) {
         gceSTATUS status;

         status = _ParseFlattenType(Compiler, decl, declBuf);
         if(gcmIS_ERROR(status)) return gcvNULL;
         decl = declBuf;
     }

     if(clmDECL_IsArithmeticType(decl)) {
       cluCONSTANT_VALUE constantValue[1];
       gctUINT8 elementCount;

       if(clmDECL_IsMat(decl)) {
         gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                         StartToken->lineNo,
                                         StartToken->stringNo,
                                         clvREPORT_ERROR,
                                         "require an scalar or vector typed argument"));
         break;
      }
      (void)gcoOS_ZeroMemory((gctPOINTER)constantValue, sizeof(cluCONSTANT_VALUE));
      elementCount = clmDATA_TYPE_vectorSize_NOCHECK_GET(decl->dataType);
      if(elementCount == 0 || elementCount == 3) {
         elementCount++;
      }
      constantValue->intValue = elementCount;

      numElements = _ParseCreateConstant(Compiler,
                                         StartToken->lineNo,
                                         StartToken->stringNo,
                                         T_INT,
                                         constantValue);
    }
    else {
      gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                      StartToken->lineNo,
                                      StartToken->stringNo,
                                      clvREPORT_ERROR,
                                      "require an integer or floating-point typed argument"));
      break;
    }
  } while(gcvFALSE);

  if(!numElements) return gcvNULL;
  else return &numElements->exprBase;
}

static gctCONST_STRING
_GetBinaryOperatorName(
IN gctINT TokenType
)
{
    switch (TokenType) {
    case '*':        return "*";
    case '/':        return "/";
    case '%':        return "%";

    case '+':        return "+";
    case '-':        return "-";

    case T_LSHIFT_OP:    return "<<";
    case T_RSHIFT_OP:    return ">>";

    case '<':        return "<";
    case '>':        return ">";

    case T_LE_OP:        return "<=";
    case T_GE_OP:        return ">=";

    case T_EQ_OP:        return "==";
    case T_NE_OP:        return "!=";

    case '&':        return "&";
    case '^':        return "^";
    case '|':        return "|";

    case T_AND_OP:        return "&&";
    case T_XOR_OP:        return "^^";
    case T_OR_OP:        return "||";

    case ',':        return ",";

    case '=':        return "=";

    case T_MUL_ASSIGN:    return "*=";
    case T_DIV_ASSIGN:    return "/=";
    case T_MOD_ASSIGN:    return "%=";
    case T_ADD_ASSIGN:    return "+=";
    case T_SUB_ASSIGN:    return "-=";
    case T_LEFT_ASSIGN:    return "<<=";
    case T_RIGHT_ASSIGN:    return ">>=";
    case T_AND_ASSIGN:    return "&=";
    case T_XOR_ASSIGN:    return "^=";
    case T_OR_ASSIGN:    return "|=";

    default:
        gcmASSERT(0);
        return "invalid";
    }
}

static gceSTATUS
_CheckArithmeticExpr(
IN cloCOMPILER Compiler,
IN clsLexToken *Operator,
IN cloIR_EXPR LeftOperand,
IN cloIR_EXPR RightOperand
)
{
  clsDECL *leftDecl;
  clsDECL *rightDecl;
  gctBOOL isMul;

  gcmASSERT(Operator);
  gcmASSERT(LeftOperand);
  gcmASSERT(RightOperand);

  leftDecl = &LeftOperand->decl;
  rightDecl = &RightOperand->decl;

  gcmASSERT(leftDecl->dataType);
  gcmASSERT(rightDecl->dataType);
  /* Check the operands */

  if(!clmDECL_IsArithmeticType(leftDecl)) {
    if(clmDECL_IsArray(leftDecl) &&
       (Operator->u.operator == '+' || Operator->u.operator == '-')) {
       if(clmDECL_IsInt(rightDecl)) return gcvSTATUS_OK;
       gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                       RightOperand->base.lineNo,
                                       RightOperand->base.stringNo,
                                       clvREPORT_ERROR,
                                       "require a scalar integer expression"));
       return gcvSTATUS_INVALID_ARGUMENT;
    }

    gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                    LeftOperand->base.lineNo,
                                    LeftOperand->base.stringNo,
                                    clvREPORT_ERROR,
                                    "require an integer or floating-point typed expression"));
    return gcvSTATUS_INVALID_ARGUMENT;
  }
  else if(clmDECL_IsPointerType(leftDecl)) { /* left operand a pointer */
    if(clmDECL_IsPointerType(rightDecl)) { /* right operand a pointer */
       if(Operator->u.operator != '-') {
          gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                          Operator->lineNo,
                                          Operator->stringNo,
                                          clvREPORT_ERROR,
                                          "operator has to be '-' on two pointer operands"));
          return gcvSTATUS_INVALID_ARGUMENT;
       }
       else if(!clsDECL_IsEqual(leftDecl, rightDecl)) {
          return gcvSTATUS_INVALID_ARGUMENT;
       }
    }
    else {
      if(Operator->u.operator != '+' && Operator->u.operator != '-') {
         gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                         Operator->lineNo,
                                         Operator->stringNo,
                                         clvREPORT_ERROR,
                                         "operator has to be either '+' or '-' for pointer arithmetic"));
         return gcvSTATUS_INVALID_ARGUMENT;
      }
      if (!clmDECL_IsInt(rightDecl)) {
     gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                     RightOperand->base.lineNo,
                     RightOperand->base.stringNo,
                     clvREPORT_ERROR,
                      "require a scalar integer expression"));
     return gcvSTATUS_INVALID_ARGUMENT;
      }
    }
  }

  if(!clmDECL_IsArithmeticType(rightDecl)) {
    gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                    RightOperand->base.lineNo,
                                    RightOperand->base.stringNo,
                                    clvREPORT_ERROR,
                                    "require an integer or floating-point typed expression"));
    return gcvSTATUS_INVALID_ARGUMENT;
  }
  else if(clmDECL_IsPointerType(rightDecl)) { /* right operand a pointer */
    if(!clmDECL_IsPointerType(leftDecl)) {
       if(Operator->u.operator != '+') {
          gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                          Operator->lineNo,
                                          Operator->stringNo,
                                          clvREPORT_ERROR,
                                          "operator has to be '+' for pointer arithmetic"));
          return gcvSTATUS_INVALID_ARGUMENT;
       }

       if (!clmDECL_IsInt(leftDecl)) {
      gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                      LeftOperand->base.lineNo,
                      LeftOperand->base.stringNo,
                      clvREPORT_ERROR,
                      "require a scalar integer expression"));
      return gcvSTATUS_INVALID_ARGUMENT;
       }
    }
  }

  isMul = Operator->u.operator == '*';
  if(clsDECL_IsEqual(leftDecl, rightDecl)) return gcvSTATUS_OK;
  if(clmDECL_IsScalar(leftDecl)) {
      if((clmDECL_IsVectorType(rightDecl) || clmDECL_IsMat(rightDecl)) &&
         clAreElementTypeInRankOrder(Compiler,
                                     leftDecl->dataType->elementType,
                                     rightDecl->dataType->elementType,
                                     cloIR_OBJECT_GetType(&LeftOperand->base) == clvIR_CONSTANT)) {
         gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                         LeftOperand->base.lineNo,
                                         LeftOperand->base.stringNo,
                                         clvREPORT_ERROR,
                                         "conversion from a scalar to a lower ranking vector "
                                         "or matrix type not allowed"));
         return gcvSTATUS_INVALID_ARGUMENT;
      }
  }
  else if(clmDECL_IsScalar(rightDecl)) {
      if(clAreElementTypeInRankOrder(Compiler,
                                     rightDecl->dataType->elementType,
                                     leftDecl->dataType->elementType,
                                     cloIR_OBJECT_GetType(&RightOperand->base) == clvIR_CONSTANT)) {
          gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                          RightOperand->base.lineNo,
                                          RightOperand->base.stringNo,
                                          clvREPORT_ERROR,
                                          "conversion from a scalar to a lower ranking vector "
                                          "or matrix type not allowed"));
          return gcvSTATUS_INVALID_ARGUMENT;
      }
  }
  else if(clmDECL_IsVectorType(leftDecl)) {
     if(clmDECL_IsVectorType(rightDecl)) { /* both are vector types */
       if(!clmDECL_IsSameVectorType(leftDecl, rightDecl)) {
         gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                         LeftOperand->base.lineNo,
                         LeftOperand->base.stringNo,
                         clvREPORT_ERROR,
                         "conversion between different vector types not allowed"));
         return gcvSTATUS_INVALID_ARGUMENT;
       }
     }
     else if (clmDECL_IsMat(rightDecl)) { /* vector and matrix */
       if(leftDecl->dataType->elementType > rightDecl->dataType->elementType) {
         gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                         LeftOperand->base.lineNo,
                         LeftOperand->base.stringNo,
                         clvREPORT_ERROR,
                         "conversion from a vector element type to a lower ranking "
                                         "element type of a matrix not allowed"));
         return gcvSTATUS_INVALID_ARGUMENT;
       }
       if(!isMul) {
         gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                         RightOperand->base.lineNo,
                                         RightOperand->base.stringNo,
                                         clvREPORT_ERROR,
                                         "require a scalar or vector of size %d expression",
                                         clmDATA_TYPE_vectorSize_NOCHECK_GET(leftDecl->dataType)));
         return gcvSTATUS_INVALID_ARGUMENT;
       }
       else if(clmDATA_TYPE_vectorSize_NOCHECK_GET(leftDecl->dataType)
           != clmDATA_TYPE_matrixRowCount_GET(rightDecl->dataType)) {
         gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                         RightOperand->base.lineNo,
                                         RightOperand->base.stringNo,
                                         clvREPORT_ERROR,
                                         "require a scalar or vector of size %d "
                                         "or float%dxm expression",
                                         clmDATA_TYPE_vectorSize_NOCHECK_GET(leftDecl->dataType),
                                         clmDATA_TYPE_vectorSize_NOCHECK_GET(leftDecl->dataType)));
         return gcvSTATUS_INVALID_ARGUMENT;
       }
     }
  }
  else if(clmDECL_IsMat(leftDecl)) { /* Left operand must be a matrix */
     if(clmDECL_IsVectorType(rightDecl)) {
       if(clAreElementTypeInRankOrder(Compiler,
                                      rightDecl->dataType->elementType,
                                      leftDecl->dataType->elementType,
                                      cloIR_OBJECT_GetType(&RightOperand->base) == clvIR_CONSTANT)) {
         gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                         RightOperand->base.lineNo,
                         RightOperand->base.stringNo,
                         clvREPORT_ERROR,
                         "conversion from a vector element type to a lower ranking "
                                         "element type of a matrix not allowed"));
         return gcvSTATUS_INVALID_ARGUMENT;
       }
       if(!isMul) {
         gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                         RightOperand->base.lineNo,
                                         RightOperand->base.stringNo,
                                         clvREPORT_ERROR,
                                         "require a scalar or matrix %dx%d expression",
                                         clmDATA_TYPE_matrixRowCount_GET(leftDecl->dataType),
                                         clmDATA_TYPE_matrixColumnCount_GET(leftDecl->dataType)));
         return gcvSTATUS_INVALID_ARGUMENT;
       }
       else if(clmDATA_TYPE_matrixColumnCount_GET(leftDecl->dataType)
           != clmDATA_TYPE_vectorSize_NOCHECK_GET(rightDecl->dataType)) {
          gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                          RightOperand->base.lineNo,
                                          RightOperand->base.stringNo,
                                          clvREPORT_ERROR,
                                          "require a scalar or vector of size %d "
                                          "or matrix %dxm expression",
                                          clmDATA_TYPE_matrixColumnCount_GET(leftDecl->dataType),
                                          clmDATA_TYPE_matrixColumnCount_GET(leftDecl->dataType)));
          return gcvSTATUS_INVALID_ARGUMENT;
       }
     }
     else if (clmDECL_IsMat(rightDecl)) {
        if(!isMul) {
          if(!clmDECL_IsSameMatrixType(leftDecl, rightDecl)) {
             gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                             LeftOperand->base.lineNo,
                             LeftOperand->base.stringNo,
                             clvREPORT_ERROR,
                             "arithmetic operation on different matrix types not allowed"));
             return gcvSTATUS_INVALID_ARGUMENT;
          }
        }
        else if(clmDATA_TYPE_matrixColumnCount_GET(leftDecl->dataType)
           != clmDATA_TYPE_matrixRowCount_GET(rightDecl->dataType)) {
          gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                          RightOperand->base.lineNo,
                                          RightOperand->base.stringNo,
                                          clvREPORT_ERROR,
                                          "require a scalar or vector of size %d "
                                          "or matrix %dxm expression",
                                          clmDATA_TYPE_matrixColumnCount_GET(leftDecl->dataType),
                                          clmDATA_TYPE_matrixColumnCount_GET(leftDecl->dataType)));
          return gcvSTATUS_INVALID_ARGUMENT;
        }
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

  return gcvSTATUS_OK;
}

static gceSTATUS
_CheckImplicitOperability(
IN cloCOMPILER Compiler,
IN cloIR_EXPR LeftOperand,
IN cloIR_EXPR RightOperand
)
{
  clsDECL *leftDecl;
  clsDECL *rightDecl;

  leftDecl = &LeftOperand->decl;
  rightDecl = &RightOperand->decl;

  gcmASSERT(leftDecl->dataType);
  gcmASSERT(rightDecl->dataType);
  /* Check the operands */

  if(!clmDECL_IsArithmeticType(leftDecl)) {
    gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                    LeftOperand->base.lineNo,
                                    LeftOperand->base.stringNo,
                                    clvREPORT_ERROR,
                                    "arithmetic operand required"));
    return gcvSTATUS_INVALID_ARGUMENT;
  }
  if(!clmDECL_IsArithmeticType(rightDecl)) {
    gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                    RightOperand->base.lineNo,
                                    RightOperand->base.stringNo,
                                    clvREPORT_ERROR,
                                    "arithmetic operand required"));
    return gcvSTATUS_INVALID_ARGUMENT;
  }

  if(clmDECL_IsScalar(leftDecl)) {
    if(clmDECL_IsVectorType(rightDecl)) {
       if(clAreElementTypeInRankOrder(Compiler,
                                      leftDecl->dataType->elementType,
                                      rightDecl->dataType->elementType,
                                      cloIR_OBJECT_GetType(&LeftOperand->base) == clvIR_CONSTANT)) {
         gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                         LeftOperand->base.lineNo,
                                         LeftOperand->base.stringNo,
                                         clvREPORT_ERROR,
                                         "conversion from a scalar to a lower ranking vector not allowed"
                                         "or matrix type not allowed"));
         return gcvSTATUS_INVALID_ARGUMENT;
       }
    }
    else if(!clmDECL_IsScalar(rightDecl)) {
      gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                      RightOperand->base.lineNo,
                                      RightOperand->base.stringNo,
                                      clvREPORT_ERROR,
                                      "require a matching typed expression"));
    }
  }
  else if(clmDECL_IsScalar(rightDecl)) {
    if(clmDECL_IsVectorType(leftDecl)) {
      if(clAreElementTypeInRankOrder(Compiler,
                                     rightDecl->dataType->elementType,
                                     leftDecl->dataType->elementType,
                                     cloIR_OBJECT_GetType(&RightOperand->base) == clvIR_CONSTANT)) {
         gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                         RightOperand->base.lineNo,
                                         RightOperand->base.stringNo,
                                         clvREPORT_ERROR,
                                         "conversion from a scalar to a lower ranking vector not allowed"
                                         "or matrix type not allowed"));
         return gcvSTATUS_INVALID_ARGUMENT;
      }
    }
    else if(!clmDECL_IsScalar(leftDecl)) {
      gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                      LeftOperand->base.lineNo,
                                      LeftOperand->base.stringNo,
                                      clvREPORT_ERROR,
                                      "require a matching typed expression"));
    }
  }
  else if(!clmDECL_IsSameVectorType(leftDecl, rightDecl)) {
    gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                    RightOperand->base.lineNo,
                    RightOperand->base.stringNo,
                    clvREPORT_ERROR,
                    "require a matching vector typed expression"));
    return gcvSTATUS_INVALID_ARGUMENT;
  }
  return gcvSTATUS_OK;
}

static gceSTATUS
_CheckAssignImplicitOperability(
IN cloCOMPILER Compiler,
IN cloIR_EXPR LeftOperand,
IN cloIR_EXPR RightOperand
)
{
  clsDECL *leftDecl;
  clsDECL *rightDecl;

  leftDecl = &LeftOperand->decl;
  rightDecl = &RightOperand->decl;

  gcmASSERT(leftDecl->dataType);
  gcmASSERT(rightDecl->dataType);

  /* Check the operands */
  if(!clmDECL_IsArithmeticType(leftDecl)) {
    gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                    LeftOperand->base.lineNo,
                                    LeftOperand->base.stringNo,
                                    clvREPORT_ERROR,
                                    "arithmetic operand required"));
    return gcvSTATUS_INVALID_ARGUMENT;
  }
  if(!clmDECL_IsArithmeticType(rightDecl)) {
    gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                    RightOperand->base.lineNo,
                                    RightOperand->base.stringNo,
                                    clvREPORT_ERROR,
                                    "arithmetic operand required"));
    return gcvSTATUS_INVALID_ARGUMENT;
  }

  if(clmDECL_IsScalar(leftDecl)) {
    if(!clmDECL_IsScalar(rightDecl)) {
         gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                         RightOperand->base.lineNo,
                                         RightOperand->base.stringNo,
                         clvREPORT_ERROR,
                         "need a scalar on the right hand side"));
         return gcvSTATUS_INVALID_ARGUMENT;
    }
  }
  else if(clmDECL_IsVectorType(leftDecl)) {
    if(!clmDECL_IsScalar(rightDecl) &&
       !clmDECL_IsSameVectorType(leftDecl, rightDecl)) {
            gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                            RightOperand->base.lineNo,
                            RightOperand->base.stringNo,
                            clvREPORT_ERROR,
                            "require a scalar or a matching vector typed expression"));
       return gcvSTATUS_INVALID_ARGUMENT;
    }
  }
  return gcvSTATUS_OK;
}

static gceSTATUS
_CheckLogicalExpr(
IN cloCOMPILER Compiler,
IN cloIR_EXPR LeftOperand,
IN cloIR_EXPR RightOperand
)
{
  gcmASSERT(LeftOperand);
  gcmASSERT(LeftOperand->decl.dataType);
  gcmASSERT(RightOperand);
  gcmASSERT(RightOperand->decl.dataType);

  if(clmDECL_IsSampler(&LeftOperand->decl)) {
     if(!clmDECL_IsSampler(&RightOperand->decl)) {
    gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                    RightOperand->base.lineNo,
                    RightOperand->base.stringNo,
                    clvREPORT_ERROR,
                    "require sampler_t typed operands"));
    return gcvSTATUS_INVALID_ARGUMENT;
     }
     else return gcvSTATUS_OK;
  }
  if(clmDECL_IsSampler(&RightOperand->decl)) {
     if(!clmDECL_IsSampler(&LeftOperand->decl)) {
    gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                    LeftOperand->base.lineNo,
                    LeftOperand->base.stringNo,
                    clvREPORT_ERROR,
                    "require sampler_t typed operands"));
    return gcvSTATUS_INVALID_ARGUMENT;
     }
     else return gcvSTATUS_OK;
  }

/* Check the operands */
/* operands need not to be of integer types */
  return _CheckImplicitOperability(Compiler, LeftOperand, RightOperand);
}

static gceSTATUS
_CheckBitwiseShiftExpr(
IN cloCOMPILER Compiler,
IN cloIR_EXPR LeftOperand,
IN cloIR_EXPR RightOperand
)
{
    gcmASSERT(LeftOperand);
    gcmASSERT(LeftOperand->decl.dataType);
    gcmASSERT(RightOperand);
    gcmASSERT(RightOperand->decl.dataType);

    /* Check the operands */
    if (clmDECL_IsInt(&LeftOperand->decl) &&
        !clmDECL_IsInt(&RightOperand->decl)) {
        gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                        RightOperand->base.lineNo,
                        RightOperand->base.stringNo,
                        clvREPORT_ERROR,
                        "require an integer expression"));
        return gcvSTATUS_INVALID_ARGUMENT;
    }


    if (!clmDECL_IsIntOrIVec(&LeftOperand->decl)) {
        gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                        LeftOperand->base.lineNo,
                        LeftOperand->base.stringNo,
                        clvREPORT_ERROR,
                        "require an integer expression"));
        return gcvSTATUS_INVALID_ARGUMENT;
    }

    if (!clmDECL_IsIntOrIVec(&RightOperand->decl)) {
        gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                        RightOperand->base.lineNo,
                        RightOperand->base.stringNo,
                        clvREPORT_ERROR,
                        "require an integer expression"));
        return gcvSTATUS_INVALID_ARGUMENT;
    }
    return gcvSTATUS_OK;
}

static gceSTATUS
_CheckLogicalAssignmentExpr(
IN cloCOMPILER Compiler,
IN clsLexToken *Operator,
IN cloIR_EXPR LeftOperand,
IN cloIR_EXPR RightOperand
)
{
  gceSTATUS status;
  clsDECL *leftDecl;
  clsDECL *rightDecl;

  gcmASSERT(LeftOperand);
  gcmASSERT(RightOperand);

/* Check the left operand */
  status = _CheckLValueExpr(Compiler, LeftOperand, "left operand");
  if (gcmIS_ERROR(status)) return status;

  leftDecl = &LeftOperand->decl;
  rightDecl = &RightOperand->decl;

  gcmASSERT(leftDecl->dataType);
  gcmASSERT(rightDecl->dataType);

  if(Operator->u.operator == T_LEFT_ASSIGN ||
     Operator->u.operator == T_RIGHT_ASSIGN) {
     return _CheckBitwiseShiftExpr(Compiler,
                                   LeftOperand,
                                   RightOperand);
  }
  else {
     if (!clmDECL_IsIntOrIVec(&LeftOperand->decl)) {
    gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                    LeftOperand->base.lineNo,
                    LeftOperand->base.stringNo,
                    clvREPORT_ERROR,
                    "require an integer expression"));
    return gcvSTATUS_INVALID_ARGUMENT;
     }

     if (!clmDECL_IsIntOrIVec(&RightOperand->decl)) {
    gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                    RightOperand->base.lineNo,
                    RightOperand->base.stringNo,
                    clvREPORT_ERROR,
                    "require an integer expression"));
    return gcvSTATUS_INVALID_ARGUMENT;
     }
     return _CheckAssignImplicitOperability(Compiler,
                                            LeftOperand,
                                            RightOperand);
  }
}

static gceSTATUS
_CheckSequenceExpr(
IN cloCOMPILER Compiler,
IN cloIR_EXPR LeftOperand,
IN cloIR_EXPR RightOperand
)
{
    gcmASSERT(LeftOperand);
    gcmASSERT(LeftOperand->decl.dataType);
    gcmASSERT(RightOperand);
    gcmASSERT(RightOperand->decl.dataType);

    return gcvSTATUS_OK;
}

gceSTATUS
clParseMakeArrayPointerExpr(
IN cloCOMPILER Compiler,
IN cloIR_EXPR ArrayOperand,
IN OUT cloIR_EXPR *ArrayPointerExpr
)
{
    gceSTATUS status;
    cloIR_EXPR expr;
    cloIR_EXPR resExpr;

    /* Create the expression &A[0] for A being an array*/
    expr =  _EvaluateIndirectionExpr(Compiler, ArrayOperand);
    gcmASSERT(expr);
    status = cloIR_UNARY_EXPR_Construct(Compiler,
                                        ArrayOperand->base.lineNo,
                                        ArrayOperand->base.stringNo,
                                        clvUNARY_ADDR,
                                        expr,
                                        gcvNULL,
                                        gcvNULL,
                                        &resExpr);
    if (gcmIS_ERROR(status)) return status;

    status = clParseSetOperandAddressed(Compiler,
                                        ArrayOperand);
    if (gcmIS_ERROR(status)) return status;

    *ArrayPointerExpr = resExpr;
    return status;
}

cloIR_EXPR
clParseAsmAppendOperandModifiers(
IN cloCOMPILER Compiler,
IN cloIR_EXPR  Expr,
IN clsASM_MODIFIERS *Modifiers
)
{
    gcmHEADER_ARG("Modifiers=0x%x", Modifiers);
    gcmASSERT(Modifiers);

    if (Modifiers)
    {
        gctPOINTER pointer = gcvNULL;
        clsASM_MODIFIERS *modifierCopy;

        cloCOMPILER_Allocate(
                        Compiler,
                        (gctSIZE_T)sizeof(clsASM_MODIFIERS),
                        &pointer);

        modifierCopy = (clsASM_MODIFIERS *) pointer;

        *modifierCopy = *Modifiers;

        Modifiers = modifierCopy;
    }

    if (Expr)
    {
        Expr->asmMods = Modifiers;
    }

    gcmFOOTER_ARG("<return>=0x%x", Expr);
    return  Expr;
}

clsASM_MODIFIERS
clParseAsmAppendModifier(
IN cloCOMPILER Compiler,
IN clsASM_MODIFIERS *Modifiers,
IN clsASM_MODIFIER  *Modifier
)
{
    clsASM_MODIFIERS modifiers;

    gcmHEADER_ARG("Modifier=0x%x", Modifier);
    gcmASSERT(Modifier);

    if (!Modifiers)
    {
        gcoOS_MemFill((gctPOINTER)&modifiers, (gctUINT8)-1, sizeof(clsASM_MODIFIERS));

        Modifiers = (clsASM_MODIFIERS *) &modifiers;
    }

    Modifiers->modifiers[Modifier->type] = *Modifier;

    gcmFOOTER_ARG("<return>=0x%x", Modifiers);
    return  *Modifiers;
}

clsASM_MODIFIER
clParseAsmModifier(
IN cloCOMPILER Compiler,
IN clsLexToken * Type,
IN clsLexToken * Value
)
{
    clsASM_MODIFIER asmMod;

    gcmHEADER_ARG("Opcode=0x%x, Opcode=0x%x", Type, Value);
    gcmASSERT(Type && Value);

    gcoOS_ZeroMemory((gctPOINTER)&asmMod, sizeof(clsASM_MODIFIER));

    asmMod.type = cleASM_MODIFIER_COUNT; /* to indicate as invalid type */

    /* DATATYPE */
    if(gcmIS_SUCCESS(gcoOS_StrCmp(Type->u.identifier.name, "f")))
    {
#define STRING_MATCH_FORMAT(STR) \
    if(gcmIS_SUCCESS(gcoOS_StrCmp(Value->u.identifier.name, #STR))) { \
    asmMod.type = cleASM_MODIFIER_OPND_FORMAT;   \
    asmMod.value = clvTYPE_##STR;   \
    }

        STRING_MATCH_FORMAT(BOOL)
        else STRING_MATCH_FORMAT(CHAR)
        else STRING_MATCH_FORMAT(UCHAR)
        else STRING_MATCH_FORMAT(SHORT)
        else STRING_MATCH_FORMAT(USHORT)
        else STRING_MATCH_FORMAT(INT)
        else STRING_MATCH_FORMAT(UINT)
        else STRING_MATCH_FORMAT(LONG)
        else STRING_MATCH_FORMAT(ULONG)
        else STRING_MATCH_FORMAT(HALF)
        else STRING_MATCH_FORMAT(FLOAT)
        else STRING_MATCH_FORMAT(DOUBLE)
        else STRING_MATCH_FORMAT(QUAD)
        else {
            gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                Value->lineNo,
                Value->stringNo,
                clvREPORT_ERROR,
                "unknown token: '%s'",
                Value->u.identifier.name));
            gcmFOOTER_ARG("<return>=%s", "<nil>");
            return asmMod;
        }

    }
    /* PRECISION */
    else if(gcmIS_SUCCESS(gcoOS_StrCmp(Type->u.identifier.name, "p")))
    {
#define STRING_MATCH_PRECISION(STR) \
    if(gcmIS_SUCCESS(gcoOS_StrCmp(Value->u.identifier.name, #STR))) { \
    asmMod.type  = cleASM_MODIFIER_OPND_PRECISION;   \
    asmMod.value = gcSHADER_PRECISION_##STR;   \
    }

        STRING_MATCH_PRECISION(DEFAULT)
        else STRING_MATCH_PRECISION(HIGH)
        else STRING_MATCH_PRECISION(MEDIUM)
        else STRING_MATCH_PRECISION(LOW)
        else {
            gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                Value->lineNo,
                Value->stringNo,
                clvREPORT_ERROR,
                "unknown token: '%s'",
                Value->u.identifier.name));
            gcmFOOTER_ARG("<return>=%s", "<nil>");
            return asmMod;
        }

#undef STRING_MATCH_PRECISION
    }
    else if(gcmIS_SUCCESS(gcoOS_StrCmp(Type->u.identifier.name, "c")))
    {
#define STRING_MATCH_CONDITION(STR) \
    if(gcmIS_SUCCESS(gcoOS_StrCmp(Value->u.identifier.name, #STR))) { \
    asmMod.type = cleASM_MODIFIER_OPCODE_CONDITION;   \
    asmMod.value = gcSL_##STR;   \
    }
        /*
        Regular expression:
        Find:    ^[ ]+gcSL_([a-zA-Z0-9_]+),.*
        Replace: else STRING_MATCH_CONDITION(\1)
        */
        STRING_MATCH_CONDITION(ALWAYS)
        else STRING_MATCH_CONDITION(NOT_EQUAL)
        else STRING_MATCH_CONDITION(LESS_OR_EQUAL)
        else STRING_MATCH_CONDITION(LESS)
        else STRING_MATCH_CONDITION(EQUAL)
        else STRING_MATCH_CONDITION(GREATER)
        else STRING_MATCH_CONDITION(GREATER_OR_EQUAL)
        else STRING_MATCH_CONDITION(AND)
        else STRING_MATCH_CONDITION(OR)
        else STRING_MATCH_CONDITION(XOR)
        else STRING_MATCH_CONDITION(NOT_ZERO)
        else STRING_MATCH_CONDITION(ZERO)
        else STRING_MATCH_CONDITION(GREATER_OR_EQUAL_ZERO)
        else STRING_MATCH_CONDITION(GREATER_ZERO)
        else STRING_MATCH_CONDITION(LESS_OREQUAL_ZERO)
        else STRING_MATCH_CONDITION(LESS_ZERO)
        else {
            gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                Value->lineNo,
                Value->stringNo,
                clvREPORT_ERROR,
                "unknown token: '%s'",
                Value->u.identifier.name));
            gcmFOOTER_ARG("<return>=%s", "<nil>");
            return asmMod;
        }
#undef STRING_MATCH_CONDITION
    }
    else if(gcmIS_SUCCESS(gcoOS_StrCmp(Type->u.identifier.name, "t")))
    {
#define STRING_MATCH_THREAD(STR) \
    if(gcmIS_SUCCESS(gcoOS_StrCmp(Value->u.identifier.name, #STR))) { \
    asmMod.type = cleASM_MODIFIER_THREAD_OPCODE_MODE;   \
    asmMod.value = gcSL_##STR;   \
    }


#undef STRING_MATCH_THREAD
    }
    else if(gcmIS_SUCCESS(gcoOS_StrCmp(Type->u.identifier.name, "rnd")))
    {
#define STRING_MATCH_ROUND(STR) \
    if(gcmIS_SUCCESS(gcoOS_StrCmp(Value->u.identifier.name, #STR))) { \
    asmMod.type = cleASM_MODIFIER_OPCODE_ROUND;   \
    asmMod.value = gcSL_ROUND_##STR;   \
    }
        STRING_MATCH_ROUND(DEFAULT)
        else STRING_MATCH_ROUND(RTZ)
        else STRING_MATCH_ROUND(RTNE)
        else STRING_MATCH_ROUND(RTP)
        else STRING_MATCH_ROUND(RTN)
        else {
            gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                Value->lineNo,
                Value->stringNo,
                clvREPORT_ERROR,
                "unknown token: '%s'",
                Value->u.identifier.name));
            gcmFOOTER_ARG("<return>=%s", "<nil>");
            return asmMod;
        }

#undef STRING_MATCH_ROUND
    }
    else if(gcmIS_SUCCESS(gcoOS_StrCmp(Type->u.identifier.name, "sat")))
    {
#define STRING_MATCH_SATURATE(STR) \
    if(gcmIS_SUCCESS(gcoOS_StrCmp(Value->u.identifier.name, #STR))) { \
    asmMod.type = cleASM_MODIFIER_OPCODE_SAT;   \
    asmMod.value = gcSL_##STR;   \
    }
        STRING_MATCH_SATURATE(NO_SATURATE)
        else STRING_MATCH_SATURATE(SATURATE)
        else {
            gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                Value->lineNo,
                Value->stringNo,
                clvREPORT_ERROR,
                "unknown token: '%s'",
                Value->u.identifier.name));
            gcmFOOTER_ARG("<return>=%s", "<nil>");
            return asmMod;
        }
#undef STRING_MATCH_SATURATE
    }
    else if(gcmIS_SUCCESS(gcoOS_StrCmp(Type->u.identifier.name, "abs")))
    {
#define STRING_MATCH_ABS(STR) \
    if(gcmIS_SUCCESS(gcoOS_StrCmp(Value->u.identifier.name, #STR))) { \
    asmMod.type = cleASM_MODIFIER_OPND_ABS;   \
    asmMod.value = 1;   \
    }
        STRING_MATCH_ABS(1)
        else {
            gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                Value->lineNo,
                Value->stringNo,
                clvREPORT_ERROR,
                "unknown token: '%s'",
                Value->u.identifier.name));
            gcmFOOTER_ARG("<return>=%s", "<nil>");
            return asmMod;
        }
#undef STRING_MATCH_ABS
    }
    else if(gcmIS_SUCCESS(gcoOS_StrCmp(Type->u.identifier.name, "neg")))
    {
#define STRING_MATCH_NEG(STR) \
    if(gcmIS_SUCCESS(gcoOS_StrCmp(Value->u.identifier.name, #STR))) { \
    asmMod.type = cleASM_MODIFIER_OPND_NEG;   \
    asmMod.value = 1;   \
    }
        STRING_MATCH_NEG(1)
        else {
            gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                Value->lineNo,
                Value->stringNo,
                clvREPORT_ERROR,
                "unknown token: '%s'",
                Value->u.identifier.name));
            gcmFOOTER_ARG("<return>=%s", "<nil>");
            return asmMod;
        }
#undef STRING_MATCH_NEG
    }
    else {
        gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
            Type->lineNo,
            Type->stringNo,
            clvREPORT_ERROR,
            "unknown type: '%s'",
            Type->u.identifier.name));
        gcmFOOTER_ARG("<return>=%s", "<nil>");
        return asmMod;
    }

    gcmFOOTER_ARG("<return>=0x%x", &asmMod);
    return  asmMod;
}

cloIR_EXPR
clParseNormalBinaryExpr(
IN cloCOMPILER Compiler,
IN cloIR_EXPR LeftOperand,
IN clsLexToken * Operator,
IN cloIR_EXPR RightOperand
)
{
    gceSTATUS        status;
    cleBINARY_EXPR_TYPE    exprType = (cleBINARY_EXPR_TYPE) 0;
    cloIR_CONSTANT        resultConstant;
    cloIR_EXPR    resExpr;

    gcmASSERT(Operator);
    if (LeftOperand == gcvNULL || RightOperand == gcvNULL) return gcvNULL;

    if (clmDECL_IsPackedGenType(&LeftOperand->decl) ||
        clmDECL_IsPackedGenType(&RightOperand->decl)) {
        gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                        Operator->lineNo,
                                        Operator->stringNo,
                                        clvREPORT_ERROR,
                                        "_viv_gentype_packed operands not allowed in binary operator '%s'",
                                        _GetBinaryOperatorName(Operator->u.operator)));
        return gcvNULL;
    }

    if (clmDECL_IsHalfType(&LeftOperand->decl) ||
        clmDECL_IsHalfType(&RightOperand->decl)) {
        if(!cloCOMPILER_ExtensionEnabled(Compiler, clvEXTENSION_CL_KHR_FP16)) {
            gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                            Operator->lineNo,
                                            Operator->stringNo,
                                            clvREPORT_ERROR,
                                            "operands of type half not supported in binary operator '%s'",
                                            _GetBinaryOperatorName(Operator->u.operator)));
            return gcvNULL;
        }
    }

    switch (Operator->u.operator) {
    case T_LSHIFT_OP:
    case T_RSHIFT_OP:
        status = _CheckBitwiseShiftExpr(Compiler,
                            LeftOperand,
                            RightOperand);
        if (gcmIS_ERROR(status)) return gcvNULL;
        switch (Operator->u.operator) {
        case T_LSHIFT_OP:
                          exprType = clvBINARY_LSHIFT;
                          break;
        case T_RSHIFT_OP:
                          exprType = clvBINARY_RSHIFT;
                          break;
        }
        break;

    case '&':
    case '^':
    case '|':
        status = _CheckLogicalExpr(Compiler,
                       LeftOperand,
                       RightOperand);
        if (gcmIS_ERROR(status)) return gcvNULL;
        switch (Operator->u.operator) {
        case '&': exprType = clvBINARY_AND_BITWISE;
                          break;
        case '|': exprType = clvBINARY_OR_BITWISE;
                          break;
        case '^': exprType = clvBINARY_XOR_BITWISE;
                          break;
        }
        break;

    case '+':
    case '-':
    case '%':
    case '*':
    case '/':
        status = _CheckArithmeticExpr(Compiler,
                                      Operator,
                                      LeftOperand,
                                      RightOperand);
        if (gcmIS_ERROR(status)) return gcvNULL;
        switch (Operator->u.operator) {
        case '*':
            exprType = clvBINARY_MUL;
            break;
        case '/':
            if((clmDECL_IsFloatingType(&RightOperand->decl) ||
               clmDECL_IsFloatingType(&LeftOperand->decl)) &&
               _clmExprIsConstantForEval(RightOperand)) {
                cloIR_CONSTANT constantOne;
                cloIR_CONSTANT resultConstant;
                cluCONSTANT_VALUE constantValue;

                (void)gcoOS_ZeroMemory((gctPOINTER)&constantValue, sizeof(cluCONSTANT_VALUE));
                constantValue.floatValue = 1.0;
                constantOne = _ParseCreateConstant(Compiler,
                                   RightOperand->base.lineNo,
                                   RightOperand->base.stringNo,
                                   T_FLOAT,
                                   &constantValue);
                if(!constantOne) return gcvNULL;
                status = cloIR_BINARY_EXPR_Evaluate(Compiler,
                                                    clvBINARY_DIV,
                                                    constantOne,
                                                    (cloIR_CONSTANT)(&RightOperand->base),
                                                    gcvNULL,
                                                    &resultConstant);
                if (gcmIS_ERROR(status)) return gcvNULL;
                RightOperand = &resultConstant->exprBase;
                exprType = clvBINARY_MUL;
            }
            else exprType = clvBINARY_DIV;
            break;

        case '+':
            if(clmDECL_IsArray(&LeftOperand->decl)) {
                status =  clParseMakeArrayPointerExpr(Compiler,
                                                      LeftOperand,
                                                      &LeftOperand);
                if (gcmIS_ERROR(status)) return gcvNULL;
            }
            exprType = clvBINARY_ADD;
            break;
        case '-':
            if(clmDECL_IsArray(&LeftOperand->decl)) {
                status =  clParseMakeArrayPointerExpr(Compiler,
                                                      LeftOperand,
                                                      &LeftOperand);
                if (gcmIS_ERROR(status)) return gcvNULL;
            }
            exprType = clvBINARY_SUB;
            break;
        case '%':
            exprType = clvBINARY_MOD;
            break;
        }
        break;

    case '<':
    case '>':
    case T_LE_OP:
    case T_GE_OP:
        status = _CheckImplicitOperability(Compiler,
                               LeftOperand,
                               RightOperand);
        if (gcmIS_ERROR(status)) return gcvNULL;
        switch (Operator->u.operator) {
        case '<': exprType = clvBINARY_LESS_THAN;
                          break;
        case '>': exprType = clvBINARY_GREATER_THAN;
                          break;
        case T_LE_OP:
                          exprType = clvBINARY_LESS_THAN_EQUAL;
                          break;
        case T_GE_OP:
                          exprType = clvBINARY_GREATER_THAN_EQUAL;
                          break;
        }
        break;

    case T_EQ_OP:
    case T_NE_OP:
        status = _CheckImplicitOperability(Compiler,
                               LeftOperand,
                               RightOperand);
        if (gcmIS_ERROR(status)) return gcvNULL;
        switch (Operator->u.operator) {
        case T_EQ_OP: exprType = clvBINARY_EQUAL;
                              break;
        case T_NE_OP: exprType = clvBINARY_NOT_EQUAL;
                              break;
        }
        break;

    case T_XOR_OP:
        gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                        Operator->lineNo,
                        Operator->stringNo,
                        clvREPORT_ERROR,
                        "reserved binary operator '%s'",
                        _GetBinaryOperatorName(Operator->u.operator)));
        return gcvNULL;

    case T_AND_OP:
    case T_OR_OP:
        status = _CheckLogicalExpr(Compiler,
                       LeftOperand,
                       RightOperand);
        if (gcmIS_ERROR(status)) return gcvNULL;
        switch (Operator->u.operator) {
        case T_AND_OP:    exprType = clvBINARY_AND;
                                break;
        case T_OR_OP:    exprType = clvBINARY_OR;
                                break;
        }
        break;

    case ',':
        status = _CheckSequenceExpr(Compiler,
                        LeftOperand,
                        RightOperand);
        if (gcmIS_ERROR(status)) return gcvNULL;
        exprType = clvBINARY_SEQUENCE;

        if (cloIR_OBJECT_GetType(&LeftOperand->base) == clvIR_CONSTANT) {
            gcmVERIFY_OK(cloIR_OBJECT_Destroy(Compiler, &LeftOperand->base));
            return RightOperand;
        }
        break;

    default:
        gcmASSERT(0);
        return gcvNULL;
    }

    /* Constant calculation */
    if (_clmExprIsConstantForEval(LeftOperand) &&
        _clmExprIsConstantForEval(RightOperand)) {
        status = cloIR_BINARY_EXPR_Evaluate(Compiler,
                            exprType,
                            (cloIR_CONSTANT)LeftOperand,
                            (cloIR_CONSTANT)RightOperand,
                            gcvNULL,
                            &resultConstant);

        if (gcmIS_ERROR(status)) return gcvNULL;
        return &resultConstant->exprBase;
    }

    /* Create binary expression */
    status = cloIR_BINARY_EXPR_Construct(Compiler,
                         LeftOperand->base.lineNo,
                         LeftOperand->base.stringNo,
                         exprType,
                         LeftOperand,
                         RightOperand,
                         &resExpr);
    if (gcmIS_ERROR(status)) return gcvNULL;

    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                      clvDUMP_PARSER,
                      "<BINARY_EXPR type=\"%s\" line=\"%d\" string=\"%d\" />",
                      _GetBinaryOperatorName(Operator->u.operator),
                      LeftOperand->base.lineNo,
                      LeftOperand->base.stringNo));
    return resExpr;
}

cloIR_EXPR
clParseArrayDeclarator(
IN cloCOMPILER Compiler,
IN clsLexToken * StartToken,
IN cloIR_EXPR ArrayDecl,
IN cloIR_EXPR ArraySize
)
{
   gceSTATUS status;
   cloIR_BINARY_EXPR binaryExpr;

   status = cloIR_ArrayDeclarator_Construct(Compiler,
                                            StartToken->lineNo,
                                            StartToken->stringNo,
                                            ArrayDecl,
                                            ArraySize,
                                            &binaryExpr);
   if(gcmIS_ERROR(status)) return gcvNULL;

   return &binaryExpr->exprBase;
}

cloIR_EXPR
clParseBinarySequenceExpr(
IN cloCOMPILER Compiler,
IN YYSTYPE *ParseStack,
IN cloIR_EXPR LeftOperand,
IN clsLexToken * Operator,
IN cloIR_EXPR RightOperand
)
{
    gceSTATUS status;
    YYSTYPE    *parseStack;
    gctBOOL    isBinarySequence = gcvFALSE;

    gcmASSERT(Operator && Operator->u.operator == ',');
    gcmASSERT(ParseStack);
    if (LeftOperand == gcvNULL || RightOperand == gcvNULL) return gcvNULL;

    for(parseStack = ParseStack;
        parseStack->token.type != T_TYPE_CAST;
        parseStack--) {
        if(parseStack->token.type == '(') {
            continue;
        }
        else isBinarySequence = gcvTRUE;
        break;
    }

    if(isBinarySequence) {
       cloIR_CONSTANT    resultConstant;
       cloIR_EXPR    resExpr;

       status = _CheckSequenceExpr(Compiler,
                                   LeftOperand,
                                   RightOperand);
       if (gcmIS_ERROR(status)) return gcvNULL;
       /* Constant calculation */
       if (_clmExprIsConstantForEval(LeftOperand) &&
           _clmExprIsConstantForEval(RightOperand)) {
           status = cloIR_BINARY_EXPR_Evaluate(Compiler,
                                               clvBINARY_SEQUENCE,
                                               (cloIR_CONSTANT)LeftOperand,
                                               (cloIR_CONSTANT)RightOperand,
                                               gcvNULL,
                                               &resultConstant);

           if (gcmIS_ERROR(status)) return gcvNULL;
           return &resultConstant->exprBase;
       }

       /* Create binary expression */
       status = cloIR_BINARY_EXPR_Construct(Compiler,
                                            LeftOperand->base.lineNo,
                                            LeftOperand->base.stringNo,
                                            clvBINARY_SEQUENCE,
                                            LeftOperand,
                                            RightOperand,
                                            &resExpr);
       if (gcmIS_ERROR(status)) return gcvNULL;

       gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                                     clvDUMP_PARSER,
                                     "<BINARY_EXPR type=\"%s\" line=\"%d\" string=\"%d\" />",
                                     _GetBinaryOperatorName(Operator->u.operator),
                                     LeftOperand->base.lineNo,
                                     LeftOperand->base.stringNo));
       return resExpr;
    }
    else return clParseTypeCastArgument(Compiler, LeftOperand, RightOperand);
}

static gceSTATUS
_CheckNonMatrixArithmeticOperands(
IN cloCOMPILER Compiler,
IN cloIR_EXPR LeftOperand,
IN cloIR_EXPR RightOperand
)
{
  clsDECL *leftDecl;
  clsDECL *rightDecl;

  gcmASSERT(LeftOperand);
  gcmASSERT(RightOperand);

  leftDecl = &LeftOperand->decl;
  rightDecl = &RightOperand->decl;

  gcmASSERT(leftDecl->dataType);
  gcmASSERT(rightDecl->dataType);
  /* Check the operands */

  if(!clmDECL_IsArithmeticType(leftDecl)) {
    gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                    LeftOperand->base.lineNo,
                                    LeftOperand->base.stringNo,
                                    clvREPORT_ERROR,
                                    "require an integer or floating-point typed operand"));
    return gcvSTATUS_INVALID_ARGUMENT;
  }
  else if(clmDECL_IsMat(leftDecl)) {
    gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                    LeftOperand->base.lineNo,
                                    LeftOperand->base.stringNo,
                                    clvREPORT_ERROR,
                                    "require a scalar or vector typed operand"));
    return gcvSTATUS_INVALID_ARGUMENT;
  }

  if(!clmDECL_IsArithmeticType(rightDecl)) {
    gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                    RightOperand->base.lineNo,
                                    RightOperand->base.stringNo,
                                    clvREPORT_ERROR,
                                    "require an integer or floating-point typed expression"));
    return gcvSTATUS_INVALID_ARGUMENT;
  }
  else if(clmDECL_IsMat(rightDecl)) {
    gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                    RightOperand->base.lineNo,
                                    RightOperand->base.stringNo,
                                    clvREPORT_ERROR,
                                    "require a scalar or vector typed operand"));
    return gcvSTATUS_INVALID_ARGUMENT;
  }

  if(clsDECL_IsEqual(leftDecl, rightDecl)) return gcvSTATUS_OK;
  if(clmDECL_IsScalar(leftDecl)) {
      if(clmDECL_IsVectorType(rightDecl) &&
         clAreElementTypeInRankOrder(Compiler,
                                     leftDecl->dataType->elementType,
                                     rightDecl->dataType->elementType,
                                     cloIR_OBJECT_GetType(&LeftOperand->base) == clvIR_CONSTANT)) {
         gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                         LeftOperand->base.lineNo,
                                         LeftOperand->base.stringNo,
                                         clvREPORT_ERROR,
                                         "conversion from a scalar to a lower ranking vector not allowed"));
         return gcvSTATUS_INVALID_ARGUMENT;
      }
  }
  else {  /* left operand being a vector */
      if(clmDECL_IsScalar(rightDecl)) {
          if(clAreElementTypeInRankOrder(Compiler,
                                         rightDecl->dataType->elementType,
                                         leftDecl->dataType->elementType,
                                         cloIR_OBJECT_GetType(&RightOperand->base) == clvIR_CONSTANT)) {
              gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                              RightOperand->base.lineNo,
                                              RightOperand->base.stringNo,
                                              clvREPORT_ERROR,
                                              "conversion from a scalar to a lower ranking vector not allowed"));
              return gcvSTATUS_INVALID_ARGUMENT;
          }
      }
      else if(!clmDECL_IsSameVectorType(leftDecl, rightDecl)) {
         gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                         LeftOperand->base.lineNo,
                         LeftOperand->base.stringNo,
                         clvREPORT_ERROR,
                         "conversion between different vector types not allowed"));
         return gcvSTATUS_INVALID_ARGUMENT;
      }
  }

  return gcvSTATUS_OK;
}

static gceSTATUS
_CheckSelectionExpr(
IN cloCOMPILER Compiler,
IN cloIR_EXPR CondExpr,
IN cloIR_EXPR TrueOperand,
IN cloIR_EXPR FalseOperand
)
{
    gcmASSERT(CondExpr);
    gcmASSERT(CondExpr->decl.dataType);
    gcmASSERT(TrueOperand);
    gcmASSERT(TrueOperand->decl.dataType);
    gcmASSERT(FalseOperand);
    gcmASSERT(FalseOperand->decl.dataType);

    /* Check the operands */
    if (!clmDECL_IsIntOrIVec(&CondExpr->decl)) {
        gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                        CondExpr->base.lineNo,
                        CondExpr->base.stringNo,
                        clvREPORT_ERROR,
                        "require a scalar or vector int expression"));
        return gcvSTATUS_INVALID_ARGUMENT;
    }

    return _CheckNonMatrixArithmeticOperands(Compiler, TrueOperand, FalseOperand);
}

cloIR_EXPR
clParseSelectionExpr(
    IN cloCOMPILER Compiler,
    IN cloIR_EXPR CondExpr,
    IN cloIR_EXPR TrueOperand,
    IN cloIR_EXPR FalseOperand
    )
{
    gceSTATUS    status;
    cloIR_SELECTION    selection;
    cloIR_CONSTANT    condConstant;
    gctBOOL        condValue;
    clsDECL    decl;

    if (CondExpr == gcvNULL || TrueOperand == gcvNULL || FalseOperand == gcvNULL) return gcvNULL;

    if (clmDECL_IsPackedGenType(&CondExpr->decl) ||
        clmDECL_IsPackedGenType(&TrueOperand->decl) ||
        clmDECL_IsPackedGenType(&FalseOperand->decl)) {
        gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                        CondExpr->base.lineNo,
                                        CondExpr->base.stringNo,
                                        clvREPORT_ERROR,
                                        "_viv_gentype_packed operands not allowed in ternery operator '?:'"));
        return gcvNULL;
    }

    if (clmDECL_IsHalfType(&CondExpr->decl) ||
        clmDECL_IsHalfType(&TrueOperand->decl) ||
        clmDECL_IsHalfType(&FalseOperand->decl)) {
        if(!cloCOMPILER_ExtensionEnabled(Compiler, clvEXTENSION_CL_KHR_FP16)) {
            gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                            CondExpr->base.lineNo,
                                            CondExpr->base.stringNo,
                                            clvREPORT_ERROR,
                                            "operands of type half not allowed in ternery operator '?:'"));
            return gcvNULL;
        }
    }

    /* Check error */
    status = _CheckSelectionExpr(Compiler,
                     CondExpr,
                     TrueOperand,
                     FalseOperand);
    if (gcmIS_ERROR(status)) return gcvNULL;

    /* Constant calculation */
    if (cloIR_OBJECT_GetType(&CondExpr->base) == clvIR_CONSTANT) {
       condConstant = (cloIR_CONSTANT)CondExpr;
       if(condConstant->valueCount == 1) {
        gcmASSERT(condConstant->values);

        condValue = condConstant->values[0].boolValue;

        gcmVERIFY_OK(cloIR_OBJECT_Destroy(Compiler, &CondExpr->base));

        if (condValue)
        {
            gcmVERIFY_OK(cloIR_OBJECT_Destroy(Compiler, &FalseOperand->base));

            return TrueOperand;
        }
        else
        {
            gcmVERIFY_OK(cloIR_OBJECT_Destroy(Compiler, &TrueOperand->base));

            return FalseOperand;
        }
       }
    }

    /* Create the selection */

    if(clmDECL_IsScalar(&TrueOperand->decl) &&
       clmDECL_IsScalar(&FalseOperand->decl) &&
       clmDECL_IsVectorType(&CondExpr->decl)) {
       decl = TrueOperand->decl;
       status = cloIR_CreateVectorType(Compiler,
                                       TrueOperand->decl.dataType,
                                       clmDATA_TYPE_vectorSize_NOCHECK_GET(CondExpr->decl.dataType),
                                       &decl.dataType);
       if(gcmIS_ERROR(status)) return gcvNULL;
    }
    else {
       if(clmDECL_IsPointerType(&TrueOperand->decl)) { /* right operand a pointer */
           status = cloCOMPILER_CloneDecl(Compiler,
                                          clvQUALIFIER_CONST,
                                          TrueOperand->decl.dataType->addrSpaceQualifier,
                                          &TrueOperand->decl,
                                          &decl);
       }
       else if(clmDECL_IsPointerType(&FalseOperand->decl)) { /* left operand a pointer */
           status = cloCOMPILER_CloneDecl(Compiler,
                                          clvQUALIFIER_CONST,
                                          FalseOperand->decl.dataType->addrSpaceQualifier,
                                          &FalseOperand->decl,
                                          &decl);
       }
       else {
           status = cloIR_GetArithmeticExprDecl(Compiler,
                                                gcvFALSE,
                                                TrueOperand,
                                                FalseOperand,
                                                &decl);
       }
       if (gcmIS_ERROR(status)) return gcvNULL;
    }

    status = cloIR_SELECTION_Construct(Compiler,
                       CondExpr->base.lineNo,
                       CondExpr->base.stringNo,
                       &decl,
                       CondExpr,
                       &TrueOperand->base,
                       &FalseOperand->base,
                       &selection);
    if (gcmIS_ERROR(status)) return gcvNULL;

    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                clvDUMP_PARSER,
                "<SELECTION_EXPR line=\"%d\" string=\"%d\" condExpr=\"0x%x\""
                " TrueOperand=\"0x%x\" FalseOperand=\"0x%x\" />",
                CondExpr->base.lineNo,
                CondExpr->base.stringNo,
                CondExpr,
                TrueOperand,
                FalseOperand));
    return &selection->exprBase;
}

static gctBOOL
_IsOperandTypeForBuiltinFuncCall(
    IN cloCOMPILER Compiler,
    IN cloIR_EXPR LeftOperand,
    IN cloIR_EXPR RightOperand
    )
{
    if(clmDECL_IsPackedType(&LeftOperand->decl) &&
       clmDECL_IsVectorType(&RightOperand->decl) &&
       (cloIR_OBJECT_GetType(&RightOperand->base) == clvIR_POLYNARY_EXPR) &&
       (clmDATA_TYPE_vectorSize_GET(LeftOperand->decl.dataType) ==
        clmDATA_TYPE_vectorSize_GET(RightOperand->decl.dataType))) {
        cloIR_POLYNARY_EXPR polynaryExpr;

        switch(LeftOperand->decl.dataType->elementType) {
        case clvTYPE_CHAR_PACKED:
            if(RightOperand->decl.dataType->elementType != clvTYPE_CHAR)
                return gcvFALSE;
            break;

        case clvTYPE_UCHAR_PACKED:
            if(RightOperand->decl.dataType->elementType != clvTYPE_UCHAR)
                return gcvFALSE;
            break;

        case clvTYPE_SHORT_PACKED:
            if(RightOperand->decl.dataType->elementType != clvTYPE_SHORT)
                return gcvFALSE;
            break;

        case clvTYPE_USHORT_PACKED:
            if(RightOperand->decl.dataType->elementType != clvTYPE_USHORT)
                return gcvFALSE;
            break;

        case clvTYPE_HALF_PACKED:
            if(RightOperand->decl.dataType->elementType != clvTYPE_HALF)
                return gcvFALSE;
            break;

        default:
            return gcvFALSE;
        }

        polynaryExpr = (cloIR_POLYNARY_EXPR) &RightOperand->base;
        if(polynaryExpr->type == clvPOLYNARY_FUNC_CALL &&
            polynaryExpr->funcName->isBuiltin &&
            gcmIS_SUCCESS(gcoOS_StrNCmp(polynaryExpr->funcSymbol, "vload", 5))) {
            return gcvTRUE;
        }
    }
    return gcvFALSE;
}

static gceSTATUS
_CheckAssignmentExpr(
    IN cloCOMPILER Compiler,
    IN cloIR_EXPR LeftOperand,
    IN cloIR_EXPR RightOperand
    )
{
    gceSTATUS status;

    gcmASSERT(LeftOperand->decl.dataType);
    gcmASSERT(RightOperand->decl.dataType);

    /* Check the left operand */
    status = _CheckLValueExpr(Compiler, LeftOperand, "left operand");

    if (gcmIS_ERROR(status)) return status;

    if(clmDECL_IsPointerType(&LeftOperand->decl)  &&
       cloIR_OBJECT_GetType(&RightOperand->base) == clvIR_CONSTANT &&
       clmDECL_IsScalar(&RightOperand->decl) &&
       clmDECL_IsIntegerType(&RightOperand->decl))
    {
        cloIR_CONSTANT constant;

        constant = (cloIR_CONSTANT)&RightOperand->base;
        if(constant->values[0].intValue == 0) return gcvTRUE;
    }

    /* Check the operands */
    if (!clsDECL_IsAssignableAndComparable(&LeftOperand->decl))
    {
        gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                        LeftOperand->base.lineNo,
                        LeftOperand->base.stringNo,
                        clvREPORT_ERROR,
                        "require any typed expression except arrays, structures"
                        " containing arrays, sampler types, and structures"
                        " containing sampler types"));
        return gcvSTATUS_INVALID_ARGUMENT;
    }

    if (!clsDECL_IsAssignableTo(&LeftOperand->decl, &RightOperand->decl))
    {
        if(cloCOMPILER_ExtensionEnabled(Compiler, clvEXTENSION_VIV_VX) &&
           _IsOperandTypeForBuiltinFuncCall(Compiler,
                                            LeftOperand,
                                            RightOperand)) {
            RightOperand->decl.dataType = LeftOperand->decl.dataType;
            return gcvSTATUS_OK;
        }
        gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                        RightOperand->base.lineNo,
                        RightOperand->base.stringNo,
                        clvREPORT_ERROR,
                        "require a matching typed expression"));
        return gcvSTATUS_INVALID_ARGUMENT;
    }

    if((LeftOperand->decl.dataType->type == T_HALF ||
        RightOperand->decl.dataType->type == T_HALF) &&
       !clmDECL_IsPointerType(&LeftOperand->decl) &&
       !cloCOMPILER_ExtensionEnabled(Compiler, clvEXTENSION_VIV_VX | clvEXTENSION_CL_KHR_FP16) &&
       !clsDECL_IsEqual(&LeftOperand->decl, &RightOperand->decl)) {
        gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                        LeftOperand->base.lineNo,
                                        LeftOperand->base.stringNo,
                                        clvREPORT_ERROR,
                                        "implicit conversion of half type not allowed"));
        return gcvSTATUS_INVALID_ARGUMENT;
    }
    return gcvSTATUS_OK;
}

gceSTATUS
clParseCheckReturnExpr(
    IN cloCOMPILER Compiler,
    IN clsDECL *RtnDecl,
    IN cloIR_EXPR RtnExpr
    )
{
    gcmASSERT(RtnDecl);
    gcmASSERT(RtnExpr->decl.dataType);

    if(clmDECL_IsPointerType(RtnDecl)  &&
       cloIR_OBJECT_GetType(&RtnExpr->base) == clvIR_CONSTANT &&
       clmDECL_IsScalar(&RtnExpr->decl) &&
       clmDECL_IsIntegerType(&RtnExpr->decl))
    {
        cloIR_CONSTANT constant;

        constant = (cloIR_CONSTANT)&RtnExpr->base;
        if(constant->values[0].intValue == 0) return gcvTRUE;
    }

    if (!clsDECL_IsAssignableTo(RtnDecl, &RtnExpr->decl))
    {
        gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                        RtnExpr->base.lineNo,
                        RtnExpr->base.stringNo,
                        clvREPORT_ERROR,
                        "require the same typed return expression"));
        return gcvSTATUS_INVALID_ARGUMENT;
    }

    return gcvSTATUS_OK;
}

static gceSTATUS
_CheckInitializationExpr(
IN cloCOMPILER Compiler,
IN cloIR_EXPR LeftOperand,
IN cloIR_EXPR RightOperand
)
{
  gcmASSERT(LeftOperand->decl.dataType);
  gcmASSERT(RightOperand->decl.dataType);

  if(!clmDECL_IsPointerType(&LeftOperand->decl)) { /* left operand not a pointer */
    switch (LeftOperand->decl.dataType->accessQualifier) {
    case clvQUALIFIER_UNIFORM:
    case clvQUALIFIER_ATTRIBUTE:
    case clvQUALIFIER_READ_ONLY:
       gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                       LeftOperand->base.lineNo,
                                       LeftOperand->base.stringNo,
                                       clvREPORT_ERROR,
                                       "left operand require an l-value expression"));
       return gcvSTATUS_INVALID_ARGUMENT;
    }
  }

/* Check the operands */
  if (!clsDECL_IsInitializable(&LeftOperand->decl)) {
     gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                     LeftOperand->base.lineNo,
                                     LeftOperand->base.stringNo,
                                     clvREPORT_ERROR,
                                     "left operand require any typed expression except sampler types"));
     return gcvSTATUS_INVALID_ARGUMENT;
  }

  if (!clsDECL_IsInitializableTo(&LeftOperand->decl, &RightOperand->decl)) {
     gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                     RightOperand->base.lineNo,
                                     RightOperand->base.stringNo,
                                     clvREPORT_ERROR,
                                     "require a matching typed expression"));
     return gcvSTATUS_INVALID_ARGUMENT;
  }

  return gcvSTATUS_OK;
}

static gceSTATUS
_CheckArithmeticAssignmentExpr(
IN cloCOMPILER Compiler,
IN clsLexToken *Operator,
IN cloIR_EXPR LeftOperand,
IN cloIR_EXPR RightOperand
)
{
  gceSTATUS status;
  clsDECL *leftDecl;
  clsDECL *rightDecl;
  gctBOOL isMul;

  gcmASSERT(LeftOperand);
  gcmASSERT(RightOperand);

/* Check the left operand */
  status = _CheckLValueExpr(Compiler, LeftOperand, "left operand");
  if (gcmIS_ERROR(status)) return status;

  leftDecl = &LeftOperand->decl;
  rightDecl = &RightOperand->decl;

  gcmASSERT(leftDecl->dataType);
  gcmASSERT(rightDecl->dataType);

/* Check the operands */
  if(!clmDECL_IsArithmeticType(leftDecl)) {
    gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                    LeftOperand->base.lineNo,
                                    LeftOperand->base.stringNo,
                                    clvREPORT_ERROR,
                                    "require an integer or floating-point typed expression"));
    return gcvSTATUS_INVALID_ARGUMENT;
  }
  else if(clmDECL_IsPointerType(leftDecl)) { /* left operand a pointer */
    if(Operator->u.operator != T_ADD_ASSIGN && Operator->u.operator != T_SUB_ASSIGN) {
       gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                       Operator->lineNo,
                                       Operator->stringNo,
                                       clvREPORT_ERROR,
                                       "operator has to be either '+=' or '-=' for pointer arithmetic"));
       return gcvSTATUS_INVALID_ARGUMENT;
    }
    if (!clmDECL_IsInt(rightDecl)) {
    gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                    RightOperand->base.lineNo,
                    RightOperand->base.stringNo,
                    clvREPORT_ERROR,
                    "require a scalar integer expression"));
    return gcvSTATUS_INVALID_ARGUMENT;
    }
  }

  if(!clmDECL_IsArithmeticType(rightDecl)) {
    gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                    RightOperand->base.lineNo,
                                    RightOperand->base.stringNo,
                                    clvREPORT_ERROR,
                                    "require an integer or floating-point typed expression"));
    return gcvSTATUS_INVALID_ARGUMENT;
  }
  else if(clmDECL_IsPointerType(rightDecl)) { /* right operand a pointer */
     gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                     Operator->lineNo,
                                     Operator->stringNo,
                                     clvREPORT_ERROR,
                                     "illegal arithmetic assignment with a pointer operand"));
     return gcvSTATUS_INVALID_ARGUMENT;
  }

  isMul = Operator->u.operator == T_MUL_ASSIGN;
  if(!clmDECL_IsArithmeticType(leftDecl)) {
    gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                    LeftOperand->base.lineNo,
                                    LeftOperand->base.stringNo,
                                    clvREPORT_ERROR,
                                    "require an integer or floating-point typed expression"));
    return gcvSTATUS_INVALID_ARGUMENT;
  }

  if(!clmDECL_IsArithmeticType(rightDecl)) {
    gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                    RightOperand->base.lineNo,
                                    RightOperand->base.stringNo,
                                    clvREPORT_ERROR,
                                    "require an integer or floating-point typed expression"));
    return gcvSTATUS_INVALID_ARGUMENT;
  }

  if(clsDECL_IsEqual(leftDecl, rightDecl)) return gcvSTATUS_OK;
  if(clmDECL_IsScalar(leftDecl)) {
     if(clmDECL_IsVectorType(rightDecl) || clmDECL_IsMat(rightDecl)) {
         gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                         RightOperand->base.lineNo,
                         RightOperand->base.stringNo,
                         clvREPORT_ERROR,
                         "require a scalar arithmetic expression"));
         return gcvSTATUS_INVALID_ARGUMENT;
     }
  }
  else if(clmDECL_IsScalar(rightDecl)) {
      if(clAreElementTypeInRankOrder(Compiler,
                                     rightDecl->dataType->elementType,
                                     leftDecl->dataType->elementType,
                                     cloIR_OBJECT_GetType(&RightOperand->base) == clvIR_CONSTANT)) {
           gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                           RightOperand->base.lineNo,
                                           RightOperand->base.stringNo,
                                           clvREPORT_ERROR,
                                           "conversion from a scalar to a lower ranking vector "
                                           "or matrix type not allowed"));
           return gcvSTATUS_INVALID_ARGUMENT;
      }
  }
  else if(clmDECL_IsVectorType(leftDecl)) {
     if(clmDECL_IsVectorType(rightDecl)) { /* both are vector types */
       if(!clmDECL_IsSameVectorType(leftDecl, rightDecl)) {
         gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                         LeftOperand->base.lineNo,
                         LeftOperand->base.stringNo,
                         clvREPORT_ERROR,
                         "conversion between different vector types not allowed"));
         return gcvSTATUS_INVALID_ARGUMENT;
       }
     }
     else if (clmDECL_IsMat(rightDecl)) { /* vector and matrix */
            gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                            RightOperand->base.lineNo,
                            RightOperand->base.stringNo,
                            clvREPORT_ERROR,
                            "require a scalar arithmetic expression"));
            return gcvSTATUS_INVALID_ARGUMENT;
     }
  }
  else if(clmDECL_IsMat(leftDecl)) { /* Left operand must be a matrix */
     if(clmDECL_IsVectorType(rightDecl)) {
       if(rightDecl->dataType->elementType > leftDecl->dataType->elementType) {
         gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                         RightOperand->base.lineNo,
                         RightOperand->base.stringNo,
                         clvREPORT_ERROR,
                         "conversion from a vector element type to a lower ranking "
                                         "element type of a matrix not allowed"));
         return gcvSTATUS_INVALID_ARGUMENT;
       }
       if(!isMul) {
         gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                         RightOperand->base.lineNo,
                                         RightOperand->base.stringNo,
                                         clvREPORT_ERROR,
                                         "require a scalar or matrix %dx%d expression",
                                         clmDATA_TYPE_matrixRowCount_GET(leftDecl->dataType),
                                         clmDATA_TYPE_matrixColumnCount_GET(leftDecl->dataType)));
         return gcvSTATUS_INVALID_ARGUMENT;
       }
       else if(clmDATA_TYPE_matrixColumnCount_GET(leftDecl->dataType)
           != clmDATA_TYPE_vectorSize_NOCHECK_GET(rightDecl->dataType)) {
          gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                          RightOperand->base.lineNo,
                                          RightOperand->base.stringNo,
                                          clvREPORT_ERROR,
                                          "require a scalar or vector of size %d "
                                          "or matrix %dxm expression",
                                          clmDATA_TYPE_matrixColumnCount_GET(leftDecl->dataType),
                                          clmDATA_TYPE_matrixColumnCount_GET(leftDecl->dataType)));
          return gcvSTATUS_INVALID_ARGUMENT;
       }
     }
     else if (clmDECL_IsMat(rightDecl)) {
        if(!isMul) {
          if(!clmDECL_IsSameMatrixType(leftDecl, rightDecl)) {
             gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                             LeftOperand->base.lineNo,
                             LeftOperand->base.stringNo,
                             clvREPORT_ERROR,
                             "arithmetic operation on different matrix types not allowed"));
             return gcvSTATUS_INVALID_ARGUMENT;
          }
        }
        else if(clmDATA_TYPE_matrixColumnCount_GET(leftDecl->dataType)
           != clmDATA_TYPE_matrixRowCount_GET(rightDecl->dataType)) {
          gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                          RightOperand->base.lineNo,
                                          RightOperand->base.stringNo,
                                          clvREPORT_ERROR,
                                          "require a scalar or vector of size %d "
                                          "or matrix %dxm expression",
                                          clmDATA_TYPE_matrixColumnCount_GET(leftDecl->dataType),
                                          clmDATA_TYPE_matrixColumnCount_GET(leftDecl->dataType)));
          return gcvSTATUS_INVALID_ARGUMENT;
        }
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

  return gcvSTATUS_OK;
}

gceSTATUS
clMakeConstantVariableName(
IN cloCOMPILER Compiler,
cloIR_CONSTANT ConstantOperand
)
{
    gceSTATUS status = gcvSTATUS_OK;

    if(ConstantOperand->variable == gcvNULL) {
        clsNAME *constantVarName;
        clsNAME_SPACE *nameSpace = gcvNULL;
        clsDECL decl[1];
        cltPOOL_STRING symbolInPool;

        status = cloCOMPILER_PushUnnamedSpace(Compiler, &nameSpace);
        if (gcmIS_ERROR(status)) return status;

        gcmONERROR(cloCOMPILER_CloneDecl(Compiler,
                                         clvQUALIFIER_CONST,
                                         clvQUALIFIER_CONSTANT,
                                         &ConstantOperand->exprBase.decl,
                                         decl));

        gcmONERROR(cloCOMPILER_MakeConstantName(Compiler,
                                                "CONSTANT",
                                                &symbolInPool));

        gcmONERROR(cloCOMPILER_CreateName(Compiler,
                                          ConstantOperand->exprBase.base.lineNo,
                                          ConstantOperand->exprBase.base.stringNo,
                                          clvVARIABLE_NAME,
                                          decl,
                                          symbolInPool,
                                          gcvNULL,
                                          clvEXTENSION_NONE,
                                          &constantVarName));

        constantVarName->u.variableInfo.builtinSpecific.s.variableType = clvBUILTIN_NONE;
        constantVarName->u.variableInfo.u.constant = ConstantOperand;
        constantVarName->u.variableInfo.isUnnamedConstant = gcvTRUE;
        constantVarName->u.variableInfo.u.constant->variable = constantVarName;

OnError:
        if(nameSpace) cloCOMPILER_PopCurrentNameSpace(Compiler, &nameSpace);
    }
    return status;
}

static gceSTATUS
_MakeConstantVariableExpr(
IN cloCOMPILER Compiler,
cloIR_CONSTANT ConstantOperand,
cloIR_EXPR *ConstVariableExpr
)
{
    gceSTATUS status;
    cloIR_VARIABLE constantVariable;
    clsNAME *constantVarName;
    clsNAME_SPACE *nameSpace = gcvNULL;

    if(ConstantOperand->variable == gcvNULL) {
        clsDECL decl[1];
        cltPOOL_STRING symbolInPool;

        status = cloCOMPILER_PushUnnamedSpace(Compiler, &nameSpace);
        if (gcmIS_ERROR(status)) return status;

        gcmONERROR(cloCOMPILER_CloneDecl(Compiler,
                                         clvQUALIFIER_CONST,
                                         clvQUALIFIER_CONSTANT,
                                         &ConstantOperand->exprBase.decl,
                                         decl));

        gcmONERROR(cloCOMPILER_MakeConstantName(Compiler,
                                                "CONSTANT",
                                                &symbolInPool));

        gcmONERROR(cloCOMPILER_CreateName(Compiler,
                                          ConstantOperand->exprBase.base.lineNo,
                                          ConstantOperand->exprBase.base.stringNo,
                                          clvVARIABLE_NAME,
                                          decl,
                                          symbolInPool,
                                          gcvNULL,
                                          clvEXTENSION_NONE,
                                          &constantVarName));

        constantVarName->u.variableInfo.u.constant = ConstantOperand;
        ConstantOperand->variable = constantVarName;
    }
    else {
        constantVarName = ConstantOperand->variable;
    }

    gcmONERROR(cloIR_VARIABLE_Construct(Compiler,
                                        constantVarName->lineNo,
                                        constantVarName->stringNo,
                                        constantVarName,
                                        &constantVariable));
    gcmONERROR(clParseMakeArrayPointerExpr(Compiler,
                                           &constantVariable->exprBase,
                                           ConstVariableExpr));
OnError:
    if(nameSpace) cloCOMPILER_PopCurrentNameSpace(Compiler, &nameSpace);
    return status;
}

static gctBOOL
_IsComponentSelectionEqual(
clsCOMPONENT_SELECTION *ComponentSelection1,
clsCOMPONENT_SELECTION *ComponentSelection2
)
{
   if(ComponentSelection1->components == ComponentSelection2->components) {
      gctUINT8 i;

      for(i = 0; i < ComponentSelection1->components; i++) {
         if(ComponentSelection1->selection[i] !=
            ComponentSelection2->selection[i]) return gcvFALSE;
      }
      return gcvTRUE;
   }
   else return gcvFALSE;
}

static gctBOOL
_IsLeftAndRightOperandIdentical(
IN cloCOMPILER Compiler,
IN cloIR_EXPR LeftOperand,
IN cloIR_EXPR RightOperand
)
{
   if(cloIR_OBJECT_GetType(&LeftOperand->base) == cloIR_OBJECT_GetType(&RightOperand->base)) {
       cloIR_BINARY_EXPR lBinaryExpr, rBinaryExpr;
       cloIR_UNARY_EXPR lUnaryExpr, rUnaryExpr;
       cloIR_VARIABLE lVariable, rVariable;
       cloIR_CONSTANT lConstant, rConstant;

       switch(cloIR_OBJECT_GetType(&LeftOperand->base)) {
       case clvIR_BINARY_EXPR:
          lBinaryExpr = (cloIR_BINARY_EXPR) (&LeftOperand->base);
          rBinaryExpr = (cloIR_BINARY_EXPR) (&RightOperand->base);

          if(lBinaryExpr->type != rBinaryExpr->type) break;
          switch(lBinaryExpr->type) {
          case clvBINARY_MUL_ASSIGN:
          case clvBINARY_DIV_ASSIGN:
          case clvBINARY_ADD_ASSIGN:
          case clvBINARY_SUB_ASSIGN:
          case clvBINARY_LEFT_ASSIGN:
          case clvBINARY_RIGHT_ASSIGN:
          case clvBINARY_AND_ASSIGN:
          case clvBINARY_XOR_ASSIGN:
          case clvBINARY_OR_ASSIGN:
          case clvBINARY_MOD_ASSIGN:
          case clvBINARY_ASSIGN:
             break;

          default:
             if(_IsLeftAndRightOperandIdentical(Compiler,
                                                lBinaryExpr->leftOperand,
                                                rBinaryExpr->leftOperand)) {
                 return _IsLeftAndRightOperandIdentical(Compiler,
                                                     lBinaryExpr->rightOperand,
                                                     rBinaryExpr->rightOperand);
             }
             break;
          }
          break;

       case clvIR_UNARY_EXPR:
          lUnaryExpr = (cloIR_UNARY_EXPR) (&LeftOperand->base);
          rUnaryExpr = (cloIR_UNARY_EXPR) (&RightOperand->base);
          if(lUnaryExpr->type != rUnaryExpr->type) break;
          switch(lUnaryExpr->type) {
          case clvUNARY_POST_INC:
          case clvUNARY_POST_DEC:
          case clvUNARY_PRE_INC:
          case clvUNARY_PRE_DEC:
             break;

          case clvUNARY_FIELD_SELECTION:
             if(lUnaryExpr->u.fieldName == rUnaryExpr->u.fieldName) {
                return _IsLeftAndRightOperandIdentical(Compiler,
                                                       lUnaryExpr->operand,
                                                       rUnaryExpr->operand);
             }
             break;

          case clvUNARY_COMPONENT_SELECTION:
             if(_IsComponentSelectionEqual(&lUnaryExpr->u.componentSelection,
                                           &rUnaryExpr->u.componentSelection)) {
                return _IsLeftAndRightOperandIdentical(Compiler,
                                                       lUnaryExpr->operand,
                                                       rUnaryExpr->operand);
             }
             break;

          default:
             break;
          }
          break;

       case clvIR_VARIABLE:
          lVariable = (cloIR_VARIABLE) (&LeftOperand->base);
          rVariable = (cloIR_VARIABLE) (&RightOperand->base);
          return lVariable->name == rVariable->name;

       case clvIR_CONSTANT:
          lConstant = (cloIR_CONSTANT) (&LeftOperand->base);
          rConstant = (cloIR_CONSTANT) (&RightOperand->base);
      if(clsDECL_IsEqual(&lConstant->exprBase.decl,
                 &rConstant->exprBase.decl) &&
             lConstant->valueCount == rConstant->valueCount) {
             gctUINT i;

         for (i = 0; i < lConstant->valueCount; i++) {
        if(lConstant->values[i].intValue != rConstant->values[i].intValue) {
                   return gcvFALSE;
        }
             }
             return gcvTRUE;
      }
          break;

       default:
          break;
       }
   }
   return gcvFALSE;
}

cloIR_EXPR
clParseAssignmentExpr(
IN cloCOMPILER Compiler,
IN cloIR_EXPR LeftOperand,
IN clsLexToken * Operator,
IN cloIR_EXPR RightOperand
)
{
    gceSTATUS        status;
    cleBINARY_EXPR_TYPE    exprType = (cleBINARY_EXPR_TYPE) 0;
    cloIR_EXPR    resExpr;
    cloIR_EXPR    leftOperand;

    gcmASSERT(Operator);

    if (LeftOperand == gcvNULL || RightOperand == gcvNULL) return gcvNULL;

    leftOperand = LeftOperand;
    if(cloIR_OBJECT_GetType(&leftOperand->base) == clvIR_UNARY_EXPR)
    {
       cloIR_UNARY_EXPR unaryExpr = (cloIR_UNARY_EXPR) &leftOperand->base;
       if(unaryExpr->type == clvUNARY_NULL) {
          leftOperand = unaryExpr->operand;
          gcmASSERT(leftOperand);
       }
    }

    switch (Operator->u.operator) {
    case T_LEFT_ASSIGN:
    case T_RIGHT_ASSIGN:
    case T_AND_ASSIGN:
    case T_XOR_ASSIGN:
    case T_OR_ASSIGN:
        switch (Operator->u.operator) {
        case T_LEFT_ASSIGN:    exprType = clvBINARY_LEFT_ASSIGN;    break;
        case T_RIGHT_ASSIGN:    exprType = clvBINARY_RIGHT_ASSIGN;    break;
        case T_AND_ASSIGN:    exprType = clvBINARY_AND_ASSIGN;    break;
        case T_XOR_ASSIGN:    exprType = clvBINARY_XOR_ASSIGN;    break;
        case T_OR_ASSIGN:    exprType = clvBINARY_OR_ASSIGN;        break;
        }

        status = _CheckLogicalAssignmentExpr(Compiler,
                             Operator,
                             leftOperand,
                             RightOperand);
        if (gcmIS_ERROR(status)) return gcvNULL;
        status = clParseSetOperandDirty(Compiler,
                                        leftOperand,
                                        RightOperand);
        if (gcmIS_ERROR(status)) return gcvNULL;

        break;

    case '=':
        exprType = clvBINARY_ASSIGN;
        status = _CheckAssignmentExpr(Compiler,
                                      leftOperand,
                                      RightOperand);
        if (gcmIS_ERROR(status)) return gcvNULL;
        if (_IsLeftAndRightOperandIdentical(Compiler,
                                            leftOperand,
                                            RightOperand)) {
           return leftOperand;
        }

        /*Check if left operand is a pointer and right operand is constant array:
          if so, create a constant variable for the right operand */
        status = clParseSetOperandDirty(Compiler,
                                        leftOperand,
                                        RightOperand);
        if (gcmIS_ERROR(status)) return gcvNULL;

        if(clmDECL_IsPointerType(&leftOperand->decl)
               && cloIR_OBJECT_GetType(&RightOperand->base) == clvIR_CONSTANT) {
           if(((cloIR_CONSTANT)RightOperand)->valueCount > 1) {
               cloIR_EXPR constVariableExpr;
               status = _MakeConstantVariableExpr(Compiler,
                                                  (cloIR_CONSTANT)RightOperand,
                                                  &constVariableExpr);
               if (gcmIS_ERROR(status)) return gcvNULL;
               RightOperand = constVariableExpr;
           }
        }
        else if(clmDECL_IsUnderlyingStructOrUnion(&leftOperand->decl) &&
                (clGetOperandCountForRegAlloc(&leftOperand->decl) > _clmMaxOperandCountToUseMemory(&leftOperand->decl) ||
                 leftOperand->decl.dataType->u.fieldSpace->scopeName->u.typeInfo.hasUnionFields)) {
           gcmASSERT(clmDECL_IsUnderlyingStructOrUnion(&RightOperand->decl));
           status = clParseSetOperandAddressed(Compiler,
                                               leftOperand);
           if (gcmIS_ERROR(status)) return gcvNULL;
           status = clParseSetOperandAddressed(Compiler,
                                               RightOperand);
           if (gcmIS_ERROR(status)) return gcvNULL;
        }
        break;

    case T_MUL_ASSIGN:
    case T_DIV_ASSIGN:
    case T_ADD_ASSIGN:
    case T_SUB_ASSIGN:
    case T_MOD_ASSIGN:
        status = _CheckArithmeticAssignmentExpr(Compiler,
                            Operator,
                            leftOperand,
                            RightOperand);
        if (gcmIS_ERROR(status)) return gcvNULL;
        switch (Operator->u.operator) {
        case T_MUL_ASSIGN:
            exprType = clvBINARY_MUL_ASSIGN;
            break;

        case T_DIV_ASSIGN:
            if((clmDECL_IsFloatingType(&RightOperand->decl) ||
               clmDECL_IsFloatingType(&leftOperand->decl)) &&
               _clmExprIsConstantForEval(RightOperand)) {
                cloIR_CONSTANT constantOne;
                cloIR_CONSTANT resultConstant;
                cluCONSTANT_VALUE constantValue;

                (void)gcoOS_ZeroMemory((gctPOINTER)&constantValue, sizeof(cluCONSTANT_VALUE));
                constantValue.floatValue = 1.0;
                constantOne = _ParseCreateConstant(Compiler,
                                   RightOperand->base.lineNo,
                                   RightOperand->base.stringNo,
                                   T_FLOAT,
                                   &constantValue);
                if(!constantOne) return gcvNULL;
                status = cloIR_BINARY_EXPR_Evaluate(Compiler,
                                    clvBINARY_DIV,
                                    constantOne,
                                    (cloIR_CONSTANT)(&RightOperand->base),
                                    gcvNULL,
                                    &resultConstant);
                if (gcmIS_ERROR(status)) return gcvNULL;
                RightOperand = &resultConstant->exprBase;
                exprType = clvBINARY_MUL_ASSIGN;
            }
            else exprType = clvBINARY_DIV_ASSIGN;
            break;

        case T_ADD_ASSIGN:
            exprType = clvBINARY_ADD_ASSIGN;
            break;
        case T_SUB_ASSIGN:
            exprType = clvBINARY_SUB_ASSIGN;
            break;
        case T_MOD_ASSIGN:
            exprType = clvBINARY_MOD_ASSIGN;
            break;
        }
        break;

    default:
        gcmASSERT(0);
        return gcvNULL;
    }

    /* Create binary expression */
    status = cloIR_BINARY_EXPR_Construct(Compiler,
                         leftOperand->base.lineNo,
                         leftOperand->base.stringNo,
                         exprType,
                         leftOperand,
                         RightOperand,
                         &resExpr);
    if (gcmIS_ERROR(status)) {
        return gcvNULL;
    }

    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                      clvDUMP_PARSER,
                      "<BINARY_EXPR type=\"%s\" line=\"%d\" string=\"%d\" />",
                      _GetBinaryOperatorName(Operator->u.operator),
                      leftOperand->base.lineNo,
                      leftOperand->base.stringNo));
    return resExpr;
}

#undef TESTCL
/*
#define TESTCL
*/

#ifdef TESTCL
/************************ JUST SOME TEST ON OPENCL  TO BE REMOVED LATER*****/
#include <stdio.h>
static void
cltest(void)
{
    enum  hue {red, blue, green, yellow, brown};
    enum { one, two, three, seven, eight, nine} number;
    enum { four, five, six } number_big;
    enum not_yet_defined;
    const char a[] = "";
    char b[5] = "abcd";
    char c[] = "abcdefg";
    char d[5] = "dfgh";
    char chr;
    const char *ptr;
    struct yet_to_define;
    struct s {
        int i;
        int a[2];
        float f;
    } s1, s2;
    typedef int arrayeight[8];
    int size;
    arrayeight intarray = {1, 2, 3, 4, 5, 6, 7, 8};
    arrayeight *intarrayptr;
    int outValues[3][8] = { 17, 01, 11, 12, 1955, 11, 5, 1985, 113, 1, 24, 1984, 7, 23, 1979, 97,
                                100, 101, 102, 103, 104, 105, 106, 107 };

    int mylabel;
        int *intptr;
        union charIntOverlay {
          char c[4];
          int i;
        } s = { "123" };
        (ptr + 1) = (char *) intptr;
        b = (char *)intarray;
    &b[0] = ptr;
        &*b = ptr;
    intptr = (&outValues[0])[1];
        printf("Array value is %d\n", *intptr);

        printf("charIntOverlay %x\n", s.i);
        printf("charIntOverlay %d\n", s.i);
/*
        intptr = &outValues[0] + 1;
        printf("Pointer to Array value is %d\n", *(intptr + 3));
*/

        intarrayptr = &outValues[0] + 1;
        printf("outValues address %x\n", outValues[0]);
        printf("outValues[1] address %x\n", outValues[1]);
        printf("outValues[2] address %x\n", outValues[2]);
        printf("intarrayptr value %x\n", intarrayptr);


        printf("intarrayptr + 1 value %x\n", intarrayptr + 1);
        printf("intarrayptr + 2 value %x\n", intarrayptr + 2);

        printf("Pointer to Array value is %d\n", *(intarrayptr + 1));
        printf("Pointer to Array value in hex is %x\n", *(intarrayptr + 1));
        printf("Pointer to Array value is %d\n", (intarrayptr + 1)[1]);
        printf("Pointer to Array value is %d\n", (intarrayptr + 1)[0][0]);

        intptr = intarray;
/*
    intptr = &intarray;

        intptr = outValues;
    intarrayptr[0] = *intarray;
    *(intarrayptr[0]) = *intarray;
    intarrayptr = &intarray;
    intarrayptr = intarray;
    *(&outValues[1])  = *intarrayptr;
    *(outValues) = *intarrayptr;
    intarray[0] = *intarrayptr;
    *intptr = *intarrayptr;
*/

    size = sizeof ptr;
        intptr = intptr + 3;
        printf("intptr value is %d\n", *intptr);

    chr = *b;
    number_big = six;
        number = three;
/*
    *b += 1;
*/
        number_big = 10;
    number = number_big;
    size = a[0];
    s2.i = 0;
    s2.a[0] = 1;
    s2.a[1] = 1;
    s1 = s2;
    d[0] = 'o';

    size = sizeof(a);
    printf("The size of a is %d\n", size);
        if(size == 0) goto mylabel;
    size = sizeof(b);
    printf("The size of b is %d\n", size);
    size = sizeof(c);
    printf("The size of c is %d\n", size);
mylabel:
    ptr = b;
        ptr = ptr - 1;
        mylabel = 1;
        printf("string is %s\n", ptr);
    ptr = a;
/*
    *ptr = 'a';
*/
        printf("string is %s\n", ptr);
/*
      int *iptr, *kptr, **jptr;
      int a[4] = {-1, -2, -3, -4};
      int b[4] = {4, 3, 2, 1};
      int c[4] = {14, 13, 12, 11};
      int d[4] = {114, 113, 112, 111};

      int *e[4];
      int ix;

          iptr = *ix;
      iptr = a;
      kptr = b;
      jptr = e;
      jptr[0] = a;
      jptr[1] = b;
      jptr[2] = c;
      jptr[3] = d;

          *(&*iptr) = 1;
          jptr =  e + *(&*iptr);

          jptr = &(&*iptr);
          jptr = &(a + 2);
      jptr = &a[2];
      jptr = &e[2];
      &*iptr = a;
          *&jptr = e;
      **&jptr = a;

      (jptr + 0) = e;
      jptr++ = e;
      &*iptr = a;
      &*jptr = e;
      &**jptr = a;
      iptr = &ix + 5;
      printf("Result of &*iptr = a \n");
      for(ix = 0; ix < 4 ; ix++) {
        printf("iptr %d = %d\n", ix, iptr[ix]);
      }

      *&iptr = a;
      printf("Result of *&iptr = a \n");
      for(ix = 0; ix < 4 ; ix++) {
        printf("iptr %d = %d\n", ix, iptr[ix]);
      }
*/
}
#endif

static gceSTATUS
_ParseMergeAttr(
IN clsNAME *TypeName,
IN clsATTRIBUTE *Attr
)
{
   gcmASSERT(TypeName);
   gcmASSERT(Attr);

   if(TypeName->u.typeInfo.specifiedAttr) {
     if(TypeName->u.typeInfo.specifiedAttr & clvATTR_PACKED) {
        Attr->specifiedAttr |= clvATTR_PACKED;
        Attr->packed = TypeName->context.packed;
     }

     if(TypeName->u.typeInfo.specifiedAttr & clvATTR_ALIGNED) {
        Attr->specifiedAttr |= clvATTR_ALIGNED;
        if(TypeName->context.alignment > Attr->alignment) {
          Attr->alignment = TypeName->context.alignment;
        }
     }
   }
   return gcvSTATUS_OK;
}

static gceSTATUS
_MergeTypeQualifiers(
IN cloCOMPILER Compiler,
IN clsDECL *FromDecl,
IN OUT clsDECL *Decl
)
{
    gceSTATUS status = gcvSTATUS_OK;
    clsDATA_TYPE dataType[1];

    *dataType = *Decl->dataType;

    /* storageQualifier */
    if(FromDecl->storageQualifier != clvSTORAGE_QUALIFIER_NONE) {
        if((Decl->storageQualifier & FromDecl->storageQualifier)) {
             gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                             cloCOMPILER_GetCurrentLineNo(Compiler),
                                             cloCOMPILER_GetCurrentStringNo(Compiler),
                                             clvREPORT_ERROR,
                                             "storage qualifiers \"%s\" multiply defined",
                                             clGetStorageQualifierName(Decl->storageQualifier & FromDecl->storageQualifier)));
             return gcvSTATUS_INVALID_ARGUMENT;
        }
        else if((Decl->storageQualifier & clvSTORAGE_QUALIFIER_STATIC) &&
                (FromDecl->storageQualifier & clvSTORAGE_QUALIFIER_EXTERN)) {
             gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                             cloCOMPILER_GetCurrentLineNo(Compiler),
                                             cloCOMPILER_GetCurrentStringNo(Compiler),
                                             clvREPORT_ERROR,
                                             "storage qualifier \"%s\" "
                                             "defined prior to this qualifier \"%s\"",
                                             clGetStorageQualifierName(clvSTORAGE_QUALIFIER_STATIC),
                                             clGetStorageQualifierName(clvSTORAGE_QUALIFIER_EXTERN)));
             return gcvSTATUS_INVALID_ARGUMENT;
        }
        else if((Decl->storageQualifier & clvSTORAGE_QUALIFIER_EXTERN) &&
             (FromDecl->storageQualifier & clvSTORAGE_QUALIFIER_STATIC)) {
             gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                             cloCOMPILER_GetCurrentLineNo(Compiler),
                                             cloCOMPILER_GetCurrentStringNo(Compiler),
                                             clvREPORT_ERROR,
                                             "storage qualifier \"%s\" "
                                             "defined prior to this qualifier \"%s\"",
                                             clGetStorageQualifierName(clvSTORAGE_QUALIFIER_EXTERN),
                                             clGetStorageQualifierName(clvSTORAGE_QUALIFIER_STATIC)));
             return gcvSTATUS_INVALID_ARGUMENT;
        }
        else Decl->storageQualifier |= FromDecl->storageQualifier;
    }

    if(FromDecl->dataType->accessQualifier != clvQUALIFIER_NONE) {
        if(dataType->accessQualifier != clvQUALIFIER_NONE) {
            gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                            cloCOMPILER_GetCurrentLineNo(Compiler),
                                            cloCOMPILER_GetCurrentStringNo(Compiler),
                                            clvREPORT_ERROR,
                                            "access qualifier \"%s\" "
                                            "defined prior to this qualifier \"%s\"",
                                            clGetQualifierName(FromDecl->dataType->accessQualifier),
                                            clGetQualifierName(dataType->accessQualifier)));
            return gcvSTATUS_INVALID_ARGUMENT;
        }
        else dataType->accessQualifier = FromDecl->dataType->accessQualifier;
    }
    if(FromDecl->dataType->addrSpaceQualifier != clvQUALIFIER_NONE) {
        if(dataType->addrSpaceQualifier != clvQUALIFIER_NONE) {
            gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                            cloCOMPILER_GetCurrentLineNo(Compiler),
                                            cloCOMPILER_GetCurrentStringNo(Compiler),
                                            clvREPORT_ERROR,
                                            "address space qualifier \"%s\" "
                                            "defined prior to this qualifier \"%s\"",
                                            clGetQualifierName(FromDecl->dataType->addrSpaceQualifier),
                                            clGetQualifierName(dataType->addrSpaceQualifier)));
            return gcvSTATUS_INVALID_ARGUMENT;
        }
        else dataType->addrSpaceQualifier = FromDecl->dataType->addrSpaceQualifier;
    }

    if(Decl->dataType->accessQualifier != dataType->accessQualifier ||
       Decl->dataType->addrSpaceQualifier != dataType->addrSpaceQualifier) {
       status = cloCOMPILER_CloneDataType(Compiler,
                                          dataType->accessQualifier,
                                          dataType->addrSpaceQualifier,
                                          Decl->dataType,
                                          &Decl->dataType);
       if(gcmIS_ERROR(status)) return gcvSTATUS_INVALID_ARGUMENT;
    }

    return gcvSTATUS_OK;
}

static gceSTATUS
_ParseFlattenType(
IN cloCOMPILER Compiler,
IN clsDECL *TypeDecl,
OUT clsDECL *Decl
)
{
    gceSTATUS status;
    clsNAME *typeName;
    clsDECL decl[1];

    typeName = TypeDecl->dataType->u.typeDef;
    gcmASSERT(typeName);

    *decl = typeName->decl;
    status = _MergeTypeQualifiers(Compiler,
                                  TypeDecl,
                                  decl);
    if(gcmIS_ERROR(status)) return status;

    if(decl->dataType->type == T_TYPE_NAME) {
        status = _ParseFlattenType(Compiler,
                                   decl,
                                   decl);
        if (gcmIS_ERROR(status)) return status;
    }
    status = clMergePtrDscrToDecl(Compiler,
                                  TypeDecl->ptrDscr,
                                  decl,
                                  TypeDecl->array.numDim == 0);
    if (gcmIS_ERROR(status)) return status;

    if (TypeDecl->array.numDim != 0) {
        /* may need to handle array of pointers to array */
        status = _ParseMergeArrayDecl(Compiler,
                                      decl,
                                      &TypeDecl->array,
                                      decl);
        if (gcmIS_ERROR(status)) return status;
    }

    return cloCOMPILER_CloneDecl(Compiler,
                                 decl->dataType->accessQualifier,
                                 decl->dataType->addrSpaceQualifier,
                                 decl,
                                 Decl);
}

static gceSTATUS
_ConvDataTypeToPacked(
IN cloCOMPILER Compiler,
IN clsNAME *Var
)
{
    gceSTATUS status = gcvSTATUS_OK;
    clsBUILTIN_DATATYPE_INFO *typeInfo;

    if(Var->decl.dataType == gcvNULL ||
       clmIsElementTypePacked(Var->decl.dataType->elementType)) return status;
    typeInfo = clGetBuiltinDataTypeInfo(Var->decl.dataType->type);

    if(typeInfo &&
       typeInfo->dualType != typeInfo->type) {
        status = cloCOMPILER_CreateDataType(Compiler,
                                            typeInfo->dualType,
                                            Var->decl.dataType->u.generic,
                                            Var->decl.dataType->accessQualifier,
                                            Var->decl.dataType->addrSpaceQualifier,
                                            &Var->decl.dataType);
    }

    return status;
}


static gceSTATUS
_ParseFillVariableAttr(
IN cloCOMPILER Compiler,
IN gctINT LineNo,
IN gctINT StringNo,
IN clsDECL *Decl,
IN clsNAME *Var,
IN clsATTRIBUTE *Attr
)
{
   clsATTRIBUTE attr[1];
   clsATTRIBUTE *attrPtr = Attr;

   gcmASSERT(Var);

   if(Decl && Decl->dataType->type == T_TYPE_NAME) {
      gceSTATUS status;

      if(!attrPtr) {
         (void) gcoOS_ZeroMemory((gctPOINTER)attr, sizeof(clsATTRIBUTE));
         attrPtr = attr;
      }

      status =  _ParseMergeAttr(Decl->dataType->u.typeDef, attrPtr);
      if(gcmIS_ERROR(status)) return status;
   }

   if(cloCOMPILER_IsBasicTypePacked(Compiler) &&
      Var->decl.dataType &&
      clmDATA_TYPE_IsVector(Var->decl.dataType)) {
       if(!attrPtr) {
           (void) gcoOS_ZeroMemory((gctPOINTER)attr, sizeof(clsATTRIBUTE));
           attrPtr = attr;
       }
       attrPtr->specifiedAttr |= clvATTR_PACKED;
       attrPtr->packed = gcvTRUE;
   }
   if(!attrPtr) return gcvSTATUS_OK;

   if((attrPtr->specifiedAttr & clvATTR_PACKED) ||
      (Var->decl.dataType &&
       clmDATA_TYPE_IsVector(Var->decl.dataType) &&
       cloCOMPILER_IsBasicTypePacked(Compiler)))  {
     Var->context.packed = attrPtr->packed;
     if(cloCOMPILER_ExtensionEnabled(Compiler, clvEXTENSION_VIV_VX)) {
         gceSTATUS status;

         status = _ConvDataTypeToPacked(Compiler, Var);
         if(gcmIS_ERROR(status)) return status;
     }
   }
   if(attrPtr->specifiedAttr & clvATTR_ALIGNED)  {
     Var->context.alignment = attrPtr->alignment;
   }
   if(attrPtr->specifiedAttr & clvATTR_ENDIAN) {
     if(!clmDECL_IsPointerType(&Var->decl)) {
         gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                         LineNo,
                                         StringNo,
                                         clvREPORT_ERROR,
                                         "the endian attribute cannot be used for variables that are not a pointer type"));
         return gcvSTATUS_INVALID_ARGUMENT;
     }
     else {
         Var->u.variableInfo.specifiedAttr |= clvATTR_ENDIAN;
         Var->u.variableInfo.hostEndian = attrPtr->hostEndian;
     }
   }
   if(Attr) {
     gcmVERIFY_OK(cloCOMPILER_Free(Compiler, Attr));
   }
   return gcvSTATUS_OK;
}

static gceSTATUS
_ParseCheckVariableNeedMemory(
cloCOMPILER Compiler,
clsNAME *VariableName
)
{
    gceSTATUS status = gcvSTATUS_OK;

/*klc*/
    if (clmDECL_IsUnderlyingStructOrUnion(&VariableName->decl) &&
        (clGetOperandCountForRegAlloc(&VariableName->decl) > _clmMaxOperandCountToUseMemory(&VariableName->decl))) {
       clsNAME_SPACE *nameSpace;

       nameSpace = VariableName->decl.dataType->u.fieldSpace;
       gcmASSERT(nameSpace->scopeName);

       if(nameSpace->scopeName) {
          gcmASSERT(nameSpace->scopeName->type == clvSTRUCT_NAME ||
                    nameSpace->scopeName->type == clvUNION_NAME);
          if(nameSpace->scopeName->u.typeInfo.needMemory) {
             status = clsNAME_SetVariableAddressed(Compiler,
                                                   VariableName);
             if (gcmIS_ERROR(status)) return status;
          }
       }
    }
    return status;
}

static clsDECL *
_HandleSpecialType(
IN cloCOMPILER Compiler,
IN clsDECL *Decl
)
{
    gceSTATUS status;
    clsARRAY array[1];
    clsDECL *declPtr;

    gcmASSERT(Decl);
    declPtr = Decl;
    switch(declPtr->dataType->type) {
    case T_IMAGE2D_PTR_T:
        status = clParseAddIndirectionOneLevel(Compiler, &declPtr->ptrDscr);
        if (gcmIS_ERROR(status)) return gcvNULL;
        break;

    case T_IMAGE2D_DYNAMIC_ARRAY_T:
        (void) gcoOS_ZeroMemory(array, sizeof(clsARRAY));
        array->numDim = 1;
        array->length[0] = cloCOMPILER_GetImageArrayMaxLevel(Compiler);
        status = cloCOMPILER_CreateArrayDecl(Compiler,
                                             declPtr->dataType,
                                             array,
                                             gcvNULL,
                                             declPtr);
        if (gcmIS_ERROR(status)) return gcvNULL;
        break;

    default:
        break;
    }

    return declPtr;
}

static gceSTATUS
_ParseVariableDecl(
IN cloCOMPILER Compiler,
IN clsDeclOrDeclList *DeclOrDeclListPtr,
IN clsLexToken * Identifier
)
{
    gceSTATUS  status;
    clsDECL decl[1];
    clsDECL *declPtr;
    clsNAME *derivedType = gcvNULL;
    gctBOOL notTypeDef;

    gcmASSERT(DeclOrDeclListPtr && DeclOrDeclListPtr->decl.dataType);
    gcmASSERT(Identifier);

#ifdef TESTCL
/****** KLC CALL TO TEST SOME OPENCL SYNTAX  ***/
    cltest();

/****/
#endif
    switch(DeclOrDeclListPtr->decl.dataType->type) {
    case T_TYPE_NAME:
        status = _ParseFlattenType(Compiler, &DeclOrDeclListPtr->decl, decl);
        if(gcmIS_ERROR(status)) return gcvSTATUS_INVALID_ARGUMENT;
        declPtr = decl;
        derivedType = DeclOrDeclListPtr->decl.dataType->u.typeDef;
        break;

    case T_ENUM:
        declPtr = &DeclOrDeclListPtr->decl;
        derivedType = DeclOrDeclListPtr->decl.dataType->u.enumerator;
        break;

    default:
        declPtr = _HandleSpecialType(Compiler,  &DeclOrDeclListPtr->decl);
        break;
    }

    notTypeDef = cloCOMPILER_GetParserState(Compiler) != clvPARSER_IN_TYPEDEF;

    if (DeclOrDeclListPtr->decl.dataType->accessQualifier == clvQUALIFIER_CONST &&
        (!cloCOMPILER_IsExternSymbolsAllowed(Compiler) ||
         !(DeclOrDeclListPtr->decl.storageQualifier & clvSTORAGE_QUALIFIER_EXTERN))) {
        if(notTypeDef && !Identifier->u.identifier.ptrDscr) {
            gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                            Identifier->lineNo,
                                            Identifier->stringNo,
                                            clvREPORT_ERROR,
                                            "require the initializer for the 'const' variable: '%s'",
                                            Identifier->u.identifier.name));
            return gcvSTATUS_INVALID_ARGUMENT;
        }

        if(DeclOrDeclListPtr->decl.dataType->addrSpaceQualifier != clvQUALIFIER_CONSTANT) {
            /* reset access qualifier to none for pointer variables */
            status = cloCOMPILER_CloneDecl(Compiler,
                                           clvQUALIFIER_NONE,
                                           declPtr->dataType->addrSpaceQualifier,
                                           declPtr,
                                           decl);
            if (gcmIS_ERROR(status)) return status;
            declPtr = decl;
        }
    }

    if (clmDATA_TYPE_IsSampler(declPtr->dataType)) {
        if(declPtr->dataType->accessQualifier != clvQUALIFIER_CONST) {
/*KLC force it to constant as conformance test did not declare it as constant*/
            status = cloCOMPILER_CloneDataType(Compiler,
                                               clvQUALIFIER_CONST,
                                               declPtr->dataType->addrSpaceQualifier,
                                               declPtr->dataType,
                                               &declPtr->dataType);
            if (gcmIS_ERROR(status)) return status;
        }
    }
    if(declPtr->dataType->type == T_IMAGE2D_DYNAMIC_ARRAY_T) {
        gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                        Identifier->lineNo,
                        Identifier->stringNo,
                        clvREPORT_ERROR,
                        "unrecognizable type '_viv_image2d_array_t' specified for variable '%s'",
                        Identifier->u.identifier.name));
        return gcvSTATUS_INVALID_ARGUMENT;
    }
    if(declPtr->dataType->type == T_GENTYPE_PACKED) {
        gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                        Identifier->lineNo,
                        Identifier->stringNo,
                        clvREPORT_ERROR,
                        "unrecognizable type '_viv_gentype_packed' specified for variable '%s'",
                        Identifier->u.identifier.name));
        return gcvSTATUS_INVALID_ARGUMENT;
    }
    if (clmDATA_TYPE_IsImage(declPtr->dataType)) {
        gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                        Identifier->lineNo,
                        Identifier->stringNo,
                        clvREPORT_ERROR,
                        "variable '%s' cannot have image type",
                        Identifier->u.identifier.name));
        return gcvSTATUS_INVALID_ARGUMENT;
    }
    if(declPtr->dataType->type == T_HALF) {
        if(!cloCOMPILER_ExtensionEnabled(Compiler, clvEXTENSION_VIV_VX | clvEXTENSION_CL_KHR_FP16)) {
            if((clmDECL_IsHalfType(declPtr) || clmDECL_IsArray(declPtr)) &&
                !Identifier->u.identifier.ptrDscr) {
                gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                                Identifier->lineNo,
                                                Identifier->stringNo,
                                                clvREPORT_ERROR,
                                                "variable '%s' cannot have half type",
                                                Identifier->u.identifier.name));
                return gcvSTATUS_INVALID_ARGUMENT;
            }
        }
    }

    if (notTypeDef &&
        clmDECL_IsStructOrUnion(declPtr) &&
        declPtr->dataType->u.fieldSpace->scopeName->isBuiltin &&
        declPtr->dataType->accessQualifier != clvQUALIFIER_UNIFORM) {
        gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                        Identifier->lineNo,
                        Identifier->stringNo,
                        clvREPORT_ERROR,
                        "variable '%s' cannot have struct '%s' type",
                        Identifier->u.identifier.name,
                        declPtr->dataType->u.fieldSpace->scopeName->symbol));
        return gcvSTATUS_INVALID_ARGUMENT;
    }

    status = cloCOMPILER_CreateName(Compiler,
                    Identifier->lineNo,
                    Identifier->stringNo,
                    clvVARIABLE_NAME,
                    declPtr,
                    Identifier->u.identifier.name,
                    Identifier->u.identifier.ptrDscr,
                    clvEXTENSION_NONE,
                    &DeclOrDeclListPtr->name);
    if (gcmIS_ERROR(status))    return status;

    DeclOrDeclListPtr->name->derivedType = derivedType;
    if(notTypeDef) {
        status = _ParseCheckVariableNeedMemory(Compiler,
                                      DeclOrDeclListPtr->name);
        if (gcmIS_ERROR(status)) return status;
    }

    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                      clvDUMP_PARSER,
                      "<VARIABLE_DECL line=\"%d\" string=\"%d\" name=\"%s\" />",
                      Identifier->lineNo,
                      Identifier->stringNo,
                      Identifier->u.identifier.name));
    return gcvSTATUS_OK;
}

static gceSTATUS
_ParseArrayVariableDecl(
IN cloCOMPILER Compiler,
IN clsDeclOrDeclList *DeclOrDeclListPtr,
IN clsLexToken * Identifier,
IN cloIR_EXPR ArrayLengthExpr
)
{
    gceSTATUS status;
    clsARRAY array[1];
    clsDECL arrayDecl[1];
    clsDECL decl[1];
    clsDECL *declPtr;
    clsNAME *derivedType = gcvNULL;

    gcmASSERT(DeclOrDeclListPtr && DeclOrDeclListPtr->decl.dataType);
    gcmASSERT(Identifier);

    switch (DeclOrDeclListPtr->decl.dataType->accessQualifier) {
    case clvQUALIFIER_CONST:
        if(!cloCOMPILER_InGlobalSpace(Compiler) ||
           !cloCOMPILER_IsExternSymbolsAllowed(Compiler) ||
           !(DeclOrDeclListPtr->decl.storageQualifier & clvSTORAGE_QUALIFIER_EXTERN)) {
            gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                            Identifier->lineNo,
                            Identifier->stringNo,
                            clvREPORT_ERROR,
                            "require the initializer for the 'const' variable: '%s'",
                            Identifier->u.identifier.name));
            return gcvSTATUS_INVALID_ARGUMENT;
        }
        break;

    case clvQUALIFIER_ATTRIBUTE:
        gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                        Identifier->lineNo,
                        Identifier->stringNo,
                        clvREPORT_ERROR,
                        "cannot declare the array: '%s' with the '%s' qualifier",
                        Identifier->u.identifier.name,
                        clGetQualifierName(DeclOrDeclListPtr->decl.dataType->accessQualifier)));
        return gcvSTATUS_INVALID_ARGUMENT;
    }

    clmEvaluateExprToArrayLength(Compiler,
                                 ArrayLengthExpr,
                                 array,
                                 gcvFALSE,
                                 status);
    if (gcmIS_ERROR(status)) return status;


    switch(DeclOrDeclListPtr->decl.dataType->type) {
    case T_TYPE_NAME:
        status = _ParseFlattenType(Compiler, &DeclOrDeclListPtr->decl, decl);
        if(gcmIS_ERROR(status)) return gcvSTATUS_INVALID_ARGUMENT;
        declPtr = decl;
        derivedType = DeclOrDeclListPtr->decl.dataType->u.typeDef;
        break;

    case T_ENUM:
        declPtr = &DeclOrDeclListPtr->decl;
        derivedType = DeclOrDeclListPtr->decl.dataType->u.enumerator;
        break;

    default:
        declPtr = _HandleSpecialType(Compiler, &DeclOrDeclListPtr->decl);
        break;
    }

    if (clmDATA_TYPE_IsSampler(declPtr->dataType)) {
        if(declPtr->dataType->accessQualifier != clvQUALIFIER_CONST) {
/*KLC force it to constant as conformance test did not declare it as constant*/
            status = cloCOMPILER_CloneDataType(Compiler,
                               clvQUALIFIER_CONST,
                               declPtr->dataType->addrSpaceQualifier,
                               declPtr->dataType,
                               &declPtr->dataType);
            if (gcmIS_ERROR(status)) return status;
        }
    }
    if (clmDECL_IsImage(declPtr)) {
        gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                        Identifier->lineNo,
                        Identifier->stringNo,
                        clvREPORT_ERROR,
                        "variable '%s' cannot have image type",
                        Identifier->u.identifier.name));
        return gcvSTATUS_INVALID_ARGUMENT;
    }

    if(declPtr->dataType->type == T_HALF) {
        if(!cloCOMPILER_ExtensionEnabled(Compiler, clvEXTENSION_VIV_VX | clvEXTENSION_CL_KHR_FP16)) {
            if((clmDECL_IsHalfType(declPtr) || clmDECL_IsArray(declPtr)) &&
                !Identifier->u.identifier.ptrDscr) {
                gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                                Identifier->lineNo,
                                                Identifier->stringNo,
                                                clvREPORT_ERROR,
                                                "variable '%s' cannot have half type",
                                                Identifier->u.identifier.name));
                return gcvSTATUS_INVALID_ARGUMENT;
            }
        }
    }

    if (clmDECL_IsStructOrUnion(declPtr) && declPtr->dataType->u.fieldSpace->scopeName->isBuiltin)  {
        gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                        Identifier->lineNo,
                        Identifier->stringNo,
                        clvREPORT_ERROR,
                        "variable '%s' cannot have struct '%s' type",
                        Identifier->u.identifier.name,
                        declPtr->dataType->u.fieldSpace->scopeName->symbol));
        return gcvSTATUS_INVALID_ARGUMENT;
    }
    status = _ParseMergeArrayDecl(Compiler,
                      declPtr,
                      array,
                      arrayDecl);
    if (gcmIS_ERROR(status)) return status;

    status = clMergePtrDscrToDecl(Compiler,
                                  Identifier->u.identifier.ptrDscr,
                                  arrayDecl,
                                  gcvFALSE);
    if (gcmIS_ERROR(status)) return status;

    status = cloCOMPILER_CreateName(Compiler,
                    Identifier->lineNo,
                    Identifier->stringNo,
                    clvVARIABLE_NAME,
                    arrayDecl,
                    Identifier->u.identifier.name,
                    gcvNULL,
                    clvEXTENSION_NONE,
                    &DeclOrDeclListPtr->name);
    if (gcmIS_ERROR(status)) return status;

    DeclOrDeclListPtr->name->derivedType = derivedType;
    if(cloCOMPILER_GetParserState(Compiler) != clvPARSER_IN_TYPEDEF) {
        status = _ParseCheckVariableNeedMemory(Compiler,
                                               DeclOrDeclListPtr->name);
        if (gcmIS_ERROR(status)) return status;
    }

    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                clvDUMP_PARSER,
                "<VARIABLE_DECL line=\"%d\" string=\"%d\" name=\"%s\" />",
                Identifier->lineNo,
                Identifier->stringNo,
                Identifier->u.identifier.name));
    return gcvSTATUS_OK;
}

#define _clmParseCreateLhs(Compiler, LineNo, StringNo, DeclList, Status)  do { \
    cloIR_VARIABLE variable; \
    (Status) = cloIR_VARIABLE_Construct((Compiler), \
                        (LineNo), \
                        (StringNo), \
                        (DeclList)->name, \
                        &variable); \
        if(gcmIS_ERROR(Status)) break; \
    (DeclList)->lhs = (DeclList)->designator = &variable->exprBase; \
    (Status) = cloCOMPILER_PushDesignationScope((Compiler), \
                            (DeclList)->lhs); \
    } while (gcvFALSE)

static gceSTATUS
_ParseArrayVariableDeclInit(
IN cloCOMPILER Compiler,
IN clsDeclOrDeclList *DeclOrDeclListPtr,
IN clsLexToken * Identifier,
IN cloIR_EXPR ArrayLengthExpr
)
{
    gceSTATUS status;
    clsARRAY array[1];
    clsDECL arrayDecl[1];
    clsDECL decl[1];
    clsDECL *declPtr;
    clsNAME *derivedType = gcvNULL;

    gcmASSERT(DeclOrDeclListPtr && DeclOrDeclListPtr->decl.dataType);
    gcmASSERT(Identifier);

    clmEvaluateExprToArrayLength(Compiler,
                                 ArrayLengthExpr,
                                 array,
                                 gcvTRUE,
                                 status);
    if (gcmIS_ERROR(status)) return status;

    switch(DeclOrDeclListPtr->decl.dataType->type) {
    case T_TYPE_NAME:
        derivedType = DeclOrDeclListPtr->decl.dataType->u.typeDef;
        status = _ParseFlattenType(Compiler, &DeclOrDeclListPtr->decl, decl);
        if(gcmIS_ERROR(status)) return status;
        declPtr = decl;
        break;

    case T_ENUM:
        derivedType = DeclOrDeclListPtr->decl.dataType->u.enumerator;
        declPtr = &DeclOrDeclListPtr->decl;
        break;

    default:
        declPtr = _HandleSpecialType(Compiler, &DeclOrDeclListPtr->decl);
        break;
    }

    if (clmDATA_TYPE_IsSampler(declPtr->dataType)) {
        if(declPtr->dataType->accessQualifier != clvQUALIFIER_CONST) {
            status = cloCOMPILER_CloneDataType(Compiler,
                               clvQUALIFIER_CONST,
                               declPtr->dataType->addrSpaceQualifier,
                               declPtr->dataType,
                               &declPtr->dataType);
            if (gcmIS_ERROR(status)) return status;
        }
    }

    if(declPtr->dataType->type == T_IMAGE2D_DYNAMIC_ARRAY_T) {
        gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                        Identifier->lineNo,
                        Identifier->stringNo,
                        clvREPORT_ERROR,
                        "unrecognizable type '_viv_image2d_array_t' specified for variable '%s'",
                        Identifier->u.identifier.name));
        return gcvSTATUS_INVALID_ARGUMENT;
    }
    if (clmDATA_TYPE_IsImage(declPtr->dataType)) {
        gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                        Identifier->lineNo,
                        Identifier->stringNo,
                        clvREPORT_ERROR,
                        "variable '%s' cannot have image type",
                        Identifier->u.identifier.name));
        return gcvSTATUS_INVALID_ARGUMENT;
    }
    status = _ParseMergeArrayDecl(Compiler,
                      declPtr,
                      array,
                      arrayDecl);
    if (gcmIS_ERROR(status)) return status;

    status = clMergePtrDscrToDecl(Compiler,
                                  Identifier->u.identifier.ptrDscr,
                                  arrayDecl,
                                  gcvFALSE);
    if (gcmIS_ERROR(status)) return status;

    status = cloCOMPILER_CreateName(Compiler,
                    Identifier->lineNo,
                    Identifier->stringNo,
                    clvVARIABLE_NAME,
                    arrayDecl,
                    Identifier->u.identifier.name,
                    gcvNULL,
                    clvEXTENSION_NONE,
                    &DeclOrDeclListPtr->name);
    if (gcmIS_ERROR(status)) return status;

    DeclOrDeclListPtr->name->derivedType = derivedType;
    if(cloCOMPILER_GetParserState(Compiler) != clvPARSER_IN_TYPEDEF) {
        status = _ParseCheckVariableNeedMemory(Compiler,
                                               DeclOrDeclListPtr->name);
        if (gcmIS_ERROR(status)) return status;
    }

    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                clvDUMP_PARSER,
                "<VARIABLE_DECL line=\"%d\" string=\"%d\" name=\"%s\" />",
                Identifier->lineNo,
                Identifier->stringNo,
                Identifier->u.identifier.name));
    return gcvSTATUS_OK;
}

clsDeclOrDeclList *
clParseVariableDecl(
IN cloCOMPILER Compiler,
IN clsDECL * Decl,
IN clsLexToken * Identifier,
IN clsATTRIBUTE *Attr
)
{
    gceSTATUS status;
    clsDeclOrDeclList *declOrDeclListPtr;
    gctPOINTER pointer;

    gcmASSERT(Identifier);
    gcmASSERT(Decl);
    status = cloCOMPILER_Allocate(Compiler,
                  (gctSIZE_T)sizeof(clsDeclOrDeclList),
                  (gctPOINTER *) &pointer);

    if (gcmIS_ERROR(status)) return gcvNULL;
    declOrDeclListPtr = pointer;

    declOrDeclListPtr->decl    = *Decl;
    declOrDeclListPtr->name    = gcvNULL;
    declOrDeclListPtr->lhs    = gcvNULL;
    declOrDeclListPtr->designator= gcvNULL;
    declOrDeclListPtr->initStatement= gcvNULL;
    declOrDeclListPtr->initStatements= gcvNULL;

    if (declOrDeclListPtr->decl.dataType == gcvNULL) return declOrDeclListPtr;

    status = _ParseVariableDecl(Compiler,
                    declOrDeclListPtr,
                    Identifier);
    if (gcmIS_ERROR(status))  return declOrDeclListPtr;

    _ParseFillVariableAttr(Compiler,
                           Identifier->lineNo,
                           Identifier->stringNo,
                           Decl, declOrDeclListPtr->name, Attr);
    return declOrDeclListPtr;
}

clsDeclOrDeclList *
clParseArrayVariableDecl(
IN cloCOMPILER Compiler,
IN clsDECL * Decl,
IN clsLexToken * Identifier,
IN cloIR_EXPR ArrayLengthExpr,
IN clsATTRIBUTE *Attr
)
{
    gceSTATUS status;
    clsDeclOrDeclList *declOrDeclListPtr;
    gctPOINTER pointer;

    gcmASSERT(Identifier);
    gcmASSERT(Decl);

    status = cloCOMPILER_Allocate(Compiler,
                      (gctSIZE_T)sizeof(clsDeclOrDeclList),
                      (gctPOINTER *) &pointer);

    if (gcmIS_ERROR(status)) return gcvNULL;
    declOrDeclListPtr = pointer;

    declOrDeclListPtr->decl    = *Decl;
    declOrDeclListPtr->name    = gcvNULL;
    declOrDeclListPtr->lhs    = gcvNULL;
    declOrDeclListPtr->designator= gcvNULL;
    declOrDeclListPtr->initStatement= gcvNULL;
    declOrDeclListPtr->initStatements= gcvNULL;

    if (declOrDeclListPtr->decl.dataType == gcvNULL) return declOrDeclListPtr;
    if (clmIR_EXPR_IsUnaryType(ArrayLengthExpr, clvUNARY_NULL)) {
       gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                       Identifier->lineNo,
                       Identifier->stringNo,
                       clvREPORT_ERROR,
                       "unspecified size array \"%s\" not supported\'%s\'",
                       Identifier->u.identifier.name));
       return declOrDeclListPtr;
    }

    status = _ParseArrayVariableDecl(Compiler,
                     declOrDeclListPtr,
                     Identifier,
                     ArrayLengthExpr);
    if (gcmIS_ERROR(status)) {
        return declOrDeclListPtr;
    }

    _ParseFillVariableAttr(Compiler,
                           Identifier->lineNo,
                           Identifier->stringNo,
                           Decl, declOrDeclListPtr->name, Attr);
    return declOrDeclListPtr;
}

clsDeclOrDeclList *
clParseVariableDeclList(
IN cloCOMPILER Compiler,
IN clsDeclOrDeclList *DeclOrDeclListPtr,
IN clsLexToken * Identifier,
IN clsATTRIBUTE *Attr
)
{
    gceSTATUS status;
    clsDeclOrDeclList *declOrDeclListPtr;

    gcmASSERT(Identifier);
    declOrDeclListPtr = DeclOrDeclListPtr;

    if (declOrDeclListPtr->name == gcvNULL ||
        declOrDeclListPtr->decl.dataType == gcvNULL) return declOrDeclListPtr;

    if (declOrDeclListPtr->name->type == clvTYPE_NAME) {
       cloCOMPILER_PushParserState(Compiler, clvPARSER_IN_TYPEDEF);
       status = _ParseVariableDecl(Compiler,
                                   declOrDeclListPtr,
                                   Identifier);
       cloCOMPILER_PopParserState(Compiler);
       if (gcmIS_ERROR(status)) {
           return declOrDeclListPtr;
       }
       declOrDeclListPtr = clParseTypeDef(Compiler,
                                          declOrDeclListPtr);
    }
    else {
       status = _ParseVariableDecl(Compiler,
                                   declOrDeclListPtr,
                                   Identifier);
       if (gcmIS_ERROR(status)) {
           return declOrDeclListPtr;
       }
    }

    _ParseFillVariableAttr(Compiler,
                           Identifier->lineNo,
                           Identifier->stringNo,
                           &declOrDeclListPtr->decl, declOrDeclListPtr->name, Attr);
    return declOrDeclListPtr;
}

clsDeclOrDeclList *
clParseArrayVariableDeclList(
IN cloCOMPILER Compiler,
IN clsDeclOrDeclList *DeclOrDeclListPtr,
IN clsLexToken * Identifier,
IN cloIR_EXPR ArrayLengthExpr,
IN clsATTRIBUTE *Attr
)
{
    gceSTATUS        status;
    clsDeclOrDeclList    *declOrDeclListPtr;

    gcmASSERT(Identifier);
    declOrDeclListPtr = DeclOrDeclListPtr;

    if (declOrDeclListPtr->decl.dataType == gcvNULL || ArrayLengthExpr == gcvNULL) return declOrDeclListPtr;

    if (declOrDeclListPtr->name->type == clvTYPE_NAME) {
       cloCOMPILER_PushParserState(Compiler, clvPARSER_IN_TYPEDEF);
       status = _ParseArrayVariableDecl(Compiler,
                                        declOrDeclListPtr,
                                        Identifier,
                                        ArrayLengthExpr);
       cloCOMPILER_PopParserState(Compiler);
       if (gcmIS_ERROR(status)) {
           return declOrDeclListPtr;
       }
       declOrDeclListPtr = clParseTypeDef(Compiler,
                                          declOrDeclListPtr);
    }
    else {
       status = _ParseArrayVariableDecl(Compiler,
                                        declOrDeclListPtr,
                                        Identifier,
                                        ArrayLengthExpr);
       if (gcmIS_ERROR(status)) {
           return declOrDeclListPtr;
       }
    }

    _ParseFillVariableAttr(Compiler,
                           Identifier->lineNo,
                           Identifier->stringNo,
                           &declOrDeclListPtr->decl, declOrDeclListPtr->name, Attr);
    return declOrDeclListPtr;
}

clsDeclOrDeclList *
clParseFinishDeclInit(
IN cloCOMPILER Compiler,
IN clsDeclOrDeclList *Designation,
IN cloIR_EXPR InitExpr
)
{
   clsDeclOrDeclList *designation;

   designation = clParseInitializeCurrentObject(Compiler,
                                                Designation,
                                                InitExpr);
   (void) cloCOMPILER_PopDesignationScope(Compiler);
   return designation;
}


static cloIR_EXPR
_FinalizeInitializer(
IN cloCOMPILER Compiler,
IN clsDeclOrDeclList *Designation,
IN cloIR_EXPR InitExpr
)
{
  gceSTATUS status;
  cloIR_EXPR resExpr;
  gcmASSERT(InitExpr);

  if(!Designation) return InitExpr;

/*KLC */
  status = cloCOMPILER_PopDesignationScope(Compiler);
  if (gcmIS_ERROR(status)) return gcvNULL;

  if(InitExpr->decl.dataType == gcvNULL) {
     InitExpr->decl = Designation->lhs->decl;
  }
  status = cloIR_BINARY_EXPR_Construct(Compiler,
                                       InitExpr->base.lineNo,
                                       InitExpr->base.stringNo,
                                       clvBINARY_ASSIGN,
                                       Designation->lhs,
                                       InitExpr,
                                       &resExpr);
  if (gcmIS_ERROR(status)) return gcvNULL;
  return resExpr;
}

cloIR_EXPR
clParseInitializerList(
IN cloCOMPILER Compiler,
IN cloIR_EXPR InitExprList,
IN clsDeclOrDeclList *Designation,
IN cloIR_EXPR InitExpr
)
{
  cloIR_EXPR resultExpr;

  gcmASSERT(InitExpr);
  resultExpr = _FinalizeInitializer(Compiler, Designation, InitExpr);

  if(InitExprList == gcvNULL) return resultExpr;
  else {
     gceSTATUS status;
     cloIR_BASE base;
     cloIR_TYPECAST_ARGS typeCastArgs;

     base = &InitExprList->base;
     gcmASSERT(cloIR_OBJECT_GetType(base) == clvIR_TYPECAST_ARGS);
    typeCastArgs = (cloIR_TYPECAST_ARGS) base;
    if(!typeCastArgs->operands) {
       status = cloIR_SET_Construct(Compiler,
                    base->lineNo,
                    base->stringNo,
                    clvEXPR_SET,
                    &typeCastArgs->operands);
       if (gcmIS_ERROR(status)) return gcvNULL;
    }
    gcmVERIFY_OK(cloIR_SET_AddMember(Compiler,
                     typeCastArgs->operands,
                     &resultExpr->base));

    if(Designation) {
       typeCastArgs->lhs = Designation->lhs;

    }
    else {
       typeCastArgs->lhs = gcvNULL;
    }
    return &typeCastArgs->exprBase;
  }
}

clsDeclOrDeclList *
clParseFinishDeclListInit(
IN cloCOMPILER Compiler,
IN clsDeclOrDeclList *Designation,
IN cloIR_EXPR InitExpr
)
{
   return clParseInitializeCurrentObject(Compiler,
                                         Designation,
                                         InitExpr);
}

clsDeclOrDeclList *
clParseSubscriptDesignator(
IN cloCOMPILER Compiler,
IN clsDeclOrDeclList *DesignatorList,
IN cloIR_EXPR Subscript,
IN gctINT TokenType
)
{
    gceSTATUS status;
    clsDeclOrDeclList *subscript = gcvNULL;
    clsDESIGNATION_SCOPE *designationScope;
    gctPOINTER pointer;

    if(DesignatorList == gcvNULL) return gcvNULL;
    switch(TokenType) {
    case T_EOF:
        /* append field selection to lhs */
    gcmASSERT(DesignatorList);
    subscript = DesignatorList;
    subscript->lhs = clParseSubscriptExpr(Compiler,
                          subscript->lhs,
                          Subscript);
        break;

    case '{':
    case ',':
        /* use designator's lhs to create lhs */
        designationScope = cloCOMPILER_GetDesignationScope(Compiler);
        gcmASSERT(designationScope);
        if(!designationScope) return DesignatorList;
        status = cloCOMPILER_Allocate(Compiler,
                      (gctSIZE_T)sizeof(clsDeclOrDeclList),
                      (gctPOINTER *) &pointer);
        if (gcmIS_ERROR(status)) return gcvNULL;

    (void) gcoOS_ZeroMemory(pointer, sizeof(clsDeclOrDeclList));
        subscript = pointer;

    subscript->lhs = clParseSubscriptExpr(Compiler,
                          designationScope->designation,
                          Subscript);
        break;

    default:
        gcmASSERT(0);
        break;
    }
    return subscript;
}

gceSTATUS
cloIR_EXPR_Clone(
IN cloCOMPILER Compiler,
IN gctUINT LineNo,
IN gctUINT StringNo,
IN cloIR_EXPR Source,
OUT cloIR_EXPR * Result
)
{
    gceSTATUS  status = gcvSTATUS_OK;
    cloIR_EXPR result = Source;
    cloIR_VARIABLE variable;

    switch(cloIR_OBJECT_GetType(&Source->base)) {
    case clvIR_VARIABLE:
        gcmONERROR(cloIR_VARIABLE_Construct(Compiler,
                                            LineNo,
                                            StringNo,
                                            ((cloIR_VARIABLE) &Source->base)->name,
                                            &variable));

        result = &variable->exprBase;
        break;

    default:
        /* Need to do for other object types */
        break;
    }

OnError:
    if(Result) *Result = result;
    return status;
}

clsDeclOrDeclList *
clParseFieldSelectionDesignator(
IN cloCOMPILER Compiler,
IN clsDeclOrDeclList *DesignatorList,
IN clsLexToken *FieldSelection,
IN gctINT TokenType
)
{
    gceSTATUS status;
    clsDESIGNATION_SCOPE *designationScope;
    clsDeclOrDeclList *fieldSelection = gcvNULL;
    cloIR_EXPR lhs;
    gctPOINTER pointer;

    gcmASSERT(DesignatorList);
    switch(TokenType) {
    case T_EOF:
        /* append field selection to lhs */
        gcmASSERT(DesignatorList);
        fieldSelection = DesignatorList;
        gcmONERROR(cloIR_EXPR_Clone(Compiler,
                                    FieldSelection->lineNo,
                                    FieldSelection->stringNo,
                                    fieldSelection->lhs,
                                    &lhs));

        fieldSelection->lhs = clParseFieldSelectionExpr(Compiler,
                                                        lhs,
                                                        FieldSelection);
        break;

    case '{':
    case ',':
        /* use designator's lhs to create lhs */
        designationScope = cloCOMPILER_GetDesignationScope(Compiler);
        gcmASSERT(designationScope);
        if(!designationScope) return DesignatorList;
        status = cloCOMPILER_Allocate(Compiler,
                                      (gctSIZE_T)sizeof(clsDeclOrDeclList),
                                      (gctPOINTER *) &pointer);

        if (gcmIS_ERROR(status)) return gcvNULL;

        (void) gcoOS_ZeroMemory(pointer, sizeof(clsDeclOrDeclList));
        fieldSelection = pointer;
        gcmONERROR(cloIR_EXPR_Clone(Compiler,
                                    FieldSelection->lineNo,
                                    FieldSelection->stringNo,
                                    designationScope->designation,
                                    &lhs));

        fieldSelection->lhs = clParseFieldSelectionExpr(Compiler,
                                                        lhs,
                                                        FieldSelection);
        break;

    default:
        gcmASSERT(0);
        break;
    }
OnError:
    return fieldSelection;
}

gceSTATUS
clParseConvertConstantValues(
IN gctUINT ValueCount,
IN cltELEMENT_TYPE SourceType,
IN cluCONSTANT_VALUE *SourceValues,
IN cltELEMENT_TYPE ToType,
IN cluCONSTANT_VALUE *ToValues
)
{
   gctUINT i;

   switch(ToType) {
   case clvTYPE_FLOAT:
   case clvTYPE_HALF_PACKED:
   case clvTYPE_HALF:
     switch(SourceType) {
     case clvTYPE_UINT:
     case clvTYPE_UCHAR:
     case clvTYPE_USHORT:
     case clvTYPE_UCHAR_PACKED:
     case clvTYPE_USHORT_PACKED:
     case clvTYPE_EVENT_T:
     case clvTYPE_SAMPLER_T:
        for(i=0; i < ValueCount; i++) {
           ToValues[i].floatValue = (gctFLOAT) SourceValues[i].uintValue;
        }
        break;

     case clvTYPE_ULONG:
        for(i=0; i < ValueCount; i++) {
           ToValues[i].floatValue = (gctFLOAT) SourceValues[i].ulongValue;
        }
        break;

     case clvTYPE_INT:
     case clvTYPE_SHORT:
     case clvTYPE_SHORT_PACKED:
        for(i=0; i < ValueCount; i++) {
           ToValues[i].floatValue = (gctFLOAT) SourceValues[i].intValue;
        }
        break;

     case clvTYPE_LONG:
        for(i=0; i < ValueCount; i++) {
           ToValues[i].floatValue = (gctFLOAT) SourceValues[i].longValue;
        }
        break;

     case clvTYPE_BOOL:
     case clvTYPE_BOOL_PACKED:
        for(i=0; i < ValueCount; i++) {
           ToValues[i].floatValue = (gctFLOAT) SourceValues[i].boolValue;
        }
        break;

     case clvTYPE_CHAR:
     case clvTYPE_CHAR_PACKED:
        for(i=0; i < ValueCount; i++) {
           ToValues[i].floatValue = (gctFLOAT) SourceValues[i].intValue;
        }
        break;

     case clvTYPE_FLOAT:
     case clvTYPE_HALF_PACKED:
        for(i=0; i < ValueCount; i++) {
           ToValues[i].floatValue = SourceValues[i].floatValue;
        }
        break;
default:
        gcmASSERT(0);
        return gcvSTATUS_INVALID_ARGUMENT;
     }
     break;

   case clvTYPE_INT:
     switch(SourceType) {
     case clvTYPE_HALF:
     case clvTYPE_FLOAT:
     case clvTYPE_HALF_PACKED:
        for(i=0; i < ValueCount; i++) {
           ToValues[i].intValue = (gctINT32) SourceValues[i].floatValue;
        }
        break;

     case clvTYPE_BOOL:
     case clvTYPE_BOOL_PACKED:
        for(i=0; i < ValueCount; i++) {
           ToValues[i].intValue = (gctINT32) SourceValues[i].boolValue;
        }
        break;

     case clvTYPE_UINT:
     case clvTYPE_UCHAR:
     case clvTYPE_USHORT:
     case clvTYPE_UCHAR_PACKED:
     case clvTYPE_USHORT_PACKED:
     case clvTYPE_EVENT_T:
     case clvTYPE_SAMPLER_T:
        for(i=0; i < ValueCount; i++) {
           ToValues[i].intValue = (gctINT32) SourceValues[i].uintValue;
        }
        break;

     case clvTYPE_ULONG:
        for(i=0; i < ValueCount; i++) {
           ToValues[i].intValue = (gctINT32) SourceValues[i].ulongValue;
        }
        break;

     case clvTYPE_INT:
     case clvTYPE_SHORT:
     case clvTYPE_CHAR:
     case clvTYPE_SHORT_PACKED:
     case clvTYPE_CHAR_PACKED:
        for(i=0; i < ValueCount; i++) {
           ToValues[i].intValue = SourceValues[i].intValue;
        }
        break;

     case clvTYPE_LONG:
        for(i=0; i < ValueCount; i++) {
           ToValues[i].intValue = (gctINT32) SourceValues[i].longValue;
        }
        break;

     default:
        gcmASSERT(0);
        return gcvSTATUS_INVALID_ARGUMENT;
     }
     break;

   case clvTYPE_LONG:
     switch(SourceType) {
     case clvTYPE_FLOAT:
     case clvTYPE_HALF_PACKED:
        for(i=0; i < ValueCount; i++) {
           ToValues[i].longValue = (gctINT64) SourceValues[i].floatValue;
        }
        break;

     case clvTYPE_BOOL:
     case clvTYPE_BOOL_PACKED:
        for(i=0; i < ValueCount; i++) {
           ToValues[i].longValue = (gctINT64) SourceValues[i].boolValue;
        }
        break;

     case clvTYPE_UINT:
     case clvTYPE_UCHAR:
     case clvTYPE_USHORT:
     case clvTYPE_UCHAR_PACKED:
     case clvTYPE_USHORT_PACKED:
     case clvTYPE_EVENT_T:
     case clvTYPE_SAMPLER_T:
        for(i=0; i < ValueCount; i++) {
           ToValues[i].longValue = (gctINT64) SourceValues[i].uintValue;
        }
        break;

     case clvTYPE_ULONG:
        for(i=0; i < ValueCount; i++) {
           ToValues[i].longValue = (gctINT64) SourceValues[i].ulongValue;
        }
        break;

     case clvTYPE_INT:
     case clvTYPE_SHORT:
     case clvTYPE_CHAR:
     case clvTYPE_SHORT_PACKED:
     case clvTYPE_CHAR_PACKED:
        for(i=0; i < ValueCount; i++) {
           ToValues[i].longValue = (gctINT64) SourceValues[i].intValue;
        }
        break;

     case clvTYPE_LONG:
        for(i=0; i < ValueCount; i++) {
           ToValues[i].longValue = SourceValues[i].longValue;
        }
        break;

     default:
        gcmASSERT(0);
        return gcvSTATUS_INVALID_ARGUMENT;
     }
     break;

   case clvTYPE_SHORT:
   case clvTYPE_SHORT_PACKED:
     switch(SourceType) {
     case clvTYPE_INT:
     case clvTYPE_CHAR:
     case clvTYPE_SHORT:
     case clvTYPE_CHAR_PACKED:
     case clvTYPE_SHORT_PACKED:
        for(i=0; i < ValueCount; i++) {
           ToValues[i].intValue = (gctINT16) SourceValues[i].intValue;
        }
        break;

     case clvTYPE_LONG:
        for(i=0; i < ValueCount; i++) {
           ToValues[i].intValue = (gctINT16) SourceValues[i].longValue;
        }
        break;

     case clvTYPE_FLOAT:
     case clvTYPE_HALF_PACKED:
        for(i=0; i < ValueCount; i++) {
           ToValues[i].intValue = (gctINT16) SourceValues[i].floatValue;
        }
        break;

     case clvTYPE_BOOL:
     case clvTYPE_BOOL_PACKED:
        for(i=0; i < ValueCount; i++) {
           ToValues[i].intValue = (gctINT16) SourceValues[i].boolValue;
        }
        break;

     case clvTYPE_UINT:
     case clvTYPE_UCHAR:
     case clvTYPE_USHORT:
     case clvTYPE_UCHAR_PACKED:
     case clvTYPE_USHORT_PACKED:
     case clvTYPE_EVENT_T:
     case clvTYPE_SAMPLER_T:
        for(i=0; i < ValueCount; i++) {
           ToValues[i].intValue = (gctINT16) SourceValues[i].uintValue;
        }
        break;

     case clvTYPE_ULONG:
        for(i=0; i < ValueCount; i++) {
           ToValues[i].intValue = (gctINT16) SourceValues[i].ulongValue;
        }
        break;

     default:
        gcmASSERT(0);
        return gcvSTATUS_INVALID_ARGUMENT;
     }
     break;

   case clvTYPE_VOID:
   case clvTYPE_EVENT_T:
   case clvTYPE_SAMPLER_T:
   case clvTYPE_UINT:
     switch(SourceType) {
     case clvTYPE_FLOAT:
        for(i=0; i < ValueCount; i++) {
           ToValues[i].uintValue = (gctUINT32) SourceValues[i].floatValue;
        }
        break;

     case clvTYPE_BOOL:
     case clvTYPE_BOOL_PACKED:
        for(i=0; i < ValueCount; i++) {
           ToValues[i].uintValue = (gctUINT32) SourceValues[i].boolValue;
        }
        break;

     case clvTYPE_INT:
     case clvTYPE_SHORT:
     case clvTYPE_SHORT_PACKED:
        for(i=0; i < ValueCount; i++) {
           ToValues[i].uintValue = (gctUINT32) SourceValues[i].intValue;
        }
        break;

     case clvTYPE_LONG:
        for(i=0; i < ValueCount; i++) {
           ToValues[i].uintValue = (gctUINT32) SourceValues[i].longValue;
        }
        break;

     case clvTYPE_CHAR:
     case clvTYPE_CHAR_PACKED:
        for(i=0; i < ValueCount; i++) {
           ToValues[i].uintValue = (gctUINT32)SourceValues[i].intValue;
        }
        break;

     case clvTYPE_UINT:
     case clvTYPE_USHORT:
     case clvTYPE_UCHAR:
     case clvTYPE_USHORT_PACKED:
     case clvTYPE_UCHAR_PACKED:
     case clvTYPE_EVENT_T:
     case clvTYPE_SAMPLER_T:
        for(i=0; i < ValueCount; i++) {
           ToValues[i].uintValue = SourceValues[i].uintValue;
        }
        break;

     case clvTYPE_ULONG:
        for(i=0; i < ValueCount; i++) {
           ToValues[i].uintValue = (gctUINT32) SourceValues[i].ulongValue;
        }
        break;

     default:
        gcmASSERT(0);
        return gcvSTATUS_INVALID_ARGUMENT;
     }
     break;

   case clvTYPE_ULONG:
     switch(SourceType) {
     case clvTYPE_FLOAT:
     case clvTYPE_HALF_PACKED:
        for(i=0; i < ValueCount; i++) {
           ToValues[i].ulongValue = (gctUINT64) SourceValues[i].floatValue;
        }
        break;

     case clvTYPE_BOOL:
     case clvTYPE_BOOL_PACKED:
        for(i=0; i < ValueCount; i++) {
           ToValues[i].ulongValue = (gctUINT64) SourceValues[i].boolValue;
        }
        break;

     case clvTYPE_INT:
     case clvTYPE_SHORT:
     case clvTYPE_SHORT_PACKED:
        for(i=0; i < ValueCount; i++) {
           ToValues[i].ulongValue = (gctUINT64) SourceValues[i].intValue;
        }
        break;

     case clvTYPE_LONG:
        for(i=0; i < ValueCount; i++) {
           ToValues[i].ulongValue = (gctUINT64) SourceValues[i].longValue;
        }
        break;

     case clvTYPE_CHAR:
     case clvTYPE_CHAR_PACKED:
        for(i=0; i < ValueCount; i++) {
           ToValues[i].ulongValue = (gctUINT64)SourceValues[i].intValue;
        }
        break;

     case clvTYPE_UINT:
     case clvTYPE_USHORT:
     case clvTYPE_UCHAR:
     case clvTYPE_USHORT_PACKED:
     case clvTYPE_UCHAR_PACKED:
     case clvTYPE_EVENT_T:
     case clvTYPE_SAMPLER_T:
        for(i=0; i < ValueCount; i++) {
           ToValues[i].ulongValue = (gctUINT64) SourceValues[i].uintValue;
        }
        break;

     case clvTYPE_ULONG:
        for(i=0; i < ValueCount; i++) {
           ToValues[i].ulongValue = SourceValues[i].ulongValue;
        }
        break;

     default:
        gcmASSERT(0);
        return gcvSTATUS_INVALID_ARGUMENT;
     }
     break;

   case clvTYPE_USHORT:
   case clvTYPE_USHORT_PACKED:
     switch(SourceType) {
     case clvTYPE_FLOAT:
     case clvTYPE_HALF_PACKED:
        for(i=0; i < ValueCount; i++) {
           ToValues[i].uintValue = (gctUINT16) SourceValues[i].floatValue;
        }
        break;

     case clvTYPE_BOOL:
     case clvTYPE_BOOL_PACKED:
        for(i=0; i < ValueCount; i++) {
           ToValues[i].uintValue = (gctUINT16) SourceValues[i].boolValue;
        }
        break;

     case clvTYPE_INT:
     case clvTYPE_SHORT:
     case clvTYPE_SHORT_PACKED:
        for(i=0; i < ValueCount; i++) {
           ToValues[i].uintValue = (gctUINT16) SourceValues[i].intValue;
        }
        break;

     case clvTYPE_LONG:
        for(i=0; i < ValueCount; i++) {
           ToValues[i].uintValue = (gctUINT16) SourceValues[i].longValue;
        }
        break;

     case clvTYPE_CHAR:
     case clvTYPE_CHAR_PACKED:
        for(i=0; i < ValueCount; i++) {
           ToValues[i].uintValue = (gctUINT16)SourceValues[i].intValue;
        }
    break;

     case clvTYPE_UINT:
     case clvTYPE_USHORT:
     case clvTYPE_USHORT_PACKED:
     case clvTYPE_UCHAR:
     case clvTYPE_UCHAR_PACKED:
     case clvTYPE_EVENT_T:
     case clvTYPE_SAMPLER_T:
        for(i=0; i < ValueCount; i++) {
           ToValues[i].uintValue = (gctUINT16) SourceValues[i].uintValue;
        }
        break;

     case clvTYPE_ULONG:
        for(i=0; i < ValueCount; i++) {
           ToValues[i].uintValue = (gctUINT16) SourceValues[i].ulongValue;
        }
        break;

     default:
        gcmASSERT(0);
        return gcvSTATUS_INVALID_ARGUMENT;
     }
     break;

   case clvTYPE_UCHAR:
   case clvTYPE_UCHAR_PACKED:
     switch(SourceType) {
     case clvTYPE_FLOAT:
     case clvTYPE_HALF_PACKED:
        for(i=0; i < ValueCount; i++) {
           ToValues[i].uintValue = (gctUINT8) SourceValues[i].floatValue;
        }
        break;

     case clvTYPE_BOOL:
     case clvTYPE_BOOL_PACKED:
        for(i=0; i < ValueCount; i++) {
           ToValues[i].uintValue = (gctUINT8) SourceValues[i].boolValue;
        }
        break;

     case clvTYPE_INT:
     case clvTYPE_SHORT:
     case clvTYPE_SHORT_PACKED:
        for(i=0; i < ValueCount; i++) {
           ToValues[i].uintValue = (gctUINT8) SourceValues[i].intValue;
        }
        break;

     case clvTYPE_LONG:
        for(i=0; i < ValueCount; i++) {
           ToValues[i].uintValue = (gctUINT8) SourceValues[i].longValue;
        }
        break;

     case clvTYPE_CHAR:
     case clvTYPE_CHAR_PACKED:
        for(i=0; i < ValueCount; i++) {
           ToValues[i].uintValue = (gctUINT8)SourceValues[i].intValue;
        }
        break;

     case clvTYPE_UINT:
     case clvTYPE_USHORT:
     case clvTYPE_USHORT_PACKED:
     case clvTYPE_UCHAR:
     case clvTYPE_UCHAR_PACKED:
     case clvTYPE_EVENT_T:
     case clvTYPE_SAMPLER_T:
        for(i=0; i < ValueCount; i++) {
           ToValues[i].uintValue = (gctUINT8)SourceValues[i].uintValue;
        }
        break;

     case clvTYPE_ULONG:
        for(i=0; i < ValueCount; i++) {
           ToValues[i].uintValue = (gctUINT8) SourceValues[i].ulongValue;
        }
        break;

     default:
        gcmASSERT(0);
        return gcvSTATUS_INVALID_ARGUMENT;
     }
     break;

   case clvTYPE_CHAR:
   case clvTYPE_CHAR_PACKED:
     switch(SourceType) {
     case clvTYPE_FLOAT:
     case clvTYPE_HALF_PACKED:
        for(i=0; i < ValueCount; i++) {
           ToValues[i].intValue = (gctCHAR) SourceValues[i].floatValue;
        }
        break;

     case clvTYPE_BOOL:
     case clvTYPE_BOOL_PACKED:
        for(i=0; i < ValueCount; i++) {
           ToValues[i].intValue = (gctCHAR) SourceValues[i].boolValue;
        }
        break;

     case clvTYPE_INT:
     case clvTYPE_SHORT:
     case clvTYPE_SHORT_PACKED:
        for(i=0; i < ValueCount; i++) {
           ToValues[i].intValue = (gctCHAR) SourceValues[i].intValue;
        }
        break;

     case clvTYPE_LONG:
        for(i=0; i < ValueCount; i++) {
           ToValues[i].intValue = (gctCHAR) SourceValues[i].longValue;
        }
        break;

     case clvTYPE_UINT:
     case clvTYPE_USHORT:
     case clvTYPE_UCHAR:
     case clvTYPE_USHORT_PACKED:
     case clvTYPE_UCHAR_PACKED:
     case clvTYPE_EVENT_T:
     case clvTYPE_SAMPLER_T:
        for(i=0; i < ValueCount; i++) {
           ToValues[i].intValue = (gctCHAR) SourceValues[i].uintValue;
        }
        break;

     case clvTYPE_ULONG:
        for(i=0; i < ValueCount; i++) {
           ToValues[i].intValue = (gctCHAR) SourceValues[i].ulongValue;
        }
        break;

     case clvTYPE_CHAR:
     case clvTYPE_CHAR_PACKED:
        for(i=0; i < ValueCount; i++) {
           ToValues[i].intValue = (gctCHAR)SourceValues[i].intValue;
        }
    break;

     default:
        gcmASSERT(0);
        return gcvSTATUS_INVALID_ARGUMENT;
     }
   default:
     break;
   }
   return gcvSTATUS_OK;
}

gceSTATUS
clParseConstantTypeConvert(
IN cloIR_CONSTANT Constant,
IN cltELEMENT_TYPE ConversionType,
IN cluCONSTANT_VALUE *Result
)
{
   if(ConversionType == clmDATA_TYPE_elementType_GET(Constant->exprBase.decl.dataType)) {
      return gcvSTATUS_OK;
   }
   clParseConvertConstantValues(Constant->valueCount,
                                Constant->exprBase.decl.dataType->elementType,
                                Constant->values,
                                ConversionType,
                                Result);
   Constant->variable = gcvNULL;
   return gcvSTATUS_OK;
}

static gctBOOL
_CheckArrayDimDefined(
clsARRAY *Array
)
{
   int i;

   for (i = 0; i < Array->numDim; i++) {
      if(Array->length[i] > 0) continue;
      else return gcvFALSE;
   }
   return gcvTRUE;
}

static void
_ParseSetAliasVariable(
IN cloCOMPILER Compiler,
IN clsNAME *Name,
IN cloIR_EXPR AliasExpr
)
{
   cloIR_BINARY_EXPR binaryExpr;
   cloIR_UNARY_EXPR  unaryExpr;
   cloIR_CONSTANT  constant;

   switch(cloIR_OBJECT_GetType(&AliasExpr->base)) {
   case clvIR_BINARY_EXPR:
      binaryExpr = (cloIR_BINARY_EXPR) &AliasExpr->base;
      if(binaryExpr->type == clvBINARY_SUBSCRIPT &&
         cloIR_OBJECT_GetType(&binaryExpr->rightOperand->base) == clvIR_CONSTANT &&
         cloIR_OBJECT_GetType(&binaryExpr->leftOperand->base) == clvIR_VARIABLE) {
         clsNAME *alias;

         alias = ((cloIR_VARIABLE) &binaryExpr->leftOperand->base)->name;
         Name->u.variableInfo.alias = alias;
         Name->u.variableInfo.aliasOffset =
            cloIR_CONSTANT_GetIntegerValue((cloIR_CONSTANT) &binaryExpr->rightOperand->base);
      }
      break;

   case clvIR_UNARY_EXPR:
      unaryExpr = (cloIR_UNARY_EXPR) &AliasExpr->base;
      if(unaryExpr->type == clvUNARY_INDIRECTION &&
         cloIR_OBJECT_GetType(&unaryExpr->operand->base) == clvIR_VARIABLE) {
            Name->u.variableInfo.alias = ((cloIR_VARIABLE) &unaryExpr->operand->base)->name;
            Name->u.variableInfo.aliasOffset = 0;
      }
      break;

   case clvIR_VARIABLE:
      Name->u.variableInfo.alias = ((cloIR_VARIABLE) &AliasExpr->base)->name;
      Name->u.variableInfo.aliasOffset = 0;
      break;

   case clvIR_CONSTANT:
      constant = (cloIR_CONSTANT) &AliasExpr->base;
      if(constant->variable) {
        Name->u.variableInfo.alias = constant->variable;
        Name->u.variableInfo.aliasOffset = 0;
      }
      break;

   default:
      break;
   }
   return;
}

clsDeclOrDeclList *
clParseInitializeCurrentObject(
IN cloCOMPILER Compiler,
IN clsDeclOrDeclList *DeclOrDeclListPtr,
IN cloIR_EXPR InitExpr
)
{
  gceSTATUS status;
  cloIR_EXPR lhs;
  cloIR_EXPR resExpr;
  cloIR_BASE    initStatement;
  cloIR_CONSTANT constant = gcvNULL;
  cloIR_TYPECAST_ARGS typeCastArgs;
  gctINT arraySize;
  cluCONSTANT_VALUE *valStart;
  clsNAME *name;
  cloIR_EXPR initExpr;

  if(DeclOrDeclListPtr->name == gcvNULL || InitExpr == gcvNULL) return DeclOrDeclListPtr;

  lhs = DeclOrDeclListPtr->lhs;
  name = DeclOrDeclListPtr->name;
  if(name->decl.dataType->addrSpaceQualifier == clvQUALIFIER_LOCAL &&
     !clmDECL_IsPointerType(&name->decl)) {
      gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                      InitExpr->base.lineNo,
                                      InitExpr->base.stringNo,
                                      clvREPORT_ERROR,
                                      "local address space variables cannot be initialized"));
      return DeclOrDeclListPtr;
  }
  initExpr = InitExpr;
  switch(cloIR_OBJECT_GetType(&initExpr->base)) {
  case clvIR_TYPECAST_ARGS:
     typeCastArgs = (cloIR_TYPECAST_ARGS) &initExpr->base;
     arraySize = _GetArraySize(typeCastArgs,
                               &name->decl,
                               0,
                               clmDECL_IsArray(&name->decl) ?
                               &name->decl.array : (clsARRAY *) gcvNULL);
     if(arraySize > 0) { /* all constants */
        if (clmDECL_IsArray(&name->decl)) {
           if(_CheckArrayDimDefined(&name->decl.array)) {
              if(lhs) lhs->decl.array = name->decl.array;
           }
           else {
              gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                              InitExpr->base.lineNo,
                                              InitExpr->base.stringNo,
                                              clvREPORT_ERROR,
                                              "size of imcomplete array type cannot be determined"));
              return DeclOrDeclListPtr;
           }
        }
        /* Allocate the constant with right sized buffer */
        status = cloIR_CONSTANT_Allocate(Compiler,
                                         InitExpr->base.lineNo,
                                         InitExpr->base.stringNo,
                                         &name->decl,
                                         &constant);
        if (gcmIS_ERROR(status)) return DeclOrDeclListPtr;

        if((_GEN_UNIFORMS_FOR_CONSTANT_ADDRESS_SPACE_VARIABLES &&
            (clmDECL_IsAggregateType(&name->decl) ||
             clmDATA_TYPE_IsHighPrecision(name->decl.dataType))) ||
            (clmDECL_IsUnderlyingStructOrUnion(&name->decl) &&
             ((clGetOperandCountForRegAlloc(&name->decl) > _clmMaxOperandCountToUseMemory(&constant->exprBase.decl)) ||
             name->u.variableInfo.isAddressed))) {
           gctSIZE_T written;
           clsVARIABLE_NESTING *nesting = gcvNULL;

           _ParseLocationMapSize = _ParseGetLocationMap(Compiler,
                                                        &name->decl,
                                                        &_ParseLocationMap);
           if(!_ParseLocationMapSize) {
              gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                              InitExpr->base.lineNo,
                                              InitExpr->base.stringNo,
                                              clvREPORT_ERROR,
                                              "internal error: failed to create location map"));
              return DeclOrDeclListPtr;
           }
           else {
              _ParseEndLocationMap = _ParseLocationMap + _ParseLocationMapSize - 1;
           }

           _ParseConstantBuffer = constant->buffer;
           _clmParseGetNextNesting(_ParseLocationMap, nesting);
           gcmASSERT(nesting);
           written = _MakeStructOrUnionConstant(Compiler,
                                                typeCastArgs,
                                                nesting,
                                                _ParseLocationMap,
                                                _ParseLocationMapSize);
           _clmParseFreeLocationMap(Compiler,
                                    _ParseLocationMap,
                                    _ParseLocationMapSize);
           _ParseLocationMap = gcvNULL;
           _ParseLocationMapSize = 0;

           if(written <= 0) {
              return DeclOrDeclListPtr;
           }
        }
        else {
           status = _MakeTypeCastArgsAsConstant(Compiler,
                                                typeCastArgs,
                                                0,
                                                &constant->exprBase.decl,
                                                constant->values,
                                                constant->values + constant->valueCount);
           if (gcmIS_ERROR(status)) return DeclOrDeclListPtr;
           cloIR_CONSTANT_CheckAndSetAllValuesEqual(Compiler,
                                                    constant);
        }

        initExpr = &constant->exprBase;
     }
     else if(arraySize < 0) {
        gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                        InitExpr->base.lineNo,
                                        InitExpr->base.stringNo,
                                        clvREPORT_ERROR,
                                        "number of initializers exceeds type defined"));
        return DeclOrDeclListPtr;
     }
     else {
       if(clmDECL_IsArray(&name->decl)) {
         gctUINT operandCount;
         gcmVERIFY_OK(cloIR_SET_GetMemberCount(Compiler,
                                               typeCastArgs->operands,
                                               &operandCount));
         if(name->decl.array.length[0] < 0) {
           name->decl.array.length[0] = operandCount;
           if(lhs)lhs->decl.array.length[0] = name->decl.array.length[0];
         }
         else {
           gctUINT elementCount;
           clmGetArrayElementCount(name->decl.array, 0, elementCount);
           if(elementCount < operandCount) {
             gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                             InitExpr->base.lineNo,
                                             InitExpr->base.stringNo,
                                             clvREPORT_ERROR,
                                             "number of initializers exceeds type defined"));
               return DeclOrDeclListPtr;
            }
          }
        }
        initExpr =  _ParseConvTypeCastToPolynaryExpr(Compiler,
                                                     &name->decl,
                                                     typeCastArgs);
     }
     break;

  case clvIR_POLYNARY_EXPR:
     {
         cloIR_CONSTANT constantExpr = gcvNULL;

         status = cloIR_POLYNARY_EXPR_Evaluate(Compiler,
                                               (cloIR_POLYNARY_EXPR) &initExpr->base,
                                               &constantExpr);

         if (gcmIS_SUCCESS(status) && constantExpr != gcvNULL) {
             initExpr = &constantExpr->exprBase;
         }
     }
     break;

  case clvIR_CONSTANT:
     constant = (cloIR_CONSTANT) &initExpr->base;
     break;

  default:
     break;
  }

  if (clmDECL_IsArray(&name->decl)) {
    if(name->decl.array.length[0] < 0) {
      if(cloIR_OBJECT_GetType(&initExpr->base) == clvIR_POLYNARY_EXPR) {
         gctUINT operandCount;
         gcmVERIFY_OK(cloIR_SET_GetMemberCount(Compiler,
                                               ((cloIR_POLYNARY_EXPR)(&initExpr->base))->operands,
                                               &operandCount));
         name->decl.array.length[0] = operandCount;
      }
      else {
         status = _CheckConstantExpr(Compiler, initExpr);
         if (gcmIS_ERROR(status)) {
            name->decl.array.length[0] = 0;
            return DeclOrDeclListPtr;
         }
         name->decl.array.length[0] = ((cloIR_CONSTANT)&initExpr->base)->valueCount;
      }
      if(lhs)lhs->decl.array.length[0] = name->decl.array.length[0];
    }
  }
  if (cloIR_OBJECT_GetType(&initExpr->base) == clvIR_CONSTANT &&
      clmDECL_IsPointerType(&name->decl) &&
      ((cloIR_CONSTANT)initExpr)->valueCount > 1) {
      cloIR_EXPR constVariableExpr;
      if(!clsDECL_IsInitializableTo(&name->decl, &initExpr->decl)) {
         gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                         initExpr->base.lineNo,
                                         initExpr->base.stringNo,
                                         clvREPORT_ERROR,
                                         "type mismatch between initializers and defined type"));
         return DeclOrDeclListPtr;
      }
      status = _MakeConstantVariableExpr(Compiler,
                                         (cloIR_CONSTANT)initExpr,
                                         &constVariableExpr);
      if (gcmIS_ERROR(status)) return gcvNULL;
      initExpr = constVariableExpr;
  }
  else {
      clsNAME *unnamedConstant = gcvNULL;

      if (cloIR_OBJECT_GetType(&initExpr->base) == clvIR_VARIABLE) {
          cloIR_VARIABLE variable = (cloIR_VARIABLE) &initExpr->base;

          if(variable->name->u.variableInfo.isUnnamedConstant &&
             cloCOMPILER_IsNameSpaceGlobal(Compiler, name->mySpace)) {
              unnamedConstant = variable->name;
              gcmASSERT(unnamedConstant->u.variableInfo.u.constant);
              initExpr = &unnamedConstant->u.variableInfo.u.constant->exprBase;
          }
      }
      if (cloIR_OBJECT_GetType(&initExpr->base) == clvIR_CONSTANT &&
          (name->decl.dataType->accessQualifier == clvQUALIFIER_CONST ||
           (!clmDECL_IsPointerType(&name->decl) &&
            (!clmDECL_IsElementScalar(&name->decl) ||
             clmDATA_TYPE_IsHighPrecision(name->decl.dataType))))) {
         name->u.variableInfo.u.constant = gcvNULL;
         if(!clsDECL_IsInitializableTo(&name->decl, &initExpr->decl)) {
            gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                            initExpr->base.lineNo,
                                            initExpr->base.stringNo,
                                            clvREPORT_ERROR,
                                            "type mismatch between initializers and defined type"));
            return DeclOrDeclListPtr;
         }
         else {
            if(name->decl.dataType->elementType != initExpr->decl.dataType->elementType) {
               clsDATA_TYPE dataType[1];

               valStart = ((cloIR_CONSTANT) initExpr)->values;

               status = clParseConstantTypeConvert((cloIR_CONSTANT)(&initExpr->base),
                                                   name->decl.dataType->elementType,
                                                   valStart);
               if (gcmIS_ERROR(status)) {
                  gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                                  InitExpr->base.lineNo,
                                                  InitExpr->base.stringNo,
                                                  clvREPORT_ERROR,
                                                  "type mismatch between initializers and defined type"));
                  return DeclOrDeclListPtr;
               }
               *dataType = *initExpr->decl.dataType;
               if(clmDECL_IsScalar(&initExpr->decl)) {
                   if(clmDECL_IsArithmeticType(&name->decl)) {
                       dataType->type = clGetVectorTerminalToken(name->decl.dataType->elementType, 1);
                       if(clmDECL_IsPackedType(&name->decl)) {
                           clsBUILTIN_DATATYPE_INFO *typeInfo = clGetBuiltinDataTypeInfo(dataType->type);
                           dataType->type = typeInfo->dualType;
                       }
                   }
               }
               else {
                   dataType->type = name->decl.dataType->type;
               }
               status = cloCOMPILER_CloneDataType(Compiler,
                                                  initExpr->decl.dataType->accessQualifier,
                                                  initExpr->decl.dataType->addrSpaceQualifier,
                                                  dataType,
                                                  &initExpr->decl.dataType);
               if (gcmIS_ERROR(status)) return DeclOrDeclListPtr;
            }
            if(name->decl.dataType->accessQualifier == clvQUALIFIER_CONST) {
               cloIR_CONSTANT constantExpr = (cloIR_CONSTANT)(&initExpr->base);

               if(clmDECL_IsScalar(&constantExpr->exprBase.decl) && !clmDECL_IsScalar(&name->decl)) {
                   status = cloCOMPILER_CloneDecl(Compiler,
                                                  initExpr->decl.dataType->accessQualifier,
                                                  initExpr->decl.dataType->addrSpaceQualifier,
                                                  &name->decl,
                                                  &constantExpr->exprBase.decl);
                   if (gcmIS_ERROR(status)) return DeclOrDeclListPtr;

                   constantExpr->allValuesEqual = gcvTRUE;
               }

               name->u.variableInfo.u.constant = constantExpr;

               name->u.variableInfo.u.constant->variable = name;
               if(unnamedConstant) {
                   gctINT memoryOffset;

                   /* Assume the properties of the unnamed constant to the lhs */
                   memoryOffset = clmNAME_VariableMemoryOffset_GET(unnamedConstant);
                   clmNAME_VariableMemoryOffset_SET(name, memoryOffset);
                   name->u.variableInfo.allocated = unnamedConstant->u.variableInfo.allocated;
                   name->u.variableInfo.isAddressed = unnamedConstant->u.variableInfo.isAddressed;
                   return DeclOrDeclListPtr;
               }
               else if(_GEN_UNIFORMS_FOR_CONSTANT_ADDRESS_SPACE_VARIABLES &&
                  (clmDECL_IsAggregateType(&name->decl) ||
                   clmDATA_TYPE_IsHighPrecision(name->decl.dataType))) {
                   status = cloCOMPILER_AllocateVariableMemory(Compiler,
                                                               name);
                   return DeclOrDeclListPtr;
               }
               /* if constant variable's elements are not scalar, make this to be allocated in driver */
               else if (!clmDECL_IsPointerType(&name->decl) &&
                   (clmDECL_IsAggregateTypeOverRegLimit(&name->decl) ||
                    (clmDECL_IsExtendedVectorType(&name->decl) &&
                     (clmDECL_IsPackedType(&name->decl) || !cloCOMPILER_ExtensionEnabled(Compiler, clvEXTENSION_VIV_VX))))) {
                 /* force constant variable to be in constant address space */
                 status = cloCOMPILER_CloneDataType(Compiler,
                                                    name->decl.dataType->accessQualifier,
                                                    clvQUALIFIER_CONSTANT,
                                                    name->decl.dataType,
                                                    &name->decl.dataType);
                 if (gcmIS_ERROR(status)) return DeclOrDeclListPtr;

                 status =  clsNAME_SetVariableAddressed(Compiler,
                                                        name);
                 return DeclOrDeclListPtr;
               }
               else goto AssignLhs;
            }

#if _CREATE_UNNAMED_CONSTANT_IN_MEMORY
            if((_GEN_UNIFORMS_FOR_CONSTANT_ADDRESS_SPACE_VARIABLES && constant &&
                (clmDATA_TYPE_IsHighPrecision(constant->exprBase.decl.dataType) ||
                 clmDECL_IsAggregateType(&constant->exprBase.decl))) ||
                (!clmDECL_IsScalar(&initExpr->decl) &&
                 (clmDECL_IsAggregateTypeOverRegLimit(&name->decl) ||
                  (constant && clmDECL_IsAggregateTypeOverRegLimit(&constant->exprBase.decl)) ||
                  (clmDECL_IsExtendedVectorType(&name->decl) &&
                   (clmDECL_IsPackedType(&name->decl) || !cloCOMPILER_ExtensionEnabled(Compiler, clvEXTENSION_VIV_VX)))))) {
               status = _CreateUnnamedConstantExpr(Compiler,
                                                   &name->decl,
                                                   (cloIR_CONSTANT) (&initExpr->base),
                                                   &initExpr);

               if (gcmIS_ERROR(status)) return DeclOrDeclListPtr;

               if(clmDECL_IsExtendedVectorType(&name->decl) && !cloCOMPILER_ExtensionEnabled(Compiler, clvEXTENSION_VIV_VX) &&
                  !((cloIR_CONSTANT) (&initExpr->base))->allValuesEqual) {
                   name->u.variableInfo.isInitializedWithExtendedVectorConstant = gcvTRUE;
               }
            }
#endif
         }
      }
  }
  {
    gcmASSERT(lhs);

    status = _CheckInitializationExpr(Compiler,
                                      lhs,
                                      initExpr);
    if (gcmIS_ERROR(status)) return DeclOrDeclListPtr;
    if(clmDECL_IsUnderlyingStructOrUnion(&name->decl) &&
       clGetOperandCountForRegAlloc(&name->decl) > _clmMaxOperandCountToUseMemory(&name->decl)) {
       clsNAME_SetVariableAddressed(Compiler,
                                    name);
    }
    else {
       _ParseSetAliasVariable(Compiler,
                              name,
                              initExpr);
    }

    if(cloCOMPILER_IsExternSymbolsAllowed(Compiler) &&
       !clmDECL_IsPointerType(&name->decl) &&
       !(name->decl.storageQualifier & clvSTORAGE_QUALIFIER_STATIC) &&
       name->decl.dataType->addrSpaceQualifier == clvQUALIFIER_CONSTANT) {
        status = cloCOMPILER_AllocateVariableMemory(Compiler,
                                                    name);
        if (gcmIS_ERROR(status)) return DeclOrDeclListPtr;
        /* clear the extern storage qualifier as it has been initialized */
        name->decl.storageQualifier &= ~clvSTORAGE_QUALIFIER_EXTERN;
    }

AssignLhs:
    status = cloIR_BINARY_EXPR_Construct(Compiler,
                                         lhs->base.lineNo,
                                         lhs->base.stringNo,
                                         clvBINARY_ASSIGN,
                                         lhs,
                                         initExpr,
                                         &resExpr);
    if (gcmIS_ERROR(status)) return DeclOrDeclListPtr;

    initStatement = &resExpr->base;

    if (DeclOrDeclListPtr->initStatements != gcvNULL) {
       gcmVERIFY_OK(cloIR_SET_AddMember(Compiler,
                                        DeclOrDeclListPtr->initStatements,
                                        initStatement));
    }
    else {
      gcmASSERT(DeclOrDeclListPtr->initStatement == gcvNULL);
      DeclOrDeclListPtr->initStatement = initStatement;
    }
    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                                  clvDUMP_PARSER,
                                  "<VARIABLE_DECL_WITH_INITIALIZER line=\"%d\" string=\"%d\""
                                  " dataType=\"0x%x\" identifier=\"%s\" initializer=\"0x%x\" />",
                                  name->lineNo,
                                  name->stringNo,
                                  name->decl.dataType,
                                  name->symbol,
                                  initExpr));
  }
  return DeclOrDeclListPtr;
}

static gceSTATUS
_ParseVariableDeclInit(
IN cloCOMPILER Compiler,
IN clsDeclOrDeclList *DeclOrDeclListPtr,
IN clsLexToken *Identifier
)
{
    gceSTATUS status;
    clsDECL decl[1];
    clsDECL *declPtr;
    clsNAME *derivedType = gcvNULL;

    gcmASSERT(DeclOrDeclListPtr->decl.dataType);

    switch(DeclOrDeclListPtr->decl.dataType->type) {
    case T_TYPE_NAME:
        status = _ParseFlattenType(Compiler, &DeclOrDeclListPtr->decl, decl);
        if(gcmIS_ERROR(status)) return status;
        declPtr = decl;
        derivedType = DeclOrDeclListPtr->decl.dataType->u.typeDef;
        break;

    case T_ENUM:
        declPtr = &DeclOrDeclListPtr->decl;
        derivedType = DeclOrDeclListPtr->decl.dataType->u.enumerator;
        break;

    default:
        declPtr = _HandleSpecialType(Compiler, &DeclOrDeclListPtr->decl);
        break;
    }

    if (clmDATA_TYPE_IsSampler(declPtr->dataType)) {
        if(declPtr->dataType->accessQualifier != clvQUALIFIER_CONST) {
/*KLC force it to constant as conformance test did not declare it as constant*/
            status = cloCOMPILER_CloneDataType(Compiler,
                               clvQUALIFIER_CONST,
                               declPtr->dataType->addrSpaceQualifier,
                               declPtr->dataType,
                               &declPtr->dataType);
            if (gcmIS_ERROR(status)) return status;
        }
    }

    if(declPtr->dataType->type == T_IMAGE2D_DYNAMIC_ARRAY_T) {
        gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                        Identifier->lineNo,
                        Identifier->stringNo,
                        clvREPORT_ERROR,
                        "unrecognizable type '_viv_image2d_array_t' specified for variable '%s'",
                        Identifier->u.identifier.name));
        return gcvSTATUS_INVALID_ARGUMENT;
    }

    if(declPtr->dataType->type == T_GENTYPE_PACKED) {
        gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                        Identifier->lineNo,
                        Identifier->stringNo,
                        clvREPORT_ERROR,
                        "unrecognizable type '_viv_gentype_packed' specified for variable '%s'",
                        Identifier->u.identifier.name));
        return gcvSTATUS_INVALID_ARGUMENT;
    }

    if (clmDATA_TYPE_IsImage(declPtr->dataType)) {
        gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                        Identifier->lineNo,
                        Identifier->stringNo,
                        clvREPORT_ERROR,
                        "variable '%s' cannot have image type",
                        Identifier->u.identifier.name));
        return gcvSTATUS_INVALID_ARGUMENT;
    }

    /* Create the name */
    status = cloCOMPILER_CreateName(Compiler,
                    Identifier->lineNo,
                    Identifier->stringNo,
                    clvVARIABLE_NAME,
                    declPtr,
                    Identifier->u.identifier.name,
                    Identifier->u.identifier.ptrDscr,
                    clvEXTENSION_NONE,
                    &DeclOrDeclListPtr->name);
    if (gcmIS_ERROR(status)) return status;

    DeclOrDeclListPtr->name->derivedType = derivedType;
    if(cloCOMPILER_GetParserState(Compiler) != clvPARSER_IN_TYPEDEF) {
        status = _ParseCheckVariableNeedMemory(Compiler,
                                               DeclOrDeclListPtr->name);
        if (gcmIS_ERROR(status)) return status;
    }

    return status;
}

clsDeclOrDeclList *
clParseVariableDeclInit(
IN cloCOMPILER Compiler,
IN clsDECL * Decl,
IN clsLexToken * Identifier,
IN clsATTRIBUTE *Attr
)
{
    gceSTATUS status;
    clsDeclOrDeclList *declOrDeclListPtr;
    gctPOINTER pointer;

    gcmASSERT(Identifier);
    gcmASSERT(Decl);
    status = cloCOMPILER_Allocate(Compiler,
                      (gctSIZE_T)sizeof(clsDeclOrDeclList),
                      (gctPOINTER *) &pointer);

    if (gcmIS_ERROR(status)) return gcvNULL;
    declOrDeclListPtr = pointer;

    declOrDeclListPtr->decl    = *Decl;
    declOrDeclListPtr->name    = gcvNULL;
    declOrDeclListPtr->lhs    = gcvNULL;
    declOrDeclListPtr->designator= gcvNULL;
    declOrDeclListPtr->initStatement= gcvNULL;
    declOrDeclListPtr->initStatements= gcvNULL;

    if (declOrDeclListPtr->decl.dataType == gcvNULL) {
        return declOrDeclListPtr;
    }

    status =  _ParseVariableDeclInit(Compiler,
                         declOrDeclListPtr,
                         Identifier);
    if (gcmIS_ERROR(status)) {
        return declOrDeclListPtr;
    }

    status = _ParseFillVariableAttr(Compiler,
                                    Identifier->lineNo,
                                    Identifier->stringNo,
                                    Decl, declOrDeclListPtr->name, Attr);
    if (gcmIS_ERROR(status)) {
        return declOrDeclListPtr;
    }

    /* Create the lhs for assignment */
    _clmParseCreateLhs(Compiler,
               Identifier->lineNo,
               Identifier->stringNo,
               declOrDeclListPtr,
               status);
    return declOrDeclListPtr;
}

clsDeclOrDeclList *
clParseVariableDeclListInit(
IN cloCOMPILER Compiler,
IN clsDeclOrDeclList *DeclOrDeclListPtr,
IN clsLexToken * Identifier,
IN clsATTRIBUTE *Attr
)
{
    gceSTATUS        status;
    clsDeclOrDeclList    *declOrDeclListPtr;

    gcmASSERT(Identifier);

    declOrDeclListPtr = DeclOrDeclListPtr;
    if (declOrDeclListPtr->decl.dataType == gcvNULL) {
        return declOrDeclListPtr;
    }
    if (declOrDeclListPtr->initStatement != gcvNULL) {
        gcmASSERT(declOrDeclListPtr->initStatements == gcvNULL);

        status = cloIR_SET_Construct(Compiler,
                         declOrDeclListPtr->initStatement->lineNo,
                         declOrDeclListPtr->initStatement->stringNo,
                         clvDECL_SET,
                         &declOrDeclListPtr->initStatements);
        if (gcmIS_ERROR(status)) {
            gcmASSERT(declOrDeclListPtr->initStatements == gcvNULL);
            return declOrDeclListPtr;
        }

        gcmVERIFY_OK(cloIR_SET_AddMember(Compiler,
                         declOrDeclListPtr->initStatements,
                         declOrDeclListPtr->initStatement));
        declOrDeclListPtr->initStatement = gcvNULL;
    }

    if(declOrDeclListPtr->name->type == clvTYPE_NAME) {
        gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                        Identifier->lineNo,
                                        Identifier->stringNo,
                                        clvREPORT_ERROR,
                                        "illegal typedef initialization"));
        return declOrDeclListPtr;
    }

    status =  _ParseVariableDeclInit(Compiler,
                                     declOrDeclListPtr,
                                     Identifier);
    if (gcmIS_ERROR(status)) {
        return declOrDeclListPtr;
    }

    status = _ParseFillVariableAttr(Compiler,
                                    Identifier->lineNo,
                                    Identifier->stringNo,
                                    &declOrDeclListPtr->decl, declOrDeclListPtr->name, Attr);
    if (gcmIS_ERROR(status)) {
        return declOrDeclListPtr;
    }

    /* Create the lhs for assignment */
    _clmParseCreateLhs(Compiler,
               Identifier->lineNo,
               Identifier->stringNo,
               declOrDeclListPtr,
               status);
    return declOrDeclListPtr;
}

clsDeclOrDeclList *
clParseArrayVariableDeclInit(
IN cloCOMPILER Compiler,
IN clsDECL * Decl,
IN clsLexToken * Identifier,
IN cloIR_EXPR ArrayLengthExpr,
IN clsATTRIBUTE *Attr
)
{
    gceSTATUS status;
    clsDeclOrDeclList *declOrDeclListPtr;
    gctPOINTER pointer;

    status = cloCOMPILER_Allocate(Compiler,
                    (gctSIZE_T)sizeof(clsDeclOrDeclList),
                    (gctPOINTER *) &pointer);

    if (gcmIS_ERROR(status)) return gcvNULL;
    declOrDeclListPtr = pointer;

    declOrDeclListPtr->decl = *Decl;
    declOrDeclListPtr->name    = gcvNULL;
    declOrDeclListPtr->lhs    = gcvNULL;
    declOrDeclListPtr->designator= gcvNULL;
    declOrDeclListPtr->initStatement= gcvNULL;
    declOrDeclListPtr->initStatements= gcvNULL;

    if (declOrDeclListPtr->decl.dataType == gcvNULL) return declOrDeclListPtr;

    status = _ParseArrayVariableDeclInit(Compiler,
                         declOrDeclListPtr,
                         Identifier,
                         ArrayLengthExpr);
    if (gcmIS_ERROR(status)) {
        return declOrDeclListPtr;
    }

    status = _ParseFillVariableAttr(Compiler,
                                    Identifier->lineNo,
                                    Identifier->stringNo,
                                    Decl, declOrDeclListPtr->name, Attr);
    if (gcmIS_ERROR(status)) {
        return declOrDeclListPtr;
    }

    /* Create the lhs for assignment */
    _clmParseCreateLhs(Compiler,
               Identifier->lineNo,
               Identifier->stringNo,
               declOrDeclListPtr,
               status);
    return declOrDeclListPtr;
}

clsDeclOrDeclList *
clParseArrayVariableDeclListInit(
IN cloCOMPILER Compiler,
IN clsDeclOrDeclList *DeclOrDeclListPtr,
IN clsLexToken * Identifier,
IN cloIR_EXPR ArrayLengthExpr,
IN clsATTRIBUTE *Attr
)
{
    gceSTATUS        status;
    clsDeclOrDeclList    *declOrDeclListPtr;

    gcmASSERT(Identifier);

    declOrDeclListPtr = DeclOrDeclListPtr;
    if (declOrDeclListPtr->decl.dataType == gcvNULL) {
        return declOrDeclListPtr;
    }
    if (declOrDeclListPtr->initStatement != gcvNULL) {
        gcmASSERT(declOrDeclListPtr->initStatements == gcvNULL);

        status = cloIR_SET_Construct(Compiler,
                         declOrDeclListPtr->initStatement->lineNo,
                         declOrDeclListPtr->initStatement->stringNo,
                         clvDECL_SET,
                         &declOrDeclListPtr->initStatements);
        if (gcmIS_ERROR(status)) {
            gcmASSERT(declOrDeclListPtr->initStatements == gcvNULL);
            return declOrDeclListPtr;
        }

        gcmVERIFY_OK(cloIR_SET_AddMember(Compiler,
                         declOrDeclListPtr->initStatements,
                         declOrDeclListPtr->initStatement));
        declOrDeclListPtr->initStatement = gcvNULL;
    }

    if(declOrDeclListPtr->name->type == clvTYPE_NAME) {
        gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                        Identifier->lineNo,
                                        Identifier->stringNo,
                                        clvREPORT_ERROR,
                                        "illegal typedef initialization"));
        return declOrDeclListPtr;
    }

    status = _ParseArrayVariableDeclInit(Compiler,
                                         declOrDeclListPtr,
                                         Identifier,
                                         ArrayLengthExpr);
    if (gcmIS_ERROR(status)) {
        return declOrDeclListPtr;
    }

    status = _ParseFillVariableAttr(Compiler,
                                    Identifier->lineNo,
                                    Identifier->stringNo,
                                    &declOrDeclListPtr->decl, declOrDeclListPtr->name, Attr);
    if (gcmIS_ERROR(status)) {
        return declOrDeclListPtr;
    }

    /* Create the lhs for assignment */
    _clmParseCreateLhs(Compiler,
               Identifier->lineNo,
               Identifier->stringNo,
               declOrDeclListPtr,
               status);

    return declOrDeclListPtr;
}

clsNAME    *
clParseFuncHeader(
IN cloCOMPILER Compiler,
IN clsDECL *Decl,
IN clsLexToken *Identifier
)
{
    gceSTATUS status;
    clsDECL   decl[1];
    clsDECL   *declPtr;
    clsNAME   * name;
    clsNAME   *derivedType = gcvNULL;

    gcmASSERT(Identifier);
    if (Decl->dataType == gcvNULL) return gcvNULL;

    switch(Decl->dataType->type) {
    case T_TYPE_NAME:
        status = _ParseFlattenType(Compiler, Decl, decl);
        if (gcmIS_ERROR(status)) return gcvNULL;
        declPtr = decl;
        derivedType = Decl->dataType->u.typeDef;
        break;

    case T_ENUM:
        declPtr = Decl;
        derivedType = Decl->dataType->u.enumerator;
        break;

    default:
        declPtr = _HandleSpecialType(Compiler, Decl);
        break;
    }

    status = cloCOMPILER_CreateName(Compiler,
                    Identifier->lineNo,
                    Identifier->stringNo,
                    clvFUNC_NAME,
                    declPtr,
                    Identifier->u.identifier.name,
                    Identifier->u.identifier.ptrDscr,
                    clvEXTENSION_NONE,
                    &name);
    if (gcmIS_ERROR(status)) return gcvNULL;

    name->derivedType = derivedType;
    _ParseFillVariableAttr(Compiler,
                           Identifier->lineNo,
                           Identifier->stringNo,
                           declPtr, name, gcvNULL);

    status = cloCOMPILER_CreateNameSpace(Compiler, &name->u.funcInfo.localSpace);
    if (gcmIS_ERROR(status)) return gcvNULL;
    name->u.funcInfo.localSpace->scopeName = name;
    name->u.funcInfo.localSpace->die = name->die;
    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                clvDUMP_PARSER,
                "<FUNCTION line=\"%d\" string=\"%d\" name=\"%s\">",
                Identifier->lineNo,
                Identifier->stringNo,
                Identifier->u.identifier.name));
    return name;
}

clsNAME    *
clParseFuncHeaderWithAttr(
IN cloCOMPILER Compiler,
IN clsATTRIBUTE *Attr,
IN clsDECL *Decl,
IN clsLexToken *Identifier
)
{
   clsNAME *funcName;

   gcmASSERT(Attr);
   funcName = clParseFuncHeader(Compiler,
                                Decl,
                                Identifier);
   if(funcName) {
     if(Attr->specifiedAttr & clvATTR_ALWAYS_INLINE) {
        funcName->u.funcInfo.isInline = gcvTRUE;
     }
   }
   return funcName;
}

clsNAME    *
clParseKernelFuncHeader(
IN cloCOMPILER Compiler,
IN clsATTRIBUTE *Attr,
IN clsDECL *Decl,
IN clsLexToken *Identifier
)
{
   gceSTATUS  status;
   clsDECL    decl[1];
   clsDECL    *declPtr;
   clsNAME    *name;
   clsNAME    *derivedType = gcvNULL;

   gcmASSERT(Identifier);
   if (Decl->dataType == gcvNULL) return gcvNULL;

    switch(Decl->dataType->type) {
    case T_TYPE_NAME:
        status = _ParseFlattenType(Compiler, Decl, decl);
        if (gcmIS_ERROR(status)) return gcvNULL;
        declPtr = decl;
        derivedType = Decl->dataType->u.typeDef;
        break;

    case T_ENUM:
        declPtr = Decl;
        derivedType = Decl->dataType->u.enumerator;
        break;

    default:
        declPtr = _HandleSpecialType(Compiler, Decl);
        break;
    }

   status = cloCOMPILER_CreateName(Compiler,
                                   Identifier->lineNo,
                                   Identifier->stringNo,
                                   clvKERNEL_FUNC_NAME,
                                   declPtr,
                                   Identifier->u.identifier.name,
                                   Identifier->u.identifier.ptrDscr,
                                   clvEXTENSION_NONE,
                                   &name);
   if (gcmIS_ERROR(status)) return gcvNULL;

   name->derivedType = derivedType;
   status = cloCOMPILER_CreateNameSpace(Compiler, &name->u.funcInfo.localSpace);
   name->u.funcInfo.localSpace->die = name->die;
   if (gcmIS_ERROR(status)) return gcvNULL;
   name->u.funcInfo.localSpace->scopeName = name;
   if(Attr) {
     name->u.funcInfo.attrQualifier.attrFlags = Attr->specifiedAttr;
     if(Attr->specifiedAttr & clvATTR_REQD_WORK_GROUP_SIZE)  {
        name->u.funcInfo.attrQualifier.reqdWorkGroupSize[0] = Attr->reqdWorkGroupSize[0];
        name->u.funcInfo.attrQualifier.reqdWorkGroupSize[1] = Attr->reqdWorkGroupSize[1];
        name->u.funcInfo.attrQualifier.reqdWorkGroupSize[2] = Attr->reqdWorkGroupSize[2];
     }

     if(Attr->specifiedAttr & clvATTR_WORK_GROUP_SIZE_HINT)  {
        name->u.funcInfo.attrQualifier.workGroupSizeHint[0] = Attr->workGroupSizeHint[0];
        name->u.funcInfo.attrQualifier.workGroupSizeHint[1] = Attr->workGroupSizeHint[1];
        name->u.funcInfo.attrQualifier.workGroupSizeHint[2] = Attr->workGroupSizeHint[2];
     }

     if(Attr->specifiedAttr & clvATTR_KERNEL_SCALE_HINT)  {
        name->u.funcInfo.attrQualifier.kernelScaleHint[0] = Attr->kernelScaleHint[0];
        name->u.funcInfo.attrQualifier.kernelScaleHint[1] = Attr->kernelScaleHint[1];
        name->u.funcInfo.attrQualifier.kernelScaleHint[2] = Attr->kernelScaleHint[2];
     }

     if(Attr->specifiedAttr & clvATTR_VEC_TYPE_HINT)  {
        name->u.funcInfo.attrQualifier.vecTypeHint = Attr->vecTypeHint;
     }
     gcmVERIFY_OK(cloCOMPILER_Free(Compiler, Attr));
   }

   gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                                 clvDUMP_PARSER,
                                 "<FUNCTION line=\"%d\" string=\"%d\" name=\"%s\">",
                                 Identifier->lineNo,
                                 Identifier->stringNo,
                                 Identifier->u.identifier.name));
   return name;
}

clsNAME    *
clParseExternKernelFuncHeader(
IN cloCOMPILER Compiler,
IN clsATTRIBUTE *Attr,
IN clsDECL *Decl,
IN clsLexToken *Identifier
)
{
   clsNAME    *name;

   name = clParseKernelFuncHeader(Compiler,
                                  Attr,
                                  Decl,
                                  Identifier);
   if(!name) return gcvNULL;

   name->decl.storageQualifier |= clvSTORAGE_QUALIFIER_EXTERN;
   gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                                 clvDUMP_PARSER,
                                 "<FUNCTION line=\"%d\" string=\"%d\" name=\"%s\">",
                                 Identifier->lineNo,
                                 Identifier->stringNo,
                                 Identifier->u.identifier.name));
   return name;
}

clsDeclOrDeclList *
clParseFuncDecl(
IN cloCOMPILER Compiler,
IN clsNAME *FuncName
)
{
    gceSTATUS status;
    clsDeclOrDeclList *declOrDeclListPtr;
    gctPOINTER pointer;
    clsNAME *firstFuncName;

    gcmASSERT(FuncName);
    status = cloCOMPILER_Allocate(Compiler,
                    (gctSIZE_T)sizeof(clsDeclOrDeclList),
                    (gctPOINTER *) &pointer);

    if (gcmIS_ERROR(status)) return gcvNULL;
    declOrDeclListPtr = pointer;

    declOrDeclListPtr->decl.dataType = FuncName->decl.dataType;
    declOrDeclListPtr->name        = gcvNULL;
    declOrDeclListPtr->lhs        = gcvNULL;
    declOrDeclListPtr->designator= gcvNULL;
    declOrDeclListPtr->initStatement= gcvNULL;
    declOrDeclListPtr->initStatements= gcvNULL;

    if (declOrDeclListPtr->decl.dataType == gcvNULL) return declOrDeclListPtr;

    cloCOMPILER_PopCurrentNameSpace(Compiler, gcvNULL);

    FuncName->u.funcInfo.isFuncDef = gcvFALSE;
    FuncName->u.funcInfo.refCount = 0;
    status = cloCOMPILER_CheckNewFuncName(Compiler,
                          FuncName,
                          &firstFuncName);
    if (gcmIS_ERROR(status)) {
        return declOrDeclListPtr;
    }

    if (FuncName != firstFuncName) {
            status = clsNAME_SPACE_ReleaseName(Compiler,
                                               cloCOMPILER_GetCurrentSpace(Compiler),
                                               FuncName);
            if (gcmIS_ERROR(status)) return declOrDeclListPtr;
    }
    else {
            status = cloCOMPILER_AddStatementPlaceHolder(Compiler,
                                                         FuncName);
            if (gcmIS_ERROR(status)) return declOrDeclListPtr;
    }

    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                      clvDUMP_PARSER,
                      "</FUNCTION>"));
    return declOrDeclListPtr;
}

/** WILL need to work on this if we want to keep the delay the typedef expansion
    till later **/

static gceSTATUS
_ParseMergeTypeAttrToVariable(
IN clsNAME *Type,
IN clsNAME *Variable
)
{
   if(Type->u.typeInfo.specifiedAttr) {
     if(Type->u.typeInfo.specifiedAttr & clvATTR_PACKED) {
        Variable->u.variableInfo.specifiedAttr |= clvATTR_PACKED;
        Variable->context.packed = Type->context.packed;
     }

     if(Type->u.typeInfo.specifiedAttr & clvATTR_ALIGNED) {
        Variable->u.variableInfo.specifiedAttr |= clvATTR_ALIGNED;
        if(Type->context.alignment > Variable->context.alignment) {
          Variable->context.alignment = Type->context.alignment;
        }
     }
   }
   return gcvSTATUS_OK;
}

clsDeclOrDeclList *
clParseTypeDef(
IN cloCOMPILER Compiler,
IN clsDeclOrDeclList *DeclOrDeclListPtr
)
{
    clsNAME *name;
    cltATTR_FLAGS specifiedAttr;
    gcmASSERT(DeclOrDeclListPtr);

    name = DeclOrDeclListPtr->name;

    if(name) {
       name->type = clvTYPE_NAME;
       gcmASSERT(name->decl.dataType);

       clmNAME_InitializeTypeInfo(name);
       specifiedAttr = name->u.variableInfo.specifiedAttr;
       name->u.typeInfo.specifiedAttr = specifiedAttr;

       if(DeclOrDeclListPtr->initStatement) {
            gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                            name->lineNo,
                                            name->stringNo,
                                            clvREPORT_ERROR,
                                            "illegal typedef initialization"));
       }
    }
    return DeclOrDeclListPtr;
}

cloIR_BASE
clParseTags(
IN cloCOMPILER Compiler,
IN clsDATA_TYPE *Tags
)
{
    cloIR_BASE initStatement = gcvNULL;

    return initStatement;
}

static gceSTATUS
_ParseFillEnumAttr(
IN cloCOMPILER Compiler,
IN clsNAME *EnumIdentifier,
IN clsDECL *Decl,
IN clsATTRIBUTE *Attr
)
{
     gceSTATUS status;
     slsSLINK_LIST *enumList;
     clsNAME *enumName;
     gcmASSERT(Attr);

     enumName = Decl->dataType->u.enumerator;
     gcmASSERT(enumName && enumName->type == clvENUM_TAG_NAME);
     enumList = enumName->context.u.enumerator;

     if(Attr->specifiedAttr & clvATTR_PACKED) {
        gctBOOL packed;

        packed = Attr->packed;
        if(enumList) {
           cloIR_CONSTANT constant;
           clsENUMERATOR *prevEnum;
           clsENUMERATOR *nextEnum;
           gctINT maxVal = 0;
           gctINT tmpVal;
           gctINT numBits;
           gctINT type;
           gctINT elementType = clvTYPE_INT;

           FOR_EACH_SLINK_NODE(enumList, clsENUMERATOR, prevEnum, nextEnum) {
             gcmASSERT(nextEnum->member->type == clvENUM_NAME);
             constant = nextEnum->member->u.variableInfo.u.constant;
             gcmASSERT(constant);
             tmpVal = constant->values[0].intValue;
             if(tmpVal < 0) {
               tmpVal = -tmpVal;
             }
             if(tmpVal > maxVal) maxVal = tmpVal;
           }
           numBits = 0;
           while(maxVal) {
             maxVal >>= 1;
             numBits++;
           }
           if(numBits < 8) {
             type = T_CHAR;
             elementType = clvTYPE_CHAR;
           }
           else if(numBits < 15) {
             type = T_SHORT;
             elementType = clvTYPE_SHORT;
           }
           else type = T_INT;
           if(type != T_INT) {
             clsDATA_TYPE *dataType;

             status = cloCOMPILER_CreateDataType(Compiler,
                                                 type,
                                                 gcvNULL,
                                                 clvQUALIFIER_CONST,
                                                 clvQUALIFIER_NONE,
                                                 &dataType);
             if (gcmIS_ERROR(status)) return status;

             FOR_EACH_SLINK_NODE(enumList, clsENUMERATOR, prevEnum, nextEnum) {
                status = clParseConstantTypeConvert(nextEnum->member->u.variableInfo.u.constant,
                                                    elementType,
                                                    nextEnum->member->u.variableInfo.u.constant->values);
                if (gcmIS_ERROR(status)) return status;
                nextEnum->member->decl.dataType = dataType;
             }
             status = cloCOMPILER_CreateDataType(Compiler,
                                                 type,
                                                 enumName,
                                                 Decl->dataType->accessQualifier,
                                                 Decl->dataType->addrSpaceQualifier,
                                                 &dataType);
             if (gcmIS_ERROR(status)) return status;

             Decl->dataType = dataType;
             if(EnumIdentifier) {
                EnumIdentifier->decl.dataType = Decl->dataType;
             }
           }
        }
        if(EnumIdentifier) {
           EnumIdentifier->u.typeInfo.packed = packed;
        }
     }

     if(Attr->specifiedAttr & clvATTR_ALIGNED)  {
        gctUINT16 alignment;

        alignment = Attr->alignment;
        if(enumList) {
           clsENUMERATOR *enumerator;

           enumerator = slmSLINK_LIST_First(enumList,  clsENUMERATOR);
           if(enumerator  &&
              enumerator->member->context.alignment < alignment) {
              enumerator->member->context.alignment = alignment;
           }
        }
        if(EnumIdentifier) {
           EnumIdentifier->context.alignment = alignment;
        }
     }
     gcmVERIFY_OK(cloCOMPILER_Free(Compiler, Attr));
     return gcvSTATUS_OK;
}

cloIR_BASE
clParseEnumTags(
IN cloCOMPILER Compiler,
IN clsDATA_TYPE *Tags,
IN clsATTRIBUTE *Attr
)
{
   cloIR_BASE initStatement = gcvNULL;
   gcmASSERT(Tags);

   if(Attr) {
      clsDECL decl[1];

      clmDECL_Initialize(decl, Tags, (clsARRAY *)0, gcvNULL, gcvFALSE, clvSTORAGE_QUALIFIER_NONE);
      _ParseFillEnumAttr(Compiler, gcvNULL, decl, Attr);
   }

   return initStatement;
}

cloIR_BASE
clParseEnum(
IN cloCOMPILER Compiler,
IN clsDATA_TYPE *Enum
)
{
    cloIR_BASE initStatement = gcvNULL;
    return initStatement;
}

cloIR_BASE
clParseDeclaration(
IN cloCOMPILER Compiler,
IN clsDeclOrDeclList *DeclOrDeclListPtr
)
{
    cloIR_BASE initStatement = gcvNULL;
    if(DeclOrDeclListPtr) {
    if (DeclOrDeclListPtr->initStatement != gcvNULL) {
        gcmASSERT(DeclOrDeclListPtr->initStatements == gcvNULL);
        initStatement = DeclOrDeclListPtr->initStatement;
    }
    else if (DeclOrDeclListPtr->initStatements != gcvNULL) {
        gcmASSERT(DeclOrDeclListPtr->initStatement == gcvNULL);
        initStatement = &DeclOrDeclListPtr->initStatements->base;
    }
    gcmVERIFY_OK(cloCOMPILER_Free(Compiler, DeclOrDeclListPtr));
    }
    return initStatement;
}

void
clParseCompoundStatementBegin(
IN cloCOMPILER Compiler
)
{
    gceSTATUS    status;
    clsNAME_SPACE *    nameSpace;
    gctUINT16   die = VSC_DI_INVALIDE_DIE;
    clsNAME_SPACE *    parentSpace = gcvNULL;

    parentSpace = cloCOMPILER_GetCurrentSpace(Compiler);
    die = cloCOMPILER_AddDIE(Compiler, VSC_DI_TAG_LEXICALBLOCK, parentSpace->die, gcvNULL, 0, 0, 0, 0);

    status = cloCOMPILER_CreateNameSpace(Compiler,
                         &nameSpace);

    nameSpace->die = die;

    if (gcmIS_ERROR(status)) return;

    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                      clvDUMP_PARSER,
                      "<COMPOUND_STATEMENT>"));
}

cloIR_SET
clParseCompoundStatementEnd(
IN cloCOMPILER Compiler,
IN clsLexToken * StartToken,
IN cloIR_SET Set,
IN clsLexToken * EndToken
)
{
    clsNAME_SPACE *    nameSpace;

    gcmASSERT(StartToken);

    if (Set == gcvNULL) return gcvNULL;

    cloCOMPILER_PopCurrentNameSpace(Compiler, &nameSpace);

    cloCOMPILER_SetDIESourceLoc(Compiler, nameSpace->die, 0, StartToken->lineNo, EndToken->lineNo, StartToken->stringNo);

    Set->base.lineNo    = StartToken->lineNo;
    Set->base.stringNo    = StartToken->stringNo;
    Set->base.endLineNo    = EndToken->lineNo;

    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                      clvDUMP_PARSER,
                      "</COMPOUND_STATEMENT>"));
    return Set;
}

void
clParseCompoundStatementNoNewScopeBegin(
IN cloCOMPILER Compiler
)
{
    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                      clvDUMP_PARSER,
                      "<COMPOUND_STATEMENT_NO_NEW_SCOPE>"));
}

cloIR_SET
clParseCompoundStatementNoNewScopeEnd(
IN cloCOMPILER Compiler,
IN clsLexToken * StartToken,
IN cloIR_SET Set,
IN clsLexToken * EndToken
)
{
    gcmASSERT(StartToken);

    if (Set == gcvNULL) return gcvNULL;

    Set->base.lineNo    = StartToken->lineNo;
    Set->base.stringNo    = StartToken->stringNo;
    Set->base.endLineNo    = EndToken->lineNo;

    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                      clvDUMP_PARSER,
                      "</COMPOUND_STATEMENT_NO_NEW_SCOPE>"));
    return Set;
}

static void
_clInsertCases(
IN cloCOMPILER Compiler,
IN cloIR_LABEL NewCase,
IN OUT cloIR_LABEL *CaseHead
)
{
   cloIR_LABEL *curLoc;
   cloIR_LABEL curCase;

   curLoc = CaseHead;
   curCase = *curLoc;
   if(NewCase->type == clvCASE) {
     gcmASSERT(clmDATA_TYPE_elementType_GET(NewCase->caseValue->exprBase.decl.dataType) == clvTYPE_INT);
   }
   while(curCase) {
      if(curCase->type == clvDEFAULT) {
        if(NewCase->type == clvDEFAULT) {
            gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                  NewCase->base.lineNo,
                            NewCase->base.stringNo,
                            clvREPORT_ERROR,
                            "default case already specified"));
        }
        break;
      }
      if(NewCase->type == clvDEFAULT ||
         NewCase->caseValue->values[0].intValue < curCase->caseValue->values[0].intValue) {
         curLoc = &curCase->u.nextCase;
     curCase = *curLoc;
         continue;
      }
      if(NewCase->caseValue->values[0].intValue == curCase->caseValue->values[0].intValue) {
         gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                            NewCase->base.lineNo,
                         NewCase->base.stringNo,
                         clvREPORT_ERROR,
                         "case value \"%d\" already used",
                                         NewCase->caseValue));
      }
      break;
   }
   NewCase->u.nextCase = curCase;
   *curLoc = NewCase;
}

cloIR_BASE
clParseCaseStatement(
IN cloCOMPILER Compiler,
IN clsLexToken *StartToken,
IN cloIR_EXPR CaseExpr
)
{
    gceSTATUS status;
        cloIR_LABEL caseLabel;
    cloIR_CONSTANT caseConstant;
    clsSWITCH_SCOPE *switchScope;

    status = _CheckIntConstantExpr(Compiler, CaseExpr);
    if (gcmIS_ERROR(status)) return gcvNULL;

    caseConstant = (cloIR_CONSTANT) CaseExpr;
    status = clParseConstantTypeConvert(caseConstant, clvTYPE_INT, caseConstant->values);
    if (gcmIS_ERROR(status)) return gcvNULL;

    gcmASSERT(StartToken);

    /* Create the case label statement */
    status = cloIR_LABEL_Construct(Compiler,
                       StartToken->lineNo,
                       StartToken->stringNo,
                       &caseLabel);
    if (gcmIS_ERROR(status)) return gcvNULL;

    caseLabel->type = clvCASE;
    caseLabel->caseValue = caseConstant;
    switchScope = cloCOMPILER_GetSwitchScope(Compiler);
    gcmASSERT(switchScope);
    _clInsertCases(Compiler, caseLabel, &switchScope->cases);

    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                      clvDUMP_PARSER,
                      "<CASE_LABEL line=\"%d\" string=\"%d\" caseExpr=\"0x%x\"",
                      StartToken->lineNo,
                      StartToken->stringNo,
                      CaseExpr));

    return &caseLabel->base;
}

cloIR_BASE
clParseDefaultStatement(
IN cloCOMPILER Compiler,
IN clsLexToken *StartToken
)
{
    gceSTATUS status;
    cloIR_LABEL defaultLabel;
    clsSWITCH_SCOPE *switchScope;

    gcmASSERT(StartToken);

    /* Create the label null statement */
    status = cloIR_LABEL_Construct(Compiler,
                       StartToken->lineNo,
                       StartToken->stringNo,
                       &defaultLabel);
    if (gcmIS_ERROR(status)) return gcvNULL;

    defaultLabel->type = clvDEFAULT;
    switchScope = cloCOMPILER_GetSwitchScope(Compiler);
    gcmASSERT(switchScope);
    _clInsertCases(Compiler, defaultLabel, &switchScope->cases);

    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                clvDUMP_PARSER,
                "<DEFAULT_LABEL line=\"%d\" string=\"%d\"",
                StartToken->lineNo,
                StartToken->stringNo));

    return &defaultLabel->base;
}

void
clParseSwitchBodyBegin(
IN cloCOMPILER Compiler
)
{
   (void) cloCOMPILER_PushSwitchScope(Compiler, gcvNULL);

   gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                     clvDUMP_PARSER,
                     "<SWITCH_BODY>"));
}

cloIR_BASE
clParseSwitchBodyEnd(
IN cloCOMPILER Compiler,
IN clsLexToken * StartToken,
IN cloIR_SET Set,
IN clsLexToken * EndToken
)
{
   clsSWITCH_SCOPE *switchScope;
   cloIR_LABEL *curLoc;
   cloIR_LABEL curCase;
   gctBOOL hasDefault = gcvFALSE;

   gcmASSERT(StartToken);

   if (Set == gcvNULL) return gcvNULL;

   Set->base.lineNo    = StartToken->lineNo;
   Set->base.stringNo    = StartToken->stringNo;
   Set->base.endLineNo    = EndToken->lineNo;

   switchScope = cloCOMPILER_GetSwitchScope(Compiler);
   curLoc = &switchScope->cases;
   curCase = *curLoc;
   while(curCase) {
      if(curCase->type == clvDEFAULT) {
         hasDefault = gcvTRUE;
         break;
      }
      curLoc = &curCase->u.nextCase;
      curCase = *curLoc;
      continue;
   }
   if(!hasDefault) {
     cloIR_BASE defaultStatement;

     defaultStatement = clParseDefaultStatement(Compiler,
                                                StartToken);
     Set = clParseStatementList2(Compiler,
                                 Set,
                                 defaultStatement);
   }

   gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                 clvDUMP_PARSER,
                 "</SWITCH_BODY>"));
   return &Set->base;
}

cloIR_SET
clParseStatementList(
IN cloCOMPILER Compiler,
IN cloIR_BASE Statement
)
{
    gceSTATUS status;
    cloIR_SET set;

    status = cloIR_SET_Construct(Compiler,
                     0,
                     0,
                     clvSTATEMENT_SET,
                     &set);
    if (gcmIS_ERROR(status)) return gcvNULL;

    if (Statement != gcvNULL) {
        gcmVERIFY_OK(cloIR_SET_AddMember(Compiler,
                         set,
                         Statement));
    }
    return set;
}

cloIR_SET
clParseStatementList2(
IN cloCOMPILER Compiler,
IN cloIR_SET Set,
IN cloIR_BASE Statement
)
{
    if (Set != gcvNULL && Statement != gcvNULL) {
        gcmVERIFY_OK(cloIR_SET_AddMember(Compiler,
                         Set,
                         Statement));
    }
    return Set;
}

cloIR_BASE
clParseExprAsStatement(
IN cloCOMPILER Compiler,
IN cloIR_EXPR Expr
)
{
    if (Expr == gcvNULL) return gcvNULL;

    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                      clvDUMP_PARSER,
                      "<EXPRESSION_STATEMENT expr=\"0x%x\" />",
                      Expr));
    return &Expr->base;
}

cloIR_BASE
clParseCompoundStatementAsStatement(
IN cloCOMPILER Compiler,
IN cloIR_SET CompoundStatement
)
{
    if (CompoundStatement == gcvNULL) return gcvNULL;

    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                      clvDUMP_PARSER,
                      "<STATEMENT compoundStatement=\"0x%x\" />",
                      CompoundStatement));
    return &CompoundStatement->base;
}

cloIR_BASE
clParseCompoundStatementNoNewScopeAsStatementNoNewScope(
IN cloCOMPILER Compiler,
IN cloIR_SET CompoundStatementNoNewScope
)
{
    if (CompoundStatementNoNewScope == gcvNULL) return gcvNULL;

    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                      clvDUMP_PARSER,
                      "<STATEMENT_NO_NEW_SCOPE compoundStatementNoNewScope=\"0x%x\" />",
                      CompoundStatementNoNewScope));
    return &CompoundStatementNoNewScope->base;
}

clsIfStatementPair
clParseIfSubStatements(
IN cloCOMPILER Compiler,
IN cloIR_BASE TrueStatement,
IN cloIR_BASE FalseStatement
)
{
    clsIfStatementPair pair;

    pair.trueStatement    = TrueStatement;
    pair.falseStatement    = FalseStatement;

    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                      clvDUMP_PARSER,
                      "<SELECTION_REST_STATEMENT trueStatement=\"0x%x\""
                      " falseStatement=\"0x%x\" />",
                      TrueStatement,
                      FalseStatement));
    return pair;
}

static gceSTATUS
_CheckCondExpr(
IN cloCOMPILER Compiler,
IN cloIR_EXPR CondExpr
)
{
    gcmASSERT(CondExpr);
    gcmASSERT(CondExpr->decl.dataType);

    /* Check the operand */
    if (!clmDECL_IsScalar(&CondExpr->decl)) {
        gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                        CondExpr->base.lineNo,
                        CondExpr->base.stringNo,
                        clvREPORT_ERROR,
                        "require a scalar typed expression"));

        return gcvSTATUS_INVALID_ARGUMENT;
    }
    return gcvSTATUS_OK;
}

static cloIR_EXPR
_EqualizeExprOperandType(
    IN cloCOMPILER Compiler,
    IN cloIR_EXPR Operand,
    IN clsDECL *Decl
    )
{
   cloIR_EXPR newOperand = Operand;
   if(clmDECL_IsScalar(&Operand->decl)) {
       gceSTATUS status;
       cloIR_EXPR tempOperand;
       cloIR_UNARY_EXPR unaryExpr;
       cloIR_BINARY_EXPR binaryExpr;

       switch(cloIR_OBJECT_GetType(&Operand->base)) {
       case clvIR_UNARY_EXPR:
          unaryExpr = (cloIR_UNARY_EXPR) &Operand->base;
          if (unaryExpr->type != clvUNARY_FIELD_SELECTION) {
              tempOperand = _EqualizeExprOperandType(Compiler,
                                                     unaryExpr->operand,
                                                     Decl);
              unaryExpr->operand = tempOperand;
          }
          else {
              status = cloIR_CAST_EXPR_Construct(Compiler,
                                                 Operand->base.lineNo,
                                                 Operand->base.stringNo,
                                                 Decl,
                                                 Operand,
                                                 &newOperand);
              if (gcmIS_ERROR(status)) return gcvNULL;
          }
          break;

       case clvIR_BINARY_EXPR:
          binaryExpr = (cloIR_BINARY_EXPR) &Operand->base;
          if(binaryExpr->type != clvBINARY_SUBSCRIPT) {
              tempOperand = _EqualizeExprOperandType(Compiler,
                                                     binaryExpr->leftOperand,
                                                     Decl);
              binaryExpr->leftOperand = tempOperand;
              tempOperand = _EqualizeExprOperandType(Compiler,
                                                     binaryExpr->rightOperand,
                                                     Decl);
              binaryExpr->rightOperand = tempOperand;
          }
          else {
              status = cloIR_CAST_EXPR_Construct(Compiler,
                                                 Operand->base.lineNo,
                                                 Operand->base.stringNo,
                                                 Decl,
                                                 Operand,
                                                 &newOperand);
              if (gcmIS_ERROR(status)) return gcvNULL;
          }

          break;

       case clvIR_VARIABLE:
          status = cloIR_CAST_EXPR_Construct(Compiler,
                                             Operand->base.lineNo,
                                             Operand->base.stringNo,
                                             Decl,
                                             Operand,
                                             &newOperand);
          if (gcmIS_ERROR(status)) return gcvNULL;
          break;

       case clvIR_CONSTANT:
          status = cloIR_CAST_EXPR_Evaluate(Compiler,
                                            Decl,
                                            (cloIR_CONSTANT)Operand);
          if (gcmIS_ERROR(status)) return gcvNULL;
          break;

       default:
          break;
       }
   }
   return newOperand;
}

static gceSTATUS
_CheckForCondExpr(
IN cloCOMPILER Compiler,
IN cloIR_EXPR CondExpr,
IN OUT cloIR_EXPR *NewCondExpr
)
{
    cloIR_EXPR newCondExpr = CondExpr;
    cloIR_EXPR operandExpr;
    cloIR_UNARY_EXPR unaryExpr;
    cloIR_BINARY_EXPR binaryExpr;
    gcmASSERT(CondExpr);
    gcmASSERT(CondExpr->decl.dataType);

    /* Check the operand */
    if (!clmDECL_IsScalar(&CondExpr->decl)) {
        gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                        CondExpr->base.lineNo,
                        CondExpr->base.stringNo,
                        clvREPORT_ERROR,
                        "require a scalar typed expression"));

        return gcvSTATUS_INVALID_ARGUMENT;
    }

    operandExpr = CondExpr;
    switch(cloIR_OBJECT_GetType(&operandExpr->base)) {
    case clvIR_UNARY_EXPR:
       unaryExpr = (cloIR_UNARY_EXPR) &operandExpr->base;
       if (unaryExpr->type != clvUNARY_FIELD_SELECTION) {
           operandExpr = unaryExpr->operand;
       }
       break;

    case clvIR_BINARY_EXPR:
       binaryExpr = (cloIR_BINARY_EXPR) &operandExpr->base;
       if(!(binaryExpr->type == clvBINARY_SUBSCRIPT ||
            binaryExpr->type == clvBINARY_LSHIFT ||
            binaryExpr->type == clvBINARY_RSHIFT)) {
           cltELEMENT_TYPE leftElementType, rightElementType;

           leftElementType = clmDATA_TYPE_elementType_GET(binaryExpr->leftOperand->decl.dataType);
           rightElementType = clmDATA_TYPE_elementType_GET(binaryExpr->rightOperand->decl.dataType);

           operandExpr = binaryExpr->rightOperand;
           if(leftElementType > rightElementType) { /* convert right */
               operandExpr = binaryExpr->leftOperand;
           }
       }
       break;

    default:
       break;
    }

/* A FOR condition expression is used more than once during FOR statement
   IR code generation. Due to implicit type conversion and the condition
   expression operands' data type may be modified. To prevent the data type
   being changed, the implicit type conversion needs to be explicitly
   embedded in the condition expression itself. The following code is to
   check for type conversion and add it as such
 */

     newCondExpr = _EqualizeExprOperandType(Compiler,
                                            CondExpr,
                                            &operandExpr->decl);
     if(!newCondExpr) {
          return gcvSTATUS_INVALID_ARGUMENT;
     }
     else *NewCondExpr = newCondExpr;

     return gcvSTATUS_OK;
}

cloIR_BASE
clParseIfStatement(
IN cloCOMPILER Compiler,
IN clsLexToken * StartToken,
IN cloIR_EXPR CondExpr,
IN clsIfStatementPair IfStatementPair
)
{
    gceSTATUS    status;
    cloIR_SELECTION    selection;
    cloIR_CONSTANT    condConstant;
    gctBOOL        condValue;
    clsDECL decl;

    gcmASSERT(StartToken);

    if (CondExpr == gcvNULL) return gcvNULL;

    /* Check error */
    status = _CheckCondExpr(Compiler, CondExpr);
    if (gcmIS_ERROR(status)) return gcvNULL;

    /* Constant calculation */
    if (cloIR_OBJECT_GetType(&CondExpr->base) == clvIR_CONSTANT) {
        condConstant = (cloIR_CONSTANT)CondExpr;
        gcmASSERT(condConstant->valueCount == 1);
        gcmASSERT(condConstant->values);

        condValue = condConstant->values[0].boolValue;

        gcmVERIFY_OK(cloIR_OBJECT_Destroy(Compiler, &CondExpr->base));

        if (condValue) {
           if (IfStatementPair.falseStatement != gcvNULL) {
            gcmVERIFY_OK(cloIR_OBJECT_Destroy(Compiler, IfStatementPair.falseStatement));
           }
           return IfStatementPair.trueStatement;
        }
        else {
           if (IfStatementPair.trueStatement != gcvNULL) {
            gcmVERIFY_OK(cloIR_OBJECT_Destroy(Compiler, IfStatementPair.trueStatement));
           }
           return IfStatementPair.falseStatement;
        }
    }

    /* Create the selection */
    clmDECL_Initialize(&decl, gcvNULL, (clsARRAY *)0, gcvNULL, gcvFALSE, clvSTORAGE_QUALIFIER_NONE);
    status = cloIR_SELECTION_Construct(Compiler,
                       StartToken->lineNo,
                       StartToken->stringNo,
                       &decl,
                       CondExpr,
                       IfStatementPair.trueStatement,
                       IfStatementPair.falseStatement,
                       &selection);
    if (gcmIS_ERROR(status)) return gcvNULL;

    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                clvDUMP_PARSER,
                "<IF_STATEMENT line=\"%d\" string=\"%d\" condExpr=\"0x%x\""
                " trueStatement=\"0x%x\" falseStatement=\"0x%x\" />",
                StartToken->lineNo,
                StartToken->stringNo,
                CondExpr,
                IfStatementPair.trueStatement,
                IfStatementPair.falseStatement));
    return &selection->exprBase.base;
}

cloIR_BASE
clParseSwitchStatement(
IN cloCOMPILER Compiler,
IN clsLexToken * StartToken,
IN cloIR_EXPR ControlExpr,
IN cloIR_BASE SwitchBody
)
{
    gceSTATUS    status;
    cloIR_SWITCH    switchSelect;
    clsDECL decl;
    cloIR_LABEL cases = gcvNULL;

    gcmASSERT(StartToken);

    if (ControlExpr == gcvNULL) return gcvNULL;

    /* Check error */
    if (!clmDECL_IsInt(&ControlExpr->decl)) {
        gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                        ControlExpr->base.lineNo,
                        ControlExpr->base.stringNo,
                        clvREPORT_ERROR,
                        "require a scalar integer expression"));
        return gcvNULL;
    }

    /* Create the selection */
    if(SwitchBody) {
      clsSWITCH_SCOPE *switchScope;

      switchScope = cloCOMPILER_GetSwitchScope(Compiler);
      if(switchScope) {
        cases =  switchScope->cases;
      }
      cloCOMPILER_PopSwitchScope(Compiler);
    }
    clmDECL_Initialize(&decl, gcvNULL, (clsARRAY *)0, gcvNULL, gcvFALSE, clvSTORAGE_QUALIFIER_NONE);
    status = cloIR_SWITCH_Construct(Compiler,
                    StartToken->lineNo,
                    StartToken->stringNo,
                    &decl,
                    ControlExpr,
                    SwitchBody,
                    cases,
                    &switchSelect);
    if (gcmIS_ERROR(status)) return gcvNULL;

    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                clvDUMP_PARSER,
                "<SWITCH_STATEMENT line=\"%d\" string=\"%d\" condExpr=\"0x%x\""
                " switchBody=\"0x%x\" cases=\"0x%x\" />",
                StartToken->lineNo,
                StartToken->stringNo,
                ControlExpr,
                SwitchBody,
                cases));
    return &switchSelect->exprBase.base;
}

static gctUINT _clStringBufferSize = 0;
static gctSTRING _clStringBuffer = gcvNULL;

static gctSTRING
_clGetStringBuffer(
IN gctUINT BufSize
)
{
   if(BufSize > _clStringBufferSize) {
      if(_clStringBuffer) {
        clFree((gctPOINTER)_clStringBuffer);
      }
      _clStringBufferSize = BufSize << 1; /* initialize with double the size */
      _clStringBuffer = (gctSTRING)clMalloc((gctSIZE_T)sizeof(gctCHAR) * _clStringBufferSize);
   }
   return _clStringBuffer;
}


static cltPOOL_STRING
_clTransformLabel(
IN cloCOMPILER Compiler,
IN gctSTRING Label
)
{
  gceSTATUS status;
  gctSIZE_T labelLen;
  gctSTRING newLabel;
  cltPOOL_STRING symbolInPool;

/* create new label name by prepending a blamk character to make it different from other
   names in the program **/
  if (Label == gcvNULL) return gcvNULL;
  labelLen = gcoOS_StrLen(Label, gcvNULL);
  newLabel = _clGetStringBuffer(labelLen + 2);
  if (newLabel == gcvNULL) return gcvNULL;
  newLabel[0] = ' ';

  gcoOS_StrCopySafe(newLabel + 1, labelLen + 1, Label);
  status = cloCOMPILER_AllocatePoolString(Compiler,
                      newLabel,
                      &symbolInPool);
  if (gcmIS_ERROR(status)) return gcvNULL;
  return symbolInPool;
}

cloIR_BASE
clParseStatementLabel(
IN cloCOMPILER Compiler,
IN clsLexToken *LabelIdentifier
)
{
    gceSTATUS status;
    clsNAME *name;
    gctSTRING newLabelName;

    newLabelName = _clTransformLabel(Compiler, LabelIdentifier->u.identifier.name);
    if(newLabelName == gcvNULL) return gcvNULL;

    status = cloCOMPILER_SearchName(Compiler, newLabelName, gcvTRUE, &name);
    if(status == gcvSTATUS_OK) { /* Statement label created already */
       if(name->u.labelInfo.label) {
        gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                        LabelIdentifier->lineNo,
                        LabelIdentifier->stringNo,
                        clvREPORT_ERROR,
                        "statement label \'%s\' already defined",
                        LabelIdentifier->u.identifier.name));
        return gcvNULL;
       }
    }
    else {
    /* Create the label */
       status = cloCOMPILER_CreateName(Compiler,
                       LabelIdentifier->lineNo,
                       LabelIdentifier->stringNo,
                       clvLABEL_NAME,
                       gcvNULL,
                       newLabelName,
                       gcvNULL,
                       clvEXTENSION_NONE,
                       &name);
       if (gcmIS_ERROR(status)) return gcvNULL;
    }

    /* Create the label null statement */
    status = cloIR_LABEL_Construct(Compiler,
                       LabelIdentifier->lineNo,
                       LabelIdentifier->stringNo,
                       &name->u.labelInfo.label);
    if (gcmIS_ERROR(status)) return gcvNULL;

    name->u.labelInfo.label->u.name = name;
        name->u.labelInfo.label->type = clvNAMED;
    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                      clvDUMP_PARSER,
                      "<STATEMENT_LABEL line=\"%d\" string=\"%d\" label=\"0x%x\"",
                      LabelIdentifier->lineNo,
                      LabelIdentifier->stringNo,
                      name->u.labelInfo.label));
    return &name->u.labelInfo.label->base;
}

void
clParseWhileStatementBegin(
IN cloCOMPILER Compiler
)
{
    gceSTATUS    status;
    clsNAME_SPACE *    nameSpace;
    gctUINT16   die = VSC_DI_INVALIDE_DIE;
    clsNAME_SPACE *    parentSpace = gcvNULL;

    parentSpace = cloCOMPILER_GetCurrentSpace(Compiler);
    die = cloCOMPILER_AddDIE(Compiler, VSC_DI_TAG_LEXICALBLOCK, parentSpace->die, gcvNULL, 0, 0, 0, 0);

    status = cloCOMPILER_CreateNameSpace(Compiler,
                         &nameSpace);
    if (gcmIS_ERROR(status)) return;

    nameSpace->die = die;

    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                      clvDUMP_PARSER,
                      "<WHILE_STATEMENT>"));
}

cloIR_BASE
clParseWhileStatementEnd(
IN cloCOMPILER Compiler,
IN clsLexToken * StartToken,
IN cloIR_EXPR CondExpr,
IN cloIR_BASE LoopBody
)
{
    gceSTATUS        status;
    cloIR_ITERATION        iteration;
    clsNAME_SPACE *    nameSpace;

    gcmASSERT(StartToken);

    cloCOMPILER_PopCurrentNameSpace(Compiler, &nameSpace);

    /* Check error */
    if (CondExpr == gcvNULL) {
        gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                        StartToken->lineNo,
                        StartToken->stringNo,
                        clvREPORT_ERROR,
                        "while statement has no condition"));
        return gcvNULL;
    }

    status = _CheckCondExpr(Compiler, CondExpr);
    if (gcmIS_ERROR(status)) return gcvNULL;

    /* Create the iteration */
    status = cloIR_ITERATION_Construct(Compiler,
                       StartToken->lineNo,
                       StartToken->stringNo,
                       clvWHILE,
                       CondExpr,
                       LoopBody,
                       gcvNULL,
                       gcvNULL,
                       gcvNULL,
                       &iteration);
    if (gcmIS_ERROR(status))  return gcvNULL;

    cloCOMPILER_SetDIESourceLoc(Compiler, nameSpace->die, 0, iteration->base.lineNo, iteration->base.endLineNo, iteration->base.stringNo);

    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                      clvDUMP_PARSER,
                     "</WHILE_STATEMENT>"));
    return &iteration->base;
}

cloIR_BASE
clParseDoWhileStatement(
IN cloCOMPILER Compiler,
IN clsLexToken * StartToken,
IN cloIR_BASE LoopBody,
IN cloIR_EXPR CondExpr
)
{
    gceSTATUS    status;
    cloIR_ITERATION    iteration;

    gcmASSERT(StartToken);

    if (CondExpr == gcvNULL) {
        gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                        StartToken->lineNo,
                        StartToken->stringNo,
                        clvREPORT_ERROR,
                        "do-while statement has no condition"));
        return gcvNULL;
    }

    status = _CheckCondExpr(Compiler, CondExpr);
    if (gcmIS_ERROR(status))  return gcvNULL;

    status = cloIR_ITERATION_Construct(Compiler,
                       StartToken->lineNo,
                       StartToken->stringNo,
                       clvDO_WHILE,
                       CondExpr,
                       LoopBody,
                       gcvNULL,
                       gcvNULL,
                       gcvNULL,
                       &iteration);
    if (gcmIS_ERROR(status))  return gcvNULL;

    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                      clvDUMP_PARSER,
                      "<DO_WHILE_STATEMENT line=\"%d\" string=\"%d\""
                      " condExpr=\"0x%x\" LoopBody=\"0x%x\" />",
                      StartToken->lineNo,
                      StartToken->stringNo,
                      CondExpr,
                      LoopBody));
    return &iteration->base;
}

void
clParseForStatementBegin(
IN cloCOMPILER Compiler)
{
    gceSTATUS    status;
    clsNAME_SPACE *    nameSpace;
    gctUINT16   die = VSC_DI_INVALIDE_DIE;
    clsNAME_SPACE *    parentSpace = gcvNULL;

    parentSpace = cloCOMPILER_GetCurrentSpace(Compiler);
    die = cloCOMPILER_AddDIE(Compiler, VSC_DI_TAG_LEXICALBLOCK, parentSpace->die, gcvNULL, 0, 0, 0, 0);

    status = cloCOMPILER_CreateNameSpace(Compiler,
                         &nameSpace);
    if (gcmIS_ERROR(status)) return;

    nameSpace->die = die;

    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                      clvDUMP_PARSER,
                      "<FOR_STATEMENT>"));
}

cloIR_BASE
clParseForStatementEnd(
IN cloCOMPILER Compiler,
IN clsLexToken * StartToken,
IN cloIR_BASE ForInitStatement,
IN clsForExprPair ForExprPair,
IN cloIR_BASE LoopBody
)
{
    gceSTATUS    status;
    cloIR_ITERATION    iteration;
    clsNAME_SPACE *    forSpace=gcvNULL;

    gcmASSERT(StartToken);

    cloCOMPILER_PopCurrentNameSpace(Compiler, &forSpace);

    if (ForExprPair.condExpr != gcvNULL) {
        status = _CheckForCondExpr(Compiler,
                                   ForExprPair.condExpr,
                                   &ForExprPair.condExpr);
        if (gcmIS_ERROR(status)) return gcvNULL;
    }

    status = cloIR_ITERATION_Construct(Compiler,
                                       StartToken->lineNo,
                                       StartToken->stringNo,
                                       clvFOR,
                                       ForExprPair.condExpr,
                                       LoopBody,
                                       forSpace,
                                       ForInitStatement,
                                       ForExprPair.restExpr,
                                       &iteration);
    if (gcmIS_ERROR(status)) return gcvNULL;

    cloCOMPILER_SetDIESourceLoc(Compiler, forSpace->die, 0, iteration->base.lineNo, iteration->base.endLineNo, iteration->base.stringNo);

    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                                  clvDUMP_PARSER,
                                  "</FOR_STATEMENT>"));
    return &iteration->base;
}

clsForExprPair
clParseForControl(
    IN cloCOMPILER Compiler,
    IN cloIR_EXPR CondExpr,
    IN cloIR_EXPR RestExpr
    )
{
    clsForExprPair pair;

    pair.condExpr    = CondExpr;
    pair.restExpr    = RestExpr;

    return pair;
}

gceSTATUS
_CheckJumpExpr(
IN cloCOMPILER Compiler,
IN gctUINT LineNo,
IN gctUINT StringNo,
IN cleJUMP_TYPE Type,
IN cloIR_EXPR ReturnExpr
)
{
    return gcvSTATUS_OK;
}

cloIR_BASE
clParseJumpStatement(
IN cloCOMPILER Compiler,
IN cleJUMP_TYPE Type,
IN clsLexToken * StartToken,
IN cloIR_EXPR ReturnExpr
)
{
    gceSTATUS    status;
    cloIR_JUMP   jump;
    cloIR_EXPR   returnExpr;

    gcmASSERT(StartToken);

    status = _CheckJumpExpr(Compiler,
                    StartToken->lineNo,
                    StartToken->stringNo,
                    Type,
                    ReturnExpr);
    if (gcmIS_ERROR(status)) return gcvNULL;

    status = _ParseSetAggregateTypedOperandAddressed(Compiler,
                                                     ReturnExpr,
                                                     &returnExpr);
    if (gcmIS_ERROR(status)) return gcvNULL;

    status = cloIR_JUMP_Construct(Compiler,
                                  StartToken->lineNo,
                                  StartToken->stringNo,
                                  Type,
                                  returnExpr,
                                  &jump);
    if (gcmIS_ERROR(status)) return gcvNULL;

    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                      clvDUMP_PARSER,
                      "<JUMP line=\"%d\" string=\"%d\""
                      " type=\"%s\" returnExpr=\"0x%x\" />",
                      StartToken->lineNo,
                      StartToken->stringNo,
                      clGetIRJumpTypeName(Type),
                      ReturnExpr));
    return &jump->base;
}

cloIR_BASE
clParseGotoStatement(
IN cloCOMPILER Compiler,
IN clsLexToken *StartToken,
IN clsLexToken *Label
)
{
    gceSTATUS    status;
    cloIR_JUMP    gotoStmt;
    clsNAME        *name;
    gctSTRING newLabelName;

    gcmASSERT(StartToken);

    newLabelName = _clTransformLabel(Compiler, Label->u.identifier.name);
    status = cloCOMPILER_SearchName(Compiler, newLabelName, gcvTRUE, &name);
    if(status == gcvSTATUS_OK) { /* Goto label defined already */
       if(name->type != clvLABEL_NAME) {
        gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                        Label->lineNo,
                        Label->stringNo,
                        clvREPORT_ERROR,
                        "incorrect goto label \'%s\' type defined",
                        name->symbol));
        return gcvNULL;
       }
    }
    else { /* create the goto label */
       status = cloCOMPILER_CreateName(Compiler,
                       Label->lineNo,
                       Label->stringNo,
                       clvLABEL_NAME,
                       gcvNULL,
                       newLabelName,
                       gcvNULL,
                       clvEXTENSION_NONE,
                       &name);
       if (gcmIS_ERROR(status)) return gcvNULL;

       name->u.labelInfo.isReferenced = gcvTRUE;
    }
    status = cloIR_GOTO_Construct(Compiler,
                      StartToken->lineNo,
                      StartToken->stringNo,
                      name,
                      &gotoStmt);
    if (gcmIS_ERROR(status)) return gcvNULL;

    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                      clvDUMP_PARSER,
                      "<GOTO line=\"%d\" string=\"%d\""
                      "label=\"0x%x\" />",
                      StartToken->lineNo,
                      StartToken->stringNo,
                      name));

    return &gotoStmt->base;
}

void
clParseExternalDecl(
IN cloCOMPILER Compiler,
IN cloIR_BASE Decl
)
{
    if (Decl == gcvNULL) return;

    gcmVERIFY_OK(cloCOMPILER_AddExternalDecl(Compiler, Decl));

    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                    clvDUMP_PARSER,
                    "<EXTERNAL_DECL decl=\"0x%x\" />",
                    Decl));
}

void
clParseFuncDef(
IN cloCOMPILER Compiler,
IN clsNAME * FuncName,
IN cloIR_SET Statements
)
{
    gceSTATUS    status;
    clsNAME *    firstFuncName;

    if (FuncName == gcvNULL) return;

    if (Statements == gcvNULL) {
        if(FuncName->type == clvKERNEL_FUNC_NAME) {
           gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                           FuncName->lineNo,
                                           FuncName->stringNo,
                                           clvREPORT_WARN,
                                           "kernel function : '%s' is empty",
                                           FuncName->symbol));
        }

        status = cloIR_SET_Construct(Compiler,
                                     FuncName->lineNo,
                                     FuncName->stringNo,
                                     clvSTATEMENT_SET,
                                     &Statements);

        cloCOMPILER_SetDIESourceLoc(Compiler, FuncName->die, 0, Statements->base.lineNo, Statements->base.endLineNo, Statements->base.stringNo);

        if (gcmIS_ERROR(status)) return;
    }

    cloCOMPILER_PopCurrentNameSpace(Compiler, gcvNULL);
    status = cloCOMPILER_CheckNewFuncName(Compiler,
                                          FuncName,
                                          &firstFuncName);
    if (gcmIS_ERROR(status)) return;
    gcmASSERT(firstFuncName);

    if (FuncName != firstFuncName) {
            status = clsNAME_BindAliasParamNames(Compiler, FuncName, firstFuncName);
            if (gcmIS_ERROR(status)) return;

            status = clsNAME_SPACE_ReleaseName(Compiler,
                                               cloCOMPILER_GetCurrentSpace(Compiler),
                                               FuncName);
            if (gcmIS_ERROR(status)) return;

            gcmASSERT(firstFuncName->u.funcInfo.funcBody &&
                      slsDLINK_LIST_IsEmpty(&firstFuncName->u.funcInfo.funcBody->members));

            slmDLINK_LIST_Prepend(&Statements->members, &firstFuncName->u.funcInfo.funcBody->members);
            slsDLINK_LIST_Initialize(&Statements->members);

            gcmVERIFY_OK(cloIR_SET_Destroy(Compiler,
                                           &Statements->base));

            gcmVERIFY_OK(cloNAME_BindFuncBody(Compiler,
                                              firstFuncName,
                                              firstFuncName->u.funcInfo.funcBody));
    }
    else {
        gcmVERIFY_OK(cloNAME_BindFuncBody(Compiler,
                                          firstFuncName,
                                          Statements));
        gcmVERIFY_OK(cloCOMPILER_AddExternalDecl(Compiler,
                                                 &Statements->base));
    }

    firstFuncName->u.funcInfo.isFuncDef = gcvTRUE;

    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                    clvDUMP_PARSER,
                    "</FUNCTION>"));

    if (gcmIS_SUCCESS(gcoOS_StrCmp(firstFuncName->symbol, "main"))) {
        if (gcmIS_ERROR(cloCOMPILER_MainDefined(Compiler))) {
            gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                            firstFuncName->lineNo,
                                            firstFuncName->stringNo,
                                            clvREPORT_ERROR,
                                            "'main' function redefined"));
        }
    }
    else if(firstFuncName->type == clvKERNEL_FUNC_NAME) {
        gctUINT argCount = 0;
        clsNAME *paramName;

        (void)cloCOMPILER_KernelFuncDefined(Compiler);

        FOR_EACH_DLINK_NODE(&firstFuncName->u.funcInfo.localSpace->names, clsNAME, paramName) {
            if (paramName->type != clvPARAMETER_NAME) break;
            if(clmDECL_IsPointerType(&paramName->decl) &&
               clGetAddrSpaceQualifier(&paramName->decl) == clvQUALIFIER_LOCAL) {
               cloCOMPILER_SetHasLocalMemoryKernelArg(Compiler);
            }
            argCount += clGetOperandCountForRegAllocByName(paramName);
        }

        /* There is a hypothesis that no uniform is added before all kernel function arguments are added. */
        cloCOMPILER_SetMaxKernelFunctionArgs(Compiler, argCount);
    }

    cloCOMPILER_SetDIESourceLoc(Compiler, FuncName->die, 0, FuncName->lineNo, Statements->base.endLineNo, Statements->base.stringNo);
}

clsNAME    *
clParseParameterList(
IN cloCOMPILER Compiler,
IN clsNAME * FuncName,
IN clsNAME * ParamName
)
{
    return FuncName;
}

clsDeclOrDeclList *
clParseTypeDecl(
IN cloCOMPILER Compiler,
IN clsDATA_TYPE * DataType
)
{
    gceSTATUS status;
    clsDeclOrDeclList *declOrDeclListPtr;
    gctPOINTER pointer;

    status = cloCOMPILER_Allocate(Compiler,
                    (gctSIZE_T)sizeof(clsDeclOrDeclList),
                    (gctPOINTER *) &pointer);

    if (gcmIS_ERROR(status)) return gcvNULL;
    declOrDeclListPtr = pointer;

    clmDECL_Initialize(&declOrDeclListPtr->decl, DataType, (clsARRAY *)0, gcvNULL, gcvFALSE, clvSTORAGE_QUALIFIER_NONE);
    declOrDeclListPtr->name        = gcvNULL;
    declOrDeclListPtr->lhs        = gcvNULL;
    declOrDeclListPtr->designator= gcvNULL;
    declOrDeclListPtr->designator    = gcvNULL;
    declOrDeclListPtr->initStatement= gcvNULL;
    declOrDeclListPtr->initStatements= gcvNULL;

    return declOrDeclListPtr;
}

clsNAME    *
clParseParameterDecl(
IN cloCOMPILER Compiler,
IN clsDECL * Decl,
IN clsLexToken * Identifier,
IN clsATTRIBUTE *Attr
)
{
    gceSTATUS  status;
    clsNAME    *name;
    gctINT lineNo;
    gctINT stringNo;
    gctSTRING symbol;
    clsNAME *derivedType = gcvNULL;
    clsDECL decl[1];
    clsDECL *declPtr;

    gcmASSERT(Decl);

    if (Decl->dataType == gcvNULL) return gcvNULL;

    switch(Decl->dataType->type) {
    case T_TYPE_NAME:
        status = _ParseFlattenType(Compiler, Decl, decl);
        if(gcmIS_ERROR(status)) return gcvNULL;
        declPtr = decl;
        derivedType = Decl->dataType->u.typeDef;
        break;

    case T_ENUM:
        declPtr = Decl;
        derivedType = Decl->dataType->u.enumerator;
        break;

    default:
        declPtr = _HandleSpecialType(Compiler, Decl);
        break;
    }

    if(Identifier == gcvNULL) {
        lineNo = cloCOMPILER_GetCurrentLineNo(Compiler);
        stringNo = cloCOMPILER_GetCurrentStringNo(Compiler);
        symbol = "";
        if(!clmHasRightLanguageVersion(Compiler, _cldCL1Dot2) &&
           clmDECL_IsVoid(declPtr)) {
            gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                            lineNo,
                            stringNo,
                            clvREPORT_ERROR,
                            "parameter '%s' type cannot be void",
                            symbol));
            return gcvNULL;
        }
    }
    else {
        lineNo = Identifier->lineNo;
        stringNo = Identifier->stringNo;
        symbol = Identifier->u.identifier.name;
    }

    if (clmDATA_TYPE_IsImage(declPtr->dataType)) {
        if(declPtr->dataType->type == T_IMAGE2D_DYNAMIC_ARRAY_T) {
            gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                            lineNo,
                            stringNo,
                            clvREPORT_ERROR,
                            "unrecognizable type '_viv_image2d_array_t' specified for parameter '%s'",
                            symbol));
            return gcvNULL;
        }
        if(clmDECL_IsArray(declPtr)) {
            gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                            lineNo,
                            stringNo,
                            clvREPORT_ERROR,
                            "image parameter '%s' cannot be an array",
                            symbol));
            return gcvNULL;
        }
        if(clmDECL_IsPointerType(declPtr)) {
            gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                            lineNo,
                            stringNo,
                            clvREPORT_ERROR,
                            "image parameter '%s' cannot be a pointer",
                            symbol));
            return gcvNULL;
        }
        if(declPtr->dataType->addrSpaceQualifier == clvQUALIFIER_LOCAL) {
            gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                            lineNo,
                            stringNo,
                            clvREPORT_ERROR,
                            "image parameter '%s' cannot be declared in local address space",
                            symbol));
            return gcvNULL;
        }
    }
    else {
        if(declPtr->dataType->accessQualifier == clvQUALIFIER_UNIFORM) {
            gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                            lineNo,
                            stringNo,
                            clvREPORT_ERROR,
                            "parameter '%s' cannot be declared with \"_viv_uniform\" qualifier",
                            symbol));
            return gcvNULL;
        }
    }

    status = cloCOMPILER_CreateName(Compiler,
                    lineNo,
                    stringNo,
                    clvPARAMETER_NAME,
                    declPtr,
                    symbol,
                    (Identifier != gcvNULL)? Identifier->u.identifier.ptrDscr : gcvNULL,
                    clvEXTENSION_NONE,
                    &name);
    if (gcmIS_ERROR(status)) return gcvNULL;

    name->derivedType = derivedType;
    _ParseFillVariableAttr(Compiler,
                           lineNo,
                           stringNo,
                           declPtr, name, Attr);
    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                    clvDUMP_PARSER,
                    "<PARAMETER_DECL decl=\"0x%x\" name=\"%s\" />",
                    Decl,
                    symbol));
    return name;
}

clsNAME    *
clParseArrayParameterDecl(
IN cloCOMPILER Compiler,
IN clsDECL * Decl,
IN clsLexToken * Identifier,
IN cloIR_EXPR ArrayLengthExpr,
IN clsATTRIBUTE *Attr
)
{
    gceSTATUS status;
    clsARRAY array[1];
    clsDECL arrayDecl;

    if (Decl->dataType == gcvNULL || ArrayLengthExpr == gcvNULL) return gcvNULL;

    clmEvaluateExprToArrayLength(Compiler,
                                 ArrayLengthExpr,
                                 array,
                                 gcvFALSE,
                                 status);
    if (gcmIS_ERROR(status)) return gcvNULL;

    status = cloCOMPILER_CreateArrayDecl(Compiler,
                                         Decl->dataType,
                                         array,
                                         Decl->ptrDscr,
                                         &arrayDecl);
    if (gcmIS_ERROR(status)) return gcvNULL;

    return clParseParameterDecl(Compiler,
                                &arrayDecl,
                                Identifier,
                                Attr);
}

#define _clmMakeIdentifierToken(Token, Symbol)  \
    do { \
    (void) gcoOS_ZeroMemory((gctPOINTER)&Token, sizeof(clsLexToken)); \
    Token.type = T_IDENTIFIER; \
    Token.u.identifier.name = Symbol; \
    } while(gcvFALSE)

clsNAME    *
clParseQualifiedParameterDecl(
IN cloCOMPILER Compiler,
IN slsSLINK_LIST * TypeQualifierList,
IN clsLexToken * ParameterQualifier,
IN clsNAME * ParameterDecl
)
{
      clsDATA_TYPE *dataType;
      clsDECL decl;

      if (ParameterDecl == gcvNULL) return gcvNULL;
      gcmASSERT(ParameterDecl->decl.dataType);
      decl = clParseQualifiedType(Compiler,
                                  TypeQualifierList,
                                  gcvTRUE,
                                  &ParameterDecl->decl);
      dataType = decl.dataType;
      if(dataType) {
        ParameterDecl->decl.dataType = dataType;
        ParameterDecl->decl.storageQualifier = decl.storageQualifier;
        if(dataType->accessQualifier == clvQUALIFIER_NONE &&
           ParameterQualifier) {
            gceSTATUS status;

            status = cloCOMPILER_CloneDataType(Compiler,
                                               ParameterQualifier->u.qualifier,
                                               dataType->addrSpaceQualifier,
                                               dataType,
                                               &ParameterDecl->decl.dataType);
            if (gcmIS_ERROR(status)) {
               return gcvNULL;
            }
        }
        if(ParameterDecl->u.variableInfo.specifiedAttr & clvATTR_ENDIAN) {
            cltQUALIFIER addrSpaceQualifier = clGetAddrSpaceQualifier(&ParameterDecl->decl);

            if(addrSpaceQualifier != clvQUALIFIER_CONSTANT &&
               addrSpaceQualifier != clvQUALIFIER_GLOBAL) {
                gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                                ParameterDecl->lineNo,
                                                ParameterDecl->stringNo,
                                                clvREPORT_ERROR,
                                                "Endian attribute can only be applied to pointer types that are in the global or constant address space"));
            }
        }
    }
    return ParameterDecl;
}

static gceSTATUS
_ParseQualifiedType(
IN cloCOMPILER Compiler,
IN clsTYPE_QUALIFIER *TypeQualifier,
IN gctBOOL ForParamDecl,
IN clsDECL *Decl
)
{
    cltQUALIFIER accessQualifier = clvQUALIFIER_NONE;
    cltQUALIFIER addrSpaceQualifier = clvQUALIFIER_NONE;
    cltQUALIFIER storageQualifier = clvSTORAGE_QUALIFIER_NONE;
    gctBOOL inTypeCast = gcvFALSE;
    gctBOOL mustAtGlobalNameSpace = gcvFALSE;
    gctBOOL atGlobalNameSpace;

    switch (TypeQualifier->type) {
    case T_CONSTANT:
    case T_GLOBAL:
        addrSpaceQualifier = TypeQualifier->qualifier;
        mustAtGlobalNameSpace = gcvTRUE;
/** Setting to in type cast mode for now to avoid reporting erroneous error
    But need to check variable declarator against the qualifier ***/
        inTypeCast = gcvTRUE;
        break;

    case T_UNIFORM:
        accessQualifier = TypeQualifier->qualifier;
        mustAtGlobalNameSpace = gcvTRUE;
        break;

    case T_LOCAL:
        addrSpaceQualifier = TypeQualifier->qualifier;
        mustAtGlobalNameSpace = gcvFALSE;
        break;

    case T_PRIVATE:
        addrSpaceQualifier = TypeQualifier->qualifier;
        mustAtGlobalNameSpace = gcvFALSE;
        break;

    case T_CONST:
        accessQualifier = TypeQualifier->qualifier;
        mustAtGlobalNameSpace = gcvFALSE;
        break;

    case T_STATIC:
        storageQualifier = TypeQualifier->qualifier;
        mustAtGlobalNameSpace = gcvTRUE;
        break;

    case T_EXTERN:
        storageQualifier = TypeQualifier->qualifier;
        mustAtGlobalNameSpace = gcvFALSE;
        break;

    case T_RESTRICT:
        storageQualifier = TypeQualifier->qualifier;
        mustAtGlobalNameSpace = gcvFALSE;
        break;

    case T_VOLATILE:
        storageQualifier = TypeQualifier->qualifier;
        mustAtGlobalNameSpace = gcvFALSE;
        break;

    case T_EOF:
        break;

    default:
        accessQualifier = TypeQualifier->qualifier;
        mustAtGlobalNameSpace = gcvFALSE;
    }

    if (!ForParamDecl) {
        if (mustAtGlobalNameSpace && !inTypeCast) {
            gcmVERIFY_OK(cloCOMPILER_AtGlobalNameSpace(Compiler, &atGlobalNameSpace));
            if (!atGlobalNameSpace) {
                gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                                cloCOMPILER_GetCurrentLineNo(Compiler),
                                                cloCOMPILER_GetCurrentStringNo(Compiler),
                                                clvREPORT_ERROR,
                                                "the \"%s\" qualifier can only be used to declare"
                                                " variables in program scope",
                                                _GetTokenName(TypeQualifier->type)));
                return gcvSTATUS_INVALID_ARGUMENT;
            }
        }
    }
    if(accessQualifier != clvQUALIFIER_NONE) {
        if(Decl->dataType->accessQualifier != clvQUALIFIER_NONE &&
           Decl->dataType->accessQualifier != accessQualifier) {
            gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                            cloCOMPILER_GetCurrentLineNo(Compiler),
                                            cloCOMPILER_GetCurrentStringNo(Compiler),
                                            clvREPORT_ERROR,
                                            "access qualifier \"%s\" "
                                            "defined prior to this qualifier \"%s\"",
                                            clGetQualifierName(Decl->dataType->accessQualifier),
                                            _GetTokenName(TypeQualifier->type)));
            return gcvSTATUS_INVALID_ARGUMENT;
        }
        else Decl->dataType->accessQualifier = accessQualifier;
    }
    if(addrSpaceQualifier != clvQUALIFIER_NONE) {
        if(Decl->dataType->addrSpaceQualifier != clvQUALIFIER_NONE) {
            gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                            cloCOMPILER_GetCurrentLineNo(Compiler),
                                            cloCOMPILER_GetCurrentStringNo(Compiler),
                                            clvREPORT_ERROR,
                                            "address space qualifier \"%s\" "
                                            "defined prior to this qualifier \"%s\"",
                                            clGetQualifierName(Decl->dataType->addrSpaceQualifier),
                                            _GetTokenName(TypeQualifier->type)));
            return gcvSTATUS_INVALID_ARGUMENT;
        }
        else Decl->dataType->addrSpaceQualifier = addrSpaceQualifier;
    }
    if(storageQualifier != clvSTORAGE_QUALIFIER_NONE) {
        if((Decl->storageQualifier & storageQualifier)) {
             gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                             cloCOMPILER_GetCurrentLineNo(Compiler),
                                             cloCOMPILER_GetCurrentStringNo(Compiler),
                                             clvREPORT_ERROR,
                                             "storage qualifier \"%s\" "
                                             "defined prior to this qualifier \"%s\"",
                                             clGetStorageQualifierName(storageQualifier),
                                             _GetTokenName(TypeQualifier->type)));
             return gcvSTATUS_INVALID_ARGUMENT;
        }
        else if((Decl->storageQualifier & clvSTORAGE_QUALIFIER_STATIC) &&
                (storageQualifier & clvSTORAGE_QUALIFIER_EXTERN)) {
             gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                             cloCOMPILER_GetCurrentLineNo(Compiler),
                                             cloCOMPILER_GetCurrentStringNo(Compiler),
                                             clvREPORT_ERROR,
                                             "storage qualifier \"%s\" "
                                             "defined prior to this qualifier \"%s\"",
                                             clGetStorageQualifierName(clvSTORAGE_QUALIFIER_STATIC),
                                             _GetTokenName(TypeQualifier->type)));
             return gcvSTATUS_INVALID_ARGUMENT;
        }
        else if((Decl->storageQualifier & clvSTORAGE_QUALIFIER_EXTERN) &&
             (storageQualifier & clvSTORAGE_QUALIFIER_STATIC)) {
             gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                             cloCOMPILER_GetCurrentLineNo(Compiler),
                                             cloCOMPILER_GetCurrentStringNo(Compiler),
                                             clvREPORT_ERROR,
                                             "storage qualifier \"%s\" "
                                             "defined prior to this qualifier \"%s\"",
                                             clGetStorageQualifierName(clvSTORAGE_QUALIFIER_EXTERN),
                                             _GetTokenName(TypeQualifier->type)));
             return gcvSTATUS_INVALID_ARGUMENT;
        }
        else Decl->storageQualifier |= storageQualifier;
    }
    return gcvSTATUS_OK;
}

clsDECL
clParseQualifiedType(
IN cloCOMPILER Compiler,
IN slsSLINK_LIST *TypeQualifierList,
IN gctBOOL ForParamDecl,
IN clsDECL * Decl
)
{
    gceSTATUS status;
    clsDATA_TYPE dataType;
    clsDECL decl;
    clsTYPE_QUALIFIER *nextDscr;
    clsTYPE_QUALIFIER *prevDscr;

    if (Decl->dataType ==  gcvNULL) return *Decl;
    dataType = *Decl->dataType;
    clmDECL_Initialize(&decl, &dataType, &Decl->array, Decl->ptrDscr, Decl->ptrDominant, Decl->storageQualifier);

    if(TypeQualifierList) {
        FOR_EACH_SLINK_NODE(TypeQualifierList, clsTYPE_QUALIFIER, prevDscr, nextDscr) {
            if(nextDscr->type == T_EOF) break;
            status = _ParseQualifiedType(Compiler,
                                         nextDscr,
                                         ForParamDecl,
                                         &decl);
            if(gcmIS_ERROR(status)) return *Decl;
        }
    }

    if (dataType.addrSpaceQualifier == clvQUALIFIER_CONSTANT) {
       dataType.accessQualifier = clvQUALIFIER_CONST;
    }

    if(ForParamDecl) {
        gctUINT stringNo = cloCOMPILER_GetCurrentStringNo(Compiler);
        gctUINT lineNo = cloCOMPILER_GetCurrentLineNo(Compiler);

        if(!clmDECL_IsPointerType(&decl) &&
           !clmDECL_IsArray(&decl)) {
            switch(clGetAddrSpaceQualifier(&decl)) {
            case clvQUALIFIER_GLOBAL:
                gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                                lineNo,
                                                stringNo,
                                                clvREPORT_ERROR,
                                                "invalid global address space qualifier specified for parameter type"));
                return decl;

            case clvQUALIFIER_CONSTANT:
                gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                                lineNo,
                                                stringNo,
                                                clvREPORT_ERROR,
                                                "invalid constant address space qualifier specified for parameter type"));
                return decl;

           case clvQUALIFIER_LOCAL:
               gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                               lineNo,
                                               stringNo,
                                               clvREPORT_ERROR,
                                               "invalid local address space qualifier specified for parameter type"));
               return decl;

           default:
               break;
           }
        }
        else {
           clsNAME_SPACE *nameSpace;
           cltQUALIFIER addrSpaceQualifier = clvQUALIFIER_NONE;

           switch(dataType.addrSpaceQualifier) {
           case clvQUALIFIER_NONE:
           case clvQUALIFIER_PRIVATE:
              nameSpace = cloCOMPILER_GetCurrentSpace(Compiler);
              if(nameSpace->scopeName && (nameSpace->scopeName->type == clvKERNEL_FUNC_NAME)) {
                  switch(clGetAddrSpaceQualifier(&decl))
                  {
                  case clvQUALIFIER_NONE:
                  case clvQUALIFIER_PRIVATE:
                      if(clmDECL_IsPointerType(&decl)) {
                          gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                                          lineNo,
                                                          stringNo,
                                                          clvREPORT_ERROR,
                                                          "kernel pointer parameters must point to"
                                                          " global, local, or constant address space"));
                           return decl;
                      }
                      else if(clmDECL_IsArray(&decl)) {
                           gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                                           lineNo,
                                                           stringNo,
                                                           clvREPORT_ERROR,
                                                           "array parameter to kernel function must be in"
                                                           " global, local, or constant address space"));
                          return decl;
                      }
                      break;

                  default:
                      break;
                  }
               }
               break;

           default:
               clGetPointedToAddrSpace(&decl, &addrSpaceQualifier);
               if(addrSpaceQualifier != clvQUALIFIER_NONE) {
                   gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                                   lineNo,
                                                   stringNo,
                                                   clvREPORT_ERROR,
                                                   "address space qualifier inappropriately specified"));
                   return decl;
               }
           }
        }
    }

    if(clmDECL_IsPointerType(&decl)) {
        slsSLINK_LIST *ptrDscr = decl.ptrDscr;

        decl.ptrDscr = gcvNULL;
        status = clMergePtrDscrToDecl(Compiler,
                                      ptrDscr,
                                      &decl,
                                      gcvTRUE);
        if (gcmIS_ERROR(status)) {
            gcmASSERT(0);
            return *Decl;
        }
    }

    if(Decl->dataType->accessQualifier != decl.dataType->accessQualifier ||
       Decl->dataType->addrSpaceQualifier != decl.dataType->addrSpaceQualifier) {
       status = cloCOMPILER_CloneDataType(Compiler,
                                          decl.dataType->accessQualifier,
                                          decl.dataType->addrSpaceQualifier,
                                          Decl->dataType,
                                          &decl.dataType);
       if(gcmIS_ERROR(status)) return *Decl;
    }
    else {
       decl.dataType = Decl->dataType;
    }

    return decl;
}

clsDECL
clParseNonStructType(
IN cloCOMPILER Compiler,
IN clsLexToken *Token
)
{
    gceSTATUS status;
    clsDATA_TYPE *dataType;
    gctINT tok;
    clsDECL decl;

    gcmASSERT(Token);

    clmDECL_Initialize(&decl, gcvNULL, (clsARRAY *)0, gcvNULL, gcvFALSE, clvSTORAGE_QUALIFIER_NONE);
    switch(Token->type) {
    case T_TYPE_NAME:
        return clParseNamedType(Compiler, Token);

    case T_FLOATNXM:
    case T_DOUBLENXM:
        return clParseMatrixType(Compiler, Token);
    }

    tok = Token->type;
    /* convert basic vector type to packed if necessary */
    if(cloCOMPILER_IsBasicTypePacked(Compiler)) {
        clsBUILTIN_DATATYPE_INFO *typeInfo;

        typeInfo = clGetBuiltinDataTypeInfo(tok);
        if(typeInfo &&
           typeInfo->type != typeInfo->dualType &&
           clmGEN_CODE_IsVectorDataType(typeInfo->dataType) &&
           !clmIsElementTypePacked(typeInfo->dataType.elementType)) {
           tok = typeInfo->dualType;
        }
    }

    status = cloCOMPILER_CreateDataType(Compiler,
                                        tok,
                                        gcvNULL,
                                        clvQUALIFIER_NONE,
                                        clvQUALIFIER_NONE,
                                        &dataType);
    if (gcmIS_ERROR(status)) return decl;

    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                                  clvDUMP_PARSER,
                                  "<DATA_TYPE line=\"%d\" string=\"%d\" name=\"%s\" />",
                                  Token->lineNo,
                                  Token->stringNo,
                                  _GetTokenName(tok)));
    return clParseCreateDeclFromDataType(Compiler, dataType);
}

/* Parse the matrix type */
clsDECL
clParseMatrixType(
IN cloCOMPILER Compiler,
IN clsLexToken *Token
)
{
    gceSTATUS status;
    clsDATA_TYPE *dataType;
    gctINT    tok;
    clsDECL decl;

    gcmASSERT(Token);
    clmDECL_Initialize(&decl, gcvNULL, (clsARRAY *)0, gcvNULL, gcvFALSE, clvSTORAGE_QUALIFIER_NONE);
    tok = Token->type;
    status = cloCOMPILER_CreateDataType(Compiler,
                                            tok,
                                            gcvNULL,
                                            clvQUALIFIER_NONE,
                                            clvQUALIFIER_NONE,
                                            &dataType);

    if (gcmIS_ERROR(status)) return decl;
/** This could be set in the function creating the data type if size were passed to it */
    dataType->matrixSize = Token->u.matrixSize;

    gcmVERIFY_OK(cloCOMPILER_Dump( Compiler,
                    clvDUMP_PARSER,
                    "<DATA_TYPE line=\"%d\" string=\"%d\" name=\"%s\" />",
                    Token->lineNo,
                    Token->stringNo,
                    _GetTokenName(tok)));
    return clParseCreateDeclFromDataType(Compiler, dataType);
}

clsDATA_TYPE *
clParseStructType(
IN cloCOMPILER Compiler,
IN clsDATA_TYPE * StructType
)
{
    gceSTATUS    status;
    clsDATA_TYPE *    dataType;

    if (StructType == gcvNULL) return gcvNULL;
    status = cloCOMPILER_CloneDataType(Compiler,
                                       clvQUALIFIER_NONE,
                                       clvQUALIFIER_NONE,
                                       StructType,
                                       &dataType);
    if (gcmIS_ERROR(status)) return gcvNULL;
    return dataType;
}

clsDECL
clParseNamedType(
IN cloCOMPILER Compiler,
IN clsLexToken * TypeName
)
{
   gceSTATUS status;
   clsDECL decl;

   gcmASSERT(TypeName && TypeName->u.typeName);

   gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                 clvDUMP_PARSER,
                 "<DATA_TYPE line=\"%d\" string=\"%d\" name=\"%s\" />",
                 TypeName->lineNo,
                 TypeName->stringNo,
                 TypeName->u.typeName->symbol));

   status = cloCOMPILER_CreateDecl(Compiler,
                                   TypeName->type,
                                   TypeName->u.typeName,
                                   TypeName->u.typeName->decl.dataType->accessQualifier,
                                   TypeName->u.typeName->decl.dataType->addrSpaceQualifier,
                                   &decl);

   if (gcmIS_ERROR(status)) {
       clmDECL_Initialize(&decl, gcvNULL, (clsARRAY *)0, gcvNULL, gcvFALSE, clvSTORAGE_QUALIFIER_NONE);
       return decl;
   }
   return decl;
}

void
clParseStructDeclBegin(
IN cloCOMPILER Compiler,
IN clsLexToken * StartToken,
IN clsLexToken * Identifier
)
{
    gceSTATUS    status;
    clsNAME_SPACE *nameSpace, *parentSpace;
    gctUINT fileNo = 0;
    gctUINT lineNo = 0;
    gctUINT colNo = 0;
    slsSLINK_LIST *ptrDscr = gcvNULL;
    cltPOOL_STRING symbol;
    clsNAME *scopeName;

    gcmASSERT(StartToken);
    parentSpace = cloCOMPILER_GetCurrentSpace(Compiler);
    if (Identifier){
        fileNo = Identifier->lineNo;
        lineNo = Identifier->lineNo;
        colNo = Identifier->stringNo;
        ptrDscr = Identifier->u.identifier.ptrDscr;
    }
    else {
        fileNo = lineNo = cloCOMPILER_GetCurrentLineNo(Compiler);
        colNo = cloCOMPILER_GetCurrentStringNo(Compiler);
    }

    symbol = Identifier ? Identifier->u.identifier.name : "";
    status = cloCOMPILER_CreateName(Compiler,
                                    lineNo,
                                    colNo,
                                    StartToken->type == T_STRUCT ? clvSTRUCT_NAME : clvUNION_NAME,
                                    gcvNULL,
                                    symbol,
                                    gcvNULL,
                                    clvEXTENSION_NONE,
                                    &scopeName);
    if (gcmIS_ERROR(status)) {
        gcmASSERT(0);
        return;
    }

    if (gcmIS_SUCCESS(gcoOS_StrCmp(symbol, "_vxc_pyramid"))) {
        scopeName->isBuiltin = gcvTRUE;
    }

    status = cloCOMPILER_CreateNameSpace(Compiler, &nameSpace);
    if (gcmIS_ERROR(status)) {
        gcmASSERT(0);
        return;
    }

    nameSpace->symbol = symbol;
    nameSpace->scopeName = scopeName;
    status = cloCOMPILER_CreateDecl(Compiler,
                                    StartToken->type,
                                    nameSpace,
                                    clvQUALIFIER_NONE,
                                    clvQUALIFIER_NONE,
                                    &scopeName->decl);
    if (gcmIS_ERROR(status)) {
        gcmASSERT(0);
        return;
    }

    status = clMergePtrDscrToDecl(Compiler,
                                  ptrDscr,
                                  &scopeName->decl,
                                  ptrDscr != gcvNULL);
    if (gcmIS_ERROR(status)) {
        gcmASSERT(0);
        return;
    }

    nameSpace->die = cloCOMPILER_AddDIE(Compiler, VSC_DI_TAG_TYPE, parentSpace->die , nameSpace->symbol, fileNo, lineNo, lineNo, colNo);

    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler, clvDUMP_PARSER, "<STRUCT_DECL>"));
}

static gctBOOL
_HasUnionType(
IN clsDECL *Decl
)
{
    if (Decl->dataType->elementType == clvTYPE_UNION) {
       return gcvTRUE;
    }
    else if (Decl->dataType->elementType == clvTYPE_STRUCT) {
       clsNAME *fieldName;

       FOR_EACH_DLINK_NODE(&Decl->dataType->u.fieldSpace->names, clsNAME, fieldName) {
          gcmASSERT(fieldName->decl.dataType);
          if(_HasUnionType(&fieldName->decl)) return gcvTRUE;
       }
    }
    return gcvFALSE;
}

static void
_ParseCheckStructNeedMemoryAllocate(
IN clsNAME *StructName
)
{
    gcmASSERT(StructName);

    if (_HasUnionType(&StructName->decl)) {
        StructName->u.typeInfo.needMemory = gcvTRUE;
    }
    return;
}

static void
_ParseCheckStructForFieldNameClash(
IN cloCOMPILER Compiler,
IN clsNAME *StructName
)
{
    if(StructName->u.typeInfo.hasUnnamedFields) {
        clsNAME *unnamedField;

        FOR_EACH_DLINK_NODE(&StructName->decl.dataType->u.fieldSpace->names, clsNAME, unnamedField) {
            if(unnamedField->symbol[0] == '\0') { /*Unnamed field */
                clsNAME *fieldInReverse;
                clsNAME *fld1, *fld2;

                FOR_EACH_DLINK_NODE(&unnamedField->decl.dataType->u.fieldSpace->names, clsNAME, fld1) {
                    if(fld1->symbol[0] == '\0') continue;
                    FOR_EACH_DLINK_NODE(&StructName->decl.dataType->u.fieldSpace->names, clsNAME, fld2) {
                        if(fld1->symbol == fld2->symbol) {
                            gctSIZE_T prefixLength;

                            if(StructName->type == clvSTRUCT_NAME) {
                                prefixLength = sizeof(cldSTRUCT_NAME_PREFIX);
                            }
                            else {
                                prefixLength = sizeof(cldUNION_NAME_PREFIX);
                            }
                            gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                                            fld1->lineNo,
                                                            fld1->stringNo,
                                                            clvREPORT_ERROR,
                                                            "unnamed struct/union field name \'%s\' clash in struct/union \'%s\'",
                                                            fld1->symbol,
                                                            StructName->symbol + prefixLength -1));
                        }
                    }
                }

                FOR_EACH_DLINK_NODE_REVERSELY(&StructName->decl.dataType->u.fieldSpace->names, clsNAME, fieldInReverse) {
                    if(fieldInReverse == unnamedField) break;
                    if(fieldInReverse->symbol[0] == '\0') { /*Unnamed field */
                        FOR_EACH_DLINK_NODE(&unnamedField->decl.dataType->u.fieldSpace->names, clsNAME, fld1) {
                            if(fld1->symbol[0] == '\0') continue;
                            FOR_EACH_DLINK_NODE(&fieldInReverse->decl.dataType->u.fieldSpace->names, clsNAME, fld2) {
                                if(fld1->symbol == fld2->symbol) {
                                    gctSIZE_T prefixLength;

                                    if(StructName->type == clvSTRUCT_NAME) {
                                        prefixLength = sizeof(cldSTRUCT_NAME_PREFIX);
                                    }
                                    else {
                                        prefixLength = sizeof(cldUNION_NAME_PREFIX);
                                    }
                                    gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                                                    fld2->lineNo,
                                                                    fld2->stringNo,
                                                                    clvREPORT_ERROR,
                                                                    "unnamed struct/uion field name \'%s\' clash with sibling\n"
                                                                    "unnamed struct/union field name in struct/union \'%s\'",
                                                                    fld2->symbol,
                                                                    StructName->symbol + prefixLength -1));
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

clsDATA_TYPE *
clParseStructDeclEnd(
IN cloCOMPILER Compiler,
IN clsLexToken * Identifier,
IN clsATTRIBUTE *Attr,
IN gceSTATUS ParsingStatus
)
{
  gceSTATUS status;
  clsDECL *decl;
  clsNAME_SPACE *prevNameSpace=gcvNULL;
  clsNAME *fieldName;

  cloCOMPILER_PopCurrentNameSpace(Compiler, &prevNameSpace);

  if(gcmIS_ERROR(ParsingStatus)) return gcvNULL;

  gcmASSERT(prevNameSpace->scopeName);

  prevNameSpace->scopeName->die = prevNameSpace->die;

  gcmVERIFY_OK(cloCOMPILER_Dump(Compiler, clvDUMP_PARSER, "</STRUCT_DECL>"));
  decl = &prevNameSpace->scopeName->decl;
  if(Attr) {
     if(Attr->specifiedAttr & clvATTR_PACKED)  {
        gctBOOL packed;

        packed = Attr->packed;
        FOR_EACH_DLINK_NODE(&decl->dataType->u.fieldSpace->names, clsNAME, fieldName) {
           gcmASSERT(fieldName->type == clvFIELD_NAME);
           fieldName->u.variableInfo.specifiedAttr |= clvATTR_PACKED;
           fieldName->context.packed = packed;
        }
     }

     if(Attr->specifiedAttr & clvATTR_ALIGNED)  {
        gctUINT16 alignment;

        alignment = Attr->alignment;
        fieldName = slsDLINK_LIST_First(&decl->dataType->u.fieldSpace->names, clsNAME);
        if(fieldName) {
           if(fieldName->u.variableInfo.specifiedAttr & clvATTR_ALIGNED) {
              if(fieldName->context.alignment < alignment) {
                 fieldName->context.alignment = alignment;
              }
           }
           else {
              fieldName->u.variableInfo.specifiedAttr |= clvATTR_ALIGNED;
              fieldName->context.alignment = alignment;
           }
        }
     }
     status = cloCOMPILER_Free(Compiler, Attr);
     if (gcmIS_ERROR(status)) return gcvNULL;
  }
  _ParseCheckStructNeedMemoryAllocate(prevNameSpace->scopeName);
  _ParseCheckStructForFieldNameClash(Compiler, prevNameSpace->scopeName);
  return decl->dataType;
}

clsDATA_TYPE *
clParseStructDeclTag(
IN cloCOMPILER Compiler,
IN clsLexToken * StartToken,
IN clsLexToken * Identifier
)
{
    gceSTATUS status;
    clsDECL decl;
    clsNAME_SPACE *    prevNameSpace=gcvNULL;

    gcmASSERT(StartToken);

    cloCOMPILER_PopCurrentNameSpace(Compiler, &prevNameSpace);

    status = cloCOMPILER_CreateDecl(Compiler,
                    StartToken->type,
                    prevNameSpace,
                    clvQUALIFIER_NONE,
                    clvQUALIFIER_NONE,
                    &decl);
    if (gcmIS_ERROR(status)) return gcvNULL;

    if (Identifier != gcvNULL) {
        status = cloCOMPILER_CreateName(Compiler, Identifier->lineNo,
                        Identifier->stringNo,
                        StartToken->type == T_STRUCT ? clvSTRUCT_NAME : clvUNION_NAME,
                        &decl,
                        Identifier->u.identifier.name,
                        Identifier->u.identifier.ptrDscr,
                        clvEXTENSION_NONE,
                        gcvNULL);
        if (gcmIS_ERROR(status)) return gcvNULL;
    }

    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler, clvDUMP_PARSER, "</STRUCT_DECL>"));
    return decl.dataType;
}

slsSLINK_LIST *
clParseAddEnumerator(
IN cloCOMPILER Compiler,
IN clsNAME *Enumerator,
slsSLINK_LIST *EnumList
)
{
   gceSTATUS status;
   clsENUMERATOR *enumerator;
   gctPOINTER pointer;

   status = cloCOMPILER_Allocate(Compiler,
                                 (gctSIZE_T)sizeof(clsENUMERATOR),
                                 (gctPOINTER *) &pointer);
   if (gcmIS_ERROR(status)) return EnumList;
   enumerator = pointer;

   if(!Enumerator->u.variableInfo.u.constant) {
      clsENUMERATOR *prevEnumerator;
      cloIR_CONSTANT constant;
      cluCONSTANT_VALUE constantValue;

      (void)gcoOS_ZeroMemory((gctPOINTER)&constantValue, sizeof(cluCONSTANT_VALUE));
      prevEnumerator = slmSLINK_LIST_Last(EnumList, clsENUMERATOR);
      if(prevEnumerator == gcvNULL) {
        constantValue.intValue = 0;
      }
      else {
        constant = prevEnumerator->member->u.variableInfo.u.constant;
        gcmASSERT(constant);
        constantValue.intValue = constant->values[0].intValue + 1;
      }
      Enumerator->u.variableInfo.u.constant = _ParseCreateConstant(Compiler,
                                                                 Enumerator->lineNo,
                                                                 Enumerator->stringNo,
                                                                 T_INT,
                                                                 &constantValue);
   }

   enumerator->member = Enumerator;
   slmSLINK_LIST_InsertLast(EnumList, &enumerator->node);
   return EnumList;
}

clsNAME *
clParseEnumerator(
IN cloCOMPILER Compiler,
IN clsLexToken *Identifier,
IN cloIR_EXPR IntExpr
)
{
   gceSTATUS status;
   clsDECL decl[1];
   clsDATA_TYPE *dataType;
   cloIR_CONSTANT enumConstant;
   clsNAME *name;

   status = cloCOMPILER_CreateDataType(Compiler,
                                       T_INT,
                                       gcvNULL,
                                       clvQUALIFIER_CONST,
                                       clvQUALIFIER_NONE,
                                       &dataType);
   if (gcmIS_ERROR(status)) return gcvNULL;

   clmDECL_Initialize(decl, dataType, (clsARRAY *)0, gcvNULL, gcvFALSE, clvSTORAGE_QUALIFIER_NONE);
   status = cloCOMPILER_CreateName(Compiler,
                                   Identifier->lineNo,
                                   Identifier->stringNo,
                                   clvENUM_NAME,
                                   decl,
                                   Identifier->u.identifier.name,
                                   Identifier->u.identifier.ptrDscr,
                                   clvEXTENSION_NONE,
                                   &name);
   if (gcmIS_ERROR(status)) return gcvNULL;

   if(IntExpr) {
     status = _CheckIntConstantExpr(Compiler, IntExpr);
     if (gcmIS_ERROR(status)) return gcvNULL;

     enumConstant = (cloIR_CONSTANT) IntExpr;
     status = clParseConstantTypeConvert(enumConstant, clvTYPE_INT, enumConstant->values);
     if (gcmIS_ERROR(status)) return gcvNULL;

     IntExpr->decl.dataType = name->decl.dataType;
     name->u.variableInfo.u.constant = (cloIR_CONSTANT)IntExpr;
   }

   return name;
}

clsDATA_TYPE *
clParseTaggedDecl(
IN cloCOMPILER Compiler,
IN clsLexToken * StartToken,
IN clsLexToken * Identifier
)
{
    gceSTATUS status;
    clsDECL decl;
    cleNAME_TYPE tagNameType;
    clsNAME *name;
    cltPOOL_STRING symbol;
    gctSTRING nameBuffer;
    gctSIZE_T length, prefixLength;
    gctSTRING prefix;
    gctPOINTER pointer;

    gcmASSERT(StartToken);
    gcmASSERT(Identifier);

    symbol = Identifier ? Identifier->u.identifier.name : "";
    switch(StartToken->type) {
    case T_STRUCT:
    case T_UNION:
        if(StartToken->type == T_STRUCT) {
            tagNameType = clvSTRUCT_NAME;
            prefix = cldSTRUCT_NAME_PREFIX;
        }
        else {
            tagNameType = clvUNION_NAME;
             prefix = cldUNION_NAME_PREFIX;
        }
        gcoOS_StrLen(prefix, &prefixLength);
        gcoOS_StrLen(symbol, &length);

        length += prefixLength + 1;
        status = cloCOMPILER_Allocate(Compiler,
                                      length,
                                      &pointer);
        if (gcmIS_ERROR(status))  return NULL;
        nameBuffer = pointer;

        gcmVERIFY_OK(gcoOS_StrCopySafe(nameBuffer, length, prefix));
        gcmVERIFY_OK(gcoOS_StrCatSafe(nameBuffer, length, symbol));

        status = cloCOMPILER_AllocatePoolString(Compiler,
                                                nameBuffer,
                                                &symbol);
        gcmVERIFY_OK(cloCOMPILER_Free(Compiler, pointer));
        if (gcmIS_ERROR(status)) return NULL;
        break;
    case T_ENUM:
        tagNameType = clvENUM_TAG_NAME;
        break;
    case T_TYPE_NAME:
        tagNameType = clvTYPE_NAME;
        break;
    default:
        gcmASSERT(0);
        tagNameType = clvTYPE_NAME;
        break;
    }
    if (Identifier != gcvNULL) {
       status = cloCOMPILER_SearchName(Compiler, symbol, gcvTRUE, &name);

       if (status == gcvSTATUS_OK) {
          if(tagNameType != name->type) {
              gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                              Identifier->lineNo,
                              Identifier->stringNo,
                              clvREPORT_ERROR,
                              "tag name \'%s\' already used",
                              Identifier->u.identifier.name));
              return gcvNULL;
          }
          return name->decl.dataType;
       }
    }
    else
    {
        return gcvNULL;
    }

    clmDECL_Initialize(&decl, gcvNULL, (clsARRAY *)0, gcvNULL, gcvFALSE, clvSTORAGE_QUALIFIER_NONE);
    status = cloCOMPILER_CreateName(Compiler,
                    Identifier->lineNo,
                    Identifier->stringNo,
                    tagNameType,
                    &decl,
                    Identifier->u.identifier.name,
                    gcvNULL,
                    clvEXTENSION_NONE,
                    &name);
    if (gcmIS_ERROR(status)) return gcvNULL;

    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler, clvDUMP_PARSER, "<DECL_TAG>"));
    return name->decl.dataType;
}

clsDATA_TYPE *
clParseEnumSpecifier(
IN cloCOMPILER Compiler,
IN clsLexToken * StartToken,
IN clsATTRIBUTE * Attr,
IN clsLexToken * Identifier,
IN gctPOINTER Generic
)
{
    gceSTATUS status;
    clsDECL decl;
    clsNAME *name = gcvNULL;

    gcmASSERT(StartToken);

    gcmASSERT(StartToken->type == T_ENUM);

    clmDECL_Initialize(&decl, gcvNULL, (clsARRAY *)0, gcvNULL, gcvFALSE, clvSTORAGE_QUALIFIER_NONE);
    if (Identifier != gcvNULL) {
        status = cloCOMPILER_SearchName(Compiler, Identifier->u.identifier.name, gcvTRUE, &name);

        if (status == gcvSTATUS_OK) {
            if(name->type != clvENUM_TAG_NAME) {
                gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                             Identifier->lineNo,
                             Identifier->stringNo,
                             clvREPORT_ERROR,
                             "enum name \'%s\' already used",
                             Identifier->u.identifier.name));
                return gcvNULL;
            }
            if(Generic) {
                if(name->context.u.enumerator) {
                    gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                    Identifier->lineNo,
                                    Identifier->stringNo,
                                    clvREPORT_ERROR,
                                    "Redefinition of enum \'%s\'",
                                    Identifier->u.identifier.name));
                    return gcvNULL;
                }
                name->context.u.enumerator = Generic;
            }
            decl = name->decl;
        }
        else {  /* name not yet defined */
            if(!Generic) {
                gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                Identifier->lineNo,
                                Identifier->stringNo,
                                clvREPORT_ERROR,
                                "Enum tag \'%s\' referenced before its specification is complete",
                                Identifier->u.identifier.name));
                return gcvNULL;
            }
        }
    }

    if(!name) {
        gctSTRING enumName = "";
        if(Identifier) enumName = Identifier->u.identifier.name;

        status = cloCOMPILER_CreateDecl(Compiler,
                                        StartToken->type,
                                        gcvNULL,
                                        clvQUALIFIER_NONE,
                                        clvQUALIFIER_NONE,
                                        &decl);
        if (gcmIS_ERROR(status)) return gcvNULL;

        status = cloCOMPILER_CreateName(Compiler,
                                        StartToken->lineNo,
                                        StartToken->stringNo,
                                        clvENUM_TAG_NAME,
                                        &decl,
                                        enumName,
                                        gcvNULL,
                                        clvEXTENSION_NONE,
                                        &name);
        if (gcmIS_ERROR(status)) return gcvNULL;
        name->context.u.enumerator = Generic;
        decl = name->decl;
    }

    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler, clvDUMP_PARSER, "<DECL_TAG>"));
    if(Attr) {
        status = _ParseFillEnumAttr(Compiler, name, &decl, Attr);
        if (gcmIS_ERROR(status)) {
            return decl.dataType;
        }
    }

    return decl.dataType;
}

gceSTATUS
clParseTypeSpecifiedFieldDeclList(
IN cloCOMPILER Compiler,
IN clsDECL * Decl,
IN slsDLINK_LIST * FieldDeclList
)
{
   gceSTATUS  status = gcvSTATUS_OK;
   gceSTATUS  rtnStatus;
   clsDECL decl[1];
   clsDECL *declPtr = Decl;
   clsNAME *derivedType = gcvNULL;
   clsFieldDecl *    fieldDecl = gcvNULL;
   slsSLINK_LIST *ptrDscr;
   clsDATA_TYPE *dataType;

   if (Decl->dataType == gcvNULL || FieldDeclList == gcvNULL)
       return gcvSTATUS_INVALID_DATA;

   if(Decl->dataType->type == T_TYPE_NAME) {
      gcmONERROR(_ParseFlattenType(Compiler, Decl, decl));
      declPtr = decl;
      derivedType = Decl->dataType->u.typeDef;
   }
   else if(Decl->dataType->type == T_ENUM) {
      derivedType = Decl->dataType->u.enumerator;
   }
   else {
      declPtr = _HandleSpecialType(Compiler, declPtr);
   }

   do {
      clsNAME_SPACE * currentSpace = cloCOMPILER_GetCurrentSpace(Compiler);

      if (clmDECL_IsVoid(declPtr)) {
          fieldDecl = slsDLINK_LIST_First(FieldDeclList, clsFieldDecl);
          gcmASSERT(fieldDecl);
          gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                          fieldDecl->field->lineNo,
                                          fieldDecl->field->stringNo,
                                          clvREPORT_ERROR,
                                          "'%s' can not be of type void",
                                          fieldDecl->field->symbol));
          status = gcvSTATUS_INVALID_DATA;
          break;
      }

      if(declPtr->dataType->type == T_IMAGE2D_DYNAMIC_ARRAY_T) {
          if(currentSpace &&
             !gcmIS_SUCCESS(gcoOS_StrCmp(currentSpace->symbol, "_vxc_pyramid"))) {
               gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                          fieldDecl->field->lineNo,
                                          fieldDecl->field->stringNo,
                                          clvREPORT_ERROR,
                                          "unrecognizable type '_viv_image2d_array_t' specified for struct field '%s'",
                                          fieldDecl->field->symbol));
               status = gcvSTATUS_INVALID_DATA;
               break;
          }
      }

      if(clmDECL_IsStructOrUnion(declPtr)) {
         gctBOOL hasUnion = gcvFALSE;

         gcmASSERT(currentSpace->scopeName);
         gcmASSERT(clmDECL_IsStructOrUnion(&currentSpace->scopeName->decl));

         if(clmDECL_IsStruct(declPtr)) {
             hasUnion = declPtr->dataType->u.fieldSpace->scopeName->u.typeInfo.hasUnionFields;
         }
         currentSpace->scopeName->u.typeInfo.hasUnionFields = clmDECL_IsUnion(declPtr) || hasUnion;

         if(gcmIS_SUCCESS(gcoOS_StrCmp(declPtr->dataType->u.fieldSpace->scopeName->symbol, "_vxc_pyramid"))) {
             fieldDecl = slsDLINK_LIST_First(FieldDeclList, clsFieldDecl);
             gcmASSERT(fieldDecl);
             gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                             fieldDecl->field->lineNo,
                                             fieldDecl->field->stringNo,
                                             clvREPORT_ERROR,
                                             "struct/union field '%s' cannot have '%s' type",
                                             fieldDecl->field->symbol,
                                             "vxc_pyramid"));
              status = gcvSTATUS_INVALID_DATA;
              break;
          }
      }
      if(clmDECL_IsImage(declPtr)) {
          fieldDecl = slsDLINK_LIST_First(FieldDeclList, clsFieldDecl);
          gcmASSERT(fieldDecl);
          gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                          fieldDecl->field->lineNo,
                                          fieldDecl->field->stringNo,
                                          clvREPORT_ERROR,
                                          "struct/union field '%s' cannot have image type",
                                          fieldDecl->field->symbol));
          status = gcvSTATUS_INVALID_DATA;
          break;
      }

      if(declPtr->storageQualifier != clvQUALIFIER_NONE) {
          fieldDecl = slsDLINK_LIST_First(FieldDeclList, clsFieldDecl);
          gcmASSERT(fieldDecl);
          gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                          fieldDecl->field->lineNo,
                                          fieldDecl->field->stringNo,
                                          clvREPORT_ERROR,
                                          "struct field '%s' cannot have storage qualifier '%s'",
                                          fieldDecl->field->symbol,
                                          clGetQualifierName(declPtr->storageQualifier)));
          status = gcvSTATUS_INVALID_DATA;
          break;
      }

      ptrDscr = declPtr->ptrDscr;
      dataType = declPtr->dataType;
      FOR_EACH_DLINK_NODE(FieldDeclList, clsFieldDecl, fieldDecl) {
          declPtr->dataType = dataType;
          if(!slmSLINK_LIST_IsEmpty(ptrDscr)) {
              status = cloCOMPILER_ClonePtrDscr(Compiler,
                                                ptrDscr,
                                                &declPtr->ptrDscr);
              if (gcmIS_ERROR(status)) return status;
          }
          status = clMergePtrDscrToDecl(Compiler,
                                        fieldDecl->field->decl.ptrDscr,
                                        declPtr,
                                        fieldDecl->array.numDim == 0);
          if (gcmIS_ERROR(status)) return status;

          fieldDecl->field->decl = *declPtr;

          if (fieldDecl->array.numDim != 0) {
              /* may need to handle array of pointers to array */
              status = _ParseMergeArrayDecl(Compiler,
                                            &fieldDecl->field->decl,
                                            &fieldDecl->array,
                                            &fieldDecl->field->decl);
              if (gcmIS_ERROR(status)) {
                  fieldDecl->field->decl.dataType = declPtr->dataType;
                  break;
              }
          }
          if(derivedType) {
              gcmONERROR(_ParseMergeTypeAttrToVariable(derivedType, fieldDecl->field));
              fieldDecl->field->derivedType = derivedType;
          }

          if(clmDECL_IsUnderlyingVectorType(&fieldDecl->field->decl) && cloCOMPILER_IsBasicTypePacked(Compiler)) {
              fieldDecl->field->context.packed = gcvTRUE;
          }
          if(fieldDecl->field->context.packed &&
             cloCOMPILER_ExtensionEnabled(Compiler, clvEXTENSION_VIV_VX)) {
              gcmONERROR(_ConvDataTypeToPacked(Compiler, fieldDecl->field));
          }
      }
   } while (gcvFALSE);

OnError:
   rtnStatus = status;
   while (!slsDLINK_LIST_IsEmpty(FieldDeclList)) {
      slsDLINK_LIST_DetachFirst(FieldDeclList, clsFieldDecl, &fieldDecl);

      gcmVERIFY_OK(cloCOMPILER_Free(Compiler, fieldDecl));
   }
   gcmVERIFY_OK(cloCOMPILER_Free(Compiler, FieldDeclList));
   return rtnStatus;
}

slsDLINK_LIST *
clParseFieldDeclList(
IN cloCOMPILER Compiler,
IN clsFieldDecl * FieldDecl
)
{
    gceSTATUS  status;
    slsDLINK_LIST *    fieldDeclList;
    gctPOINTER pointer;

    status = cloCOMPILER_Allocate(Compiler,
                    (gctSIZE_T)sizeof(slsDLINK_LIST),
                    (gctPOINTER *) &pointer);

    if (gcmIS_ERROR(status)) return gcvNULL;
    fieldDeclList = pointer;

    slsDLINK_LIST_Initialize(fieldDeclList);
    if (FieldDecl != gcvNULL) {
        slsDLINK_LIST_InsertLast(fieldDeclList, &FieldDecl->node);
    }
    return fieldDeclList;
}

slsDLINK_LIST *
clParseFieldDeclList2(
IN cloCOMPILER Compiler,
IN slsDLINK_LIST * FieldDeclList,
IN clsFieldDecl * FieldDecl
)
{
    if (FieldDeclList != gcvNULL && FieldDecl != gcvNULL) {
        slsDLINK_LIST_InsertLast(FieldDeclList, &FieldDecl->node);
    }
    return FieldDeclList;
}


clsFieldDecl *
clParseFieldDecl(
IN cloCOMPILER Compiler,
IN clsLexToken * Identifier,
IN cloIR_EXPR ArrayLengthExpr,
IN clsATTRIBUTE *Attr
)
{
    gceSTATUS  status;
    clsNAME *field;
    clsFieldDecl *fieldDecl;
    gctPOINTER pointer;
    slsSLINK_LIST *ptrDscr = gcvNULL;
    cltPOOL_STRING symbol;
    gctUINT lineNo, stringNo;

    if(Identifier == gcvNULL) {
        clsNAME_SPACE *fieldSpace;

        lineNo = cloCOMPILER_GetCurrentLineNo(Compiler);
        stringNo = cloCOMPILER_GetCurrentStringNo(Compiler);
        if(_RELAX_SYNTAX_FOR_EMBEDDED_UNNAMED_STRUCT_OR_UNION) {
           gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                           lineNo,
                                           stringNo,
                                           clvREPORT_WARN,
                                           "non-compliant extension: unnamed struct/union field"));
           status = cloCOMPILER_AllocatePoolString(Compiler,
                                                   "",
                                                   &symbol);
           if (gcmIS_ERROR(status)) return gcvNULL;
        }
        else {
           gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                           lineNo,
                                           stringNo,
                                           clvREPORT_ERROR,
                                           "syntax error: struct/union field name expected"));
           return gcvNULL;
        }

        fieldSpace = cloCOMPILER_GetCurrentSpace(Compiler);
        gcmASSERT(fieldSpace && fieldSpace->scopeName);
        gcmASSERT(clmDECL_IsStructOrUnion(&fieldSpace->scopeName->decl));
        fieldSpace->scopeName->u.typeInfo.hasUnnamedFields = gcvTRUE;
    }
    else {
        ptrDscr = Identifier->u.identifier.ptrDscr;
        symbol = Identifier->u.identifier.name;
        lineNo = Identifier->lineNo;
        stringNo = Identifier->stringNo;
    }
    status = cloCOMPILER_CreateName(Compiler,
                                    lineNo,
                                    stringNo,
                                    clvFIELD_NAME,
                                    gcvNULL,
                                    symbol,
                                    ptrDscr,
                                    clvEXTENSION_NONE,
                                    &field);
    if (gcmIS_ERROR(status)) return gcvNULL;

    status = cloCOMPILER_Allocate(Compiler,
                    (gctSIZE_T)sizeof(clsFieldDecl),
                    (gctPOINTER *) &pointer);
    if (gcmIS_ERROR(status)) return gcvNULL;
    fieldDecl = pointer;

    fieldDecl->field = field;
    if (ArrayLengthExpr == gcvNULL) {
        fieldDecl->array.numDim = 0;
    }
    else {
        clmEvaluateExprToArrayLength(Compiler,
                         ArrayLengthExpr,
                         &fieldDecl->array,
                         gcvFALSE,
                         status);
        if (gcmIS_ERROR(status)) {
            gcmASSERT(fieldDecl->array.numDim == 0);
            return fieldDecl;
        }
    }

    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                      clvDUMP_PARSER,
                      "<FIELD line=\"%d\" string=\"%d\" name=\"%s\" />",
                      lineNo,
                      stringNo,
                      symbol));
    _ParseFillVariableAttr(Compiler,
                           Identifier->lineNo,
                           Identifier->stringNo,
                           gcvNULL, field, Attr);
    return fieldDecl;
}

slsSLINK_LIST *
clParseEmptyTypeQualifierList(
IN cloCOMPILER Compiler
)
{
    gceSTATUS status;
    clsTYPE_QUALIFIER *typeQualifier;
    slsSLINK_LIST *typeQualifierList;
    gctPOINTER pointer;

    slmSLINK_LIST_Initialize(typeQualifierList);
    status = cloCOMPILER_Allocate(Compiler,
                                  (gctSIZE_T)sizeof(clsTYPE_QUALIFIER),
                                  (gctPOINTER *) &pointer);
    if (gcmIS_ERROR(status)) return gcvNULL;
    typeQualifier = pointer;
    typeQualifier->type = T_EOF;
    typeQualifier->qualifier = clvQUALIFIER_NONE;
    slmSLINK_LIST_InsertFirst(typeQualifierList, &typeQualifier->node);

    return typeQualifierList;
}


slsSLINK_LIST *
clParseTypeQualifierList(
IN cloCOMPILER Compiler,
IN clsLexToken *OneQualifier,
IN slsSLINK_LIST *TypeQualifierList
)
{
   gceSTATUS status;
   clsTYPE_QUALIFIER *typeQualifier;
   gctPOINTER pointer;

   gcmASSERT(OneQualifier);
   gcmASSERT(TypeQualifierList);

   status = cloCOMPILER_Allocate(Compiler,
                                 (gctSIZE_T)sizeof(clsTYPE_QUALIFIER),
                                 (gctPOINTER *) &pointer);
   if (gcmIS_ERROR(status)) return TypeQualifierList;
   typeQualifier = pointer;

   if (typeQualifier != gcvNULL) {
      typeQualifier->type = OneQualifier->type;
      typeQualifier->qualifier = OneQualifier->u.qualifier;
      slmSLINK_LIST_InsertFirst(TypeQualifierList, &typeQualifier->node);
   }
   return  TypeQualifierList;
}

slsSLINK_LIST *
clParsePointerTypeQualifier(
IN cloCOMPILER Compiler,
IN slsSLINK_LIST *TypeQualifierList,
IN slsSLINK_LIST *PtrDscr
)
{
   slsSLINK_LIST *typeQualifierList;
   gcmASSERT(PtrDscr);

   typeQualifierList = TypeQualifierList;
   if(typeQualifierList == gcvNULL) {
       typeQualifierList = clParseEmptyTypeQualifierList(Compiler);
   }

/* Put pointer descriptors at head of new type qualifier list to maintain first in
   first out for expression evaluation later */
   slmSLINK_LIST_Prepend(PtrDscr, typeQualifierList);
   return typeQualifierList;
}

clsNAME *
clParseMakeFakeMain(
IN cloCOMPILER Compiler,
IN clsNAME *TopKernelFunc
)
{
    gceSTATUS status;
    clsNAME *funcName;
    clsDATA_TYPE *dataType;
    clsNAME *paramName;
    clsLexToken token;
    cloIR_BASE currStatement;
    cloIR_SET  statementList;
    cloIR_POLYNARY_EXPR funcCall;
    clsDECL decl;

    gcmASSERT(TopKernelFunc);

/** Create uniform variables for calling arguments to the top kernel function **/
    FOR_EACH_DLINK_NODE(&TopKernelFunc->u.funcInfo.localSpace->names, clsNAME, paramName) {
      if (paramName->type == clvPARAMETER_NAME) {
        clsDeclOrDeclList *varDecl;

        _clmMakeIdentifierToken(token, paramName->symbol);
        if(paramName->decl.ptrDscr)  {
            status = cloCOMPILER_ClonePtrDscr(Compiler,
                                              paramName->decl.ptrDscr,
                                              &token.u.identifier.ptrDscr);
            if (gcmIS_ERROR(status)) return gcvNULL;
        }
        status = cloCOMPILER_CloneDataType(Compiler,
                                           clvQUALIFIER_UNIFORM,
                                           paramName->decl.dataType->addrSpaceQualifier,
                                           paramName->decl.dataType,
                                           &dataType);
        if (gcmIS_ERROR(status)) return gcvNULL;

        clmDECL_Initialize(&decl, dataType, &paramName->decl.array, gcvNULL, gcvFALSE, clvSTORAGE_QUALIFIER_NONE);
        varDecl = clParseVariableDecl(Compiler,
                                      &decl,
                                      &token,
                                      gcvNULL);
        if(varDecl == gcvNULL) return gcvNULL;
        varDecl->name->u.variableInfo.builtinSpecific.s.variableType = clvBUILTIN_KERNEL_ARG;
        currStatement = clParseDeclaration(Compiler,
                                           varDecl);
      }
      else continue;
    }

    _clmMakeIdentifierToken(token, "main");
    status = cloCOMPILER_CreateDataType(Compiler,
                                        T_VOID,
                                        gcvNULL,
                                        clvQUALIFIER_CONST,
                                        clvQUALIFIER_NONE,
                                        &dataType);
    if (gcmIS_ERROR(status)) return gcvNULL;
    clmDECL_Initialize(&decl, dataType, (clsARRAY *)0, gcvNULL, gcvFALSE, clvSTORAGE_QUALIFIER_NONE);
    funcName = clParseFuncHeader(Compiler, &decl, &token);

/** Create top kernel function call **/
    _clmMakeIdentifierToken(token, TopKernelFunc->symbol);
    funcCall = clParseFuncCallHeaderExpr(Compiler, &token, gcvNULL);

/** Create parameters for the top kernel function call **/
    FOR_EACH_DLINK_NODE(&TopKernelFunc->u.funcInfo.localSpace->names, clsNAME, paramName) {
      if (paramName->type == clvPARAMETER_NAME) {
        cloIR_EXPR currArg;

        _clmMakeIdentifierToken(token, paramName->symbol);
        currArg = clParseVariableIdentifier(Compiler, &token);
        funcCall = clParseFuncCallArgument(Compiler, funcCall, currArg);
      }
      else continue;
    }

    currStatement = clParseExprAsStatement(Compiler,
                                           clParseFuncCallExprAsExpr(Compiler, funcCall));
    statementList = clParseStatementList(Compiler, currStatement);
    clParseFuncDef(Compiler, funcName, statementList);
    return funcName;
}
