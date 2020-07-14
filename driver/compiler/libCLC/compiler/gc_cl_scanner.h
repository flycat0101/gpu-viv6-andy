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


#ifndef __gc_cl_scanner_h_
#define __gc_cl_scanner_h_
#include "gc_cl_compiler_int.h"
#include "gc_cl_ir.h"

typedef struct _clsLexToken
{
    gctUINT    fileNo;
    gctUINT    lineNo;
    gctUINT    stringNo;
    gctINT    type;
    union {
      cluCONSTANT_VALUE constant;
      struct {
        gctSTRING    value;
        gctINT    len;
      } stringLiteral;
          struct {
        cltPOOL_STRING name;
        slsSLINK_LIST  *ptrDscr;
      } identifier;
      cltQUALIFIER    qualifier;
      gctINT    operator;
      clsNAME *    typeName;
      cltPOOL_STRING fieldSelection;
          clsMATRIX_SIZE matrixSize;
    } u;
}
clsLexToken;

typedef struct _clsDeclOrDeclList
{
    clsNAME *name;
    cloIR_EXPR lhs;
    cloIR_EXPR designator;
    clsDECL decl;
    cloIR_BASE  initStatement;
    cloIR_SET   initStatements;
}
clsDeclOrDeclList;

typedef struct _clsFieldDecl
{
    slsDLINK_NODE node;
    clsNAME *field;
    clsARRAY array;
}
clsFieldDecl;

typedef struct _clsIfStatementPair
{
    cloIR_BASE  trueStatement;
    cloIR_BASE  falseStatement;
}
clsIfStatementPair;

typedef struct _clsForExprPair
{
    cloIR_EXPR  condExpr;
    cloIR_EXPR  restExpr;
}
clsForExprPair;

#include "gc_cl_token_def.h"
/** Number of lexical terminal tokens defined ***/
#define cldNumTerminalTokens (T_VERY_LAST_TERMINAL - T_VERY_FIRST_TERMINAL)

#ifndef T_EOF
#define T_EOF        0
#endif

gctINT
cloCOMPILER_Scan(
IN cloCOMPILER Compiler,
OUT clsLexToken * Token
);

void
clScanInitErrorHandler(
IN cloCOMPILER Compiler
);

gctCONST_STRING *
clScanInitIndexToKeywordTableEntries(void);

void
clScanInitLanguageVersion(
IN gctUINT32 LanguageVersion,
IN cleEXTENSION Extension
);

gctINT
clScanBoolConstant(
IN cloCOMPILER Compiler,
IN gctUINT LineNo,
IN gctUINT StringNo,
IN gctBOOL Value,
OUT clsLexToken * Token
);

gctINT
clScanIdentifier(
IN cloCOMPILER Compiler,
IN gctUINT LineNo,
IN gctUINT StringNo,
IN gctSTRING Symbol,
OUT clsLexToken * Token
);

gctINT
clScanReservedDataType(
IN cloCOMPILER Compiler,
IN gctUINT LineNo,
IN gctUINT StringNo,
IN gctSTRING Symbol,
OUT clsLexToken * Token
);

gctINT
clScanBuiltinDataType(
IN cloCOMPILER Compiler,
IN gctUINT LineNo,
IN gctUINT StringNo,
IN gctSTRING Symbol,
OUT clsLexToken * Token
);

gctINT
clScanVivPackedDataType(
IN cloCOMPILER Compiler,
IN gctUINT LineNo,
IN gctUINT StringNo,
IN gctSTRING Symbol,
OUT clsLexToken * Token
);

gctINT
clScanConvToUnsignedType(
IN cloCOMPILER Compiler,
IN gctUINT LineNo,
IN gctUINT StringNo,
IN gctSTRING Symbol,
OUT clsLexToken * Token
);

gctINT
clScanMatrixDimensions(
IN gctSTRING matrixType,
OUT gctUINT *N,
OUT gctUINT *M,
OUT gctINT *SquareMat
);

gctINT
clScanMatrixType(
IN cloCOMPILER Compiler,
IN gctUINT LineNo,
IN gctUINT StringNo,
IN gctINT TokenType,
IN gctSTRING Symbol,
OUT clsLexToken * Token
);

gctINT
clScanDecIntConstant(
IN cloCOMPILER Compiler,
IN gctUINT LineNo,
IN gctUINT StringNo,
IN gctSTRING Text,
OUT clsLexToken * Token
);

gctINT
clScanOctIntConstant(
IN cloCOMPILER Compiler,
IN gctUINT LineNo,
IN gctUINT StringNo,
IN gctSTRING Text,
OUT clsLexToken * Token
);

gctSIZE_T
clScanStrspn(
IN gctCONST_STRING InStr,
IN gctCONST_STRING MatchChars
);

gctINT
clScanHexIntConstant(
IN cloCOMPILER Compiler,
IN gctUINT LineNo,
IN gctUINT StringNo,
IN gctSTRING Text,
OUT clsLexToken * Token
);

gctINT
clScanFloatConstant(
IN cloCOMPILER Compiler,
IN gctUINT LineNo,
IN gctUINT StringNo,
IN gctSTRING Text,
OUT clsLexToken * Token
);

gctINT
clScanHexFloatConstant(
IN cloCOMPILER Compiler,
IN gctUINT LineNo,
IN gctUINT StringNo,
IN gctSTRING Text,
OUT clsLexToken * Token
);

gctINT
clScanCharConstant(
IN cloCOMPILER Compiler,
IN gctUINT LineNo,
IN gctUINT StringNo,
IN gctSTRING Text,
OUT clsLexToken * Token
);

gctINT
clScanStringLiteral(
IN cloCOMPILER Compiler,
IN gctUINT LineNo,
IN gctUINT StringNo,
IN gctSTRING Text,
OUT clsLexToken * Token
);

gctINT
clScanOperator(
IN cloCOMPILER Compiler,
IN gctUINT LineNo,
IN gctUINT StringNo,
IN gctSTRING Text,
IN gctINT tokenType,
OUT clsLexToken * Token
);

gctINT
clScanSpecialOperator(
IN cloCOMPILER Compiler,
IN cleEXTENSION Extension,
IN gctUINT LineNo,
IN gctUINT StringNo,
IN gctSTRING Text,
IN gctINT tokenType,
OUT clsLexToken * Token
);

gctINT
clScanFieldSelection(
IN cloCOMPILER Compiler,
IN gctUINT LineNo,
IN gctUINT StringNo,
IN gctSTRING Symbol,
OUT clsLexToken * Token
);

gceSTATUS
clScanSetCurrentLineNo(
IN cloCOMPILER Compiler,
IN gctUINT LineNo
);

gceSTATUS
clScanSetCurrentFileName(
IN cloCOMPILER Compiler,
IN gctSTRING Text
);

gceSTATUS
clScanSetCurrentStringNo(
IN cloCOMPILER Compiler,
IN gctUINT StringNo
);

gceSTATUS
clScanLookAhead(
IN cloCOMPILER Compiler,
IN gctINT LookAheadChr
);

gceSTATUS
clScanLookAheadWithSkip(
IN cloCOMPILER Compiler,
IN gctINT LookAheadChr,
IN gctINT SkipChar
);

void
clScanDeleteBuffer(
IN cloCOMPILER Compiler
);

int
cloCOMPILER_Lex(
YYSTYPE * pyylval,
cloCOMPILER Compiler
);

gceSTATUS
clCleanupKeywords(
void
);

void
yyerror(
char *msg
);

#ifndef YY_STACK_USED
#define YY_STACK_USED 0
#endif
#ifndef YY_ALWAYS_INTERACTIVE
#define YY_ALWAYS_INTERACTIVE 0
#endif
#ifndef YY_MAIN
#define YY_MAIN 0
#endif

#ifndef NULL
#define NULL ((void *)0)
#endif
#ifndef EOF
#define EOF     (-1)
#endif
#if !defined(UNDER_CE) || UNDER_CE >=800
#define FILE    void
#define stdin   NULL
#define stdout  NULL
#else
typedef void FILE;
#endif

#endif /* __gc_cl_scanner_h_ */
