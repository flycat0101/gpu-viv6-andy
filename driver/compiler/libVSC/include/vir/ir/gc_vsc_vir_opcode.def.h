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


#define HasDest             VIR_OPFLAG_HasDest
#define NoDest              VIR_OPFLAG_NoDest
#define Transcendental      VIR_OPFLAG_Transcendental
#define IntegerOnly         VIR_OPFLAG_IntegerOnly
#define ControlFlow         VIR_OPFLAG_ControlFlow
#define VX1                 VIR_OPFLAG_VX1
#define VX2                 VIR_OPFLAG_VX2
#define VX1_2               VIR_OPFLAG_VX1_2
#define UseCondCode         VIR_OPFLAG_UseCondCode
#define Componentwise       VIR_OPFLAG_Componentwise
#define Src0Componentwise   VIR_OPFLAG_Src0Componentwise
#define Src1Componentwise   VIR_OPFLAG_Src1Componentwise
#define Src2Componentwise   VIR_OPFLAG_Src2Componentwise
#define Src3Componentwise   VIR_OPFLAG_Src3Componentwise
#define Src01Componentwise  (Src0Componentwise | Src1Componentwise)
#define Src012Componentwise (Src0Componentwise | Src1Componentwise | Src2Componentwise)
#define Src12Componentwise  (Src1Componentwise | Src2Componentwise)
#define OnlyUseEnable       VIR_OPFLAG_OnlyUseEnable
#define Loads               VIR_OPFLAG_Loads
#define Stores              VIR_OPFLAG_Stores
#define Expr                VIR_OPFLAG_Expression
#define EPFromHighest       VIR_OPFLAG_ExpdPrecFromHighest
#define EPFromS0            VIR_OPFLAG_ExpdPrecFromSrc0
#define EPFromS12           VIR_OPFLAG_ExpdPrecFromSrc12
#define EPFromS2            VIR_OPFLAG_ExpdPrecFromSrc2
#define EPHP                VIR_OPFLAG_ExpdPrecHP
#define EPMP                VIR_OPFLAG_ExpdPrecMP
#define Use512BitUniform    VIR_OPFLAG_Use512Unifrom
#define VXUse512BitUniform(SrcNo)  (VX1_2|VIR_SrcNo2U512(SrcNo))
#define VX1Use512BitUniform(SrcNo) (VX1|VIR_SrcNo2U512(SrcNo))
#define EVISModifier(SrcNo)  VIR_SrcNo2EVISModifier(SrcNo)

#define NU  VIR_OPLEVEL_NotUsed
#define HL  VIR_OPLEVEL_High
#define ML  VIR_OPLEVEL_Medium
#define HM  VIR_OPLEVEL_HighMedium
#define LL  VIR_OPLEVEL_Low
#define MC  VIR_OPLEVEL_Machine
#define NM  VIR_OPLEVEL_NotMachine
#define LM  VIR_OPLEVEL_LowUnder
#define AL  VIR_OPLEVEL_All

/*  [VIR_OP_]CODENAME, NumberOfSource, flags, WriteToDest, OpCodeLevel */
    VIR_OPINFO(NOP, 0, NoDest, 0, AL), /* no op  */

    /**
     **  data move operation
     **/
    /* move data */
    VIR_OPINFO(MOV, 1, HasDest|Componentwise|Expr|EPFromS0, 1, AL),
    /* set call parameter:
    ** It must immediately precede a VIR_OP_CALL instruction.
    ** The dest holds the function label.
    ** The source0 holds a argument list operand.
    **  if the return type of this function is not VOID, also need to put the return value to the argument list.
    */
    VIR_OPINFO(PARM, 1, HasDest|Componentwise|EPFromS0, 1, HL),
    /* @ conditional move: dst = cond_op(s0, s1) ? s2 : dst; */
    VIR_OPINFO(CMOV, 3, HasDest|Componentwise|Expr|EPFromS2|UseCondCode, 1, AL),
    /* @ MOVAR, MOVAF, MOVAI, mov dynamic indexing value to a0 */
    VIR_OPINFO(MOVA, 1, HasDest|Componentwise|Expr|VIR_OPFLAG_ExpdPrecHP, 1, LM),
    /* Mov address to b0 with address calculation, b0 is used for uniform dynamic indexing
     *  addr[9:0]= base + (index<<shift) *(mul3?3:1)
     *  write the addr[9:0] to B0, where B0 is similar to A0, but one per shader group.
     *  B0 is one component S10 and use X writemask to write. There is no data conversion.
     */
    VIR_OPINFO(MOVBIX, 2, HasDest|Componentwise|Expr|VIR_OPFLAG_ExpdPrecHP, 1, LM),
    /* @ swizzle vector 8/16 components in src0 by the swizzle defined in src1, 16 bit enable mask in src2
     *   SWIZZLE dest, src0, src1, src2
     *   Vector 8 swizzling:
     *      if (src2.x'2*i+1:2*i == 0) ((short*)&dest)[i] = ((short*)&src0)[src1.x'4*i+2:4*i]
     *   Vector 16 swizzling:
     *      if (src2.x'i:i == 0) ((char*)&dest)[i] = ((char*)&src0)[src1.x'4*i+3:4*i]  for i in [7, 0]
     *      if (src2.x'i:i == 0) ((char*)&dest)[i] = ((char*)&src0)[src1.y'4*(8-i)+3:4*(8-i)]  for i in [15, 8]
     */
    VIR_OPINFO(SWIZZLE, 3, HasDest|Expr|EPFromS0, 1, AL),
    /* @ shuffle vector components */
    VIR_OPINFO(SHUFFLE, 2, HasDest|Expr|EPFromS0, 1, NU),
    /* @ Shuffle components from register pair */
    VIR_OPINFO(SHUFFLE2, 3, HasDest|Expr|EPFromS0, 1, NU),
    /* @ set predicator */
    VIR_OPINFO(SETP, 2, HasDest|Componentwise|Expr|EPFromHighest, 1, NU),
    /* compare each component
        dest = is_float(dest.type) ? ((cond_op(src0, src1) ? 1.0 : 0.0)
                                   : ((cond_op(src0, src1) ? 0xFFFFFFFF: 0) ;*/
    VIR_OPINFO(COMPARE, 2, HasDest|Componentwise|Expr|EPMP|UseCondCode, 1, NM),
    /* machine instruction: Result = CMP(compFunc, source_0, source_1) ? source_2 : 0; */
    VIR_OPINFO(CMP, 3, HasDest|Componentwise|Expr|EPFromS2|UseCondCode, 1, MC),
    /* machine instruction: Result = CMP(compFunc, source_0, source_1) ? 1.0 : 0.0; */
    VIR_OPINFO(SET, 2, HasDest|Componentwise|Expr|UseCondCode, 1, MC),
    /* @ pack a vector from SRC0, SRC1, SRC2, */
    VIR_OPINFO(PACK, 3, HasDest|Expr|EPFromHighest, 1, LM),
    /* move long/ulong data */
    VIR_OPINFO(MOV_LONG, 2, HasDest|Componentwise|Expr|EPFromHighest, 1, AL),
    /* copy data, COPY dest, source, byteSize */
    VIR_OPINFO(COPY, 2, HasDest|Componentwise|Expr|EPFromS0, 1, AL),

    /**
     ** type conversion
     **/
    /* convert source to dest type   */
    VIR_OPINFO(CONVERT, 1, HasDest|Componentwise|Expr|EPHP, 1, NM),
    /* convert source to dest type, only used in converter to match gcSL_CONV */
    VIR_OPINFO(CONV0, 2, HasDest|Componentwise|Expr|EPHP, 1, HM),
    /* Machine instruction: convert source to dest type, source type is encoded in src1   */
    VIR_OPINFO(CONV, 2, HasDest|Componentwise|Expr|EPHP, 1, MC),
    /* Machine instruciton: convert source to dest type,
       Destination data type is determined by source_1.x[7:4].
       With dual-16 mode the Destination data type for I2I instructions
       (provided as source_1.x[6:4]) must be an immediate.*/
    VIR_OPINFO(I2I, 2, HasDest|Componentwise|Expr|EPFromS0, 1, MC),
    /* Machine instruction: Integer to float convert, */
    VIR_OPINFO(I2F, 1, HasDest|Componentwise|Expr|EPFromS0, 1, MC),
    /* Machine instruction: Float to integer convert,
       RTNE supported, FP16 valid
       RTNE not implemented (as of 5.4.4) for dual-16 mode.  */
    VIR_OPINFO(F2I, 1, HasDest|Componentwise|Expr|EPFromS0, 1, MC),
    /* Machine instruction: Float to integer convert with rounding,
       RTNE supported, FP16 valid.
       RTNE not implemented (as of 5.4.4) for dual-16 mode. */
    VIR_OPINFO(F2IRND, 1, HasDest|Componentwise|Expr|EPFromS0, 1, MC),

    /* treat bit as casted to type, bits unchanged */
    VIR_OPINFO(BITCAST, 1, HasDest|Componentwise|Expr|EPFromS0, 1, NM),

    /**
     ** one operator operations
     **/
    /* saturate src by saturate mode: SAT dest, src0  */
    VIR_OPINFO(SAT, 1, HasDest|Componentwise|Expr|EPFromS0, 1, AL),
    /* get absolute value: ABS dest, src0  */
    VIR_OPINFO(ABS, 1, HasDest|Componentwise|Expr|EPFromS0, 1, AL),
    /* get negate value: NEG dest, src  */
    VIR_OPINFO(NEG, 1, HasDest|Componentwise|Expr|EPFromS0, 1, AL),
    /* get floor value:  FLOOR dest, src0  */
    VIR_OPINFO(FLOOR, 1, HasDest|Componentwise|Expr|EPFromS0, 1, AL),
    /* get fix value:  FIX dest, src0  */
    VIR_OPINFO(FIX, 1, HasDest|Componentwise|Expr|EPFromS0, 1, AL),
    /* get ceil value:  CEIL dest, src0  */
    VIR_OPINFO(CEIL, 1, HasDest|Componentwise|Expr|EPFromS0, 1, AL),
    /* base-2 logarithm of abs(src0), att least 21 bits of precision
         LOG dest.x, src0.x  ==>
            PRE_LOG2 tmp.xy, src0.x
            MUL dest.x, tmp.x, tmp.y
     */
    VIR_OPINFO(LOG2, 1, HasDest|Componentwise|Expr|EPFromS0, 1, HM),
    /* first step of base-2 logarithm of abs(src0), src0 is scalar float type */
    VIR_OPINFO(PRE_LOG2, 1, HasDest|Transcendental|Componentwise|Expr|EPFromS0, 1, LM),
    /* full precision 2 power X:  EXP dest, src0 */
    VIR_OPINFO(EXP2, 1, HasDest|Transcendental|Componentwise|Expr|EPFromS0, 1, AL),
    /* sign of operator: Integer: 0 -> 0, positive -> 1, negative -> -1
                         Float:  +/-0.0 ->+/-0.0, nan -> nan,
                                 positive -> 1.0, negative -> -1.0 */
    VIR_OPINFO(SIGN, 1, HasDest|Componentwise|Expr|EPMP, 1, AL),
    /* get fraction value of float source: FRAC dest, src */
    VIR_OPINFO(FRAC, 1, HasDest|Componentwise|Expr|EPFromS0, 1, AL),
    /* compute reciprocal value: RCP dest, src */
    VIR_OPINFO(RCP, 1, HasDest|Transcendental|Componentwise|Expr|EPFromS0, 1, AL),
    /* compute reciprocal square root: RSQ dest, src */
    VIR_OPINFO(RSQ, 1, HasDest|Transcendental|Componentwise|Expr|EPFromS0, 1, AL),
    /* compute square root: SQRT dest, src */
    VIR_OPINFO(SQRT, 1, HasDest|Transcendental|Componentwise|Expr|EPFromS0, 1, AL),
    /* compute normalization value: NORM dest, src0  */
    VIR_OPINFO(NORM, 1, HasDest|Expr|EPFromS0, 1, AL),
    /* Compute the rate of change in render target's x-direction.
          DSX dest, src0 */
    VIR_OPINFO(DSX, 1, HasDest|Componentwise|Expr|EPFromS0, 1, AL),
    /* Compute the rate of change in render target's y-direction.
          DSY dest, src0*/
    VIR_OPINFO(DSY, 1, HasDest|Componentwise|Expr|EPFromS0, 1, AL),
    /* Returns the sum of the absolute derivative in x and y
       using local differencing for the input argument p, i.e.,
       abs (dFdx (p)) + abs (dFdy (p));
          FWDITH dest, src0   ==>
            DSX TEMP1, src0
            DSY TEMP2, src0
            ADD dest, |TEMP1|,|TEMP2|

     */
    VIR_OPINFO(FWIDTH, 1, HasDest|Expr|EPFromS0, 1, HM),
    /* get lower 4 bytes of long/ulong integer */
    VIR_OPINFO(LONGLO, 1, HasDest|Expr, 1, AL),
    /* get upper 4 bytes of long/ulong integer */
    VIR_OPINFO(LONGHI, 1, HasDest|Expr, 1, AL),

    /**
     ** Relational operations
     **/
    VIR_OPINFO(ANY, 1, HasDest|Expr|EPMP, 1, NM), /* Returns true if any component of src0 is true. */
    VIR_OPINFO(ALL, 1, HasDest|Expr|EPMP, 1, NM), /* Returns true only if all components of src0 are true. */

    /**
     ** Trigonometry operations
     **/
    /* compute sine value:
         SIN dest.x, src0.x ==>
            MUL    tmp0.x, src0.x, PI or PI/2
            SINPI  tmp1.xy, tmp0.x
            MUL    dest.x, tmp1.x, tmp1.y
     */
    VIR_OPINFO(SIN, 1, HasDest|Componentwise|Expr|EPFromS0, 1, HM),
    /* compute cosine value:
         COS dest.x, src0.x ==>
            MUL    tmp0.x, src0.x, PI or PI/2
            COSPI  tmp1.xy, tmp0.x
            MUL    dest.x, tmp1.x, tmp1.y
     */
    VIR_OPINFO(COS, 1, HasDest|Componentwise|Expr|EPFromS0, 1, HM),
    /* compute tangent value:
         TAN dest.x, src0.x  ==>
            MUL tmp1.x, src0, PI or PI/2
            COSPI tmp2.xy, tmp1.x
            MUL   tmp2.x, tmp2.x, tmp2.y
            RCP   tmp2.x, tmp2.x
            SINPI tmp3.xy, tmp1.x
            MUL   tmp3.x, tmp3.x, tmp3.y
            MUL   dest.x, tmp3.x, tmp2.x
     */
    VIR_OPINFO(TAN, 1, HasDest|Componentwise|Expr|EPFromS0, 1, HM),
    /* compute arc consine value:
         ACOS dest, src0    ==>
            ACOS(x) = 1/2 pi - (x + 1/6 x^3 + 3/40 x^5 + 5/112 x^7 + 35/1152 x^9)
                    = 1/2 pi - (x (1 + x^2 (1/6 + x^2 (3/40 + x^2 (5/112 + 35/1152 x^2)))))

            MUL tmp1, src0, src0
            MAD tmp2, tmp1, asin9, asin7
            MAD tmp2, tmp1, tmp2, asin5
            MAD tmp2, tmp1, tmp2, asin3
            MAD tmp2, tmp1, tmp2, one
            MAD dest, src0, -tmp2, half_pi

     */
    VIR_OPINFO(ACOS, 1, HasDest|Componentwise|Expr|EPFromS0, 1, HM),
    /* compute arc sine value:
         ASIN dest, src0     ==>
            ASIN(x) = x + 1/6 x^3 + 3/40 x^5 + 5/112 x^7 + 35/1152 x^9
                    = x (1 + x^2 (1/6 + x^2 (3/40 + x^2 (5/112 + 35/1152 x^2))))

            MUL tmp1, src0, src0
            MAD tmp2, tmp1, asin9, asin7
            MAD tmp2, tmp1, tmp2, asin5
            MAD tmp2, tmp1, tmp2, asin3
            MAD tmp2, tmp1, tmp2, one
            MUL dest, src0, tmp2
     */
    VIR_OPINFO(ASIN, 1, HasDest|Componentwise|Expr|EPFromS0, 1, HM),
    /* compute arc tangent value:
         ATAN dest, src0  ==>
            if (|x| > 1) flag = 1; x = 1 / x; else flag = 0;

                set.gt TEMP1, |x|, 1
                rcp TEMP2, x
                select.nz TEMP2, TEMP1, TEMP2, x

            atan(x) = x - 1/3 x^3 + 1/5 x^5 - 1/7 x^7 + 1/9 x^9
                    = x (1 + x^2 (-1/3 + x^2 (1/5 + x^2 (-1/7 + 1/9 x^2 ) ) ) )

                mul TEMP3, TEMP2, TEMP2
                mad 1, TEMP3, atan9, -atan7
                mad 1, TEMP3, 1, atan5
                mad 1, TEMP3, 1, -atan3
                mad 1, TEMP3, 1, one
                mul 1, TEMP2, 1, 0

            if (x < 0) t2 = -pi/2 - abs(atan); else t2 = pi/2 - abs(atan);

                add TEMP2, PI/2, 0, |atan|
                select.lt TEMP2, x, -TEMP2, TEMP2

            return flag ? t2 : atan;

                select.nz 1, TEMP1, TEMP2, 1
     */
    VIR_OPINFO(ATAN, 1, HasDest|Componentwise|Expr|EPFromS0, 1, HM),
    /* compute tanget pi value:
         TANPI dest.x, src0.x  ==>
            COSPI tmp1.xy, src0.x
            MUL   tmp1.x, tmp1.x, tmp1.y
            RCP   tmp1.x, tmp1.x
            SINPI tmp2.xy, src0.x
            MUL   tmp2.x, tmp2.x, tmp2.y
            MUL   dest.x, tmp2.x, tmp1.x
     */
    VIR_OPINFO(TANPI, 1, HasDest|Componentwise|Expr|EPFromS0, 1, HM),
    /* compute sine pi or half pi value sin(PI[*1/2] * src)
        pi or pi/2 depents on feature bit _hasNEW_SIN_COS_LOG_DIV */
    VIR_OPINFO(SINPI, 1, HasDest|Transcendental|Componentwise|Expr|EPFromS0, 1, LM),
    /* compute cosine pi or half pi value cos(PI[*1/2] * src)
        pi or pi/2 depents on feature bit _hasNEW_SIN_COS_LOG_DIV */
    VIR_OPINFO(COSPI, 1, HasDest|Transcendental|Componentwise|Expr|EPFromS0, 1, LM),
    /* The instruction to speed up aSin/aCos/aTan/aTan2, enabled by gcvFEATURE_HALTI5 (v60)
     *  opcode directly from src2,
     *         src2[7] for CORNER_FLAG (implied, Indicate the corner cases handling)
     *         src2[2:0] = 0 ----  ACOS(opcode 8)
     *                   = 1 ----  ASIN(opcode 9)
     *                   = 2 ----  ATAN(opcode 10)
     *                   = 3 ----  ATAN2(opcode 11)
     * See P4:\\DOC\ARCH\ProposalOfACos.docx for detail
     */
    VIR_OPINFO(ARCTRIG, 3, HasDest|Expr, 1, LM),

    /**
     ** two operator operations
     **/
    /* addition : ADD dest, src0, src1 */
    VIR_OPINFO(ADD, 2, HasDest|Componentwise|Expr|EPFromHighest, 1, AL),
    /* subtraction: SUB dest, arc0, src1 */
    VIR_OPINFO(SUB, 2, HasDest|Componentwise|Expr|EPFromHighest, 1, AL),
    /* multiply: MUL dest, src0, src1  */
    VIR_OPINFO(MUL, 2, HasDest|Componentwise|Expr|EPFromHighest, 1, AL),
    /* special multiply: MUL dest, src0, src1, which inf * 0 results in 0 */
    VIR_OPINFO(MUL_Z, 2, HasDest|Componentwise|Expr|EPFromHighest, 1, AL),
    /* integer satuarated addition : ADDSAT dest, src0, src1 */
    VIR_OPINFO(ADDSAT, 2, HasDest|Componentwise|Expr|EPFromHighest, 1, AL),
    /* integer satuarated subtraction: SUBSAT dest, arc0, src1 */
    VIR_OPINFO(SUBSAT, 2, HasDest|Componentwise|Expr|EPFromHighest, 1, AL),
    /* integer satuarated multiply: MULSAT dest, src0, src1  */
    VIR_OPINFO(MULSAT, 2, HasDest|Componentwise|Expr|EPFromHighest, 1, AL),

    /* pre-norm multiply: NOR_MUL dest, src0, src1  */
    VIR_OPINFO(NORM_MUL, 2, HasDest|Componentwise|Expr|EPFromHighest, 1, LM),

    /* float division:
         DIV dest.x, src0.x, src1.x   ==>
            PRE_DIV tmp.xy, src0.x, src1.x
            MUL     dest.x, tmp.x, tmp.y
     */
    VIR_OPINFO(DIV, 2, HasDest|Transcendental|Componentwise|Expr|EPFromHighest, 1, AL),

    /* pre division: float32 scalar only */
    VIR_OPINFO(PRE_DIV, 2, HasDest|Transcendental|Componentwise|Expr|EPFromHighest, 1, LM),
    /* modulus:
        MOD dest, src0, src1    ==>
        rem(x, y) = x - y * floor(x / y)
    */
    VIR_OPINFO(MOD, 2, HasDest|Componentwise|Expr|EPFromHighest, 1, NM),
    /* remainder:
        REM dest, src0, src1    ==>
        rem(x, y) = x - y * fix(x / y)
    */
    VIR_OPINFO(REM, 2, HasDest|Componentwise|Expr|EPFromHighest, 1, NM),
    /* modulus: IMOD dest, src0, src1 */
    VIR_OPINFO(IMOD, 2, HasDest|Transcendental|Componentwise|Expr|EPFromHighest, 1, MC),
    /* maximum: MAX dest, src0, src1 */
    VIR_OPINFO(MAX, 2, HasDest|Componentwise|Expr|EPFromHighest, 1, AL),
    /* minimum: MIN dest, src0, src1 */
    VIR_OPINFO(MIN, 2, HasDest|Componentwise|Expr|EPFromHighest, 1, AL),
    /* Add results lower part. Usually a floating adder will
       result in cutting off mantisa of more than 24bits,
       however ADDLO will extend the adder operation’s
       mantisa to 46 bits and return the lower 23 bit
       mantisa of the result. Float only.
           ADDLO dest, src0, src0 */
    VIR_OPINFO(ADDLO, 2, HasDest|Componentwise|Expr, 1, AL),
    /* Multiply results lower part. Usually multiply will
       result in cutting off mantisa of more than 24bits,
       however MULLO will return the 24th-46th (lower part)
       mantisa of the result. Integer only.
           MULLO dest, src0, src1 */
    VIR_OPINFO(MULLO, 2, HasDest|Componentwise|Expr|EPFromHighest, 1, AL),
    /* high part of multiply. Integer only.
           MULHI dest, src0, src1 */
    VIR_OPINFO(MULHI, 2, HasDest|Componentwise|Expr|EPFromHighest, 1, AL),
    /* return src0 raise to the power of src1: POW dest, src0, src1  */
    VIR_OPINFO(POW, 2, HasDest|Componentwise|Expr|EPFromHighest, 1, AL),
    /* compute dot product: DOT dest, src0, src1  */
    VIR_OPINFO(DOT, 2, HasDest|Expr|EPFromHighest, 1, HM),
    /* compute vec2 dot product: DP2 dest, src0, src1  */
    VIR_OPINFO(DP2, 2, HasDest|Expr|EPFromHighest, 1, AL),
    /* compute vec3 dot product: DP3 dest, src0, src1  */
    VIR_OPINFO(DP3, 2, HasDest|Expr|EPFromHighest, 1, AL),
    /* compute vec4 dot product: DP4 dest, src0, src1  */
    VIR_OPINFO(DP4, 2, HasDest|Expr|EPFromHighest, 1, AL),
    /* compute pre-norm vec2 dot product: NORM_DP2 dest, src0  */
    VIR_OPINFO(NORM_DP2, 1, HasDest|Expr|EPFromHighest, 1, LM),
    /* compute pre-norm vec3 dot product: NORM_DP3 dest, src0  */
    VIR_OPINFO(NORM_DP3, 1, HasDest|Expr|EPFromHighest, 1, LM),
    /* compute pre-norm vec4 dot product: NORM_DP4 dest, src0  */
    VIR_OPINFO(NORM_DP4, 1, HasDest|Expr|EPFromHighest, 1, LM),
    /* compute cross product
         | x[1].y[2] ? y[1].x[2] |
         | x[2].y[0] ? y[2].x[0] |
         | x[0].y[1] ? y[0].x[1] |
           CROSS dest, src0, src1 */
    VIR_OPINFO(CROSS, 2, HasDest|Expr|EPFromHighest, 1, AL),
    /* Returns 1.0 if src0 <= src1 (edge, AL), otherwise it returns 0.0
           STEP  dest, src0, src1*/
    VIR_OPINFO(STEP, 2, HasDest|Componentwise|Expr, 1, AL),
    /* @ Calculates distance of two vectors:
           DST dest, src0, src1
     */
    VIR_OPINFO(DST, 2, HasDest|Expr|EPFromHighest, 1, AL),
    /* Calculates length of vector:
           LENGTH dest, src0
     */
    VIR_OPINFO(LENGTH, 1, HasDest|Expr|EPFromHighest, 1, HL),
    /* bitwise and: AND_BITWISE dest, src0, src1 */
    VIR_OPINFO(AND_BITWISE, 2, HasDest|Componentwise|Expr|EPFromHighest, 1, AL),
    /* bitwise or: OR_BITWISE dest, src0, src1 */
    VIR_OPINFO(OR_BITWISE, 2, HasDest|Componentwise|Expr|EPFromHighest, 1, AL),
    /* bitwise xor: XOR_BITWISE dest, src0, src1 */
    VIR_OPINFO(XOR_BITWISE, 2, HasDest|Componentwise|Expr|EPFromHighest, 1, AL),
    /* bitwise not: NOT_BITWISE dest, src0 */
    VIR_OPINFO(NOT_BITWISE, 1, HasDest|Componentwise|Expr|EPFromHighest, 1, AL),
    /* left shift: LSHIFT dest, src0, src1 */
    VIR_OPINFO(LSHIFT, 2, HasDest|Componentwise|Expr|EPFromS0, 1, AL),
    /* right shift: RSHIFT dest, src0, src1  */
    VIR_OPINFO(RSHIFT, 2, HasDest|Componentwise|Expr|EPFromS0, 1, AL),
    /* rotate bits: ROTATE dest, src0, src1 */
    VIR_OPINFO(ROTATE, 2, HasDest|Componentwise|Expr|EPFromS0, 1, AL),

    /**
     ** Logical operations.
     **/
    VIR_OPINFO(LOGICAL_NOT, 1, HasDest|Componentwise|Expr|EPFromHighest, 1, NM),

    /* compute the determinant of a mat2/mat3 */
    VIR_OPINFO(DETERMINANT_2, 2, HasDest|Expr|EPFromHighest, 1, HL),
    VIR_OPINFO(DETERMINANT_3, 3, HasDest|Expr|EPFromHighest, 1, HL),

    /**
     ** ternary operations
     **/
    /* multiply and add: dest = src0*src1 + src2
            MAD dest, src0, src1, src2 */
    VIR_OPINFO(MAD, 3, HasDest|Componentwise|Expr|EPFromHighest, 1, AL),

    /* integer satuarated multiply and add: dest = src0*src1 + src2
            MADSAT dest, src0, src1, src2 */
    VIR_OPINFO(MADSAT, 3, HasDest|Componentwise|Expr|EPFromHighest, 1, AL),

    /* Fused multiply and add: dest = src0*src1 + src2
            FMA dest, src0, src1, src2 */
    VIR_OPINFO(FMA, 3, HasDest|Componentwise|Expr|EPFromHighest, 1, AL),

    VIR_OPINFO(IMADLO0, 3, HasDest|Componentwise|Expr|EPFromHighest, 1, MC),
    VIR_OPINFO(IMADLO1, 3, HasDest|Componentwise|Expr|EPFromHighest, 1, MC),

    VIR_OPINFO(IMADHI0, 3, HasDest|Componentwise|Expr|EPFromHighest, 1, MC),
    VIR_OPINFO(IMADHI1, 3, HasDest|Componentwise|Expr|EPFromHighest, 1, MC),

    /* @ result = cond_op(src0) ? src1 : src2;
         cond_op := [Z, NZ, GEZ, GZ, LEZ, LZ, FINITE, INFIITE, ANYMSB, ALLMSB]
            CSELECT dest, src0, src1, src2 */
    VIR_OPINFO(CSELECT, 3, HasDest|Componentwise|Expr|EPFromS12|UseCondCode, 1, NM),
    /* Machine instruction: select.cond dst, src0, src1, src2
           dst = cond(src0, src1) ? src1 : src2
     */
    VIR_OPINFO(SELECT, 3, HasDest|Componentwise|Expr|EPFromS12|UseCondCode, 1, MC),
    /* sum of absolute differences: dst = (s0 - s1) + s2;
            SAD dest, src0, src1, src2 */
    VIR_OPINFO(SAD, 3, HasDest|Componentwise|Expr|EPFromHighest, 1, NU),
    /* linear blend: dst = s0 + (s1 - s0) * s2;
            MIX dest, src0, src1, src2 */
    VIR_OPINFO(MIX, 3, HasDest|Componentwise|Expr|EPFromHighest, 1, AL),
    /* partial Direct3D LIT
        dest.x = 1
        dest.y = (src0.y > 0)? src1.y : 0
        dest.z = ((src0.z > 0) && (src1.z > 0))? pow(2, src2.x) : 0
        dest.w = 1
           LITP dest, src0, src1, src2 */
    VIR_OPINFO(LITP, 3, HasDest|Expr|EPFromS12, 1, AL),
    /* Smooth step, Returns 0.0 if x <= edge0 and 1.0 if x >= edge1 and
       performs smooth Hermite interpolation between 0 and 1
       when edge0 < x < edge1. This is useful in cases where
       you would want a threshold function with a smooth
       transition. Results are undefined if edge0 >= edge1.
       src0 <- edge0, src1 <- edge1, src2 <- x
           SMOOTH dest, src0, src1, src2 */
    VIR_OPINFO(SMOOTH, 3, HasDest|Componentwise|Expr|EPFromHighest, 1, NU),
    /* half of mix: dst = src0 * (1.0 - src1)
            halfmix dest, src0, src1, */
    VIR_OPINFO(HALFMIX, 3, HasDest|Componentwise|Expr|EPFromHighest, 1, AL),

    /**
     ** load store
     **/
    /* memory load, with post-opreation: dest = addr[offset]
       src0 <- addr, src1 <- offset
           LOAD dest, src0, src1 */
    VIR_OPINFO(LOAD, 2, HasDest|Loads|Expr, 1, AL),
    /* memory store, with pre-operation: addr[offset] = val
       src0 <- addr, src1 <- offset, val <- src2, dest used as buffer for USC constrain
            STORE  dest, src0, src1, src2 */
    VIR_OPINFO(STORE, 3, HasDest|Stores|Src2Componentwise|EPFromS2, 1, AL),
    /* load address, dest = addr + offset
            LDA dest, src0, src1 */
    VIR_OPINFO(LDA, 2, HasDest|Expr, 1, NU),
    /* indirect load: dest = *(ptr+offset)
       src0 <- ptr, src1 <- offset
           ILOAD dest, src0, src1 */
    VIR_OPINFO(ILOAD, 2, HasDest|Loads|Expr, 1, NU),
    /* indirect store: *(ptr+offset) = val
       src0 <- ptr, src1 <- offset, val <- src2
            STORE  src0, src1, src2 */
    VIR_OPINFO(ISTORE, 3, HasDest|Stores, 0, NU),

    /**
     ** special load/store/atomic_add, which are used in register spill
        these instructions should "NOT" skip helper pixel
        these instruction has the same define as load/store/atomic_add
     **/
    VIR_OPINFO(LOAD_S, 2, HasDest|Loads|Expr, 1, AL),
    VIR_OPINFO(STORE_S, 3, HasDest|Stores, 1, AL),
    VIR_OPINFO(ATOMADD_S, 3, HasDest|Loads|Stores|Expr, 1, AL),

    /* local memory load/store and atomic operations */
    VIR_OPINFO(LOAD_L, 2, HasDest|Loads|Expr, 1, AL),
    VIR_OPINFO(STORE_L, 3, HasDest|Stores|Src2Componentwise|EPFromS2, 1, AL),

    VIR_OPINFO(ATOMADD_L, 3, HasDest | Loads | Stores| Expr, 1, AL),
    VIR_OPINFO(ATOMSUB_L, 3, HasDest | Loads | Stores| Expr, 1, AL),
    VIR_OPINFO(ATOMXCHG_L, 3, HasDest | Loads | Stores| Expr, 1, AL),
    /* For ATOMCMPXCHG, the Value is saved in the x channel of src2, the Comparator is saved in the y channel of src2. */
    VIR_OPINFO(ATOMCMPXCHG_L, 3, HasDest | Loads | Stores| Expr, 1, AL),
    VIR_OPINFO(ATOMMIN_L, 3, HasDest | Loads | Stores| Expr, 1, AL),
    VIR_OPINFO(ATOMMAX_L, 3, HasDest | Loads | Stores| Expr, 1, AL),
    VIR_OPINFO(ATOMOR_L, 3, HasDest | Loads | Stores| Expr, 1, AL),
    VIR_OPINFO(ATOMAND_L, 3, HasDest | Loads | Stores| Expr, 1, AL),
    VIR_OPINFO(ATOMXOR_L, 3, HasDest | Loads | Stores| Expr, 1, AL),

    VIR_OPINFO(IMG_SAMPLER, 2, HasDest|Loads|Expr, 1, HL),

    /* IMG_LOAD instructions.
    ** The source0 holds the vector 4 image descriptor:
    **      x: base address
    **      y: stride in bytes (should be set to 0 for 1D images).
    **      z: [height(31:16), width(15:0)]
    **      w: image defines (see GCREG_SH_IMAGE offset).
    ** The source1 holds the x/y coordinates (integer).
    ** The source2 holds the relative offset (immediate value) for IMG_LOAD
    **      src2.x[ 4: 0] S05 relative x offset
    **      src2.x[ 9: 5] S05 relative y offset
    ** The source3 is image sampler if presents (not undef)
    */
    VIR_OPINFO(IMG_LOAD, 4, HasDest|Expr|EPFromS0, 1, AL),

    /* IMG_LOAD_3D instructions.
    ** The source0 holds the vector 4 image descriptor:
    **      x: base address (not used in 3d instruction)
    **      y: stride in bytes (should be set to 0 for 1D images).
    **      z: [height(31:16), width(15:0)]
    **      w: image defines (see GCREG_SH_IMAGE offset).
    ** The source1.xy holds the x/y coordinates (integer).
    **     source1.z holds calculated base address of the addressing plane
    **        z = src0.x + 3d_coord.z * slice
    ** The source2 holds the relative offset (immediate value) for IMG_LOAD_3D
    **      src2.x[ 4: 0] S05 relative x offset
    **      src2.x[ 9: 5] S05 relative y offset
    **      src2.x[14:10] S05 relative z offset
    ** The source3 is image sampler if presents (not undef)
    */
    VIR_OPINFO(IMG_LOAD_3D, 4, HasDest|Loads|Expr|EPFromS0, 1, AL),

    /* IMG_STORE instructions.
    ** The source0 holds the vector 4 image descriptor (same as IMG_LOAD):
    ** The source1.xy holds the x/y coordinates (integer).
    **     source1.z holds calculated base address of the addressing plane
    **        z = src0.x + 3d_coord.z * slice
    ** The source2 holds the color value
    */
    VIR_OPINFO(IMG_STORE, 3, HasDest|Stores|EPFromS0, 1, AL),

    /* IMG_STORE_3D instructions.
    ** The source0 holds the vector 4 image descriptor (same as IMG_LOAD):
    ** The source1.xy holds the x/y coordinates (integer).
    **     source1.z holds calculated base address of the addressing plane
    **        z = src0.x + 3d_coord.z * slice
    ** The source2 holds the color value
    */
    VIR_OPINFO(IMG_STORE_3D, 3, HasDest|Stores|EPFromS0, 1, AL),

    /*
    ** IMG_ADDR instructions:
    ** Get the image address.
    ** The source0 holds the vector 4 image descriptor (same as IMG_LOAD).
    ** The source1.xy holds the x/y coordinates (integer).
    ** The source2.x holds the relativeOffset if needed (integer).
    */
    VIR_OPINFO(IMG_ADDR, 3, HasDest|Expr, 1, AL),
    VIR_OPINFO(IMG_ADDR_3D, 3, HasDest|Expr, 1, AL),

    /* IMG_QUERY instructions:
    ** The source0 holds the vector 4 image descriptor.
    ** The source1 holds the query type(It is a imm), the data type is UINT32.
    ** The source2 holds the extra data(e.g.LOD/COORD).
    */
    VIR_OPINFO(IMG_QUERY, 3, HasDest|Expr|EPFromHighest, 1, NM),

    /* src0: value, src1: max, src2: delta
       DST = MIN(MAX(SRC0 + delta(SRC2), 0), SRC1) */
    VIR_OPINFO(CLAMP0MAX, 3, HasDest|Expr, 1, AL),

    /* register array access */
    /* indexed array load:  LDARR dest, src0, src1 ==> dest = src0[src1] */
    VIR_OPINFO(LDARR, 2, HasDest|Expr|EPFromS0, 1, LM),
    /* indexed array store: STARR dest, src0, src1 ==> dest[src0] = src1 */
    VIR_OPINFO(STARR, 2, HasDest, 1, LM),

    /**
     ** texture operation
     **/
    /* fragment discard if the condition is true */
    VIR_OPINFO(KILL, 2, NoDest, 0, AL),
    /* texture lookup with optional bias, lod, gradient  */
    VIR_OPINFO(TEXLD, 4, HasDest|Loads|Expr|EPFromS0, 1, AL),
    /* texture lookup with optional bias, lod, gradient  */
    VIR_OPINFO(TEXLD_U, 4, HasDest|Loads|Expr|EPFromS0, 1, AL),
    VIR_OPINFO(TEXLD_U_F_L, 4, HasDest|Loads|Expr|EPFromS0, 1, LM),/* Extended from 0x4B */
    VIR_OPINFO(TEXLD_U_F_B, 4, HasDest|Loads|Expr|EPFromS0, 1, LM),/* Extended from 0x7B */
    VIR_OPINFO(TEXLD_U_S_L, 4, HasDest|Loads|Expr|EPFromS0, 1, LM),/* Extended from 0x49 */
    VIR_OPINFO(TEXLD_U_U_L, 4, HasDest|Loads|Expr|EPFromS0, 1, LM),/* Extended from 0x4A */
    /* projection texture lookup with optional bias, lod, gradient  */
    VIR_OPINFO(TEXLDPROJ, 4, HasDest|Loads|Expr|EPFromS0, 1, AL),
    /* percentage close filtering texture lookup with optional bias, lod, gradient */
    VIR_OPINFO(TEXLDPCF, 4, HasDest|Loads|Expr|EPFromS0, 1, AL),
    /* projection percentage close filtering texture lookup with optional bias, lod, gradient   */
    VIR_OPINFO(TEXLDPCFPROJ, 4, HasDest|Loads|Expr|EPFromS0, 1, AL),

    VIR_OPINFO(TEXLD_BIAS, 4, HasDest|Loads|Expr|EPFromS0, 1, LM),/* Extended from 0x18 */
    VIR_OPINFO(TEXLD_BIAS_PCF, 4, HasDest|Loads|Expr|EPFromS0, 1, LM),/* Extended from 0x18 */
    VIR_OPINFO(TEXLD_PLAIN, 4, HasDest|Loads|Expr|EPFromS0, 1, LM),/* Extended from 0x18 */
    VIR_OPINFO(TEXLD_PCF, 4, HasDest|Loads|Expr|EPFromS0, 1, LM),/* Extended from 0x18 */
    VIR_OPINFO(TEXLDB, 4, HasDest|Loads|Expr|EPFromS0, 1, LM),
    VIR_OPINFO(TEXLDD, 4, HasDest|Loads|Expr|EPFromS0, 1, LM),
    VIR_OPINFO(TEXLD_G, 4, HasDest|Loads|Expr|EPFromS0, 1, LM),
    VIR_OPINFO(TEXLDL, 4, HasDest|Loads|Expr|EPFromS0, 1, LM),
    VIR_OPINFO(TEXLDP, 4, HasDest|Loads|Expr|EPFromS0, 1, LM),
    VIR_OPINFO(TEXLD_LOD, 4, HasDest|Loads|Expr|EPFromS0, 1, LM),/* Extended from 0x6F */
    VIR_OPINFO(TEXLD_LOD_PCF, 4, HasDest|Loads|Expr|EPFromS0, 1, LM),/* Extended from 0x6F */
    VIR_OPINFO(TEXLD_G_PCF, 4, HasDest|Loads|Expr|EPFromS0, 1, LM),
    VIR_OPINFO(TEXLD_U_PLAIN, 3, HasDest|Loads|Expr|EPFromS0, 1, LM),/* Extended from 0x7B */
    VIR_OPINFO(TEXLD_U_LOD, 4, HasDest|Loads|Expr|EPFromS0, 1, LM),/* Extended from 0x7B */
    VIR_OPINFO(TEXLD_U_BIAS, 4, HasDest|Loads|Expr|EPFromS0, 1, LM),/* Extended from 0x7B */
    VIR_OPINFO(TEXLD_GATHER, 4, HasDest|Loads|Expr|EPFromS0, 1, LM),/* Extended from 0x7D */
    VIR_OPINFO(TEXLD_GATHER_PCF, 4, HasDest|Loads|Expr|EPFromS0, 1, LM),/* Extended from 0x7D */

    /* Use integer texture coordinate P to lookup a single texel from sampler.
       The array layer comes from the last component of P for the array forms.*/
    VIR_OPINFO(TEXLD_FETCH_MS, 4, HasDest|Loads|Expr|EPFromS0, 1, LM),

    /* fragment shader only, get current quad lod,
       same group as dsx/dsy, need HW support */
    VIR_OPINFO(GETLOD, 2, HasDest|Expr, 1, AL),
    /* texture attributes query:
                width, height, depth,
                channel_data_type,
                channel_order,
                normalized_coord
       Sampler attributes:
                force_unnorlmalized_coords,
                filter_mode,
                address_mode

        TEXQUERY dest, sampler, attribute
     */
    VIR_OPINFO(TEXQUERY, 2, HasDest|Expr, 1, NU),

    /* LODQ instructions:
    ** The source0 holds the sampler.
    ** The source1 holds the coord.
    ** The source2 holds the BIAS.
    */
    VIR_OPINFO(LODQ, 3, HasDest|Expr, 1, LM),
    VIR_OPINFO(LODQ_G, 2, HasDest|Expr, 1, LM),

    /* GET_SAMPLER_IDX:
    *  Get the sampler physical index for a texture.
    ** The source0 holds the texture.
    ** The source1 holds the offset.
    */
    VIR_OPINFO(GET_SAMPLER_IDX, 2, HasDest|Expr|EPFromS0, 1, AL),
    VIR_OPINFO(GET_SAMPLER_LMM, 2, HasDest|Expr, 1, AL),
    VIR_OPINFO(GET_SAMPLER_LBS, 2, HasDest|Expr, 1, AL),
    /* Get the levels and samples of a sampler. */
    VIR_OPINFO(GET_SAMPLER_LS, 2, HasDest|Expr, 1, AL),

    /* combining separate samplers and textures:
     *    uniform sampler s[4];    // a handle to filtering information
     *    uniform texture2D t[4];  // a handle to a texture (an image in SPIR-V)
     *    sampler2D ts1 = sampler2D(t[i], s[j]);
     *
     *   COMBINE_TEX_SAMPL sampler2D ts1, texture2D t[i], sampler s[j]
     */
    VIR_OPINFO(COMBINE_TEX_SAMPL, 2, HasDest, 1, AL),

    /**
     ** surface instructions
     **/
    /* surface load */
    VIR_OPINFO(SURLD, 2, HasDest|Expr, 1, NU),
    /* surface store */
    VIR_OPINFO(SURSTORE, 2, HasDest, 0, NU),
    /* surface reduction */
    VIR_OPINFO(SURRED, 2, HasDest, 1, NU),
    /* surface query */
    VIR_OPINFO(SURQUERY, 2, HasDest|Expr, 1, NU),

    /* synchronization and atomic operations */
    VIR_OPINFO(BARRIER, 1, NoDest, 0, AL), /*   */
    VIR_OPINFO(MEM_BARRIER, 1, NoDest, 0, AL), /*   */
    VIR_OPINFO(FENCE, 0, NoDest, 0, NU), /* memory fence */
    VIR_OPINFO(FLUSH, 1, NoDest, 0, AL), /* flush cache:
                                                   *   bit0: Flush L1 cache
                                                   *   bit1: Invalidate L1 cache
                                                   *   bit2: Invalidate texture cache.*/

    /* atomic operations */
    VIR_OPINFO(ATOMADD, 3, HasDest|Loads|Stores|Expr, 1, AL), /* atomic add */
    VIR_OPINFO(ATOMSUB, 3, HasDest|Loads|Stores|Expr, 1, AL), /* atomic sub */
    VIR_OPINFO(ATOMXCHG, 3, HasDest|Loads|Stores|Expr, 1, AL), /* atomic exchange */
    /* For ATOMCMPXCHG, the Value is saved in the x channel of src2, the Comparator is saved in the y channel of src2. */
    VIR_OPINFO(ATOMCMPXCHG, 3, HasDest|Loads|Stores|Expr, 1, AL), /* atmoic compare and exchange */
    VIR_OPINFO(ATOMMIN, 3, HasDest|Loads|Stores|Expr, 1, AL), /* atmoic min */
    VIR_OPINFO(ATOMMAX, 3, HasDest|Loads|Stores|Expr, 1, AL), /* atomic max */
    VIR_OPINFO(ATOMOR, 3, HasDest|Loads|Stores|Expr, 1, AL), /* atmoic or */
    VIR_OPINFO(ATOMAND, 3, HasDest|Loads|Stores|Expr, 1, AL), /* atmoic and */
    VIR_OPINFO(ATOMXOR, 3, HasDest|Loads|Stores|Expr, 1, AL), /* atomic xor */

    /* extended precision arithmetic instrucitons */
    VIR_OPINFO(ADDC, 2, HasDest|Componentwise|Expr, 1, NU), /* add with carrier bit */
    VIR_OPINFO(ADD_CC, 2, HasDest|Componentwise|Expr, 1, NU), /* add and set carrier bit */
    VIR_OPINFO(SUBC, 2, HasDest|Componentwise|Expr, 1, NU), /* substract with borrow bit */
    VIR_OPINFO(SUB_CC, 2, HasDest|Componentwise|Expr, 1, NU), /* substruct and set borrow bit */
    VIR_OPINFO(MADC, 2, HasDest|Componentwise|Expr, 1, NU), /* multiply and add with carrier bit */
    VIR_OPINFO(MAD_CC, 2, HasDest|Componentwise|Expr, 1, NU), /* multiply and add and set carrier bit */

    /* bit field operations */
    VIR_OPINFO(BITFIND_MSB, 1, HasDest|Componentwise|Expr|EPFromS0, 1, AL), /* OGL. Find the most significant non-sign bit for the integer */
    VIR_OPINFO(BITFIND_LSB, 1, HasDest|Componentwise|Expr|EPFromS0, 1, AL), /* OGL. Find the least significant non-sign bit for the integer */
    VIR_OPINFO(CTZ, 1, HasDest|Componentwise|Expr|EPFromS0, 1, AL), /* OCL. Returns the count of trailing 0-bits in x. If x is 0,
                                                                                  returns the size in bits of the type of x or
                                                                                  component type of x, if x is a vector. */
    VIR_OPINFO(BITSEL, 2, HasDest|Componentwise|Expr|EPFromS0, 1, AL), /* bit select */
    VIR_OPINFO(BITREV, 1, HasDest|Componentwise|Expr|EPFromS0, 1, AL), /* bit reverse of an integer */
    VIR_OPINFO(BITINSERT, 4, HasDest|Src01Componentwise|Expr|EPFromS0, 1, NM), /* bit insert, 4 operands, src2: offset, src3: bits) */
    VIR_OPINFO(BITINSERT1, 3, HasDest|Src01Componentwise|Expr|EPFromS0, 1, LM), /* bit insert, 3 operands, src2.x encoded ((bits << 8) | (offset & 0xFF)) */
    VIR_OPINFO(BITINSERT2, 3, HasDest|Src01Componentwise|Expr|EPFromS0, 1, LM), /* bit insert1, 3 operands, src2.x is offset, src2.y is bits */
    VIR_OPINFO(LEADZERO, 1, HasDest|Componentwise|Expr|EPFromS0, 1, AL), /* lead zero bits, clz()  */
    VIR_OPINFO(POPCOUNT, 1, HasDest|Componentwise|Expr|EPFromS0, 1, AL), /* bit population count */
    VIR_OPINFO(BYTEREV, 1, HasDest|Componentwise|Expr|EPFromS0, 1, AL), /* byte reverse of an integer */
    VIR_OPINFO(BITEXTRACT, 3, HasDest|Componentwise|Expr|EPFromS0, 1, AL), /* bit extract, 3 operands */
    VIR_OPINFO(BITRANGE, 2, HasDest|Componentwise|Expr|EPFromS0, 1, AL), /* bit range, 2 operands to pair with bit extract */
    VIR_OPINFO(BITRANGE1, 1, HasDest|Componentwise|Expr|EPFromS0, 1, AL), /* bit range, 2 operands to pair with bit extract */
    VIR_OPINFO(UCARRY, 2, HasDest|Componentwise|Expr|EPFromS0, 1, AL), /* this is for gcSL_UCARRY */

    /* VX EVIS instructions:
     *
     *   i E [StartBin,EndBin],
     *   j E [SourceBin, SourceBin + EndBin ? StartBin]
     */

    /* implicit cast for vx_inst parameter */
    VIR_OPINFO(VX_ICASTP, 1, HasDest|EPFromS0|Expr|VX1, 1, NM),
    /* implicit cast for vx_inst dest */
    VIR_OPINFO(VX_ICASTD, 1, HasDest|EPFromS0|Expr|VX1, 1, NM),

    /* VX_IMG_LOAD instructions.
    ** The source0 holds the vector 4 image descriptor:
    **      x: base address
    **      y: stride in bytes (should be set to 0 for 1D images).
    **      z: [height(31:16), width(15:0)]
    **      w: image defines (see GCREG_SH_IMAGE offset).
    ** The source1 holds the x/y coordinates (integer).
    ** The source2 holds the relative offset (immediate value) for IMG_LOAD
    **      src2.x[ 4: 0] S05 relative x offset
    **      src2.x[ 9: 5] S05 relative y offset
    ** The source3 holds EVIS_modifer
    */
    VIR_OPINFO(VX_IMG_LOAD, 4, HasDest|EPFromS0|Expr|VX1_2|EVISModifier(3), 1, AL),

    /* VX_IMG_LOAD_3D instructions.
    ** The source0 holds the vector 4 image descriptor:
    **      x: base address (not used in 3d instruction)
    **      y: stride in bytes (should be set to 0 for 1D images).
    **      z: [height(31:16), width(15:0)]
    **      w: image defines (see GCREG_SH_IMAGE offset).
    ** The source1.xy holds the x/y coordinates (integer).
    **     source1.z holds calculated base address of the addressing plane
    **        z = src0.x + 3d_coord.z * slice
    ** The source2 holds the relative offset (immediate value) for IMG_LOAD_3D
    **      src2.x[ 4: 0] S05 relative x offset
    **      src2.x[ 9: 5] S05 relative y offset
    **      src2.x[14:10] S05 relative z offset
    ** The source3 holds EVIS_modifer
    */
    VIR_OPINFO(VX_IMG_LOAD_3D, 4, HasDest|EPFromS0|Expr|VX1_2|EVISModifier(3), 1, AL),

    /* IMG_STORE instructions.
    ** The source0 holds the vector 4 image descriptor (same as IMG_LOAD):
    ** The source1.xy holds the x/y coordinates (integer).
    **     source1.z holds calculated base address of the addressing plane
    **        z = src0.x + 3d_coord.z * slice
    ** The source2 holds the color value
    ** The source3 holds EVIS_modifer
    */
    VIR_OPINFO(VX_IMG_STORE, 4, HasDest|Stores|EPFromS0|VX1_2|EVISModifier(3), 1, AL),

    /* VX_IMG_STORE_3D instructions.
    ** The source0 holds the vector 4 image descriptor (same as IMG_LOAD):
    ** The source1.xy holds the x/y coordinates (integer).
    **     source1.z holds calculated base address of the addressing plane
    **        z = src0.x + 3d_coord.z * slice
    ** The source2 holds the color value
    ** The source3 holds EVIS_modifer
    */
    VIR_OPINFO(VX_IMG_STORE_3D, 4, HasDest|Stores|EPFromS0|VX1_2|EVISModifier(3), 1, AL),

    /* The AbsDiff instruction computes the absolute difference between two values.
     * It works on packed data, so it can compute 16x 8-bit values or 8x 16-bit values.
     *      dest[i] = |src0[j] - src1[j] |
     */
    VIR_OPINFO(VX_ABSDIFF, 3, HasDest|Componentwise|Expr|VX1_2|EVISModifier(2), 1, AL),

    /* The IAdd instruction adds two or three integer values. It works on packed data,
     * so it can compute 16x 8-bit values or 8x 16-bit values. Valid instruction formats
     * are U8, S8, U16, and S16.
     *  dest[i] =  src0[j] + src1[j], if src2 is not valid;
     *             src0[j] + src1[j] + src2[j], if src2 is valid.
     */
    VIR_OPINFO(VX_IADD, 4, HasDest|Componentwise|Expr|VX1|EVISModifier(3), 1, AL),

    /* The IAccSq instruction squares a value and adds it to an accumulator. It works
     * on 8- and 16-bit packed data, so it can compute 16x 8-bit values or 8x 16-bit
     * values. Valid instruction formats are U8, S8, U16, and S16.
     *  dest[i] = src0[i] + (src1[j]^2 >> src2).
     */
    VIR_OPINFO(VX_IACCSQ, 4, HasDest|Componentwise|Expr|VX1_2|EVISModifier(3), 1, AL),

    /* The Lerp instruction does a linear interpolation between two values. It works
     * on 8- and 16-bit packed data, so it can compute 16x 8-bit values or 8x 16-bit
     * values. Valid instruction formats are U8, S8, U16, and S16.
     *  dest[i] = (1 - src2) * src0[j] + src2 * src1[j]
     */
    VIR_OPINFO(VX_LERP, 4, HasDest|Componentwise|Expr|VX1_2|EVISModifier(3), 1, AL),

    /* The Filter instruction performs a specific filter on a 3x3 pixel block. It works
     * on 8- and 16-bit packed data, so it can compute 16x 8-bit values or 8x 16-bit
     * values. Valid instruction formats are U8, S8, U16, and S16.
     *
     * Note that the box filter only can produce up to 8 values per cycles.
     */
    VIR_OPINFO(VX_FILTER, 4, HasDest|Componentwise|Expr|VX1|EVISModifier(3), 1, AL),

    /* The MagPhase instruction computes the magnitude and phase of two incoming
     * values. It works on 8- and 16-bit packed data, but it can only 4x 8-bit values
     * or 4x 16-bit values. Valid instruction formats are U8, S8, U16, S16, and FP16.
     * The destination value always gets clamped (signed or unsigned, depending
     * on the IType field).
     */
    VIR_OPINFO(VX_MAGPHASE, 3, HasDest|Componentwise|Expr|VX1|EVISModifier(2), 1, AL),

    /* The MulShift instruction multiplies two 8- or 16-bit integer values together and
     * shifts the result by a specified number of bits. It works on 8- and 16-bit packed
     * input data. It can produce 8x 8-bit or 16-bit values, or 4x 32-bit values. Valid
     * instruction formats are U8, S8, U16, S16, U32, S32, FP16, and FP32.
     * The destination value either gets wrapped (just the lower bits copied) or
     * clamped (signed or unsigned, depending on the IType field), depending on the
     * Clamp field inside the shader instruction.
     *
     *  dest[i] = (src0[j] * src1[j]) >> src2
     */
    VIR_OPINFO(VX_MULSHIFT, 4, HasDest|Expr|VX1_2|EVISModifier(3), 1, AL),

    /* The DP16x1 instruction performs a dot-product of two 16-component values. It
     * will produce only one output value. Valid instruction formats are U8, S8, U16,
     * S16, U32, S32, FP16, and FP32. This instruction doesn’t use the SourceBin field.
     * The destination value either gets wrapped (just the lower bits copied) or
     * clamped (signed or unsigned, depending on the IType field), depending on the
     * Clamp field inside the shader instruction.
     * Note that src0 and src1 get expanded or sign-extended if its format is smaller
     * than the instruction format; src2 is a 512-bit uniform value.
     */
    VIR_OPINFO(VX_DP16X1, 4, HasDest|Expr|VXUse512BitUniform(3)|EVISModifier(2), 1, AL),

    /* The DP8x2 instruction performs two dot-product of two 8-component values.
     * It will produce up to two output values. Valid instruction formats are U8, S8,
     * U16, S16, U32, S32, FP16, and FP32.
     */
    VIR_OPINFO(VX_DP8X2, 4, HasDest|Expr|VXUse512BitUniform(3)|EVISModifier(2), 1, AL),

    /* The DP4x4 instruction performs four dot-product of two 4-component values.
     * It will produce up to four output values. Valid instruction formats are U8, S8,
     * U16, S16, U32, S32, FP16, and FP32.
     */
    VIR_OPINFO(VX_DP4X4, 4, HasDest|Expr|VXUse512BitUniform(3)|EVISModifier(2), 1, AL),

    /* The DP2x8 instruction performs eight dot-product of two 2-component values.
     * It will produce up to eight output values. Valid instruction formats are U8, S8,
     * U16, S16, and FP16.*/
    VIR_OPINFO(VX_DP2X8, 4, HasDest|Expr|VXUse512BitUniform(3)|EVISModifier(2), 1, AL),

    /* The DP16x1 instruction performs two dot-product of two 16-component values.
     * It will produce up only one output values. Valid instruction formats are U8, S8,
     * U16, S16, U32, S32, FP16, and FP32. The _B version of the DP16 family instrucitons
     * take temp256 as its first two source, src0 is the tmp256.hi, src1 is the tmp256.lo
     */
    VIR_OPINFO(VX_DP16X1_B, 5, HasDest|Expr|VXUse512BitUniform(4)|EVISModifier(3), 1, AL),

    /* The DP8x2 instruction performs two dot-product of two 8-component values.
     * It will produce up to two output values. Valid instruction formats are U8, S8,
     * U16, S16, U32, S32, FP16, and FP32. It take3 temp256 as its first two source,
     * src0 is the tmp256.hi, src1 is the tmp256.lo
     */
    VIR_OPINFO(VX_DP8X2_B, 5, HasDest|Expr|VXUse512BitUniform(4)|EVISModifier(3), 1, AL),

    /* The DP4x4 instruction performs four dot-product of two 4-component values.
     * It will produce up to four output values. Valid instruction formats are U8, S8,
     * U16, S16, U32, S32, FP16, and FP32. It take3 temp256 as its first two source,
     * src0 is the tmp256.hi, src1 is the tmp256.lo
     */
    VIR_OPINFO(VX_DP4X4_B, 5, HasDest|Expr|VXUse512BitUniform(4)|EVISModifier(3), 1, AL),

    /* The DP2x8 instruction performs eight dot-product of two 2-component values.
     * It will produce up to eight output values. Valid instruction formats are U8, S8,
     * U16, S16, and FP16. It take3 temp256 as its first two source, src0 is the tmp256.hi,
     * src1 is the tmp256.lo
     */
    VIR_OPINFO(VX_DP2X8_B, 5, HasDest|Expr|VXUse512BitUniform(4)|EVISModifier(3), 1, AL),

    /* The DP32x1 instruction performs a dot-product of two 32-component values. It
     * will produce only one output value. Valid instruction formats are U8, S8, U16,
     * S16, U32, S32, FP16, and FP32.
     */
    VIR_OPINFO(VX_DP32X1, 4, HasDest|Expr|VXUse512BitUniform(3)|EVISModifier(2), 1, AL),

    /* The DP16x2 instruction performs a dot-product of two 16-component values.
     * It will produce up to two output values. Valid instruction formats are U8, S8,
     * U16, S16, U32, S32, FP16, and FP32.
     */
    VIR_OPINFO(VX_DP16X2, 4, HasDest|Expr|VXUse512BitUniform(3)|EVISModifier(2), 1, AL),

    /* The DP8x4 instruction performs a dot-product of two 8-component values. It
     * will produce up to four output values. Valid instruction formats are U8, S8,
     * U16, S16, U32, S32, FP16, and FP32.
     */
    VIR_OPINFO(VX_DP8X4, 4, HasDest|Expr|VXUse512BitUniform(3)|EVISModifier(2), 1, AL),

    /* The DP4x8 instruction performs a dot-product of two 4-component values. It
     * will produce up to eight output values. Valid instruction formats are U8, S8,
     * U16, S16, and FP16.
     */
    VIR_OPINFO(VX_DP4X8, 4, HasDest|Expr|VXUse512BitUniform(3)|EVISModifier(2), 1, AL),

    /* The DP2x16 instruction performs a dot-product of two 2-component values. It
     * will produce up to sixteen output values. Valid instruction formats are U8, and
     * S8.*/
    VIR_OPINFO(VX_DP2X16, 4, HasDest|Expr|VX1Use512BitUniform(3)|EVISModifier(2), 1, AL),

    /* The DP32x1 instruction performs a dot-product of two 32-component values. It
     * will produce only one output value. Valid instruction formats are U8, S8, U16,
     * S16, U32, S32, FP16, and FP32. The _B version of the DP32 family instrucitons
     * take temp256 as its first two source, src0 is the tmp256.hi, src1 is the tmp256.lo
     */
    VIR_OPINFO(VX_DP32X1_B, 5, HasDest|Expr|VXUse512BitUniform(4)|EVISModifier(3), 1, AL),

    /* The DP16x2 instruction performs a dot-product of two 16-component values.
     * It will produce up to two output values. Valid instruction formats are U8, S8,
     * U16, S16, U32, S32, FP16, and FP32. It take3 temp256 as its first two source,
     * src0 is the tmp256.hi, src1 is the tmp256.lo
     */
    VIR_OPINFO(VX_DP16X2_B, 5, HasDest|Expr|VXUse512BitUniform(4)|EVISModifier(3), 1, AL),

    /* The DP8x4 instruction performs a dot-product of two 8-component values. It
     * will produce up to four output values. Valid instruction formats are U8, S8,
     * U16, S16, U32, S32, FP16, and FP32. It take3 temp256 as its first two source,
     * src0 is the tmp256.hi, src1 is the tmp256.lo
     */
    VIR_OPINFO(VX_DP8X4_B, 5, HasDest|Expr|VXUse512BitUniform(4)|EVISModifier(3), 1, AL),

    /* The DP4x8 instruction performs a dot-product of two 4-component values. It
     * will produce up to eight output values. Valid instruction formats are U8, S8,
     * U16, S16, and FP16. It take3 temp256 as its first two source, src0 is the tmp256.hi,
     * src1 is the tmp256.lo
     */
    VIR_OPINFO(VX_DP4X8_B, 5, HasDest|Expr|VXUse512BitUniform(4)|EVISModifier(3), 1, AL),

    /* The DP2x16 instruction performs a dot-product of two 2-component values. It
     * will produce up to sixteen output values. Valid instruction formats are U8, and
     * S8.  It take3 temp256 as its first two source,
     * src0 is the tmp256.hi, src1 is the tmp256.lo
     */
    VIR_OPINFO(VX_DP2X16_B, 5, HasDest|Expr|VX1Use512BitUniform(4)|EVISModifier(3), 1, AL),

    /* The Clamp instruction clamps up to 16 values to a min and.or max value. In
     * boolean mode it will write a 0 in the result if the value is inside the specified
     * min/max range, otherwise all 1’s will be written to the result. It works on 8-
     * and 16-bit packed data, so it can produce 16x 8-bit and 8x 16-bit values. Valid
     * instruction formats are U8, S8, U16, and S16.
     */
    VIR_OPINFO(VX_CLAMP, 4, HasDest|Componentwise|Expr|VX1_2|EVISModifier(3), 1, AL),

    /* The BiLinear instruction computes a bi-linear interpolation of 4 pixel values. It
     * works on 8- and 16-bit packed data, but it can only produce 8x 8-bit or 16-bit
     * values. Valid instruction formats are U8, S8, U16, and S16.
     */
    VIR_OPINFO(VX_BILINEAR, 4, HasDest|Expr|VX1|EVISModifier(3), 1, AL),

    /* The SelectAdd instruction either adds the pixel value or increments a counter
     * inside a number of distribution (histogram) bins. Based on the destination
     * format the SelectAdd instruction can modify up to 8 bins. If more bins are
     * required, multiple SelectAdd instructions can be used. It works on 8- and 16-
     * bit packed input data. Valid input formats are U8, S8, U16, and S16. Valid
     * output formats are U16, S16, U32, and S32. Note that the SelectAdd instruction
     * can only produce up to 8 outputs per cycle.
     * Src0 has the packed source data. Src1 contains the add values for each
     * bin. Src2 holds a 512-bit uniform that contains the min value for the entire
     * instruction as well as the range for each bin.
     */
    VIR_OPINFO(VX_SELECTADD, 4, HasDest|Componentwise|Expr|VX1Use512BitUniform(2)|EVISModifier(3), 1, AL),

    /* The AtomicAdd instruction adds a valid atomically to a given address. It is in
     * fact a read/modify/write instruction that executes atomically. It works on 8-
     * or 16-bit packed input data. Valid formats are U8, S8, U16, and S16. Valid
     * output formats are U8, S8, U16, S16, U32, and S32.
     * Src0 holds the base address and src1 holds the offset. Src2 holds the values
     * that need to be added to the memory locations pointed to by src0 and src1.
     */
    VIR_OPINFO(VX_ATOMICADD, 4, HasDest|Expr|VX1_2|EVISModifier(3), 1, AL),

    /* The BitExtract instruction extracts up to 8 bitfields from a packed data stream.
     * The input is is a 256-bit blob of data. Valid output formats are U8, U16, and
     * U32.
     */
    VIR_OPINFO(VX_BITEXTRACT, 4, HasDest|Componentwise|Expr|VX1_2|EVISModifier(3), 1, AL),

    /* The BitReplace instruction replaces up to 8 bitfields inside a packed data stream.
     * Valid output formats are U8, U16, and U32. This instruction doesn’t use the
     * SourceBin field.
     */
    VIR_OPINFO(VX_BITREPLACE, 4, HasDest|Componentwise|Expr|VX1|EVISModifier(3), 1, AL),

    /* VX2
     *  The IndexAdd instruction either adds the pixel value or increments a counter inside
     *  a number of distribution (histogram) bins.  Based on the destination format the
     *  IndexAdd instruction can modify up to 4 bins.  If more bins are required, multiple
     *  IndexAdd instructions can be used.  Note that the SelectAdd instruction can only
     *  produce up to 4 outputs per cycle.
     *
     *      IndexAdd Dest, Src0, Src1, Src2
     *
     *       Dest = histogram result
     *       Src0 = BinIndexes : used to select which bin to add to
     *       Src1 = BinIncs: determines how much to increment
     *       Src2.x = BinStart: (immediate value) specifies the starting bin index
     */
    VIR_OPINFO(VX_INDEXADD, 4, HasDest|Componentwise|Expr|VX2|EVISModifier(3), 1, AL),

    /* VX2
     *  These six instructions are added to specifically target Median3x3 filter operation.
     *  The instruction support U8, S8, U16, S16 and F16 formats.  VertMin3, VertMax3 and VertMedian3
     *  perform comparison operations among three sources on all 16 8-bit (or 8 16-bit)
     *  components in parallel.  HorzMin3, HorzMax3 and HorzMedian3 perform comparison among
     *  three adjacent components on all 14 8-bit (or 6 16-bit) component groups.
     */
    /* VertMin3 dest, src0, src1, src2
     *  The following equation defines the execution of the VertMin3 instruction.
     *    dest_(StartBin:StartBin+15) = min (src_(0, bin_0,15 ),
     *                                       src_(1, bin_0,15 ),
     *                                       src_(2, bin_0,15 )  )
     */
    VIR_OPINFO(VX_VERTMIN3, 4, HasDest|Componentwise|Expr|VX2|EVISModifier(3), 1, AL),
    /* VertMax3 dest, src0, src1, src2
     *  The following equation defines the execution of the VertMax3 instruction.
     *    dest_(StartBin:StartBin+15) = max (src_(0, bin_0,15 ),
     *                                       src_(1, bin_0,15 ),
     *                                       src_(2, bin_0,15 )  )
     */
    VIR_OPINFO(VX_VERTMAX3, 4, HasDest|Componentwise|Expr|VX2|EVISModifier(3), 1, AL),
    /* VertMedian3 dest, src0, src1, src2
     *  The following equation defines the execution of the VertMedian3 instruction.
     *    dest_(StartBin:StartBin+15) = median (src_(0, bin_0,15 ),
     *                                          src_(1, bin_0,15 ),
     *                                          src_(2, bin_0,15 )  )
     */
    VIR_OPINFO(VX_VERTMED3, 4, HasDest|Componentwise|Expr|VX2|EVISModifier(3), 1, AL),
    /* HorzMin3 dest, src0
     *  The following equation defines the execution of the HorzMin3 instruction.
     *    dest_(StartBin:StartBin+15) = min (src_(0, bin_0,13 ),
     *                                       src_(0, bin_1,14 ),
     *                                       src_(0, bin_2,15 )  )
     */
    VIR_OPINFO(VX_HORZMIN3, 2, HasDest|Expr|VX2|EVISModifier(1), 1, AL),
    /* HorzMax3 dest, src0
     *  The following equation defines the execution of the HorzMax3 instruction.
     *    dest_(StartBin:StartBin+15) = max (src_(0, bin_0,13 ),
     *                                       src_(0, bin_1,14 ),
     *                                       src_(0, bin_2,15 )  )
     */
    VIR_OPINFO(VX_HORZMAX3, 2, HasDest|Expr|VX2|EVISModifier(1), 1, AL),
    /* HorzMedian3 dest, src0
     *  The following equation defines the execution of the HorzMaedian3 instruction.
     *    dest_(StartBin:StartBin+15) = median (src_(0, bin_0,13 ),
     *                                          src_(0, bin_1,14 ),
     *                                         src_(0, bin_2,15 )  )
     */
    VIR_OPINFO(VX_HORZMED3, 2, HasDest|Expr|VX2|EVISModifier(1), 1, AL),

    /* float point number attributes */
    VIR_OPINFO(COPYSIGN, 1, HasDest|Componentwise|Expr|EPFromS0, 1, LM), /* Very Low Level */
    VIR_OPINFO(NAN, 1, HasDest|Componentwise|Expr|EPFromS0, 1, LM), /* Very Low Level */
    VIR_OPINFO(NEXTAFTER, 1, HasDest|Componentwise|Expr|EPFromS0, 1, LM), /* Very Low Level */
    VIR_OPINFO(ROUNDEVEN, 1, HasDest|Componentwise|Expr|EPFromS0, 1, AL), /* round to nearest even */
    VIR_OPINFO(ROUNDAWAY, 1, HasDest|Componentwise|Expr|EPFromS0, 1, AL), /* round nearest to x rounding halfway cases away from zero */
    VIR_OPINFO(TRUNC, 1, HasDest|Componentwise|Expr|EPFromS0, 1, AL), /* round nearest to x whose absolute value is not larger than the absolute value of x. */
    VIR_OPINFO(GETEXP, 1, HasDest|Componentwise|Expr|EPFromS0, 1, AL), /* get exponent part of the float point value  */
    VIR_OPINFO(GETMANT, 1, HasDest|Componentwise|Expr|EPFromS0, 1, AL), /* get mantissa part of the float point value  */
    VIR_OPINFO(SETEXP, 1, HasDest|Componentwise|Expr|EPFromS0, 1, NU), /* set exponent part of the float point value  */
    VIR_OPINFO(SETMANT, 1, HasDest|Componentwise|Expr|EPFromS0, 1, NU), /* set mantissa part of the float point value  */
    /* Build float from significand and exponent:
     *  Result = source_0 * (2 ^ source_1);
     */
    VIR_OPINFO(LDEXP, 2, HasDest|Componentwise|Expr|EPFromS0, 1, AL),

    /* control flow */
    VIR_OPINFO(JMP, 0, HasDest|ControlFlow, 0, AL), /* unconditional direct branch */
    VIR_OPINFO(JMPC, 2, HasDest|ControlFlow|UseCondCode, 0, AL), /* conditional direct branch */
    VIR_OPINFO(JMP_ANY, 2, HasDest|ControlFlow, 0, AL), /* conditional direct branch on any component is true */
    VIR_OPINFO(IJMP, 0, HasDest|ControlFlow, 0, NU), /* unconditional indirect branch, not supported in Dual16*/
    VIR_OPINFO(CALL, 0, HasDest|ControlFlow, 0, AL), /* function call, destination is FuncId */
    VIR_OPINFO(INTRINSIC, 2, HasDest|ControlFlow|EPHP, 0, HM), /* intrinsic call, src0 is funcId, src1 is parameters */
    VIR_OPINFO(ICALL, 0, HasDest|ControlFlow, 0, NU), /* indirect call */
    VIR_OPINFO(RET, 0, NoDest|ControlFlow, 0, AL), /* function return */
    VIR_OPINFO(PHI, 1, HasDest|ControlFlow, 1, AL), /* PHI node */
    VIR_OPINFO(SPV_PHI, 1, HasDest|ControlFlow, 1, HL), /* Spirv PHI node */
    VIR_OPINFO(EXIT, 0, NoDest|ControlFlow, 0, NU), /* program exit */
    VIR_OPINFO(THREADEXIT, 0, NoDest|ControlFlow, 0, NU), /* thread exit */
    VIR_OPINFO(GETPC, 0, HasDest|ControlFlow, 1, LM), /* get the next PC */

    VIR_OPINFO(LOOP, 1, NoDest|ControlFlow, 0, LM), /* LOOP:  */
    VIR_OPINFO(ENDLOOP, 0, NoDest|ControlFlow, 0, LM), /* end of loop */
    VIR_OPINFO(REP, 1, NoDest|ControlFlow, 0, LM), /* Repeat LOOP:  */
    VIR_OPINFO(ENDREP, 0, NoDest|ControlFlow, 0, LM), /* end of repeat loop */

    VIR_OPINFO(BLOCK, 0, NoDest, 0, HM), /* code block */
    VIR_OPINFO(ENTRY, 0, NoDest, 0, HM), /* entry point of function */
    VIR_OPINFO(LABEL, 0, HasDest, 0, AL), /* label in a function */

    /* debug */
    VIR_OPINFO(ASSERT, 1, NoDest, 0, HM), /* tells compiler the condition should hold true */
    VIR_OPINFO(TRAP, 0, NoDest, 0, NU), /* abort and interrupt the GPU */
    VIR_OPINFO(BREAK, 0, NoDest, 0, NU), /* break point, suspend the shader and wait for system to resume or terminate */
    VIR_OPINFO(COMMENT, 0, NoDest, 0, HM), /* informational comments */
    VIR_OPINFO(UNREACHABLE, 0, NoDest, 0, HM), /* the instruciton is unrechable */
    VIR_OPINFO(ERROR, 1, NoDest, 0, HM), /* error report  */

    /* Reversal instructions. */


    /* Tesselation Geometry Shader extension. */
    VIR_OPINFO(STORE_ATTR, 3, HasDest|OnlyUseEnable|Stores, 0, LM), /* STORE_ATTR  RemapAddr, AttributeIndex, Value */
    VIR_OPINFO(LOAD_ATTR, 3, HasDest|Loads|Expr, 1, LM), /* LOAD_ATTR  dest, Remap.xyzw, RemapIndex, AttributeIndex */
    VIR_OPINFO(LOAD_ATTR_O, 3, HasDest|Loads|Expr, 1, LM), /* LOAD_ATTR_O  dest, Remap.xyzw, RemapIndex, AttributeIndex
                                                                      Only for TCS to load attribute from output */
    VIR_OPINFO(ATTR_ST, 3, HasDest|Stores, 1, HM), /* ATTR_ST  Output, InvocationIndex, offset, value */
    VIR_OPINFO(ATTR_LD, 3, HasDest|Src0Componentwise|Loads|Expr, 1, HM), /* ATTR_LD  dest, Attribute, InvocationIndex, offset */

    VIR_OPINFO(SELECT_MAP, 4, HasDest, 1, LM), /* SELECT_MAP dest, RangeSourceComponent, src1, src2, samperSwizzle */
    VIR_OPINFO(EMIT0, 0, NoDest, 0, AL),
    VIR_OPINFO(RESTART0, 0, NoDest, 0, AL), /* cut */
    VIR_OPINFO(EMIT, 3,HasDest, 0, LM),
    VIR_OPINFO(RESTART, 2,HasDest, 0, LM), /* cut */

    VIR_OPINFO(ALLOCA, 1, HasDest, 1, NU), /* allocate memory in stack */

    /* memory address space */
    VIR_OPINFO(QAS, 1, HasDest, 1, NU), /* query address space: QAS.global pred1, reg1 */
    VIR_OPINFO(CVTA, 2, HasDest, 1, NU), /* convert between address space */

    /* performance */
    VIR_OPINFO(PREFETCH, 2, NoDest, 0, NU), /* prefetch memory */

    /* misc */
    VIR_OPINFO(PRAGMA, 1, HasDest, 0, NU), /* pragma directive to optimizer */
    VIR_OPINFO(RGB2YUV, 2, HasDest, 1, AL), /* Convert RGB into YUV */
    VIR_OPINFO(PARAM_CHAIN, 2, HasDest, 1, HL), /* chain two sources to one dest, used by gcSL to pass multiple sources to one inst */

    VIR_OPINFO(MAXOPCODE, 0, NoDest, 0, NU),

#undef HasDest
#undef NoDest
#undef Transcendental
#undef IntegerOnly
#undef ControlFlow
#undef UseCondCode
#undef Componentwise
#undef Componentwise
#undef Src0Componentwise
#undef Src1Componentwise
#undef Src2Componentwise
#undef Src3Componentwise
#undef OnlyUseEnable
#undef Loads
#undef Stores
#undef Expr
#undef EPFromHighest
#undef EPFromS0
#undef EPFromS12
#undef EPFromS2
#undef Use512BitUniform
#undef VXUse512BitUniform
#undef EVISModifier

#undef NU
#undef HL
#undef ML
#undef HM
#undef LL
#undef MC
#undef NM
#undef LM
#undef AL

