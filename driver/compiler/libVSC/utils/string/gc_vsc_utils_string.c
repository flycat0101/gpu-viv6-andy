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

gctUINT32 vscSTR_CharToUint32(gctCHAR ch,
                              gctUINT32 base)
{
    do
    {
        if(base == 10 && ch >= '0' && ch <= '9')
        {
            break;
        }
        if(base == 8 && ch >= '0' && ch <= '7')
        {
            break;
        }
        if(base == 16 && ((ch >= '0' && ch <= '9') || (ch >= 'a' && ch <= 'f') || (ch >= 'A' && ch <= 'F')))
        {
            break;
        }
        return (gctUINT32)-1;
    } while(0);

    if(ch >= '0' && ch <= '9')
    {
        return ch - '0';
    }
    if(ch >= 'a' && ch <= 'f')
    {
        return ch - 'a' + 10;
    }
    if(ch >= 'A' && ch <= 'F')
    {
        return ch - 'A' + 10;
    }

    /* never get here */
    return 0;
}

gctUINT32 vscSTR_StrToUint32(gctSTRING str,
                             gctUINT32 len)
{
    gctUINT32 base;
    gctUINT32 cur_base = 1;
    gctSTRING pos = str + len - 1;
    gctUINT32 result = 0;

    if(str[0] == '0' && str[1] == 'x')
    {
        base = 16;
        str += 2;
    }
    else if(str[0] == '0')
    {
        base = 8;
        str++;
    }
    else
    {
        base = 10;
    }

    while(pos >= str)
    {
        gctUINT32 char_int = vscSTR_CharToUint32(*pos, base);
        if(char_int == (gctUINT32)-1)
        {
            return 0;
        }
        result = result + cur_base * char_int;
        cur_base *= base;
        pos--;
    }

    return result;
}

