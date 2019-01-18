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
     T_LAYOUT = 393,
     T_UNIFORM_BLOCK = 394,
     T_IDENTIFIER = 395,
     T_TYPE_NAME = 396,
     T_FLOATCONSTANT = 397,
     T_INTCONSTANT = 398,
     T_BOOLCONSTANT = 399,
     T_UINTCONSTANT = 400,
     T_FIELD_SELECTION = 401,
     T_LEFT_OP = 402,
     T_RIGHT_OP = 403,
     T_INC_OP = 404,
     T_DEC_OP = 405,
     T_LE_OP = 406,
     T_GE_OP = 407,
     T_EQ_OP = 408,
     T_NE_OP = 409,
     T_AND_OP = 410,
     T_OR_OP = 411,
     T_XOR_OP = 412,
     T_MUL_ASSIGN = 413,
     T_DIV_ASSIGN = 414,
     T_ADD_ASSIGN = 415,
     T_MOD_ASSIGN = 416,
     T_LEFT_ASSIGN = 417,
     T_RIGHT_ASSIGN = 418,
     T_AND_ASSIGN = 419,
     T_XOR_ASSIGN = 420,
     T_OR_ASSIGN = 421,
     T_SUB_ASSIGN = 422,
     T_LENGTH_METHOD = 423,
     T_INVARIANT = 424,
     T_HIGH_PRECISION = 425,
     T_MEDIUM_PRECISION = 426,
     T_LOW_PRECISION = 427,
     T_PRECISION = 428,
     T_PRECISE = 429,
     T_COHERENT = 430,
     T_VOLATILE = 431,
     T_RESTRICT = 432,
     T_READONLY = 433,
     T_WRITEONLY = 434,
     T_VIV_ASM = 435,
     T_ASM_OPND_BRACKET = 436
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
#define T_LAYOUT 393
#define T_UNIFORM_BLOCK 394
#define T_IDENTIFIER 395
#define T_TYPE_NAME 396
#define T_FLOATCONSTANT 397
#define T_INTCONSTANT 398
#define T_BOOLCONSTANT 399
#define T_UINTCONSTANT 400
#define T_FIELD_SELECTION 401
#define T_LEFT_OP 402
#define T_RIGHT_OP 403
#define T_INC_OP 404
#define T_DEC_OP 405
#define T_LE_OP 406
#define T_GE_OP 407
#define T_EQ_OP 408
#define T_NE_OP 409
#define T_AND_OP 410
#define T_OR_OP 411
#define T_XOR_OP 412
#define T_MUL_ASSIGN 413
#define T_DIV_ASSIGN 414
#define T_ADD_ASSIGN 415
#define T_MOD_ASSIGN 416
#define T_LEFT_ASSIGN 417
#define T_RIGHT_ASSIGN 418
#define T_AND_ASSIGN 419
#define T_XOR_ASSIGN 420
#define T_OR_ASSIGN 421
#define T_SUB_ASSIGN 422
#define T_LENGTH_METHOD 423
#define T_INVARIANT 424
#define T_HIGH_PRECISION 425
#define T_MEDIUM_PRECISION 426
#define T_LOW_PRECISION 427
#define T_PRECISION 428
#define T_PRECISE 429
#define T_COHERENT 430
#define T_VOLATILE 431
#define T_RESTRICT 432
#define T_READONLY 433
#define T_WRITEONLY 434
#define T_VIV_ASM 435
#define T_ASM_OPND_BRACKET 436




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
#line 496 "gc_glsl_parser.c"
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif



/* Copy the second part of user declarations.  */


/* Line 214 of yacc.c.  */
#line 508 "gc_glsl_parser.c"

#if !defined (yyoverflow) || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

#if YYSTACK_USE_ALLOCA
#  define YYSTACK_ALLOC slMalloc
# else
#ifndef YYSTACK_USE_ALLOCA
#if defined (alloca) || defined (_ALLOCA_H)
#    define YYSTACK_ALLOC slMalloc
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
#  define YYSTACK_ALLOC slMalloc
#  define YYSTACK_FREE slFree
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
#define YYFINAL  188
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   5959

/* YYNTOKENS -- Number of terminals. */
#define YYNTOKENS  206
/* YYNNTS -- Number of nonterminals. */
#define YYNNTS  114
/* YYNRULES -- Number of rules. */
#define YYNRULES  390
/* YYNRULES -- Number of states. */
#define YYNSTATES  556

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   436

#define YYTRANSLATE(YYX)                         \
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const unsigned char yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   191,     2,     2,     2,   197,   202,     2,
     180,   181,   195,   194,   187,   192,   186,   196,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,   188,   190,
     198,   189,   199,   203,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,   182,     2,   183,   201,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   184,   200,   185,   193,     2,     2,     2,
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
     175,   176,   177,   178,   179,   204,   205
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
     754,   756,   758,   760,   762,   764,   766,   768,   770,   772,
     774,   776,   778,   780,   782,   784,   786,   788,   790,   792,
     794,   796,   798,   800,   802,   804,   806,   808,   810,   811,
     818,   819,   825,   826,   833,   835,   838,   842,   847,   849,
     853,   855,   860,   863,   866,   870,   877,   883,   884,   891,
     892,   899,   901,   904,   906,   910,   914,   916,   921,   925,
     928,   930,   932,   934,   936,   938,   940,   942,   944,   946,
     948,   951,   952,   957,   959,   961,   964,   965,   970,   972,
     975,   977,   980,   981,   988,   994,   996,   999,  1001,  1005,
    1008,  1011,  1012,  1017,  1018,  1023,  1025,  1027,  1032,  1033,
    1040,  1048,  1049,  1057,  1059,  1061,  1063,  1064,  1067,  1071,
    1074,  1077,  1080,  1084,  1087,  1089,  1092,  1094,  1096,  1097,
    1098
};

/* YYRHS -- A `-1'-separated list of the rules' RHS. */
static const short yyrhs[] =
{
     315,     0,    -1,   140,    -1,   207,    -1,   143,    -1,   145,
      -1,   142,    -1,   144,    -1,   180,   239,   181,    -1,   208,
      -1,   209,   182,   210,   183,    -1,   217,    -1,   209,   186,
     146,    -1,   209,   186,   168,    -1,   209,   149,    -1,   209,
     150,    -1,   239,    -1,   212,   181,   190,    -1,   216,    -1,
     212,   187,   213,    -1,   237,    -1,   237,   205,   214,   199,
      -1,   215,    -1,   214,   187,   215,    -1,   140,   188,   140,
      -1,   204,   180,   140,    -1,   204,   180,   140,   205,   214,
     199,    -1,   218,    -1,   220,   181,    -1,   219,   181,    -1,
     221,   125,    -1,   221,    -1,   221,   237,    -1,   220,   187,
     237,    -1,   222,   180,    -1,   255,    -1,   140,    -1,   209,
      -1,   149,   223,    -1,   150,   223,    -1,   224,   223,    -1,
     194,    -1,   192,    -1,   191,    -1,   193,    -1,   223,    -1,
     225,   195,   223,    -1,   225,   196,   223,    -1,   225,   197,
     223,    -1,   225,    -1,   226,   194,   225,    -1,   226,   192,
     225,    -1,   226,    -1,   227,   147,   226,    -1,   227,   148,
     226,    -1,   227,    -1,   228,   198,   227,    -1,   228,   199,
     227,    -1,   228,   151,   227,    -1,   228,   152,   227,    -1,
     228,    -1,   229,   153,   228,    -1,   229,   154,   228,    -1,
     229,    -1,   230,   202,   229,    -1,   230,    -1,   231,   201,
     230,    -1,   231,    -1,   232,   200,   231,    -1,   232,    -1,
     233,   155,   232,    -1,   233,    -1,   234,   157,   233,    -1,
     234,    -1,   235,   156,   234,    -1,   235,    -1,   235,   203,
     239,   188,   237,    -1,   236,    -1,   223,   238,   237,    -1,
     189,    -1,   158,    -1,   159,    -1,   161,    -1,   160,    -1,
     167,    -1,   162,    -1,   163,    -1,   164,    -1,   165,    -1,
     166,    -1,   237,    -1,   239,   187,   237,    -1,   236,    -1,
     182,   183,   182,   183,    -1,   182,   183,   182,   240,   183,
      -1,   182,   240,   183,   182,   183,    -1,   182,   240,   183,
     182,   240,   183,    -1,   241,   182,   183,    -1,   241,   182,
     240,   183,    -1,   243,   190,    -1,   253,   190,    -1,   173,
     270,   269,   190,    -1,   280,    -1,   256,   190,    -1,   244,
     181,    -1,   246,    -1,   245,    -1,   246,   248,    -1,   245,
     187,   248,    -1,   255,   140,   180,    -1,   266,   140,    -1,
     266,   140,   182,   240,   183,    -1,   266,   140,   241,    -1,
     247,    -1,   252,    -1,   251,   247,    -1,   251,   252,    -1,
      33,    -1,    34,    -1,    35,    -1,   249,    -1,   263,    -1,
     270,    -1,   250,    -1,   251,   250,    -1,   266,    -1,   254,
      -1,   253,   187,   140,    -1,   253,   187,   140,   182,   240,
     183,    -1,   253,   187,   140,   182,   183,    -1,   253,   187,
     140,   241,    -1,   253,   187,   140,   189,   288,    -1,   253,
     187,   140,   182,   183,   189,   288,    -1,   253,   187,   140,
     182,   240,   183,   189,   288,    -1,   253,   187,   140,   241,
     189,   288,    -1,   255,    -1,   255,   140,    -1,   255,   140,
     182,   240,   183,    -1,   255,   140,   182,   183,    -1,   255,
     140,   241,    -1,   255,   140,   182,   183,   189,   288,    -1,
     255,   140,   182,   240,   183,   189,   288,    -1,   255,   140,
     189,   288,    -1,   256,   140,    -1,   255,   140,   241,   189,
     288,    -1,   266,    -1,   256,   266,    -1,   257,    -1,   256,
     257,    -1,   265,    -1,   271,    -1,   259,    -1,   169,    -1,
     174,    -1,   270,    -1,   258,    -1,   264,    -1,   137,    -1,
     136,    -1,    -1,   138,   180,   261,   260,   181,    -1,   138,
     180,     1,   181,    -1,   262,    -1,   261,   187,   262,    -1,
     140,    -1,   140,   189,   143,    -1,   140,   189,   145,    -1,
       4,    -1,   174,    -1,   135,    -1,    39,    -1,     4,    -1,
      33,    -1,    38,    33,    -1,    34,    -1,    38,    34,    -1,
       3,    -1,    37,    -1,    36,    -1,   122,    -1,   123,    -1,
     268,    -1,   268,   182,   183,    -1,   268,   182,   240,   183,
      -1,   268,   241,    -1,    59,    -1,    60,    -1,   272,    -1,
     141,    -1,    65,    -1,    66,    -1,    67,    -1,    68,    -1,
      64,    -1,    69,    -1,    62,    -1,    63,    -1,    70,    -1,
      71,    -1,    73,    -1,    74,    -1,    75,    -1,    76,    -1,
      78,    -1,    79,    -1,    80,    -1,    81,    -1,    82,    -1,
      83,    -1,    84,    -1,    85,    -1,    86,    -1,    61,    -1,
      72,    -1,    77,    -1,    90,    -1,    91,    -1,    92,    -1,
      93,    -1,    94,    -1,    95,    -1,    96,    -1,    97,    -1,
      98,    -1,    99,    -1,   100,    -1,   101,    -1,   102,    -1,
     103,    -1,   104,    -1,   105,    -1,   106,    -1,   107,    -1,
     108,    -1,   109,    -1,   112,    -1,   110,    -1,   113,    -1,
     111,    -1,   114,    -1,   121,    -1,    87,    -1,    88,    -1,
      89,    -1,   115,    -1,   116,    -1,   117,    -1,   125,    -1,
       6,    -1,     7,    -1,    40,    -1,     5,    -1,    24,    -1,
      25,    -1,    26,    -1,    18,    -1,    19,    -1,    20,    -1,
      21,    -1,    22,    -1,    23,    -1,    41,    -1,    42,    -1,
      43,    -1,    30,    -1,    44,    -1,    45,    -1,    31,    -1,
      46,    -1,    47,    -1,    32,    -1,    48,    -1,    49,    -1,
       8,    -1,    27,    -1,    28,    -1,    29,    -1,    50,    -1,
      53,    -1,    54,    -1,    51,    -1,    55,    -1,    56,    -1,
      52,    -1,    57,    -1,    58,    -1,   267,    -1,     6,    -1,
       7,    -1,   267,    -1,   170,    -1,   171,    -1,   172,    -1,
     175,    -1,   176,    -1,   177,    -1,   178,    -1,   179,    -1,
      -1,   124,   140,   184,   273,   276,   185,    -1,    -1,   124,
     184,   274,   276,   185,    -1,    -1,   124,   141,   184,   275,
     276,   185,    -1,   277,    -1,   276,   277,    -1,   266,   278,
     190,    -1,   270,   266,   278,   190,    -1,   279,    -1,   278,
     187,   279,    -1,   140,    -1,   140,   182,   240,   183,    -1,
     140,   241,    -1,   281,   190,    -1,   281,   140,   190,    -1,
     281,   140,   182,   240,   183,   190,    -1,   281,   140,   182,
     183,   190,    -1,    -1,   256,   140,   184,   282,   284,   185,
      -1,    -1,   256,   140,   184,   283,     1,   185,    -1,   286,
      -1,   284,   286,    -1,   287,    -1,   285,   187,   287,    -1,
     255,   285,   190,    -1,   140,    -1,   140,   182,   240,   183,
      -1,   140,   182,   183,    -1,   140,   241,    -1,   237,    -1,
     242,    -1,   292,    -1,   291,    -1,   289,    -1,   298,    -1,
     299,    -1,   308,    -1,   314,    -1,   211,    -1,   184,   185,
      -1,    -1,   184,   293,   297,   185,    -1,   295,    -1,   291,
      -1,   184,   185,    -1,    -1,   184,   296,   297,   185,    -1,
     290,    -1,   297,   290,    -1,   190,    -1,   239,   190,    -1,
      -1,    14,   180,   239,   181,   300,   305,    -1,   132,   180,
     210,   181,   303,    -1,   302,    -1,   301,   302,    -1,   290,
      -1,   133,   240,   188,    -1,   134,   188,    -1,   184,   185,
      -1,    -1,   184,   304,   301,   185,    -1,    -1,   290,    12,
     306,   290,    -1,   290,    -1,   239,    -1,   255,   140,   189,
     288,    -1,    -1,   126,   309,   180,   307,   181,   294,    -1,
      11,   290,   126,   180,   239,   181,   190,    -1,    -1,    13,
     310,   180,   311,   313,   181,   294,    -1,   298,    -1,   289,
      -1,   307,    -1,    -1,   312,   190,    -1,   312,   190,   239,
      -1,    10,   190,    -1,     9,   190,    -1,    16,   190,    -1,
      16,   239,   190,    -1,    15,   190,    -1,   316,    -1,   315,
     316,    -1,   317,    -1,   242,    -1,    -1,    -1,   243,   318,
     295,   319,    -1
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
     649,   651,   653,   655,   657,   659,   661,   666,   668,   674,
     673,   681,   689,   691,   696,   698,   700,   705,   707,   712,
     714,   719,   721,   723,   725,   727,   729,   731,   733,   735,
     737,   742,   743,   745,   747,   752,   754,   756,   758,   760,
     762,   764,   766,   768,   770,   772,   774,   776,   778,   780,
     782,   784,   786,   788,   790,   792,   794,   796,   798,   800,
     802,   804,   806,   808,   810,   812,   814,   816,   818,   820,
     822,   824,   826,   828,   830,   832,   834,   836,   838,   840,
     842,   844,   846,   848,   850,   852,   854,   856,   858,   860,
     862,   864,   866,   868,   870,   872,   874,   879,   881,   883,
     885,   887,   889,   891,   893,   895,   897,   899,   901,   903,
     905,   907,   909,   911,   913,   915,   917,   919,   921,   923,
     925,   927,   929,   931,   933,   935,   937,   939,   941,   943,
     945,   947,   949,   951,   953,   955,   957,   962,   964,   966,
     971,   973,   975,   980,   982,   984,   986,   988,   994,   993,
     998,   997,  1002,  1001,  1008,  1009,  1013,  1015,  1024,  1026,
    1031,  1033,  1035,  1040,  1042,  1044,  1046,  1052,  1051,  1056,
    1055,  1066,  1067,  1071,  1073,  1078,  1082,  1084,  1086,  1088,
    1093,  1098,  1103,  1105,  1112,  1114,  1116,  1118,  1120,  1122,
    1127,  1130,  1129,  1136,  1138,  1143,  1146,  1145,  1152,  1154,
    1159,  1161,  1167,  1166,  1170,  1175,  1177,  1182,  1184,  1186,
    1191,  1194,  1193,  1202,  1200,  1207,  1215,  1217,  1223,  1222,
    1226,  1229,  1228,  1235,  1237,  1242,  1245,  1249,  1251,  1256,
    1258,  1260,  1262,  1264,  1271,  1272,  1276,  1277,  1283,  1285,
    1282
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
  "T_MAT3", "T_MAT4", "T_IN", "T_OUT", "T_INOUT", "T_UNIFORM",
  "T_VARYING", "T_PATCH", "T_SAMPLE", "T_UINT", "T_UVEC2", "T_UVEC3",
  "T_UVEC4", "T_MAT2X3", "T_MAT2X4", "T_MAT3X2", "T_MAT3X4", "T_MAT4X2",
  "T_MAT4X3", "T_DMAT2", "T_DMAT3", "T_DMAT4", "T_DMAT2X3", "T_DMAT2X4",
  "T_DMAT3X2", "T_DMAT3X4", "T_DMAT4X2", "T_DMAT4X3", "T_SAMPLER2D",
  "T_SAMPLERCUBE", "T_SAMPLERCUBEARRAY", "T_SAMPLERCUBESHADOW",
  "T_SAMPLERCUBEARRAYSHADOW", "T_SAMPLER2DSHADOW", "T_SAMPLER3D",
  "T_SAMPLER1DARRAY", "T_SAMPLER2DARRAY", "T_SAMPLER1DARRAYSHADOW",
  "T_SAMPLER2DARRAYSHADOW", "T_ISAMPLER2D", "T_ISAMPLERCUBE",
  "T_ISAMPLERCUBEARRAY", "T_ISAMPLER3D", "T_ISAMPLER2DARRAY",
  "T_USAMPLER2D", "T_USAMPLERCUBE", "T_USAMPLERCUBEARRAY", "T_USAMPLER3D",
  "T_USAMPLER2DARRAY", "T_SAMPLEREXTERNALOES", "T_SAMPLER2DMS",
  "T_ISAMPLER2DMS", "T_USAMPLER2DMS", "T_SAMPLER2DMSARRAY",
  "T_ISAMPLER2DMSARRAY", "T_USAMPLER2DMSARRAY", "T_SAMPLERBUFFER",
  "T_ISAMPLERBUFFER", "T_USAMPLERBUFFER", "T_SAMPLER1D", "T_ISAMPLER1D",
  "T_USAMPLER1D", "T_SAMPLER1DSHADOW", "T_SAMPLER2DRECT",
  "T_ISAMPLER2DRECT", "T_USAMPLER2DRECT", "T_SAMPLER2DRECTSHADOW",
  "T_ISAMPLER1DARRAY", "T_USAMPLER1DARRAY", "T_IMAGE2D", "T_IIMAGE2D",
  "T_UIMAGE2D", "T_IMAGE2DARRAY", "T_IIMAGE2DARRAY", "T_UIMAGE2DARRAY",
  "T_IMAGE3D", "T_IIMAGE3D", "T_UIMAGE3D", "T_IMAGECUBE", "T_IIMAGECUBE",
  "T_UIMAGECUBE", "T_IMAGECUBEARRAY", "T_IIMAGECUBEARRAY",
  "T_UIMAGECUBEARRAY", "T_IMAGEBUFFER", "T_IIMAGEBUFFER",
  "T_UIMAGEBUFFER", "T_GEN_SAMPLER", "T_GEN_ISAMPLER", "T_GEN_USAMPLER",
  "T_ATOMIC_UINT", "T_BUFFER", "T_SHARED", "T_STRUCT", "T_VOID",
  "T_WHILE", "T_IO_BLOCK", "T_ARRAY4_OF_IVEC2", "T_TYPE_MATCH_CALLBACK0",
  "T_TYPE_MATCH_CALLBACK1", "T_TYPE_MATCH_CALLBACK2", "T_SWITCH",
  "T_CASE", "T_DEFAULT", "T_CENTROID", "T_FLAT", "T_SMOOTH", "T_LAYOUT",
  "T_UNIFORM_BLOCK", "T_IDENTIFIER", "T_TYPE_NAME", "T_FLOATCONSTANT",
  "T_INTCONSTANT", "T_BOOLCONSTANT", "T_UINTCONSTANT",
  "T_FIELD_SELECTION", "T_LEFT_OP", "T_RIGHT_OP", "T_INC_OP", "T_DEC_OP",
  "T_LE_OP", "T_GE_OP", "T_EQ_OP", "T_NE_OP", "T_AND_OP", "T_OR_OP",
  "T_XOR_OP", "T_MUL_ASSIGN", "T_DIV_ASSIGN", "T_ADD_ASSIGN",
  "T_MOD_ASSIGN", "T_LEFT_ASSIGN", "T_RIGHT_ASSIGN", "T_AND_ASSIGN",
  "T_XOR_ASSIGN", "T_OR_ASSIGN", "T_SUB_ASSIGN", "T_LENGTH_METHOD",
  "T_INVARIANT", "T_HIGH_PRECISION", "T_MEDIUM_PRECISION",
  "T_LOW_PRECISION", "T_PRECISION", "T_PRECISE", "T_COHERENT",
  "T_VOLATILE", "T_RESTRICT", "T_READONLY", "T_WRITEONLY", "'('", "')'",
  "'['", "']'", "'{'", "'}'", "'.'", "','", "':'", "'='", "';'", "'!'",
  "'-'", "'~'", "'+'", "'*'", "'/'", "'%'", "'<'", "'>'", "'|'", "'^'",
  "'&'", "'?'", "T_VIV_ASM", "T_ASM_OPND_BRACKET", "$accept",
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
  "layout_qualifier_id", "parameter_type_qualifier",
  "auxiliary_qualifier", "storage_qualifier", "type_specifier",
  "opaque_type_specifier", "type_specifier_nonarray",
  "default_precision_type_specifier", "precision_qualifier",
  "memory_access_qualifier", "struct_specifier", "@2", "@3", "@4",
  "struct_declaration_list", "struct_declaration",
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
  "translation_unit", "external_declaration", "function_definition",
  "@14", "@15", 0
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
      40,    41,    91,    93,   123,   125,    46,    44,    58,    61,
      59,    33,    45,   126,    43,    42,    47,    37,    60,    62,
     124,    94,    38,    63,   435,   436
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const unsigned short yyr1[] =
{
       0,   206,   207,   208,   208,   208,   208,   208,   208,   209,
     209,   209,   209,   209,   209,   209,   210,   211,   212,   212,
     213,   213,   214,   214,   215,   216,   216,   217,   218,   218,
     219,   219,   220,   220,   221,   222,   222,   223,   223,   223,
     223,   224,   224,   224,   224,   225,   225,   225,   225,   226,
     226,   226,   227,   227,   227,   228,   228,   228,   228,   228,
     229,   229,   229,   230,   230,   231,   231,   232,   232,   233,
     233,   234,   234,   235,   235,   236,   236,   237,   237,   238,
     238,   238,   238,   238,   238,   238,   238,   238,   238,   238,
     239,   239,   240,   241,   241,   241,   241,   241,   241,   242,
     242,   242,   242,   242,   243,   244,   244,   245,   245,   246,
     247,   247,   247,   248,   248,   248,   248,   249,   249,   249,
     250,   250,   250,   251,   251,   252,   253,   253,   253,   253,
     253,   253,   253,   253,   253,   254,   254,   254,   254,   254,
     254,   254,   254,   254,   254,   255,   255,   256,   256,   257,
     257,   257,   257,   257,   257,   257,   257,   258,   258,   260,
     259,   259,   261,   261,   262,   262,   262,   263,   263,   264,
     264,   265,   265,   265,   265,   265,   265,   265,   265,   265,
     265,   266,   266,   266,   266,   267,   267,   267,   267,   267,
     267,   267,   267,   267,   267,   267,   267,   267,   267,   267,
     267,   267,   267,   267,   267,   267,   267,   267,   267,   267,
     267,   267,   267,   267,   267,   267,   267,   267,   267,   267,
     267,   267,   267,   267,   267,   267,   267,   267,   267,   267,
     267,   267,   267,   267,   267,   267,   267,   267,   267,   267,
     267,   267,   267,   267,   267,   267,   267,   268,   268,   268,
     268,   268,   268,   268,   268,   268,   268,   268,   268,   268,
     268,   268,   268,   268,   268,   268,   268,   268,   268,   268,
     268,   268,   268,   268,   268,   268,   268,   268,   268,   268,
     268,   268,   268,   268,   268,   268,   268,   269,   269,   269,
     270,   270,   270,   271,   271,   271,   271,   271,   273,   272,
     274,   272,   275,   272,   276,   276,   277,   277,   278,   278,
     279,   279,   279,   280,   280,   280,   280,   282,   281,   283,
     281,   284,   284,   285,   285,   286,   287,   287,   287,   287,
     288,   289,   290,   290,   291,   291,   291,   291,   291,   291,
     292,   293,   292,   294,   294,   295,   296,   295,   297,   297,
     298,   298,   300,   299,   299,   301,   301,   302,   302,   302,
     303,   304,   303,   306,   305,   305,   307,   307,   309,   308,
     308,   310,   308,   311,   311,   312,   312,   313,   313,   314,
     314,   314,   314,   314,   315,   315,   316,   316,   318,   319,
     317
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
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     0,     6,
       0,     5,     0,     6,     1,     2,     3,     4,     1,     3,
       1,     4,     2,     2,     3,     6,     5,     0,     6,     0,
       6,     1,     2,     1,     3,     3,     1,     4,     3,     2,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       2,     0,     4,     1,     1,     2,     0,     4,     1,     2,
       1,     2,     0,     6,     5,     1,     2,     1,     3,     2,
       2,     0,     4,     0,     4,     1,     1,     4,     0,     6,
       7,     0,     7,     1,     1,     1,     0,     2,     3,     2,
       2,     2,     3,     2,     1,     2,     1,     1,     0,     0,
       4
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const unsigned short yydefact[] =
{
       0,   176,   171,   251,   248,   249,   273,   255,   256,   257,
     258,   259,   260,   252,   253,   254,   274,   275,   276,   264,
     267,   270,   172,   174,   178,   177,     0,   170,   250,   261,
     262,   263,   265,   266,   268,   269,   271,   272,   277,   280,
     283,   278,   279,   281,   282,   284,   285,   185,   186,   212,
     195,   196,   193,   189,   190,   191,   192,   194,   197,   198,
     213,   199,   200,   201,   202,   214,   203,   204,   205,   206,
     207,   208,   209,   210,   211,   241,   242,   243,   215,   216,
     217,   218,   219,   220,   221,   222,   223,   224,   225,   226,
     227,   228,   229,   230,   231,   232,   233,   234,   236,   238,
     235,   237,   239,   244,   245,   246,   240,   179,   180,     0,
     247,   169,   158,   157,     0,   188,   152,   290,   291,   292,
       0,   153,   293,   294,   295,   296,   297,   387,   388,     0,
     106,   105,     0,   126,   135,     0,   147,   155,   151,   156,
     149,   145,   286,   181,   154,   150,   187,   102,     0,     0,
     384,   386,   173,   175,     0,     0,   300,     0,     0,    99,
       0,   104,     0,   167,   117,   118,   119,   168,   113,   107,
     120,   123,     0,   114,   121,   125,   122,     0,   100,   136,
     143,   103,   148,   146,     0,   184,     0,   313,     1,   385,
     298,   302,     0,     0,   164,   159,   162,   287,   288,   289,
       0,   346,   389,   108,   115,   124,   116,   110,   127,   109,
       0,     0,   139,   317,     2,     6,     4,     7,     5,     0,
       0,     0,   182,    43,    42,    44,    41,     3,     9,    37,
      11,    27,     0,     0,    31,     0,    45,     0,    49,    52,
      55,    60,    63,    65,    67,    69,    71,    73,    75,    92,
       0,    35,     0,     0,     0,   314,     0,     0,     0,     0,
       0,   304,   161,     0,     0,     0,   101,   345,     0,   390,
       0,   112,     0,     0,   130,   138,     0,    45,    77,   330,
     142,     0,     0,     0,    38,    39,    90,     0,     0,    14,
      15,     0,     0,    29,    28,     0,   247,    32,    34,    40,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     183,    97,     0,     0,     0,     0,     0,   310,     0,   308,
       0,   301,   305,   165,   166,   163,   160,     0,     0,     0,
     371,     0,     0,     0,   368,     0,   341,   350,     0,   339,
       0,    18,     0,   331,     0,   135,   334,   348,   333,   332,
       0,   335,   336,   337,   338,     0,     0,   129,     0,   131,
       0,     0,   137,    80,    81,    83,    82,    85,    86,    87,
      88,    89,    84,    79,     0,   144,     0,     0,   321,     0,
       8,     0,    93,     0,     0,    16,    12,    13,    33,    46,
      47,    48,    51,    50,    53,    54,    58,    59,    56,    57,
      61,    62,    64,    66,    68,    70,    72,    74,     0,     0,
      98,   316,     0,   299,   303,     0,   312,     0,   306,     0,
     380,   379,     0,     0,     0,   383,   381,     0,     0,     0,
     340,     0,     0,     0,     0,   351,   347,   349,   111,     0,
     128,   134,   140,     0,    78,   326,     0,   323,   318,   322,
     320,    91,    94,    10,     0,    95,     0,   315,     0,   309,
     307,     0,     0,     0,   382,     0,     0,     0,    25,    17,
      19,    20,   132,     0,   141,     0,   329,     0,   325,    76,
      96,   311,     0,   374,   373,   376,   352,   366,    35,     0,
       0,   342,     0,     0,   133,   328,     0,   324,     0,   375,
       0,     0,     0,     0,     0,   361,   354,     0,     0,    22,
       0,   327,     0,   377,     0,   365,   353,     0,   344,   369,
     343,   360,     0,     0,     0,    26,    21,   370,   378,   372,
     363,   367,     0,     0,   357,     0,   355,    24,    23,     0,
       0,   359,   362,   356,   364,   358
};

/* YYDEFGOTO[NTERM-NUM]. */
static const short yydefgoto[] =
{
      -1,   227,   228,   229,   394,   349,   350,   480,   518,   519,
     351,   230,   231,   232,   233,   234,   235,   277,   237,   238,
     239,   240,   241,   242,   243,   244,   245,   246,   247,   248,
     278,   286,   384,   352,   250,   185,   353,   354,   129,   130,
     131,   168,   169,   170,   171,   172,   173,   132,   133,   251,
     252,   136,   137,   138,   265,   195,   196,   174,   139,   140,
     141,   142,   143,   200,   144,   145,   146,   256,   192,   257,
     260,   261,   328,   329,   147,   148,   282,   283,   387,   456,
     388,   457,   280,   356,   357,   358,   359,   441,   529,   530,
     268,   360,   361,   362,   512,   545,   546,   516,   532,   526,
     549,   499,   363,   438,   433,   495,   510,   511,   364,   149,
     150,   151,   160,   269
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -464
static const short yypact[] =
{
    5079,  -464,  -464,  -464,  -464,  -464,  -464,  -464,  -464,  -464,
    -464,  -464,  -464,  -464,  -464,  -464,  -464,  -464,  -464,  -464,
    -464,  -464,  -464,  -464,  -464,  -464,   127,  -464,  -464,  -464,
    -464,  -464,  -464,  -464,  -464,  -464,  -464,  -464,  -464,  -464,
    -464,  -464,  -464,  -464,  -464,  -464,  -464,  -464,  -464,  -464,
    -464,  -464,  -464,  -464,  -464,  -464,  -464,  -464,  -464,  -464,
    -464,  -464,  -464,  -464,  -464,  -464,  -464,  -464,  -464,  -464,
    -464,  -464,  -464,  -464,  -464,  -464,  -464,  -464,  -464,  -464,
    -464,  -464,  -464,  -464,  -464,  -464,  -464,  -464,  -464,  -464,
    -464,  -464,  -464,  -464,  -464,  -464,  -464,  -464,  -464,  -464,
    -464,  -464,  -464,  -464,  -464,  -464,  -464,  -464,  -464,   -98,
    -464,  -464,  -464,  -464,  -155,  -464,  -464,  -464,  -464,  -464,
    -118,  -464,  -464,  -464,  -464,  -464,  -464,  -464,  -142,   -90,
     -85,  5432,  -166,  -464,   -41,  4041,  -464,  -464,  -464,  -464,
    -464,  -464,  -464,   -66,  -464,  -464,  -464,  -464,  -130,  4902,
    -464,  -464,  -464,  -464,   -75,   -20,  -464,    13,  5818,  -464,
      -7,  -464,  5432,  -464,  -464,  -464,  -464,  -464,  -464,  -464,
    -464,  -464,  5432,  -464,  -464,    17,  -464,    43,  -464,  -133,
       3,  -464,  -464,  -464,  1878,   -37,  -162,  -464,  -464,  -464,
    -464,  -464,  5569,    22,    -4,    19,  -464,  -464,  -464,  -464,
      18,    24,  -464,  -464,  -464,  -464,  -464,    25,  -119,  -464,
    2059,  3685,  -110,   209,    33,  -464,  -464,  -464,  -464,  3685,
    3685,  3685,    32,  -464,  -464,  -464,  -464,  -464,  -464,  -115,
    -464,  -464,    38,   -77,  3863,    40,  -464,  3685,   -21,   -27,
      34,  -140,    36,    20,    14,    21,    68,    67,  -138,  -464,
      42,  -464,  5256,  2240,  2421,  -464,  5569,  5569,    86,  5706,
    4393,  -464,  -464,    35,    88,    48,  -464,  -464,  1316,  -464,
    2602,   -37,  2783,  3685,  -101,  -100,    50,   134,  -464,  -464,
    -464,  3685,  5256,   233,  -464,  -464,  -464,   -69,  2964,  -464,
    -464,  3685,  -124,  -464,  -464,  3685,    55,  -464,  -464,  -464,
    3685,  3685,  3685,  3685,  3685,  3685,  3685,  3685,  3685,  3685,
    3685,  3685,  3685,  3685,  3685,  3685,  3685,  3685,  3685,  3685,
      56,  -464,    57,    51,    60,  4561,  4729,    62,  -121,  -464,
      86,  -464,  -464,  -464,  -464,  -464,  -464,    59,    63,  1316,
    -464,    70,    64,  3145,  -464,    72,    71,  -464,    78,  -464,
     -68,  -464,   -73,  -464,  -142,  -125,  -464,  -464,  -464,  -464,
     932,  -464,  -464,  -464,  -464,    32,    76,   -88,    77,  -464,
    3685,  3685,   -86,  -464,  -464,  -464,  -464,  -464,  -464,  -464,
    -464,  -464,  -464,  -464,  3685,  -464,   107,  4218,  -464,    79,
    -464,  3685,  -464,    80,    82,    74,  -464,  -464,  -464,  -464,
    -464,  -464,   -21,   -21,   -27,   -27,    34,    34,    34,    34,
    -140,  -140,    36,    20,    14,    21,    68,    67,     4,  3323,
    -464,  -464,    83,  -464,  -464,  2602,   -37,    86,  -464,   -44,
    -464,  -464,   141,    90,  3685,  -464,  -464,   -35,    92,  3685,
    -464,  1316,   135,    84,  3685,  -464,  -464,  -464,    56,  3685,
     -82,  -464,  -464,  3685,  -464,    94,   -31,  -464,  -464,  -464,
    -464,  -464,  -464,  -464,  3685,  -464,    95,  -464,    97,  -464,
    -464,   101,  1700,   -40,  -464,  3685,   102,  1124,    81,  -464,
    -464,    85,  -464,  3685,  -464,  3504,   -37,   107,  -464,  -464,
    -464,    56,  3685,  -464,  -464,  3685,  -464,    74,   144,   106,
     105,  -464,   162,   162,  -464,    32,   120,  -464,   -39,  -464,
     114,   125,  1316,   118,  1508,   124,  -464,   122,  -158,  -464,
    -149,    56,   121,  3685,  1508,   300,  -464,  3685,  -464,  -464,
    -464,  -464,   740,   173,   162,  -464,  -464,  -464,    74,  -464,
    -464,  -464,  3685,   126,  -464,   548,  -464,  -464,  -464,  1316,
     128,  -464,  -464,  -464,  -464,  -464
};

/* YYPGOTO[NTERM-NUM].  */
static const short yypgoto[] =
{
    -464,  -464,  -464,  -464,  -113,  -464,  -464,  -464,  -188,  -216,
    -464,  -464,  -464,  -464,  -464,  -464,  -464,  -180,  -464,  -109,
    -108,  -139,  -107,     7,    16,    12,    15,    11,    23,  -464,
    -177,  -198,  -464,  -204,  -208,  -176,     5,     9,  -464,  -464,
    -464,   160,   171,  -464,   163,  -464,   164,  -464,  -464,     0,
       1,  -112,  -464,  -464,  -464,  -464,    73,  -464,  -464,  -464,
      65,   180,  -464,  -464,   -94,  -464,  -464,  -464,  -464,  -464,
     -55,  -241,    26,   -84,  -464,  -464,  -464,  -464,  -464,  -464,
     -45,  -143,  -265,  -127,  -333,  -463,  -464,  -464,  -178,   187,
    -464,   -93,  -123,  -464,  -464,  -464,  -195,  -464,  -464,  -464,
    -464,  -144,  -464,  -464,  -464,  -464,  -464,  -464,  -464,  -464,
     203,  -464,  -464,  -464
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -320
static const short yytable[] =
{
     134,   135,   276,   212,   236,   127,   432,   249,   369,   128,
     186,   307,   308,   279,   193,   179,   385,   287,   318,   332,
     254,   177,   396,   182,   178,   157,   158,   447,   255,   534,
     236,   271,   274,   249,   289,   290,   297,   176,   534,   284,
     285,   535,   154,   155,   397,   322,   324,   209,   159,   210,
     536,   528,   117,   118,   119,   -35,   211,   299,   309,   310,
     187,   528,   366,   272,   368,   319,   427,   291,   176,   428,
     273,   292,   253,   236,   236,   279,   249,   249,   176,   281,
     393,   253,   288,   279,   332,   332,   156,   395,   370,   371,
     236,   161,   236,   249,   288,   249,   419,   398,   259,   179,
     419,   449,   162,   453,   294,   451,   452,   483,   236,   190,
     295,   249,   390,   443,   391,   418,   184,   445,   391,   444,
     399,   400,   401,   236,   236,   236,   236,   236,   236,   236,
     236,   236,   236,   236,   236,   236,   236,   236,   236,   437,
     182,   496,   522,   427,   447,   253,   470,   391,   391,   134,
     135,   426,   391,   194,   127,   474,   487,   207,   128,   488,
     152,   153,   259,   259,   191,   303,   259,   304,   406,   407,
     408,   409,   279,   279,   300,   301,   302,   201,   333,   525,
     334,   305,   306,   208,   482,   263,   454,   213,   484,   311,
     312,   391,   464,   461,   402,   403,   175,   404,   405,   544,
     183,   325,   326,   262,   410,   411,   264,   270,   266,   267,
    -319,   466,   544,   -36,   288,   314,   554,   468,   504,   293,
     298,   315,   313,   316,   317,   320,   327,   175,   194,   336,
     473,   259,   259,   372,   389,   395,   -30,   175,   419,   236,
     420,   421,   249,   422,   425,   236,   481,   455,   249,   430,
     434,   279,   439,   431,   435,   279,   440,   258,   442,   448,
     450,   391,   541,   462,   460,   463,   489,   471,   355,   135,
     472,   497,   475,   467,   479,   478,   485,   506,   490,   486,
     491,   492,   386,   500,   513,   279,   502,   514,   508,   515,
     503,   497,   373,   374,   375,   376,   377,   378,   379,   380,
     381,   382,   517,   521,   523,   236,   524,   527,   249,   531,
     533,   537,   540,   547,   551,   520,   555,   183,   548,   538,
     412,   258,   258,   383,   330,   258,   476,   414,   416,   279,
     413,   415,   204,   203,   550,   205,   206,   335,   199,   355,
     135,   417,   459,   469,   507,   493,   539,   202,   477,   494,
     553,   509,   189,     0,     0,     0,   429,     0,     0,     0,
     355,   135,   236,     0,     0,   249,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   386,     0,     0,
     258,   258,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   355,   135,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   355,   135,     0,   498,     0,   355,   135,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   498,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   355,   135,   355,   135,     0,     0,     0,     0,
       0,     0,     0,     0,   355,   135,     0,     0,     0,     0,
       0,     0,   355,   135,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   355,   135,     0,     0,   355,
     135,     1,     2,     3,     4,     5,     6,   337,   338,   339,
       0,   340,   341,   342,   343,     0,     7,     8,     9,    10,
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
     107,   108,   109,   110,   344,     0,     0,     0,     0,     0,
     345,   542,   543,   111,   112,   113,   114,     0,   214,   115,
     215,   216,   217,   218,     0,     0,     0,   219,   220,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   116,   117,   118,
     119,   120,   121,   122,   123,   124,   125,   126,   221,     0,
       0,     0,   346,   552,     0,     0,     0,     0,   347,   223,
     224,   225,   226,     1,     2,     3,     4,     5,     6,   337,
     338,   339,   348,   340,   341,   342,   343,     0,     7,     8,
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
       0,   106,   107,   108,   109,   110,   344,     0,     0,     0,
       0,     0,   345,   542,   543,   111,   112,   113,   114,     0,
     214,   115,   215,   216,   217,   218,     0,     0,     0,   219,
     220,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   116,
     117,   118,   119,   120,   121,   122,   123,   124,   125,   126,
     221,     0,     0,     0,   346,     0,     0,     0,     0,     0,
     347,   223,   224,   225,   226,     1,     2,     3,     4,     5,
       6,   337,   338,   339,   348,   340,   341,   342,   343,     0,
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
       0,     0,     0,   106,   107,   108,   109,   110,   344,     0,
       0,     0,     0,     0,   345,     0,     0,   111,   112,   113,
     114,     0,   214,   115,   215,   216,   217,   218,     0,     0,
       0,   219,   220,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   116,   117,   118,   119,   120,   121,   122,   123,   124,
     125,   126,   221,     0,     0,     0,   346,   446,     0,     0,
       0,     0,   347,   223,   224,   225,   226,     1,     2,     3,
       4,     5,     6,   337,   338,   339,   348,   340,   341,   342,
     343,     0,     7,     8,     9,    10,    11,    12,    13,    14,
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
     344,     0,     0,     0,     0,     0,   345,     0,     0,   111,
     112,   113,   114,     0,   214,   115,   215,   216,   217,   218,
       0,     0,     0,   219,   220,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   116,   117,   118,   119,   120,   121,   122,
     123,   124,   125,   126,   221,     0,     0,     0,   346,   501,
       0,     0,     0,     0,   347,   223,   224,   225,   226,     1,
       2,     3,     4,     5,     6,   337,   338,   339,   348,   340,
     341,   342,   343,     0,     7,     8,     9,    10,    11,    12,
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
     109,   110,   344,     0,     0,     0,     0,     0,   345,     0,
       0,   111,   112,   113,   114,     0,   214,   115,   215,   216,
     217,   218,     0,     0,     0,   219,   220,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   116,   117,   118,   119,   120,
     121,   122,   123,   124,   125,   126,   221,     0,     0,     0,
     346,     0,     0,     0,     0,     0,   347,   223,   224,   225,
     226,     1,     2,     3,     4,     5,     6,   337,   338,   339,
     348,   340,   341,   342,   343,     0,     7,     8,     9,    10,
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
     107,   108,   109,   110,   344,     0,     0,     0,     0,     0,
     345,     0,     0,   111,   112,   113,   114,     0,   214,   115,
     215,   216,   217,   218,     0,     0,     0,   219,   220,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   116,   117,   118,
     119,   120,   121,   122,   123,   124,   125,   126,   221,     0,
       0,     0,   201,     0,     0,     0,     0,     0,   347,   223,
     224,   225,   226,     1,     2,     3,     4,     5,     6,     0,
       0,     0,   348,     0,     0,     0,     0,     0,     7,     8,
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
       0,     0,     0,     0,     0,   111,   112,   113,   114,     0,
     214,   115,   215,   216,   217,   218,     0,     0,     0,   219,
     220,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   116,
     117,   118,   119,   120,   121,   122,   123,   124,   125,   126,
     221,     1,     2,     3,     4,     5,     6,     0,     0,     0,
     347,   223,   224,   225,   226,     0,     7,     8,     9,    10,
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
       0,     0,     0,   111,   112,   113,   114,     0,   214,   115,
     215,   216,   217,   218,     0,     0,     0,   219,   220,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   116,   117,   118,
     119,     0,   121,   122,   123,   124,   125,   126,   221,     0,
       0,   222,     1,     2,     3,     4,     5,     6,     0,   223,
     224,   225,   226,     0,     0,     0,     0,     7,     8,     9,
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
       0,     0,     0,     0,   111,   112,   113,   114,     0,   214,
     115,   215,   216,   217,   218,     0,     0,     0,   219,   220,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   116,   117,
     118,   119,     0,   121,   122,   123,   124,   125,   126,   221,
       0,     0,   275,     1,     2,     3,     4,     5,     6,     0,
     223,   224,   225,   226,     0,     0,     0,     0,     7,     8,
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
       0,     0,     0,     0,     0,   111,   112,   113,   114,     0,
     214,   115,   215,   216,   217,   218,     0,     0,     0,   219,
     220,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   116,
     117,   118,   119,     0,   121,   122,   123,   124,   125,   126,
     221,     0,     0,   321,     1,     2,     3,     4,     5,     6,
       0,   223,   224,   225,   226,     0,     0,     0,     0,     7,
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
       0,   214,   115,   215,   216,   217,   218,     0,     0,     0,
     219,   220,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     116,   117,   118,   119,     0,   121,   122,   123,   124,   125,
     126,   221,     0,     0,   323,     1,     2,     3,     4,     5,
       6,     0,   223,   224,   225,   226,     0,     0,     0,     0,
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
     114,     0,   214,   115,   215,   216,   217,   218,     0,     0,
       0,   219,   220,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   116,   117,   118,   119,     0,   121,   122,   123,   124,
     125,   126,   221,     0,     0,   365,     1,     2,     3,     4,
       5,     6,     0,   223,   224,   225,   226,     0,     0,     0,
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
     113,   114,     0,   214,   115,   215,   216,   217,   218,     0,
       0,     0,   219,   220,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   116,   117,   118,   119,     0,   121,   122,   123,
     124,   125,   126,   221,     0,     0,   367,     1,     2,     3,
       4,     5,     6,     0,   223,   224,   225,   226,     0,     0,
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
     112,   113,   114,     0,   214,   115,   215,   216,   217,   218,
       0,     0,     0,   219,   220,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   116,   117,   118,   119,     0,   121,   122,
     123,   124,   125,   126,   221,     0,     0,   392,     1,     2,
       3,     4,     5,     6,     0,   223,   224,   225,   226,     0,
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
     111,   112,   113,   114,     0,   214,   115,   215,   216,   217,
     218,     0,     0,     0,   219,   220,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   116,   117,   118,   119,     0,   121,
     122,   123,   124,   125,   126,   221,     1,     2,     3,     4,
       5,     6,     0,     0,     0,   436,   223,   224,   225,   226,
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
     113,   114,     0,   214,   115,   215,   216,   217,   218,     0,
       0,     0,   219,   220,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   116,   117,   118,   119,     0,   121,   122,   123,
     124,   125,   126,   221,     0,     0,   465,     1,     2,     3,
       4,     5,     6,     0,   223,   224,   225,   226,     0,     0,
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
     112,   113,   114,     0,   214,   115,   215,   216,   217,   218,
       0,     0,     0,   219,   220,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   116,   117,   118,   119,     0,   121,   122,
     123,   124,   125,   126,   221,     0,     0,   505,     1,     2,
       3,     4,     5,     6,     0,   223,   224,   225,   226,     0,
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
     111,   112,   113,   114,     0,   214,   115,   215,   216,   217,
     218,     0,     0,     0,   219,   220,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   116,   117,   118,   119,     0,   121,
     122,   123,   124,   125,   126,   221,     1,     2,     3,     4,
       5,     6,     0,     0,     0,     0,   223,   224,   225,   226,
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
     105,     0,     0,     0,   106,   107,   108,   109,   296,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   111,   112,
     113,   114,     0,   214,   115,   215,   216,   217,   218,     0,
       0,     0,   219,   220,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   116,   117,   118,   119,     0,   121,   122,   123,
     124,   125,   126,   221,     1,     2,     3,     4,     5,     6,
       0,     0,     0,     0,   223,   224,   225,   226,     0,     7,
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
       0,   180,   115,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     116,   117,   118,   119,     0,   121,   122,   123,   124,   125,
     126,     1,     2,     3,     4,     5,     6,     0,     0,     0,
       0,   181,     0,     0,     0,     0,     7,     8,     9,    10,
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
       0,     0,     0,   111,   112,   113,   114,     0,     0,   115,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   116,   117,   118,
     119,     0,   121,   122,   123,   124,   125,   126,     3,     4,
       5,     6,     0,   458,     0,     0,     0,     0,     0,     0,
       0,     7,     8,     9,    10,    11,    12,    13,    14,    15,
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
       0,     0,     0,     0,   115,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   117,   118,   119,     3,     4,     5,     6,
       0,     0,     0,     0,     0,     0,     0,     0,   331,     7,
       8,     9,    10,    11,    12,    13,    14,    15,    16,    17,
      18,    19,    20,    21,     0,     0,     0,     0,     0,     0,
       0,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    71,    72,    73,    74,    75,    76,
      77,    78,    79,    80,    81,    82,    83,    84,    85,    86,
      87,    88,    89,    90,    91,    92,    93,    94,    95,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,     0,
       0,     0,   106,     0,     0,   109,   110,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   115,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   117,   118,   119,     3,     4,     5,     6,     0,     0,
       0,     0,     0,     0,     0,     0,   423,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,     0,     0,     0,     0,     0,     0,     0,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    73,    74,    75,    76,    77,    78,
      79,    80,    81,    82,    83,    84,    85,    86,    87,    88,
      89,    90,    91,    92,    93,    94,    95,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,     0,     0,     0,
     106,     0,     0,   109,   110,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     115,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   117,
     118,   119,   188,     0,     0,     1,     2,     3,     4,     5,
       6,     0,     0,     0,   424,     0,     0,     0,     0,     0,
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
     114,     0,     0,   115,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   116,   117,   118,   119,   120,   121,   122,   123,   124,
     125,   126,     1,     2,     3,     4,     5,     6,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     7,     8,     9,
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
       0,     0,     0,     0,   111,   112,   113,   114,     0,     0,
     115,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   116,   117,
     118,   119,   120,   121,   122,   123,   124,   125,   126,     1,
       2,     3,     4,     5,     6,     0,     0,     0,     0,     0,
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
       0,   111,   112,   113,   114,     0,     0,   115,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   116,   117,   118,   119,     0,
     121,   122,   123,   124,   125,   126,   163,     3,     4,     5,
       6,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
      17,    18,    19,    20,    21,   164,   165,   166,     0,     0,
       0,     0,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    70,    71,    72,    73,    74,    75,
      76,    77,    78,    79,    80,    81,    82,    83,    84,    85,
      86,    87,    88,    89,    90,    91,    92,    93,    94,    95,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
       0,     0,     0,   106,     0,     0,   109,   110,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   115,     3,     4,     5,     6,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,   117,   118,   119,     0,   167,     0,     0,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    73,    74,    75,    76,    77,    78,
      79,    80,    81,    82,    83,    84,    85,    86,    87,    88,
      89,    90,    91,    92,    93,    94,    95,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,     0,     0,     0,
     106,     0,     0,   109,   110,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     115,     3,     4,     5,     6,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     7,     8,     9,    10,    11,    12,
      13,    14,    15,    16,    17,    18,    19,    20,    21,   117,
     118,   119,     0,     0,     0,     0,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    94,    95,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   197,   198,     0,   106,     0,     0,
     109,   110,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   115,     0,     0,
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
       0,     0,     0,     0,     0,     0,     0,     0,     0,   115
};

static const short yycheck[] =
{
       0,     0,   210,   179,   184,     0,   339,   184,   273,     0,
     140,   151,   152,   211,     1,   140,   281,   221,   156,   260,
     182,   187,   146,   135,   190,   180,   120,   360,   190,   187,
     210,   207,   208,   210,   149,   150,   234,   131,   187,   219,
     220,   199,   140,   141,   168,   253,   254,   180,   190,   182,
     199,   514,   170,   171,   172,   180,   189,   237,   198,   199,
     190,   524,   270,   182,   272,   203,   187,   182,   162,   190,
     189,   186,   182,   253,   254,   273,   253,   254,   172,   189,
     288,   182,   182,   281,   325,   326,   184,   291,   189,   189,
     270,   181,   272,   270,   182,   272,   182,   295,   192,   140,
     182,   189,   187,   189,   181,   370,   371,   189,   288,   184,
     187,   288,   181,   181,   187,   319,   182,   190,   187,   187,
     300,   301,   302,   303,   304,   305,   306,   307,   308,   309,
     310,   311,   312,   313,   314,   315,   316,   317,   318,   343,
     252,   181,   181,   187,   477,   182,   190,   187,   187,   149,
     149,   327,   187,   140,   149,   190,   187,   140,   149,   190,
      33,    34,   256,   257,   184,   192,   260,   194,   307,   308,
     309,   310,   370,   371,   195,   196,   197,   184,   143,   512,
     145,   147,   148,   140,   449,   189,   384,   184,   453,   153,
     154,   187,   188,   391,   303,   304,   131,   305,   306,   532,
     135,   256,   257,   181,   311,   312,   187,   182,   190,   185,
       1,   419,   545,   180,   182,   201,   549,   425,   483,   181,
     180,   200,   202,   155,   157,   183,   140,   162,   140,   181,
     434,   325,   326,   183,     1,   439,   181,   172,   182,   419,
     183,   190,   419,   183,   182,   425,   444,   140,   425,   190,
     180,   449,   180,   190,   190,   453,   185,   192,   180,   183,
     183,   187,   527,   183,   185,   183,   464,   126,   268,   268,
     180,   475,   180,   190,   190,   140,   182,   485,   183,   455,
     183,   180,   282,   181,   140,   483,   205,   181,   492,   184,
     205,   495,   158,   159,   160,   161,   162,   163,   164,   165,
     166,   167,   140,   183,   190,   485,   181,   189,   485,   185,
     188,   190,    12,   140,   188,   503,   188,   252,   534,   523,
     313,   256,   257,   189,   259,   260,   439,   315,   317,   527,
     314,   316,   172,   162,   542,   172,   172,   264,   158,   339,
     339,   318,   387,   427,   487,   472,   524,   160,   441,   472,
     545,   495,   149,    -1,    -1,    -1,   330,    -1,    -1,    -1,
     360,   360,   542,    -1,    -1,   542,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   387,    -1,    -1,
     325,   326,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   441,   441,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   472,   472,    -1,   475,    -1,   477,   477,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   495,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   512,   512,   514,   514,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   524,   524,    -1,    -1,    -1,    -1,
      -1,    -1,   532,   532,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   545,   545,    -1,    -1,   549,
     549,     3,     4,     5,     6,     7,     8,     9,    10,    11,
      -1,    13,    14,    15,    16,    -1,    18,    19,    20,    21,
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
     132,   133,   134,   135,   136,   137,   138,    -1,   140,   141,
     142,   143,   144,   145,    -1,    -1,    -1,   149,   150,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   169,   170,   171,
     172,   173,   174,   175,   176,   177,   178,   179,   180,    -1,
      -1,    -1,   184,   185,    -1,    -1,    -1,    -1,   190,   191,
     192,   193,   194,     3,     4,     5,     6,     7,     8,     9,
      10,    11,   204,    13,    14,    15,    16,    -1,    18,    19,
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
      -1,   121,   122,   123,   124,   125,   126,    -1,    -1,    -1,
      -1,    -1,   132,   133,   134,   135,   136,   137,   138,    -1,
     140,   141,   142,   143,   144,   145,    -1,    -1,    -1,   149,
     150,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   169,
     170,   171,   172,   173,   174,   175,   176,   177,   178,   179,
     180,    -1,    -1,    -1,   184,    -1,    -1,    -1,    -1,    -1,
     190,   191,   192,   193,   194,     3,     4,     5,     6,     7,
       8,     9,    10,    11,   204,    13,    14,    15,    16,    -1,
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
      -1,    -1,    -1,    -1,   132,    -1,    -1,   135,   136,   137,
     138,    -1,   140,   141,   142,   143,   144,   145,    -1,    -1,
      -1,   149,   150,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   169,   170,   171,   172,   173,   174,   175,   176,   177,
     178,   179,   180,    -1,    -1,    -1,   184,   185,    -1,    -1,
      -1,    -1,   190,   191,   192,   193,   194,     3,     4,     5,
       6,     7,     8,     9,    10,    11,   204,    13,    14,    15,
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
     136,   137,   138,    -1,   140,   141,   142,   143,   144,   145,
      -1,    -1,    -1,   149,   150,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   169,   170,   171,   172,   173,   174,   175,
     176,   177,   178,   179,   180,    -1,    -1,    -1,   184,   185,
      -1,    -1,    -1,    -1,   190,   191,   192,   193,   194,     3,
       4,     5,     6,     7,     8,     9,    10,    11,   204,    13,
      14,    15,    16,    -1,    18,    19,    20,    21,    22,    23,
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
     124,   125,   126,    -1,    -1,    -1,    -1,    -1,   132,    -1,
      -1,   135,   136,   137,   138,    -1,   140,   141,   142,   143,
     144,   145,    -1,    -1,    -1,   149,   150,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   169,   170,   171,   172,   173,
     174,   175,   176,   177,   178,   179,   180,    -1,    -1,    -1,
     184,    -1,    -1,    -1,    -1,    -1,   190,   191,   192,   193,
     194,     3,     4,     5,     6,     7,     8,     9,    10,    11,
     204,    13,    14,    15,    16,    -1,    18,    19,    20,    21,
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
     132,    -1,    -1,   135,   136,   137,   138,    -1,   140,   141,
     142,   143,   144,   145,    -1,    -1,    -1,   149,   150,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   169,   170,   171,
     172,   173,   174,   175,   176,   177,   178,   179,   180,    -1,
      -1,    -1,   184,    -1,    -1,    -1,    -1,    -1,   190,   191,
     192,   193,   194,     3,     4,     5,     6,     7,     8,    -1,
      -1,    -1,   204,    -1,    -1,    -1,    -1,    -1,    18,    19,
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
      -1,    -1,    -1,    -1,    -1,   135,   136,   137,   138,    -1,
     140,   141,   142,   143,   144,   145,    -1,    -1,    -1,   149,
     150,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   169,
     170,   171,   172,   173,   174,   175,   176,   177,   178,   179,
     180,     3,     4,     5,     6,     7,     8,    -1,    -1,    -1,
     190,   191,   192,   193,   194,    -1,    18,    19,    20,    21,
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
      -1,    -1,    -1,   135,   136,   137,   138,    -1,   140,   141,
     142,   143,   144,   145,    -1,    -1,    -1,   149,   150,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   169,   170,   171,
     172,    -1,   174,   175,   176,   177,   178,   179,   180,    -1,
      -1,   183,     3,     4,     5,     6,     7,     8,    -1,   191,
     192,   193,   194,    -1,    -1,    -1,    -1,    18,    19,    20,
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
      -1,    -1,    -1,    -1,   135,   136,   137,   138,    -1,   140,
     141,   142,   143,   144,   145,    -1,    -1,    -1,   149,   150,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   169,   170,
     171,   172,    -1,   174,   175,   176,   177,   178,   179,   180,
      -1,    -1,   183,     3,     4,     5,     6,     7,     8,    -1,
     191,   192,   193,   194,    -1,    -1,    -1,    -1,    18,    19,
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
      -1,    -1,    -1,    -1,    -1,   135,   136,   137,   138,    -1,
     140,   141,   142,   143,   144,   145,    -1,    -1,    -1,   149,
     150,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   169,
     170,   171,   172,    -1,   174,   175,   176,   177,   178,   179,
     180,    -1,    -1,   183,     3,     4,     5,     6,     7,     8,
      -1,   191,   192,   193,   194,    -1,    -1,    -1,    -1,    18,
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
      -1,   140,   141,   142,   143,   144,   145,    -1,    -1,    -1,
     149,   150,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     169,   170,   171,   172,    -1,   174,   175,   176,   177,   178,
     179,   180,    -1,    -1,   183,     3,     4,     5,     6,     7,
       8,    -1,   191,   192,   193,   194,    -1,    -1,    -1,    -1,
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
     138,    -1,   140,   141,   142,   143,   144,   145,    -1,    -1,
      -1,   149,   150,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   169,   170,   171,   172,    -1,   174,   175,   176,   177,
     178,   179,   180,    -1,    -1,   183,     3,     4,     5,     6,
       7,     8,    -1,   191,   192,   193,   194,    -1,    -1,    -1,
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
     137,   138,    -1,   140,   141,   142,   143,   144,   145,    -1,
      -1,    -1,   149,   150,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   169,   170,   171,   172,    -1,   174,   175,   176,
     177,   178,   179,   180,    -1,    -1,   183,     3,     4,     5,
       6,     7,     8,    -1,   191,   192,   193,   194,    -1,    -1,
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
     136,   137,   138,    -1,   140,   141,   142,   143,   144,   145,
      -1,    -1,    -1,   149,   150,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   169,   170,   171,   172,    -1,   174,   175,
     176,   177,   178,   179,   180,    -1,    -1,   183,     3,     4,
       5,     6,     7,     8,    -1,   191,   192,   193,   194,    -1,
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
     135,   136,   137,   138,    -1,   140,   141,   142,   143,   144,
     145,    -1,    -1,    -1,   149,   150,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   169,   170,   171,   172,    -1,   174,
     175,   176,   177,   178,   179,   180,     3,     4,     5,     6,
       7,     8,    -1,    -1,    -1,   190,   191,   192,   193,   194,
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
     137,   138,    -1,   140,   141,   142,   143,   144,   145,    -1,
      -1,    -1,   149,   150,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   169,   170,   171,   172,    -1,   174,   175,   176,
     177,   178,   179,   180,    -1,    -1,   183,     3,     4,     5,
       6,     7,     8,    -1,   191,   192,   193,   194,    -1,    -1,
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
     136,   137,   138,    -1,   140,   141,   142,   143,   144,   145,
      -1,    -1,    -1,   149,   150,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   169,   170,   171,   172,    -1,   174,   175,
     176,   177,   178,   179,   180,    -1,    -1,   183,     3,     4,
       5,     6,     7,     8,    -1,   191,   192,   193,   194,    -1,
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
     135,   136,   137,   138,    -1,   140,   141,   142,   143,   144,
     145,    -1,    -1,    -1,   149,   150,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   169,   170,   171,   172,    -1,   174,
     175,   176,   177,   178,   179,   180,     3,     4,     5,     6,
       7,     8,    -1,    -1,    -1,    -1,   191,   192,   193,   194,
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
     137,   138,    -1,   140,   141,   142,   143,   144,   145,    -1,
      -1,    -1,   149,   150,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   169,   170,   171,   172,    -1,   174,   175,   176,
     177,   178,   179,   180,     3,     4,     5,     6,     7,     8,
      -1,    -1,    -1,    -1,   191,   192,   193,   194,    -1,    18,
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
      -1,   140,   141,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     169,   170,   171,   172,    -1,   174,   175,   176,   177,   178,
     179,     3,     4,     5,     6,     7,     8,    -1,    -1,    -1,
      -1,   190,    -1,    -1,    -1,    -1,    18,    19,    20,    21,
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
      -1,    -1,    -1,   135,   136,   137,   138,    -1,    -1,   141,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   169,   170,   171,
     172,    -1,   174,   175,   176,   177,   178,   179,     5,     6,
       7,     8,    -1,   185,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    18,    19,    20,    21,    22,    23,    24,    25,    26,
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
      -1,    -1,    -1,    -1,   141,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   170,   171,   172,     5,     6,     7,     8,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   185,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    73,    74,    75,    76,    77,    78,
      79,    80,    81,    82,    83,    84,    85,    86,    87,    88,
      89,    90,    91,    92,    93,    94,    95,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   113,   114,   115,   116,   117,    -1,
      -1,    -1,   121,    -1,    -1,   124,   125,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   141,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   170,   171,   172,     5,     6,     7,     8,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   185,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      81,    82,    83,    84,    85,    86,    87,    88,    89,    90,
      91,    92,    93,    94,    95,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,   114,   115,   116,   117,    -1,    -1,    -1,
     121,    -1,    -1,   124,   125,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     141,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   170,
     171,   172,     0,    -1,    -1,     3,     4,     5,     6,     7,
       8,    -1,    -1,    -1,   185,    -1,    -1,    -1,    -1,    -1,
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
     138,    -1,    -1,   141,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   169,   170,   171,   172,   173,   174,   175,   176,   177,
     178,   179,     3,     4,     5,     6,     7,     8,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    18,    19,    20,
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
      -1,    -1,    -1,    -1,   135,   136,   137,   138,    -1,    -1,
     141,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   169,   170,
     171,   172,   173,   174,   175,   176,   177,   178,   179,     3,
       4,     5,     6,     7,     8,    -1,    -1,    -1,    -1,    -1,
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
      -1,   135,   136,   137,   138,    -1,    -1,   141,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   169,   170,   171,   172,    -1,
     174,   175,   176,   177,   178,   179,     4,     5,     6,     7,
       8,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    -1,    -1,
      -1,    -1,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    71,    72,    73,    74,    75,    76,    77,
      78,    79,    80,    81,    82,    83,    84,    85,    86,    87,
      88,    89,    90,    91,    92,    93,    94,    95,    96,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,   114,   115,   116,   117,
      -1,    -1,    -1,   121,    -1,    -1,   124,   125,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   141,     5,     6,     7,     8,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,   170,   171,   172,    -1,   174,    -1,    -1,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      81,    82,    83,    84,    85,    86,    87,    88,    89,    90,
      91,    92,    93,    94,    95,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,   114,   115,   116,   117,    -1,    -1,    -1,
     121,    -1,    -1,   124,   125,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     141,     5,     6,     7,     8,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,   170,
     171,   172,    -1,    -1,    -1,    -1,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    83,
      84,    85,    86,    87,    88,    89,    90,    91,    92,    93,
      94,    95,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     114,   115,   116,   117,     6,     7,    -1,   121,    -1,    -1,
     124,   125,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   141,    -1,    -1,
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
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   141
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
     125,   135,   136,   137,   138,   141,   169,   170,   171,   172,
     173,   174,   175,   176,   177,   178,   179,   242,   243,   244,
     245,   246,   253,   254,   255,   256,   257,   258,   259,   264,
     265,   266,   267,   268,   270,   271,   272,   280,   281,   315,
     316,   317,    33,    34,   140,   141,   184,   180,   270,   190,
     318,   181,   187,     4,    33,    34,    35,   174,   247,   248,
     249,   250,   251,   252,   263,   266,   270,   187,   190,   140,
     140,   190,   257,   266,   182,   241,   140,   190,     0,   316,
     184,   184,   274,     1,   140,   261,   262,     6,     7,   267,
     269,   184,   295,   248,   247,   250,   252,   140,   140,   180,
     182,   189,   241,   184,   140,   142,   143,   144,   145,   149,
     150,   180,   183,   191,   192,   193,   194,   207,   208,   209,
     217,   218,   219,   220,   221,   222,   223,   224,   225,   226,
     227,   228,   229,   230,   231,   232,   233,   234,   235,   236,
     240,   255,   256,   182,   182,   190,   273,   275,   266,   270,
     276,   277,   181,   189,   187,   260,   190,   185,   296,   319,
     182,   241,   182,   189,   241,   183,   240,   223,   236,   237,
     288,   189,   282,   283,   223,   223,   237,   239,   182,   149,
     150,   182,   186,   181,   181,   187,   125,   237,   180,   223,
     195,   196,   197,   192,   194,   147,   148,   151,   152,   198,
     199,   153,   154,   202,   201,   200,   155,   157,   156,   203,
     183,   183,   240,   183,   240,   276,   276,   140,   278,   279,
     266,   185,   277,   143,   145,   262,   181,     9,    10,    11,
      13,    14,    15,    16,   126,   132,   184,   190,   204,   211,
     212,   216,   239,   242,   243,   255,   289,   290,   291,   292,
     297,   298,   299,   308,   314,   183,   240,   183,   240,   288,
     189,   189,   183,   158,   159,   160,   161,   162,   163,   164,
     165,   166,   167,   189,   238,   288,   255,   284,   286,     1,
     181,   187,   183,   240,   210,   239,   146,   168,   237,   223,
     223,   223,   225,   225,   226,   226,   227,   227,   227,   227,
     228,   228,   229,   230,   231,   232,   233,   234,   239,   182,
     183,   190,   183,   185,   185,   182,   241,   187,   190,   278,
     190,   190,   290,   310,   180,   190,   190,   239,   309,   180,
     185,   293,   180,   181,   187,   190,   185,   290,   183,   189,
     183,   288,   288,   189,   237,   140,   285,   287,   185,   286,
     185,   237,   183,   183,   188,   183,   240,   190,   240,   279,
     190,   126,   180,   239,   190,   180,   210,   297,   140,   190,
     213,   237,   288,   189,   288,   182,   241,   187,   190,   237,
     183,   183,   180,   289,   298,   311,   181,   239,   255,   307,
     181,   185,   205,   205,   288,   183,   240,   287,   239,   307,
     312,   313,   300,   140,   181,   184,   303,   140,   214,   215,
     214,   183,   181,   190,   181,   290,   305,   189,   291,   294,
     295,   185,   304,   188,   187,   199,   199,   190,   239,   294,
      12,   288,   133,   134,   290,   301,   302,   140,   215,   306,
     240,   188,   185,   302,   290,   188
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
#line 674 "gc_glsl.y"
    {
          sloCOMPILER_SetScannerState(Compiler, slvSCANNER_NORMAL);
        ;}
    break;

  case 160:
#line 678 "gc_glsl.y"
    {
           yyval.token = slParseLayoutQualifier(Compiler, &yyvsp[-2].token);
        ;}
    break;

  case 161:
#line 682 "gc_glsl.y"
    {
          sloCOMPILER_SetScannerState(Compiler, slvSCANNER_NORMAL);
                  yyval.token = yyvsp[-3].token;
        ;}
    break;

  case 162:
#line 690 "gc_glsl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 163:
#line 692 "gc_glsl.y"
    { yyval.token = slParseAddLayoutId(Compiler, &yyvsp[-2].token, &yyvsp[0].token); ;}
    break;

  case 164:
#line 697 "gc_glsl.y"
    { yyval.token = slParseLayoutId(Compiler, &yyvsp[0].token, gcvNULL); ;}
    break;

  case 165:
#line 699 "gc_glsl.y"
    { yyval.token = slParseLayoutId(Compiler, &yyvsp[-2].token, &yyvsp[0].token) ; ;}
    break;

  case 166:
#line 701 "gc_glsl.y"
    { yyval.token = slParseLayoutId(Compiler, &yyvsp[-2].token, &yyvsp[0].token) ; ;}
    break;

  case 167:
#line 706 "gc_glsl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 168:
#line 708 "gc_glsl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 169:
#line 713 "gc_glsl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 170:
#line 715 "gc_glsl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 171:
#line 720 "gc_glsl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 172:
#line 722 "gc_glsl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 173:
#line 724 "gc_glsl.y"
    { yyval.token = slParseCheckStorage(Compiler, yyvsp[-1].token, yyvsp[0].token); ;}
    break;

  case 174:
#line 726 "gc_glsl.y"
    { yyval.token = yyvsp[0].token; ;}
    break;

  case 175:
#line 728 "gc_glsl.y"
    { yyval.token = slParseCheckStorage(Compiler, yyvsp[-1].token, yyvsp[0].token); ;}
    break;

  case 176:
#line 730 "gc_glsl.y"
    { yyval.token = yyvsp[0].token; ;}
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

  case 182:
#line 744 "gc_glsl.y"
    { yyval.dataType = slParseArrayDataType(Compiler, yyvsp[-2].dataType, gcvNULL); ;}
    break;

  case 183:
#line 746 "gc_glsl.y"
    { yyval.dataType = slParseArrayDataType(Compiler, yyvsp[-3].dataType, yyvsp[-1].expr); ;}
    break;

  case 184:
#line 748 "gc_glsl.y"
    { yyval.dataType = slParseArrayListDataType(Compiler, yyvsp[-1].dataType, yyvsp[0].fieldDeclList); ;}
    break;

  case 185:
#line 753 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_SAMPLER2D); ;}
    break;

  case 186:
#line 755 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_SAMPLERCUBE); ;}
    break;

  case 187:
#line 757 "gc_glsl.y"
    { yyval.dataType = slParseStructType(Compiler, yyvsp[0].dataType); ;}
    break;

  case 188:
#line 759 "gc_glsl.y"
    { yyval.dataType = slParseNamedType(Compiler, &yyvsp[0].token); ;}
    break;

  case 189:
#line 761 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_SAMPLER3D); ;}
    break;

  case 190:
#line 763 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_SAMPLER1DARRAY); ;}
    break;

  case 191:
#line 765 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_SAMPLER2DARRAY); ;}
    break;

  case 192:
#line 767 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_SAMPLER1DARRAYSHADOW); ;}
    break;

  case 193:
#line 769 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_SAMPLER2DSHADOW); ;}
    break;

  case 194:
#line 771 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_SAMPLER2DARRAYSHADOW); ;}
    break;

  case 195:
#line 773 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_SAMPLERCUBESHADOW); ;}
    break;

  case 196:
#line 775 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_SAMPLERCUBEARRAYSHADOW); ;}
    break;

  case 197:
#line 777 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_ISAMPLER2D); ;}
    break;

  case 198:
#line 779 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_ISAMPLERCUBE); ;}
    break;

  case 199:
#line 781 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_ISAMPLER3D); ;}
    break;

  case 200:
#line 783 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_ISAMPLER2DARRAY); ;}
    break;

  case 201:
#line 785 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_USAMPLER2D); ;}
    break;

  case 202:
#line 787 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_USAMPLERCUBE); ;}
    break;

  case 203:
#line 789 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_USAMPLER3D); ;}
    break;

  case 204:
#line 791 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_USAMPLER2DARRAY); ;}
    break;

  case 205:
#line 793 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_SAMPLEREXTERNALOES); ;}
    break;

  case 206:
#line 795 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_SAMPLER2DMS); ;}
    break;

  case 207:
#line 797 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_ISAMPLER2DMS); ;}
    break;

  case 208:
#line 799 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_USAMPLER2DMS); ;}
    break;

  case 209:
#line 801 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_SAMPLER2DMSARRAY); ;}
    break;

  case 210:
#line 803 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_ISAMPLER2DMSARRAY); ;}
    break;

  case 211:
#line 805 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_USAMPLER2DMSARRAY); ;}
    break;

  case 212:
#line 807 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_SAMPLERCUBEARRAY); ;}
    break;

  case 213:
#line 809 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_ISAMPLERCUBEARRAY); ;}
    break;

  case 214:
#line 811 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_USAMPLERCUBEARRAY); ;}
    break;

  case 215:
#line 813 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_SAMPLER1D); ;}
    break;

  case 216:
#line 815 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_ISAMPLER1D); ;}
    break;

  case 217:
#line 817 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_USAMPLER1D); ;}
    break;

  case 218:
#line 819 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_SAMPLER1DSHADOW); ;}
    break;

  case 219:
#line 821 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_SAMPLER2DRECT); ;}
    break;

  case 220:
#line 823 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_ISAMPLER2DRECT); ;}
    break;

  case 221:
#line 825 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_USAMPLER2DRECT); ;}
    break;

  case 222:
#line 827 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_SAMPLER2DRECTSHADOW); ;}
    break;

  case 223:
#line 829 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_ISAMPLER1DARRAY); ;}
    break;

  case 224:
#line 831 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_USAMPLER1DARRAY); ;}
    break;

  case 225:
#line 833 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_IMAGE2D); ;}
    break;

  case 226:
#line 835 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_IIMAGE2D); ;}
    break;

  case 227:
#line 837 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_UIMAGE2D); ;}
    break;

  case 228:
#line 839 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_IMAGE2DARRAY); ;}
    break;

  case 229:
#line 841 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_IIMAGE2DARRAY); ;}
    break;

  case 230:
#line 843 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_UIMAGE2DARRAY); ;}
    break;

  case 231:
#line 845 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_IMAGE3D); ;}
    break;

  case 232:
#line 847 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_IIMAGE3D); ;}
    break;

  case 233:
#line 849 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_UIMAGE3D); ;}
    break;

  case 234:
#line 851 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_IMAGECUBE); ;}
    break;

  case 235:
#line 853 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_IMAGECUBEARRAY); ;}
    break;

  case 236:
#line 855 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_IIMAGECUBE); ;}
    break;

  case 237:
#line 857 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_IIMAGECUBEARRAY); ;}
    break;

  case 238:
#line 859 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_UIMAGECUBE); ;}
    break;

  case 239:
#line 861 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_UIMAGECUBEARRAY); ;}
    break;

  case 240:
#line 863 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_ATOMIC_UINT); ;}
    break;

  case 241:
#line 865 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_SAMPLERBUFFER); ;}
    break;

  case 242:
#line 867 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_ISAMPLERBUFFER); ;}
    break;

  case 243:
#line 869 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_USAMPLERBUFFER); ;}
    break;

  case 244:
#line 871 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_IMAGEBUFFER); ;}
    break;

  case 245:
#line 873 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_IIMAGEBUFFER); ;}
    break;

  case 246:
#line 875 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_UIMAGEBUFFER); ;}
    break;

  case 247:
#line 880 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_VOID); ;}
    break;

  case 248:
#line 882 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_FLOAT); ;}
    break;

  case 249:
#line 884 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_INT); ;}
    break;

  case 250:
#line 886 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_UINT); ;}
    break;

  case 251:
#line 888 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_BOOL); ;}
    break;

  case 252:
#line 890 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_VEC2); ;}
    break;

  case 253:
#line 892 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_VEC3); ;}
    break;

  case 254:
#line 894 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_VEC4); ;}
    break;

  case 255:
#line 896 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_BVEC2); ;}
    break;

  case 256:
#line 898 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_BVEC3); ;}
    break;

  case 257:
#line 900 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_BVEC4); ;}
    break;

  case 258:
#line 902 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_IVEC2); ;}
    break;

  case 259:
#line 904 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_IVEC3); ;}
    break;

  case 260:
#line 906 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_IVEC4); ;}
    break;

  case 261:
#line 908 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_UVEC2); ;}
    break;

  case 262:
#line 910 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_UVEC3); ;}
    break;

  case 263:
#line 912 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_UVEC4); ;}
    break;

  case 264:
#line 914 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_MAT2); ;}
    break;

  case 265:
#line 916 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_MAT2X3); ;}
    break;

  case 266:
#line 918 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_MAT2X4); ;}
    break;

  case 267:
#line 920 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_MAT3); ;}
    break;

  case 268:
#line 922 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_MAT3X2); ;}
    break;

  case 269:
#line 924 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_MAT3X4); ;}
    break;

  case 270:
#line 926 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_MAT4); ;}
    break;

  case 271:
#line 928 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_MAT4X2); ;}
    break;

  case 272:
#line 930 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_MAT4X3); ;}
    break;

  case 273:
#line 932 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_DOUBLE); ;}
    break;

  case 274:
#line 934 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_DVEC2); ;}
    break;

  case 275:
#line 936 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_DVEC3); ;}
    break;

  case 276:
#line 938 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_DVEC4); ;}
    break;

  case 277:
#line 940 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_DMAT2); ;}
    break;

  case 278:
#line 942 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_DMAT2X3); ;}
    break;

  case 279:
#line 944 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_DMAT2X4); ;}
    break;

  case 280:
#line 946 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_DMAT3); ;}
    break;

  case 281:
#line 948 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_DMAT3X2); ;}
    break;

  case 282:
#line 950 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_DMAT3X4); ;}
    break;

  case 283:
#line 952 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_DMAT4); ;}
    break;

  case 284:
#line 954 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_DMAT4X2); ;}
    break;

  case 285:
#line 956 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_DMAT4X3); ;}
    break;

  case 286:
#line 958 "gc_glsl.y"
    { yyval.dataType = yyvsp[0].dataType; ;}
    break;

  case 287:
#line 963 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_FLOAT); ;}
    break;

  case 288:
#line 965 "gc_glsl.y"
    { yyval.dataType = slParseNonStructType(Compiler, &yyvsp[0].token, T_INT); ;}
    break;

  case 289:
#line 967 "gc_glsl.y"
    { yyval.dataType = yyvsp[0].dataType; ;}
    break;

  case 290:
#line 972 "gc_glsl.y"
    { yyval.token = yyvsp[0].token; ;}
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
#line 981 "gc_glsl.y"
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
#line 994 "gc_glsl.y"
    { slParseStructDeclBegin(Compiler, &yyvsp[-1].token); ;}
    break;

  case 299:
#line 996 "gc_glsl.y"
    { yyval.dataType = slParseStructDeclEnd(Compiler, &yyvsp[-4].token); ;}
    break;

  case 300:
#line 998 "gc_glsl.y"
    { slParseStructDeclBegin(Compiler, gcvNULL); ;}
    break;

  case 301:
#line 1000 "gc_glsl.y"
    { yyval.dataType = slParseStructDeclEnd(Compiler, gcvNULL); ;}
    break;

  case 302:
#line 1002 "gc_glsl.y"
    { slParseStructReDeclBegin(Compiler, &yyvsp[-1].token); ;}
    break;

  case 303:
#line 1004 "gc_glsl.y"
    { yyval.dataType = slParseStructReDeclEnd(Compiler, &yyvsp[-4].token); ;}
    break;

  case 306:
#line 1014 "gc_glsl.y"
    { slParseTypeSpecifiedFieldDeclList(Compiler, yyvsp[-2].dataType, yyvsp[-1].fieldDeclList); ;}
    break;

  case 307:
#line 1016 "gc_glsl.y"
    {
           yyvsp[-2].dataType->qualifiers.precision = yyvsp[-3].token.u.qualifiers.precision;
           slsQUALIFIERS_SET_FLAG(&(yyvsp[-2].dataType->qualifiers), slvQUALIFIERS_FLAG_PRECISION);
           slParseTypeSpecifiedFieldDeclList(Compiler, yyvsp[-2].dataType, yyvsp[-1].fieldDeclList);
        ;}
    break;

  case 308:
#line 1025 "gc_glsl.y"
    { yyval.fieldDeclList = slParseFieldDeclList(Compiler, yyvsp[0].fieldDecl); ;}
    break;

  case 309:
#line 1027 "gc_glsl.y"
    { yyval.fieldDeclList = slParseFieldDeclList2(Compiler, yyvsp[-2].fieldDeclList, yyvsp[0].fieldDecl); ;}
    break;

  case 310:
#line 1032 "gc_glsl.y"
    { yyval.fieldDecl = slParseFieldDecl(Compiler, &yyvsp[0].token, gcvNULL); ;}
    break;

  case 311:
#line 1034 "gc_glsl.y"
    { yyval.fieldDecl = slParseFieldDecl(Compiler, &yyvsp[-3].token, yyvsp[-1].expr); ;}
    break;

  case 312:
#line 1036 "gc_glsl.y"
    { yyval.fieldDecl = slParseFieldListDecl(Compiler, &yyvsp[-1].token, yyvsp[0].fieldDeclList, gcvFALSE); ;}
    break;

  case 313:
#line 1041 "gc_glsl.y"
    { yyval.statement = slParseInterfaceBlock(Compiler, yyvsp[-1].blockName, gcvNULL, gcvNULL, gcvTRUE); ;}
    break;

  case 314:
#line 1043 "gc_glsl.y"
    { yyval.statement = slParseInterfaceBlock(Compiler, yyvsp[-2].blockName, &yyvsp[-1].token, gcvNULL, gcvTRUE); ;}
    break;

  case 315:
#line 1045 "gc_glsl.y"
    { yyval.statement = slParseInterfaceBlock(Compiler, yyvsp[-5].blockName, &yyvsp[-4].token, yyvsp[-2].expr, gcvTRUE); ;}
    break;

  case 316:
#line 1047 "gc_glsl.y"
    { yyval.statement = slParseInterfaceBlockImplicitArrayLength(Compiler, yyvsp[-4].blockName, &yyvsp[-3].token); ;}
    break;

  case 317:
#line 1052 "gc_glsl.y"
    { slParseInterfaceBlockDeclBegin(Compiler, &yyvsp[-2].token, &yyvsp[-1].token); ;}
    break;

  case 318:
#line 1054 "gc_glsl.y"
    { yyval.blockName = slParseInterfaceBlockDeclEnd(Compiler, &yyvsp[-5].token, &yyvsp[-4].token); ;}
    break;

  case 319:
#line 1056 "gc_glsl.y"
    { slParseInterfaceBlockDeclBegin(Compiler, &yyvsp[-2].token, &yyvsp[-1].token); ;}
    break;

  case 320:
#line 1058 "gc_glsl.y"
    {
            yyclearin;
            yyerrok;
            yyval.blockName = slParseInterfaceBlockDeclEnd(Compiler, gcvNULL, gcvNULL);
        ;}
    break;

  case 323:
#line 1072 "gc_glsl.y"
    { yyval.dataType = slParseInterfaceBlockMember(Compiler, yyvsp[-1].dataType, yyvsp[0].fieldDecl); ;}
    break;

  case 324:
#line 1074 "gc_glsl.y"
    { yyval.dataType = slParseInterfaceBlockMember(Compiler, yyvsp[-2].dataType, yyvsp[0].fieldDecl); ;}
    break;

  case 326:
#line 1083 "gc_glsl.y"
    { yyval.fieldDecl = slParseFieldDecl(Compiler, &yyvsp[0].token, gcvNULL); ;}
    break;

  case 327:
#line 1085 "gc_glsl.y"
    { yyval.fieldDecl = slParseFieldDecl(Compiler, &yyvsp[-3].token, yyvsp[-1].expr); ;}
    break;

  case 328:
#line 1087 "gc_glsl.y"
    { yyval.fieldDecl = slParseImplicitArraySizeFieldDecl(Compiler, &yyvsp[-2].token); ;}
    break;

  case 329:
#line 1089 "gc_glsl.y"
    { yyval.fieldDecl = slParseFieldListDecl(Compiler, &yyvsp[-1].token, yyvsp[0].fieldDeclList, gcvTRUE); ;}
    break;

  case 330:
#line 1094 "gc_glsl.y"
    { yyval.expr = yyvsp[0].expr; ;}
    break;

  case 331:
#line 1099 "gc_glsl.y"
    { yyval.statement = yyvsp[0].statement; ;}
    break;

  case 332:
#line 1104 "gc_glsl.y"
    { yyval.statement = slParseCompoundStatementAsStatement(Compiler, yyvsp[0].statements); ;}
    break;

  case 333:
#line 1106 "gc_glsl.y"
    { yyval.statement = yyvsp[0].statement; ;}
    break;

  case 334:
#line 1113 "gc_glsl.y"
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
#line 1128 "gc_glsl.y"
    { yyval.statements = gcvNULL; ;}
    break;

  case 341:
#line 1130 "gc_glsl.y"
    { slParseCompoundStatementBegin(Compiler); ;}
    break;

  case 342:
#line 1132 "gc_glsl.y"
    { yyval.statements = slParseCompoundStatementEnd(Compiler, &yyvsp[-3].token, yyvsp[-1].statements); ;}
    break;

  case 343:
#line 1137 "gc_glsl.y"
    { yyval.statement = slParseCompoundStatementNoNewScopeAsStatementNoNewScope(Compiler, yyvsp[0].statements); ;}
    break;

  case 344:
#line 1139 "gc_glsl.y"
    { yyval.statement = yyvsp[0].statement; ;}
    break;

  case 345:
#line 1144 "gc_glsl.y"
    { yyval.statements = gcvNULL; ;}
    break;

  case 346:
#line 1146 "gc_glsl.y"
    { slParseCompoundStatementNoNewScopeBegin(Compiler); ;}
    break;

  case 347:
#line 1148 "gc_glsl.y"
    { yyval.statements = slParseCompoundStatementNoNewScopeEnd(Compiler, &yyvsp[-3].token, yyvsp[-1].statements); ;}
    break;

  case 348:
#line 1153 "gc_glsl.y"
    { yyval.statements = slParseStatementList(Compiler, yyvsp[0].statement); ;}
    break;

  case 349:
#line 1155 "gc_glsl.y"
    { yyval.statements = slParseStatementList2(Compiler, yyvsp[-1].statements, yyvsp[0].statement); ;}
    break;

  case 350:
#line 1160 "gc_glsl.y"
    { yyval.statement = gcvNULL; ;}
    break;

  case 351:
#line 1162 "gc_glsl.y"
    { yyval.statement = slParseExprAsStatement(Compiler, yyvsp[-1].expr); ;}
    break;

  case 352:
#line 1167 "gc_glsl.y"
    { slParseSelectStatementBegin(Compiler); ;}
    break;

  case 353:
#line 1169 "gc_glsl.y"
    { yyval.statement = slParseSelectionStatement(Compiler, &yyvsp[-5].token, yyvsp[-3].expr, yyvsp[0].selectionStatementPair); ;}
    break;

  case 354:
#line 1171 "gc_glsl.y"
    { yyval.statement = slParseSwitchStatement(Compiler, &yyvsp[-4].token, yyvsp[-2].expr, yyvsp[0].statement); ;}
    break;

  case 355:
#line 1176 "gc_glsl.y"
    { yyval.statements = slParseStatementList(Compiler, yyvsp[0].statement); ;}
    break;

  case 356:
#line 1178 "gc_glsl.y"
    { yyval.statements = slParseStatementList2(Compiler, yyvsp[-1].statements, yyvsp[0].statement); ;}
    break;

  case 357:
#line 1183 "gc_glsl.y"
    { yyval.statement = yyvsp[0].statement; ;}
    break;

  case 358:
#line 1185 "gc_glsl.y"
    { yyval.statement = slParseCaseStatement(Compiler, &yyvsp[-2].token, yyvsp[-1].expr); ;}
    break;

  case 359:
#line 1187 "gc_glsl.y"
    { yyval.statement = slParseDefaultStatement(Compiler, &yyvsp[-1].token); ;}
    break;

  case 360:
#line 1192 "gc_glsl.y"
    { yyval.statement = gcvNULL; ;}
    break;

  case 361:
#line 1194 "gc_glsl.y"
    { slParseSwitchBodyBegin(Compiler); ;}
    break;

  case 362:
#line 1196 "gc_glsl.y"
    { yyval.statement = slParseSwitchBodyEnd(Compiler, &yyvsp[-3].token, yyvsp[-1].statements); ;}
    break;

  case 363:
#line 1202 "gc_glsl.y"
    { slParseSelectStatementEnd(Compiler, gcvNULL, gcvNULL);
          slParseSelectStatementBegin(Compiler); ;}
    break;

  case 364:
#line 1205 "gc_glsl.y"
    { slParseSelectStatementEnd(Compiler, gcvNULL, gcvNULL);
          yyval.selectionStatementPair = slParseSelectionRestStatement(Compiler, yyvsp[-3].statement, yyvsp[0].statement); ;}
    break;

  case 365:
#line 1208 "gc_glsl.y"
    { slParseSelectStatementEnd(Compiler, gcvNULL, gcvNULL);
          yyval.selectionStatementPair = slParseSelectionRestStatement(Compiler, yyvsp[0].statement, gcvNULL); ;}
    break;

  case 366:
#line 1216 "gc_glsl.y"
    { yyval.expr = yyvsp[0].expr; ;}
    break;

  case 367:
#line 1218 "gc_glsl.y"
    { yyval.expr = slParseCondition(Compiler, yyvsp[-3].dataType, &yyvsp[-2].token, yyvsp[0].expr); ;}
    break;

  case 368:
#line 1223 "gc_glsl.y"
    { slParseWhileStatementBegin(Compiler); ;}
    break;

  case 369:
#line 1225 "gc_glsl.y"
    { yyval.statement = slParseWhileStatementEnd(Compiler, &yyvsp[-5].token, yyvsp[-2].expr, yyvsp[0].statement); ;}
    break;

  case 370:
#line 1227 "gc_glsl.y"
    { yyval.statement = slParseDoWhileStatement(Compiler, &yyvsp[-6].token, yyvsp[-5].statement, yyvsp[-2].expr); ;}
    break;

  case 371:
#line 1229 "gc_glsl.y"
    { slParseForStatementBegin(Compiler); ;}
    break;

  case 372:
#line 1231 "gc_glsl.y"
    { yyval.statement = slParseForStatementEnd(Compiler, &yyvsp[-6].token, yyvsp[-3].statement, yyvsp[-2].forExprPair, yyvsp[0].statement); ;}
    break;

  case 373:
#line 1236 "gc_glsl.y"
    { yyval.statement = yyvsp[0].statement; ;}
    break;

  case 374:
#line 1238 "gc_glsl.y"
    { yyval.statement = yyvsp[0].statement; ;}
    break;

  case 375:
#line 1243 "gc_glsl.y"
    { yyval.expr = yyvsp[0].expr; ;}
    break;

  case 376:
#line 1245 "gc_glsl.y"
    { yyval.expr = gcvNULL; ;}
    break;

  case 377:
#line 1250 "gc_glsl.y"
    { yyval.forExprPair = slParseForRestStatement(Compiler, yyvsp[-1].expr, gcvNULL); ;}
    break;

  case 378:
#line 1252 "gc_glsl.y"
    { yyval.forExprPair = slParseForRestStatement(Compiler, yyvsp[-2].expr, yyvsp[0].expr); ;}
    break;

  case 379:
#line 1257 "gc_glsl.y"
    { yyval.statement = slParseJumpStatement(Compiler, slvCONTINUE, &yyvsp[-1].token, gcvNULL); ;}
    break;

  case 380:
#line 1259 "gc_glsl.y"
    { yyval.statement = slParseJumpStatement(Compiler, slvBREAK, &yyvsp[-1].token, gcvNULL); ;}
    break;

  case 381:
#line 1261 "gc_glsl.y"
    { yyval.statement = slParseJumpStatement(Compiler, slvRETURN, &yyvsp[-1].token, gcvNULL); ;}
    break;

  case 382:
#line 1263 "gc_glsl.y"
    { yyval.statement = slParseJumpStatement(Compiler, slvRETURN, &yyvsp[-2].token, yyvsp[-1].expr); ;}
    break;

  case 383:
#line 1265 "gc_glsl.y"
    { yyval.statement = slParseJumpStatement(Compiler, slvDISCARD, &yyvsp[-1].token, gcvNULL); ;}
    break;

  case 387:
#line 1278 "gc_glsl.y"
    { slParseExternalDecl(Compiler, yyvsp[0].statement); ;}
    break;

  case 388:
#line 1283 "gc_glsl.y"
    { slParseFuncDefinitionBegin(Compiler, yyvsp[0].funcName); ;}
    break;

  case 389:
#line 1285 "gc_glsl.y"
    { slParseFuncDefinitionEnd(Compiler, yyvsp[-2].funcName); ;}
    break;

  case 390:
#line 1286 "gc_glsl.y"
    { slParseFuncDef(Compiler, yyvsp[-3].funcName, yyvsp[-1].statements); ;}
    break;


    }

/* Line 991 of yacc.c.  */
#line 5052 "gc_glsl_parser.c"

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
     invokes YYERROR.  */
#if defined (__GNUC_MINOR__) && 2093 <= (__GNUC__ * 1000 + __GNUC_MINOR__)
  __attribute__ ((__unused__))
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


#line 1289 "gc_glsl.y"



