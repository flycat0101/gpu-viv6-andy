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

gctUINT32 vscCvtS10E5FloatToS23E8Float(gctUINT32 val16)
{
    gctUINT32 sign;
    gctUINT32 expotent;
    gctUINT32 mantissa;

    sign        = ((gctUINT32)val16 >> VSC_S10E5_SIGN_SHIFT) & VSC_S10E5_SIGN_MASK;
    expotent    = ((gctUINT32)val16 >> VSC_S10E5_EXPONENT_SHIFT) & VSC_S10E5_EXPONENT_MASK;
    mantissa    = (gctUINT32)val16 & VSC_S10E5_MANTISSA_MASK;

    if (expotent == 0u)
    {
        if (mantissa == 0u)
        {
            /* +/- 0 */
            return (gctUINT32)(sign << VSC_S23E8_SIGN_SHIFT);
        }
        else
        {
            /* Denormalized, normalize it. */

            while (!(mantissa & 0x00000400u))
            {
                mantissa <<= 1u;
                expotent -=  1u;
            }

            expotent += 1u;
            mantissa &= ~0x00000400u;
        }
    }
    else if (expotent == 31u)
    {
        if (mantissa == 0u)
        {
            /* +/- InF */
            return (gctUINT32)((sign << VSC_S23E8_SIGN_SHIFT) | 0x7f800000u);
        }
        else
        {
            /* +/- NaN */
            return (gctUINT32)((sign << VSC_S23E8_SIGN_SHIFT) | 0x7f800000u | (mantissa << 13u));
        }
    }

    expotent = expotent + (127u - 15u);
    mantissa = mantissa << 13u;

    return (gctUINT32)((sign << VSC_S23E8_SIGN_SHIFT) | (expotent << VSC_S23E8_EXPONENT_SHIFT) | mantissa);
}

gctUINT32 vscCvtS23E8FloatToS10E5Float(gctUINT32 val32)
{
    gctUINT32       sign;
    gctINT          expotent;
    gctUINT32       mantissa;

    sign        = (val32 >> 16u) & 0x00008000u;
    expotent    = (gctINT)((val32 >> VSC_S23E8_EXPONENT_SHIFT) & VSC_S23E8_EXPONENT_MASK) - (127 - 15);
    mantissa    = val32 & 0x007fffffu;

    if (expotent <= 0)
    {
        if (expotent < 0)
        {
            /* Rounds to zero. */
            return (gctUINT32)(gctUINT16) sign;
        }

        /* Converted to denormalized half, add leading 1 to significand. */
        mantissa = mantissa | 0x00800000u;

        /* Round mantissa to nearest (10+e) */
        {
            gctUINT32 t = 14u - expotent;
            gctUINT32 a = (1u << (t - 1u)) - 1u;
            gctUINT32 b = (mantissa >> t) & 1u;

            mantissa = (mantissa + a + b) >> t;
        }

        return (gctUINT32)(gctUINT16) (sign | mantissa);
    }
    else if (expotent == 0xff - (127 - 15))
    {
        if (mantissa == 0u)
        {
            return (gctUINT32)(gctUINT16) (sign | 0x7c00u);
        }
        else
        {
            mantissa >>= 13u;
            return (gctUINT32)(gctUINT16) (sign | 0x7c00u | mantissa | (mantissa == 0u));
        }
    }
    else
    {
        /* Normalized float. */
        mantissa = mantissa + 0x00000fffu + ((mantissa >> 13u) & 1u);

        if (mantissa & 0x00800000u)
        {
            mantissa  = 0u;
            expotent += 1;
        }

        if (expotent > 30)
        {
            return (gctUINT32)(gctUINT16) (sign | 0x7c00u);
        }
        return (gctUINT32)(gctUINT16) (sign | ((gctUINT32)expotent << 10u) | (mantissa >> 13u));
    }
}

