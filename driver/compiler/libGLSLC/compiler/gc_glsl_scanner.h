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


#ifndef __gc_glsl_scanner_h_
#define __gc_glsl_scanner_h_

#include "gc_glsl_compiler_int.h"
#include "gc_glsl_ir.h"

typedef struct _slsLexToken
{
    gctUINT                 lineNo;

    gctUINT                 stringNo;

    gctINT                  type;

    union
    {
        sluCONSTANT_VALUE   constant;

        sltPOOL_STRING      identifier;

        slsQUALIFIERS       qualifiers;

        gctINT              operator;

        slsNAME *           typeName;

        sltPOOL_STRING      fieldSelection;

        slsDATA_TYPE *      basicType;  /* use this field, when type == T_BASIC_TYPE */
    }
    u;
}
slsLexToken;

typedef struct _slsDeclOrDeclList
{
    slsDATA_TYPE *    dataType;

    sloIR_BASE        initStatement;

    sloIR_SET        initStatements;
}
slsDeclOrDeclList;

typedef struct _slsFieldDecl
{
    slsDLINK_NODE    node;

    slsNAME *        field;

    gctINT           arrayLength;

    gctINT           arrayLengthCount;  /* arrayLengthCount > 1 means it is an array of arrays. */

    gctINT *         arrayLengthList;  /* Array length list for an array of arrays. */
}
slsFieldDecl;

typedef struct _slsSelectionStatementPair
{
    sloIR_BASE    trueStatement;

    sloIR_BASE    falseStatement;
}
slsSelectionStatementPair;

typedef struct _slsForExprPair
{
    sloIR_EXPR    condExpr;

    sloIR_EXPR    restExpr;
}
slsForExprPair;

#include "gc_glsl_token_def.h"

#ifndef T_EOF
#define T_EOF        0
#endif

gctINT
sloCOMPILER_Scan(
    IN sloCOMPILER Compiler,
    OUT slsLexToken * Token
    );

gctINT
slScanBoolConstant(
    IN sloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gctBOOL Value,
    OUT slsLexToken * Token
    );

gctINT
slScanSpecialIdentifier(
    IN sloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gctSTRING Symbol,
    OUT slsLexToken * Token
    );

gctINT
slScanIdentifier(
    IN sloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gctSTRING Symbol,
    OUT slsLexToken * Token
    );

gceSTATUS
slCleanupKeywords(
    void
    );

gctINT
slScanConvToUnsignedType(
IN sloCOMPILER Compiler,
IN gctUINT LineNo,
IN gctUINT StringNo,
IN gctSTRING Symbol,
OUT slsLexToken * Token
);

gctINT
slScanDecIntConstant(
    IN sloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gctSTRING Text,
    OUT slsLexToken * Token
    );

gctINT
slScanOctIntConstant(
    IN sloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gctSTRING Text,
    OUT slsLexToken * Token
    );

gctINT
slScanHexIntConstant(
    IN sloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gctSTRING Text,
    OUT slsLexToken * Token
    );

gctINT
slScanFloatConstant(
    IN sloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gctSTRING Text,
    OUT slsLexToken * Token
    );

gctINT
slScanOperator(
    IN sloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gctSTRING Text,
    IN gctINT tokenType,
    OUT slsLexToken * Token
    );

gctINT
slScanSpecialOperator(
    IN sloCOMPILER Compiler,
    IN gctUINT Extention,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gctSTRING Text,
    IN gctINT tokenType,
    OUT slsLexToken * Token
    );

gctINT
slScanLengthMethod(
    IN sloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gctSTRING Text,
    OUT slsLexToken * Token
    );
gctINT
slScanFieldSelection(
    IN sloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gctSTRING Symbol,
    OUT slsLexToken * Token
    );

int
yylex(
    YYSTYPE * pyylval,
    sloCOMPILER Compiler
    );

void
yyInitScanner(void);

void
slScanDeleteBuffer(
IN sloCOMPILER Compiler
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
#ifndef FILE
#define FILE    void
#endif
#ifndef stdin
#define stdin   NULL
#endif
#ifndef stdout
#define stdout  NULL
#endif

#endif /* __gc_glsl_scanner_h_ */
