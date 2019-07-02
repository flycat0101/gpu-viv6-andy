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


#include <gc_vx_common.h>

#define _GC_OBJ_ZONE            gcdZONE_VX_KERNEL

#define _cldPrintfConvSpecifiers "diouxXfFeEgGaAcsp"
#define _cldDigits               "0123456789"
#define _cldFlags                "-+ #0"
#define _cldLowercase "0123456789abcdefghijklmnopqrstuvwxyz"
#define _cldUppercase "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"

#define LEFT 0x01
#define PLUS 0x02
#define SPACE 0x04
#define SPECIAL 0x08
#define ZEROPAD 0x10
#define SMALL 0x20
#define LARGE 0x40
#define SIGN 0x80

typedef enum _cleARGTYPE
{
    cleARGTYPE_CHAR = 1,
    cleARGTYPE_UCHAR,
    cleARGTYPE_SHORT,
    cleARGTYPE_USHORT,
    cleARGTYPE_LONG,
    cleARGTYPE_ULONG,
    cleARGTYPE_HALF,
    cleARGTYPE_INT,
    cleARGTYPE_UINT,
    cleARGTYPE_FLOAT,
    cleARGTYPE_DOUBLE

}cleARGTYPE;

gctBOOL gcfVX_IsNan(gctFLOAT val)
{
    gcuFLOAT_UINT32 uValue;
    uValue.f = val;
    if ((uValue.u & (gctUINT)0x7FFFFFFF) > (gctUINT)0x7F800000)
    {
        return gcvTRUE;
    }else{
        return gcvFALSE;
    }
}

gctBOOL gcfVX_IsInf(gctFLOAT val)
{
    gcuFLOAT_UINT32 uValue;
    uValue.f = val;
    if ((uValue.u & (gctUINT)0x7FFFFFFF) == (gctUINT)0x7F800000)
    {
        return gcvTRUE;
    }else{
        return gcvFALSE;
    }
}

gctBOOL gcfVX_IsInString(gctCHAR s, gctCHAR* string)
{
    while (*string)
    {
        if (s != *string++)
            continue;
        return gcvTRUE;
    }
    return gcvFALSE;
}

void
gcfVX_GetSingleFormat(
    gctCHAR* StartPtr,
    gctCHAR* EndPtr,
    gctCHAR* Format,
    gctINT*  VectorSize,
    gctINT*  ArgType,
    gctINT*  Flags,
    gctINT*  FieldWidth,
    gctINT*  Precision
    )
{
    gctCHAR chr;
    gctCHAR followChr;
    gctBOOL isVectorSpecifier = gcvFALSE;

    gctINT vectorSize = 0;
    *ArgType = 0;
    *VectorSize = 0;
    *Flags = 0;
    *FieldWidth = 0;
    *Precision = 0;

    while (StartPtr <= EndPtr) {
         chr = *StartPtr++;

         switch(chr) {
         case 'v':
             if (StartPtr < EndPtr)
             {
                 chr = *StartPtr++;
                 switch(chr) {
                 case '1':
                     chr = *StartPtr++;
                     if(StartPtr < EndPtr) {
                         if(chr != '6') return;
                     }
                     else return;
                     vectorSize = 16;
                     break;
                 case '2':
                     vectorSize = 2;
                     break;

                 case '3':
                     vectorSize = 3;
                     break;

                 case '4':
                     vectorSize = 4;
                     break;

                 case '8':
                     vectorSize = 8;
                     break;

                 default:
                     return;
                 }
             }
             else return;
             isVectorSpecifier = gcvTRUE;
             break;

         case 'h':
             *Format++ = chr;
             followChr = *StartPtr;
             if ((followChr == 'd') || (followChr == 'i'))
             {
                 *ArgType = cleARGTYPE_SHORT;
             }else if ((followChr == 'o') || (followChr == 'u') || (followChr == 'x') || (followChr == 'X'))
             {
                 *ArgType = cleARGTYPE_USHORT;
             }else if ((followChr == 'a') || (followChr == 'A') || (followChr == 'e') || (followChr == 'E') ||
                       (followChr == 'f') || (followChr == 'F') || (followChr == 'g') || (followChr == 'G'))
             {
                 *ArgType = cleARGTYPE_HALF;
             }
             switch (*StartPtr) {
             case 'h':
                 *Format++ = 'h';
                 StartPtr++;
                 followChr = *StartPtr;
                 if ((followChr == 'd') || (followChr == 'o'))
                 {
                     *ArgType = cleARGTYPE_CHAR;
                 }else if ((followChr == 'o') || (followChr == 'u') || (followChr == 'x') || (followChr == 'X'))
                 {
                     *ArgType = cleARGTYPE_UCHAR;
                 }
                 break;

             case 'l':
                 if(isVectorSpecifier) {  /* hl applies to vector specifier appeared only */
                     Format--;
                     StartPtr++;
                     followChr = *StartPtr;
                     if ((followChr == 'd') || (followChr == 'o'))
                     {
                         *ArgType = cleARGTYPE_INT;
                     }else if ((followChr == 'o') || (followChr == 'u') || (followChr == 'x') || (followChr == 'X'))
                     {
                         *ArgType = cleARGTYPE_UINT;
                     }else if ((followChr == 'a') || (followChr == 'A') || (followChr == 'e') || (followChr == 'E') ||
                               (followChr == 'f') || (followChr == 'F') || (followChr == 'g') || (followChr == 'G'))
                     {
                         *ArgType = cleARGTYPE_FLOAT;
                     }
                 }
                 else return;
                 break;

             default:
                 break;
             }

             break;

         case 'l':
             *Format++ = chr;
             followChr = *StartPtr;
              if (followChr == 'd' || followChr == 'o')
              {
                  *ArgType = cleARGTYPE_LONG;
              }else if (followChr == 'o' || followChr == 'u' || followChr == 'x' || followChr == 'X')
              {
                  *ArgType = cleARGTYPE_ULONG;
              }else if (followChr == 'a' || followChr == 'A' || followChr == 'e' || followChr == 'E' ||
                        followChr == 'f' || followChr == 'F' || followChr == 'g' || followChr == 'G')
              {
                  *ArgType = cleARGTYPE_DOUBLE;
              }
             break;

         default:
             {
                 /* process flags */
                 if(gcfVX_IsInString(chr, (gctCHAR*)_cldFlags))
                 {
                     *Format++ = chr;
                     switch(chr)
                     {
                     case '-':
                         *Flags |= LEFT;
                         break;
                     case '+':
                         *Flags |= PLUS;
                         break;
                     case ' ':
                         *Flags |= SPACE;
                         break;
                     case '#':
                         *Flags |= SPECIAL;
                         break;
                     case '0':
                         *Flags |= ZEROPAD;
                         break;
                     }
                 }else if (gcfVX_IsInString(chr, (gctCHAR*)_cldDigits)) /* get field width */
                 {
                     gctINT i = 0;
                     do
                     {
                         *Format++ = chr;
                         i = i*10 + chr - '0';
                         chr = *StartPtr++;
                     }while (gcfVX_IsInString(chr, (gctCHAR*)_cldDigits));
                     *FieldWidth = i;
                     StartPtr--;
                 }else if (chr == '.') /* get the precision */
                 {
                     *Format++ = chr;
                     chr = *StartPtr++;
                     if (gcfVX_IsInString(chr, (gctCHAR*)_cldDigits))
                     {
                         gctINT i = 0;
                         do
                         {
                             *Format++ = chr;
                             i = i*10 + chr - '0';
                             chr = *StartPtr++;
                         }while (gcfVX_IsInString(chr, (gctCHAR*)_cldDigits));
                         *Precision = i;
                         StartPtr--;
                     }
                     if (*Precision < 0)
                     {
                         *Precision = 0;
                     }
                 }else{
                     *Format++ = chr;
                 }
                 break;
             }
        }
    }

    *VectorSize = vectorSize;
    return;
}

void floatToaHex(
    double Num,
    gctINT Precision,
    gctBOOL IsUpperCase,
    gctCHAR* Buff,
    gctINT* DecimalPos,
    gctINT* Exp)
{
    gctINT intData;
    gctINT i = 0;
    gctINT j = 0;
    double decimalData;
    gctBOOL binaryData[255] = {0};
    gctBOOL binaryInt[255] = {0};
    gctCHAR* dig = (char*)_cldLowercase;
    gctINT len;

    if (IsUpperCase)
    {
        dig = (char*)_cldUppercase;
    }
    /* Process value sign */
    if (Num < 0.0)
    {
        Num = -Num;
        *Buff++ = '-';
    }
    else
    {
        *Buff++ = '+';
    }

    if (Num > -0.000005 && Num < 0.000005)
    {
        for (i = 0; i < Precision + 1; i++)
        {
            *Buff++ = '0';
        }
        *Buff++ = '\0';
        *DecimalPos = 1;
        *Exp = 0;
        return;
    }

    intData = (gctINT)Num;
    decimalData = Num - intData;
    *Exp = 0;

    if (intData == 0)
    {
        j = 0;
        while(decimalData < 1.0)
        {
            decimalData *= 2;
            j++;
        }
        *Exp = -j;
        intData = (int)decimalData;
        decimalData -= intData;
    }
    /* handle integer part */
    while(intData)
    {
        binaryInt[i++] = (intData % 2 == 1) ? gcvTRUE : gcvFALSE;
        intData /= 2;
    }
    *Exp = ((*Exp) == 0) ? i - 1 : *Exp;
    j = 0;
    while(i)
    {
        binaryData[j++] = binaryInt[--i];
    }

    if (decimalData > -0.000005 && decimalData < 0.000005)
    {
        for(i = 0; i < 4 * Precision; i++)
        {
            binaryData[j++] = gcvFALSE;
        }
    }else
    {
        /* handle decimal part */
        while(decimalData != 0.0)
        {
            gctINT t = (gctINT)(decimalData * 2);
            binaryData[j++] = (t > 0) ? gcvTRUE : gcvFALSE;
            decimalData = decimalData * 2 - t;
        }
    }

    len = j - 1;

    if (len % 4)
    {
        memset(binaryData + len + 1, 0, 4 - len % 4);
        len += 4 - len % 4;
    }

    *Buff++ = binaryData[0] ? '1':'0';
    *DecimalPos = 1;

    i = 0;
    j = 0;
    while((i < len))
    {
        *Buff++ = dig[((binaryData[i+1] ? 8 : 0) + (binaryData[i+2] ? 4 : 0) + (binaryData[i+3] ? 2 : 0) + (binaryData[i+4] ? 1 : 0)) % 16];
        i += 4;
        j++;
    }
    *Buff++ = '\0';
}

void fltRound(
    gctCHAR* NumBuf,
    gctINT* DecimalPos,
    gctINT Precision,
    gctBOOL IsUppderCase)
{
    gctCHAR* last_digit = NumBuf + 1 +Precision;
    gctCHAR* after_last = last_digit + 1;

    if (*after_last > '4')
    {
        gctCHAR *p = last_digit;
        gctINT carry = 1;

        do
        {
            gctINT sum;
            if (*p == '.')
            {
                p--;
            }
            if (IsUppderCase)
            {
                if (*p == '9')
                {
                    *p-- = 'A';
                    carry = 0;
                }
                else
                {
                    sum = *p + carry;
                    carry = sum > 'F';
                    *p-- = (gctCHAR)(sum - carry * 23);
                }
            }else
            {
                if (*p == '9')
                {
                    *p-- = 'a';
                    carry = 0;
                }
                else
                {
                    sum = *p + carry;
                    carry = sum > 'f';
                    *p-- = (gctCHAR)(sum - carry * 55);
                }
            }
        } while (carry && p >= NumBuf);

        /* We have fffff... which needs to be rounded to 100000.. */
        if (carry && p == NumBuf)
        {
            *p = '1';
            *DecimalPos += 1;
        }
    }
}

void floatToText(
    double Value,
    gctINT Precision,
    gctCHAR *Buf,
    gctBOOL IsUpperCase)
{
    char chBuffer[255] = {'\0'};

    char *cvtBuf = chBuffer;
    int decimalPos;
    int pos;
    int exp;

    floatToaHex(Value, Precision, IsUpperCase, cvtBuf, &decimalPos, &exp);
    fltRound(cvtBuf, &decimalPos, Precision, IsUpperCase);

    if ('-' == *cvtBuf++)
    {
        *Buf++ = '-';
    }

    if (*cvtBuf)
    {
        *Buf++ = '0';
        *Buf++ = IsUpperCase ? 'X' : 'x';
        *Buf++ = *cvtBuf;
        if (Precision > 0)
        {
            *Buf++ = '.';
        }
        gcoOS_MemCopy(Buf, cvtBuf + 1, Precision);
        Buf += Precision;
        {
            *Buf++ = IsUpperCase ? 'P' : 'p';
        }

        if (exp < 0)
        {
            *Buf++ = '-';
            exp = -exp;
        }else{
            *Buf++ = '+';
        }

        do
        {
            if ((exp / 10) == 0)
            {
                *Buf++ = (gctCHAR)(exp % 10 + '0');
                break;
            }
            else
            {
                *Buf++ = (gctCHAR)(exp / 10 + '0');
                exp = exp % 10;
            }
        }while(exp);
    }
    else
    {
        *Buf++ = '0';
        if (Precision > 0)
        {
            *Buf++ = '.';
            for (pos = 0; pos < Precision; pos++)
            {
                *Buf++ = '0';
            }
        }
    }
}

void printf_aA(
    gctCHAR *Str,
    double Num,
    gctINT Size,
    gctINT Precision,
    gctCHAR Fmt,
    gctINT Flags)
{
    gctCHAR tmp[255] = {'\0'};
    gctCHAR c, sign;
    gctINT n, i;
    gctBOOL isUpperCase = gcvFALSE;

    if (Flags & LEFT) Flags &= ~ZEROPAD;

    c = (Flags & ZEROPAD) ? '0' : ' ';
    sign = 0;
    if (Flags & SIGN)
    {
        if (Num < 0.0)
        {
            sign = '-';
            Num = -Num;
            Size--;
        }
        else if (Flags & PLUS)
        {
            sign = '+';
            Size--;
        }
        else if (Flags & SPACE)
        {
            sign = ' ';
            Size--;
        }
    }

    if (Precision < 0)
    {
        Precision = 6; /* Default precision: 6 */
    }

    isUpperCase = (Fmt == 'A') ? gcvTRUE : gcvFALSE;

    floatToText(Num, Precision, tmp, isUpperCase);

    n = (gctINT)strlen(tmp);

    Size -= n;
    if (!(Flags & (ZEROPAD | LEFT)))
    {
        while (Size-- > 0)
        {
            *Str++ = ' ';
        }
    }
    if (sign)
    {
        *Str++ = sign;
    }
    if (!(Flags & LEFT))
    {
        while (Size-- > 0)
        {
            *Str++ = c;
        }
    }
    for (i = 0; i < n; i++)
    {
        *Str++ = tmp[i];
    }
    while (Size-- > 0)
    {
        *Str++ = ' ';
    }
}

void
gcfVX_PrintData(
    gctPOINTER** Data,
    gctCHAR* Format,
    gctINT ArgType,
    gctCHAR FollowType,
    gctINT Flags,
    gctINT FieldWidth,
    gctINT Precision,
    gctBOOL IsDoublePrecision)
{
    if (ArgType)
    {
        switch (ArgType)
        {
        case cleARGTYPE_CHAR:
            printf(Format, *(gctINT8*)(**Data));
            **Data = (gctCHAR*)(**Data) + 4;
            break;
        case cleARGTYPE_UCHAR:
            printf(Format, *(gctUINT8*)(**Data));
            **Data = (gctUINT8*)(**Data) + 4;
            break;
        case cleARGTYPE_SHORT:
            printf(Format, *(gctINT16*)(**Data));
            **Data = (gctINT16*)(**Data) + 2;
            break;
        case cleARGTYPE_USHORT:
            printf(Format, *(gctUINT16*)(**Data));
            **Data = (gctUINT16*)(**Data) + 2;
            break;
        case cleARGTYPE_LONG:
            printf(Format, *(gctINT64*)(**Data));
            **Data = (gctINT64*)(**Data) + 1;
            break;
        case cleARGTYPE_ULONG:
            printf(Format, *(gctINT64*)(**Data));
            **Data = (gctINT64*)(**Data) + 1;
            break;
        case cleARGTYPE_HALF:
            printf(Format, *(gctINT16*)(**Data));
            **Data = (gctINT16*)(**Data) + 1;
            break;
        case cleARGTYPE_INT:
            printf(Format, *(gctINT32*)(**Data));
            **Data = (gctINT32*)(**Data) + 1;
            break;
        case cleARGTYPE_UINT:
            printf(Format, *(gctUINT32*)(**Data));
            **Data = (gctUINT32*)(**Data) + 1;
            break;
        case cleARGTYPE_FLOAT:
            {
                gctFLOAT value = *(gctFLOAT*)(**Data);
                if (gcfVX_IsNan(value))
                {
                    printf("%s", "nan");
                }else if (gcfVX_IsInf(value))
                {
                    printf("%s", "inf");
                }else{
                    if (FollowType == 'a' || FollowType == 'A')
                    {
                        gctCHAR tmpBuf[512] = {'\0'};
                        printf_aA(tmpBuf, *(gctFLOAT*)(**Data), FieldWidth, Precision, FollowType, Flags);
                        printf("%s", tmpBuf);
                    }else{
                        printf(Format, value);
                    }
                }

                if (IsDoublePrecision == 0)
                {
                    **Data = (gctFLOAT*)(**Data) + 1;
                }else{
                    **Data = (double*)(**Data) + 1;
                }
            }
            break;
        case cleARGTYPE_DOUBLE:
            {
                gctFLOAT value = *(gctFLOAT*)(**Data);
                if (gcfVX_IsNan(value))
                {
                    printf("%s", "nan");
                }else if (gcfVX_IsInf(value))
                {
                    printf("%s", "inf");
                }else{
                    if (FollowType == 'a' || FollowType == 'A')
                    {
                        gctCHAR tmpBuf[512] = {'\0'};
                        printf_aA(tmpBuf, *(double*)(**Data), FieldWidth, Precision, FollowType, Flags);
                        printf("%s", tmpBuf);
                    }else{
                        printf(Format, *(double*)(**Data));
                    }
                }
                if (IsDoublePrecision == 0)
                {
                    **Data = (gctFLOAT*)(**Data) + 1;
                }else{
                    **Data = (double*)(**Data) + 1;
                }
            }
            break;
        default:
            break;
        }
    }
    else
    {
        switch(FollowType)
        {
         case 'd':
         case 'i':
         case 'o':
         case 'u':
         case 'x':
         case 'X':
             printf(Format, *(gctINT32*)(**Data));
             **Data = (gctINT32*)(**Data) + 1;
             break;
         case 'a':
         case 'A':
         case 'e':
         case 'E':
         case 'f':
         case 'F':
         case 'g':
         case 'G':
             {
                 gctFLOAT value = *(gctFLOAT*)(**Data);
                 if (gcfVX_IsNan(value))
                 {
                     printf("%s", "nan");
                 }else if (gcfVX_IsInf(value))
                 {
                     printf("%s", "inf");
                 }else if (FollowType == 'a' || FollowType == 'A')
                 {
                     gctCHAR tmpBuf[512] = {'\0'};
                     printf_aA(tmpBuf, value, FieldWidth, Precision, FollowType, Flags);
                     printf("%s", tmpBuf);
                 }else
                 {
                     printf(Format, value);
                 }
                 **Data = (gctFLOAT*)(**Data) + 1;
             }
             break;
         case 'c':
             printf(Format, *(gctINT8*)(**Data));
             **Data = (gctINT8*)(**Data) + 4;
             break;
         default:
            break;
        }
    }
}

void
gcfVX_PrintfFmt(
    gctCHAR* FmtString,
    gctCHAR SpecificChar,
    gctPOINTER* Data,
    gctINT VectorSize,
    gctINT ArgType,
    gctINT Flags,
    gctINT FieldWidth,
    gctINT Precision
    )
{
    gctINT i = 0;
    gctINT numVector = (VectorSize == 0) ? 1 : VectorSize;
    gctINT isDoublePrecision = 0;
    /* Get precision info */
    isDoublePrecision = *(gctINT*)(*Data);
    /* Get Data */
    *Data = (gctINT*)(*Data) + 1;
    switch(SpecificChar) {
        case 'd':
        case 'i':
        case 'o':
        case 'u':
        case 'x':
        case 'X':
        case 'f':
        case 'F':
        case 'e':
        case 'E':
        case 'g':
        case 'G':
        case 'a':
        case 'A':
        case 'c':
            while (i < numVector)
            {
                if (i > 0 && i < numVector)
                {
                    printf(",");
                }
                gcfVX_PrintData(&Data, FmtString, ArgType, SpecificChar, Flags, FieldWidth, Precision, isDoublePrecision);
                i++;
            }
            break;

        case 'p':
            printf("%016x", *(gctUINT*)(*Data));
            *Data = (gctUINT*)(*Data) + 1;
            break;

        default:
            break;
    }
}

gctPOINTER gcfVX_PrintParseData(
    gctCHAR *formatString,
    gctPOINTER data)
{
    gctCHAR* bufPtr = formatString;

    while(*bufPtr) {
        gctCHAR c = *bufPtr++;

        if(c == '%') {
            if(*bufPtr == '%') {
                bufPtr++;
                continue;
            }else {
                /*indentify the conversion specification */
                gctCHAR* startPtr = bufPtr - 1;
                while (*bufPtr)
                {
                    c = *bufPtr++;
                    if (gcfVX_IsInString(c, (gctCHAR*)_cldPrintfConvSpecifiers))
                    {
                        gctINT vectorSize = 0;
                        gctINT argType = 0;
                        gctINT flags = 0;
                        gctINT fieldWidth = 0;
                        gctINT precision = 0;
                        gctCHAR* endPtr = bufPtr - 1;
                        gctCHAR fmt[255] = {'\0'};
                        /* get vector size and length and new format */
                        gcfVX_GetSingleFormat(startPtr, endPtr, fmt, &vectorSize, &argType, &flags, &fieldWidth, &precision);
                        if (c == 's')
                        {
                            gctUINT value = *((gctINT*)data + 1);
                            if (value == 0xFFFFFFFF) /* handle printf("%s", 0);*/
                            {
                                printf(fmt, "(null)");
                            }else{
                                /* the string is stored in const buffer, offset is stored in printf buffer */
                                gctUINT offset = value;
                                printf(fmt, formatString + offset);
                            }
                            data = (gctINT*)data + 2;
                        }else{
                            gcfVX_PrintfFmt(fmt, c, &data, vectorSize, argType, flags, fieldWidth, precision);
                        }
                        break;
                    }
                }
            }
        }else {
            gctINT constStrLen = 0;
            gctCHAR* constString;
            gctCHAR* startConstStrPtr;
            bufPtr--;
            startConstStrPtr = bufPtr;

            while((*bufPtr) && (*bufPtr != '%'))
            {
                constStrLen++;
                bufPtr++;
            }
            gcoOS_Allocate(gcvNULL, (constStrLen+1)*sizeof(gctCHAR), (gctPOINTER *)&constString);
            gcoOS_StrCopySafe(constString, constStrLen + 1, startConstStrPtr);
            *(constString + constStrLen) = '\0';
            printf("%s", constString);
            gcoOS_Free(gcvNULL, constString);
        }
    }

    return data;
}

static gceSTATUS
    gcfVX_SetUniformImageInfo(IN gcUNIFORM Uniform,
                              IN gcsVX_IMAGE_INFO_PTR Info
                              )
{
    return gcoVX_SetImageInfo(Uniform, Info);
}

static gceSTATUS
gcfVX_SetUniformValue(
    IN gcUNIFORM Uniform,
    IN gctUINT32 Count,
    IN const gctINT * Value
    )
{
#if gcdNULL_DRIVER < 2
    gceSTATUS status;
    gctUINT32 columns, rows;

    gcmHEADER_ARG("Uniform=0x%x Count=%lu Value=0x%x", Uniform, Count, Value);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Uniform, gcvOBJ_UNIFORM);
    gcmDEBUG_VERIFY_ARGUMENT(Count > 0);
    gcmDEBUG_VERIFY_ARGUMENT(Value != gcvNULL);

    gcTYPE_GetTypeInfo(Uniform->u.type, &columns, &rows, 0);
    rows *= gcmMIN((gctINT) Count, Uniform->arraySize);

    /* Program the uniform. */
    status = gcoSHADER_BindUniform(gcvNULL,
                                   Uniform->address,
                                   GetUniformPhysical(Uniform),
                                   columns, rows,
                                   1,gcvFALSE,
                                   columns * 4,
                                   0,
                                   (gctPOINTER) Value,
                                   gcvUNIFORMCVT_NONE,
                                   /*Uniform->shaderKind*/gcSHADER_TYPE_CL);

    gcmFOOTER();
    return status;
#else
    return gcvSTATUS_OK;
#endif
}


static gceSTATUS
gcfVX_SetUniformValueCombinedMode(
    IN gcUNIFORM Uniform,
    IN gctUINT32 Count,
    IN  gctINT * Values[],
    IN gctUINT ValuesCount
    )
{
#if gcdNULL_DRIVER < 2
    gceSTATUS status;
    gctUINT32 columns, rows;
    gcmHEADER_ARG("Uniform=0x%x Count=%lu Value=0x%x", Uniform, Count, Values);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Uniform, gcvOBJ_UNIFORM);
    gcmDEBUG_VERIFY_ARGUMENT(Count > 0);
    gcmDEBUG_VERIFY_ARGUMENT(Values != gcvNULL);
    gcTYPE_GetTypeInfo(Uniform->u.type, &columns, &rows, 0);
    rows *= gcmMIN((gctINT) Count, Uniform->arraySize);

    if(isUniformTempRegSpillAddress(Uniform))  /*Special handle TemRegSpill Uniform, the type is UINTX_2, but only used the first component in shader*/
    {
        columns = 1;
    }

    /* Program the uniform. */
    status = gcoSHADER_BindUniformCombinedMode(gcvNULL,
                                   Uniform->address,
                                   GetUniformPhysical(Uniform),
                                   columns, rows,
                                   1,gcvFALSE,
                                   columns * 4,
                                   0,
                                   (gctPOINTER) Values,
                                   ValuesCount,
                                   gcvUNIFORMCVT_NONE,
                                   Uniform->shaderKind
                                   );

    gcmFOOTER();
    return status;
#else
    return gcvSTATUS_OK;
#endif
}


gceSTATUS
gcfVX_AdjustLocalWorkSize(
    vx_shader    Kernel,
    gctUINT             WorkDim,
    size_t              GlobalWorkOffset[3],
    size_t              GlobalWorkSize[3],
    size_t              LocalWorkSize[3]
    )
{
    gcmHEADER_ARG("Kernel=%p, WorkDim=0x%x, GlobalWorkOffset=%p, GlobalWorkSize=%p, LocalWorkSize=%p",
        Kernel, WorkDim, GlobalWorkOffset, GlobalWorkSize, LocalWorkSize);
    if (LocalWorkSize[0] == 0 &&
        (WorkDim < 2 || LocalWorkSize[1] == 0) &&
        (WorkDim < 3 || LocalWorkSize[2] == 0))
    {
        gctSIZE_T           preferredWorkGroupSize;
        gctSIZE_T           workGroupSize = 1;
        gctUINT             i;

        /* Find the largest workgroup size which has a common multiple with
         * preferred workgroup size but still less than max work group size
         */
        for (i = 0; i < WorkDim; i++)
        {
            preferredWorkGroupSize = Kernel->preferredWorkGroupSizeMultiple;
            while (preferredWorkGroupSize % 2 == 0)
            {
                if ((GlobalWorkSize[i] % preferredWorkGroupSize == 0) &&
                    (workGroupSize * preferredWorkGroupSize <= Kernel->maxWorkGroupSize))
                {
                    LocalWorkSize[i] = preferredWorkGroupSize;
                    workGroupSize *= LocalWorkSize[i];
                    break;
                }
                preferredWorkGroupSize /= 2;
            }
        }

        if (workGroupSize == 1)
        {

            /* No common multiple found
             * Try adjusting wrt global work size
             */
            for (i=0; i<WorkDim; i++)
            {
                if (workGroupSize * GlobalWorkSize[i] <= Kernel->maxWorkGroupSize)
                {
                    LocalWorkSize[i] = GlobalWorkSize[i];
                    workGroupSize *= LocalWorkSize[i];
                }
            }
        }

        /*not lucky, so just keep localworksize invalid */
        for (i = 0; i< WorkDim; i++)
        {
            if (LocalWorkSize[i] == 0)
            {
                LocalWorkSize[i] = 1;
            }
        }
    }

    gcmFOOTER_ARG("%d", gcvSTATUS_OK);
    return gcvSTATUS_OK;
}


/* Return how many uniform entries will be reported to app */
static gctUINT
gcfVX_GetUniformArrayInfo(
    gcUNIFORM uniform,
    gctUINT *maxNameLen, /* max possible name len of this entry, exclude bottom-level "[0]" */
    gctBOOL *isArray,
    gctUINT *arraySize
)
{
    gctINT j;
    gctUINT32 length;
    gctUINT entries = 1;

    gcmHEADER_ARG("uniform=%p, maxNameLen=%p, isArray=%p, arraySize=%p", uniform, maxNameLen, isArray, arraySize);
    gcUNIFORM_GetName(uniform, &length, gcvNULL);

    /* Multiple entries will be reported for array of arrays.
    ** If a uniform is an array, or array of array, its name length should be added
    ** with several dim of array indices, like "name[x]...[x][0]".
    */
    for (j = 0; j < uniform->arrayLengthCount - 1; ++j)
    {
        gctUINT decimalLen = 1;
        gctINT arrayLen = uniform->arrayLengthList[j];

        gcmASSERT(arrayLen > 0);
        entries *= arrayLen;

        arrayLen--;    /* Get max arrayIndex */
        while (arrayLen >= 10)
        {
            ++decimalLen;
            arrayLen /= 10;
        }

        length += (decimalLen + 2);
    }

    if (maxNameLen)
    {
        *maxNameLen = length;
    }

    if (isArray)
    {
        *isArray = uniform->arrayLengthCount > 0 ? gcvTRUE : gcvFALSE;
    }

    if (arraySize)
    {
        *arraySize = uniform->arrayLengthCount > 0
                   ? (uniform->arrayLengthList[uniform->arrayLengthCount - 1] > 0 ?
                      uniform->arrayLengthList[uniform->arrayLengthCount - 1] : 0)
                   : 1;
    }

    gcmFOOTER_ARG("0x%x", entries);
    return entries;
}


static gceSTATUS
gcfVX_LoadKernelArgValues(
    vx_shader           Kernel,
    gcSHADER            Shader,
    vx_argument         Arg,
    vx_border_mode_t*   BorderMode,
    vx_uint32           batchID,
    gctUINT             WorkDim,
    size_t              GlobalWorkOffset[3],
    size_t              GlobalWorkScale[3],
    size_t              GlobalWorkSize[3],
    size_t              LocalWorkSize[3]
    )
{
    gcSHADER_TYPE       type;
    gctUINT             length;
    gcSL_FORMAT         format;
    gctBOOL             isPointer;
    gceUNIFORM_FLAGS    flags;
    gceSTATUS           status;
    gctUINT32 gpuCount, perGpuMemSize;
    gctINT totalNumGroups = (gctINT)(GlobalWorkSize[0] / (LocalWorkSize[0] ? LocalWorkSize[0] : 1)
                            * (WorkDim > 1 ? (GlobalWorkSize[1] / (LocalWorkSize[1] ? LocalWorkSize[1] : 1)) : 1)
                            * (WorkDim > 2 ? (GlobalWorkSize[2] / (LocalWorkSize[2] ? LocalWorkSize[2] : 1)) : 1));

    gctINT totalNumItems  = (gctINT)(GlobalWorkSize[0]
                            * (WorkDim > 1 ? GlobalWorkSize[1] : 1)
                            * (WorkDim > 2 ? GlobalWorkSize[2] : 1));

    gcmHEADER_ARG("Kernel=%p, Shader=%p, Arg=%p, BorderMode=%p, batchID=0x%x, WorkDim=0x%x, GlobalWorkOffset=%p, GlobalWorkScale=%p, GlobalWorkSize=%p, LocalWorkSize=%p",
        Kernel, Shader, Arg, BorderMode, batchID, WorkDim, GlobalWorkOffset, GlobalWorkScale, GlobalWorkSize, LocalWorkSize);
    if (Arg->uniform == gcvNULL)
    {
        gcmFOOTER_ARG("%d", gcvSTATUS_OK);
        return gcvSTATUS_OK;
    }

    gcmONERROR(gcoVX_GetHWConfigGpuCount(&gpuCount));
    gcmONERROR(gcUNIFORM_GetType(Arg->uniform, &type, &length));

    gcmONERROR(gcUNIFORM_GetFormat(Arg->uniform, &format, &isPointer));

    gcmONERROR(gcUNIFORM_GetFlags(Arg->uniform, &flags));

    if (isUniformKernelArg(Arg->uniform) ||
        isUniformKernelArgConstant(Arg->uniform))
    {
        if (isPointer)
        {
            if ((type == gcSHADER_IMAGE_2D_T) && (Arg->data != gcvNULL))
            {
                vx_reference ref = *(vx_reference*) Arg->data;
                vx_context base = vxoContext_GetFromReference(ref);

                if (ref && ref->type == VX_TYPE_IMAGE)
                {
                    gcsVX_IMAGE_INFO     info = {0};

                    gcoVX_Kernel_Context context;
                    INITIALIZE_STRUCT(context);

#if gcdVX_OPTIMIZER
                    context.borders = BorderMode->mode;
#else
                    context.params.borders = BorderMode->mode;
#endif
                    gcmONERROR(gcfVX_GetImageInfo(&context, (vx_image)ref, &info, 0));

                    info.isVXC =  base->evisNoInst.supportEVIS ? gcvTRUE : gcvFALSE;

                    gcmONERROR(gcfVX_SetUniformImageInfo(Arg->uniform, &info));
                }
                else if(ref && ref->type == VX_TYPE_TENSOR)
                {
                    vx_tensor tensor = (vx_tensor) ref;
                    gcsVX_IMAGE_INFO     info = {0};

                    if (Arg->noBatch)
                    {
                        gcmONERROR(gcfVX_GetImageInfoFromTensor(BorderMode->mode, tensor, 0, &info));
                    }
                    else
                    {
                        gcmONERROR(gcfVX_GetImageInfoFromTensor(BorderMode->mode, tensor, batchID, &info));
                    }

                    if (Arg->components > 1 && Arg->components <= 4 && base->evisNoInst.supportEVIS == gcvFALSE)
                    {
                        info.componentCount = Arg->components;
                    }

                    info.isVXC =  base->evisNoInst.supportEVIS ? gcvTRUE : gcvFALSE;

                    gcmONERROR(gcfVX_SetUniformImageInfo(Arg->uniform, &info));
                }
                else
                {
                    gcmONERROR(gcfVX_SetUniformValue(Arg->uniform,
                                                     length,
                                                     (gctINT*)Arg->data));
                }
            }
            else if ((type == gcSHADER_IMAGE_2D_ARRAY_T) && (Arg->data != gcvNULL))
            {
                vx_reference ref = *(vx_reference*) Arg->data;
                vx_context base = vxoContext_GetFromReference(ref);

                if (ref && ref->type == VX_TYPE_OBJECT_ARRAY)
                {
                    vx_object_array imageArray = (vx_object_array) ref;

                    gcsVX_IMAGE_INFO     info = {0};
                    gcoVX_Kernel_Context context;

                    vx_image_s image =  *(vx_image)imageArray->itemsTable[0];

                    INITIALIZE_STRUCT(context);

                    /* warp to an imageArray/image3D */
                    image.arraySize = (gctUINT32)imageArray->itemCount;


#if gcdVX_OPTIMIZER
                    context.borders = BorderMode->mode;
#else
                    context.params.borders = BorderMode->mode;
#endif

                    gcmONERROR(gcfVX_GetImageInfo(&context, (vx_image)&image, &info, 0));

                    info.isVXC = base->evisNoInst.supportEVIS ? gcvTRUE : gcvFALSE;

                    gcmONERROR(gcfVX_SetUniformImageInfo(Arg->uniform, &info));
                }
                else if (ref && ref->type == VX_TYPE_TENSOR)
                {
                    vx_tensor tensor = (vx_tensor) ref;
                    gcsVX_IMAGE_INFO     info = {0};

                    if (Arg->noBatch)
                    {
                        gcmONERROR(gcfVX_GetImageInfoFromTensor(BorderMode->mode, tensor, 0, &info));
                    }
                    else
                    {
                        gcmONERROR(gcfVX_GetImageInfoFromTensor(BorderMode->mode, tensor, batchID, &info));
                    }

                    if (Arg->components > 1 && Arg->components <= 4 && base->evisNoInst.supportEVIS == gcvFALSE)
                    {
                        info.componentCount = Arg->components;
                    }

                    info.isVXC = base->evisNoInst.supportEVIS ? gcvTRUE : gcvFALSE;

                    gcmONERROR(gcfVX_SetUniformImageInfo(Arg->uniform, &info));
                }
                else
                {
                    gcmONERROR(gcfVX_SetUniformValue(Arg->uniform,
                                                     length,
                                                     (gctINT*)Arg->data));
                }
            }
            else
            {
                vx_reference ref = *(vx_reference*) Arg->data;

                if(Arg->isVivArray == vx_true_e && ref && vxoReference_IsValidAndSpecific(ref, VX_TYPE_ARRAY))
                {
                    vx_array arrayValue = (vx_array)ref;
                    gctUINT32 address = arrayValue->memory.physicals[0] + batchID * arrayValue->memory.strides[0][1];

                    gcmONERROR(gcfVX_SetUniformValue(Arg->uniform,
                                                    length,
                                                    (gctINT*)&address));

                }
                else
                {
                    gcmONERROR(gcfVX_SetUniformValue(Arg->uniform,
                                                    length,
                                                    (gctINT*)Arg->data));
                }
            }
        }
        else
        {
            gctUINT arraySize;
            gctUINT32 physicalAddress, numCols, numRows;
            gctUINT8_PTR pData = (gctUINT8_PTR)Arg->data;
            switch (type)
            {
            case gcSHADER_FLOAT_X1:
            case gcSHADER_FLOAT_X2:
            case gcSHADER_FLOAT_X3:
            case gcSHADER_FLOAT_X4:
            case gcSHADER_FLOAT_2X2:
            case gcSHADER_FLOAT_3X3:
            case gcSHADER_FLOAT_4X4:
                gcmONERROR(gcfVX_SetUniformValue(Arg->uniform,
                                                 length,
                                                 Arg->data));
                break;

            case gcSHADER_BOOLEAN_X1:
            case gcSHADER_BOOLEAN_X2:
            case gcSHADER_BOOLEAN_X3:
            case gcSHADER_BOOLEAN_X4:
            case gcSHADER_INTEGER_X1:
            case gcSHADER_INTEGER_X2:
            case gcSHADER_INTEGER_X3:
            case gcSHADER_INTEGER_X4:
            case gcSHADER_INT8_X1:
            case gcSHADER_UINT8_X1:
            case gcSHADER_UINT_X1:
            case gcSHADER_UINT_X2:
            case gcSHADER_UINT_X3:
            case gcSHADER_UINT_X4:

                switch (format)
                {
                case gcSL_INT8:
                case gcSL_UINT8:
                case gcSL_INT16:
                case gcSL_UINT16:
                    {
                        /* Unpack argument data into 4B chunks */
                        gctUINT32 *data= gcvNULL;
                        gctUINT32 signMask, signExt;
                        gctPOINTER pointer= gcvNULL;
                        gctSIZE_T i, elemSize, numElem, bytes;

                        elemSize =  ((format == gcSL_INT8)   ? 1 :
                                     (format == gcSL_UINT8)  ? 1 :
                                     (format == gcSL_INT16)  ? 2 :
                                     (format == gcSL_UINT16) ? 2 : 0);

                        signMask =  ((format == gcSL_INT8)   ? 0x80 :
                                     (format == gcSL_INT16)  ? 0x8000 : 0);

                        signExt =   ((format == gcSL_INT8)   ? 0xFFFFFF00 :
                                     (format == gcSL_INT16)  ? 0xFFFF0000 : 0);

                        numElem = Arg->size / elemSize;
                        bytes = numElem * 4;

                        gcmONERROR(gcoOS_Allocate(gcvNULL, bytes, &pointer));
                        gcoOS_ZeroMemory(pointer, bytes);
                        data = (gctUINT32 *) pointer;
                        for (i=0; i<numElem; i++) {
                            pointer = (gctPOINTER)(((gctUINTPTR_T)Arg->data)+(i*elemSize));
                            gcoOS_MemCopy(&data[i], pointer, elemSize);
                            if (data[i]&signMask) {
                                data[i] |= signExt;
                            }
                        }
                        pointer = data;
                        status = gcfVX_SetUniformValue(Arg->uniform, length, (gctINT*)pointer);
                        gcoOS_Free(gcvNULL, data);
                        if (gcmIS_ERROR(status)) goto OnError;

                    }
                    break;

                default:
                    gcmONERROR(gcfVX_SetUniformValue(Arg->uniform, length, (gctINT*)Arg->data));
                    break;
                }
                break;

            case gcSHADER_INT64_X1:
            case gcSHADER_INT64_X2:
            case gcSHADER_INT64_X3:
            case gcSHADER_INT64_X4:
            case gcSHADER_UINT64_X1:
            case gcSHADER_UINT64_X2:
            case gcSHADER_UINT64_X3:
            case gcSHADER_UINT64_X4:
                gcmONERROR(gcSHADER_ComputeUniformPhysicalAddress(Kernel->states.programState.hints->hwConstRegBases,
                                                                   Arg->uniform,
                                                                   &physicalAddress));

                gcTYPE_GetTypeInfo(GetUniformType(Arg->uniform), &numCols, &numRows, gcvNULL);

                arraySize = Arg->uniform->arraySize;

                gcmONERROR(gcoSHADER_BindUniform(gcvNULL, physicalAddress, GetUniformPhysical(Arg->uniform),
                                                        numCols, numRows, arraySize, gcvTRUE,
                                                        sizeof(gctINT64),
                                                        4*sizeof(gctINT64),
                                                        pData, gcvUNIFORMCVT_NONE, GetUniformShaderKind(Arg->uniform)));
                break;

            case gcSHADER_IMAGE_2D_T:
            case gcSHADER_IMAGE_3D_T:
                {
                vx_image image = *(vx_image*) Arg->data;

                gcsVX_IMAGE_INFO     info = {0};

                gcoVX_Kernel_Context context;
                INITIALIZE_STRUCT(context);

#if gcdVX_OPTIMIZER
                context.borders = VX_BORDER_UNDEFINED;
#else
                context.params.borders = BorderMode->mode;
#endif

                gcmONERROR(gcfVX_GetImageInfo(&context, (vx_image)image, &info, 1));

                info.isVXC = gcvTRUE;

                gcmONERROR(gcfVX_SetUniformImageInfo(Arg->uniform, &info));

                }
                break;

            case gcSHADER_SAMPLER_T:
                {
                    status = gcvSTATUS_INVALID_ARGUMENT;
                    goto OnError;
                }
                break;

            default:
                break;
            }
        }
    }
    else if (isUniformWorkDim(Arg->uniform))
    {
        gcoOS_MemCopy(Arg->data, &WorkDim, gcmSIZEOF(WorkDim));

        gcmONERROR(gcfVX_SetUniformValue(Arg->uniform,
                                         length,
                                         (gctINT*)Arg->data));
    }
    else if (isUniformGlobalSize(Arg->uniform))
    {
        /* TODO - For 64-bit GPU. */
        /*gcoOS_MemCopy(Arg->data, GlobalWorkSize, gcmSIZEOF(size_t) * 3);*/
        gctUINT32 globalWorkSize[3];

        globalWorkSize[0] = (gctUINT32)GlobalWorkSize[0];
        globalWorkSize[1] = (gctUINT32)GlobalWorkSize[1];
        globalWorkSize[2] = (gctUINT32)GlobalWorkSize[2];
        gcoOS_MemCopy(Arg->data, globalWorkSize, gcmSIZEOF(gctUINT32) * 3);

        gcmONERROR(gcfVX_SetUniformValue(Arg->uniform,
                                         length,
                                         (gctINT*)Arg->data));
    }
    else if (isUniformLocalSize(Arg->uniform))
    {
        /* TODO - For 64-bit GPU. */
        /*gcoOS_MemCopy(Arg->data, LocalWorkSize, gcmSIZEOF(size_t) * 3);*/
        gctUINT32 localWorkSize[3];

        localWorkSize[0] = (gctUINT32)(LocalWorkSize[0]);
        localWorkSize[1] = (gctUINT32)(LocalWorkSize[1]);
        localWorkSize[2] = (gctUINT32)(LocalWorkSize[2]);
        gcoOS_MemCopy(Arg->data, localWorkSize, gcmSIZEOF(gctUINT32) * 3);

        gcmONERROR(gcfVX_SetUniformValue(Arg->uniform,
                                         length,
                                         (gctINT*)Arg->data));
    }
    else if (isUniformGlobalWorkScale(Arg->uniform))
    {
        /* TODO - For 64-bit GPU. */
        /*gcoOS_MemCopy(Arg->data, GlobalWorkScale, gcmSIZEOF(size_t) * 3);*/
        gctUINT32 globalWorkScale[3];

        globalWorkScale[0] = (gctUINT32)GlobalWorkScale[0];
        globalWorkScale[1] = (gctUINT32)GlobalWorkScale[1];
        globalWorkScale[2] = (gctUINT32)GlobalWorkScale[2];
        gcoOS_MemCopy(Arg->data, globalWorkScale, gcmSIZEOF(gctUINT32) * 3);

        gcmONERROR(gcfVX_SetUniformValue(Arg->uniform,
                                         length,
                                         (gctINT*)Arg->data));
    }
    else if (isUniformNumGroups(Arg->uniform))
    {
        /* TODO - For 64-bit GPU. */
        /*size_t numGroups[3];*/
        gctUINT32 numGroups[3];

        numGroups[0] = (gctUINT32)(GlobalWorkSize[0] / (LocalWorkSize[0] ? LocalWorkSize[0] : 1));
        numGroups[1] = (gctUINT32)(WorkDim > 1 ? (GlobalWorkSize[1] / (LocalWorkSize[1] ? LocalWorkSize[1] : 1)) : 0);
        numGroups[2] = (gctUINT32)(WorkDim > 2 ? (GlobalWorkSize[2] / (LocalWorkSize[2] ? LocalWorkSize[2] : 1)) : 0);

        gcoOS_MemCopy(Arg->data, numGroups, gcmSIZEOF(numGroups));

        gcmONERROR(gcfVX_SetUniformValue(Arg->uniform,
                                         length,
                                         (gctINT*)Arg->data));

    }
    else if (isUniformNumGroupsForSingleGPU(Arg->uniform))
    {
        gctUINT32 usedGpu;
        gctUINT i = 0, j = 0, maxDimIndex = 0;
        gctUINT eachGPUWorkGroupSizes[gcdMAX_3DGPU_COUNT] = { 0 };
        gctUINT eachGPUWorkGroupNum[gcdMAX_3DGPU_COUNT][3] = { {0} };
        gctUINT eachGPUGroupCount, restGroupCount;
        gctINT *datas[4] = {gcvNULL};
        gctUINT maxWorkGroupCount = (gctUINT) (GlobalWorkSize[0] / LocalWorkSize[0]);
        /* TODO - For 64-bit GPU. */
        /*size_t numGroups[3];*/
        gctUINT32 numGroups[3]    = { 0 };

        numGroups[0] = (gctUINT32)(GlobalWorkSize[0] / (LocalWorkSize[0] ? LocalWorkSize[0] : 1));
        numGroups[1] = (gctUINT32)(WorkDim > 1 ? (GlobalWorkSize[1] / (LocalWorkSize[1] ? LocalWorkSize[1] : 1)) : 0);
        numGroups[2] = (gctUINT32)(WorkDim > 2 ? (GlobalWorkSize[2] / (LocalWorkSize[2] ? LocalWorkSize[2] : 1)) : 0);

        usedGpu = gpuCount;

        if (gpuCount > 1)
        {
            for(i = 1; i < WorkDim; i++)  /*The split method shoud match with gcoHardware_InvokThreadWalker  */
            {
                if(GlobalWorkSize[i] / LocalWorkSize[i] > maxWorkGroupCount)
                {
                    maxWorkGroupCount = (gctUINT) (GlobalWorkSize[i] / LocalWorkSize[i]);
                    maxDimIndex = i;
                }
            }

            eachGPUGroupCount = maxWorkGroupCount / gpuCount;
            restGroupCount = maxWorkGroupCount % gpuCount;

            for(i = 0 ;i < gpuCount; i++)
            {
                eachGPUWorkGroupSizes[i] = eachGPUGroupCount;
            }

            for(i = 0 ;i <restGroupCount; i++)
            {
                eachGPUWorkGroupSizes[i]++;
            }

            if(eachGPUGroupCount == 0) usedGpu = restGroupCount;

            for (i = 0; i < WorkDim; i++)
            {
                if (i == maxDimIndex)
                {
                    for (j = 0; j < usedGpu; j++)
                    {
                        eachGPUWorkGroupNum[j][i] = eachGPUWorkGroupSizes[j];
                    }
                }
                else
                {
                    for (j = 0; j < usedGpu; j++)
                    {
                        eachGPUWorkGroupNum[j][i] = numGroups[i];
                    }
                }
            }

            for(i = 0; i < gpuCount; i++)
            {
                datas[i] = (gctINT *) eachGPUWorkGroupNum[i];
            }

            gcmONERROR(gcfVX_SetUniformValueCombinedMode(Arg->uniform,
                                                      length,
                                                      datas,
                                                      gpuCount
                                                    ));
        }
        else
        {
            gcoOS_MemCopy(Arg->data, numGroups, gcmSIZEOF(numGroups));

            gcmONERROR(gcfVX_SetUniformValue(Arg->uniform,
                                             length,
                                             (gctINT*)Arg->data));
        }
    }
    else if (isUniformGlobalOffset(Arg->uniform))
    {
        /* TODO - For 64-bit GPU. */
        /*gcoOS_MemCopy(Arg->data, GlobalWorkOffset, gcmSIZEOF(size_t) * 3);*/
        gctUINT32 glocalWorkOffset[3];

        glocalWorkOffset[0] = (gctUINT32)(GlobalWorkOffset[0]);
        glocalWorkOffset[1] = (gctUINT32)(GlobalWorkOffset[1]);
        glocalWorkOffset[2] = (gctUINT32)(GlobalWorkOffset[2]);
        gcoOS_MemCopy(Arg->data, glocalWorkOffset, gcmSIZEOF(gctUINT32) * 3);

        gcmONERROR(gcfVX_SetUniformValue(Arg->uniform,
                                         length,
                                         (gctINT*)Arg->data));
    }
    else if (isUniformWorkGroupCount(Arg->uniform))
    {
        gctUINT16 workGroupCount = 0;

        /*
        ** If workGroupCount is chosen, set the value to workGroupCount;
        ** otherwise use 0 because (i MOD 0) == i.
        */
        if (Kernel->states.programState.hints->workGroupCount != 0 &&
            (gctINT)Kernel->states.programState.hints->workGroupCount < totalNumGroups)
        {
            workGroupCount = Kernel->states.programState.hints->workGroupCount;
        }

        gcoOS_MemCopy(Arg->data, &workGroupCount, gcmSIZEOF(gctUINT16));
        gcmONERROR(gcfVX_SetUniformValue(Arg->uniform,
                                         length,
                                         (gctINT*)Arg->data));
    }
    else if (isUniformWorkThreadCount(Arg->uniform))
    {
        gctUINT16 workThreadCount = 0;

        /*
        ** If workThreadCount is chosen, set the value to workThreadCount;
        ** otherwise use 0 because (i MOD 0) == i.
        */
        if (Kernel->states.programState.hints->workThreadCount != 0 &&
            (gctINT)Kernel->states.programState.hints->workThreadCount < totalNumItems)
        {
            workThreadCount = Kernel->states.programState.hints->workThreadCount;
        }

        gcoOS_MemCopy(Arg->data, &workThreadCount, gcmSIZEOF(gctUINT16));
        gcmONERROR(gcfVX_SetUniformValue(Arg->uniform,
                                         length,
                                         (gctINT*)Arg->data));
    }
    else if (isUniformLocalAddressSpace(Arg->uniform))
    {
        vx_mem_alloc_info memAllocInfo = (vx_mem_alloc_info) Arg->data;
        gctUINT           allocateSize = (gctUINT)Kernel->localMemSize;

        /* Size may be zero if declared but never used */
        if (allocateSize > 0)
        {
            gctINT * data;
            gctINT  workGroupCount = totalNumGroups;

            if (!Arg->isMemAlloc)
            {
                status = gcvSTATUS_INVALID_DATA;
                goto OnError;
            }

            gcmASSERT(!Kernel->states.programState.hints->sharedMemAllocByCompiler);


             if (Kernel->states.programState.hints->workGroupCount != 0 &&
                (gctINT)Kernel->states.programState.hints->workGroupCount < totalNumGroups)
            {
                workGroupCount = Kernel->states.programState.hints->workGroupCount;
            }

            allocateSize *= workGroupCount;
            perGpuMemSize = gcmALIGN(memAllocInfo->allocatedSize, 128); /*OCL base alinement is 128*/
            allocateSize *= gpuCount;

            if (memAllocInfo->node && (allocateSize > memAllocInfo->allocatedSize))
            {
                gcmONERROR(gcoVX_FreeMemory(memAllocInfo->node));

                gcoOS_ZeroMemory(memAllocInfo, sizeof(vx_mem_alloc_info_s));
            }

            if (!memAllocInfo->node)
            {
                memAllocInfo->allocatedSize = allocateSize;
                /* Allocate the physical buffer */
                gcmONERROR(gcoVX_AllocateMemory(memAllocInfo->allocatedSize,
                                                &memAllocInfo->logical,
                                                &memAllocInfo->physical,
                                                &memAllocInfo->node
                                                ));
            }

            data = (gctINT *) &memAllocInfo->physical;

            if(gpuCount == 1)
            {
                gcmONERROR(gcfVX_SetUniformValue(Arg->uniform, length, data));
            }
            else
            {
                gctUINT32 gpuPhysicals[gcdMAX_3DGPU_COUNT] = {0};
                gctINT * datas[gcdMAX_3DGPU_COUNT];
                gctUINT32 index = 0;

                gpuPhysicals[0] = memAllocInfo->physical;
                datas[0] = (gctINT *) &gpuPhysicals[0];

                for(index = 1; index < gpuCount; ++index)
                {
                    gpuPhysicals[index] = gpuPhysicals[index-1] + perGpuMemSize;
                    datas[index] = (gctINT *) &gpuPhysicals[index];
                }

                gcfVX_SetUniformValueCombinedMode(Arg->uniform, length, datas, gpuCount);
            }
        }
    }
    else if (isUniformPrivateAddressSpace(Arg->uniform))
    {
        vx_mem_alloc_info memAllocInfo = (vx_mem_alloc_info) Arg->data;
        gctUINT           allocateSize = (gctUINT)Kernel->localMemSize;

        allocateSize = (gctUINT)Kernel->privateMemSize;

        /* Size may be zero if declared but never used */
        if (allocateSize > 0)
        {
            gctINT * data;

            if (!Arg->isMemAlloc)
            {
                status = gcvSTATUS_INVALID_DATA;
                goto OnError;
            }

            /* Compare the workThreadCount with total item number and choose the smaller one. */
            if (Kernel->states.programState.hints->workThreadCount != 0 &&
                (gctINT)Kernel->states.programState.hints->workThreadCount < totalNumItems)
            {
                allocateSize *= Kernel->states.programState.hints->workThreadCount;
            }
            else
            {
                allocateSize *= totalNumItems;
            }

            perGpuMemSize = gcmALIGN(allocateSize, 128);
            allocateSize = perGpuMemSize * gpuCount;

            if (memAllocInfo->node && (allocateSize > memAllocInfo->allocatedSize))
            {
                gcmONERROR(gcoVX_FreeMemory(memAllocInfo->node));

                gcoOS_ZeroMemory(memAllocInfo, sizeof(vx_mem_alloc_info_s));
            }

            if (!memAllocInfo->node)
            {
                memAllocInfo->allocatedSize = allocateSize;

                /* Allocate the physical buffer */
                gcmONERROR(gcoVX_AllocateMemory(memAllocInfo->allocatedSize,
                                                &memAllocInfo->logical,
                                                &memAllocInfo->physical,
                                                &memAllocInfo->node)
                                                );
            }

            data = (gctINT *) &memAllocInfo->physical;

            if(gpuCount == 1)
            {
                gcmONERROR(gcfVX_SetUniformValue(Arg->uniform, length, data));
            }
            else
            {
                gctUINT32 gpuPhysicals[gcdMAX_3DGPU_COUNT] = {0};
                gctINT * datas[gcdMAX_3DGPU_COUNT];
                gctUINT32 index = 0;

                gpuPhysicals[0] = memAllocInfo->physical;
                datas[0] = (gctINT *) &gpuPhysicals[0];

                for(index = 1; index < gpuCount; ++index)
                {
                    gpuPhysicals[index] = gpuPhysicals[index-1] + perGpuMemSize;
                    datas[index] = (gctINT *) &gpuPhysicals[index];
                }

                gcmONERROR(gcfVX_SetUniformValueCombinedMode(Arg->uniform, length, datas, gpuCount));
            }
        }
    }
    else if(isUniformTempRegSpillAddress(Arg->uniform))
    {
        vx_mem_alloc_info memAllocInfo = (vx_mem_alloc_info) Arg->data;

        if (memAllocInfo->allocatedSize > 0)
        {
            gctINT* datas[gcdMAX_3DGPU_COUNT];
            gctUINT32 physicals[gcdMAX_3DGPU_COUNT];
            gctUINT32 i;
            gctUINT32 physicalAddress;

            memAllocInfo->allocatedSize = gcmALIGN(memAllocInfo->allocatedSize, 64);
            perGpuMemSize =  memAllocInfo->allocatedSize;
            memAllocInfo->allocatedSize *= gpuCount;

            gcmONERROR(gcoVX_AllocateMemory(memAllocInfo->allocatedSize,
                                            &memAllocInfo->logical,
                                            &memAllocInfo->physical,
                                            &memAllocInfo->node
                                            ));

            physicals[0] = memAllocInfo->physical;
            datas[0] = (gctINT *) &physicals[0];

            for(i = 1; i < gpuCount; i++)
            {
                physicals[i] = physicals[i-1] + perGpuMemSize;
                datas[i] = (gctINT *) &physicals[i];
            }

            gcmONERROR(gcSHADER_ComputeUniformPhysicalAddress(Kernel->states.programState.hints->hwConstRegBases,
                                                              Arg->uniform,
                                                              &physicalAddress
                                                              ));

            gcmONERROR(gcfVX_SetUniformValueCombinedMode(Arg->uniform, length, datas, gpuCount));
        }

    }
    else if(isUniformWorkGroupIdOffset(Arg->uniform))
    {
        gctUINT32 usedGpu;
        gctUINT i = 0, maxDimIndex = 0;
        gctUINT eachGPUWorkGroupSizes[gcdMAX_3DGPU_COUNT] = { 0 };
        gctUINT eachGPUWorkGroupIDOffsets[gcdMAX_3DGPU_COUNT][3] = { {0} };
        gctUINT eachGPUGroupCount, restGroupCount;
        gctINT *datas[4] = {gcvNULL};
        gctUINT maxWorkGroupCount = (gctUINT) (GlobalWorkSize[0] / LocalWorkSize[0]);

        usedGpu = gpuCount;

        gcmASSERT(gpuCount > 1) ; /* We can assure it 's combined mode */

        for(i = 1; i < WorkDim; i++)  /*The split method shoud match with gcoHardware_InvokThreadWalker  */
        {
            if(GlobalWorkSize[i] / LocalWorkSize[i] > maxWorkGroupCount)
            {
                maxWorkGroupCount = (gctUINT) (GlobalWorkSize[i] / LocalWorkSize[i]);
                maxDimIndex = i;
            }
        }

        eachGPUGroupCount = maxWorkGroupCount / gpuCount;
        restGroupCount = maxWorkGroupCount % gpuCount;

        for(i = 0 ;i < gpuCount; i++)
        {
            eachGPUWorkGroupSizes[i] = eachGPUGroupCount;
        }

        for(i = 0 ;i <restGroupCount; i++)
        {
            eachGPUWorkGroupSizes[i]++;
        }

        if(eachGPUGroupCount == 0) usedGpu = restGroupCount;

        eachGPUWorkGroupIDOffsets[0][maxDimIndex] = 0;

        for(i = 1; i < usedGpu; i++)
        {
            eachGPUWorkGroupIDOffsets[i][maxDimIndex] = eachGPUWorkGroupSizes[i - 1] +  eachGPUWorkGroupIDOffsets[i-1][maxDimIndex];

        }

        for(i = 0; i < gpuCount; i++)
        {
            datas[i] = (gctINT *) eachGPUWorkGroupIDOffsets[i];
        }

        gcmONERROR(gcfVX_SetUniformValueCombinedMode(Arg->uniform,
                                                  length,
                                                  datas,
                                                  gpuCount
                                                  ));

    }
    else if (isUniformConstantAddressSpace(Arg->uniform) && (GetUniformPhysical(Arg->uniform) != -1))
    {
        vx_mem_alloc_info memAllocInfo = (vx_mem_alloc_info) Arg->data;
        gctINT * data;

        if (!Arg->isMemAlloc)
        {
            status = gcvSTATUS_INVALID_DATA;
            goto OnError;
        }

        if (memAllocInfo->allocatedSize <= 0)
        {
            status = gcvSTATUS_INVALID_DATA;
            goto OnError;
        }

        if (!memAllocInfo->node)
        {
            /* Allocate the physical buffer */
            gcmONERROR(gcoVX_AllocateMemory(memAllocInfo->allocatedSize,
                                            &memAllocInfo->logical,
                                            &memAllocInfo->physical,
                                            &memAllocInfo->node
                                            ));
        }

        data = (gctINT *) &memAllocInfo->physical;
        gcmONERROR(gcfVX_SetUniformValue(Arg->uniform, length, data));

        /* Copy the constant data to the buffer */
        gcoOS_MemCopy(memAllocInfo->logical, Kernel->constantMemBuffer, Kernel->constantMemSize);

        gcmDUMP(gcvNULL, "#[info: constant memory");

        gcmDUMP_BUFFER(gcvNULL,
                       gcvDUMP_BUFFER_MEMORY,
                       gcmPTR2SIZE(memAllocInfo->physical),
                       memAllocInfo->logical,
                       0,
                       Kernel->constantMemSize);

    }
    else if (isUniformPrintfAddress(Arg->uniform))
    {
        vx_mem_alloc_info memAllocInfo = (vx_mem_alloc_info) Arg->data;
        gctINT * data;

        if (!Arg->isMemAlloc)
        {
            status = gcvSTATUS_INVALID_DATA;
            goto OnError;
        }

        if (!memAllocInfo->node)
        {
            memAllocInfo->allocatedSize = VX_MAX_PRINTF_BUFFER_SIZE;
            /* Allocate the physical buffer */
            gcmONERROR(gcoVX_AllocateMemory(memAllocInfo->allocatedSize,
                                            &memAllocInfo->logical,
                                            &memAllocInfo->physical,
                                            &memAllocInfo->node
                                            ));
        }

        data = (gctINT *) &memAllocInfo->physical;
        gcmONERROR(gcfVX_SetUniformValue(Arg->uniform, length, data));

    }
    else if (isUniformWorkItemPrintfBufferSize(Arg->uniform))
    {
        gctINT totalNumItems  = (gctINT)(GlobalWorkSize[0]
                                  * (WorkDim > 1 ? GlobalWorkSize[1] : 1)
                                  * (WorkDim > 2 ? GlobalWorkSize[2] : 1));

        gctINT printBufferSize = VX_MAX_PRINTF_BUFFER_SIZE / totalNumItems;
        gcoOS_MemCopy(Arg->data, &printBufferSize, gcmSIZEOF(printBufferSize));

        gcmONERROR(gcfVX_SetUniformValue(Arg->uniform,
                                         length,
                                         (gctINT*)Arg->data));
    }
    else if (isUniformKernelArgSampler(Arg->uniform))
    {
        status = gcvSTATUS_INVALID_ARGUMENT;
        goto OnError;

    }
    else if (isUniformKernelArgLocal(Arg->uniform) ||
                isUniformKernelArgLocalMemSize(Arg->uniform))
    {
        status = gcvSTATUS_INVALID_ARGUMENT;
        goto OnError;
    }
    else if (isUniformKernelArgPrivate(Arg->uniform))
    {
        status = gcvSTATUS_INVALID_ARGUMENT;
        goto OnError;
    }
    else if (isUniformKernelArgPatch(Arg->uniform))
    {
        switch (type)
        {
        case gcSHADER_FLOAT_X1:
        case gcSHADER_FLOAT_X2:
        case gcSHADER_FLOAT_X3:
        case gcSHADER_FLOAT_X4:
        case gcSHADER_FLOAT_2X2:
        case gcSHADER_FLOAT_3X3:
        case gcSHADER_FLOAT_4X4:
            gcmONERROR(gcfVX_SetUniformValue(Arg->uniform,
                                             length,
                                             Arg->data));
            break;

        case gcSHADER_BOOLEAN_X1:
        case gcSHADER_BOOLEAN_X2:
        case gcSHADER_BOOLEAN_X3:
        case gcSHADER_BOOLEAN_X4:
        case gcSHADER_INTEGER_X1:
        case gcSHADER_INTEGER_X2:
        case gcSHADER_INTEGER_X3:
        case gcSHADER_INTEGER_X4:
        case gcSHADER_UINT_X1:
        case gcSHADER_UINT_X2:
        case gcSHADER_UINT_X3:
        case gcSHADER_UINT_X4:

            switch (format)
            {
            case gcSL_INT8:
            case gcSL_UINT8:
            case gcSL_INT16:
            case gcSL_UINT16:
                {
                    /* Unpack argument data into 4B chunks */
                    gctUINT32 *data= gcvNULL;
                    gctUINT32 signMask, signExt;
                    gctPOINTER pointer= gcvNULL;
                    gctSIZE_T i, elemSize, numElem, bytes;

                    elemSize =  ((format == gcSL_INT8)   ? 1 :
                                 (format == gcSL_UINT8)  ? 1 :
                                 (format == gcSL_INT16)  ? 2 :
                                 (format == gcSL_UINT16) ? 2 : 0);

                    signMask =  ((format == gcSL_INT8)   ? 0x80 :
                                 (format == gcSL_INT16)  ? 0x8000 : 0);

                    signExt =   ((format == gcSL_INT8)   ? 0xFFFFFF00 :
                                 (format == gcSL_INT16)  ? 0xFFFF0000 : 0);

                    numElem = Arg->size / elemSize;
                    bytes = numElem * 4;

                    gcmONERROR(gcoOS_Allocate(gcvNULL, bytes, &pointer));
                    gcoOS_ZeroMemory(pointer, bytes);
                    data = (gctUINT32 *) pointer;
                    for (i=0; i<numElem; i++) {
                        pointer = (gctPOINTER)(((gctUINTPTR_T)Arg->data)+(i*elemSize));
                        gcoOS_MemCopy(&data[i], pointer, elemSize);
                        if (data[i]&signMask) {
                            data[i] |= signExt;
                        }
                    }
                    pointer = data;
                    status = gcfVX_SetUniformValue(Arg->uniform, length, (gctINT*)pointer);
                    gcoOS_Free(gcvNULL, data);
                    if (gcmIS_ERROR(status)) goto OnError;

                }
                break;

            default:
                gcmONERROR(gcfVX_SetUniformValue(Arg->uniform, length, (gctINT*)Arg->data));
                break;
            }
            break;

        default:
            break;
        }
    }
    else if (gcoOS_StrCmp(Arg->uniform->name, "$ConstBorderValue") == gcvSTATUS_OK)
    {
        gctUINT32   data[4] = {0};
        gctUINT32   data32 = (gctUINT32)BorderMode->constant_value.U32;

        gctUINT8  data8 = data32 & 0xFF;
        gctUINT16 data16 = data32 & 0xFFFF;
        gctFLOAT  dataf  = (gctFLOAT)(data32);

        data[0] = data8 | (data8 << 8) | (data8 << 16) | (data8 << 24);

        data[1] = data16 | (data16 << 16);

        data[2] = data32;

        data[3] = *(gctUINT32*)(&dataf);

        gcmONERROR(gcfVX_SetUniformValue(Arg->uniform, length, (gctINT*)data));
    }
    else
    {
        gcsUNIFORM_BLOCK uniformBlock = gcvNULL;
        gcUNIFORM bUniform = gcvNULL;
        gctINT16   blockIndex = GetUniformBlockID(Arg->uniform);
        gctUINT32 physicalAddress = 0;
        gctUINT      entries, arraySize;
        gctUINT32    numCols = 0, numRows = 0;
        if ((blockIndex >= 0) && (GetUniformPhysical(Arg->uniform) != -1))
        {
            gcmONERROR(gcSHADER_GetUniformBlock(Shader, blockIndex, &uniformBlock));
            gcmONERROR(gcSHADER_GetUniform(Shader, GetUBIndex(uniformBlock), &bUniform));
            if(isUniformConstantAddressSpace(bUniform))
            {
                gctUINT8_PTR pData = (gctUINT8_PTR)(Kernel->constantMemBuffer) + GetUniformOffset(Arg->uniform);

                gcmONERROR(gcSHADER_ComputeUniformPhysicalAddress(Kernel->states.programState.hints->hwConstRegBases,
                                                                   Arg->uniform,
                                                                   &physicalAddress));

                gcTYPE_GetTypeInfo(GetUniformType(Arg->uniform), &numCols, &numRows, gcvNULL);

                entries = gcfVX_GetUniformArrayInfo(Arg->uniform, gcvNULL, gcvNULL, &arraySize);
                arraySize *= entries;   /* Expand array size for array of arrays to one dimension */

                gcmONERROR(gcoSHADER_BindUniform(gcvNULL, physicalAddress, GetUniformAddress(Arg->uniform),
                                                 numCols, numRows, arraySize, gcvFALSE,
                                                 (gctSIZE_T)GetUniformMatrixStride(Arg->uniform),
                                                 (gctSIZE_T)GetUniformArrayStride(Arg->uniform),
                                                  pData, gcvUNIFORMCVT_NONE, GetUniformShaderKind(Arg->uniform)));
            }
        }
        else
        {
            gctUINT arraySize;
            gctUINT32 physicalAddress, numCols, numRows;
            gctUINT8_PTR pData = (gctUINT8_PTR)Arg->data;
            switch (type)
            {
            case gcSHADER_FLOAT_X1:
            case gcSHADER_FLOAT_X2:
            case gcSHADER_FLOAT_X3:
            case gcSHADER_FLOAT_X4:
            case gcSHADER_FLOAT_2X2:
            case gcSHADER_FLOAT_3X3:
            case gcSHADER_FLOAT_4X4:
                gcmONERROR(gcfVX_SetUniformValue(Arg->uniform,
                                                 length,
                                                 Arg->data));
                break;

            case gcSHADER_BOOLEAN_X1:
            case gcSHADER_BOOLEAN_X2:
            case gcSHADER_BOOLEAN_X3:
            case gcSHADER_BOOLEAN_X4:
            case gcSHADER_INTEGER_X1:
            case gcSHADER_INTEGER_X2:
            case gcSHADER_INTEGER_X3:
            case gcSHADER_INTEGER_X4:
            case gcSHADER_UINT_X1:
            case gcSHADER_UINT_X2:
            case gcSHADER_UINT_X3:
            case gcSHADER_UINT_X4:
            case gcSHADER_UINT_X16:

                switch (format)
                {
                case gcSL_INT8:
                case gcSL_UINT8:
                case gcSL_INT16:
                case gcSL_UINT16:
                    {
                        /* Unpack argument data into 4B chunks */
                        gctUINT32 *data= gcvNULL;
                        gctUINT32 signMask, signExt;
                        gctPOINTER pointer= gcvNULL;
                        gctSIZE_T i, elemSize, numElem, bytes;

                        elemSize =  ((format == gcSL_INT8)   ? 1 :
                                     (format == gcSL_UINT8)  ? 1 :
                                     (format == gcSL_INT16)  ? 2 :
                                     (format == gcSL_UINT16) ? 2 : 0);

                        signMask =  ((format == gcSL_INT8)   ? 0x80 :
                                     (format == gcSL_INT16)  ? 0x8000 : 0);

                        signExt =   ((format == gcSL_INT8)   ? 0xFFFFFF00 :
                                     (format == gcSL_INT16)  ? 0xFFFF0000 : 0);

                        numElem = Arg->size / elemSize;
                        bytes = numElem * 4;

                        gcmONERROR(gcoOS_Allocate(gcvNULL, bytes, &pointer));
                        gcoOS_ZeroMemory(pointer, bytes);
                        data = (gctUINT32 *) pointer;
                        for (i=0; i<numElem; i++) {
                            pointer = (gctPOINTER)(((gctUINTPTR_T)Arg->data)+(i*elemSize));
                            gcoOS_MemCopy(&data[i], pointer, elemSize);
                            if (data[i]&signMask) {
                                data[i] |= signExt;
                            }
                        }
                        pointer = data;
                        status = gcfVX_SetUniformValue(Arg->uniform, length, (gctINT*)pointer);
                        gcoOS_Free(gcvNULL, data);
                        if (gcmIS_ERROR(status)) goto OnError;

                    }
                    break;

                default:
                    gcmONERROR(gcfVX_SetUniformValue(Arg->uniform, length, (gctINT*)Arg->data));
                    break;
                }
                break;

            case gcSHADER_INT64_X1:
            case gcSHADER_INT64_X2:
            case gcSHADER_INT64_X3:
            case gcSHADER_INT64_X4:
            case gcSHADER_UINT64_X1:
            case gcSHADER_UINT64_X2:
            case gcSHADER_UINT64_X3:
            case gcSHADER_UINT64_X4:
                gcmONERROR(gcSHADER_ComputeUniformPhysicalAddress(Kernel->states.programState.hints->hwConstRegBases,
                                                                   Arg->uniform,
                                                                   &physicalAddress));

                gcTYPE_GetTypeInfo(GetUniformType(Arg->uniform), &numCols, &numRows, gcvNULL);

                arraySize = Arg->uniform->arraySize;

                gcmONERROR(gcoSHADER_BindUniform(gcvNULL, physicalAddress, GetUniformPhysical(Arg->uniform),
                                                 numCols, numRows, arraySize, gcvTRUE,
                                                 sizeof(gctINT64),
                                                 4*sizeof(gctINT64),
                                                 pData, gcvUNIFORMCVT_NONE, GetUniformShaderKind(Arg->uniform)));
                break;


            default:
                status = gcvSTATUS_INVALID_DATA;
                goto OnError;
            }
        }
    }

    status = gcvSTATUS_OK;

OnError:

    gcmFOOTER_ARG("%d", status);
    return status;
}

static vx_argument
gcfVX_GetKernelArg(
    vx_shader    Kernel,
    gctUINT             Index,
    gctBOOL *           isLocal,
    gctBOOL *           isPrivate,
    gctBOOL *           isSampler
    )
{
    vx_argument         arg;
    gctUINT             i, argIndex = 0;

    gcmHEADER_ARG("Kernel=%p, Index=0x%x, isLocal=%p, isPrivate=%p, isSampler=%p", Kernel, Index, isLocal, isPrivate, isSampler);

    for (i = 0; i < Kernel->numArgs; i++)
    {
        arg = &Kernel->args[i];
        if (arg->uniform == gcvNULL) continue;
        if (! hasUniformKernelArgKind(arg->uniform)) continue;
        if (argIndex == Index)
        {
            if (isLocal) *isLocal = isUniformKernelArgLocal(arg->uniform);
            if (isPrivate) *isPrivate = isUniformKernelArgPrivate(arg->uniform);
            if (isSampler) *isSampler = isUniformKernelArgSampler(arg->uniform);
            gcmFOOTER_ARG("arg=%p", arg);
            return arg;
        }
        argIndex++;
    }
    gcmFOOTER_NO();
    return gcvNULL;
}

gceSTATUS
gcfVX_SetKernelArg(
    vx_shader           Kernel,
    vx_uint32           ArgIndex,
    vx_uint32           ArgSize,
    const void *        ArgValue
    )
{
    vx_argument     argument;
    gceSTATUS       status;
    gctBOOL         isLocal, isPrivate, isSampler;
    gctBOOL         acquired = gcvFALSE;

    gcmHEADER_ARG("Kernel=%p, ArgIndex=0x%x, ArgSize=0x%x, ArgValue=%p", Kernel, ArgIndex, ArgSize, ArgValue);

    if (Kernel == gcvNULL)
    {
        status = gcvSTATUS_INVALID_ARGUMENT;
        goto OnError;
    }

    if (ArgIndex > Kernel->numArgs)
    {
        status = gcvSTATUS_INVALID_ARGUMENT;
        goto OnError;
    }

 //   gcmVERIFY_OK(gcoOS_AcquireMutex(gcvNULL, Kernel->argMutex, gcvINFINITE));
 //   acquired = gcvTRUE;

    argument = gcfVX_GetKernelArg(Kernel, ArgIndex, &isLocal, &isPrivate, &isSampler);

    if (argument == gcvNULL)
    {
        status = gcvSTATUS_INVALID_ARGUMENT;
        goto OnError;
    }

    if (isLocal)
    {
        status = gcvSTATUS_INVALID_ARGUMENT;
        goto OnError;
    }
    else if (isPrivate)
    {
        status = gcvSTATUS_INVALID_ARGUMENT;
        goto OnError;
    }
    else if (isSampler)
    {
        status = gcvSTATUS_INVALID_ARGUMENT;
        goto OnError;
    }
    else
    {
        if(ArgSize != argument->size)
        {
            status = gcvSTATUS_INVALID_ARGUMENT;
            goto OnError;
        }

        gcoOS_MemCopy(argument->data, ArgValue, ArgSize);
    }

    argument->set = gcvTRUE;


    status = gcvSTATUS_OK;

OnError:
    if (acquired)
    {
       // gcmVERIFY_OK(gcoOS_ReleaseMutex(gcvNULL, Kernel->argMutex));
    }
    gcmFOOTER_ARG("%d", status);
    return status;
}

gceSTATUS
gcfVX_AllocateKernelArgs(
    vx_shader   Kernel
    )
{
    gceSTATUS       status = gcvSTATUS_OK;
    gctPOINTER      pointer;
    gctUINT32       bytes;
    vx_argument     argument;
    gctUINT         i, uniformCount;
    gcSHADER        shader = (gcSHADER)Kernel->states.binary;
    gcUNIFORM       uniform;
    vx_mem_alloc_info memAllocInfo = gcvNULL;

    gcmHEADER_ARG("Kernel=%p", Kernel);

        /* Get the number of uniforms. */
    gcmONERROR(gcSHADER_GetUniformCount(
                        shader,
                        &uniformCount));


    if (uniformCount == 0)
    {
        Kernel->args = gcvNULL;
        status = gcvSTATUS_INVALID_ARGUMENT;
        goto OnError;
    }

    Kernel->numArgs = 0;

    for (i = 0; i < uniformCount; i++)
    {
        gcmONERROR(gcSHADER_GetUniform(
                        shader,
                        i, &uniform));

        if (!uniform || (uniform && isUniformCompiletimeInitialized(uniform)))
            continue;

        Kernel->numArgs++;
    }

    /* Allocate the array of arguments. */
    bytes = Kernel->numArgs * gcmSIZEOF(vx_argument_s);
    gcmONERROR(gcoOS_Allocate(gcvNULL, bytes, &pointer));
    gcoOS_ZeroMemory(pointer, bytes);

    Kernel->args = argument = pointer;
    for (i = 0; i < uniformCount; i++)
    {
        gcUNIFORM uniform;
        gcSHADER_TYPE type;
        gcSL_FORMAT format;
        gctBOOL isPointer;
        gctUINT length;
        gctUINT32 bytes;

        gcmONERROR(gcSHADER_GetUniform(
                        shader,
                        i, &uniform));

        if (!uniform || (uniform && isUniformCompiletimeInitialized(uniform))) continue;

        gcmONERROR(gcUNIFORM_GetType(uniform, &type, &length));
        gcmONERROR(gcUNIFORM_GetFormat(uniform, &format, &isPointer));

        if (isUniformLocalAddressSpace(uniform) ||
            isUniformPrivateAddressSpace(uniform) ||
            isUniformConstantAddressSpace(uniform) ||
            isUniformKernelArgPrivate(uniform) ||
            isUniformKernelArgLocal(uniform) ||
            isUniformKernelArgLocalMemSize(uniform) ||
            isUniformPrintfAddress(uniform) ||
            isUniformTempRegSpillAddress(uniform))
        {
            gctPOINTER pointer;

            bytes = sizeof(vx_mem_alloc_info_s);

            /* Allocate the memory allocation info. */
            gcmONERROR(gcoOS_Allocate(gcvNULL, bytes, &pointer));
            gcoOS_ZeroMemory(pointer, bytes);
            memAllocInfo = (vx_mem_alloc_info)pointer;

            /* Get the required memory size.
             * For local/private kernel arguments it will be set by the application.
             */
            if (isUniformLocalAddressSpace(uniform))
            {
                if ((strcmp(uniform->name, _sldLocalStorageAddressName) == 0 && gcShaderUseLocalMem((gcSHADER) Kernel->states.binary))
                    ||
                    Kernel->states.programState.hints->sharedMemAllocByCompiler)
                {
                    /* local memory is handled by compiler or HW directly, we don't need to allocate it. */
                }
                else
                {
                    gcmONERROR(gcSHADER_GetLocalMemorySize(shader, &memAllocInfo->allocatedSize));
                    Kernel->localMemSize += memAllocInfo->allocatedSize;
                }
            }
            else if (isUniformPrivateAddressSpace(uniform))
            {
                gcmONERROR(gcSHADER_GetPrivateMemorySize(shader, &memAllocInfo->allocatedSize));
                Kernel->privateMemSize += memAllocInfo->allocatedSize;
            }
            else if (isUniformConstantAddressSpace(uniform))
            {
                gcmONERROR(gcSHADER_GetConstantMemorySize(shader, &memAllocInfo->allocatedSize, &Kernel->constantMemBuffer ));
                Kernel->constantMemSize += memAllocInfo->allocatedSize;
            }
             else if(isUniformTempRegSpillAddress(uniform))
            {
                 gcsSTORAGE_BLOCK storageBlock;
                 gctINT16   blockIndex = GetUniformBlockID(uniform);
                 gcSHADER_GetStorageBlock((gcSHADER) shader, blockIndex, &storageBlock);
                 memAllocInfo->allocatedSize = GetSBBlockSize(storageBlock);
            }

            argument->data       = memAllocInfo;
            argument->uniform    = uniform;
            argument->size       = bytes;
            argument->set        = gcvFALSE;
            argument->isMemAlloc = gcvTRUE;
            argument->isPointer  = gcvFALSE;

            if (isUniformPrintfAddress(uniform))
            {
                Kernel->hasPrintf = gcvTRUE;
            }
        }
        else
        {
            if (isPointer)
            {
                if ((type == gcSHADER_IMAGE_2D_T) || (type == gcSHADER_IMAGE_2D_ARRAY_T))
                {
                    bytes = gcmSIZEOF(gctPOINTER);
                }
                else
                {
                    bytes = gcmSIZEOF(gctUINT32);
                }
            }
            else
            {
                switch (type)
                {
                case gcSHADER_FLOAT_X1:
                case gcSHADER_BOOLEAN_X1:
                case gcSHADER_INTEGER_X1:
                case gcSHADER_UINT_X1:
                    bytes = 1 * gcmSIZEOF(vx_float32) * length;
                    break;

                case gcSHADER_UINT16_X1:
                    bytes = 1 * gcmSIZEOF(vx_uint16) * length;
                    break;

                case gcSHADER_FLOAT_X2:
                case gcSHADER_BOOLEAN_X2:
                case gcSHADER_INTEGER_X2:
                case gcSHADER_UINT_X2:
                    bytes = 2 * gcmSIZEOF(vx_float32) * length;
                    break;

                case gcSHADER_FLOAT_X3:
                case gcSHADER_BOOLEAN_X3:
                case gcSHADER_INTEGER_X3:
                case gcSHADER_UINT_X3:
                    bytes = 3 * gcmSIZEOF(vx_float32) * length;
                    break;

                case gcSHADER_FLOAT_X4:
                case gcSHADER_BOOLEAN_X4:
                case gcSHADER_INTEGER_X4:
                case gcSHADER_UINT_X4:
                    bytes = 4 * gcmSIZEOF(vx_float32) * length;
                    break;

                case gcSHADER_UINT_X16:
                    bytes = 16 * gcmSIZEOF(vx_float32) * length;
                    break;

                case gcSHADER_INT64_X1:
                case gcSHADER_UINT64_X1:
                    bytes = 1 * gcmSIZEOF(vx_int64) * length;
                    break;
                case gcSHADER_INT64_X2:
                case gcSHADER_UINT64_X2:
                    bytes = 2 * gcmSIZEOF(vx_int64) * length;
                    break;
                case gcSHADER_INT64_X3:
                case gcSHADER_UINT64_X3:
                    bytes = 3 * gcmSIZEOF(vx_int64) * length;
                    break;
                case gcSHADER_INT64_X4:
                case gcSHADER_UINT64_X4:
                    bytes = 4 * gcmSIZEOF(vx_int64) * length;
                    break;

                case gcSHADER_FLOAT_2X2:
                    bytes = 2 * 2 * gcmSIZEOF(vx_float32) * length;
                    break;

                case gcSHADER_FLOAT_3X3:
                    bytes = 3 * 3 * gcmSIZEOF(vx_float32) * length;
                    break;

                case gcSHADER_FLOAT_4X4:
                    bytes = 4 * 4 * gcmSIZEOF(vx_float32) * length;
                    break;

                case gcSHADER_IMAGE_2D_T:
                case gcSHADER_IMAGE_3D_T:
                case gcSHADER_SAMPLER_T:
                case gcSHADER_IMAGE_1D_T:
                case gcSHADER_IMAGE_1D_ARRAY_T:
                case gcSHADER_IMAGE_1D_BUFFER_T:
                case gcSHADER_IMAGE_2D_ARRAY_T:
                    bytes = 1 * gcmSIZEOF(vx_uint32) * length;
                    break;

                case gcSHADER_SAMPLER_2D:
                case gcSHADER_SAMPLER_3D:
                    bytes = 1 * gcmSIZEOF(vx_float32) * length;
                    break;

                case gcSHADER_UINT16_P2:
                case gcSHADER_INT16_P2:
                case gcSHADER_FLOAT16_P2:
                    bytes = 2 * gcmSIZEOF(vx_uint16) * length;
                    break;

                case gcSHADER_UINT8_X1:
                case gcSHADER_INT8_X1:
                    bytes = 1 * gcmSIZEOF(vx_float32) * length;
                    break;

                case gcSHADER_UINT16_P3:
                case gcSHADER_INT16_P3:
                case gcSHADER_FLOAT16_P3:
                    bytes = 3 * gcmSIZEOF(vx_uint16) * length;
                    break;
                case gcSHADER_UINT16_P4:
                case gcSHADER_INT16_P4:
                case gcSHADER_FLOAT16_P4:
                    bytes = 4 * gcmSIZEOF(vx_uint16) * length;
                    break;
                case gcSHADER_UINT16_P8:
                case gcSHADER_INT16_P8:
                case gcSHADER_FLOAT16_P8:
                    bytes = 8 * gcmSIZEOF(vx_uint16) * length;
                    break;
                case gcSHADER_UINT16_P16:
                case gcSHADER_INT16_P16:
                case gcSHADER_FLOAT16_P16:
                    bytes = 16 * gcmSIZEOF(vx_uint16) * length;
                    break;
                case gcSHADER_UINT16_P32:
                case gcSHADER_INT16_P32:
                case gcSHADER_FLOAT16_P32:
                    bytes = 32 * gcmSIZEOF(vx_uint16) * length;
                    break;

                case gcSHADER_UINT8_P2:
                case gcSHADER_INT8_P2:
                case gcSHADER_BOOLEAN_P2:
                    bytes = 2 * gcmSIZEOF(vx_uint8) * length;
                    break;
                case gcSHADER_UINT8_P3:
                case gcSHADER_INT8_P3:
                case gcSHADER_BOOLEAN_P3:
                    bytes = 3 * gcmSIZEOF(vx_uint8) * length;
                    break;
                case gcSHADER_UINT8_P4:
                case gcSHADER_INT8_P4:
                case gcSHADER_BOOLEAN_P4:
                    bytes = 4 * gcmSIZEOF(vx_uint8) * length;
                    break;
                case gcSHADER_UINT8_P8:
                case gcSHADER_INT8_P8:
                case gcSHADER_BOOLEAN_P8:
                    bytes = 8 * gcmSIZEOF(vx_uint8) * length;
                    break;
                case gcSHADER_UINT8_P16:
                case gcSHADER_INT8_P16:
                case gcSHADER_BOOLEAN_P16:
                    bytes = 16 * gcmSIZEOF(vx_uint8) * length;
                    break;
                case gcSHADER_UINT8_P32:
                case gcSHADER_INT8_P32:
                case gcSHADER_BOOLEAN_P32:
                    bytes = 32 * gcmSIZEOF(vx_uint8) * length;
                    break;

                default:
                    gcmASSERT("Unknown shader type");
                    bytes = 0;
                    status = gcvSTATUS_INVALID_ARGUMENT;
                    goto OnError;
                }

                switch (format)
                {
                case gcSL_INT8:
                case gcSL_UINT8:
                    bytes /= 4;
                    break;

                case gcSL_INT16:
                case gcSL_UINT16:
                    bytes /= 2;
                    break;

                default:
                    break;
                }
            }

            /* Allocate the data array. */
            gcmONERROR(gcoOS_Allocate(gcvNULL, bytes, &argument->data));

            gcoOS_ZeroMemory(argument->data, bytes);

            argument->uniform    = uniform;
            argument->size       = bytes;
            argument->set        = gcvFALSE;
            argument->isMemAlloc = gcvFALSE;
            argument->isPointer  = isPointer;
        }

        argument++;
    }

    gcmFOOTER_ARG("%d", gcvSTATUS_OK);
    return gcvSTATUS_OK;

OnError:
    if (memAllocInfo)
    {
        gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, memAllocInfo));
    }
    gcmFOOTER_ARG("%d", status);
    return status;
}

gceSTATUS
gcfVX_ExecuteKernel(
    vx_shader           Kernel,
    gctUINT             NumArgs,
    vx_argument         Args,
    vx_border_mode_t*   BorderMode,
    vx_uint32           batchID,
    gctUINT             WorkDim,
    size_t              GlobalWorkOffset[3],
    size_t              GlobalWorkScale[3],
    size_t              GlobalWorkSize[3],
    size_t              LocalWorkSize[3]
)
{
    gctUINT i;
    gceSTATUS status;
    gcmHEADER_ARG("Kernel=%p, NumArgs=0x%x, Args=%p, BorderMode=%p, batchID=0x%x, GlobalWorkOffset=%p, GlobalWorkScale=%p, GlobalWorkSize=%p, LocalWorkSize=%p",
        Kernel, NumArgs, Args, BorderMode, batchID, GlobalWorkOffset, GlobalWorkScale, GlobalWorkSize, LocalWorkSize);
    gcmASSERT(gcoVX_VerifyHardware());
    /* Load kernel states. */
    gcmONERROR(gcoVX_LoadKernelShader(Kernel->states.programState));


    /* Load argument values. */
    for (i = 0; i < NumArgs; i++)
    {
        if (Args[i].uniform && !isUniformInactive(Args[i].uniform))
        {
            gcmONERROR(gcfVX_LoadKernelArgValues(Kernel,
                                                (gcSHADER) Kernel->states.binary,
                                                &Args[i],
                                                BorderMode,
                                                batchID,
                                                WorkDim,
                                                GlobalWorkOffset,
                                                GlobalWorkScale,
                                                GlobalWorkSize,
                                                LocalWorkSize));
        }
    }


    gcmONERROR(gcoVX_InvokeKernelShader((gcSHADER) Kernel->states.binary,
                                  WorkDim,
                                  GlobalWorkOffset,
                                  GlobalWorkScale,
                                  GlobalWorkSize,
                                  LocalWorkSize,
                                  Kernel->states.programState.hints->valueOrder,
                                  Kernel->states.programState.hints->threadGroupSync,
                                  Kernel->states.programState.hints->memoryAccessFlags[gcvSHADER_HIGH_LEVEL][gcvPROGRAM_STAGE_OPENCL],
                                  Kernel->states.programState.hints->fsIsDual16
                                  ));

    status = gcvSTATUS_OK;

OnError:
    gcmFOOTER_ARG("%d", status);
    return status;
}




gceSTATUS
gcfVX_CreateShader(vx_program program, vx_char name[VX_MAX_KERNEL_NAME], gctBOOL constBorder, vx_shader *kernelShader)
{
    gcSHADER        pgmBinary, kernelBinary;
    gctUINT         binarySize;
    gctUINT         i;
    gctUINT         count, propertySize = 0;
    gctINT          propertyType = 0;
    gctSIZE_T       propertyValues[3] = {0};
    gceSHADER_FLAGS flags;
    gctPOINTER      pointer     = gcvNULL;
    gceSTATUS       status;
    gcsPROGRAM_STATE programState = {0};
    gctSIZE_T       strLen = 0;
#if (!VSC_LITE_BUILD)
    gctUINT32       gpuCount;
#endif
    gcKERNEL_FUNCTION   kernelFunction;
    vx_shader           kernel = gcvNULL;
    gctUINT             maxComputeUnits, threadCount, maxDeviceWorkGroupSize;

    gcmHEADER_ARG("program=%p, name=%s, constBorder=0x%x, kernelShader=%p", program, name, constBorder, kernelShader);

    /* Allocate kernel. */
    gcmONERROR(gcoOS_Allocate(gcvNULL, sizeof(vx_shader_s), (gctPOINTER*)&kernel));
    gcoOS_ZeroMemory(kernel, sizeof(vx_shader_s));

    /* Save program binary into buffer */
    pgmBinary = (gcSHADER) program->binary;
    gcmONERROR(gcSHADER_SaveEx(pgmBinary, gcvNULL, &binarySize));
    gcmONERROR(gcoOS_Allocate(gcvNULL, binarySize, &pointer));
    gcmONERROR(gcSHADER_SaveEx(pgmBinary, pointer, &binarySize));

    /* Construct kernel binary. */
    gcmONERROR(gcSHADER_Construct(gcSHADER_TYPE_CL, &kernelBinary));
    kernel->states.binary          = (gctUINT8_PTR) kernelBinary;

    /* Load kernel binary from program binary */
    gcmONERROR(gcSHADER_LoadEx(kernelBinary, pointer, binarySize));

    gcoOS_Free(gcvNULL, pointer);
    pointer = gcvNULL;

    /* Load kernel binary uniforms with the given kernel name */
#if (!VSC_LITE_BUILD)
    gcmONERROR(gcSHADER_LoadKernel(kernelBinary, name));
#endif

    /* Set the required work group size. */
    gcmONERROR(gcSHADER_GetKernelFunctionByName(kernelBinary, name, &kernelFunction));
    gcKERNEL_FUNCTION_GetPropertyCount(kernelFunction, &count);

    for (i = 0; i < count; i++)
    {
        gcKERNEL_FUNCTION_GetProperty(kernelFunction, i, &propertySize, &propertyType, (gctINT *)propertyValues);

        if (propertyType == gcvPROPERTY_REQD_WORK_GRP_SIZE)
        {
            kernel->compileWorkGroupSize[0] = propertyValues[0];
            kernel->compileWorkGroupSize[1] = propertyValues[1];
            kernel->compileWorkGroupSize[2] = propertyValues[2];
        }
    }

    /* Assume all dead code is removed by optimizer. */
    flags = gcvSHADER_RESOURCE_USAGE /*| gcvSHADER_DEAD_CODE */ | gcvSHADER_OPTIMIZER;
#if (!VSC_LITE_BUILD)
    gcSetCLCompiler(program->base.context->compileKernel);

    if (constBorder)
    {
        gcOPT_SetFeature(FB_ENABLE_CONST_BORDER);
    }

    gcmONERROR(gcoHAL_SetHardwareType(gcvNULL, gcvHARDWARE_3D));
    gcmONERROR(gcoVX_QueryDeviceCount(gcvNULL, &gpuCount));

    if(gpuCount > 1)
    {
        flags |= gcvSHADER_ENABLE_MULTI_GPU;
    }

    gcmONERROR(gcLinkKernel(kernelBinary,
                          flags | gcvSHADER_REMOVE_UNUSED_UNIFORMS,
                          &programState));

    kernel->states.programState = programState;

    if (constBorder)
    {
        gcOPT_ResetFeature(FB_ENABLE_CONST_BORDER);
    }
#endif
    /* Copy kernel name */
    strLen = gcoOS_StrLen(name, gcvNULL) + 1;
    gcmONERROR(gcoOS_Allocate(gcvNULL, strLen, &pointer));
    gcoOS_StrCopySafe((gctSTRING)pointer, strLen, name);
    kernel->name                   = (gctSTRING) pointer;

    pointer = gcvNULL;


    gcmONERROR(
        gcoHAL_QueryShaderCaps(gcvNULL,
                                    gcvNULL,
                                    gcvNULL,
                                    gcvNULL,
                                    gcvNULL,
                                    &maxComputeUnits,
                                    &threadCount,
                                    gcvNULL,
                                    gcvNULL));

    kernel->maxWorkItemSizes[0]   = gcmMIN(threadCount, 1024);
    kernel->maxWorkItemSizes[1]   = gcmMIN(threadCount, 1024);
    kernel->maxWorkItemSizes[2]   = gcmMIN(threadCount, 1024);

    kernel->maxGlobalWorkSize     = (gctUINT64) 4*1024*1024*1024;

    maxDeviceWorkGroupSize        = gcmMIN(threadCount, 1024);


    kernel->preferredWorkGroupSizeMultiple = 4 * maxComputeUnits;

    if (programState.hints->threadWalkerInPS)
    {
        kernel->maxWorkGroupSize = (gctUINT32)(113 / gcmMAX(2, kernel->states.programState.hints->fsMaxTemp)) *
                                   4 * maxComputeUnits;
    }
    else
    {
        kernel->maxWorkGroupSize = (gctUINT32)(113 / gcmMAX(2, kernel->states.programState.hints->vsMaxTemp)) *
                                   4 * maxComputeUnits;
    }

    /* maxWorkGroupSize should not over the device's maxWorkGroupSize. */
    if (kernel->maxWorkGroupSize > maxDeviceWorkGroupSize)
    {
        kernel->maxWorkGroupSize = maxDeviceWorkGroupSize;
    }

    gcmVERIFY_OK(gcSHADER_GetAttributeCount(kernelBinary, &kernel->attributeCount));

    /* Allocate kernel arguments. */
    gcmONERROR(gcfVX_AllocateKernelArgs(kernel));

    *kernelShader = kernel;

    status =  gcvSTATUS_OK;

    gcmFOOTER_ARG("%d", status);
    return status;
OnError:
    if (kernel)
    {
        vxoShader_Free(kernel);
    }

    if(pointer != gcvNULL) gcmOS_SAFE_FREE(gcvNULL, pointer);

    gcmFOOTER_ARG("%d", status);
    return status;
}

VX_PRIVATE_API vx_string _getShaderName(vx_string orignal, vx_string name)
{
    vx_char* pointer = strrchr(orignal, '.');

    gcmHEADER_ARG("orignal=%s, name=%s", orignal, name);

    if(pointer)
    {
        gctSTRING suffix = strchr(pointer, ':');
        pointer = pointer + 1;
        if(suffix)
            gcoOS_StrCopySafe(name, suffix - pointer + 1, pointer);
        else
            gcoOS_StrCopySafe(name, strlen(pointer) + 1, pointer);
    }
    else
        gcoOS_StrCopySafe(name, strlen(orignal)+1, orignal);

    gcmFOOTER_ARG("name=%s", name);
    return name;
}



gceSTATUS gcfVX_LoadShaderFromLinkedBinary(gctPOINTER linkedBinary, gctUINT32 linkedBinarySize, gctCHAR *name, vx_shader *kernelShader)
{
    gceSTATUS status = gcvSTATUS_OK;
    gcSHADER  kernelBinary = gcvNULL;
    vx_shader kernel = gcvNULL;
    gctPOINTER pointer;
    gctSIZE_T strLen;

    gcKERNEL_FUNCTION   kernelFunction;
    gctUINT             maxComputeUnits, threadCount, maxDeviceWorkGroupSize;

    gctUINT         count, propertySize = 0, i;
    gctINT          propertyType = 0;
    gctSIZE_T       propertyValues[3] = {0};

    gcmHEADER_ARG("linkedBinary=%p, linkedBinarySize=0x%x, name=%s, kernelShader=%p", linkedBinary, linkedBinarySize, name, kernelShader);

    /* Allocate kernel. */
    gcmONERROR(gcoOS_Allocate(gcvNULL, sizeof(vx_shader_s), (gctPOINTER*)&kernel));
    gcoOS_ZeroMemory(kernel, sizeof(vx_shader_s));

    /* Construct kernel binary. */
    gcmONERROR(gcSHADER_Construct(gcSHADER_TYPE_CL, &kernelBinary));
    kernel->states.binary = (gctUINT8_PTR)kernelBinary;


    gcmONERROR(gcLoadCLSingleKernel(
        (gctPOINTER)linkedBinary,
        linkedBinarySize,
        kernelBinary,
        &kernel->states.programState));


    /* Set the required work group size. */
    gcmONERROR(gcSHADER_GetKernelFunctionByName(kernelBinary, name, &kernelFunction));
    gcKERNEL_FUNCTION_GetPropertyCount(kernelFunction, &count);

    for (i = 0; i < count; i++)
    {
        gcKERNEL_FUNCTION_GetProperty(kernelFunction, i, &propertySize, &propertyType, (gctINT *)propertyValues);

        if (propertyType == gcvPROPERTY_REQD_WORK_GRP_SIZE)
        {
            kernel->compileWorkGroupSize[0] = propertyValues[0];
            kernel->compileWorkGroupSize[1] = propertyValues[1];
            kernel->compileWorkGroupSize[2] = propertyValues[2];
        }
    }

    /* Copy kernel name */
    strLen = gcoOS_StrLen(name, gcvNULL) + 1;
    gcmONERROR(gcoOS_Allocate(gcvNULL, strLen, &pointer));
    gcoOS_StrCopySafe((gctSTRING)pointer, strLen, name);
    kernel->name                   = (gctSTRING) pointer;


    gcmONERROR(
        gcoHAL_QueryShaderCaps(gcvNULL,
                                    gcvNULL,
                                    gcvNULL,
                                    gcvNULL,
                                    gcvNULL,
                                    &maxComputeUnits,
                                    &threadCount,
                                    gcvNULL,
                                    gcvNULL));

    kernel->maxWorkItemSizes[0]   = gcmMIN(threadCount, 1024);
    kernel->maxWorkItemSizes[1]   = gcmMIN(threadCount, 1024);
    kernel->maxWorkItemSizes[2]   = gcmMIN(threadCount, 1024);

    kernel->maxGlobalWorkSize     = (gctUINT64) 4*1024*1024*1024;

    maxDeviceWorkGroupSize        = gcmMIN(threadCount, 1024);


    kernel->preferredWorkGroupSizeMultiple = 4 * maxComputeUnits;

    if (kernel->states.programState.hints->threadWalkerInPS)
    {
        kernel->maxWorkGroupSize = (gctUINT32)(113 / gcmMAX(2, kernel->states.programState.hints->fsMaxTemp)) *
                                   4 * maxComputeUnits;
    }
    else
    {
        kernel->maxWorkGroupSize = (gctUINT32)(113 / gcmMAX(2, kernel->states.programState.hints->vsMaxTemp)) *
                                   4 * maxComputeUnits;
    }

    /* maxWorkGroupSize should not over the device's maxWorkGroupSize. */
    if (kernel->maxWorkGroupSize > maxDeviceWorkGroupSize)
    {
        kernel->maxWorkGroupSize = maxDeviceWorkGroupSize;
    }

    gcmVERIFY_OK(gcSHADER_GetAttributeCount(kernelBinary, &kernel->attributeCount));

    /* Allocate kernel arguments. */
    gcmONERROR(gcfVX_AllocateKernelArgs(kernel));

    *kernelShader = kernel;

    gcmFOOTER_ARG("%d", gcvSTATUS_OK);
    return gcvSTATUS_OK;

OnError:
    vxoShader_Free(kernel);

    gcmFOOTER_ARG("%d", status);
    return status;
}

VX_INTERNAL_API vx_status vxoKernel_CreateShaders(vx_program program, vx_char *name, vx_uint32* kernelShaderCount, vx_shader ** kernelShaders)
{
    gceSTATUS status;
    gctUINT    i;
    gcKERNEL_FUNCTION kernelFunction;
    gctSTRING kernelName;
    gcSHADER  kernelBinary = (gcSHADER) program->binary;
    gctUINT32 kernelCount = 0;
    gctSIZE_T lenOrg, lenFull;

    vx_char             searchName[128]= {0};
    vx_uint32           findkernelShaderCount = 0;
    vx_shader           *findKernelShaders = gcvNULL;

    gcmHEADER_ARG("program=%p, name=%s, kernelShaderCount=%p, kernelShaders=%p", program, name, kernelShaderCount, kernelShaders);

    if (kernelBinary == gcvNULL)
    {
        status = gcvSTATUS_INVALID_ARGUMENT;
        goto OnError;
    }

    gcoOS_StrLen(name, &lenOrg);

    if (!program->linked)
    {
        gcmONERROR(gcSHADER_GetKernelFunctionCount(kernelBinary, &kernelCount));
        gcmONERROR(gcoOS_Allocate(gcvNULL, gcmSIZEOF(gctPOINTER) * kernelCount * 2, (gctPOINTER*)&findKernelShaders));
        gcoOS_ZeroMemory(findKernelShaders, gcmSIZEOF(gctPOINTER) * kernelCount * 2);

        for (i = 0; i < kernelCount; i++)
        {
            gcmONERROR(gcSHADER_GetKernelFunction(kernelBinary, i, &kernelFunction));
            gcmONERROR(gcKERNEL_FUNCTION_GetName(kernelFunction, gcvNULL, (gctCONST_STRING *)&kernelName));
            gcoOS_StrLen(kernelName, &lenFull);
            gcoOS_StrCopySafe(searchName, lenFull+1, kernelName);

            if (gcoOS_StrNCmp(name, searchName, lenOrg) == 0)
            {
                gcmONERROR(gcfVX_CreateShader(program, kernelName, gcvFALSE, &findKernelShaders[findkernelShaderCount*2]));
                gcmONERROR(gcfVX_CreateShader(program, kernelName, gcvTRUE, &findKernelShaders[findkernelShaderCount*2 + 1]));
                findkernelShaderCount++;
            }
        }
    }
    else
    {
        gctUINT32 subKernelCount = 0, linkedBinarySize, kernelNameLength = 0;
        gctUINT32 *linkedBinary = (gctUINT32*)program->binary;
        gctBOOL   find = gcvFALSE;

        linkedBinary += 2;
        kernelCount = *linkedBinary;
        linkedBinary++;
        gcmONERROR(gcoOS_Allocate(gcvNULL, gcmSIZEOF(gctPOINTER) * kernelCount * 2, (gctPOINTER*)&findKernelShaders));
        gcoOS_ZeroMemory(findKernelShaders, gcmSIZEOF(gctPOINTER) * kernelCount * 2);
        /* skip the full size */
        linkedBinary++;

        for (i = 0; i < kernelCount; i++)
        {
            /* skip the kernel name length */
            kernelNameLength = *linkedBinary;
            linkedBinary++;

            kernelName = (gctCHAR*)linkedBinary;
            linkedBinary = (gctUINT32*)((gctUINT8*)linkedBinary + kernelNameLength);
            gcoOS_StrLen(kernelName, &lenFull);
            gcoOS_StrCopySafe(searchName, lenFull+1, kernelName);

            find = (gcoOS_StrNCmp(name, searchName, lenOrg) == 0);

            subKernelCount = *linkedBinary;
            if (subKernelCount != 2) goto OnError;
            linkedBinary++;

            if (*linkedBinary != gcvKERNEL_BINARY_NONE) goto OnError;
            linkedBinary++;

            linkedBinarySize = *linkedBinary;
            linkedBinary++;

            if (find)
            {
                gcmONERROR(gcfVX_LoadShaderFromLinkedBinary(
                    (gctPOINTER)linkedBinary,
                    linkedBinarySize, kernelName,
                    &findKernelShaders[findkernelShaderCount*2]));
            }

            /* locate to the next kernel binary */
            linkedBinary = (gctUINT32*) ((gctUINT8*)(linkedBinary) + linkedBinarySize);

            if (*linkedBinary != gcvKERNEL_BINARY_CONST_BORDER) goto OnError;
            linkedBinary++;

            linkedBinarySize = *linkedBinary;
            linkedBinary++;

            if (find)
            {
                gcmONERROR(gcfVX_LoadShaderFromLinkedBinary(
                    (gctPOINTER)linkedBinary,
                    linkedBinarySize,
                    kernelName,
                    &findKernelShaders[findkernelShaderCount*2 + 1]));

                findkernelShaderCount++;
            }

            /* locate to the next kernel binary */
            linkedBinary = (gctUINT32*) ((gctUINT8*)(linkedBinary) + linkedBinarySize);
        }

    }

    *kernelShaderCount  = findkernelShaderCount;
    *kernelShaders      = findKernelShaders;

    gcmFOOTER_ARG("%d", VX_SUCCESS);
    return VX_SUCCESS;

OnError:
    if (findKernelShaders)
    {
        for(i = 0; i < kernelCount * 2; i++)
        {
            if (findKernelShaders[i]) vxoShader_Free(findKernelShaders[i]);
        }

        gcoOS_Free(gcvNULL, findKernelShaders);
    }

    gcmFOOTER_ARG("%d", VX_FAILURE);
    return VX_FAILURE;
}

VX_INTERNAL_API vx_status vxoKernel_Initialize(
    vx_context context,
    vx_kernel kernel,
    const vx_char name[VX_MAX_KERNEL_NAME],
    vx_enum kernelEnum,
    vx_program program,
    vx_kernel_f function,
    vx_param_description_s *parameters,
    vx_uint32 paramCount,
    vx_kernel_validate_f validateFunction,
    vx_kernel_input_validate_f inputValidateFunction,
    vx_kernel_output_validate_f outputValidateFunction,
    vx_kernel_initialize_f initializeFunction,
    vx_kernel_deinitialize_f deinitializeFunction
#if gcdVX_OPTIMIZER
, vx_kernel_optimization_attribute_s optAttributes
#endif
    )
{
    vx_uint32 i;

    gcmHEADER_ARG("context=%p, kernel=%p, name=%s, kernelEnum=0x%x, program=%p, function=%p", context, kernel, name, kernelEnum, program, function);

    vxmASSERT(context);
    vxmASSERT(paramCount <= VX_MAX_PARAMETERS);

    if (kernel == VX_NULL)
    {
        gcmFOOTER_ARG("%d", VX_FAILURE);
        return VX_FAILURE;
    }
    vxoReference_Initialize(&kernel->base, context, VX_TYPE_KERNEL, &context->base);

    vxoContext_AddObject(context, &kernel->base);

    vxoReference_Increment(&kernel->base, VX_REF_INTERNAL);

    vxStrCopySafe(kernel->name, VX_MAX_KERNEL_NAME, name);

    kernel->enumeration             = kernelEnum;

    kernel->program                 = program;

    kernel->function                = function;

    kernel->signature.paramCount    = paramCount;

    kernel->validateFunction        = validateFunction;

    kernel->inputValidateFunction   = inputValidateFunction;
    kernel->outputValidateFunction  = outputValidateFunction;
    kernel->initializeFunction      = initializeFunction;
    kernel->deinitializeFunction    = deinitializeFunction;

    kernel->attributes.borderMode.mode              = VX_BORDER_UNDEFINED;
    kernel->attributes.borderMode.constant_value.U32= 0;

#if gcdVX_OPTIMIZER
    kernel->attributes.optAttributes                = optAttributes;
#endif

    kernel->attributes.isGPUKernel                  = vx_true_e;

    if (kernel->program != VX_NULL)
    {
        vx_char shader_name[128] = {0};

        vx_status status = vxoKernel_CreateShaders(kernel->program,
                                                    _getShaderName(kernel->name, shader_name),
                                                    &kernel->kernelShaderCount,
                                                    &kernel->kernelShader);
        if (status != VX_SUCCESS)
        {
            gcmFOOTER_ARG("%d", status);
            return status;
        }

    }

    if (parameters != VX_NULL)
    {
        for (i = 0; i < paramCount; i++)
        {
            kernel->signature.directionTable[i] = parameters[i].direction;
            kernel->signature.dataTypeTable[i]  = parameters[i].dataType;
            kernel->signature.stateTable[i]     = parameters[i].state;
            kernel->signature.isStaticTable[i]  = parameters[i].isStatic;
        }

        //kernel->enabled = vx_true_e;
    }

    gcmFOOTER_ARG("%d", VX_SUCCESS);
    return VX_SUCCESS;
}

VX_INTERNAL_API void vxoKernel_Dump(vx_kernel kernel)
{
    if (kernel == VX_NULL)
    {
        vxTrace(VX_TRACE_KERNEL, "<kernel>null</kernel>\n");
    }
    else
    {
        vxoReference_Dump((vx_reference)kernel);

        vxTrace(VX_TRACE_KERNEL,
                "<kernel>\n"
                "   <address>"VX_FORMAT_HEX"</address>\n"
                "   <name>%s</name>\n"
                "   <enumeration>"VX_FORMAT_HEX"</enumeration>\n"
                "   <enabled>%s</enabled>\n"
                "</kernel>",
                kernel, kernel->name, kernel->enumeration, vxmBOOL_TO_STRING(kernel->enabled));
    }
}

VX_INTERNAL_API vx_status vxoKernel_InternalRelease(vx_kernel_ptr kernelPtr)
{
    vx_kernel kernel;

    gcmHEADER_ARG("kernelPtr=%p", kernelPtr);

    vxmASSERT(kernelPtr);

    kernel = *kernelPtr;

    if (kernel == VX_NULL)
    {
        gcmFOOTER_NO();
        return VX_ERROR_INVALID_REFERENCE;
    }
    *kernelPtr = VX_NULL;

    if (!vxoReference_IsValidAndSpecific(&kernel->base, VX_TYPE_KERNEL))
    {
        gcmFOOTER_ARG("%d", VX_ERROR_INVALID_REFERENCE);
        return VX_ERROR_INVALID_REFERENCE;
    }

    gcmFOOTER_NO();
    return vxoReference_Release((vx_reference_ptr)&kernel, VX_TYPE_KERNEL, VX_REF_INTERNAL);
}

VX_INTERNAL_API vx_status vxoKernel_ExternalRelease(vx_kernel_ptr kernelPtr)
{
    vx_kernel kernel;

    gcmHEADER_ARG("kernelPtr=%p", kernelPtr);

    vxmASSERT(kernelPtr);

    kernel = *kernelPtr;

    if (kernel == VX_NULL)
    {
        gcmFOOTER_ARG("%d", VX_ERROR_INVALID_REFERENCE);
        return VX_ERROR_INVALID_REFERENCE;
    }
    *kernelPtr = VX_NULL;

    gcmFOOTER_NO();
    return vxoReference_Release((vx_reference_ptr)&kernel, VX_TYPE_KERNEL, VX_REF_EXTERNAL);
}


VX_INTERNAL_API vx_bool vxoKernel_IsUnique(vx_kernel kernel)
{
    vx_context context;
    vx_uint32 i, k;

    gcmHEADER_ARG("kernel=%p", kernel);

    vxmASSERT(kernel);

    context = kernel->base.context;

    vxmASSERT(context);

    for (i = 0u; i < context->targetCount; i++)
    {
        for (k = 0; k < VX_MAX_KERNEL_COUNT; k++)
        {
            if (context->targetTable[i].kernelTable[k].enabled
                && context->targetTable[i].kernelTable[k].enumeration == kernel->enumeration)
            {
                gcmFOOTER_ARG("%d", vx_false_e);
                return vx_false_e;
            }
        }
    }

    gcmFOOTER_ARG("%d", vx_true_e);
    return vx_true_e;
}

VX_PRIVATE_API vx_size gcOOS_StrNIndex(const vx_char *str, vx_char c, vx_size limit)
{
    vx_size index = 0;
    gcmHEADER_ARG("str=%s, c=%s, limit=0x%lx", str, c, limit);

    while (index < limit && *str != c)
    {
        if(!*str)
        {
            index = limit;
            break;
        }
        str++;
        index++;
    }

    gcmFOOTER_ARG("0x%lx", index);
    return index;
}


VX_PRIVATE_API vx_status vxoKernel_Remove(vx_kernel kernel)
{
    vx_status status = VX_SUCCESS;

    gcmHEADER_ARG("kernel=%p", kernel);

    if (kernel == NULL
        || !vxoReference_IsValidAndSpecific(&kernel->base, VX_TYPE_KERNEL)
        || !kernel->isUserkernel)
    {
        gcmFOOTER_ARG("%d", VX_ERROR_INVALID_PARAMETERS);
        return VX_ERROR_INVALID_PARAMETERS;
    }
    if (kernel->base.context->targetCount > 0)
    {
        vx_uint32 t = 0, k = 0;
        vx_target target = VX_NULL;

        vx_char targetName[VX_MAX_TARGET_NAME];
        vx_uint32 kernelIdx = 0u;
        vx_context context = kernel->base.context;

        /* find back references to kernel's target and kernel in target->kernels array */
        vx_uint32 index = (vx_uint32)(gcOOS_StrNIndex(kernel->name, ':', VX_MAX_TARGET_NAME));
        if (index == VX_MAX_TARGET_NAME)
        {
            strncpy(targetName, "vivante.any", VX_MAX_TARGET_NAME);
        }
        else
        {
            strncpy(targetName, kernel->name, index);
        }

        for (t = 0u; t < kernel->base.context->targetCount; t++)
        {
            target = &context->targetTable[t];
            if (strncmp(targetName,target->name, VX_MAX_TARGET_NAME) == 0)
            {
                break;
            }
            target = NULL;
        }

        if (target)
        {
            for (k = 0u; k < VX_MAX_KERNEL_COUNT; k++)
            {
                if (kernel == &(target->kernelTable[k]))
                {
                    kernelIdx = k;
                    break;
                }
            }
        }

        if (target && kernelIdx < VX_MAX_KERNEL_COUNT)
        {

            if (kernel->enabled)
            {
                kernel->enabled = vx_false_e;
                context->kernelCount --;

                if (vxoKernel_IsUnique(kernel))
                    context->uniqueKernelCount--;
            }

            target->kernelCount--;

            kernel->isUserkernel = vx_false_e;

            status = vxoReference_Decrement(&kernel->base, VX_REF_EXTERNAL);

            status = vxoReference_Release((vx_reference*)&kernel, VX_TYPE_KERNEL, VX_REF_INTERNAL);

            if (status == VX_SUCCESS)
            {
                target->kernelTable[kernelIdx].enumeration = VX_KERNEL_INVALID;
                target->kernelTable[kernelIdx].isUserkernel = vx_false_e;
            }
            else
            {
                vxError("Can't deinitialize kernel properly\n");
            }
        }
        else
        {
            vxError("Can't locate kernel in its context\n");
        }
    }

    gcmFOOTER_ARG("%d", status);
    return status;
}

VX_INTERNAL_API vx_kernel vxoKernel_GetByEnumFromTarget(vx_context context, vx_target target, vx_uint32 targetIndex, vx_enum kernelEnum)
{
    vx_uint32 kernelIndex;

    gcmHEADER_ARG("context=%p, target=%p, targetIndex=0x%x, kernelEnum=0x%x", context, target, targetIndex, kernelEnum);

    if (!vxoContext_IsValid(context))
    {
        gcmFOOTER_NO();
        return VX_NULL;
    }
    if (kernelEnum < VX_KERNEL_INVALID)
    {
        gcmFOOTER_NO();
        return (vx_kernel)vxoContext_GetErrorObject(context, VX_ERROR_INVALID_PARAMETERS);
    }

    if (target == VX_NULL || !target->enabled)
    {
        gcmFOOTER_NO();
        return (vx_kernel)vxoContext_GetErrorObject(context, VX_ERROR_INVALID_PARAMETERS);
    }
    for (kernelIndex = 0; kernelIndex < VX_MAX_KERNEL_COUNT; kernelIndex++)
    {
        if (target->kernelTable[kernelIndex].enumeration == kernelEnum)
        {
            vx_kernel kernel = &target->kernelTable[kernelIndex];

            if (!kernel->enabled) continue;

            kernel->targetIndex = targetIndex;

            vxoReference_Increment(&kernel->base, VX_REF_EXTERNAL);

            vxoKernel_Dump(kernel);

            gcmFOOTER_ARG("kernel=%p", kernel);
            return kernel;
        }
    }
    vxError("Kernel enum %d does not exist", kernelEnum);

    gcmFOOTER_NO();
    return (vx_kernel)vxoContext_GetErrorObject(context, VX_ERROR_INVALID_PARAMETERS);
}

VX_INTERNAL_API vx_kernel vxoKernel_GetByEnum(vx_context context, vx_enum kernelEnum)
{
    vx_uint32 index, targetIndex;
    vx_kernel kernel = VX_NULL;

    gcmHEADER_ARG("context=%p, kernelEnum=0x%x", context, kernelEnum);

    if (!vxoContext_IsValid(context))
    {
        gcmFOOTER_NO();
        return VX_NULL;
    }
    if (kernelEnum < VX_KERNEL_INVALID)
    {
        gcmFOOTER_NO();
        return (vx_kernel)vxoContext_GetErrorObject(context, VX_ERROR_INVALID_PARAMETERS);
    }

    for (index = 0; index < context->targetCount; index++)
    {
        vx_target target;
        targetIndex = context->targetPriorityTable[index];
        target = &context->targetTable[targetIndex];

        if (target == VX_NULL || !target->enabled) continue;

        kernel = vxoKernel_GetByEnumFromTarget(context, target, targetIndex, kernelEnum);

        if (kernel == VX_NULL) continue;

        gcmFOOTER_ARG("kernel=%p", kernel);
        return kernel;
    }

    vxError("Kernel enum %d does not exist", kernelEnum);
    gcmFOOTER_NO();
    return (vx_kernel)vxoContext_GetErrorObject(context, VX_ERROR_INVALID_PARAMETERS);
}

VX_PRIVATE_API vx_size vxString_GetCharIndex(vx_const_string string, vx_char ch, vx_size limit)
{
    vx_size index;

    vxmASSERT(string);

    for (index = 0; index < limit; index++)
    {
        if (string[index] == '\0') return limit;

        if (string[index] == ch) break;
    }

    return index;
}

VX_PRIVATE_API vx_size vxString_GetCharCount(vx_const_string string, vx_size size, vx_char ch)
{
    vx_size index;
    vx_size count = 0;

    for (index = 0; index < size; index++)
    {
        if (string[index] == '\0') break;

        if (string[index] == ch) count++;
    }

    return count;
}

VX_API_ENTRY vx_status VX_API_CALL vxLoadKernels(vx_context context, const vx_char *module)
{
    gcmDUMP_API("$VX vxLoadKernels: context=%p, module=%p", context, module);
    return vxContext_LoadKernels(context, (vx_string)module);
}

VX_API_ENTRY vx_kernel VX_API_CALL vxGetKernelByName(vx_context context, const vx_char string[VX_MAX_KERNEL_NAME])
{
    vx_char     tempString[VX_MAX_KERNEL_NAME];
    vx_uint32   targetIndex;

    vx_char     targetName[VX_MAX_TARGET_NAME] = "default";
    vx_char     kernelName[VX_MAX_KERNEL_NAME];

#if defined(OPENVX_USE_VARIANTS)
    vx_char     variantName[VX_MAX_VARIANT_NAME] = "default";

#if defined(OPENVX_USE_TARGET)
    vx_char     defaultTargets[][VX_MAX_TARGET_NAME] = {
        "default",
        "power",
        "performance",
        "memory",
        "bandwidth",
    };
#endif

#endif
    gcmHEADER_ARG("context=%p, string=%s", context, string);
    gcmDUMP_API("$VX vxGetKernelByName: context=%p, string=%s", context, string);

    if (!vxoContext_IsValid(context))
    {
        gcmFOOTER_NO();
        return VX_NULL;
    }
    vxStrCopySafe(tempString, VX_MAX_KERNEL_NAME, string);

    switch (vxString_GetCharCount(string, VX_MAX_KERNEL_NAME, ':'))
    {
        case 0:
            vxStrCopySafe(kernelName, VX_MAX_KERNEL_NAME, string);
            break;

        case 1:
            {
#if defined(OPENVX_USE_TARGET) || defined(OPENVX_USE_VARIANTS)

                vx_string   firstName   = strtok(tempString, ":");
                vx_string   lastName    = strtok(VX_NULL, ":");

#if defined(OPENVX_USE_TARGET) && defined(OPENVX_USE_VARIANTS)

                vx_bool     isTarget = vx_false_e;

                for (targetIndex = 0; targetIndex < context->targetCount; targetIndex++)
                {
                    if (vxIsSameString(firstName, context->targetTable[targetIndex].name, VX_MAX_TARGET_NAME))
                    {
                        isTarget = vx_true_e;
                        break;
                    }
                }

                if (!isTarget)
                {
                    for (targetIndex = 0u; targetIndex < vxmLENGTH_OF(defaultTargets); targetIndex++)
                    {
                        if (vxIsSameString(firstName, defaultTargets[targetIndex], VX_MAX_TARGET_NAME))
                        {
                            isTarget = vx_true_e;
                            break;
                        }
                    }
                }

                if (isTarget)
                {
                    vxStrCopySafe(targetName, VX_MAX_TARGET_NAME, firstName);
                    vxStrCopySafe(kernelName, VX_MAX_KERNEL_NAME, lastName);
                }
                else
                {
                    vxStrCopySafe(kernelName, VX_MAX_KERNEL_NAME, firstName);
                    vxStrCopySafe(variantName, VX_MAX_VARIANT_NAME, lastName);
                }

#elif defined(OPENVX_USE_TARGET)

                vxStrCopySafe(targetName, VX_MAX_TARGET_NAME, firstName);
                vxStrCopySafe(kernelName, VX_MAX_KERNEL_NAME, lastName);

#elif defined(OPENVX_USE_VARIANTS)

                vxStrCopySafe(kernelName, VX_MAX_KERNEL_NAME, firstName);
                vxStrCopySafe(variantName, VX_MAX_VARIANT_NAME, lastName);

#endif /* defined(OPENVX_USE_TARGET) && defined(OPENVX_USE_VARIANTS) */

#else
                vxError("Invalid kernel name: \"%s\"", string);
                return (vx_kernel)vxoContext_GetErrorObject(context, VX_ERROR_INVALID_PARAMETERS);
#endif /* defined(OPENVX_USE_TARGET) || defined(OPENVX_USE_VARIANTS) */
            }
            break;

        case 2:
            {
#if defined(OPENVX_USE_TARGET) && defined(OPENVX_USE_VARIANTS)
                vx_string target    = strtok(tempString, ":");
                vx_string kernel    = strtok(VX_NULL, ":");
                vx_string variant   = strtok(VX_NULL, ":");

                vxStrCopySafe(targetName, VX_MAX_TARGET_NAME, target);
                vxStrCopySafe(kernelName, VX_MAX_KERNEL_NAME, kernel);
                vxStrCopySafe(variantName, VX_MAX_VARIANT_NAME, variant);
#else
                vxError("Invalid kernel name: \"%s\"", string);
                return (vx_kernel)vxoContext_GetErrorObject(context, VX_ERROR_INVALID_PARAMETERS);
#endif
            }
            break;

        default:
            vxError("Invalid kernel name: \"%s\"", string);
            gcmFOOTER_NO();
            return (vx_kernel)vxoContext_GetErrorObject(context, VX_ERROR_INVALID_PARAMETERS);
    }

    for (targetIndex = 0; targetIndex < context->targetCount; targetIndex++)
    {
        vx_status status;
        vx_uint32 kernelIndex;
        vx_target target = &context->targetTable[context->targetPriorityTable[targetIndex]];
        vx_kernel kernel;

        if (target == VX_NULL || !target->enabled) continue;

        status = target->funcs.iskernelsupported(target, targetName, kernelName,
#if defined(OPENVX_USE_VARIANTS)
                                                    variantName,
#endif
                                                    &kernelIndex);

        if (status != VX_SUCCESS) continue;

        kernel = &target->kernelTable[kernelIndex];

        if (!kernel->enabled) continue;

        kernel->targetIndex = context->targetPriorityTable[targetIndex];

        vxoReference_Increment(&kernel->base, VX_REF_EXTERNAL);

        vxoKernel_Dump(kernel);

        gcmFOOTER_ARG("kernel=%d=p", kernel);
        return kernel;
    }

    vxError("Kernel \"%s\" does not exist", string);

    gcmFOOTER_NO();
    return (vx_kernel)vxoContext_GetErrorObject(context, VX_ERROR_INVALID_PARAMETERS);
}

VX_API_ENTRY vx_kernel VX_API_CALL vxGetKernelByEnum(vx_context context, vx_enum kernel_enum)
{
    gcmDUMP_API("$VX vxGetKernelByEnum: context=%p, kernel_enum=0x%x", context, kernel_enum);
    return vxoKernel_GetByEnum(context, kernel_enum);
}

VX_API_ENTRY vx_status VX_API_CALL vxReleaseKernel(vx_kernel *kernel)
{
    gcmHEADER_ARG("kernel=%p", kernel);
    gcmDUMP_API("$VX vxReleaseKernel: kernel=%p", kernel);

    if ((*kernel)->enumeration == VX_KERNEL_IMPORT_FROM_FILE)
    {
        /* release binary kernel*/
        vx_binary_loader_s *binaryLoad = (vx_binary_loader_s*)((*kernel)->base.reserved);
        vxoGraphBinary_ReleaseFile(binaryLoad);
        gcmFOOTER_NO();
        return vxoKernel_ExternalRelease(kernel);
    }
    else
    {
        gcmFOOTER_NO();
        return vxoKernel_ExternalRelease(kernel);
    }
}

VX_API_ENTRY vx_kernel VX_API_CALL vxoKernel_Add(
        vx_context context, const vx_char name[VX_MAX_KERNEL_NAME], vx_enum enumeration,
        vx_kernel_f func_ptr, vx_uint32 num_params, vx_kernel_validate_f validate, vx_kernel_input_validate_f input,
        vx_kernel_output_validate_f output, vx_kernel_initialize_f initialize, vx_kernel_deinitialize_f deinitialize)
{
    vx_size     colonCharIndex;
    vx_uint32   targetIndex;
    vx_char     targetName[VX_MAX_TARGET_NAME] = VX_DEFAULT_TARGET_NAME;
    vx_kernel   kernel = VX_NULL;

    gcmHEADER_ARG("context=%p, name=%s, enumeration=0x%x, func_ptr=%p, num_params=0x%x, validate=%p, input=%p, output=%p, initialize=%p, deinitialize=%p",
        context, name, enumeration, func_ptr, num_params, validate, input, output, initialize, deinitialize);
    gcmDUMP_API("$VX vxoKernel_Add: context=%p, name=%s, enumeration=0x%x, func_ptr=%p, num_params=0x%x, validate=%p, input=%p, output=%p, initialize=%p, deinitialize=%p",
        context, name, enumeration, func_ptr, num_params, validate, input, output, initialize, deinitialize);

    if (!vxoContext_IsValid(context))
    {
        gcmFOOTER_NO();
        return VX_NULL;
    }
    if (name == VX_NULL || strlen(name) == 0) goto ErrorExit;

    if (func_ptr == VX_NULL) goto ErrorExit;

    if (num_params == 0 || num_params > VX_MAX_PARAMETERS) goto ErrorExit;

    /* The initialize and de-initialize function can be null */
    if ((validate == NULL) && (input == NULL || output == NULL)) goto ErrorExit;

    colonCharIndex = vxString_GetCharIndex(name, ':', VX_MAX_TARGET_NAME);

    colonCharIndex == VX_MAX_TARGET_NAME ? strncpy(targetName, "vivante.any", VX_MAX_TARGET_NAME) : strncpy(targetName, name, colonCharIndex);

    for (targetIndex = 0; targetIndex < context->targetCount; targetIndex++)
    {
        vx_target target = &context->targetTable[targetIndex];

        if (vxIsSameString(targetName, target->name, VX_MAX_TARGET_NAME))
        {
            vxmASSERT(target->funcs.addkernel);

            kernel = target->funcs.addkernel(target, name, enumeration,
                                             VX_NULL, func_ptr, num_params, validate,
                                             input, output, initialize, deinitialize);

            kernel->attributes.isGPUKernel = vx_false_e;
            kernel->isUserkernel = vx_true_e;

            vxoReference_Increment(&kernel->base, VX_REF_EXTERNAL);

            gcmFOOTER_ARG("kernel=%p", kernel);
            return kernel;
        }
    }

    vxError("Faild to find target \"%s\" for vxAddKernel", targetName);

ErrorExit:
    gcmFOOTER_NO();
    return (vx_kernel)vxoContext_GetErrorObject(context, VX_ERROR_INVALID_PARAMETERS);
}

VX_API_ENTRY vx_kernel VX_API_CALL vxAddKernel(
        vx_context context, const vx_char name[VX_MAX_KERNEL_NAME], vx_enum enumeration,
        vx_kernel_f func_ptr, vx_uint32 num_params, vx_kernel_input_validate_f input,
        vx_kernel_output_validate_f output, vx_kernel_initialize_f initialize, vx_kernel_deinitialize_f deinitialize)
{

    gcmDUMP_API("$VX vxAddKernel: context=%p, name=%s, enumeration=0x%x, func_ptr=%p, num_params=0x%x, input=%p, output=%p, initialize=%p, deinitialize=%p",
        context, name, enumeration, func_ptr, num_params, input, output, initialize, deinitialize);

    return vxoKernel_Add(context, name, enumeration, func_ptr, num_params, VX_NULL, input, output, initialize, deinitialize);
}

VX_API_ENTRY vx_kernel VX_API_CALL vxAddUserKernel(vx_context context,
                             const vx_char name[VX_MAX_KERNEL_NAME],
                             vx_enum enumeration,
                             vx_kernel_f func_ptr,
                             vx_uint32 numParams,
                             vx_kernel_validate_f validate,
                             vx_kernel_initialize_f initialize,
                             vx_kernel_deinitialize_f deinitialize)
{
    gcmDUMP_API("$VX vxAddUserKernel: context=%p, name=%s, enumeration=0x%x, func_ptr=%p, numParams=0x%x, validate=%p, initialize=%p, deinitialize=%p",
        context, name, enumeration, func_ptr, numParams, validate, initialize, deinitialize);

    return vxoKernel_Add(context, name, enumeration, func_ptr, numParams, validate,
                         VX_NULL, VX_NULL, initialize, deinitialize);
}

VX_API_ENTRY vx_kernel VX_API_CALL vxAddTilingKernel(
        vx_context context, vx_char name[VX_MAX_KERNEL_NAME], vx_enum enumeration,
        vx_tiling_kernel_f flexible_func_ptr, vx_tiling_kernel_f fast_func_ptr,
        vx_uint32 num_params, vx_kernel_input_validate_f input, vx_kernel_output_validate_f output)
{
    vx_size     colonCharIndex;
    vx_uint32   targetIndex;
    vx_char     targetName[VX_MAX_TARGET_NAME] = VX_DEFAULT_TARGET_NAME;

    gcmHEADER_ARG("context=%p, name=%s, enumeration=0x%x, flexible_func_ptr=%p, fast_func_ptr=%p, num_params=0x%x, input=%p, output=%p",
        context, name, enumeration, flexible_func_ptr, fast_func_ptr, num_params, input, output);
    gcmDUMP_API("$VX vxAddTilingKernel: context=%p, name=%s, enumeration=0x%x, flexible_func_ptr=%p, fast_func_ptr=%p, num_params=0x%x, input=%p, output=%p",
        context, name, enumeration, flexible_func_ptr, fast_func_ptr, num_params, input, output);

    if (!vxoContext_IsValid(context))
    {
        gcmFOOTER_NO();
        return VX_NULL;
    }
    if (name == VX_NULL || strlen(name) == 0) goto ErrorExit;

    if (flexible_func_ptr == VX_NULL && fast_func_ptr == VX_NULL) goto ErrorExit;

    if (num_params == 0 || num_params > VX_MAX_PARAMETERS) goto ErrorExit;

    /* The initialize and de-initialize function can be null */
    if (input == VX_NULL || output == VX_NULL) goto ErrorExit;

    colonCharIndex = vxString_GetCharIndex(name, ':', VX_MAX_TARGET_NAME);

    if (colonCharIndex != VX_MAX_TARGET_NAME) vxStrCopySafe(targetName, colonCharIndex, name);

    for (targetIndex = 0; targetIndex < context->targetCount; targetIndex++)
    {
        vx_target target = &context->targetTable[targetIndex];

        if (vxIsSameString(targetName, target->name, VX_MAX_TARGET_NAME))
        {
            if (target->funcs.addtilingkernel != VX_NULL)
            {
                gcmFOOTER_NO();
                return target->funcs.addtilingkernel(target, name, enumeration,
                                                    flexible_func_ptr, fast_func_ptr,
                                                    num_params, input, output);
            }
        }
    }

    vxError("Faild to find target \"%s\" for vxAddTilingKernel", targetName);

ErrorExit:
    gcmFOOTER_NO();
    return (vx_kernel)vxoContext_GetErrorObject(context, VX_ERROR_INVALID_PARAMETERS);
}

VX_API_ENTRY vx_status VX_API_CALL vxRemoveKernel(vx_kernel kernel)
{
    gcmDUMP_API("$VX vxRemoveKernel: kernel=%p", kernel);
    return vxoKernel_Remove(kernel);
}

VX_API_ENTRY vx_status VX_API_CALL vxAddParameterToKernel(
        vx_kernel kernel, vx_uint32 index, vx_enum dir, vx_enum dataType, vx_enum state)
{
    gcmHEADER_ARG("kernel=%p, index=0x%x, dir=0x%x, dataType=0x%x, state=0x%x", kernel, index, dir, dataType, state);
    gcmDUMP_API("$VX vxAddParameterToKernel: kernel=%p, index=0x%x, dir=0x%x, dataType=0x%x, state=0x%x", kernel, index, dir, dataType, state);

    if (!vxoReference_IsValidAndSpecific(&kernel->base, VX_TYPE_KERNEL))
    {
        gcmFOOTER_ARG("%d", VX_ERROR_INVALID_REFERENCE);
        return VX_ERROR_INVALID_REFERENCE;
    }
    if (index >= kernel->signature.paramCount)
    {
        gcmFOOTER_ARG("%d", VX_ERROR_INVALID_PARAMETERS);
        return VX_ERROR_INVALID_PARAMETERS;
    }
    if (kernel->tilingFunction == VX_NULL)
    {
        if (!vxDataType_IsValid(dataType))
        {
            gcmFOOTER_ARG("%d", VX_ERROR_INVALID_PARAMETERS);
            return VX_ERROR_INVALID_PARAMETERS;
        }
    }
    else
    {
        if (dataType != VX_TYPE_IMAGE && dataType != VX_TYPE_SCALAR)
        {
            gcmFOOTER_ARG("%d", VX_ERROR_INVALID_PARAMETERS);
            return VX_ERROR_INVALID_PARAMETERS;
        }
    }

    if (!vxmIS_VALID_DIRECTION_FOR_USER_KERNEL(dir) || !vxmIS_VALID_STATE(state) || !vxDataType_IsValid(dataType) || (dataType == VX_TYPE_DELAY && dir != VX_INPUT))
    {
        gcmFOOTER_ARG("%d", VX_ERROR_INVALID_PARAMETERS);
        return VX_ERROR_INVALID_PARAMETERS;
    }

    kernel->signature.directionTable[index] = dir;
    kernel->signature.dataTypeTable[index]  = dataType;
    kernel->signature.stateTable[index]     = state;
    kernel->signature.isStaticTable[index]  = vx_false_e;

    gcmFOOTER_ARG("%d", VX_SUCCESS);
    return VX_SUCCESS;
}

VX_API_ENTRY vx_status VX_API_CALL vxFinalizeKernel(vx_kernel kernel)
{
    vx_uint32 i;

    gcmHEADER_ARG("kernel=%p", kernel);
    gcmDUMP_API("$VX vxFinalizeKernel: kernel=%p", kernel);

    if (!vxoReference_IsValidAndSpecific(&kernel->base, VX_TYPE_KERNEL))
    {
        gcmFOOTER_ARG("%d", VX_ERROR_INVALID_REFERENCE);
        return VX_ERROR_INVALID_REFERENCE;
    }
    for (i = 0; i < kernel->signature.paramCount; i++)
    {
        if (i >= kernel->signature.paramCount)
            break;
        if (!vxmIS_VALID_DIRECTION(kernel->signature.directionTable[i])
            || !vxDataType_IsValid(kernel->signature.dataTypeTable[i]))
        {
            gcmFOOTER_ARG("%d", VX_ERROR_INVALID_PARAMETERS);
            return VX_ERROR_INVALID_PARAMETERS;
        }
    }

    if (i == kernel->signature.paramCount)
    {

    if (vxoKernel_IsUnique(kernel))
    {
        kernel->base.context->uniqueKernelCount++;
    }

    kernel->enabled = vx_true_e;
    }

    gcmFOOTER_ARG("%d", VX_SUCCESS);
    return VX_SUCCESS;
}

VX_API_ENTRY vx_status VX_API_CALL vxQueryKernel(vx_kernel kernel, vx_enum attribute, void *ptr, vx_size size)
{
    vx_char name[VX_MAX_KERNEL_NAME];
    vx_char *namePtr;

    gcmHEADER_ARG("kernel=%p, attribute=0x%x, ptr=%p, size=0x%lx", kernel, attribute, ptr, size);
    gcmDUMP_API("$VX vxQueryKernel: kernel=%p, attribute=0x%x, ptr=%p, size=0x%lx", kernel, attribute, ptr, size);

    if (!vxoReference_IsValidAndSpecific(&kernel->base, VX_TYPE_KERNEL))
    {
        gcmFOOTER_ARG("%d", VX_ERROR_INVALID_REFERENCE);
        return VX_ERROR_INVALID_REFERENCE;
    }
    switch (attribute)
    {
        case VX_KERNEL_PARAMETERS:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_uint32, 0x3);

            *(vx_uint32 *)ptr = kernel->signature.paramCount;
            break;

        case VX_KERNEL_NAME:
            if (ptr == NULL || size > VX_MAX_KERNEL_NAME)
            {
                gcmFOOTER_ARG("%d", VX_ERROR_INVALID_PARAMETERS);
                return VX_ERROR_INVALID_PARAMETERS;
            }
            vxStrCopySafe(name, VX_MAX_KERNEL_NAME, kernel->name);

            namePtr = strtok(name, ":");

            vxStrCopySafe((vx_string)ptr, VX_MAX_KERNEL_NAME, namePtr);
            break;

        case VX_KERNEL_ENUM:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_enum, 0x3);

            *(vx_enum *)ptr = kernel->enumeration;
            break;

        case VX_KERNEL_LOCAL_DATA_SIZE:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_size, 0x3);

            *(vx_size *)ptr = kernel->attributes.localDataSize;
            break;

#ifdef OPENVX_KHR_TILING
        case VX_KERNEL_INPUT_NEIGHBORHOOD:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_neighborhood_size_t, 0x3);

            *(vx_neighborhood_size_t *)ptr = kernel->attributes.inputNeighborhoodSize;
            break;

        case VX_KERNEL_OUTPUT_TILE_BLOCK_SIZE:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_tile_block_size_t, 0x3);

            *(vx_tile_block_size_t *)ptr = kernel->attributes.tileBlockSize;
            break;
#endif

        default:
            vxError("The attribute parameter, %d, is not supported", attribute);
            gcmFOOTER_ARG("%d", VX_ERROR_NOT_SUPPORTED);
            return VX_ERROR_NOT_SUPPORTED;
    }

    gcmFOOTER_ARG("%d", VX_SUCCESS);
    return VX_SUCCESS;
}

VX_API_ENTRY vx_status VX_API_CALL vxSetKernelAttribute(vx_kernel kernel, vx_enum attribute, const void *ptr, vx_size size)
{
#ifdef OPENVX_KHR_TILING
    vx_border_t *borderMode;
#endif
    gcmHEADER_ARG("kernel=%p, attribute=0x%x, ptr=%p, size=0x%lx", kernel, attribute, ptr, size);
    gcmDUMP_API("$VX vxSetKernelAttribute: kernel=%p, attribute=0x%x, ptr=%p, size=0x%lx", kernel, attribute, ptr, size);

    if (!vxoReference_IsValidAndSpecific(&kernel->base, VX_TYPE_KERNEL))
    {
        gcmFOOTER_ARG("%d", VX_ERROR_INVALID_REFERENCE);
        return VX_ERROR_INVALID_REFERENCE;
    }
    if (kernel->enabled)
    {
        gcmFOOTER_ARG("%d", VX_ERROR_NOT_SUPPORTED);
        return VX_ERROR_NOT_SUPPORTED;
    }
    switch (attribute)
    {
        case VX_KERNEL_LOCAL_DATA_SIZE:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_size, 0x3);

            kernel->attributes.localDataSize = *(vx_size *)ptr;
            break;


#ifdef OPENVX_KHR_TILING
        case VX_KERNEL_INPUT_NEIGHBORHOOD:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_neighborhood_size_t, 0x3);

            kernel->attributes.inputNeighborhoodSize = *(vx_neighborhood_size_t *)ptr;
            break;

        case VX_KERNEL_OUTPUT_TILE_BLOCK_SIZE:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_tile_block_size_t, 0x3);

            kernel->attributes.tileBlockSize = *(vx_tile_block_size_t *)ptr;
            break;

        case VX_KERNEL_BORDER:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_border_t, 0x3);

            borderMode = (vx_border_t *)ptr;

            switch (borderMode->mode)
            {
                case VX_BORDER_MODE_SELF:
                case VX_BORDER_UNDEFINED:
                    break;

                default:
                    vxError("Unsupported border mode: %d", borderMode->mode);
                    gcmFOOTER_ARG("%d", VX_ERROR_INVALID_VALUE);
                    return VX_ERROR_INVALID_VALUE;
            }

            kernel->attributes.borderMode = *borderMode;
            break;
#endif

        default:
            vxError("The attribute parameter, %d, is not supported", attribute);
            gcmFOOTER_ARG("%d", VX_ERROR_NOT_SUPPORTED);
            return VX_ERROR_NOT_SUPPORTED;
    }
    gcmFOOTER_ARG("%d", VX_SUCCESS);
    return VX_SUCCESS;
}

#if gcdDUMP
VX_INTERNAL_API vx_status vxoDumpOutput(vx_node node, const vx_reference parameters[], vx_uint32 paramCount)
{
    vx_uint32 i = 0;

    for(i = 0; i < paramCount; i++)
    {
        vx_reference ref = (vx_reference)parameters[i];
        if (node->kernel->signature.directionTable[i] != VX_INPUT
#if (gcdDUMP == 1)
            && !ref->isVirtual
#endif
            )
        {
            vx_uint8_ptr physical = 0, logical = 0;
            vx_uint32 size = 0;

            switch(ref->type)
            {
            case VX_TYPE_IMAGE:
                physical    = (vx_uint8_ptr)(((vx_image)ref)->memory.physicals[0]);
                logical     = ((vx_image)ref)->memory.logicals[0];
                size        = (vx_uint32)(((vx_image)ref)->memory.strides[0][2] * ((vx_image)ref)->height);
                break;
            case VX_TYPE_DISTRIBUTION:
                physical    = (vx_uint8_ptr)(((vx_image)ref)->memory.physicals[0]);
                logical     = (vx_uint8_ptr)((vx_distribution)ref)->memAllocInfo.logical;
                size        = ((vx_distribution)ref)->memAllocInfo.allocatedSize;
                break;
            case VX_TYPE_LUT:
            case VX_TYPE_ARRAY:
                physical    = (vx_uint8_ptr)(((vx_image)ref)->memory.physicals[0]);
                logical     = ((vx_array)ref)->memory.logicals[0];
                size        = ((vx_array)ref)->memAllocInfo.allocatedSize;
                break;
            case VX_TYPE_SCALAR:
                physical    = (vx_uint8_ptr)(((vx_scalar)ref)->physical);
                size        = vxoScalar_GetTypeSize((vx_scalar)ref);

                logical     = (vx_uint8_ptr)((vx_scalar)ref)->value;
                break;
            default:
                vxError("Unkown type(%d)!", ref->type);
                break;
            }

            /* Dump memory */
            gcmDUMP_BUFFER(gcvNULL,
                        gcvDUMP_BUFFER_VERIFY,
                        (gctUINT32)physical,
                        (gctPOINTER)logical,
                        0,
                        size);
        }
    }

    return VX_SUCCESS;
}
#endif

VX_INTERNAL_API vx_status vxoShader_SetParameters(vx_shader kernelShader, vx_reference parameters[], vx_uint32 paramCount, vx_enum dataTypeTable[], vx_bitfield paramAttributes[])
{
    gceSTATUS status;
    gctUINT   argIndex = 0;
    vx_uint32 i;

    gcmHEADER_ARG("kernelShader=%p, parameters=%p, paramCount=0x%x, dataTypeTable=%p, paramAttributes=%p", kernelShader, parameters, paramCount, dataTypeTable, paramAttributes);

    for (i = 0; i < paramCount; i++)
    {
        vx_type_e type = (vx_type_e)((parameters[i] != gcvNULL) ? parameters[i]->type : dataTypeTable[i]);
        vx_bitfield  attribute = paramAttributes ? paramAttributes[i] : VXNNE_SHADER_PARAMETERS_ATTRIBUTE_NONE_BIT;

        switch(type)
        {
        case VX_TYPE_SCALAR:
        {
            gctBOOL isPointer;
            vx_scalar scalar = (vx_scalar)parameters[i];
            vx_argument argument = gcfVX_GetKernelArg(kernelShader, argIndex, gcvNULL, gcvNULL, gcvNULL);

            if (scalar)
            {
                gcmONERROR(gcUNIFORM_GetFormat(argument->uniform, gcvNULL, &isPointer));

                if (!isPointer)
                {
                    gcmONERROR(gcfVX_SetKernelArg(
                                kernelShader,
                                argIndex,
                                sizeof(gctUINT32),/*vxDataType_GetSize((vx_type_e)scalar->dataType),*/
                                scalar->value));
                }
                else
                {
                    gcmONERROR(gcfVX_SetKernelArg(
                                kernelShader,
                                argIndex,
                                sizeof(gctUINT32),
                                &scalar->physical));
                }

                argIndex++;
            }
            else
            {
                argIndex++;
            }
            break;
        }
        case VX_TYPE_LUT:
        case VX_TYPE_ARRAY:
        {
            vx_array array = (vx_array)parameters[i];

            if (array)
            {
                if(array->isVivArray)
                {
                    gcmONERROR(gcfVX_SetKernelArg(
                                    kernelShader,
                                    argIndex,
                                    sizeof(vx_reference),
                                    &parameters[i]));

                    kernelShader->args[argIndex].isVivArray = vx_true_e;
                    argIndex ++;
                }
                else
                {
                    gcmONERROR(gcfVX_SetKernelArg(
                                    kernelShader,
                                    argIndex,
                                    sizeof(gctUINT32),
                                    &array->capacity));
                    argIndex ++;

                    gcmONERROR(gcfVX_SetKernelArg(
                                    kernelShader,
                                    argIndex,
                                    sizeof(gctUINT32),
                                    &array->memory.physicals[0]));
                    argIndex ++;
                }
            }
            else
            {
                argIndex += 2;
            }
            break;
        }
        case VX_TYPE_CONVOLUTION:
        {
            vx_convolution conv = (vx_convolution)parameters[i];
            if (conv)
            {
                gcmONERROR(gcfVX_SetKernelArg(
                                kernelShader,
                                argIndex,
                                sizeof(gctUINT32),
                                &conv->matrix.columns));
                argIndex ++;

                gcmONERROR(gcfVX_SetKernelArg(
                                kernelShader,
                                argIndex,
                                sizeof(gctUINT32),
                                &conv->matrix.rows));
                argIndex ++;

                gcmONERROR(gcfVX_SetKernelArg(
                                kernelShader,
                                argIndex,
                                sizeof(gctUINT32),
                                &conv->matrix.memory.physicals[0]));
                argIndex ++;

                gcmONERROR(gcfVX_SetKernelArg(
                                kernelShader,
                                argIndex,
                                sizeof(gctUINT32),
                                &conv->scale));
                argIndex ++;
            }
            else
            {
                argIndex += 4;
            }
            break;
        }
        case VX_TYPE_MATRIX:
        {
            vx_matrix matrix = (vx_matrix)parameters[i];
            vxReadMatrix(matrix, NULL);

            if (matrix)
            {
                gcmONERROR(gcfVX_SetKernelArg(
                                kernelShader,
                                argIndex,
                                sizeof(gctUINT32),
                                &matrix->columns));
                argIndex ++;

                gcmONERROR(gcfVX_SetKernelArg(
                                kernelShader,
                                argIndex,
                                sizeof(gctUINT32),
                                &matrix->rows));
                argIndex ++;

                gcmONERROR(gcfVX_SetKernelArg(
                                kernelShader,
                                argIndex,
                                sizeof(gctUINT32),
                                &matrix->memory.physicals[0]));
                argIndex ++;
            }
            else
            {
                argIndex += 3;
            }
            break;
        }
        case VX_TYPE_THRESHOLD:
        {
            vx_threshold threshold = (vx_threshold)parameters[i];
            if (threshold)
            {
                //gctUINT32  value = FV(threshold->value + 1); //bobo
                //gctUINT32  lower = (gctINT32)FV(threshold->lower);
                //gctUINT32  upper = (gctINT32)FV(threshold->upper);


                gcmONERROR(gcfVX_SetKernelArg(
                                kernelShader,
                                argIndex,
                                sizeof(gctUINT32),
                                &threshold->dataType));
                argIndex ++;

                gcmONERROR(gcfVX_SetKernelArg(
                                kernelShader,
                                argIndex,
                                sizeof(vx_pixel_value_t),
                                &threshold->value));

                argIndex ++;

                gcmONERROR(gcfVX_SetKernelArg(
                                kernelShader,
                                argIndex,
                                sizeof(vx_pixel_value_t),
                                &threshold->lower));

                argIndex ++;

                gcmONERROR(gcfVX_SetKernelArg(
                                kernelShader,
                                argIndex,
                                sizeof(vx_pixel_value_t),
                                &threshold->upper));

                argIndex ++;

                gcmONERROR(gcfVX_SetKernelArg(
                                kernelShader,
                                argIndex,
                                sizeof(vx_pixel_value_t),
                                &threshold->trueValue));

                argIndex ++;

                gcmONERROR(gcfVX_SetKernelArg(
                                kernelShader,
                                argIndex,
                                sizeof(vx_pixel_value_t),
                                &threshold->falseValue));

                argIndex ++;
            }
            else
            {
                argIndex += 6;
            }

            break;

        }
        case VX_TYPE_DISTRIBUTION:
        {
            vx_distribution distribution = (vx_distribution)parameters[i];

            if (distribution)
            {
                gctUINT32  rang = distribution->memory.dims[0][VX_DIM_X] * distribution->windowX;
                gctFLOAT   window = 1.0f/distribution->windowX;

                gcmONERROR(gcfVX_SetKernelArg(
                                kernelShader,
                                argIndex,
                                sizeof(gctUINT32),
                                &distribution->memory.dims[0][VX_DIM_X]));

                argIndex ++;

                gcmONERROR(gcfVX_SetKernelArg(
                                kernelShader,
                                argIndex,
                                sizeof(gctUINT32),
                                &rang));

                argIndex ++;

                gcmONERROR(gcfVX_SetKernelArg(
                                kernelShader,
                                argIndex,
                                sizeof(gctUINT32),
                                &distribution->offsetX));

                argIndex ++;

                gcmONERROR(gcfVX_SetKernelArg(
                                kernelShader,
                                argIndex,
                                sizeof(gctUINT32),
                                &window));

                argIndex ++;

                gcmONERROR(gcfVX_SetKernelArg(
                                kernelShader,
                                argIndex,
                                sizeof(gctUINT32),
                                &distribution->memory.physicals[0]));

                argIndex ++;
            }
            else
            {
                argIndex += 5;
            }

            break;
        }
        case VX_TYPE_REMAP:
        {
            vx_remap remap = (vx_remap)parameters[i];

            if (remap)
            {

                gcmONERROR(gcfVX_SetKernelArg(
                                kernelShader,
                                argIndex,
                                sizeof(gctUINT32),
                                &remap->destWidth));

                argIndex ++;

                gcmONERROR(gcfVX_SetKernelArg(
                                kernelShader,
                                argIndex,
                                sizeof(gctUINT32),
                                &remap->destHeight));

                argIndex ++;

                gcmONERROR(gcfVX_SetKernelArg(
                                kernelShader,
                                argIndex,
                                sizeof(gctUINT32),
                                &remap->memory.physicals[0]));

                argIndex ++;
            }
            else
            {
                argIndex += 3;
            }

            break;

        }
        case VX_TYPE_IMAGE:
        {

            vx_image image = (vx_image)parameters[i];

            if (image && image->memory.logicals[0])
            {
                gcmONERROR(gcfVX_SetKernelArg(
                        kernelShader,
                        argIndex,
                        sizeof(vx_reference),
                        &parameters[i]));
            }
            else
            {
                vx_reference nullRef = VX_NULL;

                gcmONERROR(gcfVX_SetKernelArg(
                        kernelShader,
                        argIndex,
                        sizeof(vx_reference),
                        &nullRef));
            }

            argIndex ++;

            break;
        }
        case VX_TYPE_OBJECT_ARRAY:
        {

            vx_object_array objArray = (vx_object_array)parameters[i];

            if (objArray && (objArray->itemType == VX_TYPE_IMAGE))
            {
                gcmONERROR(gcfVX_SetKernelArg(
                        kernelShader,
                        argIndex,
                        sizeof(vx_reference),
                        &parameters[i]));
            }

            argIndex ++;

            break;
        }
        case VX_TYPE_TENSOR:
        {

            vx_tensor tensor = (vx_tensor)parameters[i];

            if (tensor)
            {
                gcmONERROR(gcfVX_SetKernelArg(
                        kernelShader,
                        argIndex,
                        sizeof(vx_reference),
                        &parameters[i]));

                if (attribute & VXNNE_SHADER_PARAMETERS_ATTRIBUTE_ONE_COMPONENTS)
                {
                    vx_argument argument = gcfVX_GetKernelArg(kernelShader, argIndex, gcvNULL, gcvNULL, gcvNULL);
                    argument->components = 1;
                }
                else if (attribute & VXNNE_SHADER_PARAMETERS_ATTRIBUTE_TWO_COMPONENTS)
                {
                    vx_argument argument = gcfVX_GetKernelArg(kernelShader, argIndex, gcvNULL, gcvNULL, gcvNULL);
                    argument->components = 2;
                }
                else if (attribute & VXNNE_SHADER_PARAMETERS_ATTRIBUTE_THREE_COMPONENTS)
                {
                    vx_argument argument = gcfVX_GetKernelArg(kernelShader, argIndex, gcvNULL, gcvNULL, gcvNULL);
                    argument->components = 3;
                }
                else if (attribute & VXNNE_SHADER_PARAMETERS_ATTRIBUTE_FOUR_COMPONENTS)
                {
                    vx_argument argument = gcfVX_GetKernelArg(kernelShader, argIndex, gcvNULL, gcvNULL, gcvNULL);
                    argument->components = 4;
                }

                if (attribute & VXNNE_SHADER_PARAMETERS_ATTRIBUTE_NO_BATCH_BIT)
                {
                    vx_argument argument = gcfVX_GetKernelArg(kernelShader, argIndex, gcvNULL, gcvNULL, gcvNULL);
                    argument->noBatch = gcvTRUE;
                }
                else
                {
                    vx_argument argument = gcfVX_GetKernelArg(kernelShader, argIndex, gcvNULL, gcvNULL, gcvNULL);
                    argument->noBatch = gcvFALSE;

                }
            }

            argIndex ++;

            break;
        }
        case VX_TYPE_PYRAMID:
        {
            gctUINT32 j = 0;
            vx_size maxLevel = 10;

            vx_pyramid pyramid = (vx_pyramid)parameters[i];

            if (pyramid)
            {

                gcmONERROR(gcfVX_SetKernelArg(
                                    kernelShader,
                                    argIndex,
                                    sizeof(gctUINT32),
                                    &pyramid->scale));

                argIndex ++;

                gcmONERROR(gcfVX_SetKernelArg(
                                    kernelShader,
                                    argIndex,
                                    sizeof(gctUINT32),
                                    &pyramid->width));

                argIndex ++;

                gcmONERROR(gcfVX_SetKernelArg(
                                    kernelShader,
                                    argIndex,
                                    sizeof(gctUINT32),
                                    &pyramid->height));

                argIndex ++;

                gcmONERROR(gcfVX_SetKernelArg(
                                    kernelShader,
                                    argIndex,
                                    sizeof(gctUINT32),
                                    &pyramid->format));

                argIndex ++;

                gcmONERROR(gcfVX_SetKernelArg(
                                    kernelShader,
                                    argIndex,
                                    sizeof(vx_size),
                                    &pyramid->levelCount));

                argIndex ++;

                maxLevel = gcmMIN(pyramid->levelCount, maxLevel);

                for (j = 0; j < (gctUINT32)maxLevel; j++)
                {
                    gcmONERROR(gcfVX_SetKernelArg(
                                    kernelShader,
                                    argIndex + j,
                                    sizeof(vx_image),
                                    &pyramid->levels[j]));
                }

                /* default is 10, need to match with the compiler option -cl-viv-vx-image-array-maxlevel=n */
                argIndex += 10;

            }
            else
            {
                argIndex += 15;
            }

            break;
        }
        default:
            goto OnError;
        }
    }
    gcmFOOTER_ARG("%d", VX_SUCCESS);
    return VX_SUCCESS;

OnError:
    gcmFOOTER_ARG("%d", VX_FAILURE);
    return VX_FAILURE;

}

VX_INTERNAL_API vx_status vxoShader_Execute(
    vx_node   node,
    vx_shader kernelShader,
    vx_border_mode_t *borderMode,
    vx_kernel_execution_parameters_t *shaderParameter,
    vx_uint32 batchID
    )
{
    gceSTATUS status;
    vx_size   workGroupSize = 1;
    vx_uint32           i;

    vx_kernel_execution_parameters_t newShaderParameter = *shaderParameter;

    gcmHEADER_ARG("node=%p, kernelShader=%p, borderMode=%p, shaderParameter=%p, batchID=0x%x", node, kernelShader, borderMode, shaderParameter, batchID);

    /* adjust the localworksize while it is unset */
    gcmONERROR(gcfVX_AdjustLocalWorkSize(kernelShader,
                              newShaderParameter.workDim,
                              newShaderParameter.globalWorkOffset,
                              newShaderParameter.globalWorkSize,
                              newShaderParameter.localWorkSize));

    /* validate the arguments */
    for (i = 0; i < newShaderParameter.workDim; i++)
    {
        if(newShaderParameter.globalWorkSize[i] == 0)
        {
            goto OnError;
        }

        if (newShaderParameter.globalWorkSize[i] > kernelShader->maxGlobalWorkSize)
        {
            goto OnError;
        }

        if ((newShaderParameter.globalWorkSize[i] + newShaderParameter.globalWorkOffset[i]) > kernelShader->maxGlobalWorkSize)
        {
            goto OnError;
        }


        if((newShaderParameter.globalWorkSize[i] % newShaderParameter.localWorkSize[i]) != 0)
        {
            goto OnError;
        }

        if (newShaderParameter.localWorkSize[i] > kernelShader->maxWorkItemSizes[i])
        {
            goto OnError;
        }

        workGroupSize *= newShaderParameter.localWorkSize[i];

    }

    if (workGroupSize > kernelShader->maxWorkGroupSize) goto OnError;

    gcmONERROR(gcfVX_ExecuteKernel(kernelShader,
                                   kernelShader->numArgs,
                                   kernelShader->args,
                                   borderMode,
                                   batchID,
                                   newShaderParameter.workDim,
                                   newShaderParameter.globalWorkOffset,
                                   newShaderParameter.globalWorkScale,
                                   newShaderParameter.globalWorkSize,
                                   newShaderParameter.localWorkSize
                                   ));


    gcmFOOTER_ARG("%d", VX_SUCCESS);
    return VX_SUCCESS;

OnError:
    gcmFOOTER_ARG("%d", VX_FAILURE);
    return VX_FAILURE;
}

VX_INTERNAL_API vx_status vxoProgramKernel_GetCurrentShaderID(vx_node node, gctUINT *currentShaderID)
{
    vx_uint32           i;
    gctUINT             shaderID;
    gctCHAR             kernelName[256] = {0};
    vx_char             kernelMainName[128] = {0};

    gcmHEADER_ARG("node=%p, currentShaderID=%p", node, currentShaderID);

    if (currentShaderID == NULL)
    {
        gcmFOOTER_ARG("%d", VX_FAILURE);
        return VX_FAILURE;
    }
    gcoOS_StrCopySafe(kernelName, 256, _getShaderName(node->kernel->name, kernelMainName));
    gcoOS_StrCatSafe(kernelName, 256, node->kernel->subname);

    for(i = 0; i < node->kernel->kernelShaderCount; i++)
    {
        if (gcoOS_StrCmp(node->kernel->kernelShader[i*2]->name, kernelName) == 0)
            break;
    }

    if (i == node->kernel->kernelShaderCount)
    {
        gcmFOOTER_ARG("%d", VX_FAILURE);
        return VX_FAILURE;
    }
    shaderID = ((node->kernelAttributes.borderMode.mode == VX_BORDER_MODE_CONSTANT) ? 1 : 0);

    if (currentShaderID)
        *currentShaderID = i*2 + shaderID;
    gcmFOOTER_ARG("%d", VX_SUCCESS);
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoProgramKernel_Function(vx_node node, const vx_reference parameters[], vx_uint32 paramCount)
{
    vx_uint32           i;
    gctUINT             currentShaderID = 0;
    vx_shader           kernelShader;
    vx_graph            graph = node->graph;
    vx_status           status = VX_FAILURE;
    vx_uint64           perfStart = 0;
    gctUINT8            *stateBuffer = VX_NULL;
    gctUINT32           actualSize = 0;

    gcmHEADER_ARG("node=%p, parameters=%p, paramCount=0x%x", node, parameters, paramCount);

    if (node->base.context->options.enableCNNPerf)
    {
        vxInfo("layer id: %d layer name: %s\n", node->id, (node->layer && node->layer->name) ? node->layer->name : "UserNode");

        perfStart = gcfVX_PerfStart((vx_reference)node);
    }

    status = vxoProgramKernel_GetCurrentShaderID(node, &currentShaderID);
    if (status != VX_SUCCESS) goto OnError;

    kernelShader = node->kernel->kernelShader[currentShaderID];
    node->kernel->currShaderID = currentShaderID;

    status = vxoShader_SetParameters(kernelShader, (vx_reference*)parameters, paramCount, node->kernel->signature.dataTypeTable, VX_NULL);
    if (status != VX_SUCCESS) goto OnError;

    for(i = 0; i < node->uniformCount; i++)
    {
        status = vxoShader_SetUniform(kernelShader, node->uniforms[i].name, node->uniforms[i].count, node->uniforms[i].data);
        if (status != VX_SUCCESS) goto OnError;

    }

    if (graph->binarySave)
    {
        vxmONERROR(gcoOS_Allocate(gcvNULL, VX_MAX_SH_OPERATION_STATE_SIZE, (gctPOINTER *)&stateBuffer));
        status = gcfVX_CaptureState(stateBuffer,
                                    VX_MAX_SH_OPERATION_STATE_SIZE,
                                    gcvNULL,
                                    gcvTRUE, gcvFALSE);
    }

    status = vxoShader_Execute(node,
                               kernelShader,
                               &node->kernelAttributes.borderMode,
                               &node->kernelAttributes.shaderParameter,
                               0);

    if (status != VX_SUCCESS) goto OnError;

    if (graph->binarySave)
    {
        status = gcfVX_CaptureState(gcvNULL, 0, &actualSize, gcvFALSE, gcvFALSE);
        gcmASSERT(actualSize);
        if (actualSize <= 0)
        {
            vxError("error: fail to save layer name : %s to binary in shader operation\n", node->layer->name);
        }
        /* one node just to support an operation for vxc shader*/
        vxmONERROR(vxoGraphBinary_SaveShaderOperation(node, node->layer->operations[0], kernelShader,
                                                    (vx_reference*)parameters, paramCount,
                                                    stateBuffer, actualSize, 0));
    }

    if (node->base.context->options.enableCNNPerf)
    {
        gcfVX_Flush(gcvTRUE);

        vxInfo("%s execution time:%10d us\n",
                 (node->layer && node->layer->name) ? node->layer->name : "UserNode",
                 gcfVX_PerfEnd((vx_reference)node, perfStart));
    }

#if gcdDUMP && gcdDUMP_PER_OPERATION
    /* if dump per operation, need to copy gpu output data to cpu buffer mapped by user memory handle*/
    for(i = 0; i < paramCount; i++)
    {
        vx_enum direction, type;
        direction = node->kernel->signature.directionTable[i];
        type = node->kernel->signature.dataTypeTable[i];
        if(type == VX_TYPE_IMAGE && (direction == VX_OUTPUT || direction == VX_BIDIRECTIONAL))
        {
            vx_rectangle_t rect;
            vx_image image = (vx_image)node->paramTable[i];
            vx_uint32 plane = 0;

            if(image->importType != VX_MEMORY_TYPE_HOST || image->useInternalMem == vx_false_e)
            {
                continue;
            }

            gcoVX_Flush(gcvTRUE);

            vxGetValidRegionImage(image, &rect);

            for (plane = 0; plane < image->memory.planeCount; plane++)
            {
                if (image->memory.nodePtrs[plane] != VX_NULL && image->memory.logicals[plane] != image->memory.nodePtrs[plane]->logical)
                {
                    vx_size size = 0;
                    size = vxComputeWholeImageSize(image, &rect, plane);
                    /*Only copy different memory. For CTS GraphROI.Simple */
                    if (size > 0 && (abs((vx_int32)(gcmALL_TO_UINT32(image->memory.logicals[plane]) - gcmALL_TO_UINT32(image->memory.nodePtrs[plane]->logical))) > (vx_int32)size))
                        gcoOS_MemCopy(image->memory.logicals[plane], image->memory.nodePtrs[plane]->logical, size);
                }
            }
        }
        else if(type == VX_TYPE_TENSOR && (direction == VX_OUTPUT || direction == VX_BIDIRECTIONAL))
        {
            vx_tensor tensor = (vx_tensor)node->paramTable[i];

            if(tensor->useInternalMem == vx_true_e && tensor->tensorBuffer->memory.wrapFlag == gcvALLOC_FLAG_USERMEMORY)
            {
                gcoVX_Flush(gcvTRUE);
                gcoOS_MemCopy(tensor->tensorBuffer->memory.logicals[0], tensor->tensorBuffer->memory.nodePtrs[0]->logical, tensor->tensorBuffer->memory.nodePtrs
[0]->size);
            }
        }
    }
    gcfVX_Flush(gcvTRUE);

    vxoDumpOutput(node, parameters, paramCount);
#endif

OnError:
    if (stateBuffer != VX_NULL)
    {
        gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, stateBuffer));
    }
    gcmFOOTER_ARG("%d", status);
    return status;
}

VX_INTERNAL_API vx_status vxoShader_SetCLParameters(vx_shader kernelShader, vx_reference parameters[], vx_uint32 paramCount, vx_enum dataTypeTable[], vx_enum paramAttributes[], vx_bool tensorVxcOptimize)
{
    gceSTATUS status;
    gctUINT   argIndex = 0;
    vx_uint32 i;

    gcmHEADER_ARG("kernelShader=%p, parameters=%p, paramCount=0x%x, dataTypeTable=%p, paramAttributes=%p, tensorVxcOptimize=0x%x",
        kernelShader, parameters, paramCount, dataTypeTable, paramAttributes, tensorVxcOptimize);

    for (i = 0; i < paramCount; i++)
    {
        vx_type_e type = (vx_type_e)((parameters[i] != gcvNULL) ? parameters[i]->type : dataTypeTable[i]);
        vx_enum   attribute = paramAttributes ? paramAttributes[i] : VXNNE_SHADER_PARAMETERS_ATTRIBUTE_NONE_BIT;

        switch(type)
        {
        case VX_TYPE_SCALAR:
        {
            gctBOOL isPointer;
            vx_scalar scalar = (vx_scalar)parameters[i];
            vx_argument argument = gcfVX_GetKernelArg(kernelShader, argIndex, gcvNULL, gcvNULL, gcvNULL);

            if (scalar)
            {
                gcmONERROR(gcUNIFORM_GetFormat(argument->uniform, gcvNULL, &isPointer));

                if (!isPointer)
                {
                    gcmONERROR(gcfVX_SetKernelArg(
                                kernelShader,
                                argIndex,
                                /*sizeof(gctUINT32),*/ vxDataType_GetSize((vx_type_e)scalar->dataType),
                                scalar->value));
                }
                else
                {
                    gcmONERROR(gcfVX_SetKernelArg(
                                kernelShader,
                                argIndex,
                                sizeof(gctUINT32),
                                &scalar->physical));
                }

                argIndex++;
            }
            else
            {
                argIndex++;
            }
            break;
        }
        case VX_TYPE_LUT:
        case VX_TYPE_ARRAY:
        {
            vx_array array = (vx_array)parameters[i];

            if (array)
            {

                gcmONERROR(gcfVX_SetKernelArg(
                                kernelShader,
                                argIndex,
                                sizeof(gctUINT32),
                                &array->memory.physicals[0]));
                argIndex ++;
            }
            else
            {
                argIndex +=1;
            }
            break;
        }
        case VX_TYPE_CONVOLUTION:
        {
            vx_convolution conv = (vx_convolution)parameters[i];
            if (conv)
            {

                gcmONERROR(gcfVX_SetKernelArg(
                                kernelShader,
                                argIndex,
                                sizeof(gctUINT32),
                                &conv->matrix.memory.physicals[0]));
                argIndex ++;

            }
            else
            {
                argIndex += 1;
            }
            break;
        }
        case VX_TYPE_MATRIX:
        {
            vx_matrix matrix = (vx_matrix)parameters[i];
            vxReadMatrix(matrix, NULL);

            if (matrix)
            {
                gcmONERROR(gcfVX_SetKernelArg(
                                kernelShader,
                                argIndex,
                                sizeof(gctUINT32),
                                &matrix->memory.physicals[0]));
                argIndex ++;
            }
            else
            {
                argIndex += 1;
            }
            break;
        }
        case VX_TYPE_THRESHOLD:
        {
            vx_threshold threshold = (vx_threshold)parameters[i];
            if (threshold)
            {
                gcmONERROR(gcfVX_SetKernelArg(
                                kernelShader,
                                argIndex,
                                sizeof(gctUINT32),
                                &threshold->dataType));
                argIndex ++;
            }
            else
            {
                argIndex += 1;
            }

            break;
        }
        case VX_TYPE_DISTRIBUTION:
        {
            vx_distribution distribution = (vx_distribution)parameters[i];

            if (distribution)
            {
                //gctUINT32  rang = distribution->memory.dims[0][VX_DIM_X] * distribution->windowX;

                gcmONERROR(gcfVX_SetKernelArg(
                                kernelShader,
                                argIndex,
                                sizeof(gctUINT32),
                                &distribution->memory.physicals[0]));

                argIndex ++;
            }
            else
            {
                argIndex += 1;
            }

            break;
        }
        case VX_TYPE_REMAP:
        {
            vx_remap remap = (vx_remap)parameters[i];

            if (remap)
            {
                gcmONERROR(gcfVX_SetKernelArg(
                                kernelShader,
                                argIndex,
                                sizeof(gctUINT32),
                                &remap->memory.physicals[0]));

                argIndex ++;
            }
            else
            {
                argIndex += 1;
            }

            break;

        }
        case VX_TYPE_IMAGE:
        {

            vx_image image = (vx_image)parameters[i];

            if (image && image->memory.logicals[0])
            {
                gcmONERROR(gcfVX_SetKernelArg(
                        kernelShader,
                        argIndex,
                        sizeof(vx_reference),
                        &parameters[i]));
            }
            else
            {
                vx_reference nullRef = VX_NULL;

                gcmONERROR(gcfVX_SetKernelArg(
                        kernelShader,
                        argIndex,
                        sizeof(vx_reference),
                        &nullRef));
            }

            argIndex ++;

            break;
        }
        case VX_TYPE_OBJECT_ARRAY:
        {
            break;
        }
        case VX_TYPE_TENSOR:
        {
            if (tensorVxcOptimize == vx_true_e)
            {
                vx_tensor tensor = (vx_tensor)parameters[i];

                if (tensor)
                {
                    gcmONERROR(gcfVX_SetKernelArg(
                            kernelShader,
                            argIndex,
                            sizeof(vx_reference),
                            &parameters[i]));

                    if (attribute & VXNNE_SHADER_PARAMETERS_ATTRIBUTE_NO_BATCH_BIT)
                    {
                        vx_argument argument = gcfVX_GetKernelArg(kernelShader, argIndex, gcvNULL, gcvNULL, gcvNULL);
                        argument->noBatch = gcvTRUE;
                    }
                    else
                    {
                        vx_argument argument = gcfVX_GetKernelArg(kernelShader, argIndex, gcvNULL, gcvNULL, gcvNULL);
                        argument->noBatch = gcvFALSE;

                    }
                }
            }
            else
            {
                vx_tensor tensor = (vx_tensor)parameters[i];

                if (tensor)
                {
                    gctUINT32 addr = tensor->baseAddressOffset + tensor->tensorBuffer->memory.physicals[0];
                    gcmONERROR(gcfVX_SetKernelArg(
                            kernelShader,
                            argIndex,
                            sizeof(gctUINT32),
                            &addr));
                }
            }

            argIndex ++;

            break;
        }
        case VX_TYPE_PYRAMID:
        {
            break;
        }
        default:
            goto OnError;
        }
    }
    gcmFOOTER_ARG("%d", VX_SUCCESS);
    return VX_SUCCESS;

OnError:
    gcmFOOTER_ARG("%d", VX_FAILURE);
    return VX_FAILURE;

}

VX_INTERNAL_API vx_status VX_CALLBACK vxoProgramKernel_FunctionVX(vx_node node, const vx_reference parameters[], vx_uint32 paramCount)
{
    vx_uint32           i;
    gctUINT             currentShaderID = 0;
    vx_shader           kernelShader;
    vx_graph            graph = node->graph;
    vx_status           status = VX_FAILURE;
    vx_uint64           perfStart = 0;
    gctUINT8            *stateBuffer = VX_NULL;
    gctUINT32           actualSize = 0;

    gcmHEADER_ARG("node=%p, parameters=%p, paramCount=0x%x", node, parameters, paramCount);

    if (node->base.context->options.enableCNNPerf)
    {
        vxInfo("layer id: %d layer name: %s\n", node->id, (node->layer && node->layer->name) ? node->layer->name : "UserNode");

        perfStart = gcfVX_PerfStart((vx_reference)node);
    }

    status = vxoProgramKernel_GetCurrentShaderID(node, &currentShaderID);
    if (status != VX_SUCCESS) goto OnError;

    kernelShader = node->kernel->kernelShader[currentShaderID];
    node->kernel->currShaderID = currentShaderID;

    status = vxoShader_SetCLParameters(kernelShader, (vx_reference*)parameters, paramCount, node->kernel->signature.dataTypeTable, VX_NULL, node->tensorVxcOptimize);
    if (status != VX_SUCCESS) goto OnError;

    for(i = 0; i < node->uniformCount; i++)
    {
        status = vxoShader_SetUniform(kernelShader, node->uniforms[i].name, node->uniforms[i].count, node->uniforms[i].data);
        if (status != VX_SUCCESS) goto OnError;

    }

    if (graph->binarySave)
    {
        vxmONERROR(gcoOS_Allocate(gcvNULL, VX_MAX_SH_OPERATION_STATE_SIZE, (gctPOINTER *)&stateBuffer));
        status = gcfVX_CaptureState(stateBuffer,
                                    VX_MAX_SH_OPERATION_STATE_SIZE,
                                    gcvNULL,
                                    gcvTRUE, gcvFALSE);
    }

    status = vxoShader_Execute(node,
                               kernelShader,
                               &node->kernelAttributes.borderMode,
                               &node->kernelAttributes.shaderParameter,
                               0);

    if (status != VX_SUCCESS) goto OnError;

    if (graph->binarySave)
    {
        status = gcfVX_CaptureState(gcvNULL, 0, &actualSize, gcvFALSE, gcvFALSE);
        gcmASSERT(actualSize);
        if (actualSize <= 0)
        {
            vxError("error: fail to save layer name : %s to binary in shader operation\n", node->layer->name);
        }
        /* one node just to support an operation for vxc shader*/
        vxmONERROR(vxoGraphBinary_SaveShaderOperation(node, node->layer->operations[0], kernelShader,
                                                    (vx_reference*)parameters, paramCount,
                                                    stateBuffer, actualSize, 0));
    }

    if (node->base.context->options.enableCNNPerf)
    {
        gcfVX_Flush(gcvTRUE);

        vxInfo("%s execution time:%10d us\n",
                 (node->layer && node->layer->name) ? node->layer->name : "UserNode",
                 gcfVX_PerfEnd((vx_reference)node, perfStart));
    }

#if gcdDUMP && gcdDUMP_PER_OPERATION
    /* if dump per operation, need to copy gpu output data to cpu buffer mapped by user memory handle*/
    for(i = 0; i < paramCount; i++)
    {
        vx_enum direction, type;
        direction = node->kernel->signature.directionTable[i];
        type = node->kernel->signature.dataTypeTable[i];
        if(type == VX_TYPE_IMAGE && (direction == VX_OUTPUT || direction == VX_BIDIRECTIONAL))
        {
            vx_rectangle_t rect;
            vx_image image = (vx_image)node->paramTable[i];
            vx_uint32 plane = 0;

            if(image->importType != VX_MEMORY_TYPE_HOST || image->useInternalMem == vx_false_e)
            {
                continue;
            }

            gcoVX_Flush(gcvTRUE);

            vxGetValidRegionImage(image, &rect);

            for (plane = 0; plane < image->memory.planeCount; plane++)
            {
                if (image->memory.nodePtrs[plane] != VX_NULL && image->memory.logicals[plane] != image->memory.nodePtrs[plane]->logical)
                {
                    vx_size size = 0;
                    size = vxComputeWholeImageSize(image, &rect, plane);
                    /*Only copy different memory. For CTS GraphROI.Simple */
                    if (size > 0 && (abs((vx_int32)(gcmALL_TO_UINT32(image->memory.logicals[plane]) - gcmALL_TO_UINT32(image->memory.nodePtrs[plane]->logical))) > (vx_int32)size))
                        gcoOS_MemCopy(image->memory.logicals[plane], image->memory.nodePtrs[plane]->logical, size);
                }
            }
        }
        else if(type == VX_TYPE_TENSOR && (direction == VX_OUTPUT || direction == VX_BIDIRECTIONAL))
        {
            vx_tensor tensor = (vx_tensor)node->paramTable[i];

            if(tensor->useInternalMem == vx_true_e && tensor->tensorBuffer->memory.wrapFlag == gcvALLOC_FLAG_USERMEMORY)
            {
                gcoVX_Flush(gcvTRUE);
                gcoOS_MemCopy(tensor->tensorBuffer->memory.logicals[0], tensor->tensorBuffer->memory.nodePtrs[0]->logical, tensor->tensorBuffer->memory.nodePtrs
[0]->size);
            }
        }
    }
    gcfVX_Flush(gcvTRUE);

    vxoDumpOutput(node, parameters, paramCount);
#endif

OnError:
    if (stateBuffer != VX_NULL)
    {
        gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, stateBuffer));
    }

    gcmFOOTER_ARG("%d", status);
    return status;
}

VX_API_ENTRY vx_status VX_CALLBACK vxProgramKernel_Function(vx_node node, const vx_reference parameters[], vx_uint32 paramCount)
{
    gcmHEADER_ARG("node=%p, parameters=%p, paramCount=0x%x", node, parameters, paramCount);
    gcmDUMP_API("$VX vxProgramKernel_Function: node=%p, parameters=%p, paramCount=0x%x", node, parameters, paramCount);
    gcmFOOTER_NO();
    return vxoProgramKernel_Function(node, parameters, paramCount);
}

/*unused code*/
VX_PRIVATE_API vx_status VX_CALLBACK vxoProgramKernel_Initialize(vx_node node, const vx_reference parameters[], vx_uint32 paramCount)
{
    /* TODO */

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoProgramKernel_Deinitialize(vx_node node, const vx_reference parameters[], vx_uint32 paramCount)
{
    /* TODO */

    return VX_SUCCESS;
}

VX_API_ENTRY vx_kernel VX_API_CALL vxAddKernelInProgramEx(
        vx_program program, vx_char name[VX_MAX_KERNEL_NAME], vx_enum enumeration, vx_kernel_f func_ptr,
        vx_uint32 num_params, vx_kernel_validate_f validate, vx_kernel_input_validate_f input,
        vx_kernel_output_validate_f output, vx_kernel_initialize_f initialize, vx_kernel_deinitialize_f deinitialize)
{
    vx_context  context;
    vx_uint32   targetIndex;
    vx_char     targetName[VX_MAX_TARGET_NAME] = VX_DEFAULT_TARGET_NAME;

    gcmHEADER_ARG("program=%p, name=%s, enumeration=0x%x, func_ptr=%p, num_params=0x%x, validate=%p, input=%p, output=%p, initialize=%p, deinitialize=%p",
        program, name, enumeration, func_ptr, num_params, validate, input, output, initialize, deinitialize);
    gcmDUMP_API("$VX vxAddKernelInProgramEx: program=%p, name=%s, enumeration=0x%x, func_ptr=%p, num_params=0x%x, validate=%p, input=%p, output=%p, initialize=%p, deinitialize=%p",
        program, name, enumeration, func_ptr, num_params, validate, input, output, initialize, deinitialize);


    if (!vxoReference_IsValidAndSpecific(&program->base, (vx_type_e)VX_TYPE_PROGRAM))
    {
        gcmFOOTER_NO();
        return VX_NULL;
    }
    context = program->base.context;

    if (name == VX_NULL || strlen(name) == 0) goto ErrorExit;

    for (targetIndex = 0; targetIndex < context->targetCount; targetIndex++)
    {
        vx_target target = &context->targetTable[targetIndex];

        if (vxIsSameString(targetName, target->name, VX_MAX_TARGET_NAME))
        {
            vx_kernel kernel;
            vxmASSERT(target->funcs.addkernel);

            kernel = target->funcs.addkernel(target, name, enumeration,
                                            program,
                                            (func_ptr != VX_NULL) ? func_ptr:vxoProgramKernel_Function,
                                            num_params, validate,
                                            input, output,
                                            (initialize != VX_NULL)?initialize:vxoProgramKernel_Initialize,
                                            (deinitialize != VX_NULL)?deinitialize:vxoProgramKernel_Deinitialize);

            kernel->attributes.isAllGPU = vx_true_e;

            kernel->isUserkernel = vx_true_e;

            gcmFOOTER_ARG("kernel=%p", kernel);
            return kernel;
        }
    }

    vxError("Faild to find target \"%s\" for vxAddKernelInProgram", targetName);

ErrorExit:
    gcmFOOTER_NO();
    return (vx_kernel)vxoContext_GetErrorObject(context, VX_ERROR_INVALID_PARAMETERS);
}

VX_API_ENTRY vx_kernel VX_API_CALL vxAddKernelInProgram(
        vx_program program, vx_char name[VX_MAX_KERNEL_NAME], vx_enum enumeration, vx_uint32 num_params, vx_kernel_validate_f validate,
        vx_kernel_initialize_f initialize, vx_kernel_deinitialize_f deinitialize)
{
    gcmHEADER_ARG("program=%p, name=%s, enumeration=0x%x, num_params=0x%x, validate=%p, initialize=%p, deinitialize=%p",
        program, name, enumeration, num_params, validate, initialize, deinitialize);
    gcmDUMP_API("$VX vxAddKernelInProgram: program=%p, name=%s, enumeration=0x%x, num_params=0x%x, validate=%p, initialize=%p, deinitialize=%p",
        program, name, enumeration, num_params, validate, initialize, deinitialize);

    gcmFOOTER_NO();
    return vxAddKernelInProgramEx(program, name, enumeration, vxoProgramKernel_Function, num_params, validate, VX_NULL, VX_NULL, initialize, deinitialize);
}

VX_API_ENTRY vx_status VX_API_CALL vxSelectKernelSubname(vx_node node, const vx_char * subname)
{
    gcmHEADER_ARG("node=%p, subname=%s", node, subname);
    gcmDUMP_API("$VX vxSelectKernelSubname: node=%p, subname=%s", node, subname);

    gcoOS_StrCopySafe(node->kernel->subname, VX_MAX_KERNEL_NAME, subname);

    gcmFOOTER_ARG("%d", VX_SUCCESS);
    return VX_SUCCESS;
}

VX_API_ENTRY vx_status VX_API_CALL vxSetNodeUniform(vx_node node, const vx_char * name, vx_size count, void * value)
{
    vx_uint32 size;
    vx_status vStatus = VX_FAILURE;
    gceSTATUS status;

    gcmHEADER_ARG("node=%p, name=%s, count=0x%lx, value=%p", node, name, count, value);
    gcmDUMP_API("$VX vxSetNodeUniform: node=%p, name=%s, count=0x%lx, value=%p", node, name, count, value);

    if (node->kernel->kernelShader[0] && (node->uniformCount >= node->kernel->kernelShader[0]->numArgs)) goto error;

    if (!node->uniforms)
    {
        /*allocat the maximum number uniforms */
        status = gcoOS_Allocate(gcvNULL, node->kernel->kernelShader[0]->numArgs * gcmSIZEOF(vx_node_s), (gctPOINTER*)&node->uniforms);
        if (gcmIS_ERROR(status))
        {
            vStatus = VX_FAILURE;
            goto error;
        }
    }


    vStatus = vxoShader_GetUniformSize(node->kernel->kernelShader[0], (vx_char*)name, &size);
    if (vStatus != VX_SUCCESS) goto error;

    status = gcoOS_Allocate(gcvNULL, size, (gctPOINTER*)&node->uniforms[node->uniformCount].data);
    if (gcmIS_ERROR(status))
    {
        vStatus = VX_FAILURE;
        goto error;
    }

    gcoOS_MemCopy(node->uniforms[node->uniformCount].data, value, size);

    node->uniforms[node->uniformCount].count = (vx_uint32)count;
    node->uniforms[node->uniformCount].name  = (vx_char*)name;
    node->uniforms[node->uniformCount].size  = size;

    node->uniformCount++;

error:
    gcmFOOTER_ARG("%d", vStatus);
    return vStatus;
}

gceSTATUS
gcfVX_FreeKernelArgs(
    gctUINT         NumArgs,
    vx_argument     Args,
    gctBOOL         FreeAllocData
    )
{
    gctUINT         i;

    gcmHEADER_ARG("NumArgs=0x%x, Args=%p, FreeAllocData=0x%x", NumArgs, Args, FreeAllocData);

    if (Args == gcvNULL || NumArgs == 0)
    {
        gcmFOOTER_ARG("%d", gcvSTATUS_OK);
        return gcvSTATUS_OK;
    }

    for (i = 0; i < NumArgs; i++)
    {
        if (Args[i].isMemAlloc)
        {
            vx_mem_alloc_info memAllocInfo = (vx_mem_alloc_info) Args[i].data;
            gcoVX_FreeMemory(memAllocInfo->node);
            if (FreeAllocData && memAllocInfo->data)
                gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, memAllocInfo->data));
        }

        if (Args[i].data)
        {
            gcmOS_SAFE_FREE(gcvNULL, Args[i].data);
        }
    }
    gcmOS_SAFE_FREE(gcvNULL, Args);
    gcmFOOTER_ARG("%d", gcvSTATUS_OK);
    return gcvSTATUS_OK;
}


VX_INTERNAL_API vx_status vxoShader_Free(vx_shader kernel)
{
    gcmHEADER_ARG("kernel=%p", kernel);
    if (kernel)
    {
        gcfVX_FreeKernelArgs(kernel->numArgs, kernel->args, gcvTRUE);

        gcFreeProgramState(kernel->states.programState);
        if (kernel->states.binary) gcSHADER_Destroy((gcSHADER)kernel->states.binary);
        if (kernel->name) gcoOS_Free(gcvNULL, kernel->name);

        gcoOS_Free(gcvNULL, kernel);
    }

    gcmFOOTER_ARG("%d", gcvSTATUS_OK);
    return gcvSTATUS_OK;
}

VX_INTERNAL_CALLBACK_API void vxoKernel_Destructor(vx_reference ref)
{
    vx_kernel vKernel = (vx_kernel)ref;

    gctUINT i;

    gcmHEADER_ARG("ref=%p", ref);

    for (i = 0; i < vKernel->kernelShaderCount*2; i++)
    {
        vxoShader_Free(vKernel->kernelShader[i]);
    }

    if (vKernel->kernelShader) gcoOS_Free(gcvNULL, vKernel->kernelShader);

    gcmFOOTER_NO();
}

VX_INTERNAL_API vx_status vxoKernel_ProcessKernelShaderPrint(vx_shader shader, vx_kernel_execution_parameters_t* shaderParameter)
{
    gcmHEADER_ARG("shader=%p, shaderParameter=%p", shader, shaderParameter);
    if (shader)
    {
        gctUINT i, j;
        char *fmt;

        gctUINT totalNumItems  =
            (gctUINT)(shaderParameter->globalWorkSize[0]
            * (shaderParameter->workDim > 1 ? shaderParameter->globalWorkSize[1] : 1)
            * (shaderParameter->workDim > 2 ? shaderParameter->globalWorkSize[2] : 1));

        fmt = shader->constantMemBuffer;
        for (i = 0; i < shader->numArgs; i++)
        {
            if (shader->args[i].isMemAlloc)
            {
                vx_mem_alloc_info memAllocInfo = (vx_mem_alloc_info)shader->args[i].data;
                if (strcmp(shader->args[i].uniform->name, "#printf_address") == 0)
                {
                    gctCHAR * printfAddr = (gctCHAR*)memAllocInfo->logical;
                    gctUINT* dataAddress = gcvNULL;
                    gctUINT  offset = 0;
                    for(j = 0; j < totalNumItems; j++)
                    {
                        dataAddress =(gctUINT*)printfAddr;

                        do
                        {
                            gctINT writeMaskSig1 = *(gctINT*)dataAddress;
                            gctINT writeMaskSig2 = *((gctINT*)dataAddress + 1);

                            /* Still printf left in this thread. */
                            if (writeMaskSig1 == __OCL_PRINTF_WRITE_SIG1__ &&
                                writeMaskSig2 == __OCL_PRINTF_WRITE_SIG2__)
                            {
                                dataAddress += 2;
                                offset = *dataAddress;
                                dataAddress++;
                                dataAddress = (gctUINT*)gcfVX_PrintParseData(&fmt[offset], (gctPOINTER)dataAddress);
                            }
                            else
                            {
                                break;
                            }
                        }
                        while(gcmPTR_TO_UINT64(dataAddress) < ((gcmPTR_TO_UINT64(printfAddr) + (VX_MAX_PRINTF_BUFFER_SIZE/totalNumItems))));
                        printfAddr += (VX_MAX_PRINTF_BUFFER_SIZE/totalNumItems);
                    }
                }
            }
        }
    }
    gcmFOOTER_ARG("%d", VX_SUCCESS);
    return VX_SUCCESS;
}

VX_INTERNAL_API vx_status vxoShader_SetUniform(vx_shader shader, vx_char * name, vx_size count, void * value)
{
    vx_argument         arg;
    gctUINT             i;
    gceSTATUS           status;
    gctUINT             length;
    vx_status           vStatus = VX_FAILURE;

    gcmHEADER_ARG("shader=%p, name=%s, count=0x%lx, value=%p", shader, name, count, value);

    for (i = 0; i < shader->numArgs; i++)
    {
        arg = &shader->args[i];

        if (arg->uniform == gcvNULL) continue;

        if ((gcoOS_StrCmp(arg->uniform->name, name) == gcvSTATUS_OK))
        {
            gcmONERROR(gcUNIFORM_GetType(arg->uniform, gcvNULL, &length));

            if (length != count) break;

            gcoOS_MemCopy(arg->data, value, arg->size);

            vStatus = VX_SUCCESS;
            break;
        }
    }

OnError:
    gcmFOOTER_ARG("%d", vStatus);
    return vStatus;
}


VX_API_ENTRY vx_status VX_API_CALL vxUnloadKernels(vx_context context, const vx_char *name)
{
    vx_status status = VX_SUCCESS;

    vx_char module[VX_INT_MAX_PATH];
    vx_uint32 m = 0, offset = 0;
    vx_unpublish_kernels_f unpublish = NULL;
    gcmHEADER_ARG("context=%p, name=%s", context, name);
    gcmDUMP_API("$VX vxUnloadKernels: context=%p, name=%s", context, name);
    gcoOS_PrintStrSafe(module, VX_INT_MAX_PATH, &offset, VX_MODULE_NAME("%s"), (name ? name : "openvx-ext"));

    if (vxoContext_IsValid(context) == vx_false_e)
    {
        gcmFOOTER_ARG("%d", VX_ERROR_INVALID_REFERENCE);
        return VX_ERROR_INVALID_REFERENCE;
    }

    for (m = 0; m < VX_MAX_MODULE_COUNT; m++)
    {
        if (context->moduleTable[m].handle != NULL && strncmp(name, context->moduleTable[m].name, VX_INT_MAX_PATH) == 0)
        {
            vx_symbol_handle sym = vxGetSymbol(context->moduleTable[m].handle, "vxUnpublishKernels");
            unpublish = (vx_unpublish_kernels_f)sym;
            if (unpublish == NULL)
            {
                vxError("Failed to load symbol vxUnpublishKernels\n");
                status = VX_ERROR_INVALID_MODULE;
            }
            else
            {
                vxInfo("Calling %s unpublish function\n", module);
                status = unpublish((vx_context)context);
                if (status != VX_SUCCESS)
                {
                    vxError("Failed to unpublish kernels in module\n");
                }
                else
                {
                    vxUnloadModule(context->moduleTable[m].handle);
                    context->moduleTable[m].handle = NULL;
                    context->moduleCount--;
                    return VX_SUCCESS;
                }
            }
        }
    }

    vxError("Failed to find module %s in libraries path\n", module);
    gcmFOOTER_ARG("%d", status);
    return status;
}

VX_INTERNAL_API vx_status vxoShader_GetUniformSize(vx_shader shader, vx_char * name, vx_uint32 *size)
{
    vx_argument         arg;
    gctUINT             i;

    gcmHEADER_ARG("shader=%p, name=%s, size=%p", shader, name, size);

    for (i = 0; i < shader->numArgs; i++)
    {
        arg = &shader->args[i];

        if (arg->uniform == gcvNULL) continue;

        if ((gcoOS_StrCmp(arg->uniform->name, name) == gcvSTATUS_OK))
        {
            *size = arg->size;
            gcmFOOTER_ARG("%d", VX_SUCCESS);
            return VX_SUCCESS;
        }
    }

    *size = 0;

    gcmFOOTER_ARG("%d", VX_FAILURE);
    return VX_FAILURE;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoImportKernelFromFile(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    vx_status status = VX_SUCCESS;
    vx_kernel kernel = node->kernel;
    vx_binary_loader_s *binaryLoad = (vx_binary_loader_s*)kernel->base.reserved;

    gcmHEADER_ARG("node=%p, parameters=%p, num=0x%x", node, parameters, num);

    if (!node->binLoadMem->statesBuff)
    {
        gcmFOOTER_ARG("%d", VX_ERROR_NOT_IMPLEMENTED);
        return VX_ERROR_NOT_IMPLEMENTED;
    }

    vxmONERROR(vxoGraphBinary_Run(node, binaryLoad));

OnError:
    gcmFOOTER_ARG("%d", status);
    return status;
}
VX_PRIVATE_API vx_status VX_CALLBACK vxoImportKernelFromFile_Initializer(vx_node node, const vx_reference parameters[], vx_uint32 num)
{
    vx_kernel            kernel = node->kernel;
    vx_binary_loader_s   *binaryLoad = (vx_binary_loader_s*)kernel->base.reserved;
    vx_status            status = VX_SUCCESS;
    vx_uint32            numParams = binaryLoad->fixed.header.inputCount + binaryLoad->fixed.header.outputCount;

    gcmHEADER_ARG("node=%p, parameters=%p, num=0x%x", node, parameters, num);

    if (num != numParams)
    {
        vxError("fail import kernel from file initializer, parameter num error");
        gcmFOOTER_ARG("%d", VX_FAILURE);
        return VX_FAILURE;
    }

    if (VX_NULL == node->binLoadMem)
    {
        if (gcmIS_ERROR(gcoOS_Allocate(gcvNULL, sizeof(vx_binaryLoad_memory_s), (gctPOINTER*)&node->binLoadMem)))
        {
            vxError("fail to allocate memory for binLoadMem\n");
            vxmONERROR(VX_FAILURE);
        }
        else
        {
            gcoOS_MemFill((gctPOINTER)node->binLoadMem, 0, sizeof(vx_binaryLoad_memory_s));
        }
    }

    /* use loading data to generate states buffer for nn/tp/sh */
    vxmONERROR(vxoGraphBinary_GenerateStatesBuffer(node, binaryLoad));

    vxmONERROR(vxoGraphBinary_WrapNBGKernel(node, binaryLoad));

    gcmFOOTER_ARG("%d", status);
    return status;

OnError:
    vxError("fail in import kernel from file initializer\n");
    gcmFOOTER_ARG("%d", VX_FAILURE);
    return VX_FAILURE;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoImportKernelFromFile_Deinitializer(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    gceSTATUS   status = gcvSTATUS_OK;

    gcmHEADER_ARG("node=%p, parameters=%p, num=0x%x", node, parameters, num);
    gcmONERROR(vxoGraphBinary_ReleaseStatesBuffer(node));

    if (node->binLoadMem != VX_NULL)
    {
        gcmVERIFY_OK(gcoOS_Free(gcvNULL, (gctPOINTER)node->binLoadMem));
        node->binLoadMem = VX_NULL;
    }

OnError:
    gcmFOOTER_ARG("%d", status);
    return status;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoImportKernelFromFile_ValidateInput(vx_node node, vx_uint32 index)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoImportKernelFromFile_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr)
{
    return VX_SUCCESS;
}

VX_API_ENTRY vx_kernel VX_API_CALL vxImportKernelFromURL(vx_context context, const vx_char * type, const vx_char * url)
{
    vx_binary_loader_s       *binaryLoad = VX_NULL;
    vx_kernel                kernel = VX_NULL;
    vx_status                status = VX_SUCCESS;
    vx_uint32                numParams = 0;
    vx_char                  kernelName[VX_MAX_KERNEL_NAME];
    vx_char                  targetName[VX_MAX_TARGET_NAME] = VX_DEFAULT_TARGET_NAME;
    vx_uint32                i = 0;
    vx_uint32                targetIndex = 0;

    gcmHEADER_ARG("context=%p, type=%s, url=%s", context, type, url);
    gcmDUMP_API("$VX vxImportKernelFromURL: context=%p, type=%s, url=%s", context, type, url);

    if (!vxoContext_IsValid(context))
    {
        gcmFOOTER_NO();
        return VX_NULL;
    }
    if (gcoOS_StrCmp(type, VX_VIVANTE_IMPORT_KERNEL_FROM_FILE) == gcvSTATUS_OK)
    {
        vx_uint32   urlLen = 0;
        vx_char     *cPtr = VX_NULL;
        vx_target   target = VX_NULL;
        /* load binary file */
        vxmONERROR(vxoGraphBinary_LoadFile(context, &binaryLoad, url));
        numParams = binaryLoad->fixed.header.inputCount + binaryLoad->fixed.header.outputCount;

        /* use url to get kernel's name*/
        urlLen = (vx_uint32)strlen(url);
        cPtr = strrchr(url, '/');
        if (cPtr != VX_NULL)
        {
            vx_char *tmp = strrchr(cPtr + 1, '.');
            if (tmp != VX_NULL)
            {
                vxStrCopySafe(kernelName, tmp - cPtr, cPtr + 1);
            }
            else
            {
                vxStrCopySafe(kernelName, strlen(cPtr), cPtr + 1);
            }
        }
        else
        {
            vx_char *tmp = strrchr(url, '.');
            if (tmp != VX_NULL)
            {
                vxStrCopySafe(kernelName, urlLen - strlen(tmp), url);
            }
            else
            {
                vxStrCopySafe(kernelName, VX_MAX_KERNEL_NAME, url);
            }
        }

        for (targetIndex = 0; targetIndex < context->targetCount; targetIndex++)
        {
            target = &context->targetTable[targetIndex];

            if (vxIsSameString(targetName, target->name, VX_MAX_TARGET_NAME))
            {
                vxmASSERT(target->funcs.addkernel);

                kernel = target->funcs.addkernel(target, kernelName,
                                                 VX_KERNEL_IMPORT_FROM_FILE,
                                                 VX_NULL, vxoImportKernelFromFile,
                                                 numParams, VX_NULL,
                                                 vxoImportKernelFromFile_ValidateInput,
                                                 vxoImportKernelFromFile_ValidateOutput,
                                                 vxoImportKernelFromFile_Initializer,
                                                 vxoImportKernelFromFile_Deinitializer);

                vxoReference_Increment(&kernel->base, VX_REF_EXTERNAL);
                break;
            }
        }

        kernel->attributes.borderMode.mode               = VX_BORDER_UNDEFINED;
        kernel->attributes.borderMode.constant_value.U32 = 0;
        /* graph binary only support GPU nodes now */
        kernel->attributes.isGPUKernel                   = vx_true_e;
        kernel->attributes.isAllGPU                      = vx_true_e;

        for(i = 0; i < binaryLoad->fixed.header.inputCount; i++)
        {
            vx_binary_input_output_info_s *inputs = binaryLoad->inputs;
            vx_enum dataType = vxoGraphBinary_ConvertToOVXDataType(inputs[i].dataType);

            status |= vxAddParameterToKernel(kernel, i, VX_INPUT,
                                             dataType, VX_PARAMETER_STATE_REQUIRED);
        }
        for(i = binaryLoad->fixed.header.inputCount; i < numParams; i++)
        {
            vx_binary_input_output_info_s *outputs = binaryLoad->outputs;
            vx_uint32 outIndex = i - binaryLoad->fixed.header.inputCount;
            vx_enum dataType = vxoGraphBinary_ConvertToOVXDataType(outputs[outIndex].dataType);

            status |= vxAddParameterToKernel(kernel, i, VX_OUTPUT,
                                             dataType, VX_PARAMETER_STATE_REQUIRED);
        }
        vxmONERROR(status);
        vxmONERROR(vxFinalizeKernel(kernel));
        kernel->isUserkernel            = vx_false_e;
        kernel->base.reserved           = binaryLoad;
        binaryLoad->kernel              = kernel;
    }
    else
    {
        vxError("no implement this type: %s\n", type);
        kernel = VX_NULL;
    }

    gcmFOOTER_ARG("kernel=%p", kernel);
    return kernel;
OnError:
    vxError("fail to import kernel from %s, error code: %d\n", url, status);
    gcmFOOTER_NO();
    return VX_NULL;
}

