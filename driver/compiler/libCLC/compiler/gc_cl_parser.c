/****************************************************************************
*
*    Copyright (c) 2005 - 2018 by Vivante Corp.  All rights reserved.
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

#if ! defined (YYSTYPE) && ! defined (YYSTYPE_IS_DECLARED)
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

#if ! defined (yyoverflow) || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# if YYSTACK_USE_ALLOCA
#  define YYSTACK_ALLOC alloca
# else
#  ifndef YYSTACK_USE_ALLOCA
#   if defined (alloca) || defined (_ALLOCA_H)
#    define YYSTACK_ALLOC alloca
#   else
#    ifdef __GNUC__
#     define YYSTACK_ALLOC __builtin_alloca
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning. */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
# else
#  if defined (__STDC__) || defined (__cplusplus)

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
# ifndef YYCOPY
#  if 1 < __GNUC__
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
#define YYLAST   3556

/* YYNTOKENS -- Number of terminals. */
#define YYNTOKENS  263
/* YYNNTS -- Number of nonterminals. */
#define YYNNTS  111
/* YYNRULES -- Number of rules. */
#define YYNRULES  331
/* YYNRULES -- Number of states. */
#define YYNSTATES  563

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
     422,   427,   429,   432,   434,   437,   440,   444,   447,   451,
     453,   455,   457,   460,   463,   467,   469,   472,   474,   477,
     480,   484,   486,   489,   491,   495,   496,   498,   502,   507,
     509,   510,   514,   519,   525,   526,   534,   535,   544,   546,
     550,   555,   556,   563,   564,   572,   573,   581,   582,   584,
     585,   587,   588,   593,   598,   600,   605,   614,   623,   632,
     634,   639,   641,   643,   648,   651,   657,   665,   667,   669,
     672,   675,   679,   684,   686,   688,   690,   692,   694,   696,
     698,   700,   702,   704,   706,   708,   710,   712,   714,   716,
     718,   720,   722,   724,   726,   728,   730,   732,   734,   736,
     738,   740,   742,   744,   746,   748,   750,   752,   754,   756,
     758,   760,   762,   764,   765,   773,   774,   781,   782,   790,
     791,   798,   800,   803,   807,   809,   813,   814,   817,   821,
     823,   825,   829,   833,   836,   841,   842,   845,   847,   848,
     852,   856,   859,   861,   863,   865,   867,   869,   871,   873,
     875,   878,   881,   884,   887,   888,   893,   895,   897,   900,
     901,   906,   908,   911,   913,   916,   922,   928,   930,   933,
     935,   939,   942,   945,   946,   951,   955,   957,   958,   965,
     973,   974,   980,   983,   986,   990,   992,   993,   997,  1002,
    1006,  1011,  1014,  1017,  1020,  1024,  1028,  1029,  1032,  1034,
    1036,  1038
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
      -1,   329,   310,    -1,   329,   310,   312,    -1,   329,   307,
     310,    -1,   329,   307,   310,   312,    -1,   303,    -1,   305,
     303,    -1,   306,    -1,   305,   306,    -1,   307,   303,    -1,
     307,   305,   303,    -1,   307,   306,    -1,   307,   305,   306,
      -1,   217,    -1,   218,    -1,   329,    -1,   329,   312,    -1,
     329,   308,    -1,   329,   308,   312,    -1,   328,    -1,   328,
     307,    -1,   254,    -1,   254,   308,    -1,   254,   307,    -1,
     254,   307,   308,    -1,   162,    -1,   308,   162,    -1,   309,
      -1,   239,   309,   240,    -1,    -1,   293,    -1,   241,   311,
     242,    -1,   312,   241,   311,   242,    -1,   317,    -1,    -1,
     231,   314,   317,    -1,   313,   246,   310,   322,    -1,   313,
     246,   310,   312,   322,    -1,    -1,   313,   246,   310,   322,
     248,   315,   342,    -1,    -1,   313,   246,   310,   312,   322,
     248,   316,   342,    -1,   299,    -1,   327,   310,   322,    -1,
     327,   310,   312,   322,    -1,    -1,   327,   310,   322,   248,
     318,   342,    -1,    -1,   327,   310,   312,   322,   248,   319,
     342,    -1,    -1,   223,   239,   239,   321,   323,   240,   240,
      -1,    -1,   320,    -1,    -1,   325,    -1,    -1,   323,   246,
     324,   325,    -1,   221,   239,   162,   240,    -1,   219,    -1,
     222,   239,   159,   240,    -1,   224,   239,   293,   246,   293,
     246,   293,   240,    -1,   225,   239,   293,   246,   293,   246,
     293,   240,    -1,   226,   239,   293,   246,   293,   246,   293,
     240,    -1,   220,    -1,   220,   239,   293,   240,    -1,   227,
      -1,   327,    -1,   327,   241,   293,   242,    -1,   327,   308,
      -1,   327,   308,   241,   293,   242,    -1,   239,   327,   308,
     240,   241,   293,   242,    -1,   292,    -1,   329,    -1,   307,
     329,    -1,   329,   307,    -1,   307,   329,   307,    -1,   237,
     239,   326,   240,    -1,   206,    -1,   207,    -1,   208,    -1,
     211,    -1,   212,    -1,   213,    -1,   214,    -1,   209,    -1,
     210,    -1,   216,    -1,   330,    -1,   332,    -1,   294,    -1,
     295,    -1,     4,    -1,    24,    -1,    70,    -1,    10,    -1,
     157,    -1,   158,    -1,   159,    -1,   160,    -1,   161,    -1,
      95,    -1,    96,    -1,    97,    -1,    98,    -1,    99,    -1,
     101,    -1,   102,    -1,   100,    -1,    94,    -1,   105,    -1,
     106,    -1,   107,    -1,   103,    -1,   104,    -1,   163,    -1,
     229,    -1,   230,    -1,    -1,   331,   162,   243,   333,   337,
     244,   322,    -1,    -1,   331,   243,   334,   337,   244,   322,
      -1,    -1,   331,   320,   162,   243,   335,   337,   244,    -1,
      -1,   331,   320,   243,   336,   337,   244,    -1,   338,    -1,
     337,   338,    -1,   327,   339,   249,    -1,   340,    -1,   339,
     246,   340,    -1,    -1,   310,   322,    -1,   310,   312,   322,
      -1,   243,    -1,   290,    -1,   341,   343,   244,    -1,   341,
     343,   193,    -1,   344,   342,    -1,   343,   246,   344,   342,
      -1,    -1,   345,   248,    -1,   347,    -1,    -1,   345,   346,
     347,    -1,   241,   293,   242,    -1,   245,   170,    -1,   298,
      -1,   351,    -1,   350,    -1,   348,    -1,   357,    -1,   358,
      -1,   364,    -1,   370,    -1,   162,   247,    -1,     1,   249,
      -1,     1,   244,    -1,   243,   244,    -1,    -1,   243,   352,
     356,   244,    -1,   354,    -1,   350,    -1,   243,   244,    -1,
      -1,   243,   355,   356,   244,    -1,   349,    -1,   356,   349,
      -1,   249,    -1,   292,   249,    -1,   202,   239,   292,   240,
     363,    -1,   203,   239,   268,   240,   361,    -1,   360,    -1,
     359,   360,    -1,   349,    -1,   204,   293,   247,    -1,   205,
     247,    -1,   243,   244,    -1,    -1,   243,   362,   359,   244,
      -1,   349,   201,   349,    -1,   349,    -1,    -1,   198,   365,
     239,   292,   240,   353,    -1,   200,   349,   198,   239,   292,
     240,   249,    -1,    -1,   199,   366,   367,   369,   353,    -1,
     239,   357,    -1,   239,   348,    -1,   239,     1,   249,    -1,
     292,    -1,    -1,   368,   249,   240,    -1,   368,   249,   292,
     240,    -1,     1,   249,   240,    -1,     1,   249,   292,   240,
      -1,   195,   249,    -1,   194,   249,    -1,   196,   249,    -1,
     196,   292,   249,    -1,   197,   162,   249,    -1,    -1,   371,
     372,    -1,   373,    -1,   298,    -1,   249,    -1,   299,   354,
      -1
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
     582,   584,   586,   588,   590,   592,   596,   603,   605,   607,
     613,   622,   624,   626,   628,   630,   632,   634,   636,   641,
     643,   648,   650,   653,   659,   668,   673,   678,   680,   682,
     684,   688,   690,   697,   699,   704,   705,   710,   712,   717,
     720,   719,   728,   730,   733,   732,   737,   736,   744,   746,
     748,   751,   750,   755,   754,   764,   763,   770,   771,   776,
     777,   780,   779,   786,   788,   790,   792,   794,   796,   798,
     800,   802,   807,   809,   811,   813,   815,   817,   822,   824,
     826,   828,   835,   840,   842,   844,   846,   848,   850,   852,
     854,   856,   858,   863,   865,   867,   869,   874,   876,   878,
     880,   882,   884,   886,   888,   890,   892,   894,   896,   898,
     900,   902,   904,   906,   908,   910,   912,   914,   916,   918,
     920,   924,   926,   932,   931,   936,   935,   940,   939,   944,
     943,   950,   954,   967,   974,   976,   981,   982,   984,   989,
    1011,  1013,  1015,  1020,  1022,  1027,  1028,  1041,  1044,  1043,
    1052,  1065,  1081,  1086,  1088,  1093,  1095,  1097,  1099,  1101,
    1103,  1105,  1109,  1116,  1119,  1118,  1125,  1127,  1132,  1135,
    1134,  1141,  1143,  1148,  1150,  1155,  1157,  1162,  1164,  1169,
    1171,  1173,  1178,  1181,  1180,  1187,  1189,  1197,  1196,  1200,
    1203,  1202,  1209,  1211,  1213,  1220,  1223,  1227,  1229,  1231,
    1237,  1246,  1248,  1250,  1252,  1254,  1260,  1261,  1265,  1266,
    1268,  1272
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

# ifdef YYPRINT
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
     302,   302,   302,   302,   302,   302,   302,   303,   303,   303,
     303,   304,   304,   304,   304,   304,   304,   304,   304,   305,
     305,   306,   306,   306,   306,   307,   307,   308,   308,   308,
     308,   309,   309,   310,   310,   311,   311,   312,   312,   313,
     314,   313,   313,   313,   315,   313,   316,   313,   317,   317,
     317,   318,   317,   319,   317,   321,   320,   322,   322,   323,
     323,   324,   323,   325,   325,   325,   325,   325,   325,   325,
     325,   325,   326,   326,   326,   326,   326,   326,   327,   327,
     327,   327,   327,   328,   328,   328,   328,   328,   328,   328,
     328,   328,   328,   329,   329,   329,   329,   330,   330,   330,
     330,   330,   330,   330,   330,   330,   330,   330,   330,   330,
     330,   330,   330,   330,   330,   330,   330,   330,   330,   330,
     330,   331,   331,   333,   332,   334,   332,   335,   332,   336,
     332,   337,   337,   338,   339,   339,   340,   340,   340,   341,
     342,   342,   342,   343,   343,   344,   344,   345,   346,   345,
     347,   347,   348,   349,   349,   350,   350,   350,   350,   350,
     350,   350,   350,   351,   352,   351,   353,   353,   354,   355,
     354,   356,   356,   357,   357,   358,   358,   359,   359,   360,
     360,   360,   361,   362,   361,   363,   363,   365,   364,   364,
     366,   364,   367,   367,   367,   368,   368,   369,   369,   369,
     369,   370,   370,   370,   370,   370,   371,   371,   372,   372,
     372,   373
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
       5,     5,     6,     3,     4,     4,     5,     2,     3,     3,
       4,     1,     2,     1,     2,     2,     3,     2,     3,     1,
       1,     1,     2,     2,     3,     1,     2,     1,     2,     2,
       3,     1,     2,     1,     3,     0,     1,     3,     4,     1,
       0,     3,     4,     5,     0,     7,     0,     8,     1,     3,
       4,     0,     6,     0,     7,     0,     7,     0,     1,     0,
       1,     0,     4,     4,     1,     4,     8,     8,     8,     1,
       4,     1,     1,     4,     2,     5,     7,     1,     1,     2,
       2,     3,     4,     1,     1,     1,     1,     1,     1,     1,
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
     326,     0,     1,   217,   220,   218,   219,   234,   226,   227,
     228,   229,   230,   233,   231,   232,   238,   239,   235,   236,
     237,   221,   222,   223,   224,   225,   240,   203,   204,   205,
     210,   211,   206,   207,   208,   209,   177,   212,     0,   241,
     242,   160,     0,     0,     0,   330,   215,   216,   329,   168,
       0,   117,   116,     0,     0,   159,     0,     0,   145,   198,
     213,     0,   214,   327,   328,     0,   177,   178,     0,     0,
       0,    98,     0,     0,   210,   211,   215,   216,     0,   214,
       0,   113,     0,   289,   331,   115,     0,   139,   140,   131,
     118,     0,   133,     0,   141,   199,     0,   111,     0,     0,
     151,     0,   147,     0,   153,   177,   146,   200,    97,   245,
       0,   114,     0,     0,     0,   175,   168,   161,     0,   109,
       0,   107,     0,     0,     0,     2,     8,     6,     7,     9,
      10,     3,     0,     0,     0,     0,     0,    45,    44,    46,
      43,    48,    47,     5,    11,    13,    35,    15,     0,    31,
       0,     0,    49,    30,     0,    53,    56,    59,    64,    67,
      69,    71,    73,    75,    77,    79,    81,    94,   197,     0,
     192,   112,   288,     0,   119,   132,   134,   135,     0,   137,
     155,     0,   143,   127,   142,   201,   177,     0,     0,     0,
     149,   148,   152,   123,   177,   169,   243,     0,     0,   249,
       0,     0,     0,   179,     0,     0,   106,   105,     0,     0,
       0,   125,    28,     0,    36,    37,     0,    41,    40,     0,
      39,     0,     0,     4,    18,    19,     0,     0,     0,    21,
       0,    29,     0,     0,    23,    24,     0,    84,    85,    87,
      86,    89,    90,    91,    92,    93,    88,    83,     0,    42,
      30,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   202,     0,   194,     0,     2,     0,     0,     0,
       0,   307,   310,     0,     0,     0,   284,   293,     0,   272,
     275,   291,   274,   273,     0,   276,   277,   278,   279,   136,
     138,    96,   156,     0,   129,   144,   128,   155,   177,   162,
       0,   124,   154,   150,   170,   171,     0,   256,     0,   251,
     247,     0,   126,     0,   121,   184,   189,     0,     0,     0,
       0,     0,   191,     0,   180,   102,   101,   110,   108,     0,
     104,   103,     0,     0,    12,    26,     0,    17,     0,    20,
      16,    25,    32,    22,    34,    33,    82,    50,    51,    52,
      55,    54,    57,    58,    62,    63,    60,    61,    65,    66,
      68,    70,    72,    74,    76,    78,     0,    95,     0,     0,
     282,   281,   280,   322,   321,   323,     0,     0,     0,     0,
       0,     0,     0,   283,     0,   294,   290,   292,   157,   130,
       0,   163,   164,   120,   173,     0,     0,   177,     0,   254,
     177,   252,     0,     0,   122,     0,     0,     0,     0,     0,
       0,     0,   181,   100,    99,     0,    38,    27,    14,     0,
     193,     0,   324,   325,     0,     0,     0,     0,     0,     0,
       0,   158,   166,     0,     0,   259,   260,   265,   172,   177,
     177,   257,   256,   253,   246,     0,   250,     0,     0,     0,
       0,     0,     0,   176,     0,    27,     0,    80,   195,     0,
       0,   313,   312,     0,   315,     0,     0,     0,     0,     0,
     285,     0,   165,   174,     0,     0,     0,     0,   268,   267,
     244,   258,   255,   248,   190,   183,   185,     0,     0,     0,
     182,     0,     0,   314,     0,     0,   287,   311,   286,     0,
     306,   295,   303,   296,   167,     0,   271,   262,   261,   265,
     263,   266,     0,     0,     0,     0,   196,   308,   319,     0,
     317,     0,     0,     0,   302,     0,   270,     0,   269,     0,
       0,     0,   320,   318,   309,   305,     0,     0,   299,     0,
     297,   264,     0,     0,     0,     0,   301,   304,   298,   186,
     187,   188,   300
};

/* YYDEFGOTO[NTERM-NUM]. */
static const short yydefgoto[] =
{
      -1,   143,   144,   145,   146,   348,   147,   148,   149,   150,
     151,   152,   232,   250,   154,   155,   156,   157,   158,   159,
     160,   161,   162,   163,   164,   165,   166,   167,   248,   288,
     302,    76,    77,   120,   121,   289,   116,    50,    51,    52,
      89,    90,    91,    92,    53,   103,   104,   407,   303,   184,
      54,    70,   443,   481,    55,   405,   444,    56,   203,    68,
     333,   464,   334,   169,    57,    58,    59,    60,    61,    79,
     316,   197,   412,   321,   318,   319,   408,   409,   447,   448,
     486,   487,   488,   522,   489,   290,   291,   292,   293,   394,
     507,   508,   173,   294,   295,   296,   549,   550,   513,   535,
     511,   297,   388,   389,   436,   475,   476,   298,     1,    63,
      64
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -437
static const short yypact[] =
{
    -437,  2289,  -437,  -437,  -437,  -437,  -437,  -437,  -437,  -437,
    -437,  -437,  -437,  -437,  -437,  -437,  -437,  -437,  -437,  -437,
    -437,  -437,  -437,  -437,  -437,  -437,  -437,  -437,  -437,  -437,
    -148,  -118,  -437,  -437,  -437,  -437,   -72,  -437,   -77,  -437,
    -437,  -437,  -136,  3022,   -71,  -437,   -40,  -196,  -437,   -30,
      -9,   -13,  3174,    21,  -210,  -437,  2981,  -143,   485,   485,
    -437,  -133,    -6,  -437,  -437,  3022,   -72,  -437,  3022,    10,
    2829,    14,   119,   -98,  -437,  -437,  -437,  -437,  -143,  -437,
    1782,  -437,    46,    55,  -437,  -437,  3174,  -437,  -437,  -437,
    -437,    21,  -437,  3195,   306,   485,  -143,  -437,  3022,  -143,
    -437,  -124,   371,   128,  -437,   -56,  -437,  -437,    67,  -437,
     -97,  -437,  -143,  3022,  -143,  -437,  -437,  -437,   119,    69,
     -36,  -437,    77,   119,    76,    82,  -437,  -437,  -437,  -437,
    -437,  -437,  2363,  2363,  3295,    90,  1950,  -437,  -437,  -437,
    -437,  -437,  -437,  -437,   162,  -437,  -141,  -437,  -203,    94,
      72,  3295,  -437,    36,  3295,     5,  -183,    19,   -43,    88,
      73,    80,    83,   156,   160,  -159,  -437,  -437,    97,   104,
    -207,  -437,  -437,  1260,  -437,  -437,  -437,  -437,    21,  -437,
    3295,  -143,   -83,   108,   113,  -437,  -199,  -143,   116,   120,
     105,  -437,  -437,  -437,  -152,   122,  -437,  3022,   124,  -437,
     129,  -143,   132,   304,   -16,  3295,  -437,  -437,   119,   119,
      54,  -437,  -437,  3295,  -437,  -437,  1950,  -437,  -437,  2118,
    -437,   -81,  -171,  -437,  -437,  -437,   203,  3295,   204,  -437,
    3295,  -437,  3295,   135,  -437,  -437,    63,  -437,  -437,  -437,
    -437,  -437,  -437,  -437,  -437,  -437,  -437,  -437,  3295,  -437,
    -437,  3295,  3295,  3295,  3295,  3295,  3295,  3295,  3295,  3295,
    3295,  3295,  3295,  3295,  3295,  3295,  3295,  3295,  3295,  3295,
    3295,  3295,  -437,  3295,   136,   -32,   -86,   134,   137,  1835,
     216,  -437,  -437,  1260,   142,   145,   143,  -437,  -110,  -437,
    -437,  -437,  -437,  -437,   906,  -437,  -437,  -437,  -437,  -437,
    -437,  -437,  -437,   148,   108,   113,   113,  3295,  -152,   144,
     146,  -437,  -437,  -437,   153,  -437,  3022,  -143,  2447,  -437,
    -437,  3022,  -437,   152,  -437,  -437,   163,   166,   167,   168,
     169,   171,  -437,   -80,  -437,  -437,  -437,  -437,  -437,    86,
    -437,  -437,  -171,   173,  -437,  -437,   174,  -437,   175,    97,
    -437,  -437,  -437,  -437,  -437,  -437,  -437,  -437,  -437,  -437,
       5,     5,  -183,  -183,    19,    19,    19,    19,   -43,   -43,
      88,    73,    80,    83,   156,   160,    22,  -437,   178,  3295,
    -437,  -437,  -437,  -437,  -437,  -437,   -46,   172,   176,   177,
     224,  3295,  3295,  -437,  1260,  -437,  -437,  -437,  -437,   113,
     181,   180,  -437,  -437,  -437,  2003,  2599,  -199,   -14,  -437,
     -72,  -437,  3022,  2638,  -437,  3295,   263,   270,  3295,  3295,
    3295,   193,  -437,  -437,  -437,   196,  -437,   200,  -437,  3295,
    -437,   195,  -437,  -437,  3295,  1614,   429,   205,   -52,   202,
    1083,  -437,  -437,  2003,  2003,  -437,  -437,   -70,  -437,   -72,
    -152,  -437,  -143,  -437,  -437,  2790,  -437,   206,   208,   210,
     199,   207,   212,  -437,   304,  -437,  3295,  -437,  -437,   -51,
     211,  -437,  -437,   213,    97,   214,  1437,  3295,  1260,   218,
    -437,  2003,  -437,  -437,  3295,   281,    92,  2003,   217,  -437,
    -437,  -437,  -437,  -437,  -437,  -437,  -437,  3295,  3295,  3295,
    -437,   222,  1437,  -437,  2171,  3197,  -437,  -437,  -437,   -42,
     255,  -437,   223,  -437,  -437,   227,  -437,  -437,  -437,   -70,
    -437,  -437,   -70,   220,   226,   228,  -437,  -437,  -437,     2,
    -437,    12,   231,  1260,  -437,   729,  -437,  2003,  -437,  3295,
    3295,  3295,  -437,  -437,  -437,  -437,  3295,   235,  -437,   552,
    -437,  -437,   233,   243,   244,   238,  -437,  -437,  -437,  -437,
    -437,  -437,  -437
};

/* YYPGOTO[NTERM-NUM].  */
static const short yypgoto[] =
{
    -437,  -437,  -437,  -437,  -437,    95,  -437,  -437,   141,  -437,
    -437,  -139,  -437,   -78,  -437,    34,    35,    13,    41,   225,
     221,   230,   232,   229,   219,  -437,    75,  -142,  -437,   -73,
      91,    -1,     0,  -101,   283,   491,   497,  -437,  -437,  -437,
     -47,   414,   410,   -41,   294,   -88,   403,   -37,   201,  -102,
    -437,  -437,  -437,  -437,   437,  -437,  -437,   -31,  -437,   -38,
    -437,  -437,    57,  -437,   131,  -437,     8,  -437,  -437,     3,
    -437,  -437,  -437,  -437,  -303,  -308,  -437,    87,  -437,  -395,
    -437,    23,  -437,  -437,    18,   111,  -271,  -436,  -437,  -437,
      25,   501,  -437,   157,   123,  -437,  -437,     6,  -437,  -437,
    -437,  -437,  -437,  -437,  -437,  -437,  -437,  -437,  -437,  -437,
    -437
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -317
static const short yytable[] =
{
      46,    47,   153,   194,    62,    67,   182,   168,   235,    82,
     411,    73,   390,   406,   191,   249,    67,   204,   413,   100,
     105,   269,   210,   397,    38,     3,    71,    38,   113,   108,
     110,     4,   224,   225,   273,    67,    96,   229,   100,    97,
     506,   124,   180,   230,   175,     5,   177,   102,   482,   483,
     176,   226,   179,  -177,   214,   215,   218,   183,   153,   186,
      94,    95,   188,   221,   122,   198,   506,   195,   254,   345,
     255,    38,   153,   153,    67,   200,   233,   202,   236,   192,
     305,   306,   274,   102,   308,    65,   514,    38,   351,   307,
      38,     6,   520,   352,    94,   153,   101,    66,   411,    94,
     227,    94,   313,   270,   228,   411,   356,    72,   339,   455,
     109,   102,   357,   358,   359,     7,     8,     9,    10,    11,
      12,    13,    14,    15,    16,    17,    18,    19,    20,   377,
     102,   299,   258,   259,   346,   153,   271,   300,   153,   395,
     221,   343,   551,   221,   304,   123,   199,   411,   309,   153,
     310,    38,   153,   212,   349,    67,   314,   206,   180,   344,
     421,   382,    69,    67,   323,   271,   422,    38,    80,   397,
     153,   484,    46,    47,    78,   485,    62,   335,    21,    22,
      23,    24,    25,   193,    26,   180,    94,    99,   478,   502,
     256,   257,   153,   153,   271,   271,   112,   376,   532,   114,
     271,   153,   399,   432,   271,   153,   386,   510,   207,    81,
     208,   170,   380,    83,   260,   261,   153,   381,   237,   238,
     239,   240,   241,   242,   243,   244,   245,   246,   336,   187,
     208,    85,   452,    86,   125,   453,   126,   127,   128,   129,
     130,   131,   542,   111,   201,   132,   133,   340,   271,   115,
      39,    40,   543,    42,   425,   301,   354,   118,   271,   251,
     252,   253,   545,   446,   548,   262,   263,   222,   271,   429,
     401,   364,   365,   366,   367,   217,   220,    67,   548,   423,
     301,   119,    46,    47,   247,   517,    62,   467,   360,   361,
     192,   362,   363,    46,    47,   171,   337,    62,   341,   172,
     208,   446,   446,   368,   369,   450,   134,   355,   135,   271,
     196,   216,   234,   153,   153,   211,   153,   205,   438,   349,
     209,   212,   137,   138,   139,   140,   141,   153,   317,   219,
     424,   223,   208,   142,   264,   267,   518,   231,   519,   446,
     265,   268,   266,   271,   272,   446,    93,   342,   301,   180,
     342,   153,   106,   107,   307,   311,   153,   153,   153,   102,
     312,   469,   153,   474,   378,   153,   153,   320,   322,   451,
     315,   324,   454,   347,   350,   353,    67,   379,   387,    67,
      93,   391,   301,   383,   392,   403,   384,   393,   181,   185,
     398,   414,   402,    46,    47,   446,   190,    62,   153,   153,
     153,   404,   415,   153,   509,   416,   417,   418,   419,   153,
     420,   490,   491,   426,   427,   434,   435,   428,    67,    67,
     430,   433,   437,   441,   153,   458,   153,   153,   442,   459,
     473,   529,   531,   463,    46,    47,   465,   468,    62,    46,
      47,   466,   479,    62,   477,   497,   494,   317,   495,   317,
     496,   516,   317,   498,   301,   153,   533,   153,   499,   153,
     503,   512,   504,   505,   526,   521,   539,   534,   100,   536,
     431,   153,   540,   559,   541,    46,    47,    46,    47,    62,
     544,    62,   556,   560,   561,   562,   371,   439,   375,   370,
     301,   338,    48,   301,   301,   301,   372,   374,    49,   373,
     174,    46,    47,   178,   189,    62,   457,   117,   400,   460,
     461,   462,    27,    28,    29,    74,    75,    32,    33,    34,
      35,   500,    37,   325,   326,   327,   328,   527,   329,   330,
     331,   332,    46,    47,    46,    47,    62,   317,    62,   492,
     538,   301,   537,   317,   317,   101,   471,   180,    46,    47,
      84,   440,    62,   275,     0,   558,     3,   501,   472,   301,
     102,     0,     4,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   301,   301,   301,   515,     5,    27,    28,    29,
      74,    75,    32,    33,    34,    35,   317,    37,   523,   524,
     525,   125,     0,   126,   127,   128,   129,   130,   131,     0,
       0,     0,   132,   133,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   301,   301,   301,     0,     0,     0,
       0,   301,     6,     0,     0,   102,     0,     0,     0,     0,
     552,   553,   554,     0,     0,     0,     0,   555,     0,     0,
       0,     0,     0,     0,     0,     0,     7,     8,     9,    10,
      11,    12,    13,    14,    15,    16,    17,    18,    19,    20,
       0,     0,     0,   134,     0,   135,     0,     0,   216,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  -316,   137,
     138,   139,   140,   141,     0,     0,     0,     0,     0,     0,
     142,    27,    28,    29,    74,    75,    32,    33,    34,    35,
       0,    37,     0,     0,     0,     0,     0,     0,     0,    21,
      22,    23,    24,    25,   276,    26,   126,   127,   128,   129,
     130,   131,     0,     0,     0,   132,   133,     0,     0,     0,
     275,     0,     0,     3,     0,     0,     0,     0,     0,     4,
       0,     0,     0,     0,     0,     0,   277,   278,   279,   280,
     281,   282,   283,     5,   284,   285,   546,   547,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,     0,
       0,     0,     0,     0,     0,    38,     0,     0,     0,     0,
       0,    39,    40,    41,    42,    43,   134,     0,   135,    44,
       0,   216,     0,     0,     0,   286,   557,     0,     0,     6,
       0,   287,   137,   138,   139,   140,   141,     0,     0,     0,
       0,     0,     0,   142,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     7,     8,     9,    10,    11,    12,    13,
      14,    15,    16,    17,    18,    19,    20,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    21,    22,    23,    24,
      25,   276,    26,   126,   127,   128,   129,   130,   131,     0,
       0,     0,   132,   133,     0,     0,     0,   275,     0,     0,
       3,     0,     0,     0,     0,     0,     4,     0,     0,     0,
       0,     0,     0,   277,   278,   279,   280,   281,   282,   283,
       5,   284,   285,   546,   547,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,     0,     0,     0,     0,
       0,     0,    38,     0,     0,     0,     0,     0,    39,    40,
      41,    42,    43,   134,     0,   135,    44,     0,   216,     0,
       0,     0,   286,     0,     0,     0,     6,     0,   287,   137,
     138,   139,   140,   141,     0,     0,     0,     0,     0,     0,
     142,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
      17,    18,    19,    20,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    21,    22,    23,    24,    25,   276,    26,
     126,   127,   128,   129,   130,   131,     0,     0,     0,   132,
     133,     0,     0,     0,   275,     0,     0,     3,     0,     0,
       0,     0,     0,     4,     0,     0,     0,     0,     0,     0,
     277,   278,   279,   280,   281,   282,   283,     5,   284,   285,
       0,     0,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,     0,     0,     0,     0,     0,     0,    38,
       0,     0,     0,     0,     0,    39,    40,    41,    42,    43,
     134,     0,   135,    44,     0,   216,     0,     0,     0,   286,
     396,     0,     0,     6,     0,   287,   137,   138,   139,   140,
     141,     0,     0,     0,     0,     0,     0,   142,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      21,    22,    23,    24,    25,   276,    26,   126,   127,   128,
     129,   130,   131,     0,     0,     0,   132,   133,     0,     0,
       0,   275,     0,     0,     3,     0,     0,     0,     0,     0,
       4,     0,     0,     0,     0,     0,     0,   277,   278,   279,
     280,   281,   282,   283,     5,   284,   285,     0,     0,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
       0,     0,     0,     0,     0,     0,    38,     0,     0,     0,
       0,     0,    39,    40,    41,    42,    43,   134,     0,   135,
      44,     0,   216,     0,     0,     0,   286,   480,     0,     0,
       6,     0,   287,   137,   138,   139,   140,   141,     0,     0,
       0,     0,     0,     0,   142,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     7,     8,     9,    10,    11,    12,
      13,    14,    15,    16,    17,    18,    19,    20,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    21,    22,    23,
      24,    25,   276,    26,   126,   127,   128,   129,   130,   131,
       0,     0,     0,   132,   133,     0,     0,     0,   275,     0,
       0,     3,     0,     0,     0,     0,     0,     4,     0,     0,
       0,     0,     0,     0,   277,   278,   279,   280,   281,   282,
     283,     5,   284,   285,     0,     0,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,     0,     0,     0,
       0,     0,     0,    38,     0,     0,     0,     0,     0,    39,
      40,    41,    42,    43,   134,     0,   135,    44,     0,   216,
       0,     0,     0,   286,     0,     0,     0,     6,     0,   287,
     137,   138,   139,   140,   141,     0,     0,     0,     0,     0,
       0,   142,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     7,     8,     9,    10,    11,    12,    13,    14,    15,
      16,    17,    18,    19,    20,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    21,    22,    23,    24,    25,   276,
      26,   126,   127,   128,   129,   130,   131,     0,     0,     0,
     132,   133,     0,     0,     0,   470,     0,     0,     3,     0,
       0,     0,     0,     0,     4,     0,     0,     0,     0,     0,
       0,   277,   278,   279,   280,   281,   282,   283,     5,   284,
     285,     0,     0,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,     0,     0,     0,     0,     0,     0,
      38,     0,     0,     0,     0,     0,    39,    40,    41,    42,
      43,   134,     0,   135,    44,     0,   216,     0,     0,     0,
      83,     0,     0,     0,     6,     0,   287,   137,   138,   139,
     140,   141,     0,     0,     0,     0,     0,     0,   142,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    17,    18,
      19,    20,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    21,    22,    23,    24,    25,   125,    26,   126,   127,
     128,   129,   130,   131,     0,     0,     3,   132,   133,     0,
       0,     0,     4,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     5,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,     0,     0,     0,     0,     0,     0,    38,     0,     0,
       0,     0,     0,    39,    40,    41,    42,    43,   134,     0,
     135,    44,     6,   216,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   287,   137,   138,   139,   140,   141,     0,
       0,     0,     0,     0,     0,   142,     7,     8,     9,    10,
      11,    12,    13,    14,    15,    16,    17,    18,    19,    20,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    21,
      22,    23,    24,    25,   125,    26,   126,   127,   128,   129,
     130,   131,     0,     0,     3,   132,   133,     0,     0,     0,
       4,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     5,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    27,    28,
      29,    74,    75,    32,    33,    34,    35,   125,    37,   126,
     127,   128,   129,   130,   131,     0,     0,     0,   132,   133,
       0,    39,    40,     0,    42,     0,   134,     0,   135,    44,
       6,   136,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   137,   138,   139,   140,   141,     0,     0,     0,
       0,     0,     0,   142,     7,     8,     9,    10,    11,    12,
      13,    14,    15,    16,    17,    18,    19,    20,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   134,
       0,   135,     0,     0,   216,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   385,   137,   138,   139,   140,   141,
       0,     0,     0,     0,     0,     0,   142,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    21,    22,    23,
      24,    25,   125,    26,   126,   127,   128,   129,   130,   131,
       0,     0,     3,   132,   133,     0,     0,     0,     4,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     5,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    27,    28,    29,    74,
      75,    32,    33,    34,    35,   125,    37,   126,   127,   128,
     129,   130,   131,     0,     0,     0,   132,   133,     0,    39,
      40,     0,    42,     0,   134,     0,   135,    44,     6,   216,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     137,   138,   139,   140,   141,     0,     0,     0,     0,     0,
       0,   142,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   134,     0,   135,
       0,     0,   216,     0,     0,     0,   445,     0,     0,     0,
       0,     0,     0,   137,   138,   139,   140,   141,     0,     0,
       0,     0,     0,     0,   142,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    21,    22,    23,    24,    25,
     125,    26,   126,   127,   128,   129,   130,   131,     0,     2,
       0,   132,   133,     3,     0,     0,     0,     0,     0,     4,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     5,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    27,    28,    29,    74,    75,    32,
      33,    34,    35,   125,    37,   126,   127,   128,   129,   130,
     131,     0,     0,     0,   132,   133,     0,    39,    40,     0,
      42,     0,   134,     0,   135,    44,     0,   213,     0,     6,
       0,     0,     0,     0,     0,     0,     0,     0,   137,   138,
     139,   140,   141,     0,     0,     0,     0,     0,     0,   142,
       0,     0,     0,     7,     8,     9,    10,    11,    12,    13,
      14,    15,    16,    17,    18,    19,    20,     0,     0,     0,
       0,     0,     0,     0,     0,   134,     0,   135,     0,     0,
     216,   528,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   137,   138,   139,   140,   141,     0,     0,     0,     0,
       0,     0,   142,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    21,    22,    23,    24,
      25,     3,    26,     0,     0,     0,     0,     4,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     5,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,     0,     0,     0,     0,
       0,     0,    38,     0,     0,     0,     0,     6,    39,    40,
      41,    42,    43,     0,     0,   125,    44,   126,   127,   128,
     129,   130,   131,     0,     0,     0,   132,   133,    45,     0,
       0,     7,     8,     9,    10,    11,    12,    13,    14,    15,
      16,    17,    18,    19,    20,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   134,     0,   135,
       0,     0,   213,     3,    21,    22,    23,    24,    25,     4,
      26,     0,     0,   137,   138,   139,   140,   141,     0,     0,
       0,     0,     0,     5,   142,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     3,     0,     0,     0,     0,     0,     4,     0,
       0,     0,     0,    27,    28,    29,    74,    75,    32,    33,
      34,    35,     5,    37,     0,     0,     0,     0,     0,     6,
       0,     0,     0,     0,     0,     0,    39,    40,     0,    42,
       0,     0,     0,     0,    44,     0,     0,     0,     0,     0,
       0,   410,     0,     7,     8,     9,    10,    11,    12,    13,
      14,    15,    16,    17,    18,    19,    20,     0,     6,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    21,    22,    23,    24,
      25,     0,    26,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     3,    21,    22,    23,    24,    25,
       4,    26,     0,     0,     0,    27,    28,    29,    74,    75,
      32,    33,    34,    35,     5,    37,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    39,    40,
       0,    42,     0,     3,     0,     0,    44,     0,     0,     4,
       0,     0,     0,   449,    27,    28,    29,    74,    75,    32,
      33,    34,    35,     5,    37,     0,     0,     0,     0,     0,
       6,     0,     0,     0,     0,     0,     0,    39,    40,     0,
      42,     0,     0,     0,     0,    44,     0,     0,     0,     0,
       0,     0,   456,     0,     7,     8,     9,    10,    11,    12,
      13,    14,    15,    16,    17,    18,    19,    20,     0,     6,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     7,     8,     9,    10,    11,    12,    13,
      14,    15,    16,    17,    18,    19,    20,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    21,    22,    23,
      24,    25,     0,    26,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     3,    21,    22,    23,    24,
      25,     4,    26,     0,     0,     0,    27,    28,    29,    74,
      75,    32,    33,    34,    35,     5,    37,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    39,
      40,     0,    42,     0,     0,     0,     3,    44,     0,     0,
       0,     0,     4,     0,   493,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,     5,     0,     0,     0,
       0,     6,    38,     0,     0,     0,     0,     0,    39,    40,
       0,    42,    43,     0,     0,     0,    44,     0,     0,     0,
       0,     0,     0,     0,     0,     7,     8,     9,    10,    11,
      12,    13,    14,    15,    16,    17,    18,    19,    20,     0,
       0,     0,     6,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     7,     8,     9,    10,
      11,    12,    13,    14,    15,    16,    17,    18,    19,    20,
       0,     0,     0,     0,     0,     0,     0,     0,    21,    22,
      23,    24,    25,     0,    26,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     3,    21,
      22,    23,    24,    25,     4,    26,     0,    27,    28,    29,
      74,    75,    32,    33,    34,    35,    98,    37,     5,     3,
       0,     0,     0,     0,     0,     4,     0,     0,     0,     0,
      39,    40,     0,    42,     0,     0,     0,     0,    44,     5,
       0,     0,     0,     0,     0,     0,     0,     0,    27,    28,
      29,    74,    75,    32,    33,    34,    35,     0,    37,     0,
       0,     0,     0,     0,     6,     0,     0,     0,     0,     0,
       0,    39,    40,     0,    42,     0,     0,     0,     0,    44,
       0,     0,     0,     0,     0,     6,     0,     0,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    17,    18,
      19,    20,     0,     0,     0,     0,     0,     0,     0,     7,
       8,     9,    10,    11,    12,    13,    14,    15,    16,    17,
      18,    19,    20,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    21,    22,    23,    24,    25,     0,    26,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    21,    22,    23,    24,    25,     0,    26,   125,
       0,   126,   127,   128,   129,   130,   131,     0,     0,     0,
     132,   133,     0,     0,     0,     0,     0,     0,     0,     0,
      27,    28,    29,    74,    75,    32,    33,    34,    35,     0,
      37,    87,    88,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    39,    40,     0,    42,     0,     0,     0,
       0,     0,    87,    88,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    39,    40,     0,    42,     0,     0,
       0,   134,     0,   135,     0,     0,   216,   530,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   137,   138,   139,
     140,   141,     0,     0,     0,     0,     0,   125,   142,   126,
     127,   128,   129,   130,   131,     0,     0,     0,   132,   133,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   134,
       0,   135,     0,     0,   216,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   137,   138,   139,   140,   141,
       0,     0,     0,     0,     0,     0,   142
};

static const short yycheck[] =
{
       1,     1,    80,   105,     1,    36,    94,    80,   150,    47,
     318,    42,   283,   316,   102,   154,    47,   118,   321,   162,
      57,   180,   123,   294,   223,     4,   162,   223,    66,   162,
      61,    10,   173,   174,   241,    66,   246,   240,   162,   249,
     476,    78,   241,   246,    91,    24,    93,   254,   443,   444,
      91,   192,    93,   249,   132,   133,   134,    94,   136,    96,
      52,    53,    99,   136,   162,   162,   502,   105,   251,   240,
     253,   223,   150,   151,   105,   112,     4,   114,   151,   162,
     182,   183,   170,   254,   186,   233,   481,   223,   230,   241,
     223,    70,   487,   232,    86,   173,   239,   215,   406,    91,
     241,    93,   190,   262,   245,   413,   248,   243,   209,   412,
     243,   254,   251,   252,   253,    94,    95,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   271,
     254,   178,   175,   176,   222,   213,   246,   178,   216,   249,
     213,   219,   537,   216,   181,   243,   243,   455,   186,   227,
     187,   223,   230,   239,   227,   186,   194,   193,   241,   240,
     240,   247,   239,   194,   201,   246,   246,   223,   239,   440,
     248,   241,   173,   173,    43,   245,   173,   193,   157,   158,
     159,   160,   161,   239,   163,   241,   178,    56,   240,   240,
     171,   172,   270,   271,   246,   246,    65,   270,   240,    68,
     246,   279,   304,   249,   246,   283,   279,   478,   244,   249,
     246,    80,   244,   243,   257,   258,   294,   249,   182,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   244,    98,
     246,   240,   246,   246,   162,   249,   164,   165,   166,   167,
     168,   169,   240,   249,   113,   173,   174,   193,   246,   239,
     229,   230,   240,   232,   342,   180,   193,   243,   246,   254,
     255,   256,   533,   405,   535,   177,   178,   136,   246,   247,
     308,   258,   259,   260,   261,   134,   135,   308,   549,   193,
     205,   162,   283,   283,   248,   193,   283,   429,   254,   255,
     162,   256,   257,   294,   294,   249,   205,   294,   244,   244,
     246,   443,   444,   262,   263,   407,   234,   244,   236,   246,
     243,   239,   240,   391,   392,   239,   394,   248,   391,   392,
     243,   239,   250,   251,   252,   253,   254,   405,   197,   239,
     244,   169,   246,   261,   261,   179,   244,   243,   246,   481,
     260,   181,   259,   246,   240,   487,    52,   216,   273,   241,
     219,   429,    58,    59,   241,   239,   434,   435,   436,   254,
     240,   434,   440,   436,   273,   443,   444,   243,   239,   407,
     248,   239,   410,   170,   170,   240,   407,   241,   162,   410,
      86,   239,   307,   249,   239,   239,   249,   244,    94,    95,
     242,   239,   248,   394,   394,   537,   102,   394,   476,   477,
     478,   248,   239,   481,   477,   239,   239,   239,   239,   487,
     239,   449,   450,   240,   240,   239,   239,   242,   449,   450,
     242,   249,   198,   242,   502,   162,   504,   505,   248,   159,
       1,   504,   505,   240,   435,   435,   240,   242,   435,   440,
     440,   241,   240,   440,   239,   246,   240,   316,   240,   318,
     240,   170,   321,   246,   379,   533,   201,   535,   246,   537,
     249,   243,   249,   249,   242,   248,   246,   244,   162,   242,
     379,   549,   246,   240,   246,   476,   476,   478,   478,   476,
     249,   478,   247,   240,   240,   247,   265,   392,   269,   264,
     415,   208,     1,   418,   419,   420,   266,   268,     1,   267,
      86,   502,   502,    93,   101,   502,   415,    70,   307,   418,
     419,   420,   206,   207,   208,   209,   210,   211,   212,   213,
     214,   464,   216,   219,   220,   221,   222,   502,   224,   225,
     226,   227,   533,   533,   535,   535,   533,   406,   535,   452,
     522,   466,   519,   412,   413,   239,   435,   241,   549,   549,
      49,   394,   549,     1,    -1,   549,     4,   466,   435,   484,
     254,    -1,    10,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   497,   498,   499,   484,    24,   206,   207,   208,
     209,   210,   211,   212,   213,   214,   455,   216,   497,   498,
     499,   162,    -1,   164,   165,   166,   167,   168,   169,    -1,
      -1,    -1,   173,   174,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   539,   540,   541,    -1,    -1,    -1,
      -1,   546,    70,    -1,    -1,   254,    -1,    -1,    -1,    -1,
     539,   540,   541,    -1,    -1,    -1,    -1,   546,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    94,    95,    96,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
      -1,    -1,    -1,   234,    -1,   236,    -1,    -1,   239,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   249,   250,
     251,   252,   253,   254,    -1,    -1,    -1,    -1,    -1,    -1,
     261,   206,   207,   208,   209,   210,   211,   212,   213,   214,
      -1,   216,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   157,
     158,   159,   160,   161,   162,   163,   164,   165,   166,   167,
     168,   169,    -1,    -1,    -1,   173,   174,    -1,    -1,    -1,
       1,    -1,    -1,     4,    -1,    -1,    -1,    -1,    -1,    10,
      -1,    -1,    -1,    -1,    -1,    -1,   194,   195,   196,   197,
     198,   199,   200,    24,   202,   203,   204,   205,   206,   207,
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
      24,   202,   203,   204,   205,   206,   207,   208,   209,   210,
     211,   212,   213,   214,   215,   216,    -1,    -1,    -1,    -1,
      -1,    -1,   223,    -1,    -1,    -1,    -1,    -1,   229,   230,
     231,   232,   233,   234,    -1,   236,   237,    -1,   239,    -1,
      -1,    -1,   243,    -1,    -1,    -1,    70,    -1,   249,   250,
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
     244,    -1,    -1,    70,    -1,   249,   250,   251,   252,   253,
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
     237,    -1,   239,    -1,    -1,    -1,   243,   244,    -1,    -1,
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
      -1,    -1,    -1,   173,   174,    -1,    -1,    -1,     1,    -1,
      -1,     4,    -1,    -1,    -1,    -1,    -1,    10,    -1,    -1,
      -1,    -1,    -1,    -1,   194,   195,   196,   197,   198,   199,
     200,    24,   202,   203,    -1,    -1,   206,   207,   208,   209,
     210,   211,   212,   213,   214,   215,   216,    -1,    -1,    -1,
      -1,    -1,    -1,   223,    -1,    -1,    -1,    -1,    -1,   229,
     230,   231,   232,   233,   234,    -1,   236,   237,    -1,   239,
      -1,    -1,    -1,   243,    -1,    -1,    -1,    70,    -1,   249,
     250,   251,   252,   253,   254,    -1,    -1,    -1,    -1,    -1,
      -1,   261,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    94,    95,    96,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   157,   158,   159,   160,   161,   162,
     163,   164,   165,   166,   167,   168,   169,    -1,    -1,    -1,
     173,   174,    -1,    -1,    -1,     1,    -1,    -1,     4,    -1,
      -1,    -1,    -1,    -1,    10,    -1,    -1,    -1,    -1,    -1,
      -1,   194,   195,   196,   197,   198,   199,   200,    24,   202,
     203,    -1,    -1,   206,   207,   208,   209,   210,   211,   212,
     213,   214,   215,   216,    -1,    -1,    -1,    -1,    -1,    -1,
     223,    -1,    -1,    -1,    -1,    -1,   229,   230,   231,   232,
     233,   234,    -1,   236,   237,    -1,   239,    -1,    -1,    -1,
     243,    -1,    -1,    -1,    70,    -1,   249,   250,   251,   252,
     253,   254,    -1,    -1,    -1,    -1,    -1,    -1,   261,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    94,    95,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   157,   158,   159,   160,   161,   162,   163,   164,   165,
     166,   167,   168,   169,    -1,    -1,     4,   173,   174,    -1,
      -1,    -1,    10,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    24,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     206,   207,   208,   209,   210,   211,   212,   213,   214,   215,
     216,    -1,    -1,    -1,    -1,    -1,    -1,   223,    -1,    -1,
      -1,    -1,    -1,   229,   230,   231,   232,   233,   234,    -1,
     236,   237,    70,   239,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   249,   250,   251,   252,   253,   254,    -1,
      -1,    -1,    -1,    -1,    -1,   261,    94,    95,    96,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   157,
     158,   159,   160,   161,   162,   163,   164,   165,   166,   167,
     168,   169,    -1,    -1,     4,   173,   174,    -1,    -1,    -1,
      10,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    24,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   206,   207,
     208,   209,   210,   211,   212,   213,   214,   162,   216,   164,
     165,   166,   167,   168,   169,    -1,    -1,    -1,   173,   174,
      -1,   229,   230,    -1,   232,    -1,   234,    -1,   236,   237,
      70,   239,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   250,   251,   252,   253,   254,    -1,    -1,    -1,
      -1,    -1,    -1,   261,    94,    95,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   234,
      -1,   236,    -1,    -1,   239,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   249,   250,   251,   252,   253,   254,
      -1,    -1,    -1,    -1,    -1,    -1,   261,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   157,   158,   159,
     160,   161,   162,   163,   164,   165,   166,   167,   168,   169,
      -1,    -1,     4,   173,   174,    -1,    -1,    -1,    10,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    24,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   206,   207,   208,   209,
     210,   211,   212,   213,   214,   162,   216,   164,   165,   166,
     167,   168,   169,    -1,    -1,    -1,   173,   174,    -1,   229,
     230,    -1,   232,    -1,   234,    -1,   236,   237,    70,   239,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     250,   251,   252,   253,   254,    -1,    -1,    -1,    -1,    -1,
      -1,   261,    94,    95,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   234,    -1,   236,
      -1,    -1,   239,    -1,    -1,    -1,   243,    -1,    -1,    -1,
      -1,    -1,    -1,   250,   251,   252,   253,   254,    -1,    -1,
      -1,    -1,    -1,    -1,   261,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   157,   158,   159,   160,   161,
     162,   163,   164,   165,   166,   167,   168,   169,    -1,     0,
      -1,   173,   174,     4,    -1,    -1,    -1,    -1,    -1,    10,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    24,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   206,   207,   208,   209,   210,   211,
     212,   213,   214,   162,   216,   164,   165,   166,   167,   168,
     169,    -1,    -1,    -1,   173,   174,    -1,   229,   230,    -1,
     232,    -1,   234,    -1,   236,   237,    -1,   239,    -1,    70,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   250,   251,
     252,   253,   254,    -1,    -1,    -1,    -1,    -1,    -1,   261,
      -1,    -1,    -1,    94,    95,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   234,    -1,   236,    -1,    -1,
     239,   240,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   250,   251,   252,   253,   254,    -1,    -1,    -1,    -1,
      -1,    -1,   261,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   157,   158,   159,   160,
     161,     4,   163,    -1,    -1,    -1,    -1,    10,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    24,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   206,   207,   208,   209,   210,
     211,   212,   213,   214,   215,   216,    -1,    -1,    -1,    -1,
      -1,    -1,   223,    -1,    -1,    -1,    -1,    70,   229,   230,
     231,   232,   233,    -1,    -1,   162,   237,   164,   165,   166,
     167,   168,   169,    -1,    -1,    -1,   173,   174,   249,    -1,
      -1,    94,    95,    96,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   234,    -1,   236,
      -1,    -1,   239,     4,   157,   158,   159,   160,   161,    10,
     163,    -1,    -1,   250,   251,   252,   253,   254,    -1,    -1,
      -1,    -1,    -1,    24,   261,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,     4,    -1,    -1,    -1,    -1,    -1,    10,    -1,
      -1,    -1,    -1,   206,   207,   208,   209,   210,   211,   212,
     213,   214,    24,   216,    -1,    -1,    -1,    -1,    -1,    70,
      -1,    -1,    -1,    -1,    -1,    -1,   229,   230,    -1,   232,
      -1,    -1,    -1,    -1,   237,    -1,    -1,    -1,    -1,    -1,
      -1,   244,    -1,    94,    95,    96,    97,    98,    99,   100,
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
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   229,   230,
      -1,   232,    -1,     4,    -1,    -1,   237,    -1,    -1,    10,
      -1,    -1,    -1,   244,   206,   207,   208,   209,   210,   211,
     212,   213,   214,    24,   216,    -1,    -1,    -1,    -1,    -1,
      70,    -1,    -1,    -1,    -1,    -1,    -1,   229,   230,    -1,
     232,    -1,    -1,    -1,    -1,   237,    -1,    -1,    -1,    -1,
      -1,    -1,   244,    -1,    94,    95,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,    -1,    70,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    94,    95,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   157,   158,   159,
     160,   161,    -1,   163,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,     4,   157,   158,   159,   160,
     161,    10,   163,    -1,    -1,    -1,   206,   207,   208,   209,
     210,   211,   212,   213,   214,    24,   216,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   229,
     230,    -1,   232,    -1,    -1,    -1,     4,   237,    -1,    -1,
      -1,    -1,    10,    -1,   244,   206,   207,   208,   209,   210,
     211,   212,   213,   214,   215,   216,    24,    -1,    -1,    -1,
      -1,    70,   223,    -1,    -1,    -1,    -1,    -1,   229,   230,
      -1,   232,   233,    -1,    -1,    -1,   237,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    94,    95,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,    -1,
      -1,    -1,    70,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    94,    95,    96,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   157,   158,
     159,   160,   161,    -1,   163,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,     4,   157,
     158,   159,   160,   161,    10,   163,    -1,   206,   207,   208,
     209,   210,   211,   212,   213,   214,   215,   216,    24,     4,
      -1,    -1,    -1,    -1,    -1,    10,    -1,    -1,    -1,    -1,
     229,   230,    -1,   232,    -1,    -1,    -1,    -1,   237,    24,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   206,   207,
     208,   209,   210,   211,   212,   213,   214,    -1,   216,    -1,
      -1,    -1,    -1,    -1,    70,    -1,    -1,    -1,    -1,    -1,
      -1,   229,   230,    -1,   232,    -1,    -1,    -1,    -1,   237,
      -1,    -1,    -1,    -1,    -1,    70,    -1,    -1,    94,    95,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   157,   158,   159,   160,   161,    -1,   163,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   157,   158,   159,   160,   161,    -1,   163,   162,
      -1,   164,   165,   166,   167,   168,   169,    -1,    -1,    -1,
     173,   174,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     206,   207,   208,   209,   210,   211,   212,   213,   214,    -1,
     216,   217,   218,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   229,   230,    -1,   232,    -1,    -1,    -1,
      -1,    -1,   217,   218,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   229,   230,    -1,   232,    -1,    -1,
      -1,   234,    -1,   236,    -1,    -1,   239,   240,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   250,   251,   252,
     253,   254,    -1,    -1,    -1,    -1,    -1,   162,   261,   164,
     165,   166,   167,   168,   169,    -1,    -1,    -1,   173,   174,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   234,
      -1,   236,    -1,    -1,   239,    -1,    -1,    -1,    -1,    -1,
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
     162,   239,   254,   308,   309,   310,   307,   307,   162,   243,
     320,   249,   327,   322,   327,   239,   299,   317,   243,   162,
     296,   297,   162,   243,   310,   162,   164,   165,   166,   167,
     168,   169,   173,   174,   234,   236,   239,   250,   251,   252,
     253,   254,   261,   264,   265,   266,   267,   269,   270,   271,
     272,   273,   274,   276,   277,   278,   279,   280,   281,   282,
     283,   284,   285,   286,   287,   288,   289,   290,   292,   326,
     327,   249,   244,   355,   304,   303,   306,   303,   305,   306,
     241,   307,   308,   310,   312,   307,   310,   327,   310,   309,
     307,   308,   162,   239,   312,   322,   243,   334,   162,   243,
     310,   327,   310,   321,   296,   248,   193,   244,   246,   243,
     296,   239,   239,   239,   276,   276,   239,   271,   276,   239,
     271,   292,   327,   169,   173,   174,   192,   241,   245,   240,
     246,   243,   275,     4,   240,   290,   292,   182,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   248,   291,   274,
     276,   254,   255,   256,   251,   253,   171,   172,   175,   176,
     257,   258,   177,   178,   261,   260,   259,   179,   181,   180,
     262,   246,   240,   241,   308,     1,   162,   194,   195,   196,
     197,   198,   199,   200,   202,   203,   243,   249,   292,   298,
     348,   349,   350,   351,   356,   357,   358,   364,   370,   303,
     306,   289,   293,   311,   310,   312,   312,   241,   312,   322,
     310,   239,   240,   308,   322,   248,   333,   327,   337,   338,
     243,   336,   239,   310,   239,   219,   220,   221,   222,   224,
     225,   226,   227,   323,   325,   193,   244,   293,   297,   296,
     193,   244,   327,   276,   240,   240,   308,   170,   268,   292,
     170,   290,   274,   240,   193,   244,   290,   274,   274,   274,
     278,   278,   279,   279,   280,   280,   280,   280,   281,   281,
     282,   283,   284,   285,   286,   287,   292,   290,   293,   241,
     244,   249,   247,   249,   249,   249,   292,   162,   365,   366,
     349,   239,   239,   244,   352,   249,   244,   349,   242,   312,
     311,   322,   248,   239,   248,   318,   337,   310,   339,   340,
     244,   338,   335,   337,   239,   239,   239,   239,   239,   239,
     239,   240,   246,   193,   244,   308,   240,   240,   242,   247,
     242,   293,   249,   249,   239,   239,   367,   198,   292,   268,
     356,   242,   248,   315,   319,   243,   290,   341,   342,   244,
     312,   322,   246,   249,   322,   337,   244,   293,   162,   159,
     293,   293,   293,   240,   324,   240,   241,   290,   242,   292,
       1,   348,   357,     1,   292,   368,   369,   239,   240,   240,
     244,   316,   342,   342,   241,   245,   343,   344,   345,   347,
     322,   322,   340,   244,   240,   240,   240,   246,   246,   246,
     325,   293,   240,   249,   249,   249,   350,   353,   354,   292,
     349,   363,   243,   361,   342,   293,   170,   193,   244,   246,
     342,   248,   346,   293,   293,   293,   242,   353,   240,   292,
     240,   292,   240,   201,   244,   362,   242,   344,   347,   246,
     246,   246,   240,   240,   249,   349,   204,   205,   349,   359,
     360,   342,   293,   293,   293,   293,   247,   244,   360,   240,
     240,   240,   247
};

#if ! defined (YYSIZE_T) && defined (__SIZE_TYPE__)
# define YYSIZE_T __SIZE_TYPE__
#endif
#if ! defined (YYSIZE_T) && defined (size_t)
# define YYSIZE_T size_t
#endif
#if ! defined (YYSIZE_T)
# if defined (__STDC__) || defined (__cplusplus)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# endif
#endif
#if ! defined (YYSIZE_T)
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

# ifndef YYFPRINTF

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
#ifndef    YYINITDEPTH
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

# ifndef yystrlen
#  if defined (__GLIBC__) && defined (_STRING_H)
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
static YYSIZE_T
#   if defined (__STDC__) || defined (__cplusplus)
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

# ifndef yystpcpy
#  if defined (__GLIBC__) && defined (_STRING_H) && defined (_GNU_SOURCE)
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
#   if defined (__STDC__) || defined (__cplusplus)
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
# ifdef YYPRINT
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
# if defined (__STDC__) || defined (__cplusplus)
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
# if defined (__STDC__) || defined (__cplusplus)
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
# ifndef YYSTACK_RELOCATE
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
#line 608 "gc_cl.y"
    { 
                    clsDECL decl;
                    decl = clParseQualifiedType(Compiler, yyvsp[-1].typeQualifierList, gcvTRUE, &yyvsp[-2].decl);
                    yyval.paramName = clParseParameterDecl(Compiler, &decl, &yyvsp[0].token);
                ;}
    break;

  case 130:
#line 614 "gc_cl.y"
    {
                    clsDECL decl;
                    decl = clParseQualifiedType(Compiler, yyvsp[-2].typeQualifierList, gcvTRUE, &yyvsp[-3].decl);
            yyval.paramName = clParseArrayParameterDecl(Compiler, &decl, &yyvsp[-1].token, yyvsp[0].expr);
                ;}
    break;

  case 131:
#line 623 "gc_cl.y"
    { yyval.paramName = clParseQualifiedParameterDecl(Compiler, gcvNULL, gcvNULL, yyvsp[0].paramName); ;}
    break;

  case 132:
#line 625 "gc_cl.y"
    { yyval.paramName = clParseQualifiedParameterDecl(Compiler, gcvNULL, &yyvsp[-1].token, yyvsp[0].paramName); ;}
    break;

  case 133:
#line 627 "gc_cl.y"
    { yyval.paramName = clParseQualifiedParameterDecl(Compiler, gcvNULL, gcvNULL, yyvsp[0].paramName); ;}
    break;

  case 134:
#line 629 "gc_cl.y"
    { yyval.paramName = clParseQualifiedParameterDecl(Compiler, gcvNULL, &yyvsp[-1].token, yyvsp[0].paramName); ;}
    break;

  case 135:
#line 631 "gc_cl.y"
    { yyval.paramName = clParseQualifiedParameterDecl(Compiler, yyvsp[-1].typeQualifierList, gcvNULL, yyvsp[0].paramName); ;}
    break;

  case 136:
#line 633 "gc_cl.y"
    { yyval.paramName = clParseQualifiedParameterDecl(Compiler, yyvsp[-2].typeQualifierList, &yyvsp[-1].token, yyvsp[0].paramName); ;}
    break;

  case 137:
#line 635 "gc_cl.y"
    { yyval.paramName = clParseQualifiedParameterDecl(Compiler, yyvsp[-1].typeQualifierList, gcvNULL, yyvsp[0].paramName); ;}
    break;

  case 138:
#line 637 "gc_cl.y"
    { yyval.paramName = clParseQualifiedParameterDecl(Compiler, yyvsp[-2].typeQualifierList, &yyvsp[-1].token, yyvsp[0].paramName); ;}
    break;

  case 139:
#line 642 "gc_cl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 140:
#line 644 "gc_cl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 141:
#line 649 "gc_cl.y"
    { yyval.paramName = clParseParameterDecl(Compiler, &yyvsp[0].decl, gcvNULL); ;}
    break;

  case 142:
#line 651 "gc_cl.y"
    { yyval.paramName = clParseArrayParameterDecl(Compiler, &yyvsp[-1].decl, gcvNULL, yyvsp[0].expr); ;}
    break;

  case 143:
#line 654 "gc_cl.y"
    { 
                    clsDECL decl;
                    decl = clParseQualifiedType(Compiler, yyvsp[0].typeQualifierList, gcvTRUE, &yyvsp[-1].decl);
            yyval.paramName = clParseParameterDecl(Compiler, &decl, gcvNULL);
                ;}
    break;

  case 144:
#line 660 "gc_cl.y"
    { 
                    clsDECL decl;
                    decl = clParseQualifiedType(Compiler, yyvsp[-1].typeQualifierList, gcvTRUE, &yyvsp[-2].decl);
            yyval.paramName = clParseArrayParameterDecl(Compiler, &decl, gcvNULL, yyvsp[0].expr);
                ;}
    break;

  case 145:
#line 669 "gc_cl.y"
    {
            yyval.typeQualifierList = clParseTypeQualifierList(Compiler, &yyvsp[0].token,
                                          clParseEmptyTypeQualifierList(Compiler));
        ;}
    break;

  case 146:
#line 674 "gc_cl.y"
    {yyval.typeQualifierList = clParseTypeQualifierList(Compiler, &yyvsp[-1].token, yyvsp[0].typeQualifierList); ;}
    break;

  case 147:
#line 679 "gc_cl.y"
    { yyval.typeQualifierList = clParseEmptyTypeQualifierList(Compiler); ;}
    break;

  case 148:
#line 681 "gc_cl.y"
    { yyval.typeQualifierList = clParsePointerTypeQualifier(Compiler, gcvNULL, yyvsp[0].typeQualifierList); ;}
    break;

  case 149:
#line 683 "gc_cl.y"
    {yyval.typeQualifierList = yyvsp[0].typeQualifierList;;}
    break;

  case 150:
#line 685 "gc_cl.y"
    {yyval.typeQualifierList = clParsePointerTypeQualifier(Compiler, yyvsp[-1].typeQualifierList, yyvsp[0].typeQualifierList); ;}
    break;

  case 151:
#line 689 "gc_cl.y"
    {yyval.token = yyvsp[0].token;;}
    break;

  case 152:
#line 691 "gc_cl.y"
    { 
          yyval.token = yyvsp[0].token;
          yyval.token.u.identifier.ptrDscr = yyvsp[-1].typeQualifierList;
        ;}
    break;

  case 153:
#line 698 "gc_cl.y"
    {  yyval.token = yyvsp[0].token; ;}
    break;

  case 154:
#line 700 "gc_cl.y"
    { yyval.token = yyvsp[-1].token; ;}
    break;

  case 155:
#line 704 "gc_cl.y"
    { yyval.expr = clParseNullExpr(Compiler, &yyvsp[0].token); ;}
    break;

  case 156:
#line 706 "gc_cl.y"
    { yyval.expr = yyvsp[0].expr; ;}
    break;

  case 157:
#line 711 "gc_cl.y"
    { yyval.expr = yyvsp[-1].expr; ;}
    break;

  case 158:
#line 713 "gc_cl.y"
    { yyval.expr = clParseArrayDeclarator(Compiler, &yyvsp[-2].token, yyvsp[-3].expr, yyvsp[-1].expr); ;}
    break;

  case 159:
#line 718 "gc_cl.y"
    { yyval.declOrDeclList = yyvsp[0].declOrDeclList; ;}
    break;

  case 160:
#line 720 "gc_cl.y"
    {
               cloCOMPILER_PushParserState(Compiler, clvPARSER_IN_TYPEDEF);
        ;}
    break;

  case 161:
#line 724 "gc_cl.y"
    {
           yyval.declOrDeclList = clParseTypeDef(Compiler, yyvsp[0].declOrDeclList);
           cloCOMPILER_PopParserState(Compiler);
        ;}
    break;

  case 162:
#line 729 "gc_cl.y"
    { yyval.declOrDeclList = clParseVariableDeclList(Compiler, yyvsp[-3].declOrDeclList, &yyvsp[-1].token, yyvsp[0].attr); ;}
    break;

  case 163:
#line 731 "gc_cl.y"
    { yyval.declOrDeclList = clParseArrayVariableDeclList(Compiler, yyvsp[-4].declOrDeclList, &yyvsp[-2].token, yyvsp[-1].expr, yyvsp[0].attr); ;}
    break;

  case 164:
#line 733 "gc_cl.y"
    { yyval.declOrDeclList = clParseVariableDeclListInit(Compiler, yyvsp[-4].declOrDeclList, &yyvsp[-2].token, yyvsp[-1].attr); ;}
    break;

  case 165:
#line 735 "gc_cl.y"
    { yyval.declOrDeclList = clParseFinishDeclListInit(Compiler, yyvsp[-1].declOrDeclList, yyvsp[0].expr); ;}
    break;

  case 166:
#line 737 "gc_cl.y"
    { yyval.declOrDeclList = clParseArrayVariableDeclListInit(Compiler, yyvsp[-5].declOrDeclList, &yyvsp[-3].token, yyvsp[-2].expr, yyvsp[-1].attr); ;}
    break;

  case 167:
#line 739 "gc_cl.y"
    { yyval.declOrDeclList = clParseFinishDeclListInit(Compiler, yyvsp[-1].declOrDeclList, yyvsp[0].expr); ;}
    break;

  case 168:
#line 745 "gc_cl.y"
    { yyval.declOrDeclList = clParseFuncDecl(Compiler, yyvsp[0].funcName); ;}
    break;

  case 169:
#line 747 "gc_cl.y"
    { yyval.declOrDeclList = clParseVariableDecl(Compiler, &yyvsp[-2].decl, &yyvsp[-1].token, yyvsp[0].attr); ;}
    break;

  case 170:
#line 749 "gc_cl.y"
    { yyval.declOrDeclList = clParseArrayVariableDecl(Compiler, &yyvsp[-3].decl, &yyvsp[-2].token, yyvsp[-1].expr, yyvsp[0].attr); ;}
    break;

  case 171:
#line 751 "gc_cl.y"
    { yyval.declOrDeclList = clParseVariableDeclInit(Compiler, &yyvsp[-3].decl, &yyvsp[-2].token, yyvsp[-1].attr); ;}
    break;

  case 172:
#line 753 "gc_cl.y"
    { yyval.declOrDeclList = clParseFinishDeclInit(Compiler, yyvsp[-1].declOrDeclList, yyvsp[0].expr); ;}
    break;

  case 173:
#line 755 "gc_cl.y"
    { yyval.declOrDeclList = clParseArrayVariableDeclInit(Compiler, &yyvsp[-4].decl, &yyvsp[-3].token, yyvsp[-2].expr, yyvsp[-1].attr); ;}
    break;

  case 174:
#line 757 "gc_cl.y"
    { yyval.declOrDeclList = clParseFinishDeclInit(Compiler, yyvsp[-1].declOrDeclList, yyvsp[0].expr); ;}
    break;

  case 175:
#line 764 "gc_cl.y"
    { yyval.attr = gcvNULL; ;}
    break;

  case 176:
#line 766 "gc_cl.y"
    { yyval.attr = yyvsp[-2].attr; ;}
    break;

  case 177:
#line 770 "gc_cl.y"
    { yyval.attr = gcvNULL; ;}
    break;

  case 178:
#line 772 "gc_cl.y"
    { yyval.attr = yyvsp[0].attr; ;}
    break;

  case 179:
#line 776 "gc_cl.y"
    { yyval.attr = yyvsp[0].attr; ;}
    break;

  case 180:
#line 778 "gc_cl.y"
    { yyval.attr = yyvsp[0].attr; ;}
    break;

  case 181:
#line 780 "gc_cl.y"
    { yyval.attr = yyvsp[-1].attr; ;}
    break;

  case 182:
#line 782 "gc_cl.y"
    { yyval.attr = yyvsp[0].attr; ;}
    break;

  case 183:
#line 787 "gc_cl.y"
    { yyval.attr = clParseAttributeEndianType(Compiler, yyvsp[-4].attr,  &yyvsp[-1].token); ;}
    break;

  case 184:
#line 789 "gc_cl.y"
    { yyval.attr = clParseSimpleAttribute(Compiler, &yyvsp[0].token, clvATTR_PACKED, yyvsp[-1].attr); ;}
    break;

  case 185:
#line 791 "gc_cl.y"
    { yyval.attr = clParseAttributeVecTypeHint(Compiler, yyvsp[-4].attr, &yyvsp[-1].token); ;}
    break;

  case 186:
#line 793 "gc_cl.y"
    { yyval.attr = clParseAttributeReqdWorkGroupSize(Compiler, yyvsp[-8].attr, yyvsp[-5].expr, yyvsp[-3].expr, yyvsp[-1].expr); ;}
    break;

  case 187:
#line 795 "gc_cl.y"
    { yyval.attr = clParseAttributeWorkGroupSizeHint(Compiler, yyvsp[-8].attr, yyvsp[-5].expr, yyvsp[-3].expr, yyvsp[-1].expr); ;}
    break;

  case 188:
#line 797 "gc_cl.y"
    { yyval.attr = clParseAttributeKernelScaleHint(Compiler, yyvsp[-8].attr, yyvsp[-5].expr, yyvsp[-3].expr, yyvsp[-1].expr); ;}
    break;

  case 189:
#line 799 "gc_cl.y"
    { yyval.attr = clParseAttributeAligned(Compiler, yyvsp[-1].attr, gcvNULL); ;}
    break;

  case 190:
#line 801 "gc_cl.y"
    { yyval.attr = clParseAttributeAligned(Compiler, yyvsp[-4].attr, yyvsp[-1].expr); ;}
    break;

  case 191:
#line 803 "gc_cl.y"
    { yyval.attr = clParseSimpleAttribute(Compiler, &yyvsp[0].token, clvATTR_ALWAYS_INLINE, yyvsp[-1].attr); ;}
    break;

  case 192:
#line 808 "gc_cl.y"
    { yyval.decl = clParseCreateDecl(Compiler, &yyvsp[0].decl, gcvNULL, gcvNULL); ;}
    break;

  case 193:
#line 810 "gc_cl.y"
    { yyval.decl = clParseCreateDecl(Compiler, &yyvsp[-3].decl, gcvNULL, yyvsp[-1].expr); ;}
    break;

  case 194:
#line 812 "gc_cl.y"
    { yyval.decl = clParseCreateDecl(Compiler, &yyvsp[-1].decl, yyvsp[0].typeQualifierList, gcvNULL); ;}
    break;

  case 195:
#line 814 "gc_cl.y"
    { yyval.decl = clParseCreateDecl(Compiler, &yyvsp[-4].decl, yyvsp[-3].typeQualifierList, yyvsp[-1].expr); ;}
    break;

  case 196:
#line 816 "gc_cl.y"
    { yyval.decl = clParseCreateDecl(Compiler, &yyvsp[-5].decl, yyvsp[-4].typeQualifierList, yyvsp[-1].expr); ;}
    break;

  case 197:
#line 818 "gc_cl.y"
    { yyval.decl = clParseCreateDeclFromExpression(Compiler, yyvsp[0].expr); ;}
    break;

  case 198:
#line 823 "gc_cl.y"
    { yyval.decl = clParseQualifiedType(Compiler, gcvNULL, gcvFALSE, &yyvsp[0].decl); ;}
    break;

  case 199:
#line 825 "gc_cl.y"
    { yyval.decl = clParseQualifiedType(Compiler, yyvsp[-1].typeQualifierList, gcvFALSE, &yyvsp[0].decl); ;}
    break;

  case 200:
#line 827 "gc_cl.y"
    { yyval.decl = clParseQualifiedType(Compiler, yyvsp[0].typeQualifierList, gcvFALSE, &yyvsp[-1].decl); ;}
    break;

  case 201:
#line 829 "gc_cl.y"
    {
                    clsDECL decl;

                    decl = clParseQualifiedType(Compiler, yyvsp[-2].typeQualifierList, gcvFALSE, &yyvsp[-1].decl);
                    yyval.decl = clParseQualifiedType(Compiler, yyvsp[0].typeQualifierList, gcvFALSE, &decl);
                ;}
    break;

  case 202:
#line 836 "gc_cl.y"
    { yyval.decl = yyvsp[-1].decl; ;}
    break;

  case 203:
#line 841 "gc_cl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 204:
#line 843 "gc_cl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 205:
#line 845 "gc_cl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 206:
#line 847 "gc_cl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 207:
#line 849 "gc_cl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 208:
#line 851 "gc_cl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 209:
#line 853 "gc_cl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 210:
#line 855 "gc_cl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 211:
#line 857 "gc_cl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 212:
#line 859 "gc_cl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 213:
#line 864 "gc_cl.y"
    { yyval.decl = clParseNonStructType(Compiler, &yyvsp[0].token); ;}
    break;

  case 214:
#line 866 "gc_cl.y"
    { yyval.decl = clParseCreateDeclFromDataType(Compiler, yyvsp[0].dataType); ;}
    break;

  case 215:
#line 868 "gc_cl.y"
    { yyval.decl = clParseCreateDeclFromDataType(Compiler, yyvsp[0].dataType); ;}
    break;

  case 216:
#line 870 "gc_cl.y"
    { yyval.decl = clParseCreateDeclFromDataType(Compiler, yyvsp[0].dataType); ;}
    break;

  case 217:
#line 875 "gc_cl.y"
    { yyval.token = yyvsp[0].token;}
    break;

  case 218:
#line 877 "gc_cl.y"
    { yyval.token = yyvsp[0].token;}
    break;

  case 219:
#line 879 "gc_cl.y"
    { yyval.token = yyvsp[0].token;}
    break;

  case 220:
#line 881 "gc_cl.y"
    { yyval.token = yyvsp[0].token;}
    break;

  case 221:
#line 883 "gc_cl.y"
    { yyval.token = yyvsp[0].token;}
    break;

  case 222:
#line 885 "gc_cl.y"
    { yyval.token = yyvsp[0].token;}
    break;

  case 223:
#line 887 "gc_cl.y"
    { yyval.token = yyvsp[0].token;}
    break;

  case 224:
#line 889 "gc_cl.y"
    { yyval.token = yyvsp[0].token;}
    break;

  case 225:
#line 891 "gc_cl.y"
    { yyval.token = yyvsp[0].token;}
    break;

  case 226:
#line 893 "gc_cl.y"
    { yyval.token = yyvsp[0].token;}
    break;

  case 227:
#line 895 "gc_cl.y"
    { yyval.token = yyvsp[0].token;}
    break;

  case 228:
#line 897 "gc_cl.y"
    { yyval.token = yyvsp[0].token;}
    break;

  case 229:
#line 899 "gc_cl.y"
    { yyval.token = yyvsp[0].token;}
    break;

  case 230:
#line 901 "gc_cl.y"
    { yyval.token = yyvsp[0].token;}
    break;

  case 231:
#line 903 "gc_cl.y"
    { yyval.token = yyvsp[0].token;}
    break;

  case 232:
#line 905 "gc_cl.y"
    { yyval.token = yyvsp[0].token;}
    break;

  case 233:
#line 907 "gc_cl.y"
    { yyval.token = yyvsp[0].token;}
    break;

  case 234:
#line 909 "gc_cl.y"
    { yyval.token = yyvsp[0].token;}
    break;

  case 235:
#line 911 "gc_cl.y"
    { yyval.token = yyvsp[0].token;}
    break;

  case 236:
#line 913 "gc_cl.y"
    { yyval.token = yyvsp[0].token;}
    break;

  case 237:
#line 915 "gc_cl.y"
    { yyval.token = yyvsp[0].token;}
    break;

  case 238:
#line 917 "gc_cl.y"
    { yyval.token = yyvsp[0].token;}
    break;

  case 239:
#line 919 "gc_cl.y"
    { yyval.token = yyvsp[0].token;}
    break;

  case 240:
#line 921 "gc_cl.y"
    { yyval.token = yyvsp[0].token;}
    break;

  case 241:
#line 925 "gc_cl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 242:
#line 927 "gc_cl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 243:
#line 932 "gc_cl.y"
    { clParseStructDeclBegin(Compiler, &yyvsp[-2].token,  &yyvsp[-1].token); ;}
    break;

  case 244:
#line 934 "gc_cl.y"
    { yyval.dataType = clParseStructDeclEnd(Compiler, &yyvsp[-5].token, yyvsp[0].attr, yyvsp[-2].status); ;}
    break;

  case 245:
#line 936 "gc_cl.y"
    { clParseStructDeclBegin(Compiler, &yyvsp[-1].token, gcvNULL); ;}
    break;

  case 246:
#line 938 "gc_cl.y"
    { yyval.dataType = clParseStructDeclEnd(Compiler, gcvNULL, yyvsp[0].attr, yyvsp[-2].status); ;}
    break;

  case 247:
#line 940 "gc_cl.y"
    { clParseStructDeclBegin(Compiler, &yyvsp[-3].token, &yyvsp[-1].token); ;}
    break;

  case 248:
#line 942 "gc_cl.y"
    { yyval.dataType = clParseStructDeclEnd(Compiler, &yyvsp[-4].token, yyvsp[-5].attr, yyvsp[-1].status); ;}
    break;

  case 249:
#line 944 "gc_cl.y"
    { clParseStructDeclBegin(Compiler, &yyvsp[-2].token, gcvNULL); ;}
    break;

  case 250:
#line 946 "gc_cl.y"
    { yyval.dataType = clParseStructDeclEnd(Compiler, gcvNULL, yyvsp[-4].attr, yyvsp[-1].status); ;}
    break;

  case 251:
#line 951 "gc_cl.y"
    {
           yyval.status = yyvsp[0].status;
        ;}
    break;

  case 252:
#line 955 "gc_cl.y"
    { 
           if(gcmIS_ERROR(yyvsp[-1].status)) {
                       yyval.status = yyvsp[-1].status;
           }
           else {
                       yyval.status = yyvsp[0].status;

           }
        ;}
    break;

  case 253:
#line 968 "gc_cl.y"
    {  
                   yyval.status = clParseTypeSpecifiedFieldDeclList(Compiler, &yyvsp[-2].decl, yyvsp[-1].fieldDeclList);
        ;}
    break;

  case 254:
#line 975 "gc_cl.y"
    { yyval.fieldDeclList = clParseFieldDeclList(Compiler, yyvsp[0].fieldDecl); ;}
    break;

  case 255:
#line 977 "gc_cl.y"
    { yyval.fieldDeclList = clParseFieldDeclList2(Compiler, yyvsp[-2].fieldDeclList, yyvsp[0].fieldDecl); ;}
    break;

  case 256:
#line 981 "gc_cl.y"
    { yyval.fieldDecl = clParseFieldDecl(Compiler, gcvNULL, gcvNULL, gcvNULL); ;}
    break;

  case 257:
#line 983 "gc_cl.y"
    { yyval.fieldDecl = clParseFieldDecl(Compiler, &yyvsp[-1].token, gcvNULL, yyvsp[0].attr); ;}
    break;

  case 258:
#line 985 "gc_cl.y"
    { yyval.fieldDecl = clParseFieldDecl(Compiler, &yyvsp[-2].token, yyvsp[-1].expr, yyvsp[0].attr); ;}
    break;

  case 259:
#line 990 "gc_cl.y"
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

  case 260:
#line 1012 "gc_cl.y"
    { yyval.expr = yyvsp[0].expr; ;}
    break;

  case 261:
#line 1014 "gc_cl.y"
    { yyval.expr = yyvsp[-1].expr; ;}
    break;

  case 262:
#line 1016 "gc_cl.y"
    { yyval.expr = yyvsp[-1].expr; ;}
    break;

  case 263:
#line 1021 "gc_cl.y"
    { yyval.expr = clParseInitializerList(Compiler, yyvsp[-2].expr, yyvsp[-1].declOrDeclList, yyvsp[0].expr); ;}
    break;

  case 264:
#line 1023 "gc_cl.y"
    { yyval.expr = clParseInitializerList(Compiler, yyvsp[-3].expr, yyvsp[-1].declOrDeclList, yyvsp[0].expr); ;}
    break;

  case 265:
#line 1027 "gc_cl.y"
    { yyval.declOrDeclList = gcvNULL; ;}
    break;

  case 266:
#line 1029 "gc_cl.y"
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

  case 267:
#line 1042 "gc_cl.y"
    { yyval.declOrDeclList = yyvsp[0].declOrDeclList; ;}
    break;

  case 268:
#line 1044 "gc_cl.y"
    { 
           yyval.token.type = T_EOF;
        ;}
    break;

  case 269:
#line 1048 "gc_cl.y"
    { yyval.declOrDeclList = yyvsp[0].declOrDeclList; ;}
    break;

  case 270:
#line 1053 "gc_cl.y"
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

  case 271:
#line 1066 "gc_cl.y"
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

  case 272:
#line 1082 "gc_cl.y"
    { yyval.statement = yyvsp[0].statement; ;}
    break;

  case 273:
#line 1087 "gc_cl.y"
    { yyval.statement = clParseCompoundStatementAsStatement(Compiler, yyvsp[0].statements); ;}
    break;

  case 274:
#line 1089 "gc_cl.y"
    { yyval.statement = yyvsp[0].statement; ;}
    break;

  case 275:
#line 1094 "gc_cl.y"
    { yyval.statement = yyvsp[0].statement; ;}
    break;

  case 276:
#line 1096 "gc_cl.y"
    { yyval.statement = yyvsp[0].statement; ;}
    break;

  case 277:
#line 1098 "gc_cl.y"
    { yyval.statement = yyvsp[0].statement; ;}
    break;

  case 278:
#line 1100 "gc_cl.y"
    { yyval.statement = yyvsp[0].statement; ;}
    break;

  case 279:
#line 1102 "gc_cl.y"
    { yyval.statement = yyvsp[0].statement; ;}
    break;

  case 280:
#line 1104 "gc_cl.y"
    { yyval.statement = clParseStatementLabel(Compiler, &yyvsp[-1].token); ;}
    break;

  case 281:
#line 1106 "gc_cl.y"
    { yyclearin;
          yyerrok;
          yyval.statement = gcvNULL; ;}
    break;

  case 282:
#line 1110 "gc_cl.y"
    { yyclearin;
          yyerrok;
          yyval.statement = gcvNULL; ;}
    break;

  case 283:
#line 1117 "gc_cl.y"
    { yyval.statements = gcvNULL; ;}
    break;

  case 284:
#line 1119 "gc_cl.y"
    { clParseCompoundStatementBegin(Compiler); ;}
    break;

  case 285:
#line 1121 "gc_cl.y"
    { yyval.statements = clParseCompoundStatementEnd(Compiler, &yyvsp[-3].token, yyvsp[-1].statements, &yyvsp[0].token); ;}
    break;

  case 286:
#line 1126 "gc_cl.y"
    { yyval.statement = clParseCompoundStatementNoNewScopeAsStatementNoNewScope(Compiler, yyvsp[0].statements); ;}
    break;

  case 287:
#line 1128 "gc_cl.y"
    { yyval.statement = yyvsp[0].statement; ;}
    break;

  case 288:
#line 1133 "gc_cl.y"
    { yyval.statements = gcvNULL; ;}
    break;

  case 289:
#line 1135 "gc_cl.y"
    { clParseCompoundStatementNoNewScopeBegin(Compiler); ;}
    break;

  case 290:
#line 1137 "gc_cl.y"
    { yyval.statements = clParseCompoundStatementNoNewScopeEnd(Compiler, &yyvsp[-3].token, yyvsp[-1].statements, &yyvsp[0].token); ;}
    break;

  case 291:
#line 1142 "gc_cl.y"
    { yyval.statements = clParseStatementList(Compiler, yyvsp[0].statement); ;}
    break;

  case 292:
#line 1144 "gc_cl.y"
    { yyval.statements = clParseStatementList2(Compiler, yyvsp[-1].statements, yyvsp[0].statement); ;}
    break;

  case 293:
#line 1149 "gc_cl.y"
    { yyval.statement = gcvNULL; ;}
    break;

  case 294:
#line 1151 "gc_cl.y"
    { yyval.statement = clParseExprAsStatement(Compiler, yyvsp[-1].expr); ;}
    break;

  case 295:
#line 1156 "gc_cl.y"
    { yyval.statement = clParseIfStatement(Compiler, &yyvsp[-4].token, yyvsp[-2].expr, yyvsp[0].ifStatementPair); ;}
    break;

  case 296:
#line 1158 "gc_cl.y"
    { yyval.statement = clParseSwitchStatement(Compiler, &yyvsp[-4].token, yyvsp[-2].expr, yyvsp[0].statement); ;}
    break;

  case 297:
#line 1163 "gc_cl.y"
    { yyval.statements = clParseStatementList(Compiler, yyvsp[0].statement); ;}
    break;

  case 298:
#line 1165 "gc_cl.y"
    { yyval.statements = clParseStatementList2(Compiler, yyvsp[-1].statements, yyvsp[0].statement); ;}
    break;

  case 299:
#line 1170 "gc_cl.y"
    { yyval.statement = yyvsp[0].statement; ;}
    break;

  case 300:
#line 1172 "gc_cl.y"
    { yyval.statement = clParseCaseStatement(Compiler, &yyvsp[-2].token, yyvsp[-1].expr); ;}
    break;

  case 301:
#line 1174 "gc_cl.y"
    { yyval.statement = clParseDefaultStatement(Compiler, &yyvsp[-1].token); ;}
    break;

  case 302:
#line 1179 "gc_cl.y"
    { yyval.statement = gcvNULL; ;}
    break;

  case 303:
#line 1181 "gc_cl.y"
    { clParseSwitchBodyBegin(Compiler); ;}
    break;

  case 304:
#line 1183 "gc_cl.y"
    { yyval.statement = clParseSwitchBodyEnd(Compiler, &yyvsp[-3].token, yyvsp[-1].statements, &yyvsp[0].token); ;}
    break;

  case 305:
#line 1188 "gc_cl.y"
    { yyval.ifStatementPair = clParseIfSubStatements(Compiler, yyvsp[-2].statement, yyvsp[0].statement); ;}
    break;

  case 306:
#line 1190 "gc_cl.y"
    { yyval.ifStatementPair = clParseIfSubStatements(Compiler, yyvsp[0].statement, gcvNULL); ;}
    break;

  case 307:
#line 1197 "gc_cl.y"
    { clParseWhileStatementBegin(Compiler); ;}
    break;

  case 308:
#line 1199 "gc_cl.y"
    { yyval.statement = clParseWhileStatementEnd(Compiler, &yyvsp[-5].token, yyvsp[-2].expr, yyvsp[0].statement); ;}
    break;

  case 309:
#line 1201 "gc_cl.y"
    { yyval.statement = clParseDoWhileStatement(Compiler, &yyvsp[-6].token, yyvsp[-5].statement, yyvsp[-2].expr); ;}
    break;

  case 310:
#line 1203 "gc_cl.y"
    { clParseForStatementBegin(Compiler); ;}
    break;

  case 311:
#line 1205 "gc_cl.y"
    { yyval.statement = clParseForStatementEnd(Compiler, &yyvsp[-4].token, yyvsp[-2].statement, yyvsp[-1].forExprPair, yyvsp[0].statement); ;}
    break;

  case 312:
#line 1210 "gc_cl.y"
    { yyval.statement = yyvsp[0].statement; ;}
    break;

  case 313:
#line 1212 "gc_cl.y"
    { yyval.statement = yyvsp[0].statement; ;}
    break;

  case 314:
#line 1214 "gc_cl.y"
    { yyclearin;
          yyerrok;
          yyval.statement = gcvNULL; ;}
    break;

  case 315:
#line 1221 "gc_cl.y"
    { yyval.expr = yyvsp[0].expr; ;}
    break;

  case 316:
#line 1223 "gc_cl.y"
    { yyval.expr = gcvNULL; ;}
    break;

  case 317:
#line 1228 "gc_cl.y"
    { yyval.forExprPair = clParseForControl(Compiler, yyvsp[-2].expr, gcvNULL); ;}
    break;

  case 318:
#line 1230 "gc_cl.y"
    { yyval.forExprPair = clParseForControl(Compiler, yyvsp[-3].expr, yyvsp[-1].expr); ;}
    break;

  case 319:
#line 1232 "gc_cl.y"
    { 
          clsForExprPair nullPair = {gcvNULL, gcvNULL};
          yyclearin;
          yyerrok;
          yyval.forExprPair = nullPair; ;}
    break;

  case 320:
#line 1238 "gc_cl.y"
    { 
          clsForExprPair nullPair = {gcvNULL, gcvNULL};
          yyclearin;
          yyerrok;
          yyval.forExprPair = nullPair; ;}
    break;

  case 321:
#line 1247 "gc_cl.y"
    { yyval.statement = clParseJumpStatement(Compiler, clvCONTINUE, &yyvsp[-1].token, gcvNULL); ;}
    break;

  case 322:
#line 1249 "gc_cl.y"
    { yyval.statement = clParseJumpStatement(Compiler, clvBREAK, &yyvsp[-1].token, gcvNULL); ;}
    break;

  case 323:
#line 1251 "gc_cl.y"
    { yyval.statement = clParseJumpStatement(Compiler, clvRETURN, &yyvsp[-1].token, gcvNULL); ;}
    break;

  case 324:
#line 1253 "gc_cl.y"
    { yyval.statement = clParseJumpStatement(Compiler, clvRETURN, &yyvsp[-2].token, yyvsp[-1].expr); ;}
    break;

  case 325:
#line 1255 "gc_cl.y"
    { yyval.statement = clParseGotoStatement(Compiler, &yyvsp[-2].token, &yyvsp[-1].token); ;}
    break;

  case 329:
#line 1267 "gc_cl.y"
    { clParseExternalDecl(Compiler, yyvsp[0].statement); ;}
    break;

  case 331:
#line 1273 "gc_cl.y"
    { clParseFuncDef(Compiler, yyvsp[-1].funcName, yyvsp[0].statements); ;}
    break;


    }

/* Line 991 of yacc.c.  */
#line 4506 "gc_cl_parser.c"

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


#line 1276 "gc_cl.y"



