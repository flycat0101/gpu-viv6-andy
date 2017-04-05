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
     T_KERNEL_SCALE_HINT = 481,
     T_ALWAYS_INLINE = 482,
     T_UNSIGNED = 483,
     T_STRUCT = 484,
     T_UNION = 485,
     T_TYPEDEF = 486,
     T_ENUM = 487,
     T_INLINE = 488,
     T_SIZEOF = 489,
     T_TYPE_CAST = 490,
     T_VEC_STEP = 491,
     T_TYPEOF = 492,
     T_VERY_LAST_TERMINAL = 493
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
#define T_KERNEL_SCALE_HINT 481
#define T_ALWAYS_INLINE 482
#define T_UNSIGNED 483
#define T_STRUCT 484
#define T_UNION 485
#define T_TYPEDEF 486
#define T_ENUM 487
#define T_INLINE 488
#define T_SIZEOF 489
#define T_TYPE_CAST 490
#define T_VEC_STEP 491
#define T_TYPEOF 492
#define T_VERY_LAST_TERMINAL 493




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
} YYSTYPE;
/* Line 191 of yacc.c.  */
#line 592 "gc_cl_parser.c"
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif



/* Copy the second part of user declarations.  */


/* Line 214 of yacc.c.  */
#line 604 "gc_cl_parser.c"

#if !defined (yyoverflow) || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

#if YYSTACK_USE_ALLOCA
#  define YYSTACK_ALLOC alloca
# else
#ifndef YYSTACK_USE_ALLOCA
#if defined (alloca) || defined (_ALLOCA_H)
#    define YYSTACK_ALLOC alloca
#   else
#ifdef __GNUC__
#     define YYSTACK_ALLOC __builtin_alloca
#    endif
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
     || (YYSTYPE_IS_TRIVIAL)))

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
#if 1 < __GNUC__
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
#define YYLAST   3526

/* YYNTOKENS -- Number of terminals. */
#define YYNTOKENS  263
/* YYNNTS -- Number of nonterminals. */
#define YYNNTS  111
/* YYNRULES -- Number of rules. */
#define YYNRULES  324
/* YYNRULES -- Number of states. */
#define YYNSTATES  558

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   493

#define YYTRANSLATE(YYX)                         \
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const unsigned short yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   250,     2,     2,     2,   256,   261,     2,
     239,   240,   254,   253,   246,   251,   245,   255,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,   247,   249,
     257,   248,   258,   262,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,   241,     2,   242,   260,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   243,   259,   244,   252,     2,     2,     2,
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
     235,   236,   237,   238
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
     372,   378,   384,   391,   395,   400,   405,   411,   414,   418,
     420,   423,   425,   428,   431,   435,   438,   442,   444,   446,
     448,   453,   455,   458,   460,   463,   466,   470,   472,   475,
     477,   481,   482,   484,   488,   493,   495,   496,   500,   505,
     511,   512,   520,   521,   530,   532,   536,   541,   542,   549,
     550,   558,   559,   567,   568,   570,   571,   573,   574,   579,
     584,   586,   591,   600,   609,   618,   620,   625,   627,   629,
     634,   637,   643,   651,   653,   655,   658,   663,   665,   667,
     669,   671,   673,   675,   677,   679,   681,   683,   685,   687,
     689,   691,   693,   695,   697,   699,   701,   703,   705,   707,
     709,   711,   713,   715,   717,   719,   721,   723,   725,   727,
     729,   731,   733,   735,   737,   739,   741,   743,   744,   752,
     753,   760,   761,   769,   770,   777,   779,   782,   786,   788,
     792,   795,   799,   801,   803,   807,   811,   814,   819,   820,
     823,   825,   826,   830,   834,   837,   839,   841,   843,   845,
     847,   849,   851,   853,   856,   859,   862,   865,   866,   871,
     873,   875,   878,   879,   884,   886,   889,   891,   894,   900,
     906,   908,   911,   913,   917,   920,   923,   924,   929,   933,
     935,   936,   943,   951,   952,   958,   961,   964,   968,   970,
     971,   975,   980,   984,   989,   992,   995,   998,  1002,  1006,
    1007,  1010,  1012,  1014,  1016
};

/* YYRHS -- A `-1'-separated list of the rules' RHS. */
static const short yyrhs[] =
{
     371,     0,    -1,   162,    -1,   169,    -1,   265,   169,    -1,
     264,    -1,   165,    -1,   166,    -1,   164,    -1,   167,    -1,
     168,    -1,   265,    -1,   239,   292,   240,    -1,   266,    -1,
     267,   241,   268,   242,    -1,   269,    -1,   267,   245,   170,
      -1,   267,   192,   170,    -1,   267,   173,    -1,   267,   174,
      -1,   292,    -1,   270,   240,    -1,   272,     4,   240,    -1,
     272,   240,    -1,   272,   290,    -1,   270,   246,   290,    -1,
     239,   327,   240,    -1,   239,   327,   308,   240,    -1,   162,
     239,    -1,   271,   243,    -1,   276,    -1,    -1,   271,   275,
     274,    -1,   273,   292,   244,    -1,   273,   292,   193,    -1,
     267,    -1,   173,   276,    -1,   174,   276,    -1,   236,   239,
     276,   240,    -1,   236,   271,    -1,   234,   276,    -1,   234,
     271,    -1,   277,   274,    -1,   253,    -1,   251,    -1,   250,
      -1,   252,    -1,   261,    -1,   254,    -1,   274,    -1,   278,
     254,   274,    -1,   278,   255,   274,    -1,   278,   256,   274,
      -1,   278,    -1,   279,   253,   278,    -1,   279,   251,   278,
      -1,   279,    -1,   280,   171,   279,    -1,   280,   172,   279,
      -1,   280,    -1,   281,   257,   280,    -1,   281,   258,   280,
      -1,   281,   175,   280,    -1,   281,   176,   280,    -1,   281,
      -1,   282,   177,   281,    -1,   282,   178,   281,    -1,   282,
      -1,   283,   261,   282,    -1,   283,    -1,   284,   260,   283,
      -1,   284,    -1,   285,   259,   284,    -1,   285,    -1,   286,
     179,   285,    -1,   286,    -1,   287,   181,   286,    -1,   287,
      -1,   288,   180,   287,    -1,   288,    -1,   288,   262,   292,
     247,   290,    -1,   289,    -1,   276,   291,   290,    -1,   248,
      -1,   182,    -1,   183,    -1,   185,    -1,   184,    -1,   191,
      -1,   186,    -1,   187,    -1,   188,    -1,   189,    -1,   190,
      -1,   290,    -1,   292,   246,   290,    -1,   289,    -1,   331,
     162,    -1,   232,   162,    -1,   232,   320,   162,   243,   296,
     244,    -1,   232,   320,   162,   243,   296,   193,    -1,   232,
     162,   243,   296,   244,    -1,   232,   162,   243,   296,   193,
      -1,   232,   320,   243,   296,   244,    -1,   232,   320,   243,
     296,   193,    -1,   232,   243,   296,   244,    -1,   232,   243,
     296,   193,    -1,   297,    -1,   296,   246,   297,    -1,   162,
      -1,   162,   248,   293,    -1,   313,   249,    -1,   295,   322,
     249,    -1,   294,   249,    -1,   332,   249,    -1,   300,   240,
      -1,   302,    -1,   301,    -1,   302,   304,    -1,   301,   246,
     304,    -1,   320,   215,   327,   310,   239,    -1,   215,   322,
     327,   310,   239,    -1,   210,   215,   322,   327,   310,   239,
      -1,   327,   310,   239,    -1,   320,   327,   310,   239,    -1,
     233,   327,   310,   239,    -1,   209,   233,   327,   310,   239,
      -1,   329,   310,    -1,   329,   310,   312,    -1,   303,    -1,
     305,   303,    -1,   306,    -1,   305,   306,    -1,   307,   303,
      -1,   307,   305,   303,    -1,   307,   306,    -1,   307,   305,
     306,    -1,   217,    -1,   218,    -1,   329,    -1,   329,   241,
     293,   242,    -1,   328,    -1,   328,   307,    -1,   254,    -1,
     254,   308,    -1,   254,   307,    -1,   254,   307,   308,    -1,
     162,    -1,   308,   162,    -1,   309,    -1,   239,   309,   240,
      -1,    -1,   293,    -1,   241,   311,   242,    -1,   312,   241,
     311,   242,    -1,   317,    -1,    -1,   231,   314,   317,    -1,
     313,   246,   310,   322,    -1,   313,   246,   310,   312,   322,
      -1,    -1,   313,   246,   310,   322,   248,   315,   342,    -1,
      -1,   313,   246,   310,   312,   322,   248,   316,   342,    -1,
     299,    -1,   327,   310,   322,    -1,   327,   310,   312,   322,
      -1,    -1,   327,   310,   322,   248,   318,   342,    -1,    -1,
     327,   310,   312,   322,   248,   319,   342,    -1,    -1,   223,
     239,   239,   321,   323,   240,   240,    -1,    -1,   320,    -1,
      -1,   325,    -1,    -1,   323,   246,   324,   325,    -1,   221,
     239,   162,   240,    -1,   219,    -1,   222,   239,   159,   240,
      -1,   224,   239,   293,   246,   293,   246,   293,   240,    -1,
     225,   239,   293,   246,   293,   246,   293,   240,    -1,   226,
     239,   293,   246,   293,   246,   293,   240,    -1,   220,    -1,
     220,   239,   293,   240,    -1,   227,    -1,   327,    -1,   327,
     241,   293,   242,    -1,   327,   308,    -1,   327,   308,   241,
     293,   242,    -1,   239,   327,   308,   240,   241,   293,   242,
      -1,   292,    -1,   329,    -1,   307,   329,    -1,   237,   239,
     326,   240,    -1,   206,    -1,   207,    -1,   208,    -1,   211,
      -1,   212,    -1,   213,    -1,   214,    -1,   209,    -1,   210,
      -1,   216,    -1,   330,    -1,   332,    -1,   294,    -1,   295,
      -1,     4,    -1,    24,    -1,    70,    -1,    10,    -1,   157,
      -1,   158,    -1,   159,    -1,   160,    -1,   161,    -1,    95,
      -1,    96,    -1,    97,    -1,    98,    -1,    99,    -1,   101,
      -1,   102,    -1,   100,    -1,    94,    -1,   105,    -1,   106,
      -1,   107,    -1,   103,    -1,   104,    -1,   163,    -1,   229,
      -1,   230,    -1,    -1,   331,   162,   243,   333,   337,   244,
     322,    -1,    -1,   331,   243,   334,   337,   244,   322,    -1,
      -1,   331,   320,   162,   243,   335,   337,   244,    -1,    -1,
     331,   320,   243,   336,   337,   244,    -1,   338,    -1,   337,
     338,    -1,   327,   339,   249,    -1,   340,    -1,   339,   246,
     340,    -1,   310,   322,    -1,   310,   312,   322,    -1,   243,
      -1,   290,    -1,   341,   343,   244,    -1,   341,   343,   193,
      -1,   344,   342,    -1,   343,   246,   344,   342,    -1,    -1,
     345,   248,    -1,   347,    -1,    -1,   345,   346,   347,    -1,
     241,   293,   242,    -1,   245,   170,    -1,   298,    -1,   351,
      -1,   350,    -1,   348,    -1,   357,    -1,   358,    -1,   364,
      -1,   370,    -1,   162,   247,    -1,     1,   249,    -1,     1,
     244,    -1,   243,   244,    -1,    -1,   243,   352,   356,   244,
      -1,   354,    -1,   350,    -1,   243,   244,    -1,    -1,   243,
     355,   356,   244,    -1,   349,    -1,   356,   349,    -1,   249,
      -1,   292,   249,    -1,   202,   239,   292,   240,   363,    -1,
     203,   239,   268,   240,   361,    -1,   360,    -1,   359,   360,
      -1,   349,    -1,   204,   293,   247,    -1,   205,   247,    -1,
     243,   244,    -1,    -1,   243,   362,   359,   244,    -1,   349,
     201,   349,    -1,   349,    -1,    -1,   198,   365,   239,   292,
     240,   353,    -1,   200,   349,   198,   239,   292,   240,   249,
      -1,    -1,   199,   366,   367,   369,   353,    -1,   239,   357,
      -1,   239,   348,    -1,   239,     1,   249,    -1,   292,    -1,
      -1,   368,   249,   240,    -1,   368,   249,   292,   240,    -1,
       1,   249,   240,    -1,     1,   249,   292,   240,    -1,   195,
     249,    -1,   194,   249,    -1,   196,   249,    -1,   196,   292,
     249,    -1,   197,   162,   249,    -1,    -1,   371,   372,    -1,
     373,    -1,   298,    -1,   249,    -1,   299,   354,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const unsigned short yyrline[] =
{
       0,   191,   191,   200,   202,   207,   209,   211,   213,   215,
     217,   219,   221,   226,   228,   230,   232,   234,   236,   238,
     243,   248,   250,   252,   257,   259,   264,   266,   271,   276,
     285,   288,   287,   296,   300,   307,   309,   311,   313,   315,
     317,   319,   321,   326,   328,   330,   332,   334,   336,   341,
     345,   347,   349,   354,   356,   358,   363,   365,   367,   372,
     374,   376,   378,   380,   385,   387,   389,   394,   396,   401,
     403,   408,   410,   415,   417,   422,   424,   429,   431,   436,
     438,   443,   445,   450,   452,   454,   456,   458,   460,   462,
     464,   466,   468,   470,   475,   477,   490,   495,   497,   502,
     506,   509,   513,   516,   520,   523,   527,   533,   540,   545,
     547,   552,   554,   556,   558,   563,   568,   570,   575,   577,
     582,   584,   586,   588,   590,   592,   596,   603,   605,   610,
     612,   614,   616,   618,   620,   622,   624,   629,   631,   636,
     638,   643,   648,   653,   655,   657,   659,   663,   665,   672,
     674,   679,   680,   685,   687,   692,   695,   694,   703,   705,
     708,   707,   712,   711,   719,   721,   723,   726,   725,   730,
     729,   739,   738,   745,   746,   751,   752,   755,   754,   761,
     763,   765,   767,   769,   771,   773,   775,   777,   782,   784,
     786,   788,   790,   792,   797,   799,   801,   806,   808,   810,
     812,   814,   816,   818,   820,   822,   824,   829,   831,   833,
     835,   840,   842,   844,   846,   848,   850,   852,   854,   856,
     858,   860,   862,   864,   866,   868,   870,   872,   874,   876,
     878,   880,   882,   884,   886,   890,   892,   898,   897,   902,
     901,   906,   905,   910,   909,   916,   920,   933,   940,   942,
     947,   949,   954,   976,   978,   980,   985,   987,   992,   993,
    1006,  1009,  1008,  1017,  1030,  1046,  1051,  1053,  1058,  1060,
    1062,  1064,  1066,  1068,  1070,  1074,  1081,  1084,  1083,  1090,
    1092,  1097,  1100,  1099,  1106,  1108,  1113,  1115,  1120,  1122,
    1127,  1129,  1134,  1136,  1138,  1143,  1146,  1145,  1152,  1154,
    1162,  1161,  1165,  1168,  1167,  1174,  1176,  1178,  1185,  1188,
    1192,  1194,  1196,  1202,  1211,  1213,  1215,  1217,  1219,  1225,
    1226,  1230,  1231,  1233,  1237
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
  "T_USHORT32", "T_INT", "T_INT2", "T_INT3", "T_INT4", "T_INT8",
  "T_INT16", "T_UINT", "T_UINT2", "T_UINT3", "T_UINT4", "T_UINT8",
  "T_UINT16", "T_LONG", "T_LONG2", "T_LONG3", "T_LONG4", "T_LONG8",
  "T_LONG16", "T_ULONG", "T_ULONG2", "T_ULONG3", "T_ULONG4", "T_ULONG8",
  "T_ULONG16", "T_SAMPLER_T", "T_IMAGE1D_T", "T_IMAGE1D_ARRAY_T",
  "T_IMAGE1D_BUFFER_T", "T_IMAGE2D_ARRAY_T", "T_IMAGE2D_T", "T_IMAGE3D_T",
  "T_IMAGE2D_PTR_T", "T_IMAGE2D_DYNAMIC_ARRAY_T", "T_SIZE_T", "T_EVENT_T",
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
  "T_CHARCONSTANT", "T_STRING_LITERAL", "T_FIELD_SELECTION",
  "T_LSHIFT_OP", "T_RSHIFT_OP", "T_INC_OP", "T_DEC_OP", "T_LE_OP",
  "T_GE_OP", "T_EQ_OP", "T_NE_OP", "T_AND_OP", "T_OR_OP", "T_XOR_OP",
  "T_MUL_ASSIGN", "T_DIV_ASSIGN", "T_ADD_ASSIGN", "T_MOD_ASSIGN",
  "T_LEFT_ASSIGN", "T_RIGHT_ASSIGN", "T_AND_ASSIGN", "T_XOR_ASSIGN",
  "T_OR_ASSIGN", "T_SUB_ASSIGN", "T_STRUCT_UNION_PTR",
  "T_INITIALIZER_END", "T_BREAK", "T_CONTINUE", "T_RETURN", "T_GOTO",
  "T_WHILE", "T_FOR", "T_DO", "T_ELSE", "T_IF", "T_SWITCH", "T_CASE",
  "T_DEFAULT", "T_CONST", "T_RESTRICT", "T_VOLATILE", "T_STATIC",
  "T_EXTERN", "T_CONSTANT", "T_GLOBAL", "T_LOCAL", "T_PRIVATE",
  "T_KERNEL", "T_UNIFORM", "T_READ_ONLY", "T_WRITE_ONLY", "T_PACKED",
  "T_ALIGNED", "T_ENDIAN", "T_VEC_TYPE_HINT", "T_ATTRIBUTE__",
  "T_REQD_WORK_GROUP_SIZE", "T_WORK_GROUP_SIZE_HINT",
  "T_KERNEL_SCALE_HINT", "T_ALWAYS_INLINE", "T_UNSIGNED", "T_STRUCT",
  "T_UNION", "T_TYPEDEF", "T_ENUM", "T_INLINE", "T_SIZEOF", "T_TYPE_CAST",
  "T_VEC_STEP", "T_TYPEOF", "T_VERY_LAST_TERMINAL", "'('", "')'", "'['",
  "']'", "'{'", "'}'", "'.'", "','", "':'", "'='", "';'", "'!'", "'-'",
  "'~'", "'+'", "'*'", "'/'", "'%'", "'<'", "'>'", "'|'", "'^'", "'&'",
  "'?'", "$accept", "variable_identifier", "string_literal",
  "primary_expression", "postfix_expression", "integer_expression",
  "function_call", "function_call_with_parameters", "type_cast",
  "function_call_header", "curly_bracket_type_cast", "cast_expression",
  "@1", "unary_expression", "unary_operator", "multiplicative_expression",
  "additive_expression", "shift_expression", "relational_expression",
  "equality_expression", "and_expression", "exclusive_or_expression",
  "inclusive_or_expression", "logical_and_expression",
  "logical_xor_expression", "logical_or_expression",
  "conditional_expression", "assignment_expression",
  "assignment_operator", "expression", "constant_expression", "tags",
  "enum_specifier", "enumerator_list", "enumerator", "declaration",
  "function_prototype", "function_declarator",
  "function_header_with_parameters", "function_header",
  "parameter_declarator", "parameter_declaration", "parameter_qualifier",
  "parameter_type_specifier", "type_qualifier_list", "pointer",
  "declarator", "direct_declarator", "array_size", "array_declarator",
  "init_declarator_list", "@2", "@3", "@4", "single_declaration", "@5",
  "@6", "attribute_specifier", "@7", "attribute_specifier_opt",
  "attribute_list", "@8", "attribute", "typeof_type_specifier",
  "fully_specified_type", "type_qualifier", "type_specifier", "type_name",
  "struct_or_union", "struct_union_specifier", "@9", "@10", "@11", "@12",
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
     485,   486,   487,   488,   489,   490,   491,   492,   493,    40,
      41,    91,    93,   123,   125,    46,    44,    58,    61,    59,
      33,    45,   126,    43,    42,    47,    37,    60,    62,   124,
      94,    38,    63
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const unsigned short yyr1[] =
{
       0,   263,   264,   265,   265,   266,   266,   266,   266,   266,
     266,   266,   266,   267,   267,   267,   267,   267,   267,   267,
     268,   269,   269,   269,   270,   270,   271,   271,   272,   273,
     274,   275,   274,   274,   274,   276,   276,   276,   276,   276,
     276,   276,   276,   277,   277,   277,   277,   277,   277,   278,
     278,   278,   278,   279,   279,   279,   280,   280,   280,   281,
     281,   281,   281,   281,   282,   282,   282,   283,   283,   284,
     284,   285,   285,   286,   286,   287,   287,   288,   288,   289,
     289,   290,   290,   291,   291,   291,   291,   291,   291,   291,
     291,   291,   291,   291,   292,   292,   293,   294,   294,   295,
     295,   295,   295,   295,   295,   295,   295,   296,   296,   297,
     297,   298,   298,   298,   298,   299,   300,   300,   301,   301,
     302,   302,   302,   302,   302,   302,   302,   303,   303,   304,
     304,   304,   304,   304,   304,   304,   304,   305,   305,   306,
     306,   307,   307,   308,   308,   308,   308,   309,   309,   310,
     310,   311,   311,   312,   312,   313,   314,   313,   313,   313,
     315,   313,   316,   313,   317,   317,   317,   318,   317,   319,
     317,   321,   320,   322,   322,   323,   323,   324,   323,   325,
     325,   325,   325,   325,   325,   325,   325,   325,   326,   326,
     326,   326,   326,   326,   327,   327,   327,   328,   328,   328,
     328,   328,   328,   328,   328,   328,   328,   329,   329,   329,
     329,   330,   330,   330,   330,   330,   330,   330,   330,   330,
     330,   330,   330,   330,   330,   330,   330,   330,   330,   330,
     330,   330,   330,   330,   330,   331,   331,   333,   332,   334,
     332,   335,   332,   336,   332,   337,   337,   338,   339,   339,
     340,   340,   341,   342,   342,   342,   343,   343,   344,   344,
     345,   346,   345,   347,   347,   348,   349,   349,   350,   350,
     350,   350,   350,   350,   350,   350,   351,   352,   351,   353,
     353,   354,   355,   354,   356,   356,   357,   357,   358,   358,
     359,   359,   360,   360,   360,   361,   362,   361,   363,   363,
     365,   364,   364,   366,   364,   367,   367,   367,   368,   368,
     369,   369,   369,   369,   370,   370,   370,   370,   370,   371,
     371,   372,   372,   372,   373
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
       5,     5,     6,     3,     4,     4,     5,     2,     3,     1,
       2,     1,     2,     2,     3,     2,     3,     1,     1,     1,
       4,     1,     2,     1,     2,     2,     3,     1,     2,     1,
       3,     0,     1,     3,     4,     1,     0,     3,     4,     5,
       0,     7,     0,     8,     1,     3,     4,     0,     6,     0,
       7,     0,     7,     0,     1,     0,     1,     0,     4,     4,
       1,     4,     8,     8,     8,     1,     4,     1,     1,     4,
       2,     5,     7,     1,     1,     2,     4,     1,     1,     1,
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
       3,     4,     3,     4,     2,     2,     2,     3,     3,     0,
       2,     1,     1,     1,     2
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const unsigned short yydefact[] =
{
     319,     0,     1,   211,   214,   212,   213,   228,   220,   221,
     222,   223,   224,   227,   225,   226,   232,   233,   229,   230,
     231,   215,   216,   217,   218,   219,   234,   197,   198,   199,
     204,   205,   200,   201,   202,   203,   173,   206,     0,   235,
     236,   156,     0,     0,     0,   323,   209,   210,   322,   164,
       0,   117,   116,     0,     0,   155,     0,     0,   141,   194,
     207,     0,   208,   320,   321,     0,   173,   174,     0,     0,
       0,    98,     0,     0,   204,   205,   209,   210,     0,   208,
       0,   113,     0,   282,   324,   115,     0,   137,   138,   129,
     118,     0,   131,     0,   139,   195,     0,   111,     0,     0,
     147,     0,   143,     0,   149,   173,   142,    97,   239,     0,
     114,     0,     0,     0,   171,   164,   157,     0,   109,     0,
     107,     0,     0,     0,     2,     8,     6,     7,     9,    10,
       3,     0,     0,     0,     0,     0,    45,    44,    46,    43,
      48,    47,     5,    11,    13,    35,    15,     0,    31,     0,
       0,    49,    30,     0,    53,    56,    59,    64,    67,    69,
      71,    73,    75,    77,    79,    81,    94,   193,     0,   188,
     112,   281,     0,   119,   130,   132,   133,     0,   135,     0,
     127,   173,     0,     0,     0,   145,   144,   148,   123,   151,
     173,   165,   237,     0,     0,   243,     0,     0,     0,   175,
       0,     0,   106,   105,     0,     0,     0,   125,    28,     0,
      36,    37,     0,    41,    40,     0,    39,     0,     0,     4,
      18,    19,     0,     0,     0,    21,     0,    29,     0,     0,
      23,    24,     0,    84,    85,    87,    86,    89,    90,    91,
      92,    93,    88,    83,     0,    42,    30,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   196,     0,
     190,     0,     2,     0,     0,     0,     0,   300,   303,     0,
       0,     0,   277,   286,     0,   265,   268,   284,   267,   266,
       0,   269,   270,   271,   272,   134,   136,    96,     0,   128,
     173,   158,     0,   124,   150,   146,   152,     0,   151,   166,
     167,     0,     0,     0,   245,   241,     0,   126,     0,   121,
     180,   185,     0,     0,     0,     0,     0,   187,     0,   176,
     102,   101,   110,   108,     0,   104,   103,     0,     0,    12,
      26,     0,    17,     0,    20,    16,    25,    32,    22,    34,
      33,    82,    50,    51,    52,    55,    54,    57,    58,    62,
      63,    60,    61,    65,    66,    68,    70,    72,    74,    76,
      78,     0,    95,     0,     0,   275,   274,   273,   315,   314,
     316,     0,     0,     0,     0,     0,     0,     0,   276,     0,
     287,   283,   285,   140,   159,   160,   120,   153,     0,   169,
       0,     0,   173,     0,   248,   173,   246,     0,     0,   122,
       0,     0,     0,     0,     0,     0,     0,   177,   100,    99,
       0,    38,    27,    14,     0,   189,     0,   317,   318,     0,
       0,     0,     0,     0,     0,     0,   162,     0,   154,     0,
     252,   253,   258,   168,   173,   173,   250,     0,   247,   240,
       0,   244,     0,     0,     0,     0,     0,     0,   172,     0,
      27,     0,    80,   191,     0,     0,   306,   305,     0,   308,
       0,     0,     0,     0,     0,   278,     0,   161,   170,     0,
       0,     0,     0,   261,   260,   238,   251,   249,   242,   186,
     179,   181,     0,     0,     0,   178,     0,     0,   307,     0,
       0,   280,   304,   279,     0,   299,   288,   296,   289,   163,
       0,   264,   255,   254,   258,   256,   259,     0,     0,     0,
       0,   192,   301,   312,     0,   310,     0,     0,     0,   295,
       0,   263,     0,   262,     0,     0,     0,   313,   311,   302,
     298,     0,     0,   292,     0,   290,   257,     0,     0,     0,
       0,   294,   297,   291,   182,   183,   184,   293
};

/* YYDEFGOTO[NTERM-NUM]. */
static const short yydefgoto[] =
{
      -1,   142,   143,   144,   145,   343,   146,   147,   148,   149,
     150,   151,   228,   246,   153,   154,   155,   156,   157,   158,
     159,   160,   161,   162,   163,   164,   165,   166,   244,   284,
     306,    76,    77,   119,   120,   285,   115,    50,    51,    52,
      89,    90,    91,    92,    53,   103,   104,   402,   307,   190,
      54,    70,   437,   476,    55,   400,   439,    56,   199,    68,
     328,   459,   329,   168,    57,    58,    59,    60,    61,    79,
     311,   193,   407,   316,   313,   314,   403,   404,   442,   443,
     481,   482,   483,   517,   484,   286,   287,   288,   289,   389,
     502,   503,   172,   290,   291,   292,   544,   545,   508,   530,
     506,   293,   383,   384,   431,   470,   471,   294,     1,    63,
      64
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -442
static const short yypact[] =
{
    -442,  2215,  -442,  -442,  -442,  -442,  -442,  -442,  -442,  -442,
    -442,  -442,  -442,  -442,  -442,  -442,  -442,  -442,  -442,  -442,
    -442,  -442,  -442,  -442,  -442,  -442,  -442,  -442,  -442,  -442,
    -181,  -141,  -442,  -442,  -442,  -442,  -145,  -442,  -152,  -442,
    -442,  -442,  -154,  2948,  -137,  -442,  -130,  -198,  -442,   -94,
     -86,  -108,  3100,  2255,   -68,  -442,  2907,  -138,    39,  -442,
    -442,  -102,   -79,  -442,  -442,  2948,  -145,  -442,  2948,   -59,
    2755,   -58,    37,  -144,  -442,  -442,  -442,  -442,  -138,  -442,
    1708,  -442,   -42,   -29,  -442,  -442,  3100,  -442,  -442,  -442,
    -442,  2255,  -442,  3121,  -159,  -442,  -138,  -442,  2948,  -138,
    -442,  -146,   300,    94,  -442,   -80,  -442,   -48,  -442,  -128,
    -442,  -138,  2948,  -138,  -442,  -442,  -442,    37,    12,  -134,
    -442,    40,    37,    46,    48,  -442,  -442,  -442,  -442,  -442,
    -442,  2289,  2289,  3265,    55,  1876,  -442,  -442,  -442,  -442,
    -442,  -442,  -442,   129,  -442,    -9,  -442,  -149,    56,  1761,
    3265,  -442,   274,  3265,    21,   -28,  -105,  -111,  -150,    41,
      43,    42,   125,   126,  -158,  -442,  -442,    60,    70,  -222,
    -442,  -442,  1186,  -442,  -442,  -442,  -442,  2255,  -442,  3265,
      74,   -54,  -138,    78,    79,    66,  -442,  -442,  -442,  3265,
     -47,    76,  -442,  2948,    83,  -442,    89,  -138,    90,    47,
     -67,  3265,  -442,  -442,    37,    37,   -60,  -442,  -442,  3265,
    -442,  -442,  1876,  -442,  -442,  2044,  -442,  -118,  -196,  -442,
    -442,  -442,   160,  3265,   161,  -442,  3265,  -442,  3265,    92,
    -442,  -442,   -35,  -442,  -442,  -442,  -442,  -442,  -442,  -442,
    -442,  -442,  -442,  -442,  3265,  -442,  -442,  3265,  3265,  3265,
    3265,  3265,  3265,  3265,  3265,  3265,  3265,  3265,  3265,  3265,
    3265,  3265,  3265,  3265,  3265,  3265,  3265,  3265,  -442,  3265,
      93,  -114,  -200,    84,    86,  1929,   177,  -442,  -442,  1186,
     101,   102,    98,  -442,   -46,  -442,  -442,  -442,  -442,  -442,
     832,  -442,  -442,  -442,  -442,  -442,  -442,  -442,   103,   106,
     -47,    95,   105,  -442,  -442,  -442,  -442,   107,  3265,   100,
    -442,  2948,  -138,  2373,  -442,  -442,  2948,  -442,   111,  -442,
    -442,   115,   116,   121,   123,   124,   127,  -442,  -117,  -442,
    -442,  -442,  -442,  -442,   -25,  -442,  -442,  -196,   131,  -442,
    -442,   134,  -442,   135,    60,  -442,  -442,  -442,  -442,  -442,
    -442,  -442,  -442,  -442,  -442,    21,    21,   -28,   -28,  -105,
    -105,  -105,  -105,  -111,  -111,  -150,    41,    43,    42,   125,
     126,   -16,  -442,   139,  3265,  -442,  -442,  -442,  -442,  -442,
    -442,    15,   118,   130,   137,   184,  3265,  3265,  -442,  1186,
    -442,  -442,  -442,  -442,   136,  -442,  -442,  -442,   141,  -442,
    3123,  2525,   -54,    16,  -442,  -145,  -442,  2948,  2564,  -442,
    3265,   224,   228,  3265,  3265,  3265,   150,  -442,  -442,  -442,
     151,  -442,   155,  -442,  3265,  -442,   159,  -442,  -442,  3265,
    1540,   372,   158,   -36,   162,  1009,  -442,  3123,  -442,  3123,
    -442,  -442,   -49,  -442,  -145,   -47,  -442,  -138,  -442,  -442,
    2716,  -442,   163,   165,   169,   164,   170,   171,  -442,    47,
    -442,  3265,  -442,  -442,   -32,   174,  -442,  -442,   175,    60,
     176,  1363,  3265,  1186,   168,  -442,  3123,  -442,  -442,  3265,
     248,   -20,  3123,   180,  -442,  -442,  -442,  -442,  -442,  -442,
    -442,  -442,  3265,  3265,  3265,  -442,   189,  1363,  -442,  3221,
    3244,  -442,  -442,  -442,   -24,   231,  -442,   193,  -442,  -442,
     200,  -442,  -442,  -442,   -49,  -442,  -442,   -49,   197,   198,
     199,  -442,  -442,  -442,   -12,  -442,    -7,   202,  1186,  -442,
     655,  -442,  3123,  -442,  3265,  3265,  3265,  -442,  -442,  -442,
    -442,  3265,   201,  -442,   478,  -442,  -442,   206,   207,   209,
     220,  -442,  -442,  -442,  -442,  -442,  -442,  -442
};

/* YYPGOTO[NTERM-NUM].  */
static const short yypgoto[] =
{
    -442,  -442,  -442,  -442,  -442,    81,  -442,  -442,   147,  -442,
    -442,  -142,  -442,   -78,  -442,   -13,    44,   -14,    33,   215,
     192,   216,   214,   205,   218,  -442,  -156,  -112,  -442,   -73,
      26,    -1,     0,   -96,   276,   480,   483,  -442,  -442,  -442,
     -53,   399,   393,   -50,   104,   -93,   388,   -15,   182,  -167,
    -442,  -442,  -442,  -442,   421,  -442,  -442,   -30,  -442,   -37,
    -442,  -442,    34,  -442,     5,  -442,    -3,  -442,  -442,     3,
    -442,  -442,  -442,  -442,  -296,  -308,  -442,    45,  -442,  -319,
    -442,   -19,  -442,  -442,   -23,    68,  -244,  -441,  -442,  -442,
       2,   452,  -442,   114,    85,  -442,  -442,   -40,  -442,  -442,
    -442,  -442,  -442,  -442,  -442,  -442,  -442,  -442,  -442,  -442,
    -442
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -310
static const short yytable[] =
{
      46,    47,   152,   100,    62,   406,    67,   167,    71,   186,
      82,   245,    73,   299,   300,   401,   100,    67,   121,   269,
     408,   200,   265,   297,   100,    38,   206,   258,   259,   112,
     501,   109,   102,   297,   194,   385,    67,   231,   174,   208,
     176,   175,   105,   178,   340,   297,   392,   377,    78,    94,
      95,  -173,    65,   210,   211,   214,   501,   152,   102,   202,
     107,    99,   217,   123,   254,   255,   252,   253,   191,    38,
     111,   152,   152,   113,    66,    67,   270,   232,    38,   180,
     101,   181,   179,    94,   183,   169,   347,    69,    94,    72,
      94,   225,   305,   406,   152,   102,   196,   226,   198,   122,
     406,   101,    80,   182,   266,   352,   353,   354,   102,   334,
     203,   450,   204,   297,   346,   195,   102,   197,   477,    81,
     478,    38,   339,   416,   295,   341,   330,   296,   267,   417,
     375,   152,   351,   335,   152,   376,   217,   338,    86,   217,
     218,   108,   406,    38,   301,   152,   256,   257,   152,    83,
     344,    67,   297,   309,    85,   372,    93,   509,   349,   188,
      67,   189,   106,   515,   220,   221,   152,   302,   418,    38,
     110,    46,    47,   512,    94,    62,    38,   331,    96,   204,
     114,    97,   318,   222,   336,   117,   204,   189,   152,   152,
      93,   392,   479,   371,   308,   192,   480,   152,   312,   118,
     267,   152,   381,   390,   473,   298,   185,   170,   497,   350,
     267,   267,   152,   546,   267,   171,   527,   337,   297,   419,
     337,   204,   267,   250,   513,   251,   514,   332,   537,   505,
     267,   424,   223,   538,   267,   445,   224,   355,   356,   267,
     359,   360,   361,   362,   420,    27,    28,    29,    74,    75,
      32,    33,    34,    35,   297,    37,   187,   297,   297,   297,
     201,   267,   447,   394,   427,   448,   320,   321,   322,   323,
      67,   324,   325,   326,   327,   247,   248,   249,    46,    47,
     213,   216,    62,   205,   540,   207,   543,   208,   441,    46,
      47,   363,   364,    62,   215,   373,   357,   358,   219,   227,
     543,   262,   260,   261,   263,   297,   267,   264,   152,   152,
     268,   152,   462,   433,   344,   189,   312,   303,   312,   304,
     102,   312,   152,   297,   310,   441,   315,   441,   317,   319,
     342,   345,   348,   378,   374,   379,   297,   297,   297,   382,
     386,   387,   388,   395,   396,   393,   152,   308,   399,   397,
     409,   152,   152,   152,   410,   411,   464,   152,   469,   152,
     412,   152,   413,   414,   441,   446,   415,   428,   449,   429,
     441,   421,    67,   468,   422,    67,   430,   423,   297,   297,
     297,   425,   432,   438,   436,   297,   453,   454,    46,    47,
     458,   460,    62,   152,   152,   152,   461,   472,   152,   504,
     426,   463,   474,   489,   152,   490,   312,   485,   486,   491,
     492,   507,   312,   312,    67,    67,   493,   494,   511,   152,
     441,   152,   152,   498,   499,   500,   524,   526,   516,    46,
      47,   521,   528,    62,    46,    47,   452,   529,    62,   455,
     456,   457,   531,   534,   535,   536,   554,   555,   551,   556,
     152,   539,   152,   366,   152,   312,   233,   234,   235,   236,
     237,   238,   239,   240,   241,   242,   152,   557,   434,   369,
      46,    47,    46,    47,    62,   365,    62,   368,   367,   271,
     333,    48,     3,   370,    49,   173,   177,   496,     4,   184,
     398,   116,   487,   495,   533,   532,    46,    47,   466,   522,
      62,    84,     5,   435,   553,   510,    27,    28,    29,    74,
      75,    32,    33,    34,    35,   467,    37,     0,   518,   519,
     520,     0,   243,     0,     0,     0,     0,    46,    47,    46,
      47,    62,     0,    62,   124,     0,   125,   126,   127,   128,
     129,   130,     0,    46,    47,   131,   132,    62,     6,     0,
       0,     0,     0,     0,   102,     0,     0,     0,     0,     0,
     547,   548,   549,     0,     0,     0,     0,   550,     0,     0,
       0,     0,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   133,     0,   134,     0,
       0,   212,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  -309,   136,   137,   138,   139,   140,     0,     0,     0,
       0,     0,     0,   141,     0,    21,    22,    23,    24,    25,
     272,    26,   125,   126,   127,   128,   129,   130,     0,     0,
       0,   131,   132,     0,     0,     0,   271,     0,     0,     3,
       0,     0,     0,     0,     0,     4,     0,     0,     0,     0,
       0,     0,   273,   274,   275,   276,   277,   278,   279,     5,
     280,   281,   541,   542,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,     0,     0,     0,     0,     0,
       0,    38,     0,     0,     0,     0,     0,    39,    40,    41,
      42,    43,   133,     0,   134,    44,     0,   212,     0,     0,
       0,   282,   552,     0,     0,     6,     0,   283,   136,   137,
     138,   139,   140,     0,     0,     0,     0,     0,     0,   141,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     7,
       8,     9,    10,    11,    12,    13,    14,    15,    16,    17,
      18,    19,    20,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    21,    22,    23,    24,    25,   272,    26,   125,
     126,   127,   128,   129,   130,     0,     0,     0,   131,   132,
       0,     0,     0,   271,     0,     0,     3,     0,     0,     0,
       0,     0,     4,     0,     0,     0,     0,     0,     0,   273,
     274,   275,   276,   277,   278,   279,     5,   280,   281,   541,
     542,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,     0,     0,     0,     0,     0,     0,    38,     0,
       0,     0,     0,     0,    39,    40,    41,    42,    43,   133,
       0,   134,    44,     0,   212,     0,     0,     0,   282,     0,
       0,     0,     6,     0,   283,   136,   137,   138,   139,   140,
       0,     0,     0,     0,     0,     0,   141,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     7,     8,     9,    10,
      11,    12,    13,    14,    15,    16,    17,    18,    19,    20,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    21,
      22,    23,    24,    25,   272,    26,   125,   126,   127,   128,
     129,   130,     0,     0,     0,   131,   132,     0,     0,     0,
     271,     0,     0,     3,     0,     0,     0,     0,     0,     4,
       0,     0,     0,     0,     0,     0,   273,   274,   275,   276,
     277,   278,   279,     5,   280,   281,     0,     0,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,     0,
       0,     0,     0,     0,     0,    38,     0,     0,     0,     0,
       0,    39,    40,    41,    42,    43,   133,     0,   134,    44,
       0,   212,     0,     0,     0,   282,   391,     0,     0,     6,
       0,   283,   136,   137,   138,   139,   140,     0,     0,     0,
       0,     0,     0,   141,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     7,     8,     9,    10,    11,    12,    13,
      14,    15,    16,    17,    18,    19,    20,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    21,    22,    23,    24,
      25,   272,    26,   125,   126,   127,   128,   129,   130,     0,
       0,     0,   131,   132,     0,     0,     0,   271,     0,     0,
       3,     0,     0,     0,     0,     0,     4,     0,     0,     0,
       0,     0,     0,   273,   274,   275,   276,   277,   278,   279,
       5,   280,   281,     0,     0,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,     0,     0,     0,     0,
       0,     0,    38,     0,     0,     0,     0,     0,    39,    40,
      41,    42,    43,   133,     0,   134,    44,     0,   212,     0,
       0,     0,   282,   475,     0,     0,     6,     0,   283,   136,
     137,   138,   139,   140,     0,     0,     0,     0,     0,     0,
     141,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
      17,    18,    19,    20,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    21,    22,    23,    24,    25,   272,    26,
     125,   126,   127,   128,   129,   130,     0,     0,     0,   131,
     132,     0,     0,     0,   271,     0,     0,     3,     0,     0,
       0,     0,     0,     4,     0,     0,     0,     0,     0,     0,
     273,   274,   275,   276,   277,   278,   279,     5,   280,   281,
       0,     0,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,     0,     0,     0,     0,     0,     0,    38,
       0,     0,     0,     0,     0,    39,    40,    41,    42,    43,
     133,     0,   134,    44,     0,   212,     0,     0,     0,   282,
       0,     0,     0,     6,     0,   283,   136,   137,   138,   139,
     140,     0,     0,     0,     0,     0,     0,   141,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      21,    22,    23,    24,    25,   272,    26,   125,   126,   127,
     128,   129,   130,     0,     0,     0,   131,   132,     0,     0,
       0,   465,     0,     0,     3,     0,     0,     0,     0,     0,
       4,     0,     0,     0,     0,     0,     0,   273,   274,   275,
     276,   277,   278,   279,     5,   280,   281,     0,     0,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
       0,     0,     0,     0,     0,     0,    38,     0,     0,     0,
       0,     0,    39,    40,    41,    42,    43,   133,     0,   134,
      44,     0,   212,     0,     0,     0,    83,     0,     0,     0,
       6,     0,   283,   136,   137,   138,   139,   140,     0,     0,
       0,     0,     0,     0,   141,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     7,     8,     9,    10,    11,    12,
      13,    14,    15,    16,    17,    18,    19,    20,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    21,    22,    23,
      24,    25,   124,    26,   125,   126,   127,   128,   129,   130,
       0,     0,     3,   131,   132,     0,     0,     0,     4,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     5,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,     0,     0,     0,
       0,     0,     0,    38,     0,   229,     0,     0,     0,    39,
      40,    41,    42,    43,   133,     0,   134,    44,     6,   212,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   283,
     136,   137,   138,   139,   140,     0,     0,     0,     0,     0,
       0,   141,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    21,    22,    23,    24,    25,
     124,    26,   125,   126,   127,   128,   129,   130,     0,     0,
       3,   131,   132,     0,     0,     0,     4,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       5,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    27,    28,    29,    74,    75,    32,
      33,    34,    35,   124,    37,   125,   126,   127,   128,   129,
     130,     0,     0,     0,   131,   132,     0,    39,    40,     0,
      42,     0,   133,     0,   134,    44,     6,   135,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   136,   137,
     138,   139,   140,     0,     0,     0,     0,     0,     0,   141,
       7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
      17,    18,    19,    20,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   133,     0,   134,     0,     0,
     212,   230,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   136,   137,   138,   139,   140,     0,     0,     0,     0,
       0,     0,   141,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    21,    22,    23,    24,    25,   124,    26,
     125,   126,   127,   128,   129,   130,     0,     0,     3,   131,
     132,     0,     0,     0,     4,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     5,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    27,    28,    29,    74,    75,    32,    33,    34,
      35,   124,    37,   125,   126,   127,   128,   129,   130,     0,
       0,     0,   131,   132,     0,    39,    40,     0,    42,     0,
     133,     0,   134,    44,     6,   212,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   136,   137,   138,   139,
     140,     0,     0,     0,     0,     0,     0,   141,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    17,    18,
      19,    20,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   133,     0,   134,     0,     0,   212,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   380,   136,
     137,   138,   139,   140,     0,     0,     0,     0,     0,     0,
     141,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    21,    22,    23,    24,    25,   124,    26,   125,   126,
     127,   128,   129,   130,     0,     2,     0,   131,   132,     3,
       0,     0,     0,     0,     0,     4,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     5,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      27,    28,    29,    74,    75,    32,    33,    34,    35,     3,
      37,     0,     0,     0,     0,     4,     0,     0,     0,     0,
       0,     0,     0,    39,    40,     0,    42,     0,   133,     5,
     134,    44,     0,   209,     0,     6,     0,     0,     0,     0,
       0,     0,     0,     0,   136,   137,   138,   139,   140,     0,
       0,     0,     0,     0,     0,   141,     0,     0,     0,     7,
       8,     9,    10,    11,    12,    13,    14,    15,    16,    17,
      18,    19,    20,     0,     0,     6,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     7,
       8,     9,    10,    11,    12,    13,    14,    15,    16,    17,
      18,    19,    20,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    21,    22,    23,    24,    25,     3,    26,     0,
       0,     0,     0,     4,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     5,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    21,    22,    23,    24,    25,     0,    26,     0,
       0,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,     0,     0,     0,     0,     0,     0,    38,     0,
       0,     0,     0,     6,    39,    40,    41,    42,    43,     0,
       0,   124,    44,   125,   126,   127,   128,   129,   130,     0,
       0,     0,   131,   132,    45,     0,     0,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,     0,     0,     0,    39,    40,     0,    42,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   133,     0,   134,     0,     0,   209,     3,
      21,    22,    23,    24,    25,     4,    26,     0,     0,   136,
     137,   138,   139,   140,     0,     0,     0,     0,     0,     5,
     141,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     3,     0,
       0,     0,     0,     0,     4,     0,     0,     0,     0,    27,
      28,    29,    74,    75,    32,    33,    34,    35,     5,    37,
       0,     0,     0,     0,     0,     6,     0,     0,     0,     0,
       0,     0,    39,    40,     0,    42,     0,     0,     0,     0,
      44,     0,     0,     0,     0,     0,     0,   405,     0,     7,
       8,     9,    10,    11,    12,    13,    14,    15,    16,    17,
      18,    19,    20,     0,     6,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    17,    18,
      19,    20,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    21,    22,    23,    24,    25,     0,    26,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       3,    21,    22,    23,    24,    25,     4,    26,     0,     0,
       0,    27,    28,    29,    74,    75,    32,    33,    34,    35,
       5,    37,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    39,    40,     0,    42,     0,     3,
       0,     0,    44,     0,     0,     4,     0,     0,     0,   444,
      27,    28,    29,    74,    75,    32,    33,    34,    35,     5,
      37,     0,     0,     0,     0,     0,     6,     0,     0,     0,
       0,     0,     0,    39,    40,     0,    42,     0,     0,     0,
       0,    44,     0,     0,     0,     0,     0,     0,   451,     0,
       7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
      17,    18,    19,    20,     0,     6,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     7,
       8,     9,    10,    11,    12,    13,    14,    15,    16,    17,
      18,    19,    20,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    21,    22,    23,    24,    25,     0,    26,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     3,    21,    22,    23,    24,    25,     4,    26,     0,
       0,     0,    27,    28,    29,    74,    75,    32,    33,    34,
      35,     5,    37,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    39,    40,     0,    42,     0,
       0,     0,     3,    44,     0,     0,     0,     0,     4,     0,
     488,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,     5,     0,     0,     0,     0,     6,    38,     0,
       0,     0,     0,     0,    39,    40,     0,    42,    43,     0,
       0,     0,    44,     0,     0,     0,     0,     0,     0,     0,
       0,     7,     8,     9,    10,    11,    12,    13,    14,    15,
      16,    17,    18,    19,    20,     0,     0,     0,     6,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,     0,     0,     0,     0,
       0,     0,     0,     0,    21,    22,    23,    24,    25,     0,
      26,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     3,    21,    22,    23,    24,    25,
       4,    26,     0,    27,    28,    29,    74,    75,    32,    33,
      34,    35,    98,    37,     5,     3,     0,     0,     0,     0,
       0,     4,     0,     0,     0,     0,    39,    40,     0,    42,
       0,     0,     0,     0,    44,     5,     0,     0,     0,     0,
       0,     0,     0,     0,    27,    28,    29,    74,    75,    32,
      33,    34,    35,     0,    37,     0,     0,     0,     0,     0,
       6,     0,     0,     0,     0,     0,     0,    39,    40,     0,
      42,     0,     0,     0,     0,    44,     0,     0,     0,     0,
       0,     6,     0,     0,     7,     8,     9,    10,    11,    12,
      13,    14,    15,    16,    17,    18,    19,    20,     0,     0,
       0,     0,     0,     0,     0,     7,     8,     9,    10,    11,
      12,    13,    14,    15,    16,    17,    18,    19,    20,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    21,    22,    23,
      24,    25,     0,    26,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    21,    22,
      23,    24,    25,     0,    26,   124,     0,   125,   126,   127,
     128,   129,   130,     0,     0,     0,   131,   132,     0,     0,
       0,     0,     0,     0,     0,     0,    27,    28,    29,    74,
      75,    32,    33,    34,    35,     0,    37,    87,    88,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    39,
      40,     0,    42,     0,     0,     0,     0,     0,    87,    88,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      39,    40,     0,    42,     0,     0,     0,   133,     0,   134,
       0,     0,   212,     0,     0,     0,   440,     0,     0,     0,
       0,     0,     0,   136,   137,   138,   139,   140,     0,     0,
       0,     0,     0,   124,   141,   125,   126,   127,   128,   129,
     130,     0,     0,     0,   131,   132,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   124,     0,   125,   126,
     127,   128,   129,   130,     0,     0,     0,   131,   132,     0,
       0,     0,     0,     0,     0,     0,     0,   124,     0,   125,
     126,   127,   128,   129,   130,     0,     0,     0,   131,   132,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   133,     0,   134,     0,     0,
     212,   523,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   136,   137,   138,   139,   140,     0,     0,   133,     0,
     134,     0,   141,   212,   525,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   136,   137,   138,   139,   140,   133,
       0,   134,     0,     0,   212,   141,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   136,   137,   138,   139,   140,
       0,     0,     0,     0,     0,     0,   141
};

static const short yycheck[] =
{
       1,     1,    80,   162,     1,   313,    36,    80,   162,   102,
      47,   153,    42,   180,   181,   311,   162,    47,   162,   241,
     316,   117,   180,   179,   162,   223,   122,   177,   178,    66,
     471,    61,   254,   189,   162,   279,    66,   149,    91,   239,
      93,    91,    57,    93,   240,   201,   290,   247,    43,    52,
      53,   249,   233,   131,   132,   133,   497,   135,   254,   193,
     162,    56,   135,    78,   175,   176,   171,   172,   105,   223,
      65,   149,   150,    68,   215,   105,   169,   150,   223,    94,
     239,    96,   241,    86,    99,    80,   228,   239,    91,   243,
      93,   240,   185,   401,   172,   254,   111,   246,   113,   243,
     408,   239,   239,    98,   262,   247,   248,   249,   254,   205,
     244,   407,   246,   269,   226,   243,   254,   112,   437,   249,
     439,   223,   240,   240,   177,   218,   193,   177,   246,   246,
     244,   209,   244,   193,   212,   249,   209,   215,   246,   212,
     135,   243,   450,   223,   181,   223,   257,   258,   226,   243,
     223,   181,   308,   190,   240,   267,    52,   476,   193,   239,
     190,   241,    58,   482,   173,   174,   244,   182,   193,   223,
     249,   172,   172,   193,   177,   172,   223,   244,   246,   246,
     239,   249,   197,   192,   244,   243,   246,   241,   266,   267,
      86,   435,   241,   266,   241,   243,   245,   275,   193,   162,
     246,   279,   275,   249,   240,   179,   102,   249,   240,   244,
     246,   246,   290,   532,   246,   244,   240,   212,   374,   244,
     215,   246,   246,   251,   244,   253,   246,   201,   240,   473,
     246,   247,   241,   240,   246,   402,   245,   250,   251,   246,
     254,   255,   256,   257,   337,   206,   207,   208,   209,   210,
     211,   212,   213,   214,   410,   216,   162,   413,   414,   415,
     248,   246,   246,   300,   249,   249,   219,   220,   221,   222,
     300,   224,   225,   226,   227,   254,   255,   256,   279,   279,
     133,   134,   279,   243,   528,   239,   530,   239,   400,   290,
     290,   258,   259,   290,   239,   269,   252,   253,   169,   243,
     544,   259,   261,   260,   179,   461,   246,   181,   386,   387,
     240,   389,   424,   386,   387,   241,   311,   239,   313,   240,
     254,   316,   400,   479,   248,   437,   243,   439,   239,   239,
     170,   170,   240,   249,   241,   249,   492,   493,   494,   162,
     239,   239,   244,   248,   239,   242,   424,   241,   248,   242,
     239,   429,   430,   431,   239,   239,   429,   435,   431,   437,
     239,   439,   239,   239,   476,   402,   239,   249,   405,   239,
     482,   240,   402,     1,   240,   405,   239,   242,   534,   535,
     536,   242,   198,   242,   248,   541,   162,   159,   389,   389,
     240,   240,   389,   471,   472,   473,   241,   239,   476,   472,
     374,   242,   240,   240,   482,   240,   401,   444,   445,   240,
     246,   243,   407,   408,   444,   445,   246,   246,   170,   497,
     532,   499,   500,   249,   249,   249,   499,   500,   248,   430,
     430,   242,   201,   430,   435,   435,   410,   244,   435,   413,
     414,   415,   242,   246,   246,   246,   240,   240,   247,   240,
     528,   249,   530,   261,   532,   450,   182,   183,   184,   185,
     186,   187,   188,   189,   190,   191,   544,   247,   387,   264,
     471,   471,   473,   473,   471,   260,   473,   263,   262,     1,
     204,     1,     4,   265,     1,    86,    93,   461,    10,   101,
     308,    70,   447,   459,   517,   514,   497,   497,   430,   497,
     497,    49,    24,   389,   544,   479,   206,   207,   208,   209,
     210,   211,   212,   213,   214,   430,   216,    -1,   492,   493,
     494,    -1,   248,    -1,    -1,    -1,    -1,   528,   528,   530,
     530,   528,    -1,   530,   162,    -1,   164,   165,   166,   167,
     168,   169,    -1,   544,   544,   173,   174,   544,    70,    -1,
      -1,    -1,    -1,    -1,   254,    -1,    -1,    -1,    -1,    -1,
     534,   535,   536,    -1,    -1,    -1,    -1,   541,    -1,    -1,
      -1,    -1,    94,    95,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   234,    -1,   236,    -1,
      -1,   239,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   249,   250,   251,   252,   253,   254,    -1,    -1,    -1,
      -1,    -1,    -1,   261,    -1,   157,   158,   159,   160,   161,
     162,   163,   164,   165,   166,   167,   168,   169,    -1,    -1,
      -1,   173,   174,    -1,    -1,    -1,     1,    -1,    -1,     4,
      -1,    -1,    -1,    -1,    -1,    10,    -1,    -1,    -1,    -1,
      -1,    -1,   194,   195,   196,   197,   198,   199,   200,    24,
     202,   203,   204,   205,   206,   207,   208,   209,   210,   211,
     212,   213,   214,   215,   216,    -1,    -1,    -1,    -1,    -1,
      -1,   223,    -1,    -1,    -1,    -1,    -1,   229,   230,   231,
     232,   233,   234,    -1,   236,   237,    -1,   239,    -1,    -1,
      -1,   243,   244,    -1,    -1,    70,    -1,   249,   250,   251,
     252,   253,   254,    -1,    -1,    -1,    -1,    -1,    -1,   261,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   157,   158,   159,   160,   161,   162,   163,   164,
     165,   166,   167,   168,   169,    -1,    -1,    -1,   173,   174,
      -1,    -1,    -1,     1,    -1,    -1,     4,    -1,    -1,    -1,
      -1,    -1,    10,    -1,    -1,    -1,    -1,    -1,    -1,   194,
     195,   196,   197,   198,   199,   200,    24,   202,   203,   204,
     205,   206,   207,   208,   209,   210,   211,   212,   213,   214,
     215,   216,    -1,    -1,    -1,    -1,    -1,    -1,   223,    -1,
      -1,    -1,    -1,    -1,   229,   230,   231,   232,   233,   234,
      -1,   236,   237,    -1,   239,    -1,    -1,    -1,   243,    -1,
      -1,    -1,    70,    -1,   249,   250,   251,   252,   253,   254,
      -1,    -1,    -1,    -1,    -1,    -1,   261,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    94,    95,    96,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   157,
     158,   159,   160,   161,   162,   163,   164,   165,   166,   167,
     168,   169,    -1,    -1,    -1,   173,   174,    -1,    -1,    -1,
       1,    -1,    -1,     4,    -1,    -1,    -1,    -1,    -1,    10,
      -1,    -1,    -1,    -1,    -1,    -1,   194,   195,   196,   197,
     198,   199,   200,    24,   202,   203,    -1,    -1,   206,   207,
     208,   209,   210,   211,   212,   213,   214,   215,   216,    -1,
      -1,    -1,    -1,    -1,    -1,   223,    -1,    -1,    -1,    -1,
      -1,   229,   230,   231,   232,   233,   234,    -1,   236,   237,
      -1,   239,    -1,    -1,    -1,   243,   244,    -1,    -1,    70,
      -1,   249,   250,   251,   252,   253,   254,    -1,    -1,    -1,
      -1,    -1,    -1,   261,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    94,    95,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   157,   158,   159,   160,
     161,   162,   163,   164,   165,   166,   167,   168,   169,    -1,
      -1,    -1,   173,   174,    -1,    -1,    -1,     1,    -1,    -1,
       4,    -1,    -1,    -1,    -1,    -1,    10,    -1,    -1,    -1,
      -1,    -1,    -1,   194,   195,   196,   197,   198,   199,   200,
      24,   202,   203,    -1,    -1,   206,   207,   208,   209,   210,
     211,   212,   213,   214,   215,   216,    -1,    -1,    -1,    -1,
      -1,    -1,   223,    -1,    -1,    -1,    -1,    -1,   229,   230,
     231,   232,   233,   234,    -1,   236,   237,    -1,   239,    -1,
      -1,    -1,   243,   244,    -1,    -1,    70,    -1,   249,   250,
     251,   252,   253,   254,    -1,    -1,    -1,    -1,    -1,    -1,
     261,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      94,    95,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   157,   158,   159,   160,   161,   162,   163,
     164,   165,   166,   167,   168,   169,    -1,    -1,    -1,   173,
     174,    -1,    -1,    -1,     1,    -1,    -1,     4,    -1,    -1,
      -1,    -1,    -1,    10,    -1,    -1,    -1,    -1,    -1,    -1,
     194,   195,   196,   197,   198,   199,   200,    24,   202,   203,
      -1,    -1,   206,   207,   208,   209,   210,   211,   212,   213,
     214,   215,   216,    -1,    -1,    -1,    -1,    -1,    -1,   223,
      -1,    -1,    -1,    -1,    -1,   229,   230,   231,   232,   233,
     234,    -1,   236,   237,    -1,   239,    -1,    -1,    -1,   243,
      -1,    -1,    -1,    70,    -1,   249,   250,   251,   252,   253,
     254,    -1,    -1,    -1,    -1,    -1,    -1,   261,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    94,    95,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     157,   158,   159,   160,   161,   162,   163,   164,   165,   166,
     167,   168,   169,    -1,    -1,    -1,   173,   174,    -1,    -1,
      -1,     1,    -1,    -1,     4,    -1,    -1,    -1,    -1,    -1,
      10,    -1,    -1,    -1,    -1,    -1,    -1,   194,   195,   196,
     197,   198,   199,   200,    24,   202,   203,    -1,    -1,   206,
     207,   208,   209,   210,   211,   212,   213,   214,   215,   216,
      -1,    -1,    -1,    -1,    -1,    -1,   223,    -1,    -1,    -1,
      -1,    -1,   229,   230,   231,   232,   233,   234,    -1,   236,
     237,    -1,   239,    -1,    -1,    -1,   243,    -1,    -1,    -1,
      70,    -1,   249,   250,   251,   252,   253,   254,    -1,    -1,
      -1,    -1,    -1,    -1,   261,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    94,    95,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   157,   158,   159,
     160,   161,   162,   163,   164,   165,   166,   167,   168,   169,
      -1,    -1,     4,   173,   174,    -1,    -1,    -1,    10,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    24,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   206,   207,   208,   209,
     210,   211,   212,   213,   214,   215,   216,    -1,    -1,    -1,
      -1,    -1,    -1,   223,    -1,     4,    -1,    -1,    -1,   229,
     230,   231,   232,   233,   234,    -1,   236,   237,    70,   239,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   249,
     250,   251,   252,   253,   254,    -1,    -1,    -1,    -1,    -1,
      -1,   261,    94,    95,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   157,   158,   159,   160,   161,
     162,   163,   164,   165,   166,   167,   168,   169,    -1,    -1,
       4,   173,   174,    -1,    -1,    -1,    10,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      24,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   206,   207,   208,   209,   210,   211,
     212,   213,   214,   162,   216,   164,   165,   166,   167,   168,
     169,    -1,    -1,    -1,   173,   174,    -1,   229,   230,    -1,
     232,    -1,   234,    -1,   236,   237,    70,   239,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   250,   251,
     252,   253,   254,    -1,    -1,    -1,    -1,    -1,    -1,   261,
      94,    95,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   234,    -1,   236,    -1,    -1,
     239,   240,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   250,   251,   252,   253,   254,    -1,    -1,    -1,    -1,
      -1,    -1,   261,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   157,   158,   159,   160,   161,   162,   163,
     164,   165,   166,   167,   168,   169,    -1,    -1,     4,   173,
     174,    -1,    -1,    -1,    10,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    24,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   206,   207,   208,   209,   210,   211,   212,   213,
     214,   162,   216,   164,   165,   166,   167,   168,   169,    -1,
      -1,    -1,   173,   174,    -1,   229,   230,    -1,   232,    -1,
     234,    -1,   236,   237,    70,   239,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   250,   251,   252,   253,
     254,    -1,    -1,    -1,    -1,    -1,    -1,   261,    94,    95,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   234,    -1,   236,    -1,    -1,   239,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   249,   250,
     251,   252,   253,   254,    -1,    -1,    -1,    -1,    -1,    -1,
     261,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   157,   158,   159,   160,   161,   162,   163,   164,   165,
     166,   167,   168,   169,    -1,     0,    -1,   173,   174,     4,
      -1,    -1,    -1,    -1,    -1,    10,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    24,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     206,   207,   208,   209,   210,   211,   212,   213,   214,     4,
     216,    -1,    -1,    -1,    -1,    10,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   229,   230,    -1,   232,    -1,   234,    24,
     236,   237,    -1,   239,    -1,    70,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   250,   251,   252,   253,   254,    -1,
      -1,    -1,    -1,    -1,    -1,   261,    -1,    -1,    -1,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,    -1,    -1,    70,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   157,   158,   159,   160,   161,     4,   163,    -1,
      -1,    -1,    -1,    10,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    24,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   157,   158,   159,   160,   161,    -1,   163,    -1,
      -1,   206,   207,   208,   209,   210,   211,   212,   213,   214,
     215,   216,    -1,    -1,    -1,    -1,    -1,    -1,   223,    -1,
      -1,    -1,    -1,    70,   229,   230,   231,   232,   233,    -1,
      -1,   162,   237,   164,   165,   166,   167,   168,   169,    -1,
      -1,    -1,   173,   174,   249,    -1,    -1,    94,    95,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,    -1,    -1,    -1,   229,   230,    -1,   232,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   234,    -1,   236,    -1,    -1,   239,     4,
     157,   158,   159,   160,   161,    10,   163,    -1,    -1,   250,
     251,   252,   253,   254,    -1,    -1,    -1,    -1,    -1,    24,
     261,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,     4,    -1,
      -1,    -1,    -1,    -1,    10,    -1,    -1,    -1,    -1,   206,
     207,   208,   209,   210,   211,   212,   213,   214,    24,   216,
      -1,    -1,    -1,    -1,    -1,    70,    -1,    -1,    -1,    -1,
      -1,    -1,   229,   230,    -1,   232,    -1,    -1,    -1,    -1,
     237,    -1,    -1,    -1,    -1,    -1,    -1,   244,    -1,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,    -1,    70,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    94,    95,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   157,   158,   159,   160,   161,    -1,   163,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
       4,   157,   158,   159,   160,   161,    10,   163,    -1,    -1,
      -1,   206,   207,   208,   209,   210,   211,   212,   213,   214,
      24,   216,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   229,   230,    -1,   232,    -1,     4,
      -1,    -1,   237,    -1,    -1,    10,    -1,    -1,    -1,   244,
     206,   207,   208,   209,   210,   211,   212,   213,   214,    24,
     216,    -1,    -1,    -1,    -1,    -1,    70,    -1,    -1,    -1,
      -1,    -1,    -1,   229,   230,    -1,   232,    -1,    -1,    -1,
      -1,   237,    -1,    -1,    -1,    -1,    -1,    -1,   244,    -1,
      94,    95,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,    -1,    70,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   157,   158,   159,   160,   161,    -1,   163,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,     4,   157,   158,   159,   160,   161,    10,   163,    -1,
      -1,    -1,   206,   207,   208,   209,   210,   211,   212,   213,
     214,    24,   216,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   229,   230,    -1,   232,    -1,
      -1,    -1,     4,   237,    -1,    -1,    -1,    -1,    10,    -1,
     244,   206,   207,   208,   209,   210,   211,   212,   213,   214,
     215,   216,    24,    -1,    -1,    -1,    -1,    70,   223,    -1,
      -1,    -1,    -1,    -1,   229,   230,    -1,   232,   233,    -1,
      -1,    -1,   237,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    94,    95,    96,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,    -1,    -1,    -1,    70,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    94,    95,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   157,   158,   159,   160,   161,    -1,
     163,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,     4,   157,   158,   159,   160,   161,
      10,   163,    -1,   206,   207,   208,   209,   210,   211,   212,
     213,   214,   215,   216,    24,     4,    -1,    -1,    -1,    -1,
      -1,    10,    -1,    -1,    -1,    -1,   229,   230,    -1,   232,
      -1,    -1,    -1,    -1,   237,    24,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   206,   207,   208,   209,   210,   211,
     212,   213,   214,    -1,   216,    -1,    -1,    -1,    -1,    -1,
      70,    -1,    -1,    -1,    -1,    -1,    -1,   229,   230,    -1,
     232,    -1,    -1,    -1,    -1,   237,    -1,    -1,    -1,    -1,
      -1,    70,    -1,    -1,    94,    95,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    94,    95,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   157,   158,   159,
     160,   161,    -1,   163,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   157,   158,
     159,   160,   161,    -1,   163,   162,    -1,   164,   165,   166,
     167,   168,   169,    -1,    -1,    -1,   173,   174,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   206,   207,   208,   209,
     210,   211,   212,   213,   214,    -1,   216,   217,   218,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   229,
     230,    -1,   232,    -1,    -1,    -1,    -1,    -1,   217,   218,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     229,   230,    -1,   232,    -1,    -1,    -1,   234,    -1,   236,
      -1,    -1,   239,    -1,    -1,    -1,   243,    -1,    -1,    -1,
      -1,    -1,    -1,   250,   251,   252,   253,   254,    -1,    -1,
      -1,    -1,    -1,   162,   261,   164,   165,   166,   167,   168,
     169,    -1,    -1,    -1,   173,   174,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   162,    -1,   164,   165,
     166,   167,   168,   169,    -1,    -1,    -1,   173,   174,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   162,    -1,   164,
     165,   166,   167,   168,   169,    -1,    -1,    -1,   173,   174,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   234,    -1,   236,    -1,    -1,
     239,   240,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   250,   251,   252,   253,   254,    -1,    -1,   234,    -1,
     236,    -1,   261,   239,   240,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   250,   251,   252,   253,   254,   234,
      -1,   236,    -1,    -1,   239,   261,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   250,   251,   252,   253,   254,
      -1,    -1,    -1,    -1,    -1,    -1,   261
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const unsigned short yystos[] =
{
       0,   371,     0,     4,    10,    24,    70,    94,    95,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   157,   158,   159,   160,   161,   163,   206,   207,   208,
     209,   210,   211,   212,   213,   214,   215,   216,   223,   229,
     230,   231,   232,   233,   237,   249,   294,   295,   298,   299,
     300,   301,   302,   307,   313,   317,   320,   327,   328,   329,
     330,   331,   332,   372,   373,   233,   215,   320,   322,   239,
     314,   162,   243,   320,   209,   210,   294,   295,   327,   332,
     239,   249,   322,   243,   354,   240,   246,   217,   218,   303,
     304,   305,   306,   307,   329,   329,   246,   249,   215,   327,
     162,   239,   254,   308,   309,   310,   307,   162,   243,   320,
     249,   327,   322,   327,   239,   299,   317,   243,   162,   296,
     297,   162,   243,   310,   162,   164,   165,   166,   167,   168,
     169,   173,   174,   234,   236,   239,   250,   251,   252,   253,
     254,   261,   264,   265,   266,   267,   269,   270,   271,   272,
     273,   274,   276,   277,   278,   279,   280,   281,   282,   283,
     284,   285,   286,   287,   288,   289,   290,   292,   326,   327,
     249,   244,   355,   304,   303,   306,   303,   305,   306,   241,
     310,   310,   327,   310,   309,   307,   308,   162,   239,   241,
     312,   322,   243,   334,   162,   243,   310,   327,   310,   321,
     296,   248,   193,   244,   246,   243,   296,   239,   239,   239,
     276,   276,   239,   271,   276,   239,   271,   292,   327,   169,
     173,   174,   192,   241,   245,   240,   246,   243,   275,     4,
     240,   290,   292,   182,   183,   184,   185,   186,   187,   188,
     189,   190,   191,   248,   291,   274,   276,   254,   255,   256,
     251,   253,   171,   172,   175,   176,   257,   258,   177,   178,
     261,   260,   259,   179,   181,   180,   262,   246,   240,   241,
     308,     1,   162,   194,   195,   196,   197,   198,   199,   200,
     202,   203,   243,   249,   292,   298,   348,   349,   350,   351,
     356,   357,   358,   364,   370,   303,   306,   289,   293,   312,
     312,   322,   310,   239,   240,   308,   293,   311,   241,   322,
     248,   333,   327,   337,   338,   243,   336,   239,   310,   239,
     219,   220,   221,   222,   224,   225,   226,   227,   323,   325,
     193,   244,   293,   297,   296,   193,   244,   327,   276,   240,
     240,   308,   170,   268,   292,   170,   290,   274,   240,   193,
     244,   290,   274,   274,   274,   278,   278,   279,   279,   280,
     280,   280,   280,   281,   281,   282,   283,   284,   285,   286,
     287,   292,   290,   293,   241,   244,   249,   247,   249,   249,
     249,   292,   162,   365,   366,   349,   239,   239,   244,   352,
     249,   244,   349,   242,   322,   248,   239,   242,   311,   248,
     318,   337,   310,   339,   340,   244,   338,   335,   337,   239,
     239,   239,   239,   239,   239,   239,   240,   246,   193,   244,
     308,   240,   240,   242,   247,   242,   293,   249,   249,   239,
     239,   367,   198,   292,   268,   356,   248,   315,   242,   319,
     243,   290,   341,   342,   244,   312,   322,   246,   249,   322,
     337,   244,   293,   162,   159,   293,   293,   293,   240,   324,
     240,   241,   290,   242,   292,     1,   348,   357,     1,   292,
     368,   369,   239,   240,   240,   244,   316,   342,   342,   241,
     245,   343,   344,   345,   347,   322,   322,   340,   244,   240,
     240,   240,   246,   246,   246,   325,   293,   240,   249,   249,
     249,   350,   353,   354,   292,   349,   363,   243,   361,   342,
     293,   170,   193,   244,   246,   342,   248,   346,   293,   293,
     293,   242,   353,   240,   292,   240,   292,   240,   201,   244,
     362,   242,   344,   347,   246,   246,   246,   240,   240,   249,
     349,   204,   205,   349,   359,   360,   342,   293,   293,   293,
     293,   247,   244,   360,   240,   240,   240,   247
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
#define YYERROR        goto yyerrlab1

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
# define YYLLOC_DEFAULT(Current, Rhs, N)         \
  Current.first_line   = Rhs[1].first_line;      \
  Current.first_column = Rhs[1].first_column;    \
  Current.last_line    = Rhs[N].last_line;       \
  Current.last_column  = Rhs[N].last_column;
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
| TOP (cinluded).                                                   |
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
  unsigned int yylineno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %u), ",
             yyrule - 1, yylineno);
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

#if YYMAXDEPTH == 0
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
#line 192 "gc_cl.y"
    { yyval.expr = clParseVariableIdentifier(Compiler, &yyvsp[0].token);
          if(yyval.expr == gcvNULL) {
             YYERROR;
          }
        ;}
    break;

  case 3:
#line 201 "gc_cl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 4:
#line 203 "gc_cl.y"
    { yyval.token = clParseCatStringLiteral(Compiler, &yyvsp[-1].token, &yyvsp[0].token); ;}
    break;

  case 5:
#line 208 "gc_cl.y"
    { yyval.expr = yyvsp[0].expr; ;}
    break;

  case 6:
#line 210 "gc_cl.y"
    { yyval.expr = clParseScalarConstant(Compiler, &yyvsp[0].token); ;}
    break;

  case 7:
#line 212 "gc_cl.y"
    { yyval.expr = clParseScalarConstant(Compiler, &yyvsp[0].token); ;}
    break;

  case 8:
#line 214 "gc_cl.y"
    { yyval.expr = clParseScalarConstant(Compiler, &yyvsp[0].token); ;}
    break;

  case 9:
#line 216 "gc_cl.y"
    { yyval.expr = clParseScalarConstant(Compiler, &yyvsp[0].token); ;}
    break;

  case 10:
#line 218 "gc_cl.y"
    { yyval.expr = clParseScalarConstant(Compiler, &yyvsp[0].token); ;}
    break;

  case 11:
#line 220 "gc_cl.y"
    { yyval.expr = clParseStringLiteral(Compiler, &yyvsp[0].token); ;}
    break;

  case 12:
#line 222 "gc_cl.y"
    { yyval.expr = yyvsp[-1].expr; ;}
    break;

  case 13:
#line 227 "gc_cl.y"
    { yyval.expr = yyvsp[0].expr; ;}
    break;

  case 14:
#line 229 "gc_cl.y"
    { yyval.expr = clParseSubscriptExpr(Compiler, yyvsp[-3].expr, yyvsp[-1].expr); ;}
    break;

  case 15:
#line 231 "gc_cl.y"
    { yyval.expr = clParseFuncCallExprAsExpr(Compiler, yyvsp[0].funcCall); ;}
    break;

  case 16:
#line 233 "gc_cl.y"
    { yyval.expr = clParseFieldSelectionExpr(Compiler, yyvsp[-2].expr, &yyvsp[0].token); ;}
    break;

  case 17:
#line 235 "gc_cl.y"
    { yyval.expr = clParsePtrFieldSelectionExpr(Compiler, yyvsp[-2].expr, &yyvsp[0].token); ;}
    break;

  case 18:
#line 237 "gc_cl.y"
    { yyval.expr = clParseIncOrDecExpr(Compiler, gcvNULL, clvUNARY_POST_INC, yyvsp[-1].expr); ;}
    break;

  case 19:
#line 239 "gc_cl.y"
    { yyval.expr = clParseIncOrDecExpr(Compiler, gcvNULL, clvUNARY_POST_DEC, yyvsp[-1].expr); ;}
    break;

  case 20:
#line 244 "gc_cl.y"
    { yyval.expr = yyvsp[0].expr; ;}
    break;

  case 21:
#line 249 "gc_cl.y"
    { yyval.funcCall = yyvsp[-1].funcCall; ;}
    break;

  case 22:
#line 251 "gc_cl.y"
    { yyval.funcCall = yyvsp[-2].funcCall; ;}
    break;

  case 23:
#line 253 "gc_cl.y"
    { yyval.funcCall = yyvsp[-1].funcCall; ;}
    break;

  case 24:
#line 258 "gc_cl.y"
    { yyval.funcCall = clParseFuncCallArgument(Compiler, yyvsp[-1].funcCall, yyvsp[0].expr); ;}
    break;

  case 25:
#line 260 "gc_cl.y"
    { yyval.funcCall = clParseFuncCallArgument(Compiler, yyvsp[-2].funcCall, yyvsp[0].expr); ;}
    break;

  case 26:
#line 265 "gc_cl.y"
    { yyval.decl = yyvsp[-1].decl; ;}
    break;

  case 27:
#line 267 "gc_cl.y"
    { yyval.decl = clParseCreateDecl(Compiler, &yyvsp[-2].decl, yyvsp[-1].typeQualifierList, gcvNULL); ;}
    break;

  case 28:
#line 272 "gc_cl.y"
    { yyval.funcCall = clParseFuncCallHeaderExpr(Compiler, &yyvsp[-1].token, gcvNULL); ;}
    break;

  case 29:
#line 277 "gc_cl.y"
    {
        clParseCastExprBegin(Compiler, &yyvsp[-1].decl);
        (void) gcoOS_ZeroMemory((gctPOINTER)&yyval.token, sizeof(clsLexToken));
        yyval.token.type = T_TYPE_CAST;
    ;}
    break;

  case 30:
#line 286 "gc_cl.y"
    { yyval.expr = yyvsp[0].expr; ;}
    break;

  case 31:
#line 288 "gc_cl.y"
    {
           clParseCastExprBegin(Compiler, gcvNULL);
               (void) gcoOS_ZeroMemory((gctPOINTER)&yyval.token, sizeof(clsLexToken));
           yyval.token.type = T_TYPE_CAST;
        ;}
    break;

  case 32:
#line 294 "gc_cl.y"
    { yyval.expr = clParseCastExprEnd(Compiler, &yyvsp[-2].decl, yyvsp[0].expr);
                ;}
    break;

  case 33:
#line 298 "gc_cl.y"
    { yyval.expr = clParseCastExprEnd(Compiler, gcvNULL, yyvsp[-1].expr);
                ;}
    break;

  case 34:
#line 302 "gc_cl.y"
    { yyval.expr = clParseCastExprEnd(Compiler, gcvNULL, yyvsp[-1].expr);
                ;}
    break;

  case 35:
#line 308 "gc_cl.y"
    { yyval.expr = yyvsp[0].expr; ;}
    break;

  case 36:
#line 310 "gc_cl.y"
    { yyval.expr = clParseIncOrDecExpr(Compiler, &yyvsp[-1].token, clvUNARY_PRE_INC, yyvsp[0].expr); ;}
    break;

  case 37:
#line 312 "gc_cl.y"
    { yyval.expr = clParseIncOrDecExpr(Compiler, &yyvsp[-1].token, clvUNARY_PRE_DEC, yyvsp[0].expr); ;}
    break;

  case 38:
#line 314 "gc_cl.y"
    { yyval.expr = clParseVecStep(Compiler, &yyvsp[-3].token, &yyvsp[-1].expr->decl); ;}
    break;

  case 39:
#line 316 "gc_cl.y"
    { yyval.expr = clParseVecStep(Compiler, &yyvsp[-1].token, &yyvsp[0].decl); ;}
    break;

  case 40:
#line 318 "gc_cl.y"
    { yyval.expr = clParseSizeofExpr(Compiler, &yyvsp[-1].token, yyvsp[0].expr); ;}
    break;

  case 41:
#line 320 "gc_cl.y"
    { yyval.expr = clParseSizeofTypeDecl(Compiler, &yyvsp[-1].token, &yyvsp[0].decl); ;}
    break;

  case 42:
#line 322 "gc_cl.y"
    { yyval.expr = clParseNormalUnaryExpr(Compiler, &yyvsp[-1].token, yyvsp[0].expr); ;}
    break;

  case 43:
#line 327 "gc_cl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 44:
#line 329 "gc_cl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 45:
#line 331 "gc_cl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 46:
#line 333 "gc_cl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 47:
#line 335 "gc_cl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 48:
#line 337 "gc_cl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 49:
#line 342 "gc_cl.y"
    {
           yyval.expr = yyvsp[0].expr;
        ;}
    break;

  case 50:
#line 346 "gc_cl.y"
    { yyval.expr = clParseNormalBinaryExpr(Compiler, yyvsp[-2].expr, &yyvsp[-1].token, yyvsp[0].expr); ;}
    break;

  case 51:
#line 348 "gc_cl.y"
    { yyval.expr = clParseNormalBinaryExpr(Compiler, yyvsp[-2].expr, &yyvsp[-1].token, yyvsp[0].expr); ;}
    break;

  case 52:
#line 350 "gc_cl.y"
    { yyval.expr = clParseNormalBinaryExpr(Compiler, yyvsp[-2].expr, &yyvsp[-1].token, yyvsp[0].expr); ;}
    break;

  case 53:
#line 355 "gc_cl.y"
    { yyval.expr = yyvsp[0].expr; ;}
    break;

  case 54:
#line 357 "gc_cl.y"
    { yyval.expr = clParseNormalBinaryExpr(Compiler, yyvsp[-2].expr, &yyvsp[-1].token, yyvsp[0].expr); ;}
    break;

  case 55:
#line 359 "gc_cl.y"
    { yyval.expr = clParseNormalBinaryExpr(Compiler, yyvsp[-2].expr, &yyvsp[-1].token, yyvsp[0].expr); ;}
    break;

  case 56:
#line 364 "gc_cl.y"
    { yyval.expr = yyvsp[0].expr; ;}
    break;

  case 57:
#line 366 "gc_cl.y"
    { yyval.expr = clParseNormalBinaryExpr(Compiler, yyvsp[-2].expr, &yyvsp[-1].token, yyvsp[0].expr); ;}
    break;

  case 58:
#line 368 "gc_cl.y"
    { yyval.expr = clParseNormalBinaryExpr(Compiler, yyvsp[-2].expr, &yyvsp[-1].token, yyvsp[0].expr); ;}
    break;

  case 59:
#line 373 "gc_cl.y"
    { yyval.expr = yyvsp[0].expr; ;}
    break;

  case 60:
#line 375 "gc_cl.y"
    { yyval.expr = clParseNormalBinaryExpr(Compiler, yyvsp[-2].expr, &yyvsp[-1].token, yyvsp[0].expr); ;}
    break;

  case 61:
#line 377 "gc_cl.y"
    { yyval.expr = clParseNormalBinaryExpr(Compiler, yyvsp[-2].expr, &yyvsp[-1].token, yyvsp[0].expr); ;}
    break;

  case 62:
#line 379 "gc_cl.y"
    { yyval.expr = clParseNormalBinaryExpr(Compiler, yyvsp[-2].expr, &yyvsp[-1].token, yyvsp[0].expr); ;}
    break;

  case 63:
#line 381 "gc_cl.y"
    { yyval.expr = clParseNormalBinaryExpr(Compiler, yyvsp[-2].expr, &yyvsp[-1].token, yyvsp[0].expr); ;}
    break;

  case 64:
#line 386 "gc_cl.y"
    { yyval.expr = yyvsp[0].expr; ;}
    break;

  case 65:
#line 388 "gc_cl.y"
    { yyval.expr = clParseNormalBinaryExpr(Compiler, yyvsp[-2].expr, &yyvsp[-1].token, yyvsp[0].expr); ;}
    break;

  case 66:
#line 390 "gc_cl.y"
    { yyval.expr = clParseNormalBinaryExpr(Compiler, yyvsp[-2].expr, &yyvsp[-1].token, yyvsp[0].expr); ;}
    break;

  case 67:
#line 395 "gc_cl.y"
    { yyval.expr = yyvsp[0].expr; ;}
    break;

  case 68:
#line 397 "gc_cl.y"
    { yyval.expr = clParseNormalBinaryExpr(Compiler, yyvsp[-2].expr, &yyvsp[-1].token, yyvsp[0].expr); ;}
    break;

  case 69:
#line 402 "gc_cl.y"
    { yyval.expr = yyvsp[0].expr; ;}
    break;

  case 70:
#line 404 "gc_cl.y"
    { yyval.expr = clParseNormalBinaryExpr(Compiler, yyvsp[-2].expr, &yyvsp[-1].token, yyvsp[0].expr); ;}
    break;

  case 71:
#line 409 "gc_cl.y"
    { yyval.expr = yyvsp[0].expr; ;}
    break;

  case 72:
#line 411 "gc_cl.y"
    { yyval.expr = clParseNormalBinaryExpr(Compiler, yyvsp[-2].expr, &yyvsp[-1].token, yyvsp[0].expr); ;}
    break;

  case 73:
#line 416 "gc_cl.y"
    { yyval.expr = yyvsp[0].expr; ;}
    break;

  case 74:
#line 418 "gc_cl.y"
    { yyval.expr = clParseNormalBinaryExpr(Compiler, yyvsp[-2].expr, &yyvsp[-1].token, yyvsp[0].expr); ;}
    break;

  case 75:
#line 423 "gc_cl.y"
    { yyval.expr = yyvsp[0].expr; ;}
    break;

  case 76:
#line 425 "gc_cl.y"
    { yyval.expr = clParseNormalBinaryExpr(Compiler, yyvsp[-2].expr, &yyvsp[-1].token, yyvsp[0].expr); ;}
    break;

  case 77:
#line 430 "gc_cl.y"
    { yyval.expr = yyvsp[0].expr; ;}
    break;

  case 78:
#line 432 "gc_cl.y"
    { yyval.expr = clParseNormalBinaryExpr(Compiler, yyvsp[-2].expr, &yyvsp[-1].token, yyvsp[0].expr); ;}
    break;

  case 79:
#line 437 "gc_cl.y"
    { yyval.expr = yyvsp[0].expr; ;}
    break;

  case 80:
#line 439 "gc_cl.y"
    { yyval.expr = clParseSelectionExpr(Compiler, yyvsp[-4].expr, yyvsp[-2].expr, yyvsp[0].expr); ;}
    break;

  case 81:
#line 444 "gc_cl.y"
    { yyval.expr = yyvsp[0].expr;;}
    break;

  case 82:
#line 446 "gc_cl.y"
    { yyval.expr = clParseAssignmentExpr(Compiler, yyvsp[-2].expr, &yyvsp[-1].token, yyvsp[0].expr); ;}
    break;

  case 83:
#line 451 "gc_cl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 84:
#line 453 "gc_cl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 85:
#line 455 "gc_cl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 86:
#line 457 "gc_cl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 87:
#line 459 "gc_cl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 88:
#line 461 "gc_cl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 89:
#line 463 "gc_cl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 90:
#line 465 "gc_cl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 91:
#line 467 "gc_cl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 92:
#line 469 "gc_cl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 93:
#line 471 "gc_cl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 94:
#line 476 "gc_cl.y"
    { yyval.expr = yyvsp[0].expr; ;}
    break;

  case 95:
#line 478 "gc_cl.y"
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
#line 491 "gc_cl.y"
    { yyval.expr = yyvsp[0].expr; ;}
    break;

  case 97:
#line 496 "gc_cl.y"
    { yyval.dataType = clParseTaggedDecl(Compiler, &yyvsp[-1].token, &yyvsp[0].token); ;}
    break;

  case 98:
#line 498 "gc_cl.y"
    { yyval.dataType = clParseTaggedDecl(Compiler, &yyvsp[-1].token, &yyvsp[0].token); ;}
    break;

  case 99:
#line 505 "gc_cl.y"
    { yyval.dataType = clParseEnumSpecifier(Compiler, &yyvsp[-5].token, yyvsp[-4].attr, &yyvsp[-3].token, (gctPOINTER)yyvsp[-1].enumeratorList); ;}
    break;

  case 100:
#line 508 "gc_cl.y"
    { yyval.dataType = clParseEnumSpecifier(Compiler, &yyvsp[-5].token, yyvsp[-4].attr, &yyvsp[-3].token, (gctPOINTER)yyvsp[-1].enumeratorList); ;}
    break;

  case 101:
#line 512 "gc_cl.y"
    { yyval.dataType = clParseEnumSpecifier(Compiler, &yyvsp[-4].token, gcvNULL, &yyvsp[-3].token, (gctPOINTER)yyvsp[-1].enumeratorList); ;}
    break;

  case 102:
#line 515 "gc_cl.y"
    { yyval.dataType = clParseEnumSpecifier(Compiler, &yyvsp[-4].token, gcvNULL, &yyvsp[-3].token, (gctPOINTER)yyvsp[-1].enumeratorList); ;}
    break;

  case 103:
#line 519 "gc_cl.y"
    { yyval.dataType = clParseEnumSpecifier(Compiler, &yyvsp[-4].token, yyvsp[-3].attr, gcvNULL, (gctPOINTER)yyvsp[-1].enumeratorList); ;}
    break;

  case 104:
#line 522 "gc_cl.y"
    { yyval.dataType = clParseEnumSpecifier(Compiler, &yyvsp[-4].token, yyvsp[-3].attr, gcvNULL, (gctPOINTER)yyvsp[-1].enumeratorList); ;}
    break;

  case 105:
#line 526 "gc_cl.y"
    { yyval.dataType = clParseEnumSpecifier(Compiler, &yyvsp[-3].token, gcvNULL, gcvNULL, (gctPOINTER)yyvsp[-1].enumeratorList); ;}
    break;

  case 106:
#line 529 "gc_cl.y"
    { yyval.dataType = clParseEnumSpecifier(Compiler, &yyvsp[-3].token, gcvNULL, gcvNULL, (gctPOINTER)yyvsp[-1].enumeratorList); ;}
    break;

  case 107:
#line 534 "gc_cl.y"
    {
           slsSLINK_LIST *enumList;

           slmSLINK_LIST_Initialize(enumList);
                   yyval.enumeratorList = clParseAddEnumerator(Compiler, yyvsp[0].enumeratorName, enumList);
        ;}
    break;

  case 108:
#line 541 "gc_cl.y"
    { yyval.enumeratorList = clParseAddEnumerator(Compiler, yyvsp[0].enumeratorName, yyvsp[-2].enumeratorList); ;}
    break;

  case 109:
#line 546 "gc_cl.y"
    { yyval.enumeratorName = clParseEnumerator(Compiler, &yyvsp[0].token, gcvNULL); ;}
    break;

  case 110:
#line 548 "gc_cl.y"
    { yyval.enumeratorName = clParseEnumerator(Compiler, &yyvsp[-2].token, yyvsp[0].expr); ;}
    break;

  case 111:
#line 553 "gc_cl.y"
    { yyval.statement = clParseDeclaration(Compiler, yyvsp[-1].declOrDeclList); ;}
    break;

  case 112:
#line 555 "gc_cl.y"
    { yyval.statement = clParseEnumTags(Compiler, yyvsp[-2].dataType, yyvsp[-1].attr); ;}
    break;

  case 113:
#line 557 "gc_cl.y"
    { yyval.statement = clParseTags(Compiler, yyvsp[-1].dataType); ;}
    break;

  case 114:
#line 559 "gc_cl.y"
    { yyval.statement = clParseTags(Compiler, yyvsp[-1].dataType); ;}
    break;

  case 115:
#line 564 "gc_cl.y"
    { yyval.funcName = yyvsp[-1].funcName; ;}
    break;

  case 116:
#line 569 "gc_cl.y"
    { yyval.funcName = yyvsp[0].funcName; ;}
    break;

  case 117:
#line 571 "gc_cl.y"
    { yyval.funcName = yyvsp[0].funcName; ;}
    break;

  case 118:
#line 576 "gc_cl.y"
    { yyval.funcName = clParseParameterList(Compiler, yyvsp[-1].funcName, yyvsp[0].paramName); ;}
    break;

  case 119:
#line 578 "gc_cl.y"
    { yyval.funcName = clParseParameterList(Compiler, yyvsp[-2].funcName, yyvsp[0].paramName); ;}
    break;

  case 120:
#line 583 "gc_cl.y"
    { yyval.funcName = clParseKernelFuncHeader(Compiler, yyvsp[-4].attr, &yyvsp[-2].decl, &yyvsp[-1].token); ;}
    break;

  case 121:
#line 585 "gc_cl.y"
    { yyval.funcName = clParseKernelFuncHeader(Compiler, yyvsp[-3].attr, &yyvsp[-2].decl, &yyvsp[-1].token); ;}
    break;

  case 122:
#line 587 "gc_cl.y"
    { yyval.funcName = clParseExternKernelFuncHeader(Compiler, yyvsp[-3].attr, &yyvsp[-2].decl, &yyvsp[-1].token); ;}
    break;

  case 123:
#line 589 "gc_cl.y"
    { yyval.funcName = clParseFuncHeader(Compiler, &yyvsp[-2].decl, &yyvsp[-1].token); ;}
    break;

  case 124:
#line 591 "gc_cl.y"
    { yyval.funcName = clParseFuncHeaderWithAttr(Compiler, yyvsp[-3].attr, &yyvsp[-2].decl, &yyvsp[-1].token); ;}
    break;

  case 125:
#line 593 "gc_cl.y"
    { yyval.funcName = clParseFuncHeader(Compiler, &yyvsp[-2].decl, &yyvsp[-1].token);
          if(yyval.funcName) yyval.funcName->u.funcInfo.isInline = gcvTRUE;
        ;}
    break;

  case 126:
#line 597 "gc_cl.y"
    { yyval.funcName = clParseFuncHeader(Compiler, &yyvsp[-2].decl, &yyvsp[-1].token);
          if(yyval.funcName) yyval.funcName->u.funcInfo.isInline = gcvTRUE;
        ;}
    break;

  case 127:
#line 604 "gc_cl.y"
    { yyval.paramName = clParseParameterDecl(Compiler, &yyvsp[-1].decl, &yyvsp[0].token); ;}
    break;

  case 128:
#line 606 "gc_cl.y"
    { yyval.paramName = clParseArrayParameterDecl(Compiler, &yyvsp[-2].decl, &yyvsp[-1].token, yyvsp[0].expr); ;}
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
    { yyval.paramName = clParseQualifiedParameterDecl(Compiler, gcvNULL, gcvNULL, yyvsp[0].paramName); ;}
    break;

  case 132:
#line 617 "gc_cl.y"
    { yyval.paramName = clParseQualifiedParameterDecl(Compiler, gcvNULL, &yyvsp[-1].token, yyvsp[0].paramName); ;}
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
#line 623 "gc_cl.y"
    { yyval.paramName = clParseQualifiedParameterDecl(Compiler, yyvsp[-1].typeQualifierList, gcvNULL, yyvsp[0].paramName); ;}
    break;

  case 136:
#line 625 "gc_cl.y"
    { yyval.paramName = clParseQualifiedParameterDecl(Compiler, yyvsp[-2].typeQualifierList, &yyvsp[-1].token, yyvsp[0].paramName); ;}
    break;

  case 137:
#line 630 "gc_cl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 138:
#line 632 "gc_cl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 139:
#line 637 "gc_cl.y"
    { yyval.paramName = clParseParameterDecl(Compiler, &yyvsp[0].decl, gcvNULL); ;}
    break;

  case 140:
#line 639 "gc_cl.y"
    { yyval.paramName = clParseArrayParameterDecl(Compiler, &yyvsp[-3].decl, gcvNULL, yyvsp[-1].expr); ;}
    break;

  case 141:
#line 644 "gc_cl.y"
    {
            yyval.typeQualifierList = clParseTypeQualifierList(Compiler, &yyvsp[0].token,
                                          clParseEmptyTypeQualifierList(Compiler));
        ;}
    break;

  case 142:
#line 649 "gc_cl.y"
    {yyval.typeQualifierList = clParseTypeQualifierList(Compiler, &yyvsp[-1].token, yyvsp[0].typeQualifierList); ;}
    break;

  case 143:
#line 654 "gc_cl.y"
    { yyval.typeQualifierList = clParseEmptyTypeQualifierList(Compiler); ;}
    break;

  case 144:
#line 656 "gc_cl.y"
    { yyval.typeQualifierList = clParsePointerTypeQualifier(Compiler, gcvNULL, yyvsp[0].typeQualifierList); ;}
    break;

  case 145:
#line 658 "gc_cl.y"
    {yyval.typeQualifierList = yyvsp[0].typeQualifierList;;}
    break;

  case 146:
#line 660 "gc_cl.y"
    {yyval.typeQualifierList = clParsePointerTypeQualifier(Compiler, yyvsp[-1].typeQualifierList, yyvsp[0].typeQualifierList); ;}
    break;

  case 147:
#line 664 "gc_cl.y"
    {yyval.token = yyvsp[0].token;;}
    break;

  case 148:
#line 666 "gc_cl.y"
    {
          yyval.token = yyvsp[0].token;
          yyval.token.u.identifier.ptrDscr = yyvsp[-1].typeQualifierList;
        ;}
    break;

  case 149:
#line 673 "gc_cl.y"
    {  yyval.token = yyvsp[0].token; ;}
    break;

  case 150:
#line 675 "gc_cl.y"
    { yyval.token = yyvsp[-1].token; ;}
    break;

  case 151:
#line 679 "gc_cl.y"
    { yyval.expr = clParseNullExpr(Compiler, &yyvsp[0].token); ;}
    break;

  case 152:
#line 681 "gc_cl.y"
    { yyval.expr = yyvsp[0].expr; ;}
    break;

  case 153:
#line 686 "gc_cl.y"
    { yyval.expr = yyvsp[-1].expr; ;}
    break;

  case 154:
#line 688 "gc_cl.y"
    { yyval.expr = clParseArrayDeclarator(Compiler, &yyvsp[-2].token, yyvsp[-3].expr, yyvsp[-1].expr); ;}
    break;

  case 155:
#line 693 "gc_cl.y"
    { yyval.declOrDeclList = yyvsp[0].declOrDeclList; ;}
    break;

  case 156:
#line 695 "gc_cl.y"
    {
               cloCOMPILER_PushParserState(Compiler, clvPARSER_IN_TYPEDEF);
        ;}
    break;

  case 157:
#line 699 "gc_cl.y"
    {
           yyval.declOrDeclList = clParseTypeDef(Compiler, yyvsp[0].declOrDeclList);
           cloCOMPILER_PopParserState(Compiler);
        ;}
    break;

  case 158:
#line 704 "gc_cl.y"
    { yyval.declOrDeclList = clParseVariableDeclList(Compiler, yyvsp[-3].declOrDeclList, &yyvsp[-1].token, yyvsp[0].attr); ;}
    break;

  case 159:
#line 706 "gc_cl.y"
    { yyval.declOrDeclList = clParseArrayVariableDeclList(Compiler, yyvsp[-4].declOrDeclList, &yyvsp[-2].token, yyvsp[-1].expr, yyvsp[0].attr); ;}
    break;

  case 160:
#line 708 "gc_cl.y"
    { yyval.declOrDeclList = clParseVariableDeclListInit(Compiler, yyvsp[-4].declOrDeclList, &yyvsp[-2].token, yyvsp[-1].attr); ;}
    break;

  case 161:
#line 710 "gc_cl.y"
    { yyval.declOrDeclList = clParseFinishDeclListInit(Compiler, yyvsp[-1].declOrDeclList, yyvsp[0].expr); ;}
    break;

  case 162:
#line 712 "gc_cl.y"
    { yyval.declOrDeclList = clParseArrayVariableDeclListInit(Compiler, yyvsp[-5].declOrDeclList, &yyvsp[-3].token, yyvsp[-2].expr, yyvsp[-1].attr); ;}
    break;

  case 163:
#line 714 "gc_cl.y"
    { yyval.declOrDeclList = clParseFinishDeclListInit(Compiler, yyvsp[-1].declOrDeclList, yyvsp[0].expr); ;}
    break;

  case 164:
#line 720 "gc_cl.y"
    { yyval.declOrDeclList = clParseFuncDecl(Compiler, yyvsp[0].funcName); ;}
    break;

  case 165:
#line 722 "gc_cl.y"
    { yyval.declOrDeclList = clParseVariableDecl(Compiler, &yyvsp[-2].decl, &yyvsp[-1].token, yyvsp[0].attr); ;}
    break;

  case 166:
#line 724 "gc_cl.y"
    { yyval.declOrDeclList = clParseArrayVariableDecl(Compiler, &yyvsp[-3].decl, &yyvsp[-2].token, yyvsp[-1].expr, yyvsp[0].attr); ;}
    break;

  case 167:
#line 726 "gc_cl.y"
    { yyval.declOrDeclList = clParseVariableDeclInit(Compiler, &yyvsp[-3].decl, &yyvsp[-2].token, yyvsp[-1].attr); ;}
    break;

  case 168:
#line 728 "gc_cl.y"
    { yyval.declOrDeclList = clParseFinishDeclInit(Compiler, yyvsp[-1].declOrDeclList, yyvsp[0].expr); ;}
    break;

  case 169:
#line 730 "gc_cl.y"
    { yyval.declOrDeclList = clParseArrayVariableDeclInit(Compiler, &yyvsp[-4].decl, &yyvsp[-3].token, yyvsp[-2].expr, yyvsp[-1].attr); ;}
    break;

  case 170:
#line 732 "gc_cl.y"
    { yyval.declOrDeclList = clParseFinishDeclInit(Compiler, yyvsp[-1].declOrDeclList, yyvsp[0].expr); ;}
    break;

  case 171:
#line 739 "gc_cl.y"
    { yyval.attr = gcvNULL; ;}
    break;

  case 172:
#line 741 "gc_cl.y"
    { yyval.attr = yyvsp[-2].attr; ;}
    break;

  case 173:
#line 745 "gc_cl.y"
    { yyval.attr = gcvNULL; ;}
    break;

  case 174:
#line 747 "gc_cl.y"
    { yyval.attr = yyvsp[0].attr; ;}
    break;

  case 175:
#line 751 "gc_cl.y"
    { yyval.attr = yyvsp[0].attr; ;}
    break;

  case 176:
#line 753 "gc_cl.y"
    { yyval.attr = yyvsp[0].attr; ;}
    break;

  case 177:
#line 755 "gc_cl.y"
    { yyval.attr = yyvsp[-1].attr; ;}
    break;

  case 178:
#line 757 "gc_cl.y"
    { yyval.attr = yyvsp[0].attr; ;}
    break;

  case 179:
#line 762 "gc_cl.y"
    { yyval.attr = clParseAttributeEndianType(Compiler, yyvsp[-4].attr,  &yyvsp[-1].token); ;}
    break;

  case 180:
#line 764 "gc_cl.y"
    { yyval.attr = clParseSimpleAttribute(Compiler, &yyvsp[0].token, clvATTR_PACKED, yyvsp[-1].attr); ;}
    break;

  case 181:
#line 766 "gc_cl.y"
    { yyval.attr = clParseAttributeVecTypeHint(Compiler, yyvsp[-4].attr, &yyvsp[-1].token); ;}
    break;

  case 182:
#line 768 "gc_cl.y"
    { yyval.attr = clParseAttributeReqdWorkGroupSize(Compiler, yyvsp[-8].attr, yyvsp[-5].expr, yyvsp[-3].expr, yyvsp[-1].expr); ;}
    break;

  case 183:
#line 770 "gc_cl.y"
    { yyval.attr = clParseAttributeWorkGroupSizeHint(Compiler, yyvsp[-8].attr, yyvsp[-5].expr, yyvsp[-3].expr, yyvsp[-1].expr); ;}
    break;

  case 184:
#line 772 "gc_cl.y"
    { yyval.attr = clParseAttributeKernelScaleHint(Compiler, yyvsp[-8].attr, yyvsp[-5].expr, yyvsp[-3].expr, yyvsp[-1].expr); ;}
    break;

  case 185:
#line 774 "gc_cl.y"
    { yyval.attr = clParseAttributeAligned(Compiler, yyvsp[-1].attr, gcvNULL); ;}
    break;

  case 186:
#line 776 "gc_cl.y"
    { yyval.attr = clParseAttributeAligned(Compiler, yyvsp[-4].attr, yyvsp[-1].expr); ;}
    break;

  case 187:
#line 778 "gc_cl.y"
    { yyval.attr = clParseSimpleAttribute(Compiler, &yyvsp[0].token, clvATTR_ALWAYS_INLINE, yyvsp[-1].attr); ;}
    break;

  case 188:
#line 783 "gc_cl.y"
    { yyval.decl = clParseCreateDecl(Compiler, &yyvsp[0].decl, gcvNULL, gcvNULL); ;}
    break;

  case 189:
#line 785 "gc_cl.y"
    { yyval.decl = clParseCreateDecl(Compiler, &yyvsp[-3].decl, gcvNULL, yyvsp[-1].expr); ;}
    break;

  case 190:
#line 787 "gc_cl.y"
    { yyval.decl = clParseCreateDecl(Compiler, &yyvsp[-1].decl, yyvsp[0].typeQualifierList, gcvNULL); ;}
    break;

  case 191:
#line 789 "gc_cl.y"
    { yyval.decl = clParseCreateDecl(Compiler, &yyvsp[-4].decl, yyvsp[-3].typeQualifierList, yyvsp[-1].expr); ;}
    break;

  case 192:
#line 791 "gc_cl.y"
    { yyval.decl = clParseCreateDecl(Compiler, &yyvsp[-5].decl, yyvsp[-4].typeQualifierList, yyvsp[-1].expr); ;}
    break;

  case 193:
#line 793 "gc_cl.y"
    { yyval.decl = clParseCreateDeclFromExpression(Compiler, yyvsp[0].expr); ;}
    break;

  case 194:
#line 798 "gc_cl.y"
    { yyval.decl = clParseQualifiedType(Compiler, gcvNULL, gcvFALSE, &yyvsp[0].decl); ;}
    break;

  case 195:
#line 800 "gc_cl.y"
    { yyval.decl = clParseQualifiedType(Compiler, yyvsp[-1].typeQualifierList, gcvFALSE, &yyvsp[0].decl); ;}
    break;

  case 196:
#line 802 "gc_cl.y"
    { yyval.decl = yyvsp[-1].decl; ;}
    break;

  case 197:
#line 807 "gc_cl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 198:
#line 809 "gc_cl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 199:
#line 811 "gc_cl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 200:
#line 813 "gc_cl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 201:
#line 815 "gc_cl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 202:
#line 817 "gc_cl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 203:
#line 819 "gc_cl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 204:
#line 821 "gc_cl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 205:
#line 823 "gc_cl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 206:
#line 825 "gc_cl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 207:
#line 830 "gc_cl.y"
    { yyval.decl = clParseNonStructType(Compiler, &yyvsp[0].token); ;}
    break;

  case 208:
#line 832 "gc_cl.y"
    { yyval.decl = clParseCreateDeclFromDataType(Compiler, yyvsp[0].dataType); ;}
    break;

  case 209:
#line 834 "gc_cl.y"
    { yyval.decl = clParseCreateDeclFromDataType(Compiler, yyvsp[0].dataType); ;}
    break;

  case 210:
#line 836 "gc_cl.y"
    { yyval.decl = clParseCreateDeclFromDataType(Compiler, yyvsp[0].dataType); ;}
    break;

  case 211:
#line 841 "gc_cl.y"
    { yyval.token = yyvsp[0].token;}
    break;

  case 212:
#line 843 "gc_cl.y"
    { yyval.token = yyvsp[0].token;}
    break;

  case 213:
#line 845 "gc_cl.y"
    { yyval.token = yyvsp[0].token;}
    break;

  case 214:
#line 847 "gc_cl.y"
    { yyval.token = yyvsp[0].token;}
    break;

  case 215:
#line 849 "gc_cl.y"
    { yyval.token = yyvsp[0].token;}
    break;

  case 216:
#line 851 "gc_cl.y"
    { yyval.token = yyvsp[0].token;}
    break;

  case 217:
#line 853 "gc_cl.y"
    { yyval.token = yyvsp[0].token;}
    break;

  case 218:
#line 855 "gc_cl.y"
    { yyval.token = yyvsp[0].token;}
    break;

  case 219:
#line 857 "gc_cl.y"
    { yyval.token = yyvsp[0].token;}
    break;

  case 220:
#line 859 "gc_cl.y"
    { yyval.token = yyvsp[0].token;}
    break;

  case 221:
#line 861 "gc_cl.y"
    { yyval.token = yyvsp[0].token;}
    break;

  case 222:
#line 863 "gc_cl.y"
    { yyval.token = yyvsp[0].token;}
    break;

  case 223:
#line 865 "gc_cl.y"
    { yyval.token = yyvsp[0].token;}
    break;

  case 224:
#line 867 "gc_cl.y"
    { yyval.token = yyvsp[0].token;}
    break;

  case 225:
#line 869 "gc_cl.y"
    { yyval.token = yyvsp[0].token;}
    break;

  case 226:
#line 871 "gc_cl.y"
    { yyval.token = yyvsp[0].token;}
    break;

  case 227:
#line 873 "gc_cl.y"
    { yyval.token = yyvsp[0].token;}
    break;

  case 228:
#line 875 "gc_cl.y"
    { yyval.token = yyvsp[0].token;}
    break;

  case 229:
#line 877 "gc_cl.y"
    { yyval.token = yyvsp[0].token;}
    break;

  case 230:
#line 879 "gc_cl.y"
    { yyval.token = yyvsp[0].token;}
    break;

  case 231:
#line 881 "gc_cl.y"
    { yyval.token = yyvsp[0].token;}
    break;

  case 232:
#line 883 "gc_cl.y"
    { yyval.token = yyvsp[0].token;}
    break;

  case 233:
#line 885 "gc_cl.y"
    { yyval.token = yyvsp[0].token;}
    break;

  case 234:
#line 887 "gc_cl.y"
    { yyval.token = yyvsp[0].token;}
    break;

  case 235:
#line 891 "gc_cl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 236:
#line 893 "gc_cl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 237:
#line 898 "gc_cl.y"
    { clParseStructDeclBegin(Compiler, &yyvsp[-1].token); ;}
    break;

  case 238:
#line 900 "gc_cl.y"
    { yyval.dataType = clParseStructDeclEnd(Compiler, &yyvsp[-6].token, &yyvsp[-5].token, yyvsp[0].attr, yyvsp[-2].status); ;}
    break;

  case 239:
#line 902 "gc_cl.y"
    { clParseStructDeclBegin(Compiler, gcvNULL); ;}
    break;

  case 240:
#line 904 "gc_cl.y"
    { yyval.dataType = clParseStructDeclEnd(Compiler, &yyvsp[-5].token, gcvNULL, yyvsp[0].attr, yyvsp[-2].status); ;}
    break;

  case 241:
#line 906 "gc_cl.y"
    { clParseStructDeclBegin(Compiler, &yyvsp[-1].token); ;}
    break;

  case 242:
#line 908 "gc_cl.y"
    { yyval.dataType = clParseStructDeclEnd(Compiler, &yyvsp[-6].token, &yyvsp[-4].token, yyvsp[-5].attr, yyvsp[-1].status); ;}
    break;

  case 243:
#line 910 "gc_cl.y"
    { clParseStructDeclBegin(Compiler, gcvNULL); ;}
    break;

  case 244:
#line 912 "gc_cl.y"
    { yyval.dataType = clParseStructDeclEnd(Compiler, &yyvsp[-5].token, gcvNULL, yyvsp[-4].attr, yyvsp[-1].status); ;}
    break;

  case 245:
#line 917 "gc_cl.y"
    {
           yyval.status = yyvsp[0].status;
        ;}
    break;

  case 246:
#line 921 "gc_cl.y"
    {
           if(gcmIS_ERROR(yyvsp[-1].status)) {
                       yyval.status = yyvsp[-1].status;
           }
           else {
                       yyval.status = yyvsp[0].status;

           }
        ;}
    break;

  case 247:
#line 934 "gc_cl.y"
    {
                   yyval.status = clParseTypeSpecifiedFieldDeclList(Compiler, &yyvsp[-2].decl, yyvsp[-1].fieldDeclList);
        ;}
    break;

  case 248:
#line 941 "gc_cl.y"
    { yyval.fieldDeclList = clParseFieldDeclList(Compiler, yyvsp[0].fieldDecl); ;}
    break;

  case 249:
#line 943 "gc_cl.y"
    { yyval.fieldDeclList = clParseFieldDeclList2(Compiler, yyvsp[-2].fieldDeclList, yyvsp[0].fieldDecl); ;}
    break;

  case 250:
#line 948 "gc_cl.y"
    { yyval.fieldDecl = clParseFieldDecl(Compiler, &yyvsp[-1].token, gcvNULL, yyvsp[0].attr); ;}
    break;

  case 251:
#line 950 "gc_cl.y"
    { yyval.fieldDecl = clParseFieldDecl(Compiler, &yyvsp[-2].token, yyvsp[-1].expr, yyvsp[0].attr); ;}
    break;

  case 252:
#line 955 "gc_cl.y"
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

  case 253:
#line 977 "gc_cl.y"
    { yyval.expr = yyvsp[0].expr; ;}
    break;

  case 254:
#line 979 "gc_cl.y"
    { yyval.expr = yyvsp[-1].expr; ;}
    break;

  case 255:
#line 981 "gc_cl.y"
    { yyval.expr = yyvsp[-1].expr; ;}
    break;

  case 256:
#line 986 "gc_cl.y"
    { yyval.expr = clParseInitializerList(Compiler, yyvsp[-2].expr, yyvsp[-1].declOrDeclList, yyvsp[0].expr); ;}
    break;

  case 257:
#line 988 "gc_cl.y"
    { yyval.expr = clParseInitializerList(Compiler, yyvsp[-3].expr, yyvsp[-1].declOrDeclList, yyvsp[0].expr); ;}
    break;

  case 258:
#line 992 "gc_cl.y"
    { yyval.declOrDeclList = gcvNULL; ;}
    break;

  case 259:
#line 994 "gc_cl.y"
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

  case 260:
#line 1007 "gc_cl.y"
    { yyval.declOrDeclList = yyvsp[0].declOrDeclList; ;}
    break;

  case 261:
#line 1009 "gc_cl.y"
    {
           yyval.token.type = T_EOF;
        ;}
    break;

  case 262:
#line 1013 "gc_cl.y"
    { yyval.declOrDeclList = yyvsp[0].declOrDeclList; ;}
    break;

  case 263:
#line 1018 "gc_cl.y"
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

  case 264:
#line 1031 "gc_cl.y"
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

  case 265:
#line 1047 "gc_cl.y"
    { yyval.statement = yyvsp[0].statement; ;}
    break;

  case 266:
#line 1052 "gc_cl.y"
    { yyval.statement = clParseCompoundStatementAsStatement(Compiler, yyvsp[0].statements); ;}
    break;

  case 267:
#line 1054 "gc_cl.y"
    { yyval.statement = yyvsp[0].statement; ;}
    break;

  case 268:
#line 1059 "gc_cl.y"
    { yyval.statement = yyvsp[0].statement; ;}
    break;

  case 269:
#line 1061 "gc_cl.y"
    { yyval.statement = yyvsp[0].statement; ;}
    break;

  case 270:
#line 1063 "gc_cl.y"
    { yyval.statement = yyvsp[0].statement; ;}
    break;

  case 271:
#line 1065 "gc_cl.y"
    { yyval.statement = yyvsp[0].statement; ;}
    break;

  case 272:
#line 1067 "gc_cl.y"
    { yyval.statement = yyvsp[0].statement; ;}
    break;

  case 273:
#line 1069 "gc_cl.y"
    { yyval.statement = clParseStatementLabel(Compiler, &yyvsp[-1].token); ;}
    break;

  case 274:
#line 1071 "gc_cl.y"
    { yyclearin;
          yyerrok;
          yyval.statement = gcvNULL; ;}
    break;

  case 275:
#line 1075 "gc_cl.y"
    { yyclearin;
          yyerrok;
          yyval.statement = gcvNULL; ;}
    break;

  case 276:
#line 1082 "gc_cl.y"
    { yyval.statements = gcvNULL; ;}
    break;

  case 277:
#line 1084 "gc_cl.y"
    { clParseCompoundStatementBegin(Compiler); ;}
    break;

  case 278:
#line 1086 "gc_cl.y"
    { yyval.statements = clParseCompoundStatementEnd(Compiler, &yyvsp[-3].token, yyvsp[-1].statements); ;}
    break;

  case 279:
#line 1091 "gc_cl.y"
    { yyval.statement = clParseCompoundStatementNoNewScopeAsStatementNoNewScope(Compiler, yyvsp[0].statements); ;}
    break;

  case 280:
#line 1093 "gc_cl.y"
    { yyval.statement = yyvsp[0].statement; ;}
    break;

  case 281:
#line 1098 "gc_cl.y"
    { yyval.statements = gcvNULL; ;}
    break;

  case 282:
#line 1100 "gc_cl.y"
    { clParseCompoundStatementNoNewScopeBegin(Compiler); ;}
    break;

  case 283:
#line 1102 "gc_cl.y"
    { yyval.statements = clParseCompoundStatementNoNewScopeEnd(Compiler, &yyvsp[-3].token, yyvsp[-1].statements); ;}
    break;

  case 284:
#line 1107 "gc_cl.y"
    { yyval.statements = clParseStatementList(Compiler, yyvsp[0].statement); ;}
    break;

  case 285:
#line 1109 "gc_cl.y"
    { yyval.statements = clParseStatementList2(Compiler, yyvsp[-1].statements, yyvsp[0].statement); ;}
    break;

  case 286:
#line 1114 "gc_cl.y"
    { yyval.statement = gcvNULL; ;}
    break;

  case 287:
#line 1116 "gc_cl.y"
    { yyval.statement = clParseExprAsStatement(Compiler, yyvsp[-1].expr); ;}
    break;

  case 288:
#line 1121 "gc_cl.y"
    { yyval.statement = clParseIfStatement(Compiler, &yyvsp[-4].token, yyvsp[-2].expr, yyvsp[0].ifStatementPair); ;}
    break;

  case 289:
#line 1123 "gc_cl.y"
    { yyval.statement = clParseSwitchStatement(Compiler, &yyvsp[-4].token, yyvsp[-2].expr, yyvsp[0].statement); ;}
    break;

  case 290:
#line 1128 "gc_cl.y"
    { yyval.statements = clParseStatementList(Compiler, yyvsp[0].statement); ;}
    break;

  case 291:
#line 1130 "gc_cl.y"
    { yyval.statements = clParseStatementList2(Compiler, yyvsp[-1].statements, yyvsp[0].statement); ;}
    break;

  case 292:
#line 1135 "gc_cl.y"
    { yyval.statement = yyvsp[0].statement; ;}
    break;

  case 293:
#line 1137 "gc_cl.y"
    { yyval.statement = clParseCaseStatement(Compiler, &yyvsp[-2].token, yyvsp[-1].expr); ;}
    break;

  case 294:
#line 1139 "gc_cl.y"
    { yyval.statement = clParseDefaultStatement(Compiler, &yyvsp[-1].token); ;}
    break;

  case 295:
#line 1144 "gc_cl.y"
    { yyval.statement = gcvNULL; ;}
    break;

  case 296:
#line 1146 "gc_cl.y"
    { clParseSwitchBodyBegin(Compiler); ;}
    break;

  case 297:
#line 1148 "gc_cl.y"
    { yyval.statement = clParseSwitchBodyEnd(Compiler, &yyvsp[-3].token, yyvsp[-1].statements); ;}
    break;

  case 298:
#line 1153 "gc_cl.y"
    { yyval.ifStatementPair = clParseIfSubStatements(Compiler, yyvsp[-2].statement, yyvsp[0].statement); ;}
    break;

  case 299:
#line 1155 "gc_cl.y"
    { yyval.ifStatementPair = clParseIfSubStatements(Compiler, yyvsp[0].statement, gcvNULL); ;}
    break;

  case 300:
#line 1162 "gc_cl.y"
    { clParseWhileStatementBegin(Compiler); ;}
    break;

  case 301:
#line 1164 "gc_cl.y"
    { yyval.statement = clParseWhileStatementEnd(Compiler, &yyvsp[-5].token, yyvsp[-2].expr, yyvsp[0].statement); ;}
    break;

  case 302:
#line 1166 "gc_cl.y"
    { yyval.statement = clParseDoWhileStatement(Compiler, &yyvsp[-6].token, yyvsp[-5].statement, yyvsp[-2].expr); ;}
    break;

  case 303:
#line 1168 "gc_cl.y"
    { clParseForStatementBegin(Compiler); ;}
    break;

  case 304:
#line 1170 "gc_cl.y"
    { yyval.statement = clParseForStatementEnd(Compiler, &yyvsp[-4].token, yyvsp[-2].statement, yyvsp[-1].forExprPair, yyvsp[0].statement); ;}
    break;

  case 305:
#line 1175 "gc_cl.y"
    { yyval.statement = yyvsp[0].statement; ;}
    break;

  case 306:
#line 1177 "gc_cl.y"
    { yyval.statement = yyvsp[0].statement; ;}
    break;

  case 307:
#line 1179 "gc_cl.y"
    { yyclearin;
          yyerrok;
          yyval.statement = gcvNULL; ;}
    break;

  case 308:
#line 1186 "gc_cl.y"
    { yyval.expr = yyvsp[0].expr; ;}
    break;

  case 309:
#line 1188 "gc_cl.y"
    { yyval.expr = gcvNULL; ;}
    break;

  case 310:
#line 1193 "gc_cl.y"
    { yyval.forExprPair = clParseForControl(Compiler, yyvsp[-2].expr, gcvNULL); ;}
    break;

  case 311:
#line 1195 "gc_cl.y"
    { yyval.forExprPair = clParseForControl(Compiler, yyvsp[-3].expr, yyvsp[-1].expr); ;}
    break;

  case 312:
#line 1197 "gc_cl.y"
    {
          clsForExprPair nullPair = {gcvNULL, gcvNULL};
          yyclearin;
          yyerrok;
          yyval.forExprPair = nullPair; ;}
    break;

  case 313:
#line 1203 "gc_cl.y"
    {
          clsForExprPair nullPair = {gcvNULL, gcvNULL};
          yyclearin;
          yyerrok;
          yyval.forExprPair = nullPair; ;}
    break;

  case 314:
#line 1212 "gc_cl.y"
    { yyval.statement = clParseJumpStatement(Compiler, clvCONTINUE, &yyvsp[-1].token, gcvNULL); ;}
    break;

  case 315:
#line 1214 "gc_cl.y"
    { yyval.statement = clParseJumpStatement(Compiler, clvBREAK, &yyvsp[-1].token, gcvNULL); ;}
    break;

  case 316:
#line 1216 "gc_cl.y"
    { yyval.statement = clParseJumpStatement(Compiler, clvRETURN, &yyvsp[-1].token, gcvNULL); ;}
    break;

  case 317:
#line 1218 "gc_cl.y"
    { yyval.statement = clParseJumpStatement(Compiler, clvRETURN, &yyvsp[-2].token, yyvsp[-1].expr); ;}
    break;

  case 318:
#line 1220 "gc_cl.y"
    { yyval.statement = clParseGotoStatement(Compiler, &yyvsp[-2].token, &yyvsp[-1].token); ;}
    break;

  case 322:
#line 1232 "gc_cl.y"
    { clParseExternalDecl(Compiler, yyvsp[0].statement); ;}
    break;

  case 324:
#line 1238 "gc_cl.y"
    { clParseFuncDef(Compiler, yyvsp[-1].funcName, yyvsp[0].statements); ;}
    break;


    }

/* Line 991 of yacc.c.  */
#line 4434 "gc_cl_parser.c"

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
      char *yymsg;
      int yyx, yycount;

      yycount = 0;
      /* Start YYX at -YYN if negative to avoid negative indexes in
         YYCHECK.  */
      for (yyx = yyn < 0 ? -yyn : 0;
           yyx < (int) (sizeof (yytname) / sizeof (char *)); yyx++)
        if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
          yysize += yystrlen (yytname[yyx]) + 15, yycount++;
      yysize += yystrlen ("syntax error, unexpected ") + 1;
      yysize += yystrlen (yytname[yytype]);
      yymsg = (char *) YYSTACK_ALLOC (yysize);
      if (yymsg != 0)
        {
          char *yyp = yystpcpy (yymsg, "syntax error, unexpected ");
          yyp = yystpcpy (yyp, yytname[yytype]);

          if (yycount < 5)
        {
          yycount = 0;
          for (yyx = yyn < 0 ? -yyn : 0;
               yyx < (int) (sizeof (yytname) / sizeof (char *));
               yyx++)
            if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
              {
            const char *yyq = ! yycount ? ", expecting " : " or ";
            yyp = yystpcpy (yyp, yyq);
            yyp = yystpcpy (yyp, yytname[yyx]);
            yycount++;
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

      /* Return failure if at end of input.  */
      if (yychar == YYEOF)
        {
      /* Pop the error token.  */
          YYPOPSTACK;
      /* Pop the rest of the stack.  */
      while (yyss < yyssp)
        {
          YYDSYMPRINTF ("Error: popping", yystos[*yyssp], yyvsp, yylsp);
          yydestruct (yystos[*yyssp], yyvsp);
          YYPOPSTACK;
        }
      YYABORT;
        }

      YYDSYMPRINTF ("Error: discarding", yytoken, &yylval, &yylloc);
      yydestruct (yytoken, &yylval);
      yychar = YYEMPTY;

    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab2;


/*----------------------------------------------------.
| yyerrlab1 -- error raised explicitly by an action.  |
`----------------------------------------------------*/
yyerrlab1:

  /* Suppress GCC warning that yyerrlab1 is unused when no action
     invokes YYERROR.  Doesn't work in C++ */
#ifndef __cplusplus
#if defined (__GNUC_MINOR__) && 2093 <= (__GNUC__ * 1000 + __GNUC_MINOR__)
  __attribute__ ((__unused__))
#endif
#endif


  goto yyerrlab2;


/*---------------------------------------------------------------.
| yyerrlab2 -- pop states until the error token can be shifted.  |
`---------------------------------------------------------------*/
yyerrlab2:
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
      yyvsp--;
      yystate = *--yyssp;

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


#line 1241 "gc_cl.y"



