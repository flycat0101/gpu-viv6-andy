/****************************************************************************
*
*    Copyright (c) 2005 - 2020 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


#ifndef __gc_vsc_utils_math_h_
#define __gc_vsc_utils_math_h_

BEGIN_EXTERN_C()

/* Number range for integer types */
#define VSC_UINT32_MAX               (0xFFFFFFFF)
#define VSC_UINT32_MIN               (0x0)
#define VSC_INT32_MAX                (0x7FFFFFFF)
#define VSC_INT32_MIN                (-0x80000000)
#define VSC_UINT20_MAX               (0xFFFFF)
#define VSC_UINT20_MIN               (0x0)
#define VSC_INT20_MAX                (0x7FFFF)
#define VSC_INT20_MIN                (-0x80000)
#define VSC_UINT16_MAX               (0xFFFF)
#define VSC_UINT16_MIN               (0x0)
#define VSC_INT16_MAX                (0x7FFF)
#define VSC_INT16_MIN                (-0x8000)

/* S23E8 float format */
#define VSC_S23E8_MANTISSA_SHIFT     0
#define VSC_S23E8_MANTISSA_MASK      0x7FFFFF
#define VSC_S23E8_EXPONENT_SHIFT     23
#define VSC_S23E8_EXPONENT_MASK      0xFF
#define VSC_S23E8_SIGN_SHIFT         31
#define VSC_S23E8_SIGN_MASK          0x01
#define VSC_S23E8_EXPONENT_OFFSET    127

/* S11E8 float format */
#define VSC_S11E8_MANTISSA_SHIFT     0
#define VSC_S11E8_MANTISSA_MASK      0x7FF
#define VSC_S11E8_EXPONENT_SHIFT     11
#define VSC_S11E8_EXPONENT_MASK      0xFF
#define VSC_S11E8_SIGN_SHIFT         19
#define VSC_S11E8_SIGN_MASK          0x01
#define VSC_S11E8_EXPONENT_OFFSET    127

/* S10E5 float format */
#define VSC_S10E5_MANTISSA_SHIFT     0
#define VSC_S10E5_MANTISSA_MASK      0x3FF
#define VSC_S10E5_EXPONENT_SHIFT     10
#define VSC_S10E5_EXPONENT_MASK      0x1F
#define VSC_S10E5_SIGN_SHIFT         15
#define VSC_S10E5_SIGN_MASK          0x01

#define ENCODE_FLOAT(fpType, sign, exponent, mantissa)           \
            (((sign)     << VSC_##fpType##_SIGN_SHIFT)       |   \
             ((exponent) << VSC_##fpType##_EXPONENT_SHIFT)   |   \
             ((mantissa) << VSC_##fpType##_MANTISSA_SHIFT))

#define CAN_EXACTLY_CVT_S23E8_2_S11E8(fS23E8) (((fS23E8) & 0xfff) == 0)
#define CAN_EXACTLY_CVT_U32_2_U20(u32)        ((u32) <= VSC_UINT20_MAX)
#define CAN_EXACTLY_CVT_S32_2_S20(s32)        ((s32) > VSC_INT20_MIN && (s32) <= VSC_INT20_MAX)
#define CAN_EXACTLY_CVT_U32_2_U16(u32)        ((u32) <= VSC_UINT16_MAX)
#define CAN_EXACTLY_CVT_S32_2_S16(s32)        ((s32) > VSC_INT16_MIN && (s32) <= VSC_INT16_MAX)
#define CAN_EXACTLY_CVT_S23E8_2_S10E5(fS23E8) vscCanCvtS23E8FloatToS10E5Float((fS23E8))

gctBOOL vscCanCvtS23E8FloatToS10E5Float(gctUINT fS23E8);

gctUINT vscCvtS23E8FloatToS11E8Float(gctUINT floatS23E8);
gctUINT vscCvtS11E8FloatToS23E8Float(gctUINT floatS11E8);
gctUINT32 vscCvtS10E5FloatToS23E8Float(gctUINT32 val32);
gctUINT32 vscCvtS23E8FloatToS10E5Float(gctUINT32 val32);

END_EXTERN_C()

#endif /* __gc_vsc_utils_math_h_ */

