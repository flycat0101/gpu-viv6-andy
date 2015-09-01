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


#ifndef __gc_glsl_token_def_h_
#define __gc_glsl_token_def_h_
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




#if !defined (YYSTYPE) && ! defined (YYSTYPE_IS_DECLARED)
#line 25 "gc_glsl.y"
typedef union YYSTYPE {
	slsLexToken					token;

	slsDeclOrDeclList			declOrDeclList;

	slsDLINK_LIST *				fieldDeclList;

	slsFieldDecl *				fieldDecl;

	slsDATA_TYPE *				dataType;

	sloIR_EXPR				expr;

	slsNAME	*				funcName;

	slsNAME	*				paramName;

	slsNAME *				blockName;

	sloIR_SET					statements;

	sloIR_BASE					statement;

	slsSelectionStatementPair	selectionStatementPair;

	slsForExprPair				forExprPair;

	sloIR_POLYNARY_EXPR			funcCall;

    slsASM_OPCODE               asmOpcode;

    sloIR_VIV_ASM               vivAsm;

    slsASM_MODIFIER             asmModifier;

    slsASM_MODIFIERS            asmModifiers;
} YYSTYPE;
/* Line 1268 of yacc.c.  */
#line 391 "gc_glsl_token_def.h"
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif





#endif /* __gc_glsl_token_def_h_ */
