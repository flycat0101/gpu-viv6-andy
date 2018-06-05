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
     T_BREAK = 263,
     T_CONTINUE = 264,
     T_DO = 265,
     T_ELSE = 266,
     T_FOR = 267,
     T_IF = 268,
     T_DISCARD = 269,
     T_RETURN = 270,
     T_BASIC_TYPE = 271,
     T_BVEC2 = 272,
     T_BVEC3 = 273,
     T_BVEC4 = 274,
     T_IVEC2 = 275,
     T_IVEC3 = 276,
     T_IVEC4 = 277,
     T_VEC2 = 278,
     T_VEC3 = 279,
     T_VEC4 = 280,
     T_MAT2 = 281,
     T_MAT3 = 282,
     T_MAT4 = 283,
     T_IN = 284,
     T_OUT = 285,
     T_INOUT = 286,
     T_UNIFORM = 287,
     T_VARYING = 288,
     T_PATCH = 289,
     T_SAMPLE = 290,
     T_UINT = 291,
     T_UVEC2 = 292,
     T_UVEC3 = 293,
     T_UVEC4 = 294,
     T_MAT2X3 = 295,
     T_MAT2X4 = 296,
     T_MAT3X2 = 297,
     T_MAT3X4 = 298,
     T_MAT4X2 = 299,
     T_MAT4X3 = 300,
     T_SAMPLER2D = 301,
     T_SAMPLERCUBE = 302,
     T_SAMPLERCUBEARRAY = 303,
     T_SAMPLERCUBESHADOW = 304,
     T_SAMPLERCUBEARRAYSHADOW = 305,
     T_SAMPLER2DSHADOW = 306,
     T_SAMPLER3D = 307,
     T_SAMPLER1DARRAY = 308,
     T_SAMPLER2DARRAY = 309,
     T_SAMPLER1DARRAYSHADOW = 310,
     T_SAMPLER2DARRAYSHADOW = 311,
     T_ISAMPLER2D = 312,
     T_ISAMPLERCUBE = 313,
     T_ISAMPLERCUBEARRAY = 314,
     T_ISAMPLER3D = 315,
     T_ISAMPLER2DARRAY = 316,
     T_USAMPLER2D = 317,
     T_USAMPLERCUBE = 318,
     T_USAMPLERCUBEARRAY = 319,
     T_USAMPLER3D = 320,
     T_USAMPLER2DARRAY = 321,
     T_SAMPLEREXTERNALOES = 322,
     T_SAMPLER2DMS = 323,
     T_ISAMPLER2DMS = 324,
     T_USAMPLER2DMS = 325,
     T_SAMPLER2DMSARRAY = 326,
     T_ISAMPLER2DMSARRAY = 327,
     T_USAMPLER2DMSARRAY = 328,
     T_SAMPLERBUFFER = 329,
     T_ISAMPLERBUFFER = 330,
     T_USAMPLERBUFFER = 331,
     T_IMAGE2D = 332,
     T_IIMAGE2D = 333,
     T_UIMAGE2D = 334,
     T_IMAGE2DARRAY = 335,
     T_IIMAGE2DARRAY = 336,
     T_UIMAGE2DARRAY = 337,
     T_IMAGE3D = 338,
     T_IIMAGE3D = 339,
     T_UIMAGE3D = 340,
     T_IMAGECUBE = 341,
     T_IIMAGECUBE = 342,
     T_UIMAGECUBE = 343,
     T_IMAGECUBEARRAY = 344,
     T_IIMAGECUBEARRAY = 345,
     T_UIMAGECUBEARRAY = 346,
     T_IMAGEBUFFER = 347,
     T_IIMAGEBUFFER = 348,
     T_UIMAGEBUFFER = 349,
     T_GEN_SAMPLER = 350,
     T_GEN_ISAMPLER = 351,
     T_GEN_USAMPLER = 352,
     T_ATOMIC_UINT = 353,
     T_BUFFER = 354,
     T_SHARED = 355,
     T_STRUCT = 356,
     T_VOID = 357,
     T_WHILE = 358,
     T_IO_BLOCK = 359,
     T_ARRAY4_OF_IVEC2 = 360,
     T_TYPE_MATCH_CALLBACK0 = 361,
     T_TYPE_MATCH_CALLBACK1 = 362,
     T_TYPE_MATCH_CALLBACK2 = 363,
     T_SWITCH = 364,
     T_CASE = 365,
     T_DEFAULT = 366,
     T_CENTROID = 367,
     T_FLAT = 368,
     T_SMOOTH = 369,
     T_LAYOUT = 370,
     T_UNIFORM_BLOCK = 371,
     T_IDENTIFIER = 372,
     T_TYPE_NAME = 373,
     T_FLOATCONSTANT = 374,
     T_INTCONSTANT = 375,
     T_BOOLCONSTANT = 376,
     T_UINTCONSTANT = 377,
     T_FIELD_SELECTION = 378,
     T_LEFT_OP = 379,
     T_RIGHT_OP = 380,
     T_INC_OP = 381,
     T_DEC_OP = 382,
     T_LE_OP = 383,
     T_GE_OP = 384,
     T_EQ_OP = 385,
     T_NE_OP = 386,
     T_AND_OP = 387,
     T_OR_OP = 388,
     T_XOR_OP = 389,
     T_MUL_ASSIGN = 390,
     T_DIV_ASSIGN = 391,
     T_ADD_ASSIGN = 392,
     T_MOD_ASSIGN = 393,
     T_LEFT_ASSIGN = 394,
     T_RIGHT_ASSIGN = 395,
     T_AND_ASSIGN = 396,
     T_XOR_ASSIGN = 397,
     T_OR_ASSIGN = 398,
     T_SUB_ASSIGN = 399,
     T_LENGTH_METHOD = 400,
     T_INVARIANT = 401,
     T_HIGH_PRECISION = 402,
     T_MEDIUM_PRECISION = 403,
     T_LOW_PRECISION = 404,
     T_PRECISION = 405,
     T_PRECISE = 406,
     T_COHERENT = 407,
     T_VOLATILE = 408,
     T_RESTRICT = 409,
     T_READONLY = 410,
     T_WRITEONLY = 411,
     T_VIV_ASM = 412,
     T_ASM_OPND_BRACKET = 413
   };
#endif
#define T_ATTRIBUTE 258
#define T_CONST 259
#define T_BOOL 260
#define T_FLOAT 261
#define T_INT 262
#define T_BREAK 263
#define T_CONTINUE 264
#define T_DO 265
#define T_ELSE 266
#define T_FOR 267
#define T_IF 268
#define T_DISCARD 269
#define T_RETURN 270
#define T_BASIC_TYPE 271
#define T_BVEC2 272
#define T_BVEC3 273
#define T_BVEC4 274
#define T_IVEC2 275
#define T_IVEC3 276
#define T_IVEC4 277
#define T_VEC2 278
#define T_VEC3 279
#define T_VEC4 280
#define T_MAT2 281
#define T_MAT3 282
#define T_MAT4 283
#define T_IN 284
#define T_OUT 285
#define T_INOUT 286
#define T_UNIFORM 287
#define T_VARYING 288
#define T_PATCH 289
#define T_SAMPLE 290
#define T_UINT 291
#define T_UVEC2 292
#define T_UVEC3 293
#define T_UVEC4 294
#define T_MAT2X3 295
#define T_MAT2X4 296
#define T_MAT3X2 297
#define T_MAT3X4 298
#define T_MAT4X2 299
#define T_MAT4X3 300
#define T_SAMPLER2D 301
#define T_SAMPLERCUBE 302
#define T_SAMPLERCUBEARRAY 303
#define T_SAMPLERCUBESHADOW 304
#define T_SAMPLERCUBEARRAYSHADOW 305
#define T_SAMPLER2DSHADOW 306
#define T_SAMPLER3D 307
#define T_SAMPLER1DARRAY 308
#define T_SAMPLER2DARRAY 309
#define T_SAMPLER1DARRAYSHADOW 310
#define T_SAMPLER2DARRAYSHADOW 311
#define T_ISAMPLER2D 312
#define T_ISAMPLERCUBE 313
#define T_ISAMPLERCUBEARRAY 314
#define T_ISAMPLER3D 315
#define T_ISAMPLER2DARRAY 316
#define T_USAMPLER2D 317
#define T_USAMPLERCUBE 318
#define T_USAMPLERCUBEARRAY 319
#define T_USAMPLER3D 320
#define T_USAMPLER2DARRAY 321
#define T_SAMPLEREXTERNALOES 322
#define T_SAMPLER2DMS 323
#define T_ISAMPLER2DMS 324
#define T_USAMPLER2DMS 325
#define T_SAMPLER2DMSARRAY 326
#define T_ISAMPLER2DMSARRAY 327
#define T_USAMPLER2DMSARRAY 328
#define T_SAMPLERBUFFER 329
#define T_ISAMPLERBUFFER 330
#define T_USAMPLERBUFFER 331
#define T_IMAGE2D 332
#define T_IIMAGE2D 333
#define T_UIMAGE2D 334
#define T_IMAGE2DARRAY 335
#define T_IIMAGE2DARRAY 336
#define T_UIMAGE2DARRAY 337
#define T_IMAGE3D 338
#define T_IIMAGE3D 339
#define T_UIMAGE3D 340
#define T_IMAGECUBE 341
#define T_IIMAGECUBE 342
#define T_UIMAGECUBE 343
#define T_IMAGECUBEARRAY 344
#define T_IIMAGECUBEARRAY 345
#define T_UIMAGECUBEARRAY 346
#define T_IMAGEBUFFER 347
#define T_IIMAGEBUFFER 348
#define T_UIMAGEBUFFER 349
#define T_GEN_SAMPLER 350
#define T_GEN_ISAMPLER 351
#define T_GEN_USAMPLER 352
#define T_ATOMIC_UINT 353
#define T_BUFFER 354
#define T_SHARED 355
#define T_STRUCT 356
#define T_VOID 357
#define T_WHILE 358
#define T_IO_BLOCK 359
#define T_ARRAY4_OF_IVEC2 360
#define T_TYPE_MATCH_CALLBACK0 361
#define T_TYPE_MATCH_CALLBACK1 362
#define T_TYPE_MATCH_CALLBACK2 363
#define T_SWITCH 364
#define T_CASE 365
#define T_DEFAULT 366
#define T_CENTROID 367
#define T_FLAT 368
#define T_SMOOTH 369
#define T_LAYOUT 370
#define T_UNIFORM_BLOCK 371
#define T_IDENTIFIER 372
#define T_TYPE_NAME 373
#define T_FLOATCONSTANT 374
#define T_INTCONSTANT 375
#define T_BOOLCONSTANT 376
#define T_UINTCONSTANT 377
#define T_FIELD_SELECTION 378
#define T_LEFT_OP 379
#define T_RIGHT_OP 380
#define T_INC_OP 381
#define T_DEC_OP 382
#define T_LE_OP 383
#define T_GE_OP 384
#define T_EQ_OP 385
#define T_NE_OP 386
#define T_AND_OP 387
#define T_OR_OP 388
#define T_XOR_OP 389
#define T_MUL_ASSIGN 390
#define T_DIV_ASSIGN 391
#define T_ADD_ASSIGN 392
#define T_MOD_ASSIGN 393
#define T_LEFT_ASSIGN 394
#define T_RIGHT_ASSIGN 395
#define T_AND_ASSIGN 396
#define T_XOR_ASSIGN 397
#define T_OR_ASSIGN 398
#define T_SUB_ASSIGN 399
#define T_LENGTH_METHOD 400
#define T_INVARIANT 401
#define T_HIGH_PRECISION 402
#define T_MEDIUM_PRECISION 403
#define T_LOW_PRECISION 404
#define T_PRECISION 405
#define T_PRECISE 406
#define T_COHERENT 407
#define T_VOLATILE 408
#define T_RESTRICT 409
#define T_READONLY 410
#define T_WRITEONLY 411
#define T_VIV_ASM 412
#define T_ASM_OPND_BRACKET 413




/* Copy the first part of user declarations.  */


#include "gc_glsl_parser.h"
#ifndef FILE
#define FILE        void
#endif
#ifndef stderr
#define stderr      gcvNULL
#endif


#define YY_NO_UNISTD_H

#define YYPARSE_PARAM_DECL  sloCOMPILER
#define YYPARSE_PARAM       Compiler

#define YYLEX_PARAM         Compiler

#define YY_DECL             int yylex(YYSTYPE * pyylval, sloCOMPILER Compiler)

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

typedef union YYSTYPE {
    slsLexToken                 token;

    slsDeclOrDeclList           declOrDeclList;

    slsDLINK_LIST *             fieldDeclList;

    slsFieldDecl *              fieldDecl;

    slsDATA_TYPE *              dataType;

    sloIR_EXPR              expr;

    slsNAME *               funcName;

    slsNAME *               paramName;

    slsNAME *               blockName;

    sloIR_SET                   statements;

    sloIR_BASE                  statement;

    slsSelectionStatementPair   selectionStatementPair;

    slsForExprPair              forExprPair;

    sloIR_POLYNARY_EXPR         funcCall;

    slsASM_OPCODE               asmOpcode;

    sloIR_VIV_ASM               vivAsm;

    slsASM_MODIFIER             asmModifier;

    slsASM_MODIFIERS            asmModifiers;
} YYSTYPE;
/* Line 191 of yacc.c.  */

# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif



/* Copy the second part of user declarations.  */


/* Line 214 of yacc.c.  */


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
     ((N) * (sizeof (short) + sizeof (YYSTYPE))             \
      + YYSTACK_GAP_MAXIMUM)

/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
#ifndef YYCOPY
#if defined (__GNUC__) && 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)      \
      do                    \
    {                   \
      register YYSIZE_T yyi;        \
      for (yyi = 0; yyi < (Count); yyi++)   \
        (To)[yyi] = (From)[yyi];        \
    }                   \
      while (0)
#  endif
# endif

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack)                    \
    do                                  \
      {                                 \
    YYSIZE_T yynewbytes;                        \
    YYCOPY (&yyptr->Stack, Stack, yysize);              \
    Stack = &yyptr->Stack;                      \
    yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
    yyptr += yynewbytes / sizeof (*yyptr);              \
      }                                 \
    while (0)

#endif

#if defined (__STDC__) || defined (__cplusplus)
   typedef signed char yysigned_char;
#else
   typedef short yysigned_char;
#endif

/* YYFINAL -- State number of the termination state. */
#define YYFINAL  165
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   5206

/* YYNTOKENS -- Number of terminals. */
#define YYNTOKENS  183
/* YYNNTS -- Number of nonterminals. */
#define YYNNTS  114
/* YYNRULES -- Number of rules. */
#define YYNRULES  367
/* YYNRULES -- Number of states. */
#define YYNSTATES  533

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   413

#define YYTRANSLATE(YYX)                        \
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const unsigned char yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   168,     2,     2,     2,   174,   179,     2,
     157,   158,   172,   171,   164,   169,   163,   173,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,   165,   167,
     175,   166,   176,   180,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,   159,     2,   160,   178,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   161,   177,   162,   170,     2,     2,     2,
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
     155,   156,   181,   182
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
     513,   519,   524,   526,   530,   532,   536,   540,   542,   544,
     546,   548,   550,   552,   555,   557,   560,   562,   564,   566,
     568,   570,   572,   576,   581,   584,   586,   588,   590,   592,
     594,   596,   598,   600,   602,   604,   606,   608,   610,   612,
     614,   616,   618,   620,   622,   624,   626,   628,   630,   632,
     634,   636,   638,   640,   642,   644,   646,   648,   650,   652,
     654,   656,   658,   660,   662,   664,   666,   668,   670,   672,
     674,   676,   678,   680,   682,   684,   686,   688,   690,   692,
     694,   696,   698,   700,   702,   704,   706,   708,   710,   712,
     714,   716,   718,   720,   722,   724,   726,   728,   730,   732,
     734,   736,   738,   740,   742,   744,   746,   748,   750,   752,
     754,   756,   758,   760,   762,   764,   765,   772,   773,   779,
     780,   787,   789,   792,   796,   801,   803,   807,   809,   814,
     817,   820,   824,   831,   837,   838,   845,   846,   853,   855,
     858,   860,   864,   868,   870,   875,   879,   882,   884,   886,
     888,   890,   892,   894,   896,   898,   900,   902,   905,   906,
     911,   913,   915,   918,   919,   924,   926,   929,   931,   934,
     935,   942,   948,   950,   953,   955,   959,   962,   965,   966,
     971,   972,   977,   979,   981,   986,   987,   994,  1002,  1003,
    1011,  1013,  1015,  1017,  1018,  1021,  1025,  1028,  1031,  1034,
    1038,  1041,  1043,  1046,  1048,  1050,  1051,  1052
};

/* YYRHS -- A `-1'-separated list of the rules' RHS. */
static const short yyrhs[] =
{
     292,     0,    -1,   117,    -1,   184,    -1,   120,    -1,   122,
      -1,   119,    -1,   121,    -1,   157,   216,   158,    -1,   185,
      -1,   186,   159,   187,   160,    -1,   194,    -1,   186,   163,
     123,    -1,   186,   163,   145,    -1,   186,   126,    -1,   186,
     127,    -1,   216,    -1,   189,   158,   167,    -1,   193,    -1,
     189,   164,   190,    -1,   214,    -1,   214,   182,   191,   176,
      -1,   192,    -1,   191,   164,   192,    -1,   117,   165,   117,
      -1,   181,   157,   117,    -1,   181,   157,   117,   182,   191,
     176,    -1,   195,    -1,   197,   158,    -1,   196,   158,    -1,
     198,   102,    -1,   198,    -1,   198,   214,    -1,   197,   164,
     214,    -1,   199,   157,    -1,   232,    -1,   117,    -1,   186,
      -1,   126,   200,    -1,   127,   200,    -1,   201,   200,    -1,
     171,    -1,   169,    -1,   168,    -1,   170,    -1,   200,    -1,
     202,   172,   200,    -1,   202,   173,   200,    -1,   202,   174,
     200,    -1,   202,    -1,   203,   171,   202,    -1,   203,   169,
     202,    -1,   203,    -1,   204,   124,   203,    -1,   204,   125,
     203,    -1,   204,    -1,   205,   175,   204,    -1,   205,   176,
     204,    -1,   205,   128,   204,    -1,   205,   129,   204,    -1,
     205,    -1,   206,   130,   205,    -1,   206,   131,   205,    -1,
     206,    -1,   207,   179,   206,    -1,   207,    -1,   208,   178,
     207,    -1,   208,    -1,   209,   177,   208,    -1,   209,    -1,
     210,   132,   209,    -1,   210,    -1,   211,   134,   210,    -1,
     211,    -1,   212,   133,   211,    -1,   212,    -1,   212,   180,
     216,   165,   214,    -1,   213,    -1,   200,   215,   214,    -1,
     166,    -1,   135,    -1,   136,    -1,   138,    -1,   137,    -1,
     144,    -1,   139,    -1,   140,    -1,   141,    -1,   142,    -1,
     143,    -1,   214,    -1,   216,   164,   214,    -1,   213,    -1,
     159,   160,   159,   160,    -1,   159,   160,   159,   217,   160,
      -1,   159,   217,   160,   159,   160,    -1,   159,   217,   160,
     159,   217,   160,    -1,   218,   159,   160,    -1,   218,   159,
     217,   160,    -1,   220,   167,    -1,   230,   167,    -1,   150,
     247,   246,   167,    -1,   257,    -1,   233,   167,    -1,   221,
     158,    -1,   223,    -1,   222,    -1,   223,   225,    -1,   222,
     164,   225,    -1,   232,   117,   157,    -1,   243,   117,    -1,
     243,   117,   159,   217,   160,    -1,   243,   117,   218,    -1,
     224,    -1,   229,    -1,   228,   224,    -1,   228,   229,    -1,
      29,    -1,    30,    -1,    31,    -1,   226,    -1,   240,    -1,
     247,    -1,   227,    -1,   228,   227,    -1,   243,    -1,   231,
      -1,   230,   164,   117,    -1,   230,   164,   117,   159,   217,
     160,    -1,   230,   164,   117,   159,   160,    -1,   230,   164,
     117,   218,    -1,   230,   164,   117,   166,   265,    -1,   230,
     164,   117,   159,   160,   166,   265,    -1,   230,   164,   117,
     159,   217,   160,   166,   265,    -1,   230,   164,   117,   218,
     166,   265,    -1,   232,    -1,   232,   117,    -1,   232,   117,
     159,   217,   160,    -1,   232,   117,   159,   160,    -1,   232,
     117,   218,    -1,   232,   117,   159,   160,   166,   265,    -1,
     232,   117,   159,   217,   160,   166,   265,    -1,   232,   117,
     166,   265,    -1,   233,   117,    -1,   232,   117,   218,   166,
     265,    -1,   243,    -1,   233,   243,    -1,   234,    -1,   233,
     234,    -1,   242,    -1,   248,    -1,   236,    -1,   146,    -1,
     151,    -1,   247,    -1,   235,    -1,   241,    -1,   114,    -1,
     113,    -1,    -1,   115,   157,   238,   237,   158,    -1,   115,
     157,     1,   158,    -1,   239,    -1,   238,   164,   239,    -1,
     117,    -1,   117,   166,   120,    -1,   117,   166,   122,    -1,
       4,    -1,   151,    -1,   112,    -1,    35,    -1,     4,    -1,
      29,    -1,    34,    29,    -1,    30,    -1,    34,    30,    -1,
       3,    -1,    33,    -1,    32,    -1,    99,    -1,   100,    -1,
     245,    -1,   245,   159,   160,    -1,   245,   159,   217,   160,
      -1,   245,   218,    -1,    46,    -1,    47,    -1,   249,    -1,
     118,    -1,    52,    -1,    53,    -1,    54,    -1,    55,    -1,
      51,    -1,    56,    -1,    49,    -1,    50,    -1,    57,    -1,
      58,    -1,    60,    -1,    61,    -1,    62,    -1,    63,    -1,
      65,    -1,    66,    -1,    67,    -1,    68,    -1,    69,    -1,
      70,    -1,    71,    -1,    72,    -1,    73,    -1,    48,    -1,
      59,    -1,    64,    -1,    77,    -1,    78,    -1,    79,    -1,
      80,    -1,    81,    -1,    82,    -1,    83,    -1,    84,    -1,
      85,    -1,    86,    -1,    89,    -1,    87,    -1,    90,    -1,
      88,    -1,    91,    -1,    98,    -1,    74,    -1,    75,    -1,
      76,    -1,    92,    -1,    93,    -1,    94,    -1,   102,    -1,
       6,    -1,     7,    -1,    36,    -1,     5,    -1,    23,    -1,
      24,    -1,    25,    -1,    17,    -1,    18,    -1,    19,    -1,
      20,    -1,    21,    -1,    22,    -1,    37,    -1,    38,    -1,
      39,    -1,    26,    -1,    40,    -1,    41,    -1,    27,    -1,
      42,    -1,    43,    -1,    28,    -1,    44,    -1,    45,    -1,
     244,    -1,     6,    -1,     7,    -1,   244,    -1,   147,    -1,
     148,    -1,   149,    -1,   152,    -1,   153,    -1,   154,    -1,
     155,    -1,   156,    -1,    -1,   101,   117,   161,   250,   253,
     162,    -1,    -1,   101,   161,   251,   253,   162,    -1,    -1,
     101,   118,   161,   252,   253,   162,    -1,   254,    -1,   253,
     254,    -1,   243,   255,   167,    -1,   247,   243,   255,   167,
      -1,   256,    -1,   255,   164,   256,    -1,   117,    -1,   117,
     159,   217,   160,    -1,   117,   218,    -1,   258,   167,    -1,
     258,   117,   167,    -1,   258,   117,   159,   217,   160,   167,
      -1,   258,   117,   159,   160,   167,    -1,    -1,   233,   117,
     161,   259,   261,   162,    -1,    -1,   233,   117,   161,   260,
       1,   162,    -1,   263,    -1,   261,   263,    -1,   264,    -1,
     262,   164,   264,    -1,   232,   262,   167,    -1,   117,    -1,
     117,   159,   217,   160,    -1,   117,   159,   160,    -1,   117,
     218,    -1,   214,    -1,   219,    -1,   269,    -1,   268,    -1,
     266,    -1,   275,    -1,   276,    -1,   285,    -1,   291,    -1,
     188,    -1,   161,   162,    -1,    -1,   161,   270,   274,   162,
      -1,   272,    -1,   268,    -1,   161,   162,    -1,    -1,   161,
     273,   274,   162,    -1,   267,    -1,   274,   267,    -1,   167,
      -1,   216,   167,    -1,    -1,    13,   157,   216,   158,   277,
     282,    -1,   109,   157,   187,   158,   280,    -1,   279,    -1,
     278,   279,    -1,   267,    -1,   110,   217,   165,    -1,   111,
     165,    -1,   161,   162,    -1,    -1,   161,   281,   278,   162,
      -1,    -1,   267,    11,   283,   267,    -1,   267,    -1,   216,
      -1,   232,   117,   166,   265,    -1,    -1,   103,   286,   157,
     284,   158,   271,    -1,    10,   267,   103,   157,   216,   158,
     167,    -1,    -1,    12,   287,   157,   288,   290,   158,   271,
      -1,   275,    -1,   266,    -1,   284,    -1,    -1,   289,   167,
      -1,   289,   167,   216,    -1,     9,   167,    -1,     8,   167,
      -1,    15,   167,    -1,    15,   216,   167,    -1,    14,   167,
      -1,   293,    -1,   292,   293,    -1,   294,    -1,   219,    -1,
      -1,    -1,   220,   295,   272,   296,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const unsigned short yyrline[] =
{
       0,   183,   183,   188,   190,   192,   194,   196,   198,   203,
     205,   207,   209,   211,   213,   215,   220,   225,   230,   232,
     237,   239,   244,   246,   251,   256,   258,   263,   268,   270,
     275,   277,   282,   284,   289,   294,   296,   301,   303,   305,
     307,   314,   316,   318,   320,   327,   329,   331,   333,   338,
     340,   342,   347,   349,   351,   356,   358,   360,   362,   364,
     369,   371,   373,   378,   380,   385,   387,   392,   394,   399,
     401,   406,   408,   413,   415,   420,   422,   427,   429,   434,
     436,   438,   440,   442,   444,   446,   448,   450,   452,   454,
     459,   461,   466,   471,   473,   475,   477,   479,   481,   486,
     488,   490,   492,   494,   499,   504,   506,   511,   513,   518,
     523,   525,   527,   532,   534,   536,   538,   543,   545,   547,
     553,   555,   557,   561,   563,   568,   582,   584,   586,   588,
     590,   592,   594,   596,   598,   603,   605,   607,   609,   611,
     613,   615,   617,   619,   621,   628,   630,   635,   637,   642,
     644,   646,   648,   650,   652,   654,   656,   661,   663,   669,
     668,   676,   684,   686,   691,   693,   695,   700,   702,   707,
     709,   714,   716,   718,   720,   722,   724,   726,   728,   730,
     732,   737,   738,   740,   742,   747,   749,   751,   753,   755,
     757,   759,   761,   763,   765,   767,   769,   771,   773,   775,
     777,   779,   781,   783,   785,   787,   789,   791,   793,   795,
     797,   799,   801,   803,   805,   807,   809,   811,   813,   815,
     817,   819,   821,   823,   825,   827,   829,   831,   833,   835,
     837,   839,   841,   843,   845,   847,   849,   854,   856,   858,
     860,   862,   864,   866,   868,   870,   872,   874,   876,   878,
     880,   882,   884,   886,   888,   890,   892,   894,   896,   898,
     900,   902,   904,   906,   911,   913,   915,   920,   922,   924,
     929,   931,   933,   935,   937,   943,   942,   947,   946,   951,
     950,   957,   958,   962,   964,   973,   975,   980,   982,   984,
     989,   991,   993,   995,  1001,  1000,  1005,  1004,  1015,  1016,
    1020,  1022,  1027,  1031,  1033,  1035,  1037,  1042,  1047,  1052,
    1054,  1061,  1063,  1065,  1067,  1069,  1071,  1076,  1079,  1078,
    1085,  1087,  1092,  1095,  1094,  1101,  1103,  1108,  1110,  1116,
    1115,  1119,  1124,  1126,  1131,  1133,  1135,  1140,  1143,  1142,
    1151,  1149,  1156,  1164,  1166,  1172,  1171,  1175,  1178,  1177,
    1184,  1186,  1191,  1194,  1198,  1200,  1205,  1207,  1209,  1211,
    1213,  1220,  1221,  1225,  1226,  1232,  1234,  1231
};
#endif

#if YYDEBUG || YYERROR_VERBOSE
/* YYTNME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals. */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "T_ATTRIBUTE", "T_CONST", "T_BOOL",
  "T_FLOAT", "T_INT", "T_BREAK", "T_CONTINUE", "T_DO", "T_ELSE", "T_FOR",
  "T_IF", "T_DISCARD", "T_RETURN", "T_BASIC_TYPE", "T_BVEC2", "T_BVEC3",
  "T_BVEC4", "T_IVEC2", "T_IVEC3", "T_IVEC4", "T_VEC2", "T_VEC3", "T_VEC4",
  "T_MAT2", "T_MAT3", "T_MAT4", "T_IN", "T_OUT", "T_INOUT", "T_UNIFORM",
  "T_VARYING", "T_PATCH", "T_SAMPLE", "T_UINT", "T_UVEC2", "T_UVEC3",
  "T_UVEC4", "T_MAT2X3", "T_MAT2X4", "T_MAT3X2", "T_MAT3X4", "T_MAT4X2",
  "T_MAT4X3", "T_SAMPLER2D", "T_SAMPLERCUBE", "T_SAMPLERCUBEARRAY",
  "T_SAMPLERCUBESHADOW", "T_SAMPLERCUBEARRAYSHADOW", "T_SAMPLER2DSHADOW",
  "T_SAMPLER3D", "T_SAMPLER1DARRAY", "T_SAMPLER2DARRAY",
  "T_SAMPLER1DARRAYSHADOW", "T_SAMPLER2DARRAYSHADOW", "T_ISAMPLER2D",
  "T_ISAMPLERCUBE", "T_ISAMPLERCUBEARRAY", "T_ISAMPLER3D",
  "T_ISAMPLER2DARRAY", "T_USAMPLER2D", "T_USAMPLERCUBE",
  "T_USAMPLERCUBEARRAY", "T_USAMPLER3D", "T_USAMPLER2DARRAY",
  "T_SAMPLEREXTERNALOES", "T_SAMPLER2DMS", "T_ISAMPLER2DMS",
  "T_USAMPLER2DMS", "T_SAMPLER2DMSARRAY", "T_ISAMPLER2DMSARRAY",
  "T_USAMPLER2DMSARRAY", "T_SAMPLERBUFFER", "T_ISAMPLERBUFFER",
  "T_USAMPLERBUFFER", "T_IMAGE2D", "T_IIMAGE2D", "T_UIMAGE2D",
  "T_IMAGE2DARRAY", "T_IIMAGE2DARRAY", "T_UIMAGE2DARRAY", "T_IMAGE3D",
  "T_IIMAGE3D", "T_UIMAGE3D", "T_IMAGECUBE", "T_IIMAGECUBE",
  "T_UIMAGECUBE", "T_IMAGECUBEARRAY", "T_IIMAGECUBEARRAY",
  "T_UIMAGECUBEARRAY", "T_IMAGEBUFFER", "T_IIMAGEBUFFER", "T_UIMAGEBUFFER",
  "T_GEN_SAMPLER", "T_GEN_ISAMPLER", "T_GEN_USAMPLER", "T_ATOMIC_UINT",
  "T_BUFFER", "T_SHARED", "T_STRUCT", "T_VOID", "T_WHILE", "T_IO_BLOCK",
  "T_ARRAY4_OF_IVEC2", "T_TYPE_MATCH_CALLBACK0", "T_TYPE_MATCH_CALLBACK1",
  "T_TYPE_MATCH_CALLBACK2", "T_SWITCH", "T_CASE", "T_DEFAULT",
  "T_CENTROID", "T_FLAT", "T_SMOOTH", "T_LAYOUT", "T_UNIFORM_BLOCK",
  "T_IDENTIFIER", "T_TYPE_NAME", "T_FLOATCONSTANT", "T_INTCONSTANT",
  "T_BOOLCONSTANT", "T_UINTCONSTANT", "T_FIELD_SELECTION", "T_LEFT_OP",
  "T_RIGHT_OP", "T_INC_OP", "T_DEC_OP", "T_LE_OP", "T_GE_OP", "T_EQ_OP",
  "T_NE_OP", "T_AND_OP", "T_OR_OP", "T_XOR_OP", "T_MUL_ASSIGN",
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
     405,   406,   407,   408,   409,   410,   411,    40,    41,    91,
      93,   123,   125,    46,    44,    58,    61,    59,    33,    45,
     126,    43,    42,    47,    37,    60,    62,   124,    94,    38,
      63,   412,   413
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const unsigned short yyr1[] =
{
       0,   183,   184,   185,   185,   185,   185,   185,   185,   186,
     186,   186,   186,   186,   186,   186,   187,   188,   189,   189,
     190,   190,   191,   191,   192,   193,   193,   194,   195,   195,
     196,   196,   197,   197,   198,   199,   199,   200,   200,   200,
     200,   201,   201,   201,   201,   202,   202,   202,   202,   203,
     203,   203,   204,   204,   204,   205,   205,   205,   205,   205,
     206,   206,   206,   207,   207,   208,   208,   209,   209,   210,
     210,   211,   211,   212,   212,   213,   213,   214,   214,   215,
     215,   215,   215,   215,   215,   215,   215,   215,   215,   215,
     216,   216,   217,   218,   218,   218,   218,   218,   218,   219,
     219,   219,   219,   219,   220,   221,   221,   222,   222,   223,
     224,   224,   224,   225,   225,   225,   225,   226,   226,   226,
     227,   227,   227,   228,   228,   229,   230,   230,   230,   230,
     230,   230,   230,   230,   230,   231,   231,   231,   231,   231,
     231,   231,   231,   231,   231,   232,   232,   233,   233,   234,
     234,   234,   234,   234,   234,   234,   234,   235,   235,   237,
     236,   236,   238,   238,   239,   239,   239,   240,   240,   241,
     241,   242,   242,   242,   242,   242,   242,   242,   242,   242,
     242,   243,   243,   243,   243,   244,   244,   244,   244,   244,
     244,   244,   244,   244,   244,   244,   244,   244,   244,   244,
     244,   244,   244,   244,   244,   244,   244,   244,   244,   244,
     244,   244,   244,   244,   244,   244,   244,   244,   244,   244,
     244,   244,   244,   244,   244,   244,   244,   244,   244,   244,
     244,   244,   244,   244,   244,   244,   244,   245,   245,   245,
     245,   245,   245,   245,   245,   245,   245,   245,   245,   245,
     245,   245,   245,   245,   245,   245,   245,   245,   245,   245,
     245,   245,   245,   245,   246,   246,   246,   247,   247,   247,
     248,   248,   248,   248,   248,   250,   249,   251,   249,   252,
     249,   253,   253,   254,   254,   255,   255,   256,   256,   256,
     257,   257,   257,   257,   259,   258,   260,   258,   261,   261,
     262,   262,   263,   264,   264,   264,   264,   265,   266,   267,
     267,   268,   268,   268,   268,   268,   268,   269,   270,   269,
     271,   271,   272,   273,   272,   274,   274,   275,   275,   277,
     276,   276,   278,   278,   279,   279,   279,   280,   281,   280,
     283,   282,   282,   284,   284,   286,   285,   285,   287,   285,
     288,   288,   289,   289,   290,   290,   291,   291,   291,   291,
     291,   292,   292,   293,   293,   295,   296,   294
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
       1,     1,     1,     1,     1,     1,     1,     1,     1,     0,
       5,     4,     1,     3,     1,     3,     3,     1,     1,     1,
       1,     1,     1,     2,     1,     2,     1,     1,     1,     1,
       1,     1,     3,     4,     2,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     0,     6,     0,     5,     0,
       6,     1,     2,     3,     4,     1,     3,     1,     4,     2,
       2,     3,     6,     5,     0,     6,     0,     6,     1,     2,
       1,     3,     3,     1,     4,     3,     2,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     2,     0,     4,
       1,     1,     2,     0,     4,     1,     2,     1,     2,     0,
       6,     5,     1,     2,     1,     3,     2,     2,     0,     4,
       0,     4,     1,     1,     4,     0,     6,     7,     0,     7,
       1,     1,     1,     0,     2,     3,     2,     2,     2,     3,
       2,     1,     2,     1,     1,     0,     0,     4
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const unsigned short yydefact[] =
{
       0,   176,   171,   241,   238,   239,   245,   246,   247,   248,
     249,   250,   242,   243,   244,   254,   257,   260,   172,   174,
     178,   177,     0,   170,   240,   251,   252,   253,   255,   256,
     258,   259,   261,   262,   185,   186,   212,   195,   196,   193,
     189,   190,   191,   192,   194,   197,   198,   213,   199,   200,
     201,   202,   214,   203,   204,   205,   206,   207,   208,   209,
     210,   211,   231,   232,   233,   215,   216,   217,   218,   219,
     220,   221,   222,   223,   224,   226,   228,   225,   227,   229,
     234,   235,   236,   230,   179,   180,     0,   237,   169,   158,
     157,     0,   188,   152,   267,   268,   269,     0,   153,   270,
     271,   272,   273,   274,   364,   365,     0,   106,   105,     0,
     126,   135,     0,   147,   155,   151,   156,   149,   145,   263,
     181,   154,   150,   187,   102,     0,     0,   361,   363,   173,
     175,     0,     0,   277,     0,     0,    99,     0,   104,     0,
     167,   117,   118,   119,   168,   113,   107,   120,   123,     0,
     114,   121,   125,   122,     0,   100,   136,   143,   103,   148,
     146,     0,   184,     0,   290,     1,   362,   275,   279,     0,
       0,   164,   159,   162,   264,   265,   266,     0,   323,   366,
     108,   115,   124,   116,   110,   127,   109,     0,     0,   139,
     294,     2,     6,     4,     7,     5,     0,     0,     0,   182,
      43,    42,    44,    41,     3,     9,    37,    11,    27,     0,
       0,    31,     0,    45,     0,    49,    52,    55,    60,    63,
      65,    67,    69,    71,    73,    75,    92,     0,    35,     0,
       0,     0,   291,     0,     0,     0,     0,     0,   281,   161,
       0,     0,     0,   101,   322,     0,   367,     0,   112,     0,
       0,   130,   138,     0,    45,    77,   307,   142,     0,     0,
       0,    38,    39,    90,     0,     0,    14,    15,     0,     0,
      29,    28,     0,   237,    32,    34,    40,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   183,    97,     0,
       0,     0,     0,     0,   287,     0,   285,     0,   278,   282,
     165,   166,   163,   160,     0,     0,     0,   348,     0,     0,
       0,   345,     0,   318,   327,     0,   316,     0,    18,     0,
     308,     0,   135,   311,   325,   310,   309,     0,   312,   313,
     314,   315,     0,     0,   129,     0,   131,     0,     0,   137,
      80,    81,    83,    82,    85,    86,    87,    88,    89,    84,
      79,     0,   144,     0,     0,   298,     0,     8,     0,    93,
       0,     0,    16,    12,    13,    33,    46,    47,    48,    51,
      50,    53,    54,    58,    59,    56,    57,    61,    62,    64,
      66,    68,    70,    72,    74,     0,     0,    98,   293,     0,
     276,   280,     0,   289,     0,   283,     0,   357,   356,     0,
       0,     0,   360,   358,     0,     0,     0,   317,     0,     0,
       0,     0,   328,   324,   326,   111,     0,   128,   134,   140,
       0,    78,   303,     0,   300,   295,   299,   297,    91,    94,
      10,     0,    95,     0,   292,     0,   286,   284,     0,     0,
       0,   359,     0,     0,     0,    25,    17,    19,    20,   132,
       0,   141,     0,   306,     0,   302,    76,    96,   288,     0,
     351,   350,   353,   329,   343,    35,     0,     0,   319,     0,
       0,   133,   305,     0,   301,     0,   352,     0,     0,     0,
       0,     0,   338,   331,     0,     0,    22,     0,   304,     0,
     354,     0,   342,   330,     0,   321,   346,   320,   337,     0,
       0,     0,    26,    21,   347,   355,   349,   340,   344,     0,
       0,   334,     0,   332,    24,    23,     0,     0,   336,   339,
     333,   341,   335
};

/* YYDEFGOTO[NTERM-NUM]. */
static const short yydefgoto[] =
{
      -1,   204,   205,   206,   371,   326,   327,   457,   495,   496,
     328,   207,   208,   209,   210,   211,   212,   254,   214,   215,
     216,   217,   218,   219,   220,   221,   222,   223,   224,   225,
     255,   263,   361,   329,   227,   162,   330,   331,   106,   107,
     108,   145,   146,   147,   148,   149,   150,   109,   110,   228,
     229,   113,   114,   115,   242,   172,   173,   151,   116,   117,
     118,   119,   120,   177,   121,   122,   123,   233,   169,   234,
     237,   238,   305,   306,   124,   125,   259,   260,   364,   433,
     365,   434,   257,   333,   334,   335,   336,   418,   506,   507,
     245,   337,   338,   339,   489,   522,   523,   493,   509,   503,
     526,   476,   340,   415,   410,   472,   487,   488,   341,   126,
     127,   128,   137,   246
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -463
static const short yypact[] =
{
    4462,  -463,  -463,  -463,  -463,  -463,  -463,  -463,  -463,  -463,
    -463,  -463,  -463,  -463,  -463,  -463,  -463,  -463,  -463,  -463,
    -463,  -463,    -9,  -463,  -463,  -463,  -463,  -463,  -463,  -463,
    -463,  -463,  -463,  -463,  -463,  -463,  -463,  -463,  -463,  -463,
    -463,  -463,  -463,  -463,  -463,  -463,  -463,  -463,  -463,  -463,
    -463,  -463,  -463,  -463,  -463,  -463,  -463,  -463,  -463,  -463,
    -463,  -463,  -463,  -463,  -463,  -463,  -463,  -463,  -463,  -463,
    -463,  -463,  -463,  -463,  -463,  -463,  -463,  -463,  -463,  -463,
    -463,  -463,  -463,  -463,  -463,  -463,  -102,  -463,  -463,  -463,
    -463,  -121,  -463,  -463,  -463,  -463,  -463,    31,  -463,  -463,
    -463,  -463,  -463,  -463,  -463,  -127,  -109,  -110,  4769,   -70,
    -463,   -51,  3586,  -463,  -463,  -463,  -463,  -463,  -463,  -463,
     -91,  -463,  -463,  -463,  -463,  -112,  4308,  -463,  -463,  -463,
    -463,   -47,   -36,  -463,     5,  5088,  -463,    20,  -463,  4769,
    -463,  -463,  -463,  -463,  -463,  -463,  -463,  -463,  -463,  4769,
    -463,  -463,   -39,  -463,    52,  -463,  -113,    22,  -463,  -463,
    -463,  1699,    17,  -132,  -463,  -463,  -463,  -463,  -463,  4885,
      27,    25,    36,  -463,  -463,  -463,  -463,    38,    41,  -463,
    -463,  -463,  -463,  -463,    48,  -114,  -463,  1857,  3276,   -96,
     209,    57,  -463,  -463,  -463,  -463,  3276,  3276,  3276,    62,
    -463,  -463,  -463,  -463,  -463,  -463,   -84,  -463,  -463,    69,
     -54,  3431,    66,  -463,  3276,    15,  -107,   -23,  -115,    65,
      50,    53,    56,    98,   100,  -122,  -463,    72,  -463,  4616,
    2015,  2173,  -463,  4885,  4885,   118,  4999,  3892,  -463,  -463,
     -31,   119,    82,  -463,  -463,  1205,  -463,  2331,    17,  2489,
    3276,   -83,   -82,    81,    18,  -463,  -463,  -463,  3276,  4616,
     241,  -463,  -463,  -463,   -53,  2647,  -463,  -463,  3276,  -104,
    -463,  -463,  3276,    90,  -463,  -463,  -463,  3276,  3276,  3276,
    3276,  3276,  3276,  3276,  3276,  3276,  3276,  3276,  3276,  3276,
    3276,  3276,  3276,  3276,  3276,  3276,  3276,    91,  -463,    92,
      84,    94,  4025,  4158,    96,   -44,  -463,   118,  -463,  -463,
    -463,  -463,  -463,  -463,    93,    95,  1205,  -463,    99,   101,
    2805,  -463,   104,    87,  -463,   106,  -463,   -49,  -463,     4,
    -463,  -127,  -100,  -463,  -463,  -463,  -463,   865,  -463,  -463,
    -463,  -463,    62,    97,   -79,   105,  -463,  3276,  3276,   -73,
    -463,  -463,  -463,  -463,  -463,  -463,  -463,  -463,  -463,  -463,
    -463,  3276,  -463,   150,  3740,  -463,   107,  -463,  3276,  -463,
     110,   112,   109,  -463,  -463,  -463,  -463,  -463,  -463,    15,
      15,  -107,  -107,   -23,   -23,   -23,   -23,  -115,  -115,    65,
      50,    53,    56,    98,   100,    34,  2960,  -463,  -463,   111,
    -463,  -463,  2331,    17,   118,  -463,     6,  -463,  -463,   171,
     120,  3276,  -463,  -463,     8,   124,  3276,  -463,  1205,   159,
     116,  3276,  -463,  -463,  -463,    91,  3276,   -67,  -463,  -463,
    3276,  -463,   125,    10,  -463,  -463,  -463,  -463,  -463,  -463,
    -463,  3276,  -463,   127,  -463,   130,  -463,  -463,   128,  1544,
     -46,  -463,  3276,   133,  1035,   113,  -463,  -463,   114,  -463,
    3276,  -463,  3118,    17,   150,  -463,  -463,  -463,    91,  3276,
    -463,  -463,  3276,  -463,   109,   176,   136,   137,  -463,   182,
     182,  -463,    62,   141,  -463,   -45,  -463,   135,   146,  1205,
     140,  1375,   145,  -463,   143,  -142,  -463,  -139,    91,   142,
    3276,  1375,   299,  -463,  3276,  -463,  -463,  -463,  -463,   695,
     194,   182,  -463,  -463,  -463,   109,  -463,  -463,  -463,  3276,
     147,  -463,   525,  -463,  -463,  -463,  1205,   149,  -463,  -463,
    -463,  -463,  -463
};

/* YYPGOTO[NTERM-NUM].  */
static const short yypgoto[] =
{
    -463,  -463,  -463,  -463,  -101,  -463,  -463,  -463,  -161,  -191,
    -463,  -463,  -463,  -463,  -463,  -463,  -463,  -149,  -463,   -72,
     -71,  -120,   -63,    37,    30,    32,    39,    35,    40,  -463,
    -159,  -155,  -463,  -172,  -180,  -153,    23,    24,  -463,  -463,
    -463,   181,   192,  -463,   184,  -463,   185,  -463,  -463,     0,
       1,  -108,  -463,  -463,  -463,  -463,   102,  -463,  -463,  -463,
      89,   201,  -463,  -463,    55,  -463,  -463,  -463,  -463,  -463,
     -16,  -229,    33,   -62,  -463,  -463,  -463,  -463,  -463,  -463,
     -20,  -123,  -240,  -103,  -307,  -462,  -463,  -463,  -156,   210,
    -463,   -68,   -98,  -463,  -463,  -463,  -174,  -463,  -463,  -463,
    -463,  -119,  -463,  -463,  -463,  -463,  -463,  -463,  -463,  -463,
     226,  -463,  -463,  -463
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -297
static const short yytable[] =
{
     111,   112,   226,   189,   159,   163,   170,   253,   309,   409,
     346,   295,   213,   284,   285,   131,   132,   156,   362,   373,
     129,   130,   511,   104,   105,   511,   264,   231,   226,   505,
     424,   248,   251,   256,   512,   232,   134,   513,   213,   505,
     136,   374,   266,   267,   186,   249,   187,   261,   262,   138,
     299,   301,   250,   188,   139,   164,   274,   -35,   296,   133,
     286,   287,   280,   230,   281,   276,   156,   343,   161,   345,
     258,   226,   226,   309,   309,   268,   230,   265,   184,   269,
     265,   213,   213,   347,   348,   370,   396,   426,   226,   310,
     226,   311,   396,   430,   154,   256,   372,   155,   213,   460,
     213,   282,   283,   256,   271,   367,   226,   428,   429,   420,
     272,   368,   473,   499,   167,   421,   213,   375,   368,   368,
     404,   159,   171,   405,   395,   168,   111,   112,   376,   377,
     378,   213,   213,   213,   213,   213,   213,   213,   213,   213,
     213,   213,   213,   213,   213,   213,   213,   424,   414,   104,
     105,   403,   135,   350,   351,   352,   353,   354,   355,   356,
     357,   358,   359,   153,   383,   384,   385,   386,   368,   185,
     404,   422,   368,   447,   464,   451,   230,   465,    94,    95,
      96,   178,   502,   190,   360,   239,   459,   277,   278,   279,
     461,   240,   256,   256,   153,   288,   289,   152,   368,   441,
     241,   160,   521,   244,   153,   243,   431,   247,   379,   380,
    -296,   381,   382,   438,   -36,   521,   443,   302,   303,   531,
     481,   265,   445,   275,   236,   387,   388,   270,   152,   290,
     293,   291,   297,   292,   294,   304,   171,   226,   152,   450,
     313,   349,   366,   226,   372,   332,   112,   213,   -30,   417,
     396,   398,   397,   213,   399,   402,   411,   425,   235,   363,
     407,   416,   408,   419,   518,   427,   458,   432,   412,   437,
     439,   256,   440,   368,   448,   256,   455,   449,   444,   463,
     474,   452,   483,   456,   462,   469,   466,   467,   236,   236,
     468,   477,   236,   490,   491,   479,   480,   485,   492,   494,
     474,   498,   500,   226,   501,   256,   504,   508,   510,   514,
     517,   524,   528,   213,   532,   453,   332,   112,   160,   497,
     525,   390,   235,   235,   391,   307,   235,   389,   515,   393,
     181,   180,   392,   182,   183,   394,   176,   332,   112,   527,
     406,   484,   446,   312,   436,   516,   470,   179,   530,   256,
     454,   471,   166,   486,     0,     0,     0,   236,   236,     0,
     226,     0,     0,     0,   363,     0,     0,     0,     0,     0,
     213,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   235,   235,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   332,   112,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   332,
     112,     0,   475,     0,   332,   112,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   475,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   332,
     112,   332,   112,     0,     0,     0,     0,     0,     0,     0,
       0,   332,   112,     0,     0,     0,     0,     0,     0,   332,
     112,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   332,   112,     0,     0,   332,   112,     1,     2,
       3,     4,     5,   314,   315,   316,     0,   317,   318,   319,
     320,     0,     6,     7,     8,     9,    10,    11,    12,    13,
      14,    15,    16,    17,    18,    19,     0,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    71,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
       0,     0,     0,    83,    84,    85,    86,    87,   321,     0,
       0,     0,     0,     0,   322,   519,   520,    88,    89,    90,
      91,     0,   191,    92,   192,   193,   194,   195,     0,     0,
       0,   196,   197,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    93,    94,    95,    96,    97,    98,    99,   100,   101,
     102,   103,   198,     0,     0,     0,   323,   529,     0,     0,
       0,     0,   324,   200,   201,   202,   203,     0,     1,     2,
       3,     4,     5,   314,   315,   316,   325,   317,   318,   319,
     320,     0,     6,     7,     8,     9,    10,    11,    12,    13,
      14,    15,    16,    17,    18,    19,     0,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    71,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
       0,     0,     0,    83,    84,    85,    86,    87,   321,     0,
       0,     0,     0,     0,   322,   519,   520,    88,    89,    90,
      91,     0,   191,    92,   192,   193,   194,   195,     0,     0,
       0,   196,   197,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    93,    94,    95,    96,    97,    98,    99,   100,   101,
     102,   103,   198,     0,     0,     0,   323,     0,     0,     0,
       0,     0,   324,   200,   201,   202,   203,     0,     1,     2,
       3,     4,     5,   314,   315,   316,   325,   317,   318,   319,
     320,     0,     6,     7,     8,     9,    10,    11,    12,    13,
      14,    15,    16,    17,    18,    19,     0,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    71,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
       0,     0,     0,    83,    84,    85,    86,    87,   321,     0,
       0,     0,     0,     0,   322,     0,     0,    88,    89,    90,
      91,     0,   191,    92,   192,   193,   194,   195,     0,     0,
       0,   196,   197,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    93,    94,    95,    96,    97,    98,    99,   100,   101,
     102,   103,   198,     0,     0,     0,   323,   423,     0,     0,
       0,     0,   324,   200,   201,   202,   203,     0,     1,     2,
       3,     4,     5,   314,   315,   316,   325,   317,   318,   319,
     320,     0,     6,     7,     8,     9,    10,    11,    12,    13,
      14,    15,    16,    17,    18,    19,     0,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    71,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
       0,     0,     0,    83,    84,    85,    86,    87,   321,     0,
       0,     0,     0,     0,   322,     0,     0,    88,    89,    90,
      91,     0,   191,    92,   192,   193,   194,   195,     0,     0,
       0,   196,   197,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    93,    94,    95,    96,    97,    98,    99,   100,   101,
     102,   103,   198,     0,     0,     0,   323,   478,     0,     0,
       0,     0,   324,   200,   201,   202,   203,     0,     1,     2,
       3,     4,     5,   314,   315,   316,   325,   317,   318,   319,
     320,     0,     6,     7,     8,     9,    10,    11,    12,    13,
      14,    15,    16,    17,    18,    19,     0,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    71,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
       0,     0,     0,    83,    84,    85,    86,    87,   321,     0,
       0,     0,     0,     0,   322,     0,     0,    88,    89,    90,
      91,     0,   191,    92,   192,   193,   194,   195,     0,     0,
       0,   196,   197,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    93,    94,    95,    96,    97,    98,    99,   100,   101,
     102,   103,   198,     0,     0,     0,   323,     0,     0,     0,
       0,     0,   324,   200,   201,   202,   203,     0,     1,     2,
       3,     4,     5,   314,   315,   316,   325,   317,   318,   319,
     320,     0,     6,     7,     8,     9,    10,    11,    12,    13,
      14,    15,    16,    17,    18,    19,     0,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    71,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
       0,     0,     0,    83,    84,    85,    86,    87,   321,     0,
       0,     0,     0,     0,   322,     0,     0,    88,    89,    90,
      91,     0,   191,    92,   192,   193,   194,   195,     0,     0,
       0,   196,   197,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    93,    94,    95,    96,    97,    98,    99,   100,   101,
     102,   103,   198,     0,     0,     0,   178,     0,     0,     0,
       0,     0,   324,   200,   201,   202,   203,     1,     2,     3,
       4,     5,     0,     0,     0,     0,   325,     0,     0,     0,
       0,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,     0,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    82,     0,
       0,     0,    83,    84,    85,    86,    87,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    88,    89,    90,    91,
       0,   191,    92,   192,   193,   194,   195,     0,     0,     0,
     196,   197,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      93,    94,    95,    96,    97,    98,    99,   100,   101,   102,
     103,   198,     1,     2,     3,     4,     5,     0,     0,     0,
       0,   324,   200,   201,   202,   203,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
       0,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    73,    74,    75,    76,    77,    78,
      79,    80,    81,    82,     0,     0,     0,    83,    84,    85,
      86,    87,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    88,    89,    90,    91,     0,   191,    92,   192,   193,
     194,   195,     0,     0,     0,   196,   197,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    93,    94,    95,    96,     0,
      98,    99,   100,   101,   102,   103,   198,     0,     0,   199,
       1,     2,     3,     4,     5,     0,     0,   200,   201,   202,
     203,     0,     0,     0,     6,     7,     8,     9,    10,    11,
      12,    13,    14,    15,    16,    17,    18,    19,     0,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      81,    82,     0,     0,     0,    83,    84,    85,    86,    87,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    88,
      89,    90,    91,     0,   191,    92,   192,   193,   194,   195,
       0,     0,     0,   196,   197,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    93,    94,    95,    96,     0,    98,    99,
     100,   101,   102,   103,   198,     0,     0,   252,     1,     2,
       3,     4,     5,     0,     0,   200,   201,   202,   203,     0,
       0,     0,     6,     7,     8,     9,    10,    11,    12,    13,
      14,    15,    16,    17,    18,    19,     0,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    71,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
       0,     0,     0,    83,    84,    85,    86,    87,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    88,    89,    90,
      91,     0,   191,    92,   192,   193,   194,   195,     0,     0,
       0,   196,   197,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    93,    94,    95,    96,     0,    98,    99,   100,   101,
     102,   103,   198,     0,     0,   298,     1,     2,     3,     4,
       5,     0,     0,   200,   201,   202,   203,     0,     0,     0,
       6,     7,     8,     9,    10,    11,    12,    13,    14,    15,
      16,    17,    18,    19,     0,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,     0,     0,
       0,    83,    84,    85,    86,    87,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    88,    89,    90,    91,     0,
     191,    92,   192,   193,   194,   195,     0,     0,     0,   196,
     197,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    93,
      94,    95,    96,     0,    98,    99,   100,   101,   102,   103,
     198,     0,     0,   300,     1,     2,     3,     4,     5,     0,
       0,   200,   201,   202,   203,     0,     0,     0,     6,     7,
       8,     9,    10,    11,    12,    13,    14,    15,    16,    17,
      18,    19,     0,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    71,    72,    73,    74,    75,    76,
      77,    78,    79,    80,    81,    82,     0,     0,     0,    83,
      84,    85,    86,    87,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    88,    89,    90,    91,     0,   191,    92,
     192,   193,   194,   195,     0,     0,     0,   196,   197,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    93,    94,    95,
      96,     0,    98,    99,   100,   101,   102,   103,   198,     0,
       0,   342,     1,     2,     3,     4,     5,     0,     0,   200,
     201,   202,   203,     0,     0,     0,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
       0,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    73,    74,    75,    76,    77,    78,
      79,    80,    81,    82,     0,     0,     0,    83,    84,    85,
      86,    87,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    88,    89,    90,    91,     0,   191,    92,   192,   193,
     194,   195,     0,     0,     0,   196,   197,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    93,    94,    95,    96,     0,
      98,    99,   100,   101,   102,   103,   198,     0,     0,   344,
       1,     2,     3,     4,     5,     0,     0,   200,   201,   202,
     203,     0,     0,     0,     6,     7,     8,     9,    10,    11,
      12,    13,    14,    15,    16,    17,    18,    19,     0,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      81,    82,     0,     0,     0,    83,    84,    85,    86,    87,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    88,
      89,    90,    91,     0,   191,    92,   192,   193,   194,   195,
       0,     0,     0,   196,   197,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    93,    94,    95,    96,     0,    98,    99,
     100,   101,   102,   103,   198,     0,     0,   369,     1,     2,
       3,     4,     5,     0,     0,   200,   201,   202,   203,     0,
       0,     0,     6,     7,     8,     9,    10,    11,    12,    13,
      14,    15,    16,    17,    18,    19,     0,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    71,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
       0,     0,     0,    83,    84,    85,    86,    87,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    88,    89,    90,
      91,     0,   191,    92,   192,   193,   194,   195,     0,     0,
       0,   196,   197,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    93,    94,    95,    96,     0,    98,    99,   100,   101,
     102,   103,   198,     1,     2,     3,     4,     5,     0,     0,
       0,     0,   413,   200,   201,   202,   203,     6,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    17,    18,
      19,     0,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    71,    72,    73,    74,    75,    76,    77,
      78,    79,    80,    81,    82,     0,     0,     0,    83,    84,
      85,    86,    87,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    88,    89,    90,    91,     0,   191,    92,   192,
     193,   194,   195,     0,     0,     0,   196,   197,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    93,    94,    95,    96,
       0,    98,    99,   100,   101,   102,   103,   198,     0,     0,
     442,     1,     2,     3,     4,     5,     0,     0,   200,   201,
     202,   203,     0,     0,     0,     6,     7,     8,     9,    10,
      11,    12,    13,    14,    15,    16,    17,    18,    19,     0,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    72,    73,    74,    75,    76,    77,    78,    79,
      80,    81,    82,     0,     0,     0,    83,    84,    85,    86,
      87,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      88,    89,    90,    91,     0,   191,    92,   192,   193,   194,
     195,     0,     0,     0,   196,   197,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    93,    94,    95,    96,     0,    98,
      99,   100,   101,   102,   103,   198,     0,     0,   482,     1,
       2,     3,     4,     5,     0,     0,   200,   201,   202,   203,
       0,     0,     0,     6,     7,     8,     9,    10,    11,    12,
      13,    14,    15,    16,    17,    18,    19,     0,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,     0,     0,     0,    83,    84,    85,    86,    87,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    88,    89,
      90,    91,     0,   191,    92,   192,   193,   194,   195,     0,
       0,     0,   196,   197,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    93,    94,    95,    96,     0,    98,    99,   100,
     101,   102,   103,   198,     1,     2,     3,     4,     5,     0,
       0,     0,     0,     0,   200,   201,   202,   203,     6,     7,
       8,     9,    10,    11,    12,    13,    14,    15,    16,    17,
      18,    19,     0,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    71,    72,    73,    74,    75,    76,
      77,    78,    79,    80,    81,    82,     0,     0,     0,    83,
      84,    85,    86,   273,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    88,    89,    90,    91,     0,   191,    92,
     192,   193,   194,   195,     0,     0,     0,   196,   197,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    93,    94,    95,
      96,     0,    98,    99,   100,   101,   102,   103,   198,     1,
       2,     3,     4,     5,     0,     0,     0,     0,     0,   200,
     201,   202,   203,     6,     7,     8,     9,    10,    11,    12,
      13,    14,    15,    16,    17,    18,    19,     0,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,     0,     0,     0,    83,    84,    85,    86,    87,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    88,    89,
      90,    91,     0,   157,    92,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    93,    94,    95,    96,     0,    98,    99,   100,
     101,   102,   103,     1,     2,     3,     4,     5,     0,     0,
       0,     0,     0,   158,     0,     0,     0,     6,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    17,    18,
      19,     0,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    71,    72,    73,    74,    75,    76,    77,
      78,    79,    80,    81,    82,     0,     0,     0,    83,    84,
      85,    86,    87,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    88,    89,    90,    91,     0,     0,    92,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    93,    94,    95,    96,
       0,    98,    99,   100,   101,   102,   103,     3,     4,     5,
       0,     0,   435,     0,     0,     0,     0,     0,     0,     6,
       7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
      17,     0,     0,     0,     0,     0,     0,     0,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    70,    71,    72,    73,    74,    75,
      76,    77,    78,    79,    80,    81,    82,     0,     0,     0,
      83,     0,     0,    86,    87,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      92,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       3,     4,     5,     0,     0,     0,     0,     0,     0,    94,
      95,    96,     6,     7,     8,     9,    10,    11,    12,    13,
      14,    15,    16,    17,   308,     0,     0,     0,     0,     0,
       0,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    71,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
       0,     0,     0,    83,     0,     0,    86,    87,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    92,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     3,     4,     5,     0,     0,     0,     0,
       0,     0,    94,    95,    96,     6,     7,     8,     9,    10,
      11,    12,    13,    14,    15,    16,    17,   400,     0,     0,
       0,     0,     0,     0,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    72,    73,    74,    75,    76,    77,    78,    79,
      80,    81,    82,     0,     0,     0,    83,     0,     0,    86,
      87,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    92,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    94,    95,    96,   165,     0,
       0,     1,     2,     3,     4,     5,     0,     0,     0,     0,
     401,     0,     0,     0,     0,     6,     7,     8,     9,    10,
      11,    12,    13,    14,    15,    16,    17,    18,    19,     0,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    72,    73,    74,    75,    76,    77,    78,    79,
      80,    81,    82,     0,     0,     0,    83,    84,    85,    86,
      87,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      88,    89,    90,    91,     0,     0,    92,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    93,    94,    95,    96,    97,    98,
      99,   100,   101,   102,   103,     1,     2,     3,     4,     5,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     6,
       7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
      17,    18,    19,     0,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    70,    71,    72,    73,    74,    75,
      76,    77,    78,    79,    80,    81,    82,     0,     0,     0,
      83,    84,    85,    86,    87,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    88,    89,    90,    91,     0,     0,
      92,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,     1,
       2,     3,     4,     5,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     6,     7,     8,     9,    10,    11,    12,
      13,    14,    15,    16,    17,    18,    19,     0,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,     0,     0,     0,    83,    84,    85,    86,    87,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    88,    89,
      90,    91,     0,     0,    92,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    93,    94,    95,    96,     0,    98,    99,   100,
     101,   102,   103,   140,     3,     4,     5,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,   141,   142,
     143,     0,     0,     0,     0,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    73,    74,    75,    76,    77,    78,
      79,    80,    81,    82,     0,     0,     0,    83,     0,     0,
      86,    87,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    92,     0,     0,
       3,     4,     5,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     6,     7,     8,     9,    10,    11,    12,    13,
      14,    15,    16,    17,     0,     0,    94,    95,    96,     0,
     144,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    71,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
       0,     0,     0,    83,     0,     0,    86,    87,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    92,     3,     4,     5,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,     0,     0,
       0,     0,    94,    95,    96,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    73,    74,    75,    76,    77,    78,
      79,    80,    81,    82,   174,   175,     0,    83,     0,     0,
      86,    87,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    92,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    72,    73,    74,    75,    76,    77,    78,    79,
      80,    81,    82,     0,     0,     0,    83,     0,     0,    86,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    92
};

static const short yycheck[] =
{
       0,     0,   161,   156,   112,   117,     1,   187,   237,   316,
     250,   133,   161,   128,   129,   117,   118,   117,   258,   123,
      29,    30,   164,     0,     0,   164,   198,   159,   187,   491,
     337,   184,   185,   188,   176,   167,   157,   176,   187,   501,
     167,   145,   126,   127,   157,   159,   159,   196,   197,   158,
     230,   231,   166,   166,   164,   167,   211,   157,   180,   161,
     175,   176,   169,   159,   171,   214,   117,   247,   159,   249,
     166,   230,   231,   302,   303,   159,   159,   159,   117,   163,
     159,   230,   231,   166,   166,   265,   159,   166,   247,   120,
     249,   122,   159,   166,   164,   250,   268,   167,   247,   166,
     249,   124,   125,   258,   158,   158,   265,   347,   348,   158,
     164,   164,   158,   158,   161,   164,   265,   272,   164,   164,
     164,   229,   117,   167,   296,   161,   126,   126,   277,   278,
     279,   280,   281,   282,   283,   284,   285,   286,   287,   288,
     289,   290,   291,   292,   293,   294,   295,   454,   320,   126,
     126,   304,    97,   135,   136,   137,   138,   139,   140,   141,
     142,   143,   144,   108,   284,   285,   286,   287,   164,   117,
     164,   167,   164,   167,   164,   167,   159,   167,   147,   148,
     149,   161,   489,   161,   166,   158,   426,   172,   173,   174,
     430,   166,   347,   348,   139,   130,   131,   108,   164,   165,
     164,   112,   509,   162,   149,   167,   361,   159,   280,   281,
       1,   282,   283,   368,   157,   522,   396,   233,   234,   526,
     460,   159,   402,   157,   169,   288,   289,   158,   139,   179,
     132,   178,   160,   177,   134,   117,   117,   396,   149,   411,
     158,   160,     1,   402,   416,   245,   245,   396,   158,   162,
     159,   167,   160,   402,   160,   159,   157,   160,   169,   259,
     167,   157,   167,   157,   504,   160,   421,   117,   167,   162,
     160,   426,   160,   164,   103,   430,   117,   157,   167,   432,
     452,   157,   462,   167,   159,   157,   441,   160,   233,   234,
     160,   158,   237,   117,   158,   182,   182,   469,   161,   117,
     472,   160,   167,   462,   158,   460,   166,   162,   165,   167,
      11,   117,   165,   462,   165,   416,   316,   316,   229,   480,
     511,   291,   233,   234,   292,   236,   237,   290,   500,   294,
     149,   139,   293,   149,   149,   295,   135,   337,   337,   519,
     307,   464,   404,   241,   364,   501,   449,   137,   522,   504,
     418,   449,   126,   472,    -1,    -1,    -1,   302,   303,    -1,
     519,    -1,    -1,    -1,   364,    -1,    -1,    -1,    -1,    -1,
     519,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   302,   303,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   418,   418,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   449,
     449,    -1,   452,    -1,   454,   454,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   472,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   489,
     489,   491,   491,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   501,   501,    -1,    -1,    -1,    -1,    -1,    -1,   509,
     509,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   522,   522,    -1,    -1,   526,   526,     3,     4,
       5,     6,     7,     8,     9,    10,    -1,    12,    13,    14,
      15,    -1,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    -1,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      -1,    -1,    -1,    98,    99,   100,   101,   102,   103,    -1,
      -1,    -1,    -1,    -1,   109,   110,   111,   112,   113,   114,
     115,    -1,   117,   118,   119,   120,   121,   122,    -1,    -1,
      -1,   126,   127,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   146,   147,   148,   149,   150,   151,   152,   153,   154,
     155,   156,   157,    -1,    -1,    -1,   161,   162,    -1,    -1,
      -1,    -1,   167,   168,   169,   170,   171,    -1,     3,     4,
       5,     6,     7,     8,     9,    10,   181,    12,    13,    14,
      15,    -1,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    -1,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      -1,    -1,    -1,    98,    99,   100,   101,   102,   103,    -1,
      -1,    -1,    -1,    -1,   109,   110,   111,   112,   113,   114,
     115,    -1,   117,   118,   119,   120,   121,   122,    -1,    -1,
      -1,   126,   127,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   146,   147,   148,   149,   150,   151,   152,   153,   154,
     155,   156,   157,    -1,    -1,    -1,   161,    -1,    -1,    -1,
      -1,    -1,   167,   168,   169,   170,   171,    -1,     3,     4,
       5,     6,     7,     8,     9,    10,   181,    12,    13,    14,
      15,    -1,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    -1,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      -1,    -1,    -1,    98,    99,   100,   101,   102,   103,    -1,
      -1,    -1,    -1,    -1,   109,    -1,    -1,   112,   113,   114,
     115,    -1,   117,   118,   119,   120,   121,   122,    -1,    -1,
      -1,   126,   127,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   146,   147,   148,   149,   150,   151,   152,   153,   154,
     155,   156,   157,    -1,    -1,    -1,   161,   162,    -1,    -1,
      -1,    -1,   167,   168,   169,   170,   171,    -1,     3,     4,
       5,     6,     7,     8,     9,    10,   181,    12,    13,    14,
      15,    -1,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    -1,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      -1,    -1,    -1,    98,    99,   100,   101,   102,   103,    -1,
      -1,    -1,    -1,    -1,   109,    -1,    -1,   112,   113,   114,
     115,    -1,   117,   118,   119,   120,   121,   122,    -1,    -1,
      -1,   126,   127,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   146,   147,   148,   149,   150,   151,   152,   153,   154,
     155,   156,   157,    -1,    -1,    -1,   161,   162,    -1,    -1,
      -1,    -1,   167,   168,   169,   170,   171,    -1,     3,     4,
       5,     6,     7,     8,     9,    10,   181,    12,    13,    14,
      15,    -1,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    -1,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      -1,    -1,    -1,    98,    99,   100,   101,   102,   103,    -1,
      -1,    -1,    -1,    -1,   109,    -1,    -1,   112,   113,   114,
     115,    -1,   117,   118,   119,   120,   121,   122,    -1,    -1,
      -1,   126,   127,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   146,   147,   148,   149,   150,   151,   152,   153,   154,
     155,   156,   157,    -1,    -1,    -1,   161,    -1,    -1,    -1,
      -1,    -1,   167,   168,   169,   170,   171,    -1,     3,     4,
       5,     6,     7,     8,     9,    10,   181,    12,    13,    14,
      15,    -1,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    -1,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      -1,    -1,    -1,    98,    99,   100,   101,   102,   103,    -1,
      -1,    -1,    -1,    -1,   109,    -1,    -1,   112,   113,   114,
     115,    -1,   117,   118,   119,   120,   121,   122,    -1,    -1,
      -1,   126,   127,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   146,   147,   148,   149,   150,   151,   152,   153,   154,
     155,   156,   157,    -1,    -1,    -1,   161,    -1,    -1,    -1,
      -1,    -1,   167,   168,   169,   170,   171,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,   181,    -1,    -1,    -1,
      -1,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    -1,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    70,    71,    72,    73,    74,    75,
      76,    77,    78,    79,    80,    81,    82,    83,    84,    85,
      86,    87,    88,    89,    90,    91,    92,    93,    94,    -1,
      -1,    -1,    98,    99,   100,   101,   102,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   112,   113,   114,   115,
      -1,   117,   118,   119,   120,   121,   122,    -1,    -1,    -1,
     126,   127,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     146,   147,   148,   149,   150,   151,   152,   153,   154,   155,
     156,   157,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,   167,   168,   169,   170,   171,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      -1,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      81,    82,    83,    84,    85,    86,    87,    88,    89,    90,
      91,    92,    93,    94,    -1,    -1,    -1,    98,    99,   100,
     101,   102,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   112,   113,   114,   115,    -1,   117,   118,   119,   120,
     121,   122,    -1,    -1,    -1,   126,   127,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   146,   147,   148,   149,    -1,
     151,   152,   153,   154,   155,   156,   157,    -1,    -1,   160,
       3,     4,     5,     6,     7,    -1,    -1,   168,   169,   170,
     171,    -1,    -1,    -1,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    -1,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    71,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    85,    86,    87,    88,    89,    90,    91,    92,
      93,    94,    -1,    -1,    -1,    98,    99,   100,   101,   102,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   112,
     113,   114,   115,    -1,   117,   118,   119,   120,   121,   122,
      -1,    -1,    -1,   126,   127,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   146,   147,   148,   149,    -1,   151,   152,
     153,   154,   155,   156,   157,    -1,    -1,   160,     3,     4,
       5,     6,     7,    -1,    -1,   168,   169,   170,   171,    -1,
      -1,    -1,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    -1,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      -1,    -1,    -1,    98,    99,   100,   101,   102,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   112,   113,   114,
     115,    -1,   117,   118,   119,   120,   121,   122,    -1,    -1,
      -1,   126,   127,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   146,   147,   148,   149,    -1,   151,   152,   153,   154,
     155,   156,   157,    -1,    -1,   160,     3,     4,     5,     6,
       7,    -1,    -1,   168,   169,   170,   171,    -1,    -1,    -1,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    -1,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    71,    72,    73,    74,    75,    76,
      77,    78,    79,    80,    81,    82,    83,    84,    85,    86,
      87,    88,    89,    90,    91,    92,    93,    94,    -1,    -1,
      -1,    98,    99,   100,   101,   102,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   112,   113,   114,   115,    -1,
     117,   118,   119,   120,   121,   122,    -1,    -1,    -1,   126,
     127,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   146,
     147,   148,   149,    -1,   151,   152,   153,   154,   155,   156,
     157,    -1,    -1,   160,     3,     4,     5,     6,     7,    -1,
      -1,   168,   169,   170,   171,    -1,    -1,    -1,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    -1,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    73,    74,    75,    76,    77,    78,
      79,    80,    81,    82,    83,    84,    85,    86,    87,    88,
      89,    90,    91,    92,    93,    94,    -1,    -1,    -1,    98,
      99,   100,   101,   102,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   112,   113,   114,   115,    -1,   117,   118,
     119,   120,   121,   122,    -1,    -1,    -1,   126,   127,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   146,   147,   148,
     149,    -1,   151,   152,   153,   154,   155,   156,   157,    -1,
      -1,   160,     3,     4,     5,     6,     7,    -1,    -1,   168,
     169,   170,   171,    -1,    -1,    -1,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      -1,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      81,    82,    83,    84,    85,    86,    87,    88,    89,    90,
      91,    92,    93,    94,    -1,    -1,    -1,    98,    99,   100,
     101,   102,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   112,   113,   114,   115,    -1,   117,   118,   119,   120,
     121,   122,    -1,    -1,    -1,   126,   127,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   146,   147,   148,   149,    -1,
     151,   152,   153,   154,   155,   156,   157,    -1,    -1,   160,
       3,     4,     5,     6,     7,    -1,    -1,   168,   169,   170,
     171,    -1,    -1,    -1,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    -1,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    71,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    85,    86,    87,    88,    89,    90,    91,    92,
      93,    94,    -1,    -1,    -1,    98,    99,   100,   101,   102,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   112,
     113,   114,   115,    -1,   117,   118,   119,   120,   121,   122,
      -1,    -1,    -1,   126,   127,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   146,   147,   148,   149,    -1,   151,   152,
     153,   154,   155,   156,   157,    -1,    -1,   160,     3,     4,
       5,     6,     7,    -1,    -1,   168,   169,   170,   171,    -1,
      -1,    -1,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    -1,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      -1,    -1,    -1,    98,    99,   100,   101,   102,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   112,   113,   114,
     115,    -1,   117,   118,   119,   120,   121,   122,    -1,    -1,
      -1,   126,   127,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   146,   147,   148,   149,    -1,   151,   152,   153,   154,
     155,   156,   157,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,   167,   168,   169,   170,   171,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    -1,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    72,    73,    74,    75,    76,    77,    78,    79,
      80,    81,    82,    83,    84,    85,    86,    87,    88,    89,
      90,    91,    92,    93,    94,    -1,    -1,    -1,    98,    99,
     100,   101,   102,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   112,   113,   114,   115,    -1,   117,   118,   119,
     120,   121,   122,    -1,    -1,    -1,   126,   127,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   146,   147,   148,   149,
      -1,   151,   152,   153,   154,   155,   156,   157,    -1,    -1,
     160,     3,     4,     5,     6,     7,    -1,    -1,   168,   169,
     170,   171,    -1,    -1,    -1,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    -1,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    94,    -1,    -1,    -1,    98,    99,   100,   101,
     102,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     112,   113,   114,   115,    -1,   117,   118,   119,   120,   121,
     122,    -1,    -1,    -1,   126,   127,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   146,   147,   148,   149,    -1,   151,
     152,   153,   154,   155,   156,   157,    -1,    -1,   160,     3,
       4,     5,     6,     7,    -1,    -1,   168,   169,   170,   171,
      -1,    -1,    -1,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    -1,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    83,
      84,    85,    86,    87,    88,    89,    90,    91,    92,    93,
      94,    -1,    -1,    -1,    98,    99,   100,   101,   102,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   112,   113,
     114,   115,    -1,   117,   118,   119,   120,   121,   122,    -1,
      -1,    -1,   126,   127,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   146,   147,   148,   149,    -1,   151,   152,   153,
     154,   155,   156,   157,     3,     4,     5,     6,     7,    -1,
      -1,    -1,    -1,    -1,   168,   169,   170,   171,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    -1,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    73,    74,    75,    76,    77,    78,
      79,    80,    81,    82,    83,    84,    85,    86,    87,    88,
      89,    90,    91,    92,    93,    94,    -1,    -1,    -1,    98,
      99,   100,   101,   102,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   112,   113,   114,   115,    -1,   117,   118,
     119,   120,   121,   122,    -1,    -1,    -1,   126,   127,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   146,   147,   148,
     149,    -1,   151,   152,   153,   154,   155,   156,   157,     3,
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,   168,
     169,   170,   171,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    -1,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    83,
      84,    85,    86,    87,    88,    89,    90,    91,    92,    93,
      94,    -1,    -1,    -1,    98,    99,   100,   101,   102,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   112,   113,
     114,   115,    -1,   117,   118,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   146,   147,   148,   149,    -1,   151,   152,   153,
     154,   155,   156,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,   167,    -1,    -1,    -1,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    -1,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    72,    73,    74,    75,    76,    77,    78,    79,
      80,    81,    82,    83,    84,    85,    86,    87,    88,    89,
      90,    91,    92,    93,    94,    -1,    -1,    -1,    98,    99,
     100,   101,   102,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   112,   113,   114,   115,    -1,    -1,   118,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   146,   147,   148,   149,
      -1,   151,   152,   153,   154,   155,   156,     5,     6,     7,
      -1,    -1,   162,    -1,    -1,    -1,    -1,    -1,    -1,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    71,    72,    73,    74,    75,    76,    77,
      78,    79,    80,    81,    82,    83,    84,    85,    86,    87,
      88,    89,    90,    91,    92,    93,    94,    -1,    -1,    -1,
      98,    -1,    -1,   101,   102,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     118,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
       5,     6,     7,    -1,    -1,    -1,    -1,    -1,    -1,   147,
     148,   149,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,   162,    -1,    -1,    -1,    -1,    -1,
      -1,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      -1,    -1,    -1,    98,    -1,    -1,   101,   102,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   118,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    -1,   147,   148,   149,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,   162,    -1,    -1,
      -1,    -1,    -1,    -1,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    94,    -1,    -1,    -1,    98,    -1,    -1,   101,
     102,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   118,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   147,   148,   149,     0,    -1,
      -1,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
     162,    -1,    -1,    -1,    -1,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    -1,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    94,    -1,    -1,    -1,    98,    99,   100,   101,
     102,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     112,   113,   114,   115,    -1,    -1,   118,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   146,   147,   148,   149,   150,   151,
     152,   153,   154,   155,   156,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    -1,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    71,    72,    73,    74,    75,    76,    77,
      78,    79,    80,    81,    82,    83,    84,    85,    86,    87,
      88,    89,    90,    91,    92,    93,    94,    -1,    -1,    -1,
      98,    99,   100,   101,   102,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   112,   113,   114,   115,    -1,    -1,
     118,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   146,   147,
     148,   149,   150,   151,   152,   153,   154,   155,   156,     3,
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    -1,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    83,
      84,    85,    86,    87,    88,    89,    90,    91,    92,    93,
      94,    -1,    -1,    -1,    98,    99,   100,   101,   102,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   112,   113,
     114,   115,    -1,    -1,   118,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   146,   147,   148,   149,    -1,   151,   152,   153,
     154,   155,   156,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    -1,    -1,    -1,    -1,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      81,    82,    83,    84,    85,    86,    87,    88,    89,    90,
      91,    92,    93,    94,    -1,    -1,    -1,    98,    -1,    -1,
     101,   102,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   118,    -1,    -1,
       5,     6,     7,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    -1,    -1,   147,   148,   149,    -1,
     151,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      -1,    -1,    -1,    98,    -1,    -1,   101,   102,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   118,     5,     6,     7,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    -1,    -1,
      -1,    -1,   147,   148,   149,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      81,    82,    83,    84,    85,    86,    87,    88,    89,    90,
      91,    92,    93,    94,     6,     7,    -1,    98,    -1,    -1,
     101,   102,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   118,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    94,    -1,    -1,    -1,    98,    -1,    -1,   101,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   118
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const unsigned short yystos[] =
{
       0,     3,     4,     5,     6,     7,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    94,    98,    99,   100,   101,   102,   112,   113,
     114,   115,   118,   146,   147,   148,   149,   150,   151,   152,
     153,   154,   155,   156,   219,   220,   221,   222,   223,   230,
     231,   232,   233,   234,   235,   236,   241,   242,   243,   244,
     245,   247,   248,   249,   257,   258,   292,   293,   294,    29,
      30,   117,   118,   161,   157,   247,   167,   295,   158,   164,
       4,    29,    30,    31,   151,   224,   225,   226,   227,   228,
     229,   240,   243,   247,   164,   167,   117,   117,   167,   234,
     243,   159,   218,   117,   167,     0,   293,   161,   161,   251,
       1,   117,   238,   239,     6,     7,   244,   246,   161,   272,
     225,   224,   227,   229,   117,   117,   157,   159,   166,   218,
     161,   117,   119,   120,   121,   122,   126,   127,   157,   160,
     168,   169,   170,   171,   184,   185,   186,   194,   195,   196,
     197,   198,   199,   200,   201,   202,   203,   204,   205,   206,
     207,   208,   209,   210,   211,   212,   213,   217,   232,   233,
     159,   159,   167,   250,   252,   243,   247,   253,   254,   158,
     166,   164,   237,   167,   162,   273,   296,   159,   218,   159,
     166,   218,   160,   217,   200,   213,   214,   265,   166,   259,
     260,   200,   200,   214,   216,   159,   126,   127,   159,   163,
     158,   158,   164,   102,   214,   157,   200,   172,   173,   174,
     169,   171,   124,   125,   128,   129,   175,   176,   130,   131,
     179,   178,   177,   132,   134,   133,   180,   160,   160,   217,
     160,   217,   253,   253,   117,   255,   256,   243,   162,   254,
     120,   122,   239,   158,     8,     9,    10,    12,    13,    14,
      15,   103,   109,   161,   167,   181,   188,   189,   193,   216,
     219,   220,   232,   266,   267,   268,   269,   274,   275,   276,
     285,   291,   160,   217,   160,   217,   265,   166,   166,   160,
     135,   136,   137,   138,   139,   140,   141,   142,   143,   144,
     166,   215,   265,   232,   261,   263,     1,   158,   164,   160,
     217,   187,   216,   123,   145,   214,   200,   200,   200,   202,
     202,   203,   203,   204,   204,   204,   204,   205,   205,   206,
     207,   208,   209,   210,   211,   216,   159,   160,   167,   160,
     162,   162,   159,   218,   164,   167,   255,   167,   167,   267,
     287,   157,   167,   167,   216,   286,   157,   162,   270,   157,
     158,   164,   167,   162,   267,   160,   166,   160,   265,   265,
     166,   214,   117,   262,   264,   162,   263,   162,   214,   160,
     160,   165,   160,   217,   167,   217,   256,   167,   103,   157,
     216,   167,   157,   187,   274,   117,   167,   190,   214,   265,
     166,   265,   159,   218,   164,   167,   214,   160,   160,   157,
     266,   275,   288,   158,   216,   232,   284,   158,   162,   182,
     182,   265,   160,   217,   264,   216,   284,   289,   290,   277,
     117,   158,   161,   280,   117,   191,   192,   191,   160,   158,
     167,   158,   267,   282,   166,   268,   271,   272,   162,   281,
     165,   164,   176,   176,   167,   216,   271,    11,   265,   110,
     111,   267,   278,   279,   117,   192,   283,   217,   165,   162,
     279,   267,   165
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

#define yyerrok     (yyerrstatus = 0)
#define yyclearin   (yychar = YYEMPTY)
#define YYEMPTY     (-2)
#define YYEOF       0

#define YYACCEPT    goto yyacceptlab
#define YYABORT     goto yyabortlab
#define YYERROR     goto yyerrorlab


/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */

#define YYFAIL      goto yyerrlab

#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                  \
do                              \
  if (yychar == YYEMPTY && yylen == 1)              \
    {                               \
      yychar = (Token);                     \
      yylval = (Value);                     \
      yytoken = YYTRANSLATE (yychar);               \
      YYPOPSTACK;                       \
      goto yybackup;                        \
    }                               \
  else                              \
    {                               \
      yyerror ("syntax error: cannot back up");\
      YYERROR;                          \
    }                               \
while (0)

#define YYTERROR    1
#define YYERRCODE   256

/* YYLLOC_DEFAULT -- Compute the default location (before the actions
   are run).  */

#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)        \
   ((Current).first_line   = (Rhs)[1].first_line,   \
    (Current).first_column = (Rhs)[1].first_column, \
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
  if (yydebug)                  \
    YYFPRINTF Args;             \
} while (0)

# define YYDSYMPRINT(Args)          \
do {                        \
  if (yydebug)                  \
    yysymprint Args;                \
} while (0)

# define YYDSYMPRINTF(Title, Token, Value, Location)        \
do {                                \
  if (yydebug)                          \
    {                               \
      YYFPRINTF (stderr, "%s ", Title);             \
      yysymprint (stderr,                   \
                  Token, Value);    \
      YYFPRINTF (stderr, "\n");                 \
    }                               \
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
  if (yydebug)                          \
    yy_stack_print ((Bottom), (Top));               \
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

# define YY_REDUCE_PRINT(Rule)      \
do {                    \
  if (yydebug)              \
    yy_reduce_print (Rule);     \
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
  short yyssa[YYINITDEPTH];
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
  yychar = YYEMPTY;     /* Cause a token to be read.  */

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

    { yyval.expr = slParseVariableIdentifier(Compiler, &yyvsp[0].token); ;}
    break;

  case 3:

    { yyval.expr = yyvsp[0].expr; ;}
    break;

  case 4:

    { yyval.expr = slParseIntConstant(Compiler, &yyvsp[0].token); ;}
    break;

  case 5:

    { yyval.expr = slParseUintConstant(Compiler, &yyvsp[0].token); ;}
    break;

  case 6:

    { yyval.expr = slParseFloatConstant(Compiler, &yyvsp[0].token); ;}
    break;

  case 7:

    { yyval.expr = slParseBoolConstant(Compiler, &yyvsp[0].token); ;}
    break;

  case 8:

    { yyval.expr = yyvsp[-1].expr; ;}
    break;

  case 9:

    { yyval.expr = yyvsp[0].expr; ;}
    break;

  case 10:

    { yyval.expr = slParseSubscriptExpr(Compiler, yyvsp[-3].expr, yyvsp[-1].expr); ;}
    break;

  case 11:

    { yyval.expr = slParseFuncCallExprAsExpr(Compiler, yyvsp[0].funcCall); ;}
    break;

  case 12:

    { yyval.expr = slParseFieldSelectionExpr(Compiler, yyvsp[-2].expr, &yyvsp[0].token); ;}
    break;

  case 13:

    { yyval.expr = slParseLengthMethodExpr(Compiler, yyvsp[-2].expr); ;}
    break;

  case 14:

    { yyval.expr = slParseIncOrDecExpr(Compiler, gcvNULL, slvUNARY_POST_INC, yyvsp[-1].expr); ;}
    break;

  case 15:

    { yyval.expr = slParseIncOrDecExpr(Compiler, gcvNULL, slvUNARY_POST_DEC, yyvsp[-1].expr); ;}
    break;

  case 16:

    { yyval.expr = yyvsp[0].expr; ;}
    break;

  case 17:

    { yyval.statement = slParseAsmAsStatement(Compiler, yyvsp[-2].vivAsm); ;}
    break;

  case 18:

    { yyval.vivAsm = slParseAsmOpcode(Compiler, &yyvsp[0].asmOpcode); ;}
    break;

  case 19:

    { yyval.vivAsm = slParseAsmOperand(Compiler, yyvsp[-2].vivAsm, yyvsp[0].expr); ;}
    break;

  case 20:

    { yyval.expr = yyvsp[0].expr; ;}
    break;

  case 21:

    { yyval.expr = slParseAsmAppendOperandModifiers(Compiler, yyvsp[-3].expr, &yyvsp[-1].asmModifiers); ;}
    break;

  case 22:

    { yyval.asmModifiers = slParseAsmAppendModifier(Compiler, gcvNULL, &yyvsp[0].asmModifier); ;}
    break;

  case 23:

    { yyval.asmModifiers = slParseAsmAppendModifier(Compiler, &yyvsp[-2].asmModifiers, &yyvsp[0].asmModifier); ;}
    break;

  case 24:

    { yyval.asmModifier = slParseAsmModifier(Compiler, &yyvsp[-2].token, &yyvsp[0].token); ;}
    break;

  case 25:

    { yyval.asmOpcode = slParseAsmCreateOpcode(Compiler, &yyvsp[0].token, gcvNULL); ;}
    break;

  case 26:

    { yyval.asmOpcode = slParseAsmCreateOpcode(Compiler, &yyvsp[-3].token, &yyvsp[-1].asmModifiers); ;}
    break;

  case 27:

    { yyval.funcCall = yyvsp[0].funcCall; ;}
    break;

  case 28:

    { yyval.funcCall = yyvsp[-1].funcCall; ;}
    break;

  case 29:

    { yyval.funcCall = yyvsp[-1].funcCall; ;}
    break;

  case 30:

    { yyval.funcCall = yyvsp[-1].funcCall; ;}
    break;

  case 31:

    { yyval.funcCall = yyvsp[0].funcCall; ;}
    break;

  case 32:

    { yyval.funcCall = slParseFuncCallArgument(Compiler, yyvsp[-1].funcCall, yyvsp[0].expr); ;}
    break;

  case 33:

    { yyval.funcCall = slParseFuncCallArgument(Compiler, yyvsp[-2].funcCall, yyvsp[0].expr); ;}
    break;

  case 34:

    { yyval.funcCall = slParseFuncCallHeaderExpr(Compiler, &yyvsp[-1].token); ;}
    break;

  case 35:

    { yyval.token = slParseBasicType(Compiler, yyvsp[0].dataType); ;}
    break;

  case 36:

    { yyval.token = yyvsp[0].token; ;}
    break;

  case 37:

    { yyval.expr = yyvsp[0].expr; ;}
    break;

  case 38:

    { yyval.expr = slParseIncOrDecExpr(Compiler, &yyvsp[-1].token, slvUNARY_PRE_INC, yyvsp[0].expr); ;}
    break;

  case 39:

    { yyval.expr = slParseIncOrDecExpr(Compiler, &yyvsp[-1].token, slvUNARY_PRE_DEC, yyvsp[0].expr); ;}
    break;

  case 40:

    { yyval.expr = slParseNormalUnaryExpr(Compiler, &yyvsp[-1].token, yyvsp[0].expr); ;}
    break;

  case 41:

    { yyval.token = yyvsp[0].token; ;}
    break;

  case 42:

    { yyval.token = yyvsp[0].token; ;}
    break;

  case 43:

    { yyval.token = yyvsp[0].token; ;}
    break;

  case 44:

    { yyval.token = yyvsp[0].token; ;}
    break;

  case 45:

    { yyval.expr = yyvsp[0].expr; ;}
    break;

  case 46:

    { yyval.expr = slParseNormalBinaryExpr(Compiler, yyvsp[-2].expr, &yyvsp[-1].token, yyvsp[0].expr); ;}
    break;

  case 47:

    { yyval.expr = slParseNormalBinaryExpr(Compiler, yyvsp[-2].expr, &yyvsp[-1].token, yyvsp[0].expr); ;}
    break;

  case 48:

    { yyval.expr = slParseNormalBinaryExpr(Compiler, yyvsp[-2].expr, &yyvsp[-1].token, yyvsp[0].expr); ;}
    break;

  case 49:

    { yyval.expr = yyvsp[0].expr; ;}
    break;

  case 50:

    { yyval.expr = slParseNormalBinaryExpr(Compiler, yyvsp[-2].expr, &yyvsp[-1].token, yyvsp[0].expr); ;}
    break;

  case 51:

    { yyval.expr = slParseNormalBinaryExpr(Compiler, yyvsp[-2].expr, &yyvsp[-1].token, yyvsp[0].expr); ;}
    break;

  case 52:

    { yyval.expr = yyvsp[0].expr; ;}
    break;

  case 53:

    { yyval.expr = slParseNormalBinaryExpr(Compiler, yyvsp[-2].expr, &yyvsp[-1].token, yyvsp[0].expr); ;}
    break;

  case 54:

    { yyval.expr = slParseNormalBinaryExpr(Compiler, yyvsp[-2].expr, &yyvsp[-1].token, yyvsp[0].expr); ;}
    break;

  case 55:

    { yyval.expr = yyvsp[0].expr; ;}
    break;

  case 56:

    { yyval.expr = slParseNormalBinaryExpr(Compiler, yyvsp[-2].expr, &yyvsp[-1].token, yyvsp[0].expr); ;}
    break;

  case 57:

    { yyval.expr = slParseNormalBinaryExpr(Compiler, yyvsp[-2].expr, &yyvsp[-1].token, yyvsp[0].expr); ;}
    break;

  case 58:

    { yyval.expr = slParseNormalBinaryExpr(Compiler, yyvsp[-2].expr, &yyvsp[-1].token, yyvsp[0].expr); ;}
    break;

  case 59:

    { yyval.expr = slParseNormalBinaryExpr(Compiler, yyvsp[-2].expr, &yyvsp[-1].token, yyvsp[0].expr); ;}
    break;

  case 60:

    { yyval.expr = yyvsp[0].expr; ;}
    break;

  case 61:

    { yyval.expr = slParseNormalBinaryExpr(Compiler, yyvsp[-2].expr, &yyvsp[-1].token, yyvsp[0].expr); ;}
    break;

  case 62:

    { yyval.expr = slParseNormalBinaryExpr(Compiler, yyvsp[-2].expr, &yyvsp[-1].token, yyvsp[0].expr); ;}
    break;

  case 63:

    { yyval.expr = yyvsp[0].expr; ;}
    break;

  case 64:

    { yyval.expr = slParseNormalBinaryExpr(Compiler, yyvsp[-2].expr, &yyvsp[-1].token, yyvsp[0].expr); ;}
    break;

  case 65:

    { yyval.expr = yyvsp[0].expr; ;}
    break;

  case 66:

    { yyval.expr = slParseNormalBinaryExpr(Compiler, yyvsp[-2].expr, &yyvsp[-1].token, yyvsp[0].expr); ;}
    break;

  case 67:

    { yyval.expr = yyvsp[0].expr; ;}
    break;

  case 68:

    { yyval.expr = slParseNormalBinaryExpr(Compiler, yyvsp[-2].expr, &yyvsp[-1].token, yyvsp[0].expr); ;}
    break;

  case 69:

    { yyval.expr = yyvsp[0].expr; ;}
    break;

  case 70:

    { yyval.expr = slParseNormalBinaryExpr(Compiler, yyvsp[-2].expr, &yyvsp[-1].token, yyvsp[0].expr); ;}
    break;

  case 71:

    { yyval.expr = yyvsp[0].expr; ;}
    break;

  case 72:

    { yyval.expr = slParseNormalBinaryExpr(Compiler, yyvsp[-2].expr, &yyvsp[-1].token, yyvsp[0].expr); ;}
    break;

  case 73:

    { yyval.expr = yyvsp[0].expr; ;}
    break;

  case 74:

    { yyval.expr = slParseNormalBinaryExpr(Compiler, yyvsp[-2].expr, &yyvsp[-1].token, yyvsp[0].expr); ;}
    break;

  case 75:

    { yyval.expr = yyvsp[0].expr; ;}
    break;

  case 76:

    { yyval.expr = slParseSelectionExpr(Compiler, yyvsp[-4].expr, yyvsp[-2].expr, yyvsp[0].expr); ;}
    break;

  case 77:

    { yyval.expr = yyvsp[0].expr; ;}
    break;

  case 78:

    { yyval.expr = slParseAssignmentExpr(Compiler, yyvsp[-2].expr, &yyvsp[-1].token, yyvsp[0].expr); ;}
    break;

  case 79:

    { yyval.token = yyvsp[0].token; ;}
    break;

  case 80:

    { yyval.token = yyvsp[0].token; ;}
    break;

  case 81:

    { yyval.token = yyvsp[0].token; ;}
    break;

  case 82:

    { yyval.token = yyvsp[0].token; ;}
    break;

  case 83:

    { yyval.token = yyvsp[0].token; ;}
    break;

  case 84:

    { yyval.token = yyvsp[0].token; ;}
    break;

  case 85:

    { yyval.token = yyvsp[0].token; ;}
    break;

  case 86:

    { yyval.token = yyvsp[0].token; ;}
    break;

  case 87:

    { yyval.token = yyvsp[0].token; ;}
    break;

  case 88:

    { yyval.token = yyvsp[0].token; ;}
    break;

  case 89:

    { yyval.token = yyvsp[0].token; ;}
    break;

  case 90:

    { yyval.expr = yyvsp[0].expr; ;}
    break;

  case 91:

    { yyval.expr = slParseNormalBinaryExpr(Compiler, yyvsp[-2].expr, &yyvsp[-1].token, yyvsp[0].expr); ;}
    break;

  case 92:

    { yyval.expr = yyvsp[0].expr; ;}
    break;

  case 93:

    { yyval.fieldDeclList = slParseArrayLengthList(Compiler, gcvNULL, gcvNULL, gcvNULL); ;}
    break;

  case 94:

    { yyval.fieldDeclList = slParseArrayLengthList(Compiler, gcvNULL, gcvNULL, yyvsp[-1].expr); ;}
    break;

  case 95:

    { yyval.fieldDeclList = slParseArrayLengthList(Compiler, gcvNULL, yyvsp[-3].expr, gcvNULL); ;}
    break;

  case 96:

    { yyval.fieldDeclList = slParseArrayLengthList(Compiler, gcvNULL, yyvsp[-4].expr, yyvsp[-1].expr); ;}
    break;

  case 97:

    { yyval.fieldDeclList = slParseArrayLengthList2(Compiler, yyvsp[-2].fieldDeclList, gcvNULL, gcvNULL); ;}
    break;

  case 98:

    { yyval.fieldDeclList = slParseArrayLengthList2(Compiler, yyvsp[-3].fieldDeclList, yyvsp[-1].expr, gcvNULL); ;}
    break;

  case 99:

    { yyval.statement = slParseFuncDecl(Compiler, yyvsp[-1].funcName); ;}
    break;

  case 100:

    { yyval.statement = slParseDeclaration(Compiler, yyvsp[-1].declOrDeclList); ;}
    break;

  case 101:

    { yyval.statement = slParseDefaultPrecisionQualifier(Compiler, &yyvsp[-3].token, &yyvsp[-2].token, yyvsp[-1].dataType); ;}
    break;

  case 102:

    { yyval.statement = yyvsp[0].statement; ;}
    break;

  case 103:

    { yyval.statement = slParseQualifierAsStatement(Compiler, &yyvsp[-1].token); ;}
    break;

  case 104:

    { yyval.funcName = yyvsp[-1].funcName; ;}
    break;

  case 105:

    { yyval.funcName = yyvsp[0].funcName; ;}
    break;

  case 106:

    { yyval.funcName = yyvsp[0].funcName; ;}
    break;

  case 107:

    { yyval.funcName = slParseParameterList(Compiler, yyvsp[-1].funcName, yyvsp[0].paramName); ;}
    break;

  case 108:

    { yyval.funcName = slParseParameterList(Compiler, yyvsp[-2].funcName, yyvsp[0].paramName); ;}
    break;

  case 109:

    { yyval.funcName = slParseFuncHeader(Compiler, yyvsp[-2].dataType, &yyvsp[-1].token); ;}
    break;

  case 110:

    { yyval.funcName = slParseNonArrayParameterDecl(Compiler, yyvsp[-1].dataType, &yyvsp[0].token); ;}
    break;

  case 111:

    { yyval.funcName = slParseArrayParameterDecl(Compiler, yyvsp[-4].dataType, &yyvsp[-3].token, yyvsp[-1].expr); ;}
    break;

  case 112:

    { yyval.funcName = slParseArrayListParameterDecl(Compiler, yyvsp[-2].dataType, &yyvsp[-1].token, yyvsp[0].fieldDeclList); ;}
    break;

  case 113:

    { yyval.paramName = slParseQualifiedParameterDecl(Compiler, gcvNULL, yyvsp[0].funcName); ;}
    break;

  case 114:

    { yyval.paramName = slParseQualifiedParameterDecl(Compiler, gcvNULL, yyvsp[0].funcName); ;}
    break;

  case 115:

    { yyval.paramName = slParseQualifiedParameterDecl(Compiler, &yyvsp[-1].token, yyvsp[0].funcName); ;}
    break;

  case 116:

    { yyval.paramName = slParseQualifiedParameterDecl(Compiler, &yyvsp[-1].token, yyvsp[0].funcName); ;}
    break;

  case 117:

    { yyval.token = yyvsp[0].token; ;}
    break;

  case 118:

    { yyval.token = yyvsp[0].token; ;}
    break;

  case 119:

    { yyval.token = yyvsp[0].token; ;}
    break;

  case 120:

    { yyval.token = yyvsp[0].token; ;}
    break;

  case 121:

    { yyval.token = yyvsp[0].token; ;}
    break;

  case 122:

    { yyval.token = yyvsp[0].token; ;}
    break;

  case 123:

    { yyval.token = yyvsp[0].token; ;}
    break;

  case 124:

    { yyval.token = slMergeParameterQualifiers(Compiler, &yyvsp[-1].token, &yyvsp[0].token); ;}
    break;

  case 125:

    { yyval.funcName = slParseNonArrayParameterDecl(Compiler, yyvsp[0].dataType, gcvNULL); ;}
    break;

  case 126:

    { yyval.declOrDeclList = yyvsp[0].declOrDeclList; ;}
    break;

  case 127:

    { yyval.declOrDeclList = slParseNonArrayVariableDecl2(Compiler, yyvsp[-2].declOrDeclList, &yyvsp[0].token); ;}
    break;

  case 128:

    { yyval.declOrDeclList = slParseArrayVariableDecl2(Compiler, yyvsp[-5].declOrDeclList, &yyvsp[-3].token, yyvsp[-1].expr); ;}
    break;

  case 129:

    { yyval.declOrDeclList = slParseArrayVariableDecl2(Compiler, yyvsp[-4].declOrDeclList, &yyvsp[-2].token, gcvNULL); ;}
    break;

  case 130:

    { yyval.declOrDeclList = slParseArrayListVariableDecl2(Compiler, yyvsp[-3].declOrDeclList, &yyvsp[-1].token, yyvsp[0].fieldDeclList); ;}
    break;

  case 131:

    { yyval.declOrDeclList = slParseVariableDeclWithInitializer2(Compiler, yyvsp[-4].declOrDeclList, &yyvsp[-2].token, yyvsp[0].expr); ;}
    break;

  case 132:

    { yyval.declOrDeclList = slParseArrayVariableDeclWithInitializer2(Compiler, yyvsp[-6].declOrDeclList, &yyvsp[-4].token, gcvNULL, yyvsp[0].expr); ;}
    break;

  case 133:

    { yyval.declOrDeclList = slParseArrayVariableDeclWithInitializer2(Compiler, yyvsp[-7].declOrDeclList, &yyvsp[-5].token, yyvsp[-3].expr, yyvsp[0].expr); ;}
    break;

  case 134:

    { yyval.declOrDeclList = slParseArrayListVariableDeclWithInitializer2(Compiler, yyvsp[-5].declOrDeclList, &yyvsp[-3].token, yyvsp[-2].fieldDeclList, yyvsp[0].expr); ;}
    break;

  case 135:

    { yyval.declOrDeclList = slParseTypeDecl(Compiler, yyvsp[0].dataType); ;}
    break;

  case 136:

    { yyval.declOrDeclList = slParseNonArrayVariableDecl(Compiler, yyvsp[-1].dataType, &yyvsp[0].token); ;}
    break;

  case 137:

    { yyval.declOrDeclList = slParseArrayVariableDecl(Compiler, yyvsp[-4].dataType, &yyvsp[-3].token, yyvsp[-1].expr); ;}
    break;

  case 138:

    { yyval.declOrDeclList = slParseArrayVariableDecl(Compiler, yyvsp[-3].dataType, &yyvsp[-2].token, gcvNULL); ;}
    break;

  case 139:

    { yyval.declOrDeclList = slParseArrayListVariableDecl(Compiler, yyvsp[-2].dataType, &yyvsp[-1].token, yyvsp[0].fieldDeclList); ;}
    break;

  case 140:

    { yyval.declOrDeclList = slParseArrayVariableDeclWithInitializer(Compiler, yyvsp[-5].dataType, &yyvsp[-4].token, gcvNULL, yyvsp[0].expr); ;}
    break;

  case 141:

    { yyval.declOrDeclList = slParseArrayVariableDeclWithInitializer(Compiler, yyvsp[-6].dataType, &yyvsp[-5].token, yyvsp[-3].expr, yyvsp[0].expr); ;}
    break;

  case 142:

    { yyval.declOrDeclList = slParseVariableDeclWithInitializer(Compiler, yyvsp[-3].dataType, &yyvsp[-2].token, yyvsp[0].expr); ;}
    break;

  case 143:

    { yyval.declOrDeclList = slParseInvariantOrPreciseDecl(Compiler, &yyvsp[-1].token, &yyvsp[0].token); ;}
    break;

  case 144:

    { yyval.declOrDeclList = slParseArrayListVariableDeclWithInitializer(Compiler, yyvsp[-4].dataType, &yyvsp[-3].token, yyvsp[-2].fieldDeclList, yyvsp[0].expr); ;}
    break;

  case 145:

    { yyval.dataType = slParseFullySpecifiedType(Compiler, gcvNULL, yyvsp[0].dataType); ;}
    break;

  case 146:

    { yyval.dataType = slParseFullySpecifiedType(Compiler, &yyvsp[-1].token, yyvsp[0].dataType); ;}
    break;

  case 147:

    { yyval.token = slMergeTypeQualifiers(Compiler, gcvNULL, &yyvsp[0].token); ;}
    break;

  case 148:

    { yyval.token = slMergeTypeQualifiers(Compiler, &yyvsp[-1].token, &yyvsp[0].token); ;}
    break;

  case 149:

    { yyval.token = yyvsp[0].token; ;}
    break;

  case 150:

    { yyval.token = yyvsp[0].token; ;}
    break;

  case 151:

    { yyval.token = yyvsp[0].token; ;}
    break;

  case 152:

    { yyval.token = yyvsp[0].token; ;}
    break;

  case 153:

    { yyval.token = yyvsp[0].token; ;}
    break;

  case 154:

    { yyval.token = yyvsp[0].token; ;}
    break;

  case 155:

    { yyval.token = yyvsp[0].token; ;}
    break;

  case 156:

    { yyval.token = yyvsp[0].token; ;}
    break;

  case 157:

    { yyval.token = yyvsp[0].token; ;}
    break;

  case 158:

    { yyval.token = yyvsp[0].token; ;}
    break;

  case 159:

    {
          sloCOMPILER_SetScannerState(Compiler, slvSCANNER_NORMAL);
        ;}
    break;

  case 160:

    {
           yyval.token = slParseLayoutQualifier(Compiler, &yyvsp[-2].token);
        ;}
    break;

  case 161:

    {
          sloCOMPILER_SetScannerState(Compiler, slvSCANNER_NORMAL);
                  yyval.token = yyvsp[-3].token;
        ;}
    break;

  case 162:

    { yyval.token = yyvsp[0].token; ;}
    break;

  case 163:

    { yyval.token = slParseAddLayoutId(Compiler, &yyvsp[-2].token, &yyvsp[0].token); ;}
    break;

  case 164:

    { yyval.token = slParseLayoutId(Compiler, &yyvsp[0].token, gcvNULL); ;}
    break;

  case 165:

    { yyval.token = slParseLayoutId(Compiler, &yyvsp[-2].token, &yyvsp[0].token) ; ;}
    break;

  case 166:

    { yyval.token = slParseLayoutId(Compiler, &yyvsp[-2].token, &yyvsp[0].token) ; ;}
    break;

  case 167:

    { yyval.token = yyvsp[0].token; ;}
    break;

  case 168:

    { yyval.token = yyvsp[0].token; ;}
    break;

  case 169:

    { yyval.token = yyvsp[0].token; ;}
    break;

  case 170:

    { yyval.token = yyvsp[0].token; ;}
    break;

  case 171:

    { yyval.token = yyvsp[0].token; ;}
    break;

  case 172:

    { yyval.token = yyvsp[0].token; ;}
    break;

  case 173:

    { yyval.token = slParseCheckStorage(Compiler, yyvsp[-1].token, yyvsp[0].token); ;}
    break;

  case 174:

    { yyval.token = yyvsp[0].token; ;}
    break;

  case 175:

    { yyval.token = slParseCheckStorage(Compiler, yyvsp[-1].token, yyvsp[0].token); ;}
    break;

  case 176:

    { yyval.token = yyvsp[0].token; ;}
    break;

  case 177:

    { yyval.token = yyvsp[0].token; ;}
    break;

  case 178:

    { yyval.token = yyvsp[0].token; ;}
    break;

  case 179:

    { yyval.token = yyvsp[0].token; ;}
    break;

  case 180:

    { yyval.token = yyvsp[0].token; ;}
    break;

  case 182:

    { yyval.dataType = slParseArrayDataType(Compiler, yyvsp[-2].dataType, gcvNULL); ;}
    break;

  case 183:

    { yyval.dataType = slParseArrayDataType(Compiler, yyvsp[-3].dataType, yyvsp[-1].expr); ;}
    break;

  case 184:

    { yyval.dataType = slParseArrayListDataType(Compiler, yyvsp[-1].dataType, yyvsp[0].fieldDeclList); ;}
    break;

  case 185:

    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_SAMPLER2D); ;}
    break;

  case 186:

    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_SAMPLERCUBE); ;}
    break;

  case 187:

    { yyval.dataType = slParseStructType(Compiler, yyvsp[0].dataType); ;}
    break;

  case 188:

    { yyval.dataType = slParseNamedType(Compiler, &yyvsp[0].token); ;}
    break;

  case 189:

    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_SAMPLER3D); ;}
    break;

  case 190:

    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_SAMPLER1DARRAY); ;}
    break;

  case 191:

    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_SAMPLER2DARRAY); ;}
    break;

  case 192:

    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_SAMPLER1DARRAYSHADOW); ;}
    break;

  case 193:

    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_SAMPLER2DSHADOW); ;}
    break;

  case 194:

    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_SAMPLER2DARRAYSHADOW); ;}
    break;

  case 195:

    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_SAMPLERCUBESHADOW); ;}
    break;

  case 196:

    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_SAMPLERCUBEARRAYSHADOW); ;}
    break;

  case 197:

    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_ISAMPLER2D); ;}
    break;

  case 198:

    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_ISAMPLERCUBE); ;}
    break;

  case 199:

    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_ISAMPLER3D); ;}
    break;

  case 200:

    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_ISAMPLER2DARRAY); ;}
    break;

  case 201:

    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_USAMPLER2D); ;}
    break;

  case 202:

    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_USAMPLERCUBE); ;}
    break;

  case 203:

    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_USAMPLER3D); ;}
    break;

  case 204:

    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_USAMPLER2DARRAY); ;}
    break;

  case 205:

    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_SAMPLEREXTERNALOES); ;}
    break;

  case 206:

    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_SAMPLER2DMS); ;}
    break;

  case 207:

    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_ISAMPLER2DMS); ;}
    break;

  case 208:

    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_USAMPLER2DMS); ;}
    break;

  case 209:

    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_SAMPLER2DMSARRAY); ;}
    break;

  case 210:

    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_ISAMPLER2DMSARRAY); ;}
    break;

  case 211:

    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_USAMPLER2DMSARRAY); ;}
    break;

  case 212:

    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_SAMPLERCUBEARRAY); ;}
    break;

  case 213:

    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_ISAMPLERCUBEARRAY); ;}
    break;

  case 214:

    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_USAMPLERCUBEARRAY); ;}
    break;

  case 215:

    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_IMAGE2D); ;}
    break;

  case 216:

    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_IIMAGE2D); ;}
    break;

  case 217:

    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_UIMAGE2D); ;}
    break;

  case 218:

    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_IMAGE2DARRAY); ;}
    break;

  case 219:

    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_IIMAGE2DARRAY); ;}
    break;

  case 220:

    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_UIMAGE2DARRAY); ;}
    break;

  case 221:

    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_IMAGE3D); ;}
    break;

  case 222:

    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_IIMAGE3D); ;}
    break;

  case 223:

    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_UIMAGE3D); ;}
    break;

  case 224:

    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_IMAGECUBE); ;}
    break;

  case 225:

    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_IMAGECUBEARRAY); ;}
    break;

  case 226:

    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_IIMAGECUBE); ;}
    break;

  case 227:

    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_IIMAGECUBEARRAY); ;}
    break;

  case 228:

    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_UIMAGECUBE); ;}
    break;

  case 229:

    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_UIMAGECUBEARRAY); ;}
    break;

  case 230:

    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_ATOMIC_UINT); ;}
    break;

  case 231:

    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_SAMPLERBUFFER); ;}
    break;

  case 232:

    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_ISAMPLERBUFFER); ;}
    break;

  case 233:

    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_USAMPLERBUFFER); ;}
    break;

  case 234:

    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_IMAGEBUFFER); ;}
    break;

  case 235:

    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_IIMAGEBUFFER); ;}
    break;

  case 236:

    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_UIMAGEBUFFER); ;}
    break;

  case 237:

    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_VOID); ;}
    break;

  case 238:

    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_FLOAT); ;}
    break;

  case 239:

    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_INT); ;}
    break;

  case 240:

    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_UINT); ;}
    break;

  case 241:

    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_BOOL); ;}
    break;

  case 242:

    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_VEC2); ;}
    break;

  case 243:

    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_VEC3); ;}
    break;

  case 244:

    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_VEC4); ;}
    break;

  case 245:

    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_BVEC2); ;}
    break;

  case 246:

    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_BVEC3); ;}
    break;

  case 247:

    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_BVEC4); ;}
    break;

  case 248:

    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_IVEC2); ;}
    break;

  case 249:

    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_IVEC3); ;}
    break;

  case 250:

    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_IVEC4); ;}
    break;

  case 251:

    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_UVEC2); ;}
    break;

  case 252:

    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_UVEC3); ;}
    break;

  case 253:

    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_UVEC4); ;}
    break;

  case 254:

    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_MAT2); ;}
    break;

  case 255:

    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_MAT2X3); ;}
    break;

  case 256:

    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_MAT2X4); ;}
    break;

  case 257:

    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_MAT3); ;}
    break;

  case 258:

    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_MAT3X2); ;}
    break;

  case 259:

    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_MAT3X4); ;}
    break;

  case 260:

    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_MAT4); ;}
    break;

  case 261:

    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_MAT4X2); ;}
    break;

  case 262:

    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_MAT4X3); ;}
    break;

  case 263:

    { yyval.dataType = yyvsp[0].dataType; ;}
    break;

  case 264:

    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_FLOAT); ;}
    break;

  case 265:

    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_INT); ;}
    break;

  case 266:

    { yyval.dataType = yyvsp[0].dataType; ;}
    break;

  case 267:

    { yyval.token = yyvsp[0].token; ;}
    break;

  case 268:

    { yyval.token = yyvsp[0].token; ;}
    break;

  case 269:

    { yyval.token = yyvsp[0].token; ;}
    break;

  case 270:

    { yyval.token = yyvsp[0].token; ;}
    break;

  case 271:

    { yyval.token = yyvsp[0].token; ;}
    break;

  case 272:

    { yyval.token = yyvsp[0].token; ;}
    break;

  case 273:

    { yyval.token = yyvsp[0].token; ;}
    break;

  case 274:

    { yyval.token = yyvsp[0].token; ;}
    break;

  case 275:

    { slParseStructDeclBegin(Compiler, &yyvsp[-1].token); ;}
    break;

  case 276:

    { yyval.dataType = slParseStructDeclEnd(Compiler, &yyvsp[-4].token); ;}
    break;

  case 277:

    { slParseStructDeclBegin(Compiler, gcvNULL); ;}
    break;

  case 278:

    { yyval.dataType = slParseStructDeclEnd(Compiler, gcvNULL); ;}
    break;

  case 279:

    { slParseStructReDeclBegin(Compiler, &yyvsp[-1].token); ;}
    break;

  case 280:

    { yyval.dataType = slParseStructReDeclEnd(Compiler, &yyvsp[-4].token); ;}
    break;

  case 283:

    { slParseTypeSpecifiedFieldDeclList(Compiler, yyvsp[-2].dataType, yyvsp[-1].fieldDeclList); ;}
    break;

  case 284:

    {
           yyvsp[-2].dataType->qualifiers.precision = yyvsp[-3].token.u.qualifiers.precision;
           slsQUALIFIERS_SET_FLAG(&(yyvsp[-2].dataType->qualifiers), slvQUALIFIERS_FLAG_PRECISION);
           slParseTypeSpecifiedFieldDeclList(Compiler, yyvsp[-2].dataType, yyvsp[-1].fieldDeclList);
        ;}
    break;

  case 285:

    { yyval.fieldDeclList = slParseFieldDeclList(Compiler, yyvsp[0].fieldDecl); ;}
    break;

  case 286:

    { yyval.fieldDeclList = slParseFieldDeclList2(Compiler, yyvsp[-2].fieldDeclList, yyvsp[0].fieldDecl); ;}
    break;

  case 287:

    { yyval.fieldDecl = slParseFieldDecl(Compiler, &yyvsp[0].token, gcvNULL); ;}
    break;

  case 288:

    { yyval.fieldDecl = slParseFieldDecl(Compiler, &yyvsp[-3].token, yyvsp[-1].expr); ;}
    break;

  case 289:

    { yyval.fieldDecl = slParseFieldListDecl(Compiler, &yyvsp[-1].token, yyvsp[0].fieldDeclList, gcvFALSE); ;}
    break;

  case 290:

    { yyval.statement = slParseInterfaceBlock(Compiler, yyvsp[-1].blockName, gcvNULL, gcvNULL, gcvTRUE); ;}
    break;

  case 291:

    { yyval.statement = slParseInterfaceBlock(Compiler, yyvsp[-2].blockName, &yyvsp[-1].token, gcvNULL, gcvTRUE); ;}
    break;

  case 292:

    { yyval.statement = slParseInterfaceBlock(Compiler, yyvsp[-5].blockName, &yyvsp[-4].token, yyvsp[-2].expr, gcvTRUE); ;}
    break;

  case 293:

    { yyval.statement = slParseInterfaceBlockImplicitArrayLength(Compiler, yyvsp[-4].blockName, &yyvsp[-3].token); ;}
    break;

  case 294:

    { slParseInterfaceBlockDeclBegin(Compiler, &yyvsp[-2].token, &yyvsp[-1].token); ;}
    break;

  case 295:

    { yyval.blockName = slParseInterfaceBlockDeclEnd(Compiler, &yyvsp[-5].token, &yyvsp[-4].token); ;}
    break;

  case 296:

    { slParseInterfaceBlockDeclBegin(Compiler, &yyvsp[-2].token, &yyvsp[-1].token); ;}
    break;

  case 297:

    {
            yyclearin;
            yyerrok;
            yyval.blockName = slParseInterfaceBlockDeclEnd(Compiler, gcvNULL, gcvNULL);
        ;}
    break;

  case 300:

    { yyval.dataType = slParseInterfaceBlockMember(Compiler, yyvsp[-1].dataType, yyvsp[0].fieldDecl); ;}
    break;

  case 301:

    { yyval.dataType = slParseInterfaceBlockMember(Compiler, yyvsp[-2].dataType, yyvsp[0].fieldDecl); ;}
    break;

  case 303:

    { yyval.fieldDecl = slParseFieldDecl(Compiler, &yyvsp[0].token, gcvNULL); ;}
    break;

  case 304:

    { yyval.fieldDecl = slParseFieldDecl(Compiler, &yyvsp[-3].token, yyvsp[-1].expr); ;}
    break;

  case 305:

    { yyval.fieldDecl = slParseImplicitArraySizeFieldDecl(Compiler, &yyvsp[-2].token); ;}
    break;

  case 306:

    { yyval.fieldDecl = slParseFieldListDecl(Compiler, &yyvsp[-1].token, yyvsp[0].fieldDeclList, gcvTRUE); ;}
    break;

  case 307:

    { yyval.expr = yyvsp[0].expr; ;}
    break;

  case 308:

    { yyval.statement = yyvsp[0].statement; ;}
    break;

  case 309:

    { yyval.statement = slParseCompoundStatementAsStatement(Compiler, yyvsp[0].statements); ;}
    break;

  case 310:

    { yyval.statement = yyvsp[0].statement; ;}
    break;

  case 311:

    { yyval.statement = yyvsp[0].statement; ;}
    break;

  case 312:

    { yyval.statement = yyvsp[0].statement; ;}
    break;

  case 313:

    { yyval.statement = yyvsp[0].statement; ;}
    break;

  case 314:

    { yyval.statement = yyvsp[0].statement; ;}
    break;

  case 315:

    { yyval.statement = yyvsp[0].statement; ;}
    break;

  case 316:

    { yyval.statement = yyvsp[0].statement; ;}
    break;

  case 317:

    { yyval.statements = gcvNULL; ;}
    break;

  case 318:

    { slParseCompoundStatementBegin(Compiler); ;}
    break;

  case 319:

    { yyval.statements = slParseCompoundStatementEnd(Compiler, &yyvsp[-3].token, yyvsp[-1].statements); ;}
    break;

  case 320:

    { yyval.statement = slParseCompoundStatementNoNewScopeAsStatementNoNewScope(Compiler, yyvsp[0].statements); ;}
    break;

  case 321:

    { yyval.statement = yyvsp[0].statement; ;}
    break;

  case 322:

    { yyval.statements = gcvNULL; ;}
    break;

  case 323:

    { slParseCompoundStatementNoNewScopeBegin(Compiler); ;}
    break;

  case 324:

    { yyval.statements = slParseCompoundStatementNoNewScopeEnd(Compiler, &yyvsp[-3].token, yyvsp[-1].statements); ;}
    break;

  case 325:

    { yyval.statements = slParseStatementList(Compiler, yyvsp[0].statement); ;}
    break;

  case 326:

    { yyval.statements = slParseStatementList2(Compiler, yyvsp[-1].statements, yyvsp[0].statement); ;}
    break;

  case 327:

    { yyval.statement = gcvNULL; ;}
    break;

  case 328:

    { yyval.statement = slParseExprAsStatement(Compiler, yyvsp[-1].expr); ;}
    break;

  case 329:

    { slParseSelectStatementBegin(Compiler); ;}
    break;

  case 330:

    { yyval.statement = slParseSelectionStatement(Compiler, &yyvsp[-5].token, yyvsp[-3].expr, yyvsp[0].selectionStatementPair); ;}
    break;

  case 331:

    { yyval.statement = slParseSwitchStatement(Compiler, &yyvsp[-4].token, yyvsp[-2].expr, yyvsp[0].statement); ;}
    break;

  case 332:

    { yyval.statements = slParseStatementList(Compiler, yyvsp[0].statement); ;}
    break;

  case 333:

    { yyval.statements = slParseStatementList2(Compiler, yyvsp[-1].statements, yyvsp[0].statement); ;}
    break;

  case 334:

    { yyval.statement = yyvsp[0].statement; ;}
    break;

  case 335:

    { yyval.statement = slParseCaseStatement(Compiler, &yyvsp[-2].token, yyvsp[-1].expr); ;}
    break;

  case 336:

    { yyval.statement = slParseDefaultStatement(Compiler, &yyvsp[-1].token); ;}
    break;

  case 337:

    { yyval.statement = gcvNULL; ;}
    break;

  case 338:

    { slParseSwitchBodyBegin(Compiler); ;}
    break;

  case 339:

    { yyval.statement = slParseSwitchBodyEnd(Compiler, &yyvsp[-3].token, yyvsp[-1].statements); ;}
    break;

  case 340:

    { slParseSelectStatementEnd(Compiler, gcvNULL, gcvNULL);
          slParseSelectStatementBegin(Compiler); ;}
    break;

  case 341:

    { slParseSelectStatementEnd(Compiler, gcvNULL, gcvNULL);
          yyval.selectionStatementPair = slParseSelectionRestStatement(Compiler, yyvsp[-3].statement, yyvsp[0].statement); ;}
    break;

  case 342:

    { slParseSelectStatementEnd(Compiler, gcvNULL, gcvNULL);
          yyval.selectionStatementPair = slParseSelectionRestStatement(Compiler, yyvsp[0].statement, gcvNULL); ;}
    break;

  case 343:

    { yyval.expr = yyvsp[0].expr; ;}
    break;

  case 344:

    { yyval.expr = slParseCondition(Compiler, yyvsp[-3].dataType, &yyvsp[-2].token, yyvsp[0].expr); ;}
    break;

  case 345:

    { slParseWhileStatementBegin(Compiler); ;}
    break;

  case 346:

    { yyval.statement = slParseWhileStatementEnd(Compiler, &yyvsp[-5].token, yyvsp[-2].expr, yyvsp[0].statement); ;}
    break;

  case 347:

    { yyval.statement = slParseDoWhileStatement(Compiler, &yyvsp[-6].token, yyvsp[-5].statement, yyvsp[-2].expr); ;}
    break;

  case 348:

    { slParseForStatementBegin(Compiler); ;}
    break;

  case 349:

    { yyval.statement = slParseForStatementEnd(Compiler, &yyvsp[-6].token, yyvsp[-3].statement, yyvsp[-2].forExprPair, yyvsp[0].statement); ;}
    break;

  case 350:

    { yyval.statement = yyvsp[0].statement; ;}
    break;

  case 351:

    { yyval.statement = yyvsp[0].statement; ;}
    break;

  case 352:

    { yyval.expr = yyvsp[0].expr; ;}
    break;

  case 353:

    { yyval.expr = gcvNULL; ;}
    break;

  case 354:

    { yyval.forExprPair = slParseForRestStatement(Compiler, yyvsp[-1].expr, gcvNULL); ;}
    break;

  case 355:

    { yyval.forExprPair = slParseForRestStatement(Compiler, yyvsp[-2].expr, yyvsp[0].expr); ;}
    break;

  case 356:

    { yyval.statement = slParseJumpStatement(Compiler, slvCONTINUE, &yyvsp[-1].token, gcvNULL); ;}
    break;

  case 357:

    { yyval.statement = slParseJumpStatement(Compiler, slvBREAK, &yyvsp[-1].token, gcvNULL); ;}
    break;

  case 358:

    { yyval.statement = slParseJumpStatement(Compiler, slvRETURN, &yyvsp[-1].token, gcvNULL); ;}
    break;

  case 359:

    { yyval.statement = slParseJumpStatement(Compiler, slvRETURN, &yyvsp[-2].token, yyvsp[-1].expr); ;}
    break;

  case 360:

    { yyval.statement = slParseJumpStatement(Compiler, slvDISCARD, &yyvsp[-1].token, gcvNULL); ;}
    break;

  case 364:

    { slParseExternalDecl(Compiler, yyvsp[0].statement); ;}
    break;

  case 365:

    { slParseFuncDefinitionBegin(Compiler, yyvsp[0].funcName); ;}
    break;

  case 366:

    { slParseFuncDefinitionEnd(Compiler, yyvsp[-2].funcName); ;}
    break;

  case 367:

    { slParseFuncDef(Compiler, yyvsp[-3].funcName, yyvsp[-1].statements); ;}
    break;


    }

/* Line 1000 of yacc.c.  */


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
  yyerrstatus = 3;  /* Each real token shifted decrements this.  */

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






