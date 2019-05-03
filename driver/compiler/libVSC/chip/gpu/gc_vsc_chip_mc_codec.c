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


#include "gc_vsc.h"

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

   5.

   6. Since we use 3-srcs ALU inst layout to codec load_attr, but load_attr has several special bits
      for following:
          a. shStageClient (bit[3-4] of word1) to indicate from which upstreaming shader stage
             load_attr loads attrs,
          b. attrLayout (bit 6 of word1) to indicate attributes layout (interleaved or linear)
      so we need cast 3-srcs ALU inst to store_attr inst layout to do codec for this bit.

   7. MSB5 of word0 (samplerSlot as usual in normal layouts) has special meaning for mul/norm_mul/
      mad/mullo/dst/dp/norm_dp. If that 5bits are zero, result of these insts will generate INF for
      (0 * INF), otherwise, 0 is generated for (0 * INF). Note that, that MSB5 are reserved in these
      insts' layout.

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
#if !gcdENDIAN_BIG
        gctUINT        baseOpcodeBit0_5      : 6; /* Together with baseOpcodeBit6 to compose base-opcode */
        gctUINT        reserved0             : 2; /* Must be zero'ed */
        gctUINT        bEndOfBB              : 1;
        gctUINT        reserved1             : 23;/* Must be zero'ed */
#else
        gctUINT        reserved1             : 23;/* Must be zero'ed */
        gctUINT        bEndOfBB              : 1;
        gctUINT        reserved0             : 2; /* Must be zero'ed */
        gctUINT        baseOpcodeBit0_5      : 6; /* Together with baseOpcodeBit6 to compose base-opcode */
#endif

        gctUINT        reserved2             : 32;/* Must be zero'ed */

#if !gcdENDIAN_BIG
        gctUINT        reserved3             : 16;/* Must be zero'ed */
        gctUINT        baseOpcodeBit6        : 1;
        gctUINT        reserved4             : 15;/* Must be zero'ed */
#else
        gctUINT        reserved4             : 15;/* Must be zero'ed */
        gctUINT        baseOpcodeBit6        : 1;
        gctUINT        reserved3             : 16;/* Must be zero'ed */
#endif

#if !gcdENDIAN_BIG
        gctUINT        reserved5             : 4; /* Must be zero'ed for non-extopcode mode, otherwise, see NOTE 4 */
        gctUINT        extOpcode             : 8;
        gctUINT        reserved6             : 20;/* Must be zero'ed for non-extopcode mode, otherwise, see NOTE 4 */
#else
        gctUINT        reserved6             : 20;/* Must be zero'ed for non-extopcode mode, otherwise, see NOTE 4 */
        gctUINT        extOpcode             : 8;
        gctUINT        reserved5             : 4; /* Must be zero'ed for non-extopcode mode, otherwise, see NOTE 4 */
#endif
    } inst;

    gctUINT            data[4];
}
VSC_MC_NO_OPERAND_INST;

/* 3-srcs alu inst case */
typedef union _VSC_MC_ALU_3_SRCS_INST
{
    struct
    {
#if !gcdENDIAN_BIG
        gctUINT        baseOpcodeBit0_5      : 6; /* Together with baseOpcodeBit6 to compose base-opcode */
        gctUINT        bBigEndian            : 1; /* need to swap big endian data to little endian if true */
        gctUINT        reserved0             : 1; /* Must be zero'ed */
        gctUINT        bEndOfBB              : 1; /* End Of Basic Block bit for non-control-clow instructions */
        gctUINT        reserved1             : 2; /* Must be zero'ed */
        gctUINT        bResultSat            : 1;
        gctUINT        bDstValid             : 1; /* Must be valid */
        gctUINT        dstRelAddr            : 3;
        gctUINT        dstRegNoBit0_6        : 7; /* Together with dstRegNoBit7 and dstRegNoBit8 to compose dstRegNo */
        gctUINT        writeMask             : 4;
        gctUINT        bInfX0ToZero          : 1;
        gctUINT        reserved2             : 4; /* Must be zero'ed */
#else
        gctUINT        reserved2             : 4; /* Must be zero'ed */
        gctUINT        bInfX0ToZero          : 1;
        gctUINT        writeMask             : 4;
        gctUINT        dstRegNoBit0_6        : 7; /* Together with dstRegNoBit7 and dstRegNoBit8 to compose dstRegNo */
        gctUINT        dstRelAddr            : 3;
        gctUINT        bDstValid             : 1; /* Must be valid */
        gctUINT        bResultSat            : 1;
        gctUINT        reserved1             : 2; /* Must be zero'ed */
        gctUINT        bEndOfBB              : 1; /* End Of Basic Block bit for non-control-clow instructions */
        gctUINT        reserved0             : 1; /* Must be zero'ed */
        gctUINT        bBigEndian            : 1; /* need to swap big endian data to little endian if true */
        gctUINT        baseOpcodeBit0_5      : 6; /* Together with baseOpcodeBit6 to compose base-opcode */
#endif

#if !gcdENDIAN_BIG
        gctUINT        roundMode             : 2;
        gctUINT        packMode              : 1;
        gctUINT        reserved3             : 4; /* For load_attr, see NOTE 6; For others, must be zero'ed */
        gctUINT        bSkipForHelperKickoff : 1;
        gctUINT        reserved4             : 2; /* Must be zero'ed for non-atomics. For atomics, see NOTE 8 */
        gctUINT        bDenorm               : 1;
        gctUINT        bSrc0Valid            : 1; /* Must be valid */
        gctUINT        src0RegNo             : 9;
        gctUINT        instTypeBit0          : 1; /* Together with instTypeBit1_2 to compose instType */
        gctUINT        src0Swizzle           : 8;
        gctUINT        bSrc0ModNeg           : 1;
        gctUINT        bSrc0ModAbs           : 1;
#else
        gctUINT        bSrc0ModAbs           : 1;
        gctUINT        bSrc0ModNeg           : 1;
        gctUINT        src0Swizzle           : 8;
        gctUINT        instTypeBit0          : 1; /* Together with instTypeBit1_2 to compose instType */
        gctUINT        src0RegNo             : 9;
        gctUINT        bSrc0Valid            : 1; /* Must be valid */
        gctUINT        bDenorm               : 1;
        gctUINT        reserved4             : 2; /* Must be zero'ed for non-atomics. For atomics, see NOTE 8 */
        gctUINT        bSkipForHelperKickoff : 1;
        gctUINT        reserved3             : 4; /* For load_attr, see NOTE 6; For others, must be zero'ed */
        gctUINT        packMode              : 1;
        gctUINT        roundMode             : 2;
#endif

#if !gcdENDIAN_BIG
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
#else
        gctUINT        instTypeBit1_2        : 2;
        gctUINT        src1RelAddr           : 3;
        gctUINT        bSrc1ModAbs           : 1;
        gctUINT        bSrc1ModNeg           : 1;
        gctUINT        src1Swizzle           : 8;
        gctUINT        baseOpcodeBit6        : 1;
        gctUINT        src1RegNo             : 9;
        gctUINT        bSrc1Valid            : 1; /* Must be valid */
        gctUINT        src0Type              : 3;
        gctUINT        src0RelAddr           : 3;
#endif

#if !gcdENDIAN_BIG
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
#else
        gctUINT        dstType               : 1;
        gctUINT        src2Type              : 3;
        gctUINT        src2RelAddr           : 3;
        gctUINT        dstRegNoBit8          : 1;
        gctUINT        bSrc2ModAbs           : 1;
        gctUINT        bSrc2ModNeg           : 1;
        gctUINT        src2Swizzle           : 8;
        gctUINT        dstRegNoBit7          : 1;
        gctUINT        src2RegNo             : 9;
        gctUINT        bSrc2Valid            : 1; /* Must be valid */
        gctUINT        src1Type              : 3;
#endif
    } inst;

    gctUINT            data[4];
}
VSC_MC_ALU_3_SRCS_INST;

/* 3-srcs alu with condition code inst case */
typedef union _VSC_MC_ALU_3_SRCS_CC_INST
{
    struct
    {
#if !gcdENDIAN_BIG
        gctUINT        baseOpcodeBit0_5      : 6; /* Together with baseOpcodeBit6 to compose base-opcode */
        gctUINT        condOpCode            : 5;
        gctUINT        bResultSat            : 1;
        gctUINT        bDstValid             : 1; /* Must be valid */
        gctUINT        dstRelAddr            : 3;
        gctUINT        dstRegNoBit0_6        : 7; /* Together with dstRegNoBit7 and dstRegNoBit8 to compose dstRegNo */
        gctUINT        writeMask             : 4;
        gctUINT        bInfX0ToZero          : 1;
        gctUINT        reserved0             : 4; /* Must be zero'ed */
#else
        gctUINT        reserved0             : 4; /* Must be zero'ed */
        gctUINT        bInfX0ToZero          : 1;
        gctUINT        writeMask             : 4;
        gctUINT        dstRegNoBit0_6        : 7; /* Together with dstRegNoBit7 and dstRegNoBit8 to compose dstRegNo */
        gctUINT        dstRelAddr            : 3;
        gctUINT        bDstValid             : 1; /* Must be valid */
        gctUINT        bResultSat            : 1;
        gctUINT        condOpCode            : 5;
        gctUINT        baseOpcodeBit0_5      : 6; /* Together with baseOpcodeBit6 to compose base-opcode */
#endif

#if !gcdENDIAN_BIG
        gctUINT        roundMode             : 2;
        gctUINT        packMode              : 1;
        gctUINT        reserved1             : 3; /* For load_attr, see NOTE 6; For others, must be zero'ed */
        gctUINT        bEndOfBB              : 1; /* End Of Basic Block bit for non-control-clow instructions */
        gctUINT        bSkipForHelperKickoff : 1;
        gctUINT        reserved2             : 2; /* Must be zero'ed for non-atomics. For atomics, see NOTE 8 */
        gctUINT        bDenorm               : 1;
        gctUINT        bSrc0Valid            : 1; /* Must be valid */
        gctUINT        src0RegNo             : 9;
        gctUINT        instTypeBit0          : 1; /* Together with instTypeBit1_2 to compose instType */
        gctUINT        src0Swizzle           : 8;
        gctUINT        bSrc0ModNeg           : 1;
        gctUINT        bSrc0ModAbs           : 1;
#else
        gctUINT        bSrc0ModAbs           : 1;
        gctUINT        bSrc0ModNeg           : 1;
        gctUINT        src0Swizzle           : 8;
        gctUINT        instTypeBit0          : 1; /* Together with instTypeBit1_2 to compose instType */
        gctUINT        src0RegNo             : 9;
        gctUINT        bSrc0Valid            : 1; /* Must be valid */
        gctUINT        bDenorm               : 1;
        gctUINT        reserved2             : 2; /* Must be zero'ed for non-atomics. For atomics, see NOTE 8 */
        gctUINT        bSkipForHelperKickoff : 1;
        gctUINT        bEndOfBB              : 1; /* End Of Basic Block bit for non-control-clow instructions */
        gctUINT        reserved1             : 3; /* For load_attr, see NOTE 6; For others, must be zero'ed */
        gctUINT        packMode              : 1;
        gctUINT        roundMode             : 2;
#endif

#if !gcdENDIAN_BIG
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
#else
        gctUINT        instTypeBit1_2        : 2;
        gctUINT        src1RelAddr           : 3;
        gctUINT        bSrc1ModAbs           : 1;
        gctUINT        bSrc1ModNeg           : 1;
        gctUINT        src1Swizzle           : 8;
        gctUINT        baseOpcodeBit6        : 1;
        gctUINT        src1RegNo             : 9;
        gctUINT        bSrc1Valid            : 1; /* Must be valid */
        gctUINT        src0Type              : 3;
        gctUINT        src0RelAddr           : 3;
#endif

#if !gcdENDIAN_BIG
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
#else
        gctUINT        dstType               : 1;
        gctUINT        src2Type              : 3;
        gctUINT        src2RelAddr           : 3;
        gctUINT        dstRegNoBit8          : 1;
        gctUINT        bSrc2ModAbs           : 1;
        gctUINT        bSrc2ModNeg           : 1;
        gctUINT        src2Swizzle           : 8;
        gctUINT        dstRegNoBit7          : 1;
        gctUINT        src2RegNo             : 9;
        gctUINT        bSrc2Valid            : 1; /* Must be valid */
        gctUINT        src1Type              : 3;
#endif
    } inst;

    gctUINT            data[4];
}
VSC_MC_ALU_3_SRCS_CC_INST;

/* 2-srcs (src0 + src1) alu inst case */
typedef union _VSC_MC_ALU_2_SRCS_SRC0_SRC1_INST
{
    struct
    {
#if !gcdENDIAN_BIG
        gctUINT        baseOpcodeBit0_5      : 6; /* Together with baseOpcodeBit6 to compose base-opcode */
        gctUINT        bBigEndian            : 1; /* need to swap big endian data to little endian if true */
        gctUINT        reserved0             : 1; /* Must be zero'ed */
        gctUINT        bEndOfBB              : 1; /* End Of Basic Block bit for non-control-clow instructions */
        gctUINT        reserved1             : 2; /* Must be zero'ed */
        gctUINT        bResultSat            : 1;
        gctUINT        bDstValid             : 1;
        gctUINT        dstRelAddr            : 3;
        gctUINT        dstRegNoBit0_6        : 7; /* Together with dstRegNoBit7 and dstRegNoBit8 to compose dstRegNo */
        gctUINT        writeMask             : 4;
        gctUINT        bInfX0ToZero          : 1;
        gctUINT        reserved2             : 4; /* Must be zero'ed */
#else
        gctUINT        reserved2             : 4; /* Must be zero'ed */
        gctUINT        bInfX0ToZero          : 1;
        gctUINT        writeMask             : 4;
        gctUINT        dstRegNoBit0_6        : 7; /* Together with dstRegNoBit7 and dstRegNoBit8 to compose dstRegNo */
        gctUINT        dstRelAddr            : 3;
        gctUINT        bDstValid             : 1;
        gctUINT        bResultSat            : 1;
        gctUINT        reserved1             : 2; /* Must be zero'ed */
        gctUINT        bEndOfBB              : 1; /* End Of Basic Block bit for non-control-clow instructions */
        gctUINT        reserved0             : 1; /* Must be zero'ed */
        gctUINT        bBigEndian            : 1; /* need to swap big endian data to little endian if true */
        gctUINT        baseOpcodeBit0_5      : 6; /* Together with baseOpcodeBit6 to compose base-opcode */
#endif

#if !gcdENDIAN_BIG
        gctUINT        roundMode             : 2;
        gctUINT        packMode              : 1;
        gctUINT        reserved3             : 8; /* For load_attr, see NOTE 6; For others, must be zero'ed */
        gctUINT        bSrc0Valid            : 1; /* Must be valid */
        gctUINT        src0RegNo             : 9;
        gctUINT        instTypeBit0          : 1; /* Together with instTypeBit1_2 to compose instType */
        gctUINT        src0Swizzle           : 8;
        gctUINT        bSrc0ModNeg           : 1;
        gctUINT        bSrc0ModAbs           : 1;
#else
        gctUINT        bSrc0ModAbs           : 1;
        gctUINT        bSrc0ModNeg           : 1;
        gctUINT        src0Swizzle           : 8;
        gctUINT        instTypeBit0          : 1; /* Together with instTypeBit1_2 to compose instType */
        gctUINT        src0RegNo             : 9;
        gctUINT        bSrc0Valid            : 1; /* Must be valid */
        gctUINT        reserved3             : 8; /* For load_attr, see NOTE 6; For others, must be zero'ed */
        gctUINT        packMode              : 1;
        gctUINT        roundMode             : 2;
#endif

#if !gcdENDIAN_BIG
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
#else
        gctUINT        instTypeBit1_2        : 2;
        gctUINT        src1RelAddr           : 3;
        gctUINT        bSrc1ModAbs           : 1;
        gctUINT        bSrc1ModNeg           : 1;
        gctUINT        src1Swizzle           : 8;
        gctUINT        baseOpcodeBit6        : 1;
        gctUINT        src1RegNo             : 9;
        gctUINT        bSrc1Valid            : 1; /* Must be valid */
        gctUINT        src0Type              : 3;
        gctUINT        src0RelAddr           : 3;
#endif

#if !gcdENDIAN_BIG
        gctUINT        src1Type              : 3;
        gctUINT        reserved4             : 1; /* Must be zero'ed for non-extopcode mode, otherwise, see NOTE 4 */
        gctUINT        extOpcode             : 8;
        gctUINT        reserved5             : 1; /* Must be zero'ed */
        gctUINT        dstRegNoBit7          : 1;
        gctUINT        reserved6             : 10;/* Must be zero'ed */
        gctUINT        dstRegNoBit8          : 1;
        gctUINT        reserved7             : 6; /* Must be zero'ed for non-extopcode mode, otherwise, see NOTE 4 */
        gctUINT        dstType               : 1;
#else
        gctUINT        dstType               : 1;
        gctUINT        reserved7             : 6; /* Must be zero'ed for non-extopcode mode, otherwise, see NOTE 4 */
        gctUINT        dstRegNoBit8          : 1;
        gctUINT        reserved6             : 10;/* Must be zero'ed */
        gctUINT        dstRegNoBit7          : 1;
        gctUINT        reserved5             : 1; /* Must be zero'ed */
        gctUINT        extOpcode             : 8;
        gctUINT        reserved4             : 1; /* Must be zero'ed for non-extopcode mode, otherwise, see NOTE 4 */
        gctUINT        src1Type              : 3;
#endif
    } inst;

    gctUINT            data[4];
}
VSC_MC_ALU_2_SRCS_SRC0_SRC1_INST;

/* 2-srcs (src0 + src1) alu inst case */
typedef union _VSC_MC_ALU_2_SRCS_SRC0_SRC1_CC_INST
{
    struct
    {
#if !gcdENDIAN_BIG
        gctUINT        baseOpcodeBit0_5      : 6; /* Together with baseOpcodeBit6 to compose base-opcode */
        gctUINT        condOpCode            : 5;
        gctUINT        bResultSat            : 1;
        gctUINT        bDstValid             : 1;
        gctUINT        dstRelAddr            : 3;
        gctUINT        dstRegNoBit0_6        : 7; /* Together with dstRegNoBit7 and dstRegNoBit8 to compose dstRegNo */
        gctUINT        writeMask             : 4;
        gctUINT        bInfX0ToZero          : 1;
        gctUINT        reserved0             : 4; /* Must be zero'ed */
#else
        gctUINT        reserved0             : 4; /* Must be zero'ed */
        gctUINT        bInfX0ToZero          : 1;
        gctUINT        writeMask             : 4;
        gctUINT        dstRegNoBit0_6        : 7; /* Together with dstRegNoBit7 and dstRegNoBit8 to compose dstRegNo */
        gctUINT        dstRelAddr            : 3;
        gctUINT        bDstValid             : 1;
        gctUINT        bResultSat            : 1;
        gctUINT        condOpCode            : 5;
        gctUINT        baseOpcodeBit0_5      : 6; /* Together with baseOpcodeBit6 to compose base-opcode */
#endif

#if !gcdENDIAN_BIG
        gctUINT        roundMode             : 2;
        gctUINT        packMode              : 1;
        gctUINT        reserved1             : 3; /* For load_attr, see NOTE 6; For others, must be zero'ed */
        gctUINT        bEndOfBB              : 1; /* End Of Basic Block bit for non-control-clow instructions */
        gctUINT        reserved2             : 4; /* Must be zero'ed */
        gctUINT        bSrc0Valid            : 1; /* Must be valid */
        gctUINT        src0RegNo             : 9;
        gctUINT        instTypeBit0          : 1; /* Together with instTypeBit1_2 to compose instType */
        gctUINT        src0Swizzle           : 8;
        gctUINT        bSrc0ModNeg           : 1;
        gctUINT        bSrc0ModAbs           : 1;
#else
        gctUINT        bSrc0ModAbs           : 1;
        gctUINT        bSrc0ModNeg           : 1;
        gctUINT        src0Swizzle           : 8;
        gctUINT        instTypeBit0          : 1; /* Together with instTypeBit1_2 to compose instType */
        gctUINT        src0RegNo             : 9;
        gctUINT        bSrc0Valid            : 1; /* Must be valid */
        gctUINT        reserved2             : 4; /* Must be zero'ed */
        gctUINT        bEndOfBB              : 1; /* End Of Basic Block bit for non-control-clow instructions */
        gctUINT        reserved1             : 3; /* For load_attr, see NOTE 6; For others, must be zero'ed */
        gctUINT        packMode              : 1;
        gctUINT        roundMode             : 2;
#endif

#if !gcdENDIAN_BIG
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
#else
        gctUINT        instTypeBit1_2        : 2;
        gctUINT        src1RelAddr           : 3;
        gctUINT        bSrc1ModAbs           : 1;
        gctUINT        bSrc1ModNeg           : 1;
        gctUINT        src1Swizzle           : 8;
        gctUINT        baseOpcodeBit6        : 1;
        gctUINT        src1RegNo             : 9;
        gctUINT        bSrc1Valid            : 1; /* Must be valid */
        gctUINT        src0Type              : 3;
        gctUINT        src0RelAddr           : 3;
#endif

#if !gcdENDIAN_BIG
        gctUINT        src1Type              : 3;
        gctUINT        reserved3             : 1; /* Must be zero'ed for non-extopcode mode, otherwise, see NOTE 4 */
        gctUINT        extOpcode             : 8;
        gctUINT        reserved4             : 1; /* Must be zero'ed */
        gctUINT        dstRegNoBit7          : 1;
        gctUINT        reserved5             : 10;/* Must be zero'ed */
        gctUINT        dstRegNoBit8          : 1;
        gctUINT        reserved6             : 6; /* Must be zero'ed for non-extopcode mode, otherwise, see NOTE 4 */
        gctUINT        dstType               : 1;
#else
        gctUINT        dstType               : 1;
        gctUINT        reserved6             : 6; /* Must be zero'ed for non-extopcode mode, otherwise, see NOTE 4 */
        gctUINT        dstRegNoBit8          : 1;
        gctUINT        reserved5             : 10;/* Must be zero'ed */
        gctUINT        dstRegNoBit7          : 1;
        gctUINT        reserved4             : 1; /* Must be zero'ed */
        gctUINT        extOpcode             : 8;
        gctUINT        reserved3             : 1; /* Must be zero'ed for non-extopcode mode, otherwise, see NOTE 4 */
        gctUINT        src1Type              : 3;
#endif
    } inst;

    gctUINT            data[4];
}
VSC_MC_ALU_2_SRCS_SRC0_SRC1_CC_INST;

/* 2-srcs (src0 + src2) alu inst case */
typedef union _VSC_MC_ALU_2_SRCS_SRC0_SRC2_INST
{
    struct
    {
#if !gcdENDIAN_BIG
        gctUINT        baseOpcodeBit0_5      : 6; /* Together with baseOpcodeBit6 to compose base-opcode */
        gctUINT        bBigEndian            : 1; /* need to swap big endian data to little endian if true */
        gctUINT        reserved0             : 1; /* Must be zero'ed */
        gctUINT        bEndOfBB              : 1; /* End Of Basic Block bit for non-control-clow instructions */
        gctUINT        reserved1             : 2; /* Must be zero'ed */
        gctUINT        bResultSat            : 1;
        gctUINT        bDstValid             : 1; /* Must be valid */
        gctUINT        dstRelAddr            : 3;
        gctUINT        dstRegNoBit0_6        : 7; /* Together with dstRegNoBit7 and dstRegNoBit8 to compose dstRegNo */
        gctUINT        writeMask             : 4;
        gctUINT        reserved2             : 5; /* Must be zero'ed */
#else
        gctUINT        reserved2             : 5; /* Must be zero'ed */
        gctUINT        writeMask             : 4;
        gctUINT        dstRegNoBit0_6        : 7; /* Together with dstRegNoBit7 and dstRegNoBit8 to compose dstRegNo */
        gctUINT        dstRelAddr            : 3;
        gctUINT        bDstValid             : 1; /* Must be valid */
        gctUINT        bResultSat            : 1;
        gctUINT        reserved1             : 2; /* Must be zero'ed */
        gctUINT        bEndOfBB              : 1; /* End Of Basic Block bit for non-control-clow instructions */
        gctUINT        reserved0             : 1; /* Must be zero'ed */
        gctUINT        bBigEndian            : 1; /* need to swap big endian data to little endian if true */
        gctUINT        baseOpcodeBit0_5      : 6; /* Together with baseOpcodeBit6 to compose base-opcode */
#endif

#if !gcdENDIAN_BIG
        gctUINT        roundMode             : 2;
        gctUINT        packMode              : 1;
        gctUINT        reserved3             : 8; /* Must be zero'ed */
        gctUINT        bSrc0Valid            : 1; /* Must be valid */
        gctUINT        src0RegNo             : 9;
        gctUINT        instTypeBit0          : 1; /* Together with instTypeBit1_2 to compose instType */
        gctUINT        src0Swizzle           : 8;
        gctUINT        bSrc0ModNeg           : 1;
        gctUINT        bSrc0ModAbs           : 1;
#else
        gctUINT        bSrc0ModAbs           : 1;
        gctUINT        bSrc0ModNeg           : 1;
        gctUINT        src0Swizzle           : 8;
        gctUINT        instTypeBit0          : 1; /* Together with instTypeBit1_2 to compose instType */
        gctUINT        src0RegNo             : 9;
        gctUINT        bSrc0Valid            : 1; /* Must be valid */
        gctUINT        reserved3             : 8; /* Must be zero'ed */
        gctUINT        packMode              : 1;
        gctUINT        roundMode             : 2;
#endif

#if !gcdENDIAN_BIG
        gctUINT        src0RelAddr           : 3;
        gctUINT        src0Type              : 3;
        gctUINT        reserved4             : 10;/* Must be zero'ed */
        gctUINT        baseOpcodeBit6        : 1;
        gctUINT        reserved5             : 13;/* Must be zero'ed */
        gctUINT        instTypeBit1_2        : 2;
#else
        gctUINT        instTypeBit1_2        : 2;
        gctUINT        reserved5             : 13;/* Must be zero'ed */
        gctUINT        baseOpcodeBit6        : 1;
        gctUINT        reserved4             : 10;/* Must be zero'ed */
        gctUINT        src0Type              : 3;
        gctUINT        src0RelAddr           : 3;
#endif

#if !gcdENDIAN_BIG
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
#else
        gctUINT        dstType               : 1;
        gctUINT        src2Type              : 3;
        gctUINT        src2RelAddr           : 3;
        gctUINT        dstRegNoBit8          : 1;
        gctUINT        bSrc2ModAbs           : 1;
        gctUINT        bSrc2ModNeg           : 1;
        gctUINT        src2Swizzle           : 8;
        gctUINT        dstRegNoBit7          : 1;
        gctUINT        src2RegNo             : 9;
        gctUINT        bSrc2Valid            : 1; /* Must be valid */
        gctUINT        reserved6             : 3; /* Must be zero'ed */
#endif
    } inst;

    gctUINT            data[4];
}
VSC_MC_ALU_2_SRCS_SRC0_SRC2_INST;

/* 2-srcs (src1 + src2) alu inst case */
typedef union _VSC_MC_ALU_2_SRCS_SRC1_SRC2_INST
{
    struct
    {
#if !gcdENDIAN_BIG
        gctUINT        baseOpcodeBit0_5      : 6; /* Together with baseOpcodeBit6 to compose base-opcode */
        gctUINT        bBigEndian            : 1; /* need to swap big endian data to little endian if true */
        gctUINT        reserved0             : 1; /* Must be zero'ed */
        gctUINT        bEndOfBB              : 1; /* End Of Basic Block bit for non-control-clow instructions */
        gctUINT        reserved1             : 2; /* Must be zero'ed */
        gctUINT        bResultSat            : 1;
        gctUINT        bDstValid             : 1; /* Must be valid */
        gctUINT        dstRelAddr            : 3;
        gctUINT        dstRegNoBit0_6        : 7; /* Together with dstRegNoBit7 and dstRegNoBit8 to compose dstRegNo */
        gctUINT        writeMask             : 4;
        gctUINT        reserved2             : 5; /* Must be zero'ed */
#else
        gctUINT        reserved2             : 5; /* Must be zero'ed */
        gctUINT        writeMask             : 4;
        gctUINT        dstRegNoBit0_6        : 7; /* Together with dstRegNoBit7 and dstRegNoBit8 to compose dstRegNo */
        gctUINT        dstRelAddr            : 3;
        gctUINT        bDstValid             : 1; /* Must be valid */
        gctUINT        bResultSat            : 1;
        gctUINT        reserved1             : 2; /* Must be zero'ed */
        gctUINT        bEndOfBB              : 1; /* End Of Basic Block bit for non-control-clow instructions */
        gctUINT        reserved0             : 1; /* Must be zero'ed */
        gctUINT        bBigEndian            : 1; /* need to swap big endian data to little endian if true */
        gctUINT        baseOpcodeBit0_5      : 6; /* Together with baseOpcodeBit6 to compose base-opcode */
#endif

#if !gcdENDIAN_BIG
        gctUINT        roundMode             : 2;
        gctUINT        packMode              : 1;
        gctUINT        reserved3             : 18;/* Must be zero'ed */
        gctUINT        instTypeBit0          : 1; /* Together with instTypeBit1_2 to compose instType */
        gctUINT        reserved4             : 10;/* Must be zero'ed */
#else
        gctUINT        reserved4             : 10;/* Must be zero'ed */
        gctUINT        instTypeBit0          : 1; /* Together with instTypeBit1_2 to compose instType */
        gctUINT        reserved3             : 18;/* Must be zero'ed */
        gctUINT        packMode              : 1;
        gctUINT        roundMode             : 2;
#endif

#if !gcdENDIAN_BIG
        gctUINT        reserved5             : 6; /* Must be zero'ed */
        gctUINT        bSrc1Valid            : 1; /* Must be valid */
        gctUINT        src1RegNo             : 9;
        gctUINT        baseOpcodeBit6        : 1;
        gctUINT        src1Swizzle           : 8;
        gctUINT        bSrc1ModNeg           : 1;
        gctUINT        bSrc1ModAbs           : 1;
        gctUINT        src1RelAddr           : 3;
        gctUINT        instTypeBit1_2        : 2;
#else
        gctUINT        instTypeBit1_2        : 2;
        gctUINT        src1RelAddr           : 3;
        gctUINT        bSrc1ModAbs           : 1;
        gctUINT        bSrc1ModNeg           : 1;
        gctUINT        src1Swizzle           : 8;
        gctUINT        baseOpcodeBit6        : 1;
        gctUINT        src1RegNo             : 9;
        gctUINT        bSrc1Valid            : 1; /* Must be valid */
        gctUINT        reserved5             : 6; /* Must be zero'ed */
#endif

#if !gcdENDIAN_BIG
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
#else
        gctUINT        dstType               : 1;
        gctUINT        src2Type              : 3;
        gctUINT        src2RelAddr           : 3;
        gctUINT        dstRegNoBit8          : 1;
        gctUINT        bSrc2ModAbs           : 1;
        gctUINT        bSrc2ModNeg           : 1;
        gctUINT        src2Swizzle           : 8;
        gctUINT        dstRegNoBit7          : 1;
        gctUINT        src2RegNo             : 9;
        gctUINT        bSrc2Valid            : 1; /* Must be valid */
        gctUINT        src1Type              : 3;
#endif
    } inst;

    gctUINT            data[4];
}
VSC_MC_ALU_2_SRCS_SRC1_SRC2_INST;

/* 1-src (src0) alu inst case */
typedef union _VSC_MC_ALU_1_SRC_SRC0_INST
{
    struct
    {
#if !gcdENDIAN_BIG
        gctUINT        baseOpcodeBit0_5      : 6; /* Together with baseOpcodeBit6 to compose base-opcode */
        gctUINT        bBigEndian            : 1; /* need to swap big endian data to little endian if true */
        gctUINT        reserved0             : 1; /* Must be zero'ed */
        gctUINT        bEndOfBB              : 1; /* End Of Basic Block bit for non-control-clow instructions */
        gctUINT        reserved1             : 2; /* Must be zero'ed */
        gctUINT        bResultSat            : 1;
        gctUINT        bDstValid             : 1; /* Must be valid */
        gctUINT        dstRelAddr            : 3;
        gctUINT        dstRegNoBit0_6        : 7; /* Together with dstRegNoBit7 and dstRegNoBit8 to compose dstRegNo */
        gctUINT        writeMask             : 4;
        gctUINT        bInfX0ToZero          : 1;
        gctUINT        reserved2             : 4; /* Must be zero'ed */
#else
        gctUINT        reserved2             : 4; /* Must be zero'ed */
        gctUINT        bInfX0ToZero          : 1;
        gctUINT        writeMask             : 4;
        gctUINT        dstRegNoBit0_6        : 7; /* Together with dstRegNoBit7 and dstRegNoBit8 to compose dstRegNo */
        gctUINT        dstRelAddr            : 3;
        gctUINT        bDstValid             : 1; /* Must be valid */
        gctUINT        bResultSat            : 1;
        gctUINT        reserved1             : 2; /* Must be zero'ed */
        gctUINT        bEndOfBB              : 1; /* End Of Basic Block bit for non-control-clow instructions */
        gctUINT        reserved0             : 1; /* Must be zero'ed */
        gctUINT        bBigEndian            : 1; /* need to swap big endian data to little endian if true */
        gctUINT        baseOpcodeBit0_5      : 6; /* Together with baseOpcodeBit6 to compose base-opcode */
#endif

#if !gcdENDIAN_BIG
        gctUINT        roundMode             : 2;
        gctUINT        packMode              : 1;
        gctUINT        reserved3             : 8; /* Must be zero'ed */
        gctUINT        bSrc0Valid            : 1; /* Must be valid */
        gctUINT        src0RegNo             : 9;
        gctUINT        instTypeBit0          : 1; /* Together with instTypeBit1_2 to compose instType */
        gctUINT        src0Swizzle           : 8;
        gctUINT        bSrc0ModNeg           : 1;
        gctUINT        bSrc0ModAbs           : 1;
#else
        gctUINT        bSrc0ModAbs           : 1;
        gctUINT        bSrc0ModNeg           : 1;
        gctUINT        src0Swizzle           : 8;
        gctUINT        instTypeBit0          : 1; /* Together with instTypeBit1_2 to compose instType */
        gctUINT        src0RegNo             : 9;
        gctUINT        bSrc0Valid            : 1; /* Must be valid */
        gctUINT        reserved3             : 8; /* Must be zero'ed */
        gctUINT        packMode              : 1;
        gctUINT        roundMode             : 2;
#endif

#if !gcdENDIAN_BIG
        gctUINT        src0RelAddr           : 3;
        gctUINT        src0Type              : 3;
        gctUINT        reserved4             : 10;/* Must be zero'ed */
        gctUINT        baseOpcodeBit6        : 1;
        gctUINT        reserved5             : 13;/* Must be zero'ed */
        gctUINT        instTypeBit1_2        : 2;
#else
        gctUINT        instTypeBit1_2        : 2;
        gctUINT        reserved5             : 13;/* Must be zero'ed */
        gctUINT        baseOpcodeBit6        : 1;
        gctUINT        reserved4             : 10;/* Must be zero'ed */
        gctUINT        src0Type              : 3;
        gctUINT        src0RelAddr           : 3;
#endif

#if !gcdENDIAN_BIG
        gctUINT        reserved6             : 4; /* Must be zero'ed for non-extopcode mode, otherwise, see NOTE 4 */
        gctUINT        extOpcode             : 8;
        gctUINT        reserved7             : 1; /* Must be zero'ed */
        gctUINT        dstRegNoBit7          : 1;
        gctUINT        reserved8             : 10;/* Must be zero'ed */
        gctUINT        dstRegNoBit8          : 1;
        gctUINT        reserved9             : 6; /* Must be zero'ed for non-extopcode mode, otherwise, see NOTE 4 */
        gctUINT        dstType               : 1;
#else
        gctUINT        dstType               : 1;
        gctUINT        reserved9             : 6; /* Must be zero'ed for non-extopcode mode, otherwise, see NOTE 4 */
        gctUINT        dstRegNoBit8          : 1;
        gctUINT        reserved8             : 10;/* Must be zero'ed */
        gctUINT        dstRegNoBit7          : 1;
        gctUINT        reserved7             : 1; /* Must be zero'ed */
        gctUINT        extOpcode             : 8;
        gctUINT        reserved6             : 4; /* Must be zero'ed for non-extopcode mode, otherwise, see NOTE 4 */
#endif
    } inst;

    gctUINT            data[4];
}
VSC_MC_ALU_1_SRC_SRC0_INST;

/* 1-src (src1) alu inst case */
typedef union _VSC_MC_ALU_1_SRC_SRC1_INST
{
    struct
    {
#if !gcdENDIAN_BIG
        gctUINT        baseOpcodeBit0_5      : 6; /* Together with baseOpcodeBit6 to compose base-opcode */
        gctUINT        bBigEndian            : 1; /* need to swap big endian data to little endian if true */
        gctUINT        reserved0             : 1; /* Must be zero'ed */
        gctUINT        bEndOfBB              : 1; /* End Of Basic Block bit for non-control-clow instructions */
        gctUINT        reserved1             : 2; /* Must be zero'ed */
        gctUINT        bResultSat            : 1;
        gctUINT        bDstValid             : 1; /* Must be valid */
        gctUINT        dstRelAddr            : 3;
        gctUINT        dstRegNoBit0_6        : 7; /* Together with dstRegNoBit7 and dstRegNoBit8 to compose dstRegNo */
        gctUINT        writeMask             : 4;
        gctUINT        reserved2             : 5; /* Must be zero'ed */
#else
        gctUINT        reserved2             : 5; /* Must be zero'ed */
        gctUINT        writeMask             : 4;
        gctUINT        dstRegNoBit0_6        : 7; /* Together with dstRegNoBit7 and dstRegNoBit8 to compose dstRegNo */
        gctUINT        dstRelAddr            : 3;
        gctUINT        bDstValid             : 1; /* Must be valid */
        gctUINT        bResultSat            : 1;
        gctUINT        reserved1             : 2; /* Must be zero'ed */
        gctUINT        bEndOfBB              : 1; /* End Of Basic Block bit for non-control-clow instructions */
        gctUINT        reserved0             : 1; /* Must be zero'ed */
        gctUINT        bBigEndian            : 1; /* need to swap big endian data to little endian if true */
        gctUINT        baseOpcodeBit0_5      : 6; /* Together with baseOpcodeBit6 to compose base-opcode */
#endif

#if !gcdENDIAN_BIG
        gctUINT        roundMode             : 2;
        gctUINT        packMode              : 1;
        gctUINT        reserved3             : 18;/* Must be zero'ed */
        gctUINT        instTypeBit0          : 1; /* Together with instTypeBit1_2 to compose instType */
        gctUINT        reserved4             : 10;/* Must be zero'ed */
#else
        gctUINT        reserved4             : 10;/* Must be zero'ed */
        gctUINT        instTypeBit0          : 1; /* Together with instTypeBit1_2 to compose instType */
        gctUINT        reserved3             : 18;/* Must be zero'ed */
        gctUINT        packMode              : 1;
        gctUINT        roundMode             : 2;
#endif

#if !gcdENDIAN_BIG
        gctUINT        reserved5             : 6; /* Must be zero'ed */
        gctUINT        bSrc1Valid            : 1; /* Must be valid */
        gctUINT        src1RegNo             : 9;
        gctUINT        baseOpcodeBit6        : 1;
        gctUINT        src1Swizzle           : 8;
        gctUINT        bSrc1ModNeg           : 1;
        gctUINT        bSrc1ModAbs           : 1;
        gctUINT        src1RelAddr           : 3;
        gctUINT        instTypeBit1_2        : 2;
#else
        gctUINT        instTypeBit1_2        : 2;
        gctUINT        src1RelAddr           : 3;
        gctUINT        bSrc1ModAbs           : 1;
        gctUINT        bSrc1ModNeg           : 1;
        gctUINT        src1Swizzle           : 8;
        gctUINT        baseOpcodeBit6        : 1;
        gctUINT        src1RegNo             : 9;
        gctUINT        bSrc1Valid            : 1; /* Must be valid */
        gctUINT        reserved5             : 6; /* Must be zero'ed */
#endif

#if !gcdENDIAN_BIG
        gctUINT        src1Type              : 3;
        gctUINT        reserved6             : 1; /* Must be zero'ed for non-extopcode mode, otherwise, see NOTE 4 */
        gctUINT        extOpcode             : 8;
        gctUINT        reserved7             : 1; /* Must be zero'ed */
        gctUINT        dstRegNoBit7          : 1;
        gctUINT        reserved8             : 10;/* Must be zero'ed */
        gctUINT        dstRegNoBit8          : 1;
        gctUINT        reserved9             : 6; /* Must be zero'ed for non-extopcode mode, otherwise, see NOTE 4 */
        gctUINT        dstType               : 1;
#else
        gctUINT        dstType               : 1;
        gctUINT        reserved9             : 6; /* Must be zero'ed for non-extopcode mode, otherwise, see NOTE 4 */
        gctUINT        dstRegNoBit8          : 1;
        gctUINT        reserved8             : 10;/* Must be zero'ed */
        gctUINT        dstRegNoBit7          : 1;
        gctUINT        reserved7             : 1; /* Must be zero'ed */
        gctUINT        extOpcode             : 8;
        gctUINT        reserved6             : 1; /* Must be zero'ed for non-extopcode mode, otherwise, see NOTE 4 */
        gctUINT        src1Type              : 3;
#endif
    } inst;

    gctUINT            data[4];
}
VSC_MC_ALU_1_SRC_SRC1_INST;

/* 1-src (src2) alu inst case */
typedef union _VSC_MC_ALU_1_SRC_SRC2_INST
{
    struct
    {
#if !gcdENDIAN_BIG
        gctUINT        baseOpcodeBit0_5      : 6; /* Together with baseOpcodeBit6 to compose base-opcode */
        gctUINT        bBigEndian            : 1; /* need to swap big endian data to little endian if true */
        gctUINT        reserved0             : 1; /* Must be zero'ed */
        gctUINT        bEndOfBB              : 1; /* End Of Basic Block bit for non-control-clow instructions */
        gctUINT        reserved1             : 2; /* Must be zero'ed */
        gctUINT        bResultSat            : 1;
        gctUINT        bDstValid             : 1; /* Must be valid */
        gctUINT        dstRelAddr            : 3;
        gctUINT        dstRegNoBit0_6        : 7; /* Together with dstRegNoBit7 and dstRegNoBit8 to compose dstRegNo */
        gctUINT        writeMask             : 4;
        gctUINT        reserved2             : 5; /* Must be zero'ed */
#else
        gctUINT        reserved2             : 5; /* Must be zero'ed */
        gctUINT        writeMask             : 4;
        gctUINT        dstRegNoBit0_6        : 7; /* Together with dstRegNoBit7 and dstRegNoBit8 to compose dstRegNo */
        gctUINT        dstRelAddr            : 3;
        gctUINT        bDstValid             : 1; /* Must be valid */
        gctUINT        bResultSat            : 1;
        gctUINT        reserved1             : 2; /* Must be zero'ed */
        gctUINT        bEndOfBB              : 1; /* End Of Basic Block bit for non-control-clow instructions */
        gctUINT        reserved0             : 1; /* Must be zero'ed */
        gctUINT        bBigEndian            : 1; /* need to swap big endian data to little endian if true */
        gctUINT        baseOpcodeBit0_5      : 6; /* Together with baseOpcodeBit6 to compose base-opcode */
#endif

#if !gcdENDIAN_BIG
        gctUINT        roundMode             : 2;
        gctUINT        packMode              : 1;
        gctUINT        reserved3             : 18;/* Must be zero'ed */
        gctUINT        instTypeBit0          : 1; /* Together with instTypeBit1_2 to compose instType */
        gctUINT        reserved4             : 10;/* Must be zero'ed */
#else
        gctUINT        reserved4             : 10;/* Must be zero'ed */
        gctUINT        instTypeBit0          : 1; /* Together with instTypeBit1_2 to compose instType */
        gctUINT        reserved3             : 18;/* Must be zero'ed */
        gctUINT        packMode              : 1;
        gctUINT        roundMode             : 2;
#endif

#if !gcdENDIAN_BIG
        gctUINT        reserved5             : 16;/* Must be zero'ed */
        gctUINT        baseOpcodeBit6        : 1;
        gctUINT        reserved6             : 13;/* Must be zero'ed */
        gctUINT        instTypeBit1_2        : 2;
#else
        gctUINT        instTypeBit1_2        : 2;
        gctUINT        reserved6             : 13;/* Must be zero'ed */
        gctUINT        baseOpcodeBit6        : 1;
        gctUINT        reserved5             : 16;/* Must be zero'ed */
#endif

#if !gcdENDIAN_BIG
        gctUINT        reserved7             : 3; /* Must be zero'ed */
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
#else
        gctUINT        dstType               : 1;
        gctUINT        src2Type              : 3;
        gctUINT        src2RelAddr           : 3;
        gctUINT        dstRegNoBit8          : 1;
        gctUINT        bSrc2ModAbs           : 1;
        gctUINT        bSrc2ModNeg           : 1;
        gctUINT        src2Swizzle           : 8;
        gctUINT        dstRegNoBit7          : 1;
        gctUINT        src2RegNo             : 9;
        gctUINT        bSrc2Valid            : 1; /* Must be valid */
        gctUINT        reserved7             : 3; /* Must be zero'ed */
#endif
    } inst;

    gctUINT            data[4];
}
VSC_MC_ALU_1_SRC_SRC2_INST;

/* Pack inst 0x71 */
typedef union _VSC_MC_PACK_INST
{
    struct
    {
#if !gcdENDIAN_BIG
        gctUINT        baseOpcodeBit0_5      : 6; /* Together with baseOpcodeBit6 to compose base-opcode */
        gctUINT        reserved0             : 2; /* Must be zero'ed */
        gctUINT        bEndOfBB              : 1; /* End Of Basic Block bit for non-control-clow instructions */
        gctUINT        reserved1             : 2; /* Must be zero'ed */
        gctUINT        bResultSat            : 1;
        gctUINT        bDstValid             : 1; /* Must be valid */
        gctUINT        dstRelAddr            : 3;
        gctUINT        dstRegNoBit0_6        : 7; /* Together with dstRegNoBit7 and dstRegNoBit8 to compose dstRegNo */
        gctUINT        writeMask             : 4;
        gctUINT        reserved2             : 5; /* Must be zero'ed */
#else
        gctUINT        reserved2             : 5; /* Must be zero'ed */
        gctUINT        writeMask             : 4;
        gctUINT        dstRegNoBit0_6        : 7; /* Together with dstRegNoBit7 and dstRegNoBit8 to compose dstRegNo */
        gctUINT        dstRelAddr            : 3;
        gctUINT        bDstValid             : 1; /* Must be valid */
        gctUINT        bResultSat            : 1;
        gctUINT        reserved1             : 2; /* Must be zero'ed */
        gctUINT        bEndOfBB              : 1; /* End Of Basic Block bit for non-control-clow instructions */
        gctUINT        reserved0             : 2; /* Must be zero'ed */
        gctUINT        baseOpcodeBit0_5      : 6; /* Together with baseOpcodeBit6 to compose base-opcode */
#endif

#if !gcdENDIAN_BIG
        gctUINT        reserved3             : 3; /* Must be zero'ed */
        gctUINT        srcSelect             : 8;
        gctUINT        bSrc0Valid            : 1; /* Must be valid */
        gctUINT        src0RegNo             : 9;
        gctUINT        instTypeBit0          : 1; /* Together with instTypeBit1_2 to compose instType */
        gctUINT        src0Swizzle           : 8;
        gctUINT        bSrc0ModNeg           : 1;
        gctUINT        bSrc0ModAbs           : 1;
#else
        gctUINT        bSrc0ModAbs           : 1;
        gctUINT        bSrc0ModNeg           : 1;
        gctUINT        src0Swizzle           : 8;
        gctUINT        instTypeBit0          : 1; /* Together with instTypeBit1_2 to compose instType */
        gctUINT        src0RegNo             : 9;
        gctUINT        bSrc0Valid            : 1; /* Must be valid */
        gctUINT        srcSelect             : 8;
        gctUINT        reserved3             : 3; /* Must be zero'ed */
#endif

#if !gcdENDIAN_BIG
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
#else
        gctUINT        instTypeBit1_2        : 2;
        gctUINT        src1RelAddr           : 3;
        gctUINT        bSrc1ModAbs           : 1;
        gctUINT        bSrc1ModNeg           : 1;
        gctUINT        src1Swizzle           : 8;
        gctUINT        baseOpcodeBit6        : 1;
        gctUINT        src1RegNo             : 9;
        gctUINT        bSrc1Valid            : 1; /* Must be valid */
        gctUINT        src0Type              : 3;
        gctUINT        src0RelAddr           : 3;
#endif

#if !gcdENDIAN_BIG
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
#else
        gctUINT        dstType               : 1;
        gctUINT        src2Type              : 3;
        gctUINT        src2RelAddr           : 3;
        gctUINT        dstRegNoBit8          : 1;
        gctUINT        bSrc2ModAbs           : 1;
        gctUINT        bSrc2ModNeg           : 1;
        gctUINT        src2Swizzle           : 8;
        gctUINT        dstRegNoBit7          : 1;
        gctUINT        src2RegNo             : 9;
        gctUINT        bSrc2Valid            : 1; /* Must be valid */
        gctUINT        src1Type              : 3;
#endif
    } inst;

    gctUINT            data[4];
}
VSC_MC_PACK_INST;

/* Sample inst case */
typedef union _VSC_MC_SAMPLE_INST
{
    struct
    {
#if !gcdENDIAN_BIG
        gctUINT        baseOpcodeBit0_5      : 6; /* Together with baseOpcodeBit6 to compose base-opcode */
        gctUINT        reserved0             : 2; /* Must be zero'ed */
        gctUINT        bEndOfBB              : 1; /* End Of Basic Block bit for non-control-clow instructions */
        gctUINT        reserved1             : 2; /* Must be zero'ed */
        gctUINT        bResultSat            : 1;
        gctUINT        bDstValid             : 1; /* Must be valid */
        gctUINT        dstRelAddr            : 3;
        gctUINT        dstRegNoBit0_6        : 7; /* Together with dstRegNoBit7 and dstRegNoBit8 to compose dstRegNo */
        gctUINT        writeMask             : 4;
        gctUINT        samplerSlot           : 5;
#else
        gctUINT        samplerSlot           : 5;
        gctUINT        writeMask             : 4;
        gctUINT        dstRegNoBit0_6        : 7; /* Together with dstRegNoBit7 and dstRegNoBit8 to compose dstRegNo */
        gctUINT        dstRelAddr            : 3;
        gctUINT        bDstValid             : 1; /* Must be valid */
        gctUINT        bResultSat            : 1;
        gctUINT        reserved1             : 2; /* Must be zero'ed */
        gctUINT        bEndOfBB              : 1; /* End Of Basic Block bit for non-control-clow instructions */
        gctUINT        reserved0             : 2; /* Must be zero'ed */
        gctUINT        baseOpcodeBit0_5      : 6; /* Together with baseOpcodeBit6 to compose base-opcode */
#endif

#if !gcdENDIAN_BIG
        gctUINT        samplerRelAddr        : 3;
        gctUINT        samplerSwizzle        : 8;
        gctUINT        bSrc0Valid            : 1; /* Must be valid */
        gctUINT        src0RegNo             : 9;
        gctUINT        instTypeBit0          : 1; /* Together with instTypeBit1_2 to compose instType */
        gctUINT        src0Swizzle           : 8;
        gctUINT        bSrc0ModNeg           : 1;
        gctUINT        bSrc0ModAbs           : 1;
#else
        gctUINT        bSrc0ModAbs           : 1;
        gctUINT        bSrc0ModNeg           : 1;
        gctUINT        src0Swizzle           : 8;
        gctUINT        instTypeBit0          : 1; /* Together with instTypeBit1_2 to compose instType */
        gctUINT        src0RegNo             : 9;
        gctUINT        bSrc0Valid            : 1; /* Must be valid */
        gctUINT        samplerSwizzle        : 8;
        gctUINT        samplerRelAddr        : 3;
#endif

#if !gcdENDIAN_BIG
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
#else
        gctUINT        instTypeBit1_2        : 2;
        gctUINT        src1RelAddr           : 3;
        gctUINT        bSrc1ModAbs           : 1;
        gctUINT        bSrc1ModNeg           : 1;
        gctUINT        src1Swizzle           : 8;
        gctUINT        baseOpcodeBit6        : 1;
        gctUINT        src1RegNo             : 9;
        gctUINT        bSrc1Valid            : 1;
        gctUINT        src0Type              : 3;
        gctUINT        src0RelAddr           : 3;
#endif

#if !gcdENDIAN_BIG
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
#else
        gctUINT        dstType               : 1;
        gctUINT        src2Type              : 3;
        gctUINT        src2RelAddr           : 3;
        gctUINT        dstRegNoBit8          : 1;
        gctUINT        bSrc2ModAbs           : 1;
        gctUINT        bSrc2ModNeg           : 1;
        gctUINT        src2Swizzle           : 8;
        gctUINT        dstRegNoBit7          : 1;
        gctUINT        src2RegNo             : 9;
        gctUINT        bSrc2Valid            : 1;
        gctUINT        src1Type              : 3;
#endif
    } inst;

    gctUINT            data[4];
}
VSC_MC_SAMPLE_INST;

/* Extended sample inst case */
typedef union _VSC_MC_SAMPLE_EXT_INST
{
    struct
    {
#if !gcdENDIAN_BIG
        gctUINT        baseOpcodeBit0_5      : 6; /* Together with baseOpcodeBit6 to compose base-opcode */
        gctUINT        reserved0             : 2; /* Must be zero'ed */
        gctUINT        bEndOfBB              : 1; /* End Of Basic Block bit for non-control-clow instructions */
        gctUINT        reserved1             : 2; /* Must be zero'ed */
        gctUINT        bResultSat            : 1;
        gctUINT        bDstValid             : 1; /* Must be valid */
        gctUINT        dstRelAddr            : 3;
        gctUINT        dstRegNoBit0_6        : 7; /* Together with dstRegNoBit7 and dstRegNoBit8 to compose dstRegNo */
        gctUINT        writeMask             : 4;
        gctUINT        samplerSlot           : 5;
#else
        gctUINT        samplerSlot           : 5;
        gctUINT        writeMask             : 4;
        gctUINT        dstRegNoBit0_6        : 7; /* Together with dstRegNoBit7 and dstRegNoBit8 to compose dstRegNo */
        gctUINT        dstRelAddr            : 3;
        gctUINT        bDstValid             : 1; /* Must be valid */
        gctUINT        bResultSat            : 1;
        gctUINT        reserved1             : 2; /* Must be zero'ed */
        gctUINT        bEndOfBB              : 1; /* End Of Basic Block bit for non-control-clow instructions */
        gctUINT        reserved0             : 2; /* Must be zero'ed */
        gctUINT        baseOpcodeBit0_5      : 6; /* Together with baseOpcodeBit6 to compose base-opcode */
#endif

#if !gcdENDIAN_BIG
        gctUINT        samplerRelAddr        : 3;
        gctUINT        samplerSwizzle        : 8;
        gctUINT        bSrc0Valid            : 1; /* Must be valid */
        gctUINT        src0RegNo             : 9;
        gctUINT        instTypeBit0          : 1; /* Together with instTypeBit1_2 to compose instType */
        gctUINT        src0Swizzle           : 8;
        gctUINT        bSrc0ModNeg           : 1;
        gctUINT        bSrc0ModAbs           : 1;
#else
        gctUINT        bSrc0ModAbs           : 1;
        gctUINT        bSrc0ModNeg           : 1;
        gctUINT        src0Swizzle           : 8;
        gctUINT        instTypeBit0          : 1; /* Together with instTypeBit1_2 to compose instType */
        gctUINT        src0RegNo             : 9;
        gctUINT        bSrc0Valid            : 1; /* Must be valid */
        gctUINT        samplerSwizzle        : 8;
        gctUINT        samplerRelAddr        : 3;
#endif

#if !gcdENDIAN_BIG
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
#else
        gctUINT        instTypeBit1_2        : 2;
        gctUINT        src1RelAddr           : 3;
        gctUINT        bSrc1ModAbs           : 1;
        gctUINT        bSrc1ModNeg           : 1;
        gctUINT        src1Swizzle           : 8;
        gctUINT        baseOpcodeBit6        : 1;
        gctUINT        src1RegNo             : 9;
        gctUINT        bSrc1Valid            : 1;
        gctUINT        src0Type              : 3;
        gctUINT        src0RelAddr           : 3;
#endif

#if !gcdENDIAN_BIG
        gctUINT        src1Type              : 3;
        gctUINT        reserved2             : 1; /* Must be zero'ed for non-extopcode mode, otherwise, see NOTE 4 */
        gctUINT        extOpcode             : 8;
        gctUINT        reserved3             : 1; /* Must be zero'ed */
        gctUINT        dstRegNoBit7          : 1;
        gctUINT        reserved4             : 10;/* Must be zero'ed */
        gctUINT        dstRegNoBit8          : 1;
        gctUINT        reserved5             : 6; /* Must be zero'ed for non-extopcode mode, otherwise, see NOTE 4 */
        gctUINT        dstType               : 1;
#else
        gctUINT        dstType               : 1;
        gctUINT        reserved5             : 6; /* Must be zero'ed for non-extopcode mode, otherwise, see NOTE 4 */
        gctUINT        dstRegNoBit8          : 1;
        gctUINT        reserved4             : 10;/* Must be zero'ed */
        gctUINT        dstRegNoBit7          : 1;
        gctUINT        reserved3             : 1; /* Must be zero'ed */
        gctUINT        extOpcode             : 8;
        gctUINT        reserved2             : 1; /* Must be zero'ed for non-extopcode mode, otherwise, see NOTE 4 */
        gctUINT        src1Type              : 3;
#endif
    } inst;

    gctUINT            data[4];
}
VSC_MC_SAMPLE_EXT_INST;

/* Load inst case */
typedef union _VSC_MC_LD_INST
{
    struct
    {
#if !gcdENDIAN_BIG
        gctUINT        baseOpcodeBit0_5      : 6; /* Together with baseOpcodeBit6 to compose base-opcode */
        gctUINT        bBigEndian            : 1; /* need to swap big endian data to little endian if true */
        gctUINT        bUnallocate           : 1; /* USC Unallocate Bit for global memory Load/Store  */
        gctUINT        bEndOfBB              : 1; /* End Of Basic Block bit for non-control-clow instructions */
        gctUINT        reserved0             : 2; /* Must be zero'ed */
        gctUINT        bResultSat            : 1;
        gctUINT        bDstValid             : 1; /* Must be valid */
        gctUINT        dstRelAddr            : 3;
        gctUINT        dstRegNoBit0_6        : 7; /* Together with dstRegNoBit7 and dstRegNoBit8 to compose dstRegNo */
        gctUINT        writeMask             : 4;
        gctUINT        reserved1             : 5; /* Must be zero'ed */
#else
        gctUINT        reserved1             : 5; /* Must be zero'ed */
        gctUINT        writeMask             : 4;
        gctUINT        dstRegNoBit0_6        : 7; /* Together with dstRegNoBit7 and dstRegNoBit8 to compose dstRegNo */
        gctUINT        dstRelAddr            : 3;
        gctUINT        bDstValid             : 1; /* Must be valid */
        gctUINT        bResultSat            : 1;
        gctUINT        reserved0             : 2; /* Must be zero'ed */
        gctUINT        bEndOfBB              : 1; /* End Of Basic Block bit for non-control-clow instructions */
        gctUINT        bUnallocate           : 1; /* USC Unallocate Bit for global memory Load/Store  */
        gctUINT        bBigEndian            : 1; /* need to swap big endian data to little endian if true */
        gctUINT        baseOpcodeBit0_5      : 6; /* Together with baseOpcodeBit6 to compose base-opcode */
#endif

#if !gcdENDIAN_BIG
        gctUINT        reserved2             : 2; /* Must be zero'ed */
        gctUINT        packMode              : 1;
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
#else
        gctUINT        bSrc0ModAbs           : 1;
        gctUINT        bSrc0ModNeg           : 1;
        gctUINT        src0Swizzle           : 8;
        gctUINT        instTypeBit0          : 1; /* Together with instTypeBit1_2 and instTypeBit3 to compose instType */
        gctUINT        src0RegNo             : 9;
        gctUINT        bSrc0Valid            : 1; /* Must be valid */
        gctUINT        bDenorm               : 1;
        gctUINT        instTypeBit3          : 1;
        gctUINT        bAccessLocalStorage   : 1;
        gctUINT        bSkipForHelperKickoff : 1;
        gctUINT        bOffsetX3             : 1;
        gctUINT        offsetLeftShift       : 3; /* Act after X3, do (<< offsetLeftShift) */
        gctUINT        packMode              : 1;
        gctUINT        reserved2             : 2; /* Must be zero'ed */
#endif

#if !gcdENDIAN_BIG
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
#else
        gctUINT        instTypeBit1_2        : 2;
        gctUINT        src1RelAddr           : 3;
        gctUINT        bSrc1ModAbs           : 1;
        gctUINT        bSrc1ModNeg           : 1;
        gctUINT        src1Swizzle           : 8;
        gctUINT        baseOpcodeBit6        : 1;
        gctUINT        src1RegNo             : 9;
        gctUINT        bSrc1Valid            : 1; /* Must be valid */
        gctUINT        src0Type              : 3;
        gctUINT        src0RelAddr           : 3;
#endif

#if !gcdENDIAN_BIG
        gctUINT        src1Type              : 3;
        gctUINT        reserved3             : 1; /* Must be zero'ed for non-extopcode mode, otherwise, see NOTE 4 */
        gctUINT        extOpcode             : 8;
        gctUINT        reserved4             : 1; /* Must be zero'ed */
        gctUINT        dstRegNoBit7          : 1;
        gctUINT        reserved5             : 10;/* Must be zero'ed */
        gctUINT        dstRegNoBit8          : 1;
        gctUINT        reserved6             : 6; /* Must be zero'ed for non-extopcode mode, otherwise, see NOTE 4 */
        gctUINT        dstType               : 1;
#else
        gctUINT        dstType               : 1;
        gctUINT        reserved6             : 6; /* Must be zero'ed for non-extopcode mode, otherwise, see NOTE 4 */
        gctUINT        dstRegNoBit8          : 1;
        gctUINT        reserved5             : 10;/* Must be zero'ed */
        gctUINT        dstRegNoBit7          : 1;
        gctUINT        reserved4             : 1; /* Must be zero'ed */
        gctUINT        extOpcode             : 8;
        gctUINT        reserved3             : 1; /* Must be zero'ed for non-extopcode mode, otherwise, see NOTE 4 */
        gctUINT        src1Type              : 3;
#endif
    } inst;

    gctUINT            data[4];
}
VSC_MC_LD_INST;

/* Image-load inst case */
typedef union _VSC_MC_IMG_LD_INST
{
    struct
    {
#if !gcdENDIAN_BIG
        gctUINT        baseOpcodeBit0_5      : 6; /* Together with baseOpcodeBit6 to compose base-opcode */
        gctUINT        bBigEndian            : 1; /* need to swap big endian data to little endian if true */
        gctUINT        bUnallocate           : 1; /* USC Unallocate Bit for global memory Load/Store  */
        gctUINT        bEndOfBB              : 1; /* End Of Basic Block bit for non-control-clow instructions */
        gctUINT        reserved0             : 2; /* Must be zero'ed */
        gctUINT        bResultSat            : 1;
        gctUINT        bDstValid             : 1; /* Must be valid */
        gctUINT        dstRelAddr            : 3;
        gctUINT        dstRegNoBit0_6        : 7; /* Together with dstRegNoBit7 and dstRegNoBit8 to compose dstRegNo */
        gctUINT        writeMask             : 4;
        gctUINT        reserved1             : 5; /* For non-evis-mode, must be zero'ed; otherwise, see NOTE 9 */
#else
        gctUINT        reserved1             : 5; /* For non-evis-mode, must be zero'ed; otherwise, see NOTE 9 */
        gctUINT        writeMask             : 4;
        gctUINT        dstRegNoBit0_6        : 7; /* Together with dstRegNoBit7 and dstRegNoBit8 to compose dstRegNo */
        gctUINT        dstRelAddr            : 3;
        gctUINT        bDstValid             : 1; /* Must be valid */
        gctUINT        bResultSat            : 1;
        gctUINT        reserved0             : 2; /* Must be zero'ed */
        gctUINT        bEndOfBB              : 1; /* End Of Basic Block bit for non-control-clow instructions */
        gctUINT        bUnallocate           : 1; /* USC Unallocate Bit for global memory Load/Store  */
        gctUINT        bBigEndian            : 1; /* need to swap big endian data to little endian if true */
        gctUINT        baseOpcodeBit0_5      : 6; /* Together with baseOpcodeBit6 to compose base-opcode */
#endif

#if !gcdENDIAN_BIG
        gctUINT        reserved2             : 7; /* Must be zero'ed for non-evis mode, otherwise, see NOTE 9 */
        gctUINT        bSkipForHelperKickoff : 1;
        gctUINT        bAccessLocalStorage   : 1;
        gctUINT        reserved3             : 1; /* Must be zero'ed */
        gctUINT        bDenorm               : 1;
        gctUINT        bSrc0Valid            : 1; /* Must be valid */
        gctUINT        src0RegNo             : 9;
        gctUINT        instTypeBit0          : 1; /* Together with instTypeBit1_2 to compose instType */
        gctUINT        src0Swizzle           : 8;
        gctUINT        bSrc0ModNeg           : 1;
        gctUINT        bSrc0ModAbs           : 1;
#else
        gctUINT        bSrc0ModAbs           : 1;
        gctUINT        bSrc0ModNeg           : 1;
        gctUINT        src0Swizzle           : 8;
        gctUINT        instTypeBit0          : 1; /* Together with instTypeBit1_2 to compose instType */
        gctUINT        src0RegNo             : 9;
        gctUINT        bSrc0Valid            : 1; /* Must be valid */
        gctUINT        bDenorm               : 1;
        gctUINT        reserved3             : 1; /* Must be zero'ed */
        gctUINT        bAccessLocalStorage   : 1;
        gctUINT        bSkipForHelperKickoff : 1;
        gctUINT        reserved2             : 7; /* Must be zero'ed for non-evis mode, otherwise, see NOTE 9 */
#endif

#if !gcdENDIAN_BIG
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
#else
        gctUINT        instTypeBit1_2        : 2;
        gctUINT        src1RelAddr           : 3;
        gctUINT        bSrc1ModAbs           : 1;
        gctUINT        bSrc1ModNeg           : 1;
        gctUINT        src1Swizzle           : 8;
        gctUINT        baseOpcodeBit6        : 1;
        gctUINT        src1RegNo             : 9;
        gctUINT        bSrc1Valid            : 1; /* Must be valid */
        gctUINT        src0Type              : 3;
        gctUINT        src0RelAddr           : 3;
#endif

#if !gcdENDIAN_BIG
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
#else
        gctUINT        dstType               : 1;
        gctUINT        src2Type              : 3;
        gctUINT        src2RelAddr           : 3;
        gctUINT        dstRegNoBit8          : 1;
        gctUINT        bSrc2ModAbs           : 1;
        gctUINT        bSrc2ModNeg           : 1;
        gctUINT        src2Swizzle           : 8;
        gctUINT        dstRegNoBit7          : 1;
        gctUINT        src2RegNo             : 9;
        gctUINT        bSrc2Valid            : 1; /* Must be valid */
        gctUINT        src1Type              : 3;
#endif
    } inst;

    gctUINT            data[4];
}
VSC_MC_IMG_LD_INST;

/* Store inst case */
typedef union _VSC_MC_ST_INST
{
    struct
    {
#if !gcdENDIAN_BIG
        gctUINT        baseOpcodeBit0_5      : 6; /* Together with baseOpcodeBit6 to compose base-opcode */
        gctUINT        bBigEndian            : 1; /* need to swap big endian data to little endian if true */
        gctUINT        bUnallocate           : 1; /* USC Unallocate Bit for global memory Load/Store  */
        gctUINT        bEndOfBB              : 1; /* End Of Basic Block bit for non-control-clow instructions */
        gctUINT        reserved0             : 14; /* Must be zero'ed */
        gctUINT        writeMask             : 4;
        gctUINT        reserved1             : 5; /* Must be zero'ed */
#else
        gctUINT        reserved1             : 5; /* Must be zero'ed */
        gctUINT        writeMask             : 4;
        gctUINT        reserved0             : 14; /* Must be zero'ed */
        gctUINT        bEndOfBB              : 1; /* End Of Basic Block bit for non-control-clow instructions */
        gctUINT        bUnallocate           : 1; /* USC Unallocate Bit for global memory Load/Store  */
        gctUINT        bBigEndian            : 1; /* need to swap big endian data to little endian if true */
        gctUINT        baseOpcodeBit0_5      : 6; /* Together with baseOpcodeBit6 to compose base-opcode */
#endif

#if !gcdENDIAN_BIG
        gctUINT        reserved2             : 2; /* Must be zero'ed */
        gctUINT        packMode              : 1;
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
#else
        gctUINT        bSrc0ModAbs           : 1;
        gctUINT        bSrc0ModNeg           : 1;
        gctUINT        src0Swizzle           : 8;
        gctUINT        instTypeBit0          : 1; /* Together with instTypeBit1_2 and instTypeBit3 to compose instType */
        gctUINT        src0RegNo             : 9;
        gctUINT        bSrc0Valid            : 1; /* Must be valid */
        gctUINT        bDenorm               : 1;
        gctUINT        instTypeBit3          : 1;
        gctUINT        bAccessLocalStorage   : 1;
        gctUINT        bSkipForHelperKickoff : 1;
        gctUINT        bOffsetX3             : 1;
        gctUINT        offsetLeftShift       : 3; /* Act after X3, do (<< offsetLeftShift) */
        gctUINT        packMode              : 1;
        gctUINT        reserved2             : 2; /* Must be zero'ed */
#endif

#if !gcdENDIAN_BIG
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
#else
        gctUINT        instTypeBit1_2        : 2;
        gctUINT        src1RelAddr           : 3;
        gctUINT        bSrc1ModAbs           : 1;
        gctUINT        bSrc1ModNeg           : 1;
        gctUINT        src1Swizzle           : 8;
        gctUINT        baseOpcodeBit6        : 1;
        gctUINT        src1RegNo             : 9;
        gctUINT        bSrc1Valid            : 1; /* Must be valid */
        gctUINT        src0Type              : 3;
        gctUINT        src0RelAddr           : 3;
#endif

#if !gcdENDIAN_BIG
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
        gctUINT        reserved3             : 1; /* Must be zero'ed */
#else
        gctUINT        reserved3             : 1; /* Must be zero'ed */
        gctUINT        src2Type              : 3;
        gctUINT        src2RelAddr           : 3;
        gctUINT        threadTypeBit1        : 1;
        gctUINT        bSrc2ModAbs           : 1;
        gctUINT        bSrc2ModNeg           : 1;
        gctUINT        src2Swizzle           : 8;
        gctUINT        threadTypeBit0        : 1;
        gctUINT        src2RegNo             : 9;
        gctUINT        bSrc2Valid            : 1; /* Must be valid */
        gctUINT        src1Type              : 3;
#endif
    } inst;

    gctUINT            data[4];
}
VSC_MC_ST_INST;

/* Image-store inst case */
typedef union _VSC_MC_IMG_ST_INST
{
    struct
    {
#if !gcdENDIAN_BIG
        gctUINT        baseOpcodeBit0_5      : 6; /* Together with baseOpcodeBit6 to compose base-opcode */
        gctUINT        bBigEndian            : 1; /* need to swap big endian data to little endian if true */
        gctUINT        bUnallocate           : 1; /* USC Unallocate Bit for global memory Load/Store  */
        gctUINT        bEndOfBB              : 1; /* End Of Basic Block bit for non-control-clow instructions */
        gctUINT        reserved0             : 14; /* Must be zero'ed */
        gctUINT        writeMask             : 4; /* For evis-mode, see Note 9 */
        gctUINT        reserved1             : 5; /* For non-evis-mode, must be zero'ed; otherwise, see NOTE 9 */
#else
        gctUINT        reserved1             : 5; /* For non-evis-mode, must be zero'ed; otherwise, see NOTE 9 */
        gctUINT        writeMask             : 4; /* For evis-mode, see Note 9 */
        gctUINT        reserved0             : 14; /* Must be zero'ed */
        gctUINT        bEndOfBB              : 1; /* End Of Basic Block bit for non-control-clow instructions */
        gctUINT        bUnallocate           : 1; /* USC Unallocate Bit for global memory Load/Store  */
        gctUINT        bBigEndian            : 1; /* need to swap big endian data to little endian if true */
        gctUINT        baseOpcodeBit0_5      : 6; /* Together with baseOpcodeBit6 to compose base-opcode */
#endif

#if !gcdENDIAN_BIG
        gctUINT        reserved2             : 7; /* Must be zero'ed for non-evis mode, otherwise, see NOTE 9 */
        gctUINT        bSkipForHelperKickoff : 1;
        gctUINT        bAccessLocalStorage   : 1;
        gctUINT        reserved3             : 1; /* Must be zero'ed */
        gctUINT        bDenorm               : 1;
        gctUINT        bSrc0Valid            : 1; /* Must be valid */
        gctUINT        src0RegNo             : 9;
        gctUINT        instTypeBit0          : 1; /* Together with instTypeBit1_2 to compose instType */
        gctUINT        src0Swizzle           : 8;
        gctUINT        bSrc0ModNeg           : 1;
        gctUINT        bSrc0ModAbs           : 1;
#else
        gctUINT        bSrc0ModAbs           : 1;
        gctUINT        bSrc0ModNeg           : 1;
        gctUINT        src0Swizzle           : 8;
        gctUINT        instTypeBit0          : 1; /* Together with instTypeBit1_2 to compose instType */
        gctUINT        src0RegNo             : 9;
        gctUINT        bSrc0Valid            : 1; /* Must be valid */
        gctUINT        bDenorm               : 1;
        gctUINT        reserved3             : 1; /* Must be zero'ed */
        gctUINT        bAccessLocalStorage   : 1;
        gctUINT        bSkipForHelperKickoff : 1;
        gctUINT        reserved2             : 7; /* Must be zero'ed for non-evis mode, otherwise, see NOTE 9 */
#endif

#if !gcdENDIAN_BIG
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
#else
        gctUINT        instTypeBit1_2        : 2;
        gctUINT        src1RelAddr           : 3;
        gctUINT        bSrc1ModAbs           : 1;
        gctUINT        bSrc1ModNeg           : 1;
        gctUINT        src1Swizzle           : 8;
        gctUINT        baseOpcodeBit6        : 1;
        gctUINT        src1RegNo             : 9;
        gctUINT        bSrc1Valid            : 1; /* Must be valid */
        gctUINT        src0Type              : 3;
        gctUINT        src0RelAddr           : 3;
#endif

#if !gcdENDIAN_BIG
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
#else
        gctUINT        reserved4             : 1; /* Must be zero'ed */
        gctUINT        src2Type              : 3;
        gctUINT        src2RelAddr           : 3;
        gctUINT        threadTypeBit1        : 1;
        gctUINT        bSrc2ModAbs           : 1;
        gctUINT        bSrc2ModNeg           : 1;
        gctUINT        src2Swizzle           : 8;
        gctUINT        threadTypeBit0        : 1;
        gctUINT        src2RegNo             : 9;
        gctUINT        bSrc2Valid            : 1; /* Must be valid */
        gctUINT        src1Type              : 3;
#endif
    } inst;

    gctUINT            data[4];
}
VSC_MC_IMG_ST_INST;

/* Image-atomic inst case */
typedef union _VSC_MC_IMG_ATOM_INST
{
    struct
    {
#if !gcdENDIAN_BIG
        gctUINT        baseOpcodeBit0_5      : 6; /* Together with baseOpcodeBit6 to compose base-opcode */
        gctUINT        bBigEndian            : 1; /* need to swap big endian data to little endian if true */
        gctUINT        bUnallocate           : 1; /* USC Unallocate Bit for global memory Load/Store  */
        gctUINT        bEndOfBB              : 1; /* End Of Basic Block bit for non-control-clow instructions */
        gctUINT        reserved0             : 2; /* Must be zero'ed */
        gctUINT        bResultSat            : 1;
        gctUINT        bDstValid             : 1; /* Must be valid */
        gctUINT        dstRelAddr            : 3;
        gctUINT        dstRegNoBit0_6        : 7; /* Together with dstRegNoBit7 and dstRegNoBit8 to compose dstRegNo */
        gctUINT        writeMask             : 4;
        gctUINT        reserved1             : 5; /* Must be zero'ed */
#else
        gctUINT        reserved1             : 5; /* Must be zero'ed */
        gctUINT        writeMask             : 4;
        gctUINT        dstRegNoBit0_6        : 7; /* Together with dstRegNoBit7 and dstRegNoBit8 to compose dstRegNo */
        gctUINT        dstRelAddr            : 3;
        gctUINT        bDstValid             : 1; /* Must be valid */
        gctUINT        bResultSat            : 1;
        gctUINT        reserved0             : 2; /* Must be zero'ed */
        gctUINT        bEndOfBB              : 1; /* End Of Basic Block bit for non-control-clow instructions */
        gctUINT        bUnallocate           : 1; /* USC Unallocate Bit for global memory Load/Store  */
        gctUINT        bBigEndian            : 1; /* need to swap big endian data to little endian if true */
        gctUINT        baseOpcodeBit0_5      : 6; /* Together with baseOpcodeBit6 to compose base-opcode */
#endif

#if !gcdENDIAN_BIG
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
#else
        gctUINT        bSrc0ModAbs           : 1;
        gctUINT        bSrc0ModNeg           : 1;
        gctUINT        src0Swizzle           : 8;
        gctUINT        instTypeBit0          : 1; /* Together with instTypeBit1_2 to compose instType */
        gctUINT        src0RegNo             : 9;
        gctUINT        bSrc0Valid            : 1; /* Must be valid */
        gctUINT        reserved3             : 2; /* Must be zero'ed */
        gctUINT        bAccessLocalStorage   : 1;
        gctUINT        bSkipForHelperKickoff : 1;
        gctUINT        b3dImgMode            : 1;
        gctUINT        atomicMode            : 3;
        gctUINT        reserved2             : 3; /* Must be zero'ed */
#endif

#if !gcdENDIAN_BIG
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
#else
        gctUINT        instTypeBit1_2        : 2;
        gctUINT        src1RelAddr           : 3;
        gctUINT        bSrc1ModAbs           : 1;
        gctUINT        bSrc1ModNeg           : 1;
        gctUINT        src1Swizzle           : 8;
        gctUINT        baseOpcodeBit6        : 1;
        gctUINT        src1RegNo             : 9;
        gctUINT        bSrc1Valid            : 1; /* Must be valid */
        gctUINT        src0Type              : 3;
        gctUINT        src0RelAddr           : 3;
#endif

#if !gcdENDIAN_BIG
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
#else
        gctUINT        dstType               : 1;
        gctUINT        src2Type              : 3;
        gctUINT        src2RelAddr           : 3;
        gctUINT        dstRegNoBit8          : 1;
        gctUINT        bSrc2ModAbs           : 1;
        gctUINT        bSrc2ModNeg           : 1;
        gctUINT        src2Swizzle           : 8;
        gctUINT        dstRegNoBit7          : 1;
        gctUINT        src2RegNo             : 9;
        gctUINT        bSrc2Valid            : 1; /* Must be valid */
        gctUINT        src1Type              : 3;
#endif
    } inst;

    gctUINT            data[4];
}
VSC_MC_IMG_ATOM_INST;

/* Store_attr inst 0x42 */
typedef union _VSC_MC_STORE_ATTR_INST
{
    struct
    {
#if !gcdENDIAN_BIG
        gctUINT        baseOpcodeBit0_5      : 6; /* Together with baseOpcodeBit6 to compose base-opcode */
        gctUINT        reserved0             : 2; /* Must be zero'ed */
        gctUINT        bEndOfBB              : 1; /* End Of Basic Block bit for non-control-clow instructions */
        gctUINT        reserved1             : 14; /* Must be zero'ed */
        gctUINT        writeMask             : 4;
        gctUINT        reserved2             : 3; /* Must be zero'ed */
        gctUINT        reserved3             : 2; /* Must be zero'ed */
#else
        gctUINT        reserved3             : 2; /* Must be zero'ed */
        gctUINT        reserved2             : 3; /* Must be zero'ed */
        gctUINT        writeMask             : 4;
        gctUINT        reserved1             : 14; /* Must be zero'ed */
        gctUINT        bEndOfBB              : 1; /* End Of Basic Block bit for non-control-clow instructions */
        gctUINT        reserved0             : 2; /* Must be zero'ed */
        gctUINT        baseOpcodeBit0_5      : 6; /* Together with baseOpcodeBit6 to compose base-opcode */
#endif

#if !gcdENDIAN_BIG
        gctUINT        reserved4             : 3; /* Must be zero'ed */
        gctUINT        shStageClient         : 2;
        gctUINT        reserved5             : 1; /* Must be zero'ed */
        gctUINT        attrLayout            : 1;
        gctUINT        reserved6             : 4; /* Must be zero'ed */
        gctUINT        bSrc0Valid            : 1; /* Must be valid */
        gctUINT        src0RegNo             : 9;
        gctUINT        instTypeBit0          : 1; /* Together with instTypeBit1_2 to compose instType */
        gctUINT        src0Swizzle           : 8;
        gctUINT        bSrc0ModNeg           : 1;
        gctUINT        bSrc0ModAbs           : 1;
#else
        gctUINT        bSrc0ModAbs           : 1;
        gctUINT        bSrc0ModNeg           : 1;
        gctUINT        src0Swizzle           : 8;
        gctUINT        instTypeBit0          : 1; /* Together with instTypeBit1_2 to compose instType */
        gctUINT        src0RegNo             : 9;
        gctUINT        bSrc0Valid            : 1; /* Must be valid */
        gctUINT        reserved6             : 4; /* Must be zero'ed */
        gctUINT        attrLayout            : 1;
        gctUINT        reserved5             : 1; /* Must be zero'ed */
        gctUINT        shStageClient         : 2;
        gctUINT        reserved4             : 3; /* Must be zero'ed */
#endif

#if !gcdENDIAN_BIG
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
#else
        gctUINT        instTypeBit1_2        : 2;
        gctUINT        src1RelAddr           : 3;
        gctUINT        bSrc1ModAbs           : 1;
        gctUINT        bSrc1ModNeg           : 1;
        gctUINT        src1Swizzle           : 8;
        gctUINT        baseOpcodeBit6        : 1;
        gctUINT        src1RegNo             : 9;
        gctUINT        bSrc1Valid            : 1; /* Must be valid */
        gctUINT        src0Type              : 3;
        gctUINT        src0RelAddr           : 3;
#endif

#if !gcdENDIAN_BIG
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
        gctUINT        reserved7             : 1; /* Must be zero'ed */
#else
        gctUINT        reserved7             : 1; /* Must be zero'ed */
        gctUINT        src2Type              : 3;
        gctUINT        src2RelAddr           : 3;
        gctUINT        threadTypeBit1        : 1;
        gctUINT        bSrc2ModAbs           : 1;
        gctUINT        bSrc2ModNeg           : 1;
        gctUINT        src2Swizzle           : 8;
        gctUINT        threadTypeBit0        : 1;
        gctUINT        src2RegNo             : 9;
        gctUINT        bSrc2Valid            : 1; /* Must be valid */
        gctUINT        src1Type              : 3;
#endif
    } inst;

    gctUINT            data[4];
}
VSC_MC_STORE_ATTR_INST;

/* Select_map inst 0x43 */
typedef union _VSC_MC_SELECT_MAP_INST
{
    struct
    {
#if !gcdENDIAN_BIG
        gctUINT        baseOpcodeBit0_5      : 6; /* Together with baseOpcodeBit6 to compose base-opcode */
        gctUINT        reserved0             : 2; /* Must be zero'ed */
        gctUINT        bEndOfBB              : 1; /* End Of Basic Block bit for non-control-clow instructions */
        gctUINT        reserved1             : 2; /* Must be zero'ed */
        gctUINT        bResultSat            : 1;
        gctUINT        bDstValid             : 1; /* Must be valid, see rangeToMatch for more info */
        gctUINT        dstRelAddr            : 3;
        gctUINT        dstRegNoBit0_6        : 7; /* Together with dstRegNoBit7 and dstRegNoBit8 to compose dstRegNo */
        gctUINT        writeMask             : 4;
        gctUINT        reserved2             : 5; /* Must be zero'ed */
#else
        gctUINT        reserved2             : 5; /* Must be zero'ed */
        gctUINT        writeMask             : 4;
        gctUINT        dstRegNoBit0_6        : 7; /* Together with dstRegNoBit7 and dstRegNoBit8 to compose dstRegNo */
        gctUINT        dstRelAddr            : 3;
        gctUINT        bDstValid             : 1; /* Must be valid, see rangeToMatch for more info */
        gctUINT        bResultSat            : 1;
        gctUINT        reserved1             : 2; /* Must be zero'ed */
        gctUINT        bEndOfBB              : 1; /* End Of Basic Block bit for non-control-clow instructions */
        gctUINT        reserved0             : 2; /* Must be zero'ed */
        gctUINT        baseOpcodeBit0_5      : 6; /* Together with baseOpcodeBit6 to compose base-opcode */
#endif

#if !gcdENDIAN_BIG
        gctUINT        reserved3             : 3; /* Must be zero'ed */
        gctUINT        rangeToMatch          : 4; /* Only the range is matched, the dst will be written */
        gctUINT        reserved4             : 3; /* Must be zero'ed */
        gctUINT        bCompSel              : 1; /* If TRUE, component select, otherwise, src select */
        gctUINT        bSrc0Valid            : 1; /* Must be valid */
        gctUINT        src0RegNo             : 9;
        gctUINT        instTypeBit0          : 1; /* Together with instTypeBit1_2 to compose instType */
        gctUINT        src0Swizzle           : 8;
        gctUINT        bSrc0ModNeg           : 1;
        gctUINT        bSrc0ModAbs           : 1;
#else
        gctUINT        bSrc0ModAbs           : 1;
        gctUINT        bSrc0ModNeg           : 1;
        gctUINT        src0Swizzle           : 8;
        gctUINT        instTypeBit0          : 1; /* Together with instTypeBit1_2 to compose instType */
        gctUINT        src0RegNo             : 9;
        gctUINT        bSrc0Valid            : 1; /* Must be valid */
        gctUINT        bCompSel              : 1; /* If TRUE, component select, otherwise, src select */
        gctUINT        reserved4             : 3; /* Must be zero'ed */
        gctUINT        rangeToMatch          : 4; /* Only the range is matched, the dst will be written */
        gctUINT        reserved3             : 3; /* Must be zero'ed */
#endif

#if !gcdENDIAN_BIG
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
#else
        gctUINT        instTypeBit1_2        : 2;
        gctUINT        src1RelAddr           : 3;
        gctUINT        bSrc1ModAbs           : 1;
        gctUINT        bSrc1ModNeg           : 1;
        gctUINT        src1Swizzle           : 8;
        gctUINT        baseOpcodeBit6        : 1;
        gctUINT        src1RegNo             : 9;
        gctUINT        bSrc1Valid            : 1; /* Must be valid */
        gctUINT        src0Type              : 3;
        gctUINT        src0RelAddr           : 3;
#endif

#if !gcdENDIAN_BIG
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
#else
        gctUINT        dstType               : 1;
        gctUINT        src2Type              : 3;
        gctUINT        src2RelAddr           : 3;
        gctUINT        dstRegNoBit8          : 1;
        gctUINT        bSrc2ModAbs           : 1;
        gctUINT        bSrc2ModNeg           : 1;
        gctUINT        src2Swizzle           : 8;
        gctUINT        dstRegNoBit7          : 1;
        gctUINT        src2RegNo             : 9;
        gctUINT        bSrc2Valid            : 1; /* Must be valid */
        gctUINT        src1Type              : 3;
#endif
    } inst;

    gctUINT            data[4];
}
VSC_MC_SELECT_MAP_INST;

/* Direct-branch inst case 0, this is common direct case, can not be used under dual16 */
typedef union _VSC_MC_DIRECT_BRANCH_0_INST
{
    struct
    {
#if !gcdENDIAN_BIG
        gctUINT        baseOpcodeBit0_5      : 6; /* Together with baseOpcodeBit6 to compose base-opcode */
        gctUINT        condOpCode            : 5;
        gctUINT        reserved0             : 21;/* Must be zero'ed */
#else
        gctUINT        reserved0             : 21;/* Must be zero'ed */
        gctUINT        condOpCode            : 5;
        gctUINT        baseOpcodeBit0_5      : 6; /* Together with baseOpcodeBit6 to compose base-opcode */
#endif

#if !gcdENDIAN_BIG
        gctUINT        reserved1             : 2; /* Must be zero'ed */
        gctUINT        packMode              : 1;
        gctUINT        reserved2             : 8; /* Must be zero'ed */
        gctUINT        bSrc0Valid            : 1;
        gctUINT        src0RegNo             : 9;
        gctUINT        instTypeBit0          : 1; /* Together with instTypeBit1_2 to compose instType */
        gctUINT        src0Swizzle           : 8;
        gctUINT        bSrc0ModNeg           : 1;
        gctUINT        bSrc0ModAbs           : 1;
#else
        gctUINT        bSrc0ModAbs           : 1;
        gctUINT        bSrc0ModNeg           : 1;
        gctUINT        src0Swizzle           : 8;
        gctUINT        instTypeBit0          : 1; /* Together with instTypeBit1_2 to compose instType */
        gctUINT        src0RegNo             : 9;
        gctUINT        bSrc0Valid            : 1;
        gctUINT        reserved2             : 8; /* Must be zero'ed */
        gctUINT        packMode              : 1;
        gctUINT        reserved1             : 2; /* Must be zero'ed */
#endif

#if !gcdENDIAN_BIG
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
#else
        gctUINT        instTypeBit1_2        : 2;
        gctUINT        src1RelAddr           : 3;
        gctUINT        bSrc1ModAbs           : 1;
        gctUINT        bSrc1ModNeg           : 1;
        gctUINT        src1Swizzle           : 8;
        gctUINT        baseOpcodeBit6        : 1;
        gctUINT        src1RegNo             : 9;
        gctUINT        bSrc1Valid            : 1;
        gctUINT        src0Type              : 3;
        gctUINT        src0RelAddr           : 3;
#endif

#if !gcdENDIAN_BIG
        gctUINT        src1Type              : 3;
        gctUINT        reserved3             : 1; /* Must be zero'ed */
        gctUINT        loopOpType            : 1; /* Must be 0x0 under dual16 mode */
        gctUINT        reserved4             : 2; /* Must be zero'ed */
        gctUINT        branchTarget          : 20;
        gctUINT        reserved5             : 5; /* Must be zero'ed */
#else
        gctUINT        reserved5             : 5; /* Must be zero'ed */
        gctUINT        branchTarget          : 20;
        gctUINT        reserved4             : 2; /* Must be zero'ed */
        gctUINT        loopOpType            : 1; /* Must be 0x0 under dual16 mode */
        gctUINT        reserved3             : 1; /* Must be zero'ed */
        gctUINT        src1Type              : 3;
#endif
    } inst;

    gctUINT            data[4];
}
VSC_MC_DIRECT_BRANCH_0_INST;

/* Direct-branch inst case 1, can be used under dual16 */
typedef union _VSC_MC_DIRECT_BRANCH_1_INST
{
    struct
    {
#if !gcdENDIAN_BIG
        gctUINT        baseOpcodeBit0_5      : 6; /* Together with baseOpcodeBit6 to compose base-opcode */
        gctUINT        condOpCode            : 5;
        gctUINT        reserved0             : 21;/* Must be zero'ed */
#else
        gctUINT        reserved0             : 21;/* Must be zero'ed */
        gctUINT        condOpCode            : 5;
        gctUINT        baseOpcodeBit0_5      : 6; /* Together with baseOpcodeBit6 to compose base-opcode */
#endif

#if !gcdENDIAN_BIG
        gctUINT        reserved1             : 2; /* Must be zero'ed */
        gctUINT        packMode              : 1;
        gctUINT        reserved2             : 8; /* Must be zero'ed */
        gctUINT        bSrc0Valid            : 1;
        gctUINT        src0RegNo             : 9;
        gctUINT        instTypeBit0          : 1; /* Together with instTypeBit1_2 to compose instType */
        gctUINT        src0Swizzle           : 8;
        gctUINT        bSrc0ModNeg           : 1;
        gctUINT        bSrc0ModAbs           : 1;
#else
        gctUINT        bSrc0ModAbs           : 1;
        gctUINT        bSrc0ModNeg           : 1;
        gctUINT        src0Swizzle           : 8;
        gctUINT        instTypeBit0          : 1; /* Together with instTypeBit1_2 to compose instType */
        gctUINT        src0RegNo             : 9;
        gctUINT        bSrc0Valid            : 1;
        gctUINT        reserved2             : 8; /* Must be zero'ed */
        gctUINT        packMode              : 1;
        gctUINT        reserved1             : 2; /* Must be zero'ed */
#endif

#if !gcdENDIAN_BIG
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
#else
        gctUINT        instTypeBit1_2        : 2;
        gctUINT        src1RelAddr           : 3;
        gctUINT        bSrc1ModAbs           : 1;
        gctUINT        bSrc1ModNeg           : 1;
        gctUINT        src1Swizzle           : 8;
        gctUINT        baseOpcodeBit6        : 1;
        gctUINT        src1RegNo             : 9;
        gctUINT        bSrc1Valid            : 1;
        gctUINT        src0Type              : 3;
        gctUINT        src0RelAddr           : 3;
#endif

#if !gcdENDIAN_BIG
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
#else
        gctUINT        reserved3             : 1; /* Must be zero'ed */
        gctUINT        src2Type              : 3; /* Must be set to 0x7 */
        gctUINT        immType               : 2;
        gctUINT        branchTargetBit19     : 1;
        gctUINT        threadTypeBit1        : 1;
        gctUINT        branchTargetBit9_18   : 10;
        gctUINT        threadTypeBit0        : 1;
        gctUINT        branchTargetBit0_8    : 9;
        gctUINT        bSrc2Valid            : 1; /* Must be valid */
        gctUINT        src1Type              : 3;
#endif
    } inst;

    gctUINT            data[4];
}
VSC_MC_DIRECT_BRANCH_1_INST;

/* Indirect-branch inst case, can be used under dual16 */
typedef union _VSC_MC_INDIRECT_BRANCH_INST
{
    struct
    {
#if !gcdENDIAN_BIG
        gctUINT        baseOpcodeBit0_5      : 6; /* Together with baseOpcodeBit6 to compose base-opcode */
        gctUINT        condOpCode            : 5; /* Must be set to 0x00 */
        gctUINT        reserved0             : 21;/* Must be zero'ed */
#else
        gctUINT        reserved0             : 21;/* Must be zero'ed */
        gctUINT        condOpCode            : 5; /* Must be set to 0x00 */
        gctUINT        baseOpcodeBit0_5      : 6; /* Together with baseOpcodeBit6 to compose base-opcode */
#endif

#if !gcdENDIAN_BIG
        gctUINT        reserved1             : 2; /* Must be zero'ed */
        gctUINT        packMode              : 1;
        gctUINT        reserved2             : 18;/* Must be zero'ed */
        gctUINT        instTypeBit0          : 1; /* Together with instTypeBit1_2 to compose instType */
        gctUINT        reserved3             : 10;/* Must be zero'ed */
#else
        gctUINT        reserved3             : 10;/* Must be zero'ed */
        gctUINT        instTypeBit0          : 1; /* Together with instTypeBit1_2 to compose instType */
        gctUINT        reserved2             : 18;/* Must be zero'ed */
        gctUINT        packMode              : 1;
        gctUINT        reserved1             : 2; /* Must be zero'ed */
#endif

#if !gcdENDIAN_BIG
        gctUINT        reserved4             : 16;/* Must be zero'ed */
        gctUINT        baseOpcodeBit6        : 1;
        gctUINT        reserved5             : 13;/* Must be zero'ed */
        gctUINT        instTypeBit1_2        : 2;
#else
        gctUINT        instTypeBit1_2        : 2;
        gctUINT        reserved5             : 13;/* Must be zero'ed */
        gctUINT        baseOpcodeBit6        : 1;
        gctUINT        reserved4             : 16;/* Must be zero'ed */
#endif

#if !gcdENDIAN_BIG
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
#else
        gctUINT        reserved7             : 1; /* Must be zero'ed */
        gctUINT        src2Type              : 3;
        gctUINT        src2RelAddr           : 3;
        gctUINT        threadTypeBit1        : 1;
        gctUINT        bSrc2ModAbs           : 1;
        gctUINT        bSrc2ModNeg           : 1;
        gctUINT        src2Swizzle           : 8;
        gctUINT        threadTypeBit0        : 1;
        gctUINT        src2RegNo             : 9;
        gctUINT        bSrc2Valid            : 1; /* Must be valid */
        gctUINT        reserved6             : 3; /* Must be zero'ed */
#endif
    } inst;

    gctUINT            data[4];
}
VSC_MC_INDIRECT_BRANCH_INST;

/* Direct-call inst case, can not be used under dual16 */
typedef union _VSC_MC_DIRECT_CALL_INST
{
    struct
    {
#if !gcdENDIAN_BIG
        gctUINT        baseOpcodeBit0_5      : 6; /* Together with baseOpcodeBit6 to compose base-opcode */
        gctUINT        reserved0             : 26;/* Must be zero'ed */
#else
        gctUINT        reserved0             : 26;/* Must be zero'ed */
        gctUINT        baseOpcodeBit0_5      : 6; /* Together with baseOpcodeBit6 to compose base-opcode */
#endif

        gctUINT        reserved1             : 32;/* Must be zero'ed */

#if !gcdENDIAN_BIG
        gctUINT        reserved2             : 16;/* Must be zero'ed */
        gctUINT        baseOpcodeBit6        : 1;
        gctUINT        reserved3             : 15;/* Must be zero'ed */
#else
        gctUINT        reserved3             : 15;/* Must be zero'ed */
        gctUINT        baseOpcodeBit6        : 1;
        gctUINT        reserved2             : 16;/* Must be zero'ed */
#endif

#if !gcdENDIAN_BIG
        gctUINT        reserved4             : 7; /* Must be zero'ed */
        gctUINT        callTarget            : 20;
        gctUINT        reserved5             : 5; /* Must be zero'ed */
#else
        gctUINT        reserved5             : 5; /* Must be zero'ed */
        gctUINT        callTarget            : 20;
        gctUINT        reserved4             : 7; /* Must be zero'ed */
#endif
    } inst;

    gctUINT            data[4];
}
VSC_MC_DIRECT_CALL_INST;

/* Indirect-call inst case, can be used under dual16 */
typedef union _VSC_MC_INDIRECT_CALL_INST
{
    struct
    {
#if !gcdENDIAN_BIG
        gctUINT        baseOpcodeBit0_5      : 6; /* Together with baseOpcodeBit6 to compose base-opcode */
        gctUINT        reserved0             : 26;/* Must be zero'ed */
#else
        gctUINT        reserved0             : 26;/* Must be zero'ed */
        gctUINT        baseOpcodeBit0_5      : 6; /* Together with baseOpcodeBit6 to compose base-opcode */
#endif

#if !gcdENDIAN_BIG
        gctUINT        reserved1             : 21;/* Must be zero'ed */
        gctUINT        instTypeBit0          : 1; /* Together with instTypeBit1_2 to compose instType */
        gctUINT        reserved2             : 10;
#else
        gctUINT        reserved2             : 10;
        gctUINT        instTypeBit0          : 1; /* Together with instTypeBit1_2 to compose instType */
        gctUINT        reserved1             : 21;/* Must be zero'ed */
#endif

#if !gcdENDIAN_BIG
        gctUINT        reserved3             : 16;/* Must be zero'ed */
        gctUINT        baseOpcodeBit6        : 1;
        gctUINT        reserved4             : 13;/* Must be zero'ed */
        gctUINT        instTypeBit1_2        : 2;
#else
        gctUINT        instTypeBit1_2        : 2;
        gctUINT        reserved4             : 13;/* Must be zero'ed */
        gctUINT        baseOpcodeBit6        : 1;
        gctUINT        reserved3             : 16;/* Must be zero'ed */
#endif

#if !gcdENDIAN_BIG
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
#else
        gctUINT        reserved6             : 1; /* Must be zero'ed */
        gctUINT        src2Type              : 3;
        gctUINT        src2RelAddr           : 3;
        gctUINT        threadTypeBit1        : 1;
        gctUINT        bSrc2ModAbs           : 1;
        gctUINT        bSrc2ModNeg           : 1;
        gctUINT        src2Swizzle           : 8;
        gctUINT        threadTypeBit0        : 1;
        gctUINT        src2RegNo             : 9;
        gctUINT        bSrc2Valid            : 1; /* Must be valid */
        gctUINT        reserved5             : 3; /* Must be zero'ed */
#endif
    } inst;

    gctUINT            data[4];
}
VSC_MC_INDIRECT_CALL_INST;

/* Loop/rep inst case */
typedef union _VSC_MC_LOOP_INST
{
    struct
    {
#if !gcdENDIAN_BIG
        gctUINT        baseOpcodeBit0_5      : 6; /* Together with baseOpcodeBit6 to compose base-opcode */
        gctUINT        reserved0             : 26;/* Must be zero'ed */
#else
        gctUINT        reserved0             : 26;/* Must be zero'ed */
        gctUINT        baseOpcodeBit0_5      : 6; /* Together with baseOpcodeBit6 to compose base-opcode */
#endif

#if !gcdENDIAN_BIG
        gctUINT        reserved1             : 21;/* Must be zero'ed */
        gctUINT        instTypeBit0          : 1; /* Together with instTypeBit1_2 to compose instType */
        gctUINT        reserved2             : 10;
#else
        gctUINT        reserved2             : 10;
        gctUINT        instTypeBit0          : 1; /* Together with instTypeBit1_2 to compose instType */
        gctUINT        reserved1             : 21;/* Must be zero'ed */
#endif

#if !gcdENDIAN_BIG
        gctUINT        reserved3             : 6;
        gctUINT        bSrc1Valid            : 1; /* Must be valid */
        gctUINT        src1RegNo             : 9;
        gctUINT        baseOpcodeBit6        : 1;
        gctUINT        src1Swizzle           : 8;
        gctUINT        bSrc1ModNeg           : 1;
        gctUINT        bSrc1ModAbs           : 1;
        gctUINT        src1RelAddr           : 3;
        gctUINT        instTypeBit1_2        : 2;
#else
        gctUINT        instTypeBit1_2        : 2;
        gctUINT        src1RelAddr           : 3;
        gctUINT        bSrc1ModAbs           : 1;
        gctUINT        bSrc1ModNeg           : 1;
        gctUINT        src1Swizzle           : 8;
        gctUINT        baseOpcodeBit6        : 1;
        gctUINT        src1RegNo             : 9;
        gctUINT        bSrc1Valid            : 1; /* Must be valid */
        gctUINT        reserved3             : 6;
#endif

#if !gcdENDIAN_BIG
        gctUINT        src1Type              : 3;
        gctUINT        reserved4             : 4; /* Must be zero'ed */
        gctUINT        branchTarget          : 20;
        gctUINT        reserved5             : 5; /* Must be zero'ed */
#else
        gctUINT        reserved5             : 5; /* Must be zero'ed */
        gctUINT        branchTarget          : 20;
        gctUINT        reserved4             : 4; /* Must be zero'ed */
        gctUINT        src1Type              : 3;
#endif
    } inst;

    gctUINT            data[4];
}
VSC_MC_LOOP_INST;

/* Emit inst case */
typedef union _VSC_MC_EMIT_INST
{
    struct
    {
#if !gcdENDIAN_BIG
        gctUINT        baseOpcodeBit0_5      : 6; /* Together with baseOpcodeBit6 to compose base-opcode */
        gctUINT        reserved0             : 2; /* Must be zero'ed */
        gctUINT        bEndOfBB              : 1; /* End Of Basic Block bit for non-control-clow instructions */
        gctUINT        reserved1             : 2; /* Must be zero'ed */
        gctUINT        bResultSat            : 1;
        gctUINT        bDstValid             : 1; /* Must be valid */
        gctUINT        dstRelAddr            : 3;
        gctUINT        dstRegNoBit0_6        : 7; /* Together with dstRegNoBit7 and dstRegNoBit8 to compose dstRegNo */
        gctUINT        writeMask             : 4;
        gctUINT        reserved2             : 5; /* Must be zero'ed */
#else
        gctUINT        reserved2             : 5; /* Must be zero'ed */
        gctUINT        writeMask             : 4;
        gctUINT        dstRegNoBit0_6        : 7; /* Together with dstRegNoBit7 and dstRegNoBit8 to compose dstRegNo */
        gctUINT        dstRelAddr            : 3;
        gctUINT        bDstValid             : 1; /* Must be valid */
        gctUINT        bResultSat            : 1;
        gctUINT        reserved1             : 2; /* Must be zero'ed */
        gctUINT        bEndOfBB              : 1; /* End Of Basic Block bit for non-control-clow instructions */
        gctUINT        reserved0             : 2; /* Must be zero'ed */
        gctUINT        baseOpcodeBit0_5      : 6; /* Together with baseOpcodeBit6 to compose base-opcode */
#endif

#if !gcdENDIAN_BIG
        gctUINT        reserved3             : 3; /* Must be zero'ed */
        gctUINT        bNeedRestartPrim      : 1;
        gctUINT        bNoJmpToEndOnMaxVtxCnt: 1;
        gctUINT        reserved4             : 6; /* Must be zero'ed */
        gctUINT        bSrc0Valid            : 1; /* Must be valid */
        gctUINT        src0RegNo             : 9;
        gctUINT        instTypeBit0          : 1; /* Together with instTypeBit1_2 to compose instType */
        gctUINT        src0Swizzle           : 8;
        gctUINT        bSrc0ModNeg           : 1;
        gctUINT        bSrc0ModAbs           : 1;
#else
        gctUINT        bSrc0ModAbs           : 1;
        gctUINT        bSrc0ModNeg           : 1;
        gctUINT        src0Swizzle           : 8;
        gctUINT        instTypeBit0          : 1; /* Together with instTypeBit1_2 to compose instType */
        gctUINT        src0RegNo             : 9;
        gctUINT        bSrc0Valid            : 1; /* Must be valid */
        gctUINT        reserved4             : 6; /* Must be zero'ed */
        gctUINT        bNoJmpToEndOnMaxVtxCnt: 1;
        gctUINT        bNeedRestartPrim      : 1;
        gctUINT        reserved3             : 3; /* Must be zero'ed */
#endif

#if !gcdENDIAN_BIG
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
#else
        gctUINT        instTypeBit1_2        : 2;
        gctUINT        src1RelAddr           : 3;
        gctUINT        bSrc1ModAbs           : 1;
        gctUINT        bSrc1ModNeg           : 1;
        gctUINT        src1Swizzle           : 8;
        gctUINT        baseOpcodeBit6        : 1;
        gctUINT        src1RegNo             : 9;
        gctUINT        bSrc1Valid            : 1;
        gctUINT        src0Type              : 3;
        gctUINT        src0RelAddr           : 3;
#endif

#if !gcdENDIAN_BIG
        gctUINT        src1Type              : 3;
        gctUINT        reserved5             : 1; /* Must be zero'ed for non-extopcode mode, otherwise, see NOTE 4 */
        gctUINT        extOpcode             : 8;
        gctUINT        reserved6             : 1; /* Must be zero'ed */
        gctUINT        dstRegNoBit7          : 1;
        gctUINT        reserved7             : 10;/* Must be zero'ed */
        gctUINT        dstRegNoBit8          : 1;
        gctUINT        reserved8             : 6; /* Must be zero'ed for non-extopcode mode, otherwise, see NOTE 4 */
        gctUINT        dstType               : 1;
#else
        gctUINT        dstType               : 1;
        gctUINT        reserved8             : 6; /* Must be zero'ed for non-extopcode mode, otherwise, see NOTE 4 */
        gctUINT        dstRegNoBit8          : 1;
        gctUINT        reserved7             : 10;/* Must be zero'ed */
        gctUINT        dstRegNoBit7          : 1;
        gctUINT        reserved6             : 1; /* Must be zero'ed */
        gctUINT        extOpcode             : 8;
        gctUINT        reserved5             : 1; /* Must be zero'ed for non-extopcode mode, otherwise, see NOTE 4 */
        gctUINT        src1Type              : 3;
#endif
    } inst;

    gctUINT            data[4];
}
VSC_MC_EMIT_INST;

/* Evis-mode inst case */
typedef union _VSC_MC_EVIS_INST
{
    struct
    {
#if !gcdENDIAN_BIG
        gctUINT        baseOpcodeBit0_5      : 6; /* Together with baseOpcodeBit6 to compose base-opcode, and base-opcode must be 0x45 */
        gctUINT        reserved0             : 2; /* Must be zero'ed */
        gctUINT        bEndOfBB              : 1; /* End Of Basic Block bit for non-control-clow instructions */
        gctUINT        reserved1             : 2; /* Must be zero'ed */
        gctUINT        bResultSat            : 1;
        gctUINT        bDstValid             : 1; /* Must be valid */
        gctUINT        extEvisOpCodeBit0_2   : 3; /* Together with extEvisOpCodeBit3 and extEvisOpCodeBit4_5 to extEvisOpCode */
        gctUINT        dstRegNoBit0_6        : 7; /* Together with dstRegNoBit7 and dstRegNoBit8 to compose dstRegNo */
        gctUINT        startDstCompIdx       : 4;
        gctUINT        endDstCompIdx         : 4;
        gctUINT        extEvisOpCodeBit3     : 1;
#else
        gctUINT        extEvisOpCodeBit3     : 1;
        gctUINT        endDstCompIdx         : 4;
        gctUINT        startDstCompIdx       : 4;
        gctUINT        dstRegNoBit0_6        : 7; /* Together with dstRegNoBit7 and dstRegNoBit8 to compose dstRegNo */
        gctUINT        extEvisOpCodeBit0_2   : 3; /* Together with extEvisOpCodeBit3 and extEvisOpCodeBit4_5 to extEvisOpCode */
        gctUINT        bDstValid             : 1; /* Must be valid */
        gctUINT        bResultSat            : 1;
        gctUINT        reserved1             : 2; /* Must be zero'ed */
        gctUINT        bEndOfBB              : 1; /* End Of Basic Block bit for non-control-clow instructions */
        gctUINT        reserved0             : 2; /* Must be zero'ed */
        gctUINT        baseOpcodeBit0_5      : 6; /* Together with baseOpcodeBit6 to compose base-opcode, and base-opcode must be 0x45 */
#endif

#if !gcdENDIAN_BIG
        gctUINT        extEvisOpCodeBit4_5   : 2;
        gctUINT        evisState             : 9;
        gctUINT        bSrc0Valid            : 1; /* Must be valid */
        gctUINT        src0RegNo             : 9;
        gctUINT        instTypeBit0          : 1; /* Together with instTypeBit1_2 to compose instType */
        gctUINT        startSrcCompIdx       : 4; /* AKA sourceBin */
        gctUINT        reserved2             : 6; /* Must be zero'ed */
#else
        gctUINT        reserved2             : 6; /* Must be zero'ed */
        gctUINT        startSrcCompIdx       : 4; /* AKA sourceBin */
        gctUINT        instTypeBit0          : 1; /* Together with instTypeBit1_2 to compose instType */
        gctUINT        src0RegNo             : 9;
        gctUINT        bSrc0Valid            : 1; /* Must be valid */
        gctUINT        evisState             : 9;
        gctUINT        extEvisOpCodeBit4_5   : 2;
#endif

#if !gcdENDIAN_BIG
        gctUINT        src0RelAddr           : 3;
        gctUINT        src0Type              : 3;
        gctUINT        bSrc1Valid            : 1; /* Must be valid */
        gctUINT        src1RegNo             : 9;
        gctUINT        baseOpcodeBit6        : 1;
        gctUINT        src1Swizzle           : 8;
        gctUINT        reserved3             : 2; /* Must be zero'ed */
        gctUINT        src1RelAddr           : 3;
        gctUINT        instTypeBit1_2        : 2;
#else
        gctUINT        instTypeBit1_2        : 2;
        gctUINT        src1RelAddr           : 3;
        gctUINT        reserved3             : 2; /* Must be zero'ed */
        gctUINT        src1Swizzle           : 8;
        gctUINT        baseOpcodeBit6        : 1;
        gctUINT        src1RegNo             : 9;
        gctUINT        bSrc1Valid            : 1; /* Must be valid */
        gctUINT        src0Type              : 3;
        gctUINT        src0RelAddr           : 3;
#endif

#if !gcdENDIAN_BIG
        gctUINT        src1Type              : 3;
        gctUINT        bSrc2Valid            : 1;
        gctUINT        src2RegNo             : 9;
        gctUINT        dstRegNoBit7          : 1;
        gctUINT        src2Swizzle           : 8;
        gctUINT        reserved4             : 2; /* Must be zero'ed */
        gctUINT        dstRegNoBit8          : 1;
        gctUINT        src2RelAddr           : 3;
        gctUINT        src2Type              : 3;
        gctUINT        dstType               : 1;
#else
        gctUINT        dstType               : 1;
        gctUINT        src2Type              : 3;
        gctUINT        src2RelAddr           : 3;
        gctUINT        dstRegNoBit8          : 1;
        gctUINT        reserved4             : 2; /* Must be zero'ed */
        gctUINT        src2Swizzle           : 8;
        gctUINT        dstRegNoBit7          : 1;
        gctUINT        src2RegNo             : 9;
        gctUINT        bSrc2Valid            : 1;
        gctUINT        src1Type              : 3;
#endif
    } inst;

    gctUINT            data[4];
}
VSC_MC_EVIS_INST;

/* CONV inst case */
typedef union _VSC_MC_CONV_INST
{
    struct
    {
#if !gcdENDIAN_BIG
        gctUINT        baseOpcodeBit0_5      : 6; /* Together with baseOpcodeBit6 to compose base-opcode */
        gctUINT        reserved0             : 2; /* Must be zero'ed */
        gctUINT        bEndOfBB              : 1; /* End Of Basic Block bit for non-control-clow instructions */
        gctUINT        reserved1             : 2; /* Must be zero'ed */
        gctUINT        bResultSat            : 1;
        gctUINT        bDstValid             : 1; /* Must be valid */
        gctUINT        dstRelAddr            : 3;
        gctUINT        dstRegNoBit0_6        : 7; /* Together with dstRegNoBit7 and dstRegNoBit8 to compose dstRegNo */
        gctUINT        writeMask             : 4;
        gctUINT        reserved2             : 5; /* Must be zero'ed */
#else
        gctUINT        reserved2             : 5; /* Must be zero'ed */
        gctUINT        writeMask             : 4;
        gctUINT        dstRegNoBit0_6        : 7; /* Together with dstRegNoBit7 and dstRegNoBit8 to compose dstRegNo */
        gctUINT        dstRelAddr            : 3;
        gctUINT        bDstValid             : 1; /* Must be valid */
        gctUINT        bResultSat            : 1;
        gctUINT        reserved1             : 2; /* Must be zero'ed */
        gctUINT        bEndOfBB              : 1; /* End Of Basic Block bit for non-control-clow instructions */
        gctUINT        reserved0             : 2; /* Must be zero'ed */
        gctUINT        baseOpcodeBit0_5      : 6; /* Together with baseOpcodeBit6 to compose base-opcode */
#endif

#if !gcdENDIAN_BIG
        gctUINT        roundMode             : 2;
        gctUINT        bEvisMode             : 1;
        gctUINT        reserved3             : 4;
        gctUINT        bDstPack              : 1;
        gctUINT        bSrcPack              : 1;
        gctUINT        reserved4             : 2; /* Must be zero'ed */
        gctUINT        bSrc0Valid            : 1; /* Must be valid */
        gctUINT        src0RegNo             : 9;
        gctUINT        instTypeBit0          : 1; /* Together with instTypeBit1_2 to compose instType */
        gctUINT        src0Swizzle           : 8;
        gctUINT        bSrc0ModNeg           : 1;
        gctUINT        bSrc0ModAbs           : 1;
#else
        gctUINT        bSrc0ModAbs           : 1;
        gctUINT        bSrc0ModNeg           : 1;
        gctUINT        src0Swizzle           : 8;
        gctUINT        instTypeBit0          : 1; /* Together with instTypeBit1_2 to compose instType */
        gctUINT        src0RegNo             : 9;
        gctUINT        bSrc0Valid            : 1; /* Must be valid */
        gctUINT        reserved4             : 2; /* Must be zero'ed */
        gctUINT        bSrcPack              : 1;
        gctUINT        bDstPack              : 1;
        gctUINT        reserved3             : 4;
        gctUINT        bEvisMode             : 1;
        gctUINT        roundMode             : 2;
#endif

#if !gcdENDIAN_BIG
        gctUINT        src0RelAddr           : 3;
        gctUINT        src0Type              : 3;
        gctUINT        reserved5             : 10;/* Must be zero'ed */
        gctUINT        baseOpcodeBit6        : 1;
        gctUINT        reserved6             : 13;/* Must be zero'ed */
        gctUINT        instTypeBit1_2        : 2;
#else
        gctUINT        instTypeBit1_2        : 2;
        gctUINT        reserved6             : 13;/* Must be zero'ed */
        gctUINT        baseOpcodeBit6        : 1;
        gctUINT        reserved5             : 10;/* Must be zero'ed */
        gctUINT        src0Type              : 3;
        gctUINT        src0RelAddr           : 3;
#endif

#if !gcdENDIAN_BIG
        gctUINT        reserved7             : 3; /* Must be zero'ed */
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
#else
        gctUINT        dstType               : 1;
        gctUINT        src2Type              : 3;
        gctUINT        src2RelAddr           : 3;
        gctUINT        dstRegNoBit8          : 1;
        gctUINT        bSrc2ModAbs           : 1;
        gctUINT        bSrc2ModNeg           : 1;
        gctUINT        src2Swizzle           : 8;
        gctUINT        dstRegNoBit7          : 1;
        gctUINT        src2RegNo             : 9;
        gctUINT        bSrc2Valid            : 1; /* Must be valid */
        gctUINT        reserved7             : 3; /* Must be zero'ed */
#endif
    } inst;

    gctUINT            data[4];
}
VSC_MC_CONV_INST;

/* complex inst case */
typedef union _VSC_MC_CMPLX_INST
{
    struct
    {
#if !gcdENDIAN_BIG
        gctUINT        baseOpcodeBit0_5      : 6; /* Together with baseOpcodeBit6 to compose base-opcode */
        gctUINT        reserved0             : 2; /* Must be zero'ed */
        gctUINT        bEndOfBB              : 1; /* End Of Basic Block bit for non-control-clow instructions */
        gctUINT        reserved1             : 2; /* Must be zero'ed */
        gctUINT        bResultSat            : 1;
        gctUINT        bDstValid             : 1; /* Must be valid */
        gctUINT        dstRelAddr            : 3;
        gctUINT        dstRegNoBit0_6        : 7; /* Together with dstRegNoBit7 and dstRegNoBit8 to compose dstRegNo */
        gctUINT        writeMask             : 4;
        gctUINT        multInf               : 1;
        gctUINT        reserved2             : 4; /* For load_attr, see NOTE 6; For mad, see NOTE 7; For others, must be zero'ed */
#else
        gctUINT        reserved2             : 4; /* For load_attr, see NOTE 6; For mad, see NOTE 7; For others, must be zero'ed */
        gctUINT        multInf               : 1;
        gctUINT        writeMask             : 4;
        gctUINT        dstRegNoBit0_6        : 7; /* Together with dstRegNoBit7 and dstRegNoBit8 to compose dstRegNo */
        gctUINT        dstRelAddr            : 3;
        gctUINT        bDstValid             : 1; /* Must be valid */
        gctUINT        bResultSat            : 1;
        gctUINT        reserved1             : 2; /* Must be zero'ed */
        gctUINT        bEndOfBB              : 1; /* End Of Basic Block bit for non-control-clow instructions */
        gctUINT        reserved0             : 2; /* Must be zero'ed */
        gctUINT        baseOpcodeBit0_5      : 6; /* Together with baseOpcodeBit6 to compose base-opcode */
#endif

#if !gcdENDIAN_BIG
        gctUINT        roundMode             : 2; /* For atom, bit 0 is used for endian_swap. */
        gctUINT        packMode              : 1;
        gctUINT        subOpcode             : 4; /* For complex, bit 3-6 is subOpcode */
        gctUINT        bSkipForHelperKickoff : 1;
        gctUINT        reserved3             : 2; /* Must be zero'ed for non-atomics. For atomics, see NOTE 8 */
        gctUINT        bDenorm               : 1;
        gctUINT        bSrc0Valid            : 1; /* Must be valid */
        gctUINT        src0RegNo             : 9;
        gctUINT        instTypeBit0          : 1; /* Together with instTypeBit1_2 to compose instType */
        gctUINT        src0Swizzle           : 8;
        gctUINT        bSrc0ModNeg           : 1;
        gctUINT        bSrc0ModAbs           : 1;
#else
        gctUINT        bSrc0ModAbs           : 1;
        gctUINT        bSrc0ModNeg           : 1;
        gctUINT        src0Swizzle           : 8;
        gctUINT        instTypeBit0          : 1; /* Together with instTypeBit1_2 to compose instType */
        gctUINT        src0RegNo             : 9;
        gctUINT        bSrc0Valid            : 1; /* Must be valid */
        gctUINT        bDenorm               : 1;
        gctUINT        reserved3             : 2; /* Must be zero'ed for non-atomics. For atomics, see NOTE 8 */
        gctUINT        bSkipForHelperKickoff : 1;
        gctUINT        subOpcode             : 4; /* For complex, bit 3-6 is subOpcode */
        gctUINT        packMode              : 1;
        gctUINT        roundMode             : 2; /* For atom, bit 0 is used for endian_swap. */
#endif

#if !gcdENDIAN_BIG
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
#else
        gctUINT        instTypeBit1_2        : 2;
        gctUINT        src1RelAddr           : 3;
        gctUINT        bSrc1ModAbs           : 1;
        gctUINT        bSrc1ModNeg           : 1;
        gctUINT        src1Swizzle           : 8;
        gctUINT        baseOpcodeBit6        : 1;
        gctUINT        src1RegNo             : 9;
        gctUINT        bSrc1Valid            : 1; /* Must be valid */
        gctUINT        src0Type              : 3;
        gctUINT        src0RelAddr           : 3;
#endif

#if !gcdENDIAN_BIG
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
#else
        gctUINT        dstType               : 1;
        gctUINT        src2Type              : 3;
        gctUINT        src2RelAddr           : 3;
        gctUINT        dstRegNoBit8          : 1;
        gctUINT        bSrc2ModAbs           : 1;
        gctUINT        bSrc2ModNeg           : 1;
        gctUINT        src2Swizzle           : 8;
        gctUINT        dstRegNoBit7          : 1;
        gctUINT        src2RegNo             : 9;
        gctUINT        bSrc2Valid            : 1; /* Must be valid */
        gctUINT        src1Type              : 3;
#endif
    } inst;

    gctUINT            data[4];
}
VSC_MC_CMPLX_INST;


typedef union _VSC_MC_INST
{
    VSC_MC_RAW_INST                     raw_inst;
    VSC_MC_NO_OPERAND_INST              no_opnd_inst;
    VSC_MC_ALU_3_SRCS_INST              tri_srcs_alu_inst;
    VSC_MC_ALU_3_SRCS_CC_INST           tri_srcs_cc_alu_inst;
    VSC_MC_ALU_2_SRCS_SRC0_SRC1_INST    bin_srcs_src0_src1_alu_inst;
    VSC_MC_ALU_2_SRCS_SRC0_SRC1_CC_INST bin_srcs_src0_src1_alu_cc_inst;
    VSC_MC_ALU_2_SRCS_SRC0_SRC2_INST    bin_srcs_src0_src2_alu_inst;
    VSC_MC_ALU_1_SRC_SRC0_INST          una_src_src0_alu_inst;
    VSC_MC_ALU_1_SRC_SRC1_INST          una_src_src1_alu_inst;
    VSC_MC_ALU_1_SRC_SRC2_INST          una_src_src2_alu_inst;
    VSC_MC_PACK_INST                    pack_inst;
    VSC_MC_SAMPLE_INST                  sample_inst;
    VSC_MC_SAMPLE_EXT_INST              sample_ext_inst;
    VSC_MC_LD_INST                      load_inst;
    VSC_MC_IMG_LD_INST                  img_load_inst;
    VSC_MC_ST_INST                      store_inst;
    VSC_MC_IMG_ST_INST                  img_store_inst;
    VSC_MC_IMG_ATOM_INST                img_atom_inst;
    VSC_MC_STORE_ATTR_INST              store_attr_inst;
    VSC_MC_SELECT_MAP_INST              select_map_inst;
    VSC_MC_DIRECT_BRANCH_0_INST         direct_branch0_inst;
    VSC_MC_DIRECT_BRANCH_1_INST         direct_branch1_inst;
    VSC_MC_INDIRECT_BRANCH_INST         indirect_branch_inst;
    VSC_MC_DIRECT_CALL_INST             direct_call_inst;
    VSC_MC_INDIRECT_CALL_INST           indirect_call_inst;
    VSC_MC_LOOP_INST                    loop_inst;
    VSC_MC_EMIT_INST                    emit_inst;
    VSC_MC_EVIS_INST                    evis_inst;
    VSC_MC_CONV_INST                    conv_inst;
    VSC_MC_CMPLX_INST                   cmplx_inst;
} VSC_MC_INST;

typedef enum _VSC_MC_CODEC_TYPE
{
    VSC_MC_CODEC_TYPE_UNKNOWN                = 0,
    VSC_MC_CODEC_TYPE_NO_OPND,
    VSC_MC_CODEC_TYPE_3_SRCS_ALU,
    VSC_MC_CODEC_TYPE_3_SRCS_CC_ALU,
    VSC_MC_CODEC_TYPE_2_SRCS_SRC0_SRC1_ALU,
    VSC_MC_CODEC_TYPE_2_SRCS_SRC0_SRC1_CC_ALU,
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
    VSC_MC_CODEC_TYPE_CONV,
    VSC_MC_CODEC_TYPE_SCATTER,
    /* Not used now. */
    VSC_MC_CODEC_TYPE_EVIS,
    VSC_MC_CODEC_TYPE_CMPLX,
}VSC_MC_CODEC_TYPE;

#define MC_SRC0_BIT   1
#define MC_SRC1_BIT   2
#define MC_SRC2_BIT   4

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
    VSC_MC_CODEC_INST* pInCodecHelperInst;
    VSC_MC_INST*       pMcInst;

    if (codecMode == VSC_MC_CODEC_MODE_ENCODE)
    {
        pInCodecHelperInst = (VSC_MC_CODEC_INST*)pRefInInst;

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
        pMcInst = (VSC_MC_INST*)pRefInInst;

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
    VSC_MC_CODEC_INST* pInCodecHelperInst;
    VSC_MC_INST*       pMcInst;

    if (codecMode == VSC_MC_CODEC_MODE_ENCODE)
    {
        pInCodecHelperInst = (VSC_MC_CODEC_INST*)pRefInInst;

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
        pMcInst = (VSC_MC_INST*)pRefInInst;

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

static VSC_MC_CODEC_TYPE _GetExtendedOpcodeCodecType(VSC_MC_CODEC* pMcCodec,
                                                     gctUINT baseOpcode,
                                                     gctUINT extOpcode)
{
    if (baseOpcode == 0x7F)
    {
        switch (extOpcode)
        {
        case 0x01:
            return VSC_MC_CODEC_TYPE_EMIT;

        case 0x02:
        case 0x0F:
        case 0x10:
            return VSC_MC_CODEC_TYPE_2_SRCS_SRC0_SRC1_ALU;

        case 0x03:
        case 0x0B:
        case 0x0C:
            return VSC_MC_CODEC_TYPE_1_SRC_SRC0_ALU;

        case 0x04:
        case 0x0D:
        case 0x0E:
            return VSC_MC_CODEC_TYPE_SAMPLE_EXT;

        case 0x13:
            return VSC_MC_CODEC_TYPE_LOAD;
        }
    }
    else if (baseOpcode == 0x45)
    {
        switch (extOpcode)
        {
        case 0x01:
        case 0x06:
            return VSC_MC_CODEC_TYPE_2_SRCS_SRC0_SRC1_ALU;

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
        case 0x0B:
        case 0x16:
        case 0x0A:
        case 0x15:
        case 0x09:
        case 0x14:
        case 0x08:
        case 0x13:
        case 0x12:
            return VSC_MC_CODEC_TYPE_3_SRCS_ALU;
        case 0x17:
            return VSC_MC_CODEC_TYPE_3_SRCS_ALU;
        case 0x18:
        case 0x19:
        case 0x1A:
            return VSC_MC_CODEC_TYPE_3_SRCS_ALU;
        case 0x1B:
        case 0x1C:
        case 0x1D:
            return VSC_MC_CODEC_TYPE_1_SRC_SRC0_ALU;
        case 0x1E:
            return VSC_MC_CODEC_TYPE_2_SRCS_SRC0_SRC1_ALU;
        case 0x1F:
        case MC_AUXILIARY_OP_CODE_USC_SCATTER:
            return VSC_MC_CODEC_TYPE_SCATTER;
        case 0x20:
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
        return VSC_MC_CODEC_TYPE_3_SRCS_CC_ALU;
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
    case 0x60:
    case 0x54:
    case 0x55:
    case 0x78:
    case 0x63:
        return VSC_MC_CODEC_TYPE_3_SRCS_ALU;
    case 0x36:
        {
            if (codecMode == VSC_MC_CODEC_MODE_ENCODE)
            {
                /* src2 of CLAMP0_MAX is optional */
                VSC_MC_CODEC_INST* pInstHelper = (VSC_MC_CODEC_INST *)pRefInInst;
                return pInstHelper->srcCount == 3 ? VSC_MC_CODEC_TYPE_3_SRCS_ALU
                                                  : VSC_MC_CODEC_TYPE_2_SRCS_SRC0_SRC1_ALU;
            }
            else
            {
                VSC_MC_INST* pMcInst = (VSC_MC_INST*)pRefInInst;
                return (pMcInst->tri_srcs_alu_inst.inst.bSrc2Valid) ? VSC_MC_CODEC_TYPE_3_SRCS_ALU
                                                                    : VSC_MC_CODEC_TYPE_2_SRCS_SRC0_SRC1_ALU;
            }
        }
    case 0x10:
    case 0x17:  /* A special one that has no dst valid bit */
        return VSC_MC_CODEC_TYPE_2_SRCS_SRC0_SRC1_CC_ALU;
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
    case 0x6F:
    case MC_AUXILIARY_OP_CODE_TEXLD_LOD:        /* From 0x6F */
    case MC_AUXILIARY_OP_CODE_TEXLD_LOD_PCF:    /* From 0x6F */
    case 0x18:
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
    case 0x7D:
    case MC_AUXILIARY_OP_CODE_TEXLD_GATHER:     /* From 0x7D */
    case MC_AUXILIARY_OP_CODE_TEXLD_GATHER_PCF: /* From 0x7D */
    case 0x4B:
    case 0x49:
    case 0x4A:
        return VSC_MC_CODEC_TYPE_SAMPLE;

    case 0x32:
    case 0x39:
        return VSC_MC_CODEC_TYPE_LOAD;

    case 0x79:
    case 0x34:
    case 0x37:
    case 0x38:
        return VSC_MC_CODEC_TYPE_IMG_LOAD;

    case 0x33:
    case MC_AUXILIARY_OP_CODE_USC_STORE:
    case 0x3A:
    case MC_AUXILIARY_OP_CODE_USC_STOREP:
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
        return _GetExtendedOpcodeCodecType(pMcCodec, baseOpcode, extOpcode);

    case 0x72:
        return VSC_MC_CODEC_TYPE_CONV;

    case 0x62:
        return VSC_MC_CODEC_TYPE_CMPLX;
    }

    return VSC_MC_CODEC_TYPE_UNKNOWN;
}

static gctUINT _MapSampleAuxOpcodeToHwOpcode(VSC_MC_CODEC* pMcCodec, gctUINT auxOpcode)
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

static gctUINT _MapSampleHwOpcodeToAuxOpcode(VSC_MC_CODEC* pMcCodec,
                                             gctUINT sampleHwOpcode,
                                             gctUINT srcIdx1And2MaskOfMc,
                                             gctUINT texldUModeReg)
{
    gctUINT texldUMode;

    if (sampleHwOpcode == 0x6F)
    {
        if (srcIdx1And2MaskOfMc & (1 << 2))
        {
            return MC_AUXILIARY_OP_CODE_TEXLD_LOD_PCF;
        }
        else if (srcIdx1And2MaskOfMc & (1 << 1))
        {
            return MC_AUXILIARY_OP_CODE_TEXLD_LOD;
        }
    }
    else if (sampleHwOpcode == 0x18)
    {
        if (srcIdx1And2MaskOfMc & (1 << 1) && srcIdx1And2MaskOfMc & (1 << 2))
        {
            return MC_AUXILIARY_OP_CODE_TEXLD_BIAS_PCF;
        }
        else if (srcIdx1And2MaskOfMc & (1 << 1))
        {
            return MC_AUXILIARY_OP_CODE_TEXLD_BIAS;
        }
        else if (srcIdx1And2MaskOfMc & (1 << 2))
        {
            return MC_AUXILIARY_OP_CODE_TEXLD_PCF;
        }
        else if (srcIdx1And2MaskOfMc == 0)
        {
            return MC_AUXILIARY_OP_CODE_TEXLD_PLAIN;
        }
    }
    else if (sampleHwOpcode == 0x7B /* 0x7B */)
    {
        if (pMcCodec->pHwCfg->hwFeatureFlags.hasUniversalTexldV2)
        {
            if (srcIdx1And2MaskOfMc & (1 << 1))
            {
                return MC_AUXILIARY_OP_CODE_TEXLD_U_F_B_BIAS;
            }
            else if (srcIdx1And2MaskOfMc & (1 << 2))
            {
                return MC_AUXILIARY_OP_CODE_TEXLD_U_F_B_PLAIN;
            }
        }
        else
        {
            texldUMode = (texldUModeReg >> 12) & 0x3;

            if (srcIdx1And2MaskOfMc & (1 << 1))
            {
                if (texldUMode == 0x1)
                {
                    return MC_AUXILIARY_OP_CODE_TEXLD_U_LOD;
                }
                else if (texldUMode == 0x2)
                {
                    return MC_AUXILIARY_OP_CODE_TEXLD_U_BIAS;
                }
            }
            else if (srcIdx1And2MaskOfMc & (1 << 2))
            {
                gcmASSERT(texldUMode == 0x0);
                return MC_AUXILIARY_OP_CODE_TEXLD_U_PLAIN;
            }
        }
    }
    else if (sampleHwOpcode == 0x7D)
    {
        if (srcIdx1And2MaskOfMc & (1 << 2))
        {
            return MC_AUXILIARY_OP_CODE_TEXLD_GATHER_PCF;
        }
        else if (srcIdx1And2MaskOfMc & (1 << 1))
        {
            return MC_AUXILIARY_OP_CODE_TEXLD_GATHER;
        }
    }

    return sampleHwOpcode;
}

static gctUINT _MapLdStAuxOpcodeToHwOpcode(gctUINT auxOpcode)
{
    if (auxOpcode == MC_AUXILIARY_OP_CODE_USC_STORE)
    {
        return 0x33;
    }

    if (auxOpcode == MC_AUXILIARY_OP_CODE_USC_STOREP)
    {
        return 0x3A;
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

static gctUINT _MapLdStHwOpcodeToAuxOpcode(gctUINT ldStHwOpcode, gctBOOL bDstEnabled)
{
    if (bDstEnabled)
    {
        if (ldStHwOpcode == 0x33)
        {
            return MC_AUXILIARY_OP_CODE_USC_STORE;
        }
        if (ldStHwOpcode == 0x3A)
        {
            return MC_AUXILIARY_OP_CODE_USC_STOREP;
        }
        else if (ldStHwOpcode == 0x7A)
        {
            return MC_AUXILIARY_OP_CODE_USC_IMG_STORE;
        }
        else if (ldStHwOpcode == 0x35)
        {
            return MC_AUXILIARY_OP_CODE_USC_IMG_STORE_3D;
        }
        else if (ldStHwOpcode == 0x42)
        {
            return MC_AUXILIARY_OP_CODE_USC_STORE_ATTR;
        }
    }

    return ldStHwOpcode;
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

static gctUINT _DecodeExtendedOpcode(VSC_MC_INST* pMcInst, gctUINT baseOpcode)
{
    gctUINT extOpcode = NOT_ASSIGNED;

    if (baseOpcode == 0x7F)
    {
        extOpcode = pMcInst->no_opnd_inst.inst.extOpcode;
    }
    else if (baseOpcode == 0x45)
    {
        extOpcode = pMcInst->evis_inst.inst.extEvisOpCodeBit0_2 |
                    (pMcInst->evis_inst.inst.extEvisOpCodeBit3 << 3) |
                    (pMcInst->evis_inst.inst.extEvisOpCodeBit4_5 << 4);
    }

    return extOpcode;
}

#define ENCODE_BASE_OPCODE(pMcInst, baseOpcode)                                               \
        (pMcInst)->no_opnd_inst.inst.baseOpcodeBit0_5 = ((baseOpcode) & 0x3F);                \
        (pMcInst)->no_opnd_inst.inst.baseOpcodeBit6   = (((baseOpcode) >> 6) & 0x01);

#define DECODE_BASE_OPCODE(pMcInst)                                                           \
        ((pMcInst)->no_opnd_inst.inst.baseOpcodeBit0_5 |                                      \
         ((pMcInst)->no_opnd_inst.inst.baseOpcodeBit6 << 6))

#define ENCODE_EXT_OPCODE(pMcInst, baseOpcode, extOpcode)                                     \
        if ((baseOpcode) == 0x7F || (baseOpcode) == 0x45) \
        {                                                                                     \
            _EncodeExtendedOpcode((baseOpcode), (extOpcode), (VSC_MC_INST*)(pMcInst));        \
        }

#define DECODE_EXT_OPCODE(pMcInst, baseOpcode)                                                \
        _DecodeExtendedOpcode((VSC_MC_INST*)(pMcInst), (baseOpcode));

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

static VSC_MC_CODEC_IMM_VALUE _Conver20BitImm_2_32BitImm(gctUINT raw20BitImmValue,
                                                         gctUINT immType)
{
    VSC_MC_CODEC_IMM_VALUE immValue;

    immValue.f = 0;

    switch (immType)
    {
    case 0x0:
        immValue.ui = vscCvtS11E8FloatToS23E8Float(raw20BitImmValue);
        break;

    case 0x1:
        immValue.si = raw20BitImmValue;
        break;

    case 0x2:
    case 0x3:
        immValue.ui = raw20BitImmValue;
    }

    return immValue;
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

static VSC_MC_CODEC_IMM_VALUE _DecodeImmData(VSC_MC_INST* pMcInst,
                                             gctUINT srcIdx,
                                             gctUINT* pImmType)
{
    gctUINT                raw20BitImmValue = 0;

    gcmASSERT(srcIdx < 3);

    if (srcIdx == 0)
    {
        raw20BitImmValue = pMcInst->tri_srcs_alu_inst.inst.src0RegNo;
        raw20BitImmValue |= (pMcInst->tri_srcs_alu_inst.inst.src0Swizzle << 9);
        raw20BitImmValue |= (pMcInst->tri_srcs_alu_inst.inst.bSrc0ModNeg << 17);
        raw20BitImmValue |= (pMcInst->tri_srcs_alu_inst.inst.bSrc0ModAbs << 18);
        raw20BitImmValue |= ((pMcInst->tri_srcs_alu_inst.inst.src0RelAddr & 0x01) << 19);

        *pImmType = (pMcInst->tri_srcs_alu_inst.inst.src0RelAddr >> 1);
    }
    else if (srcIdx == 1)
    {
        raw20BitImmValue = pMcInst->tri_srcs_alu_inst.inst.src1RegNo;
        raw20BitImmValue |= (pMcInst->tri_srcs_alu_inst.inst.src1Swizzle << 9);
        raw20BitImmValue |= (pMcInst->tri_srcs_alu_inst.inst.bSrc1ModNeg << 17);
        raw20BitImmValue |= (pMcInst->tri_srcs_alu_inst.inst.bSrc1ModAbs << 18);
        raw20BitImmValue |= ((pMcInst->tri_srcs_alu_inst.inst.src1RelAddr & 0x01) << 19);

        *pImmType = (pMcInst->tri_srcs_alu_inst.inst.src1RelAddr >> 1);
    }
    else
    {
        raw20BitImmValue = pMcInst->tri_srcs_alu_inst.inst.src2RegNo;
        raw20BitImmValue |= (pMcInst->tri_srcs_alu_inst.inst.src2Swizzle << 9);
        raw20BitImmValue |= (pMcInst->tri_srcs_alu_inst.inst.bSrc2ModNeg << 17);
        raw20BitImmValue |= (pMcInst->tri_srcs_alu_inst.inst.bSrc2ModAbs << 18);
        raw20BitImmValue |= ((pMcInst->tri_srcs_alu_inst.inst.src2RelAddr & 0x01) << 19);

        *pImmType = (pMcInst->tri_srcs_alu_inst.inst.src2RelAddr >> 1);
    }

    return _Conver20BitImm_2_32BitImm(raw20BitImmValue, *pImmType);
}

static void _EncodeDst(VSC_MC_CODEC* pMcCodec,
                       VSC_MC_CODEC_DST* pMcCodecDst,
                       gctBOOL bEvisMode,
                       VSC_MC_INST* pMcInst)
{
    pMcInst->tri_srcs_alu_inst.inst.bDstValid = gcvTRUE;
    pMcInst->tri_srcs_alu_inst.inst.dstType = pMcCodecDst->regType;

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

static gctBOOL _DecodeDst(VSC_MC_CODEC* pMcCodec,
                          VSC_MC_INST* pMcInst,
                          gctBOOL bEvisMode,
                          VSC_MC_CODEC_DST* pMcCodecDst)
{
    if (!pMcInst->tri_srcs_alu_inst.inst.bDstValid)
    {
        return gcvFALSE;
    }

    pMcCodecDst->regType = pMcInst->tri_srcs_alu_inst.inst.dstType;

    if (bEvisMode)
    {
        pMcCodecDst->u.evisDst.startCompIdx = pMcInst->evis_inst.inst.startDstCompIdx;
        pMcCodecDst->u.evisDst.compIdxRange = pMcInst->evis_inst.inst.endDstCompIdx -
                                              pMcInst->evis_inst.inst.startDstCompIdx + 1;
    }
    else
    {
        pMcCodecDst->u.nmlDst.indexingAddr = pMcInst->tri_srcs_alu_inst.inst.dstRelAddr;
        pMcCodecDst->u.nmlDst.writeMask = pMcInst->tri_srcs_alu_inst.inst.writeMask;
    }

    if (pMcCodec->bDual16ModeEnabled)
    {
        pMcCodecDst->regNo = pMcInst->tri_srcs_alu_inst.inst.dstRegNoBit0_6;
    }
    else
    {
        pMcCodecDst->regNo = pMcInst->tri_srcs_alu_inst.inst.dstRegNoBit0_6;
        pMcCodecDst->regNo |= (pMcInst->tri_srcs_alu_inst.inst.dstRegNoBit7 << 7);
        pMcCodecDst->regNo |= (pMcInst->tri_srcs_alu_inst.inst.dstRegNoBit8 << 8);
    }

    return gcvTRUE;
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

static gctBOOL _DecodeSrc(VSC_MC_CODEC_INST* pOutCodecHelperInst,
                          VSC_MC_CODEC* pMcCodec,
                          gctUINT srcIdx,
                          VSC_MC_INST* pMcInst,
                          gctBOOL bEvisMode,
                          VSC_MC_CODEC_SRC* pMcCodecSrc)
{
    if (srcIdx == 0)
    {
        if (!pMcInst->tri_srcs_alu_inst.inst.bSrc0Valid)
        {
            return gcvFALSE;
        }

        pMcCodecSrc->regType = pMcInst->tri_srcs_alu_inst.inst.src0Type;

        if (pMcCodecSrc->regType != 0x7)
        {
            pMcCodecSrc->u.reg.regNo = pMcInst->tri_srcs_alu_inst.inst.src0RegNo;
            pMcCodecSrc->u.reg.indexingAddr = pMcInst->tri_srcs_alu_inst.inst.src0RelAddr;

            if (!bEvisMode)
            {
                pMcCodecSrc->u.reg.swizzle = pMcInst->tri_srcs_alu_inst.inst.src0Swizzle;
                pMcCodecSrc->u.reg.bAbs = pMcInst->tri_srcs_alu_inst.inst.bSrc0ModAbs;
                pMcCodecSrc->u.reg.bNegative = pMcInst->tri_srcs_alu_inst.inst.bSrc0ModNeg;
            }
        }
    }
    else if (srcIdx == 1)
    {
        if (!pMcInst->tri_srcs_alu_inst.inst.bSrc1Valid)
        {
            return gcvFALSE;
        }

        pMcCodecSrc->regType = pMcInst->tri_srcs_alu_inst.inst.src1Type;

        if (pMcCodecSrc->regType != 0x7)
        {
            pMcCodecSrc->u.reg.regNo = pMcInst->tri_srcs_alu_inst.inst.src1RegNo;
            pMcCodecSrc->u.reg.swizzle = pMcInst->tri_srcs_alu_inst.inst.src1Swizzle;
            pMcCodecSrc->u.reg.indexingAddr = pMcInst->tri_srcs_alu_inst.inst.src1RelAddr;

            if (!bEvisMode)
            {
                pMcCodecSrc->u.reg.bAbs = pMcInst->tri_srcs_alu_inst.inst.bSrc1ModAbs;
                pMcCodecSrc->u.reg.bNegative = pMcInst->tri_srcs_alu_inst.inst.bSrc1ModNeg;
            }
        }
    }
    else if (srcIdx == 2)
    {
        if (!pMcInst->tri_srcs_alu_inst.inst.bSrc2Valid)
        {
            return gcvFALSE;
        }

        pMcCodecSrc->regType = pMcInst->tri_srcs_alu_inst.inst.src2Type;

        if (pMcCodecSrc->regType != 0x7)
        {
            pMcCodecSrc->u.reg.regNo = pMcInst->tri_srcs_alu_inst.inst.src2RegNo;
            pMcCodecSrc->u.reg.swizzle = pMcInst->tri_srcs_alu_inst.inst.src2Swizzle;
            pMcCodecSrc->u.reg.indexingAddr = pMcInst->tri_srcs_alu_inst.inst.src2RelAddr;

            if (!bEvisMode)
            {
                pMcCodecSrc->u.reg.bAbs = pMcInst->tri_srcs_alu_inst.inst.bSrc2ModAbs;
                pMcCodecSrc->u.reg.bNegative = pMcInst->tri_srcs_alu_inst.inst.bSrc2ModNeg;
            }

            if (pOutCodecHelperInst->extOpcode == 0x08     ||
                pOutCodecHelperInst->extOpcode == 0x09      ||
                pOutCodecHelperInst->extOpcode == 0x0A      ||
                pOutCodecHelperInst->extOpcode == 0x0B      ||
                pOutCodecHelperInst->extOpcode == 0x12     ||
                pOutCodecHelperInst->extOpcode == 0x13     ||
                pOutCodecHelperInst->extOpcode == 0x14      ||
                pOutCodecHelperInst->extOpcode == 0x15      ||
                pOutCodecHelperInst->extOpcode == 0x16)
            {
                gcmASSERT(pMcCodecSrc->regType == 0x4);
                pMcCodecSrc->u.reg.bConstReg = gcvTRUE;
            }
        }
    }

    if (pMcCodecSrc->regType == 0x7)
    {
        pMcCodecSrc->u.imm.immData = _DecodeImmData(pMcInst, srcIdx, &pMcCodecSrc->u.imm.immType);
    }
    else if (pMcCodecSrc->regType == 0x2)
    {
        pMcCodecSrc->u.reg.bConstReg = gcvTRUE;
    }

    return gcvTRUE;
}

static gctBOOL _DecodeSrcWrapper(VSC_MC_CODEC_INST* pOutCodecHelperInst,
                                 VSC_MC_CODEC* pMcCodec,
                                 gctUINT* pSrcIdx,
                                 gctUINT  expectedSrcIdxMask,
                                 VSC_MC_INST* pMcInst,
                                 gctBOOL bEvisMode,
                                 VSC_MC_CODEC_SRC* pMcCodecSrc)
{
    /* Skip untouched srcs */
    while (*pSrcIdx < 3)
    {
        if (*pSrcIdx == 0 && pMcInst->tri_srcs_alu_inst.inst.bSrc0Valid && (expectedSrcIdxMask & MC_SRC0_BIT))
        {
            break;
        }
        else if (*pSrcIdx == 1 && pMcInst->tri_srcs_alu_inst.inst.bSrc1Valid && (expectedSrcIdxMask & MC_SRC1_BIT))
        {
            break;
        }
        else if (*pSrcIdx == 2 && pMcInst->tri_srcs_alu_inst.inst.bSrc2Valid && (expectedSrcIdxMask & MC_SRC2_BIT))
        {
            break;
        }

        (*pSrcIdx) ++;
    }

    /* No available srcs any more */
    if (*pSrcIdx >= 3)
    {
        return gcvFALSE;
    }

    /* Decode now */
    if (!_DecodeSrc(pOutCodecHelperInst, pMcCodec, *pSrcIdx, pMcInst, bEvisMode, pMcCodecSrc))
    {
        return gcvFALSE;
    }

    /* Move on to next src for next iteration */
    (*pSrcIdx) ++;

    return gcvTRUE;
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

static gctUINT _DecodeInstType(VSC_MC_CODEC* pMcCodec,
                               VSC_MC_CODEC_TYPE mcCodecType,
                               VSC_MC_INST* pMcInst)
{
    gctUINT     instType;

    instType = pMcInst->tri_srcs_alu_inst.inst.instTypeBit0;
    instType |= (pMcInst->tri_srcs_alu_inst.inst.instTypeBit1_2 << 1);

    if (mcCodecType == VSC_MC_CODEC_TYPE_LOAD ||
        mcCodecType == VSC_MC_CODEC_TYPE_STORE)
    {
        /* There is an extra 4th bit for inst type */
        instType |= (pMcInst->load_inst.inst.instTypeBit3 << 3);
    }

    return instType;
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

static gctUINT _DecodeThreadType(VSC_MC_CODEC* pMcCodec,
                                 VSC_MC_CODEC_TYPE mcCodecType,
                                 VSC_MC_INST* pMcInst)
{
    gctUINT    threadType = 0;

    if (pMcCodec->bDual16ModeEnabled)
    {
        if (mcCodecType == VSC_MC_CODEC_TYPE_STORE ||
            mcCodecType == VSC_MC_CODEC_TYPE_IMG_STORE ||
            mcCodecType == VSC_MC_CODEC_TYPE_STORE_ATTR ||
            mcCodecType == VSC_MC_CODEC_TYPE_DIRECT_BRANCH_1 ||
            mcCodecType == VSC_MC_CODEC_TYPE_INDIRECT_BRANCH ||
            mcCodecType == VSC_MC_CODEC_TYPE_INDIRECT_CALL)
        {
            threadType = pMcInst->store_inst.inst.threadTypeBit0;
            threadType |= (pMcInst->store_inst.inst.threadTypeBit1 << 1);
        }
        else
        {
            threadType = pMcInst->tri_srcs_alu_inst.inst.dstRegNoBit7;
            threadType |= (pMcInst->tri_srcs_alu_inst.inst.dstRegNoBit8 << 1);
        }
    }

    return threadType;
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
    pOutMcInst->tri_srcs_alu_inst.inst.bResultSat = pInCodecHelperInst->instCtrl.bResultSat;
    if (bEvisMode)
    {
        /* EVIS insts have evis-state and sourceBin */
        ((VSC_MC_INST*)pOutMcInst)->evis_inst.inst.evisState = pInCodecHelperInst->instCtrl.u.visionCtrl.evisState;
        ((VSC_MC_INST*)pOutMcInst)->evis_inst.inst.startSrcCompIdx = pInCodecHelperInst->instCtrl.u.visionCtrl.startSrcCompIdx;

    }
    else
    {
        pOutMcInst->tri_srcs_alu_inst.inst.roundMode = pInCodecHelperInst->instCtrl.roundingMode;

        /*
        ** For CONV instruction, bit:2 of word1 is saved the evis mode, not packMode, and we use data type to check if it is packMode.
        ** Right now, it is disabled.
        */
        if (pInCodecHelperInst->baseOpcode == 0x72)
        {
            ((VSC_MC_INST*)pOutMcInst)->evis_inst.inst.evisState &= 0x1FE;
        }
        else
        {
            pOutMcInst->tri_srcs_alu_inst.inst.packMode = pInCodecHelperInst->instCtrl.packMode;
        }
    }

    if (pMcCodec->pHwCfg->hwFeatureFlags.supportEndOfBBReissue)
    {
        /* set End Of Basic Block Bit For non-control-flow instructions */
        if (mcCodecType == VSC_MC_CODEC_TYPE_3_SRCS_CC_ALU)
        {
            pOutMcInst->tri_srcs_cc_alu_inst.inst.bEndOfBB = pInCodecHelperInst->instCtrl.bEndOfBB;
        }
        else if (mcCodecType == VSC_MC_CODEC_TYPE_2_SRCS_SRC0_SRC1_CC_ALU)
        {
            pOutMcInst->bin_srcs_src0_src1_alu_cc_inst.inst.bEndOfBB = pInCodecHelperInst->instCtrl.bEndOfBB;
        }
        else
        {
            /* use bit 8 of word0 for all other inst kind */
            pOutMcInst->tri_srcs_alu_inst.inst.bEndOfBB = pInCodecHelperInst->instCtrl.bEndOfBB;
        }
    }

    return gcvTRUE;
}

static gctBOOL _Common_Decode_Mc_Alu_Inst(VSC_MC_CODEC* pMcCodec,
                                          VSC_MC_CODEC_TYPE mcCodecType,
                                          VSC_MC_INST* pInMcInst,
                                          gctUINT expectedMcSrcIdxMask,
                                          VSC_MC_CODEC_INST* pOutCodecHelperInst)
{
    gctUINT           srcIdxOfHelperInst, srcIdxOfMc = 0;
    gctBOOL           bEvisMode;

    /* Opcode */
    pOutCodecHelperInst->baseOpcode = DECODE_BASE_OPCODE(pInMcInst);

    bEvisMode = (pOutCodecHelperInst->baseOpcode == 0x45);

    /* Dst */
    pOutCodecHelperInst->bDstValid = _DecodeDst(pMcCodec, pInMcInst, bEvisMode, &pOutCodecHelperInst->dst);

    /* Src */
    for (srcIdxOfHelperInst = 0; ; srcIdxOfHelperInst ++)
    {
        if (!_DecodeSrcWrapper(pOutCodecHelperInst, pMcCodec, &srcIdxOfMc, expectedMcSrcIdxMask, pInMcInst,
                               bEvisMode, &pOutCodecHelperInst->src[srcIdxOfHelperInst]))
        {
            break;
        }

        pOutCodecHelperInst->srcCount = srcIdxOfHelperInst + 1;

        if (pOutCodecHelperInst->src[srcIdxOfHelperInst].regType == 0x4)
        {
            pOutCodecHelperInst->instCtrl.u.visionCtrl.bUseUniform512 = gcvTRUE;
        }
    }

    /* Inst ctrl */
    pOutCodecHelperInst->instCtrl.instType = _DecodeInstType(pMcCodec, mcCodecType, pInMcInst);
    pOutCodecHelperInst->instCtrl.threadType = _DecodeThreadType(pMcCodec, mcCodecType, pInMcInst);
    pOutCodecHelperInst->instCtrl.bResultSat = pInMcInst->tri_srcs_alu_inst.inst.bResultSat;
    if (bEvisMode)
    {
        /* EVIS insts have evis-state and sourceBin */
        pOutCodecHelperInst->instCtrl.u.visionCtrl.evisState = pInMcInst->evis_inst.inst.evisState;
        pOutCodecHelperInst->instCtrl.u.visionCtrl.startSrcCompIdx = pInMcInst->evis_inst.inst.startSrcCompIdx;

    }
    else
    {
        pOutCodecHelperInst->instCtrl.roundingMode = pInMcInst->tri_srcs_alu_inst.inst.roundMode;

        /*
        ** For CONV instruction, bit:2 of word1 is saved the evis mode, not packMode, and we use data type to check if it is packMode.
        */
        if (pOutCodecHelperInst->baseOpcode == 0x72)
        {
            pOutCodecHelperInst->instCtrl.u.visionCtrl.evisState = pInMcInst->evis_inst.inst.evisState;
        }
        else
        {
            pOutCodecHelperInst->instCtrl.packMode = pInMcInst->tri_srcs_alu_inst.inst.packMode;
        }
    }

    if (pMcCodec->pHwCfg->hwFeatureFlags.supportEndOfBBReissue)
    {
        /* set End Of Basic Block Bit For non-control-flow instructions */
        if (mcCodecType == VSC_MC_CODEC_TYPE_3_SRCS_CC_ALU)
        {
            pOutCodecHelperInst->instCtrl.bEndOfBB = pInMcInst->tri_srcs_cc_alu_inst.inst.bEndOfBB;
        }
        else if (mcCodecType == VSC_MC_CODEC_TYPE_2_SRCS_SRC0_SRC1_CC_ALU)
        {
            pOutCodecHelperInst->instCtrl.bEndOfBB = pInMcInst->bin_srcs_src0_src1_alu_cc_inst.inst.bEndOfBB;
        }
        else
        {
            /* use bit 8 of word0 for all other inst kind */
            pOutCodecHelperInst->instCtrl.bEndOfBB = pInMcInst->tri_srcs_alu_inst.inst.bEndOfBB;
        }
    }

    return gcvTRUE;
}

static gctBOOL _Common_Encode_Mc_Sample_Inst(VSC_MC_CODEC* pMcCodec,
                                             VSC_MC_CODEC_TYPE mcCodecType,
                                             VSC_MC_CODEC_INST* pInCodecHelperInst,
                                             VSC_MC_INST* pOutMcInst)
{
    gctUINT  baseOpcode = _MapSampleAuxOpcodeToHwOpcode(pMcCodec, pInCodecHelperInst->baseOpcode);

    /* Opcode */
    ENCODE_BASE_OPCODE(pOutMcInst, baseOpcode);

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
    pOutMcInst->sample_inst.inst.bResultSat = pInCodecHelperInst->instCtrl.bResultSat;
    if (pMcCodec->pHwCfg->hwFeatureFlags.supportEndOfBBReissue)
    {
        pOutMcInst->sample_inst.inst.bEndOfBB = pInCodecHelperInst->instCtrl.bEndOfBB;
    }

    return gcvTRUE;
}

static gctBOOL _Common_Decode_Mc_Sample_Inst(VSC_MC_CODEC* pMcCodec,
                                             VSC_MC_CODEC_TYPE mcCodecType,
                                             VSC_MC_INST* pInMcInst,
                                             gctUINT expectedMcSrcIdxMask,
                                             VSC_MC_CODEC_INST* pOutCodecHelperInst)
{
    gctUINT           baseOpcode;
    gctUINT           srcIdxOfHelperInst, srcIdxOfMc = 0;
    gctUINT           srcIdx1And2MaskOfMc = 0; /* Mask srcIdx == 1 and srcIdx == 2 */
    gctUINT           texldUModeReg = 0;

    /* Opcode */
    baseOpcode = DECODE_BASE_OPCODE(pInMcInst);

    /* Dst */
    pOutCodecHelperInst->bDstValid = _DecodeDst(pMcCodec, pInMcInst, gcvFALSE, &pOutCodecHelperInst->dst);

    /* Src */
    pOutCodecHelperInst->src[0].regType = MC_AUXILIARY_SRC_TYPE_SAMPLER;
    pOutCodecHelperInst->src[0].u.reg.regNo = pInMcInst->sample_inst.inst.samplerSlot;
    pOutCodecHelperInst->src[0].u.reg.swizzle = pInMcInst->sample_inst.inst.samplerSwizzle;
    pOutCodecHelperInst->src[0].u.reg.indexingAddr = pInMcInst->sample_inst.inst.samplerRelAddr;
    pOutCodecHelperInst->srcCount = 1;

    _DecodeSrcWrapper(pOutCodecHelperInst, pMcCodec, &srcIdxOfMc, expectedMcSrcIdxMask, pInMcInst, gcvFALSE, &pOutCodecHelperInst->src[1]);
    pOutCodecHelperInst->srcCount ++;

    for (srcIdxOfHelperInst = 2; ; srcIdxOfHelperInst ++)
    {
        if (!_DecodeSrcWrapper(pOutCodecHelperInst, pMcCodec, &srcIdxOfMc, expectedMcSrcIdxMask, pInMcInst,
                               gcvFALSE, &pOutCodecHelperInst->src[srcIdxOfHelperInst]))
        {
            break;
        }

        if (baseOpcode == 0x7B && (srcIdxOfMc - 1) == 2)
        {
            gcmASSERT(pOutCodecHelperInst->src[srcIdxOfHelperInst].regType == 0x7);

            texldUModeReg = pOutCodecHelperInst->src[srcIdxOfHelperInst].u.imm.immData.ui;
        }

        srcIdx1And2MaskOfMc |= (1 << (srcIdxOfMc - 1));
        pOutCodecHelperInst->srcCount = srcIdxOfHelperInst + 1;
    }

    /* Inst ctrl */
    pOutCodecHelperInst->instCtrl.instType = _DecodeInstType(pMcCodec, mcCodecType, pInMcInst);
    pOutCodecHelperInst->instCtrl.threadType = _DecodeThreadType(pMcCodec, mcCodecType, pInMcInst);
    pOutCodecHelperInst->instCtrl.bResultSat = pInMcInst->sample_inst.inst.bResultSat;
    if (pMcCodec->pHwCfg->hwFeatureFlags.supportEndOfBBReissue)
    {
        pOutCodecHelperInst->instCtrl.bEndOfBB = pInMcInst->sample_inst.inst.bEndOfBB;
    }

    /* Map sample hw-opcode to sample aux-opcode based on different src-idx */
    pOutCodecHelperInst->baseOpcode = _MapSampleHwOpcodeToAuxOpcode(pMcCodec, baseOpcode, srcIdx1And2MaskOfMc, texldUModeReg);

    return gcvTRUE;
}

static gctBOOL _Common_Encode_Mc_Load_Store_Inst(VSC_MC_CODEC* pMcCodec,
                                                 VSC_MC_CODEC_TYPE mcCodecType,
                                                 VSC_MC_CODEC_INST* pInCodecHelperInst,
                                                 gctBOOL bForImgLS,
                                                 gctBOOL bIsImgAtom,
                                                 VSC_MC_INST* pOutMcInst)
{
    gctUINT           srcIdx, baseOpcode = _MapLdStAuxOpcodeToHwOpcode(pInCodecHelperInst->baseOpcode);
    gctBOOL           bEvisMode = pInCodecHelperInst->instCtrl.u.maCtrl.bUnderEvisMode;

    /* Opcode */
    ENCODE_BASE_OPCODE(pOutMcInst, baseOpcode);

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

    if (!bIsImgAtom)
    {
        pOutMcInst->load_inst.inst.bDenorm = pInCodecHelperInst->instCtrl.bDenorm;
    }

    if (!bForImgLS)
    {
        gcmASSERT(!bEvisMode);

        pOutMcInst->load_inst.inst.packMode = pInCodecHelperInst->instCtrl.packMode;
        pOutMcInst->load_inst.inst.bOffsetX3 = pInCodecHelperInst->instCtrl.u.maCtrl.u.lsCtrl.bOffsetX3;
        pOutMcInst->load_inst.inst.offsetLeftShift = pInCodecHelperInst->instCtrl.u.maCtrl.u.lsCtrl.offsetLeftShift;
    }
    else if (bEvisMode)
    {
        /* Always mark LSB1 of evis-state to be enabled */
        ((VSC_MC_INST*)pOutMcInst)->evis_inst.inst.evisState = 0x01;
    }

    if (bIsImgAtom)
    {
        ((VSC_MC_INST*)pOutMcInst)->img_atom_inst.inst.atomicMode = pInCodecHelperInst->instCtrl.u.maCtrl.u.imgAtomCtrl.atomicMode;
        ((VSC_MC_INST*)pOutMcInst)->img_atom_inst.inst.b3dImgMode = pInCodecHelperInst->instCtrl.u.maCtrl.u.imgAtomCtrl.b3dImgMode;
    }

    _EncodeInstType(pMcCodec, mcCodecType, pOutMcInst, pInCodecHelperInst->instCtrl.instType);
    _EncodeThreadType(pMcCodec, mcCodecType, pOutMcInst, pInCodecHelperInst->instCtrl.threadType);

    if (pInCodecHelperInst->bDstValid || bForImgLS)
    {
        pOutMcInst->load_inst.inst.bResultSat = pInCodecHelperInst->instCtrl.bResultSat;
    }

    /* set USC Unallocate Bit For Global Memory Load/Store Instructions */
    if (pMcCodec->pHwCfg->hwFeatureFlags.supportUSCUnalloc &&
        pInCodecHelperInst->instCtrl.u.maCtrl.bUnallocate &&
        (mcCodecType == VSC_MC_CODEC_TYPE_STORE ||
         mcCodecType == VSC_MC_CODEC_TYPE_IMG_STORE ||
         mcCodecType == VSC_MC_CODEC_TYPE_LOAD ||
         mcCodecType == VSC_MC_CODEC_TYPE_IMG_LOAD ||
         mcCodecType == VSC_MC_CODEC_TYPE_IMG_ATOM) )
    {
        pOutMcInst->load_inst.inst.bUnallocate = pInCodecHelperInst->instCtrl.u.maCtrl.bUnallocate;
    }

    /* set Endian Control Bit For Global Memory Access Instructions */
    if (pMcCodec->pHwCfg->hwFeatureFlags.supportBigEndianLdSt &&
        pInCodecHelperInst->instCtrl.u.maCtrl.bBigEndian &&
        (mcCodecType == VSC_MC_CODEC_TYPE_STORE ||
         mcCodecType == VSC_MC_CODEC_TYPE_IMG_STORE ||
         mcCodecType == VSC_MC_CODEC_TYPE_LOAD ||
         mcCodecType == VSC_MC_CODEC_TYPE_IMG_LOAD ||
         mcCodecType == VSC_MC_CODEC_TYPE_IMG_ATOM ||
         IS_ATOMIC_MC_OPCODE(pInCodecHelperInst->baseOpcode)) )
    {
        pOutMcInst->load_inst.inst.bBigEndian = pInCodecHelperInst->instCtrl.u.maCtrl.bBigEndian;
    }

    if (pMcCodec->pHwCfg->hwFeatureFlags.supportEndOfBBReissue)
    {
        /* load and store use the same bit forendOfBB */
        pOutMcInst->load_inst.inst.bEndOfBB = pInCodecHelperInst->instCtrl.bEndOfBB;
    }

    return gcvTRUE;
}

static gctBOOL _Common_Decode_Mc_Load_Store_Inst(VSC_MC_CODEC* pMcCodec,
                                                 VSC_MC_CODEC_TYPE mcCodecType,
                                                 VSC_MC_INST* pInMcInst,
                                                 gctUINT expectedMcSrcIdxMask,
                                                 gctBOOL bForImgLS,
                                                 gctBOOL bIsImgAtom,
                                                 VSC_MC_CODEC_INST* pOutCodecHelperInst)
{
    gctUINT           baseOpcode;
    gctUINT           srcIdxOfHelperInst, srcIdxOfMc = 0;
    gctBOOL           bEvisMode = bForImgLS ? (pInMcInst->evis_inst.inst.evisState == 0x01) : gcvFALSE;

    /* Opcode */
    baseOpcode = DECODE_BASE_OPCODE(pInMcInst);

    /* Dst */
    pOutCodecHelperInst->bDstValid = _DecodeDst(pMcCodec, pInMcInst, bEvisMode, &pOutCodecHelperInst->dst);

    /* Src */
    for (srcIdxOfHelperInst = 0; ; srcIdxOfHelperInst ++)
    {
        if (!_DecodeSrcWrapper(pOutCodecHelperInst, pMcCodec, &srcIdxOfMc, expectedMcSrcIdxMask, pInMcInst,
                               gcvFALSE, &pOutCodecHelperInst->src[srcIdxOfHelperInst]))
        {
            break;
        }

        pOutCodecHelperInst->srcCount = srcIdxOfHelperInst + 1;
    }

    /* Inst ctrl */
    pOutCodecHelperInst->instCtrl.bSkipForHelperKickoff = pInMcInst->load_inst.inst.bSkipForHelperKickoff;
    pOutCodecHelperInst->instCtrl.u.maCtrl.bAccessLocalStorage = pInMcInst->load_inst.inst.bAccessLocalStorage;

    if (!bIsImgAtom)
    {
        pOutCodecHelperInst->instCtrl.bDenorm = pInMcInst->load_inst.inst.bDenorm;
    }

    if (!bForImgLS)
    {
        pOutCodecHelperInst->instCtrl.packMode = pInMcInst->load_inst.inst.packMode;
        pOutCodecHelperInst->instCtrl.u.maCtrl.u.lsCtrl.bOffsetX3 = pInMcInst->load_inst.inst.bOffsetX3;
        pOutCodecHelperInst->instCtrl.u.maCtrl.u.lsCtrl.offsetLeftShift = pInMcInst->load_inst.inst.offsetLeftShift;
    }
    else
    {
        pOutCodecHelperInst->instCtrl.u.maCtrl.bUnderEvisMode = bEvisMode;
    }

    if (bIsImgAtom)
    {
        pOutCodecHelperInst->instCtrl.u.maCtrl.u.imgAtomCtrl.atomicMode = pInMcInst->img_atom_inst.inst.atomicMode;
        pOutCodecHelperInst->instCtrl.u.maCtrl.u.imgAtomCtrl.b3dImgMode = pInMcInst->img_atom_inst.inst.b3dImgMode;
    }

    pOutCodecHelperInst->instCtrl.instType = _DecodeInstType(pMcCodec, mcCodecType, pInMcInst);
    pOutCodecHelperInst->instCtrl.threadType = _DecodeThreadType(pMcCodec, mcCodecType, pInMcInst);

    if (pOutCodecHelperInst->bDstValid || bForImgLS)
    {
        pOutCodecHelperInst->instCtrl.bResultSat = pInMcInst->load_inst.inst.bResultSat;
    }

    if (pMcCodec->pHwCfg->hwFeatureFlags.supportUSCUnalloc &&
        pInMcInst->load_inst.inst.bUnallocate &&
        (mcCodecType == VSC_MC_CODEC_TYPE_STORE ||
        mcCodecType == VSC_MC_CODEC_TYPE_IMG_STORE ||
        mcCodecType == VSC_MC_CODEC_TYPE_LOAD ||
        mcCodecType == VSC_MC_CODEC_TYPE_IMG_LOAD ||
        mcCodecType == VSC_MC_CODEC_TYPE_IMG_ATOM))
    {
        pOutCodecHelperInst->instCtrl.u.maCtrl.bUnallocate = pInMcInst->load_inst.inst.bUnallocate;
    }

    if (pMcCodec->pHwCfg->hwFeatureFlags.supportBigEndianLdSt &&
        pInMcInst->load_inst.inst.bBigEndian &&
        (mcCodecType == VSC_MC_CODEC_TYPE_STORE ||
                  mcCodecType == VSC_MC_CODEC_TYPE_IMG_STORE ||
                  mcCodecType == VSC_MC_CODEC_TYPE_LOAD ||
                  mcCodecType == VSC_MC_CODEC_TYPE_IMG_LOAD ||
                  mcCodecType == VSC_MC_CODEC_TYPE_IMG_ATOM ||
                  IS_ATOMIC_MC_OPCODE(pOutCodecHelperInst->baseOpcode) ))
    {
        pOutCodecHelperInst->instCtrl.u.maCtrl.bBigEndian = pInMcInst->load_inst.inst.bBigEndian;
    }

    if (pMcCodec->pHwCfg->hwFeatureFlags.supportEndOfBBReissue)
    {
        /* load and store use the same bit forendOfBB */
        pOutCodecHelperInst->instCtrl.bEndOfBB = pInMcInst->load_inst.inst.bEndOfBB;
    }

    /* Map ld/st hw-opcode to ld/st aux-opcode based on whether dst is enabled */
    pOutCodecHelperInst->baseOpcode = _MapLdStHwOpcodeToAuxOpcode(baseOpcode, pOutCodecHelperInst->bDstValid);

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

    if (pMcCodec->pHwCfg->hwFeatureFlags.supportEndOfBBReissue)
    {
        pOutMcInst->inst.bEndOfBB = pInCodecHelperInst->instCtrl.bEndOfBB;
    }
    return gcvTRUE;
}

static gctBOOL _Encode_Mc_3_Srcs_Alu_Inst(VSC_MC_CODEC* pMcCodec,
                                          VSC_MC_CODEC_TYPE mcCodecType,
                                          VSC_MC_CODEC_INST* pInCodecHelperInst,
                                          VSC_MC_ALU_3_SRCS_INST* pOutMcInst)
{
    gctUINT srcMap[3];

    gcmASSERT(IS_ATOMIC_MC_OPCODE(pInCodecHelperInst->baseOpcode) || pInCodecHelperInst->bDstValid);
    gcmASSERT(mcCodecType == VSC_MC_CODEC_TYPE_3_SRCS_ALU || mcCodecType == VSC_MC_CODEC_TYPE_3_SRCS_CC_ALU);

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
    if (mcCodecType == VSC_MC_CODEC_TYPE_3_SRCS_CC_ALU)
    {
        ((VSC_MC_ALU_3_SRCS_CC_INST*)pOutMcInst)->inst.condOpCode = pInCodecHelperInst->instCtrl.condOpCode;
    }

    /* Atomic operations may
       1. skip for helper pixel,
       2. access local memory, not always global memory
     */
    if (IS_ATOMIC_MC_OPCODE(pInCodecHelperInst->baseOpcode))
    {
        pOutMcInst->inst.bSkipForHelperKickoff = pInCodecHelperInst->instCtrl.bSkipForHelperKickoff;
        ((VSC_MC_INST*)pOutMcInst)->load_inst.inst.bAccessLocalStorage = pInCodecHelperInst->instCtrl.u.maCtrl.bAccessLocalStorage;
    }

    /* Load_attr needs stage client and layout */
    if (pInCodecHelperInst->baseOpcode == 0x78)
    {
        ((VSC_MC_INST*)pOutMcInst)->store_attr_inst.inst.shStageClient =
                                         pInCodecHelperInst->instCtrl.u.lsAttrCtrl.shStageClient;

        ((VSC_MC_INST*)pOutMcInst)->store_attr_inst.inst.attrLayout =
                                         pInCodecHelperInst->instCtrl.u.lsAttrCtrl.attrLayout;
    }

    /* MAD uses MSB5 of word0 as control bit to control result of (0 * INF) */
    if (pInCodecHelperInst->baseOpcode == 0x02)
    {
        pOutMcInst->inst.bInfX0ToZero = pInCodecHelperInst->instCtrl.u.bInfX0ToZero;
    }

    pOutMcInst->inst.bDenorm = pInCodecHelperInst->instCtrl.bDenorm;

    return _Common_Encode_Mc_Alu_Inst(pMcCodec, mcCodecType, pInCodecHelperInst, &srcMap[0], (VSC_MC_INST*)pOutMcInst);
}

static gctBOOL _Encode_Mc_2_Srcs_Src0_Src1_Alu_Inst(VSC_MC_CODEC* pMcCodec,
                                                    VSC_MC_CODEC_TYPE mcCodecType,
                                                    VSC_MC_CODEC_INST* pInCodecHelperInst,
                                                    VSC_MC_ALU_2_SRCS_SRC0_SRC1_INST* pOutMcInst)
{
    gctUINT srcMap[2] = {0, 1};

    gcmASSERT(mcCodecType == VSC_MC_CODEC_TYPE_2_SRCS_SRC0_SRC1_ALU ||
              mcCodecType == VSC_MC_CODEC_TYPE_2_SRCS_SRC0_SRC1_CC_ALU);
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
    if (mcCodecType == VSC_MC_CODEC_TYPE_2_SRCS_SRC0_SRC1_CC_ALU)
    {
        ((VSC_MC_ALU_2_SRCS_SRC0_SRC1_CC_INST *)pOutMcInst)->inst.condOpCode = pInCodecHelperInst->instCtrl.condOpCode;
    }

    if (pInCodecHelperInst->baseOpcode == 0x03 ||
        pInCodecHelperInst->baseOpcode == 0x77 ||
        pInCodecHelperInst->baseOpcode == 0x29 ||
        pInCodecHelperInst->baseOpcode == 0x04 ||
        pInCodecHelperInst->baseOpcode == 0x73 ||
        pInCodecHelperInst->baseOpcode == 0x05 ||
        pInCodecHelperInst->baseOpcode == 0x06)
    {
        /* NORM_MUL/MUL/MULLO/DST/DP use MSB5 of word0 as control bit to control result of (0 * INF) */
        pOutMcInst->inst.bInfX0ToZero = pInCodecHelperInst->instCtrl.u.bInfX0ToZero;
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

    /* NORM_DP uses MSB5 of word0 as control bit to control result of (0 * INF) */
    if (pInCodecHelperInst->baseOpcode == 0x74 ||
        pInCodecHelperInst->baseOpcode == 0x75 ||
        pInCodecHelperInst->baseOpcode == 0x76)
    {
        pOutMcInst->inst.bInfX0ToZero = pInCodecHelperInst->instCtrl.u.bInfX0ToZero;
    }

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
    pOutMcInst->inst.bResultSat = pInCodecHelperInst->instCtrl.bResultSat;
    if (pMcCodec->pHwCfg->hwFeatureFlags.supportEndOfBBReissue)
    {
        pOutMcInst->inst.bEndOfBB = pInCodecHelperInst->instCtrl.bEndOfBB;
    }

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

    ENCODE_EXT_OPCODE(pOutMcInst, pInCodecHelperInst->baseOpcode, pInCodecHelperInst->extOpcode);
    return _Common_Encode_Mc_Load_Store_Inst(pMcCodec, mcCodecType, pInCodecHelperInst, gcvFALSE, gcvFALSE, (VSC_MC_INST*)pOutMcInst);
}

static gctBOOL _Encode_Mc_Img_Load_Inst(VSC_MC_CODEC* pMcCodec,
                                        VSC_MC_CODEC_TYPE mcCodecType,
                                        VSC_MC_CODEC_INST* pInCodecHelperInst,
                                        VSC_MC_IMG_LD_INST* pOutMcInst)
{
    gcmASSERT(mcCodecType == VSC_MC_CODEC_TYPE_IMG_LOAD);
    gcmASSERT(pInCodecHelperInst->bDstValid);
    gcmASSERT(pInCodecHelperInst->srcCount >= 2);

    return _Common_Encode_Mc_Load_Store_Inst(pMcCodec, mcCodecType, pInCodecHelperInst, gcvTRUE, gcvFALSE, (VSC_MC_INST*)pOutMcInst);
}

static gctBOOL _Encode_Mc_Store_Inst(VSC_MC_CODEC* pMcCodec,
                                     VSC_MC_CODEC_TYPE mcCodecType,
                                     VSC_MC_CODEC_INST* pInCodecHelperInst,
                                     VSC_MC_ST_INST* pOutMcInst)
{
    gcmASSERT(mcCodecType == VSC_MC_CODEC_TYPE_STORE);
    gcmASSERT(!pInCodecHelperInst->bDstValid || pInCodecHelperInst->baseOpcode == MC_AUXILIARY_OP_CODE_USC_STORE
                                             || pInCodecHelperInst->baseOpcode == MC_AUXILIARY_OP_CODE_USC_STOREP);
    gcmASSERT(pInCodecHelperInst->srcCount == 3);

    if (pInCodecHelperInst->baseOpcode != MC_AUXILIARY_OP_CODE_USC_STORE)
    {
        /* Normal store has a special memory writemask */
        pOutMcInst->inst.writeMask = pInCodecHelperInst->dst.u.nmlDst.writeMask;
    }

    return _Common_Encode_Mc_Load_Store_Inst(pMcCodec, mcCodecType, pInCodecHelperInst, gcvFALSE, gcvFALSE, (VSC_MC_INST*)pOutMcInst);
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
        if (pInCodecHelperInst->instCtrl.u.maCtrl.bUnderEvisMode)
        {
            /*  Why has no channel to be written? */
            gcmASSERT(pInCodecHelperInst->dst.u.evisDst.compIdxRange > 0);

            ((VSC_MC_INST*)pOutMcInst)->evis_inst.inst.startDstCompIdx = pInCodecHelperInst->dst.u.evisDst.startCompIdx;
            ((VSC_MC_INST*)pOutMcInst)->evis_inst.inst.endDstCompIdx = pInCodecHelperInst->dst.u.evisDst.startCompIdx +
                                                                       pInCodecHelperInst->dst.u.evisDst.compIdxRange - 1;

        }
        else
        {
            /* Normal img-store without evis mode must have full memory writemask */
            pOutMcInst->inst.writeMask = WRITEMASK_ALL;
        }
    }

    return _Common_Encode_Mc_Load_Store_Inst(pMcCodec, mcCodecType, pInCodecHelperInst, gcvTRUE, gcvFALSE, (VSC_MC_INST*)pOutMcInst);
}

static gctBOOL _Encode_Mc_Img_Atom_Inst(VSC_MC_CODEC* pMcCodec,
                                        VSC_MC_CODEC_TYPE mcCodecType,
                                        VSC_MC_CODEC_INST* pInCodecHelperInst,
                                        VSC_MC_IMG_ATOM_INST* pOutMcInst)
{
    gcmASSERT(mcCodecType == VSC_MC_CODEC_TYPE_IMG_ATOM);
    gcmASSERT(pInCodecHelperInst->bDstValid);
    gcmASSERT(pInCodecHelperInst->srcCount == 3);

    return _Common_Encode_Mc_Load_Store_Inst(pMcCodec, mcCodecType, pInCodecHelperInst, gcvTRUE, gcvTRUE, (VSC_MC_INST*)pOutMcInst);
}

static gctBOOL _Encode_Mc_Store_Attr_Inst(VSC_MC_CODEC* pMcCodec,
                                          VSC_MC_CODEC_TYPE mcCodecType,
                                          VSC_MC_CODEC_INST* pInCodecHelperInst,
                                          VSC_MC_STORE_ATTR_INST* pOutMcInst)
{
    gctUINT           srcIdx, baseOpcode = _MapLdStAuxOpcodeToHwOpcode(pInCodecHelperInst->baseOpcode);

    gcmASSERT(mcCodecType == VSC_MC_CODEC_TYPE_STORE_ATTR);
    gcmASSERT(!pInCodecHelperInst->bDstValid || pInCodecHelperInst->baseOpcode == MC_AUXILIARY_OP_CODE_USC_STORE_ATTR);
    gcmASSERT(pInCodecHelperInst->srcCount == 3);

    /* Opcode */
    ENCODE_BASE_OPCODE((VSC_MC_INST*)pOutMcInst, baseOpcode);

    /* Dst */
    if (pInCodecHelperInst->baseOpcode == MC_AUXILIARY_OP_CODE_USC_STORE_ATTR)
    {
        _EncodeDst(pMcCodec, &pInCodecHelperInst->dst, gcvFALSE, (VSC_MC_INST*)pOutMcInst);
    }
    else
    {
        /* Normal store_attr has a special memory writemask */
        pOutMcInst->inst.writeMask = pInCodecHelperInst->dst.u.nmlDst.writeMask;
    }

    /* Usc stage client and layout */
    pOutMcInst->inst.shStageClient = pInCodecHelperInst->instCtrl.u.lsAttrCtrl.shStageClient;
    pOutMcInst->inst.attrLayout = pInCodecHelperInst->instCtrl.u.lsAttrCtrl.attrLayout;

    /* Src */
    for (srcIdx = 0; srcIdx < pInCodecHelperInst->srcCount; srcIdx ++)
    {
        _EncodeSrc(pMcCodec, srcIdx, &pInCodecHelperInst->src[srcIdx], gcvFALSE, (VSC_MC_INST*)pOutMcInst);
    }

    _EncodeInstType(pMcCodec, mcCodecType, (VSC_MC_INST*)pOutMcInst, pInCodecHelperInst->instCtrl.instType);
    _EncodeThreadType(pMcCodec, mcCodecType, (VSC_MC_INST*)pOutMcInst, pInCodecHelperInst->instCtrl.threadType);

    if (pMcCodec->pHwCfg->hwFeatureFlags.supportEndOfBBReissue)
    {
        pOutMcInst->inst.bEndOfBB = pInCodecHelperInst->instCtrl.bEndOfBB;
    }
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
    pOutMcInst->inst.bResultSat = pInCodecHelperInst->instCtrl.bResultSat;

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
    pOutMcInst->inst.packMode = pInCodecHelperInst->instCtrl.packMode;
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
    pOutMcInst->inst.packMode = pInCodecHelperInst->instCtrl.packMode;
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
    pOutMcInst->inst.packMode = pInCodecHelperInst->instCtrl.packMode;
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
    pOutMcInst->inst.bNeedRestartPrim = pInCodecHelperInst->instCtrl.u.emitCtrl.bNeedRestartPrim;
    pOutMcInst->inst.bNoJmpToEndOnMaxVtxCnt = pInCodecHelperInst->instCtrl.u.emitCtrl.bNoJmpToEndOnMaxVtxCnt;
    pOutMcInst->inst.bResultSat = pInCodecHelperInst->instCtrl.bResultSat;

    if (pMcCodec->pHwCfg->hwFeatureFlags.supportEndOfBBReissue)
    {
        pOutMcInst->inst.bEndOfBB = pInCodecHelperInst->instCtrl.bEndOfBB;
    }
    return gcvTRUE;
}

static gctBOOL _Encode_Mc_Conv_Inst(VSC_MC_CODEC* pMcCodec,
                                    VSC_MC_CODEC_TYPE mcCodecType,
                                    VSC_MC_CODEC_INST* pInCodecHelperInst,
                                    VSC_MC_CONV_INST* pOutMcInst)
{
    gctUINT           srcIdx;

    gcmASSERT(mcCodecType == VSC_MC_CODEC_TYPE_CONV);
    gcmASSERT(pInCodecHelperInst->bDstValid);
    gcmASSERT(pInCodecHelperInst->srcCount == 2);

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
    _EncodeInstType(pMcCodec, mcCodecType, (VSC_MC_INST*)pOutMcInst, pInCodecHelperInst->instCtrl.instType);
    _EncodeThreadType(pMcCodec, mcCodecType, (VSC_MC_INST*)pOutMcInst, pInCodecHelperInst->instCtrl.threadType);
    ((VSC_MC_INST*)pOutMcInst)->tri_srcs_alu_inst.inst.bResultSat = pInCodecHelperInst->instCtrl.bResultSat;

    pOutMcInst->inst.roundMode = pInCodecHelperInst->instCtrl.roundingMode;
    pOutMcInst->inst.bEvisMode = pInCodecHelperInst->instCtrl.u.convCtrl.bEvisMode;
    pOutMcInst->inst.bDstPack = pInCodecHelperInst->instCtrl.u.convCtrl.bDstPack;
    pOutMcInst->inst.bSrcPack = pInCodecHelperInst->instCtrl.u.convCtrl.bSrcPack;

    if (pMcCodec->pHwCfg->hwFeatureFlags.supportEndOfBBReissue)
    {
        pOutMcInst->inst.bEndOfBB = pInCodecHelperInst->instCtrl.bEndOfBB;
    }
    return gcvTRUE;
}

static gctBOOL _Encode_Mc_Scatter_Inst(VSC_MC_CODEC* pMcCodec,
                                       VSC_MC_CODEC_TYPE mcCodecType,
                                       VSC_MC_CODEC_INST* pInCodecHelperInst,
                                       VSC_MC_INST* pOutMcInst)
{
    gctUINT           srcIdx;

    gcmASSERT(mcCodecType == VSC_MC_CODEC_TYPE_SCATTER);
    gcmASSERT(!pInCodecHelperInst->bDstValid || pInCodecHelperInst->extOpcode == MC_AUXILIARY_OP_CODE_USC_SCATTER);
    gcmASSERT(pInCodecHelperInst->srcCount == 3);

    /* Opcode */
    ENCODE_BASE_OPCODE(pOutMcInst, 0x45);
    ENCODE_EXT_OPCODE(pOutMcInst, 0x45, 0x1F);

    /* Dst */
    if (pInCodecHelperInst->bDstValid)
    {
        _EncodeDst(pMcCodec, &pInCodecHelperInst->dst, gcvTRUE, pOutMcInst);
    }

    /* Get the startBin/endBin first. */
    pOutMcInst->evis_inst.inst.startDstCompIdx = pInCodecHelperInst->dst.u.evisDst.startCompIdx;
    pOutMcInst->evis_inst.inst.endDstCompIdx = pInCodecHelperInst->dst.u.evisDst.startCompIdx + pInCodecHelperInst->dst.u.evisDst.compIdxRange - 1;

    /* Src */
    for (srcIdx = 0; srcIdx < pInCodecHelperInst->srcCount; srcIdx ++)
    {
        _EncodeSrc(pMcCodec, srcIdx, &pInCodecHelperInst->src[srcIdx], gcvTRUE, pOutMcInst);
    }

    /* Inst ctrl */
    pOutMcInst->load_inst.inst.bSkipForHelperKickoff = pInCodecHelperInst->instCtrl.bSkipForHelperKickoff;
    pOutMcInst->load_inst.inst.bAccessLocalStorage = pInCodecHelperInst->instCtrl.u.maCtrl.bAccessLocalStorage;

    /* EVIS insts have evis-state and sourceBin */
    pOutMcInst->evis_inst.inst.evisState = pInCodecHelperInst->instCtrl.u.visionCtrl.evisState;
    pOutMcInst->evis_inst.inst.startSrcCompIdx = pInCodecHelperInst->instCtrl.u.visionCtrl.startSrcCompIdx;

    _EncodeInstType(pMcCodec, mcCodecType, pOutMcInst, pInCodecHelperInst->instCtrl.instType);
    _EncodeThreadType(pMcCodec, mcCodecType, pOutMcInst, pInCodecHelperInst->instCtrl.threadType);

    return gcvTRUE;
}

static gctBOOL _Encode_Mc_Cmplx_Inst(VSC_MC_CODEC* pMcCodec,
                                     VSC_MC_CODEC_TYPE mcCodecType,
                                     VSC_MC_CODEC_INST* pInCodecHelperInst,
                                     VSC_MC_CMPLX_INST* pOutMcInst)
{
    gctUINT srcMap[3];

    gcmASSERT(IS_ATOMIC_MC_OPCODE(pInCodecHelperInst->baseOpcode) || pInCodecHelperInst->bDstValid);
    gcmASSERT(mcCodecType == VSC_MC_CODEC_TYPE_CMPLX);

    if (pInCodecHelperInst->extOpcode == 0x1)
    {
        gcmASSERT(pInCodecHelperInst->srcCount == 3);

        srcMap[0] = 0;
        srcMap[1] = 1;
        srcMap[2] = 2;
    }
    else if (pInCodecHelperInst->extOpcode == 0x0)
    {
        gcmASSERT(pInCodecHelperInst->srcCount == 2);

        srcMap[0] = 0;
        srcMap[1] = 1;
    }
    else
    {
        srcMap[0] = 0;
        srcMap[1] = 1;
        srcMap[2] = 2;
    }

    pOutMcInst->inst.bDenorm = pInCodecHelperInst->instCtrl.bDenorm;

    pOutMcInst->inst.subOpcode = pInCodecHelperInst->extOpcode;

    return _Common_Encode_Mc_Alu_Inst(pMcCodec, mcCodecType, pInCodecHelperInst, &srcMap[0], (VSC_MC_INST*)pOutMcInst);
}


static PFN_MC_ENCODER _pfn_mc_encoder[] =
{
    gcvNULL,
    (PFN_MC_ENCODER)_Encode_Mc_No_Opnd_Inst,
    (PFN_MC_ENCODER)_Encode_Mc_3_Srcs_Alu_Inst,
    (PFN_MC_ENCODER)_Encode_Mc_3_Srcs_Alu_Inst, /* VSC_MC_CODEC_TYPE_3_SRCS_CC_ALU */
    (PFN_MC_ENCODER)_Encode_Mc_2_Srcs_Src0_Src1_Alu_Inst,
    (PFN_MC_ENCODER)_Encode_Mc_2_Srcs_Src0_Src1_Alu_Inst, /* VSC_MC_CODEC_TYPE_2_SRCS_SRC0_SRC1_CC_ALU */
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
    (PFN_MC_ENCODER)_Encode_Mc_Conv_Inst,
    (PFN_MC_ENCODER)_Encode_Mc_Scatter_Inst,
    (PFN_MC_ENCODER)_Encode_Mc_3_Srcs_Alu_Inst,
    (PFN_MC_ENCODER)_Encode_Mc_Cmplx_Inst,
};

/* Decode routines */

static gctBOOL _Decode_Mc_No_Opnd_Inst(VSC_MC_CODEC* pMcCodec,
                                       VSC_MC_CODEC_TYPE mcCodecType,
                                       VSC_MC_NO_OPERAND_INST* pInMcInst,
                                       VSC_MC_CODEC_INST* pOutCodecHelperInst)
{
    gcmASSERT(mcCodecType == VSC_MC_CODEC_TYPE_NO_OPND);

    pOutCodecHelperInst->baseOpcode = DECODE_BASE_OPCODE((VSC_MC_INST*)pInMcInst);
    pOutCodecHelperInst->extOpcode = DECODE_EXT_OPCODE((VSC_MC_INST*)pInMcInst, pOutCodecHelperInst->baseOpcode);

    if (pMcCodec->pHwCfg->hwFeatureFlags.supportEndOfBBReissue)
    {
        pOutCodecHelperInst->instCtrl.bEndOfBB = pInMcInst->inst.bEndOfBB;
    }
    return gcvTRUE;
}

static gctBOOL _Decode_Mc_3_Srcs_Alu_Inst(VSC_MC_CODEC* pMcCodec,
                                          VSC_MC_CODEC_TYPE mcCodecType,
                                          VSC_MC_ALU_3_SRCS_INST* pInMcInst,
                                          VSC_MC_CODEC_INST* pOutCodecHelperInst)
{
    gctUINT  baseOpcode = DECODE_BASE_OPCODE((VSC_MC_INST*)pInMcInst);
    gctUINT  expectedMcSrcIdxMask = MC_SRC0_BIT | MC_SRC1_BIT | MC_SRC2_BIT;

    gcmASSERT(mcCodecType == VSC_MC_CODEC_TYPE_3_SRCS_ALU ||
              mcCodecType == VSC_MC_CODEC_TYPE_3_SRCS_CC_ALU );

    pOutCodecHelperInst->extOpcode = DECODE_EXT_OPCODE((VSC_MC_INST*)pInMcInst, baseOpcode);
    if (mcCodecType == VSC_MC_CODEC_TYPE_3_SRCS_CC_ALU)
    {
        pOutCodecHelperInst->instCtrl.condOpCode = ((VSC_MC_ALU_3_SRCS_CC_INST *)pInMcInst)->inst.condOpCode;
    }

    /* Atomic operations may skip for helper pixel, also atomics might access local memory, not always global memory */
    if (IS_ATOMIC_MC_OPCODE(baseOpcode))
    {
        pOutCodecHelperInst->instCtrl.bSkipForHelperKickoff = pInMcInst->inst.bSkipForHelperKickoff;
        pOutCodecHelperInst->instCtrl.u.maCtrl.bAccessLocalStorage = ((VSC_MC_INST*)pInMcInst)->load_inst.inst.bAccessLocalStorage;
    }

    /* Load_attr needs stage client and layout */
    if (baseOpcode == 0x78)
    {
        pOutCodecHelperInst->instCtrl.u.lsAttrCtrl.shStageClient = ((VSC_MC_INST*)pInMcInst)->store_attr_inst.inst.shStageClient;
        pOutCodecHelperInst->instCtrl.u.lsAttrCtrl.attrLayout = ((VSC_MC_INST*)pInMcInst)->store_attr_inst.inst.attrLayout;
    }

    if (baseOpcode == 0x02)
    {
        pOutCodecHelperInst->instCtrl.u.bInfX0ToZero = pInMcInst->inst.bInfX0ToZero;
    }

    pOutCodecHelperInst->instCtrl.bDenorm = pInMcInst->inst.bDenorm;

    return _Common_Decode_Mc_Alu_Inst(pMcCodec, mcCodecType, (VSC_MC_INST*)pInMcInst, expectedMcSrcIdxMask, pOutCodecHelperInst);
}

static gctBOOL _Decode_Mc_2_Srcs_Src0_Src1_Alu_Inst(VSC_MC_CODEC* pMcCodec,
                                                    VSC_MC_CODEC_TYPE mcCodecType,
                                                    VSC_MC_ALU_2_SRCS_SRC0_SRC1_INST* pInMcInst,
                                                    VSC_MC_CODEC_INST* pOutCodecHelperInst)
{
    gctUINT  baseOpcode = DECODE_BASE_OPCODE((VSC_MC_INST*)pInMcInst);
    gctUINT  expectedMcSrcIdxMask = MC_SRC0_BIT | MC_SRC1_BIT;

    gcmASSERT(mcCodecType == VSC_MC_CODEC_TYPE_2_SRCS_SRC0_SRC1_ALU ||
              mcCodecType == VSC_MC_CODEC_TYPE_2_SRCS_SRC0_SRC1_CC_ALU);

    pOutCodecHelperInst->extOpcode = DECODE_EXT_OPCODE((VSC_MC_INST*)pInMcInst, baseOpcode);
    if (mcCodecType == VSC_MC_CODEC_TYPE_2_SRCS_SRC0_SRC1_CC_ALU)
    {
        pOutCodecHelperInst->instCtrl.condOpCode = ((VSC_MC_ALU_2_SRCS_SRC0_SRC1_CC_INST *)pInMcInst)->inst.condOpCode;
    }

    if (baseOpcode == 0x03 ||
        baseOpcode == 0x77 ||
        baseOpcode == 0x29 ||
        baseOpcode == 0x04 ||
        baseOpcode == 0x73 ||
        baseOpcode == 0x05 ||
        baseOpcode == 0x06)
    {
        pOutCodecHelperInst->instCtrl.u.bInfX0ToZero = pInMcInst->inst.bInfX0ToZero;
    }

    return _Common_Decode_Mc_Alu_Inst(pMcCodec, mcCodecType, (VSC_MC_INST*)pInMcInst, expectedMcSrcIdxMask, pOutCodecHelperInst);
}

static gctBOOL _Decode_Mc_2_Srcs_Src0_Src2_Alu_Inst(VSC_MC_CODEC* pMcCodec,
                                                    VSC_MC_CODEC_TYPE mcCodecType,
                                                    VSC_MC_ALU_2_SRCS_SRC0_SRC2_INST* pInMcInst,
                                                    VSC_MC_CODEC_INST* pOutCodecHelperInst)
{
    gctUINT  baseOpcode = DECODE_BASE_OPCODE((VSC_MC_INST*)pInMcInst);
    gctUINT  expectedMcSrcIdxMask = MC_SRC0_BIT | MC_SRC2_BIT;

    gcmASSERT(mcCodecType == VSC_MC_CODEC_TYPE_2_SRCS_SRC0_SRC2_ALU);

    pOutCodecHelperInst->extOpcode = DECODE_EXT_OPCODE((VSC_MC_INST*)pInMcInst, baseOpcode);

    return _Common_Decode_Mc_Alu_Inst(pMcCodec, mcCodecType, (VSC_MC_INST*)pInMcInst, expectedMcSrcIdxMask, pOutCodecHelperInst);
}

static gctBOOL _Decode_Mc_2_Srcs_Src1_Src2_Alu_Inst(VSC_MC_CODEC* pMcCodec,
                                                    VSC_MC_CODEC_TYPE mcCodecType,
                                                    VSC_MC_ALU_2_SRCS_SRC1_SRC2_INST* pInMcInst,
                                                    VSC_MC_CODEC_INST* pOutCodecHelperInst)
{
    gctUINT  expectedMcSrcIdxMask = MC_SRC1_BIT | MC_SRC2_BIT;

    gcmASSERT(mcCodecType == VSC_MC_CODEC_TYPE_2_SRCS_SRC1_SRC2_ALU);

    return _Common_Decode_Mc_Alu_Inst(pMcCodec, mcCodecType, (VSC_MC_INST*)pInMcInst, expectedMcSrcIdxMask, pOutCodecHelperInst);
}

static gctBOOL _Decode_Mc_1_Src_Src0_Alu_Inst(VSC_MC_CODEC* pMcCodec,
                                              VSC_MC_CODEC_TYPE mcCodecType,
                                              VSC_MC_ALU_1_SRC_SRC0_INST* pInMcInst,
                                              VSC_MC_CODEC_INST* pOutCodecHelperInst)
{
    gctUINT  baseOpcode = DECODE_BASE_OPCODE((VSC_MC_INST*)pInMcInst);
    gctUINT  expectedMcSrcIdxMask = MC_SRC0_BIT;

    gcmASSERT(mcCodecType == VSC_MC_CODEC_TYPE_1_SRC_SRC0_ALU);

    pOutCodecHelperInst->extOpcode = DECODE_EXT_OPCODE((VSC_MC_INST*)pInMcInst, baseOpcode);

    if (baseOpcode == 0x74 ||
        baseOpcode == 0x75 ||
        baseOpcode == 0x76)
    {
        pOutCodecHelperInst->instCtrl.u.bInfX0ToZero = pInMcInst->inst.bInfX0ToZero;
    }

    return _Common_Decode_Mc_Alu_Inst(pMcCodec, mcCodecType, (VSC_MC_INST*)pInMcInst, expectedMcSrcIdxMask, pOutCodecHelperInst);
}

static gctBOOL _Decode_Mc_1_Src_Src1_Alu_Inst(VSC_MC_CODEC* pMcCodec,
                                              VSC_MC_CODEC_TYPE mcCodecType,
                                              VSC_MC_ALU_1_SRC_SRC1_INST* pInMcInst,
                                              VSC_MC_CODEC_INST* pOutCodecHelperInst)
{
    gctUINT  baseOpcode = DECODE_BASE_OPCODE((VSC_MC_INST*)pInMcInst);
    gctUINT  expectedMcSrcIdxMask = MC_SRC1_BIT;

    gcmASSERT(mcCodecType == VSC_MC_CODEC_TYPE_1_SRC_SRC1_ALU);

    pOutCodecHelperInst->extOpcode = DECODE_EXT_OPCODE((VSC_MC_INST*)pInMcInst, baseOpcode);

    return _Common_Decode_Mc_Alu_Inst(pMcCodec, mcCodecType, (VSC_MC_INST*)pInMcInst, expectedMcSrcIdxMask, pOutCodecHelperInst);
}

static gctBOOL _Decode_Mc_1_Src_Src2_Alu_Inst(VSC_MC_CODEC* pMcCodec,
                                              VSC_MC_CODEC_TYPE mcCodecType,
                                              VSC_MC_ALU_1_SRC_SRC2_INST* pInMcInst,
                                              VSC_MC_CODEC_INST* pOutCodecHelperInst)
{
    gctUINT  expectedMcSrcIdxMask = MC_SRC2_BIT;

    gcmASSERT(mcCodecType == VSC_MC_CODEC_TYPE_1_SRC_SRC2_ALU);

    return _Common_Decode_Mc_Alu_Inst(pMcCodec, mcCodecType, (VSC_MC_INST*)pInMcInst, expectedMcSrcIdxMask, pOutCodecHelperInst);
}

static gctBOOL _Decode_Mc_Pack_Inst(VSC_MC_CODEC* pMcCodec,
                                    VSC_MC_CODEC_TYPE mcCodecType,
                                    VSC_MC_PACK_INST* pInMcInst,
                                    VSC_MC_CODEC_INST* pOutCodecHelperInst)
{
    gctUINT           srcIdxOfHelperInst, srcIdxOfMc = 0;
    gctUINT           expectedMcSrcIdxMask = MC_SRC0_BIT | MC_SRC1_BIT | MC_SRC2_BIT;

    gcmASSERT(mcCodecType == VSC_MC_CODEC_TYPE_PACK);

    /* Opcode */
    pOutCodecHelperInst->baseOpcode = DECODE_BASE_OPCODE((VSC_MC_INST*)pInMcInst);

    /* Dst */
    pOutCodecHelperInst->bDstValid = _DecodeDst(pMcCodec, (VSC_MC_INST*)pInMcInst, gcvFALSE, &pOutCodecHelperInst->dst);

    /* Src */
    for (srcIdxOfHelperInst = 0; ; srcIdxOfHelperInst ++)
    {
        if (!_DecodeSrcWrapper(pOutCodecHelperInst, pMcCodec, &srcIdxOfMc, expectedMcSrcIdxMask, (VSC_MC_INST*)pInMcInst,
                               gcvFALSE, &pOutCodecHelperInst->src[srcIdxOfHelperInst]))
        {
            break;
        }

        pOutCodecHelperInst->srcCount = srcIdxOfHelperInst + 1;
    }

    /* Inst ctrl */
    pOutCodecHelperInst->instCtrl.u.srcSelect = pInMcInst->inst.srcSelect;
    pOutCodecHelperInst->instCtrl.instType = _DecodeInstType(pMcCodec, mcCodecType, (VSC_MC_INST*)pInMcInst);
    pOutCodecHelperInst->instCtrl.threadType = _DecodeThreadType(pMcCodec, mcCodecType, (VSC_MC_INST*)pInMcInst);
    pOutCodecHelperInst->instCtrl.bResultSat = pInMcInst->inst.bResultSat;
    if (pMcCodec->pHwCfg->hwFeatureFlags.supportEndOfBBReissue)
    {
        pOutCodecHelperInst->instCtrl.bEndOfBB = pInMcInst->inst.bEndOfBB;
    }

    return gcvTRUE;
}

static gctBOOL _Decode_Mc_Sample_Inst(VSC_MC_CODEC* pMcCodec,
                                      VSC_MC_CODEC_TYPE mcCodecType,
                                      VSC_MC_SAMPLE_INST* pInMcInst,
                                      VSC_MC_CODEC_INST* pOutCodecHelperInst)
{
    gctUINT  expectedMcSrcIdxMask = MC_SRC0_BIT | MC_SRC1_BIT | MC_SRC2_BIT;

    gcmASSERT(mcCodecType == VSC_MC_CODEC_TYPE_SAMPLE);

    return _Common_Decode_Mc_Sample_Inst(pMcCodec, mcCodecType, (VSC_MC_INST*)pInMcInst, expectedMcSrcIdxMask, pOutCodecHelperInst);
}

static gctBOOL _Decode_Mc_Sample_Ext_Inst(VSC_MC_CODEC* pMcCodec,
                                          VSC_MC_CODEC_TYPE mcCodecType,
                                          VSC_MC_SAMPLE_EXT_INST* pInMcInst,
                                          VSC_MC_CODEC_INST* pOutCodecHelperInst)
{
    gctUINT  baseOpcode = DECODE_BASE_OPCODE((VSC_MC_INST*)pInMcInst);
    gctUINT  expectedMcSrcIdxMask = MC_SRC0_BIT | MC_SRC1_BIT;

    gcmASSERT(mcCodecType == VSC_MC_CODEC_TYPE_SAMPLE_EXT);

    pOutCodecHelperInst->extOpcode = DECODE_EXT_OPCODE((VSC_MC_INST*)pInMcInst, baseOpcode);

    return _Common_Decode_Mc_Sample_Inst(pMcCodec, mcCodecType, (VSC_MC_INST*)pInMcInst, expectedMcSrcIdxMask, pOutCodecHelperInst);
}

static gctBOOL _Decode_Mc_Load_Inst(VSC_MC_CODEC* pMcCodec,
                                    VSC_MC_CODEC_TYPE mcCodecType,
                                    VSC_MC_LD_INST* pInMcInst,
                                    VSC_MC_CODEC_INST* pOutCodecHelperInst)
{
    gctUINT  baseOpcode = DECODE_BASE_OPCODE((VSC_MC_INST*)pInMcInst);
    gctUINT  expectedMcSrcIdxMask = MC_SRC0_BIT | MC_SRC1_BIT;

    gcmASSERT(mcCodecType == VSC_MC_CODEC_TYPE_LOAD);

    pOutCodecHelperInst->extOpcode = DECODE_EXT_OPCODE((VSC_MC_INST*)pInMcInst, baseOpcode);

    return _Common_Decode_Mc_Load_Store_Inst(pMcCodec, mcCodecType, (VSC_MC_INST*)pInMcInst,
                                             expectedMcSrcIdxMask, gcvFALSE, gcvFALSE, pOutCodecHelperInst);
}

static gctBOOL _Decode_Mc_Img_Load_Inst(VSC_MC_CODEC* pMcCodec,
                                        VSC_MC_CODEC_TYPE mcCodecType,
                                        VSC_MC_IMG_LD_INST* pInMcInst,
                                        VSC_MC_CODEC_INST* pOutCodecHelperInst)
{
    gctUINT  expectedMcSrcIdxMask = MC_SRC0_BIT | MC_SRC1_BIT | MC_SRC2_BIT;

    gcmASSERT(mcCodecType == VSC_MC_CODEC_TYPE_IMG_LOAD);

    return _Common_Decode_Mc_Load_Store_Inst(pMcCodec, mcCodecType, (VSC_MC_INST*)pInMcInst,
                                             expectedMcSrcIdxMask, gcvTRUE, gcvFALSE, pOutCodecHelperInst);
}

static gctBOOL _Decode_Mc_Store_Inst(VSC_MC_CODEC* pMcCodec,
                                     VSC_MC_CODEC_TYPE mcCodecType,
                                     VSC_MC_ST_INST* pInMcInst,
                                     VSC_MC_CODEC_INST* pOutCodecHelperInst)
{
    gctBOOL  bRet;
    gctUINT  expectedMcSrcIdxMask = MC_SRC0_BIT | MC_SRC1_BIT | MC_SRC2_BIT;

    gcmASSERT(mcCodecType == VSC_MC_CODEC_TYPE_STORE);

    bRet = _Common_Decode_Mc_Load_Store_Inst(pMcCodec, mcCodecType, (VSC_MC_INST*)pInMcInst,
                                             expectedMcSrcIdxMask, gcvFALSE, gcvFALSE, pOutCodecHelperInst);

    if (pOutCodecHelperInst->baseOpcode != MC_AUXILIARY_OP_CODE_USC_STORE)
    {
        /* Normal store has a special memory writemask */
        pOutCodecHelperInst->dst.u.nmlDst.writeMask = pInMcInst->inst.writeMask;
    }

    return bRet;
}

static gctBOOL _Decode_Mc_Img_Store_Inst(VSC_MC_CODEC* pMcCodec,
                                         VSC_MC_CODEC_TYPE mcCodecType,
                                         VSC_MC_ST_INST* pInMcInst,
                                         VSC_MC_CODEC_INST* pOutCodecHelperInst)
{
    gctBOOL  bRet;
    gctUINT  expectedMcSrcIdxMask = MC_SRC0_BIT | MC_SRC1_BIT | MC_SRC2_BIT;

    gcmASSERT(mcCodecType == VSC_MC_CODEC_TYPE_IMG_STORE);

    bRet = _Common_Decode_Mc_Load_Store_Inst(pMcCodec, mcCodecType, (VSC_MC_INST*)pInMcInst,
                                             expectedMcSrcIdxMask, gcvTRUE, gcvFALSE, pOutCodecHelperInst);

    if (pOutCodecHelperInst->baseOpcode != MC_AUXILIARY_OP_CODE_USC_IMG_STORE &&
        pOutCodecHelperInst->baseOpcode != MC_AUXILIARY_OP_CODE_USC_IMG_STORE_3D)
    {
        if (pOutCodecHelperInst->instCtrl.u.maCtrl.bUnderEvisMode)
        {
            pOutCodecHelperInst->dst.u.evisDst.startCompIdx = ((VSC_MC_INST*)pInMcInst)->evis_inst.inst.startDstCompIdx;
            pOutCodecHelperInst->dst.u.evisDst.compIdxRange = ((VSC_MC_INST*)pInMcInst)->evis_inst.inst.endDstCompIdx -
                                                              ((VSC_MC_INST*)pInMcInst)->evis_inst.inst.startDstCompIdx + 1;
        }
    }

    return bRet;
}

static gctBOOL _Decode_Mc_Img_Atom_Inst(VSC_MC_CODEC* pMcCodec,
                                         VSC_MC_CODEC_TYPE mcCodecType,
                                         VSC_MC_IMG_ATOM_INST* pInMcInst,
                                         VSC_MC_CODEC_INST* pOutCodecHelperInst)
{
    gctUINT  expectedMcSrcIdxMask = MC_SRC0_BIT | MC_SRC1_BIT | MC_SRC2_BIT;

    gcmASSERT(mcCodecType == VSC_MC_CODEC_TYPE_IMG_ATOM);

    return _Common_Decode_Mc_Load_Store_Inst(pMcCodec, mcCodecType, (VSC_MC_INST*)pInMcInst,
                                             expectedMcSrcIdxMask, gcvTRUE, gcvTRUE, pOutCodecHelperInst);
}

static gctBOOL _Decode_Mc_Store_Attr_Inst(VSC_MC_CODEC* pMcCodec,
                                          VSC_MC_CODEC_TYPE mcCodecType,
                                          VSC_MC_STORE_ATTR_INST* pInMcInst,
                                          VSC_MC_CODEC_INST* pOutCodecHelperInst)
{
    gctUINT           baseOpcode;
    gctUINT           srcIdxOfHelperInst, srcIdxOfMc = 0;
    gctUINT           expectedMcSrcIdxMask = MC_SRC0_BIT | MC_SRC1_BIT | MC_SRC2_BIT;

    gcmASSERT(mcCodecType == VSC_MC_CODEC_TYPE_STORE_ATTR);

    /* Opcode */
    baseOpcode = DECODE_BASE_OPCODE((VSC_MC_INST*)pInMcInst);

    /* Dst */
    pOutCodecHelperInst->bDstValid = _DecodeDst(pMcCodec, (VSC_MC_INST*)pInMcInst, gcvFALSE, &pOutCodecHelperInst->dst);
    if (!pOutCodecHelperInst->bDstValid)
    {
        /* Normal store_attr has a special memory writemask */
        pOutCodecHelperInst->dst.u.nmlDst.writeMask = pInMcInst->inst.writeMask;
    }

    /* Usc stage client and layout */
    pOutCodecHelperInst->instCtrl.u.lsAttrCtrl.shStageClient = pInMcInst->inst.shStageClient;
    pOutCodecHelperInst->instCtrl.u.lsAttrCtrl.attrLayout = pInMcInst->inst.attrLayout;

    /* Src */
    for (srcIdxOfHelperInst = 0; ; srcIdxOfHelperInst ++)
    {
        if (!_DecodeSrcWrapper(pOutCodecHelperInst, pMcCodec, &srcIdxOfMc, expectedMcSrcIdxMask, (VSC_MC_INST*)pInMcInst,
                               gcvFALSE, &pOutCodecHelperInst->src[srcIdxOfHelperInst]))
        {
            break;
        }

        pOutCodecHelperInst->srcCount = srcIdxOfHelperInst + 1;
    }

    pOutCodecHelperInst->instCtrl.instType = _DecodeInstType(pMcCodec, mcCodecType, (VSC_MC_INST*)pInMcInst);
    pOutCodecHelperInst->instCtrl.threadType = _DecodeThreadType(pMcCodec, mcCodecType, (VSC_MC_INST*)pInMcInst);

    /* Map st-att hw-opcode to st-att aux-opcode based on whether dst is enabled */
    pOutCodecHelperInst->baseOpcode = _MapLdStHwOpcodeToAuxOpcode(baseOpcode, pOutCodecHelperInst->bDstValid);
    if (pMcCodec->pHwCfg->hwFeatureFlags.supportEndOfBBReissue)
    {
        pOutCodecHelperInst->instCtrl.bEndOfBB = pInMcInst->inst.bEndOfBB;
    }

    return gcvTRUE;
}

static gctBOOL _Decode_Mc_Select_Map_Inst(VSC_MC_CODEC* pMcCodec,
                                          VSC_MC_CODEC_TYPE mcCodecType,
                                          VSC_MC_SELECT_MAP_INST* pInMcInst,
                                          VSC_MC_CODEC_INST* pOutCodecHelperInst)
{
    gctUINT           srcIdxOfHelperInst, srcIdxOfMc = 0;
    gctUINT           expectedMcSrcIdxMask = MC_SRC0_BIT | MC_SRC1_BIT | MC_SRC2_BIT;

    gcmASSERT(mcCodecType == VSC_MC_CODEC_TYPE_SELECT_MAP);

    /* Opcode */
    pOutCodecHelperInst->baseOpcode = DECODE_BASE_OPCODE((VSC_MC_INST*)pInMcInst);

    /* Dst */
    pOutCodecHelperInst->bDstValid = _DecodeDst(pMcCodec, (VSC_MC_INST*)pInMcInst, gcvFALSE, &pOutCodecHelperInst->dst);

    /* Src */
    for (srcIdxOfHelperInst = 0; ; srcIdxOfHelperInst ++)
    {
        if (!_DecodeSrcWrapper(pOutCodecHelperInst, pMcCodec, &srcIdxOfMc, expectedMcSrcIdxMask, (VSC_MC_INST*)pInMcInst,
                               gcvFALSE, &pOutCodecHelperInst->src[srcIdxOfHelperInst]))
        {
            break;
        }

        pOutCodecHelperInst->srcCount = srcIdxOfHelperInst + 1;
    }

    /* Inst ctrl */
    pOutCodecHelperInst->instCtrl.u.smCtrl.rangeToMatch = pInMcInst->inst.rangeToMatch;
    pOutCodecHelperInst->instCtrl.u.smCtrl.bCompSel = pInMcInst->inst.bCompSel;
    pOutCodecHelperInst->instCtrl.instType = _DecodeInstType(pMcCodec, mcCodecType, (VSC_MC_INST*)pInMcInst);
    pOutCodecHelperInst->instCtrl.threadType = _DecodeThreadType(pMcCodec, mcCodecType, (VSC_MC_INST*)pInMcInst);
    pOutCodecHelperInst->instCtrl.bResultSat = pInMcInst->inst.bResultSat;

    return gcvTRUE;
}

static gctBOOL _Decode_Mc_Direct_Branch_0_Inst(VSC_MC_CODEC* pMcCodec,
                                               VSC_MC_CODEC_TYPE mcCodecType,
                                               VSC_MC_DIRECT_BRANCH_0_INST* pInMcInst,
                                               VSC_MC_CODEC_INST* pOutCodecHelperInst)
{
    gctUINT           srcIdxOfHelperInst, srcIdxOfMc = 0, branchTargetSrcIdx;
    gctUINT           expectedMcSrcIdxMask = MC_SRC0_BIT | MC_SRC1_BIT;

    gcmASSERT(mcCodecType == VSC_MC_CODEC_TYPE_DIRECT_BRANCH_0);

    /* Opcode */
    pOutCodecHelperInst->baseOpcode = DECODE_BASE_OPCODE((VSC_MC_INST*)pInMcInst);

    /* Src */
    for (srcIdxOfHelperInst = 0; ; srcIdxOfHelperInst ++)
    {
        if (!_DecodeSrcWrapper(pOutCodecHelperInst, pMcCodec, &srcIdxOfMc, expectedMcSrcIdxMask, (VSC_MC_INST*)pInMcInst,
                               gcvFALSE, &pOutCodecHelperInst->src[srcIdxOfHelperInst]))
        {
            break;
        }

        pOutCodecHelperInst->srcCount = srcIdxOfHelperInst + 1;
    }

    branchTargetSrcIdx = _condOp2SrcCount[pInMcInst->inst.condOpCode];
    gcmASSERT(pOutCodecHelperInst->srcCount == branchTargetSrcIdx);

    /* Branch target */
    pOutCodecHelperInst->src[branchTargetSrcIdx].regType = 0x7;
    pOutCodecHelperInst->src[branchTargetSrcIdx].u.imm.immType = 0x2;
    pOutCodecHelperInst->src[branchTargetSrcIdx].u.imm.immData.ui = pInMcInst->inst.branchTarget;
    pOutCodecHelperInst->srcCount ++;

    /* Inst ctrl */
    pOutCodecHelperInst->instCtrl.condOpCode = pInMcInst->inst.condOpCode;
    pOutCodecHelperInst->instCtrl.packMode = pInMcInst->inst.packMode;
    pOutCodecHelperInst->instCtrl.u.loopOpType = pInMcInst->inst.loopOpType;
    pOutCodecHelperInst->instCtrl.instType = _DecodeInstType(pMcCodec, mcCodecType, (VSC_MC_INST*)pInMcInst);

    return gcvTRUE;
}

static gctBOOL _Decode_Mc_Direct_Branch_1_Inst(VSC_MC_CODEC* pMcCodec,
                                               VSC_MC_CODEC_TYPE mcCodecType,
                                               VSC_MC_DIRECT_BRANCH_1_INST* pInMcInst,
                                               VSC_MC_CODEC_INST* pOutCodecHelperInst)
{
    gctUINT           srcIdxOfHelperInst, srcIdxOfMc = 0;
    gctUINT           branchTargetSrcIdx = _condOp2SrcCount[pInMcInst->inst.condOpCode];
    gctUINT           branchTarget20Bit;
    gctUINT           expectedMcSrcIdxMask = MC_SRC0_BIT | MC_SRC1_BIT | MC_SRC2_BIT;

    gcmASSERT(mcCodecType == VSC_MC_CODEC_TYPE_DIRECT_BRANCH_1);

    /* Opcode */
    pOutCodecHelperInst->baseOpcode = DECODE_BASE_OPCODE((VSC_MC_INST*)pInMcInst);

    /* Src */
    for (srcIdxOfHelperInst = 0; ; srcIdxOfHelperInst ++)
    {
        if (pOutCodecHelperInst->srcCount == branchTargetSrcIdx)
        {
            break;
        }

        if (!_DecodeSrcWrapper(pOutCodecHelperInst, pMcCodec, &srcIdxOfMc, expectedMcSrcIdxMask, (VSC_MC_INST*)pInMcInst,
                               gcvFALSE, &pOutCodecHelperInst->src[srcIdxOfHelperInst]))
        {
            break;
        }

        pOutCodecHelperInst->srcCount = srcIdxOfHelperInst + 1;
    }

    /* Branch target */
    branchTarget20Bit = pInMcInst->inst.branchTargetBit0_8;
    branchTarget20Bit |= (pInMcInst->inst.branchTargetBit9_18 << 9);
    branchTarget20Bit |= (pInMcInst->inst.branchTargetBit19 << 19);
    pOutCodecHelperInst->src[branchTargetSrcIdx].regType = 0x7;
    pOutCodecHelperInst->src[branchTargetSrcIdx].u.imm.immType = pInMcInst->inst.immType;
    pOutCodecHelperInst->src[branchTargetSrcIdx].u.imm.immData = _Conver20BitImm_2_32BitImm(branchTarget20Bit,
                                                                                            pInMcInst->inst.immType);
    pOutCodecHelperInst->srcCount ++;

    /* Inst ctrl */
    pOutCodecHelperInst->instCtrl.condOpCode = pInMcInst->inst.condOpCode;
    pOutCodecHelperInst->instCtrl.packMode = pInMcInst->inst.packMode;
    pOutCodecHelperInst->instCtrl.instType = _DecodeInstType(pMcCodec, mcCodecType, (VSC_MC_INST*)pInMcInst);
    pOutCodecHelperInst->instCtrl.threadType = _DecodeThreadType(pMcCodec, mcCodecType, (VSC_MC_INST*)pInMcInst);

    return gcvTRUE;
}

static gctBOOL _Decode_Mc_Indirect_Branch_Inst(VSC_MC_CODEC* pMcCodec,
                                               VSC_MC_CODEC_TYPE mcCodecType,
                                               VSC_MC_INDIRECT_BRANCH_INST* pInMcInst,
                                               VSC_MC_CODEC_INST* pOutCodecHelperInst)
{
    gctUINT           srcIdxOfBranchTarget = 2;
    gctUINT           expectedMcSrcIdxMask = MC_SRC2_BIT;

    gcmASSERT(mcCodecType == VSC_MC_CODEC_TYPE_INDIRECT_BRANCH);

    /* Opcode */
    pOutCodecHelperInst->baseOpcode = DECODE_BASE_OPCODE((VSC_MC_INST*)pInMcInst);

    /* Branch target */
    _DecodeSrcWrapper(pOutCodecHelperInst, pMcCodec, &srcIdxOfBranchTarget, expectedMcSrcIdxMask,
                      (VSC_MC_INST*)pInMcInst, gcvFALSE, &pOutCodecHelperInst->src[0]);
    pOutCodecHelperInst->srcCount = 1;

    /* Inst ctrl */
    pOutCodecHelperInst->instCtrl.condOpCode = pInMcInst->inst.condOpCode;
    pOutCodecHelperInst->instCtrl.packMode = pInMcInst->inst.packMode;
    pOutCodecHelperInst->instCtrl.instType = _DecodeInstType(pMcCodec, mcCodecType, (VSC_MC_INST*)pInMcInst);
    pOutCodecHelperInst->instCtrl.threadType = _DecodeThreadType(pMcCodec, mcCodecType, (VSC_MC_INST*)pInMcInst);

    return gcvTRUE;
}

static gctBOOL _Decode_Mc_Direct_Call_Inst(VSC_MC_CODEC* pMcCodec,
                                           VSC_MC_CODEC_TYPE mcCodecType,
                                           VSC_MC_DIRECT_CALL_INST* pInMcInst,
                                           VSC_MC_CODEC_INST* pOutCodecHelperInst)
{
    gcmASSERT(mcCodecType == VSC_MC_CODEC_TYPE_DIRECT_CALL);

    /* Opcode */
    pOutCodecHelperInst->baseOpcode = DECODE_BASE_OPCODE((VSC_MC_INST*)pInMcInst);

    /* Call target */
    pOutCodecHelperInst->src[0].regType = 0x7;
    pOutCodecHelperInst->src[0].u.imm.immType = 0x2;
    pOutCodecHelperInst->src[0].u.imm.immData.ui = pInMcInst->inst.callTarget;
    pOutCodecHelperInst->srcCount = 1;

    return gcvTRUE;
}

static gctBOOL _Decode_Mc_Indirect_Call_Inst(VSC_MC_CODEC* pMcCodec,
                                             VSC_MC_CODEC_TYPE mcCodecType,
                                             VSC_MC_INDIRECT_CALL_INST* pInMcInst,
                                             VSC_MC_CODEC_INST* pOutCodecHelperInst)
{
    gctUINT           srcIdxOfCallTarget = 2;
    gctUINT           expectedMcSrcIdxMask = MC_SRC2_BIT;

    gcmASSERT(mcCodecType == VSC_MC_CODEC_TYPE_INDIRECT_CALL);

    /* Opcode */
    pOutCodecHelperInst->baseOpcode = DECODE_BASE_OPCODE((VSC_MC_INST*)pInMcInst);

    /* Call target */
    _DecodeSrcWrapper(pOutCodecHelperInst, pMcCodec, &srcIdxOfCallTarget, expectedMcSrcIdxMask,
                      (VSC_MC_INST*)pInMcInst, gcvFALSE, &pOutCodecHelperInst->src[0]);
    pOutCodecHelperInst->srcCount = 1;

    /* Inst ctrl */
    pOutCodecHelperInst->instCtrl.instType = _DecodeInstType(pMcCodec, mcCodecType, (VSC_MC_INST*)pInMcInst);
    pOutCodecHelperInst->instCtrl.threadType = _DecodeThreadType(pMcCodec, mcCodecType, (VSC_MC_INST*)pInMcInst);

    return gcvTRUE;
}

static gctBOOL _Decode_Mc_Loop_Inst(VSC_MC_CODEC* pMcCodec,
                                    VSC_MC_CODEC_TYPE mcCodecType,
                                    VSC_MC_LOOP_INST* pInMcInst,
                                    VSC_MC_CODEC_INST* pOutCodecHelperInst)
{
    gctUINT           srcIdxOfMc = 1;
    gctUINT           expectedMcSrcIdxMask = MC_SRC1_BIT;

    gcmASSERT(mcCodecType == VSC_MC_CODEC_TYPE_LOOP);

    /* Opcode */
    pOutCodecHelperInst->baseOpcode = DECODE_BASE_OPCODE((VSC_MC_INST*)pInMcInst);

    /* Src */
    _DecodeSrcWrapper(pOutCodecHelperInst, pMcCodec, &srcIdxOfMc, expectedMcSrcIdxMask,
                      (VSC_MC_INST*)pInMcInst, gcvFALSE, &pOutCodecHelperInst->src[0]);
    pOutCodecHelperInst->srcCount = 1;

    /* Loop target */
    pOutCodecHelperInst->src[1].regType = 0x7;
    pOutCodecHelperInst->src[1].u.imm.immType = 0x2;
    pOutCodecHelperInst->src[1].u.imm.immData.ui = pInMcInst->inst.branchTarget;
    pOutCodecHelperInst->srcCount ++;

    /* Inst ctrl */
    pOutCodecHelperInst->instCtrl.instType = _DecodeInstType(pMcCodec, mcCodecType, (VSC_MC_INST*)pInMcInst);

    return gcvTRUE;
}

static gctBOOL _Decode_Mc_Emit_Inst(VSC_MC_CODEC* pMcCodec,
                                    VSC_MC_CODEC_TYPE mcCodecType,
                                    VSC_MC_EMIT_INST* pInMcInst,
                                    VSC_MC_CODEC_INST* pOutCodecHelperInst)
{
    gctUINT           srcIdxOfHelperInst, srcIdxOfMc = 0;
    gctUINT           expectedMcSrcIdxMask = MC_SRC0_BIT | MC_SRC1_BIT;

    gcmASSERT(mcCodecType == VSC_MC_CODEC_TYPE_EMIT);

    /* Opcode */
    pOutCodecHelperInst->baseOpcode = DECODE_BASE_OPCODE((VSC_MC_INST*)pInMcInst);
    pOutCodecHelperInst->extOpcode = DECODE_EXT_OPCODE((VSC_MC_INST*)pInMcInst, pOutCodecHelperInst->baseOpcode);

    /* Dst */
    pOutCodecHelperInst->bDstValid = _DecodeDst(pMcCodec, (VSC_MC_INST*)pInMcInst, gcvFALSE, &pOutCodecHelperInst->dst);

    /* Src */
    for (srcIdxOfHelperInst = 0; ; srcIdxOfHelperInst ++)
    {
        if (!_DecodeSrcWrapper(pOutCodecHelperInst, pMcCodec, &srcIdxOfMc, expectedMcSrcIdxMask, (VSC_MC_INST*)pInMcInst,
                               gcvFALSE, &pOutCodecHelperInst->src[srcIdxOfHelperInst]))
        {
            break;
        }

        pOutCodecHelperInst->srcCount = srcIdxOfHelperInst + 1;
    }

    /* Inst ctrl */
    pOutCodecHelperInst->instCtrl.instType = _DecodeInstType(pMcCodec, mcCodecType, (VSC_MC_INST*)pInMcInst);
    pOutCodecHelperInst->instCtrl.u.emitCtrl.bNeedRestartPrim = pInMcInst->inst.bNeedRestartPrim;
    pOutCodecHelperInst->instCtrl.u.emitCtrl.bNoJmpToEndOnMaxVtxCnt = pInMcInst->inst.bNoJmpToEndOnMaxVtxCnt;
    pOutCodecHelperInst->instCtrl.bResultSat = pInMcInst->inst.bResultSat;

    if (pMcCodec->pHwCfg->hwFeatureFlags.supportEndOfBBReissue)
    {
        pOutCodecHelperInst->instCtrl.bEndOfBB = pInMcInst->inst.bEndOfBB;
    }

    return gcvTRUE;
}

static gctBOOL _Decode_Mc_Conv_Inst(VSC_MC_CODEC* pMcCodec,
                                    VSC_MC_CODEC_TYPE mcCodecType,
                                    VSC_MC_CONV_INST* pInMcInst,
                                    VSC_MC_CODEC_INST* pOutCodecHelperInst)
{
    gctUINT           srcIdxOfHelperInst, srcIdxOfMc = 0;
    gctUINT           expectedMcSrcIdxMask = MC_SRC0_BIT | MC_SRC1_BIT;

    /* Opcode */
    pOutCodecHelperInst->baseOpcode = DECODE_BASE_OPCODE((VSC_MC_INST*)pInMcInst);

    /* Dst */
    pOutCodecHelperInst->bDstValid = _DecodeDst(pMcCodec, (VSC_MC_INST*)pInMcInst, gcvFALSE, &pOutCodecHelperInst->dst);

    /* Src */
    for (srcIdxOfHelperInst = 0; ; srcIdxOfHelperInst ++)
    {
        if (!_DecodeSrcWrapper(pOutCodecHelperInst, pMcCodec, &srcIdxOfMc, expectedMcSrcIdxMask, (VSC_MC_INST*)pInMcInst,
                               gcvFALSE, &pOutCodecHelperInst->src[srcIdxOfHelperInst]))
        {
            break;
        }

        pOutCodecHelperInst->srcCount = srcIdxOfHelperInst + 1;
    }

    /* Inst ctrl */
    pOutCodecHelperInst->instCtrl.instType = _DecodeInstType(pMcCodec, mcCodecType, (VSC_MC_INST*)pInMcInst);
    pOutCodecHelperInst->instCtrl.threadType = _DecodeThreadType(pMcCodec, mcCodecType, (VSC_MC_INST*)pInMcInst);
    pOutCodecHelperInst->instCtrl.bResultSat = ((VSC_MC_INST*)pInMcInst)->tri_srcs_alu_inst.inst.bResultSat;
    pOutCodecHelperInst->instCtrl.roundingMode = ((VSC_MC_INST*)pInMcInst)->tri_srcs_alu_inst.inst.roundMode;

    pOutCodecHelperInst->instCtrl.u.convCtrl.bEvisMode = pInMcInst->inst.bEvisMode;
    pOutCodecHelperInst->instCtrl.u.convCtrl.bDstPack = pInMcInst->inst.bDstPack;
    pOutCodecHelperInst->instCtrl.u.convCtrl.bSrcPack = pInMcInst->inst.bSrcPack;

    if (pMcCodec->pHwCfg->hwFeatureFlags.supportEndOfBBReissue)
    {
        pOutCodecHelperInst->instCtrl.bEndOfBB = pInMcInst->inst.bEndOfBB;
    }

    return gcvTRUE;
}

static gctBOOL _Decode_Mc_Scatter_Inst(VSC_MC_CODEC* pMcCodec,
                                       VSC_MC_CODEC_TYPE mcCodecType,
                                       VSC_MC_INST* pInMcInst,
                                       VSC_MC_CODEC_INST* pOutCodecHelperInst)
{
    gctUINT  expectedMcSrcIdxMask = MC_SRC0_BIT | MC_SRC1_BIT | MC_SRC2_BIT;
    gctUINT  srcIdxOfHelperInst, srcIdxOfMc = 0;

    gcmASSERT(mcCodecType == VSC_MC_CODEC_TYPE_SCATTER);

    pOutCodecHelperInst->baseOpcode = 0x45;
    pOutCodecHelperInst->extOpcode = 0x1F;

    /* Dst */
    pOutCodecHelperInst->bDstValid = _DecodeDst(pMcCodec, pInMcInst, gcvTRUE, &pOutCodecHelperInst->dst);

    if (pOutCodecHelperInst->bDstValid)
    {
        pOutCodecHelperInst->extOpcode = MC_AUXILIARY_OP_CODE_USC_SCATTER;
    }
    else
    {
        pOutCodecHelperInst->dst.u.evisDst.startCompIdx = pInMcInst->evis_inst.inst.startDstCompIdx;
        pOutCodecHelperInst->dst.u.evisDst.compIdxRange = pInMcInst->evis_inst.inst.endDstCompIdx - pInMcInst->evis_inst.inst.startDstCompIdx + 1;
    }

    /* Src */
    for (srcIdxOfHelperInst = 0; ; srcIdxOfHelperInst ++)
    {
        if (!_DecodeSrcWrapper(pOutCodecHelperInst, pMcCodec, &srcIdxOfMc, expectedMcSrcIdxMask, pInMcInst,
                               gcvTRUE, &pOutCodecHelperInst->src[srcIdxOfHelperInst]))
        {
            break;
        }

        pOutCodecHelperInst->srcCount = srcIdxOfHelperInst + 1;
    }

    /* Inst ctrl */
    pOutCodecHelperInst->instCtrl.bSkipForHelperKickoff = pInMcInst->load_inst.inst.bSkipForHelperKickoff;
    pOutCodecHelperInst->instCtrl.u.maCtrl.bAccessLocalStorage = pInMcInst->load_inst.inst.bAccessLocalStorage;

    /* EVIS insts have evis-state and sourceBin */
    pOutCodecHelperInst->instCtrl.u.visionCtrl.evisState = pInMcInst->evis_inst.inst.evisState;
    pOutCodecHelperInst->instCtrl.u.visionCtrl.startSrcCompIdx = pInMcInst->evis_inst.inst.startSrcCompIdx;

    pOutCodecHelperInst->instCtrl.instType = _DecodeInstType(pMcCodec, mcCodecType, pInMcInst);
    pOutCodecHelperInst->instCtrl.threadType = _DecodeThreadType(pMcCodec, mcCodecType, pInMcInst);

    return gcvTRUE;
}

static gctBOOL _Decode_Mc_Cmplx_Inst(VSC_MC_CODEC* pMcCodec,
                                     VSC_MC_CODEC_TYPE mcCodecType,
                                     VSC_MC_CMPLX_INST* pInMcInst,
                                     VSC_MC_CODEC_INST* pOutCodecHelperInst)
{
    gctUINT  expectedMcSrcIdxMask;

    if (pInMcInst->inst.subOpcode == 0x1)
    {
        expectedMcSrcIdxMask = MC_SRC0_BIT | MC_SRC1_BIT | MC_SRC2_BIT;
    }
    else if (pInMcInst->inst.subOpcode == 0x0)
    {
        expectedMcSrcIdxMask = MC_SRC0_BIT | MC_SRC1_BIT;
    }
    else
    {
        expectedMcSrcIdxMask = MC_SRC0_BIT | MC_SRC1_BIT | MC_SRC2_BIT;
    }

    gcmASSERT(mcCodecType == VSC_MC_CODEC_TYPE_CMPLX);

    pOutCodecHelperInst->instCtrl.bDenorm = pInMcInst->inst.bDenorm;
    pOutCodecHelperInst->extOpcode = pInMcInst->inst.subOpcode;

    return _Common_Decode_Mc_Alu_Inst(pMcCodec, mcCodecType, (VSC_MC_INST*)pInMcInst, expectedMcSrcIdxMask, pOutCodecHelperInst);
}

static PFN_MC_DECODER _pfn_mc_decoder[] =
{
    gcvNULL,
    (PFN_MC_DECODER)_Decode_Mc_No_Opnd_Inst,
    (PFN_MC_DECODER)_Decode_Mc_3_Srcs_Alu_Inst,
    (PFN_MC_DECODER)_Decode_Mc_3_Srcs_Alu_Inst, /* VSC_MC_CODEC_TYPE_3_SRCS_CC_ALU */
    (PFN_MC_DECODER)_Decode_Mc_2_Srcs_Src0_Src1_Alu_Inst,
    (PFN_MC_DECODER)_Decode_Mc_2_Srcs_Src0_Src1_Alu_Inst, /* VSC_MC_CODEC_TYPE_2_SRCS_SRC0_SRC1_CC_ALU */
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
    (PFN_MC_DECODER)_Decode_Mc_Conv_Inst,
    (PFN_MC_DECODER)_Decode_Mc_Scatter_Inst,
    (PFN_MC_DECODER)_Decode_Mc_3_Srcs_Alu_Inst,
    (PFN_MC_DECODER)_Decode_Mc_Cmplx_Inst,
};

static void _dbgStopHere(void)
{
    return ;
}

#define GotoError()  do { _dbgStopHere(); goto OnError; } while (0)

static gctBOOL _VerifyMCLegality(VSC_MC_CODEC* pMcCodec, VSC_MC_CODEC_INST* pCodecHelperInst)
{
    gctUINT  srcIdx, i, firstCstRegNo;
    gctBOOL  bFirstCstRegIndexing;
    gctUINT8 src2Format;
    gctBOOL  isVX2 = gcHWCaps.hwFeatureFlags.supportEVISVX2;
    gctBOOL  bEvisMode = (pCodecHelperInst->baseOpcode == 0x45);

    if (pCodecHelperInst->instCtrl.bResultSat)
    {
        /* Now only support destModifier for FLOAT32 or LOAD/STORE/IMG_STORE/I2I/CONV */
        if (pCodecHelperInst->instCtrl.instType != 0x0    &&
            pCodecHelperInst->baseOpcode != 0x32                        &&
            pCodecHelperInst->baseOpcode != 0x33                       &&
            pCodecHelperInst->baseOpcode != 0x39                       &&
            pCodecHelperInst->baseOpcode != 0x3A                      &&
            pCodecHelperInst->baseOpcode != 0x7A                   &&
            pCodecHelperInst->baseOpcode != 0x35                &&
            pCodecHelperInst->baseOpcode != 0x2C                         &&
            pCodecHelperInst->baseOpcode != 0x72                        &&
            pCodecHelperInst->baseOpcode != 0x45                        &&
            pCodecHelperInst->baseOpcode != MC_AUXILIARY_OP_CODE_USC_IMG_STORE          &&
            pCodecHelperInst->baseOpcode != MC_AUXILIARY_OP_CODE_USC_IMG_STORE_3D)
        {
            GotoError();
        }
    }

    if (pCodecHelperInst->instCtrl.instType == 0x1)
    {
        if (pCodecHelperInst->baseOpcode != 0x72 &&
            /* For MOV, it can support FLOAT16 only in dual-t. */
            !(pCodecHelperInst->baseOpcode == 0x09
              &&
              pMcCodec->bDual16ModeEnabled
              &&
              pCodecHelperInst->instCtrl.threadType == 0x0) &&
            pCodecHelperInst->baseOpcode != 0x2B &&
            pCodecHelperInst->baseOpcode != 0x32 &&
            pCodecHelperInst->baseOpcode != 0x39 &&
            pCodecHelperInst->baseOpcode != MC_AUXILIARY_OP_CODE_USC_STORE &&
            pCodecHelperInst->baseOpcode != MC_AUXILIARY_OP_CODE_USC_STOREP &&
            pCodecHelperInst->baseOpcode != MC_AUXILIARY_OP_CODE_USC_IMG_STORE &&
            pCodecHelperInst->baseOpcode != MC_AUXILIARY_OP_CODE_USC_IMG_STORE_3D &&
            pCodecHelperInst->baseOpcode != 0x33 &&
            pCodecHelperInst->baseOpcode != 0x3A &&
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
              pCodecHelperInst->extOpcode != 0x15 &&
              pCodecHelperInst->extOpcode != 0x18 &&
              pCodecHelperInst->extOpcode != 0x19 &&
              pCodecHelperInst->extOpcode != 0x1A &&
              pCodecHelperInst->extOpcode != 0x1B &&
              pCodecHelperInst->extOpcode != 0x1C &&
              pCodecHelperInst->extOpcode != 0x1D &&
              !(pCodecHelperInst->extOpcode == 0x0C && isVX2))))
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
                pCodecHelperInst->baseOpcode == 0x39 ||
                pCodecHelperInst->baseOpcode == 0x33 ||
                pCodecHelperInst->baseOpcode == 0x3A ||
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

            /* Except src2 of branch, all other imm src must be V16 imm under daul-t */
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
    if (pCodecHelperInst->srcCount >= 2 &&
        ((!pMcCodec->pHwCfg->hwFeatureFlags.noOneConstLimit)
         ||
         (bEvisMode && pCodecHelperInst->instCtrl.u.visionCtrl.bUseUniform512))
       )
    {
        for (i = 0; i < pCodecHelperInst->srcCount - 1; i ++)
        {
            firstCstRegNo = NOT_ASSIGNED;
            bFirstCstRegIndexing = gcvFALSE;

            for (srcIdx = 0; srcIdx < pCodecHelperInst->srcCount; srcIdx ++)
            {
                if (pCodecHelperInst->src[srcIdx].u.reg.bConstReg)
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

    /* Atom related inst, check the source2 swizzle if dest is valid.  */
    if (pMcCodec->pHwCfg->hwFeatureFlags.supportUSC &&
        pCodecHelperInst->bDstValid &&
        pCodecHelperInst->srcCount >= 2 &&
        IS_ATOMIC_MC_OPCODE(pCodecHelperInst->baseOpcode) &&
        pCodecHelperInst->src[2].regType != 0x7)
    {
        gctUINT swizzle[4];

        for (i = 0; i < 4; i++)
        {
            swizzle[i] = (((pCodecHelperInst->src[2].u.reg.swizzle) >> ((i) << 1)) & 0x3);
        }

        if (pCodecHelperInst->baseOpcode == 0x67)
        {
            if (!((swizzle[0] == swizzle[2]) && (swizzle[1] == swizzle[3])))
            {
                GotoError();
            }
        }
        else
        {
            if (!((swizzle[0] == swizzle[1]) && (swizzle[0] == swizzle[2]) && (swizzle[0] == swizzle[3])))
            {
                GotoError();
            }
        }
    }

    /* Image related inst, for elder chips, src0 must be constant reg */
    if (pCodecHelperInst->baseOpcode == 0x79 ||
        pCodecHelperInst->baseOpcode == 0x34 ||
        pCodecHelperInst->baseOpcode == 0x7A ||
        pCodecHelperInst->baseOpcode == 0x35 ||
        pCodecHelperInst->baseOpcode == 0x37 ||
        pCodecHelperInst->baseOpcode == 0x38 ||
        pCodecHelperInst->baseOpcode == MC_AUXILIARY_OP_CODE_USC_IMG_STORE_3D ||
        pCodecHelperInst->baseOpcode == MC_AUXILIARY_OP_CODE_USC_IMG_STORE)
    {
        if (!pMcCodec->pHwCfg->hwFeatureFlags.canSrc0OfImgLdStBeTemp)
        {
            if (pCodecHelperInst->src[0].regType != 0x2)
            {
                GotoError();
            }
        }
    }

    /* For USC related stores, USC must be armed on chips */
    if (pCodecHelperInst->baseOpcode == MC_AUXILIARY_OP_CODE_USC_STORE ||
        pCodecHelperInst->baseOpcode == MC_AUXILIARY_OP_CODE_USC_IMG_STORE ||
        pCodecHelperInst->baseOpcode == MC_AUXILIARY_OP_CODE_USC_IMG_STORE_3D ||
        pCodecHelperInst->baseOpcode == MC_AUXILIARY_OP_CODE_USC_STORE_ATTR ||
        pCodecHelperInst->extOpcode  == MC_AUXILIARY_OP_CODE_USC_SCATTER)
    {
        if (!pMcCodec->pHwCfg->hwFeatureFlags.supportUSC)
        {
            GotoError();
        }
    }

    if (IS_MEM_ACCESS_MC_OPCODE(pCodecHelperInst->baseOpcode))
    {
        /* For local memory l/s, USC must be armed on chips */
        if (pCodecHelperInst->instCtrl.u.maCtrl.bAccessLocalStorage)
        {
            if (!(pMcCodec->pHwCfg->maxLocalMemSizeInByte > 0))
            {
                GotoError();
            }
        }
    }

    /* For EVIS inst */
    if (pCodecHelperInst->baseOpcode == 0x45)
    {
        /* Not all HWs support EVIS mode */
        if (!pMcCodec->pHwCfg->hwFeatureFlags.supportEVIS)
        {
            GotoError();
        }

        if (pMcCodec->pHwCfg->hwFeatureFlags.useSrc0SwizzleAsSrcBin)
        {
            /* Src0 can not be imm */
            if (pCodecHelperInst->src[0].regType == 0x7)
            {
                GotoError();
            }
            else if (pCodecHelperInst->src[0].u.reg.swizzle != VIR_SWIZZLE_XXXX &&
                     pCodecHelperInst->src[0].u.reg.swizzle != VIR_SWIZZLE_XYYY &&
                     pCodecHelperInst->src[0].u.reg.swizzle != VIR_SWIZZLE_XYZZ &&
                     pCodecHelperInst->src[0].u.reg.swizzle != VIR_SWIZZLE_XYZW
                     )
            {
                /* the EVIS inst src0's swizzle bits are used for EVIS info like startBin,
                 * make sure the src0 operand does NOT use them
                 */
                GotoError();
            }
        }

        /* When src0 is temp256, src1’s swizzle has to be XYZW */
        if (pCodecHelperInst->src[0].regType == 0x1)
        {
            if (pCodecHelperInst->src[1].regType != 0x7)
            {
                if (pCodecHelperInst->src[1].u.reg.swizzle != 0xE4)
                {
                    GotoError();
                }
            }
        }

        /* For DPx related opcodes, src2 must be 4-consecutive-constant type */
        if (pCodecHelperInst->extOpcode == 0x08 ||
            pCodecHelperInst->extOpcode == 0x09 ||
            pCodecHelperInst->extOpcode == 0x0A ||
            pCodecHelperInst->extOpcode == 0x0B ||
            pCodecHelperInst->extOpcode == 0x12 ||
            pCodecHelperInst->extOpcode == 0x13 ||
            pCodecHelperInst->extOpcode == 0x14 ||
            pCodecHelperInst->extOpcode == 0x15 ||
            pCodecHelperInst->extOpcode == 0x16)
        {
            if (pCodecHelperInst->src[2].regType != 0x4)
            {
                GotoError();
            }
        }

        /* Bit-extract/Bit-replace only support u8/u16/u32 type */
        if (pCodecHelperInst->extOpcode == 0x10 ||
            pCodecHelperInst->extOpcode == 0x11)
        {
            if (pCodecHelperInst->instCtrl.instType != 0x7 &&
                pCodecHelperInst->instCtrl.instType != 0x6 &&
                pCodecHelperInst->instCtrl.instType != 0x5)
            {
                GotoError();
            }
        }

        /* MUL_SHIFT/SELECT_ADD cannot output more than 8 bin */
        if (pCodecHelperInst->extOpcode == 0x07 ||
            pCodecHelperInst->extOpcode == 0x0E)
        {
            if (!pCodecHelperInst->bDstValid ||
                pCodecHelperInst->dst.u.evisDst.compIdxRange > 8)
            {
                GotoError();
            }
        }
        /* BI_LINEAR cannot output more than 7 bin */
        if (pCodecHelperInst->extOpcode == 0x0D)
        {
            if (!pCodecHelperInst->bDstValid ||
                pCodecHelperInst->dst.u.evisDst.compIdxRange > 7)
            {
                GotoError();
            }
        }

        /* Specials for evis-atomic-add */
        if (pCodecHelperInst->extOpcode == 0x0F)
        {
            /* SourceBin must be 0 */
            if (pCodecHelperInst->instCtrl.u.visionCtrl.startSrcCompIdx != 0)
            {
                GotoError();
            }

            /* Format of src2 has special requiremnt for different inst-type */
            src2Format = pCodecHelperInst->instCtrl.u.visionCtrl.evisState & 0x7;
            if (pCodecHelperInst->instCtrl.instType == 0x7)
            {
                if (src2Format != 0x7)
                {
                    GotoError();
                }
            }
            else if (pCodecHelperInst->instCtrl.instType == 0x6)
            {
                if (src2Format != 0x7 &&
                    src2Format != 0x6)
                {
                    GotoError();
                }
            }
            else if (pCodecHelperInst->instCtrl.instType == 0x5)
            {
                if (src2Format != 0x7 &&
                    src2Format != 0x6 &&
                    src2Format != 0x5)
                {
                    GotoError();
                }
            }
            else if (pCodecHelperInst->instCtrl.instType == 0x2)
            {
                if (src2Format == 0x5)
                {
                    GotoError();
                }
            }
        }
    }

    /* Not all HWs support EVIS mode */
    if (pCodecHelperInst->baseOpcode == 0x79 ||
        pCodecHelperInst->baseOpcode == 0x34 ||
        pCodecHelperInst->baseOpcode == 0x7A ||
        pCodecHelperInst->baseOpcode == 0x35 ||
        pCodecHelperInst->baseOpcode == MC_AUXILIARY_OP_CODE_USC_IMG_STORE ||
        pCodecHelperInst->baseOpcode == MC_AUXILIARY_OP_CODE_USC_IMG_STORE_3D)
    {
        if (pCodecHelperInst->instCtrl.u.maCtrl.bUnderEvisMode &&
            !pMcCodec->pHwCfg->hwFeatureFlags.supportEVIS)
        {
            GotoError();
        }
    }

    /* Not all HWs support FMA/HALF_MIX */
    if (pCodecHelperInst->baseOpcode == 0x30 ||
        (pCodecHelperInst->baseOpcode == 0x7F &&
         pCodecHelperInst->extOpcode == 0x0F))
    {
        if (!pMcCodec->pHwCfg->hwFeatureFlags.supportAdvancedInsts)
        {
            GotoError();
        }
    }

    /* Not all HWs support img_atom */
    if (pCodecHelperInst->baseOpcode == 0x46)
    {
        if (!pMcCodec->pHwCfg->hwFeatureFlags.supportImgAtomic)
        {
            GotoError();
        }
    }

    /* For TEXLD_U related insts, src2 of MC must be an imm. This is a pure SW limitation, and does
       not impact HW. The reason of this SW limitation is because HW uses src2 to parse TEXLD_U mode
       to differenciate MC_AUXILIARY_OP_CODE_TEXLD_U_LOD and MC_AUXILIARY_OP_CODE_TEXLD_U_BIAS (as well
       as MC_AUXILIARY_OP_CODE_TEXLD_U_PLAIN), but for mc-decoder, if src2 of MC is not an imm src, we
       then can not differenciate them, so we limit src2 of MC to be imm */
    if (pCodecHelperInst->baseOpcode >= MC_AUXILIARY_OP_CODE_TEXLD_U_PLAIN &&
        pCodecHelperInst->baseOpcode <= MC_AUXILIARY_OP_CODE_TEXLD_U_BIAS)
    {
        if (pCodecHelperInst->baseOpcode == MC_AUXILIARY_OP_CODE_TEXLD_U_PLAIN)
        {
            /* Src2 must be imm */
            if (pCodecHelperInst->src[2].regType != 0x7)
            {
                GotoError();
            }
        }

        if (pCodecHelperInst->baseOpcode == MC_AUXILIARY_OP_CODE_TEXLD_U_LOD ||
            pCodecHelperInst->baseOpcode == MC_AUXILIARY_OP_CODE_TEXLD_U_BIAS)
        {
            /* Src3 must be imm */
            if (pCodecHelperInst->src[3].regType != 0x7)
            {
                GotoError();
            }
        }
    }

    /* Dst of MOVBIX is a scalar (only x channel) */
    if (pCodecHelperInst->baseOpcode == 0x7F &&
        pCodecHelperInst->extOpcode == 0x13)
    {
        if (!pMcCodec->pHwCfg->hwFeatureFlags.supportVectorB0 &&
            pCodecHelperInst->dst.u.nmlDst.writeMask != WRITEMASK_X)
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
                      gctBOOL bDual16ModeEnabled,
                      gctBOOL bCanReverseEngineCheck)
{
    gcmASSERT(pHwCfg->hwFeatureFlags.supportDual16 || !bDual16ModeEnabled);

    pMcCodec->pHwCfg = pHwCfg;
    pMcCodec->bInit = gcvTRUE;
    pMcCodec->bDual16ModeEnabled = bDual16ModeEnabled;
    pMcCodec->bCanReverseEngineCheck = bCanReverseEngineCheck;
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
            baseOpcode != 0x3A &&
            baseOpcode != 0x7A &&
            baseOpcode != 0x35 &&
            baseOpcode != 0x42 &&
            !(baseOpcode == 0x45 && extOpcode == 0x1F))
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
    gctBOOL           bDecodeSucc;
#if (_DEBUG || DEBUG)
    VSC_MC_RAW_INST   reMcInst;
#endif

    gcmASSERT(pMcCodec->bInit);

    /* Initialize resultant output MC inst to 0 */
    memset(pOutCodecHelperInst, 0, sizeof(VSC_MC_CODEC_INST));

    /* HW defines PACKED is 0, and UNPACKED is 1, so we need set packmode to UNPACKED by default */
    pOutCodecHelperInst->instCtrl.packMode = 0x1;

    baseOpcode = DECODE_BASE_OPCODE(pMcInst);
    extOpcode = DECODE_EXT_OPCODE(pMcInst, baseOpcode);

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
    bDecodeSucc = _pfn_mc_decoder[mcCodecType](pMcCodec, mcCodecType, pMcInst, pOutCodecHelperInst);

#if (_DEBUG || DEBUG)
    /* We try to send decoded helper-inst into encoder to get a new mc-inst, then
       compare this new mc-inst with coming-in mc-inst. We expect these two mc-insts
       are totally same. */
    if (pMcCodec->bCanReverseEngineCheck)
    {
        vscMC_EncodeInst(pMcCodec, pOutCodecHelperInst, &reMcInst);
        gcmASSERT(memcmp(&reMcInst, pInMCInst, sizeof(VSC_MC_RAW_INST)) == 0);
    }
#endif

    return bDecodeSucc;
}

gctBOOL vscMC_EncodeSrc(VSC_MC_CODEC*     pMcCodec,
                        VSC_MC_CODEC_SRC* pSrc,
                        gctBOOL           bEvisMode,
                        gctUINT           mcSrcIdx,
                        VSC_MC_RAW_INST*  pOutMCInst)
{
    if (mcSrcIdx >= 3)
    {
        return gcvFALSE;
    }

    _EncodeSrc(pMcCodec, mcSrcIdx, pSrc, bEvisMode, (VSC_MC_INST*)pOutMCInst);

    return gcvTRUE;
}

gctBOOL vscMC_DecodeSrc(VSC_MC_CODEC_INST*            pOutCodecHelperInst,
                        VSC_MC_CODEC*                 pMcCodec,
                        VSC_MC_RAW_INST*              pInMCInst,
                        gctBOOL                       bEvisMode,
                        gctUINT                       mcSrcIdx,
                        VSC_MC_CODEC_SRC*             pOutSrc)
{
    if (mcSrcIdx >= 3)
    {
        return gcvFALSE;
    }

    return _DecodeSrc(pOutCodecHelperInst, pMcCodec, mcSrcIdx, (VSC_MC_INST*)pInMCInst, bEvisMode, pOutSrc);
}

gctUINT vscMC_GetFreeSrcCount(VSC_MC_CODEC*    pMcCodec,
                              VSC_MC_RAW_INST* pMCInst,
                              gctUINT*         pOutMcSrcIdxArray)
{
    gctUINT  freeSrcCount = 0;

    VSC_MC_INST* pMcInst = (VSC_MC_INST*)pMCInst;

    if (!pMcInst->tri_srcs_alu_inst.inst.bSrc0Valid)
    {
        pOutMcSrcIdxArray[freeSrcCount ++] = 0;
    }

    if (!pMcInst->tri_srcs_alu_inst.inst.bSrc1Valid)
    {
        pOutMcSrcIdxArray[freeSrcCount ++] = 1;
    }

    if (!pMcInst->tri_srcs_alu_inst.inst.bSrc2Valid)
    {
        pOutMcSrcIdxArray[freeSrcCount ++] = 2;
    }

    return freeSrcCount;
}


