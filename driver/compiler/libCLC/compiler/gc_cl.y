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


%{
#include "gc_cl_parser.h"

#define YY_NO_UNISTD_H

/* define to int to avoid compilation error of
   conversion from 'int' to 'yytype_int16' */
#define YYTYPE_INT16  int

#define YYPARSE_PARAM_DECL cloCOMPILER
#define YYPARSE_PARAM Compiler

#define YYLEX_PARAM  Compiler

#define yylex cloCOMPILER_Lex
int yylex(YYSTYPE * pyylval, cloCOMPILER Compiler);

%}

%pure_parser

%union
{
	clsLexToken		token;
	slsSLINK_LIST *		typeQualifierList;
	clsDeclOrDeclList	*declOrDeclList;
	slsDLINK_LIST *		fieldDeclList;
	clsFieldDecl *		fieldDecl;
	clsDATA_TYPE *		dataType;
	clsDECL			decl;
	cloIR_EXPR		expr;
	clsNAME	*		funcName;
	clsNAME	*		paramName;
	clsATTRIBUTE *		attr;
	slsSLINK_LIST *		enumeratorList;
	clsNAME	*		enumeratorName;
	cloIR_SET		statements;
	cloIR_BASE		statement;
	clsIfStatementPair	ifStatementPair;
	clsForExprPair		forExprPair;
	cloIR_POLYNARY_EXPR	funcCall;
	gceSTATUS		status;
}

/* ATTENTION!!!
   Define terminal tokens ONLY between T_VERY_FIRST_TERMINAL and T_VERY_LAST_TERMINAL
   This is used to calculate the number of terminals
*/
%token<token>           T_VERY_FIRST_TERMINAL

/* BEWARE: builtin data types only for the next group of tokens;
           any new token for builtin data types needs to be defined before any other */
%token<token>		T_VOID
			T_MAT2 T_MAT3 T_MAT4 T_MAT8 T_MAT16
			T_BOOL T_BOOL2 T_BOOL3 T_BOOL4 T_BOOL8 T_BOOL16 T_BOOL32
			T_HALF T_HALF2 T_HALF3 T_HALF4 T_HALF8 T_HALF16 T_HALF32
			T_FLOAT T_FLOAT2 T_FLOAT3 T_FLOAT4 T_FLOAT8 T_FLOAT16
			T_DOUBLE T_DOUBLE2 T_DOUBLE3 T_DOUBLE4 T_DOUBLE8 T_DOUBLE16
			T_QUAD T_QUAD2 T_QUAD3 T_QUAD4 T_QUAD8 T_QUAD16
			T_CHAR T_CHAR2 T_CHAR3 T_CHAR4 T_CHAR8 T_CHAR16 T_CHAR32
			T_UCHAR T_UCHAR2 T_UCHAR3 T_UCHAR4 T_UCHAR8 T_UCHAR16 T_UCHAR32
			T_SHORT T_SHORT2 T_SHORT3 T_SHORT4 T_SHORT8 T_SHORT16 T_SHORT32
			T_USHORT T_USHORT2 T_USHORT3 T_USHORT4 T_USHORT8 T_USHORT16 T_USHORT32
			T_INT T_INT2 T_INT3 T_INT4 T_INT8 T_INT16
			T_UINT T_UINT2 T_UINT3 T_UINT4 T_UINT8 T_UINT16
			T_LONG T_LONG2 T_LONG3 T_LONG4 T_LONG8 T_LONG16
			T_ULONG T_ULONG2 T_ULONG3 T_ULONG4 T_ULONG8 T_ULONG16
                        T_SAMPLER_T
			T_IMAGE1D_T T_IMAGE1D_ARRAY_T T_IMAGE1D_BUFFER_T
			T_IMAGE2D_ARRAY_T
			T_IMAGE2D_T T_IMAGE3D_T
			T_IMAGE2D_PTR_T T_IMAGE2D_DYNAMIC_ARRAY_T
			T_SIZE_T T_EVENT_T
			T_PTRDIFF_T T_INTPTR_T T_UINTPTR_T
			T_GENTYPE T_F_GENTYPE T_IU_GENTYPE T_I_GENTYPE T_U_GENTYPE T_SIU_GENTYPE
			T_BOOL_PACKED T_BOOL2_PACKED T_BOOL3_PACKED T_BOOL4_PACKED T_BOOL8_PACKED T_BOOL16_PACKED T_BOOL32_PACKED
			T_CHAR_PACKED T_CHAR2_PACKED T_CHAR3_PACKED T_CHAR4_PACKED T_CHAR8_PACKED T_CHAR16_PACKED T_CHAR32_PACKED
			T_UCHAR_PACKED T_UCHAR2_PACKED T_UCHAR3_PACKED T_UCHAR4_PACKED T_UCHAR8_PACKED T_UCHAR16_PACKED T_UCHAR32_PACKED
			T_SHORT_PACKED T_SHORT2_PACKED T_SHORT3_PACKED T_SHORT4_PACKED T_SHORT8_PACKED T_SHORT16_PACKED T_SHORT32_PACKED
			T_USHORT_PACKED T_USHORT2_PACKED T_USHORT3_PACKED T_USHORT4_PACKED T_USHORT8_PACKED T_USHORT16_PACKED T_USHORT32_PACKED
			T_HALF_PACKED T_HALF2_PACKED T_HALF3_PACKED T_HALF4_PACKED T_HALF8_PACKED T_HALF16_PACKED T_HALF32_PACKED
			T_GENTYPE_PACKED

%token<token>		T_FLOATNXM T_DOUBLENXM
 			T_BUILTIN_DATA_TYPE T_RESERVED_DATA_TYPE T_VIV_PACKED_DATA_TYPE

%token<token>		T_IDENTIFIER T_TYPE_NAME
			T_FLOATCONSTANT T_UINTCONSTANT T_INTCONSTANT T_BOOLCONSTANT T_CHARCONSTANT T_STRING_LITERAL
			T_FIELD_SELECTION
			T_LSHIFT_OP T_RSHIFT_OP
			T_INC_OP T_DEC_OP T_LE_OP T_GE_OP T_EQ_OP T_NE_OP
			T_AND_OP T_OR_OP T_XOR_OP T_MUL_ASSIGN T_DIV_ASSIGN T_ADD_ASSIGN
			T_MOD_ASSIGN T_LEFT_ASSIGN T_RIGHT_ASSIGN T_AND_ASSIGN T_XOR_ASSIGN T_OR_ASSIGN
			T_SUB_ASSIGN
			T_STRUCT_UNION_PTR
			T_INITIALIZER_END

%token<token>		T_BREAK T_CONTINUE T_RETURN  T_GOTO
			T_WHILE T_FOR T_DO
			T_ELSE T_IF T_SWITCH T_CASE T_DEFAULT

%token<token>	   	T_CONST
			T_RESTRICT T_VOLATILE
                        T_STATIC T_EXTERN
			T_CONSTANT T_GLOBAL T_LOCAL T_PRIVATE T_KERNEL T_UNIFORM
			T_READ_ONLY T_WRITE_ONLY
			T_PACKED T_ALIGNED T_ENDIAN T_VEC_TYPE_HINT
			T_ATTRIBUTE__
			T_REQD_WORK_GROUP_SIZE
			T_WORK_GROUP_SIZE_HINT
			T_KERNEL_SCALE_HINT
			T_ALWAYS_INLINE
			T_UNSIGNED

%token<token>		T_STRUCT T_UNION
			T_TYPEDEF T_ENUM T_INLINE
			T_SIZEOF T_TYPE_CAST
			T_VEC_STEP
			T_TYPEOF

%token<token>           T_VERY_LAST_TERMINAL
/* types */
%type<token>		'(' ')' '[' ']' '{' '}' '.' ',' ':' '=' ';' '!'
					'-' '~' '+' '*' '/' '%' '<' '>' '|' '^' '&' '?'

%type<token>		unary_operator assignment_operator
					type_qualifier parameter_qualifier
					declarator direct_declarator
					type_name
					struct_or_union
					string_literal
					curly_bracket_type_cast

%type<expr>			variable_identifier primary_expression postfix_expression integer_expression
					unary_expression cast_expression
					multiplicative_expression additive_expression shift_expression
					relational_expression equality_expression and_expression
					exclusive_or_expression inclusive_or_expression logical_and_expression
					logical_xor_expression logical_or_expression conditional_expression
					assignment_expression expression constant_expression array_size
					conditionopt array_declarator
					initializer initializer_list initializer_list_start

%type<declOrDeclList>	init_declarator_list single_declaration
			designation designator_list designator

%type<typeQualifierList> type_qualifier_list pointer

%type<dataType>		struct_union_specifier enum_specifier tags

%type<decl>		fully_specified_type type_specifier type_cast typeof_type_specifier

%type<fieldDeclList>	struct_declarator_list

%type<fieldDecl>	struct_declarator
%type<status>	        struct_declaration struct_declaration_list

%type<funcName>		function_prototype function_declarator function_header_with_parameters
					function_header

%type<attr>		attribute attribute_list attribute_specifier attribute_specifier_opt

%type<paramName>	parameter_declaration parameter_declarator parameter_type_specifier

%type<enumeratorList>	enumerator_list

%type<enumeratorName>	enumerator

%type<statements>	compound_statement compound_statement_no_new_scope statement_list
					switch_body_statement_list

%type<statement>	declaration statement statement_no_new_scope simple_statement
					declaration_statement expression_statement selection_statement
					iteration_statement jump_statement for_init_statement
					switch_body switch_body_statement

%type<ifStatementPair>	if_sub_statement

%type<forExprPair>	for_control

%type<funcCall>		function_call function_call_header function_call_with_parameters

/* yacc rules */
%right T_IF T_ELSE

%start translation_unit

%%

variable_identifier :
	T_IDENTIFIER
		{ $$ = clParseVariableIdentifier(Compiler, &$1);
		  if($$ == gcvNULL) {
		     YYERROR;
		  }
		}
	;

string_literal :
	T_STRING_LITERAL
		{ $$ = $1; }
	| string_literal T_STRING_LITERAL
		{ $$ = clParseCatStringLiteral(Compiler, &$1, &$2); }
	;

primary_expression :
	variable_identifier
		{ $$ = $1; }
	| T_UINTCONSTANT
		{ $$ = clParseScalarConstant(Compiler, &$1); }
	| T_INTCONSTANT
		{ $$ = clParseScalarConstant(Compiler, &$1); }
	| T_FLOATCONSTANT
		{ $$ = clParseScalarConstant(Compiler, &$1); }
	| T_BOOLCONSTANT
		{ $$ = clParseScalarConstant(Compiler, &$1); }
	| T_CHARCONSTANT
		{ $$ = clParseScalarConstant(Compiler, &$1); }
	| string_literal
		{ $$ = clParseStringLiteral(Compiler, &$1); }
	| '(' expression ')'
		{ $$ = $2; }
	;

postfix_expression :
	primary_expression
		{ $$ = $1; }
	| postfix_expression '[' integer_expression ']'
		{ $$ = clParseSubscriptExpr(Compiler, $1, $3); }
	| function_call
		{ $$ = clParseFuncCallExprAsExpr(Compiler, $1); }
	| postfix_expression '.' T_FIELD_SELECTION
		{ $$ = clParseFieldSelectionExpr(Compiler, $1, &$3); }
	| postfix_expression T_STRUCT_UNION_PTR T_FIELD_SELECTION
		{ $$ = clParsePtrFieldSelectionExpr(Compiler, $1, &$3); }
	| postfix_expression T_INC_OP
		{ $$ = clParseIncOrDecExpr(Compiler, gcvNULL, clvUNARY_POST_INC, $1); }
	| postfix_expression T_DEC_OP
		{ $$ = clParseIncOrDecExpr(Compiler, gcvNULL, clvUNARY_POST_DEC, $1); }
	;

integer_expression :
	expression
		{ $$ = $1; }
	;

function_call :
	function_call_with_parameters ')'
		{ $$ = $1; }
	| function_call_header T_VOID ')'
		{ $$ = $1; }
	| function_call_header ')'
		{ $$ = $1; }
	;

function_call_with_parameters :
	function_call_header assignment_expression
		{ $$ = clParseFuncCallArgument(Compiler, $1, $2); }
	| function_call_with_parameters ',' assignment_expression
		{ $$ = clParseFuncCallArgument(Compiler, $1, $3); }
	;

type_cast :
	'(' fully_specified_type ')'
		{ $$ = $2; }
	| '(' fully_specified_type pointer ')'
		{ $$ = clParseCreateDecl(Compiler, &$2, $3, gcvNULL); }
	;

function_call_header :
	T_IDENTIFIER '('
		{ $$ = clParseFuncCallHeaderExpr(Compiler, &$1, gcvNULL); }
	;

curly_bracket_type_cast :
	type_cast '{'
	{
	    clParseCastExprBegin(Compiler, &$1);
	    (void) gcoOS_ZeroMemory((gctPOINTER)&$<token>$, sizeof(clsLexToken));
	    $<token>$.type = T_TYPE_CAST;
	}
	;

cast_expression :
	unary_expression
		{ $$ = $1; }
	| type_cast
		{
		   clParseCastExprBegin(Compiler, gcvNULL);
	           (void) gcoOS_ZeroMemory((gctPOINTER)&$<token>$, sizeof(clsLexToken));
		   $<token>$.type = T_TYPE_CAST;
		}
		cast_expression
		{ $$ = clParseCastExprEnd(Compiler, &$1, $3);
                }
	| curly_bracket_type_cast
		expression '}'
		{ $$ = clParseCastExprEnd(Compiler, gcvNULL, $2);
                }
	| curly_bracket_type_cast
		expression T_INITIALIZER_END
		{ $$ = clParseCastExprEnd(Compiler, gcvNULL, $2);
                }
	;

unary_expression :
	postfix_expression
		{ $$ = $1; }
	| T_INC_OP unary_expression
		{ $$ = clParseIncOrDecExpr(Compiler, &$1, clvUNARY_PRE_INC, $2); }
	| T_DEC_OP unary_expression
		{ $$ = clParseIncOrDecExpr(Compiler, &$1, clvUNARY_PRE_DEC, $2); }
	| T_VEC_STEP '(' unary_expression ')'
		{ $$ = clParseVecStep(Compiler, &$1, &$3->decl); }
	| T_VEC_STEP type_cast
		{ $$ = clParseVecStep(Compiler, &$1, &$2); }
	| T_SIZEOF unary_expression
		{ $$ = clParseSizeofExpr(Compiler, &$1, $2); }
	| T_SIZEOF type_cast
		{ $$ = clParseSizeofTypeDecl(Compiler, &$1, &$2); }
	| unary_operator cast_expression
		{ $$ = clParseNormalUnaryExpr(Compiler, &$1, $2); }
	;

unary_operator:
	'+'
		{ $$ = $1; }
	| '-'
		{ $$ = $1; }
	| '!'
		{ $$ = $1; }
	| '~'	/* reserved */
		{ $$ = $1; }
	| '&'
		{ $$ = $1; }
	| '*'
		{ $$ = $1; }
	;

multiplicative_expression :
	cast_expression
		{
		   $$ = $1;
		}
	| multiplicative_expression '*' cast_expression
		{ $$ = clParseNormalBinaryExpr(Compiler, $1, &$2, $3); }
	| multiplicative_expression '/' cast_expression
		{ $$ = clParseNormalBinaryExpr(Compiler, $1, &$2, $3); }
	| multiplicative_expression '%' cast_expression	/* reserved */
		{ $$ = clParseNormalBinaryExpr(Compiler, $1, &$2, $3); }
	;

additive_expression :
	multiplicative_expression
		{ $$ = $1; }
	| additive_expression '+' multiplicative_expression
		{ $$ = clParseNormalBinaryExpr(Compiler, $1, &$2, $3); }
	| additive_expression '-' multiplicative_expression
		{ $$ = clParseNormalBinaryExpr(Compiler, $1, &$2, $3); }
	;

shift_expression :
	additive_expression
		{ $$ = $1; }
	| shift_expression T_LSHIFT_OP additive_expression
		{ $$ = clParseNormalBinaryExpr(Compiler, $1, &$2, $3); }
	| shift_expression T_RSHIFT_OP additive_expression
		{ $$ = clParseNormalBinaryExpr(Compiler, $1, &$2, $3); }
	;

relational_expression :
	shift_expression
		{ $$ = $1; }
	| relational_expression '<' shift_expression
		{ $$ = clParseNormalBinaryExpr(Compiler, $1, &$2, $3); }
	| relational_expression '>' shift_expression
		{ $$ = clParseNormalBinaryExpr(Compiler, $1, &$2, $3); }
	| relational_expression T_LE_OP shift_expression
		{ $$ = clParseNormalBinaryExpr(Compiler, $1, &$2, $3); }
	| relational_expression T_GE_OP shift_expression
		{ $$ = clParseNormalBinaryExpr(Compiler, $1, &$2, $3); }
	;

equality_expression:
	relational_expression
		{ $$ = $1; }
	| equality_expression T_EQ_OP relational_expression
		{ $$ = clParseNormalBinaryExpr(Compiler, $1, &$2, $3); }
	| equality_expression T_NE_OP relational_expression
		{ $$ = clParseNormalBinaryExpr(Compiler, $1, &$2, $3); }
	;

and_expression :
	equality_expression
		{ $$ = $1; }
	| and_expression '&' equality_expression	/* reserved */
		{ $$ = clParseNormalBinaryExpr(Compiler, $1, &$2, $3); }
	;

exclusive_or_expression :
	and_expression
		{ $$ = $1; }
	| exclusive_or_expression '^' and_expression	/* reserved */
		{ $$ = clParseNormalBinaryExpr(Compiler, $1, &$2, $3); }
	;

inclusive_or_expression :
	exclusive_or_expression
		{ $$ = $1; }
	| inclusive_or_expression '|' exclusive_or_expression	/* reserved */
		{ $$ = clParseNormalBinaryExpr(Compiler, $1, &$2, $3); }
	;

logical_and_expression :
	inclusive_or_expression
		{ $$ = $1; }
	| logical_and_expression T_AND_OP inclusive_or_expression
		{ $$ = clParseNormalBinaryExpr(Compiler, $1, &$2, $3); }
	;

logical_xor_expression :
	logical_and_expression
		{ $$ = $1; }
	| logical_xor_expression T_XOR_OP logical_and_expression
		{ $$ = clParseNormalBinaryExpr(Compiler, $1, &$2, $3); }
	;

logical_or_expression :
	logical_xor_expression
		{ $$ = $1; }
	| logical_or_expression T_OR_OP logical_xor_expression
		{ $$ = clParseNormalBinaryExpr(Compiler, $1, &$2, $3); }
	;

conditional_expression :
	logical_or_expression
		{ $$ = $1; }
	| logical_or_expression '?' expression ':' assignment_expression
		{ $$ = clParseSelectionExpr(Compiler, $1, $3, $5); }
	;

assignment_expression :
	conditional_expression
		{ $$ = $1;}
	| unary_expression assignment_operator assignment_expression
		{ $$ = clParseAssignmentExpr(Compiler, $1, &$2, $3); }
	;

assignment_operator:
	'='
		{ $$ = $1; }
	| T_MUL_ASSIGN
		{ $$ = $1; }
	| T_DIV_ASSIGN
		{ $$ = $1; }
	| T_MOD_ASSIGN		/* reserved */
		{ $$ = $1; }
	| T_ADD_ASSIGN
		{ $$ = $1; }
	| T_SUB_ASSIGN
		{ $$ = $1; }
	| T_LEFT_ASSIGN		/* reserved */
		{ $$ = $1; }
	| T_RIGHT_ASSIGN	/* reserved */
		{ $$ = $1; }
	| T_AND_ASSIGN		/* reserved */
		{ $$ = $1; }
	| T_XOR_ASSIGN		/* reserved */
		{ $$ = $1; }
	| T_OR_ASSIGN		/* reserved */
		{ $$ = $1; }
	;

expression :
	assignment_expression
	  { $$ = $1; }
	| expression ',' assignment_expression
	  {
	    if(cloCOMPILER_GetParserState(Compiler) == clvPARSER_IN_TYPE_CAST) {
               $$ = clParseBinarySequenceExpr(Compiler,
					      (YYSTYPE *)&$<token>0, $1, &$2, $3);
	    }
	    else {
               $$ = clParseNormalBinaryExpr(Compiler, $1, &$2, $3);
	    }
	  }
	;

constant_expression :
	conditional_expression
		{ $$ = $1; }
	;

tags :
	struct_or_union T_IDENTIFIER
		{ $$ = clParseTaggedDecl(Compiler, &$1, &$2); }
	| T_ENUM T_IDENTIFIER
		{ $$ = clParseTaggedDecl(Compiler, &$1, &$2); }
	;

enum_specifier :
	T_ENUM attribute_specifier T_IDENTIFIER '{'
		enumerator_list
		'}'
		{ $$ = clParseEnumSpecifier(Compiler, &$1, $2, &$3, (gctPOINTER)$5); }
	| T_ENUM attribute_specifier T_IDENTIFIER '{'
		enumerator_list T_INITIALIZER_END
		{ $$ = clParseEnumSpecifier(Compiler, &$1, $2, &$3, (gctPOINTER)$5); }
	| T_ENUM T_IDENTIFIER '{'
		enumerator_list
		'}'
		{ $$ = clParseEnumSpecifier(Compiler, &$1, gcvNULL, &$2, (gctPOINTER)$4); }
	| T_ENUM T_IDENTIFIER '{'
		enumerator_list  T_INITIALIZER_END
		{ $$ = clParseEnumSpecifier(Compiler, &$1, gcvNULL, &$2, (gctPOINTER)$4); }
	| T_ENUM attribute_specifier '{'
		enumerator_list
		'}'
		{ $$ = clParseEnumSpecifier(Compiler, &$1, $2, gcvNULL, (gctPOINTER)$4); }
	| T_ENUM attribute_specifier '{'
		enumerator_list T_INITIALIZER_END
		{ $$ = clParseEnumSpecifier(Compiler, &$1, $2, gcvNULL, (gctPOINTER)$4); }
	| T_ENUM '{'
		enumerator_list
		'}'
		{ $$ = clParseEnumSpecifier(Compiler, &$1, gcvNULL, gcvNULL, (gctPOINTER)$3); }
	| T_ENUM '{'
		enumerator_list T_INITIALIZER_END
		{ $$ = clParseEnumSpecifier(Compiler, &$1, gcvNULL, gcvNULL, (gctPOINTER)$3); }
	;

enumerator_list :
	enumerator
		{
		   slsSLINK_LIST *enumList;

		   slmSLINK_LIST_Initialize(enumList);
                   $$ = clParseAddEnumerator(Compiler, $1, enumList);
		}
	| enumerator_list ',' enumerator
		{ $$ = clParseAddEnumerator(Compiler, $3, $1); }
	;

enumerator :
	T_IDENTIFIER
		{ $$ = clParseEnumerator(Compiler, &$1, gcvNULL); }
	| T_IDENTIFIER '=' constant_expression
		{ $$ = clParseEnumerator(Compiler, &$1, $3); }
	;

declaration :
	init_declarator_list ';'
		{ $$ = clParseDeclaration(Compiler, $1); }
	| enum_specifier attribute_specifier_opt ';'
		{ $$ = clParseEnumTags(Compiler, $1, $2); }
	| tags ';'
		{ $$ = clParseTags(Compiler, $1); }
	| struct_union_specifier ';'
		{ $$ = clParseTags(Compiler, $1); }
	;

function_prototype :
	function_declarator ')'
		{ $$ = $1; }
	;

function_declarator :
	function_header
		{ $$ = $1; }
	| function_header_with_parameters
		{ $$ = $1; }
	;

function_header_with_parameters :
	function_header parameter_declaration
		{ $$ = clParseParameterList(Compiler, $1, $2); }
	| function_header_with_parameters ',' parameter_declaration
		{ $$ = clParseParameterList(Compiler, $1, $3); }
	;

function_header :
	attribute_specifier T_KERNEL fully_specified_type direct_declarator '('
		{ $$ = clParseKernelFuncHeader(Compiler, $1, &$3, &$4); }
	| T_KERNEL attribute_specifier_opt fully_specified_type direct_declarator '('
		{ $$ = clParseKernelFuncHeader(Compiler, $2, &$3, &$4); }
	| T_EXTERN T_KERNEL attribute_specifier_opt fully_specified_type direct_declarator '('
		{ $$ = clParseExternKernelFuncHeader(Compiler, $3, &$4, &$5); }
	| fully_specified_type direct_declarator '('
		{ $$ = clParseFuncHeader(Compiler, &$1, &$2); }
	| attribute_specifier fully_specified_type direct_declarator '('
		{ $$ = clParseFuncHeaderWithAttr(Compiler, $1, &$2, &$3); }
	| T_INLINE fully_specified_type direct_declarator '('
		{ $$ = clParseFuncHeader(Compiler, &$2, &$3);
		  if($$) $$->u.funcInfo.isInline = gcvTRUE;
		}
	| T_STATIC T_INLINE fully_specified_type direct_declarator '('
		{ $$ = clParseFuncHeader(Compiler, &$3, &$4);
		  if($$) $$->u.funcInfo.isInline = gcvTRUE;
		}
	;

parameter_declarator :
	type_specifier direct_declarator
		{ $$ = clParseParameterDecl(Compiler, &$1, &$2); }
	| type_specifier direct_declarator array_declarator
		{ $$ = clParseArrayParameterDecl(Compiler, &$1, &$2, $3); }
        | type_specifier type_qualifier_list direct_declarator
		{ 
                    clsDECL decl;
                    decl = clParseQualifiedType(Compiler, $2, gcvTRUE, &$1);
                    $$ = clParseParameterDecl(Compiler, &decl, &$3);
                }
	| type_specifier type_qualifier_list direct_declarator array_declarator
                {
                    clsDECL decl;
                    decl = clParseQualifiedType(Compiler, $2, gcvTRUE, &$1);
		    $$ = clParseArrayParameterDecl(Compiler, &decl, &$3, $4);
                }
	;

parameter_declaration :
	parameter_declarator
		{ $$ = clParseQualifiedParameterDecl(Compiler, gcvNULL, gcvNULL, $1); }
	| parameter_qualifier parameter_declarator
		{ $$ = clParseQualifiedParameterDecl(Compiler, gcvNULL, &$1, $2); }
	| parameter_type_specifier
		{ $$ = clParseQualifiedParameterDecl(Compiler, gcvNULL, gcvNULL, $1); }
	| parameter_qualifier parameter_type_specifier
		{ $$ = clParseQualifiedParameterDecl(Compiler, gcvNULL, &$1, $2); }
	| type_qualifier_list parameter_declarator
		{ $$ = clParseQualifiedParameterDecl(Compiler, $1, gcvNULL, $2); }
	| type_qualifier_list parameter_qualifier parameter_declarator
		{ $$ = clParseQualifiedParameterDecl(Compiler, $1, &$2, $3); }
	| type_qualifier_list parameter_type_specifier
		{ $$ = clParseQualifiedParameterDecl(Compiler, $1, gcvNULL, $2); }
	| type_qualifier_list parameter_qualifier parameter_type_specifier
		{ $$ = clParseQualifiedParameterDecl(Compiler, $1, &$2, $3); }
	;

parameter_qualifier :
	T_READ_ONLY
		{ $$ = $1; }
	| T_WRITE_ONLY
		{ $$ = $1; }
	;

parameter_type_specifier :
	type_specifier
		{ $$ = clParseParameterDecl(Compiler, &$1, gcvNULL); }
	| type_specifier array_declarator
		{ $$ = clParseArrayParameterDecl(Compiler, &$1, gcvNULL, $2); }

	| type_specifier pointer 
		{ 
                    clsDECL decl;
                    decl = clParseQualifiedType(Compiler, $2, gcvTRUE, &$1);
		    $$ = clParseParameterDecl(Compiler, &decl, gcvNULL);
                }
	| type_specifier pointer array_declarator
		{ 
                    clsDECL decl;
                    decl = clParseQualifiedType(Compiler, $2, gcvTRUE, &$1);
		    $$ = clParseArrayParameterDecl(Compiler, &decl, gcvNULL, $3);
                }
	;

type_qualifier_list :
	type_qualifier
		{
		    $$ = clParseTypeQualifierList(Compiler, &$1,
		                                  clParseEmptyTypeQualifierList(Compiler));
		}
	| type_qualifier type_qualifier_list
		{$$ = clParseTypeQualifierList(Compiler, &$1, $2); }
	;

pointer :
	'*'
		{ $$ = clParseEmptyTypeQualifierList(Compiler); }
	| '*' pointer
		{ $$ = clParsePointerTypeQualifier(Compiler, gcvNULL, $2); }
	| '*' type_qualifier_list
		{$$ = $2;}
	| '*' type_qualifier_list pointer
		{$$ = clParsePointerTypeQualifier(Compiler, $2, $3); }
	;

declarator : T_IDENTIFIER
	   	{$$ = $1;}
	   | pointer T_IDENTIFIER
		{
		  $$ = $2;
		  $$.u.identifier.ptrDscr = $1;
		}

direct_declarator :
	declarator
	  {  $$ = $1; }
	| '(' declarator ')'
	  { $$ = $2; }
	;

array_size :
		{ $$ = clParseNullExpr(Compiler, &$<token>0); }
	| constant_expression
		{ $$ = $1; }
	;

array_declarator :
          '[' array_size ']'
            { $$ = $2; }
	| array_declarator '[' array_size ']'
            { $$ = clParseArrayDeclarator(Compiler, &$2, $1, $3); }
	;

init_declarator_list :
	single_declaration
		{ $$ = $1; }
	| T_TYPEDEF
		{
    		   cloCOMPILER_PushParserState(Compiler, clvPARSER_IN_TYPEDEF);
		}
		single_declaration
		{
		   $$ = clParseTypeDef(Compiler, $3);
		   cloCOMPILER_PopParserState(Compiler);
		}
	| init_declarator_list ',' direct_declarator attribute_specifier_opt
		{ $$ = clParseVariableDeclList(Compiler, $1, &$3, $4); }
	| init_declarator_list ',' direct_declarator array_declarator attribute_specifier_opt
		{ $$ = clParseArrayVariableDeclList(Compiler, $1, &$3, $4, $5); }
	| init_declarator_list ',' direct_declarator attribute_specifier_opt '='
		{ $<declOrDeclList>$ = clParseVariableDeclListInit(Compiler, $1, &$3, $4); }
		initializer
		{ $$ = clParseFinishDeclListInit(Compiler, $<declOrDeclList>6, $7); }
	| init_declarator_list ',' direct_declarator array_declarator attribute_specifier_opt '='
		{ $<declOrDeclList>$ = clParseArrayVariableDeclListInit(Compiler, $1, &$3, $4, $5); }
 		initializer
		{ $$ = clParseFinishDeclListInit(Compiler, $<declOrDeclList>7, $8); }
	;


single_declaration :
	function_prototype
		{ $$ = clParseFuncDecl(Compiler, $1); }
	| fully_specified_type direct_declarator attribute_specifier_opt
		{ $$ = clParseVariableDecl(Compiler, &$1, &$2, $3); }
	| fully_specified_type direct_declarator array_declarator attribute_specifier_opt
		{ $$ = clParseArrayVariableDecl(Compiler, &$1, &$2, $3, $4); }
	| fully_specified_type direct_declarator attribute_specifier_opt '='
		{ $<declOrDeclList>$ = clParseVariableDeclInit(Compiler, &$1, &$2, $3); }
 		initializer
		{ $$ = clParseFinishDeclInit(Compiler, $<declOrDeclList>5, $6); }
	| fully_specified_type direct_declarator array_declarator attribute_specifier_opt '='
		{ $<declOrDeclList>$ = clParseArrayVariableDeclInit(Compiler, &$1, &$2, $3, $4); }
 		initializer
		{ $$ = clParseFinishDeclInit(Compiler, $<declOrDeclList>6, $7); }
	;

/* Grammar Note: No 'enum', or 'typedef'. */

attribute_specifier :
	T_ATTRIBUTE__ '(' '('
		{ $<attr>$ = gcvNULL; }
		attribute_list ')' ')'
		{ $$ = $5; }
	;

attribute_specifier_opt :
	{ $$ = gcvNULL; }
	| attribute_specifier
		{ $$ = $1; }
	;

attribute_list :
		{ $$ = $<attr>0; }
	| attribute
		{ $$ = $1; }
	| attribute_list ','
		{ $<attr>$ = $1; }
		attribute
		{ $$ = $4; }
	;

attribute :
	T_ENDIAN '(' T_IDENTIFIER ')'
		{ $$ = clParseAttributeEndianType(Compiler, $<attr>0,  &$3); }
	| T_PACKED
		{ $$ = clParseSimpleAttribute(Compiler, &$1, clvATTR_PACKED, $<attr>0); }
	| T_VEC_TYPE_HINT '(' T_BUILTIN_DATA_TYPE ')'
		{ $$ = clParseAttributeVecTypeHint(Compiler, $<attr>0, &$3); }
	| T_REQD_WORK_GROUP_SIZE '(' constant_expression ',' constant_expression ',' constant_expression ')'
		{ $$ = clParseAttributeReqdWorkGroupSize(Compiler, $<attr>0, $3, $5, $7); }
	| T_WORK_GROUP_SIZE_HINT '(' constant_expression ',' constant_expression ',' constant_expression ')'
		{ $$ = clParseAttributeWorkGroupSizeHint(Compiler, $<attr>0, $3, $5, $7); }
	| T_KERNEL_SCALE_HINT '(' constant_expression ',' constant_expression ',' constant_expression ')'
		{ $$ = clParseAttributeKernelScaleHint(Compiler, $<attr>0, $3, $5, $7); }
	| T_ALIGNED
		{ $$ = clParseAttributeAligned(Compiler, $<attr>0, gcvNULL); }
	| T_ALIGNED '(' constant_expression ')'
		{ $$ = clParseAttributeAligned(Compiler, $<attr>0, $3); }
	| T_ALWAYS_INLINE
		{ $$ = clParseSimpleAttribute(Compiler, &$1, clvATTR_ALWAYS_INLINE, $<attr>0); }
	;

typeof_type_specifier :
	fully_specified_type
		{ $$ = clParseCreateDecl(Compiler, &$1, gcvNULL, gcvNULL); }
	| fully_specified_type '[' constant_expression ']'
		{ $$ = clParseCreateDecl(Compiler, &$1, gcvNULL, $3); }
	| fully_specified_type pointer
		{ $$ = clParseCreateDecl(Compiler, &$1, $2, gcvNULL); }
	| fully_specified_type pointer '[' constant_expression ']'
		{ $$ = clParseCreateDecl(Compiler, &$1, $2, $4); }
	| '(' fully_specified_type pointer ')' '[' constant_expression ']'
		{ $$ = clParseCreateDecl(Compiler, &$2, $3, $6); }
        |  expression
		{ $$ = clParseCreateDeclFromExpression(Compiler, $1); }
	;

fully_specified_type :
	type_specifier
                { $$ = clParseQualifiedType(Compiler, gcvNULL, gcvFALSE, &$1); }
	| type_qualifier_list type_specifier
                { $$ = clParseQualifiedType(Compiler, $1, gcvFALSE, &$2); }
	| type_specifier type_qualifier_list
                { $$ = clParseQualifiedType(Compiler, $2, gcvFALSE, &$1); }
	| type_qualifier_list type_specifier type_qualifier_list
                {
                    clsDECL decl;

                    decl = clParseQualifiedType(Compiler, $1, gcvFALSE, &$2);
                    $$ = clParseQualifiedType(Compiler, $3, gcvFALSE, &decl);
                }
	| T_TYPEOF '(' typeof_type_specifier ')'
                { $$ = $3; }
	;

type_qualifier :
	T_CONST
		{ $$ = $1; }
	| T_RESTRICT
		{ $$ = $1; }
	| T_VOLATILE
		{ $$ = $1; }
	| T_CONSTANT
		{ $$ = $1; }
	| T_GLOBAL
		{ $$ = $1; }
	| T_LOCAL
		{ $$ = $1; }
	| T_PRIVATE
		{ $$ = $1; }
	| T_STATIC
		{ $$ = $1; }
	| T_EXTERN
		{ $$ = $1; }
	| T_UNIFORM
		{ $$ = $1; }
	;

type_specifier :
	type_name
		{ $$ = clParseNonStructType(Compiler, &$1); }
	| struct_union_specifier
		{ $$ = clParseCreateDeclFromDataType(Compiler, $1); }
	| tags
		{ $$ = clParseCreateDeclFromDataType(Compiler, $1); }
        | enum_specifier
		{ $$ = clParseCreateDeclFromDataType(Compiler, $1); }
	;

type_name :
	T_VOID
		{ $$ = $1}
	| T_FLOAT
		{ $$ = $1}
	| T_INT
		{ $$ = $1}
	| T_BOOL
		{ $$ = $1}
	| T_FLOATNXM
		{ $$ = $1}
	| T_DOUBLENXM
		{ $$ = $1}
	| T_BUILTIN_DATA_TYPE
		{ $$ = $1}
	| T_RESERVED_DATA_TYPE
		{ $$ = $1}
	| T_VIV_PACKED_DATA_TYPE
		{ $$ = $1}
	| T_IMAGE1D_T
		{ $$ = $1}
	| T_IMAGE1D_ARRAY_T
		{ $$ = $1}
	| T_IMAGE1D_BUFFER_T
		{ $$ = $1}
	| T_IMAGE2D_ARRAY_T
		{ $$ = $1}
	| T_IMAGE2D_T
		{ $$ = $1}
	| T_IMAGE2D_PTR_T
		{ $$ = $1}
	| T_IMAGE2D_DYNAMIC_ARRAY_T
		{ $$ = $1}
	| T_IMAGE3D_T
		{ $$ = $1}
	| T_SAMPLER_T
		{ $$ = $1}
	| T_PTRDIFF_T
		{ $$ = $1}
	| T_INTPTR_T
		{ $$ = $1}
	| T_UINTPTR_T
		{ $$ = $1}
	| T_SIZE_T
		{ $$ = $1}
	| T_EVENT_T
		{ $$ = $1}
	| T_TYPE_NAME
		{ $$ = $1}
	;

struct_or_union : T_STRUCT
			{ $$ = $1; }
	        | T_UNION
			{ $$ = $1; }
	        ;

struct_union_specifier :
	struct_or_union T_IDENTIFIER '{'
		{ clParseStructDeclBegin(Compiler, &$1,  &$2); }
		struct_declaration_list '}' attribute_specifier_opt
		{ $$ = clParseStructDeclEnd(Compiler, &$2, $7, $5); }
	| struct_or_union '{'
		{ clParseStructDeclBegin(Compiler, &$1, gcvNULL); }
		struct_declaration_list '}' attribute_specifier_opt
		{ $$ = clParseStructDeclEnd(Compiler, gcvNULL, $6, $4); }
	| struct_or_union attribute_specifier T_IDENTIFIER '{'
		{ clParseStructDeclBegin(Compiler, &$1, &$3); }
		struct_declaration_list '}'
		{ $$ = clParseStructDeclEnd(Compiler, &$3, $2, $6); }
	| struct_or_union attribute_specifier '{'
		{ clParseStructDeclBegin(Compiler, &$1, gcvNULL); }
		struct_declaration_list '}'
		{ $$ = clParseStructDeclEnd(Compiler, gcvNULL, $2, $5); }
	;

struct_declaration_list :
	struct_declaration
		{
		   $$ = $1;
		}
	| struct_declaration_list struct_declaration
		{
		   if(gcmIS_ERROR($1)) {
                       $$ = $1;
		   }
		   else {
                       $$ = $2;

		   }
		}
	;

struct_declaration :
	fully_specified_type struct_declarator_list ';'
		{
                   $$ = clParseTypeSpecifiedFieldDeclList(Compiler, &$1, $2);
		}
	;

struct_declarator_list :
	struct_declarator
		{ $$ = clParseFieldDeclList(Compiler, $1); }
	| struct_declarator_list ',' struct_declarator
		{ $$ = clParseFieldDeclList2(Compiler, $1, $3); }
	;

struct_declarator :
		{ $$ = clParseFieldDecl(Compiler, gcvNULL, gcvNULL, gcvNULL); }
	| direct_declarator attribute_specifier_opt
		{ $$ = clParseFieldDecl(Compiler, &$1, gcvNULL, $2); }
	| direct_declarator array_declarator attribute_specifier_opt
		{ $$ = clParseFieldDecl(Compiler, &$1, $2, $3); }
	;

initializer_list_start :
	'{'
		{
		   cloIR_TYPECAST_ARGS typeCastArgs;
		   gceSTATUS status;

		   /* Create type cast expression */
		   status = cloIR_TYPECAST_ARGS_Construct(Compiler,
							  $<token>1.lineNo,
							  $<token>1.stringNo,
							  &typeCastArgs);
		   if(gcmIS_ERROR(status)) {
		      YYERROR;
		      $$ = gcvNULL;
		   }
		   else {
		      $$ = &typeCastArgs->exprBase;
		   }
		}
	;


initializer :
	assignment_expression
		{ $$ = $1; }
	| initializer_list_start initializer_list '}'
		{ $$ = $2; }
	| initializer_list_start  initializer_list T_INITIALIZER_END
		{ $$ = $2; }
	;

initializer_list :
	designation initializer
		{ $$ = clParseInitializerList(Compiler, $<expr>0, $1, $2); }
	| initializer_list ',' designation initializer
		{ $$ = clParseInitializerList(Compiler, $1, $3, $4); }
	;

designation :
		{ $$ = gcvNULL; }
	    | designator_list '='
		{
		   gceSTATUS status;
		   status = cloCOMPILER_PushDesignationScope(Compiler,
							     $1->lhs);
		   if(gcmIS_ERROR(status)) {
		     YYERROR;
		   }
		   $$ = $1;
		}
	    ;

designator_list :
	designator
		{ $$ = $1; }
	| designator_list
		{
		   $<token>$.type = T_EOF;
		}
		designator
		{ $$ = $3; }
	;

designator :
	 '[' constant_expression ']'
		{
		  clsLexToken *token;

		  token = &$<token>0;
		  if(token->type != '{' &&
		     token->type != ',' &&
		     token->type != T_EOF) {
		     gcmASSERT(0);
		     YYERROR;
		  }
		  $$ = clParseSubscriptDesignator(Compiler, $<declOrDeclList>-1, $2, token->type);
		}
	| '.'  T_FIELD_SELECTION
		{
		  clsLexToken *token;

		  token = &$<token>0;
		  if(token->type != '{' &&
		     token->type != ',' &&
		     token->type != T_EOF) {
		     gcmASSERT(0);
		     YYERROR;
		  }
		  $$ = clParseFieldSelectionDesignator(Compiler, $<declOrDeclList>-1, &$2, token->type);
		}
	;

declaration_statement :
	declaration
		{ $$ = $1; }
	;

statement :
	compound_statement
		{ $$ = clParseCompoundStatementAsStatement(Compiler, $1); }
	| simple_statement
		{ $$ = $1; }
	;

simple_statement :
	declaration_statement
		{ $$ = $1; }
	| expression_statement
		{ $$ = $1; }
	| selection_statement
		{ $$ = $1; }
	| iteration_statement
		{ $$ = $1; }
	| jump_statement
		{ $$ = $1; }
	| T_IDENTIFIER ':'
		{ $$ = clParseStatementLabel(Compiler, &$1); }
	| error ';'
		{ yyclearin;
		  yyerrok;
		  $$ = gcvNULL; }
	| error '}'
		{ yyclearin;
		  yyerrok;
		  $$ = gcvNULL; }
	;

compound_statement :
	'{' '}'
		{ $$ = gcvNULL; }
	| '{'
		{ clParseCompoundStatementBegin(Compiler); }
		statement_list '}'
		{ $$ = clParseCompoundStatementEnd(Compiler, &$1, $3, &$4); }
	;

statement_no_new_scope :
	compound_statement_no_new_scope
		{ $$ = clParseCompoundStatementNoNewScopeAsStatementNoNewScope(Compiler, $1); }
	| simple_statement
		{ $$ = $1; }
	;

compound_statement_no_new_scope :
	'{' '}'
		{ $$ = gcvNULL; }
	| '{'
			{ clParseCompoundStatementNoNewScopeBegin(Compiler); }
		statement_list '}'
		{ $$ = clParseCompoundStatementNoNewScopeEnd(Compiler, &$1, $3, &$4); }
	;

statement_list :
	statement
		{ $$ = clParseStatementList(Compiler, $1); }
	| statement_list statement
		{ $$ = clParseStatementList2(Compiler, $1, $2); }
	;

expression_statement :
	';'
		{ $$ = gcvNULL; }
	| expression ';'
		{ $$ = clParseExprAsStatement(Compiler, $1); }
	;

selection_statement :
	T_IF '(' expression ')' if_sub_statement
		{ $$ = clParseIfStatement(Compiler, &$1, $3, $5); }
	| T_SWITCH '(' integer_expression ')' switch_body
		{ $$ = clParseSwitchStatement(Compiler, &$1, $3, $5); }
	;

switch_body_statement_list :
	switch_body_statement
		{ $$ = clParseStatementList(Compiler, $1); }
	| switch_body_statement_list switch_body_statement
		{ $$ = clParseStatementList2(Compiler, $1, $2); }
	;

switch_body_statement:
	statement
		{ $$ = $1; }
	| T_CASE constant_expression  ':'
		{ $$ = clParseCaseStatement(Compiler, &$1, $2); }
	| T_DEFAULT ':'
		{ $$ = clParseDefaultStatement(Compiler, &$1); }
	;

switch_body :
	'{' '}'
		{ $$ = gcvNULL; }
	| '{'
		{ clParseSwitchBodyBegin(Compiler); }
		switch_body_statement_list '}'
		{ $$ = clParseSwitchBodyEnd(Compiler, &$1, $3, &$4); }
	;

if_sub_statement :
	statement T_ELSE statement
		{ $$ = clParseIfSubStatements(Compiler, $1, $3); }
	| statement						%prec T_IF
		{ $$ = clParseIfSubStatements(Compiler, $1, gcvNULL); }
	;

/* Grammar Note: No 'switch'. Switch statements not supported. */

iteration_statement :
	T_WHILE
		{ clParseWhileStatementBegin(Compiler); }
		'(' expression ')' statement_no_new_scope
		{ $$ = clParseWhileStatementEnd(Compiler, &$1, $4, $6); }
	| T_DO statement T_WHILE '(' expression ')' ';'
		{ $$ = clParseDoWhileStatement(Compiler, &$1, $2, $5); }
	| T_FOR
		{ clParseForStatementBegin(Compiler); }
		for_init_statement for_control statement_no_new_scope
		{ $$ = clParseForStatementEnd(Compiler, &$1, $3, $4, $5); }
	;

for_init_statement :
	'(' expression_statement
		{ $$ = $2; }
	| '(' declaration_statement
		{ $$ = $2; }
	| '(' error ';'
		{ yyclearin;
		  yyerrok;
		  $$ = gcvNULL; }
	;

conditionopt :
	expression
		{ $$ = $1; }
	| /* empty */
		{ $$ = gcvNULL; }
	;

for_control :
	conditionopt ';' ')'
		{ $$ = clParseForControl(Compiler, $1, gcvNULL); }
	| conditionopt ';' expression ')'
		{ $$ = clParseForControl(Compiler, $1, $3); }
	| error ';' ')'
		{
		  clsForExprPair nullPair = {gcvNULL, gcvNULL};
		  yyclearin;
		  yyerrok;
		  $$ = nullPair; }
	| error ';' expression ')'
		{
		  clsForExprPair nullPair = {gcvNULL, gcvNULL};
		  yyclearin;
		  yyerrok;
		  $$ = nullPair; }
	;

jump_statement :
	T_CONTINUE ';'
		{ $$ = clParseJumpStatement(Compiler, clvCONTINUE, &$1, gcvNULL); }
	| T_BREAK ';'
		{ $$ = clParseJumpStatement(Compiler, clvBREAK, &$1, gcvNULL); }
	| T_RETURN ';'
		{ $$ = clParseJumpStatement(Compiler, clvRETURN, &$1, gcvNULL); }
	| T_RETURN expression ';'
		{ $$ = clParseJumpStatement(Compiler, clvRETURN, &$1, $2); }
	| T_GOTO T_IDENTIFIER ';'
		{ $$ = clParseGotoStatement(Compiler, &$1, &$2); }
	;

/* Grammar Note: No 'goto'. Gotos are not supported. */

translation_unit :
	| translation_unit external_declaration
	;

external_declaration :
	function_definition
	| declaration
		{ clParseExternalDecl(Compiler, $1); }
	| ';'
	;

function_definition :
	function_prototype compound_statement_no_new_scope
		{ clParseFuncDef(Compiler, $1, $2); }
	;

%%

