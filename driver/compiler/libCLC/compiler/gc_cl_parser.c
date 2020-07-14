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
     T_VIV_GENERIC_GL_SAMPLER = 350,
     T_IMAGE1D_T = 351,
     T_IMAGE1D_ARRAY_T = 352,
     T_IMAGE1D_BUFFER_T = 353,
     T_IMAGE2D_ARRAY_T = 354,
     T_IMAGE2D_T = 355,
     T_IMAGE3D_T = 356,
     T_IMAGE2D_PTR_T = 357,
     T_IMAGE2D_DYNAMIC_ARRAY_T = 358,
     T_VIV_GENERIC_IMAGE_T = 359,
     T_VIV_GENERIC_GL_IMAGE = 360,
     T_SIZE_T = 361,
     T_EVENT_T = 362,
     T_PTRDIFF_T = 363,
     T_INTPTR_T = 364,
     T_UINTPTR_T = 365,
     T_GENTYPE = 366,
     T_F_GENTYPE = 367,
     T_IU_GENTYPE = 368,
     T_I_GENTYPE = 369,
     T_U_GENTYPE = 370,
     T_SIU_GENTYPE = 371,
     T_BOOL_PACKED = 372,
     T_BOOL2_PACKED = 373,
     T_BOOL3_PACKED = 374,
     T_BOOL4_PACKED = 375,
     T_BOOL8_PACKED = 376,
     T_BOOL16_PACKED = 377,
     T_BOOL32_PACKED = 378,
     T_CHAR_PACKED = 379,
     T_CHAR2_PACKED = 380,
     T_CHAR3_PACKED = 381,
     T_CHAR4_PACKED = 382,
     T_CHAR8_PACKED = 383,
     T_CHAR16_PACKED = 384,
     T_CHAR32_PACKED = 385,
     T_UCHAR_PACKED = 386,
     T_UCHAR2_PACKED = 387,
     T_UCHAR3_PACKED = 388,
     T_UCHAR4_PACKED = 389,
     T_UCHAR8_PACKED = 390,
     T_UCHAR16_PACKED = 391,
     T_UCHAR32_PACKED = 392,
     T_SHORT_PACKED = 393,
     T_SHORT2_PACKED = 394,
     T_SHORT3_PACKED = 395,
     T_SHORT4_PACKED = 396,
     T_SHORT8_PACKED = 397,
     T_SHORT16_PACKED = 398,
     T_SHORT32_PACKED = 399,
     T_USHORT_PACKED = 400,
     T_USHORT2_PACKED = 401,
     T_USHORT3_PACKED = 402,
     T_USHORT4_PACKED = 403,
     T_USHORT8_PACKED = 404,
     T_USHORT16_PACKED = 405,
     T_USHORT32_PACKED = 406,
     T_HALF_PACKED = 407,
     T_HALF2_PACKED = 408,
     T_HALF3_PACKED = 409,
     T_HALF4_PACKED = 410,
     T_HALF8_PACKED = 411,
     T_HALF16_PACKED = 412,
     T_HALF32_PACKED = 413,
     T_GENTYPE_PACKED = 414,
     T_F_GENTYPE_PACKED = 415,
     T_IU_GENTYPE_PACKED = 416,
     T_I_GENTYPE_PACKED = 417,
     T_U_GENTYPE_PACKED = 418,
     T_FLOATNXM = 419,
     T_DOUBLENXM = 420,
     T_BUILTIN_DATA_TYPE = 421,
     T_RESERVED_DATA_TYPE = 422,
     T_VIV_PACKED_DATA_TYPE = 423,
     T_IDENTIFIER = 424,
     T_TYPE_NAME = 425,
     T_FLOATCONSTANT = 426,
     T_UINTCONSTANT = 427,
     T_INTCONSTANT = 428,
     T_BOOLCONSTANT = 429,
     T_CHARCONSTANT = 430,
     T_STRING_LITERAL = 431,
     T_FIELD_SELECTION = 432,
     T_LSHIFT_OP = 433,
     T_RSHIFT_OP = 434,
     T_INC_OP = 435,
     T_DEC_OP = 436,
     T_LE_OP = 437,
     T_GE_OP = 438,
     T_EQ_OP = 439,
     T_NE_OP = 440,
     T_AND_OP = 441,
     T_OR_OP = 442,
     T_XOR_OP = 443,
     T_MUL_ASSIGN = 444,
     T_DIV_ASSIGN = 445,
     T_ADD_ASSIGN = 446,
     T_MOD_ASSIGN = 447,
     T_LEFT_ASSIGN = 448,
     T_RIGHT_ASSIGN = 449,
     T_AND_ASSIGN = 450,
     T_XOR_ASSIGN = 451,
     T_OR_ASSIGN = 452,
     T_SUB_ASSIGN = 453,
     T_STRUCT_UNION_PTR = 454,
     T_INITIALIZER_END = 455,
     T_BREAK = 456,
     T_CONTINUE = 457,
     T_RETURN = 458,
     T_GOTO = 459,
     T_WHILE = 460,
     T_FOR = 461,
     T_DO = 462,
     T_ELSE = 463,
     T_IF = 464,
     T_SWITCH = 465,
     T_CASE = 466,
     T_DEFAULT = 467,
     T_CONST = 468,
     T_RESTRICT = 469,
     T_VOLATILE = 470,
     T_STATIC = 471,
     T_EXTERN = 472,
     T_CONSTANT = 473,
     T_GLOBAL = 474,
     T_LOCAL = 475,
     T_PRIVATE = 476,
     T_KERNEL = 477,
     T_UNIFORM = 478,
     T_READ_ONLY = 479,
     T_WRITE_ONLY = 480,
     T_PACKED = 481,
     T_ALIGNED = 482,
     T_ENDIAN = 483,
     T_VEC_TYPE_HINT = 484,
     T_ATTRIBUTE__ = 485,
     T_REQD_WORK_GROUP_SIZE = 486,
     T_WORK_GROUP_SIZE_HINT = 487,
     T_KERNEL_SCALE_HINT = 488,
     T_ALWAYS_INLINE = 489,
     T_UNSIGNED = 490,
     T_STRUCT = 491,
     T_UNION = 492,
     T_TYPEDEF = 493,
     T_ENUM = 494,
     T_INLINE = 495,
     T_SIZEOF = 496,
     T_TYPE_CAST = 497,
     T_VEC_STEP = 498,
     T_TYPEOF = 499,
     T_ASM_OPND_BRACKET = 500,
     T_VERY_LAST_TERMINAL = 501
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
#define T_VIV_GENERIC_GL_SAMPLER 350
#define T_IMAGE1D_T 351
#define T_IMAGE1D_ARRAY_T 352
#define T_IMAGE1D_BUFFER_T 353
#define T_IMAGE2D_ARRAY_T 354
#define T_IMAGE2D_T 355
#define T_IMAGE3D_T 356
#define T_IMAGE2D_PTR_T 357
#define T_IMAGE2D_DYNAMIC_ARRAY_T 358
#define T_VIV_GENERIC_IMAGE_T 359
#define T_VIV_GENERIC_GL_IMAGE 360
#define T_SIZE_T 361
#define T_EVENT_T 362
#define T_PTRDIFF_T 363
#define T_INTPTR_T 364
#define T_UINTPTR_T 365
#define T_GENTYPE 366
#define T_F_GENTYPE 367
#define T_IU_GENTYPE 368
#define T_I_GENTYPE 369
#define T_U_GENTYPE 370
#define T_SIU_GENTYPE 371
#define T_BOOL_PACKED 372
#define T_BOOL2_PACKED 373
#define T_BOOL3_PACKED 374
#define T_BOOL4_PACKED 375
#define T_BOOL8_PACKED 376
#define T_BOOL16_PACKED 377
#define T_BOOL32_PACKED 378
#define T_CHAR_PACKED 379
#define T_CHAR2_PACKED 380
#define T_CHAR3_PACKED 381
#define T_CHAR4_PACKED 382
#define T_CHAR8_PACKED 383
#define T_CHAR16_PACKED 384
#define T_CHAR32_PACKED 385
#define T_UCHAR_PACKED 386
#define T_UCHAR2_PACKED 387
#define T_UCHAR3_PACKED 388
#define T_UCHAR4_PACKED 389
#define T_UCHAR8_PACKED 390
#define T_UCHAR16_PACKED 391
#define T_UCHAR32_PACKED 392
#define T_SHORT_PACKED 393
#define T_SHORT2_PACKED 394
#define T_SHORT3_PACKED 395
#define T_SHORT4_PACKED 396
#define T_SHORT8_PACKED 397
#define T_SHORT16_PACKED 398
#define T_SHORT32_PACKED 399
#define T_USHORT_PACKED 400
#define T_USHORT2_PACKED 401
#define T_USHORT3_PACKED 402
#define T_USHORT4_PACKED 403
#define T_USHORT8_PACKED 404
#define T_USHORT16_PACKED 405
#define T_USHORT32_PACKED 406
#define T_HALF_PACKED 407
#define T_HALF2_PACKED 408
#define T_HALF3_PACKED 409
#define T_HALF4_PACKED 410
#define T_HALF8_PACKED 411
#define T_HALF16_PACKED 412
#define T_HALF32_PACKED 413
#define T_GENTYPE_PACKED 414
#define T_F_GENTYPE_PACKED 415
#define T_IU_GENTYPE_PACKED 416
#define T_I_GENTYPE_PACKED 417
#define T_U_GENTYPE_PACKED 418
#define T_FLOATNXM 419
#define T_DOUBLENXM 420
#define T_BUILTIN_DATA_TYPE 421
#define T_RESERVED_DATA_TYPE 422
#define T_VIV_PACKED_DATA_TYPE 423
#define T_IDENTIFIER 424
#define T_TYPE_NAME 425
#define T_FLOATCONSTANT 426
#define T_UINTCONSTANT 427
#define T_INTCONSTANT 428
#define T_BOOLCONSTANT 429
#define T_CHARCONSTANT 430
#define T_STRING_LITERAL 431
#define T_FIELD_SELECTION 432
#define T_LSHIFT_OP 433
#define T_RSHIFT_OP 434
#define T_INC_OP 435
#define T_DEC_OP 436
#define T_LE_OP 437
#define T_GE_OP 438
#define T_EQ_OP 439
#define T_NE_OP 440
#define T_AND_OP 441
#define T_OR_OP 442
#define T_XOR_OP 443
#define T_MUL_ASSIGN 444
#define T_DIV_ASSIGN 445
#define T_ADD_ASSIGN 446
#define T_MOD_ASSIGN 447
#define T_LEFT_ASSIGN 448
#define T_RIGHT_ASSIGN 449
#define T_AND_ASSIGN 450
#define T_XOR_ASSIGN 451
#define T_OR_ASSIGN 452
#define T_SUB_ASSIGN 453
#define T_STRUCT_UNION_PTR 454
#define T_INITIALIZER_END 455
#define T_BREAK 456
#define T_CONTINUE 457
#define T_RETURN 458
#define T_GOTO 459
#define T_WHILE 460
#define T_FOR 461
#define T_DO 462
#define T_ELSE 463
#define T_IF 464
#define T_SWITCH 465
#define T_CASE 466
#define T_DEFAULT 467
#define T_CONST 468
#define T_RESTRICT 469
#define T_VOLATILE 470
#define T_STATIC 471
#define T_EXTERN 472
#define T_CONSTANT 473
#define T_GLOBAL 474
#define T_LOCAL 475
#define T_PRIVATE 476
#define T_KERNEL 477
#define T_UNIFORM 478
#define T_READ_ONLY 479
#define T_WRITE_ONLY 480
#define T_PACKED 481
#define T_ALIGNED 482
#define T_ENDIAN 483
#define T_VEC_TYPE_HINT 484
#define T_ATTRIBUTE__ 485
#define T_REQD_WORK_GROUP_SIZE 486
#define T_WORK_GROUP_SIZE_HINT 487
#define T_KERNEL_SCALE_HINT 488
#define T_ALWAYS_INLINE 489
#define T_UNSIGNED 490
#define T_STRUCT 491
#define T_UNION 492
#define T_TYPEDEF 493
#define T_ENUM 494
#define T_INLINE 495
#define T_SIZEOF 496
#define T_TYPE_CAST 497
#define T_VEC_STEP 498
#define T_TYPEOF 499
#define T_ASM_OPND_BRACKET 500
#define T_VERY_LAST_TERMINAL 501




/* Copy the first part of user declarations.  */
#line 1 "gc_cl.y"

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
#line 23 "gc_cl.y"
typedef union YYSTYPE {
    clsLexToken        token;
    slsSLINK_LIST *        typeQualifierList;
    clsDeclOrDeclList    *declOrDeclList;
    slsDLINK_LIST *        fieldDeclList;
    clsFieldDecl *        fieldDecl;
    clsDATA_TYPE *        dataType;
    clsDECL            decl;
    cloIR_EXPR        expr;
    clsNAME    *        funcName;
    clsNAME    *        paramName;
    clsATTRIBUTE *        attr;
    slsSLINK_LIST *        enumeratorList;
    clsNAME    *        enumeratorName;
    cloIR_SET        statements;
    cloIR_BASE        statement;
    clsIfStatementPair    ifStatementPair;
    clsForExprPair        forExprPair;
    cloIR_POLYNARY_EXPR    funcCall;
    gceSTATUS        status;
    clsASM_MODIFIER     asmModifier;
    clsASM_MODIFIERS    asmModifiers;
} YYSTYPE;
/* Line 191 of yacc.c.  */
#line 611 "gc_cl_parser.c"
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif



/* Copy the second part of user declarations.  */


/* Line 214 of yacc.c.  */
#line 623 "gc_cl_parser.c"

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
     ((N) * (sizeof (short) + sizeof (YYSTYPE))                \
      + YYSTACK_GAP_MAXIMUM)

/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
#ifndef YYCOPY
#if defined (__GNUC__) && 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)        \
      do                    \
    {                    \
      register YYSIZE_T yyi;        \
      for (yyi = 0; yyi < (Count); yyi++)    \
        (To)[yyi] = (From)[yyi];        \
    }                    \
      while (0)
#  endif
# endif

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack)                    \
    do                                    \
      {                                    \
    YYSIZE_T yynewbytes;                        \
    YYCOPY (&yyptr->Stack, Stack, yysize);                \
    Stack = &yyptr->Stack;                        \
    yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
    yyptr += yynewbytes / sizeof (*yyptr);                \
      }                                    \
    while (0)

#endif

#if defined (__STDC__) || defined (__cplusplus)
   typedef signed char yysigned_char;
#else
   typedef short yysigned_char;
#endif

/* YYFINAL -- State number of the termination state. */
#define YYFINAL  2
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   3616

/* YYNTOKENS -- Number of terminals. */
#define YYNTOKENS  271
/* YYNNTS -- Number of nonterminals. */
#define YYNNTS  115
/* YYNRULES -- Number of rules. */
#define YYNRULES  341
/* YYNRULES -- Number of states. */
#define YYNSTATES  582

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   501

#define YYTRANSLATE(YYX)                         \
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const unsigned short yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   258,     2,     2,     2,   264,   269,     2,
     247,   248,   262,   261,   254,   259,   253,   263,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,   255,   257,
     265,   256,   266,   270,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,   249,     2,   250,   268,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   251,   267,   252,   260,     2,     2,     2,
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
     235,   236,   237,   238,   239,   240,   241,   242,   243,   244,
     245,   246
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const unsigned short yyprhs[] =
{
       0,     0,     3,     5,     7,    10,    12,    14,    16,    18,
      20,    22,    24,    28,    30,    35,    37,    41,    45,    48,
      51,    53,    56,    60,    63,    66,    70,    72,    77,    79,
      83,    87,    91,    96,    99,   102,   104,   105,   109,   113,
     117,   119,   122,   125,   130,   133,   136,   139,   142,   144,
     146,   148,   150,   152,   154,   156,   160,   164,   168,   170,
     174,   178,   180,   184,   188,   190,   194,   198,   202,   206,
     208,   212,   216,   218,   222,   224,   228,   230,   234,   236,
     240,   242,   246,   248,   252,   254,   260,   262,   266,   268,
     270,   272,   274,   276,   278,   280,   282,   284,   286,   288,
     290,   294,   296,   299,   302,   309,   316,   322,   328,   334,
     340,   345,   350,   352,   356,   358,   362,   365,   369,   372,
     375,   378,   380,   382,   385,   389,   395,   401,   408,   412,
     417,   422,   428,   432,   437,   442,   448,   450,   453,   455,
     458,   461,   465,   468,   472,   474,   476,   478,   481,   484,
     488,   490,   493,   495,   498,   501,   505,   507,   510,   512,
     516,   517,   519,   523,   528,   530,   531,   535,   540,   546,
     547,   555,   556,   565,   567,   571,   576,   577,   584,   585,
     593,   594,   602,   604,   607,   608,   610,   611,   613,   614,
     619,   624,   626,   631,   640,   649,   658,   660,   665,   667,
     669,   674,   677,   683,   691,   693,   695,   698,   701,   705,
     710,   712,   714,   716,   718,   720,   722,   724,   726,   728,
     730,   732,   734,   736,   738,   740,   742,   744,   746,   748,
     750,   752,   754,   756,   758,   760,   762,   764,   766,   768,
     770,   772,   774,   776,   778,   780,   782,   784,   786,   788,
     790,   792,   794,   796,   797,   805,   806,   813,   814,   822,
     823,   830,   832,   835,   839,   841,   845,   846,   849,   853,
     855,   857,   861,   865,   868,   873,   874,   877,   879,   880,
     884,   888,   891,   893,   895,   897,   899,   901,   903,   905,
     907,   910,   913,   916,   919,   920,   925,   927,   929,   932,
     933,   938,   940,   943,   945,   948,   954,   960,   962,   965,
     967,   971,   974,   977,   978,   983,   987,   989,   990,   997,
    1005,  1006,  1012,  1015,  1018,  1022,  1024,  1025,  1029,  1034,
    1038,  1043,  1046,  1049,  1052,  1056,  1060,  1061,  1064,  1066,
    1068,  1070
};

/* YYRHS -- A `-1'-separated list of the rules' RHS. */
static const short yyrhs[] =
{
     383,     0,    -1,   169,    -1,   176,    -1,   273,   176,    -1,
     272,    -1,   172,    -1,   173,    -1,   171,    -1,   174,    -1,
     175,    -1,   273,    -1,   247,   303,   248,    -1,   274,    -1,
     275,   249,   276,   250,    -1,   277,    -1,   275,   253,   177,
      -1,   275,   199,   177,    -1,   275,   180,    -1,   275,   181,
      -1,   303,    -1,   278,   248,    -1,   283,     4,   248,    -1,
     283,   248,    -1,   283,   279,    -1,   278,   254,   279,    -1,
     301,    -1,   301,   245,   280,   266,    -1,   281,    -1,   280,
     254,   281,    -1,   169,   255,   169,    -1,   247,   339,   248,
      -1,   247,   339,   319,   248,    -1,   169,   247,    -1,   282,
     251,    -1,   287,    -1,    -1,   282,   286,   285,    -1,   284,
     303,   252,    -1,   284,   303,   200,    -1,   275,    -1,   180,
     287,    -1,   181,   287,    -1,   243,   247,   287,   248,    -1,
     243,   282,    -1,   241,   287,    -1,   241,   282,    -1,   288,
     285,    -1,   261,    -1,   259,    -1,   258,    -1,   260,    -1,
     269,    -1,   262,    -1,   285,    -1,   289,   262,   285,    -1,
     289,   263,   285,    -1,   289,   264,   285,    -1,   289,    -1,
     290,   261,   289,    -1,   290,   259,   289,    -1,   290,    -1,
     291,   178,   290,    -1,   291,   179,   290,    -1,   291,    -1,
     292,   265,   291,    -1,   292,   266,   291,    -1,   292,   182,
     291,    -1,   292,   183,   291,    -1,   292,    -1,   293,   184,
     292,    -1,   293,   185,   292,    -1,   293,    -1,   294,   269,
     293,    -1,   294,    -1,   295,   268,   294,    -1,   295,    -1,
     296,   267,   295,    -1,   296,    -1,   297,   186,   296,    -1,
     297,    -1,   298,   188,   297,    -1,   298,    -1,   299,   187,
     298,    -1,   299,    -1,   299,   270,   303,   255,   301,    -1,
     300,    -1,   287,   302,   301,    -1,   256,    -1,   189,    -1,
     190,    -1,   192,    -1,   191,    -1,   198,    -1,   193,    -1,
     194,    -1,   195,    -1,   196,    -1,   197,    -1,   301,    -1,
     303,   254,   301,    -1,   300,    -1,   343,   169,    -1,   239,
     169,    -1,   239,   333,   169,   251,   307,   252,    -1,   239,
     333,   169,   251,   307,   200,    -1,   239,   169,   251,   307,
     252,    -1,   239,   169,   251,   307,   200,    -1,   239,   333,
     251,   307,   252,    -1,   239,   333,   251,   307,   200,    -1,
     239,   251,   307,   252,    -1,   239,   251,   307,   200,    -1,
     308,    -1,   307,   254,   308,    -1,   169,    -1,   169,   256,
     304,    -1,   324,   257,    -1,   306,   334,   257,    -1,   305,
     257,    -1,   344,   257,    -1,   311,   248,    -1,   313,    -1,
     312,    -1,   313,   315,    -1,   312,   254,   315,    -1,   333,
     222,   339,   321,   247,    -1,   222,   334,   339,   321,   247,
      -1,   217,   222,   334,   339,   321,   247,    -1,   339,   321,
     247,    -1,   333,   339,   321,   247,    -1,   240,   339,   321,
     247,    -1,   216,   240,   339,   321,   247,    -1,   341,   321,
     334,    -1,   341,   321,   323,   334,    -1,   341,   318,   321,
     334,    -1,   341,   318,   321,   323,   334,    -1,   314,    -1,
     316,   314,    -1,   317,    -1,   316,   317,    -1,   318,   314,
      -1,   318,   316,   314,    -1,   318,   317,    -1,   318,   316,
     317,    -1,   224,    -1,   225,    -1,   341,    -1,   341,   323,
      -1,   341,   319,    -1,   341,   319,   323,    -1,   340,    -1,
     340,   318,    -1,   262,    -1,   262,   319,    -1,   262,   318,
      -1,   262,   318,   319,    -1,   169,    -1,   319,   169,    -1,
     320,    -1,   247,   320,   248,    -1,    -1,   304,    -1,   249,
     322,   250,    -1,   323,   249,   322,   250,    -1,   328,    -1,
      -1,   238,   325,   328,    -1,   324,   254,   321,   334,    -1,
     324,   254,   321,   323,   334,    -1,    -1,   324,   254,   321,
     334,   256,   326,   354,    -1,    -1,   324,   254,   321,   323,
     334,   256,   327,   354,    -1,   310,    -1,   339,   321,   334,
      -1,   339,   321,   323,   334,    -1,    -1,   339,   321,   334,
     256,   329,   354,    -1,    -1,   339,   321,   323,   334,   256,
     330,   354,    -1,    -1,   230,   247,   247,   332,   335,   248,
     248,    -1,   331,    -1,   333,   331,    -1,    -1,   333,    -1,
      -1,   337,    -1,    -1,   335,   254,   336,   337,    -1,   228,
     247,   169,   248,    -1,   226,    -1,   229,   247,   166,   248,
      -1,   231,   247,   304,   254,   304,   254,   304,   248,    -1,
     232,   247,   304,   254,   304,   254,   304,   248,    -1,   233,
     247,   304,   254,   304,   254,   304,   248,    -1,   227,    -1,
     227,   247,   304,   248,    -1,   234,    -1,   339,    -1,   339,
     249,   304,   250,    -1,   339,   319,    -1,   339,   319,   249,
     304,   250,    -1,   247,   339,   319,   248,   249,   304,   250,
      -1,   303,    -1,   341,    -1,   318,   341,    -1,   341,   318,
      -1,   318,   341,   318,    -1,   244,   247,   338,   248,    -1,
     213,    -1,   214,    -1,   215,    -1,   218,    -1,   219,    -1,
     220,    -1,   221,    -1,   216,    -1,   217,    -1,   223,    -1,
     342,    -1,   344,    -1,   305,    -1,   306,    -1,     4,    -1,
      24,    -1,    70,    -1,    10,    -1,   164,    -1,   165,    -1,
     166,    -1,   167,    -1,   168,    -1,    96,    -1,    97,    -1,
      98,    -1,    99,    -1,   100,    -1,   102,    -1,   103,    -1,
     101,    -1,   104,    -1,   105,    -1,    94,    -1,    95,    -1,
     108,    -1,   109,    -1,   110,    -1,   106,    -1,   107,    -1,
     170,    -1,   236,    -1,   237,    -1,    -1,   343,   169,   251,
     345,   349,   252,   334,    -1,    -1,   343,   251,   346,   349,
     252,   334,    -1,    -1,   343,   331,   169,   251,   347,   349,
     252,    -1,    -1,   343,   331,   251,   348,   349,   252,    -1,
     350,    -1,   349,   350,    -1,   339,   351,   257,    -1,   352,
      -1,   351,   254,   352,    -1,    -1,   321,   334,    -1,   321,
     323,   334,    -1,   251,    -1,   301,    -1,   353,   355,   252,
      -1,   353,   355,   200,    -1,   356,   354,    -1,   355,   254,
     356,   354,    -1,    -1,   357,   256,    -1,   359,    -1,    -1,
     357,   358,   359,    -1,   249,   304,   250,    -1,   253,   177,
      -1,   309,    -1,   363,    -1,   362,    -1,   360,    -1,   369,
      -1,   370,    -1,   376,    -1,   382,    -1,   169,   255,    -1,
       1,   257,    -1,     1,   252,    -1,   251,   252,    -1,    -1,
     251,   364,   368,   252,    -1,   366,    -1,   362,    -1,   251,
     252,    -1,    -1,   251,   367,   368,   252,    -1,   361,    -1,
     368,   361,    -1,   257,    -1,   303,   257,    -1,   209,   247,
     303,   248,   375,    -1,   210,   247,   276,   248,   373,    -1,
     372,    -1,   371,   372,    -1,   361,    -1,   211,   304,   255,
      -1,   212,   255,    -1,   251,   252,    -1,    -1,   251,   374,
     371,   252,    -1,   361,   208,   361,    -1,   361,    -1,    -1,
     205,   377,   247,   303,   248,   365,    -1,   207,   361,   205,
     247,   303,   248,   257,    -1,    -1,   206,   378,   379,   381,
     365,    -1,   247,   369,    -1,   247,   360,    -1,   247,     1,
     257,    -1,   303,    -1,    -1,   380,   257,   248,    -1,   380,
     257,   303,   248,    -1,     1,   257,   248,    -1,     1,   257,
     303,   248,    -1,   202,   257,    -1,   201,   257,    -1,   203,
     257,    -1,   203,   303,   257,    -1,   204,   169,   257,    -1,
      -1,   383,   384,    -1,   385,    -1,   309,    -1,   257,    -1,
     310,   366,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const unsigned short yyrline[] =
{
       0,   201,   201,   210,   212,   217,   219,   221,   223,   225,
     227,   229,   231,   236,   238,   240,   242,   244,   246,   248,
     253,   258,   260,   262,   267,   269,   273,   275,   280,   282,
     286,   290,   292,   297,   302,   311,   314,   313,   322,   326,
     333,   335,   337,   339,   341,   343,   345,   347,   352,   354,
     356,   358,   360,   362,   367,   371,   373,   375,   380,   382,
     384,   389,   391,   393,   398,   400,   402,   404,   406,   411,
     413,   415,   420,   422,   427,   429,   434,   436,   441,   443,
     448,   450,   455,   457,   462,   464,   469,   471,   476,   478,
     480,   482,   484,   486,   488,   490,   492,   494,   496,   501,
     503,   516,   521,   523,   528,   532,   535,   539,   542,   546,
     549,   553,   559,   566,   571,   573,   578,   580,   582,   584,
     589,   594,   596,   601,   603,   608,   610,   612,   614,   616,
     618,   622,   629,   631,   633,   639,   648,   650,   652,   654,
     656,   658,   660,   662,   667,   669,   674,   676,   678,   684,
     693,   698,   703,   705,   707,   709,   713,   715,   722,   724,
     729,   730,   735,   737,   742,   745,   744,   753,   755,   758,
     757,   762,   761,   769,   771,   773,   776,   775,   780,   779,
     789,   788,   795,   797,   802,   803,   808,   809,   812,   811,
     818,   820,   822,   824,   826,   828,   830,   832,   834,   839,
     841,   843,   845,   847,   849,   854,   856,   858,   860,   867,
     872,   874,   876,   878,   880,   882,   884,   886,   888,   890,
     895,   897,   899,   901,   906,   908,   910,   912,   914,   916,
     918,   920,   922,   924,   926,   928,   930,   932,   934,   936,
     938,   940,   942,   944,   946,   948,   950,   952,   954,   956,
     958,   962,   964,   970,   969,   974,   973,   978,   977,   982,
     981,   988,   992,  1005,  1012,  1014,  1019,  1020,  1022,  1027,
    1049,  1051,  1053,  1058,  1060,  1065,  1066,  1079,  1082,  1081,
    1090,  1103,  1119,  1124,  1126,  1131,  1133,  1135,  1137,  1139,
    1141,  1143,  1147,  1154,  1157,  1156,  1163,  1165,  1170,  1173,
    1172,  1179,  1181,  1186,  1188,  1193,  1195,  1200,  1202,  1207,
    1209,  1211,  1216,  1219,  1218,  1225,  1227,  1235,  1234,  1238,
    1241,  1240,  1247,  1249,  1251,  1258,  1261,  1265,  1267,  1269,
    1275,  1284,  1286,  1288,  1290,  1292,  1298,  1299,  1303,  1304,
    1306,  1310
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
  "T_SAMPLER_T", "T_VIV_GENERIC_GL_SAMPLER", "T_IMAGE1D_T",
  "T_IMAGE1D_ARRAY_T", "T_IMAGE1D_BUFFER_T", "T_IMAGE2D_ARRAY_T",
  "T_IMAGE2D_T", "T_IMAGE3D_T", "T_IMAGE2D_PTR_T",
  "T_IMAGE2D_DYNAMIC_ARRAY_T", "T_VIV_GENERIC_IMAGE_T",
  "T_VIV_GENERIC_GL_IMAGE", "T_SIZE_T", "T_EVENT_T", "T_PTRDIFF_T",
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
  "T_GENTYPE_PACKED", "T_F_GENTYPE_PACKED", "T_IU_GENTYPE_PACKED",
  "T_I_GENTYPE_PACKED", "T_U_GENTYPE_PACKED", "T_FLOATNXM", "T_DOUBLENXM",
  "T_BUILTIN_DATA_TYPE", "T_RESERVED_DATA_TYPE", "T_VIV_PACKED_DATA_TYPE",
  "T_IDENTIFIER", "T_TYPE_NAME", "T_FLOATCONSTANT", "T_UINTCONSTANT",
  "T_INTCONSTANT", "T_BOOLCONSTANT", "T_CHARCONSTANT", "T_STRING_LITERAL",
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
  "T_WORK_GROUP_SIZE_HINT", "T_KERNEL_SCALE_HINT", "T_ALWAYS_INLINE",
  "T_UNSIGNED", "T_STRUCT", "T_UNION", "T_TYPEDEF", "T_ENUM", "T_INLINE",
  "T_SIZEOF", "T_TYPE_CAST", "T_VEC_STEP", "T_TYPEOF",
  "T_ASM_OPND_BRACKET", "T_VERY_LAST_TERMINAL", "'('", "')'", "'['", "']'",
  "'{'", "'}'", "'.'", "','", "':'", "'='", "';'", "'!'", "'-'", "'~'",
  "'+'", "'*'", "'/'", "'%'", "'<'", "'>'", "'|'", "'^'", "'&'", "'?'",
  "$accept", "variable_identifier", "string_literal", "primary_expression",
  "postfix_expression", "integer_expression", "function_call",
  "function_call_with_parameters", "viv_asm_opnd", "viv_asm_modifier_list",
  "viv_asm_modifier", "type_cast", "function_call_header",
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
  "@6", "attribute_specifier", "@7", "attribute_specifier_list",
  "attribute_specifier_opt", "attribute_list", "@8", "attribute",
  "typeof_type_specifier", "fully_specified_type", "type_qualifier",
  "type_specifier", "type_name", "struct_or_union",
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
     485,   486,   487,   488,   489,   490,   491,   492,   493,   494,
     495,   496,   497,   498,   499,   500,   501,    40,    41,    91,
      93,   123,   125,    46,    44,    58,    61,    59,    33,    45,
     126,    43,    42,    47,    37,    60,    62,   124,    94,    38,
      63
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const unsigned short yyr1[] =
{
       0,   271,   272,   273,   273,   274,   274,   274,   274,   274,
     274,   274,   274,   275,   275,   275,   275,   275,   275,   275,
     276,   277,   277,   277,   278,   278,   279,   279,   280,   280,
     281,   282,   282,   283,   284,   285,   286,   285,   285,   285,
     287,   287,   287,   287,   287,   287,   287,   287,   288,   288,
     288,   288,   288,   288,   289,   289,   289,   289,   290,   290,
     290,   291,   291,   291,   292,   292,   292,   292,   292,   293,
     293,   293,   294,   294,   295,   295,   296,   296,   297,   297,
     298,   298,   299,   299,   300,   300,   301,   301,   302,   302,
     302,   302,   302,   302,   302,   302,   302,   302,   302,   303,
     303,   304,   305,   305,   306,   306,   306,   306,   306,   306,
     306,   306,   307,   307,   308,   308,   309,   309,   309,   309,
     310,   311,   311,   312,   312,   313,   313,   313,   313,   313,
     313,   313,   314,   314,   314,   314,   315,   315,   315,   315,
     315,   315,   315,   315,   316,   316,   317,   317,   317,   317,
     318,   318,   319,   319,   319,   319,   320,   320,   321,   321,
     322,   322,   323,   323,   324,   325,   324,   324,   324,   326,
     324,   327,   324,   328,   328,   328,   329,   328,   330,   328,
     332,   331,   333,   333,   334,   334,   335,   335,   336,   335,
     337,   337,   337,   337,   337,   337,   337,   337,   337,   338,
     338,   338,   338,   338,   338,   339,   339,   339,   339,   339,
     340,   340,   340,   340,   340,   340,   340,   340,   340,   340,
     341,   341,   341,   341,   342,   342,   342,   342,   342,   342,
     342,   342,   342,   342,   342,   342,   342,   342,   342,   342,
     342,   342,   342,   342,   342,   342,   342,   342,   342,   342,
     342,   343,   343,   345,   344,   346,   344,   347,   344,   348,
     344,   349,   349,   350,   351,   351,   352,   352,   352,   353,
     354,   354,   354,   355,   355,   356,   356,   357,   358,   357,
     359,   359,   360,   361,   361,   362,   362,   362,   362,   362,
     362,   362,   362,   363,   364,   363,   365,   365,   366,   367,
     366,   368,   368,   369,   369,   370,   370,   371,   371,   372,
     372,   372,   373,   374,   373,   375,   375,   377,   376,   376,
     378,   376,   379,   379,   379,   380,   380,   381,   381,   381,
     381,   382,   382,   382,   382,   382,   383,   383,   384,   384,
     384,   385
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const unsigned char yyr2[] =
{
       0,     2,     1,     1,     2,     1,     1,     1,     1,     1,
       1,     1,     3,     1,     4,     1,     3,     3,     2,     2,
       1,     2,     3,     2,     2,     3,     1,     4,     1,     3,
       3,     3,     4,     2,     2,     1,     0,     3,     3,     3,
       1,     2,     2,     4,     2,     2,     2,     2,     1,     1,
       1,     1,     1,     1,     1,     3,     3,     3,     1,     3,
       3,     1,     3,     3,     1,     3,     3,     3,     3,     1,
       3,     3,     1,     3,     1,     3,     1,     3,     1,     3,
       1,     3,     1,     3,     1,     5,     1,     3,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       3,     1,     2,     2,     6,     6,     5,     5,     5,     5,
       4,     4,     1,     3,     1,     3,     2,     3,     2,     2,
       2,     1,     1,     2,     3,     5,     5,     6,     3,     4,
       4,     5,     3,     4,     4,     5,     1,     2,     1,     2,
       2,     3,     2,     3,     1,     1,     1,     2,     2,     3,
       1,     2,     1,     2,     2,     3,     1,     2,     1,     3,
       0,     1,     3,     4,     1,     0,     3,     4,     5,     0,
       7,     0,     8,     1,     3,     4,     0,     6,     0,     7,
       0,     7,     1,     2,     0,     1,     0,     1,     0,     4,
       4,     1,     4,     8,     8,     8,     1,     4,     1,     1,
       4,     2,     5,     7,     1,     1,     2,     2,     3,     4,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     0,     7,     0,     6,     0,     7,     0,
       6,     1,     2,     3,     1,     3,     0,     2,     3,     1,
       1,     3,     3,     2,     4,     0,     2,     1,     0,     3,
       3,     2,     1,     1,     1,     1,     1,     1,     1,     1,
       2,     2,     2,     2,     0,     4,     1,     1,     2,     0,
       4,     1,     2,     1,     2,     5,     5,     1,     2,     1,
       3,     2,     2,     0,     4,     3,     1,     0,     6,     7,
       0,     5,     2,     2,     3,     1,     0,     3,     4,     3,
       4,     2,     2,     2,     3,     3,     0,     2,     1,     1,
       1,     2
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const unsigned short yydefact[] =
{
     336,     0,     1,   224,   227,   225,   226,   243,   244,   233,
     234,   235,   236,   237,   240,   238,   239,   241,   242,   248,
     249,   245,   246,   247,   228,   229,   230,   231,   232,   250,
     210,   211,   212,   217,   218,   213,   214,   215,   216,   184,
     219,     0,   251,   252,   165,     0,     0,     0,   340,   222,
     223,   339,   173,     0,   122,   121,     0,     0,   164,   182,
       0,     0,   150,   205,   220,     0,   221,   337,   338,     0,
     184,   185,     0,     0,     0,   103,     0,     0,   217,   218,
     222,   223,     0,   221,     0,   118,     0,   299,   341,   120,
       0,   144,   145,   136,   123,     0,   138,     0,   146,   206,
       0,   116,     0,   183,     0,   156,     0,   152,     0,   158,
     184,   151,   207,   102,   255,     0,   119,     0,     0,     0,
     180,   173,   166,     0,   114,     0,   112,     0,     0,     0,
       2,     8,     6,     7,     9,    10,     3,     0,     0,     0,
       0,     0,    50,    49,    51,    48,    53,    52,     5,    11,
      13,    40,    15,     0,    36,     0,     0,    54,    35,     0,
      58,    61,    64,    69,    72,    74,    76,    78,    80,    82,
      84,    86,    99,   204,     0,   199,   117,   298,     0,   124,
     137,   139,   140,     0,   142,   160,     0,   148,   184,   147,
     208,   184,     0,     0,     0,   154,   153,   157,   128,   184,
     174,   253,     0,     0,   259,     0,     0,     0,   186,     0,
       0,   111,   110,     0,     0,     0,   130,    33,     0,    41,
      42,     0,    46,    45,     0,    44,     0,     0,     4,    18,
      19,     0,     0,     0,    21,     0,    34,     0,     0,    23,
      24,    26,     0,    89,    90,    92,    91,    94,    95,    96,
      97,    98,    93,    88,     0,    47,    35,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   209,     0,
     201,     0,     2,     0,     0,     0,     0,   317,   320,     0,
       0,     0,   294,   303,     0,   282,   285,   301,   284,   283,
       0,   286,   287,   288,   289,   141,   143,   101,   161,     0,
     184,   149,   184,   132,   160,   184,   167,     0,   129,   159,
     155,   175,   176,     0,   266,     0,   261,   257,     0,   131,
       0,   126,   191,   196,     0,     0,     0,     0,     0,   198,
       0,   187,   107,   106,   115,   113,     0,   109,   108,     0,
       0,    12,    31,     0,    17,     0,    20,    16,    25,    37,
      22,     0,    39,    38,    87,    55,    56,    57,    60,    59,
      62,    63,    67,    68,    65,    66,    70,    71,    73,    75,
      77,    79,    81,    83,     0,   100,     0,     0,   292,   291,
     290,   332,   331,   333,     0,     0,     0,     0,     0,     0,
       0,   293,     0,   304,   300,   302,   162,   184,   134,   133,
       0,   168,   169,   125,   178,     0,     0,   184,     0,   264,
     184,   262,     0,     0,   127,     0,     0,     0,     0,     0,
       0,     0,   188,   105,   104,     0,    43,    32,    14,     0,
       0,    28,     0,   200,     0,   334,   335,     0,     0,     0,
       0,     0,     0,     0,   135,   163,   171,     0,     0,   269,
     270,   275,   177,   184,   184,   267,   266,   263,   256,     0,
     260,     0,     0,     0,     0,     0,     0,   181,     0,    32,
       0,     0,     0,    27,    85,   202,     0,     0,   323,   322,
       0,   325,     0,     0,     0,     0,     0,   295,     0,   170,
     179,     0,     0,     0,     0,   278,   277,   254,   268,   265,
     258,   197,   190,   192,     0,     0,     0,   189,     0,    30,
      29,     0,   324,     0,     0,   297,   321,   296,     0,   316,
     305,   313,   306,   172,     0,   281,   272,   271,   275,   273,
     276,     0,     0,     0,     0,   203,   318,   329,     0,   327,
       0,     0,     0,   312,     0,   280,     0,   279,     0,     0,
       0,   330,   328,   319,   315,     0,     0,   309,     0,   307,
     274,     0,     0,     0,     0,   311,   314,   308,   193,   194,
     195,   310
};

/* YYDEFGOTO[NTERM-NUM]. */
static const short yydefgoto[] =
{
      -1,   148,   149,   150,   151,   355,   152,   153,   240,   440,
     441,   154,   155,   156,   157,   237,   256,   159,   160,   161,
     162,   163,   164,   165,   166,   167,   168,   169,   170,   171,
     172,   254,   294,   308,    80,    81,   125,   126,   295,   121,
      53,    54,    55,    93,    94,    95,    96,    56,   108,   109,
     417,   309,   189,    57,    74,   457,   498,    58,   415,   458,
      59,   208,    71,    72,   340,   478,   341,   174,    61,    62,
      63,    64,    65,    83,   323,   202,   422,   328,   325,   326,
     418,   419,   461,   462,   503,   504,   505,   541,   506,   296,
     297,   298,   299,   402,   526,   527,   178,   300,   301,   302,
     568,   569,   532,   554,   530,   303,   396,   397,   449,   492,
     493,   304,     1,    67,    68
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -474
static const short yypact[] =
{
    -474,  2254,  -474,  -474,  -474,  -474,  -474,  -474,  -474,  -474,
    -474,  -474,  -474,  -474,  -474,  -474,  -474,  -474,  -474,  -474,
    -474,  -474,  -474,  -474,  -474,  -474,  -474,  -474,  -474,  -474,
    -474,  -474,  -474,  -217,  -156,  -474,  -474,  -474,  -474,  -153,
    -474,  -154,  -474,  -474,  -474,  -124,  3019,  -145,  -474,   -91,
    -203,  -474,  -110,   -68,   -67,  3178,  3220,  -176,  -474,  -474,
    2978,  -164,   362,   362,  -474,  -122,   -54,  -474,  -474,  3019,
    -153,  -153,  3019,   -31,  2819,   -53,    74,  -116,  -474,  -474,
    -474,  -474,  -164,  -474,  1720,  -474,   -34,    24,  -474,  -474,
    3178,  -474,  -474,  -474,  -474,  3220,  -474,  3199,   263,   362,
    -164,  -474,  3019,  -474,  -164,  -474,  -151,    54,    86,  -474,
    -188,  -474,  -474,    28,  -474,  -129,  -474,  -164,  3019,  -164,
    -474,  -474,  -474,    74,    30,  -167,  -474,    33,    74,    51,
      60,  -474,  -474,  -474,  -474,  -474,  -474,  3324,  3324,  3347,
      63,  1898,  -474,  -474,  -474,  -474,  -474,  -474,  -474,   125,
    -474,   -61,  -474,   -99,    62,  1951,  3347,  -474,    35,  3347,
      19,  -100,    67,  -133,    64,    46,    56,    55,   145,   147,
    -166,  -474,  -474,    79,    91,  -173,  -474,  -474,  1170,  -474,
    -474,  -474,  -474,  3220,  -474,  3347,  -164,  -157,  -187,    92,
    -474,  -187,  -164,    93,    94,    81,  -474,  -474,  -474,  -161,
      88,  -474,  3019,    97,  -474,   103,  -164,   105,   360,   -82,
    3347,  -474,  -474,    74,    74,   -40,  -474,  -474,  3347,  -474,
    -474,  1898,  -474,  -474,  2076,  -474,   -79,  -112,  -474,  -474,
    -474,   176,  3347,   177,  -474,  3347,  -474,  3347,   108,  -474,
    -474,   112,   -37,  -474,  -474,  -474,  -474,  -474,  -474,  -474,
    -474,  -474,  -474,  -474,  3347,  -474,  -474,  3347,  3347,  3347,
    3347,  3347,  3347,  3347,  3347,  3347,  3347,  3347,  3347,  3347,
    3347,  3347,  3347,  3347,  3347,  3347,  3347,  3347,  -474,  3347,
     111,   -95,  -121,   106,   107,  1773,   193,  -474,  -474,  1170,
     122,   123,   113,  -474,  -115,  -474,  -474,  -474,  -474,  -474,
     802,  -474,  -474,  -474,  -474,  -474,  -474,  -474,  -474,   121,
    -187,    92,  -161,  -474,  3347,  -161,   127,   132,  -474,  -474,
    -474,   136,  -474,  3019,  -164,  2419,  -474,  -474,  3019,  -474,
     141,  -474,  -474,   146,   150,   151,   152,   153,   157,  -474,
     -72,  -474,  -474,  -474,  -474,  -474,   -32,  -474,  -474,  -112,
     126,  -474,  -474,   158,  -474,   160,    79,  -474,  -474,  -474,
    -474,   242,  -474,  -474,  -474,  -474,  -474,  -474,    19,    19,
    -100,  -100,    67,    67,    67,    67,  -133,  -133,    64,    46,
      56,    55,   145,   147,    42,  -474,   165,  3347,  -474,  -474,
    -474,  -474,  -474,  -474,   -83,   159,   171,   174,   219,  3347,
    3347,  -474,  1170,  -474,  -474,  -474,  -474,  -161,  -474,  -474,
     175,   170,  -474,  -474,  -474,  2129,  2578,  -187,   -47,  -474,
    -153,  -474,  3019,  2619,  -474,  3347,   258,   262,  3347,  3347,
    3347,   181,  -474,  -474,  -474,   182,  -474,   184,  -474,   179,
    -170,  -474,  3347,  -474,   186,  -474,  -474,  3347,  1538,  1480,
     190,   -57,   191,   986,  -474,  -474,  -474,  2129,  2129,  -474,
    -474,  -198,  -474,  -153,  -161,  -474,  -164,  -474,  -474,  2778,
    -474,   197,   198,   207,   187,   203,   206,  -474,   360,  -474,
    3347,   292,   242,  -474,  -474,  -474,   -49,   205,  -474,  -474,
     210,    79,   211,  1354,  3347,  1170,   212,  -474,  2129,  -474,
    -474,  3347,   287,   -15,  2129,   213,  -474,  -474,  -474,  -474,
    -474,  -474,  -474,  -474,  3347,  3347,  3347,  -474,   220,  -474,
    -474,  1354,  -474,  3202,  3301,  -474,  -474,  -474,   -13,   264,
    -474,   222,  -474,  -474,   235,  -474,  -474,  -474,  -198,  -474,
    -474,  -198,   234,   236,   237,  -474,  -474,  -474,   -12,  -474,
     -10,   232,  1170,  -474,   618,  -474,  2129,  -474,  3347,  3347,
    3347,  -474,  -474,  -474,  -474,  3347,   245,  -474,   434,  -474,
    -474,   249,   254,   255,   250,  -474,  -474,  -474,  -474,  -474,
    -474,  -474
};

/* YYPGOTO[NTERM-NUM].  */
static const short yypgoto[] =
{
    -474,  -474,  -474,  -474,  -474,   109,  -474,  -474,   271,  -474,
      25,   118,  -474,  -474,  -142,  -474,   -81,  -474,    44,    65,
      -2,    61,   238,   240,   241,   243,   244,   239,  -474,  -169,
    -149,  -474,   -74,  -178,     0,     3,   -93,   302,   516,   518,
    -474,  -474,  -474,   -59,   432,   426,   -58,   296,   -96,   420,
      48,   231,   -97,  -474,  -474,  -474,  -474,   472,  -474,  -474,
     129,  -474,    -1,     2,  -474,  -474,    69,  -474,   -38,  -474,
     -27,  -474,  -474,     6,  -474,  -474,  -474,  -474,  -309,  -316,
    -474,    82,  -474,  -433,  -474,    11,  -474,  -474,     9,   114,
    -274,  -473,  -474,  -474,    38,   504,  -474,   161,   116,  -474,
    -474,    -7,  -474,  -474,  -474,  -474,  -474,  -474,  -474,  -474,
    -474,  -474,  -474,  -474,  -474
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -327
static const short yytable[] =
{
      60,    49,   187,   158,    50,   105,   241,    66,    82,   421,
     173,   196,   197,   199,   416,   398,   307,   255,   105,   423,
     525,   275,   104,    69,   499,   500,   405,    41,    98,    99,
     209,   117,   344,   211,   119,   215,   180,   181,   182,   184,
     203,   307,    41,    41,    77,    75,   175,   113,   525,   264,
     265,   501,    86,   127,  -184,   502,   219,   220,   223,   198,
     158,   185,   185,    98,   192,   533,    70,   226,    98,    41,
      98,   539,   118,    60,   158,   158,   279,    41,   100,   280,
     206,   101,   242,   106,   482,   212,   241,   213,   314,   107,
     311,   312,   185,    73,   315,   359,   483,   158,   107,   320,
     421,   386,    84,   227,   276,   364,    41,   421,    41,   110,
     307,   107,   200,   469,    41,   365,   366,   367,   342,   229,
     230,   346,   204,   570,   305,   306,   217,    76,   385,   114,
     129,   353,   266,   267,   390,   128,   352,   158,   231,   277,
     158,    87,   403,   350,   226,   307,   188,   226,   191,   234,
     107,   158,   193,   421,   158,   235,    98,   388,   356,   260,
     347,   261,   389,   362,   324,   205,    85,   207,   433,   351,
     343,   277,   213,   158,   445,   277,   431,    60,    49,   405,
      89,    50,   432,   349,    66,   536,   349,    90,   232,   103,
     313,   495,   233,   316,   115,   158,   158,   277,   123,   521,
     103,   321,   384,   116,   158,   277,   103,   466,   158,   444,
     467,   394,   348,   407,   213,   363,   120,   277,   307,   158,
     434,   529,   213,   176,   243,   244,   245,   246,   247,   248,
     249,   250,   251,   252,   310,   551,   561,   537,   562,   538,
     317,   277,   277,   124,   277,   262,   263,   471,   268,   269,
     474,   475,   476,   435,   330,   197,   307,   222,   225,   307,
     307,   307,   372,   373,   374,   375,   460,    30,    31,    32,
      78,    79,    35,    36,    37,    38,   177,    40,   564,   201,
     567,   257,   258,   259,   214,   324,   210,   324,    60,    49,
     324,   253,    50,   484,   567,    66,   277,   442,   216,    60,
      49,   228,   518,    50,   368,   369,    66,   217,   460,   460,
     224,   307,   408,   236,   409,   270,   107,   411,   158,   158,
     464,   158,   272,   534,   271,   451,   356,   370,   371,   376,
     377,   273,   307,   277,   158,   274,   542,   543,   544,   278,
     318,   314,   319,   107,   322,   307,   307,   307,   327,   460,
     329,    97,   331,   354,   357,   460,   360,   361,   111,   112,
     387,   158,   395,   391,   392,   401,   158,   158,   158,   399,
     400,   406,   158,   486,   436,   491,   158,   158,   324,   413,
     571,   572,   573,   412,   324,   324,    97,   574,   424,   307,
     307,   307,   414,   425,   186,   190,   307,   426,   427,   428,
     429,    60,    49,   195,   430,    50,   437,   460,    66,   454,
     438,   439,   158,   158,   158,   443,   446,   158,   447,   465,
     528,   448,   468,   158,   450,   455,   456,   472,   473,   477,
     479,   324,   105,   480,   481,   281,   485,   494,     3,   496,
     158,   514,   158,   158,     4,   511,   512,    60,    49,   548,
     550,    50,    60,    49,    66,   513,    50,   515,     5,    66,
     516,   519,   522,   531,   535,   507,   508,   523,   524,   540,
     545,   158,   552,   158,   553,   158,    30,    31,    32,    78,
      79,    35,    36,    37,    38,   555,    40,   158,   558,   563,
     559,   560,    60,    49,    60,    49,    50,   578,    50,    66,
     575,    66,   579,   580,     6,   581,   358,   520,   378,   452,
     106,   379,   185,   380,   383,   345,   381,    51,   382,    52,
      60,    49,   179,   183,    50,   107,   194,    66,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,   410,   122,   517,   509,   556,
     557,    60,    49,    60,    49,    50,    88,    50,    66,   546,
      66,   577,   488,   453,   489,     0,     0,    60,    49,     0,
       0,    50,     0,     0,    66,    30,    31,    32,    78,    79,
      35,    36,    37,    38,     0,    40,   332,   333,   334,   335,
       0,   336,   337,   338,   339,     0,     0,     0,    24,    25,
      26,    27,    28,   282,    29,   131,   132,   133,   134,   135,
     136,     0,     0,     0,   137,   138,     0,     0,     0,   281,
       0,     0,     3,     0,     0,     0,     0,     0,     4,     0,
       0,     0,     0,     0,     0,   283,   284,   285,   286,   287,
     288,   289,     5,   290,   291,   565,   566,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,     0,     0,
       0,     0,     0,     0,    41,     0,     0,     0,     0,     0,
      42,    43,    44,    45,    46,   139,     0,   140,    47,     0,
       0,   221,     0,     0,     0,   292,   576,     0,     6,     0,
       0,   293,   142,   143,   144,   145,   146,     0,     0,     0,
       0,     0,     0,   147,     0,     0,     0,     0,     0,     0,
       0,     0,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    24,    25,    26,    27,    28,   282,    29,   131,
     132,   133,   134,   135,   136,     0,     0,     0,   137,   138,
       0,     0,     0,   281,     0,     0,     3,     0,     0,     0,
       0,     0,     4,     0,     0,     0,     0,     0,     0,   283,
     284,   285,   286,   287,   288,   289,     5,   290,   291,   565,
     566,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,     0,     0,     0,     0,     0,     0,    41,     0,
       0,     0,     0,     0,    42,    43,    44,    45,    46,   139,
       0,   140,    47,     0,     0,   221,     0,     0,     0,   292,
       0,     0,     6,     0,     0,   293,   142,   143,   144,   145,
     146,     0,     0,     0,     0,     0,     0,   147,     0,     0,
       0,     0,     0,     0,     0,     0,     7,     8,     9,    10,
      11,    12,    13,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    24,    25,    26,    27,
      28,   282,    29,   131,   132,   133,   134,   135,   136,     0,
       0,     0,   137,   138,     0,     0,     0,   281,     0,     0,
       3,     0,     0,     0,     0,     0,     4,     0,     0,     0,
       0,     0,     0,   283,   284,   285,   286,   287,   288,   289,
       5,   290,   291,     0,     0,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,     0,     0,     0,     0,
       0,     0,    41,     0,     0,     0,     0,     0,    42,    43,
      44,    45,    46,   139,     0,   140,    47,     0,     0,   221,
       0,     0,     0,   292,   404,     0,     6,     0,     0,   293,
     142,   143,   144,   145,   146,     0,     0,     0,     0,     0,
       0,   147,     0,     0,     0,     0,     0,     0,     0,     0,
       7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      24,    25,    26,    27,    28,   282,    29,   131,   132,   133,
     134,   135,   136,     0,     0,     0,   137,   138,     0,     0,
       0,   281,     0,     0,     3,     0,     0,     0,     0,     0,
       4,     0,     0,     0,     0,     0,     0,   283,   284,   285,
     286,   287,   288,   289,     5,   290,   291,     0,     0,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
       0,     0,     0,     0,     0,     0,    41,     0,     0,     0,
       0,     0,    42,    43,    44,    45,    46,   139,     0,   140,
      47,     0,     0,   221,     0,     0,     0,   292,   497,     0,
       6,     0,     0,   293,   142,   143,   144,   145,   146,     0,
       0,     0,     0,     0,     0,   147,     0,     0,     0,     0,
       0,     0,     0,     0,     7,     8,     9,    10,    11,    12,
      13,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    24,    25,    26,    27,    28,   282,
      29,   131,   132,   133,   134,   135,   136,     0,     0,     0,
     137,   138,     0,     0,     0,   281,     0,     0,     3,     0,
       0,     0,     0,     0,     4,     0,     0,     0,     0,     0,
       0,   283,   284,   285,   286,   287,   288,   289,     5,   290,
     291,     0,     0,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,     0,     0,     0,     0,     0,     0,
      41,     0,     0,     0,     0,     0,    42,    43,    44,    45,
      46,   139,     0,   140,    47,     0,     0,   221,     0,     0,
       0,   292,     0,     0,     6,     0,     0,   293,   142,   143,
     144,   145,   146,     0,     0,     0,     0,     0,     0,   147,
       0,     0,     0,     0,     0,     0,     0,     0,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   490,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    24,    25,
      26,    27,    28,   282,    29,   131,   132,   133,   134,   135,
     136,     0,     0,     0,   137,   138,     0,     0,     0,   487,
       0,     0,     3,     0,     0,     0,     0,     0,     4,     0,
       0,     0,     0,     0,     0,   283,   284,   285,   286,   287,
     288,   289,     5,   290,   291,     0,     0,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,     0,     0,
       0,     0,     0,     0,    41,     0,     0,     0,     0,     0,
      42,    43,    44,    45,    46,   139,     0,   140,    47,     0,
       0,   221,     0,     0,     0,    87,     0,     0,     6,     0,
       0,   293,   142,   143,   144,   145,   146,     0,     0,     0,
       0,     0,     0,   147,     0,     0,     0,     0,     0,     0,
       0,     0,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,   130,
       0,   131,   132,   133,   134,   135,   136,     0,     0,     0,
     137,   138,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    24,    25,    26,    27,    28,   130,    29,   131,
     132,   133,   134,   135,   136,     0,     0,     0,   137,   138,
       0,   139,     0,   140,     3,     0,     0,   221,     0,     0,
       4,     0,     0,     0,     0,     0,     0,  -326,   142,   143,
     144,   145,   146,     0,     5,     0,     0,     0,     0,   147,
       0,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,     0,     0,     0,     0,     0,     0,    41,     0,
       0,     0,     0,     0,    42,    43,    44,    45,    46,   139,
       0,   140,    47,     0,     0,   221,     0,     0,     0,     0,
       6,     0,     0,     0,     0,   293,   142,   143,   144,   145,
     146,     0,     0,     0,     0,     0,     0,   147,     0,     0,
       0,     0,     0,     0,     7,     8,     9,    10,    11,    12,
      13,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    24,    25,    26,    27,    28,   130,
      29,   131,   132,   133,   134,   135,   136,     0,     0,     0,
     137,   138,     3,     0,     0,     0,     0,     0,     4,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     5,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    30,    31,    32,    78,    79,    35,    36,
      37,    38,   130,    40,   131,   132,   133,   134,   135,   136,
       0,     0,     0,   137,   138,   238,    42,    43,     0,    45,
       0,   139,     0,   140,    47,     0,     0,   141,     6,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   142,   143,
     144,   145,   146,     0,     0,     0,     0,     0,     0,   147,
       0,     0,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,     0,
       0,     0,     0,     0,   139,     0,   140,     0,     0,     0,
     221,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     393,   142,   143,   144,   145,   146,     0,     0,     0,     0,
       0,     0,   147,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    24,    25,    26,    27,    28,   130,    29,   131,
     132,   133,   134,   135,   136,     0,     0,     0,   137,   138,
       3,     0,     0,     0,     0,     0,     4,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       5,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    30,    31,    32,    78,    79,    35,    36,    37,    38,
     130,    40,   131,   132,   133,   134,   135,   136,     0,     0,
       0,   137,   138,     0,    42,    43,     0,    45,     0,   139,
       0,   140,    47,     0,     0,   221,     6,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   142,   143,   144,   145,
     146,     0,     0,     0,     0,     0,     0,   147,     0,     0,
       7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,     0,     0,     0,
       0,     0,   139,     0,   140,     0,     0,     0,   221,   239,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   142,
     143,   144,   145,   146,     0,     0,     0,     0,     0,     0,
     147,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      24,    25,    26,    27,    28,   130,    29,   131,   132,   133,
     134,   135,   136,     0,     2,     0,   137,   138,     3,     0,
       0,     0,     0,     0,     4,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     5,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    30,
      31,    32,    78,    79,    35,    36,    37,    38,   130,    40,
     131,   132,   133,   134,   135,   136,     0,     0,     0,   137,
     138,     0,    42,    43,     0,    45,     0,   139,     0,   140,
      47,     0,     0,   218,     6,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   142,   143,   144,   145,   146,     0,
       0,     0,     0,     0,     0,   147,     0,     0,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,     0,     0,     0,     0,     0,
     139,     0,   140,     0,     0,     0,   221,     0,     0,     0,
     459,     0,     0,     0,     0,     0,     0,   142,   143,   144,
     145,   146,     0,     0,     0,     0,     0,     0,   147,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    24,    25,
      26,    27,    28,     3,    29,     0,     0,     0,     0,     4,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     5,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,     0,     0,
       0,     0,     0,     0,    41,     0,     0,     0,     0,     6,
      42,    43,    44,    45,    46,     0,     0,     0,    47,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    48,     0,     7,     8,     9,    10,    11,    12,    13,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     3,    24,    25,    26,    27,    28,     4,    29,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     5,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     3,     0,     0,     0,     0,     0,     4,
       0,     0,    30,    31,    32,    78,    79,    35,    36,    37,
      38,     0,    40,     5,     0,     0,     0,     0,     6,     0,
       0,     0,     0,     0,     0,    42,    43,     0,    45,     0,
       0,     0,     0,    47,     0,     0,     0,     0,     0,     0,
       0,   420,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,     6,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     7,     8,     9,    10,    11,    12,    13,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    24,    25,    26,    27,    28,     0,    29,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     3,    24,    25,    26,    27,    28,     4,    29,
       0,    30,    31,    32,    78,    79,    35,    36,    37,    38,
       0,    40,     5,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    42,    43,     0,    45,     0,     0,
       0,     0,    47,     3,     0,     0,     0,     0,     0,     4,
     463,     0,    30,    31,    32,    78,    79,    35,    36,    37,
      38,     0,    40,     5,     0,     0,     0,     0,     6,     0,
       0,     0,     0,     0,     0,    42,    43,     0,    45,     0,
       0,     0,     0,    47,     0,     0,     0,     0,     0,     0,
       0,   470,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,     6,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     7,     8,     9,    10,    11,    12,    13,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    24,    25,    26,    27,    28,     0,    29,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     3,    24,    25,    26,    27,    28,     4,    29,
       0,    30,    31,    32,    78,    79,    35,    36,    37,    38,
       0,    40,     5,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    42,    43,     0,    45,     0,     0,
       0,     0,    47,     3,     0,     0,     0,     0,     0,     4,
     510,     0,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,     5,     0,     0,     0,     0,     6,    41,
       0,     0,     0,     0,     0,    42,    43,     0,    45,    46,
       0,     0,     0,    47,     0,     0,     0,     0,     0,     0,
       0,     0,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,     6,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     7,     8,     9,    10,    11,    12,    13,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    24,    25,    26,    27,    28,     0,    29,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     3,    24,    25,    26,    27,    28,     4,    29,
       0,    30,    31,    32,    78,    79,    35,    36,    37,    38,
     102,    40,     5,     3,     0,     0,     0,     0,    41,     4,
       0,     0,     0,     0,    42,    43,     0,    45,     0,     0,
       0,     0,    47,     5,     3,     0,     0,     0,     0,     0,
       4,     0,    30,    31,    32,    78,    79,    35,    36,    37,
      38,     0,    40,     0,     5,     0,     0,     0,     6,     0,
       0,     0,     0,     0,     0,    42,    43,     0,    45,     0,
       0,     0,     0,    47,     0,     0,     0,     0,     0,     6,
       0,     0,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,     0,
       6,     0,     0,     7,     8,     9,    10,    11,    12,    13,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
       0,     0,     0,     0,     7,     8,     9,    10,    11,    12,
      13,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    24,    25,    26,    27,    28,     0,    29,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    24,    25,    26,    27,    28,     0,    29,
       0,   130,     0,   131,   132,   133,   134,   135,   136,     0,
       0,     0,   137,   138,    24,    25,    26,    27,    28,     0,
      29,    30,    31,    32,    78,    79,    35,    36,    37,    38,
       0,    40,    91,    92,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    42,    43,     0,    45,     0,     0,
       0,     0,     0,    91,    92,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    42,    43,     0,    45,     0,
       0,     0,     0,   139,     0,   140,     0,     0,     0,   221,
     547,     0,     0,     0,     0,     0,    42,    43,     0,    45,
     142,   143,   144,   145,   146,     0,     0,     0,     0,     0,
     130,   147,   131,   132,   133,   134,   135,   136,     0,     0,
       0,   137,   138,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   130,     0,   131,   132,   133,   134,   135,
     136,     0,     0,     0,   137,   138,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   130,     0,   131,   132,
     133,   134,   135,   136,     0,     0,     0,   137,   138,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   139,     0,   140,     0,     0,     0,   221,   549,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   142,
     143,   144,   145,   146,     0,   139,     0,   140,     0,     0,
     147,   218,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   142,   143,   144,   145,   146,     0,   139,     0,
     140,     0,     0,   147,   221,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   142,   143,   144,   145,   146,
       0,     0,     0,     0,     0,     0,   147
};

static const short yycheck[] =
{
       1,     1,    98,    84,     1,   169,   155,     1,    46,   325,
      84,   107,   169,   110,   323,   289,   185,   159,   169,   328,
     493,   187,    60,   240,   457,   458,   300,   230,    55,    56,
     123,    69,   210,   200,    72,   128,    95,    95,    97,    97,
     169,   210,   230,   230,    45,   169,    84,   169,   521,   182,
     183,   249,    50,   169,   257,   253,   137,   138,   139,   247,
     141,   249,   249,    90,   102,   498,   222,   141,    95,   230,
      97,   504,    70,    74,   155,   156,   249,   230,   254,   175,
     118,   257,   156,   247,   254,   252,   235,   254,   249,   262,
     187,   188,   249,   247,   191,   237,   266,   178,   262,   195,
     416,   279,   247,   141,   270,   254,   230,   423,   230,    61,
     279,   262,   110,   422,   230,   257,   258,   259,   200,   180,
     181,   214,   251,   556,   183,   183,   247,   251,   277,   251,
      82,   227,   265,   266,   255,   251,   248,   218,   199,   254,
     221,   251,   257,   224,   218,   314,    98,   221,   100,   248,
     262,   232,   104,   469,   235,   254,   183,   252,   232,   259,
     200,   261,   257,   200,   202,   117,   257,   119,   200,   248,
     252,   254,   254,   254,   257,   254,   248,   178,   178,   453,
     248,   178,   254,   221,   178,   200,   224,   254,   249,    60,
     188,   248,   253,   191,    65,   276,   277,   254,   251,   248,
      71,   199,   276,   257,   285,   254,    77,   254,   289,   387,
     257,   285,   252,   310,   254,   252,   247,   254,   387,   300,
     252,   495,   254,   257,   189,   190,   191,   192,   193,   194,
     195,   196,   197,   198,   186,   248,   248,   252,   248,   254,
     192,   254,   254,   169,   254,   178,   179,   425,   184,   185,
     428,   429,   430,   349,   206,   169,   425,   139,   140,   428,
     429,   430,   264,   265,   266,   267,   415,   213,   214,   215,
     216,   217,   218,   219,   220,   221,   252,   223,   552,   251,
     554,   262,   263,   264,   251,   323,   256,   325,   289,   289,
     328,   256,   289,   442,   568,   289,   254,   255,   247,   300,
     300,   176,   480,   300,   260,   261,   300,   247,   457,   458,
     247,   480,   310,   251,   312,   269,   262,   315,   399,   400,
     417,   402,   267,   501,   268,   399,   400,   262,   263,   268,
     269,   186,   501,   254,   415,   188,   514,   515,   516,   248,
     247,   249,   248,   262,   256,   514,   515,   516,   251,   498,
     247,    55,   247,   177,   177,   504,   248,   245,    62,    63,
     249,   442,   169,   257,   257,   252,   447,   448,   449,   247,
     247,   250,   453,   447,   248,   449,   457,   458,   416,   247,
     558,   559,   560,   256,   422,   423,    90,   565,   247,   558,
     559,   560,   256,   247,    98,    99,   565,   247,   247,   247,
     247,   402,   402,   107,   247,   402,   248,   556,   402,   407,
     250,   169,   493,   494,   495,   250,   257,   498,   247,   417,
     494,   247,   420,   504,   205,   250,   256,   169,   166,   248,
     248,   469,   169,   249,   255,     1,   250,   247,     4,   248,
     521,   254,   523,   524,    10,   248,   248,   448,   448,   523,
     524,   448,   453,   453,   448,   248,   453,   254,    24,   453,
     254,   169,   257,   251,   177,   463,   464,   257,   257,   256,
     250,   552,   208,   554,   252,   556,   213,   214,   215,   216,
     217,   218,   219,   220,   221,   250,   223,   568,   254,   257,
     254,   254,   493,   493,   495,   495,   493,   248,   495,   493,
     255,   495,   248,   248,    70,   255,   235,   482,   270,   400,
     247,   271,   249,   272,   275,   213,   273,     1,   274,     1,
     521,   521,    90,    97,   521,   262,   106,   521,    94,    95,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   314,    74,   478,   466,   538,
     541,   552,   552,   554,   554,   552,    52,   554,   552,   521,
     554,   568,   448,   402,   448,    -1,    -1,   568,   568,    -1,
      -1,   568,    -1,    -1,   568,   213,   214,   215,   216,   217,
     218,   219,   220,   221,    -1,   223,   226,   227,   228,   229,
      -1,   231,   232,   233,   234,    -1,    -1,    -1,   164,   165,
     166,   167,   168,   169,   170,   171,   172,   173,   174,   175,
     176,    -1,    -1,    -1,   180,   181,    -1,    -1,    -1,     1,
      -1,    -1,     4,    -1,    -1,    -1,    -1,    -1,    10,    -1,
      -1,    -1,    -1,    -1,    -1,   201,   202,   203,   204,   205,
     206,   207,    24,   209,   210,   211,   212,   213,   214,   215,
     216,   217,   218,   219,   220,   221,   222,   223,    -1,    -1,
      -1,    -1,    -1,    -1,   230,    -1,    -1,    -1,    -1,    -1,
     236,   237,   238,   239,   240,   241,    -1,   243,   244,    -1,
      -1,   247,    -1,    -1,    -1,   251,   252,    -1,    70,    -1,
      -1,   257,   258,   259,   260,   261,   262,    -1,    -1,    -1,
      -1,    -1,    -1,   269,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    94,    95,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   164,   165,   166,   167,   168,   169,   170,   171,
     172,   173,   174,   175,   176,    -1,    -1,    -1,   180,   181,
      -1,    -1,    -1,     1,    -1,    -1,     4,    -1,    -1,    -1,
      -1,    -1,    10,    -1,    -1,    -1,    -1,    -1,    -1,   201,
     202,   203,   204,   205,   206,   207,    24,   209,   210,   211,
     212,   213,   214,   215,   216,   217,   218,   219,   220,   221,
     222,   223,    -1,    -1,    -1,    -1,    -1,    -1,   230,    -1,
      -1,    -1,    -1,    -1,   236,   237,   238,   239,   240,   241,
      -1,   243,   244,    -1,    -1,   247,    -1,    -1,    -1,   251,
      -1,    -1,    70,    -1,    -1,   257,   258,   259,   260,   261,
     262,    -1,    -1,    -1,    -1,    -1,    -1,   269,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    94,    95,    96,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   164,   165,   166,   167,
     168,   169,   170,   171,   172,   173,   174,   175,   176,    -1,
      -1,    -1,   180,   181,    -1,    -1,    -1,     1,    -1,    -1,
       4,    -1,    -1,    -1,    -1,    -1,    10,    -1,    -1,    -1,
      -1,    -1,    -1,   201,   202,   203,   204,   205,   206,   207,
      24,   209,   210,    -1,    -1,   213,   214,   215,   216,   217,
     218,   219,   220,   221,   222,   223,    -1,    -1,    -1,    -1,
      -1,    -1,   230,    -1,    -1,    -1,    -1,    -1,   236,   237,
     238,   239,   240,   241,    -1,   243,   244,    -1,    -1,   247,
      -1,    -1,    -1,   251,   252,    -1,    70,    -1,    -1,   257,
     258,   259,   260,   261,   262,    -1,    -1,    -1,    -1,    -1,
      -1,   269,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      94,    95,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     164,   165,   166,   167,   168,   169,   170,   171,   172,   173,
     174,   175,   176,    -1,    -1,    -1,   180,   181,    -1,    -1,
      -1,     1,    -1,    -1,     4,    -1,    -1,    -1,    -1,    -1,
      10,    -1,    -1,    -1,    -1,    -1,    -1,   201,   202,   203,
     204,   205,   206,   207,    24,   209,   210,    -1,    -1,   213,
     214,   215,   216,   217,   218,   219,   220,   221,   222,   223,
      -1,    -1,    -1,    -1,    -1,    -1,   230,    -1,    -1,    -1,
      -1,    -1,   236,   237,   238,   239,   240,   241,    -1,   243,
     244,    -1,    -1,   247,    -1,    -1,    -1,   251,   252,    -1,
      70,    -1,    -1,   257,   258,   259,   260,   261,   262,    -1,
      -1,    -1,    -1,    -1,    -1,   269,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    94,    95,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   164,   165,   166,   167,   168,   169,
     170,   171,   172,   173,   174,   175,   176,    -1,    -1,    -1,
     180,   181,    -1,    -1,    -1,     1,    -1,    -1,     4,    -1,
      -1,    -1,    -1,    -1,    10,    -1,    -1,    -1,    -1,    -1,
      -1,   201,   202,   203,   204,   205,   206,   207,    24,   209,
     210,    -1,    -1,   213,   214,   215,   216,   217,   218,   219,
     220,   221,   222,   223,    -1,    -1,    -1,    -1,    -1,    -1,
     230,    -1,    -1,    -1,    -1,    -1,   236,   237,   238,   239,
     240,   241,    -1,   243,   244,    -1,    -1,   247,    -1,    -1,
      -1,   251,    -1,    -1,    70,    -1,    -1,   257,   258,   259,
     260,   261,   262,    -1,    -1,    -1,    -1,    -1,    -1,   269,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    94,    95,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,     1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   164,   165,
     166,   167,   168,   169,   170,   171,   172,   173,   174,   175,
     176,    -1,    -1,    -1,   180,   181,    -1,    -1,    -1,     1,
      -1,    -1,     4,    -1,    -1,    -1,    -1,    -1,    10,    -1,
      -1,    -1,    -1,    -1,    -1,   201,   202,   203,   204,   205,
     206,   207,    24,   209,   210,    -1,    -1,   213,   214,   215,
     216,   217,   218,   219,   220,   221,   222,   223,    -1,    -1,
      -1,    -1,    -1,    -1,   230,    -1,    -1,    -1,    -1,    -1,
     236,   237,   238,   239,   240,   241,    -1,   243,   244,    -1,
      -1,   247,    -1,    -1,    -1,   251,    -1,    -1,    70,    -1,
      -1,   257,   258,   259,   260,   261,   262,    -1,    -1,    -1,
      -1,    -1,    -1,   269,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    94,    95,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   169,
      -1,   171,   172,   173,   174,   175,   176,    -1,    -1,    -1,
     180,   181,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   164,   165,   166,   167,   168,   169,   170,   171,
     172,   173,   174,   175,   176,    -1,    -1,    -1,   180,   181,
      -1,   241,    -1,   243,     4,    -1,    -1,   247,    -1,    -1,
      10,    -1,    -1,    -1,    -1,    -1,    -1,   257,   258,   259,
     260,   261,   262,    -1,    24,    -1,    -1,    -1,    -1,   269,
      -1,   213,   214,   215,   216,   217,   218,   219,   220,   221,
     222,   223,    -1,    -1,    -1,    -1,    -1,    -1,   230,    -1,
      -1,    -1,    -1,    -1,   236,   237,   238,   239,   240,   241,
      -1,   243,   244,    -1,    -1,   247,    -1,    -1,    -1,    -1,
      70,    -1,    -1,    -1,    -1,   257,   258,   259,   260,   261,
     262,    -1,    -1,    -1,    -1,    -1,    -1,   269,    -1,    -1,
      -1,    -1,    -1,    -1,    94,    95,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   164,   165,   166,   167,   168,   169,
     170,   171,   172,   173,   174,   175,   176,    -1,    -1,    -1,
     180,   181,     4,    -1,    -1,    -1,    -1,    -1,    10,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    24,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   213,   214,   215,   216,   217,   218,   219,
     220,   221,   169,   223,   171,   172,   173,   174,   175,   176,
      -1,    -1,    -1,   180,   181,     4,   236,   237,    -1,   239,
      -1,   241,    -1,   243,   244,    -1,    -1,   247,    70,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   258,   259,
     260,   261,   262,    -1,    -1,    -1,    -1,    -1,    -1,   269,
      -1,    -1,    94,    95,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,    -1,
      -1,    -1,    -1,    -1,   241,    -1,   243,    -1,    -1,    -1,
     247,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     257,   258,   259,   260,   261,   262,    -1,    -1,    -1,    -1,
      -1,    -1,   269,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   164,   165,   166,   167,   168,   169,   170,   171,
     172,   173,   174,   175,   176,    -1,    -1,    -1,   180,   181,
       4,    -1,    -1,    -1,    -1,    -1,    10,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      24,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   213,   214,   215,   216,   217,   218,   219,   220,   221,
     169,   223,   171,   172,   173,   174,   175,   176,    -1,    -1,
      -1,   180,   181,    -1,   236,   237,    -1,   239,    -1,   241,
      -1,   243,   244,    -1,    -1,   247,    70,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   258,   259,   260,   261,
     262,    -1,    -1,    -1,    -1,    -1,    -1,   269,    -1,    -1,
      94,    95,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,    -1,    -1,    -1,
      -1,    -1,   241,    -1,   243,    -1,    -1,    -1,   247,   248,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   258,
     259,   260,   261,   262,    -1,    -1,    -1,    -1,    -1,    -1,
     269,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     164,   165,   166,   167,   168,   169,   170,   171,   172,   173,
     174,   175,   176,    -1,     0,    -1,   180,   181,     4,    -1,
      -1,    -1,    -1,    -1,    10,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    24,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   213,
     214,   215,   216,   217,   218,   219,   220,   221,   169,   223,
     171,   172,   173,   174,   175,   176,    -1,    -1,    -1,   180,
     181,    -1,   236,   237,    -1,   239,    -1,   241,    -1,   243,
     244,    -1,    -1,   247,    70,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   258,   259,   260,   261,   262,    -1,
      -1,    -1,    -1,    -1,    -1,   269,    -1,    -1,    94,    95,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,    -1,    -1,    -1,    -1,    -1,
     241,    -1,   243,    -1,    -1,    -1,   247,    -1,    -1,    -1,
     251,    -1,    -1,    -1,    -1,    -1,    -1,   258,   259,   260,
     261,   262,    -1,    -1,    -1,    -1,    -1,    -1,   269,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   164,   165,
     166,   167,   168,     4,   170,    -1,    -1,    -1,    -1,    10,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    24,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   213,   214,   215,
     216,   217,   218,   219,   220,   221,   222,   223,    -1,    -1,
      -1,    -1,    -1,    -1,   230,    -1,    -1,    -1,    -1,    70,
     236,   237,   238,   239,   240,    -1,    -1,    -1,   244,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   257,    -1,    94,    95,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,     4,   164,   165,   166,   167,   168,    10,   170,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    24,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,     4,    -1,    -1,    -1,    -1,    -1,    10,
      -1,    -1,   213,   214,   215,   216,   217,   218,   219,   220,
     221,    -1,   223,    24,    -1,    -1,    -1,    -1,    70,    -1,
      -1,    -1,    -1,    -1,    -1,   236,   237,    -1,   239,    -1,
      -1,    -1,    -1,   244,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   252,    94,    95,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,    70,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    94,    95,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   164,   165,   166,   167,   168,    -1,   170,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,     4,   164,   165,   166,   167,   168,    10,   170,
      -1,   213,   214,   215,   216,   217,   218,   219,   220,   221,
      -1,   223,    24,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   236,   237,    -1,   239,    -1,    -1,
      -1,    -1,   244,     4,    -1,    -1,    -1,    -1,    -1,    10,
     252,    -1,   213,   214,   215,   216,   217,   218,   219,   220,
     221,    -1,   223,    24,    -1,    -1,    -1,    -1,    70,    -1,
      -1,    -1,    -1,    -1,    -1,   236,   237,    -1,   239,    -1,
      -1,    -1,    -1,   244,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   252,    94,    95,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,    70,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    94,    95,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   164,   165,   166,   167,   168,    -1,   170,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,     4,   164,   165,   166,   167,   168,    10,   170,
      -1,   213,   214,   215,   216,   217,   218,   219,   220,   221,
      -1,   223,    24,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   236,   237,    -1,   239,    -1,    -1,
      -1,    -1,   244,     4,    -1,    -1,    -1,    -1,    -1,    10,
     252,    -1,   213,   214,   215,   216,   217,   218,   219,   220,
     221,   222,   223,    24,    -1,    -1,    -1,    -1,    70,   230,
      -1,    -1,    -1,    -1,    -1,   236,   237,    -1,   239,   240,
      -1,    -1,    -1,   244,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    94,    95,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,    70,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    94,    95,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   164,   165,   166,   167,   168,    -1,   170,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,     4,   164,   165,   166,   167,   168,    10,   170,
      -1,   213,   214,   215,   216,   217,   218,   219,   220,   221,
     222,   223,    24,     4,    -1,    -1,    -1,    -1,   230,    10,
      -1,    -1,    -1,    -1,   236,   237,    -1,   239,    -1,    -1,
      -1,    -1,   244,    24,     4,    -1,    -1,    -1,    -1,    -1,
      10,    -1,   213,   214,   215,   216,   217,   218,   219,   220,
     221,    -1,   223,    -1,    24,    -1,    -1,    -1,    70,    -1,
      -1,    -1,    -1,    -1,    -1,   236,   237,    -1,   239,    -1,
      -1,    -1,    -1,   244,    -1,    -1,    -1,    -1,    -1,    70,
      -1,    -1,    94,    95,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,    -1,
      70,    -1,    -1,    94,    95,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
      -1,    -1,    -1,    -1,    94,    95,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   164,   165,   166,   167,   168,    -1,   170,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   164,   165,   166,   167,   168,    -1,   170,
      -1,   169,    -1,   171,   172,   173,   174,   175,   176,    -1,
      -1,    -1,   180,   181,   164,   165,   166,   167,   168,    -1,
     170,   213,   214,   215,   216,   217,   218,   219,   220,   221,
      -1,   223,   224,   225,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   236,   237,    -1,   239,    -1,    -1,
      -1,    -1,    -1,   224,   225,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   236,   237,    -1,   239,    -1,
      -1,    -1,    -1,   241,    -1,   243,    -1,    -1,    -1,   247,
     248,    -1,    -1,    -1,    -1,    -1,   236,   237,    -1,   239,
     258,   259,   260,   261,   262,    -1,    -1,    -1,    -1,    -1,
     169,   269,   171,   172,   173,   174,   175,   176,    -1,    -1,
      -1,   180,   181,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   169,    -1,   171,   172,   173,   174,   175,
     176,    -1,    -1,    -1,   180,   181,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   169,    -1,   171,   172,
     173,   174,   175,   176,    -1,    -1,    -1,   180,   181,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   241,    -1,   243,    -1,    -1,    -1,   247,   248,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   258,
     259,   260,   261,   262,    -1,   241,    -1,   243,    -1,    -1,
     269,   247,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   258,   259,   260,   261,   262,    -1,   241,    -1,
     243,    -1,    -1,   269,   247,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   258,   259,   260,   261,   262,
      -1,    -1,    -1,    -1,    -1,    -1,   269
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const unsigned short yystos[] =
{
       0,   383,     0,     4,    10,    24,    70,    94,    95,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   164,   165,   166,   167,   168,   170,
     213,   214,   215,   216,   217,   218,   219,   220,   221,   222,
     223,   230,   236,   237,   238,   239,   240,   244,   257,   305,
     306,   309,   310,   311,   312,   313,   318,   324,   328,   331,
     333,   339,   340,   341,   342,   343,   344,   384,   385,   240,
     222,   333,   334,   247,   325,   169,   251,   333,   216,   217,
     305,   306,   339,   344,   247,   257,   334,   251,   366,   248,
     254,   224,   225,   314,   315,   316,   317,   318,   341,   341,
     254,   257,   222,   331,   339,   169,   247,   262,   319,   320,
     321,   318,   318,   169,   251,   331,   257,   339,   334,   339,
     247,   310,   328,   251,   169,   307,   308,   169,   251,   321,
     169,   171,   172,   173,   174,   175,   176,   180,   181,   241,
     243,   247,   258,   259,   260,   261,   262,   269,   272,   273,
     274,   275,   277,   278,   282,   283,   284,   285,   287,   288,
     289,   290,   291,   292,   293,   294,   295,   296,   297,   298,
     299,   300,   301,   303,   338,   339,   257,   252,   367,   315,
     314,   317,   314,   316,   317,   249,   318,   319,   321,   323,
     318,   321,   339,   321,   320,   318,   319,   169,   247,   323,
     334,   251,   346,   169,   251,   321,   339,   321,   332,   307,
     256,   200,   252,   254,   251,   307,   247,   247,   247,   287,
     287,   247,   282,   287,   247,   282,   303,   339,   176,   180,
     181,   199,   249,   253,   248,   254,   251,   286,     4,   248,
     279,   301,   303,   189,   190,   191,   192,   193,   194,   195,
     196,   197,   198,   256,   302,   285,   287,   262,   263,   264,
     259,   261,   178,   179,   182,   183,   265,   266,   184,   185,
     269,   268,   267,   186,   188,   187,   270,   254,   248,   249,
     319,     1,   169,   201,   202,   203,   204,   205,   206,   207,
     209,   210,   251,   257,   303,   309,   360,   361,   362,   363,
     368,   369,   370,   376,   382,   314,   317,   300,   304,   322,
     321,   323,   323,   334,   249,   323,   334,   321,   247,   248,
     319,   334,   256,   345,   339,   349,   350,   251,   348,   247,
     321,   247,   226,   227,   228,   229,   231,   232,   233,   234,
     335,   337,   200,   252,   304,   308,   307,   200,   252,   339,
     287,   248,   248,   319,   177,   276,   303,   177,   279,   285,
     248,   245,   200,   252,   301,   285,   285,   285,   289,   289,
     290,   290,   291,   291,   291,   291,   292,   292,   293,   294,
     295,   296,   297,   298,   303,   301,   304,   249,   252,   257,
     255,   257,   257,   257,   303,   169,   377,   378,   361,   247,
     247,   252,   364,   257,   252,   361,   250,   323,   334,   334,
     322,   334,   256,   247,   256,   329,   349,   321,   351,   352,
     252,   350,   347,   349,   247,   247,   247,   247,   247,   247,
     247,   248,   254,   200,   252,   319,   248,   248,   250,   169,
     280,   281,   255,   250,   304,   257,   257,   247,   247,   379,
     205,   303,   276,   368,   334,   250,   256,   326,   330,   251,
     301,   353,   354,   252,   323,   334,   254,   257,   334,   349,
     252,   304,   169,   166,   304,   304,   304,   248,   336,   248,
     249,   255,   254,   266,   301,   250,   303,     1,   360,   369,
       1,   303,   380,   381,   247,   248,   248,   252,   327,   354,
     354,   249,   253,   355,   356,   357,   359,   334,   334,   352,
     252,   248,   248,   248,   254,   254,   254,   337,   304,   169,
     281,   248,   257,   257,   257,   362,   365,   366,   303,   361,
     375,   251,   373,   354,   304,   177,   200,   252,   254,   354,
     256,   358,   304,   304,   304,   250,   365,   248,   303,   248,
     303,   248,   208,   252,   374,   250,   356,   359,   254,   254,
     254,   248,   248,   257,   361,   211,   212,   361,   371,   372,
     354,   304,   304,   304,   304,   255,   252,   372,   248,   248,
     248,   255
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

#define yyerrok        (yyerrstatus = 0)
#define yyclearin    (yychar = YYEMPTY)
#define YYEMPTY        (-2)
#define YYEOF        0

#define YYACCEPT    goto yyacceptlab
#define YYABORT        goto yyabortlab
#define YYERROR        goto yyerrorlab


/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */

#define YYFAIL        goto yyerrlab

#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                    \
do                                \
  if (yychar == YYEMPTY && yylen == 1)                \
    {                                \
      yychar = (Token);                        \
      yylval = (Value);                        \
      yytoken = YYTRANSLATE (yychar);                \
      YYPOPSTACK;                        \
      goto yybackup;                        \
    }                                \
  else                                \
    {                                 \
      yyerror ("syntax error: cannot back up");\
      YYERROR;                            \
    }                                \
while (0)

#define YYTERROR    1
#define YYERRCODE    256

/* YYLLOC_DEFAULT -- Compute the default location (before the actions
   are run).  */

#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)        \
   ((Current).first_line   = (Rhs)[1].first_line,    \
    (Current).first_column = (Rhs)[1].first_column,    \
    (Current).last_line    = (Rhs)[N].last_line,    \
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

# define YYDPRINTF(Args)            \
do {                        \
  if (yydebug)                    \
    YYFPRINTF Args;                \
} while (0)

# define YYDSYMPRINT(Args)            \
do {                        \
  if (yydebug)                    \
    yysymprint Args;                \
} while (0)

# define YYDSYMPRINTF(Title, Token, Value, Location)        \
do {                                \
  if (yydebug)                            \
    {                                \
      YYFPRINTF (stderr, "%s ", Title);                \
      yysymprint (stderr,                     \
                  Token, Value);    \
      YYFPRINTF (stderr, "\n");                    \
    }                                \
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

# define YY_STACK_PRINT(Bottom, Top)                \
do {                                \
  if (yydebug)                            \
    yy_stack_print ((Bottom), (Top));                \
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

# define YY_REDUCE_PRINT(Rule)        \
do {                    \
  if (yydebug)                \
    yy_reduce_print (Rule);        \
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
  short    yyssa[YYINITDEPTH];
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
  yychar = YYEMPTY;        /* Cause a token to be read.  */

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
#line 202 "gc_cl.y"
    { yyval.expr = clParseVariableIdentifier(Compiler, &yyvsp[0].token);
          if(yyval.expr == gcvNULL) {
             YYERROR;
          }
        ;}
    break;

  case 3:
#line 211 "gc_cl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 4:
#line 213 "gc_cl.y"
    { yyval.token = clParseCatStringLiteral(Compiler, &yyvsp[-1].token, &yyvsp[0].token); ;}
    break;

  case 5:
#line 218 "gc_cl.y"
    { yyval.expr = yyvsp[0].expr; ;}
    break;

  case 6:
#line 220 "gc_cl.y"
    { yyval.expr = clParseScalarConstant(Compiler, &yyvsp[0].token); ;}
    break;

  case 7:
#line 222 "gc_cl.y"
    { yyval.expr = clParseScalarConstant(Compiler, &yyvsp[0].token); ;}
    break;

  case 8:
#line 224 "gc_cl.y"
    { yyval.expr = clParseScalarConstant(Compiler, &yyvsp[0].token); ;}
    break;

  case 9:
#line 226 "gc_cl.y"
    { yyval.expr = clParseScalarConstant(Compiler, &yyvsp[0].token); ;}
    break;

  case 10:
#line 228 "gc_cl.y"
    { yyval.expr = clParseScalarConstant(Compiler, &yyvsp[0].token); ;}
    break;

  case 11:
#line 230 "gc_cl.y"
    { yyval.expr = clParseStringLiteral(Compiler, &yyvsp[0].token); ;}
    break;

  case 12:
#line 232 "gc_cl.y"
    { yyval.expr = yyvsp[-1].expr; ;}
    break;

  case 13:
#line 237 "gc_cl.y"
    { yyval.expr = yyvsp[0].expr; ;}
    break;

  case 14:
#line 239 "gc_cl.y"
    { yyval.expr = clParseSubscriptExpr(Compiler, yyvsp[-3].expr, yyvsp[-1].expr); ;}
    break;

  case 15:
#line 241 "gc_cl.y"
    { yyval.expr = clParseFuncCallExprAsExpr(Compiler, yyvsp[0].funcCall); ;}
    break;

  case 16:
#line 243 "gc_cl.y"
    { yyval.expr = clParseFieldSelectionExpr(Compiler, yyvsp[-2].expr, &yyvsp[0].token); ;}
    break;

  case 17:
#line 245 "gc_cl.y"
    { yyval.expr = clParsePtrFieldSelectionExpr(Compiler, yyvsp[-2].expr, &yyvsp[0].token); ;}
    break;

  case 18:
#line 247 "gc_cl.y"
    { yyval.expr = clParseIncOrDecExpr(Compiler, gcvNULL, clvUNARY_POST_INC, yyvsp[-1].expr); ;}
    break;

  case 19:
#line 249 "gc_cl.y"
    { yyval.expr = clParseIncOrDecExpr(Compiler, gcvNULL, clvUNARY_POST_DEC, yyvsp[-1].expr); ;}
    break;

  case 20:
#line 254 "gc_cl.y"
    { yyval.expr = yyvsp[0].expr; ;}
    break;

  case 21:
#line 259 "gc_cl.y"
    { yyval.funcCall = yyvsp[-1].funcCall; ;}
    break;

  case 22:
#line 261 "gc_cl.y"
    { yyval.funcCall = yyvsp[-2].funcCall; ;}
    break;

  case 23:
#line 263 "gc_cl.y"
    { yyval.funcCall = yyvsp[-1].funcCall; ;}
    break;

  case 24:
#line 268 "gc_cl.y"
    { yyval.funcCall = clParseFuncCallArgument(Compiler, yyvsp[-1].funcCall, yyvsp[0].expr); ;}
    break;

  case 25:
#line 270 "gc_cl.y"
    { yyval.funcCall = clParseFuncCallArgument(Compiler, yyvsp[-2].funcCall, yyvsp[0].expr); ;}
    break;

  case 26:
#line 274 "gc_cl.y"
    { yyval.expr = yyvsp[0].expr; ;}
    break;

  case 27:
#line 276 "gc_cl.y"
    { yyval.expr = clParseAsmAppendOperandModifiers(Compiler, yyvsp[-3].expr, &yyvsp[-1].asmModifiers); ;}
    break;

  case 28:
#line 281 "gc_cl.y"
    { yyval.asmModifiers = clParseAsmAppendModifier(Compiler, gcvNULL, &yyvsp[0].asmModifier); ;}
    break;

  case 29:
#line 283 "gc_cl.y"
    { yyval.asmModifiers = clParseAsmAppendModifier(Compiler, &yyvsp[-2].asmModifiers, &yyvsp[0].asmModifier); ;}
    break;

  case 30:
#line 287 "gc_cl.y"
    { yyval.asmModifier = clParseAsmModifier(Compiler, &yyvsp[-2].token, &yyvsp[0].token); ;}
    break;

  case 31:
#line 291 "gc_cl.y"
    { yyval.decl = yyvsp[-1].decl; ;}
    break;

  case 32:
#line 293 "gc_cl.y"
    { yyval.decl = clParseCreateDecl(Compiler, &yyvsp[-2].decl, yyvsp[-1].typeQualifierList, gcvNULL); ;}
    break;

  case 33:
#line 298 "gc_cl.y"
    { yyval.funcCall = clParseFuncCallHeaderExpr(Compiler, &yyvsp[-1].token, gcvNULL); ;}
    break;

  case 34:
#line 303 "gc_cl.y"
    {
        clParseCastExprBegin(Compiler, &yyvsp[-1].decl);
        (void) gcoOS_ZeroMemory((gctPOINTER)&yyval.token, sizeof(clsLexToken));
        yyval.token.type = T_TYPE_CAST;
    ;}
    break;

  case 35:
#line 312 "gc_cl.y"
    { yyval.expr = yyvsp[0].expr; ;}
    break;

  case 36:
#line 314 "gc_cl.y"
    {
           clParseCastExprBegin(Compiler, gcvNULL);
               (void) gcoOS_ZeroMemory((gctPOINTER)&yyval.token, sizeof(clsLexToken));
           yyval.token.type = T_TYPE_CAST;
        ;}
    break;

  case 37:
#line 320 "gc_cl.y"
    { yyval.expr = clParseCastExprEnd(Compiler, &yyvsp[-2].decl, yyvsp[0].expr);
                ;}
    break;

  case 38:
#line 324 "gc_cl.y"
    { yyval.expr = clParseCastExprEnd(Compiler, gcvNULL, yyvsp[-1].expr);
                ;}
    break;

  case 39:
#line 328 "gc_cl.y"
    { yyval.expr = clParseCastExprEnd(Compiler, gcvNULL, yyvsp[-1].expr);
                ;}
    break;

  case 40:
#line 334 "gc_cl.y"
    { yyval.expr = yyvsp[0].expr; ;}
    break;

  case 41:
#line 336 "gc_cl.y"
    { yyval.expr = clParseIncOrDecExpr(Compiler, &yyvsp[-1].token, clvUNARY_PRE_INC, yyvsp[0].expr); ;}
    break;

  case 42:
#line 338 "gc_cl.y"
    { yyval.expr = clParseIncOrDecExpr(Compiler, &yyvsp[-1].token, clvUNARY_PRE_DEC, yyvsp[0].expr); ;}
    break;

  case 43:
#line 340 "gc_cl.y"
    { yyval.expr = clParseVecStep(Compiler, &yyvsp[-3].token, &yyvsp[-1].expr->decl); ;}
    break;

  case 44:
#line 342 "gc_cl.y"
    { yyval.expr = clParseVecStep(Compiler, &yyvsp[-1].token, &yyvsp[0].decl); ;}
    break;

  case 45:
#line 344 "gc_cl.y"
    { yyval.expr = clParseSizeofExpr(Compiler, &yyvsp[-1].token, yyvsp[0].expr); ;}
    break;

  case 46:
#line 346 "gc_cl.y"
    { yyval.expr = clParseSizeofTypeDecl(Compiler, &yyvsp[-1].token, &yyvsp[0].decl); ;}
    break;

  case 47:
#line 348 "gc_cl.y"
    { yyval.expr = clParseNormalUnaryExpr(Compiler, &yyvsp[-1].token, yyvsp[0].expr); ;}
    break;

  case 48:
#line 353 "gc_cl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 49:
#line 355 "gc_cl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 50:
#line 357 "gc_cl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 51:
#line 359 "gc_cl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 52:
#line 361 "gc_cl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 53:
#line 363 "gc_cl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 54:
#line 368 "gc_cl.y"
    {
           yyval.expr = yyvsp[0].expr;
        ;}
    break;

  case 55:
#line 372 "gc_cl.y"
    { yyval.expr = clParseNormalBinaryExpr(Compiler, yyvsp[-2].expr, &yyvsp[-1].token, yyvsp[0].expr); ;}
    break;

  case 56:
#line 374 "gc_cl.y"
    { yyval.expr = clParseNormalBinaryExpr(Compiler, yyvsp[-2].expr, &yyvsp[-1].token, yyvsp[0].expr); ;}
    break;

  case 57:
#line 376 "gc_cl.y"
    { yyval.expr = clParseNormalBinaryExpr(Compiler, yyvsp[-2].expr, &yyvsp[-1].token, yyvsp[0].expr); ;}
    break;

  case 58:
#line 381 "gc_cl.y"
    { yyval.expr = yyvsp[0].expr; ;}
    break;

  case 59:
#line 383 "gc_cl.y"
    { yyval.expr = clParseNormalBinaryExpr(Compiler, yyvsp[-2].expr, &yyvsp[-1].token, yyvsp[0].expr); ;}
    break;

  case 60:
#line 385 "gc_cl.y"
    { yyval.expr = clParseNormalBinaryExpr(Compiler, yyvsp[-2].expr, &yyvsp[-1].token, yyvsp[0].expr); ;}
    break;

  case 61:
#line 390 "gc_cl.y"
    { yyval.expr = yyvsp[0].expr; ;}
    break;

  case 62:
#line 392 "gc_cl.y"
    { yyval.expr = clParseNormalBinaryExpr(Compiler, yyvsp[-2].expr, &yyvsp[-1].token, yyvsp[0].expr); ;}
    break;

  case 63:
#line 394 "gc_cl.y"
    { yyval.expr = clParseNormalBinaryExpr(Compiler, yyvsp[-2].expr, &yyvsp[-1].token, yyvsp[0].expr); ;}
    break;

  case 64:
#line 399 "gc_cl.y"
    { yyval.expr = yyvsp[0].expr; ;}
    break;

  case 65:
#line 401 "gc_cl.y"
    { yyval.expr = clParseNormalBinaryExpr(Compiler, yyvsp[-2].expr, &yyvsp[-1].token, yyvsp[0].expr); ;}
    break;

  case 66:
#line 403 "gc_cl.y"
    { yyval.expr = clParseNormalBinaryExpr(Compiler, yyvsp[-2].expr, &yyvsp[-1].token, yyvsp[0].expr); ;}
    break;

  case 67:
#line 405 "gc_cl.y"
    { yyval.expr = clParseNormalBinaryExpr(Compiler, yyvsp[-2].expr, &yyvsp[-1].token, yyvsp[0].expr); ;}
    break;

  case 68:
#line 407 "gc_cl.y"
    { yyval.expr = clParseNormalBinaryExpr(Compiler, yyvsp[-2].expr, &yyvsp[-1].token, yyvsp[0].expr); ;}
    break;

  case 69:
#line 412 "gc_cl.y"
    { yyval.expr = yyvsp[0].expr; ;}
    break;

  case 70:
#line 414 "gc_cl.y"
    { yyval.expr = clParseNormalBinaryExpr(Compiler, yyvsp[-2].expr, &yyvsp[-1].token, yyvsp[0].expr); ;}
    break;

  case 71:
#line 416 "gc_cl.y"
    { yyval.expr = clParseNormalBinaryExpr(Compiler, yyvsp[-2].expr, &yyvsp[-1].token, yyvsp[0].expr); ;}
    break;

  case 72:
#line 421 "gc_cl.y"
    { yyval.expr = yyvsp[0].expr; ;}
    break;

  case 73:
#line 423 "gc_cl.y"
    { yyval.expr = clParseNormalBinaryExpr(Compiler, yyvsp[-2].expr, &yyvsp[-1].token, yyvsp[0].expr); ;}
    break;

  case 74:
#line 428 "gc_cl.y"
    { yyval.expr = yyvsp[0].expr; ;}
    break;

  case 75:
#line 430 "gc_cl.y"
    { yyval.expr = clParseNormalBinaryExpr(Compiler, yyvsp[-2].expr, &yyvsp[-1].token, yyvsp[0].expr); ;}
    break;

  case 76:
#line 435 "gc_cl.y"
    { yyval.expr = yyvsp[0].expr; ;}
    break;

  case 77:
#line 437 "gc_cl.y"
    { yyval.expr = clParseNormalBinaryExpr(Compiler, yyvsp[-2].expr, &yyvsp[-1].token, yyvsp[0].expr); ;}
    break;

  case 78:
#line 442 "gc_cl.y"
    { yyval.expr = yyvsp[0].expr; ;}
    break;

  case 79:
#line 444 "gc_cl.y"
    { yyval.expr = clParseNormalBinaryExpr(Compiler, yyvsp[-2].expr, &yyvsp[-1].token, yyvsp[0].expr); ;}
    break;

  case 80:
#line 449 "gc_cl.y"
    { yyval.expr = yyvsp[0].expr; ;}
    break;

  case 81:
#line 451 "gc_cl.y"
    { yyval.expr = clParseNormalBinaryExpr(Compiler, yyvsp[-2].expr, &yyvsp[-1].token, yyvsp[0].expr); ;}
    break;

  case 82:
#line 456 "gc_cl.y"
    { yyval.expr = yyvsp[0].expr; ;}
    break;

  case 83:
#line 458 "gc_cl.y"
    { yyval.expr = clParseNormalBinaryExpr(Compiler, yyvsp[-2].expr, &yyvsp[-1].token, yyvsp[0].expr); ;}
    break;

  case 84:
#line 463 "gc_cl.y"
    { yyval.expr = yyvsp[0].expr; ;}
    break;

  case 85:
#line 465 "gc_cl.y"
    { yyval.expr = clParseSelectionExpr(Compiler, yyvsp[-4].expr, yyvsp[-2].expr, yyvsp[0].expr); ;}
    break;

  case 86:
#line 470 "gc_cl.y"
    { yyval.expr = yyvsp[0].expr;;}
    break;

  case 87:
#line 472 "gc_cl.y"
    { yyval.expr = clParseAssignmentExpr(Compiler, yyvsp[-2].expr, &yyvsp[-1].token, yyvsp[0].expr); ;}
    break;

  case 88:
#line 477 "gc_cl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 89:
#line 479 "gc_cl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 90:
#line 481 "gc_cl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 91:
#line 483 "gc_cl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 92:
#line 485 "gc_cl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 93:
#line 487 "gc_cl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 94:
#line 489 "gc_cl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 95:
#line 491 "gc_cl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 96:
#line 493 "gc_cl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 97:
#line 495 "gc_cl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 98:
#line 497 "gc_cl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 99:
#line 502 "gc_cl.y"
    { yyval.expr = yyvsp[0].expr; ;}
    break;

  case 100:
#line 504 "gc_cl.y"
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

  case 101:
#line 517 "gc_cl.y"
    { yyval.expr = yyvsp[0].expr; ;}
    break;

  case 102:
#line 522 "gc_cl.y"
    { yyval.dataType = clParseTaggedDecl(Compiler, &yyvsp[-1].token, &yyvsp[0].token); ;}
    break;

  case 103:
#line 524 "gc_cl.y"
    { yyval.dataType = clParseTaggedDecl(Compiler, &yyvsp[-1].token, &yyvsp[0].token); ;}
    break;

  case 104:
#line 531 "gc_cl.y"
    { yyval.dataType = clParseEnumSpecifier(Compiler, &yyvsp[-5].token, yyvsp[-4].attr, &yyvsp[-3].token, (gctPOINTER)yyvsp[-1].enumeratorList); ;}
    break;

  case 105:
#line 534 "gc_cl.y"
    { yyval.dataType = clParseEnumSpecifier(Compiler, &yyvsp[-5].token, yyvsp[-4].attr, &yyvsp[-3].token, (gctPOINTER)yyvsp[-1].enumeratorList); ;}
    break;

  case 106:
#line 538 "gc_cl.y"
    { yyval.dataType = clParseEnumSpecifier(Compiler, &yyvsp[-4].token, gcvNULL, &yyvsp[-3].token, (gctPOINTER)yyvsp[-1].enumeratorList); ;}
    break;

  case 107:
#line 541 "gc_cl.y"
    { yyval.dataType = clParseEnumSpecifier(Compiler, &yyvsp[-4].token, gcvNULL, &yyvsp[-3].token, (gctPOINTER)yyvsp[-1].enumeratorList); ;}
    break;

  case 108:
#line 545 "gc_cl.y"
    { yyval.dataType = clParseEnumSpecifier(Compiler, &yyvsp[-4].token, yyvsp[-3].attr, gcvNULL, (gctPOINTER)yyvsp[-1].enumeratorList); ;}
    break;

  case 109:
#line 548 "gc_cl.y"
    { yyval.dataType = clParseEnumSpecifier(Compiler, &yyvsp[-4].token, yyvsp[-3].attr, gcvNULL, (gctPOINTER)yyvsp[-1].enumeratorList); ;}
    break;

  case 110:
#line 552 "gc_cl.y"
    { yyval.dataType = clParseEnumSpecifier(Compiler, &yyvsp[-3].token, gcvNULL, gcvNULL, (gctPOINTER)yyvsp[-1].enumeratorList); ;}
    break;

  case 111:
#line 555 "gc_cl.y"
    { yyval.dataType = clParseEnumSpecifier(Compiler, &yyvsp[-3].token, gcvNULL, gcvNULL, (gctPOINTER)yyvsp[-1].enumeratorList); ;}
    break;

  case 112:
#line 560 "gc_cl.y"
    {
           slsSLINK_LIST *enumList;

           slmSLINK_LIST_Initialize(enumList);
                   yyval.enumeratorList = clParseAddEnumerator(Compiler, yyvsp[0].enumeratorName, enumList);
        ;}
    break;

  case 113:
#line 567 "gc_cl.y"
    { yyval.enumeratorList = clParseAddEnumerator(Compiler, yyvsp[0].enumeratorName, yyvsp[-2].enumeratorList); ;}
    break;

  case 114:
#line 572 "gc_cl.y"
    { yyval.enumeratorName = clParseEnumerator(Compiler, &yyvsp[0].token, gcvNULL); ;}
    break;

  case 115:
#line 574 "gc_cl.y"
    { yyval.enumeratorName = clParseEnumerator(Compiler, &yyvsp[-2].token, yyvsp[0].expr); ;}
    break;

  case 116:
#line 579 "gc_cl.y"
    { yyval.statement = clParseDeclaration(Compiler, yyvsp[-1].declOrDeclList); ;}
    break;

  case 117:
#line 581 "gc_cl.y"
    { yyval.statement = clParseEnumTags(Compiler, yyvsp[-2].dataType, yyvsp[-1].attr); ;}
    break;

  case 118:
#line 583 "gc_cl.y"
    { yyval.statement = clParseTags(Compiler, yyvsp[-1].dataType); ;}
    break;

  case 119:
#line 585 "gc_cl.y"
    { yyval.statement = clParseTags(Compiler, yyvsp[-1].dataType); ;}
    break;

  case 120:
#line 590 "gc_cl.y"
    { yyval.funcName = yyvsp[-1].funcName; ;}
    break;

  case 121:
#line 595 "gc_cl.y"
    { yyval.funcName = yyvsp[0].funcName; ;}
    break;

  case 122:
#line 597 "gc_cl.y"
    { yyval.funcName = yyvsp[0].funcName; ;}
    break;

  case 123:
#line 602 "gc_cl.y"
    { yyval.funcName = clParseParameterList(Compiler, yyvsp[-1].funcName, yyvsp[0].paramName); ;}
    break;

  case 124:
#line 604 "gc_cl.y"
    { yyval.funcName = clParseParameterList(Compiler, yyvsp[-2].funcName, yyvsp[0].paramName); ;}
    break;

  case 125:
#line 609 "gc_cl.y"
    { yyval.funcName = clParseKernelFuncHeader(Compiler, yyvsp[-4].attr, &yyvsp[-2].decl, &yyvsp[-1].token); ;}
    break;

  case 126:
#line 611 "gc_cl.y"
    { yyval.funcName = clParseKernelFuncHeader(Compiler, yyvsp[-3].attr, &yyvsp[-2].decl, &yyvsp[-1].token); ;}
    break;

  case 127:
#line 613 "gc_cl.y"
    { yyval.funcName = clParseExternKernelFuncHeader(Compiler, yyvsp[-3].attr, &yyvsp[-2].decl, &yyvsp[-1].token); ;}
    break;

  case 128:
#line 615 "gc_cl.y"
    { yyval.funcName = clParseFuncHeader(Compiler, &yyvsp[-2].decl, &yyvsp[-1].token); ;}
    break;

  case 129:
#line 617 "gc_cl.y"
    { yyval.funcName = clParseFuncHeaderWithAttr(Compiler, yyvsp[-3].attr, &yyvsp[-2].decl, &yyvsp[-1].token); ;}
    break;

  case 130:
#line 619 "gc_cl.y"
    { yyval.funcName = clParseFuncHeader(Compiler, &yyvsp[-2].decl, &yyvsp[-1].token);
          if(yyval.funcName) yyval.funcName->u.funcInfo.isInline = gcvTRUE;
        ;}
    break;

  case 131:
#line 623 "gc_cl.y"
    { yyval.funcName = clParseFuncHeader(Compiler, &yyvsp[-2].decl, &yyvsp[-1].token);
          if(yyval.funcName) yyval.funcName->u.funcInfo.isInline = gcvTRUE;
        ;}
    break;

  case 132:
#line 630 "gc_cl.y"
    { yyval.paramName = clParseParameterDecl(Compiler, &yyvsp[-2].decl, &yyvsp[-1].token, yyvsp[0].attr); ;}
    break;

  case 133:
#line 632 "gc_cl.y"
    { yyval.paramName = clParseArrayParameterDecl(Compiler, &yyvsp[-3].decl, &yyvsp[-2].token, yyvsp[-1].expr, yyvsp[0].attr); ;}
    break;

  case 134:
#line 634 "gc_cl.y"
    {
                    clsDECL decl;
                    decl = clParseQualifiedType(Compiler, yyvsp[-2].typeQualifierList, gcvTRUE, &yyvsp[-3].decl);
                    yyval.paramName = clParseParameterDecl(Compiler, &decl, &yyvsp[-1].token, yyvsp[0].attr);
        ;}
    break;

  case 135:
#line 640 "gc_cl.y"
    {
                    clsDECL decl;
                    decl = clParseQualifiedType(Compiler, yyvsp[-3].typeQualifierList, gcvTRUE, &yyvsp[-4].decl);
            yyval.paramName = clParseArrayParameterDecl(Compiler, &decl, &yyvsp[-2].token, yyvsp[-1].expr, yyvsp[0].attr);
        ;}
    break;

  case 136:
#line 649 "gc_cl.y"
    { yyval.paramName = clParseQualifiedParameterDecl(Compiler, gcvNULL, gcvNULL, yyvsp[0].paramName); ;}
    break;

  case 137:
#line 651 "gc_cl.y"
    { yyval.paramName = clParseQualifiedParameterDecl(Compiler, gcvNULL, &yyvsp[-1].token, yyvsp[0].paramName); ;}
    break;

  case 138:
#line 653 "gc_cl.y"
    { yyval.paramName = clParseQualifiedParameterDecl(Compiler, gcvNULL, gcvNULL, yyvsp[0].paramName); ;}
    break;

  case 139:
#line 655 "gc_cl.y"
    { yyval.paramName = clParseQualifiedParameterDecl(Compiler, gcvNULL, &yyvsp[-1].token, yyvsp[0].paramName); ;}
    break;

  case 140:
#line 657 "gc_cl.y"
    { yyval.paramName = clParseQualifiedParameterDecl(Compiler, yyvsp[-1].typeQualifierList, gcvNULL, yyvsp[0].paramName); ;}
    break;

  case 141:
#line 659 "gc_cl.y"
    { yyval.paramName = clParseQualifiedParameterDecl(Compiler, yyvsp[-2].typeQualifierList, &yyvsp[-1].token, yyvsp[0].paramName); ;}
    break;

  case 142:
#line 661 "gc_cl.y"
    { yyval.paramName = clParseQualifiedParameterDecl(Compiler, yyvsp[-1].typeQualifierList, gcvNULL, yyvsp[0].paramName); ;}
    break;

  case 143:
#line 663 "gc_cl.y"
    { yyval.paramName = clParseQualifiedParameterDecl(Compiler, yyvsp[-2].typeQualifierList, &yyvsp[-1].token, yyvsp[0].paramName); ;}
    break;

  case 144:
#line 668 "gc_cl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 145:
#line 670 "gc_cl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 146:
#line 675 "gc_cl.y"
    { yyval.paramName = clParseParameterDecl(Compiler, &yyvsp[0].decl, gcvNULL, gcvNULL); ;}
    break;

  case 147:
#line 677 "gc_cl.y"
    { yyval.paramName = clParseArrayParameterDecl(Compiler, &yyvsp[-1].decl, gcvNULL, yyvsp[0].expr, gcvNULL); ;}
    break;

  case 148:
#line 679 "gc_cl.y"
    {
                    clsDECL decl;
                    decl = clParseQualifiedType(Compiler, yyvsp[0].typeQualifierList, gcvTRUE, &yyvsp[-1].decl);
            yyval.paramName = clParseParameterDecl(Compiler, &decl, gcvNULL, gcvNULL);
        ;}
    break;

  case 149:
#line 685 "gc_cl.y"
    {
                    clsDECL decl;
                    decl = clParseQualifiedType(Compiler, yyvsp[-1].typeQualifierList, gcvTRUE, &yyvsp[-2].decl);
            yyval.paramName = clParseArrayParameterDecl(Compiler, &decl, gcvNULL, yyvsp[0].expr, gcvNULL);
        ;}
    break;

  case 150:
#line 694 "gc_cl.y"
    {
            yyval.typeQualifierList = clParseTypeQualifierList(Compiler, &yyvsp[0].token,
                                          clParseEmptyTypeQualifierList(Compiler));
        ;}
    break;

  case 151:
#line 699 "gc_cl.y"
    {yyval.typeQualifierList = clParseTypeQualifierList(Compiler, &yyvsp[-1].token, yyvsp[0].typeQualifierList); ;}
    break;

  case 152:
#line 704 "gc_cl.y"
    { yyval.typeQualifierList = clParseEmptyTypeQualifierList(Compiler); ;}
    break;

  case 153:
#line 706 "gc_cl.y"
    { yyval.typeQualifierList = clParsePointerTypeQualifier(Compiler, gcvNULL, yyvsp[0].typeQualifierList); ;}
    break;

  case 154:
#line 708 "gc_cl.y"
    {yyval.typeQualifierList = yyvsp[0].typeQualifierList;;}
    break;

  case 155:
#line 710 "gc_cl.y"
    {yyval.typeQualifierList = clParsePointerTypeQualifier(Compiler, yyvsp[-1].typeQualifierList, yyvsp[0].typeQualifierList); ;}
    break;

  case 156:
#line 714 "gc_cl.y"
    {yyval.token = yyvsp[0].token;;}
    break;

  case 157:
#line 716 "gc_cl.y"
    {
          yyval.token = yyvsp[0].token;
          yyval.token.u.identifier.ptrDscr = yyvsp[-1].typeQualifierList;
        ;}
    break;

  case 158:
#line 723 "gc_cl.y"
    {  yyval.token = yyvsp[0].token; ;}
    break;

  case 159:
#line 725 "gc_cl.y"
    { yyval.token = yyvsp[-1].token; ;}
    break;

  case 160:
#line 729 "gc_cl.y"
    { yyval.expr = clParseNullExpr(Compiler, &yyvsp[0].token); ;}
    break;

  case 161:
#line 731 "gc_cl.y"
    { yyval.expr = yyvsp[0].expr; ;}
    break;

  case 162:
#line 736 "gc_cl.y"
    { yyval.expr = yyvsp[-1].expr; ;}
    break;

  case 163:
#line 738 "gc_cl.y"
    { yyval.expr = clParseArrayDeclarator(Compiler, &yyvsp[-2].token, yyvsp[-3].expr, yyvsp[-1].expr); ;}
    break;

  case 164:
#line 743 "gc_cl.y"
    { yyval.declOrDeclList = yyvsp[0].declOrDeclList; ;}
    break;

  case 165:
#line 745 "gc_cl.y"
    {
               cloCOMPILER_PushParserState(Compiler, clvPARSER_IN_TYPEDEF);
        ;}
    break;

  case 166:
#line 749 "gc_cl.y"
    {
           yyval.declOrDeclList = clParseTypeDef(Compiler, yyvsp[0].declOrDeclList);
           cloCOMPILER_PopParserState(Compiler);
        ;}
    break;

  case 167:
#line 754 "gc_cl.y"
    { yyval.declOrDeclList = clParseVariableDeclList(Compiler, yyvsp[-3].declOrDeclList, &yyvsp[-1].token, yyvsp[0].attr); ;}
    break;

  case 168:
#line 756 "gc_cl.y"
    { yyval.declOrDeclList = clParseArrayVariableDeclList(Compiler, yyvsp[-4].declOrDeclList, &yyvsp[-2].token, yyvsp[-1].expr, yyvsp[0].attr); ;}
    break;

  case 169:
#line 758 "gc_cl.y"
    { yyval.declOrDeclList = clParseVariableDeclListInit(Compiler, yyvsp[-4].declOrDeclList, &yyvsp[-2].token, yyvsp[-1].attr); ;}
    break;

  case 170:
#line 760 "gc_cl.y"
    { yyval.declOrDeclList = clParseFinishDeclListInit(Compiler, yyvsp[-1].declOrDeclList, yyvsp[0].expr); ;}
    break;

  case 171:
#line 762 "gc_cl.y"
    { yyval.declOrDeclList = clParseArrayVariableDeclListInit(Compiler, yyvsp[-5].declOrDeclList, &yyvsp[-3].token, yyvsp[-2].expr, yyvsp[-1].attr); ;}
    break;

  case 172:
#line 764 "gc_cl.y"
    { yyval.declOrDeclList = clParseFinishDeclListInit(Compiler, yyvsp[-1].declOrDeclList, yyvsp[0].expr); ;}
    break;

  case 173:
#line 770 "gc_cl.y"
    { yyval.declOrDeclList = clParseFuncDecl(Compiler, yyvsp[0].funcName); ;}
    break;

  case 174:
#line 772 "gc_cl.y"
    { yyval.declOrDeclList = clParseVariableDecl(Compiler, &yyvsp[-2].decl, &yyvsp[-1].token, yyvsp[0].attr); ;}
    break;

  case 175:
#line 774 "gc_cl.y"
    { yyval.declOrDeclList = clParseArrayVariableDecl(Compiler, &yyvsp[-3].decl, &yyvsp[-2].token, yyvsp[-1].expr, yyvsp[0].attr); ;}
    break;

  case 176:
#line 776 "gc_cl.y"
    { yyval.declOrDeclList = clParseVariableDeclInit(Compiler, &yyvsp[-3].decl, &yyvsp[-2].token, yyvsp[-1].attr); ;}
    break;

  case 177:
#line 778 "gc_cl.y"
    { yyval.declOrDeclList = clParseFinishDeclInit(Compiler, yyvsp[-1].declOrDeclList, yyvsp[0].expr); ;}
    break;

  case 178:
#line 780 "gc_cl.y"
    { yyval.declOrDeclList = clParseArrayVariableDeclInit(Compiler, &yyvsp[-4].decl, &yyvsp[-3].token, yyvsp[-2].expr, yyvsp[-1].attr); ;}
    break;

  case 179:
#line 782 "gc_cl.y"
    { yyval.declOrDeclList = clParseFinishDeclInit(Compiler, yyvsp[-1].declOrDeclList, yyvsp[0].expr); ;}
    break;

  case 180:
#line 789 "gc_cl.y"
    { yyval.attr = gcvNULL; ;}
    break;

  case 181:
#line 791 "gc_cl.y"
    { yyval.attr = yyvsp[-2].attr; ;}
    break;

  case 182:
#line 796 "gc_cl.y"
    { yyval.attr = yyvsp[0].attr; ;}
    break;

  case 183:
#line 798 "gc_cl.y"
    {yyval.attr = clParseMergeAttributeSpecifier(Compiler, yyvsp[0].attr, yyvsp[-1].attr); ;}
    break;

  case 184:
#line 802 "gc_cl.y"
    { yyval.attr = gcvNULL; ;}
    break;

  case 185:
#line 804 "gc_cl.y"
    { yyval.attr = yyvsp[0].attr; ;}
    break;

  case 186:
#line 808 "gc_cl.y"
    { yyval.attr = yyvsp[0].attr; ;}
    break;

  case 187:
#line 810 "gc_cl.y"
    { yyval.attr = yyvsp[0].attr; ;}
    break;

  case 188:
#line 812 "gc_cl.y"
    { yyval.attr = yyvsp[-1].attr; ;}
    break;

  case 189:
#line 814 "gc_cl.y"
    { yyval.attr = yyvsp[0].attr; ;}
    break;

  case 190:
#line 819 "gc_cl.y"
    { yyval.attr = clParseAttributeEndianType(Compiler, yyvsp[-4].attr,  &yyvsp[-1].token); ;}
    break;

  case 191:
#line 821 "gc_cl.y"
    { yyval.attr = clParseSimpleAttribute(Compiler, &yyvsp[0].token, clvATTR_PACKED, yyvsp[-1].attr); ;}
    break;

  case 192:
#line 823 "gc_cl.y"
    { yyval.attr = clParseAttributeVecTypeHint(Compiler, yyvsp[-4].attr, &yyvsp[-1].token); ;}
    break;

  case 193:
#line 825 "gc_cl.y"
    { yyval.attr = clParseAttributeReqdWorkGroupSize(Compiler, yyvsp[-8].attr, yyvsp[-5].expr, yyvsp[-3].expr, yyvsp[-1].expr); ;}
    break;

  case 194:
#line 827 "gc_cl.y"
    { yyval.attr = clParseAttributeWorkGroupSizeHint(Compiler, yyvsp[-8].attr, yyvsp[-5].expr, yyvsp[-3].expr, yyvsp[-1].expr); ;}
    break;

  case 195:
#line 829 "gc_cl.y"
    { yyval.attr = clParseAttributeKernelScaleHint(Compiler, yyvsp[-8].attr, yyvsp[-5].expr, yyvsp[-3].expr, yyvsp[-1].expr); ;}
    break;

  case 196:
#line 831 "gc_cl.y"
    { yyval.attr = clParseAttributeAligned(Compiler, yyvsp[-1].attr, gcvNULL); ;}
    break;

  case 197:
#line 833 "gc_cl.y"
    { yyval.attr = clParseAttributeAligned(Compiler, yyvsp[-4].attr, yyvsp[-1].expr); ;}
    break;

  case 198:
#line 835 "gc_cl.y"
    { yyval.attr = clParseSimpleAttribute(Compiler, &yyvsp[0].token, clvATTR_ALWAYS_INLINE, yyvsp[-1].attr); ;}
    break;

  case 199:
#line 840 "gc_cl.y"
    { yyval.decl = clParseCreateDecl(Compiler, &yyvsp[0].decl, gcvNULL, gcvNULL); ;}
    break;

  case 200:
#line 842 "gc_cl.y"
    { yyval.decl = clParseCreateDecl(Compiler, &yyvsp[-3].decl, gcvNULL, yyvsp[-1].expr); ;}
    break;

  case 201:
#line 844 "gc_cl.y"
    { yyval.decl = clParseCreateDecl(Compiler, &yyvsp[-1].decl, yyvsp[0].typeQualifierList, gcvNULL); ;}
    break;

  case 202:
#line 846 "gc_cl.y"
    { yyval.decl = clParseCreateDecl(Compiler, &yyvsp[-4].decl, yyvsp[-3].typeQualifierList, yyvsp[-1].expr); ;}
    break;

  case 203:
#line 848 "gc_cl.y"
    { yyval.decl = clParseCreateDecl(Compiler, &yyvsp[-5].decl, yyvsp[-4].typeQualifierList, yyvsp[-1].expr); ;}
    break;

  case 204:
#line 850 "gc_cl.y"
    { yyval.decl = clParseCreateDeclFromExpression(Compiler, yyvsp[0].expr); ;}
    break;

  case 205:
#line 855 "gc_cl.y"
    { yyval.decl = clParseQualifiedType(Compiler, gcvNULL, gcvFALSE, &yyvsp[0].decl); ;}
    break;

  case 206:
#line 857 "gc_cl.y"
    { yyval.decl = clParseQualifiedType(Compiler, yyvsp[-1].typeQualifierList, gcvFALSE, &yyvsp[0].decl); ;}
    break;

  case 207:
#line 859 "gc_cl.y"
    { yyval.decl = clParseQualifiedType(Compiler, yyvsp[0].typeQualifierList, gcvFALSE, &yyvsp[-1].decl); ;}
    break;

  case 208:
#line 861 "gc_cl.y"
    {
                    clsDECL decl;

                    decl = clParseQualifiedType(Compiler, yyvsp[-2].typeQualifierList, gcvFALSE, &yyvsp[-1].decl);
                    yyval.decl = clParseQualifiedType(Compiler, yyvsp[0].typeQualifierList, gcvFALSE, &decl);
                ;}
    break;

  case 209:
#line 868 "gc_cl.y"
    { yyval.decl = yyvsp[-1].decl; ;}
    break;

  case 210:
#line 873 "gc_cl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 211:
#line 875 "gc_cl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 212:
#line 877 "gc_cl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 213:
#line 879 "gc_cl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 214:
#line 881 "gc_cl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 215:
#line 883 "gc_cl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 216:
#line 885 "gc_cl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 217:
#line 887 "gc_cl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 218:
#line 889 "gc_cl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 219:
#line 891 "gc_cl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 220:
#line 896 "gc_cl.y"
    { yyval.decl = clParseNonStructType(Compiler, &yyvsp[0].token); ;}
    break;

  case 221:
#line 898 "gc_cl.y"
    { yyval.decl = clParseCreateDeclFromDataType(Compiler, yyvsp[0].dataType); ;}
    break;

  case 222:
#line 900 "gc_cl.y"
    { yyval.decl = clParseCreateDeclFromDataType(Compiler, yyvsp[0].dataType); ;}
    break;

  case 223:
#line 902 "gc_cl.y"
    { yyval.decl = clParseCreateDeclFromDataType(Compiler, yyvsp[0].dataType); ;}
    break;

  case 224:
#line 907 "gc_cl.y"
    { yyval.token = yyvsp[0].token;}
    break;

  case 225:
#line 909 "gc_cl.y"
    { yyval.token = yyvsp[0].token;}
    break;

  case 226:
#line 911 "gc_cl.y"
    { yyval.token = yyvsp[0].token;}
    break;

  case 227:
#line 913 "gc_cl.y"
    { yyval.token = yyvsp[0].token;}
    break;

  case 228:
#line 915 "gc_cl.y"
    { yyval.token = yyvsp[0].token;}
    break;

  case 229:
#line 917 "gc_cl.y"
    { yyval.token = yyvsp[0].token;}
    break;

  case 230:
#line 919 "gc_cl.y"
    { yyval.token = yyvsp[0].token;}
    break;

  case 231:
#line 921 "gc_cl.y"
    { yyval.token = yyvsp[0].token;}
    break;

  case 232:
#line 923 "gc_cl.y"
    { yyval.token = yyvsp[0].token;}
    break;

  case 233:
#line 925 "gc_cl.y"
    { yyval.token = yyvsp[0].token;}
    break;

  case 234:
#line 927 "gc_cl.y"
    { yyval.token = yyvsp[0].token;}
    break;

  case 235:
#line 929 "gc_cl.y"
    { yyval.token = yyvsp[0].token;}
    break;

  case 236:
#line 931 "gc_cl.y"
    { yyval.token = yyvsp[0].token;}
    break;

  case 237:
#line 933 "gc_cl.y"
    { yyval.token = yyvsp[0].token;}
    break;

  case 238:
#line 935 "gc_cl.y"
    { yyval.token = yyvsp[0].token;}
    break;

  case 239:
#line 937 "gc_cl.y"
    { yyval.token = yyvsp[0].token;}
    break;

  case 240:
#line 939 "gc_cl.y"
    { yyval.token = yyvsp[0].token;}
    break;

  case 241:
#line 941 "gc_cl.y"
    { yyval.token = yyvsp[0].token;}
    break;

  case 242:
#line 943 "gc_cl.y"
    { yyval.token = yyvsp[0].token;}
    break;

  case 243:
#line 945 "gc_cl.y"
    { yyval.token = yyvsp[0].token;}
    break;

  case 244:
#line 947 "gc_cl.y"
    { yyval.token = yyvsp[0].token;}
    break;

  case 245:
#line 949 "gc_cl.y"
    { yyval.token = yyvsp[0].token;}
    break;

  case 246:
#line 951 "gc_cl.y"
    { yyval.token = yyvsp[0].token;}
    break;

  case 247:
#line 953 "gc_cl.y"
    { yyval.token = yyvsp[0].token;}
    break;

  case 248:
#line 955 "gc_cl.y"
    { yyval.token = yyvsp[0].token;}
    break;

  case 249:
#line 957 "gc_cl.y"
    { yyval.token = yyvsp[0].token;}
    break;

  case 250:
#line 959 "gc_cl.y"
    { yyval.token = yyvsp[0].token;}
    break;

  case 251:
#line 963 "gc_cl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 252:
#line 965 "gc_cl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 253:
#line 970 "gc_cl.y"
    { clParseStructDeclBegin(Compiler, &yyvsp[-2].token,  &yyvsp[-1].token); ;}
    break;

  case 254:
#line 972 "gc_cl.y"
    { yyval.dataType = clParseStructDeclEnd(Compiler, &yyvsp[-5].token, yyvsp[0].attr, yyvsp[-2].status); ;}
    break;

  case 255:
#line 974 "gc_cl.y"
    { clParseStructDeclBegin(Compiler, &yyvsp[-1].token, gcvNULL); ;}
    break;

  case 256:
#line 976 "gc_cl.y"
    { yyval.dataType = clParseStructDeclEnd(Compiler, gcvNULL, yyvsp[0].attr, yyvsp[-2].status); ;}
    break;

  case 257:
#line 978 "gc_cl.y"
    { clParseStructDeclBegin(Compiler, &yyvsp[-3].token, &yyvsp[-1].token); ;}
    break;

  case 258:
#line 980 "gc_cl.y"
    { yyval.dataType = clParseStructDeclEnd(Compiler, &yyvsp[-4].token, yyvsp[-5].attr, yyvsp[-1].status); ;}
    break;

  case 259:
#line 982 "gc_cl.y"
    { clParseStructDeclBegin(Compiler, &yyvsp[-2].token, gcvNULL); ;}
    break;

  case 260:
#line 984 "gc_cl.y"
    { yyval.dataType = clParseStructDeclEnd(Compiler, gcvNULL, yyvsp[-4].attr, yyvsp[-1].status); ;}
    break;

  case 261:
#line 989 "gc_cl.y"
    {
           yyval.status = yyvsp[0].status;
        ;}
    break;

  case 262:
#line 993 "gc_cl.y"
    {
           if(gcmIS_ERROR(yyvsp[-1].status)) {
                       yyval.status = yyvsp[-1].status;
           }
           else {
                       yyval.status = yyvsp[0].status;

           }
        ;}
    break;

  case 263:
#line 1006 "gc_cl.y"
    {
                   yyval.status = clParseTypeSpecifiedFieldDeclList(Compiler, &yyvsp[-2].decl, yyvsp[-1].fieldDeclList);
        ;}
    break;

  case 264:
#line 1013 "gc_cl.y"
    { yyval.fieldDeclList = clParseFieldDeclList(Compiler, yyvsp[0].fieldDecl); ;}
    break;

  case 265:
#line 1015 "gc_cl.y"
    { yyval.fieldDeclList = clParseFieldDeclList2(Compiler, yyvsp[-2].fieldDeclList, yyvsp[0].fieldDecl); ;}
    break;

  case 266:
#line 1019 "gc_cl.y"
    { yyval.fieldDecl = clParseFieldDecl(Compiler, gcvNULL, gcvNULL, gcvNULL); ;}
    break;

  case 267:
#line 1021 "gc_cl.y"
    { yyval.fieldDecl = clParseFieldDecl(Compiler, &yyvsp[-1].token, gcvNULL, yyvsp[0].attr); ;}
    break;

  case 268:
#line 1023 "gc_cl.y"
    { yyval.fieldDecl = clParseFieldDecl(Compiler, &yyvsp[-2].token, yyvsp[-1].expr, yyvsp[0].attr); ;}
    break;

  case 269:
#line 1028 "gc_cl.y"
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

  case 270:
#line 1050 "gc_cl.y"
    { yyval.expr = yyvsp[0].expr; ;}
    break;

  case 271:
#line 1052 "gc_cl.y"
    { yyval.expr = yyvsp[-1].expr; ;}
    break;

  case 272:
#line 1054 "gc_cl.y"
    { yyval.expr = yyvsp[-1].expr; ;}
    break;

  case 273:
#line 1059 "gc_cl.y"
    { yyval.expr = clParseInitializerList(Compiler, yyvsp[-2].expr, yyvsp[-1].declOrDeclList, yyvsp[0].expr); ;}
    break;

  case 274:
#line 1061 "gc_cl.y"
    { yyval.expr = clParseInitializerList(Compiler, yyvsp[-3].expr, yyvsp[-1].declOrDeclList, yyvsp[0].expr); ;}
    break;

  case 275:
#line 1065 "gc_cl.y"
    { yyval.declOrDeclList = gcvNULL; ;}
    break;

  case 276:
#line 1067 "gc_cl.y"
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

  case 277:
#line 1080 "gc_cl.y"
    { yyval.declOrDeclList = yyvsp[0].declOrDeclList; ;}
    break;

  case 278:
#line 1082 "gc_cl.y"
    {
           yyval.token.type = T_EOF;
        ;}
    break;

  case 279:
#line 1086 "gc_cl.y"
    { yyval.declOrDeclList = yyvsp[0].declOrDeclList; ;}
    break;

  case 280:
#line 1091 "gc_cl.y"
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

  case 281:
#line 1104 "gc_cl.y"
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

  case 282:
#line 1120 "gc_cl.y"
    { yyval.statement = yyvsp[0].statement; ;}
    break;

  case 283:
#line 1125 "gc_cl.y"
    { yyval.statement = clParseCompoundStatementAsStatement(Compiler, yyvsp[0].statements); ;}
    break;

  case 284:
#line 1127 "gc_cl.y"
    { yyval.statement = yyvsp[0].statement; ;}
    break;

  case 285:
#line 1132 "gc_cl.y"
    { yyval.statement = yyvsp[0].statement; ;}
    break;

  case 286:
#line 1134 "gc_cl.y"
    { yyval.statement = yyvsp[0].statement; ;}
    break;

  case 287:
#line 1136 "gc_cl.y"
    { yyval.statement = yyvsp[0].statement; ;}
    break;

  case 288:
#line 1138 "gc_cl.y"
    { yyval.statement = yyvsp[0].statement; ;}
    break;

  case 289:
#line 1140 "gc_cl.y"
    { yyval.statement = yyvsp[0].statement; ;}
    break;

  case 290:
#line 1142 "gc_cl.y"
    { yyval.statement = clParseStatementLabel(Compiler, &yyvsp[-1].token); ;}
    break;

  case 291:
#line 1144 "gc_cl.y"
    { yyclearin;
          yyerrok;
          yyval.statement = gcvNULL; ;}
    break;

  case 292:
#line 1148 "gc_cl.y"
    { yyclearin;
          yyerrok;
          yyval.statement = gcvNULL; ;}
    break;

  case 293:
#line 1155 "gc_cl.y"
    { yyval.statements = gcvNULL; ;}
    break;

  case 294:
#line 1157 "gc_cl.y"
    { clParseCompoundStatementBegin(Compiler); ;}
    break;

  case 295:
#line 1159 "gc_cl.y"
    { yyval.statements = clParseCompoundStatementEnd(Compiler, &yyvsp[-3].token, yyvsp[-1].statements, &yyvsp[0].token); ;}
    break;

  case 296:
#line 1164 "gc_cl.y"
    { yyval.statement = clParseCompoundStatementNoNewScopeAsStatementNoNewScope(Compiler, yyvsp[0].statements); ;}
    break;

  case 297:
#line 1166 "gc_cl.y"
    { yyval.statement = yyvsp[0].statement; ;}
    break;

  case 298:
#line 1171 "gc_cl.y"
    { yyval.statements = gcvNULL; ;}
    break;

  case 299:
#line 1173 "gc_cl.y"
    { clParseCompoundStatementNoNewScopeBegin(Compiler); ;}
    break;

  case 300:
#line 1175 "gc_cl.y"
    { yyval.statements = clParseCompoundStatementNoNewScopeEnd(Compiler, &yyvsp[-3].token, yyvsp[-1].statements, &yyvsp[0].token); ;}
    break;

  case 301:
#line 1180 "gc_cl.y"
    { yyval.statements = clParseStatementList(Compiler, yyvsp[0].statement); ;}
    break;

  case 302:
#line 1182 "gc_cl.y"
    { yyval.statements = clParseStatementList2(Compiler, yyvsp[-1].statements, yyvsp[0].statement); ;}
    break;

  case 303:
#line 1187 "gc_cl.y"
    { yyval.statement = gcvNULL; ;}
    break;

  case 304:
#line 1189 "gc_cl.y"
    { yyval.statement = clParseExprAsStatement(Compiler, yyvsp[-1].expr); ;}
    break;

  case 305:
#line 1194 "gc_cl.y"
    { yyval.statement = clParseIfStatement(Compiler, &yyvsp[-4].token, yyvsp[-2].expr, yyvsp[0].ifStatementPair); ;}
    break;

  case 306:
#line 1196 "gc_cl.y"
    { yyval.statement = clParseSwitchStatement(Compiler, &yyvsp[-4].token, yyvsp[-2].expr, yyvsp[0].statement); ;}
    break;

  case 307:
#line 1201 "gc_cl.y"
    { yyval.statements = clParseStatementList(Compiler, yyvsp[0].statement); ;}
    break;

  case 308:
#line 1203 "gc_cl.y"
    { yyval.statements = clParseStatementList2(Compiler, yyvsp[-1].statements, yyvsp[0].statement); ;}
    break;

  case 309:
#line 1208 "gc_cl.y"
    { yyval.statement = yyvsp[0].statement; ;}
    break;

  case 310:
#line 1210 "gc_cl.y"
    { yyval.statement = clParseCaseStatement(Compiler, &yyvsp[-2].token, yyvsp[-1].expr); ;}
    break;

  case 311:
#line 1212 "gc_cl.y"
    { yyval.statement = clParseDefaultStatement(Compiler, &yyvsp[-1].token); ;}
    break;

  case 312:
#line 1217 "gc_cl.y"
    { yyval.statement = gcvNULL; ;}
    break;

  case 313:
#line 1219 "gc_cl.y"
    { clParseSwitchBodyBegin(Compiler); ;}
    break;

  case 314:
#line 1221 "gc_cl.y"
    { yyval.statement = clParseSwitchBodyEnd(Compiler, &yyvsp[-3].token, yyvsp[-1].statements, &yyvsp[0].token); ;}
    break;

  case 315:
#line 1226 "gc_cl.y"
    { yyval.ifStatementPair = clParseIfSubStatements(Compiler, yyvsp[-2].statement, yyvsp[0].statement); ;}
    break;

  case 316:
#line 1228 "gc_cl.y"
    { yyval.ifStatementPair = clParseIfSubStatements(Compiler, yyvsp[0].statement, gcvNULL); ;}
    break;

  case 317:
#line 1235 "gc_cl.y"
    { clParseWhileStatementBegin(Compiler); ;}
    break;

  case 318:
#line 1237 "gc_cl.y"
    { yyval.statement = clParseWhileStatementEnd(Compiler, &yyvsp[-5].token, yyvsp[-2].expr, yyvsp[0].statement); ;}
    break;

  case 319:
#line 1239 "gc_cl.y"
    { yyval.statement = clParseDoWhileStatement(Compiler, &yyvsp[-6].token, yyvsp[-5].statement, yyvsp[-2].expr); ;}
    break;

  case 320:
#line 1241 "gc_cl.y"
    { clParseForStatementBegin(Compiler); ;}
    break;

  case 321:
#line 1243 "gc_cl.y"
    { yyval.statement = clParseForStatementEnd(Compiler, &yyvsp[-4].token, yyvsp[-2].statement, yyvsp[-1].forExprPair, yyvsp[0].statement); ;}
    break;

  case 322:
#line 1248 "gc_cl.y"
    { yyval.statement = yyvsp[0].statement; ;}
    break;

  case 323:
#line 1250 "gc_cl.y"
    { yyval.statement = yyvsp[0].statement; ;}
    break;

  case 324:
#line 1252 "gc_cl.y"
    { yyclearin;
          yyerrok;
          yyval.statement = gcvNULL; ;}
    break;

  case 325:
#line 1259 "gc_cl.y"
    { yyval.expr = yyvsp[0].expr; ;}
    break;

  case 326:
#line 1261 "gc_cl.y"
    { yyval.expr = gcvNULL; ;}
    break;

  case 327:
#line 1266 "gc_cl.y"
    { yyval.forExprPair = clParseForControl(Compiler, yyvsp[-2].expr, gcvNULL); ;}
    break;

  case 328:
#line 1268 "gc_cl.y"
    { yyval.forExprPair = clParseForControl(Compiler, yyvsp[-3].expr, yyvsp[-1].expr); ;}
    break;

  case 329:
#line 1270 "gc_cl.y"
    {
          clsForExprPair nullPair = {gcvNULL, gcvNULL};
          yyclearin;
          yyerrok;
          yyval.forExprPair = nullPair; ;}
    break;

  case 330:
#line 1276 "gc_cl.y"
    {
          clsForExprPair nullPair = {gcvNULL, gcvNULL};
          yyclearin;
          yyerrok;
          yyval.forExprPair = nullPair; ;}
    break;

  case 331:
#line 1285 "gc_cl.y"
    { yyval.statement = clParseJumpStatement(Compiler, clvCONTINUE, &yyvsp[-1].token, gcvNULL); ;}
    break;

  case 332:
#line 1287 "gc_cl.y"
    { yyval.statement = clParseJumpStatement(Compiler, clvBREAK, &yyvsp[-1].token, gcvNULL); ;}
    break;

  case 333:
#line 1289 "gc_cl.y"
    { yyval.statement = clParseJumpStatement(Compiler, clvRETURN, &yyvsp[-1].token, gcvNULL); ;}
    break;

  case 334:
#line 1291 "gc_cl.y"
    { yyval.statement = clParseJumpStatement(Compiler, clvRETURN, &yyvsp[-2].token, yyvsp[-1].expr); ;}
    break;

  case 335:
#line 1293 "gc_cl.y"
    { yyval.statement = clParseGotoStatement(Compiler, &yyvsp[-2].token, &yyvsp[-1].token); ;}
    break;

  case 339:
#line 1305 "gc_cl.y"
    { clParseExternalDecl(Compiler, yyvsp[0].statement); ;}
    break;

  case 341:
#line 1311 "gc_cl.y"
    { clParseFuncDef(Compiler, yyvsp[-1].funcName, yyvsp[0].statements); ;}
    break;


    }

/* Line 1000 of yacc.c.  */
#line 4613 "gc_cl_parser.c"

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
  yyerrstatus = 3;    /* Each real token shifted decrements this.  */

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


#line 1314 "gc_cl.y"



