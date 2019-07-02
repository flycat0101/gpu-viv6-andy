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


#ifndef __gc_glsl_token_def_h_
#define __gc_glsl_token_def_h_

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
/* Line 1275 of yacc.c.  */
#line 439 "gc_glsl_token_def.h"
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif





#endif /* __gc_glsl_token_def_h_ */
