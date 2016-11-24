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


#ifndef __gc_cl_token_def_h_
#define __gc_cl_token_def_h_
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




#if !defined (YYSTYPE) && ! defined (YYSTYPE_IS_DECLARED)
#line 29 "gc_cl.y"
typedef union YYSTYPE {
    clsLexToken     token;
    slsSLINK_LIST *     typeQualifierList;
    clsDeclOrDeclList   *declOrDeclList;
    slsDLINK_LIST *     fieldDeclList;
    clsFieldDecl *      fieldDecl;
    clsDATA_TYPE *      dataType;
    clsDECL         decl;
    cloIR_EXPR      expr;
    clsNAME *       funcName;
    clsNAME *       paramName;
    clsATTRIBUTE *      attr;
    slsSLINK_LIST *     enumeratorList;
    clsNAME *       enumeratorName;
    cloIR_SET       statements;
    cloIR_BASE      statement;
    clsIfStatementPair  ifStatementPair;
    clsForExprPair      forExprPair;
    cloIR_POLYNARY_EXPR funcCall;
    gceSTATUS       status;
} YYSTYPE;
/* Line 1268 of yacc.c.  */
#line 531 "gc_cl.tab.h"
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif





#endif /* __gc_cl_token_def_h_ */
