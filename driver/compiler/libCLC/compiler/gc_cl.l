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


%{
#include "gc_cl_scanner.h"

static gctUINT CurrentLineNo		= 0;
static gctUINT CurrentStringNo		= 0;
static gctSTRING CurrentFileName	= gcvNULL;

void
clScanComment(
	IN cloCOMPILER Compiler
	);

void
clScanSkipToEOL(
	IN cloCOMPILER Compiler
	);

#define YY_NO_UNISTD_H

/*#define YY_FATAL_ERROR(msg)	\
	clReport(0, 0, clvREPORT_FATAL_ERROR, msg)*/

#define YY_INPUT(buf, result, max_size) \
	{ \
		 result = clInput(max_size, buf); \
	}

#define YY_DECL \
	gctINT \
	cloCOMPILER_Scan( \
		IN cloCOMPILER Compiler, \
		OUT clsLexToken * Token \
		)

#define DUMP_SOURCE() \
	gcmVERIFY_OK(cloCOMPILER_Dump( \
							Compiler, \
							clvDUMP_SOURCE, \
							"<SOURCE line=\"%d\" string=\"%d\">'%s'</SOURCE>", \
							CurrentLineNo, \
							CurrentStringNo, \
							yytext))

#define DUMP_SOURCE_COMMENT_BEGIN() \
	gcmVERIFY_OK(cloCOMPILER_Dump( \
							Compiler, \
							clvDUMP_SOURCE, \
							"<SOURCE_COMMENT line=\"%d\" string=\"%d\">'%s", \
							CurrentLineNo, \
							CurrentStringNo, \
							yytext))

#define DUMP_SOURCE_COMMENT_END() \
	gcmVERIFY_OK(cloCOMPILER_Dump( \
							Compiler, \
							clvDUMP_SOURCE, \
							"'</SOURCE_COMMENT>"))

#define DUMP_SOURCE_CHAR(ch) \
	gcmVERIFY_OK(cloCOMPILER_Dump( \
							Compiler, \
							clvDUMP_SOURCE, \
							"%c", \
							ch))


#ifdef CL_SCAN_NO_ACTION

	#define CL_SCAN_DOUBLE_MATRIX_TYPE()	T_DOUBLENXM
	#define CL_SCAN_FLOAT_MATRIX_TYPE)	T_FLOATNXM
	#define CL_SCAN_UNSIGNED_INTEGER_TYPE() T_UINT
	#define CL_SCAN_BUILTIN_DATA_TYPE()	T_BUILTIN_DATA_TYPE
	#define CL_SCAN_VIV_PACKED_DATA_TYPE()	T_VIV_PACKED_DATA_TYPE
	#define CL_SCAN_RESERVED_DATA_TYPE()	T_RESERVED_DATA_TYPE
	#define CL_SCAN_IDENTIFIER()		T_IDENTIFIER
	#define CL_SCAN_BOOLCONSTANT(value)	T_BOOLCONSTANT
	#define CL_SCAN_DEC_INTCONSTANT()	T_INTCONSTANT
	#define CL_SCAN_OCT_INTCONSTANT()	T_INTCONSTANT
	#define CL_SCAN_HEX_INTCONSTANT()	T_INTCONSTANT
	#define CL_SCAN_FLOATCONSTANT()		T_FLOATCONSTANT
	#define CL_SCAN_HEXFLOATCONSTANT()	T_FLOATCONSTANT
	#define CL_SCAN_CHARCONSTANT()		T_CHARCONSTANT
	#define CL_SCAN_STRING_LITERAL()	T_STRING_LITERAL
	#define CL_SCAN_OPERATOR(type)		(type)
	#define CL_SCAN_FIELD_SELECTION()	T_FIELD_SELECTION

#else

	#define CL_SCAN_FLOAT_MATRIX_TYPE()		\
		clScanMatrixType(Compiler, CurrentLineNo, CurrentStringNo, T_FLOATNXM, yytext, Token)
	#define CL_SCAN_DOUBLE_MATRIX_TYPE()		\
		clScanMatrixType(Compiler, CurrentLineNo, CurrentStringNo, T_DOUBLENXM, yytext, Token)
	#define CL_SCAN_UNSIGNED_INTEGER_TYPE()		\
		clScanConvToUnsignedType(Compiler, CurrentLineNo, CurrentStringNo, yytext, Token)
	#define CL_SCAN_BUILTIN_DATA_TYPE()		\
		clScanBuiltinDataType(Compiler, CurrentLineNo, CurrentStringNo, yytext, Token)
	#define CL_SCAN_VIV_PACKED_DATA_TYPE()		\
		clScanVivPackedDataType(Compiler, CurrentLineNo, CurrentStringNo, yytext, Token)
	#define CL_SCAN_RESERVED_DATA_TYPE()		\
		clScanReservedDataType(Compiler, CurrentLineNo, CurrentStringNo, yytext, Token)
	#define CL_SCAN_IDENTIFIER()		\
		clScanIdentifier(Compiler, CurrentLineNo, CurrentStringNo, yytext, Token)

	#define CL_SCAN_BOOLCONSTANT(value)	\
		clScanBoolConstant(Compiler, CurrentLineNo, CurrentStringNo, value, Token)

	#define CL_SCAN_DEC_INTCONSTANT()	\
		clScanDecIntConstant(Compiler, CurrentLineNo, CurrentStringNo, yytext, Token)

	#define CL_SCAN_OCT_INTCONSTANT()	\
		clScanOctIntConstant(Compiler, CurrentLineNo, CurrentStringNo, yytext, Token)

	#define CL_SCAN_HEX_INTCONSTANT()	\
		clScanHexIntConstant(Compiler, CurrentLineNo, CurrentStringNo, yytext, Token)

	#define CL_SCAN_FLOATCONSTANT()		\
		clScanFloatConstant(Compiler, CurrentLineNo, CurrentStringNo, yytext, Token)

	#define CL_SCAN_HEXFLOATCONSTANT()		\
		clScanHexFloatConstant(Compiler, CurrentLineNo, CurrentStringNo, yytext, Token)

	#define CL_SCAN_CHARCONSTANT()		\
		clScanCharConstant(Compiler, CurrentLineNo, CurrentStringNo, yytext, Token)

	#define CL_SCAN_STRING_LITERAL()		\
		clScanStringLiteral(Compiler, CurrentLineNo, CurrentStringNo, yytext, Token)

	#define CL_SCAN_OPERATOR(type)		\
		clScanOperator(Compiler, CurrentLineNo, CurrentStringNo, yytext, type, Token)

	#define CL_SCAN_FIELD_SELECTION()	\
		clScanFieldSelection(Compiler, CurrentLineNo, CurrentStringNo, yytext, Token)

#endif /* CL_SCAN_NO_ACTION */

#define ECHO        do { /* Do nothing */ } while (gcvFALSE)
%}

%option outfile="gc_cl_scanner.c"
%option never-interactive
/*
%option nounput
*/

%x START_OF_DIRECTIVE
%x SCAN_FIELD_SELECTION
%x SCAN_DIRECTIVE_LINE_NO
%x SCAN_DIRECTIVE_FILE_NAME

identifier					{nodigit}({nodigit}|{digit})*
nodigit						[_A-Za-z]
digit						[0-9]
nonzero_digit					[1-9]
vec_elems 					2|3|4|8|16
extended_vec_elems				2|3|4|8|16|32
character_constant				\'[^'\n]+\'
string_literal                                  \"[^"\n]*\"

decimal_constant				{nozero_digit}{digit}*
octal_constant					0{octal_digit}*
hexadecimal_constant				0[Xx]{hexadecimal_digit}+

nozero_digit					[1-9]
octal_digit					[0-7]
hexadecimal_digit				[0-9A-Fa-f]

floating_constant				{floating_constant_1}|{floating_constant_2}
floating_constant_1				{fractional_constant}{exponent_part}?
floating_constant_2				{digit_sequence}{exponent_part}

hex_floating_constant				{hex_fractional_constant}{hex_exponent_part}

fractional_constant				{fractional_constant_1}|{fractional_constant_2}
fractional_constant_1				{digit_sequence}?"."{digit_sequence}
fractional_constant_2				{digit_sequence}"."

hex_fractional_constant				{hexadecimal_constant}"."{hexadecimal_digit}+

unsigned_suffix					[uU]
long_suffix					[lL]
long_long_suffix				ll|LL
floating_suffix					[fFlL]
int_precision					{long_suffix}|{long_long_suffix}
integer_suffix					{unsigned_suffix}{int_precision}?|{int_precision}{unsigned_suffix}?
exponent_part					[eE]{sign}?{digit_sequence}
hex_exponent_part				[pP]{sign}?{digit_sequence}
sign						[+-]
digit_sequence					{digit}+
signed_integer_type				int|long|char|short
builtin_data_type				{signed_integer_type}|float|uint|ulong|uchar|ushort|bool
viv_packed_data_type				_viv_bool|_viv_char|_viv_uchar|_viv_short|_viv_ushort|_viv_half
reserved_data_type				half|quad|double
white_sp                                        [ \t]
white_sp_nl                                     [ \t\n]
file_name                                       {string_literal}|[^ \t\n]+
%%

"true"						{ DUMP_SOURCE(); return CL_SCAN_BOOLCONSTANT(gcvTRUE); }
"false"						{ DUMP_SOURCE(); return CL_SCAN_BOOLCONSTANT(gcvFALSE); }

"unsigned"{white_sp_nl}+{signed_integer_type}?	{ DUMP_SOURCE();
						    return CL_SCAN_UNSIGNED_INTEGER_TYPE(); }
"float"{vec_elems}+x{nonzero_digit}+	        { DUMP_SOURCE(); return CL_SCAN_FLOAT_MATRIX_TYPE(); }
"double"{vec_elems}+x{nonzero_digit}+           { DUMP_SOURCE(); return CL_SCAN_DOUBLE_MATRIX_TYPE(); }
{builtin_data_type}{vec_elems}*		        { DUMP_SOURCE(); return CL_SCAN_BUILTIN_DATA_TYPE(); }
{viv_packed_data_type}{extended_vec_elems}*"_packed"	{ DUMP_SOURCE(); return CL_SCAN_VIV_PACKED_DATA_TYPE(); }
{reserved_data_type}{vec_elems}*		{ DUMP_SOURCE(); return CL_SCAN_RESERVED_DATA_TYPE(); }
{identifier}					{ DUMP_SOURCE(); return CL_SCAN_IDENTIFIER(); }

{decimal_constant}{integer_suffix}?		{ DUMP_SOURCE(); return CL_SCAN_DEC_INTCONSTANT(); }
{octal_constant}{integer_suffix}?		{ DUMP_SOURCE(); return CL_SCAN_OCT_INTCONSTANT(); }
{hexadecimal_constant}{integer_suffix}?		{ DUMP_SOURCE(); return CL_SCAN_HEX_INTCONSTANT(); }

{floating_constant}{floating_suffix}?		{ DUMP_SOURCE(); return CL_SCAN_FLOATCONSTANT(); }
{hex_floating_constant}{floating_suffix}?		{ DUMP_SOURCE(); return CL_SCAN_HEXFLOATCONSTANT(); }

{character_constant}				{
						  /*check if char before closing quote is a '\' */
						  if(yytext[yyleng - 2] == '\\') {
						     yyless(yyleng - 1); /* return last quote ' */
						     yymore();
						  }
						  else {
						    DUMP_SOURCE(); return CL_SCAN_CHARCONSTANT();
						  }
						}

{string_literal}				{
						  /*check if char before closing quote is a '\' */
						  if(yytext[yyleng - 2] == '\\') {
						     yyless(yyleng - 1); /* return last quote " */
						     yymore();
						  }
						  else {
						    DUMP_SOURCE(); return CL_SCAN_STRING_LITERAL();
						  }
						}

"<<"						{ DUMP_SOURCE(); return CL_SCAN_OPERATOR(T_LSHIFT_OP); }
">>"						{ DUMP_SOURCE(); return CL_SCAN_OPERATOR(T_RSHIFT_OP); }
"++"						{ DUMP_SOURCE(); return CL_SCAN_OPERATOR(T_INC_OP); }
"--"						{ DUMP_SOURCE(); return CL_SCAN_OPERATOR(T_DEC_OP); }
"<="						{ DUMP_SOURCE(); return CL_SCAN_OPERATOR(T_LE_OP); }
">="						{ DUMP_SOURCE(); return CL_SCAN_OPERATOR(T_GE_OP); }
"=="						{ DUMP_SOURCE(); return CL_SCAN_OPERATOR(T_EQ_OP); }
"!="						{ DUMP_SOURCE(); return CL_SCAN_OPERATOR(T_NE_OP); }
"&&"						{ DUMP_SOURCE(); return CL_SCAN_OPERATOR(T_AND_OP); }
"||"						{ DUMP_SOURCE(); return CL_SCAN_OPERATOR(T_OR_OP); }
"^^"						{ DUMP_SOURCE(); return CL_SCAN_OPERATOR(T_XOR_OP); }
"*="						{ DUMP_SOURCE(); return CL_SCAN_OPERATOR(T_MUL_ASSIGN); }
"/="						{ DUMP_SOURCE(); return CL_SCAN_OPERATOR(T_DIV_ASSIGN); }
"+="						{ DUMP_SOURCE(); return CL_SCAN_OPERATOR(T_ADD_ASSIGN); }
"%="						{ DUMP_SOURCE(); return CL_SCAN_OPERATOR(T_MOD_ASSIGN); }
"<<="						{ DUMP_SOURCE(); return CL_SCAN_OPERATOR(T_LEFT_ASSIGN); }
">>="						{ DUMP_SOURCE(); return CL_SCAN_OPERATOR(T_RIGHT_ASSIGN); }
"&="						{ DUMP_SOURCE(); return CL_SCAN_OPERATOR(T_AND_ASSIGN); }
"^="						{ DUMP_SOURCE(); return CL_SCAN_OPERATOR(T_XOR_ASSIGN); }
"|="						{ DUMP_SOURCE(); return CL_SCAN_OPERATOR(T_OR_ASSIGN); }
"-="						{ DUMP_SOURCE(); return CL_SCAN_OPERATOR(T_SUB_ASSIGN); }

"("						{ DUMP_SOURCE(); return CL_SCAN_OPERATOR('('); }
")"						{ DUMP_SOURCE(); return CL_SCAN_OPERATOR(')'); }
"["						{ DUMP_SOURCE(); return CL_SCAN_OPERATOR('['); }
"<:"						{ DUMP_SOURCE(); return CL_SCAN_OPERATOR('['); }
"]"						{ DUMP_SOURCE(); return CL_SCAN_OPERATOR(']'); }
":>"						{ DUMP_SOURCE(); return CL_SCAN_OPERATOR(']'); }
"{"						{ DUMP_SOURCE(); return CL_SCAN_OPERATOR('{'); }
"<%"						{ DUMP_SOURCE(); return CL_SCAN_OPERATOR('{'); }
"}"						{ DUMP_SOURCE(); return CL_SCAN_OPERATOR('}'); }
"%>"						{ DUMP_SOURCE(); return CL_SCAN_OPERATOR('}'); }
"."						{ BEGIN(SCAN_FIELD_SELECTION);
							DUMP_SOURCE(); return CL_SCAN_OPERATOR('.'); }
"->"						{ BEGIN(SCAN_FIELD_SELECTION);
						  DUMP_SOURCE(); return CL_SCAN_OPERATOR(T_STRUCT_UNION_PTR); }
","						{ DUMP_SOURCE(); return CL_SCAN_OPERATOR(','); }
":"						{ DUMP_SOURCE(); return CL_SCAN_OPERATOR(':'); }
"="						{ DUMP_SOURCE(); return CL_SCAN_OPERATOR('='); }
";"						{ DUMP_SOURCE(); return CL_SCAN_OPERATOR(';'); }
"!"						{ DUMP_SOURCE(); return CL_SCAN_OPERATOR('!'); }
"-"						{ DUMP_SOURCE(); return CL_SCAN_OPERATOR('-'); }
"~"						{ DUMP_SOURCE(); return CL_SCAN_OPERATOR('~'); }
"+"						{ DUMP_SOURCE(); return CL_SCAN_OPERATOR('+'); }
"*"						{ DUMP_SOURCE(); return CL_SCAN_OPERATOR('*'); }
"/"						{ DUMP_SOURCE(); return CL_SCAN_OPERATOR('/'); }
"%"						{ DUMP_SOURCE(); return CL_SCAN_OPERATOR('%'); }
"<"						{ DUMP_SOURCE(); return CL_SCAN_OPERATOR('<'); }
">"						{ DUMP_SOURCE(); return CL_SCAN_OPERATOR('>'); }
"|"						{ DUMP_SOURCE(); return CL_SCAN_OPERATOR('|'); }
"^"						{ DUMP_SOURCE(); return CL_SCAN_OPERATOR('^'); }
"&"						{ DUMP_SOURCE(); return CL_SCAN_OPERATOR('&'); }
"?"						{ DUMP_SOURCE(); return CL_SCAN_OPERATOR('?'); }
","{white_sp_nl}*"}"				{ DUMP_SOURCE(); return CL_SCAN_OPERATOR(T_INITIALIZER_END); }

^{white_sp}*"#"					{ DUMP_SOURCE(); BEGIN(START_OF_DIRECTIVE); }

^{white_sp}*"%:"				{ DUMP_SOURCE(); BEGIN(START_OF_DIRECTIVE); }

<SCAN_FIELD_SELECTION>{identifier}		{ BEGIN(INITIAL);
						  DUMP_SOURCE(); return CL_SCAN_FIELD_SELECTION(); }


<*>"/*"						{ DUMP_SOURCE_COMMENT_BEGIN(); clScanComment(Compiler); }

<SCAN_DIRECTIVE_LINE_NO>{decimal_constant}      { gctINT lineNo;
					          DUMP_SOURCE();
   						  gcoOS_StrToInt(yytext, &lineNo);
						  cloCOMPILER_SetCurrentLineNo(Compiler, lineNo - 1);
					          BEGIN(SCAN_DIRECTIVE_FILE_NAME); }
<SCAN_DIRECTIVE_FILE_NAME>{file_name}           { DUMP_SOURCE();
					          cloCOMPILER_SetCurrentFileName(Compiler, yytext);
                                                  clScanSkipToEOL(Compiler);
                                                  BEGIN(INITIAL); }
<START_OF_DIRECTIVE>"line"?{white_sp}+           { DUMP_SOURCE();
					          BEGIN(SCAN_DIRECTIVE_LINE_NO);}

<START_OF_DIRECTIVE>"pragma"{white_sp}+          { DUMP_SOURCE();
                                                  clScanSkipToEOL(Compiler);
						  BEGIN(INITIAL);	}

<START_OF_DIRECTIVE>"pragma"\n          	{ DUMP_SOURCE(); BEGIN(INITIAL);}

<*>[ \t\n\r\f\v]				{ DUMP_SOURCE(); /* Ignore */ }

<*>"/""/".*					{ DUMP_SOURCE(); /* Ignore */ }

<*>.						{ DUMP_SOURCE();
				          		cloCOMPILER_Report(
										Compiler,
										CurrentLineNo,
										CurrentStringNo,
										clvREPORT_ERROR,
										"unexpected character: '%s'", yytext); }

<*><<EOF>>					{ yy_delete_buffer(YY_CURRENT_BUFFER); yyterminate(); }

%%

void
clScanComment(IN cloCOMPILER Compiler)
{
	gctINT ch;

	while (gcvTRUE) {
		while ((ch = input()) != '*' && ch != EOF) DUMP_SOURCE_CHAR(ch);

		if (ch == EOF) {
			cloCOMPILER_Report(Compiler,
					CurrentLineNo,
					CurrentStringNo,
					clvREPORT_ERROR,
					"invalid comment: unexpected end of file");
			break;
		}
		DUMP_SOURCE_CHAR(ch);
		while ((ch = input()) == '*') DUMP_SOURCE_CHAR(ch);
		if (ch == EOF) {
			cloCOMPILER_Report(Compiler,
					CurrentLineNo,
					CurrentStringNo,
					clvREPORT_ERROR,
					"invalid comment: unexpected end of file");
			break;
		}
		else if (ch == '/') {
			DUMP_SOURCE_CHAR(ch);
			break;
		}
		else {
			DUMP_SOURCE_CHAR(ch);
		}
	}
	DUMP_SOURCE_COMMENT_END();
}

void
clScanSkipToEOL(IN cloCOMPILER Compiler)
{
	gctINT ch;
	while ((ch = input()) != '\n' && ch != EOF) DUMP_SOURCE_CHAR(ch);
	return;
}

gceSTATUS
clScanSetCurrentFileName(
IN cloCOMPILER Compiler,
IN gctSTRING FileName
)
{
   /* Verify the arguments. */
   clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
   CurrentFileName = FileName;
   return gcvSTATUS_OK;
}

gceSTATUS
clScanSetCurrentLineNo(
IN cloCOMPILER Compiler,
IN gctUINT LineNo
)
{
	/* Verify the arguments. */
	clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
	CurrentLineNo = LineNo;
	return gcvSTATUS_OK;
}

gceSTATUS
clScanSetCurrentStringNo(
IN cloCOMPILER Compiler,
IN gctUINT StringNo
)
{
	/* Verify the arguments. */
	clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
	CurrentStringNo	= StringNo;
	return gcvSTATUS_OK;
}

gceSTATUS
clScanLookAhead(
IN cloCOMPILER Compiler,
IN gctINT LookAheadChr
)
{
  gctINT ch;
  while ((ch = input()) != EOF) {
     if((ch ==  ' ') || (ch == '\t')) { /* skip white spaces */
          DUMP_SOURCE_CHAR(ch);
          continue;
     }
     unput(ch);
     if(ch == LookAheadChr) { /*found the lookahead*/
        return gcvSTATUS_OK;
     }
     else break;
  }
  if(yyleng == 1 && yytext[0] == LookAheadChr) {
     return gcvSTATUS_OK;
  }
  return gcvSTATUS_NOT_FOUND;
}

gceSTATUS
clScanLookAheadWithSkip(
IN cloCOMPILER Compiler,
IN gctINT LookAheadChr,
IN gctINT SkipChar
)
{
  gctINT ch;
  gctINT skipCount = 0;
  gctINT i;

  while ((ch = input()) != EOF) {
     if((ch ==  ' ') || (ch == '\t')) { /* skip white spaces */
          DUMP_SOURCE_CHAR(ch);
          continue;
     }
     else if(ch == SkipChar) { /* Is skip character */
          skipCount++;
          continue;
     }

     unput(ch);
     if(ch == LookAheadChr) { /*found the lookahead*/
        for (i = 0; i < skipCount; i++) { /*put back the skipped chararacters*/
          unput(SkipChar);
        }
        return gcvSTATUS_OK;
     }
     else break;
  }
  if(yyleng == 1 && yytext[0] == LookAheadChr) {
     return gcvSTATUS_OK;
  }
  return gcvSTATUS_NOT_FOUND;
}

void
clScanDeleteBuffer(
IN cloCOMPILER Compiler
)
{
   yy_delete_buffer(YY_CURRENT_BUFFER);
}

extern void
yyerror(char *msg)
{
	char *formatString = "%s at '%s'";
	clReport(CurrentLineNo,
		CurrentStringNo,
		clvREPORT_ERROR,
		formatString,
		msg,
		yytext);

}
