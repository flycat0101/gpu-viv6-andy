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


#ifndef __gc_glsl_parser_h_
#define __gc_glsl_parser_h_

#include "gc_glsl_scanner.h"
#include "gc_glsl_built_ins.h"

sloIR_EXPR
slParseVariableIdentifier(
    IN sloCOMPILER Compiler,
    IN slsLexToken * Identifier
    );

sloIR_EXPR
slParseIntConstant(
    IN sloCOMPILER Compiler,
    IN slsLexToken * IntConstant
    );

sloIR_EXPR
slParseUintConstant(
    IN sloCOMPILER Compiler,
    IN slsLexToken * UintConstant
    );

sloIR_EXPR
slParseFloatConstant(
    IN sloCOMPILER Compiler,
    IN slsLexToken * FloatConstant
    );

sloIR_EXPR
slParseBoolConstant(
    IN sloCOMPILER Compiler,
    IN slsLexToken * BoolConstant
    );

sloIR_EXPR
slParseSubscriptExpr(
    IN sloCOMPILER Compiler,
    IN sloIR_EXPR LeftOperand,
    IN sloIR_EXPR RightOperand
    );

sloIR_EXPR
slParseFuncCallExprAsExpr(
    IN sloCOMPILER Compiler,
    IN sloIR_POLYNARY_EXPR FuncCall
    );

sloIR_POLYNARY_EXPR
slParseFuncCallHeaderExpr(
    IN sloCOMPILER Compiler,
    IN slsLexToken * FuncIdentifier
    );

sloIR_POLYNARY_EXPR
slParseFuncCallArgument(
    IN sloCOMPILER Compiler,
    IN sloIR_POLYNARY_EXPR FuncCall,
    IN sloIR_EXPR Argument
    );


sloIR_VIV_ASM
slParseAsmOpcode(
    IN sloCOMPILER         Compiler,
    IN slsASM_OPCODE       *AsmOpcode
    );

sloIR_VIV_ASM
slParseAsmOperand(
    IN sloCOMPILER         Compiler,
    IN sloIR_VIV_ASM       VivAsm,
    IN sloIR_EXPR          Operand
    );

sloIR_EXPR
slParseAsmAppendOperandModifiers(
    IN sloCOMPILER Compiler,
    IN sloIR_EXPR  Expr,
    IN slsASM_MODIFIERS *Modifiers
    );

slsASM_MODIFIERS
slParseAsmAppendModifier(
    IN sloCOMPILER Compiler,
    IN slsASM_MODIFIERS *Modifiers,
    IN slsASM_MODIFIER  *Modifier
    );

slsASM_MODIFIER
slParseAsmModifier(
    IN sloCOMPILER Compiler,
    IN slsLexToken * Type,
    IN slsLexToken * Value
    );

slsASM_OPCODE
slParseAsmCreateOpcode(
    IN sloCOMPILER Compiler,
    IN slsLexToken * Opcode,
    IN slsASM_MODIFIERS * Modifiers
    );

sloIR_EXPR
slParseFieldSelectionExpr(
    IN sloCOMPILER Compiler,
    IN sloIR_EXPR Operand,
    IN slsLexToken * FieldSelection
    );

sloIR_EXPR
slParseIncOrDecExpr(
    IN sloCOMPILER Compiler,
    IN slsLexToken * StartToken,
    IN sleUNARY_EXPR_TYPE ExprType,
    IN sloIR_EXPR Operand
    );

sloIR_EXPR
slParseNormalUnaryExpr(
    IN sloCOMPILER Compiler,
    IN slsLexToken * Operator,
    IN sloIR_EXPR Operand
    );

sloIR_EXPR
slParseNormalBinaryExpr(
    IN sloCOMPILER Compiler,
    IN sloIR_EXPR LeftOperand,
    IN slsLexToken * Operator,
    IN sloIR_EXPR RightOperand
    );

sloIR_EXPR
slParseSelectionExpr(
    IN sloCOMPILER Compiler,
    IN sloIR_EXPR CondExpr,
    IN sloIR_EXPR TrueOperand,
    IN sloIR_EXPR FalseOperand
    );

sloIR_EXPR
slParseAssignmentExpr(
    IN sloCOMPILER Compiler,
    IN sloIR_EXPR LeftOperand,
    IN slsLexToken * Operator,
    IN sloIR_EXPR RightOperand
    );

slsDeclOrDeclList
slParseNonArrayVariableDecl(
    IN sloCOMPILER Compiler,
    IN slsDATA_TYPE * DataType,
    IN slsLexToken * Identifier
    );

slsDeclOrDeclList
slParseArrayVariableDecl(
    IN sloCOMPILER Compiler,
    IN slsDATA_TYPE * DataType,
    IN slsLexToken * Identifier,
    IN sloIR_EXPR ArrayLengthExpr
    );

slsDeclOrDeclList
slParseNonArrayVariableDecl2(
    IN sloCOMPILER Compiler,
    IN slsDeclOrDeclList DeclOrDeclList,
    IN slsLexToken * Identifier
    );

slsDeclOrDeclList
slParseArrayVariableDecl2(
    IN sloCOMPILER Compiler,
    IN slsDeclOrDeclList DeclOrDeclList,
    IN slsLexToken * Identifier,
    IN sloIR_EXPR ArrayLengthExpr
    );

slsDeclOrDeclList
slParseVariableDeclWithInitializer(
    IN sloCOMPILER Compiler,
    IN slsDATA_TYPE * DataType,
    IN slsLexToken * Identifier,
    IN sloIR_EXPR Initializer
    );

slsDeclOrDeclList
slParseVariableDeclWithInitializer2(
    IN sloCOMPILER Compiler,
    IN slsDeclOrDeclList DeclOrDeclList,
    IN slsLexToken * Identifier,
    IN sloIR_EXPR Initializer
    );

slsDeclOrDeclList
slParseArrayVariableDeclWithInitializer(
    IN sloCOMPILER Compiler,
    IN slsDATA_TYPE * DataType,
    IN slsLexToken * Identifier,
    IN sloIR_EXPR ArrayLengthExpr,
    IN sloIR_EXPR Initializer
    );

slsDeclOrDeclList
slParseArrayVariableDeclWithInitializer2(
    IN sloCOMPILER Compiler,
    IN slsDeclOrDeclList DeclOrDeclList,
    IN slsLexToken * Identifier,
    IN sloIR_EXPR ArrayLengthExpr,
    IN sloIR_EXPR Initializer
    );

slsNAME *
slParseFuncHeader(
    IN sloCOMPILER Compiler,
    IN slsDATA_TYPE * DataType,
    IN slsLexToken * Identifier
    );

sloIR_BASE
slParseFuncDecl(
    IN sloCOMPILER Compiler,
    IN slsNAME * FuncName
    );

sloIR_BASE
slParseDeclaration(
    IN sloCOMPILER Compiler,
    IN slsDeclOrDeclList DeclOrDeclList
    );

sloIR_BASE
slParseDefaultPrecisionQualifier(
    IN sloCOMPILER Compiler,
    IN slsLexToken * StartToken,
    IN slsLexToken * PrecisionQualifier,
    IN slsDATA_TYPE * DataType
    );

void
slParseCompoundStatementBegin(
    IN sloCOMPILER Compiler
    );

sloIR_SET
slParseCompoundStatementEnd(
    IN sloCOMPILER Compiler,
    IN slsLexToken * StartToken,
    IN sloIR_SET Set
    );

void
slParseCompoundStatementNoNewScopeBegin(
    IN sloCOMPILER Compiler
    );

sloIR_SET
slParseCompoundStatementNoNewScopeEnd(
    IN sloCOMPILER Compiler,
    IN slsLexToken * StartToken,
    IN sloIR_SET Set
    );

sloIR_SET
slParseStatementList(
    IN sloCOMPILER Compiler,
    IN sloIR_BASE Statement
    );

sloIR_SET
slParseStatementList2(
    IN sloCOMPILER Compiler,
    IN sloIR_SET Set,
    IN sloIR_BASE Statement
    );

sloIR_BASE
slParseExprAsStatement(
    IN sloCOMPILER Compiler,
    IN sloIR_EXPR Expr
    );

sloIR_BASE
slParseAsmAsStatement(
    IN sloCOMPILER Compiler,
    IN sloIR_VIV_ASM VivAsm
    );

sloIR_BASE
slParseQualifierAsStatement(
    IN sloCOMPILER Compiler,
    IN slsLexToken *Qualifier
    );

sloIR_BASE
slParseDataTypeAsStatement(
    IN sloCOMPILER Compiler,
    IN slsDATA_TYPE *DataType
    );

sloIR_BASE
slParseCompoundStatementAsStatement(
    IN sloCOMPILER Compiler,
    IN sloIR_SET CompoundStatement
    );

sloIR_BASE
slParseCompoundStatementNoNewScopeAsStatementNoNewScope(
    IN sloCOMPILER Compiler,
    IN sloIR_SET CompoundStatementNoNewScope
    );

slsSelectionStatementPair
slParseSelectionRestStatement(
    IN sloCOMPILER Compiler,
    IN sloIR_BASE TrueStatement,
    IN sloIR_BASE FalseStatement
    );

sloIR_BASE
slParseSelectionStatement(
    IN sloCOMPILER Compiler,
    IN slsLexToken * StartToken,
    IN sloIR_EXPR CondExpr,
    IN slsSelectionStatementPair SelectionStatementPair
    );

sloIR_BASE
slParseSwitchStatement(
IN sloCOMPILER Compiler,
IN slsLexToken * StartToken,
IN sloIR_EXPR ControlExpr,
IN sloIR_BASE SwitchBody
);

sloIR_BASE
slParseCaseStatement(
IN sloCOMPILER Compiler,
IN slsLexToken * StartToken,
IN sloIR_EXPR CaseExpr
);

sloIR_BASE
slParseDefaultStatement(
IN sloCOMPILER Compiler,
IN slsLexToken * DefaultToken
);

void
slParseSwitchBodyBegin(
IN sloCOMPILER Compiler
);

sloIR_BASE
slParseSwitchBodyEnd(
IN sloCOMPILER Compiler,
IN slsLexToken * StartToken,
IN sloIR_SET Set
);

void
slParseWhileStatementBegin(
    IN sloCOMPILER Compiler
    );

sloIR_BASE
slParseWhileStatementEnd(
    IN sloCOMPILER Compiler,
    IN slsLexToken * StartToken,
    IN sloIR_EXPR CondExpr,
    IN sloIR_BASE LoopBody
    );

sloIR_BASE
slParseDoWhileStatement(
    IN sloCOMPILER Compiler,
    IN slsLexToken * StartToken,
    IN sloIR_BASE LoopBody,
    IN sloIR_EXPR CondExpr
    );

void
slParseForStatementBegin(
    IN sloCOMPILER Compiler
    );

sloIR_BASE
slParseForStatementEnd(
    IN sloCOMPILER Compiler,
    IN slsLexToken * StartToken,
    IN sloIR_BASE ForInitStatement,
    IN slsForExprPair ForExprPair,
    IN sloIR_BASE LoopBody
    );

sloIR_EXPR
slParseCondition(
    IN sloCOMPILER Compiler,
    IN slsDATA_TYPE * DataType,
    IN slsLexToken * Identifier,
    IN sloIR_EXPR Initializer
    );

slsForExprPair
slParseForRestStatement(
    IN sloCOMPILER Compiler,
    IN sloIR_EXPR CondExpr,
    IN sloIR_EXPR RestExpr
    );

sloIR_BASE
slParseJumpStatement(
    IN sloCOMPILER Compiler,
    IN sleJUMP_TYPE Type,
    IN slsLexToken * StartToken,
    IN sloIR_EXPR ReturnExpr
    );

void
slParseExternalDecl(
    IN sloCOMPILER Compiler,
    IN sloIR_BASE Decl
    );

void
slParseFuncDef(
    IN sloCOMPILER Compiler,
    IN slsNAME * FuncName,
    IN sloIR_SET Statements
    );

slsNAME    *
slParseParameterList(
    IN sloCOMPILER Compiler,
    IN slsNAME * FuncName,
    IN slsNAME * ParamName
    );

slsDeclOrDeclList
slParseTypeDecl(
    IN sloCOMPILER Compiler,
    IN slsDATA_TYPE * DataType
    );

slsDeclOrDeclList
slParseInvariantOrPreciseDecl(
    IN sloCOMPILER Compiler,
    IN slsLexToken * StartToken,
    IN slsLexToken * Identifier
    );

slsNAME    *
slParseNonArrayParameterDecl(
    IN sloCOMPILER Compiler,
    IN slsDATA_TYPE * DataType,
    IN slsLexToken * Identifier
    );

slsNAME    *
slParseArrayParameterDecl(
    IN sloCOMPILER Compiler,
    IN slsDATA_TYPE * DataType,
    IN slsLexToken * Identifier,
    IN sloIR_EXPR ArrayLengthExpr
    );

slsLexToken
slParseBasicType(
    IN sloCOMPILER Compiler,
    IN slsDATA_TYPE *BasicType
    );

slsDATA_TYPE *
slParseArrayDataType(
    IN sloCOMPILER Compiler,
    IN slsDATA_TYPE * DataType,
    IN sloIR_EXPR ArrayLengthExpr
    );

slsDATA_TYPE *
slParseArrayListDataType(
    IN sloCOMPILER Compiler,
    IN slsDATA_TYPE * DataType,
    IN slsDLINK_LIST * LengthList
    );

slsNAME    *
slParseQualifiedParameterDecl(
    IN sloCOMPILER Compiler,
    IN slsLexToken * ParameterQualifiers,
    IN slsNAME * ParameterDecl
    );

slsDATA_TYPE *
slParseFullySpecifiedType(
    IN sloCOMPILER Compiler,
    IN slsLexToken * TypeQualifier,
    IN slsDATA_TYPE * DataType
    );

slsLexToken
slMergeTypeQualifiers(
    IN sloCOMPILER Compiler,
    IN slsLexToken *Qualifiers,
    IN slsLexToken *ComingQualifier
    );

slsLexToken
slMergeParameterQualifiers(
    IN sloCOMPILER Compiler,
    IN slsLexToken *CurrentQualifiers,
    IN slsLexToken *IncomingQualifiers
    );

slsDATA_TYPE *
slParseNonStructType(
    IN sloCOMPILER Compiler,
    IN slsLexToken * Token,
    IN gctINT TokenType
    );

slsDATA_TYPE *
slParseStructType(
    IN sloCOMPILER Compiler,
    IN slsDATA_TYPE * StructType
    );

slsDATA_TYPE *
slParseNamedType(
    IN sloCOMPILER Compiler,
    IN slsLexToken * TypeName
    );

void
slParseStructDeclBegin(
    IN sloCOMPILER Compiler,
    IN slsLexToken * Identifier
    );

slsDATA_TYPE *
slParseStructDeclEnd(
    IN sloCOMPILER Compiler,
    IN slsLexToken * Identifier
    );

void
slParseStructReDeclBegin(
    IN sloCOMPILER Compiler,
    IN slsLexToken * TypeName
    );

slsDATA_TYPE *
slParseStructReDeclEnd(
    IN sloCOMPILER Compiler,
    IN slsLexToken * TypeName
    );

void
slParseTypeSpecifiedFieldDeclList(
    IN sloCOMPILER Compiler,
    IN slsDATA_TYPE * DataType,
    IN slsDLINK_LIST * FieldDeclList
    );

slsDLINK_LIST *
slParseArrayLengthList(
    IN sloCOMPILER Compiler,
    IN slsDLINK_LIST * LengthList,
    IN sloIR_EXPR ArrayLengthExpr1,
    IN sloIR_EXPR ArrayLengthExpr2
    );

slsDLINK_LIST *
slParseArrayLengthList2(
    IN sloCOMPILER Compiler,
    IN slsDLINK_LIST * LengthList,
    IN sloIR_EXPR ArrayLengthExpr1,
    IN sloIR_EXPR ArrayLengthExpr2
    );

slsNAME    *
slParseArrayListParameterDecl(
    IN sloCOMPILER Compiler,
    IN slsDATA_TYPE * DataType,
    IN slsLexToken * Identifier,
    IN slsDLINK_LIST * LengthList
    );


slsDeclOrDeclList
slParseArrayListVariableDecl2(
    IN sloCOMPILER Compiler,
    IN slsDeclOrDeclList DeclOrDeclList,
    IN slsLexToken * Identifier,
    IN slsDLINK_LIST * LengthList
    );

slsDeclOrDeclList
slParseArrayListVariableDecl(
    IN sloCOMPILER Compiler,
    IN slsDATA_TYPE * DataType,
    IN slsLexToken * Identifier,
    IN slsDLINK_LIST * LengthList
    );

slsFieldDecl *
slParseFieldListDecl(
    IN sloCOMPILER Compiler,
    IN slsLexToken * Identifier,
    IN slsDLINK_LIST * LengthList,
    IN gctBOOL IsBlockMember
    );

slsDeclOrDeclList
slParseArrayListVariableDeclWithInitializer(
    IN sloCOMPILER Compiler,
    IN slsDATA_TYPE * DataType,
    IN slsLexToken * Identifier,
    IN slsDLINK_LIST * LengthList,
    IN sloIR_EXPR Initializer
    );

slsDeclOrDeclList
slParseArrayListVariableDeclWithInitializer2(
    IN sloCOMPILER Compiler,
    IN slsDeclOrDeclList DeclOrDeclList,
    IN slsLexToken * Identifier,
    IN slsDLINK_LIST * LengthList,
    IN sloIR_EXPR Initializer
    );

slsDLINK_LIST *
slParseFieldDeclList(
    IN sloCOMPILER Compiler,
    IN slsFieldDecl * FieldDecl
    );

slsDLINK_LIST *
slParseFieldDeclList2(
    IN sloCOMPILER Compiler,
    IN slsDLINK_LIST * FieldDeclList,
    IN slsFieldDecl * FieldDecl
    );

slsFieldDecl *
slParseFieldDecl(
    IN sloCOMPILER Compiler,
    IN slsLexToken * Identifier,
    IN sloIR_EXPR ArrayLengthExpr
    );

slsFieldDecl *
slParseImplicitArraySizeFieldDecl(
    IN sloCOMPILER Compiler,
    IN slsLexToken * Identifier
    );

slsLexToken
slParseLayoutId(
    IN sloCOMPILER Compiler,
    IN slsLexToken * LayoutId,
    IN slsLexToken * Value
    );

slsLexToken
slParseLayoutQualifier(
    IN sloCOMPILER Compiler,
    IN slsLexToken * LayoutIdList
    );

slsLexToken
slParseAddLayoutId(
    IN sloCOMPILER Compiler,
    IN slsLexToken * LayoutIdList,
    IN slsLexToken * LayoutId
    );
void
slParseInterfaceBlockDeclBegin(
    IN sloCOMPILER Compiler,
    IN slsLexToken * BlockType,
    IN slsLexToken * BlockName
    );

slsNAME *
slParseInterfaceBlockDeclEnd(
    IN sloCOMPILER Compiler,
    IN slsLexToken * BlockType,
    IN slsLexToken * BlockName
    );

slsDATA_TYPE *
slParseInterfaceBlockMember(
    IN sloCOMPILER Compiler,
    IN slsDATA_TYPE *DataType,
    IN slsFieldDecl *Member
    );

sloIR_BASE
slParseInterfaceBlock(
    IN sloCOMPILER Compiler,
    IN slsNAME *Block,
    IN slsLexToken *BlockInstance,
    IN sloIR_EXPR ArrayLengthExpr,
    IN gctBOOL CheckArrayLength
    );

sloIR_BASE
slParseInterfaceBlockImplicitArrayLength(
    IN sloCOMPILER Compiler,
    IN slsNAME *Block,
    IN slsLexToken *BlockInstance
    );

sloIR_EXPR
slParseLengthMethodExpr(
    IN sloCOMPILER Compiler,
    IN sloIR_EXPR Operand
    );

slsLexToken
slParseCheckStorage(
    IN sloCOMPILER Compiler,
    IN slsLexToken Storage,
    IN slsLexToken InOrOut
    );

void
slParseFuncDefinitionBegin(
    IN sloCOMPILER Compiler,
    IN slsNAME * FuncName
    );

void
slParseFuncDefinitionEnd(
    IN sloCOMPILER Compiler,
    IN slsNAME * FuncName
    );

#endif /* __gc_glsl_parser_h_ */
