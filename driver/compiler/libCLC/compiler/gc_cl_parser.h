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


#ifndef __gc_cl_parser_h_
#define __gc_cl_parser_h_

#include "gc_cl_scanner.h"
#include "gc_cl_built_ins.h"

cloIR_EXPR
clParseVariableIdentifier(
    IN cloCOMPILER Compiler,
    IN clsLexToken * Identifier
    );

cloIR_EXPR
clParseScalarConstant(
    IN cloCOMPILER Compiler,
    IN clsLexToken * IntConstant
    );

cloIR_EXPR
clParseStringLiteral(
    IN cloCOMPILER Compiler,
    IN clsLexToken * StringLiteral
    );

clsLexToken
clParseCatStringLiteral(
    IN cloCOMPILER Compiler,
    IN clsLexToken * FirstStr,
    IN clsLexToken * SecondStr
    );

clsATTRIBUTE *
clParseAttributeEndianType(
    IN cloCOMPILER Compiler,
    IN clsATTRIBUTE *Attr,
    IN clsLexToken *EndianType
    );

clsATTRIBUTE *
clParseAttributeAligned(
    IN cloCOMPILER Compiler,
    IN clsATTRIBUTE *Attr,
    IN cloIR_EXPR Aligment
    );

clsATTRIBUTE *
clParseAttributeVecTypeHint(
    IN cloCOMPILER Compiler,
    IN clsATTRIBUTE *Attr,
    IN clsLexToken *DataType
    );

clsATTRIBUTE *
clParseAttributeReqdWorkGroupSize(
    IN cloCOMPILER Compiler,
    IN clsATTRIBUTE *Attr,
    IN cloIR_EXPR X,
    IN cloIR_EXPR Y,
    IN cloIR_EXPR Z
    );

clsATTRIBUTE *
clParseAttributeWorkGroupSizeHint(
    IN cloCOMPILER Compiler,
    IN clsATTRIBUTE *Attr,
    IN cloIR_EXPR X,
    IN cloIR_EXPR Y,
    IN cloIR_EXPR Z
    );

clsATTRIBUTE *
clParseAttributeKernelScaleHint(
    IN cloCOMPILER Compiler,
    IN clsATTRIBUTE *Attr,
    IN cloIR_EXPR X,
    IN cloIR_EXPR Y,
    IN cloIR_EXPR Z
    );

clsATTRIBUTE *
clParseSimpleAttribute(
    IN cloCOMPILER Compiler,
    IN clsLexToken *StartToken,
    IN cltATTR_FLAGS AttrType,
    IN clsATTRIBUTE *Attr
    );

cloIR_EXPR
clParseSubscriptExpr(
    IN cloCOMPILER Compiler,
    IN cloIR_EXPR LeftOperand,
    IN cloIR_EXPR RightOperand
    );

cloIR_EXPR
clParseNullExpr(
    IN cloCOMPILER Compiler,
    IN clsLexToken *StartToken
    );

cloIR_EXPR
clParseArrayDeclarator(
    IN cloCOMPILER Compiler,
    IN clsLexToken *StartToken,
    IN cloIR_EXPR ArrayDecl,
    IN cloIR_EXPR ArraySize
    );

gceSTATUS
clParseCheckReturnExpr(
    IN cloCOMPILER Compiler,
    IN clsDECL *RtnDecl,
    IN cloIR_EXPR RtnExpr
    );

cloIR_EXPR
clParseFuncCallExprAsExpr(
    IN cloCOMPILER Compiler,
    IN cloIR_POLYNARY_EXPR FuncCall
    );

cloIR_POLYNARY_EXPR
clParseFuncCallHeaderExpr(
    IN cloCOMPILER Compiler,
    IN clsLexToken * FuncIdentifier,
    IN clsARRAY *Array
    );

cloIR_POLYNARY_EXPR
clParseFuncCallArgument(
    IN cloCOMPILER Compiler,
    IN cloIR_POLYNARY_EXPR FuncCall,
    IN cloIR_EXPR Argument
    );

cloIR_EXPR
clParseFieldSelectionExpr(
    IN cloCOMPILER Compiler,
    IN cloIR_EXPR Operand,
    IN clsLexToken * FieldSelection
    );

cloIR_EXPR
clParsePtrFieldSelectionExpr(
    IN cloCOMPILER Compiler,
    IN cloIR_EXPR Operand,
    IN clsLexToken * FieldSelection
    );

cloIR_EXPR
clParseIncOrDecExpr(
    IN cloCOMPILER Compiler,
    IN clsLexToken * StartToken,
    IN cleUNARY_EXPR_TYPE ExprType,
    IN cloIR_EXPR Operand
    );

cloIR_EXPR
clParseNormalUnaryExpr(
    IN cloCOMPILER Compiler,
    IN clsLexToken * Operator,
    IN cloIR_EXPR Operand
    );

gctINT
clParseCountIndirectionLevel(
IN slsSLINK_LIST *ptrDscr
);

void
clParseRemoveIndirectionOneLevel(
IN cloCOMPILER Compiler,
IN slsSLINK_LIST **ptrDscr
);

gceSTATUS
clParseAddIndirectionOneLevel(
IN cloCOMPILER Compiler,
IN slsSLINK_LIST **ptrDscr
);

clsNAME *
clParseFindLeafName(
IN cloCOMPILER Compiler,
IN cloIR_EXPR Expr
);

clsNAME *
clParseFindPointerVariable(
IN cloCOMPILER Compiler,
IN cloIR_EXPR Expr
);

gceSTATUS
clParseSetOperandAddressed(
IN cloCOMPILER Compiler,
IN cloIR_EXPR Operand
);

gceSTATUS
clParseSetOperandDirty(
IN cloCOMPILER Compiler,
IN cloIR_EXPR LeftOperand,
IN cloIR_EXPR RightOperand
);

clsDECL
clParseCreateDeclFromDataType(
    IN cloCOMPILER Compiler,
    IN clsDATA_TYPE *DataType
    );

clsDECL
clParseCreateDeclFromExpression(
    IN cloCOMPILER Compiler,
    IN cloIR_EXPR Expr
    );

clsDECL
clParseCreateDecl(
IN cloCOMPILER Compiler,
IN clsDECL *Decl,
IN slsSLINK_LIST *ptrDscr,
IN cloIR_EXPR ArrayLengthExpr
);

gceSTATUS
cloCOMPILER_SetParserState(
    IN cloCOMPILER Compiler,
    IN clePARSER_STATE State
    );

gceSTATUS
cloCOMPILER_PushParserState(
    IN cloCOMPILER Compiler,
    IN clePARSER_STATE State
    );

gceSTATUS
cloCOMPILER_PopParserState(
    IN cloCOMPILER Compiler
    );

clsPARSER_STATE *
cloCOMPILER_GetParserStateHandle(
IN cloCOMPILER Compiler
);

clePARSER_STATE
cloCOMPILER_GetParserState(
    IN cloCOMPILER Compiler
    );

void
clParseCastExprBegin(
IN cloCOMPILER Compiler,
IN clsDECL *CastType
);

cloIR_EXPR
clParseCastExprEnd(
    IN cloCOMPILER Compiler,
    IN clsDECL *Decl,
    IN cloIR_EXPR Operand
    );

cloIR_EXPR
clParseSizeofTypeDecl(
    IN cloCOMPILER Compiler,
    IN clsLexToken *StartToken,
    IN clsDECL *Decl
    );

cloIR_EXPR
clParseSizeofExpr(
IN cloCOMPILER Compiler,
IN clsLexToken *StartToken,
IN cloIR_EXPR Expr
);

cloIR_EXPR
clParseVecStep(
IN cloCOMPILER Compiler,
IN clsLexToken *StartToken,
IN clsDECL *Decl
);

cloIR_EXPR
clParseBinarySequenceExpr(
IN cloCOMPILER Compiler,
IN YYSTYPE *ParseStack,
IN cloIR_EXPR LeftOperand,
IN clsLexToken * Operator,
IN cloIR_EXPR RightOperand
);

cloIR_EXPR
clParseNormalBinaryExpr(
    IN cloCOMPILER Compiler,
    IN cloIR_EXPR LeftOperand,
    IN clsLexToken * Operator,
    IN cloIR_EXPR RightOperand
    );

cloIR_EXPR
clParseSelectionExpr(
    IN cloCOMPILER Compiler,
    IN cloIR_EXPR CondExpr,
    IN cloIR_EXPR TrueOperand,
    IN cloIR_EXPR FalseOperand
    );

cloIR_EXPR
clParseAssignmentExpr(
    IN cloCOMPILER Compiler,
    IN cloIR_EXPR LeftOperand,
    IN clsLexToken * Operator,
    IN cloIR_EXPR RightOperand
    );

slsSLINK_LIST *
clParseEmptyTypeQualifierList(
    IN cloCOMPILER Compiler
    );

slsSLINK_LIST *
clParseTypeQualifierList(
    IN cloCOMPILER Compiler,
    IN clsLexToken *NewQualifier,
    IN slsSLINK_LIST *QualifierList
    );

slsSLINK_LIST *
clParsePointerTypeQualifier(
    IN cloCOMPILER Compiler,
    IN slsSLINK_LIST *QualifierList,
    IN slsSLINK_LIST *Pointer
    );

clsDeclOrDeclList *
clParseVariableDecl(
    IN cloCOMPILER Compiler,
    IN clsDECL * Decl,
    IN clsLexToken * Identifier,
    IN clsATTRIBUTE *Attr
    );

clsDeclOrDeclList *
clParseArrayVariableDecl(
    IN cloCOMPILER Compiler,
    IN clsDECL * Decl,
    IN clsLexToken * Identifier,
    IN cloIR_EXPR ArrayLengthExpr,
    IN clsATTRIBUTE *Attr
    );

clsDeclOrDeclList *
clParseVariableDeclList(
    IN cloCOMPILER Compiler,
    IN clsDeclOrDeclList *DeclOrDeclListPtr,
    IN clsLexToken * Identifier,
    IN clsATTRIBUTE *Attr
    );

clsDeclOrDeclList *
clParseArrayVariableDeclList(
    IN cloCOMPILER Compiler,
    IN clsDeclOrDeclList *DeclOrDeclListPtr,
    IN clsLexToken * Identifier,
    IN cloIR_EXPR ArrayLengthExpr,
    IN clsATTRIBUTE *Attr
    );

clsDeclOrDeclList *
clParseVariableDeclInit(
    IN cloCOMPILER Compiler,
    IN clsDECL * Decl,
    IN clsLexToken * Identifier,
    IN clsATTRIBUTE *Attr
    );

clsDeclOrDeclList *
clParseVariableDeclListInit(
    IN cloCOMPILER Compiler,
    IN clsDeclOrDeclList *DeclOrDeclListPtr,
    IN clsLexToken * Identifier,
    IN clsATTRIBUTE *Attr
    );

clsDeclOrDeclList *
clParseArrayVariableDeclInit(
    IN cloCOMPILER Compiler,
    IN clsDECL * Decl,
    IN clsLexToken * Identifier,
    IN cloIR_EXPR ArrayDecl,
    IN clsATTRIBUTE *Attr
    );

clsDeclOrDeclList *
clParseArrayVariableDeclListInit(
    IN cloCOMPILER Compiler,
    IN clsDeclOrDeclList *DeclOrDeclListPtr,
    IN clsLexToken * Identifier,
    IN cloIR_EXPR ArrayDecl,
    IN clsATTRIBUTE *Attr
    );

cloIR_EXPR
clParseInitializerList(
IN cloCOMPILER Compiler,
IN cloIR_EXPR InitExprList,
IN clsDeclOrDeclList *Designation,
IN cloIR_EXPR InitExpr
);

clsDeclOrDeclList *
clParseFinishDeclInit(
IN cloCOMPILER Compiler,
IN clsDeclOrDeclList *Designation,
IN cloIR_EXPR InitExpr
);

clsDeclOrDeclList *
clParseFinishDeclListInit(
IN cloCOMPILER Compiler,
IN clsDeclOrDeclList *Designation,
IN cloIR_EXPR InitExpr
);

cloIR_EXPR
clParseInitializerList(
IN cloCOMPILER Compiler,
IN cloIR_EXPR InitializerList,
IN clsDeclOrDeclList *Designation,
IN cloIR_EXPR InitExpr
);

clsDeclOrDeclList *
clParseSubscriptDesignator(
IN cloCOMPILER Compiler,
IN clsDeclOrDeclList *Designator,
IN cloIR_EXPR Subscript,
IN gctINT TokenType
);

clsDeclOrDeclList *
clParseFieldSelectionDesignator(
IN cloCOMPILER Compiler,
IN clsDeclOrDeclList *Designator,
IN clsLexToken *FieldSelection,
IN gctINT TokenType
);

gceSTATUS
clParseConvertConstantValues(
IN gctUINT ValueCount,
IN cltELEMENT_TYPE SourceType,
IN cluCONSTANT_VALUE *SourceValues,
IN cltELEMENT_TYPE ToType,
IN cluCONSTANT_VALUE *ToValues
);

gceSTATUS
clParseConstantTypeConvert(
IN cloIR_CONSTANT Constant,
IN cltELEMENT_TYPE ConversionType,
IN cluCONSTANT_VALUE *Result
);

clsDeclOrDeclList *
clParseInitializeCurrentObject(
IN cloCOMPILER Compiler,
IN clsDeclOrDeclList *DeclOrDeclListPtr,
IN cloIR_EXPR InitExpr
);

clsNAME *
clParseFuncHeader(
    IN cloCOMPILER Compiler,
    IN clsDECL * Decl,
    IN clsLexToken * Identifier
    );

clsNAME *
clParseFuncHeaderWithAttr(
    IN cloCOMPILER Compiler,
    IN clsATTRIBUTE *Attr,
    IN clsDECL * Decl,
    IN clsLexToken * Identifier
    );

clsNAME *
clParseKernelFuncHeader(
    IN cloCOMPILER Compiler,
    IN clsATTRIBUTE *Attr,
    IN clsDECL * Decl,
    IN clsLexToken * Identifier
    );

clsNAME *
clParseExternKernelFuncHeader(
    IN cloCOMPILER Compiler,
    IN clsATTRIBUTE *Attr,
    IN clsDECL * Decl,
    IN clsLexToken * Identifier
    );

clsDeclOrDeclList *
clParseFuncDecl(
    IN cloCOMPILER Compiler,
    IN clsNAME * FuncName
    );

cloIR_BASE
clParseDeclaration(
    IN cloCOMPILER Compiler,
    IN clsDeclOrDeclList *DeclOrDeclListPtr
    );

slsSLINK_LIST *
clParseAddEnumerator(
IN cloCOMPILER Compiler,
IN clsNAME *Enumerator,
slsSLINK_LIST *EnumList
);

clsNAME *
clParseEnumerator(
IN cloCOMPILER Compiler,
IN clsLexToken *Identiifer,
IN cloIR_EXPR IntExpr
);

clsDATA_TYPE *
clParseTaggedDecl(
IN cloCOMPILER Compiler,
IN clsLexToken * StartToken,
IN clsLexToken * Identifier
);

clsDATA_TYPE *
clParseEnumSpecifier(
IN cloCOMPILER Compiler,
IN clsLexToken * StartToken,
IN clsATTRIBUTE * Attr,
IN clsLexToken * Identifier,
IN gctPOINTER Generic
);

cloIR_BASE
clParseTags(
    IN cloCOMPILER Compiler,
    IN clsDATA_TYPE *Tags
    );


cloIR_BASE
clParseEnumTags(
    IN cloCOMPILER Compiler,
    IN clsDATA_TYPE *Tags,
    IN clsATTRIBUTE *Attr
    );


clsDeclOrDeclList *
clParseTypeDef(
IN cloCOMPILER Compiler,
IN clsDeclOrDeclList *DeclOrDeclListPtr
);


void
clParseCompoundStatementBegin(
    IN cloCOMPILER Compiler
    );

cloIR_SET
clParseCompoundStatementEnd(
    IN cloCOMPILER Compiler,
    IN clsLexToken * StartToken,
    IN cloIR_SET Set,
    IN clsLexToken * EndToken
    );

void
clParseCompoundStatementNoNewScopeBegin(
    IN cloCOMPILER Compiler
    );

cloIR_SET
clParseCompoundStatementNoNewScopeEnd(
    IN cloCOMPILER Compiler,
    IN clsLexToken *StartToken,
    IN cloIR_SET Set,
    IN clsLexToken *EndToken
    );

cloIR_BASE
clParseCaseStatement(
    IN cloCOMPILER Compiler,
    IN clsLexToken *StartToken,
    IN cloIR_EXPR CaseExpr
    );

cloIR_BASE
clParseDefaultStatement(
    IN cloCOMPILER Compiler,
    IN clsLexToken *StartToken
    );

void
clParseSwitchBodyBegin(
    IN cloCOMPILER Compiler
    );

cloIR_BASE
clParseSwitchBodyEnd(
    IN cloCOMPILER Compiler,
    IN clsLexToken * StartToken,
    IN cloIR_SET Set,
    IN clsLexToken * EndToken
    );

cloIR_SET
clParseStatementList(
    IN cloCOMPILER Compiler,
    IN cloIR_BASE Statement
    );

cloIR_SET
clParseStatementList2(
    IN cloCOMPILER Compiler,
    IN cloIR_SET Set,
    IN cloIR_BASE Statement
    );

cloIR_BASE
clParseExprAsStatement(
    IN cloCOMPILER Compiler,
    IN cloIR_EXPR Expr
    );

cloIR_BASE
clParseCompoundStatementAsStatement(
    IN cloCOMPILER Compiler,
    IN cloIR_SET CompoundStatement
    );

cloIR_BASE
clParseCompoundStatementNoNewScopeAsStatementNoNewScope(
    IN cloCOMPILER Compiler,
    IN cloIR_SET CompoundStatementNoNewScope
    );

clsIfStatementPair
clParseIfSubStatements(
    IN cloCOMPILER Compiler,
    IN cloIR_BASE TrueStatement,
    IN cloIR_BASE FalseStatement
    );

cloIR_BASE
clParseIfStatement(
    IN cloCOMPILER Compiler,
    IN clsLexToken * StartToken,
    IN cloIR_EXPR CondExpr,
    IN clsIfStatementPair IfStatementPair
    );

cloIR_BASE
clParseSwitchStatement(
    IN cloCOMPILER Compiler,
    IN clsLexToken * StartToken,
    IN cloIR_EXPR ControlExpr,
    IN cloIR_BASE SwitchBody
    );

cloIR_BASE
clParseStatementLabel(
    IN cloCOMPILER Compiler,
    IN clsLexToken *Label
    );

void
clParseWhileStatementBegin(
    IN cloCOMPILER Compiler
    );

cloIR_BASE
clParseWhileStatementEnd(
    IN cloCOMPILER Compiler,
    IN clsLexToken * StartToken,
    IN cloIR_EXPR CondExpr,
    IN cloIR_BASE LoopBody
    );

cloIR_BASE
clParseDoWhileStatement(
    IN cloCOMPILER Compiler,
    IN clsLexToken * StartToken,
    IN cloIR_BASE LoopBody,
    IN cloIR_EXPR CondExpr
    );

void
clParseForStatementBegin(
    IN cloCOMPILER Compiler
    );

cloIR_BASE
clParseForStatementEnd(
    IN cloCOMPILER Compiler,
    IN clsLexToken * StartToken,
    IN cloIR_BASE ForInitStatement,
    IN clsForExprPair ForExprPair,
    IN cloIR_BASE LoopBody
    );

cloIR_EXPR
clParseCondition(
    IN cloCOMPILER Compiler,
    IN clsDATA_TYPE * DataType,
    IN clsLexToken * Identifier,
    IN cloIR_EXPR Initializer
    );

clsForExprPair
clParseForControl(
    IN cloCOMPILER Compiler,
    IN cloIR_EXPR CondExpr,
    IN cloIR_EXPR RestExpr
    );

cloIR_BASE
clParseJumpStatement(
    IN cloCOMPILER Compiler,
    IN cleJUMP_TYPE Type,
    IN clsLexToken * StartToken,
    IN cloIR_EXPR ReturnExpr
    );

cloIR_BASE
clParseGotoStatement(
    IN cloCOMPILER Compiler,
    IN clsLexToken *StartToken,
    IN clsLexToken *Label
    );

void
clParseExternalDecl(
    IN cloCOMPILER Compiler,
    IN cloIR_BASE Decl
    );

void
clParseFuncDef(
    IN cloCOMPILER Compiler,
    IN clsNAME * FuncName,
    IN cloIR_SET Statements
    );

clsNAME    *
clParseParameterList(
    IN cloCOMPILER Compiler,
    IN clsNAME * FuncName,
    IN clsNAME * ParamName
    );

clsDeclOrDeclList *
clParseTypeDecl(
    IN cloCOMPILER Compiler,
    IN clsDATA_TYPE * DataType
    );

clsNAME    *
clParseParameterDecl(
    IN cloCOMPILER Compiler,
    IN clsDECL * Decl,
    IN clsLexToken * Identifier
    );

clsNAME    *
clParseArrayParameterDecl(
    IN cloCOMPILER Compiler,
    IN clsDECL * Decl,
    IN clsLexToken * Identifier,
    IN cloIR_EXPR ArrayLengthExpr
    );

clsNAME    *
clParseQualifiedParameterDecl(
    IN cloCOMPILER Compiler,
    IN slsSLINK_LIST * TypeQualifierList,
    IN clsLexToken * ParameterQualifier,
    IN clsNAME * ParameterDecl
    );

clsDECL
clParseQualifiedType(
    IN cloCOMPILER Compiler,
    IN slsSLINK_LIST * TypeQualifierList,
    IN gctBOOL ForParamDecl,
    IN clsDECL * Decl
    );

clsDECL
clParseNonStructType(
    IN cloCOMPILER Compiler,
    IN clsLexToken * Token
    );

clsDECL
clParseMatrixType(
    IN cloCOMPILER Compiler,
    IN clsLexToken * Token
    );

clsDATA_TYPE *
clParseStructType(
    IN cloCOMPILER Compiler,
    IN clsDATA_TYPE * StructType
    );

clsDECL
clParseNamedType(
    IN cloCOMPILER Compiler,
    IN clsLexToken *TypeName
    );

void
clParseStructDeclBegin(
    IN cloCOMPILER Compiler,
    IN clsLexToken * Identifier
    );

clsDATA_TYPE *
clParseStructDeclEnd(
    IN cloCOMPILER Compiler,
    IN clsLexToken * StartToken,
    IN clsLexToken * Identifier,
    IN clsATTRIBUTE *Attr,
    IN gceSTATUS ParsingStatus
    );

gceSTATUS
clParseTypeSpecifiedFieldDeclList(
    IN cloCOMPILER Compiler,
    IN clsDECL * Decl,
    IN slsDLINK_LIST * FieldDeclList
    );

slsDLINK_LIST *
clParseFieldDeclList(
    IN cloCOMPILER Compiler,
    IN clsFieldDecl * FieldDecl
    );

slsDLINK_LIST *
clParseFieldDeclList2(
    IN cloCOMPILER Compiler,
    IN slsDLINK_LIST * FieldDeclList,
    IN clsFieldDecl * FieldDecl
    );

clsFieldDecl *
clParseFieldDecl(
    IN cloCOMPILER Compiler,
    IN clsLexToken * Identifier,
    IN cloIR_EXPR ArrayLengthExpr,
    IN clsATTRIBUTE * Attr
    );

clsNAME *
clParseMakeFakeMain(
IN cloCOMPILER Compiler,
IN clsNAME *TopKernelFunc
);

gceSTATUS
clParseMakeArrayPointerExpr(
IN cloCOMPILER Compiler,
IN cloIR_EXPR ArrayOperand,
IN OUT cloIR_EXPR *ArrayPointerExpr
);
#endif /* __gc_cl_parser_h_ */
