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

gctUINT vscCvtS23E8FloatToS11E8Float(gctUINT floatS23E8)
{
    gctUINT floatS11E8 = 0, sgn, exp, man;

    sgn = (floatS23E8 >> VSC_S23E8_SIGN_SHIFT) & VSC_S23E8_SIGN_MASK;
    exp = (floatS23E8 >> VSC_S23E8_EXPONENT_SHIFT) & VSC_S23E8_EXPONENT_MASK;
    man = ((floatS23E8 >> VSC_S23E8_MANTISSA_SHIFT) & VSC_S23E8_MANTISSA_MASK) >> 12;

    floatS11E8 = ENCODE_FLOAT(S11E8, sgn, exp, man);

    return floatS11E8;
}

gctUINT vscCvtS11E8FloatToS23E8Float(gctUINT floatS11E8)
{
    gctUINT floatS23E8 = 0, sgn, exp, man;

    sgn = (floatS11E8 >> VSC_S11E8_SIGN_SHIFT) & VSC_S11E8_SIGN_MASK;
    exp = (floatS11E8 >> VSC_S11E8_EXPONENT_SHIFT) & VSC_S11E8_EXPONENT_MASK;
    man = ((floatS11E8 >> VSC_S11E8_MANTISSA_SHIFT) & VSC_S11E8_MANTISSA_MASK) << 12;

    floatS23E8 = ENCODE_FLOAT(S23E8, sgn, exp, man);

    return floatS23E8;
}

