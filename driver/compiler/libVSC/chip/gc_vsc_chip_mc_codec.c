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


#include "gc_vsc.h"
#include "chip/gc_vsc_chip_mc_codec.h"

/* Since MC inst layout defined in AQShader.h by HW is not friendly with SW, we define
   auxiliary structures to codec MC binary. It must be changed if HW's design is changed.
   For example, if future chip fully redesign a new HW inst layout, we need add new ones
   for such change.
*/

/*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~    !!!!NOTE!!!!    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

   1. Most following MC codec layout structures do not indicate immediate value explicitly,
      but HW can support 20-bits IMM when srcType is 0x7 for
      any src. These 20-bits IMM will use below 22-bits with that MSB2 is IMM data type, and
      LSB20 is real IMM data.

      from lsb to msb
          srcRegNo   : 9;
          srcSwizzle : 8;
          bSrcModNeg : 1;
          bSrcModAbs : 1;
          srcRelAddr : 3;

   2. Under dual16 mode, thread-type needs to be set per inst if opcode supports it. As IMM,
      not all layouts point out this info except few ones. However, following 2 bits will be
      reused for thread-type programming.

      from lsb to msb
         dstRegNoBit7 : 1;
         dstRegNoBit8 : 1;

   3. SINXHPI/COSXHPI/LOG/DIV/MULLO is not enabled unless rounding mode to RTZ, in this case,
      rounding mode is borrowed as enable bit. If this enable bit is not set, NAN will be returned.

   4. When extended opcode is used (base-opcode is 0x7F at this time), HW
      borrows imm of src2 to save extended opcode (LSB8 of 20bit imm), so HW will use all info
      that imm value needs described in NOTE 2, as well as src2Valid bit. But actually, such
      implementation is bit-wasted and SW unfriendly because HW should be able to directly
      decode that 8-bits extended opcode based on base opcode is 0x7F. So,
      to hide such HW implementation, in our MC inst layout, we only provide 8-bit 'extOpcode',
      no other imm info and src2Valid bit are provided (they are called as reserved ones). When
      codecing extOpcode, any inst layout will be casted into ALU_3_SRCS_INST layout to codec
      those extra info (that means relative reserved ones might be written with non-zero value).

   5. MSB5 of word0 (samplerSlot as usual in normal layouts) has special meaning for NORM_MUL.
      If that 5bits are zero, result of NORM_MUL will generate INF for (0 * INF), otherwise,
      0 is generated for (0 * INF). For the purpose of this opcode which handling normalize(0),
      so we need always set that 5bits non-zero. Note that, that MSB5 are reserved in NORM_MUL
      layout, so we will cast to VSC_MC_SAMPLE_INST when codecing (that means relative reserved
      one might be written with non-zero value).

   6. Since we use 3-srcs ALU inst layout to codec load_attr, but load_attr has a special bNeedUscSync
      bit (bit-30 of word0) to indicate whether needs USC level data sync, so we need cast 3-srcs ALU
      inst to store_attr inst layout to do codec for this bit.

   7. MSB5 of word0 (samplerSlot as usual in normal layouts) has special meaning for MUL. If that
      5bits are zero, result of MUL will generate INF for (0 * INF), otherwise, 0 is generated for
      (0 * INF). Note that, that MSB5 are reserved in MUL layout, so we will cast to VSC_MC_SAMPLE_INST
      when codecing (that means relative reserved one might be written with non-zero value).

   8. Since we use 3-srcs ALU inst layout to codec atomic operations, but atomic operations have a
      special bAccessLocalStorage bit (bit-8 of word1) to indicate whether the operation will act on
      local memory, not global memory, so we need cast 3-srcs ALU inst to load inst layout to do codec
      for this bit.

   9. Img_ld/img_st can be executed on evis mode. When codecing such img_ld/img_st, we will cast to
      VSC_MC_EVIS_INST to codec evis-state/startDstCompIdx/endDstCompIdx (note no sourcebin is needed)

   ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

/* No dst/src case */
typedef union _VSC_MC_NO_OPERAND_INST
{
    struct
    {
        gctUINT        baseOpcodeBit0_5      : 6; /* Together with baseOpcodeBit6 to compose base-opcode */
        gctUINT        reserved0             : 26;/* Must be zero'ed */

        gctUINT        reserved1             : 32;/* Must be zero'ed */

        gctUINT        reserved2             : 16;/* Must be zero'ed */
        gctUINT        baseOpcodeBit6        : 1;
        gctUINT        reserved3             : 15;/* Must be zero'ed */

        gctUINT        reserved4             : 4; /* Must be zero'ed for non-extopcode mode, otherwise, see NOTE 4 */
        gctUINT        extOpcode             : 8;
        gctUINT        reserved5             : 20;/* Must be zero'ed for non-extopcode mode, otherwise, see NOTE 4 */
    } inst;

    gctUINT            data[4];
}
VSC_MC_NO_OPERAND_INST;

/* 3-srcs alu inst case */
typedef union _VSC_MC_ALU_3_SRCS_INST
{
    struct
    {
        gctUINT        baseOpcodeBit0_5      : 6; /* Together with baseOpcodeBit6 to compose base-opcode */
        gctUINT        condOpCode            : 5;
        gctUINT        bDstModSat            : 1;
        gctUINT        bDstValid             : 1; /* Must be valid */
        gctUINT        dstRelAddr            : 3;
        gctUINT        dstRegNoBit0_6        : 7; /* Together with dstRegNoBit7 and dstRegNoBit8 to compose dstRegNo */
        gctUINT        writeMask             : 4;
        gctUINT        reserved0             : 5; /* Must be zero'ed for non load_attr, for load_attr, see NOTE 6 */

        gctUINT        roundMode             : 2;
        gctUINT        bPackMode             : 1;
        gctUINT        reserved1             : 4; /* Must be zero'ed */
        gctUINT        bSkipForHelperKickoff : 1;
        gctUINT        reserved2             : 3; /* Must be zero'ed for non-atomics. For atomics, see NOTE 8 */
        gctUINT        bSrc0Valid            : 1; /* Must be valid */
        gctUINT        src0RegNo             : 9;
        gctUINT        instTypeBit0          : 1; /* Together with instTypeBit1_2 to compose instType */
        gctUINT        src0Swizzle           : 8;
        gctUINT        bSrc0ModNeg           : 1;
        gctUINT        bSrc0ModAbs           : 1;

        gctUINT        src0RelAddr           : 3;
        gctUINT        src0Type              : 3;
        gctUINT        bSrc1Valid            : 1; /* Must be valid */
        gctUINT        src1RegNo             : 9;
        gctUINT        baseOpcodeBit6        : 1;
        gctUINT        src1Swizzle           : 8;
        gctUINT        bSrc1ModNeg           : 1;
        gctUINT        bSrc1ModAbs           : 1;
        gctUINT        src1RelAddr           : 3;
        gctUINT        instTypeBit1_2        : 2;

        gctUINT        src1Type              : 3;
        gctUINT        bSrc2Valid            : 1; /* Must be valid */
        gctUINT        src2RegNo             : 9;
        gctUINT        dstRegNoBit7          : 1;
        gctUINT        src2Swizzle           : 8;
        gctUINT        bSrc2ModNeg           : 1;
        gctUINT        bSrc2ModAbs           : 1;
        gctUINT        dstRegNoBit8          : 1;
        gctUINT        src2RelAddr           : 3;
        gctUINT        src2Type              : 3;
        gctUINT        dstType               : 1;
    } inst;

    gctUINT            data[4];
}
VSC_MC_ALU_3_SRCS_INST;

/* 2-srcs (src0 + src1) alu inst case */
typedef union _VSC_MC_ALU_2_SRCS_SRC0_SRC1_INST
{
    struct
    {
        gctUINT        baseOpcodeBit0_5      : 6; /* Together with baseOpcodeBit6 to compose base-opcode */
        gctUINT        condOpCode            : 5;
        gctUINT        bDstModSat            : 1;
        gctUINT        bDstValid             : 1;
        gctUINT        dstRelAddr            : 3;
        gctUINT        dstRegNoBit0_6        : 7; /* Together with dstRegNoBit7 and dstRegNoBit8 to compose dstRegNo */
        gctUINT        writeMask             : 4;
        gctUINT        reserved0             : 5; /* Must be zero'ed for non NORM_MUL/MUL, otherwise, see NOTE 5/7 */

        gctUINT        roundMode             : 2;
        gctUINT        bPackMode             : 1;
        gctUINT        reserved1             : 8; /* Must be zero'ed */
        gctUINT        bSrc0Valid            : 1; /* Must be valid */
        gctUINT        src0RegNo             : 9;
        gctUINT        instTypeBit0          : 1; /* Together with instTypeBit1_2 to compose instType */
        gctUINT        src0Swizzle           : 8;
        gctUINT        bSrc0ModNeg           : 1;
        gctUINT        bSrc0ModAbs           : 1;

        gctUINT        src0RelAddr           : 3;
        gctUINT        src0Type              : 3;
        gctUINT        bSrc1Valid            : 1; /* Must be valid */
        gctUINT        src1RegNo             : 9;
        gctUINT        baseOpcodeBit6        : 1;
        gctUINT        src1Swizzle           : 8;
        gctUINT        bSrc1ModNeg           : 1;
        gctUINT        bSrc1ModAbs           : 1;
        gctUINT        src1RelAddr           : 3;
        gctUINT        instTypeBit1_2        : 2;

        gctUINT        src1Type              : 3;
        gctUINT        reserved2             : 1; /* Must be zero'ed for non-extopcode mode, otherwise, see NOTE 4 */
        gctUINT        extOpcode             : 8;
        gctUINT        reserved3             : 1; /* Must be zero'ed */
        gctUINT        dstRegNoBit7          : 1;
        gctUINT        reserved4             : 10;/* Must be zero'ed */
        gctUINT        dstRegNoBit8          : 1;
        gctUINT        reserved5             : 6; /* Must be zero'ed for non-extopcode mode, otherwise, see NOTE 4 */
        gctUINT        dstType               : 1;
    } inst;

    gctUINT            data[4];
}
VSC_MC_ALU_2_SRCS_SRC0_SRC1_INST;

/* 2-srcs (src0 + src2) alu inst case */
typedef union _VSC_MC_ALU_2_SRCS_SRC0_SRC2_INST
{
    struct
    {
        gctUINT        baseOpcodeBit0_5      : 6; /* Together with baseOpcodeBit6 to compose base-opcode */
        gctUINT        reserved0             : 5; /* Must be zero'ed */
        gctUINT        bDstModSat            : 1;
        gctUINT        bDstValid             : 1; /* Must be valid */
        gctUINT        dstRelAddr            : 3;
        gctUINT        dstRegNoBit0_6        : 7; /* Together with dstRegNoBit7 and dstRegNoBit8 to compose dstRegNo */
        gctUINT        writeMask             : 4;
        gctUINT        reserved1             : 5; /* Must be zero'ed */

        gctUINT        roundMode             : 2;
        gctUINT        bPackMode             : 1;
        gctUINT        reserved2             : 8; /* Must be zero'ed */
        gctUINT        bSrc0Valid            : 1; /* Must be valid */
        gctUINT        src0RegNo             : 9;
        gctUINT        instTypeBit0          : 1; /* Together with instTypeBit1_2 to compose instType */
        gctUINT        src0Swizzle           : 8;
        gctUINT        bSrc0ModNeg           : 1;
        gctUINT        bSrc0ModAbs           : 1;

        gctUINT        src0RelAddr           : 3;
        gctUINT        src0Type              : 3;
        gctUINT        reserved3             : 10;/* Must be zero'ed */
        gctUINT        baseOpcodeBit6        : 1;
        gctUINT        reserved4             : 13;/* Must be zero'ed */
        gctUINT        instTypeBit1_2        : 2;

        gctUINT        reserved5             : 3; /* Must be zero'ed */
        gctUINT        bSrc2Valid            : 1; /* Must be valid */
        gctUINT        src2RegNo             : 9;
        gctUINT        dstRegNoBit7          : 1;
        gctUINT        src2Swizzle           : 8;
        gctUINT        bSrc2ModNeg           : 1;
        gctUINT        bSrc2ModAbs           : 1;
        gctUINT        dstRegNoBit8          : 1;
        gctUINT        src2RelAddr           : 3;
        gctUINT        src2Type              : 3;
        gctUINT        dstType               : 1;
    } inst;

    gctUINT            data[4];
}
VSC_MC_ALU_2_SRCS_SRC0_SRC2_INST;

/* 2-srcs (src1 + src2) alu inst case */
typedef union _VSC_MC_ALU_2_SRCS_SRC1_SRC2_INST
{
    struct
    {
        gctUINT        baseOpcodeBit0_5      : 6; /* Together with baseOpcodeBit6 to compose base-opcode */
        gctUINT        reserved0             : 5; /* Must be zero'ed */
        gctUINT        bDstModSat            : 1;
        gctUINT        bDstValid             : 1; /* Must be valid */
        gctUINT        dstRelAddr            : 3;
        gctUINT        dstRegNoBit0_6        : 7; /* Together with dstRegNoBit7 and dstRegNoBit8 to compose dstRegNo */
        gctUINT        writeMask             : 4;
        gctUINT        reserved1             : 5; /* Must be zero'ed */

        gctUINT        roundMode             : 2;
        gctUINT        bPackMode             : 1;
        gctUINT        reserved2             : 18;/* Must be zero'ed */
        gctUINT        instTypeBit0          : 1; /* Together with instTypeBit1_2 to compose instType */
        gctUINT        reserved3             : 10;/* Must be zero'ed */

        gctUINT        reserved4             : 6; /* Must be zero'ed */
        gctUINT        bSrc1Valid            : 1; /* Must be valid */
        gctUINT        src1RegNo             : 9;
        gctUINT        baseOpcodeBit6        : 1;
        gctUINT        src1Swizzle           : 8;
        gctUINT        bSrc1ModNeg           : 1;
        gctUINT        bSrc1ModAbs           : 1;
        gctUINT        src1RelAddr           : 3;
        gctUINT        instTypeBit1_2        : 2;

        gctUINT        src1Type              : 3;
        gctUINT        bSrc2Valid            : 1; /* Must be valid */
        gctUINT        src2RegNo             : 9;
        gctUINT        dstRegNoBit7          : 1;
        gctUINT        src2Swizzle           : 8;
        gctUINT        bSrc2ModNeg           : 1;
        gctUINT        bSrc2ModAbs           : 1;
        gctUINT        dstRegNoBit8          : 1;
        gctUINT        src2RelAddr           : 3;
        gctUINT        src2Type              : 3;
        gctUINT        dstType               : 1;
    } inst;

    gctUINT            data[4];
}
VSC_MC_ALU_2_SRCS_SRC1_SRC2_INST;

/* 1-src (src0) alu inst case */
typedef union _VSC_MC_ALU_1_SRC_SRC0_INST
{
    struct
    {
        gctUINT        baseOpcodeBit0_5      : 6; /* Together with baseOpcodeBit6 to compose base-opcode */
        gctUINT        reserved0             : 5; /* Must be zero'ed */
        gctUINT        bDstModSat            : 1;
        gctUINT        bDstValid             : 1; /* Must be valid */
        gctUINT        dstRelAddr            : 3;
        gctUINT        dstRegNoBit0_6        : 7; /* Together with dstRegNoBit7 and dstRegNoBit8 to compose dstRegNo */
        gctUINT        writeMask             : 4;
        gctUINT        reserved1             : 5; /* Must be zero'ed */

        gctUINT        roundMode             : 2;
        gctUINT        bPackMode             : 1;
        gctUINT        reserved2             : 8; /* Must be zero'ed */
        gctUINT        bSrc0Valid            : 1; /* Must be valid */
        gctUINT        src0RegNo             : 9;
        gctUINT        instTypeBit0          : 1; /* Together with instTypeBit1_2 to compose instType */
        gctUINT        src0Swizzle           : 8;
        gctUINT        bSrc0ModNeg           : 1;
        gctUINT        bSrc0ModAbs           : 1;

        gctUINT        src0RelAddr           : 3;
        gctUINT        src0Type              : 3;
        gctUINT        reserved3             : 10;/* Must be zero'ed */
        gctUINT        baseOpcodeBit6        : 1;
        gctUINT        reserved4             : 13;/* Must be zero'ed */
        gctUINT        instTypeBit1_2        : 2;

        gctUINT        reserved5             : 4; /* Must be zero'ed for non-extopcode mode, otherwise, see NOTE 4 */
        gctUINT        extOpcode             : 8;
        gctUINT        reserved6             : 1; /* Must be zero'ed */
        gctUINT        dstRegNoBit7          : 1;
        gctUINT        reserved7             : 10;/* Must be zero'ed */
        gctUINT        dstRegNoBit8          : 1;
        gctUINT        reserved8             : 6; /* Must be zero'ed for non-extopcode mode, otherwise, see NOTE 4 */
        gctUINT        dstType               : 1;
    } inst;

    gctUINT            data[4];
}
VSC_MC_ALU_1_SRC_SRC0_INST;

/* 1-src (src1) alu inst case */
typedef union _VSC_MC_ALU_1_SRC_SRC1_INST
{
    struct
    {
        gctUINT        baseOpcodeBit0_5      : 6; /* Together with baseOpcodeBit6 to compose base-opcode */
        gctUINT        reserved0             : 5; /* Must be zero'ed */
        gctUINT        bDstModSat            : 1;
        gctUINT        bDstValid             : 1; /* Must be valid */
        gctUINT        dstRelAddr            : 3;
        gctUINT        dstRegNoBit0_6        : 7; /* Together with dstRegNoBit7 and dstRegNoBit8 to compose dstRegNo */
        gctUINT        writeMask             : 4;
        gctUINT        reserved1             : 5; /* Must be zero'ed */

        gctUINT        roundMode             : 2;
        gctUINT        bPackMode             : 1;
        gctUINT        reserved2             : 18;/* Must be zero'ed */
        gctUINT        instTypeBit0          : 1; /* Together with instTypeBit1_2 to compose instType */
        gctUINT        reserved3             : 10;/* Must be zero'ed */

        gctUINT        reserved4             : 6; /* Must be zero'ed */
        gctUINT        bSrc1Valid            : 1; /* Must be valid */
        gctUINT        src1RegNo             : 9;
        gctUINT        baseOpcodeBit6        : 1;
        gctUINT        src1Swizzle           : 8;
        gctUINT        bSrc1ModNeg           : 1;
        gctUINT        bSrc1ModAbs           : 1;
        gctUINT        src1RelAddr           : 3;
        gctUINT        instTypeBit1_2        : 2;

        gctUINT        src1Type              : 3;
        gctUINT        reserved5             : 1; /* Must be zero'ed for non-extopcode mode, otherwise, see NOTE 4 */
        gctUINT        extOpcode             : 8;
        gctUINT        reserved6             : 1; /* Must be zero'ed */
        gctUINT        dstRegNoBit7          : 1;
        gctUINT        reserved7             : 10;/* Must be zero'ed */
        gctUINT        dstRegNoBit8          : 1;
        gctUINT        reserved8             : 6; /* Must be zero'ed for non-extopcode mode, otherwise, see NOTE 4 */
        gctUINT        dstType               : 1;
    } inst;

    gctUINT            data[4];
}
VSC_MC_ALU_1_SRC_SRC1_INST;

/* 1-src (src2) alu inst case */
typedef union _VSC_MC_ALU_1_SRC_SRC2_INST
{
    struct
    {
        gctUINT        baseOpcodeBit0_5      : 6; /* Together with baseOpcodeBit6 to compose base-opcode */
        gctUINT        reserved0             : 5; /* Must be zero'ed */
        gctUINT        bDstModSat            : 1;
        gctUINT        bDstValid             : 1; /* Must be valid */
        gctUINT        dstRelAddr            : 3;
        gctUINT        dstRegNoBit0_6        : 7; /* Together with dstRegNoBit7 and dstRegNoBit8 to compose dstRegNo */
        gctUINT        writeMask             : 4;
        gctUINT        reserved1             : 5; /* Must be zero'ed */

        gctUINT        roundMode             : 2;
        gctUINT        bPackMode             : 1;
        gctUINT        reserved2             : 18;/* Must be zero'ed */
        gctUINT        instTypeBit0          : 1; /* Together with instTypeBit1_2 to compose instType */
        gctUINT        reserved3             : 10;/* Must be zero'ed */

        gctUINT        reserved4             : 16;/* Must be zero'ed */
        gctUINT        baseOpcodeBit6        : 1;
        gctUINT        reserved5             : 13;/* Must be zero'ed */
        gctUINT        instTypeBit1_2        : 2;

        gctUINT        reserved6             : 3; /* Must be zero'ed */
        gctUINT        bSrc2Valid            : 1; /* Must be valid */
        gctUINT        src2RegNo             : 9;
        gctUINT        dstRegNoBit7          : 1;
        gctUINT        src2Swizzle           : 8;
        gctUINT        bSrc2ModNeg           : 1;
        gctUINT        bSrc2ModAbs           : 1;
        gctUINT        dstRegNoBit8          : 1;
        gctUINT        src2RelAddr           : 3;
        gctUINT        src2Type              : 3;
        gctUINT        dstType               : 1;
    } inst;

    gctUINT            data[4];
}
VSC_MC_ALU_1_SRC_SRC2_INST;

/* Pack inst 0x71 */
typedef union _VSC_MC_PACK_INST
{
    struct
    {
        gctUINT        baseOpcodeBit0_5      : 6; /* Together with baseOpcodeBit6 to compose base-opcode */
        gctUINT        reserved0             : 5; /* Must be zero'ed */
        gctUINT        bDstModSat            : 1;
        gctUINT        bDstValid             : 1; /* Must be valid */
        gctUINT        dstRelAddr            : 3;
        gctUINT        dstRegNoBit0_6        : 7; /* Together with dstRegNoBit7 and dstRegNoBit8 to compose dstRegNo */
        gctUINT        writeMask             : 4;
        gctUINT        reserved1             : 5; /* Must be zero'ed */

        gctUINT        reserved2             : 3; /* Must be zero'ed */
        gctUINT        srcSelect             : 8;
        gctUINT        bSrc0Valid            : 1; /* Must be valid */
        gctUINT        src0RegNo             : 9;
        gctUINT        instTypeBit0          : 1; /* Together with instTypeBit1_2 to compose instType */
        gctUINT        src0Swizzle           : 8;
        gctUINT        bSrc0ModNeg           : 1;
        gctUINT        bSrc0ModAbs           : 1;

        gctUINT        src0RelAddr           : 3;
        gctUINT        src0Type              : 3;
        gctUINT        bSrc1Valid            : 1; /* Must be valid */
        gctUINT        src1RegNo             : 9;
        gctUINT        baseOpcodeBit6        : 1;
        gctUINT        src1Swizzle           : 8;
        gctUINT        bSrc1ModNeg           : 1;
        gctUINT        bSrc1ModAbs           : 1;
        gctUINT        src1RelAddr           : 3;
        gctUINT        instTypeBit1_2        : 2;

        gctUINT        src1Type              : 3;
        gctUINT        bSrc2Valid            : 1; /* Must be valid */
        gctUINT        src2RegNo             : 9;
        gctUINT        dstRegNoBit7          : 1;
        gctUINT        src2Swizzle           : 8;
        gctUINT        bSrc2ModNeg           : 1;
        gctUINT        bSrc2ModAbs           : 1;
        gctUINT        dstRegNoBit8          : 1;
        gctUINT        src2RelAddr           : 3;
        gctUINT        src2Type              : 3;
        gctUINT        dstType               : 1;
    } inst;

    gctUINT            data[4];
}
VSC_MC_PACK_INST;

/* Sample inst case */
typedef union _VSC_MC_SAMPLE_INST
{
    struct
    {
        gctUINT        baseOpcodeBit0_5      : 6; /* Together with baseOpcodeBit6 to compose base-opcode */
        gctUINT        reserved0             : 5; /* Must be zero'ed */
        gctUINT        bDstModSat            : 1;
        gctUINT        bDstValid             : 1; /* Must be valid */
        gctUINT        dstRelAddr            : 3;
        gctUINT        dstRegNoBit0_6        : 7; /* Together with dstRegNoBit7 and dstRegNoBit8 to compose dstRegNo */
        gctUINT        writeMask             : 4;
        gctUINT        samplerSlot           : 5;

        gctUINT        samplerRelAddr        : 3;
        gctUINT        samplerSwizzle        : 8;
        gctUINT        bSrc0Valid            : 1; /* Must be valid */
        gctUINT        src0RegNo             : 9;
        gctUINT        instTypeBit0          : 1; /* Together with instTypeBit1_2 to compose instType */
        gctUINT        src0Swizzle           : 8;
        gctUINT        bSrc0ModNeg           : 1;
        gctUINT        bSrc0ModAbs           : 1;

        gctUINT        src0RelAddr           : 3;
        gctUINT        src0Type              : 3;
        gctUINT        bSrc1Valid            : 1;
        gctUINT        src1RegNo             : 9;
        gctUINT        baseOpcodeBit6        : 1;
        gctUINT        src1Swizzle           : 8;
        gctUINT        bSrc1ModNeg           : 1;
        gctUINT        bSrc1ModAbs           : 1;
        gctUINT        src1RelAddr           : 3;
        gctUINT        instTypeBit1_2        : 2;

        gctUINT        src1Type              : 3;
        gctUINT        bSrc2Valid            : 1;
        gctUINT        src2RegNo             : 9;
        gctUINT        dstRegNoBit7          : 1;
        gctUINT        src2Swizzle           : 8;
        gctUINT        bSrc2ModNeg           : 1;
        gctUINT        bSrc2ModAbs           : 1;
        gctUINT        dstRegNoBit8          : 1;
        gctUINT        src2RelAddr           : 3;
        gctUINT        src2Type              : 3;
        gctUINT        dstType               : 1;
    } inst;

    gctUINT            data[4];
}
VSC_MC_SAMPLE_INST;

/* Extended sample inst case */
typedef union _VSC_MC_SAMPLE_EXT_INST
{
    struct
    {
        gctUINT        baseOpcodeBit0_5      : 6; /* Together with baseOpcodeBit6 to compose base-opcode */
        gctUINT        reserved0             : 5; /* Must be zero'ed */
        gctUINT        bDstModSat            : 1;
        gctUINT        bDstValid             : 1; /* Must be valid */
        gctUINT        dstRelAddr            : 3;
        gctUINT        dstRegNoBit0_6        : 7; /* Together with dstRegNoBit7 and dstRegNoBit8 to compose dstRegNo */
        gctUINT        writeMask             : 4;
        gctUINT        samplerSlot           : 5;

        gctUINT        samplerRelAddr        : 3;
        gctUINT        samplerSwizzle        : 8;
        gctUINT        bSrc0Valid            : 1; /* Must be valid */
        gctUINT        src0RegNo             : 9;
        gctUINT        instTypeBit0          : 1; /* Together with instTypeBit1_2 to compose instType */
        gctUINT        src0Swizzle           : 8;
        gctUINT        bSrc0ModNeg           : 1;
        gctUINT        bSrc0ModAbs           : 1;

        gctUINT        src0RelAddr           : 3;
        gctUINT        src0Type              : 3;
        gctUINT        bSrc1Valid            : 1;
        gctUINT        src1RegNo             : 9;
        gctUINT        baseOpcodeBit6        : 1;
        gctUINT        src1Swizzle           : 8;
        gctUINT        bSrc1ModNeg           : 1;
        gctUINT        bSrc1ModAbs           : 1;
        gctUINT        src1RelAddr           : 3;
        gctUINT        instTypeBit1_2        : 2;

        gctUINT        src1Type              : 3;
        gctUINT        reserved1             : 1; /* Must be zero'ed for non-extopcode mode, otherwise, see NOTE 4 */
        gctUINT        extOpcode             : 8;
        gctUINT        reserved2             : 1; /* Must be zero'ed */
        gctUINT        dstRegNoBit7          : 1;
        gctUINT        reserved3             : 10;/* Must be zero'ed */
        gctUINT        dstRegNoBit8          : 1;
        gctUINT        reserved4             : 6; /* Must be zero'ed for non-extopcode mode, otherwise, see NOTE 4 */
        gctUINT        dstType               : 1;
    } inst;

    gctUINT            data[4];
}
VSC_MC_SAMPLE_EXT_INST;

/* Load inst case */
typedef union _VSC_MC_LD_INST
{
    struct
    {
        gctUINT        baseOpcodeBit0_5      : 6; /* Together with baseOpcodeBit6 to compose base-opcode */
        gctUINT        reserved0             : 5; /* Must be zero'ed */
        gctUINT        bDstModSat            : 1;
        gctUINT        bDstValid             : 1; /* Must be valid */
        gctUINT        dstRelAddr            : 3;
        gctUINT        dstRegNoBit0_6        : 7; /* Together with dstRegNoBit7 and dstRegNoBit8 to compose dstRegNo */
        gctUINT        writeMask             : 4;
        gctUINT        reserved1             : 5; /* Must be zero'ed */

        gctUINT        reserved2             : 2; /* Must be zero'ed */
        gctUINT        bPackMode             : 1;
        gctUINT        offsetLeftShift       : 3; /* Act after X3, do (<< offsetLeftShift) */
        gctUINT        bOffsetX3             : 1;
        gctUINT        bSkipForHelperKickoff : 1;
        gctUINT        bAccessLocalStorage   : 1;
        gctUINT        instTypeBit3          : 1;
        gctUINT        bDenorm               : 1;
        gctUINT        bSrc0Valid            : 1; /* Must be valid */
        gctUINT        src0RegNo             : 9;
        gctUINT        instTypeBit0          : 1; /* Together with instTypeBit1_2 and instTypeBit3 to compose instType */
        gctUINT        src0Swizzle           : 8;
        gctUINT        bSrc0ModNeg           : 1;
        gctUINT        bSrc0ModAbs           : 1;

        gctUINT        src0RelAddr           : 3;
        gctUINT        src0Type              : 3;
        gctUINT        bSrc1Valid            : 1; /* Must be valid */
        gctUINT        src1RegNo             : 9;
        gctUINT        baseOpcodeBit6        : 1;
        gctUINT        src1Swizzle           : 8;
        gctUINT        bSrc1ModNeg           : 1;
        gctUINT        bSrc1ModAbs           : 1;
        gctUINT        src1RelAddr           : 3;
        gctUINT        instTypeBit1_2        : 2;

        gctUINT        src1Type              : 3;
        gctUINT        reserved4             : 10;/* Must be zero'ed */
        gctUINT        dstRegNoBit7          : 1;
        gctUINT        reserved5             : 10;/* Must be zero'ed */
        gctUINT        dstRegNoBit8          : 1;
        gctUINT        reserved6             : 6; /* Must be zero'ed */
        gctUINT        dstType               : 1;
    } inst;

    gctUINT            data[4];
}
VSC_MC_LD_INST;

/* Image-load inst case */
typedef union _VSC_MC_IMG_LD_INST
{
    struct
    {
        gctUINT        baseOpcodeBit0_5      : 6; /* Together with baseOpcodeBit6 to compose base-opcode */
        gctUINT        reserved0             : 5; /* Must be zero'ed */
        gctUINT        bDstModSat            : 1;
        gctUINT        bDstValid             : 1; /* Must be valid */
        gctUINT        dstRelAddr            : 3;
        gctUINT        dstRegNoBit0_6        : 7; /* Together with dstRegNoBit7 and dstRegNoBit8 to compose dstRegNo */
        gctUINT        writeMask             : 4;
        gctUINT        reserved1             : 5; /* Must be zero'ed */

        gctUINT        reserved2             : 7; /* Must be zero'ed for non-evis mode, otherwise, see NOTE 9 */
        gctUINT        bSkipForHelperKickoff : 1;
        gctUINT        bAccessLocalStorage   : 1;
        gctUINT        reserved3             : 2; /* Must be zero'ed */
        gctUINT        bSrc0Valid            : 1; /* Must be valid */
        gctUINT        src0RegNo             : 9;
        gctUINT        instTypeBit0          : 1; /* Together with instTypeBit1_2 to compose instType */
        gctUINT        src0Swizzle           : 8;
        gctUINT        bSrc0ModNeg           : 1;
        gctUINT        bSrc0ModAbs           : 1;

        gctUINT        src0RelAddr           : 3;
        gctUINT        src0Type              : 3;
        gctUINT        bSrc1Valid            : 1; /* Must be valid */
        gctUINT        src1RegNo             : 9;
        gctUINT        baseOpcodeBit6        : 1;
        gctUINT        src1Swizzle           : 8;
        gctUINT        bSrc1ModNeg           : 1;
        gctUINT        bSrc1ModAbs           : 1;
        gctUINT        src1RelAddr           : 3;
        gctUINT        instTypeBit1_2        : 2;

        gctUINT        src1Type              : 3;
        gctUINT        bSrc2Valid            : 1; /* Must be valid */
        gctUINT        src2RegNo             : 9;
        gctUINT        dstRegNoBit7          : 1;
        gctUINT        src2Swizzle           : 8;
        gctUINT        bSrc2ModNeg           : 1;
        gctUINT        bSrc2ModAbs           : 1;
        gctUINT        dstRegNoBit8          : 1;
        gctUINT        src2RelAddr           : 3;
        gctUINT        src2Type              : 3;
        gctUINT        dstType               : 1;
    } inst;

    gctUINT            data[4];
}
VSC_MC_IMG_LD_INST;

/* Store inst case */
typedef union _VSC_MC_ST_INST
{
    struct
    {
        gctUINT        baseOpcodeBit0_5      : 6; /* Together with baseOpcodeBit6 to compose base-opcode */
        gctUINT        reserved0             : 17;/* Must be zero'ed */
        gctUINT        writeMask             : 4;
        gctUINT        reserved1             : 5; /* Must be zero'ed */

        gctUINT        reserved2             : 2; /* Must be zero'ed */
        gctUINT        bPackMode             : 1;
        gctUINT        offsetLeftShift       : 3; /* Act after X3, do (<< offsetLeftShift) */
        gctUINT        bOffsetX3             : 1;
        gctUINT        bSkipForHelperKickoff : 1;
        gctUINT        bAccessLocalStorage   : 1;
        gctUINT        instTypeBit3          : 1;
        gctUINT        bDenorm               : 1;
        gctUINT        bSrc0Valid            : 1; /* Must be valid */
        gctUINT        src0RegNo             : 9;
        gctUINT        instTypeBit0          : 1; /* Together with instTypeBit1_2 and instTypeBit3 to compose instType */
        gctUINT        src0Swizzle           : 8;
        gctUINT        bSrc0ModNeg           : 1;
        gctUINT        bSrc0ModAbs           : 1;

        gctUINT        src0RelAddr           : 3;
        gctUINT        src0Type              : 3;
        gctUINT        bSrc1Valid            : 1; /* Must be valid */
        gctUINT        src1RegNo             : 9;
        gctUINT        baseOpcodeBit6        : 1;
        gctUINT        src1Swizzle           : 8;
        gctUINT        bSrc1ModNeg           : 1;
        gctUINT        bSrc1ModAbs           : 1;
        gctUINT        src1RelAddr           : 3;
        gctUINT        instTypeBit1_2        : 2;

        gctUINT        src1Type              : 3;
        gctUINT        bSrc2Valid            : 1; /* Must be valid */
        gctUINT        src2RegNo             : 9;
        gctUINT        threadTypeBit0        : 1;
        gctUINT        src2Swizzle           : 8;
        gctUINT        bSrc2ModNeg           : 1;
        gctUINT        bSrc2ModAbs           : 1;
        gctUINT        threadTypeBit1        : 1;
        gctUINT        src2RelAddr           : 3;
        gctUINT        src2Type              : 3;
        gctUINT        reserved4             : 1; /* Must be zero'ed */
    } inst;

    gctUINT            data[4];
}
VSC_MC_ST_INST;

/* Image-store inst case */
typedef union _VSC_MC_IMG_ST_INST
{
    struct
    {
        gctUINT        baseOpcodeBit0_5      : 6; /* Together with baseOpcodeBit6 to compose base-opcode */
        gctUINT        reserved0             : 17;/* Must be zero'ed */
        gctUINT        writeMask             : 4;
        gctUINT        reserved1             : 5; /* Must be zero'ed */

        gctUINT        reserved2             : 7; /* Must be zero'ed for non-evis mode, otherwise, see NOTE 9 */
        gctUINT        bSkipForHelperKickoff : 1;
        gctUINT        bAccessLocalStorage   : 1;
        gctUINT        reserved3             : 2; /* Must be zero'ed */
        gctUINT        bSrc0Valid            : 1; /* Must be valid */
        gctUINT        src0RegNo             : 9;
        gctUINT        instTypeBit0          : 1; /* Together with instTypeBit1_2 to compose instType */
        gctUINT        src0Swizzle           : 8;
        gctUINT        bSrc0ModNeg           : 1;
        gctUINT        bSrc0ModAbs           : 1;

        gctUINT        src0RelAddr           : 3;
        gctUINT        src0Type              : 3;
        gctUINT        bSrc1Valid            : 1; /* Must be valid */
        gctUINT        src1RegNo             : 9;
        gctUINT        baseOpcodeBit6        : 1;
        gctUINT        src1Swizzle           : 8;
        gctUINT        bSrc1ModNeg           : 1;
        gctUINT        bSrc1ModAbs           : 1;
        gctUINT        src1RelAddr           : 3;
        gctUINT        instTypeBit1_2        : 2;

        gctUINT        src1Type              : 3;
        gctUINT        bSrc2Valid            : 1; /* Must be valid */
        gctUINT        src2RegNo             : 9;
        gctUINT        threadTypeBit0        : 1;
        gctUINT        src2Swizzle           : 8;
        gctUINT        bSrc2ModNeg           : 1;
        gctUINT        bSrc2ModAbs           : 1;
        gctUINT        threadTypeBit1        : 1;
        gctUINT        src2RelAddr           : 3;
        gctUINT        src2Type              : 3;
        gctUINT        reserved4             : 1; /* Must be zero'ed */
    } inst;

    gctUINT            data[4];
}
VSC_MC_IMG_ST_INST;

/* Image-atomic inst case */
typedef union _VSC_MC_IMG_ATOM_INST
{
    struct
    {
        gctUINT        baseOpcodeBit0_5      : 6; /* Together with baseOpcodeBit6 to compose base-opcode */
        gctUINT        reserved0             : 5; /* Must be zero'ed */
        gctUINT        bDstModSat            : 1;
        gctUINT        bDstValid             : 1; /* Must be valid */
        gctUINT        dstRelAddr            : 3;
        gctUINT        dstRegNoBit0_6        : 7; /* Together with dstRegNoBit7 and dstRegNoBit8 to compose dstRegNo */
        gctUINT        writeMask             : 4;
        gctUINT        reserved1             : 5; /* Must be zero'ed */

        gctUINT        reserved2             : 3; /* Must be zero'ed */
        gctUINT        atomicMode            : 3;
        gctUINT        b3dImgMode            : 1;
        gctUINT        bSkipForHelperKickoff : 1;
        gctUINT        bAccessLocalStorage   : 1;
        gctUINT        reserved3             : 2; /* Must be zero'ed */
        gctUINT        bSrc0Valid            : 1; /* Must be valid */
        gctUINT        src0RegNo             : 9;
        gctUINT        instTypeBit0          : 1; /* Together with instTypeBit1_2 to compose instType */
        gctUINT        src0Swizzle           : 8;
        gctUINT        bSrc0ModNeg           : 1;
        gctUINT        bSrc0ModAbs           : 1;

        gctUINT        src0RelAddr           : 3;
        gctUINT        src0Type              : 3;
        gctUINT        bSrc1Valid            : 1; /* Must be valid */
        gctUINT        src1RegNo             : 9;
        gctUINT        baseOpcodeBit6        : 1;
        gctUINT        src1Swizzle           : 8;
        gctUINT        bSrc1ModNeg           : 1;
        gctUINT        bSrc1ModAbs           : 1;
        gctUINT        src1RelAddr           : 3;
        gctUINT        instTypeBit1_2        : 2;

        gctUINT        src1Type              : 3;
        gctUINT        bSrc2Valid            : 1; /* Must be valid */
        gctUINT        src2RegNo             : 9;
        gctUINT        dstRegNoBit7          : 1;
        gctUINT        src2Swizzle           : 8;
        gctUINT        bSrc2ModNeg           : 1;
        gctUINT        bSrc2ModAbs           : 1;
        gctUINT        dstRegNoBit8          : 1;
        gctUINT        src2RelAddr           : 3;
        gctUINT        src2Type              : 3;
        gctUINT        dstType               : 1;
    } inst;

    gctUINT            data[4];
}
VSC_MC_IMG_ATOM_INST;

/* Store_attr inst 0x42 */
typedef union _VSC_MC_STORE_ATTR_INST
{
    struct
    {
        gctUINT        baseOpcodeBit0_5      : 6; /* Together with baseOpcodeBit6 to compose base-opcode */
        gctUINT        reserved0             : 17;/* Must be zero'ed */
        gctUINT        writeMask             : 4;
        gctUINT        reserved1             : 3; /* Must be zero'ed */
        gctUINT        bNeedUscSync          : 1;
        gctUINT        reserved2             : 1; /* Must be zero'ed */

        gctUINT        reserved3             : 11;/* Must be zero'ed */
        gctUINT        bSrc0Valid            : 1; /* Must be valid */
        gctUINT        src0RegNo             : 9;
        gctUINT        instTypeBit0          : 1; /* Together with instTypeBit1_2 to compose instType */
        gctUINT        src0Swizzle           : 8;
        gctUINT        bSrc0ModNeg           : 1;
        gctUINT        bSrc0ModAbs           : 1;

        gctUINT        src0RelAddr           : 3;
        gctUINT        src0Type              : 3;
        gctUINT        bSrc1Valid            : 1; /* Must be valid */
        gctUINT        src1RegNo             : 9;
        gctUINT        baseOpcodeBit6        : 1;
        gctUINT        src1Swizzle           : 8;
        gctUINT        bSrc1ModNeg           : 1;
        gctUINT        bSrc1ModAbs           : 1;
        gctUINT        src1RelAddr           : 3;
        gctUINT        instTypeBit1_2        : 2;

        gctUINT        src1Type              : 3;
        gctUINT        bSrc2Valid            : 1; /* Must be valid */
        gctUINT        src2RegNo             : 9;
        gctUINT        threadTypeBit0        : 1;
        gctUINT        src2Swizzle           : 8;
        gctUINT        bSrc2ModNeg           : 1;
        gctUINT        bSrc2ModAbs           : 1;
        gctUINT        threadTypeBit1        : 1;
        gctUINT        src2RelAddr           : 3;
        gctUINT        src2Type              : 3;
        gctUINT        reserved4             : 1; /* Must be zero'ed */
    } inst;

    gctUINT            data[4];
}
VSC_MC_STORE_ATTR_INST;

/* Select_map inst 0x43 */
typedef union _VSC_MC_SELECT_MAP_INST
{
    struct
    {
        gctUINT        baseOpcodeBit0_5      : 6; /* Together with baseOpcodeBit6 to compose base-opcode */
        gctUINT        reserved0             : 5; /* Must be zero'ed */
        gctUINT        bDstModSat            : 1;
        gctUINT        bDstValid             : 1; /* Must be valid, see rangeToMatch for more info */
        gctUINT        dstRelAddr            : 3;
        gctUINT        dstRegNoBit0_6        : 7; /* Together with dstRegNoBit7 and dstRegNoBit8 to compose dstRegNo */
        gctUINT        writeMask             : 4;
        gctUINT        reserved1             : 5; /* Must be zero'ed */

        gctUINT        reserved2             : 3; /* Must be zero'ed */
        gctUINT        rangeToMatch          : 4; /* Only the range is matched, the dst will be written */
        gctUINT        reserved3             : 3; /* Must be zero'ed */
        gctUINT        bCompSel              : 1; /* If TRUE, component select, otherwise, src select */
        gctUINT        bSrc0Valid            : 1; /* Must be valid */
        gctUINT        src0RegNo             : 9;
        gctUINT        instTypeBit0          : 1; /* Together with instTypeBit1_2 to compose instType */
        gctUINT        src0Swizzle           : 8;
        gctUINT        bSrc0ModNeg           : 1;
        gctUINT        bSrc0ModAbs           : 1;

        gctUINT        src0RelAddr           : 3;
        gctUINT        src0Type              : 3;
        gctUINT        bSrc1Valid            : 1; /* Must be valid */
        gctUINT        src1RegNo             : 9;
        gctUINT        baseOpcodeBit6        : 1;
        gctUINT        src1Swizzle           : 8;
        gctUINT        bSrc1ModNeg           : 1;
        gctUINT        bSrc1ModAbs           : 1;
        gctUINT        src1RelAddr           : 3;
        gctUINT        instTypeBit1_2        : 2;

        gctUINT        src1Type              : 3;
        gctUINT        bSrc2Valid            : 1; /* Must be valid */
        gctUINT        src2RegNo             : 9;
        gctUINT        dstRegNoBit7          : 1;
        gctUINT        src2Swizzle           : 8;
        gctUINT        bSrc2ModNeg           : 1;
        gctUINT        bSrc2ModAbs           : 1;
        gctUINT        dstRegNoBit8          : 1;
        gctUINT        src2RelAddr           : 3;
        gctUINT        src2Type              : 3;
        gctUINT        dstType               : 1;
    } inst;

    gctUINT            data[4];
}
VSC_MC_SELECT_MAP_INST;

/* Direct-branch inst case 0, this is common direct case, can not be used under dual16 */
typedef union _VSC_MC_DIRECT_BRANCH_0_INST
{
    struct
    {
        gctUINT        baseOpcodeBit0_5      : 6; /* Together with baseOpcodeBit6 to compose base-opcode */
        gctUINT        condOpCode            : 5;
        gctUINT        reserved0             : 21;/* Must be zero'ed */

        gctUINT        reserved1             : 2; /* Must be zero'ed */
        gctUINT        bPackMode             : 1;
        gctUINT        reserved2             : 8; /* Must be zero'ed */
        gctUINT        bSrc0Valid            : 1;
        gctUINT        src0RegNo             : 9;
        gctUINT        instTypeBit0          : 1; /* Together with instTypeBit1_2 to compose instType */
        gctUINT        src0Swizzle           : 8;
        gctUINT        bSrc0ModNeg           : 1;
        gctUINT        bSrc0ModAbs           : 1;

        gctUINT        src0RelAddr           : 3;
        gctUINT        src0Type              : 3;
        gctUINT        bSrc1Valid            : 1;
        gctUINT        src1RegNo             : 9;
        gctUINT        baseOpcodeBit6        : 1;
        gctUINT        src1Swizzle           : 8;
        gctUINT        bSrc1ModNeg           : 1;
        gctUINT        bSrc1ModAbs           : 1;
        gctUINT        src1RelAddr           : 3;
        gctUINT        instTypeBit1_2        : 2;

        gctUINT        src1Type              : 3;
        gctUINT        reserved3             : 1; /* Must be zero'ed */
        gctUINT        loopOpType            : 1; /* Must be 0x0 under dual16 mode */
        gctUINT        reserved4             : 2; /* Must be zero'ed */
        gctUINT        branchTarget          : 20;
        gctUINT        reserved5             : 5; /* Must be zero'ed */
    } inst;

    gctUINT            data[4];
}
VSC_MC_DIRECT_BRANCH_0_INST;

/* Direct-branch inst case 1, can be used under dual16 */
typedef union _VSC_MC_DIRECT_BRANCH_1_INST
{
    struct
    {
        gctUINT        baseOpcodeBit0_5      : 6; /* Together with baseOpcodeBit6 to compose base-opcode */
        gctUINT        condOpCode            : 5;
        gctUINT        reserved0             : 21;/* Must be zero'ed */

        gctUINT        reserved1             : 2; /* Must be zero'ed */
        gctUINT        bPackMode             : 1;
        gctUINT        reserved2             : 8; /* Must be zero'ed */
        gctUINT        bSrc0Valid            : 1;
        gctUINT        src0RegNo             : 9;
        gctUINT        instTypeBit0          : 1; /* Together with instTypeBit1_2 to compose instType */
        gctUINT        src0Swizzle           : 8;
        gctUINT        bSrc0ModNeg           : 1;
        gctUINT        bSrc0ModAbs           : 1;

        gctUINT        src0RelAddr           : 3;
        gctUINT        src0Type              : 3;
        gctUINT        bSrc1Valid            : 1;
        gctUINT        src1RegNo             : 9;
        gctUINT        baseOpcodeBit6        : 1;
        gctUINT        src1Swizzle           : 8;
        gctUINT        bSrc1ModNeg           : 1;
        gctUINT        bSrc1ModAbs           : 1;
        gctUINT        src1RelAddr           : 3;
        gctUINT        instTypeBit1_2        : 2;

        gctUINT        src1Type              : 3;
        gctUINT        bSrc2Valid            : 1; /* Must be valid */
        gctUINT        branchTargetBit0_8    : 9;
        gctUINT        threadTypeBit0        : 1;
        gctUINT        branchTargetBit9_18   : 10;
        gctUINT        threadTypeBit1        : 1;
        gctUINT        branchTargetBit19     : 1;
        gctUINT        immType               : 2;
        gctUINT        src2Type              : 3; /* Must be set to 0x7 */
        gctUINT        reserved3             : 1; /* Must be zero'ed */
    } inst;

    gctUINT            data[4];
}
VSC_MC_DIRECT_BRANCH_1_INST;

/* Indirect-branch inst case, can be used under dual16 */
typedef union _VSC_MC_INDIRECT_BRANCH_INST
{
    struct
    {
        gctUINT        baseOpcodeBit0_5      : 6; /* Together with baseOpcodeBit6 to compose base-opcode */
        gctUINT        condOpCode            : 5; /* Must be set to 0x00 */
        gctUINT        reserved0             : 21;/* Must be zero'ed */

        gctUINT        reserved1             : 2; /* Must be zero'ed */
        gctUINT        bPackMode             : 1;
        gctUINT        reserved2             : 18;/* Must be zero'ed */
        gctUINT        instTypeBit0          : 1; /* Together with instTypeBit1_2 to compose instType */
        gctUINT        reserved3             : 10;/* Must be zero'ed */

        gctUINT        reserved4             : 16;/* Must be zero'ed */
        gctUINT        baseOpcodeBit6        : 1;
        gctUINT        reserved5             : 13;/* Must be zero'ed */
        gctUINT        instTypeBit1_2        : 2;

        gctUINT        reserved6             : 3; /* Must be zero'ed */
        gctUINT        bSrc2Valid            : 1; /* Must be valid */
        gctUINT        src2RegNo             : 9;
        gctUINT        threadTypeBit0        : 1;
        gctUINT        src2Swizzle           : 8;
        gctUINT        bSrc2ModNeg           : 1;
        gctUINT        bSrc2ModAbs           : 1;
        gctUINT        threadTypeBit1        : 1;
        gctUINT        src2RelAddr           : 3;
        gctUINT        src2Type              : 3;
        gctUINT        reserved7             : 1; /* Must be zero'ed */
    } inst;

    gctUINT            data[4];
}
VSC_MC_INDIRECT_BRANCH_INST;

/* Direct-call inst case, can not be used under dual16 */
typedef union _VSC_MC_DIRECT_CALL_INST
{
    struct
    {
        gctUINT        baseOpcodeBit0_5      : 6; /* Together with baseOpcodeBit6 to compose base-opcode */
        gctUINT        reserved0             : 26;/* Must be zero'ed */

        gctUINT        reserved1             : 32;/* Must be zero'ed */

        gctUINT        reserved2             : 16;/* Must be zero'ed */
        gctUINT        baseOpcodeBit6        : 1;
        gctUINT        reserved3             : 15;/* Must be zero'ed */

        gctUINT        reserved4             : 7; /* Must be zero'ed */
        gctUINT        callTarget            : 20;
        gctUINT        reserved5             : 5; /* Must be zero'ed */
    } inst;

    gctUINT            data[4];
}
VSC_MC_DIRECT_CALL_INST;

/* Indirect-call inst case, can be used under dual16 */
typedef union _VSC_MC_INDIRECT_CALL_INST
{
    struct
    {
        gctUINT        baseOpcodeBit0_5      : 6; /* Together with baseOpcodeBit6 to compose base-opcode */
        gctUINT        reserved0             : 26;/* Must be zero'ed */

        gctUINT        reserved1             : 21;/* Must be zero'ed */
        gctUINT        instTypeBit0          : 1; /* Together with instTypeBit1_2 to compose instType */
        gctUINT        reserved2             : 10;

        gctUINT        reserved3             : 16;/* Must be zero'ed */
        gctUINT        baseOpcodeBit6        : 1;
        gctUINT        reserved4             : 13;/* Must be zero'ed */
        gctUINT        instTypeBit1_2        : 2;

        gctUINT        reserved5             : 3; /* Must be zero'ed */
        gctUINT        bSrc2Valid            : 1; /* Must be valid */
        gctUINT        src2RegNo             : 9;
        gctUINT        threadTypeBit0        : 1;
        gctUINT        src2Swizzle           : 8;
        gctUINT        bSrc2ModNeg           : 1;
        gctUINT        bSrc2ModAbs           : 1;
        gctUINT        threadTypeBit1        : 1;
        gctUINT        src2RelAddr           : 3;
        gctUINT        src2Type              : 3;
        gctUINT        reserved6             : 1; /* Must be zero'ed */
    } inst;

    gctUINT            data[4];
}
VSC_MC_INDIRECT_CALL_INST;

/* Loop/rep inst case */
typedef union _VSC_MC_LOOP_INST
{
    struct
    {
        gctUINT        baseOpcodeBit0_5      : 6; /* Together with baseOpcodeBit6 to compose base-opcode */
        gctUINT        reserved0             : 26;/* Must be zero'ed */

        gctUINT        reserved1             : 21;/* Must be zero'ed */
        gctUINT        instTypeBit0          : 1; /* Together with instTypeBit1_2 to compose instType */
        gctUINT        reserved2             : 10;

        gctUINT        reserved3             : 6;
        gctUINT        bSrc1Valid            : 1; /* Must be valid */
        gctUINT        src1RegNo             : 9;
        gctUINT        baseOpcodeBit6        : 1;
        gctUINT        src1Swizzle           : 8;
        gctUINT        bSrc1ModNeg           : 1;
        gctUINT        bSrc1ModAbs           : 1;
        gctUINT        src1RelAddr           : 3;
        gctUINT        instTypeBit1_2        : 2;

        gctUINT        src1Type              : 3;
        gctUINT        reserved4             : 4; /* Must be zero'ed */
        gctUINT        branchTarget          : 20;
        gctUINT        reserved5             : 5; /* Must be zero'ed */
    } inst;

    gctUINT            data[4];
}
VSC_MC_LOOP_INST;

/* Emit inst case */
typedef union _VSC_MC_EMIT_INST
{
    struct
    {
        gctUINT        baseOpcodeBit0_5      : 6; /* Together with baseOpcodeBit6 to compose base-opcode */
        gctUINT        reserved0             : 5; /* Must be zero'ed */
        gctUINT        bDstModSat            : 1;
        gctUINT        bDstValid             : 1; /* Must be valid */
        gctUINT        dstRelAddr            : 3;
        gctUINT        dstRegNoBit0_6        : 7; /* Together with dstRegNoBit7 and dstRegNoBit8 to compose dstRegNo */
        gctUINT        writeMask             : 4;
        gctUINT        reserved1             : 5; /* Must be zero'ed */

        gctUINT        reserved2             : 3; /* Must be zero'ed */
        gctUINT        bNeedRestartPrim      : 1;
        gctUINT        reserved3             : 7; /* Must be zero'ed */
        gctUINT        bSrc0Valid            : 1; /* Must be valid */
        gctUINT        src0RegNo             : 9;
        gctUINT        instTypeBit0          : 1; /* Together with instTypeBit1_2 to compose instType */
        gctUINT        src0Swizzle           : 8;
        gctUINT        bSrc0ModNeg           : 1;
        gctUINT        bSrc0ModAbs           : 1;

        gctUINT        src0RelAddr           : 3;
        gctUINT        src0Type              : 3;
        gctUINT        bSrc1Valid            : 1;
        gctUINT        src1RegNo             : 9;
        gctUINT        baseOpcodeBit6        : 1;
        gctUINT        src1Swizzle           : 8;
        gctUINT        bSrc1ModNeg           : 1;
        gctUINT        bSrc1ModAbs           : 1;
        gctUINT        src1RelAddr           : 3;
        gctUINT        instTypeBit1_2        : 2;

        gctUINT        src1Type              : 3;
        gctUINT        reserved4             : 1; /* Must be zero'ed for non-extopcode mode, otherwise, see NOTE 4 */
        gctUINT        extOpcode             : 8;
        gctUINT        reserved5             : 1; /* Must be zero'ed */
        gctUINT        dstRegNoBit7          : 1;
        gctUINT        reserved6             : 10;/* Must be zero'ed */
        gctUINT        dstRegNoBit8          : 1;
        gctUINT        reserved7             : 6; /* Must be zero'ed for non-extopcode mode, otherwise, see NOTE 4 */
        gctUINT        dstType               : 1;
    } inst;

    gctUINT            data[4];
}
VSC_MC_EMIT_INST;

/* Evis-mode inst case */
typedef union _VSC_MC_EVIS_INST
{
    struct
    {
        gctUINT        baseOpcodeBit0_5      : 6; /* Together with baseOpcodeBit6 to compose base-opcode, and base-opcode must be 0x45 */
        gctUINT        reserved0             : 5; /* Must be zero'ed */
        gctUINT        bDstModSat            : 1;
        gctUINT        bDstValid             : 1; /* Must be valid */
        gctUINT        extEvisOpCodeBit0_2   : 3; /* Together with extEvisOpCodeBit3 and extEvisOpCodeBit4_5 to extEvisOpCode */
        gctUINT        dstRegNoBit0_6        : 7; /* Together with dstRegNoBit7 and dstRegNoBit8 to compose dstRegNo */
        gctUINT        startDstCompIdx       : 4;
        gctUINT        endDstCompIdx         : 4;
        gctUINT        extEvisOpCodeBit3     : 1;

        gctUINT        extEvisOpCodeBit4_5   : 2;
        gctUINT        evisState             : 9;
        gctUINT        bSrc0Valid            : 1; /* Must be valid */
        gctUINT        src0RegNo             : 9;
        gctUINT        instTypeBit0          : 1; /* Together with instTypeBit1_2 to compose instType */
        gctUINT        startSrcCompIdx       : 4; /* AKA sourceBin */
        gctUINT        reserved1             : 6; /* Must be zero'ed */

        gctUINT        src0RelAddr           : 3;
        gctUINT        src0Type              : 3;
        gctUINT        bSrc1Valid            : 1; /* Must be valid */
        gctUINT        src1RegNo             : 9;
        gctUINT        baseOpcodeBit6        : 1;
        gctUINT        src1Swizzle           : 8;
        gctUINT        reserved2             : 2; /* Must be zero'ed */
        gctUINT        src1RelAddr           : 3;
        gctUINT        instTypeBit1_2        : 2;

        gctUINT        src1Type              : 3;
        gctUINT        bSrc2Valid            : 1;
        gctUINT        src2RegNo             : 9;
        gctUINT        dstRegNoBit7          : 1;
        gctUINT        src2Swizzle           : 8;
        gctUINT        reserved3             : 2; /* Must be zero'ed */
        gctUINT        dstRegNoBit8          : 1;
        gctUINT        src2RelAddr           : 3;
        gctUINT        src2Type              : 3;
        gctUINT        dstType               : 1;
    } inst;

    gctUINT            data[4];
}
VSC_MC_EVIS_INST;

typedef union _VSC_MC_INST
{
    VSC_MC_RAW_INST                    raw_inst;
    VSC_MC_NO_OPERAND_INST             no_opnd_inst;
    VSC_MC_ALU_3_SRCS_INST             tri_srcs_alu_inst;
    VSC_MC_ALU_2_SRCS_SRC0_SRC1_INST   bin_srcs_src0_src1_alu_inst;
    VSC_MC_ALU_2_SRCS_SRC0_SRC2_INST   bin_srcs_src0_src2_alu_inst;
    VSC_MC_ALU_2_SRCS_SRC1_SRC2_INST   bin_srcs_src1_src2_alu_inst;
    VSC_MC_ALU_1_SRC_SRC0_INST         una_src_src0_alu_inst;
    VSC_MC_ALU_1_SRC_SRC1_INST         una_src_src1_alu_inst;
    VSC_MC_ALU_1_SRC_SRC2_INST         una_src_src2_alu_inst;
    VSC_MC_PACK_INST                   pack_inst;
    VSC_MC_SAMPLE_INST                 sample_inst;
    VSC_MC_SAMPLE_EXT_INST             sample_ext_inst;
    VSC_MC_LD_INST                     load_inst;
    VSC_MC_IMG_LD_INST                 img_load_inst;
    VSC_MC_ST_INST                     store_inst;
    VSC_MC_IMG_ST_INST                 img_store_inst;
    VSC_MC_IMG_ATOM_INST               img_atom_inst;
    VSC_MC_STORE_ATTR_INST             store_attr_inst;
    VSC_MC_SELECT_MAP_INST             select_map_inst;
    VSC_MC_DIRECT_BRANCH_0_INST        direct_branch0_inst;
    VSC_MC_DIRECT_BRANCH_1_INST        direct_branch1_inst;
    VSC_MC_INDIRECT_BRANCH_INST        indirect_branch_inst;
    VSC_MC_DIRECT_CALL_INST            direct_call_inst;
    VSC_MC_INDIRECT_CALL_INST          indirect_call_inst;
    VSC_MC_LOOP_INST                   loop_inst;
    VSC_MC_EMIT_INST                   emit_inst;
    VSC_MC_EVIS_INST                   evis_inst;
}VSC_MC_INST;

typedef enum _VSC_MC_CODEC_TYPE
{
    VSC_MC_CODEC_TYPE_UNKNOWN                = 0,
    VSC_MC_CODEC_TYPE_NO_OPND,
    VSC_MC_CODEC_TYPE_3_SRCS_ALU,
    VSC_MC_CODEC_TYPE_2_SRCS_SRC0_SRC1_ALU,
    VSC_MC_CODEC_TYPE_2_SRCS_SRC0_SRC2_ALU,
    VSC_MC_CODEC_TYPE_2_SRCS_SRC1_SRC2_ALU,
    VSC_MC_CODEC_TYPE_1_SRC_SRC0_ALU,
    VSC_MC_CODEC_TYPE_1_SRC_SRC1_ALU,
    VSC_MC_CODEC_TYPE_1_SRC_SRC2_ALU,
    VSC_MC_CODEC_TYPE_PACK,
    VSC_MC_CODEC_TYPE_SAMPLE,
    VSC_MC_CODEC_TYPE_SAMPLE_EXT,
    VSC_MC_CODEC_TYPE_LOAD,
    VSC_MC_CODEC_TYPE_IMG_LOAD,
    VSC_MC_CODEC_TYPE_STORE,
    VSC_MC_CODEC_TYPE_IMG_STORE,
    VSC_MC_CODEC_TYPE_IMG_ATOM,
    VSC_MC_CODEC_TYPE_STORE_ATTR,
    VSC_MC_CODEC_TYPE_SELECT_MAP,
    VSC_MC_CODEC_TYPE_DIRECT_BRANCH_0,
    VSC_MC_CODEC_TYPE_DIRECT_BRANCH_1,
    VSC_MC_CODEC_TYPE_INDIRECT_BRANCH,
    VSC_MC_CODEC_TYPE_DIRECT_CALL,
    VSC_MC_CODEC_TYPE_INDIRECT_CALL,
    VSC_MC_CODEC_TYPE_LOOP,
    VSC_MC_CODEC_TYPE_EMIT,
    VSC_MC_CODEC_TYPE_EVIS
}VSC_MC_CODEC_TYPE;

typedef gctBOOL (*PFN_MC_ENCODER)(VSC_MC_CODEC*, VSC_MC_CODEC_TYPE, VSC_MC_CODEC_INST*, VSC_MC_INST*);
typedef gctBOOL (*PFN_MC_DECODER)(VSC_MC_CODEC*, VSC_MC_CODEC_TYPE, VSC_MC_INST*, VSC_MC_CODEC_INST*);

typedef enum _VSC_MC_CODEC_MODE
{
    VSC_MC_CODEC_MODE_ENCODE,
    VSC_MC_CODEC_MODE_DECODE
}VSC_MC_CODEC_MODE;

static VSC_MC_CODEC_TYPE _GetBranchCodecType(VSC_MC_CODEC* pMcCodec,
                                             void* pRefInInst,
                                             VSC_MC_CODEC_MODE codecMode)
{
    VSC_MC_CODEC_INST* pInCodecHelperInst = (VSC_MC_CODEC_INST*)pRefInInst;
    VSC_MC_INST*       pMcInst = (VSC_MC_INST*)pRefInInst;

    if (codecMode == VSC_MC_CODEC_MODE_ENCODE)
    {
        if (pInCodecHelperInst->src[pInCodecHelperInst->srcCount - 1].regType ==
            0x7)
        {
            if (pInCodecHelperInst->src[pInCodecHelperInst->srcCount - 1].u.imm.immType ==
                0x2)
            {
                if (pMcCodec->bDual16ModeEnabled)
                {
                    return VSC_MC_CODEC_TYPE_DIRECT_BRANCH_1;
                }
                else
                {
                    return VSC_MC_CODEC_TYPE_DIRECT_BRANCH_0;
                }
            }
            else
            {
                return VSC_MC_CODEC_TYPE_DIRECT_BRANCH_1;
            }
        }
        else
        {
            return VSC_MC_CODEC_TYPE_INDIRECT_BRANCH;
        }
    }
    else
    {
        if (pMcInst->tri_srcs_alu_inst.inst.bSrc2Valid)
        {
            if (pMcInst->tri_srcs_alu_inst.inst.src2Type == 0x7)
            {
                return VSC_MC_CODEC_TYPE_DIRECT_BRANCH_1;
            }
            else
            {
                return VSC_MC_CODEC_TYPE_INDIRECT_BRANCH;
            }
        }
        else
        {
            return VSC_MC_CODEC_TYPE_DIRECT_BRANCH_0;
        }
    }
}

static VSC_MC_CODEC_TYPE _GetCallCodecType(VSC_MC_CODEC* pMcCodec,
                                           void* pRefInInst,
                                           VSC_MC_CODEC_MODE codecMode)
{
    VSC_MC_CODEC_INST* pInCodecHelperInst = (VSC_MC_CODEC_INST*)pRefInInst;
    VSC_MC_INST*       pMcInst = (VSC_MC_INST*)pRefInInst;

    if (codecMode == VSC_MC_CODEC_MODE_ENCODE)
    {
        if (pInCodecHelperInst->src[pInCodecHelperInst->srcCount - 1].regType ==
            0x7)
        {
            if (pMcCodec->bDual16ModeEnabled)
            {
                return VSC_MC_CODEC_TYPE_INDIRECT_CALL;
            }
            else
            {
                return VSC_MC_CODEC_TYPE_DIRECT_CALL;
            }
        }
        else
        {
            return VSC_MC_CODEC_TYPE_INDIRECT_CALL;
        }
    }
    else
    {
        if (pMcInst->tri_srcs_alu_inst.inst.bSrc2Valid)
        {
            return VSC_MC_CODEC_TYPE_INDIRECT_CALL;
        }
        else
        {
            return VSC_MC_CODEC_TYPE_DIRECT_CALL;
        }
    }
}

static VSC_MC_CODEC_TYPE _GetExtendedOpcodeCodeType(VSC_MC_CODEC* pMcCodec,
                                                    gctUINT baseOpcode,
                                                    gctUINT extOpcode,
                                                    void* pRefInInst,
                                                    VSC_MC_CODEC_MODE codecMode)
{
    if (baseOpcode == 0x7F)
    {
        switch (extOpcode)
        {
        case 0x01:
            return VSC_MC_CODEC_TYPE_EMIT;

        case 0x02:
            return VSC_MC_CODEC_TYPE_2_SRCS_SRC0_SRC1_ALU;

        case 0x03:
        case 0x0B:
        case 0x0C:
            return VSC_MC_CODEC_TYPE_1_SRC_SRC0_ALU;

        case 0x04:
        case 0x0D:
            return VSC_MC_CODEC_TYPE_SAMPLE_EXT;
        }
    }
    else if (baseOpcode == 0x45)
    {
        switch (extOpcode)
        {
        case 0x01:
        case 0x06:
            return VSC_MC_CODEC_TYPE_2_SRCS_SRC0_SRC1_ALU;

        case 0x08:
        case 0x09:
        case 0x0A:
        case 0x0B:
            return VSC_MC_CODEC_TYPE_2_SRCS_SRC0_SRC2_ALU;

        case 0x02:
        case 0x03:
        case 0x04:
        case 0x05:
        case 0x07:
        case 0x0C:
        case 0x0D:
        case 0x0E:
        case 0x0F:
        case 0x10:
        case 0x11:
        case 0x12:
        case 0x13:
        case 0x14:
        case 0x15:
        case 0x16:
            return VSC_MC_CODEC_TYPE_3_SRCS_ALU;
        }
    }

    return VSC_MC_CODEC_TYPE_UNKNOWN;
}

static VSC_MC_CODEC_TYPE _GetMcCodecType(VSC_MC_CODEC* pMcCodec,
                                         gctUINT baseOpcode,
                                         gctUINT extOpcode,
                                         void* pRefInInst,
                                         VSC_MC_CODEC_MODE codecMode)
{
    switch (baseOpcode)
    {
    case 0x00:
    case 0x2A:
    case 0x15:
        return VSC_MC_CODEC_TYPE_NO_OPND;

    case 0x09:
    case 0x56:
    case 0x0A:
    case 0x0B:
    case 0x0F:
    case 0x31:
    case 0x02:
    case 0x0E:
    case 0x30:
    case 0x2B:
    case 0x4C:
    case 0x4D:
    case 0x50:
    case 0x51:
    case 0x4E:
    case 0x4F:
    case 0x52:
    case 0x53:
    case 0x65:
    case 0x66:
    case 0x67:
    case 0x68:
    case 0x69:
    case 0x6A:
    case 0x6B:
    case 0x6C:
    case 0x36:
    case 0x60:
    case 0x54:
    case 0x55:
    case 0x78:
    case 0x63:
        return VSC_MC_CODEC_TYPE_3_SRCS_ALU;

    case 0x10:
    case 0x17:  /* A special one that has no dst valid bit */
    case 0x72:
    case 0x2C:
    case 0x77:
    case 0x03:
    case 0x29:
    case 0x73:
    case 0x05:
    case 0x06:
    case 0x04:
    case 0x3C:
    case 0x3D:
    case 0x3E:
    case 0x3F:
    case 0x40:
    case 0x41:
    case 0x44:
    case 0x48:
        return VSC_MC_CODEC_TYPE_2_SRCS_SRC0_SRC1_ALU;

    case 0x07:
    case 0x08:
    case 0x5C:
    case 0x5D:
    case 0x5E:
    case 0x59:
    case 0x5A:
    case 0x5B:
    case 0x3B:
    case 0x01:
    case 0x28:
        return VSC_MC_CODEC_TYPE_2_SRCS_SRC0_SRC2_ALU;

    case 0x64:
        return VSC_MC_CODEC_TYPE_2_SRCS_SRC1_SRC2_ALU;

    case 0x2D:
    case 0x2E:
    case 0x2F:
    case 0x6D:
    case 0x6E:
    case 0x74:
    case 0x75:
    case 0x76:
        return VSC_MC_CODEC_TYPE_1_SRC_SRC0_ALU;

    case 0x0C:
    case 0x0D:
    case 0x21:
    case 0x11:
    case 0x12:
    case 0x13:
    case 0x22:
    case 0x23:
    case 0x25:
    case 0x26:
    case 0x27:
    case 0x57:
    case 0x58:
    case 0x5F:
    case 0x61:
        return VSC_MC_CODEC_TYPE_1_SRC_SRC2_ALU;

    case 0x71:
        return VSC_MC_CODEC_TYPE_PACK;

    case 0x19:
    case 0x1C:
    case 0x1B:
    case 0x1A:
    case 0x70:
    case 0x7C:
    case MC_AUXILIARY_OP_CODE_TEXLD_LOD:        /* From 0x6F */
    case MC_AUXILIARY_OP_CODE_TEXLD_LOD_PCF:    /* From 0x6F */
    case MC_AUXILIARY_OP_CODE_TEXLD_BIAS:       /* From 0x18 */
    case MC_AUXILIARY_OP_CODE_TEXLD_BIAS_PCF:   /* From 0x18 */
    case MC_AUXILIARY_OP_CODE_TEXLD_PLAIN:      /* From 0x18 */
    case MC_AUXILIARY_OP_CODE_TEXLD_PCF:        /* From 0x18 */
    case 0x7B:
    case MC_AUXILIARY_OP_CODE_TEXLD_U_PLAIN:    /* From 0x7B */
    case MC_AUXILIARY_OP_CODE_TEXLD_U_LOD:      /* From 0x7B */
    case MC_AUXILIARY_OP_CODE_TEXLD_U_BIAS:     /* From 0x7B */
    case MC_AUXILIARY_OP_CODE_TEXLD_U_F_B_PLAIN:/* From 0x7B */
    case MC_AUXILIARY_OP_CODE_TEXLD_U_F_B_BIAS:  /* From 0x7B */
    case MC_AUXILIARY_OP_CODE_TEXLD_GATHER:     /* From 0x7D */
    case MC_AUXILIARY_OP_CODE_TEXLD_GATHER_PCF: /* From 0x7D */

    case 0x4B:
    case 0x49:
    case 0x4A:
        return VSC_MC_CODEC_TYPE_SAMPLE;

    case 0x32:
        return VSC_MC_CODEC_TYPE_LOAD;

    case 0x79:
    case 0x34:
    case 0x37:
    case 0x38:
        return VSC_MC_CODEC_TYPE_IMG_LOAD;

    case 0x33:
    case MC_AUXILIARY_OP_CODE_USC_STORE:
        return VSC_MC_CODEC_TYPE_STORE;

    case 0x7A:
    case 0x35:
    case MC_AUXILIARY_OP_CODE_USC_IMG_STORE:
    case MC_AUXILIARY_OP_CODE_USC_IMG_STORE_3D:
        return VSC_MC_CODEC_TYPE_IMG_STORE;

    case 0x46:
        return VSC_MC_CODEC_TYPE_IMG_ATOM;

    case 0x42:
    case MC_AUXILIARY_OP_CODE_USC_STORE_ATTR:
        return VSC_MC_CODEC_TYPE_STORE_ATTR;

    case 0x43:
        return VSC_MC_CODEC_TYPE_SELECT_MAP;

    case 0x16:
    case 0x24:
        return _GetBranchCodecType(pMcCodec, pRefInInst, codecMode);

    case 0x14:
        return _GetCallCodecType(pMcCodec, pRefInInst, codecMode);

    case 0x1F:
    case 0x1D:
    case 0x20:
    case 0x1E:
        return VSC_MC_CODEC_TYPE_LOOP;

    case 0x7F:
    case 0x45:
        return _GetExtendedOpcodeCodeType(pMcCodec, baseOpcode, extOpcode, pRefInInst, codecMode);
    }

    return VSC_MC_CODEC_TYPE_UNKNOWN;
}

static gctUINT _MapSampleAuxOpcodeToHwOpcode(gctUINT auxOpcode)
{
    if (auxOpcode >= MC_AUXILIARY_OP_CODE_TEXLD_LOD &&
        auxOpcode <= MC_AUXILIARY_OP_CODE_TEXLD_LOD_PCF)
    {
        return 0x6F;
    }

    if (auxOpcode >= MC_AUXILIARY_OP_CODE_TEXLD_BIAS &&
        auxOpcode <= MC_AUXILIARY_OP_CODE_TEXLD_PCF)
    {
        return 0x18;
    }

    if (auxOpcode >= MC_AUXILIARY_OP_CODE_TEXLD_U_PLAIN &&
        auxOpcode <= MC_AUXILIARY_OP_CODE_TEXLD_U_BIAS)
    {
        return 0x7B;
    }

    if (auxOpcode >= MC_AUXILIARY_OP_CODE_TEXLD_U_F_B_PLAIN &&
        auxOpcode <= MC_AUXILIARY_OP_CODE_TEXLD_U_F_B_BIAS)
    {
        return 0x7B;
    }

    if (auxOpcode >= MC_AUXILIARY_OP_CODE_TEXLD_GATHER &&
        auxOpcode <= MC_AUXILIARY_OP_CODE_TEXLD_GATHER_PCF)
    {
        return 0x7D;
    }

    return auxOpcode;
}

static gctUINT _MapLdStAuxOpcodeToHwOpcode(gctUINT auxOpcode)
{
    if (auxOpcode == MC_AUXILIARY_OP_CODE_USC_STORE)
    {
        return 0x33;
    }

    if (auxOpcode == MC_AUXILIARY_OP_CODE_USC_IMG_STORE)
    {
        return 0x7A;
    }

    if (auxOpcode == MC_AUXILIARY_OP_CODE_USC_IMG_STORE_3D)
    {
        return 0x35;
    }

    if (auxOpcode == MC_AUXILIARY_OP_CODE_USC_STORE_ATTR)
    {
        return 0x42;
    }

    return auxOpcode;
}


/* src of cond-op is always from src0 */
static gctUINT _condOp2SrcCount[] =
{
    0, /* 0x00      */
    2, /* 0x01        */
    2, /* 0x02        */
    2, /* 0x03        */
    2, /* 0x04        */
    2, /* 0x05        */
    2, /* 0x06        */
    2, /* 0x07       */
    2, /* 0x08        */
    2, /* 0x09       */
    1, /* 0x0A       */
    1, /* 0x0B        */
    1, /* 0x0C       */
    1, /* 0x0D        */
    1, /* 0x0E       */
    1, /* 0x0F        */
    1, /* 0x10    */
    1, /* 0x11  */
    1, /* 0x12       */
    1, /* 0x13    */
    1, /* 0x14    */
    1, /* 0x15    */
    1, /* 0x16    */
    2, /* 0x17    */
    0, /* 0x18    */
    0, /* 0x19 */
};

static gctBOOL _IsSupportCondOp(gctUINT baseOpcode, gctUINT extOpcode)
{
    if (baseOpcode < 0x7F)
    {
        return (((baseOpcode) == 0x09)      ||
                ((baseOpcode) == 0x56)    ||
                ((baseOpcode) == 0x0A)    ||
                ((baseOpcode) == 0x0B)    ||
                ((baseOpcode) == 0x0F)   ||
                ((baseOpcode) == 0x31)      ||
                ((baseOpcode) == 0x10)      ||
                ((baseOpcode) == 0x17)  ||
                ((baseOpcode) == 0x16));
    }
    else
    {
        /* Nothing this time */
    }

    return gcvFALSE;
}


static void _EncodeExtendedOpcode(gctUINT baseOpcode,
                                  gctUINT extOpcode,
                                  VSC_MC_INST* pMcInst)
{
    if (baseOpcode == 0x7F)
    {
        pMcInst->no_opnd_inst.inst.extOpcode = extOpcode;

        /* Extra info */
        pMcInst->tri_srcs_alu_inst.inst.bSrc2Valid = gcvTRUE;
        pMcInst->tri_srcs_alu_inst.inst.src2Type = 0x7;
        pMcInst->tri_srcs_alu_inst.inst.src2RelAddr = (0x2 << 1);
    }
    else if (baseOpcode == 0x45)
    {
        pMcInst->evis_inst.inst.extEvisOpCodeBit0_2 = (extOpcode & 0x7);
        pMcInst->evis_inst.inst.extEvisOpCodeBit3 = ((extOpcode >> 3) & 0x1);
        pMcInst->evis_inst.inst.extEvisOpCodeBit4_5 = ((extOpcode >> 4) & 0x3);
    }
    else
    {
        gcmASSERT(gcvFALSE);
    }
}


#define ENCODE_BASE_OPCODE(pMcInst, baseOpcode)                                           \
        (pMcInst)->no_opnd_inst.inst.baseOpcodeBit0_5 = ((baseOpcode) & 0x3F);            \
        (pMcInst)->no_opnd_inst.inst.baseOpcodeBit6   = (((baseOpcode) >> 6) & 0x01);

#define DECODE_BASE_OPCODE(pMcInst)                                                       \
        ((pMcInst)->no_opnd_inst.inst.baseOpcodeBit0_5 |                                  \
         ((pMcInst)->no_opnd_inst.inst.baseOpcodeBit6 << 6))

#define ENCODE_EXT_OPCODE(pMcInst, baseOpcode, extOpcode)                                 \
    if ((baseOpcode) == 0x7F || (baseOpcode) == 0x45) \
    {                                                                                     \
        _EncodeExtendedOpcode((baseOpcode), (extOpcode), (VSC_MC_INST*)(pMcInst));        \
    }

#define DECODE_EXT_OPCODE(pMcInst, baseOpcode, extOpcode)                                 \
    if ((baseOpcode) == 0x7F)                                         \
    {                                                                                     \
        (extOpcode) = _DecodeExtendedOpcode((VSC_MC_INST*)(pMcInst));                     \
    }

static gctUINT _Conver32BitImm_2_20BitImm(VSC_MC_CODEC_IMM_VALUE immValue,
                                          gctUINT immType)
{
    gctUINT imm20Bit = 0;

    switch (immType)
    {
    case 0x0:
        imm20Bit = vscCvtS23E8FloatToS11E8Float(immValue.ui);
        break;

    case 0x1:
            imm20Bit = (immValue.si & 0xFFFFF);
        break;

    case 0x2:
        imm20Bit = (immValue.ui & 0xFFFFF);
        break;

    case 0x3:
        imm20Bit = (immValue.ui & 0xFFFF); /* Only lsb 16bit is enough */
        break;
    }

    return imm20Bit;
}

static void _EncodeImmData(VSC_MC_INST* pMcInst,
                           gctUINT srcIdx,
                           VSC_MC_CODEC_IMM_VALUE immValue,
                           gctUINT immType)
{
    gctUINT raw20BitImmValue;

    gcmASSERT(srcIdx < 3);

    raw20BitImmValue = _Conver32BitImm_2_20BitImm(immValue, immType);

    if (srcIdx == 0)
    {
        pMcInst->tri_srcs_alu_inst.inst.src0RegNo =   (raw20BitImmValue & 0x1FF);
        pMcInst->tri_srcs_alu_inst.inst.src0Swizzle = ((raw20BitImmValue >> 9) & 0xFF);
        pMcInst->tri_srcs_alu_inst.inst.bSrc0ModNeg = ((raw20BitImmValue >> 17) & 0x01);
        pMcInst->tri_srcs_alu_inst.inst.bSrc0ModAbs = ((raw20BitImmValue >> 18) & 0x01);
        pMcInst->tri_srcs_alu_inst.inst.src0RelAddr = ((immType << 1) | ((raw20BitImmValue >> 19) & 0x01));
    }
    else if (srcIdx == 1)
    {
        pMcInst->tri_srcs_alu_inst.inst.src1RegNo =   (raw20BitImmValue & 0x1FF);
        pMcInst->tri_srcs_alu_inst.inst.src1Swizzle = ((raw20BitImmValue >> 9) & 0xFF);
        pMcInst->tri_srcs_alu_inst.inst.bSrc1ModNeg = ((raw20BitImmValue >> 17) & 0x01);
        pMcInst->tri_srcs_alu_inst.inst.bSrc1ModAbs = ((raw20BitImmValue >> 18) & 0x01);
        pMcInst->tri_srcs_alu_inst.inst.src1RelAddr = ((immType << 1) | ((raw20BitImmValue >> 19) & 0x01));
    }
    else
    {
        pMcInst->tri_srcs_alu_inst.inst.src2RegNo =   (raw20BitImmValue & 0x1FF);
        pMcInst->tri_srcs_alu_inst.inst.src2Swizzle = ((raw20BitImmValue >> 9) & 0xFF);
        pMcInst->tri_srcs_alu_inst.inst.bSrc2ModNeg = ((raw20BitImmValue >> 17) & 0x01);
        pMcInst->tri_srcs_alu_inst.inst.bSrc2ModAbs = ((raw20BitImmValue >> 18) & 0x01);
        pMcInst->tri_srcs_alu_inst.inst.src2RelAddr = ((immType << 1) | ((raw20BitImmValue >> 19) & 0x01));
    }
}


static void _EncodeDst(VSC_MC_CODEC* pMcCodec,
                       VSC_MC_CODEC_DST* pMcCodecDst,
                       gctBOOL bEvisMode,
                       VSC_MC_INST* pMcInst)
{
    pMcInst->tri_srcs_alu_inst.inst.bDstValid = gcvTRUE;
    pMcInst->tri_srcs_alu_inst.inst.dstType = pMcCodecDst->regType;
    pMcInst->tri_srcs_alu_inst.inst.bDstModSat = pMcCodecDst->bSaturated;

    if (bEvisMode)
    {
        /*  Why dst is valid, but has no channel to be written? */
        gcmASSERT(pMcCodecDst->u.evisDst.compIdxRange > 0);

        pMcInst->evis_inst.inst.startDstCompIdx = pMcCodecDst->u.evisDst.startCompIdx;
        pMcInst->evis_inst.inst.endDstCompIdx = pMcCodecDst->u.evisDst.startCompIdx + pMcCodecDst->u.evisDst.compIdxRange - 1;
    }
    else
    {
        /* Why dst is valid, but has no writemask? */
        gcmASSERT(pMcCodecDst->u.nmlDst.writeMask != WRITEMASK_NONE);

        pMcInst->tri_srcs_alu_inst.inst.dstRelAddr = pMcCodecDst->u.nmlDst.indexingAddr;
        pMcInst->tri_srcs_alu_inst.inst.writeMask = pMcCodecDst->u.nmlDst.writeMask;
    }

    if (pMcCodec->bDual16ModeEnabled)
    {
        gcmASSERT(pMcCodecDst->regNo < 128);
        pMcInst->tri_srcs_alu_inst.inst.dstRegNoBit0_6 = (pMcCodecDst->regNo & 0x7F);
    }
    else
    {
        gcmASSERT(pMcCodecDst->regNo < 512);
        pMcInst->tri_srcs_alu_inst.inst.dstRegNoBit0_6 = (pMcCodecDst->regNo & 0x7F);
        pMcInst->tri_srcs_alu_inst.inst.dstRegNoBit7 = ((pMcCodecDst->regNo >> 7) & 0x01);
        pMcInst->tri_srcs_alu_inst.inst.dstRegNoBit8 = ((pMcCodecDst->regNo >> 8) & 0x01);
    }
}


static void _EncodeSrc(VSC_MC_CODEC* pMcCodec,
                       gctUINT srcIdx,
                       VSC_MC_CODEC_SRC* pMcCodecSrc,
                       gctBOOL bEvisMode,
                       VSC_MC_INST* pMcInst)
{
    gcmASSERT(srcIdx < 3);

    if (pMcCodecSrc->regType == 0x7)
    {
        if (srcIdx == 0)
        {
            pMcInst->tri_srcs_alu_inst.inst.bSrc0Valid = gcvTRUE;
            pMcInst->tri_srcs_alu_inst.inst.src0Type = pMcCodecSrc->regType;
        }
        else if (srcIdx == 1)
        {
            pMcInst->tri_srcs_alu_inst.inst.bSrc1Valid = gcvTRUE;
            pMcInst->tri_srcs_alu_inst.inst.src1Type = pMcCodecSrc->regType;
        }
        else
        {
            pMcInst->tri_srcs_alu_inst.inst.bSrc2Valid = gcvTRUE;
            pMcInst->tri_srcs_alu_inst.inst.src2Type = pMcCodecSrc->regType;
        }

        _EncodeImmData(pMcInst, srcIdx, pMcCodecSrc->u.imm.immData, pMcCodecSrc->u.imm.immType);
    }
    else
    {
        gcmASSERT(pMcCodecSrc->u.reg.regNo < 512);
        if (srcIdx == 0)
        {
            pMcInst->tri_srcs_alu_inst.inst.bSrc0Valid = gcvTRUE;
            pMcInst->tri_srcs_alu_inst.inst.src0RegNo = pMcCodecSrc->u.reg.regNo;
            pMcInst->tri_srcs_alu_inst.inst.src0RelAddr = pMcCodecSrc->u.reg.indexingAddr;
            pMcInst->tri_srcs_alu_inst.inst.src0Type = pMcCodecSrc->regType;

            if (!bEvisMode)
            {
                pMcInst->tri_srcs_alu_inst.inst.src0Swizzle = pMcCodecSrc->u.reg.swizzle;
                pMcInst->tri_srcs_alu_inst.inst.bSrc0ModAbs = pMcCodecSrc->u.reg.bAbs;
                pMcInst->tri_srcs_alu_inst.inst.bSrc0ModNeg = pMcCodecSrc->u.reg.bNegative;
            }
        }
        else if (srcIdx == 1)
        {
            pMcInst->tri_srcs_alu_inst.inst.bSrc1Valid = gcvTRUE;
            pMcInst->tri_srcs_alu_inst.inst.src1RegNo = pMcCodecSrc->u.reg.regNo;
            pMcInst->tri_srcs_alu_inst.inst.src1Swizzle = pMcCodecSrc->u.reg.swizzle;
            pMcInst->tri_srcs_alu_inst.inst.src1RelAddr = pMcCodecSrc->u.reg.indexingAddr;
            pMcInst->tri_srcs_alu_inst.inst.src1Type = pMcCodecSrc->regType;

            if (!bEvisMode)
            {
                pMcInst->tri_srcs_alu_inst.inst.bSrc1ModAbs = pMcCodecSrc->u.reg.bAbs;
                pMcInst->tri_srcs_alu_inst.inst.bSrc1ModNeg = pMcCodecSrc->u.reg.bNegative;
            }
        }
        else
        {
            pMcInst->tri_srcs_alu_inst.inst.bSrc2Valid = gcvTRUE;
            pMcInst->tri_srcs_alu_inst.inst.src2RegNo = pMcCodecSrc->u.reg.regNo;
            pMcInst->tri_srcs_alu_inst.inst.src2Swizzle = pMcCodecSrc->u.reg.swizzle;
            pMcInst->tri_srcs_alu_inst.inst.src2RelAddr = pMcCodecSrc->u.reg.indexingAddr;
            pMcInst->tri_srcs_alu_inst.inst.src2Type = pMcCodecSrc->regType;

            if (!bEvisMode)
            {
                pMcInst->tri_srcs_alu_inst.inst.bSrc2ModAbs = pMcCodecSrc->u.reg.bAbs;
                pMcInst->tri_srcs_alu_inst.inst.bSrc2ModNeg = pMcCodecSrc->u.reg.bNegative;
            }
        }
    }
}


static void _EncodeInstType(VSC_MC_CODEC* pMcCodec,
                            VSC_MC_CODEC_TYPE mcCodecType,
                            VSC_MC_INST* pMcInst,
                            gctUINT instType)
{
    pMcInst->tri_srcs_alu_inst.inst.instTypeBit0 = (instType & 0x01);
    pMcInst->tri_srcs_alu_inst.inst.instTypeBit1_2 = ((instType >> 1) & 0x03);

    if (mcCodecType == VSC_MC_CODEC_TYPE_LOAD ||
        mcCodecType == VSC_MC_CODEC_TYPE_STORE)
    {
        /* There is an extra 4th bit for inst type */
        pMcInst->load_inst.inst.instTypeBit3 = ((instType >> 3) & 0x01);
    }
}


static void _EncodeThreadType(VSC_MC_CODEC* pMcCodec,
                              VSC_MC_CODEC_TYPE mcCodecType,
                              VSC_MC_INST* pMcInst,
                              gctUINT threadType)
{
    if (pMcCodec->bDual16ModeEnabled)
    {
        if (mcCodecType == VSC_MC_CODEC_TYPE_STORE ||
            mcCodecType == VSC_MC_CODEC_TYPE_IMG_STORE ||
            mcCodecType == VSC_MC_CODEC_TYPE_STORE_ATTR ||
            mcCodecType == VSC_MC_CODEC_TYPE_DIRECT_BRANCH_1 ||
            mcCodecType == VSC_MC_CODEC_TYPE_INDIRECT_BRANCH ||
            mcCodecType == VSC_MC_CODEC_TYPE_INDIRECT_CALL)
        {
            pMcInst->store_inst.inst.threadTypeBit0 = (threadType & 0x01);
            pMcInst->store_inst.inst.threadTypeBit1 = ((threadType >> 1) & 0x01);
        }
        else
        {
            pMcInst->tri_srcs_alu_inst.inst.dstRegNoBit7 = (threadType & 0x01);
            pMcInst->tri_srcs_alu_inst.inst.dstRegNoBit8 = ((threadType >> 1) & 0x01);
        }
    }
}


static gctBOOL _Common_Encode_Mc_Alu_Inst(VSC_MC_CODEC* pMcCodec,
                                          VSC_MC_CODEC_TYPE mcCodecType,
                                          VSC_MC_CODEC_INST* pInCodecHelperInst,
                                          gctUINT* pSrcMap,
                                          VSC_MC_INST* pOutMcInst)
{
    gctUINT           srcIdx;
    gctBOOL           bEvisMode = (pInCodecHelperInst->baseOpcode == 0x45);

    /* Opcode */
    ENCODE_BASE_OPCODE(pOutMcInst, pInCodecHelperInst->baseOpcode);

    /* Dst */
    if (pInCodecHelperInst->bDstValid)
    {
        _EncodeDst(pMcCodec, &pInCodecHelperInst->dst, bEvisMode, pOutMcInst);
    }

    /* Src */
    for (srcIdx = 0; srcIdx < pInCodecHelperInst->srcCount; srcIdx ++)
    {
        _EncodeSrc(pMcCodec, pSrcMap[srcIdx], &pInCodecHelperInst->src[srcIdx], bEvisMode, pOutMcInst);
    }

    /* Inst ctrl */
    _EncodeInstType(pMcCodec, mcCodecType, pOutMcInst, pInCodecHelperInst->instCtrl.instType);
    _EncodeThreadType(pMcCodec, mcCodecType, pOutMcInst, pInCodecHelperInst->instCtrl.threadType);
    if (bEvisMode)
    {
        /* EVIS insts have evis-state and sourceBin */
        ((VSC_MC_INST*)pOutMcInst)->evis_inst.inst.evisState = pInCodecHelperInst->instCtrl.u.visionCtrl.evisState;
        ((VSC_MC_INST*)pOutMcInst)->evis_inst.inst.startSrcCompIdx = pInCodecHelperInst->instCtrl.u.visionCtrl.startSrcCompIdx;

    }
    else
    {
        pOutMcInst->tri_srcs_alu_inst.inst.roundMode = pInCodecHelperInst->instCtrl.roundingMode;
        pOutMcInst->tri_srcs_alu_inst.inst.bPackMode = pInCodecHelperInst->instCtrl.bPacked;
    }

    return gcvTRUE;
}

static gctBOOL _Common_Encode_Mc_Sample_Inst(VSC_MC_CODEC* pMcCodec,
                                             VSC_MC_CODEC_TYPE mcCodecType,
                                             VSC_MC_CODEC_INST* pInCodecHelperInst,
                                             VSC_MC_INST* pOutMcInst)
{
    /* Opcode */
    ENCODE_BASE_OPCODE(pOutMcInst, _MapSampleAuxOpcodeToHwOpcode(pInCodecHelperInst->baseOpcode));

    /* Dst */
    _EncodeDst(pMcCodec, &pInCodecHelperInst->dst, gcvFALSE, pOutMcInst);

    /* Src */
    gcmASSERT(pInCodecHelperInst->src[0].regType == MC_AUXILIARY_SRC_TYPE_SAMPLER);
    pOutMcInst->sample_inst.inst.samplerSlot = pInCodecHelperInst->src[0].u.reg.regNo;
    pOutMcInst->sample_inst.inst.samplerSwizzle = pInCodecHelperInst->src[0].u.reg.swizzle;
    pOutMcInst->sample_inst.inst.samplerRelAddr = pInCodecHelperInst->src[0].u.reg.indexingAddr;

    _EncodeSrc(pMcCodec, 0, &pInCodecHelperInst->src[1], gcvFALSE, pOutMcInst);

    /* Different opcode encodes different src */
    if (pInCodecHelperInst->baseOpcode == MC_AUXILIARY_OP_CODE_TEXLD_LOD ||
        pInCodecHelperInst->baseOpcode == MC_AUXILIARY_OP_CODE_TEXLD_BIAS ||
        pInCodecHelperInst->baseOpcode == MC_AUXILIARY_OP_CODE_TEXLD_GATHER ||
        (pInCodecHelperInst->baseOpcode == 0x7F &&
         (pInCodecHelperInst->extOpcode == 0x04 ||
          pInCodecHelperInst->extOpcode == 0x0D)))
    {
        gcmASSERT(pInCodecHelperInst->srcCount == 3);

        _EncodeSrc(pMcCodec, 1, &pInCodecHelperInst->src[2], gcvFALSE, pOutMcInst);
    }
    else if (pInCodecHelperInst->baseOpcode == MC_AUXILIARY_OP_CODE_TEXLD_PCF ||
             pInCodecHelperInst->baseOpcode == MC_AUXILIARY_OP_CODE_TEXLD_U_PLAIN ||
             pInCodecHelperInst->baseOpcode == MC_AUXILIARY_OP_CODE_TEXLD_U_F_B_PLAIN)
    {
        gcmASSERT(pInCodecHelperInst->srcCount == 3);

        _EncodeSrc(pMcCodec, 2, &pInCodecHelperInst->src[2], gcvFALSE, pOutMcInst);
    }
    else if (pInCodecHelperInst->baseOpcode == 0x1A ||
             pInCodecHelperInst->baseOpcode == 0x70 ||
             pInCodecHelperInst->baseOpcode == 0x7C ||
             pInCodecHelperInst->baseOpcode == MC_AUXILIARY_OP_CODE_TEXLD_LOD_PCF ||
             pInCodecHelperInst->baseOpcode == MC_AUXILIARY_OP_CODE_TEXLD_BIAS_PCF ||
             pInCodecHelperInst->baseOpcode == MC_AUXILIARY_OP_CODE_TEXLD_U_LOD ||
             pInCodecHelperInst->baseOpcode == MC_AUXILIARY_OP_CODE_TEXLD_U_BIAS ||
             pInCodecHelperInst->baseOpcode == MC_AUXILIARY_OP_CODE_TEXLD_GATHER_PCF ||
             pInCodecHelperInst->baseOpcode == MC_AUXILIARY_OP_CODE_TEXLD_U_F_B_BIAS ||
             pInCodecHelperInst->baseOpcode == 0x4B ||
             pInCodecHelperInst->baseOpcode == 0x49 ||
             pInCodecHelperInst->baseOpcode == 0x4A)
    {
        gcmASSERT(pInCodecHelperInst->srcCount == 4);

        _EncodeSrc(pMcCodec, 1, &pInCodecHelperInst->src[2], gcvFALSE, pOutMcInst);
        _EncodeSrc(pMcCodec, 2, &pInCodecHelperInst->src[3], gcvFALSE, pOutMcInst);
    }

    /* Inst ctrl */
    _EncodeInstType(pMcCodec, mcCodecType, pOutMcInst, pInCodecHelperInst->instCtrl.instType);
    _EncodeThreadType(pMcCodec, mcCodecType, pOutMcInst, pInCodecHelperInst->instCtrl.threadType);

    return gcvTRUE;
}

static gctBOOL _Common_Encode_Mc_Load_Store_Inst(VSC_MC_CODEC* pMcCodec,
                                                 VSC_MC_CODEC_TYPE mcCodecType,
                                                 VSC_MC_CODEC_INST* pInCodecHelperInst,
                                                 gctBOOL bForImgLS,
                                                 VSC_MC_INST* pOutMcInst)
{
    gctUINT           srcIdx;
    gctBOOL           bEvisMode = pInCodecHelperInst->instCtrl.u.maCtrl.bUnderEvisMode;

    /* Opcode */
    ENCODE_BASE_OPCODE(pOutMcInst, _MapLdStAuxOpcodeToHwOpcode(pInCodecHelperInst->baseOpcode));

    /* Dst */
    if (pInCodecHelperInst->bDstValid)
    {
        _EncodeDst(pMcCodec, &pInCodecHelperInst->dst, bEvisMode, pOutMcInst);
    }

    /* Src */
    for (srcIdx = 0; srcIdx < pInCodecHelperInst->srcCount; srcIdx ++)
    {
        _EncodeSrc(pMcCodec, srcIdx, &pInCodecHelperInst->src[srcIdx], gcvFALSE, pOutMcInst);
    }

    /* Inst ctrl */
    pOutMcInst->load_inst.inst.bSkipForHelperKickoff = pInCodecHelperInst->instCtrl.bSkipForHelperKickoff;
    pOutMcInst->load_inst.inst.bAccessLocalStorage = pInCodecHelperInst->instCtrl.u.maCtrl.bAccessLocalStorage;
    if (!bForImgLS)
    {
        pOutMcInst->load_inst.inst.bPackMode = pInCodecHelperInst->instCtrl.bPacked;
        pOutMcInst->load_inst.inst.bDenorm = pInCodecHelperInst->instCtrl.u.maCtrl.u.lsCtrl.bDenorm;
        pOutMcInst->load_inst.inst.bOffsetX3 = pInCodecHelperInst->instCtrl.u.maCtrl.u.lsCtrl.bOffsetX3;
        pOutMcInst->load_inst.inst.offsetLeftShift = pInCodecHelperInst->instCtrl.u.maCtrl.u.lsCtrl.offsetLeftShift;
    }
    if (pInCodecHelperInst->baseOpcode == 0x46)
    {
        ((VSC_MC_INST*)pOutMcInst)->img_atom_inst.inst.atomicMode = pInCodecHelperInst->instCtrl.u.maCtrl.u.imgAtomCtrl.atomicMode;
        ((VSC_MC_INST*)pOutMcInst)->img_atom_inst.inst.b3dImgMode = pInCodecHelperInst->instCtrl.u.maCtrl.u.imgAtomCtrl.b3dImgMode;
    }
    if (bEvisMode)
    {
        /* Always mark LSB1 of evis-state to be enabled */
        ((VSC_MC_INST*)pOutMcInst)->evis_inst.inst.evisState = 0x01;
    }

    _EncodeInstType(pMcCodec, mcCodecType, pOutMcInst, pInCodecHelperInst->instCtrl.instType);
    _EncodeThreadType(pMcCodec, mcCodecType, pOutMcInst, pInCodecHelperInst->instCtrl.threadType);

    return gcvTRUE;
}

/* Encode routines */

static gctBOOL _Encode_Mc_No_Opnd_Inst(VSC_MC_CODEC* pMcCodec,
                                       VSC_MC_CODEC_TYPE mcCodecType,
                                       VSC_MC_CODEC_INST* pInCodecHelperInst,
                                       VSC_MC_NO_OPERAND_INST* pOutMcInst)
{
    gcmASSERT(mcCodecType == VSC_MC_CODEC_TYPE_NO_OPND);

    /* Opcode */
    ENCODE_BASE_OPCODE((VSC_MC_INST*)pOutMcInst, pInCodecHelperInst->baseOpcode);
    ENCODE_EXT_OPCODE(pOutMcInst, pInCodecHelperInst->baseOpcode, pInCodecHelperInst->extOpcode);

    return gcvTRUE;
}

static gctBOOL _Encode_Mc_3_Srcs_Alu_Inst(VSC_MC_CODEC* pMcCodec,
                                          VSC_MC_CODEC_TYPE mcCodecType,
                                          VSC_MC_CODEC_INST* pInCodecHelperInst,
                                          VSC_MC_ALU_3_SRCS_INST* pOutMcInst)
{
    gctUINT srcMap[3];

    gcmASSERT(pInCodecHelperInst->bDstValid);
    gcmASSERT(mcCodecType == VSC_MC_CODEC_TYPE_3_SRCS_ALU);

    if (_IsSupportCondOp(pInCodecHelperInst->baseOpcode, pInCodecHelperInst->extOpcode))
    {
        if (pInCodecHelperInst->baseOpcode == 0x0F)
        {
            /* Select has special case */
            if (_condOp2SrcCount[pInCodecHelperInst->instCtrl.condOpCode] == 0)
            {
                gcmASSERT(pInCodecHelperInst->srcCount == 2);

                srcMap[0] = 1;
                srcMap[1] = 2;
            }
            else
            {
                gcmASSERT(pInCodecHelperInst->srcCount == 3);

                srcMap[0] = 0;
                srcMap[1] = 1;
                srcMap[2] = 2;
            }
        }
        else
        {
            gcmASSERT(pInCodecHelperInst->srcCount == _condOp2SrcCount[pInCodecHelperInst->instCtrl.condOpCode] + 1);

            if (_condOp2SrcCount[pInCodecHelperInst->instCtrl.condOpCode] == 0)
            {
                srcMap[0] = 2;
            }
            else if (_condOp2SrcCount[pInCodecHelperInst->instCtrl.condOpCode] == 1)
            {
                srcMap[0] = 0;
                srcMap[1] = 2;
            }
            else
            {
                srcMap[0] = 0;
                srcMap[1] = 1;
                srcMap[2] = 2;
            }
        }
    }
    else
    {
        gcmASSERT(pInCodecHelperInst->srcCount == 3);

        srcMap[0] = 0;
        srcMap[1] = 1;
        srcMap[2] = 2;
    }

    ENCODE_EXT_OPCODE(pOutMcInst, pInCodecHelperInst->baseOpcode, pInCodecHelperInst->extOpcode);
    pOutMcInst->inst.condOpCode = pInCodecHelperInst->instCtrl.condOpCode;

    /* Atomic operations may skip for helper pixel, also atomics might access local memory, not always global memory */
    if (pInCodecHelperInst->baseOpcode == 0x65      ||
        pInCodecHelperInst->baseOpcode == 0x66     ||
        pInCodecHelperInst->baseOpcode == 0x67 ||
        pInCodecHelperInst->baseOpcode == 0x68      ||
        pInCodecHelperInst->baseOpcode == 0x69      ||
        pInCodecHelperInst->baseOpcode == 0x6A       ||
        pInCodecHelperInst->baseOpcode == 0x6B      ||
        pInCodecHelperInst->baseOpcode == 0x6C)
    {
        pOutMcInst->inst.bSkipForHelperKickoff = pInCodecHelperInst->instCtrl.bSkipForHelperKickoff;
        ((VSC_MC_INST*)pOutMcInst)->load_inst.inst.bAccessLocalStorage = pInCodecHelperInst->instCtrl.u.maCtrl.bAccessLocalStorage;
    }

    /* Load_attr needs a sync bit */
    if (pInCodecHelperInst->baseOpcode == 0x78)
    {
        ((VSC_MC_INST*)pOutMcInst)->store_attr_inst.inst.bNeedUscSync =
                                         pInCodecHelperInst->instCtrl.u.bNeedUscSync ? 1 : 0;
    }

    return _Common_Encode_Mc_Alu_Inst(pMcCodec, mcCodecType, pInCodecHelperInst, &srcMap[0], (VSC_MC_INST*)pOutMcInst);
}

static gctBOOL _Encode_Mc_2_Srcs_Src0_Src1_Alu_Inst(VSC_MC_CODEC* pMcCodec,
                                                    VSC_MC_CODEC_TYPE mcCodecType,
                                                    VSC_MC_CODEC_INST* pInCodecHelperInst,
                                                    VSC_MC_ALU_2_SRCS_SRC0_SRC1_INST* pOutMcInst)
{
    gctUINT srcMap[2] = {0, 1};

    gcmASSERT(mcCodecType == VSC_MC_CODEC_TYPE_2_SRCS_SRC0_SRC1_ALU);
    gcmASSERT(pInCodecHelperInst->bDstValid || pInCodecHelperInst->baseOpcode == 0x17);

    if (_IsSupportCondOp(pInCodecHelperInst->baseOpcode, pInCodecHelperInst->extOpcode))
    {
        gcmASSERT(pInCodecHelperInst->srcCount == _condOp2SrcCount[pInCodecHelperInst->instCtrl.condOpCode]);
    }
    else if (pInCodecHelperInst->baseOpcode == 0x7F &&
             pInCodecHelperInst->extOpcode == 0x02)
    {
        gcmASSERT(pInCodecHelperInst->srcCount >= 1 && pInCodecHelperInst->srcCount <= 2);
    }
    else
    {
        gcmASSERT(pInCodecHelperInst->srcCount == 2);
    }

    ENCODE_EXT_OPCODE(pOutMcInst, pInCodecHelperInst->baseOpcode, pInCodecHelperInst->extOpcode);
    pOutMcInst->inst.condOpCode = pInCodecHelperInst->instCtrl.condOpCode;

    if ((pInCodecHelperInst->baseOpcode == 0x03 && pInCodecHelperInst->instCtrl.u.bInfX0ToZero) ||
        (pInCodecHelperInst->baseOpcode == 0x77))
    {
        /* MUL/NORM_MUL use MSB5 of word0 as control bit to control result of (0 * INF) */
        ((VSC_MC_INST*)pOutMcInst)->sample_inst.inst.samplerSlot = 1;
    }

    if (pInCodecHelperInst->baseOpcode == 0x29)
    {
        gcmASSERT(pInCodecHelperInst->instCtrl.roundingMode == 0x0 ||
                  pInCodecHelperInst->instCtrl.roundingMode == 0x1);

        /* This is actually a enable bit, not rounding mode */
        pInCodecHelperInst->instCtrl.roundingMode = 0x1;
    }

    return _Common_Encode_Mc_Alu_Inst(pMcCodec, mcCodecType, pInCodecHelperInst, &srcMap[0], (VSC_MC_INST*)pOutMcInst);
}

static gctBOOL _Encode_Mc_2_Srcs_Src0_Src2_Alu_Inst(VSC_MC_CODEC* pMcCodec,
                                                    VSC_MC_CODEC_TYPE mcCodecType,
                                                    VSC_MC_CODEC_INST* pInCodecHelperInst,
                                                    VSC_MC_ALU_2_SRCS_SRC0_SRC2_INST* pOutMcInst)
{
    gctUINT srcMap[2] = {0, 2};

    gcmASSERT(mcCodecType == VSC_MC_CODEC_TYPE_2_SRCS_SRC0_SRC2_ALU);
    gcmASSERT(pInCodecHelperInst->bDstValid);
    gcmASSERT(pInCodecHelperInst->srcCount == 2);

    ENCODE_EXT_OPCODE(pOutMcInst, pInCodecHelperInst->baseOpcode, pInCodecHelperInst->extOpcode);

    return _Common_Encode_Mc_Alu_Inst(pMcCodec, mcCodecType, pInCodecHelperInst, &srcMap[0], (VSC_MC_INST*)pOutMcInst);
}

static gctBOOL _Encode_Mc_2_Srcs_Src1_Src2_Alu_Inst(VSC_MC_CODEC* pMcCodec,
                                                    VSC_MC_CODEC_TYPE mcCodecType,
                                                    VSC_MC_CODEC_INST* pInCodecHelperInst,
                                                    VSC_MC_ALU_2_SRCS_SRC1_SRC2_INST* pOutMcInst)
{
    gctUINT srcMap[2] = {1, 2};

    gcmASSERT(mcCodecType == VSC_MC_CODEC_TYPE_2_SRCS_SRC1_SRC2_ALU);
    gcmASSERT(pInCodecHelperInst->bDstValid);
    gcmASSERT(pInCodecHelperInst->srcCount == 2);

    if (pInCodecHelperInst->baseOpcode == 0x64)
    {
        gcmASSERT(pInCodecHelperInst->instCtrl.roundingMode == 0x0 ||
                  pInCodecHelperInst->instCtrl.roundingMode == 0x1);

        /* This is actually a enable bit, not rounding mode */
        if (pMcCodec->pHwCfg->hwFeatureFlags.hasNewSinCosLogDiv)
        {
            pInCodecHelperInst->instCtrl.roundingMode = 0x1;
        }
    }

    return _Common_Encode_Mc_Alu_Inst(pMcCodec, mcCodecType, pInCodecHelperInst, &srcMap[0], (VSC_MC_INST*)pOutMcInst);
}

static gctBOOL _Encode_Mc_1_Src_Src0_Alu_Inst(VSC_MC_CODEC* pMcCodec,
                                              VSC_MC_CODEC_TYPE mcCodecType,
                                              VSC_MC_CODEC_INST* pInCodecHelperInst,
                                              VSC_MC_ALU_1_SRC_SRC0_INST* pOutMcInst)
{
    gctUINT srcMap[1] = {0};

    gcmASSERT(mcCodecType == VSC_MC_CODEC_TYPE_1_SRC_SRC0_ALU);
    gcmASSERT(pInCodecHelperInst->bDstValid);
    gcmASSERT(pInCodecHelperInst->srcCount == 1);

    ENCODE_EXT_OPCODE(pOutMcInst, pInCodecHelperInst->baseOpcode, pInCodecHelperInst->extOpcode);

    return _Common_Encode_Mc_Alu_Inst(pMcCodec, mcCodecType, pInCodecHelperInst, &srcMap[0], (VSC_MC_INST*)pOutMcInst);
}

static gctBOOL _Encode_Mc_1_Src_Src1_Alu_Inst(VSC_MC_CODEC* pMcCodec,
                                              VSC_MC_CODEC_TYPE mcCodecType,
                                              VSC_MC_CODEC_INST* pInCodecHelperInst,
                                              VSC_MC_ALU_1_SRC_SRC1_INST* pOutMcInst)
{
    gctUINT srcMap[1] = {1};

    gcmASSERT(mcCodecType == VSC_MC_CODEC_TYPE_1_SRC_SRC1_ALU);
    gcmASSERT(pInCodecHelperInst->bDstValid);
    gcmASSERT(pInCodecHelperInst->srcCount == 1);

    ENCODE_EXT_OPCODE(pOutMcInst, pInCodecHelperInst->baseOpcode, pInCodecHelperInst->extOpcode);

    return _Common_Encode_Mc_Alu_Inst(pMcCodec, mcCodecType, pInCodecHelperInst, &srcMap[0], (VSC_MC_INST*)pOutMcInst);
}

static gctBOOL _Encode_Mc_1_Src_Src2_Alu_Inst(VSC_MC_CODEC* pMcCodec,
                                              VSC_MC_CODEC_TYPE mcCodecType,
                                              VSC_MC_CODEC_INST* pInCodecHelperInst,
                                              VSC_MC_ALU_1_SRC_SRC2_INST* pOutMcInst)
{
    gctUINT srcMap[1] = {2};

    gcmASSERT(mcCodecType == VSC_MC_CODEC_TYPE_1_SRC_SRC2_ALU);
    gcmASSERT(pInCodecHelperInst->bDstValid);
    gcmASSERT(pInCodecHelperInst->srcCount == 1);

    if (pInCodecHelperInst->baseOpcode == 0x22 ||
        pInCodecHelperInst->baseOpcode == 0x23 ||
        pInCodecHelperInst->baseOpcode == 0x12)
    {
        gcmASSERT(pInCodecHelperInst->instCtrl.roundingMode == 0x0 ||
                  pInCodecHelperInst->instCtrl.roundingMode == 0x1);

        /* This is actually a enable bit, not rounding mode */
        if (pMcCodec->pHwCfg->hwFeatureFlags.hasNewSinCosLogDiv)
        {
            pInCodecHelperInst->instCtrl.roundingMode = 0x1;
        }
    }

    return _Common_Encode_Mc_Alu_Inst(pMcCodec, mcCodecType, pInCodecHelperInst, &srcMap[0], (VSC_MC_INST*)pOutMcInst);
}

static gctBOOL _Encode_Mc_Pack_Inst(VSC_MC_CODEC* pMcCodec,
                                    VSC_MC_CODEC_TYPE mcCodecType,
                                    VSC_MC_CODEC_INST* pInCodecHelperInst,
                                    VSC_MC_PACK_INST* pOutMcInst)
{
    gctUINT           srcIdx;

    gcmASSERT(mcCodecType == VSC_MC_CODEC_TYPE_PACK);
    gcmASSERT(pInCodecHelperInst->bDstValid);
    gcmASSERT(pInCodecHelperInst->srcCount == 3);

    /* Opcode */
    ENCODE_BASE_OPCODE((VSC_MC_INST*)pOutMcInst, pInCodecHelperInst->baseOpcode);

    /* Dst */
    _EncodeDst(pMcCodec, &pInCodecHelperInst->dst, gcvFALSE, (VSC_MC_INST*)pOutMcInst);

    /* Src */
    for (srcIdx = 0; srcIdx < pInCodecHelperInst->srcCount; srcIdx ++)
    {
        _EncodeSrc(pMcCodec, srcIdx, &pInCodecHelperInst->src[srcIdx], gcvFALSE, (VSC_MC_INST*)pOutMcInst);
    }

    /* Inst ctrl */
    pOutMcInst->inst.srcSelect = pInCodecHelperInst->instCtrl.u.srcSelect;
    _EncodeInstType(pMcCodec, VSC_MC_CODEC_TYPE_PACK, (VSC_MC_INST*)pOutMcInst,
                    pInCodecHelperInst->instCtrl.instType);
    _EncodeThreadType(pMcCodec, VSC_MC_CODEC_TYPE_PACK, (VSC_MC_INST*)pOutMcInst,
                      pInCodecHelperInst->instCtrl.threadType);

    return gcvTRUE;
}

static gctBOOL _Encode_Mc_Sample_Inst(VSC_MC_CODEC* pMcCodec,
                                      VSC_MC_CODEC_TYPE mcCodecType,
                                      VSC_MC_CODEC_INST* pInCodecHelperInst,
                                      VSC_MC_SAMPLE_INST* pOutMcInst)
{
    gcmASSERT(mcCodecType == VSC_MC_CODEC_TYPE_SAMPLE);
    gcmASSERT(pInCodecHelperInst->bDstValid);
    gcmASSERT(pInCodecHelperInst->srcCount >= 1 && pInCodecHelperInst->srcCount <= 4);

    return _Common_Encode_Mc_Sample_Inst(pMcCodec, mcCodecType, pInCodecHelperInst, (VSC_MC_INST*)pOutMcInst);
}

static gctBOOL _Encode_Mc_Sample_Ext_Inst(VSC_MC_CODEC* pMcCodec,
                                          VSC_MC_CODEC_TYPE mcCodecType,
                                          VSC_MC_CODEC_INST* pInCodecHelperInst,
                                          VSC_MC_SAMPLE_EXT_INST* pOutMcInst)
{
    gcmASSERT(mcCodecType == VSC_MC_CODEC_TYPE_SAMPLE_EXT);
    gcmASSERT(pInCodecHelperInst->bDstValid);
    gcmASSERT(pInCodecHelperInst->srcCount >= 1 && pInCodecHelperInst->srcCount <= 3);

    ENCODE_EXT_OPCODE(pOutMcInst, pInCodecHelperInst->baseOpcode, pInCodecHelperInst->extOpcode);
    return _Common_Encode_Mc_Sample_Inst(pMcCodec, mcCodecType, pInCodecHelperInst, (VSC_MC_INST*)pOutMcInst);
}

static gctBOOL _Encode_Mc_Load_Inst(VSC_MC_CODEC* pMcCodec,
                                    VSC_MC_CODEC_TYPE mcCodecType,
                                    VSC_MC_CODEC_INST* pInCodecHelperInst,
                                    VSC_MC_LD_INST* pOutMcInst)
{
    gcmASSERT(mcCodecType == VSC_MC_CODEC_TYPE_LOAD);
    gcmASSERT(pInCodecHelperInst->bDstValid);
    gcmASSERT(pInCodecHelperInst->srcCount == 2);

    return _Common_Encode_Mc_Load_Store_Inst(pMcCodec, mcCodecType, pInCodecHelperInst, gcvFALSE, (VSC_MC_INST*)pOutMcInst);
}

static gctBOOL _Encode_Mc_Img_Load_Inst(VSC_MC_CODEC* pMcCodec,
                                        VSC_MC_CODEC_TYPE mcCodecType,
                                        VSC_MC_CODEC_INST* pInCodecHelperInst,
                                        VSC_MC_IMG_LD_INST* pOutMcInst)
{
    gcmASSERT(mcCodecType == VSC_MC_CODEC_TYPE_IMG_LOAD);
    gcmASSERT(pInCodecHelperInst->bDstValid);
    gcmASSERT(pInCodecHelperInst->srcCount >= 2);

    return _Common_Encode_Mc_Load_Store_Inst(pMcCodec, mcCodecType, pInCodecHelperInst, gcvTRUE, (VSC_MC_INST*)pOutMcInst);
}

static gctBOOL _Encode_Mc_Store_Inst(VSC_MC_CODEC* pMcCodec,
                                     VSC_MC_CODEC_TYPE mcCodecType,
                                     VSC_MC_CODEC_INST* pInCodecHelperInst,
                                     VSC_MC_ST_INST* pOutMcInst)
{
    gcmASSERT(mcCodecType == VSC_MC_CODEC_TYPE_STORE);
    gcmASSERT(!pInCodecHelperInst->bDstValid || pInCodecHelperInst->baseOpcode == MC_AUXILIARY_OP_CODE_USC_STORE);
    gcmASSERT(pInCodecHelperInst->srcCount == 3);

    if (pInCodecHelperInst->baseOpcode != MC_AUXILIARY_OP_CODE_USC_STORE)
    {
        /* Normal store has a special memory writemask */
        pOutMcInst->inst.writeMask = pInCodecHelperInst->dst.u.nmlDst.writeMask;
    }

    return _Common_Encode_Mc_Load_Store_Inst(pMcCodec, mcCodecType, pInCodecHelperInst, gcvFALSE, (VSC_MC_INST*)pOutMcInst);
}

static gctBOOL _Encode_Mc_Img_Store_Inst(VSC_MC_CODEC* pMcCodec,
                                         VSC_MC_CODEC_TYPE mcCodecType,
                                         VSC_MC_CODEC_INST* pInCodecHelperInst,
                                         VSC_MC_IMG_ST_INST* pOutMcInst)
{
    gcmASSERT(mcCodecType == VSC_MC_CODEC_TYPE_IMG_STORE);
    gcmASSERT(!pInCodecHelperInst->bDstValid ||
              (pInCodecHelperInst->baseOpcode == MC_AUXILIARY_OP_CODE_USC_IMG_STORE ||
               pInCodecHelperInst->baseOpcode == MC_AUXILIARY_OP_CODE_USC_IMG_STORE_3D));
    gcmASSERT(pInCodecHelperInst->srcCount == 3);

    if (pInCodecHelperInst->baseOpcode != MC_AUXILIARY_OP_CODE_USC_IMG_STORE &&
        pInCodecHelperInst->baseOpcode != MC_AUXILIARY_OP_CODE_USC_IMG_STORE_3D)
    {
        /* Normal img-store must have full memory writemask */
        pOutMcInst->inst.writeMask = WRITEMASK_ALL;
    }

    return _Common_Encode_Mc_Load_Store_Inst(pMcCodec, mcCodecType, pInCodecHelperInst, gcvTRUE, (VSC_MC_INST*)pOutMcInst);
}

static gctBOOL _Encode_Mc_Img_Atom_Inst(VSC_MC_CODEC* pMcCodec,
                                        VSC_MC_CODEC_TYPE mcCodecType,
                                        VSC_MC_CODEC_INST* pInCodecHelperInst,
                                        VSC_MC_IMG_ATOM_INST* pOutMcInst)
{
    gcmASSERT(mcCodecType == VSC_MC_CODEC_TYPE_IMG_ATOM);
    gcmASSERT(pInCodecHelperInst->bDstValid);
    gcmASSERT(pInCodecHelperInst->srcCount == 3);

    return _Common_Encode_Mc_Load_Store_Inst(pMcCodec, mcCodecType, pInCodecHelperInst, gcvTRUE, (VSC_MC_INST*)pOutMcInst);
}

static gctBOOL _Encode_Mc_Store_Attr_Inst(VSC_MC_CODEC* pMcCodec,
                                          VSC_MC_CODEC_TYPE mcCodecType,
                                          VSC_MC_CODEC_INST* pInCodecHelperInst,
                                          VSC_MC_STORE_ATTR_INST* pOutMcInst)
{
    gctUINT           srcIdx;

    gcmASSERT(mcCodecType == VSC_MC_CODEC_TYPE_STORE_ATTR);
    gcmASSERT(!pInCodecHelperInst->bDstValid || pInCodecHelperInst->baseOpcode == MC_AUXILIARY_OP_CODE_USC_STORE_ATTR);
    gcmASSERT(pInCodecHelperInst->srcCount == 3);

    /* Opcode */
    ENCODE_BASE_OPCODE((VSC_MC_INST*)pOutMcInst, _MapLdStAuxOpcodeToHwOpcode(pInCodecHelperInst->baseOpcode));

    if (pInCodecHelperInst->baseOpcode == MC_AUXILIARY_OP_CODE_USC_STORE_ATTR)
    {
        _EncodeDst(pMcCodec, &pInCodecHelperInst->dst, gcvFALSE, (VSC_MC_INST*)pOutMcInst);
    }
    else
    {
        /* Normal store_attr has a special memory writemask */
        pOutMcInst->inst.writeMask = pInCodecHelperInst->dst.u.nmlDst.writeMask;
    }

    /* Usc sync bit */
    pOutMcInst->inst.bNeedUscSync = pInCodecHelperInst->instCtrl.u.bNeedUscSync ? 1 : 0;

    /* Src */
    for (srcIdx = 0; srcIdx < pInCodecHelperInst->srcCount; srcIdx ++)
    {
        _EncodeSrc(pMcCodec, srcIdx, &pInCodecHelperInst->src[srcIdx], gcvFALSE, (VSC_MC_INST*)pOutMcInst);
    }

    _EncodeInstType(pMcCodec, mcCodecType, (VSC_MC_INST*)pOutMcInst, pInCodecHelperInst->instCtrl.instType);
    _EncodeThreadType(pMcCodec, mcCodecType, (VSC_MC_INST*)pOutMcInst, pInCodecHelperInst->instCtrl.threadType);

    return gcvTRUE;
}

static gctBOOL _Encode_Mc_Select_Map_Inst(VSC_MC_CODEC* pMcCodec,
                                          VSC_MC_CODEC_TYPE mcCodecType,
                                          VSC_MC_CODEC_INST* pInCodecHelperInst,
                                          VSC_MC_SELECT_MAP_INST* pOutMcInst)
{
    gctUINT           srcIdx;

    gcmASSERT(mcCodecType == VSC_MC_CODEC_TYPE_SELECT_MAP);
    gcmASSERT(pInCodecHelperInst->bDstValid);
    gcmASSERT(pInCodecHelperInst->srcCount == 3);

    /* Opcode */
    ENCODE_BASE_OPCODE((VSC_MC_INST*)pOutMcInst, pInCodecHelperInst->baseOpcode);

    /* Dst */
    _EncodeDst(pMcCodec, &pInCodecHelperInst->dst, gcvFALSE, (VSC_MC_INST*)pOutMcInst);

    /* Src */
    for (srcIdx = 0; srcIdx < pInCodecHelperInst->srcCount; srcIdx ++)
    {
        _EncodeSrc(pMcCodec, srcIdx, &pInCodecHelperInst->src[srcIdx], gcvFALSE, (VSC_MC_INST*)pOutMcInst);
    }

    /* Inst ctrl */
    pOutMcInst->inst.rangeToMatch = pInCodecHelperInst->instCtrl.u.smCtrl.rangeToMatch;
    pOutMcInst->inst.bCompSel = pInCodecHelperInst->instCtrl.u.smCtrl.bCompSel;
    _EncodeInstType(pMcCodec, mcCodecType, (VSC_MC_INST*)pOutMcInst, pInCodecHelperInst->instCtrl.instType);
    _EncodeThreadType(pMcCodec, mcCodecType, (VSC_MC_INST*)pOutMcInst, pInCodecHelperInst->instCtrl.threadType);

    return gcvTRUE;
}

static gctBOOL _Encode_Mc_Direct_Branch_0_Inst(VSC_MC_CODEC* pMcCodec,
                                               VSC_MC_CODEC_TYPE mcCodecType,
                                               VSC_MC_CODEC_INST* pInCodecHelperInst,
                                               VSC_MC_DIRECT_BRANCH_0_INST* pOutMcInst)
{
    gctUINT           srcIdx, branchTargetSrcIdx;

    gcmASSERT(mcCodecType == VSC_MC_CODEC_TYPE_DIRECT_BRANCH_0);
    gcmASSERT(!pInCodecHelperInst->bDstValid);

    /* Opcode */
    ENCODE_BASE_OPCODE((VSC_MC_INST*)pOutMcInst, pInCodecHelperInst->baseOpcode);

    /* Src */
    gcmASSERT(pInCodecHelperInst->srcCount == _condOp2SrcCount[pInCodecHelperInst->instCtrl.condOpCode] + 1);

    for (srcIdx = 0; srcIdx < _condOp2SrcCount[pInCodecHelperInst->instCtrl.condOpCode]; srcIdx ++)
    {
        _EncodeSrc(pMcCodec, srcIdx, &pInCodecHelperInst->src[srcIdx], gcvFALSE, (VSC_MC_INST*)pOutMcInst);
    }

    branchTargetSrcIdx = _condOp2SrcCount[pInCodecHelperInst->instCtrl.condOpCode];

    /* Branch target */
    gcmASSERT(pInCodecHelperInst->src[branchTargetSrcIdx].regType == 0x7);
    gcmASSERT(pInCodecHelperInst->src[branchTargetSrcIdx].u.imm.immType == 0x2);
    pOutMcInst->inst.branchTarget = pInCodecHelperInst->src[branchTargetSrcIdx].u.imm.immData.ui;

    /* Inst ctrl */
    pOutMcInst->inst.condOpCode = pInCodecHelperInst->instCtrl.condOpCode;
    pOutMcInst->inst.bPackMode = pInCodecHelperInst->instCtrl.bPacked;
    pOutMcInst->inst.loopOpType = pInCodecHelperInst->instCtrl.u.loopOpType;
    _EncodeInstType(pMcCodec, VSC_MC_CODEC_TYPE_DIRECT_BRANCH_0, (VSC_MC_INST*)pOutMcInst,
                    pInCodecHelperInst->instCtrl.instType);

    return gcvTRUE;
}

static gctBOOL _Encode_Mc_Direct_Branch_1_Inst(VSC_MC_CODEC* pMcCodec,
                                               VSC_MC_CODEC_TYPE mcCodecType,
                                               VSC_MC_CODEC_INST* pInCodecHelperInst,
                                               VSC_MC_DIRECT_BRANCH_1_INST* pOutMcInst)
{
    gctUINT           srcIdx, branchTargetSrcIdx;
    gctUINT           branchTarget20Bit;

    gcmASSERT(mcCodecType == VSC_MC_CODEC_TYPE_DIRECT_BRANCH_1);
    gcmASSERT(!pInCodecHelperInst->bDstValid);

    /* Opcode */
    ENCODE_BASE_OPCODE((VSC_MC_INST*)pOutMcInst, pInCodecHelperInst->baseOpcode);

    /* Src */
    gcmASSERT(pInCodecHelperInst->srcCount == _condOp2SrcCount[pInCodecHelperInst->instCtrl.condOpCode] + 1);

    for (srcIdx = 0; srcIdx < _condOp2SrcCount[pInCodecHelperInst->instCtrl.condOpCode]; srcIdx ++)
    {
        _EncodeSrc(pMcCodec, srcIdx, &pInCodecHelperInst->src[srcIdx], gcvFALSE, (VSC_MC_INST*)pOutMcInst);
    }

    branchTargetSrcIdx = _condOp2SrcCount[pInCodecHelperInst->instCtrl.condOpCode];

    /* Branch target */
    gcmASSERT(pInCodecHelperInst->src[branchTargetSrcIdx].regType == 0x7);
    branchTarget20Bit = _Conver32BitImm_2_20BitImm(pInCodecHelperInst->src[branchTargetSrcIdx].u.imm.immData,
                                                   pInCodecHelperInst->src[branchTargetSrcIdx].u.imm.immType);
    pOutMcInst->inst.bSrc2Valid = gcvTRUE;
    pOutMcInst->inst.src2Type = 0x7;
    pOutMcInst->inst.immType = pInCodecHelperInst->src[branchTargetSrcIdx].u.imm.immType;
    pOutMcInst->inst.branchTargetBit0_8 = (branchTarget20Bit & 0x1FF);
    pOutMcInst->inst.branchTargetBit9_18 = ((branchTarget20Bit>>9) & 0x3FF);
    pOutMcInst->inst.branchTargetBit19 = ((branchTarget20Bit>>19) & 0x01);

    /* Inst ctrl */
    pOutMcInst->inst.condOpCode = pInCodecHelperInst->instCtrl.condOpCode;
    pOutMcInst->inst.bPackMode = pInCodecHelperInst->instCtrl.bPacked;
    _EncodeInstType(pMcCodec, VSC_MC_CODEC_TYPE_DIRECT_BRANCH_1, (VSC_MC_INST*)pOutMcInst,
                    pInCodecHelperInst->instCtrl.instType);
    _EncodeThreadType(pMcCodec, VSC_MC_CODEC_TYPE_DIRECT_BRANCH_1, (VSC_MC_INST*)pOutMcInst,
                      pInCodecHelperInst->instCtrl.threadType);

    return gcvTRUE;
}

static gctBOOL _Encode_Mc_Indirect_Branch_Inst(VSC_MC_CODEC* pMcCodec,
                                               VSC_MC_CODEC_TYPE mcCodecType,
                                               VSC_MC_CODEC_INST* pInCodecHelperInst,
                                               VSC_MC_INDIRECT_BRANCH_INST* pOutMcInst)
{
    gcmASSERT(mcCodecType == VSC_MC_CODEC_TYPE_INDIRECT_BRANCH);
    gcmASSERT(!pInCodecHelperInst->bDstValid);
    gcmASSERT(pInCodecHelperInst->srcCount == 1);

    gcmASSERT(pInCodecHelperInst->instCtrl.condOpCode == 0x00);

    /* Opcode */
    ENCODE_BASE_OPCODE((VSC_MC_INST*)pOutMcInst, pInCodecHelperInst->baseOpcode);

    /* Branch target */
    _EncodeSrc(pMcCodec, 2, &pInCodecHelperInst->src[0], gcvFALSE, (VSC_MC_INST*)pOutMcInst);

    /* Inst ctrl */
    pOutMcInst->inst.condOpCode = pInCodecHelperInst->instCtrl.condOpCode;
    pOutMcInst->inst.bPackMode = pInCodecHelperInst->instCtrl.bPacked;
    _EncodeInstType(pMcCodec, VSC_MC_CODEC_TYPE_INDIRECT_BRANCH, (VSC_MC_INST*)pOutMcInst,
                    pInCodecHelperInst->instCtrl.instType);
    _EncodeThreadType(pMcCodec, VSC_MC_CODEC_TYPE_INDIRECT_BRANCH, (VSC_MC_INST*)pOutMcInst,
                      pInCodecHelperInst->instCtrl.threadType);

    return gcvTRUE;
}

static gctBOOL _Encode_Mc_Direct_Call_Inst(VSC_MC_CODEC* pMcCodec,
                                           VSC_MC_CODEC_TYPE mcCodecType,
                                           VSC_MC_CODEC_INST* pInCodecHelperInst,
                                           VSC_MC_DIRECT_CALL_INST* pOutMcInst)
{
    gcmASSERT(mcCodecType == VSC_MC_CODEC_TYPE_DIRECT_CALL);
    gcmASSERT(!pInCodecHelperInst->bDstValid);
    gcmASSERT(pInCodecHelperInst->srcCount == 1);

    /* Opcode */
    ENCODE_BASE_OPCODE((VSC_MC_INST*)pOutMcInst, pInCodecHelperInst->baseOpcode);

    /* Call target */
    gcmASSERT(pInCodecHelperInst->src[0].regType == 0x7);
    gcmASSERT(pInCodecHelperInst->src[0].u.imm.immType == 0x2);
    pOutMcInst->inst.callTarget = pInCodecHelperInst->src[0].u.imm.immData.ui;

    return gcvTRUE;
}

static gctBOOL _Encode_Mc_Indirect_Call_Inst(VSC_MC_CODEC* pMcCodec,
                                             VSC_MC_CODEC_TYPE mcCodecType,
                                             VSC_MC_CODEC_INST* pInCodecHelperInst,
                                             VSC_MC_INDIRECT_CALL_INST* pOutMcInst)
{
    gcmASSERT(mcCodecType == VSC_MC_CODEC_TYPE_INDIRECT_CALL);
    gcmASSERT(!pInCodecHelperInst->bDstValid);
    gcmASSERT(pInCodecHelperInst->srcCount == 1);

    /* Opcode */
    ENCODE_BASE_OPCODE((VSC_MC_INST*)pOutMcInst, pInCodecHelperInst->baseOpcode);

    /* Call target */
    _EncodeSrc(pMcCodec, 2, &pInCodecHelperInst->src[0], gcvFALSE, (VSC_MC_INST*)pOutMcInst);

    /* Inst ctrl */
    _EncodeInstType(pMcCodec, VSC_MC_CODEC_TYPE_INDIRECT_CALL, (VSC_MC_INST*)pOutMcInst,
                    pInCodecHelperInst->instCtrl.instType);
    _EncodeThreadType(pMcCodec, VSC_MC_CODEC_TYPE_INDIRECT_CALL, (VSC_MC_INST*)pOutMcInst,
                      pInCodecHelperInst->instCtrl.threadType);

    return gcvTRUE;
}

static gctBOOL _Encode_Mc_Loop_Inst(VSC_MC_CODEC* pMcCodec,
                                    VSC_MC_CODEC_TYPE mcCodecType,
                                    VSC_MC_CODEC_INST* pInCodecHelperInst,
                                    VSC_MC_LOOP_INST* pOutMcInst)
{
    gcmASSERT(mcCodecType == VSC_MC_CODEC_TYPE_LOOP);
    gcmASSERT(!pInCodecHelperInst->bDstValid);
    gcmASSERT(pInCodecHelperInst->srcCount == 2);

    /* Opcode */
    ENCODE_BASE_OPCODE((VSC_MC_INST*)pOutMcInst, pInCodecHelperInst->baseOpcode);

    /* Src */
    _EncodeSrc(pMcCodec, 1, &pInCodecHelperInst->src[0], gcvFALSE, (VSC_MC_INST*)pOutMcInst);

    /* Loop target */
    gcmASSERT(pInCodecHelperInst->src[1].regType == 0x7);
    gcmASSERT(pInCodecHelperInst->src[1].u.imm.immType == 0x2);
    pOutMcInst->inst.branchTarget = pInCodecHelperInst->src[1].u.imm.immData.ui;

    /* Inst ctrl */
    _EncodeInstType(pMcCodec, VSC_MC_CODEC_TYPE_LOOP, (VSC_MC_INST*)pOutMcInst,
                    pInCodecHelperInst->instCtrl.instType);

    return gcvTRUE;
}

static gctBOOL _Encode_Mc_Emit_Inst(VSC_MC_CODEC* pMcCodec,
                                    VSC_MC_CODEC_TYPE mcCodecType,
                                    VSC_MC_CODEC_INST* pInCodecHelperInst,
                                    VSC_MC_EMIT_INST* pOutMcInst)
{
    gctUINT           srcIdx;

    gcmASSERT(mcCodecType == VSC_MC_CODEC_TYPE_EMIT);
    gcmASSERT(pInCodecHelperInst->bDstValid);
    gcmASSERT(pInCodecHelperInst->srcCount >= 1 && pInCodecHelperInst->srcCount <= 2);
    gcmASSERT(pInCodecHelperInst->src[0].u.reg.regNo == 0);
    gcmASSERT(pInCodecHelperInst->dst.regNo == 0);

    /* Opcode */
    ENCODE_BASE_OPCODE((VSC_MC_INST*)pOutMcInst, pInCodecHelperInst->baseOpcode);
    ENCODE_EXT_OPCODE(pOutMcInst, pInCodecHelperInst->baseOpcode, pInCodecHelperInst->extOpcode);

    /* Dst */
    _EncodeDst(pMcCodec, &pInCodecHelperInst->dst, gcvFALSE, (VSC_MC_INST*)pOutMcInst);

    /* Src */
    for (srcIdx = 0; srcIdx < pInCodecHelperInst->srcCount; srcIdx ++)
    {
        _EncodeSrc(pMcCodec, srcIdx, &pInCodecHelperInst->src[srcIdx], gcvFALSE, (VSC_MC_INST*)pOutMcInst);
    }

    /* Inst ctrl */
    _EncodeInstType(pMcCodec, VSC_MC_CODEC_TYPE_EMIT, (VSC_MC_INST*)pOutMcInst,
                    pInCodecHelperInst->instCtrl.instType);
    pOutMcInst->inst.bNeedRestartPrim = pInCodecHelperInst->instCtrl.u.bNeedRestartPrim;

    return gcvTRUE;
}

static PFN_MC_ENCODER _pfn_mc_encoder[] =
{
    gcvNULL,
    (PFN_MC_ENCODER)_Encode_Mc_No_Opnd_Inst,
    (PFN_MC_ENCODER)_Encode_Mc_3_Srcs_Alu_Inst,
    (PFN_MC_ENCODER)_Encode_Mc_2_Srcs_Src0_Src1_Alu_Inst,
    (PFN_MC_ENCODER)_Encode_Mc_2_Srcs_Src0_Src2_Alu_Inst,
    (PFN_MC_ENCODER)_Encode_Mc_2_Srcs_Src1_Src2_Alu_Inst,
    (PFN_MC_ENCODER)_Encode_Mc_1_Src_Src0_Alu_Inst,
    (PFN_MC_ENCODER)_Encode_Mc_1_Src_Src1_Alu_Inst,
    (PFN_MC_ENCODER)_Encode_Mc_1_Src_Src2_Alu_Inst,
    (PFN_MC_ENCODER)_Encode_Mc_Pack_Inst,
    (PFN_MC_ENCODER)_Encode_Mc_Sample_Inst,
    (PFN_MC_ENCODER)_Encode_Mc_Sample_Ext_Inst,
    (PFN_MC_ENCODER)_Encode_Mc_Load_Inst,
    (PFN_MC_ENCODER)_Encode_Mc_Img_Load_Inst,
    (PFN_MC_ENCODER)_Encode_Mc_Store_Inst,
    (PFN_MC_ENCODER)_Encode_Mc_Img_Store_Inst,
    (PFN_MC_ENCODER)_Encode_Mc_Img_Atom_Inst,
    (PFN_MC_ENCODER)_Encode_Mc_Store_Attr_Inst,
    (PFN_MC_ENCODER)_Encode_Mc_Select_Map_Inst,
    (PFN_MC_ENCODER)_Encode_Mc_Direct_Branch_0_Inst,
    (PFN_MC_ENCODER)_Encode_Mc_Direct_Branch_1_Inst,
    (PFN_MC_ENCODER)_Encode_Mc_Indirect_Branch_Inst,
    (PFN_MC_ENCODER)_Encode_Mc_Direct_Call_Inst,
    (PFN_MC_ENCODER)_Encode_Mc_Indirect_Call_Inst,
    (PFN_MC_ENCODER)_Encode_Mc_Loop_Inst,
    (PFN_MC_ENCODER)_Encode_Mc_Emit_Inst,
    gcvNULL
};

/* Decode routines */

static gctBOOL _Decode_Mc_No_Opnd_Inst(VSC_MC_CODEC* pMcCodec,
                                       VSC_MC_CODEC_TYPE mcCodecType,
                                       VSC_MC_NO_OPERAND_INST* pInMCInst,
                                       VSC_MC_CODEC_INST* pOutCodecHelperInst)
{
    gcmASSERT(mcCodecType == VSC_MC_CODEC_TYPE_NO_OPND);

    return gcvTRUE;
}

static gctBOOL _Decode_Mc_3_Srcs_Alu_Inst(VSC_MC_CODEC* pMcCodec,
                                          VSC_MC_CODEC_TYPE mcCodecType,
                                          VSC_MC_ALU_3_SRCS_INST* pInMcInst,
                                          VSC_MC_CODEC_INST* pOutCodecHelperInst)
{
    gcmASSERT(mcCodecType == VSC_MC_CODEC_TYPE_3_SRCS_ALU);

    return gcvTRUE;
}

static gctBOOL _Decode_Mc_2_Srcs_Src0_Src1_Alu_Inst(VSC_MC_CODEC* pMcCodec,
                                                    VSC_MC_CODEC_TYPE mcCodecType,
                                                    VSC_MC_ALU_2_SRCS_SRC0_SRC1_INST* pInMcInst,
                                                    VSC_MC_CODEC_INST* pOutCodecHelperInst)
{
    gcmASSERT(mcCodecType == VSC_MC_CODEC_TYPE_2_SRCS_SRC0_SRC1_ALU);

    return gcvTRUE;
}

static gctBOOL _Decode_Mc_2_Srcs_Src0_Src2_Alu_Inst(VSC_MC_CODEC* pMcCodec,
                                                    VSC_MC_CODEC_TYPE mcCodecType,
                                                    VSC_MC_ALU_2_SRCS_SRC0_SRC2_INST* pInMcInst,
                                                    VSC_MC_CODEC_INST* pOutCodecHelperInst)
{
    gcmASSERT(mcCodecType == VSC_MC_CODEC_TYPE_2_SRCS_SRC0_SRC2_ALU);

    return gcvTRUE;
}

static gctBOOL _Decode_Mc_2_Srcs_Src1_Src2_Alu_Inst(VSC_MC_CODEC* pMcCodec,
                                                    VSC_MC_CODEC_TYPE mcCodecType,
                                                    VSC_MC_ALU_2_SRCS_SRC1_SRC2_INST* pInMcInst,
                                                    VSC_MC_CODEC_INST* pOutCodecHelperInst)
{
    gcmASSERT(mcCodecType == VSC_MC_CODEC_TYPE_2_SRCS_SRC1_SRC2_ALU);

    return gcvTRUE;
}

static gctBOOL _Decode_Mc_1_Src_Src0_Alu_Inst(VSC_MC_CODEC* pMcCodec,
                                              VSC_MC_CODEC_TYPE mcCodecType,
                                              VSC_MC_ALU_1_SRC_SRC0_INST* pInMcInst,
                                              VSC_MC_CODEC_INST* pOutCodecHelperInst)
{
    gcmASSERT(mcCodecType == VSC_MC_CODEC_TYPE_1_SRC_SRC0_ALU);

    return gcvTRUE;
}

static gctBOOL _Decode_Mc_1_Src_Src1_Alu_Inst(VSC_MC_CODEC* pMcCodec,
                                              VSC_MC_CODEC_TYPE mcCodecType,
                                              VSC_MC_ALU_1_SRC_SRC1_INST* pInMcInst,
                                              VSC_MC_CODEC_INST* pOutCodecHelperInst)
{
    gcmASSERT(mcCodecType == VSC_MC_CODEC_TYPE_1_SRC_SRC1_ALU);

    return gcvTRUE;
}

static gctBOOL _Decode_Mc_1_Src_Src2_Alu_Inst(VSC_MC_CODEC* pMcCodec,
                                              VSC_MC_CODEC_TYPE mcCodecType,
                                              VSC_MC_ALU_1_SRC_SRC2_INST* pInMcInst,
                                              VSC_MC_CODEC_INST* pOutCodecHelperInst)
{
    gcmASSERT(mcCodecType == VSC_MC_CODEC_TYPE_1_SRC_SRC2_ALU);

    return gcvTRUE;
}

static gctBOOL _Decode_Mc_Pack_Inst(VSC_MC_CODEC* pMcCodec,
                                    VSC_MC_CODEC_TYPE mcCodecType,
                                    VSC_MC_PACK_INST* pInMcInst,
                                    VSC_MC_CODEC_INST* pOutCodecHelperInst)
{
    gcmASSERT(mcCodecType == VSC_MC_CODEC_TYPE_PACK);

    return gcvTRUE;
}

static gctBOOL _Decode_Mc_Sample_Inst(VSC_MC_CODEC* pMcCodec,
                                      VSC_MC_CODEC_TYPE mcCodecType,
                                      VSC_MC_SAMPLE_INST* pInMcInst,
                                      VSC_MC_CODEC_INST* pOutCodecHelperInst)
{
    gcmASSERT(mcCodecType == VSC_MC_CODEC_TYPE_SAMPLE);

    return gcvTRUE;
}

static gctBOOL _Decode_Mc_Sample_Ext_Inst(VSC_MC_CODEC* pMcCodec,
                                          VSC_MC_CODEC_TYPE mcCodecType,
                                          VSC_MC_SAMPLE_EXT_INST* pInMcInst,
                                          VSC_MC_CODEC_INST* pOutCodecHelperInst)
{
    gcmASSERT(mcCodecType == VSC_MC_CODEC_TYPE_SAMPLE_EXT);

    return gcvTRUE;
}

static gctBOOL _Decode_Mc_Load_Inst(VSC_MC_CODEC* pMcCodec,
                                    VSC_MC_CODEC_TYPE mcCodecType,
                                    VSC_MC_LD_INST* pInMcInst,
                                    VSC_MC_CODEC_INST* pOutCodecHelperInst)
{
    gcmASSERT(mcCodecType == VSC_MC_CODEC_TYPE_LOAD);

    return gcvTRUE;
}

static gctBOOL _Decode_Mc_Img_Load_Inst(VSC_MC_CODEC* pMcCodec,
                                        VSC_MC_CODEC_TYPE mcCodecType,
                                        VSC_MC_IMG_LD_INST* pInMcInst,
                                        VSC_MC_CODEC_INST* pOutCodecHelperInst)
{
    gcmASSERT(mcCodecType == VSC_MC_CODEC_TYPE_IMG_LOAD);

    return gcvTRUE;
}

static gctBOOL _Decode_Mc_Store_Inst(VSC_MC_CODEC* pMcCodec,
                                     VSC_MC_CODEC_TYPE mcCodecType,
                                     VSC_MC_ST_INST* pInMcInst,
                                     VSC_MC_CODEC_INST* pOutCodecHelperInst)
{
    gcmASSERT(mcCodecType == VSC_MC_CODEC_TYPE_STORE);

    return gcvTRUE;
}

static gctBOOL _Decode_Mc_Img_Store_Inst(VSC_MC_CODEC* pMcCodec,
                                         VSC_MC_CODEC_TYPE mcCodecType,
                                         VSC_MC_ST_INST* pInMcInst,
                                         VSC_MC_CODEC_INST* pOutCodecHelperInst)
{
    gcmASSERT(mcCodecType == VSC_MC_CODEC_TYPE_IMG_STORE);

    return gcvTRUE;
}

static gctBOOL _Decode_Mc_Img_Atom_Inst(VSC_MC_CODEC* pMcCodec,
                                         VSC_MC_CODEC_TYPE mcCodecType,
                                         VSC_MC_IMG_ATOM_INST* pInMcInst,
                                         VSC_MC_CODEC_INST* pOutCodecHelperInst)
{
    gcmASSERT(mcCodecType == VSC_MC_CODEC_TYPE_IMG_ATOM);

    return gcvTRUE;
}

static gctBOOL _Decode_Mc_Store_Attr_Inst(VSC_MC_CODEC* pMcCodec,
                                          VSC_MC_CODEC_TYPE mcCodecType,
                                          VSC_MC_STORE_ATTR_INST* pInMcInst,
                                          VSC_MC_CODEC_INST* pOutCodecHelperInst)
{
    gcmASSERT(mcCodecType == VSC_MC_CODEC_TYPE_STORE_ATTR);

    return gcvTRUE;
}

static gctBOOL _Decode_Mc_Select_Map_Inst(VSC_MC_CODEC* pMcCodec,
                                          VSC_MC_CODEC_TYPE mcCodecType,
                                          VSC_MC_SELECT_MAP_INST* pInMcInst,
                                          VSC_MC_CODEC_INST* pOutCodecHelperInst)
{
    gcmASSERT(mcCodecType == VSC_MC_CODEC_TYPE_SELECT_MAP);

    return gcvTRUE;
}

static gctBOOL _Decode_Mc_Direct_Branch_0_Inst(VSC_MC_CODEC* pMcCodec,
                                               VSC_MC_CODEC_TYPE mcCodecType,
                                               VSC_MC_DIRECT_BRANCH_0_INST* pInMcInst,
                                               VSC_MC_CODEC_INST* pOutCodecHelperInst)
{
    gcmASSERT(mcCodecType == VSC_MC_CODEC_TYPE_DIRECT_BRANCH_0);

    return gcvTRUE;
}

static gctBOOL _Decode_Mc_Direct_Branch_1_Inst(VSC_MC_CODEC* pMcCodec,
                                               VSC_MC_CODEC_TYPE mcCodecType,
                                               VSC_MC_DIRECT_BRANCH_1_INST* pInMcInst,
                                               VSC_MC_CODEC_INST* pOutCodecHelperInst)
{
    gcmASSERT(mcCodecType == VSC_MC_CODEC_TYPE_DIRECT_BRANCH_1);

    return gcvTRUE;
}

static gctBOOL _Decode_Mc_Indirect_Branch_Inst(VSC_MC_CODEC* pMcCodec,
                                               VSC_MC_CODEC_TYPE mcCodecType,
                                               VSC_MC_INDIRECT_BRANCH_INST* pInMcInst,
                                               VSC_MC_CODEC_INST* pOutCodecHelperInst)
{
    gcmASSERT(mcCodecType == VSC_MC_CODEC_TYPE_INDIRECT_BRANCH);

    return gcvTRUE;
}

static gctBOOL _Decode_Mc_Direct_Call_Inst(VSC_MC_CODEC* pMcCodec,
                                           VSC_MC_CODEC_TYPE mcCodecType,
                                           VSC_MC_DIRECT_CALL_INST* pInMcInst,
                                           VSC_MC_CODEC_INST* pOutCodecHelperInst)
{
    gcmASSERT(mcCodecType == VSC_MC_CODEC_TYPE_DIRECT_CALL);

    return gcvTRUE;
}

static gctBOOL _Decode_Mc_Indirect_Call_Inst(VSC_MC_CODEC* pMcCodec,
                                             VSC_MC_CODEC_TYPE mcCodecType,
                                             VSC_MC_INDIRECT_CALL_INST* pInMcInst,
                                             VSC_MC_CODEC_INST* pOutCodecHelperInst)
{
    gcmASSERT(mcCodecType == VSC_MC_CODEC_TYPE_INDIRECT_CALL);

    return gcvTRUE;
}

static gctBOOL _Decode_Mc_Loop_Inst(VSC_MC_CODEC* pMcCodec,
                                    VSC_MC_CODEC_TYPE mcCodecType,
                                    VSC_MC_LOOP_INST* pInMcInst,
                                    VSC_MC_CODEC_INST* pOutCodecHelperInst)
{
    gcmASSERT(mcCodecType == VSC_MC_CODEC_TYPE_LOOP);

    return gcvTRUE;
}

static gctBOOL _Decode_Mc_Emit_Inst(VSC_MC_CODEC* pMcCodec,
                                    VSC_MC_CODEC_TYPE mcCodecType,
                                    VSC_MC_EMIT_INST* pInMcInst,
                                    VSC_MC_CODEC_INST* pOutCodecHelperInst)
{
    gcmASSERT(mcCodecType == VSC_MC_CODEC_TYPE_EMIT);

    return gcvTRUE;
}

static PFN_MC_DECODER _pfn_mc_decoder[] =
{
    gcvNULL,
    (PFN_MC_DECODER)_Decode_Mc_No_Opnd_Inst,
    (PFN_MC_DECODER)_Decode_Mc_3_Srcs_Alu_Inst,
    (PFN_MC_DECODER)_Decode_Mc_2_Srcs_Src0_Src1_Alu_Inst,
    (PFN_MC_DECODER)_Decode_Mc_2_Srcs_Src0_Src2_Alu_Inst,
    (PFN_MC_DECODER)_Decode_Mc_2_Srcs_Src1_Src2_Alu_Inst,
    (PFN_MC_DECODER)_Decode_Mc_1_Src_Src0_Alu_Inst,
    (PFN_MC_DECODER)_Decode_Mc_1_Src_Src1_Alu_Inst,
    (PFN_MC_DECODER)_Decode_Mc_1_Src_Src2_Alu_Inst,
    (PFN_MC_DECODER)_Decode_Mc_Pack_Inst,
    (PFN_MC_DECODER)_Decode_Mc_Sample_Inst,
    (PFN_MC_DECODER)_Decode_Mc_Sample_Ext_Inst,
    (PFN_MC_DECODER)_Decode_Mc_Load_Inst,
    (PFN_MC_DECODER)_Decode_Mc_Img_Load_Inst,
    (PFN_MC_DECODER)_Decode_Mc_Store_Inst,
    (PFN_MC_DECODER)_Decode_Mc_Img_Store_Inst,
    (PFN_MC_DECODER)_Decode_Mc_Img_Atom_Inst,
    (PFN_MC_DECODER)_Decode_Mc_Store_Attr_Inst,
    (PFN_MC_DECODER)_Decode_Mc_Select_Map_Inst,
    (PFN_MC_DECODER)_Decode_Mc_Direct_Branch_0_Inst,
    (PFN_MC_DECODER)_Decode_Mc_Direct_Branch_1_Inst,
    (PFN_MC_DECODER)_Decode_Mc_Indirect_Branch_Inst,
    (PFN_MC_DECODER)_Decode_Mc_Direct_Call_Inst,
    (PFN_MC_DECODER)_Decode_Mc_Indirect_Call_Inst,
    (PFN_MC_DECODER)_Decode_Mc_Loop_Inst,
    (PFN_MC_DECODER)_Decode_Mc_Emit_Inst,
    gcvNULL
};

static void _dbgStopHere()
{
    return ;
}

#define GotoError()  do { _dbgStopHere(); goto OnError; } while (0)

static gctBOOL _VerifyMCLegality(VSC_MC_CODEC* pMcCodec, VSC_MC_CODEC_INST* pCodecHelperInst)
{
    gctUINT srcIdx, i, firstCstRegNo;
    gctBOOL bFirstCstRegIndexing;

    if (pCodecHelperInst->instCtrl.instType == 0x1)
    {
        if (pCodecHelperInst->baseOpcode != 0x72 &&
            pCodecHelperInst->baseOpcode != 0x32 &&
            pCodecHelperInst->baseOpcode != 0x33 &&
            pCodecHelperInst->baseOpcode != MC_AUXILIARY_OP_CODE_USC_STORE &&
            pCodecHelperInst->baseOpcode != MC_AUXILIARY_OP_CODE_USC_IMG_STORE &&
            pCodecHelperInst->baseOpcode != MC_AUXILIARY_OP_CODE_USC_IMG_STORE_3D &&
            pCodecHelperInst->baseOpcode != 0x79 &&
            pCodecHelperInst->baseOpcode != 0x34 &&
            pCodecHelperInst->baseOpcode != 0x7A &&
            pCodecHelperInst->baseOpcode != 0x35 &&
            pCodecHelperInst->baseOpcode != 0x37 &&
            pCodecHelperInst->baseOpcode != 0x38 &&
            ((pCodecHelperInst->baseOpcode != 0x45) ||
             (pCodecHelperInst->extOpcode != 0x06 &&
              pCodecHelperInst->extOpcode != 0x07 &&
              pCodecHelperInst->extOpcode != 0x08 &&
              pCodecHelperInst->extOpcode != 0x09 &&
              pCodecHelperInst->extOpcode != 0x0A &&
              pCodecHelperInst->extOpcode != 0x0B &&
              pCodecHelperInst->extOpcode != 0x12 &&
              pCodecHelperInst->extOpcode != 0x13 &&
              pCodecHelperInst->extOpcode != 0x14 &&
              pCodecHelperInst->extOpcode != 0x15)))
        {
            GotoError();
        }
    }

    if (pMcCodec->bDual16ModeEnabled)
    {
        if (pCodecHelperInst->baseOpcode == 0x14 ||
            pCodecHelperInst->baseOpcode == 0x15 ||
            pCodecHelperInst->baseOpcode == 0x1D ||
            pCodecHelperInst->baseOpcode == 0x1E ||
            pCodecHelperInst->baseOpcode == 0x1F ||
            pCodecHelperInst->baseOpcode == 0x20)
        {
            GotoError();
        }

        if (pCodecHelperInst->instCtrl.threadType == 0x0)
        {
            if (pCodecHelperInst->baseOpcode == 0x32 ||
                pCodecHelperInst->baseOpcode == 0x33 ||
                pCodecHelperInst->baseOpcode == 0x79 ||
                pCodecHelperInst->baseOpcode == 0x34 ||
                pCodecHelperInst->baseOpcode == 0x7A ||
                pCodecHelperInst->baseOpcode == 0x35 ||
                pCodecHelperInst->baseOpcode == 0x65 ||
                pCodecHelperInst->baseOpcode == 0x66 ||
                pCodecHelperInst->baseOpcode == 0x67 ||
                pCodecHelperInst->baseOpcode == 0x68 ||
                pCodecHelperInst->baseOpcode == 0x69 ||
                pCodecHelperInst->baseOpcode == 0x6A ||
                pCodecHelperInst->baseOpcode == 0x6B ||
                pCodecHelperInst->baseOpcode == 0x6C ||
                pCodecHelperInst->baseOpcode == 0x46 ||
                pCodecHelperInst->baseOpcode == 0x0A ||
                pCodecHelperInst->baseOpcode == 0x0B ||
                pCodecHelperInst->baseOpcode == 0x56 ||
                pCodecHelperInst->baseOpcode == 0x28 ||
                pCodecHelperInst->baseOpcode == 0x29 ||
                pCodecHelperInst->baseOpcode == 0x2B ||
                pCodecHelperInst->baseOpcode == 0x44 ||
                pCodecHelperInst->baseOpcode == 0x48)
            {
                GotoError();
            }

            /* Except src2 of branch and src1 of I2I/CONV, all other imm src must be V16 imm under daul-t */
            for (srcIdx = 0; srcIdx < pCodecHelperInst->srcCount; srcIdx ++)
            {
                if (pCodecHelperInst->src[srcIdx].regType == 0x7)
                {
                    if ((pCodecHelperInst->baseOpcode == 0x16 ||
                         pCodecHelperInst->baseOpcode == 0x24) &&
                         (srcIdx == pCodecHelperInst->srcCount - 1))
                    {
                        continue;
                    }

                    if (pCodecHelperInst->src[srcIdx].u.imm.immType != 0x3)
                    {
                        GotoError();
                    }
                }
            }
        }

        if (pCodecHelperInst->bDstValid)
        {
            if (pCodecHelperInst->dst.regNo >= 128)
            {
                GotoError();
            }
        }

        if (pCodecHelperInst->baseOpcode == 0x16 ||
            pCodecHelperInst->baseOpcode == 0x24)
        {
            if (pCodecHelperInst->src[pCodecHelperInst->srcCount - 1].regType ==
                0x7)
            {
                if (pCodecHelperInst->src[pCodecHelperInst->srcCount - 1].u.imm.immType !=
                    0x2)
                {
                    GotoError();
                }
            }
        }

        if (pCodecHelperInst->baseOpcode == 0x16 ||
            pCodecHelperInst->baseOpcode == 0x24 ||
            pCodecHelperInst->baseOpcode == 0x14)
        {
            if (pCodecHelperInst->instCtrl.u.loopOpType != 0x0)
            {
                GotoError();
            }
        }

        if (pCodecHelperInst->baseOpcode == 0x0A ||
            pCodecHelperInst->baseOpcode == 0x0B ||
            pCodecHelperInst->baseOpcode == 0x56)
        {
            if (pCodecHelperInst->instCtrl.threadType == 0x0)
            {
                GotoError();
            }
        }

        if (pCodecHelperInst->baseOpcode == 0x72 ||
            pCodecHelperInst->baseOpcode == 0x2C)
        {
            if (pCodecHelperInst->src[1].regType != 0x7)
            {
                GotoError();
            }
        }
    }

    for (srcIdx = 0; srcIdx < pCodecHelperInst->srcCount; srcIdx ++)
    {
        /* HW only supports absolute src modifier for float now */
        if (pCodecHelperInst->src[srcIdx].u.reg.bAbs &&
            pCodecHelperInst->src[srcIdx].regType != 0x7)
        {
            if (pCodecHelperInst->instCtrl.instType != 0x0 &&
                pCodecHelperInst->instCtrl.instType != 0x1)
            {
                GotoError();
            }
        }
    }

    /* Check constant reg file read port limitation */
    if (pCodecHelperInst->srcCount >= 2)
    {
        for (i = 0; i < pCodecHelperInst->srcCount - 1; i ++)
        {
            firstCstRegNo = NOT_ASSIGNED;
            bFirstCstRegIndexing = gcvFALSE;

            for (srcIdx = 0; srcIdx < pCodecHelperInst->srcCount; srcIdx ++)
            {
                if (pCodecHelperInst->src[srcIdx].regType == 0x2)
                {
                    if (firstCstRegNo == NOT_ASSIGNED)
                    {
                        firstCstRegNo = pCodecHelperInst->src[srcIdx].u.reg.regNo;
                        bFirstCstRegIndexing =
                            (pCodecHelperInst->src[srcIdx].u.reg.indexingAddr != 0x0);
                    }
                    else
                    {
                        if (!bFirstCstRegIndexing &&
                            pCodecHelperInst->src[srcIdx].u.reg.indexingAddr == 0x0)
                        {
                            if (firstCstRegNo != pCodecHelperInst->src[srcIdx].u.reg.regNo)
                            {
                                GotoError();
                            }
                        }
                        else if (bFirstCstRegIndexing ||
                                 pCodecHelperInst->src[srcIdx].u.reg.indexingAddr != 0x0)
                        {
                            GotoError();
                        }
                    }
                }
            }
        }
    }

    /* Image related inst, src0 must be constant reg */
     if (pCodecHelperInst->baseOpcode == 0x79 ||
         pCodecHelperInst->baseOpcode == 0x34 ||
         pCodecHelperInst->baseOpcode == 0x7A ||
         pCodecHelperInst->baseOpcode == 0x35 ||
         pCodecHelperInst->baseOpcode == 0x37 ||
         pCodecHelperInst->baseOpcode == 0x38 ||
         pCodecHelperInst->baseOpcode == MC_AUXILIARY_OP_CODE_USC_IMG_STORE_3D ||
         pCodecHelperInst->baseOpcode == MC_AUXILIARY_OP_CODE_USC_IMG_STORE)
    {
        if (pCodecHelperInst->src[0].regType != 0x2)
        {
            GotoError();
        }
    }

    /* For USC related stores, USC must be armed on chips */
    if (pCodecHelperInst->baseOpcode == MC_AUXILIARY_OP_CODE_USC_STORE ||
        pCodecHelperInst->baseOpcode == MC_AUXILIARY_OP_CODE_USC_IMG_STORE ||
        pCodecHelperInst->baseOpcode == MC_AUXILIARY_OP_CODE_USC_IMG_STORE_3D ||
        pCodecHelperInst->baseOpcode == MC_AUXILIARY_OP_CODE_USC_STORE_ATTR)
    {
        if (!(pMcCodec->pHwCfg->hwFeatureFlags.hasHalti5 && pMcCodec->pHwCfg->maxUSCSizeInKbyte > 0))
        {
            GotoError();
        }
    }

    /* For EVIS inst, src0 can not be imm */
    if (pCodecHelperInst->baseOpcode == 0x45)
    {
        if (pCodecHelperInst->src[0].regType == 0x7)
        {
            GotoError();
        }
    }

    return gcvTRUE;

OnError:
    return gcvFALSE;
}

void vscMC_BeginCodec(VSC_MC_CODEC* pMcCodec,
                      VSC_HW_CONFIG* pHwCfg,
                      gctBOOL bDual16ModeEnabled)
{
    gcmASSERT(pHwCfg->hwFeatureFlags.supportDual16 || !bDual16ModeEnabled);

    pMcCodec->pHwCfg = pHwCfg;
    pMcCodec->bInit = gcvTRUE;
    pMcCodec->bDual16ModeEnabled = bDual16ModeEnabled;
}

void vscMC_EndCodec(VSC_MC_CODEC* pMcCodec)
{
    pMcCodec->bInit = gcvFALSE;
}

gctBOOL vscMC_EncodeInstDirect(VSC_MC_CODEC*          pMcCodec,
                               gctUINT                baseOpcode,
                               gctUINT                extOpcode,
                               VSC_MC_CODEC_INST_CTRL instCtrl,
                               VSC_MC_CODEC_DST*      pDst,
                               VSC_MC_CODEC_SRC*      pSrc,
                               gctUINT                srcCount,
                               VSC_MC_RAW_INST*       pOutMCInst)
{
    VSC_MC_CODEC_INST   mcCodecHelperInst = {0};
    gctUINT             srcIdx;

    gcmASSERT(pMcCodec->bInit);

    mcCodecHelperInst.baseOpcode = baseOpcode;
    mcCodecHelperInst.extOpcode = extOpcode;
    mcCodecHelperInst.instCtrl = instCtrl;

    if (pDst)
    {
        mcCodecHelperInst.dst = *pDst;

        /* All normal (not USC) stores have no real dst, but they may use writemask in dst to
           identify which portion of memory to write */
        if (baseOpcode != 0x33 &&
            baseOpcode != 0x7A &&
            baseOpcode != 0x35 &&
            baseOpcode != 0x42)
        {
            mcCodecHelperInst.bDstValid = gcvTRUE;
        }
    }

    for (srcIdx = 0; srcIdx < srcCount; srcIdx ++)
    {
        mcCodecHelperInst.src[srcIdx] = *(pSrc + srcIdx);
    }
    mcCodecHelperInst.srcCount = srcCount;

    /* Call real encoder now */
    return vscMC_EncodeInst(pMcCodec, &mcCodecHelperInst, pOutMCInst);
}

gctBOOL vscMC_EncodeInst(VSC_MC_CODEC*                pMcCodec,
                         VSC_MC_CODEC_INST*           pInCodecHelperInst,
                         VSC_MC_RAW_INST*             pOutMCInst)
{
    VSC_MC_INST*      pMcInst = (VSC_MC_INST*)pOutMCInst;
    VSC_MC_CODEC_TYPE mcCodecType;

    gcmASSERT(pMcCodec->bInit);

    /* Initialize resultant output MC inst to 0 */
    memset(pMcInst, 0, sizeof(VSC_MC_INST));

    /* Verify whether requested MC encoding inst is legal */
    if (!_VerifyMCLegality(pMcCodec, pInCodecHelperInst))
    {
        gcmASSERT(gcvFALSE);
        return gcvFALSE;
    }

    /* Check codec type based on opcode */
    mcCodecType = _GetMcCodecType(pMcCodec,
                                  pInCodecHelperInst->baseOpcode,
                                  pInCodecHelperInst->extOpcode,
                                  pInCodecHelperInst,
                                  VSC_MC_CODEC_MODE_ENCODE);
    if (mcCodecType == VSC_MC_CODEC_TYPE_UNKNOWN)
    {
        gcmASSERT(gcvFALSE);
        return gcvFALSE;
    }

    /* Now go to different encode path for each different opcode codec-category
       to do real encoding */
    return _pfn_mc_encoder[mcCodecType](pMcCodec, mcCodecType, pInCodecHelperInst, pMcInst);
}

gctBOOL vscMC_DecodeInst(VSC_MC_CODEC*                pMcCodec,
                         VSC_MC_RAW_INST*             pInMCInst,
                         VSC_MC_CODEC_INST*           pOutCodecHelperInst)
{
    VSC_MC_INST*      pMcInst = (VSC_MC_INST*)pInMCInst;
    VSC_MC_CODEC_TYPE mcCodecType;
    gctUINT           baseOpcode, extOpcode;

    gcmASSERT(pMcCodec->bInit);

    /* Initialize resultant output MC inst to 0 */
    memset(pOutCodecHelperInst, 0, sizeof(VSC_MC_CODEC_INST));

    baseOpcode = DECODE_BASE_OPCODE(pMcInst);
    extOpcode = pMcInst->no_opnd_inst.inst.extOpcode;

    /* Check codec type based on opcode */
    mcCodecType = _GetMcCodecType(pMcCodec,
                                  baseOpcode,
                                  extOpcode,
                                  pInMCInst,
                                  VSC_MC_CODEC_MODE_DECODE);
    if (mcCodecType == VSC_MC_CODEC_TYPE_UNKNOWN)
    {
        gcmASSERT(gcvFALSE);
        return gcvFALSE;
    }

    /* Now go to different encode path for each different opcode codec-category
       to do real decoding */
    return _pfn_mc_decoder[mcCodecType](pMcCodec, mcCodecType, pMcInst, pOutCodecHelperInst);
}

