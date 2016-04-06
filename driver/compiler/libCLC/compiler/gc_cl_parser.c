/****************************************************************************
*
*    Copyright (c) 2005 - 2016 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


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
     T_BOOL32 = 271,
     T_HALF = 272,
     T_HALF2 = 273,
     T_HALF3 = 274,
     T_HALF4 = 275,
     T_HALF8 = 276,
     T_HALF16 = 277,
     T_HALF32 = 278,
     T_FLOAT = 279,
     T_FLOAT2 = 280,
     T_FLOAT3 = 281,
     T_FLOAT4 = 282,
     T_FLOAT8 = 283,
     T_FLOAT16 = 284,
     T_DOUBLE = 285,
     T_DOUBLE2 = 286,
     T_DOUBLE3 = 287,
     T_DOUBLE4 = 288,
     T_DOUBLE8 = 289,
     T_DOUBLE16 = 290,
     T_QUAD = 291,
     T_QUAD2 = 292,
     T_QUAD3 = 293,
     T_QUAD4 = 294,
     T_QUAD8 = 295,
     T_QUAD16 = 296,
     T_CHAR = 297,
     T_CHAR2 = 298,
     T_CHAR3 = 299,
     T_CHAR4 = 300,
     T_CHAR8 = 301,
     T_CHAR16 = 302,
     T_CHAR32 = 303,
     T_UCHAR = 304,
     T_UCHAR2 = 305,
     T_UCHAR3 = 306,
     T_UCHAR4 = 307,
     T_UCHAR8 = 308,
     T_UCHAR16 = 309,
     T_UCHAR32 = 310,
     T_SHORT = 311,
     T_SHORT2 = 312,
     T_SHORT3 = 313,
     T_SHORT4 = 314,
     T_SHORT8 = 315,
     T_SHORT16 = 316,
     T_SHORT32 = 317,
     T_USHORT = 318,
     T_USHORT2 = 319,
     T_USHORT3 = 320,
     T_USHORT4 = 321,
     T_USHORT8 = 322,
     T_USHORT16 = 323,
     T_USHORT32 = 324,
     T_INT = 325,
     T_INT2 = 326,
     T_INT3 = 327,
     T_INT4 = 328,
     T_INT8 = 329,
     T_INT16 = 330,
     T_UINT = 331,
     T_UINT2 = 332,
     T_UINT3 = 333,
     T_UINT4 = 334,
     T_UINT8 = 335,
     T_UINT16 = 336,
     T_LONG = 337,
     T_LONG2 = 338,
     T_LONG3 = 339,
     T_LONG4 = 340,
     T_LONG8 = 341,
     T_LONG16 = 342,
     T_ULONG = 343,
     T_ULONG2 = 344,
     T_ULONG3 = 345,
     T_ULONG4 = 346,
     T_ULONG8 = 347,
     T_ULONG16 = 348,
     T_SAMPLER_T = 349,
     T_IMAGE1D_T = 350,
     T_IMAGE1D_ARRAY_T = 351,
     T_IMAGE1D_BUFFER_T = 352,
     T_IMAGE2D_ARRAY_T = 353,
     T_IMAGE2D_T = 354,
     T_IMAGE3D_T = 355,
     T_IMAGE2D_PTR_T = 356,
     T_IMAGE2D_DYNAMIC_ARRAY_T = 357,
     T_SIZE_T = 358,
     T_EVENT_T = 359,
     T_PTRDIFF_T = 360,
     T_INTPTR_T = 361,
     T_UINTPTR_T = 362,
     T_GENTYPE = 363,
     T_F_GENTYPE = 364,
     T_IU_GENTYPE = 365,
     T_I_GENTYPE = 366,
     T_U_GENTYPE = 367,
     T_SIU_GENTYPE = 368,
     T_BOOL_PACKED = 369,
     T_BOOL2_PACKED = 370,
     T_BOOL3_PACKED = 371,
     T_BOOL4_PACKED = 372,
     T_BOOL8_PACKED = 373,
     T_BOOL16_PACKED = 374,
     T_BOOL32_PACKED = 375,
     T_CHAR_PACKED = 376,
     T_CHAR2_PACKED = 377,
     T_CHAR3_PACKED = 378,
     T_CHAR4_PACKED = 379,
     T_CHAR8_PACKED = 380,
     T_CHAR16_PACKED = 381,
     T_CHAR32_PACKED = 382,
     T_UCHAR_PACKED = 383,
     T_UCHAR2_PACKED = 384,
     T_UCHAR3_PACKED = 385,
     T_UCHAR4_PACKED = 386,
     T_UCHAR8_PACKED = 387,
     T_UCHAR16_PACKED = 388,
     T_UCHAR32_PACKED = 389,
     T_SHORT_PACKED = 390,
     T_SHORT2_PACKED = 391,
     T_SHORT3_PACKED = 392,
     T_SHORT4_PACKED = 393,
     T_SHORT8_PACKED = 394,
     T_SHORT16_PACKED = 395,
     T_SHORT32_PACKED = 396,
     T_USHORT_PACKED = 397,
     T_USHORT2_PACKED = 398,
     T_USHORT3_PACKED = 399,
     T_USHORT4_PACKED = 400,
     T_USHORT8_PACKED = 401,
     T_USHORT16_PACKED = 402,
     T_USHORT32_PACKED = 403,
     T_HALF_PACKED = 404,
     T_HALF2_PACKED = 405,
     T_HALF3_PACKED = 406,
     T_HALF4_PACKED = 407,
     T_HALF8_PACKED = 408,
     T_HALF16_PACKED = 409,
     T_HALF32_PACKED = 410,
     T_GENTYPE_PACKED = 411,
     T_FLOATNXM = 412,
     T_DOUBLENXM = 413,
     T_BUILTIN_DATA_TYPE = 414,
     T_RESERVED_DATA_TYPE = 415,
     T_VIV_PACKED_DATA_TYPE = 416,
     T_IDENTIFIER = 417,
     T_TYPE_NAME = 418,
     T_FLOATCONSTANT = 419,
     T_UINTCONSTANT = 420,
     T_INTCONSTANT = 421,
     T_BOOLCONSTANT = 422,
     T_CHARCONSTANT = 423,
     T_STRING_LITERAL = 424,
     T_FIELD_SELECTION = 425,
     T_LSHIFT_OP = 426,
     T_RSHIFT_OP = 427,
     T_INC_OP = 428,
     T_DEC_OP = 429,
     T_LE_OP = 430,
     T_GE_OP = 431,
     T_EQ_OP = 432,
     T_NE_OP = 433,
     T_AND_OP = 434,
     T_OR_OP = 435,
     T_XOR_OP = 436,
     T_MUL_ASSIGN = 437,
     T_DIV_ASSIGN = 438,
     T_ADD_ASSIGN = 439,
     T_MOD_ASSIGN = 440,
     T_LEFT_ASSIGN = 441,
     T_RIGHT_ASSIGN = 442,
     T_AND_ASSIGN = 443,
     T_XOR_ASSIGN = 444,
     T_OR_ASSIGN = 445,
     T_SUB_ASSIGN = 446,
     T_STRUCT_UNION_PTR = 447,
     T_INITIALIZER_END = 448,
     T_BREAK = 449,
     T_CONTINUE = 450,
     T_RETURN = 451,
     T_GOTO = 452,
     T_WHILE = 453,
     T_FOR = 454,
     T_DO = 455,
     T_ELSE = 456,
     T_IF = 457,
     T_SWITCH = 458,
     T_CASE = 459,
     T_DEFAULT = 460,
     T_CONST = 461,
     T_RESTRICT = 462,
     T_VOLATILE = 463,
     T_STATIC = 464,
     T_EXTERN = 465,
     T_CONSTANT = 466,
     T_GLOBAL = 467,
     T_LOCAL = 468,
     T_PRIVATE = 469,
     T_KERNEL = 470,
     T_UNIFORM = 471,
     T_READ_ONLY = 472,
     T_WRITE_ONLY = 473,
     T_PACKED = 474,
     T_ALIGNED = 475,
     T_ENDIAN = 476,
     T_VEC_TYPE_HINT = 477,
     T_ATTRIBUTE__ = 478,
     T_REQD_WORK_GROUP_SIZE = 479,
     T_WORK_GROUP_SIZE_HINT = 480,
     T_ALWAYS_INLINE = 481,
     T_UNSIGNED = 482,
     T_STRUCT = 483,
     T_UNION = 484,
     T_TYPEDEF = 485,
     T_ENUM = 486,
     T_INLINE = 487,
     T_SIZEOF = 488,
     T_TYPE_CAST = 489,
     T_VEC_STEP = 490,
     T_VERY_LAST_TERMINAL = 491
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
#define T_BOOL32 271
#define T_HALF 272
#define T_HALF2 273
#define T_HALF3 274
#define T_HALF4 275
#define T_HALF8 276
#define T_HALF16 277
#define T_HALF32 278
#define T_FLOAT 279
#define T_FLOAT2 280
#define T_FLOAT3 281
#define T_FLOAT4 282
#define T_FLOAT8 283
#define T_FLOAT16 284
#define T_DOUBLE 285
#define T_DOUBLE2 286
#define T_DOUBLE3 287
#define T_DOUBLE4 288
#define T_DOUBLE8 289
#define T_DOUBLE16 290
#define T_QUAD 291
#define T_QUAD2 292
#define T_QUAD3 293
#define T_QUAD4 294
#define T_QUAD8 295
#define T_QUAD16 296
#define T_CHAR 297
#define T_CHAR2 298
#define T_CHAR3 299
#define T_CHAR4 300
#define T_CHAR8 301
#define T_CHAR16 302
#define T_CHAR32 303
#define T_UCHAR 304
#define T_UCHAR2 305
#define T_UCHAR3 306
#define T_UCHAR4 307
#define T_UCHAR8 308
#define T_UCHAR16 309
#define T_UCHAR32 310
#define T_SHORT 311
#define T_SHORT2 312
#define T_SHORT3 313
#define T_SHORT4 314
#define T_SHORT8 315
#define T_SHORT16 316
#define T_SHORT32 317
#define T_USHORT 318
#define T_USHORT2 319
#define T_USHORT3 320
#define T_USHORT4 321
#define T_USHORT8 322
#define T_USHORT16 323
#define T_USHORT32 324
#define T_INT 325
#define T_INT2 326
#define T_INT3 327
#define T_INT4 328
#define T_INT8 329
#define T_INT16 330
#define T_UINT 331
#define T_UINT2 332
#define T_UINT3 333
#define T_UINT4 334
#define T_UINT8 335
#define T_UINT16 336
#define T_LONG 337
#define T_LONG2 338
#define T_LONG3 339
#define T_LONG4 340
#define T_LONG8 341
#define T_LONG16 342
#define T_ULONG 343
#define T_ULONG2 344
#define T_ULONG3 345
#define T_ULONG4 346
#define T_ULONG8 347
#define T_ULONG16 348
#define T_SAMPLER_T 349
#define T_IMAGE1D_T 350
#define T_IMAGE1D_ARRAY_T 351
#define T_IMAGE1D_BUFFER_T 352
#define T_IMAGE2D_ARRAY_T 353
#define T_IMAGE2D_T 354
#define T_IMAGE3D_T 355
#define T_IMAGE2D_PTR_T 356
#define T_IMAGE2D_DYNAMIC_ARRAY_T 357
#define T_SIZE_T 358
#define T_EVENT_T 359
#define T_PTRDIFF_T 360
#define T_INTPTR_T 361
#define T_UINTPTR_T 362
#define T_GENTYPE 363
#define T_F_GENTYPE 364
#define T_IU_GENTYPE 365
#define T_I_GENTYPE 366
#define T_U_GENTYPE 367
#define T_SIU_GENTYPE 368
#define T_BOOL_PACKED 369
#define T_BOOL2_PACKED 370
#define T_BOOL3_PACKED 371
#define T_BOOL4_PACKED 372
#define T_BOOL8_PACKED 373
#define T_BOOL16_PACKED 374
#define T_BOOL32_PACKED 375
#define T_CHAR_PACKED 376
#define T_CHAR2_PACKED 377
#define T_CHAR3_PACKED 378
#define T_CHAR4_PACKED 379
#define T_CHAR8_PACKED 380
#define T_CHAR16_PACKED 381
#define T_CHAR32_PACKED 382
#define T_UCHAR_PACKED 383
#define T_UCHAR2_PACKED 384
#define T_UCHAR3_PACKED 385
#define T_UCHAR4_PACKED 386
#define T_UCHAR8_PACKED 387
#define T_UCHAR16_PACKED 388
#define T_UCHAR32_PACKED 389
#define T_SHORT_PACKED 390
#define T_SHORT2_PACKED 391
#define T_SHORT3_PACKED 392
#define T_SHORT4_PACKED 393
#define T_SHORT8_PACKED 394
#define T_SHORT16_PACKED 395
#define T_SHORT32_PACKED 396
#define T_USHORT_PACKED 397
#define T_USHORT2_PACKED 398
#define T_USHORT3_PACKED 399
#define T_USHORT4_PACKED 400
#define T_USHORT8_PACKED 401
#define T_USHORT16_PACKED 402
#define T_USHORT32_PACKED 403
#define T_HALF_PACKED 404
#define T_HALF2_PACKED 405
#define T_HALF3_PACKED 406
#define T_HALF4_PACKED 407
#define T_HALF8_PACKED 408
#define T_HALF16_PACKED 409
#define T_HALF32_PACKED 410
#define T_GENTYPE_PACKED 411
#define T_FLOATNXM 412
#define T_DOUBLENXM 413
#define T_BUILTIN_DATA_TYPE 414
#define T_RESERVED_DATA_TYPE 415
#define T_VIV_PACKED_DATA_TYPE 416
#define T_IDENTIFIER 417
#define T_TYPE_NAME 418
#define T_FLOATCONSTANT 419
#define T_UINTCONSTANT 420
#define T_INTCONSTANT 421
#define T_BOOLCONSTANT 422
#define T_CHARCONSTANT 423
#define T_STRING_LITERAL 424
#define T_FIELD_SELECTION 425
#define T_LSHIFT_OP 426
#define T_RSHIFT_OP 427
#define T_INC_OP 428
#define T_DEC_OP 429
#define T_LE_OP 430
#define T_GE_OP 431
#define T_EQ_OP 432
#define T_NE_OP 433
#define T_AND_OP 434
#define T_OR_OP 435
#define T_XOR_OP 436
#define T_MUL_ASSIGN 437
#define T_DIV_ASSIGN 438
#define T_ADD_ASSIGN 439
#define T_MOD_ASSIGN 440
#define T_LEFT_ASSIGN 441
#define T_RIGHT_ASSIGN 442
#define T_AND_ASSIGN 443
#define T_XOR_ASSIGN 444
#define T_OR_ASSIGN 445
#define T_SUB_ASSIGN 446
#define T_STRUCT_UNION_PTR 447
#define T_INITIALIZER_END 448
#define T_BREAK 449
#define T_CONTINUE 450
#define T_RETURN 451
#define T_GOTO 452
#define T_WHILE 453
#define T_FOR 454
#define T_DO 455
#define T_ELSE 456
#define T_IF 457
#define T_SWITCH 458
#define T_CASE 459
#define T_DEFAULT 460
#define T_CONST 461
#define T_RESTRICT 462
#define T_VOLATILE 463
#define T_STATIC 464
#define T_EXTERN 465
#define T_CONSTANT 466
#define T_GLOBAL 467
#define T_LOCAL 468
#define T_PRIVATE 469
#define T_KERNEL 470
#define T_UNIFORM 471
#define T_READ_ONLY 472
#define T_WRITE_ONLY 473
#define T_PACKED 474
#define T_ALIGNED 475
#define T_ENDIAN 476
#define T_VEC_TYPE_HINT 477
#define T_ATTRIBUTE__ 478
#define T_REQD_WORK_GROUP_SIZE 479
#define T_WORK_GROUP_SIZE_HINT 480
#define T_ALWAYS_INLINE 481
#define T_UNSIGNED 482
#define T_STRUCT 483
#define T_UNION 484
#define T_TYPEDEF 485
#define T_ENUM 486
#define T_INLINE 487
#define T_SIZEOF 488
#define T_TYPE_CAST 489
#define T_VEC_STEP 490
#define T_VERY_LAST_TERMINAL 491




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
#line 595 "gc_cl_parser.c"
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif



/* Copy the second part of user declarations.  */


/* Line 214 of yacc.c.  */
#line 607 "gc_cl_parser.c"

#if !defined (yyoverflow) || YYERROR_VERBOSE

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
#  define YYSTACK_ALLOC clMalloc
#  define YYSTACK_FREE clFree
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
#define YYFINAL  105
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   3353

/* YYNTOKENS -- Number of terminals. */
#define YYNTOKENS  261
/* YYNNTS -- Number of nonterminals. */
#define YYNNTS  110
/* YYNRULES -- Number of rules. */
#define YYNRULES  314
/* YYNRULES -- Number of states. */
#define YYNSTATES  522

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   491

#define YYTRANSLATE(YYX) 						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const unsigned short yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   248,     2,     2,     2,   254,   259,     2,
     237,   238,   252,   251,   244,   249,   243,   253,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,   245,   247,
     255,   246,   256,   260,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,   239,     2,   240,   258,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   241,   257,   242,   250,     2,     2,     2,
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
     225,   226,   227,   228,   229,   230,   231,   232,   233,   234,
     235,   236
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
     443,   446,   448,   451,   454,   458,   460,   463,   465,   469,
     470,   472,   476,   481,   483,   484,   488,   493,   499,   500,
     508,   509,   518,   520,   524,   529,   530,   537,   538,   546,
     547,   555,   556,   558,   559,   561,   562,   567,   572,   574,
     579,   588,   597,   599,   604,   606,   608,   611,   613,   615,
     617,   619,   621,   623,   625,   627,   629,   631,   633,   635,
     637,   639,   641,   643,   645,   647,   649,   651,   653,   655,
     657,   659,   661,   663,   665,   667,   669,   671,   673,   675,
     677,   679,   681,   683,   685,   687,   689,   691,   692,   700,
     701,   708,   709,   717,   718,   725,   727,   730,   734,   736,
     740,   743,   747,   749,   751,   755,   759,   762,   767,   768,
     771,   773,   774,   778,   782,   785,   787,   789,   791,   793,
     795,   797,   799,   801,   804,   807,   810,   813,   814,   819,
     821,   823,   826,   827,   832,   834,   837,   839,   842,   848,
     854,   856,   859,   861,   865,   868,   871,   872,   877,   881,
     883,   884,   891,   899,   900,   906,   909,   912,   916,   918,
     919,   923,   928,   932,   937,   940,   943,   946,   950,   954,
     956,   959,   961,   963,   965
};

/* YYRHS -- A `-1'-separated list of the rules' RHS. */
static const short yyrhs[] =
{
     368,     0,    -1,   162,    -1,   169,    -1,   263,   169,    -1,
     262,    -1,   165,    -1,   166,    -1,   164,    -1,   167,    -1,
     168,    -1,   263,    -1,   237,   290,   238,    -1,   264,    -1,
     265,   239,   266,   240,    -1,   267,    -1,   265,   243,   170,
      -1,   265,   192,   170,    -1,   265,   173,    -1,   265,   174,
      -1,   290,    -1,   268,   238,    -1,   270,     4,   238,    -1,
     270,   238,    -1,   270,   288,    -1,   268,   244,   288,    -1,
     237,   324,   238,    -1,   237,   324,   306,   238,    -1,   162,
     237,    -1,   269,   241,    -1,   274,    -1,    -1,   269,   273,
     272,    -1,   271,   290,   242,    -1,   271,   290,   193,    -1,
     265,    -1,   173,   274,    -1,   174,   274,    -1,   235,   237,
     274,   238,    -1,   235,   269,    -1,   233,   274,    -1,   233,
     269,    -1,   275,   272,    -1,   251,    -1,   249,    -1,   248,
      -1,   250,    -1,   259,    -1,   252,    -1,   272,    -1,   276,
     252,   272,    -1,   276,   253,   272,    -1,   276,   254,   272,
      -1,   276,    -1,   277,   251,   276,    -1,   277,   249,   276,
      -1,   277,    -1,   278,   171,   277,    -1,   278,   172,   277,
      -1,   278,    -1,   279,   255,   278,    -1,   279,   256,   278,
      -1,   279,   175,   278,    -1,   279,   176,   278,    -1,   279,
      -1,   280,   177,   279,    -1,   280,   178,   279,    -1,   280,
      -1,   281,   259,   280,    -1,   281,    -1,   282,   258,   281,
      -1,   282,    -1,   283,   257,   282,    -1,   283,    -1,   284,
     179,   283,    -1,   284,    -1,   285,   181,   284,    -1,   285,
      -1,   286,   180,   285,    -1,   286,    -1,   286,   260,   290,
     245,   288,    -1,   287,    -1,   274,   289,   288,    -1,   246,
      -1,   182,    -1,   183,    -1,   185,    -1,   184,    -1,   191,
      -1,   186,    -1,   187,    -1,   188,    -1,   189,    -1,   190,
      -1,   288,    -1,   290,   244,   288,    -1,   287,    -1,   328,
     162,    -1,   231,   162,    -1,   231,   318,   162,   241,   294,
     242,    -1,   231,   318,   162,   241,   294,   193,    -1,   231,
     162,   241,   294,   242,    -1,   231,   162,   241,   294,   193,
      -1,   231,   318,   241,   294,   242,    -1,   231,   318,   241,
     294,   193,    -1,   231,   241,   294,   242,    -1,   231,   241,
     294,   193,    -1,   295,    -1,   294,   244,   295,    -1,   162,
      -1,   162,   246,   291,    -1,   311,   247,    -1,   293,   320,
     247,    -1,   292,   247,    -1,   329,   247,    -1,   298,   238,
      -1,   300,    -1,   299,    -1,   300,   302,    -1,   299,   244,
     302,    -1,   215,   320,   324,   308,   237,    -1,   210,   215,
     320,   324,   308,   237,    -1,   324,   308,   237,    -1,   318,
     324,   308,   237,    -1,   232,   324,   308,   237,    -1,   326,
     308,    -1,   326,   308,   310,    -1,   301,    -1,   303,   301,
      -1,   304,    -1,   303,   304,    -1,   305,   301,    -1,   305,
     303,   301,    -1,   305,   304,    -1,   305,   303,   304,    -1,
     217,    -1,   218,    -1,   326,    -1,   326,   239,   291,   240,
      -1,   325,    -1,   325,   305,    -1,   252,    -1,   252,   306,
      -1,   252,   305,    -1,   252,   305,   306,    -1,   162,    -1,
     306,   162,    -1,   307,    -1,   237,   307,   238,    -1,    -1,
     291,    -1,   239,   309,   240,    -1,   310,   239,   309,   240,
      -1,   315,    -1,    -1,   230,   312,   315,    -1,   311,   244,
     308,   320,    -1,   311,   244,   308,   310,   320,    -1,    -1,
     311,   244,   308,   320,   246,   313,   339,    -1,    -1,   311,
     244,   308,   310,   320,   246,   314,   339,    -1,   297,    -1,
     324,   308,   320,    -1,   324,   308,   310,   320,    -1,    -1,
     324,   308,   320,   246,   316,   339,    -1,    -1,   324,   308,
     310,   320,   246,   317,   339,    -1,    -1,   223,   237,   237,
     319,   321,   238,   238,    -1,    -1,   318,    -1,    -1,   323,
      -1,    -1,   321,   244,   322,   323,    -1,   221,   237,   162,
     238,    -1,   219,    -1,   222,   237,   159,   238,    -1,   224,
     237,   291,   244,   291,   244,   291,   238,    -1,   225,   237,
     291,   244,   291,   244,   291,   238,    -1,   220,    -1,   220,
     237,   291,   238,    -1,   226,    -1,   326,    -1,   305,   326,
      -1,   206,    -1,   207,    -1,   208,    -1,   211,    -1,   212,
      -1,   213,    -1,   214,    -1,   209,    -1,   210,    -1,   216,
      -1,   327,    -1,   329,    -1,   292,    -1,   293,    -1,     4,
      -1,    24,    -1,    70,    -1,    10,    -1,   157,    -1,   158,
      -1,   159,    -1,   160,    -1,   161,    -1,    95,    -1,    96,
      -1,    97,    -1,    98,    -1,    99,    -1,   101,    -1,   102,
      -1,   100,    -1,    94,    -1,   105,    -1,   106,    -1,   107,
      -1,   103,    -1,   104,    -1,   163,    -1,   228,    -1,   229,
      -1,    -1,   328,   162,   241,   330,   334,   242,   320,    -1,
      -1,   328,   241,   331,   334,   242,   320,    -1,    -1,   328,
     318,   162,   241,   332,   334,   242,    -1,    -1,   328,   318,
     241,   333,   334,   242,    -1,   335,    -1,   334,   335,    -1,
     324,   336,   247,    -1,   337,    -1,   336,   244,   337,    -1,
     308,   320,    -1,   308,   310,   320,    -1,   241,    -1,   288,
      -1,   338,   340,   242,    -1,   338,   340,   193,    -1,   341,
     339,    -1,   340,   244,   341,   339,    -1,    -1,   342,   246,
      -1,   344,    -1,    -1,   342,   343,   344,    -1,   239,   291,
     240,    -1,   243,   170,    -1,   296,    -1,   348,    -1,   347,
      -1,   345,    -1,   354,    -1,   355,    -1,   361,    -1,   367,
      -1,   162,   245,    -1,     1,   247,    -1,     1,   242,    -1,
     241,   242,    -1,    -1,   241,   349,   353,   242,    -1,   351,
      -1,   347,    -1,   241,   242,    -1,    -1,   241,   352,   353,
     242,    -1,   346,    -1,   353,   346,    -1,   247,    -1,   290,
     247,    -1,   202,   237,   290,   238,   360,    -1,   203,   237,
     266,   238,   358,    -1,   357,    -1,   356,   357,    -1,   346,
      -1,   204,   291,   245,    -1,   205,   245,    -1,   241,   242,
      -1,    -1,   241,   359,   356,   242,    -1,   346,   201,   346,
      -1,   346,    -1,    -1,   198,   362,   237,   290,   238,   350,
      -1,   200,   346,   198,   237,   290,   238,   247,    -1,    -1,
     199,   363,   364,   366,   350,    -1,   237,   354,    -1,   237,
     345,    -1,   237,     1,   247,    -1,   290,    -1,    -1,   365,
     247,   238,    -1,   365,   247,   290,   238,    -1,     1,   247,
     238,    -1,     1,   247,   290,   238,    -1,   195,   247,    -1,
     194,   247,    -1,   196,   247,    -1,   196,   290,   247,    -1,
     197,   162,   247,    -1,   369,    -1,   368,   369,    -1,   370,
      -1,   296,    -1,   247,    -1,   297,   351,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const unsigned short yyrline[] =
{
       0,   195,   195,   204,   206,   211,   213,   215,   217,   219,
     221,   223,   225,   230,   232,   234,   236,   238,   240,   242,
     247,   252,   254,   256,   261,   263,   268,   270,   275,   280,
     289,   292,   291,   300,   304,   311,   313,   315,   317,   319,
     321,   323,   325,   330,   332,   334,   336,   338,   340,   345,
     349,   351,   353,   358,   360,   362,   367,   369,   371,   376,
     378,   380,   382,   384,   389,   391,   393,   398,   400,   405,
     407,   412,   414,   419,   421,   426,   428,   433,   435,   440,
     442,   447,   449,   454,   456,   458,   460,   462,   464,   466,
     468,   470,   472,   474,   479,   481,   494,   499,   501,   506,
     510,   513,   517,   520,   524,   527,   531,   537,   544,   549,
     551,   556,   558,   560,   562,   567,   572,   574,   579,   581,
     586,   588,   590,   592,   594,   601,   603,   608,   610,   612,
     614,   616,   618,   620,   622,   627,   629,   634,   636,   641,
     646,   651,   653,   655,   657,   661,   663,   670,   672,   677,
     678,   683,   685,   690,   693,   692,   701,   703,   706,   705,
     710,   709,   717,   719,   721,   724,   723,   728,   727,   737,
     736,   743,   744,   749,   750,   753,   752,   759,   761,   763,
     765,   767,   769,   771,   773,   778,   780,   785,   787,   789,
     791,   793,   795,   797,   799,   801,   803,   808,   810,   812,
     814,   819,   821,   823,   825,   827,   829,   831,   833,   835,
     837,   839,   841,   843,   845,   847,   849,   851,   853,   855,
     857,   859,   861,   863,   865,   869,   871,   877,   876,   881,
     880,   885,   884,   889,   888,   895,   899,   912,   919,   921,
     926,   928,   933,   955,   957,   959,   964,   966,   971,   972,
     985,   988,   987,   996,  1009,  1025,  1030,  1032,  1037,  1039,
    1041,  1043,  1045,  1047,  1049,  1053,  1060,  1063,  1062,  1069,
    1071,  1076,  1079,  1078,  1085,  1087,  1092,  1094,  1099,  1101,
    1106,  1108,  1113,  1115,  1117,  1122,  1125,  1124,  1131,  1133,
    1141,  1140,  1144,  1147,  1146,  1153,  1155,  1157,  1164,  1167,
    1171,  1173,  1175,  1181,  1190,  1192,  1194,  1196,  1198,  1205,
    1206,  1210,  1211,  1213,  1217
};
#endif

#if YYDEBUG || YYERROR_VERBOSE
/* YYTNME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals. */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "T_VERY_FIRST_TERMINAL", "T_VOID",
  "T_MAT2", "T_MAT3", "T_MAT4", "T_MAT8", "T_MAT16", "T_BOOL", "T_BOOL2",
  "T_BOOL3", "T_BOOL4", "T_BOOL8", "T_BOOL16", "T_BOOL32", "T_HALF",
  "T_HALF2", "T_HALF3", "T_HALF4", "T_HALF8", "T_HALF16", "T_HALF32",
  "T_FLOAT", "T_FLOAT2", "T_FLOAT3", "T_FLOAT4", "T_FLOAT8", "T_FLOAT16",
  "T_DOUBLE", "T_DOUBLE2", "T_DOUBLE3", "T_DOUBLE4", "T_DOUBLE8",
  "T_DOUBLE16", "T_QUAD", "T_QUAD2", "T_QUAD3", "T_QUAD4", "T_QUAD8",
  "T_QUAD16", "T_CHAR", "T_CHAR2", "T_CHAR3", "T_CHAR4", "T_CHAR8",
  "T_CHAR16", "T_CHAR32", "T_UCHAR", "T_UCHAR2", "T_UCHAR3", "T_UCHAR4",
  "T_UCHAR8", "T_UCHAR16", "T_UCHAR32", "T_SHORT", "T_SHORT2", "T_SHORT3",
  "T_SHORT4", "T_SHORT8", "T_SHORT16", "T_SHORT32", "T_USHORT",
  "T_USHORT2", "T_USHORT3", "T_USHORT4", "T_USHORT8", "T_USHORT16",
  "T_USHORT32", "T_INT", "T_INT2", "T_INT3", "T_INT4", "T_INT8", "T_INT16",
  "T_UINT", "T_UINT2", "T_UINT3", "T_UINT4", "T_UINT8", "T_UINT16",
  "T_LONG", "T_LONG2", "T_LONG3", "T_LONG4", "T_LONG8", "T_LONG16",
  "T_ULONG", "T_ULONG2", "T_ULONG3", "T_ULONG4", "T_ULONG8", "T_ULONG16",
  "T_SAMPLER_T", "T_IMAGE1D_T", "T_IMAGE1D_ARRAY_T", "T_IMAGE1D_BUFFER_T",
  "T_IMAGE2D_ARRAY_T", "T_IMAGE2D_T", "T_IMAGE3D_T", "T_IMAGE2D_PTR_T",
  "T_IMAGE2D_DYNAMIC_ARRAY_T", "T_SIZE_T", "T_EVENT_T", "T_PTRDIFF_T",
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
  "T_GENTYPE_PACKED", "T_FLOATNXM", "T_DOUBLENXM", "T_BUILTIN_DATA_TYPE",
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
  "T_UNIFORM", "T_READ_ONLY", "T_WRITE_ONLY", "T_PACKED", "T_ALIGNED",
  "T_ENDIAN", "T_VEC_TYPE_HINT", "T_ATTRIBUTE__", "T_REQD_WORK_GROUP_SIZE",
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
     475,   476,   477,   478,   479,   480,   481,   482,   483,   484,
     485,   486,   487,   488,   489,   490,   491,    40,    41,    91,
      93,   123,   125,    46,    44,    58,    61,    59,    33,    45,
     126,    43,    42,    47,    37,    60,    62,   124,    94,    38,
      63
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const unsigned short yyr1[] =
{
       0,   261,   262,   263,   263,   264,   264,   264,   264,   264,
     264,   264,   264,   265,   265,   265,   265,   265,   265,   265,
     266,   267,   267,   267,   268,   268,   269,   269,   270,   271,
     272,   273,   272,   272,   272,   274,   274,   274,   274,   274,
     274,   274,   274,   275,   275,   275,   275,   275,   275,   276,
     276,   276,   276,   277,   277,   277,   278,   278,   278,   279,
     279,   279,   279,   279,   280,   280,   280,   281,   281,   282,
     282,   283,   283,   284,   284,   285,   285,   286,   286,   287,
     287,   288,   288,   289,   289,   289,   289,   289,   289,   289,
     289,   289,   289,   289,   290,   290,   291,   292,   292,   293,
     293,   293,   293,   293,   293,   293,   293,   294,   294,   295,
     295,   296,   296,   296,   296,   297,   298,   298,   299,   299,
     300,   300,   300,   300,   300,   301,   301,   302,   302,   302,
     302,   302,   302,   302,   302,   303,   303,   304,   304,   305,
     305,   306,   306,   306,   306,   307,   307,   308,   308,   309,
     309,   310,   310,   311,   312,   311,   311,   311,   313,   311,
     314,   311,   315,   315,   315,   316,   315,   317,   315,   319,
     318,   320,   320,   321,   321,   322,   321,   323,   323,   323,
     323,   323,   323,   323,   323,   324,   324,   325,   325,   325,
     325,   325,   325,   325,   325,   325,   325,   326,   326,   326,
     326,   327,   327,   327,   327,   327,   327,   327,   327,   327,
     327,   327,   327,   327,   327,   327,   327,   327,   327,   327,
     327,   327,   327,   327,   327,   328,   328,   330,   329,   331,
     329,   332,   329,   333,   329,   334,   334,   335,   336,   336,
     337,   337,   338,   339,   339,   339,   340,   340,   341,   341,
     342,   343,   342,   344,   344,   345,   346,   346,   347,   347,
     347,   347,   347,   347,   347,   347,   348,   349,   348,   350,
     350,   351,   352,   351,   353,   353,   354,   354,   355,   355,
     356,   356,   357,   357,   357,   358,   359,   358,   360,   360,
     362,   361,   361,   363,   361,   364,   364,   364,   365,   365,
     366,   366,   366,   366,   367,   367,   367,   367,   367,   368,
     368,   369,   369,   369,   370
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
       2,     1,     2,     2,     3,     1,     2,     1,     3,     0,
       1,     3,     4,     1,     0,     3,     4,     5,     0,     7,
       0,     8,     1,     3,     4,     0,     6,     0,     7,     0,
       7,     0,     1,     0,     1,     0,     4,     4,     1,     4,
       8,     8,     1,     4,     1,     1,     2,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     0,     7,     0,
       6,     0,     7,     0,     6,     1,     2,     3,     1,     3,
       2,     3,     1,     1,     3,     3,     2,     4,     0,     2,
       1,     0,     3,     3,     2,     1,     1,     1,     1,     1,
       1,     1,     1,     2,     2,     2,     2,     0,     4,     1,
       1,     2,     0,     4,     1,     2,     1,     2,     5,     5,
       1,     2,     1,     3,     2,     2,     0,     4,     3,     1,
       0,     6,     7,     0,     5,     2,     2,     3,     1,     0,
       3,     4,     3,     4,     2,     2,     2,     3,     3,     1,
       2,     1,     1,     1,     2
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const unsigned short yydefact[] =
{
       0,   201,   204,   202,   203,   218,   210,   211,   212,   213,
     214,   217,   215,   216,   222,   223,   219,   220,   221,   205,
     206,   207,   208,   209,   224,   187,   188,   189,   194,   195,
     190,   191,   192,   193,   171,   196,     0,   225,   226,   154,
       0,     0,   313,   199,   200,   312,   162,     0,   117,   116,
       0,     0,   153,     0,     0,   139,   185,   197,     0,   198,
       0,   309,   311,   171,   172,     0,     0,     0,    98,     0,
       0,   195,   199,   200,     0,   198,   113,     0,   272,   314,
     115,     0,   135,   136,   127,   118,     0,   129,     0,   137,
     186,     0,   111,     0,   145,     0,   141,     0,   147,   171,
     140,    97,   229,     0,   114,     1,   310,     0,     0,   169,
     162,   155,     0,   109,     0,   107,     0,     0,     0,   112,
     271,     0,   119,   128,   130,   131,     0,   133,     0,   125,
     171,     0,     0,   143,   142,   146,   122,   149,   171,   163,
     227,     0,     0,   233,     0,     0,   173,     0,     0,   106,
     105,     0,     0,     0,   124,     0,     2,     8,     6,     7,
       9,    10,     3,     0,     0,     0,     0,     0,     0,   290,
     293,     0,     0,     0,     0,     0,     0,   267,   276,    45,
      44,    46,    43,    48,    47,     5,    11,    13,    35,    15,
       0,    31,     0,     0,    49,    30,     0,    53,    56,    59,
      64,    67,    69,    71,    73,    75,    77,    79,    81,    94,
       0,   255,   258,   274,   257,   256,     0,   259,   260,   261,
     262,   132,   134,     2,    30,    96,     0,   126,   171,   156,
     123,   148,   144,   150,     0,   149,   164,   165,     0,     0,
       0,   235,   231,     0,     0,   120,   178,   182,     0,     0,
       0,     0,   184,     0,   174,   102,   101,   110,   108,     0,
     104,   103,   265,   264,    28,   263,     0,    36,    37,   305,
     304,   306,     0,     0,     0,     0,     0,     0,     0,    41,
      40,     0,    39,     0,     0,   266,     0,     4,    18,    19,
       0,     0,     0,    21,     0,    29,     0,     0,    23,    24,
       0,    84,    85,    87,    86,    89,    90,    91,    92,    93,
      88,    83,     0,    42,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   277,   273,   275,   138,   157,
     158,   151,     0,   167,     0,     0,   171,     0,   238,   171,
     236,     0,     0,   121,     0,     0,     0,     0,     0,     0,
     175,   100,    99,   307,   308,     0,     0,     0,     0,     0,
       0,    20,     0,    12,    26,     0,     0,    17,     0,    16,
      25,    32,    22,    34,    33,    82,    50,    51,    52,    55,
      54,    57,    58,    62,    63,    60,    61,    65,    66,    68,
      70,    72,    74,    76,    78,     0,    95,   160,     0,   152,
       0,   242,   243,   248,   166,   171,   171,   240,     0,   237,
     230,     0,   234,     0,     0,     0,     0,     0,   170,     0,
       0,     0,   296,   295,     0,   298,     0,     0,     0,     0,
       0,    38,    27,   268,    14,     0,     0,   159,   168,     0,
       0,     0,     0,   251,   250,   228,   241,   239,   232,   183,
     177,   179,     0,     0,   176,     0,   297,     0,     0,   270,
     294,   269,     0,   289,   278,   286,   279,    80,   161,     0,
     254,   245,   244,   248,   246,   249,     0,     0,     0,   291,
     302,     0,   300,     0,     0,     0,   285,     0,   253,     0,
     252,     0,     0,   303,   301,   292,   288,     0,     0,   282,
       0,   280,   247,     0,     0,     0,   284,   287,   281,   180,
     181,   283
};

/* YYDEFGOTO[NTERM-NUM]. */
static const short yydefgoto[] =
{
      -1,   185,   186,   187,   188,   370,   189,   190,   191,   192,
     193,   194,   296,   195,   196,   197,   198,   199,   200,   201,
     202,   203,   204,   205,   206,   207,   208,   209,   312,   210,
     233,    72,    73,   114,   115,   211,   110,    47,    48,    49,
      84,    85,    86,    87,    50,    97,    98,   346,   234,   138,
      51,    67,   408,   446,    52,   344,   410,    53,   146,    65,
     253,   429,   254,    54,    55,    56,    57,    58,    75,   238,
     141,   351,   243,   240,   241,   347,   348,   413,   414,   451,
     452,   453,   486,   454,   212,   213,   214,   215,   286,   470,
     471,   121,   216,   217,   218,   510,   511,   476,   497,   474,
     219,   274,   275,   367,   436,   437,   220,    60,    61,    62
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -422
static const short yypact[] =
{
    2383,  -422,  -422,  -422,  -422,  -422,  -422,  -422,  -422,  -422,
    -422,  -422,  -422,  -422,  -422,  -422,  -422,  -422,  -422,  -422,
    -422,  -422,  -422,  -422,  -422,  -422,  -422,  -422,  -422,  -212,
    -422,  -422,  -422,  -422,  -188,  -422,  -187,  -422,  -422,  -422,
    -147,  3122,  -422,  -190,  -185,  -422,  -161,   -93,   -71,  3080,
     805,  -189,  -422,  3122,  -141,   324,  -422,  -422,  -144,  -156,
    2220,  -422,  -422,  -188,  -422,  3122,   -54,  2928,   -56,    50,
    -134,  -422,  -422,  -422,  -141,  -422,  -422,   -16,   -27,  -422,
    -422,  3080,  -422,  -422,  -422,  -422,   805,  -422,  2242,  -135,
    -422,  -141,  -422,  -141,  -422,  -153,   207,    60,  -422,   -61,
    -422,    -3,  -422,  -128,  -422,  -422,  -422,  3122,  -141,  -422,
    -422,  -422,    50,   -12,  -157,  -422,    -1,    50,     7,  -422,
    -422,  1285,  -422,  -422,  -422,  -422,   805,  -422,  1829,    19,
    -167,    46,    -2,    33,  -422,  -422,  -422,  1829,  -158,    45,
    -422,  3122,    51,  -422,  -141,    57,   103,   -92,  1829,  -422,
    -422,    50,    50,   -75,  -422,   -77,  -192,  -422,  -422,  -422,
    -422,  -422,  -422,  1936,  1936,    53,    54,   949,   135,  -422,
    -422,  1285,    61,    67,  1829,    68,  1883,    64,  -422,  -422,
    -422,  -422,  -422,  -422,  -422,  -422,   141,  -422,   -18,  -422,
    -109,    70,   525,  1829,  -422,   299,  1829,    -6,  -139,   -34,
    -124,    28,    59,    55,    73,   133,   140,  -160,  -422,  -422,
    -169,  -422,  -422,  -422,  -422,  -422,   853,  -422,  -422,  -422,
    -422,  -422,  -422,    94,  -422,  -422,    92,    96,  -158,    87,
    -422,  -422,  -422,  -422,    99,  1829,    90,  -422,  3122,  -141,
    2546,  -422,  -422,  3122,   106,  -422,  -422,   109,   113,   115,
     116,   117,  -422,   -80,  -422,  -422,  -422,  -422,  -422,   -65,
    -422,  -422,  -422,  -422,  -422,  -422,  1829,  -422,  -422,  -422,
    -422,  -422,   -81,    93,   118,   119,   143,  1829,  1829,  -422,
    -422,  2049,  -422,   -57,  -184,  -422,  1285,  -422,  -422,  -422,
     189,  1829,   190,  -422,  1829,  -422,  1829,   123,  -422,  -422,
     -60,  -422,  -422,  -422,  -422,  -422,  -422,  -422,  -422,  -422,
    -422,  -422,  1829,  -422,  1829,  1829,  1829,  1829,  1829,  1829,
    1829,  1829,  1829,  1829,  1829,  1829,  1829,  1829,  1829,  1829,
    1829,  1829,  1829,  1829,  1829,  -422,  -422,  -422,  -422,   122,
    -422,  -422,   125,  -422,  1165,  2698,  -167,   -15,  -422,  -188,
    -422,  3122,  2737,  -422,  1829,   200,   204,  1829,  1829,   131,
    -422,  -422,  -422,  -422,  -422,  1829,  1717,   306,   134,   -45,
     136,   128,   137,  -422,  -422,   144,  1069,  -422,   138,  -422,
    -422,  -422,  -422,  -422,  -422,  -422,  -422,  -422,  -422,    -6,
      -6,  -139,  -139,   -34,   -34,   -34,   -34,  -124,  -124,    28,
      59,    55,    73,   133,   140,     6,  -422,  -422,  1165,  -422,
    1165,  -422,  -422,   -29,  -422,  -188,  -158,  -422,  -141,  -422,
    -422,  2889,  -422,   150,   151,   152,   147,   149,  -422,   103,
     -44,   132,  -422,  -422,   148,   128,   153,  1501,  1829,  1285,
     157,  -422,  -422,  -422,  -422,  1829,  1165,  -422,  -422,  1829,
     211,   -33,  1165,   155,  -422,  -422,  -422,  -422,  -422,  -422,
    -422,  -422,  1829,  1829,  -422,  1501,  -422,  1381,  1597,  -422,
    -422,  -422,   -43,   198,  -422,   161,  -422,  -422,  -422,   164,
    -422,  -422,  -422,   -29,  -422,  -422,   -29,   162,   165,  -422,
    -422,   -25,  -422,   -11,   158,  1285,  -422,   637,  -422,  1165,
    -422,  1829,  1829,  -422,  -422,  -422,  -422,  1829,   179,  -422,
     421,  -422,  -422,   172,   188,   182,  -422,  -422,  -422,  -422,
    -422,  -422
};

/* YYPGOTO[NTERM-NUM].  */
static const short yypgoto[] =
{
    -422,  -422,  -422,  -422,  -422,   139,  -422,  -422,   104,  -422,
    -422,  -172,  -422,   -55,  -422,   -37,   -31,   -67,   -30,   101,
     105,   100,   112,   120,   121,  -422,  -115,  -126,  -422,  -130,
    -105,     0,     1,    37,   281,    30,    32,  -422,  -422,  -422,
      71,   353,   347,   102,    -7,   -86,   341,    -5,   209,  -123,
    -422,  -422,  -422,  -422,   381,  -422,  -422,   -32,  -422,    -4,
    -422,  -422,    20,   -36,  -422,    65,  -422,  -422,     4,  -422,
    -422,  -422,  -422,  -224,  -229,  -422,    36,  -422,  -369,  -422,
     -28,  -422,  -422,   -26,    84,  -146,  -421,  -422,  -422,    -9,
     411,  -422,   175,    97,  -422,  -422,   -52,  -422,  -422,  -422,
    -422,  -422,  -422,  -422,  -422,  -422,  -422,  -422,   402,  -422
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -300
static const short yytable[] =
{
      43,    44,    64,    63,    59,    74,   227,   228,    70,    94,
     134,   350,    64,   225,   345,    68,   469,    93,   101,   352,
     332,    94,   225,   226,   313,   276,   103,    94,   116,   108,
      45,    64,    46,   225,   142,    36,   149,   272,    36,   447,
      77,   448,    88,   257,   469,   264,   283,   232,   100,    99,
      66,   321,   322,   265,   374,    91,    36,    76,    92,   107,
      43,    44,  -171,   300,    59,    36,   299,    64,    96,   118,
     337,   144,   137,   224,    88,   334,    36,   478,   335,    36,
      78,   235,   224,   484,   129,   150,   130,   151,   131,   133,
      45,   104,    46,   224,    69,   139,    95,   102,    64,    96,
     333,   255,    95,   145,   128,   239,    64,   117,   267,   268,
     317,    96,   318,   143,    89,    90,   350,    96,   260,   280,
     225,    43,    44,   350,   381,    59,   229,   421,   361,   293,
     512,   323,   324,   383,   236,   294,   283,   319,   320,   244,
     284,   224,   386,   387,   388,    80,    89,   369,   371,   147,
     256,    89,   151,    89,   153,   288,   289,   123,   359,   125,
     481,   371,    36,   334,   360,   262,   363,   261,   380,   151,
     263,    43,    44,    81,   290,    59,   136,   362,   137,   151,
     224,   373,   384,   109,   334,   112,   385,   334,   124,   259,
     127,    89,   350,   439,   465,   494,    64,   221,   375,   334,
     334,   334,   239,   405,   239,   325,   326,   239,   406,   482,
     449,   483,   113,   503,   450,   120,    43,    44,   412,   334,
      59,   291,   135,   416,   339,   292,   372,   504,   222,   418,
     337,   119,   419,   334,   148,   430,   231,   435,   140,   225,
     152,   224,   225,   225,   154,   284,   314,   315,   316,   423,
     334,   445,   426,   427,   393,   394,   395,   396,   137,   224,
     224,   224,   224,   224,   224,   224,   224,   224,   224,   224,
     224,   224,   224,   224,   224,   224,   224,   224,   279,   282,
     389,   390,   412,   230,   412,    96,    43,    44,   391,   392,
      59,   237,   242,   473,   245,   397,   398,   273,   277,   224,
     269,   270,   224,   224,   278,   281,   285,   434,   472,   239,
     287,   295,   330,   328,    64,   239,   239,    64,   327,   477,
     412,   331,   246,   247,   248,   249,   412,   250,   251,   252,
     329,   264,   338,   340,   225,   235,   343,   491,   493,   341,
     364,   368,   417,   353,   479,   420,   354,   225,   225,   506,
     355,   509,   356,   357,   358,   365,   366,   487,   488,   377,
     379,   382,   424,   425,   509,   409,    43,    44,   407,   428,
      59,   438,   334,   412,   440,   441,    43,    44,   444,   466,
      59,   480,   442,    64,    64,   239,   225,   225,   459,   460,
     461,   462,   225,   463,   224,   467,   513,   514,   475,   495,
     468,   485,   515,   496,   498,   505,   501,   224,   224,   502,
     519,   455,   456,    25,    26,    27,    28,    71,    30,    31,
      32,    33,   155,    35,   516,     1,   520,   521,   399,   401,
     378,     2,   258,   400,   122,   126,   132,    43,    44,    43,
      44,    59,   402,    59,   342,     3,   224,   224,   111,   464,
     432,   403,   224,   404,   457,   499,   489,    79,   518,    96,
     500,   376,   106,   433,     0,    43,    44,     0,   223,    59,
     157,   158,   159,   160,   161,   162,     0,     0,     0,   163,
     164,   301,   302,   303,   304,   305,   306,   307,   308,   309,
     310,     4,     0,     0,     0,    43,    44,    43,    44,    59,
       0,    59,     0,     0,     0,     0,     0,     0,     0,     0,
      43,    44,     0,     0,    59,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,   297,
      25,    26,    27,    28,    71,    30,    31,    32,    33,   174,
      35,   175,     0,   176,     0,   311,     0,     0,     0,     0,
       0,     0,     0,  -299,   179,   180,   181,   182,   183,     0,
       0,     0,     0,     0,     0,   184,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    19,    20,
      21,    22,    23,   156,    24,   157,   158,   159,   160,   161,
     162,     0,     0,     0,   163,   164,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   165,   166,   167,   168,   169,
     170,   171,     0,   172,   173,   507,   508,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    35,   155,     0,
       0,     1,     0,     0,    36,     0,     0,     2,     0,    37,
      38,    39,    40,    41,   174,     0,   175,     0,   176,     0,
       0,     3,   177,   517,     0,     0,     0,     0,   178,   179,
     180,   181,   182,   183,     0,     0,     0,     0,     0,     0,
     184,     0,     0,     0,     0,     0,     0,   223,     0,   157,
     158,   159,   160,   161,   162,     0,     0,     0,   163,   164,
       0,     0,     0,     0,     0,     0,     0,     4,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     5,     6,     7,     8,     9,    10,    11,    12,    13,
      14,    15,    16,    17,    18,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   174,     0,
     175,     0,   176,   298,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   179,   180,   181,   182,   183,     0,     0,
       0,     0,     0,     0,   184,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    19,    20,    21,    22,    23,   156,
      24,   157,   158,   159,   160,   161,   162,     0,     0,     1,
     163,   164,     0,     0,     0,     2,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     3,
       0,   165,   166,   167,   168,   169,   170,   171,     0,   172,
     173,   507,   508,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,   155,     0,     0,     1,     0,     0,
      36,     0,     0,     2,     0,    37,    38,    39,    40,    41,
     174,     0,   175,     0,   176,     4,     0,     3,   177,     0,
       0,     0,     0,     0,   178,   179,   180,   181,   182,   183,
       0,     0,     0,     0,     0,     0,   184,     0,     0,     5,
       6,     7,     8,     9,    10,    11,    12,    13,    14,    15,
      16,    17,    18,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     4,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     5,     6,     7,
       8,     9,    10,    11,    12,    13,    14,    15,    16,    17,
      18,     0,    19,    20,    21,    22,    23,     0,    24,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      19,    20,    21,    22,    23,   156,    24,   157,   158,   159,
     160,   161,   162,     0,     0,     0,   163,   164,     0,     0,
       0,     0,     0,    37,    38,     0,    40,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   165,   166,   167,
     168,   169,   170,   171,     0,   172,   173,     0,     0,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
     155,     0,     0,     1,     0,     0,    36,     0,     0,     2,
       0,    37,    38,    39,    40,    41,   174,     0,   175,     0,
     176,     0,     0,     3,   177,   336,     0,     0,     0,     0,
     178,   179,   180,   181,   182,   183,     0,     0,     0,     0,
       0,   223,   184,   157,   158,   159,   160,   161,   162,     0,
       0,     0,   163,   164,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     4,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     5,     6,     7,     8,     9,    10,    11,
      12,    13,    14,    15,    16,    17,    18,     0,     0,     0,
       0,     0,   174,     0,   175,     0,   176,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   271,   179,   180,   181,
     182,   183,     0,     0,     0,     0,     0,     0,   184,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    19,    20,    21,    22,
      23,   156,    24,   157,   158,   159,   160,   161,   162,     0,
       0,     0,   163,   164,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   165,   166,   167,   168,   169,   170,   171,
       0,   172,   173,     0,     0,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,   155,     0,     0,     1,
       0,     0,    36,     0,     0,     2,     0,    37,    38,    39,
      40,    41,   174,     0,   175,     0,   176,     0,     0,     3,
     177,   443,     0,     0,     0,     0,   178,   179,   180,   181,
     182,   183,     0,     0,     0,     0,     0,   223,   184,   157,
     158,   159,   160,   161,   162,     0,     0,     0,   163,   164,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     4,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     5,
       6,     7,     8,     9,    10,    11,    12,    13,    14,    15,
      16,    17,    18,     0,     0,     0,     0,     0,   174,     0,
     175,     0,   176,     0,     0,     0,   411,     0,     0,     0,
       0,     0,     0,   179,   180,   181,   182,   183,     0,     0,
       0,     0,     0,     0,   184,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    19,    20,    21,    22,    23,   156,    24,   157,
     158,   159,   160,   161,   162,     0,     0,     0,   163,   164,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   165,
     166,   167,   168,   169,   170,   171,     0,   172,   173,     0,
       0,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    35,   155,     0,     0,     1,     0,     0,    36,     0,
       0,     2,     0,    37,    38,    39,    40,    41,   174,     0,
     175,     0,   176,     0,     0,     3,   177,     0,     0,     0,
       0,     0,   178,   179,   180,   181,   182,   183,     0,     0,
       0,     0,     0,   223,   184,   157,   158,   159,   160,   161,
     162,     0,     0,     0,   163,   164,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     4,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,     0,
       0,     0,     0,     0,   174,     0,   175,     0,   176,   490,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   179,
     180,   181,   182,   183,     0,     0,     0,     0,     0,     0,
     184,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    19,    20,
      21,    22,    23,   156,    24,   157,   158,   159,   160,   161,
     162,     0,     0,     0,   163,   164,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   165,   166,   167,   168,   169,
     170,   171,     0,   172,   173,     0,     0,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    35,   431,     0,
       0,     1,     0,     0,    36,     0,     0,     2,     0,    37,
      38,    39,    40,    41,   174,     0,   175,     0,   176,     0,
       0,     3,    78,     0,     0,     0,     0,     0,   178,   179,
     180,   181,   182,   183,     0,     0,     0,     0,     0,   223,
     184,   157,   158,   159,   160,   161,   162,     0,     0,     0,
     163,   164,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     4,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     5,     6,     7,     8,     9,    10,    11,    12,    13,
      14,    15,    16,    17,    18,     0,     0,     0,     0,     0,
     174,     0,   175,     0,   176,   492,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   179,   180,   181,   182,   183,
       0,     0,     0,     0,     0,     0,   184,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    19,    20,    21,    22,    23,   223,
      24,   157,   158,   159,   160,   161,   162,     1,     0,     0,
     163,   164,     0,     2,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     3,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,     0,     0,     0,     0,     0,     0,
      36,     0,     0,     0,     0,    37,    38,    39,    40,    41,
     174,     0,   175,     4,   176,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   178,   179,   180,   181,   182,   183,
       0,     0,     0,     0,     0,     0,   184,     5,     6,     7,
       8,     9,    10,    11,    12,    13,    14,    15,    16,    17,
      18,   223,     0,   157,   158,   159,   160,   161,   162,     0,
       0,     0,   163,   164,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      19,    20,    21,    22,    23,   223,    24,   157,   158,   159,
     160,   161,   162,     1,     0,     0,   163,   164,     0,     2,
       0,     0,   174,     0,   175,     0,   176,     0,     0,     0,
       0,     0,     0,     3,     0,     0,     0,   179,   180,   181,
     182,   183,     0,     0,     0,     0,     0,     0,   184,    25,
      26,    27,    28,    71,    30,    31,    32,    33,   223,    35,
     157,   158,   159,   160,   161,   162,     0,     0,     0,   163,
     164,    37,    38,     0,    40,     0,   174,     0,   175,     4,
     176,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   179,   180,   181,   182,   183,     0,     0,     0,     0,
       0,     0,   184,     5,     6,     7,     8,     9,    10,    11,
      12,    13,    14,    15,    16,    17,    18,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   174,
       0,   175,     0,   266,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   179,   180,   181,   182,   183,     0,
       0,     0,     0,     0,     0,   184,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    19,    20,    21,    22,
      23,   223,    24,   157,   158,   159,   160,   161,   162,     0,
     105,     0,   163,   164,     1,     0,     0,     0,     0,     0,
       2,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     3,     0,     1,     0,     0,     0,
       0,     0,     2,     0,     0,    25,    26,    27,    28,    71,
      30,    31,    32,    33,     0,    35,     3,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    37,    38,     0,
      40,     0,   174,     0,   175,     0,   266,     0,     0,     0,
       4,     0,     0,     0,     0,     0,     0,   179,   180,   181,
     182,   183,     0,     0,     0,     0,     0,     0,   184,     0,
       0,     0,     4,     0,     5,     6,     7,     8,     9,    10,
      11,    12,    13,    14,    15,    16,    17,    18,     0,     0,
       0,     0,     0,     0,     0,     0,     5,     6,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    17,    18,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    19,    20,    21,
      22,    23,     0,    24,     0,     0,     0,     1,     0,     0,
       0,     0,     0,     2,     0,     0,     0,     0,     0,    19,
      20,    21,    22,    23,     0,    24,     0,     3,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    35,     0,     0,     0,
       0,     0,     0,    36,     0,     0,     0,     0,    37,    38,
      39,    40,    41,     4,     0,     0,     0,     0,     0,    82,
      83,     0,     0,     0,     0,     0,     0,    42,     0,     0,
      37,    38,     0,    40,     0,     0,     0,     5,     6,     7,
       8,     9,    10,    11,    12,    13,    14,    15,    16,    17,
      18,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      19,    20,    21,    22,    23,     0,    24,     0,     0,     0,
       1,     0,     0,     0,     0,     0,     2,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       3,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
       0,     0,     0,     0,     0,     0,    36,     0,     0,     0,
       0,    37,    38,    39,    40,    41,     4,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      42,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     1,    19,    20,    21,    22,    23,     2,    24,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     3,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     1,     0,     0,     0,     0,     0,     2,     0,     0,
       0,     0,    25,    26,    27,    28,    71,    30,    31,    32,
      33,     3,    35,     0,     0,     0,     0,     0,     4,     0,
       0,     0,     0,     0,    37,    38,     0,    40,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   349,     0,
       0,     0,     5,     6,     7,     8,     9,    10,    11,    12,
      13,    14,    15,    16,    17,    18,     0,     4,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     5,     6,     7,     8,     9,    10,    11,    12,    13,
      14,    15,    16,    17,    18,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    19,    20,    21,    22,    23,
       0,    24,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     1,    19,    20,    21,    22,    23,     2,
      24,     0,     0,     0,    25,    26,    27,    28,    71,    30,
      31,    32,    33,     3,    35,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    37,    38,     0,    40,
       0,     0,     1,     0,     0,     0,     0,     0,     2,     0,
     415,     0,     0,    25,    26,    27,    28,    71,    30,    31,
      32,    33,     3,    35,     0,     0,     0,     0,     0,     4,
       0,     0,     0,     0,     0,    37,    38,     0,    40,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   422,
       0,     0,     0,     5,     6,     7,     8,     9,    10,    11,
      12,    13,    14,    15,    16,    17,    18,     0,     4,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     5,     6,     7,     8,     9,    10,    11,    12,
      13,    14,    15,    16,    17,    18,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    19,    20,    21,    22,
      23,     0,    24,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     1,    19,    20,    21,    22,    23,
       2,    24,     0,     0,     0,    25,    26,    27,    28,    71,
      30,    31,    32,    33,     3,    35,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    37,    38,     0,
      40,     0,     0,     0,     0,     0,     1,     0,     0,     0,
       0,   458,     2,     0,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    35,     0,     3,     0,     0,     0,
       4,    36,     0,     0,     0,     0,    37,    38,     0,    40,
      41,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     5,     6,     7,     8,     9,    10,
      11,    12,    13,    14,    15,    16,    17,    18,     0,     0,
       0,     0,     4,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     5,     6,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    17,    18,
       0,     0,     0,     0,     0,     0,     0,    19,    20,    21,
      22,    23,     0,    24,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    19,
      20,    21,    22,    23,     0,    24,    25,    26,    27,    28,
      71,    30,    31,    32,    33,     0,    35,    82,    83,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    37,    38,
       0,    40,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    25,    26,
      27,    28,    71,    30,    31,    32,    33,     0,    35,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      37,    38,     0,    40
};

static const short yycheck[] =
{
       0,     0,    34,   215,     0,    41,   129,   130,    40,   162,
      96,   240,    44,   128,   238,   162,   437,    53,   162,   243,
     180,   162,   137,   128,   196,   171,    58,   162,   162,    65,
       0,    63,     0,   148,   162,   223,   193,   167,   223,   408,
      44,   410,    49,   148,   465,   237,   176,   133,    55,    54,
     237,   175,   176,   245,   238,   244,   223,   247,   247,    63,
      60,    60,   247,   193,    60,   223,   192,    99,   252,    74,
     216,   107,   239,   128,    81,   244,   223,   446,   247,   223,
     241,   239,   137,   452,    89,   242,    91,   244,    93,    96,
      60,   247,    60,   148,   241,    99,   237,   241,   130,   252,
     260,   193,   237,   108,   239,   141,   138,   241,   163,   164,
     249,   252,   251,   241,    49,    50,   345,   252,   193,   174,
     235,   121,   121,   352,   296,   121,   130,   351,   193,   238,
     499,   255,   256,   193,   138,   244,   266,   171,   172,   144,
     176,   196,   314,   315,   316,   238,    81,   277,   278,   112,
     242,    86,   244,    88,   117,   173,   174,    86,   238,    88,
     193,   291,   223,   244,   244,   242,   247,   242,   294,   244,
     247,   171,   171,   244,   192,   171,   237,   242,   239,   244,
     235,   238,   242,   237,   244,   241,   312,   244,    86,   152,
      88,   126,   421,   238,   238,   238,   228,   126,   284,   244,
     244,   244,   238,   333,   240,   177,   178,   243,   334,   242,
     239,   244,   162,   238,   243,   242,   216,   216,   344,   244,
     216,   239,   162,   346,   228,   243,   281,   238,   126,   244,
     376,   247,   247,   244,   246,   365,   238,   367,   241,   354,
     241,   296,   357,   358,   237,   281,   252,   253,   254,   354,
     244,   245,   357,   358,   321,   322,   323,   324,   239,   314,
     315,   316,   317,   318,   319,   320,   321,   322,   323,   324,
     325,   326,   327,   328,   329,   330,   331,   332,   174,   175,
     317,   318,   408,   237,   410,   252,   286,   286,   319,   320,
     286,   246,   241,   439,   237,   325,   326,   162,   237,   354,
     247,   247,   357,   358,   237,   237,   242,     1,   438,   345,
     169,   241,   179,   258,   346,   351,   352,   349,   259,   445,
     446,   181,   219,   220,   221,   222,   452,   224,   225,   226,
     257,   237,   240,   246,   449,   239,   246,   467,   468,   240,
     247,   198,   346,   237,   449,   349,   237,   462,   463,   495,
     237,   497,   237,   237,   237,   237,   237,   462,   463,   170,
     170,   238,   162,   159,   510,   240,   366,   366,   246,   238,
     366,   237,   244,   499,   238,   238,   376,   376,   240,   247,
     376,   170,   238,   415,   416,   421,   501,   502,   238,   238,
     238,   244,   507,   244,   449,   247,   501,   502,   241,   201,
     247,   246,   507,   242,   240,   247,   244,   462,   463,   244,
     238,   415,   416,   206,   207,   208,   209,   210,   211,   212,
     213,   214,     1,   216,   245,     4,   238,   245,   327,   329,
     291,    10,   151,   328,    81,    88,    95,   437,   437,   439,
     439,   437,   330,   439,   235,    24,   501,   502,    67,   429,
     366,   331,   507,   332,   418,   483,   465,    46,   510,   252,
     486,   286,    60,   366,    -1,   465,   465,    -1,   162,   465,
     164,   165,   166,   167,   168,   169,    -1,    -1,    -1,   173,
     174,   182,   183,   184,   185,   186,   187,   188,   189,   190,
     191,    70,    -1,    -1,    -1,   495,   495,   497,   497,   495,
      -1,   497,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     510,   510,    -1,    -1,   510,    94,    95,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,     4,
     206,   207,   208,   209,   210,   211,   212,   213,   214,   233,
     216,   235,    -1,   237,    -1,   246,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   247,   248,   249,   250,   251,   252,    -1,
      -1,    -1,    -1,    -1,    -1,   259,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   157,   158,
     159,   160,   161,   162,   163,   164,   165,   166,   167,   168,
     169,    -1,    -1,    -1,   173,   174,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   194,   195,   196,   197,   198,
     199,   200,    -1,   202,   203,   204,   205,   206,   207,   208,
     209,   210,   211,   212,   213,   214,   215,   216,     1,    -1,
      -1,     4,    -1,    -1,   223,    -1,    -1,    10,    -1,   228,
     229,   230,   231,   232,   233,    -1,   235,    -1,   237,    -1,
      -1,    24,   241,   242,    -1,    -1,    -1,    -1,   247,   248,
     249,   250,   251,   252,    -1,    -1,    -1,    -1,    -1,    -1,
     259,    -1,    -1,    -1,    -1,    -1,    -1,   162,    -1,   164,
     165,   166,   167,   168,   169,    -1,    -1,    -1,   173,   174,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    70,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    94,    95,    96,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   233,    -1,
     235,    -1,   237,   238,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   248,   249,   250,   251,   252,    -1,    -1,
      -1,    -1,    -1,    -1,   259,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   157,   158,   159,   160,   161,   162,
     163,   164,   165,   166,   167,   168,   169,    -1,    -1,     4,
     173,   174,    -1,    -1,    -1,    10,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    24,
      -1,   194,   195,   196,   197,   198,   199,   200,    -1,   202,
     203,   204,   205,   206,   207,   208,   209,   210,   211,   212,
     213,   214,   215,   216,     1,    -1,    -1,     4,    -1,    -1,
     223,    -1,    -1,    10,    -1,   228,   229,   230,   231,   232,
     233,    -1,   235,    -1,   237,    70,    -1,    24,   241,    -1,
      -1,    -1,    -1,    -1,   247,   248,   249,   250,   251,   252,
      -1,    -1,    -1,    -1,    -1,    -1,   259,    -1,    -1,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    70,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    94,    95,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,    -1,   157,   158,   159,   160,   161,    -1,   163,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     157,   158,   159,   160,   161,   162,   163,   164,   165,   166,
     167,   168,   169,    -1,    -1,    -1,   173,   174,    -1,    -1,
      -1,    -1,    -1,   228,   229,    -1,   231,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   194,   195,   196,
     197,   198,   199,   200,    -1,   202,   203,    -1,    -1,   206,
     207,   208,   209,   210,   211,   212,   213,   214,   215,   216,
       1,    -1,    -1,     4,    -1,    -1,   223,    -1,    -1,    10,
      -1,   228,   229,   230,   231,   232,   233,    -1,   235,    -1,
     237,    -1,    -1,    24,   241,   242,    -1,    -1,    -1,    -1,
     247,   248,   249,   250,   251,   252,    -1,    -1,    -1,    -1,
      -1,   162,   259,   164,   165,   166,   167,   168,   169,    -1,
      -1,    -1,   173,   174,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    70,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    94,    95,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,    -1,    -1,    -1,
      -1,    -1,   233,    -1,   235,    -1,   237,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   247,   248,   249,   250,
     251,   252,    -1,    -1,    -1,    -1,    -1,    -1,   259,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   157,   158,   159,   160,
     161,   162,   163,   164,   165,   166,   167,   168,   169,    -1,
      -1,    -1,   173,   174,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   194,   195,   196,   197,   198,   199,   200,
      -1,   202,   203,    -1,    -1,   206,   207,   208,   209,   210,
     211,   212,   213,   214,   215,   216,     1,    -1,    -1,     4,
      -1,    -1,   223,    -1,    -1,    10,    -1,   228,   229,   230,
     231,   232,   233,    -1,   235,    -1,   237,    -1,    -1,    24,
     241,   242,    -1,    -1,    -1,    -1,   247,   248,   249,   250,
     251,   252,    -1,    -1,    -1,    -1,    -1,   162,   259,   164,
     165,   166,   167,   168,   169,    -1,    -1,    -1,   173,   174,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    70,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,    -1,    -1,    -1,    -1,    -1,   233,    -1,
     235,    -1,   237,    -1,    -1,    -1,   241,    -1,    -1,    -1,
      -1,    -1,    -1,   248,   249,   250,   251,   252,    -1,    -1,
      -1,    -1,    -1,    -1,   259,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   157,   158,   159,   160,   161,   162,   163,   164,
     165,   166,   167,   168,   169,    -1,    -1,    -1,   173,   174,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   194,
     195,   196,   197,   198,   199,   200,    -1,   202,   203,    -1,
      -1,   206,   207,   208,   209,   210,   211,   212,   213,   214,
     215,   216,     1,    -1,    -1,     4,    -1,    -1,   223,    -1,
      -1,    10,    -1,   228,   229,   230,   231,   232,   233,    -1,
     235,    -1,   237,    -1,    -1,    24,   241,    -1,    -1,    -1,
      -1,    -1,   247,   248,   249,   250,   251,   252,    -1,    -1,
      -1,    -1,    -1,   162,   259,   164,   165,   166,   167,   168,
     169,    -1,    -1,    -1,   173,   174,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    70,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    94,    95,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,    -1,
      -1,    -1,    -1,    -1,   233,    -1,   235,    -1,   237,   238,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   248,
     249,   250,   251,   252,    -1,    -1,    -1,    -1,    -1,    -1,
     259,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   157,   158,
     159,   160,   161,   162,   163,   164,   165,   166,   167,   168,
     169,    -1,    -1,    -1,   173,   174,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   194,   195,   196,   197,   198,
     199,   200,    -1,   202,   203,    -1,    -1,   206,   207,   208,
     209,   210,   211,   212,   213,   214,   215,   216,     1,    -1,
      -1,     4,    -1,    -1,   223,    -1,    -1,    10,    -1,   228,
     229,   230,   231,   232,   233,    -1,   235,    -1,   237,    -1,
      -1,    24,   241,    -1,    -1,    -1,    -1,    -1,   247,   248,
     249,   250,   251,   252,    -1,    -1,    -1,    -1,    -1,   162,
     259,   164,   165,   166,   167,   168,   169,    -1,    -1,    -1,
     173,   174,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    70,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    94,    95,    96,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,    -1,    -1,    -1,    -1,    -1,
     233,    -1,   235,    -1,   237,   238,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   248,   249,   250,   251,   252,
      -1,    -1,    -1,    -1,    -1,    -1,   259,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   157,   158,   159,   160,   161,   162,
     163,   164,   165,   166,   167,   168,   169,     4,    -1,    -1,
     173,   174,    -1,    10,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    24,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   206,   207,   208,   209,   210,   211,   212,
     213,   214,   215,   216,    -1,    -1,    -1,    -1,    -1,    -1,
     223,    -1,    -1,    -1,    -1,   228,   229,   230,   231,   232,
     233,    -1,   235,    70,   237,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   247,   248,   249,   250,   251,   252,
      -1,    -1,    -1,    -1,    -1,    -1,   259,    94,    95,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   162,    -1,   164,   165,   166,   167,   168,   169,    -1,
      -1,    -1,   173,   174,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     157,   158,   159,   160,   161,   162,   163,   164,   165,   166,
     167,   168,   169,     4,    -1,    -1,   173,   174,    -1,    10,
      -1,    -1,   233,    -1,   235,    -1,   237,    -1,    -1,    -1,
      -1,    -1,    -1,    24,    -1,    -1,    -1,   248,   249,   250,
     251,   252,    -1,    -1,    -1,    -1,    -1,    -1,   259,   206,
     207,   208,   209,   210,   211,   212,   213,   214,   162,   216,
     164,   165,   166,   167,   168,   169,    -1,    -1,    -1,   173,
     174,   228,   229,    -1,   231,    -1,   233,    -1,   235,    70,
     237,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   248,   249,   250,   251,   252,    -1,    -1,    -1,    -1,
      -1,    -1,   259,    94,    95,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   233,
      -1,   235,    -1,   237,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   248,   249,   250,   251,   252,    -1,
      -1,    -1,    -1,    -1,    -1,   259,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   157,   158,   159,   160,
     161,   162,   163,   164,   165,   166,   167,   168,   169,    -1,
       0,    -1,   173,   174,     4,    -1,    -1,    -1,    -1,    -1,
      10,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    24,    -1,     4,    -1,    -1,    -1,
      -1,    -1,    10,    -1,    -1,   206,   207,   208,   209,   210,
     211,   212,   213,   214,    -1,   216,    24,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   228,   229,    -1,
     231,    -1,   233,    -1,   235,    -1,   237,    -1,    -1,    -1,
      70,    -1,    -1,    -1,    -1,    -1,    -1,   248,   249,   250,
     251,   252,    -1,    -1,    -1,    -1,    -1,    -1,   259,    -1,
      -1,    -1,    70,    -1,    94,    95,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    94,    95,    96,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   157,   158,   159,
     160,   161,    -1,   163,    -1,    -1,    -1,     4,    -1,    -1,
      -1,    -1,    -1,    10,    -1,    -1,    -1,    -1,    -1,   157,
     158,   159,   160,   161,    -1,   163,    -1,    24,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   206,   207,   208,   209,
     210,   211,   212,   213,   214,   215,   216,    -1,    -1,    -1,
      -1,    -1,    -1,   223,    -1,    -1,    -1,    -1,   228,   229,
     230,   231,   232,    70,    -1,    -1,    -1,    -1,    -1,   217,
     218,    -1,    -1,    -1,    -1,    -1,    -1,   247,    -1,    -1,
     228,   229,    -1,   231,    -1,    -1,    -1,    94,    95,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     157,   158,   159,   160,   161,    -1,   163,    -1,    -1,    -1,
       4,    -1,    -1,    -1,    -1,    -1,    10,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      24,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   206,
     207,   208,   209,   210,   211,   212,   213,   214,   215,   216,
      -1,    -1,    -1,    -1,    -1,    -1,   223,    -1,    -1,    -1,
      -1,   228,   229,   230,   231,   232,    70,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     247,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      94,    95,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,     4,   157,   158,   159,   160,   161,    10,   163,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    24,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,     4,    -1,    -1,    -1,    -1,    -1,    10,    -1,    -1,
      -1,    -1,   206,   207,   208,   209,   210,   211,   212,   213,
     214,    24,   216,    -1,    -1,    -1,    -1,    -1,    70,    -1,
      -1,    -1,    -1,    -1,   228,   229,    -1,   231,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   242,    -1,
      -1,    -1,    94,    95,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,    -1,    70,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    94,    95,    96,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   157,   158,   159,   160,   161,
      -1,   163,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,     4,   157,   158,   159,   160,   161,    10,
     163,    -1,    -1,    -1,   206,   207,   208,   209,   210,   211,
     212,   213,   214,    24,   216,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   228,   229,    -1,   231,
      -1,    -1,     4,    -1,    -1,    -1,    -1,    -1,    10,    -1,
     242,    -1,    -1,   206,   207,   208,   209,   210,   211,   212,
     213,   214,    24,   216,    -1,    -1,    -1,    -1,    -1,    70,
      -1,    -1,    -1,    -1,    -1,   228,   229,    -1,   231,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   242,
      -1,    -1,    -1,    94,    95,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,    -1,    70,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    94,    95,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   157,   158,   159,   160,
     161,    -1,   163,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,     4,   157,   158,   159,   160,   161,
      10,   163,    -1,    -1,    -1,   206,   207,   208,   209,   210,
     211,   212,   213,   214,    24,   216,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   228,   229,    -1,
     231,    -1,    -1,    -1,    -1,    -1,     4,    -1,    -1,    -1,
      -1,   242,    10,    -1,   206,   207,   208,   209,   210,   211,
     212,   213,   214,   215,   216,    -1,    24,    -1,    -1,    -1,
      70,   223,    -1,    -1,    -1,    -1,   228,   229,    -1,   231,
     232,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    94,    95,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,    -1,    -1,
      -1,    -1,    70,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    94,    95,    96,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   157,   158,   159,
     160,   161,    -1,   163,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   157,
     158,   159,   160,   161,    -1,   163,   206,   207,   208,   209,
     210,   211,   212,   213,   214,    -1,   216,   217,   218,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   228,   229,
      -1,   231,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   206,   207,
     208,   209,   210,   211,   212,   213,   214,    -1,   216,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     228,   229,    -1,   231
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const unsigned short yystos[] =
{
       0,     4,    10,    24,    70,    94,    95,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   157,
     158,   159,   160,   161,   163,   206,   207,   208,   209,   210,
     211,   212,   213,   214,   215,   216,   223,   228,   229,   230,
     231,   232,   247,   292,   293,   296,   297,   298,   299,   300,
     305,   311,   315,   318,   324,   325,   326,   327,   328,   329,
     368,   369,   370,   215,   318,   320,   237,   312,   162,   241,
     318,   210,   292,   293,   324,   329,   247,   320,   241,   351,
     238,   244,   217,   218,   301,   302,   303,   304,   305,   326,
     326,   244,   247,   324,   162,   237,   252,   306,   307,   308,
     305,   162,   241,   318,   247,     0,   369,   320,   324,   237,
     297,   315,   241,   162,   294,   295,   162,   241,   308,   247,
     242,   352,   302,   301,   304,   301,   303,   304,   239,   308,
     308,   308,   307,   305,   306,   162,   237,   239,   310,   320,
     241,   331,   162,   241,   324,   308,   319,   294,   246,   193,
     242,   244,   241,   294,   237,     1,   162,   164,   165,   166,
     167,   168,   169,   173,   174,   194,   195,   196,   197,   198,
     199,   200,   202,   203,   233,   235,   237,   241,   247,   248,
     249,   250,   251,   252,   259,   262,   263,   264,   265,   267,
     268,   269,   270,   271,   272,   274,   275,   276,   277,   278,
     279,   280,   281,   282,   283,   284,   285,   286,   287,   288,
     290,   296,   345,   346,   347,   348,   353,   354,   355,   361,
     367,   301,   304,   162,   274,   287,   291,   310,   310,   320,
     237,   238,   306,   291,   309,   239,   320,   246,   330,   324,
     334,   335,   241,   333,   308,   237,   219,   220,   221,   222,
     224,   225,   226,   321,   323,   193,   242,   291,   295,   294,
     193,   242,   242,   247,   237,   245,   237,   274,   274,   247,
     247,   247,   290,   162,   362,   363,   346,   237,   237,   269,
     274,   237,   269,   290,   324,   242,   349,   169,   173,   174,
     192,   239,   243,   238,   244,   241,   273,     4,   238,   288,
     290,   182,   183,   184,   185,   186,   187,   188,   189,   190,
     191,   246,   289,   272,   252,   253,   254,   249,   251,   171,
     172,   175,   176,   255,   256,   177,   178,   259,   258,   257,
     179,   181,   180,   260,   244,   247,   242,   346,   240,   320,
     246,   240,   309,   246,   316,   334,   308,   336,   337,   242,
     335,   332,   334,   237,   237,   237,   237,   237,   237,   238,
     244,   193,   242,   247,   247,   237,   237,   364,   198,   290,
     266,   290,   274,   238,   238,   306,   353,   170,   266,   170,
     288,   272,   238,   193,   242,   288,   272,   272,   272,   276,
     276,   277,   277,   278,   278,   278,   278,   279,   279,   280,
     281,   282,   283,   284,   285,   290,   288,   246,   313,   240,
     317,   241,   288,   338,   339,   242,   310,   320,   244,   247,
     320,   334,   242,   291,   162,   159,   291,   291,   238,   322,
     290,     1,   345,   354,     1,   290,   365,   366,   237,   238,
     238,   238,   238,   242,   240,   245,   314,   339,   339,   239,
     243,   340,   341,   342,   344,   320,   320,   337,   242,   238,
     238,   238,   244,   244,   323,   238,   247,   247,   247,   347,
     350,   351,   290,   346,   360,   241,   358,   288,   339,   291,
     170,   193,   242,   244,   339,   246,   343,   291,   291,   350,
     238,   290,   238,   290,   238,   201,   242,   359,   240,   341,
     344,   244,   244,   238,   238,   247,   346,   204,   205,   346,
     356,   357,   339,   291,   291,   291,   245,   242,   357,   238,
     238,   245
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
#line 196 "gc_cl.y"
    { yyval.expr = clParseVariableIdentifier(Compiler, &yyvsp[0].token);
		  if(yyval.expr == gcvNULL) {
		     YYERROR;
		  }
		;}
    break;

  case 3:
#line 205 "gc_cl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 4:
#line 207 "gc_cl.y"
    { yyval.token = clParseCatStringLiteral(Compiler, &yyvsp[-1].token, &yyvsp[0].token); ;}
    break;

  case 5:
#line 212 "gc_cl.y"
    { yyval.expr = yyvsp[0].expr; ;}
    break;

  case 6:
#line 214 "gc_cl.y"
    { yyval.expr = clParseScalarConstant(Compiler, &yyvsp[0].token); ;}
    break;

  case 7:
#line 216 "gc_cl.y"
    { yyval.expr = clParseScalarConstant(Compiler, &yyvsp[0].token); ;}
    break;

  case 8:
#line 218 "gc_cl.y"
    { yyval.expr = clParseScalarConstant(Compiler, &yyvsp[0].token); ;}
    break;

  case 9:
#line 220 "gc_cl.y"
    { yyval.expr = clParseScalarConstant(Compiler, &yyvsp[0].token); ;}
    break;

  case 10:
#line 222 "gc_cl.y"
    { yyval.expr = clParseScalarConstant(Compiler, &yyvsp[0].token); ;}
    break;

  case 11:
#line 224 "gc_cl.y"
    { yyval.expr = clParseStringLiteral(Compiler, &yyvsp[0].token); ;}
    break;

  case 12:
#line 226 "gc_cl.y"
    { yyval.expr = yyvsp[-1].expr; ;}
    break;

  case 13:
#line 231 "gc_cl.y"
    { yyval.expr = yyvsp[0].expr; ;}
    break;

  case 14:
#line 233 "gc_cl.y"
    { yyval.expr = clParseSubscriptExpr(Compiler, yyvsp[-3].expr, yyvsp[-1].expr); ;}
    break;

  case 15:
#line 235 "gc_cl.y"
    { yyval.expr = clParseFuncCallExprAsExpr(Compiler, yyvsp[0].funcCall); ;}
    break;

  case 16:
#line 237 "gc_cl.y"
    { yyval.expr = clParseFieldSelectionExpr(Compiler, yyvsp[-2].expr, &yyvsp[0].token); ;}
    break;

  case 17:
#line 239 "gc_cl.y"
    { yyval.expr = clParsePtrFieldSelectionExpr(Compiler, yyvsp[-2].expr, &yyvsp[0].token); ;}
    break;

  case 18:
#line 241 "gc_cl.y"
    { yyval.expr = clParseIncOrDecExpr(Compiler, gcvNULL, clvUNARY_POST_INC, yyvsp[-1].expr); ;}
    break;

  case 19:
#line 243 "gc_cl.y"
    { yyval.expr = clParseIncOrDecExpr(Compiler, gcvNULL, clvUNARY_POST_DEC, yyvsp[-1].expr); ;}
    break;

  case 20:
#line 248 "gc_cl.y"
    { yyval.expr = yyvsp[0].expr; ;}
    break;

  case 21:
#line 253 "gc_cl.y"
    { yyval.funcCall = yyvsp[-1].funcCall; ;}
    break;

  case 22:
#line 255 "gc_cl.y"
    { yyval.funcCall = yyvsp[-2].funcCall; ;}
    break;

  case 23:
#line 257 "gc_cl.y"
    { yyval.funcCall = yyvsp[-1].funcCall; ;}
    break;

  case 24:
#line 262 "gc_cl.y"
    { yyval.funcCall = clParseFuncCallArgument(Compiler, yyvsp[-1].funcCall, yyvsp[0].expr); ;}
    break;

  case 25:
#line 264 "gc_cl.y"
    { yyval.funcCall = clParseFuncCallArgument(Compiler, yyvsp[-2].funcCall, yyvsp[0].expr); ;}
    break;

  case 26:
#line 269 "gc_cl.y"
    { yyval.decl = yyvsp[-1].decl; ;}
    break;

  case 27:
#line 271 "gc_cl.y"
    { yyval.decl = clParseCreateDecl(Compiler, &yyvsp[-2].decl, yyvsp[-1].typeQualifierList); ;}
    break;

  case 28:
#line 276 "gc_cl.y"
    { yyval.funcCall = clParseFuncCallHeaderExpr(Compiler, &yyvsp[-1].token, gcvNULL); ;}
    break;

  case 29:
#line 281 "gc_cl.y"
    {
	    clParseCastExprBegin(Compiler, &yyvsp[-1].decl);
	    (void) gcoOS_ZeroMemory((gctPOINTER)&yyval.token, sizeof(clsLexToken));
	    yyval.token.type = T_TYPE_CAST;
	;}
    break;

  case 30:
#line 290 "gc_cl.y"
    { yyval.expr = yyvsp[0].expr; ;}
    break;

  case 31:
#line 292 "gc_cl.y"
    {
		   clParseCastExprBegin(Compiler, gcvNULL);
	           (void) gcoOS_ZeroMemory((gctPOINTER)&yyval.token, sizeof(clsLexToken));
		   yyval.token.type = T_TYPE_CAST;
		;}
    break;

  case 32:
#line 298 "gc_cl.y"
    { yyval.expr = clParseCastExprEnd(Compiler, &yyvsp[-2].decl, yyvsp[0].expr);
                ;}
    break;

  case 33:
#line 302 "gc_cl.y"
    { yyval.expr = clParseCastExprEnd(Compiler, gcvNULL, yyvsp[-1].expr);
                ;}
    break;

  case 34:
#line 306 "gc_cl.y"
    { yyval.expr = clParseCastExprEnd(Compiler, gcvNULL, yyvsp[-1].expr);
                ;}
    break;

  case 35:
#line 312 "gc_cl.y"
    { yyval.expr = yyvsp[0].expr; ;}
    break;

  case 36:
#line 314 "gc_cl.y"
    { yyval.expr = clParseIncOrDecExpr(Compiler, &yyvsp[-1].token, clvUNARY_PRE_INC, yyvsp[0].expr); ;}
    break;

  case 37:
#line 316 "gc_cl.y"
    { yyval.expr = clParseIncOrDecExpr(Compiler, &yyvsp[-1].token, clvUNARY_PRE_DEC, yyvsp[0].expr); ;}
    break;

  case 38:
#line 318 "gc_cl.y"
    { yyval.expr = clParseVecStep(Compiler, &yyvsp[-3].token, &yyvsp[-1].expr->decl); ;}
    break;

  case 39:
#line 320 "gc_cl.y"
    { yyval.expr = clParseVecStep(Compiler, &yyvsp[-1].token, &yyvsp[0].decl); ;}
    break;

  case 40:
#line 322 "gc_cl.y"
    { yyval.expr = clParseSizeofExpr(Compiler, &yyvsp[-1].token, yyvsp[0].expr); ;}
    break;

  case 41:
#line 324 "gc_cl.y"
    { yyval.expr = clParseSizeofTypeDecl(Compiler, &yyvsp[-1].token, &yyvsp[0].decl); ;}
    break;

  case 42:
#line 326 "gc_cl.y"
    { yyval.expr = clParseNormalUnaryExpr(Compiler, &yyvsp[-1].token, yyvsp[0].expr); ;}
    break;

  case 43:
#line 331 "gc_cl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 44:
#line 333 "gc_cl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 45:
#line 335 "gc_cl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 46:
#line 337 "gc_cl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 47:
#line 339 "gc_cl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 48:
#line 341 "gc_cl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 49:
#line 346 "gc_cl.y"
    {
		   yyval.expr = yyvsp[0].expr;
		;}
    break;

  case 50:
#line 350 "gc_cl.y"
    { yyval.expr = clParseNormalBinaryExpr(Compiler, yyvsp[-2].expr, &yyvsp[-1].token, yyvsp[0].expr); ;}
    break;

  case 51:
#line 352 "gc_cl.y"
    { yyval.expr = clParseNormalBinaryExpr(Compiler, yyvsp[-2].expr, &yyvsp[-1].token, yyvsp[0].expr); ;}
    break;

  case 52:
#line 354 "gc_cl.y"
    { yyval.expr = clParseNormalBinaryExpr(Compiler, yyvsp[-2].expr, &yyvsp[-1].token, yyvsp[0].expr); ;}
    break;

  case 53:
#line 359 "gc_cl.y"
    { yyval.expr = yyvsp[0].expr; ;}
    break;

  case 54:
#line 361 "gc_cl.y"
    { yyval.expr = clParseNormalBinaryExpr(Compiler, yyvsp[-2].expr, &yyvsp[-1].token, yyvsp[0].expr); ;}
    break;

  case 55:
#line 363 "gc_cl.y"
    { yyval.expr = clParseNormalBinaryExpr(Compiler, yyvsp[-2].expr, &yyvsp[-1].token, yyvsp[0].expr); ;}
    break;

  case 56:
#line 368 "gc_cl.y"
    { yyval.expr = yyvsp[0].expr; ;}
    break;

  case 57:
#line 370 "gc_cl.y"
    { yyval.expr = clParseNormalBinaryExpr(Compiler, yyvsp[-2].expr, &yyvsp[-1].token, yyvsp[0].expr); ;}
    break;

  case 58:
#line 372 "gc_cl.y"
    { yyval.expr = clParseNormalBinaryExpr(Compiler, yyvsp[-2].expr, &yyvsp[-1].token, yyvsp[0].expr); ;}
    break;

  case 59:
#line 377 "gc_cl.y"
    { yyval.expr = yyvsp[0].expr; ;}
    break;

  case 60:
#line 379 "gc_cl.y"
    { yyval.expr = clParseNormalBinaryExpr(Compiler, yyvsp[-2].expr, &yyvsp[-1].token, yyvsp[0].expr); ;}
    break;

  case 61:
#line 381 "gc_cl.y"
    { yyval.expr = clParseNormalBinaryExpr(Compiler, yyvsp[-2].expr, &yyvsp[-1].token, yyvsp[0].expr); ;}
    break;

  case 62:
#line 383 "gc_cl.y"
    { yyval.expr = clParseNormalBinaryExpr(Compiler, yyvsp[-2].expr, &yyvsp[-1].token, yyvsp[0].expr); ;}
    break;

  case 63:
#line 385 "gc_cl.y"
    { yyval.expr = clParseNormalBinaryExpr(Compiler, yyvsp[-2].expr, &yyvsp[-1].token, yyvsp[0].expr); ;}
    break;

  case 64:
#line 390 "gc_cl.y"
    { yyval.expr = yyvsp[0].expr; ;}
    break;

  case 65:
#line 392 "gc_cl.y"
    { yyval.expr = clParseNormalBinaryExpr(Compiler, yyvsp[-2].expr, &yyvsp[-1].token, yyvsp[0].expr); ;}
    break;

  case 66:
#line 394 "gc_cl.y"
    { yyval.expr = clParseNormalBinaryExpr(Compiler, yyvsp[-2].expr, &yyvsp[-1].token, yyvsp[0].expr); ;}
    break;

  case 67:
#line 399 "gc_cl.y"
    { yyval.expr = yyvsp[0].expr; ;}
    break;

  case 68:
#line 401 "gc_cl.y"
    { yyval.expr = clParseNormalBinaryExpr(Compiler, yyvsp[-2].expr, &yyvsp[-1].token, yyvsp[0].expr); ;}
    break;

  case 69:
#line 406 "gc_cl.y"
    { yyval.expr = yyvsp[0].expr; ;}
    break;

  case 70:
#line 408 "gc_cl.y"
    { yyval.expr = clParseNormalBinaryExpr(Compiler, yyvsp[-2].expr, &yyvsp[-1].token, yyvsp[0].expr); ;}
    break;

  case 71:
#line 413 "gc_cl.y"
    { yyval.expr = yyvsp[0].expr; ;}
    break;

  case 72:
#line 415 "gc_cl.y"
    { yyval.expr = clParseNormalBinaryExpr(Compiler, yyvsp[-2].expr, &yyvsp[-1].token, yyvsp[0].expr); ;}
    break;

  case 73:
#line 420 "gc_cl.y"
    { yyval.expr = yyvsp[0].expr; ;}
    break;

  case 74:
#line 422 "gc_cl.y"
    { yyval.expr = clParseNormalBinaryExpr(Compiler, yyvsp[-2].expr, &yyvsp[-1].token, yyvsp[0].expr); ;}
    break;

  case 75:
#line 427 "gc_cl.y"
    { yyval.expr = yyvsp[0].expr; ;}
    break;

  case 76:
#line 429 "gc_cl.y"
    { yyval.expr = clParseNormalBinaryExpr(Compiler, yyvsp[-2].expr, &yyvsp[-1].token, yyvsp[0].expr); ;}
    break;

  case 77:
#line 434 "gc_cl.y"
    { yyval.expr = yyvsp[0].expr; ;}
    break;

  case 78:
#line 436 "gc_cl.y"
    { yyval.expr = clParseNormalBinaryExpr(Compiler, yyvsp[-2].expr, &yyvsp[-1].token, yyvsp[0].expr); ;}
    break;

  case 79:
#line 441 "gc_cl.y"
    { yyval.expr = yyvsp[0].expr; ;}
    break;

  case 80:
#line 443 "gc_cl.y"
    { yyval.expr = clParseSelectionExpr(Compiler, yyvsp[-4].expr, yyvsp[-2].expr, yyvsp[0].expr); ;}
    break;

  case 81:
#line 448 "gc_cl.y"
    { yyval.expr = yyvsp[0].expr;;}
    break;

  case 82:
#line 450 "gc_cl.y"
    { yyval.expr = clParseAssignmentExpr(Compiler, yyvsp[-2].expr, &yyvsp[-1].token, yyvsp[0].expr); ;}
    break;

  case 83:
#line 455 "gc_cl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 84:
#line 457 "gc_cl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 85:
#line 459 "gc_cl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 86:
#line 461 "gc_cl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 87:
#line 463 "gc_cl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 88:
#line 465 "gc_cl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 89:
#line 467 "gc_cl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 90:
#line 469 "gc_cl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 91:
#line 471 "gc_cl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 92:
#line 473 "gc_cl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 93:
#line 475 "gc_cl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 94:
#line 480 "gc_cl.y"
    { yyval.expr = yyvsp[0].expr; ;}
    break;

  case 95:
#line 482 "gc_cl.y"
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
#line 495 "gc_cl.y"
    { yyval.expr = yyvsp[0].expr; ;}
    break;

  case 97:
#line 500 "gc_cl.y"
    { yyval.dataType = clParseTaggedDecl(Compiler, &yyvsp[-1].token, &yyvsp[0].token); ;}
    break;

  case 98:
#line 502 "gc_cl.y"
    { yyval.dataType = clParseTaggedDecl(Compiler, &yyvsp[-1].token, &yyvsp[0].token); ;}
    break;

  case 99:
#line 509 "gc_cl.y"
    { yyval.dataType = clParseEnumSpecifier(Compiler, &yyvsp[-5].token, yyvsp[-4].attr, &yyvsp[-3].token, (gctPOINTER)yyvsp[-1].enumeratorList); ;}
    break;

  case 100:
#line 512 "gc_cl.y"
    { yyval.dataType = clParseEnumSpecifier(Compiler, &yyvsp[-5].token, yyvsp[-4].attr, &yyvsp[-3].token, (gctPOINTER)yyvsp[-1].enumeratorList); ;}
    break;

  case 101:
#line 516 "gc_cl.y"
    { yyval.dataType = clParseEnumSpecifier(Compiler, &yyvsp[-4].token, gcvNULL, &yyvsp[-3].token, (gctPOINTER)yyvsp[-1].enumeratorList); ;}
    break;

  case 102:
#line 519 "gc_cl.y"
    { yyval.dataType = clParseEnumSpecifier(Compiler, &yyvsp[-4].token, gcvNULL, &yyvsp[-3].token, (gctPOINTER)yyvsp[-1].enumeratorList); ;}
    break;

  case 103:
#line 523 "gc_cl.y"
    { yyval.dataType = clParseEnumSpecifier(Compiler, &yyvsp[-4].token, yyvsp[-3].attr, gcvNULL, (gctPOINTER)yyvsp[-1].enumeratorList); ;}
    break;

  case 104:
#line 526 "gc_cl.y"
    { yyval.dataType = clParseEnumSpecifier(Compiler, &yyvsp[-4].token, yyvsp[-3].attr, gcvNULL, (gctPOINTER)yyvsp[-1].enumeratorList); ;}
    break;

  case 105:
#line 530 "gc_cl.y"
    { yyval.dataType = clParseEnumSpecifier(Compiler, &yyvsp[-3].token, gcvNULL, gcvNULL, (gctPOINTER)yyvsp[-1].enumeratorList); ;}
    break;

  case 106:
#line 533 "gc_cl.y"
    { yyval.dataType = clParseEnumSpecifier(Compiler, &yyvsp[-3].token, gcvNULL, gcvNULL, (gctPOINTER)yyvsp[-1].enumeratorList); ;}
    break;

  case 107:
#line 538 "gc_cl.y"
    {
		   slsSLINK_LIST *enumList;

		   slmSLINK_LIST_Initialize(enumList);
                   yyval.enumeratorList = clParseAddEnumerator(Compiler, yyvsp[0].enumeratorName, enumList);
		;}
    break;

  case 108:
#line 545 "gc_cl.y"
    { yyval.enumeratorList = clParseAddEnumerator(Compiler, yyvsp[0].enumeratorName, yyvsp[-2].enumeratorList); ;}
    break;

  case 109:
#line 550 "gc_cl.y"
    { yyval.enumeratorName = clParseEnumerator(Compiler, &yyvsp[0].token, gcvNULL); ;}
    break;

  case 110:
#line 552 "gc_cl.y"
    { yyval.enumeratorName = clParseEnumerator(Compiler, &yyvsp[-2].token, yyvsp[0].expr); ;}
    break;

  case 111:
#line 557 "gc_cl.y"
    { yyval.statement = clParseDeclaration(Compiler, yyvsp[-1].declOrDeclList); ;}
    break;

  case 112:
#line 559 "gc_cl.y"
    { yyval.statement = clParseEnumTags(Compiler, yyvsp[-2].dataType, yyvsp[-1].attr); ;}
    break;

  case 113:
#line 561 "gc_cl.y"
    { yyval.statement = clParseTags(Compiler, yyvsp[-1].dataType); ;}
    break;

  case 114:
#line 563 "gc_cl.y"
    { yyval.statement = clParseTags(Compiler, yyvsp[-1].dataType); ;}
    break;

  case 115:
#line 568 "gc_cl.y"
    { yyval.funcName = yyvsp[-1].funcName; ;}
    break;

  case 116:
#line 573 "gc_cl.y"
    { yyval.funcName = yyvsp[0].funcName; ;}
    break;

  case 117:
#line 575 "gc_cl.y"
    { yyval.funcName = yyvsp[0].funcName; ;}
    break;

  case 118:
#line 580 "gc_cl.y"
    { yyval.funcName = clParseParameterList(Compiler, yyvsp[-1].funcName, yyvsp[0].paramName); ;}
    break;

  case 119:
#line 582 "gc_cl.y"
    { yyval.funcName = clParseParameterList(Compiler, yyvsp[-2].funcName, yyvsp[0].paramName); ;}
    break;

  case 120:
#line 587 "gc_cl.y"
    { yyval.funcName = clParseKernelFuncHeader(Compiler, yyvsp[-3].attr, &yyvsp[-2].decl, &yyvsp[-1].token); ;}
    break;

  case 121:
#line 589 "gc_cl.y"
    { yyval.funcName = clParseExternKernelFuncHeader(Compiler, yyvsp[-3].attr, &yyvsp[-2].decl, &yyvsp[-1].token); ;}
    break;

  case 122:
#line 591 "gc_cl.y"
    { yyval.funcName = clParseFuncHeader(Compiler, &yyvsp[-2].decl, &yyvsp[-1].token); ;}
    break;

  case 123:
#line 593 "gc_cl.y"
    { yyval.funcName = clParseFuncHeaderWithAttr(Compiler, yyvsp[-3].attr, &yyvsp[-2].decl, &yyvsp[-1].token); ;}
    break;

  case 124:
#line 595 "gc_cl.y"
    { yyval.funcName = clParseFuncHeader(Compiler, &yyvsp[-2].decl, &yyvsp[-1].token);
		  if(yyval.funcName) yyval.funcName->u.funcInfo.isInline = gcvTRUE;
		;}
    break;

  case 125:
#line 602 "gc_cl.y"
    { yyval.paramName = clParseParameterDecl(Compiler, &yyvsp[-1].decl, &yyvsp[0].token); ;}
    break;

  case 126:
#line 604 "gc_cl.y"
    { yyval.paramName = clParseArrayParameterDecl(Compiler, &yyvsp[-2].decl, &yyvsp[-1].token, yyvsp[0].expr); ;}
    break;

  case 127:
#line 609 "gc_cl.y"
    { yyval.paramName = clParseQualifiedParameterDecl(Compiler, gcvNULL, gcvNULL, yyvsp[0].paramName); ;}
    break;

  case 128:
#line 611 "gc_cl.y"
    { yyval.paramName = clParseQualifiedParameterDecl(Compiler, gcvNULL, &yyvsp[-1].token, yyvsp[0].paramName); ;}
    break;

  case 129:
#line 613 "gc_cl.y"
    { yyval.paramName = clParseQualifiedParameterDecl(Compiler, gcvNULL, gcvNULL, yyvsp[0].paramName); ;}
    break;

  case 130:
#line 615 "gc_cl.y"
    { yyval.paramName = clParseQualifiedParameterDecl(Compiler, gcvNULL, &yyvsp[-1].token, yyvsp[0].paramName); ;}
    break;

  case 131:
#line 617 "gc_cl.y"
    { yyval.paramName = clParseQualifiedParameterDecl(Compiler, yyvsp[-1].typeQualifierList, gcvNULL, yyvsp[0].paramName); ;}
    break;

  case 132:
#line 619 "gc_cl.y"
    { yyval.paramName = clParseQualifiedParameterDecl(Compiler, yyvsp[-2].typeQualifierList, &yyvsp[-1].token, yyvsp[0].paramName); ;}
    break;

  case 133:
#line 621 "gc_cl.y"
    { yyval.paramName = clParseQualifiedParameterDecl(Compiler, yyvsp[-1].typeQualifierList, gcvNULL, yyvsp[0].paramName); ;}
    break;

  case 134:
#line 623 "gc_cl.y"
    { yyval.paramName = clParseQualifiedParameterDecl(Compiler, yyvsp[-2].typeQualifierList, &yyvsp[-1].token, yyvsp[0].paramName); ;}
    break;

  case 135:
#line 628 "gc_cl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 136:
#line 630 "gc_cl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 137:
#line 635 "gc_cl.y"
    { yyval.paramName = clParseParameterDecl(Compiler, &yyvsp[0].decl, gcvNULL); ;}
    break;

  case 138:
#line 637 "gc_cl.y"
    { yyval.paramName = clParseArrayParameterDecl(Compiler, &yyvsp[-3].decl, gcvNULL, yyvsp[-1].expr); ;}
    break;

  case 139:
#line 642 "gc_cl.y"
    {
		    yyval.typeQualifierList = clParseTypeQualifierList(Compiler, &yyvsp[0].token,
		                                  clParseEmptyTypeQualifierList(Compiler));
		;}
    break;

  case 140:
#line 647 "gc_cl.y"
    {yyval.typeQualifierList = clParseTypeQualifierList(Compiler, &yyvsp[-1].token, yyvsp[0].typeQualifierList); ;}
    break;

  case 141:
#line 652 "gc_cl.y"
    { yyval.typeQualifierList = clParseEmptyTypeQualifierList(Compiler); ;}
    break;

  case 142:
#line 654 "gc_cl.y"
    { yyval.typeQualifierList = clParsePointerTypeQualifier(Compiler, gcvNULL, yyvsp[0].typeQualifierList); ;}
    break;

  case 143:
#line 656 "gc_cl.y"
    {yyval.typeQualifierList = yyvsp[0].typeQualifierList;;}
    break;

  case 144:
#line 658 "gc_cl.y"
    {yyval.typeQualifierList = clParsePointerTypeQualifier(Compiler, yyvsp[-1].typeQualifierList, yyvsp[0].typeQualifierList); ;}
    break;

  case 145:
#line 662 "gc_cl.y"
    {yyval.token = yyvsp[0].token;;}
    break;

  case 146:
#line 664 "gc_cl.y"
    {
		  yyval.token = yyvsp[0].token;
		  yyval.token.u.identifier.ptrDscr = yyvsp[-1].typeQualifierList;
		;}
    break;

  case 147:
#line 671 "gc_cl.y"
    {  yyval.token = yyvsp[0].token; ;}
    break;

  case 148:
#line 673 "gc_cl.y"
    { yyval.token = yyvsp[-1].token; ;}
    break;

  case 149:
#line 677 "gc_cl.y"
    { yyval.expr = clParseNullExpr(Compiler, &yyvsp[0].token); ;}
    break;

  case 150:
#line 679 "gc_cl.y"
    { yyval.expr = yyvsp[0].expr; ;}
    break;

  case 151:
#line 684 "gc_cl.y"
    { yyval.expr = yyvsp[-1].expr; ;}
    break;

  case 152:
#line 686 "gc_cl.y"
    { yyval.expr = clParseArrayDeclarator(Compiler, &yyvsp[-2].token, yyvsp[-3].expr, yyvsp[-1].expr); ;}
    break;

  case 153:
#line 691 "gc_cl.y"
    { yyval.declOrDeclList = yyvsp[0].declOrDeclList; ;}
    break;

  case 154:
#line 693 "gc_cl.y"
    {
    		   cloCOMPILER_PushParserState(Compiler, clvPARSER_IN_TYPEDEF);
		;}
    break;

  case 155:
#line 697 "gc_cl.y"
    {
		   yyval.declOrDeclList = clParseTypeDef(Compiler, yyvsp[0].declOrDeclList);
		   cloCOMPILER_PopParserState(Compiler);
		;}
    break;

  case 156:
#line 702 "gc_cl.y"
    { yyval.declOrDeclList = clParseVariableDeclList(Compiler, yyvsp[-3].declOrDeclList, &yyvsp[-1].token, yyvsp[0].attr); ;}
    break;

  case 157:
#line 704 "gc_cl.y"
    { yyval.declOrDeclList = clParseArrayVariableDeclList(Compiler, yyvsp[-4].declOrDeclList, &yyvsp[-2].token, yyvsp[-1].expr, yyvsp[0].attr); ;}
    break;

  case 158:
#line 706 "gc_cl.y"
    { yyval.declOrDeclList = clParseVariableDeclListInit(Compiler, yyvsp[-4].declOrDeclList, &yyvsp[-2].token, yyvsp[-1].attr); ;}
    break;

  case 159:
#line 708 "gc_cl.y"
    { yyval.declOrDeclList = clParseFinishDeclListInit(Compiler, yyvsp[-1].declOrDeclList, yyvsp[0].expr); ;}
    break;

  case 160:
#line 710 "gc_cl.y"
    { yyval.declOrDeclList = clParseArrayVariableDeclListInit(Compiler, yyvsp[-5].declOrDeclList, &yyvsp[-3].token, yyvsp[-2].expr, yyvsp[-1].attr); ;}
    break;

  case 161:
#line 712 "gc_cl.y"
    { yyval.declOrDeclList = clParseFinishDeclListInit(Compiler, yyvsp[-1].declOrDeclList, yyvsp[0].expr); ;}
    break;

  case 162:
#line 718 "gc_cl.y"
    { yyval.declOrDeclList = clParseFuncDecl(Compiler, yyvsp[0].funcName); ;}
    break;

  case 163:
#line 720 "gc_cl.y"
    { yyval.declOrDeclList = clParseVariableDecl(Compiler, &yyvsp[-2].decl, &yyvsp[-1].token, yyvsp[0].attr); ;}
    break;

  case 164:
#line 722 "gc_cl.y"
    { yyval.declOrDeclList = clParseArrayVariableDecl(Compiler, &yyvsp[-3].decl, &yyvsp[-2].token, yyvsp[-1].expr, yyvsp[0].attr); ;}
    break;

  case 165:
#line 724 "gc_cl.y"
    { yyval.declOrDeclList = clParseVariableDeclInit(Compiler, &yyvsp[-3].decl, &yyvsp[-2].token, yyvsp[-1].attr); ;}
    break;

  case 166:
#line 726 "gc_cl.y"
    { yyval.declOrDeclList = clParseFinishDeclInit(Compiler, yyvsp[-1].declOrDeclList, yyvsp[0].expr); ;}
    break;

  case 167:
#line 728 "gc_cl.y"
    { yyval.declOrDeclList = clParseArrayVariableDeclInit(Compiler, &yyvsp[-4].decl, &yyvsp[-3].token, yyvsp[-2].expr, yyvsp[-1].attr); ;}
    break;

  case 168:
#line 730 "gc_cl.y"
    { yyval.declOrDeclList = clParseFinishDeclInit(Compiler, yyvsp[-1].declOrDeclList, yyvsp[0].expr); ;}
    break;

  case 169:
#line 737 "gc_cl.y"
    { yyval.attr = gcvNULL; ;}
    break;

  case 170:
#line 739 "gc_cl.y"
    { yyval.attr = yyvsp[-2].attr; ;}
    break;

  case 171:
#line 743 "gc_cl.y"
    { yyval.attr = gcvNULL; ;}
    break;

  case 172:
#line 745 "gc_cl.y"
    { yyval.attr = yyvsp[0].attr; ;}
    break;

  case 173:
#line 749 "gc_cl.y"
    { yyval.attr = yyvsp[0].attr; ;}
    break;

  case 174:
#line 751 "gc_cl.y"
    { yyval.attr = yyvsp[0].attr; ;}
    break;

  case 175:
#line 753 "gc_cl.y"
    { yyval.attr = yyvsp[-1].attr; ;}
    break;

  case 176:
#line 755 "gc_cl.y"
    { yyval.attr = yyvsp[0].attr; ;}
    break;

  case 177:
#line 760 "gc_cl.y"
    { yyval.attr = clParseAttributeEndianType(Compiler, yyvsp[-4].attr,  &yyvsp[-1].token); ;}
    break;

  case 178:
#line 762 "gc_cl.y"
    { yyval.attr = clParseSimpleAttribute(Compiler, &yyvsp[0].token, clvATTR_PACKED, yyvsp[-1].attr); ;}
    break;

  case 179:
#line 764 "gc_cl.y"
    { yyval.attr = clParseAttributeVecTypeHint(Compiler, yyvsp[-4].attr, &yyvsp[-1].token); ;}
    break;

  case 180:
#line 766 "gc_cl.y"
    { yyval.attr = clParseAttributeReqdWorkGroupSize(Compiler, yyvsp[-8].attr, yyvsp[-5].expr, yyvsp[-3].expr, yyvsp[-1].expr); ;}
    break;

  case 181:
#line 768 "gc_cl.y"
    { yyval.attr = clParseAttributeWorkGroupSizeHint(Compiler, yyvsp[-8].attr, yyvsp[-5].expr, yyvsp[-3].expr, yyvsp[-1].expr); ;}
    break;

  case 182:
#line 770 "gc_cl.y"
    { yyval.attr = clParseAttributeAligned(Compiler, yyvsp[-1].attr, gcvNULL); ;}
    break;

  case 183:
#line 772 "gc_cl.y"
    { yyval.attr = clParseAttributeAligned(Compiler, yyvsp[-4].attr, yyvsp[-1].expr); ;}
    break;

  case 184:
#line 774 "gc_cl.y"
    { yyval.attr = clParseSimpleAttribute(Compiler, &yyvsp[0].token, clvATTR_ALWAYS_INLINE, yyvsp[-1].attr); ;}
    break;

  case 185:
#line 779 "gc_cl.y"
    { yyval.decl = clParseQualifiedType(Compiler, gcvNULL, gcvFALSE, &yyvsp[0].decl); ;}
    break;

  case 186:
#line 781 "gc_cl.y"
    { yyval.decl = clParseQualifiedType(Compiler, yyvsp[-1].typeQualifierList, gcvFALSE, &yyvsp[0].decl); ;}
    break;

  case 187:
#line 786 "gc_cl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 188:
#line 788 "gc_cl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 189:
#line 790 "gc_cl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 190:
#line 792 "gc_cl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 191:
#line 794 "gc_cl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 192:
#line 796 "gc_cl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 193:
#line 798 "gc_cl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 194:
#line 800 "gc_cl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 195:
#line 802 "gc_cl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 196:
#line 804 "gc_cl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 197:
#line 809 "gc_cl.y"
    { yyval.decl = clParseNonStructType(Compiler, &yyvsp[0].token); ;}
    break;

  case 198:
#line 811 "gc_cl.y"
    { yyval.decl = clParseCreateDeclFromDataType(Compiler, yyvsp[0].dataType); ;}
    break;

  case 199:
#line 813 "gc_cl.y"
    { yyval.decl = clParseCreateDeclFromDataType(Compiler, yyvsp[0].dataType); ;}
    break;

  case 200:
#line 815 "gc_cl.y"
    { yyval.decl = clParseCreateDeclFromDataType(Compiler, yyvsp[0].dataType); ;}
    break;

  case 201:
#line 820 "gc_cl.y"
    { yyval.token = yyvsp[0].token;}
    break;

  case 202:
#line 822 "gc_cl.y"
    { yyval.token = yyvsp[0].token;}
    break;

  case 203:
#line 824 "gc_cl.y"
    { yyval.token = yyvsp[0].token;}
    break;

  case 204:
#line 826 "gc_cl.y"
    { yyval.token = yyvsp[0].token;}
    break;

  case 205:
#line 828 "gc_cl.y"
    { yyval.token = yyvsp[0].token;}
    break;

  case 206:
#line 830 "gc_cl.y"
    { yyval.token = yyvsp[0].token;}
    break;

  case 207:
#line 832 "gc_cl.y"
    { yyval.token = yyvsp[0].token;}
    break;

  case 208:
#line 834 "gc_cl.y"
    { yyval.token = yyvsp[0].token;}
    break;

  case 209:
#line 836 "gc_cl.y"
    { yyval.token = yyvsp[0].token;}
    break;

  case 210:
#line 838 "gc_cl.y"
    { yyval.token = yyvsp[0].token;}
    break;

  case 211:
#line 840 "gc_cl.y"
    { yyval.token = yyvsp[0].token;}
    break;

  case 212:
#line 842 "gc_cl.y"
    { yyval.token = yyvsp[0].token;}
    break;

  case 213:
#line 844 "gc_cl.y"
    { yyval.token = yyvsp[0].token;}
    break;

  case 214:
#line 846 "gc_cl.y"
    { yyval.token = yyvsp[0].token;}
    break;

  case 215:
#line 848 "gc_cl.y"
    { yyval.token = yyvsp[0].token;}
    break;

  case 216:
#line 850 "gc_cl.y"
    { yyval.token = yyvsp[0].token;}
    break;

  case 217:
#line 852 "gc_cl.y"
    { yyval.token = yyvsp[0].token;}
    break;

  case 218:
#line 854 "gc_cl.y"
    { yyval.token = yyvsp[0].token;}
    break;

  case 219:
#line 856 "gc_cl.y"
    { yyval.token = yyvsp[0].token;}
    break;

  case 220:
#line 858 "gc_cl.y"
    { yyval.token = yyvsp[0].token;}
    break;

  case 221:
#line 860 "gc_cl.y"
    { yyval.token = yyvsp[0].token;}
    break;

  case 222:
#line 862 "gc_cl.y"
    { yyval.token = yyvsp[0].token;}
    break;

  case 223:
#line 864 "gc_cl.y"
    { yyval.token = yyvsp[0].token;}
    break;

  case 224:
#line 866 "gc_cl.y"
    { yyval.token = yyvsp[0].token;}
    break;

  case 225:
#line 870 "gc_cl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 226:
#line 872 "gc_cl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 227:
#line 877 "gc_cl.y"
    { clParseStructDeclBegin(Compiler, &yyvsp[-1].token); ;}
    break;

  case 228:
#line 879 "gc_cl.y"
    { yyval.dataType = clParseStructDeclEnd(Compiler, &yyvsp[-6].token, &yyvsp[-5].token, yyvsp[0].attr, yyvsp[-2].status); ;}
    break;

  case 229:
#line 881 "gc_cl.y"
    { clParseStructDeclBegin(Compiler, gcvNULL); ;}
    break;

  case 230:
#line 883 "gc_cl.y"
    { yyval.dataType = clParseStructDeclEnd(Compiler, &yyvsp[-5].token, gcvNULL, yyvsp[0].attr, yyvsp[-2].status); ;}
    break;

  case 231:
#line 885 "gc_cl.y"
    { clParseStructDeclBegin(Compiler, &yyvsp[-1].token); ;}
    break;

  case 232:
#line 887 "gc_cl.y"
    { yyval.dataType = clParseStructDeclEnd(Compiler, &yyvsp[-6].token, &yyvsp[-4].token, yyvsp[-5].attr, yyvsp[-1].status); ;}
    break;

  case 233:
#line 889 "gc_cl.y"
    { clParseStructDeclBegin(Compiler, gcvNULL); ;}
    break;

  case 234:
#line 891 "gc_cl.y"
    { yyval.dataType = clParseStructDeclEnd(Compiler, &yyvsp[-5].token, gcvNULL, yyvsp[-4].attr, yyvsp[-1].status); ;}
    break;

  case 235:
#line 896 "gc_cl.y"
    {
		   yyval.status = yyvsp[0].status;
		;}
    break;

  case 236:
#line 900 "gc_cl.y"
    {
		   if(gcmIS_ERROR(yyvsp[-1].status)) {
                       yyval.status = yyvsp[-1].status;
		   }
		   else {
                       yyval.status = yyvsp[0].status;

		   }
		;}
    break;

  case 237:
#line 913 "gc_cl.y"
    {
                   yyval.status = clParseTypeSpecifiedFieldDeclList(Compiler, &yyvsp[-2].decl, yyvsp[-1].fieldDeclList);
		;}
    break;

  case 238:
#line 920 "gc_cl.y"
    { yyval.fieldDeclList = clParseFieldDeclList(Compiler, yyvsp[0].fieldDecl); ;}
    break;

  case 239:
#line 922 "gc_cl.y"
    { yyval.fieldDeclList = clParseFieldDeclList2(Compiler, yyvsp[-2].fieldDeclList, yyvsp[0].fieldDecl); ;}
    break;

  case 240:
#line 927 "gc_cl.y"
    { yyval.fieldDecl = clParseFieldDecl(Compiler, &yyvsp[-1].token, gcvNULL, yyvsp[0].attr); ;}
    break;

  case 241:
#line 929 "gc_cl.y"
    { yyval.fieldDecl = clParseFieldDecl(Compiler, &yyvsp[-2].token, yyvsp[-1].expr, yyvsp[0].attr); ;}
    break;

  case 242:
#line 934 "gc_cl.y"
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

  case 243:
#line 956 "gc_cl.y"
    { yyval.expr = yyvsp[0].expr; ;}
    break;

  case 244:
#line 958 "gc_cl.y"
    { yyval.expr = yyvsp[-1].expr; ;}
    break;

  case 245:
#line 960 "gc_cl.y"
    { yyval.expr = yyvsp[-1].expr; ;}
    break;

  case 246:
#line 965 "gc_cl.y"
    { yyval.expr = clParseInitializerList(Compiler, yyvsp[-2].expr, yyvsp[-1].declOrDeclList, yyvsp[0].expr); ;}
    break;

  case 247:
#line 967 "gc_cl.y"
    { yyval.expr = clParseInitializerList(Compiler, yyvsp[-3].expr, yyvsp[-1].declOrDeclList, yyvsp[0].expr); ;}
    break;

  case 248:
#line 971 "gc_cl.y"
    { yyval.declOrDeclList = gcvNULL; ;}
    break;

  case 249:
#line 973 "gc_cl.y"
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

  case 250:
#line 986 "gc_cl.y"
    { yyval.declOrDeclList = yyvsp[0].declOrDeclList; ;}
    break;

  case 251:
#line 988 "gc_cl.y"
    {
		   yyval.token.type = T_EOF;
		;}
    break;

  case 252:
#line 992 "gc_cl.y"
    { yyval.declOrDeclList = yyvsp[0].declOrDeclList; ;}
    break;

  case 253:
#line 997 "gc_cl.y"
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

  case 254:
#line 1010 "gc_cl.y"
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

  case 255:
#line 1026 "gc_cl.y"
    { yyval.statement = yyvsp[0].statement; ;}
    break;

  case 256:
#line 1031 "gc_cl.y"
    { yyval.statement = clParseCompoundStatementAsStatement(Compiler, yyvsp[0].statements); ;}
    break;

  case 257:
#line 1033 "gc_cl.y"
    { yyval.statement = yyvsp[0].statement; ;}
    break;

  case 258:
#line 1038 "gc_cl.y"
    { yyval.statement = yyvsp[0].statement; ;}
    break;

  case 259:
#line 1040 "gc_cl.y"
    { yyval.statement = yyvsp[0].statement; ;}
    break;

  case 260:
#line 1042 "gc_cl.y"
    { yyval.statement = yyvsp[0].statement; ;}
    break;

  case 261:
#line 1044 "gc_cl.y"
    { yyval.statement = yyvsp[0].statement; ;}
    break;

  case 262:
#line 1046 "gc_cl.y"
    { yyval.statement = yyvsp[0].statement; ;}
    break;

  case 263:
#line 1048 "gc_cl.y"
    { yyval.statement = clParseStatementLabel(Compiler, &yyvsp[-1].token); ;}
    break;

  case 264:
#line 1050 "gc_cl.y"
    { yyclearin;
		  yyerrok;
		  yyval.statement = gcvNULL; ;}
    break;

  case 265:
#line 1054 "gc_cl.y"
    { yyclearin;
		  yyerrok;
		  yyval.statement = gcvNULL; ;}
    break;

  case 266:
#line 1061 "gc_cl.y"
    { yyval.statements = gcvNULL; ;}
    break;

  case 267:
#line 1063 "gc_cl.y"
    { clParseCompoundStatementBegin(Compiler); ;}
    break;

  case 268:
#line 1065 "gc_cl.y"
    { yyval.statements = clParseCompoundStatementEnd(Compiler, &yyvsp[-3].token, yyvsp[-1].statements); ;}
    break;

  case 269:
#line 1070 "gc_cl.y"
    { yyval.statement = clParseCompoundStatementNoNewScopeAsStatementNoNewScope(Compiler, yyvsp[0].statements); ;}
    break;

  case 270:
#line 1072 "gc_cl.y"
    { yyval.statement = yyvsp[0].statement; ;}
    break;

  case 271:
#line 1077 "gc_cl.y"
    { yyval.statements = gcvNULL; ;}
    break;

  case 272:
#line 1079 "gc_cl.y"
    { clParseCompoundStatementNoNewScopeBegin(Compiler); ;}
    break;

  case 273:
#line 1081 "gc_cl.y"
    { yyval.statements = clParseCompoundStatementNoNewScopeEnd(Compiler, &yyvsp[-3].token, yyvsp[-1].statements); ;}
    break;

  case 274:
#line 1086 "gc_cl.y"
    { yyval.statements = clParseStatementList(Compiler, yyvsp[0].statement); ;}
    break;

  case 275:
#line 1088 "gc_cl.y"
    { yyval.statements = clParseStatementList2(Compiler, yyvsp[-1].statements, yyvsp[0].statement); ;}
    break;

  case 276:
#line 1093 "gc_cl.y"
    { yyval.statement = gcvNULL; ;}
    break;

  case 277:
#line 1095 "gc_cl.y"
    { yyval.statement = clParseExprAsStatement(Compiler, yyvsp[-1].expr); ;}
    break;

  case 278:
#line 1100 "gc_cl.y"
    { yyval.statement = clParseIfStatement(Compiler, &yyvsp[-4].token, yyvsp[-2].expr, yyvsp[0].ifStatementPair); ;}
    break;

  case 279:
#line 1102 "gc_cl.y"
    { yyval.statement = clParseSwitchStatement(Compiler, &yyvsp[-4].token, yyvsp[-2].expr, yyvsp[0].statement); ;}
    break;

  case 280:
#line 1107 "gc_cl.y"
    { yyval.statements = clParseStatementList(Compiler, yyvsp[0].statement); ;}
    break;

  case 281:
#line 1109 "gc_cl.y"
    { yyval.statements = clParseStatementList2(Compiler, yyvsp[-1].statements, yyvsp[0].statement); ;}
    break;

  case 282:
#line 1114 "gc_cl.y"
    { yyval.statement = yyvsp[0].statement; ;}
    break;

  case 283:
#line 1116 "gc_cl.y"
    { yyval.statement = clParseCaseStatement(Compiler, &yyvsp[-2].token, yyvsp[-1].expr); ;}
    break;

  case 284:
#line 1118 "gc_cl.y"
    { yyval.statement = clParseDefaultStatement(Compiler, &yyvsp[-1].token); ;}
    break;

  case 285:
#line 1123 "gc_cl.y"
    { yyval.statement = gcvNULL; ;}
    break;

  case 286:
#line 1125 "gc_cl.y"
    { clParseSwitchBodyBegin(Compiler); ;}
    break;

  case 287:
#line 1127 "gc_cl.y"
    { yyval.statement = clParseSwitchBodyEnd(Compiler, &yyvsp[-3].token, yyvsp[-1].statements); ;}
    break;

  case 288:
#line 1132 "gc_cl.y"
    { yyval.ifStatementPair = clParseIfSubStatements(Compiler, yyvsp[-2].statement, yyvsp[0].statement); ;}
    break;

  case 289:
#line 1134 "gc_cl.y"
    { yyval.ifStatementPair = clParseIfSubStatements(Compiler, yyvsp[0].statement, gcvNULL); ;}
    break;

  case 290:
#line 1141 "gc_cl.y"
    { clParseWhileStatementBegin(Compiler); ;}
    break;

  case 291:
#line 1143 "gc_cl.y"
    { yyval.statement = clParseWhileStatementEnd(Compiler, &yyvsp[-5].token, yyvsp[-2].expr, yyvsp[0].statement); ;}
    break;

  case 292:
#line 1145 "gc_cl.y"
    { yyval.statement = clParseDoWhileStatement(Compiler, &yyvsp[-6].token, yyvsp[-5].statement, yyvsp[-2].expr); ;}
    break;

  case 293:
#line 1147 "gc_cl.y"
    { clParseForStatementBegin(Compiler); ;}
    break;

  case 294:
#line 1149 "gc_cl.y"
    { yyval.statement = clParseForStatementEnd(Compiler, &yyvsp[-4].token, yyvsp[-2].statement, yyvsp[-1].forExprPair, yyvsp[0].statement); ;}
    break;

  case 295:
#line 1154 "gc_cl.y"
    { yyval.statement = yyvsp[0].statement; ;}
    break;

  case 296:
#line 1156 "gc_cl.y"
    { yyval.statement = yyvsp[0].statement; ;}
    break;

  case 297:
#line 1158 "gc_cl.y"
    { yyclearin;
		  yyerrok;
		  yyval.statement = gcvNULL; ;}
    break;

  case 298:
#line 1165 "gc_cl.y"
    { yyval.expr = yyvsp[0].expr; ;}
    break;

  case 299:
#line 1167 "gc_cl.y"
    { yyval.expr = gcvNULL; ;}
    break;

  case 300:
#line 1172 "gc_cl.y"
    { yyval.forExprPair = clParseForControl(Compiler, yyvsp[-2].expr, gcvNULL); ;}
    break;

  case 301:
#line 1174 "gc_cl.y"
    { yyval.forExprPair = clParseForControl(Compiler, yyvsp[-3].expr, yyvsp[-1].expr); ;}
    break;

  case 302:
#line 1176 "gc_cl.y"
    {
		  clsForExprPair nullPair = {gcvNULL, gcvNULL};
		  yyclearin;
		  yyerrok;
		  yyval.forExprPair = nullPair; ;}
    break;

  case 303:
#line 1182 "gc_cl.y"
    {
		  clsForExprPair nullPair = {gcvNULL, gcvNULL};
		  yyclearin;
		  yyerrok;
		  yyval.forExprPair = nullPair; ;}
    break;

  case 304:
#line 1191 "gc_cl.y"
    { yyval.statement = clParseJumpStatement(Compiler, clvCONTINUE, &yyvsp[-1].token, gcvNULL); ;}
    break;

  case 305:
#line 1193 "gc_cl.y"
    { yyval.statement = clParseJumpStatement(Compiler, clvBREAK, &yyvsp[-1].token, gcvNULL); ;}
    break;

  case 306:
#line 1195 "gc_cl.y"
    { yyval.statement = clParseJumpStatement(Compiler, clvRETURN, &yyvsp[-1].token, gcvNULL); ;}
    break;

  case 307:
#line 1197 "gc_cl.y"
    { yyval.statement = clParseJumpStatement(Compiler, clvRETURN, &yyvsp[-2].token, yyvsp[-1].expr); ;}
    break;

  case 308:
#line 1199 "gc_cl.y"
    { yyval.statement = clParseGotoStatement(Compiler, &yyvsp[-2].token, &yyvsp[-1].token); ;}
    break;

  case 312:
#line 1212 "gc_cl.y"
    { clParseExternalDecl(Compiler, yyvsp[0].statement); ;}
    break;

  case 314:
#line 1218 "gc_cl.y"
    { clParseFuncDef(Compiler, yyvsp[-1].funcName, yyvsp[0].statements); ;}
    break;


    }

/* Line 993 of yacc.c.  */
#line 4329 "gc_cl_parser.c"

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


#line 1221 "gc_cl.y"



