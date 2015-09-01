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


/* A Bison parser, made by GNU Bison 1.875c.  */

/* Skeleton parser for Yacc-like parsing with Bison,
   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

/* As a special exception, when this file is copied by Bison into a
   Bison output file, you may use that output file without restriction.
   This special exception was added by the Free Software Foundation
   in version 1.24 of Bison.  */

/* Written by Richard Stallman by simplifying the original so called
   ``semantic'' parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 1

/* Using locations.  */
#define YYLSP_NEEDED 0



/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     T_VERY_FIRST_TERMINAL = 258,
     T_VOID = 259,
     T_MAT2 = 260,
     T_MAT3 = 261,
     T_MAT4 = 262,
     T_MAT8 = 263,
     T_MAT16 = 264,
     T_BOOL = 265,
     T_BOOL2 = 266,
     T_BOOL3 = 267,
     T_BOOL4 = 268,
     T_BOOL8 = 269,
     T_BOOL16 = 270,
     T_HALF = 271,
     T_HALF2 = 272,
     T_HALF3 = 273,
     T_HALF4 = 274,
     T_HALF8 = 275,
     T_HALF16 = 276,
     T_FLOAT = 277,
     T_FLOAT2 = 278,
     T_FLOAT3 = 279,
     T_FLOAT4 = 280,
     T_FLOAT8 = 281,
     T_FLOAT16 = 282,
     T_DOUBLE = 283,
     T_DOUBLE2 = 284,
     T_DOUBLE3 = 285,
     T_DOUBLE4 = 286,
     T_DOUBLE8 = 287,
     T_DOUBLE16 = 288,
     T_QUAD = 289,
     T_QUAD2 = 290,
     T_QUAD3 = 291,
     T_QUAD4 = 292,
     T_QUAD8 = 293,
     T_QUAD16 = 294,
     T_CHAR = 295,
     T_CHAR2 = 296,
     T_CHAR3 = 297,
     T_CHAR4 = 298,
     T_CHAR8 = 299,
     T_CHAR16 = 300,
     T_UCHAR = 301,
     T_UCHAR2 = 302,
     T_UCHAR3 = 303,
     T_UCHAR4 = 304,
     T_UCHAR8 = 305,
     T_UCHAR16 = 306,
     T_SHORT = 307,
     T_SHORT2 = 308,
     T_SHORT3 = 309,
     T_SHORT4 = 310,
     T_SHORT8 = 311,
     T_SHORT16 = 312,
     T_USHORT = 313,
     T_USHORT2 = 314,
     T_USHORT3 = 315,
     T_USHORT4 = 316,
     T_USHORT8 = 317,
     T_USHORT16 = 318,
     T_INT = 319,
     T_INT2 = 320,
     T_INT3 = 321,
     T_INT4 = 322,
     T_INT8 = 323,
     T_INT16 = 324,
     T_UINT = 325,
     T_UINT2 = 326,
     T_UINT3 = 327,
     T_UINT4 = 328,
     T_UINT8 = 329,
     T_UINT16 = 330,
     T_LONG = 331,
     T_LONG2 = 332,
     T_LONG3 = 333,
     T_LONG4 = 334,
     T_LONG8 = 335,
     T_LONG16 = 336,
     T_ULONG = 337,
     T_ULONG2 = 338,
     T_ULONG3 = 339,
     T_ULONG4 = 340,
     T_ULONG8 = 341,
     T_ULONG16 = 342,
     T_SAMPLER_T = 343,
     T_IMAGE1D_T = 344,
     T_IMAGE1D_ARRAY_T = 345,
     T_IMAGE1D_BUFFER_T = 346,
     T_IMAGE2D_ARRAY_T = 347,
     T_IMAGE2D_T = 348,
     T_IMAGE3D_T = 349,
     T_SIZE_T = 350,
     T_EVENT_T = 351,
     T_PTRDIFF_T = 352,
     T_INTPTR_T = 353,
     T_UINTPTR_T = 354,
     T_GENTYPE = 355,
     T_F_GENTYPE = 356,
     T_IU_GENTYPE = 357,
     T_I_GENTYPE = 358,
     T_U_GENTYPE = 359,
     T_SIU_GENTYPE = 360,
     T_BOOL_PACKED = 361,
     T_BOOL2_PACKED = 362,
     T_BOOL3_PACKED = 363,
     T_BOOL4_PACKED = 364,
     T_BOOL8_PACKED = 365,
     T_BOOL16_PACKED = 366,
     T_BOOL32_PACKED = 367,
     T_CHAR_PACKED = 368,
     T_CHAR2_PACKED = 369,
     T_CHAR3_PACKED = 370,
     T_CHAR4_PACKED = 371,
     T_CHAR8_PACKED = 372,
     T_CHAR16_PACKED = 373,
     T_CHAR32_PACKED = 374,
     T_UCHAR_PACKED = 375,
     T_UCHAR2_PACKED = 376,
     T_UCHAR3_PACKED = 377,
     T_UCHAR4_PACKED = 378,
     T_UCHAR8_PACKED = 379,
     T_UCHAR16_PACKED = 380,
     T_UCHAR32_PACKED = 381,
     T_SHORT_PACKED = 382,
     T_SHORT2_PACKED = 383,
     T_SHORT3_PACKED = 384,
     T_SHORT4_PACKED = 385,
     T_SHORT8_PACKED = 386,
     T_SHORT16_PACKED = 387,
     T_SHORT32_PACKED = 388,
     T_USHORT_PACKED = 389,
     T_USHORT2_PACKED = 390,
     T_USHORT3_PACKED = 391,
     T_USHORT4_PACKED = 392,
     T_USHORT8_PACKED = 393,
     T_USHORT16_PACKED = 394,
     T_USHORT32_PACKED = 395,
     T_HALF_PACKED = 396,
     T_HALF2_PACKED = 397,
     T_HALF3_PACKED = 398,
     T_HALF4_PACKED = 399,
     T_HALF8_PACKED = 400,
     T_HALF16_PACKED = 401,
     T_HALF32_PACKED = 402,
     T_FLOATNXM = 403,
     T_DOUBLENXM = 404,
     T_BUILTIN_DATA_TYPE = 405,
     T_RESERVED_DATA_TYPE = 406,
     T_VIV_PACKED_DATA_TYPE = 407,
     T_IDENTIFIER = 408,
     T_TYPE_NAME = 409,
     T_FLOATCONSTANT = 410,
     T_UINTCONSTANT = 411,
     T_INTCONSTANT = 412,
     T_BOOLCONSTANT = 413,
     T_CHARCONSTANT = 414,
     T_STRING_LITERAL = 415,
     T_FIELD_SELECTION = 416,
     T_LSHIFT_OP = 417,
     T_RSHIFT_OP = 418,
     T_INC_OP = 419,
     T_DEC_OP = 420,
     T_LE_OP = 421,
     T_GE_OP = 422,
     T_EQ_OP = 423,
     T_NE_OP = 424,
     T_AND_OP = 425,
     T_OR_OP = 426,
     T_XOR_OP = 427,
     T_MUL_ASSIGN = 428,
     T_DIV_ASSIGN = 429,
     T_ADD_ASSIGN = 430,
     T_MOD_ASSIGN = 431,
     T_LEFT_ASSIGN = 432,
     T_RIGHT_ASSIGN = 433,
     T_AND_ASSIGN = 434,
     T_XOR_ASSIGN = 435,
     T_OR_ASSIGN = 436,
     T_SUB_ASSIGN = 437,
     T_STRUCT_UNION_PTR = 438,
     T_INITIALIZER_END = 439,
     T_BREAK = 440,
     T_CONTINUE = 441,
     T_RETURN = 442,
     T_GOTO = 443,
     T_WHILE = 444,
     T_FOR = 445,
     T_DO = 446,
     T_ELSE = 447,
     T_IF = 448,
     T_SWITCH = 449,
     T_CASE = 450,
     T_DEFAULT = 451,
     T_CONST = 452,
     T_RESTRICT = 453,
     T_VOLATILE = 454,
     T_STATIC = 455,
     T_EXTERN = 456,
     T_CONSTANT = 457,
     T_GLOBAL = 458,
     T_LOCAL = 459,
     T_PRIVATE = 460,
     T_KERNEL = 461,
     T_READ_ONLY = 462,
     T_WRITE_ONLY = 463,
     T_PACKED = 464,
     T_ALIGNED = 465,
     T_ENDIAN = 466,
     T_VEC_TYPE_HINT = 467,
     T_ATTRIBUTE__ = 468,
     T_REQD_WORK_GROUP_SIZE = 469,
     T_WORK_GROUP_SIZE_HINT = 470,
     T_ALWAYS_INLINE = 471,
     T_UNSIGNED = 472,
     T_STRUCT = 473,
     T_UNION = 474,
     T_TYPEDEF = 475,
     T_ENUM = 476,
     T_INLINE = 477,
     T_SIZEOF = 478,
     T_TYPE_CAST = 479,
     T_VEC_STEP = 480,
     T_VERY_LAST_TERMINAL = 481
   };
#endif
#define T_VERY_FIRST_TERMINAL 258
#define T_VOID 259
#define T_MAT2 260
#define T_MAT3 261
#define T_MAT4 262
#define T_MAT8 263
#define T_MAT16 264
#define T_BOOL 265
#define T_BOOL2 266
#define T_BOOL3 267
#define T_BOOL4 268
#define T_BOOL8 269
#define T_BOOL16 270
#define T_HALF 271
#define T_HALF2 272
#define T_HALF3 273
#define T_HALF4 274
#define T_HALF8 275
#define T_HALF16 276
#define T_FLOAT 277
#define T_FLOAT2 278
#define T_FLOAT3 279
#define T_FLOAT4 280
#define T_FLOAT8 281
#define T_FLOAT16 282
#define T_DOUBLE 283
#define T_DOUBLE2 284
#define T_DOUBLE3 285
#define T_DOUBLE4 286
#define T_DOUBLE8 287
#define T_DOUBLE16 288
#define T_QUAD 289
#define T_QUAD2 290
#define T_QUAD3 291
#define T_QUAD4 292
#define T_QUAD8 293
#define T_QUAD16 294
#define T_CHAR 295
#define T_CHAR2 296
#define T_CHAR3 297
#define T_CHAR4 298
#define T_CHAR8 299
#define T_CHAR16 300
#define T_UCHAR 301
#define T_UCHAR2 302
#define T_UCHAR3 303
#define T_UCHAR4 304
#define T_UCHAR8 305
#define T_UCHAR16 306
#define T_SHORT 307
#define T_SHORT2 308
#define T_SHORT3 309
#define T_SHORT4 310
#define T_SHORT8 311
#define T_SHORT16 312
#define T_USHORT 313
#define T_USHORT2 314
#define T_USHORT3 315
#define T_USHORT4 316
#define T_USHORT8 317
#define T_USHORT16 318
#define T_INT 319
#define T_INT2 320
#define T_INT3 321
#define T_INT4 322
#define T_INT8 323
#define T_INT16 324
#define T_UINT 325
#define T_UINT2 326
#define T_UINT3 327
#define T_UINT4 328
#define T_UINT8 329
#define T_UINT16 330
#define T_LONG 331
#define T_LONG2 332
#define T_LONG3 333
#define T_LONG4 334
#define T_LONG8 335
#define T_LONG16 336
#define T_ULONG 337
#define T_ULONG2 338
#define T_ULONG3 339
#define T_ULONG4 340
#define T_ULONG8 341
#define T_ULONG16 342
#define T_SAMPLER_T 343
#define T_IMAGE1D_T 344
#define T_IMAGE1D_ARRAY_T 345
#define T_IMAGE1D_BUFFER_T 346
#define T_IMAGE2D_ARRAY_T 347
#define T_IMAGE2D_T 348
#define T_IMAGE3D_T 349
#define T_SIZE_T 350
#define T_EVENT_T 351
#define T_PTRDIFF_T 352
#define T_INTPTR_T 353
#define T_UINTPTR_T 354
#define T_GENTYPE 355
#define T_F_GENTYPE 356
#define T_IU_GENTYPE 357
#define T_I_GENTYPE 358
#define T_U_GENTYPE 359
#define T_SIU_GENTYPE 360
#define T_BOOL_PACKED 361
#define T_BOOL2_PACKED 362
#define T_BOOL3_PACKED 363
#define T_BOOL4_PACKED 364
#define T_BOOL8_PACKED 365
#define T_BOOL16_PACKED 366
#define T_BOOL32_PACKED 367
#define T_CHAR_PACKED 368
#define T_CHAR2_PACKED 369
#define T_CHAR3_PACKED 370
#define T_CHAR4_PACKED 371
#define T_CHAR8_PACKED 372
#define T_CHAR16_PACKED 373
#define T_CHAR32_PACKED 374
#define T_UCHAR_PACKED 375
#define T_UCHAR2_PACKED 376
#define T_UCHAR3_PACKED 377
#define T_UCHAR4_PACKED 378
#define T_UCHAR8_PACKED 379
#define T_UCHAR16_PACKED 380
#define T_UCHAR32_PACKED 381
#define T_SHORT_PACKED 382
#define T_SHORT2_PACKED 383
#define T_SHORT3_PACKED 384
#define T_SHORT4_PACKED 385
#define T_SHORT8_PACKED 386
#define T_SHORT16_PACKED 387
#define T_SHORT32_PACKED 388
#define T_USHORT_PACKED 389
#define T_USHORT2_PACKED 390
#define T_USHORT3_PACKED 391
#define T_USHORT4_PACKED 392
#define T_USHORT8_PACKED 393
#define T_USHORT16_PACKED 394
#define T_USHORT32_PACKED 395
#define T_HALF_PACKED 396
#define T_HALF2_PACKED 397
#define T_HALF3_PACKED 398
#define T_HALF4_PACKED 399
#define T_HALF8_PACKED 400
#define T_HALF16_PACKED 401
#define T_HALF32_PACKED 402
#define T_FLOATNXM 403
#define T_DOUBLENXM 404
#define T_BUILTIN_DATA_TYPE 405
#define T_RESERVED_DATA_TYPE 406
#define T_VIV_PACKED_DATA_TYPE 407
#define T_IDENTIFIER 408
#define T_TYPE_NAME 409
#define T_FLOATCONSTANT 410
#define T_UINTCONSTANT 411
#define T_INTCONSTANT 412
#define T_BOOLCONSTANT 413
#define T_CHARCONSTANT 414
#define T_STRING_LITERAL 415
#define T_FIELD_SELECTION 416
#define T_LSHIFT_OP 417
#define T_RSHIFT_OP 418
#define T_INC_OP 419
#define T_DEC_OP 420
#define T_LE_OP 421
#define T_GE_OP 422
#define T_EQ_OP 423
#define T_NE_OP 424
#define T_AND_OP 425
#define T_OR_OP 426
#define T_XOR_OP 427
#define T_MUL_ASSIGN 428
#define T_DIV_ASSIGN 429
#define T_ADD_ASSIGN 430
#define T_MOD_ASSIGN 431
#define T_LEFT_ASSIGN 432
#define T_RIGHT_ASSIGN 433
#define T_AND_ASSIGN 434
#define T_XOR_ASSIGN 435
#define T_OR_ASSIGN 436
#define T_SUB_ASSIGN 437
#define T_STRUCT_UNION_PTR 438
#define T_INITIALIZER_END 439
#define T_BREAK 440
#define T_CONTINUE 441
#define T_RETURN 442
#define T_GOTO 443
#define T_WHILE 444
#define T_FOR 445
#define T_DO 446
#define T_ELSE 447
#define T_IF 448
#define T_SWITCH 449
#define T_CASE 450
#define T_DEFAULT 451
#define T_CONST 452
#define T_RESTRICT 453
#define T_VOLATILE 454
#define T_STATIC 455
#define T_EXTERN 456
#define T_CONSTANT 457
#define T_GLOBAL 458
#define T_LOCAL 459
#define T_PRIVATE 460
#define T_KERNEL 461
#define T_READ_ONLY 462
#define T_WRITE_ONLY 463
#define T_PACKED 464
#define T_ALIGNED 465
#define T_ENDIAN 466
#define T_VEC_TYPE_HINT 467
#define T_ATTRIBUTE__ 468
#define T_REQD_WORK_GROUP_SIZE 469
#define T_WORK_GROUP_SIZE_HINT 470
#define T_ALWAYS_INLINE 471
#define T_UNSIGNED 472
#define T_STRUCT 473
#define T_UNION 474
#define T_TYPEDEF 475
#define T_ENUM 476
#define T_INLINE 477
#define T_SIZEOF 478
#define T_TYPE_CAST 479
#define T_VEC_STEP 480
#define T_VERY_LAST_TERMINAL 481




/* Copy the first part of user declarations.  */
#line 1 "gc_cl.y"

#include "gc_cl_parser.h"
#define FILE		void
#if !defined(UNDER_CE)
#define stderr		gcvNULL
#endif


#define YY_NO_UNISTD_H

/* define to int to avoid compilation error of
   conversion from 'int' to 'yytype_int16' */
#define YYTYPE_INT16  int

#define YYPARSE_PARAM_DECL cloCOMPILER
#define YYPARSE_PARAM Compiler

#define YYLEX_PARAM  Compiler

#define yylex cloCOMPILER_Lex
int yylex(YYSTYPE * pyylval, cloCOMPILER Compiler);




/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

#if !defined (YYSTYPE) && ! defined (YYSTYPE_IS_DECLARED)
#line 29 "gc_cl.y"
typedef union YYSTYPE {
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
} YYSTYPE;
/* Line 191 of yacc.c.  */
#line 575 "gc_cl_parser.c"
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif



/* Copy the second part of user declarations.  */


/* Line 214 of yacc.c.  */
#line 587 "gc_cl_parser.c"

#if !defined (yyoverflow) || YYERROR_VERBOSE

#ifndef YYFREE
#  define YYFREE clFree
# endif
#ifndef YYMALLOC
#  define YYMALLOC clMalloc
# endif

/* The parser invokes alloca or malloc; define the necessary symbols.  */

#ifdef YYSTACK_USE_ALLOCA
#if YYSTACK_USE_ALLOCA
#   define YYSTACK_ALLOC alloca
#  endif
# else
#if defined (alloca) || defined (_ALLOCA_H)
#   define YYSTACK_ALLOC alloca
#  else
#ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   endif
#  endif
# endif

#ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning. */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
# else
#if defined (__STDC__) || defined (__cplusplus)
#   define YYSIZE_T size_t
#  endif
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
# endif
#endif /* ! defined (yyoverflow) || YYERROR_VERBOSE */


#if (! defined (yyoverflow) \
     && (! defined (__cplusplus) \
	 || (defined (YYSTYPE_IS_TRIVIAL) && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  short yyss;
  YYSTYPE yyvs;
  };

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (short) + sizeof (YYSTYPE))				\
      + YYSTACK_GAP_MAXIMUM)

/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
#ifndef YYCOPY
#if defined (__GNUC__) && 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)		\
      do					\
	{					\
	  register YYSIZE_T yyi;		\
	  for (yyi = 0; yyi < (Count); yyi++)	\
	    (To)[yyi] = (From)[yyi];		\
	}					\
      while (0)
#  endif
# endif

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack)					\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack, Stack, yysize);				\
	Stack = &yyptr->Stack;						\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (0)

#endif

#if defined (__STDC__) || defined (__cplusplus)
   typedef signed char yysigned_char;
#else
   typedef short yysigned_char;
#endif

/* YYFINAL -- State number of the termination state. */
#define YYFINAL  102
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   3054

/* YYNTOKENS -- Number of terminals. */
#define YYNTOKENS  251
/* YYNNTS -- Number of nonterminals. */
#define YYNNTS  110
/* YYNRULES -- Number of rules. */
#define YYNRULES  310
/* YYNRULES -- Number of states. */
#define YYNSTATES  518

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   481

#define YYTRANSLATE(YYX) 						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const unsigned char yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   238,     2,     2,     2,   244,   249,     2,
     227,   228,   242,   241,   234,   239,   233,   243,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,   235,   237,
     245,   236,   246,   250,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,   229,     2,   230,   248,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   231,   247,   232,   240,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   123,   124,
     125,   126,   127,   128,   129,   130,   131,   132,   133,   134,
     135,   136,   137,   138,   139,   140,   141,   142,   143,   144,
     145,   146,   147,   148,   149,   150,   151,   152,   153,   154,
     155,   156,   157,   158,   159,   160,   161,   162,   163,   164,
     165,   166,   167,   168,   169,   170,   171,   172,   173,   174,
     175,   176,   177,   178,   179,   180,   181,   182,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,   196,   197,   198,   199,   200,   201,   202,   203,   204,
     205,   206,   207,   208,   209,   210,   211,   212,   213,   214,
     215,   216,   217,   218,   219,   220,   221,   222,   223,   224,
     225,   226
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const unsigned short yyprhs[] =
{
       0,     0,     3,     5,     7,    10,    12,    14,    16,    18,
      20,    22,    24,    28,    30,    35,    37,    41,    45,    48,
      51,    53,    56,    60,    63,    66,    70,    74,    79,    82,
      85,    87,    88,    92,    96,   100,   102,   105,   108,   113,
     116,   119,   122,   125,   127,   129,   131,   133,   135,   137,
     139,   143,   147,   151,   153,   157,   161,   163,   167,   171,
     173,   177,   181,   185,   189,   191,   195,   199,   201,   205,
     207,   211,   213,   217,   219,   223,   225,   229,   231,   235,
     237,   243,   245,   249,   251,   253,   255,   257,   259,   261,
     263,   265,   267,   269,   271,   273,   277,   279,   282,   285,
     292,   299,   305,   311,   317,   323,   328,   333,   335,   339,
     341,   345,   348,   352,   355,   358,   361,   363,   365,   368,
     372,   378,   385,   389,   394,   399,   402,   406,   408,   411,
     413,   416,   419,   423,   426,   430,   432,   434,   436,   441,
     443,   446,   448,   451,   455,   457,   460,   462,   466,   467,
     469,   473,   478,   480,   481,   485,   490,   496,   497,   505,
     506,   515,   517,   521,   526,   527,   534,   535,   543,   544,
     552,   553,   555,   556,   558,   559,   564,   569,   571,   576,
     585,   594,   596,   601,   603,   605,   608,   610,   612,   614,
     616,   618,   620,   622,   624,   626,   628,   630,   632,   634,
     636,   638,   640,   642,   644,   646,   648,   650,   652,   654,
     656,   658,   660,   662,   664,   666,   668,   670,   672,   674,
     676,   678,   680,   682,   683,   691,   692,   699,   700,   708,
     709,   716,   718,   721,   725,   727,   731,   734,   738,   740,
     742,   746,   750,   753,   758,   759,   762,   764,   765,   769,
     773,   776,   778,   780,   782,   784,   786,   788,   790,   792,
     795,   798,   801,   804,   805,   810,   812,   814,   817,   818,
     823,   825,   828,   830,   833,   839,   845,   847,   850,   852,
     856,   859,   862,   863,   868,   872,   874,   875,   882,   890,
     891,   897,   900,   903,   907,   909,   910,   914,   919,   923,
     928,   931,   934,   937,   941,   945,   947,   950,   952,   954,
     956
};

/* YYRHS -- A `-1'-separated list of the rules' RHS. */
static const short yyrhs[] =
{
     358,     0,    -1,   153,    -1,   160,    -1,   253,   160,    -1,
     252,    -1,   156,    -1,   157,    -1,   155,    -1,   158,    -1,
     159,    -1,   253,    -1,   227,   280,   228,    -1,   254,    -1,
     255,   229,   256,   230,    -1,   257,    -1,   255,   233,   161,
      -1,   255,   183,   161,    -1,   255,   164,    -1,   255,   165,
      -1,   280,    -1,   258,   228,    -1,   260,     4,   228,    -1,
     260,   228,    -1,   260,   278,    -1,   258,   234,   278,    -1,
     227,   314,   228,    -1,   227,   314,   296,   228,    -1,   153,
     227,    -1,   259,   231,    -1,   264,    -1,    -1,   259,   263,
     262,    -1,   261,   280,   232,    -1,   261,   280,   184,    -1,
     255,    -1,   164,   264,    -1,   165,   264,    -1,   225,   227,
     264,   228,    -1,   225,   259,    -1,   223,   264,    -1,   223,
     259,    -1,   265,   262,    -1,   241,    -1,   239,    -1,   238,
      -1,   240,    -1,   249,    -1,   242,    -1,   262,    -1,   266,
     242,   262,    -1,   266,   243,   262,    -1,   266,   244,   262,
      -1,   266,    -1,   267,   241,   266,    -1,   267,   239,   266,
      -1,   267,    -1,   268,   162,   267,    -1,   268,   163,   267,
      -1,   268,    -1,   269,   245,   268,    -1,   269,   246,   268,
      -1,   269,   166,   268,    -1,   269,   167,   268,    -1,   269,
      -1,   270,   168,   269,    -1,   270,   169,   269,    -1,   270,
      -1,   271,   249,   270,    -1,   271,    -1,   272,   248,   271,
      -1,   272,    -1,   273,   247,   272,    -1,   273,    -1,   274,
     170,   273,    -1,   274,    -1,   275,   172,   274,    -1,   275,
      -1,   276,   171,   275,    -1,   276,    -1,   276,   250,   280,
     235,   278,    -1,   277,    -1,   264,   279,   278,    -1,   236,
      -1,   173,    -1,   174,    -1,   176,    -1,   175,    -1,   182,
      -1,   177,    -1,   178,    -1,   179,    -1,   180,    -1,   181,
      -1,   278,    -1,   280,   234,   278,    -1,   277,    -1,   318,
     153,    -1,   221,   153,    -1,   221,   308,   153,   231,   284,
     232,    -1,   221,   308,   153,   231,   284,   184,    -1,   221,
     153,   231,   284,   232,    -1,   221,   153,   231,   284,   184,
      -1,   221,   308,   231,   284,   232,    -1,   221,   308,   231,
     284,   184,    -1,   221,   231,   284,   232,    -1,   221,   231,
     284,   184,    -1,   285,    -1,   284,   234,   285,    -1,   153,
      -1,   153,   236,   281,    -1,   301,   237,    -1,   283,   310,
     237,    -1,   282,   237,    -1,   319,   237,    -1,   288,   228,
      -1,   290,    -1,   289,    -1,   290,   292,    -1,   289,   234,
     292,    -1,   206,   310,   314,   298,   227,    -1,   201,   206,
     310,   314,   298,   227,    -1,   314,   298,   227,    -1,   308,
     314,   298,   227,    -1,   222,   314,   298,   227,    -1,   316,
     298,    -1,   316,   298,   300,    -1,   291,    -1,   293,   291,
      -1,   294,    -1,   293,   294,    -1,   295,   291,    -1,   295,
     293,   291,    -1,   295,   294,    -1,   295,   293,   294,    -1,
     207,    -1,   208,    -1,   316,    -1,   316,   229,   281,   230,
      -1,   315,    -1,   315,   295,    -1,   242,    -1,   242,   295,
      -1,   242,   295,   296,    -1,   153,    -1,   296,   153,    -1,
     297,    -1,   227,   297,   228,    -1,    -1,   281,    -1,   229,
     299,   230,    -1,   300,   229,   299,   230,    -1,   305,    -1,
      -1,   220,   302,   305,    -1,   301,   234,   298,   310,    -1,
     301,   234,   298,   300,   310,    -1,    -1,   301,   234,   298,
     310,   236,   303,   329,    -1,    -1,   301,   234,   298,   300,
     310,   236,   304,   329,    -1,   287,    -1,   314,   298,   310,
      -1,   314,   298,   300,   310,    -1,    -1,   314,   298,   310,
     236,   306,   329,    -1,    -1,   314,   298,   300,   310,   236,
     307,   329,    -1,    -1,   213,   227,   227,   309,   311,   228,
     228,    -1,    -1,   308,    -1,    -1,   313,    -1,    -1,   311,
     234,   312,   313,    -1,   211,   227,   153,   228,    -1,   209,
      -1,   212,   227,   150,   228,    -1,   214,   227,   281,   234,
     281,   234,   281,   228,    -1,   215,   227,   281,   234,   281,
     234,   281,   228,    -1,   210,    -1,   210,   227,   281,   228,
      -1,   216,    -1,   316,    -1,   295,   316,    -1,   197,    -1,
     198,    -1,   199,    -1,   202,    -1,   203,    -1,   204,    -1,
     205,    -1,   200,    -1,   201,    -1,   317,    -1,   319,    -1,
     282,    -1,   283,    -1,     4,    -1,    22,    -1,    64,    -1,
      10,    -1,   148,    -1,   149,    -1,   150,    -1,   151,    -1,
     152,    -1,    89,    -1,    90,    -1,    91,    -1,    92,    -1,
      93,    -1,    94,    -1,    88,    -1,    97,    -1,    98,    -1,
      99,    -1,    95,    -1,    96,    -1,   154,    -1,   218,    -1,
     219,    -1,    -1,   318,   153,   231,   320,   324,   232,   310,
      -1,    -1,   318,   231,   321,   324,   232,   310,    -1,    -1,
     318,   308,   153,   231,   322,   324,   232,    -1,    -1,   318,
     308,   231,   323,   324,   232,    -1,   325,    -1,   324,   325,
      -1,   314,   326,   237,    -1,   327,    -1,   326,   234,   327,
      -1,   298,   310,    -1,   298,   300,   310,    -1,   231,    -1,
     278,    -1,   328,   330,   232,    -1,   328,   330,   184,    -1,
     331,   329,    -1,   330,   234,   331,   329,    -1,    -1,   332,
     236,    -1,   334,    -1,    -1,   332,   333,   334,    -1,   229,
     281,   230,    -1,   233,   161,    -1,   286,    -1,   338,    -1,
     337,    -1,   335,    -1,   344,    -1,   345,    -1,   351,    -1,
     357,    -1,   153,   235,    -1,     1,   237,    -1,     1,   232,
      -1,   231,   232,    -1,    -1,   231,   339,   343,   232,    -1,
     341,    -1,   337,    -1,   231,   232,    -1,    -1,   231,   342,
     343,   232,    -1,   336,    -1,   343,   336,    -1,   237,    -1,
     280,   237,    -1,   193,   227,   280,   228,   350,    -1,   194,
     227,   256,   228,   348,    -1,   347,    -1,   346,   347,    -1,
     336,    -1,   195,   281,   235,    -1,   196,   235,    -1,   231,
     232,    -1,    -1,   231,   349,   346,   232,    -1,   336,   192,
     336,    -1,   336,    -1,    -1,   189,   352,   227,   280,   228,
     340,    -1,   191,   336,   189,   227,   280,   228,   237,    -1,
      -1,   190,   353,   354,   356,   340,    -1,   227,   344,    -1,
     227,   335,    -1,   227,     1,   237,    -1,   280,    -1,    -1,
     355,   237,   228,    -1,   355,   237,   280,   228,    -1,     1,
     237,   228,    -1,     1,   237,   280,   228,    -1,   186,   237,
      -1,   185,   237,    -1,   187,   237,    -1,   187,   280,   237,
      -1,   188,   153,   237,    -1,   359,    -1,   358,   359,    -1,
     360,    -1,   286,    -1,   237,    -1,   287,   341,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const unsigned short yyrline[] =
{
       0,   193,   193,   202,   204,   209,   211,   213,   215,   217,
     219,   221,   223,   228,   230,   232,   234,   236,   238,   240,
     245,   250,   252,   254,   259,   261,   266,   268,   273,   278,
     287,   290,   289,   298,   302,   309,   311,   313,   315,   317,
     319,   321,   323,   328,   330,   332,   334,   336,   338,   343,
     347,   349,   351,   356,   358,   360,   365,   367,   369,   374,
     376,   378,   380,   382,   387,   389,   391,   396,   398,   403,
     405,   410,   412,   417,   419,   424,   426,   431,   433,   438,
     440,   445,   447,   452,   454,   456,   458,   460,   462,   464,
     466,   468,   470,   472,   477,   479,   492,   497,   499,   504,
     508,   511,   515,   518,   522,   525,   529,   535,   542,   547,
     549,   554,   556,   558,   560,   565,   570,   572,   577,   579,
     584,   586,   588,   590,   592,   599,   601,   606,   608,   610,
     612,   614,   616,   618,   620,   625,   627,   632,   634,   639,
     644,   649,   651,   653,   657,   659,   666,   668,   673,   674,
     679,   681,   686,   689,   688,   697,   699,   702,   701,   706,
     705,   713,   715,   717,   720,   719,   724,   723,   733,   732,
     739,   740,   745,   746,   749,   748,   755,   757,   759,   761,
     763,   765,   767,   769,   774,   776,   781,   783,   785,   787,
     789,   791,   793,   795,   797,   802,   804,   806,   808,   813,
     815,   817,   819,   821,   823,   825,   827,   829,   831,   833,
     835,   837,   839,   841,   843,   845,   847,   849,   851,   853,
     855,   859,   861,   867,   866,   871,   870,   875,   874,   879,
     878,   885,   889,   902,   909,   911,   916,   918,   923,   945,
     947,   949,   954,   956,   961,   962,   975,   978,   977,   986,
     999,  1015,  1020,  1022,  1027,  1029,  1031,  1033,  1035,  1037,
    1039,  1043,  1050,  1053,  1052,  1059,  1061,  1066,  1069,  1068,
    1075,  1077,  1082,  1084,  1089,  1091,  1096,  1098,  1103,  1105,
    1107,  1112,  1115,  1114,  1121,  1123,  1131,  1130,  1134,  1137,
    1136,  1143,  1145,  1147,  1154,  1157,  1161,  1163,  1165,  1171,
    1180,  1182,  1184,  1186,  1188,  1195,  1196,  1200,  1201,  1203,
    1207
};
#endif

#if YYDEBUG || YYERROR_VERBOSE
/* YYTNME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals. */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "T_VERY_FIRST_TERMINAL", "T_VOID",
  "T_MAT2", "T_MAT3", "T_MAT4", "T_MAT8", "T_MAT16", "T_BOOL", "T_BOOL2",
  "T_BOOL3", "T_BOOL4", "T_BOOL8", "T_BOOL16", "T_HALF", "T_HALF2",
  "T_HALF3", "T_HALF4", "T_HALF8", "T_HALF16", "T_FLOAT", "T_FLOAT2",
  "T_FLOAT3", "T_FLOAT4", "T_FLOAT8", "T_FLOAT16", "T_DOUBLE", "T_DOUBLE2",
  "T_DOUBLE3", "T_DOUBLE4", "T_DOUBLE8", "T_DOUBLE16", "T_QUAD", "T_QUAD2",
  "T_QUAD3", "T_QUAD4", "T_QUAD8", "T_QUAD16", "T_CHAR", "T_CHAR2",
  "T_CHAR3", "T_CHAR4", "T_CHAR8", "T_CHAR16", "T_UCHAR", "T_UCHAR2",
  "T_UCHAR3", "T_UCHAR4", "T_UCHAR8", "T_UCHAR16", "T_SHORT", "T_SHORT2",
  "T_SHORT3", "T_SHORT4", "T_SHORT8", "T_SHORT16", "T_USHORT", "T_USHORT2",
  "T_USHORT3", "T_USHORT4", "T_USHORT8", "T_USHORT16", "T_INT", "T_INT2",
  "T_INT3", "T_INT4", "T_INT8", "T_INT16", "T_UINT", "T_UINT2", "T_UINT3",
  "T_UINT4", "T_UINT8", "T_UINT16", "T_LONG", "T_LONG2", "T_LONG3",
  "T_LONG4", "T_LONG8", "T_LONG16", "T_ULONG", "T_ULONG2", "T_ULONG3",
  "T_ULONG4", "T_ULONG8", "T_ULONG16", "T_SAMPLER_T", "T_IMAGE1D_T",
  "T_IMAGE1D_ARRAY_T", "T_IMAGE1D_BUFFER_T", "T_IMAGE2D_ARRAY_T",
  "T_IMAGE2D_T", "T_IMAGE3D_T", "T_SIZE_T", "T_EVENT_T", "T_PTRDIFF_T",
  "T_INTPTR_T", "T_UINTPTR_T", "T_GENTYPE", "T_F_GENTYPE", "T_IU_GENTYPE",
  "T_I_GENTYPE", "T_U_GENTYPE", "T_SIU_GENTYPE", "T_BOOL_PACKED",
  "T_BOOL2_PACKED", "T_BOOL3_PACKED", "T_BOOL4_PACKED", "T_BOOL8_PACKED",
  "T_BOOL16_PACKED", "T_BOOL32_PACKED", "T_CHAR_PACKED", "T_CHAR2_PACKED",
  "T_CHAR3_PACKED", "T_CHAR4_PACKED", "T_CHAR8_PACKED", "T_CHAR16_PACKED",
  "T_CHAR32_PACKED", "T_UCHAR_PACKED", "T_UCHAR2_PACKED",
  "T_UCHAR3_PACKED", "T_UCHAR4_PACKED", "T_UCHAR8_PACKED",
  "T_UCHAR16_PACKED", "T_UCHAR32_PACKED", "T_SHORT_PACKED",
  "T_SHORT2_PACKED", "T_SHORT3_PACKED", "T_SHORT4_PACKED",
  "T_SHORT8_PACKED", "T_SHORT16_PACKED", "T_SHORT32_PACKED",
  "T_USHORT_PACKED", "T_USHORT2_PACKED", "T_USHORT3_PACKED",
  "T_USHORT4_PACKED", "T_USHORT8_PACKED", "T_USHORT16_PACKED",
  "T_USHORT32_PACKED", "T_HALF_PACKED", "T_HALF2_PACKED", "T_HALF3_PACKED",
  "T_HALF4_PACKED", "T_HALF8_PACKED", "T_HALF16_PACKED", "T_HALF32_PACKED",
  "T_FLOATNXM", "T_DOUBLENXM", "T_BUILTIN_DATA_TYPE",
  "T_RESERVED_DATA_TYPE", "T_VIV_PACKED_DATA_TYPE", "T_IDENTIFIER",
  "T_TYPE_NAME", "T_FLOATCONSTANT", "T_UINTCONSTANT", "T_INTCONSTANT",
  "T_BOOLCONSTANT", "T_CHARCONSTANT", "T_STRING_LITERAL",
  "T_FIELD_SELECTION", "T_LSHIFT_OP", "T_RSHIFT_OP", "T_INC_OP",
  "T_DEC_OP", "T_LE_OP", "T_GE_OP", "T_EQ_OP", "T_NE_OP", "T_AND_OP",
  "T_OR_OP", "T_XOR_OP", "T_MUL_ASSIGN", "T_DIV_ASSIGN", "T_ADD_ASSIGN",
  "T_MOD_ASSIGN", "T_LEFT_ASSIGN", "T_RIGHT_ASSIGN", "T_AND_ASSIGN",
  "T_XOR_ASSIGN", "T_OR_ASSIGN", "T_SUB_ASSIGN", "T_STRUCT_UNION_PTR",
  "T_INITIALIZER_END", "T_BREAK", "T_CONTINUE", "T_RETURN", "T_GOTO",
  "T_WHILE", "T_FOR", "T_DO", "T_ELSE", "T_IF", "T_SWITCH", "T_CASE",
  "T_DEFAULT", "T_CONST", "T_RESTRICT", "T_VOLATILE", "T_STATIC",
  "T_EXTERN", "T_CONSTANT", "T_GLOBAL", "T_LOCAL", "T_PRIVATE", "T_KERNEL",
  "T_READ_ONLY", "T_WRITE_ONLY", "T_PACKED", "T_ALIGNED", "T_ENDIAN",
  "T_VEC_TYPE_HINT", "T_ATTRIBUTE__", "T_REQD_WORK_GROUP_SIZE",
  "T_WORK_GROUP_SIZE_HINT", "T_ALWAYS_INLINE", "T_UNSIGNED", "T_STRUCT",
  "T_UNION", "T_TYPEDEF", "T_ENUM", "T_INLINE", "T_SIZEOF", "T_TYPE_CAST",
  "T_VEC_STEP", "T_VERY_LAST_TERMINAL", "'('", "')'", "'['", "']'", "'{'",
  "'}'", "'.'", "','", "':'", "'='", "';'", "'!'", "'-'", "'~'", "'+'",
  "'*'", "'/'", "'%'", "'<'", "'>'", "'|'", "'^'", "'&'", "'?'", "$accept",
  "variable_identifier", "string_literal", "primary_expression",
  "postfix_expression", "integer_expression", "function_call",
  "function_call_with_parameters", "type_cast", "function_call_header",
  "curly_bracket_type_cast", "cast_expression", "@1", "unary_expression",
  "unary_operator", "multiplicative_expression", "additive_expression",
  "shift_expression", "relational_expression", "equality_expression",
  "and_expression", "exclusive_or_expression", "inclusive_or_expression",
  "logical_and_expression", "logical_xor_expression",
  "logical_or_expression", "conditional_expression",
  "assignment_expression", "assignment_operator", "expression",
  "constant_expression", "tags", "enum_specifier", "enumerator_list",
  "enumerator", "declaration", "function_prototype", "function_declarator",
  "function_header_with_parameters", "function_header",
  "parameter_declarator", "parameter_declaration", "parameter_qualifier",
  "parameter_type_specifier", "type_qualifier_list", "pointer",
  "declarator", "direct_declarator", "array_size", "array_declarator",
  "init_declarator_list", "@2", "@3", "@4", "single_declaration", "@5",
  "@6", "attribute_specifier", "@7", "attribute_specifier_opt",
  "attribute_list", "@8", "attribute", "fully_specified_type",
  "type_qualifier", "type_specifier", "type_name", "struct_or_union",
  "struct_union_specifier", "@9", "@10", "@11", "@12",
  "struct_declaration_list", "struct_declaration",
  "struct_declarator_list", "struct_declarator", "initializer_list_start",
  "initializer", "initializer_list", "designation", "designator_list",
  "@13", "designator", "declaration_statement", "statement",
  "simple_statement", "compound_statement", "@14",
  "statement_no_new_scope", "compound_statement_no_new_scope", "@15",
  "statement_list", "expression_statement", "selection_statement",
  "switch_body_statement_list", "switch_body_statement", "switch_body",
  "@16", "if_sub_statement", "iteration_statement", "@17", "@18",
  "for_init_statement", "conditionopt", "for_control", "jump_statement",
  "translation_unit", "external_declaration", "function_definition", 0
};
#endif

#ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const unsigned short yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,   316,   317,   318,   319,   320,   321,   322,   323,   324,
     325,   326,   327,   328,   329,   330,   331,   332,   333,   334,
     335,   336,   337,   338,   339,   340,   341,   342,   343,   344,
     345,   346,   347,   348,   349,   350,   351,   352,   353,   354,
     355,   356,   357,   358,   359,   360,   361,   362,   363,   364,
     365,   366,   367,   368,   369,   370,   371,   372,   373,   374,
     375,   376,   377,   378,   379,   380,   381,   382,   383,   384,
     385,   386,   387,   388,   389,   390,   391,   392,   393,   394,
     395,   396,   397,   398,   399,   400,   401,   402,   403,   404,
     405,   406,   407,   408,   409,   410,   411,   412,   413,   414,
     415,   416,   417,   418,   419,   420,   421,   422,   423,   424,
     425,   426,   427,   428,   429,   430,   431,   432,   433,   434,
     435,   436,   437,   438,   439,   440,   441,   442,   443,   444,
     445,   446,   447,   448,   449,   450,   451,   452,   453,   454,
     455,   456,   457,   458,   459,   460,   461,   462,   463,   464,
     465,   466,   467,   468,   469,   470,   471,   472,   473,   474,
     475,   476,   477,   478,   479,   480,   481,    40,    41,    91,
      93,   123,   125,    46,    44,    58,    61,    59,    33,    45,
     126,    43,    42,    47,    37,    60,    62,   124,    94,    38,
      63
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const unsigned short yyr1[] =
{
       0,   251,   252,   253,   253,   254,   254,   254,   254,   254,
     254,   254,   254,   255,   255,   255,   255,   255,   255,   255,
     256,   257,   257,   257,   258,   258,   259,   259,   260,   261,
     262,   263,   262,   262,   262,   264,   264,   264,   264,   264,
     264,   264,   264,   265,   265,   265,   265,   265,   265,   266,
     266,   266,   266,   267,   267,   267,   268,   268,   268,   269,
     269,   269,   269,   269,   270,   270,   270,   271,   271,   272,
     272,   273,   273,   274,   274,   275,   275,   276,   276,   277,
     277,   278,   278,   279,   279,   279,   279,   279,   279,   279,
     279,   279,   279,   279,   280,   280,   281,   282,   282,   283,
     283,   283,   283,   283,   283,   283,   283,   284,   284,   285,
     285,   286,   286,   286,   286,   287,   288,   288,   289,   289,
     290,   290,   290,   290,   290,   291,   291,   292,   292,   292,
     292,   292,   292,   292,   292,   293,   293,   294,   294,   295,
     295,   296,   296,   296,   297,   297,   298,   298,   299,   299,
     300,   300,   301,   302,   301,   301,   301,   303,   301,   304,
     301,   305,   305,   305,   306,   305,   307,   305,   309,   308,
     310,   310,   311,   311,   312,   311,   313,   313,   313,   313,
     313,   313,   313,   313,   314,   314,   315,   315,   315,   315,
     315,   315,   315,   315,   315,   316,   316,   316,   316,   317,
     317,   317,   317,   317,   317,   317,   317,   317,   317,   317,
     317,   317,   317,   317,   317,   317,   317,   317,   317,   317,
     317,   318,   318,   320,   319,   321,   319,   322,   319,   323,
     319,   324,   324,   325,   326,   326,   327,   327,   328,   329,
     329,   329,   330,   330,   331,   331,   332,   333,   332,   334,
     334,   335,   336,   336,   337,   337,   337,   337,   337,   337,
     337,   337,   338,   339,   338,   340,   340,   341,   342,   341,
     343,   343,   344,   344,   345,   345,   346,   346,   347,   347,
     347,   348,   349,   348,   350,   350,   352,   351,   351,   353,
     351,   354,   354,   354,   355,   355,   356,   356,   356,   356,
     357,   357,   357,   357,   357,   358,   358,   359,   359,   359,
     360
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const unsigned char yyr2[] =
{
       0,     2,     1,     1,     2,     1,     1,     1,     1,     1,
       1,     1,     3,     1,     4,     1,     3,     3,     2,     2,
       1,     2,     3,     2,     2,     3,     3,     4,     2,     2,
       1,     0,     3,     3,     3,     1,     2,     2,     4,     2,
       2,     2,     2,     1,     1,     1,     1,     1,     1,     1,
       3,     3,     3,     1,     3,     3,     1,     3,     3,     1,
       3,     3,     3,     3,     1,     3,     3,     1,     3,     1,
       3,     1,     3,     1,     3,     1,     3,     1,     3,     1,
       5,     1,     3,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     3,     1,     2,     2,     6,
       6,     5,     5,     5,     5,     4,     4,     1,     3,     1,
       3,     2,     3,     2,     2,     2,     1,     1,     2,     3,
       5,     6,     3,     4,     4,     2,     3,     1,     2,     1,
       2,     2,     3,     2,     3,     1,     1,     1,     4,     1,
       2,     1,     2,     3,     1,     2,     1,     3,     0,     1,
       3,     4,     1,     0,     3,     4,     5,     0,     7,     0,
       8,     1,     3,     4,     0,     6,     0,     7,     0,     7,
       0,     1,     0,     1,     0,     4,     4,     1,     4,     8,
       8,     1,     4,     1,     1,     2,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     0,     7,     0,     6,     0,     7,     0,
       6,     1,     2,     3,     1,     3,     2,     3,     1,     1,
       3,     3,     2,     4,     0,     2,     1,     0,     3,     3,
       2,     1,     1,     1,     1,     1,     1,     1,     1,     2,
       2,     2,     2,     0,     4,     1,     1,     2,     0,     4,
       1,     2,     1,     2,     5,     5,     1,     2,     1,     3,
       2,     2,     0,     4,     3,     1,     0,     6,     7,     0,
       5,     2,     2,     3,     1,     0,     3,     4,     3,     4,
       2,     2,     2,     3,     3,     1,     2,     1,     1,     1,
       2
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const unsigned short yydefact[] =
{
       0,   199,   202,   200,   201,   214,   208,   209,   210,   211,
     212,   213,   218,   219,   215,   216,   217,   203,   204,   205,
     206,   207,   220,   186,   187,   188,   193,   194,   189,   190,
     191,   192,   170,     0,   221,   222,   153,     0,     0,   309,
     197,   198,   308,   161,     0,   117,   116,     0,     0,   152,
       0,     0,   139,   184,   195,     0,   196,     0,   305,   307,
     170,   171,     0,     0,     0,    98,     0,     0,   194,   197,
     198,     0,   196,   113,     0,   268,   310,   115,     0,   135,
     136,   127,   118,     0,   129,     0,   137,   185,     0,   111,
       0,   144,     0,   141,     0,   146,   170,   140,    97,   225,
       0,   114,     1,   306,     0,     0,   168,   161,   154,     0,
     109,     0,   107,     0,     0,     0,   112,   267,     0,   119,
     128,   130,   131,     0,   133,     0,   125,   170,     0,     0,
     142,   145,   122,   148,   170,   162,   223,     0,     0,   229,
       0,     0,   172,     0,     0,   106,   105,     0,     0,     0,
     124,     0,     2,     8,     6,     7,     9,    10,     3,     0,
       0,     0,     0,     0,     0,   286,   289,     0,     0,     0,
       0,     0,     0,   263,   272,    45,    44,    46,    43,    48,
      47,     5,    11,    13,    35,    15,     0,    31,     0,     0,
      49,    30,     0,    53,    56,    59,    64,    67,    69,    71,
      73,    75,    77,    79,    81,    94,     0,   251,   254,   270,
     253,   252,     0,   255,   256,   257,   258,   132,   134,     2,
      30,    96,     0,   126,   170,   155,   123,   147,   143,   149,
       0,   148,   163,   164,     0,     0,     0,   231,   227,     0,
       0,   120,   177,   181,     0,     0,     0,     0,   183,     0,
     173,   102,   101,   110,   108,     0,   104,   103,   261,   260,
      28,   259,     0,    36,    37,   301,   300,   302,     0,     0,
       0,     0,     0,     0,     0,    41,    40,     0,    39,     0,
       0,   262,     0,     4,    18,    19,     0,     0,     0,    21,
       0,    29,     0,     0,    23,    24,     0,    84,    85,    87,
      86,    89,    90,    91,    92,    93,    88,    83,     0,    42,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   273,   269,   271,   138,   156,   157,   150,     0,   166,
       0,     0,   170,     0,   234,   170,   232,     0,     0,   121,
       0,     0,     0,     0,     0,     0,   174,   100,    99,   303,
     304,     0,     0,     0,     0,     0,     0,    20,     0,    12,
      26,     0,     0,    17,     0,    16,    25,    32,    22,    34,
      33,    82,    50,    51,    52,    55,    54,    57,    58,    62,
      63,    60,    61,    65,    66,    68,    70,    72,    74,    76,
      78,     0,    95,   159,     0,   151,     0,   238,   239,   244,
     165,   170,   170,   236,     0,   233,   226,     0,   230,     0,
       0,     0,     0,     0,   169,     0,     0,     0,   292,   291,
       0,   294,     0,     0,     0,     0,     0,    38,    27,   264,
      14,     0,     0,   158,   167,     0,     0,     0,     0,   247,
     246,   224,   237,   235,   228,   182,   176,   178,     0,     0,
     175,     0,   293,     0,     0,   266,   290,   265,     0,   285,
     274,   282,   275,    80,   160,     0,   250,   241,   240,   244,
     242,   245,     0,     0,     0,   287,   298,     0,   296,     0,
       0,     0,   281,     0,   249,     0,   248,     0,     0,   299,
     297,   288,   284,     0,     0,   278,     0,   276,   243,     0,
       0,     0,   280,   283,   277,   179,   180,   279
};

/* YYDEFGOTO[NTERM-NUM]. */
static const short yydefgoto[] =
{
      -1,   181,   182,   183,   184,   366,   185,   186,   187,   188,
     189,   190,   292,   191,   192,   193,   194,   195,   196,   197,
     198,   199,   200,   201,   202,   203,   204,   205,   308,   206,
     229,    69,    70,   111,   112,   207,   107,    44,    45,    46,
      81,    82,    83,    84,    47,    94,    95,   342,   230,   134,
      48,    64,   404,   442,    49,   340,   406,    50,   142,    62,
     249,   425,   250,    51,    52,    53,    54,    55,    72,   234,
     137,   347,   239,   236,   237,   343,   344,   409,   410,   447,
     448,   449,   482,   450,   208,   209,   210,   211,   282,   466,
     467,   118,   212,   213,   214,   506,   507,   472,   493,   470,
     215,   270,   271,   363,   432,   433,   216,    57,    58,    59
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -413
static const short yypact[] =
{
    1899,  -413,  -413,  -413,  -413,  -413,  -413,  -413,  -413,  -413,
    -413,  -413,  -413,  -413,  -413,  -413,  -413,  -413,  -413,  -413,
    -413,  -413,  -413,  -413,  -413,  -413,  -413,  -142,  -413,  -413,
    -413,  -413,  -112,   -78,  -413,  -413,  -413,  -106,  2665,  -413,
     -79,  -174,  -413,  -104,   -53,   -54,  2516,  2783,  -163,  -413,
    2665,  -137,   250,  -413,  -413,  -102,   -38,  1803,  -413,  -413,
    -112,  -413,  2665,    14,  2480,   -45,    36,  -108,  -413,  -413,
    -413,  -137,  -413,  -413,    25,    49,  -413,  -413,  2516,  -413,
    -413,  -413,  -413,  2783,  -413,  2684,  -144,  -413,  -137,  -413,
    -137,  -413,  -125,   250,    92,  -413,   -48,  -413,    37,  -413,
    -105,  -413,  -413,  -413,  2665,  -137,  -413,  -413,  -413,    36,
      78,   -84,  -413,    53,    36,    69,  -413,  -413,  1030,  -413,
    -413,  -413,  -413,  2783,  -413,  2215,    87,  -173,    90,    94,
      81,  -413,  -413,  2215,  -161,    89,  -413,  2665,    95,  -413,
    -137,   100,    60,   -44,  2215,  -413,  -413,    36,    36,   -37,
    -413,  -177,  -143,  -413,  -413,  -413,  -413,  -413,  -413,  2400,
    2400,    93,    96,  1470,   175,  -413,  -413,  1030,   105,   107,
    2215,   108,  1523,   104,  -413,  -413,  -413,  -413,  -413,  -413,
    -413,  -413,   177,  -413,   -87,  -413,  -153,   109,   262,  2215,
    -413,   129,  2215,  -156,    46,   -49,  -152,    35,    97,   101,
      98,   169,   170,  -151,  -413,  -413,   -36,  -413,  -413,  -413,
    -413,  -413,   706,  -413,  -413,  -413,  -413,  -413,  -413,   114,
    -413,  -413,   113,   121,  -161,   122,  -413,  -413,  -413,  -413,
     126,  2215,   123,  -413,  2665,  -137,  2050,  -413,  -413,  2665,
     130,  -413,  -413,   137,   140,   143,   144,   147,  -413,  -100,
    -413,  -413,  -413,  -413,  -413,   -25,  -413,  -413,  -413,  -413,
    -413,  -413,  2215,  -413,  -413,  -413,  -413,  -413,   -19,   138,
     150,   151,   190,  2215,  2215,  -413,  -413,  1641,  -413,   -89,
    -209,  -413,  1030,  -413,  -413,  -413,   219,  2215,   223,  -413,
    2215,  -413,  2215,   157,  -413,  -413,     8,  -413,  -413,  -413,
    -413,  -413,  -413,  -413,  -413,  -413,  -413,  -413,  2215,  -413,
    2215,  2215,  2215,  2215,  2215,  2215,  2215,  2215,  2215,  2215,
    2215,  2215,  2215,  2215,  2215,  2215,  2215,  2215,  2215,  2215,
    2215,  -413,  -413,  -413,  -413,   158,  -413,  -413,   163,  -413,
    2751,  2146,  -173,    24,  -413,  -112,  -413,  2665,  2295,  -413,
    2215,   242,   247,  2215,  2215,   173,  -413,  -413,  -413,  -413,
    -413,  2215,  1354,  1301,   171,   -74,   174,   172,   181,  -413,
    -413,   182,   868,  -413,   184,  -413,  -413,  -413,  -413,  -413,
    -413,  -413,  -413,  -413,  -413,  -156,  -156,    46,    46,   -49,
     -49,   -49,   -49,  -152,  -152,    35,    97,   101,    98,   169,
     170,    54,  -413,  -413,  2751,  -413,  2751,  -413,  -413,   -51,
    -413,  -112,  -161,  -413,  -137,  -413,  -413,  2331,  -413,   183,
     188,   195,   178,   191,  -413,    60,   -62,   166,  -413,  -413,
     187,   172,   192,  1192,  2215,  1030,   197,  -413,  -413,  -413,
    -413,  2215,  2751,  -413,  -413,  2215,   269,    33,  2751,   196,
    -413,  -413,  -413,  -413,  -413,  -413,  -413,  -413,  2215,  2215,
    -413,  1192,  -413,  2785,  2805,  -413,  -413,  -413,   -43,   239,
    -413,   206,  -413,  -413,  -413,   210,  -413,  -413,  -413,   -51,
    -413,  -413,   -51,   207,   208,  -413,  -413,    16,  -413,    20,
     220,  1030,  -413,   544,  -413,  2751,  -413,  2215,  2215,  -413,
    -413,  -413,  -413,  2215,   209,  -413,   382,  -413,  -413,   215,
     217,   221,  -413,  -413,  -413,  -413,  -413,  -413
};

/* YYPGOTO[NTERM-NUM].  */
static const short yypgoto[] =
{
    -413,  -413,  -413,  -413,  -413,   176,  -413,  -413,   120,  -413,
    -413,  -180,  -413,   -90,  -413,   -21,   -18,   -40,   -22,   135,
     136,   134,   141,   139,   154,  -413,  -107,  -147,  -413,  -110,
     -98,     0,     1,    48,   317,    32,    38,  -413,  -413,  -413,
     -41,   390,   384,    50,    13,  -124,   391,    65,   253,  -103,
    -413,  -413,  -413,  -413,   422,  -413,  -413,   -30,  -413,   -24,
    -413,  -413,    63,   -28,  -413,    91,  -413,  -413,     4,  -413,
    -413,  -413,  -413,  -226,  -233,  -413,    82,  -413,  -375,  -413,
      19,  -413,  -413,    17,   146,  -162,  -412,  -413,  -413,    44,
     466,  -413,   230,   152,  -413,  -413,     7,  -413,  -413,  -413,
    -413,  -413,  -413,  -413,  -413,  -413,  -413,  -413,   458,  -413
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -296
static const short yytable[] =
{
      40,    41,    61,   346,    56,   272,   228,    67,   341,    91,
      71,    61,   309,   348,   317,   318,    91,    74,   221,   370,
     328,   465,    90,   223,   224,   100,   221,   222,    91,   443,
      61,   444,    42,    93,   105,   220,   104,   221,    43,    33,
      33,   295,   120,   220,   122,   113,   253,    65,   138,   465,
     333,    98,    33,   268,   220,   258,   133,    40,    41,    85,
     259,    56,   279,  -170,    60,    97,    61,   474,   231,   263,
     264,    88,   135,   480,    89,   289,   140,   284,   285,   296,
     276,   290,   217,    92,   260,   125,   310,   311,   312,    42,
      92,    85,   261,   319,   320,    43,   286,    61,    93,   329,
     145,    33,   220,   225,    61,    93,   130,    33,   346,   235,
     232,    33,   377,   315,   316,   346,    96,    93,    40,    41,
     508,   417,    56,   114,   221,    66,   139,    75,   355,    99,
     382,   383,   384,   121,   356,   124,   115,    86,    87,   369,
     251,   220,   287,   376,   280,   330,   288,   256,   146,    63,
     147,   126,   279,   127,   435,   128,   371,   143,    73,   357,
     330,   381,   149,   365,   367,    33,   461,    40,    41,    86,
     141,    56,   330,   218,    86,    77,    86,   367,   445,   132,
      78,   133,   446,   402,   346,   490,   109,   368,   252,   110,
     147,   330,   379,   408,    61,   257,   255,   147,   330,   101,
     335,   331,   220,   321,   322,   240,   235,   358,   235,   147,
     333,   235,    40,    41,    86,   330,    56,   477,   359,   401,
     220,   220,   220,   220,   220,   220,   220,   220,   220,   220,
     220,   220,   220,   220,   220,   220,   220,   220,   220,   412,
     380,   106,   330,   221,   499,   131,   221,   221,   500,   280,
     330,   426,   419,   431,   330,   422,   423,   408,   414,   408,
     220,   415,   116,   220,   220,   478,   293,   479,   136,   242,
     243,   244,   245,   469,   246,   247,   248,   389,   390,   391,
     392,   117,    40,    41,   148,   313,    56,   314,   330,   441,
     275,   278,   385,   386,   473,   408,   150,   387,   388,   393,
     394,   408,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,    61,   235,   144,    61,   133,   226,   413,   235,
     235,   416,   227,    93,   468,   233,   238,   241,   269,   502,
     265,   505,   273,   266,   274,   277,   281,   283,   221,   326,
     291,   260,   327,   334,   505,   325,   323,   475,   408,   324,
     231,   221,   221,   487,   489,   220,   337,   349,   336,   339,
     483,   484,    40,    41,   350,   307,    56,   351,   220,   220,
     352,   353,    40,    41,   354,   360,    56,   361,   362,   364,
     373,    61,    61,   151,   375,   378,     1,   451,   452,   235,
     221,   221,     2,   405,   403,   420,   221,   421,   434,   509,
     510,   424,   436,   462,     3,   511,   330,   220,   220,   437,
     438,   455,   458,   220,   440,   219,   456,   153,   154,   155,
     156,   157,   158,   457,   463,   459,   159,   160,   471,   464,
     476,   491,   481,    40,    41,    40,    41,    56,   492,    56,
     494,   497,   498,   515,   512,   516,     4,    23,    24,    25,
      26,    68,    28,    29,    30,    31,   517,   501,   395,   397,
     396,    40,    41,   374,   254,    56,   399,   398,   119,   123,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,   400,   129,   338,   170,   108,   171,   460,   172,
     294,    40,    41,    40,    41,    56,   453,    56,   495,   496,
     175,   176,   177,   178,   179,   485,    40,    41,   428,    76,
      56,   180,   372,   514,   429,   103,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      17,    18,    19,    20,    21,   152,    22,   153,   154,   155,
     156,   157,   158,     0,     0,   151,   159,   160,     1,     0,
       0,     0,     0,     0,     2,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     3,   161,   162,   163,
     164,   165,   166,   167,     0,   168,   169,   503,   504,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,     0,
       0,     0,     0,     0,     0,    33,     0,     0,     0,     0,
      34,    35,    36,    37,    38,   170,     0,   171,     4,   172,
       0,     0,     0,   173,   513,     0,     0,     0,     0,   174,
     175,   176,   177,   178,   179,     0,     0,     0,     0,     0,
       0,   180,     5,     6,     7,     8,     9,    10,    11,    12,
      13,    14,    15,    16,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    17,    18,    19,    20,    21,   152,    22,   153,
     154,   155,   156,   157,   158,     0,     0,   151,   159,   160,
       1,     0,     0,     0,     0,     0,     2,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     3,   161,
     162,   163,   164,   165,   166,   167,     0,   168,   169,   503,
     504,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,     0,     0,     0,     0,     0,     0,    33,     0,     0,
       0,     0,    34,    35,    36,    37,    38,   170,     0,   171,
       4,   172,     0,     0,     0,   173,     0,     0,     0,     0,
       0,   174,   175,   176,   177,   178,   179,     0,     0,     0,
       0,     0,     0,   180,     5,     6,     7,     8,     9,    10,
      11,    12,    13,    14,    15,    16,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    17,    18,    19,    20,    21,   152,
      22,   153,   154,   155,   156,   157,   158,     0,     0,   151,
     159,   160,     1,     0,     0,     0,     0,     0,     2,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       3,   161,   162,   163,   164,   165,   166,   167,     0,   168,
     169,     0,     0,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,     0,     0,     0,     0,     0,     0,    33,
       0,     0,     0,     0,    34,    35,    36,    37,    38,   170,
       0,   171,     4,   172,     0,     0,     0,   173,   332,     0,
       0,     0,     0,   174,   175,   176,   177,   178,   179,     0,
       0,     0,     0,     0,     0,   180,     5,     6,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    17,    18,    19,    20,
      21,   152,    22,   153,   154,   155,   156,   157,   158,     0,
       0,   151,   159,   160,     1,     0,     0,     0,     0,     0,
       2,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     3,   161,   162,   163,   164,   165,   166,   167,
       0,   168,   169,     0,     0,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,     0,     0,     0,     0,     0,
       0,    33,     0,     0,     0,     0,    34,    35,    36,    37,
      38,   170,     0,   171,     4,   172,     0,     0,     0,   173,
     439,     0,     0,     0,     0,   174,   175,   176,   177,   178,
     179,     0,     0,     0,     0,     0,     0,   180,     5,     6,
       7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    17,    18,
      19,    20,    21,   152,    22,   153,   154,   155,   156,   157,
     158,     0,     0,   151,   159,   160,     1,     0,     0,     0,
       0,     0,     2,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     3,   161,   162,   163,   164,   165,
     166,   167,     0,   168,   169,     0,     0,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,     0,     0,     0,
       0,     0,     0,    33,     0,     0,     0,     0,    34,    35,
      36,    37,    38,   170,     0,   171,     4,   172,     0,     0,
       0,   173,     0,     0,     0,     0,     0,   174,   175,   176,
     177,   178,   179,     0,     0,     0,     0,     0,     0,   180,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   430,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      17,    18,    19,    20,    21,   152,    22,   153,   154,   155,
     156,   157,   158,     0,     0,   427,   159,   160,     1,     0,
       0,     0,     0,     0,     2,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     3,   161,   162,   163,
     164,   165,   166,   167,     0,   168,   169,     0,     0,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,     0,
       0,     0,     0,     0,     0,    33,     0,     0,     0,     0,
      34,    35,    36,    37,    38,   170,     0,   171,     4,   172,
       0,     0,     0,    75,     0,     0,     0,     0,     0,   174,
     175,   176,   177,   178,   179,     0,     0,     0,     0,     0,
       0,   180,     5,     6,     7,     8,     9,    10,    11,    12,
      13,    14,    15,    16,   219,     0,   153,   154,   155,   156,
     157,   158,     0,     0,     0,   159,   160,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    17,    18,    19,    20,    21,   219,    22,   153,
     154,   155,   156,   157,   158,     0,     0,     0,   159,   160,
       0,     0,     0,     0,   170,     0,   171,     1,   172,     0,
       0,     0,     0,     2,     0,     0,     0,     0,  -295,   175,
     176,   177,   178,   179,     0,     3,     0,     0,     0,     0,
     180,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,     0,     0,     0,     0,     0,     0,    33,     0,     0,
       0,     0,    34,    35,    36,    37,    38,   170,     0,   171,
       0,   172,     0,     0,     0,     0,     0,     4,     0,     0,
       0,   174,   175,   176,   177,   178,   179,     0,     0,     0,
       0,     0,     0,   180,     0,     0,     0,     0,     0,     0,
       0,     5,     6,     7,     8,     9,    10,    11,    12,    13,
      14,    15,    16,   219,     0,   153,   154,   155,   156,   157,
     158,     0,     0,     0,   159,   160,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     1,     0,     0,     0,     0,
       0,     2,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     3,     0,     0,     0,     0,     0,     0,
       0,    17,    18,    19,    20,    21,   219,    22,   153,   154,
     155,   156,   157,   158,     0,     0,     0,   159,   160,     0,
       0,     0,     0,   170,     0,   171,     0,   172,     0,     0,
       0,     0,     0,     0,     0,     4,     0,   267,   175,   176,
     177,   178,   179,     0,     0,     0,     0,     0,     0,   180,
      23,    24,    25,    26,    68,    28,    29,    30,    31,     5,
       6,     7,     8,     9,    10,    11,    12,    13,    14,    15,
      16,    34,    35,     0,    37,     0,   170,     0,   171,     0,
     172,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   175,   176,   177,   178,   179,     0,     0,     0,     0,
       0,     0,   180,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    17,
      18,    19,    20,    21,   219,    22,   153,   154,   155,   156,
     157,   158,     0,   102,     0,   159,   160,     1,     0,     0,
       0,     0,     0,     2,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     3,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    23,    24,
      25,    26,    68,    28,    29,    30,    31,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    34,
      35,     0,    37,     0,   170,     0,   171,     4,   262,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   175,
     176,   177,   178,   179,     0,     0,     0,     0,     0,     0,
     180,     5,     6,     7,     8,     9,    10,    11,    12,    13,
      14,    15,    16,     1,     0,     0,     0,     0,     0,     2,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     3,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    17,    18,    19,    20,    21,     0,    22,     0,     0,
       0,     0,     0,     4,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     5,     6,     7,
       8,     9,    10,    11,    12,    13,    14,    15,    16,     0,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
       0,     0,     0,     0,     0,     0,    33,     0,     0,     0,
       0,    34,    35,    36,    37,    38,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      39,     0,     0,     0,     0,     0,     0,    17,    18,    19,
      20,    21,     0,    22,     1,     0,     0,     0,     0,     0,
       2,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     3,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,     0,     0,     0,     0,
       0,     0,    33,     0,     4,     0,     0,    34,    35,    36,
      37,    38,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    39,     0,     5,     6,
       7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
       1,     0,     0,     0,     0,     0,     2,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     3,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    17,    18,
      19,    20,    21,     0,    22,     0,     0,     0,     0,     0,
       4,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     5,     6,     7,     8,     9,    10,
      11,    12,    13,    14,    15,    16,     0,    23,    24,    25,
      26,    68,    28,    29,    30,    31,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    34,    35,
       0,    37,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   345,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    17,    18,    19,    20,    21,     1,
      22,     0,     0,     0,     0,     2,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     3,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     1,     0,     0,     0,     0,
       0,     2,     0,    23,    24,    25,    26,    68,    28,    29,
      30,    31,     0,     3,     0,     0,     0,     0,     0,     4,
       0,     0,     0,     0,    34,    35,     0,    37,   219,     0,
     153,   154,   155,   156,   157,   158,     0,     0,   411,   159,
     160,     0,     0,     5,     6,     7,     8,     9,    10,    11,
      12,    13,    14,    15,    16,     4,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     5,
       6,     7,     8,     9,    10,    11,    12,    13,    14,    15,
      16,     0,     0,     0,     0,     0,     0,     0,   170,     0,
     171,     0,   172,    17,    18,    19,    20,    21,     0,    22,
       0,     0,     0,   175,   176,   177,   178,   179,     0,     0,
       0,     0,     0,     0,   180,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    17,
      18,    19,    20,    21,     1,    22,     0,     0,     0,     0,
       2,     0,    23,    24,    25,    26,    68,    28,    29,    30,
      31,     0,     3,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    34,    35,     0,    37,     0,     0,     0,
       1,     0,     0,     0,     0,     0,     2,   418,    23,    24,
      25,    26,    68,    28,    29,    30,    31,     0,     3,     0,
       0,     0,     0,     0,     4,     0,     0,     0,     0,    34,
      35,     0,    37,   219,     0,   153,   154,   155,   156,   157,
     158,     0,     0,   454,   159,   160,     0,     0,     5,     6,
       7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
       4,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     5,     6,     7,     8,     9,    10,
      11,    12,    13,    14,    15,    16,     0,     0,     0,     0,
       0,     0,     0,   170,     0,   171,     0,   262,    17,    18,
      19,    20,    21,     0,    22,     0,     0,     0,   175,   176,
     177,   178,   179,     0,     0,     0,     0,     0,     0,   180,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    17,    18,    19,    20,    21,     1,
      22,     0,     0,     0,     0,     2,     0,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,     3,     1,     0,
       0,     0,     0,    33,     2,     0,     0,     0,    34,    35,
       0,    37,    38,     0,     0,     0,     3,     0,     0,     0,
       0,     0,     0,    23,    24,    25,    26,    68,    28,    29,
      30,    31,     0,    79,    80,     0,     0,     0,     0,     4,
       0,     0,     0,     0,    34,    35,     0,    37,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     4,     0,
       0,     0,     0,     5,     6,     7,     8,     9,    10,    11,
      12,    13,    14,    15,    16,     0,     0,     0,     0,     0,
       0,     0,     5,     6,     7,     8,     9,    10,    11,    12,
      13,    14,    15,    16,     0,     0,     0,     1,     0,     0,
       0,     0,     0,     2,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     3,     0,     0,     0,     0,
       0,     0,     0,    17,    18,    19,    20,    21,     0,    22,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    17,    18,    19,    20,    21,     0,    22,     0,
       0,     0,     0,     0,     0,     0,     0,     4,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    23,    24,    25,    26,    68,    28,    29,    30,
      31,     5,     6,     7,     8,     9,    10,    11,    12,    13,
      14,    15,    16,    34,    35,     0,    37,     0,     0,     0,
       0,    79,    80,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    34,    35,   219,    37,   153,   154,   155,   156,
     157,   158,     0,     0,     0,   159,   160,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    17,    18,    19,    20,    21,     0,    22,   219,     0,
     153,   154,   155,   156,   157,   158,     0,     0,     0,   159,
     160,     0,     0,     0,     0,     0,     0,     0,   219,     0,
     153,   154,   155,   156,   157,   158,     0,     0,     0,   159,
     160,     0,     0,     0,   170,     0,   171,     0,   172,     0,
       0,     0,   407,     0,     0,     0,     0,     0,     0,   175,
     176,   177,   178,   179,     0,     0,     0,     0,     0,     0,
     180,    34,    35,     0,    37,     0,     0,     0,   170,     0,
     171,     0,   172,   486,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   175,   176,   177,   178,   179,   170,     0,
     171,     0,   172,   488,   180,     0,     0,     0,     0,     0,
       0,     0,     0,   175,   176,   177,   178,   179,     0,     0,
       0,     0,     0,     0,   180
};

static const short yycheck[] =
{
       0,     0,    32,   236,     0,   167,   130,    37,   234,   153,
      38,    41,   192,   239,   166,   167,   153,    41,   125,   228,
     171,   433,    50,   126,   127,    55,   133,   125,   153,   404,
      60,   406,     0,   242,    62,   125,    60,   144,     0,   213,
     213,   188,    83,   133,    85,   153,   144,   153,   153,   461,
     212,   153,   213,   163,   144,   232,   229,    57,    57,    46,
     237,    57,   172,   237,   206,    52,    96,   442,   229,   159,
     160,   234,    96,   448,   237,   228,   104,   164,   165,   189,
     170,   234,   123,   227,   227,   229,   242,   243,   244,    57,
     227,    78,   235,   245,   246,    57,   183,   127,   242,   250,
     184,   213,   192,   127,   134,   242,    93,   213,   341,   137,
     134,   213,   292,   162,   163,   348,    51,   242,   118,   118,
     495,   347,   118,   231,   231,   231,   231,   231,   228,   231,
     310,   311,   312,    83,   234,    85,    71,    46,    47,   228,
     184,   231,   229,   290,   172,   234,   233,   184,   232,   227,
     234,    86,   262,    88,   228,    90,   280,   109,   237,   184,
     234,   308,   114,   273,   274,   213,   228,   167,   167,    78,
     105,   167,   234,   123,    83,   228,    85,   287,   229,   227,
     234,   229,   233,   330,   417,   228,   231,   277,   232,   153,
     234,   234,   184,   340,   224,   232,   148,   234,   234,   237,
     224,   237,   292,   168,   169,   140,   234,   232,   236,   234,
     372,   239,   212,   212,   123,   234,   212,   184,   237,   329,
     310,   311,   312,   313,   314,   315,   316,   317,   318,   319,
     320,   321,   322,   323,   324,   325,   326,   327,   328,   342,
     232,   227,   234,   350,   228,   153,   353,   354,   228,   277,
     234,   361,   350,   363,   234,   353,   354,   404,   234,   406,
     350,   237,   237,   353,   354,   232,     4,   234,   231,   209,
     210,   211,   212,   435,   214,   215,   216,   317,   318,   319,
     320,   232,   282,   282,   231,   239,   282,   241,   234,   235,
     170,   171,   313,   314,   441,   442,   227,   315,   316,   321,
     322,   448,   173,   174,   175,   176,   177,   178,   179,   180,
     181,   182,   342,   341,   236,   345,   229,   227,   342,   347,
     348,   345,   228,   242,   434,   236,   231,   227,   153,   491,
     237,   493,   227,   237,   227,   227,   232,   160,   445,   170,
     231,   227,   172,   230,   506,   247,   249,   445,   495,   248,
     229,   458,   459,   463,   464,   445,   230,   227,   236,   236,
     458,   459,   362,   362,   227,   236,   362,   227,   458,   459,
     227,   227,   372,   372,   227,   237,   372,   227,   227,   189,
     161,   411,   412,     1,   161,   228,     4,   411,   412,   417,
     497,   498,    10,   230,   236,   153,   503,   150,   227,   497,
     498,   228,   228,   237,    22,   503,   234,   497,   498,   228,
     228,   228,   234,   503,   230,   153,   228,   155,   156,   157,
     158,   159,   160,   228,   237,   234,   164,   165,   231,   237,
     161,   192,   236,   433,   433,   435,   435,   433,   232,   435,
     230,   234,   234,   228,   235,   228,    64,   197,   198,   199,
     200,   201,   202,   203,   204,   205,   235,   237,   323,   325,
     324,   461,   461,   287,   147,   461,   327,   326,    78,    85,
      88,    89,    90,    91,    92,    93,    94,    95,    96,    97,
      98,    99,   328,    92,   231,   223,    64,   225,   425,   227,
     228,   491,   491,   493,   493,   491,   414,   493,   479,   482,
     238,   239,   240,   241,   242,   461,   506,   506,   362,    43,
     506,   249,   282,   506,   362,    57,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     148,   149,   150,   151,   152,   153,   154,   155,   156,   157,
     158,   159,   160,    -1,    -1,     1,   164,   165,     4,    -1,
      -1,    -1,    -1,    -1,    10,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    22,   185,   186,   187,
     188,   189,   190,   191,    -1,   193,   194,   195,   196,   197,
     198,   199,   200,   201,   202,   203,   204,   205,   206,    -1,
      -1,    -1,    -1,    -1,    -1,   213,    -1,    -1,    -1,    -1,
     218,   219,   220,   221,   222,   223,    -1,   225,    64,   227,
      -1,    -1,    -1,   231,   232,    -1,    -1,    -1,    -1,   237,
     238,   239,   240,   241,   242,    -1,    -1,    -1,    -1,    -1,
      -1,   249,    88,    89,    90,    91,    92,    93,    94,    95,
      96,    97,    98,    99,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   148,   149,   150,   151,   152,   153,   154,   155,
     156,   157,   158,   159,   160,    -1,    -1,     1,   164,   165,
       4,    -1,    -1,    -1,    -1,    -1,    10,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    22,   185,
     186,   187,   188,   189,   190,   191,    -1,   193,   194,   195,
     196,   197,   198,   199,   200,   201,   202,   203,   204,   205,
     206,    -1,    -1,    -1,    -1,    -1,    -1,   213,    -1,    -1,
      -1,    -1,   218,   219,   220,   221,   222,   223,    -1,   225,
      64,   227,    -1,    -1,    -1,   231,    -1,    -1,    -1,    -1,
      -1,   237,   238,   239,   240,   241,   242,    -1,    -1,    -1,
      -1,    -1,    -1,   249,    88,    89,    90,    91,    92,    93,
      94,    95,    96,    97,    98,    99,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   148,   149,   150,   151,   152,   153,
     154,   155,   156,   157,   158,   159,   160,    -1,    -1,     1,
     164,   165,     4,    -1,    -1,    -1,    -1,    -1,    10,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      22,   185,   186,   187,   188,   189,   190,   191,    -1,   193,
     194,    -1,    -1,   197,   198,   199,   200,   201,   202,   203,
     204,   205,   206,    -1,    -1,    -1,    -1,    -1,    -1,   213,
      -1,    -1,    -1,    -1,   218,   219,   220,   221,   222,   223,
      -1,   225,    64,   227,    -1,    -1,    -1,   231,   232,    -1,
      -1,    -1,    -1,   237,   238,   239,   240,   241,   242,    -1,
      -1,    -1,    -1,    -1,    -1,   249,    88,    89,    90,    91,
      92,    93,    94,    95,    96,    97,    98,    99,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   148,   149,   150,   151,
     152,   153,   154,   155,   156,   157,   158,   159,   160,    -1,
      -1,     1,   164,   165,     4,    -1,    -1,    -1,    -1,    -1,
      10,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    22,   185,   186,   187,   188,   189,   190,   191,
      -1,   193,   194,    -1,    -1,   197,   198,   199,   200,   201,
     202,   203,   204,   205,   206,    -1,    -1,    -1,    -1,    -1,
      -1,   213,    -1,    -1,    -1,    -1,   218,   219,   220,   221,
     222,   223,    -1,   225,    64,   227,    -1,    -1,    -1,   231,
     232,    -1,    -1,    -1,    -1,   237,   238,   239,   240,   241,
     242,    -1,    -1,    -1,    -1,    -1,    -1,   249,    88,    89,
      90,    91,    92,    93,    94,    95,    96,    97,    98,    99,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   148,   149,
     150,   151,   152,   153,   154,   155,   156,   157,   158,   159,
     160,    -1,    -1,     1,   164,   165,     4,    -1,    -1,    -1,
      -1,    -1,    10,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    22,   185,   186,   187,   188,   189,
     190,   191,    -1,   193,   194,    -1,    -1,   197,   198,   199,
     200,   201,   202,   203,   204,   205,   206,    -1,    -1,    -1,
      -1,    -1,    -1,   213,    -1,    -1,    -1,    -1,   218,   219,
     220,   221,   222,   223,    -1,   225,    64,   227,    -1,    -1,
      -1,   231,    -1,    -1,    -1,    -1,    -1,   237,   238,   239,
     240,   241,   242,    -1,    -1,    -1,    -1,    -1,    -1,   249,
      88,    89,    90,    91,    92,    93,    94,    95,    96,    97,
      98,    99,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,     1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     148,   149,   150,   151,   152,   153,   154,   155,   156,   157,
     158,   159,   160,    -1,    -1,     1,   164,   165,     4,    -1,
      -1,    -1,    -1,    -1,    10,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    22,   185,   186,   187,
     188,   189,   190,   191,    -1,   193,   194,    -1,    -1,   197,
     198,   199,   200,   201,   202,   203,   204,   205,   206,    -1,
      -1,    -1,    -1,    -1,    -1,   213,    -1,    -1,    -1,    -1,
     218,   219,   220,   221,   222,   223,    -1,   225,    64,   227,
      -1,    -1,    -1,   231,    -1,    -1,    -1,    -1,    -1,   237,
     238,   239,   240,   241,   242,    -1,    -1,    -1,    -1,    -1,
      -1,   249,    88,    89,    90,    91,    92,    93,    94,    95,
      96,    97,    98,    99,   153,    -1,   155,   156,   157,   158,
     159,   160,    -1,    -1,    -1,   164,   165,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   148,   149,   150,   151,   152,   153,   154,   155,
     156,   157,   158,   159,   160,    -1,    -1,    -1,   164,   165,
      -1,    -1,    -1,    -1,   223,    -1,   225,     4,   227,    -1,
      -1,    -1,    -1,    10,    -1,    -1,    -1,    -1,   237,   238,
     239,   240,   241,   242,    -1,    22,    -1,    -1,    -1,    -1,
     249,   197,   198,   199,   200,   201,   202,   203,   204,   205,
     206,    -1,    -1,    -1,    -1,    -1,    -1,   213,    -1,    -1,
      -1,    -1,   218,   219,   220,   221,   222,   223,    -1,   225,
      -1,   227,    -1,    -1,    -1,    -1,    -1,    64,    -1,    -1,
      -1,   237,   238,   239,   240,   241,   242,    -1,    -1,    -1,
      -1,    -1,    -1,   249,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    88,    89,    90,    91,    92,    93,    94,    95,    96,
      97,    98,    99,   153,    -1,   155,   156,   157,   158,   159,
     160,    -1,    -1,    -1,   164,   165,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,     4,    -1,    -1,    -1,    -1,
      -1,    10,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    22,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   148,   149,   150,   151,   152,   153,   154,   155,   156,
     157,   158,   159,   160,    -1,    -1,    -1,   164,   165,    -1,
      -1,    -1,    -1,   223,    -1,   225,    -1,   227,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    64,    -1,   237,   238,   239,
     240,   241,   242,    -1,    -1,    -1,    -1,    -1,    -1,   249,
     197,   198,   199,   200,   201,   202,   203,   204,   205,    88,
      89,    90,    91,    92,    93,    94,    95,    96,    97,    98,
      99,   218,   219,    -1,   221,    -1,   223,    -1,   225,    -1,
     227,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   238,   239,   240,   241,   242,    -1,    -1,    -1,    -1,
      -1,    -1,   249,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   148,
     149,   150,   151,   152,   153,   154,   155,   156,   157,   158,
     159,   160,    -1,     0,    -1,   164,   165,     4,    -1,    -1,
      -1,    -1,    -1,    10,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    22,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   197,   198,
     199,   200,   201,   202,   203,   204,   205,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   218,
     219,    -1,   221,    -1,   223,    -1,   225,    64,   227,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   238,
     239,   240,   241,   242,    -1,    -1,    -1,    -1,    -1,    -1,
     249,    88,    89,    90,    91,    92,    93,    94,    95,    96,
      97,    98,    99,     4,    -1,    -1,    -1,    -1,    -1,    10,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    22,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   148,   149,   150,   151,   152,    -1,   154,    -1,    -1,
      -1,    -1,    -1,    64,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    88,    89,    90,
      91,    92,    93,    94,    95,    96,    97,    98,    99,    -1,
     197,   198,   199,   200,   201,   202,   203,   204,   205,   206,
      -1,    -1,    -1,    -1,    -1,    -1,   213,    -1,    -1,    -1,
      -1,   218,   219,   220,   221,   222,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     237,    -1,    -1,    -1,    -1,    -1,    -1,   148,   149,   150,
     151,   152,    -1,   154,     4,    -1,    -1,    -1,    -1,    -1,
      10,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    22,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   197,   198,   199,   200,
     201,   202,   203,   204,   205,   206,    -1,    -1,    -1,    -1,
      -1,    -1,   213,    -1,    64,    -1,    -1,   218,   219,   220,
     221,   222,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   237,    -1,    88,    89,
      90,    91,    92,    93,    94,    95,    96,    97,    98,    99,
       4,    -1,    -1,    -1,    -1,    -1,    10,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    22,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   148,   149,
     150,   151,   152,    -1,   154,    -1,    -1,    -1,    -1,    -1,
      64,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    88,    89,    90,    91,    92,    93,
      94,    95,    96,    97,    98,    99,    -1,   197,   198,   199,
     200,   201,   202,   203,   204,   205,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   218,   219,
      -1,   221,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   232,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   148,   149,   150,   151,   152,     4,
     154,    -1,    -1,    -1,    -1,    10,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    22,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,     4,    -1,    -1,    -1,    -1,
      -1,    10,    -1,   197,   198,   199,   200,   201,   202,   203,
     204,   205,    -1,    22,    -1,    -1,    -1,    -1,    -1,    64,
      -1,    -1,    -1,    -1,   218,   219,    -1,   221,   153,    -1,
     155,   156,   157,   158,   159,   160,    -1,    -1,   232,   164,
     165,    -1,    -1,    88,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    99,    64,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    88,
      89,    90,    91,    92,    93,    94,    95,    96,    97,    98,
      99,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   223,    -1,
     225,    -1,   227,   148,   149,   150,   151,   152,    -1,   154,
      -1,    -1,    -1,   238,   239,   240,   241,   242,    -1,    -1,
      -1,    -1,    -1,    -1,   249,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   148,
     149,   150,   151,   152,     4,   154,    -1,    -1,    -1,    -1,
      10,    -1,   197,   198,   199,   200,   201,   202,   203,   204,
     205,    -1,    22,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   218,   219,    -1,   221,    -1,    -1,    -1,
       4,    -1,    -1,    -1,    -1,    -1,    10,   232,   197,   198,
     199,   200,   201,   202,   203,   204,   205,    -1,    22,    -1,
      -1,    -1,    -1,    -1,    64,    -1,    -1,    -1,    -1,   218,
     219,    -1,   221,   153,    -1,   155,   156,   157,   158,   159,
     160,    -1,    -1,   232,   164,   165,    -1,    -1,    88,    89,
      90,    91,    92,    93,    94,    95,    96,    97,    98,    99,
      64,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    88,    89,    90,    91,    92,    93,
      94,    95,    96,    97,    98,    99,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   223,    -1,   225,    -1,   227,   148,   149,
     150,   151,   152,    -1,   154,    -1,    -1,    -1,   238,   239,
     240,   241,   242,    -1,    -1,    -1,    -1,    -1,    -1,   249,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   148,   149,   150,   151,   152,     4,
     154,    -1,    -1,    -1,    -1,    10,    -1,   197,   198,   199,
     200,   201,   202,   203,   204,   205,   206,    22,     4,    -1,
      -1,    -1,    -1,   213,    10,    -1,    -1,    -1,   218,   219,
      -1,   221,   222,    -1,    -1,    -1,    22,    -1,    -1,    -1,
      -1,    -1,    -1,   197,   198,   199,   200,   201,   202,   203,
     204,   205,    -1,   207,   208,    -1,    -1,    -1,    -1,    64,
      -1,    -1,    -1,    -1,   218,   219,    -1,   221,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    64,    -1,
      -1,    -1,    -1,    88,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    99,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    88,    89,    90,    91,    92,    93,    94,    95,
      96,    97,    98,    99,    -1,    -1,    -1,     4,    -1,    -1,
      -1,    -1,    -1,    10,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    22,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   148,   149,   150,   151,   152,    -1,   154,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   148,   149,   150,   151,   152,    -1,   154,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    64,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   197,   198,   199,   200,   201,   202,   203,   204,
     205,    88,    89,    90,    91,    92,    93,    94,    95,    96,
      97,    98,    99,   218,   219,    -1,   221,    -1,    -1,    -1,
      -1,   207,   208,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   218,   219,   153,   221,   155,   156,   157,   158,
     159,   160,    -1,    -1,    -1,   164,   165,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   148,   149,   150,   151,   152,    -1,   154,   153,    -1,
     155,   156,   157,   158,   159,   160,    -1,    -1,    -1,   164,
     165,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   153,    -1,
     155,   156,   157,   158,   159,   160,    -1,    -1,    -1,   164,
     165,    -1,    -1,    -1,   223,    -1,   225,    -1,   227,    -1,
      -1,    -1,   231,    -1,    -1,    -1,    -1,    -1,    -1,   238,
     239,   240,   241,   242,    -1,    -1,    -1,    -1,    -1,    -1,
     249,   218,   219,    -1,   221,    -1,    -1,    -1,   223,    -1,
     225,    -1,   227,   228,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   238,   239,   240,   241,   242,   223,    -1,
     225,    -1,   227,   228,   249,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   238,   239,   240,   241,   242,    -1,    -1,
      -1,    -1,    -1,    -1,   249
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const unsigned short yystos[] =
{
       0,     4,    10,    22,    64,    88,    89,    90,    91,    92,
      93,    94,    95,    96,    97,    98,    99,   148,   149,   150,
     151,   152,   154,   197,   198,   199,   200,   201,   202,   203,
     204,   205,   206,   213,   218,   219,   220,   221,   222,   237,
     282,   283,   286,   287,   288,   289,   290,   295,   301,   305,
     308,   314,   315,   316,   317,   318,   319,   358,   359,   360,
     206,   308,   310,   227,   302,   153,   231,   308,   201,   282,
     283,   314,   319,   237,   310,   231,   341,   228,   234,   207,
     208,   291,   292,   293,   294,   295,   316,   316,   234,   237,
     314,   153,   227,   242,   296,   297,   298,   295,   153,   231,
     308,   237,     0,   359,   310,   314,   227,   287,   305,   231,
     153,   284,   285,   153,   231,   298,   237,   232,   342,   292,
     291,   294,   291,   293,   294,   229,   298,   298,   298,   297,
     295,   153,   227,   229,   300,   310,   231,   321,   153,   231,
     314,   298,   309,   284,   236,   184,   232,   234,   231,   284,
     227,     1,   153,   155,   156,   157,   158,   159,   160,   164,
     165,   185,   186,   187,   188,   189,   190,   191,   193,   194,
     223,   225,   227,   231,   237,   238,   239,   240,   241,   242,
     249,   252,   253,   254,   255,   257,   258,   259,   260,   261,
     262,   264,   265,   266,   267,   268,   269,   270,   271,   272,
     273,   274,   275,   276,   277,   278,   280,   286,   335,   336,
     337,   338,   343,   344,   345,   351,   357,   291,   294,   153,
     264,   277,   281,   300,   300,   310,   227,   228,   296,   281,
     299,   229,   310,   236,   320,   314,   324,   325,   231,   323,
     298,   227,   209,   210,   211,   212,   214,   215,   216,   311,
     313,   184,   232,   281,   285,   284,   184,   232,   232,   237,
     227,   235,   227,   264,   264,   237,   237,   237,   280,   153,
     352,   353,   336,   227,   227,   259,   264,   227,   259,   280,
     314,   232,   339,   160,   164,   165,   183,   229,   233,   228,
     234,   231,   263,     4,   228,   278,   280,   173,   174,   175,
     176,   177,   178,   179,   180,   181,   182,   236,   279,   262,
     242,   243,   244,   239,   241,   162,   163,   166,   167,   245,
     246,   168,   169,   249,   248,   247,   170,   172,   171,   250,
     234,   237,   232,   336,   230,   310,   236,   230,   299,   236,
     306,   324,   298,   326,   327,   232,   325,   322,   324,   227,
     227,   227,   227,   227,   227,   228,   234,   184,   232,   237,
     237,   227,   227,   354,   189,   280,   256,   280,   264,   228,
     228,   296,   343,   161,   256,   161,   278,   262,   228,   184,
     232,   278,   262,   262,   262,   266,   266,   267,   267,   268,
     268,   268,   268,   269,   269,   270,   271,   272,   273,   274,
     275,   280,   278,   236,   303,   230,   307,   231,   278,   328,
     329,   232,   300,   310,   234,   237,   310,   324,   232,   281,
     153,   150,   281,   281,   228,   312,   280,     1,   335,   344,
       1,   280,   355,   356,   227,   228,   228,   228,   228,   232,
     230,   235,   304,   329,   329,   229,   233,   330,   331,   332,
     334,   310,   310,   327,   232,   228,   228,   228,   234,   234,
     313,   228,   237,   237,   237,   337,   340,   341,   280,   336,
     350,   231,   348,   278,   329,   281,   161,   184,   232,   234,
     329,   236,   333,   281,   281,   340,   228,   280,   228,   280,
     228,   192,   232,   349,   230,   331,   334,   234,   234,   228,
     228,   237,   336,   195,   196,   336,   346,   347,   329,   281,
     281,   281,   235,   232,   347,   228,   228,   235
};

#if !defined (YYSIZE_T) && defined (__SIZE_TYPE__)
# define YYSIZE_T __SIZE_TYPE__
#endif
#if !defined (YYSIZE_T) && defined (size_t)
# define YYSIZE_T size_t
#endif
#if !defined (YYSIZE_T)
#if defined (__STDC__) || defined (__cplusplus)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# endif
#endif
#if !defined (YYSIZE_T)
# define YYSIZE_T unsigned int
#endif

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		(-2)
#define YYEOF		0

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrorlab


/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */

#define YYFAIL		goto yyerrlab

#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)					\
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    {								\
      yychar = (Token);						\
      yylval = (Value);						\
      yytoken = YYTRANSLATE (yychar);				\
      YYPOPSTACK;						\
      goto yybackup;						\
    }								\
  else								\
    { 								\
      yyerror ("syntax error: cannot back up");\
      YYERROR;							\
    }								\
while (0)

#define YYTERROR	1
#define YYERRCODE	256

/* YYLLOC_DEFAULT -- Compute the default location (before the actions
   are run).  */

#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)		\
   ((Current).first_line   = (Rhs)[1].first_line,	\
    (Current).first_column = (Rhs)[1].first_column,	\
    (Current).last_line    = (Rhs)[N].last_line,	\
    (Current).last_column  = (Rhs)[N].last_column)
#endif

/* YYLEX -- calling `yylex' with the right arguments.  */

#ifdef YYLEX_PARAM
# define YYLEX yylex (&yylval, YYLEX_PARAM)
#else
# define YYLEX yylex (&yylval)
#endif

/* Enable debugging if requested.  */
#if YYDEBUG

#ifndef YYFPRINTF
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)			\
do {						\
  if (yydebug)					\
    YYFPRINTF Args;				\
} while (0)

# define YYDSYMPRINT(Args)			\
do {						\
  if (yydebug)					\
    yysymprint Args;				\
} while (0)

# define YYDSYMPRINTF(Title, Token, Value, Location)		\
do {								\
  if (yydebug)							\
    {								\
      YYFPRINTF (stderr, "%s ", Title);				\
      yysymprint (stderr, 					\
                  Token, Value);	\
      YYFPRINTF (stderr, "\n");					\
    }								\
} while (0)

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yy_stack_print (short *bottom, short *top)
#else
static void
yy_stack_print (bottom, top)
    short *bottom;
    short *top;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (/* Nothing. */; bottom <= top; ++bottom)
    YYFPRINTF (stderr, " %d", *bottom);
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)				\
do {								\
  if (yydebug)							\
    yy_stack_print ((Bottom), (Top));				\
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yy_reduce_print (int yyrule)
#else
static void
yy_reduce_print (yyrule)
    int yyrule;
#endif
{
  int yyi;
  unsigned int yylno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %u), ",
             yyrule - 1, yylno);
  /* Print the symbols being reduced, and their result.  */
  for (yyi = yyprhs[yyrule]; 0 <= yyrhs[yyi]; yyi++)
    YYFPRINTF (stderr, "%s ", yytname [yyrhs[yyi]]);
  YYFPRINTF (stderr, "-> %s\n", yytname [yyr1[yyrule]]);
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (Rule);		\
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YYDSYMPRINT(Args)
# define YYDSYMPRINTF(Title, Token, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   SIZE_MAX < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#if defined (YYMAXDEPTH) && YYMAXDEPTH == 0
# undef YYMAXDEPTH
#endif

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif



#if YYERROR_VERBOSE

#ifndef yystrlen
#if defined (__GLIBC__) && defined (_STRING_H)
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
static YYSIZE_T
#if defined (__STDC__) || defined (__cplusplus)
yystrlen (const char *yystr)
#   else
yystrlen (yystr)
     const char *yystr;
#   endif
{
  register const char *yys = yystr;

  while (*yys++ != '\0')
    continue;

  return yys - yystr - 1;
}
#  endif
# endif

#ifndef yystpcpy
#if defined (__GLIBC__) && defined (_STRING_H) && defined (_GNU_SOURCE)
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
#if defined (__STDC__) || defined (__cplusplus)
yystpcpy (char *yydest, const char *yysrc)
#   else
yystpcpy (yydest, yysrc)
     char *yydest;
     const char *yysrc;
#   endif
{
  register char *yyd = yydest;
  register const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

#endif /* !YYERROR_VERBOSE */



#if YYDEBUG
/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yysymprint (FILE *yyoutput, int yytype, YYSTYPE *yyvaluep)
#else
static void
yysymprint (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  /* Pacify ``unused variable'' warnings.  */
  (void) yyvaluep;

  if (yytype < YYNTOKENS)
    {
      YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
#ifdef YYPRINT
      YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# endif
    }
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  switch (yytype)
    {
      default:
        break;
    }
  YYFPRINTF (yyoutput, ")");
}

#endif /* ! YYDEBUG */
/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yydestruct (int yytype, YYSTYPE *yyvaluep)
#else
static void
yydestruct (yytype, yyvaluep)
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  /* Pacify ``unused variable'' warnings.  */
  (void) yyvaluep;

  switch (yytype)
    {

      default:
        break;
    }
}


/* Prevent warnings from -Wmissing-prototypes.  */

#ifdef YYPARSE_PARAM
#if defined (__STDC__) || defined (__cplusplus)
int yyparse (void *YYPARSE_PARAM);
# else
int yyparse ();
# endif
#else /* ! YYPARSE_PARAM */
#if defined (__STDC__) || defined (__cplusplus)
int yyparse (void);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */






/*----------.
| yyparse.  |
`----------*/

#ifdef YYPARSE_PARAM
#if defined (__STDC__) || defined (__cplusplus)
int yyparse (void *YYPARSE_PARAM)
# else
int yyparse (YYPARSE_PARAM)
  void *YYPARSE_PARAM;
# endif
#else /* ! YYPARSE_PARAM */
#if defined (__STDC__) || defined (__cplusplus)
int
yyparse (void)
#else
int
yyparse ()

#endif
#endif
{
  /* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;

/* Number of syntax errors so far.  */
int yynerrs;

  register int yystate;
  register int yyn;
  int yyresult;
  /* Number of tokens to shift before error messages enabled.  */
  int yyerrstatus;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken = 0;

  /* Three stacks and their tools:
     `yyss': related to states,
     `yyvs': related to semantic values,
     `yyls': related to locations.

     Refer to the stacks thru separate pointers, to allow yyoverflow
     to reallocate them elsewhere.  */

  /* The state stack.  */
  short	yyssa[YYINITDEPTH];
  short *yyss = yyssa;
  register short *yyssp;

  /* The semantic value stack.  */
  YYSTYPE yyvsa[YYINITDEPTH];
  YYSTYPE *yyvs = yyvsa;
  register YYSTYPE *yyvsp;



#define YYPOPSTACK   (yyvsp--, yyssp--)

  YYSIZE_T yystacksize = YYINITDEPTH;

  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;


  /* When reducing, the number of symbols on the RHS of the reduced
     rule.  */
  int yylen;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY;		/* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  yyssp = yyss;
  yyvsp = yyvs;

  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed. so pushing a state here evens the stacks.
     */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
	/* Give user a chance to reallocate the stack. Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	short *yyss1 = yyss;


	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  This used to be a
	   conditional around just the two extra args, but that might
	   be undefined if yyoverflow is a macro.  */
	yyoverflow ("parser stack overflow",
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),

		    &yystacksize);

	yyss = yyss1;
	yyvs = yyvs1;
      }
#else /* no yyoverflow */
#ifndef YYSTACK_RELOCATE
      goto yyoverflowlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
	goto yyoverflowlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
	yystacksize = YYMAXDEPTH;

      {
	short *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyoverflowlab;
	YYSTACK_RELOCATE (yyss);
	YYSTACK_RELOCATE (yyvs);

#  undef YYSTACK_RELOCATE
	if (yyss1 != yyssa)
	  YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;


      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
		  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
	YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

/* Do appropriate processing given the current state.  */
/* Read a lookahead token if we need one and don't already have one.  */
/* yyresume: */

  /* First try to decide what to do without reference to lookahead token.  */

  yyn = yypact[yystate];
  if (yyn == YYPACT_NINF)
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YYDSYMPRINTF ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yyn == 0 || yyn == YYTABLE_NINF)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  /* Shift the lookahead token.  */
  YYDPRINTF ((stderr, "Shifting token %s, ", yytname[yytoken]));

  /* Discard the token being shifted unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  *++yyvsp = yylval;


  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  yystate = yyn;
  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     `$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 2:
#line 194 "gc_cl.y"
    { yyval.expr = clParseVariableIdentifier(Compiler, &yyvsp[0].token);
		  if(yyval.expr == gcvNULL) {
		     YYERROR;
		  }
		;}
    break;

  case 3:
#line 203 "gc_cl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 4:
#line 205 "gc_cl.y"
    { yyval.token = clParseCatStringLiteral(Compiler, &yyvsp[-1].token, &yyvsp[0].token); ;}
    break;

  case 5:
#line 210 "gc_cl.y"
    { yyval.expr = yyvsp[0].expr; ;}
    break;

  case 6:
#line 212 "gc_cl.y"
    { yyval.expr = clParseScalarConstant(Compiler, &yyvsp[0].token); ;}
    break;

  case 7:
#line 214 "gc_cl.y"
    { yyval.expr = clParseScalarConstant(Compiler, &yyvsp[0].token); ;}
    break;

  case 8:
#line 216 "gc_cl.y"
    { yyval.expr = clParseScalarConstant(Compiler, &yyvsp[0].token); ;}
    break;

  case 9:
#line 218 "gc_cl.y"
    { yyval.expr = clParseScalarConstant(Compiler, &yyvsp[0].token); ;}
    break;

  case 10:
#line 220 "gc_cl.y"
    { yyval.expr = clParseScalarConstant(Compiler, &yyvsp[0].token); ;}
    break;

  case 11:
#line 222 "gc_cl.y"
    { yyval.expr = clParseStringLiteral(Compiler, &yyvsp[0].token); ;}
    break;

  case 12:
#line 224 "gc_cl.y"
    { yyval.expr = yyvsp[-1].expr; ;}
    break;

  case 13:
#line 229 "gc_cl.y"
    { yyval.expr = yyvsp[0].expr; ;}
    break;

  case 14:
#line 231 "gc_cl.y"
    { yyval.expr = clParseSubscriptExpr(Compiler, yyvsp[-3].expr, yyvsp[-1].expr); ;}
    break;

  case 15:
#line 233 "gc_cl.y"
    { yyval.expr = clParseFuncCallExprAsExpr(Compiler, yyvsp[0].funcCall); ;}
    break;

  case 16:
#line 235 "gc_cl.y"
    { yyval.expr = clParseFieldSelectionExpr(Compiler, yyvsp[-2].expr, &yyvsp[0].token); ;}
    break;

  case 17:
#line 237 "gc_cl.y"
    { yyval.expr = clParsePtrFieldSelectionExpr(Compiler, yyvsp[-2].expr, &yyvsp[0].token); ;}
    break;

  case 18:
#line 239 "gc_cl.y"
    { yyval.expr = clParseIncOrDecExpr(Compiler, gcvNULL, clvUNARY_POST_INC, yyvsp[-1].expr); ;}
    break;

  case 19:
#line 241 "gc_cl.y"
    { yyval.expr = clParseIncOrDecExpr(Compiler, gcvNULL, clvUNARY_POST_DEC, yyvsp[-1].expr); ;}
    break;

  case 20:
#line 246 "gc_cl.y"
    { yyval.expr = yyvsp[0].expr; ;}
    break;

  case 21:
#line 251 "gc_cl.y"
    { yyval.funcCall = yyvsp[-1].funcCall; ;}
    break;

  case 22:
#line 253 "gc_cl.y"
    { yyval.funcCall = yyvsp[-2].funcCall; ;}
    break;

  case 23:
#line 255 "gc_cl.y"
    { yyval.funcCall = yyvsp[-1].funcCall; ;}
    break;

  case 24:
#line 260 "gc_cl.y"
    { yyval.funcCall = clParseFuncCallArgument(Compiler, yyvsp[-1].funcCall, yyvsp[0].expr); ;}
    break;

  case 25:
#line 262 "gc_cl.y"
    { yyval.funcCall = clParseFuncCallArgument(Compiler, yyvsp[-2].funcCall, yyvsp[0].expr); ;}
    break;

  case 26:
#line 267 "gc_cl.y"
    { yyval.decl = yyvsp[-1].decl; ;}
    break;

  case 27:
#line 269 "gc_cl.y"
    { yyval.decl = clParseCreateDecl(Compiler, &yyvsp[-2].decl, yyvsp[-1].typeQualifierList); ;}
    break;

  case 28:
#line 274 "gc_cl.y"
    { yyval.funcCall = clParseFuncCallHeaderExpr(Compiler, &yyvsp[-1].token, gcvNULL); ;}
    break;

  case 29:
#line 279 "gc_cl.y"
    {
	    clParseCastExprBegin(Compiler, &yyvsp[-1].decl);
	    (void) gcoOS_ZeroMemory((gctPOINTER)&yyval.token, sizeof(clsLexToken));
	    yyval.token.type = T_TYPE_CAST;
	;}
    break;

  case 30:
#line 288 "gc_cl.y"
    { yyval.expr = yyvsp[0].expr; ;}
    break;

  case 31:
#line 290 "gc_cl.y"
    {
		   clParseCastExprBegin(Compiler, gcvNULL);
	           (void) gcoOS_ZeroMemory((gctPOINTER)&yyval.token, sizeof(clsLexToken));
		   yyval.token.type = T_TYPE_CAST;
		;}
    break;

  case 32:
#line 296 "gc_cl.y"
    { yyval.expr = clParseCastExprEnd(Compiler, &yyvsp[-2].decl, yyvsp[0].expr);
                ;}
    break;

  case 33:
#line 300 "gc_cl.y"
    { yyval.expr = clParseCastExprEnd(Compiler, gcvNULL, yyvsp[-1].expr);
                ;}
    break;

  case 34:
#line 304 "gc_cl.y"
    { yyval.expr = clParseCastExprEnd(Compiler, gcvNULL, yyvsp[-1].expr);
                ;}
    break;

  case 35:
#line 310 "gc_cl.y"
    { yyval.expr = yyvsp[0].expr; ;}
    break;

  case 36:
#line 312 "gc_cl.y"
    { yyval.expr = clParseIncOrDecExpr(Compiler, &yyvsp[-1].token, clvUNARY_PRE_INC, yyvsp[0].expr); ;}
    break;

  case 37:
#line 314 "gc_cl.y"
    { yyval.expr = clParseIncOrDecExpr(Compiler, &yyvsp[-1].token, clvUNARY_PRE_DEC, yyvsp[0].expr); ;}
    break;

  case 38:
#line 316 "gc_cl.y"
    { yyval.expr = clParseVecStep(Compiler, &yyvsp[-3].token, &yyvsp[-1].expr->decl); ;}
    break;

  case 39:
#line 318 "gc_cl.y"
    { yyval.expr = clParseVecStep(Compiler, &yyvsp[-1].token, &yyvsp[0].decl); ;}
    break;

  case 40:
#line 320 "gc_cl.y"
    { yyval.expr = clParseSizeofExpr(Compiler, &yyvsp[-1].token, yyvsp[0].expr); ;}
    break;

  case 41:
#line 322 "gc_cl.y"
    { yyval.expr = clParseSizeofTypeDecl(Compiler, &yyvsp[-1].token, &yyvsp[0].decl); ;}
    break;

  case 42:
#line 324 "gc_cl.y"
    { yyval.expr = clParseNormalUnaryExpr(Compiler, &yyvsp[-1].token, yyvsp[0].expr); ;}
    break;

  case 43:
#line 329 "gc_cl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 44:
#line 331 "gc_cl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 45:
#line 333 "gc_cl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 46:
#line 335 "gc_cl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 47:
#line 337 "gc_cl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 48:
#line 339 "gc_cl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 49:
#line 344 "gc_cl.y"
    {
		   yyval.expr = yyvsp[0].expr;
		;}
    break;

  case 50:
#line 348 "gc_cl.y"
    { yyval.expr = clParseNormalBinaryExpr(Compiler, yyvsp[-2].expr, &yyvsp[-1].token, yyvsp[0].expr); ;}
    break;

  case 51:
#line 350 "gc_cl.y"
    { yyval.expr = clParseNormalBinaryExpr(Compiler, yyvsp[-2].expr, &yyvsp[-1].token, yyvsp[0].expr); ;}
    break;

  case 52:
#line 352 "gc_cl.y"
    { yyval.expr = clParseNormalBinaryExpr(Compiler, yyvsp[-2].expr, &yyvsp[-1].token, yyvsp[0].expr); ;}
    break;

  case 53:
#line 357 "gc_cl.y"
    { yyval.expr = yyvsp[0].expr; ;}
    break;

  case 54:
#line 359 "gc_cl.y"
    { yyval.expr = clParseNormalBinaryExpr(Compiler, yyvsp[-2].expr, &yyvsp[-1].token, yyvsp[0].expr); ;}
    break;

  case 55:
#line 361 "gc_cl.y"
    { yyval.expr = clParseNormalBinaryExpr(Compiler, yyvsp[-2].expr, &yyvsp[-1].token, yyvsp[0].expr); ;}
    break;

  case 56:
#line 366 "gc_cl.y"
    { yyval.expr = yyvsp[0].expr; ;}
    break;

  case 57:
#line 368 "gc_cl.y"
    { yyval.expr = clParseNormalBinaryExpr(Compiler, yyvsp[-2].expr, &yyvsp[-1].token, yyvsp[0].expr); ;}
    break;

  case 58:
#line 370 "gc_cl.y"
    { yyval.expr = clParseNormalBinaryExpr(Compiler, yyvsp[-2].expr, &yyvsp[-1].token, yyvsp[0].expr); ;}
    break;

  case 59:
#line 375 "gc_cl.y"
    { yyval.expr = yyvsp[0].expr; ;}
    break;

  case 60:
#line 377 "gc_cl.y"
    { yyval.expr = clParseNormalBinaryExpr(Compiler, yyvsp[-2].expr, &yyvsp[-1].token, yyvsp[0].expr); ;}
    break;

  case 61:
#line 379 "gc_cl.y"
    { yyval.expr = clParseNormalBinaryExpr(Compiler, yyvsp[-2].expr, &yyvsp[-1].token, yyvsp[0].expr); ;}
    break;

  case 62:
#line 381 "gc_cl.y"
    { yyval.expr = clParseNormalBinaryExpr(Compiler, yyvsp[-2].expr, &yyvsp[-1].token, yyvsp[0].expr); ;}
    break;

  case 63:
#line 383 "gc_cl.y"
    { yyval.expr = clParseNormalBinaryExpr(Compiler, yyvsp[-2].expr, &yyvsp[-1].token, yyvsp[0].expr); ;}
    break;

  case 64:
#line 388 "gc_cl.y"
    { yyval.expr = yyvsp[0].expr; ;}
    break;

  case 65:
#line 390 "gc_cl.y"
    { yyval.expr = clParseNormalBinaryExpr(Compiler, yyvsp[-2].expr, &yyvsp[-1].token, yyvsp[0].expr); ;}
    break;

  case 66:
#line 392 "gc_cl.y"
    { yyval.expr = clParseNormalBinaryExpr(Compiler, yyvsp[-2].expr, &yyvsp[-1].token, yyvsp[0].expr); ;}
    break;

  case 67:
#line 397 "gc_cl.y"
    { yyval.expr = yyvsp[0].expr; ;}
    break;

  case 68:
#line 399 "gc_cl.y"
    { yyval.expr = clParseNormalBinaryExpr(Compiler, yyvsp[-2].expr, &yyvsp[-1].token, yyvsp[0].expr); ;}
    break;

  case 69:
#line 404 "gc_cl.y"
    { yyval.expr = yyvsp[0].expr; ;}
    break;

  case 70:
#line 406 "gc_cl.y"
    { yyval.expr = clParseNormalBinaryExpr(Compiler, yyvsp[-2].expr, &yyvsp[-1].token, yyvsp[0].expr); ;}
    break;

  case 71:
#line 411 "gc_cl.y"
    { yyval.expr = yyvsp[0].expr; ;}
    break;

  case 72:
#line 413 "gc_cl.y"
    { yyval.expr = clParseNormalBinaryExpr(Compiler, yyvsp[-2].expr, &yyvsp[-1].token, yyvsp[0].expr); ;}
    break;

  case 73:
#line 418 "gc_cl.y"
    { yyval.expr = yyvsp[0].expr; ;}
    break;

  case 74:
#line 420 "gc_cl.y"
    { yyval.expr = clParseNormalBinaryExpr(Compiler, yyvsp[-2].expr, &yyvsp[-1].token, yyvsp[0].expr); ;}
    break;

  case 75:
#line 425 "gc_cl.y"
    { yyval.expr = yyvsp[0].expr; ;}
    break;

  case 76:
#line 427 "gc_cl.y"
    { yyval.expr = clParseNormalBinaryExpr(Compiler, yyvsp[-2].expr, &yyvsp[-1].token, yyvsp[0].expr); ;}
    break;

  case 77:
#line 432 "gc_cl.y"
    { yyval.expr = yyvsp[0].expr; ;}
    break;

  case 78:
#line 434 "gc_cl.y"
    { yyval.expr = clParseNormalBinaryExpr(Compiler, yyvsp[-2].expr, &yyvsp[-1].token, yyvsp[0].expr); ;}
    break;

  case 79:
#line 439 "gc_cl.y"
    { yyval.expr = yyvsp[0].expr; ;}
    break;

  case 80:
#line 441 "gc_cl.y"
    { yyval.expr = clParseSelectionExpr(Compiler, yyvsp[-4].expr, yyvsp[-2].expr, yyvsp[0].expr); ;}
    break;

  case 81:
#line 446 "gc_cl.y"
    { yyval.expr = yyvsp[0].expr;;}
    break;

  case 82:
#line 448 "gc_cl.y"
    { yyval.expr = clParseAssignmentExpr(Compiler, yyvsp[-2].expr, &yyvsp[-1].token, yyvsp[0].expr); ;}
    break;

  case 83:
#line 453 "gc_cl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 84:
#line 455 "gc_cl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 85:
#line 457 "gc_cl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 86:
#line 459 "gc_cl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 87:
#line 461 "gc_cl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 88:
#line 463 "gc_cl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 89:
#line 465 "gc_cl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 90:
#line 467 "gc_cl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 91:
#line 469 "gc_cl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 92:
#line 471 "gc_cl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 93:
#line 473 "gc_cl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 94:
#line 478 "gc_cl.y"
    { yyval.expr = yyvsp[0].expr; ;}
    break;

  case 95:
#line 480 "gc_cl.y"
    {
	    if(cloCOMPILER_GetParserState(Compiler) == clvPARSER_IN_TYPE_CAST) {
               yyval.expr = clParseBinarySequenceExpr(Compiler,
					      (YYSTYPE *)&yyvsp[-3].token, yyvsp[-2].expr, &yyvsp[-1].token, yyvsp[0].expr);
	    }
	    else {
               yyval.expr = clParseNormalBinaryExpr(Compiler, yyvsp[-2].expr, &yyvsp[-1].token, yyvsp[0].expr);
	    }
	  ;}
    break;

  case 96:
#line 493 "gc_cl.y"
    { yyval.expr = yyvsp[0].expr; ;}
    break;

  case 97:
#line 498 "gc_cl.y"
    { yyval.dataType = clParseTaggedDecl(Compiler, &yyvsp[-1].token, &yyvsp[0].token); ;}
    break;

  case 98:
#line 500 "gc_cl.y"
    { yyval.dataType = clParseTaggedDecl(Compiler, &yyvsp[-1].token, &yyvsp[0].token); ;}
    break;

  case 99:
#line 507 "gc_cl.y"
    { yyval.dataType = clParseEnumSpecifier(Compiler, &yyvsp[-5].token, yyvsp[-4].attr, &yyvsp[-3].token, (gctPOINTER)yyvsp[-1].enumeratorList); ;}
    break;

  case 100:
#line 510 "gc_cl.y"
    { yyval.dataType = clParseEnumSpecifier(Compiler, &yyvsp[-5].token, yyvsp[-4].attr, &yyvsp[-3].token, (gctPOINTER)yyvsp[-1].enumeratorList); ;}
    break;

  case 101:
#line 514 "gc_cl.y"
    { yyval.dataType = clParseEnumSpecifier(Compiler, &yyvsp[-4].token, gcvNULL, &yyvsp[-3].token, (gctPOINTER)yyvsp[-1].enumeratorList); ;}
    break;

  case 102:
#line 517 "gc_cl.y"
    { yyval.dataType = clParseEnumSpecifier(Compiler, &yyvsp[-4].token, gcvNULL, &yyvsp[-3].token, (gctPOINTER)yyvsp[-1].enumeratorList); ;}
    break;

  case 103:
#line 521 "gc_cl.y"
    { yyval.dataType = clParseEnumSpecifier(Compiler, &yyvsp[-4].token, yyvsp[-3].attr, gcvNULL, (gctPOINTER)yyvsp[-1].enumeratorList); ;}
    break;

  case 104:
#line 524 "gc_cl.y"
    { yyval.dataType = clParseEnumSpecifier(Compiler, &yyvsp[-4].token, yyvsp[-3].attr, gcvNULL, (gctPOINTER)yyvsp[-1].enumeratorList); ;}
    break;

  case 105:
#line 528 "gc_cl.y"
    { yyval.dataType = clParseEnumSpecifier(Compiler, &yyvsp[-3].token, gcvNULL, gcvNULL, (gctPOINTER)yyvsp[-1].enumeratorList); ;}
    break;

  case 106:
#line 531 "gc_cl.y"
    { yyval.dataType = clParseEnumSpecifier(Compiler, &yyvsp[-3].token, gcvNULL, gcvNULL, (gctPOINTER)yyvsp[-1].enumeratorList); ;}
    break;

  case 107:
#line 536 "gc_cl.y"
    {
		   slsSLINK_LIST *enumList;

		   slmSLINK_LIST_Initialize(enumList);
                   yyval.enumeratorList = clParseAddEnumerator(Compiler, yyvsp[0].enumeratorName, enumList);
		;}
    break;

  case 108:
#line 543 "gc_cl.y"
    { yyval.enumeratorList = clParseAddEnumerator(Compiler, yyvsp[0].enumeratorName, yyvsp[-2].enumeratorList); ;}
    break;

  case 109:
#line 548 "gc_cl.y"
    { yyval.enumeratorName = clParseEnumerator(Compiler, &yyvsp[0].token, gcvNULL); ;}
    break;

  case 110:
#line 550 "gc_cl.y"
    { yyval.enumeratorName = clParseEnumerator(Compiler, &yyvsp[-2].token, yyvsp[0].expr); ;}
    break;

  case 111:
#line 555 "gc_cl.y"
    { yyval.statement = clParseDeclaration(Compiler, yyvsp[-1].declOrDeclList); ;}
    break;

  case 112:
#line 557 "gc_cl.y"
    { yyval.statement = clParseEnumTags(Compiler, yyvsp[-2].dataType, yyvsp[-1].attr); ;}
    break;

  case 113:
#line 559 "gc_cl.y"
    { yyval.statement = clParseTags(Compiler, yyvsp[-1].dataType); ;}
    break;

  case 114:
#line 561 "gc_cl.y"
    { yyval.statement = clParseTags(Compiler, yyvsp[-1].dataType); ;}
    break;

  case 115:
#line 566 "gc_cl.y"
    { yyval.funcName = yyvsp[-1].funcName; ;}
    break;

  case 116:
#line 571 "gc_cl.y"
    { yyval.funcName = yyvsp[0].funcName; ;}
    break;

  case 117:
#line 573 "gc_cl.y"
    { yyval.funcName = yyvsp[0].funcName; ;}
    break;

  case 118:
#line 578 "gc_cl.y"
    { yyval.funcName = clParseParameterList(Compiler, yyvsp[-1].funcName, yyvsp[0].paramName); ;}
    break;

  case 119:
#line 580 "gc_cl.y"
    { yyval.funcName = clParseParameterList(Compiler, yyvsp[-2].funcName, yyvsp[0].paramName); ;}
    break;

  case 120:
#line 585 "gc_cl.y"
    { yyval.funcName = clParseKernelFuncHeader(Compiler, yyvsp[-3].attr, &yyvsp[-2].decl, &yyvsp[-1].token); ;}
    break;

  case 121:
#line 587 "gc_cl.y"
    { yyval.funcName = clParseExternKernelFuncHeader(Compiler, yyvsp[-3].attr, &yyvsp[-2].decl, &yyvsp[-1].token); ;}
    break;

  case 122:
#line 589 "gc_cl.y"
    { yyval.funcName = clParseFuncHeader(Compiler, &yyvsp[-2].decl, &yyvsp[-1].token); ;}
    break;

  case 123:
#line 591 "gc_cl.y"
    { yyval.funcName = clParseFuncHeaderWithAttr(Compiler, yyvsp[-3].attr, &yyvsp[-2].decl, &yyvsp[-1].token); ;}
    break;

  case 124:
#line 593 "gc_cl.y"
    { yyval.funcName = clParseFuncHeader(Compiler, &yyvsp[-2].decl, &yyvsp[-1].token);
		  if(yyval.funcName) yyval.funcName->u.funcInfo.isInline = gcvTRUE;
		;}
    break;

  case 125:
#line 600 "gc_cl.y"
    { yyval.paramName = clParseParameterDecl(Compiler, &yyvsp[-1].decl, &yyvsp[0].token); ;}
    break;

  case 126:
#line 602 "gc_cl.y"
    { yyval.paramName = clParseArrayParameterDecl(Compiler, &yyvsp[-2].decl, &yyvsp[-1].token, yyvsp[0].expr); ;}
    break;

  case 127:
#line 607 "gc_cl.y"
    { yyval.paramName = clParseQualifiedParameterDecl(Compiler, gcvNULL, gcvNULL, yyvsp[0].paramName); ;}
    break;

  case 128:
#line 609 "gc_cl.y"
    { yyval.paramName = clParseQualifiedParameterDecl(Compiler, gcvNULL, &yyvsp[-1].token, yyvsp[0].paramName); ;}
    break;

  case 129:
#line 611 "gc_cl.y"
    { yyval.paramName = clParseQualifiedParameterDecl(Compiler, gcvNULL, gcvNULL, yyvsp[0].paramName); ;}
    break;

  case 130:
#line 613 "gc_cl.y"
    { yyval.paramName = clParseQualifiedParameterDecl(Compiler, gcvNULL, &yyvsp[-1].token, yyvsp[0].paramName); ;}
    break;

  case 131:
#line 615 "gc_cl.y"
    { yyval.paramName = clParseQualifiedParameterDecl(Compiler, yyvsp[-1].typeQualifierList, gcvNULL, yyvsp[0].paramName); ;}
    break;

  case 132:
#line 617 "gc_cl.y"
    { yyval.paramName = clParseQualifiedParameterDecl(Compiler, yyvsp[-2].typeQualifierList, &yyvsp[-1].token, yyvsp[0].paramName); ;}
    break;

  case 133:
#line 619 "gc_cl.y"
    { yyval.paramName = clParseQualifiedParameterDecl(Compiler, yyvsp[-1].typeQualifierList, gcvNULL, yyvsp[0].paramName); ;}
    break;

  case 134:
#line 621 "gc_cl.y"
    { yyval.paramName = clParseQualifiedParameterDecl(Compiler, yyvsp[-2].typeQualifierList, &yyvsp[-1].token, yyvsp[0].paramName); ;}
    break;

  case 135:
#line 626 "gc_cl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 136:
#line 628 "gc_cl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 137:
#line 633 "gc_cl.y"
    { yyval.paramName = clParseParameterDecl(Compiler, &yyvsp[0].decl, gcvNULL); ;}
    break;

  case 138:
#line 635 "gc_cl.y"
    { yyval.paramName = clParseArrayParameterDecl(Compiler, &yyvsp[-3].decl, gcvNULL, yyvsp[-1].expr); ;}
    break;

  case 139:
#line 640 "gc_cl.y"
    {
		    yyval.typeQualifierList = clParseTypeQualifierList(Compiler, &yyvsp[0].token,
		                                  clParseEmptyTypeQualifierList(Compiler));
		;}
    break;

  case 140:
#line 645 "gc_cl.y"
    {yyval.typeQualifierList = clParseTypeQualifierList(Compiler, &yyvsp[-1].token, yyvsp[0].typeQualifierList); ;}
    break;

  case 141:
#line 650 "gc_cl.y"
    { yyval.typeQualifierList = clParseEmptyTypeQualifierList(Compiler); ;}
    break;

  case 142:
#line 652 "gc_cl.y"
    {yyval.typeQualifierList = yyvsp[0].typeQualifierList;;}
    break;

  case 143:
#line 654 "gc_cl.y"
    {yyval.typeQualifierList = clParsePointerTypeQualifier(Compiler, yyvsp[-1].typeQualifierList, yyvsp[0].typeQualifierList); ;}
    break;

  case 144:
#line 658 "gc_cl.y"
    {yyval.token = yyvsp[0].token;;}
    break;

  case 145:
#line 660 "gc_cl.y"
    {
		  yyval.token = yyvsp[0].token;
		  yyval.token.u.identifier.ptrDscr = yyvsp[-1].typeQualifierList;
		;}
    break;

  case 146:
#line 667 "gc_cl.y"
    {  yyval.token = yyvsp[0].token; ;}
    break;

  case 147:
#line 669 "gc_cl.y"
    { yyval.token = yyvsp[-1].token; ;}
    break;

  case 148:
#line 673 "gc_cl.y"
    { yyval.expr = clParseNullExpr(Compiler, &yyvsp[0].token); ;}
    break;

  case 149:
#line 675 "gc_cl.y"
    { yyval.expr = yyvsp[0].expr; ;}
    break;

  case 150:
#line 680 "gc_cl.y"
    { yyval.expr = yyvsp[-1].expr; ;}
    break;

  case 151:
#line 682 "gc_cl.y"
    { yyval.expr = clParseArrayDeclarator(Compiler, &yyvsp[-2].token, yyvsp[-3].expr, yyvsp[-1].expr); ;}
    break;

  case 152:
#line 687 "gc_cl.y"
    { yyval.declOrDeclList = yyvsp[0].declOrDeclList; ;}
    break;

  case 153:
#line 689 "gc_cl.y"
    {
    		   cloCOMPILER_PushParserState(Compiler, clvPARSER_IN_TYPEDEF);
		;}
    break;

  case 154:
#line 693 "gc_cl.y"
    {
		   yyval.declOrDeclList = clParseTypeDef(Compiler, yyvsp[0].declOrDeclList);
		   cloCOMPILER_PopParserState(Compiler);
		;}
    break;

  case 155:
#line 698 "gc_cl.y"
    { yyval.declOrDeclList = clParseVariableDeclList(Compiler, yyvsp[-3].declOrDeclList, &yyvsp[-1].token, yyvsp[0].attr); ;}
    break;

  case 156:
#line 700 "gc_cl.y"
    { yyval.declOrDeclList = clParseArrayVariableDeclList(Compiler, yyvsp[-4].declOrDeclList, &yyvsp[-2].token, yyvsp[-1].expr, yyvsp[0].attr); ;}
    break;

  case 157:
#line 702 "gc_cl.y"
    { yyval.declOrDeclList = clParseVariableDeclListInit(Compiler, yyvsp[-4].declOrDeclList, &yyvsp[-2].token, yyvsp[-1].attr); ;}
    break;

  case 158:
#line 704 "gc_cl.y"
    { yyval.declOrDeclList = clParseFinishDeclListInit(Compiler, yyvsp[-1].declOrDeclList, yyvsp[0].expr); ;}
    break;

  case 159:
#line 706 "gc_cl.y"
    { yyval.declOrDeclList = clParseArrayVariableDeclListInit(Compiler, yyvsp[-5].declOrDeclList, &yyvsp[-3].token, yyvsp[-2].expr, yyvsp[-1].attr); ;}
    break;

  case 160:
#line 708 "gc_cl.y"
    { yyval.declOrDeclList = clParseFinishDeclListInit(Compiler, yyvsp[-1].declOrDeclList, yyvsp[0].expr); ;}
    break;

  case 161:
#line 714 "gc_cl.y"
    { yyval.declOrDeclList = clParseFuncDecl(Compiler, yyvsp[0].funcName); ;}
    break;

  case 162:
#line 716 "gc_cl.y"
    { yyval.declOrDeclList = clParseVariableDecl(Compiler, &yyvsp[-2].decl, &yyvsp[-1].token, yyvsp[0].attr); ;}
    break;

  case 163:
#line 718 "gc_cl.y"
    { yyval.declOrDeclList = clParseArrayVariableDecl(Compiler, &yyvsp[-3].decl, &yyvsp[-2].token, yyvsp[-1].expr, yyvsp[0].attr); ;}
    break;

  case 164:
#line 720 "gc_cl.y"
    { yyval.declOrDeclList = clParseVariableDeclInit(Compiler, &yyvsp[-3].decl, &yyvsp[-2].token, yyvsp[-1].attr); ;}
    break;

  case 165:
#line 722 "gc_cl.y"
    { yyval.declOrDeclList = clParseFinishDeclInit(Compiler, yyvsp[-1].declOrDeclList, yyvsp[0].expr); ;}
    break;

  case 166:
#line 724 "gc_cl.y"
    { yyval.declOrDeclList = clParseArrayVariableDeclInit(Compiler, &yyvsp[-4].decl, &yyvsp[-3].token, yyvsp[-2].expr, yyvsp[-1].attr); ;}
    break;

  case 167:
#line 726 "gc_cl.y"
    { yyval.declOrDeclList = clParseFinishDeclInit(Compiler, yyvsp[-1].declOrDeclList, yyvsp[0].expr); ;}
    break;

  case 168:
#line 733 "gc_cl.y"
    { yyval.attr = gcvNULL; ;}
    break;

  case 169:
#line 735 "gc_cl.y"
    { yyval.attr = yyvsp[-2].attr; ;}
    break;

  case 170:
#line 739 "gc_cl.y"
    { yyval.attr = gcvNULL; ;}
    break;

  case 171:
#line 741 "gc_cl.y"
    { yyval.attr = yyvsp[0].attr; ;}
    break;

  case 172:
#line 745 "gc_cl.y"
    { yyval.attr = yyvsp[0].attr; ;}
    break;

  case 173:
#line 747 "gc_cl.y"
    { yyval.attr = yyvsp[0].attr; ;}
    break;

  case 174:
#line 749 "gc_cl.y"
    { yyval.attr = yyvsp[-1].attr; ;}
    break;

  case 175:
#line 751 "gc_cl.y"
    { yyval.attr = yyvsp[0].attr; ;}
    break;

  case 176:
#line 756 "gc_cl.y"
    { yyval.attr = clParseAttributeEndianType(Compiler, yyvsp[-4].attr,  &yyvsp[-1].token); ;}
    break;

  case 177:
#line 758 "gc_cl.y"
    { yyval.attr = clParseSimpleAttribute(Compiler, &yyvsp[0].token, clvATTR_PACKED, yyvsp[-1].attr); ;}
    break;

  case 178:
#line 760 "gc_cl.y"
    { yyval.attr = clParseAttributeVecTypeHint(Compiler, yyvsp[-4].attr, &yyvsp[-1].token); ;}
    break;

  case 179:
#line 762 "gc_cl.y"
    { yyval.attr = clParseAttributeReqdWorkGroupSize(Compiler, yyvsp[-8].attr, yyvsp[-5].expr, yyvsp[-3].expr, yyvsp[-1].expr); ;}
    break;

  case 180:
#line 764 "gc_cl.y"
    { yyval.attr = clParseAttributeWorkGroupSizeHint(Compiler, yyvsp[-8].attr, yyvsp[-5].expr, yyvsp[-3].expr, yyvsp[-1].expr); ;}
    break;

  case 181:
#line 766 "gc_cl.y"
    { yyval.attr = clParseAttributeAligned(Compiler, yyvsp[-1].attr, gcvNULL); ;}
    break;

  case 182:
#line 768 "gc_cl.y"
    { yyval.attr = clParseAttributeAligned(Compiler, yyvsp[-4].attr, yyvsp[-1].expr); ;}
    break;

  case 183:
#line 770 "gc_cl.y"
    { yyval.attr = clParseSimpleAttribute(Compiler, &yyvsp[0].token, clvATTR_ALWAYS_INLINE, yyvsp[-1].attr); ;}
    break;

  case 184:
#line 775 "gc_cl.y"
    { yyval.decl = clParseQualifiedType(Compiler, gcvNULL, gcvFALSE, &yyvsp[0].decl); ;}
    break;

  case 185:
#line 777 "gc_cl.y"
    { yyval.decl = clParseQualifiedType(Compiler, yyvsp[-1].typeQualifierList, gcvFALSE, &yyvsp[0].decl); ;}
    break;

  case 186:
#line 782 "gc_cl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 187:
#line 784 "gc_cl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 188:
#line 786 "gc_cl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 189:
#line 788 "gc_cl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 190:
#line 790 "gc_cl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 191:
#line 792 "gc_cl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 192:
#line 794 "gc_cl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 193:
#line 796 "gc_cl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 194:
#line 798 "gc_cl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 195:
#line 803 "gc_cl.y"
    { yyval.decl = clParseNonStructType(Compiler, &yyvsp[0].token); ;}
    break;

  case 196:
#line 805 "gc_cl.y"
    { yyval.decl = clParseCreateDeclFromDataType(Compiler, yyvsp[0].dataType); ;}
    break;

  case 197:
#line 807 "gc_cl.y"
    { yyval.decl = clParseCreateDeclFromDataType(Compiler, yyvsp[0].dataType); ;}
    break;

  case 198:
#line 809 "gc_cl.y"
    { yyval.decl = clParseCreateDeclFromDataType(Compiler, yyvsp[0].dataType); ;}
    break;

  case 199:
#line 814 "gc_cl.y"
    { yyval.token = yyvsp[0].token;}
    break;

  case 200:
#line 816 "gc_cl.y"
    { yyval.token = yyvsp[0].token;}
    break;

  case 201:
#line 818 "gc_cl.y"
    { yyval.token = yyvsp[0].token;}
    break;

  case 202:
#line 820 "gc_cl.y"
    { yyval.token = yyvsp[0].token;}
    break;

  case 203:
#line 822 "gc_cl.y"
    { yyval.token = yyvsp[0].token;}
    break;

  case 204:
#line 824 "gc_cl.y"
    { yyval.token = yyvsp[0].token;}
    break;

  case 205:
#line 826 "gc_cl.y"
    { yyval.token = yyvsp[0].token;}
    break;

  case 206:
#line 828 "gc_cl.y"
    { yyval.token = yyvsp[0].token;}
    break;

  case 207:
#line 830 "gc_cl.y"
    { yyval.token = yyvsp[0].token;}
    break;

  case 208:
#line 832 "gc_cl.y"
    { yyval.token = yyvsp[0].token;}
    break;

  case 209:
#line 834 "gc_cl.y"
    { yyval.token = yyvsp[0].token;}
    break;

  case 210:
#line 836 "gc_cl.y"
    { yyval.token = yyvsp[0].token;}
    break;

  case 211:
#line 838 "gc_cl.y"
    { yyval.token = yyvsp[0].token;}
    break;

  case 212:
#line 840 "gc_cl.y"
    { yyval.token = yyvsp[0].token;}
    break;

  case 213:
#line 842 "gc_cl.y"
    { yyval.token = yyvsp[0].token;}
    break;

  case 214:
#line 844 "gc_cl.y"
    { yyval.token = yyvsp[0].token;}
    break;

  case 215:
#line 846 "gc_cl.y"
    { yyval.token = yyvsp[0].token;}
    break;

  case 216:
#line 848 "gc_cl.y"
    { yyval.token = yyvsp[0].token;}
    break;

  case 217:
#line 850 "gc_cl.y"
    { yyval.token = yyvsp[0].token;}
    break;

  case 218:
#line 852 "gc_cl.y"
    { yyval.token = yyvsp[0].token;}
    break;

  case 219:
#line 854 "gc_cl.y"
    { yyval.token = yyvsp[0].token;}
    break;

  case 220:
#line 856 "gc_cl.y"
    { yyval.token = yyvsp[0].token;}
    break;

  case 221:
#line 860 "gc_cl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 222:
#line 862 "gc_cl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 223:
#line 867 "gc_cl.y"
    { clParseStructDeclBegin(Compiler); ;}
    break;

  case 224:
#line 869 "gc_cl.y"
    { yyval.dataType = clParseStructDeclEnd(Compiler, &yyvsp[-6].token, &yyvsp[-5].token, yyvsp[0].attr, yyvsp[-2].status); ;}
    break;

  case 225:
#line 871 "gc_cl.y"
    { clParseStructDeclBegin(Compiler); ;}
    break;

  case 226:
#line 873 "gc_cl.y"
    { yyval.dataType = clParseStructDeclEnd(Compiler, &yyvsp[-5].token, gcvNULL, yyvsp[0].attr, yyvsp[-2].status); ;}
    break;

  case 227:
#line 875 "gc_cl.y"
    { clParseStructDeclBegin(Compiler); ;}
    break;

  case 228:
#line 877 "gc_cl.y"
    { yyval.dataType = clParseStructDeclEnd(Compiler, &yyvsp[-6].token, &yyvsp[-4].token, yyvsp[-5].attr, yyvsp[-1].status); ;}
    break;

  case 229:
#line 879 "gc_cl.y"
    { clParseStructDeclBegin(Compiler); ;}
    break;

  case 230:
#line 881 "gc_cl.y"
    { yyval.dataType = clParseStructDeclEnd(Compiler, &yyvsp[-5].token, gcvNULL, yyvsp[-4].attr, yyvsp[-1].status); ;}
    break;

  case 231:
#line 886 "gc_cl.y"
    {
		   yyval.status = yyvsp[0].status;
		;}
    break;

  case 232:
#line 890 "gc_cl.y"
    {
		   if(gcmIS_ERROR(yyvsp[-1].status)) {
                       yyval.status = yyvsp[-1].status;
		   }
		   else {
                       yyval.status = yyvsp[0].status;

		   }
		;}
    break;

  case 233:
#line 903 "gc_cl.y"
    {
                   yyval.status = clParseTypeSpecifiedFieldDeclList(Compiler, &yyvsp[-2].decl, yyvsp[-1].fieldDeclList);
		;}
    break;

  case 234:
#line 910 "gc_cl.y"
    { yyval.fieldDeclList = clParseFieldDeclList(Compiler, yyvsp[0].fieldDecl); ;}
    break;

  case 235:
#line 912 "gc_cl.y"
    { yyval.fieldDeclList = clParseFieldDeclList2(Compiler, yyvsp[-2].fieldDeclList, yyvsp[0].fieldDecl); ;}
    break;

  case 236:
#line 917 "gc_cl.y"
    { yyval.fieldDecl = clParseFieldDecl(Compiler, &yyvsp[-1].token, gcvNULL, yyvsp[0].attr); ;}
    break;

  case 237:
#line 919 "gc_cl.y"
    { yyval.fieldDecl = clParseFieldDecl(Compiler, &yyvsp[-2].token, yyvsp[-1].expr, yyvsp[0].attr); ;}
    break;

  case 238:
#line 924 "gc_cl.y"
    {
		   cloIR_TYPECAST_ARGS typeCastArgs;
		   gceSTATUS status;

		   /* Create type cast expression */
		   status = cloIR_TYPECAST_ARGS_Construct(Compiler,
							  yyvsp[0].token.lineNo,
							  yyvsp[0].token.stringNo,
							  &typeCastArgs);
		   if(gcmIS_ERROR(status)) {
		      YYERROR;
		      yyval.expr = gcvNULL;
		   }
		   else {
		      yyval.expr = &typeCastArgs->exprBase;
		   }
		;}
    break;

  case 239:
#line 946 "gc_cl.y"
    { yyval.expr = yyvsp[0].expr; ;}
    break;

  case 240:
#line 948 "gc_cl.y"
    { yyval.expr = yyvsp[-1].expr; ;}
    break;

  case 241:
#line 950 "gc_cl.y"
    { yyval.expr = yyvsp[-1].expr; ;}
    break;

  case 242:
#line 955 "gc_cl.y"
    { yyval.expr = clParseInitializerList(Compiler, yyvsp[-2].expr, yyvsp[-1].declOrDeclList, yyvsp[0].expr); ;}
    break;

  case 243:
#line 957 "gc_cl.y"
    { yyval.expr = clParseInitializerList(Compiler, yyvsp[-3].expr, yyvsp[-1].declOrDeclList, yyvsp[0].expr); ;}
    break;

  case 244:
#line 961 "gc_cl.y"
    { yyval.declOrDeclList = gcvNULL; ;}
    break;

  case 245:
#line 963 "gc_cl.y"
    {
		   gceSTATUS status;
		   status = cloCOMPILER_PushDesignationScope(Compiler,
							     yyvsp[-1].declOrDeclList->lhs);
		   if(gcmIS_ERROR(status)) {
		     YYERROR;
		   }
		   yyval.declOrDeclList = yyvsp[-1].declOrDeclList;
		;}
    break;

  case 246:
#line 976 "gc_cl.y"
    { yyval.declOrDeclList = yyvsp[0].declOrDeclList; ;}
    break;

  case 247:
#line 978 "gc_cl.y"
    {
		   yyval.token.type = T_EOF;
		;}
    break;

  case 248:
#line 982 "gc_cl.y"
    { yyval.declOrDeclList = yyvsp[0].declOrDeclList; ;}
    break;

  case 249:
#line 987 "gc_cl.y"
    {
		  clsLexToken *token;

		  token = &yyvsp[-3].token;
		  if(token->type != '{' &&
		     token->type != ',' &&
		     token->type != T_EOF) {
		     gcmASSERT(0);
		     YYERROR;
		  }
		  yyval.declOrDeclList = clParseSubscriptDesignator(Compiler, yyvsp[-4].declOrDeclList, yyvsp[-1].expr, token->type);
		;}
    break;

  case 250:
#line 1000 "gc_cl.y"
    {
		  clsLexToken *token;

		  token = &yyvsp[-2].token;
		  if(token->type != '{' &&
		     token->type != ',' &&
		     token->type != T_EOF) {
		     gcmASSERT(0);
		     YYERROR;
		  }
		  yyval.declOrDeclList = clParseFieldSelectionDesignator(Compiler, yyvsp[-3].declOrDeclList, &yyvsp[0].token, token->type);
		;}
    break;

  case 251:
#line 1016 "gc_cl.y"
    { yyval.statement = yyvsp[0].statement; ;}
    break;

  case 252:
#line 1021 "gc_cl.y"
    { yyval.statement = clParseCompoundStatementAsStatement(Compiler, yyvsp[0].statements); ;}
    break;

  case 253:
#line 1023 "gc_cl.y"
    { yyval.statement = yyvsp[0].statement; ;}
    break;

  case 254:
#line 1028 "gc_cl.y"
    { yyval.statement = yyvsp[0].statement; ;}
    break;

  case 255:
#line 1030 "gc_cl.y"
    { yyval.statement = yyvsp[0].statement; ;}
    break;

  case 256:
#line 1032 "gc_cl.y"
    { yyval.statement = yyvsp[0].statement; ;}
    break;

  case 257:
#line 1034 "gc_cl.y"
    { yyval.statement = yyvsp[0].statement; ;}
    break;

  case 258:
#line 1036 "gc_cl.y"
    { yyval.statement = yyvsp[0].statement; ;}
    break;

  case 259:
#line 1038 "gc_cl.y"
    { yyval.statement = clParseStatementLabel(Compiler, &yyvsp[-1].token); ;}
    break;

  case 260:
#line 1040 "gc_cl.y"
    { yyclearin;
		  yyerrok;
		  yyval.statement = gcvNULL; ;}
    break;

  case 261:
#line 1044 "gc_cl.y"
    { yyclearin;
		  yyerrok;
		  yyval.statement = gcvNULL; ;}
    break;

  case 262:
#line 1051 "gc_cl.y"
    { yyval.statements = gcvNULL; ;}
    break;

  case 263:
#line 1053 "gc_cl.y"
    { clParseCompoundStatementBegin(Compiler); ;}
    break;

  case 264:
#line 1055 "gc_cl.y"
    { yyval.statements = clParseCompoundStatementEnd(Compiler, &yyvsp[-3].token, yyvsp[-1].statements); ;}
    break;

  case 265:
#line 1060 "gc_cl.y"
    { yyval.statement = clParseCompoundStatementNoNewScopeAsStatementNoNewScope(Compiler, yyvsp[0].statements); ;}
    break;

  case 266:
#line 1062 "gc_cl.y"
    { yyval.statement = yyvsp[0].statement; ;}
    break;

  case 267:
#line 1067 "gc_cl.y"
    { yyval.statements = gcvNULL; ;}
    break;

  case 268:
#line 1069 "gc_cl.y"
    { clParseCompoundStatementNoNewScopeBegin(Compiler); ;}
    break;

  case 269:
#line 1071 "gc_cl.y"
    { yyval.statements = clParseCompoundStatementNoNewScopeEnd(Compiler, &yyvsp[-3].token, yyvsp[-1].statements); ;}
    break;

  case 270:
#line 1076 "gc_cl.y"
    { yyval.statements = clParseStatementList(Compiler, yyvsp[0].statement); ;}
    break;

  case 271:
#line 1078 "gc_cl.y"
    { yyval.statements = clParseStatementList2(Compiler, yyvsp[-1].statements, yyvsp[0].statement); ;}
    break;

  case 272:
#line 1083 "gc_cl.y"
    { yyval.statement = gcvNULL; ;}
    break;

  case 273:
#line 1085 "gc_cl.y"
    { yyval.statement = clParseExprAsStatement(Compiler, yyvsp[-1].expr); ;}
    break;

  case 274:
#line 1090 "gc_cl.y"
    { yyval.statement = clParseIfStatement(Compiler, &yyvsp[-4].token, yyvsp[-2].expr, yyvsp[0].ifStatementPair); ;}
    break;

  case 275:
#line 1092 "gc_cl.y"
    { yyval.statement = clParseSwitchStatement(Compiler, &yyvsp[-4].token, yyvsp[-2].expr, yyvsp[0].statement); ;}
    break;

  case 276:
#line 1097 "gc_cl.y"
    { yyval.statements = clParseStatementList(Compiler, yyvsp[0].statement); ;}
    break;

  case 277:
#line 1099 "gc_cl.y"
    { yyval.statements = clParseStatementList2(Compiler, yyvsp[-1].statements, yyvsp[0].statement); ;}
    break;

  case 278:
#line 1104 "gc_cl.y"
    { yyval.statement = yyvsp[0].statement; ;}
    break;

  case 279:
#line 1106 "gc_cl.y"
    { yyval.statement = clParseCaseStatement(Compiler, &yyvsp[-2].token, yyvsp[-1].expr); ;}
    break;

  case 280:
#line 1108 "gc_cl.y"
    { yyval.statement = clParseDefaultStatement(Compiler, &yyvsp[-1].token); ;}
    break;

  case 281:
#line 1113 "gc_cl.y"
    { yyval.statement = gcvNULL; ;}
    break;

  case 282:
#line 1115 "gc_cl.y"
    { clParseSwitchBodyBegin(Compiler); ;}
    break;

  case 283:
#line 1117 "gc_cl.y"
    { yyval.statement = clParseSwitchBodyEnd(Compiler, &yyvsp[-3].token, yyvsp[-1].statements); ;}
    break;

  case 284:
#line 1122 "gc_cl.y"
    { yyval.ifStatementPair = clParseIfSubStatements(Compiler, yyvsp[-2].statement, yyvsp[0].statement); ;}
    break;

  case 285:
#line 1124 "gc_cl.y"
    { yyval.ifStatementPair = clParseIfSubStatements(Compiler, yyvsp[0].statement, gcvNULL); ;}
    break;

  case 286:
#line 1131 "gc_cl.y"
    { clParseWhileStatementBegin(Compiler); ;}
    break;

  case 287:
#line 1133 "gc_cl.y"
    { yyval.statement = clParseWhileStatementEnd(Compiler, &yyvsp[-5].token, yyvsp[-2].expr, yyvsp[0].statement); ;}
    break;

  case 288:
#line 1135 "gc_cl.y"
    { yyval.statement = clParseDoWhileStatement(Compiler, &yyvsp[-6].token, yyvsp[-5].statement, yyvsp[-2].expr); ;}
    break;

  case 289:
#line 1137 "gc_cl.y"
    { clParseForStatementBegin(Compiler); ;}
    break;

  case 290:
#line 1139 "gc_cl.y"
    { yyval.statement = clParseForStatementEnd(Compiler, &yyvsp[-4].token, yyvsp[-2].statement, yyvsp[-1].forExprPair, yyvsp[0].statement); ;}
    break;

  case 291:
#line 1144 "gc_cl.y"
    { yyval.statement = yyvsp[0].statement; ;}
    break;

  case 292:
#line 1146 "gc_cl.y"
    { yyval.statement = yyvsp[0].statement; ;}
    break;

  case 293:
#line 1148 "gc_cl.y"
    { yyclearin;
		  yyerrok;
		  yyval.statement = gcvNULL; ;}
    break;

  case 294:
#line 1155 "gc_cl.y"
    { yyval.expr = yyvsp[0].expr; ;}
    break;

  case 295:
#line 1157 "gc_cl.y"
    { yyval.expr = gcvNULL; ;}
    break;

  case 296:
#line 1162 "gc_cl.y"
    { yyval.forExprPair = clParseForControl(Compiler, yyvsp[-2].expr, gcvNULL); ;}
    break;

  case 297:
#line 1164 "gc_cl.y"
    { yyval.forExprPair = clParseForControl(Compiler, yyvsp[-3].expr, yyvsp[-1].expr); ;}
    break;

  case 298:
#line 1166 "gc_cl.y"
    {
		  clsForExprPair nullPair = {gcvNULL, gcvNULL};
		  yyclearin;
		  yyerrok;
		  yyval.forExprPair = nullPair; ;}
    break;

  case 299:
#line 1172 "gc_cl.y"
    {
		  clsForExprPair nullPair = {gcvNULL, gcvNULL};
		  yyclearin;
		  yyerrok;
		  yyval.forExprPair = nullPair; ;}
    break;

  case 300:
#line 1181 "gc_cl.y"
    { yyval.statement = clParseJumpStatement(Compiler, clvCONTINUE, &yyvsp[-1].token, gcvNULL); ;}
    break;

  case 301:
#line 1183 "gc_cl.y"
    { yyval.statement = clParseJumpStatement(Compiler, clvBREAK, &yyvsp[-1].token, gcvNULL); ;}
    break;

  case 302:
#line 1185 "gc_cl.y"
    { yyval.statement = clParseJumpStatement(Compiler, clvRETURN, &yyvsp[-1].token, gcvNULL); ;}
    break;

  case 303:
#line 1187 "gc_cl.y"
    { yyval.statement = clParseJumpStatement(Compiler, clvRETURN, &yyvsp[-2].token, yyvsp[-1].expr); ;}
    break;

  case 304:
#line 1189 "gc_cl.y"
    { yyval.statement = clParseGotoStatement(Compiler, &yyvsp[-2].token, &yyvsp[-1].token); ;}
    break;

  case 308:
#line 1202 "gc_cl.y"
    { clParseExternalDecl(Compiler, yyvsp[0].statement); ;}
    break;

  case 310:
#line 1208 "gc_cl.y"
    { clParseFuncDef(Compiler, yyvsp[-1].funcName, yyvsp[0].statements); ;}
    break;


    }

/* Line 1000 of yacc.c.  */
#line 4228 "gc_cl_parser.c"

  yyvsp -= yylen;
  yyssp -= yylen;


  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;


  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if YYERROR_VERBOSE
      yyn = yypact[yystate];

      if (YYPACT_NINF < yyn && yyn < YYLAST)
	{
	  YYSIZE_T yysize = 0;
	  int yytype = YYTRANSLATE (yychar);
	  const char* yyprefix;
	  char *yymsg;
	  int yyx;

	  /* Start YYX at -YYN if negative to avoid negative indexes in
	     YYCHECK.  */
	  int yyxbegin = yyn < 0 ? -yyn : 0;

	  /* Stay within bounds of both yycheck and yytname.  */
	  int yychecklim = YYLAST - yyn;
	  int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
	  int yycount = 0;

	  yyprefix = ", expecting ";
	  for (yyx = yyxbegin; yyx < yyxend; ++yyx)
	    if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
	      {
		yysize += yystrlen (yyprefix) + yystrlen (yytname [yyx]);
		yycount += 1;
		if (yycount == 5)
		  {
		    yysize = 0;
		    break;
		  }
	      }
	  yysize += (sizeof ("syntax error, unexpected ")
		     + yystrlen (yytname[yytype]));
	  yymsg = (char *) YYSTACK_ALLOC (yysize);
	  if (yymsg != 0)
	    {
	      char *yyp = yystpcpy (yymsg, "syntax error, unexpected ");
	      yyp = yystpcpy (yyp, yytname[yytype]);

	      if (yycount < 5)
		{
		  yyprefix = ", expecting ";
		  for (yyx = yyxbegin; yyx < yyxend; ++yyx)
		    if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
		      {
			yyp = yystpcpy (yyp, yyprefix);
			yyp = yystpcpy (yyp, yytname[yyx]);
			yyprefix = " or ";
		      }
		}
	      yyerror (yymsg);
	      YYSTACK_FREE (yymsg);
	    }
	  else
	    yyerror ("syntax error; also virtual memory exhausted");
	}
      else
#endif /* YYERROR_VERBOSE */
	yyerror ("syntax error");
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
	 error, discard it.  */

      if (yychar <= YYEOF)
        {
          /* If at end of input, pop the error token,
	     then the rest of the stack, then return failure.  */
	  if (yychar == YYEOF)
	     for (;;)
	       {
		 YYPOPSTACK;
		 if (yyssp == yyss)
		   YYABORT;
		 YYDSYMPRINTF ("Error: popping", yystos[*yyssp], yyvsp, yylsp);
		 yydestruct (yystos[*yyssp], yyvsp);
	       }
        }
      else
	{
	  YYDSYMPRINTF ("Error: discarding", yytoken, &yylval, &yylloc);
	  yydestruct (yytoken, &yylval);
	  yychar = YYEMPTY;

	}
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

#ifdef __GNUC__
  /* Pacify GCC when the user code never invokes YYERROR and the label
     yyerrorlab therefore never appears in user code.  */
  if (0)
     goto yyerrorlab;
#endif

  yyvsp -= yylen;
  yyssp -= yylen;
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;	/* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (yyn != YYPACT_NINF)
	{
	  yyn += YYTERROR;
	  if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
	    {
	      yyn = yytable[yyn];
	      if (0 < yyn)
		break;
	    }
	}

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
	YYABORT;

      YYDSYMPRINTF ("Error: popping", yystos[*yyssp], yyvsp, yylsp);
      yydestruct (yystos[yystate], yyvsp);
      YYPOPSTACK;
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  YYDPRINTF ((stderr, "Shifting error token, "));

  *++yyvsp = yylval;


  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#ifndef yyoverflow
/*----------------------------------------------.
| yyoverflowlab -- parser overflow comes here.  |
`----------------------------------------------*/
yyoverflowlab:
  yyerror ("parser stack overflow");
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
  return yyresult;
}


#line 1211 "gc_cl.y"



