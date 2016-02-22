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

gctINT vscFindPopulation(gctUINT uInt)
{
    uInt = uInt - ((uInt >> 1) & 0x55555555);
    uInt = (uInt & 0x33333333) + ((uInt >> 2) & 0x33333333);
    uInt = (uInt + (uInt >> 4)) & 0x0f0f0f0f;
    uInt = (uInt + (uInt >> 8));
    uInt = (uInt + (uInt >> 16));

    return (uInt & 0x3f);
}

gctINT vscFindLeastSigBit(gctUINT uInt)
{
    register gctINT res;

    if (uInt == 0)
    {
        return INVALID_BIT_LOC;
    }

    uInt &= (~uInt + 1);
    res = (uInt & 0xAAAAAAAA) != 0;
    res |= ((uInt & 0xCCCCCCCC) != 0) << 1;
    res |= ((uInt & 0xF0F0F0F0) != 0) << 2;
    res |= ((uInt & 0xFF00FF00) != 0) << 3;
    res |= ((uInt & 0xFFFF0000) != 0) << 4;

    return res;
}

gctINT vscFindMostSigBit(gctUINT uInt)
{
    register gctUINT res = 0;

    if (uInt == 0)
    {
        return INVALID_BIT_LOC;
    }

    if ((uInt >> 16) > 0) {uInt >>= 16; res +=16;}
    if ((uInt >> 8) > 0) {uInt >>= 8; res += 8;}
    if ((uInt >> 4) > 0) {uInt >>= 4; res += 4;}
    if ((uInt >> 2) > 0) {uInt >>= 2; res += 2;}
    if ((uInt >> 1) > 0) {uInt >>= 1; res += 1;}

    return res;
}

