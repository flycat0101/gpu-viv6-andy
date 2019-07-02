/****************************************************************************
*
*    Copyright (c) 2005 - 2019 by Vivante Corp.  All rights reserved.
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
     T_ATTRIBUTE = 258,
     T_CONST = 259,
     T_BOOL = 260,
     T_FLOAT = 261,
     T_INT = 262,
     T_DOUBLE = 263,
     T_BREAK = 264,
     T_CONTINUE = 265,
     T_DO = 266,
     T_ELSE = 267,
     T_FOR = 268,
     T_IF = 269,
     T_DISCARD = 270,
     T_RETURN = 271,
     T_BASIC_TYPE = 272,
     T_BVEC2 = 273,
     T_BVEC3 = 274,
     T_BVEC4 = 275,
     T_IVEC2 = 276,
     T_IVEC3 = 277,
     T_IVEC4 = 278,
     T_VEC2 = 279,
     T_VEC3 = 280,
     T_VEC4 = 281,
     T_DVEC2 = 282,
     T_DVEC3 = 283,
     T_DVEC4 = 284,
     T_MAT2 = 285,
     T_MAT3 = 286,
     T_MAT4 = 287,
     T_IN = 288,
     T_OUT = 289,
     T_INOUT = 290,
     T_UNIFORM = 291,
     T_VARYING = 292,
     T_PATCH = 293,
     T_SAMPLE = 294,
     T_UINT = 295,
     T_UVEC2 = 296,
     T_UVEC3 = 297,
     T_UVEC4 = 298,
     T_MAT2X3 = 299,
     T_MAT2X4 = 300,
     T_MAT3X2 = 301,
     T_MAT3X4 = 302,
     T_MAT4X2 = 303,
     T_MAT4X3 = 304,
     T_DMAT2 = 305,
     T_DMAT3 = 306,
     T_DMAT4 = 307,
     T_DMAT2X3 = 308,
     T_DMAT2X4 = 309,
     T_DMAT3X2 = 310,
     T_DMAT3X4 = 311,
     T_DMAT4X2 = 312,
     T_DMAT4X3 = 313,
     T_SAMPLER2D = 314,
     T_SAMPLERCUBE = 315,
     T_SAMPLERCUBEARRAY = 316,
     T_SAMPLERCUBESHADOW = 317,
     T_SAMPLERCUBEARRAYSHADOW = 318,
     T_SAMPLER2DSHADOW = 319,
     T_SAMPLER3D = 320,
     T_SAMPLER1DARRAY = 321,
     T_SAMPLER2DARRAY = 322,
     T_SAMPLER1DARRAYSHADOW = 323,
     T_SAMPLER2DARRAYSHADOW = 324,
     T_ISAMPLER2D = 325,
     T_ISAMPLERCUBE = 326,
     T_ISAMPLERCUBEARRAY = 327,
     T_ISAMPLER3D = 328,
     T_ISAMPLER2DARRAY = 329,
     T_USAMPLER2D = 330,
     T_USAMPLERCUBE = 331,
     T_USAMPLERCUBEARRAY = 332,
     T_USAMPLER3D = 333,
     T_USAMPLER2DARRAY = 334,
     T_SAMPLEREXTERNALOES = 335,
     T_SAMPLER2DMS = 336,
     T_ISAMPLER2DMS = 337,
     T_USAMPLER2DMS = 338,
     T_SAMPLER2DMSARRAY = 339,
     T_ISAMPLER2DMSARRAY = 340,
     T_USAMPLER2DMSARRAY = 341,
     T_SAMPLERBUFFER = 342,
     T_ISAMPLERBUFFER = 343,
     T_USAMPLERBUFFER = 344,
     T_SAMPLER1D = 345,
     T_ISAMPLER1D = 346,
     T_USAMPLER1D = 347,
     T_SAMPLER1DSHADOW = 348,
     T_SAMPLER2DRECT = 349,
     T_ISAMPLER2DRECT = 350,
     T_USAMPLER2DRECT = 351,
     T_SAMPLER2DRECTSHADOW = 352,
     T_ISAMPLER1DARRAY = 353,
     T_USAMPLER1DARRAY = 354,
     T_IMAGE2D = 355,
     T_IIMAGE2D = 356,
     T_UIMAGE2D = 357,
     T_IMAGE2DARRAY = 358,
     T_IIMAGE2DARRAY = 359,
     T_UIMAGE2DARRAY = 360,
     T_IMAGE3D = 361,
     T_IIMAGE3D = 362,
     T_UIMAGE3D = 363,
     T_IMAGECUBE = 364,
     T_IIMAGECUBE = 365,
     T_UIMAGECUBE = 366,
     T_IMAGECUBEARRAY = 367,
     T_IIMAGECUBEARRAY = 368,
     T_UIMAGECUBEARRAY = 369,
     T_IMAGEBUFFER = 370,
     T_IIMAGEBUFFER = 371,
     T_UIMAGEBUFFER = 372,
     T_GEN_SAMPLER = 373,
     T_GEN_ISAMPLER = 374,
     T_GEN_USAMPLER = 375,
     T_ATOMIC_UINT = 376,
     T_BUFFER = 377,
     T_SHARED = 378,
     T_STRUCT = 379,
     T_VOID = 380,
     T_WHILE = 381,
     T_IO_BLOCK = 382,
     T_ARRAY4_OF_IVEC2 = 383,
     T_TYPE_MATCH_CALLBACK0 = 384,
     T_TYPE_MATCH_CALLBACK1 = 385,
     T_TYPE_MATCH_CALLBACK2 = 386,
     T_SWITCH = 387,
     T_CASE = 388,
     T_DEFAULT = 389,
     T_CENTROID = 390,
     T_FLAT = 391,
     T_SMOOTH = 392,
     T_NOPERSPECTIVE = 393,
     T_LAYOUT = 394,
     T_UNIFORM_BLOCK = 395,
     T_IDENTIFIER = 396,
     T_TYPE_NAME = 397,
     T_FLOATCONSTANT = 398,
     T_INTCONSTANT = 399,
     T_BOOLCONSTANT = 400,
     T_UINTCONSTANT = 401,
     T_FIELD_SELECTION = 402,
     T_LEFT_OP = 403,
     T_RIGHT_OP = 404,
     T_INC_OP = 405,
     T_DEC_OP = 406,
     T_LE_OP = 407,
     T_GE_OP = 408,
     T_EQ_OP = 409,
     T_NE_OP = 410,
     T_AND_OP = 411,
     T_OR_OP = 412,
     T_XOR_OP = 413,
     T_MUL_ASSIGN = 414,
     T_DIV_ASSIGN = 415,
     T_ADD_ASSIGN = 416,
     T_MOD_ASSIGN = 417,
     T_LEFT_ASSIGN = 418,
     T_RIGHT_ASSIGN = 419,
     T_AND_ASSIGN = 420,
     T_XOR_ASSIGN = 421,
     T_OR_ASSIGN = 422,
     T_SUB_ASSIGN = 423,
     T_LENGTH_METHOD = 424,
     T_INVARIANT = 425,
     T_HIGH_PRECISION = 426,
     T_MEDIUM_PRECISION = 427,
     T_LOW_PRECISION = 428,
     T_PRECISION = 429,
     T_PRECISE = 430,
     T_COHERENT = 431,
     T_VOLATILE = 432,
     T_RESTRICT = 433,
     T_READONLY = 434,
     T_WRITEONLY = 435,
     T_VIV_ASM = 436,
     T_ASM_OPND_BRACKET = 437
   };
#endif
#define T_ATTRIBUTE 258
#define T_CONST 259
#define T_BOOL 260
#define T_FLOAT 261
#define T_INT 262
#define T_DOUBLE 263
#define T_BREAK 264
#define T_CONTINUE 265
#define T_DO 266
#define T_ELSE 267
#define T_FOR 268
#define T_IF 269
#define T_DISCARD 270
#define T_RETURN 271
#define T_BASIC_TYPE 272
#define T_BVEC2 273
#define T_BVEC3 274
#define T_BVEC4 275
#define T_IVEC2 276
#define T_IVEC3 277
#define T_IVEC4 278
#define T_VEC2 279
#define T_VEC3 280
#define T_VEC4 281
#define T_DVEC2 282
#define T_DVEC3 283
#define T_DVEC4 284
#define T_MAT2 285
#define T_MAT3 286
#define T_MAT4 287
#define T_IN 288
#define T_OUT 289
#define T_INOUT 290
#define T_UNIFORM 291
#define T_VARYING 292
#define T_PATCH 293
#define T_SAMPLE 294
#define T_UINT 295
#define T_UVEC2 296
#define T_UVEC3 297
#define T_UVEC4 298
#define T_MAT2X3 299
#define T_MAT2X4 300
#define T_MAT3X2 301
#define T_MAT3X4 302
#define T_MAT4X2 303
#define T_MAT4X3 304
#define T_DMAT2 305
#define T_DMAT3 306
#define T_DMAT4 307
#define T_DMAT2X3 308
#define T_DMAT2X4 309
#define T_DMAT3X2 310
#define T_DMAT3X4 311
#define T_DMAT4X2 312
#define T_DMAT4X3 313
#define T_SAMPLER2D 314
#define T_SAMPLERCUBE 315
#define T_SAMPLERCUBEARRAY 316
#define T_SAMPLERCUBESHADOW 317
#define T_SAMPLERCUBEARRAYSHADOW 318
#define T_SAMPLER2DSHADOW 319
#define T_SAMPLER3D 320
#define T_SAMPLER1DARRAY 321
#define T_SAMPLER2DARRAY 322
#define T_SAMPLER1DARRAYSHADOW 323
#define T_SAMPLER2DARRAYSHADOW 324
#define T_ISAMPLER2D 325
#define T_ISAMPLERCUBE 326
#define T_ISAMPLERCUBEARRAY 327
#define T_ISAMPLER3D 328
#define T_ISAMPLER2DARRAY 329
#define T_USAMPLER2D 330
#define T_USAMPLERCUBE 331
#define T_USAMPLERCUBEARRAY 332
#define T_USAMPLER3D 333
#define T_USAMPLER2DARRAY 334
#define T_SAMPLEREXTERNALOES 335
#define T_SAMPLER2DMS 336
#define T_ISAMPLER2DMS 337
#define T_USAMPLER2DMS 338
#define T_SAMPLER2DMSARRAY 339
#define T_ISAMPLER2DMSARRAY 340
#define T_USAMPLER2DMSARRAY 341
#define T_SAMPLERBUFFER 342
#define T_ISAMPLERBUFFER 343
#define T_USAMPLERBUFFER 344
#define T_SAMPLER1D 345
#define T_ISAMPLER1D 346
#define T_USAMPLER1D 347
#define T_SAMPLER1DSHADOW 348
#define T_SAMPLER2DRECT 349
#define T_ISAMPLER2DRECT 350
#define T_USAMPLER2DRECT 351
#define T_SAMPLER2DRECTSHADOW 352
#define T_ISAMPLER1DARRAY 353
#define T_USAMPLER1DARRAY 354
#define T_IMAGE2D 355
#define T_IIMAGE2D 356
#define T_UIMAGE2D 357
#define T_IMAGE2DARRAY 358
#define T_IIMAGE2DARRAY 359
#define T_UIMAGE2DARRAY 360
#define T_IMAGE3D 361
#define T_IIMAGE3D 362
#define T_UIMAGE3D 363
#define T_IMAGECUBE 364
#define T_IIMAGECUBE 365
#define T_UIMAGECUBE 366
#define T_IMAGECUBEARRAY 367
#define T_IIMAGECUBEARRAY 368
#define T_UIMAGECUBEARRAY 369
#define T_IMAGEBUFFER 370
#define T_IIMAGEBUFFER 371
#define T_UIMAGEBUFFER 372
#define T_GEN_SAMPLER 373
#define T_GEN_ISAMPLER 374
#define T_GEN_USAMPLER 375
#define T_ATOMIC_UINT 376
#define T_BUFFER 377
#define T_SHARED 378
#define T_STRUCT 379
#define T_VOID 380
#define T_WHILE 381
#define T_IO_BLOCK 382
#define T_ARRAY4_OF_IVEC2 383
#define T_TYPE_MATCH_CALLBACK0 384
#define T_TYPE_MATCH_CALLBACK1 385
#define T_TYPE_MATCH_CALLBACK2 386
#define T_SWITCH 387
#define T_CASE 388
#define T_DEFAULT 389
#define T_CENTROID 390
#define T_FLAT 391
#define T_SMOOTH 392
#define T_NOPERSPECTIVE 393
#define T_LAYOUT 394
#define T_UNIFORM_BLOCK 395
#define T_IDENTIFIER 396
#define T_TYPE_NAME 397
#define T_FLOATCONSTANT 398
#define T_INTCONSTANT 399
#define T_BOOLCONSTANT 400
#define T_UINTCONSTANT 401
#define T_FIELD_SELECTION 402
#define T_LEFT_OP 403
#define T_RIGHT_OP 404
#define T_INC_OP 405
#define T_DEC_OP 406
#define T_LE_OP 407
#define T_GE_OP 408
#define T_EQ_OP 409
#define T_NE_OP 410
#define T_AND_OP 411
#define T_OR_OP 412
#define T_XOR_OP 413
#define T_MUL_ASSIGN 414
#define T_DIV_ASSIGN 415
#define T_ADD_ASSIGN 416
#define T_MOD_ASSIGN 417
#define T_LEFT_ASSIGN 418
#define T_RIGHT_ASSIGN 419
#define T_AND_ASSIGN 420
#define T_XOR_ASSIGN 421
#define T_OR_ASSIGN 422
#define T_SUB_ASSIGN 423
#define T_LENGTH_METHOD 424
#define T_INVARIANT 425
#define T_HIGH_PRECISION 426
#define T_MEDIUM_PRECISION 427
#define T_LOW_PRECISION 428
#define T_PRECISION 429
#define T_PRECISE 430
#define T_COHERENT 431
#define T_VOLATILE 432
#define T_RESTRICT 433
#define T_READONLY 434
#define T_WRITEONLY 435
#define T_VIV_ASM 436
#define T_ASM_OPND_BRACKET 437




/* Copy the first part of user declarations.  */
#line 1 "gc_glsl.y"

#include "gc_glsl_parser.h"

#define YY_NO_UNISTD_H

#define YYPARSE_PARAM_DECL    sloCOMPILER
#define YYPARSE_PARAM        Compiler

#define YYLEX_PARAM            Compiler

#define YY_DECL                int yylex(YYSTYPE * pyylval, sloCOMPILER Compiler)

#define YYFPRINTF           yyfprintf

static int yyfprintf(FILE *file, const char * msg, ...)
{
    /* Do nothing */
    return 0;
}


/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 1
#endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

#if !defined (YYSTYPE) && ! defined (YYSTYPE_IS_DECLARED)
#line 25 "gc_glsl.y"
typedef union YYSTYPE {
    slsLexToken                    token;

    slsDeclOrDeclList            declOrDeclList;

    slsDLINK_LIST *                fieldDeclList;

    slsFieldDecl *                fieldDecl;

    slsDATA_TYPE *                dataType;

    sloIR_EXPR                expr;

    slsNAME    *                funcName;

    slsNAME    *                paramName;

    slsNAME *                blockName;

    sloIR_SET                    statements;

    sloIR_BASE                    statement;

    slsSelectionStatementPair    selectionStatementPair;

    slsForExprPair                forExprPair;

    sloIR_POLYNARY_EXPR            funcCall;

    slsASM_OPCODE               asmOpcode;

    sloIR_VIV_ASM               vivAsm;

    slsASM_MODIFIER             asmModifier;

    slsASM_MODIFIERS            asmModifiers;
} YYSTYPE;
/* Line 191 of yacc.c.  */
#line 499 "gc_glsl_parser.c"
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif



/* Copy the second part of user declarations.  */


/* Line 214 of yacc.c.  */
#line 511 "gc_glsl_parser.c"

#if !defined (yyoverflow) || YYERROR_VERBOSE

#ifndef YYFREE
#  define YYFREE slFree
# endif
#ifndef YYMALLOC
#  define YYMALLOC slMalloc
# endif

/* The parser invokes alloca or malloc; define the necessary symbols.  */

#ifdef YYSTACK_USE_ALLOCA
#if YYSTACK_USE_ALLOCA
#   define YYSTACK_ALLOC slMalloc
#  endif
# else
#if defined (alloca) || defined (_ALLOCA_H)
#   define YYSTACK_ALLOC slMalloc
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
#define YYFINAL  189
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   5990

/* YYNTOKENS -- Number of terminals. */
#define YYNTOKENS  207
/* YYNNTS -- Number of nonterminals. */
#define YYNNTS  114
/* YYNRULES -- Number of rules. */
#define YYNRULES  391
/* YYNRULES -- Number of states. */
#define YYNSTATES  557

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   437

#define YYTRANSLATE(YYX)                         \
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const unsigned char yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   192,     2,     2,     2,   198,   203,     2,
     181,   182,   196,   195,   188,   193,   187,   197,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,   189,   191,
     199,   190,   200,   204,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,   183,     2,   184,   202,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   185,   201,   186,   194,     2,     2,     2,
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
     175,   176,   177,   178,   179,   180,   205,   206
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const unsigned short yyprhs[] =
{
       0,     0,     3,     5,     7,     9,    11,    13,    15,    19,
      21,    26,    28,    32,    36,    39,    42,    44,    48,    50,
      54,    56,    61,    63,    67,    71,    75,    82,    84,    87,
      90,    93,    95,    98,   102,   105,   107,   109,   111,   114,
     117,   120,   122,   124,   126,   128,   130,   134,   138,   142,
     144,   148,   152,   154,   158,   162,   164,   168,   172,   176,
     180,   182,   186,   190,   192,   196,   198,   202,   204,   208,
     210,   214,   216,   220,   222,   226,   228,   234,   236,   240,
     242,   244,   246,   248,   250,   252,   254,   256,   258,   260,
     262,   264,   268,   270,   275,   281,   287,   294,   298,   303,
     306,   309,   314,   316,   319,   322,   324,   326,   329,   333,
     337,   340,   346,   350,   352,   354,   357,   360,   362,   364,
     366,   368,   370,   372,   374,   377,   379,   381,   385,   392,
     398,   403,   409,   417,   426,   433,   435,   438,   444,   449,
     453,   460,   468,   473,   476,   482,   484,   487,   489,   492,
     494,   496,   498,   500,   502,   504,   506,   508,   510,   512,
     514,   515,   521,   526,   528,   532,   534,   538,   542,   544,
     546,   548,   550,   552,   554,   557,   559,   562,   564,   566,
     568,   570,   572,   574,   578,   583,   586,   588,   590,   592,
     594,   596,   598,   600,   602,   604,   606,   608,   610,   612,
     614,   616,   618,   620,   622,   624,   626,   628,   630,   632,
     634,   636,   638,   640,   642,   644,   646,   648,   650,   652,
     654,   656,   658,   660,   662,   664,   666,   668,   670,   672,
     674,   676,   678,   680,   682,   684,   686,   688,   690,   692,
     694,   696,   698,   700,   702,   704,   706,   708,   710,   712,
     714,   716,   718,   720,   722,   724,   726,   728,   730,   732,
     734,   736,   738,   740,   742,   744,   746,   748,   750,   752,
     754,   756,   758,   760,   762,   764,   766,   768,   770,   772,
     774,   776,   778,   780,   782,   784,   786,   788,   790,   792,
     794,   796,   798,   800,   802,   804,   806,   808,   810,   812,
     813,   820,   821,   827,   828,   835,   837,   840,   844,   849,
     851,   855,   857,   862,   865,   868,   872,   879,   885,   886,
     893,   894,   901,   903,   906,   908,   912,   916,   918,   923,
     927,   930,   932,   934,   936,   938,   940,   942,   944,   946,
     948,   950,   953,   954,   959,   961,   963,   966,   967,   972,
     974,   977,   979,   982,   983,   990,   996,   998,  1001,  1003,
    1007,  1010,  1013,  1014,  1019,  1020,  1025,  1027,  1029,  1034,
    1035,  1042,  1050,  1051,  1059,  1061,  1063,  1065,  1066,  1069,
    1073,  1076,  1079,  1082,  1086,  1089,  1091,  1094,  1096,  1098,
    1099,  1100
};

/* YYRHS -- A `-1'-separated list of the rules' RHS. */
static const short yyrhs[] =
{
     316,     0,    -1,   141,    -1,   208,    -1,   144,    -1,   146,
      -1,   143,    -1,   145,    -1,   181,   240,   182,    -1,   209,
      -1,   210,   183,   211,   184,    -1,   218,    -1,   210,   187,
     147,    -1,   210,   187,   169,    -1,   210,   150,    -1,   210,
     151,    -1,   240,    -1,   213,   182,   191,    -1,   217,    -1,
     213,   188,   214,    -1,   238,    -1,   238,   206,   215,   200,
      -1,   216,    -1,   215,   188,   216,    -1,   141,   189,   141,
      -1,   205,   181,   141,    -1,   205,   181,   141,   206,   215,
     200,    -1,   219,    -1,   221,   182,    -1,   220,   182,    -1,
     222,   125,    -1,   222,    -1,   222,   238,    -1,   221,   188,
     238,    -1,   223,   181,    -1,   256,    -1,   141,    -1,   210,
      -1,   150,   224,    -1,   151,   224,    -1,   225,   224,    -1,
     195,    -1,   193,    -1,   192,    -1,   194,    -1,   224,    -1,
     226,   196,   224,    -1,   226,   197,   224,    -1,   226,   198,
     224,    -1,   226,    -1,   227,   195,   226,    -1,   227,   193,
     226,    -1,   227,    -1,   228,   148,   227,    -1,   228,   149,
     227,    -1,   228,    -1,   229,   199,   228,    -1,   229,   200,
     228,    -1,   229,   152,   228,    -1,   229,   153,   228,    -1,
     229,    -1,   230,   154,   229,    -1,   230,   155,   229,    -1,
     230,    -1,   231,   203,   230,    -1,   231,    -1,   232,   202,
     231,    -1,   232,    -1,   233,   201,   232,    -1,   233,    -1,
     234,   156,   233,    -1,   234,    -1,   235,   158,   234,    -1,
     235,    -1,   236,   157,   235,    -1,   236,    -1,   236,   204,
     240,   189,   238,    -1,   237,    -1,   224,   239,   238,    -1,
     190,    -1,   159,    -1,   160,    -1,   162,    -1,   161,    -1,
     168,    -1,   163,    -1,   164,    -1,   165,    -1,   166,    -1,
     167,    -1,   238,    -1,   240,   188,   238,    -1,   237,    -1,
     183,   184,   183,   184,    -1,   183,   184,   183,   241,   184,
      -1,   183,   241,   184,   183,   184,    -1,   183,   241,   184,
     183,   241,   184,    -1,   242,   183,   184,    -1,   242,   183,
     241,   184,    -1,   244,   191,    -1,   254,   191,    -1,   174,
     271,   270,   191,    -1,   281,    -1,   257,   191,    -1,   245,
     182,    -1,   247,    -1,   246,    -1,   247,   249,    -1,   246,
     188,   249,    -1,   256,   141,   181,    -1,   267,   141,    -1,
     267,   141,   183,   241,   184,    -1,   267,   141,   242,    -1,
     248,    -1,   253,    -1,   252,   248,    -1,   252,   253,    -1,
      33,    -1,    34,    -1,    35,    -1,   250,    -1,   264,    -1,
     271,    -1,   251,    -1,   252,   251,    -1,   267,    -1,   255,
      -1,   254,   188,   141,    -1,   254,   188,   141,   183,   241,
     184,    -1,   254,   188,   141,   183,   184,    -1,   254,   188,
     141,   242,    -1,   254,   188,   141,   190,   289,    -1,   254,
     188,   141,   183,   184,   190,   289,    -1,   254,   188,   141,
     183,   241,   184,   190,   289,    -1,   254,   188,   141,   242,
     190,   289,    -1,   256,    -1,   256,   141,    -1,   256,   141,
     183,   241,   184,    -1,   256,   141,   183,   184,    -1,   256,
     141,   242,    -1,   256,   141,   183,   184,   190,   289,    -1,
     256,   141,   183,   241,   184,   190,   289,    -1,   256,   141,
     190,   289,    -1,   257,   141,    -1,   256,   141,   242,   190,
     289,    -1,   267,    -1,   257,   267,    -1,   258,    -1,   257,
     258,    -1,   266,    -1,   272,    -1,   260,    -1,   170,    -1,
     175,    -1,   271,    -1,   259,    -1,   265,    -1,   137,    -1,
     136,    -1,   138,    -1,    -1,   139,   181,   262,   261,   182,
      -1,   139,   181,     1,   182,    -1,   263,    -1,   262,   188,
     263,    -1,   141,    -1,   141,   190,   144,    -1,   141,   190,
     146,    -1,     4,    -1,   175,    -1,   135,    -1,    39,    -1,
       4,    -1,    33,    -1,    38,    33,    -1,    34,    -1,    38,
      34,    -1,     3,    -1,    37,    -1,    36,    -1,   122,    -1,
     123,    -1,   269,    -1,   269,   183,   184,    -1,   269,   183,
     241,   184,    -1,   269,   242,    -1,    59,    -1,    60,    -1,
     273,    -1,   142,    -1,    65,    -1,    66,    -1,    67,    -1,
      68,    -1,    64,    -1,    69,    -1,    62,    -1,    63,    -1,
      70,    -1,    71,    -1,    73,    -1,    74,    -1,    75,    -1,
      76,    -1,    78,    -1,    79,    -1,    80,    -1,    81,    -1,
      82,    -1,    83,    -1,    84,    -1,    85,    -1,    86,    -1,
      61,    -1,    72,    -1,    77,    -1,    90,    -1,    91,    -1,
      92,    -1,    93,    -1,    94,    -1,    95,    -1,    96,    -1,
      97,    -1,    98,    -1,    99,    -1,   100,    -1,   101,    -1,
     102,    -1,   103,    -1,   104,    -1,   105,    -1,   106,    -1,
     107,    -1,   108,    -1,   109,    -1,   112,    -1,   110,    -1,
     113,    -1,   111,    -1,   114,    -1,   121,    -1,    87,    -1,
      88,    -1,    89,    -1,   115,    -1,   116,    -1,   117,    -1,
     125,    -1,     6,    -1,     7,    -1,    40,    -1,     5,    -1,
      24,    -1,    25,    -1,    26,    -1,    18,    -1,    19,    -1,
      20,    -1,    21,    -1,    22,    -1,    23,    -1,    41,    -1,
      42,    -1,    43,    -1,    30,    -1,    44,    -1,    45,    -1,
      31,    -1,    46,    -1,    47,    -1,    32,    -1,    48,    -1,
      49,    -1,     8,    -1,    27,    -1,    28,    -1,    29,    -1,
      50,    -1,    53,    -1,    54,    -1,    51,    -1,    55,    -1,
      56,    -1,    52,    -1,    57,    -1,    58,    -1,   268,    -1,
       6,    -1,     7,    -1,   268,    -1,   171,    -1,   172,    -1,
     173,    -1,   176,    -1,   177,    -1,   178,    -1,   179,    -1,
     180,    -1,    -1,   124,   141,   185,   274,   277,   186,    -1,
      -1,   124,   185,   275,   277,   186,    -1,    -1,   124,   142,
     185,   276,   277,   186,    -1,   278,    -1,   277,   278,    -1,
     267,   279,   191,    -1,   271,   267,   279,   191,    -1,   280,
      -1,   279,   188,   280,    -1,   141,    -1,   141,   183,   241,
     184,    -1,   141,   242,    -1,   282,   191,    -1,   282,   141,
     191,    -1,   282,   141,   183,   241,   184,   191,    -1,   282,
     141,   183,   184,   191,    -1,    -1,   257,   141,   185,   283,
     285,   186,    -1,    -1,   257,   141,   185,   284,     1,   186,
      -1,   287,    -1,   285,   287,    -1,   288,    -1,   286,   188,
     288,    -1,   256,   286,   191,    -1,   141,    -1,   141,   183,
     241,   184,    -1,   141,   183,   184,    -1,   141,   242,    -1,
     238,    -1,   243,    -1,   293,    -1,   292,    -1,   290,    -1,
     299,    -1,   300,    -1,   309,    -1,   315,    -1,   212,    -1,
     185,   186,    -1,    -1,   185,   294,   298,   186,    -1,   296,
      -1,   292,    -1,   185,   186,    -1,    -1,   185,   297,   298,
     186,    -1,   291,    -1,   298,   291,    -1,   191,    -1,   240,
     191,    -1,    -1,    14,   181,   240,   182,   301,   306,    -1,
     132,   181,   211,   182,   304,    -1,   303,    -1,   302,   303,
      -1,   291,    -1,   133,   241,   189,    -1,   134,   189,    -1,
     185,   186,    -1,    -1,   185,   305,   302,   186,    -1,    -1,
     291,    12,   307,   291,    -1,   291,    -1,   240,    -1,   256,
     141,   190,   289,    -1,    -1,   126,   310,   181,   308,   182,
     295,    -1,    11,   291,   126,   181,   240,   182,   191,    -1,
      -1,    13,   311,   181,   312,   314,   182,   295,    -1,   299,
      -1,   290,    -1,   308,    -1,    -1,   313,   191,    -1,   313,
     191,   240,    -1,    10,   191,    -1,     9,   191,    -1,    16,
     191,    -1,    16,   240,   191,    -1,    15,   191,    -1,   317,
      -1,   316,   317,    -1,   318,    -1,   243,    -1,    -1,    -1,
     244,   319,   296,   320,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const unsigned short yyrline[] =
{
       0,   188,   188,   193,   195,   197,   199,   201,   203,   208,
     210,   212,   214,   216,   218,   220,   225,   230,   235,   237,
     242,   244,   249,   251,   256,   261,   263,   268,   273,   275,
     280,   282,   287,   289,   294,   299,   301,   306,   308,   310,
     312,   319,   321,   323,   325,   332,   334,   336,   338,   343,
     345,   347,   352,   354,   356,   361,   363,   365,   367,   369,
     374,   376,   378,   383,   385,   390,   392,   397,   399,   404,
     406,   411,   413,   418,   420,   425,   427,   432,   434,   439,
     441,   443,   445,   447,   449,   451,   453,   455,   457,   459,
     464,   466,   471,   476,   478,   480,   482,   484,   486,   491,
     493,   495,   497,   499,   504,   509,   511,   516,   518,   523,
     528,   530,   532,   537,   539,   541,   543,   548,   550,   552,
     558,   560,   562,   566,   568,   573,   587,   589,   591,   593,
     595,   597,   599,   601,   603,   608,   610,   612,   614,   616,
     618,   620,   622,   624,   626,   633,   635,   640,   642,   647,
     649,   651,   653,   655,   657,   659,   661,   666,   668,   670,
     676,   675,   683,   691,   693,   698,   700,   702,   707,   709,
     714,   716,   721,   723,   725,   727,   729,   731,   733,   735,
     737,   739,   744,   745,   747,   749,   754,   756,   758,   760,
     762,   764,   766,   768,   770,   772,   774,   776,   778,   780,
     782,   784,   786,   788,   790,   792,   794,   796,   798,   800,
     802,   804,   806,   808,   810,   812,   814,   816,   818,   820,
     822,   824,   826,   828,   830,   832,   834,   836,   838,   840,
     842,   844,   846,   848,   850,   852,   854,   856,   858,   860,
     862,   864,   866,   868,   870,   872,   874,   876,   881,   883,
     885,   887,   889,   891,   893,   895,   897,   899,   901,   903,
     905,   907,   909,   911,   913,   915,   917,   919,   921,   923,
     925,   927,   929,   931,   933,   935,   937,   939,   941,   943,
     945,   947,   949,   951,   953,   955,   957,   959,   964,   966,
     968,   973,   975,   977,   982,   984,   986,   988,   990,   996,
     995,  1000,   999,  1004,  1003,  1010,  1011,  1015,  1017,  1026,
    1028,  1033,  1035,  1037,  1042,  1044,  1046,  1048,  1054,  1053,
    1058,  1057,  1068,  1069,  1073,  1075,  1080,  1084,  1086,  1088,
    1090,  1095,  1100,  1105,  1107,  1114,  1116,  1118,  1120,  1122,
    1124,  1129,  1132,  1131,  1138,  1140,  1145,  1148,  1147,  1154,
    1156,  1161,  1163,  1169,  1168,  1172,  1177,  1179,  1184,  1186,
    1188,  1193,  1196,  1195,  1204,  1202,  1209,  1217,  1219,  1225,
    1224,  1228,  1231,  1230,  1237,  1239,  1244,  1247,  1251,  1253,
    1258,  1260,  1262,  1264,  1266,  1273,  1274,  1278,  1279,  1285,
    1287,  1284
};
#endif

#if YYDEBUG || YYERROR_VERBOSE
/* YYTNME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals. */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "T_ATTRIBUTE", "T_CONST", "T_BOOL",
  "T_FLOAT", "T_INT", "T_DOUBLE", "T_BREAK", "T_CONTINUE", "T_DO",
  "T_ELSE", "T_FOR", "T_IF", "T_DISCARD", "T_RETURN", "T_BASIC_TYPE",
  "T_BVEC2", "T_BVEC3", "T_BVEC4", "T_IVEC2", "T_IVEC3", "T_IVEC4",
  "T_VEC2", "T_VEC3", "T_VEC4", "T_DVEC2", "T_DVEC3", "T_DVEC4", "T_MAT2",
  "T_MAT3", "T_MAT4", "T_IN", "T_OUT", "T_INOUT", "T_UNIFORM", "T_VARYING",
  "T_PATCH", "T_SAMPLE", "T_UINT", "T_UVEC2", "T_UVEC3", "T_UVEC4",
  "T_MAT2X3", "T_MAT2X4", "T_MAT3X2", "T_MAT3X4", "T_MAT4X2", "T_MAT4X3",
  "T_DMAT2", "T_DMAT3", "T_DMAT4", "T_DMAT2X3", "T_DMAT2X4", "T_DMAT3X2",
  "T_DMAT3X4", "T_DMAT4X2", "T_DMAT4X3", "T_SAMPLER2D", "T_SAMPLERCUBE",
  "T_SAMPLERCUBEARRAY", "T_SAMPLERCUBESHADOW", "T_SAMPLERCUBEARRAYSHADOW",
  "T_SAMPLER2DSHADOW", "T_SAMPLER3D", "T_SAMPLER1DARRAY",
  "T_SAMPLER2DARRAY", "T_SAMPLER1DARRAYSHADOW", "T_SAMPLER2DARRAYSHADOW",
  "T_ISAMPLER2D", "T_ISAMPLERCUBE", "T_ISAMPLERCUBEARRAY", "T_ISAMPLER3D",
  "T_ISAMPLER2DARRAY", "T_USAMPLER2D", "T_USAMPLERCUBE",
  "T_USAMPLERCUBEARRAY", "T_USAMPLER3D", "T_USAMPLER2DARRAY",
  "T_SAMPLEREXTERNALOES", "T_SAMPLER2DMS", "T_ISAMPLER2DMS",
  "T_USAMPLER2DMS", "T_SAMPLER2DMSARRAY", "T_ISAMPLER2DMSARRAY",
  "T_USAMPLER2DMSARRAY", "T_SAMPLERBUFFER", "T_ISAMPLERBUFFER",
  "T_USAMPLERBUFFER", "T_SAMPLER1D", "T_ISAMPLER1D", "T_USAMPLER1D",
  "T_SAMPLER1DSHADOW", "T_SAMPLER2DRECT", "T_ISAMPLER2DRECT",
  "T_USAMPLER2DRECT", "T_SAMPLER2DRECTSHADOW", "T_ISAMPLER1DARRAY",
  "T_USAMPLER1DARRAY", "T_IMAGE2D", "T_IIMAGE2D", "T_UIMAGE2D",
  "T_IMAGE2DARRAY", "T_IIMAGE2DARRAY", "T_UIMAGE2DARRAY", "T_IMAGE3D",
  "T_IIMAGE3D", "T_UIMAGE3D", "T_IMAGECUBE", "T_IIMAGECUBE",
  "T_UIMAGECUBE", "T_IMAGECUBEARRAY", "T_IIMAGECUBEARRAY",
  "T_UIMAGECUBEARRAY", "T_IMAGEBUFFER", "T_IIMAGEBUFFER", "T_UIMAGEBUFFER",
  "T_GEN_SAMPLER", "T_GEN_ISAMPLER", "T_GEN_USAMPLER", "T_ATOMIC_UINT",
  "T_BUFFER", "T_SHARED", "T_STRUCT", "T_VOID", "T_WHILE", "T_IO_BLOCK",
  "T_ARRAY4_OF_IVEC2", "T_TYPE_MATCH_CALLBACK0", "T_TYPE_MATCH_CALLBACK1",
  "T_TYPE_MATCH_CALLBACK2", "T_SWITCH", "T_CASE", "T_DEFAULT",
  "T_CENTROID", "T_FLAT", "T_SMOOTH", "T_NOPERSPECTIVE", "T_LAYOUT",
  "T_UNIFORM_BLOCK", "T_IDENTIFIER", "T_TYPE_NAME", "T_FLOATCONSTANT",
  "T_INTCONSTANT", "T_BOOLCONSTANT", "T_UINTCONSTANT", "T_FIELD_SELECTION",
  "T_LEFT_OP", "T_RIGHT_OP", "T_INC_OP", "T_DEC_OP", "T_LE_OP", "T_GE_OP",
  "T_EQ_OP", "T_NE_OP", "T_AND_OP", "T_OR_OP", "T_XOR_OP", "T_MUL_ASSIGN",
  "T_DIV_ASSIGN", "T_ADD_ASSIGN", "T_MOD_ASSIGN", "T_LEFT_ASSIGN",
  "T_RIGHT_ASSIGN", "T_AND_ASSIGN", "T_XOR_ASSIGN", "T_OR_ASSIGN",
  "T_SUB_ASSIGN", "T_LENGTH_METHOD", "T_INVARIANT", "T_HIGH_PRECISION",
  "T_MEDIUM_PRECISION", "T_LOW_PRECISION", "T_PRECISION", "T_PRECISE",
  "T_COHERENT", "T_VOLATILE", "T_RESTRICT", "T_READONLY", "T_WRITEONLY",
  "'('", "')'", "'['", "']'", "'{'", "'}'", "'.'", "','", "':'", "'='",
  "';'", "'!'", "'-'", "'~'", "'+'", "'*'", "'/'", "'%'", "'<'", "'>'",
  "'|'", "'^'", "'&'", "'?'", "T_VIV_ASM", "T_ASM_OPND_BRACKET", "$accept",
  "variable_identifier", "primary_expression", "postfix_expression",
  "integer_expression", "viv_asm_statement", "viv_asm_opnd_list",
  "viv_asm_opnd", "viv_asm_modifier_list", "viv_asm_modifier",
  "viv_asm_opcode", "function_call", "function_call_generic",
  "function_call_header_no_parameters",
  "function_call_header_with_parameters", "function_call_header",
  "function_identifier", "unary_expression", "unary_operator",
  "multiplicative_expression", "additive_expression", "shift_expression",
  "relational_expression", "equality_expression", "and_expression",
  "exclusive_or_expression", "inclusive_or_expression",
  "logical_and_expression", "logical_xor_expression",
  "logical_or_expression", "conditional_expression",
  "assignment_expression", "assignment_operator", "expression",
  "constant_expression", "array_length_list", "declaration",
  "function_prototype", "function_declarator",
  "function_header_with_parameters", "function_header",
  "parameter_declarator", "parameter_declaration",
  "parameter_storage_qualifier", "single_parameter_qualifier",
  "parameter_qualifiers", "parameter_type_specifier",
  "init_declarator_list", "single_declaration", "fully_specified_type",
  "type_qualifiers", "type_qualifier", "interpolation_qualifier",
  "layout_qualifier", "@1", "layout_qualifier_id_list",
  "layout_qualifier_id", "parameter_type_qualifier", "auxiliary_qualifier",
  "storage_qualifier", "type_specifier", "opaque_type_specifier",
  "type_specifier_nonarray", "default_precision_type_specifier",
  "precision_qualifier", "memory_access_qualifier", "struct_specifier",
  "@2", "@3", "@4", "struct_declaration_list", "struct_declaration",
  "struct_declarator_list", "struct_declarator", "interface_block",
  "interface_block_decl", "@5", "@6", "interface_block_member_list",
  "interface_block_member_declarator_list", "interface_block_member",
  "interface_block_member_declarator", "initializer",
  "declaration_statement", "statement", "simple_statement",
  "compound_statement", "@7", "statement_no_new_scope",
  "compound_statement_no_new_scope", "@8", "statement_list",
  "expression_statement", "selection_statement", "@9",
  "switch_body_statement_list", "switch_body_statement", "switch_body",
  "@10", "selection_rest_statement", "@11", "condition",
  "iteration_statement", "@12", "@13", "for_init_statement",
  "conditionopt", "for_rest_statement", "jump_statement",
  "translation_unit", "external_declaration", "function_definition", "@14",
  "@15", 0
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
     435,    40,    41,    91,    93,   123,   125,    46,    44,    58,
      61,    59,    33,    45,   126,    43,    42,    47,    37,    60,
      62,   124,    94,    38,    63,   436,   437
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const unsigned short yyr1[] =
{
       0,   207,   208,   209,   209,   209,   209,   209,   209,   210,
     210,   210,   210,   210,   210,   210,   211,   212,   213,   213,
     214,   214,   215,   215,   216,   217,   217,   218,   219,   219,
     220,   220,   221,   221,   222,   223,   223,   224,   224,   224,
     224,   225,   225,   225,   225,   226,   226,   226,   226,   227,
     227,   227,   228,   228,   228,   229,   229,   229,   229,   229,
     230,   230,   230,   231,   231,   232,   232,   233,   233,   234,
     234,   235,   235,   236,   236,   237,   237,   238,   238,   239,
     239,   239,   239,   239,   239,   239,   239,   239,   239,   239,
     240,   240,   241,   242,   242,   242,   242,   242,   242,   243,
     243,   243,   243,   243,   244,   245,   245,   246,   246,   247,
     248,   248,   248,   249,   249,   249,   249,   250,   250,   250,
     251,   251,   251,   252,   252,   253,   254,   254,   254,   254,
     254,   254,   254,   254,   254,   255,   255,   255,   255,   255,
     255,   255,   255,   255,   255,   256,   256,   257,   257,   258,
     258,   258,   258,   258,   258,   258,   258,   259,   259,   259,
     261,   260,   260,   262,   262,   263,   263,   263,   264,   264,
     265,   265,   266,   266,   266,   266,   266,   266,   266,   266,
     266,   266,   267,   267,   267,   267,   268,   268,   268,   268,
     268,   268,   268,   268,   268,   268,   268,   268,   268,   268,
     268,   268,   268,   268,   268,   268,   268,   268,   268,   268,
     268,   268,   268,   268,   268,   268,   268,   268,   268,   268,
     268,   268,   268,   268,   268,   268,   268,   268,   268,   268,
     268,   268,   268,   268,   268,   268,   268,   268,   268,   268,
     268,   268,   268,   268,   268,   268,   268,   268,   269,   269,
     269,   269,   269,   269,   269,   269,   269,   269,   269,   269,
     269,   269,   269,   269,   269,   269,   269,   269,   269,   269,
     269,   269,   269,   269,   269,   269,   269,   269,   269,   269,
     269,   269,   269,   269,   269,   269,   269,   269,   270,   270,
     270,   271,   271,   271,   272,   272,   272,   272,   272,   274,
     273,   275,   273,   276,   273,   277,   277,   278,   278,   279,
     279,   280,   280,   280,   281,   281,   281,   281,   283,   282,
     284,   282,   285,   285,   286,   286,   287,   288,   288,   288,
     288,   289,   290,   291,   291,   292,   292,   292,   292,   292,
     292,   293,   294,   293,   295,   295,   296,   297,   296,   298,
     298,   299,   299,   301,   300,   300,   302,   302,   303,   303,
     303,   304,   305,   304,   307,   306,   306,   308,   308,   310,
     309,   309,   311,   309,   312,   312,   313,   313,   314,   314,
     315,   315,   315,   315,   315,   316,   316,   317,   317,   319,
     320,   318
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const unsigned char yyr2[] =
{
       0,     2,     1,     1,     1,     1,     1,     1,     3,     1,
       4,     1,     3,     3,     2,     2,     1,     3,     1,     3,
       1,     4,     1,     3,     3,     3,     6,     1,     2,     2,
       2,     1,     2,     3,     2,     1,     1,     1,     2,     2,
       2,     1,     1,     1,     1,     1,     3,     3,     3,     1,
       3,     3,     1,     3,     3,     1,     3,     3,     3,     3,
       1,     3,     3,     1,     3,     1,     3,     1,     3,     1,
       3,     1,     3,     1,     3,     1,     5,     1,     3,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     3,     1,     4,     5,     5,     6,     3,     4,     2,
       2,     4,     1,     2,     2,     1,     1,     2,     3,     3,
       2,     5,     3,     1,     1,     2,     2,     1,     1,     1,
       1,     1,     1,     1,     2,     1,     1,     3,     6,     5,
       4,     5,     7,     8,     6,     1,     2,     5,     4,     3,
       6,     7,     4,     2,     5,     1,     2,     1,     2,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       0,     5,     4,     1,     3,     1,     3,     3,     1,     1,
       1,     1,     1,     1,     2,     1,     2,     1,     1,     1,
       1,     1,     1,     3,     4,     2,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     0,
       6,     0,     5,     0,     6,     1,     2,     3,     4,     1,
       3,     1,     4,     2,     2,     3,     6,     5,     0,     6,
       0,     6,     1,     2,     1,     3,     3,     1,     4,     3,
       2,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     2,     0,     4,     1,     1,     2,     0,     4,     1,
       2,     1,     2,     0,     6,     5,     1,     2,     1,     3,
       2,     2,     0,     4,     0,     4,     1,     1,     4,     0,
       6,     7,     0,     7,     1,     1,     1,     0,     2,     3,
       2,     2,     2,     3,     2,     1,     2,     1,     1,     0,
       0,     4
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const unsigned short yydefact[] =
{
       0,   177,   172,   252,   249,   250,   274,   256,   257,   258,
     259,   260,   261,   253,   254,   255,   275,   276,   277,   265,
     268,   271,   173,   175,   179,   178,     0,   171,   251,   262,
     263,   264,   266,   267,   269,   270,   272,   273,   278,   281,
     284,   279,   280,   282,   283,   285,   286,   186,   187,   213,
     196,   197,   194,   190,   191,   192,   193,   195,   198,   199,
     214,   200,   201,   202,   203,   215,   204,   205,   206,   207,
     208,   209,   210,   211,   212,   242,   243,   244,   216,   217,
     218,   219,   220,   221,   222,   223,   224,   225,   226,   227,
     228,   229,   230,   231,   232,   233,   234,   235,   237,   239,
     236,   238,   240,   245,   246,   247,   241,   180,   181,     0,
     248,   170,   158,   157,   159,     0,   189,   152,   291,   292,
     293,     0,   153,   294,   295,   296,   297,   298,   388,   389,
       0,   106,   105,     0,   126,   135,     0,   147,   155,   151,
     156,   149,   145,   287,   182,   154,   150,   188,   102,     0,
       0,   385,   387,   174,   176,     0,     0,   301,     0,     0,
      99,     0,   104,     0,   168,   117,   118,   119,   169,   113,
     107,   120,   123,     0,   114,   121,   125,   122,     0,   100,
     136,   143,   103,   148,   146,     0,   185,     0,   314,     1,
     386,   299,   303,     0,     0,   165,   160,   163,   288,   289,
     290,     0,   347,   390,   108,   115,   124,   116,   110,   127,
     109,     0,     0,   139,   318,     2,     6,     4,     7,     5,
       0,     0,     0,   183,    43,    42,    44,    41,     3,     9,
      37,    11,    27,     0,     0,    31,     0,    45,     0,    49,
      52,    55,    60,    63,    65,    67,    69,    71,    73,    75,
      92,     0,    35,     0,     0,     0,   315,     0,     0,     0,
       0,     0,   305,   162,     0,     0,     0,   101,   346,     0,
     391,     0,   112,     0,     0,   130,   138,     0,    45,    77,
     331,   142,     0,     0,     0,    38,    39,    90,     0,     0,
      14,    15,     0,     0,    29,    28,     0,   248,    32,    34,
      40,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   184,    97,     0,     0,     0,     0,     0,   311,     0,
     309,     0,   302,   306,   166,   167,   164,   161,     0,     0,
       0,   372,     0,     0,     0,   369,     0,   342,   351,     0,
     340,     0,    18,     0,   332,     0,   135,   335,   349,   334,
     333,     0,   336,   337,   338,   339,     0,     0,   129,     0,
     131,     0,     0,   137,    80,    81,    83,    82,    85,    86,
      87,    88,    89,    84,    79,     0,   144,     0,     0,   322,
       0,     8,     0,    93,     0,     0,    16,    12,    13,    33,
      46,    47,    48,    51,    50,    53,    54,    58,    59,    56,
      57,    61,    62,    64,    66,    68,    70,    72,    74,     0,
       0,    98,   317,     0,   300,   304,     0,   313,     0,   307,
       0,   381,   380,     0,     0,     0,   384,   382,     0,     0,
       0,   341,     0,     0,     0,     0,   352,   348,   350,   111,
       0,   128,   134,   140,     0,    78,   327,     0,   324,   319,
     323,   321,    91,    94,    10,     0,    95,     0,   316,     0,
     310,   308,     0,     0,     0,   383,     0,     0,     0,    25,
      17,    19,    20,   132,     0,   141,     0,   330,     0,   326,
      76,    96,   312,     0,   375,   374,   377,   353,   367,    35,
       0,     0,   343,     0,     0,   133,   329,     0,   325,     0,
     376,     0,     0,     0,     0,     0,   362,   355,     0,     0,
      22,     0,   328,     0,   378,     0,   366,   354,     0,   345,
     370,   344,   361,     0,     0,     0,    26,    21,   371,   379,
     373,   364,   368,     0,     0,   358,     0,   356,    24,    23,
       0,     0,   360,   363,   357,   365,   359
};

/* YYDEFGOTO[NTERM-NUM]. */
static const short yydefgoto[] =
{
      -1,   228,   229,   230,   395,   350,   351,   481,   519,   520,
     352,   231,   232,   233,   234,   235,   236,   278,   238,   239,
     240,   241,   242,   243,   244,   245,   246,   247,   248,   249,
     279,   287,   385,   353,   251,   186,   354,   355,   130,   131,
     132,   169,   170,   171,   172,   173,   174,   133,   134,   252,
     253,   137,   138,   139,   266,   196,   197,   175,   140,   141,
     142,   143,   144,   201,   145,   146,   147,   257,   193,   258,
     261,   262,   329,   330,   148,   149,   283,   284,   388,   457,
     389,   458,   281,   357,   358,   359,   360,   442,   530,   531,
     269,   361,   362,   363,   513,   546,   547,   517,   533,   527,
     550,   500,   364,   439,   434,   496,   511,   512,   365,   150,
     151,   152,   161,   270
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -503
static const short yypact[] =
{
    5105,  -503,  -503,  -503,  -503,  -503,  -503,  -503,  -503,  -503,
    -503,  -503,  -503,  -503,  -503,  -503,  -503,  -503,  -503,  -503,
    -503,  -503,  -503,  -503,  -503,  -503,   152,  -503,  -503,  -503,
    -503,  -503,  -503,  -503,  -503,  -503,  -503,  -503,  -503,  -503,
    -503,  -503,  -503,  -503,  -503,  -503,  -503,  -503,  -503,  -503,
    -503,  -503,  -503,  -503,  -503,  -503,  -503,  -503,  -503,  -503,
    -503,  -503,  -503,  -503,  -503,  -503,  -503,  -503,  -503,  -503,
    -503,  -503,  -503,  -503,  -503,  -503,  -503,  -503,  -503,  -503,
    -503,  -503,  -503,  -503,  -503,  -503,  -503,  -503,  -503,  -503,
    -503,  -503,  -503,  -503,  -503,  -503,  -503,  -503,  -503,  -503,
    -503,  -503,  -503,  -503,  -503,  -503,  -503,  -503,  -503,  -123,
    -503,  -503,  -503,  -503,  -503,  -165,  -503,  -503,  -503,  -503,
    -503,     3,  -503,  -503,  -503,  -503,  -503,  -503,  -503,  -132,
    -150,  -120,  5460,  -161,  -503,   -58,  4061,  -503,  -503,  -503,
    -503,  -503,  -503,  -503,   -96,  -503,  -503,  -503,  -503,  -131,
    4927,  -503,  -503,  -503,  -503,   -87,   -83,  -503,    11,  5848,
    -503,   -74,  -503,  5460,  -503,  -503,  -503,  -503,  -503,  -503,
    -503,  -503,  -503,  5460,  -503,  -503,     4,  -503,     7,  -503,
    -117,   -69,  -503,  -503,  -503,  1886,    -3,  -139,  -503,  -503,
    -503,  -503,  -503,  5598,    13,    19,    10,  -503,  -503,  -503,
    -503,    14,     5,  -503,  -503,  -503,  -503,  -503,    28,  -105,
    -503,  2068,  3703,  -104,   234,    55,  -503,  -503,  -503,  -503,
    3703,  3703,  3703,    56,  -503,  -503,  -503,  -503,  -503,  -503,
    -129,  -503,  -503,    59,  -137,  3882,    57,  -503,  3703,    -8,
    -126,    51,  -103,    47,    39,    42,    44,    93,    94,  -143,
    -503,    67,  -503,  5283,  2250,  2432,  -503,  5598,  5598,   113,
    5736,  4415,  -503,  -503,    50,   114,    74,  -503,  -503,  1321,
    -503,  2614,    -3,  2796,  3703,  -101,   -91,    73,    60,  -503,
    -503,  -503,  3703,  5283,   258,  -503,  -503,  -503,   -75,  2978,
    -503,  -503,  3703,  -121,  -503,  -503,  3703,    78,  -503,  -503,
    -503,  3703,  3703,  3703,  3703,  3703,  3703,  3703,  3703,  3703,
    3703,  3703,  3703,  3703,  3703,  3703,  3703,  3703,  3703,  3703,
    3703,    79,  -503,    80,    72,    81,  4584,  4753,    84,   -76,
    -503,   113,  -503,  -503,  -503,  -503,  -503,  -503,    77,    82,
    1321,  -503,    91,    83,  3160,  -503,    95,    98,  -503,    96,
    -503,   -39,  -503,   -47,  -503,  -132,  -124,  -503,  -503,  -503,
    -503,   935,  -503,  -503,  -503,  -503,    56,    97,   -82,   101,
    -503,  3703,  3703,   -44,  -503,  -503,  -503,  -503,  -503,  -503,
    -503,  -503,  -503,  -503,  -503,  3703,  -503,   138,  4239,  -503,
     102,  -503,  3703,  -503,   103,   105,    92,  -503,  -503,  -503,
    -503,  -503,  -503,    -8,    -8,  -126,  -126,    51,    51,    51,
      51,  -103,  -103,    47,    39,    42,    44,    93,    94,    15,
    3339,  -503,  -503,    99,  -503,  -503,  2614,    -3,   113,  -503,
     -35,  -503,  -503,   165,   111,  3703,  -503,  -503,   -22,   112,
    3703,  -503,  1321,   154,   106,  3703,  -503,  -503,  -503,    79,
    3703,   -43,  -503,  -503,  3703,  -503,   115,    -7,  -503,  -503,
    -503,  -503,  -503,  -503,  -503,  3703,  -503,   116,  -503,   117,
    -503,  -503,   118,  1707,   -27,  -503,  3703,   120,  1128,    90,
    -503,  -503,   104,  -503,  3703,  -503,  3521,    -3,   138,  -503,
    -503,  -503,    79,  3703,  -503,  -503,  3703,  -503,    92,   163,
     125,   123,  -503,   168,   168,  -503,    56,   128,  -503,   -25,
    -503,   122,   133,  1321,   126,  1514,   131,  -503,   129,  -164,
    -503,  -159,    79,   130,  3703,  1514,   307,  -503,  3703,  -503,
    -503,  -503,  -503,   742,   179,   168,  -503,  -503,  -503,    92,
    -503,  -503,  -503,  3703,   134,  -503,   549,  -503,  -503,  -503,
    1321,   135,  -503,  -503,  -503,  -503,  -503
};

/* YYPGOTO[NTERM-NUM].  */
static const short yypgoto[] =
{
    -503,  -503,  -503,  -503,  -118,  -503,  -503,  -503,  -178,  -210,
    -503,  -503,  -503,  -503,  -503,  -503,  -503,  -183,  -503,   -97,
     -77,  -138,   -81,    16,    12,    17,    20,    18,    24,  -503,
    -180,  -179,  -503,  -182,  -208,  -174,     8,     9,  -503,  -503,
    -503,   155,   166,  -503,   158,  -503,   159,  -503,  -503,     0,
       1,  -116,  -503,  -503,  -503,  -503,    69,  -503,  -503,  -503,
     -93,   180,  -503,  -503,   -79,  -503,  -503,  -503,  -503,  -503,
     -42,  -250,    21,   -90,  -503,  -503,  -503,  -503,  -503,  -503,
     -41,  -144,  -267,  -128,  -336,  -502,  -503,  -503,  -177,   185,
    -503,   -92,  -122,  -503,  -503,  -503,  -193,  -503,  -503,  -503,
    -503,  -142,  -503,  -503,  -503,  -503,  -503,  -503,  -503,  -503,
     205,  -503,  -503,  -503
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -321
static const short yytable[] =
{
     135,   136,   237,   277,   433,   250,   213,   370,   128,   129,
     187,   333,   194,   529,   319,   386,   158,   180,   155,   156,
     183,   290,   291,   529,   535,   448,   397,   178,   237,   535,
     179,   250,   162,   280,   272,   275,   536,   285,   286,   176,
     288,   537,   159,   184,   255,   295,   323,   325,   398,   308,
     309,   296,   256,   177,   292,   300,   298,   -35,   293,   160,
     188,   320,   157,   367,   210,   369,   211,   304,   163,   305,
     176,   237,   237,   212,   250,   250,   333,   333,   273,   254,
     176,   394,   254,   180,   177,   274,   282,   185,   237,   371,
     237,   250,   289,   250,   177,   280,   310,   311,   191,   372,
     259,   289,   192,   280,   452,   453,   237,   391,   450,   250,
     396,   202,   428,   392,   260,   429,   214,   399,   400,   401,
     402,   237,   237,   237,   237,   237,   237,   237,   237,   237,
     237,   237,   237,   237,   237,   237,   237,   183,   419,   420,
     420,   392,   448,   444,   446,   208,   454,   484,   209,   445,
     135,   136,   195,   428,   427,   497,   471,   523,   128,   129,
     184,   392,   438,   392,   259,   259,   392,   331,   259,   475,
     407,   408,   409,   410,   118,   119,   120,   526,   260,   260,
     254,   488,   260,   483,   489,   153,   154,   485,   301,   302,
     303,   268,   280,   280,   334,   263,   335,   545,   265,   306,
     307,   312,   313,   392,   465,   267,   455,   403,   404,   264,
     545,   271,   467,   462,   555,   326,   327,   505,   469,   374,
     375,   376,   377,   378,   379,   380,   381,   382,   383,   405,
     406,   411,   412,   259,   259,  -320,   -36,   237,   299,   289,
     250,   294,   314,   237,   315,   316,   250,   260,   260,   317,
     384,   321,   318,   474,   328,   195,   337,   373,   396,   390,
     -30,   542,   420,   422,   421,   423,   482,   426,   431,   356,
     136,   280,   435,   432,   436,   280,   440,   443,   507,   456,
     392,   449,   487,   387,   441,   451,   490,   463,   461,   464,
     468,   472,   473,   476,   498,   479,   503,   480,   486,   493,
     491,   492,   501,   237,   514,   280,   250,   515,   516,   518,
     504,   509,   522,   524,   498,   525,   528,   532,   534,   541,
     548,   538,   477,   552,   556,   549,   521,   414,   205,   204,
     413,   206,   207,   415,   336,   551,   417,   416,   470,   200,
     356,   136,   539,   418,   508,   494,   203,   460,   540,   280,
     478,   495,   430,   554,   510,   190,     0,     0,     0,     0,
     237,   356,   136,   250,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   387,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   356,   136,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   356,   136,     0,   499,     0,   356,   136,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   499,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   356,   136,   356,   136,     0,     0,     0,
       0,     0,     0,     0,     0,   356,   136,     0,     0,     0,
       0,     0,     0,   356,   136,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   356,   136,     0,     0,
     356,   136,     1,     2,     3,     4,     5,     6,   338,   339,
     340,     0,   341,   342,   343,   344,     0,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,     0,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    73,    74,    75,    76,    77,    78,
      79,    80,    81,    82,    83,    84,    85,    86,    87,    88,
      89,    90,    91,    92,    93,    94,    95,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,     0,     0,     0,
     106,   107,   108,   109,   110,   345,     0,     0,     0,     0,
       0,   346,   543,   544,   111,   112,   113,   114,   115,     0,
     215,   116,   216,   217,   218,   219,     0,     0,     0,   220,
     221,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   117,
     118,   119,   120,   121,   122,   123,   124,   125,   126,   127,
     222,     0,     0,     0,   347,   553,     0,     0,     0,     0,
     348,   224,   225,   226,   227,     1,     2,     3,     4,     5,
       6,   338,   339,   340,   349,   341,   342,   343,   344,     0,
       7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,     0,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    70,    71,    72,    73,    74,    75,
      76,    77,    78,    79,    80,    81,    82,    83,    84,    85,
      86,    87,    88,    89,    90,    91,    92,    93,    94,    95,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
       0,     0,     0,   106,   107,   108,   109,   110,   345,     0,
       0,     0,     0,     0,   346,   543,   544,   111,   112,   113,
     114,   115,     0,   215,   116,   216,   217,   218,   219,     0,
       0,     0,   220,   221,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   117,   118,   119,   120,   121,   122,   123,   124,
     125,   126,   127,   222,     0,     0,     0,   347,     0,     0,
       0,     0,     0,   348,   224,   225,   226,   227,     1,     2,
       3,     4,     5,     6,   338,   339,   340,   349,   341,   342,
     343,   344,     0,     7,     8,     9,    10,    11,    12,    13,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
       0,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    71,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    85,    86,    87,    88,    89,    90,    91,    92,
      93,    94,    95,    96,    97,    98,    99,   100,   101,   102,
     103,   104,   105,     0,     0,     0,   106,   107,   108,   109,
     110,   345,     0,     0,     0,     0,     0,   346,     0,     0,
     111,   112,   113,   114,   115,     0,   215,   116,   216,   217,
     218,   219,     0,     0,     0,   220,   221,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   117,   118,   119,   120,   121,
     122,   123,   124,   125,   126,   127,   222,     0,     0,     0,
     347,   447,     0,     0,     0,     0,   348,   224,   225,   226,
     227,     1,     2,     3,     4,     5,     6,   338,   339,   340,
     349,   341,   342,   343,   344,     0,     7,     8,     9,    10,
      11,    12,    13,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,     0,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    72,    73,    74,    75,    76,    77,    78,    79,
      80,    81,    82,    83,    84,    85,    86,    87,    88,    89,
      90,    91,    92,    93,    94,    95,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,     0,     0,     0,   106,
     107,   108,   109,   110,   345,     0,     0,     0,     0,     0,
     346,     0,     0,   111,   112,   113,   114,   115,     0,   215,
     116,   216,   217,   218,   219,     0,     0,     0,   220,   221,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   117,   118,
     119,   120,   121,   122,   123,   124,   125,   126,   127,   222,
       0,     0,     0,   347,   502,     0,     0,     0,     0,   348,
     224,   225,   226,   227,     1,     2,     3,     4,     5,     6,
     338,   339,   340,   349,   341,   342,   343,   344,     0,     7,
       8,     9,    10,    11,    12,    13,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,     0,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    71,    72,    73,    74,    75,    76,
      77,    78,    79,    80,    81,    82,    83,    84,    85,    86,
      87,    88,    89,    90,    91,    92,    93,    94,    95,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,     0,
       0,     0,   106,   107,   108,   109,   110,   345,     0,     0,
       0,     0,     0,   346,     0,     0,   111,   112,   113,   114,
     115,     0,   215,   116,   216,   217,   218,   219,     0,     0,
       0,   220,   221,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   117,   118,   119,   120,   121,   122,   123,   124,   125,
     126,   127,   222,     0,     0,     0,   347,     0,     0,     0,
       0,     0,   348,   224,   225,   226,   227,     1,     2,     3,
       4,     5,     6,   338,   339,   340,   349,   341,   342,   343,
     344,     0,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,     0,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    83,
      84,    85,    86,    87,    88,    89,    90,    91,    92,    93,
      94,    95,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,     0,     0,     0,   106,   107,   108,   109,   110,
     345,     0,     0,     0,     0,     0,   346,     0,     0,   111,
     112,   113,   114,   115,     0,   215,   116,   216,   217,   218,
     219,     0,     0,     0,   220,   221,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   117,   118,   119,   120,   121,   122,
     123,   124,   125,   126,   127,   222,     0,     0,     0,   202,
       0,     0,     0,     0,     0,   348,   224,   225,   226,   227,
       1,     2,     3,     4,     5,     6,     0,     0,     0,   349,
       0,     0,     0,     0,     0,     7,     8,     9,    10,    11,
      12,    13,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,     0,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      81,    82,    83,    84,    85,    86,    87,    88,    89,    90,
      91,    92,    93,    94,    95,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,     0,     0,     0,   106,   107,
     108,   109,   110,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   111,   112,   113,   114,   115,     0,   215,   116,
     216,   217,   218,   219,     0,     0,     0,   220,   221,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   117,   118,   119,
     120,   121,   122,   123,   124,   125,   126,   127,   222,     1,
       2,     3,     4,     5,     6,     0,     0,     0,   348,   224,
     225,   226,   227,     0,     7,     8,     9,    10,    11,    12,
      13,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,     0,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    94,    95,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,     0,     0,     0,   106,   107,   108,
     109,   110,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   111,   112,   113,   114,   115,     0,   215,   116,   216,
     217,   218,   219,     0,     0,     0,   220,   221,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   117,   118,   119,   120,
       0,   122,   123,   124,   125,   126,   127,   222,     0,     0,
     223,     1,     2,     3,     4,     5,     6,     0,   224,   225,
     226,   227,     0,     0,     0,     0,     7,     8,     9,    10,
      11,    12,    13,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,     0,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    72,    73,    74,    75,    76,    77,    78,    79,
      80,    81,    82,    83,    84,    85,    86,    87,    88,    89,
      90,    91,    92,    93,    94,    95,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,     0,     0,     0,   106,
     107,   108,   109,   110,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   111,   112,   113,   114,   115,     0,   215,
     116,   216,   217,   218,   219,     0,     0,     0,   220,   221,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   117,   118,
     119,   120,     0,   122,   123,   124,   125,   126,   127,   222,
       0,     0,   276,     1,     2,     3,     4,     5,     6,     0,
     224,   225,   226,   227,     0,     0,     0,     0,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,     0,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    71,    72,    73,    74,    75,    76,    77,
      78,    79,    80,    81,    82,    83,    84,    85,    86,    87,
      88,    89,    90,    91,    92,    93,    94,    95,    96,    97,
      98,    99,   100,   101,   102,   103,   104,   105,     0,     0,
       0,   106,   107,   108,   109,   110,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   111,   112,   113,   114,   115,
       0,   215,   116,   216,   217,   218,   219,     0,     0,     0,
     220,   221,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     117,   118,   119,   120,     0,   122,   123,   124,   125,   126,
     127,   222,     0,     0,   322,     1,     2,     3,     4,     5,
       6,     0,   224,   225,   226,   227,     0,     0,     0,     0,
       7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,     0,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    70,    71,    72,    73,    74,    75,
      76,    77,    78,    79,    80,    81,    82,    83,    84,    85,
      86,    87,    88,    89,    90,    91,    92,    93,    94,    95,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
       0,     0,     0,   106,   107,   108,   109,   110,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   111,   112,   113,
     114,   115,     0,   215,   116,   216,   217,   218,   219,     0,
       0,     0,   220,   221,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   117,   118,   119,   120,     0,   122,   123,   124,
     125,   126,   127,   222,     0,     0,   324,     1,     2,     3,
       4,     5,     6,     0,   224,   225,   226,   227,     0,     0,
       0,     0,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,     0,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    83,
      84,    85,    86,    87,    88,    89,    90,    91,    92,    93,
      94,    95,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,     0,     0,     0,   106,   107,   108,   109,   110,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   111,
     112,   113,   114,   115,     0,   215,   116,   216,   217,   218,
     219,     0,     0,     0,   220,   221,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   117,   118,   119,   120,     0,   122,
     123,   124,   125,   126,   127,   222,     0,     0,   366,     1,
       2,     3,     4,     5,     6,     0,   224,   225,   226,   227,
       0,     0,     0,     0,     7,     8,     9,    10,    11,    12,
      13,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,     0,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    94,    95,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,     0,     0,     0,   106,   107,   108,
     109,   110,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   111,   112,   113,   114,   115,     0,   215,   116,   216,
     217,   218,   219,     0,     0,     0,   220,   221,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   117,   118,   119,   120,
       0,   122,   123,   124,   125,   126,   127,   222,     0,     0,
     368,     1,     2,     3,     4,     5,     6,     0,   224,   225,
     226,   227,     0,     0,     0,     0,     7,     8,     9,    10,
      11,    12,    13,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,     0,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    72,    73,    74,    75,    76,    77,    78,    79,
      80,    81,    82,    83,    84,    85,    86,    87,    88,    89,
      90,    91,    92,    93,    94,    95,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,     0,     0,     0,   106,
     107,   108,   109,   110,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   111,   112,   113,   114,   115,     0,   215,
     116,   216,   217,   218,   219,     0,     0,     0,   220,   221,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   117,   118,
     119,   120,     0,   122,   123,   124,   125,   126,   127,   222,
       0,     0,   393,     1,     2,     3,     4,     5,     6,     0,
     224,   225,   226,   227,     0,     0,     0,     0,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,     0,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    71,    72,    73,    74,    75,    76,    77,
      78,    79,    80,    81,    82,    83,    84,    85,    86,    87,
      88,    89,    90,    91,    92,    93,    94,    95,    96,    97,
      98,    99,   100,   101,   102,   103,   104,   105,     0,     0,
       0,   106,   107,   108,   109,   110,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   111,   112,   113,   114,   115,
       0,   215,   116,   216,   217,   218,   219,     0,     0,     0,
     220,   221,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     117,   118,   119,   120,     0,   122,   123,   124,   125,   126,
     127,   222,     1,     2,     3,     4,     5,     6,     0,     0,
       0,   437,   224,   225,   226,   227,     0,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,     0,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    73,    74,    75,    76,    77,    78,
      79,    80,    81,    82,    83,    84,    85,    86,    87,    88,
      89,    90,    91,    92,    93,    94,    95,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,     0,     0,     0,
     106,   107,   108,   109,   110,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   111,   112,   113,   114,   115,     0,
     215,   116,   216,   217,   218,   219,     0,     0,     0,   220,
     221,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   117,
     118,   119,   120,     0,   122,   123,   124,   125,   126,   127,
     222,     0,     0,   466,     1,     2,     3,     4,     5,     6,
       0,   224,   225,   226,   227,     0,     0,     0,     0,     7,
       8,     9,    10,    11,    12,    13,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,     0,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    71,    72,    73,    74,    75,    76,
      77,    78,    79,    80,    81,    82,    83,    84,    85,    86,
      87,    88,    89,    90,    91,    92,    93,    94,    95,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,     0,
       0,     0,   106,   107,   108,   109,   110,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   111,   112,   113,   114,
     115,     0,   215,   116,   216,   217,   218,   219,     0,     0,
       0,   220,   221,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   117,   118,   119,   120,     0,   122,   123,   124,   125,
     126,   127,   222,     0,     0,   506,     1,     2,     3,     4,
       5,     6,     0,   224,   225,   226,   227,     0,     0,     0,
       0,     7,     8,     9,    10,    11,    12,    13,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,     0,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,     0,     0,     0,   106,   107,   108,   109,   110,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   111,   112,
     113,   114,   115,     0,   215,   116,   216,   217,   218,   219,
       0,     0,     0,   220,   221,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   117,   118,   119,   120,     0,   122,   123,
     124,   125,   126,   127,   222,     1,     2,     3,     4,     5,
       6,     0,     0,     0,     0,   224,   225,   226,   227,     0,
       7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,     0,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    70,    71,    72,    73,    74,    75,
      76,    77,    78,    79,    80,    81,    82,    83,    84,    85,
      86,    87,    88,    89,    90,    91,    92,    93,    94,    95,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
       0,     0,     0,   106,   107,   108,   109,   297,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   111,   112,   113,
     114,   115,     0,   215,   116,   216,   217,   218,   219,     0,
       0,     0,   220,   221,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   117,   118,   119,   120,     0,   122,   123,   124,
     125,   126,   127,   222,     1,     2,     3,     4,     5,     6,
       0,     0,     0,     0,   224,   225,   226,   227,     0,     7,
       8,     9,    10,    11,    12,    13,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,     0,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    71,    72,    73,    74,    75,    76,
      77,    78,    79,    80,    81,    82,    83,    84,    85,    86,
      87,    88,    89,    90,    91,    92,    93,    94,    95,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,     0,
       0,     0,   106,   107,   108,   109,   110,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   111,   112,   113,   114,
     115,     0,   181,   116,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   117,   118,   119,   120,     0,   122,   123,   124,   125,
     126,   127,     1,     2,     3,     4,     5,     6,     0,     0,
       0,     0,   182,     0,     0,     0,     0,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,     0,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    73,    74,    75,    76,    77,    78,
      79,    80,    81,    82,    83,    84,    85,    86,    87,    88,
      89,    90,    91,    92,    93,    94,    95,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,     0,     0,     0,
     106,   107,   108,   109,   110,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   111,   112,   113,   114,   115,     0,
       0,   116,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   117,
     118,   119,   120,     0,   122,   123,   124,   125,   126,   127,
       3,     4,     5,     6,     0,   459,     0,     0,     0,     0,
       0,     0,     0,     7,     8,     9,    10,    11,    12,    13,
      14,    15,    16,    17,    18,    19,    20,    21,     0,     0,
       0,     0,     0,     0,     0,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    71,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    85,    86,    87,    88,    89,    90,    91,    92,
      93,    94,    95,    96,    97,    98,    99,   100,   101,   102,
     103,   104,   105,     0,     0,     0,   106,     0,     0,   109,
     110,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   116,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   118,   119,   120,     3,
       4,     5,     6,     0,     0,     0,     0,     0,     0,     0,
       0,   332,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,     0,     0,     0,
       0,     0,     0,     0,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    83,
      84,    85,    86,    87,    88,    89,    90,    91,    92,    93,
      94,    95,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,     0,     0,     0,   106,     0,     0,   109,   110,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   116,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   118,   119,   120,     3,     4,
       5,     6,     0,     0,     0,     0,     0,     0,     0,     0,
     424,     7,     8,     9,    10,    11,    12,    13,    14,    15,
      16,    17,    18,    19,    20,    21,     0,     0,     0,     0,
       0,     0,     0,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,     0,     0,     0,   106,     0,     0,   109,   110,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   116,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   118,   119,   120,   189,     0,     0,
       1,     2,     3,     4,     5,     6,     0,     0,     0,   425,
       0,     0,     0,     0,     0,     7,     8,     9,    10,    11,
      12,    13,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,     0,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      81,    82,    83,    84,    85,    86,    87,    88,    89,    90,
      91,    92,    93,    94,    95,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,     0,     0,     0,   106,   107,
     108,   109,   110,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   111,   112,   113,   114,   115,     0,     0,   116,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   117,   118,   119,
     120,   121,   122,   123,   124,   125,   126,   127,     1,     2,
       3,     4,     5,     6,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     7,     8,     9,    10,    11,    12,    13,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
       0,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    71,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    85,    86,    87,    88,    89,    90,    91,    92,
      93,    94,    95,    96,    97,    98,    99,   100,   101,   102,
     103,   104,   105,     0,     0,     0,   106,   107,   108,   109,
     110,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     111,   112,   113,   114,   115,     0,     0,   116,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   117,   118,   119,   120,   121,
     122,   123,   124,   125,   126,   127,     1,     2,     3,     4,
       5,     6,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     7,     8,     9,    10,    11,    12,    13,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,     0,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,     0,     0,     0,   106,   107,   108,   109,   110,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   111,   112,
     113,   114,   115,     0,     0,   116,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   117,   118,   119,   120,     0,   122,   123,
     124,   125,   126,   127,   164,     3,     4,     5,     6,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    17,    18,
      19,    20,    21,   165,   166,   167,     0,     0,     0,     0,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    71,    72,    73,    74,    75,    76,    77,
      78,    79,    80,    81,    82,    83,    84,    85,    86,    87,
      88,    89,    90,    91,    92,    93,    94,    95,    96,    97,
      98,    99,   100,   101,   102,   103,   104,   105,     0,     0,
       0,   106,     0,     0,   109,   110,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   116,     3,     4,     5,     6,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     7,     8,     9,    10,
      11,    12,    13,    14,    15,    16,    17,    18,    19,    20,
      21,   118,   119,   120,     0,   168,     0,     0,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    72,    73,    74,    75,    76,    77,    78,    79,
      80,    81,    82,    83,    84,    85,    86,    87,    88,    89,
      90,    91,    92,    93,    94,    95,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,     0,     0,     0,   106,
       0,     0,   109,   110,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     116,     3,     4,     5,     6,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     7,     8,     9,    10,    11,    12,
      13,    14,    15,    16,    17,    18,    19,    20,    21,   118,
     119,   120,     0,     0,     0,     0,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    94,    95,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   198,   199,     0,   106,     0,     0,
     109,   110,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   116,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    72,    73,    74,    75,    76,    77,    78,    79,
      80,    81,    82,    83,    84,    85,    86,    87,    88,    89,
      90,    91,    92,    93,    94,    95,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,     0,     0,     0,   106,
       0,     0,   109,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     116
};

static const short yycheck[] =
{
       0,     0,   185,   211,   340,   185,   180,   274,     0,     0,
     141,   261,     1,   515,   157,   282,   181,   141,   141,   142,
     136,   150,   151,   525,   188,   361,   147,   188,   211,   188,
     191,   211,   182,   212,   208,   209,   200,   220,   221,   132,
     222,   200,   121,   136,   183,   182,   254,   255,   169,   152,
     153,   188,   191,   132,   183,   238,   235,   181,   187,   191,
     191,   204,   185,   271,   181,   273,   183,   193,   188,   195,
     163,   254,   255,   190,   254,   255,   326,   327,   183,   183,
     173,   289,   183,   141,   163,   190,   190,   183,   271,   190,
     273,   271,   183,   273,   173,   274,   199,   200,   185,   190,
     193,   183,   185,   282,   371,   372,   289,   182,   190,   289,
     292,   185,   188,   188,   193,   191,   185,   296,   301,   302,
     303,   304,   305,   306,   307,   308,   309,   310,   311,   312,
     313,   314,   315,   316,   317,   318,   319,   253,   320,   183,
     183,   188,   478,   182,   191,   141,   190,   190,   141,   188,
     150,   150,   141,   188,   328,   182,   191,   182,   150,   150,
     253,   188,   344,   188,   257,   258,   188,   260,   261,   191,
     308,   309,   310,   311,   171,   172,   173,   513,   257,   258,
     183,   188,   261,   450,   191,    33,    34,   454,   196,   197,
     198,   186,   371,   372,   144,   182,   146,   533,   188,   148,
     149,   154,   155,   188,   189,   191,   385,   304,   305,   190,
     546,   183,   420,   392,   550,   257,   258,   484,   426,   159,
     160,   161,   162,   163,   164,   165,   166,   167,   168,   306,
     307,   312,   313,   326,   327,     1,   181,   420,   181,   183,
     420,   182,   203,   426,   202,   201,   426,   326,   327,   156,
     190,   184,   158,   435,   141,   141,   182,   184,   440,     1,
     182,   528,   183,   191,   184,   184,   445,   183,   191,   269,
     269,   450,   181,   191,   191,   454,   181,   181,   486,   141,
     188,   184,   456,   283,   186,   184,   465,   184,   186,   184,
     191,   126,   181,   181,   476,   141,   206,   191,   183,   181,
     184,   184,   182,   486,   141,   484,   486,   182,   185,   141,
     206,   493,   184,   191,   496,   182,   190,   186,   189,    12,
     141,   191,   440,   189,   189,   535,   504,   315,   173,   163,
     314,   173,   173,   316,   265,   543,   318,   317,   428,   159,
     340,   340,   524,   319,   488,   473,   161,   388,   525,   528,
     442,   473,   331,   546,   496,   150,    -1,    -1,    -1,    -1,
     543,   361,   361,   543,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   388,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   442,   442,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   473,   473,    -1,   476,    -1,   478,   478,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   496,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   513,   513,   515,   515,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   525,   525,    -1,    -1,    -1,
      -1,    -1,    -1,   533,   533,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   546,   546,    -1,    -1,
     550,   550,     3,     4,     5,     6,     7,     8,     9,    10,
      11,    -1,    13,    14,    15,    16,    -1,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    -1,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      81,    82,    83,    84,    85,    86,    87,    88,    89,    90,
      91,    92,    93,    94,    95,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,   114,   115,   116,   117,    -1,    -1,    -1,
     121,   122,   123,   124,   125,   126,    -1,    -1,    -1,    -1,
      -1,   132,   133,   134,   135,   136,   137,   138,   139,    -1,
     141,   142,   143,   144,   145,   146,    -1,    -1,    -1,   150,
     151,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   170,
     171,   172,   173,   174,   175,   176,   177,   178,   179,   180,
     181,    -1,    -1,    -1,   185,   186,    -1,    -1,    -1,    -1,
     191,   192,   193,   194,   195,     3,     4,     5,     6,     7,
       8,     9,    10,    11,   205,    13,    14,    15,    16,    -1,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    -1,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    71,    72,    73,    74,    75,    76,    77,
      78,    79,    80,    81,    82,    83,    84,    85,    86,    87,
      88,    89,    90,    91,    92,    93,    94,    95,    96,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,   114,   115,   116,   117,
      -1,    -1,    -1,   121,   122,   123,   124,   125,   126,    -1,
      -1,    -1,    -1,    -1,   132,   133,   134,   135,   136,   137,
     138,   139,    -1,   141,   142,   143,   144,   145,   146,    -1,
      -1,    -1,   150,   151,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   170,   171,   172,   173,   174,   175,   176,   177,
     178,   179,   180,   181,    -1,    -1,    -1,   185,    -1,    -1,
      -1,    -1,    -1,   191,   192,   193,   194,   195,     3,     4,
       5,     6,     7,     8,     9,    10,    11,   205,    13,    14,
      15,    16,    -1,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      -1,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,    -1,    -1,    -1,   121,   122,   123,   124,
     125,   126,    -1,    -1,    -1,    -1,    -1,   132,    -1,    -1,
     135,   136,   137,   138,   139,    -1,   141,   142,   143,   144,
     145,   146,    -1,    -1,    -1,   150,   151,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   170,   171,   172,   173,   174,
     175,   176,   177,   178,   179,   180,   181,    -1,    -1,    -1,
     185,   186,    -1,    -1,    -1,    -1,   191,   192,   193,   194,
     195,     3,     4,     5,     6,     7,     8,     9,    10,    11,
     205,    13,    14,    15,    16,    -1,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    -1,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    94,    95,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   113,   114,   115,   116,   117,    -1,    -1,    -1,   121,
     122,   123,   124,   125,   126,    -1,    -1,    -1,    -1,    -1,
     132,    -1,    -1,   135,   136,   137,   138,   139,    -1,   141,
     142,   143,   144,   145,   146,    -1,    -1,    -1,   150,   151,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   170,   171,
     172,   173,   174,   175,   176,   177,   178,   179,   180,   181,
      -1,    -1,    -1,   185,   186,    -1,    -1,    -1,    -1,   191,
     192,   193,   194,   195,     3,     4,     5,     6,     7,     8,
       9,    10,    11,   205,    13,    14,    15,    16,    -1,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    -1,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    73,    74,    75,    76,    77,    78,
      79,    80,    81,    82,    83,    84,    85,    86,    87,    88,
      89,    90,    91,    92,    93,    94,    95,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   113,   114,   115,   116,   117,    -1,
      -1,    -1,   121,   122,   123,   124,   125,   126,    -1,    -1,
      -1,    -1,    -1,   132,    -1,    -1,   135,   136,   137,   138,
     139,    -1,   141,   142,   143,   144,   145,   146,    -1,    -1,
      -1,   150,   151,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   170,   171,   172,   173,   174,   175,   176,   177,   178,
     179,   180,   181,    -1,    -1,    -1,   185,    -1,    -1,    -1,
      -1,    -1,   191,   192,   193,   194,   195,     3,     4,     5,
       6,     7,     8,     9,    10,    11,   205,    13,    14,    15,
      16,    -1,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    -1,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    70,    71,    72,    73,    74,    75,
      76,    77,    78,    79,    80,    81,    82,    83,    84,    85,
      86,    87,    88,    89,    90,    91,    92,    93,    94,    95,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   113,   114,   115,
     116,   117,    -1,    -1,    -1,   121,   122,   123,   124,   125,
     126,    -1,    -1,    -1,    -1,    -1,   132,    -1,    -1,   135,
     136,   137,   138,   139,    -1,   141,   142,   143,   144,   145,
     146,    -1,    -1,    -1,   150,   151,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   170,   171,   172,   173,   174,   175,
     176,   177,   178,   179,   180,   181,    -1,    -1,    -1,   185,
      -1,    -1,    -1,    -1,    -1,   191,   192,   193,   194,   195,
       3,     4,     5,     6,     7,     8,    -1,    -1,    -1,   205,
      -1,    -1,    -1,    -1,    -1,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    -1,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    71,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    85,    86,    87,    88,    89,    90,    91,    92,
      93,    94,    95,    96,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
     113,   114,   115,   116,   117,    -1,    -1,    -1,   121,   122,
     123,   124,   125,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   135,   136,   137,   138,   139,    -1,   141,   142,
     143,   144,   145,   146,    -1,    -1,    -1,   150,   151,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   170,   171,   172,
     173,   174,   175,   176,   177,   178,   179,   180,   181,     3,
       4,     5,     6,     7,     8,    -1,    -1,    -1,   191,   192,
     193,   194,   195,    -1,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    -1,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    83,
      84,    85,    86,    87,    88,    89,    90,    91,    92,    93,
      94,    95,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     114,   115,   116,   117,    -1,    -1,    -1,   121,   122,   123,
     124,   125,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   135,   136,   137,   138,   139,    -1,   141,   142,   143,
     144,   145,   146,    -1,    -1,    -1,   150,   151,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   170,   171,   172,   173,
      -1,   175,   176,   177,   178,   179,   180,   181,    -1,    -1,
     184,     3,     4,     5,     6,     7,     8,    -1,   192,   193,
     194,   195,    -1,    -1,    -1,    -1,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    -1,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    94,    95,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   113,   114,   115,   116,   117,    -1,    -1,    -1,   121,
     122,   123,   124,   125,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   135,   136,   137,   138,   139,    -1,   141,
     142,   143,   144,   145,   146,    -1,    -1,    -1,   150,   151,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   170,   171,
     172,   173,    -1,   175,   176,   177,   178,   179,   180,   181,
      -1,    -1,   184,     3,     4,     5,     6,     7,     8,    -1,
     192,   193,   194,   195,    -1,    -1,    -1,    -1,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    -1,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    72,    73,    74,    75,    76,    77,    78,    79,
      80,    81,    82,    83,    84,    85,    86,    87,    88,    89,
      90,    91,    92,    93,    94,    95,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   113,   114,   115,   116,   117,    -1,    -1,
      -1,   121,   122,   123,   124,   125,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   135,   136,   137,   138,   139,
      -1,   141,   142,   143,   144,   145,   146,    -1,    -1,    -1,
     150,   151,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     170,   171,   172,   173,    -1,   175,   176,   177,   178,   179,
     180,   181,    -1,    -1,   184,     3,     4,     5,     6,     7,
       8,    -1,   192,   193,   194,   195,    -1,    -1,    -1,    -1,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    -1,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    71,    72,    73,    74,    75,    76,    77,
      78,    79,    80,    81,    82,    83,    84,    85,    86,    87,
      88,    89,    90,    91,    92,    93,    94,    95,    96,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,   114,   115,   116,   117,
      -1,    -1,    -1,   121,   122,   123,   124,   125,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   135,   136,   137,
     138,   139,    -1,   141,   142,   143,   144,   145,   146,    -1,
      -1,    -1,   150,   151,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   170,   171,   172,   173,    -1,   175,   176,   177,
     178,   179,   180,   181,    -1,    -1,   184,     3,     4,     5,
       6,     7,     8,    -1,   192,   193,   194,   195,    -1,    -1,
      -1,    -1,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    -1,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    70,    71,    72,    73,    74,    75,
      76,    77,    78,    79,    80,    81,    82,    83,    84,    85,
      86,    87,    88,    89,    90,    91,    92,    93,    94,    95,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   113,   114,   115,
     116,   117,    -1,    -1,    -1,   121,   122,   123,   124,   125,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   135,
     136,   137,   138,   139,    -1,   141,   142,   143,   144,   145,
     146,    -1,    -1,    -1,   150,   151,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   170,   171,   172,   173,    -1,   175,
     176,   177,   178,   179,   180,   181,    -1,    -1,   184,     3,
       4,     5,     6,     7,     8,    -1,   192,   193,   194,   195,
      -1,    -1,    -1,    -1,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    -1,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    83,
      84,    85,    86,    87,    88,    89,    90,    91,    92,    93,
      94,    95,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     114,   115,   116,   117,    -1,    -1,    -1,   121,   122,   123,
     124,   125,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   135,   136,   137,   138,   139,    -1,   141,   142,   143,
     144,   145,   146,    -1,    -1,    -1,   150,   151,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   170,   171,   172,   173,
      -1,   175,   176,   177,   178,   179,   180,   181,    -1,    -1,
     184,     3,     4,     5,     6,     7,     8,    -1,   192,   193,
     194,   195,    -1,    -1,    -1,    -1,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    -1,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    94,    95,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   113,   114,   115,   116,   117,    -1,    -1,    -1,   121,
     122,   123,   124,   125,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   135,   136,   137,   138,   139,    -1,   141,
     142,   143,   144,   145,   146,    -1,    -1,    -1,   150,   151,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   170,   171,
     172,   173,    -1,   175,   176,   177,   178,   179,   180,   181,
      -1,    -1,   184,     3,     4,     5,     6,     7,     8,    -1,
     192,   193,   194,   195,    -1,    -1,    -1,    -1,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    -1,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    72,    73,    74,    75,    76,    77,    78,    79,
      80,    81,    82,    83,    84,    85,    86,    87,    88,    89,
      90,    91,    92,    93,    94,    95,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   113,   114,   115,   116,   117,    -1,    -1,
      -1,   121,   122,   123,   124,   125,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   135,   136,   137,   138,   139,
      -1,   141,   142,   143,   144,   145,   146,    -1,    -1,    -1,
     150,   151,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     170,   171,   172,   173,    -1,   175,   176,   177,   178,   179,
     180,   181,     3,     4,     5,     6,     7,     8,    -1,    -1,
      -1,   191,   192,   193,   194,   195,    -1,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    -1,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      81,    82,    83,    84,    85,    86,    87,    88,    89,    90,
      91,    92,    93,    94,    95,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,   114,   115,   116,   117,    -1,    -1,    -1,
     121,   122,   123,   124,   125,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   135,   136,   137,   138,   139,    -1,
     141,   142,   143,   144,   145,   146,    -1,    -1,    -1,   150,
     151,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   170,
     171,   172,   173,    -1,   175,   176,   177,   178,   179,   180,
     181,    -1,    -1,   184,     3,     4,     5,     6,     7,     8,
      -1,   192,   193,   194,   195,    -1,    -1,    -1,    -1,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    -1,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    73,    74,    75,    76,    77,    78,
      79,    80,    81,    82,    83,    84,    85,    86,    87,    88,
      89,    90,    91,    92,    93,    94,    95,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   113,   114,   115,   116,   117,    -1,
      -1,    -1,   121,   122,   123,   124,   125,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   135,   136,   137,   138,
     139,    -1,   141,   142,   143,   144,   145,   146,    -1,    -1,
      -1,   150,   151,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   170,   171,   172,   173,    -1,   175,   176,   177,   178,
     179,   180,   181,    -1,    -1,   184,     3,     4,     5,     6,
       7,     8,    -1,   192,   193,   194,   195,    -1,    -1,    -1,
      -1,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    34,    -1,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    71,    72,    73,    74,    75,    76,
      77,    78,    79,    80,    81,    82,    83,    84,    85,    86,
      87,    88,    89,    90,    91,    92,    93,    94,    95,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,   114,   115,   116,
     117,    -1,    -1,    -1,   121,   122,   123,   124,   125,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   135,   136,
     137,   138,   139,    -1,   141,   142,   143,   144,   145,   146,
      -1,    -1,    -1,   150,   151,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   170,   171,   172,   173,    -1,   175,   176,
     177,   178,   179,   180,   181,     3,     4,     5,     6,     7,
       8,    -1,    -1,    -1,    -1,   192,   193,   194,   195,    -1,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    -1,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    71,    72,    73,    74,    75,    76,    77,
      78,    79,    80,    81,    82,    83,    84,    85,    86,    87,
      88,    89,    90,    91,    92,    93,    94,    95,    96,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,   114,   115,   116,   117,
      -1,    -1,    -1,   121,   122,   123,   124,   125,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   135,   136,   137,
     138,   139,    -1,   141,   142,   143,   144,   145,   146,    -1,
      -1,    -1,   150,   151,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   170,   171,   172,   173,    -1,   175,   176,   177,
     178,   179,   180,   181,     3,     4,     5,     6,     7,     8,
      -1,    -1,    -1,    -1,   192,   193,   194,   195,    -1,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    -1,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    73,    74,    75,    76,    77,    78,
      79,    80,    81,    82,    83,    84,    85,    86,    87,    88,
      89,    90,    91,    92,    93,    94,    95,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   113,   114,   115,   116,   117,    -1,
      -1,    -1,   121,   122,   123,   124,   125,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   135,   136,   137,   138,
     139,    -1,   141,   142,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   170,   171,   172,   173,    -1,   175,   176,   177,   178,
     179,   180,     3,     4,     5,     6,     7,     8,    -1,    -1,
      -1,    -1,   191,    -1,    -1,    -1,    -1,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    -1,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      81,    82,    83,    84,    85,    86,    87,    88,    89,    90,
      91,    92,    93,    94,    95,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,   114,   115,   116,   117,    -1,    -1,    -1,
     121,   122,   123,   124,   125,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   135,   136,   137,   138,   139,    -1,
      -1,   142,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   170,
     171,   172,   173,    -1,   175,   176,   177,   178,   179,   180,
       5,     6,     7,     8,    -1,   186,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,    -1,    -1,    -1,   121,    -1,    -1,   124,
     125,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   142,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   171,   172,   173,     5,
       6,     7,     8,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   186,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    70,    71,    72,    73,    74,    75,
      76,    77,    78,    79,    80,    81,    82,    83,    84,    85,
      86,    87,    88,    89,    90,    91,    92,    93,    94,    95,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   113,   114,   115,
     116,   117,    -1,    -1,    -1,   121,    -1,    -1,   124,   125,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   142,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   171,   172,   173,     5,     6,
       7,     8,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     186,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    71,    72,    73,    74,    75,    76,
      77,    78,    79,    80,    81,    82,    83,    84,    85,    86,
      87,    88,    89,    90,    91,    92,    93,    94,    95,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,   114,   115,   116,
     117,    -1,    -1,    -1,   121,    -1,    -1,   124,   125,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   142,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   171,   172,   173,     0,    -1,    -1,
       3,     4,     5,     6,     7,     8,    -1,    -1,    -1,   186,
      -1,    -1,    -1,    -1,    -1,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    -1,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    71,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    85,    86,    87,    88,    89,    90,    91,    92,
      93,    94,    95,    96,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
     113,   114,   115,   116,   117,    -1,    -1,    -1,   121,   122,
     123,   124,   125,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   135,   136,   137,   138,   139,    -1,    -1,   142,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   170,   171,   172,
     173,   174,   175,   176,   177,   178,   179,   180,     3,     4,
       5,     6,     7,     8,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      -1,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,    -1,    -1,    -1,   121,   122,   123,   124,
     125,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     135,   136,   137,   138,   139,    -1,    -1,   142,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   170,   171,   172,   173,   174,
     175,   176,   177,   178,   179,   180,     3,     4,     5,     6,
       7,     8,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    34,    -1,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    71,    72,    73,    74,    75,    76,
      77,    78,    79,    80,    81,    82,    83,    84,    85,    86,
      87,    88,    89,    90,    91,    92,    93,    94,    95,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,   114,   115,   116,
     117,    -1,    -1,    -1,   121,   122,   123,   124,   125,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   135,   136,
     137,   138,   139,    -1,    -1,   142,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   170,   171,   172,   173,    -1,   175,   176,
     177,   178,   179,   180,     4,     5,     6,     7,     8,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    -1,    -1,    -1,    -1,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    72,    73,    74,    75,    76,    77,    78,    79,
      80,    81,    82,    83,    84,    85,    86,    87,    88,    89,
      90,    91,    92,    93,    94,    95,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   113,   114,   115,   116,   117,    -1,    -1,
      -1,   121,    -1,    -1,   124,   125,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   142,     5,     6,     7,     8,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,   171,   172,   173,    -1,   175,    -1,    -1,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    94,    95,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   113,   114,   115,   116,   117,    -1,    -1,    -1,   121,
      -1,    -1,   124,   125,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     142,     5,     6,     7,     8,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,   171,
     172,   173,    -1,    -1,    -1,    -1,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    83,
      84,    85,    86,    87,    88,    89,    90,    91,    92,    93,
      94,    95,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     114,   115,   116,   117,     6,     7,    -1,   121,    -1,    -1,
     124,   125,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   142,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    94,    95,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   113,   114,   115,   116,   117,    -1,    -1,    -1,   121,
      -1,    -1,   124,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     142
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const unsigned short yystos[] =
{
       0,     3,     4,     5,     6,     7,     8,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    94,    95,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   113,   114,   115,   116,   117,   121,   122,   123,   124,
     125,   135,   136,   137,   138,   139,   142,   170,   171,   172,
     173,   174,   175,   176,   177,   178,   179,   180,   243,   244,
     245,   246,   247,   254,   255,   256,   257,   258,   259,   260,
     265,   266,   267,   268,   269,   271,   272,   273,   281,   282,
     316,   317,   318,    33,    34,   141,   142,   185,   181,   271,
     191,   319,   182,   188,     4,    33,    34,    35,   175,   248,
     249,   250,   251,   252,   253,   264,   267,   271,   188,   191,
     141,   141,   191,   258,   267,   183,   242,   141,   191,     0,
     317,   185,   185,   275,     1,   141,   262,   263,     6,     7,
     268,   270,   185,   296,   249,   248,   251,   253,   141,   141,
     181,   183,   190,   242,   185,   141,   143,   144,   145,   146,
     150,   151,   181,   184,   192,   193,   194,   195,   208,   209,
     210,   218,   219,   220,   221,   222,   223,   224,   225,   226,
     227,   228,   229,   230,   231,   232,   233,   234,   235,   236,
     237,   241,   256,   257,   183,   183,   191,   274,   276,   267,
     271,   277,   278,   182,   190,   188,   261,   191,   186,   297,
     320,   183,   242,   183,   190,   242,   184,   241,   224,   237,
     238,   289,   190,   283,   284,   224,   224,   238,   240,   183,
     150,   151,   183,   187,   182,   182,   188,   125,   238,   181,
     224,   196,   197,   198,   193,   195,   148,   149,   152,   153,
     199,   200,   154,   155,   203,   202,   201,   156,   158,   157,
     204,   184,   184,   241,   184,   241,   277,   277,   141,   279,
     280,   267,   186,   278,   144,   146,   263,   182,     9,    10,
      11,    13,    14,    15,    16,   126,   132,   185,   191,   205,
     212,   213,   217,   240,   243,   244,   256,   290,   291,   292,
     293,   298,   299,   300,   309,   315,   184,   241,   184,   241,
     289,   190,   190,   184,   159,   160,   161,   162,   163,   164,
     165,   166,   167,   168,   190,   239,   289,   256,   285,   287,
       1,   182,   188,   184,   241,   211,   240,   147,   169,   238,
     224,   224,   224,   226,   226,   227,   227,   228,   228,   228,
     228,   229,   229,   230,   231,   232,   233,   234,   235,   240,
     183,   184,   191,   184,   186,   186,   183,   242,   188,   191,
     279,   191,   191,   291,   311,   181,   191,   191,   240,   310,
     181,   186,   294,   181,   182,   188,   191,   186,   291,   184,
     190,   184,   289,   289,   190,   238,   141,   286,   288,   186,
     287,   186,   238,   184,   184,   189,   184,   241,   191,   241,
     280,   191,   126,   181,   240,   191,   181,   211,   298,   141,
     191,   214,   238,   289,   190,   289,   183,   242,   188,   191,
     238,   184,   184,   181,   290,   299,   312,   182,   240,   256,
     308,   182,   186,   206,   206,   289,   184,   241,   288,   240,
     308,   313,   314,   301,   141,   182,   185,   304,   141,   215,
     216,   215,   184,   182,   191,   182,   291,   306,   190,   292,
     295,   296,   186,   305,   189,   188,   200,   200,   191,   240,
     295,    12,   289,   133,   134,   291,   302,   303,   141,   216,
     307,   241,   189,   186,   303,   291,   189
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
#line 189 "gc_glsl.y"
    { yyval.expr = slParseVariableIdentifier(Compiler, &yyvsp[0].token); ;}
    break;

  case 3:
#line 194 "gc_glsl.y"
    { yyval.expr = yyvsp[0].expr; ;}
    break;

  case 4:
#line 196 "gc_glsl.y"
    { yyval.expr = slParseIntConstant(Compiler, &yyvsp[0].token); ;}
    break;

  case 5:
#line 198 "gc_glsl.y"
    { yyval.expr = slParseUintConstant(Compiler, &yyvsp[0].token); ;}
    break;

  case 6:
#line 200 "gc_glsl.y"
    { yyval.expr = slParseFloatConstant(Compiler, &yyvsp[0].token); ;}
    break;

  case 7:
#line 202 "gc_glsl.y"
    { yyval.expr = slParseBoolConstant(Compiler, &yyvsp[0].token); ;}
    break;

  case 8:
#line 204 "gc_glsl.y"
    { yyval.expr = yyvsp[-1].expr; ;}
    break;

  case 9:
#line 209 "gc_glsl.y"
    { yyval.expr = yyvsp[0].expr; ;}
    break;

  case 10:
#line 211 "gc_glsl.y"
    { yyval.expr = slParseSubscriptExpr(Compiler, yyvsp[-3].expr, yyvsp[-1].expr); ;}
    break;

  case 11:
#line 213 "gc_glsl.y"
    { yyval.expr = slParseFuncCallExprAsExpr(Compiler, yyvsp[0].funcCall); ;}
    break;

  case 12:
#line 215 "gc_glsl.y"
    { yyval.expr = slParseFieldSelectionExpr(Compiler, yyvsp[-2].expr, &yyvsp[0].token); ;}
    break;

  case 13:
#line 217 "gc_glsl.y"
    { yyval.expr = slParseLengthMethodExpr(Compiler, yyvsp[-2].expr); ;}
    break;

  case 14:
#line 219 "gc_glsl.y"
    { yyval.expr = slParseIncOrDecExpr(Compiler, gcvNULL, slvUNARY_POST_INC, yyvsp[-1].expr); ;}
    break;

  case 15:
#line 221 "gc_glsl.y"
    { yyval.expr = slParseIncOrDecExpr(Compiler, gcvNULL, slvUNARY_POST_DEC, yyvsp[-1].expr); ;}
    break;

  case 16:
#line 226 "gc_glsl.y"
    { yyval.expr = yyvsp[0].expr; ;}
    break;

  case 17:
#line 231 "gc_glsl.y"
    { yyval.statement = slParseAsmAsStatement(Compiler, yyvsp[-2].vivAsm); ;}
    break;

  case 18:
#line 236 "gc_glsl.y"
    { yyval.vivAsm = slParseAsmOpcode(Compiler, &yyvsp[0].asmOpcode); ;}
    break;

  case 19:
#line 238 "gc_glsl.y"
    { yyval.vivAsm = slParseAsmOperand(Compiler, yyvsp[-2].vivAsm, yyvsp[0].expr); ;}
    break;

  case 20:
#line 243 "gc_glsl.y"
    { yyval.expr = yyvsp[0].expr; ;}
    break;

  case 21:
#line 245 "gc_glsl.y"
    { yyval.expr = slParseAsmAppendOperandModifiers(Compiler, yyvsp[-3].expr, &yyvsp[-1].asmModifiers); ;}
    break;

  case 22:
#line 250 "gc_glsl.y"
    { yyval.asmModifiers = slParseAsmAppendModifier(Compiler, gcvNULL, &yyvsp[0].asmModifier); ;}
    break;

  case 23:
#line 252 "gc_glsl.y"
    { yyval.asmModifiers = slParseAsmAppendModifier(Compiler, &yyvsp[-2].asmModifiers, &yyvsp[0].asmModifier); ;}
    break;

  case 24:
#line 257 "gc_glsl.y"
    { yyval.asmModifier = slParseAsmModifier(Compiler, &yyvsp[-2].token, &yyvsp[0].token); ;}
    break;

  case 25:
#line 262 "gc_glsl.y"
    { yyval.asmOpcode = slParseAsmCreateOpcode(Compiler, &yyvsp[0].token, gcvNULL); ;}
    break;

  case 26:
#line 264 "gc_glsl.y"
    { yyval.asmOpcode = slParseAsmCreateOpcode(Compiler, &yyvsp[-3].token, &yyvsp[-1].asmModifiers); ;}
    break;

  case 27:
#line 269 "gc_glsl.y"
    { yyval.funcCall = yyvsp[0].funcCall; ;}
    break;

  case 28:
#line 274 "gc_glsl.y"
    { yyval.funcCall = yyvsp[-1].funcCall; ;}
    break;

  case 29:
#line 276 "gc_glsl.y"
    { yyval.funcCall = yyvsp[-1].funcCall; ;}
    break;

  case 30:
#line 281 "gc_glsl.y"
    { yyval.funcCall = yyvsp[-1].funcCall; ;}
    break;

  case 31:
#line 283 "gc_glsl.y"
    { yyval.funcCall = yyvsp[0].funcCall; ;}
    break;

  case 32:
#line 288 "gc_glsl.y"
    { yyval.funcCall = slParseFuncCallArgument(Compiler, yyvsp[-1].funcCall, yyvsp[0].expr); ;}
    break;

  case 33:
#line 290 "gc_glsl.y"
    { yyval.funcCall = slParseFuncCallArgument(Compiler, yyvsp[-2].funcCall, yyvsp[0].expr); ;}
    break;

  case 34:
#line 295 "gc_glsl.y"
    { yyval.funcCall = slParseFuncCallHeaderExpr(Compiler, &yyvsp[-1].token); ;}
    break;

  case 35:
#line 300 "gc_glsl.y"
    { yyval.token = slParseBasicType(Compiler, yyvsp[0].dataType); ;}
    break;

  case 36:
#line 302 "gc_glsl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 37:
#line 307 "gc_glsl.y"
    { yyval.expr = yyvsp[0].expr; ;}
    break;

  case 38:
#line 309 "gc_glsl.y"
    { yyval.expr = slParseIncOrDecExpr(Compiler, &yyvsp[-1].token, slvUNARY_PRE_INC, yyvsp[0].expr); ;}
    break;

  case 39:
#line 311 "gc_glsl.y"
    { yyval.expr = slParseIncOrDecExpr(Compiler, &yyvsp[-1].token, slvUNARY_PRE_DEC, yyvsp[0].expr); ;}
    break;

  case 40:
#line 313 "gc_glsl.y"
    { yyval.expr = slParseNormalUnaryExpr(Compiler, &yyvsp[-1].token, yyvsp[0].expr); ;}
    break;

  case 41:
#line 320 "gc_glsl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 42:
#line 322 "gc_glsl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 43:
#line 324 "gc_glsl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 44:
#line 326 "gc_glsl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 45:
#line 333 "gc_glsl.y"
    { yyval.expr = yyvsp[0].expr; ;}
    break;

  case 46:
#line 335 "gc_glsl.y"
    { yyval.expr = slParseNormalBinaryExpr(Compiler, yyvsp[-2].expr, &yyvsp[-1].token, yyvsp[0].expr); ;}
    break;

  case 47:
#line 337 "gc_glsl.y"
    { yyval.expr = slParseNormalBinaryExpr(Compiler, yyvsp[-2].expr, &yyvsp[-1].token, yyvsp[0].expr); ;}
    break;

  case 48:
#line 339 "gc_glsl.y"
    { yyval.expr = slParseNormalBinaryExpr(Compiler, yyvsp[-2].expr, &yyvsp[-1].token, yyvsp[0].expr); ;}
    break;

  case 49:
#line 344 "gc_glsl.y"
    { yyval.expr = yyvsp[0].expr; ;}
    break;

  case 50:
#line 346 "gc_glsl.y"
    { yyval.expr = slParseNormalBinaryExpr(Compiler, yyvsp[-2].expr, &yyvsp[-1].token, yyvsp[0].expr); ;}
    break;

  case 51:
#line 348 "gc_glsl.y"
    { yyval.expr = slParseNormalBinaryExpr(Compiler, yyvsp[-2].expr, &yyvsp[-1].token, yyvsp[0].expr); ;}
    break;

  case 52:
#line 353 "gc_glsl.y"
    { yyval.expr = yyvsp[0].expr; ;}
    break;

  case 53:
#line 355 "gc_glsl.y"
    { yyval.expr = slParseNormalBinaryExpr(Compiler, yyvsp[-2].expr, &yyvsp[-1].token, yyvsp[0].expr); ;}
    break;

  case 54:
#line 357 "gc_glsl.y"
    { yyval.expr = slParseNormalBinaryExpr(Compiler, yyvsp[-2].expr, &yyvsp[-1].token, yyvsp[0].expr); ;}
    break;

  case 55:
#line 362 "gc_glsl.y"
    { yyval.expr = yyvsp[0].expr; ;}
    break;

  case 56:
#line 364 "gc_glsl.y"
    { yyval.expr = slParseNormalBinaryExpr(Compiler, yyvsp[-2].expr, &yyvsp[-1].token, yyvsp[0].expr); ;}
    break;

  case 57:
#line 366 "gc_glsl.y"
    { yyval.expr = slParseNormalBinaryExpr(Compiler, yyvsp[-2].expr, &yyvsp[-1].token, yyvsp[0].expr); ;}
    break;

  case 58:
#line 368 "gc_glsl.y"
    { yyval.expr = slParseNormalBinaryExpr(Compiler, yyvsp[-2].expr, &yyvsp[-1].token, yyvsp[0].expr); ;}
    break;

  case 59:
#line 370 "gc_glsl.y"
    { yyval.expr = slParseNormalBinaryExpr(Compiler, yyvsp[-2].expr, &yyvsp[-1].token, yyvsp[0].expr); ;}
    break;

  case 60:
#line 375 "gc_glsl.y"
    { yyval.expr = yyvsp[0].expr; ;}
    break;

  case 61:
#line 377 "gc_glsl.y"
    { yyval.expr = slParseNormalBinaryExpr(Compiler, yyvsp[-2].expr, &yyvsp[-1].token, yyvsp[0].expr); ;}
    break;

  case 62:
#line 379 "gc_glsl.y"
    { yyval.expr = slParseNormalBinaryExpr(Compiler, yyvsp[-2].expr, &yyvsp[-1].token, yyvsp[0].expr); ;}
    break;

  case 63:
#line 384 "gc_glsl.y"
    { yyval.expr = yyvsp[0].expr; ;}
    break;

  case 64:
#line 386 "gc_glsl.y"
    { yyval.expr = slParseNormalBinaryExpr(Compiler, yyvsp[-2].expr, &yyvsp[-1].token, yyvsp[0].expr); ;}
    break;

  case 65:
#line 391 "gc_glsl.y"
    { yyval.expr = yyvsp[0].expr; ;}
    break;

  case 66:
#line 393 "gc_glsl.y"
    { yyval.expr = slParseNormalBinaryExpr(Compiler, yyvsp[-2].expr, &yyvsp[-1].token, yyvsp[0].expr); ;}
    break;

  case 67:
#line 398 "gc_glsl.y"
    { yyval.expr = yyvsp[0].expr; ;}
    break;

  case 68:
#line 400 "gc_glsl.y"
    { yyval.expr = slParseNormalBinaryExpr(Compiler, yyvsp[-2].expr, &yyvsp[-1].token, yyvsp[0].expr); ;}
    break;

  case 69:
#line 405 "gc_glsl.y"
    { yyval.expr = yyvsp[0].expr; ;}
    break;

  case 70:
#line 407 "gc_glsl.y"
    { yyval.expr = slParseNormalBinaryExpr(Compiler, yyvsp[-2].expr, &yyvsp[-1].token, yyvsp[0].expr); ;}
    break;

  case 71:
#line 412 "gc_glsl.y"
    { yyval.expr = yyvsp[0].expr; ;}
    break;

  case 72:
#line 414 "gc_glsl.y"
    { yyval.expr = slParseNormalBinaryExpr(Compiler, yyvsp[-2].expr, &yyvsp[-1].token, yyvsp[0].expr); ;}
    break;

  case 73:
#line 419 "gc_glsl.y"
    { yyval.expr = yyvsp[0].expr; ;}
    break;

  case 74:
#line 421 "gc_glsl.y"
    { yyval.expr = slParseNormalBinaryExpr(Compiler, yyvsp[-2].expr, &yyvsp[-1].token, yyvsp[0].expr); ;}
    break;

  case 75:
#line 426 "gc_glsl.y"
    { yyval.expr = yyvsp[0].expr; ;}
    break;

  case 76:
#line 428 "gc_glsl.y"
    { yyval.expr = slParseSelectionExpr(Compiler, yyvsp[-4].expr, yyvsp[-2].expr, yyvsp[0].expr); ;}
    break;

  case 77:
#line 433 "gc_glsl.y"
    { yyval.expr = yyvsp[0].expr; ;}
    break;

  case 78:
#line 435 "gc_glsl.y"
    { yyval.expr = slParseAssignmentExpr(Compiler, yyvsp[-2].expr, &yyvsp[-1].token, yyvsp[0].expr); ;}
    break;

  case 79:
#line 440 "gc_glsl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 80:
#line 442 "gc_glsl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 81:
#line 444 "gc_glsl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 82:
#line 446 "gc_glsl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 83:
#line 448 "gc_glsl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 84:
#line 450 "gc_glsl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 85:
#line 452 "gc_glsl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 86:
#line 454 "gc_glsl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 87:
#line 456 "gc_glsl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 88:
#line 458 "gc_glsl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 89:
#line 460 "gc_glsl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 90:
#line 465 "gc_glsl.y"
    { yyval.expr = yyvsp[0].expr; ;}
    break;

  case 91:
#line 467 "gc_glsl.y"
    { yyval.expr = slParseNormalBinaryExpr(Compiler, yyvsp[-2].expr, &yyvsp[-1].token, yyvsp[0].expr); ;}
    break;

  case 92:
#line 472 "gc_glsl.y"
    { yyval.expr = yyvsp[0].expr; ;}
    break;

  case 93:
#line 477 "gc_glsl.y"
    { yyval.fieldDeclList = slParseArrayLengthList(Compiler, gcvNULL, gcvNULL, gcvNULL); ;}
    break;

  case 94:
#line 479 "gc_glsl.y"
    { yyval.fieldDeclList = slParseArrayLengthList(Compiler, gcvNULL, gcvNULL, yyvsp[-1].expr); ;}
    break;

  case 95:
#line 481 "gc_glsl.y"
    { yyval.fieldDeclList = slParseArrayLengthList(Compiler, gcvNULL, yyvsp[-3].expr, gcvNULL); ;}
    break;

  case 96:
#line 483 "gc_glsl.y"
    { yyval.fieldDeclList = slParseArrayLengthList(Compiler, gcvNULL, yyvsp[-4].expr, yyvsp[-1].expr); ;}
    break;

  case 97:
#line 485 "gc_glsl.y"
    { yyval.fieldDeclList = slParseArrayLengthList2(Compiler, yyvsp[-2].fieldDeclList, gcvNULL, gcvNULL); ;}
    break;

  case 98:
#line 487 "gc_glsl.y"
    { yyval.fieldDeclList = slParseArrayLengthList2(Compiler, yyvsp[-3].fieldDeclList, yyvsp[-1].expr, gcvNULL); ;}
    break;

  case 99:
#line 492 "gc_glsl.y"
    { yyval.statement = slParseFuncDecl(Compiler, yyvsp[-1].funcName); ;}
    break;

  case 100:
#line 494 "gc_glsl.y"
    { yyval.statement = slParseDeclaration(Compiler, yyvsp[-1].declOrDeclList); ;}
    break;

  case 101:
#line 496 "gc_glsl.y"
    { yyval.statement = slParseDefaultPrecisionQualifier(Compiler, &yyvsp[-3].token, &yyvsp[-2].token, yyvsp[-1].dataType); ;}
    break;

  case 102:
#line 498 "gc_glsl.y"
    { yyval.statement = yyvsp[0].statement; ;}
    break;

  case 103:
#line 500 "gc_glsl.y"
    { yyval.statement = slParseQualifierAsStatement(Compiler, &yyvsp[-1].token); ;}
    break;

  case 104:
#line 505 "gc_glsl.y"
    { yyval.funcName = yyvsp[-1].funcName; ;}
    break;

  case 105:
#line 510 "gc_glsl.y"
    { yyval.funcName = yyvsp[0].funcName; ;}
    break;

  case 106:
#line 512 "gc_glsl.y"
    { yyval.funcName = yyvsp[0].funcName; ;}
    break;

  case 107:
#line 517 "gc_glsl.y"
    { yyval.funcName = slParseParameterList(Compiler, yyvsp[-1].funcName, yyvsp[0].paramName); ;}
    break;

  case 108:
#line 519 "gc_glsl.y"
    { yyval.funcName = slParseParameterList(Compiler, yyvsp[-2].funcName, yyvsp[0].paramName); ;}
    break;

  case 109:
#line 524 "gc_glsl.y"
    { yyval.funcName = slParseFuncHeader(Compiler, yyvsp[-2].dataType, &yyvsp[-1].token); ;}
    break;

  case 110:
#line 529 "gc_glsl.y"
    { yyval.funcName = slParseNonArrayParameterDecl(Compiler, yyvsp[-1].dataType, &yyvsp[0].token); ;}
    break;

  case 111:
#line 531 "gc_glsl.y"
    { yyval.funcName = slParseArrayParameterDecl(Compiler, yyvsp[-4].dataType, &yyvsp[-3].token, yyvsp[-1].expr); ;}
    break;

  case 112:
#line 533 "gc_glsl.y"
    { yyval.funcName = slParseArrayListParameterDecl(Compiler, yyvsp[-2].dataType, &yyvsp[-1].token, yyvsp[0].fieldDeclList); ;}
    break;

  case 113:
#line 538 "gc_glsl.y"
    { yyval.paramName = slParseQualifiedParameterDecl(Compiler, gcvNULL, yyvsp[0].funcName); ;}
    break;

  case 114:
#line 540 "gc_glsl.y"
    { yyval.paramName = slParseQualifiedParameterDecl(Compiler, gcvNULL, yyvsp[0].funcName); ;}
    break;

  case 115:
#line 542 "gc_glsl.y"
    { yyval.paramName = slParseQualifiedParameterDecl(Compiler, &yyvsp[-1].token, yyvsp[0].funcName); ;}
    break;

  case 116:
#line 544 "gc_glsl.y"
    { yyval.paramName = slParseQualifiedParameterDecl(Compiler, &yyvsp[-1].token, yyvsp[0].funcName); ;}
    break;

  case 117:
#line 549 "gc_glsl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 118:
#line 551 "gc_glsl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 119:
#line 553 "gc_glsl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 120:
#line 559 "gc_glsl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 121:
#line 561 "gc_glsl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 122:
#line 563 "gc_glsl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 123:
#line 567 "gc_glsl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 124:
#line 569 "gc_glsl.y"
    { yyval.token = slMergeParameterQualifiers(Compiler, &yyvsp[-1].token, &yyvsp[0].token); ;}
    break;

  case 125:
#line 574 "gc_glsl.y"
    { yyval.funcName = slParseNonArrayParameterDecl(Compiler, yyvsp[0].dataType, gcvNULL); ;}
    break;

  case 126:
#line 588 "gc_glsl.y"
    { yyval.declOrDeclList = yyvsp[0].declOrDeclList; ;}
    break;

  case 127:
#line 590 "gc_glsl.y"
    { yyval.declOrDeclList = slParseNonArrayVariableDecl2(Compiler, yyvsp[-2].declOrDeclList, &yyvsp[0].token); ;}
    break;

  case 128:
#line 592 "gc_glsl.y"
    { yyval.declOrDeclList = slParseArrayVariableDecl2(Compiler, yyvsp[-5].declOrDeclList, &yyvsp[-3].token, yyvsp[-1].expr); ;}
    break;

  case 129:
#line 594 "gc_glsl.y"
    { yyval.declOrDeclList = slParseArrayVariableDecl2(Compiler, yyvsp[-4].declOrDeclList, &yyvsp[-2].token, gcvNULL); ;}
    break;

  case 130:
#line 596 "gc_glsl.y"
    { yyval.declOrDeclList = slParseArrayListVariableDecl2(Compiler, yyvsp[-3].declOrDeclList, &yyvsp[-1].token, yyvsp[0].fieldDeclList); ;}
    break;

  case 131:
#line 598 "gc_glsl.y"
    { yyval.declOrDeclList = slParseVariableDeclWithInitializer2(Compiler, yyvsp[-4].declOrDeclList, &yyvsp[-2].token, yyvsp[0].expr); ;}
    break;

  case 132:
#line 600 "gc_glsl.y"
    { yyval.declOrDeclList = slParseArrayVariableDeclWithInitializer2(Compiler, yyvsp[-6].declOrDeclList, &yyvsp[-4].token, gcvNULL, yyvsp[0].expr); ;}
    break;

  case 133:
#line 602 "gc_glsl.y"
    { yyval.declOrDeclList = slParseArrayVariableDeclWithInitializer2(Compiler, yyvsp[-7].declOrDeclList, &yyvsp[-5].token, yyvsp[-3].expr, yyvsp[0].expr); ;}
    break;

  case 134:
#line 604 "gc_glsl.y"
    { yyval.declOrDeclList = slParseArrayListVariableDeclWithInitializer2(Compiler, yyvsp[-5].declOrDeclList, &yyvsp[-3].token, yyvsp[-2].fieldDeclList, yyvsp[0].expr); ;}
    break;

  case 135:
#line 609 "gc_glsl.y"
    { yyval.declOrDeclList = slParseTypeDecl(Compiler, yyvsp[0].dataType); ;}
    break;

  case 136:
#line 611 "gc_glsl.y"
    { yyval.declOrDeclList = slParseNonArrayVariableDecl(Compiler, yyvsp[-1].dataType, &yyvsp[0].token); ;}
    break;

  case 137:
#line 613 "gc_glsl.y"
    { yyval.declOrDeclList = slParseArrayVariableDecl(Compiler, yyvsp[-4].dataType, &yyvsp[-3].token, yyvsp[-1].expr); ;}
    break;

  case 138:
#line 615 "gc_glsl.y"
    { yyval.declOrDeclList = slParseArrayVariableDecl(Compiler, yyvsp[-3].dataType, &yyvsp[-2].token, gcvNULL); ;}
    break;

  case 139:
#line 617 "gc_glsl.y"
    { yyval.declOrDeclList = slParseArrayListVariableDecl(Compiler, yyvsp[-2].dataType, &yyvsp[-1].token, yyvsp[0].fieldDeclList); ;}
    break;

  case 140:
#line 619 "gc_glsl.y"
    { yyval.declOrDeclList = slParseArrayVariableDeclWithInitializer(Compiler, yyvsp[-5].dataType, &yyvsp[-4].token, gcvNULL, yyvsp[0].expr); ;}
    break;

  case 141:
#line 621 "gc_glsl.y"
    { yyval.declOrDeclList = slParseArrayVariableDeclWithInitializer(Compiler, yyvsp[-6].dataType, &yyvsp[-5].token, yyvsp[-3].expr, yyvsp[0].expr); ;}
    break;

  case 142:
#line 623 "gc_glsl.y"
    { yyval.declOrDeclList = slParseVariableDeclWithInitializer(Compiler, yyvsp[-3].dataType, &yyvsp[-2].token, yyvsp[0].expr); ;}
    break;

  case 143:
#line 625 "gc_glsl.y"
    { yyval.declOrDeclList = slParseInvariantOrPreciseDecl(Compiler, &yyvsp[-1].token, &yyvsp[0].token); ;}
    break;

  case 144:
#line 627 "gc_glsl.y"
    { yyval.declOrDeclList = slParseArrayListVariableDeclWithInitializer(Compiler, yyvsp[-4].dataType, &yyvsp[-3].token, yyvsp[-2].fieldDeclList, yyvsp[0].expr); ;}
    break;

  case 145:
#line 634 "gc_glsl.y"
    { yyval.dataType = slParseFullySpecifiedType(Compiler, gcvNULL, yyvsp[0].dataType); ;}
    break;

  case 146:
#line 636 "gc_glsl.y"
    { yyval.dataType = slParseFullySpecifiedType(Compiler, &yyvsp[-1].token, yyvsp[0].dataType); ;}
    break;

  case 147:
#line 641 "gc_glsl.y"
    { yyval.token = slMergeTypeQualifiers(Compiler, gcvNULL, &yyvsp[0].token); ;}
    break;

  case 148:
#line 643 "gc_glsl.y"
    { yyval.token = slMergeTypeQualifiers(Compiler, &yyvsp[-1].token, &yyvsp[0].token); ;}
    break;

  case 149:
#line 648 "gc_glsl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 150:
#line 650 "gc_glsl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 151:
#line 652 "gc_glsl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 152:
#line 654 "gc_glsl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 153:
#line 656 "gc_glsl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 154:
#line 658 "gc_glsl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 155:
#line 660 "gc_glsl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 156:
#line 662 "gc_glsl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 157:
#line 667 "gc_glsl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 158:
#line 669 "gc_glsl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 159:
#line 671 "gc_glsl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 160:
#line 676 "gc_glsl.y"
    {
          sloCOMPILER_SetScannerState(Compiler, slvSCANNER_NORMAL);
        ;}
    break;

  case 161:
#line 680 "gc_glsl.y"
    {
           yyval.token = slParseLayoutQualifier(Compiler, &yyvsp[-2].token);
        ;}
    break;

  case 162:
#line 684 "gc_glsl.y"
    {
          sloCOMPILER_SetScannerState(Compiler, slvSCANNER_NORMAL);
                  yyval.token = yyvsp[-3].token;
        ;}
    break;

  case 163:
#line 692 "gc_glsl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 164:
#line 694 "gc_glsl.y"
    { yyval.token = slParseAddLayoutId(Compiler, &yyvsp[-2].token, &yyvsp[0].token); ;}
    break;

  case 165:
#line 699 "gc_glsl.y"
    { yyval.token = slParseLayoutId(Compiler, &yyvsp[0].token, gcvNULL); ;}
    break;

  case 166:
#line 701 "gc_glsl.y"
    { yyval.token = slParseLayoutId(Compiler, &yyvsp[-2].token, &yyvsp[0].token) ; ;}
    break;

  case 167:
#line 703 "gc_glsl.y"
    { yyval.token = slParseLayoutId(Compiler, &yyvsp[-2].token, &yyvsp[0].token) ; ;}
    break;

  case 168:
#line 708 "gc_glsl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 169:
#line 710 "gc_glsl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 170:
#line 715 "gc_glsl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 171:
#line 717 "gc_glsl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 172:
#line 722 "gc_glsl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 173:
#line 724 "gc_glsl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 174:
#line 726 "gc_glsl.y"
    { yyval.token = slParseCheckStorage(Compiler, yyvsp[-1].token, yyvsp[0].token); ;}
    break;

  case 175:
#line 728 "gc_glsl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 176:
#line 730 "gc_glsl.y"
    { yyval.token = slParseCheckStorage(Compiler, yyvsp[-1].token, yyvsp[0].token); ;}
    break;

  case 177:
#line 732 "gc_glsl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 178:
#line 734 "gc_glsl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 179:
#line 736 "gc_glsl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 180:
#line 738 "gc_glsl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 181:
#line 740 "gc_glsl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 183:
#line 746 "gc_glsl.y"
    { yyval.dataType = slParseArrayDataType(Compiler, yyvsp[-2].dataType, gcvNULL); ;}
    break;

  case 184:
#line 748 "gc_glsl.y"
    { yyval.dataType = slParseArrayDataType(Compiler, yyvsp[-3].dataType, yyvsp[-1].expr); ;}
    break;

  case 185:
#line 750 "gc_glsl.y"
    { yyval.dataType = slParseArrayListDataType(Compiler, yyvsp[-1].dataType, yyvsp[0].fieldDeclList); ;}
    break;

  case 186:
#line 755 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_SAMPLER2D); ;}
    break;

  case 187:
#line 757 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_SAMPLERCUBE); ;}
    break;

  case 188:
#line 759 "gc_glsl.y"
    { yyval.dataType = slParseStructType(Compiler, yyvsp[0].dataType); ;}
    break;

  case 189:
#line 761 "gc_glsl.y"
    { yyval.dataType = slParseNamedType(Compiler, &yyvsp[0].token); ;}
    break;

  case 190:
#line 763 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_SAMPLER3D); ;}
    break;

  case 191:
#line 765 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_SAMPLER1DARRAY); ;}
    break;

  case 192:
#line 767 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_SAMPLER2DARRAY); ;}
    break;

  case 193:
#line 769 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_SAMPLER1DARRAYSHADOW); ;}
    break;

  case 194:
#line 771 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_SAMPLER2DSHADOW); ;}
    break;

  case 195:
#line 773 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_SAMPLER2DARRAYSHADOW); ;}
    break;

  case 196:
#line 775 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_SAMPLERCUBESHADOW); ;}
    break;

  case 197:
#line 777 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_SAMPLERCUBEARRAYSHADOW); ;}
    break;

  case 198:
#line 779 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_ISAMPLER2D); ;}
    break;

  case 199:
#line 781 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_ISAMPLERCUBE); ;}
    break;

  case 200:
#line 783 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_ISAMPLER3D); ;}
    break;

  case 201:
#line 785 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_ISAMPLER2DARRAY); ;}
    break;

  case 202:
#line 787 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_USAMPLER2D); ;}
    break;

  case 203:
#line 789 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_USAMPLERCUBE); ;}
    break;

  case 204:
#line 791 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_USAMPLER3D); ;}
    break;

  case 205:
#line 793 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_USAMPLER2DARRAY); ;}
    break;

  case 206:
#line 795 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_SAMPLEREXTERNALOES); ;}
    break;

  case 207:
#line 797 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_SAMPLER2DMS); ;}
    break;

  case 208:
#line 799 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_ISAMPLER2DMS); ;}
    break;

  case 209:
#line 801 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_USAMPLER2DMS); ;}
    break;

  case 210:
#line 803 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_SAMPLER2DMSARRAY); ;}
    break;

  case 211:
#line 805 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_ISAMPLER2DMSARRAY); ;}
    break;

  case 212:
#line 807 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_USAMPLER2DMSARRAY); ;}
    break;

  case 213:
#line 809 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_SAMPLERCUBEARRAY); ;}
    break;

  case 214:
#line 811 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_ISAMPLERCUBEARRAY); ;}
    break;

  case 215:
#line 813 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_USAMPLERCUBEARRAY); ;}
    break;

  case 216:
#line 815 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_SAMPLER1D); ;}
    break;

  case 217:
#line 817 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_ISAMPLER1D); ;}
    break;

  case 218:
#line 819 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_USAMPLER1D); ;}
    break;

  case 219:
#line 821 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_SAMPLER1DSHADOW); ;}
    break;

  case 220:
#line 823 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_SAMPLER2DRECT); ;}
    break;

  case 221:
#line 825 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_ISAMPLER2DRECT); ;}
    break;

  case 222:
#line 827 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_USAMPLER2DRECT); ;}
    break;

  case 223:
#line 829 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_SAMPLER2DRECTSHADOW); ;}
    break;

  case 224:
#line 831 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_ISAMPLER1DARRAY); ;}
    break;

  case 225:
#line 833 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_USAMPLER1DARRAY); ;}
    break;

  case 226:
#line 835 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_IMAGE2D); ;}
    break;

  case 227:
#line 837 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_IIMAGE2D); ;}
    break;

  case 228:
#line 839 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_UIMAGE2D); ;}
    break;

  case 229:
#line 841 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_IMAGE2DARRAY); ;}
    break;

  case 230:
#line 843 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_IIMAGE2DARRAY); ;}
    break;

  case 231:
#line 845 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_UIMAGE2DARRAY); ;}
    break;

  case 232:
#line 847 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_IMAGE3D); ;}
    break;

  case 233:
#line 849 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_IIMAGE3D); ;}
    break;

  case 234:
#line 851 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_UIMAGE3D); ;}
    break;

  case 235:
#line 853 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_IMAGECUBE); ;}
    break;

  case 236:
#line 855 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_IMAGECUBEARRAY); ;}
    break;

  case 237:
#line 857 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_IIMAGECUBE); ;}
    break;

  case 238:
#line 859 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_IIMAGECUBEARRAY); ;}
    break;

  case 239:
#line 861 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_UIMAGECUBE); ;}
    break;

  case 240:
#line 863 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_UIMAGECUBEARRAY); ;}
    break;

  case 241:
#line 865 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_ATOMIC_UINT); ;}
    break;

  case 242:
#line 867 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_SAMPLERBUFFER); ;}
    break;

  case 243:
#line 869 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_ISAMPLERBUFFER); ;}
    break;

  case 244:
#line 871 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_USAMPLERBUFFER); ;}
    break;

  case 245:
#line 873 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_IMAGEBUFFER); ;}
    break;

  case 246:
#line 875 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_IIMAGEBUFFER); ;}
    break;

  case 247:
#line 877 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_UIMAGEBUFFER); ;}
    break;

  case 248:
#line 882 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_VOID); ;}
    break;

  case 249:
#line 884 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_FLOAT); ;}
    break;

  case 250:
#line 886 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_INT); ;}
    break;

  case 251:
#line 888 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_UINT); ;}
    break;

  case 252:
#line 890 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_BOOL); ;}
    break;

  case 253:
#line 892 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_VEC2); ;}
    break;

  case 254:
#line 894 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_VEC3); ;}
    break;

  case 255:
#line 896 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_VEC4); ;}
    break;

  case 256:
#line 898 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_BVEC2); ;}
    break;

  case 257:
#line 900 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_BVEC3); ;}
    break;

  case 258:
#line 902 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_BVEC4); ;}
    break;

  case 259:
#line 904 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_IVEC2); ;}
    break;

  case 260:
#line 906 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_IVEC3); ;}
    break;

  case 261:
#line 908 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_IVEC4); ;}
    break;

  case 262:
#line 910 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_UVEC2); ;}
    break;

  case 263:
#line 912 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_UVEC3); ;}
    break;

  case 264:
#line 914 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_UVEC4); ;}
    break;

  case 265:
#line 916 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_MAT2); ;}
    break;

  case 266:
#line 918 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_MAT2X3); ;}
    break;

  case 267:
#line 920 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_MAT2X4); ;}
    break;

  case 268:
#line 922 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_MAT3); ;}
    break;

  case 269:
#line 924 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_MAT3X2); ;}
    break;

  case 270:
#line 926 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_MAT3X4); ;}
    break;

  case 271:
#line 928 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_MAT4); ;}
    break;

  case 272:
#line 930 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_MAT4X2); ;}
    break;

  case 273:
#line 932 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_MAT4X3); ;}
    break;

  case 274:
#line 934 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_DOUBLE); ;}
    break;

  case 275:
#line 936 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_DVEC2); ;}
    break;

  case 276:
#line 938 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_DVEC3); ;}
    break;

  case 277:
#line 940 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_DVEC4); ;}
    break;

  case 278:
#line 942 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_DMAT2); ;}
    break;

  case 279:
#line 944 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_DMAT2X3); ;}
    break;

  case 280:
#line 946 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_DMAT2X4); ;}
    break;

  case 281:
#line 948 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_DMAT3); ;}
    break;

  case 282:
#line 950 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_DMAT3X2); ;}
    break;

  case 283:
#line 952 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_DMAT3X4); ;}
    break;

  case 284:
#line 954 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_DMAT4); ;}
    break;

  case 285:
#line 956 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_DMAT4X2); ;}
    break;

  case 286:
#line 958 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_DMAT4X3); ;}
    break;

  case 287:
#line 960 "gc_glsl.y"
    { yyval.dataType = yyvsp[0].dataType; ;}
    break;

  case 288:
#line 965 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_FLOAT); ;}
    break;

  case 289:
#line 967 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_INT); ;}
    break;

  case 290:
#line 969 "gc_glsl.y"
    { yyval.dataType = yyvsp[0].dataType; ;}
    break;

  case 291:
#line 974 "gc_glsl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 292:
#line 976 "gc_glsl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 293:
#line 978 "gc_glsl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 294:
#line 983 "gc_glsl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 295:
#line 985 "gc_glsl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 296:
#line 987 "gc_glsl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 297:
#line 989 "gc_glsl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 298:
#line 991 "gc_glsl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 299:
#line 996 "gc_glsl.y"
    { slParseStructDeclBegin(Compiler, &yyvsp[-1].token); ;}
    break;

  case 300:
#line 998 "gc_glsl.y"
    { yyval.dataType = slParseStructDeclEnd(Compiler, &yyvsp[-4].token); ;}
    break;

  case 301:
#line 1000 "gc_glsl.y"
    { slParseStructDeclBegin(Compiler, gcvNULL); ;}
    break;

  case 302:
#line 1002 "gc_glsl.y"
    { yyval.dataType = slParseStructDeclEnd(Compiler, gcvNULL); ;}
    break;

  case 303:
#line 1004 "gc_glsl.y"
    { slParseStructReDeclBegin(Compiler, &yyvsp[-1].token); ;}
    break;

  case 304:
#line 1006 "gc_glsl.y"
    { yyval.dataType = slParseStructReDeclEnd(Compiler, &yyvsp[-4].token); ;}
    break;

  case 307:
#line 1016 "gc_glsl.y"
    { slParseTypeSpecifiedFieldDeclList(Compiler, yyvsp[-2].dataType, yyvsp[-1].fieldDeclList); ;}
    break;

  case 308:
#line 1018 "gc_glsl.y"
    {
           yyvsp[-2].dataType->qualifiers.precision = yyvsp[-3].token.u.qualifiers.precision;
           slsQUALIFIERS_SET_FLAG(&(yyvsp[-2].dataType->qualifiers), slvQUALIFIERS_FLAG_PRECISION);
           slParseTypeSpecifiedFieldDeclList(Compiler, yyvsp[-2].dataType, yyvsp[-1].fieldDeclList);
        ;}
    break;

  case 309:
#line 1027 "gc_glsl.y"
    { yyval.fieldDeclList = slParseFieldDeclList(Compiler, yyvsp[0].fieldDecl); ;}
    break;

  case 310:
#line 1029 "gc_glsl.y"
    { yyval.fieldDeclList = slParseFieldDeclList2(Compiler, yyvsp[-2].fieldDeclList, yyvsp[0].fieldDecl); ;}
    break;

  case 311:
#line 1034 "gc_glsl.y"
    { yyval.fieldDecl = slParseFieldDecl(Compiler, &yyvsp[0].token, gcvNULL); ;}
    break;

  case 312:
#line 1036 "gc_glsl.y"
    { yyval.fieldDecl = slParseFieldDecl(Compiler, &yyvsp[-3].token, yyvsp[-1].expr); ;}
    break;

  case 313:
#line 1038 "gc_glsl.y"
    { yyval.fieldDecl = slParseFieldListDecl(Compiler, &yyvsp[-1].token, yyvsp[0].fieldDeclList, gcvFALSE); ;}
    break;

  case 314:
#line 1043 "gc_glsl.y"
    { yyval.statement = slParseInterfaceBlock(Compiler, yyvsp[-1].blockName, gcvNULL, gcvNULL, gcvTRUE); ;}
    break;

  case 315:
#line 1045 "gc_glsl.y"
    { yyval.statement = slParseInterfaceBlock(Compiler, yyvsp[-2].blockName, &yyvsp[-1].token, gcvNULL, gcvTRUE); ;}
    break;

  case 316:
#line 1047 "gc_glsl.y"
    { yyval.statement = slParseInterfaceBlock(Compiler, yyvsp[-5].blockName, &yyvsp[-4].token, yyvsp[-2].expr, gcvTRUE); ;}
    break;

  case 317:
#line 1049 "gc_glsl.y"
    { yyval.statement = slParseInterfaceBlockImplicitArrayLength(Compiler, yyvsp[-4].blockName, &yyvsp[-3].token); ;}
    break;

  case 318:
#line 1054 "gc_glsl.y"
    { slParseInterfaceBlockDeclBegin(Compiler, &yyvsp[-2].token, &yyvsp[-1].token); ;}
    break;

  case 319:
#line 1056 "gc_glsl.y"
    { yyval.blockName = slParseInterfaceBlockDeclEnd(Compiler, &yyvsp[-5].token, &yyvsp[-4].token); ;}
    break;

  case 320:
#line 1058 "gc_glsl.y"
    { slParseInterfaceBlockDeclBegin(Compiler, &yyvsp[-2].token, &yyvsp[-1].token); ;}
    break;

  case 321:
#line 1060 "gc_glsl.y"
    {
            yyclearin;
            yyerrok;
            yyval.blockName = slParseInterfaceBlockDeclEnd(Compiler, gcvNULL, gcvNULL);
        ;}
    break;

  case 324:
#line 1074 "gc_glsl.y"
    { yyval.dataType = slParseInterfaceBlockMember(Compiler, yyvsp[-1].dataType, yyvsp[0].fieldDecl); ;}
    break;

  case 325:
#line 1076 "gc_glsl.y"
    { yyval.dataType = slParseInterfaceBlockMember(Compiler, yyvsp[-2].dataType, yyvsp[0].fieldDecl); ;}
    break;

  case 327:
#line 1085 "gc_glsl.y"
    { yyval.fieldDecl = slParseFieldDecl(Compiler, &yyvsp[0].token, gcvNULL); ;}
    break;

  case 328:
#line 1087 "gc_glsl.y"
    { yyval.fieldDecl = slParseFieldDecl(Compiler, &yyvsp[-3].token, yyvsp[-1].expr); ;}
    break;

  case 329:
#line 1089 "gc_glsl.y"
    { yyval.fieldDecl = slParseImplicitArraySizeFieldDecl(Compiler, &yyvsp[-2].token); ;}
    break;

  case 330:
#line 1091 "gc_glsl.y"
    { yyval.fieldDecl = slParseFieldListDecl(Compiler, &yyvsp[-1].token, yyvsp[0].fieldDeclList, gcvTRUE); ;}
    break;

  case 331:
#line 1096 "gc_glsl.y"
    { yyval.expr = yyvsp[0].expr; ;}
    break;

  case 332:
#line 1101 "gc_glsl.y"
    { yyval.statement = yyvsp[0].statement; ;}
    break;

  case 333:
#line 1106 "gc_glsl.y"
    { yyval.statement = slParseCompoundStatementAsStatement(Compiler, yyvsp[0].statements); ;}
    break;

  case 334:
#line 1108 "gc_glsl.y"
    { yyval.statement = yyvsp[0].statement; ;}
    break;

  case 335:
#line 1115 "gc_glsl.y"
    { yyval.statement = yyvsp[0].statement; ;}
    break;

  case 336:
#line 1117 "gc_glsl.y"
    { yyval.statement = yyvsp[0].statement; ;}
    break;

  case 337:
#line 1119 "gc_glsl.y"
    { yyval.statement = yyvsp[0].statement; ;}
    break;

  case 338:
#line 1121 "gc_glsl.y"
    { yyval.statement = yyvsp[0].statement; ;}
    break;

  case 339:
#line 1123 "gc_glsl.y"
    { yyval.statement = yyvsp[0].statement; ;}
    break;

  case 340:
#line 1125 "gc_glsl.y"
    { yyval.statement = yyvsp[0].statement; ;}
    break;

  case 341:
#line 1130 "gc_glsl.y"
    { yyval.statements = gcvNULL; ;}
    break;

  case 342:
#line 1132 "gc_glsl.y"
    { slParseCompoundStatementBegin(Compiler); ;}
    break;

  case 343:
#line 1134 "gc_glsl.y"
    { yyval.statements = slParseCompoundStatementEnd(Compiler, &yyvsp[-3].token, yyvsp[-1].statements); ;}
    break;

  case 344:
#line 1139 "gc_glsl.y"
    { yyval.statement = slParseCompoundStatementNoNewScopeAsStatementNoNewScope(Compiler, yyvsp[0].statements); ;}
    break;

  case 345:
#line 1141 "gc_glsl.y"
    { yyval.statement = yyvsp[0].statement; ;}
    break;

  case 346:
#line 1146 "gc_glsl.y"
    { yyval.statements = gcvNULL; ;}
    break;

  case 347:
#line 1148 "gc_glsl.y"
    { slParseCompoundStatementNoNewScopeBegin(Compiler); ;}
    break;

  case 348:
#line 1150 "gc_glsl.y"
    { yyval.statements = slParseCompoundStatementNoNewScopeEnd(Compiler, &yyvsp[-3].token, yyvsp[-1].statements); ;}
    break;

  case 349:
#line 1155 "gc_glsl.y"
    { yyval.statements = slParseStatementList(Compiler, yyvsp[0].statement); ;}
    break;

  case 350:
#line 1157 "gc_glsl.y"
    { yyval.statements = slParseStatementList2(Compiler, yyvsp[-1].statements, yyvsp[0].statement); ;}
    break;

  case 351:
#line 1162 "gc_glsl.y"
    { yyval.statement = gcvNULL; ;}
    break;

  case 352:
#line 1164 "gc_glsl.y"
    { yyval.statement = slParseExprAsStatement(Compiler, yyvsp[-1].expr); ;}
    break;

  case 353:
#line 1169 "gc_glsl.y"
    { slParseSelectStatementBegin(Compiler); ;}
    break;

  case 354:
#line 1171 "gc_glsl.y"
    { yyval.statement = slParseSelectionStatement(Compiler, &yyvsp[-5].token, yyvsp[-3].expr, yyvsp[0].selectionStatementPair); ;}
    break;

  case 355:
#line 1173 "gc_glsl.y"
    { yyval.statement = slParseSwitchStatement(Compiler, &yyvsp[-4].token, yyvsp[-2].expr, yyvsp[0].statement); ;}
    break;

  case 356:
#line 1178 "gc_glsl.y"
    { yyval.statements = slParseStatementList(Compiler, yyvsp[0].statement); ;}
    break;

  case 357:
#line 1180 "gc_glsl.y"
    { yyval.statements = slParseStatementList2(Compiler, yyvsp[-1].statements, yyvsp[0].statement); ;}
    break;

  case 358:
#line 1185 "gc_glsl.y"
    { yyval.statement = yyvsp[0].statement; ;}
    break;

  case 359:
#line 1187 "gc_glsl.y"
    { yyval.statement = slParseCaseStatement(Compiler, &yyvsp[-2].token, yyvsp[-1].expr); ;}
    break;

  case 360:
#line 1189 "gc_glsl.y"
    { yyval.statement = slParseDefaultStatement(Compiler, &yyvsp[-1].token); ;}
    break;

  case 361:
#line 1194 "gc_glsl.y"
    { yyval.statement = gcvNULL; ;}
    break;

  case 362:
#line 1196 "gc_glsl.y"
    { slParseSwitchBodyBegin(Compiler); ;}
    break;

  case 363:
#line 1198 "gc_glsl.y"
    { yyval.statement = slParseSwitchBodyEnd(Compiler, &yyvsp[-3].token, yyvsp[-1].statements); ;}
    break;

  case 364:
#line 1204 "gc_glsl.y"
    { slParseSelectStatementEnd(Compiler, gcvNULL, gcvNULL);
          slParseSelectStatementBegin(Compiler); ;}
    break;

  case 365:
#line 1207 "gc_glsl.y"
    { slParseSelectStatementEnd(Compiler, gcvNULL, gcvNULL);
          yyval.selectionStatementPair = slParseSelectionRestStatement(Compiler, yyvsp[-3].statement, yyvsp[0].statement); ;}
    break;

  case 366:
#line 1210 "gc_glsl.y"
    { slParseSelectStatementEnd(Compiler, gcvNULL, gcvNULL);
          yyval.selectionStatementPair = slParseSelectionRestStatement(Compiler, yyvsp[0].statement, gcvNULL); ;}
    break;

  case 367:
#line 1218 "gc_glsl.y"
    { yyval.expr = yyvsp[0].expr; ;}
    break;

  case 368:
#line 1220 "gc_glsl.y"
    { yyval.expr = slParseCondition(Compiler, yyvsp[-3].dataType, &yyvsp[-2].token, yyvsp[0].expr); ;}
    break;

  case 369:
#line 1225 "gc_glsl.y"
    { slParseWhileStatementBegin(Compiler); ;}
    break;

  case 370:
#line 1227 "gc_glsl.y"
    { yyval.statement = slParseWhileStatementEnd(Compiler, &yyvsp[-5].token, yyvsp[-2].expr, yyvsp[0].statement); ;}
    break;

  case 371:
#line 1229 "gc_glsl.y"
    { yyval.statement = slParseDoWhileStatement(Compiler, &yyvsp[-6].token, yyvsp[-5].statement, yyvsp[-2].expr); ;}
    break;

  case 372:
#line 1231 "gc_glsl.y"
    { slParseForStatementBegin(Compiler); ;}
    break;

  case 373:
#line 1233 "gc_glsl.y"
    { yyval.statement = slParseForStatementEnd(Compiler, &yyvsp[-6].token, yyvsp[-3].statement, yyvsp[-2].forExprPair, yyvsp[0].statement); ;}
    break;

  case 374:
#line 1238 "gc_glsl.y"
    { yyval.statement = yyvsp[0].statement; ;}
    break;

  case 375:
#line 1240 "gc_glsl.y"
    { yyval.statement = yyvsp[0].statement; ;}
    break;

  case 376:
#line 1245 "gc_glsl.y"
    { yyval.expr = yyvsp[0].expr; ;}
    break;

  case 377:
#line 1247 "gc_glsl.y"
    { yyval.expr = gcvNULL; ;}
    break;

  case 378:
#line 1252 "gc_glsl.y"
    { yyval.forExprPair = slParseForRestStatement(Compiler, yyvsp[-1].expr, gcvNULL); ;}
    break;

  case 379:
#line 1254 "gc_glsl.y"
    { yyval.forExprPair = slParseForRestStatement(Compiler, yyvsp[-2].expr, yyvsp[0].expr); ;}
    break;

  case 380:
#line 1259 "gc_glsl.y"
    { yyval.statement = slParseJumpStatement(Compiler, slvCONTINUE, &yyvsp[-1].token, gcvNULL); ;}
    break;

  case 381:
#line 1261 "gc_glsl.y"
    { yyval.statement = slParseJumpStatement(Compiler, slvBREAK, &yyvsp[-1].token, gcvNULL); ;}
    break;

  case 382:
#line 1263 "gc_glsl.y"
    { yyval.statement = slParseJumpStatement(Compiler, slvRETURN, &yyvsp[-1].token, gcvNULL); ;}
    break;

  case 383:
#line 1265 "gc_glsl.y"
    { yyval.statement = slParseJumpStatement(Compiler, slvRETURN, &yyvsp[-2].token, yyvsp[-1].expr); ;}
    break;

  case 384:
#line 1267 "gc_glsl.y"
    { yyval.statement = slParseJumpStatement(Compiler, slvDISCARD, &yyvsp[-1].token, gcvNULL); ;}
    break;

  case 388:
#line 1280 "gc_glsl.y"
    { slParseExternalDecl(Compiler, yyvsp[0].statement); ;}
    break;

  case 389:
#line 1285 "gc_glsl.y"
    { slParseFuncDefinitionBegin(Compiler, yyvsp[0].funcName); ;}
    break;

  case 390:
#line 1287 "gc_glsl.y"
    { slParseFuncDefinitionEnd(Compiler, yyvsp[-2].funcName); ;}
    break;

  case 391:
#line 1288 "gc_glsl.y"
    { slParseFuncDef(Compiler, yyvsp[-3].funcName, yyvsp[-1].statements); ;}
    break;


    }

/* Line 1000 of yacc.c.  */
#line 5074 "gc_glsl_parser.c"

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


#line 1291 "gc_glsl.y"



