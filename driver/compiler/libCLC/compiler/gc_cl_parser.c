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
     T_IMAGE2D_PTR_T = 350,
     T_SIZE_T = 351,
     T_EVENT_T = 352,
     T_PTRDIFF_T = 353,
     T_INTPTR_T = 354,
     T_UINTPTR_T = 355,
     T_GENTYPE = 356,
     T_F_GENTYPE = 357,
     T_IU_GENTYPE = 358,
     T_I_GENTYPE = 359,
     T_U_GENTYPE = 360,
     T_SIU_GENTYPE = 361,
     T_BOOL_PACKED = 362,
     T_BOOL2_PACKED = 363,
     T_BOOL3_PACKED = 364,
     T_BOOL4_PACKED = 365,
     T_BOOL8_PACKED = 366,
     T_BOOL16_PACKED = 367,
     T_BOOL32_PACKED = 368,
     T_CHAR_PACKED = 369,
     T_CHAR2_PACKED = 370,
     T_CHAR3_PACKED = 371,
     T_CHAR4_PACKED = 372,
     T_CHAR8_PACKED = 373,
     T_CHAR16_PACKED = 374,
     T_CHAR32_PACKED = 375,
     T_UCHAR_PACKED = 376,
     T_UCHAR2_PACKED = 377,
     T_UCHAR3_PACKED = 378,
     T_UCHAR4_PACKED = 379,
     T_UCHAR8_PACKED = 380,
     T_UCHAR16_PACKED = 381,
     T_UCHAR32_PACKED = 382,
     T_SHORT_PACKED = 383,
     T_SHORT2_PACKED = 384,
     T_SHORT3_PACKED = 385,
     T_SHORT4_PACKED = 386,
     T_SHORT8_PACKED = 387,
     T_SHORT16_PACKED = 388,
     T_SHORT32_PACKED = 389,
     T_USHORT_PACKED = 390,
     T_USHORT2_PACKED = 391,
     T_USHORT3_PACKED = 392,
     T_USHORT4_PACKED = 393,
     T_USHORT8_PACKED = 394,
     T_USHORT16_PACKED = 395,
     T_USHORT32_PACKED = 396,
     T_HALF_PACKED = 397,
     T_HALF2_PACKED = 398,
     T_HALF3_PACKED = 399,
     T_HALF4_PACKED = 400,
     T_HALF8_PACKED = 401,
     T_HALF16_PACKED = 402,
     T_HALF32_PACKED = 403,
     T_GENTYPE_PACKED = 404,
     T_FLOATNXM = 405,
     T_DOUBLENXM = 406,
     T_BUILTIN_DATA_TYPE = 407,
     T_RESERVED_DATA_TYPE = 408,
     T_VIV_PACKED_DATA_TYPE = 409,
     T_IDENTIFIER = 410,
     T_TYPE_NAME = 411,
     T_FLOATCONSTANT = 412,
     T_UINTCONSTANT = 413,
     T_INTCONSTANT = 414,
     T_BOOLCONSTANT = 415,
     T_CHARCONSTANT = 416,
     T_STRING_LITERAL = 417,
     T_FIELD_SELECTION = 418,
     T_LSHIFT_OP = 419,
     T_RSHIFT_OP = 420,
     T_INC_OP = 421,
     T_DEC_OP = 422,
     T_LE_OP = 423,
     T_GE_OP = 424,
     T_EQ_OP = 425,
     T_NE_OP = 426,
     T_AND_OP = 427,
     T_OR_OP = 428,
     T_XOR_OP = 429,
     T_MUL_ASSIGN = 430,
     T_DIV_ASSIGN = 431,
     T_ADD_ASSIGN = 432,
     T_MOD_ASSIGN = 433,
     T_LEFT_ASSIGN = 434,
     T_RIGHT_ASSIGN = 435,
     T_AND_ASSIGN = 436,
     T_XOR_ASSIGN = 437,
     T_OR_ASSIGN = 438,
     T_SUB_ASSIGN = 439,
     T_STRUCT_UNION_PTR = 440,
     T_INITIALIZER_END = 441,
     T_BREAK = 442,
     T_CONTINUE = 443,
     T_RETURN = 444,
     T_GOTO = 445,
     T_WHILE = 446,
     T_FOR = 447,
     T_DO = 448,
     T_ELSE = 449,
     T_IF = 450,
     T_SWITCH = 451,
     T_CASE = 452,
     T_DEFAULT = 453,
     T_CONST = 454,
     T_RESTRICT = 455,
     T_VOLATILE = 456,
     T_STATIC = 457,
     T_EXTERN = 458,
     T_CONSTANT = 459,
     T_GLOBAL = 460,
     T_LOCAL = 461,
     T_PRIVATE = 462,
     T_KERNEL = 463,
     T_UNIFORM = 464,
     T_READ_ONLY = 465,
     T_WRITE_ONLY = 466,
     T_PACKED = 467,
     T_ALIGNED = 468,
     T_ENDIAN = 469,
     T_VEC_TYPE_HINT = 470,
     T_ATTRIBUTE__ = 471,
     T_REQD_WORK_GROUP_SIZE = 472,
     T_WORK_GROUP_SIZE_HINT = 473,
     T_ALWAYS_INLINE = 474,
     T_UNSIGNED = 475,
     T_STRUCT = 476,
     T_UNION = 477,
     T_TYPEDEF = 478,
     T_ENUM = 479,
     T_INLINE = 480,
     T_SIZEOF = 481,
     T_TYPE_CAST = 482,
     T_VEC_STEP = 483,
     T_VERY_LAST_TERMINAL = 484
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
#define T_IMAGE2D_PTR_T 350
#define T_SIZE_T 351
#define T_EVENT_T 352
#define T_PTRDIFF_T 353
#define T_INTPTR_T 354
#define T_UINTPTR_T 355
#define T_GENTYPE 356
#define T_F_GENTYPE 357
#define T_IU_GENTYPE 358
#define T_I_GENTYPE 359
#define T_U_GENTYPE 360
#define T_SIU_GENTYPE 361
#define T_BOOL_PACKED 362
#define T_BOOL2_PACKED 363
#define T_BOOL3_PACKED 364
#define T_BOOL4_PACKED 365
#define T_BOOL8_PACKED 366
#define T_BOOL16_PACKED 367
#define T_BOOL32_PACKED 368
#define T_CHAR_PACKED 369
#define T_CHAR2_PACKED 370
#define T_CHAR3_PACKED 371
#define T_CHAR4_PACKED 372
#define T_CHAR8_PACKED 373
#define T_CHAR16_PACKED 374
#define T_CHAR32_PACKED 375
#define T_UCHAR_PACKED 376
#define T_UCHAR2_PACKED 377
#define T_UCHAR3_PACKED 378
#define T_UCHAR4_PACKED 379
#define T_UCHAR8_PACKED 380
#define T_UCHAR16_PACKED 381
#define T_UCHAR32_PACKED 382
#define T_SHORT_PACKED 383
#define T_SHORT2_PACKED 384
#define T_SHORT3_PACKED 385
#define T_SHORT4_PACKED 386
#define T_SHORT8_PACKED 387
#define T_SHORT16_PACKED 388
#define T_SHORT32_PACKED 389
#define T_USHORT_PACKED 390
#define T_USHORT2_PACKED 391
#define T_USHORT3_PACKED 392
#define T_USHORT4_PACKED 393
#define T_USHORT8_PACKED 394
#define T_USHORT16_PACKED 395
#define T_USHORT32_PACKED 396
#define T_HALF_PACKED 397
#define T_HALF2_PACKED 398
#define T_HALF3_PACKED 399
#define T_HALF4_PACKED 400
#define T_HALF8_PACKED 401
#define T_HALF16_PACKED 402
#define T_HALF32_PACKED 403
#define T_GENTYPE_PACKED 404
#define T_FLOATNXM 405
#define T_DOUBLENXM 406
#define T_BUILTIN_DATA_TYPE 407
#define T_RESERVED_DATA_TYPE 408
#define T_VIV_PACKED_DATA_TYPE 409
#define T_IDENTIFIER 410
#define T_TYPE_NAME 411
#define T_FLOATCONSTANT 412
#define T_UINTCONSTANT 413
#define T_INTCONSTANT 414
#define T_BOOLCONSTANT 415
#define T_CHARCONSTANT 416
#define T_STRING_LITERAL 417
#define T_FIELD_SELECTION 418
#define T_LSHIFT_OP 419
#define T_RSHIFT_OP 420
#define T_INC_OP 421
#define T_DEC_OP 422
#define T_LE_OP 423
#define T_GE_OP 424
#define T_EQ_OP 425
#define T_NE_OP 426
#define T_AND_OP 427
#define T_OR_OP 428
#define T_XOR_OP 429
#define T_MUL_ASSIGN 430
#define T_DIV_ASSIGN 431
#define T_ADD_ASSIGN 432
#define T_MOD_ASSIGN 433
#define T_LEFT_ASSIGN 434
#define T_RIGHT_ASSIGN 435
#define T_AND_ASSIGN 436
#define T_XOR_ASSIGN 437
#define T_OR_ASSIGN 438
#define T_SUB_ASSIGN 439
#define T_STRUCT_UNION_PTR 440
#define T_INITIALIZER_END 441
#define T_BREAK 442
#define T_CONTINUE 443
#define T_RETURN 444
#define T_GOTO 445
#define T_WHILE 446
#define T_FOR 447
#define T_DO 448
#define T_ELSE 449
#define T_IF 450
#define T_SWITCH 451
#define T_CASE 452
#define T_DEFAULT 453
#define T_CONST 454
#define T_RESTRICT 455
#define T_VOLATILE 456
#define T_STATIC 457
#define T_EXTERN 458
#define T_CONSTANT 459
#define T_GLOBAL 460
#define T_LOCAL 461
#define T_PRIVATE 462
#define T_KERNEL 463
#define T_UNIFORM 464
#define T_READ_ONLY 465
#define T_WRITE_ONLY 466
#define T_PACKED 467
#define T_ALIGNED 468
#define T_ENDIAN 469
#define T_VEC_TYPE_HINT 470
#define T_ATTRIBUTE__ 471
#define T_REQD_WORK_GROUP_SIZE 472
#define T_WORK_GROUP_SIZE_HINT 473
#define T_ALWAYS_INLINE 474
#define T_UNSIGNED 475
#define T_STRUCT 476
#define T_UNION 477
#define T_TYPEDEF 478
#define T_ENUM 479
#define T_INLINE 480
#define T_SIZEOF 481
#define T_TYPE_CAST 482
#define T_VEC_STEP 483
#define T_VERY_LAST_TERMINAL 484




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
#line 581 "gc_cl_parser.c"
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif



/* Copy the second part of user declarations.  */


/* Line 214 of yacc.c.  */
#line 593 "gc_cl_parser.c"

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
#define YYFINAL  104
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   3294

/* YYNTOKENS -- Number of terminals. */
#define YYNTOKENS  254
/* YYNNTS -- Number of nonterminals. */
#define YYNNTS  110
/* YYNRULES -- Number of rules. */
#define YYNRULES  313
/* YYNRULES -- Number of states. */
#define YYNSTATES  521

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   484

#define YYTRANSLATE(YYX) 						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const unsigned char yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   241,     2,     2,     2,   247,   252,     2,
     230,   231,   245,   244,   237,   242,   236,   246,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,   238,   240,
     248,   239,   249,   253,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,   232,     2,   233,   251,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   234,   250,   235,   243,     2,     2,     2,
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
     225,   226,   227,   228,   229
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
     677,   679,   681,   683,   685,   687,   689,   690,   698,   699,
     706,   707,   715,   716,   723,   725,   728,   732,   734,   738,
     741,   745,   747,   749,   753,   757,   760,   765,   766,   769,
     771,   772,   776,   780,   783,   785,   787,   789,   791,   793,
     795,   797,   799,   802,   805,   808,   811,   812,   817,   819,
     821,   824,   825,   830,   832,   835,   837,   840,   846,   852,
     854,   857,   859,   863,   866,   869,   870,   875,   879,   881,
     882,   889,   897,   898,   904,   907,   910,   914,   916,   917,
     921,   926,   930,   935,   938,   941,   944,   948,   952,   954,
     957,   959,   961,   963
};

/* YYRHS -- A `-1'-separated list of the rules' RHS. */
static const short yyrhs[] =
{
     361,     0,    -1,   155,    -1,   162,    -1,   256,   162,    -1,
     255,    -1,   158,    -1,   159,    -1,   157,    -1,   160,    -1,
     161,    -1,   256,    -1,   230,   283,   231,    -1,   257,    -1,
     258,   232,   259,   233,    -1,   260,    -1,   258,   236,   163,
      -1,   258,   185,   163,    -1,   258,   166,    -1,   258,   167,
      -1,   283,    -1,   261,   231,    -1,   263,     4,   231,    -1,
     263,   231,    -1,   263,   281,    -1,   261,   237,   281,    -1,
     230,   317,   231,    -1,   230,   317,   299,   231,    -1,   155,
     230,    -1,   262,   234,    -1,   267,    -1,    -1,   262,   266,
     265,    -1,   264,   283,   235,    -1,   264,   283,   186,    -1,
     258,    -1,   166,   267,    -1,   167,   267,    -1,   228,   230,
     267,   231,    -1,   228,   262,    -1,   226,   267,    -1,   226,
     262,    -1,   268,   265,    -1,   244,    -1,   242,    -1,   241,
      -1,   243,    -1,   252,    -1,   245,    -1,   265,    -1,   269,
     245,   265,    -1,   269,   246,   265,    -1,   269,   247,   265,
      -1,   269,    -1,   270,   244,   269,    -1,   270,   242,   269,
      -1,   270,    -1,   271,   164,   270,    -1,   271,   165,   270,
      -1,   271,    -1,   272,   248,   271,    -1,   272,   249,   271,
      -1,   272,   168,   271,    -1,   272,   169,   271,    -1,   272,
      -1,   273,   170,   272,    -1,   273,   171,   272,    -1,   273,
      -1,   274,   252,   273,    -1,   274,    -1,   275,   251,   274,
      -1,   275,    -1,   276,   250,   275,    -1,   276,    -1,   277,
     172,   276,    -1,   277,    -1,   278,   174,   277,    -1,   278,
      -1,   279,   173,   278,    -1,   279,    -1,   279,   253,   283,
     238,   281,    -1,   280,    -1,   267,   282,   281,    -1,   239,
      -1,   175,    -1,   176,    -1,   178,    -1,   177,    -1,   184,
      -1,   179,    -1,   180,    -1,   181,    -1,   182,    -1,   183,
      -1,   281,    -1,   283,   237,   281,    -1,   280,    -1,   321,
     155,    -1,   224,   155,    -1,   224,   311,   155,   234,   287,
     235,    -1,   224,   311,   155,   234,   287,   186,    -1,   224,
     155,   234,   287,   235,    -1,   224,   155,   234,   287,   186,
      -1,   224,   311,   234,   287,   235,    -1,   224,   311,   234,
     287,   186,    -1,   224,   234,   287,   235,    -1,   224,   234,
     287,   186,    -1,   288,    -1,   287,   237,   288,    -1,   155,
      -1,   155,   239,   284,    -1,   304,   240,    -1,   286,   313,
     240,    -1,   285,   240,    -1,   322,   240,    -1,   291,   231,
      -1,   293,    -1,   292,    -1,   293,   295,    -1,   292,   237,
     295,    -1,   208,   313,   317,   301,   230,    -1,   203,   208,
     313,   317,   301,   230,    -1,   317,   301,   230,    -1,   311,
     317,   301,   230,    -1,   225,   317,   301,   230,    -1,   319,
     301,    -1,   319,   301,   303,    -1,   294,    -1,   296,   294,
      -1,   297,    -1,   296,   297,    -1,   298,   294,    -1,   298,
     296,   294,    -1,   298,   297,    -1,   298,   296,   297,    -1,
     210,    -1,   211,    -1,   319,    -1,   319,   232,   284,   233,
      -1,   318,    -1,   318,   298,    -1,   245,    -1,   245,   299,
      -1,   245,   298,    -1,   245,   298,   299,    -1,   155,    -1,
     299,   155,    -1,   300,    -1,   230,   300,   231,    -1,    -1,
     284,    -1,   232,   302,   233,    -1,   303,   232,   302,   233,
      -1,   308,    -1,    -1,   223,   305,   308,    -1,   304,   237,
     301,   313,    -1,   304,   237,   301,   303,   313,    -1,    -1,
     304,   237,   301,   313,   239,   306,   332,    -1,    -1,   304,
     237,   301,   303,   313,   239,   307,   332,    -1,   290,    -1,
     317,   301,   313,    -1,   317,   301,   303,   313,    -1,    -1,
     317,   301,   313,   239,   309,   332,    -1,    -1,   317,   301,
     303,   313,   239,   310,   332,    -1,    -1,   216,   230,   230,
     312,   314,   231,   231,    -1,    -1,   311,    -1,    -1,   316,
      -1,    -1,   314,   237,   315,   316,    -1,   214,   230,   155,
     231,    -1,   212,    -1,   215,   230,   152,   231,    -1,   217,
     230,   284,   237,   284,   237,   284,   231,    -1,   218,   230,
     284,   237,   284,   237,   284,   231,    -1,   213,    -1,   213,
     230,   284,   231,    -1,   219,    -1,   319,    -1,   298,   319,
      -1,   199,    -1,   200,    -1,   201,    -1,   204,    -1,   205,
      -1,   206,    -1,   207,    -1,   202,    -1,   203,    -1,   209,
      -1,   320,    -1,   322,    -1,   285,    -1,   286,    -1,     4,
      -1,    22,    -1,    64,    -1,    10,    -1,   150,    -1,   151,
      -1,   152,    -1,   153,    -1,   154,    -1,    89,    -1,    90,
      -1,    91,    -1,    92,    -1,    93,    -1,    95,    -1,    94,
      -1,    88,    -1,    98,    -1,    99,    -1,   100,    -1,    96,
      -1,    97,    -1,   156,    -1,   221,    -1,   222,    -1,    -1,
     321,   155,   234,   323,   327,   235,   313,    -1,    -1,   321,
     234,   324,   327,   235,   313,    -1,    -1,   321,   311,   155,
     234,   325,   327,   235,    -1,    -1,   321,   311,   234,   326,
     327,   235,    -1,   328,    -1,   327,   328,    -1,   317,   329,
     240,    -1,   330,    -1,   329,   237,   330,    -1,   301,   313,
      -1,   301,   303,   313,    -1,   234,    -1,   281,    -1,   331,
     333,   235,    -1,   331,   333,   186,    -1,   334,   332,    -1,
     333,   237,   334,   332,    -1,    -1,   335,   239,    -1,   337,
      -1,    -1,   335,   336,   337,    -1,   232,   284,   233,    -1,
     236,   163,    -1,   289,    -1,   341,    -1,   340,    -1,   338,
      -1,   347,    -1,   348,    -1,   354,    -1,   360,    -1,   155,
     238,    -1,     1,   240,    -1,     1,   235,    -1,   234,   235,
      -1,    -1,   234,   342,   346,   235,    -1,   344,    -1,   340,
      -1,   234,   235,    -1,    -1,   234,   345,   346,   235,    -1,
     339,    -1,   346,   339,    -1,   240,    -1,   283,   240,    -1,
     195,   230,   283,   231,   353,    -1,   196,   230,   259,   231,
     351,    -1,   350,    -1,   349,   350,    -1,   339,    -1,   197,
     284,   238,    -1,   198,   238,    -1,   234,   235,    -1,    -1,
     234,   352,   349,   235,    -1,   339,   194,   339,    -1,   339,
      -1,    -1,   191,   355,   230,   283,   231,   343,    -1,   193,
     339,   191,   230,   283,   231,   240,    -1,    -1,   192,   356,
     357,   359,   343,    -1,   230,   347,    -1,   230,   338,    -1,
     230,     1,   240,    -1,   283,    -1,    -1,   358,   240,   231,
      -1,   358,   240,   283,   231,    -1,     1,   240,   231,    -1,
       1,   240,   283,   231,    -1,   188,   240,    -1,   187,   240,
      -1,   189,   240,    -1,   189,   283,   240,    -1,   190,   155,
     240,    -1,   362,    -1,   361,   362,    -1,   363,    -1,   289,
      -1,   240,    -1,   290,   344,    -1
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
     857,   859,   861,   863,   867,   869,   875,   874,   879,   878,
     883,   882,   887,   886,   893,   897,   910,   917,   919,   924,
     926,   931,   953,   955,   957,   962,   964,   969,   970,   983,
     986,   985,   994,  1007,  1023,  1028,  1030,  1035,  1037,  1039,
    1041,  1043,  1045,  1047,  1051,  1058,  1061,  1060,  1067,  1069,
    1074,  1077,  1076,  1083,  1085,  1090,  1092,  1097,  1099,  1104,
    1106,  1111,  1113,  1115,  1120,  1123,  1122,  1129,  1131,  1139,
    1138,  1142,  1145,  1144,  1151,  1153,  1155,  1162,  1165,  1169,
    1171,  1173,  1179,  1188,  1190,  1192,  1194,  1196,  1203,  1204,
    1208,  1209,  1211,  1215
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
  "T_IMAGE2D_T", "T_IMAGE3D_T", "T_IMAGE2D_PTR_T", "T_SIZE_T", "T_EVENT_T",
  "T_PTRDIFF_T", "T_INTPTR_T", "T_UINTPTR_T", "T_GENTYPE", "T_F_GENTYPE",
  "T_IU_GENTYPE", "T_I_GENTYPE", "T_U_GENTYPE", "T_SIU_GENTYPE",
  "T_BOOL_PACKED", "T_BOOL2_PACKED", "T_BOOL3_PACKED", "T_BOOL4_PACKED",
  "T_BOOL8_PACKED", "T_BOOL16_PACKED", "T_BOOL32_PACKED", "T_CHAR_PACKED",
  "T_CHAR2_PACKED", "T_CHAR3_PACKED", "T_CHAR4_PACKED", "T_CHAR8_PACKED",
  "T_CHAR16_PACKED", "T_CHAR32_PACKED", "T_UCHAR_PACKED",
  "T_UCHAR2_PACKED", "T_UCHAR3_PACKED", "T_UCHAR4_PACKED",
  "T_UCHAR8_PACKED", "T_UCHAR16_PACKED", "T_UCHAR32_PACKED",
  "T_SHORT_PACKED", "T_SHORT2_PACKED", "T_SHORT3_PACKED",
  "T_SHORT4_PACKED", "T_SHORT8_PACKED", "T_SHORT16_PACKED",
  "T_SHORT32_PACKED", "T_USHORT_PACKED", "T_USHORT2_PACKED",
  "T_USHORT3_PACKED", "T_USHORT4_PACKED", "T_USHORT8_PACKED",
  "T_USHORT16_PACKED", "T_USHORT32_PACKED", "T_HALF_PACKED",
  "T_HALF2_PACKED", "T_HALF3_PACKED", "T_HALF4_PACKED", "T_HALF8_PACKED",
  "T_HALF16_PACKED", "T_HALF32_PACKED", "T_GENTYPE_PACKED", "T_FLOATNXM",
  "T_DOUBLENXM", "T_BUILTIN_DATA_TYPE", "T_RESERVED_DATA_TYPE",
  "T_VIV_PACKED_DATA_TYPE", "T_IDENTIFIER", "T_TYPE_NAME",
  "T_FLOATCONSTANT", "T_UINTCONSTANT", "T_INTCONSTANT", "T_BOOLCONSTANT",
  "T_CHARCONSTANT", "T_STRING_LITERAL", "T_FIELD_SELECTION", "T_LSHIFT_OP",
  "T_RSHIFT_OP", "T_INC_OP", "T_DEC_OP", "T_LE_OP", "T_GE_OP", "T_EQ_OP",
  "T_NE_OP", "T_AND_OP", "T_OR_OP", "T_XOR_OP", "T_MUL_ASSIGN",
  "T_DIV_ASSIGN", "T_ADD_ASSIGN", "T_MOD_ASSIGN", "T_LEFT_ASSIGN",
  "T_RIGHT_ASSIGN", "T_AND_ASSIGN", "T_XOR_ASSIGN", "T_OR_ASSIGN",
  "T_SUB_ASSIGN", "T_STRUCT_UNION_PTR", "T_INITIALIZER_END", "T_BREAK",
  "T_CONTINUE", "T_RETURN", "T_GOTO", "T_WHILE", "T_FOR", "T_DO", "T_ELSE",
  "T_IF", "T_SWITCH", "T_CASE", "T_DEFAULT", "T_CONST", "T_RESTRICT",
  "T_VOLATILE", "T_STATIC", "T_EXTERN", "T_CONSTANT", "T_GLOBAL",
  "T_LOCAL", "T_PRIVATE", "T_KERNEL", "T_UNIFORM", "T_READ_ONLY",
  "T_WRITE_ONLY", "T_PACKED", "T_ALIGNED", "T_ENDIAN", "T_VEC_TYPE_HINT",
  "T_ATTRIBUTE__", "T_REQD_WORK_GROUP_SIZE", "T_WORK_GROUP_SIZE_HINT",
  "T_ALWAYS_INLINE", "T_UNSIGNED", "T_STRUCT", "T_UNION", "T_TYPEDEF",
  "T_ENUM", "T_INLINE", "T_SIZEOF", "T_TYPE_CAST", "T_VEC_STEP",
  "T_VERY_LAST_TERMINAL", "'('", "')'", "'['", "']'", "'{'", "'}'", "'.'",
  "','", "':'", "'='", "';'", "'!'", "'-'", "'~'", "'+'", "'*'", "'/'",
  "'%'", "'<'", "'>'", "'|'", "'^'", "'&'", "'?'", "$accept",
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
      40,    41,    91,    93,   123,   125,    46,    44,    58,    61,
      59,    33,    45,   126,    43,    42,    47,    37,    60,    62,
     124,    94,    38,    63
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const unsigned short yyr1[] =
{
       0,   254,   255,   256,   256,   257,   257,   257,   257,   257,
     257,   257,   257,   258,   258,   258,   258,   258,   258,   258,
     259,   260,   260,   260,   261,   261,   262,   262,   263,   264,
     265,   266,   265,   265,   265,   267,   267,   267,   267,   267,
     267,   267,   267,   268,   268,   268,   268,   268,   268,   269,
     269,   269,   269,   270,   270,   270,   271,   271,   271,   272,
     272,   272,   272,   272,   273,   273,   273,   274,   274,   275,
     275,   276,   276,   277,   277,   278,   278,   279,   279,   280,
     280,   281,   281,   282,   282,   282,   282,   282,   282,   282,
     282,   282,   282,   282,   283,   283,   284,   285,   285,   286,
     286,   286,   286,   286,   286,   286,   286,   287,   287,   288,
     288,   289,   289,   289,   289,   290,   291,   291,   292,   292,
     293,   293,   293,   293,   293,   294,   294,   295,   295,   295,
     295,   295,   295,   295,   295,   296,   296,   297,   297,   298,
     298,   299,   299,   299,   299,   300,   300,   301,   301,   302,
     302,   303,   303,   304,   305,   304,   304,   304,   306,   304,
     307,   304,   308,   308,   308,   309,   308,   310,   308,   312,
     311,   313,   313,   314,   314,   315,   314,   316,   316,   316,
     316,   316,   316,   316,   316,   317,   317,   318,   318,   318,
     318,   318,   318,   318,   318,   318,   318,   319,   319,   319,
     319,   320,   320,   320,   320,   320,   320,   320,   320,   320,
     320,   320,   320,   320,   320,   320,   320,   320,   320,   320,
     320,   320,   320,   320,   321,   321,   323,   322,   324,   322,
     325,   322,   326,   322,   327,   327,   328,   329,   329,   330,
     330,   331,   332,   332,   332,   333,   333,   334,   334,   335,
     336,   335,   337,   337,   338,   339,   339,   340,   340,   340,
     340,   340,   340,   340,   340,   341,   342,   341,   343,   343,
     344,   345,   344,   346,   346,   347,   347,   348,   348,   349,
     349,   350,   350,   350,   351,   352,   351,   353,   353,   355,
     354,   354,   356,   354,   357,   357,   357,   358,   358,   359,
     359,   359,   359,   360,   360,   360,   360,   360,   361,   361,
     362,   362,   362,   363
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
       1,     1,     1,     1,     1,     1,     0,     7,     0,     6,
       0,     7,     0,     6,     1,     2,     3,     1,     3,     2,
       3,     1,     1,     3,     3,     2,     4,     0,     2,     1,
       0,     3,     3,     2,     1,     1,     1,     1,     1,     1,
       1,     1,     2,     2,     2,     2,     0,     4,     1,     1,
       2,     0,     4,     1,     2,     1,     2,     5,     5,     1,
       2,     1,     3,     2,     2,     0,     4,     3,     1,     0,
       6,     7,     0,     5,     2,     2,     3,     1,     0,     3,
       4,     3,     4,     2,     2,     2,     3,     3,     1,     2,
       1,     1,     1,     2
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const unsigned short yydefact[] =
{
       0,   201,   204,   202,   203,   217,   210,   211,   212,   213,
     214,   216,   215,   221,   222,   218,   219,   220,   205,   206,
     207,   208,   209,   223,   187,   188,   189,   194,   195,   190,
     191,   192,   193,   171,   196,     0,   224,   225,   154,     0,
       0,   312,   199,   200,   311,   162,     0,   117,   116,     0,
       0,   153,     0,     0,   139,   185,   197,     0,   198,     0,
     308,   310,   171,   172,     0,     0,     0,    98,     0,     0,
     195,   199,   200,     0,   198,   113,     0,   271,   313,   115,
       0,   135,   136,   127,   118,     0,   129,     0,   137,   186,
       0,   111,     0,   145,     0,   141,     0,   147,   171,   140,
      97,   228,     0,   114,     1,   309,     0,     0,   169,   162,
     155,     0,   109,     0,   107,     0,     0,     0,   112,   270,
       0,   119,   128,   130,   131,     0,   133,     0,   125,   171,
       0,     0,   143,   142,   146,   122,   149,   171,   163,   226,
       0,     0,   232,     0,     0,   173,     0,     0,   106,   105,
       0,     0,     0,   124,     0,     2,     8,     6,     7,     9,
      10,     3,     0,     0,     0,     0,     0,     0,   289,   292,
       0,     0,     0,     0,     0,     0,   266,   275,    45,    44,
      46,    43,    48,    47,     5,    11,    13,    35,    15,     0,
      31,     0,     0,    49,    30,     0,    53,    56,    59,    64,
      67,    69,    71,    73,    75,    77,    79,    81,    94,     0,
     254,   257,   273,   256,   255,     0,   258,   259,   260,   261,
     132,   134,     2,    30,    96,     0,   126,   171,   156,   123,
     148,   144,   150,     0,   149,   164,   165,     0,     0,     0,
     234,   230,     0,     0,   120,   178,   182,     0,     0,     0,
       0,   184,     0,   174,   102,   101,   110,   108,     0,   104,
     103,   264,   263,    28,   262,     0,    36,    37,   304,   303,
     305,     0,     0,     0,     0,     0,     0,     0,    41,    40,
       0,    39,     0,     0,   265,     0,     4,    18,    19,     0,
       0,     0,    21,     0,    29,     0,     0,    23,    24,     0,
      84,    85,    87,    86,    89,    90,    91,    92,    93,    88,
      83,     0,    42,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   276,   272,   274,   138,   157,   158,
     151,     0,   167,     0,     0,   171,     0,   237,   171,   235,
       0,     0,   121,     0,     0,     0,     0,     0,     0,   175,
     100,    99,   306,   307,     0,     0,     0,     0,     0,     0,
      20,     0,    12,    26,     0,     0,    17,     0,    16,    25,
      32,    22,    34,    33,    82,    50,    51,    52,    55,    54,
      57,    58,    62,    63,    60,    61,    65,    66,    68,    70,
      72,    74,    76,    78,     0,    95,   160,     0,   152,     0,
     241,   242,   247,   166,   171,   171,   239,     0,   236,   229,
       0,   233,     0,     0,     0,     0,     0,   170,     0,     0,
       0,   295,   294,     0,   297,     0,     0,     0,     0,     0,
      38,    27,   267,    14,     0,     0,   159,   168,     0,     0,
       0,     0,   250,   249,   227,   240,   238,   231,   183,   177,
     179,     0,     0,   176,     0,   296,     0,     0,   269,   293,
     268,     0,   288,   277,   285,   278,    80,   161,     0,   253,
     244,   243,   247,   245,   248,     0,     0,     0,   290,   301,
       0,   299,     0,     0,     0,   284,     0,   252,     0,   251,
       0,     0,   302,   300,   291,   287,     0,     0,   281,     0,
     279,   246,     0,     0,     0,   283,   286,   280,   180,   181,
     282
};

/* YYDEFGOTO[NTERM-NUM]. */
static const short yydefgoto[] =
{
      -1,   184,   185,   186,   187,   369,   188,   189,   190,   191,
     192,   193,   295,   194,   195,   196,   197,   198,   199,   200,
     201,   202,   203,   204,   205,   206,   207,   208,   311,   209,
     232,    71,    72,   113,   114,   210,   109,    46,    47,    48,
      83,    84,    85,    86,    49,    96,    97,   345,   233,   137,
      50,    66,   407,   445,    51,   343,   409,    52,   145,    64,
     252,   428,   253,    53,    54,    55,    56,    57,    74,   237,
     140,   350,   242,   239,   240,   346,   347,   412,   413,   450,
     451,   452,   485,   453,   211,   212,   213,   214,   285,   469,
     470,   120,   215,   216,   217,   509,   510,   475,   496,   473,
     218,   273,   274,   366,   435,   436,   219,    59,    60,    61
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -414
static const short yypact[] =
{
    2080,  -414,  -414,  -414,  -414,  -414,  -414,  -414,  -414,  -414,
    -414,  -414,  -414,  -414,  -414,  -414,  -414,  -414,  -414,  -414,
    -414,  -414,  -414,  -414,  -414,  -414,  -414,  -414,  -152,  -414,
    -414,  -414,  -414,  -171,  -414,  -178,  -414,  -414,  -414,  -112,
    2917,  -414,  -176,  -174,  -414,  -172,  -102,   -73,  2766,  3070,
    -124,  -414,  2917,  -127,   120,  -414,  -414,  -109,  -144,  1983,
    -414,  -414,  -171,  -414,  2917,   -47,  2669,   -49,    51,  -117,
    -414,  -414,  -414,  -127,  -414,  -414,   -30,    -4,  -414,  -414,
    2766,  -414,  -414,  -414,  -414,  3070,  -414,  2932,  -135,  -414,
    -127,  -414,  -127,  -414,  -146,   222,    65,  -414,  -162,  -414,
      16,  -414,  -107,  -414,  -414,  -414,  2917,  -127,  -414,  -414,
    -414,    51,    13,   -32,  -414,    20,    51,    28,  -414,  -414,
    1132,  -414,  -414,  -414,  -414,  3070,  -414,  2400,    47,  -175,
      68,    69,    85,  -414,  -414,  -414,  2400,  -163,    71,  -414,
    2917,    97,  -414,  -127,   102,   257,   -23,  2400,  -414,  -414,
      51,    51,   -14,  -414,    -1,  -194,  -414,  -414,  -414,  -414,
    -414,  -414,  3041,  3041,    93,    94,  1700,   181,  -414,  -414,
    1132,   108,   110,  2400,   113,  1647,   111,  -414,  -414,  -414,
    -414,  -414,  -414,  -414,  -414,   183,  -414,    10,  -414,  -150,
     116,  1758,  2400,  -414,   178,  2400,    37,  -156,    44,  -147,
      54,    95,   100,   117,   191,   190,  -141,  -414,  -414,   -40,
    -414,  -414,  -414,  -414,  -414,   788,  -414,  -414,  -414,  -414,
    -414,  -414,   138,  -414,  -414,   137,   139,  -163,   133,  -414,
    -414,  -414,  -414,   140,  2400,   135,  -414,  2917,  -127,  2233,
    -414,  -414,  2917,   152,  -414,  -414,   159,   160,   162,   165,
     166,  -414,   -82,  -414,  -414,  -414,  -414,  -414,    -5,  -414,
    -414,  -414,  -414,  -414,  -414,  2400,  -414,  -414,  -414,  -414,
    -414,    -2,   157,   168,   169,   209,  2400,  2400,  -414,  -414,
    1812,  -414,   -53,   -78,  -414,  1132,  -414,  -414,  -414,   238,
    2400,   241,  -414,  2400,  -414,  2400,   174,  -414,  -414,    12,
    -414,  -414,  -414,  -414,  -414,  -414,  -414,  -414,  -414,  -414,
    -414,  2400,  -414,  2400,  2400,  2400,  2400,  2400,  2400,  2400,
    2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,
    2400,  2400,  2400,  2400,  -414,  -414,  -414,  -414,   167,  -414,
    -414,   176,  -414,   -83,  2330,  -175,    11,  -414,  -171,  -414,
    2917,  2481,  -414,  2400,   255,   259,  2400,  2400,   182,  -414,
    -414,  -414,  -414,  -414,  2400,  1476,  1422,   184,   -45,   187,
     175,   188,  -414,  -414,   189,   960,  -414,   197,  -414,  -414,
    -414,  -414,  -414,  -414,  -414,  -414,  -414,  -414,    37,    37,
    -156,  -156,    44,    44,    44,    44,  -147,  -147,    54,    95,
     100,   117,   191,   190,    19,  -414,  -414,   -83,  -414,   -83,
    -414,  -414,   -80,  -414,  -171,  -163,  -414,  -127,  -414,  -414,
    2518,  -414,   201,   202,   203,   198,   204,  -414,   257,   -44,
     210,  -414,  -414,   211,   175,   213,  1304,  2400,  1132,   215,
    -414,  -414,  -414,  -414,  2400,   -83,  -414,  -414,  2400,   252,
      43,   -83,   205,  -414,  -414,  -414,  -414,  -414,  -414,  -414,
    -414,  2400,  2400,  -414,  1304,  -414,  1865,  2945,  -414,  -414,
    -414,   -43,   249,  -414,   220,  -414,  -414,  -414,   223,  -414,
    -414,  -414,   -80,  -414,  -414,   -80,   224,   225,  -414,  -414,
     -20,  -414,    -9,   217,  1132,  -414,   616,  -414,   -83,  -414,
    2400,  2400,  -414,  -414,  -414,  -414,  2400,   221,  -414,   444,
    -414,  -414,   229,   232,   235,  -414,  -414,  -414,  -414,  -414,
    -414
};

/* YYPGOTO[NTERM-NUM].  */
static const short yypgoto[] =
{
    -414,  -414,  -414,  -414,  -414,   192,  -414,  -414,   114,  -414,
    -414,  -180,  -414,   -54,  -414,   -12,    -6,   -27,   -18,   151,
     153,   150,   154,   149,   155,  -414,  -120,  -154,  -414,   126,
    -113,     0,     1,   -61,   331,    30,    35,  -414,  -414,  -414,
      41,   404,   398,    57,   -15,   -92,   393,    58,   254,  -118,
    -414,  -414,  -414,  -414,   423,  -414,  -414,   -31,  -414,   -37,
    -414,  -414,    63,   -35,  -414,    88,  -414,  -414,     4,  -414,
    -414,  -414,  -414,  -218,  -221,  -414,    76,  -414,  -360,  -414,
      17,  -414,  -414,    18,   136,  -157,  -413,  -414,  -414,    38,
     459,  -414,   226,   141,  -414,  -414,     3,  -414,  -414,  -414,
    -414,  -414,  -414,  -414,  -414,  -414,  -414,  -414,   446,  -414
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -299
static const short yytable[] =
{
      42,    43,    63,   133,    58,    73,    76,   224,    69,    93,
     226,   227,    63,   275,   225,   312,   224,    92,   349,   344,
      93,   320,   321,   468,   351,   106,   102,   224,    93,   107,
      44,    63,   331,    87,   256,    45,   263,   298,   115,    99,
     231,    35,    35,    67,   264,    35,   100,   446,   141,   447,
     146,   468,    65,    35,    35,   152,    62,   136,   336,    42,
      43,   138,    77,    58,    75,    87,  -171,    63,   135,   234,
     136,   143,   222,   223,   156,   157,   158,   159,   160,   161,
     132,   292,   223,   162,   163,   477,   316,   293,   317,    44,
     258,   483,   228,   223,    45,    94,   103,   127,    63,    95,
     235,   322,   323,    94,    35,   238,    63,    35,   266,   267,
      95,    98,   332,    90,   224,   380,    91,   116,    95,   279,
      42,    43,    68,   349,    58,   101,   122,   142,   124,    79,
     349,   117,   420,   385,   386,   387,    88,    89,   511,   379,
     283,   223,   123,   173,   126,   174,   128,   175,   129,   358,
     130,   410,   448,   373,   148,   359,   449,   384,   178,   179,
     180,   181,   182,   254,    80,   144,   220,    95,    88,   183,
      42,    43,   259,    88,    58,    88,   287,   288,   372,   405,
     223,   360,   221,   108,   333,   111,   438,   464,   493,   411,
     338,   374,   333,   333,   333,   289,    63,   333,   382,   349,
     334,   243,   238,   149,   238,   150,   112,   238,   318,   319,
     118,   502,   255,    88,   150,    42,    43,   333,   336,    58,
     134,   260,   503,   150,   324,   325,   371,   415,   333,   480,
     361,   119,   150,   224,   261,   333,   224,   224,   362,   262,
     422,   223,   290,   425,   426,   283,   291,   383,   417,   333,
     139,   418,   147,   411,   151,   411,   333,   444,   153,   223,
     223,   223,   223,   223,   223,   223,   223,   223,   223,   223,
     223,   223,   223,   223,   223,   223,   223,   223,   481,   136,
     482,   472,   313,   314,   315,    42,    43,   278,   281,    58,
     476,   411,   271,   392,   393,   394,   395,   411,   229,   223,
     230,   282,   223,   223,   388,   389,   396,   397,   416,   238,
     236,   419,   390,   391,    63,   238,   238,    63,   299,    24,
      25,    26,    27,    70,    29,    30,    31,    32,   224,    34,
      95,   241,   244,   268,   269,   478,   272,   505,   276,   508,
     277,   224,   224,   280,   411,   286,   284,   326,   486,   487,
     294,   327,   508,   300,   301,   302,   303,   304,   305,   306,
     307,   308,   309,   329,   330,    42,    43,   328,   263,    58,
     337,   234,   339,   340,   342,    42,    43,   454,   455,    58,
     224,   224,   352,    63,    63,   238,   224,   512,   513,   353,
     354,   282,   355,   514,   223,   356,   357,   363,   364,   365,
     367,   376,   368,   370,   378,   381,   406,   223,   223,   408,
     423,   424,   333,   427,   437,   479,   370,   310,   439,   440,
     441,    24,    25,    26,    27,    70,    29,    30,    31,    32,
     443,    34,   458,   459,   460,   461,    42,    43,    42,    43,
      58,   462,    58,   494,   484,   154,   223,   223,     1,   474,
     465,   466,   223,   467,     2,   495,   497,   504,   404,   515,
     518,   500,   501,   519,    42,    43,     3,    95,    58,   245,
     246,   247,   248,   520,   249,   250,   251,   398,   400,   402,
     399,   257,   377,   401,   121,   125,   403,   131,   341,   110,
     429,   463,   434,   456,    42,    43,    42,    43,    58,   498,
      58,   431,   488,   499,    78,   105,   432,     0,     4,    42,
      43,   375,   517,    58,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     5,     6,     7,     8,     9,    10,    11,    12,
      13,    14,    15,    16,    17,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   471,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   490,   492,    18,    19,    20,    21,    22,   155,
      23,   156,   157,   158,   159,   160,   161,     0,     0,     0,
     162,   163,     0,     0,     0,     0,     0,   154,     0,     0,
       1,     0,     0,     0,     0,     0,     2,     0,     0,     0,
       0,   164,   165,   166,   167,   168,   169,   170,     3,   171,
     172,   506,   507,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,     0,     0,     0,     0,     0,     0,
      35,     0,     0,     0,     0,    36,    37,    38,    39,    40,
     173,     0,   174,     0,   175,     0,     0,     0,   176,   516,
       4,     0,     0,     0,   177,   178,   179,   180,   181,   182,
       0,     0,     0,     0,     0,     0,   183,     0,     0,     0,
       0,     0,     0,     0,     5,     6,     7,     8,     9,    10,
      11,    12,    13,    14,    15,    16,    17,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    18,    19,    20,    21,
      22,   155,    23,   156,   157,   158,   159,   160,   161,     0,
       0,     0,   162,   163,     0,     0,     0,     0,     0,   154,
       0,     0,     1,     0,     0,     0,     0,     0,     2,     0,
       0,     0,     0,   164,   165,   166,   167,   168,   169,   170,
       3,   171,   172,   506,   507,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,     0,     0,     0,     0,
       0,     0,    35,     0,     0,     0,     0,    36,    37,    38,
      39,    40,   173,     0,   174,     0,   175,     0,     0,     0,
     176,     0,     4,     0,     0,     0,   177,   178,   179,   180,
     181,   182,     0,     0,     0,     0,     0,     0,   183,     0,
       0,     0,     0,     0,     0,     0,     5,     6,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    17,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    18,    19,
      20,    21,    22,   155,    23,   156,   157,   158,   159,   160,
     161,     0,     0,     0,   162,   163,     0,     0,     0,     0,
       0,   154,     0,     0,     1,     0,     0,     0,     0,     0,
       2,     0,     0,     0,     0,   164,   165,   166,   167,   168,
     169,   170,     3,   171,   172,     0,     0,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    34,     0,     0,
       0,     0,     0,     0,    35,     0,     0,     0,     0,    36,
      37,    38,    39,    40,   173,     0,   174,     0,   175,     0,
       0,     0,   176,   335,     4,     0,     0,     0,   177,   178,
     179,   180,   181,   182,     0,     0,     0,     0,     0,     0,
     183,     0,     0,     0,     0,     0,     0,     0,     5,     6,
       7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
      17,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      18,    19,    20,    21,    22,   155,    23,   156,   157,   158,
     159,   160,   161,     0,     0,     0,   162,   163,     0,     0,
       0,     0,     0,   154,     0,     0,     1,     0,     0,     0,
       0,     0,     2,     0,     0,     0,     0,   164,   165,   166,
     167,   168,   169,   170,     3,   171,   172,     0,     0,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
       0,     0,     0,     0,     0,     0,    35,     0,     0,     0,
       0,    36,    37,    38,    39,    40,   173,     0,   174,     0,
     175,     0,     0,     0,   176,   442,     4,     0,     0,     0,
     177,   178,   179,   180,   181,   182,     0,     0,     0,     0,
       0,     0,   183,     0,     0,     0,     0,     0,     0,     0,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    18,    19,    20,    21,    22,   155,    23,   156,
     157,   158,   159,   160,   161,     0,     0,     0,   162,   163,
       0,     0,     0,     0,     0,   154,     0,     0,     1,     0,
       0,     0,     0,     0,     2,     0,     0,     0,     0,   164,
     165,   166,   167,   168,   169,   170,     3,   171,   172,     0,
       0,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,     0,     0,     0,     0,     0,     0,    35,     0,
       0,     0,     0,    36,    37,    38,    39,    40,   173,     0,
     174,     0,   175,     0,     0,     0,   176,     0,     4,     0,
       0,     0,   177,   178,   179,   180,   181,   182,     0,     0,
       0,     0,     0,     0,   183,     0,     0,     0,     0,     0,
       0,     0,     5,     6,     7,     8,     9,    10,    11,    12,
      13,    14,    15,    16,    17,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   433,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    18,    19,    20,    21,    22,   155,
      23,   156,   157,   158,   159,   160,   161,     0,     0,     0,
     162,   163,     0,     0,     0,     0,     0,   430,     0,     0,
       1,     0,     0,     0,     0,     0,     2,     0,     0,     0,
       0,   164,   165,   166,   167,   168,   169,   170,     3,   171,
     172,     0,     0,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,     0,     0,     0,     0,     0,     0,
      35,     0,     0,     0,     0,    36,    37,    38,    39,    40,
     173,     0,   174,     0,   175,     0,     0,     0,    77,     0,
       4,     0,     0,     0,   177,   178,   179,   180,   181,   182,
       0,     0,     0,     0,     0,     0,   183,     0,     0,     0,
       0,     0,     0,     0,     5,     6,     7,     8,     9,    10,
      11,    12,    13,    14,    15,    16,    17,   222,     0,   156,
     157,   158,   159,   160,   161,     0,     0,     0,   162,   163,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    18,    19,    20,    21,
      22,   222,    23,   156,   157,   158,   159,   160,   161,     0,
       0,     0,   162,   163,     0,     0,     0,     0,   173,     0,
     174,     1,   175,     0,     0,     0,     0,     2,     0,     0,
       0,     0,  -298,   178,   179,   180,   181,   182,     0,     3,
       0,     0,     0,     0,   183,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,     0,     0,     0,     0,
       0,     0,    35,     0,     0,     0,     0,    36,    37,    38,
      39,    40,   173,     0,   174,     0,   175,     0,     0,     0,
       0,     4,     0,     0,     0,     0,   177,   178,   179,   180,
     181,   182,     0,     0,     0,     0,     0,     0,   183,     0,
       0,     0,     0,     0,     0,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   296,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    18,    19,    20,
      21,    22,   222,    23,   156,   157,   158,   159,   160,   161,
       0,     0,     0,   162,   163,     0,     1,     0,     0,     0,
       0,     0,     2,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     3,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    24,    25,    26,    27,
      70,    29,    30,    31,    32,   222,    34,   156,   157,   158,
     159,   160,   161,     0,     0,     0,   162,   163,    36,    37,
       0,    39,     0,   173,     0,   174,     4,   175,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   178,   179,
     180,   181,   182,     0,     0,     0,     0,     0,     0,   183,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,   222,     0,   156,   157,   158,   159,   160,
     161,     0,     0,     0,   162,   163,   173,     0,   174,     0,
     175,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     270,   178,   179,   180,   181,   182,     0,     0,     0,     0,
       0,     0,   183,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    18,    19,    20,    21,    22,   222,    23,   156,
     157,   158,   159,   160,   161,     0,     0,     0,   162,   163,
       0,     0,     0,   104,   173,     0,   174,     1,   175,   297,
       0,     0,     0,     2,     0,     0,     0,     0,     0,   178,
     179,   180,   181,   182,     0,     3,     0,     0,     0,     0,
     183,    24,    25,    26,    27,    70,    29,    30,    31,    32,
     222,    34,   156,   157,   158,   159,   160,   161,     0,     0,
       0,   162,   163,    36,    37,     0,    39,     0,   173,     0,
     174,     0,   265,     0,     0,     0,     0,     4,     0,     0,
       0,     0,     0,   178,   179,   180,   181,   182,     0,     0,
       0,     0,     0,     0,   183,     0,     0,     0,     0,     0,
       0,     5,     6,     7,     8,     9,    10,    11,    12,    13,
      14,    15,    16,    17,     1,     0,     0,     0,     0,     0,
       2,   173,     0,   174,     0,   175,   489,     0,     0,     0,
       0,     0,     3,     0,     0,     0,   178,   179,   180,   181,
     182,     0,     0,     0,     0,     0,     0,   183,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    18,    19,    20,    21,    22,     0,    23,
       0,     0,     0,     0,     4,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     5,     6,
       7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
      17,     0,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,     0,     0,     0,     0,     0,     0,    35,
       0,     0,     0,     0,    36,    37,    38,    39,    40,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    41,     0,     0,     0,     0,     0,     0,
      18,    19,    20,    21,    22,     0,    23,     1,     0,     0,
       0,     0,     0,     2,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     3,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
       0,     0,     0,     0,     0,     0,    35,     4,     0,     0,
       0,    36,    37,    38,    39,    40,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      41,     5,     6,     7,     8,     9,    10,    11,    12,    13,
      14,    15,    16,    17,     1,     0,     0,     0,     0,     0,
       2,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     3,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    18,    19,    20,    21,    22,     0,    23,
       0,     0,     0,     0,     4,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     5,     6,
       7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
      17,     0,    24,    25,    26,    27,    70,    29,    30,    31,
      32,     0,    34,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    36,    37,     0,    39,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   348,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      18,    19,    20,    21,    22,     1,    23,     0,     0,     0,
       0,     2,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     3,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     1,     0,     0,     0,     0,     0,     2,    24,
      25,    26,    27,    70,    29,    30,    31,    32,     0,    34,
       3,     0,     0,     0,     0,     4,     0,     0,     0,     0,
       0,    36,    37,     0,    39,   222,     0,   156,   157,   158,
     159,   160,   161,     0,     0,   414,   162,   163,     0,     5,
       6,     7,     8,     9,    10,    11,    12,    13,    14,    15,
      16,    17,     4,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     5,     6,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    17,     0,
       0,     0,     0,     0,     0,     0,   173,     0,   174,     0,
     175,    18,    19,    20,    21,    22,     0,    23,     0,     0,
       0,   178,   179,   180,   181,   182,     0,     0,     0,     0,
       0,     0,   183,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    18,    19,
      20,    21,    22,     1,    23,     0,     0,     0,     0,     2,
      24,    25,    26,    27,    70,    29,    30,    31,    32,     0,
      34,     3,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    36,    37,     0,    39,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   421,    24,    25,    26,
      27,    70,    29,    30,    31,    32,     0,    34,     0,     0,
       0,     0,     0,     4,     0,     0,     0,     0,     0,    36,
      37,     0,    39,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   457,     0,     0,     0,     5,     6,     7,
       8,     9,    10,    11,    12,    13,    14,    15,    16,    17,
       1,     0,     0,     0,     0,     0,     2,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     3,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    18,
      19,    20,    21,    22,     0,    23,     0,     0,     0,     0,
       4,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     5,     6,     7,     8,     9,    10,
      11,    12,    13,    14,    15,    16,    17,     0,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,     0,
       0,     0,     0,     0,     0,    35,     0,     0,     0,     0,
      36,    37,     0,    39,    40,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    18,    19,    20,    21,
      22,     1,    23,     0,     0,     0,     0,     2,     0,     0,
       0,     0,     0,     0,     0,     0,     1,     0,     0,     3,
       0,     0,     2,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     3,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    24,    25,    26,    27,    70,
      29,    30,    31,    32,     0,    34,    81,    82,     0,     0,
       0,     4,     0,     0,     0,     0,     0,    36,    37,     0,
      39,     0,     0,     0,     0,     0,     4,     0,     0,     0,
       0,     0,     0,     0,     0,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,     0,     0,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    18,    19,    20,
      21,    22,     0,    23,     1,     0,     0,     0,     0,     0,
       2,     0,    18,    19,    20,    21,    22,     0,    23,     0,
       0,     0,     3,     0,     0,     0,     0,     0,     0,     0,
     222,     0,   156,   157,   158,   159,   160,   161,     0,     0,
       0,   162,   163,     0,     0,     0,    24,    25,    26,    27,
      70,    29,    30,    31,    32,     0,    34,     0,     0,     0,
       0,     0,     0,     0,     4,     0,     0,     0,    36,    37,
       0,    39,    81,    82,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    36,    37,     0,    39,     0,     5,     6,
       7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
      17,   173,     0,   174,     0,   175,   491,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   178,   179,   180,   181,
     182,     0,     0,     0,     0,     0,   222,   183,   156,   157,
     158,   159,   160,   161,     0,     0,     0,   162,   163,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      18,    19,    20,    21,    22,     0,    23,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   173,     0,   174,
       0,   265,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   178,   179,   180,   181,   182,     0,     0,     0,
       0,    36,    37,   183,    39
};

static const short yycheck[] =
{
       0,     0,    33,    95,     0,    40,    43,   127,    39,   155,
     128,   129,    43,   170,   127,   195,   136,    52,   239,   237,
     155,   168,   169,   436,   242,    62,    57,   147,   155,    64,
       0,    62,   173,    48,   147,     0,   230,   191,   155,    54,
     132,   216,   216,   155,   238,   216,   155,   407,   155,   409,
     111,   464,   230,   216,   216,   116,   208,   232,   215,    59,
      59,    98,   234,    59,   240,    80,   240,    98,   230,   232,
     232,   106,   155,   127,   157,   158,   159,   160,   161,   162,
      95,   231,   136,   166,   167,   445,   242,   237,   244,    59,
     151,   451,   129,   147,    59,   230,   240,   232,   129,   245,
     137,   248,   249,   230,   216,   140,   137,   216,   162,   163,
     245,    53,   253,   237,   234,   295,   240,   234,   245,   173,
     120,   120,   234,   344,   120,   234,    85,   234,    87,   231,
     351,    73,   350,   313,   314,   315,    48,    49,   498,   293,
     175,   195,    85,   226,    87,   228,    88,   230,    90,   231,
      92,   234,   232,   231,   186,   237,   236,   311,   241,   242,
     243,   244,   245,   186,   237,   107,   125,   245,    80,   252,
     170,   170,   186,    85,   170,    87,   166,   167,   231,   333,
     234,   186,   125,   230,   237,   234,   231,   231,   231,   343,
     227,   283,   237,   237,   237,   185,   227,   237,   186,   420,
     240,   143,   237,   235,   239,   237,   155,   242,   164,   165,
     240,   231,   235,   125,   237,   215,   215,   237,   375,   215,
     155,   235,   231,   237,   170,   171,   280,   345,   237,   186,
     235,   235,   237,   353,   235,   237,   356,   357,   240,   240,
     353,   295,   232,   356,   357,   280,   236,   235,   237,   237,
     234,   240,   239,   407,   234,   409,   237,   238,   230,   313,
     314,   315,   316,   317,   318,   319,   320,   321,   322,   323,
     324,   325,   326,   327,   328,   329,   330,   331,   235,   232,
     237,   438,   245,   246,   247,   285,   285,   173,   174,   285,
     444,   445,   166,   320,   321,   322,   323,   451,   230,   353,
     231,   175,   356,   357,   316,   317,   324,   325,   345,   344,
     239,   348,   318,   319,   345,   350,   351,   348,   192,   199,
     200,   201,   202,   203,   204,   205,   206,   207,   448,   209,
     245,   234,   230,   240,   240,   448,   155,   494,   230,   496,
     230,   461,   462,   230,   498,   162,   235,   252,   461,   462,
     234,   251,   509,   175,   176,   177,   178,   179,   180,   181,
     182,   183,   184,   172,   174,   365,   365,   250,   230,   365,
     233,   232,   239,   233,   239,   375,   375,   414,   415,   375,
     500,   501,   230,   414,   415,   420,   506,   500,   501,   230,
     230,   265,   230,   506,   448,   230,   230,   240,   230,   230,
     191,   163,   276,   277,   163,   231,   239,   461,   462,   233,
     155,   152,   237,   231,   230,   163,   290,   239,   231,   231,
     231,   199,   200,   201,   202,   203,   204,   205,   206,   207,
     233,   209,   231,   231,   231,   237,   436,   436,   438,   438,
     436,   237,   438,   194,   239,     1,   500,   501,     4,   234,
     240,   240,   506,   240,    10,   235,   233,   240,   332,   238,
     231,   237,   237,   231,   464,   464,    22,   245,   464,   212,
     213,   214,   215,   238,   217,   218,   219,   326,   328,   330,
     327,   150,   290,   329,    80,    87,   331,    94,   234,    66,
     364,   428,   366,   417,   494,   494,   496,   496,   494,   482,
     496,   365,   464,   485,    45,    59,   365,    -1,    64,   509,
     509,   285,   509,   509,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    88,    89,    90,    91,    92,    93,    94,    95,
      96,    97,    98,    99,   100,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   437,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   466,   467,   150,   151,   152,   153,   154,   155,
     156,   157,   158,   159,   160,   161,   162,    -1,    -1,    -1,
     166,   167,    -1,    -1,    -1,    -1,    -1,     1,    -1,    -1,
       4,    -1,    -1,    -1,    -1,    -1,    10,    -1,    -1,    -1,
      -1,   187,   188,   189,   190,   191,   192,   193,    22,   195,
     196,   197,   198,   199,   200,   201,   202,   203,   204,   205,
     206,   207,   208,   209,    -1,    -1,    -1,    -1,    -1,    -1,
     216,    -1,    -1,    -1,    -1,   221,   222,   223,   224,   225,
     226,    -1,   228,    -1,   230,    -1,    -1,    -1,   234,   235,
      64,    -1,    -1,    -1,   240,   241,   242,   243,   244,   245,
      -1,    -1,    -1,    -1,    -1,    -1,   252,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    88,    89,    90,    91,    92,    93,
      94,    95,    96,    97,    98,    99,   100,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   150,   151,   152,   153,
     154,   155,   156,   157,   158,   159,   160,   161,   162,    -1,
      -1,    -1,   166,   167,    -1,    -1,    -1,    -1,    -1,     1,
      -1,    -1,     4,    -1,    -1,    -1,    -1,    -1,    10,    -1,
      -1,    -1,    -1,   187,   188,   189,   190,   191,   192,   193,
      22,   195,   196,   197,   198,   199,   200,   201,   202,   203,
     204,   205,   206,   207,   208,   209,    -1,    -1,    -1,    -1,
      -1,    -1,   216,    -1,    -1,    -1,    -1,   221,   222,   223,
     224,   225,   226,    -1,   228,    -1,   230,    -1,    -1,    -1,
     234,    -1,    64,    -1,    -1,    -1,   240,   241,   242,   243,
     244,   245,    -1,    -1,    -1,    -1,    -1,    -1,   252,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    88,    89,    90,    91,
      92,    93,    94,    95,    96,    97,    98,    99,   100,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   150,   151,
     152,   153,   154,   155,   156,   157,   158,   159,   160,   161,
     162,    -1,    -1,    -1,   166,   167,    -1,    -1,    -1,    -1,
      -1,     1,    -1,    -1,     4,    -1,    -1,    -1,    -1,    -1,
      10,    -1,    -1,    -1,    -1,   187,   188,   189,   190,   191,
     192,   193,    22,   195,   196,    -1,    -1,   199,   200,   201,
     202,   203,   204,   205,   206,   207,   208,   209,    -1,    -1,
      -1,    -1,    -1,    -1,   216,    -1,    -1,    -1,    -1,   221,
     222,   223,   224,   225,   226,    -1,   228,    -1,   230,    -1,
      -1,    -1,   234,   235,    64,    -1,    -1,    -1,   240,   241,
     242,   243,   244,   245,    -1,    -1,    -1,    -1,    -1,    -1,
     252,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    88,    89,
      90,    91,    92,    93,    94,    95,    96,    97,    98,    99,
     100,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     150,   151,   152,   153,   154,   155,   156,   157,   158,   159,
     160,   161,   162,    -1,    -1,    -1,   166,   167,    -1,    -1,
      -1,    -1,    -1,     1,    -1,    -1,     4,    -1,    -1,    -1,
      -1,    -1,    10,    -1,    -1,    -1,    -1,   187,   188,   189,
     190,   191,   192,   193,    22,   195,   196,    -1,    -1,   199,
     200,   201,   202,   203,   204,   205,   206,   207,   208,   209,
      -1,    -1,    -1,    -1,    -1,    -1,   216,    -1,    -1,    -1,
      -1,   221,   222,   223,   224,   225,   226,    -1,   228,    -1,
     230,    -1,    -1,    -1,   234,   235,    64,    -1,    -1,    -1,
     240,   241,   242,   243,   244,   245,    -1,    -1,    -1,    -1,
      -1,    -1,   252,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      88,    89,    90,    91,    92,    93,    94,    95,    96,    97,
      98,    99,   100,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   150,   151,   152,   153,   154,   155,   156,   157,
     158,   159,   160,   161,   162,    -1,    -1,    -1,   166,   167,
      -1,    -1,    -1,    -1,    -1,     1,    -1,    -1,     4,    -1,
      -1,    -1,    -1,    -1,    10,    -1,    -1,    -1,    -1,   187,
     188,   189,   190,   191,   192,   193,    22,   195,   196,    -1,
      -1,   199,   200,   201,   202,   203,   204,   205,   206,   207,
     208,   209,    -1,    -1,    -1,    -1,    -1,    -1,   216,    -1,
      -1,    -1,    -1,   221,   222,   223,   224,   225,   226,    -1,
     228,    -1,   230,    -1,    -1,    -1,   234,    -1,    64,    -1,
      -1,    -1,   240,   241,   242,   243,   244,   245,    -1,    -1,
      -1,    -1,    -1,    -1,   252,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    88,    89,    90,    91,    92,    93,    94,    95,
      96,    97,    98,    99,   100,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,     1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   150,   151,   152,   153,   154,   155,
     156,   157,   158,   159,   160,   161,   162,    -1,    -1,    -1,
     166,   167,    -1,    -1,    -1,    -1,    -1,     1,    -1,    -1,
       4,    -1,    -1,    -1,    -1,    -1,    10,    -1,    -1,    -1,
      -1,   187,   188,   189,   190,   191,   192,   193,    22,   195,
     196,    -1,    -1,   199,   200,   201,   202,   203,   204,   205,
     206,   207,   208,   209,    -1,    -1,    -1,    -1,    -1,    -1,
     216,    -1,    -1,    -1,    -1,   221,   222,   223,   224,   225,
     226,    -1,   228,    -1,   230,    -1,    -1,    -1,   234,    -1,
      64,    -1,    -1,    -1,   240,   241,   242,   243,   244,   245,
      -1,    -1,    -1,    -1,    -1,    -1,   252,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    88,    89,    90,    91,    92,    93,
      94,    95,    96,    97,    98,    99,   100,   155,    -1,   157,
     158,   159,   160,   161,   162,    -1,    -1,    -1,   166,   167,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   150,   151,   152,   153,
     154,   155,   156,   157,   158,   159,   160,   161,   162,    -1,
      -1,    -1,   166,   167,    -1,    -1,    -1,    -1,   226,    -1,
     228,     4,   230,    -1,    -1,    -1,    -1,    10,    -1,    -1,
      -1,    -1,   240,   241,   242,   243,   244,   245,    -1,    22,
      -1,    -1,    -1,    -1,   252,   199,   200,   201,   202,   203,
     204,   205,   206,   207,   208,   209,    -1,    -1,    -1,    -1,
      -1,    -1,   216,    -1,    -1,    -1,    -1,   221,   222,   223,
     224,   225,   226,    -1,   228,    -1,   230,    -1,    -1,    -1,
      -1,    64,    -1,    -1,    -1,    -1,   240,   241,   242,   243,
     244,   245,    -1,    -1,    -1,    -1,    -1,    -1,   252,    -1,
      -1,    -1,    -1,    -1,    -1,    88,    89,    90,    91,    92,
      93,    94,    95,    96,    97,    98,    99,   100,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,     4,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   150,   151,   152,
     153,   154,   155,   156,   157,   158,   159,   160,   161,   162,
      -1,    -1,    -1,   166,   167,    -1,     4,    -1,    -1,    -1,
      -1,    -1,    10,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    22,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   199,   200,   201,   202,
     203,   204,   205,   206,   207,   155,   209,   157,   158,   159,
     160,   161,   162,    -1,    -1,    -1,   166,   167,   221,   222,
      -1,   224,    -1,   226,    -1,   228,    64,   230,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   241,   242,
     243,   244,   245,    -1,    -1,    -1,    -1,    -1,    -1,   252,
      88,    89,    90,    91,    92,    93,    94,    95,    96,    97,
      98,    99,   100,   155,    -1,   157,   158,   159,   160,   161,
     162,    -1,    -1,    -1,   166,   167,   226,    -1,   228,    -1,
     230,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     240,   241,   242,   243,   244,   245,    -1,    -1,    -1,    -1,
      -1,    -1,   252,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   150,   151,   152,   153,   154,   155,   156,   157,
     158,   159,   160,   161,   162,    -1,    -1,    -1,   166,   167,
      -1,    -1,    -1,     0,   226,    -1,   228,     4,   230,   231,
      -1,    -1,    -1,    10,    -1,    -1,    -1,    -1,    -1,   241,
     242,   243,   244,   245,    -1,    22,    -1,    -1,    -1,    -1,
     252,   199,   200,   201,   202,   203,   204,   205,   206,   207,
     155,   209,   157,   158,   159,   160,   161,   162,    -1,    -1,
      -1,   166,   167,   221,   222,    -1,   224,    -1,   226,    -1,
     228,    -1,   230,    -1,    -1,    -1,    -1,    64,    -1,    -1,
      -1,    -1,    -1,   241,   242,   243,   244,   245,    -1,    -1,
      -1,    -1,    -1,    -1,   252,    -1,    -1,    -1,    -1,    -1,
      -1,    88,    89,    90,    91,    92,    93,    94,    95,    96,
      97,    98,    99,   100,     4,    -1,    -1,    -1,    -1,    -1,
      10,   226,    -1,   228,    -1,   230,   231,    -1,    -1,    -1,
      -1,    -1,    22,    -1,    -1,    -1,   241,   242,   243,   244,
     245,    -1,    -1,    -1,    -1,    -1,    -1,   252,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   150,   151,   152,   153,   154,    -1,   156,
      -1,    -1,    -1,    -1,    64,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    88,    89,
      90,    91,    92,    93,    94,    95,    96,    97,    98,    99,
     100,    -1,   199,   200,   201,   202,   203,   204,   205,   206,
     207,   208,   209,    -1,    -1,    -1,    -1,    -1,    -1,   216,
      -1,    -1,    -1,    -1,   221,   222,   223,   224,   225,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   240,    -1,    -1,    -1,    -1,    -1,    -1,
     150,   151,   152,   153,   154,    -1,   156,     4,    -1,    -1,
      -1,    -1,    -1,    10,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    22,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   199,
     200,   201,   202,   203,   204,   205,   206,   207,   208,   209,
      -1,    -1,    -1,    -1,    -1,    -1,   216,    64,    -1,    -1,
      -1,   221,   222,   223,   224,   225,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     240,    88,    89,    90,    91,    92,    93,    94,    95,    96,
      97,    98,    99,   100,     4,    -1,    -1,    -1,    -1,    -1,
      10,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    22,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   150,   151,   152,   153,   154,    -1,   156,
      -1,    -1,    -1,    -1,    64,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    88,    89,
      90,    91,    92,    93,    94,    95,    96,    97,    98,    99,
     100,    -1,   199,   200,   201,   202,   203,   204,   205,   206,
     207,    -1,   209,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   221,   222,    -1,   224,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   235,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     150,   151,   152,   153,   154,     4,   156,    -1,    -1,    -1,
      -1,    10,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    22,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,     4,    -1,    -1,    -1,    -1,    -1,    10,   199,
     200,   201,   202,   203,   204,   205,   206,   207,    -1,   209,
      22,    -1,    -1,    -1,    -1,    64,    -1,    -1,    -1,    -1,
      -1,   221,   222,    -1,   224,   155,    -1,   157,   158,   159,
     160,   161,   162,    -1,    -1,   235,   166,   167,    -1,    88,
      89,    90,    91,    92,    93,    94,    95,    96,    97,    98,
      99,   100,    64,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    88,    89,    90,    91,
      92,    93,    94,    95,    96,    97,    98,    99,   100,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   226,    -1,   228,    -1,
     230,   150,   151,   152,   153,   154,    -1,   156,    -1,    -1,
      -1,   241,   242,   243,   244,   245,    -1,    -1,    -1,    -1,
      -1,    -1,   252,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   150,   151,
     152,   153,   154,     4,   156,    -1,    -1,    -1,    -1,    10,
     199,   200,   201,   202,   203,   204,   205,   206,   207,    -1,
     209,    22,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   221,   222,    -1,   224,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   235,   199,   200,   201,
     202,   203,   204,   205,   206,   207,    -1,   209,    -1,    -1,
      -1,    -1,    -1,    64,    -1,    -1,    -1,    -1,    -1,   221,
     222,    -1,   224,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   235,    -1,    -1,    -1,    88,    89,    90,
      91,    92,    93,    94,    95,    96,    97,    98,    99,   100,
       4,    -1,    -1,    -1,    -1,    -1,    10,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    22,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   150,
     151,   152,   153,   154,    -1,   156,    -1,    -1,    -1,    -1,
      64,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    88,    89,    90,    91,    92,    93,
      94,    95,    96,    97,    98,    99,   100,    -1,   199,   200,
     201,   202,   203,   204,   205,   206,   207,   208,   209,    -1,
      -1,    -1,    -1,    -1,    -1,   216,    -1,    -1,    -1,    -1,
     221,   222,    -1,   224,   225,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   150,   151,   152,   153,
     154,     4,   156,    -1,    -1,    -1,    -1,    10,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,     4,    -1,    -1,    22,
      -1,    -1,    10,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    22,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   199,   200,   201,   202,   203,
     204,   205,   206,   207,    -1,   209,   210,   211,    -1,    -1,
      -1,    64,    -1,    -1,    -1,    -1,    -1,   221,   222,    -1,
     224,    -1,    -1,    -1,    -1,    -1,    64,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    88,    89,    90,    91,    92,
      93,    94,    95,    96,    97,    98,    99,   100,    -1,    -1,
      88,    89,    90,    91,    92,    93,    94,    95,    96,    97,
      98,    99,   100,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   150,   151,   152,
     153,   154,    -1,   156,     4,    -1,    -1,    -1,    -1,    -1,
      10,    -1,   150,   151,   152,   153,   154,    -1,   156,    -1,
      -1,    -1,    22,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     155,    -1,   157,   158,   159,   160,   161,   162,    -1,    -1,
      -1,   166,   167,    -1,    -1,    -1,   199,   200,   201,   202,
     203,   204,   205,   206,   207,    -1,   209,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    64,    -1,    -1,    -1,   221,   222,
      -1,   224,   210,   211,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   221,   222,    -1,   224,    -1,    88,    89,
      90,    91,    92,    93,    94,    95,    96,    97,    98,    99,
     100,   226,    -1,   228,    -1,   230,   231,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   241,   242,   243,   244,
     245,    -1,    -1,    -1,    -1,    -1,   155,   252,   157,   158,
     159,   160,   161,   162,    -1,    -1,    -1,   166,   167,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     150,   151,   152,   153,   154,    -1,   156,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   226,    -1,   228,
      -1,   230,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   241,   242,   243,   244,   245,    -1,    -1,    -1,
      -1,   221,   222,   252,   224
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const unsigned short yystos[] =
{
       0,     4,    10,    22,    64,    88,    89,    90,    91,    92,
      93,    94,    95,    96,    97,    98,    99,   100,   150,   151,
     152,   153,   154,   156,   199,   200,   201,   202,   203,   204,
     205,   206,   207,   208,   209,   216,   221,   222,   223,   224,
     225,   240,   285,   286,   289,   290,   291,   292,   293,   298,
     304,   308,   311,   317,   318,   319,   320,   321,   322,   361,
     362,   363,   208,   311,   313,   230,   305,   155,   234,   311,
     203,   285,   286,   317,   322,   240,   313,   234,   344,   231,
     237,   210,   211,   294,   295,   296,   297,   298,   319,   319,
     237,   240,   317,   155,   230,   245,   299,   300,   301,   298,
     155,   234,   311,   240,     0,   362,   313,   317,   230,   290,
     308,   234,   155,   287,   288,   155,   234,   301,   240,   235,
     345,   295,   294,   297,   294,   296,   297,   232,   301,   301,
     301,   300,   298,   299,   155,   230,   232,   303,   313,   234,
     324,   155,   234,   317,   301,   312,   287,   239,   186,   235,
     237,   234,   287,   230,     1,   155,   157,   158,   159,   160,
     161,   162,   166,   167,   187,   188,   189,   190,   191,   192,
     193,   195,   196,   226,   228,   230,   234,   240,   241,   242,
     243,   244,   245,   252,   255,   256,   257,   258,   260,   261,
     262,   263,   264,   265,   267,   268,   269,   270,   271,   272,
     273,   274,   275,   276,   277,   278,   279,   280,   281,   283,
     289,   338,   339,   340,   341,   346,   347,   348,   354,   360,
     294,   297,   155,   267,   280,   284,   303,   303,   313,   230,
     231,   299,   284,   302,   232,   313,   239,   323,   317,   327,
     328,   234,   326,   301,   230,   212,   213,   214,   215,   217,
     218,   219,   314,   316,   186,   235,   284,   288,   287,   186,
     235,   235,   240,   230,   238,   230,   267,   267,   240,   240,
     240,   283,   155,   355,   356,   339,   230,   230,   262,   267,
     230,   262,   283,   317,   235,   342,   162,   166,   167,   185,
     232,   236,   231,   237,   234,   266,     4,   231,   281,   283,
     175,   176,   177,   178,   179,   180,   181,   182,   183,   184,
     239,   282,   265,   245,   246,   247,   242,   244,   164,   165,
     168,   169,   248,   249,   170,   171,   252,   251,   250,   172,
     174,   173,   253,   237,   240,   235,   339,   233,   313,   239,
     233,   302,   239,   309,   327,   301,   329,   330,   235,   328,
     325,   327,   230,   230,   230,   230,   230,   230,   231,   237,
     186,   235,   240,   240,   230,   230,   357,   191,   283,   259,
     283,   267,   231,   231,   299,   346,   163,   259,   163,   281,
     265,   231,   186,   235,   281,   265,   265,   265,   269,   269,
     270,   270,   271,   271,   271,   271,   272,   272,   273,   274,
     275,   276,   277,   278,   283,   281,   239,   306,   233,   310,
     234,   281,   331,   332,   235,   303,   313,   237,   240,   313,
     327,   235,   284,   155,   152,   284,   284,   231,   315,   283,
       1,   338,   347,     1,   283,   358,   359,   230,   231,   231,
     231,   231,   235,   233,   238,   307,   332,   332,   232,   236,
     333,   334,   335,   337,   313,   313,   330,   235,   231,   231,
     231,   237,   237,   316,   231,   240,   240,   240,   340,   343,
     344,   283,   339,   353,   234,   351,   281,   332,   284,   163,
     186,   235,   237,   332,   239,   336,   284,   284,   343,   231,
     283,   231,   283,   231,   194,   235,   352,   233,   334,   337,
     237,   237,   231,   231,   240,   339,   197,   198,   339,   349,
     350,   332,   284,   284,   284,   238,   235,   350,   231,   231,
     238
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
#line 868 "gc_cl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 225:
#line 870 "gc_cl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 226:
#line 875 "gc_cl.y"
    { clParseStructDeclBegin(Compiler); ;}
    break;

  case 227:
#line 877 "gc_cl.y"
    { yyval.dataType = clParseStructDeclEnd(Compiler, &yyvsp[-6].token, &yyvsp[-5].token, yyvsp[0].attr, yyvsp[-2].status); ;}
    break;

  case 228:
#line 879 "gc_cl.y"
    { clParseStructDeclBegin(Compiler); ;}
    break;

  case 229:
#line 881 "gc_cl.y"
    { yyval.dataType = clParseStructDeclEnd(Compiler, &yyvsp[-5].token, gcvNULL, yyvsp[0].attr, yyvsp[-2].status); ;}
    break;

  case 230:
#line 883 "gc_cl.y"
    { clParseStructDeclBegin(Compiler); ;}
    break;

  case 231:
#line 885 "gc_cl.y"
    { yyval.dataType = clParseStructDeclEnd(Compiler, &yyvsp[-6].token, &yyvsp[-4].token, yyvsp[-5].attr, yyvsp[-1].status); ;}
    break;

  case 232:
#line 887 "gc_cl.y"
    { clParseStructDeclBegin(Compiler); ;}
    break;

  case 233:
#line 889 "gc_cl.y"
    { yyval.dataType = clParseStructDeclEnd(Compiler, &yyvsp[-5].token, gcvNULL, yyvsp[-4].attr, yyvsp[-1].status); ;}
    break;

  case 234:
#line 894 "gc_cl.y"
    {
		   yyval.status = yyvsp[0].status;
		;}
    break;

  case 235:
#line 898 "gc_cl.y"
    {
		   if(gcmIS_ERROR(yyvsp[-1].status)) {
                       yyval.status = yyvsp[-1].status;
		   }
		   else {
                       yyval.status = yyvsp[0].status;

		   }
		;}
    break;

  case 236:
#line 911 "gc_cl.y"
    {
                   yyval.status = clParseTypeSpecifiedFieldDeclList(Compiler, &yyvsp[-2].decl, yyvsp[-1].fieldDeclList);
		;}
    break;

  case 237:
#line 918 "gc_cl.y"
    { yyval.fieldDeclList = clParseFieldDeclList(Compiler, yyvsp[0].fieldDecl); ;}
    break;

  case 238:
#line 920 "gc_cl.y"
    { yyval.fieldDeclList = clParseFieldDeclList2(Compiler, yyvsp[-2].fieldDeclList, yyvsp[0].fieldDecl); ;}
    break;

  case 239:
#line 925 "gc_cl.y"
    { yyval.fieldDecl = clParseFieldDecl(Compiler, &yyvsp[-1].token, gcvNULL, yyvsp[0].attr); ;}
    break;

  case 240:
#line 927 "gc_cl.y"
    { yyval.fieldDecl = clParseFieldDecl(Compiler, &yyvsp[-2].token, yyvsp[-1].expr, yyvsp[0].attr); ;}
    break;

  case 241:
#line 932 "gc_cl.y"
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

  case 242:
#line 954 "gc_cl.y"
    { yyval.expr = yyvsp[0].expr; ;}
    break;

  case 243:
#line 956 "gc_cl.y"
    { yyval.expr = yyvsp[-1].expr; ;}
    break;

  case 244:
#line 958 "gc_cl.y"
    { yyval.expr = yyvsp[-1].expr; ;}
    break;

  case 245:
#line 963 "gc_cl.y"
    { yyval.expr = clParseInitializerList(Compiler, yyvsp[-2].expr, yyvsp[-1].declOrDeclList, yyvsp[0].expr); ;}
    break;

  case 246:
#line 965 "gc_cl.y"
    { yyval.expr = clParseInitializerList(Compiler, yyvsp[-3].expr, yyvsp[-1].declOrDeclList, yyvsp[0].expr); ;}
    break;

  case 247:
#line 969 "gc_cl.y"
    { yyval.declOrDeclList = gcvNULL; ;}
    break;

  case 248:
#line 971 "gc_cl.y"
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

  case 249:
#line 984 "gc_cl.y"
    { yyval.declOrDeclList = yyvsp[0].declOrDeclList; ;}
    break;

  case 250:
#line 986 "gc_cl.y"
    {
		   yyval.token.type = T_EOF;
		;}
    break;

  case 251:
#line 990 "gc_cl.y"
    { yyval.declOrDeclList = yyvsp[0].declOrDeclList; ;}
    break;

  case 252:
#line 995 "gc_cl.y"
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

  case 253:
#line 1008 "gc_cl.y"
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

  case 254:
#line 1024 "gc_cl.y"
    { yyval.statement = yyvsp[0].statement; ;}
    break;

  case 255:
#line 1029 "gc_cl.y"
    { yyval.statement = clParseCompoundStatementAsStatement(Compiler, yyvsp[0].statements); ;}
    break;

  case 256:
#line 1031 "gc_cl.y"
    { yyval.statement = yyvsp[0].statement; ;}
    break;

  case 257:
#line 1036 "gc_cl.y"
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
    { yyval.statement = clParseStatementLabel(Compiler, &yyvsp[-1].token); ;}
    break;

  case 263:
#line 1048 "gc_cl.y"
    { yyclearin;
		  yyerrok;
		  yyval.statement = gcvNULL; ;}
    break;

  case 264:
#line 1052 "gc_cl.y"
    { yyclearin;
		  yyerrok;
		  yyval.statement = gcvNULL; ;}
    break;

  case 265:
#line 1059 "gc_cl.y"
    { yyval.statements = gcvNULL; ;}
    break;

  case 266:
#line 1061 "gc_cl.y"
    { clParseCompoundStatementBegin(Compiler); ;}
    break;

  case 267:
#line 1063 "gc_cl.y"
    { yyval.statements = clParseCompoundStatementEnd(Compiler, &yyvsp[-3].token, yyvsp[-1].statements); ;}
    break;

  case 268:
#line 1068 "gc_cl.y"
    { yyval.statement = clParseCompoundStatementNoNewScopeAsStatementNoNewScope(Compiler, yyvsp[0].statements); ;}
    break;

  case 269:
#line 1070 "gc_cl.y"
    { yyval.statement = yyvsp[0].statement; ;}
    break;

  case 270:
#line 1075 "gc_cl.y"
    { yyval.statements = gcvNULL; ;}
    break;

  case 271:
#line 1077 "gc_cl.y"
    { clParseCompoundStatementNoNewScopeBegin(Compiler); ;}
    break;

  case 272:
#line 1079 "gc_cl.y"
    { yyval.statements = clParseCompoundStatementNoNewScopeEnd(Compiler, &yyvsp[-3].token, yyvsp[-1].statements); ;}
    break;

  case 273:
#line 1084 "gc_cl.y"
    { yyval.statements = clParseStatementList(Compiler, yyvsp[0].statement); ;}
    break;

  case 274:
#line 1086 "gc_cl.y"
    { yyval.statements = clParseStatementList2(Compiler, yyvsp[-1].statements, yyvsp[0].statement); ;}
    break;

  case 275:
#line 1091 "gc_cl.y"
    { yyval.statement = gcvNULL; ;}
    break;

  case 276:
#line 1093 "gc_cl.y"
    { yyval.statement = clParseExprAsStatement(Compiler, yyvsp[-1].expr); ;}
    break;

  case 277:
#line 1098 "gc_cl.y"
    { yyval.statement = clParseIfStatement(Compiler, &yyvsp[-4].token, yyvsp[-2].expr, yyvsp[0].ifStatementPair); ;}
    break;

  case 278:
#line 1100 "gc_cl.y"
    { yyval.statement = clParseSwitchStatement(Compiler, &yyvsp[-4].token, yyvsp[-2].expr, yyvsp[0].statement); ;}
    break;

  case 279:
#line 1105 "gc_cl.y"
    { yyval.statements = clParseStatementList(Compiler, yyvsp[0].statement); ;}
    break;

  case 280:
#line 1107 "gc_cl.y"
    { yyval.statements = clParseStatementList2(Compiler, yyvsp[-1].statements, yyvsp[0].statement); ;}
    break;

  case 281:
#line 1112 "gc_cl.y"
    { yyval.statement = yyvsp[0].statement; ;}
    break;

  case 282:
#line 1114 "gc_cl.y"
    { yyval.statement = clParseCaseStatement(Compiler, &yyvsp[-2].token, yyvsp[-1].expr); ;}
    break;

  case 283:
#line 1116 "gc_cl.y"
    { yyval.statement = clParseDefaultStatement(Compiler, &yyvsp[-1].token); ;}
    break;

  case 284:
#line 1121 "gc_cl.y"
    { yyval.statement = gcvNULL; ;}
    break;

  case 285:
#line 1123 "gc_cl.y"
    { clParseSwitchBodyBegin(Compiler); ;}
    break;

  case 286:
#line 1125 "gc_cl.y"
    { yyval.statement = clParseSwitchBodyEnd(Compiler, &yyvsp[-3].token, yyvsp[-1].statements); ;}
    break;

  case 287:
#line 1130 "gc_cl.y"
    { yyval.ifStatementPair = clParseIfSubStatements(Compiler, yyvsp[-2].statement, yyvsp[0].statement); ;}
    break;

  case 288:
#line 1132 "gc_cl.y"
    { yyval.ifStatementPair = clParseIfSubStatements(Compiler, yyvsp[0].statement, gcvNULL); ;}
    break;

  case 289:
#line 1139 "gc_cl.y"
    { clParseWhileStatementBegin(Compiler); ;}
    break;

  case 290:
#line 1141 "gc_cl.y"
    { yyval.statement = clParseWhileStatementEnd(Compiler, &yyvsp[-5].token, yyvsp[-2].expr, yyvsp[0].statement); ;}
    break;

  case 291:
#line 1143 "gc_cl.y"
    { yyval.statement = clParseDoWhileStatement(Compiler, &yyvsp[-6].token, yyvsp[-5].statement, yyvsp[-2].expr); ;}
    break;

  case 292:
#line 1145 "gc_cl.y"
    { clParseForStatementBegin(Compiler); ;}
    break;

  case 293:
#line 1147 "gc_cl.y"
    { yyval.statement = clParseForStatementEnd(Compiler, &yyvsp[-4].token, yyvsp[-2].statement, yyvsp[-1].forExprPair, yyvsp[0].statement); ;}
    break;

  case 294:
#line 1152 "gc_cl.y"
    { yyval.statement = yyvsp[0].statement; ;}
    break;

  case 295:
#line 1154 "gc_cl.y"
    { yyval.statement = yyvsp[0].statement; ;}
    break;

  case 296:
#line 1156 "gc_cl.y"
    { yyclearin;
		  yyerrok;
		  yyval.statement = gcvNULL; ;}
    break;

  case 297:
#line 1163 "gc_cl.y"
    { yyval.expr = yyvsp[0].expr; ;}
    break;

  case 298:
#line 1165 "gc_cl.y"
    { yyval.expr = gcvNULL; ;}
    break;

  case 299:
#line 1170 "gc_cl.y"
    { yyval.forExprPair = clParseForControl(Compiler, yyvsp[-2].expr, gcvNULL); ;}
    break;

  case 300:
#line 1172 "gc_cl.y"
    { yyval.forExprPair = clParseForControl(Compiler, yyvsp[-3].expr, yyvsp[-1].expr); ;}
    break;

  case 301:
#line 1174 "gc_cl.y"
    {
		  clsForExprPair nullPair = {gcvNULL, gcvNULL};
		  yyclearin;
		  yyerrok;
		  yyval.forExprPair = nullPair; ;}
    break;

  case 302:
#line 1180 "gc_cl.y"
    {
		  clsForExprPair nullPair = {gcvNULL, gcvNULL};
		  yyclearin;
		  yyerrok;
		  yyval.forExprPair = nullPair; ;}
    break;

  case 303:
#line 1189 "gc_cl.y"
    { yyval.statement = clParseJumpStatement(Compiler, clvCONTINUE, &yyvsp[-1].token, gcvNULL); ;}
    break;

  case 304:
#line 1191 "gc_cl.y"
    { yyval.statement = clParseJumpStatement(Compiler, clvBREAK, &yyvsp[-1].token, gcvNULL); ;}
    break;

  case 305:
#line 1193 "gc_cl.y"
    { yyval.statement = clParseJumpStatement(Compiler, clvRETURN, &yyvsp[-1].token, gcvNULL); ;}
    break;

  case 306:
#line 1195 "gc_cl.y"
    { yyval.statement = clParseJumpStatement(Compiler, clvRETURN, &yyvsp[-2].token, yyvsp[-1].expr); ;}
    break;

  case 307:
#line 1197 "gc_cl.y"
    { yyval.statement = clParseGotoStatement(Compiler, &yyvsp[-2].token, &yyvsp[-1].token); ;}
    break;

  case 311:
#line 1210 "gc_cl.y"
    { clParseExternalDecl(Compiler, yyvsp[0].statement); ;}
    break;

  case 313:
#line 1216 "gc_cl.y"
    { clParseFuncDef(Compiler, yyvsp[-1].funcName, yyvsp[0].statements); ;}
    break;


    }

/* Line 993 of yacc.c.  */
#line 4295 "gc_cl_parser.c"

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


#line 1219 "gc_cl.y"



