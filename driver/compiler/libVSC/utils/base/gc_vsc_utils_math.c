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


#include "gc_vsc.h"

gctBOOL vscCanCvtS23E8FloatToS10E5Float(gctUINT fS23E8)
{
    gctINT exp = (fS23E8 & 0x7f800000) >> 23;
    gctUINT man = fS23E8 & 0x7fffff;

    if(exp == 0xff)
    {
        return gcvTRUE;
    }

    if(man & 0x1fff)
    {
        return gcvFALSE;
    }

    if(exp == 0 && man == 0)
    {
        return gcvTRUE;
    }

    if((exp -127) == -15)
    {
        if(man & 0x3fff)
        {
            return gcvFALSE;
        }
        else
        {
            return gcvTRUE;
        }
    }

    if((exp - 127) > 15 || (exp - 127) < -14)
    {
        return gcvFALSE;
    }
    return gcvTRUE;
}

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

/*
gctUINT vscCvtS23E8FloatToS10E5Float(gctUINT floatS11E8, gctBOOL rtz)
{
    gctBOOL negtive = (floatS11E8 & 0x80000000);
    gctINT exp = (floatS11E8 & 0x7f800000) >> 23;
    gctUINT man = floatS11E8 & 0x7fffff;

    if(exp == 0xff)
    {
        if(man)
        {
            return 0x7e00;
        }
        else
        {
            if(negtive)
            {
                if(rtz)
                {
                    return 0x87ff;
                }
                else
                {
                    return 0xfc00;
                }
            }
            else
            {
                if(rtz)
                {
                    return 0x7bff;
                }
                else
                {
                    return 0x7c00;
                }
            }
        }
    }
    else if((exp - 127) > 15 || (exp - 127) < -14)
    {
        if(rtz)
        {

        }
        return gcvFALSE;
    }

    return gcvTRUE;
}
*/

